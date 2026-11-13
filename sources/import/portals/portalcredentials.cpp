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

#ifdef Q_OS_MACOS
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

namespace
{
		///The keychain item of a portal: a generic password whose service
		///is the target name, the same name the Windows store uses
	CFMutableDictionaryRef keychainQuery(const QString &target)
	{
		CFMutableDictionaryRef query = CFDictionaryCreateMutable(
					nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(query, kSecClass, kSecClassGenericPassword);
		CFStringRef service = target.toCFString();
		CFDictionarySetValue(query, kSecAttrService, service);
		CFRelease(service);
		return query;
	}

	QString keychainError(OSStatus status)
	{
		QString text;
		if (CFStringRef message = SecCopyErrorMessageString(status, nullptr)) {
			text = QString::fromCFString(message);
			CFRelease(message);
		}
		return text.isEmpty() ? QString::number(int(status)) : text;
	}
}
#endif

QString PortalCredentials::targetName(const QString &portal)
{
	QString prefix = qEnvironmentVariable("QET_CREDENTIAL_PREFIX");
	if (prefix.isEmpty()) prefix = QStringLiteral("QElectroTech/portal");
	return prefix + QLatin1Char('/') + portal;
}

bool PortalCredentials::available()
{
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
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
#elif defined(Q_OS_MACOS)
		//One item per portal: the old one goes, the new one comes
	CFMutableDictionaryRef query = keychainQuery(targetName(portal));
	SecItemDelete(query);
	CFStringRef user = account.user.toCFString();
	CFStringRef label = QStringLiteral("QElectroTech: manufacturer portal %1").arg(portal).toCFString();
	QByteArray blob = account.secret.toUtf8();
	CFDataRef data = CFDataCreate(nullptr, reinterpret_cast<const UInt8 *>(blob.constData()), blob.size());
	blob.fill('\0');	// no copy of the secret left in memory
	CFDictionarySetValue(query, kSecAttrAccount, user);
	CFDictionarySetValue(query, kSecAttrLabel, label);
	CFDictionarySetValue(query, kSecValueData, data);
	const OSStatus status = SecItemAdd(query, nullptr);
	CFRelease(data);
	CFRelease(label);
	CFRelease(user);
	CFRelease(query);
	if (status != errSecSuccess && error)
		*error = QCoreApplication::translate("PortalCredentials",
						     "The keychain refused the account (%1).").arg(keychainError(status));
	return status == errSecSuccess;
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
#elif defined(Q_OS_MACOS)
	CFMutableDictionaryRef query = keychainQuery(targetName(portal));
	CFDictionarySetValue(query, kSecReturnAttributes, kCFBooleanTrue);
	CFDictionarySetValue(query, kSecReturnData, kCFBooleanTrue);
	CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitOne);
	CFTypeRef result = nullptr;
	if (SecItemCopyMatching(query, &result) == errSecSuccess && result
		&& CFGetTypeID(result) == CFDictionaryGetTypeID())
	{
		auto item = static_cast<CFDictionaryRef>(result);
		auto user = static_cast<CFStringRef>(CFDictionaryGetValue(item, kSecAttrAccount));
		if (user && CFGetTypeID(user) == CFStringGetTypeID())
			account.user = QString::fromCFString(user);
		auto data = static_cast<CFDataRef>(CFDictionaryGetValue(item, kSecValueData));
		if (data && CFGetTypeID(data) == CFDataGetTypeID())
			account.secret = QString::fromUtf8(reinterpret_cast<const char *>(CFDataGetBytePtr(data)),
							   int(CFDataGetLength(data)));
	}
	if (result) CFRelease(result);
	CFRelease(query);
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
#elif defined(Q_OS_MACOS)
	CFMutableDictionaryRef query = keychainQuery(targetName(portal));
	CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitOne);
	const OSStatus status = SecItemCopyMatching(query, nullptr);
	CFRelease(query);
	return status == errSecSuccess;
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
#elif defined(Q_OS_MACOS)
	CFMutableDictionaryRef query = keychainQuery(targetName(portal));
	const OSStatus status = SecItemDelete(query);
	CFRelease(query);
	if (status == errSecSuccess || status == errSecItemNotFound) return true;
	if (error) *error = QCoreApplication::translate("PortalCredentials",
							"The account cannot be removed from the keychain (%1).").arg(keychainError(status));
	return false;
#else
	Q_UNUSED(portal) Q_UNUSED(error)
	return true;
#endif
}
