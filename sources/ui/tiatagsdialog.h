/*
	Copyright 2006-2026 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech. If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef TIATAGSDIALOG_H
#define TIATAGSDIALOG_H

#include "../plc/tiaplctags.h"

#include <QDialog>
#include <QPointer>

class QCheckBox;
class QLabel;
class QPushButton;
class QETProject;
class QTreeWidget;
class QTreeWidgetItem;

/**
	@brief The TiaTagsDialog class
	PLC tags of the project and Siemens TIA Portal in one window: the
	channels of the PLC modules as they are exported (address, name, data
	type, comment), "Export for TIA..." writes them as tag table, "Compare
	with TIA table..." reads one exported from TIA Portal and marks per
	channel what differs, "Take over names and comments" writes the TIA
	names and comments into the IO tables (one undo step). Clicking a row
	shows the channel on its folio.
*/
class TiaTagsDialog : public QDialog
{
	Q_OBJECT
	public:
		explicit TiaTagsDialog(QETProject *project, QWidget *parent = nullptr);

	private:
		void refresh();
		void fill();
		void exportTags();
		void compareWith();
		void takeOver();
		void saveReport();
		void showRow(QTreeWidgetItem *row);

		QPointer<QETProject> m_project;
		QLabel *m_summary;
		QTreeWidget *m_tree;
		QCheckBox *m_only_differences;
		QPushButton *m_take_over;
		QPushButton *m_report;
		QString m_tia_file;
		TiaPlcTags::Comparison m_cmp;
		QStringList m_notes;
		bool m_compared = false;
};

#endif // TIATAGSDIALOG_H
