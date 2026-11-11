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
#ifndef ASSISTANTSETUPPAGE_H
#define ASSISTANTSETUPPAGE_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QNetworkAccessManager;
class QNetworkReply;
class QPlainTextEdit;
class QProcess;
class QPushButton;
class QStackedWidget;

/**
	@brief The AssistantSetupPage class
	The guided setup of the built-in assistant, shown in its dock until
	everything it needs is there:
	- the Anthropic API key, tested with one short request and kept in
	  the operating system's credential store (PortalCredentials, account
	  "claude"; never in the settings or a file), or ANTHROPIC_API_KEY;
	- a Python to run the bridge with;
	- the Python package "anthropic", which a button installs with pip;
	- the bridge script beside QElectroTech.
	Each step shows a green check or a red cross. When all are green the
	page says so with completed().
*/
class AssistantSetupPage : public QWidget
{
	Q_OBJECT

	public:
		explicit AssistantSetupPage(QWidget *parent = nullptr);
		~AssistantSetupPage() override;

			/// Check every step again (the package in the background).
			/// completed() follows when all turn green and @a switch_when_done
			/// is set, or a step was red on the way.
		void refresh(bool switch_when_done = false);
			/// Every step green
		bool isComplete() const;

			/// The stored key, empty when there is none
		static QString apiKey();
		static bool hasApiKey();
			/// The key from the store, else from ANTHROPIC_API_KEY
		static bool keyAvailable();
			/// The steps that need no program run: key, Python, bridge
		static bool quickCheck();
			/// Claude through the user's own Claude account instead of an
			/// API key (only in builds with QET_CLAUDE_KONTO; the default there)
		static bool kontoMode();
		static void setKontoMode(bool konto);

	signals:
			/// All steps turned green (see refresh())
		void completed();
			/// The key or the Python changed: a running bridge is stale
		void settingsChanged();
			/// "Back to the chat"
		void chatRequested();

	private:
		enum class State { Unknown, Checking, Ok, Failed };

		void checkKey();
		void testKey();
		void keyTested(QNetworkReply *reply, const QString &key);
		void removeKey();
		void checkPython();
		void choosePython();
		void checkPackage();
		void installPackage();
		void checkBridge();
		void setState(QLabel *label, State state);
		void updateCompletion();
		void showMode();
		QWidget *buildKontoPage(QWidget *parent);
		void checkKonto();
		void connectKonto();
		void kontoNext();
		void runKonto(const QStringList &args, const QString &what);
		void stopConnecting(const QString &message);

		QLabel *m_key_state, *m_key_text;
		QLineEdit *m_key;
		QPushButton *m_test, *m_remove_key;
		QLabel *m_python_state, *m_python_text;
		QLabel *m_package_state, *m_package_text;
		QPushButton *m_install;
		QPlainTextEdit *m_output;
		QLabel *m_bridge_state, *m_bridge_text;
		QLabel *m_summary;
		QPushButton *m_to_chat;
		QLabel *m_title, *m_intro;
		QStackedWidget *m_modes;
		QPushButton *m_mode_switch;
			//Claude account
		QPushButton *m_connect;
		QLabel *m_k_python_state, *m_k_python_text, *m_k_sdk_state, *m_k_sdk_text,
		       *m_k_login_state, *m_k_login_text, *m_k_link;
		QPlainTextEdit *m_k_output;

		QNetworkAccessManager *m_network = nullptr;
		QProcess *m_process = nullptr;
		State m_key_ok = State::Unknown, m_python_ok = State::Unknown,
		      m_package_ok = State::Unknown, m_bridge_ok = State::Unknown;
		bool m_switch_when_done = false;
		State m_k_python = State::Unknown, m_k_sdk = State::Unknown, m_k_login = State::Unknown;
		bool m_connecting = false, m_tried_install = false, m_tried_login = false;
};

#endif // ASSISTANTSETUPPAGE_H
