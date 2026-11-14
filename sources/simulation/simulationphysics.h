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

	/// gas side of a hydraulic accumulator: precharge @a p_pre (absolute) at gas volume @a v0, exponent @a n
struct GasCharge { double p_pre, v0, n; };

	/// oil in the accumulator at absolute pressure @a p (gas law; empty below the precharge)
inline double accumulatorOil(double p, const GasCharge &g)
{
	return p <= g.p_pre ? 0.0 : g.v0 - g.v0 * std::pow(g.p_pre / p, 1.0 / g.n);
}

	/**
		liquid volume with accumulators on it, solved implicitly from the volume
		balance  V/E (p_new - p) + oil(p_new) - oil(p) = (sum(q) - dV/dt) dt,
		so the oil is conserved for any step, also when the pressure crosses a
		precharge (where the explicit form would overshoot)
	*/
template <class List>
double oilNodeWithAccumulators(double p, double volume, double dvdt, double q, double bulk, double dt,
							   double p_min, const List &gas)
{
	auto stored = [&](double x) {
		double s = 0.0;
		for (const GasCharge &g : gas) s += accumulatorOil(x, g);
		return s;
	};
	const double inflow = (q - dvdt) * dt;
	const double before = stored(p);
	auto f = [&](double x) { return volume / bulk * (x - p) + stored(x) - before - inflow; };
	const double rigid = p + bulk / volume * inflow;     // no gas: the largest change
	double lo = std::min(p, rigid), hi = std::max(p, rigid);
	lo = std::max(lo, p_min);
	if (hi <= lo || f(lo) >= 0.0) return std::max(p_min, lo);
	// bisection to the last bit, so rounding does not add up over many steps
	for (int i = 0; i < 200; ++i) {
		const double m = 0.5 * (lo + hi);
		if (m <= lo || m >= hi) break;
		(f(m) > 0.0 ? hi : lo) = m;
	}
	return std::abs(f(lo)) <= std::abs(f(hi)) ? lo : hi;
}

	/**
		orifice with the square-root law, smoothed near equal pressure so it
		is odd, passive and continuously differentiable:
		q = k dp / (dp² + ps²)^(1/4), k = Cd A sqrt(2/rho); for |dp| >> ps q = k sqrt(dp)
	*/
inline double orificeFlow(double dp, double k, double ps)
{
	return k * dp / std::pow(dp * dp + ps * ps, 0.25);
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
