/*
	Copyright 2006-2026 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech. If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-2.0-or-later
#include "tiatagsdialog.h"

#include <diagram.h>
#include <diagramview.h>
#include <qet.h>
#include <qetgraphicsitem/element.h>
#include <qetproject.h>

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QScopeGuard>
#include <QSettings>
#include <QTreeWidget>
#include <QUndoCommand>
#include <QUndoStack>
#include <QVBoxLayout>

using TiaPlcTags::Status;

namespace {
enum Column { StatusCol, AddressCol, NameCol, TiaNameCol, TypeCol, CommentCol, TiaCommentCol, ModuleCol, FolioCol };
const int ChannelRole = Qt::UserRole + 1;

QString lastDir(QETProject *project)
{
	QSettings settings;
	const QString dir = settings.value(QStringLiteral("tia_tags/last_dir")).toString();
	if (!dir.isEmpty()) return dir;
	return project && !project->filePath().isEmpty() ? QFileInfo(project->filePath()).absolutePath()
													  : QString();
}
}

TiaTagsDialog::TiaTagsDialog(QETProject *project, QWidget *parent) :
	QDialog(parent),
	m_project(project),
	m_summary(new QLabel(this)),
	m_tree(new QTreeWidget(this)),
	m_only_differences(new QCheckBox(tr("Only differences"), this))
{
	setWindowTitle(tr("PLC tags and TIA Portal"));
	resize(1100, 560);
	auto *layout = new QVBoxLayout(this);
	m_summary->setWordWrap(true);
	m_summary->setObjectName(QStringLiteral("tiaTagsSummary"));
	layout->addWidget(m_summary);
	auto *help = new QLabel(
		tr("Each row is a channel of a PLC module as it goes to TIA Portal. "
		   "\"Export for TIA…\" writes the tag table (.xlsx: open the tag "
		   "table in TIA Portal and click Import). \"Compare with TIA "
		   "table…\" reads a tag table exported from TIA Portal and shows "
		   "what differs; the TIA names and comments can then be taken "
		   "into the plan. Click a row to show the channel."),
		this);
	help->setWordWrap(true);
	layout->addWidget(help);

	m_tree->setObjectName(QStringLiteral("tiaTagsRows"));
	m_tree->setHeaderLabels({tr("Status"), tr("Address"), tr("Name (plan)"), tr("Name (TIA)"),
							 tr("Data type"), tr("Comment (plan)"), tr("Comment (TIA)"),
							 tr("Module"), tr("Folio")});
	m_tree->setRootIsDecorated(false);
	m_tree->setAlternatingRowColors(true);
	m_tree->setSortingEnabled(true);
	m_tree->header()->setSectionResizeMode(QHeaderView::Interactive);
	layout->addWidget(m_tree);
	connect(m_tree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *row, int) { showRow(row); });
	connect(m_tree, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem *row, int) { showRow(row); });

	auto *row = new QHBoxLayout();
	m_only_differences->setChecked(true);
	connect(m_only_differences, &QCheckBox::toggled, this, &TiaTagsDialog::fill);
	row->addWidget(m_only_differences);
	row->addStretch();
	layout->addLayout(row);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	auto *export_button = buttons->addButton(tr("Export for TIA…"), QDialogButtonBox::ActionRole);
	auto *compare_button = buttons->addButton(tr("Compare with TIA table…"), QDialogButtonBox::ActionRole);
	m_take_over = buttons->addButton(tr("Take over names and comments"), QDialogButtonBox::ActionRole);
	m_report = buttons->addButton(tr("Save report…"), QDialogButtonBox::ActionRole);
	export_button->setObjectName(QStringLiteral("tiaTagsExport"));
	compare_button->setObjectName(QStringLiteral("tiaTagsCompare"));
	m_take_over->setObjectName(QStringLiteral("tiaTagsTakeOver"));
	connect(export_button, &QPushButton::clicked, this, &TiaTagsDialog::exportTags);
	connect(compare_button, &QPushButton::clicked, this, &TiaTagsDialog::compareWith);
	connect(m_take_over, &QPushButton::clicked, this, &TiaTagsDialog::takeOver);
	connect(m_report, &QPushButton::clicked, this, &TiaTagsDialog::saveReport);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttons);
	if (project) connect(project, &QObject::destroyed, this, &QDialog::close);
	QET::trackDialogGeometry(this);
	refresh();
}

/// channels of the project again; compared again with the last TIA table
void TiaTagsDialog::refresh()
{
	if (!m_project) return;
	QApplication::setOverrideCursor(Qt::WaitCursor);
	const auto restore = qScopeGuard([]() { QApplication::restoreOverrideCursor(); });
	m_notes.clear();
	const auto channels = TiaPlcTags::channels(m_project, &m_notes);
	QVector<TiaPlcTags::Tag> tags;
	if (m_compared) {
		QString error;
		tags = TiaPlcTags::read(m_tia_file, &error);
		if (!error.isEmpty()) m_notes << error;
	}
	m_cmp = TiaPlcTags::compare(channels, tags);
	fill();
}

void TiaTagsDialog::fill()
{
	m_tree->setSortingEnabled(false);
	m_tree->clear();
	int applicable = 0, counted = 0, exported = 0;
	for (int i = 0; i < m_cmp.rows.size(); ++i) {
		const auto &d = m_cmp.rows.at(i);
		const TiaPlcTags::Channel *c = d.channel >= 0 ? &m_cmp.channels.at(d.channel) : nullptr;
		const TiaPlcTags::Tag *t = d.tag >= 0 ? &m_cmp.tags.at(d.tag) : nullptr;
		if (c) {
			++exported;
			if (c->autoAddress) ++counted;
		}
		const bool differs = m_compared && d.status != Status::Same;
		if (m_compared && t && c && d.status != Status::Same && d.status != Status::AddressDiffers)
			++applicable;
		if (m_compared && m_only_differences->isChecked() && !differs) continue;

		auto *item = new QTreeWidgetItem(m_tree);
		item->setData(StatusCol, ChannelRole, i);
		item->setText(StatusCol, m_compared ? TiaPlcTags::statusText(d.status) : tr("not compared"));
		QString address = c ? c->address : TiaPlcTags::normalizedAddress(t->address);
		if (c && t && d.status == Status::AddressDiffers)
			address = tr("%1 (TIA: %2)").arg(c->address, TiaPlcTags::normalizedAddress(t->address));
		item->setText(AddressCol, address);
		if (c && c->autoAddress)
			item->setToolTip(AddressCol, tr("Counted from the module's start address (channel %1); set the "
											"element information plc_start_address / plc_start_address_2 of "
											"the module for other addresses.").arg(c->rawAddress));
		item->setText(NameCol, c ? c->name : QString());
		item->setText(TiaNameCol, t ? t->name : QString());
		item->setText(TypeCol, c ? c->dataType : t->dataType);
		item->setText(CommentCol, c ? c->function : QString());
		item->setText(TiaCommentCol, t ? t->comment : QString());
		item->setText(ModuleCol, c ? c->module : QString());
		item->setText(FolioCol, c && c->folio >= 0 ? QString::number(c->folio + 1) : QString());
		if (differs) {
			const QColor colour = d.status == Status::MissingInTia || d.status == Status::OnlyInTia
				? QColor(255, 225, 200) : QColor(255, 245, 190);
			for (int col = 0; col <= FolioCol; ++col) item->setBackground(col, colour);
		}
	}
	m_tree->setSortingEnabled(true);
	m_tree->sortByColumn(AddressCol, Qt::AscendingOrder);
	for (int col = 0; col <= FolioCol; ++col) m_tree->resizeColumnToContents(col);

	QString text;
	if (!m_compared) {
		text = tr("%n channel(s) of PLC modules, ", nullptr, exported)
			 + tr("%n with an address counted from the module start.", nullptr, counted);
	} else {
		text = tr("Compared with %1: %2 the same, %3 name differs, %4 comment differs, %5 address differs, "
				  "%6 missing in TIA, %7 only in TIA.")
				   .arg(QFileInfo(m_tia_file).fileName())
				   .arg(m_cmp.count(Status::Same))
				   .arg(m_cmp.count(Status::NameDiffers) + m_cmp.count(Status::BothDiffer))
				   .arg(m_cmp.count(Status::CommentDiffers) + m_cmp.count(Status::BothDiffer))
				   .arg(m_cmp.count(Status::AddressDiffers))
				   .arg(m_cmp.count(Status::MissingInTia))
				   .arg(m_cmp.count(Status::OnlyInTia));
		if (m_cmp.otherTags)
			text += QLatin1Char(' ') + tr("%n tag(s) without I/Q address (memory, data blocks) not compared.",
										 nullptr, m_cmp.otherTags);
	}
	if (!m_notes.isEmpty())
		text += QStringLiteral("\n") + m_notes.join(QLatin1Char('\n'));
	m_summary->setText(text);
	m_take_over->setEnabled(applicable > 0 && m_project && !m_project->isReadOnly());
	m_report->setEnabled(m_compared);
}

void TiaTagsDialog::exportTags()
{
	if (!m_project) return;
	QString base = QFileInfo(m_project->filePath()).completeBaseName();
	if (base.isEmpty()) base = QStringLiteral("QET");
	QString filter;
	const QString path = QFileDialog::getSaveFileName(
		this, tr("Export PLC tags for TIA Portal"),
		lastDir(m_project) + QLatin1Char('/') + base + QStringLiteral("_PLC_Tags.xlsx"),
		tr("TIA Portal tag table (*.xlsx);;TIA Portal Openness (*.xml)"), &filter);
	if (path.isEmpty()) return;
	QSettings().setValue(QStringLiteral("tia_tags/last_dir"), QFileInfo(path).absolutePath());
	QString error;
	if (!TiaPlcTags::write(path, m_cmp.channels, QString(), QString(), &error)) {
		QMessageBox::warning(this, windowTitle(), tr("Could not write %1:\n%2").arg(path, error));
		return;
	}
	int written = 0;
	for (const auto &c : m_cmp.channels) if (!c.address.isEmpty()) ++written;
	QMessageBox::information(
		this, windowTitle(),
		tr("%n PLC tag(s) written to %1.", nullptr, written).arg(QDir::toNativeSeparators(path))
			+ QStringLiteral("\n\n")
			+ (path.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive)
				   ? tr("Import it with TIA Portal Openness into the PLC tags of the CPU.")
				   : tr("In TIA Portal open the tag table of the CPU (PLC tags) and click Import.")));
}

void TiaTagsDialog::compareWith()
{
	const QString path = QFileDialog::getOpenFileName(
		this, tr("Compare with TIA tag table"), lastDir(m_project),
		tr("TIA Portal tag tables (*.xlsx *.xml)"));
	if (path.isEmpty()) return;
	QSettings().setValue(QStringLiteral("tia_tags/last_dir"), QFileInfo(path).absolutePath());
	QString error;
	const auto tags = TiaPlcTags::read(path, &error);
	if (tags.isEmpty()) {
		QMessageBox::warning(this, windowTitle(),
							 error.isEmpty() ? tr("%1 has no PLC tags.").arg(path) : error);
		return;
	}
	m_tia_file = path;
	m_compared = true;
	refresh();
}

void TiaTagsDialog::takeOver()
{
	if (!m_project || m_project->isReadOnly()) return;
	auto *undo = new QUndoCommand(tr("Take over PLC tags from TIA Portal"));
	const int rows = TiaPlcTags::apply(m_cmp, undo);
	if (undo->childCount() == 0) {
		delete undo;
		return;
	}
	m_project->undoStack()->push(undo);
	refresh();
	m_summary->setText(tr("%n channel(s) took name and comment from TIA Portal (undo with Ctrl+Z).",
						  nullptr, rows) + QStringLiteral("\n") + m_summary->text());
}

void TiaTagsDialog::saveReport()
{
	const QString path = QFileDialog::getSaveFileName(
		this, tr("Save comparison"), lastDir(m_project) + QStringLiteral("/TIA_Abgleich.csv"),
		tr("CSV (*.csv)"));
	if (path.isEmpty()) return;
	QSaveFile file(path);
	bool ok = file.open(QIODevice::WriteOnly);
	if (ok) {
		file.write(TiaPlcTags::reportCsv(m_cmp).toUtf8());
		ok = file.commit();
	}
	if (!ok) QMessageBox::warning(this, windowTitle(), tr("Could not write %1.").arg(path));
}

void TiaTagsDialog::showRow(QTreeWidgetItem *row)
{
	if (!m_project || !row) return;
	const int i = row->data(StatusCol, ChannelRole).toInt();
	if (i < 0 || i >= m_cmp.rows.size()) return;
	const int c = m_cmp.rows.at(i).channel;
	if (c < 0) return;
	Element *element = m_cmp.channels.at(c).slave ? m_cmp.channels.at(c).slave.data()
												  : m_cmp.channels.at(c).master.data();
	Diagram *diagram = element ? element->diagram() : nullptr;
	if (!diagram || m_project->folioIndex(diagram) < 0) return;
	diagram->showMe();
	diagram->clearSelection();
	element->setSelected(true);
	const QRectF bounds = element->sceneBoundingRect().adjusted(-100, -100, 100, 100);
	for (QGraphicsView *view : diagram->views()) {
		if (auto *diagram_view = qobject_cast<DiagramView *>(view)) diagram_view->zoomToRect(bounds);
	}
}
