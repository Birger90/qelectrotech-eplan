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
#include "assistantdock.h"

#include "assistantbridge.h"
#include "assistantsettingsdialog.h"
#include "../diagram.h"
#include "../diagramview.h"
#include "../projectview.h"
#include "../qetapp.h"
#include "../qetdiagrameditor.h"
#include "../qetproject.h"
#include "../scripting/liveserver.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

AssistantDock::AssistantDock(QETDiagramEditor *editor) :
	QDockWidget(tr("Claude assistant"), editor),
	m_editor(editor),
	m_bridge(new AssistantBridge(this))
{
	// not "assistant_dock": that is the live mode's log dock, and two docks
	// with one name mix up their places when the window state is restored
	setObjectName(QStringLiteral("claude_assistant_dock"));
	auto *body = new QWidget(this);
	auto *layout = new QVBoxLayout(body);
	layout->setContentsMargins(4, 4, 4, 4);

	auto *top = new QHBoxLayout();
	m_activity = new QLabel(body);
	m_activity->setWordWrap(true);
	m_activity->setStyleSheet(QStringLiteral("color: palette(mid);"));
	m_new = new QToolButton(body);
	m_new->setText(tr("New"));
	m_new->setToolTip(tr("Start a new conversation"));
	m_settings = new QToolButton(body);
	m_settings->setText(tr("Settings…"));
	m_settings->setToolTip(tr("API key and Python of the assistant"));
	top->addWidget(m_activity, 1);
	top->addWidget(m_new);
	top->addWidget(m_settings);
	layout->addLayout(top);

	m_history = new QTextBrowser(body);
	m_history->setOpenExternalLinks(true);
	m_history->setPlaceholderText(tr("Ask about the open drawing, or tell Claude what to "
					 "draw. Every change Claude makes is one undo step."));
	layout->addWidget(m_history, 1);

	m_input = new QPlainTextEdit(body);
	m_input->setPlaceholderText(tr("Your question… (Enter sends, Shift+Enter: new line)"));
	m_input->setMaximumHeight(90);
	m_input->installEventFilter(this);
	layout->addWidget(m_input);

	auto *buttons = new QHBoxLayout();
	m_cancel = new QPushButton(tr("Stop"), body);
	m_cancel->setToolTip(tr("Stop the answer"));
	m_cancel->setEnabled(false);
	m_send = new QPushButton(tr("Send"), body);
	m_send->setDefault(true);
	buttons->addStretch(1);
	buttons->addWidget(m_cancel);
	buttons->addWidget(m_send);
	layout->addLayout(buttons);
	setWidget(body);

	m_render_timer = new QTimer(this);
	m_render_timer->setSingleShot(true);
	m_render_timer->setInterval(80);
	connect(m_render_timer, &QTimer::timeout, this, &AssistantDock::render);

	connect(m_send, &QPushButton::clicked, this, &AssistantDock::sendQuestion);
	connect(m_cancel, &QPushButton::clicked, this, &AssistantDock::cancel);
	connect(m_new, &QToolButton::clicked, this, &AssistantDock::newConversation);
	connect(m_settings, &QToolButton::clicked, this, &AssistantDock::openSettings);
	connect(m_bridge, &AssistantBridge::message, this, &AssistantDock::onMessage);
	connect(m_bridge, &AssistantBridge::stopped, this, &AssistantDock::onStopped);
}

AssistantDock::~AssistantDock()
{
	m_bridge->stop();
	LiveServer::instance().closeForAssistant();
}

bool AssistantDock::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == m_input && event->type() == QEvent::KeyPress) {
		auto *key = static_cast<QKeyEvent *>(event);
		if ((key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter)
		    && !(key->modifiers() & Qt::ShiftModifier)) {
			if (!m_busy) sendQuestion();
			return true;
		}
	}
	return QDockWidget::eventFilter(watched, event);
}

/**
	@brief AssistantDock::ensureBridge
	Start the bridge if it is not running: its own live channel, the key
	from the credential store (to the child only), this QElectroTech.
*/
bool AssistantDock::ensureBridge()
{
	if (m_bridge->isRunning()) return true;

	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	QString key = AssistantSettingsDialog::apiKey();
	if (key.isEmpty() && env.value(QStringLiteral("ANTHROPIC_API_KEY")).isEmpty()) {
		AssistantSettingsDialog dialog(this);
		dialog.exec();
		key = AssistantSettingsDialog::apiKey();
		if (key.isEmpty()) {
			add(Entry::Error, tr("No API key: the assistant cannot answer without one."));
			return false;
		}
	}
	if (!key.isEmpty()) env.insert(QStringLiteral("ANTHROPIC_API_KEY"), key);

	QString socket_name, token;
	if (!LiveServer::instance().openForAssistant(&socket_name, &token)) {
		add(Entry::Error, tr("The channel to the drawing could not be opened."));
		return false;
	}
	env.insert(QStringLiteral("QET_LIVE_SOCKET"), socket_name);
	env.insert(QStringLiteral("QET_LIVE_TOKEN"), token);
	env.insert(QStringLiteral("QET_BINARY"), QCoreApplication::applicationFilePath());
	env.insert(QStringLiteral("PYTHONIOENCODING"), QStringLiteral("utf-8"));
	env.insert(QStringLiteral("PYTHONUTF8"), QStringLiteral("1"));
		//What the tools may read: the element collections and the folder
		//of the open project
	QStringList workspace{QETApp::commonElementsDir(), QETApp::companyElementsDir(),
			      QETApp::customElementsDir()};
	if (QETProject *project = m_editor->currentProject()) {
		if (!project->filePath().isEmpty())
			workspace << QFileInfo(project->filePath()).absolutePath();
	}
	env.insert(QStringLiteral("QET_MCP_WORKSPACE"), workspace.join(QDir::listSeparator()));

	QString error;
	if (!m_bridge->start(env, &error)) {
		add(Entry::Error, error);
		return false;
	}
	return true;
}

QJsonObject AssistantDock::context() const
{
	QJsonObject context;
	QETProject *project = m_editor->currentProject();
	if (!project) {
		context.insert(QStringLiteral("projekt"), QStringLiteral("keins offen"));
		return context;
	}
	context.insert(QStringLiteral("projekt"), project->title());
	if (!project->filePath().isEmpty())
		context.insert(QStringLiteral("datei"), QDir::toNativeSeparators(project->filePath()));
	context.insert(QStringLiteral("blaetter"), static_cast<int>(project->diagrams().size()));
	ProjectView *project_view = m_editor->currentProjectView();
	if (DiagramView *view = project_view ? project_view->currentDiagram() : nullptr) {
		if (Diagram *diagram = view->diagram()) {
			context.insert(QStringLiteral("blatt_index_ab_0"), project->folioIndex(diagram));
			context.insert(QStringLiteral("blatt_titel"), diagram->title());
			context.insert(QStringLiteral("auswahl"),
				       static_cast<int>(diagram->selectedItems().size()));
		}
	}
	return context;
}

void AssistantDock::sendQuestion()
{
	const QString text = m_input->toPlainText().trimmed();
	if (text.isEmpty() || m_busy) return;
	if (!ensureBridge()) return;
	m_input->clear();
	add(Entry::User, text);
	m_answer_open = false;
	setBusy(true);
	m_activity->setText(tr("thinking…"));
	m_bridge->send(QJsonObject{{QStringLiteral("type"), QStringLiteral("ask")},
				   {QStringLiteral("text"), text},
				   {QStringLiteral("kontext"), context()}});
}

void AssistantDock::cancel()
{
	if (!m_busy) return;
	m_bridge->send(QJsonObject{{QStringLiteral("type"), QStringLiteral("cancel")}});
	m_activity->setText(tr("stopping…"));
}

void AssistantDock::newConversation()
{
	if (m_bridge->isRunning())
		m_bridge->send(QJsonObject{{QStringLiteral("type"), QStringLiteral("reset")}});
		//The yes to changes was for that conversation
	LiveServer::instance().setAssistantWritesAllowed(false);
	m_entries.clear();
	m_answer_open = false;
	setBusy(false);
	m_activity->clear();
	render();
}

void AssistantDock::openSettings()
{
	AssistantSettingsDialog dialog(this);
	if (dialog.exec() == QDialog::Accepted && m_bridge->isRunning() && !m_busy) {
			//A new key or Python takes effect with a new bridge
		m_bridge->stop();
		LiveServer::instance().setAssistantWritesAllowed(false);
		add(Entry::Note, tr("Settings saved; the next question starts a new conversation."));
	}
}

void AssistantDock::onMessage(const QJsonObject &message)
{
	const QString type = message.value(QStringLiteral("type")).toString();
	if (type == QLatin1String("text")) {
		appendToAnswer(message.value(QStringLiteral("delta")).toString());
		m_activity->setText(tr("writing…"));
	} else if (type == QLatin1String("status")) {
		m_activity->setText(message.value(QStringLiteral("text")).toString());
	} else if (type == QLatin1String("tool")) {
		const QString label = message.value(QStringLiteral("label")).toString();
		const QString name = message.value(QStringLiteral("name")).toString();
		if (message.value(QStringLiteral("phase")).toString() == QLatin1String("start")) {
			m_activity->setText(tr("Claude %1 (%2)").arg(label, name));
		} else {
			const bool ok = message.value(QStringLiteral("ok")).toBool();
			m_answer_open = false;
			add(Entry::Note, (ok ? QStringLiteral("✓ ") : QStringLiteral("✗ ")) + label);
		}
	} else if (type == QLatin1String("confirm")) {
		confirm(message);
	} else if (type == QLatin1String("error")) {
		m_answer_open = false;
		add(Entry::Error, message.value(QStringLiteral("message")).toString());
	} else if (type == QLatin1String("done")) {
		m_answer_open = false;
		setBusy(false);
		const QString stop = message.value(QStringLiteral("stop_reason")).toString();
		m_activity->setText(stop == QLatin1String("cancelled") ? tr("stopped") : QString());
		if (stop == QLatin1String("cancelled")) add(Entry::Note, tr("Stopped."));
	} else if (type == QLatin1String("ready")) {
		if (!message.value(QStringLiteral("key")).toBool())
			add(Entry::Error, tr("The bridge has no API key: see Settings."));
		m_activity->setText(tr("connected (%1)").arg(message.value(QStringLiteral("model")).toString()));
	}
}

/**
	@brief AssistantDock::confirm
	Before the first change of a conversation: show what Claude wants to do
	and ask. A yes holds until "New" (and is what lets the live channel
	run the change at all).
*/
void AssistantDock::confirm(const QJsonObject &message)
{
	const QString label = message.value(QStringLiteral("label")).toString();
	const QString detail = message.value(QStringLiteral("detail")).toString();
	const QString source = message.value(QStringLiteral("source")).toString();
	QMessageBox box(QMessageBox::Question, tr("Claude wants to change the drawing"),
			tr("Claude %1%2.\n\nEach change is its own undo step (Ctrl+Z). "
			   "Allow Claude to change the drawing in this conversation?")
			.arg(label, detail.isEmpty() ? QString() : QStringLiteral(": „%1“").arg(detail)),
			QMessageBox::NoButton, this);
	QPushButton *allow = box.addButton(tr("&Allow"), QMessageBox::AcceptRole);
	box.addButton(tr("&Decline"), QMessageBox::RejectRole);
	box.setDefaultButton(allow);
	if (!source.isEmpty()) box.setDetailedText(source);
	box.exec();
	const bool yes = box.clickedButton() == allow;
	LiveServer::instance().setAssistantWritesAllowed(yes);
	if (!yes) add(Entry::Note, tr("Change declined."));
	m_bridge->send(QJsonObject{{QStringLiteral("type"), QStringLiteral("confirm_answer")},
				   {QStringLiteral("id"), message.value(QStringLiteral("id"))},
				   {QStringLiteral("allow"), yes}});
}

void AssistantDock::onStopped(const QString &why)
{
	QString text = why;
	const QStringList errors = m_bridge->recentErrors();
	if (!errors.isEmpty())
		text += QStringLiteral("\n\n") + errors.mid(qMax(0, int(errors.size()) - 5)).join(QLatin1Char('\n'));
	add(Entry::Error, text);
	LiveServer::instance().setAssistantWritesAllowed(false);
	setBusy(false);
	m_activity->clear();
}

void AssistantDock::add(Entry::Kind kind, const QString &text)
{
	m_entries.append(Entry{kind, text});
	render();
}

void AssistantDock::appendToAnswer(const QString &text)
{
	if (!m_answer_open || m_entries.isEmpty() || m_entries.last().kind != Entry::Assistant) {
		m_entries.append(Entry{Entry::Assistant, QString()});
		m_answer_open = true;
	}
	m_entries.last().text += text;
	if (!m_render_timer->isActive()) m_render_timer->start();
}

void AssistantDock::render()
{
	QString markdown;
	for (const Entry &entry : std::as_const(m_entries)) {
		switch (entry.kind) {
			case Entry::User:
				markdown += QStringLiteral("**%1:** ").arg(tr("You"))
					    + entry.text + QStringLiteral("\n\n");
				break;
			case Entry::Assistant:
				markdown += QStringLiteral("**Claude:** ") + entry.text + QStringLiteral("\n\n");
				break;
			case Entry::Note:
				markdown += QStringLiteral("*") + entry.text + QStringLiteral("*\n\n");
				break;
			case Entry::Error:
				markdown += QStringLiteral("**%1:** ").arg(tr("Error"))
					    + entry.text + QStringLiteral("\n\n");
				break;
		}
	}
	QScrollBar *bar = m_history->verticalScrollBar();
	const bool at_end = bar->value() >= bar->maximum() - 4;
	m_history->setMarkdown(markdown);
	if (at_end) bar->setValue(bar->maximum());
}

void AssistantDock::setBusy(bool busy)
{
	m_busy = busy;
	m_send->setEnabled(!busy);
	m_cancel->setEnabled(busy);
	m_new->setEnabled(!busy);
	m_settings->setEnabled(!busy);
}
