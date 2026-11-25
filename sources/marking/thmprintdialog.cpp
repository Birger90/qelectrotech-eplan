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
#include "thmprintdialog.h"

#include "thmmaterialdialog.h"

#include "../dataBase/projectdatabase.h"
#include "../qet.h"
#include "../qetproject.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
const char *settings_group = "thm_print_dialog";

QDoubleSpinBox *millimetres(QWidget *parent, double max = 500)
{
	auto *spin = new QDoubleSpinBox(parent);
	spin->setRange(0, max);
	spin->setDecimals(2);
	spin->setSingleStep(0.5);
	spin->setSuffix(QStringLiteral(" mm"));
	return spin;
}
}

ThmPrintDialog::ThmPrintDialog(QETProject *project, QWidget *parent) :
	QDialog(parent),
	m_project(project),
	m_client(new ThmClient(this))
{
	setWindowTitle(tr("Print THM labels"));
	QET::trackDialogGeometry(this);
	if (m_project && m_project->dataBase()) m_project->dataBase()->updateDB();
	m_printers = ThmPrinters::load();
	connect(m_client, &ThmClient::finished, this, &ThmPrintDialog::onReply);

	QSettings settings;
	settings.beginGroup(settings_group);
	auto *main_layout = new QVBoxLayout(this);
	auto *top = new QHBoxLayout;
	main_layout->addLayout(top);

		//What gets a label
	auto *left = new QVBoxLayout;
	top->addLayout(left, 1);
	auto *kinds = new QGroupBox(tr("Markers"), this);
	auto *kinds_layout = new QHBoxLayout(kinds);
	const int kind_flags = settings.value("kinds", int(MarkingExport::Wire)).toInt();
	m_wires = new QCheckBox(tr("Wires"), kinds);
	m_cables = new QCheckBox(tr("Cables"), kinds);
	m_terminals = new QCheckBox(tr("Terminals"), kinds);
	m_devices = new QCheckBox(tr("Devices"), kinds);
	m_wires->setChecked(kind_flags & MarkingExport::Wire);
	m_cables->setChecked(kind_flags & MarkingExport::Cable);
	m_terminals->setChecked(kind_flags & MarkingExport::Terminal);
	m_devices->setChecked(kind_flags & MarkingExport::Device);
	for (auto *box : {m_wires, m_cables, m_terminals, m_devices}) kinds_layout->addWidget(box);
	left->addWidget(kinds);

	auto *range = new QGroupBox(tr("Range"), this);
	auto *range_form = new QFormLayout(range);
	const int folios = m_project ? m_project->diagrams().size() : 0;
	auto *folio_row = new QHBoxLayout;
	m_folio_from = new QSpinBox(range);
	m_folio_to = new QSpinBox(range);
	for (auto *spin : {m_folio_from, m_folio_to}) spin->setRange(1, qMax(1, folios));
	m_folio_from->setValue(1);
	m_folio_to->setValue(qMax(1, folios));
	folio_row->addWidget(m_folio_from);
	folio_row->addWidget(new QLabel(tr("to"), range));
	folio_row->addWidget(m_folio_to);
	folio_row->addStretch();
	range_form->addRow(tr("Folios:"), folio_row);
	m_full = new QCheckBox(tr("Full reference designation (=plant+location-device)"), range);
	m_full->setChecked(settings.value("full_designation", false).toBool());
	m_both_ends = new QCheckBox(tr("A marker at each end of wires and cables"), range);
	m_both_ends->setChecked(settings.value("both_ends", true).toBool());
	m_merge = new QCheckBox(tr("Merge equal texts"), range);
	m_merge->setObjectName(QStringLiteral("thm_merge"));
	m_merge->setChecked(settings.value("merge", true).toBool());
	m_copies = new QSpinBox(range);
	m_copies->setRange(1, 99);
	m_copies->setValue(settings.value("copies", 1).toInt());
	range_form->addRow(m_full);
	range_form->addRow(m_both_ends);
	range_form->addRow(m_merge);
	range_form->addRow(tr("Copies of each marker:"), m_copies);
	left->addWidget(range);

	m_preview = new QTableWidget(0, 2, this);
	m_preview->setHorizontalHeaderLabels({tr("Text"), tr("Labels")});
	m_preview->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_preview->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_preview->verticalHeader()->setVisible(false);
	m_preview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_preview->setSelectionMode(QAbstractItemView::NoSelection);
	m_preview->setObjectName(QStringLiteral("thm_preview"));
	left->addWidget(m_preview, 1);
	m_count = new QLabel(this);
	m_count->setObjectName(QStringLiteral("thm_count"));
	left->addWidget(m_count);

		//Where to: the article picks the printer
	auto *target = new QGroupBox(tr("Target"), this);
	top->addWidget(target, 1);
	auto *target_layout = new QVBoxLayout(target);
	auto *target_form = new QFormLayout;
	target_layout->addLayout(target_form);
	m_article = new QComboBox(target);
	m_article->setEditable(true);
	m_article->setInsertPolicy(QComboBox::NoInsert);
	m_article->setObjectName(QStringLiteral("thm_article"));
	auto *article_row = new QHBoxLayout;
	article_row->addWidget(m_article, 1);
	auto *choose_button = new QPushButton(tr("Choose..."), target);
	choose_button->setObjectName(QStringLiteral("thm_choose_material"));
	choose_button->setToolTip(tr("Choose the material from the catalog, filtered by category, size and colour"));
	article_row->addWidget(choose_button);
	target_form->addRow(tr("Article variant:"), article_row);
	connect(choose_button, &QPushButton::clicked, this, [this]() {
		ThmMaterialDialog dialog(ThmMaterials::entries(m_layouts, m_catalog, m_printers),
								 currentArticle(), this);
		if (dialog.exec() != QDialog::Accepted || dialog.article().isEmpty()) return;
		const int i = m_article->findData(dialog.article());
		if (i >= 0) m_article->setCurrentIndex(i);
		else m_article->setCurrentText(dialog.article());
		updateTarget();
	});
	auto *catalog_row = new QHBoxLayout;
	m_catalog_label = new QLabel(target);
	m_catalog_label->setWordWrap(true);
	auto *catalog_button = new QPushButton(tr("Catalog file..."), target);
	catalog_row->addWidget(m_catalog_label, 1);
	catalog_row->addWidget(catalog_button);
	target_form->addRow(tr("Catalog:"), catalog_row);
	auto *layouts_row = new QHBoxLayout;
	m_layouts_label = new QLabel(target);
	m_layouts_label->setWordWrap(true);
	m_layouts_label->setObjectName(QStringLiteral("thm_layouts"));
	auto *layouts_button = new QPushButton(tr("Layout file..."), target);
	layouts_row->addWidget(m_layouts_label, 1);
	layouts_row->addWidget(layouts_button);
	target_form->addRow(tr("Layouts:"), layouts_row);
	m_printer = new QComboBox(target);
	m_printer->setObjectName(QStringLiteral("thm_printer"));
	target_form->addRow(tr("Printer:"), m_printer);
	m_target_message = new QLabel(target);
	m_target_message->setWordWrap(true);
	m_target_message->setObjectName(QStringLiteral("thm_target_message"));
	target_layout->addWidget(m_target_message);
	m_profile_source = new QLabel(target);
	m_profile_source->setWordWrap(true);
	m_profile_source->setObjectName(QStringLiteral("thm_profile_source"));
	target_layout->addWidget(m_profile_source);
	auto *target_buttons = new QHBoxLayout;
	auto *printers_button = new QPushButton(tr("Printers..."), target);
	auto *change_button = new QPushButton(tr("Change material..."), target);
	change_button->setObjectName(QStringLiteral("thm_change_material"));
	change_button->setToolTip(tr("Load the article chosen on a printer and confirm it"));
	target_buttons->addWidget(printers_button);
	target_buttons->addWidget(change_button);
	target_buttons->addStretch();
	target_layout->addLayout(target_buttons);

	auto *device = new QGroupBox(tr("Printer state"), target);
	auto *device_layout = new QVBoxLayout(device);
	auto *query_row = new QHBoxLayout;
	auto *status_button = new QPushButton(tr("Query status"), device);
	auto *material_button = new QPushButton(tr("Query material"), device);
	query_row->addWidget(status_button);
	query_row->addWidget(material_button);
	query_row->addStretch();
	device_layout->addLayout(query_row);
	auto *control_row = new QHBoxLayout;
	auto *pause_button = new QPushButton(tr("Pause"), device);
	auto *resume_button = new QPushButton(tr("Resume"), device);
	auto *cancel_button = new QPushButton(tr("Cancel job..."), device);
	control_row->addWidget(pause_button);
	control_row->addWidget(resume_button);
	control_row->addWidget(cancel_button);
	control_row->addStretch();
	device_layout->addLayout(control_row);
	m_device_state = new QLabel(device);
	m_device_state->setWordWrap(true);
	m_device_state->setTextInteractionFlags(Qt::TextSelectableByMouse);
	device_layout->addWidget(m_device_state);
	target_layout->addWidget(device);
	target_layout->addStretch();
	m_device_buttons = {status_button, material_button, pause_button, resume_button, cancel_button};

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	m_save = buttons->addButton(tr("Save as .prn..."), QDialogButtonBox::ActionRole);
	m_print = buttons->addButton(tr("Print..."), QDialogButtonBox::AcceptRole);
	m_print->setObjectName(QStringLiteral("thm_print"));
	main_layout->addWidget(buttons);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(m_print, &QPushButton::clicked, this, &ThmPrintDialog::print);
	connect(m_save, &QPushButton::clicked, this, &ThmPrintDialog::saveJob);
	connect(catalog_button, &QPushButton::clicked, this, &ThmPrintDialog::chooseCatalog);
	connect(printers_button, &QPushButton::clicked, this, &ThmPrintDialog::managePrinters);
	connect(change_button, &QPushButton::clicked, this, &ThmPrintDialog::changeMaterial);
	connect(layouts_button, &QPushButton::clicked, this, &ThmPrintDialog::chooseLayouts);
	connect(status_button, &QPushButton::clicked, this, [this]() {
		query({ThmClient::Request::Version, ThmClient::Request::Status});
	});
	connect(material_button, &QPushButton::clicked, this, [this]() {
		query({ThmClient::Request::Material});
	});
	connect(pause_button, &QPushButton::clicked, this, [this]() { query({ThmClient::Request::Pause}); });
	connect(resume_button, &QPushButton::clicked, this, [this]() { query({ThmClient::Request::Resume}); });
	connect(cancel_button, &QPushButton::clicked, this, &ThmPrintDialog::cancelJob);

	for (auto *box : {m_wires, m_cables, m_terminals, m_devices, m_full, m_both_ends, m_merge}) {
		connect(box, &QCheckBox::toggled, this, &ThmPrintDialog::updatePreview);
	}
	for (auto *spin : {m_folio_from, m_folio_to, m_copies}) {
		connect(spin, &QSpinBox::valueChanged, this, &ThmPrintDialog::updatePreview);
	}
	connect(m_article, &QComboBox::currentTextChanged, this, &ThmPrintDialog::updateTarget);
	connect(m_printer, &QComboBox::currentIndexChanged, this, [this]() {
		m_material = ThmJob::Material();
		m_device_state->clear();
		updateTarget();
	});

	const QString article = settings.value("article").toString();
	settings.endGroup();
	loadCatalog();
	loadLayouts();
	fillArticles();
	if (!article.isEmpty()) m_article->setCurrentText(article);
	updatePreview();
	updateTarget();
}

MarkingExport::Options ThmPrintDialog::options() const
{
	MarkingExport::Options o;
	o.kinds = (m_wires->isChecked() ? MarkingExport::Wire : 0)
			  | (m_terminals->isChecked() ? MarkingExport::Terminal : 0)
			  | (m_devices->isChecked() ? MarkingExport::Device : 0)
			  | (m_cables->isChecked() ? MarkingExport::Cable : 0);
	o.folio_from = qMin(m_folio_from->value(), m_folio_to->value());
	o.folio_to = qMax(m_folio_from->value(), m_folio_to->value());
	o.full_designation = m_full->isChecked();
	o.both_ends = m_both_ends->isChecked();
	return o;
}

QList<ThmJob::Label> ThmPrintDialog::labels() const
{
	const MarkingExport::Options o = options();
	if (!o.kinds) return {};
	return ThmJob::toLabels(MarkingExport::collect(m_project, o), m_copies->value(),
							m_merge->isChecked());
}

void ThmPrintDialog::loadCatalog()
{
	QSettings settings;
	const QString path = ThmPrinters::catalogFile(settings);
	m_catalog.clear();
	m_catalog_error.clear();
	if (path.isEmpty()) {
		m_catalog_label->setText(tr("none chosen"));
		return;
	}
	m_catalog = ThmCatalog::read(path, &m_catalog_error);
	m_catalog_label->setText(m_catalog_error.isEmpty()
							 ? tr("%1 (%n variant(s))", nullptr, int(m_catalog.size()))
							   .arg(QFileInfo(path).fileName())
							 : m_catalog_error);
}

void ThmPrintDialog::chooseCatalog()
{
	QSettings settings;
	const QString current = ThmPrinters::catalogFile(settings);
	const QString path = QFileDialog::getOpenFileName(
		this, tr("Weidmüller catalog file"),
		current.isEmpty() ? QDir::homePath() : QFileInfo(current).absolutePath(),
		tr("Weidmüller catalog (my_cat*.mca *.mca);;All files (*)"));
	if (path.isEmpty()) return;
	ThmPrinters::setCatalogFile(settings, path);
	const QString article = m_article->currentText();
	loadCatalog();
	fillArticles();
	m_article->setCurrentText(article);
}

void ThmPrintDialog::loadLayouts()
{
	QSettings settings;
	const QString path = ThmPrinters::layoutsFile(settings);
	m_layouts.clear();
	m_layouts_error.clear();
	if (path.isEmpty()) {
		m_layouts_label->setText(tr("none chosen: the printer's own profile is used"));
		return;
	}
	m_layouts = ThmLayouts::read(path, &m_layouts_error);
	int printable = 0;
	for (const ThmLayouts::Variant &v : m_layouts) printable += v.refusal.isEmpty();
	m_layouts_label->setText(m_layouts_error.isEmpty()
							 ? tr("%1 (%2 of %3 variants printable)").arg(QFileInfo(path).fileName())
							   .arg(printable).arg(m_layouts.size())
							 : m_layouts_error);
}

void ThmPrintDialog::chooseLayouts()
{
	QSettings settings;
	const QString current = ThmPrinters::layoutsFile(settings);
	const QString path = QFileDialog::getOpenFileName(
		this, tr("Weidmüller layout file"),
		current.isEmpty() ? QDir::homePath() : QFileInfo(current).absolutePath(),
		tr("Layout file (*.json);;All files (*)"));
	if (path.isEmpty()) return;
	ThmPrinters::setLayoutsFile(settings, path);
	const QString article = m_article->currentText();
	loadLayouts();
	fillArticles();
	m_article->setCurrentText(article);
	updateTarget();
}

bool ThmPrintDialog::jobProfile(const ThmPrinter *printer, ThmJob::Profile *profile,
								QString *source, QString *error) const
{
	const QString article = currentArticle();
	return ThmLayouts::jobProfile(m_layouts, article.isEmpty() && printer ? printer->material_no : article,
								  printer, profile, source, error);
}

void ThmPrintDialog::changeMaterial()
{
	const QString article = currentArticle();
	if (article.isEmpty()) {
		QMessageBox::information(this, tr("Change material"), tr("Choose the article to load first."));
		return;
	}
	if (m_printers.isEmpty()) {
		QMessageBox::information(this, tr("Change material"), tr("No THM printer is set up yet."));
		return;
	}
	const int v = ThmLayouts::best(m_layouts, article);
	const ThmLayouts::Variant *variant = v >= 0 ? &m_layouts.at(v) : nullptr;
	QString name = variant ? variant->name : QString();
	for (const ThmCatalog::Variant &c : m_catalog) {
		if (name.isEmpty() && ThmPrinters::normalizedMaterial(c.material_no) == ThmPrinters::normalizedMaterial(article))
			name = c.name;
	}

	QDialog dialog(this);
	dialog.setWindowTitle(tr("Change material"));
	auto *layout = new QVBoxLayout(&dialog);
	auto *form = new QFormLayout;
	layout->addLayout(form);
	form->addRow(tr("Article to load:"), new QLabel(name.isEmpty() ? article : article + QStringLiteral(" – ") + name, &dialog));
	if (variant) {
		if (!variant->ribbon.isEmpty()) form->addRow(tr("Ribbon:"), new QLabel(variant->ribbon, &dialog));
		if (!variant->support_plate.isEmpty()) form->addRow(tr("Support:"), new QLabel(variant->support_plate, &dialog));
		if (!variant->refusal.isEmpty()) {
			auto *warning = new QLabel(tr("QElectroTech cannot print it yet: %1").arg(variant->refusal), &dialog);
			warning->setWordWrap(true);
			form->addRow(warning);
		}
	}
	auto *printer = new QComboBox(&dialog);
	printer->setObjectName(QStringLiteral("thm_change_printer"));
	for (const ThmPrinter &p : m_printers) printer->addItem(p.name, p.id);
	const int chosen = printer->findData(m_printer->currentData());
	if (chosen >= 0) printer->setCurrentIndex(chosen);
	form->addRow(tr("Printer:"), printer);
	auto *state = new QLabel(&dialog);
	state->setWordWrap(true);
	form->addRow(state);
	auto *roller = new QLineEdit(&dialog);
	roller->setPlaceholderText(tr("the roller now fitted, as on its label"));
	form->addRow(tr("Roller:"), roller);
	auto *loaded = new QCheckBox(tr("The article and its roller are loaded"), &dialog);
	loaded->setObjectName(QStringLiteral("thm_change_loaded"));
	auto *usual = new QCheckBox(tr("Make it the usual article of this printer"), &dialog);
	usual->setObjectName(QStringLiteral("thm_change_usual"));
	layout->addWidget(loaded);
	layout->addWidget(usual);
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	layout->addWidget(buttons);
	QPushButton *ok = buttons->button(QDialogButtonBox::Ok);
	ok->setText(tr("Confirm loading"));
	ok->setEnabled(false);
	connect(loaded, &QCheckBox::toggled, ok, &QPushButton::setEnabled);
	connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	const auto show = [&]() {
		const int i = ThmPrinters::indexOf(m_printers, printer->currentData().toString());
		if (i < 0) return;
		const ThmPrinter &p = m_printers.at(i);
		QString text = tr("Loaded now: %1").arg(p.material_no.isEmpty() ? tr("nothing confirmed") : p.material_no);
		if (p.loaded_at.isValid())
			text += QStringLiteral(" (") + QLocale().toString(p.loaded_at, QLocale::ShortFormat) + QLatin1Char(')');
		text += QStringLiteral("\n") + tr("Usual article: %1").arg(p.usual_material_no.isEmpty() ? tr("none") : p.usual_material_no);
		state->setText(text);
		roller->setText(p.roller);
	};
	connect(printer, &QComboBox::currentIndexChanged, &dialog, show);
	show();
	if (dialog.exec() != QDialog::Accepted) return;

	const int i = ThmPrinters::indexOf(m_printers, printer->currentData().toString());
	if (i < 0) return;
	ThmPrinters::confirmLoading(&m_printers[i], article, name, QDateTime::currentDateTime(), usual->isChecked());
	m_printers[i].roller = roller->text().trimmed();
	ThmPrinters::save(m_printers);
	const QString text = m_article->currentText();
	fillArticles();
	m_article->setCurrentText(text);
	updateTarget();
	const int keep = m_printer->findData(m_printers.at(i).id);
	if (keep >= 0) m_printer->setCurrentIndex(keep);
}

void ThmPrintDialog::fillArticles()
{
	const QSignalBlocker blocker(m_article);
	const QString text = m_article->currentText();
	m_article->clear();
	QStringList seen;
		//The articles loaded on a printer first, then the catalog
	for (const ThmPrinter &p : m_printers) {
		const QString no = p.material_no.trimmed();
		if (no.isEmpty() || seen.contains(ThmPrinters::normalizedMaterial(no))) continue;
		seen << ThmPrinters::normalizedMaterial(no);
		m_article->addItem(p.material_name.isEmpty() ? no : QStringLiteral("%1 – %2").arg(no, p.material_name), no);
	}
	for (const ThmCatalog::Variant &v : m_catalog) {
		if (seen.contains(ThmPrinters::normalizedMaterial(v.material_no))) continue;
		seen << ThmPrinters::normalizedMaterial(v.material_no);
		m_article->addItem(ThmCatalog::title(v), v.material_no);
		m_article->setItemData(m_article->count() - 1, v.description, Qt::ToolTipRole);
	}
	for (const ThmLayouts::Variant &v : m_layouts) {
		if (seen.contains(ThmPrinters::normalizedMaterial(v.article))) continue;
		seen << ThmPrinters::normalizedMaterial(v.article);
		m_article->addItem(ThmLayouts::title(v), v.article);
		if (!v.refusal.isEmpty()) m_article->setItemData(m_article->count() - 1, v.refusal, Qt::ToolTipRole);
	}
	m_article->setCurrentText(text);
}

void ThmPrintDialog::updatePreview()
{
	const QList<ThmJob::Label> list = labels();
	m_preview->setRowCount(list.size());
	for (int i = 0; i < list.size(); ++i) {
		m_preview->setItem(i, 0, new QTableWidgetItem(list.at(i).text));
		auto *count = new QTableWidgetItem(QString::number(list.at(i).count));
		count->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_preview->setItem(i, 1, count);
	}
	if (!options().kinds) {
		m_count->setText(tr("Choose at least one kind of marker."));
	} else {
		m_count->setText(tr("%n label(s)", nullptr, ThmJob::labelCount(list))
						 + QStringLiteral(", ")
						 + tr("%n different text(s)", nullptr, int(list.size())));
	}
	m_print->setEnabled(!list.isEmpty() && currentPrinter() && !m_client->busy());
	m_save->setEnabled(!list.isEmpty() && currentPrinter());
}

	/// The material number of the article field: the item's data when an
	/// item is chosen, else what was typed
static QString articleNumber(const QComboBox *combo)
{
	const int i = combo->findText(combo->currentText());
	if (i >= 0 && !combo->itemData(i).toString().isEmpty()) return combo->itemData(i).toString();
	return combo->currentText().trimmed();
}

QString ThmPrintDialog::currentArticle() const
{
	return articleNumber(m_article);
}

void ThmPrintDialog::updateTarget()
{
	const QString material = articleNumber(m_article);
	const ThmPrinters::Choice choice = ThmPrinters::choose(m_printers, material);
	const QString previous = m_printer->currentData().toString();
	{
		const QSignalBlocker blocker(m_printer);
		m_printer->clear();
		for (int i : choice.matches) {
			const ThmPrinter &p = m_printers.at(i);
			m_printer->addItem(QStringLiteral("%1 (%2:%3)").arg(p.name, p.host).arg(p.port), p.id);
		}
		if (choice.outcome == ThmPrinters::Choice::Several) {
				//Several fit: the user decides, nothing is chosen for them
			m_printer->insertItem(0, tr("(choose a printer)"), QString());
			const int keep = m_printer->findData(previous);
			m_printer->setCurrentIndex(keep > 0 ? keep : 0);
		}
	}
	m_printer->setEnabled(!choice.matches.isEmpty());
	QString message = choice.message;
	if (const ThmPrinter *p = currentPrinter()) {
		if (ThmPrinters::changedFromUsual(*p))
			message += QStringLiteral(" ") + tr("Usually it carries %1.").arg(p->usual_material_no);
	}
	m_target_message->setText(message);
	{
		QString source, error;
		const ThmPrinter *p = currentPrinter();
		if (material.isEmpty()) m_profile_source->clear();
		else if (jobProfile(p, nullptr, &source, &error)) m_profile_source->setText(tr("Profile: %1").arg(source));
		else m_profile_source->setText(error);
	}
	m_material = ThmJob::Material();
	m_device_state->clear();
	updatePreview();
	setBusy(m_client->busy());
}

const ThmPrinter *ThmPrintDialog::currentPrinter() const
{
	const int i = ThmPrinters::indexOf(m_printers, m_printer->currentData().toString());
	return i >= 0 ? &m_printers.at(i) : nullptr;
}

void ThmPrintDialog::managePrinters()
{
	ThmPrintersDialog dialog(m_printers, m_catalog, this, m_layouts);
	if (dialog.exec() != QDialog::Accepted) return;
	m_printers = dialog.printers();
	ThmPrinters::save(m_printers);
	const QString article = m_article->currentText();
	fillArticles();
	m_article->setCurrentText(article);
	updateTarget();
}

void ThmPrintDialog::query(const QList<ThmClient::Request> &requests)
{
	const ThmPrinter *p = currentPrinter();
	if (!p) {
		m_device_state->setText(tr("Choose a printer first."));
		return;
	}
	if (m_client->busy()) return;
	m_client->setTarget(p->host, p->port);
	m_queue = requests;
	m_lines.clear();
	m_device_state->setText(tr("Asking %1 ...").arg(p->name));
	setBusy(true);
	m_client->start(m_queue.takeFirst());
}

void ThmPrintDialog::onReply(const ThmClient::Reply &reply)
{
	QString line;
	const ThmPrinter *p = currentPrinter();
	if (!reply.ok) {
		m_queue.clear();
		line = tr("%1: %2").arg(ThmClient::requestTitle(reply.request), reply.error);
	} else {
		switch (reply.request) {
			case ThmClient::Request::Version:
				line = tr("Printer: %1").arg(ThmJob::parseVersion(reply.data));
				break;
			case ThmClient::Request::Status:
				line = tr("Status: %1").arg(ThmJob::statusText(ThmJob::parseStatus(reply.data)));
				break;
			case ThmClient::Request::Material: {
				m_material = ThmJob::parseMaterial(reply.data);
				m_material_printer = p ? p->id : QString();
				line = tr("Material: %1").arg(ThmJob::materialText(m_material));
				ThmJob::Profile profile;
				if (p && jobProfile(p, &profile)) {
					const ThmJob::Fit fit = ThmJob::materialFit(m_material, profile);
					const double pitch = profile.height + profile.gap;
					if (fit == ThmJob::Fit::Fits)
						line += QStringLiteral(" – ") + tr("fits the profile (%1 mm)").arg(ThmJob::number(pitch));
					else if (fit == ThmJob::Fit::Differs)
						line += QStringLiteral(" – ") + tr("does NOT fit the profile (%1 mm)").arg(ThmJob::number(pitch));
				}
				break;
			}
			case ThmClient::Request::Job:
				line = tr("The job was sent.");
				break;
			default:
				line = tr("%1 sent.").arg(ThmClient::requestTitle(reply.request));
		}
	}
	m_lines << line;
	m_device_state->setText(m_lines.join(QLatin1Char('\n')));
	if (reply.request == ThmClient::Request::Job) {
		if (reply.ok) QMessageBox::information(this, tr("Print THM labels"), line);
		else QMessageBox::warning(this, tr("Print THM labels"),
								  tr("The job was not sent completely.\n%1").arg(reply.error));
	}
	if (!m_queue.isEmpty()) {
		m_client->start(m_queue.takeFirst());
		return;
	}
	setBusy(false);
}

void ThmPrintDialog::cancelJob()
{
	const ThmPrinter *p = currentPrinter();
	if (!p) return;
	if (QMessageBox::question(this, tr("Cancel job"),
							  tr("Cancel the current job on \"%1\" and empty its buffer?\n"
								 "Labels not yet printed are lost.").arg(p->name))
			!= QMessageBox::Yes) {
		return;
	}
	query({ThmClient::Request::Cancel});
}

void ThmPrintDialog::saveJob()
{
	const ThmPrinter *p = currentPrinter();
	if (!p) return;
	QString error;
	ThmJob::Profile profile;
	if (!jobProfile(p, &profile, nullptr, &error)) {
		QMessageBox::warning(this, tr("Print THM labels"), error);
		return;
	}
	const QByteArray job = ThmJob::build(labels(), profile, &error);
	if (job.isEmpty()) {
		QMessageBox::warning(this, tr("Print THM labels"), error);
		return;
	}
	QSettings settings;
	settings.beginGroup(settings_group);
	QString dir = settings.value("dir").toString();
	if (dir.isEmpty() && m_project && !m_project->filePath().isEmpty())
		dir = QFileInfo(m_project->filePath()).absolutePath();
	QString base = m_project ? QFileInfo(m_project->filePath()).completeBaseName() : QString();
	if (base.isEmpty()) base = tr("project");
	const QString path = QFileDialog::getSaveFileName(
		this, tr("Save THM print job"), QDir(dir).filePath(base + QStringLiteral("_THM.prn")),
		tr("Print job (*.prn)"));
	if (path.isEmpty()) return;
	settings.setValue("dir", QFileInfo(path).absolutePath());
	QSaveFile file(path);
	if (!file.open(QIODevice::WriteOnly) || file.write(job) != job.size() || !file.commit()) {
		QMessageBox::warning(this, tr("Print THM labels"),
							 tr("The file could not be written: %1").arg(file.errorString()));
		return;
	}
	remember();
}

void ThmPrintDialog::print()
{
	const ThmPrinter *p = currentPrinter();
	const QList<ThmJob::Label> list = labels();
	if (!p || list.isEmpty()) return;
	QString error, source;
	ThmJob::Profile profile;
	if (!ThmPrinters::validate(*p, &error, false) || !jobProfile(p, &profile, &source, &error)) {
		QMessageBox::warning(this, tr("Print THM labels"), error);
		return;
	}
	const QByteArray job = ThmJob::build(list, profile, &error);
	if (job.isEmpty()) {
		QMessageBox::warning(this, tr("Print THM labels"), error);
		return;
	}
		//The summary: nothing goes out before it is confirmed
	const QString article = m_article->currentText().trimmed();
	QString material_line;
	if (m_material.valid && m_material_printer == p->id) {
		material_line = tr("Material reported: %1").arg(ThmJob::materialText(m_material));
		if (ThmJob::materialFit(m_material, profile) == ThmJob::Fit::Differs)
			material_line += QStringLiteral("\n") + tr("WARNING: the label distance does not fit the profile.");
	} else {
		material_line = tr("Material not queried.");
	}
	const QString summary = tr("Target: %1 (%2:%3)%4\nArticle: %5\nQuantity: %n label(s)", nullptr,
							   ThmJob::labelCount(list))
			.arg(p->name, p->host)
			.arg(p->transport == ThmClient::Transport::Lpd ? QStringLiteral("515, LPD") : QString::number(p->port))
			.arg(p->model.isEmpty() ? QString() : QStringLiteral(", ") + p->model, article)
			+ QStringLiteral("\n") + tr("%n different text(s)", nullptr, int(list.size()))
			+ QStringLiteral("\n") + tr("Profile: %1").arg(source)
			+ QStringLiteral("\n") + material_line
			+ QStringLiteral("\n\n") + tr("Print now?");
	if (QMessageBox::question(this, tr("Print THM labels"), summary) != QMessageBox::Yes) return;
	remember();
	m_client->setTarget(p->host, p->port);
	m_client->setJobTransport(p->transport, p->lpd_queue);
	m_queue.clear();
	m_lines.clear();
	m_device_state->setText(tr("Sending to %1 ...").arg(p->name));
	setBusy(true);
	m_client->start(ThmClient::Request::Job, job);
}

void ThmPrintDialog::remember() const
{
	QSettings settings;
	settings.beginGroup(settings_group);
	settings.setValue("kinds", options().kinds);
	settings.setValue("full_designation", m_full->isChecked());
	settings.setValue("both_ends", m_both_ends->isChecked());
	settings.setValue("merge", m_merge->isChecked());
	settings.setValue("copies", m_copies->value());
	settings.setValue("article", m_article->currentText());
}

void ThmPrintDialog::setBusy(bool busy)
{
	const bool printer = currentPrinter() != nullptr;
	for (QPushButton *b : m_device_buttons) b->setEnabled(!busy && printer);
	m_print->setEnabled(!busy && printer && m_preview->rowCount() > 0);
}

// ThmPrintersDialog

ThmPrintersDialog::ThmPrintersDialog(const QList<ThmPrinter> &printers,
									 const QList<ThmCatalog::Variant> &catalog,
									 QWidget *parent,
									 const QList<ThmLayouts::Variant> &layouts) :
	QDialog(parent),
	m_printers(printers),
	m_catalog(catalog),
	m_layouts(layouts)
{
	setWindowTitle(tr("THM printers"));
	QET::trackDialogGeometry(this);
	auto *layout = new QVBoxLayout(this);
	auto *split = new QHBoxLayout;
	layout->addLayout(split, 1);

	auto *list_column = new QVBoxLayout;
	m_list = new QListWidget(this);
	list_column->addWidget(m_list, 1);
	auto *list_buttons = new QHBoxLayout;
	auto *add_button = new QPushButton(tr("Add"), this);
	auto *remove_button = new QPushButton(tr("Remove"), this);
	list_buttons->addWidget(add_button);
	list_buttons->addWidget(remove_button);
	list_column->addLayout(list_buttons);
	split->addLayout(list_column, 1);

	m_form = new QWidget(this);
	auto *form = new QFormLayout(m_form);
	m_id = new QLabel(m_form);
	m_name = new QLineEdit(m_form);
	m_model = new QLineEdit(m_form);
	m_model->setPlaceholderText(tr("as the printer reports it"));
	m_host = new QLineEdit(m_form);
	m_host->setPlaceholderText(tr("IP address or host name"));
	m_port = new QSpinBox(m_form);
	m_port->setRange(1, 65535);
	m_transport = new QComboBox(m_form);
	m_transport->addItem(tr("RawIP on the port above"), ThmClient::transportKey(ThmClient::Transport::RawIp));
	m_transport->addItem(tr("LPD (port 515, queue lp)"), ThmClient::transportKey(ThmClient::Transport::Lpd));
	m_transport->setToolTip(tr("Choose LPD when the printer takes the connection on its RawIP port but prints nothing; "
							   "status and material are still asked by RawIP."));
	m_material = new QComboBox(m_form);
	m_material->setEditable(true);
	m_material->setInsertPolicy(QComboBox::NoInsert);
	m_usual = new QComboBox(m_form);
	m_usual->setEditable(true);
	m_usual->setInsertPolicy(QComboBox::NoInsert);
	QStringList seen;
	for (const ThmCatalog::Variant &v : m_catalog) {
		seen << ThmPrinters::normalizedMaterial(v.material_no);
		m_material->addItem(ThmCatalog::title(v), v.material_no);
		m_usual->addItem(ThmCatalog::title(v), v.material_no);
	}
	for (const ThmLayouts::Variant &v : m_layouts) {
		if (seen.contains(ThmPrinters::normalizedMaterial(v.article))) continue;
		seen << ThmPrinters::normalizedMaterial(v.article);
		m_material->addItem(ThmLayouts::title(v), v.article);
		m_usual->addItem(ThmLayouts::title(v), v.article);
	}
	m_roller = new QLineEdit(m_form);
	m_roller->setPlaceholderText(tr("e.g. the colour and number of the roller fitted"));
	m_loaded_at = new QLabel(m_form);
	form->addRow(tr("ID:"), m_id);
	form->addRow(tr("Name:"), m_name);
	form->addRow(tr("Model:"), m_model);
	form->addRow(tr("Address:"), m_host);
	form->addRow(tr("Port:"), m_port);
	form->addRow(tr("Send jobs by:"), m_transport);
		//A combo of articles with a button opening the filtered choice
	const auto with_choice = [this](QComboBox *combo) {
		auto *box = new QWidget(m_form);
		auto *row = new QHBoxLayout(box);
		row->setContentsMargins(0, 0, 0, 0);
		row->addWidget(combo, 1);
		auto *button = new QPushButton(tr("..."), box);
		button->setToolTip(tr("Choose the material from the catalog, filtered by category, size and colour"));
		row->addWidget(button);
		connect(button, &QPushButton::clicked, this, [this, combo]() {
			ThmMaterialDialog dialog(ThmMaterials::entries(m_layouts, m_catalog, m_printers),
									 combo->currentData().toString(), this);
			if (dialog.exec() != QDialog::Accepted || dialog.article().isEmpty()) return;
			const int i = combo->findData(dialog.article());
			if (i >= 0) combo->setCurrentIndex(i);
			else combo->setCurrentText(dialog.article());
		});
		return box;
	};
	form->addRow(tr("Usual article:"), with_choice(m_usual));
	form->addRow(tr("Roller:"), m_roller);
	form->addRow(tr("Article variant loaded:"), with_choice(m_material));
	form->addRow(tr("Loading confirmed:"), m_loaded_at);

	m_width = millimetres(m_form);
	m_height = millimetres(m_form);
	m_gap = millimetres(m_form, 100);
	m_sensor = new QComboBox(m_form);
	for (auto s : {ThmJob::Sensor::Gap, ThmJob::Sensor::Endless,
				   ThmJob::Sensor::ReflectTop, ThmJob::Sensor::ReflectBottom}) {
		m_sensor->addItem(ThmJob::sensorTitle(s), ThmJob::sensorKey(s));
	}
	m_columns = new QSpinBox(m_form);
	m_columns->setRange(1, 50);
	m_pitch = millimetres(m_form);
	m_pitch->setSpecialValueText(tr("= label width"));
	m_margin_left = millimetres(m_form, 100);
	m_margin_right = millimetres(m_form, 100);
	m_offset_x = millimetres(m_form, 100);
	m_offset_x->setMinimum(-100);
	m_offset_y = millimetres(m_form, 100);
	m_offset_y->setMinimum(-100);
	m_font = new QSpinBox(m_form);
	m_font->setRange(-5, 9999);
	m_text_size = millimetres(m_form, 100);
	m_baseline = millimetres(m_form, 500);
	m_baseline->setSpecialValueText(tr("centred"));
	m_set_heat = new QCheckBox(tr("Set speed and heat (H command)"), m_form);
	m_speed = new QSpinBox(m_form);
		//The widest range of both families; validate() applies the
		//printer's own (from its model)
	m_speed->setRange(ThmJob::speedSteps(ThmJob::Family::MultiMark).first(),
					  ThmJob::speedSteps(ThmJob::Family::MultiMark).last());
	m_speed->setSuffix(QStringLiteral(" mm/s"));
	m_heat = new QSpinBox(m_form);
	m_heat->setRange(ThmJob::heatMin(ThmJob::Family::MultiMark), ThmJob::heatMax(ThmJob::Family::MultiMark));
	m_transfer = new QCheckBox(tr("Thermal transfer (with ribbon)"), m_form);
	m_rotate = new QCheckBox(tr("Turn the print by 180°"), m_form);
	form->addRow(tr("Label width:"), m_width);
	form->addRow(tr("Label height:"), m_height);
	form->addRow(tr("Gap:"), m_gap);
	form->addRow(tr("Sensor:"), m_sensor);
	form->addRow(tr("Columns:"), m_columns);
	form->addRow(tr("Column pitch:"), m_pitch);
	form->addRow(tr("Margin left:"), m_margin_left);
	form->addRow(tr("Margin right:"), m_margin_right);
	form->addRow(tr("Offset x:"), m_offset_x);
	form->addRow(tr("Offset y:"), m_offset_y);
	form->addRow(tr("Font number:"), m_font);
	form->addRow(tr("Text size:"), m_text_size);
	form->addRow(tr("Text baseline:"), m_baseline);
	form->addRow(m_set_heat);
	form->addRow(tr("Speed:"), m_speed);
	form->addRow(tr("Heat correction:"), m_heat);
	form->addRow(m_transfer);
	form->addRow(m_rotate);
	split->addWidget(m_form, 2);

	auto *hint = new QLabel(tr("An article found in the layout file is printed with its layout; the profile "
							   "below is for articles without one. Label sizes, font and heat are those of your "
							   "material; QElectroTech fills none in. Try a new profile on one label first."), this);
	hint->setWordWrap(true);
	layout->addWidget(hint);
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	layout->addWidget(buttons);
	connect(buttons, &QDialogButtonBox::accepted, this, &ThmPrintersDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(add_button, &QPushButton::clicked, this, &ThmPrintersDialog::add);
	connect(remove_button, &QPushButton::clicked, this, &ThmPrintersDialog::removeCurrent);
	connect(m_list, &QListWidget::currentRowChanged, this, [this](int row) {
		store();
		showPrinter(row);
	});
	connect(m_name, &QLineEdit::textEdited, this, [this](const QString &text) {
		if (QListWidgetItem *item = m_list->currentItem()) item->setText(text);
	});

	for (const ThmPrinter &p : m_printers) m_list->addItem(p.name);
	if (!m_printers.isEmpty()) m_list->setCurrentRow(0);
	else showPrinter(-1);
}

void ThmPrintersDialog::showPrinter(int row)
{
	m_row = row;
	m_form->setEnabled(row >= 0 && row < m_printers.size());
	if (!m_form->isEnabled()) return;
	m_showing = true;
	const ThmPrinter &p = m_printers.at(row);
	const ThmJob::Profile &f = p.profile;
	m_id->setText(p.id);
	m_name->setText(p.name);
	m_model->setText(p.model);
	m_host->setText(p.host);
	m_port->setValue(p.port);
	m_transport->setCurrentIndex(qMax(0, m_transport->findData(ThmClient::transportKey(p.transport))));
	const int known = m_material->findData(p.material_no);
	if (known >= 0) m_material->setCurrentIndex(known);
	else m_material->setCurrentText(p.material_no);
	const int usual = m_usual->findData(p.usual_material_no);
	if (usual >= 0) m_usual->setCurrentIndex(usual);
	else m_usual->setCurrentText(p.usual_material_no);
	m_roller->setText(p.roller);
	m_loaded_at->setText(p.loaded_at.isValid() ? QLocale().toString(p.loaded_at, QLocale::ShortFormat)
											   : tr("not yet"));
	m_width->setValue(f.width);
	m_height->setValue(f.height);
	m_gap->setValue(f.gap);
	m_sensor->setCurrentIndex(qMax(0, m_sensor->findData(ThmJob::sensorKey(f.sensor))));
	m_columns->setValue(f.columns);
	m_pitch->setValue(f.column_pitch);
	m_margin_left->setValue(f.margin_left);
	m_margin_right->setValue(f.margin_right);
	m_offset_x->setValue(f.offset_x);
	m_offset_y->setValue(f.offset_y);
	m_font->setValue(f.font);
	m_text_size->setValue(f.text_size);
	m_baseline->setValue(f.baseline);
	m_set_heat->setChecked(f.set_heat);
	m_speed->setValue(f.speed);
	m_heat->setValue(f.heat);
	m_transfer->setChecked(f.transfer);
	m_rotate->setChecked(f.rotate);
	m_showing = false;
}

void ThmPrintersDialog::store()
{
	if (m_showing || m_row < 0 || m_row >= m_printers.size()) return;
	ThmPrinter &p = m_printers[m_row];
	ThmJob::Profile &f = p.profile;
	p.name = m_name->text().trimmed();
	p.model = m_model->text().trimmed();
	p.host = m_host->text().trimmed();
	p.port = quint16(m_port->value());
	p.transport = ThmClient::transportFromKey(m_transport->currentData().toString());
	const auto chosen = [](const QComboBox *combo) {
		const int i = combo->findText(combo->currentText());
		if (i >= 0 && !combo->itemData(i).toString().isEmpty()) return combo->itemData(i).toString();
		return combo->currentText().trimmed();
	};
	const QString loaded = chosen(m_material);
	if (ThmPrinters::normalizedMaterial(loaded) != ThmPrinters::normalizedMaterial(p.material_no)) {
			//Entered here: a loading confirmed now
		ThmPrinters::confirmLoading(&p, loaded, nameOf(loaded), QDateTime::currentDateTime());
		m_loaded_at->setText(QLocale().toString(p.loaded_at, QLocale::ShortFormat));
	}
	p.usual_material_no = chosen(m_usual);
	p.usual_material_name = nameOf(p.usual_material_no);
	p.roller = m_roller->text().trimmed();
	f.width = m_width->value();
	f.height = m_height->value();
	f.gap = m_gap->value();
	ThmJob::sensorFromKey(m_sensor->currentData().toString(), &f.sensor);
	f.columns = m_columns->value();
	f.column_pitch = m_pitch->value();
	f.margin_left = m_margin_left->value();
	f.margin_right = m_margin_right->value();
	f.offset_x = m_offset_x->value();
	f.offset_y = m_offset_y->value();
	f.font = m_font->value();
	f.text_size = m_text_size->value();
	f.baseline = m_baseline->value();
	f.set_heat = m_set_heat->isChecked();
	f.speed = m_speed->value();
	f.heat = m_heat->value();
	f.transfer = m_transfer->isChecked();
	f.rotate = m_rotate->isChecked();
}

QString ThmPrintersDialog::nameOf(const QString &material_no) const
{
	const QString want = ThmPrinters::normalizedMaterial(material_no);
	for (const ThmCatalog::Variant &v : m_catalog)
		if (ThmPrinters::normalizedMaterial(v.material_no) == want) return v.name;
	for (const ThmLayouts::Variant &v : m_layouts)
		if (ThmPrinters::normalizedMaterial(v.article) == want) return v.name;
	return QString();
}

void ThmPrintersDialog::add()
{
	store();
	ThmPrinter p;
	p.id = ThmPrinters::newId(m_printers);
	p.name = tr("THM printer %1").arg(m_printers.size() + 1);
	m_printers << p;
	m_list->addItem(p.name);
	m_list->setCurrentRow(m_printers.size() - 1);
	m_name->setFocus();
}

void ThmPrintersDialog::removeCurrent()
{
	const int row = m_list->currentRow();
	if (row < 0) return;
	if (QMessageBox::question(this, tr("THM printers"),
							  tr("Remove the printer \"%1\"?").arg(m_printers.at(row).name))
			!= QMessageBox::Yes) {
		return;
	}
	m_row = -1;
	m_printers.removeAt(row);
	delete m_list->takeItem(row);
	showPrinter(m_list->currentRow());
}

void ThmPrintersDialog::accept()
{
	store();
	for (int i = 0; i < m_printers.size(); ++i) {
		QString error;
			//An article with a layout needs no own profile
		const bool own_profile = ThmLayouts::best(m_layouts, m_printers.at(i).material_no) < 0;
		if (!ThmPrinters::validate(m_printers.at(i), &error, own_profile)) {
			m_list->setCurrentRow(i);
			QMessageBox::warning(this, tr("THM printers"), error);
			return;
		}
	}
	QDialog::accept();
}
