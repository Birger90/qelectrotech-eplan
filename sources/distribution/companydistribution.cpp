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
#include "companydistribution.h"

#include "../logging/qetlogger.h"
#include "../qetapp.h"
#include "../qetversion.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QSysInfo>
#include <QVBoxLayout>
#include <QVersionNumber>
#include <QWidget>

namespace {

const char *const PRODUCT = "QET-Fork";

	/// version.json of @a dir, empty object when it is not ours
QJsonObject manifest(const QString &dir)
{
	if (dir.isEmpty()) return {};
	QFile f(QDir(dir).filePath(QStringLiteral("version.json")));
	if (!f.open(QIODevice::ReadOnly)) return {};
	const QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
	return o.value(QStringLiteral("product")).toString() == QLatin1String(PRODUCT) ? o : QJsonObject();
}

bool isDistribution(const QString &dir) { return !manifest(dir).isEmpty(); }

	/// reports written while the distribution folder was not there
QString pendingDir()
{
	return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)).filePath(QStringLiteral("meldungen-offen"));
}

bool copyTree(const QString &from, const QString &to)
{
	if (!QDir().mkpath(to)) return false;
	for (const QFileInfo &fi : QDir(from).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
		const QString target = QDir(to).filePath(fi.fileName());
		if (fi.isDir() ? !copyTree(fi.absoluteFilePath(), target) : !QFile::copy(fi.absoluteFilePath(), target)) return false;
	}
	return true;
}

QString userName()
{
	const QString n = qEnvironmentVariable("USERNAME", qEnvironmentVariable("USER"));
	return n.isEmpty() ? QStringLiteral("unbekannt") : n;
}

} // namespace

QString CompanyDistribution::detectFolder()
{
	QStringList roots;
#ifdef Q_OS_WIN
	// the folders OneDrive synchronises (SharePoint libraries among them)
	QSettings providers(QStringLiteral("HKEY_CURRENT_USER\\Software\\SyncEngines\\Providers\\OneDrive"), QSettings::NativeFormat);
	for (const QString &key : providers.childGroups()) {
		const QString mount = providers.value(key + QStringLiteral("/MountPoint")).toString();
		if (!mount.isEmpty()) roots << QDir::fromNativeSeparators(mount);
	}
#endif
	// OneDrive for business puts its folders under the profile: "<Firma>/<Website> - <Bibliothek>"
	for (const QFileInfo &fi : QDir(QDir::homePath()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
		roots << fi.absoluteFilePath();
	roots.removeDuplicates();
	for (const QString &root : std::as_const(roots)) {
		if (isDistribution(root)) return root;
		// one or two levels below: "<Bibliothek>/QET-Fork" or "<Firma>/<Bibliothek>"
		for (const QFileInfo &a : QDir(root).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
			if (isDistribution(a.absoluteFilePath())) return a.absoluteFilePath();
			if (!a.fileName().contains(QLatin1String("QET"), Qt::CaseInsensitive)) continue;
			for (const QFileInfo &b : QDir(a.absoluteFilePath()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
				if (isDistribution(b.absoluteFilePath())) return b.absoluteFilePath();
		}
	}
	return QString();
}

QString CompanyDistribution::folder()
{
	static QString cached;
	static bool done = false;
	if (done) return cached;
	done = true;
	const QString env = QDir::fromNativeSeparators(qEnvironmentVariable("QET_DISTRIBUTION_DIR"));
	if (isDistribution(env)) return cached = env;
	QSettings settings;
	const QString set = settings.value(QStringLiteral("distribution/folder")).toString();
	if (isDistribution(set)) return cached = set;
	cached = detectFolder();
	if (!cached.isEmpty()) settings.setValue(QStringLiteral("distribution/folder"), cached);
	return cached;
}

bool CompanyDistribution::isNewer(const QString &a, const QString &b)
{
	return QVersionNumber::compare(QVersionNumber::fromString(a), QVersionNumber::fromString(b)) > 0;
}

void CompanyDistribution::applyCompanyCollection()
{
	const QString dir = folder();
	if (dir.isEmpty()) return;
	const QString parts = QDir(dir).filePath(QStringLiteral("Bauteile"));
	if (!QDir(parts).exists()) return;
	// only when no own collection was chosen (or ours is set already)
	QSettings settings;
	const QString key = QStringLiteral("elements-collections/company-collection-path");
	const QString now = settings.value(key, QStringLiteral("default")).toString();
	const QString ours = settings.value(QStringLiteral("distribution/company-collection")).toString();
	if (now == QLatin1String("default") || now.isEmpty() || now == ours || !QDir(now).exists()) {
		settings.setValue(key, parts);
		settings.setValue(QStringLiteral("distribution/company-collection"), parts);
	}
}

void CompanyDistribution::checkForUpdate(bool quiet)
{
	const QString dir = folder();
	QWidget *parent = QApplication::activeWindow();
	if (dir.isEmpty()) {
		if (!quiet)
			QMessageBox::information(parent, QObject::tr("Updates"),
				QObject::tr("No distribution folder found. Synchronise the SharePoint library \"QET-Fork\" with OneDrive "
							"or name its folder in the environment variable QET_DISTRIBUTION_DIR."));
		return;
	}
	const QJsonObject m = manifest(dir);
	const QString latest = m.value(QStringLiteral("version")).toString();
	const QString current = QetVersion::forkVersion();
	if (!isNewer(latest, current)) {
		if (!quiet)
			QMessageBox::information(parent, QObject::tr("Updates"), QObject::tr("This version (%1) is up to date.").arg(current));
		return;
	}
	QSettings settings;
	if (quiet && settings.value(QStringLiteral("distribution/skipped-version")).toString() == latest) return;
	QMessageBox box(QMessageBox::Information, QObject::tr("Update available"),
		QObject::tr("Version %1 is available (installed: %2).").arg(latest, current), QMessageBox::NoButton, parent);
	const QString notes = m.value(QStringLiteral("notes")).toString();
	if (!notes.isEmpty()) box.setInformativeText(notes);
	QPushButton *now = box.addButton(QObject::tr("Install now"), QMessageBox::AcceptRole);
	QPushButton *skip = box.addButton(QObject::tr("Skip this version"), QMessageBox::RejectRole);
	box.addButton(QObject::tr("Later"), QMessageBox::RejectRole);
	box.exec();
	if (box.clickedButton() == skip) {
		settings.setValue(QStringLiteral("distribution/skipped-version"), latest);
		return;
	}
	if (box.clickedButton() != now) return;
	// the script waits until this process has ended, then installs and starts the new version
	const QString script = QDir::toNativeSeparators(QDir(dir).filePath(QStringLiteral("Installieren.ps1")));
	if (!QFileInfo::exists(script)
		|| !QProcess::startDetached(QStringLiteral("powershell.exe"),
			{QStringLiteral("-NoProfile"), QStringLiteral("-ExecutionPolicy"), QStringLiteral("Bypass"),
			 QStringLiteral("-File"), script, QStringLiteral("-Update"),
			 QStringLiteral("-WartenAuf"), QString::number(QCoreApplication::applicationPid())})) {
		QMessageBox::warning(parent, QObject::tr("Update"), QObject::tr("The installer %1 could not be started.").arg(script));
		return;
	}
	// asks to save modified projects; if the user cancels, the script installs when QElectroTech ends
	QETApp::instance()->quitQET();
}

void CompanyDistribution::startupTasks()
{
	const QString dir = folder();
	if (dir.isEmpty()) return;
	// reports written offline go to the folder now
	const QString target = QDir(dir).filePath(QStringLiteral("Meldungen"));
	for (const QFileInfo &fi : QDir(pendingDir()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
		if (copyTree(fi.absoluteFilePath(), QDir(target).filePath(fi.fileName())))
			QDir(fi.absoluteFilePath()).removeRecursively();
#ifdef Q_OS_WIN
	// keep the parts on this computer (OneDrive "always keep on this device"), once per folder
	QSettings settings;
	const QString parts = QDir(dir).filePath(QStringLiteral("Bauteile"));
	if (QDir(parts).exists() && settings.value(QStringLiteral("distribution/pinned")).toString() != parts) {
		QProcess::startDetached(QStringLiteral("attrib.exe"),
			{QStringLiteral("+P"), QDir::toNativeSeparators(parts + QStringLiteral("/*")), QStringLiteral("/S"), QStringLiteral("/D")});
		settings.setValue(QStringLiteral("distribution/pinned"), parts);
	}
#endif
	checkForUpdate(true);
}

// ----------------------------------------------------------------------------- report dialog
ReportDialog::ReportDialog(QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Report a bug or a wish"));
	resize(620, 520);
	// the window as it was before the dialog opened
	if (QWidget *w = parent ? parent->window() : QApplication::activeWindow()) m_image = w->grab().toImage();

	auto *layout = new QVBoxLayout(this);
	auto *hint = new QLabel(tr("Describe what happened or what you wish for. The report goes to the developers "
							   "together with the program version; nothing is sent to the internet."), this);
	hint->setWordWrap(true);
	layout->addWidget(hint);
	auto *form = new QFormLayout;
	m_kind = new QComboBox(this);
	m_kind->addItem(tr("Bug"), QStringLiteral("Fehler"));
	m_kind->addItem(tr("Wish / new feature"), QStringLiteral("Wunsch"));
	form->addRow(tr("Kind"), m_kind);
	m_title = new QLineEdit(this);
	m_title->setPlaceholderText(tr("in a few words"));
	form->addRow(tr("Title"), m_title);
	m_name = new QLineEdit(userName(), this);
	form->addRow(tr("Name"), m_name);
	layout->addLayout(form);
	m_text = new QPlainTextEdit(this);
	m_text->setPlaceholderText(tr("What did you do, what did you expect, what happened instead? Which project or sheet?"));
	layout->addWidget(m_text, 1);
	m_screenshot = new QCheckBox(tr("Attach a screenshot of the window"), this);
	m_screenshot->setChecked(!m_image.isNull());
	m_screenshot->setEnabled(!m_image.isNull());
	layout->addWidget(m_screenshot);
	m_diagnostics = new QCheckBox(tr("Attach the diagnostic report (latest log entries)"), this);
	m_diagnostics->setChecked(true);
	layout->addWidget(m_diagnostics);
	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
	QPushButton *send = buttons->addButton(tr("Send"), QDialogButtonBox::AcceptRole);
	connect(send, &QPushButton::clicked, this, &ReportDialog::send);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttons);
}

void ReportDialog::send()
{
	if (m_title->text().trimmed().isEmpty() && m_text->toPlainText().trimmed().isEmpty()) {
		QMessageBox::information(this, windowTitle(), tr("Please enter a title or a description."));
		return;
	}
	const QString kind = m_kind->currentData().toString();
	QString who = m_name->text().trimmed();
	who.replace(QRegularExpression(QStringLiteral("[^\\w.-]+")), QStringLiteral("_"));
	const QDateTime now = QDateTime::currentDateTime();
	const QString name = QStringLiteral("%1_%2_%3").arg(now.toString(QStringLiteral("yyyy-MM-dd_HHmmss")), kind,
														   who.isEmpty() ? userName() : who);
	const QString dist = CompanyDistribution::folder();
	const bool online = !dist.isEmpty();
	const QString dir = QDir(online ? QDir(dist).filePath(QStringLiteral("Meldungen")) : pendingDir()).filePath(name);
	if (!QDir().mkpath(dir)) {
		QMessageBox::warning(this, windowTitle(), tr("The folder %1 could not be created.").arg(QDir::toNativeSeparators(dir)));
		return;
	}
	QFile f(QDir(dir).filePath(QStringLiteral("meldung.txt")));
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, windowTitle(), tr("The report could not be written."));
		return;
	}
	const QString text = QStringLiteral("Art: %1\nTitel: %2\nName: %3\nDatum: %4\nVersion: %5\nSystem: %6\n\n%7\n")
		.arg(kind, m_title->text().trimmed(), m_name->text().trimmed(), now.toString(Qt::ISODate),
			 QetVersion::displayedVersionWithFork(), QSysInfo::prettyProductName(), m_text->toPlainText());
	f.write(text.toUtf8());
	f.close();
	if (m_screenshot->isChecked() && !m_image.isNull()) m_image.save(QDir(dir).filePath(QStringLiteral("bild.png")));
	if (m_diagnostics->isChecked()) {
		QFile d(QDir(dir).filePath(QStringLiteral("diagnose.txt")));
		if (d.open(QIODevice::WriteOnly)) d.write(QetLogger::instance().buildDiagnosticsReport());
	}
	QMessageBox::information(this, windowTitle(), online
		? tr("Thank you, the report has been saved.")
		: tr("Thank you. The distribution folder is not available; the report is kept on this computer and "
			 "handed in at the next start."));
	accept();
}
