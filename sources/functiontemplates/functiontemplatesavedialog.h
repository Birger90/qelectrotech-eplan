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
#ifndef FUNCTIONTEMPLATESAVEDIALOG_H
#define FUNCTIONTEMPLATESAVEDIALOG_H

#include "functiontemplate.h"

#include <QDialog>

class Diagram;
class QComboBox;
class QGraphicsItem;
class QLineEdit;
class QPlainTextEdit;
class QTableWidget;

/**
	@brief "Save as function template…": a name, a category, and what
	is asked when it is placed. Every placeholder {NAME} the selection
	holds is listed as a value to ask for, with a guess of what it is;
	a table can be pasted (Excel or CSV, the first column picks the
	row). Labels written as they are, K1, need nothing: they are counted
	on to the next free one wherever the template is placed.
*/
class FunctionTemplateSaveDialog : public QDialog
{
	Q_OBJECT

	public:
		FunctionTemplateSaveDialog(Diagram *folio, const QList<QGraphicsItem *> &items,
								   QWidget *parent = nullptr);
		QString savedPath() const {return m_saved;}

			/// A pasted table: tab, semicolon or comma separated, the
			/// header first, the first column the key
		static FunctionTemplate::Table parseTable(const QString &text, QString *error = nullptr);

	public slots:
		void accept() override;

	private:
		Diagram *m_folio;
		QList<QGraphicsItem *> m_items;
		QLineEdit *m_name;
		QComboBox *m_category;
		QPlainTextEdit *m_description;
		QTableWidget *m_parameters;
		QPlainTextEdit *m_table;
		QString m_saved;
};

#endif // FUNCTIONTEMPLATESAVEDIALOG_H
