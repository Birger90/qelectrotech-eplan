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

	/// the free ways on the plate, for wires no duct takes: a grid of
	/// PanelWiring::kGridMm, the devices are in the way; a step along wires
	/// laid before costs less, so later wires follow them and form a bundle
	/// (OpenECraft #275 does the same in 3D); a turn costs half a step
class FreeGrid
{
public:
	explicit FreeGrid(const QList<PanelDevice> &devices) : m_devices(devices)
	{
		QRectF all;
		for (const PanelDevice &d : devices) all |= d.rect_mm.normalized();
		if (all.isEmpty()) return;
		m_step = PanelWiring::kGridMm;
		while ((all.width() / m_step + 5) * (all.height() / m_step + 5) > 60000) m_step *= 2;
		all.adjust(-2 * m_step, -2 * m_step, 2 * m_step, 2 * m_step);
		m_origin = all.topLeft();
		m_nx = int(std::ceil(all.width() / m_step)) + 1;
		m_ny = int(std::ceil(all.height() / m_step)) + 1;
		const int n = m_nx * m_ny;
		m_blocked.fill(false, n);
		m_h.fill(0, n); m_v.fill(0, n);
		m_h_mm2.fill(0, n); m_v_mm2.fill(0, n);
		for (const PanelDevice &d : devices) {
			const QRectF r = d.rect_mm.normalized().adjusted(0.5, 0.5, -0.5, -0.5);
			if (r.isEmpty()) continue;
			for (int y = 0; y < m_ny; ++y)
				for (int x = 0; x < m_nx; ++x)
					if (r.contains(at(node(x, y)))) m_blocked[node(x, y)] = true;
		}
	}

		/// lays one wire between two devices; its length device to device,
		/// mm, or -1 when no free way joins them
	double lay(int a, int b, double mm2)
	{
		const int from = entry(a), to = entry(b);
		if (from < 0 || to < 0) return -1;
		auto stub = [&](int device, int point) {
			const QPointF c = m_devices.at(device).rect_mm.normalized().center(), p = at(point);
			return qAbs(c.x() - p.x()) + qAbs(c.y() - p.y());
		};
		const double stubs = stub(a, from) + stub(b, to);
		if (from == to) return stubs;
			// states: point * 4 + direction (0 +x, 1 -x, 2 +y, 3 -y)
		const int states = m_nx * m_ny * 4;
		QVector<double> dist(states, kInf);
		QVector<int> prev(states, -1);
		using Item = std::pair<double, int>;
		std::priority_queue<Item, std::vector<Item>, std::greater<Item>> queue;
		for (int d = 0; d < 4; ++d) {dist[from * 4 + d] = 0; queue.push({0, from * 4 + d});}
		static const int dx[] = {1, -1, 0, 0}, dy[] = {0, 0, 1, -1};
		int end = -1;
		while (!queue.empty()) {
			const auto [cost, s] = queue.top();
			queue.pop();
			if (cost > dist[s]) continue;
			const int p = s / 4, dir = s % 4;
			if (p == to) {end = s; break;}
			const int x = p % m_nx, y = p / m_nx;
			for (int d = 0; d < 4; ++d) {
				const int nx = x + dx[d], ny = y + dy[d];
				if (nx < 0 || ny < 0 || nx >= m_nx || ny >= m_ny) continue;
				const int q = node(nx, ny);
				if (m_blocked.at(q) && q != to) continue;
				const int used = d < 2 ? m_h.at(node(qMin(x, nx), y)) : m_v.at(node(x, qMin(y, ny)));
				double step = m_step * (used > 0 ? PanelWiring::kBundleCost : 1.0);
				if (d != dir) step += m_step / 2;
				const int t = q * 4 + d;
				if (cost + step < dist[t]) {dist[t] = cost + step; prev[t] = s; queue.push({cost + step, t});}
			}
		}
		if (end < 0) return -1;
		int steps = 0;
		for (int s = end; prev[s] >= 0; s = prev[s]) {
			const int p = s / 4, q = prev[s] / 4;
			const int x = p % m_nx, y = p / m_nx, qx = q % m_nx, qy = q / m_nx;
			if (y == qy) {const int e = node(qMin(x, qx), y); ++m_h[e]; m_h_mm2[e] += mm2;}
			else {const int e = node(x, qMin(y, qy)); ++m_v[e]; m_v_mm2[e] += mm2;}
			++steps;
		}
		return steps * m_step + stubs;
	}

		/// the straight runs of the free ways, the cable ties and the
		/// thickest bundle into @p out
	void bundles(PanelWiring &out) const
	{
		if (!m_nx) return;
		auto scan = [&](bool horizontal) {
			const int lines = horizontal ? m_ny : m_nx, along = horizontal ? m_nx - 1 : m_ny - 1;
			for (int l = 0; l < lines; ++l) {
				auto edge = [&](int i) {return horizontal ? node(i, l) : node(l, i);};
				auto wires = [&](int i) {return horizontal ? m_h.at(edge(i)) : m_v.at(edge(i));};
				auto mm2 = [&](int i) {return horizontal ? m_h_mm2.at(edge(i)) : m_v_mm2.at(edge(i));};
				auto point = [&](int i) {return at(horizontal ? node(i, l) : node(l, i));};
				for (int i = 0; i < along;) {
					if (!wires(i)) {++i; continue;}
					int j = i;
					double most = 0;
					while (j < along && wires(j) == wires(i)) most = qMax(most, mm2(j++));
					PanelBundleRun run;
					run.line_mm = QLineF(point(i), point(j));
					run.wires = wires(i);
					run.diameter_mm = std::sqrt(most / 0.75);
					out.bundles << run;
					if (run.wires >= 2) out.max_bundle_mm = qMax(out.max_bundle_mm, run.diameter_mm);
					i = j;
				}
					// cable ties: along each stretch where two wires or more lie together
				for (int i = 0; i < along;) {
					if (wires(i) < 2) {++i; continue;}
					int j = i;
					while (j < along && wires(j) >= 2) ++j;
					out.cable_ties += qMax(1, int(std::lround((j - i) * m_step / PanelWiring::kTieSpacingMm)));
					i = j;
				}
			}
		};
		scan(true);
		scan(false);
	}

private:
	int node(int x, int y) const {return y * m_nx + x;}
	QPointF at(int n) const {return m_origin + QPointF((n % m_nx) * m_step, (n / m_nx) * m_step);}
		/// the free point nearest the outline of a device
	int entry(int device)
	{
		if (m_entry.contains(device)) return m_entry.value(device);
		const QRectF r = m_devices.at(device).rect_mm.normalized();
		int best = -1;
		double best_gap = kInf, best_centre = kInf;
		for (int n = 0; n < m_nx * m_ny; ++n) {
			if (m_blocked.at(n)) continue;
			const QPointF p = at(n);
			const double g = gap(QRectF(p, QSizeF(0, 0)), r);
			const double c = QLineF(p, r.center()).length();
			if (g < best_gap - 1e-6 || (g < best_gap + 1e-6 && c < best_centre)) {best = n; best_gap = g; best_centre = c;}
		}
		m_entry.insert(device, best);
		return best;
	}

	const QList<PanelDevice> &m_devices;
	double m_step = PanelWiring::kGridMm;
	QPointF m_origin;
	int m_nx = 0, m_ny = 0;
	QVector<bool> m_blocked;
	QVector<int> m_h, m_v;            ///< wires on the edge to the right / below of a point
	QVector<double> m_h_mm2, m_v_mm2;
	QHash<int, int> m_entry;
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
		//The number of each potential, from its first wire that has one
	QHash<Terminal *, QString> net_text;
	for (Diagram *folio : diagram->project()->folios()) {
		if (!PanelLayout::surfaces(folio).isEmpty()) continue;
		for (Conductor *c : folio->conductors()) {
			const QString text = c->properties().text.trimmed();
			Terminal *r = root(c->terminal1);
			if (!text.isEmpty() && text != QLatin1String("_") && !net_text.contains(r)) net_text.insert(r, text);
		}
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
	QList<double> net_s;
	QList<bool> net_assumed;
	QStringList net_wire;
	for (auto it = net_devices.constBegin(); it != net_devices.constEnd(); ++it) {
		if (it.value().size() < 2) continue;
		nets << it.value();
		double s = net_section.value(it.key());
		if (!net_with_section.contains(it.key()) || s <= 0) {
			s = kDefaultSection;
			out.assumed_section += int(it.value().size()) - 1;
			net_assumed << true;
		} else {
			net_assumed << false;
		}
		net_s << s;
		net_wire << net_text.value(it.key());
		const double d = outerDiameter(s);
		net_mm2 << d * d;
		if (!ducts.isEmpty()) for (int i : it.value()) entry(i);
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
	FreeGrid free(devices);
	auto addRun = [&](int n, int a, int b, double length, bool free_way, bool unrouted) {
		PanelWireRun run;
		run.order = int(out.runs.size()) + 1;
		run.wire = net_wire.value(n);
		run.from = devices.at(a).designation;
		run.to = devices.at(b).designation;
		run.section_mm2 = net_s.value(n);
		run.assumed_section = net_assumed.value(n);
		run.length_mm = length;
		run.free = free_way;
		run.unrouted = unrouted;
		out.runs << run;
	};
	for (int n = 0; n < nets.size(); ++n) {
		const QList<int> &net = nets.at(n);   // layout devices
		QList<int> in_tree{net.first()};
		QList<int> left = net.mid(1);
		while (!ducts.isEmpty() && !left.isEmpty()) {
			double best = kInf;
			int bi = -1, bt = -1, bd = -1;
			for (int t : std::as_const(in_tree)) {
				const int tp = device_point.value(t);
				from(tp);
				for (int j = 0; j < left.size(); ++j) {
					const double d = dist[tp].at(device_point.value(left.at(j)));
					if (d < best) {best = d; bi = j; bt = tp; bd = t;}
				}
			}
			if (bi < 0) break;
			const int target = left.takeAt(bi);
			in_tree << target;
			++out.wires;
			out.length_mm += best;
			addRun(n, bd, target, best, false, false);
			for (int p = device_point.value(target); p != bt && p >= 0; p = prev_point[bt].at(p)) {
				const int s = prev_stretch[bt].at(p);
				if (s >= 0) {used[s] += net_mm2.at(n); ++count[s];}
			}
		}
			// what no duct joins: laid on the plate, the shortest free way,
			// along the bundles laid before (Prim over the straight distances)
		while (!left.isEmpty()) {
			double best = kInf;
			int bi = -1, bt = -1;
			for (int t : std::as_const(in_tree))
				for (int j = 0; j < left.size(); ++j) {
					const QPointF a = devices.at(t).rect_mm.center(), b = devices.at(left.at(j)).rect_mm.center();
					const double d = qAbs(a.x() - b.x()) + qAbs(a.y() - b.y());
					if (d < best) {best = d; bi = j; bt = t;}
				}
			const int target = left.takeAt(bi);
			in_tree << target;
			const double length = free.lay(bt, target, net_mm2.at(n));
			if (length < 0) {++out.unrouted; addRun(n, bt, target, 0, true, true); continue;}
			addRun(n, bt, target, length, true, false);
			++out.free_wires;
			out.length_mm += length;
			out.free_length_mm += length;
		}
	}
	for (int s = 0; s < stretch_duct.size(); ++s) {
		PanelDuctFill &f = out.ducts[stretch_duct.at(s)];
		if (used.at(s) > f.used_mm2) {f.used_mm2 = used.at(s); f.wires = count.at(s);}
	}
	free.bundles(out);
	return out;
}

double PanelWireRun::cutLength(double reserve_mm) const
{
	if (unrouted) return 0;
	return std::ceil((length_mm + 2 * qMax(0.0, reserve_mm)) / 10.0) * 10.0;
}

QString PanelWireRun::ferrule() const
{
		// Phoenix Contact AI (DIN 46228-4, with collar): cross-section and
		// sleeve length in the type, catalogue values (FORMELN.md W)
	static const struct {double s; const char *type;} table[] = {
		{0.25, "AI 0,25-8"}, {0.34, "AI 0,34-8"}, {0.5, "AI 0,5-8"}, {0.75, "AI 0,75-8"},
		{1.0, "AI 1-8"}, {1.5, "AI 1,5-8"}, {2.5, "AI 2,5-8"}, {4, "AI 4-10"}, {6, "AI 6-12"},
		{10, "AI 10-12"}, {16, "AI 16-12"}, {25, "AI 25-16"}, {35, "AI 35-16"}};
	for (const auto &row : table)
		if (std::abs(row.s - section_mm2) < 1e-6) return QString::fromLatin1(row.type);
	return QString();
}

QString PanelWiring::wireListCsv(double reserve_mm) const
{
	const auto cell = [](QString s) {
		s.replace(QLatin1Char('"'), QStringLiteral("\"\""));
		return QStringLiteral("\"%1\"").arg(s);
	};
	QString out = QStringLiteral("Reihenfolge;Drahtnummer;Von;Nach;Querschnitt mm²;Aderendhülse;"
								 "Weg mm;Zuschnitt mm;Verlegung\r\n");
	for (const PanelWireRun &r : runs) {
		const QString way = r.unrouted ? QStringLiteral("ohne Weg")
						  : r.free ? QStringLiteral("frei gebündelt") : QStringLiteral("Kanal");
		out += QStringLiteral("%1;%2;%3;%4;%5;%6;%7;%8;%9\r\n").arg(r.order)
				.arg(cell(r.wire), cell(r.from), cell(r.to),
					 fmt(r.section_mm2, 2) + (r.assumed_section ? QStringLiteral(" (angenommen)") : QString()),
					 cell(r.ferrule()), fmt(r.length_mm), fmt(r.cutLength(reserve_mm)), way);
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
	if (ducts.isEmpty() && !free_wires && !unrouted) return QString();
	QStringList parts;
	if (!ducts.isEmpty()) {
		const PanelDuctFill *full = nullptr;
		for (const PanelDuctFill &f : ducts) if (!full || f.percent() > full->percent()) full = &f;
		QString text = QStringLiteral("%1 Leitungen durch die Kanäle (%2 m)")
						   .arg(wires).arg(fmt((length_mm - free_length_mm) / 1000, 1));
		if (full && full->used_mm2 > 0)
			text += QStringLiteral(", am vollsten %1 mit %2 %").arg(full->name, fmt(full->percent()));
		parts << text;
	}
	if (free_wires) {
		QString text = QStringLiteral("%1 Leitungen ohne Kanal auf der Platte gebündelt (%2 m")
						   .arg(free_wires).arg(fmt(free_length_mm / 1000, 1));
		if (cable_ties) text += QStringLiteral(", %1 Kabelbinder, Bündel bis %2 mm").arg(cable_ties).arg(fmt(max_bundle_mm));
		parts << text + QLatin1Char(')');
	}
	if (assumed_section) parts << QStringLiteral("%1 ohne Querschnitt (1,5 mm² angenommen)").arg(assumed_section);
	if (unrouted) parts << QStringLiteral("%1 ohne Weg (von Geräten eingeschlossen)").arg(unrouted);
	return QStringLiteral("Verdrahtung: ") + parts.join(QStringLiteral(", "));
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
	QJsonArray runs;
	for (const PanelBundleRun &b : bundles)
		runs.append(QJsonObject{{QStringLiteral("x1"), b.line_mm.x1()}, {QStringLiteral("y1"), b.line_mm.y1()},
								{QStringLiteral("x2"), b.line_mm.x2()}, {QStringLiteral("y2"), b.line_mm.y2()},
								{QStringLiteral("wires"), b.wires},
								{QStringLiteral("diameter_mm"), qRound(b.diameter_mm * 10) / 10.0}});
	QJsonArray wire_runs;
	for (const PanelWireRun &r : this->runs)
		wire_runs.append(QJsonObject{{QStringLiteral("order"), r.order},
									 {QStringLiteral("wire"), r.wire},
									 {QStringLiteral("from"), r.from},
									 {QStringLiteral("to"), r.to},
									 {QStringLiteral("section_mm2"), r.section_mm2},
									 {QStringLiteral("assumed_section"), r.assumed_section},
									 {QStringLiteral("ferrule"), r.ferrule()},
									 {QStringLiteral("length_mm"), qRound(r.length_mm)},
									 {QStringLiteral("free"), r.free},
									 {QStringLiteral("unrouted"), r.unrouted}});
	return QJsonObject{{QStringLiteral("ducts"), list},
					   {QStringLiteral("runs"), wire_runs},
					   {QStringLiteral("bundles"), runs},
					   {QStringLiteral("wires"), wires},
					   {QStringLiteral("free_wires"), free_wires},
					   {QStringLiteral("free_length_mm"), qRound(free_length_mm)},
					   {QStringLiteral("cable_ties"), cable_ties},
					   {QStringLiteral("max_bundle_mm"), qRound(max_bundle_mm * 10) / 10.0},
					   {QStringLiteral("unrouted"), unrouted},
					   {QStringLiteral("assumed_section"), assumed_section},
					   {QStringLiteral("length_mm"), qRound(length_mm)},
					   {QStringLiteral("limit_percent"), kWarnPercent},
					   {QStringLiteral("warnings"), QJsonArray::fromStringList(warnings())},
					   {QStringLiteral("summary"), summary()}};
}
