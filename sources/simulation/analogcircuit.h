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
#ifndef ANALOGCIRCUIT_H
#define ANALOGCIRCUIT_H

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>

/**
	@brief The AnalogCircuit class
	R17 stage 1 (FORMELN.md E01–E05): an electric circuit by Kirchhoff's laws, modified nodal analysis with the
	implicit Euler method: resistors, capacitors and inductors (companion models), DC and sine voltage and current
	sources, diodes (Shockley, Newton iteration with a limited voltage step) and ground. Nodes are the electric nets
	of the simulation; the reference is every net with a ground part.
	R17 stage 3 (E09, E10): bipolar transistors (Ebers-Moll transport model) and operational amplifiers (open-loop
	gain with smooth output limit, input and output resistance).
	R17 stage 2 (E06–E08): switches (a contact of the switching logic, R_on or R_off) and coils (R and L in series;
	whether the relay or valve solenoid picks up is decided from the coil current in SimulationModel).
*/
class AnalogCircuit
{
	public:
		enum class Kind { Resistor, Capacitor, Inductor, VoltageSource, CurrentSource, Diode, Ground, Switch, Coil, Transistor, OpAmp, Protection, DcMotor, InductionMotor };
		struct Part {
			Kind kind = Kind::Resistor;
			QString label;
			int a = -1, b = -1;                 // nets (a: first terminal, plus side of a source, anode)
			double value = 0.0;                 // R [Ohm], C [F], L [H], source amplitude [V or A]
			double offset = 0.0, freq = 0.0, phase = 0.0;   // source: DC offset, frequency [Hz], phase [deg]
			double is = 1e-12, n = 1.0;         // diode saturation current [A], emission coefficient
			double r = 0.0;                     // coil: winding resistance [Ohm] (value: inductance [H], may be 0)
			double ron = 0.01, roff = 1e9;      // switch: closed and open resistance [Ohm] (project definition E06)
			bool closed = false;                // switch state, set by the switching logic before each step
			double v = 0.0, i = 0.0;            // voltage a − b and current a → b of the last step
			// R17 stage 3 (E09, E10): transistor a = collector, b = base, c = emitter (v = U_CE, i = I_C, i2 = I_B);
			// op-amp a = +, b = −, c = output (v = output voltage to ground, i = output current into the net)
			int c = -1;
			double sign = 1.0;                  // transistor: +1 NPN, −1 PNP
			double bf = 100.0, br = 1.0;        // transistor: forward and reverse current gain (Ebers-Moll)
			double gain = 1e5, rin = 1e6, rout = 1.0, vmax = 15.0, vmin = -15.0;   // op-amp
			double i2 = 0.0;
			// R17 stage 4 (E11): fuse or circuit breaker between a and b; closed resistance r, open roff
			double inom = 1.0, kf = 1.6, tau = 1.0, im = 0.0;   // rated current, thermal factor, time constant, magnetic multiple
			double wth = 0.0, w = 0.0;          // thermal limit and state [A²s]
			bool tripped = false;
			QString trip_kind;                  // "Sicherung", "Leitungsschutzschalter" … for the message
			// R17 stage 5 (E12, E13): DC motor a/b = armature (r = Ra, la = La, km = k), induction motor a/b/c = L1/L2/L3
			// (star, no neutral); shaft J, viscous b, constant load torque ml; omega [rad/s], torque i2 [N·m]
			double la = 0.0, km = 0.0, jm = 0.0, bm = 0.0, ml = 0.0, omega = 0.0;
			double rs = 0.0, rr = 0.0, lls = 0.0, llr = 0.0, lm = 0.0, poles = 1.0;
			double psi[8] = {0, 0, 0, 0, 0, 0, 0, 0};   // ψs α, β, ψr α, β, is α, β, us α, β of the last step
		};

		int add(const Part &p);             // index of the part
		void setClosed(int index, bool closed);
		void setSource(int index, double value);
		bool reset(const QString &label);   // a tripped protective device closes again (new fuse, breaker switched on)
		QStringList takeTrips();            // messages of devices that tripped since the last call
		bool isEmpty() const { return m_parts.isEmpty(); }
		int partCount() const { return static_cast<int>(m_parts.size()); }
		QString check() const;              // empty or the reason why the circuit cannot be solved
			/// one implicit Euler step to time @a t; false when the system is singular or Newton fails
		bool step(double t, double dt);
		double nodeVoltage(int net) const;
		const Part *part(const QString &label) const;
		const QList<Part> &parts() const { return m_parts; }
		QString failure() const { return m_failure; }

	private:
		double sourceValue(const Part &p, double t) const;
		QList<Part> m_parts;
		QHash<int, int> m_index;            // net -> unknown (−1: ground)
		QVector<double> m_x;                // node voltages, then source currents
		bool m_built = false;
		QString m_failure;
		QStringList m_trips;
		void build();
		struct Asm { double G[3][3]; double j[3]; double Y[2][2]; double yh[2]; double P[2]; double Q[2][2]; };
		Asm asmStamp(const Part &p, double dt) const;
};

#endif // ANALOGCIRCUIT_H
