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
#ifndef PORTALCREDENTIALS_H
#define PORTALCREDENTIALS_H

#include <QString>

/// the account of a manufacturer portal: user name and secret (password or token)
struct PortalAccount
{
	QString user;
	QString secret;
	bool isEmpty() const { return user.isEmpty() && secret.isEmpty(); }
};

/**
	@brief The accounts of the manufacturer portals in the credential store
	of the operating system.

	On Windows this is the Credential Manager (CredWrite/CredRead, generic
	credentials, "persist on this computer"): the secret is kept encrypted
	for the Windows user and never written to the settings or a file of
	QElectroTech. One account per portal, target name
	"QElectroTech/portal/<portal id>".

	The environment variable QET_CREDENTIAL_PREFIX replaces the prefix
	"QElectroTech/portal", for tests that must not touch the real accounts.

	Other systems have no store here yet: available() is false and
	nothing is kept.
*/
class PortalCredentials
{
	public:
		static bool available();
		static bool store(const QString &portal, const PortalAccount &account, QString *error = nullptr);
		/// the stored account, empty when there is none
		static PortalAccount load(const QString &portal);
		static bool has(const QString &portal);
		static bool remove(const QString &portal, QString *error = nullptr);
		/// the target name in the credential store
		static QString targetName(const QString &portal);
};

#endif // PORTALCREDENTIALS_H
