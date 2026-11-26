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
#include <memory>

#include "analogcircuit.h"
#include "simulationphysics.h"

class Diagram;
class Element;
class Terminal;
class PlcProgram;
class Grafcet;

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
		struct CylinderState { Element *element; QString label; double x_mm; double stroke_mm; double v; bool rotary = false; };
			/// position 1-based; rest 1-based (initial position); spool: where the spool stands, 0-based
			/// and fractional while it switches (0.4: from position 1 on the way to position 2)
		struct ValveState { Element *element; QString label; int position; int positions; int rest = 1; double spool = 0; };
			/// a measurable quantity (roadmap R01): stable id, SI unit, reference and sign
		struct Signal {
			QString id;          ///< e.g. "p:-QM1.4", "q:-QM1.4", "x:-MM1"
			QString quantity;    ///< pressure, mass_flow, volume_flow, position, velocity, force, state
			QString unit;        ///< SI: Pa, kg/s, m3/s, m, m/s, N, 1
			QString reference;   ///< gauge (to 101325 Pa), absolute, into_part, extending …
			QString part;        ///< label of the part, else its name
			QString port;        ///< port name, empty for the part as a whole
		};

		explicit SimulationModel(const QList<Diagram *> &diagrams, double supply_bar = 6.0);
		~SimulationModel();

		void step(double dt);
		void runUntil(double time, double dt = 1e-5);
			/// R16: runs until the next state change (a new message: valve, end position, relay, step …) or until
			/// @a limit; true when a change happened
		bool runUntilChange(double limit, double dt = 1e-5);
			/// R16: integration steps so far (performance figures)
		long long stepCount() const { return m_step_count; }
			/// R16: number of fluid nets (problem size for the performance figures)
		int netCount() const { return static_cast<int>(m_p.size()); }
		double time() const { return m_time; }

		void pressButton(const QString &label, bool pressed);
			/// R15: repairs every fault of the part (FluidSIM repair mode); false: no fault there
		bool repair(const QString &label);
		bool actuateValve(const QString &label, bool left, bool on);
			/// setpoint of a proportional valve in volts (-10 … 10, R07); false when there is none with @a label
		bool setSetpoint(const QString &label, double volts);

		QStringList buttons() const;
		QStringList valves() const;
		QList<CylinderState> cylinders() const;
		QList<ValveState> valveStates() const;
		QList<Message> takeMessages();
		QString summary() const;
			/// short state text per element for the display (stroke, position, relay, counter, PLC)
		QList<QPair<Element *, QString>> elementStates() const;
			/// active steps of the GRAFCET charts
		QList<Element *> activeSteps() const;
			/// errors of the PLC programs, empty when they compiled
		QStringList programErrors() const { return m_program_errors; }
			/// errors of the plan data (units, cylinder, accumulator, pump values); the run does not start
		QStringList modelErrors() const { return m_model_errors; }
			/// true once the run stopped on invalid data or a computing fault (not finite value)
		bool failed() const { return m_failed; }
		QString failure() const { return m_failure; }
			/// void (m³) torn off in the oil net of @a terminal, 0 when filled
		double cavity(Terminal *terminal) const;
			/// current value of a PLC variable (address, symbol or flag)
		double plcValue(const QString &name) const;
			/// all PLC variables with their current value (inputs, outputs, flags, block outputs)
		QHash<QString, double> plcVariables() const { return m_plc_vars; }

			/// gauge pressure (bar) at a fluid terminal; NaN when not fluid
		double pressureBar(Terminal *terminal) const;
			/// true when the net of an electric terminal carries the voltage
		bool energized(Terminal *terminal) const;
		bool isFluid(Terminal *terminal) const;
		bool isElectric(Terminal *terminal) const;
		bool isPressed(const QString &label) const { return m_pressed.contains(label); }

			/// every quantity this model can measure (ids stable for the same project)
		QList<Signal> signalList() const;
			/// the current value of @a id in its SI unit; @a ok false for an unknown id (no invented 0)
		double signalValue(const QString &id, bool *ok = nullptr) const;
			/// record the directed flow at each part port (needed for the q: signals); off by default
		void setRecordFlows(bool on) { m_record_flows = on; }

	private:
		struct Valve;
		struct Cylinder;
		struct Pump;
		struct Motor;
		struct FlowValve;
		struct Contact;
		struct Relay;
		struct Coil;
		struct GrafcetIo;
		struct Counter;
		struct PlcChannel;
		struct Plc;
		struct Accumulator;
		struct PressureSwitch;
		struct Ejector;
		struct Pipe;
		struct PropValve;
		struct SuctionCup;
		struct Analog;
		struct LsUnit;
		struct Valve63;
		struct Steering;
		struct Fault;
		enum class FaultKind { Stuck, CoilOpen, Leak, Clogged, CheckClosed, CheckOpen, Never, Always };
		void buildFaults(const QList<Diagram *> &diagrams);
		const Fault *fault(const Element *e, FaultKind kind) const;
		struct Joystick;
		struct LsBlock;
		struct PriorityValve;
		struct Intensifier;

		int node(Terminal *t) const;
		int node(Element *e, const QString &port) const;
		void buildNets(const QList<Diagram *> &diagrams);
		void buildComponents(const QList<Diagram *> &diagrams, double supply_bar);
		void electrics();
		void analog(double dt);
		void linkAnalog(const QList<Diagram *> &diagrams);
		void coupleCircuit();                      // R17 stage 2: contacts, coils in the analog circuit
			/// potential of an electric net [V]: an analog output, else the supply it is connected to
		double volt(int net) const;
		void fail(const QString &why);
		void multiStep(Cylinder *z, double dt, QVector<double> &vol, QVector<double> &dvdt);
		QString netName(int net) const;
		bool contactClosed(const Contact &c) const;

		QHash<Terminal *, int> m_node_of;
		QVector<double> m_p;
		QSet<int> m_hydraulic;
		QSet<int> m_electric;
		QHash<int, double> m_fixed;
		QList<Valve *> m_valves;
		QList<Cylinder *> m_cylinders;
		QList<Pump *> m_pumps;
		QList<Motor *> m_motors;
		QList<Ejector *> m_ejectors;               // vacuum generators (R10)
		QList<Pipe *> m_pipes;                     // lines with geometry (R05)
		QList<PropValve *> m_prop_valves;          // proportional valves (R07)
		QList<Intensifier *> m_intensifiers;      // R12 hydraulic pressure intensifiers
		QList<LsUnit *> m_ls_units;                // R13 load sensing power units
		QList<Valve63 *> m_valves63;               // R13 6/3 hand lever valves
		QList<Steering *> m_steerings;             // R13 steering units (Orbitrol)
		QList<Fault *> m_faults;                   // R15 defects (sim_fault)
		QList<Joystick *> m_joysticks;             // R13 pilot valve joysticks
		QList<LsBlock *> m_ls_blocks;              // R13 load sensing control blocks
		QList<PriorityValve *> m_priority_valves;  // R13 priority valves LS
		QList<SuctionCup *> m_cups;                // suction cups (R10)
		QList<Analog *> m_analog;                  // analog signal blocks in their order of evaluation (R06)
		QHash<int, double> m_volt;                 // nets driven by an analog output [V]
		QHash<int, double> m_source_volt;          // supply terminals with a given potential [V]
		QHash<int, double> m_net_volt;             // potential of the electric nets from the supplies [V]
		QList<FlowValve *> m_flow_valves;
		QList<Contact *> m_contacts;
		QList<Relay *> m_relays;
		QList<Coil *> m_coils;
		QList<GrafcetIo *> m_grafcet_io;
		QSet<QString> m_latched;                   // selector switches switched on
		QList<Counter *> m_counters;
		QList<PlcChannel *> m_plc_channels;
		QList<Plc *> m_plcs;
		QList<Accumulator *> m_accumulators;
		QList<PressureSwitch *> m_pressure_switches;
		QHash<int, double> m_extra_volume;          // compressed air receivers (m³ per net)
		QList<QPair<int, int>> m_vessel_links;      // ports of one receiver
		double m_vessel_c = 0.0;
		QHash<QString, double> m_plc_vars;
		std::unique_ptr<Grafcet> m_grafcet;
		AnalogCircuit m_circuit;                   // R17 stage 1: analog electric circuit (sim_role circuit)
		struct CircuitIo { int part = -1; QStringList names; double umin = 0.0, umax = 0.0, raw = 0.0; bool output = false; };
		QList<CircuitIo> m_circuit_io;             // R17 stage 2: PLC analog inputs (AI) and outputs (AQ) of the circuit
		QStringList m_program_errors;
		QStringList m_model_errors;
		bool m_failed = false;
		QString m_failure;
		QHash<int, double> m_cavity;                // oil nets: void volume at the lowest pressure (m³)
		QSet<int> m_cavity_open;
		QList<int> m_silenced;                      // free ports with a silencer
		double m_silencer_c = 0.0;
		double m_next_plc_cycle = 0.0;
		long long m_step_count = 0;
		QList<QPair<QList<int>, QList<int>>> m_sources;
		QSet<int> m_plus_nets;
		QSet<QString> m_pressed;
		QHash<QString, QString> m_end_position;   // cylinder label -> "vorne"/"hinten"/""
		QHash<int, QString> m_fixed_by;            // which part holds a net at a fixed pressure (R03)
		QVector<SimPhysics::OscillationWatch> m_watch;   // numerical stability per net (R03)
		bool m_record_flows = false;
		QHash<QPair<Element *, int>, double> m_port_flow;   // flow into the part at its port's net (last step)
		QHash<Element *, double> m_cylinder_force;           // fluid force on the piston (last step)
		QList<Message> m_messages;
		double m_time = 0.0;
};

#endif // SIMULATIONMODEL_H
