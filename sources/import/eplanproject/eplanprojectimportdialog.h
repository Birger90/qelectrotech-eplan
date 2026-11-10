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
#ifndef EPLANPROJECTIMPORTDIALOG_H
#define EPLANPROJECTIMPORTDIALOG_H

#include <QDialog>
#include <QProcess>

class QCheckBox;
class QDragEnterEvent;
class QDropEvent;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

/**
	@brief Dialog of "File > Import EPLAN project": turns a packed EPLAN
	project (.zw1) into a QElectroTech project.

	The translation itself is the Python program
	tools/eplan_import/eplan_zw1/eplan_nach_qet.py; the dialog finds it and
	a Python that can run it, starts it as a QProcess, shows its output and
	hands back the written .qet, which the editor then opens.
*/
class EplanProjectImportDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit EplanProjectImportDialog(QWidget *parent = nullptr);
		~EplanProjectImportDialog() override;

		/// asks for the archive; false when the user cancels
		bool chooseArchive();
		/// takes @p path as the archive to import
		void setArchive(const QString &path);
		/// the .qet the last import wrote, empty while there is none
		QString resultPath() const { return m_result; }

		/// eplan_nach_qet.py, or empty when it is nowhere to be found
		static QString findScript(const QString &application_dir);
		/// <folder of the archive>/QET/<name>_EPLAN_Import.qet, as the
		/// program itself names it
		static QString defaultTarget(const QString &archive);
		/// the command line for the program
		static QStringList arguments(const QString &script,
					     const QString &archive,
					     const QString &target,
					     bool pdf, bool data_portal,
					     bool collection);
		/// the path after "fertig: " in the program's output, or empty
		static QString resultFromOutput(const QString &output);

	protected:
		void reject() override;
		void dragEnterEvent(QDragEnterEvent *event) override;
		void dropEvent(QDropEvent *event) override;

	private:
		void chooseTarget();
		void chooseScript();
		void choosePython();
		void start();
		void readOutput();
		void processFinished(int exit_code, QProcess::ExitStatus status);
		void updateButtons();
		void appendLog(const QString &text);
		QString usablePython(const QString &script);
		bool pythonRuns(const QString &python, const QString &script, QString *error);

		QLineEdit *m_archive = nullptr;
		QLineEdit *m_target = nullptr;
		QLineEdit *m_script = nullptr;
		QLineEdit *m_python = nullptr;
		QCheckBox *m_pdf = nullptr;
		QCheckBox *m_data_portal = nullptr;
		QCheckBox *m_collection = nullptr;
		QLabel *m_status = nullptr;
		QPlainTextEdit *m_log = nullptr;
		QPushButton *m_start = nullptr;
		QPushButton *m_close = nullptr;
		QProcess *m_process = nullptr;
		QString m_output;
		QString m_result;
		bool m_target_edited = false;	///< the user chose the target themself
};

#endif // EPLANPROJECTIMPORTDIALOG_H
