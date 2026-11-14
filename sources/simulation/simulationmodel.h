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
#ifndef SIMULATIONMODEL_H
#define SIMULATIONMODEL_H

#include <QHash>
#include <QList>
#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

class Diagram;
class Element;
class Terminal;

/**
	@brief The SimulationModel class
	Simulates the pneumatic, hydraulic and electric parts of a project from
	the fluid data of its elements (positions, flow paths, actuation and
	characteristic values) and the way they are wired:

	- every net of connected terminals is a node; pneumatic nodes hold air
	  (isothermal ideal gas), hydraulic nodes oil (bulk modulus), an open
	  port vents to the atmosphere;
	- directional valves join nodes with the paths of their current
	  position, flow after ISO 6358 from the nominal flow qNN (air) or from
	  the resistance rT (oil), switching delays, spring return;
	- check, throttle, one-way flow control, shut-off, quick exhaust,
	  pressure relief and pressure regulating valves, silencers;
	- air source, hydraulic pump with built-in relief, tank;
	- cylinders: force balance with mass, static and sliding friction,
	  spring and end stops;
	- electrics: voltage source, push buttons, limit switches (function
	  text "-MM1 vorne"/"-MM1 hinten"), relays with linked contacts, timer
	  relays (function text "1 s"), valve solenoids (label of the valve,
	  "/14" left, "/12" right).

	Headless: the GUI dock and the command line both drive it.
*/
class SimulationModel
{
	public:
		struct Message { double time; QString text; };
		struct CylinderState { Element *element; QString label; double x_mm; double stroke_mm; double v; };
		struct ValveState { Element *element; QString label; int position; int positions; };

		explicit SimulationModel(const QList<Diagram *> &diagrams, double supply_bar = 6.0);
		~SimulationModel();

		void step(double dt);
		void runUntil(double time, double dt = 1e-5);
		double time() const { return m_time; }

		void pressButton(const QString &label, bool pressed);
		bool actuateValve(const QString &label, bool left, bool on);

		QStringList buttons() const;
		QStringList valves() const;
		QList<CylinderState> cylinders() const;
		QList<ValveState> valveStates() const;
		QList<Message> takeMessages();
		QString summary() const;

			/// gauge pressure (bar) at a fluid terminal; NaN when not fluid
		double pressureBar(Terminal *terminal) const;
			/// true when the net of an electric terminal carries the voltage
		bool energized(Terminal *terminal) const;
		bool isFluid(Terminal *terminal) const;
		bool isElectric(Terminal *terminal) const;
		bool isPressed(const QString &label) const { return m_pressed.contains(label); }

	private:
		struct Valve;
		struct Cylinder;
		struct Pump;
		struct FlowValve;
		struct Contact;
		struct Relay;
		struct Coil;

		int node(Terminal *t) const;
		int node(Element *e, const QString &port) const;
		void buildNets(const QList<Diagram *> &diagrams);
		void buildComponents(const QList<Diagram *> &diagrams, double supply_bar);
		void electrics();
		bool contactClosed(const Contact &c) const;

		QHash<Terminal *, int> m_node_of;
		QVector<double> m_p;
		QSet<int> m_hydraulic;
		QSet<int> m_electric;
		QHash<int, double> m_fixed;
		QList<Valve *> m_valves;
		QList<Cylinder *> m_cylinders;
		QList<Pump *> m_pumps;
		QList<FlowValve *> m_flow_valves;
		QList<Contact *> m_contacts;
		QList<Relay *> m_relays;
		QList<Coil *> m_coils;
		QList<QPair<QList<int>, QList<int>>> m_sources;
		QSet<int> m_plus_nets;
		QSet<QString> m_pressed;
		QHash<QString, QString> m_end_position;   // cylinder label -> "vorne"/"hinten"/""
		QList<Message> m_messages;
		double m_time = 0.0;
};

#endif // SIMULATIONMODEL_H
