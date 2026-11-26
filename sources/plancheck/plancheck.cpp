/*
	Copyright 2006-2026 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-2.0-or-later
#include "plancheck.h"

#include "../TerminalStrip/realterminal.h"
#include "../cable/cable.h"
#include "../dataBase/projectdatabase.h"
#include "../devicetype/devicetypecheck.h"
#include "../diagram.h"
#include "../diagramcontent.h"
#include "../duplicatelabels.h"
#include "../fluid/fluiddata.h"
#include "../fluid/fluidtypes.h"
#include "../panel/panelclimate.h"
#include "../panel/panellayout.h"
#include "../panel/panelwiring.h"
#include "../qet.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/qetshapeitem.h"
#include "../qetgraphicsitem/terminal.h"
#include "../qetgraphicsitem/terminalelement.h"
#include "../qetproject.h"
#include "../utils/qetsettings.h"

#include <QCoreApplication>
#include <QHash>
#include <QJsonArray>
#include <QSqlQuery>
#include <QSet>

#include <algorithm>
#include <climits>

namespace {

using PlanCheck::Finding;
using Severity = ProjectCheck::Severity;

	/// the rules in the order of the report, EPLAN's numbers and the
	/// category of the panel they are listed in
using Kind = ProjectCheck::Kind;
struct RuleDef { const char *key; const char *eplan; Kind kind; };
const RuleDef kRules[] = {
	{"contact_without_master",  "007005", Kind::Links},
	{"report_without_partner",  "011002", Kind::Links},
	{"solenoid_without_coil",   "",       Kind::Links},
	{"coil_without_contact",    "007009", Kind::Links},
	{"plc_channel_address",     "004023", Kind::PlcChannels},
	{"cable_without_target",    "003005", Kind::Cables},
	{"terminal_without_strip",  "001010", Kind::TerminalStrips},
	{"unlabelled_device",       "017001", Kind::LabelsAndParts},
	{"device_without_part",     "007017", Kind::LabelsAndParts},
	{"open_terminal",           "005026", Kind::OpenTerminals},
	{"open_port",               "005026", Kind::OpenTerminals},
	{"cabinet_layout",          "",       Kind::CabinetLayout},
};

int checkOrder(const QString &key)
{
	int i = 0;
	for (const RuleDef &c : kRules) {
		if (key == QLatin1String(c.key)) return i;
		++i;
	}
	return i;
}

QString eplanOf(const QString &key)
{
	for (const RuleDef &c : kRules)
		if (key == QLatin1String(c.key)) return QString::fromLatin1(c.eplan);
	return QString();
}

class Collector
{
	public:
		explicit Collector(QETProject *project) : m_project(project) {}

			/// @p eplan: a null string takes the number of the check,
			/// an empty one says EPLAN has none for this case
		void add(Severity severity, const QString &check, Diagram *folio,
				 const QString &label, const QString &text,
				 const QList<QUuid> &items = {},
				 const QString &eplan = QString())
		{
			Finding f;
			f.severity = severity;
			f.rule = check;
			f.eplan = eplan.isNull() ? eplanOf(check) : eplan;
			f.folio = folio;
			f.label = label;
			f.text = text;
			f.items = items;
			m_findings << f;
		}
		QVector<Finding> m_findings;
	private:
		QETProject *m_project;
};

QString labelOf(Element *element)
{
	return DuplicateLabels::displayedLabel(element).trimmed();
}

	/// a part of a cabinet layout, not a symbol of the schematic
bool isPanelDevice(Element *element)
{
	return !element->elementInformations()
			.value(QStringLiteral("panel_of")).toString().isEmpty();
}

bool isReport(const ElementData &data)
{
	return data.m_type == ElementData::NextReport
		|| data.m_type == ElementData::PreviousReport;
}

bool hasNoPart(Element *element)
{
	const DiagramContext info = element->elementInformations();
	return info.value(QStringLiteral("manufacturer_reference")).toString().trimmed().isEmpty()
		&& !QET::infoFlagIsTrue(info.value(QStringLiteral("exclude_from_bom")).toString());
}

QString nameOf(Terminal *terminal, int index)
{
	const QString n = terminal->name().trimmed();
	return n.isEmpty() ? QStringLiteral("#%1").arg(index + 1) : n;
}

int plcRow(Element *card, Element *channel)
{
	const int row = card->groupIndexForElement(channel);
	return row >= 0 ? row : channel->groupIndexForElement(card);
}

using LooseTerminals = QHash<QString, QList<QPair<Diagram *, Element *>>>;

/**
	The checks of one symbol of the schematic: open terminals and ports,
	links (contacts, reports, PLC channels), label and part.
*/
void checkElement(Collector &c, Diagram *d, Element *e, LooseTerminals &loose_terminals)
{
	const ElementData data = e->elementData();
	if (data.m_type == ElementData::Thumbnail
		|| data.m_type == ElementData::ConductorDefinition)
		return;
	const QString label = labelOf(e);
	const QString shown = label.isEmpty() ? e->name() : label;
	const QList<QUuid> self{e->uuid()};
	const QList<Element *> linked = e->linkedElements();

		//Folio reports: a partner, and something on them
	if (isReport(data)) {
		if (linked.isEmpty())
			c.add(Severity::Error, QStringLiteral("report_without_partner"), d, shown,
				  ProjectCheck::tr("Folio report without its partner: it leads nowhere"), self);
		bool wired = false;
		for (Terminal *t : e->terminals())
			if (!t->conductors().isEmpty()) wired = true;
		if (!wired && !e->terminals().isEmpty())
			c.add(Severity::Warning, QStringLiteral("report_without_partner"), d, shown,
				  ProjectCheck::tr("Folio report connected to nothing"), self, QStringLiteral("011003"));
		return;
	}

		//Open terminals and fluid ports, one finding per symbol
	QStringList open, ports;
	const QList<Terminal *> terminals = e->terminals();
	for (int i = 0; i < terminals.count(); ++i) {
		Terminal *t = terminals.at(i);
		switch (t->medium()) {
			case Fluid::Medium::Electric:
				if (t->conductors().isEmpty()) open << nameOf(t, i);
				break;
			case Fluid::Medium::Pneumatic:
			case Fluid::Medium::Hydraulic:
				if (Fluid::isOpenPort(t->medium(), t->portEnd(), t->conductorsCount()))
					ports << nameOf(t, i);
				break;
			default:
				break;
		}
	}
	const bool is_terminal = data.m_type == ElementData::Terminal;
		//a symbol with no wire at all is the panel's own check ("Symbols
		//with no wire"); a strip terminal only when that check leaves
		//strip terminals out
	bool any_wired = false;
	for (Terminal *t : terminals)
		if (t->conductorsCount()) any_wired = true;
	if (!any_wired && !(is_terminal && QetSettings::unwiredSymbolsSkipStripTerminals())) {
		open.clear();
		ports.clear();
	}
	if (!open.isEmpty()) {
		if (is_terminal)
			c.add(Severity::Warning, QStringLiteral("open_terminal"), d, shown,
				  ProjectCheck::tr("Terminal not connected on both sides: %1 open").arg(open.join(QStringLiteral(", "))),
				  self, QStringLiteral("005053"));
		else
			c.add(Severity::Info, QStringLiteral("open_terminal"), d, shown,
				  ProjectCheck::tr("%n unconnected terminal(s): %1", nullptr, int(open.count()))
					  .arg(open.join(QStringLiteral(", "))), self);
	}
	if (!ports.isEmpty())
		c.add(Severity::Warning, QStringLiteral("open_port"), d, shown,
			  ProjectCheck::tr("%n open fluid port(s) without line, silencer or plug: %1", nullptr, int(ports.count()))
				  .arg(ports.join(QStringLiteral(", "))), self);

		//Fluid power: solenoids and their coils (as "Check fluid power...")
	const int sides = data.m_fluid.solenoidSides();
	if (sides > linked.size())
		c.add(Severity::Warning, QStringLiteral("solenoid_without_coil"), d, shown,
			  ProjectCheck::tr("%n solenoid(s) without a coil", nullptr, sides - int(linked.size())), self);
	if (data.m_fluid.m_kind == FluidData::Kind::Solenoid) {
		if (linked.isEmpty())
			c.add(Severity::Warning, QStringLiteral("solenoid_without_coil"), d, shown,
				  ProjectCheck::tr("Valve coil linked to no valve"), self);
		return;
	}

		//Contacts and PLC channels without their master
	if (data.m_type == ElementData::Slave) {
		if (linked.isEmpty()) {
			if (data.m_slave_type == ElementData::PLCSlave)
				c.add(Severity::Error, QStringLiteral("plc_channel_address"), d, shown,
					  ProjectCheck::tr("PLC channel assigned to no PLC card, so it has no address"), self);
			else
				c.add(Severity::Error, QStringLiteral("contact_without_master"), d, shown,
					  ProjectCheck::tr("Contact without its coil or main function"), self);
		} else if (data.m_slave_type == ElementData::PLCSlave) {
			Element *card = linked.first();
			const int row = plcRow(card, e);
			const auto ios = card->elementData().plcMasterData().ios;
			if (row < 0 || row >= ios.count()) {
				if (!ios.isEmpty())
					c.add(Severity::Warning, QStringLiteral("plc_channel_address"), d, shown,
						  ProjectCheck::tr("PLC channel linked to %1 but to none of its channels").arg(labelOf(card)),
						  self, QStringLiteral("004027"));
			} else if (ios.at(row).address.trimmed().isEmpty()) {
				c.add(Severity::Warning, QStringLiteral("plc_channel_address"), d, shown,
					  ProjectCheck::tr("PLC channel without an address (channel %1 of %2)")
						  .arg(row + 1).arg(labelOf(card)), self);
			}
		}
		return;
	}

		//Terminals: in a strip, with a part
	if (is_terminal) {
		auto *te = qobject_cast<TerminalElement *>(e);
		const auto real = te ? te->realTerminal() : QSharedPointer<RealTerminal>();
		if (!real || !real->parentStrip())
			loose_terminals[label.section(QLatin1Char(':'), 0, 0)] << qMakePair(d, e);
		if (hasNoPart(e))
			c.add(Severity::Info, QStringLiteral("device_without_part"), d, shown,
				  ProjectCheck::tr("Terminal without a part number: the bill of materials leaves it out"), self);
		return;
	}

	if (data.m_type != ElementData::Master && data.m_type != ElementData::Simple)
		return;

		//Masters: a label, contacts, the used PLC channels with an address
	if (data.m_type == ElementData::Master) {
		if (label.isEmpty())
			c.add(Severity::Warning, QStringLiteral("unlabelled_device"), d, e->name(),
				  ProjectCheck::tr("Device without a label: its contacts cannot point to it"), self);
		if (data.m_master_type == ElementData::Coil && linked.isEmpty() && sides == 0)
			c.add(Severity::Info, QStringLiteral("coil_without_contact"), d, shown,
				  ProjectCheck::tr("Coil without any contact"), self);
		if (data.m_master_type == ElementData::PLC) {
				//rows with a linked channel are reported at the channel;
				//here only rows used by a cross reference alone
			const auto ios = data.plcMasterData().ios;
			QSet<int> linked_rows, used;
			for (Element *channel : linked) {
				const int row = plcRow(e, channel);
				if (row >= 0) linked_rows << row;
			}
			for (int row = 0; row < ios.count(); ++row)
				if (!ios.at(row).crossRef.trimmed().isEmpty() && !linked_rows.contains(row))
					used << row;
			QList<int> rows(used.begin(), used.end());
			std::sort(rows.begin(), rows.end());
			for (int row : std::as_const(rows))
				if (row < ios.count() && ios.at(row).address.trimmed().isEmpty())
					c.add(Severity::Warning, QStringLiteral("plc_channel_address"), d, shown,
						  ProjectCheck::tr("Channel %1 of the PLC card is used but has no address").arg(row + 1), self);
		}
	}

		//The part, for the bill of materials (a label marks a device)
	if ((data.m_type == ElementData::Master || !label.isEmpty()) && hasNoPart(e))
		c.add(data.m_type == ElementData::Master ? Severity::Warning : Severity::Info,
			  QStringLiteral("device_without_part"), d, shown,
			  ProjectCheck::tr("Device without a part number: the bill of materials shows it without one"), self);
}

using Wires = QHash<QUuid, QPair<Diagram *, Conductor *>>;

void checkCables(Collector &c, QETProject *project, const Wires &wires)
{
	for (Cable *cable : project->cables()) {
		const QString name = cable->designation().trimmed();
		Diagram *folio = nullptr;
		for (const CablePartData &part : cable->parts()) {
			folio = project->diagramByUuid(part.diagram);
			if (folio) break;
		}
		const QString shown = name.isEmpty() ? cable->type() : name;
		const QList<CableCore> cores = cable->usedCores();
		QList<QUuid> items;
		for (const CableCore &core : cores)
			if (wires.contains(core.conductor)) items << core.conductor;

		if (name.isEmpty())
			c.add(Severity::Warning, QStringLiteral("cable_without_target"), folio, shown,
				  ProjectCheck::tr("Cable without a label"), items, QStringLiteral("017001"));
		if (cores.isEmpty()) {
			c.add(Severity::Warning, QStringLiteral("cable_without_target"), folio, shown,
				  ProjectCheck::tr("Cable without a target: none of its %n core(s) is on a wire", nullptr,
					 cable->coreCount()), items, QString(""));
			continue;
		}
		QStringList gone, unlabelled;
		for (const CableCore &core : cores) {
			const QString color = cable->colorOfCore(core.core);
			const QString core_name = color.isEmpty()
				? QString::number(core.core + 1)
				: QStringLiteral("%1 (%2)").arg(core.core + 1).arg(color);
			if (!wires.contains(core.conductor)) {
				gone << core_name;
				continue;
			}
			const auto hit = wires.value(core.conductor);
			if (!folio) folio = hit.first;
			for (Terminal *t : {hit.second->terminal1, hit.second->terminal2}) {
				Element *end = t ? t->parentElement() : nullptr;
				if (!end || isReport(end->elementData())) continue;
				if (labelOf(end).isEmpty()) {
					unlabelled << core_name;
					break;
				}
			}
		}
		if (!gone.isEmpty())
			c.add(Severity::Error, QStringLiteral("cable_without_target"), folio, shown,
				  ProjectCheck::tr("Core(s) %1 lead to a wire that no longer exists").arg(gone.join(QStringLiteral(", "))),
				  items, QString(""));
		if (!unlabelled.isEmpty())
			c.add(Severity::Warning, QStringLiteral("cable_without_target"), folio, shown,
				  ProjectCheck::tr("Cable target without a label: core(s) %1 end at a symbol without a label")
					  .arg(unlabelled.join(QStringLiteral(", "))), items);
	}
}

} // namespace

ProjectCheck::Kind PlanCheck::kindOf(const QString &rule)
{
	for (const RuleDef &c : kRules)
		if (rule == QLatin1String(c.key)) return c.kind;
	return Kind::Links;
}

QVector<PlanCheck::Finding> PlanCheck::run(QETProject *project)
{
	if (!project) return {};
		//links reach across folios
	if (project->unloadedFolioCount()) project->loadFolios();

	Collector c(project);
	const QList<Diagram *> diagrams = project->diagrams();

		//cabinet layouts are checked as layouts, not as schematics
	QSet<Diagram *> panel_folios;
	for (Diagram *d : diagrams) {
		const QList<QetShapeItem *> surfaces = PanelLayout::surfaces(d);
		if (surfaces.isEmpty()) continue;
		panel_folios << d;
		for (QetShapeItem *surface : surfaces) {
			const PanelLayout layout(surface);
			QStringList warnings = layout.warnings();
				// the plate inside the enclosure: full ducts, cooling needed
			if (surface->panelData().face.isEmpty()) {
				warnings << PanelWiring::route(layout).warnings();
				const PanelClimate climate = PanelClimate::compute(surface);
				if (climate.needsCooling()) warnings << climate.summary();
			}
			for (const QString &w : std::as_const(warnings)) {
				const QString tag = w.section(QLatin1Char(':'), 0, 0).trimmed();
				const QString rest = w.section(QLatin1Char(':'), 1).trimmed();
				c.add(Severity::Warning, QStringLiteral("cabinet_layout"), d,
					  rest.isEmpty() ? QString() : tag, rest.isEmpty() ? w : rest);
			}
		}
	}

	Wires wires;
	LooseTerminals loose_terminals;
	for (Diagram *d : diagrams) {
		for (Conductor *w : d->content().conductors())
			wires.insert(w->uuid(), qMakePair(d, w));
		if (panel_folios.contains(d)) continue;
		for (Element *e : d->elements()) {
			if (isPanelDevice(e)) continue;
			checkElement(c, d, e, loose_terminals);
		}
	}

		//terminals without a strip: one finding per strip label
	for (auto it = loose_terminals.cbegin(); it != loose_terminals.cend(); ++it) {
		QList<QUuid> items;
		for (const auto &pair : it.value()) items << pair.second->uuid();
		const QString strip = it.key().isEmpty() ? ProjectCheck::tr("(no label)") : it.key();
		c.add(Severity::Warning, QStringLiteral("terminal_without_strip"),
			  it.value().first().first, strip,
			  ProjectCheck::tr("%n terminal(s) in no terminal strip: the terminal diagram leaves them out",
				 nullptr, int(it.value().count())), items);
	}

	checkCables(c, project, wires);

	QVector<Finding> out = c.m_findings;
	std::stable_sort(out.begin(), out.end(), [project](const Finding &a, const Finding &b) {
		if (a.severity != b.severity) return int(a.severity) < int(b.severity);
		const int fa = a.folio ? project->folioIndex(a.folio) : INT_MAX;
		const int fb = b.folio ? project->folioIndex(b.folio) : INT_MAX;
		if (fa != fb) return fa < fb;
		const int ca = checkOrder(a.rule), cb = checkOrder(b.rule);
		if (ca != cb) return ca < cb;
		return QString::localeAwareCompare(a.label, b.label) < 0;
	});
	return out;
}

/* ---- ProjectCheck: the fork's part of the Check panel ---------------- */

namespace {

	/// "A-03 — Power", as ProjectCheck words a folio
QString folioTextOf(QETProject *project, Diagram *diagram)
{
	if (!project || !diagram) return QString();
	QString folio = diagram->border_and_titleblock.finalfolio().trimmed();
	if (folio.isEmpty())
		folio = QString::number(project->folioIndex(diagram) + 1);
	return QStringLiteral("%1 — %2").arg(folio, diagram->title());
}

ProjectCheck::Place placeOf(QETProject *project, QGraphicsObject *item, const QString &text)
{
	ProjectCheck::Place place;
	place.text = text;
	place.item = item;
	if (Diagram *diagram = DuplicateLabels::diagramOf(item)) {
		place.folio = folioTextOf(project, diagram);
		place.cell = diagram->convertPosition(item->sceneBoundingRect().center()).toString();
	}
	return place;
}

QString placeText(QGraphicsObject *item)
{
	if (auto *e = qobject_cast<Element *>(item)) {
		const QString label = labelOf(e);
		return label.isEmpty() ? e->name() : label;
	}
	if (auto *w = qobject_cast<Conductor *>(item)) {
		const QString text = w->properties().text.trimmed();
		return text.isEmpty() ? ProjectCheck::tr("Wire") : ProjectCheck::tr("Wire %1").arg(text);
	}
	return QString();
}

bool onPanel(Element *e)
{
	return isPanelDevice(e) || !PanelLayout::surfaces(e->diagram()).isEmpty();
}

	/// The fork's severity of each finding of the panel's own checks, and
	/// its rule; findings the fork's rules report better are dropped
void rate(ProjectCheck::Result &result)
{
	using K = ProjectCheck::Kind;
	QVector<ProjectCheck::Finding> kept;
	for (ProjectCheck::Finding f : std::as_const(result.findings)) {
		switch (result.kind) {
			case K::DuplicateLabels: {
					//a wire number on separate potentials is a warning; one
					//label on separate devices an error, unless they are the
					//terminals of one strip (X1:1, X1:2) or the cabinet
					//layout's copy of a device
				bool wires = false;
				QSet<QUuid> devices;
				for (const auto &place : std::as_const(f.places)) {
					if (qobject_cast<Conductor *>(place.item.data())) wires = true;
					auto *e = qobject_cast<Element *>(place.item.data());
					if (!e || onPanel(e)) continue;
					if (e->elementData().m_type == ElementData::Terminal) continue;
					const QList<Element *> linked = e->linkedElements();
					Element *head = (e->linkType() & Element::Slave) && !linked.isEmpty()
						? linked.first() : e;
					devices << head->uuid();
				}
				if (wires) {
					f.severity = Severity::Warning;
					f.rule = QStringLiteral("duplicate_wire_number");
				} else {
					f.severity = devices.size() >= 2 ? Severity::Error : Severity::Info;
					f.rule = QStringLiteral("duplicate_label");
					f.eplan = QStringLiteral("017005");
				}
				break;
			}
			case K::PotentialTypes:
				f.severity = Severity::Error;
				f.rule = QStringLiteral("potential_types");
				f.eplan = QStringLiteral("005016");
				break;
			case K::OpenWireEnds:
				f.severity = Severity::Warning;
				f.rule = QStringLiteral("open_wire_end");
				break;
			case K::UnwiredSymbols:
				f.severity = Severity::Info;
				f.rule = QStringLiteral("unwired_symbol");
				f.eplan = QStringLiteral("005026");
				break;
			case K::DeviceTypes: {
					//the contacts linked to no master: the rule
					//"contact_without_master" lists every one of them
				const int n = int(f.places.size());
				if (f.text == ProjectCheck::tr("%n contact(s) linked to no master", nullptr, n))
					continue;
				f.severity = Severity::Warning;
				f.rule = QStringLiteral("device_type");
				f.eplan = QStringLiteral("007001");
				break;
			}
			case K::WiresPerTerminal:
				f.severity = Severity::Warning;
				f.rule = QStringLiteral("wires_per_terminal");
				break;
			case K::TerminalNames:
				f.severity = Severity::Warning;
				f.rule = QStringLiteral("terminal_names");
				break;
			default:
				break;
		}
		kept << f;
	}
		//the worst first, as the fork's rules
	std::stable_sort(kept.begin(), kept.end(),
		[](const ProjectCheck::Finding &a, const ProjectCheck::Finding &b) {
			return int(a.severity) < int(b.severity);
		});
	result.findings = kept;
}

} // namespace

bool ProjectCheck::isForkKind(Kind kind)
{
	switch (kind) {
		case Kind::Links:
		case Kind::PlcChannels:
		case Kind::Cables:
		case Kind::TerminalStrips:
		case Kind::LabelsAndParts:
		case Kind::OpenTerminals:
		case Kind::CabinetLayout:
			return true;
		default:
			return false;
	}
}

QString ProjectCheck::forkTitle(Kind kind)
{
	switch (kind) {
		case Kind::Links: return tr("Contacts, coils and folio reports without their partner");
		case Kind::PlcChannels: return tr("PLC channels without an address");
		case Kind::Cables: return tr("Cables without a target");
		case Kind::TerminalStrips: return tr("Terminals without a terminal strip");
		case Kind::LabelsAndParts: return tr("Devices without a label or part");
		case Kind::OpenTerminals: return tr("Unconnected terminals");
		case Kind::CabinetLayout: return tr("Cabinet layout");
		default: return QString();
	}
}

QString ProjectCheck::forkToolTip(Kind kind)
{
	switch (kind) {
		case Kind::Links:
			return tr("Contacts linked to no coil or main function, coils without any contact, "
					  "solenoid valves without their coil and folio reports without their partner "
					  "(EPLAN 007005, 007009, 011002).");
		case Kind::PlcChannels:
			return tr("PLC channels assigned to no PLC card or to a channel without an address "
					  "(EPLAN 004023, 004027).");
		case Kind::Cables:
			return tr("Cables without a label, without cores on a wire, with cores on a wire "
					  "that no longer exists or ending at a symbol without a label (EPLAN 003005).");
		case Kind::TerminalStrips:
			return tr("Terminals which are in no terminal strip: the terminal diagram leaves "
					  "them out (EPLAN 001010).");
		case Kind::LabelsAndParts:
			return tr("Devices without a label, and devices without a part number, which the "
					  "bill of materials shows without one (EPLAN 017001, 007017).");
		case Kind::OpenTerminals:
			return tr("Terminals and fluid ports left open on a symbol that has wires "
					  "(EPLAN 005026, 005053). A symbol with no wire at all is listed under "
					  "Symbols with no wire.");
		case Kind::CabinetLayout:
			return tr("The warnings of the cabinet layouts: devices outside their mounting "
					  "surface, overlapping or without a size.");
		default: return QString();
	}
}

QString ProjectCheck::key(Kind kind)
{
	switch (kind) {
		case Kind::DuplicateLabels: return QStringLiteral("duplicate_labels");
		case Kind::PotentialTypes: return QStringLiteral("potential_types");
		case Kind::OpenWireEnds: return QStringLiteral("open_wire_ends");
		case Kind::UnwiredSymbols: return QStringLiteral("unwired_symbols");
		case Kind::DeviceTypes: return QStringLiteral("device_types");
		case Kind::WiresPerTerminal: return QStringLiteral("wires_per_terminal");
		case Kind::TerminalNames: return QStringLiteral("terminal_names");
		case Kind::Links: return QStringLiteral("links");
		case Kind::PlcChannels: return QStringLiteral("plc_channels");
		case Kind::Cables: return QStringLiteral("cables");
		case Kind::TerminalStrips: return QStringLiteral("terminal_strips");
		case Kind::LabelsAndParts: return QStringLiteral("labels_and_parts");
		case Kind::OpenTerminals: return QStringLiteral("open_terminals");
		case Kind::CabinetLayout: return QStringLiteral("cabinet_layout");
	}
	return QString();
}

QString ProjectCheck::severityKey(Severity severity)
{
	switch (severity) {
		case Severity::Error:   return QStringLiteral("error");
		case Severity::Warning: return QStringLiteral("warning");
		default:                return QStringLiteral("info");
	}
}

QString ProjectCheck::severityText(Severity severity)
{
	switch (severity) {
		case Severity::Error:   return tr("Error");
		case Severity::Warning: return tr("Warning");
		default:                return tr("Note");
	}
}

/**
	@brief ProjectCheck::runAll
	Every check of kinds(): the panel's own ones, each finding rated, and
	the fork's rules in one pass over the folios (PlanCheck::run()), spread
	over their categories. Builds the folios not built yet.
*/
QVector<ProjectCheck::Result> ProjectCheck::runAll(QETProject *project)
{
	QVector<Result> results;
	if (!project) return results;
	if (project->unloadedFolioCount()) project->loadFolios();

	QHash<int, int> index;
	for (Kind kind : kinds()) {
		index.insert(int(kind), int(results.size()));
		if (isForkKind(kind)) {
			Result result;
			result.kind = kind;
			results << result;
			continue;
		}
		Result result = run(project, kind);
		rate(result);
		results << result;
	}

	QHash<QUuid, QGraphicsObject *> items;
	for (Diagram *d : project->diagrams()) {
		for (Element *e : d->elements()) items.insert(e->uuid(), e);
		for (Conductor *w : d->content().conductors()) items.insert(w->uuid(), w);
	}
	for (const PlanCheck::Finding &f : PlanCheck::run(project)) {
		Result &result = results[index.value(int(PlanCheck::kindOf(f.rule)))];
		Finding finding;
		finding.severity = f.severity;
		finding.rule = f.rule;
		finding.eplan = f.eplan;
		//: %1 is the label of the device, cable or strip, %2 what is wrong with it
		finding.text = f.label.isEmpty() ? f.text : tr("%1: %2").arg(f.label, f.text);
		for (const QUuid &uuid : f.items)
			if (QGraphicsObject *item = items.value(uuid))
				finding.places << placeOf(project, item,
										  f.items.size() == 1 ? f.label : placeText(item));
		if (finding.places.isEmpty()) {
			Place place;
			place.text = f.label;
			place.folio = folioTextOf(project, f.folio);
			finding.places << place;
		}
		result.findings << finding;
	}
	return results;
}

Diagram *ProjectCheck::diagramOf(QETProject *project, const Place &place)
{
	if (!project) return nullptr;
	if (place.item) return DuplicateLabels::diagramOf(place.item);
	if (!place.diagram.isNull()) return project->diagramByUuid(place.diagram);
	if (!place.wires.isEmpty()) {
		QSqlQuery query = project->dataBase()->newQuery();
		query.prepare(QStringLiteral("SELECT diagram_uuid FROM conductor WHERE uuid = :uuid"));
		for (const QUuid &wire : place.wires) {
			query.bindValue(QStringLiteral(":uuid"), wire.toString());
			if (query.exec() && query.next())
				if (Diagram *d = project->diagramByUuid(QUuid(query.value(0).toString())))
					return d;
		}
	}
	return nullptr;
}

QVariantMap ProjectCheck::toVariant(const Result &result, const Finding &finding,
									QETProject *project)
{
	Diagram *diagram = nullptr;
	QString label;
	QStringList uuids;
	for (const Place &place : finding.places) {
		if (!diagram) diagram = diagramOf(project, place);
		if (label.isEmpty()) label = place.text;
		if (auto *e = qobject_cast<Element *>(place.item.data()))
			uuids << e->uuid().toString(QUuid::WithoutBraces);
		else if (auto *w = qobject_cast<Conductor *>(place.item.data()))
			uuids << w->uuid().toString(QUuid::WithoutBraces);
		if (!place.element.isNull())
			uuids << place.element.toString(QUuid::WithoutBraces);
		for (const QUuid &wire : place.wires)
			uuids << wire.toString(QUuid::WithoutBraces);
	}
	QVariantMap m;
	m.insert(QStringLiteral("severity"), severityKey(finding.severity));
	m.insert(QStringLiteral("check"), finding.rule);
	m.insert(QStringLiteral("category"), key(result.kind));
	m.insert(QStringLiteral("categoryTitle"), title(result.kind));
	m.insert(QStringLiteral("eplan"), finding.eplan);
	m.insert(QStringLiteral("folio"), diagram ? project->folioIndex(diagram) : -1);
	m.insert(QStringLiteral("folioTitle"), diagram ? diagram->title() : QString());
	m.insert(QStringLiteral("label"), label);
	m.insert(QStringLiteral("text"), finding.text);
	m.insert(QStringLiteral("items"), uuids);
	return m;
}

QVariantList ProjectCheck::toVariantList(const QVector<Result> &results,
										 QETProject *project, int folio)
{
	QVariantList list;
	for (const Result &result : results)
		for (const Finding &finding : result.findings) {
			const QVariantMap m = toVariant(result, finding, project);
			if (folio >= 0 && m.value(QStringLiteral("folio")).toInt() != folio) continue;
			list << m;
		}
	return list;
}

QJsonObject ProjectCheck::toJson(const QVector<Result> &results, QETProject *project)
{
	QJsonArray list;
	int errors = 0, warnings = 0, infos = 0;
	for (const QVariant &v : toVariantList(results, project)) {
		const QVariantMap m = v.toMap();
		const QString severity = m.value(QStringLiteral("severity")).toString();
		if (severity == QLatin1String("error")) ++errors;
		else if (severity == QLatin1String("warning")) ++warnings;
		else ++infos;
		list.append(QJsonObject::fromVariantMap(m));
	}
	QJsonObject summary;
	summary[QStringLiteral("errors")] = errors;
	summary[QStringLiteral("warnings")] = warnings;
	summary[QStringLiteral("infos")] = infos;
	QJsonObject notes;
	for (const Result &result : results)
		if (!result.note.isEmpty()) notes[key(result.kind)] = result.note;
	QJsonObject root;
	root[QStringLiteral("project")] = project ? project->title() : QString();
	root[QStringLiteral("ok")] = errors == 0;
	root[QStringLiteral("summary")] = summary;
	if (!notes.isEmpty()) root[QStringLiteral("notes")] = notes;
	root[QStringLiteral("findings")] = list;
	return root;
}
