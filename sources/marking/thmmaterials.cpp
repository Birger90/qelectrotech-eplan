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
#include "thmmaterials.h"

#include <QHash>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>

namespace {

	/// The colour codes Weidmüller puts at the end of its names
QString colourCode(const QString &code)
{
	static const QHash<QString, QString> codes {
		{QStringLiteral("WS"), QStringLiteral("Weiß")}, {QStringLiteral("W"), QStringLiteral("Weiß")},
		{QStringLiteral("IW"), QStringLiteral("Weiß")}, {QStringLiteral("GE"), QStringLiteral("Gelb")},
		{QStringLiteral("Y"), QStringLiteral("Gelb")}, {QStringLiteral("RT"), QStringLiteral("Rot")},
		{QStringLiteral("R"), QStringLiteral("Rot")}, {QStringLiteral("BL"), QStringLiteral("Blau")},
		{QStringLiteral("B"), QStringLiteral("Blau")}, {QStringLiteral("GN"), QStringLiteral("Grün")},
		{QStringLiteral("G"), QStringLiteral("Grün")}, {QStringLiteral("GR"), QStringLiteral("Grau")},
		{QStringLiteral("SW"), QStringLiteral("Schwarz")}, {QStringLiteral("SI"), QStringLiteral("Silber")},
		{QStringLiteral("OR"), QStringLiteral("Orange")}, {QStringLiteral("TU"), QStringLiteral("Türkis")},
		{QStringLiteral("TR"), QStringLiteral("Transparent")}, {QStringLiteral("VI"), QStringLiteral("Violett")},
		{QStringLiteral("BR"), QStringLiteral("Braun")}};
	return codes.value(code.toUpper());
}

	/// "Länge x Breite: 18 x 4 mm" (or Höhe/Breite) of a description
bool sizeOf(const QString &description, double *a, double *b)
{
	static const QRegularExpression form(QStringLiteral(
		"(?:Länge|Breite|Höhe|Length|Width|Height)\\s*[x×]\\s*(?:Länge|Breite|Höhe|Length|Width|Height):\\s*"
		"(\\d+(?:[.,]\\d+)?)\\s*[x×]\\s*(\\d+(?:[.,]\\d+)?)\\s*mm"), QRegularExpression::CaseInsensitiveOption);
	const QRegularExpressionMatch m = form.match(description);
	if (!m.hasMatch()) return false;
	*a = m.captured(1).replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
	*b = m.captured(2).replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
	return true;
}

bool twoSided(const QString &name, const QString &layout)
{
	return name.endsWith(QLatin1String("-DS"), Qt::CaseInsensitive)
		|| layout.endsWith(QLatin1String("_ds.mcf"), Qt::CaseInsensitive);
}

QStringList loadedOn(const QList<ThmPrinter> &printers, const QString &article)
{
	QStringList names;
	for (const ThmPrinter &p : printers) {
		if (!article.isEmpty() && ThmPrinters::normalizedMaterial(p.material_no)
				== ThmPrinters::normalizedMaterial(article))
			names << p.name;
	}
	return names;
}

QStringList distinct(const QList<ThmMaterials::Entry> &entries, QString (*value)(const ThmMaterials::Entry &))
{
	QSet<QString> seen;
	QStringList out;
	for (const ThmMaterials::Entry &e : entries) {
		const QString v = value(e);
		if (!v.isEmpty() && !seen.contains(v)) {
			seen.insert(v);
			out << v;
		}
	}
	std::sort(out.begin(), out.end(), [](const QString &a, const QString &b) {
		return a.localeAwareCompare(b) < 0;
	});
	return out;
}

}

QString ThmMaterials::Entry::sizeText() const
{
	if (width <= 0 || height <= 0) return QString();
	return QStringLiteral("%1 × %2 mm").arg(ThmJob::number(width), ThmJob::number(height));
}

QString ThmMaterials::colourOf(const QString &name, const QString &description)
{
	static const QRegularExpression farbe(QStringLiteral("Farbe:\\s*([^,;]+?)(?=Farbe:|,|;|$)"),
										  QRegularExpression::CaseInsensitiveOption);
	const QRegularExpressionMatch m = farbe.match(description);
	if (m.hasMatch()) {
		QString c = m.captured(1).trimmed();
		if (c.compare(QLatin1String("Yellow"), Qt::CaseInsensitive) == 0) c = QStringLiteral("Gelb");
		if (!c.isEmpty()) return c.left(1).toUpper() + c.mid(1);
	}
		//The last word of the name: "WS", "Y30M", "W-DS", "RT/WS"
	QString code = name.section(QLatin1Char(' '), -1).toUpper();
	code.remove(QRegularExpression(QStringLiteral("-DS$")));
	code.remove(QRegularExpression(QStringLiteral("\\d+(?:[.,]\\d+)?M$")));
	QStringList parts;
	for (const QString &part : code.split(QLatin1Char('/'), Qt::SkipEmptyParts)) {
		const QString c = colourCode(part);
		if (c.isEmpty()) return QString();
		parts << c;
	}
	return parts.join(QLatin1Char('/'));
}

QList<ThmMaterials::Entry> ThmMaterials::entries(const QList<ThmLayouts::Variant> &layouts,
												 const QList<ThmCatalog::Variant> &catalog,
												 const QList<ThmPrinter> &printers)
{
	QHash<QString, const ThmCatalog::Variant *> by_number;
	for (const ThmCatalog::Variant &c : catalog)
		by_number.insert(ThmPrinters::normalizedMaterial(c.material_no), &c);

	QList<Entry> out;
	QSet<QString> with_layout;
	for (const ThmLayouts::Variant &v : layouts) {
		const ThmCatalog::Variant *c = by_number.value(ThmPrinters::normalizedMaterial(v.article));
		Entry e;
		e.article = v.article;
		e.key = v.key();
		e.name = v.name;
		e.category = v.category;
		e.width = v.marker_width;
		e.height = v.marker_height;
		if ((e.width <= 0 || e.height <= 0) && c) sizeOf(c->description, &e.width, &e.height);
		e.colour = colourOf(v.name, c ? c->description : QString());
		e.two_sided = twoSided(v.name, v.layout);
		e.columns = v.columns;
		e.refusal = v.refusal;
		e.has_layout = true;
		e.loaded_on = loadedOn(printers, v.article);
		with_layout.insert(ThmPrinters::normalizedMaterial(v.article));
		out << e;
	}
	for (const ThmCatalog::Variant &c : catalog) {
		if (with_layout.contains(ThmPrinters::normalizedMaterial(c.material_no))) continue;
		Entry e;
		e.article = c.material_no;
		e.name = c.name;
		sizeOf(c.description, &e.width, &e.height);
		e.colour = colourOf(c.name, c.description);
		e.two_sided = twoSided(c.name, QString());
		e.loaded_on = loadedOn(printers, c.material_no);
		out << e;
	}
	std::stable_sort(out.begin(), out.end(), [](const Entry &a, const Entry &b) {
		const int n = a.name.compare(b.name, Qt::CaseInsensitive);
		return n ? n < 0 : a.article < b.article;
	});
	return out;
}

bool ThmMaterials::matches(const Entry &e, const Filter &f)
{
	if (f.printable_only && !e.refusal.isEmpty()) return false;
	if (f.loaded_only && e.loaded_on.isEmpty()) return false;
	if (!f.category.isEmpty() && e.category != f.category) return false;
	if (!f.colour.isEmpty() && e.colour.compare(f.colour, Qt::CaseInsensitive) != 0) return false;
	if (!f.size.isEmpty() && e.sizeText() != f.size) return false;
	if (f.sides == 1 && e.two_sided) return false;
	if (f.sides == 2 && !e.two_sided) return false;
	const QString text = f.text.trimmed();
	if (!text.isEmpty()) {
		for (const QString &word : text.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
			if (!e.name.contains(word, Qt::CaseInsensitive) && !e.article.contains(word))
				return false;
		}
	}
	return true;
}

QStringList ThmMaterials::categories(const QList<Entry> &entries)
{
	return distinct(entries, [](const Entry &e) { return e.category; });
}

QStringList ThmMaterials::colours(const QList<Entry> &entries)
{
	return distinct(entries, [](const Entry &e) { return e.colour; });
}

QStringList ThmMaterials::sizes(const QList<Entry> &entries)
{
	QStringList out = distinct(entries, [](const Entry &e) { return e.sizeText(); });
		//by width, then height, as numbers
	std::sort(out.begin(), out.end(), [](const QString &a, const QString &b) {
		const double aw = a.section(QLatin1Char(' '), 0, 0).toDouble(), bw = b.section(QLatin1Char(' '), 0, 0).toDouble();
		if (aw != bw) return aw < bw;
		return a.section(QLatin1Char(' '), 2, 2).toDouble() < b.section(QLatin1Char(' '), 2, 2).toDouble();
	});
	return out;
}
