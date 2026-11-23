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
#include "purchasefetch.h"

#include "../import/edz/edzarchive.h"
#include "../import/eplandxf/dataportalclient.h"
#include "../import/portals/manufacturerportals.h"

#include <QDate>
#include <QDir>
#include <QDirIterator>
#include <QDomDocument>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSet>

namespace {

constexpr int MAX_AGE_DAYS = 30;

QString today()
{
	return QDate::currentDate().toString(Qt::ISODate);
}

bool fresh(const PurchaseList::ArticleData &a)
{
	const QDate date = QDate::fromString(a.date, Qt::ISODate);
	return date.isValid() && date.daysTo(QDate::currentDate()) < MAX_AGE_DAYS;
}

	/// "en_US@Piece;de_DE@Stück;" -> the German text, else the English one
QString pickLanguage(const QString &multi)
{
	if (!multi.contains(QLatin1Char('@'))) return multi.trimmed();
	QHash<QString, QString> texts;
	QString first;
	for (const QString &part : multi.split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
		const QString lang = part.section(QLatin1Char('@'), 0, 0).trimmed();
		const QString text = part.section(QLatin1Char('@'), 1).trimmed();
		if (text.isEmpty()) continue;
		if (first.isEmpty()) first = text;
		texts.insert(lang.left(2).toLower(), text);
	}
	if (texts.contains(QStringLiteral("de"))) return texts.value(QStringLiteral("de"));
	if (texts.contains(QStringLiteral("en"))) return texts.value(QStringLiteral("en"));
	return first;
}

	/// the EDZ packages of the data folder by article key: the folder
	/// herstellerdaten/<manufacturer>/<order number>/ the part fetcher
	/// files them in, or the name of the package itself
QHash<QString, QString> edzPackages()
{
	QHash<QString, QString> packages;
	const QString raw = ManufacturerPortals::rawDir();
	if (raw.isEmpty() || !QFileInfo(raw).isDir()) return packages;
	QDirIterator it(raw, {QStringLiteral("*.edz")}, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		const QString path = it.next();
		const QFileInfo info(path);
		const QString folder = PurchaseList::key(info.dir().dirName());
		const QString base = PurchaseList::key(info.completeBaseName());
		if (!folder.isEmpty() && !packages.contains(folder)) packages.insert(folder, path);
		if (!base.isEmpty() && !packages.contains(base)) packages.insert(base, path);
	}
	return packages;
}

	/// the parts of the Data Portal whose number is @p part_number
QVector<DataPortalPart> searchPortal(const QString &part_number, QString *error)
{
	DataPortalClient client;
	client.setLanguage(QStringLiteral("de_DE"));
	DataPortalQuery query;
	query.text = part_number;
	query.exact = true;
	query.limit = 20;
	QVector<DataPortalPart> found;
	QEventLoop loop;
	QObject::connect(&client, &DataPortalClient::searchFinished, &loop,
					 [&](const QVector<DataPortalPart> &parts, int, int) {
		found = parts;
		loop.quit();
	});
	QObject::connect(&client, &DataPortalClient::failed, &loop, [&](const QString &message) {
		if (error) *error = message;
		loop.quit();
	});
	client.search(query);
	loop.exec();
	return found;
}

bool sameNumber(const DataPortalPart &p, const QString &k)
{
	return PurchaseList::key(p.order_number) == k
		   || PurchaseList::key(p.part_number) == k
		   || PurchaseList::key(p.type_number) == k;
}

	/// the part of @p parts that is the article: the same number, and the
	/// same manufacturer when the project names one. Two manufacturers
	/// with the same number and none named: none, rather than a guess.
const DataPortalPart *bestPart(const QVector<DataPortalPart> &parts, const QString &k,
							   const QString &manufacturer)
{
	const QString wanted = manufacturer.trimmed().toLower();
	const DataPortalPart *best = nullptr;
	for (const DataPortalPart &p : parts) {
		if (!sameNumber(p, k)) continue;
		const QString name = p.manufacturer.toLower();
		const bool maker = !wanted.isEmpty() && !name.isEmpty()
				&& (name.contains(wanted) || wanted.contains(name)
					|| p.manufacturer_short.toLower() == wanted);
		if (maker) return &p;
		if (best && best->manufacturer != p.manufacturer) return nullptr;
		if (!best) best = &p;
	}
	return best;
}

} // namespace

QJsonObject PurchaseFetch::Result::toJson() const
{
	QJsonObject o;
	o.insert(QStringLiteral("articles"), articles);
	o.insert(QStringLiteral("cached"), cached);
	o.insert(QStringLiteral("from_portal"), from_portal);
	o.insert(QStringLiteral("from_edz"), from_edz);
	o.insert(QStringLiteral("not_found"), not_found);
	if (!error.isEmpty()) o.insert(QStringLiteral("error"), error);
	return o;
}

bool PurchaseFetch::readEdz(const QString &edz_path, PurchaseList::ArticleData *article)
{
	EdzArchive archive;
	if (!archive.extract(edz_path)) return false;
	QFile file(archive.partXmlPath());
	if (!file.open(QIODevice::ReadOnly)) return false;
	QDomDocument doc;
	if (!doc.setContent(&file)) return false;
	const QDomElement part = doc.documentElement().firstChildElement(QStringLiteral("part"));
	if (part.isNull()) return false;

	const auto attr = [&](const char *name) {
		return part.attribute(QLatin1String(name)).trimmed();
	};
	PurchaseList::ArticleData a;
	a.part_number = attr("P_ARTICLE_ORDERNR");
	if (a.part_number.isEmpty()) a.part_number = attr("P_ARTICLE_PARTNR");
	a.manufacturer = attr("P_ARTICLE_MANUFACTURER");
	PurchaseList::splitIdentcode(attr("P_ARTICLE_IDENTCODE"), &a.gtin, &a.gtin_type);
	const double packaging = PurchaseList::parseNumber(attr("P_ARTICLE_PACKAGINGQUANTITY"));
	if (packaging > 0) a.packaging = packaging;
	a.unit = pickLanguage(attr("P_ARTICLE_QUANTITYUNIT"));
		//EPLAN writes 0 for a price nobody entered: no price
	const double price = PurchaseList::parseNumber(attr("P_ARTICLE_SALESPRICE_1"));
	if (price > 0) {
		a.price = price;
		const double unit = PurchaseList::parseNumber(attr("P_ARTICLE_PRICEUNIT"));
		a.price_unit = unit > 0 ? unit : 1;
	}
	a.source = QStringLiteral("EDZ");
	a.date = today();
	if (article) *article = a;
	return !a.gtin.isEmpty() || a.packaging > 0 || a.price > 0 || !a.unit.isEmpty();
}

PurchaseFetch::Result PurchaseFetch::fetch(const QList<PurchaseList::Item> &items, bool online,
										   const Progress &progress)
{
	Result result;
	QList<PurchaseList::ArticleData> cache = PurchaseList::loadArticles(PurchaseList::cachePath());
	QHash<QString, int> index;
	for (int i = 0; i < cache.size(); ++i) index.insert(PurchaseList::key(cache.at(i).part_number), i);

	QList<const PurchaseList::Item *> todo;
	QSet<QString> seen;
	for (const PurchaseList::Item &item : items) {
		if (item.open) continue;
		const QString k = PurchaseList::key(item.part_number);
		if (k.isEmpty() || seen.contains(k)) continue;
		seen.insert(k);
		++result.articles;
		if (index.contains(k) && fresh(cache.at(index.value(k)))) {
			++result.cached;
			continue;
		}
		todo << &item;
	}
	if (todo.isEmpty()) return result;

	const QHash<QString, QString> packages = edzPackages();
	bool changed = false;
	int done = 0;
	for (const PurchaseList::Item *item : std::as_const(todo)) {
		const QString k = PurchaseList::key(item->part_number);
		if (progress && !progress(done, int(todo.size()), item->part_number)) break;
		++done;

		PurchaseList::ArticleData a;
		a.manufacturer = item->manufacturer;
		a.part_number = item->part_number;
		a.date = today();
		QStringList sources;

		const auto package = packages.constFind(k);
		if (package != packages.constEnd()) {
			PurchaseList::ArticleData edz;
			if (readEdz(package.value(), &edz)) {
				a.gtin = edz.gtin;
				a.gtin_type = edz.gtin_type;
				a.packaging = edz.packaging;
				a.unit = edz.unit;
				a.price = edz.price;
				a.price_unit = edz.price_unit;
				sources << QStringLiteral("EDZ");
				++result.from_edz;
			}
		}

		bool asked = false;
		if (online && result.error.isEmpty()) {
			QString error;
			const QVector<DataPortalPart> parts = searchPortal(item->part_number, &error);
			if (!error.isEmpty()) {
					//No connection, or the portal refuses: stop asking,
					//keep what the EDZ packages gave
				result.error = error;
			} else {
				asked = true;
				if (const DataPortalPart *p = bestPart(parts, k, item->manufacturer)) {
					QString gtin, type;
					PurchaseList::splitIdentcode(p->identcode, &gtin, &type);
					if (!gtin.isEmpty()) { a.gtin = gtin; a.gtin_type = type; }
					if (!p->lifecycle.isEmpty()) a.lifecycle = p->lifecycle;
					if (a.manufacturer.isEmpty()) a.manufacturer = p->manufacturer;
					sources.prepend(QStringLiteral("Data Portal"));
					++result.from_portal;
				}
			}
		}

		if (sources.isEmpty()) {
				//Remembered as unknown only when the portal was asked:
				//offline, the next export asks again
			if (!asked) continue;
			a.source = QStringLiteral("nicht gefunden");
			++result.not_found;
		} else {
			a.source = sources.join(QStringLiteral(", "));
		}
		const auto known = index.constFind(k);
		if (known != index.constEnd()) {
			cache[known.value()] = a;
		} else {
			index.insert(k, int(cache.size()));
			cache << a;
		}
		changed = true;
	}
	if (progress) progress(done, int(todo.size()), QString());
	if (changed) {
		QString error;
		if (!PurchaseList::saveCache(cache, &error) && result.error.isEmpty())
			result.error = error;
	}
	return result;
}
