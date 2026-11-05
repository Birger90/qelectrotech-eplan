/*
	Copyright 2006-2026 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech. If not, see <http://www.gnu.org/licenses/>.
*/
#include "eplanmacroreader.h"

#include <QDomDocument>
#include <QFile>
#include <QMap>
#include <QRegularExpression>

namespace {
	/// "x/y" or "x/y/z" -> point (EPLAN mm, y up)
	bool point(const QString &s, QPointF *p)
	{
		const QStringList v = s.split(QLatin1Char('/'));
		if (v.size() < 2) return false;
		bool ok_x = false, ok_y = false;
		const double x = v.at(0).toDouble(&ok_x), y = v.at(1).toDouble(&ok_y);
		if (!ok_x || !ok_y) return false;
		*p = QPointF(x, y);
		return true;
	}

	/// EPLAN multilingual text "de_DE@Text;en_US@Text;" -> German, else
	/// English, else the first language; plain text is kept as it is
	QString language(const QString &s)
	{
		static const QRegularExpression part(QStringLiteral("([a-z?]{2}_[A-Z?]{2})@([^;]*)(;|$)"));
		QMap<QString, QString> by_lang;
		QString first;
		auto it = part.globalMatch(s);
		while (it.hasNext()) {
			const auto m = it.next();
			by_lang.insert(m.captured(1), m.captured(2));
			if (first.isEmpty()) first = m.captured(2);
		}
		if (by_lang.isEmpty()) return s;
		if (by_lang.contains(QStringLiteral("de_DE"))) return by_lang.value(QStringLiteral("de_DE"));
		if (by_lang.contains(QStringLiteral("en_US"))) return by_lang.value(QStringLiteral("en_US"));
		return first;
	}

	void addRect(QVector<QLineF> &lines, const QPointF &a, const QPointF &b)
	{
		lines << QLineF(a.x(), a.y(), b.x(), a.y()) << QLineF(b.x(), a.y(), b.x(), b.y())
		      << QLineF(b.x(), b.y(), a.x(), b.y()) << QLineF(a.x(), b.y(), a.x(), a.y());
	}

	EplanDxfText text(const QString &s, const QPointF &at, int halign)
	{
		EplanDxfText t;
		t.text = s;
		t.anchor = at;
		t.insert = at;
		t.height = 2.5;
		t.halign = halign;
		return t;
	}
}

bool EplanMacroReader::read(const QString &path)
{
	m_symbol = EplanSymbolData();
	m_letters.clear();
	m_description.clear();
	m_skipped.clear();
	m_error.clear();

	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) {
		m_error = QStringLiteral("Cannot read %1").arg(path);
		return false;
	}
	QDomDocument doc;
	if (!doc.setContent(&f)) {
		m_error = QStringLiteral("%1 is not an EPLAN macro (XML)").arg(path);
		return false;
	}
	const QDomElement root = doc.documentElement();
	if (root.tagName() != QLatin1String("EplanPxfRoot")) {
		m_error = QStringLiteral("%1 is not an EPLAN macro").arg(path);
		return false;
	}
	// the multi-line schematic; the other variants are panel layout,
	// single-line, overview and the like
	QDomElement variant;
	for (QDomElement v = root.firstChildElement(QStringLiteral("MacroVariant")); !v.isNull();
	     v = v.nextSiblingElement(QStringLiteral("MacroVariant"))) {
		if (v.attribute(QStringLiteral("MacroFuncType")) == QLatin1String("1")) {
			variant = v;
			break;
		}
	}
	if (variant.isNull()) {
		m_error = QStringLiteral("The macro has no multi-line schematic variant");
		return false;
	}
	m_description = language(variant.attribute(QStringLiteral("Description"),
						    root.attribute(QStringLiteral("Description"))))
			.section(QLatin1Char('\n'), 0, 0).trimmed();

	// containers and settings rather than drawing objects
	static const QStringList structural {
		QStringLiteral("O4"), QStringLiteral("O14"), QStringLiteral("O26"),
		QStringLiteral("O37"), QStringLiteral("O76")};
	QMap<QString, int> skipped;
	// every element below the variant, in document order (QDom has no "*")
	QVector<QDomElement> all;
	for (QDomElement e = variant.firstChildElement(); !e.isNull();) {
		all << e;
		if (!e.firstChildElement().isNull()) {
			e = e.firstChildElement();
			continue;
		}
		while (!e.isNull() && e != variant && e.nextSiblingElement().isNull())
			e = e.parentNode().toElement();
		if (e.isNull() || e == variant) break;
		e = e.nextSiblingElement();
	}
	for (const QDomElement &e : std::as_const(all)) {
		const QString tag = e.tagName();
		if (!tag.startsWith(QLatin1Char('O')) || structural.contains(tag)) continue;

		if (tag == QLatin1String("O34")) {  // line or polyline
			QVector<QPointF> pts;
			for (const QString &s : e.attribute(QStringLiteral("A621")).split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
				QPointF p;
				if (point(s, &p)) pts << p;
			}
			for (int k = 1; k < pts.size(); ++k) m_symbol.lines << QLineF(pts.at(k - 1), pts.at(k));
		} else if (tag == QLatin1String("O89")) {  // rectangle
			QPointF a, b;
			if (!point(e.attribute(QStringLiteral("A1651")), &a)
			    || !point(e.attribute(QStringLiteral("A1652")), &b))
				continue;
			if (e.attribute(QStringLiteral("A1656")) == QLatin1String("1"))
				m_symbol.filled << QRectF(a, b).normalized();
			else
				addRect(m_symbol.lines, a, b);
		} else if (tag == QLatin1String("O17")) {  // function: connection point or black box
			QPointF pos;
			const QString where = e.firstChildElement(QStringLiteral("S40x1201")).attribute(QStringLiteral("A762"));
			if (!point(where, &pos)) continue;
			const QDomElement props = e.firstChildElement(QStringLiteral("P11"));
			const QDomElement frame = e.firstChildElement(QStringLiteral("S89x5"));
			if (!frame.isNull()) {
				QPointF a, b;
				if (!point(frame.attribute(QStringLiteral("A1651")), &a)
				    || !point(frame.attribute(QStringLiteral("A1652")), &b))
					continue;
				const QRectF box = QRectF(pos + a, pos + b).normalized();
				addRect(m_symbol.lines, box.topLeft(), box.bottomRight());
				for (const QString &t : {QStringLiteral("P150"), QStringLiteral("P144")}) {
					const QString letters = e.firstChildElement(t).attribute(QStringLiteral("P20013"));
					if (!letters.isEmpty() && m_letters.isEmpty()) m_letters = letters;
				}
				// technical data "P: 2000VA IN: 208...550VAC OUT: 2x115VAC":
				// one line per "KEY:" as EPLAN shows it, right-aligned in the
				// upper right of the box (QRectF bottom() is the upper edge, y up)
				const QString data = language(props.attribute(QStringLiteral("P20027")));
				static const QRegularExpression keys(QStringLiteral("\\s+(?=[A-Za-z]{1,6}:)|\\n"));
				// QET's font runs wider than EPLAN's: keep clear of the frame
				double y = box.bottom() - 6;
				for (const QString &line : data.split(keys, Qt::SkipEmptyParts)) {
					m_symbol.texts << text(line.trimmed(), QPointF(box.right() - 5, y), 2);
					y -= 3.5;
				}
				// part number ("MURR.86153" -> "86153") at the lower right corner
				const QString number = props.attribute(QStringLiteral("P20100_1")).section(QLatin1Char('.'), 1);
				if (!number.isEmpty())
					m_symbol.texts << text(number, QPointF(box.right() - 2, box.top() + 2), 2);
			} else if (e.hasAttribute(QStringLiteral("A185"))) {
				EplanDxfPin pin;
				pin.pos = pos;
				pin.name = e.attribute(QStringLiteral("A185"));
				// A185 = connection designation (FUNC_TERMINALDESIGNATION 20028),
				// A186 = connection description (20029), e.g. "DI a.0", "2L+".
				// P20406 is the plug designation ("X10"), not the description.
				pin.function = language(e.attribute(QStringLiteral("A186")));
				// Without a description, show the plug designation where the
				// macro itself places it as a property text (A771 = 20406),
				// e.g. "IN"/"OUT" on a transformer, as EPLAN draws it.
				if (pin.function.isEmpty()) {
					for (QDomElement c = e.firstChildElement(); !c.isNull();
					     c = c.nextSiblingElement()) {
						if (c.attribute(QStringLiteral("A771")) == QLatin1String("20406")) {
							pin.function = language(props.attribute(QStringLiteral("P20406")));
							break;
						}
					}
				}
				m_symbol.pins << pin;
				// EPLAN's connection point: a small circle the wire ends at
				m_symbol.circles << qMakePair(pos, 1.0);
			}
		} else if (tag == QLatin1String("O91")) {
			++skipped[QStringLiteral("picture")];
		} else if (e.hasAttribute(QStringLiteral("Build"))) {
			++skipped[tag];
		}
	}
	for (auto it = skipped.cbegin(); it != skipped.cend(); ++it)
		m_skipped << QStringLiteral("%1 x %2").arg(it.value()).arg(it.key());

	if (m_symbol.pins.isEmpty() && m_symbol.lines.isEmpty()) {
		m_error = QStringLiteral("The schematic of the macro holds no drawing");
		return false;
	}
	m_symbol.pin_fields = false;
	m_symbol.source = QStringLiteral("Generated from the manufacturer's EPLAN macro");
	return true;
}
