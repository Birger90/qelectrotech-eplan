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
#include "../panel/panellayout.h"
#include "../qet.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/qetshapeitem.h"
#include "../qetgraphicsitem/terminal.h"
#include "../qetgraphicsitem/terminalelement.h"
#include "../qetproject.h"

#include <QCoreApplication>
#include <QHash>
#include <QJsonArray>
#include <QSet>

#include <algorithm>
#include <climits>

namespace {

using PlanCheck::Finding;
using PlanCheck::Severity;

QString tr(const char *text, const char *comment = nullptr, int n = -1)
{
	return QCoreApplication::translate("PlanCheck", text, comment, n);
}

	/// the order of the checks in the report, and EPLAN's numbers
struct CheckDef { const char *key; const char *eplan; const char *title; };
const CheckDef kChecks[] = {
	{"duplicate_label",         "017005", QT_TRANSLATE_NOOP("PlanCheck", "Duplicate device labels")},
	{"duplicate_wire_number",   "",       QT_TRANSLATE_NOOP("PlanCheck", "Duplicate wire numbers")},
	{"potential_types",         "005016", QT_TRANSLATE_NOOP("PlanCheck", "Potentials with wires of different types")},
	{"contact_without_master",  "007005", QT_TRANSLATE_NOOP("PlanCheck", "Contacts without their coil or main function")},
	{"report_without_partner",  "011002", QT_TRANSLATE_NOOP("PlanCheck", "Folio reports without their partner")},
	{"plc_channel_address",     "004023", QT_TRANSLATE_NOOP("PlanCheck", "PLC channels without an address")},
	{"cable_without_target",    "003005", QT_TRANSLATE_NOOP("PlanCheck", "Cables without a target")},
	{"terminal_without_strip",  "001010", QT_TRANSLATE_NOOP("PlanCheck", "Terminals without a terminal strip")},
	{"device_without_part",     "007017", QT_TRANSLATE_NOOP("PlanCheck", "Devices without a part")},
	{"unlabelled_device",       "017001", QT_TRANSLATE_NOOP("PlanCheck", "Devices without a label")},
	{"open_terminal",           "005026", QT_TRANSLATE_NOOP("PlanCheck", "Unconnected terminals")},
	{"open_port",               "005026", QT_TRANSLATE_NOOP("PlanCheck", "Open fluid ports")},
	{"solenoid_without_coil",   "",       QT_TRANSLATE_NOOP("PlanCheck", "Solenoid valves without their coil")},
	{"coil_without_contact",    "007009", QT_TRANSLATE_NOOP("PlanCheck", "Coils without any contact")},
	{"device_type",             "007001", QT_TRANSLATE_NOOP("PlanCheck", "Devices not drawn as their device type")},
	{"cabinet_layout",          "",       QT_TRANSLATE_NOOP("PlanCheck", "Cabinet layout")},
};

int checkOrder(const QString &key)
{
	int i = 0;
	for (const CheckDef &c : kChecks) {
		if (key == QLatin1String(c.key)) return i;
		++i;
	}
	return i;
}

QString eplanOf(const QString &key)
{
	for (const CheckDef &c : kChecks)
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
			f.check = check;
			f.eplan = eplan.isNull() ? eplanOf(check) : eplan;
			f.folio = folio ? m_project->folioIndex(folio) : -1;
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
				  tr("Folio report without its partner: it leads nowhere"), self);
		bool wired = false;
		for (Terminal *t : e->terminals())
			if (!t->conductors().isEmpty()) wired = true;
		if (!wired && !e->terminals().isEmpty())
			c.add(Severity::Warning, QStringLiteral("report_without_partner"), d, shown,
				  tr("Folio report connected to nothing"), self, QStringLiteral("011003"));
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
	if (!open.isEmpty()) {
		if (is_terminal)
			c.add(Severity::Warning, QStringLiteral("open_terminal"), d, shown,
				  tr("Terminal not connected on both sides: %1 open").arg(open.join(QStringLiteral(", "))),
				  self, QStringLiteral("005053"));
		else
			c.add(Severity::Info, QStringLiteral("open_terminal"), d, shown,
				  tr("%n unconnected terminal(s): %1", nullptr, int(open.count()))
					  .arg(open.join(QStringLiteral(", "))), self);
	}
	if (!ports.isEmpty())
		c.add(Severity::Warning, QStringLiteral("open_port"), d, shown,
			  tr("%n open fluid port(s) without line, silencer or plug: %1", nullptr, int(ports.count()))
				  .arg(ports.join(QStringLiteral(", "))), self);

		//Fluid power: solenoids and their coils (as "Check fluid power...")
	const int sides = data.m_fluid.solenoidSides();
	if (sides > linked.size())
		c.add(Severity::Warning, QStringLiteral("solenoid_without_coil"), d, shown,
			  tr("%n solenoid(s) without a coil", nullptr, sides - int(linked.size())), self);
	if (data.m_fluid.m_kind == FluidData::Kind::Solenoid) {
		if (linked.isEmpty())
			c.add(Severity::Warning, QStringLiteral("solenoid_without_coil"), d, shown,
				  tr("Valve coil linked to no valve"), self);
		return;
	}

		//Contacts and PLC channels without their master
	if (data.m_type == ElementData::Slave) {
		if (linked.isEmpty()) {
			if (data.m_slave_type == ElementData::PLCSlave)
				c.add(Severity::Error, QStringLiteral("plc_channel_address"), d, shown,
					  tr("PLC channel assigned to no PLC card, so it has no address"), self);
			else
				c.add(Severity::Error, QStringLiteral("contact_without_master"), d, shown,
					  tr("Contact without its coil or main function"), self);
		} else if (data.m_slave_type == ElementData::PLCSlave) {
			Element *card = linked.first();
			const int row = plcRow(card, e);
			const auto ios = card->elementData().plcMasterData().ios;
			if (row < 0 || row >= ios.count()) {
				if (!ios.isEmpty())
					c.add(Severity::Warning, QStringLiteral("plc_channel_address"), d, shown,
						  tr("PLC channel linked to %1 but to none of its channels").arg(labelOf(card)),
						  self, QStringLiteral("004027"));
			} else if (ios.at(row).address.trimmed().isEmpty()) {
				c.add(Severity::Warning, QStringLiteral("plc_channel_address"), d, shown,
					  tr("PLC channel without an address (channel %1 of %2)")
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
				  tr("Terminal without a part number: the bill of materials leaves it out"), self);
		return;
	}

	if (data.m_type != ElementData::Master && data.m_type != ElementData::Simple)
		return;

		//Masters: a label, contacts, the used PLC channels with an address
	if (data.m_type == ElementData::Master) {
		if (label.isEmpty())
			c.add(Severity::Warning, QStringLiteral("unlabelled_device"), d, e->name(),
				  tr("Device without a label: its contacts cannot point to it"), self);
		if (data.m_master_type == ElementData::Coil && linked.isEmpty() && sides == 0)
			c.add(Severity::Info, QStringLiteral("coil_without_contact"), d, shown,
				  tr("Coil without any contact"), self);
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
						  tr("Channel %1 of the PLC card is used but has no address").arg(row + 1), self);
		}
	}

		//The part, for the bill of materials (a label marks a device)
	if ((data.m_type == ElementData::Master || !label.isEmpty()) && hasNoPart(e))
		c.add(data.m_type == ElementData::Master ? Severity::Warning : Severity::Info,
			  QStringLiteral("device_without_part"), d, shown,
			  tr("Device without a part number: the bill of materials shows it without one"), self);
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
				  tr("Cable without a label"), items, QStringLiteral("017001"));
		if (cores.isEmpty()) {
			c.add(Severity::Warning, QStringLiteral("cable_without_target"), folio, shown,
				  tr("Cable without a target: none of its %n core(s) is on a wire", nullptr,
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
				  tr("Core(s) %1 lead to a wire that no longer exists").arg(gone.join(QStringLiteral(", "))),
				  items, QString(""));
		if (!unlabelled.isEmpty())
			c.add(Severity::Warning, QStringLiteral("cable_without_target"), folio, shown,
				  tr("Cable target without a label: core(s) %1 end at a symbol without a label")
					  .arg(unlabelled.join(QStringLiteral(", "))), items);
	}
}

void checkDuplicates(Collector &c, QETProject *project, const QSet<Diagram *> &panel_folios)
{
	const QVector<DuplicateLabels::Group> groups = DuplicateLabels::find(project);
	for (const DuplicateLabels::Group &group : groups) {
		QList<QUuid> items;
		QSet<int> entities;
		Diagram *first = nullptr;
		bool all_terminals = true;
		for (const DuplicateLabels::Occurrence &o : group.occurrences) {
			if (!o.item) continue;
			Diagram *d = DuplicateLabels::diagramOf(o.item);
			if (!d || panel_folios.contains(d)) continue;
			if (auto *e = qobject_cast<Element *>(o.item.data())) {
				if (isPanelDevice(e)) continue;
				if (e->elementData().m_type != ElementData::Terminal) all_terminals = false;
				items << e->uuid();
			} else if (auto *w = qobject_cast<Conductor *>(o.item.data())) {
				items << w->uuid();
			}
			entities << o.entity;
			if (!first) first = d;
		}
		if (entities.size() < 2) continue;
			//the terminals of one strip share its label (X1) on purpose:
			//they are told apart by their numbers
		if (group.kind == DuplicateLabels::Kind::Component && all_terminals) continue;
		const QString label = group.place.isEmpty() ? group.label : group.place + group.label;
		if (group.kind == DuplicateLabels::Kind::Component)
			c.add(Severity::Error, QStringLiteral("duplicate_label"), first, label,
				  tr("Label used by %n devices which are not linked", nullptr, int(entities.size())),
				  items);
		else
			c.add(Severity::Warning, QStringLiteral("duplicate_wire_number"), first, label,
				  tr("Wire number on %n separate potentials", nullptr, int(entities.size())),
				  items);
	}
}

void checkDeviceTypes(Collector &c, QETProject *project, const QSet<Diagram *> &panel_folios)
{
	const QVector<DeviceTypeCheck::Device> devices = DeviceTypeCheck::check(project);
	for (const DeviceTypeCheck::Device &device : devices) {
		if (!device.known() || device.symbols.isEmpty()) continue;
		Element *head = device.symbols.first();
		if (!head) continue;
		Diagram *d = head->diagram();
		if (panel_folios.contains(d)) continue;
		QList<QUuid> items;
		for (const auto &s : device.symbols)
			if (s) items << s->uuid();
		const QString label = device.label.isEmpty() ? device.designation : device.label;
		if (!device.missing.isEmpty()) {
			QStringList parts;
			const auto &type_parts = device.type.parts;
			for (int i : device.missing)
				if (i >= 0 && i < type_parts.count())
					parts << DeviceTypeCheck::partText(type_parts.at(i));
			c.add(Severity::Info, QStringLiteral("device_type"), d, label,
				  tr("Not drawn of device type %1: %2").arg(device.typeName, parts.join(QStringLiteral(", "))),
				  items, QStringLiteral("007002"));
		}
		if (device.beyond() > 0)
			c.add(Severity::Warning, QStringLiteral("device_type"), d, label,
				  tr("%n symbol(s) beyond device type %1", nullptr, device.beyond()).arg(device.typeName),
				  items);
		const int extra = DeviceTypeCheck::extraContacts(device);
		if (extra > 0)
			c.add(Severity::Warning, QStringLiteral("device_type"), d, label,
				  tr("%n contact(s) more than device type %1 has", nullptr, extra).arg(device.typeName),
				  items);
	}
}

} // namespace

QList<PlanCheck::CheckInfo> PlanCheck::checks()
{
	QList<CheckInfo> list;
	for (const CheckDef &c : kChecks)
		list << CheckInfo{QString::fromLatin1(c.key), QString::fromLatin1(c.eplan),
						  QCoreApplication::translate("PlanCheck", c.title)};
	return list;
}

QVector<PlanCheck::Finding> PlanCheck::run(QETProject *project, int folio)
{
	if (!project) return {};
		//links, potentials and duplicates reach across folios
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
			for (const QString &w : layout.warnings()) {
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
		const QString strip = it.key().isEmpty() ? tr("(no label)") : it.key();
		c.add(Severity::Warning, QStringLiteral("terminal_without_strip"),
			  it.value().first().first, strip,
			  tr("%n terminal(s) in no terminal strip: the terminal diagram leaves them out",
				 nullptr, int(it.value().count())), items);
	}

	checkCables(c, project, wires);
	checkDuplicates(c, project, panel_folios);

	if (auto *db = project->dataBase()) {
		for (const auto &mismatch : db->potentialTypeMismatches()) {
			Diagram *d = nullptr;
			for (const QUuid &u : mismatch.wires)
				if (wires.contains(u)) {d = wires.value(u).first; break;}
			c.add(Severity::Error, QStringLiteral("potential_types"), d, QString(),
				  tr("One potential with wires of different types: %1")
					  .arg(mismatch.types.join(QStringLiteral(" / "))), mismatch.wires);
		}
	}

	checkDeviceTypes(c, project, panel_folios);

	QVector<Finding> out;
	for (const Finding &f : std::as_const(c.m_findings))
		if (folio < 0 || f.folio == folio) out << f;
	std::stable_sort(out.begin(), out.end(), [](const Finding &a, const Finding &b) {
		if (a.severity != b.severity) return int(a.severity) < int(b.severity);
		const int fa = a.folio < 0 ? INT_MAX : a.folio, fb = b.folio < 0 ? INT_MAX : b.folio;
		if (fa != fb) return fa < fb;
		const int ca = checkOrder(a.check), cb = checkOrder(b.check);
		if (ca != cb) return ca < cb;
		return QString::localeAwareCompare(a.label, b.label) < 0;
	});
	return out;
}

QString PlanCheck::severityKey(Severity severity)
{
	switch (severity) {
		case Severity::Error:   return QStringLiteral("error");
		case Severity::Warning: return QStringLiteral("warning");
		default:                return QStringLiteral("info");
	}
}

QString PlanCheck::severityText(Severity severity)
{
	switch (severity) {
		case Severity::Error:   return tr("Error");
		case Severity::Warning: return tr("Warning");
		default:                return tr("Note");
	}
}

int PlanCheck::count(const QVector<Finding> &findings, Severity severity)
{
	return int(std::count_if(findings.cbegin(), findings.cend(),
		[severity](const Finding &f) {return f.severity == severity;}));
}

QVariantMap PlanCheck::toVariant(const Finding &finding, QETProject *project)
{
	QVariantMap m;
	m.insert(QStringLiteral("severity"), severityKey(finding.severity));
	m.insert(QStringLiteral("check"), finding.check);
	m.insert(QStringLiteral("eplan"), finding.eplan);
	m.insert(QStringLiteral("folio"), finding.folio);
	const QList<Diagram *> diagrams = project ? project->diagrams() : QList<Diagram *>();
	m.insert(QStringLiteral("folioTitle"),
			 finding.folio >= 0 && finding.folio < diagrams.count()
				 ? diagrams.at(finding.folio)->title() : QString());
	m.insert(QStringLiteral("label"), finding.label);
	m.insert(QStringLiteral("text"), finding.text);
	QStringList items;
	for (const QUuid &u : finding.items) items << u.toString(QUuid::WithoutBraces);
	m.insert(QStringLiteral("items"), items);
	return m;
}

QJsonObject PlanCheck::toJson(const QVector<Finding> &findings, QETProject *project)
{
	QJsonArray list;
	for (const Finding &f : findings)
		list.append(QJsonObject::fromVariantMap(toVariant(f, project)));
	QJsonObject summary;
	summary[QStringLiteral("errors")] = count(findings, Severity::Error);
	summary[QStringLiteral("warnings")] = count(findings, Severity::Warning);
	summary[QStringLiteral("infos")] = count(findings, Severity::Info);
	QJsonObject root;
	root[QStringLiteral("project")] = project ? project->title() : QString();
	root[QStringLiteral("ok")] = count(findings, Severity::Error) == 0;
	root[QStringLiteral("summary")] = summary;
	root[QStringLiteral("findings")] = list;
	return root;
}
