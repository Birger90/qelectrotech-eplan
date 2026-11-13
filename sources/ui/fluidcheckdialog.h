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
#ifndef FLUIDCHECKDIALOG_H
#define FLUIDCHECKDIALOG_H

#include <QDialog>
#include <QHash>
#include <QPointer>

class Element;
class QLabel;
class QETProject;
class QTreeWidget;
class QTreeWidgetItem;

/**
	@brief The FluidCheckDialog class
	Lists what is unfinished in the fluid power parts of a project, as
	FluidSIM checks a drawing: a pneumatic or hydraulic port with no
	line and no silencer or plug on it, a solenoid valve whose solenoids
	are not all linked to a coil, a valve coil linked to no valve.
	Clicking a row selects the element on its folio.
*/
class FluidCheckDialog : public QDialog
{
	Q_OBJECT
	public:
		explicit FluidCheckDialog(QETProject *project, QWidget *parent = nullptr);

	public slots:
		void refresh();
		void showElement(QTreeWidgetItem *row);

	private:
		QPointer<QETProject> m_project;
		QTreeWidget *m_tree;
		QLabel *m_summary;
		QHash<QTreeWidgetItem *, QPointer<Element>> m_elements;
};

#endif
