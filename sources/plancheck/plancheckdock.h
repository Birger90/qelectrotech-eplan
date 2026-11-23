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
#ifndef PLANCHECKDOCK_H
#define PLANCHECKDOCK_H

#include "plancheck.h"

#include <QDockWidget>
#include <QPointer>

class QComboBox;
class QLabel;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;
class QETProject;

/**
	@brief The PlanCheckDock class
	"Check plan" as a side panel: every finding of PlanCheck::run() as a
	list of severity, folio, label and text, like EPLAN's message
	management. A click opens the folio and selects what the finding is
	about. While the panel is open the plan is checked again a moment
	after each change, as long as a run is quick; on a large project the
	"Check again" button does it.
*/
class PlanCheckDock : public QDockWidget
{
	Q_OBJECT
	public:
		explicit PlanCheckDock(QWidget *parent = nullptr);
		void setProject(QETProject *project);
		QETProject *project() const {return m_project;}

	public slots:
		void refresh();

	private:
		void fill();
		void showRow(QTreeWidgetItem *row);
		void changed();

		QPointer<QETProject> m_project;
		QVector<PlanCheck::Finding> m_findings;
		QTreeWidget *m_tree = nullptr;
		QLabel *m_summary = nullptr;
		QComboBox *m_filter = nullptr;
		QTimer *m_timer = nullptr;
		qint64 m_last_run_ms = 0;
};

#endif // PLANCHECKDOCK_H
