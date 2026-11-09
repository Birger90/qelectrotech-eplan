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
#include "portalcredentials.h"

#include <QByteArray>
#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wincred.h>
#endif

QString PortalCredentials::targetName(const QString &portal)
{
	QString prefix = qEnvironmentVariable("QET_CREDENTIAL_PREFIX");
	if (prefix.isEmpty()) prefix = QStringLiteral("QElectroTech/portal");
	return prefix + QLatin1Char('/') + portal;
}

bool PortalCredentials::available()
{
#ifdef Q_OS_WIN
	return true;
#else
	return false;
#endif
}

bool PortalCredentials::store(const QString &portal, const PortalAccount &account, QString *error)
{
#ifdef Q_OS_WIN
	const std::wstring target = targetName(portal).toStdWString();
	const std::wstring user = account.user.toStdWString();
	const std::wstring comment = QStringLiteral("QElectroTech: manufacturer portal").toStdWString();
	QByteArray blob = account.secret.toUtf8();
	CREDENTIALW cred {};
	cred.Type = CRED_TYPE_GENERIC;
	cred.TargetName = const_cast<LPWSTR>(target.c_str());
	cred.UserName = const_cast<LPWSTR>(user.c_str());
	cred.Comment = const_cast<LPWSTR>(comment.c_str());
	cred.CredentialBlobSize = DWORD(blob.size());
	cred.CredentialBlob = reinterpret_cast<LPBYTE>(blob.data());
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
	const bool ok = CredWriteW(&cred, 0);
	const DWORD code = ok ? 0 : GetLastError();
	blob.fill('\0');	// no copy of the secret left in memory
	if (!ok && error)
		*error = QCoreApplication::translate("PortalCredentials",
						     "The credential store refused the account (error %1).").arg(code);
	return ok;
#else
	Q_UNUSED(portal) Q_UNUSED(account)
	if (error) *error = QCoreApplication::translate("PortalCredentials",
							"No credential store on this system.");
	return false;
#endif
}

PortalAccount PortalCredentials::load(const QString &portal)
{
	PortalAccount account;
#ifdef Q_OS_WIN
	PCREDENTIALW cred = nullptr;
	if (!CredReadW(targetName(portal).toStdWString().c_str(), CRED_TYPE_GENERIC, 0, &cred)) return account;
	if (cred->UserName) account.user = QString::fromWCharArray(cred->UserName);
	if (cred->CredentialBlob && cred->CredentialBlobSize)
		account.secret = QString::fromUtf8(reinterpret_cast<const char *>(cred->CredentialBlob),
						   int(cred->CredentialBlobSize));
	CredFree(cred);
#else
	Q_UNUSED(portal)
#endif
	return account;
}

bool PortalCredentials::has(const QString &portal)
{
#ifdef Q_OS_WIN
	PCREDENTIALW cred = nullptr;
	if (!CredReadW(targetName(portal).toStdWString().c_str(), CRED_TYPE_GENERIC, 0, &cred)) return false;
	CredFree(cred);
	return true;
#else
	Q_UNUSED(portal)
	return false;
#endif
}

bool PortalCredentials::remove(const QString &portal, QString *error)
{
#ifdef Q_OS_WIN
	if (CredDeleteW(targetName(portal).toStdWString().c_str(), CRED_TYPE_GENERIC, 0)) return true;
	const DWORD code = GetLastError();
	if (code == ERROR_NOT_FOUND) return true;
	if (error) *error = QCoreApplication::translate("PortalCredentials",
							"The account cannot be removed (error %1).").arg(code);
	return false;
#else
	Q_UNUSED(portal) Q_UNUSED(error)
	return true;
#endif
}
