/*
	Copyright 2006-2026 The QElectroTech Team
	This file is part of QElectroTech.

	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with QElectroTech. If not, see <http://www.gnu.org/licenses/>.
*/
#include "eplanprojectimportdialog.h"

#include "../../qetapp.h"
#include "../../panel/eplanproject.h"

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace {
	const char DIR_KEY[]        = "eplanimport/project_dir";
	const char PYTHON_KEY[]     = "eplanimport/python";
	const char SCRIPT_KEY[]     = "eplanimport/project_script";
	const char PDF_KEY[]        = "eplanimport/project_pdf";
	const char PORTAL_KEY[]     = "eplanimport/project_data_portal";
	const char COLLECTION_KEY[] = "eplanimport/project_collection";
	const char SCRIPT_NAME[]    = "eplan_nach_qet.py";

	/// what the program needs besides the arguments: this QElectroTech
	/// for its own runs, the collections the user set, UTF-8 output
	QProcessEnvironment importEnvironment()
	{
		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert(QStringLiteral("QET_EXE"),
			   QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
		env.insert(QStringLiteral("QET_BINARY"),
			   QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
		const QString company = QETApp::companyElementsDir();
		if (!company.isEmpty())
			env.insert(QStringLiteral("QET_COMPANY_ELEMENTS_DIR"), QDir::toNativeSeparators(company));
		const QString common = QETApp::commonElementsDir();
		if (!common.isEmpty())
			env.insert(QStringLiteral("QET_COMMON_ELEMENTS_DIR"), QDir::toNativeSeparators(common));
			//Where the program keeps the title blocks it makes, and the 7-Zip
			//it unpacks with: found here, so that it does not have to know
			//where each system puts them
		env.insert(QStringLiteral("QET_TITLEBLOCKS_DIR"),
			   QDir::toNativeSeparators(QETApp::customTitleBlockTemplatesDir()));
		if (!env.contains(QStringLiteral("SIEBENZIP"))) {
			const QString seven = EplanProject::sevenZip();
			if (!seven.isEmpty())
				env.insert(QStringLiteral("SIEBENZIP"), QDir::toNativeSeparators(seven));
		}
		env.insert(QStringLiteral("PYTHONIOENCODING"), QStringLiteral("utf-8"));
		env.insert(QStringLiteral("PYTHONUTF8"), QStringLiteral("1"));
		env.insert(QStringLiteral("PYTHONUNBUFFERED"), QStringLiteral("1"));
		return env;
	}

	QLineEdit *pathRow(QWidget *parent, QFormLayout *form, const QString &label,
			   QPushButton **button)
	{
		auto *row = new QHBoxLayout;
		auto *edit = new QLineEdit(parent);
		*button = new QPushButton(QCoreApplication::translate("EplanProjectImportDialog", "Change…"), parent);
		row->addWidget(edit, 1);
		row->addWidget(*button);
		form->addRow(label, row);
		return edit;
	}
}

EplanProjectImportDialog::EplanProjectImportDialog(QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Import an EPLAN project"));
	setAcceptDrops(true);
	resize(760, 560);

	auto *intro = new QLabel(tr("Reads a packed EPLAN project (.zw1) and writes a QElectroTech "
				    "project with its pages, symbols, connections, cross references and "
				    "lists. The import opens in QElectroTech when it is done."), this);
	intro->setWordWrap(true);

	auto *form = new QFormLayout;
	QPushButton *archive_button = nullptr, *target_button = nullptr,
		*script_button = nullptr, *python_button = nullptr;
	m_archive = pathRow(this, form, tr("EPLAN project (.zw1):"), &archive_button);
	m_target = pathRow(this, form, tr("Save as:"), &target_button);

	QSettings settings;
	m_pdf = new QCheckBox(tr("Also write a PDF (takes a while on big projects)"), this);
	m_pdf->setChecked(settings.value(PDF_KEY, false).toBool());
	m_data_portal = new QCheckBox(tr("Fetch missing parts from the EPLAN Data Portal"), this);
	m_data_portal->setChecked(settings.value(PORTAL_KEY, true).toBool());
	m_collection = new QCheckBox(tr("Write parts of the project into the company collection"), this);
	m_collection->setChecked(settings.value(COLLECTION_KEY, true).toBool());
	form->addRow(QString(), m_pdf);
	form->addRow(QString(), m_data_portal);
	form->addRow(QString(), m_collection);

	m_script = pathRow(this, form, tr("Import program:"), &script_button);
	m_script->setText(QDir::toNativeSeparators(findScript(QCoreApplication::applicationDirPath())));
	m_python = pathRow(this, form, tr("Python:"), &python_button);
	m_python->setText(settings.value(PYTHON_KEY).toString());
	m_python->setPlaceholderText(tr("found when the import starts"));

	m_status = new QLabel(this);
	m_status->setWordWrap(true);
	m_progress = new QProgressBar(this);
	m_progress->setRange(0, 100);
	m_progress->setValue(0);
	m_progress->setTextVisible(true);
	m_progress->setFormat(QStringLiteral("%p %"));
	m_progress->setAlignment(Qt::AlignCenter);
	m_progress->setMinimumHeight(22);
	// green on every Windows style (Windows 11 would draw it in the accent colour)
	m_progress->setStyleSheet(QStringLiteral(
		"QProgressBar { border: 1px solid #8c8c8c; border-radius: 4px;"
		" background: #e8e8e8; color: #000000; text-align: center; }"
		"QProgressBar::chunk { background-color: #2ea043; border-radius: 3px; }"));
	m_progress->setVisible(false);
	m_log = new QPlainTextEdit(this);
	m_log->setReadOnly(true);
	m_log->setLineWrapMode(QPlainTextEdit::WidgetWidth);
	m_log->setPlaceholderText(tr("The output of the import appears here."));

	auto *buttons = new QDialogButtonBox(this);
	m_start = buttons->addButton(tr("Import"), QDialogButtonBox::AcceptRole);
	m_close = buttons->addButton(QDialogButtonBox::Close);
	m_start->setDefault(true);

	auto *layout = new QVBoxLayout(this);
	layout->addWidget(intro);
	layout->addLayout(form);
	layout->addWidget(m_status);
	layout->addWidget(m_progress);
	layout->addWidget(m_log, 1);
	layout->addWidget(buttons);

	connect(archive_button, &QPushButton::clicked, this, &EplanProjectImportDialog::chooseArchive);
	connect(target_button, &QPushButton::clicked, this, &EplanProjectImportDialog::chooseTarget);
	connect(script_button, &QPushButton::clicked, this, &EplanProjectImportDialog::chooseScript);
	connect(python_button, &QPushButton::clicked, this, &EplanProjectImportDialog::choosePython);
	connect(m_archive, &QLineEdit::textChanged, this, [this]() {
		if (!m_target_edited)
			m_target->setText(QDir::toNativeSeparators(defaultTarget(m_archive->text().trimmed())));
		updateButtons();
	});
	connect(m_target, &QLineEdit::textEdited, this, [this]() { m_target_edited = true; });
	connect(m_target, &QLineEdit::textChanged, this, &EplanProjectImportDialog::updateButtons);
	connect(m_script, &QLineEdit::textChanged, this, &EplanProjectImportDialog::updateButtons);
	// the buttons do not go through the box's accepted/rejected: "Import"
	// must not close the dialog
	connect(m_start, &QPushButton::clicked, this, &EplanProjectImportDialog::start);
	connect(m_close, &QPushButton::clicked, this, &EplanProjectImportDialog::reject);
	updateButtons();
}

EplanProjectImportDialog::~EplanProjectImportDialog()
{
	if (m_process && m_process->state() != QProcess::NotRunning) {
		m_process->disconnect(this);
		m_process->kill();
		m_process->waitForFinished(3000);
	}
}

/**
	The program in this order: the environment variable QET_EPLAN_IMPORT,
	the path chosen earlier, next to an installed QElectroTech
	(eplan_import/eplan_zw1/), and in the source tree a build/deploy
	folder lies in (tools/eplan_import/eplan_zw1/).
*/
QString EplanProjectImportDialog::findScript(const QString &application_dir)
{
	QStringList candidates;
	const QString env = qEnvironmentVariable("QET_EPLAN_IMPORT").trimmed();
	if (!env.isEmpty()) candidates << env;
	const QString chosen = QSettings().value(SCRIPT_KEY).toString();
	if (!chosen.isEmpty()) candidates << chosen;
	const QDir app(application_dir);
	const QString tail = QStringLiteral("eplan_import/eplan_zw1/") + SCRIPT_NAME;
	candidates << app.filePath(tail)
		   << app.filePath(QStringLiteral("../") + tail)
		   << app.filePath(QStringLiteral("../Resources/") + tail)	// macOS bundle
		   << app.filePath(QStringLiteral("../share/qelectrotech/") + tail)
		   << app.filePath(QStringLiteral("../../tools/") + tail)
		   << app.filePath(QStringLiteral("../tools/") + tail);
	for (const QString &c : candidates) {
		const QFileInfo fi(c);
		if (fi.isFile()) return QDir::cleanPath(fi.absoluteFilePath());
	}
	return QString();
}

QString EplanProjectImportDialog::defaultTarget(const QString &archive)
{
	if (archive.isEmpty()) return QString();
	const QFileInfo fi(archive);
	QString name = fi.completeBaseName();
	name.replace(QLatin1Char(' '), QLatin1Char('_'));
	return QDir(fi.absolutePath()).filePath(QStringLiteral("QET/") + name
						+ QStringLiteral("_EPLAN_Import.qet"));
}

QStringList EplanProjectImportDialog::arguments(const QString &script,
						const QString &archive,
						const QString &target,
						bool pdf, bool data_portal,
						bool collection)
{
	QStringList args{QStringLiteral("-u"), QDir::toNativeSeparators(script),
			 QDir::toNativeSeparators(archive), QDir::toNativeSeparators(target)};
	if (pdf) args << QStringLiteral("--pdf");
	if (data_portal) args << QStringLiteral("--dataportal");
	if (!collection) args << QStringLiteral("--ohne-sammlung");
	return args;
}

QString EplanProjectImportDialog::resultFromOutput(const QString &output)
{
	static const QRegularExpression done(QStringLiteral("^fertig: (.+?)\\s*$"),
					     QRegularExpression::MultilineOption);
	QString result;
	auto it = done.globalMatch(output);
	while (it.hasNext()) result = it.next().captured(1);
	return result;
}

QString EplanProjectImportDialog::progressText(const QString &line, int *percent)
{
	static const QRegularExpression progress(
		QStringLiteral("^FORTSCHRITT (\\d+) (\\w+)(?: (\\d+) (\\d+))?\\s*$"));
	const auto m = progress.match(line);
	if (!m.hasMatch()) return QString();
	if (percent) *percent = qBound(0, m.captured(1).toInt(), 100);
	const QString step = m.captured(2);
	if (step == QLatin1String("lesen"))         return tr("Unpacking and reading the EPLAN archive");
	if (step == QLatin1String("funktionen"))    return tr("Reading functions and articles");
	if (step == QLatin1String("seiten"))        return tr("Sorting the pages");
	if (step == QLatin1String("symbole"))       return tr("Reading symbols and page graphics");
	if (step == QLatin1String("sammlung"))      return tr("Looking up parts in the company collection");
	if (step == QLatin1String("portal"))        return tr("Looking up parts in the company collection and fetching missing ones from the EPLAN Data Portal");
	if (step == QLatin1String("kaesten"))       return tr("Device boxes and black boxes");
	if (step == QLatin1String("setzen"))
		return tr("Placing functions (%1 of %2)").arg(m.captured(3), m.captured(4));
	if (step == QLatin1String("grafik"))        return tr("Page graphics and title blocks");
	if (step == QLatin1String("unterbrechung")) return tr("Interruption points");
	if (step == QLatin1String("verbindungen"))  return tr("Drawing connections");
	if (step == QLatin1String("listen"))        return tr("Terminal diagram, cable overview and parts list");
	if (step == QLatin1String("schreiben"))     return tr("Writing the QElectroTech project");
	if (step == QLatin1String("pdf"))           return tr("Writing the PDF");
	if (step == QLatin1String("fertig"))        return tr("Done");
	return step;
}

bool EplanProjectImportDialog::chooseArchive()
{
	QString dir = QSettings().value(DIR_KEY).toString();
	if (!m_archive->text().trimmed().isEmpty())
		dir = QFileInfo(m_archive->text().trimmed()).absolutePath();
	const QString path = QFileDialog::getOpenFileName(this, tr("Import an EPLAN project"), dir,
		tr("EPLAN project archives (*.zw1);;All files (*)"));
	if (path.isEmpty()) return false;
	setArchive(path);
	return true;
}

void EplanProjectImportDialog::setArchive(const QString &path)
{
	QSettings().setValue(DIR_KEY, QFileInfo(path).absolutePath());
	m_archive->setText(QDir::toNativeSeparators(path));
}

void EplanProjectImportDialog::chooseTarget()
{
	QString start = m_target->text().trimmed();
	if (start.isEmpty()) start = defaultTarget(m_archive->text().trimmed());
	const QString path = QFileDialog::getSaveFileName(this, tr("Save the imported project as"), start,
		tr("QElectroTech projects (*.qet)"));
	if (path.isEmpty()) return;
	m_target_edited = true;
	m_target->setText(QDir::toNativeSeparators(path.endsWith(QStringLiteral(".qet"), Qt::CaseInsensitive)
						   ? path : path + QStringLiteral(".qet")));
}

void EplanProjectImportDialog::chooseScript()
{
	const QString path = QFileDialog::getOpenFileName(this, tr("Where is the import program?"),
		m_script->text().trimmed(), tr("Import program (%1)").arg(QString::fromLatin1(SCRIPT_NAME)));
	if (path.isEmpty()) return;
	QSettings().setValue(SCRIPT_KEY, path);
	m_script->setText(QDir::toNativeSeparators(path));
}

void EplanProjectImportDialog::choosePython()
{
#ifdef Q_OS_WIN
	const QString filter = tr("Programs (*.exe)");
#else
	const QString filter;
#endif
	const QString path = QFileDialog::getOpenFileName(this, tr("Which Python should run the import?"),
		m_python->text().trimmed(), filter);
	if (path.isEmpty()) return;
	m_python->setText(QDir::toNativeSeparators(path));
}

void EplanProjectImportDialog::updateButtons()
{
	const bool running = m_process && m_process->state() != QProcess::NotRunning;
	const QString archive = m_archive->text().trimmed();
	QString missing;
	if (archive.isEmpty())
		missing = tr("Choose the EPLAN project (.zw1).");
	else if (!QFileInfo(archive).isFile())
		missing = tr("The EPLAN project %1 does not exist.").arg(archive);
	else if (m_target->text().trimmed().isEmpty())
		missing = tr("Choose where the QElectroTech project is to be saved.");
	else if (!QFileInfo(m_script->text().trimmed()).isFile())
		missing = tr("The import program (%1) was not found. Choose it with \"Change…\".")
			.arg(QString::fromLatin1(SCRIPT_NAME));
	if (!running && !missing.isEmpty()) m_status->setText(missing);
	m_start->setEnabled(!running && missing.isEmpty());
	m_close->setText(running ? tr("Cancel") : tr("Close"));
	const QList<QWidget *> inputs{m_archive, m_target, m_script, m_python,
				      m_pdf, m_data_portal, m_collection};
	for (QWidget *w : inputs) w->setEnabled(!running);
}

/**
	Whether @p python imports the program with everything it needs
	(fontTools and the others); the import stops at the first missing piece.
*/
bool EplanProjectImportDialog::pythonRuns(const QString &python, const QString &script, QString *error)
{
	const QString dir = QFileInfo(script).absolutePath();
	const QString code = QStringLiteral(
		// fontTools and pymupdf are imported only where they are used
		"import sys; sys.path[:0] = [r'%1', r'%2']; import eplan_nach_qet, fontTools.ttLib, pymupdf")
		.arg(QDir::toNativeSeparators(dir),
		     QDir::toNativeSeparators(QFileInfo(dir).absolutePath()));
	QProcess p;
	p.setProcessEnvironment(importEnvironment());
	p.setWorkingDirectory(dir);
	p.setProcessChannelMode(QProcess::MergedChannels);
	p.start(python, {QStringLiteral("-c"), code});
	if (!p.waitForStarted(10000)) {
		if (error) *error = tr("%1 does not start.").arg(python);
		return false;
	}
	if (!p.waitForFinished(60000)) {
		p.kill();
		p.waitForFinished(3000);
		if (error) *error = tr("%1 did not answer.").arg(python);
		return false;
	}
	if (p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0) return true;
	if (error) {
		const QStringList lines = QString::fromUtf8(p.readAll()).trimmed().split(QLatin1Char('\n'));
		*error = QStringLiteral("%1: %2").arg(python, lines.isEmpty() ? QString() : lines.last().trimmed());
	}
	return false;
}

/**
	The Python in the field when it runs the program; otherwise the first
	that does among the ones the "py" launcher knows and the one on PATH.
	Empty when none does; the reasons are in the log then.
*/
QString EplanProjectImportDialog::usablePython(const QString &script)
{
	QStringList candidates;
	const QString chosen = m_python->text().trimmed();
	if (!chosen.isEmpty()) candidates << chosen;
#ifdef Q_OS_WIN
	const QString py = QStandardPaths::findExecutable(QStringLiteral("py"));
	if (!py.isEmpty()) {
		QProcess p;
		p.start(py, {QStringLiteral("-0p")});
		if (p.waitForFinished(10000)) {
			static const QRegularExpression path(QStringLiteral("([A-Za-z]:\\\\.*python\\.exe)\\s*$"),
							     QRegularExpression::CaseInsensitiveOption);
			const QStringList lines = QString::fromLocal8Bit(p.readAllStandardOutput()).split(QLatin1Char('\n'));
			for (const QString &line : lines) {
				const auto m = path.match(line.trimmed());
				if (m.hasMatch()) candidates << m.captured(1);
			}
		}
	}
	const QStringList names{QStringLiteral("python"), QStringLiteral("python3")};
#else
	const QStringList names{QStringLiteral("python3"), QStringLiteral("python")};
#endif
	for (const QString &name : names) {
		const QString found = QStandardPaths::findExecutable(name);
		// Windows' WindowsApps shortcut only opens the Store
		if (!found.isEmpty() && !found.contains(QStringLiteral("WindowsApps"), Qt::CaseInsensitive))
			candidates << found;
	}
	candidates.removeDuplicates();

	QStringList reasons;
	for (const QString &c : candidates) {
		m_status->setText(tr("Checking %1 …").arg(QDir::toNativeSeparators(c)));
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		QString why;
		if (pythonRuns(c, script, &why)) return c;
		reasons << why;
	}
	appendLog(tr("No Python runs the import program:") + QLatin1Char('\n') + reasons.join(QLatin1Char('\n'))
		  + QLatin1Char('\n')
		  + tr("Install what is missing, e.g. \"py -m pip install fonttools pymupdf\", "
		       "or choose another Python."));
	return QString();
}

void EplanProjectImportDialog::start()
{
	const QString archive = m_archive->text().trimmed();
	const QString target = QDir::cleanPath(QDir::fromNativeSeparators(m_target->text().trimmed()));
	const QString script = m_script->text().trimmed();

	if (QETApp::diagramEditorForFile(target)) {
		QMessageBox::warning(this, windowTitle(),
			tr("%1 is open in QElectroTech. Close it first or choose another file.")
				.arg(QDir::toNativeSeparators(target)));
		return;
	}
	if (QFileInfo::exists(target)
	    && QMessageBox::question(this, windowTitle(),
		       tr("%1 already exists. Replace it?").arg(QDir::toNativeSeparators(target)))
		       != QMessageBox::Yes)
		return;
	if (!QDir().mkpath(QFileInfo(target).absolutePath())) {
		QMessageBox::warning(this, windowTitle(),
			tr("The folder %1 cannot be created.")
				.arg(QDir::toNativeSeparators(QFileInfo(target).absolutePath())));
		return;
	}

	m_log->clear();
	m_output.clear();
	m_pending.clear();
	m_result.clear();
	m_progress->setValue(0);
	m_progress->setVisible(true);
	m_start->setEnabled(false);
	QApplication::setOverrideCursor(Qt::BusyCursor);
	const QString python = usablePython(script);
	QApplication::restoreOverrideCursor();
	if (python.isEmpty()) {
		updateButtons();
		m_status->setText(tr("No usable Python found."));
		return;
	}
	m_python->setText(QDir::toNativeSeparators(python));

	QSettings settings;
	settings.setValue(PYTHON_KEY, python);
	settings.setValue(PDF_KEY, m_pdf->isChecked());
	settings.setValue(PORTAL_KEY, m_data_portal->isChecked());
	settings.setValue(COLLECTION_KEY, m_collection->isChecked());

	m_process = new QProcess(this);
	m_process->setProcessEnvironment(importEnvironment());
	m_process->setWorkingDirectory(QFileInfo(script).absolutePath());
	m_process->setProcessChannelMode(QProcess::MergedChannels);
	connect(m_process, &QProcess::readyRead, this, &EplanProjectImportDialog::readOutput);
	connect(m_process, &QProcess::finished, this, &EplanProjectImportDialog::processFinished);
	connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError e) {
		if (e != QProcess::FailedToStart || !m_process) return;
		appendLog(tr("The import does not start: %1").arg(m_process->errorString()));
		m_process->deleteLater();
		m_process = nullptr;
		updateButtons();
		m_status->setText(tr("The import did not start."));
	});
	const QStringList args = arguments(script, archive, target, m_pdf->isChecked(),
					   m_data_portal->isChecked(), m_collection->isChecked());
	appendLog(QStringLiteral("> %1 %2").arg(QDir::toNativeSeparators(python), args.join(QLatin1Char(' '))));
	m_process->start(python, args);
	updateButtons();
	m_status->setText(tr("Importing %1 … this can take several minutes.").arg(QFileInfo(archive).fileName()));
}

void EplanProjectImportDialog::readOutput()
{
	if (!m_process) return;
	m_pending += QString::fromUtf8(m_process->readAll());
	// whole lines only: progress lines move the bar, all others go to the log
	const int end = m_pending.lastIndexOf(QLatin1Char('\n'));
	if (end < 0) return;
	const QStringList lines = m_pending.left(end).split(QLatin1Char('\n'));
	m_pending.remove(0, end + 1);
	for (QString line : lines) {
		line.remove(QLatin1Char('\r'));
		int percent = 0;
		const QString step = progressText(line, &percent);
		if (!step.isEmpty()) {
			m_progress->setValue(percent);
			m_status->setText(step);
			continue;
		}
		m_output += line + QLatin1Char('\n');
		m_log->appendPlainText(line);
	}
}

void EplanProjectImportDialog::processFinished(int exit_code, QProcess::ExitStatus status)
{
	readOutput();
	if (!m_pending.isEmpty()) {		// a last line without line break
		const QString rest = m_pending;
		m_pending.clear();
		for (const QString &line : rest.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
			int percent = 0;
			if (progressText(line.trimmed(), &percent).isEmpty()) {
				m_output += line + QLatin1Char('\n');
				m_log->appendPlainText(line);
			} else {
				m_progress->setValue(percent);
			}
		}
	}
	m_process->deleteLater();
	m_process = nullptr;
	const QString result = resultFromOutput(m_output);
	updateButtons();
	if (status == QProcess::NormalExit && exit_code == 0 && QFileInfo(result).isFile()) {
		m_progress->setValue(100);
		m_result = QDir::cleanPath(QDir::fromNativeSeparators(result));
		m_status->setText(tr("Done: %1").arg(QDir::toNativeSeparators(m_result)));
		accept();
		return;
	}
	m_status->setText(status == QProcess::CrashExit
			  ? tr("The import was stopped.")
			  : tr("The import failed (exit code %1). The reason is in the output below.").arg(exit_code));
}

void EplanProjectImportDialog::appendLog(const QString &text)
{
	m_log->appendPlainText(text);
}

void EplanProjectImportDialog::reject()
{
	if (m_process && m_process->state() != QProcess::NotRunning) {
		if (QMessageBox::question(this, windowTitle(), tr("Stop the running import?"))
		    != QMessageBox::Yes)
			return;
		m_process->kill();	// processFinished() tidies up
		return;
	}
	QDialog::reject();
}

void EplanProjectImportDialog::dragEnterEvent(QDragEnterEvent *event)
{
	const auto urls = event->mimeData()->urls();
	if (!m_process && urls.size() == 1 && urls.first().isLocalFile()
	    && urls.first().toLocalFile().endsWith(QStringLiteral(".zw1"), Qt::CaseInsensitive))
		event->acceptProposedAction();
}

void EplanProjectImportDialog::dropEvent(QDropEvent *event)
{
	setArchive(event->mimeData()->urls().first().toLocalFile());
	event->acceptProposedAction();
}
