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

#include "../diagramview.h"
#include "../projectview.h"

#include "simulationmodel.h"
#include "simulationoverlay.h"
#include "plcprogram.h"
#include "../undocommand/changeelementinformationcommand.h"
#include "../diagram.h"
#include "../fluid/fluidtypes.h"
#include "../qetdiagrameditor.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/terminal.h"
#include "../qetproject.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QInputDialog>
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
#include <QTreeWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <cmath>
#include <functional>

SimulationDock::SimulationDock(QETDiagramEditor *editor) :
	QDockWidget(tr("Simulation"), editor),
	m_editor(editor),
	m_timer(new QTimer(this)),
	m_overlay(std::make_unique<SimulationOverlay>())
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
	// R16: step through the run
	auto *steps = new QHBoxLayout();
	m_step = new QPushButton(tr("Step 10 ms"), content);
	m_step->setToolTip(tr("Run one control cycle (10 ms) and stop"));
	m_next = new QPushButton(tr("To next change"), content);
	m_next->setToolTip(tr("Run until the next state change (valve, end position, relay, step …), at most 10 s"));
	steps->addWidget(m_step);
	steps->addWidget(m_next);
	layout->addLayout(steps);
	m_folio_only = new QCheckBox(tr("Current folio only"), content);
	m_folio_only->setToolTip(tr("Simulate only the folio shown in the editor (used at the next start)"));
	layout->addWidget(m_folio_only);
	m_program = new QPushButton(tr("PLC program..."), content);
	m_program->setToolTip(tr("Write the program a PLC of the project runs in the simulation"));
	layout->addWidget(m_program);

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

	m_vars_label = new QLabel(tr("PLC variables:"), content);
	m_vars = new QTreeWidget(content);
	m_vars->setColumnCount(2);
	m_vars->setHeaderLabels({tr("Variable"), tr("Value")});
	m_vars->setRootIsDecorated(false);
	m_vars->setSortingEnabled(true);
	m_vars->sortByColumn(0, Qt::AscendingOrder);
	m_vars->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_vars->setToolTip(tr("Current values of the PLC: inputs, outputs, flags, steps and function block outputs"));
	layout->addWidget(m_vars_label);
	layout->addWidget(m_vars, 1);
	m_vars_label->hide();
	m_vars->hide();

	m_log = new QListWidget(content);
	layout->addWidget(new QLabel(tr("Events:"), content));
	layout->addWidget(m_log, 1);
	setWidget(content);

	m_pause->setEnabled(false);
	m_stop->setEnabled(false);
	connect(m_start, &QPushButton::clicked, this, &SimulationDock::start);
	connect(m_step, &QPushButton::clicked, this, &SimulationDock::singleStep);
	connect(m_next, &QPushButton::clicked, this, &SimulationDock::toNextChange);
	connect(m_pause, &QPushButton::clicked, this, &SimulationDock::pause);
	connect(m_stop, &QPushButton::clicked, this, &SimulationDock::stop);
	connect(m_program, &QPushButton::clicked, this, &SimulationDock::editProgram);
	connect(m_timer, &QTimer::timeout, this, &SimulationDock::tick);
	connect(this, &QDockWidget::visibilityChanged, this, [this](bool visible) {
		if (!visible && m_model) stop();
	});
}

SimulationDock::~SimulationDock()
{
	clearOverlay();
}

bool SimulationDock::ensureModel()
{
	if (m_model && m_model->failed()) {
		// a run that stopped on a computing fault cannot continue: only Stop and a new start
		m_summary->setStyleSheet(QStringLiteral("color: #c00000"));
		m_summary->setText(tr("Simulation stopped: %1").arg(m_model->failure()));
		return false;
	}
	if (!m_model) {
		QETProject *project = m_editor->currentProject();
		if (!project) {
			m_summary->setText(tr("Open a project first."));
			return false;
		}
		m_project = project;
		QList<Diagram *> diagrams = project->diagrams();
		// R16: the folio shown in the editor only
		ProjectView *pv = m_editor->currentProjectView();
		DiagramView *dv = pv ? pv->currentDiagram() : nullptr;
		if (m_folio_only->isChecked() && dv && dv->diagram()) diagrams = {dv->diagram()};
		m_model = std::make_unique<SimulationModel>(diagrams, m_supply->value());
		const QStringList problems = m_model->modelErrors() + m_model->programErrors();
		m_summary->setStyleSheet(problems.isEmpty() ? QStringLiteral("color: gray") : QStringLiteral("color: #c00000"));
		m_summary->setText(problems.isEmpty() ? m_model->summary()
			: m_model->summary() + QStringLiteral("\n") + problems.join(QStringLiteral("\n")));
		if (m_model->failed()) {
			// invalid plan data: show why and start nothing (no timer, no event filters)
			m_summary->setText(tr("Simulation stopped: %1").arg(m_model->failure()));
			m_model.reset();
			m_project.clear();
			return false;
		}
		m_log->clear();
		rebuildControls();
		// push buttons and valves can also be operated by clicking their symbol
		for (Diagram *d : project->diagrams()) {
			d->installEventFilter(this);
			m_watched << QPointer<QObject>(d);
		}
	}
	m_stop->setEnabled(true);
	m_supply->setEnabled(false);
	m_folio_only->setEnabled(false);
	return true;
}

void SimulationDock::singleStep()
{
	if (!ensureModel()) return;
	if (m_timer->isActive()) pause();
	m_model->runUntil(m_model->time() + 0.010);
	afterRun();
}

void SimulationDock::toNextChange()
{
	if (!ensureModel()) return;
	if (m_timer->isActive()) pause();
	if (!m_model->runUntilChange(m_model->time() + 10.0))
		m_log->addItem(tr("%1 s  no state change within 10 s").arg(m_model->time(), 0, 'f', 3));
	afterRun();
}

void SimulationDock::afterRun()
{
	if (m_model->failed()) {
		m_summary->setStyleSheet(QStringLiteral("color: #c00000"));
		m_summary->setText(tr("Simulation stopped: %1").arg(m_model->failure()));
	}
	for (const auto &m : m_model->takeMessages())
		m_log->addItem(QStringLiteral("%1 s  %2").arg(m.time, 0, 'f', 3).arg(m.text));
	m_log->scrollToBottom();
	m_time->setText(QStringLiteral("%1 s").arg(m_model->time(), 0, 'f', 2));
	m_start->setText(tr("Continue"));
	updateVariables();
	updateOverlay();
}

void SimulationDock::start()
{
	if (!ensureModel()) return;
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
	m_folio_only->setEnabled(true);
	m_time->setText(tr("stopped"));
	m_vars->clear();
	m_var_items.clear();
	m_vars_label->hide();
	m_vars->hide();
}

void SimulationDock::updateVariables()
{
	if (!m_model) return;
	const QHash<QString, double> vars = m_model->plcVariables();
	const bool any = !vars.isEmpty();
	m_vars_label->setVisible(any);
	m_vars->setVisible(any);
	if (!any) return;
	m_vars->setSortingEnabled(false);
	for (auto it = vars.constBegin(); it != vars.constEnd(); ++it) {
		QTreeWidgetItem *item = m_var_items.value(it.key());
		if (!item) {
			item = new QTreeWidgetItem(m_vars, {it.key(), QString()});
			m_var_items.insert(it.key(), item);
		}
		const double v = it.value();
		const QString text = it.key().endsWith(QLatin1String(".ET"))
			? QStringLiteral("%1 s").arg(v, 0, 'f', 2)
			: (std::fabs(v - std::round(v)) < 1e-9 ? QString::number(qint64(std::llround(v)))
												  : QString::number(v, 'g', 6));
		if (item->text(1) != text) {
			item->setText(1, text);
			QFont f = item->font(1);
			f.setBold(v != 0.0);
			item->setFont(1, f);
			item->setFont(0, f);
		}
	}
	m_vars->setSortingEnabled(true);
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
	if (m_model->failed()) {
		pause();
		m_summary->setStyleSheet(QStringLiteral("color: #c00000"));
		m_summary->setText(tr("Simulation stopped: %1").arg(m_model->failure()));
	}
	for (const auto &m : m_model->takeMessages())
		m_log->addItem(QStringLiteral("%1 s  %2").arg(m.time, 0, 'f', 3).arg(m.text));
	m_log->scrollToBottom();
	m_time->setText(QStringLiteral("%1 s").arg(m_model->time(), 0, 'f', 2));
	updateVariables();
	updateOverlay();
}

void SimulationDock::updateOverlay()
{
	if (!m_project || !m_model) return;
	m_overlay->update(*m_model, m_project->diagrams());
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
	m_overlay->clear(!m_project.isNull());
}

/**
	Edits the program of a PLC of the current project (Structured Text, see
	PlcProgram). It is kept as the element information "sim_program" of the
	PLC module, with undo.
*/
void SimulationDock::editProgram()
{
	QETProject *project = m_editor->currentProject();
	if (!project) return;
	QList<Element *> plcs;
	QStringList names;
	for (Diagram *d : project->diagrams())
		for (Element *e : d->elements()) {
			const ElementData data = e->elementData();
			if (data.m_type == ElementData::Master && data.m_master_type == ElementData::PLC) {
				plcs << e;
				const QString lbl = e->elementInformations()[QStringLiteral("label")].toString();
				names << QStringLiteral("%1 (%2, Blatt %3)").arg(lbl.isEmpty() ? e->name() : lbl, e->name())
						 .arg(d->folioIndex() + 1);
			}
		}
	if (plcs.isEmpty()) {
		m_summary->setText(tr("The project has no PLC (module with an IO table)."));
		return;
	}
	int index = 0;
	if (plcs.size() > 1) {
		bool ok = false;
		const QString choice = QInputDialog::getItem(this, tr("PLC program"), tr("Program of which PLC?"),
													 names, 0, false, &ok);
		if (!ok) return;
		index = static_cast<int>(names.indexOf(choice));
	}
	Element *plc = plcs.at(index);
	const DiagramContext old_info = plc->elementInformations();
	QString source = old_info[QStringLiteral("sim_program")].toString();
	if (source.isEmpty())
		source = QStringLiteral("(* Strukturierter Text, z. B. eine Schrittkette:\n"
								"VAR Schritt : INT := 0; T1 : TON; END_VAR\n"
								"CASE Schritt OF\n"
								"  0: IF DI0 THEN Schritt := 1; END_IF;\n"
								"  1: DQ0 := TRUE; IF DI1 THEN Schritt := 2; END_IF;\n"
								"  2: T1(IN := TRUE, PT := T#1s);\n"
								"     IF T1.Q THEN DQ0 := FALSE; T1(IN := FALSE); Schritt := 0; END_IF;\n"
								"END_CASE; *)\n");
	bool ok = false;
	const QString text = QInputDialog::getMultiLineText(this, tr("PLC program"),
		tr("Structured Text (IEC 61131-3 subset): IF/ELSIF, CASE step sequences, INT and BOOL, "
		   "TON, TOF, CTU, CTD, R_TRIG, F_TRIG, SR, RS. Variables are the IO addresses and symbols of the IO table:"),
		source, &ok);
	if (!ok || text == source) return;
	PlcProgram check;
	QString error;
	if (!check.compile(text, &error))
		m_summary->setText(tr("Saved, but the program has an error: %1").arg(error));
	DiagramContext new_info = old_info;
	new_info.addValue(QStringLiteral("sim_program"), text);
	if (plc->diagram())
		plc->diagram()->undoStack().push(new ChangeElementInformationCommand(plc, old_info, new_info));
}
