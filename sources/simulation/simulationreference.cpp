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
#include "simulationreference.h"

#include "grafcet.h"
#include "plcprogram.h"
#include "simulationphysics.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QHash>
#include <QStringList>
#include <QVector>

#include <cmath>
#include <functional>
#include <limits>

/*
	Reference cases with an analytical solution or a reference run
	(OpenModelica 1.27.1, DASSL, tolerance 1e-9), same parameters as the
	reference models: isothermal gas with R = 287.05 J/(kg K), T = 300 K,
	linear valve conductance m = G (pa - pb). They run the integration steps of
	SimPhysics, which SimulationModel::step() uses.
*/
namespace {

constexpr double RT = 287.05 * 300.0;
constexpr double DP_SMOOTH = 0.3e5;     // smoothing pressure of the oil orifices in SimulationModel

struct Check {
	QString name;
	bool passed = false;
	QString detail;
};

	/// largest error of an integrated curve against @a exact; samples every 1 ms
double maxError(double dt, double t_end, double p0,
				const std::function<double(double, double)> &advance,
				const std::function<double(double)> &exact)
{
	const long long n = std::llround(t_end / dt);
	const long long every = std::max(1LL, std::llround(0.001 / dt));
	double p = p0, worst = 0.0;
	for (long long i = 0; i <= n; ++i) {
		const double t = i * dt;
		if (i % every == 0) worst = std::max(worst, std::abs(p - exact(t)));
		if (i < n) p = advance(p, dt);
	}
	return worst;
}

	/// analytical case at dt and dt/2: error bound and first-order convergence
Check analytical(const QString &name, double p0, double limit, bool exact_for_euler,
				 const std::function<double(double, double)> &advance,
				 const std::function<double(double)> &exact)
{
	const double e1 = maxError(1e-5, 1.0, p0, advance, exact);
	const double e2 = maxError(5e-6, 1.0, p0, advance, exact);
	Check c{name};
	const double ratio = e2 > 0 ? e1 / e2 : std::numeric_limits<double>::infinity();
	if (exact_for_euler) {
		c.passed = e1 <= limit && e2 <= limit;
		c.detail = QStringLiteral("max. Fehler %1 Pa (dt 10 µs), %2 Pa (dt 5 µs); Grenze %3 Pa (Euler ist hier exakt)")
					   .arg(e1, 0, 'g', 3).arg(e2, 0, 'g', 3).arg(limit);
	} else {
		c.passed = e1 <= limit && ratio > 1.8 && ratio < 2.2;
		c.detail = QStringLiteral("max. Fehler %1 Pa (dt 10 µs), %2 Pa (dt 5 µs), Verhältnis %3 (1. Ordnung: 2); Grenze %4 Pa")
					   .arg(e1, 0, 'f', 3).arg(e2, 0, 'f', 3).arg(ratio, 0, 'f', 2).arg(limit);
	}
	return c;
}

struct Piston {
	double m = 0.5, stroke = 0.06, f_static = 5.0, f_slide = 5.0, damping = 20.0;
	double x = 0.0, v = 0.0;
};

struct Sample { double t, x, pa, pb; };

	/// CylinderCycle: two 3/2 valves, supply 6 bar abs, switch at 0.4 s; samples every 1 ms
QVector<Sample> cylinderCycle(double dt)
{
	const double ps = 6e5, pe = 1e5, pamb = 1e5, g = 2e-9, area = 4e-4, dead = 5e-6;
	Piston z;
	double pa = 1e5, pb = 1e5;
	QVector<Sample> out;
	const long long n = std::llround(1.0 / dt), every = std::llround(0.001 / dt);
	for (long long i = 0; i <= n; ++i) {
		const double t = i * dt;
		if (i % every == 0) out << Sample{t, z.x, pa, pb};
		if (i == n) break;
		const bool left = t < 0.4;
		const double ma = g * ((left ? ps : pe) - pa);
		const double mb = g * ((left ? pe : ps) - pb);
		SimPhysics::pistonStep(z, (pa - pamb) * area - (pb - pamb) * area, dt);
		pa = SimPhysics::gasNode(pa, dead + area * z.x, area * z.v, ma, RT, dt);
		pb = SimPhysics::gasNode(pb, dead + area * (z.stroke - z.x), -area * z.v, mb, RT, dt);
	}
	return out;
}

double firstTime(const QVector<Sample> &s, double after, const std::function<bool(const Sample &)> &cond)
{
	for (const Sample &x : s)
		if (x.t >= after && cond(x)) return x.t;
	return std::numeric_limits<double>::quiet_NaN();
}

	/// reads time and the named columns of an OpenModelica result file (rows with repeated times kept once)
bool readResult(const QString &file, const QStringList &columns, QVector<QVector<double>> *rows)
{
	QFile f(file);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
	QStringList head = QString::fromUtf8(f.readLine()).trimmed().split(QLatin1Char(','));
	for (QString &h : head) h.remove(QLatin1Char('"'));
	QVector<int> idx;
	for (const QString &c : QStringList{QStringLiteral("time")} + columns) {
		idx << static_cast<int>(head.indexOf(c));
		if (idx.last() < 0) return false;
	}
	double last_t = -1.0;
	while (!f.atEnd()) {
		const QStringList cells = QString::fromUtf8(f.readLine()).trimmed().split(QLatin1Char(','));
		QVector<double> row;
		for (int i : idx) row << cells.value(i).toDouble();
		if (row.first() == last_t) { rows->last() = row; continue; }   // keep the value after an event
		last_t = row.first();
		*rows << row;
	}
	return !rows->isEmpty();
}

}

int runSimulationReference(const QString &reference_dir_given, QTextStream &out)
{
	// without a folder: the reference runs installed beside the program
	QString reference_dir = reference_dir_given;
	if (reference_dir.isEmpty()) {
		const QDir app(QCoreApplication::applicationDirPath());
		for (const QString &rel : {QStringLiteral("simulation/referenz"), QStringLiteral("../simulation/referenz"),
								   QStringLiteral("../Resources/simulation/referenz"),
								   QStringLiteral("../share/qelectrotech/simulation/referenz")})
			if (QFileInfo::exists(app.filePath(rel + QStringLiteral("/OwnSim.Examples.CylinderCycle_res.csv")))) {
				reference_dir = QDir::cleanPath(app.filePath(rel));
				break;
			}
	}
	if (!reference_dir.isEmpty()) out << "Referenzläufe: " << QDir::toNativeSeparators(reference_dir) << "\n";
	QList<Check> checks;

	// GasFill: constant mass flow into a fixed volume, p = p0 + R T m t / V
	checks << analytical(QStringLiteral("GasFill (Luft, konstanter Zustrom)"), 1e5, 0.05, true,
		[](double p, double dt) { return SimPhysics::gasNode(p, 0.001, 0.0, 1e-5, RT, dt); },
		[](double t) { return 1e5 + RT * 1e-5 * t / 0.001; });

	// GasCharge: 2 bar abs through a resistance, tau = 0.2 s
	const double g = 0.001 / (RT * 0.2);
	checks << analytical(QStringLiteral("GasCharge (Luft über Widerstand)"), 1e5, 5.0, false,
		[g](double p, double dt) { return SimPhysics::gasNode(p, 0.001, 0.0, g * (2e5 - p), RT, dt); },
		[](double t) { return 2e5 - 1e5 * std::exp(-t / 0.2); });

	// HydraulicCharge: 10 bar through R = 1e12 Pa s/m³ into 1 l with E = 1e9 Pa, tau = 1 s
	checks << analytical(QStringLiteral("HydraulicCharge (Öl über Widerstand)"), 0.0, 5.0, false,
		[](double p, double dt) {
			return SimPhysics::oilNode(p, 0.001, 0.0, (1e6 - p) / 1e12, 1e9, dt,
									   -std::numeric_limits<double>::infinity());
		},
		[](double t) { return 1e6 * (1.0 - std::exp(-t)); });

	// PeriodicCounter: the PLC counts a pulse every 0.1 s; 9 at 0.95 s
	{
		PlcProgram plc;
		QString error;
		Check c{QStringLiteral("PeriodicCounter (SPS-Zähler CTU, Zyklus 10 ms)")};
		if (!plc.compile(QStringLiteral("VAR Z : CTU; N : INT := 0; END_VAR\n"
										"Z(CU := TAKT, R := FALSE, PV := 1000);\nN := Z.CV;\n"), &error)) {
			c.detail = error;
		} else {
			QHash<QString, double> vars;
			QStringList late;
			double last = 0.0;
			for (int i = 0; i <= 95; ++i) {
				const double t = i * 0.010;
				vars.insert(QStringLiteral("TAKT"), (i % 10 == 0 && i > 0) ? 1.0 : 0.0);
				plc.cycle(t, vars);
				const double n = vars.value(QStringLiteral("N"));
				if (n != last) {
					if (std::abs(t - n * 0.1) > 1e-9) late << QString::number(t, 'f', 2);
					last = n;
				}
			}
			const double n = vars.value(QStringLiteral("N"));
			c.passed = n == 9.0 && late.isEmpty();
			c.detail = QStringLiteral("Zählerstand %1 bei 0,95 s (soll 9)%2").arg(n)
						   .arg(late.isEmpty() ? QString() : QStringLiteral(", zu spät bei ") + late.join(QStringLiteral(", ")));
		}
		checks << c;
	}

	// PLC language: every statement and block against hand-computed values
	{
		Check c{QStringLiteral("SPS-Sprachumfang (ELSIF, CASE, MOD, SR, RS, F_TRIG, TOF, CTD, Fehlerzeile)")};
		PlcProgram plc;
		QString error;
		const bool compiled = plc.compile(QStringLiteral(
			"VAR K : INT := 7; S : SR; R : RS; F : F_TRIG; T : TOF; Z : CTD; END_VAR\n"
			"IF K < 5 THEN A := 1; ELSIF K MOD 4 = 3 THEN A := 2; ELSE A := 3; END_IF;\n"
			"CASE K OF 1, 2: B := 10; 3..8: B := 20; ELSE B := 30; END_CASE;\n"
			"CASE K + 10 OF 1: C := 1; ELSE C := -1; END_CASE;\n"
			"K := K / 2;\n"
			"S(S1 := X, R := TRUE); R(S := X, R1 := TRUE);\n"
			"F(CLK := X); T(IN := X, PT := T#50ms); Z(CD := X, LD := L, PV := 2);\n"
			"QS := S.Q1; QR := R.Q1; QF := F.Q; QT := T.Q; QZ := Z.Q;\n"), &error);
		QStringList wrong;
		if (!compiled) {
			wrong << error;
		} else {
			QHash<QString, double> v;
			auto expect = [&](const QString &name, double want, const QString &when) {
				if (std::abs(v.value(name) - want) > 1e-9)
					wrong << QStringLiteral("%1 = %2 statt %3 (%4)").arg(name).arg(v.value(name)).arg(want).arg(when);
			};
			v.insert(QStringLiteral("X"), 1.0); v.insert(QStringLiteral("L"), 1.0);
			plc.cycle(0.00, v);           // K = 7
			expect(QStringLiteral("A"), 2, QStringLiteral("ELSIF"));
			expect(QStringLiteral("B"), 20, QStringLiteral("CASE 3..8"));
			expect(QStringLiteral("C"), -1, QStringLiteral("CASE ELSE"));
			expect(QStringLiteral("K"), 3, QStringLiteral("INT 7/2"));
			expect(QStringLiteral("QS"), 1, QStringLiteral("SR: Setzen vorrangig"));
			expect(QStringLiteral("QR"), 0, QStringLiteral("RS: Rücksetzen vorrangig"));
			expect(QStringLiteral("QF"), 0, QStringLiteral("F_TRIG ohne fallende Flanke"));
			expect(QStringLiteral("QT"), 1, QStringLiteral("TOF bei IN"));
			v.insert(QStringLiteral("L"), 0.0);
			plc.cycle(0.01, v);           // K = 3: IF K < 5
			expect(QStringLiteral("A"), 1, QStringLiteral("IF"));
			v.insert(QStringLiteral("X"), 0.0);
			plc.cycle(0.02, v);           // falling edge, TOF starts, CTD counts on the next rising edge
			expect(QStringLiteral("QF"), 1, QStringLiteral("F_TRIG fallende Flanke"));
			expect(QStringLiteral("QT"), 1, QStringLiteral("TOF läuft nach"));
			plc.cycle(0.08, v);
			expect(QStringLiteral("QT"), 0, QStringLiteral("TOF nach 50 ms"));
			v.insert(QStringLiteral("X"), 1.0); plc.cycle(0.09, v);
			v.insert(QStringLiteral("X"), 0.0); plc.cycle(0.10, v);
			v.insert(QStringLiteral("X"), 1.0); plc.cycle(0.11, v);
			expect(QStringLiteral("QZ"), 1, QStringLiteral("CTD 2 Flanken"));
			expect(QStringLiteral("Z.CV"), 0, QStringLiteral("CTD Zählwert"));
		}
		PlcProgram bad;
		QString bad_error;
		if (bad.compile(QStringLiteral("A := 1;\nIF A THEN\n  B := ;\nEND_IF;\n"), &bad_error)
			|| !bad_error.startsWith(QStringLiteral("Zeile 3")))
			wrong << QStringLiteral("Fehler nicht in Zeile 3 gemeldet: „%1“").arg(bad_error);
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? QStringLiteral("alle 14 Erwartungen erfüllt, Syntaxfehler: „%1“").arg(bad_error)
								   : wrong.join(QStringLiteral("; "));
		checks << c;
	}

	// ---- hydraulics (OwnHydraulics reference models, gauge pressure, SI)
	const double kOrifice = 0.65 * 3e-6 * std::sqrt(2.0 / 850.0);   // Cd A sqrt(2/rho)

	// PumpFill: 1e-5 m³/s into 1 l with E = 1e9 Pa: p = 1e7 Pa/s · t
	checks << analytical(QStringLiteral("PumpFill (Pumpe in Ölvolumen)"), 0.0, 0.1, true,
		[](double p, double dt) {
			return SimPhysics::oilNode(p, 1e-3, 0.0, 1e-5, 1e9, dt, -std::numeric_limits<double>::infinity());
		},
		[](double t) { return 1e7 * t; });

	// Orifice: square-root law, both directions, zero at equal pressure, passive and odd
	{
		Check c{QStringLiteral("Orifice (Blende vorwärts, rückwärts, gleicher Druck, Passivität)")};
		const double fwd = SimPhysics::orificeFlow(1e6, kOrifice, 1000.0);
		const double rev = SimPhysics::orificeFlow(-1e6, kOrifice, 1000.0);
		const double zero = SimPhysics::orificeFlow(0.0, kOrifice, 1000.0);
		const double ref = 9.458887011696121e-05;       // OpenModelica, OrificeForward
		bool passive = true, odd = true;
		for (int i = -200; i <= 200; ++i) {
			const double dp = i * 5000.0;
			const double q = SimPhysics::orificeFlow(dp, kOrifice, 1000.0);
			passive = passive && dp * q >= 0.0;
			odd = odd && std::abs(q + SimPhysics::orificeFlow(-dp, kOrifice, 1000.0)) < 1e-18;
		}
		c.passed = std::abs(fwd - ref) < 1e-12 && std::abs(rev + ref) < 1e-12 && zero == 0.0 && passive && odd;
		c.detail = QStringLiteral("q(10 bar) = %1 m³/s (Referenz %2), rückwärts %3, bei Gleichdruck %4; "
								  "Verlustleistung nie negativ: %5, ungerade: %6")
					   .arg(fwd, 0, 'g', 10).arg(ref, 0, 'g', 10).arg(rev, 0, 'g', 10).arg(zero)
					   .arg(passive ? QStringLiteral("ja") : QStringLiteral("nein"))
					   .arg(odd ? QStringLiteral("ja") : QStringLiteral("nein"));
		checks << c;
	}

	// AccumulatorCharge: 1e-5 m³/s into an accumulator, 1 l gas at 10 bar abs, n = 1.4:
	// p_abs = 1e6 (0.001 / (0.001 - 1e-5 t))^1.4
	{
		auto run = [](double dt) {
			double p = 1e6, worst = 0.0;
			const long long n = std::llround(1.0 / dt), every = std::llround(0.001 / dt);
			for (long long i = 0; i <= n; ++i) {
				const double t = i * dt;
				if (i % every == 0)
					worst = std::max(worst, std::abs(p - 1e6 * std::pow(0.001 / (0.001 - 1e-5 * t), 1.4)));
				if (i < n)
					p = SimPhysics::oilNodeWithAccumulators(p, 1e-12, 0.0, 1e-5, 1e9, dt, 0.0,
						QVector<SimPhysics::GasCharge>{{1e6, 1e-3, 1.4}});
			}
			return worst;
		};
		const double e1 = run(1e-5), e2 = run(5e-6);
		// filling from 1 bar abs through the precharge (5.013 bar abs): 1.933e-4 m³/s, 1 l gas, oil 1e-4 m³ with E = 7e8
		auto fill = [](double dt) {
			const double pa = 1.013e5, pre = pa + 0.4e6, q = 8e-6 * 1450.0 / 60.0;
			const QVector<SimPhysics::GasCharge> g{{pre, 1e-3, 1.4}};
			double p = pa;
			const long long n = std::llround(2.0 / dt);
			for (long long i = 0; i < n; ++i) p = SimPhysics::oilNodeWithAccumulators(p, 1e-4, 0.0, q, 7e8, dt, 0.0, g);
			// exact: oil q·2 s minus what compressed the line oil, gas law for the rest
			double lo = pre, hi = 1e8;
			for (int i = 0; i < 200; ++i) {
				const double m = 0.5 * (lo + hi);
				const double oil = 1e-3 - 1e-3 * std::pow(pre / m, 1.0 / 1.4) + 1e-4 / 7e8 * (m - pa);
				(oil > q * 2.0 ? hi : lo) = m;
			}
			return std::abs(p - 0.5 * (lo + hi));
		};
		const double f1 = fill(1e-5), f2 = fill(1e-4);
		Check c{QStringLiteral("AccumulatorCharge (Hydrospeicher, Gasgesetz n = 1,4, auch über den Vorfülldruck)")};
		c.passed = e1 <= 0.1 && e2 <= 0.1 && f1 <= 1.0 && f2 <= 1.0;
		c.detail = QStringLiteral("Referenzfall max. Fehler %1 Pa (dt 10 µs), %2 Pa (dt 5 µs); Laden ab 1 bar über den "
								  "Vorfülldruck: Fehler nach 2 s %3 Pa (dt 10 µs), %4 Pa (dt 100 µs); Grenzen 0,1 / 1 Pa")
					   .arg(e1, 0, 'g', 3).arg(e2, 0, 'g', 3).arg(f1, 0, 'g', 3).arg(f2, 0, 'g', 3);
		checks << c;
	}

	// hydraulic CylinderCycle: pump, 4/3 valve, differential cylinder (OwnHydraulics.Examples.CylinderCycle)
	{
		struct Run { QVector<Sample> s; double balance = 0.0, pmin = 0.0, pmax = 0.0; };
		auto run = [&](double dt) {
			Run r;
			const double bulk = 5e8, aa = 1e-3, ab = 7e-4, dead = 1e-4, floor = -0.9e5;
			Piston z;
			z.m = 50.0; z.stroke = 0.1; z.f_static = 20.0; z.f_slide = 20.0; z.damping = 5000.0; z.x = 0.02;
			double ps = 0.0, pa = 0.0, pb = 0.0;
			r.pmin = 0.0; r.pmax = 0.0;
			const long long n = std::llround(1.8 / dt), every = std::llround(0.001 / dt);
			for (long long i = 0; i <= n; ++i) {
				const double t = i * dt;
				if (i % every == 0) r.s << Sample{t, z.x, pa, pb};
				if (i == n) break;
				const int cmd = t < 0.7 ? 1 : (t < 0.9 ? 0 : -1);
				// pump 1e-4 m³/s, built-in pressure limit 5.1 MPa (full flow up to 5.0 MPa, as the reference relief valve)
				const double q_pump = 1e-4 * qBound(0.0, (5.1e6 - ps) / 1e5, 1.0);
				const double q_pa = cmd > 0 ? SimPhysics::orificeFlow(ps - pa, kOrifice, DP_SMOOTH) : 0.0;
				const double q_bt = cmd > 0 ? SimPhysics::orificeFlow(pb, kOrifice, DP_SMOOTH) : 0.0;
				const double q_pb = cmd < 0 ? SimPhysics::orificeFlow(ps - pb, kOrifice, DP_SMOOTH) : 0.0;
				const double q_at = cmd < 0 ? SimPhysics::orificeFlow(pa, kOrifice, DP_SMOOTH) : 0.0;
				SimPhysics::pistonStep(z, pa * aa - pb * ab, dt);
				const double va = dead + aa * z.x, vb = dead + ab * (z.stroke - z.x);
				const double pa_new = SimPhysics::oilNode(pa, va, aa * z.v, q_pa - q_at, bulk, dt, floor);
				const double pb_new = SimPhysics::oilNode(pb, vb, -ab * z.v, q_pb - q_bt, bulk, dt, floor);
				// volume balance of chamber A: inflow = compression + displacement
				if (pa_new > floor)
					r.balance = std::max(r.balance, std::abs((q_pa - q_at) * dt - (va / bulk * (pa_new - pa) + aa * z.v * dt)));
				pa = pa_new;
				pb = pb_new;
				ps = SimPhysics::oilNode(ps, 1e-4, 0.0, q_pump - q_pa - q_pb, bulk, dt, floor);
				r.pmin = std::min({r.pmin, pa, pb});
				r.pmax = std::max({r.pmax, pa, pb, ps});
			}
			return r;
		};
		const Run a = run(1e-5), b = run(5e-6);
		Check c{QStringLiteral("Hydraulik-CylinderCycle (Pumpe, 4/3-Ventil, Differentialzylinder)")};
		auto at = [](const QVector<Sample> &s, double t) {
			const long long k = std::llround(t / 0.001);
			return k >= 0 && k < s.size() ? s.at(k).x : std::numeric_limits<double>::quiet_NaN();
		};
		double xmin = 1e9, xmax = -1e9;
		for (const Sample &s : a.s) { xmin = std::min(xmin, s.x); xmax = std::max(xmax, s.x); }
		const double drift = std::abs(at(a.s, 0.9) - at(a.s, 0.8));
		const double t_back = firstTime(a.s, 0.9, [](const Sample &s) { return s.x <= 0.002; });
		const double t_back2 = firstTime(b.s, 0.9, [](const Sample &s) { return s.x <= 0.002; });
		bool ok = xmin >= -0.0001 && xmax <= 0.1001 && at(a.s, 0.699) >= 0.075 && at(a.s, 1.75) < 0.002
				  && drift < 0.002 && a.pmin > -98000 && a.pmax < 1e7 && a.balance < 1e-12
				  && std::abs(at(a.s, 0.699) - at(b.s, 0.699)) < 1e-4 && std::abs(t_back - t_back2) <= 0.002;
		c.detail = QStringLiteral("bei 0,7 s %1 mm, Halten 0,8–0,9 s Drift %2 mm, zurück (< 2 mm) bei %3 s; Drücke %4 … %5 bar; "
								  "Volumenbilanz Kammer A %6 m³ je Schritt; dt halbiert: %7 mm / %8 s")
					   .arg(at(a.s, 0.699) * 1000, 0, 'f', 2).arg(drift * 1000, 0, 'f', 3).arg(t_back, 0, 'f', 3)
					   .arg(a.pmin / 1e5, 0, 'f', 2).arg(a.pmax / 1e5, 0, 'f', 1).arg(a.balance, 0, 'g', 2)
					   .arg(at(b.s, 0.699) * 1000, 0, 'f', 2).arg(t_back2, 0, 'f', 3);
		const QString file = QDir(reference_dir).filePath(QStringLiteral("OwnHydraulics.Examples.CylinderCycle_res.csv"));
		QVector<QVector<double>> ref;
		if (!reference_dir.isEmpty()
			&& readResult(file, {QStringLiteral("cylinder.x"), QStringLiteral("cylinder.pressureA"),
								 QStringLiteral("cylinder.pressureB")}, &ref)) {
			QVector<Sample> r;
			for (const auto &row : ref) r << Sample{row.at(0), row.at(1), row.at(2), row.at(3)};
			auto ref_at = [&](double t) {
				for (const Sample &s : r) if (s.t >= t - 1e-9) return s.x;
				return std::numeric_limits<double>::quiet_NaN();
			};
			const double r_back = firstTime(r, 0.9, [](const Sample &s) { return s.x <= 0.002; });
			// speeds: extend between 0.1 and 0.6 s, retract between 1.0 and 1.4 s
			const double v_out = (at(a.s, 0.6) - at(a.s, 0.1)) / 0.5, rv_out = (ref_at(0.6) - ref_at(0.1)) / 0.5;
			const double v_in = (at(a.s, 1.4) - at(a.s, 1.0)) / 0.4, rv_in = (ref_at(1.4) - ref_at(1.0)) / 0.4;
			const bool close = std::abs(v_out / rv_out - 1.0) <= 0.03 && std::abs(v_in / rv_in - 1.0) <= 0.03
							   && std::abs(t_back - r_back) <= 0.05 * (r_back - 0.9);
			ok = ok && close;
			c.detail += QStringLiteral("; OpenModelica: Ausfahren %1 mm/s (wir %2), Einfahren %3 mm/s (wir %4), "
									   "zurück bei %5 s (Grenze ±3 % Geschwindigkeit, ±5 % Rückhubzeit)")
							.arg(rv_out * 1000, 0, 'f', 2).arg(v_out * 1000, 0, 'f', 2)
							.arg(rv_in * 1000, 0, 'f', 2).arg(v_in * 1000, 0, 'f', 2).arg(r_back, 0, 'f', 3);
		} else {
			c.detail += QStringLiteral("; ohne Vergleichslauf");
		}
		c.passed = ok;
		checks << c;
	}

	// GRAFCET notation (IEC 60848) to Structured Text, then evaluated
	{
		Check c{QStringLiteral("GRAFCET-Schreibweise (Zeiten, Kennzeichen, ·, /, Flanke)")};
		QStringList wrong;
		struct Case { QString text; QHash<QString, double> vars; double want; };
		const QList<Case> cases{
			{QStringLiteral("2s/X3"), {{QStringLiteral("X3.T"), 2.0}}, 1.0},
			{QStringLiteral("2s/X3"), {{QStringLiteral("X3.T"), 1.99}}, 0.0},
			{QStringLiteral("t/X1/500ms"), {{QStringLiteral("X1.T"), 0.5}}, 1.0},
			{QStringLiteral("-BG1 · /-BG2"), {{QStringLiteral("BMK_BG1"), 1.0}}, 1.0},
			{QStringLiteral("-BG1 · /-BG2"), {{QStringLiteral("BMK_BG1"), 1.0}, {QStringLiteral("BMK_BG2"), 1.0}}, 0.0},
			{QStringLiteral("-QM1/14 ∨ DI0"), {{QStringLiteral("BMK_QM1__14"), 1.0}}, 1.0},
			{QStringLiteral("=1"), {}, 1.0},
		};
		for (const Case &k : cases) {
			PlcProgram e;
			QString error;
			QHash<QString, double> v = k.vars;
			if (!e.compileExpression(Grafcet::toStructuredText(k.text), &error))
				wrong << QStringLiteral("„%1“: %2").arg(k.text, error);
			else if (e.evaluate(0.0, v) != k.want)
				wrong << QStringLiteral("„%1“ ergibt %2 statt %3").arg(k.text).arg(e.evaluate(0.0, v)).arg(k.want);
		}
		// rising edge: true only in the cycle the value changes
		PlcProgram edge;
		edge.compileExpression(Grafcet::toStructuredText(QStringLiteral("↑-SF1")));
		QHash<QString, double> v;
		QString seen;
		for (int i = 0; i < 4; ++i) {
			v.insert(QStringLiteral("BMK_SF1"), i >= 1 ? 1.0 : 0.0);
			seen += edge.evaluate(i * 0.01, v) != 0.0 ? QLatin1Char('1') : QLatin1Char('0');
		}
		if (seen != QLatin1String("0100")) wrong << QStringLiteral("Flanke ↑-SF1: %1 statt 0100").arg(seen);
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? QStringLiteral("8 Fälle richtig") : wrong.join(QStringLiteral("; "));
		checks << c;
	}

	// CylinderCycle: functional criteria, step size convergence, comparison with the reference run
	{
		const QVector<Sample> a = cylinderCycle(1e-5), b = cylinderCycle(5e-6);
		Check c{QStringLiteral("CylinderCycle (doppeltwirkender Zylinder, zwei 3/2-Ventile)")};
		double xmin = 1e9, xmax = -1e9, pmin = 1e12;
		for (const Sample &s : a) {
			xmin = std::min(xmin, s.x); xmax = std::max(xmax, s.x);
			pmin = std::min({pmin, s.pa, s.pb});
		}
		const double t_out = firstTime(a, 0.0, [](const Sample &s) { return s.x >= 0.059; });
		const double t_back = firstTime(a, 0.4, [](const Sample &s) { return s.x <= 0.001; });
		const double t_out2 = firstTime(b, 0.0, [](const Sample &s) { return s.x >= 0.059; });
		const double t_back2 = firstTime(b, 0.4, [](const Sample &s) { return s.x <= 0.001; });
		double x_after = 0.0;
		for (const Sample &s : a) if (s.t >= 0.9) x_after = std::max(x_after, s.x);
		bool ok = xmin >= -0.002 && xmax <= 0.062 && t_out < 0.4 && x_after < 0.001 && pmin > 0
				  && std::abs(t_out - t_out2) <= 0.001 && std::abs(t_back - t_back2) <= 0.001;
		c.detail = QStringLiteral("aus bei %1 s, zurück bei %2 s; dt halbiert: %3 s / %4 s; min. Druck %5 bar abs")
					   .arg(t_out, 0, 'f', 3).arg(t_back, 0, 'f', 3).arg(t_out2, 0, 'f', 3)
					   .arg(t_back2, 0, 'f', 3).arg(pmin / 1e5, 0, 'f', 3);

		const QString file = QDir(reference_dir).filePath(QStringLiteral("OwnSim.Examples.CylinderCycle_res.csv"));
		QVector<QVector<double>> ref;
		if (!reference_dir.isEmpty()
			&& readResult(file, {QStringLiteral("cylinder.x"), QStringLiteral("cylinder.chamberA.p"),
								 QStringLiteral("cylinder.chamberB.p")}, &ref)) {
			QVector<Sample> r;
			for (const auto &row : ref) r << Sample{row.at(0), row.at(1), row.at(2), row.at(3)};
			const double r_out = firstTime(r, 0.0, [](const Sample &s) { return s.x >= 0.059; });
			const double r_back = firstTime(r, 0.4, [](const Sample &s) { return s.x <= 0.001; });
			double dx = 0.0, dp = 0.0;
			for (const Sample &s : r) {
				const long long k = std::llround(s.t / 0.001);
				if (k < 0 || k >= a.size() || std::abs(a.at(k).t - s.t) > 1e-9) continue;
				dx = std::max(dx, std::abs(a.at(k).x - s.x));
				dp = std::max({dp, std::abs(a.at(k).pa - s.pa), std::abs(a.at(k).pb - s.pb)});
			}
			// travel time from the reference run, within 5 % of the stroke time
			const double tol_out = 0.05 * r_out, tol_back = 0.05 * (r_back - 0.4);
			ok = ok && std::abs(t_out - r_out) <= tol_out && std::abs(t_back - r_back) <= tol_back;
			c.detail += QStringLiteral("; OpenModelica: aus bei %1 s, zurück bei %2 s (Abweichung %3 ms / %4 ms, "
									   "Grenze %5 / %6 ms); größte Abweichung Weg %7 mm, Druck %8 bar")
							.arg(r_out, 0, 'f', 3).arg(r_back, 0, 'f', 3)
							.arg((t_out - r_out) * 1000, 0, 'f', 1).arg((t_back - r_back) * 1000, 0, 'f', 1)
							.arg(tol_out * 1000, 0, 'f', 1).arg(tol_back * 1000, 0, 'f', 1)
							.arg(dx * 1000, 0, 'f', 2).arg(dp / 1e5, 0, 'f', 3);
		} else {
			c.detail += QStringLiteral("; ohne Vergleichslauf (Ordner mit OwnSim.Examples.CylinderCycle_res.csv angeben)");
		}
		c.passed = ok;
		checks << c;
	}

	int failed = 0;
	for (const Check &c : checks) {
		out << (c.passed ? "OK    " : "FEHLER") << "  " << c.name << "\n        " << c.detail << "\n";
		if (!c.passed) ++failed;
	}
	out << (failed ? QStringLiteral("%1 von %2 Prüfungen fehlgeschlagen").arg(failed).arg(checks.size())
				   : QStringLiteral("Alle %1 Prüfungen bestanden").arg(checks.size()))
		<< "\n";
	out.flush();
	return failed ? 1 : 0;
}
