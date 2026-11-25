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
#ifndef MARKINGEXPORT_H
#define MARKINGEXPORT_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QString>

class QETProject;

/**
	@brief Marking export: the wire, terminal, device and cable markers of a
	project as a file for the marking software of the terminal makers
	(Phoenix Contact MARKING system / PROJECT complete, Weidmüller M-Print PRO,
	WAGO Smart Script), or as a neutral CSV.

	Everything is read from the public views of the project database
	(qet_wires, qet_terminals, qet_device_list, qet_cables), the rows the
	wiring list, the terminal strip list and the cable list show, so the
	markers say what the drawing says.

	None of the three makers publishes a fixed column layout: each import
	assigns the columns of the file by hand once and keeps that assignment
	(sources in docs/beschriftung-export.md). So every format is a CSV with a
	header line, cut to what that import is documented to do -- the column
	the material is assigned from, whether a quantity column is read, the
	character set -- and described column by column in that document.
*/
namespace MarkingExport
{
		/// What a marker is put on, as flags to choose several
	enum Kind {
		Wire = 1,      ///< one marker per wire end: the wire number
		Terminal = 2,  ///< one marker per terminal, and one per strip
		Device = 4,    ///< one marker per device (reference designation)
		Cable = 8,     ///< one marker per cable end
		AllKinds = 15
	};

		/// The file written
	enum Format {
		Neutral,         ///< described CSV, every column
		PhoenixContact,  ///< MARKING system / PROJECT complete
		Weidmueller,     ///< M-Print PRO
		Wago             ///< Smart Script
	};

	struct Options
	{
		int kinds = AllKinds;
			/// Folios by number, counted from 1 as the title blocks do;
			/// 0 leaves that end open
		int folio_from = 0;
		int folio_to = 0;
			/// Only what lies in this plant (=) and location (+); empty
			/// takes every one. Compared without the sign.
		QString plant;
		QString location;
			/// =plant+location-label, else only -label
		bool full_designation = true;
			/// Each wire end its own marker and two per cable; else one
		bool both_ends = true;
		Format format = Neutral;
			/// Weidmüller: material number per kind key (Draht, Klemme,
			/// Klemmleiste, Geraet, Kabel) for the reference column; a kind
			/// without one keeps its alias
		QHash<QString, QString> materials;
	};

		/// One marker, or several alike (count)
	struct Item
	{
		Kind kind = Device;
		QString text;         ///< what is printed
		QString target;       ///< wire: the other end; cable: its type
		QString group;        ///< terminal: its strip; wire: the end it is on
		QString plant;
		QString location;
		QString folio;        ///< the folio as its title block names it
		int folio_number = 0; ///< counted from 1
		QString manufacturer;
		QString article;
		int count = 1;
		bool strip_marker = false; ///< terminal kind: the strip's own marker
	};

	QList<Item> collect(QETProject *project, const Options &options,
						QString *error = nullptr);
		/// The file's bytes; @p markers gets the number of markers in it
	QByteArray write(const QList<Item> &items, Format format,
					 int *markers = nullptr,
					 const QHash<QString, QString> &materials = {});
	bool exportFile(QETProject *project, const QString &path,
					const Options &options, QString *error = nullptr,
					int *markers = nullptr);

		/// The reference a marker's material is assigned from
	QString materialReference(const Item &item);

		/// "neutral", "phoenix", "weidmueller", "wago"
	QString formatKey(Format format);
	bool formatFromKey(const QString &key, Format *format);
	QString formatTitle(Format format);
	QString fileFilter(Format format);
		/// Comma separated: wires, terminals, devices, cables (or the
		/// German Draht, Klemme, Gerät, Kabel; "all")
	bool kindsFromKeys(const QString &keys, int *kinds);
	QString kindTitle(Kind kind);
}

#endif // MARKINGEXPORT_H
