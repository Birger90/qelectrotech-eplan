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
#ifndef ENCLOSURESIZE_H
#define ENCLOSURESIZE_H

#include <QString>
#include <QStringList>
#include <QVector>

/**
	@brief Outer sizes of an enclosure in millimetres: width, height and
	depth; 0 stands for "not known" in a size read, and for "any" in a
	size asked for.

	The EPLAN Data Portal has no field for the sizes of a part: neither
	its API nor the part data of a DXF package (commercialdata.csv). They
	stand in the description only, e.g. "BHT: 800x1400x500 mm", "B x H x
	T: 1200x2000x800 mm", "Breite/Höhe/Tiefe: 1600 x 1800 x 400", "WHD:
	1600x1800x500" and, for parts of the American market, height first:
	"HWD 200x150x150 mm". fromText() reads these.
*/
struct EnclosureSize
{
	double width = 0, height = 0, depth = 0;

	bool isNull() const {return width <= 0 && height <= 0 && depth <= 0;}

		/// the sizes named in a description; null when it names none
	static EnclosureSize fromText(const QString &text);

		/// The width and height of a mounting plate named in its
		/// description: "BH: 275x585 mm", "B x H: 899 x 1696 mm". Null
		/// when it names none, or names the enclosure the plate is for
		/// ("für BH: 800x300 mm"); the depth stays 0.
	static EnclosureSize plateFromText(const QString &text);

		/// the series of a part by its type number: "KX" of "KX.1570800"
	static QString seriesOf(const QString &type_number);

		/// How far @p of is larger than this size asked for, summed over
		/// the sizes asked for (those above 0), in mm: 0 is an exact fit.
		/// Below 0: @p of is smaller in one of them, or does not say.
	double excess(const EnclosureSize &of) const;

		/// The list of enclosures to choose from: of @p texts (one per
		/// enclosure: its numbers and description) the indexes of those
		/// of at least this size that hold every word of @p words, the
		/// closest fit first, then small to large. Without a size all
		/// that hold the words, those naming no size last. An empty text
		/// is left out. @p sizes gets the size read from each text.
	QVector<int> choose(const QStringList &texts, const QString &words,
						QVector<EnclosureSize> *sizes = nullptr) const;
};

#endif // ENCLOSURESIZE_H
