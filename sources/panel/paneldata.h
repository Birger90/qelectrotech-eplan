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
#ifndef PANELDATA_H
#define PANELDATA_H

#include <QDomElement>
#include <QList>
#include <QPointF>
#include <QString>

/**
	@brief One drilled or cut feature, the way EPLAN describes it in a
	drilling pattern (drillingPosition): type, sub type, up to three
	dimensions, position and angle, and for rows of holes the repeat
	spacing, the end distance and "drill every n-th hole".
	Positions and dimensions are in millimetres.
*/
struct PanelHole
{
	enum Type {Undefined = 0, Drill = 1, Thread = 2, Rectangle = 3, Slot = 4,
			   Hexagon = 5, Octagon = 6, Contour = 7};
	enum SubType {Plain = 0, Square = 1, Chamfered = 2, Rounded = 3};

	Type type = Drill;
	SubType subtype = Plain;
	double d1 = 0;           ///< diameter / thread size / width
	double d2 = 0;           ///< height (rectangle, slot)
	double d3 = 0;           ///< corner radius or chamfer
	double x = 0, y = 0;     ///< centre, mm
	double angle = 0;        ///< degrees
	double repeat = 0;       ///< row spacing, 0 = single hole
	double end = 0;          ///< end distance of a row
	int nth = 0;             ///< drill every n-th hole of a row, 0 = all
	QString contour;         ///< contour name (type Contour)
	QString source;          ///< what made the hole (BMK, rail, cut-out) -- not saved

	QDomElement toXml(QDomDocument &document, const QString &tag = QStringLiteral("hole")) const;
	static PanelHole fromXml(const QDomElement &element);
	QString kindName() const;    ///< e.g. "D5.5", "M5", "R92x92"
};

/**
	@brief A drilling pattern: the holes one mounting variant of a part
	leaves in the surface it is fixed to (EPLAN construction). A variant
	without holes is a part that snaps onto a DIN rail.
*/
struct PanelPattern
{
	QString variant;         ///< e.g. "DIN rail", "Screw mounting"
	bool din_rail = false;   ///< the variant snaps onto a DIN rail: no holes
	QList<PanelHole> holes;

	QDomElement toXml(QDomDocument &document) const;
	static PanelPattern fromXml(const QDomElement &element);
};

/**
	@brief What a shape stands for in a cabinet layout, and the data the
	manufacturing needs from it (sizes in millimetres).

	- Surface: a mounting plate, a door, a wall -- a field which is
	  machined; holds the scale of the drawing (mm per scene unit).
	- DinRail / Duct / CProfile: length-variable parts lying on a surface;
	  their fixing holes come from a row (diameter or thread, spacing,
	  end distance, zero cut side).
	- Cutout: a hole or cut-out set by hand.
	- Keepout: a surface where no hole may be drilled.
*/
struct PanelData
{
	enum Role {None = 0, Surface, DinRail, Duct, CProfile, Cutout, Keepout};

	Role role = None;
	QString designation;     ///< BMK of the surface or part (e.g. +CA-MP1)
	QString article;         ///< order number of the article
	QString manufacturer;

		//Surface
	double mm_per_unit = 0.4; ///< drawing scale: millimetres per scene unit
	double thickness = 3;     ///< sheet thickness, mm
	bool machined = true;     ///< "mechanical processing": holes go into this field
	int origin = 0;           ///< zero point: 0 bottom left, 1 top left, 2 top right, 3 bottom right

		//Length-variable parts
	double profile_depth = 0; ///< height above the surface (rail 7.5, duct e.g. 60)
	PanelHole fixing;         ///< hole of the fixing row (type, d1)
	double spacing = 100;     ///< distance between fixing holes, mm
	double end_distance = 25; ///< first/last hole from the ends, mm
	int rows = 1;             ///< rows of fixing holes across the width (ducts: 1 or 2)
	double row_offset = 0;    ///< distance of the two rows from the centre line, mm
	bool zero_from_end = false; ///< zero cut: rows are measured from the other end

		//Cut-out
	PanelHole cutout;

		//3D: a STEP model shown instead of a box (path, may use %QET_STEP%)
	QString step_file;

	bool isNull() const {return role == None;}
	QDomElement toXml(QDomDocument &document) const;
	static PanelData fromXml(const QDomElement &element);
	static QString roleName(Role role);
	static Role roleFromName(const QString &name);
	static PanelData defaults(Role role);   ///< sensible values for a new part of that role
};

#endif // PANELDATA_H
