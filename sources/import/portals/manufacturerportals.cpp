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
#include "manufacturerportals.h"

#include "portalcredentials.h"
#include "../../qetapp.h"
#include "../eplandxf/dataportalclient.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>

#include <algorithm>

namespace {
	const char LEGACY_TOKEN_KEY[] = "eplanimport/dataportal_token";
	const int DAY_SECONDS = 24 * 3600;
}

QList<ManufacturerPortal> ManufacturerPortals::portals()
{
	QList<ManufacturerPortal> list;
	ManufacturerPortal dp;
	dp.id = QStringLiteral("dataportal");
	dp.name = tr("EPLAN Data Portal");
	dp.login_url = QStringLiteral("https://dataportal.eplan.com");
	dp.secret_label = tr("Personal Access Token");
	dp.access = tr("Official API: search and DXF packages with part data (Siemens, Rittal and many more). "
		       "The token comes from your ePulse profile.");
	dp.api_login = true;
	list << dp;

	ManufacturerPortal sie;
	sie.id = QStringLiteral("siemens");
	sie.name = tr("Siemens SiePortal");
	sie.login_url = QStringLiteral("https://sieportal.siemens.com");
	sie.secret_label = tr("Password");
	sie.access = tr("Public files of static.siemens.com (EPLAN macro, DXF, STEP), from the links of the "
			"SiePortal product page. SiePortal refuses programs and has no CAx API: the links come "
			"from the browser (link file). The password stays in the store, QElectroTech does not send it.");
	list << sie;

	ManufacturerPortal rit;
	rit.id = QStringLiteral("rittal");
	rit.name = tr("Rittal");
	rit.login_url = QStringLiteral("https://www.rittal.com");
	rit.secret_label = tr("Password");
	rit.access = tr("No API of its own: the EPLAN data of Rittal comes from the Data Portal. "
			"The password stays in the store, QElectroTech does not send it.");
	list << rit;

	ManufacturerPortal cad;
	cad.id = QStringLiteral("3dfindit");
	cad.name = tr("3Dfindit");
	cad.login_url = QStringLiteral("https://www.3dfindit.com");
	cad.secret_label = tr("Password");
	cad.access = tr("No API for users: the part opens in the browser, QElectroTech counts the CAD requests "
			"(50 per 24 h and account) and takes the downloaded STEP files into the 3D folder.");
	cad.daily_limit = 50;
	list << cad;

	ManufacturerPortal key;
	key.id = QStringLiteral("keyence");
	key.name = tr("Keyence");
	key.login_url = QStringLiteral("https://www.keyence.de/user/login/loginWithRegist/");
	key.secret_label = tr("Password");
	key.access = tr("CAD data (STEP) only for logged-in users of keyence.de, not in the Data Portal or on "
			"3Dfindit. Keyence has no API and logs in through its web page only: log in and load the CAD "
			"data in the browser, save the file (ZIP or STEP) into herstellerdaten/Keyence/<order number>; "
			"\"Fetch missing parts\" stores it as 3D/Keyence/<order number>.stp. "
			"The password stays in the store, QElectroTech does not send it.");
	list << key;
	return list;
}

ManufacturerPortal ManufacturerPortals::portal(const QString &id)
{
	const QList<ManufacturerPortal> list = portals();
	for (const ManufacturerPortal &p : list)
		if (p.id == id) return p;
	return ManufacturerPortal();
}

QString ManufacturerPortals::dataDir()
{
	const QString env = qEnvironmentVariable("QET_PORTAL_DATA_DIR");
	if (!env.isEmpty()) return QDir::fromNativeSeparators(env);
	const QString company = QETApp::companyElementsDir();
	if (company.isEmpty()) return QString();
	QDir d(company);
	if (!d.cdUp()) return QString();
	return d.absolutePath();
}

QString ManufacturerPortals::rawDir()
{
	const QString data = dataDir();
	return data.isEmpty() ? QString() : QDir(data).filePath(QStringLiteral("herstellerdaten"));
}

QString ManufacturerPortals::modelDir()
{
	const QString env = qEnvironmentVariable("QET_3D_DIR");
	if (!env.isEmpty()) return QDir::fromNativeSeparators(env);
	const QString data = dataDir();
	return data.isEmpty() ? QString() : QDir(data).filePath(QStringLiteral("3D"));
}

QString ManufacturerPortals::dataPortalToken()
{
	const PortalAccount account = PortalCredentials::load(QStringLiteral("dataportal"));
	if (!account.secret.isEmpty()) return account.secret;
		// the token of older versions, kept in clear in the settings:
		// moved into the credential store and removed from the settings
	QSettings settings;
	const QString legacy = settings.value(QLatin1String(LEGACY_TOKEN_KEY)).toString().trimmed();
	if (legacy.isEmpty()) return QString();
	if (PortalCredentials::available()
	    && PortalCredentials::store(QStringLiteral("dataportal"), PortalAccount{QString(), legacy}))
		settings.remove(QLatin1String(LEGACY_TOKEN_KEY));
	return legacy;
}

QString ManufacturerPortals::checkDataPortalToken(const QString &token)
{
	if (token.trimmed().isEmpty()) return tr("No token.");
	DataPortalClient base;	// for the base URL (QET_DATAPORTAL_URL in tests)
	QNetworkAccessManager manager;
	QNetworkRequest req(QUrl(base.baseUrl() + QStringLiteral("/basketitems?page%5Blimit%5D=1")));
	req.setTransferTimeout(30000);
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("QElectroTech"));
	req.setRawHeader("Accept", "application/vnd.api+json");
	req.setRawHeader("Authorization", DataPortalClient::authorization(token));
	QNetworkReply *reply = manager.get(req);
	QEventLoop loop;
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	const QString net_error = reply->errorString();
	reply->deleteLater();
	if (status >= 200 && status < 300) return QString();
	if (status == 401 || status == 403)
		return tr("The Data Portal does not accept the token (HTTP %1).").arg(status);
	if (status == 0) return tr("The Data Portal cannot be reached: %1").arg(net_error);
	return tr("The Data Portal answers HTTP %1.").arg(status);
}

bool ManufacturerPortals::loggedIn(const QString &id)
{
	if (id == QLatin1String("dataportal")) return !dataPortalToken().isEmpty();
	return PortalCredentials::has(id);
}

QString ManufacturerPortals::user(const QString &id)
{
	return PortalCredentials::load(id).user;
}

QJsonObject ManufacturerPortals::status(const QString &id)
{
	const ManufacturerPortal p = portal(id);
	QJsonObject o;
	if (p.id.isEmpty()) return o;
	o.insert(QStringLiteral("id"), p.id);
	o.insert(QStringLiteral("name"), p.name);
	o.insert(QStringLiteral("logged_in"), loggedIn(id));
	o.insert(QStringLiteral("user"), user(id));
	o.insert(QStringLiteral("api_login"), p.api_login);
	o.insert(QStringLiteral("access"), p.access);
	if (p.daily_limit > 0) {
		o.insert(QStringLiteral("limit"), p.daily_limit);
		o.insert(QStringLiteral("used"), requestsUsed(id));
		o.insert(QStringLiteral("left"), requestsLeft(id));
		const QDateTime next = nextFreeRequest(id);
		if (next.isValid()) o.insert(QStringLiteral("next_free"), next.toString(Qt::ISODate));
	}
	return o;
}

QJsonObject ManufacturerPortals::statusAll()
{
	QJsonArray list;
	const QList<ManufacturerPortal> all = portals();
	for (const ManufacturerPortal &p : all) list.append(status(p.id));
	QJsonObject o;
	o.insert(QStringLiteral("portals"), list);
	o.insert(QStringLiteral("credential_store"), PortalCredentials::available());
	o.insert(QStringLiteral("data_dir"), QDir::toNativeSeparators(dataDir()));
	o.insert(QStringLiteral("model_dir"), QDir::toNativeSeparators(modelDir()));
	return o;
}

QString ManufacturerPortals::requestLog(const QString &id)
{
	const QString raw = rawDir();
	return raw.isEmpty() ? QString() : QDir(raw).filePath(id + QStringLiteral("_anforderungen.log"));
}

/// the requests of the last 24 h, oldest first
QList<QDateTime> ManufacturerPortals::requests(const QString &id)
{
	QList<QDateTime> list;
	QFile f(requestLog(id));
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return list;
	const QDateTime since = QDateTime::currentDateTimeUtc().addSecs(-DAY_SECONDS);
	const QStringList lines = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
	for (const QString &line : lines) {
		const QDateTime t = QDateTime::fromString(line.section(QLatin1Char('\t'), 0, 0).trimmed(), Qt::ISODate);
		if (t.isValid() && t > since) list << t;
	}
	std::sort(list.begin(), list.end());
	return list;
}

int ManufacturerPortals::requestsUsed(const QString &id)
{
	return int(requests(id).size());
}

int ManufacturerPortals::requestsLeft(const QString &id)
{
	const int limit = portal(id).daily_limit;
	return limit > 0 ? qMax(0, limit - requestsUsed(id)) : -1;
}

QDateTime ManufacturerPortals::nextFreeRequest(const QString &id)
{
	const QList<QDateTime> list = requests(id);
	return list.isEmpty() ? QDateTime() : list.first().addSecs(DAY_SECONDS).toLocalTime();
}

bool ManufacturerPortals::registerRequest(const QString &id, const QString &order, QString *error)
{
	const ManufacturerPortal p = portal(id);
	if (p.daily_limit <= 0) return true;
	if (requestsLeft(id) <= 0) {
		if (error)
			*error = tr("The limit of %1 CAD requests per 24 h is reached; the next one is free at %2.")
				 .arg(p.daily_limit).arg(QLocale().toString(nextFreeRequest(id), QLocale::ShortFormat));
		return false;
	}
	const QString log = requestLog(id);
	if (log.isEmpty() || !QDir().mkpath(QFileInfo(log).absolutePath())) {
		if (error) *error = tr("No data folder for the request log.");
		return false;
	}
	QFile f(log);
	if (!f.open(QIODevice::Append | QIODevice::Text)) {
		if (error) *error = tr("The request log cannot be written: %1").arg(f.errorString());
		return false;
	}
	f.write((QDateTime::currentDateTimeUtc().toString(Qt::ISODate) + QLatin1Char('\t')
		 + order.simplified() + QLatin1Char('\n')).toUtf8());
	return true;
}

QString ManufacturerPortals::productUrl(const QString &id, const QString &order)
{
	QString compact = order.simplified();
	compact.remove(QLatin1Char(' '));
	const QString o = QString::fromLatin1(QUrl::toPercentEncoding(compact));
	if (id == QLatin1String("siemens"))
		return QStringLiteral("https://sieportal.siemens.com/de-de/products-services/detail/") + o;
	if (id == QLatin1String("keyence"))
		return QStringLiteral("https://www.keyence.de/search/all/?q=") + o;
	return portal(id).login_url;
}
