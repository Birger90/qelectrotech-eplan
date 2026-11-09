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
#include "portalsdialog.h"

#include "manufacturerportals.h"
#include "partfetcher.h"
#include "portalcredentials.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>

PortalsDialog::PortalsDialog(QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Manufacturer portals"));
	setMinimumSize(720, 620);

	auto *intro = new QLabel(
		tr("One account per portal. Passwords and tokens are kept in the Windows Credential Manager, "
		   "never in the settings. QElectroTech uses official interfaces and public files only; "
		   "it does not fill in login forms and solves no CAPTCHAs."),
		this);
	intro->setWordWrap(true);

	auto *tabs = new QTabWidget(this);
	const QList<ManufacturerPortal> list = ManufacturerPortals::portals();
	for (const ManufacturerPortal &p : list)
		tabs->addTab(portalPage(p.id), p.name);

		// processing a list of missing parts
	m_list = new QLineEdit(this);
	const QString data = ManufacturerPortals::dataDir();
	if (!data.isEmpty())
		m_list->setText(QDir::toNativeSeparators(
			QDir(data).filePath(QStringLiteral("Fehlende_Komponenten_nach_Hersteller.txt"))));
	auto *browse = new QPushButton(tr("Choose…"), this);
	connect(browse, &QPushButton::clicked, this, &PortalsDialog::chooseList);
	auto *list_row = new QHBoxLayout;
	list_row->addWidget(m_list, 1);
	list_row->addWidget(browse);
	m_makers = new QLineEdit(this);
	m_makers->setPlaceholderText(tr("e.g. SIE, RIT; empty: all manufacturers"));
	m_dry_run = new QCheckBox(tr("Only show what would be loaded"), this);
	m_run = new QPushButton(tr("Fetch missing parts"), this);
	m_cancel = new QPushButton(tr("Stop"), this);
	m_cancel->setEnabled(false);
	connect(m_run, &QPushButton::clicked, this, &PortalsDialog::runList);
	connect(m_cancel, &QPushButton::clicked, this, [this]() { if (m_fetcher) m_fetcher->cancel(); });
	auto *run_row = new QHBoxLayout;
	run_row->addWidget(m_dry_run, 1);
	run_row->addWidget(m_run);
	run_row->addWidget(m_cancel);
	m_progress = new QProgressBar(this);
	m_progress->setVisible(false);
	m_log = new QPlainTextEdit(this);
	m_log->setReadOnly(true);
	m_log->setPlaceholderText(tr("The parts and what was loaded for them are listed here."));

	auto *fetch_form = new QFormLayout;
	fetch_form->addRow(tr("List of missing parts:"), list_row);
	fetch_form->addRow(tr("Manufacturers:"), m_makers);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto *layout = new QVBoxLayout(this);
	layout->addWidget(intro);
	layout->addWidget(tabs);
	layout->addLayout(fetch_form);
	layout->addLayout(run_row);
	layout->addWidget(m_progress);
	layout->addWidget(m_log, 1);
	layout->addWidget(buttons);
	updateStatus();
}

PortalsDialog::~PortalsDialog()
{
	if (m_fetcher) m_fetcher->cancel();
}

QWidget *PortalsDialog::portalPage(const QString &id)
{
	const ManufacturerPortal p = ManufacturerPortals::portal(id);
	auto *page = new QWidget(this);
	auto *form = new QFormLayout(page);

	auto *access = new QLabel(p.access, page);
	access->setWordWrap(true);
	form->addRow(access);

	Row row;
	row.status = new QLabel(page);
	row.status->setTextInteractionFlags(Qt::TextSelectableByMouse);
	form->addRow(tr("Status:"), row.status);
	const PortalAccount account = PortalCredentials::load(id);
	if (id != QLatin1String("dataportal")) {
		row.user = new QLineEdit(account.user, page);
		row.user->setPlaceholderText(tr("User name or e-mail"));
		form->addRow(tr("User:"), row.user);
	}
	row.secret = new QLineEdit(page);
	row.secret->setEchoMode(QLineEdit::Password);	// never shown in clear, never filled in
	row.secret->setClearButtonEnabled(true);
	row.secret->setPlaceholderText(account.secret.isEmpty() ? QString() : tr("stored; type to replace"));
	form->addRow(p.secret_label + QLatin1Char(':'), row.secret);

	row.login = new QPushButton(p.api_login ? tr("Log in") : tr("Store account"), page);
	row.logout = new QPushButton(tr("Log out"), page);
	auto *open = new QPushButton(tr("Open portal"), page);
	connect(row.login, &QPushButton::clicked, this, [this, id]() { login(id); });
	connect(row.logout, &QPushButton::clicked, this, [this, id]() { logout(id); });
	const QString url = p.login_url;
	connect(open, &QPushButton::clicked, this, [url]() { QDesktopServices::openUrl(QUrl(url)); });
	auto *buttons = new QHBoxLayout;
	buttons->addWidget(row.login);
	buttons->addWidget(row.logout);
	buttons->addStretch();
	buttons->addWidget(open);
	form->addRow(buttons);

	if (p.daily_limit > 0) {
			// 3Dfindit: the limit, counted and kept
		m_quota = new QLabel(page);
		form->addRow(tr("CAD requests:"), m_quota);
		m_request_order = new QLineEdit(page);
		m_request_order->setPlaceholderText(tr("order number of the part requested"));
		auto *count = new QPushButton(tr("Count a request"), page);
		count->setToolTip(tr("Counts a CAD request made in the browser. Above %1 in 24 h QElectroTech "
				     "refuses further ones.").arg(p.daily_limit));
		connect(count, &QPushButton::clicked, this, &PortalsDialog::countRequest);
		auto *count_row = new QHBoxLayout;
		count_row->addWidget(m_request_order, 1);
		count_row->addWidget(count);
		form->addRow(tr("Request:"), count_row);
	}
	if (id == QLatin1String("siemens")) {
		m_links_info = new QLabel(page);
		m_links_info->setWordWrap(true);
		form->addRow(tr("Links:"), m_links_info);
		auto *add = new QPushButton(tr("Add links…"), page);
		add->setToolTip(tr("Paste the links of the SiePortal download dialog (static.siemens.com), "
				   "each line \"<order number> <link>\"."));
		connect(add, &QPushButton::clicked, this, &PortalsDialog::addLinks);
		auto *open_file = new QPushButton(tr("Open link folder"), page);
		connect(open_file, &QPushButton::clicked, this, []() {
			const QString file = PartFetcher::siemensLinkFile();
			if (file.isEmpty()) return;
			QDir().mkpath(QFileInfo(file).absolutePath());
			QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(file).absolutePath()));
		});
		auto *link_row = new QHBoxLayout;
		link_row->addWidget(add);
		link_row->addWidget(open_file);
		link_row->addStretch();
		form->addRow(link_row);
	}
	m_rows.insert(id, row);
	return page;
}

void PortalsDialog::login(const QString &id)
{
	const Row row = m_rows.value(id);
	const ManufacturerPortal p = ManufacturerPortals::portal(id);
	PortalAccount account = PortalCredentials::load(id);
	if (row.user) account.user = row.user->text().trimmed();
	if (!row.secret->text().isEmpty()) account.secret = row.secret->text();
	if (id == QLatin1String("dataportal")) {
		if (account.secret.isEmpty()) {
			QMessageBox::information(this, p.name, tr("Enter the Personal Access Token first."));
			return;
		}
		QGuiApplication::setOverrideCursor(Qt::WaitCursor);
		const QString error = ManufacturerPortals::checkDataPortalToken(account.secret);
		QGuiApplication::restoreOverrideCursor();
		if (!error.isEmpty()) {
			QMessageBox::warning(this, p.name, error);
			return;
		}
	} else if (account.user.isEmpty()) {
		QMessageBox::information(this, p.name, tr("Enter the user name first."));
		return;
	}
	QString error;
	if (!PortalCredentials::store(id, account, &error)) {
		QMessageBox::warning(this, p.name, error);
		return;
	}
	if (id == QLatin1String("dataportal"))
		QSettings().remove(QStringLiteral("eplanimport/dataportal_token"));	// no plain copy left
	account.secret.fill(QLatin1Char('\0'));
	row.secret->clear();
	row.secret->setPlaceholderText(tr("stored; type to replace"));
	updateStatus();
}

void PortalsDialog::logout(const QString &id)
{
	QString error;
	if (!PortalCredentials::remove(id, &error))
		QMessageBox::warning(this, ManufacturerPortals::portal(id).name, error);
	if (id == QLatin1String("dataportal"))
		QSettings().remove(QStringLiteral("eplanimport/dataportal_token"));	// the plain token of older versions too
	const Row row = m_rows.value(id);
	row.secret->clear();
	row.secret->setPlaceholderText(QString());
	updateStatus();
}

void PortalsDialog::updateStatus()
{
	for (auto it = m_rows.constBegin(); it != m_rows.constEnd(); ++it) {
		const QString id = it.key();
		const ManufacturerPortal p = ManufacturerPortals::portal(id);
		const bool in = PortalCredentials::has(id);
		const QString user = in ? ManufacturerPortals::user(id) : QString();
		QString text;
		if (!PortalCredentials::available())
			text = tr("No credential store on this system.");
		else if (!in)
			text = tr("Not logged in.");
		else if (p.api_login)
			text = tr("Logged in (token in the Credential Manager).");
		else
			text = user.isEmpty() ? tr("Account stored.") : tr("Account %1 stored.").arg(user);
		it.value().status->setText(text);
		it.value().logout->setEnabled(in);
	}
	if (m_quota) {
		const QString id = QStringLiteral("3dfindit");
		const int limit = ManufacturerPortals::portal(id).daily_limit;
		const int used = ManufacturerPortals::requestsUsed(id);
		QString text = tr("%1 of %2 used in the last 24 h, %3 left.").arg(used).arg(limit).arg(qMax(0, limit - used));
		const QDateTime next = ManufacturerPortals::nextFreeRequest(id);
		if (used >= limit && next.isValid())
			text += QLatin1Char(' ') + tr("Next request at %1.").arg(QLocale().toString(next, QLocale::ShortFormat));
		m_quota->setText(text);
	}
	if (m_links_info) {
		const QString file = PartFetcher::siemensLinkFile();
		const int parts = int(PartFetcher::readLinks(file).size());
		m_links_info->setText(tr("%1 parts with links in %2. QElectroTech loads the public files of these links itself.")
				      .arg(parts).arg(QDir::toNativeSeparators(file)));
	}
}

void PortalsDialog::countRequest()
{
	const QString order = m_request_order->text().simplified();
	if (order.isEmpty()) return;
	QString error;
	if (!ManufacturerPortals::registerRequest(QStringLiteral("3dfindit"), order, &error))
		QMessageBox::warning(this, tr("3Dfindit"), error);
	else
		m_request_order->clear();
	updateStatus();
}

void PortalsDialog::addLinks()
{
	bool ok = false;
	const QString text = QInputDialog::getMultiLineText(
		this, tr("SiePortal links"),
		tr("One link per line, after the order number:\n3RV2011-0FA10 https://static.siemens.com/mimes/imagedb_stp/…"),
		QString(), &ok);
	if (!ok || text.trimmed().isEmpty()) return;
	QStringList good;
	int bad = 0;
	const QStringList lines = text.split(QLatin1Char('\n'));
	for (const QString &line : lines) {
		const QString l = line.simplified();
		if (l.isEmpty()) continue;
		const int at = l.indexOf(QStringLiteral("https://"));
		const QString url = at > 0 ? l.mid(at).section(QLatin1Char(' '), 0, 0) : QString();
		if (at > 0 && PartFetcher::isSiemensFileUrl(url))
			good << l.left(at).trimmed() + QLatin1Char('\t') + url;
		else
			++bad;
	}
	const QString file = PartFetcher::siemensLinkFile();
	if (!good.isEmpty()) {
		QDir().mkpath(QFileInfo(file).absolutePath());
		QFile f(file);
		if (f.open(QIODevice::Append | QIODevice::Text))
			f.write((good.join(QLatin1Char('\n')) + QLatin1Char('\n')).toUtf8());
	}
	if (bad)
		QMessageBox::information(this, tr("SiePortal links"),
					 tr("%1 lines taken, %2 left out: only links of static.siemens.com/mimes "
					    "after an order number.").arg(good.size()).arg(bad));
	updateStatus();
}

void PortalsDialog::chooseList()
{
	const QString file = QFileDialog::getOpenFileName(this, tr("List of missing parts"), m_list->text(),
							  tr("Text files (*.txt);;All files (*)"));
	if (!file.isEmpty()) m_list->setText(QDir::toNativeSeparators(file));
}

void PortalsDialog::runList()
{
	QString error;
	const QList<FetchItem> items = PartFetcher::readList(QDir::fromNativeSeparators(m_list->text()), &error);
	if (items.isEmpty()) {
		QMessageBox::warning(this, windowTitle(), error);
		return;
	}
	FetchOptions options;
	options.dry_run = m_dry_run->isChecked();
	static const QRegularExpression separator(QStringLiteral("[,;]"));
	const QStringList makers = m_makers->text().split(separator, Qt::SkipEmptyParts);
	for (const QString &m : makers) options.makers << m.trimmed();

	m_fetcher = new PartFetcher(this);
	m_run->setEnabled(false);
	m_cancel->setEnabled(true);
	m_progress->setVisible(true);
	m_progress->setValue(0);
	m_log->clear();
	connect(m_fetcher, &PartFetcher::progress, this, [this](int done, int total, const QString &line) {
		m_progress->setMaximum(total);
		m_progress->setValue(done);
		m_log->appendPlainText(line);
	});
	const QList<FetchResult> results = m_fetcher->run(items, options);
	m_log->clear();
	QHash<QString, int> count;
	for (const FetchResult &r : results) {
		++count[r.status];
		if (r.status == QLatin1String("present")) continue;
		m_log->appendPlainText(QStringLiteral("%1 %2: %3").arg(r.maker, r.order, r.status));
		for (const QString &n : r.notes) m_log->appendPlainText(QStringLiteral("    ") + n);
		m_written << r.written;
	}
	m_log->appendPlainText(tr("\n%1 parts: %2 done, %3 partly, %4 there already, %5 not found, %6 planned.")
			       .arg(results.size()).arg(count.value(QStringLiteral("done")))
			       .arg(count.value(QStringLiteral("partial"))).arg(count.value(QStringLiteral("present")))
			       .arg(count.value(QStringLiteral("missing"))).arg(count.value(QStringLiteral("planned"))));
	m_fetcher->deleteLater();
	m_run->setEnabled(true);
	m_cancel->setEnabled(false);
	m_progress->setVisible(false);
	updateStatus();
}
