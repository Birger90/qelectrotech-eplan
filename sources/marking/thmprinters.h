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
#ifndef THMPRINTERS_H
#define THMPRINTERS_H

#include "thmjob.h"

#include <QList>
#include <QString>

class QSettings;

/**
	@brief The THM printers a user has set up, kept in the settings
	(group "thm_print", array "printers"): each with its address and the
	article variant it is loaded with, and the print profile of that
	material. No address is built in: a printer exists only once the user
	has entered it.

	A print goes to a printer loaded with the article chosen: choose()
	finds them, and says what is missing when there is none.
*/
struct ThmPrinter
{
	QString id;          ///< fixed when created, the key for --printer
	QString name;
	QString model;       ///< as the user calls it, e.g. what v answers
	QString host;
	quint16 port = 9100;
	QString material_no; ///< the article variant loaded (material number)
	QString material_name;
	ThmJob::Profile profile;
};

namespace ThmPrinters
{
	QList<ThmPrinter> load(QSettings &settings);
	void save(QSettings &settings, const QList<ThmPrinter> &printers);
		/// The application's settings
	QList<ThmPrinter> load();
	void save(const QList<ThmPrinter> &printers);

		/// The catalog file chosen last (my_cat*.mca), empty if none
	QString catalogFile(QSettings &settings);
	void setCatalogFile(QSettings &settings, const QString &path);

	QString newId(const QList<ThmPrinter> &existing);
	bool validate(const ThmPrinter &printer, QString *error = nullptr);
		/// Material numbers compared without spaces and case
	QString normalizedMaterial(const QString &material_no);
	int indexOf(const QList<ThmPrinter> &printers, const QString &id);

	struct Choice
	{
		enum Outcome {
			NoPrinters,  ///< none set up
			NoMaterial,  ///< no article chosen
			NoMatch,     ///< none loaded with the article
			Unique,      ///< exactly one: chosen
			Several      ///< the user picks one of matches
		};
		Outcome outcome = NoPrinters;
		QList<int> matches;  ///< indexes into the list given
		QString message;     ///< for a person
	};
	Choice choose(const QList<ThmPrinter> &printers, const QString &material_no);
}

#endif // THMPRINTERS_H
