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
#include "simulationdock.h"

#include "simulationmodel.h"
#include "../diagram.h"
#include "../fluid/fluidtypes.h"
#include "../qetdiagrameditor.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/terminal.h"
#include "../qetproject.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPen>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include <cmath>
#include <functional>

SimulationDock::SimulationDock(QETDiagramEditor *editor) :
	QDockWidget(tr("Simulation"), editor),
	m_editor(editor),
	m_timer(new QTimer(this))
{
	setObjectName(QStringLiteral("simulation_dock"));
	auto *content = new QWidget(this);
	auto *layout = new QVBoxLayout(content);

	auto *buttons = new QHBoxLayout();
	m_start = new QPushButton(tr("Start"), content);
	m_pause = new QPushButton(tr("Pause"), content);
	m_stop = new QPushButton(tr("Stop"), content);
	m_start->setToolTip(tr("Simulate the current project: pneumatics, hydraulics and the electric control"));
	m_stop->setToolTip(tr("End the simulation and remove its display from the folios"));
	buttons->addWidget(m_start);
	buttons->addWidget(m_pause);
	buttons->addWidget(m_stop);
	layout->addLayout(buttons);

	auto *form = new QFormLayout();
	m_speed = new QComboBox(content);
	m_speed->addItem(tr("1/10 (slow motion)"), 0.1);
	m_speed->addItem(tr("1/4"), 0.25);
	m_speed->addItem(tr("1/2"), 0.5);
	m_speed->addItem(tr("real time"), 1.0);
	m_speed->setCurrentIndex(2);
	m_supply = new QDoubleSpinBox(content);
	m_supply->setRange(0.5, 16.0);
	m_supply->setDecimals(1);
	m_supply->setValue(6.0);
	m_supply->setSuffix(tr(" bar"));
	m_supply->setToolTip(tr("Pressure of the compressed air sources, used at the next start"));
	m_time = new QLabel(tr("stopped"), content);
	form->addRow(tr("Speed:"), m_speed);
	form->addRow(tr("Air supply:"), m_supply);
	form->addRow(tr("Time:"), m_time);
	layout->addLayout(form);

	m_summary = new QLabel(content);
	m_summary->setWordWrap(true);
	m_summary->setStyleSheet(QStringLiteral("color: gray"));
	layout->addWidget(m_summary);

	auto *scroll = new QScrollArea(content);
	scroll->setWidgetResizable(true);
	m_controls = new QWidget(scroll);
	m_controls_layout = new QVBoxLayout(m_controls);
	m_controls_layout->addStretch();
	scroll->setWidget(m_controls);
	layout->addWidget(new QLabel(tr("Operate (hold the button):"), content));
	layout->addWidget(scroll, 1);

	m_log = new QListWidget(content);
	layout->addWidget(new QLabel(tr("Events:"), content));
	layout->addWidget(m_log, 1);
	setWidget(content);

	m_pause->setEnabled(false);
	m_stop->setEnabled(false);
	connect(m_start, &QPushButton::clicked, this, &SimulationDock::start);
	connect(m_pause, &QPushButton::clicked, this, &SimulationDock::pause);
	connect(m_stop, &QPushButton::clicked, this, &SimulationDock::stop);
	connect(m_timer, &QTimer::timeout, this, &SimulationDock::tick);
	connect(this, &QDockWidget::visibilityChanged, this, [this](bool visible) {
		if (!visible && m_model) stop();
	});
}

SimulationDock::~SimulationDock()
{
	clearOverlay();
}

void SimulationDock::start()
{
	if (!m_model) {
		QETProject *project = m_editor->currentProject();
		if (!project) {
			m_summary->setText(tr("Open a project first."));
			return;
		}
		m_project = project;
		m_model = std::make_unique<SimulationModel>(project->diagrams(), m_supply->value());
		m_summary->setText(m_model->summary());
		m_log->clear();
		rebuildControls();
		// push buttons and valves can also be operated by clicking their symbol
		for (Diagram *d : project->diagrams()) {
			d->installEventFilter(this);
			m_watched << QPointer<QObject>(d);
		}
	}
	m_clock.restart();
	m_timer->start(30);
	m_start->setEnabled(false);
	m_pause->setEnabled(true);
	m_stop->setEnabled(true);
	m_supply->setEnabled(false);
}

void SimulationDock::pause()
{
	m_timer->stop();
	m_start->setEnabled(true);
	m_start->setText(tr("Continue"));
	m_pause->setEnabled(false);
}

void SimulationDock::stop()
{
	m_timer->stop();
	for (const QPointer<QObject> &w : std::as_const(m_watched))
		if (w) w->removeEventFilter(this);
	m_watched.clear();
	m_held_button.clear();
	m_held_valve.clear();
	clearOverlay();
	m_model.reset();
	m_project.clear();
	while (m_controls_layout->count() > 1) {
		QLayoutItem *item = m_controls_layout->takeAt(0);
		delete item->widget();
		delete item;
	}
	m_start->setEnabled(true);
	m_start->setText(tr("Start"));
	m_pause->setEnabled(false);
	m_stop->setEnabled(false);
	m_supply->setEnabled(true);
	m_time->setText(tr("stopped"));
}

void SimulationDock::rebuildControls()
{
	auto add_hold = [this](const QString &text, const std::function<void(bool)> &act) {
		auto *b = new QPushButton(text, m_controls);
		connect(b, &QPushButton::pressed, this, [act]() { act(true); });
		connect(b, &QPushButton::released, this, [act]() { act(false); });
		m_controls_layout->insertWidget(m_controls_layout->count() - 1, b);
	};
	for (const QString &label : m_model->buttons())
		add_hold(tr("Push button %1").arg(label), [this, label](bool on) {
			if (m_model) m_model->pressButton(label, on);
		});
	for (const QString &label : m_model->valves()) {
		auto *row = new QWidget(m_controls);
		auto *h = new QHBoxLayout(row);
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(new QLabel(tr("Valve %1 by hand:").arg(label), row));
		for (bool left : {true, false}) {
			auto *b = new QPushButton(left ? tr("left") : tr("right"), row);
			connect(b, &QPushButton::pressed, this, [this, label, left]() {
				if (m_model) m_model->actuateValve(label, left, true);
			});
			connect(b, &QPushButton::released, this, [this, label, left]() {
				if (m_model) m_model->actuateValve(label, left, false);
			});
			h->addWidget(b);
		}
		m_controls_layout->insertWidget(m_controls_layout->count() - 1, row);
	}
}

void SimulationDock::tick()
{
	if (!m_model || !m_project) {
		stop();
		return;
	}
	const double speed = m_speed->currentData().toDouble();
	const double real = m_clock.restart() / 1000.0;
	const double target = m_model->time() + std::min(real * speed, 0.05);
	m_model->runUntil(target);
	for (const auto &m : m_model->takeMessages())
		m_log->addItem(QStringLiteral("%1 s  %2").arg(m.time, 0, 'f', 3).arg(m.text));
	m_log->scrollToBottom();
	m_time->setText(QStringLiteral("%1 s").arg(m_model->time(), 0, 'f', 2));
	updateOverlay();
}

void SimulationDock::updateOverlay()
{
	if (!m_project) return;
	for (Diagram *d : m_project->diagrams()) {
		for (Conductor *c : d->conductors()) {
			Terminal *t = c->terminal1;
			QColor colour;
			double width = 3.0;
			if (m_model->isFluid(t)) {
				const double bar = m_model->pressureBar(t);
				const bool oil = t->medium() == Fluid::Medium::Hydraulic;
				if (!std::isnan(bar) && bar > 0.2) {
					const double share = std::min(1.0, bar / (oil ? 60.0 : 6.0));
					colour = oil ? QColor(220, 40, 30) : QColor(0, 90, 220);
					colour.setAlpha(int(70 + 160 * share));
					width = 2.0 + 3.0 * share;
				}
			} else if (m_model->isElectric(t) && m_model->energized(t)) {
				colour = QColor(230, 0, 0, 200);
				width = 2.5;
			}
			QGraphicsItem *&item = m_wire_items[c];
			if (!colour.isValid()) {
				if (item) item->setVisible(false);
				continue;
			}
			auto *path = static_cast<QGraphicsPathItem *>(item);
			if (!path) {
				path = new QGraphicsPathItem();
				path->setZValue(10000);
				path->setAcceptedMouseButtons(Qt::NoButton);
				d->addItem(path);
				item = path;
			}
			path->setPath(c->mapToScene(c->path()));
			path->setPen(QPen(colour, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
			path->setVisible(true);
		}
	}
	auto text_at = [this](Element *e, const QString &text) {
		if (!e || !e->scene()) return;
		QGraphicsItem *&item = m_element_items[e];
		auto *label = static_cast<QGraphicsSimpleTextItem *>(item);
		if (!label) {
			label = new QGraphicsSimpleTextItem();
			label->setZValue(10001);
			label->setBrush(QColor(0, 120, 120));
			label->setAcceptedMouseButtons(Qt::NoButton);
			e->scene()->addItem(label);
			item = label;
		}
		label->setText(text);
		const QRectF r = e->sceneBoundingRect();
		label->setPos(r.left(), r.top() - 14);
	};
	for (const auto &c : m_model->cylinders())
		text_at(c.element, QStringLiteral("%1: %2 / %3 mm%4").arg(c.label)
				.arg(c.x_mm, 0, 'f', 0).arg(c.stroke_mm, 0, 'f', 0)
				.arg(c.v > 1e-3 ? QStringLiteral("  >>") : (c.v < -1e-3 ? QStringLiteral("  <<") : QString())));
	for (const auto &v : m_model->valveStates())
		text_at(v.element, tr("%1: position %2").arg(v.label).arg(v.position));
}

/**
	While the simulation runs, pressing the mouse on a push button presses
	it until the mouse is released; on a directional valve it actuates the
	side clicked (left or right half) the same way, like a manual override.
	Those clicks do not select or move the symbol.
*/
bool SimulationDock::eventFilter(QObject *watched, QEvent *event)
{
	if (!m_model || !m_timer->isActive())
		return QDockWidget::eventFilter(watched, event);
	const auto type = event->type();
	if (type == QEvent::GraphicsSceneMouseRelease) {
		if (!m_held_button.isEmpty()) {
			m_model->pressButton(m_held_button, false);
			m_held_button.clear();
			return true;
		}
		if (!m_held_valve.isEmpty()) {
			m_model->actuateValve(m_held_valve, m_held_left, false);
			m_held_valve.clear();
			return true;
		}
		return QDockWidget::eventFilter(watched, event);
	}
	if (type != QEvent::GraphicsSceneMousePress && type != QEvent::GraphicsSceneMouseDoubleClick)
		return QDockWidget::eventFilter(watched, event);
	auto *scene = qobject_cast<QGraphicsScene *>(watched);
	auto *me = static_cast<QGraphicsSceneMouseEvent *>(event);
	if (!scene || me->button() != Qt::LeftButton)
		return QDockWidget::eventFilter(watched, event);
	for (QGraphicsItem *item : scene->items(me->scenePos())) {
		for (QGraphicsItem *it = item; it; it = it->parentItem()) {
			auto *element = qgraphicsitem_cast<Element *>(it);
			if (!element) continue;
			const QString label = element->elementInformations()[QStringLiteral("label")].toString();
			if (label.isEmpty()) break;
			if (m_model->buttons().contains(label)) {
				if (type == QEvent::GraphicsSceneMousePress) {
					m_model->pressButton(label, true);
					m_held_button = label;
				}
				return true;
			}
			if (m_model->valves().contains(label)) {
				if (type == QEvent::GraphicsSceneMousePress) {
					m_held_left = me->scenePos().x() < element->sceneBoundingRect().center().x();
					m_model->actuateValve(label, m_held_left, true);
					m_held_valve = label;
				}
				return true;
			}
			break;
		}
	}
	return QDockWidget::eventFilter(watched, event);
}

void SimulationDock::clearOverlay()
{
	if (!m_project) {          // the folios are gone, and with them every overlay item
		m_wire_items.clear();
		m_element_items.clear();
		return;
	}
	for (QGraphicsItem *item : std::as_const(m_wire_items)) {
		if (!item) continue;
		if (item->scene()) item->scene()->removeItem(item);
		delete item;
	}
	for (QGraphicsItem *item : std::as_const(m_element_items)) {
		if (!item) continue;
		if (item->scene()) item->scene()->removeItem(item);
		delete item;
	}
	m_wire_items.clear();
	m_element_items.clear();
}
