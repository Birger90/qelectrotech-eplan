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
#ifndef THMMATERIALS_H
#define THMMATERIALS_H

#include "thmcatalog.h"
#include "thmlayouts.h"
#include "thmprinters.h"

#include <QList>
#include <QString>
#include <QStringList>

/**
	@brief The materials to choose from, as M-Print PRO lists them: one row
	per article variant with category, size, colour and sides, filtered by
	these. Built from the layout file (sizes, refusals) and the catalog file
	(colour from its description), whichever is chosen; no widget, so it
	can be tested on its own.
*/
namespace ThmMaterials
{
	struct Entry
	{
		QString article;
		QString key;         ///< variant key (article/layout), empty without layout
		QString name;
		QString category;
		double width = 0;    ///< one marker, mm; 0 unknown
		double height = 0;
		QString colour;      ///< e.g. "Gelb"; empty unknown
		bool two_sided = false;
		int columns = 0;     ///< markers side by side; 0 unknown
		QString refusal;     ///< why it cannot be printed; empty: it can
		bool has_layout = false;
		QStringList loaded_on;  ///< names of the printers loaded with it
		QString sizeText() const;  ///< "15 × 6 mm", empty unknown
	};

	struct Filter
	{
		QString text;        ///< in name or number, any case
		QString category;    ///< empty: all
		QString colour;      ///< empty: all
		QString size;        ///< sizeText(); empty: all
		int sides = 0;       ///< 0 all, 1 one-sided, 2 two-sided
		bool printable_only = true;
		bool loaded_only = false;
	};

		/// One entry per layout variant, then the catalog's articles that
		/// have no layout; sorted by name
	QList<Entry> entries(const QList<ThmLayouts::Variant> &layouts,
						 const QList<ThmCatalog::Variant> &catalog,
						 const QList<ThmPrinter> &printers);
	bool matches(const Entry &entry, const Filter &filter);

		/// "Farbe: Gelb" of a catalog description, else the colour code at
		/// the end of the name (WS, GE, RT, BL, ...)
	QString colourOf(const QString &name, const QString &description = QString());
		/// The distinct values of a column, sorted
	QStringList categories(const QList<Entry> &entries);
	QStringList colours(const QList<Entry> &entries);
	QStringList sizes(const QList<Entry> &entries);
}

#endif // THMMATERIALS_H
