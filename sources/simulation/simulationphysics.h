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
#include <limits>

/**
	The integration steps of the simulation (explicit Euler), shared by
	SimulationModel and the reference checks (QElectroTech --simulate-reference),
	so the checks test exactly the code the simulation runs. SI units, absolute
	gas pressures.
*/
namespace SimPhysics {

constexpr double NOT_A_NUMBER = std::numeric_limits<double>::quiet_NaN();

	/**
		isothermal gas volume: V dp/dt + p dV/dt = R T sum(m_flow) (tools/simulation/FORMELN.md P01).
		A value that is not finite (or a volume <= 0) stays not finite, so the
		caller sees the fault instead of a plausible floor pressure.
	*/
inline double gasNode(double p, double volume, double dvdt, double m_flow, double rt, double dt,
					  double p_min = 1000.0)
{
	const double next = p + (rt * m_flow - p * dvdt) / volume * dt;
	if (!std::isfinite(next) || !(volume > 0.0)) return NOT_A_NUMBER;
	return std::max(p_min, next);
}

	/**
		liquid volume with bulk modulus E: (V/E) dp/dt = sum(q) - dV/dt (FORMELN.md H01, H02).

		Single phase only down to @a p_min (absolute). Oil that would have to
		leave below it is not lost: it becomes the void @a cavity (m³) at
		p_min, and oil flowing in fills that void first before the pressure
		rises again, so the volume balance holds in every step. The caller
		reports a void (the model has no vapour pressure or degassing).
		Not finite input or a volume/bulk modulus <= 0 gives NaN.
	*/
inline double oilNode(double p, double volume, double dvdt, double q, double bulk, double dt,
					  double p_min, double &cavity)
{
	double inflow = (q - dvdt) * dt;
	if (!std::isfinite(inflow) || !std::isfinite(p) || !(volume > 0.0) || !(bulk > 0.0)) return NOT_A_NUMBER;
	double next;
	if (cavity > 0.0) {
		if (inflow <= cavity) {            // the void grows or is partly refilled; the pressure stays
			cavity -= inflow;
			return p_min;
		}
		inflow -= cavity;                  // refilled: the rest compresses the oil from p_min
		cavity = 0.0;
		next = p_min + bulk / volume * inflow;
	} else {
		next = p + bulk / volume * (q - dvdt) * dt;
	}
	if (next >= p_min) return next;
	cavity += (p_min - next) * volume / bulk;
	return p_min;
}

	/**
		watch for a numerically unstable node (roadmap R03): with an explicit step a
		net whose volume is too small for its flows swings from step to step with
		growing or steady amplitude instead of settling. @a update gets the pressure
		change of each step and returns true once the sign has alternated for
		@a limit steps in a row with changes above @a floor that do not die out.
	*/
struct OscillationWatch {
	double last = 0.0;
	int count = 0;
	bool update(double dp, double floor, int limit = 200)
	{
		const bool swing = std::abs(dp) > floor && std::abs(last) > floor && (dp > 0) != (last > 0)
						   && std::abs(dp) >= 0.9 * std::abs(last);
		count = swing ? count + 1 : 0;
		last = dp;
		return count >= limit;
	}
};

	/**
		hydraulic motor with shaft (roadmap R08, FORMELN.md H06): displacement D [m³/rev], setting alpha
		(-1 … 1), leakage cleak [m³/(s·Pa)] from inlet to outlet, inertia J [kg·m²], viscous friction b
		[N·m·s/rad], dry friction tau_c [N·m] (smoothed over 0.1 rad/s), angle phi, speed omega.
		  q   = alpha·D·omega/(2π) + cleak·dp        (dp = p_in - p_out, q from inlet to outlet)
		  tau = alpha·D·dp/(2π)
		  J·domega/dt = tau - tau_load - b·omega - tau_c·tanh(omega/0.1)
	*/
struct Shaft {
	double D = 8e-6, alpha = 1.0, cleak = 0.0, J = 1e-3, b = 1e-3, tau_c = 0.0;
	double omega = 0.0, phi = 0.0;
	double flow(double dp) const { return alpha * D * omega / (2.0 * M_PI) + cleak * dp; }
	double torque(double dp) const { return alpha * D * dp / (2.0 * M_PI); }
	double friction() const { return b * omega + tau_c * std::tanh(omega / 0.1); }
	void step(double dp, double tau_load, double dt)
	{
		omega += (torque(dp) - tau_load - friction()) / J * dt;
		phi += omega * dt;
	}
};

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
							   double p_min, const List &gas, double &cavity)
{
	auto stored = [&](double x) {
		double s = 0.0;
		for (const GasCharge &g : gas) s += accumulatorOil(x, g);
		return s;
	};
	double inflow = (q - dvdt) * dt;
	if (!std::isfinite(inflow) || !std::isfinite(p) || !(volume > 0.0) || !(bulk > 0.0)) return NOT_A_NUMBER;
	for (const GasCharge &g : gas)
		if (!(g.p_pre > 0.0) || !(g.v0 > 0.0) || !(g.n > 0.0)) return NOT_A_NUMBER;
	if (cavity > 0.0) {                    // a void at p_min is refilled first (see oilNode)
		if (inflow <= cavity) {
			cavity -= inflow;
			return p_min;
		}
		inflow -= cavity;
		cavity = 0.0;
		p = p_min;
	}
	const double before = stored(p);
	auto f = [&](double x) { return volume / bulk * (x - p) + stored(x) - before - inflow; };
	const double rigid = p + bulk / volume * inflow;     // no gas: the largest change
	double lo = std::min(p, rigid), hi = std::max(p, rigid);
	if (lo < p_min) {
		const double excess = f(p_min);      // > 0: even at p_min more oil than there is
		if (excess >= 0.0) {
			cavity += excess;
			return p_min;
		}
		lo = p_min;
	}
	if (hi <= lo) return lo;
	// bisection to the last bit, so rounding does not add up over many steps
	for (int i = 0; i < 200; ++i) {
		const double m = 0.5 * (lo + hi);
		if (m <= lo || m >= hi) break;
		(f(m) > 0.0 ? hi : lo) = m;
	}
	return std::abs(f(lo)) <= std::abs(f(hi)) ? lo : hi;
}

	/**
		orifice with the square-root law (FORMELN.md H03), smoothed near equal pressure so it
		is odd, passive and continuously differentiable:
		q = k dp / (dp² + ps²)^(1/4), k = Cd A sqrt(2/rho); for |dp| >> ps q = k sqrt(dp)
	*/
inline double orificeFlow(double dp, double k, double ps)
{
	return k * dp / std::pow(dp * dp + ps * ps, 0.25);
}


	/**
		contact force of an elastic end stop (roadmap R02, FORMELN.md M04): a spring stop_k with
		one-sided damping stop_c beyond 0 and beyond the stroke, as
		  x < 0:      F = -k·x - c·min(v, 0)
		  x > stroke: F = -k·(x - stroke) - c·max(v, 0)
		The damping only acts while the piston moves into the stop: it can only
		take energy out, never push.
	*/
inline double stopForce(double x, double v, double stroke, double k, double c)
{
	if (x < 0.0) return -k * x - c * std::min(v, 0.0);
	if (x > stroke) return -k * (x - stroke) - c * std::max(v, 0.0);
	return 0.0;
}

	/**
		piston with static and sliding friction, viscous damping and end stops;
		@a force is the pressure, load and spring force in +x.
		Z needs m, stroke, f_static, f_slide, damping, stop_k, stop_c, x, v.
		stop_k = 0: hard stops (position and speed held at the end); stop_k > 0:
		elastic stops (stopForce), the piston may enter them and bounce back.
		Semi-implicit Euler (speed first, then position with the new speed).
	*/
template <class Z>
void pistonStep(Z &z, double force, double dt)
{
	const double contact = z.stop_k > 0.0 ? stopForce(z.x, z.v, z.stroke, z.stop_k, z.stop_c) : 0.0;
	const double total = force + contact;
	if (std::abs(z.v) < 1e-4 && std::abs(total) <= z.f_static) {
		z.v = 0.0;
		return;
	}
	const double dir = std::abs(z.v) >= 1e-4 ? z.v : total;
	const double f_net = total - std::copysign(z.f_slide, dir) - z.damping * z.v;
	z.v += f_net / z.m * dt;
	z.x += z.v * dt;
	if (z.stop_k > 0.0) return;
	if (z.x <= 0.0) { z.x = 0.0; z.v = 0.0; }
	else if (z.x >= z.stroke) { z.x = z.stroke; z.v = 0.0; }
}

}

#endif // SIMULATIONPHYSICS_H
