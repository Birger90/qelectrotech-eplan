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
#include "../ElementsCollection/elementslocation.h"
#include "../factory/elementpicturefactory.h"
#include "../fluid/fluidtypes.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/terminal.h"

#include <QDomDocument>
#include <QDomElement>
#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>
#include <QPainterPath>
#include <QPen>
#include <QPicture>
#include <QPointer>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <cmath>
#include <limits>

// ------------------------------------------------------------------------- moving symbols
/**
	How a symbol moves while the simulation runs: the parts of its
	description that travel (piston and rod, the fields of a valve), the
	direction in the symbol's own coordinates and the way per unit (the
	free stroke of the piston in px, the width of a field).
*/
struct SimulationOverlay::Pose {
	QPointer<Element> element;
	QDomElement description;      // the symbol's own, read only
	QSet<int> moving;             // indexes of the parts of the description that travel
	QHash<int, int> stretch;      // lines from a pilot or coil port: only this end (1, 2) travels
	QPointF axis;                 // unit vector of the travel
	double way = 0;               // px per unit of the value
	double last = std::numeric_limits<double>::quiet_NaN();
	bool ok = false;
};

namespace {
struct Seg { double u1, v1, u2, v2; };   // a straight edge in (u along the travel, v across)
struct Port { QString name; double u, v; QString orientation; bool fluid; };

double attr(const QDomElement &e, const QString &name) { return e.attribute(name).toDouble(); }

	/// the straight edges of a part; @a horizontal: u = x, else u = y
QList<Seg> edges(const QDomElement &part, bool horizontal)
{
	QList<QPointF> pts;
	bool closed = false;
	const QString tag = part.tagName();
	if (tag == QLatin1String("line")) {
		pts << QPointF(attr(part, "x1"), attr(part, "y1")) << QPointF(attr(part, "x2"), attr(part, "y2"));
	} else if (tag == QLatin1String("rect")) {
		const double x = attr(part, "x"), y = attr(part, "y"), w = attr(part, "width"), h = attr(part, "height");
		pts << QPointF(x, y) << QPointF(x + w, y) << QPointF(x + w, y + h) << QPointF(x, y + h);
		closed = true;
	} else if (tag == QLatin1String("polygon")) {
		for (int i = 1; part.hasAttribute(QStringLiteral("x%1").arg(i)); ++i)
			pts << QPointF(attr(part, QStringLiteral("x%1").arg(i)), attr(part, QStringLiteral("y%1").arg(i)));
		closed = part.attribute(QStringLiteral("closed")) != QLatin1String("false");
	}
	QList<Seg> out;
	const int n = static_cast<int>(pts.size());
	if (n < 2) return out;
	for (int i = 0; i + 1 < n + (closed ? 1 : 0); ++i) {
		const QPointF a = pts.at(i), b = pts.at((i + 1) % n);
		out << (horizontal ? Seg{a.x(), a.y(), b.x(), b.y()} : Seg{a.y(), a.x(), b.y(), b.x()});
	}
	return out;
}

bool along(const Seg &s) { return std::abs(s.v1 - s.v2) < 0.01 && std::abs(s.u1 - s.u2) > 0.5; }
bool across(const Seg &s) { return std::abs(s.u1 - s.u2) < 0.01 && std::abs(s.v1 - s.v2) > 0.5; }
bool atPort(double u, double v, const QList<Port> &ports)
{
	for (const Port &p : ports)
		if (std::abs(p.u - u) < 0.6 && std::abs(p.v - v) < 0.6) return true;
	return false;
}
bool touches(const Seg &s, const QList<Port> &ports) { return atPort(s.u1, s.v1, ports) || atPort(s.u2, s.v2, ports); }

QList<Port> ports(const QDomElement &description, bool horizontal)
{
	QList<Port> out;
	for (QDomElement t = description.firstChildElement(QStringLiteral("terminal")); !t.isNull();
		 t = t.nextSiblingElement(QStringLiteral("terminal"))) {
		const QString medium = t.attribute(QStringLiteral("medium"));
		const double x = attr(t, "x"), y = attr(t, "y");
		out << Port{t.attribute(QStringLiteral("name")), horizontal ? x : y, horizontal ? y : x,
					t.attribute(QStringLiteral("orientation")),
					medium == QLatin1String("pneumatic") || medium == QLatin1String("hydraulic")};
	}
	return out;
}

	/// the parts of the description in order (indexes as in Pose::moving)
QList<QDomElement> parts(const QDomElement &description)
{
	QList<QDomElement> out;
	for (QDomElement p = description.firstChildElement(); !p.isNull(); p = p.nextSiblingElement()) out << p;
	return out;
}

/**
	Cylinder: the barrel is the pair of edges along the axis furthest
	apart; the rod the edges along the axis inside the barrel that reach out
	of it; the piston the edges across, nearly as high as the barrel,
	inside it. They travel from where the piston is drawn (retracted) until
	its face meets the end of the barrel.
*/
bool findCylinder(const QDomElement &description, bool horizontal, SimulationOverlay::Pose &pose)
{
	const QList<Port> pts = ports(description, horizontal);
	const QList<QDomElement> list = parts(description);
	QList<QList<Seg>> segs;
	for (const QDomElement &p : list) segs << edges(p, horizontal);
	double v_top = std::numeric_limits<double>::max(), v_bot = -v_top;
	for (const auto &ss : segs)
		for (const Seg &s : ss)
			if (along(s) && !touches(s, pts)) { v_top = std::min(v_top, s.v1); v_bot = std::max(v_bot, s.v1); }
	if (v_bot - v_top < 4.0) return false;
	double u_min = std::numeric_limits<double>::max(), u_max = -u_min;
	for (const auto &ss : segs)
		for (const Seg &s : ss)
			if (along(s) && (std::abs(s.v1 - v_top) < 0.3 || std::abs(s.v1 - v_bot) < 0.3)) {
				u_min = std::min({u_min, s.u1, s.u2});
				u_max = std::max({u_max, s.u1, s.u2});
			}
	if (u_max - u_min < 5.0) return false;
	const double wall = 0.5 * (u_max - u_min);
	QSet<int> rod_right, rod_left, piston;
	double front_right = -std::numeric_limits<double>::max(), front_left = std::numeric_limits<double>::max();
	for (int i = 0; i < segs.size(); ++i) {
		bool is_wall = false;
		for (const Seg &s : segs.at(i))
			if (along(s) && (std::abs(s.v1 - v_top) < 0.3 || std::abs(s.v1 - v_bot) < 0.3)
				&& std::abs(s.u1 - s.u2) > wall) is_wall = true;
		if (is_wall) continue;
		for (const Seg &s : segs.at(i)) {
			if (touches(s, pts)) continue;
			if (along(s) && s.v1 > v_top + 0.3 && s.v1 < v_bot - 0.3) {
				if (std::max(s.u1, s.u2) > u_max + 1.0) rod_right << i;
				if (std::min(s.u1, s.u2) < u_min - 1.0) rod_left << i;
			}
			if (across(s) && std::abs(s.v1 - s.v2) >= 0.6 * (v_bot - v_top)
				&& s.u1 > u_min + 0.6 && s.u1 < u_max - 0.6) {
				piston << i;
				front_right = std::max(front_right, s.u1);
				front_left = std::min(front_left, s.u1);
			}
		}
	}
	if (piston.isEmpty() || (rod_right.isEmpty() && rod_left.isEmpty())) return false;
	const bool right = !rod_right.isEmpty();
	const double way = right ? u_max - front_right : front_left - u_min;
	if (way < 1.0) return false;
	pose.moving = piston + (right ? rod_right : rod_left);
	const QPointF dir = horizontal ? QPointF(1, 0) : QPointF(0, 1);
	pose.axis = right ? dir : -dir;
	pose.way = way;
	return true;
}

/**
	Valve: the ports stand at the field of the rest position. Its edges
	are the nearest lines across the band of the fields, on either side of
	the ports, that do not run through a port (the flow arrows do); its
	width is the way of one position. All parts but the stubs to the ports
	and the texts slide.
*/
bool findValve(const QDomElement &description, bool horizontal, SimulationOverlay::Pose &pose)
{
	const QList<Port> all = ports(description, horizontal);
	QList<Port> work;
	for (const Port &p : all)
		if (p.fluid && (horizontal ? (p.orientation == QLatin1String("n") || p.orientation == QLatin1String("s"))
								   : (p.orientation == QLatin1String("e") || p.orientation == QLatin1String("w"))))
			work << p;
	if (work.isEmpty()) return false;
	const QList<QDomElement> list = parts(description);
	QList<QList<Seg>> segs;
	for (const QDomElement &p : list) segs << edges(p, horizontal);
	// the stubs from the ports to the fields stay where they are (FluidSIM symbols draw them, QET ones not)
	QSet<int> stubs;
	for (int i = 0; i < segs.size(); ++i)
		for (const Seg &s : segs.at(i))
			if (across(s) && touches(s, work)) stubs << i;
	// the band of the fields: the nearest edges along the row, over a port, inside the ports above and below
	const auto upper = [&](const Port &p) {
		return p.orientation == QLatin1String(horizontal ? "n" : "w");
	};
	double top_port = -std::numeric_limits<double>::max(), bot_port = std::numeric_limits<double>::max();
	for (const Port &p : work) {
		if (upper(p)) top_port = std::max(top_port, p.v);
		else bot_port = std::min(bot_port, p.v);
	}
	double v_top = std::numeric_limits<double>::max(), v_bot = -v_top;
	for (int i = 0; i < segs.size(); ++i) {
		if (stubs.contains(i)) continue;
		for (const Seg &s : segs.at(i)) {
			if (!along(s)) continue;
			bool over_port = false;
			for (const Port &p : work)
				if (p.u >= std::min(s.u1, s.u2) - 0.01 && p.u <= std::max(s.u1, s.u2) + 0.01) over_port = true;
			if (!over_port || s.v1 <= top_port + 0.01 || s.v1 >= bot_port - 0.01) continue;
			v_top = std::min(v_top, s.v1);
			v_bot = std::max(v_bot, s.v1);
		}
	}
	if (v_bot - v_top < 4.0) return false;
	double p_min = std::numeric_limits<double>::max(), p_max = -p_min;
	for (const Port &p : work) { p_min = std::min(p_min, p.u); p_max = std::max(p_max, p.u); }
	const double none = std::numeric_limits<double>::max();
	double left = -none, right = none;
	for (int i = 0; i < segs.size(); ++i) {
		if (stubs.contains(i)) continue;
		for (const Seg &s : segs.at(i)) {
			if (!across(s)) continue;
			if (std::min(s.v1, s.v2) > v_top + 0.6 || std::max(s.v1, s.v2) < v_bot - 0.6) continue;
			bool through_port = false;
			for (const Port &p : work) if (std::abs(p.u - s.u1) < 1.0) through_port = true;
			if (through_port) continue;
			if (s.u1 < p_min - 0.5) left = std::max(left, s.u1);
			if (s.u1 > p_max + 0.5) right = std::min(right, s.u1);
		}
	}
	if (left == -none || right == none || right - left < 5.0) return false;
	for (int i = 0; i < list.size(); ++i) {
		const QString tag = list.at(i).tagName();
		if (stubs.contains(i) || tag == QLatin1String("text") || tag == QLatin1String("terminal")
			|| tag == QLatin1String("dynamic_text") || tag == QLatin1String("plc_table")) continue;
		pose.moving << i;
		// a line from another port (pilot 12/14, coil) stays at the port and stretches to the moving actuator
		if (tag == QLatin1String("line") && segs.at(i).size() == 1) {
			const Seg &s = segs.at(i).first();
			if (atPort(s.u1, s.v1, all)) pose.stretch.insert(i, 2);
			else if (atPort(s.u2, s.v2, all)) pose.stretch.insert(i, 1);
		}
	}
	pose.axis = horizontal ? QPointF(1, 0) : QPointF(0, 1);
	pose.way = right - left;
	return true;
}

	/// @a part moved by @a d: every coordinate attribute (x, y, x1, y2 …); @a only_end 1 or 2: only x1/y1 or x2/y2
void shift(QDomElement &part, const QPointF &d, int only_end = 0)
{
	static const QRegularExpression coord(QStringLiteral("^([xy])(\\d*)$"));
	const QDomNamedNodeMap map = part.attributes();
	QList<QPair<QString, double>> changed;
	for (int i = 0; i < map.count(); ++i) {
		const QDomAttr a = map.item(i).toAttr();
		const auto m = coord.match(a.name());
		if (!m.hasMatch()) continue;
		if (only_end && m.captured(2) != QString::number(only_end)) continue;
		changed << qMakePair(a.name(), a.value().toDouble() + (m.captured(1) == QLatin1String("x") ? d.x() : d.y()));
	}
	for (const auto &c : changed) part.setAttribute(c.first, QString::number(c.second, 'g', 10));
}
}

void SimulationOverlay::pose(Element *e, bool valve, double value, int rest)
{
	if (!e || !e->scene()) return;
	Pose *&p = m_poses[e];
	if (!p) {
		p = new Pose;
		p->element = e;
		const QDomElement definition = e->location().xml();
		p->description = definition.firstChildElement(QStringLiteral("description"));
		if (!p->description.isNull()) {
			// the cylinder or the row of fields lies along x or along y of the symbol
			for (const bool horizontal : {true, false}) {
				Pose trial;
				if (valve ? findValve(p->description, horizontal, trial) : findCylinder(p->description, horizontal, trial)) {
					p->moving = trial.moving;
					p->stretch = trial.stretch;
					p->axis = trial.axis;
					p->way = trial.way;
					p->ok = true;
					break;
				}
			}
		}
	}
	if (!p->ok || !p->element) return;
	// cylinder: share of the stroke; valve: fields from the rest position to where the spool stands
	const double offset = valve ? (rest - value) * p->way : std::clamp(value, 0.0, 1.0) * p->way;
	if (!std::isnan(p->last) && std::abs(offset - p->last) < 0.05) return;
	p->last = offset;
	if (std::abs(offset) < 0.05) {
		e->setSimulationPicture(QPicture());   // the symbol as drawn
		return;
	}
	QDomDocument doc;
	QDomElement copy = doc.importNode(p->description, true).toElement();
	const QPointF d = p->axis * offset;
	int i = 0;
	for (QDomElement part = copy.firstChildElement(); !part.isNull(); part = part.nextSiblingElement(), ++i)
		if (p->moving.contains(i)) shift(part, d, p->stretch.value(i, 0));
	e->setSimulationPicture(ElementPictureFactory::instance()->pictureFromDescription(copy, e->readableTransform()));
}

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
	// the symbols move like in FluidSIM: piston and rod with the stroke, the fields of the valves
	for (const auto &z : model.cylinders())
		if (!z.rotary && z.stroke_mm > 0) pose(z.element, false, z.x_mm / z.stroke_mm, 0);
	for (const auto &v : model.valveStates())
		pose(v.element, true, v.spool, v.rest - 1);
	// GRAFCET: active steps get a token and a green frame
	const QList<Element *> active = model.activeSteps();
	for (auto it = m_marks.begin(); it != m_marks.end(); ++it)
		if (it.value()) it.value()->setVisible(active.contains(it.key()));
	for (Element *e : active) {
		if (!e->scene()) continue;
		QGraphicsItem *&item = m_marks[e];
		auto *mark = static_cast<QGraphicsPathItem *>(item);
		if (!mark) {
			mark = new QGraphicsPathItem();
			mark->setZValue(10002);
			mark->setAcceptedMouseButtons(Qt::NoButton);
			mark->setPen(QPen(QColor(0, 150, 60), 2.5));
			mark->setBrush(QColor(0, 150, 60, 60));
			e->scene()->addItem(mark);
			item = mark;
		}
		QPainterPath frame;
		frame.addRect(e->mapRectToScene(QRectF(-22, -22, 44, 44)));
		QPainterPath token;
		token.addEllipse(e->mapToScene(QPointF(0, 12)), 3.5, 3.5);   // under the step number
		mark->setPath(frame);
		auto *dot = static_cast<QGraphicsPathItem *>(m_tokens.value(e));
		if (!dot) {
			dot = new QGraphicsPathItem();
			dot->setZValue(10003);
			dot->setAcceptedMouseButtons(Qt::NoButton);
			dot->setPen(Qt::NoPen);
			dot->setBrush(QColor(20, 20, 20));
			e->scene()->addItem(dot);
			m_tokens.insert(e, dot);
		}
		dot->setPath(token);
		mark->setVisible(true);
	}
	for (auto it = m_tokens.begin(); it != m_tokens.end(); ++it)
		if (it.value()) it.value()->setVisible(active.contains(it.key()));
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
		for (const auto *list : {&m_marks, &m_tokens})
			for (QGraphicsItem *item : *list) {
				if (!item) continue;
				if (item->scene()) item->scene()->removeItem(item);
				delete item;
			}
	}
	for (Pose *p : std::as_const(m_poses)) {
		if (p && p->element && p->element->hasSimulationPicture()) p->element->setSimulationPicture(QPicture());
		delete p;
	}
	m_poses.clear();
	m_wires.clear();
	m_texts.clear();
	m_marks.clear();
	m_tokens.clear();
}
