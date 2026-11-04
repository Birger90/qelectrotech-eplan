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
	bool has_dxf = false;
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

		/// parts whose order, type or part number or text match @p text
		void search(const QString &text, const QString &manufacturer, int limit = 50);
		/// the DXF package (MAC_<part>.zip) of a part; needs the token
		void downloadDxf(const QString &part_id);
		void abort();
		bool isBusy() const { return !m_reply.isNull(); }

		/// the parts of a JSON:API reply of GET /parts
		static QVector<DataPortalPart> parseParts(const QByteArray &json, const QString &language,
							  int *total = nullptr, QString *error = nullptr);
		/// the value of the Authorization header for @p token
		static QByteArray authorization(const QString &token);

	signals:
		void searchFinished(const QVector<DataPortalPart> &parts, int total);
		void downloadFinished(const QByteArray &data, const QString &file_name);
		void failed(const QString &message);

	private:
		QNetworkRequest request(const QString &path) const;
		void finished(QNetworkReply *reply, bool download);
		QString errorText(QNetworkReply *reply, const QByteArray &body, bool download) const;

		QNetworkAccessManager *m_manager = nullptr;
		QPointer<QNetworkReply> m_reply;
		QString m_base;
		QString m_token;
		QString m_language = QStringLiteral("en_US");
};

#endif // DATAPORTALCLIENT_H
