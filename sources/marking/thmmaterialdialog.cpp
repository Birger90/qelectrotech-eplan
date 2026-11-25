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
#include "thmmaterialdialog.h"

#include "../qet.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
const char *settings_group = "thm_print/material_filter";

QComboBox *choices(const QString &all, const QStringList &values, QWidget *parent)
{
	auto *combo = new QComboBox(parent);
	combo->addItem(all, QString());
	for (const QString &v : values) combo->addItem(v, v);
	return combo;
}

void select(QComboBox *combo, const QString &value)
{
	const int i = combo->findData(value);
	combo->setCurrentIndex(i >= 0 ? i : 0);
}
}

ThmMaterialDialog::ThmMaterialDialog(const QList<ThmMaterials::Entry> &entries,
									 const QString &current, QWidget *parent) :
	QDialog(parent),
	m_entries(entries)
{
	setWindowTitle(tr("Choose material"));
	setObjectName(QStringLiteral("thm_material_dialog"));
	QET::trackDialogGeometry(this);
	auto *layout = new QVBoxLayout(this);

	QSettings settings;
	settings.beginGroup(settings_group);
	auto *filters = new QFormLayout;
	layout->addLayout(filters);
	m_text = new QLineEdit(this);
	m_text->setObjectName(QStringLiteral("thm_material_text"));
	m_text->setPlaceholderText(tr("name or number, e.g. ESG 6/15 or 2619950000"));
	m_text->setClearButtonEnabled(true);
	filters->addRow(tr("Search:"), m_text);

	auto *row = new QHBoxLayout;
	m_category = choices(tr("All categories"), ThmMaterials::categories(m_entries), this);
	m_category->setObjectName(QStringLiteral("thm_material_category"));
	m_size = choices(tr("All sizes"), ThmMaterials::sizes(m_entries), this);
	m_size->setObjectName(QStringLiteral("thm_material_size"));
	m_colour = choices(tr("All colours"), ThmMaterials::colours(m_entries), this);
	m_colour->setObjectName(QStringLiteral("thm_material_colour"));
	m_sides = new QComboBox(this);
	m_sides->setObjectName(QStringLiteral("thm_material_sides"));
	m_sides->addItem(tr("One- and two-sided"), 0);
	m_sides->addItem(tr("One-sided"), 1);
	m_sides->addItem(tr("Two-sided"), 2);
	for (QComboBox *c : {m_category, m_size, m_colour, m_sides}) row->addWidget(c);
	filters->addRow(tr("Filter:"), row);
	select(m_category, settings.value("category").toString());
	select(m_colour, settings.value("colour").toString());
	select(m_size, settings.value("size").toString());
	m_sides->setCurrentIndex(qBound(0, settings.value("sides", 0).toInt(), 2));

	auto *checks = new QHBoxLayout;
	m_printable = new QCheckBox(tr("Only what QElectroTech can print"), this);
	m_printable->setObjectName(QStringLiteral("thm_material_printable"));
	m_printable->setChecked(settings.value("printable", true).toBool());
	m_loaded = new QCheckBox(tr("Only what a printer is loaded with"), this);
	m_loaded->setObjectName(QStringLiteral("thm_material_loaded"));
	m_loaded->setChecked(settings.value("loaded", false).toBool());
	checks->addWidget(m_printable);
	checks->addWidget(m_loaded);
	checks->addStretch();
	layout->addLayout(checks);
	settings.endGroup();

	m_table = new QTableWidget(0, 9, this);
	m_table->setObjectName(QStringLiteral("thm_material_table"));
	m_table->setHorizontalHeaderLabels({tr("Name"), tr("Number"), tr("Category"), tr("Size"), tr("Colour"),
										tr("Columns"), tr("Sides"), tr("Loaded on"), tr("Note")});
	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setStretchLastSection(true);
	m_table->verticalHeader()->setVisible(false);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->setSelectionMode(QAbstractItemView::SingleSelection);
	m_table->setSortingEnabled(true);
	layout->addWidget(m_table, 1);
	m_count = new QLabel(this);
	m_count->setObjectName(QStringLiteral("thm_material_count"));
	layout->addWidget(m_count);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	layout->addWidget(buttons);
	QPushButton *ok = buttons->button(QDialogButtonBox::Ok);
	ok->setText(tr("Take material"));
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(m_table, &QTableWidget::cellDoubleClicked, this, [this]() { if (chosen()) accept(); });
	connect(m_table, &QTableWidget::itemSelectionChanged, ok, [this, ok]() { ok->setEnabled(chosen()); });
	connect(m_text, &QLineEdit::textChanged, this, &ThmMaterialDialog::fill);
	for (QComboBox *c : {m_category, m_size, m_colour, m_sides})
		connect(c, &QComboBox::currentIndexChanged, this, &ThmMaterialDialog::fill);
	for (QCheckBox *c : {m_printable, m_loaded})
		connect(c, &QCheckBox::toggled, this, &ThmMaterialDialog::fill);
	connect(this, &QDialog::finished, this, [this]() {
		QSettings s;
		s.beginGroup(settings_group);
		s.setValue("category", m_category->currentData());
		s.setValue("colour", m_colour->currentData());
		s.setValue("size", m_size->currentData());
		s.setValue("sides", m_sides->currentIndex());
		s.setValue("printable", m_printable->isChecked());
		s.setValue("loaded", m_loaded->isChecked());
	});

	fill();
		//The material chosen so far: selected when the filter shows it
	if (!current.trimmed().isEmpty()) {
		for (int r = 0; r < m_table->rowCount(); ++r) {
			if (m_table->item(r, 1)->text() == current.trimmed()) {
				m_table->selectRow(r);
				m_table->scrollToItem(m_table->item(r, 0));
				break;
			}
		}
	}
	ok->setEnabled(chosen());
	resize(qMax(width(), 900), qMax(height(), 520));
}

void ThmMaterialDialog::fill()
{
	ThmMaterials::Filter f;
	f.text = m_text->text();
	f.category = m_category->currentData().toString();
	f.size = m_size->currentData().toString();
	f.colour = m_colour->currentData().toString();
	f.sides = m_sides->currentData().toInt();
	f.printable_only = m_printable->isChecked();
	f.loaded_only = m_loaded->isChecked();

	m_table->setSortingEnabled(false);
	m_table->setRowCount(0);
	int shown = 0;
	for (int i = 0; i < m_entries.size(); ++i) {
		const ThmMaterials::Entry &e = m_entries.at(i);
		if (!ThmMaterials::matches(e, f)) continue;
		const int r = m_table->rowCount();
		m_table->insertRow(r);
		const auto cell = [&](int column, const QString &text) {
			auto *item = new QTableWidgetItem(text);
			if (column == 0) item->setData(Qt::UserRole, i);
			if (!e.refusal.isEmpty()) item->setToolTip(e.refusal);
			m_table->setItem(r, column, item);
		};
		cell(0, e.name);
		cell(1, e.article);
		cell(2, e.category);
		cell(3, e.sizeText());
		cell(4, e.colour);
		cell(5, e.columns > 0 ? QString::number(e.columns) : QString());
		cell(6, e.two_sided ? tr("two") : tr("one"));
		cell(7, e.loaded_on.join(QStringLiteral(", ")));
		cell(8, !e.refusal.isEmpty() ? e.refusal
				: (e.has_layout ? QString() : tr("no layout: the printer's own profile")));
		++shown;
	}
	m_table->setSortingEnabled(true);
	m_count->setText(tr("%1 of %2 materials").arg(shown).arg(m_entries.size()));
}

const ThmMaterials::Entry *ThmMaterialDialog::chosen() const
{
	const QList<QTableWidgetItem *> items = m_table->selectedItems();
	for (QTableWidgetItem *item : items) {
		const QTableWidgetItem *first = m_table->item(item->row(), 0);
		if (!first) continue;
		bool ok = false;
		const int i = first->data(Qt::UserRole).toInt(&ok);
		if (ok && i >= 0 && i < m_entries.size()) return &m_entries.at(i);
	}
	return nullptr;
}

QString ThmMaterialDialog::article() const
{
	const ThmMaterials::Entry *e = chosen();
	return e ? e->article : QString();
}

QString ThmMaterialDialog::name() const
{
	const ThmMaterials::Entry *e = chosen();
	return e ? e->name : QString();
}
