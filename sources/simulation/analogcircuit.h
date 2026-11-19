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
#include <QVector>

/**
	@brief The AnalogCircuit class
	R17 stage 1 (FORMELN.md E01–E05): an electric circuit by Kirchhoff's laws, modified nodal analysis with the
	implicit Euler method: resistors, capacitors and inductors (companion models), DC and sine voltage and current
	sources, diodes (Shockley, Newton iteration with a limited voltage step) and ground. Nodes are the electric nets
	of the simulation; the reference is every net with a ground part. The circuit stands beside the switching logic:
	in stage 1 it does not drive relays, coils or the PLC.
*/
class AnalogCircuit
{
	public:
		enum class Kind { Resistor, Capacitor, Inductor, VoltageSource, CurrentSource, Diode, Ground };
		struct Part {
			Kind kind = Kind::Resistor;
			QString label;
			int a = -1, b = -1;                 // nets (a: first terminal, plus side of a source, anode)
			double value = 0.0;                 // R [Ohm], C [F], L [H], source amplitude [V or A]
			double offset = 0.0, freq = 0.0, phase = 0.0;   // source: DC offset, frequency [Hz], phase [deg]
			double is = 1e-12, n = 1.0;         // diode saturation current [A], emission coefficient
			double v = 0.0, i = 0.0;            // voltage a − b and current a → b of the last step
		};

		void add(const Part &p);
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
		void build();
};

#endif // ANALOGCIRCUIT_H
