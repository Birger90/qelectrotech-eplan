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
#include "edzpart.h"

#include <QDir>
#include <QDirIterator>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QPair>
#include <QRegularExpression>

namespace {

/**
	Parse EPLAN's `de_DE@..;en_EN@..;..` multilingual blob into ordered
	(full_code, text) pairs. Values may contain `;;` as an in-text separator, so
	we split on language *markers* (xx_YY@) rather than on bare `;`.
*/
QList<QPair<QString, QString>> langDict(const QString &value)
{
	QList<QPair<QString, QString>> out;
	if (value.isEmpty()) {
		return out;
	}
	static const QRegularExpression re(QStringLiteral("[a-z]{2}_[A-Z]{2}@"));
	QList<QRegularExpressionMatch> marks;
	auto it = re.globalMatch(value);
	while (it.hasNext()) {
		marks.append(it.next());
	}
	for (int i = 0; i < marks.size(); ++i) {
		const QRegularExpressionMatch &m = marks.at(i);
		const QString code = value.mid(m.capturedStart(),
						   m.capturedLength() - 1); // drop '@'
		const int text_start = m.capturedEnd();
		const int text_end = (i + 1 < marks.size())
					 ? marks.at(i + 1).capturedStart()
					 : value.size();
		QString text = value.mid(text_start, text_end - text_start);
		while (!text.isEmpty() && text.back().isSpace()) {
			text.chop(1);
		}
		while (!text.isEmpty() && text.back() == QLatin1Char(';')) {
			text.chop(1);
		}
		text.replace(QStringLiteral(";;"), QStringLiteral("; "));
		text = text.trimmed();
		if (!text.isEmpty()) {
			out.append({code, text});
		}
	}
	return out;
}

/** Pick one language: preferred English variants, then any English, then first. */
QString pickLang(const QString &value)
{
	if (value.isEmpty()) {
		return QString();
	}
	const QList<QPair<QString, QString>> d = langDict(value);
	if (d.isEmpty()) {
		return value.trimmed();
	}
	for (const QString &code : {QStringLiteral("en_EN"), QStringLiteral("en_US"),
					QStringLiteral("en_GB")}) {
		for (const auto &p : d) {
			if (p.first == code && !p.second.isEmpty()) {
				return p.second;
			}
		}
	}
	for (const auto &p : d) {
		if (p.first.startsWith(QStringLiteral("en_")) && !p.second.isEmpty()) {
			return p.second;
		}
	}
	return d.first().second;
}

/** {2-letter QET lang -> text}; first variant of each language wins. */
QMap<QString, QString> multilangNames(const QString &value)
{
	QMap<QString, QString> names;
	for (const auto &p : langDict(value)) {
		const QString s = p.first.left(2).toLower();
		if (!names.contains(s)) {
			names.insert(s, p.second);
		}
	}
	return names;
}

/** Tokenise a designation into (is-text, number, text) runs for natural sort. */
struct NatTok { int type; qlonglong num; QString str; };

QList<NatTok> natKey(const QString &s)
{
	QList<NatTok> out;
	static const QRegularExpression re(QStringLiteral("\\d+|\\D+"));
	auto it = re.globalMatch(s);
	while (it.hasNext()) {
		const QString t = it.next().captured();
		bool all_digit = !t.isEmpty();
		for (const QChar c : t) {
			if (!c.isDigit()) { all_digit = false; break; }
		}
		if (all_digit) {
			out.append({0, t.toLongLong(), QString()});
		} else {
			out.append({1, 0, t.toLower()});
		}
	}
	return out;
}

bool natLess(const QString &a, const QString &b)
{
	const QList<NatTok> ka = natKey(a), kb = natKey(b);
	const int n = qMin(ka.size(), kb.size());
	for (int i = 0; i < n; ++i) {
		if (ka[i].type != kb[i].type) {
			return ka[i].type < kb[i].type;
		}
		if (ka[i].type == 0) {
			if (ka[i].num != kb[i].num) return ka[i].num < kb[i].num;
		} else {
			if (ka[i].str != kb[i].str) return ka[i].str < kb[i].str;
		}
	}
	return ka.size() < kb.size();
}

double mmAttr(const QDomElement &e, const char *name)
{
	return e.attribute(QLatin1String(name)).replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
}

/**
	The <construction> of a part (EPLAN "drilling pattern"): the part.xml
	names it in P_ARTICLE_REF_CONSTRUCTION_NAME, the EDZ carries it in a
	file *.construction.xml beside the part.xml.
*/
QDomElement findConstruction(const QString &dir, const QString &name)
{
	if (name.isEmpty()) return QDomElement();
	QDirIterator it(dir, {QStringLiteral("*.construction.xml")}, QDir::Files);
	while (it.hasNext()) {
		QFile f(it.next());
		if (!f.open(QIODevice::ReadOnly)) continue;
		QDomDocument doc;
		if (!doc.setContent(&f)) continue;
		const QDomNodeList list = doc.elementsByTagName(QStringLiteral("construction"));
		for (int i = 0; i < list.size(); ++i) {
			const QDomElement c = list.at(i).toElement();
			if (c.attribute(QStringLiteral("P_PART_CONSTRUCTION_NAME")) == name) return c;
		}
	}
	return QDomElement();
}
} // namespace

/** Parse the part.xml at @a part_xml_path into this object. */
bool EdzPart::parse(const QString &part_xml_path)
{
	m_error.clear();

	QFile file(part_xml_path);
	if (!file.open(QIODevice::ReadOnly)) {
		m_error = QStringLiteral("Cannot open %1").arg(part_xml_path);
		return false;
	}
	QDomDocument doc;
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0) // TODO Qt 6.5: remove
	QString parse_err;
	int line = 0, col = 0;
	if (!doc.setContent(&file, &parse_err, &line, &col)) {
		m_error = QStringLiteral("Malformed part.xml (line %1): %2")
				  .arg(line).arg(parse_err);
#else
	const auto result = doc.setContent(&file);
	if (!result) {
		m_error = QStringLiteral("Malformed part.xml (line %1, column %2): %3")
				  .arg(result.errorLine)
				  .arg(result.errorColumn)
				  .arg(result.errorMessage);
#endif
		return false;
	}
	file.close();

	const QDomElement part =
		doc.documentElement().firstChildElement(QStringLiteral("part"));
	if (part.isNull()) {
		m_error = QStringLiteral("No <part> element in part.xml");
		return false;
	}

	m_part_number  = part.attribute(QStringLiteral("P_ARTICLE_PARTNR"));
	m_manufacturer = part.attribute(QStringLiteral("P_ARTICLE_MANUFACTURER"));
	m_order_number = part.attribute(QStringLiteral("P_ARTICLE_ORDERNR"));
	m_type_number  = part.attribute(QStringLiteral("P_ARTICLE_TYPENR"));

	const QString descr1 = part.attribute(QStringLiteral("P_ARTICLE_DESCR1"));
	const QString descr2 = part.attribute(QStringLiteral("P_ARTICLE_DESCR2"));
	m_description = pickLang(descr1);
	m_comment     = pickLang(descr2);
	m_names       = multilangNames(descr1);

	// Mounting data (EPLAN Pro Panel). The drilling positions of the
	// construction are measured from the part's placement point, the lower
	// left corner of its front, x to the right and y up, shifted by the
	// <constructionPosition> of the part; the element keeps them from its
	// upper left corner (y up, so negative downwards).
	m_width = mmAttr(part, "P_ARTICLE_WIDTH");
	m_height = mmAttr(part, "P_ARTICLE_HEIGHT");
	m_depth = mmAttr(part, "P_ARTICLE_DEPTH");
	m_snap_height = mmAttr(part, "P_ARTICLE_SNAPHEIGHT");
	m_middle_offset = mmAttr(part, "P_ARTICLE_MIDDLEOFFSET");
	m_patterns.clear();
	{
		PanelPattern rail;
		rail.variant = QStringLiteral("Hutschiene");
		rail.din_rail = true;
		const QDomElement construction = findConstruction(
			QFileInfo(part_xml_path).absolutePath(),
			part.attribute(QStringLiteral("P_ARTICLE_REF_CONSTRUCTION_NAME")));
		const QDomElement cpos = part.firstChildElement(QStringLiteral("constructionPosition"));
		const double off_x = mmAttr(cpos, "offsetx"), off_y = mmAttr(cpos, "offsety");
		PanelPattern screws;
		screws.variant = QStringLiteral("Schraubbefestigung");
		for (QDomElement d = construction.firstChildElement(QStringLiteral("drillingPosition"));
			 !d.isNull(); d = d.nextSiblingElement(QStringLiteral("drillingPosition"))) {
			PanelHole h;
			h.type = PanelHole::Type(d.attribute(QStringLiteral("type"), QStringLiteral("1")).toInt());
			h.subtype = PanelHole::SubType(d.attribute(QStringLiteral("subtype")).toInt());
			h.d1 = mmAttr(d, "dimension1");
			h.d2 = mmAttr(d, "dimension2");
			h.d3 = mmAttr(d, "dimension3");
			h.x = mmAttr(d, "xpos") + off_x;
			h.y = mmAttr(d, "ypos") + off_y - m_height;
			h.angle = mmAttr(d, "angle");
			h.repeat = mmAttr(d, "spacingrepeat");
			h.end = mmAttr(d, "spacingend");
			h.nth = d.attribute(QStringLiteral("drillnthhole")).toInt();
			if (h.nth == 1) h.nth = 0;
			if (h.d1 > 0) screws.holes << h;
		}
			// a part with a drilling pattern is screwed by default, unless it
			// has a snap height; one without can only snap onto a rail
		if (!screws.holes.isEmpty()) {
			if (m_snap_height > 0) m_patterns << rail << screws;
			else m_patterns << screws << rail;
		} else {
			m_patterns << rail;
		}
	}

	const QString pic = part.attribute(QStringLiteral("P_ARTICLE_PICTUREFILE"));
	if (!pic.isEmpty()) {
		m_picture = QFileInfo(QString(pic).replace('\\', '/')).fileName();
	}

	// Pins = function templates that carry a physical connection designation.
	const QDomNodeList fts =
		part.elementsByTagName(QStringLiteral("functiontemplate"));
	for (int i = 0; i < fts.size(); ++i) {
		const QDomElement ft = fts.at(i).toElement();
		// One function template is one function (a contact, a coil, a PLC
		// channel); its connectionDesignation lists ALL of that function's
		// connection points, one per line, e.g. ".2\n.1\n.4" for a changeover
		// contact (EPLAN help, XML export: functiontemplate).  The matching
		// connectiondescription is line-aligned.
		static const QRegularExpression lines(QStringLiteral("\\r?\\n"));
		const QStringList desigs =
			ft.attribute(QStringLiteral("connectionDesignation")).split(lines);
		const QStringList descrs =
			ft.attribute(QStringLiteral("connectiondescription")).split(lines);
		// terminalNr identifies the physical connector socket (X01, X31, …) and
		// is the preferred group key.  Older EPLAN formats may omit it and carry
		// a text functiondefinition block name (FINP, MOUT, …) instead, either on
		// the <functiontemplate> itself or on its parent wrapper element.
		QString group = ft.attribute(QStringLiteral("terminalNr")).trimmed();
		if (group.isEmpty()) {
			group = ft.attribute(QStringLiteral("functiondefinition")).trimmed();
			if (group.isEmpty())
				group = ft.parentNode().toElement()
						.attribute(QStringLiteral("functiondefinition")).trimmed();
		}
		for (int k = 0; k < desigs.size(); ++k) {
			const QString desig = desigs.at(k).trimmed();
			if (desig.isEmpty()) {
				continue;
			}
			EdzPin pin;
			pin.designation = desig;
			pin.description = k < descrs.size() ? descrs.at(k).trimmed() : QString();
			pin.group = group;
			m_pins.append(pin);
		}
	}

	// Sort: preserve the XML order of functional groups (first-seen wins), and
	// within each group sort by designation using natural sort so pins stack
	// numerically (1, 2, 3 …) on the symbol.
	QMap<QString, int> group_order;
	for (const EdzPin &p : m_pins) {
		if (!group_order.contains(p.group))
			group_order.insert(p.group, group_order.size());
	}
	std::stable_sort(m_pins.begin(), m_pins.end(),
			 [&group_order](const EdzPin &a, const EdzPin &b) {
				 const int ga = group_order.value(a.group, 0);
				 const int gb = group_order.value(b.group, 0);
				 if (ga != gb) return ga < gb;
				 return natLess(a.designation, b.designation);
			 });
	return true;
}

/**
	@brief EdzPart::appendMountingData
	Write the outline (width_mm, height_mm, depth_mm, snap height and
	middle offset) into the element information of @p element_definition
	and its mounting variants into <drillingpatterns>, for the cabinet
	layout.
*/
void EdzPart::appendMountingData(QDomDocument &element_definition) const
{
	QDomElement root = element_definition.documentElement();
	if (root.isNull()) return;
	QDomElement infos = root.firstChildElement(QStringLiteral("elementInformations"));
	if (infos.isNull()) {
		infos = element_definition.createElement(QStringLiteral("elementInformations"));
		root.insertAfter(infos, root.firstChildElement(QStringLiteral("names")));
	}
	auto set = [&](const QString &name, double value) {
		if (value == 0) return;
		QDomElement e = infos.firstChildElement();
		while (!e.isNull()) {
			QDomElement next = e.nextSiblingElement();
			if (e.attribute(QStringLiteral("name")) == name) infos.removeChild(e);
			e = next;
		}
		QDomElement n = element_definition.createElement(QStringLiteral("elementInformation"));
		n.setAttribute(QStringLiteral("name"), name);
		n.setAttribute(QStringLiteral("show"), 0);
		n.appendChild(element_definition.createTextNode(
			QString::number(value, 'g', 6).replace(QLatin1Char('.'), QLatin1Char(','))));
		infos.appendChild(n);
	};
	set(QStringLiteral("width_mm"), m_width);
	set(QStringLiteral("height_mm"), m_height);
	set(QStringLiteral("depth_mm"), m_depth);
	set(QStringLiteral("snap_height_mm"), m_snap_height);
	set(QStringLiteral("middle_offset_mm"), m_middle_offset);

	QDomElement old = root.firstChildElement(QStringLiteral("drillingpatterns"));
	if (!old.isNull()) root.removeChild(old);
	if (m_patterns.isEmpty()) return;
	QDomElement patterns = element_definition.createElement(QStringLiteral("drillingpatterns"));
	for (const PanelPattern &p : m_patterns) patterns.appendChild(p.toXml(element_definition));
	root.insertBefore(patterns, root.firstChildElement(QStringLiteral("description")));
}
