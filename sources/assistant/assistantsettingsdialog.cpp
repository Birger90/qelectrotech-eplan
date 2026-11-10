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
#include "assistantsettingsdialog.h"

#include "assistantbridge.h"
#include "../import/portals/portalcredentials.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

AssistantSettingsDialog::AssistantSettingsDialog(QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Assistant settings"));
	auto *layout = new QVBoxLayout(this);

	auto *intro = new QLabel(tr("The assistant uses Claude (Anthropic). It needs an API key "
				    "from console.anthropic.com. The key is kept in the "
				    "Windows Credential Manager, never in a file of QElectroTech."), this);
	intro->setWordWrap(true);
	layout->addWidget(intro);

	auto *form = new QFormLayout();
	m_key = new QLineEdit(this);
	m_key->setEchoMode(QLineEdit::Password);
	m_key->setClearButtonEnabled(true);
	form->addRow(tr("API key:"), m_key);
	m_state = new QLabel(this);
	m_state->setWordWrap(true);
	form->addRow(QString(), m_state);

	auto *python_row = new QHBoxLayout();
	m_python = new QLineEdit(QSettings().value(QStringLiteral("assistant/python")).toString(), this);
	const QString found = AssistantBridge::pythonProgram();
	m_python->setPlaceholderText(found.isEmpty() ? tr("not found") : found);
	auto *browse = new QPushButton(tr("Choose…"), this);
	python_row->addWidget(m_python, 1);
	python_row->addWidget(browse);
	form->addRow(tr("Python:"), python_row);
	connect(browse, &QPushButton::clicked, this, [this]() {
		const QString file = QFileDialog::getOpenFileName(this, tr("Python program"));
		if (!file.isEmpty()) m_python->setText(file);
	});
	layout->addLayout(form);

	auto *hint = new QLabel(tr("The bridge needs the Python package “anthropic” "
				   "(py -m pip install anthropic)."), this);
	hint->setWordWrap(true);
	layout->addWidget(hint);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
	QPushButton *remove = buttons->addButton(tr("Remove key"), QDialogButtonBox::DestructiveRole);
	connect(buttons, &QDialogButtonBox::accepted, this, &AssistantSettingsDialog::save);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(remove, &QPushButton::clicked, this, &AssistantSettingsDialog::removeKey);
	layout->addWidget(buttons);

	if (!PortalCredentials::available()) {
		m_key->setEnabled(false);
		remove->setEnabled(false);
	}
	updateState();
	resize(520, sizeHint().height());
}

QString AssistantSettingsDialog::credentialId()
{
	return QStringLiteral("claude");
}

QString AssistantSettingsDialog::apiKey()
{
	return PortalCredentials::load(credentialId()).secret;
}

bool AssistantSettingsDialog::hasApiKey()
{
	return PortalCredentials::has(credentialId());
}

void AssistantSettingsDialog::updateState()
{
	if (!PortalCredentials::available())
		m_state->setText(tr("No credential store on this system: set ANTHROPIC_API_KEY "
				    "in the environment instead."));
	else if (hasApiKey())
		m_state->setText(tr("A key is stored. Leave the field empty to keep it."));
	else
		m_state->setText(tr("No key stored yet."));
}

void AssistantSettingsDialog::save()
{
	const QString key = m_key->text().trimmed();
	if (!key.isEmpty()) {
		QString error;
		PortalAccount account;
		account.user = QStringLiteral("api-key");
		account.secret = key;
		if (!PortalCredentials::store(credentialId(), account, &error)) {
			QMessageBox::warning(this, windowTitle(),
					     tr("The key could not be stored: %1").arg(error));
			return;
		}
		m_key->clear();
	}
	QSettings settings;
	const QString python = m_python->text().trimmed();
	if (python.isEmpty()) settings.remove(QStringLiteral("assistant/python"));
	else settings.setValue(QStringLiteral("assistant/python"), python);
	accept();
}

void AssistantSettingsDialog::removeKey()
{
	if (!hasApiKey()) return;
	if (QMessageBox::question(this, windowTitle(), tr("Remove the stored API key?"))
	    != QMessageBox::Yes)
		return;
	QString error;
	if (!PortalCredentials::remove(credentialId(), &error))
		QMessageBox::warning(this, windowTitle(), tr("The key could not be removed: %1").arg(error));
	updateState();
}
