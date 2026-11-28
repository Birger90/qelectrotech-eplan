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
#ifndef WIRENUMBERPATTERNDIALOG_H
#define WIRENUMBERPATTERNDIALOG_H

#include "../autoNum/wirenumberpattern.h"

#include <QDialog>
#include <QPointer>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTreeWidget;
class QETDiagramEditor;
class QETProject;

/**
	@brief The WireNumberPatternDialog class
	Project > Wire numbers by pattern: the pattern of the project
	(WireNumberPattern), a preview of what it changes, and numbering now.
*/
class WireNumberPatternDialog : public QDialog
{
	Q_OBJECT
	public:
		explicit WireNumberPatternDialog(QETProject *project, QWidget *parent = nullptr);
			/// The entry in the project menu of @p editor
		static void addMenu(QETDiagramEditor *editor, QMenu *project_menu);

	private:
		WireNumberPattern::Settings chosen() const;
		void updatePreview();
		void save();
		void numberNow();

		QPointer<QETProject> m_project;
		QCheckBox *m_active = nullptr;
		QLineEdit *m_pattern = nullptr;
		QComboBox *m_scope = nullptr, *m_order = nullptr, *m_mode = nullptr;
		QSpinBox *m_reserve = nullptr, *m_start = nullptr;
		QLabel *m_problem = nullptr;
		QTreeWidget *m_preview = nullptr;
		QPushButton *m_number = nullptr;
};

#endif // WIRENUMBERPATTERNDIALOG_H
