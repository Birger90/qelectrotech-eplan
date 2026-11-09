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
#ifndef MANUFACTURERPORTALS_H
#define MANUFACTURERPORTALS_H

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QStringList>

/// a manufacturer portal QElectroTech knows
struct ManufacturerPortal
{
	QString id;            ///< dataportal, siemens, rittal, 3dfindit
	QString name;          ///< shown name
	QString login_url;     ///< the portal's own login page, for the browser
	QString secret_label;  ///< "Password" or "Personal Access Token"
	QString access;        ///< what QElectroTech does with the portal
	bool api_login = false;///< the secret is used by an official interface
	int daily_limit = 0;   ///< CAD requests per 24 h and account, 0: none
};

/**
	@brief The manufacturer portals: one account each, its status, and the
	request limit of 3Dfindit.

	Only official interfaces and public files are used:
	- EPLAN Data Portal: the documented API (https://dataportal.eplan.com/api)
	  with the user's Personal Access Token: search, DXF packages with
	  part data. Holds the data of Siemens, Rittal and many more.
	- Siemens: the public files on static.siemens.com that the SiePortal
	  product page links (EPLAN macro EDZ, DXF, STEP). SiePortal itself
	  refuses programs (bot protection) and has no API for the CAx data,
	  so the list of links of a part comes from the browser (link file).
	- Rittal: no API of its own; its EPLAN data comes from the Data Portal.
	- 3Dfindit: no API for users (the PARTcommunity web service is for
	  the catalog partners). QElectroTech opens the part in the browser
	  and keeps the 50 CAD requests per 24 h and account: counted, shown,
	  and refused above the limit. One account only, no rotation.

	The passwords of portals without an official login interface are kept
	in the credential store for the user, but never sent by QElectroTech:
	it does not fill in login forms or solve CAPTCHAs.
*/
class ManufacturerPortals
{
	Q_DECLARE_TR_FUNCTIONS(ManufacturerPortals)

	public:
		static QList<ManufacturerPortal> portals();
		static ManufacturerPortal portal(const QString &id);

		/// the data folder (the parent of the company collection), QET_PORTAL_DATA_DIR wins
		static QString dataDir();
		/// raw manufacturer files: <data>/herstellerdaten
		static QString rawDir();
		/// STEP models: QET_3D_DIR, else <data>/3D
		static QString modelDir();

		/// the Personal Access Token of the Data Portal: credential store, else
		/// the old plain setting, which is then moved into the store
		static QString dataPortalToken();
		/// the token checked against the Data Portal (GET /basketitems); empty: valid
		static QString checkDataPortalToken(const QString &token);

		static bool loggedIn(const QString &id);
		static QString user(const QString &id);
		/// {id, name, logged_in, user, access, limit, used, left} of one portal
		static QJsonObject status(const QString &id);
		/// {portals: [...], credential_store}
		static QJsonObject statusAll();

		/// 3Dfindit: CAD requests in the last 24 h (log file in the data folder)
		static int requestsUsed(const QString &id);
		static int requestsLeft(const QString &id);
		/// when the oldest request of the last 24 h drops out; null when none
		static QDateTime nextFreeRequest(const QString &id);
		/// counts one request; false (and nothing counted) above the limit
		static bool registerRequest(const QString &id, const QString &order, QString *error = nullptr);

		/// the product (or search) page of @p order in the portal, for the browser
		static QString productUrl(const QString &id, const QString &order);

	private:
		static QString requestLog(const QString &id);
		static QList<QDateTime> requests(const QString &id);
};

#endif // MANUFACTURERPORTALS_H
