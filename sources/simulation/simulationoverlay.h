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
#ifndef SIMULATIONOVERLAY_H
#define SIMULATIONOVERLAY_H

#include <QHash>
#include <QList>
#include <QString>

class Conductor;
class Diagram;
class Element;
class QGraphicsItem;
class SimulationModel;

/**
	@brief The SimulationOverlay class
	Draws the state of a running simulation on the folios without changing
	the project: lines under pressure blue (air) or red (oil), live wires
	red, stroke of the cylinders, position of the valves, relay, counter and
	PLC states. The items are plain graphics items on top of the folio;
	clear() removes them.

	Like FluidSIM the symbols move: the piston and the rod of a cylinder
	travel with the stroke, the fields of a directional valve slide under
	the ports so that the active position stands at the lines (part way
	while the spool switches). The overlay finds the piston, the rod and
	the fields in the symbol's drawing, draws it again with these parts
	moved and shows that picture in place of the element's own
	(Element::setSimulationPicture()); clear() gives the own one back.
*/
class SimulationOverlay
{
	public:
		~SimulationOverlay();
		void update(const SimulationModel &model, const QList<Diagram *> &diagrams);
			/// @a scenes_alive false when the folios are already deleted
		void clear(bool scenes_alive = true);

			/// how a symbol moves, found once from its drawing
		struct Pose;

	private:
		void text(Element *element, const QString &text);
		void pose(Element *element, bool valve, double value, int rest);
		QHash<Element *, Pose *> m_poses;
		QHash<Conductor *, QGraphicsItem *> m_wires;
		QHash<Element *, QGraphicsItem *> m_texts;
		QHash<Element *, QGraphicsItem *> m_marks;     // GRAFCET: frame of an active step
		QHash<Element *, QGraphicsItem *> m_tokens;    // GRAFCET: token of an active step
};

#endif // SIMULATIONOVERLAY_H
