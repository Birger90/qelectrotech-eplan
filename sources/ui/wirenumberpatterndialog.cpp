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
#include "wirenumberpatterndialog.h"

#include "../autoNum/wirenumberpattern.h"
#include "../qetdiagrameditor.h"
#include "../qetproject.h"
#include "../shortcutmanager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <QUndoStack>
#include <QVBoxLayout>

WireNumberPatternDialog::WireNumberPatternDialog(QETProject *project, QWidget *parent) :
	QDialog(parent),
	m_project(project)
{
	setWindowTitle(tr("Wire numbers by pattern"));
	auto *layout = new QVBoxLayout(this);
	auto *form = new QFormLayout();

	m_active = new QCheckBox(tr("New wires take their number from the pattern"), this);
	form->addRow(m_active);
	m_pattern = new QLineEdit(this);
	m_pattern->setToolTip(tr("{seite} folio, {spalte} column, {zeile} row, {schrank} location (+), "
							 "{anlage} plant (=), {n} counter, {n:2} at least two digits, "
							 "{n:A} letters A, B ... Z, AA"));
	form->addRow(tr("Pattern:"), m_pattern);
	m_scope = new QComboBox(this);
	m_scope->addItem(tr("Starts again on each folio"));
	m_scope->addItem(tr("Starts again in each cabinet (+)"));
	m_scope->addItem(tr("One counter for the project"));
	form->addRow(tr("Counter:"), m_scope);
	m_reserve = new QSpinBox(this);
	m_reserve->setRange(0, 999);
	m_reserve->setToolTip(tr("Numbers left free after each folio, for wires drawn later; "
							 "the next folios keep their numbers"));
	form->addRow(tr("Reserve per folio:"), m_reserve);
	m_order = new QComboBox(this);
	m_order->addItem(tr("By columns (left to right)"));
	m_order->addItem(tr("By rows (top down)"));
	form->addRow(tr("Order on the folio:"), m_order);
	m_mode = new QComboBox(this);
	m_mode->addItem(tr("Fixed: numbers stay, new wires take the first free one"));
	m_mode->addItem(tr("Recalculate: number every wire anew, in order"));
	m_mode->setToolTip(tr("Numbers typed by hand (L1, +24V) and frozen wires stay in both"));
	form->addRow(tr("Numbering:"), m_mode);
	m_start = new QSpinBox(this);
	m_start->setRange(0, 99999);
	form->addRow(tr("First number:"), m_start);
	layout->addLayout(form);

	m_problem = new QLabel(this);
	m_problem->setStyleSheet(QStringLiteral("color: #b00020"));
	layout->addWidget(m_problem);
	m_preview = new QTreeWidget(this);
	m_preview->setColumnCount(2);
	m_preview->setHeaderLabels({tr("Now"), tr("New")});
	m_preview->setRootIsDecorated(false);
	layout->addWidget(m_preview, 1);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	m_number = buttons->addButton(tr("Number wires now"), QDialogButtonBox::ApplyRole);
	layout->addWidget(buttons);
	connect(buttons, &QDialogButtonBox::accepted, this, [this]() { save(); accept(); });
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(m_number, &QPushButton::clicked, this, [this]() {
		save();
		numberNow();
		updatePreview();
	});

	const WireNumberPattern::Settings s = WireNumberPattern::settings(project);
	m_active->setChecked(s.active);
	m_pattern->setText(s.pattern);
	m_scope->setCurrentIndex(int(s.scope));
	m_reserve->setValue(s.reserve);
	m_order->setCurrentIndex(int(s.order));
	m_mode->setCurrentIndex(int(s.mode));
	m_start->setValue(s.start);

	connect(m_pattern, &QLineEdit::textChanged, this, &WireNumberPatternDialog::updatePreview);
	for (QComboBox *box : {m_scope, m_order, m_mode})
		connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
				&WireNumberPatternDialog::updatePreview);
	for (QSpinBox *box : {m_reserve, m_start})
		connect(box, QOverload<int>::of(&QSpinBox::valueChanged), this,
				&WireNumberPatternDialog::updatePreview);
	resize(560, 520);
	updatePreview();
}

void WireNumberPatternDialog::addMenu(QETDiagramEditor *editor, QMenu *project_menu)
{
	QAction *action = project_menu->addAction(tr("Wire numbers by pattern..."));
	action->setStatusTip(tr("Number the wires by a pattern like {seite}.{n:2}: counter per folio, "
							"cabinet or project, reserve per folio, by columns or rows"));
	ShortcutManager::instance().registerAction(action, "diagrameditor.wire_number_pattern",
											   tr("Diagram editors"), QKeySequence());
	QObject::connect(action, &QAction::triggered, editor, [editor]() {
		QETProject *project = editor->currentProject();
		if (!project) return;
		WireNumberPatternDialog dialog(project, editor);
		dialog.exec();
	});
}

WireNumberPattern::Settings WireNumberPatternDialog::chosen() const
{
	WireNumberPattern::Settings s;
	s.active = m_active->isChecked();
	s.pattern = m_pattern->text().trimmed();
	s.scope = WireNumberPattern::Scope(m_scope->currentIndex());
	s.reserve = m_reserve->value();
	s.order = WireNumberPattern::Order(m_order->currentIndex());
	s.mode = WireNumberPattern::Mode(m_mode->currentIndex());
	s.start = m_start->value();
	return s;
}

void WireNumberPatternDialog::updatePreview()
{
	m_preview->clear();
	const WireNumberPattern::Settings s = chosen();
	const QString problem = WireNumberPattern::problem(s.pattern);
	m_problem->setText(problem);
	m_number->setEnabled(problem.isEmpty() && m_project && !m_project->isReadOnly());
	if (!problem.isEmpty() || !m_project) return;
	const WireNumberPattern::Plan plan = WireNumberPattern::plan(m_project, s);
	for (const WireNumberPattern::Change &c : plan.changes) {
		auto *row = new QTreeWidgetItem(m_preview);
		row->setText(0, c.old_text.isEmpty() ? tr("(none)") : c.old_text);
		row->setText(1, c.new_text);
	}
	if (plan.changes.isEmpty()) {
		auto *row = new QTreeWidgetItem(m_preview);
		row->setText(0, tr("Every wire already has its number"));
	}
}

void WireNumberPatternDialog::save()
{
	if (m_project && WireNumberPattern::problem(chosen().pattern).isEmpty())
		WireNumberPattern::setSettings(m_project, chosen());
}

void WireNumberPatternDialog::numberNow()
{
	if (!m_project || m_project->isReadOnly()) return;
	const WireNumberPattern::Plan plan = WireNumberPattern::plan(m_project, chosen());
	if (QUndoCommand *cmd = WireNumberPattern::command(plan))
		m_project->undoStack()->push(cmd);
}
