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
#ifndef ASSISTANTDOCK_H
#define ASSISTANTDOCK_H

#include <QDockWidget>
#include <QJsonObject>
#include <QList>

class AssistantBridge;
class QETDiagramEditor;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTextBrowser;
class QTimer;
class QToolButton;

/**
	@brief The AssistantDock class
	The built-in assistant: a conversation with Claude about the open
	project. Claude works through the live channel (LiveServer, its own
	token), so every change it makes is one undo step; before the first
	change of a session the user is asked once.

	Behind it runs AssistantBridge (Python, Anthropic SDK). The dock shows
	the conversation, which tool Claude is using, and has a button to
	stop the answer.
*/
class AssistantDock : public QDockWidget
{
	Q_OBJECT

	public:
		explicit AssistantDock(QETDiagramEditor *editor);
		~AssistantDock() override;

	protected:
		bool eventFilter(QObject *watched, QEvent *event) override;

	private:
		struct Entry {
			enum Kind { User, Assistant, Note, Error } kind;
			QString text;
		};

		void sendQuestion();
		void cancel();
		void newConversation();
		void openSettings();
		bool ensureBridge();
		void onMessage(const QJsonObject &message);
		void onStopped(const QString &why);
		void confirm(const QJsonObject &message);
		QJsonObject context() const;
		void add(Entry::Kind kind, const QString &text);
		void appendToAnswer(const QString &text);
		void render();
		void setBusy(bool busy);

		QETDiagramEditor *m_editor;
		AssistantBridge *m_bridge;
		QTextBrowser *m_history;
		QPlainTextEdit *m_input;
		QPushButton *m_send;
		QPushButton *m_cancel;
		QToolButton *m_new;
		QToolButton *m_settings;
		QLabel *m_activity;
		QTimer *m_render_timer;
		QList<Entry> m_entries;
		bool m_busy = false;
		bool m_answer_open = false;	///< the last entry is the answer being streamed
};

#endif // ASSISTANTDOCK_H
