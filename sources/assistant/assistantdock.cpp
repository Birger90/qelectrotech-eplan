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
#include "assistantsetuppage.h"
#include "../diagram.h"
#include "../diagramview.h"
#include "../projectview.h"
#include "../qetapp.h"
#include "../qetdiagrameditor.h"
#include "../qetproject.h"
#include "../scripting/liveserver.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QFileDialog>
#include <QImage>
#include <QImageReader>
#include <QMimeData>
#include <QMimeDatabase>
#include <QStyle>
#include <QUrl>

#include <functional>
#include <QtMath>
#include <cmath>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSettings>
#include <QStackedWidget>
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

namespace {
		/// the input field: pictures and files pasted or dragged in go to
		/// the dock as attachments, text stays text
	class AttachInput : public QPlainTextEdit
	{
		public:
			using QPlainTextEdit::QPlainTextEdit;
			std::function<bool(const QMimeData *)> take;

		protected:
			bool canInsertFromMimeData(const QMimeData *source) const override
			{
				if (source->hasImage()) return true;
				for (const QUrl &url : source->urls())
					if (url.isLocalFile()) return true;
				return QPlainTextEdit::canInsertFromMimeData(source);
			}
			void insertFromMimeData(const QMimeData *source) override
			{
				if (take && take(source)) return;
				QPlainTextEdit::insertFromMimeData(source);
			}
	};

	constexpr int MAX_ATTACHMENTS = 10;
	constexpr qint64 MAX_PDF_BYTES = 20 * 1024 * 1024;
	constexpr int MAX_IMAGE_SIDE = 2000;
	constexpr int MAX_IMAGE_BYTES = 3 * 1024 * 1024;	///< base64 makes 4 MB of it, under the 5 MB limit
}

AssistantDock::AssistantDock(QETDiagramEditor *editor) :
	QDockWidget(tr("Claude assistant"), editor),
	m_editor(editor),
	m_bridge(new AssistantBridge(this))
{
	// not "assistant_dock": that is the live mode's log dock, and two docks
	// with one name mix up their places when the window state is restored
	setObjectName(QStringLiteral("claude_assistant_dock"));
	setWindowIcon(icon());
	m_stack = new QStackedWidget(this);
	m_setup = new AssistantSetupPage(m_stack);
	m_stack->addWidget(m_setup);
	auto *body = new QWidget(m_stack);
	m_chat = body;
	m_stack->addWidget(body);
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
	m_settings->setText(tr("Settings"));
	m_settings->setToolTip(tr("Back to the setup: API key, Python, package"));
	top->addWidget(m_activity, 1);
	top->addWidget(m_new);
	top->addWidget(m_settings);
	layout->addLayout(top);

	m_history = new QTextBrowser(body);
	m_history->setOpenExternalLinks(true);
	m_history->setPlaceholderText(tr("Ask about the open drawing, or tell Claude what to "
					 "draw. Every change Claude makes is one undo step."));
	layout->addWidget(m_history, 1);

	m_attachment_bar = new QWidget(body);
	auto *attachment_layout = new QHBoxLayout(m_attachment_bar);
	attachment_layout->setContentsMargins(0, 0, 0, 0);
	m_attachment_bar->hide();
	layout->addWidget(m_attachment_bar);

	auto *input = new AttachInput(body);
	input->take = [this](const QMimeData *data) { return takeMimeData(data); };
	m_input = input;
	m_input->setPlaceholderText(tr("Your question… (Enter sends, Shift+Enter: new line; "
				       "paste or drag in pictures and PDFs)"));
	m_input->setMaximumHeight(90);
	m_input->installEventFilter(this);
	layout->addWidget(m_input);

	auto *buttons = new QHBoxLayout();
	m_attach = new QToolButton(body);
	m_attach->setText(QStringLiteral("📎"));
	m_attach->setToolTip(tr("Attach pictures or PDFs (also: Ctrl+V or drag them into the field)"));
	buttons->addWidget(m_attach);
	m_cancel = new QPushButton(tr("Stop"), body);
	m_cancel->setToolTip(tr("Stop the answer"));
	m_cancel->setEnabled(false);
	m_send = new QPushButton(tr("Send"), body);
	m_send->setDefault(true);
	buttons->addStretch(1);
	buttons->addWidget(m_cancel);
	buttons->addWidget(m_send);
	layout->addLayout(buttons);
	setWidget(m_stack);

	m_render_timer = new QTimer(this);
	m_render_timer->setSingleShot(true);
	m_render_timer->setInterval(80);
	connect(m_render_timer, &QTimer::timeout, this, &AssistantDock::render);

	connect(m_send, &QPushButton::clicked, this, &AssistantDock::sendQuestion);
	connect(m_attach, &QToolButton::clicked, this, &AssistantDock::chooseFiles);
	connect(m_cancel, &QPushButton::clicked, this, &AssistantDock::cancel);
	connect(m_new, &QToolButton::clicked, this, &AssistantDock::newConversation);
	connect(m_settings, &QToolButton::clicked, this, [this]() { showSetup(false); });
	connect(m_setup, &AssistantSetupPage::completed, this, [this]() {
		QSettings().setValue(QStringLiteral("assistant/setup_done"), true);
		showChat();
	});
	connect(m_setup, &AssistantSetupPage::chatRequested, this, &AssistantDock::showChat);
	connect(m_setup, &AssistantSetupPage::settingsChanged, this, &AssistantDock::settingsChanged);
	connect(this, &QDockWidget::visibilityChanged, this, [this](bool visible) {
		if (visible && !m_shown_once) firstShown();
	});
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
	QStringList args;
	if (AssistantSetupPage::kontoMode()) {
			//The Claude account: no key, and none from the environment
			//either, or Claude Code would bill the API instead
		env.remove(QStringLiteral("ANTHROPIC_API_KEY"));
		env.remove(QStringLiteral("ANTHROPIC_AUTH_TOKEN"));
		args << QStringLiteral("--konto");
	} else {
		const QString key = AssistantSetupPage::apiKey();
		if (key.isEmpty() && env.value(QStringLiteral("ANTHROPIC_API_KEY")).isEmpty()) {
				//The question stays in the field for after the setup
			showSetup();
			return false;
		}
		if (!key.isEmpty()) env.insert(QStringLiteral("ANTHROPIC_API_KEY"), key);
	}

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
	if (!m_bridge->start(env, &error, args)) {
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
	if ((text.isEmpty() && m_attachments.isEmpty()) || m_busy) return;
	if (!ensureBridge()) return;
	m_input->clear();
	QJsonArray attachments;
	QStringList names;
	for (const Attachment &a : std::as_const(m_attachments)) {
		attachments.append(QJsonObject{{QStringLiteral("name"), a.name},
					       {QStringLiteral("mime"), a.mime},
					       {QStringLiteral("data"), QString::fromLatin1(a.data.toBase64())}});
		names << a.name;
	}
	m_attachments.clear();
	updateAttachments();
	QString shown = text;
	if (!names.isEmpty()) {
		if (!shown.isEmpty()) shown += QStringLiteral("  \n");
		shown += QStringLiteral("📎 ") + names.join(QStringLiteral(", "));
	}
	add(Entry::User, shown);
	m_answer_open = false;
	setBusy(true);
	m_activity->setText(tr("thinking…"));
	QJsonObject ask{{QStringLiteral("type"), QStringLiteral("ask")},
			{QStringLiteral("text"), text},
			{QStringLiteral("kontext"), context()}};
	if (!attachments.isEmpty()) ask.insert(QStringLiteral("anhaenge"), attachments);
	m_bridge->send(ask);
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

/**
	@brief AssistantDock::firstShown
	The first time the dock is shown: straight to the chat when the setup
	was done before and its quick checks still hold, else to the setup.
	Whether the package is still there shows when the bridge starts.
*/
void AssistantDock::firstShown()
{
	m_shown_once = true;
	if (QSettings().value(QStringLiteral("assistant/setup_done")).toBool()
	    && AssistantSetupPage::quickCheck())
		showChat();
	else
		showSetup();
}

void AssistantDock::showSetup(bool switch_when_done)
{
	m_shown_once = true;
	m_stack->setCurrentWidget(m_setup);
	m_setup->refresh(switch_when_done);
}

void AssistantDock::showChat()
{
	m_shown_once = true;
	m_stack->setCurrentWidget(m_chat);
	m_input->setFocus();
}

/**
	@brief AssistantDock::settingsChanged
	A new key or Python takes effect with a new bridge
*/
void AssistantDock::settingsChanged()
{
	if (!m_bridge->isRunning() || m_busy) return;
	m_bridge->stop();
	LiveServer::instance().setAssistantWritesAllowed(false);
	add(Entry::Note, tr("Settings changed; the next question starts a new conversation."));
}

/**
	@brief AssistantDock::icon
	A speech bubble with a spark, drawn so that it needs no image file
*/
QIcon AssistantDock::icon()
{
	QIcon icon;
	for (int size : {16, 22, 24, 32, 48}) {
		QPixmap pixmap(size, size);
		pixmap.fill(Qt::transparent);
		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::Antialiasing);
		const qreal s = size;
		QPainterPath bubble;
		bubble.addRoundedRect(QRectF(s * 0.04, s * 0.06, s * 0.92, s * 0.70), s * 0.2, s * 0.2);
		QPainterPath tail;
		tail.moveTo(s * 0.22, s * 0.70);
		tail.lineTo(s * 0.18, s * 0.96);
		tail.lineTo(s * 0.46, s * 0.70);
		tail.closeSubpath();
		painter.fillPath(bubble.united(tail), QColor(0xD9, 0x77, 0x57));
			//The spark: four rays and four shorter ones
		painter.setPen(QPen(Qt::white, qMax(1.0, s / 11.0), Qt::SolidLine, Qt::RoundCap));
		const QPointF c(s * 0.5, s * 0.41);
		for (int i = 0 ; i < 8 ; ++i) {
			const qreal length = (i % 2 ? 0.16 : 0.25) * s;
			const qreal angle = i * M_PI / 4.0;
			painter.drawLine(c, c + QPointF(std::cos(angle) * length, std::sin(angle) * length));
		}
		painter.end();
		icon.addPixmap(pixmap);
	}
	return icon;
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
	m_attach->setEnabled(!busy);
	m_settings->setEnabled(!busy);
}

AssistantDock::Attachment AssistantDock::imageAttachment(const QImage &image, const QString &name)
{
	QImage scaled = image;
	if (qMax(scaled.width(), scaled.height()) > MAX_IMAGE_SIDE)
		scaled = scaled.scaled(MAX_IMAGE_SIDE, MAX_IMAGE_SIDE, Qt::KeepAspectRatio,
				       Qt::SmoothTransformation);
	Attachment a{name, QStringLiteral("image/png"), QByteArray()};
	{
		QBuffer buffer(&a.data);
		buffer.open(QIODevice::WriteOnly);
		scaled.save(&buffer, "PNG");
	}
		//A photo is far smaller as JPEG; quality down until it fits
	for (int quality = 90; a.data.size() > MAX_IMAGE_BYTES && quality >= 40; quality -= 15) {
		QByteArray jpeg;
		QBuffer buffer(&jpeg);
		buffer.open(QIODevice::WriteOnly);
		scaled.convertToFormat(QImage::Format_RGB888).save(&buffer, "JPEG", quality);
		a.data = jpeg;
		a.mime = QStringLiteral("image/jpeg");
		if (!a.name.endsWith(QLatin1String(".jpg"), Qt::CaseInsensitive))
			a.name = QFileInfo(name).completeBaseName() + QStringLiteral(".jpg");
	}
	return a;
}

bool AssistantDock::fileAttachment(const QString &path, Attachment *out, QString *error)
{
	const QFileInfo info(path);
	const QString mime = QMimeDatabase().mimeTypeForFile(info).name();
	if (mime == QLatin1String("application/pdf")) {
		if (info.size() > MAX_PDF_BYTES) {
			if (error) *error = tr("%1 is larger than 20 MB.").arg(info.fileName());
			return false;
		}
		QFile file(path);
		if (!file.open(QIODevice::ReadOnly)) {
			if (error) *error = tr("%1 cannot be read.").arg(info.fileName());
			return false;
		}
		*out = Attachment{info.fileName(), mime, file.readAll()};
		return true;
	}
	QImageReader reader(path);
	reader.setAutoTransform(true);	// photos taken upright stay upright
	const QImage image = reader.read();
	if (image.isNull()) {
		if (error) *error = tr("%1 is neither a picture nor a PDF.").arg(info.fileName());
		return false;
	}
	*out = imageAttachment(image, info.fileName());
	return true;
}

/**
	@brief AssistantDock::takeMimeData
	What is pasted or dragged into the input field: a picture (a screenshot
	from the clipboard) or files become attachments. False when there is
	none of these, so the field takes it as text.
*/
bool AssistantDock::takeMimeData(const QMimeData *data)
{
	if (m_busy) return false;
	QStringList files;
	for (const QUrl &url : data->urls())
		if (url.isLocalFile()) files << url.toLocalFile();
	if (!files.isEmpty()) {
		for (const QString &f : std::as_const(files)) {
			Attachment a;
			QString error;
			if (fileAttachment(f, &a, &error)) addAttachment(a);
			else add(Entry::Error, error);
		}
		return true;
	}
	if (data->hasImage()) {
		const QImage image = qvariant_cast<QImage>(data->imageData());
		if (image.isNull()) return false;
		addAttachment(imageAttachment(image, tr("Picture %1.png").arg(m_attachments.size() + 1)));
		return true;
	}
	return false;
}

void AssistantDock::addAttachment(const Attachment &attachment)
{
	if (m_attachments.size() >= MAX_ATTACHMENTS) {
		add(Entry::Error, tr("At most %1 attachments per question.").arg(MAX_ATTACHMENTS));
		return;
	}
	m_attachments.append(attachment);
	updateAttachments();
}

void AssistantDock::chooseFiles()
{
	const QStringList files = QFileDialog::getOpenFileNames(this, tr("Attach pictures or PDFs"),
		QSettings().value(QStringLiteral("assistant/attach_dir")).toString(),
		tr("Pictures and PDFs (*.png *.jpg *.jpeg *.gif *.webp *.bmp *.pdf);;All files (*)"));
	if (files.isEmpty()) return;
	QSettings().setValue(QStringLiteral("assistant/attach_dir"), QFileInfo(files.first()).absolutePath());
	for (const QString &f : files) {
		Attachment a;
		QString error;
		if (fileAttachment(f, &a, &error)) addAttachment(a);
		else add(Entry::Error, error);
	}
	m_input->setFocus();
}

/**
	@brief AssistantDock::updateAttachments
	One small button per attachment above the input field: a thumbnail
	(or a file icon for a PDF) and the name; a click takes it off again.
*/
void AssistantDock::updateAttachments()
{
	auto *bar_layout = static_cast<QHBoxLayout *>(m_attachment_bar->layout());
	while (QLayoutItem *item = bar_layout->takeAt(0)) {
		delete item->widget();
		delete item;
	}
	for (int i = 0 ; i < m_attachments.size() ; ++i) {
		const Attachment &a = m_attachments.at(i);
		auto *chip = new QToolButton(m_attachment_bar);
		chip->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		chip->setAutoRaise(true);
		QString shown = a.name;
		if (shown.size() > 22) shown = shown.left(20) + QStringLiteral("…");
		chip->setText(shown + QStringLiteral("  ✕"));
		chip->setToolTip(tr("%1 – click to remove").arg(a.name));
		if (a.mime.startsWith(QLatin1String("image/"))) {
			QImage thumb;
			thumb.loadFromData(a.data);
			chip->setIcon(QPixmap::fromImage(thumb.scaled(32, 32, Qt::KeepAspectRatio,
								      Qt::SmoothTransformation)));
		} else {
			chip->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
		}
		chip->setIconSize(QSize(24, 24));
		connect(chip, &QToolButton::clicked, this, [this, i]() {
			if (i < m_attachments.size()) m_attachments.removeAt(i);
			updateAttachments();
		});
		bar_layout->addWidget(chip);
	}
	bar_layout->addStretch(1);
	m_attachment_bar->setVisible(!m_attachments.isEmpty());
}
