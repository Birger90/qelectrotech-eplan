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
#ifndef ASSISTANTSETTINGSDIALOG_H
#define ASSISTANTSETTINGSDIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;

/**
	@brief The AssistantSettingsDialog class
	Settings of the built-in assistant: the Anthropic API key, kept in
	the operating system's credential store (PortalCredentials, account
	"claude"; never in the settings or a file), and the Python that runs
	the bridge.
*/
class AssistantSettingsDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit AssistantSettingsDialog(QWidget *parent = nullptr);

			/// The stored key, empty when there is none
		static QString apiKey();
		static bool hasApiKey();
			/// The credential store's name for the account
		static QString credentialId();

	private:
		void save();
		void removeKey();
		void updateState();

		QLineEdit *m_key = nullptr;
		QLineEdit *m_python = nullptr;
		QLabel *m_state = nullptr;
};

#endif // ASSISTANTSETTINGSDIALOG_H
