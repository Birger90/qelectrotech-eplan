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
#ifndef NETWORKADDRESSDIALOG_H
#define NETWORKADDRESSDIALOG_H

#include <QDialog>
#include <QList>
#include <QPointer>

class Element;
class QETDiagramEditor;
class QETProject;
class QLineEdit;
class QMenu;
class QTableWidget;

/**
	@brief The NetworkAddressDialog class
	Project > Network addresses: the address table (NetworkAddresses) to
	read and edit, the IP networks, giving IP addresses and PROFINET
	names, CSV export.
*/
class NetworkAddressDialog : public QDialog
{
	Q_OBJECT
	public:
		explicit NetworkAddressDialog(QETProject *project, QWidget *parent = nullptr);
		~NetworkAddressDialog() override;
		static void addMenu(QETDiagramEditor *editor, QMenu *project_menu);

	private:
		void fill();
		void saveNetworks();
		void cellEdited(int row, int column);

		QPointer<QETProject> m_project;
		QLineEdit *m_networks = nullptr;
		QTableWidget *m_table = nullptr;
		QList<QPointer<Element>> m_rows;
};

#endif // NETWORKADDRESSDIALOG_H
