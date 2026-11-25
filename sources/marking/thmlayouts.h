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
#ifndef THMLAYOUTS_H
#define THMLAYOUTS_H

#include "thmjob.h"
#include "thmprinters.h"

#include <QByteArray>
#include <QList>
#include <QString>

/**
	@brief The print layouts of the maker's article variants, read from a
	layout file the user chooses; nothing of it is part of QElectroTech,
	and QElectroTech decrypts nothing.

	The file is JSON: "layouts" (by layout file name) with the page size,
	the markers (labels) on it and their text field, all lengths in
	micrometres, and "variants" (article number, name, layout,
	"printer_parameters_raw" as the maker's software keeps them). Read are
	only the keys named in docs/beschriftung-export.md.

	From a variant and a device family comes a ThmJob::Profile, the way the
	maker's software writes its THM jobs (compared with its own .prn
	output): S over the whole page width, page height = label height =
	pitch, the vertical offset from print_position_mmp, H with the
	MultiMark speed and darkness, B30 and O T,P for tear off, else O P.
	What cannot be printed that way yet is refused with a reason:
	continuous material with layout rules (multi_rows), pages wider than
	the print head, layouts without a marker text field.
*/
namespace ThmLayouts
{
	struct Variant
	{
		QString article;    ///< material number
		QString name;
		QString category;
		QString layout;     ///< the layout's file name
		QString ribbon;     ///< the ribbon the maker names for MultiMark
		QString support_plate;
			/// The geometry, millimetres
		double page_width = 0;
		double page_height = 0;
		bool pitched_strip = false;  ///< page axes turned (strips such as DEK)
		int columns = 0;
		double first_x = 0;
		double column_pitch = 0;
		double marker_y = 0;
		double marker_width = 0;
		double marker_height = 0;
		double field_x0 = 0, field_y0 = 0, field_x1 = 0, field_y1 = 0;
		int orientation = 0;         ///< of the text field, degrees
		double font_points = 0;
			/// The print parameters
		QString sensor;              ///< gap, continuous
		QString print_mode;          ///< normal, tear_off
		double position_y = 0;       ///< print_position_mmp, mm
		int speed_multimark = 0;     ///< print_speed_muma
		int speed_plus = 0;          ///< print_speed_muma_plus
		int darkness = 0;            ///< print_darkness_muma_plus
		bool rotate = false;         ///< rotate_180_degrees on
			/// Empty when it can be printed, else why not
		QString refusal;
		QString key() const { return article + QLatin1Char('/') + layout; }
	};

	QList<Variant> parse(const QByteArray &json, QString *error = nullptr);
	QList<Variant> read(const QString &path, QString *error = nullptr);

		/// The variants of an article ("2005610000"), or exactly one by
		/// its key ("2005610000/lab_tm_i_18_mm.mcf"); indexes into @p all
	QList<int> find(const QList<Variant> &all, const QString &article_or_key);
		/// The printable one of find(), the first if several; -1 if none
	int best(const QList<Variant> &all, const QString &article_or_key);
		/// "name – article"
	QString title(const Variant &variant);

		/// The profile of @p variant for a printer of @p family. False with
		/// @p error when the variant cannot be printed (its refusal, or a
		/// profile ThmJob::validate() does not take).
	bool profile(const Variant &variant, ThmJob::Family family,
				 ThmJob::Profile *out, QString *error = nullptr);

		/// The profile a job on @p article is printed with: the layout's
		/// when @p layouts has the article (refused variants are not
		/// printed with anything else), else the own profile of @p printer.
		/// The family is the printer's, MMP without one. @p source says
		/// which, for a person.
	bool jobProfile(const QList<Variant> &layouts, const QString &article,
					const ThmPrinter *printer, ThmJob::Profile *out,
					QString *source = nullptr, QString *error = nullptr);

		/// The layout file of the settings, read once per path
	const QList<Variant> &settingsLayouts(QString *path = nullptr, QString *error = nullptr);
}

#endif // THMLAYOUTS_H
