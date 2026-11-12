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
#include "partfetcher.h"

#include "manufacturerportals.h"
#include "../../qetapp.h"
#include "../../panel/panelstep.h"
#include "../eplandxf/dataportalclient.h"
#include "../eplandxf/eplanpartimporter.h"
#include "../../container/qetzip.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTimer>
#include <QUrl>

namespace {
	const qint64 MAX_FILE = 300LL * 1024 * 1024;	// no manufacturer file is larger
	const qint64 PLACEHOLDER_STEP = 30 * 1024;	// smaller STEP files are placeholder boxes
	const int TIMEOUT_MS = 180000;

QString userAgent()
{
	return QStringLiteral("QElectroTech/%1 (Herstellerportale)").arg(QCoreApplication::applicationVersion());
}

	/// a folder name from a catalog name
QString safeName(QString s)
{
	static const QRegularExpression bad(QStringLiteral("[\\\\/:*?\"<>|]+"));
	s.replace(bad, QStringLiteral(" - "));
	return s.simplified();
}

	/// the order number as folder and file name: Siemens without blanks
QString orderName(const QString &maker, const QString &order)
{
	QString o = order.simplified();
	if (maker == QLatin1String("Siemens")) o.remove(QLatin1Char(' '));
	return safeName(o);
}

	/// makers whose STEP models 3Dfindit has (not Rittal, Murr, Pilz, SEW ...)
bool on3Dfindit(const QString &maker)
{
	static const QStringList makers {
		QStringLiteral("Siemens"), QStringLiteral("Phoenix Contact"), QStringLiteral("Weidmüller"),
		QStringLiteral("WAGO"), QStringLiteral("Schmersal"), QStringLiteral("METZ CONNECT"),
		QStringLiteral("ABB"), QStringLiteral("Eaton"), QStringLiteral("Festo"), QStringLiteral("Balluff"),
		QStringLiteral("ifm electronic"), QStringLiteral("SICK"), QStringLiteral("HARTING")};
	return makers.contains(maker, Qt::CaseInsensitive);
}

	/// a STEP file name
bool isStep(const QString &file)
{
	const QString s = QFileInfo(file).suffix().toLower();
	return s == QLatin1String("stp") || s == QLatin1String("step");
}

	/**
		Unpacks the STEP files of the ZIP archives in @p folder (a CAD
		download saved from the browser, e.g. of keyence.de) next to them,
		without overwriting. The Data Portal packages are left alone.
		@return the files unpacked, or (@p dry_run) that would be
	*/
QStringList unpackStepArchives(const QString &folder, bool dry_run, QStringList *notes)
{
	QStringList out;
	const QFileInfoList zips = QDir(folder).entryInfoList({QStringLiteral("*.zip")}, QDir::Files, QDir::Name);
	for (const QFileInfo &zip : zips) {
		if (zip.fileName().startsWith(QLatin1String("dataportal_"))) continue;
		QList<QetZip::Entry> entries;
		QString error;
		if (!QetZip::read(zip.absoluteFilePath(), &entries, &error)) {
			notes->append(QCoreApplication::translate("PartFetcher", "%1: %2").arg(zip.fileName(), error));
			continue;
		}
		for (const QetZip::Entry &e : std::as_const(entries)) {
			if (e.name.endsWith(QLatin1Char('/')) || !isStep(e.name)) continue;
			const QString target = QDir(folder).filePath(safeName(QFileInfo(e.name).fileName()));
			if (QFileInfo::exists(target)) continue;
			if (dry_run) {
				notes->append(QCoreApplication::translate("PartFetcher", "would unpack %1 from %2")
					      .arg(QFileInfo(target).fileName(), zip.fileName()));
				out << target;
				continue;
			}
			QSaveFile f(target);
			if (f.open(QIODevice::WriteOnly) && f.write(e.data) == e.data.size() && f.commit()) {
				notes->append(QCoreApplication::translate("PartFetcher", "unpacked %1 from %2")
					      .arg(QFileInfo(target).fileName(), zip.fileName()));
				out << target;
			} else {
				notes->append(QCoreApplication::translate("PartFetcher", "%1 cannot be written")
					      .arg(QDir::toNativeSeparators(target)));
			}
		}
	}
	return out;
}
}

QJsonObject FetchResult::toJson() const
{
	QJsonObject o;
	o.insert(QStringLiteral("manufacturer"), maker);
	o.insert(QStringLiteral("order"), order);
	o.insert(QStringLiteral("status"), status);
	o.insert(QStringLiteral("notes"), QJsonArray::fromStringList(notes));
	QStringList w;
	for (const QString &p : written) w << QDir::toNativeSeparators(p);
	o.insert(QStringLiteral("written"), QJsonArray::fromStringList(w));
	if (!step_file.isEmpty()) o.insert(QStringLiteral("step"), step_file);
	return o;
}

PartFetcher::PartFetcher(QObject *parent) :
	QObject(parent),
	m_manager(new QNetworkAccessManager(this))
{
	m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

PartFetcher::~PartFetcher() = default;

QString PartFetcher::orderKey(const QString &order)
{
	static const QRegularExpression other(QStringLiteral("[^A-Z0-9]"));
	return order.toUpper().remove(other);
}

QString PartFetcher::makerName(const QString &maker)
{
	static const QHash<QString, QString> names {
		{QStringLiteral("SIE"), QStringLiteral("Siemens")},
		{QStringLiteral("RIT"), QStringLiteral("Rittal")},
		{QStringLiteral("PXC"), QStringLiteral("Phoenix Contact")},
		{QStringLiteral("WEI"), QStringLiteral("Weidmüller")},
		{QStringLiteral("WAGO"), QStringLiteral("WAGO")},
		{QStringLiteral("ETN"), QStringLiteral("Eaton")},
		{QStringLiteral("SMS"), QStringLiteral("Schmersal")},
		{QStringLiteral("MUR"), QStringLiteral("Murrelektronik")},
		{QStringLiteral("PIZ"), QStringLiteral("Pilz")},
		{QStringLiteral("SEW"), QStringLiteral("SEW-EURODRIVE")},
		{QStringLiteral("FIN"), QStringLiteral("Finder")},
		{QStringLiteral("FES"), QStringLiteral("Festo")},
		{QStringLiteral("KEY"), QStringLiteral("Keyence")}};
	const QString m = maker.trimmed();
	const QString n = names.value(m.toUpper());
	return n.isEmpty() ? EplanPartImporter::manufacturerName(m) : n;
}

bool PartFetcher::isSiemensFileUrl(const QString &url)
{
	const QUrl u(url);
	static const QRegularExpression path(QStringLiteral("^/mimes/imagedb_(edz|dxf|stp)/[^/]+$"));
	return u.scheme() == QLatin1String("https") && u.host() == QLatin1String("static.siemens.com")
	       && path.match(u.path()).hasMatch();
}

QString PartFetcher::siemensLinkFile(const QString &raw_dir)
{
	const QString raw = raw_dir.isEmpty() ? ManufacturerPortals::rawDir() : raw_dir;
	return raw.isEmpty() ? QString() : QDir(raw).filePath(QStringLiteral("Siemens/sieportal_links.txt"));
}

QList<FetchItem> PartFetcher::readList(const QString &path, QString *error)
{
	QList<FetchItem> items;
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
		if (error) *error = tr("The list cannot be read: %1").arg(f.errorString());
		return items;
	}
	static const QRegularExpression header(QStringLiteral("^===\\s*(.*?)\\s*\\(\\d+\\s+\\w+\\)\\s*===\\s*$"));
		// "  <order>   DXF ja    STEP FEHLT <description>  [projects]"
	static const QRegularExpression missing(
		QStringLiteral("^\\s+(\\S.*?)\\s+DXF\\s+\\S+\\s+STEP\\s+\\S+\\s*(.*?)\\s*(\\[[^\\]]*\\])?\\s*$"));
	QString maker;
	const QStringList lines = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
	for (QString line : lines) {
		line.remove(QLatin1Char('\r'));
		const QString trimmed = line.trimmed();
		if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) continue;
		QRegularExpressionMatch m = header.match(line);
		if (m.hasMatch()) {
			maker = m.captured(1);
			if (maker.startsWith(QLatin1Char('('))) maker.clear();	// "(ohne Hersteller)"
			continue;
		}
		m = missing.match(line);
		FetchItem item;
		if (m.hasMatch()) {
			item.maker = maker;
			item.order = m.captured(1).simplified();
			item.description = m.captured(2).simplified();
		} else if (line.contains(QLatin1Char(';'))) {
			item.maker = line.section(QLatin1Char(';'), 0, 0).trimmed();
			item.order = line.section(QLatin1Char(';'), 1, 1).simplified();
			item.description = line.section(QLatin1Char(';'), 2).simplified();
		} else if (!line.startsWith(QLatin1Char(' ')) && trimmed.count(QLatin1Char(' ')) > 2) {
			continue;	// a title line of the list
		} else {
			item.maker = maker;
			item.order = trimmed.simplified();
		}
		if (item.order.isEmpty() || item.maker.isEmpty()) continue;
		items << item;
	}
	if (items.isEmpty() && error) *error = tr("No parts in the list.");
	return items;
}

QHash<QString, QStringList> PartFetcher::readLinks(const QString &path)
{
	QHash<QString, QStringList> links;
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return links;
	static const QRegularExpression space(QStringLiteral("\\s"));
	const QStringList lines = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
	for (QString line : lines) {
		line = line.section(QLatin1Char('#'), 0, 0).trimmed();
		const int at = line.indexOf(QStringLiteral("https://"));
		if (at < 1) continue;
		const QString order = line.left(at).trimmed();
		const QString url = line.mid(at).section(space, 0, 0);
		if (!isSiemensFileUrl(url)) continue;
		QStringList &list = links[orderKey(order)];
		if (!list.contains(url)) list << url;
	}
	return links;
}

void PartFetcher::loadLinks(const FetchOptions &options)
{
	QStringList files = options.link_files;
	files.prepend(siemensLinkFile(options.raw_dir));
	QString key = files.join(QLatin1Char('|'));
	for (const QString &file : std::as_const(files))
		key += QLatin1Char('|') + QFileInfo(file).lastModified().toString(Qt::ISODateWithMs);
	if (key == m_links_key) return;
	m_links.clear();
	m_links_key = key;
	for (const QString &file : std::as_const(files)) {
		const QHash<QString, QStringList> l = readLinks(file);
		for (auto it = l.constBegin(); it != l.constEnd(); ++it)
			for (const QString &url : it.value())
				if (!m_links[it.key()].contains(url)) m_links[it.key()] << url;
	}
}

QString PartFetcher::existingSymbolFolder(const QString &collection, const QString &maker, const QString &order)
{
	const QString key = orderKey(order);
	QDir base(collection);
	if (!base.cd(maker)) return QString();
	const QFileInfoList categories = base.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (const QFileInfo &category : categories) {
		const QFileInfoList parts = QDir(category.absoluteFilePath()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (const QFileInfo &part : parts)
			if (orderKey(part.fileName()) == key
			    && !QDir(part.absoluteFilePath()).entryList({QStringLiteral("*.elmt")}, QDir::Files).isEmpty())
				return part.absoluteFilePath();
	}
	return QString();
}

bool PartFetcher::download(const QString &url, const QString &file, int pause_ms, QString *error)
{
		// one request after the other, with a pause per server
	const QString host = QUrl(url).host();
	const qint64 wait = m_last_request.value(host) + pause_ms - QDateTime::currentMSecsSinceEpoch();
	if (wait > 0) {
		QEventLoop pause;
		QTimer::singleShot(int(wait), &pause, &QEventLoop::quit);
		pause.exec();
	}
	QNetworkRequest req{QUrl(url)};
	req.setTransferTimeout(TIMEOUT_MS);
	req.setHeader(QNetworkRequest::UserAgentHeader, userAgent());
	QNetworkReply *reply = m_manager->get(req);
	QEventLoop loop;
	connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	connect(reply, &QNetworkReply::downloadProgress, reply, [reply](qint64 received, qint64) {
		if (received > MAX_FILE) reply->abort();
	});
	loop.exec();
	m_last_request.insert(host, QDateTime::currentMSecsSinceEpoch());
	const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	const QByteArray data = reply->readAll();
	const QString net_error = reply->errorString();
	const bool ok = reply->error() == QNetworkReply::NoError;
	reply->deleteLater();
	if (!ok || status != 200 || data.isEmpty()) {
		if (error) *error = status ? tr("HTTP %1 for %2").arg(status).arg(url) : net_error;
		return false;
	}
	QDir().mkpath(QFileInfo(file).absolutePath());
	QSaveFile out(file);
	if (!out.open(QIODevice::WriteOnly) || out.write(data) != data.size() || !out.commit()) {
		if (error) *error = tr("%1 cannot be written: %2").arg(QDir::toNativeSeparators(file), out.errorString());
		return false;
	}
	return true;
}

bool PartFetcher::dataPortalLookup(const QString &order, const QString &maker, QString *part_id,
				   QString *category, QString *maker_long, QString *error)
{
	DataPortalClient client;
	client.setLanguage(QStringLiteral("de_DE"));
	QEventLoop loop;
	QVector<DataPortalPart> found;
	QString failure;
	connect(&client, &DataPortalClient::searchFinished, &loop,
		[&](const QVector<DataPortalPart> &parts, int, int) { found = parts; loop.quit(); });
	connect(&client, &DataPortalClient::failed, &loop,
		[&](const QString &message) { failure = message; loop.quit(); });
	DataPortalQuery q;
	q.text = order;
	q.exact = true;
	q.limit = 20;
		// the list names the maker by its Data Portal short name (SIE, RIT)
	if (maker.size() <= 5 && maker == maker.toUpper()) q.manufacturer = maker;
	client.search(q);
	loop.exec();
	const QString key = orderKey(order);
	for (const DataPortalPart &p : std::as_const(found)) {
		if (orderKey(p.order_number) != key) continue;
		if (part_id) *part_id = p.has_dxf ? p.id : QString();
		if (category) *category = p.product_group;
		if (maker_long) *maker_long = p.manufacturer;
		return true;
	}
	if (error) *error = failure.isEmpty() ? tr("not in the Data Portal") : failure;
	return false;
}

bool PartFetcher::dataPortalPackage(const QString &part_id, const QString &file, QString *error)
{
	const QString token = ManufacturerPortals::dataPortalToken();
	if (token.isEmpty()) {
		if (error) *error = tr("no Data Portal token (Part import > Manufacturer portals)");
		return false;
	}
	DataPortalClient client;
	client.setToken(token);
	QEventLoop loop;
	QByteArray package;
	QString failure;
	connect(&client, &DataPortalClient::downloadFinished, &loop,
		[&](const QByteArray &data, const QString &) { package = data; loop.quit(); });
	connect(&client, &DataPortalClient::failed, &loop,
		[&](const QString &message) { failure = message; loop.quit(); });
	client.downloadDxf(part_id);
	loop.exec();
	if (package.isEmpty()) {
		if (error) *error = failure.isEmpty() ? tr("the download failed") : failure;
		return false;
	}
	QDir().mkpath(QFileInfo(file).absolutePath());
	QSaveFile out(file);
	if (!out.open(QIODevice::WriteOnly) || out.write(package) != package.size() || !out.commit()) {
		if (error) *error = out.errorString();
		return false;
	}
	return true;
}

FetchResult PartFetcher::fetch(const FetchItem &item, const FetchOptions &options)
{
	FetchResult r;
	r.maker = makerName(item.maker);
	const QString order = orderName(r.maker, item.order);
	r.order = order;
	if (r.maker.isEmpty() || order.isEmpty()) {
		r.status = QStringLiteral("failed");
		r.notes << tr("no manufacturer or order number");
		return r;
	}
	const QString collection = options.collection_dir.isEmpty() ? QETApp::companyElementsDir() : options.collection_dir;
	const QString raw_root = options.raw_dir.isEmpty() ? ManufacturerPortals::rawDir() : options.raw_dir;
	const QString model_root = options.model_dir.isEmpty() ? ManufacturerPortals::modelDir() : options.model_dir;
	const QString raw = QDir(raw_root).filePath(safeName(r.maker) + QLatin1Char('/') + order);
	const bool siemens = r.maker == QLatin1String("Siemens");
	const bool keyence = r.maker == QLatin1String("Keyence");
	loadLinks(options);

		// what is there already
	const QString symbol_folder = existingSymbolFolder(collection, safeName(r.maker), order);
	bool has_symbol = !symbol_folder.isEmpty() && !options.force;
	const QString model = PanelStep::findModel(order, r.maker);
	bool has_step = !model.isEmpty() && !options.force;
	if (has_symbol) r.notes << tr("symbol: in the collection (%1)").arg(QDir::toNativeSeparators(symbol_folder));
	if (has_step) r.notes << tr("STEP: there (%1)").arg(QFileInfo(model).fileName());
	if (has_symbol && has_step) {
		r.status = QStringLiteral("present");
		return r;
	}

		// the files of the part: those of the raw folder, then the public links
	QStringList files;
	QStringList planned;	// dry run: the files a download would give
	auto scanRaw = [&]() {
		files.clear();
		const QFileInfoList list = QDir(raw).entryInfoList(QDir::Files, QDir::Name);
		for (const QFileInfo &fi : list) files << fi.absoluteFilePath();
		files << planned;
	};
	scanRaw();
	if (siemens) {
		const QStringList urls = m_links.value(orderKey(order));
		if (urls.isEmpty() && files.isEmpty())
			r.notes << tr("Siemens: no SiePortal links for this part; collect them in the browser "
				      "(tools/herstellerportale/sieportal_links.js) into %1")
				   .arg(QDir::toNativeSeparators(siemensLinkFile(raw_root)));
		for (const QString &url : urls) {
			if (m_cancel) break;
			const QString name = QFileInfo(QUrl(url).path()).fileName();
			const QString suffix = QFileInfo(name).suffix().toLower();
				// only what is still missing: EDZ/DXF for the symbol, STP for 3D
			if (suffix == QLatin1String("stp") ? has_step : has_symbol) continue;
			const QString target = QDir(raw).filePath(name);
			if (QFileInfo::exists(target)) continue;
			if (options.dry_run) {
				r.notes << tr("would load %1").arg(url);
				planned << target;
				continue;
			}
			QString error;
			if (download(url, target, options.pause_ms, &error))
				r.notes << tr("loaded %1 (%2 KB)").arg(name).arg(QFileInfo(target).size() / 1024);
			else
				r.notes << tr("not loaded: %1").arg(error);
		}
		scanRaw();
	}

		// the symbol: EPLAN macro first, then a DXF, then the Data Portal package
	QString category, maker_long, part_id, lookup_error;
	bool looked_up = false;
	if (!has_symbol && options.use_dataportal) {
		looked_up = true;
		dataPortalLookup(item.order.simplified(), item.maker, &part_id, &category, &maker_long, &lookup_error);
	}
	if (!has_symbol) {
		const QString cat = category.isEmpty() ? QStringLiteral("Allgemein") : safeName(category);
		QStringList edz, dxf;
		for (const QString &f : std::as_const(files)) {
			const QString s = QFileInfo(f).suffix().toLower();
			if (s == QLatin1String("edz")) edz << f;
			else if (s == QLatin1String("dxf")) dxf << f;
		}
		if (options.dry_run) {
			if (!edz.isEmpty()) r.notes << tr("would import the EPLAN macro into %1").arg(cat);
			else if (!dxf.isEmpty()) r.notes << tr("would import a DXF into %1").arg(cat);
			else if (!part_id.isEmpty()) r.notes << tr("would load the Data Portal package into %1").arg(cat);
		} else {
			EplanPartImporter importer;
			for (const QString &f : std::as_const(edz)) {
				if (importer.importEdz(f, safeName(r.maker), cat, order, collection)) {
					r.written << importer.writtenPaths();
					r.notes << tr("symbol: EPLAN macro %1 imported into %2").arg(QFileInfo(f).fileName(), cat);
					has_symbol = true;
					break;
				}
				r.notes << tr("%1: %2").arg(QFileInfo(f).fileName(), importer.errorString());
			}
			for (const QString &f : std::as_const(dxf)) {
				if (has_symbol) break;
				if (importer.importDxf(f, QString(), safeName(r.maker), cat, order, collection)) {
					r.written << importer.writtenPaths();
					r.notes << tr("symbol: DXF %1 imported into %2").arg(QFileInfo(f).fileName(), cat);
					has_symbol = true;
				}
			}
			if (!has_symbol && !part_id.isEmpty()) {
				const QString package = QDir(raw).filePath(QStringLiteral("dataportal_%1.zip").arg(part_id));
				QString error;
				if (QFileInfo::exists(package) || dataPortalPackage(part_id, package, &error)) {
					if (importer.importPackage(package, cat, collection)) {
						r.written << importer.writtenPaths();
						r.notes << tr("symbol: Data Portal package imported into %1").arg(cat);
						has_symbol = true;
					} else {
						r.notes << tr("Data Portal package: %1").arg(importer.errorString());
					}
				} else {
					r.notes << tr("Data Portal: %1").arg(error);
				}
			}
			if (!has_symbol && looked_up && part_id.isEmpty())
				r.notes << (lookup_error.isEmpty() ? tr("Data Portal: no DXF data for this part")
								  : tr("Data Portal: %1").arg(lookup_error));
		}
	}

		// the STEP model: the largest one, not a placeholder box
	if (!has_step) {
			// a CAD download saved as ZIP (keyence.de and others)
		const QStringList unpacked = unpackStepArchives(raw, options.dry_run, &r.notes);
		if (!unpacked.isEmpty()) {
			if (options.dry_run) planned << unpacked;
			scanRaw();
		}
		QString best;
		for (const QString &f : std::as_const(files)) {
			if (isStep(f) && (best.isEmpty() || QFileInfo(f).size() > QFileInfo(best).size()))
				best = f;
		}
		if (!best.isEmpty() && QFileInfo::exists(best) && QFileInfo(best).size() < PLACEHOLDER_STEP) {
			r.notes << tr("STEP: %1 is only a placeholder (%2 KB), not taken")
				   .arg(QFileInfo(best).fileName()).arg(QFileInfo(best).size() / 1024);
			best.clear();
		}
		if (!best.isEmpty()) {
			const QString target = QDir(model_root).filePath(safeName(r.maker) + QLatin1Char('/') + order
									 + QStringLiteral(".stp"));
			if (options.dry_run) {
				r.notes << tr("would store the STEP model as %1").arg(QDir::toNativeSeparators(target));
			} else {
				QDir().mkpath(QFileInfo(target).absolutePath());
				QFile::remove(target);
				if (QFile::copy(best, target)) {
					r.step_file = QDir::toNativeSeparators(target);
					r.written << target;
					r.notes << tr("STEP: stored as %1").arg(QDir::toNativeSeparators(target));
					has_step = true;
				} else {
					r.notes << tr("STEP: %1 cannot be written").arg(QDir::toNativeSeparators(target));
				}
			}
		} else if (keyence) {
				// keyence.de: login and download in the browser only
			if (!options.dry_run) QDir().mkpath(raw);
			r.notes << tr("STEP: log in on keyence.de in the browser, load the CAD data (STEP) of %1 (%2) "
				      "and save the file (ZIP or STEP) into %3")
				   .arg(order, ManufacturerPortals::productUrl(QStringLiteral("keyence"), order),
					QDir::toNativeSeparators(raw));
		} else if (on3Dfindit(r.maker)) {
			r.notes << tr("STEP: request it on 3Dfindit in the browser (%1 of %2 requests left today)")
				   .arg(ManufacturerPortals::requestsLeft(QStringLiteral("3dfindit")))
				   .arg(ManufacturerPortals::portal(QStringLiteral("3dfindit")).daily_limit);
		}
	}

	if (options.dry_run) r.status = QStringLiteral("planned");
	else if (has_symbol && has_step) r.status = r.written.isEmpty() ? QStringLiteral("present") : QStringLiteral("done");
	else if (has_symbol || has_step) r.status = QStringLiteral("partial");
	else r.status = QStringLiteral("missing");
	return r;
}

QList<FetchResult> PartFetcher::run(const QList<FetchItem> &items, const FetchOptions &options)
{
	m_cancel = false;
	QList<FetchItem> todo;
	for (const FetchItem &item : items) {
		if (!options.makers.isEmpty()) {
			bool wanted = false;
			for (const QString &m : options.makers)
				if (m.compare(item.maker, Qt::CaseInsensitive) == 0
				    || m.compare(makerName(item.maker), Qt::CaseInsensitive) == 0)
					wanted = true;
			if (!wanted) continue;
		}
		todo << item;
		if (options.limit > 0 && todo.size() >= options.limit) break;
	}
		// the STEP models of the 3D folder given are searched too
	const QByteArray old_3d = qgetenv("QET_3D_DIR");
	if (!options.model_dir.isEmpty())
		qputenv("QET_3D_DIR", QDir::toNativeSeparators(options.model_dir).toLocal8Bit());
	QList<FetchResult> results;
	for (int i = 0; i < todo.size() && !m_cancel; ++i) {
		const FetchResult r = fetch(todo.at(i), options);
		results << r;
		emit progress(i + 1, int(todo.size()), QStringLiteral("%1 %2: %3").arg(r.maker, r.order, r.status));
	}
	if (!options.model_dir.isEmpty()) {
		if (old_3d.isEmpty()) qunsetenv("QET_3D_DIR");
		else qputenv("QET_3D_DIR", old_3d);
	}
	return results;
}

QJsonObject PartFetcher::toJson(const QList<FetchResult> &results)
{
	QJsonArray list;
	QHash<QString, int> count;
	for (const FetchResult &r : results) {
		list.append(r.toJson());
		++count[r.status];
	}
	QJsonObject summary;
	for (auto it = count.constBegin(); it != count.constEnd(); ++it) summary.insert(it.key(), it.value());
	QJsonObject o;
	o.insert(QStringLiteral("parts"), list);
	o.insert(QStringLiteral("summary"), summary);
	o.insert(QStringLiteral("3dfindit_left"), ManufacturerPortals::requestsLeft(QStringLiteral("3dfindit")));
	return o;
}
