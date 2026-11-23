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
#ifndef PURCHASEEXPORT_H
#define PURCHASEEXPORT_H

#include <QCoreApplication>

class QETProject;
class QWidget;

/**
	@brief The menu action "Export purchase list": one file dialog, then
	everything else by itself -- the article data looked up (EDZ
	packages, Data Portal), the workbook and the ERP file written, a
	short report with buttons to open the file and the price list.
*/
class PurchaseExport
{
	Q_DECLARE_TR_FUNCTIONS(PurchaseExport)

	public:
		static void run(QETProject *project, QWidget *parent);
};

#endif // PURCHASEEXPORT_H
