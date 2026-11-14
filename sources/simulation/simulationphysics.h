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
#ifndef SIMULATIONPHYSICS_H
#define SIMULATIONPHYSICS_H

#include <algorithm>
#include <cmath>

/**
	The integration steps of the simulation (explicit Euler), shared by
	SimulationModel and the reference checks (QElectroTech --simulate-reference),
	so the checks test exactly the code the simulation runs. SI units, absolute
	gas pressures.
*/
namespace SimPhysics {

	/// isothermal gas volume: V dp/dt + p dV/dt = R T sum(m_flow)
inline double gasNode(double p, double volume, double dvdt, double m_flow, double rt, double dt,
					  double p_min = 1000.0)
{
	return std::max(p_min, p + (rt * m_flow - p * dvdt) / volume * dt);
}

	/// liquid volume with bulk modulus E: (V/E) dp/dt = sum(q) - dV/dt
inline double oilNode(double p, double volume, double dvdt, double q, double bulk, double dt,
					  double p_min)
{
	return std::max(p_min, p + bulk / volume * (q - dvdt) * dt);
}

	/**
		piston with static and sliding friction, viscous damping and hard end
		stops; @a force is the pressure, load and spring force in +x.
		Z needs m, stroke, f_static, f_slide, damping, x, v.
	*/
template <class Z>
void pistonStep(Z &z, double force, double dt)
{
	if (std::abs(z.v) < 1e-4 && std::abs(force) <= z.f_static) {
		z.v = 0.0;
		return;
	}
	const double dir = std::abs(z.v) >= 1e-4 ? z.v : force;
	const double f_net = force - std::copysign(z.f_slide, dir) - z.damping * z.v;
	z.v += f_net / z.m * dt;
	z.x += z.v * dt;
	if (z.x <= 0.0) { z.x = 0.0; z.v = 0.0; }
	else if (z.x >= z.stroke) { z.x = z.stroke; z.v = 0.0; }
}

}

#endif // SIMULATIONPHYSICS_H
