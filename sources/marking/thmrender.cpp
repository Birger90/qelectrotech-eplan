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
#include "thmrender.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QFontMetricsF>
#include <QPainter>
#include <QRectF>

namespace {

struct Geometry
{
	double pitch = 0;
	double field_left = 0, field_right = 0, field_top = 0, field_bottom = 0;
	int rotation = 0;
};

Geometry geometry(const ThmJob::Profile &p)
{
	Geometry g;
	g.pitch = p.columns > 1 && p.column_pitch > 0 ? p.column_pitch : p.width;
	g.field_left = p.margin_left;
	g.field_right = p.width - p.margin_right;
	g.field_top = p.field_top;
	g.field_bottom = p.field_bottom > 0 ? p.field_bottom : p.height;
	g.rotation = ((p.text_rotation % 360) + 360) % 360;
	return g;
}

	/// Length along the text and across it, millimetres
QSizeF room(const Geometry &g)
{
	const double w = g.field_right - g.field_left, h = g.field_bottom - g.field_top;
	return (g.rotation == 90 || g.rotation == 270) ? QSizeF(h, w) : QSizeF(w, h);
}

	/// The font for @p text in a field of @p room (mm) at @p units per mm:
	/// the profile's size, shrunk until it fits both ways
QFont fitted(const ThmJob::Profile &p, const QString &text, QSizeF room, double units)
{
	QFont f = ThmRender::font(p, units);
	const double along = ThmRender::fit(f, text, room.width() * units);
	const double cap = QFontMetricsF(f).capHeight();
	const double across = cap > 0 ? qMin(1.0, room.height() * units / cap) : 1.0;
	const double s = qMin(along, across);
	if (s < 1) f.setPixelSize(qMax(1, int(f.pixelSize() * s)));
	return f;
}

QString line(const QString &s) { return s + QStringLiteral("\r\n"); }

} // namespace

QFont ThmRender::font(const ThmJob::Profile &profile, double units_per_mm)
{
	QFont f(QStringLiteral("Arial"));
		//The size is the height of a capital; Arial's em is 1/0.716 of it
	f.setPixelSize(qMax(1, qRound(profile.text_size / 0.716 * units_per_mm)));
	f.setHintingPreference(QFont::PreferNoHinting);
	return f;
}

double ThmRender::fit(const QFont &font, const QString &text, double room)
{
	if (text.isEmpty() || room <= 0) return 1;
	const double w = QFontMetricsF(font).horizontalAdvance(text);
	return w > room ? room / w : 1;
}

QFont ThmRender::fontFor(const ThmJob::Profile &profile, const QString &text, double units_per_mm)
{
	return fitted(profile, text, room(geometry(profile)), units_per_mm);
}

bool ThmRender::overflows(const ThmJob::Profile &profile, const QString &text)
{
	const Geometry g = geometry(profile);
	const QFont f = font(profile, 10);
	return fit(f, text, room(g).width() * 10) < 1;
}

QImage ThmRender::renderRow(const QStringList &texts, const ThmJob::Profile &p)
{
	const Geometry g = geometry(p);
	const double row_width = ThmJob::rowWidth(p);
	const int w = qMax(1, qRound(row_width * kDotsPerMm));
	const int h = qMax(1, qRound(p.height * kDotsPerMm));
	QImage image(w, h, QImage::Format_RGB32);
	image.fill(Qt::white);
	{
		QPainter painter(&image);
		painter.setRenderHint(QPainter::TextAntialiasing, false);
		painter.setRenderHint(QPainter::Antialiasing, false);
		painter.setPen(Qt::black);
		for (int c = 0; c < texts.size() && c < qMax(1, p.columns); ++c) {
			const QString text = texts.at(c);
			if (text.isEmpty()) continue;
			const double left = p.first_x + c * g.pitch;
			const QPointF centre(((left + g.field_left + left + g.field_right) / 2) * kDotsPerMm,
								 ((g.field_top + g.field_bottom) / 2) * kDotsPerMm);
			const QFont f = fitted(p, text, room(g), kDotsPerMm);
			painter.setFont(f);
			const QFontMetricsF fm(f);
				//The capitals centred in the field, turned as the layout
				//says (anticlockwise, as JScript turns)
			painter.save();
			painter.translate(centre);
			painter.rotate(-g.rotation);
			painter.drawText(QPointF(-fm.horizontalAdvance(text) / 2, fm.capHeight() / 2), text);
			painter.restore();
		}
	}
	return image.convertToFormat(QImage::Format_Mono, Qt::ThresholdDither | Qt::AvoidDither);
}

QByteArray ThmRender::build(const QList<ThmJob::Label> &labels, const ThmJob::Profile &p, QString *error)
{
	if (!ThmJob::validate(p, error)) return QByteArray();
	if (ThmJob::labelCount(labels) <= 0) {
		if (error) *error = QCoreApplication::translate("ThmJob", "There is nothing to print.");
		return QByteArray();
	}
	if (error) error->clear();
	const int columns = qMax(1, p.columns);
		//The rows of the material, equal rows in a row counted once
	QStringList cells;
	for (const ThmJob::Label &l : labels)
		for (int n = 0; n < l.count; ++n) cells << l.text;
	QList<QPair<QStringList, int>> rows;
	for (int i = 0; i < cells.size(); i += columns) {
		QStringList texts = cells.mid(i, columns);
		while (texts.size() < columns) texts << QString();
		if (!rows.isEmpty() && rows.last().first == texts) ++rows.last().second;
		else rows.append({texts, 1});
	}

		//The label lines as M-Print PRO writes them: H, O, S, then the picture
	QString heat = QStringLiteral("H%1,%2%3,%4").arg(p.speed).arg(p.heat >= 0 ? QStringLiteral("+") : QString())
				   .arg(p.heat).arg(p.transfer ? QStringLiteral("T") : QStringLiteral("D"));
	if (!p.heat_options.isEmpty()) heat += QLatin1Char(',') + p.heat_options;
	QStringList options = p.print_options;
	if (p.rotate && !options.contains(QStringLiteral("R"))) options.prepend(QStringLiteral("R"));
	const QString size = QStringLiteral("S%1;%2,%3,%4,%5,%6")
			.arg(ThmJob::sensorKey(p.sensor), ThmJob::number(p.offset_x), ThmJob::number(p.offset_y),
				 ThmJob::number(p.height), ThmJob::number(p.height + p.gap), ThmJob::number(ThmJob::rowWidth(p)));

	QByteArray out;
	for (const auto &row : std::as_const(rows)) {
		QByteArray png;
		QBuffer buffer(&png);
		buffer.open(QIODevice::WriteOnly);
		renderRow(row.first, p).save(&buffer, "PNG");
		out += line(QStringLiteral("eIMG;*")).toLatin1();
		out += line(QStringLiteral("dPNG;0")).toLatin1();
		out += "\x1b:" + png + "\x1b" "end-of-data\r\n";
		QStringList lines {QStringLiteral("mm")};
		if (p.plain_zero) lines << QStringLiteral("zO");
		lines << QStringLiteral("J");
		if (p.set_heat) lines << heat;
		if (!options.isEmpty()) lines << QStringLiteral("O ") + options.join(QLatin1Char(','));
		lines << size;
		lines << QStringLiteral("I:Field1;0,0,0;0");
		lines << QStringLiteral("A%1").arg(row.second);
		for (const QString &l : std::as_const(lines)) out += line(l).toLatin1();
	}
	return out;
}
