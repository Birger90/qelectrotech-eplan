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
#include "assistantsetuppage.h"

#include "assistantbridge.h"
#include "assistantdock.h"
#include "../import/portals/portalcredentials.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QScrollArea>
#include <QStackedWidget>
#include <QSettings>
#include <QStandardPaths>
#include <QTextCursor>
#include <QUrl>
#include <QVBoxLayout>

namespace {
	const QString CREDENTIAL = QStringLiteral("claude");
	const QString PYTHON_SETTING = QStringLiteral("assistant/python");

		/// A wrapped label takes the dock's width instead of widening it
	void fitWidth(QLabel *label)
	{
		label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
		label->setMinimumWidth(120);
	}

		/// A step's text: heading in bold, explanation below, links open
	QLabel *stepText(QWidget *parent)
	{
		auto *label = new QLabel(parent);
		label->setWordWrap(true);
		label->setTextFormat(Qt::RichText);
		label->setOpenExternalLinks(true);
		label->setTextInteractionFlags(Qt::TextBrowserInteraction);
		fitWidth(label);
		return label;
	}

	QString step(const QString &title, const QString &detail)
	{
		return QStringLiteral("<b>%1</b><br>%2").arg(title.toHtmlEscaped(), detail);
	}
}

AssistantSetupPage::AssistantSetupPage(QWidget *parent) :
	QWidget(parent)
{
	auto *outer = new QVBoxLayout(this);
	outer->setContentsMargins(0, 0, 0, 0);
	auto *scroll = new QScrollArea(this);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	outer->addWidget(scroll);
	auto *body = new QWidget(scroll);
	scroll->setWidget(body);
	auto *layout = new QVBoxLayout(body);
	layout->setContentsMargins(8, 8, 8, 8);

	m_title = new QLabel(body);
	layout->addWidget(m_title);
	m_intro = new QLabel(body);
	m_intro->setWordWrap(true);
	fitWidth(m_intro);
	layout->addWidget(m_intro);

	auto *grid = new QGridLayout();
	grid->setColumnStretch(1, 1);
	grid->setHorizontalSpacing(8);
	grid->setVerticalSpacing(14);
	int row = 0;
	auto state_label = [body]() {
		auto *label = new QLabel(body);
		label->setTextFormat(Qt::RichText);
		label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		label->setMinimumWidth(24);
		return label;
	};

		//1. API key
	m_key_state = state_label();
	m_key_text = stepText(body);
	auto *key_box = new QVBoxLayout();
	key_box->setSpacing(4);
	key_box->addWidget(m_key_text);
	auto *key_row = new QHBoxLayout();
	m_key = new QLineEdit(body);
	m_key->setEchoMode(QLineEdit::Password);
	m_key->setClearButtonEnabled(true);
	m_key->setPlaceholderText(QStringLiteral("sk-ant-…"));
	m_key->setMinimumWidth(80);
	m_test = new QPushButton(tr("Check"), body);
	m_test->setToolTip(tr("Test the key with one short request to Anthropic and store it"));
	key_row->addWidget(m_key, 1);
	key_row->addWidget(m_test);
	key_box->addLayout(key_row);
	m_remove_key = new QPushButton(tr("Remove stored key"), body);
	key_box->addWidget(m_remove_key, 0, Qt::AlignLeft);
	grid->addWidget(m_key_state, row, 0);
	grid->addLayout(key_box, row++, 1);

		//2. Python
	m_python_state = state_label();
	m_python_text = stepText(body);
	auto *python_box = new QVBoxLayout();
	python_box->setSpacing(4);
	python_box->addWidget(m_python_text);
	auto *choose = new QPushButton(tr("Choose Python…"), body);
	python_box->addWidget(choose, 0, Qt::AlignLeft);
	grid->addWidget(m_python_state, row, 0);
	grid->addLayout(python_box, row++, 1);

		//3. The package
	m_package_state = state_label();
	m_package_text = stepText(body);
	auto *package_box = new QVBoxLayout();
	package_box->setSpacing(4);
	package_box->addWidget(m_package_text);
	m_install = new QPushButton(tr("Install now"), body);
	m_install->setToolTip(tr("Runs: python -m pip install --upgrade anthropic"));
	package_box->addWidget(m_install, 0, Qt::AlignLeft);
	m_output = new QPlainTextEdit(body);
	m_output->setReadOnly(true);
	m_output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	m_output->setMinimumHeight(120);
	m_output->hide();
	package_box->addWidget(m_output);
	grid->addWidget(m_package_state, row, 0);
	grid->addLayout(package_box, row++, 1);

	m_modes = new QStackedWidget(body);
	auto *api_page = new QWidget(m_modes);
	api_page->setLayout(grid);
	grid->setContentsMargins(0, 0, 0, 0);
	m_modes->addWidget(api_page);
	m_modes->addWidget(buildKontoPage(m_modes));
	layout->addWidget(m_modes);

		//The bridge script: only shown when it is missing
	auto *bridge_row = new QHBoxLayout();
	m_bridge_state = state_label();
	m_bridge_text = stepText(body);
	bridge_row->addWidget(m_bridge_state, 0, Qt::AlignTop);
	bridge_row->addWidget(m_bridge_text, 1);
	layout->addLayout(bridge_row);

	m_summary = new QLabel(body);
	m_summary->setWordWrap(true);
	m_summary->setTextFormat(Qt::RichText);
	fitWidth(m_summary);
	layout->addSpacing(6);
	layout->addWidget(m_summary);
	auto *bottom = new QHBoxLayout();
	auto *again = new QPushButton(tr("Check again"), body);
	m_to_chat = new QPushButton(tr("To the chat"), body);
	m_to_chat->setDefault(true);
	bottom->addWidget(again);
	bottom->addStretch(1);
	bottom->addWidget(m_to_chat);
	layout->addLayout(bottom);
		//Account or key: the other way, one click away
	m_mode_switch = new QPushButton(body);
	m_mode_switch->setFlat(true);
	m_mode_switch->setCursor(Qt::PointingHandCursor);
	layout->addWidget(m_mode_switch, 0, Qt::AlignLeft);
	connect(m_mode_switch, &QPushButton::clicked, this, [this]() {
		if (m_process && m_process->state() != QProcess::NotRunning) return;
		setKontoMode(!kontoMode());
		emit settingsChanged();
		refresh(true);
	});
#ifndef QET_CLAUDE_KONTO
	m_mode_switch->hide();
#endif
	layout->addStretch(1);

	connect(m_test, &QPushButton::clicked, this, &AssistantSetupPage::testKey);
	connect(m_key, &QLineEdit::returnPressed, this, &AssistantSetupPage::testKey);
	connect(m_remove_key, &QPushButton::clicked, this, &AssistantSetupPage::removeKey);
	connect(choose, &QPushButton::clicked, this, &AssistantSetupPage::choosePython);
	connect(m_install, &QPushButton::clicked, this, &AssistantSetupPage::installPackage);
	connect(again, &QPushButton::clicked, this, [this]() { refresh(false); });
	connect(m_to_chat, &QPushButton::clicked, this, &AssistantSetupPage::chatRequested);

	if (!PortalCredentials::available()) {
		m_key->setEnabled(false);
		m_test->setEnabled(false);
	}
	setState(m_key_state, State::Unknown);
	setState(m_python_state, State::Unknown);
	setState(m_package_state, State::Unknown);
	setState(m_k_python_state, State::Unknown);
	setState(m_k_sdk_state, State::Unknown);
	setState(m_k_login_state, State::Unknown);
	m_install->hide();
	showMode();
	m_bridge_state->hide();
	m_bridge_text->hide();
}

AssistantSetupPage::~AssistantSetupPage()
{
	if (m_process && m_process->state() != QProcess::NotRunning) {
		m_process->kill();
		m_process->waitForFinished(2000);
	}
}

QString AssistantSetupPage::apiKey()
{
	return PortalCredentials::load(CREDENTIAL).secret;
}

bool AssistantSetupPage::hasApiKey()
{
	return PortalCredentials::has(CREDENTIAL);
}

bool AssistantSetupPage::keyAvailable()
{
	return hasApiKey()
		|| !qEnvironmentVariable("ANTHROPIC_API_KEY").trimmed().isEmpty();
}

bool AssistantSetupPage::quickCheck()
{
	return (kontoMode() || keyAvailable())
		&& !AssistantBridge::pythonProgram().isEmpty()
		&& !AssistantBridge::bridgeScript().isEmpty();
}

void AssistantSetupPage::refresh(bool switch_when_done)
{
	m_switch_when_done = switch_when_done;
		//Forget the last results: all green from before must not count
		//while the checks run again
	m_key_ok = m_python_ok = m_package_ok = m_bridge_ok = State::Unknown;
	m_k_python = m_k_sdk = m_k_login = State::Unknown;
	showMode();
	checkBridge();
	if (kontoMode()) {
		checkKonto();
		return;
	}
	checkKey();
	checkPython();
	checkPackage();
}

bool AssistantSetupPage::isComplete() const
{
	if (kontoMode())
		return m_k_python == State::Ok && m_k_sdk == State::Ok
			&& m_k_login == State::Ok && m_bridge_ok == State::Ok;
	return m_key_ok == State::Ok && m_python_ok == State::Ok
		&& m_package_ok == State::Ok && m_bridge_ok == State::Ok;
}

void AssistantSetupPage::setState(QLabel *label, State state)
{
	switch (state) {
		case State::Ok:
			label->setText(QStringLiteral("<span style=\"color:#2e9d3a; font-size:16pt;\">✔</span>"));
			label->setToolTip(tr("done"));
			break;
		case State::Failed:
			label->setText(QStringLiteral("<span style=\"color:#d23c3c; font-size:16pt;\">✘</span>"));
			label->setToolTip(tr("still to do"));
			break;
		case State::Checking:
			label->setText(QStringLiteral("<span style=\"color:gray; font-size:16pt;\">…</span>"));
			label->setToolTip(tr("checking…"));
			break;
		case State::Unknown:
			label->setText(QStringLiteral("<span style=\"color:gray; font-size:16pt;\">○</span>"));
			label->setToolTip(QString());
			break;
	}
}

void AssistantSetupPage::updateCompletion()
{
	const bool complete = isComplete();
	m_to_chat->setEnabled(complete);
	if (complete)
		m_summary->setText(QStringLiteral("<b style=\"color:#2e9d3a;\">%1</b>")
				   .arg(tr("All set: Claude is ready.").toHtmlEscaped()));
	else
		m_summary->setText(tr("When every step is green, the chat opens by itself.").toHtmlEscaped());
		//Something the user had to do: when it is done, on to the chat
	const bool konto = kontoMode();
	const State states[] = {konto ? m_k_python : m_python_ok, konto ? m_k_sdk : m_key_ok,
				konto ? m_k_login : m_package_ok, m_bridge_ok};
	for (State state : states)
		if (state == State::Failed) m_switch_when_done = true;
	if (complete && m_switch_when_done) {
		m_switch_when_done = false;
		emit completed();
	}
}

	//1. API key

void AssistantSetupPage::checkKey()
{
	const QString title = tr("1. API key");
	const QString link = QStringLiteral("<a href=\"https://console.anthropic.com/settings/keys\">console.anthropic.com</a>");
	m_remove_key->setVisible(hasApiKey());
	if (hasApiKey()) {
		m_key_ok = State::Ok;
		m_key_text->setText(step(title,
			tr("A key is stored in the Windows Credential Manager. To change it, "
			   "enter a new one and press Check.").toHtmlEscaped()));
	} else if (!qEnvironmentVariable("ANTHROPIC_API_KEY").trimmed().isEmpty()) {
		m_key_ok = State::Ok;
		m_key_text->setText(step(title,
			tr("The key comes from the environment variable ANTHROPIC_API_KEY.").toHtmlEscaped()));
	} else if (!PortalCredentials::available()) {
		m_key_ok = State::Failed;
		m_key_text->setText(step(title,
			tr("This system has no credential store: set the environment variable "
			   "ANTHROPIC_API_KEY and start QElectroTech again.").toHtmlEscaped()));
	} else {
		m_key_ok = State::Failed;
		m_key_text->setText(step(title,
			tr("Create a key at %1 (sign in, then “API Keys” › “Create Key”), "
			   "paste it here and press Check. It is kept in the Windows "
			   "Credential Manager, never in a file.").arg(link)));
	}
	setState(m_key_state, m_key_ok);
	updateCompletion();
}

void AssistantSetupPage::testKey()
{
	const QString key = m_key->text().trimmed();
	if (key.isEmpty()) {
		m_key->setFocus();
		return;
	}
	if (!m_network) m_network = new QNetworkAccessManager(this);
		//The cheapest request there is: list one model. It costs no
		//tokens and fails with 401 for a wrong key.
	QNetworkRequest request(QUrl(QStringLiteral("https://api.anthropic.com/v1/models?limit=1")));
	request.setRawHeader("x-api-key", key.toUtf8());
	request.setRawHeader("anthropic-version", "2023-06-01");
	request.setTransferTimeout(20000);
	m_test->setEnabled(false);
	m_key->setEnabled(false);
	setState(m_key_state, State::Checking);
	QNetworkReply *reply = m_network->get(request);
	connect(reply, &QNetworkReply::finished, this, [this, reply, key]() { keyTested(reply, key); });
}

void AssistantSetupPage::keyTested(QNetworkReply *reply, const QString &key)
{
	reply->deleteLater();
	m_test->setEnabled(true);
	m_key->setEnabled(true);
	const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	QString problem;
	if (status == 200) {
		PortalAccount account;
		account.user = QStringLiteral("api-key");
		account.secret = key;
		QString error;
		if (PortalCredentials::store(CREDENTIAL, account, &error)) {
			m_key->clear();
			checkKey();
			emit settingsChanged();
			return;
		}
		problem = tr("The key is valid but could not be stored: %1").arg(error);
	} else if (status == 401) {
		problem = tr("Anthropic does not accept this key. Copy it again from the console, "
			     "completely.");
	} else if (status == 403) {
		problem = tr("The key has no permission for the API (403).");
	} else if (status == 429 || status == 529) {
		problem = tr("Anthropic is busy right now (%1). Please try again in a moment.").arg(status);
	} else if (status > 0) {
		problem = tr("Anthropic answered with error %1.").arg(status);
	} else {
		problem = tr("No connection to api.anthropic.com: %1").arg(reply->errorString());
	}
	m_key_ok = State::Failed;
	setState(m_key_state, State::Failed);
	m_key_text->setText(step(tr("1. API key"),
		QStringLiteral("<span style=\"color:#d23c3c;\">%1</span>").arg(problem.toHtmlEscaped())));
	updateCompletion();
}

void AssistantSetupPage::removeKey()
{
	if (!hasApiKey()) return;
	if (QMessageBox::question(this, tr("Claude"), tr("Remove the stored API key?"))
	    != QMessageBox::Yes)
		return;
	QString error;
	if (!PortalCredentials::remove(CREDENTIAL, &error))
		QMessageBox::warning(this, tr("Claude"), tr("The key could not be removed: %1").arg(error));
	checkKey();
	emit settingsChanged();
}

	//2. Python

void AssistantSetupPage::checkPython()
{
	const QString title = kontoMode() ? tr("1. Python") : tr("2. Python");
	bool store_shortcut = false;
	const QString python = AssistantBridge::pythonProgram(&store_shortcut);
	const QString configured = QSettings().value(PYTHON_SETTING).toString().trimmed();
	const QString link = QStringLiteral("<a href=\"https://www.python.org/downloads/\">python.org</a>");
	if (!configured.isEmpty() && !QFileInfo(configured).isFile()
	    && QStandardPaths::findExecutable(configured).isEmpty()) {
		m_python_ok = State::Failed;
		m_python_text->setText(step(title,
			tr("The chosen program “%1” does not exist. Choose another one.")
			.arg(configured).toHtmlEscaped()));
	} else if (python.isEmpty()) {
		m_python_ok = State::Failed;
		m_python_text->setText(step(title,
			tr("No Python found. Install Python 3 from %1 (tick “Add python.exe to PATH”), "
			   "then press Check again.").arg(link)));
	} else if (store_shortcut) {
		m_python_ok = State::Failed;
		m_python_text->setText(step(title,
			tr("Only Windows' shortcut to the Microsoft Store was found. Install Python 3 "
			   "from %1, then press Check again.").arg(link)));
	} else {
		m_python_ok = State::Ok;
		m_python_text->setText(step(title,
			tr("Found: %1").arg(QDir::toNativeSeparators(python)).toHtmlEscaped()));
	}
	setState(m_python_state, m_python_ok);
	updateCompletion();
}

void AssistantSetupPage::choosePython()
{
	const QString file = QFileDialog::getOpenFileName(this, tr("Python program"), QString(),
#ifdef Q_OS_WIN
							  tr("Programs (*.exe)")
#else
							  QString()
#endif
							  );
	if (file.isEmpty()) return;
	QSettings().setValue(PYTHON_SETTING, file);
	emit settingsChanged();
	refresh(false);
}

	//3. The package

void AssistantSetupPage::checkPackage()
{
	const QString title = tr("3. Python package “anthropic”");
	if (m_python_ok != State::Ok) {
		m_package_ok = State::Unknown;
		m_package_text->setText(step(title, tr("Is checked once Python is there.").toHtmlEscaped()));
		m_install->hide();
		setState(m_package_state, m_package_ok);
		updateCompletion();
		return;
	}
	if (m_process && m_process->state() != QProcess::NotRunning) return;
	const QString python = AssistantBridge::pythonProgram();
	delete m_process;
	m_process = new QProcess(this);
	m_package_ok = State::Checking;
	setState(m_package_state, m_package_ok);
	m_package_text->setText(step(title, tr("checking…").toHtmlEscaped()));
	m_install->hide();
	updateCompletion();
	QProcess *process = m_process;
	connect(process, &QProcess::finished, this, [this, process, title](int code, QProcess::ExitStatus status) {
		const QString version = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
		if (status == QProcess::NormalExit && code == 0 && !version.isEmpty()) {
			m_package_ok = State::Ok;
			m_package_text->setText(step(title,
				tr("Installed, version %1.").arg(version).toHtmlEscaped()));
			m_install->hide();
		} else {
			m_package_ok = State::Failed;
			m_package_text->setText(step(title,
				tr("Not installed yet. The button installs it with pip "
				   "(takes about a minute, needs internet).").toHtmlEscaped()));
			m_install->show();
			m_install->setEnabled(true);
		}
		setState(m_package_state, m_package_ok);
		updateCompletion();
	});
	connect(process, &QProcess::errorOccurred, this, [this, title](QProcess::ProcessError error) {
		if (error != QProcess::FailedToStart) return;
		m_package_ok = State::Failed;
		m_package_text->setText(step(title, tr("Python could not be started.").toHtmlEscaped()));
		setState(m_package_state, m_package_ok);
		updateCompletion();
	});
	process->start(python, AssistantBridge::pythonArguments(python)
		       << QStringLiteral("-c")
		       << QStringLiteral("import anthropic; print(anthropic.__version__)"));
}

void AssistantSetupPage::installPackage()
{
	if (m_python_ok != State::Ok) return;
	if (m_process && m_process->state() != QProcess::NotRunning) return;
	const QString python = AssistantBridge::pythonProgram();
	const QStringList args = AssistantBridge::pythonArguments(python)
		<< QStringLiteral("-m") << QStringLiteral("pip") << QStringLiteral("install")
		<< QStringLiteral("--upgrade") << QStringLiteral("--disable-pip-version-check")
		<< QStringLiteral("anthropic");
	delete m_process;
	m_process = new QProcess(this);
	m_process->setProcessChannelMode(QProcess::MergedChannels);
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert(QStringLiteral("PYTHONIOENCODING"), QStringLiteral("utf-8"));
	m_process->setProcessEnvironment(env);
	m_output->clear();
	m_output->show();
	m_output->appendPlainText(QStringLiteral("> %1 %2\n")
				  .arg(QDir::toNativeSeparators(python), args.join(QLatin1Char(' '))));
	m_install->setEnabled(false);
	m_package_ok = State::Checking;
	setState(m_package_state, m_package_ok);
	m_package_text->setText(step(tr("3. Python package “anthropic”"), tr("installing…").toHtmlEscaped()));
	updateCompletion();
	QProcess *process = m_process;
	connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
		m_output->moveCursor(QTextCursor::End);
		m_output->insertPlainText(QString::fromUtf8(process->readAllStandardOutput()));
		m_output->moveCursor(QTextCursor::End);
	});
	connect(process, &QProcess::finished, this, [this](int code, QProcess::ExitStatus) {
		m_output->appendPlainText(code == 0 ? tr("Done.") : tr("pip ended with code %1.").arg(code));
			//Not from inside the finished signal: checkPackage replaces the process
		QMetaObject::invokeMethod(this, &AssistantSetupPage::checkPackage, Qt::QueuedConnection);
	});
	connect(process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
		if (error != QProcess::FailedToStart) return;
		m_output->appendPlainText(tr("Python could not be started."));
		m_install->setEnabled(true);
	});
	process->start(python, args);
}

	//The bridge script

void AssistantSetupPage::checkBridge()
{
	const QString script = AssistantBridge::bridgeScript();
	m_bridge_ok = script.isEmpty() ? State::Failed : State::Ok;
	m_bridge_state->setVisible(script.isEmpty());
	m_bridge_text->setVisible(script.isEmpty());
	if (script.isEmpty())
		m_bridge_text->setText(step(tr("Program part missing"),
			tr("mcp/qet-assistent/bruecke.py was not found beside QElectroTech. "
			   "Install QElectroTech again.").toHtmlEscaped()));
	setState(m_bridge_state, m_bridge_ok);
	updateCompletion();
}

	//Claude account (QET_CLAUDE_KONTO): the Claude Agent SDK starts its
	//Claude Code, signed in with the subscription of the user; no API key

bool AssistantSetupPage::kontoMode()
{
#ifdef QET_CLAUDE_KONTO
	return QSettings().value(QStringLiteral("assistant/mode"), QStringLiteral("konto")).toString()
		== QLatin1String("konto");
#else
	return false;
#endif
}

void AssistantSetupPage::setKontoMode(bool konto)
{
	QSettings().setValue(QStringLiteral("assistant/mode"),
			     konto ? QStringLiteral("konto") : QStringLiteral("api"));
}

QWidget *AssistantSetupPage::buildKontoPage(QWidget *parent)
{
	auto *page = new QWidget(parent);
	auto *layout = new QVBoxLayout(page);
	layout->setContentsMargins(0, 0, 0, 0);
	m_connect = new QPushButton(AssistantDock::icon(), tr("Connect with Claude"), page);
	m_connect->setMinimumHeight(36);
	QFont font = m_connect->font();
	font.setBold(true);
	m_connect->setFont(font);
	m_connect->setToolTip(tr("Installs what is missing and signs in with your Claude account "
				 "in the browser"));
	layout->addWidget(m_connect);
	layout->addSpacing(6);

	auto *grid = new QGridLayout();
	grid->setColumnStretch(1, 1);
	grid->setHorizontalSpacing(8);
	grid->setVerticalSpacing(10);
	auto row = [page, grid](int r, QLabel **state, QLabel **text) {
		*state = new QLabel(page);
		(*state)->setTextFormat(Qt::RichText);
		(*state)->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		(*state)->setMinimumWidth(24);
		*text = stepText(page);
		grid->addWidget(*state, r, 0);
		grid->addWidget(*text, r, 1);
	};
	row(0, &m_k_python_state, &m_k_python_text);
	row(1, &m_k_sdk_state, &m_k_sdk_text);
	row(2, &m_k_login_state, &m_k_login_text);
	layout->addLayout(grid);

	m_k_link = stepText(page);
	m_k_link->hide();
	layout->addWidget(m_k_link);
	m_k_output = new QPlainTextEdit(page);
	m_k_output->setReadOnly(true);
	m_k_output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	m_k_output->setMinimumHeight(110);
	m_k_output->hide();
	layout->addWidget(m_k_output);
	connect(m_connect, &QPushButton::clicked, this, &AssistantSetupPage::connectKonto);
	return page;
}

void AssistantSetupPage::showMode()
{
	const bool konto = kontoMode();
	m_modes->setCurrentIndex(konto ? 1 : 0);
	if (konto) {
		m_title->setText(QStringLiteral("<h3>%1</h3>").arg(tr("Connect Claude").toHtmlEscaped()));
		m_intro->setText(tr("Claude answers questions about the open drawing and draws for you. "
				    "It uses your Claude subscription (Pro or Max): no API key and no "
				    "extra costs, it counts against the usage of your plan. One click on "
				    "the button sets everything up.").toHtmlEscaped());
		m_mode_switch->setText(tr("Use an API key instead"));
	} else {
		m_title->setText(QStringLiteral("<h3>%1</h3>").arg(tr("Set up Claude").toHtmlEscaped()));
		m_intro->setText(tr("Claude answers questions about the open drawing and draws "
				    "for you. Three things are needed once; each step turns green "
				    "when it is done.").toHtmlEscaped());
		m_mode_switch->setText(tr("Use my Claude account instead (no API costs)"));
	}
}

void AssistantSetupPage::checkKonto()
{
	checkPython();
	m_k_python = m_python_ok;
	setState(m_k_python_state, m_k_python);
	m_k_python_text->setText(m_python_text->text());
	const QString sdk_title = tr("2. Claude for QElectroTech");
	const QString login_title = tr("3. Signed in with your Claude account");
	if (m_k_python != State::Ok || m_bridge_ok != State::Ok) {
		m_k_sdk = m_k_login = State::Unknown;
		m_k_sdk_text->setText(step(sdk_title, tr("Is checked once Python is there.").toHtmlEscaped()));
		m_k_login_text->setText(step(login_title, QString()));
		setState(m_k_sdk_state, m_k_sdk);
		setState(m_k_login_state, m_k_login);
		updateCompletion();
		if (m_connecting) stopConnecting(tr("Python is missing: install it first (step 1)."));
		return;
	}
	if (m_process && m_process->state() != QProcess::NotRunning) return;
	m_k_sdk = m_k_login = State::Checking;
	setState(m_k_sdk_state, m_k_sdk);
	setState(m_k_login_state, m_k_login);
	updateCompletion();

	const QString python = AssistantBridge::pythonProgram();
	delete m_process;
	m_process = new QProcess(this);
	QProcess *process = m_process;
	connect(process, &QProcess::finished, this, [=](int, QProcess::ExitStatus) {
		QJsonObject status;
		const QList<QByteArray> lines = process->readAllStandardOutput().split('\n');
		for (const QByteArray &line : lines) {
			const QJsonDocument doc = QJsonDocument::fromJson(line.trimmed());
			if (doc.isObject()) status = doc.object();
		}
		const bool sdk = status.value(QStringLiteral("sdk")).toBool();
		const bool logged_in = status.value(QStringLiteral("loggedIn")).toBool();
		m_k_sdk = sdk ? State::Ok : State::Failed;
		m_k_sdk_text->setText(step(sdk_title, sdk
			? tr("Installed (claude-agent-sdk %1).")
			  .arg(status.value(QStringLiteral("version")).toString()).toHtmlEscaped()
			: tr("Not installed yet: the button above installs it.").toHtmlEscaped()));
		if (!sdk) {
			m_k_login = State::Unknown;
			m_k_login_text->setText(step(login_title, QString()));
		} else if (logged_in) {
			m_k_login = State::Ok;
			const QString email = status.value(QStringLiteral("email")).toString();
			const QString plan = status.value(QStringLiteral("subscriptionType")).toString();
			QString who = email.isEmpty() ? tr("Connected.") : tr("Connected as %1.").arg(email);
			if (!plan.isEmpty()) who += QLatin1Char(' ') + tr("Plan: %1.").arg(plan);
			m_k_login_text->setText(step(login_title, who.toHtmlEscaped()));
		} else {
			m_k_login = State::Failed;
			QString text = tr("Not connected yet: the button above opens the sign-in page "
					  "of Claude in your browser.");
			const QString error = status.value(QStringLiteral("error")).toString();
			if (!error.isEmpty()) text += QLatin1Char(' ') + error;
			m_k_login_text->setText(step(login_title, text.toHtmlEscaped()));
		}
		setState(m_k_sdk_state, m_k_sdk);
		setState(m_k_login_state, m_k_login);
		updateCompletion();
		if (m_connecting)
			QMetaObject::invokeMethod(this, &AssistantSetupPage::kontoNext, Qt::QueuedConnection);
	});
	connect(process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
		if (error != QProcess::FailedToStart) return;
		m_k_sdk = State::Failed;
		setState(m_k_sdk_state, m_k_sdk);
		updateCompletion();
		if (m_connecting) stopConnecting(tr("Python could not be started."));
	});
	process->start(python, AssistantBridge::pythonArguments(python)
		       << AssistantBridge::bridgeScript() << QStringLiteral("--konto-status"));
}

void AssistantSetupPage::connectKonto()
{
	if (m_connecting) {
			//The button reads "Cancel" while connecting
		if (m_process && m_process->state() != QProcess::NotRunning) m_process->kill();
		stopConnecting(tr("Cancelled."));
		return;
	}
	m_connecting = true;
	m_tried_install = m_tried_login = false;
	m_connect->setText(tr("Cancel"));
	m_k_output->clear();
	m_k_output->show();
	m_k_link->hide();
	m_switch_when_done = true;
	if (m_process && m_process->state() != QProcess::NotRunning) return;	//its end goes on
	checkKonto();
}

/**
	@brief AssistantSetupPage::kontoNext
	One step of "Connect with Claude" after each check: install the
	package, then sign in, then done. Each is tried once per click.
*/
void AssistantSetupPage::kontoNext()
{
	if (!m_connecting) return;
	if (m_k_python != State::Ok) {
		stopConnecting(tr("Python is missing: install it first (step 1)."));
	} else if (m_k_sdk != State::Ok) {
		if (m_tried_install) {
			stopConnecting(tr("The package could not be installed: see the output above."));
			return;
		}
		m_tried_install = true;
		runKonto(QStringList{QStringLiteral("-m"), QStringLiteral("pip"), QStringLiteral("install"),
				     QStringLiteral("--upgrade"), QStringLiteral("--disable-pip-version-check"),
				     QStringLiteral("claude-agent-sdk")},
			 tr("Installing Claude for QElectroTech (about a minute)…"));
	} else if (m_k_login != State::Ok) {
		if (m_tried_login) {
			stopConnecting(tr("Not signed in. Press the button to try again."));
			return;
		}
		m_tried_login = true;
		runKonto(QStringList{AssistantBridge::bridgeScript(), QStringLiteral("--konto-login")},
			 tr("Your browser opens the Claude sign-in page. Sign in there and allow "
			    "the access; this page goes on by itself."));
	} else {
		stopConnecting(tr("Connected."));
	}
}

void AssistantSetupPage::runKonto(const QStringList &args, const QString &what)
{
	const QString python = AssistantBridge::pythonProgram();
	const QStringList all = AssistantBridge::pythonArguments(python) + args;
	delete m_process;
	m_process = new QProcess(this);
	m_process->setProcessChannelMode(QProcess::MergedChannels);
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert(QStringLiteral("PYTHONIOENCODING"), QStringLiteral("utf-8"));
	env.remove(QStringLiteral("ANTHROPIC_API_KEY"));
	m_process->setProcessEnvironment(env);
	m_k_output->appendPlainText(what);
	QProcess *process = m_process;
	connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
		const QString text = QString::fromUtf8(process->readAllStandardOutput());
		m_k_output->moveCursor(QTextCursor::End);
		m_k_output->insertPlainText(text);
		m_k_output->moveCursor(QTextCursor::End);
			//The sign-in link, in case the browser did not open
		static const QRegularExpression url(QStringLiteral("https://\\S+"));
		const QRegularExpressionMatch match = url.match(text);
		if (match.hasMatch()) {
			m_k_link->setText(QStringLiteral("<a href=\"%1\">%2</a>")
					  .arg(match.captured(0).toHtmlEscaped(),
					       tr("Open the sign-in page").toHtmlEscaped()));
			m_k_link->show();
		}
	});
	connect(process, &QProcess::finished, this, [this](int, QProcess::ExitStatus) {
		QMetaObject::invokeMethod(this, &AssistantSetupPage::checkKonto, Qt::QueuedConnection);
	});
	connect(process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
		if (error == QProcess::FailedToStart) stopConnecting(tr("Python could not be started."));
	});
	process->start(python, all);
}

void AssistantSetupPage::stopConnecting(const QString &message)
{
	m_connecting = false;
	m_connect->setText(tr("Connect with Claude"));
	if (!message.isEmpty()) m_k_output->appendPlainText(message);
	if (isComplete()) emit settingsChanged();
}
