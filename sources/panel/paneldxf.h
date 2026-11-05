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
#ifndef PANELDXF_H
#define PANELDXF_H

#include <QString>
#include <QStringList>

class PanelLayout;

/**
	@brief Manufacturing DXF of one mounting surface (EPLAN: NC data, DXF):
	one file per field, millimetres, the surface's zero point at (0,0).

	Layers: FELD (outline of the field), BOHRUNG_D<d> (drilled holes),
	GEWINDE_M<d> (threaded holes, circle of the nominal size), AUSSCHNITT
	(rectangles, slots, hexagons, octagons as closed polylines),
	BESCHRIFTUNG (optional: what made each hole). Written as DXF R12
	(AC1009), the version every CAD and CAM program reads.
*/
class PanelDxf
{
	public:
		struct Options {
			bool labels = false;     ///< write BESCHRIFTUNG texts
			bool thread_core = false; ///< threads as core hole circle instead of nominal size
		};

		static bool write(const PanelLayout &layout, const QString &path,
						  const Options &options, QString *error = nullptr);
		static double coreDiameter(double thread);  ///< metric coarse thread core drill
};

#endif // PANELDXF_H
