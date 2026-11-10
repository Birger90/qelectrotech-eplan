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
#include "assistantbridge.h"

#include "../ui/aiassistantsetup.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMetaObject>
#include <QProcess>
#include <QSettings>

AssistantBridge::AssistantBridge(QObject *parent) :
	QObject(parent)
{
}

AssistantBridge::~AssistantBridge()
{
	stop();
}

bool AssistantBridge::isRunning() const
{
	return m_process && m_process->state() != QProcess::NotRunning;
}

QString AssistantBridge::pythonProgram(bool *store_shortcut)
{
	if (store_shortcut) *store_shortcut = false;
	const QString configured = QSettings().value(QStringLiteral("assistant/python")).toString().trimmed();
	if (!configured.isEmpty()) return configured;
#ifdef Q_OS_WIN
	const bool windows = true;
#else
	const bool windows = false;
#endif
	const AiAssistantSetup::Paths paths = AiAssistantSetup::detect(
		QCoreApplication::applicationDirPath(),
		QCoreApplication::applicationFilePath(), windows);
	if (store_shortcut)
		*store_shortcut = paths.python_status == AiAssistantSetup::PythonStatus::StoreShortcut;
	if (paths.python_status == AiAssistantSetup::PythonStatus::Missing) return QString();
	return paths.python;
}

QString AssistantBridge::bridgeScript()
{
	const QString env = qEnvironmentVariable("QET_ASSISTENT_BRUECKE").trimmed();
	if (!env.isEmpty() && QFileInfo(env).isFile()) return QDir::cleanPath(env);
	const QString configured = QSettings().value(QStringLiteral("assistant/bridge")).toString().trimmed();
	if (!configured.isEmpty() && QFileInfo(configured).isFile()) return QDir::cleanPath(configured);
		//The Windows folder (exe beside mcp/), the installers (bin/ beside
		//mcp/) and make install (share/qelectrotech/mcp/)
	const QDir bin(QCoreApplication::applicationDirPath());
	for (const QString &candidate : {
		     QStringLiteral("mcp/qet-assistent/bruecke.py"),
		     QStringLiteral("../mcp/qet-assistent/bruecke.py"),
		     QStringLiteral("../share/qelectrotech/mcp/qet-assistent/bruecke.py")}) {
		const QString path = bin.filePath(candidate);
		if (QFileInfo(path).isFile()) return QDir::cleanPath(path);
	}
	return QString();
}

bool AssistantBridge::start(const QProcessEnvironment &env, QString *error)
{
	if (isRunning()) return true;
	bool store_shortcut = false;
	const QString python = pythonProgram(&store_shortcut);
	if (python.isEmpty()) {
		if (error) *error = tr("No Python found. Install Python 3 (python.org) "
				       "or set the program in the assistant's settings.");
		return false;
	}
	const QString script = bridgeScript();
	if (script.isEmpty()) {
		if (error) *error = tr("The assistant's bridge (mcp/qet-assistent/bruecke.py) "
				       "was not found beside QElectroTech.");
		return false;
	}

	delete m_process;
	m_process = new QProcess(this);
	m_process->setProcessEnvironment(env);
	m_process->setWorkingDirectory(QFileInfo(script).absolutePath());
	m_buffer.clear();
	m_errors.clear();
	m_stopping = false;
	connect(m_process, &QProcess::readyReadStandardOutput, this, &AssistantBridge::readOutput);
	connect(m_process, &QProcess::readyReadStandardError, this, &AssistantBridge::readErrors);
	connect(m_process, &QProcess::finished, this, [this](int code, QProcess::ExitStatus status) {
		if (m_stopping) return;
		const QString why = status == QProcess::CrashExit
				? tr("The assistant's bridge stopped unexpectedly.")
				: tr("The assistant's bridge ended (code %1).").arg(code);
		emit stopped(why);
	});

	QStringList args;
	if (QFileInfo(python).fileName().compare(QStringLiteral("py.exe"), Qt::CaseInsensitive) == 0
	    || python == QLatin1String("py"))
		args << QStringLiteral("-3");
	args << QStringLiteral("-u") << script;
	m_process->start(python, args);
	if (!m_process->waitForStarted(10000)) {
		if (error) {
			*error = tr("Python could not be started (%1).").arg(python);
			if (store_shortcut)
				*error += QLatin1Char(' ') + tr("This is only Windows' shortcut to the "
								"Microsoft Store: install Python from python.org.");
		}
		delete m_process;
		m_process = nullptr;
		return false;
	}
	return true;
}

void AssistantBridge::stop()
{
	if (!m_process) return;
	m_stopping = true;
	if (m_process->state() != QProcess::NotRunning) {
		send(QJsonObject{{QStringLiteral("type"), QStringLiteral("quit")}});
		m_process->closeWriteChannel();
		if (!m_process->waitForFinished(3000)) {
			m_process->kill();
			m_process->waitForFinished(2000);
		}
	}
	m_process->deleteLater();
	m_process = nullptr;
}

void AssistantBridge::send(const QJsonObject &message)
{
	if (!isRunning()) return;
	m_process->write(QJsonDocument(message).toJson(QJsonDocument::Compact) + '\n');
}

void AssistantBridge::readOutput()
{
	if (!m_process) return;
	m_buffer += m_process->readAllStandardOutput();
	int newline;
	while ((newline = m_buffer.indexOf('\n')) >= 0) {
		const QByteArray line = m_buffer.left(newline).trimmed();
		m_buffer.remove(0, newline + 1);
		if (line.isEmpty()) continue;
		const QJsonDocument doc = QJsonDocument::fromJson(line);
		if (!doc.isObject()) {
			m_errors << QString::fromUtf8(line);
			continue;
		}
		const QJsonObject object = doc.object();
		QMetaObject::invokeMethod(this, [this, object]() { emit message(object); },
					  Qt::QueuedConnection);
	}
}

void AssistantBridge::readErrors()
{
	if (!m_process) return;
	const QString text = QString::fromUtf8(m_process->readAllStandardError());
	for (const QString &line : text.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
		m_errors << line.trimmed();
		qInfo().noquote() << "[assistant bridge]" << line.trimmed();
	}
	while (m_errors.size() > 40) m_errors.removeFirst();
}
