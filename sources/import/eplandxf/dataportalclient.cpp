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
#include "dataportalclient.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

#include <algorithm>

namespace {
	const char DEFAULT_URL[] = "https://dataportal.eplan.com/api";
	const int TIMEOUT_MS = 60000;
	const int SEARCH_TIMEOUT_MS = 20000;   ///< a search should not keep the user waiting

	/// the server's own words of a JSON:API error reply, if any
	QString serverDetail(const QByteArray &body)
	{
		const QJsonArray errors = QJsonDocument::fromJson(body).object()
					  .value(QStringLiteral("errors")).toArray();
		if (errors.isEmpty()) return QString();
		const QJsonObject e = errors.first().toObject();
		const QString detail = e.value(QStringLiteral("detail")).toString();
		return detail.isEmpty() ? e.value(QStringLiteral("title")).toString() : detail;
	}

	/// text of a {"de_DE": ..., "en_US": ...} object in @p language
	QString localized(const QJsonObject &texts, const QString &language)
	{
		if (texts.contains(language)) return texts.value(language).toString();
		const QString prefix = language.left(2) + QLatin1Char('_');
		for (auto it = texts.begin(); it != texts.end(); ++it)
			if (it.key().startsWith(prefix)) return it.value().toString();
		if (texts.contains(QStringLiteral("en_US"))) return texts.value(QStringLiteral("en_US")).toString();
		return texts.isEmpty() ? QString() : texts.begin().value().toString();
	}

	QString relationId(const QJsonObject &relationships, const QString &name)
	{
		return relationships.value(name).toObject().value(QStringLiteral("data"))
				.toObject().value(QStringLiteral("id")).toString();
	}
}

DataPortalClient::DataPortalClient(QObject *parent) :
	QObject(parent),
	m_manager(new QNetworkAccessManager(this))
{
	const QString env = qEnvironmentVariable("QET_DATAPORTAL_URL");
	setBaseUrl(env.isEmpty() ? QString::fromLatin1(DEFAULT_URL) : env);
}

DataPortalClient::~DataPortalClient()
{
	abort();
}

void DataPortalClient::setBaseUrl(const QString &url)
{
	m_base = url;
	while (m_base.endsWith(QLatin1Char('/'))) m_base.chop(1);
}

/**
	The Authorization header for @p token. A Personal Access Token goes as
	"Bearer PAT:<token>"; a short-lived login token (a JWT, three parts
	joined by dots) as "Bearer <token>". A pasted "Bearer " or "PAT:" in
	front is taken off first.
*/
QByteArray DataPortalClient::authorization(const QString &token)
{
	QString t = token.trimmed();
	if (t.startsWith(QLatin1String("Bearer "), Qt::CaseInsensitive)) t = t.mid(7).trimmed();
	bool pat = true;
	if (t.startsWith(QLatin1String("PAT:"), Qt::CaseInsensitive)) {
		t = t.mid(4).trimmed();
	} else if (t.startsWith(QLatin1String("eyJ")) && t.count(QLatin1Char('.')) == 2) {
		pat = false;
	}
	return (pat ? QByteArrayLiteral("Bearer PAT:") : QByteArrayLiteral("Bearer ")) + t.toUtf8();
}

QNetworkRequest DataPortalClient::request(const QString &path) const
{
	QNetworkRequest req(QUrl(m_base + path));
	req.setTransferTimeout(TIMEOUT_MS);
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("QElectroTech"));
	req.setRawHeader("Accept-Language", m_language.toUtf8());
	if (!m_token.isEmpty()) req.setRawHeader("Authorization", authorization(m_token));
	return req;
}

void DataPortalClient::search(const DataPortalQuery &q)
{
	abort();
	QUrlQuery query;
	const QString text = q.text.trimmed();
	if (!text.isEmpty()) query.addQueryItem(QStringLiteral("search"), text.left(100));
	if (!q.manufacturer.trimmed().isEmpty())
		query.addQueryItem(QStringLiteral("filter[manufacturer]"), q.manufacturer.trimmed());
	if (q.only_dxf) query.addQueryItem(QStringLiteral("filter[data_types]"), QStringLiteral("dxf_data"));
	if (q.exact && !text.isEmpty()) query.addQueryItem(QStringLiteral("fuzziness"), QStringLiteral("0"));
	query.addQueryItem(QStringLiteral("include"),
			   QStringLiteral("manufacturer,product_group,picture_file.preview"));
	query.addQueryItem(QStringLiteral("page[offset]"), QString::number(q.offset));
	query.addQueryItem(QStringLiteral("page[limit]"), QString::number(q.limit));
	QNetworkRequest req = request(QStringLiteral("/parts"));
	req.setTransferTimeout(SEARCH_TIMEOUT_MS);
	QUrl url = req.url();
	url.setQuery(query);
	req.setUrl(url);
	req.setRawHeader("Accept", "application/vnd.api+json, application/json");
	QNetworkReply *reply = m_manager->get(req);
	m_reply = reply;
	const int offset = q.offset;
	connect(reply, &QNetworkReply::finished, this, [this, reply, offset]() { finished(reply, false, offset); });
}

void DataPortalClient::downloadDxf(const QString &part_id)
{
	abort();
	QNetworkRequest req = request(QStringLiteral("/download/dxf_data/part/")
				      + QString::fromUtf8(QUrl::toPercentEncoding(part_id)));
	QNetworkReply *reply = m_manager->get(req);
	m_reply = reply;
	connect(reply, &QNetworkReply::finished, this, [this, reply]() { finished(reply, true, 0); });
}

/// a failure here is not reported: the filter then stays a text field
void DataPortalClient::loadManufacturers()
{
	QNetworkRequest req = request(QStringLiteral("/manufacturers"));
	req.setRawHeader("Accept", "application/vnd.api+json, application/json");
	QNetworkReply *reply = m_manager->get(req);
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		if (reply->error() != QNetworkReply::NoError) return;
		emit manufacturersLoaded(parseManufacturers(reply->readAll()));
	});
}

/// the pictures lie on a public file store: sent without the token
void DataPortalClient::loadPicture(const QString &url)
{
	if (!m_picture_reply.isNull()) {
		QNetworkReply *old = m_picture_reply;
		old->disconnect(this);
		old->abort();
		old->deleteLater();
	}
	QNetworkRequest req{QUrl(url)};
	req.setTransferTimeout(TIMEOUT_MS);
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("QElectroTech"));
	QNetworkReply *reply = m_manager->get(req);
	m_picture_reply = reply;
	connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
		reply->deleteLater();
		if (reply->error() != QNetworkReply::NoError) return;
		emit pictureLoaded(url, reply->readAll());
	});
}

/// cancel the running request without a signal
void DataPortalClient::abort()
{
	if (m_reply.isNull()) return;
	QNetworkReply *reply = m_reply;
	m_reply.clear();
	reply->disconnect(this);
	reply->abort();
	reply->deleteLater();
}

void DataPortalClient::finished(QNetworkReply *reply, bool download, int offset)
{
	reply->deleteLater();
	if (m_reply == reply) m_reply.clear();
	const QByteArray body = reply->readAll();
	const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	if (reply->error() != QNetworkReply::NoError || status >= 400) {
		emit failed(errorText(reply, body, download));
		return;
	}
	if (!download) {
		int total = 0;
		QString error;
		const QVector<DataPortalPart> parts = parseParts(body, m_language, &total, &error);
		if (!error.isEmpty()) {
			emit failed(tr("The reply of the Data Portal cannot be read: %1").arg(error));
			return;
		}
		emit searchFinished(parts, total, offset);
		return;
	}
	// a wrong URL may answer with a web page instead of the package
	if (!body.startsWith("PK")) {
		emit failed(tr("The Data Portal sent no ZIP package."));
		return;
	}
	QString name;
	static const QRegularExpression file_name(
		QStringLiteral("filename\\*?=(?:UTF-8'')?\"?([^\";]+)\"?"),
		QRegularExpression::CaseInsensitiveOption);
	const auto m = file_name.match(QString::fromUtf8(reply->rawHeader("Content-Disposition")));
	if (m.hasMatch()) name = QUrl::fromPercentEncoding(m.captured(1).toUtf8());
	// only the file name, never a path the server names
	name = name.section(QLatin1Char('/'), -1).section(QLatin1Char('\\'), -1);
	if (name.isEmpty() || !name.endsWith(QLatin1String(".zip"), Qt::CaseInsensitive))
		name = QStringLiteral("MAC_%1.zip").arg(reply->url().fileName());
	emit downloadFinished(body, name);
}

QString DataPortalClient::errorText(QNetworkReply *reply, const QByteArray &body, bool download) const
{
	const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	const QString detail = serverDetail(body);
	switch (status) {
		case 0:
			if (reply->error() == QNetworkReply::OperationCanceledError
			    || reply->error() == QNetworkReply::TimeoutError)
				return tr("The EPLAN Data Portal did not answer in time. "
					  "Check the internet connection and try again.");
			return tr("The EPLAN Data Portal cannot be reached. Check the internet "
				  "connection or the proxy.\n(%1)").arg(reply->errorString());
		case 401:
			if (m_token.isEmpty())
				return tr("Downloading needs your Personal Access Token of the Data Portal. "
					  "Enter it in the field \"Token\".");
			return tr("The Data Portal did not accept the token. Check that it is a "
				  "Personal Access Token made for the Data Portal and that it has "
				  "not expired.");
		case 403:
			return tr("The token has no right for this. A token of the ePulse Community "
				  "does not work: it must belong to an organisation.");
		case 404:
			if (download)
				return tr("The Data Portal has no DXF package for this part: it has no DXF "
					  "data, or the manufacturer does not allow DXF downloads.");
			return tr("The Data Portal does not know this request (HTTP 404).");
		case 429: {
			const int wait = reply->rawHeader("Retry-After").toInt();
			if (wait > 0)
				return tr("The Data Portal limits the number of requests and the limit is "
					  "reached. Try again in %n second(s).", "", wait);
			return tr("The Data Portal limits the number of requests and the limit is "
				  "reached. Try again later.");
		}
		default:
			break;
	}
	if (status == 400 && !detail.isEmpty())
		return tr("The Data Portal refused the request: %1").arg(detail);
	if (status >= 500)
		return tr("The Data Portal reported an error (HTTP %1). It may be in "
			  "maintenance; try again later.").arg(status);
	return tr("The Data Portal reported an error (HTTP %1): %2")
			.arg(status).arg(detail.isEmpty() ? reply->errorString() : detail);
}

/**
	The parts of a JSON:API reply of GET /parts?include=manufacturer,product_group.
	@param total number of matching parts, more than returned on one page
	@param error set when @p json is no such reply
*/
QVector<DataPortalPart> DataPortalClient::parseParts(const QByteArray &json, const QString &language,
						     int *total, QString *error)
{
	QJsonParseError parse;
	const QJsonDocument doc = QJsonDocument::fromJson(json, &parse);
	if (parse.error != QJsonParseError::NoError || !doc.isObject()
	    || !doc.object().value(QStringLiteral("data")).isArray()) {
		if (error) *error = parse.error != QJsonParseError::NoError
				    ? parse.errorString() : QStringLiteral("no \"data\" list");
		return {};
	}
	const QJsonObject root = doc.object();
	// included objects by "type/id"
	QHash<QString, QJsonObject> objects;
	for (const QJsonValue &v : root.value(QStringLiteral("included")).toArray()) {
		const QJsonObject o = v.toObject();
		objects.insert(o.value(QStringLiteral("type")).toString() + QLatin1Char('/')
			       + o.value(QStringLiteral("id")).toString(), o);
	}
	QHash<QString, QJsonObject> included;   // their attributes
	for (auto it = objects.cbegin(); it != objects.cend(); ++it)
		included.insert(it.key(), it.value().value(QStringLiteral("attributes")).toObject());

	QVector<DataPortalPart> parts;
	for (const QJsonValue &v : root.value(QStringLiteral("data")).toArray()) {
		const QJsonObject o = v.toObject();
		const QJsonObject a = o.value(QStringLiteral("attributes")).toObject();
		const QJsonObject rel = o.value(QStringLiteral("relationships")).toObject();
		DataPortalPart p;
		const QJsonValue id = o.value(QStringLiteral("id"));
		p.id = id.isString() ? id.toString() : QString::number(id.toInteger());
		p.part_number = a.value(QStringLiteral("part_number")).toString();
		p.order_number = a.value(QStringLiteral("order_number")).toString();
		p.type_number = a.value(QStringLiteral("type_number")).toString();
		p.designation = localized(a.value(QStringLiteral("designation")).toObject(), language);
		p.description = localized(a.value(QStringLiteral("description")).toObject(), language);
		const QJsonObject man = included.value(
			QStringLiteral("manufacturers/") + relationId(rel, QStringLiteral("manufacturer")));
		p.manufacturer = man.value(QStringLiteral("long_name")).toString();
		p.manufacturer_short = man.value(QStringLiteral("short_name")).toString();
		if (p.manufacturer_short.isEmpty())
			p.manufacturer_short = a.value(QStringLiteral("data_supplier")).toString();
		if (p.manufacturer_short.isEmpty()) p.manufacturer_short = p.part_number.section(QLatin1Char('.'), 0, 0);
		if (p.manufacturer.isEmpty()) p.manufacturer = p.manufacturer_short;
		p.product_group = included.value(
			QStringLiteral("eplan_catalog_entries/") + relationId(rel, QStringLiteral("product_group")))
				.value(QStringLiteral("name")).toString();
		// picture: part -> picturefile -> preview {"96": url, "512": url}
		const QJsonObject picture = objects.value(
			QStringLiteral("picturefile/") + relationId(rel, QStringLiteral("picture_file")));
		const QJsonObject preview = included.value(
			QStringLiteral("preview/") + relationId(picture.value(QStringLiteral("relationships")).toObject(),
								QStringLiteral("preview")));
		p.picture_url = preview.value(QStringLiteral("96")).toString();
		if (p.picture_url.isEmpty()) p.picture_url = preview.value(QStringLiteral("512")).toString();
		for (const QJsonValue &t : rel.value(QStringLiteral("data_types")).toObject()
					       .value(QStringLiteral("data")).toArray()) {
			if (t.toObject().value(QStringLiteral("id")).toString() == QLatin1String("dxf_data"))
				p.has_dxf = true;
		}
		if (!p.id.isEmpty()) parts.append(p);
	}
	if (total) {
		*total = root.value(QStringLiteral("meta")).toObject().value(QStringLiteral("page"))
				 .toObject().value(QStringLiteral("total")).toInt(int(parts.size()));
	}
	return parts;
}

/**
	The manufacturers of a reply of GET /manufacturers, sorted by long name.
	The Data Portal lists some twice, once for NFPA data; both are kept.
*/
QVector<DataPortalManufacturer> DataPortalClient::parseManufacturers(const QByteArray &json)
{
	QVector<DataPortalManufacturer> list;
	for (const QJsonValue &v : QJsonDocument::fromJson(json).object()
				       .value(QStringLiteral("data")).toArray()) {
		const QJsonObject a = v.toObject().value(QStringLiteral("attributes")).toObject();
		DataPortalManufacturer m;
		m.short_name = a.value(QStringLiteral("short_name")).toString();
		m.long_name = a.value(QStringLiteral("long_name")).toString();
		m.allow_dxf = a.value(QStringLiteral("allow_dxf")).toBool();
		if (m.short_name.isEmpty()) continue;
		if (m.long_name.isEmpty()) m.long_name = m.short_name;
		list.append(m);
	}
	std::sort(list.begin(), list.end(), [](const DataPortalManufacturer &x, const DataPortalManufacturer &y) {
		return x.long_name.compare(y.long_name, Qt::CaseInsensitive) < 0;
	});
	return list;
}
