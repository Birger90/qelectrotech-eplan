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
#ifndef SPLANIMPORTDIALOG_H
#define SPLANIMPORTDIALOG_H

#include <QDialog>
#include <QProcess>

class QCheckBox;
class QDragEnterEvent;
class QDropEvent;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;

/**
	@brief Dialog of the imports that a Python program translates:
	- "File > Import sPlan schematic": turns an sPlan 8 schematic (.spl8,
	  Abacom) into a QElectroTech project that is drawn anew: the same
	  devices and connections, laid out on the grid
	  (tools/splan_import/splan_nach_qet.py);
	- "File > Import WSCAD project": turns a WSCAD project folder
	  (Database/WSCAD6.db and the .wsELD schematic pages, WSCAD ELECTRIX /
	  SUITE 6.x and later) into an editable QElectroTech project with the
	  same sheets, symbols, wires and cross references
	  (tools/wscad_import/wscad_nach_qet.py).

	The dialog finds the program and a Python that can run it, starts it as
	a QProcess, moves the progress bar with its FORTSCHRITT lines and hands
	back the written .qet, which the editor then opens. Both imports share
	this one dialog; Kind picks the texts, the program and the settings.
*/
class SplanImportDialog : public QDialog
{
	Q_OBJECT

	public:
		/// which import the dialog runs
		enum class Kind { Splan, Wscad };

		explicit SplanImportDialog(QWidget *parent = nullptr, Kind kind = Kind::Splan);
		~SplanImportDialog() override;

		/// asks for the schematic (sPlan) or the project folder (WSCAD); false
		/// when the user cancels
		bool chooseSource();
		/// takes @p path as the schematic or project to import
		void setSource(const QString &path);
		/// the .qet the last import wrote, empty while there is none
		QString resultPath() const { return m_result; }

		/// splan_nach_qet.py or wscad_nach_qet.py, or empty when it is nowhere
		/// to be found
		static QString findScript(const QString &application_dir, Kind kind = Kind::Splan);
		/// sPlan: <folder of the schematic>/QET/<name>.qet; WSCAD:
		/// <project folder>/QET/<project>_WSCAD_Import.qet, as the programs
		/// name them
		static QString defaultTarget(const QString &source, Kind kind = Kind::Splan);
		/// the command line for the program
		static QStringList arguments(const QString &script, const QString &source,
					     const QString &target, bool pdf, Kind kind = Kind::Splan);
		/// the path after "fertig: " in the program's output, or empty
		static QString resultFromOutput(const QString &output);
		/// the text for a progress line "FORTSCHRITT <percent> <step>" of the
		/// program, in the language of QElectroTech; @p percent gets the
		/// percentage. Empty when @p line is no progress line.
		static QString progressText(const QString &line, int *percent, Kind kind = Kind::Splan);
		/// WSCAD: the project folder of @p path (the folder itself, a page
		/// file in it or Database/WSCAD6.db), empty when it is none
		static QString wscadProjectFolder(const QString &path);

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
		QString usablePython(const QString &script);
		bool pythonRuns(const QString &python, const QString &script, QString *error);

		QLineEdit *m_source = nullptr;
		QLineEdit *m_target = nullptr;
		QLineEdit *m_script = nullptr;
		QLineEdit *m_python = nullptr;
		QCheckBox *m_pdf = nullptr;
		QLabel *m_status = nullptr;
		QPlainTextEdit *m_log = nullptr;
		QProgressBar *m_progress = nullptr;
		QPushButton *m_start = nullptr;
		QPushButton *m_close = nullptr;
		QProcess *m_process = nullptr;
		QString m_output;
		QString m_pending;	///< output after the last line break
		QString m_result;
		bool m_target_edited = false;	///< the user chose the target themself
		Kind m_kind = Kind::Splan;
};

#endif // SPLANIMPORTDIALOG_H
