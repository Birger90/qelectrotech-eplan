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
#include "markingexport.h"

#include "../dataBase/projectdatabase.h"
#include "../qetproject.h"

#include <QCoreApplication>
#include <QSaveFile>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

namespace {

	/// @p value without the sign it may have been typed with
QString unsigned_(const QString &value, QChar sign)
{
	return value.startsWith(sign) ? value.mid(1) : value;
}

	/// "-label", one sign whether or not it was typed with one
QString shortDesignation(const QString &label)
{
	if (label.isEmpty()) {
		return QString();
	}
	return label.startsWith(QLatin1Char('-')) ? label : QLatin1Char('-') + label;
}

	/// A wire end as the terminal lists write it: device:terminal
QString wireEnd(const QString &device, const QString &terminal)
{
	if (device.isEmpty()) {
		return terminal;
	}
	return terminal.isEmpty() ? device : device + QLatin1Char(':') + terminal;
}

bool folioWanted(const MarkingExport::Options &o, int folio_number)
{
	if (o.folio_from > 0 && folio_number < o.folio_from) {
		return false;
	}
	if (o.folio_to > 0 && folio_number > o.folio_to) {
		return false;
	}
	return true;
}

bool placeWanted(const MarkingExport::Options &o, const QString &plant,
				 const QString &location)
{
	const QString want_plant = unsigned_(o.plant.trimmed(), QLatin1Char('='));
	const QString want_location = unsigned_(o.location.trimmed(), QLatin1Char('+'));
	if (!want_plant.isEmpty()
			&& unsigned_(plant, QLatin1Char('=')).compare(want_plant, Qt::CaseInsensitive) != 0) {
		return false;
	}
	if (!want_location.isEmpty()
			&& unsigned_(location, QLatin1Char('+')).compare(want_location, Qt::CaseInsensitive) != 0) {
		return false;
	}
	return true;
}

	/// The terminal's own number: "X1:3" on strip X1 is 3
QString terminalNumber(const QString &label, const QString &strip)
{
	if (!strip.isEmpty()) {
		const QString bare = unsigned_(strip, QLatin1Char('-'));
		for (const QString &prefix : {bare + QLatin1Char(':'),
									  QLatin1Char('-') + bare + QLatin1Char(':'),
									  bare + QLatin1Char('.')}) {
			if (label.startsWith(prefix, Qt::CaseInsensitive) && label.size() > prefix.size()) {
				return label.mid(prefix.size());
			}
		}
	}
	return unsigned_(label, QLatin1Char('-'));
}

bool run(QSqlQuery &query, QString *error)
{
	if (query.exec()) {
		return true;
	}
	if (error) {
		*error = query.lastError().text();
	}
	return false;
}

	/// Every field quoted, quotes doubled, ';' between them
QByteArray record(const QStringList &values, const QByteArray &eol, bool latin1)
{
	QStringList quoted;
	for (QString value : values) {
		value.replace(QLatin1Char('"'), QStringLiteral("\"\""));
		value.replace(QLatin1Char('\r'), QLatin1Char(' '));
		value.replace(QLatin1Char('\n'), QLatin1Char(' '));
		quoted << QLatin1Char('"') + value + QLatin1Char('"');
	}
	const QString line = quoted.join(QLatin1Char(';'));
	return (latin1 ? line.toLatin1() : line.toUtf8()) + eol;
}

QString kindKey(const MarkingExport::Item &item)
{
	switch (item.kind) {
		case MarkingExport::Wire: return QStringLiteral("Draht");
		case MarkingExport::Terminal:
			return item.strip_marker ? QStringLiteral("Klemmleiste")
									 : QStringLiteral("Klemme");
		case MarkingExport::Cable: return QStringLiteral("Kabel");
		default: return QStringLiteral("Geraet");
	}
}

} // namespace

QList<MarkingExport::Item> MarkingExport::collect(QETProject *project,
												  const Options &o,
												  QString *error)
{
	QList<Item> items;
	if (!project || !project->dataBase()) {
		if (error) *error = QCoreApplication::translate("MarkingExport", "No project");
		return items;
	}
	project->dataBase()->updateDB();
	auto *db = project->dataBase();

		//Wires: one marker per end, its number, the other end as target
	if (o.kinds & Wire) {
		QSqlQuery q = db->newQuery(QStringLiteral(
			"SELECT w.wire_number, w.folio_number, w.folio, "
			"w.from_structure_id, w.from_device, w.from_terminal, "
			"w.to_structure_id, w.to_device, w.to_terminal, "
			"COALESCE(f.plant, ''), COALESCE(f.location, '') "
			"FROM qet_wires w LEFT JOIN qet_folios f ON f.folio_number = w.folio_number "
			"WHERE COALESCE(TRIM(w.wire_number), '') <> '' "
			"ORDER BY w.folio_number, w.wire_number, w.from_structure_id"));
		if (!run(q, error)) return {};
		while (q.next()) {
			const int folio_number = q.value(1).toInt();
			const QString plant = q.value(9).toString(), location = q.value(10).toString();
			if (!folioWanted(o, folio_number) || !placeWanted(o, plant, location)) continue;
			const QString from = wireEnd(o.full_designation ? q.value(3).toString()
															 : shortDesignation(q.value(4).toString()),
										 q.value(5).toString());
			const QString to = wireEnd(o.full_designation ? q.value(6).toString()
														   : shortDesignation(q.value(7).toString()),
									   q.value(8).toString());
			Item item;
			item.kind = Wire;
			item.text = q.value(0).toString().trimmed();
			item.folio_number = folio_number;
			item.folio = q.value(2).toString();
			item.plant = plant;
			item.location = location;
			item.group = from;
			item.target = to;
			items << item;
			if (o.both_ends) {
				item.group = to;
				item.target = from;
				items << item;
			}
		}
	}

		//Terminals: each one by its strip, and the strip's own marker
	if (o.kinds & Terminal) {
		QSqlQuery q = db->newQuery(QStringLiteral(
			"SELECT t.strip_id, t.plant, t.location, t.strip, t.position, t.label, "
			"t.folio_number, t.folio, COALESCE(d.manufacturer, ''), COALESCE(d.article, ''), "
			"t.strip_uuid "
			"FROM qet_terminals t LEFT JOIN qet_devices d ON d.device_uuid = t.element_uuid "
			"ORDER BY t.strip_id, t.strip_uuid, t.position, t.level"));
		if (!run(q, error)) return {};
		QString strip_uuid;
		QSet<QString> seen;
		while (q.next()) {
			const QString plant = q.value(1).toString(), location = q.value(2).toString();
			const int folio_number = q.value(6).toInt();
			const QString label = q.value(5).toString();
			if (label.isEmpty() || !placeWanted(o, plant, location)
					|| !folioWanted(o, folio_number)) {
				continue;
			}
			const QString strip = q.value(3).toString();
			const QString group = o.full_designation ? q.value(0).toString()
													 : shortDesignation(strip);
			Item item;
			item.kind = Terminal;
			item.group = group;
			item.plant = plant;
			item.location = location;
			item.folio_number = folio_number;
			item.folio = q.value(7).toString();
			if (q.value(10).toString() != strip_uuid) {
				strip_uuid = q.value(10).toString();
				Item strip_item = item;
				strip_item.text = group;
				strip_item.strip_marker = true;
				items << strip_item;
			}
				//The levels of one terminal named alike are one marker
			item.text = terminalNumber(label, strip);
			const QString key = strip_uuid + QLatin1Char('|') + q.value(4).toString()
								+ QLatin1Char('|') + item.text;
			if (seen.contains(key)) continue;
			seen.insert(key);
			item.manufacturer = q.value(8).toString();
			item.article = q.value(9).toString();
			items << item;
		}
	}

		//Devices: one per reference designation, terminals left out
	if (o.kinds & Device) {
		QSqlQuery q = db->newQuery(QStringLiteral(
			"SELECT l.structure_id, l.plant, l.location, l.label, l.folio_number, l.folio, "
			"COALESCE(l.articles, ''), COALESCE(NULLIF(l.description, ''), l.designation, ''), "
			"COALESCE(d.manufacturer, '') "
			"FROM qet_device_list l LEFT JOIN qet_devices d ON d.device_uuid = l.head_uuid "
			"WHERE COALESCE(d.kind, '') <> 'terminal' "
			"ORDER BY l.folio_number, l.position, l.structure_id"));
		if (!run(q, error)) return {};
		while (q.next()) {
			const QString plant = q.value(1).toString(), location = q.value(2).toString();
			const int folio_number = q.value(4).toInt();
			if (!folioWanted(o, folio_number) || !placeWanted(o, plant, location)) continue;
			Item item;
			item.kind = Device;
			item.text = o.full_designation ? q.value(0).toString()
										   : shortDesignation(q.value(3).toString());
			if (item.text.isEmpty()) continue;
			item.plant = plant;
			item.location = location;
			item.folio_number = folio_number;
			item.folio = q.value(5).toString();
			item.article = q.value(6).toString();
			item.target = q.value(7).toString();
			item.manufacturer = q.value(8).toString();
			items << item;
		}
	}

		//Cables: a marker at each end, its type as target
	if (o.kinds & Cable) {
		QSqlQuery q = db->newQuery(QStringLiteral(
			"SELECT structure_id, plant, location, designation, COALESCE(type, ''), "
			"folio_number, folio FROM qet_cables "
			"WHERE COALESCE(designation, '') <> '' ORDER BY folio_number, structure_id"));
		if (!run(q, error)) return {};
		while (q.next()) {
			const QString plant = q.value(1).toString(), location = q.value(2).toString();
			const int folio_number = q.value(5).toInt();
			if (!folioWanted(o, folio_number) || !placeWanted(o, plant, location)) continue;
			Item item;
			item.kind = Cable;
			item.text = o.full_designation ? q.value(0).toString()
										   : shortDesignation(q.value(3).toString());
			item.target = q.value(4).toString();
			item.plant = plant;
			item.location = location;
			item.folio_number = folio_number;
			item.folio = q.value(6).toString();
			item.count = o.both_ends ? 2 : 1;
			items << item;
		}
	}
		//A folio whose title block names it nothing goes by its number
	for (Item &item : items) {
		if (item.folio.isEmpty() && item.folio_number > 0) {
			item.folio = QString::number(item.folio_number);
		}
	}
	return items;
}

QString MarkingExport::materialReference(const Item &item)
{
		//A terminal's marker fits its terminal: its part number is what
		//the import assigns the marker material from (Phoenix Contact:
		//the terminal part numbers column, see docs/beschriftung-export.md)
	if (item.kind == Terminal && !item.strip_marker && !item.article.isEmpty()) {
		return item.article;
	}
	return kindKey(item);
}

QByteArray MarkingExport::write(const QList<Item> &items, Format format,
								int *markers, const QHash<QString, QString> &materials)
{
	QByteArray out;
	int count = 0;
	const QByteArray crlf("\r\n");
	switch (format) {
		case Neutral: {
			out = QByteArray("\xEF\xBB\xBF");
			out += record({QStringLiteral("Art"), QStringLiteral("Beschriftung"),
						   QStringLiteral("Ziel"), QStringLiteral("Gruppe"),
						   QStringLiteral("Anlage"), QStringLiteral("Ort"),
						   QStringLiteral("Blatt"), QStringLiteral("Blattnummer"),
						   QStringLiteral("Hersteller"), QStringLiteral("Artikel"),
						   QStringLiteral("Referenz"), QStringLiteral("Anzahl")},
						  crlf, false);
			for (const Item &i : items) {
				out += record({kindKey(i), i.text, i.target, i.group, i.plant, i.location,
							   i.folio, QString::number(i.folio_number), i.manufacturer,
							   i.article, materialReference(i), QString::number(i.count)},
							  crlf, false);
				count += i.count;
			}
			break;
		}
		case PhoenixContact: {
				//The marking text first: the Excel import of PROJECT complete
				//reads the texts from one column. Material is the column the
				//"Material mapping" assigns the marker from. No quantity column
				//is documented, so a marker wanted twice is a line twice.
			out = QByteArray("\xEF\xBB\xBF");
			out += record({QStringLiteral("Beschriftung"), QStringLiteral("Material"),
						   QStringLiteral("Art"), QStringLiteral("Gruppe"),
						   QStringLiteral("Ziel"), QStringLiteral("Blatt"),
						   QStringLiteral("Anlage"), QStringLiteral("Ort"),
						   QStringLiteral("Hersteller"), QStringLiteral("Artikel")},
						  crlf, false);
			for (const Item &i : items) {
				for (int n = 0; n < i.count; ++n) {
					out += record({i.text, materialReference(i), kindKey(i), i.group,
								   i.target, i.folio, i.plant, i.location,
								   i.manufacturer, i.article}, crlf, false);
					++count;
				}
			}
			break;
		}
		case Weidmueller: {
				//M-Print PRO, import "VK Text Format": every line names its
				//marker material in a reference column (a material number, or
				//an alias looked up in a reference file *.mpr) and an empty one
				//takes the line above's, so it is never empty here; a column
				//with the quantity repeats a line. Windows text (ANSI): the
				//manual offers "UTF-8 Kompatibilität" only as the fix for
				//special characters shown wrong.
			out += record({QStringLiteral("Referenz"), QStringLiteral("Anzahl"),
						   QStringLiteral("Beschriftung"), QStringLiteral("Ziel"),
						   QStringLiteral("Gruppe"), QStringLiteral("Art"),
						   QStringLiteral("Blatt"), QStringLiteral("Anlage"),
						   QStringLiteral("Ort"), QStringLiteral("Artikel")},
						  crlf, true);
			for (const Item &i : items) {
					//A material number of the printer's own when one is set:
					//"Die Referenzspalte enthält Materialnummern"
				const QString reference = materials.value(kindKey(i)).isEmpty()
						? materialReference(i) : materials.value(kindKey(i));
				out += record({reference, QString::number(i.count), i.text,
							   i.target, i.group, kindKey(i), i.folio, i.plant,
							   i.location, i.article}, crlf, true);
				count += i.count;
			}
			break;
		}
		case Wago: {
				//Smart Script reads Excel, CSV, TXT and DAT from a CAE system
				//and maps the columns in a template (headings = group names):
				//no material column is documented, the marker type is chosen
				//in Smart Script. One line per marker.
			out = QByteArray("\xEF\xBB\xBF");
			out += record({QStringLiteral("Beschriftung"), QStringLiteral("Art"),
						   QStringLiteral("Gruppe"), QStringLiteral("Ziel"),
						   QStringLiteral("Blatt"), QStringLiteral("Anlage"),
						   QStringLiteral("Ort"), QStringLiteral("Artikel")},
						  crlf, false);
			for (const Item &i : items) {
				for (int n = 0; n < i.count; ++n) {
					out += record({i.text, kindKey(i), i.group, i.target, i.folio,
								   i.plant, i.location, i.article}, crlf, false);
					++count;
				}
			}
			break;
		}
	}
	if (markers) *markers = count;
	return out;
}

bool MarkingExport::exportFile(QETProject *project, const QString &path,
							   const Options &options, QString *error,
							   int *markers)
{
	QString err;
	const QList<Item> items = collect(project, options, &err);
	if (!err.isEmpty()) {
		if (error) *error = err;
		return false;
	}
	int count = 0;
	const QByteArray bytes = write(items, options.format, &count, options.materials);
	if (markers) *markers = count;
	QSaveFile file(path);
	if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size()
			|| !file.commit()) {
		if (error) *error = file.errorString();
		return false;
	}
	return true;
}

QString MarkingExport::formatKey(Format format)
{
	switch (format) {
		case PhoenixContact: return QStringLiteral("phoenix");
		case Weidmueller: return QStringLiteral("weidmueller");
		case Wago: return QStringLiteral("wago");
		default: return QStringLiteral("neutral");
	}
}

bool MarkingExport::formatFromKey(const QString &key, Format *format)
{
	const QString k = key.trimmed().toLower();
	Format f;
	if (k == QLatin1String("neutral") || k == QLatin1String("csv")) f = Neutral;
	else if (k.startsWith(QLatin1String("phoenix")) || k == QLatin1String("pxc")
			 || k == QLatin1String("marking-system")) f = PhoenixContact;
	else if (k.startsWith(QLatin1String("weidm")) || k == QLatin1String("m-print")
			 || k == QLatin1String("mprint")) f = Weidmueller;
	else if (k.startsWith(QLatin1String("wago")) || k == QLatin1String("smartscript")) f = Wago;
	else return false;
	if (format) *format = f;
	return true;
}

QString MarkingExport::formatTitle(Format format)
{
	switch (format) {
		case PhoenixContact: return QCoreApplication::translate("MarkingExport", "Phoenix Contact MARKING system / PROJECT complete");
		case Weidmueller: return QCoreApplication::translate("MarkingExport", "Weidmüller M-Print PRO");
		case Wago: return QCoreApplication::translate("MarkingExport", "WAGO Smart Script");
		default: return QCoreApplication::translate("MarkingExport", "Neutral CSV (all columns)");
	}
}

QString MarkingExport::fileFilter(Format format)
{
	Q_UNUSED(format)
	return QCoreApplication::translate("MarkingExport", "CSV files (*.csv)");
}

bool MarkingExport::kindsFromKeys(const QString &keys, int *kinds)
{
	int k = 0;
	for (QString key : keys.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
		key = key.trimmed().toLower();
		if (key == QLatin1String("all") || key == QLatin1String("alle")) k |= AllKinds;
		else if (key.startsWith(QLatin1String("wire")) || key.startsWith(QLatin1String("draht"))
				 || key.startsWith(QString::fromUtf8("dräht"))) k |= Wire;
		else if (key.startsWith(QLatin1String("terminal")) || key.startsWith(QLatin1String("klemm"))) k |= Terminal;
		else if (key.startsWith(QLatin1String("device")) || key.startsWith(QLatin1String("ger"))) k |= Device;
		else if (key.startsWith(QLatin1String("cable")) || key.startsWith(QLatin1String("kabel"))) k |= Cable;
		else return false;
	}
	if (!k) return false;
	if (kinds) *kinds = k;
	return true;
}

QString MarkingExport::kindTitle(Kind kind)
{
	switch (kind) {
		case Wire: return QCoreApplication::translate("MarkingExport", "Wire markers");
		case Terminal: return QCoreApplication::translate("MarkingExport", "Terminal markers");
		case Cable: return QCoreApplication::translate("MarkingExport", "Cable markers");
		default: return QCoreApplication::translate("MarkingExport", "Device markers");
	}
}
