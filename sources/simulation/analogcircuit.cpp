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
#include "analogcircuit.h"

#include <QSet>

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <utility>

namespace {
// thermal voltage kT/q at 20 °C (293.15 K, the reference temperature of the simulation)
constexpr double V_T = 1.380649e-23 * 293.15 / 1.602176634e-19;
constexpr double G_MIN = 1e-12;     // S from every node to ground and across every diode (keeps the matrix regular)
}

int AnalogCircuit::add(const Part &p)
{
	m_parts << p;
	m_built = false;
	return static_cast<int>(m_parts.size()) - 1;
}

void AnalogCircuit::setClosed(int index, bool closed)
{
	if (index >= 0 && index < m_parts.size()) m_parts[index].closed = closed;
}

void AnalogCircuit::setSource(int index, double value)
{
	if (index >= 0 && index < m_parts.size()) m_parts[index].value = value;
}

bool AnalogCircuit::reset(const QString &label)
{
	bool any = false;
	for (Part &p : m_parts)
		if (p.kind == Kind::Protection && p.label == label && p.tripped) {
			p.tripped = false;
			p.w = 0.0;
			any = true;
		}
	return any;
}

QStringList AnalogCircuit::takeTrips()
{
	QStringList out;
	out.swap(m_trips);
	return out;
}

QString AnalogCircuit::check() const
{
	bool ground = false;
	for (const Part &p : m_parts) {
		ground = ground || p.kind == Kind::Ground;
		if ((p.kind == Kind::Resistor || p.kind == Kind::Capacitor || p.kind == Kind::Inductor) && !(p.value > 0))
			return QStringLiteral("%1: Wert muss größer als 0 sein").arg(p.label);
		if (p.kind == Kind::Coil && (!(p.r > 0) || !(p.value >= 0)))
			return QStringLiteral("%1: Spule braucht R > 0 und L ≥ 0 (sim_coil)").arg(p.label);
		if (p.kind == Kind::Switch && (!(p.ron > 0) || !(p.roff > p.ron)))
			return QStringLiteral("%1: Kontaktwiderstände ungültig (Ron > 0, Roff > Ron)").arg(p.label);
		if (p.kind == Kind::Transistor && (!(p.is > 0) || !(p.n > 0) || !(p.bf > 0) || !(p.br > 0)))
			return QStringLiteral("%1: Transistor braucht Is, n, BF und BR größer als 0").arg(p.label);
		if (p.kind == Kind::OpAmp && (!(p.gain > 0) || !(p.rin > 0) || !(p.rout > 0) || !(p.vmax > p.vmin)))
			return QStringLiteral("%1: Operationsverstärker braucht A, Rin, Rout > 0 und Vmax > Vmin").arg(p.label);
		if ((p.kind == Kind::Transistor || p.kind == Kind::OpAmp) && (p.a < 0 || p.b < 0 || p.c < 0))
			return QStringLiteral("%1: alle drei Anschlüsse müssen angeschlossen sein").arg(p.label);
		if (p.kind == Kind::Protection && (!(p.inom > 0) || !(p.r > 0) || !(p.tau > 0) || !(p.wth > 0) || !(p.im >= 0)))
			return QStringLiteral("%1: Schutzorgan braucht In, R, Zeitkonstante und Auslösegrenze größer als 0").arg(p.label);
		if (p.kind == Kind::DcMotor && (!(p.r > 0) || !(p.la >= 0) || !(p.km > 0) || !(p.jm > 0) || !(p.bm >= 0)))
			return QStringLiteral("%1: Gleichstrommotor braucht Ra, k, J > 0 und La, b ≥ 0").arg(p.label);
		if (p.kind == Kind::InductionMotor && (!(p.rs > 0) || !(p.rr > 0) || !(p.lls > 0) || !(p.llr > 0) || !(p.lm > 0)
											   || !(p.poles >= 1) || !(p.jm > 0) || !(p.bm >= 0)))
			return QStringLiteral("%1: Asynchronmotor braucht Rs, Rr, Lls, Llr, Lm, p ≥ 1, J > 0").arg(p.label);
		if (p.kind == Kind::InductionMotor && (p.a < 0 || p.b < 0 || p.c < 0))
			return QStringLiteral("%1: alle drei Außenleiter (L1, L2, L3) müssen angeschlossen sein").arg(p.label);
		if (p.kind == Kind::Diode && (!(p.is > 0) || !(p.n > 0)))
			return QStringLiteral("%1: Is und n müssen größer als 0 sein").arg(p.label);
		if (p.kind != Kind::Ground && p.a < 0 && p.b < 0)
			return QStringLiteral("%1: nicht angeschlossen").arg(p.label);
	}
	if (!m_parts.isEmpty() && !ground) return QStringLiteral("Stromkreis ohne Masse (Bezugspotential)");
	return QString();
}

void AnalogCircuit::build()
{
	m_index.clear();
	QSet<int> grounds;
	for (const Part &p : m_parts)
		if (p.kind == Kind::Ground) {
			if (p.a >= 0) grounds << p.a;
			if (p.b >= 0) grounds << p.b;
		}
	int n = 0;
	for (const Part &p : m_parts) {
		if (p.kind == Kind::Ground) continue;
		for (int net : {p.a, p.b, p.c})
			if (net >= 0 && !grounds.contains(net) && !m_index.contains(net)) m_index.insert(net, n++);
	}
	for (int g : grounds) m_index.insert(g, -1);
	int sources = 0;
	for (const Part &p : m_parts) sources += p.kind == Kind::VoltageSource ? 1 : 0;
	m_x = QVector<double>(n + sources, 0.0);
	m_built = true;
}

AnalogCircuit::Asm AnalogCircuit::asmStamp(const Part &p, double dt) const
{
	// E13: space vector model in the stator frame (amplitude invariant Clarke transform), trapezoidal rule with the
	// speed of the last step (implicit Euler would damp the 50 Hz rotation of the rotor flux by ω²·Δt/2, which acts
	// like a larger rotor resistance). Rotor: dψr/dt = −Rr/Lr·ψr + Rr·Lm/Lr·is + p·ω·J·ψr; stator: us = Rs·is + dψs/dt,
	// ψs = σ·Ls·is + Lm/Lr·ψr. Result: phase currents I = G·v + j (into the motor from the nets L1, L2, L3)
	Asm s{};
	const double ls = p.lls + p.lm, lr = p.llr + p.lm, kr = p.lm / lr, sig_ls = ls - p.lm * p.lm / lr;
	const double a = 1.0 + dt * p.rr / lr / 2, b = 1.0 - dt * p.rr / lr / 2, w = dt * p.poles * p.omega / 2, det = a * a + w * w;
	const double mi[2][2] = {{a / det, -w / det}, {w / det, a / det}};   // inverse of [[a, w], [−w, a]]
	const double cq = dt * p.rr * p.lm / lr / 2;
	const double r0[2] = {b * p.psi[2] - w * p.psi[3] + cq * p.psi[4], b * p.psi[3] + w * p.psi[2] + cq * p.psi[5]};
	for (int r = 0; r < 2; ++r) {
		s.P[r] = mi[r][0] * r0[0] + mi[r][1] * r0[1];
		for (int k = 0; k < 2; ++k) s.Q[r][k] = cq * mi[r][k];
	}
	double Z[2][2];
	for (int r = 0; r < 2; ++r)
		for (int k = 0; k < 2; ++k) Z[r][k] = (r == k ? p.rs + 2 * sig_ls / dt : 0.0) + 2 * kr * s.Q[r][k] / dt;
	const double h[2] = {2 * (kr * s.P[0] - p.psi[0]) / dt - p.psi[6] + p.rs * p.psi[4],
						 2 * (kr * s.P[1] - p.psi[1]) / dt - p.psi[7] + p.rs * p.psi[5]};
	const double dz = Z[0][0] * Z[1][1] - Z[0][1] * Z[1][0];
	s.Y[0][0] = Z[1][1] / dz; s.Y[0][1] = -Z[0][1] / dz; s.Y[1][0] = -Z[1][0] / dz; s.Y[1][1] = Z[0][0] / dz;
	for (int r = 0; r < 2; ++r) s.yh[r] = s.Y[r][0] * h[0] + s.Y[r][1] * h[1];
	static const double T[2][3] = {{2.0 / 3, -1.0 / 3, -1.0 / 3}, {0.0, 1.0 / std::sqrt(3.0), -1.0 / std::sqrt(3.0)}};
	static const double Cm[3][2] = {{1.0, 0.0}, {-0.5, std::sqrt(3.0) / 2}, {-0.5, -std::sqrt(3.0) / 2}};
	for (int r = 0; r < 3; ++r) {
		for (int k = 0; k < 3; ++k) {
			double g = 0.0;
			for (int m = 0; m < 2; ++m)
				for (int n = 0; n < 2; ++n) g += Cm[r][m] * s.Y[m][n] * T[n][k];
			s.G[r][k] = g;
		}
		s.j[r] = -(Cm[r][0] * s.yh[0] + Cm[r][1] * s.yh[1]);
	}
	return s;
}

double AnalogCircuit::sourceValue(const Part &p, double t) const
{
	if (p.freq > 0) return p.offset + p.value * std::sin(2.0 * M_PI * p.freq * t + p.phase * M_PI / 180.0);
	return p.value + p.offset;
}

double AnalogCircuit::nodeVoltage(int net) const
{
	const int k = m_index.value(net, -2);
	if (k == -1) return 0.0;
	if (k < 0 || k >= m_x.size()) return std::nan("");
	return m_x.at(k);
}

const AnalogCircuit::Part *AnalogCircuit::part(const QString &label) const
{
	for (const Part &p : m_parts)
		if (p.label == label) return &p;
	return nullptr;
}

bool AnalogCircuit::step(double t, double dt)
{
	if (!m_built) build();
	int nodes = 0;
	for (auto it = m_index.constBegin(); it != m_index.constEnd(); ++it) nodes = std::max(nodes, it.value() + 1);
	const int size = static_cast<int>(m_x.size());
	auto idx = [&](int net) { return net < 0 ? -1 : m_index.value(net, -1); };
	auto volt = [&](const QVector<double> &x, int net) { const int k = idx(net); return k < 0 ? 0.0 : x.at(k); };
	QVector<double> x = m_x;
	for (int iter = 0; iter < 100; ++iter) {
		QVector<double> A(size * size, 0.0), rhs(size, 0.0);
		auto at = [&](int r, int c) -> double & { return A[r * size + c]; };
		auto conductance = [&](int a, int b, double g) {
			const int ia = idx(a), ib = idx(b);
			if (ia >= 0) at(ia, ia) += g;
			if (ib >= 0) at(ib, ib) += g;
			if (ia >= 0 && ib >= 0) { at(ia, ib) -= g; at(ib, ia) -= g; }
		};
		auto inject = [&](int a, int b, double j) {   // current j pushed into node a, drawn from node b
			const int ia = idx(a), ib = idx(b);
			if (ia >= 0) rhs[ia] += j;
			if (ib >= 0) rhs[ib] -= j;
		};
		// a nonlinear device: current j0 leaves @a node into the device at the present iterate, its derivatives
		// dJ/dv per net; stamped as the Newton linearisation J ≈ j0 + Σ g·(v − v0)
		auto device = [&](int node, double j0, std::initializer_list<std::pair<int, double>> d) {
			const int in = idx(node);
			if (in < 0) return;
			double lin = j0;
			for (const auto &nd : d) {
				const int k = idx(nd.first);
				if (k >= 0) at(in, k) += nd.second;
				lin -= nd.second * volt(x, nd.first);
			}
			rhs[in] -= lin;
		};
		for (int k = 0; k < nodes; ++k) at(k, k) += G_MIN;
		int vs = nodes;
		bool nonlinear = false;
		for (const Part &p : m_parts) {
			switch (p.kind) {
				case Kind::Resistor: conductance(p.a, p.b, 1.0 / p.value); break;
				case Kind::Capacitor: {     // i = C/dt·(v − v_prev)
					const double g = p.value / dt;
					conductance(p.a, p.b, g);
					inject(p.a, p.b, g * p.v);
					break;
				}
				case Kind::Inductor: {      // i = dt/L·v + i_prev
					const double g = dt / p.value;
					conductance(p.a, p.b, g);
					inject(p.a, p.b, -p.i);
					break;
				}
				case Kind::VoltageSource: { // v_a − v_b = E; the source current a → b inside the source is an unknown
					const int ia = idx(p.a), ib = idx(p.b);
					if (ia >= 0) { at(ia, vs) += 1; at(vs, ia) += 1; }
					if (ib >= 0) { at(ib, vs) -= 1; at(vs, ib) -= 1; }
					rhs[vs] = sourceValue(p, t);
					++vs;
					break;
				}
				case Kind::CurrentSource: inject(p.a, p.b, sourceValue(p, t)); break;
				case Kind::Switch: conductance(p.a, p.b, 1.0 / (p.closed ? p.ron : p.roff)); break;
				case Kind::Protection: conductance(p.a, p.b, 1.0 / (p.tripped ? p.roff : p.r)); break;
				case Kind::DcMotor: {       // E12: Ra, La and the back EMF k·ω with the shaft solved in the same step
					const double d = 1.0 + p.bm * dt / p.jm, beta = p.km * p.km * dt / (p.jm * d);
					const double e0 = p.km * (p.omega - dt * p.ml / p.jm) / d, g = 1.0 / (p.r + p.la / dt + beta);
					conductance(p.a, p.b, g);
					inject(p.a, p.b, -g * (p.la / dt * p.i - e0));
					break;
				}
				case Kind::InductionMotor: {    // E13: linear 3 × 3 admittance of the phases for this step
					const Asm s = asmStamp(p, dt);
					const int nets[3] = {p.a, p.b, p.c};
					for (int r = 0; r < 3; ++r) {
						const int in = idx(nets[r]);
						if (in < 0) continue;
						for (int k = 0; k < 3; ++k) {
							const int col = idx(nets[k]);
							if (col >= 0) at(in, col) += s.G[r][k];
						}
						rhs[in] -= s.j[r];
					}
					break;
				}
				case Kind::Coil: {          // R and L in series: v = R·i + L/dt·(i − i_prev)
					const double g = 1.0 / (p.r + p.value / dt);
					conductance(p.a, p.b, g);
					inject(p.a, p.b, -g * p.value / dt * p.i);
					break;
				}
				case Kind::Diode: {         // Shockley, linearised at the present iterate
					nonlinear = true;
					const double nv = p.n * V_T;
					const double v0 = std::min(volt(x, p.a) - volt(x, p.b), 40.0 * nv);
					const double e = std::exp(v0 / nv);
					const double g = p.is / nv * e + G_MIN;
					const double i0 = p.is * (e - 1.0);
					conductance(p.a, p.b, g);
					inject(p.a, p.b, -(i0 - g * v0));
					break;
				}
				case Kind::Transistor: {    // Ebers-Moll transport model (E09), NPN or PNP by sign
					nonlinear = true;
					const double s = p.sign, nv = p.n * V_T;
					const double vb = volt(x, p.b), vc = volt(x, p.a), ve = volt(x, p.c);
					const double ef = std::exp(std::min(s * (vb - ve), 40.0 * nv) / nv);
					const double er = std::exp(std::min(s * (vb - vc), 40.0 * nv) / nv);
					const double i_f = p.is * (ef - 1.0), i_r = p.is * (er - 1.0);
					const double gf = p.is / nv * ef + G_MIN, gr = p.is / nv * er + G_MIN, k = 1.0 + 1.0 / p.br;
					const double jc = s * (i_f - k * i_r), jb = s * (i_f / p.bf + i_r / p.br);
					device(p.a, jc, {{p.b, gf - k * gr}, {p.c, -gf}, {p.a, k * gr}});
					device(p.b, jb, {{p.b, gf / p.bf + gr / p.br}, {p.c, -gf / p.bf}, {p.a, -gr / p.br}});
					device(p.c, -jc - jb, {{p.b, -(gf - k * gr) - (gf / p.bf + gr / p.br)}, {p.c, gf + gf / p.bf},
										   {p.a, -k * gr + gr / p.br}});
					break;
				}
				case Kind::OpAmp: {         // open-loop gain A with tanh output limit, Rin, Rout (E10)
					nonlinear = true;
					conductance(p.a, p.b, 1.0 / p.rin);
					const double vh = (p.vmax - p.vmin) / 2, vm = (p.vmax + p.vmin) / 2;
					const double th = std::tanh(p.gain * (volt(x, p.a) - volt(x, p.b)) / vh);
					const double e = vm + vh * th, g = p.gain * (1.0 - th * th);
					device(p.c, (volt(x, p.c) - e) / p.rout, {{p.c, 1.0 / p.rout}, {p.a, -g / p.rout}, {p.b, g / p.rout}});
					break;
				}
				case Kind::Ground: break;
			}
		}
		// Gaussian elimination with partial pivoting
		QVector<double> y = rhs;
		for (int c = 0; c < size; ++c) {
			int piv = c;
			for (int r = c + 1; r < size; ++r)
				if (std::abs(at(r, c)) > std::abs(at(piv, c))) piv = r;
			if (std::abs(at(piv, c)) < 1e-30) {
				m_failure = QStringLiteral("Stromkreis nicht lösbar (Spannungsquellen in Masche oder offener Knoten)");
				return false;
			}
			if (piv != c) {
				for (int k = 0; k < size; ++k) std::swap(at(c, k), at(piv, k));
				std::swap(y[c], y[piv]);
			}
			for (int r = c + 1; r < size; ++r) {
				const double f = at(r, c) / at(c, c);
				if (f == 0.0) continue;
				for (int k = c; k < size; ++k) at(r, k) -= f * at(c, k);
				y[r] -= f * y[c];
			}
		}
		QVector<double> nx(size, 0.0);
		for (int r = size - 1; r >= 0; --r) {
			double s = y[r];
			for (int k = r + 1; k < size; ++k) s -= at(r, k) * nx[k];
			nx[r] = s / at(r, r);
		}
		// diode voltage limiting (SPICE pnjlim idea): large node steps grow only logarithmically
		double change = 0.0;
		for (int k = 0; k < nodes; ++k) change = std::max(change, std::abs(nx[k] - x[k]));
		if (nonlinear)
			for (int k = 0; k < nodes; ++k) {
				const double d = nx[k] - x[k];
				if (std::abs(d) > 0.5) nx[k] = x[k] + (d > 0 ? 1.0 : -1.0) * (0.5 + std::log1p(std::abs(d) - 0.5));
			}
		x = nx;
		if (!nonlinear || change < 1e-9) break;
		if (iter == 99) {
			m_failure = QStringLiteral("Diodenstromkreis konvergiert nicht (Newton)");
			return false;
		}
	}
	m_x = x;
	// voltages and currents of the parts (current a → b through the part)
	int vs = nodes;
	for (Part &p : m_parts) {
		const double v = volt(x, p.a) - volt(x, p.b);
		if (p.kind == Kind::Transistor) {
			const double s = p.sign, nv = p.n * V_T;
			const double vb = volt(x, p.b), vc = volt(x, p.a), ve = volt(x, p.c);
			const double i_f = p.is * (std::exp(std::min(s * (vb - ve), 40.0 * nv) / nv) - 1.0);
			const double i_r = p.is * (std::exp(std::min(s * (vb - vc), 40.0 * nv) / nv) - 1.0);
			p.v = vc - ve;
			p.i = s * (i_f - (1.0 + 1.0 / p.br) * i_r);
			p.i2 = s * (i_f / p.bf + i_r / p.br);
			continue;
		}
		if (p.kind == Kind::OpAmp) {
			const double vh = (p.vmax - p.vmin) / 2, vm = (p.vmax + p.vmin) / 2;
			const double e = vm + vh * std::tanh(p.gain * (volt(x, p.a) - volt(x, p.b)) / vh);
			p.v = volt(x, p.c);
			p.i = (e - p.v) / p.rout;
			p.i2 = (volt(x, p.a) - volt(x, p.b)) / p.rin;
			continue;
		}
		switch (p.kind) {
			case Kind::Resistor: p.i = v / p.value; break;
			case Kind::Capacitor: p.i = p.value / dt * (v - p.v); break;
			case Kind::Inductor: p.i = dt / p.value * v + p.i; break;
			case Kind::VoltageSource: p.i = x.at(vs++); break;
			case Kind::CurrentSource: p.i = -sourceValue(p, t); break;
			case Kind::Switch: p.i = v / (p.closed ? p.ron : p.roff); break;
			case Kind::DcMotor: {
				const double d = 1.0 + p.bm * dt / p.jm, beta = p.km * p.km * dt / (p.jm * d);
				const double e0 = p.km * (p.omega - dt * p.ml / p.jm) / d, g = 1.0 / (p.r + p.la / dt + beta);
				p.i = g * v + g * (p.la / dt * p.i - e0);
				p.i2 = p.km * p.i;
				p.omega = (p.omega + dt / p.jm * (p.i2 - p.ml)) / d;
				break;
			}
			case Kind::InductionMotor: {
				const Asm s = asmStamp(p, dt);
				const double va = volt(x, p.a), vb = volt(x, p.b), vc = volt(x, p.c);
				const double ua = (2 * va - vb - vc) / 3, ub = (vb - vc) / std::sqrt(3.0);
				const double ia = s.Y[0][0] * ua + s.Y[0][1] * ub - s.yh[0], ib = s.Y[1][0] * ua + s.Y[1][1] * ub - s.yh[1];
				const double lr = p.llr + p.lm, ls = p.lls + p.lm, kr = p.lm / lr, sig_ls = ls - p.lm * p.lm / lr;
				p.psi[2] = s.P[0] + s.Q[0][0] * ia + s.Q[0][1] * ib;
				p.psi[3] = s.P[1] + s.Q[1][0] * ia + s.Q[1][1] * ib;
				p.psi[0] = sig_ls * ia + kr * p.psi[2];
				p.psi[1] = sig_ls * ib + kr * p.psi[3];
				p.psi[4] = ia; p.psi[5] = ib; p.psi[6] = ua; p.psi[7] = ub;
				p.i = ia;                   // current of L1
				p.v = va - vb;              // line voltage L1–L2
				p.i2 = 1.5 * p.poles * (p.psi[0] * ib - p.psi[1] * ia);
				const double d = 1.0 + p.bm * dt / p.jm;
				p.omega = (p.omega + dt / p.jm * (p.i2 - p.ml)) / d;
				continue;
			}
			case Kind::Protection: {
				// E11: thermal state dW/dt = i² − W/τ, solved exactly for the current of this step; trips at W ≥ W_th
				// (thermal) or |i| ≥ Im·In (magnetic, im > 0); open from the next step on
				p.i = v / (p.tripped ? p.roff : p.r);
				if (p.tripped) break;
				const double e = std::exp(-dt / p.tau);
				p.w = p.w * e + p.i * p.i * p.tau * (1.0 - e);
				const bool magnetic = p.im > 0 && std::abs(p.i) >= p.im * p.inom;
				if (magnetic || p.w >= p.wth) {
					p.tripped = true;
					m_trips << QStringLiteral("%1 %2 löst aus (%3, %4 A)").arg(p.trip_kind, p.label,
						magnetic ? QStringLiteral("magnetisch") : QStringLiteral("thermisch")).arg(std::abs(p.i), 0, 'f', 3);
				}
				break;
			}
			case Kind::Coil: { const double g = 1.0 / (p.r + p.value / dt); p.i = g * v + g * p.value / dt * p.i; break; }
			case Kind::Diode: p.i = p.is * (std::exp(std::min(v, 40.0 * p.n * V_T) / (p.n * V_T)) - 1.0); break;
			case Kind::Ground: p.i = 0.0; break;
		}
		p.v = v;
	}
	return true;
}
