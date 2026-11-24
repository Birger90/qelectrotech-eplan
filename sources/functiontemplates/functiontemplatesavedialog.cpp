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
#include "functiontemplatesavedialog.h"

#include "../diagram.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QVBoxLayout>

namespace
{
	enum Column {Name, Label, Kind, Default, Unit, Count};

	const QStringList &kinds()
	{
		static const QStringList list{QStringLiteral("text"), QStringLiteral("label"),
									  QStringLiteral("terminal"), QStringLiteral("number")};
		return list;
	}
}

FunctionTemplateSaveDialog::FunctionTemplateSaveDialog(Diagram *folio,
													   const QList<QGraphicsItem *> &items,
													   QWidget *parent) :
	QDialog(parent),
	m_folio(folio),
	m_items(items)
{
	setWindowTitle(tr("Save as function template"));
	resize(640, 560);
	auto *layout = new QVBoxLayout(this);
	auto *form = new QFormLayout();
	m_name = new QLineEdit(this);
	m_name->setPlaceholderText(tr("Motor feeder DOL"));
	form->addRow(tr("Name:"), m_name);
	m_category = new QComboBox(this);
	m_category->setEditable(true);
	QStringList categories;
	for (const FunctionTemplate &tpl : FunctionTemplate::available())
		if (!tpl.category().isEmpty() && !categories.contains(tpl.category()))
			categories << tpl.category();
	categories.sort(Qt::CaseInsensitive);
	m_category->addItems(categories);
	m_category->setCurrentText(QString());
	form->addRow(tr("Category:"), m_category);
	m_description = new QPlainTextEdit(this);
	m_description->setMaximumHeight(60);
	form->addRow(tr("Description:"), m_description);
	layout->addLayout(form);

	auto *hint = new QLabel(tr("Write {NAME} in a label, a text or an article to ask for it when "
							   "the template is placed: {BMK}, {P_kW}, {ADR}. {N+1} is the value "
							   "counted on by one. Labels without braces are counted on to the "
							   "next free one by themselves."), this);
	hint->setWordWrap(true);
	layout->addWidget(hint);

	m_parameters = new QTableWidget(0, Count, this);
	m_parameters->setHorizontalHeaderLabels({tr("Placeholder"), tr("Asked as"), tr("Kind"),
											 tr("Default"), tr("Unit")});
	m_parameters->horizontalHeader()->setSectionResizeMode(Label, QHeaderView::Stretch);
	m_parameters->verticalHeader()->hide();
	for (const FunctionTemplate::Parameter &p : FunctionTemplate::parametersFor(folio, items)) {
		const int row = m_parameters->rowCount();
		m_parameters->insertRow(row);
		auto *name = new QTableWidgetItem(p.name);
		name->setFlags(name->flags() & ~Qt::ItemIsEditable);
		m_parameters->setItem(row, Name, name);
		m_parameters->setItem(row, Label, new QTableWidgetItem(p.label));
		auto *kind = new QComboBox(m_parameters);
		kind->addItems({tr("Text"), tr("Device tag (next free one)"),
						tr("Terminal number (next free one)"), tr("Number")});
		kind->setCurrentIndex(qMax(0, kinds().indexOf(p.kind)));
		m_parameters->setCellWidget(row, Kind, kind);
		m_parameters->setItem(row, Default, new QTableWidgetItem(p.defaultValue));
		m_parameters->setItem(row, Unit, new QTableWidgetItem(p.unit));
	}
	if (!m_parameters->rowCount()) {
		auto *none = new QLabel(tr("The selection holds no placeholder: it is placed as it is, "
								   "its labels counted on."), this);
		none->setWordWrap(true);
		layout->addWidget(none);
		m_parameters->hide();
	}
	layout->addWidget(m_parameters, 1);

	auto *table_hint = new QLabel(tr("Choice table (optional, pasted from Excel): the first line "
									 "names the columns, the first column is the placeholder which "
									 "picks the row, the others become placeholders too."), this);
	table_hint->setWordWrap(true);
	layout->addWidget(table_hint);
	m_table = new QPlainTextEdit(this);
	m_table->setPlaceholderText(QStringLiteral("P_kW;QA_ART;QA_I;KM_ART\n4;3RV2011-1JA10;7...10 A;3RT2016-1BB41"));
	m_table->setMaximumHeight(90);
	layout->addWidget(m_table);

	layout->addWidget(new QLabel(tr("Saved in: %1")
								 .arg(QDir::toNativeSeparators(FunctionTemplate::userDirectory())), this));
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &FunctionTemplateSaveDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttons);
}

FunctionTemplate::Table FunctionTemplateSaveDialog::parseTable(const QString &text, QString *error)
{
	FunctionTemplate::Table table;
	QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
	for (QString &line : lines) line = line.trimmed();
	lines.removeAll(QString());
	if (lines.isEmpty()) return table;
	const QString header = lines.takeFirst();
	QChar separator = QLatin1Char('\t');
	if (!header.contains(separator))
		separator = header.contains(QLatin1Char(';')) ? QLatin1Char(';') : QLatin1Char(',');
	for (const QString &column : header.split(separator))
		table.columns << column.trimmed().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
	if (table.columns.isEmpty() || table.columns.first().isEmpty()) {
		if (error) *error = tr("The first line of the table has to name its columns.");
		return FunctionTemplate::Table();
	}
	table.key = table.columns.first();
	for (const QString &line : std::as_const(lines)) {
		const QStringList cells = line.split(separator);
		QMap<QString, QString> row;
		for (int i = 0 ; i < table.columns.size() && i < cells.size() ; ++i)
			if (!table.columns.at(i).isEmpty()) row.insert(table.columns.at(i), cells.at(i).trimmed());
		if (!row.value(table.key).isEmpty()) table.rows << row;
	}
	return table;
}

void FunctionTemplateSaveDialog::accept()
{
	FunctionTemplate::SaveOptions options;
	options.name = m_name->text().trimmed();
	options.category = m_category->currentText().trimmed();
	options.description = m_description->toPlainText().trimmed();
	for (int row = 0 ; row < m_parameters->rowCount() ; ++row) {
		FunctionTemplate::Parameter p;
		p.name = m_parameters->item(row, Name)->text();
		p.label = m_parameters->item(row, Label) ? m_parameters->item(row, Label)->text().trimmed() : QString();
		if (auto *kind = qobject_cast<QComboBox *>(m_parameters->cellWidget(row, Kind)))
			p.kind = kinds().value(kind->currentIndex(), QStringLiteral("text"));
		p.defaultValue = m_parameters->item(row, Default) ? m_parameters->item(row, Default)->text() : QString();
		p.unit = m_parameters->item(row, Unit) ? m_parameters->item(row, Unit)->text().trimmed() : QString();
			//A terminal number counts on in the strip named before it
		if (p.kind == QLatin1String("terminal") && !options.parameters.isEmpty())
			p.of = options.parameters.last().name;
		options.parameters << p;
	}
	QString error;
	const FunctionTemplate::Table table = parseTable(m_table->toPlainText(), &error);
	if (!error.isEmpty()) {
		QMessageBox::warning(this, windowTitle(), error);
		return;
	}
	if (!table.key.isEmpty()) options.tables << table;
	if (options.name.isEmpty()) {
		QMessageBox::warning(this, windowTitle(), tr("Give the template a name."));
		m_name->setFocus();
		return;
	}
	m_saved = FunctionTemplate::save(m_folio, m_items, options, &error);
	if (m_saved.isEmpty()) {
		QMessageBox::critical(this, windowTitle(), error);
		return;
	}
	QDialog::accept();
}
