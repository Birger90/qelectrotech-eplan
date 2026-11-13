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
#include "fluidcheckdialog.h"

#include <diagram.h>
#include <diagramview.h>
#include <fluid/fluidtypes.h>
#include <qet.h>
#include <qetgraphicsitem/element.h>
#include <qetgraphicsitem/terminal.h>
#include <qetproject.h>

#include <QApplication>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScopeGuard>
#include <QTreeWidget>
#include <QVBoxLayout>

FluidCheckDialog::FluidCheckDialog(QETProject *project, QWidget *parent) :
	QDialog(parent),
	m_project(project),
	m_tree(new QTreeWidget(this)),
	m_summary(new QLabel(this))
{
	setWindowTitle(tr("Check fluid power"));
	resize(680, 420);
	auto *layout = new QVBoxLayout(this);
	m_summary->setWordWrap(true);
	m_summary->setObjectName(QStringLiteral("fluidCheckSummary"));
	layout->addWidget(m_summary);
	auto *help = new QLabel(
		tr("Each row is a pneumatic or hydraulic port left open, a "
		   "solenoid valve without its coil or a valve coil without its "
		   "valve. Close a port with a line, or give it a silencer or a "
		   "plug in the element editor. Click a row to select the "
		   "element."),
		this);
	help->setWordWrap(true);
	layout->addWidget(help);
	m_tree->setObjectName(QStringLiteral("fluidCheckRows"));
	m_tree->setHeaderLabels({tr("Problem"), tr("Element"), tr("Folio")});
	m_tree->setRootIsDecorated(false);
	m_tree->setAlternatingRowColors(true);
	m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_tree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	layout->addWidget(m_tree);
	connect(m_tree, &QTreeWidget::itemClicked, this,
		[this](QTreeWidgetItem *row, int) { showElement(row); });
	connect(m_tree, &QTreeWidget::itemActivated, this,
		[this](QTreeWidgetItem *row, int) { showElement(row); });
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	auto *refresh_button = buttons->addButton(tr("Refresh"), QDialogButtonBox::ActionRole);
	connect(refresh_button, &QPushButton::clicked, this, &FluidCheckDialog::refresh);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttons);
	if (project) {
		connect(project, &QObject::destroyed, this, &QDialog::close);
	}
	QET::trackDialogGeometry(this);
	refresh();
}

void FluidCheckDialog::refresh()
{
	m_elements.clear();
	m_tree->clear();
	if (!m_project) {
		return;
	}
		//Links and lines reach across folios: every folio is read
	if (m_project->unloadedFolioCount()) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		const auto restore = qScopeGuard([]() { QApplication::restoreOverrideCursor(); });
		m_project->loadFolios();
	}

	auto add_row = [this](const QString &problem, Element *element,
						  Diagram *diagram) {
		auto *row = new QTreeWidgetItem(m_tree);
		row->setText(0, problem);
		const QString label = element->actualLabel();
		row->setText(1, label.isEmpty() ? element->name() : label);
		row->setText(2, tr("%1 — %2")
			.arg(m_project->folioIndex(diagram) + 1)
			.arg(diagram->title()));
		m_elements.insert(row, element);
	};

	for (Diagram *diagram : m_project->diagrams()) {
		for (Element *element : diagram->elements()) {
			for (Terminal *terminal : element->terminals()) {
				if (Fluid::isOpenPort(terminal->medium(),
									  terminal->portEnd(),
									  terminal->conductorsCount())) {
					const QString port = terminal->name();
					add_row(port.isEmpty()
								? tr("Open port")
								: tr("Open port %1").arg(port),
							element, diagram);
				}
			}

			const FluidData fluid = element->elementData().m_fluid;
			const int linked = element->linkedElements().size();
			const int sides = fluid.solenoidSides();
			if (sides > linked) {
				add_row(tr("%n solenoid(s) without a coil", nullptr,
						   sides - linked),
						element, diagram);
			}
			if (fluid.m_kind == FluidData::Kind::Solenoid && linked == 0) {
				add_row(tr("Coil linked to no valve"), element, diagram);
			}
		}
	}

	const int count = m_tree->topLevelItemCount();
	if (count == 0) {
		m_summary->setText(tr("No open fluid port, every solenoid has its coil."));
	} else {
		m_summary->setText(tr("%n problem(s) found.", nullptr, count));
	}
}

void FluidCheckDialog::showElement(QTreeWidgetItem *row)
{
	if (!m_project || !m_elements.contains(row)) {
		return;
	}
	Element *element = m_elements.value(row);
	Diagram *diagram = element ? element->diagram() : nullptr;
	if (!diagram || m_project->folioIndex(diagram) < 0) {
		m_summary->setText(tr("This element no longer exists. Refresh the results."));
		return;
	}
	diagram->showMe();
	diagram->clearSelection();
	element->setSelected(true);
	const QRectF bounds =
		element->sceneBoundingRect().adjusted(-100, -100, 100, 100);
	for (QGraphicsView *view : diagram->views()) {
		if (auto *diagram_view = qobject_cast<DiagramView *>(view)) {
			diagram_view->zoomToRect(bounds);
		}
	}
}
