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
#include "panelimport.h"
#include "../import/portals/manufacturerportals.h"

#include "../container/miniz/miniz.h"
#include "../import/eplandxf/dataportalclient.h"
#include "../qetapp.h"

#include <QDirIterator>
#include <QDomDocument>

#include <QDialogButtonBox>
#include <QEventLoop>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHash>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QTransform>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QtMath>

#include <functional>

#include <algorithm>

namespace {

struct Pair {int code; QString value;};

	/// one DXF entity as its group code pairs
struct Entity
{
	QString type;
	QVector<Pair> pairs;
	double num(int code, double fallback = 0) const {
		for (const Pair &p : pairs) if (p.code == code) return p.value.toDouble();
		return fallback;
	}
	QString str(int code) const {
		for (const Pair &p : pairs) if (p.code == code) return p.value;
		return QString();
	}
};

struct Geometry
{
	QList<QLineF> lines;
	QList<PanelOutline::Arc> arcs;
};

	/// the transform of an INSERT: scale, turn, move; mirrored when one
	/// scale is negative
struct Placement
{
	QTransform t;
	double scale = 1;
	bool mirrored = false;
};

	/// the arc of a polyline segment from a to b with a bulge
	/// (counter-clockwise for a positive bulge)
QPointF bulgeCentre(const QPointF &a, const QPointF &b, double bulge, double *r, double *a0, double *a1)
{
	const double theta = 4 * qAtan(qAbs(bulge));
	const double c = QLineF(a, b).length();
	*r = c / (2 * qSin(theta / 2));
	const QPointF mid = (a + b) / 2;
	QPointF n(-(b.y() - a.y()) / c, (b.x() - a.x()) / c);   // left of a->b
	if (bulge < 0) n = -n;
		//signed: beyond a half circle the centre lies on the other side
	const QPointF centre = mid + n * (*r * qCos(theta / 2));
	double s = qRadiansToDegrees(qAtan2(a.y() - centre.y(), a.x() - centre.x()));
	double e = qRadiansToDegrees(qAtan2(b.y() - centre.y(), b.x() - centre.x()));
	if (bulge < 0) std::swap(s, e);
	*a0 = s;
	*a1 = e;
	return centre;
}

class Reader
{
	public:
		bool parse(const QByteArray &data, QString *error)
		{
			const QStringList text = QString::fromLatin1(data).split(QLatin1Char('\n'));
			QVector<Pair> pairs;
			for (int i = 0; i + 1 < text.size(); i += 2) {
				bool ok = false;
				const int code = text.at(i).trimmed().toInt(&ok);
				if (!ok) {if (error) *error = QStringLiteral("Not an ASCII DXF"); return false;}
				pairs.append({code, text.at(i + 1).trimmed()});
			}
				//layers that are switched off (negative colour) or frozen: what
				//lies on them is not drawn (EPLAN_Invisible, for one)
			for (int i = 0; i < pairs.size(); ++i) {
				if (pairs.at(i).code != 0 || pairs.at(i).value != QLatin1String("LAYER")) continue;
				QString name;
				int colour = 7, flags = 0;
				for (int j = i + 1; j < pairs.size() && pairs.at(j).code != 0; ++j) {
					if (pairs.at(j).code == 2) name = pairs.at(j).value;
					else if (pairs.at(j).code == 62) colour = pairs.at(j).value.toInt();
					else if (pairs.at(j).code == 70) flags = pairs.at(j).value.toInt();
				}
				if (!name.isEmpty() && (colour < 0 || (flags & 1))) m_hidden << name;
			}
			QString section;
			Entity *current = nullptr;
			QVector<Entity> *target = nullptr;
			for (const Pair &p : pairs) {
				if (p.code == 0) {
					if (p.value == QLatin1String("SECTION") || p.value == QLatin1String("ENDSEC")) {
						section.clear(); current = nullptr; target = nullptr; m_block_pending = false;
						continue;
					}
					if (section == QLatin1String("BLOCKS") && p.value == QLatin1String("BLOCK")) {
						m_block_pending = true;
						m_block_header = Entity{p.value, {}};
						current = &m_block_header;
						target = nullptr;
						continue;
					}
					if (section == QLatin1String("BLOCKS") && p.value == QLatin1String("ENDBLK")) {
						target = nullptr; current = nullptr; m_block_pending = false;
						continue;
					}
					if (m_block_pending) {
						const QString block = m_block_header.str(2);
						m_bases[block] = QPointF(m_block_header.num(10), m_block_header.num(20));
						m_block_pending = false;
						target = &m_blocks[block];
					}
					if (section == QLatin1String("ENTITIES")) target = &m_entities;
					if (!target) {current = nullptr; continue;}
					target->append(Entity{p.value, {}});
					current = &target->last();
					continue;
				}
				if (p.code == 2 && section.isEmpty() && !current) {section = p.value; continue;}
				if (current) current->pairs.append(p);
			}
			if (m_entities.isEmpty()) {if (error) *error = QStringLiteral("The DXF holds no drawing"); return false;}
			collect(m_entities, Placement(), 0);
			return true;
		}

		Geometry geometry;
		QRectF symbol;   ///< what lies on EPLAN_Symbol: the device's outline

	private:
		bool m_symbol_layer = false;
		void addLine(const Placement &pl, QPointF a, QPointF b)
		{
			const QLineF l(pl.t.map(a), pl.t.map(b));
			geometry.lines << l;
			if (m_symbol_layer) symbol |= QRectF(l.p1(), l.p2()).normalized().adjusted(-1e-6, -1e-6, 1e-6, 1e-6);
		}
		void addArc(const Placement &pl, QPointF c, double r, double a0, double a1)
		{
			PanelOutline::Arc arc;
			arc.centre = pl.t.map(c);
			arc.r = r * pl.scale;
			if (qAbs(qAbs(a1 - a0) - 360) < 1e-6) {
				arc.a0 = 0; arc.a1 = 360;
			} else {
					//the angles turn with the placement; a mirror reverses them
				const QPointF ps = pl.t.map(c + QPointF(qCos(qDegreesToRadians(a0)), qSin(qDegreesToRadians(a0))) * r);
				const QPointF pe = pl.t.map(c + QPointF(qCos(qDegreesToRadians(a1)), qSin(qDegreesToRadians(a1))) * r);
				double s = qRadiansToDegrees(qAtan2(ps.y() - arc.centre.y(), ps.x() - arc.centre.x()));
				double e = qRadiansToDegrees(qAtan2(pe.y() - arc.centre.y(), pe.x() - arc.centre.x()));
				if (pl.mirrored) std::swap(s, e);
				arc.a0 = s; arc.a1 = e;
			}
			geometry.arcs << arc;
			if (m_symbol_layer) symbol |= QRectF(arc.centre - QPointF(arc.r, arc.r), QSizeF(2 * arc.r, 2 * arc.r));
		}
		void polyline(const Placement &pl, const QVector<QPointF> &pts, const QVector<double> &bulges, bool closed)
		{
			const int n = pts.size();
			for (int i = 0; i < n; ++i) {
				if (i == n - 1 && !closed) break;
				const QPointF a = pts.at(i), b = pts.at((i + 1) % n);
				if (QLineF(a, b).length() < 1e-9) continue;
				const double bulge = bulges.value(i);
				if (qAbs(bulge) < 1e-9) {addLine(pl, a, b); continue;}
				double r, a0, a1;
				const QPointF c = bulgeCentre(a, b, bulge, &r, &a0, &a1);
				addArc(pl, c, r, a0, a1);
			}
		}

		void collect(const QVector<Entity> &entities, const Placement &pl, int depth)
		{
			for (int i = 0; i < entities.size(); ++i) {
				const Entity &e = entities.at(i);
				if (!m_hidden.isEmpty() && m_hidden.contains(e.str(8))) {
						// a hidden polyline takes its vertices along
					if (e.type == QLatin1String("POLYLINE"))
						while (i + 1 < entities.size() && entities.at(i + 1).type != QLatin1String("SEQEND")) ++i;
					continue;
				}
				m_symbol_layer = e.str(8).compare(QLatin1String("EPLAN_Symbol"), Qt::CaseInsensitive) == 0;
				if (e.type == QLatin1String("LINE")) {
					addLine(pl, QPointF(e.num(10), e.num(20)), QPointF(e.num(11), e.num(21)));
				} else if (e.type == QLatin1String("CIRCLE")) {
					addArc(pl, QPointF(e.num(10), e.num(20)), e.num(40), 0, 360);
				} else if (e.type == QLatin1String("ARC")) {
					addArc(pl, QPointF(e.num(10), e.num(20)), e.num(40), e.num(50), e.num(51));
				} else if (e.type == QLatin1String("ELLIPSE")) {
						//centre, end of the major axis (relative), ratio, start and
						//end parameter: as a chain of short lines
					const QPointF c(e.num(10), e.num(20)), major(e.num(11), e.num(21));
					const double ratio = e.num(40, 1), t0 = e.num(41, 0), t1 = e.num(42, 2 * M_PI);
					const QPointF minor(-major.y() * ratio, major.x() * ratio);
					double span = t1 - t0;
					while (span <= 0) span += 2 * M_PI;
					const int n = qBound(4, int(span / (2 * M_PI) * 48), 48);
					QPointF prev = c + major * qCos(t0) + minor * qSin(t0);
					for (int k = 1; k <= n; ++k) {
						const double t = t0 + span * k / n;
						const QPointF p = c + major * qCos(t) + minor * qSin(t);
						addLine(pl, prev, p);
						prev = p;
					}
				} else if (e.type == QLatin1String("LWPOLYLINE")) {
					QVector<QPointF> pts;
					QVector<double> bulges;
					double x = 0;
					for (const Pair &p : e.pairs) {
						if (p.code == 10) x = p.value.toDouble();
						else if (p.code == 20) {pts << QPointF(x, p.value.toDouble()); bulges << 0;}
						else if (p.code == 42 && !bulges.isEmpty()) bulges.last() = p.value.toDouble();
					}
					polyline(pl, pts, bulges, int(e.num(70)) & 1);
				} else if (e.type == QLatin1String("POLYLINE")) {
					QVector<QPointF> pts;
					QVector<double> bulges;
					int j = i + 1;
					for (; j < entities.size() && entities.at(j).type == QLatin1String("VERTEX"); ++j) {
						pts << QPointF(entities.at(j).num(10), entities.at(j).num(20));
						bulges << entities.at(j).num(42);
					}
					polyline(pl, pts, bulges, int(e.num(70)) & 1);
					i = j;   // SEQEND
				} else if (e.type == QLatin1String("INSERT") && depth < 8) {
					const QString name = e.str(2);
					if (!m_blocks.contains(name)) continue;
					const double sx = e.num(41, 1), sy = e.num(42, 1), rot = e.num(50);
					const QPointF base = m_bases.value(name);
					QTransform local;
					local.translate(e.num(10), e.num(20));
					local.rotate(rot);
					local.scale(sx, sy);
					local.translate(-base.x(), -base.y());
					Placement inner;
					inner.t = local * pl.t;
					inner.scale = pl.scale * qAbs(sx);
					inner.mirrored = pl.mirrored != ((sx < 0) != (sy < 0));
					collect(m_blocks.value(name), inner, depth + 1);
				}
			}
		}

		QVector<Entity> m_entities;
		QHash<QString, QVector<Entity>> m_blocks;
		QHash<QString, QPointF> m_bases;
		QSet<QString> m_hidden;
		Entity m_block_header;
		bool m_block_pending = false;
};

	/// the extent of an arc: its end points and the quadrant points it passes
QRectF arcBounds(const PanelOutline::Arc &a)
{
	if (a.isCircle()) return QRectF(a.centre - QPointF(a.r, a.r), QSizeF(2 * a.r, 2 * a.r));
	QRectF r;
	auto add = [&](double deg) {
		const QPointF p = a.centre + QPointF(qCos(qDegreesToRadians(deg)), qSin(qDegreesToRadians(deg))) * a.r;
		const QRectF pr(p, QSizeF(1e-9, 1e-9));
		r = r.isNull() ? pr : r.united(pr);
	};
	double s = a.a0, e = a.a1;
	while (e < s) e += 360;
	add(s);
	add(e);
	for (int q = int(qCeil(s / 90)) * 90; q < e; q += 90) add(q);
	return r;
}

QString safeName(QString s)
{
	s.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]")), QStringLiteral("_"));
		//Windows drops a trailing blank or dot of a folder name: none at the end
	s = s.simplified().left(120);
	while (s.endsWith(QLatin1Char(' ')) || s.endsWith(QLatin1Char('.'))) s.chop(1);
	return s;
}

} // namespace

/* ------------------------------------------------------------------ */

PanelOutline PanelImport::readDxf(const QByteArray &dxf, QString *error)
{
	PanelOutline out;
	Reader reader;
	if (!reader.parse(dxf, error)) return out;
	double x0 = 1e300, y0 = 1e300, x1 = -1e300, y1 = -1e300;
	auto grow = [&](const QRectF &r) {
		x0 = qMin(x0, r.left()); y0 = qMin(y0, r.top());
		x1 = qMax(x1, r.right()); y1 = qMax(y1, r.bottom());
	};
	for (const QLineF &l : reader.geometry.lines) grow(QRectF(l.p1(), l.p2()).normalized());
	for (const PanelOutline::Arc &a : reader.geometry.arcs) grow(arcBounds(a));
	if (x1 - x0 <= 0 || y1 - y0 <= 0) {
		if (error) *error = QStringLiteral("The DXF has no lines or arcs");
		return out;
	}
	const QPointF o(x0, y0);   // smallest x and y: the lower left corner
	for (const QLineF &l : reader.geometry.lines)
		if (l.length() > 1e-6) out.lines << l.translated(-o);
	for (PanelOutline::Arc a : reader.geometry.arcs) {
		a.centre -= o;
		out.arcs << a;
	}
	out.width = x1 - x0;
	out.height = y1 - y0;
	if (reader.symbol.isValid() && reader.symbol.width() > 1 && reader.symbol.height() > 1)
		out.frame = reader.symbol.translated(-o);
	return out;
}

PanelOutline PanelImport::readDxfFile(const QString &path, QString *error)
{
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) {
		if (error) *error = QStringLiteral("Cannot read %1").arg(path);
		return PanelOutline();
	}
	PanelOutline o = readDxf(f.readAll(), error);
	o.name = templateName(path);
	return o;
}

QByteArray PanelImport::plateViewFromPackage(const QByteArray &zip, QString *error)
{
	mz_zip_archive archive;
	mz_zip_zero_struct(&archive);
	if (!mz_zip_reader_init_mem(&archive, zip.constData(), size_t(zip.size()), 0)) {
		if (error) *error = QStringLiteral("The package is no zip archive");
		return QByteArray();
	}
	QByteArray best;
	int best_holes = -1;
	double best_area = -1;
	QString best_name;
	const mz_uint count = mz_zip_reader_get_num_files(&archive);
	for (mz_uint i = 0; i < count && i < 2000; ++i) {
		mz_zip_archive_file_stat stat;
		if (!mz_zip_reader_file_stat(&archive, i, &stat) || stat.m_is_directory) continue;
		const QString name = QString::fromUtf8(stat.m_filename);
		if (!name.endsWith(QLatin1String(".dxf"), Qt::CaseInsensitive)
			|| !(name.contains(QLatin1String("panel layout"), Qt::CaseInsensitive)
				 || name.contains(QLatin1String("schaltschrankaufbau"), Qt::CaseInsensitive))) continue;
		if (stat.m_uncomp_size > 64 * 1024 * 1024) continue;
		size_t size = 0;
		void *data = mz_zip_reader_extract_to_heap(&archive, i, &size, 0);
		if (!data) continue;
		const QByteArray bytes(static_cast<const char *>(data), int(size));
		mz_free(data);
		const PanelOutline o = readDxf(bytes);
		if (o.isEmpty()) continue;
		const int holes = int(o.circles().size());
		const double area = o.width * o.height;
			//the front: most holes, then the largest view, then view A
		const bool better = holes > best_holes
				|| (holes == best_holes && area > best_area + 1)
				|| (holes == best_holes && qAbs(area - best_area) <= 1 && name < best_name);
		if (better) {best = bytes; best_holes = holes; best_area = area; best_name = name;}
	}
	mz_zip_reader_end(&archive);
	if (best.isEmpty() && error) *error = QStringLiteral("The package has no panel layout view");
	return best;
}

QMap<QString, QByteArray> PanelImport::viewsFromPackage(const QByteArray &zip)
{
	QMap<QString, QByteArray> out;
	mz_zip_archive archive;
	mz_zip_zero_struct(&archive);
	if (!mz_zip_reader_init_mem(&archive, zip.constData(), size_t(zip.size()), 0)) return out;
	static const QRegularExpression letter(QStringLiteral("_([A-P])\\d*\\.dxf$"), QRegularExpression::CaseInsensitiveOption);
	const mz_uint count = mz_zip_reader_get_num_files(&archive);
	for (mz_uint i = 0; i < count && i < 2000; ++i) {
		mz_zip_archive_file_stat stat;
		if (!mz_zip_reader_file_stat(&archive, i, &stat) || stat.m_is_directory) continue;
		const QString name = QString::fromUtf8(stat.m_filename);
		if (!name.contains(QLatin1String("panel layout"), Qt::CaseInsensitive)
			&& !name.contains(QLatin1String("schaltschrankaufbau"), Qt::CaseInsensitive)) continue;
		const QRegularExpressionMatch m = letter.match(name);
		if (!m.hasMatch() || stat.m_uncomp_size > 64 * 1024 * 1024) continue;
		size_t size = 0;
		void *data = mz_zip_reader_extract_to_heap(&archive, i, &size, 0);
		if (!data) continue;
		out.insert(m.captured(1).toUpper(), QByteArray(static_cast<const char *>(data), int(size)));
		mz_free(data);
	}
	mz_zip_reader_end(&archive);
	return out;
}

QByteArray PanelImport::deviceViewFromPackage(const QByteArray &zip)
{
		//the front: view A of the panel layout, else the first one there is
	const QMap<QString, QByteArray> views = viewsFromPackage(zip);
	if (views.contains(QStringLiteral("A"))) return views.value(QStringLiteral("A"));
	return views.isEmpty() ? QByteArray() : views.first();
}

PanelOutline PanelImport::frontView(const PanelOutline &drawing)
{
		// 1. EPLAN marks the device's outline on the layer EPLAN_Symbol
	auto crop = [&drawing](const QRectF &area) {
		const QRectF main = area.adjusted(-0.5, -0.5, 0.5, 0.5);
		auto inside = [&](const QRectF &b) {
			return b.left() >= main.left() && b.right() <= main.right()
				&& b.top() >= main.top() && b.bottom() <= main.bottom();
		};
		PanelOutline out;
		out.name = drawing.name;
		const QPointF shift = area.topLeft();
		for (const QLineF &l : drawing.lines)
			if (inside(QRectF(l.p1(), l.p2()).normalized())) out.lines << l.translated(-shift);
		for (PanelOutline::Arc a : drawing.arcs)
			if (inside(arcBounds(a))) {
				a.centre -= shift;
				out.arcs << a;
			}
		out.width = area.width();
		out.height = area.height();
		return out;
	};
	if (drawing.frame.isValid() && drawing.frame.width() > 1 && drawing.frame.height() > 1) {
		const PanelOutline out = crop(drawing.frame);
		if (!out.isEmpty()) return out;
	}

	struct Item {QRectF box; int line = -1; int arc = -1;};
	QVector<Item> items;
	for (int i = 0; i < drawing.lines.size(); ++i) {
		const QLineF &l = drawing.lines.at(i);
		items.append({QRectF(l.p1(), l.p2()).normalized(), i, -1});
	}
	for (int i = 0; i < drawing.arcs.size(); ++i) {
		const PanelOutline::Arc &a = drawing.arcs.at(i);
		items.append({arcBounds(a), -1, i});
	}
	const int n = int(items.size());
	if (n < 2) return drawing;

		// connected parts: items whose boxes touch (0.3 mm), by a sweep over x
	QVector<int> parent(n);
	for (int i = 0; i < n; ++i) parent[i] = i;
	std::function<int(int)> find = [&](int i) {return parent[i] == i ? i : parent[i] = find(parent[i]);};
	QVector<int> order(n);
	for (int i = 0; i < n; ++i) order[i] = i;
	std::sort(order.begin(), order.end(), [&](int a, int b) {return items[a].box.left() < items[b].box.left();});
	const double tol = 0.3;
	for (int oi = 0; oi < n; ++oi) {
		const QRectF a = items[order[oi]].box.adjusted(-tol, -tol, tol, tol);
		for (int oj = oi + 1; oj < n; ++oj) {
			const QRectF &b = items[order[oj]].box;
			if (b.left() > a.right()) break;
			if (b.top() <= a.bottom() && b.bottom() >= a.top()) {
				const int ra = find(order[oi]), rb = find(order[oj]);
				if (ra != rb) parent[ra] = rb;
			}
		}
	}
	QHash<int, QRectF> boxes;
	QHash<int, int> counts;
	for (int i = 0; i < n; ++i) {
		const int r = find(i);
		boxes[r] = boxes.contains(r) ? boxes[r].united(items[i].box) : items[i].box;
		counts[r] += 1;
	}
		// a drawing frame of a standard sheet is no device
	auto paper = [](const QRectF &r) {
		static const QList<QSizeF> sizes = {{210, 148}, {297, 210}, {420, 297}, {594, 420}, {841, 594}};
		for (const QSizeF &s : sizes)
			if ((qAbs(r.width() - s.width()) < 3 && qAbs(r.height() - s.height()) < 3)
				|| (qAbs(r.width() - s.height()) < 3 && qAbs(r.height() - s.width()) < 3)) return true;
		return false;
	};
		// 2. only a drawing with a sheet frame is cut down: to its largest
		//    part; 3. any other drawing is the device as it is
	bool has_frame = false;
	for (auto it = counts.constBegin(); it != counts.constEnd(); ++it)
		if (paper(boxes[it.key()])) has_frame = true;
	if (!has_frame) return drawing;
	int best = -1;
	auto area = [&](int k) {return boxes[k].width() * boxes[k].height();};
	for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
		if (paper(boxes[it.key()])) continue;
		if (best < 0 || area(it.key()) > area(best)) best = it.key();
	}
	if (best < 0) return drawing;
	const QRectF main = boxes[best].adjusted(-0.5, -0.5, 0.5, 0.5);

	PanelOutline out;
	out.name = drawing.name;
		// QRectF::contains() takes a straight line (no width or no height)
		// for nothing: compare the coordinates
	auto inside = [&](const QRectF &b) {
		return b.left() >= main.left() && b.right() <= main.right()
			&& b.top() >= main.top() && b.bottom() <= main.bottom();
	};
	for (const Item &it : std::as_const(items)) {
		if (!inside(it.box)) continue;
		if (it.line >= 0) out.lines << drawing.lines.at(it.line).translated(-main.topLeft() - QPointF(0.5, 0.5));
		else {
			PanelOutline::Arc a = drawing.arcs.at(it.arc);
			a.centre -= main.topLeft() + QPointF(0.5, 0.5);
			out.arcs << a;
		}
	}
	out.width = main.width() - 1;
	out.height = main.height() - 1;
	return out.isEmpty() ? drawing : out;
}

QString PanelImport::devicesDirectory()
{
	return templatesDirectory() + QStringLiteral("/devices");
}

PanelOutline PanelImport::collectionDeviceView(const QString &order_number, const QString &manufacturer)
{
	Q_UNUSED(manufacturer)
	auto norm = [](QString t) {
		static const QRegularExpression strip(QStringLiteral("[\\s.\\-]"));
		return t.remove(strip).toUpper();
	};
	const QString key = norm(order_number);
	if (key.isEmpty()) return PanelOutline();

		// one look through the collections per run: order number -> layout views
	static QHash<QString, QStringList> index;
	static bool indexed = false;
	if (!indexed) {
		indexed = true;
		static const QRegularExpression ref(QStringLiteral("name=\"manufacturer_reference\"[^>]*>([^<]+)<"));
		for (const QString &dir : {QETApp::companyElementsDir(), QETApp::customElementsDir()}) {
			QDirIterator it(dir, {QStringLiteral("*.elmt")}, QDir::Files, QDirIterator::Subdirectories);
			while (it.hasNext()) {
				const QString path = it.next();
				if (!QFileInfo(path).fileName().contains(QLatin1String("Aufbau"), Qt::CaseInsensitive)) continue;
				if (path.contains(QLatin1String("/Plaene/"))) continue;
				QFile f(path);
				if (!f.open(QIODevice::ReadOnly)) continue;
				const QString head = QString::fromUtf8(f.read(8192));
				const QRegularExpressionMatch m = ref.match(head);
				if (m.hasMatch()) index[norm(m.captured(1))] << path;
			}
		}
	}
	QStringList candidates = index.value(key);
	if (candidates.isEmpty()) return PanelOutline();
		// the manufacturer's own view first (view A), a page-scaled copy last
	static const QRegularExpression other_view(QStringLiteral("_[B-F]6"));
	std::stable_sort(candidates.begin(), candidates.end(), [](const QString &a, const QString &b) {
		auto rank = [](const QString &p) {
			const QString n = QFileInfo(p).fileName();
			return (n.contains(QLatin1String("Schrankplan")) ? 2 : 0) + (n.contains(other_view) ? 1 : 0);
		};
		return rank(a) < rank(b);
	});
	for (const QString &path : std::as_const(candidates)) {
		QFile f(path);
		if (!f.open(QIODevice::ReadOnly)) continue;
		QDomDocument doc;
		if (!doc.setContent(&f)) continue;
		const QDomElement def = doc.documentElement();
		double w_mm = 0, h_mm = 0;
		for (QDomElement i = def.firstChildElement(QStringLiteral("elementInformations")).firstChildElement(QStringLiteral("elementInformation"));
			 !i.isNull(); i = i.nextSiblingElement(QStringLiteral("elementInformation"))) {
			const QString v = i.text().trimmed().replace(QLatin1Char(','), QLatin1Char('.'));
			if (i.attribute(QStringLiteral("name")) == QLatin1String("width_mm")) w_mm = v.toDouble();
			if (i.attribute(QStringLiteral("name")) == QLatin1String("height_mm")) h_mm = v.toDouble();
		}
		const bool page_copy = QFileInfo(path).fileName().contains(QLatin1String("Schrankplan"));
		if (page_copy && (w_mm <= 0 || h_mm <= 0)) continue;   // scale unknown

			// the drawing in element units
		QList<QLineF> lines;
		QList<PanelOutline::Arc> arcs;
		QRectF bounds;
		auto grow = [&bounds](const QRectF &r) {bounds = bounds.isNull() ? r : bounds.united(r);};
		auto num = [](const QDomElement &e, const char *a) {return e.attribute(QLatin1String(a)).toDouble();};
		const QDomElement desc = def.firstChildElement(QStringLiteral("description"));
		for (QDomElement e = desc.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
			const QString t = e.tagName();
			if (t == QLatin1String("line")) {
				const QLineF l(num(e, "x1"), num(e, "y1"), num(e, "x2"), num(e, "y2"));
				lines << l; grow(QRectF(l.p1(), l.p2()).normalized().adjusted(-0.01, -0.01, 0.01, 0.01));
			} else if (t == QLatin1String("rect")) {
				const QRectF r(num(e, "x"), num(e, "y"), num(e, "width"), num(e, "height"));
				lines << QLineF(r.topLeft(), r.topRight()) << QLineF(r.topRight(), r.bottomRight())
					  << QLineF(r.bottomRight(), r.bottomLeft()) << QLineF(r.bottomLeft(), r.topLeft());
				grow(r);
			} else if (t == QLatin1String("polygon")) {
				QList<QPointF> pts;
				for (int i = 1; e.hasAttribute(QStringLiteral("x%1").arg(i)); ++i)
					pts << QPointF(e.attribute(QStringLiteral("x%1").arg(i)).toDouble(), e.attribute(QStringLiteral("y%1").arg(i)).toDouble());
				for (int i = 0; i + 1 < pts.size(); ++i) {lines << QLineF(pts[i], pts[i + 1]); grow(QRectF(pts[i], pts[i + 1]).normalized());}
				if (pts.size() > 2 && e.attribute(QStringLiteral("closed")) != QLatin1String("false")) lines << QLineF(pts.last(), pts.first());
			} else if (t == QLatin1String("ellipse") || t == QLatin1String("circle") || t == QLatin1String("arc")) {
				const double x = num(e, "x"), y = num(e, "y");
				const double d = t == QLatin1String("circle") ? num(e, "diameter") : num(e, "width");
				const double hgt = t == QLatin1String("circle") ? d : num(e, "height");
				const QRectF r(x, y, d, hgt);
				grow(r);
				PanelOutline::Arc a;
				a.centre = r.center();
				a.r = (d + hgt) / 4;
				if (t == QLatin1String("arc")) {
					a.a0 = num(e, "start");
					a.a1 = a.a0 + num(e, "angle");
				} else {a.a0 = 0; a.a1 = 360;}
				arcs << a;
			}
		}
		if (bounds.isEmpty() || (lines.isEmpty() && arcs.isEmpty())) continue;
		const double sx = page_copy ? bounds.width() / w_mm : 2.5;
		const double sy = page_copy ? bounds.height() / h_mm : 2.5;
		if (sx <= 0 || sy <= 0) continue;
		PanelOutline o;
		o.name = order_number;
		o.width = bounds.width() / sx;
		o.height = bounds.height() / sy;
		auto mm = [&](const QPointF &p) {return QPointF((p.x() - bounds.left()) / sx, (bounds.bottom() - p.y()) / sy);};
		for (const QLineF &l : std::as_const(lines)) o.lines << QLineF(mm(l.p1()), mm(l.p2()));
		for (PanelOutline::Arc a : std::as_const(arcs)) {
			a.centre = mm(a.centre);
			a.r /= (sx + sy) / 2;
				// y turns over: an arc of the element runs the other way
			if (!a.isCircle()) {const double a0 = a.a0; a.a0 = -a.a1; a.a1 = -a0;}
			o.arcs << a;
		}
		return o;
	}
	return PanelOutline();
}

PanelOutline PanelImport::fetchDeviceView(const QString &order_number, const QString &manufacturer,
										  QString *error)
{
	const QString order = order_number.trimmed();
	if (order.isEmpty()) return PanelOutline();
		// the parts already in the collections come first: no download
	const PanelOutline own = collectionDeviceView(order, manufacturer);
	if (!own.isEmpty()) return frontView(own);
	QString key = (manufacturer.trimmed() + QLatin1Char('_') + order);
	key.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_.+-]")), QStringLiteral("_"));
	const QString cached = QDir(devicesDirectory()).filePath(key + QStringLiteral(".dxf"));
	const QString none = QDir(devicesDirectory()).filePath(key + QStringLiteral(".none"));
	if (QFileInfo::exists(cached)) return frontView(readDxfFile(cached, error));
	if (QFileInfo::exists(none)) {
		if (error) *error = QStringLiteral("no panel layout drawing in the Data Portal");
		return PanelOutline();
	}
	const QString token = ManufacturerPortals::dataPortalToken();
	if (token.isEmpty()) {
		if (error) *error = QStringLiteral("no Data Portal token");
		return PanelOutline();
	}

		//one request after the other, waiting for each
	DataPortalClient client;
	client.setToken(token);
	QEventLoop loop;
	QTimer guard;
	guard.setSingleShot(true);
	QObject::connect(&guard, &QTimer::timeout, &loop, &QEventLoop::quit);
	QVector<DataPortalPart> found;
	QByteArray package;
	QString failure;
	QObject::connect(&client, &DataPortalClient::searchFinished, &loop,
					 [&](const QVector<DataPortalPart> &parts, int, int) {found = parts; loop.quit();});
	QObject::connect(&client, &DataPortalClient::downloadFinished, &loop,
					 [&](const QByteArray &data, const QString &) {package = data; loop.quit();});
	QObject::connect(&client, &DataPortalClient::failed, &loop,
					 [&](const QString &message) {failure = message; loop.quit();});
	DataPortalQuery q;
	q.text = order;
	q.only_dxf = true;
	q.exact = true;
	q.limit = 20;
	client.search(q);
	guard.start(30000);
	loop.exec();
	DataPortalPart part;
	for (const DataPortalPart &p : std::as_const(found)) {
		if (p.order_number.compare(order, Qt::CaseInsensitive) != 0) continue;
		const bool same_maker = manufacturer.isEmpty()
				|| p.manufacturer.contains(manufacturer, Qt::CaseInsensitive)
				|| manufacturer.contains(p.manufacturer_short, Qt::CaseInsensitive);
		if (part.id.isEmpty() || same_maker) part = p;
		if (same_maker) break;
	}
	if (part.id.isEmpty()) {
		if (error) *error = failure.isEmpty() ? QStringLiteral("not in the Data Portal") : failure;
		if (failure.isEmpty()) {QDir().mkpath(devicesDirectory()); QFile f(none); f.open(QIODevice::WriteOnly);}
		return PanelOutline();
	}
	client.downloadDxf(part.id);
	guard.start(60000);
	loop.exec();
	if (package.isEmpty()) {
		if (error) *error = failure.isEmpty() ? QStringLiteral("download failed") : failure;
		return PanelOutline();
	}
	const QByteArray view = deviceViewFromPackage(package);
	QDir().mkpath(devicesDirectory());
	if (view.isEmpty()) {
		QFile f(none);
		f.open(QIODevice::WriteOnly);
		if (error) *error = QStringLiteral("no panel layout drawing in the Data Portal");
		return PanelOutline();
	}
	QSaveFile f(cached);
	if (f.open(QIODevice::WriteOnly)) {f.write(view); f.commit();}
	PanelOutline o = readDxf(view, error);
	o.name = part.part_number;
	return frontView(o);
}

QString PanelImport::enclosuresDirectory()
{
	return templatesDirectory() + QStringLiteral("/enclosures");
}

QStringList PanelImport::enclosureFolders()
{
	QDir dir(enclosuresDirectory());
	QStringList out;
	for (const QString &n : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
		if (QFileInfo::exists(dir.filePath(n + QStringLiteral("/A.dxf")))) out << dir.absoluteFilePath(n);
	return out;
}

PanelImport::Enclosure PanelImport::readEnclosure(const QString &folder)
{
	Enclosure e;
	e.folder = folder;
	e.name = QFileInfo(folder).fileName();
	QFile info(QDir(folder).filePath(QStringLiteral("info.txt")));
	if (info.open(QIODevice::ReadOnly)) e.description = QString::fromUtf8(info.readAll()).trimmed();
		//"BHT: 800x1400x500 mm", "Breite/Höhe/Tiefe: 800 x 1200 x 500 mm"
	static const QRegularExpression bht(QStringLiteral("(\\d{2,4})\\s*x\\s*(\\d{2,4})\\s*x\\s*(\\d{2,4})"));
	const QRegularExpressionMatch m = bht.match(e.description);
	e.front = readDxfFile(QDir(folder).filePath(QStringLiteral("A.dxf")));
	e.front.name = e.name;
	if (QFileInfo::exists(QDir(folder).filePath(QStringLiteral("B.dxf")))) {
		e.plate = readDxfFile(QDir(folder).filePath(QStringLiteral("B.dxf")));
		e.plate.name = e.name;
	}
	if (m.hasMatch()) {
		e.width = m.captured(1).toDouble();
		e.height = m.captured(2).toDouble();
		e.depth = m.captured(3).toDouble();
	} else {
		e.width = e.front.width;
		e.height = e.front.height;
		e.depth = 400;
	}
		//a view B narrower than the front is the mounting plate (Rittal:
		//enclosure width - 101, height - 104); otherwise there is none
	if (!e.plate.isEmpty() && (e.plate.width > e.width - 20 || e.plate.height > e.height - 20))
		e.plate = PanelOutline();
	return e;
}

QString PanelImport::saveEnclosure(const QString &name, const QString &description,
								   const QMap<QString, QByteArray> &views, QString *error)
{
	if (!views.contains(QStringLiteral("A"))) {
		if (error) *error = QStringLiteral("The package has no front view (A)");
		return QString();
	}
	const QString folder = QDir(enclosuresDirectory()).filePath(safeName(name));
	QDir().mkpath(folder);
	for (auto it = views.constBegin(); it != views.constEnd(); ++it) {
		QSaveFile f(QDir(folder).filePath(it.key() + QStringLiteral(".dxf")));
		if (!f.open(QIODevice::WriteOnly) || f.write(it.value()) != it.value().size() || !f.commit()) {
			if (error) *error = QStringLiteral("Cannot write into %1").arg(folder);
			return QString();
		}
	}
	QSaveFile info(QDir(folder).filePath(QStringLiteral("info.txt")));
	if (info.open(QIODevice::WriteOnly)) {info.write(description.toUtf8()); info.commit();}
	return folder;
}

QString PanelImport::templatesDirectory()
{
	const QString env = qEnvironmentVariable("QET_PANEL_TEMPLATES");
	if (!env.isEmpty()) return env;
	return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QStringLiteral("/panel-templates");
}

QStringList PanelImport::templateFiles()
{
	QDir dir(templatesDirectory());
	QStringList out;
	for (const QString &n : dir.entryList({QStringLiteral("*.dxf"), QStringLiteral("*.DXF")}, QDir::Files, QDir::Name))
		out << dir.absoluteFilePath(n);
	return out;
}

QString PanelImport::templateName(const QString &file)
{
	return QFileInfo(file).completeBaseName();
}

QString PanelImport::saveTemplate(const QString &name, const QByteArray &dxf, QString *error)
{
	QDir().mkpath(templatesDirectory());
	const QString path = QDir(templatesDirectory()).filePath(safeName(name) + QStringLiteral(".dxf"));
	QSaveFile f(path);
	if (!f.open(QIODevice::WriteOnly) || f.write(dxf) != dxf.size() || !f.commit()) {
		if (error) *error = QStringLiteral("Cannot write %1").arg(path);
		return QString();
	}
	return path;
}

/* ------------------------------------------------------------------ */

PanelTemplateDialog::PanelTemplateDialog(QWidget *parent, bool enclosure) :
	QDialog(parent),
	m_enclosure(enclosure)
{
	setWindowTitle(enclosure ? tr("Enclosure from the EPLAN Data Portal")
							 : tr("Mounting plate from the EPLAN Data Portal"));
	resize(820, 520);
	m_client = new DataPortalClient(this);
	m_client->setLanguage(QLocale(QETApp::langFromSetting()).name());

		// the Data Portal finds plates best by the German designation
	m_search = new QLineEdit(enclosure ? QStringLiteral("Schrank") : QStringLiteral("Montageplatte"), this);
	m_manufacturer = new QLineEdit(QStringLiteral("RIT"), this);
	m_manufacturer->setMaximumWidth(80);
	m_manufacturer->setToolTip(tr("Short name of the manufacturer (RIT = Rittal), empty: all"));
	auto *go = new QPushButton(tr("Search"), this);
	auto *row = new QHBoxLayout;
	row->addWidget(m_search, 1);
	row->addWidget(new QLabel(tr("Manufacturer"), this));
	row->addWidget(m_manufacturer);
	row->addWidget(go);

	m_list = new QTreeWidget(this);
	m_list->setRootIsDecorated(false);
	m_list->setHeaderLabels({tr("Order number"), tr("Type"), tr("Description")});
	m_list->header()->setStretchLastSection(true);
	m_status = new QLabel(this);
	m_status->setWordWrap(true);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
	m_take = buttons->addButton(tr("Use as template"), QDialogButtonBox::ActionRole);
	m_take->setEnabled(false);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(m_take, &QPushButton::clicked, this, &PanelTemplateDialog::download);
	connect(m_list, &QTreeWidget::itemSelectionChanged, this, [this]() {
		m_take->setEnabled(!m_list->selectedItems().isEmpty() && !m_client->isBusy());
	});
	connect(m_list, &QTreeWidget::itemDoubleClicked, this, &PanelTemplateDialog::download);
	connect(go, &QPushButton::clicked, this, &PanelTemplateDialog::runSearch);
	connect(m_search, &QLineEdit::returnPressed, this, &PanelTemplateDialog::runSearch);
	connect(m_client, &DataPortalClient::searchFinished, this, &PanelTemplateDialog::showParts);
	connect(m_client, &DataPortalClient::downloadFinished, this, &PanelTemplateDialog::downloaded);
	connect(m_client, &DataPortalClient::failed, this, &PanelTemplateDialog::failed);

	auto *box = new QVBoxLayout(this);
	box->addLayout(row);
	box->addWidget(m_list, 1);
	box->addWidget(m_status);
	box->addWidget(buttons);
	runSearch();
}

void PanelTemplateDialog::runSearch()
{
	DataPortalQuery q;
	q.text = m_search->text().trimmed();
	q.manufacturer = m_manufacturer->text().trimmed();
	q.only_dxf = true;
	q.exact = true;
	q.limit = 100;
	m_list->clear();
	m_status->setText(tr("Searching…"));
	m_client->search(q);
}

void PanelTemplateDialog::showParts(const QVector<DataPortalPart> &parts, int total, int offset)
{
	Q_UNUSED(offset)
		// plates first: what is called a mounting plate, then the rest;
		// configuration parts without an order number are left out
	QVector<DataPortalPart> sorted;
	for (const DataPortalPart &p : parts) if (!p.order_number.isEmpty()) sorted << p;
	std::stable_sort(sorted.begin(), sorted.end(), [this](const DataPortalPart &a, const DataPortalPart &b) {
		auto plate = [this](const DataPortalPart &p) {
			if (m_enclosure)
				return p.designation.contains(QLatin1String("schrank"), Qt::CaseInsensitive)
					|| p.designation.contains(QLatin1String("gehäuse"), Qt::CaseInsensitive)
					|| p.designation.contains(QLatin1String("enclosure"), Qt::CaseInsensitive);
			return p.designation.contains(QLatin1String("Montageplatte"), Qt::CaseInsensitive)
				|| p.designation.contains(QLatin1String("mounting plate"), Qt::CaseInsensitive);
		};
		return plate(a) && !plate(b);
	});
	for (const DataPortalPart &p : sorted) {
		auto *item = new QTreeWidgetItem(m_list, {p.order_number, p.type_number,
												  p.description.isEmpty() ? p.designation : p.description});
		item->setData(0, Qt::UserRole, p.id);
		item->setToolTip(2, p.description);
	}
	m_list->resizeColumnToContents(0);
	m_list->resizeColumnToContents(1);
	m_status->setText(tr("%1 of %2 parts. Only parts with DXF data are listed.").arg(sorted.size()).arg(total));
}

void PanelTemplateDialog::download()
{
	const QList<QTreeWidgetItem *> sel = m_list->selectedItems();
	if (sel.isEmpty()) return;
	const QString token = ManufacturerPortals::dataPortalToken();
	if (token.isEmpty()) {
		m_status->setText(tr("The download needs your EPLAN Data Portal token: enter it once in "
							 "Part import > From the EPLAN Data Portal (\"Remember the token\")."));
		return;
	}
	QTreeWidgetItem *item = sel.first();
	m_pending_name = item->text(1) + QStringLiteral(" - ") + item->text(2);
	m_pending_description = item->text(2);
	m_client->setToken(token);
	m_take->setEnabled(false);
	m_status->setText(tr("Downloading %1…").arg(item->text(0)));
	m_client->downloadDxf(item->data(0, Qt::UserRole).toString());
}

void PanelTemplateDialog::downloaded(const QByteArray &data, const QString &file_name)
{
	Q_UNUSED(file_name)
	QString error;
	if (m_enclosure) {
		const QString folder = PanelImport::saveEnclosure(m_pending_name, m_pending_description,
														  PanelImport::viewsFromPackage(data), &error);
		if (folder.isEmpty()) {failed(error); return;}
		m_file = folder;
		accept();
		return;
	}
	const QByteArray view = PanelImport::plateViewFromPackage(data, &error);
	if (view.isEmpty()) {failed(error); return;}
	const QString path = PanelImport::saveTemplate(m_pending_name, view, &error);
	if (path.isEmpty()) {failed(error); return;}
	m_file = path;
	accept();
}

void PanelTemplateDialog::failed(const QString &message)
{
	m_status->setText(message);
	m_take->setEnabled(!m_list->selectedItems().isEmpty());
}
