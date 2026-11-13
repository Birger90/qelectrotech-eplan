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
#ifndef EPLANPROJECT_H
#define EPLANPROJECT_H

#include <QList>
#include <QPair>
#include <QPointF>
#include <QRectF>
#include <QString>

/**
	@brief The EplanProject class
	Reads the panel layout of an EPLAN Electric P8 project: where each
	device sits on the mounting plate. A project archive (.zw1, a 7z
	archive) is unpacked with 7-Zip; a project folder (.edb) is read as it
	is. Only two tables are needed: Function.eod (the devices with their
	full designation, page, insertion point and outline) and Instance.eod
	(the mounting plate macro on the layout page, its corner is the zero
	point of the plate).

	The tables are EPLAN's own binary format; the fields read here carry
	EPLAN property numbers (1100 plant, 1200 location, 20013/20014/20015
	identifier, counter and sub counter of the designation).
*/
class EplanProject
{
	public:
		struct Placement {
			QString plant, location, bmk;   ///< "SC", "CA", "-A665"
			QRectF rect_mm;                 ///< on the plate, y up from its lower left corner
			bool centre_only = false;       ///< no outline: rect_mm is a point at the centre
		};
		struct Layout {
			QList<Placement> items;
				/// texts on the layout page (e.g. the type written on a plain
				/// box) with their position on the plate, upper case
			QList<QPair<QString, QPointF>> texts;
			QString error;
		};
		static Layout readLayout(const QString &path);
			///7-Zip: on the search path (7z, 7zz, 7za) or in its Windows folder; empty when missing
		static QString sevenZip();

	private:
		static QString unpack(const QString &zw1, const QString &dir, QString *error);
};

#endif
