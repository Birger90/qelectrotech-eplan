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
#include "functiontemplatedock.h"

#include "../diagram.h"
#include "../diagramview.h"
#include "../projectview.h"
#include "../qetdiagrameditor.h"
#include "../qetproject.h"
#include "diagrameventaddfunctiontemplate.h"

#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
	const int PathRole = Qt::UserRole + 1;
}

// ---------------------------------------------------------------------------
// FunctionTemplateForm
// ---------------------------------------------------------------------------

FunctionTemplateForm::FunctionTemplateForm(QWidget *parent) :
	QWidget(parent)
{
	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	m_layout = new QFormLayout();
	m_layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	layout->addLayout(m_layout);
	m_info = new QLabel(this);
	m_info->setWordWrap(true);
	m_info->setTextInteractionFlags(Qt::TextSelectableByMouse);
	m_info->setStyleSheet(QStringLiteral("color: palette(mid);"));
	layout->addWidget(m_info);
}

void FunctionTemplateForm::setTemplate(const FunctionTemplate &tpl, Diagram *folio)
{
	m_template = tpl;
	m_folio = folio;
	while (m_layout->rowCount()) m_layout->removeRow(0);
	m_fields.clear();

	const QMap<QString, QString> proposed = tpl.proposals(folio);
	for (const FunctionTemplate::Parameter &p : tpl.parameters()) {
		auto *field = new QComboBox(this);
		field->setEditable(true);
		field->setInsertPolicy(QComboBox::NoInsert);
		field->addItems(tpl.choicesFor(p));
		field->setCurrentText(proposed.value(p.name));
		if (!p.help.isEmpty()) field->setToolTip(p.help);
		QString label = p.label.isEmpty() ? p.name : p.label;
		if (!p.unit.isEmpty()) label += QStringLiteral(" [%1]").arg(p.unit);
		m_layout->addRow(label + QLatin1Char(':'), field);
		m_fields.insert(p.name, field);
		connect(field, &QComboBox::currentTextChanged, this, [this]() {
			refreshInfo();
			emit changed();
		});
	}
	if (tpl.isValid() && tpl.parameters().isEmpty())
		m_layout->addRow(new QLabel(tr("This template asks for nothing: its labels are counted on."), this));
	refreshInfo();
}

QMap<QString, QString> FunctionTemplateForm::values() const
{
	QMap<QString, QString> values;
	for (auto it = m_fields.cbegin() ; it != m_fields.cend() ; ++it)
		values.insert(it.key(), it.value()->currentText().trimmed());
	return values;
}

void FunctionTemplateForm::refreshInfo()
{
	if (m_template.tables().isEmpty()) {
		m_info->clear();
		m_info->hide();
		return;
	}
	QStringList notes;
	const QMap<QString, QString> resolved = m_template.resolve(values(), m_folio, &notes);
	QStringList lines;
	for (const FunctionTemplate::Table &table : m_template.tables()) {
		const int row = FunctionTemplate::rowFor(table, resolved.value(table.key));
		if (row < 0) continue;
		QStringList cells;
		for (const QString &column : table.columns)
			if (column != table.key) cells << QStringLiteral("%1 = %2").arg(column, table.rows.at(row).value(column));
		lines << tr("Row %1 = %2: %3").arg(table.key, table.rows.at(row).value(table.key),
										  cells.join(QStringLiteral(", ")));
	}
	lines << notes;
	m_info->setText(lines.join(QLatin1Char('\n')));
	m_info->setVisible(!lines.isEmpty());
}

// ---------------------------------------------------------------------------
// FunctionTemplateValuesDialog
// ---------------------------------------------------------------------------

FunctionTemplateValuesDialog::FunctionTemplateValuesDialog(const FunctionTemplate &tpl, Diagram *folio,
														   QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Function template %1").arg(tpl.name()));
	auto *layout = new QVBoxLayout(this);
	if (!tpl.description().isEmpty()) {
		auto *description = new QLabel(tpl.description(), this);
		description->setWordWrap(true);
		layout->addWidget(description);
	}
	m_form = new FunctionTemplateForm(this);
	m_form->setTemplate(tpl, folio);
	layout->addWidget(m_form);
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttons);
}

QMap<QString, QString> FunctionTemplateValuesDialog::values() const
{
	return m_form->values();
}

// ---------------------------------------------------------------------------
// FunctionTemplateDock
// ---------------------------------------------------------------------------

FunctionTemplateDock::FunctionTemplateDock(QETDiagramEditor *editor) :
	QDockWidget(tr("Function templates"), editor),
	m_editor(editor)
{
	setObjectName(QStringLiteral("function_templates_dock"));
	auto *content = new QWidget(this);
	auto *layout = new QVBoxLayout(content);

	auto *top = new QHBoxLayout();
	m_search = new QLineEdit(content);
	m_search->setPlaceholderText(tr("Search a template"));
	m_search->setClearButtonEnabled(true);
	top->addWidget(m_search);
	auto *reload_button = new QToolButton(content);
	reload_button->setText(QStringLiteral("⟳"));
	reload_button->setToolTip(tr("Read the templates again"));
	top->addWidget(reload_button);
	auto *folder_button = new QToolButton(content);
	folder_button->setText(QStringLiteral("…"));
	folder_button->setToolTip(tr("Open the folder of your own templates"));
	top->addWidget(folder_button);
	layout->addLayout(top);

	auto *splitter = new QSplitter(Qt::Vertical, content);
	m_tree = new QTreeWidget(splitter);
	m_tree->setHeaderHidden(true);
	m_tree->setRootIsDecorated(true);

	auto *details = new QWidget();
	auto *details_layout = new QVBoxLayout(details);
	details_layout->setContentsMargins(0, 0, 0, 0);
	m_preview = new QLabel(details);
	m_preview->setAlignment(Qt::AlignCenter);
	m_preview->setMinimumHeight(140);
	m_preview->setFrameShape(QFrame::StyledPanel);
	m_preview->setStyleSheet(QStringLiteral("background: white; color: gray;"));
	details_layout->addWidget(m_preview);
	m_description = new QLabel(details);
	m_description->setWordWrap(true);
	details_layout->addWidget(m_description);
	m_form = new FunctionTemplateForm(details);
	details_layout->addWidget(m_form);
	auto *buttons = new QHBoxLayout();
	m_insert = new QPushButton(tr("Insert"), details);
	m_insert->setToolTip(tr("Place it with the mouse on the current folio"));
	m_insert->setDefault(true);
	m_new_folio = new QPushButton(tr("On a new folio"), details);
	m_new_folio->setToolTip(tr("Add a folio at the end of the project and put the template on it"));
	buttons->addWidget(m_insert);
	buttons->addWidget(m_new_folio);
	details_layout->addLayout(buttons);
	m_result = new QLabel(details);
	m_result->setWordWrap(true);
	details_layout->addWidget(m_result);
	details_layout->addStretch();
	auto *scroll = new QScrollArea(splitter);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setWidget(details);
	splitter->addWidget(m_tree);
	splitter->addWidget(scroll);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 2);
	layout->addWidget(splitter);
	setWidget(content);

	m_preview_timer = new QTimer(this);
	m_preview_timer->setSingleShot(true);
	m_preview_timer->setInterval(250);
	connect(m_preview_timer, &QTimer::timeout, this, &FunctionTemplateDock::refreshPreview);

	connect(m_search, &QLineEdit::textChanged, this, &FunctionTemplateDock::fill);
	connect(reload_button, &QToolButton::clicked, this, &FunctionTemplateDock::reload);
	connect(folder_button, &QToolButton::clicked, this, []() {
		QDir().mkpath(FunctionTemplate::userDirectory());
		QDesktopServices::openUrl(QUrl::fromLocalFile(FunctionTemplate::userDirectory()));
	});
	connect(m_tree, &QTreeWidget::currentItemChanged, this,
			[this](QTreeWidgetItem *item) {select(item);});
	connect(m_tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item) {
		if (item && !item->data(0, PathRole).toString().isEmpty()) insertHere();
	});
	connect(m_form, &FunctionTemplateForm::changed, m_preview_timer, qOverload<>(&QTimer::start));
	connect(m_insert, &QPushButton::clicked, this, &FunctionTemplateDock::insertHere);
	connect(m_new_folio, &QPushButton::clicked, this, &FunctionTemplateDock::insertOnNewFolio);
	connect(this, &QDockWidget::visibilityChanged, this, [this](bool visible) {
		if (visible && m_templates.isEmpty()) reload();
	});
	select(nullptr);
}

void FunctionTemplateDock::reload()
{
	const QString was = current().path();
	m_templates = FunctionTemplate::available();
	fill();
	if (was.isEmpty()) return;
	for (QTreeWidgetItemIterator it(m_tree) ; *it ; ++it)
		if ((*it)->data(0, PathRole).toString() == was) {
			m_tree->setCurrentItem(*it);
			break;
		}
}

void FunctionTemplateDock::fill()
{
	const QString search = m_search->text().trimmed();
	m_tree->clear();
	QMap<QString, QTreeWidgetItem *> categories;
	for (const FunctionTemplate &tpl : std::as_const(m_templates)) {
		if (!search.isEmpty()
				&& !tpl.name().contains(search, Qt::CaseInsensitive)
				&& !tpl.category().contains(search, Qt::CaseInsensitive)
				&& !tpl.description().contains(search, Qt::CaseInsensitive))
			continue;
		QString category = tpl.category().isEmpty() ? tr("Without category") : tpl.category();
		if (tpl.isExample()) category = tr("Examples") + QStringLiteral(" / ") + category;
		QTreeWidgetItem *parent = categories.value(category);
		if (!parent) {
			parent = new QTreeWidgetItem(m_tree, {category});
			parent->setFlags(Qt::ItemIsEnabled);
			categories.insert(category, parent);
		}
		auto *item = new QTreeWidgetItem(parent, {tpl.name()});
		item->setData(0, PathRole, tpl.path());
		item->setToolTip(0, QDir::toNativeSeparators(tpl.path()));
	}
	m_tree->sortItems(0, Qt::AscendingOrder);
	m_tree->expandAll();
	if (m_templates.isEmpty()) {
		auto *hint = new QTreeWidgetItem(m_tree, {tr("No template yet: select part of a folio and "
													 "choose \"Save as function template…\".")});
		hint->setFlags(Qt::ItemIsEnabled);
	}
}

FunctionTemplate FunctionTemplateDock::current() const
{
	const QTreeWidgetItem *item = m_tree ? m_tree->currentItem() : nullptr;
	const QString path = item ? item->data(0, PathRole).toString() : QString();
	if (path.isEmpty()) return FunctionTemplate();
	for (const FunctionTemplate &tpl : m_templates)
		if (tpl.path() == path) return tpl;
	return FunctionTemplate();
}

Diagram *FunctionTemplateDock::currentFolio() const
{
	DiagramView *view = m_editor ? m_editor->currentDiagramView() : nullptr;
	return view ? view->diagram() : nullptr;
}

void FunctionTemplateDock::select(QTreeWidgetItem *)
{
	const FunctionTemplate tpl = current();
	const bool valid = tpl.isValid();
	m_insert->setEnabled(valid);
	m_new_folio->setEnabled(valid);
	m_result->clear();
	if (!valid) {
		m_preview->setPixmap(QPixmap());
		m_preview->setText(tr("Choose a template"));
		m_description->clear();
		m_form->setTemplate(FunctionTemplate(), nullptr);
		m_form->hide();
		return;
	}
	m_form->show();
	m_description->setText(tpl.description());
	m_form->setTemplate(tpl, currentFolio());
	refreshPreview();
}

void FunctionTemplateDock::refreshPreview()
{
	const FunctionTemplate tpl = current();
	if (!tpl.isValid()) return;
	const int width = qMax(160, m_preview->width() - 8);
	const QImage image = tpl.preview(m_form->values(), QSize(width, 260));
	if (image.isNull()) m_preview->setText(tr("No picture"));
	else m_preview->setPixmap(QPixmap::fromImage(image));
}

void FunctionTemplateDock::insertHere()
{
	const FunctionTemplate tpl = current();
	DiagramView *view = m_editor ? m_editor->currentDiagramView() : nullptr;
	if (!tpl.isValid()) return;
	if (!view || !view->diagram() || view->diagram()->isReadOnly()) {
		m_result->setText(tr("Open a folio first, or put the template on a new folio."));
		return;
	}
	Diagram *folio = view->diagram();
	auto *event = new DiagramEventAddFunctionTemplate(tpl, m_form->values(), folio,
													 view->mapToScene(view->viewport()->rect().center()));
	connect(event, &DiagramEventAddFunctionTemplate::placed, this, &FunctionTemplateDock::report);
	folio->setEventInterface(event);
	view->setFocus();
}

void FunctionTemplateDock::insertOnNewFolio()
{
	const FunctionTemplate tpl = current();
	QETProject *project = m_editor ? m_editor->currentProject() : nullptr;
	if (!tpl.isValid()) return;
	if (!project) {
		m_result->setText(tr("Open a project first."));
		return;
	}
	const FunctionTemplate::Placed placed = tpl.placeOnNewFolio(project, m_form->values());
	report(placed);
	if (placed.ok && placed.folio)
		if (ProjectView *view = m_editor->currentProjectView())
			view->showDiagram(placed.folio);
}

void FunctionTemplateDock::report(const FunctionTemplate::Placed &placed)
{
	if (!placed.ok) {
		m_result->setText(placed.error);
		return;
	}
	QStringList lines;
	lines << tr("%n symbol(s) placed.", "", static_cast<int>(placed.elements.size()));
	lines << placed.notes;
	m_result->setText(lines.join(QLatin1Char('\n')));
		//The next labels and terminals are free ones again
	m_form->setTemplate(current(), currentFolio());
}
