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
#ifndef COMPANYDISTRIBUTION_H
#define COMPANYDISTRIBUTION_H

#include <QDialog>
#include <QImage>
#include <QString>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPlainTextEdit;

/**
	Distribution of the fork inside a company without git: a shared folder
	(e.g. a SharePoint library synchronised by OneDrive) holds

	- version.json   {"product": "QET-Fork", "version": "1.31.0",
	                  "package": "Programm/QET-Fork-1.31.0.zip", "notes": "…"}
	- Programm/      the zipped program of each version
	- Installieren.ps1 / Installieren.cmd   install and update script
	- Bauteile/      the company element collection (read only for the users)
	- Meldungen/     bug reports and wishes written by the program

	The folder is the setting "distribution/folder", the environment variable
	QET_DISTRIBUTION_DIR, or found among the OneDrive synchronised folders.
	Everything here is optional: without the folder the program runs as before.
*/
namespace CompanyDistribution {
	/// the distribution folder, empty when there is none
	QString folder();
	/// looks for a synchronised folder that holds a version.json of the product
	QString detectFolder();
	/// true when version @a a (1.30.0) is newer than @a b
	bool isNewer(const QString &a, const QString &b);
	/// before the collections load: the company collection is the folder "Bauteile"
	void applyCompanyCollection();
	/// after the start: hand in reports written offline, offer an update, keep the parts offline
	void startupTasks();
	/// shows whether an update is there; @a quiet: say nothing when there is none
	void checkForUpdate(bool quiet);
}

/**
	@brief The ReportDialog class
	"Help > Report a bug or a wish": the text, the version, an optional
	screenshot and the diagnostic report go into a folder of
	Meldungen/ in the distribution folder (or stay local until it is there).
*/
class ReportDialog : public QDialog
{
	Q_OBJECT
	public:
		explicit ReportDialog(QWidget *parent = nullptr);

	private:
		void send();

		QComboBox *m_kind;
		QLineEdit *m_title, *m_name;
		QPlainTextEdit *m_text;
		QCheckBox *m_screenshot, *m_diagnostics;
		QImage m_image;
};

#endif // COMPANYDISTRIBUTION_H
