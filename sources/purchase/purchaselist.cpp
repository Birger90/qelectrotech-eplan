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
#include "purchaselist.h"

#include "../bomexport.h"
#include "../container/qetzip.h"
#include "../dataBase/projectdatabase.h"
#include "../import/portals/manufacturerportals.h"
#include "../materiallist/materiallist.h"
#include "../qetproject.h"

#include <QCollator>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <algorithm>
#include <cmath>

namespace {

	/// a number written out without trailing zeros, '.' or ',' as decimal
	/// sign
QString decimal(double value, QChar point)
{
	QString text = QString::number(value, 'f', 4);
	if (text.contains(QLatin1Char('.'))) {
		while (text.endsWith(QLatin1Char('0'))) text.chop(1);
		if (text.endsWith(QLatin1Char('.'))) text.chop(1);
	}
	if (text == QLatin1String("-0")) text = QStringLiteral("0");
	if (point != QLatin1Char('.')) text.replace(QLatin1Char('.'), point);
	return text;
}

	/// an amount of money: two decimals at least ("42,10"), more only
	/// when the price has them ("0,0425")
QString money(double value, QChar point)
{
	QString text = decimal(value, point);
	const int at = int(text.indexOf(point));
	if (at < 0) return text + point + QStringLiteral("00");
	while (text.size() - at - 1 < 2) text += QLatin1Char('0');
	return text;
}

	/// a heading of the price list or the cache, folded so that
	/// "Bestell-Nr.", "bestellnummer" and "Order number" compare
QString folded(QString header)
{
	header = header.toLower();
	header.remove(QRegularExpression(QStringLiteral("[^a-z0-9\u00e4\u00f6\u00fc\u00df]")));
	return header;
}

	/// the field of ArticleData a heading stands for
QString columnFor(const QString &header)
{
	static const QHash<QString, QString> aliases {
		{QStringLiteral("bestellnummer"), QStringLiteral("part")},
		{QStringLiteral("bestellnr"), QStringLiteral("part")},
		{QStringLiteral("artikelnummer"), QStringLiteral("part")},
		{QStringLiteral("artikelnr"), QStringLiteral("part")},
		{QStringLiteral("ordernumber"), QStringLiteral("part")},
		{QStringLiteral("partnumber"), QStringLiteral("part")},
		{QStringLiteral("manufacturerreference"), QStringLiteral("part")},
		{QStringLiteral("hersteller"), QStringLiteral("manufacturer")},
		{QStringLiteral("manufacturer"), QStringLiteral("manufacturer")},
		{QStringLiteral("preis"), QStringLiteral("price")},
		{QStringLiteral("listenpreis"), QStringLiteral("price")},
		{QStringLiteral("einkaufspreis"), QStringLiteral("price")},
		{QStringLiteral("ekpreis"), QStringLiteral("price")},
		{QStringLiteral("price"), QStringLiteral("price")},
		{QStringLiteral("unitprice"), QStringLiteral("price")},
		{QStringLiteral("preiseinheit"), QStringLiteral("price_unit")},
		{QStringLiteral("priceunit"), QStringLiteral("price_unit")},
		{QStringLiteral("w\u00e4hrung"), QStringLiteral("currency")},
		{QStringLiteral("waehrung"), QStringLiteral("currency")},
		{QStringLiteral("currency"), QStringLiteral("currency")},
		{QStringLiteral("lieferzeit"), QStringLiteral("delivery")},
		{QStringLiteral("deliverytime"), QStringLiteral("delivery")},
		{QStringLiteral("leadtime"), QStringLiteral("delivery")},
		{QStringLiteral("lieferant"), QStringLiteral("supplier")},
		{QStringLiteral("supplier"), QStringLiteral("supplier")},
		{QStringLiteral("vpe"), QStringLiteral("packaging")},
		{QStringLiteral("verpackungseinheit"), QStringLiteral("packaging")},
		{QStringLiteral("packagingunit"), QStringLiteral("packaging")},
		{QStringLiteral("ean"), QStringLiteral("gtin")},
		{QStringLiteral("gtin"), QStringLiteral("gtin")},
		{QStringLiteral("eangtin"), QStringLiteral("gtin")},
		{QStringLiteral("eanart"), QStringLiteral("gtin_type")},
		{QStringLiteral("einheit"), QStringLiteral("unit")},
		{QStringLiteral("unit"), QStringLiteral("unit")},
		{QStringLiteral("lebenszyklus"), QStringLiteral("lifecycle")},
		{QStringLiteral("lifecycle"), QStringLiteral("lifecycle")},
		{QStringLiteral("quelle"), QStringLiteral("source")},
		{QStringLiteral("source"), QStringLiteral("source")},
		{QStringLiteral("stand"), QStringLiteral("date")},
		{QStringLiteral("date"), QStringLiteral("date")},
	};
	return aliases.value(folded(header));
}

QString xmlEscaped(QString text)
{
	text.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
	text.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
	text.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
	text.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
		//Control characters other than tab and line breaks are not
		//allowed in XML 1.0
	static const QRegularExpression control(QStringLiteral("[\\x{0}-\\x{8}\\x{B}\\x{C}\\x{E}-\\x{1F}]"));
	text.remove(control);
	return text;
}

QString columnLetter(int column)	// 0 -> A
{
	QString letters;
	++column;
	while (column > 0) {
		const int rest = (column - 1) % 26;
		letters.prepend(QChar('A' + rest));
		column = (column - 1) / 26;
	}
	return letters;
}

	/// the columns of the purchasing list, in their order
enum Column {
	ColPos, ColSupplier, ColManufacturer, ColPart, ColName, ColRefs,
	ColQty, ColUnit, ColPackaging, ColOrderQty, ColPrice, ColPriceUnit,
	ColCurrency, ColTotal, ColDelivery, ColGtin, ColLifecycle, ColSource,
	ColOpen, ColCount
};

	/// one line of the purchasing list as texts, see PurchaseList::toCsv()
QStringList csvLine(int position, const PurchaseList::Item &item)
{
	const QChar comma(QLatin1Char(','));
	QStringList line;
	line.reserve(ColCount);
	line << QString::number(position)
		 << item.supplier
		 << item.manufacturer
		 << item.part_number
		 << item.name
		 << item.references.join(QStringLiteral(", "))
		 << decimal(item.quantity, comma)
		 << item.unit
		 << (item.packaging > 0 ? decimal(item.packaging, comma) : QString())
		 << decimal(item.orderQuantity(), comma)
		 << (item.price >= 0 ? money(item.price, comma) : QString())
		 << (item.price >= 0 ? decimal(item.price_unit, comma) : QString())
		 << item.currency
		 << (item.total() >= 0 ? money(item.total(), comma) : QString())
		 << item.delivery_time
		 << item.gtin
		 << item.lifecycle
		 << item.price_source
		 << (item.open ? PurchaseList::tr("yes", "article open") : QString());
	return line;
}

	/// a cell starting with = + @ would be read as a formula by a
	/// spreadsheet opening the CSV; an apostrophe in front keeps it text
	/// (a device tag "+S1-K1" would otherwise become a formula)
QStringList defused(QStringList line)
{
	for (QString &cell : line) {
		if (cell.size() > 1 && QStringLiteral("=+@").contains(cell.at(0)))
			cell.prepend(QLatin1Char('\''));
	}
	return line;
}

	/// the order of the groups: suppliers alphabetically, open ones last
bool lessItem(const PurchaseList::Item &a, const PurchaseList::Item &b,
			  const QCollator &collator)
{
	if (a.open != b.open) return !a.open;
	const int group = collator.compare(a.groupName(), b.groupName());
	if (group != 0) return group < 0;
	const int part = collator.compare(a.part_number, b.part_number);
	if (part != 0) return part < 0;
	return collator.compare(a.references.join(QLatin1Char(',')),
							b.references.join(QLatin1Char(','))) < 0;
}

QCollator naturalCollator()
{
	QCollator collator;
	collator.setNumericMode(true);
	collator.setCaseSensitivity(Qt::CaseInsensitive);
	return collator;
}

} // namespace

double PurchaseList::Item::orderQuantity() const
{
	if (packaging <= 0) return quantity;
	return std::ceil(quantity / packaging - 1e-9) * packaging;
}

double PurchaseList::Item::total() const
{
	if (price < 0) return -1;
	const double per = price_unit > 0 ? price_unit : 1;
	return orderQuantity() * price / per;
}

QString PurchaseList::Item::groupName() const
{
	if (open) return tr("Article open");
	if (!supplier.trimmed().isEmpty()) return supplier.trimmed();
	if (!manufacturer.trimmed().isEmpty()) return manufacturer.trimmed();
	return tr("No supplier");
}

QJsonObject PurchaseList::Summary::toJson() const
{
	QJsonObject o;
	o.insert(QStringLiteral("articles"), articles);
	o.insert(QStringLiteral("open"), open);
	o.insert(QStringLiteral("priced"), priced);
	o.insert(QStringLiteral("with_gtin"), with_gtin);
	o.insert(QStringLiteral("total"), std::round(total * 10000) / 10000);
	o.insert(QStringLiteral("currencies"), QJsonArray::fromStringList(currencies));
	return o;
}

QString PurchaseList::purchaseDir()
{
	const QString env = qEnvironmentVariable("QET_PURCHASE_DIR");
	if (!env.isEmpty()) return QDir::fromNativeSeparators(env);
	const QString data = ManufacturerPortals::dataDir();
	if (data.isEmpty()) return QString();
	return QDir(data).filePath(QStringLiteral("einkauf"));
}

QString PurchaseList::priceListPath()
{
	const QString dir = purchaseDir();
	return dir.isEmpty() ? QString() : QDir(dir).filePath(QStringLiteral("preisliste.csv"));
}

QString PurchaseList::cachePath()
{
	const QString dir = purchaseDir();
	return dir.isEmpty() ? QString() : QDir(dir).filePath(QStringLiteral("artikeldaten.csv"));
}

bool PurchaseList::ensurePriceList(QString *error)
{
	const QString path = priceListPath();
	if (path.isEmpty()) {
		if (error) *error = tr("No company data folder is set up.");
		return false;
	}
	if (QFileInfo::exists(path)) return true;
	if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
		if (error) *error = tr("Cannot create the folder %1.")
								.arg(QDir::toNativeSeparators(QFileInfo(path).absolutePath()));
		return false;
	}
		//The four columns the company fills in, then the optional ones
	QByteArray header("\xEF\xBB\xBF");
	header += QStringLiteral("Bestellnummer;Preis;Lieferzeit;Lieferant;Preiseinheit;VPE;EAN;Hersteller;W\u00e4hrung\n").toUtf8();
	return BomExport::writeCsv(path, header, error);
}

QString PurchaseList::key(const QString &part_number)
{
	QString k = part_number.trimmed().toUpper();
	static const QRegularExpression spaces(QStringLiteral("\\s+"));
	k.remove(spaces);
		//"SIE.3RT2015-1BB41" is the EPLAN part number of the order
		//number 3RT2015-1BB41: the manufacturer's short name and a dot
	static const QRegularExpression prefix(QStringLiteral("^[A-Z]{2,5}\\.(?=.)"));
	const auto m = prefix.match(k);
	if (m.hasMatch()) k.remove(0, int(m.capturedLength()));
	return k;
}

double PurchaseList::parseNumber(const QString &text)
{
	QString t = text.trimmed();
	static const QRegularExpression noise(QStringLiteral("\\s|\u20ac|\\$|\u00a3|EUR|CHF|USD"));
	t.remove(noise);
	if (t.isEmpty()) return -1;
	const int comma = int(t.lastIndexOf(QLatin1Char(',')));
	const int dot = int(t.lastIndexOf(QLatin1Char('.')));
	if (comma >= 0 && dot >= 0) {
			//the last of the two is the decimal sign, the other groups
		if (comma > dot) { t.remove(QLatin1Char('.')); t.replace(QLatin1Char(','), QLatin1Char('.')); }
		else t.remove(QLatin1Char(','));
	} else if (comma >= 0) {
		t.replace(QLatin1Char(','), QLatin1Char('.'));
	}
	bool ok = false;
	const double value = t.toDouble(&ok);
	return (ok && value >= 0 && std::isfinite(value)) ? value : -1;
}

void PurchaseList::splitIdentcode(const QString &identcode, QString *gtin, QString *type)
{
		//"4011209780774 EAN", "4017332999168 GTIN", "4028177250819 EAN-13"
	static const QRegularExpression re(QStringLiteral("^\\s*(\\d{8,14})\\s*(.*)$"));
	const auto m = re.match(identcode);
	if (gtin) *gtin = m.hasMatch() ? m.captured(1) : QString();
	if (type) *type = m.hasMatch() ? m.captured(2).trimmed() : QString();
}

QList<PurchaseList::ArticleData> PurchaseList::loadArticles(const QString &path, QString *error)
{
	QList<ArticleData> articles;
	if (error) error->clear();
	if (path.isEmpty()) return articles;
	QFile file(path);
	if (!file.exists()) return articles;
	if (!file.open(QIODevice::ReadOnly)) {
		if (error) *error = file.errorString();
		return articles;
	}
	QString content = QString::fromUtf8(file.readAll());
	if (content.startsWith(QChar(0xFEFF))) content.remove(0, 1);
	const QString first = content.section(QLatin1Char('\n'), 0, 0);
	const QList<QStringList> rows = MaterialList::parseCsv(content, MaterialList::detectSeparator(first));
	if (rows.isEmpty()) return articles;

	QStringList columns;
	for (const QString &h : rows.first()) columns << columnFor(h);
	if (!columns.contains(QStringLiteral("part"))) {
		if (error) *error = tr("%1 has no column \"Bestellnummer\".").arg(QDir::toNativeSeparators(path));
		return articles;
	}
	for (int r = 1; r < rows.size(); ++r) {
		const QStringList &row = rows.at(r);
		ArticleData a;
		for (int c = 0; c < row.size() && c < columns.size(); ++c) {
			const QString v = row.at(c).trimmed();
			const QString &col = columns.at(c);
			if (col == QLatin1String("part")) a.part_number = v;
			else if (col == QLatin1String("manufacturer")) a.manufacturer = v;
			else if (col == QLatin1String("price")) a.price = parseNumber(v);
			else if (col == QLatin1String("price_unit")) a.price_unit = std::max(0.0, parseNumber(v));
			else if (col == QLatin1String("currency")) a.currency = v;
			else if (col == QLatin1String("delivery")) a.delivery_time = v;
			else if (col == QLatin1String("supplier")) a.supplier = v;
			else if (col == QLatin1String("packaging")) a.packaging = std::max(0.0, parseNumber(v));
			else if (col == QLatin1String("gtin")) {
				QString g, t;
				splitIdentcode(v, &g, &t);
				a.gtin = g.isEmpty() ? v : g;
				if (!t.isEmpty() && a.gtin_type.isEmpty()) a.gtin_type = t;
			}
			else if (col == QLatin1String("gtin_type")) { if (!v.isEmpty()) a.gtin_type = v; }
			else if (col == QLatin1String("unit")) a.unit = v;
			else if (col == QLatin1String("lifecycle")) a.lifecycle = v;
			else if (col == QLatin1String("source")) a.source = v;
			else if (col == QLatin1String("date")) a.date = v;
		}
		if (!key(a.part_number).isEmpty()) articles << a;
	}
	return articles;
}

bool PurchaseList::saveCache(const QList<ArticleData> &articles, QString *error)
{
	const QString path = cachePath();
	if (path.isEmpty()) {
		if (error) *error = tr("No company data folder is set up.");
		return false;
	}
	QDir().mkpath(QFileInfo(path).absolutePath());
	QByteArray csv("\xEF\xBB\xBF");
	csv += BomExport::csvRecord({
		QStringLiteral("Hersteller"), QStringLiteral("Bestellnummer"),
		QStringLiteral("EAN"), QStringLiteral("EAN-Art"), QStringLiteral("VPE"),
		QStringLiteral("Einheit"), QStringLiteral("Preis"), QStringLiteral("Preiseinheit"),
		QStringLiteral("W\u00e4hrung"), QStringLiteral("Lebenszyklus"),
		QStringLiteral("Quelle"), QStringLiteral("Stand")});
	const QChar dot(QLatin1Char('.'));
	for (const ArticleData &a : articles) {
		csv += BomExport::csvRecord({
			a.manufacturer, a.part_number, a.gtin, a.gtin_type,
			a.packaging > 0 ? decimal(a.packaging, dot) : QString(),
			a.unit,
			a.price >= 0 ? decimal(a.price, dot) : QString(),
			a.price_unit > 0 ? decimal(a.price_unit, dot) : QString(),
			a.currency, a.lifecycle, a.source, a.date});
	}
	return BomExport::writeCsv(path, csv, error);
}

QHash<QString, PurchaseList::ArticleData> PurchaseList::byKey(const QList<ArticleData> &articles)
{
	QHash<QString, ArticleData> map;
	for (const ArticleData &a : articles) map.insert(key(a.part_number), a);
	return map;
}

QList<PurchaseList::Item> PurchaseList::collect(QETProject *project, QString *error)
{
	QList<Item> items;
	if (error) error->clear();
	if (!project || !project->dataBase()) {
		if (error) *error = tr("No project.");
		return items;
	}
	project->dataBase()->updateDB();

		//Every element and each of its auxiliary blocks, as one set of
		//columns; the auxiliary blocks have no price of their own
	const auto part = [](const QString &suffix) {
		return QStringLiteral(
			"SELECT TRIM(COALESCE(label, '')) AS label, element_type, "
			"TRIM(COALESCE(manufacturer%1, '')) AS manufacturer, "
			"TRIM(COALESCE(manufacturer_reference%1, '')) AS article, "
			"COALESCE(NULLIF(TRIM(description%1), ''), TRIM(COALESCE(designation%1, ''))) AS name, "
			"TRIM(COALESCE(unity%1, '')) AS unit, "
			"TRIM(COALESCE(quantity%1, '')) AS qty, "
			"TRIM(COALESCE(supplier%1, '')) AS supplier, "
			"%2 AS price, %3 AS main "
			"FROM element_nomenclature_view "
			"WHERE element_type IN ('simple', 'master', 'slave', 'terminal')")
				.arg(suffix,
					 suffix.isEmpty() ? QStringLiteral("TRIM(COALESCE(unit_price, ''))")
									  : QStringLiteral("''"),
					 suffix.isEmpty() ? QStringLiteral("1") : QStringLiteral("0"));
	};
	QString sql = part(QString());
	for (int i = 1; i <= 4; ++i)
		sql += QStringLiteral(" UNION ALL ") + part(QStringLiteral("_auxiliary%1").arg(i));

	QString rejection;
	QSqlQuery query = project->dataBase()->newQuery(sql, &rejection);
	if (!rejection.isEmpty()) {
		if (error) *error = rejection;
		return items;
	}
	if (!query.exec()) {
		if (error) *error = query.lastError().text();
		return items;
	}

	struct Group {
		Item item;
		QStringList prices;     ///< as written on each element
		QStringList suppliers;
	};
	QHash<QString, int> index;   ///< group key -> position in groups
	QList<Group> groups;
	QSet<QString> labels_with_article;
	QList<Item> open_rows;

	while (query.next()) {
		const QString label = query.value(0).toString();
		const QString type = query.value(1).toString();
		const QString manufacturer = query.value(2).toString();
		const QString article = query.value(3).toString();
		const QString name = query.value(4).toString();
		const QString unit = query.value(5).toString();
		double qty = parseNumber(query.value(6).toString());
		if (qty < 0) qty = 1;
		const QString supplier = query.value(7).toString();
		const QString price = query.value(8).toString();
		const bool main = query.value(9).toInt() == 1;

		if (article.isEmpty()) {
				//A device without an article: a master or simple element
				//with a label, or a terminal or contact block given a name.
				//An unnamed contact block belongs to its master, a bare
				//terminal to its strip; an empty auxiliary block is none.
			if (!main) continue;
			const bool device = (type == QLatin1String("simple") || type == QLatin1String("master"))
								&& !label.isEmpty();
			if (!device && name.isEmpty()) continue;
			Item open;
			open.open = true;
			open.manufacturer = manufacturer;
			open.name = name;
			open.unit = unit;
			open.quantity = qty;
			open.supplier = supplier;
			if (!label.isEmpty()) open.references << label;
			open_rows << open;
			continue;
		}
		if (!label.isEmpty()) labels_with_article.insert(label);

		const QString group_key = folded(manufacturer) + QLatin1Char('|') + key(article)
								  + QLatin1Char('|') + unit.toLower();
		int at = index.value(group_key, -1);
		if (at < 0) {
			Group g;
			g.item.manufacturer = manufacturer;
			g.item.part_number = article;
			g.item.unit = unit;
			at = int(groups.size());
			groups << g;
			index.insert(group_key, at);
		}
		Group &g = groups[at];
		if (g.item.name.isEmpty()) g.item.name = name;
		g.item.quantity += qty;
		if (!label.isEmpty() && !g.item.references.contains(label)) g.item.references << label;
		if (main) g.prices << price;
		if (!supplier.isEmpty() && !g.suppliers.contains(supplier)) g.suppliers << supplier;
	}

	const QHash<QString, ArticleData> cache = byKey(loadArticles(cachePath()));
	const QHash<QString, ArticleData> prices = byKey(loadArticles(priceListPath()));
	const QCollator collator = naturalCollator();

	for (Group &g : groups) {
		Item &item = g.item;
			//1. the elements: one price on all of them, else none
		double common = -2;
		bool all_priced = !g.prices.isEmpty();
		for (const QString &p : std::as_const(g.prices)) {
			const double v = parseNumber(p);
			if (v < 0) { all_priced = false; break; }
			if (common == -2) common = v;
			else if (std::abs(common - v) > 1e-9) { all_priced = false; break; }
		}
		if (all_priced && common >= 0) {
			item.price = common;
			item.price_source = tr("Project");
		}
		if (g.suppliers.size() == 1) item.supplier = g.suppliers.first();

		const QString k = key(item.part_number);
			//2. the article data cache: Data Portal, EDZ
		const auto cached = cache.constFind(k);
		if (cached != cache.constEnd()) {
			const ArticleData &a = cached.value();
			if (!a.gtin.isEmpty()) item.gtin = a.gtin;
			if (a.packaging > 0) item.packaging = a.packaging;
			if (item.unit.isEmpty()) item.unit = a.unit;
			if (!a.lifecycle.isEmpty()) item.lifecycle = a.lifecycle;
				//EPLAN writes 0 for a price nobody entered
			if (item.price < 0 && a.price > 0) {
				item.price = a.price;
				item.price_unit = a.price_unit > 0 ? a.price_unit : 1;
				item.currency = a.currency;
				item.price_source = QStringLiteral("EDZ");
			}
		}
			//3. the company price list overrides everything it gives
		const auto listed = prices.constFind(k);
		if (listed != prices.constEnd()) {
			const ArticleData &a = listed.value();
			if (a.price >= 0) {
				item.price = a.price;
				item.price_unit = a.price_unit > 0 ? a.price_unit : 1;
				item.currency = a.currency.isEmpty() ? QStringLiteral("EUR") : a.currency;
				item.price_source = tr("Price list");
			}
			if (!a.delivery_time.isEmpty()) item.delivery_time = a.delivery_time;
			if (!a.supplier.isEmpty()) item.supplier = a.supplier;
			if (a.packaging > 0) item.packaging = a.packaging;
			if (!a.gtin.isEmpty()) item.gtin = a.gtin;
			if (item.unit.isEmpty()) item.unit = a.unit;
		}
		std::sort(item.references.begin(), item.references.end(),
				  [&](const QString &a, const QString &b) { return collator.compare(a, b) < 0; });
		items << item;
	}

		//The devices without an article, one line per label. A device
		//one of whose elements has an article is not open (a PLC channel
		//drawn apart from its module, say).
	QHash<QString, int> open_index;
	for (const Item &o : std::as_const(open_rows)) {
		const QString label = o.references.value(0);
		if (!label.isEmpty() && labels_with_article.contains(label)) continue;
		if (!label.isEmpty() && open_index.contains(label)) {
			Item &known = items[open_index.value(label)];
			if (known.name.isEmpty()) known.name = o.name;
			continue;
		}
		if (!label.isEmpty()) open_index.insert(label, int(items.size()));
		items << o;
	}
	sort(items);
	return items;
}

void PurchaseList::sort(QList<Item> &items)
{
	const QCollator collator = naturalCollator();
	std::stable_sort(items.begin(), items.end(), [&](const Item &a, const Item &b) {
		return lessItem(a, b, collator);
	});
}

PurchaseList::Summary PurchaseList::summarize(const QList<Item> &items)
{
	Summary s;
	for (const Item &i : items) {
		if (i.open) { ++s.open; continue; }
		++s.articles;
		if (!i.gtin.isEmpty()) ++s.with_gtin;
		if (i.price >= 0) {
			++s.priced;
			s.total += i.total();
			if (!i.currency.isEmpty() && !s.currencies.contains(i.currency))
				s.currencies << i.currency;
		}
	}
	return s;
}

QStringList PurchaseList::headers()
{
	return {
		tr("Pos."), tr("Supplier"), tr("Manufacturer"), tr("Order number"),
		tr("Name"), tr("Device tags"), tr("Quantity"), tr("Unit"),
		tr("Packaging unit"), tr("Order quantity"), tr("Price"),
		tr("Price unit"), tr("Currency"), tr("Total price"),
		tr("Delivery time"), tr("EAN/GTIN"), tr("Lifecycle"),
		tr("Price source"), tr("Article open")
	};
}

QStringList PurchaseList::erpHeaders()
{
	return {
		QStringLiteral("line"), QStringLiteral("supplier"), QStringLiteral("manufacturer"),
		QStringLiteral("order_number"), QStringLiteral("description"),
		QStringLiteral("references"), QStringLiteral("quantity"),
		QStringLiteral("order_quantity"), QStringLiteral("unit"),
		QStringLiteral("packaging_unit"), QStringLiteral("unit_price"),
		QStringLiteral("price_unit"), QStringLiteral("currency"),
		QStringLiteral("total_price"), QStringLiteral("delivery_time"),
		QStringLiteral("gtin"), QStringLiteral("lifecycle"),
		QStringLiteral("price_source"), QStringLiteral("article_open")
	};
}

QByteArray PurchaseList::toCsv(const QList<Item> &items)
{
	const QChar comma(QLatin1Char(','));
	QByteArray csv("\xEF\xBB\xBF");
	csv += BomExport::csvRecord(headers());
	int position = 0;
	double grand = 0;
	bool any_price = false;
	for (int i = 0; i < items.size(); ) {
		const QString group = items.at(i).groupName();
		const bool open = items.at(i).open;
		double sum = 0;
		bool priced = false;
		for (; i < items.size() && items.at(i).groupName() == group
				&& items.at(i).open == open; ++i) {
			const Item &item = items.at(i);
			csv += BomExport::csvRecord(defused(csvLine(++position, item)));
			if (item.total() >= 0) { sum += item.total(); priced = true; }
		}
		if (open) continue;
		QStringList line;
		for (int c = 0; c < ColCount; ++c) line << QString();
		line[ColSupplier] = tr("Sum %1").arg(group);
		line[ColTotal] = priced ? money(sum, comma) : QString();
		csv += BomExport::csvRecord(defused(line));
		grand += sum;
		any_price = any_price || priced;
	}
	QStringList line;
	for (int c = 0; c < ColCount; ++c) line << QString();
	line[ColSupplier] = tr("Grand total");
	line[ColTotal] = any_price ? money(grand, comma) : QString();
	csv += BomExport::csvRecord(line);
	return csv;
}

QByteArray PurchaseList::toErpCsv(const QList<Item> &items)
{
	const QChar dot(QLatin1Char('.'));
	QByteArray csv = BomExport::csvRecord(erpHeaders());
	int line = 0;
	for (const Item &item : items) {
		csv += BomExport::csvRecord({
			QString::number(++line),
			item.supplier, item.manufacturer, item.part_number, item.name,
			item.references.join(QLatin1Char(',')),
			decimal(item.quantity, dot),
			decimal(item.orderQuantity(), dot),
			item.unit,
			item.packaging > 0 ? decimal(item.packaging, dot) : QString(),
			item.price >= 0 ? decimal(item.price, dot) : QString(),
			item.price >= 0 ? decimal(item.price_unit, dot) : QString(),
			item.currency,
			item.total() >= 0 ? decimal(item.total(), dot) : QString(),
			item.delivery_time, item.gtin, item.lifecycle, item.price_source,
			item.open ? QStringLiteral("1") : QStringLiteral("0")});
	}
	return csv;
}

QByteArray PurchaseList::toXlsx(const QList<Item> &items, QString *error)
{
	const QChar dot(QLatin1Char('.'));
		//styles: 0 plain, 1 bold, 2 number #,##0.00, 3 bold number,
		//4 bold heading on a fill
	const auto text_cell = [](int col, int row, const QString &value, int style = 0) {
		if (value.isEmpty() && style == 0) return QString();
		return QStringLiteral("<c r=\"%1%2\" t=\"inlineStr\"%3><is><t xml:space=\"preserve\">%4</t></is></c>")
				.arg(columnLetter(col)).arg(row)
				.arg(style ? QStringLiteral(" s=\"%1\"").arg(style) : QString(), xmlEscaped(value));
	};
	const auto number_cell = [&](int col, int row, double value, int style = 0) {
		return QStringLiteral("<c r=\"%1%2\"%3><v>%4</v></c>")
				.arg(columnLetter(col)).arg(row)
				.arg(style ? QStringLiteral(" s=\"%1\"").arg(style) : QString(), decimal(value, dot));
	};
		//A formula with the value it has now, so a viewer that does not
		//calculate shows it too; a formula giving "" is a string
	const auto formula_cell = [&](int col, int row, const QString &formula, double cached,
								  bool has_value, int style) {
		return QStringLiteral("<c r=\"%1%2\" s=\"%3\"%4><f>%5</f><v>%6</v></c>")
				.arg(columnLetter(col)).arg(row).arg(style)
				.arg(has_value ? QString() : QStringLiteral(" t=\"str\""),
					 xmlEscaped(formula), has_value ? decimal(cached, dot) : QString());
	};

	QString rows;
	int row = 1;
	{
		QString cells;
		const QStringList h = headers();
		for (int c = 0; c < h.size(); ++c) cells += text_cell(c, row, h.at(c), 4);
		rows += QStringLiteral("<row r=\"1\">%1</row>").arg(cells);
	}
	const QString total_col = columnLetter(ColTotal);
	int position = 0;
	double grand = 0;
	for (int i = 0; i < items.size(); ) {
		const QString group = items.at(i).groupName();
		const bool open = items.at(i).open;
		const int first_row = row + 1;
		double sum = 0;
		for (; i < items.size() && items.at(i).groupName() == group
				&& items.at(i).open == open; ++i) {
			const Item &item = items.at(i);
			++row;
			QString cells;
			cells += number_cell(ColPos, row, ++position);
			cells += text_cell(ColSupplier, row, item.supplier);
			cells += text_cell(ColManufacturer, row, item.manufacturer);
			cells += text_cell(ColPart, row, item.part_number);
			cells += text_cell(ColName, row, item.name);
			cells += text_cell(ColRefs, row, item.references.join(QStringLiteral(", ")));
			cells += number_cell(ColQty, row, item.quantity);
			cells += text_cell(ColUnit, row, item.unit);
			if (item.packaging > 0) cells += number_cell(ColPackaging, row, item.packaging);
				//The order quantity follows quantity and packaging unit,
				//so an edit in the sheet carries through
			cells += formula_cell(ColOrderQty, row,
				QStringLiteral("IF(%1%3>0,CEILING(%2%3/%1%3,1)*%1%3,%2%3)")
					.arg(columnLetter(ColPackaging), columnLetter(ColQty)).arg(row),
				item.orderQuantity(), true, 0);
			if (item.price >= 0) {
				cells += number_cell(ColPrice, row, item.price, 2);
				cells += number_cell(ColPriceUnit, row, item.price_unit);
			}
			cells += text_cell(ColCurrency, row, item.currency);
				//Empty until a price is there: a price typed into the
				//sheet by purchasing gives the total at once
			cells += formula_cell(ColTotal, row,
				QStringLiteral("IF(%1%4=\"\",\"\",%2%4*%1%4/IF(%3%4>0,%3%4,1))")
					.arg(columnLetter(ColPrice), columnLetter(ColOrderQty),
						 columnLetter(ColPriceUnit)).arg(row),
				item.total(), item.total() >= 0, 2);
			cells += text_cell(ColDelivery, row, item.delivery_time);
			cells += text_cell(ColGtin, row, item.gtin);
			cells += text_cell(ColLifecycle, row, item.lifecycle);
			cells += text_cell(ColSource, row, item.price_source);
			if (item.open) cells += text_cell(ColOpen, row, tr("yes", "article open"));
			rows += QStringLiteral("<row r=\"%1\">%2</row>").arg(row).arg(cells);
			if (item.total() >= 0) sum += item.total();
		}
		if (open) continue;
		++row;
		rows += QStringLiteral("<row r=\"%1\">%2%3</row>").arg(row)
				.arg(text_cell(ColSupplier, row, tr("Sum %1").arg(group), 1),
					 formula_cell(ColTotal, row,
								  QStringLiteral("SUBTOTAL(9,%1%2:%1%3)").arg(total_col).arg(first_row).arg(row - 1),
								  sum, true, 3));
		grand += sum;
	}
	++row;
		//SUBTOTAL leaves out the group sums, which are SUBTOTALs themselves
	rows += QStringLiteral("<row r=\"%1\">%2%3</row>").arg(row)
			.arg(text_cell(ColSupplier, row, tr("Grand total"), 1),
				 formula_cell(ColTotal, row,
							  QStringLiteral("SUBTOTAL(9,%1%2:%1%3)").arg(total_col).arg(2).arg(row - 1),
							  grand, true, 3));

		//Widths in characters, roughly what the columns hold
	static const int widths[ColCount] = {6, 22, 18, 22, 40, 24, 9, 7, 8, 11, 10, 9, 8, 12, 12, 16, 11, 14, 10};
	QString cols;
	for (int c = 0; c < ColCount; ++c)
		cols += QStringLiteral("<col min=\"%1\" max=\"%1\" width=\"%2\" customWidth=\"1\"/>").arg(c + 1).arg(widths[c]);

	const QString last = columnLetter(ColCount - 1);
	const QString sheet = QStringLiteral(
		"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
		"<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
		"xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
		"<dimension ref=\"A1:%1%2\"/>"
		"<sheetViews><sheetView workbookViewId=\"0\"><pane ySplit=\"1\" topLeftCell=\"A2\" "
		"activePane=\"bottomLeft\" state=\"frozen\"/></sheetView></sheetViews>"
		"<sheetFormatPr defaultRowHeight=\"15\"/>"
		"<cols>%3</cols><sheetData>%4</sheetData>"
		"<autoFilter ref=\"A1:%1%2\"/>"
		"</worksheet>").arg(last).arg(row).arg(cols, rows);

	const QString styles = QStringLiteral(
		"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
		"<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
		"<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
		"<font><b/><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
		"<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill>"
		"<fill><patternFill patternType=\"gray125\"/></fill>"
		"<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFD9E1F2\"/><bgColor indexed=\"64\"/></patternFill></fill></fills>"
		"<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
		"<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
		"<cellXfs count=\"5\">"
		"<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
		"<xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/>"
		"<xf numFmtId=\"4\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyNumberFormat=\"1\"/>"
		"<xf numFmtId=\"4\" fontId=\"1\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\" applyNumberFormat=\"1\"/>"
		"<xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\"/>"
		"</cellXfs>"
		"<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>"
		"</styleSheet>");

		//Excel allows 31 characters and none of []:*?/\ in a sheet name
	QString sheet_name = tr("Purchase list");
	static const QRegularExpression forbidden(QStringLiteral("[\\[\\]:*?/\\\\]"));
	sheet_name.remove(forbidden);
	sheet_name = sheet_name.left(31);
	QString quoted_name = sheet_name;
	quoted_name.replace(QLatin1Char('\''), QStringLiteral("''"));

	const QString workbook = QStringLiteral(
		"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
		"<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
		"xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
		"<sheets><sheet name=\"%1\" sheetId=\"1\" r:id=\"rId1\"/></sheets>"
		"<definedNames><definedName name=\"_xlnm._FilterDatabase\" localSheetId=\"0\" hidden=\"1\">"
		"'%2'!$A$1:$%3$%4</definedName></definedNames>"
		"<calcPr calcId=\"191029\" fullCalcOnLoad=\"1\"/>"
		"</workbook>").arg(xmlEscaped(sheet_name), xmlEscaped(quoted_name), last).arg(row);

	QList<QetZip::Entry> entries;
	entries << QetZip::Entry{QStringLiteral("[Content_Types].xml"), QStringLiteral(
			"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
			"<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
			"<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
			"<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
			"<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
			"<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
			"<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
			"</Types>").toUtf8(), true}
			<< QetZip::Entry{QStringLiteral("_rels/.rels"), QStringLiteral(
			"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
			"<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
			"<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
			"</Relationships>").toUtf8(), true}
			<< QetZip::Entry{QStringLiteral("xl/workbook.xml"), workbook.toUtf8(), true}
			<< QetZip::Entry{QStringLiteral("xl/_rels/workbook.xml.rels"), QStringLiteral(
			"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
			"<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
			"<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
			"<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>"
			"</Relationships>").toUtf8(), true}
			<< QetZip::Entry{QStringLiteral("xl/worksheets/sheet1.xml"), sheet.toUtf8(), true}
			<< QetZip::Entry{QStringLiteral("xl/styles.xml"), styles.toUtf8(), true};
	return QetZip::toBytes(entries, error);
}

QString PurchaseList::erpPath(const QString &output)
{
	const QFileInfo info(output);
	return info.dir().filePath(info.completeBaseName() + QStringLiteral("_ERP.csv"));
}

bool PurchaseList::write(const QList<Item> &items, const QString &output, bool erp,
						 QStringList *written, QString *error)
{
	if (error) error->clear();
	QByteArray data;
	if (output.endsWith(QLatin1String(".xlsx"), Qt::CaseInsensitive)) {
		data = toXlsx(items, error);
		if (data.isEmpty()) return false;
	} else {
		data = toCsv(items);
	}
	if (!BomExport::writeCsv(output, data, error)) return false;
	if (written) *written << QDir::toNativeSeparators(output);
	if (erp) {
		const QString path = erpPath(output);
		if (!BomExport::writeCsv(path, toErpCsv(items), error)) return false;
		if (written) *written << QDir::toNativeSeparators(path);
	}
	return true;
}
