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

#include <QDir>
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

int runSimulationReference(const QString &reference_dir, QTextStream &out)
{
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
