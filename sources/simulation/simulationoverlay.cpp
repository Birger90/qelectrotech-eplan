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
#include "simulationoverlay.h"

#include "simulationmodel.h"
#include "../diagram.h"
#include "../fluid/fluidtypes.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/terminal.h"

#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>
#include <QPen>
#include <cmath>

SimulationOverlay::~SimulationOverlay()
{
	clear(false);
}

void SimulationOverlay::update(const SimulationModel &model, const QList<Diagram *> &diagrams)
{
	for (Diagram *d : diagrams) {
		for (Conductor *c : d->conductors()) {
			Terminal *t = c->terminal1;
			QColor colour;
			double width = 3.0;
			if (model.isFluid(t)) {
				const double bar = model.pressureBar(t);
				const bool oil = t->medium() == Fluid::Medium::Hydraulic;
				if (!std::isnan(bar) && bar > 0.2) {
					const double share = std::min(1.0, bar / (oil ? 60.0 : 6.0));
					colour = oil ? QColor(220, 40, 30) : QColor(0, 90, 220);
					colour.setAlpha(int(70 + 160 * share));
					width = 2.0 + 3.0 * share;
				}
			} else if (model.isElectric(t) && model.energized(t)) {
				colour = QColor(230, 0, 0, 200);
				width = 2.5;
			}
			QGraphicsItem *&item = m_wires[c];
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
	for (const auto &state : model.elementStates())
		text(state.first, state.second);
}

void SimulationOverlay::text(Element *e, const QString &text)
{
	if (!e || !e->scene()) return;
	QGraphicsItem *&item = m_texts[e];
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
	label->setVisible(!text.isEmpty());
	const QRectF r = e->sceneBoundingRect();
	// right of the symbol, so the state does not cover its label above it
	label->setPos(r.right() + 4, r.top());
}

void SimulationOverlay::clear(bool scenes_alive)
{
	if (scenes_alive) {
		for (QGraphicsItem *item : std::as_const(m_wires)) {
			if (!item) continue;
			if (item->scene()) item->scene()->removeItem(item);
			delete item;
		}
		for (QGraphicsItem *item : std::as_const(m_texts)) {
			if (!item) continue;
			if (item->scene()) item->scene()->removeItem(item);
			delete item;
		}
	}
	m_wires.clear();
	m_texts.clear();
}
