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
#ifndef REPORTSTARCOMMAND_H
#define REPORTSTARCOMMAND_H

class QUndoCommand;
class ReportElement;

namespace ReportStar
{
	/**
		@brief command
		One undo step that makes @p report a star -- a folio report linked
		to every free report of the other kind with its name on another
		folio (QETProject::starReportCandidates()) -- or, with
		@p star false, a report linked to its first partner only again.
		@return the command, nullptr when there is nothing to change
	*/
	QUndoCommand *command(ReportElement *report, bool star, QUndoCommand *parent = nullptr);
}

#endif // REPORTSTARCOMMAND_H
