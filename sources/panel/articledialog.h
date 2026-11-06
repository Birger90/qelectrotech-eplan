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
#ifndef ARTICLEDIALOG_H
#define ARTICLEDIALOG_H

#include "../import/eplandxf/dataportalclient.h"

#include <QDialog>

class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QTreeWidget;

/**
	@brief The ArticleDialog class
	EPLAN's part selection: a device of the schematic gets its part
	(manufacturer and order number) from the EPLAN Data Portal. Everything
	the cabinet layout needs later (size, panel layout drawing, drilling
	pattern, mounting) is looked up by that order number.
*/
class ArticleDialog : public QDialog
{
	Q_OBJECT
	public:
		ArticleDialog(QWidget *parent, const QString &what, const QString &search);
		DataPortalPart part() const {return m_part;}

	private:
		void runSearch();
		void showParts(const QVector<DataPortalPart> &parts, int total, int offset);
		void take();

		DataPortalClient *m_client = nullptr;
		QLineEdit *m_search = nullptr;
		QLineEdit *m_manufacturer = nullptr;
		QCheckBox *m_exact = nullptr;
		QTreeWidget *m_list = nullptr;
		QLabel *m_status = nullptr;
		QPushButton *m_take = nullptr;
		QVector<DataPortalPart> m_parts;
		DataPortalPart m_part;
};

#endif
