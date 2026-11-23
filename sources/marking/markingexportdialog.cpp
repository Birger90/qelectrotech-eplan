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
#include "markingexportdialog.h"

#include "../dataBase/projectdatabase.h"
#include "../diagram.h"
#include "../qet.h"
#include "../qetproject.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QSqlQuery>
#include <QVBoxLayout>

namespace {
const char *settings_group = "marking_export";

	/// The distinct values of a column of qet_locations, '' first
QStringList places(QETProject *project, const QString &column)
{
	QStringList values;
	if (!project || !project->dataBase()) return values;
	QSqlQuery q = project->dataBase()->newQuery(
		QStringLiteral("SELECT DISTINCT %1 FROM qet_locations "
					   "WHERE COALESCE(%1, '') <> '' ORDER BY %1").arg(column));
	if (q.exec()) {
		while (q.next()) values << q.value(0).toString();
	}
	return values;
}
}

MarkingExportDialog::MarkingExportDialog(QETProject *project, QWidget *parent) :
	QDialog(parent),
	m_project(project)
{
	setWindowTitle(tr("Export markers"));
	QET::trackDialogGeometry(this);
	if (m_project && m_project->dataBase()) m_project->dataBase()->updateDB();

	QSettings settings;
	settings.beginGroup(settings_group);

	auto *main_layout = new QVBoxLayout(this);

		//For which marking software
	auto *format_form = new QFormLayout;
	m_format = new QComboBox(this);
	for (auto f : {MarkingExport::PhoenixContact, MarkingExport::Weidmueller,
				   MarkingExport::Wago, MarkingExport::Neutral}) {
		m_format->addItem(MarkingExport::formatTitle(f), int(f));
	}
	MarkingExport::Format remembered = MarkingExport::PhoenixContact;
	MarkingExport::formatFromKey(settings.value("format", "phoenix").toString(), &remembered);
	m_format->setCurrentIndex(qMax(0, m_format->findData(int(remembered))));
	format_form->addRow(tr("Marking software:"), m_format);
	main_layout->addLayout(format_form);

	m_hint = new QLabel(this);
	m_hint->setWordWrap(true);
	m_hint->setStyleSheet(QStringLiteral("color: palette(mid);"));
	main_layout->addWidget(m_hint);

		//What gets a marker
	auto *kinds = new QGroupBox(tr("Markers"), this);
	auto *kinds_layout = new QHBoxLayout(kinds);
	const int kind_flags = settings.value("kinds", int(MarkingExport::AllKinds)).toInt();
	m_wires = new QCheckBox(tr("Wires"), kinds);
	m_terminals = new QCheckBox(tr("Terminals"), kinds);
	m_devices = new QCheckBox(tr("Devices"), kinds);
	m_cables = new QCheckBox(tr("Cables"), kinds);
	m_wires->setChecked(kind_flags & MarkingExport::Wire);
	m_terminals->setChecked(kind_flags & MarkingExport::Terminal);
	m_devices->setChecked(kind_flags & MarkingExport::Device);
	m_cables->setChecked(kind_flags & MarkingExport::Cable);
	for (auto *box : {m_wires, m_terminals, m_devices, m_cables}) kinds_layout->addWidget(box);
	main_layout->addWidget(kinds);

		//From where: folios and structure; never remembered, a new
		//project has other folios and places
	auto *range = new QGroupBox(tr("Range"), this);
	auto *range_form = new QFormLayout(range);
	const int folios = m_project ? m_project->diagrams().size() : 0;
	auto *folio_row = new QHBoxLayout;
	m_folio_from = new QSpinBox(range);
	m_folio_to = new QSpinBox(range);
	for (auto *spin : {m_folio_from, m_folio_to}) {
		spin->setRange(1, qMax(1, folios));
	}
	m_folio_from->setValue(1);
	m_folio_to->setValue(qMax(1, folios));
	folio_row->addWidget(m_folio_from);
	folio_row->addWidget(new QLabel(tr("to"), range));
	folio_row->addWidget(m_folio_to);
	folio_row->addStretch();
	range_form->addRow(tr("Folios:"), folio_row);
	m_plant = new QComboBox(range);
	m_plant->addItem(tr("All"), QString());
	for (const QString &p : places(m_project, QStringLiteral("plant"))) {
		m_plant->addItem(QLatin1Char('=') + p, p);
	}
	m_location = new QComboBox(range);
	m_location->addItem(tr("All"), QString());
	for (const QString &l : places(m_project, QStringLiteral("location"))) {
		m_location->addItem(QLatin1Char('+') + l, l);
	}
	range_form->addRow(tr("Plant (=):"), m_plant);
	range_form->addRow(tr("Location (+):"), m_location);
	main_layout->addWidget(range);

		//How the texts read
	m_full = new QCheckBox(tr("Full reference designation (=plant+location-device)"), this);
	m_full->setChecked(settings.value("full_designation", true).toBool());
	m_both_ends = new QCheckBox(tr("A marker at each end of wires and cables"), this);
	m_both_ends->setChecked(settings.value("both_ends", true).toBool());
	main_layout->addWidget(m_full);
	main_layout->addWidget(m_both_ends);

	m_count = new QLabel(this);
	main_layout->addWidget(m_count);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
	auto *export_button = buttons->addButton(tr("Export..."), QDialogButtonBox::AcceptRole);
	export_button->setDefault(true);
	main_layout->addWidget(buttons);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(export_button, &QPushButton::clicked, this, &MarkingExportDialog::exportFile);

	for (auto *box : {m_wires, m_terminals, m_devices, m_cables, m_full, m_both_ends}) {
		connect(box, &QCheckBox::toggled, this, &MarkingExportDialog::updateCount);
	}
	for (auto *spin : {m_folio_from, m_folio_to}) {
		connect(spin, &QSpinBox::valueChanged, this, &MarkingExportDialog::updateCount);
	}
	for (auto *combo : {m_format, m_plant, m_location}) {
		connect(combo, &QComboBox::currentIndexChanged, this, &MarkingExportDialog::updateCount);
	}
	updateCount();
}

MarkingExport::Options MarkingExportDialog::options() const
{
	MarkingExport::Options o;
	o.kinds = (m_wires->isChecked() ? MarkingExport::Wire : 0)
			  | (m_terminals->isChecked() ? MarkingExport::Terminal : 0)
			  | (m_devices->isChecked() ? MarkingExport::Device : 0)
			  | (m_cables->isChecked() ? MarkingExport::Cable : 0);
	o.folio_from = qMin(m_folio_from->value(), m_folio_to->value());
	o.folio_to = qMax(m_folio_from->value(), m_folio_to->value());
	o.plant = m_plant->currentData().toString();
	o.location = m_location->currentData().toString();
	o.full_designation = m_full->isChecked();
	o.both_ends = m_both_ends->isChecked();
	o.format = MarkingExport::Format(m_format->currentData().toInt());
	return o;
}

void MarkingExportDialog::updateCount()
{
	const MarkingExport::Options o = options();
	switch (o.format) {
		case MarkingExport::PhoenixContact:
			m_hint->setText(tr("In PROJECT complete: File > Data exchange, assign the "
							   "column \"Material\" once in the material mapping."));
			break;
		case MarkingExport::Weidmueller:
			m_hint->setText(tr("In M-Print PRO: File > Import, filter \"VK Text Format\", "
							   "separator semicolon; reference column \"Referenz\", "
							   "quantity column \"Anzahl\"."));
			break;
		case MarkingExport::Wago:
			m_hint->setText(tr("In Smart Script: import with a template and assign the "
							   "column \"Beschriftung\"; choose the marker type there."));
			break;
		default:
			m_hint->setText(tr("Every column, for any other marking software."));
	}
	if (!o.kinds) {
		m_count->setText(tr("Choose at least one kind of marker."));
		return;
	}
	int markers = 0;
	MarkingExport::write(MarkingExport::collect(m_project, o), o.format, &markers);
	m_count->setText(tr("%n marker(s) will be exported.", nullptr, markers));
}

void MarkingExportDialog::remember() const
{
	const MarkingExport::Options o = options();
	QSettings settings;
	settings.beginGroup(settings_group);
	settings.setValue("format", MarkingExport::formatKey(o.format));
	settings.setValue("kinds", o.kinds);
	settings.setValue("full_designation", o.full_designation);
	settings.setValue("both_ends", o.both_ends);
}

void MarkingExportDialog::exportFile()
{
	const MarkingExport::Options o = options();
	if (!o.kinds) {
		return;
	}
	QSettings settings;
	settings.beginGroup(settings_group);
	QString dir = settings.value("dir").toString();
	if (dir.isEmpty() && m_project && !m_project->filePath().isEmpty()) {
		dir = QFileInfo(m_project->filePath()).absolutePath();
	}
	QString base = m_project ? QFileInfo(m_project->filePath()).completeBaseName() : QString();
	if (base.isEmpty()) base = tr("project");
	const QString suggested = QDir(dir).filePath(
		base + QStringLiteral("_Beschriftung_") + MarkingExport::formatKey(o.format)
		+ QStringLiteral(".csv"));
	const QString path = QFileDialog::getSaveFileName(
		this, tr("Export markers"), suggested, MarkingExport::fileFilter(o.format));
	if (path.isEmpty()) {
		return;
	}
	settings.setValue("dir", QFileInfo(path).absolutePath());
	remember();

	QString error;
	int markers = 0;
	if (!MarkingExport::exportFile(m_project, path, o, &error, &markers)) {
		QMessageBox::warning(this, tr("Export markers"),
							 tr("The file could not be written: %1").arg(error));
		return;
	}
	QMessageBox::information(this, tr("Export markers"),
							 tr("%n marker(s) written to %1.", nullptr, markers)
								 .arg(QDir::toNativeSeparators(path)));
	accept();
}
