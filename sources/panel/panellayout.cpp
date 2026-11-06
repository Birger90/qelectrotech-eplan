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
#include "panellayout.h"

#include "../diagramcontext.h"

#include <QTransform>

#include "../diagram.h"
#include "../ElementsCollection/elementslocation.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/qetshapeitem.h"

#include <QLineF>
#include <QMap>
#include <QtMath>

#include <algorithm>

namespace {
QRectF sceneRectOf(QetShapeItem *shape)
{
	return shape->mapToScene(shape->rect().normalized()).boundingRect();
}

QString fmt(double v)
{
	return QString::number(v, 'f', 1).replace(QLatin1Char('.'), QLatin1Char(','));
}

double infoNumber(const DiagramContext &info, const QString &key)
{
	return info.value(key).toString().replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
}
}

/**
	@brief PanelLayout::PanelLayout
	@param surface a shape whose role is PanelData::Surface
*/
PanelLayout::PanelLayout(QetShapeItem *surface) :
	m_surface(surface)
{
	if (!m_surface) return;
	m_data = m_surface->panelData();
	if (m_data.mm_per_unit <= 0) m_data.mm_per_unit = 0.4;
	m_scene_rect = sceneRectOf(m_surface);
	m_width = m_scene_rect.width() * m_data.mm_per_unit;
	m_height = m_scene_rect.height() * m_data.mm_per_unit;
	collect();
}

/**
	@brief PanelLayout::surfaces
	@return the mounting surfaces (doors, walls, plates) of @p diagram,
	from top left to bottom right
*/
QList<QetShapeItem *> PanelLayout::surfaces(Diagram *diagram)
{
	QList<QetShapeItem *> out;
	if (!diagram) return out;
	for (QGraphicsItem *item : diagram->items())
		if (auto *shape = dynamic_cast<QetShapeItem *>(item))
			if (shape->panelData().role == PanelData::Surface)
				out << shape;
	std::sort(out.begin(), out.end(), [](QetShapeItem *a, QetShapeItem *b) {
		const QRectF ra = sceneRectOf(a), rb = sceneRectOf(b);
		return qFuzzyCompare(ra.top(), rb.top()) ? ra.left() < rb.left() : ra.top() < rb.top();
	});
	return out;
}

/**
	@brief PanelLayout::patternsOf
	@return the drilling patterns (mounting variants) the definition of
	@p element declares: <drillingpatterns><pattern variant=...> in the
	.elmt, written by the part import from the manufacturer's data or by
	hand in the element editor.
*/
QList<PanelPattern> PanelLayout::patternsOf(Element *element)
{
	QList<PanelPattern> out;
	if (!element) return out;
	const QDomElement definition = element->location().xml();
	const QDomElement patterns = definition.firstChildElement(QStringLiteral("drillingpatterns"));
	for (QDomElement p = patterns.firstChildElement(QStringLiteral("pattern")); !p.isNull();
		 p = p.nextSiblingElement(QStringLiteral("pattern")))
		out << PanelPattern::fromXml(p);
	return out;
}

QPointF PanelLayout::railSnap(Diagram *diagram, Element *element)
{
	if (!diagram || !element) return QPointF();
	QRectF box = element->sceneBoundingRect();
	for (QetShapeItem *surface : surfaces(diagram))
	{
		const QRectF plate = sceneRectOf(surface);
		if (!plate.contains(box.center())) continue;
		const PanelData data = surface->panelData();
		const double k = data.mm_per_unit > 0 ? data.mm_per_unit : 1;
			//the footprint from the device's size, as the layout sees it
		const DiagramContext info = element->elementInformations();
		const double w = infoNumber(info, QStringLiteral("width_mm")), h = infoNumber(info, QStringLiteral("height_mm"));
		if (w > 0 && h > 0) {
			QTransform t;
			t.translate(element->scenePos().x(), element->scenePos().y());
			t.rotate(element->rotation());
			box = t.mapRect(QRectF(0, 0, w / k, h / k));
		}
		const double near = 25 / k, catch_x = 15 / k;
			//the rail: horizontal, under the device or just beside it
		QRectF rail;
		double best = 1e300;
		for (QGraphicsItem *item : diagram->items()) {
			auto *shape = dynamic_cast<QetShapeItem *>(item);
			if (!shape || shape->panelData().role != PanelData::DinRail) continue;
			const QRectF r = sceneRectOf(shape);
			if (r.width() < r.height() || !plate.contains(r.center())) continue;
			if (r.right() < box.left() || r.left() > box.right()) continue;
				// caught when the device's middle is near the rail: pulled
				// further away it comes free again
			if (qAbs(box.center().y() - r.center().y()) > r.height() / 2 + near) continue;
			const double d = qAbs(box.center().y() - r.center().y());
			if (d < best) {best = d; rail = r;}
		}
		if (rail.isNull()) {
				// no rail: a device hung onto another one (a contactor under its
				// motor starter, an auxiliary block on a contactor) snaps edge to
				// edge onto the device right above or below it
			auto footprint = [k](Element *e) {
				const DiagramContext i = e->elementInformations();
				const double ew = infoNumber(i, QStringLiteral("width_mm")), eh = infoNumber(i, QStringLiteral("height_mm"));
				if (ew <= 0 || eh <= 0) return QRectF();
				QTransform t;
				t.translate(e->scenePos().x(), e->scenePos().y());
				t.rotate(e->rotation());
				return t.mapRect(QRectF(0, 0, ew / k, eh / k));
			};
			const double catch_edge = 15 / k;
			double sdx = 0, sdy = 0, sbest = 1e300;
			for (QGraphicsItem *item : diagram->items()) {
				auto *other = dynamic_cast<Element *>(item);
				if (!other || other == element || other->parentItem()) continue;
				const QRectF ob = footprint(other);
				if (ob.isNull()) continue;
				if (box.right() <= ob.left() || box.left() >= ob.right()) continue;   // not above or below it
				for (double d : {ob.bottom() - box.top(), ob.top() - box.bottom()}) {   // under it / over it
					if (qAbs(d) > catch_edge || qAbs(d) >= sbest) continue;
					sbest = qAbs(d);
					sdy = d;
						// in line with it when nearly so
					const double ldx = ob.left() - box.left();
					sdx = qAbs(ldx) <= catch_x ? ldx : 0;
				}
			}
			if (sbest > 1e299) return QPointF();
			return QPointF(sdx, sdy);
		}
		const double offset = infoNumber(info, QStringLiteral("middle_offset_mm")) / k;
		const double dy = rail.center().y() - offset - box.center().y();
			//sideways: against the nearest neighbour on the rail, or its start
		double dx = 0, nearest = catch_x;
		auto consider = [&](double target_edge, double own_edge) {
			const double d = target_edge - own_edge;
			if (qAbs(d) < nearest) {nearest = qAbs(d); dx = d;}
		};
		consider(rail.left() + 5 / k, box.left());
		QList<QRectF> neighbours;
		for (QGraphicsItem *item : diagram->items()) {
			auto *other = dynamic_cast<Element *>(item);
			if (!other || other == element || other->parentItem()) continue;
			QRectF ob = other->sceneBoundingRect();
			const DiagramContext oi = other->elementInformations();
			const double ow = infoNumber(oi, QStringLiteral("width_mm")), oh = infoNumber(oi, QStringLiteral("height_mm"));
			if (ow > 0 && oh > 0) {
				QTransform t;
				t.translate(other->scenePos().x(), other->scenePos().y());
				t.rotate(other->rotation());
				ob = t.mapRect(QRectF(0, 0, ow / k, oh / k));
			}
			if (!ob.intersects(rail)) continue;
			neighbours << ob;
			consider(ob.right(), box.left());    // to the right of it
			consider(ob.left(), box.right());    // to the left of it
		}
			// never on top of a neighbour: to the nearer free side of it
		for (int round = 0; round < 8; ++round) {
			const QRectF placed = box.translated(dx, 0).adjusted(0.01, 0, -0.01, 0);
			const QRectF *hit = nullptr;
			for (const QRectF &ob : std::as_const(neighbours))
				if (placed.left() < ob.right() && placed.right() > ob.left()) {hit = &ob; break;}
			if (!hit) break;
			const double to_right = hit->right() - box.left(), to_left = hit->left() - box.right();
			dx = qAbs(to_right - dx) <= qAbs(to_left - dx) ? to_right : to_left;
		}
		return QPointF(dx, dy);
	}
	return QPointF();
}

QString PanelLayout::name() const
{
	return m_data.designation.isEmpty() ? QStringLiteral("Montageflaeche") : m_data.designation;
}

/**
	@brief PanelLayout::toMm
	Scene point -> millimetres from the zero point of the surface (x to
	the right, y away from the zero edge, like EPLAN's mounting surface).
*/
QPointF PanelLayout::toMm(const QPointF &scene) const
{
	const double k = m_data.mm_per_unit;
	const QRectF r = m_scene_rect;
	switch (m_data.origin) {
		case 1:  return QPointF((scene.x() - r.left()) * k, (scene.y() - r.top()) * k);
		case 2:  return QPointF((r.right() - scene.x()) * k, (scene.y() - r.top()) * k);
		case 3:  return QPointF((r.right() - scene.x()) * k, (r.bottom() - scene.y()) * k);
		default: return QPointF((scene.x() - r.left()) * k, (r.bottom() - scene.y()) * k);
	}
}

QPointF PanelLayout::toScene(const QPointF &mm) const
{
	const double k = m_data.mm_per_unit;
	const QRectF r = m_scene_rect;
	switch (m_data.origin) {
		case 1:  return QPointF(r.left() + mm.x() / k, r.top() + mm.y() / k);
		case 2:  return QPointF(r.right() - mm.x() / k, r.top() + mm.y() / k);
		case 3:  return QPointF(r.right() - mm.x() / k, r.bottom() - mm.y() / k);
		default: return QPointF(r.left() + mm.x() / k, r.bottom() - mm.y() / k);
	}
}

/**
	@brief PanelLayout::collect
	Gather what lies on the surface -- a part belongs to the surface its
	centre is on -- and work out the holes.
*/
void PanelLayout::collect()
{
	Diagram *diagram = qobject_cast<Diagram *>(m_surface->scene());
	if (!diagram) return;
	const double k = m_data.mm_per_unit;

	QList<QetShapeItem *> cutouts;
	for (QGraphicsItem *item : diagram->items())
	{
		auto *shape = dynamic_cast<QetShapeItem *>(item);
		if (!shape || shape == m_surface) continue;
		const PanelData data = shape->panelData();
		if (data.isNull() || data.role == PanelData::Surface) continue;
		const QRectF r = sceneRectOf(shape);
		if (!m_scene_rect.contains(r.center())) continue;

		switch (data.role) {
			case PanelData::DinRail:
			case PanelData::Duct:
			case PanelData::CProfile: {
				PanelProfile p;
				p.shape = shape;
				p.data = data;
				const bool horizontal = r.width() >= r.height();
				const QPointF a = horizontal ? QPointF(r.left(), r.center().y()) : QPointF(r.center().x(), r.bottom());
				const QPointF b = horizontal ? QPointF(r.right(), r.center().y()) : QPointF(r.center().x(), r.top());
				p.start_mm = toMm(a);
				p.end_mm = toMm(b);
				p.length_mm = QLineF(p.start_mm, p.end_mm).length();
				p.width_mm = (horizontal ? r.height() : r.width()) * k;
				m_profiles << p;
				break;
			}
			case PanelData::Cutout:
				cutouts << shape;
				break;
			case PanelData::Keepout: {
				const QPointF p1 = toMm(r.topLeft()), p2 = toMm(r.bottomRight());
				m_keepouts << QRectF(p1, p2).normalized();
				break;
			}
			default:
				break;
		}
	}

	for (QGraphicsItem *item : diagram->items())
	{
		auto *element = dynamic_cast<Element *>(item);
		if (!element || element->parentItem()) continue;
		QRectF r = element->sceneBoundingRect();
		if (!m_scene_rect.contains(r.center())) continue;

		PanelDevice d;
		d.element = element;
		d.designation = element->actualLabel();
		d.pos_mm = toMm(element->scenePos());
		d.angle = element->rotation();
		const DiagramContext info = element->elementInformations();
		d.width_mm = infoNumber(info, QStringLiteral("width_mm"));
		d.height_mm = infoNumber(info, QStringLiteral("height_mm"));
		d.depth_mm = infoNumber(info, QStringLiteral("depth_mm"));
		d.step_file = info.value(QStringLiteral("step_file")).toString();
			//The outline: the real size from the insertion point (top left
			//corner of the device, like EPLAN's placement point), turned with
			//the element; without a size the drawn symbol
		if (d.width_mm > 0 && d.height_mm > 0) {
			QTransform t;
			t.translate(element->scenePos().x(), element->scenePos().y());
			t.rotate(element->rotation());
			r = t.mapRect(QRectF(0, 0, d.width_mm / k, d.height_mm / k));
		}
		d.rect_mm = QRectF(toMm(r.topLeft()), toMm(r.bottomRight())).normalized();
		if (d.width_mm <= 0) d.width_mm = d.rect_mm.width();
		if (d.height_mm <= 0) d.height_mm = d.rect_mm.height();
		d.patterns = patternsOf(element);

			//On a DIN rail when the rail runs through the device
		for (const PanelProfile &p : std::as_const(m_profiles)) {
			if (p.data.role != PanelData::DinRail) continue;
			if (sceneRectOf(p.shape).intersects(r)) {d.on_rail = true; break;}
		}

			//The mounting variant: chosen by hand (info mount_variant), else
			//the DIN rail one on a rail, else the first one with holes
		const QString chosen = info.value(QStringLiteral("mount_variant")).toString();
		int use = -1;
		for (int i = 0; i < d.patterns.size(); ++i)
			if (!chosen.isEmpty() && d.patterns.at(i).variant == chosen) use = i;
		if (use < 0)
			for (int i = 0; i < d.patterns.size(); ++i)
				if (d.patterns.at(i).din_rail == d.on_rail) {use = i; break;}
		bool has_rail_variant = false;
		for (const PanelPattern &p : std::as_const(d.patterns)) has_rail_variant |= p.din_rail;
		if (d.on_rail && !d.patterns.isEmpty() && !has_rail_variant)
			m_warnings << QStringLiteral("%1: liegt auf einer Hutschiene, hat aber keine Hutschienen-Variante")
							  .arg(d.designation);
		const bool touches_rail = d.on_rail;
		if (use >= 0) {
			d.variant = d.patterns.at(use).variant;
			d.on_rail = d.patterns.at(use).din_rail;
			if (touches_rail && !d.on_rail)
				m_warnings << QStringLiteral("%1: geschraubt (\"%2\"), liegt aber auf einer Hutschiene")
								  .arg(d.designation, d.variant);
			if (!d.on_rail && d.patterns.at(use).holes.isEmpty())
				m_warnings << QStringLiteral("%1: frei auf der Fläche, Variante \"%2\" ohne Bohrbild")
								  .arg(d.designation, d.variant);
		} else if (!d.on_rail && !d.designation.isEmpty()) {
			m_warnings << QStringLiteral("%1: frei auf der Fläche, aber kein Bohrbild im Bauteil")
							  .arg(d.designation);
		}
		m_devices << d;
	}

		//devices in reading order: top to bottom, left to right
	std::sort(m_devices.begin(), m_devices.end(), [](const PanelDevice &a, const PanelDevice &b) {
		if (qAbs(a.rect_mm.top() - b.rect_mm.top()) > 1) return a.rect_mm.top() > b.rect_mm.top();
		return a.rect_mm.left() < b.rect_mm.left();
	});
		//The contour of a template plate: its round holes are drilled with
		//the rest when the plate is made in house, or are there already
		//and keep everything else away from them
	if (!m_data.outline.isEmpty()) {
		const PanelOutline o = m_data.outline.stretched(m_width, m_height);
		const double W = m_width, H = m_height;
		const int origin = m_data.origin;
		auto map = [&](const QPointF &p) {
			switch (origin) {
				case 1:  return QPointF(p.x(), H - p.y());
				case 2:  return QPointF(W - p.x(), H - p.y());
				case 3:  return QPointF(W - p.x(), p.y());
				default: return p;
			}
		};
		m_outline.width = W;
		m_outline.height = H;
		m_outline.name = o.name;
		for (const QLineF &l : o.lines) m_outline.lines << QLineF(map(l.p1()), map(l.p2()));
		const QString source = o.name.isEmpty() ? QStringLiteral("Vorlage") : o.name;
		for (const PanelOutline::Arc &a : o.arcs) {
			PanelOutline::Arc b = a;
			b.centre = map(a.centre);
			if (a.isCircle()) {
				if (m_data.outline_machined) {
					PanelHole h;
					h.type = PanelHole::Drill;
					h.d1 = 2 * a.r;
					h.x = b.centre.x();
					h.y = b.centre.y();
					h.source = source;
					addHole(h);
				} else {
					m_keepouts << QRectF(b.centre - QPointF(a.r + 2, a.r + 2), QSizeF(2 * a.r + 4, 2 * a.r + 4));
				}
				continue;
			}
				//mirroring turns an arc round: new angles, the other way
			switch (origin) {
				case 1: b.a0 = -a.a1; b.a1 = -a.a0; break;
				case 2: b.a0 = a.a0 + 180; b.a1 = a.a1 + 180; break;
				case 3: b.a0 = 180 - a.a1; b.a1 = 180 - a.a0; break;
				default: break;
			}
			m_outline.arcs << b;
		}
	}

	for (const PanelProfile &p : std::as_const(m_profiles)) addProfileHoles(p);
	for (const PanelDevice &d : std::as_const(m_devices)) addDeviceHoles(d);
	for (QetShapeItem *shape : std::as_const(cutouts))
	{
		const PanelData data = shape->panelData();
		const QRectF r = sceneRectOf(shape);
		PanelHole h = data.cutout;
		const QPointF c = toMm(r.center());
		h.x = c.x();
		h.y = c.y();
		if ((h.type == PanelHole::Rectangle || h.type == PanelHole::Slot) && h.d1 <= 0) {
			h.d1 = r.width() * k;
			h.d2 = r.height() * k;
		}
		if (h.d1 <= 0) h.d1 = qMin(r.width(), r.height()) * k;
		h.source = data.designation.isEmpty() ? QStringLiteral("Durchbruch") : data.designation;
		addHole(h);
	}
}

/**
	@brief PanelLayout::addProfileHoles
	The fixing row of a length-variable part: from the zero end at the end
	distance, then every spacing, and always one at the end distance of the
	other end (EPLAN: repeat spacing, end distance, zero cut). Two rows sit
	symmetric to the centre line.
*/
void PanelLayout::addProfileHoles(const PanelProfile &p)
{
	const double L = p.length_mm;
	if (L <= 0) return;
	const double e = qMax(0.0, p.data.end_distance);
	const double s = p.data.spacing;
	QList<double> ts;
	if (L <= 2 * e + 1e-6) {
		ts << L / 2;
	} else {
		for (double t = e; t <= L - e + 1e-6; t += (s > 0 ? s : L)) ts << t;
		if (ts.isEmpty() || ts.last() < L - e - 1.0) ts << L - e;
	}
	if (p.data.zero_from_end)
		for (double &t : ts) t = L - t;

	const QPointF dir = (p.end_mm - p.start_mm) / L;
	const QPointF normal(-dir.y(), dir.x());
	QList<double> offsets;
	if (p.data.rows >= 2) {
		const double o = p.data.row_offset > 0 ? p.data.row_offset : p.width_mm / 4;
		offsets << -o << o;
	} else {
		offsets << 0;
	}
	const QString source = p.data.designation.isEmpty()
			? (p.data.role == PanelData::DinRail ? QStringLiteral("Hutschiene")
			   : p.data.role == PanelData::Duct ? QStringLiteral("Kanal") : QStringLiteral("Profil"))
			: p.data.designation;
	for (double t : std::as_const(ts))
		for (double o : std::as_const(offsets)) {
			PanelHole h = p.data.fixing;
			const QPointF c = p.start_mm + dir * t + normal * o;
			h.x = c.x();
			h.y = c.y();
			h.source = source;
			addHole(h);
		}
}

/**
	@brief PanelLayout::addDeviceHoles
	The drilling pattern of the mounting variant in use, turned with the
	device. Pattern coordinates: millimetres from the insertion point,
	x to the right, y upwards, as in EPLAN's drilling pattern.
*/
void PanelLayout::addDeviceHoles(const PanelDevice &d)
{
	if (d.on_rail) return;
	for (const PanelPattern &p : d.patterns)
	{
		if (p.variant != d.variant) continue;
		const double rad = qDegreesToRadians(d.angle);
		const double c = qCos(rad), s = qSin(rad);
		for (PanelHole h : p.holes)
		{
				//scene: y down, rotation clockwise; pattern: y up
			const double lx = h.x / m_data.mm_per_unit, ly = -h.y / m_data.mm_per_unit;
			const QPointF scene = d.element->scenePos() + QPointF(lx * c - ly * s, lx * s + ly * c);
			const QPointF mm = toMm(scene);
			h.x = mm.x();
			h.y = mm.y();
			h.angle += d.angle;
			h.source = d.designation;
			addHole(h);
		}
		return;
	}
}

void PanelLayout::addHole(PanelHole hole)
{
	if (hole.x < 0 || hole.y < 0 || hole.x > m_width || hole.y > m_height) {
		m_warnings << QStringLiteral("%1: Loch %2 bei (%3 | %4) liegt außerhalb der Fläche")
						  .arg(hole.source, hole.kindName(), fmt(hole.x), fmt(hole.y));
		return;
	}
	if (insideKeepout(QPointF(hole.x, hole.y))) {
		m_warnings << QStringLiteral("%1: Loch %2 bei (%3 | %4) liegt in einer Sperrfläche – nicht gebohrt")
						  .arg(hole.source, hole.kindName(), fmt(hole.x), fmt(hole.y));
		return;
	}
	for (const PanelHole &other : std::as_const(m_holes))
		if (QLineF(other.x, other.y, hole.x, hole.y).length() < (other.d1 + hole.d1) / 2) {
			m_warnings << QStringLiteral("%1: Loch %2 bei (%3 | %4) überschneidet sich mit %5")
							  .arg(hole.source, hole.kindName(), fmt(hole.x), fmt(hole.y), other.source);
			break;
		}
	m_holes << hole;
}

bool PanelLayout::insideKeepout(const QPointF &mm) const
{
	for (const QRectF &r : m_keepouts)
		if (r.contains(mm)) return true;
	return false;
}

/**
	@brief PanelLayout::cutList
	One line per article of length-variable parts: how many pieces of
	which length (rounded to the millimetre), like EPLAN's mounting list.
*/
QStringList PanelLayout::cutList() const
{
	QMap<QString, QList<int>> pieces;
	for (const PanelProfile &p : m_profiles)
	{
		const QString kind = p.data.role == PanelData::DinRail ? QStringLiteral("Hutschiene")
							 : p.data.role == PanelData::Duct ? QStringLiteral("Kanal %1 mm").arg(qRound(p.width_mm))
							 : QStringLiteral("C-Profil");
		const QString key = p.data.article.isEmpty() ? kind : QStringLiteral("%1 %2").arg(kind, p.data.article);
		pieces[key] << qRound(p.length_mm);
	}
	QStringList out;
	for (auto it = pieces.cbegin(); it != pieces.cend(); ++it)
	{
		QList<int> lengths = it.value();
		std::sort(lengths.begin(), lengths.end(), std::greater<int>());
		QStringList parts;
		int total = 0;
		for (int l : lengths) {parts << QStringLiteral("%1 mm").arg(l); total += l;}
		out << QStringLiteral("%1: %2 Stück (%3), gesamt %4 mm")
				   .arg(it.key()).arg(lengths.size()).arg(parts.join(QStringLiteral(", "))).arg(total);
	}
	return out;
}
