// SPDX-License-Identifier: GPL-2.0-or-later
// The Claude button of the main toolbar and the setup page of the assistant
// dock, offscreen and with settings of its own: the button opens the dock,
// without a key the setup is shown with the key step red, with a key the
// page turns into the chat by itself, "Settings" leads back to the setup.
// Reads the credential store, never writes it. No real key is used.
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QToolBar>
#include <QToolButton>
#include <cstdio>
#include <functional>
#include <stdexcept>
#include "../../sources/qetapp.h"
#include "../../sources/qetdiagrameditor.h"
#include "../../sources/qetproject.h"
#include "../../sources/qetmessagebox.h"
#include "../../sources/assistant/assistantdock.h"
#include "../../sources/assistant/assistantsetuppage.h"

static void check(bool value, const char *why) { if (!value) throw std::runtime_error(why); }

static void waitFor(const std::function<bool()> &done, int ms)
{
	QElapsedTimer timer; timer.start();
	while (!done() && timer.elapsed() < ms) QApplication::processEvents(QEventLoop::AllEvents, 50);
}

	/// The visible texts of the page, to see in the report what it says
static QString texts(QWidget *widget)
{
	QStringList all;
	for (QLabel *label : widget->findChildren<QLabel *>())
		if (label->isVisibleTo(widget) && !label->text().isEmpty()) all << label->text();
	return all.join(QStringLiteral(" | "));
}

static bool checking(QWidget *page) { return texts(page).contains(QStringLiteral("color:gray; font-size:16pt;\">…")); }

int main(int argc, char **argv)
{
	const QString report = QString::fromLocal8Bit(argv[1]);
	const QString output = QString::fromLocal8Bit(argv[2]);
	const QString bridge = QString::fromLocal8Bit(argv[3]);
	const QString python = QString::fromLocal8Bit(argv[4]);
	const QString python_without_package = argc > 5 ? QString::fromLocal8Bit(argv[5]) : QString();
	QTemporaryDir isolated;
	QByteArray config = "--config-dir=" + isolated.path().toUtf8();
	QByteArray data = "--data-dir=" + isolated.path().toUtf8();
	QByteArray collection = "--common-elements-dir=" + isolated.path().toUtf8();
	char *args[] = {argv[0], config.data(), data.data(), collection.data()}; int count = 4;
	qunsetenv("ANTHROPIC_API_KEY");
	qputenv("QET_ASSISTENT_BRUECKE", bridge.toLocal8Bit());
	QApplication app(count, args);
	std::freopen(report.toLocal8Bit().constData(), "w", stdout);
	std::setbuf(stdout, nullptr);
	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, isolated.path());
	QSettings().setValue(QStringLiteral("assistant/python"), python);
	QETProject::setBackupEnabled(false); QET::QetMessageBox::setNonInteractive(true);
	try {
		auto *qet = new QETApp; Q_UNUSED(qet);
		auto editors = QETApp::diagramEditors(); check(!editors.isEmpty(), "editor created");
		QETDiagramEditor *editor = editors.first();
		editor->resize(1400, 900);
		editor->show();
		QApplication::processEvents();

			//The button: in the main toolbar, icon and text
		auto *main_bar = editor->findChild<QToolBar *>(QStringLiteral("toolbar"));
		check(main_bar, "main toolbar");
		QToolButton *claude = nullptr;
		for (QToolButton *button : editor->findChildren<QToolButton *>())
			if (button->text() == QStringLiteral("Claude") && button->parentWidget() == main_bar
			    && button->isVisibleTo(main_bar)) claude = button;
		if (!claude) {
			for (QAction *action : main_bar->actions())
				std::printf("toolbar action: %s\n", qUtf8Printable(action->text()));
			for (QToolButton *button : editor->findChildren<QToolButton *>())
				if (button->text().contains(QStringLiteral("Claude")))
					std::printf("button %s parent %s\n", qUtf8Printable(button->text()),
						    button->parentWidget() ? qUtf8Printable(button->parentWidget()->objectName()) : "-");
		}
		check(claude, "Claude button in the main toolbar");
		check(claude->toolButtonStyle() == Qt::ToolButtonTextBesideIcon && !claude->icon().isNull(),
		      "Claude button has icon and text");
		std::printf("PASS: Claude button in the main toolbar, tooltip: %s\n", qUtf8Printable(claude->toolTip()));
		editor->grab().save(output + "/0_fenster.png");

		auto *dock = editor->findChild<AssistantDock *>();
		check(dock && !dock->isVisible(), "dock hidden at start");
		claude->click();
		waitFor([dock]() { return dock->isVisible(); }, 3000);
		check(dock->isVisible(), "button opens the dock");
		check(editor->dockWidgetArea(dock) == Qt::RightDockWidgetArea, "dock on the right");
		check(claude->isChecked(), "button checked while open");
		auto *stack = dock->findChild<QStackedWidget *>();
		auto *page = dock->findChild<AssistantSetupPage *>();
		check(stack && page, "stack and setup page");
		std::printf("PASS: button opens the dock on the right: %s\n", qUtf8Printable(dock->windowTitle()));

			//No key: setup page, key step red, not complete
		if (AssistantSetupPage::hasApiKey()) {
			std::puts("SKIP: a key is stored in the credential store, the missing-key state is not tested");
		} else {
			check(stack->currentWidget() == page, "without a key the setup is shown");
			waitFor([page]() { return !checking(page); }, 30000);
			check(!page->isComplete(), "not complete without a key");
			std::printf("PASS: without a key the setup page shows: %s\n", qUtf8Printable(texts(page)));
			dock->grab().save(output + "/1_einrichtung_ohne_schluessel.png");

				//A wrong key: Check asks Anthropic, which refuses it; nothing stored
			auto *key = page->findChild<QLineEdit *>();
			check(key, "key field");
			key->setText(QStringLiteral("sk-ant-attrappe-0000"));
			QPushButton *test = nullptr;
			for (QPushButton *button : page->findChildren<QPushButton *>())
				if (button->text() == QStringLiteral("Prüfen") || button->text() == QStringLiteral("Check"))
					test = button;
			check(test, "Check button");
			test->click();
			waitFor([test]() { return test->isEnabled(); }, 30000);
			QApplication::processEvents();
			check(!AssistantSetupPage::hasApiKey(), "a refused key is not stored");
			check(!page->isComplete(), "still not complete");
			std::printf("PASS: wrong key refused and not stored: %s\n", qUtf8Printable(texts(page)));
			dock->grab().save(output + "/2_falscher_schluessel.png");
		}

			//Python without the package: red, install button shown
		if (!python_without_package.isEmpty()) {
			QSettings().setValue(QStringLiteral("assistant/python"), python_without_package);
			page->refresh();
			waitFor([page]() { return !checking(page); }, 30000);
			QPushButton *install = nullptr;
			for (QPushButton *button : page->findChildren<QPushButton *>())
				if (button->toolTip().contains(QStringLiteral("pip install"))) install = button;
			check(install && install->isVisibleTo(page), "install button for the missing package");
			std::printf("PASS: missing package shows the install button: %s\n", qUtf8Printable(texts(page)));
			dock->grab().save(output + "/3_paket_fehlt.png");
			QSettings().setValue(QStringLiteral("assistant/python"), python);
		}

			//A dummy key from the environment: every step green, the chat by itself
		qputenv("ANTHROPIC_API_KEY", "sk-ant-attrappe-umgebung");
		int completed = 0;
		QObject::connect(page, &AssistantSetupPage::completed, page, [&completed]() { ++completed; });
		page->refresh();
		waitFor([stack, page]() { return stack->currentWidget() != page; }, 30000);
		check(page->isComplete(), "complete with key, Python and package");
		check(completed == 1, "completed once");
		check(stack->currentWidget() != page, "switched to the chat by itself");
		check(QSettings().value(QStringLiteral("assistant/setup_done")).toBool(), "setup remembered");
		std::printf("PASS: with a key the dock switched to the chat: %s\n", qUtf8Printable(dock->windowTitle()));
		dock->grab().save(output + "/4_chat.png");

			//Settings in the chat: back to the setup, all green, To the chat
		QToolButton *settings = nullptr;
		for (QToolButton *button : stack->currentWidget()->findChildren<QToolButton *>())
			if (button->text() == QStringLiteral("Einstellungen") || button->text() == QStringLiteral("Settings"))
				settings = button;
		check(settings, "Settings button in the chat");
		settings->click();
		check(stack->currentWidget() == page, "Settings shows the setup");
		waitFor([page]() { return page->isComplete(); }, 30000);
		QApplication::processEvents();
		check(stack->currentWidget() == page, "the setup stays open when it was asked for");
		std::printf("PASS: Settings leads back to the setup: %s\n", qUtf8Printable(texts(page)));
		dock->grab().save(output + "/5_einstellungen.png");
		editor->grab().save(output + "/6_fenster_mit_dock.png");

			//The button closes it again
		claude->click();
		waitFor([dock]() { return !dock->isVisible(); }, 3000);
		check(!dock->isVisible() && !claude->isChecked(), "button closes the dock");
		std::puts("PASS: button closes the dock");
	} catch (const std::exception &error) {
		std::printf("FAIL: %s\n", error.what()); std::fflush(stdout); std::_Exit(1);
	}
	std::fflush(stdout); std::_Exit(0);
}
