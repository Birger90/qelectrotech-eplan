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
*/
class SimulationOverlay
{
	public:
		~SimulationOverlay();
		void update(const SimulationModel &model, const QList<Diagram *> &diagrams);
			/// @a scenes_alive false when the folios are already deleted
		void clear(bool scenes_alive = true);

	private:
		void text(Element *element, const QString &text);
		QHash<Conductor *, QGraphicsItem *> m_wires;
		QHash<Element *, QGraphicsItem *> m_texts;
};

#endif // SIMULATIONOVERLAY_H
