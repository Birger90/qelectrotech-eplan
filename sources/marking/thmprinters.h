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

#include <QDateTime>
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
		/// The article variant loaded now, as last confirmed (material
		/// number, or number/layout when an article has several layouts)
	QString material_no;
	QString material_name;
	QDateTime loaded_at;         ///< when that loading was confirmed
		/// The article this printer usually carries, a default kept apart
		/// from what is loaded now (M-Print PRO: material assignment)
	QString usual_material_no;
	QString usual_material_name;
		/// The roller fitted for the usual article, as the user notes it
	QString roller;
	ThmJob::Profile profile;     ///< own profile, for an article without layout
	ThmJob::Family family() const { return ThmJob::familyFromModel(model); }
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
		/// The layout file chosen last (JSON, ThmLayouts), empty if none
	QString layoutsFile(QSettings &settings);
	void setLayoutsFile(QSettings &settings, const QString &path);

	QString newId(const QList<ThmPrinter> &existing);
		/// Address and loaded article; with @p check_profile also the
		/// printer's own profile (not when the profile comes from a layout)
	bool validate(const ThmPrinter &printer, QString *error = nullptr,
				  bool check_profile = true);
		/// Material change confirmed by the user: @p material_no is loaded
		/// now (at @p when); with @p usual it becomes the usual one too
	void confirmLoading(ThmPrinter *printer, const QString &material_no,
						const QString &material_name, const QDateTime &when,
						bool usual = false);
		/// Whether the printer carries something else than usual
	bool changedFromUsual(const ThmPrinter &printer);
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
