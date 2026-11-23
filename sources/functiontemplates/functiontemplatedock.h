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
#ifndef FUNCTIONTEMPLATEDOCK_H
#define FUNCTIONTEMPLATEDOCK_H

#include "functiontemplate.h"

#include <QDialog>
#include <QDockWidget>
#include <QMap>
#include <QPointer>

class Diagram;
class QETDiagramEditor;
class QComboBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;

/**
	@brief The few values a function template asks for, one field each:
	a label, a power chosen from its table, a PLC address. What the
	tables pick for the values is shown below, the article of the
	breaker for the power, so it is seen before it is placed.
*/
class FunctionTemplateForm : public QWidget
{
	Q_OBJECT

	public:
		explicit FunctionTemplateForm(QWidget *parent = nullptr);
		void setTemplate(const FunctionTemplate &tpl, Diagram *folio);
		QMap<QString, QString> values() const;

	signals:
		void changed();

	private:
		void refreshInfo();

		FunctionTemplate m_template;
		QPointer<Diagram> m_folio;
		QFormLayout *m_layout = nullptr;
		QLabel *m_info = nullptr;
		QMap<QString, QComboBox *> m_fields;
};

/**
	@brief Asks for the values of a function template dropped from the
	collections, as the side panel would.
*/
class FunctionTemplateValuesDialog : public QDialog
{
	Q_OBJECT

	public:
		FunctionTemplateValuesDialog(const FunctionTemplate &tpl, Diagram *folio,
									 QWidget *parent = nullptr);
		QMap<QString, QString> values() const;

	private:
		FunctionTemplateForm *m_form;
};

/**
	@brief The side panel "Function templates".
	Lists the templates of the user and the examples by category, shows
	the one chosen with its picture and its values, and places it: with
	the mouse on the current folio, or on a folio of its own.
*/
class FunctionTemplateDock : public QDockWidget
{
	Q_OBJECT

	public:
		explicit FunctionTemplateDock(QETDiagramEditor *editor);
		void reload();

	private:
		void fill();
		void select(QTreeWidgetItem *item);
		void refreshPreview();
		void insertHere();
		void insertOnNewFolio();
		void report(const FunctionTemplate::Placed &placed);
		Diagram *currentFolio() const;
		FunctionTemplate current() const;

		QETDiagramEditor *m_editor;
		QList<FunctionTemplate> m_templates;
		QLineEdit *m_search = nullptr;
		QTreeWidget *m_tree = nullptr;
		QLabel *m_preview = nullptr;
		QLabel *m_description = nullptr;
		FunctionTemplateForm *m_form = nullptr;
		QPushButton *m_insert = nullptr;
		QPushButton *m_new_folio = nullptr;
		QLabel *m_result = nullptr;
		QTimer *m_preview_timer = nullptr;
};

#endif // FUNCTIONTEMPLATEDOCK_H
