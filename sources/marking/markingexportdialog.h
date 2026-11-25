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
#ifndef MARKINGEXPORTDIALOG_H
#define MARKINGEXPORTDIALOG_H

#include "markingexport.h"
#include "mprintpro.h"

#include <QDialog>

class QETProject;
class QCheckBox;
class QComboBox;
class QLabel;
class QSpinBox;
class QGroupBox;
class QLineEdit;
class QPushButton;

/**
	@brief The dialog of Project > Export markers...: one page, the choices
	remembered from the last export, the number of markers shown as they
	change, then the file.
*/
class MarkingExportDialog : public QDialog
{
	Q_OBJECT
	public:
		explicit MarkingExportDialog(QETProject *project, QWidget *parent = nullptr);
		MarkingExport::Options options() const;

	private:
		void updateCount();
		void exportFile();
		void remember() const;
		void fillMaterials();
		MPrintPro::Settings mprintSettings() const;
		void openInMPrintPro();

		QETProject *m_project = nullptr;
		QComboBox *m_format = nullptr;
		QCheckBox *m_wires = nullptr;
		QCheckBox *m_terminals = nullptr;
		QCheckBox *m_devices = nullptr;
		QCheckBox *m_cables = nullptr;
		QSpinBox *m_folio_from = nullptr;
		QSpinBox *m_folio_to = nullptr;
		QComboBox *m_plant = nullptr;
		QComboBox *m_location = nullptr;
		QCheckBox *m_full = nullptr;
		QCheckBox *m_both_ends = nullptr;
		QLabel *m_count = nullptr;
		QLabel *m_hint = nullptr;
			//Weidmüller printer, shown for M-Print PRO only
		QGroupBox *m_weidmueller = nullptr;
		QComboBox *m_printer = nullptr;
		QComboBox *m_material[MPrintPro::SlotCount] = {};
		QLineEdit *m_exe = nullptr;
		QCheckBox *m_print_now = nullptr;
		QPushButton *m_open = nullptr;
};

#endif // MARKINGEXPORTDIALOG_H
