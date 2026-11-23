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
#include "plancheckdock.h"

#include "../diagram.h"
#include "../diagramcontent.h"
#include "../diagramview.h"
#include "../qetdiagrameditor.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetproject.h"

#include <QApplication>
#include <QComboBox>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScopeGuard>
#include <QSet>
#include <QStyle>
#include <QTimer>
#include <QTreeWidget>
#include <QUndoStack>
#include <QVBoxLayout>

namespace {
	enum Column {SeverityCol, FolioCol, LabelCol, TextCol, NumberCol};
	enum Filter {All, ErrorsAndWarnings, ErrorsOnly};
		/// a run quicker than this is repeated after each change
	const qint64 kAutoLimitMs = 800;
}

PlanCheckDock::PlanCheckDock(QWidget *parent) :
	QDockWidget(tr("Check plan"), parent)
{
	setObjectName(QStringLiteral("plan_check_dock"));
	auto *body = new QWidget(this);

	m_summary = new QLabel(body);
	m_summary->setWordWrap(true);

	m_filter = new QComboBox(body);
	m_filter->addItem(tr("All findings"), All);
	m_filter->addItem(tr("Errors and warnings"), ErrorsAndWarnings);
	m_filter->addItem(tr("Errors only"), ErrorsOnly);
	m_filter->setToolTip(tr("Which findings the list shows"));
	auto *again = new QPushButton(tr("Check again"), body);
	auto *top = new QHBoxLayout;
	top->addWidget(m_filter, 1);
	top->addWidget(again);

	m_tree = new QTreeWidget(body);
	m_tree->setRootIsDecorated(false);
	m_tree->setAlternatingRowColors(true);
	m_tree->setWordWrap(true);
	m_tree->setHeaderLabels({tr("Severity"), tr("Folio"), tr("Label"), tr("Message"), tr("No.")});
	m_tree->header()->setSectionResizeMode(TextCol, QHeaderView::Stretch);
	m_tree->header()->setStretchLastSection(false);
	m_tree->setToolTip(tr("Click a finding to open its folio and select what it is about"));

	auto *box = new QVBoxLayout(body);
	box->addLayout(top);
	box->addWidget(m_summary);
	box->addWidget(m_tree, 1);
	setWidget(body);

	m_timer = new QTimer(this);
	m_timer->setSingleShot(true);
	m_timer->setInterval(1200);
	connect(m_timer, &QTimer::timeout, this, &PlanCheckDock::refresh);
	connect(again, &QPushButton::clicked, this, &PlanCheckDock::refresh);
	connect(m_filter, qOverload<int>(&QComboBox::currentIndexChanged), this, &PlanCheckDock::fill);
	connect(m_tree, &QTreeWidget::itemClicked, this, &PlanCheckDock::showRow);
	connect(m_tree, &QTreeWidget::itemActivated, this, &PlanCheckDock::showRow);
}

void PlanCheckDock::setProject(QETProject *project)
{
	if (m_project == project) return;
	if (m_project) disconnect(m_project->undoStack(), nullptr, this, nullptr);
	m_project = project;
	if (m_project)
		connect(m_project->undoStack(), &QUndoStack::indexChanged, this, &PlanCheckDock::changed);
	m_findings.clear();
	m_last_run_ms = 0;
	if (isVisible()) refresh();
	else fill();
}

void PlanCheckDock::changed()
{
		//EPLAN checks offline; here a quick run follows the drawing, a
		//slow one waits for the button
	if (isVisible() && m_last_run_ms < kAutoLimitMs) m_timer->start();
}

void PlanCheckDock::refresh()
{
	m_timer->stop();
	m_findings.clear();
	if (m_project) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		const auto restore = qScopeGuard([]() {QApplication::restoreOverrideCursor();});
		QElapsedTimer clock;
		clock.start();
		m_findings = PlanCheck::run(m_project);
		m_last_run_ms = clock.elapsed();
	}
	fill();
}

void PlanCheckDock::fill()
{
	m_tree->clear();
	if (!m_project) {
		m_summary->setText(tr("No project open."));
		return;
	}
	using PlanCheck::Severity;
	const int filter = m_filter->currentData().toInt();
	const QList<Diagram *> diagrams = m_project->diagrams();
	QStyle *st = style();
	for (int i = 0; i < m_findings.count(); ++i) {
		const PlanCheck::Finding &f = m_findings.at(i);
		if (filter == ErrorsOnly && f.severity != Severity::Error) continue;
		if (filter == ErrorsAndWarnings && f.severity == Severity::Info) continue;
		auto *row = new QTreeWidgetItem(m_tree);
		row->setData(0, Qt::UserRole, i);
		row->setText(SeverityCol, PlanCheck::severityText(f.severity));
		row->setIcon(SeverityCol, st->standardIcon(
			f.severity == Severity::Error ? QStyle::SP_MessageBoxCritical
			: f.severity == Severity::Warning ? QStyle::SP_MessageBoxWarning
			: QStyle::SP_MessageBoxInformation));
		if (f.folio >= 0 && f.folio < diagrams.count()) {
			row->setText(FolioCol, QString::number(f.folio + 1));
			row->setToolTip(FolioCol, diagrams.at(f.folio)->title());
		}
		row->setText(LabelCol, f.label);
		row->setText(TextCol, f.text);
		row->setToolTip(TextCol, f.text);
		row->setText(NumberCol, f.eplan);
		if (!f.eplan.isEmpty())
			row->setToolTip(NumberCol, tr("Number of the same message in EPLAN"));
	}
	for (int c = 0; c < m_tree->columnCount(); ++c)
		if (c != TextCol) m_tree->resizeColumnToContents(c);

	const int errors = PlanCheck::count(m_findings, Severity::Error);
	const int warnings = PlanCheck::count(m_findings, Severity::Warning);
	const int infos = PlanCheck::count(m_findings, Severity::Info);
	if (m_findings.isEmpty())
		m_summary->setText(tr("No findings: the plan is fine."));
	else
		m_summary->setText(tr("%1, %2, %3. Click a finding to show it.")
			.arg(tr("%n error(s)", nullptr, errors),
				 tr("%n warning(s)", nullptr, warnings),
				 tr("%n note(s)", nullptr, infos)));
}

void PlanCheckDock::showRow(QTreeWidgetItem *row)
{
	if (!row || !m_project) return;
	const int i = row->data(0, Qt::UserRole).toInt();
	if (i < 0 || i >= m_findings.count()) return;
	const PlanCheck::Finding &f = m_findings.at(i);
	const QList<Diagram *> diagrams = m_project->diagrams();
	const QSet<QUuid> wanted(f.items.cbegin(), f.items.cend());

		//what the finding is about, on its folio or the first one that
		//has some of it
	Diagram *target = nullptr;
	QList<QGraphicsItem *> hits;
	auto look = [&](Diagram *d) {
		for (Element *e : d->elements()) {
			if (wanted.contains(e->uuid())
				|| (wanted.isEmpty() && !f.label.isEmpty() && e->actualLabel() == f.label))
				hits << e;
		}
		for (Conductor *w : d->content().conductors())
			if (wanted.contains(w->uuid())) hits << w;
	};
	if (f.folio >= 0 && f.folio < diagrams.count()) {
		target = diagrams.at(f.folio);
		look(target);
	}
	if (hits.isEmpty() && !wanted.isEmpty()) {
		for (Diagram *d : diagrams) {
			look(d);
			if (!hits.isEmpty()) {target = d; break;}
		}
	}
	if (!target) {
		m_summary->setText(tr("This finding concerns the whole project, there is no folio to show."));
		return;
	}
	if (auto *editor = qobject_cast<QETDiagramEditor *>(parentWidget()))
		editor->activateProject(m_project);
	target->showMe();
	target->clearSelection();
	QRectF area;
	for (QGraphicsItem *gi : std::as_const(hits)) {
		if (gi->scene() != target) continue;
		gi->setSelected(true);
		area |= gi->sceneBoundingRect();
	}
	if (area.isNull()) return;
	area.adjust(-100, -100, 100, 100);
	for (QGraphicsView *view : target->views()) {
		if (auto *dv = qobject_cast<DiagramView *>(view)) dv->zoomToRect(area);
		else view->centerOn(area.center());
	}
}
