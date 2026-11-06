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
#include "paneldata.h"

#include <QDomDocument>
#include <QtMath>

namespace {
QString num(double v) {return QString::number(v, 'g', 10);}
double dbl(const QDomElement &e, const char *name, double fallback = 0)
{
	bool ok = false;
	const double v = e.attribute(QLatin1String(name)).toDouble(&ok);
	return ok ? v : fallback;
}
}

QDomElement PanelHole::toXml(QDomDocument &document, const QString &tag) const
{
	QDomElement e = document.createElement(tag);
	e.setAttribute(QStringLiteral("type"), int(type));
	if (subtype != Plain) e.setAttribute(QStringLiteral("subtype"), int(subtype));
	e.setAttribute(QStringLiteral("d1"), num(d1));
	if (d2 != 0) e.setAttribute(QStringLiteral("d2"), num(d2));
	if (d3 != 0) e.setAttribute(QStringLiteral("d3"), num(d3));
	e.setAttribute(QStringLiteral("x"), num(x));
	e.setAttribute(QStringLiteral("y"), num(y));
	if (angle != 0) e.setAttribute(QStringLiteral("angle"), num(angle));
	if (repeat != 0) e.setAttribute(QStringLiteral("repeat"), num(repeat));
	if (end != 0) e.setAttribute(QStringLiteral("end"), num(end));
	if (nth != 0) e.setAttribute(QStringLiteral("nth"), nth);
	if (!contour.isEmpty()) e.setAttribute(QStringLiteral("contour"), contour);
	return e;
}

PanelHole PanelHole::fromXml(const QDomElement &e)
{
	PanelHole h;
	if (e.isNull()) return h;
	h.type = Type(e.attribute(QStringLiteral("type"), QStringLiteral("1")).toInt());
	h.subtype = SubType(e.attribute(QStringLiteral("subtype"), QStringLiteral("0")).toInt());
	h.d1 = dbl(e, "d1");
	h.d2 = dbl(e, "d2");
	h.d3 = dbl(e, "d3");
	h.x = dbl(e, "x");
	h.y = dbl(e, "y");
	h.angle = dbl(e, "angle");
	h.repeat = dbl(e, "repeat");
	h.end = dbl(e, "end");
	h.nth = e.attribute(QStringLiteral("nth"), QStringLiteral("0")).toInt();
	h.contour = e.attribute(QStringLiteral("contour"));
	return h;
}

QString PanelHole::kindName() const
{
	auto n = [](double v) {return QString::number(v, 'g', 6).replace(QLatin1Char('.'), QLatin1Char(','));};
	switch (type) {
		case Thread:    return QStringLiteral("M%1").arg(n(d1));
		case Rectangle: return QStringLiteral("R%1x%2").arg(n(d1), n(d2));
		case Slot:      return QStringLiteral("L%1x%2").arg(n(d1), n(d2));
		case Hexagon:   return QStringLiteral("H%1").arg(n(d1));
		case Octagon:   return QStringLiteral("O%1").arg(n(d1));
		case Contour:   return QStringLiteral("K_%1").arg(contour);
		default:        return QStringLiteral("D%1").arg(n(d1));
	}
}

QDomElement PanelPattern::toXml(QDomDocument &document) const
{
	QDomElement e = document.createElement(QStringLiteral("pattern"));
	e.setAttribute(QStringLiteral("variant"), variant);
	if (din_rail) e.setAttribute(QStringLiteral("din_rail"), QStringLiteral("1"));
	for (const PanelHole &h : holes) e.appendChild(h.toXml(document));
	return e;
}

PanelPattern PanelPattern::fromXml(const QDomElement &e)
{
	PanelPattern p;
	p.variant = e.attribute(QStringLiteral("variant"));
	p.din_rail = e.attribute(QStringLiteral("din_rail")) == QLatin1String("1");
	for (QDomElement h = e.firstChildElement(QStringLiteral("hole")); !h.isNull();
		 h = h.nextSiblingElement(QStringLiteral("hole")))
		p.holes.append(PanelHole::fromXml(h));
	return p;
}

QString PanelData::roleName(Role role)
{
	switch (role) {
		case Surface:  return QStringLiteral("surface");
		case DinRail:  return QStringLiteral("din_rail");
		case Duct:     return QStringLiteral("duct");
		case CProfile: return QStringLiteral("c_profile");
		case Cutout:   return QStringLiteral("cutout");
		case Keepout:  return QStringLiteral("keepout");
		case Enclosure: return QStringLiteral("enclosure");
		default:       return QString();
	}
}

PanelData::Role PanelData::roleFromName(const QString &name)
{
	for (Role r : {Surface, DinRail, Duct, CProfile, Cutout, Keepout, Enclosure})
		if (roleName(r) == name) return r;
	return None;
}

PanelData PanelData::defaults(Role role)
{
	PanelData d;
	d.role = role;
	switch (role) {
		case DinRail:
				//TS35 x 7.5: M5 every 100 mm, 25 mm from the ends
			d.profile_depth = 7.5;
			d.fixing.type = PanelHole::Thread;
			d.fixing.d1 = 5;
			d.spacing = 100;
			d.end_distance = 25;
			break;
		case Duct:
				//wiring duct: M4 every 250 mm on the centre line
			d.profile_depth = 60;
			d.fixing.type = PanelHole::Thread;
			d.fixing.d1 = 4;
			d.spacing = 250;
			d.end_distance = 30;
			break;
		case CProfile:
			d.profile_depth = 15;
			d.fixing.type = PanelHole::Thread;
			d.fixing.d1 = 6;
			d.spacing = 200;
			d.end_distance = 25;
			break;
		case Cutout:
			d.cutout.type = PanelHole::Drill;
			d.cutout.d1 = 22.5;
			break;
		default:
			break;
	}
	return d;
}

QDomElement PanelData::toXml(QDomDocument &document) const
{
	QDomElement e = document.createElement(QStringLiteral("panel"));
	e.setAttribute(QStringLiteral("role"), roleName(role));
	if (!designation.isEmpty()) e.setAttribute(QStringLiteral("designation"), designation);
	if (!article.isEmpty()) e.setAttribute(QStringLiteral("article"), article);
	if (!manufacturer.isEmpty()) e.setAttribute(QStringLiteral("manufacturer"), manufacturer);
	if (!step_file.isEmpty()) e.setAttribute(QStringLiteral("step"), step_file);
	switch (role) {
		case Surface:
			e.setAttribute(QStringLiteral("mm_per_unit"), num(mm_per_unit));
			e.setAttribute(QStringLiteral("thickness"), num(thickness));
			e.setAttribute(QStringLiteral("machined"), machined ? 1 : 0);
			e.setAttribute(QStringLiteral("origin"), origin);
			if (!outline.isEmpty()) {
				QDomElement o = outline.toXml(document);
				o.setAttribute(QStringLiteral("machined"), outline_machined ? 1 : 0);
				e.appendChild(o);
			}
			break;
		case DinRail:
		case Duct:
		case CProfile:
			e.setAttribute(QStringLiteral("depth"), num(profile_depth));
			e.setAttribute(QStringLiteral("spacing"), num(spacing));
			e.setAttribute(QStringLiteral("end_distance"), num(end_distance));
			e.setAttribute(QStringLiteral("rows"), rows);
			if (row_offset != 0) e.setAttribute(QStringLiteral("row_offset"), num(row_offset));
			if (zero_from_end) e.setAttribute(QStringLiteral("zero_from_end"), 1);
			e.appendChild(fixing.toXml(document, QStringLiteral("fixing")));
			break;
		case Cutout:
			e.appendChild(cutout.toXml(document, QStringLiteral("cutout")));
			break;
		case Enclosure:
				//the front view of the enclosure and its depth
			e.setAttribute(QStringLiteral("depth"), num(profile_depth));
			e.setAttribute(QStringLiteral("mm_per_unit"), num(mm_per_unit));
			if (!outline.isEmpty()) e.appendChild(outline.toXml(document));
			break;
		default:
			break;
	}
	return e;
}

PanelData PanelData::fromXml(const QDomElement &e)
{
	PanelData d;
	if (e.isNull() || e.tagName() != QLatin1String("panel")) return d;
	d.role = roleFromName(e.attribute(QStringLiteral("role")));
	d.designation = e.attribute(QStringLiteral("designation"));
	d.article = e.attribute(QStringLiteral("article"));
	d.manufacturer = e.attribute(QStringLiteral("manufacturer"));
	d.step_file = e.attribute(QStringLiteral("step"));
	d.mm_per_unit = dbl(e, "mm_per_unit", 0.4);
	d.thickness = dbl(e, "thickness", 3);
	d.machined = e.attribute(QStringLiteral("machined"), QStringLiteral("1")) != QLatin1String("0");
	d.origin = e.attribute(QStringLiteral("origin"), QStringLiteral("0")).toInt();
	d.profile_depth = dbl(e, "depth");
	d.spacing = dbl(e, "spacing", 100);
	d.end_distance = dbl(e, "end_distance", 25);
	d.rows = e.attribute(QStringLiteral("rows"), QStringLiteral("1")).toInt();
	d.row_offset = dbl(e, "row_offset");
	d.zero_from_end = e.attribute(QStringLiteral("zero_from_end")) == QLatin1String("1");
	d.fixing = PanelHole::fromXml(e.firstChildElement(QStringLiteral("fixing")));
	d.cutout = PanelHole::fromXml(e.firstChildElement(QStringLiteral("cutout")));
	const QDomElement o = e.firstChildElement(QStringLiteral("outline"));
	d.outline = PanelOutline::fromXml(o);
	d.outline_machined = o.attribute(QStringLiteral("machined"), QStringLiteral("1")) != QLatin1String("0");
	return d;
}

/* ------------------------------------------------------------------ */

PanelOutline PanelOutline::stretched(double w, double h) const
{
	if (isEmpty() || width <= 0 || height <= 0) return *this;
	const double dx = w - width, dy = h - height;
	const double mx = width / 2, my = height / 2;
	auto move = [&](const QPointF &p) {
		return QPointF(p.x() + (p.x() > mx ? dx : 0), p.y() + (p.y() > my ? dy : 0));
	};
	PanelOutline out;
	out.width = w;
	out.height = h;
	out.name = name;
	for (const QLineF &l : lines) out.lines << QLineF(move(l.p1()), move(l.p2()));
	for (const Arc &a : arcs) {
		Arc b = a;
		b.centre = move(a.centre);
		out.arcs << b;
	}
	return out;
}

QList<PanelOutline::Arc> PanelOutline::circles() const
{
	QList<Arc> out;
	for (const Arc &a : arcs)
		if (a.isCircle()) out << a;
	return out;
}

QDomElement PanelOutline::toXml(QDomDocument &document) const
{
	QDomElement e = document.createElement(QStringLiteral("outline"));
	e.setAttribute(QStringLiteral("width"), num(width));
	e.setAttribute(QStringLiteral("height"), num(height));
	if (!name.isEmpty()) e.setAttribute(QStringLiteral("name"), name);
	for (const QLineF &l : lines) {
		QDomElement x = document.createElement(QStringLiteral("line"));
		x.setAttribute(QStringLiteral("x1"), num(l.x1()));
		x.setAttribute(QStringLiteral("y1"), num(l.y1()));
		x.setAttribute(QStringLiteral("x2"), num(l.x2()));
		x.setAttribute(QStringLiteral("y2"), num(l.y2()));
		e.appendChild(x);
	}
	for (const Arc &a : arcs) {
		QDomElement x = document.createElement(a.isCircle() ? QStringLiteral("circle") : QStringLiteral("arc"));
		x.setAttribute(QStringLiteral("x"), num(a.centre.x()));
		x.setAttribute(QStringLiteral("y"), num(a.centre.y()));
		x.setAttribute(QStringLiteral("r"), num(a.r));
		if (!a.isCircle()) {
			x.setAttribute(QStringLiteral("a0"), num(a.a0));
			x.setAttribute(QStringLiteral("a1"), num(a.a1));
		}
		e.appendChild(x);
	}
	return e;
}

PanelOutline PanelOutline::fromXml(const QDomElement &e)
{
	PanelOutline o;
	if (e.isNull()) return o;
	o.width = dbl(e, "width");
	o.height = dbl(e, "height");
	o.name = e.attribute(QStringLiteral("name"));
	for (QDomElement x = e.firstChildElement(); !x.isNull(); x = x.nextSiblingElement()) {
		if (x.tagName() == QLatin1String("line"))
			o.lines << QLineF(dbl(x, "x1"), dbl(x, "y1"), dbl(x, "x2"), dbl(x, "y2"));
		else if (x.tagName() == QLatin1String("circle") || x.tagName() == QLatin1String("arc")) {
			Arc a;
			a.centre = QPointF(dbl(x, "x"), dbl(x, "y"));
			a.r = dbl(x, "r");
			if (x.tagName() == QLatin1String("arc")) {
				a.a0 = dbl(x, "a0");
				a.a1 = dbl(x, "a1");
			}
			o.arcs << a;
		}
	}
	return o;
}
