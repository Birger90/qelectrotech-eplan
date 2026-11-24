/*
	Copyright 2006-2025 The QElectroTech Team
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
#ifndef REVISIONUI_H
#define REVISIONUI_H

class QETDiagramEditor;
class QMenu;

/**
	Project > Revision: close a revision, show its marks, compare two
	states. Everything else happens by itself (ProjectRevisions).
*/
namespace RevisionUi {
	/// Add the submenu to @p project_menu of @p editor
	void addMenu(QETDiagramEditor *editor, QMenu *project_menu);
}

#endif // REVISIONUI_H
