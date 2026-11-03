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
#ifndef EPLANIMPORTDIALOG_H
#define EPLANIMPORTDIALOG_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

/**
	@brief Dialog of the "Part import" menu: imports manufacturer EPLAN data
	(Data Portal DXF package, single DXF, .edz part) into an element
	collection, sorted into manufacturer / category / type folders.
*/
class EplanImportDialog : public QDialog
{
	Q_OBJECT

	public:
		enum class Source { Package, Folder, Dxf, Edz };

		explicit EplanImportDialog(Source source, QWidget *parent = nullptr);

		/// asks for the file first; false when the user cancels
		bool chooseSource();
		/// the elements written by the last import
		QStringList writtenPaths() const { return m_written; }

	private:
		void sourceChanged();
		void chooseEdz();
		void readEdz(const QString &path);
		void updateButtons();
		void runImport();
		QString targetDir();

		Source m_kind;
		QLineEdit *m_source = nullptr;
		QLineEdit *m_manufacturer = nullptr;
		QLineEdit *m_category = nullptr;
		QLineEdit *m_type = nullptr;
		QLineEdit *m_edz = nullptr;
		QPushButton *m_edz_button = nullptr;
		QComboBox *m_target = nullptr;
		QCheckBox *m_terms = nullptr;
		QLabel *m_missing = nullptr;   ///< what still has to be filled in
		QPushButton *m_import = nullptr;
		QString m_other_dir;
		QStringList m_written;
};

#endif // EPLANIMPORTDIALOG_H
