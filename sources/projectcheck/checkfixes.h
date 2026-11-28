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
#ifndef CHECKFIXES_H
#define CHECKFIXES_H

#include "../projectcheck.h"

#include <QString>

class QETProject;

/**
	@brief The CheckFixes namespace
	The one-click corrections of the check panel: for a finding whose
	right answer is plain from the project, what fixes it, as one undo
	step. The checks themselves change nothing (ProjectCheck, PlanCheck);
	this is where a finding is put right, from the panel, the script API
	(qet.checkFixes(), qet.applyCheckFix()) or nowhere at all.

	The corrections:
	- a folio report without its partner: link it to the one free report
	  of its name on another folio, to the star of its name, make the
	  report of its name that already has a partner a star, or make it a
	  star itself when several free reports of its name wait for it
	  (@sa Element::isReportStar());
	- a contact without its coil: link it to the one device of its label;
	- a device without a label which follows an element numbering: give
	  it the first free number;
	- a wire number given twice: a new number for the other wires (the
	  wire number pattern of the project, @sa WireNumberPattern).
*/
namespace CheckFixes
{
		/// What fixing @p finding does, translated; empty when there is no fix
	QString label(QETProject *project, const ProjectCheck::Finding &finding);
		/// Fix @p finding, one undo step on the project's stack.
		/// @return what was done, empty when it could not be fixed
	QString apply(QETProject *project, const ProjectCheck::Finding &finding);
}

#endif // CHECKFIXES_H
