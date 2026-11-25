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
#ifndef THMRENDER_H
#define THMRENDER_H

#include "thmjob.h"

#include <QByteArray>
#include <QFont>
#include <QImage>
#include <QStringList>

/**
	@brief Labels printed as pictures, as M-Print PRO does: each row of
	the material is drawn into a black and white PNG at the printer's
	resolution and sent with the label data, so the print is what the
	preview shows (any font, any size, shrunk to fit).

	The job has the form of M-Print PRO's own THM output (checked against
	its .prn files): eIMG;* (erase the stored pictures), dPNG;0 with the
	picture between ESC : and ESC end-of-data, then the label lines with
	I:Field1;0.00,0.00,0;0 putting picture 0 at the top left, and A n.
	The preview (ThmLabelSheet) uses the same font and fitting.
*/
namespace ThmRender
{
		/// The printers' resolution: 300 dpi, dots per millimetre
	constexpr double kDotsPerMm = 300.0 / 25.4;
		/// The font of the labels at @p units_per_mm (pixels per mm)
	QFont font(const ThmJob::Profile &profile, double units_per_mm);
		/// Shrink factor (at most 1) so @p text fits @p room (same units
		/// as the font); M-Print PRO's "adapt font size"
	double fit(const QFont &font, const QString &text, double room);
		/// The font @p text is drawn with in a label of @p profile, at
		/// @p units_per_mm: the profile's size, shrunk until it fits
	QFont fontFor(const ThmJob::Profile &profile, const QString &text, double units_per_mm);
		/// Whether @p text needs shrinking to fit a label of @p profile
	bool overflows(const ThmJob::Profile &profile, const QString &text);
		/// One row of the material with the texts of its columns, black on
		/// white at kDotsPerMm, as wide as the row and as high as a label
	QImage renderRow(const QStringList &texts, const ThmJob::Profile &profile);
		/// The whole job, one picture per different row; empty with
		/// @p error set when the profile or the labels do not allow it
	QByteArray build(const QList<ThmJob::Label> &labels, const ThmJob::Profile &profile,
					 QString *error = nullptr);
}

#endif // THMRENDER_H
