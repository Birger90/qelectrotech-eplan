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
#ifndef CABLEOVERVIEWDIALOG_H
#define CABLEOVERVIEWDIALOG_H

#include <QDialog>
#include <QPointer>

class Cable;
class QETProject;
class QLineEdit;
class QPushButton;
class QSortFilterProxyModel;
class QStandardItemModel;
class QTableView;

/**
	@brief The list of every cable of a project, one line each, where the
	type of a cable is chosen -- from the user's own cable type file or
	from a manufacturer catalogue -- without hunting the cable down on its
	folio first.

	"Choose type..." (or a double click) opens the very window a cable's
	properties open to change its type, so the cores are reworked the
	same way and the change is one undo step.
*/
class CableOverviewDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit CableOverviewDialog(QETProject *project, QWidget *parent = nullptr);

	private:
		void fill();
		void chooseType();
		void selectionChanged();
		Cable *currentCable() const;

		QPointer<QETProject> m_project;
		QStandardItemModel *m_model = nullptr;
		QSortFilterProxyModel *m_proxy = nullptr;
		QTableView *m_table = nullptr;
		QLineEdit *m_search = nullptr;
		QPushButton *m_choose = nullptr;
		QList<QPointer<Cable>> m_cables;
};

#endif // CABLEOVERVIEWDIALOG_H
