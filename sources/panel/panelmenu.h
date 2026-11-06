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
#ifndef PANELMENU_H
#define PANELMENU_H

#include "paneldata.h"

#include <QMenu>
#include <QPointer>

class Diagram;
class QETDiagramEditor;
class QetShapeItem;
class Element;

/**
	@brief The menu "Cabinet layout" of the diagram editor: the EPLAN
	Pro Panel 2D workflow on an ordinary folio.

	1. Insert a mounting surface (plate, door, side wall) at a scale.
	2. Put DIN rails, ducts and C profiles on it (length-variable parts).
	3. Place the devices of the schematic on it: each becomes a thumbnail
	   of its real size, snapped onto the rails, with the drilling
	   patterns of its part.
	4. Cut-outs and keep-out areas.
	5. Choose the mounting variant per device (DIN rail or screws).
	6. Check the drilling view, the report and the 3D view.
	7. Export the manufacturing data: one DXF per surface, hole list,
	   cut list.
*/
class PanelMenu : public QMenu
{
	Q_OBJECT

	public:
		explicit PanelMenu(QETDiagramEditor *editor);
		void updateActions();

			/// mm per scene unit for a drawing scale 1:@p n
		static double mmPerUnitForScale(double n);
		static double scaleForMmPerUnit(double mm_per_unit);
			/// put a terminal strip of @p count terminals on the first DIN rail
			/// of the folio's mounting surface with room for it: the
			/// manufacturer's drawing of one terminal (by order number) in a
			/// row, else plain terminals of @p width_mm x @p height_mm.
			/// @return a line for the report, empty when nothing was placed
		static QString placeTerminalStrip(Diagram *diagram, const QString &designation, int count,
										  const QString &order, const QString &manufacturer,
										  double width_mm = 5.2, double height_mm = 60);

	private:
		Diagram *diagram() const;
		QetShapeItem *targetSurface(Diagram *d) const;
		QetShapeItem *addShape(Diagram *d, const QRectF &scene_rect, bool ellipse,
							   const PanelData &data, const QString &undo_text);

		void insertEnclosure();
		void insertSurface();
		void insertProfile(PanelData::Role role);
		void insertCutout();
		void insertKeepout();
		void placeDevices();
		void placeTerminalStrips();
		void editSelected();
		void editShape(QetShapeItem *shape);
		void editDevice(Element *element);
		void toggleHoleView(bool on);
		void refreshHoleView();
		void showReport();
		void exportManufacturing();
		void show3D();

		QETDiagramEditor *m_editor = nullptr;
		QList<QAction *> m_edit_actions;
		QAction *m_hole_view = nullptr;
		QList<QPointer<QObject>> m_overlays;
};

#endif // PANELMENU_H
