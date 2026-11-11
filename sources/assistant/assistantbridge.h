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
#ifndef ASSISTANTBRIDGE_H
#define ASSISTANTBRIDGE_H

#include <QJsonObject>
#include <QObject>
#include <QProcessEnvironment>
#include <QStringList>

class QProcess;

/**
	@brief The AssistantBridge class
	The Python bridge behind the built-in assistant
	(misc/qet-assistent/bruecke.py), run as a child process. One JSON
	object per line each way over stdin/stdout; see the script's
	docstring for the messages.

	Each line read is handed on through a queued call, never from inside
	readyRead: the dock opens a modal question for some of them.
*/
class AssistantBridge : public QObject
{
	Q_OBJECT

	public:
		explicit AssistantBridge(QObject *parent = nullptr);
		~AssistantBridge() override;

		bool isRunning() const;
			/// Start the bridge with \a env (which holds the API key: it is
			/// handed to the child only, never logged).
		bool start(const QProcessEnvironment &env, QString *error,
			   const QStringList &extra_args = QStringList());
		void stop();
		void send(const QJsonObject &message);
			/// The last lines the bridge wrote to stderr, for an error report
		QStringList recentErrors() const { return m_errors; }

			/// The Python to run the bridge with: the setting, else the one
			/// AiAssistantSetup finds. \a store_shortcut is set when that
			/// is only Windows' Microsoft Store shortcut.
		static QString pythonProgram(bool *store_shortcut = nullptr);
			/// What goes before the script: "-3" for Windows' py launcher
		static QStringList pythonArguments(const QString &python);
			/// bruecke.py: the setting, else beside this QElectroTech
		static QString bridgeScript();

	signals:
		void message(const QJsonObject &message);
		void stopped(const QString &why);

	private:
		void readOutput();
		void readErrors();

		QProcess *m_process = nullptr;
		QByteArray m_buffer;
		QStringList m_errors;
		bool m_stopping = false;
};

#endif // ASSISTANTBRIDGE_H
