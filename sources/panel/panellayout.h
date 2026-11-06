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
#ifndef PANELLAYOUT_H
#define PANELLAYOUT_H

#include "paneldata.h"

#include <QList>
#include <QPointF>
#include <QRectF>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

class Diagram;
class Element;
class QetShapeItem;

/**
	@brief One length-variable part (DIN rail, duct, C profile) seen from
	its surface: where it lies and how long it is, in millimetres.
*/
struct PanelProfile
{
	QetShapeItem *shape = nullptr;
	PanelData data;
	QPointF start_mm, end_mm;   ///< centre line, surface coordinates
	double length_mm = 0;
	double width_mm = 0;
};

/**
	@brief A part (element) placed on a surface.
*/
struct PanelDevice
{
	Element *element = nullptr;
	QString designation;
	QPointF pos_mm;              ///< insertion point, surface coordinates
	double angle = 0;
	double width_mm = 0, height_mm = 0, depth_mm = 0;
	QRectF rect_mm;              ///< outline on the surface
	QString variant;             ///< mounting variant in use
	bool on_rail = false;        ///< sits on a DIN rail: leaves no holes
	QList<PanelPattern> patterns;
	QString step_file;
	QStringList accessories;     ///< order numbers of the parts mounted with it (BaseUnit, labels)
	QStringList terminals;       ///< a terminal strip: order number of each terminal, left to right
};

/**
	@brief Everything on one mounting surface the manufacturing needs,
	worked out from the drawing: the surface itself, the parts lying on it
	and the holes they leave, in millimetres with the surface's zero point.

	The cabinet layout is drawn on an ordinary folio; what makes it a
	layout is the meaning of its shapes (PanelData) and the drilling
	patterns of the parts. Like EPLAN, a hole belongs to the surface which
	is machined: a duct on a mounting plate leaves holes in the plate, a
	device on a DIN rail leaves none.
*/
class PanelLayout
{
	public:
		explicit PanelLayout(QetShapeItem *surface);

		static QList<QetShapeItem *> surfaces(Diagram *diagram);
		static QList<PanelPattern> patternsOf(Element *element);
			/// where a device dropped near a DIN rail snaps to: its centre on
			/// the rail's centre line, its side against a neighbour or the
			/// rail's start when close; the move in scene units, null when
			/// there is no rail near it
		static QPointF railSnap(Diagram *diagram, Element *element);
			/// a pair of devices that may hang on each other (geraetekombinationen.csv)
		struct DeviceCombination {
			QRegularExpression main, added;
			QString side;          ///< unten, oben, links, rechts: where the added one sits
			double dx_mm = 0, dy_mm = 0;
			QString link;          ///< the part that joins them
		};
		static const QList<DeviceCombination> &deviceCombinations();

		QetShapeItem *surface() const {return m_surface;}
		PanelData surfaceData() const {return m_data;}
		double widthMm() const {return m_width;}
		double heightMm() const {return m_height;}
		QString name() const;

		QPointF toMm(const QPointF &scene) const;
		QPointF toScene(const QPointF &mm) const;
		double mmPerUnit() const {return m_data.mm_per_unit;}

		QList<PanelProfile> profiles() const {return m_profiles;}
		QList<PanelDevice> devices() const {return m_devices;}
		QList<PanelHole> holes() const {return m_holes;}
		QList<QRectF> keepouts() const {return m_keepouts;}
		QStringList warnings() const {return m_warnings;}
			/// the template contour without its round holes, in the
			/// surface's coordinates (zero point and direction as set)
		PanelOutline outline() const {return m_outline;}

			///Cut list: length-variable parts by article with their lengths
		QStringList cutList() const;

	private:
		void collect();
		void addProfileHoles(const PanelProfile &profile);
		void addDeviceHoles(const PanelDevice &device);
		void addHole(PanelHole hole);
		bool insideKeepout(const QPointF &mm) const;

		QetShapeItem *m_surface = nullptr;
		PanelData m_data;
		QRectF m_scene_rect;
		double m_width = 0, m_height = 0;
		QList<PanelProfile> m_profiles;
		QList<PanelDevice> m_devices;
		QList<PanelHole> m_holes;
		QList<QRectF> m_keepouts;      ///< mm
		QStringList m_warnings;
		PanelOutline m_outline;
};

#endif // PANELLAYOUT_H
