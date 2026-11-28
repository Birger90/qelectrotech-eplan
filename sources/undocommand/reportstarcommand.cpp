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
#include "reportstarcommand.h"

#include "linkelementcommand.h"
#include "../QPropertyUndoCommand/qpropertyundocommand.h"
#include "../diagram.h"
#include "../qetgraphicsitem/reportelement.h"
#include "../qetproject.h"

#include <QCoreApplication>

QUndoCommand *ReportStar::command(ReportElement *report, bool star, QUndoCommand *parent)
{
	if (!report || !report->diagram() || !report->diagram()->project())
		return nullptr;
	QETProject *project = report->diagram()->project();
	const QList<ReportElement *> candidates = star
			? project->starReportCandidates(report) : QList<ReportElement *>();
	if (report->isReportStar() == star && candidates.isEmpty())
		return nullptr;

	auto *macro = new QUndoCommand(star
			? QCoreApplication::translate("ReportStar", "Star folio report")
			: QCoreApplication::translate("ReportStar", "Folio report with one partner"),
			parent);
	if (report->isReportStar() != star)
		new QPropertyUndoCommand(report, "reportStar", report->isReportStar(), star, macro);
	if (star && !candidates.isEmpty()) {
			//Linked while redo() runs: the property step before it has
			//made the report a star by then, which isLinkable() would not
			//see yet
		QList<Element *> partners = report->linkedElements();
		for (ReportElement *other : candidates)
			partners << other;
		auto *link = new LinkElementCommand(report, macro);
		link->setLinkedAfter(partners);
	}
	return macro;
}
