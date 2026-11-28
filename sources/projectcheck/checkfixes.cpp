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
#include "checkfixes.h"

#include "../autoNum/elementautonumschemecommand.h"
#include "../autoNum/wirenumberpattern.h"
#include "../qetgraphicsitem/conductor.h"
#include "../diagram.h"
#include "../duplicatelabels.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/reportelement.h"
#include "../qetinformation.h"
#include "../qetproject.h"
#include "../undocommand/linkelementcommand.h"
#include "../undocommand/reportstarcommand.h"
#include "../undocommand/pairrepresentation.h"
#include "../undocommand/deleteqgraphicsitemcommand.h"
#include "../diagramcontent.h"

#include <QCoreApplication>
#include <QUndoStack>
#include <functional>

namespace {

	/// A fix: what it says, and the step that does it
struct Plan {
	QString label;
	std::function<QUndoCommand *()> make;
};

QString folioName(Diagram *diagram)
{
	if (!diagram) return QString();
	const QString folio = diagram->border_and_titleblock.finalfolio().trimmed();
	return folio.isEmpty() ? QString::number(diagram->folioIndex() + 1) : folio;
}

	/// the symbol a finding is about: the first of its places
Element *elementOf(const ProjectCheck::Finding &finding)
{
	for (const ProjectCheck::Place &place : finding.places)
		if (auto *e = qobject_cast<Element *>(place.item.data()))
			return e;
	return nullptr;
}

QList<ReportElement *> reportsOf(QETProject *project)
{
	QList<ReportElement *> out;
	for (Diagram *d : project->diagrams())
		for (Element *e : d->elements())
			if (auto *r = qobject_cast<ReportElement *>(e))
				out << r;
	return out;
}

Plan reportPlan(QETProject *project, ReportElement *report)
{
	Plan plan;
	if (!report->isFree() && !report->isReportStar()) return plan;
	const QString name = QETProject::starReportName(report);
	if (name.isEmpty()) return plan;
	const int inverse = report->linkType() == Element::NextReport
			? Element::PreviousReport : Element::NextReport;

		//A star of its name on another folio takes it
	ReportElement *linked_partner = nullptr;
	for (ReportElement *other : reportsOf(project)) {
		if (other->linkType() != inverse || other->diagram() == report->diagram()
				|| QETProject::starReportName(other) != name)
			continue;
		if (other->isReportStar()) {
			plan.label = QCoreApplication::translate("CheckFixes", "Link to the star %1 on folio %2").arg(name, folioName(other->diagram()));
			plan.make = [report, other]() -> QUndoCommand * {
				auto *cmd = new LinkElementCommand(other);
				cmd->setLink(other->linkedElements() << report);
				return cmd;
			};
			return plan;
		}
		if (!other->isFree() && !linked_partner) linked_partner = other;
	}

	const QList<ReportElement *> free = project->starReportCandidates(report);
		//The report of its name that has its partner already: one source
		//going out to more than one folio, a star
	if (linked_partner) {
		plan.label = QCoreApplication::translate("CheckFixes", "Make %1 on folio %2 a star (links %n more report(s))", nullptr,
						int(project->starReportCandidates(linked_partner).size()))
				.arg(name, folioName(linked_partner->diagram()));
		plan.make = [linked_partner]() { return ReportStar::command(linked_partner, true); };
		return plan;
	}
	if (free.size() == 1) {
		ReportElement *other = free.first();
		plan.label = QCoreApplication::translate("CheckFixes", "Link to %1 on folio %2").arg(name, folioName(other->diagram()));
		plan.make = [report, other]() -> QUndoCommand * {
			auto *cmd = new LinkElementCommand(report);
			cmd->setLink(other);
			return cmd;
		};
		return plan;
	}
	if (free.size() > 1 && report->linkType() == Element::NextReport) {
		plan.label = QCoreApplication::translate("CheckFixes", "Make it a star: link the %n report(s) %1 on the other folios", nullptr,
						int(free.size())).arg(name);
		plan.make = [report]() { return ReportStar::command(report, true); };
	}
	return plan;
}

Plan contactPlan(QETProject *project, Element *contact)
{
	Plan plan;
	if (!contact->isFree()) return plan;
	const QString label = DuplicateLabels::displayedLabel(contact).trimmed();
	if (label.isEmpty()) return plan;
	Element *master = nullptr;
	int count = 0;
	for (Diagram *d : project->diagrams())
		for (Element *e : d->elements())
			if (e->linkType() == Element::Master
					&& e->elementData().m_master_type != ElementData::PLC
					&& DuplicateLabels::displayedLabel(e).trimmed() == label) {
				master = e;
				++count;
			}
	if (count != 1 || !LinkElementCommand::isLinkable(contact, master)) return plan;
	plan.label = QCoreApplication::translate("CheckFixes", "Link to %1 on folio %2").arg(label, folioName(master->diagram()));
	plan.make = [contact, master]() -> QUndoCommand * {
		auto *cmd = new LinkElementCommand(contact);
		cmd->setLink(master);
		return cmd;
	};
	return plan;
}

Plan labelPlan(QETProject *project, Element *device)
{
	Plan plan;
	const QString title = project->elementAutoNumTitle(
			QUuid(device->elementInformations().value(QETInformation::ELMT_FORMULA_ID).toString()));
	if (title.isEmpty()) return plan;
	const QList<int> free = ElementAutoNumSchemeCommand::freeNumbers(project, title, device, 50);
	if (free.isEmpty()) return plan;
	const int number = free.first();
	const QString label = ElementAutoNumSchemeCommand::labelForNumber(project, title, device, number);
	plan.label = label.isEmpty() ? QCoreApplication::translate("CheckFixes", "Give it number %1 of %2").arg(number).arg(title)
								 : QCoreApplication::translate("CheckFixes", "Give it the label %1").arg(label);
	plan.make = [project, device, number]() -> QUndoCommand * {
		return ElementAutoNumSchemeCommand::assignNumber(project, device, number);
	};
	return plan;
}

	/// A wire number given twice: the first potential keeps it, the others
	/// take a new one by the project's wire number pattern
Plan duplicateWirePlan(QETProject *project, const ProjectCheck::Finding &finding)
{
	Plan plan;
	QList<Conductor *> wires;
	for (const ProjectCheck::Place &place : finding.places)
		if (auto *w = qobject_cast<Conductor *>(place.item.data()))
			wires << w;
	if (wires.size() < 2) return plan;
	QList<Conductor *> others;
	const QSet<Conductor *> first = wires.first()->relatedPotentialConductors(true) + QSet<Conductor *>{wires.first()};
	for (Conductor *w : std::as_const(wires))
		if (!first.contains(w) && !w->isFreezeLabel()) others << w;
	if (others.isEmpty()) return plan;
	WireNumberPattern::Settings s = WireNumberPattern::settings(project);
	s.mode = WireNumberPattern::Mode::Fixed;
	const WireNumberPattern::Plan numbers = WireNumberPattern::plan(project, s, others, true);
	if (numbers.changes.isEmpty()) return plan;
	QStringList news;
	for (const WireNumberPattern::Change &c : numbers.changes) news << c.new_text;
	plan.label = QCoreApplication::translate("CheckFixes", "New number for the other wire(s): %1")
			.arg(news.join(QStringLiteral(", ")));
	plan.make = [numbers]() { return WireNumberPattern::command(numbers); };
	return plan;
}

	/// A pair cross-reference whose contact is gone: delete it
Plan orphanRepresentationPlan(Element *rep)
{
	Plan plan;
	if (!rep || !rep->isPairRepresentation() || PairRepresentation::realOf(rep) || !rep->diagram())
		return plan;
	plan.label = QCoreApplication::translate("CheckFixes", "Delete the pair cross-reference");
	plan.make = [rep]() -> QUndoCommand * {
		DiagramContent content;
		content.m_elements << rep;
		return new DeleteQGraphicsItemCommand(rep->diagram(), content);
	};
	return plan;
}

Plan planOf(QETProject *project, const ProjectCheck::Finding &finding)
{
	if (!project || project->isReadOnly()) return Plan();
	Element *element = elementOf(finding);
	if (finding.rule == QLatin1String("report_without_partner")
			&& finding.eplan == QLatin1String("011002")) {
		if (auto *report = qobject_cast<ReportElement *>(element))
			return reportPlan(project, report);
	}
	if (finding.rule == QLatin1String("contact_without_master") && element)
		return contactPlan(project, element);
	if (finding.rule == QLatin1String("unlabelled_device") && element)
		return labelPlan(project, element);
	if (finding.rule == QLatin1String("pair_representation") && element)
		return orphanRepresentationPlan(element);
	if (finding.rule == QLatin1String("duplicate_wire_number"))
		return duplicateWirePlan(project, finding);
	return Plan();
}

}

QString CheckFixes::label(QETProject *project, const ProjectCheck::Finding &finding)
{
	return planOf(project, finding).label;
}

QString CheckFixes::apply(QETProject *project, const ProjectCheck::Finding &finding)
{
	const Plan plan = planOf(project, finding);
	if (plan.label.isEmpty() || !plan.make) return QString();
	QUndoCommand *cmd = plan.make();
	if (!cmd) return QString();
	if (cmd->text().isEmpty()) cmd->setText(plan.label);
	project->undoStack()->push(cmd);
	return plan.label;
}
