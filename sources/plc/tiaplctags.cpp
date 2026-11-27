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
#include "tiaplctags.h"

#include "../container/qetzip.h"
#include "../diagram.h"
#include "../properties/elementdata.h"
#include "../qetgraphicsitem/element.h"
#include "../qetproject.h"
#include "../qetversion.h"
#include "../undocommand/changeelementdatacommand.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMap>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>

#include <algorithm>

namespace TiaPlcTags {

namespace {

QString tr(const char *text)
{
	return QCoreApplication::translate("TiaPlcTags", text);
}

QString oneLine(QString text)
{
	text.replace(QLatin1String("\r\n"), QLatin1String(" "));
	text.replace(QLatin1Char('\n'), QLatin1Char(' '));
	text.replace(QLatin1Char('\r'), QLatin1Char(' '));
	return text.simplified();
}

QString elementLabel(Element *element)
{
	if (!element) return {};
	QString label = element->actualLabel();
	if (label.isEmpty())
		label = element->elementInformations().value(QStringLiteral("label")).toString();
	return label.isEmpty() ? element->name() : label;
}

bool isInput(ElementData::PlcIOType type)
{
	return type == ElementData::EntreeDigitale || type == ElementData::EntreeAnalogique
		|| type == ElementData::EntreeUniverselle;
}

bool isAnalog(ElementData::PlcIOType type)
{
	return type == ElementData::EntreeAnalogique || type == ElementData::SortieAnalogique;
}

QString kindOf(ElementData::PlcIOType type)
{
	if (isAnalog(type)) return isInput(type) ? QStringLiteral("AI") : QStringLiteral("AQ");
	return isInput(type) ? QStringLiteral("DI") : QStringLiteral("DQ");
}

/// parts of a normalized address: area I/Q, size ""/B/W/D, byte, bit (-1)
struct Parts {
	QChar area;
	QString size;
	int byte = -1;
	int bit = -1;
	bool valid() const { return byte >= 0; }
	int length() const {
		if (size == QLatin1String("W")) return 2;
		if (size == QLatin1String("D")) return 4;
		return 1;
	}
};

Parts parse(const QString &raw)
{
	Parts parts;
	QString s = raw.trimmed().toUpper();
	s.remove(QLatin1Char(' '));
	if (s.startsWith(QLatin1Char('%'))) s.remove(0, 1);
	if (s.endsWith(QLatin1String(":P"))) s.chop(2);
	static const QRegularExpression rx(
		QStringLiteral("^P?([IEQA])([XBWD])?(\\d{1,5})(?:\\.([0-7]))?$"));
	const auto m = rx.match(s);
	if (!m.hasMatch()) return parts;
	QString size = m.captured(2);
	const bool has_bit = m.capturedLength(4) > 0;
	if (size == QLatin1String("X")) {
		if (!has_bit) return parts;
		size.clear();
	}
	// "E0" names a channel, not a byte: an absolute address has a bit or a size
	if (size.isEmpty() && !has_bit) return parts;
	if (!size.isEmpty() && has_bit) return parts;
	const QChar a = m.captured(1).at(0);
	parts.area = (a == QLatin1Char('I') || a == QLatin1Char('E')) ? QLatin1Char('I') : QLatin1Char('Q');
	parts.size = size;
	parts.byte = m.captured(3).toInt();
	parts.bit = has_bit ? m.captured(4).toInt() : -1;
	return parts;
}

QString format(const Parts &p)
{
	if (!p.valid()) return {};
	QString s = QLatin1Char('%') + QString(p.area) + p.size + QString::number(p.byte);
	if (p.bit >= 0) s += QLatin1Char('.') + QString::number(p.bit);
	return s;
}

/// channel number of a row that only names its channel ("DI3", "AQ0", "3")
int relativeChannel(const QString &raw)
{
	static const QRegularExpression rx(
		QStringLiteral("^(?:DI|DQ|DO|DE|DA|AI|AQ|AO|AE|AA|UI|UO|CH|KANAL|E|A|I|Q)?\\s*_?(\\d{1,3})$"),
		QRegularExpression::CaseInsensitiveOption);
	const auto m = rx.match(raw.trimmed());
	return m.hasMatch() ? m.captured(1).toInt() : -1;
}

QString dataTypeFor(const Parts &p, bool analog)
{
	if (p.bit >= 0 || p.size.isEmpty()) return QStringLiteral("Bool");
	if (p.size == QLatin1String("B")) return QStringLiteral("Byte");
	if (p.size == QLatin1String("W")) return analog ? QStringLiteral("Int") : QStringLiteral("Word");
	return analog ? QStringLiteral("DInt") : QStringLiteral("DWord");
}

/// first byte at or after @p from with @p length free bytes in @p used
int freeRun(const QSet<int> &used, int length, int from = 0)
{
	int start = qMax(0, from);
	for (;;) {
		bool ok = true;
		for (int i = 0; i < length; ++i) {
			if (used.contains(start + i)) { start += i + 1; ok = false; break; }
		}
		if (ok) return start;
	}
}

QString nameToken(const QString &text)
{
	QString s = text;
	s.remove(QLatin1Char('"'));
	s.remove(QLatin1Char('%'));
	while (s.startsWith(QLatin1Char('-')) || s.startsWith(QLatin1Char('+')) || s.startsWith(QLatin1Char('=')))
		s.remove(0, 1);
	s.replace(QRegularExpression(QStringLiteral("[\\s.:%/\\\\]+")), QStringLiteral("_"));
	return s;
}

// ---------------------------------------------------------------- xlsx

QString xmlEscape(const QString &text)
{
	return text.toHtmlEscaped();
}

QString columnName(int column)
{
	QString name;
	++column;
	while (column > 0) {
		const int rest = (column - 1) % 26;
		name.prepend(QChar('A' + rest));
		column = (column - 1) / 26;
	}
	return name;
}

class SharedStrings
{
	public:
		int index(const QString &text) {
			auto it = m_index.constFind(text);
			if (it != m_index.constEnd()) return it.value();
			m_list << text;
			m_index.insert(text, m_list.size() - 1);
			return m_list.size() - 1;
		}
		int uses = 0;
		QByteArray xml() const {
			QString s = QStringLiteral(
				"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
				"<sst xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
				"count=\"%1\" uniqueCount=\"%2\">").arg(uses).arg(m_list.size());
			for (const QString &t : m_list)
				s += QStringLiteral("<si><t xml:space=\"preserve\">") + xmlEscape(t) + QStringLiteral("</t></si>");
			s += QStringLiteral("</sst>");
			return s.toUtf8();
		}
	private:
		QStringList m_list;
		QHash<QString, int> m_index;
};

QByteArray sheetXml(const QVector<QStringList> &rows, SharedStrings &strings)
{
	QString s = QStringLiteral(
		"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
		"<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheetData>");
	for (int r = 0; r < rows.size(); ++r) {
		s += QStringLiteral("<row r=\"%1\">").arg(r + 1);
		const QStringList &row = rows.at(r);
		for (int c = 0; c < row.size(); ++c) {
			if (row.at(c).isEmpty()) continue;
			++strings.uses;
			s += QStringLiteral("<c r=\"%1%2\" t=\"s\"><v>%3</v></c>")
				.arg(columnName(c)).arg(r + 1).arg(strings.index(row.at(c)));
		}
		s += QStringLiteral("</row>");
	}
	s += QStringLiteral("</sheetData></worksheet>");
	return s.toUtf8();
}

bool writeXlsx(const QString &path, const QVector<Channel> &channels, const QString &table,
			   QString *error)
{
	QVector<QStringList> tags;
	tags << QStringList{QStringLiteral("Name"), QStringLiteral("Path"), QStringLiteral("Data Type"),
						QStringLiteral("Logical Address"), QStringLiteral("Comment"),
						QStringLiteral("Hmi Visible"), QStringLiteral("Hmi Accessible"),
						QStringLiteral("Hmi Writeable"), QStringLiteral("Typeobject ID"),
						QStringLiteral("Version ID")};
	for (const Channel &c : channels) {
		if (c.address.isEmpty()) continue;
		tags << QStringList{c.name, table, c.dataType, c.address, c.function,
							QStringLiteral("True"), QStringLiteral("True"), QStringLiteral("True"),
							QString(), QString()};
	}
	QVector<QStringList> props;
	props << QStringList{QStringLiteral("Path"), QStringLiteral("BelongsToUnit"), QStringLiteral("Accessibility")};
	props << QStringList{table, QString(), QString()};

	SharedStrings strings;
	const QByteArray sheet1 = sheetXml(tags, strings);
	const QByteArray sheet2 = sheetXml(props, strings);

	const QByteArray head("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
	QList<QetZip::Entry> entries;
	entries << QetZip::Entry{QStringLiteral("[Content_Types].xml"), head +
		"<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
		"<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
		"<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
		"<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
		"<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
		"<Override PartName=\"/xl/worksheets/sheet2.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
		"<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
		"<Override PartName=\"/xl/sharedStrings.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml\"/>"
		"<Override PartName=\"/docProps/core.xml\" ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/>"
		"<Override PartName=\"/docProps/app.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.extended-properties+xml\"/>"
		"<Override PartName=\"/docProps/custom.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.custom-properties+xml\"/>"
		"</Types>"};
	entries << QetZip::Entry{QStringLiteral("_rels/.rels"), head +
		"<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
		"<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
		"<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties\" Target=\"docProps/core.xml\"/>"
		"<Relationship Id=\"rId3\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties\" Target=\"docProps/app.xml\"/>"
		"<Relationship Id=\"rId4\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/custom-properties\" Target=\"docProps/custom.xml\"/>"
		"</Relationships>"};
	entries << QetZip::Entry{QStringLiteral("docProps/core.xml"), head +
		"<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" "
		"xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:dcterms=\"http://purl.org/dc/terms/\" "
		"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
		"<dc:creator>QElectroTech</dc:creator></cp:coreProperties>"};
	entries << QetZip::Entry{QStringLiteral("docProps/app.xml"), head +
		"<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\">"
		"<Application>QElectroTech</Application></Properties>"};
	// "Missing version ID in the properties of the XLSX file" without it (help "Importing PLC tags")
	entries << QetZip::Entry{QStringLiteral("docProps/custom.xml"), head +
		"<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties\" "
		"xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\">"
		"<property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"2\" name=\"TIA_Version\">"
		"<vt:lpwstr>1.0</vt:lpwstr></property></Properties>"};
	entries << QetZip::Entry{QStringLiteral("xl/workbook.xml"), head +
		"<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
		"xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><sheets>"
		"<sheet name=\"PLCTags\" sheetId=\"1\" r:id=\"rId1\"/>"
		"<sheet name=\"TagTableProperties\" sheetId=\"2\" r:id=\"rId2\"/>"
		"</sheets></workbook>"};
	entries << QetZip::Entry{QStringLiteral("xl/_rels/workbook.xml.rels"), head +
		"<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
		"<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
		"<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet2.xml\"/>"
		"<Relationship Id=\"rId3\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>"
		"<Relationship Id=\"rId4\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings\" Target=\"sharedStrings.xml\"/>"
		"</Relationships>"};
	entries << QetZip::Entry{QStringLiteral("xl/styles.xml"), head +
		"<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
		"<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
		"<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
		"<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
		"<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
		"<cellXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/></cellXfs>"
		"<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>"
		"</styleSheet>"};
	entries << QetZip::Entry{QStringLiteral("xl/worksheets/sheet1.xml"), sheet1};
	entries << QetZip::Entry{QStringLiteral("xl/worksheets/sheet2.xml"), sheet2};
	entries << QetZip::Entry{QStringLiteral("xl/sharedStrings.xml"), strings.xml()};
	return QetZip::write(path, entries, error);
}

QDomDocument parsedXml(const QByteArray &data)
{
	QDomDocument doc;
	doc.setContent(data);
	return doc;
}

QString cellText(const QDomElement &cell, const QStringList &shared)
{
	const QString type = cell.attribute(QStringLiteral("t"));
	if (type == QLatin1String("inlineStr")) {
		QString text;
		const QDomNodeList ts = cell.elementsByTagName(QStringLiteral("t"));
		for (int i = 0; i < ts.count(); ++i) text += ts.at(i).toElement().text();
		return text;
	}
	const QString v = cell.firstChildElement(QStringLiteral("v")).text();
	if (type == QLatin1String("s")) return shared.value(v.toInt());
	if (type == QLatin1String("b")) return v == QLatin1String("1") ? QStringLiteral("True") : QStringLiteral("False");
	return v;
}

int columnOf(const QString &ref)
{
	int col = 0;
	for (const QChar ch : ref) {
		if (!ch.isLetter()) break;
		col = col * 26 + (ch.toUpper().unicode() - 'A' + 1);
	}
	return col - 1;
}

QVector<Tag> readXlsx(const QString &path, QString *error)
{
	QList<QetZip::Entry> entries;
	if (!QetZip::read(path, &entries, error)) return {};
	QHash<QString, QByteArray> files;
	for (const auto &e : entries) files.insert(e.name, e.data);

	QStringList shared;
	{
		const QDomDocument doc = parsedXml(files.value(QStringLiteral("xl/sharedStrings.xml")));
		const QDomNodeList sis = doc.elementsByTagName(QStringLiteral("si"));
		for (int i = 0; i < sis.count(); ++i) {
			QString text;
			const QDomNodeList ts = sis.at(i).toElement().elementsByTagName(QStringLiteral("t"));
			for (int k = 0; k < ts.count(); ++k) text += ts.at(k).toElement().text();
			shared << text;
		}
	}
	// sheet name -> part, in workbook order
	QHash<QString, QString> targets;
	{
		const QDomDocument rels = parsedXml(files.value(QStringLiteral("xl/_rels/workbook.xml.rels")));
		const QDomNodeList rs = rels.elementsByTagName(QStringLiteral("Relationship"));
		for (int i = 0; i < rs.count(); ++i) {
			const QDomElement r = rs.at(i).toElement();
			QString target = r.attribute(QStringLiteral("Target"));
			if (target.startsWith(QLatin1Char('/'))) target.remove(0, 1);
			else target.prepend(QStringLiteral("xl/"));
			targets.insert(r.attribute(QStringLiteral("Id")), target);
		}
	}
	QStringList parts;
	{
		const QDomDocument wb = parsedXml(files.value(QStringLiteral("xl/workbook.xml")));
		const QDomNodeList sheets = wb.elementsByTagName(QStringLiteral("sheet"));
		QStringList preferred, others;
		for (int i = 0; i < sheets.count(); ++i) {
			const QDomElement s = sheets.at(i).toElement();
			QString id = s.attribute(QStringLiteral("r:id"));
			if (id.isEmpty()) id = s.attributeNS(QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"), QStringLiteral("id"));
			const QString part = targets.value(id);
			const QString name = s.attribute(QStringLiteral("name")).remove(QLatin1Char(' ')).toLower();
			(name == QLatin1String("plctags") ? preferred : others) << part;
		}
		parts = preferred + others;
	}

	for (const QString &part : parts) {
		const QDomDocument sheet = parsedXml(files.value(part));
		const QDomNodeList rows = sheet.elementsByTagName(QStringLiteral("row"));
		QHash<QString, int> header;
		QVector<Tag> tags;
		for (int r = 0; r < rows.count(); ++r) {
			QMap<int, QString> cells;
			const QDomNodeList cs = rows.at(r).toElement().elementsByTagName(QStringLiteral("c"));
			int next = 0;
			for (int c = 0; c < cs.count(); ++c) {
				const QDomElement cell = cs.at(c).toElement();
				const QString ref = cell.attribute(QStringLiteral("r"));
				const int col = ref.isEmpty() ? next : columnOf(ref);
				cells.insert(col, cellText(cell, shared));
				next = col + 1;
			}
			if (header.isEmpty()) {
				for (auto it = cells.constBegin(); it != cells.constEnd(); ++it)
					header.insert(it.value().trimmed().toLower(), it.key());
				if (!header.contains(QStringLiteral("name"))
					|| !header.contains(QStringLiteral("logical address"))) {
					break;	// not a tag sheet
				}
				continue;
			}
			auto get = [&](const char *key) {
				const int col = header.value(QLatin1String(key), -1);
				return col < 0 ? QString() : cells.value(col).trimmed();
			};
			Tag tag;
			tag.name = get("name");
			if (tag.name.isEmpty()) continue;
			tag.path = get("path");
			tag.dataType = get("data type");
			tag.address = get("logical address");
			tag.comment = get("comment");
			tags << tag;
		}
		if (!header.isEmpty() && header.contains(QStringLiteral("logical address")))
			return tags;
	}
	if (error) *error = tr("The workbook has no sheet with the columns Name and Logical Address.");
	return {};
}

// ---------------------------------------------------------------- SimaticML

QString childText(const QDomElement &attribute_list, const char *name)
{
	return attribute_list.firstChildElement(QLatin1String(name)).text().trimmed();
}

QVector<Tag> readXml(const QString &path, QString *error)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		if (error) *error = file.errorString();
		return {};
	}
	QDomDocument doc;
	const auto parsed = doc.setContent(&file);
	if (!parsed) {
		if (error) *error = tr("Not an XML file: %1").arg(parsed.errorMessage);
		return {};
	}
	QVector<Tag> tags;
	const QDomNodeList list = doc.elementsByTagName(QStringLiteral("SW.Tags.PlcTag"));
	for (int i = 0; i < list.count(); ++i) {
		const QDomElement e = list.at(i).toElement();
		const QDomElement attrs = e.firstChildElement(QStringLiteral("AttributeList"));
		Tag tag;
		tag.name = childText(attrs, "Name");
		tag.dataType = childText(attrs, "DataTypeName");
		tag.address = childText(attrs, "LogicalAddress");
		// the comment: German first, else the first language
		QString first, german;
		const QDomNodeList texts = e.elementsByTagName(QStringLiteral("MultilingualText"));
		for (int t = 0; t < texts.count(); ++t) {
			const QDomElement mt = texts.at(t).toElement();
			if (mt.attribute(QStringLiteral("CompositionName")) != QLatin1String("Comment")) continue;
			const QDomNodeList items = mt.elementsByTagName(QStringLiteral("MultilingualTextItem"));
			for (int k = 0; k < items.count(); ++k) {
				const QDomElement a = items.at(k).toElement().firstChildElement(QStringLiteral("AttributeList"));
				const QString text = childText(a, "Text");
				if (text.isEmpty()) continue;
				if (first.isEmpty()) first = text;
				if (childText(a, "Culture").startsWith(QLatin1String("de"))) german = text;
			}
		}
		tag.comment = german.isEmpty() ? first : german;
		for (QDomNode p = e.parentNode(); !p.isNull(); p = p.parentNode()) {
			if (p.toElement().tagName() == QLatin1String("SW.Tags.PlcTagTable")) {
				tag.path = childText(p.firstChildElement(QStringLiteral("AttributeList")), "Name");
				break;
			}
		}
		if (!tag.name.isEmpty()) tags << tag;
	}
	if (list.isEmpty() && doc.elementsByTagName(QStringLiteral("SW.Tags.PlcTagTable")).isEmpty()) {
		if (error) *error = tr("The XML file is no TIA Portal tag table (SW.Tags.PlcTagTable).");
	}
	return tags;
}

// ------------------------------------------------- IEC 61131-3 (CODESYS, PLCopen)

/// @p address of TIA (%I50.0, %IW256, %QB3) as IEC 61131-3 writes it:
/// a bit gets its size X (%IX50.0), the rest stays
QString iecAddress(const QString &address)
{
	static const QRegularExpression bit(QStringLiteral("^%([IQ])(\\d+\\.\\d+)$"));
	const QRegularExpressionMatch m = bit.match(address);
	if (m.hasMatch()) return QStringLiteral("%%1X%2").arg(m.captured(1), m.captured(2));
	return address;
}

/// a TIA data type (Bool, Int, Word ...) as IEC 61131-3 elementary type
QString iecType(const QString &type)
{
	return type.isEmpty() ? QStringLiteral("BOOL") : type.toUpper();
}

/// @p name as IEC 61131-3 identifier: letters, digits and single
/// underscores, not beginning with a digit
QString iecName(const QString &name)
{
	QString s;
	for (const QChar ch : name) {
		const bool ok = (ch >= QLatin1Char('a') && ch <= QLatin1Char('z')) || (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z'))
						|| (ch >= QLatin1Char('0') && ch <= QLatin1Char('9'));
		if (ok) s += ch;
		else if (!s.endsWith(QLatin1Char('_'))) s += QLatin1Char('_');
	}
	while (s.endsWith(QLatin1Char('_'))) s.chop(1);
	while (s.startsWith(QLatin1Char('_'))) s.remove(0, 1);   // "-G1" -> "G1"
	if (s.isEmpty() || s.at(0).isDigit()) s.prepend(QStringLiteral("T_"));
	return s;
}

/// the channels with an address, each with a unique IEC name
QVector<QPair<QString, const Channel *>> iecChannels(const QVector<Channel> &channels)
{
	QVector<QPair<QString, const Channel *>> out;
	QSet<QString> used;
	for (const Channel &c : channels) {
		if (c.address.isEmpty()) continue;
		QString name = iecName(c.name);
		const QString base = name;
		for (int n = 2; used.contains(name.toUpper()); ++n) name = base + QStringLiteral("_%1").arg(n);
		used << name.toUpper();
		out << qMakePair(name, &c);
	}
	return out;
}

bool saveText(const QString &path, const QString &text, QString *error)
{
	QSaveFile file(path);
	if (!file.open(QIODevice::WriteOnly)) {
		if (error) *error = file.errorString();
		return false;
	}
	file.write(text.toUtf8());
	if (!file.commit()) {
		if (error) *error = file.errorString();
		return false;
	}
	return true;
}

/// CODESYS global variable list (Structured Text): one variable per
/// channel, at its address, the function text as comment
bool writeCodesysGvl(const QString &path, const QVector<Channel> &channels, const QString &table,
					 QString *error)
{
	QString s = QStringLiteral("{attribute 'qualified_only'}\n// %1: PLC channels from QElectroTech\n"
							   "VAR_GLOBAL\n").arg(iecName(table));
	for (const auto &e : iecChannels(channels)) {
		const Channel &c = *e.second;
		s += QStringLiteral("\t%1 AT %2 : %3;").arg(e.first, iecAddress(c.address), iecType(c.dataType));
		QString comment = c.function.isEmpty() ? c.symbol : c.function;
		comment.replace(QStringLiteral("*)"), QStringLiteral("* )")).replace(QLatin1Char('\n'), QLatin1Char(' '));
		if (!comment.trimmed().isEmpty()) s += QStringLiteral(" (* %1 *)").arg(comment.trimmed());
		s += QLatin1Char('\n');
	}
	s += QStringLiteral("END_VAR\n");
	return saveText(path, s, error);
}

/// PLCopen XML (IEC 61131-10 / TC6 XML v2.01): a project whose resource
/// holds the channels as global variables, for CODESYS, TwinCAT, Beremiz
/// and other tools that import PLCopen
bool writePlcOpen(const QString &path, const QVector<Channel> &channels, const QString &table,
				  QString *error)
{
	const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
	QString s = QStringLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
							   "<project xmlns=\"http://www.plcopen.org/xml/tc6_0201\" "
							   "xmlns:xhtml=\"http://www.w3.org/1999/xhtml\">\n"
							   "  <fileHeader companyName=\"\" productName=\"QElectroTech\" "
							   "productVersion=\"%1\" creationDateTime=\"%2\"/>\n"
							   "  <contentHeader name=\"%3\" modificationDateTime=\"%2\">\n"
							   "    <coordinateInfo>\n"
							   "      <fbd><scaling x=\"1\" y=\"1\"/></fbd>\n"
							   "      <ld><scaling x=\"1\" y=\"1\"/></ld>\n"
							   "      <sfc><scaling x=\"1\" y=\"1\"/></sfc>\n"
							   "    </coordinateInfo>\n"
							   "  </contentHeader>\n"
							   "  <types>\n    <dataTypes/>\n    <pous/>\n  </types>\n"
							   "  <instances>\n    <configurations>\n"
							   "      <configuration name=\"Config\">\n"
							   "        <resource name=\"Resource\">\n"
							   "          <globalVars name=\"%3\">\n")
					.arg(QetVersion::displayedVersionWithFork(), now, xmlEscape(iecName(table)));
	for (const auto &e : iecChannels(channels)) {
		const Channel &c = *e.second;
		s += QStringLiteral("            <variable name=\"%1\" address=\"%2\">\n"
							"              <type><%3/></type>\n")
				 .arg(xmlEscape(e.first), xmlEscape(iecAddress(c.address)), iecType(c.dataType));
		const QString comment = c.function.isEmpty() ? c.symbol : c.function;
		if (!comment.trimmed().isEmpty())
			s += QStringLiteral("              <documentation><xhtml:p>%1</xhtml:p></documentation>\n")
					 .arg(xmlEscape(comment.trimmed()));
		s += QStringLiteral("            </variable>\n");
	}
	s += QStringLiteral("          </globalVars>\n        </resource>\n      </configuration>\n"
						"    </configurations>\n  </instances>\n</project>\n");
	return saveText(path, s, error);
}

bool writeXml(const QString &path, const QVector<Channel> &channels, const QString &table,
			  const QString &culture, QString *error)
{
	int id = 0;
	auto nextId = [&id]() { return QString::number(id++, 16).toUpper(); };
	QString s = QStringLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<Document>\n"
							   "  <Engineering version=\"V21\" />\n");
	s += QStringLiteral("  <SW.Tags.PlcTagTable ID=\"%1\">\n").arg(nextId());
	s += QStringLiteral("    <AttributeList>\n      <Name>%1</Name>\n    </AttributeList>\n"
						"    <ObjectList>\n").arg(xmlEscape(table));
	for (const Channel &c : channels) {
		if (c.address.isEmpty()) continue;
		s += QStringLiteral("      <SW.Tags.PlcTag ID=\"%1\" CompositionName=\"Tags\">\n").arg(nextId());
		s += QStringLiteral("        <AttributeList>\n"
							"          <DataTypeName>%1</DataTypeName>\n"
							"          <ExternalAccessible>true</ExternalAccessible>\n"
							"          <ExternalVisible>true</ExternalVisible>\n"
							"          <ExternalWritable>true</ExternalWritable>\n"
							"          <LogicalAddress>%2</LogicalAddress>\n"
							"          <Name>%3</Name>\n"
							"        </AttributeList>\n")
			 .arg(xmlEscape(c.dataType), xmlEscape(c.address), xmlEscape(c.name));
		s += QStringLiteral("        <ObjectList>\n"
							"          <MultilingualText ID=\"%1\" CompositionName=\"Comment\">\n"
							"            <ObjectList>\n").arg(nextId());
		s += QStringLiteral("              <MultilingualTextItem ID=\"%1\" CompositionName=\"Items\">\n"
							"                <AttributeList>\n"
							"                  <Culture>%2</Culture>\n"
							"                  <Text>%3</Text>\n"
							"                </AttributeList>\n"
							"              </MultilingualTextItem>\n")
			 .arg(nextId(), xmlEscape(culture), xmlEscape(c.function));
		s += QStringLiteral("            </ObjectList>\n          </MultilingualText>\n"
							"        </ObjectList>\n      </SW.Tags.PlcTag>\n");
	}
	s += QStringLiteral("    </ObjectList>\n  </SW.Tags.PlcTagTable>\n</Document>\n");

	QSaveFile file(path);
	if (!file.open(QIODevice::WriteOnly)) {
		if (error) *error = file.errorString();
		return false;
	}
	file.write(s.toUtf8());
	if (!file.commit()) {
		if (error) *error = file.errorString();
		return false;
	}
	return true;
}

} // namespace

int Comparison::count(Status status) const
{
	return int(std::count_if(rows.cbegin(), rows.cend(),
							 [status](const Difference &d) { return d.status == status; }));
}

QString normalizedAddress(const QString &raw)
{
	return format(parse(raw));
}

QVector<Channel> channels(QETProject *project, QStringList *notes)
{
	QVector<Channel> result;
	if (!project) return result;

	// modules in folio order, top to bottom, left to right
	QVector<Element *> modules;
	const QList<Diagram *> diagrams = project->diagrams();
	for (Diagram *diagram : diagrams) {
		QList<Element *> elements = diagram->elements();
		std::sort(elements.begin(), elements.end(), [](Element *a, Element *b) {
			const QPointF pa = a->scenePos(), pb = b->scenePos();
			return qFuzzyCompare(pa.y(), pb.y()) ? pa.x() < pb.x() : pa.y() < pb.y();
		});
		for (Element *e : elements) {
			const ElementData &data = e->elementData();
			if (data.m_type == ElementData::Master && data.m_master_type == ElementData::PLC
				&& !data.plcMasterData().ios.isEmpty())
				modules << e;
		}
	}

	QSet<int> used_i, used_q;
	auto useBytes = [&](const Parts &p) {
		QSet<int> &used = p.area == QLatin1Char('I') ? used_i : used_q;
		for (int i = 0; i < p.length(); ++i) used.insert(p.byte + i);
	};

	struct Pending { int index; int channel; bool input; bool analog; };
	QVector<QVector<Pending>> pending_per_module;

	for (Element *module : modules) {
		const auto ios = module->elementData().plcMasterData().ios;
		// the channel symbols linked onto the rows
		QHash<int, Element *> slaves;
		for (Element *s : module->linkedElements()) {
			const int g = module->groupIndexForElement(s);
			if (g >= 0 && !slaves.contains(g)) slaves.insert(g, s);
		}
		QVector<Pending> pending;
		for (int i = 0; i < ios.size(); ++i) {
			const ElementData::PlcIO &io = ios.at(i);
			Channel c;
			c.master = module;
			c.slave = slaves.value(i, nullptr);
			c.io = i;
			c.module = elementLabel(module);
			c.kind = kindOf(io.type);
			c.rawAddress = io.address.trimmed();
			c.symbol = io.comment.trimmed();
			c.function = oneLine(io.functionText);
			Diagram *d = c.slave ? c.slave->diagram() : module->diagram();
			c.folio = d ? project->folioIndex(d) : -1;
			const bool analog = isAnalog(io.type);
			Parts p = parse(c.rawAddress);
			if (!p.valid()) {
				// "1.0": byte.bit without area
				static const QRegularExpression bit_rx(QStringLiteral("^(\\d{1,5})\\.([0-7])$"));
				const auto m = bit_rx.match(c.rawAddress);
				if (m.hasMatch()) {
					p.area = isInput(io.type) ? QLatin1Char('I') : QLatin1Char('Q');
					p.byte = m.captured(1).toInt();
					p.bit = m.captured(2).toInt();
				}
			}
			if (p.valid()) {
				c.address = format(p);
				c.dataType = dataTypeFor(p, analog);
				useBytes(p);
			} else {
				const int n = relativeChannel(c.rawAddress.isEmpty() ? QString::number(i) : c.rawAddress);
				if (n >= 0) pending << Pending{int(result.size()), n, isInput(io.type), analog};
				else if (notes)
					*notes << tr("%1, row %2: address \"%3\" is neither an I/Q address nor a channel.")
								  .arg(c.module).arg(i + 1).arg(c.rawAddress);
			}
			result << c;
		}
		pending_per_module << pending;
	}

	// channel names: counted from the module's start address or the next free bytes
	for (int m = 0; m < modules.size(); ++m) {
		const QVector<Pending> &pending = pending_per_module.at(m);
		if (pending.isEmpty()) continue;
		const DiagramContext info = modules.at(m)->elementInformations();
		for (const bool input : {true, false}) {
			int digital = -1, analog = -1;
			for (const Pending &p : pending) {
				if (p.input != input) continue;
				int &top = p.analog ? analog : digital;
				top = qMax(top, p.channel);
			}
			if (digital < 0 && analog < 0) continue;
			const int digital_bytes = digital < 0 ? 0 : digital / 8 + 1;
			const int length = digital_bytes + (analog < 0 ? 0 : 2 * (analog + 1));
			QSet<int> &used = input ? used_i : used_q;
			bool ok = false;
			const int given = info.value(input ? QStringLiteral("plc_start_address")
											   : QStringLiteral("plc_start_address_2"))
								  .toString().trimmed().toInt(&ok);
			const int start = ok && given >= 0 ? given : freeRun(used, length);
			for (int i = 0; i < length; ++i) used.insert(start + i);
			for (const Pending &p : pending) {
				if (p.input != input) continue;
				Parts parts;
				parts.area = input ? QLatin1Char('I') : QLatin1Char('Q');
				if (p.analog) {
					parts.size = QStringLiteral("W");
					parts.byte = start + digital_bytes + 2 * p.channel;
				} else {
					parts.byte = start + p.channel / 8;
					parts.bit = p.channel % 8;
				}
				Channel &c = result[p.index];
				c.address = format(parts);
				c.dataType = dataTypeFor(parts, p.analog);
				c.autoAddress = true;
			}
		}
	}

	// tag names: unique in the CPU, without regard to case
	QSet<QString> names;
	QHash<QString, int> addresses;
	for (Channel &c : result) {
		if (c.address.isEmpty()) continue;
		QString name = c.symbol;
		name.remove(QLatin1Char('"'));
		name = name.trimmed();
		if (name.isEmpty()) {
			const QString channel = c.autoAddress || c.rawAddress.isEmpty()
				? (c.rawAddress.isEmpty() ? c.kind + QString::number(c.io) : c.rawAddress)
				: c.address;
			name = nameToken(c.module) + QLatin1Char('_') + nameToken(channel);
		}
		QString unique = name;
		for (int n = 2; names.contains(unique.toLower()); ++n)
			unique = name + QLatin1Char('_') + QString::number(n);
		names.insert(unique.toLower());
		c.name = unique;
		if (unique != name && notes)
			*notes << tr("%1, row %2: the name \"%3\" is taken, written as \"%4\".")
						  .arg(c.module).arg(c.io + 1).arg(name, unique);
		if (addresses.contains(c.address) && notes)
			*notes << tr("%1, row %2: address %3 is used twice.").arg(c.module).arg(c.io + 1).arg(c.address);
		addresses.insert(c.address, 1);
	}
	return result;
}

bool write(const QString &path, const QVector<Channel> &channels, const QString &table,
		   const QString &culture, QString *error)
{
	const QString name = table.isEmpty() ? QStringLiteral("QET_EA") : table;
	const QString suffix = QFileInfo(path).suffix().toLower();
	if (suffix == QLatin1String("gvl") || suffix == QLatin1String("st") || suffix == QLatin1String("exp"))
		return writeCodesysGvl(path, channels, name, error);
	if (path.endsWith(QLatin1String(".plcopen.xml"), Qt::CaseInsensitive))
		return writePlcOpen(path, channels, name, error);
	if (QFileInfo(path).suffix().compare(QLatin1String("xml"), Qt::CaseInsensitive) == 0)
		return writeXml(path, channels, name, culture.isEmpty() ? QStringLiteral("de-DE") : culture, error);
	return writeXlsx(path, channels, name, error);
}

QVector<Tag> read(const QString &path, QString *error)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		if (error) *error = file.errorString();
		return {};
	}
	const QByteArray head = file.read(4);
	file.close();
	if (head.startsWith("PK")) return readXlsx(path, error);
	return readXml(path, error);
}

Comparison compare(const QVector<Channel> &channels, const QVector<Tag> &tags)
{
	Comparison cmp;
	cmp.channels = channels;
	cmp.tags = tags;

	QHash<QString, int> by_address, by_name;
	for (int i = 0; i < tags.size(); ++i) {
		const QString a = normalizedAddress(tags.at(i).address);
		if (a.isEmpty()) { ++cmp.otherTags; continue; }
		if (!by_address.contains(a)) by_address.insert(a, i);
		by_name.insert(tags.at(i).name.toLower(), i);
	}
	QSet<int> matched;
	for (int c = 0; c < channels.size(); ++c) {
		const Channel &ch = channels.at(c);
		if (ch.address.isEmpty()) continue;
		Difference d;
		d.channel = c;
		const int t = by_address.value(ch.address, -1);
		if (t >= 0) {
			d.tag = t;
			matched.insert(t);
			const bool name_same = tags.at(t).name == ch.name;
			const bool comment_same = oneLine(tags.at(t).comment) == ch.function;
			d.status = name_same && comment_same ? Status::Same
					 : !name_same && !comment_same ? Status::BothDiffer
					 : name_same ? Status::CommentDiffers : Status::NameDiffers;
		} else {
			const int n = by_name.value(ch.name.toLower(), -1);
			if (n >= 0 && !matched.contains(n)) {
				d.tag = n;
				matched.insert(n);
				d.status = Status::AddressDiffers;
			} else {
				d.status = Status::MissingInTia;
			}
		}
		cmp.rows << d;
	}
	for (int t = 0; t < tags.size(); ++t) {
		if (matched.contains(t) || normalizedAddress(tags.at(t).address).isEmpty()) continue;
		Difference d;
		d.status = Status::OnlyInTia;
		d.tag = t;
		cmp.rows << d;
	}
	return cmp;
}

QString statusText(Status status)
{
	switch (status) {
		case Status::Same:           return tr("same");
		case Status::NameDiffers:    return tr("name differs");
		case Status::CommentDiffers: return tr("comment differs");
		case Status::BothDiffer:     return tr("name and comment differ");
		case Status::AddressDiffers: return tr("address differs");
		case Status::MissingInTia:   return tr("missing in TIA");
		case Status::OnlyInTia:      return tr("only in TIA");
	}
	return {};
}

QString reportCsv(const Comparison &cmp)
{
	auto field = [](QString s) {
		if (s.contains(QLatin1Char(';')) || s.contains(QLatin1Char('"')) || s.contains(QLatin1Char('\n'))) {
			s.replace(QLatin1Char('"'), QLatin1String("\"\""));
			s = QLatin1Char('"') + s + QLatin1Char('"');
		}
		return s;
	};
	static const char *codes[] = {"same", "name", "comment", "name+comment", "address", "missing", "only_tia"};
	QString csv(QStringLiteral("status;address_plan;address_tia;name_plan;name_tia;comment_plan;comment_tia;module;row;folio\n"));
	for (const Difference &d : cmp.rows) {
		const Channel *c = d.channel >= 0 ? &cmp.channels.at(d.channel) : nullptr;
		const Tag *t = d.tag >= 0 ? &cmp.tags.at(d.tag) : nullptr;
		QStringList row;
		row << QLatin1String(codes[int(d.status)])
			<< (c ? c->address : QString())
			<< (t ? normalizedAddress(t->address) : QString())
			<< (c ? c->name : QString())
			<< (t ? t->name : QString())
			<< (c ? c->function : QString())
			<< (t ? oneLine(t->comment) : QString())
			<< (c ? c->module : QString())
			<< (c ? QString::number(c->io + 1) : QString())
			<< (c && c->folio >= 0 ? QString::number(c->folio + 1) : QString());
		for (QString &f : row) f = field(f);
		csv += row.join(QLatin1Char(';')) + QLatin1Char('\n');
	}
	return csv;
}

int apply(const Comparison &cmp, QUndoCommand *undo_parent)
{
	QHash<Element *, ElementData> changed;
	QVector<Element *> order;
	int rows = 0;
	for (const Difference &d : cmp.rows) {
		if (d.channel < 0 || d.tag < 0 || d.status == Status::Same
			|| d.status == Status::AddressDiffers)
			continue;
		const Channel &c = cmp.channels.at(d.channel);
		const Tag &t = cmp.tags.at(d.tag);
		Element *master = c.master.data();
		if (!master) continue;
		if (!changed.contains(master)) {
			changed.insert(master, master->elementData());
			order << master;
		}
		ElementData &data = changed[master];
		ElementData::PlcMasterData plc = data.plcMasterData();
		if (c.io < 0 || c.io >= plc.ios.size()) continue;
		ElementData::PlcIO &io = plc.ios[c.io];
		bool row_changed = false;
		if (io.comment != t.name) { io.comment = t.name; row_changed = true; }
		const QString comment = oneLine(t.comment);
		if (!comment.isEmpty() && oneLine(io.functionText) != comment) {
			io.functionText = comment;
			row_changed = true;
		}
		if (row_changed) {
			++rows;
			data.setPlcMasterData(plc);
		}
	}
	for (Element *master : order) {
		if (changed.value(master).plcMasterData() == master->elementData().plcMasterData()) continue;
		if (undo_parent) new ChangeElementDataCommand(master, changed.value(master), undo_parent);
		else master->setElementData(changed.value(master));
	}
	return rows;
}

} // namespace TiaPlcTags
