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
#ifndef DATAPORTALCLIENT_H
#define DATAPORTALCLIENT_H

#include <QByteArray>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

/// one part of a search in the EPLAN Data Portal
struct DataPortalPart
{
	QString id;            ///< Data Portal part id, used for the download
	QString part_number;   ///< e.g. SIE.3RT2015-1BB41
	QString order_number;
	QString type_number;
	QString manufacturer;  ///< long name, e.g. Siemens
	QString manufacturer_short;
	QString designation;   ///< in the language asked for, else English
	QString product_group;
	QString picture_url;   ///< product picture, public, may be empty
	bool has_dxf = false;
};

/// one manufacturer of the Data Portal
struct DataPortalManufacturer
{
	QString short_name;    ///< filter value, e.g. WAGO
	QString long_name;
	bool allow_dxf = false;
};

/// what to search for
struct DataPortalQuery
{
	QString text;
	QString manufacturer;  ///< short name, empty: all
	bool only_dxf = false; ///< only parts with a DXF package
	bool exact = false;    ///< no fuzzy matching
	int offset = 0;
	int limit = 50;
};

/**
	@brief Client of the EPLAN Data Portal API (https://dataportal.eplan.com/api).

	Searching parts needs no login. Downloading the DXF package of a part
	needs the user's Personal Access Token (PAT) from the ePulse profile,
	sent as "Authorization: Bearer PAT:<token>". The base URL can be set
	by the environment variable QET_DATAPORTAL_URL, for tests against a
	local stub.

	One request at a time; a new one cancels the running one.
*/
class DataPortalClient : public QObject
{
	Q_OBJECT

	public:
		explicit DataPortalClient(QObject *parent = nullptr);
		~DataPortalClient() override;

		void setToken(const QString &token) { m_token = token.trimmed(); }
		void setLanguage(const QString &locale) { m_language = locale; }
		QString baseUrl() const { return m_base; }
		void setBaseUrl(const QString &url);

		/// parts whose order, type or part number or text match the query
		void search(const DataPortalQuery &query);
		/// the DXF package (MAC_<part>.zip) of a part; needs the token
		void downloadDxf(const QString &part_id);
		/// all manufacturers, for the filter; no token needed
		void loadManufacturers();
		/// a product picture; a newer request replaces a running one
		void loadPicture(const QString &url);
		void abort();
		bool isBusy() const { return !m_reply.isNull(); }

		/// the parts of a JSON:API reply of GET /parts
		static QVector<DataPortalPart> parseParts(const QByteArray &json, const QString &language,
							  int *total = nullptr, QString *error = nullptr);
		/// the manufacturers of a reply of GET /manufacturers, by long name
		static QVector<DataPortalManufacturer> parseManufacturers(const QByteArray &json);
		/// the value of the Authorization header for @p token
		static QByteArray authorization(const QString &token);

	signals:
		/// @p offset is the one of the query: 0 for a new search
		void searchFinished(const QVector<DataPortalPart> &parts, int total, int offset);
		void downloadFinished(const QByteArray &data, const QString &file_name);
		void manufacturersLoaded(const QVector<DataPortalManufacturer> &manufacturers);
		void pictureLoaded(const QString &url, const QByteArray &data);
		void failed(const QString &message);

	private:
		QNetworkRequest request(const QString &path) const;
		void finished(QNetworkReply *reply, bool download, int offset);
		QString errorText(QNetworkReply *reply, const QByteArray &body, bool download) const;

		QNetworkAccessManager *m_manager = nullptr;
		QPointer<QNetworkReply> m_reply;          ///< search or download
		QPointer<QNetworkReply> m_picture_reply;
		QString m_base;
		QString m_token;
		QString m_language = QStringLiteral("en_US");
};

#endif // DATAPORTALCLIENT_H
