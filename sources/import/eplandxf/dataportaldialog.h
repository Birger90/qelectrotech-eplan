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
#ifndef DATAPORTALDIALOG_H
#define DATAPORTALDIALOG_H

#include "dataportalclient.h"

#include <QDialog>
#include <QStringList>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;

/**
	@brief Dialog of the "Part import" menu that searches parts in the EPLAN
	Data Portal and imports their DXF packages into the user collection,
	sorted into manufacturer / category / type folders like a package
	imported from a file.

	The token is the user's own Personal Access Token; it is kept in the
	settings only when "Remember the token" is checked.
*/
class DataPortalDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit DataPortalDialog(QWidget *parent = nullptr);
		~DataPortalDialog() override;

		/// the elements written while the dialog was open
		QStringList writtenPaths() const { return m_written; }

	private:
		void runSearch();
		void showParts(const QVector<DataPortalPart> &parts, int total);
		void runImport();
		void importNext();
		void importPackage(const QByteArray &data, const QString &file_name);
		void failed(const QString &message);
		void finishImport();
		void storeToken();
		void updateButtons();
		void setBusy(bool busy, const QString &text = QString());

		DataPortalClient *m_client = nullptr;
		QLineEdit *m_token = nullptr;
		QCheckBox *m_remember = nullptr;
		QLineEdit *m_search = nullptr;
		QLineEdit *m_manufacturer = nullptr;
		QPushButton *m_search_button = nullptr;
		QTreeWidget *m_results = nullptr;
		QLabel *m_count = nullptr;
		QLineEdit *m_category = nullptr;
		QCheckBox *m_terms = nullptr;
		QLabel *m_status = nullptr;
		QPushButton *m_import = nullptr;
		QVector<DataPortalPart> m_parts;   ///< parts listed in m_results
		QVector<DataPortalPart> m_queue;   ///< parts still to import
		DataPortalPart m_current;          ///< part being downloaded
		QStringList m_report;              ///< one line per imported part
		QStringList m_written;
		bool m_busy = false;
};

#endif // DATAPORTALDIALOG_H
