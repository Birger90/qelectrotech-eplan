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
#include "paneldxf.h"

#include "panellayout.h"

#include <QMap>
#include <QPointF>
#include <QSaveFile>
#include <QtMath>

namespace {
QString f(double v) {return QString::number(v, 'f', 4);}

struct Writer
{
	QString out;
	void pair(int code, const QString &value) {out += QStringLiteral("%1\n%2\n").arg(code).arg(value);}
	void pair(int code, double value) {pair(code, f(value));}
	void pair(int code, int value) {pair(code, QString::number(value));}

	void circle(const QString &layer, QPointF c, double r)
	{
		pair(0, QStringLiteral("CIRCLE")); pair(8, layer);
		pair(10, c.x()); pair(20, c.y()); pair(30, 0.0); pair(40, r);
	}
		/// closed polyline; bulges[i] belongs to the segment from point i
	void polyline(const QString &layer, const QList<QPointF> &points, const QList<double> &bulges = {})
	{
		pair(0, QStringLiteral("POLYLINE")); pair(8, layer); pair(66, 1); pair(10, 0.0); pair(20, 0.0); pair(30, 0.0);
		pair(70, 1);
		for (int i = 0; i < points.size(); ++i) {
			pair(0, QStringLiteral("VERTEX")); pair(8, layer);
			pair(10, points.at(i).x()); pair(20, points.at(i).y()); pair(30, 0.0);
			if (i < bulges.size() && bulges.at(i) != 0) pair(42, bulges.at(i));
		}
		pair(0, QStringLiteral("SEQEND")); pair(8, layer);
	}
	void text(const QString &layer, QPointF p, double height, const QString &s)
	{
		pair(0, QStringLiteral("TEXT")); pair(8, layer);
		pair(10, p.x()); pair(20, p.y()); pair(30, 0.0); pair(40, height); pair(1, s);
	}
};

QPointF rot(QPointF p, double deg)
{
	const double a = qDegreesToRadians(deg), c = qCos(a), s = qSin(a);
	return QPointF(p.x() * c - p.y() * s, p.x() * s + p.y() * c);
}

	/// DXF R12 names and texts: ASCII only
QString ascii(QString s)
{
	s.replace(QStringLiteral("ä"), QStringLiteral("ae")).replace(QStringLiteral("ö"), QStringLiteral("oe"))
	 .replace(QStringLiteral("ü"), QStringLiteral("ue")).replace(QStringLiteral("Ä"), QStringLiteral("Ae"))
	 .replace(QStringLiteral("Ö"), QStringLiteral("Oe")).replace(QStringLiteral("Ü"), QStringLiteral("Ue"))
	 .replace(QStringLiteral("ß"), QStringLiteral("ss")).replace(QLatin1Char(','), QLatin1Char('.'));
	QString out;
	for (const QChar c : s) out += c.unicode() < 128 ? c : QLatin1Char('_');
	return out;
}
}

/**
	@brief PanelDxf::coreDiameter
	Core drill of a metric ISO coarse thread (DIN 336), for workshops that
	want the hole to drill rather than the thread to cut.
*/
double PanelDxf::coreDiameter(double thread)
{
	static const QMap<double, double> core = {
		{2, 1.6}, {2.5, 2.05}, {3, 2.5}, {4, 3.3}, {5, 4.2}, {6, 5.0}, {8, 6.8},
		{10, 8.5}, {12, 10.2}, {16, 14.0}, {20, 17.5}, {24, 21.0}};
	auto it = core.constFind(thread);
	return it != core.constEnd() ? it.value() : thread * 0.85;
}

/**
	@brief PanelDxf::write
	@return true when the file was written; @p error says why not
*/
bool PanelDxf::write(const PanelLayout &layout, const QString &path,
					 const Options &options, QString *error)
{
	const QList<PanelHole> holes = layout.holes();
	const double W = layout.widthMm(), H = layout.heightMm();

	auto layerOf = [&](const PanelHole &h) {
		switch (h.type) {
			case PanelHole::Drill:  return ascii(QStringLiteral("BOHRUNG_") + h.kindName());
			case PanelHole::Thread: return ascii(QStringLiteral("GEWINDE_") + h.kindName());
			default:                return QStringLiteral("AUSSCHNITT");
		}
	};
	QStringList layers{QStringLiteral("FELD"), QStringLiteral("AUSSCHNITT")};
	for (const PanelHole &h : holes) if (!layers.contains(layerOf(h))) layers << layerOf(h);
	if (options.labels) layers << QStringLiteral("BESCHRIFTUNG");

	Writer w;
		//HEADER: R12, millimetres (MEASUREMENT 1 = metric)
	w.pair(0, QStringLiteral("SECTION")); w.pair(2, QStringLiteral("HEADER"));
	w.pair(9, QStringLiteral("$ACADVER")); w.pair(1, QStringLiteral("AC1009"));
	w.pair(9, QStringLiteral("$INSBASE")); w.pair(10, 0.0); w.pair(20, 0.0); w.pair(30, 0.0);
	w.pair(9, QStringLiteral("$EXTMIN")); w.pair(10, 0.0); w.pair(20, 0.0); w.pair(30, 0.0);
	w.pair(9, QStringLiteral("$EXTMAX")); w.pair(10, W); w.pair(20, H); w.pair(30, 0.0);
	w.pair(9, QStringLiteral("$MEASUREMENT")); w.pair(70, 1);
	w.pair(0, QStringLiteral("ENDSEC"));

		//TABLES: line type and layers (colours: field white, drills red,
		//threads blue, cut-outs green, labels grey)
	w.pair(0, QStringLiteral("SECTION")); w.pair(2, QStringLiteral("TABLES"));
	w.pair(0, QStringLiteral("TABLE")); w.pair(2, QStringLiteral("LTYPE")); w.pair(70, 1);
	w.pair(0, QStringLiteral("LTYPE")); w.pair(2, QStringLiteral("CONTINUOUS")); w.pair(70, 0);
	w.pair(3, QStringLiteral("Solid line")); w.pair(72, 65); w.pair(73, 0); w.pair(40, 0.0);
	w.pair(0, QStringLiteral("ENDTAB"));
	w.pair(0, QStringLiteral("TABLE")); w.pair(2, QStringLiteral("LAYER")); w.pair(70, int(layers.size()));
	for (const QString &l : layers) {
		int colour = 7;
		if (l.startsWith(QLatin1String("BOHRUNG"))) colour = 1;
		else if (l.startsWith(QLatin1String("GEWINDE"))) colour = 5;
		else if (l == QLatin1String("AUSSCHNITT")) colour = 3;
		else if (l == QLatin1String("BESCHRIFTUNG")) colour = 8;
		w.pair(0, QStringLiteral("LAYER")); w.pair(2, l); w.pair(70, 0); w.pair(62, colour);
		w.pair(6, QStringLiteral("CONTINUOUS"));
	}
	w.pair(0, QStringLiteral("ENDTAB"));
	w.pair(0, QStringLiteral("ENDSEC"));

		//ENTITIES
	w.pair(0, QStringLiteral("SECTION")); w.pair(2, QStringLiteral("ENTITIES"));
	w.polyline(QStringLiteral("FELD"), {QPointF(0, 0), QPointF(W, 0), QPointF(W, H), QPointF(0, H)});

	for (const PanelHole &h : holes)
	{
		const QPointF c(h.x, h.y);
		const QString layer = layerOf(h);
		switch (h.type) {
			case PanelHole::Thread:
				w.circle(layer, c, (options.thread_core ? coreDiameter(h.d1) : h.d1) / 2);
				break;
			case PanelHole::Rectangle: {
				const double a = h.d1 / 2, b = h.d2 / 2, r = qMin(h.d3, qMin(a, b));
				QList<QPointF> pts;
				QList<double> bulges;
				const QList<QPointF> corners8 = {QPointF(-a + r, -b), QPointF(a - r, -b), QPointF(a, -b + r), QPointF(a, b - r),
												 QPointF(a - r, b), QPointF(-a + r, b), QPointF(-a, b - r), QPointF(-a, -b + r)};
				if (r > 0 && h.subtype == PanelHole::Rounded) {
						//corners as quarter arcs: bulge tan(90°/4)
					const double k = qTan(M_PI / 8);
					for (int i = 0; i < corners8.size(); ++i) {pts << c + rot(corners8.at(i), h.angle); bulges << (i % 2 ? k : 0.0);}
				} else if (r > 0 && h.subtype == PanelHole::Chamfered) {
					for (const QPointF &p : corners8) pts << c + rot(p, h.angle);
				} else {
					for (const QPointF &p : {QPointF(-a, -b), QPointF(a, -b), QPointF(a, b), QPointF(-a, b)})
						pts << c + rot(p, h.angle);
				}
				w.polyline(layer, pts, bulges);
				break;
			}
			case PanelHole::Slot: {
					//slot: overall length and width from d1/d2, round ends (bulge 1 = half circle)
				const double r = qMin(h.d1, h.d2) / 2, half = qMax(h.d1, h.d2) / 2 - r;
				const bool along_x = h.d1 >= h.d2;
				const QList<QPointF> raw = along_x
						? QList<QPointF>{QPointF(-half, -r), QPointF(half, -r), QPointF(half, r), QPointF(-half, r)}
						: QList<QPointF>{QPointF(r, -half), QPointF(r, half), QPointF(-r, half), QPointF(-r, -half)};
				QList<QPointF> pts;
				for (const QPointF &p : raw) pts << c + rot(p, h.angle);
				w.polyline(layer, pts, {0.0, 1.0, 0.0, 1.0});
				break;
			}
			case PanelHole::Hexagon:
			case PanelHole::Octagon: {
					//d1 = width across flats
				const int n = h.type == PanelHole::Hexagon ? 6 : 8;
				const double R = h.d1 / 2 / qCos(M_PI / n);
				QList<QPointF> pts;
				for (int i = 0; i < n; ++i) {
					const double a = 2 * M_PI * i / n + M_PI / n;
					pts << c + rot(QPointF(R * qCos(a), R * qSin(a)), h.angle);
				}
				w.polyline(layer, pts);
				break;
			}
			default:
				w.circle(layer, c, h.d1 / 2);
				break;
		}
		if (options.labels && !h.source.isEmpty())
			w.text(QStringLiteral("BESCHRIFTUNG"), c + QPointF(h.d1 / 2 + 1, h.d1 / 2 + 1), 2.5,
				   ascii(QStringLiteral("%1 %2").arg(h.source, h.kindName())));
	}
	w.pair(0, QStringLiteral("ENDSEC"));
	w.pair(0, QStringLiteral("EOF"));

	QSaveFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		if (error) *error = file.errorString();
		return false;
	}
	file.write(w.out.toLatin1());
	if (!file.commit()) {
		if (error) *error = file.errorString();
		return false;
	}
	return true;
}
