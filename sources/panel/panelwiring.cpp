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
#include "panelwiring.h"

#include "panellayout.h"
#include "panelmenu.h"
#include "../TerminalStrip/realterminal.h"
#include "../TerminalStrip/terminalstrip.h"
#include "../diagram.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/qetshapeitem.h"
#include "../qetgraphicsitem/terminal.h"
#include "../qetgraphicsitem/terminalelement.h"
#include "../qetproject.h"

#include <QHash>
#include <QJsonArray>
#include <QLineF>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>

namespace {

const double kInf = std::numeric_limits<double>::infinity();

QString fmt(double v, int decimals = 0)
{
	return QString::number(v, 'f', decimals).replace(QLatin1Char('.'), QLatin1Char(','));
}

	/// the cross-section in mm² written on a conductor ("1,5", "2.5 mm²",
	/// "3x1,5"); 0 when none
double sectionOf(const QString &text)
{
	static const QRegularExpression times(QStringLiteral("[xX×]"));
	static const QRegularExpression value(QStringLiteral("[0-9]+(?:[.,][0-9]+)?"));
	QString t = text.trimmed();
	const int x = t.lastIndexOf(times);
	if (x >= 0) t = t.mid(x + 1);
	const QRegularExpressionMatch m = value.match(t);
	return m.hasMatch() ? m.captured(0).replace(QLatin1Char(','), QLatin1Char('.')).toDouble() : 0;
}

QString stripKey(QString name)
{
	name = name.trimmed();
	while (name.startsWith(QLatin1Char('-'))) name.remove(0, 1);
	return QStringLiteral("strip:") + name.toUpper();
}

	/// the nearest points of two segments (2D)
QLineF nearest(const QLineF &a, const QLineF &b)
{
	auto onto = [](const QPointF &p, const QLineF &s) {
		const QPointF d = s.p2() - s.p1();
		const double l2 = d.x() * d.x() + d.y() * d.y();
		if (l2 <= 0) return s.p1();
		const double t = qBound(0.0, ((p.x() - s.x1()) * d.x() + (p.y() - s.y1()) * d.y()) / l2, 1.0);
		return s.p1() + t * d;
	};
	QPointF cross;
	if (a.intersects(b, &cross) == QLineF::BoundedIntersection) return QLineF(cross, cross);
	QLineF best(a.p1(), onto(a.p1(), b));
	for (const QLineF &c : {QLineF(a.p2(), onto(a.p2(), b)), QLineF(onto(b.p1(), a), b.p1()),
							QLineF(onto(b.p2(), a), b.p2())})
		if (c.length() < best.length()) best = c;
	return best;
}

	/// the gap between two rectangles (0 when they touch or overlap)
double gap(const QRectF &a, const QRectF &b)
{
	const double dx = qMax(0.0, qMax(a.left() - b.right(), b.left() - a.right()));
	const double dy = qMax(0.0, qMax(a.top() - b.bottom(), b.top() - a.bottom()));
	return std::hypot(dx, dy);
}

	/// the route network: points on the duct centre lines, joined along
	/// each duct (a stretch) and where ducts meet; devices hang on it
struct Graph
{
	struct Edge {int to; double w; int stretch;};   // stretch -1: not inside a duct
	QList<QPointF> points;
	QList<QList<Edge>> edges;
	int add(const QPointF &p) {points << p; edges << QList<Edge>(); return int(points.size()) - 1;}
	void join(int a, int b, double w, int stretch) {
		edges[a] << Edge{b, w, stretch};
		edges[b] << Edge{a, w, stretch};
	}
		/// shortest distances from @p from, with the edge each point is reached by
	void dijkstra(int from, QVector<double> &dist, QVector<int> &prev_point, QVector<int> &prev_stretch) const {
		const int n = int(points.size());
		dist.fill(kInf, n);
		prev_point.fill(-1, n);
		prev_stretch.fill(-1, n);
		using Item = std::pair<double, int>;
		std::priority_queue<Item, std::vector<Item>, std::greater<Item>> queue;
		dist[from] = 0;
		queue.push({0, from});
		while (!queue.empty()) {
			const auto [d, u] = queue.top();
			queue.pop();
			if (d > dist[u]) continue;
			for (const Edge &e : edges.at(u)) {
				const double nd = d + e.w;
				if (nd < dist[e.to]) {
					dist[e.to] = nd;
					prev_point[e.to] = u;
					prev_stretch[e.to] = e.stretch;
					queue.push({nd, e.to});
				}
			}
		}
	}
};

} // namespace

/* ------------------------------------------------------------------ */

double PanelWiring::outerDiameter(double s)
{
		// mm² -> mm, guide values of single core PVC wires (H05V-K up to
		// 1 mm², H07V-K above), between them linear
	static const double table[][2] = {
		{0.5, 2.1}, {0.75, 2.3}, {1.0, 2.5}, {1.5, 3.1}, {2.5, 3.7}, {4, 4.4}, {6, 4.9},
		{10, 6.3}, {16, 7.5}, {25, 9.4}, {35, 10.8}, {50, 13.0}, {70, 15.0}, {95, 17.0}, {120, 19.0}};
	const int n = int(sizeof(table) / sizeof(table[0]));
	if (s <= table[0][0]) return table[0][1];
	for (int i = 1; i < n; ++i)
		if (s <= table[i][0]) {
			const double t = (s - table[i - 1][0]) / (table[i][0] - table[i - 1][0]);
			return table[i - 1][1] + t * (table[i][1] - table[i - 1][1]);
		}
	return table[n - 1][1] * std::sqrt(s / table[n - 1][0]);
}

PanelWiring PanelWiring::route(const PanelLayout &layout)
{
	PanelWiring out;
	QetShapeItem *surface = layout.surface();
	auto *diagram = surface ? qobject_cast<Diagram *>(surface->scene()) : nullptr;
	if (!diagram || !diagram->project()) return out;

		// the ducts: centre line, room inside
	struct Duct {QLineF line; QRectF rect;};
	QList<Duct> ducts;
	for (const PanelProfile &p : layout.profiles()) {
		if (p.data.role != PanelData::Duct) continue;
		PanelDuctFill f;
		f.duct = p.shape;
		f.name = p.data.designation.isEmpty() ? QStringLiteral("Kabelkanal") : p.data.designation;
		f.width_mm = p.width_mm;
		f.depth_mm = p.data.profile_depth > 0 ? p.data.profile_depth : 60;
		f.inner_mm2 = qMax(0.0, f.width_mm - 4) * qMax(0.0, f.depth_mm - 4);
		out.ducts << f;
		const QLineF line(p.start_mm, p.end_mm);
		const double half = p.width_mm / 2;
		const QRectF r = QRectF(p.start_mm, p.end_mm).normalized();
		ducts << Duct{line, qAbs(line.dx()) >= qAbs(line.dy()) ? r.adjusted(0, -half, 0, half)
															 : r.adjusted(-half, 0, half, 0)};
	}
	if (ducts.isEmpty()) return out;

		// the devices of this surface by what they stand for
	const QList<PanelDevice> devices = layout.devices();
	QHash<QString, int> by_key, by_label;
	for (int i = 0; i < devices.size(); ++i) {
		const PanelDevice &d = devices.at(i);
		if (!d.element) continue;
		const QString of = d.element->elementInformations().value(QStringLiteral("panel_of")).toString();
		if (of.startsWith(QLatin1String("strip:"))) by_key.insert(stripKey(of.mid(6)), i);
		else if (!of.isEmpty()) by_key.insert(of, i);
		if (!d.designation.isEmpty()) by_label.insert(d.designation, i);
	}
	auto deviceOf = [&](Element *e) -> int {
		if (auto *te = dynamic_cast<TerminalElement *>(e))
			if (te->realTerminal() && te->realTerminal()->parentStrip())
				return by_key.value(stripKey(te->realTerminal()->parentStrip()->name()), -1);
		Element *holder = PanelMenu::partHolder(e);
		const PanelLayout::DeviceTag tag = PanelLayout::deviceTag(holder);
		int i = by_key.value(tag.key(), -1);
		if (i < 0) i = by_label.value(holder->actualLabel().trimmed(), -1);
		if (i < 0 && !tag.bmk.isEmpty()) i = by_label.value(tag.bmk, -1);
		return i;
	};

		// the potentials of the schematic: terminals joined by conductors,
		// junctions and folio reports
	QHash<Terminal *, Terminal *> parent;
	std::function<Terminal *(Terminal *)> root = [&](Terminal *t) -> Terminal * {
		Terminal *p = parent.value(t, t);
		if (p == t) return t;
		Terminal *r = root(p);
		parent.insert(t, r);
		return r;
	};
	auto unite = [&](Terminal *a, Terminal *b) {
		if (!a || !b) return;
		Terminal *ra = root(a), *rb = root(b);
		if (ra != rb) parent.insert(ra, rb);
	};
	QList<QPair<Terminal *, double>> sections;
	QList<QPair<Element *, int>> members;   // device elements of the schematic and their layout device
	for (Diagram *folio : diagram->project()->folios()) {
		if (!PanelLayout::surfaces(folio).isEmpty()) continue;
		for (Conductor *c : folio->conductors()) {
			unite(c->terminal1, c->terminal2);
			sections << qMakePair(c->terminal1, sectionOf(c->properties().m_wire_section));
		}
		for (Element *e : folio->elements()) {
			const int kind = e->linkType();
			if (kind & (Element::AllReport | Element::Junction)) {
				const QList<Terminal *> own = e->terminals();
				for (int i = 1; i < own.size(); ++i) unite(own.first(), own.at(i));
				if (kind & Element::AllReport)
					for (Element *other : e->linkedElements())
						if (!own.isEmpty() && !other->terminals().isEmpty()) unite(own.first(), other->terminals().first());
				continue;
			}
			if (kind & (Element::Thumbnail | Element::ConductorDefinition)) continue;
			const int i = deviceOf(e);
			if (i >= 0) members << qMakePair(e, i);
		}
	}
	QHash<Terminal *, QList<int>> net_devices;
	for (const auto &m : std::as_const(members))
		for (Terminal *t : m.first->terminals()) {
			QList<int> &list = net_devices[root(t)];
			if (!list.contains(m.second)) list << m.second;
		}
	QHash<Terminal *, double> net_section;
	QSet<Terminal *> net_with_section;
	for (const auto &s : std::as_const(sections)) {
		Terminal *r = root(s.first);
		if (s.second > 0) net_with_section << r;
		net_section[r] = qMax(net_section.value(r), s.second);
	}

		// the route network
	Graph g;
	QList<QList<QPair<double, int>>> on_duct(ducts.size());   // position along the duct, point
	auto place = [&](int k, const QPointF &p) {
		const int id = g.add(p);
		on_duct[k] << qMakePair(QLineF(ducts.at(k).line.p1(), p).length(), id);
		return id;
	};
	for (int a = 0; a < ducts.size(); ++a)
		for (int b = a + 1; b < ducts.size(); ++b) {
			if (!ducts.at(a).rect.adjusted(-2, -2, 2, 2).intersects(ducts.at(b).rect)) continue;
			const QLineF n = nearest(ducts.at(a).line, ducts.at(b).line);
			g.join(place(a, n.p1()), place(b, n.p2()), n.length(), -1);
		}
	QHash<int, int> device_point;   // layout device -> its point
	auto entry = [&](int i) {
		if (device_point.contains(i)) return device_point.value(i);
		const QRectF r = devices.at(i).rect_mm.normalized();
		int best = -1;
		double best_gap = kInf;
		for (int k = 0; k < ducts.size(); ++k) {
			const double d = gap(r, ducts.at(k).rect);
			if (d < best_gap) {best_gap = d; best = k;}
		}
		const QLineF to = nearest(QLineF(r.center(), r.center()), ducts.at(best).line);
		const int dev = g.add(r.center());
		g.join(dev, place(best, to.p2()), to.length(), -1);
		device_point.insert(i, dev);
		return dev;
	};
	QList<QList<int>> nets;
	QList<double> net_mm2;
	for (auto it = net_devices.constBegin(); it != net_devices.constEnd(); ++it) {
		if (it.value().size() < 2) continue;
		nets << it.value();
		double s = net_section.value(it.key());
		if (!net_with_section.contains(it.key()) || s <= 0) {
			s = kDefaultSection;
			out.assumed_section += int(it.value().size()) - 1;
		}
		const double d = outerDiameter(s);
		net_mm2 << d * d;
		for (int i : it.value()) entry(i);
	}
	QList<int> stretch_duct;
	for (int k = 0; k < ducts.size(); ++k) {
		auto &list = on_duct[k];
		list << qMakePair(0.0, -1) << qMakePair(ducts.at(k).line.length(), -1);
		for (auto &p : list) if (p.second < 0) p.second = g.add(ducts.at(k).line.pointAt(p.first / qMax(1e-9, ducts.at(k).line.length())));
		std::sort(list.begin(), list.end(), [](const auto &x, const auto &y) {return x.first < y.first;});
		for (int j = 1; j < list.size(); ++j) {
			stretch_duct << k;
			g.join(list.at(j - 1).second, list.at(j).second, list.at(j).first - list.at(j - 1).first,
				   int(stretch_duct.size()) - 1);
		}
	}

		// each potential: its shortest wires (Prim over the route lengths)
	QHash<int, QVector<double>> dist;
	QHash<int, QVector<int>> prev_point, prev_stretch;
	auto from = [&](int point) {
		if (!dist.contains(point)) g.dijkstra(point, dist[point], prev_point[point], prev_stretch[point]);
	};
	QVector<double> used(stretch_duct.size(), 0);
	QVector<int> count(stretch_duct.size(), 0);
	for (int n = 0; n < nets.size(); ++n) {
		const QList<int> &net = nets.at(n);
		QList<int> points;
		for (int i : net) points << device_point.value(i);
		QList<int> in_tree{points.first()};
		QList<int> left = points.mid(1);
		while (!left.isEmpty()) {
			double best = kInf;
			int bi = -1, bt = -1;
			for (int t : std::as_const(in_tree)) {
				from(t);
				for (int j = 0; j < left.size(); ++j)
					if (dist[t].at(left.at(j)) < best) {best = dist[t].at(left.at(j)); bi = j; bt = t;}
			}
			if (bi < 0) {out.unrouted += int(left.size()); break;}
			const int target = left.takeAt(bi);
			in_tree << target;
			++out.wires;
			out.length_mm += best;
			for (int p = target; p != bt && p >= 0; p = prev_point[bt].at(p)) {
				const int s = prev_stretch[bt].at(p);
				if (s >= 0) {used[s] += net_mm2.at(n); ++count[s];}
			}
		}
	}
	for (int s = 0; s < stretch_duct.size(); ++s) {
		PanelDuctFill &f = out.ducts[stretch_duct.at(s)];
		if (used.at(s) > f.used_mm2) {f.used_mm2 = used.at(s); f.wires = count.at(s);}
	}
	return out;
}

QStringList PanelWiring::warnings() const
{
	QStringList list;
	for (const PanelDuctFill &f : ducts)
		if (f.percent() >= kWarnPercent)
			list << QStringLiteral("%1: Kanal zu %2 % gefüllt (%3 Leitungen, %4 von %5 mm², Grenze %6 %)")
						.arg(f.name, fmt(f.percent())).arg(f.wires)
						.arg(fmt(f.used_mm2), fmt(f.inner_mm2), fmt(kWarnPercent));
	return list;
}

QString PanelWiring::summary() const
{
	if (ducts.isEmpty()) return QString();
	const PanelDuctFill *full = nullptr;
	for (const PanelDuctFill &f : ducts) if (!full || f.percent() > full->percent()) full = &f;
	QString text = QStringLiteral("Kanäle: %1 Leitungen durch die Kanäle (%2 m)").arg(wires).arg(fmt(length_mm / 1000, 1));
	if (full && full->used_mm2 > 0)
		text += QStringLiteral(", am vollsten %1 mit %2 %").arg(full->name, fmt(full->percent()));
	if (assumed_section) text += QStringLiteral(", %1 ohne Querschnitt (1,5 mm² angenommen)").arg(assumed_section);
	if (unrouted) text += QStringLiteral(", %1 ohne Kanalweg").arg(unrouted);
	return text;
}

QJsonObject PanelWiring::toJson() const
{
	QJsonArray list;
	for (const PanelDuctFill &f : ducts)
		list.append(QJsonObject{{QStringLiteral("name"), f.name},
								{QStringLiteral("width_mm"), f.width_mm},
								{QStringLiteral("depth_mm"), f.depth_mm},
								{QStringLiteral("inner_mm2"), qRound(f.inner_mm2 * 10) / 10.0},
								{QStringLiteral("used_mm2"), qRound(f.used_mm2 * 10) / 10.0},
								{QStringLiteral("wires"), f.wires},
								{QStringLiteral("percent"), qRound(f.percent() * 10) / 10.0}});
	return QJsonObject{{QStringLiteral("ducts"), list},
					   {QStringLiteral("wires"), wires},
					   {QStringLiteral("unrouted"), unrouted},
					   {QStringLiteral("assumed_section"), assumed_section},
					   {QStringLiteral("length_mm"), qRound(length_mm)},
					   {QStringLiteral("limit_percent"), kWarnPercent},
					   {QStringLiteral("warnings"), QJsonArray::fromStringList(warnings())},
					   {QStringLiteral("summary"), summary()}};
}
