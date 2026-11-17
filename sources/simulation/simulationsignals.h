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
#ifndef SIMULATIONSIGNALS_H
#define SIMULATIONSIGNALS_H

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

/**
	Analog signal blocks of the simulation (roadmap R06, tools/simulation/FORMELN.md A01–A10).

	Contract: every block is continuous in time and is advanced with the integration step of the
	model (explicit, no hidden sampling); voltages are potentials against the 0 V net. The cyclic
	PLC/GRAFCET program does not see these values (it samples only digital inputs every 10 ms).
	Nothing here knows QElectroTech: the model hands in the input voltages and takes the outputs.
*/
namespace SimSignals {

constexpr double TWO_PI = 6.283185307179586;

	/// A01: linear characteristic, input @a v in [v0, v1] onto [u0, u1]; outside the range the end value
	/// (FluidSIM: only values inside the given range count)
inline double scale(double v, double v0, double v1, double u0, double u1)
{
	if (v1 == v0) return u0;
	const double r = std::clamp((v - v0) / (v1 - v0), 0.0, 1.0);
	return u0 + r * (u1 - u0);
}

	/// A02: function generator (FluidSIM waveForm 1 square, 2 sine, 3 triangle, 4 constant, 5 profile)
struct Waveform {
	int form = 2;
	double frequency = 1.0, amplitude = 5.0, offset = 0.0, vmin = -30.0, vmax = 30.0;
	std::vector<std::pair<double, double>> profile;   // (t [s], u [V]), rising t
	bool repeat = false;

	double profileAt(double t) const {
		if (profile.empty()) return 0.0;
		const double end = profile.back().first;
		if (repeat && end > 0) t = std::fmod(t, end);
		if (t <= profile.front().first) return profile.front().second;
		for (size_t i = 1; i < profile.size(); ++i)
			if (t <= profile[i].first) {
				const auto &a = profile[i - 1], &b = profile[i];
				return b.first > a.first ? a.second + (b.second - a.second) * (t - a.first) / (b.first - a.first) : b.second;
			}
		return profile.back().second;
	}
	double value(double t) const {
		const double phase = frequency * t - std::floor(frequency * t);
		double y = offset;
		switch (form) {
			case 1: y += phase < 0.5 ? amplitude : -amplitude; break;
			case 2: y += amplitude * std::sin(TWO_PI * frequency * t); break;
			case 3: y += amplitude * (phase < 0.25 ? 4 * phase : phase < 0.75 ? 2 - 4 * phase : 4 * phase - 4); break;
			case 5: y = profileAt(t); break;
			default: break;     // 4 constant: the offset
		}
		return std::clamp(y, vmin, vmax);
	}
};

	/// A03: setpoint card, up to 8 setpoints W1 … W8 and 4 ramps R1 … R4 [s/V] (0 = jump);
	/// mode 1 wait the switching time, 2 next setpoint once reached, 3 external choice by I1 … I3
struct SetpointCard {
	int mode = 1, count = 1;
	double w[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	double ramp[4] = {0, 0, 0, 0};
	double t_switch = 1.0;
	int active = 0;
	double y = 0.0, since = 0.0;
	bool started = false;

	double step(double time, double dt, int bits) {
		if (!started) { started = true; since = time; active = 0; y = 0.0; }
		const int n = std::clamp(count, 1, 8);
		if (mode == 3) active = std::clamp(bits, 0, 7);
		else if (mode == 1 && n > 1 && time - since >= t_switch - 1e-12) { active = (active + 1) % n; since = time; }
		const double target = w[active];
		if (target != y) {
			// R1 rising from 0 V upwards, R2 falling down to 0 V, R3 falling below 0 V, R4 rising up to 0 V
			const bool up = target > y;
			const double r = up ? (y >= 0 ? ramp[0] : ramp[3]) : (y > 0 ? ramp[1] : ramp[2]);
			if (r <= 0) y = target;
			else {
				double next = y + (up ? dt : -dt) / r;
				if ((y < 0 && next > 0) || (y > 0 && next < 0)) next = 0.0;     // the ramp changes at 0 V
				y = up ? std::min(next, target) : std::max(next, target);
			}
		}
		if (mode == 2 && y == target && n > 1) { active = (active + 1) % n; since = time; }
		return y;
	}
	void reset() { started = false; y = 0.0; }
};

	/**
		A04: PID controller in the stated variant of the roadmap (GLEICHUNGEN_UND_MODELLE.md ch. 10),
		parallel form (FluidSIM gives KP, KI [1/s], KD [s]): derivative of the measurement through a
		first order filter tau_d, back-calculation anti-windup with t_track. Without KI there is no
		integrator: the output is only limited.
	*/
struct Pid {
	double kp = 1.0, ki = 0.0, kd = 0.0, offset = 0.0, umin = -10.0, umax = 10.0;
	double tau_d = 1e-3, t_track = 0.1;
	double i = 0.0, z = 0.0, e = 0.0, raw = 0.0;
	bool started = false;

	double step(double r, double y, double dt) {
		if (!started) { started = true; z = y; i = 0.0; }
		e = r - y;
		const double dfilt = (y - z) / tau_d;
		z = y + (z - y) * std::exp(-dt / tau_d);
		raw = kp * e + i - kd * dfilt + offset;
		const double u = std::clamp(raw, umin, umax);
		if (ki != 0.0) i += dt * (ki * e + (u - raw) / t_track);
		return u;
	}
	void reset() { started = false; i = 0.0; e = 0.0; raw = 0.0; }
};

	/// A05: state controller: deviation, velocity and acceleration of the measurement (both differentiated
	/// through a first order filter tau_f), u = gain·(kx·(w - x) - kv·v - ka·a) + offset
struct StateController {
	double kx = 1.0, kv = 0.0, ka = 0.0, gain = 1.0, offset = 0.0, umin = -10.0, umax = 10.0, tau_f = 1e-3;
	double x_f = 0.0, v = 0.0, v_f = 0.0, a = 0.0, e = 0.0;
	bool started = false;

	double step(double w, double x, double dt) {
		if (!started) { started = true; x_f = x; v = 0.0; v_f = 0.0; a = 0.0; }
		const double k = std::exp(-dt / tau_f);
		v = (x - x_f) / tau_f;
		x_f = x + (x_f - x) * k;
		a = (v - v_f) / tau_f;
		v_f = v + (v_f - v) * k;
		e = w - x;
		return std::clamp(gain * (kx * e - kv * v - ka * a) + offset, umin, umax);
	}
	void reset() { started = false; e = 0.0; }
};

	/// A06: second order lag T²·y'' + 2·d·T·y' + y = K·u (semi-implicit Euler, sub-steps of at most T/20)
struct Pt2 {
	double t = 0.01, k = 1.0, d = 1.0;
	double y = 0.0, yd = 0.0;

	double step(double u, double dt) {
		const int n = std::max(1, int(std::ceil(dt / (0.05 * t))));
		const double h = dt / n;
		for (int s = 0; s < n; ++s) {
			yd += h * (k * u - y - 2 * d * t * yd) / (t * t);
			y += h * yd;
		}
		return y;
	}
};

	/// A07: integrator y' = ki·u, limited to [-limit, limit] (the stored value stops at the limit: no windup)
struct Integrator {
	double ki = 100.0, y = 0.0;
	double step(double u, double dt, double limit) {
		y = std::clamp(y + dt * ki * u, -limit, limit);
		return y;
	}
};

	/**
		A08: output range of the TP 1013 blocks from the potentials at A1 and A2: both at least @a vmin →
		± efficiency·min(|A1|, |A2|); only one connected → that potential is passed through (@a pass);
		else no output (limit 0)
	*/
struct SupplyRange { double limit = 0.0; bool pass = false; double pass_value = 0.0; };
inline SupplyRange supplyRange(bool a1_connected, double a1, bool a2_connected, double a2, double efficiency, double vmin)
{
	SupplyRange r;
	if (a1_connected && a2_connected) {
		if (std::abs(a1) >= vmin && std::abs(a2) >= vmin) r.limit = efficiency * std::min(std::abs(a1), std::abs(a2));
	} else if (a1_connected || a2_connected) {
		r.pass = true;
		r.pass_value = a1_connected ? a1 : a2;
	}
	return r;
}

	/// A09: comparator with hysteresis, on at vset + hyst/2, off below vset - hyst/2
inline bool comparator(bool on, double u, double vset, double hyst)
{
	if (!on && u >= vset + hyst / 2) return true;
	if (on && u < vset - hyst / 2) return false;
	return on;
}

	/**
		A10: proportional amplifier: the setpoint voltage as the valve sees it, with the jump current
		(share of the maximum current at the smallest positive input, against the overlap of the valve);
		@a two_channel: -10 … 10 V (A positive, B negative), else 0 … 10 V
	*/
inline double amplifier(double u, double jump, bool two_channel)
{
	const double m = std::min(std::abs(u), 10.0);
	if (m <= 0.0 || (!two_channel && u < 0)) return 0.0;
	const double out = 10.0 * (jump + (1.0 - jump) * m / 10.0);
	return u < 0 ? -out : out;
}

} // namespace SimSignals

#endif // SIMULATIONSIGNALS_H
