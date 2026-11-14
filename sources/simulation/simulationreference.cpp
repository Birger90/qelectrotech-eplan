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
#include "simulationmodel.h"
#include "simulationphysics.h"
#include "simulationunits.h"
#include "../diagram.h"
#include "../fluid/fluiddata.h"
#include "../properties/elementdata.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/terminal.h"
#include "../qetproject.h"

#include <algorithm>
#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QHash>
#include <QRegularExpression>
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
	bool skipped = false;       ///< not run (no reference data); shown as SKIP, never as passed
};

Check skip(const QString &name, const QString &why)
{
	Check c{name, false, why};
	c.skipped = true;
	return c;
}

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

	/**
		reads time and the named columns of an OpenModelica result file; rows
		with a repeated time (event) keep the value after the event.
		@return empty, or why the file cannot be used: missing, a column
		missing, a cell that is no finite number, time going back, or a run
		ending before @a end_time. Implausible reference data fails here,
		before anything is compared with it.
	*/
QString readResult(const QString &file, const QStringList &columns, double end_time,
				   QVector<QVector<double>> *rows)
{
	QFile f(file);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
		return QStringLiteral("Referenzdatei fehlt: %1").arg(QDir::toNativeSeparators(file));
	QStringList head = QString::fromUtf8(f.readLine()).trimmed().split(QLatin1Char(','));
	for (QString &h : head) h.remove(QLatin1Char('"'));
	QVector<int> idx;
	for (const QString &c : QStringList{QStringLiteral("time")} + columns) {
		idx << static_cast<int>(head.indexOf(c));
		if (idx.last() < 0) return QStringLiteral("Spalte %1 fehlt in %2").arg(c, QFileInfo(file).fileName());
	}
	double last_t = -std::numeric_limits<double>::infinity();
	int line = 1;
	while (!f.atEnd()) {
		++line;
		const QStringList cells = QString::fromUtf8(f.readLine()).trimmed().split(QLatin1Char(','));
		QVector<double> row;
		for (int i : idx) {
			bool ok = false;
			const double v = cells.value(i).toDouble(&ok);
			if (!ok || !std::isfinite(v))
				return QStringLiteral("%1 Zeile %2: „%3“ ist keine endliche Zahl")
					.arg(QFileInfo(file).fileName()).arg(line).arg(cells.value(i));
			row << v;
		}
		if (row.first() < last_t)
			return QStringLiteral("%1 Zeile %2: Zeit läuft rückwärts").arg(QFileInfo(file).fileName()).arg(line);
		if (row.first() == last_t) { rows->last() = row; continue; }   // keep the value after an event
		last_t = row.first();
		*rows << row;
	}
	if (rows->isEmpty() || last_t < end_time - 1e-9)
		return QStringLiteral("%1 endet bei %2 s, erwartet %3 s").arg(QFileInfo(file).fileName()).arg(last_t).arg(end_time);
	return QString();
}

	/// limits of the physics: void at the pressure floor, not finite input, units
QList<Check> physicsLimits()
{
	QList<Check> out;
	{
		// sealed 0.1 l of oil at 1 bar abs: 1e-4 m³/s out for 0.1 s, then the same back in
		Check c{QStringLiteral("Unterdruck im Öl: Hohlraum bilanziert und wieder gefüllt")};
		const double v = 1e-4, e = 7e8, floor = 1.013e5 - 0.9e5, dt = 1e-5, start = 1.013e5;
		double p = start, cavity = 0.0, worst = 0.0, deepest = 0.0, cav_max = 0.0;
		for (int i = 0; i < 20000; ++i) {
			const double q = i < 10000 ? -1e-4 : 1e-4;
			const double c0 = cavity, p0 = p;
			p = SimPhysics::oilNode(p, v, 0.0, q, e, dt, floor, cavity);
			worst = std::max(worst, std::abs(q * dt - (v / e * (p - p0) - (cavity - c0))));
			deepest = std::min(deepest, p - floor);
			cav_max = std::max(cav_max, cavity);
		}
		const double expected_void = 1e-5 - v / e * (start - floor);
		c.passed = worst < 1e-15 && deepest >= 0.0 && std::abs(cav_max - expected_void) < 1e-12
				   && std::abs(p - start) < 1e-6 && cavity == 0.0;
		c.detail = QStringLiteral("größter Hohlraum %1 cm³ (soll %2), nie unter der Druckuntergrenze, Bilanzfehler je Schritt "
								  "%3 m³; nach dem Wiederbefüllen %4 bar abs (Start %5), Hohlraum %6")
					   .arg(cav_max * 1e6, 0, 'f', 6).arg(expected_void * 1e6, 0, 'f', 6).arg(worst, 0, 'g', 2)
					   .arg(p / 1e5, 0, 'f', 6).arg(start / 1e5, 0, 'f', 6).arg(cavity);
		out << c;
	}
	{
		Check c{QStringLiteral("Ungültige Werte (NaN, Volumen 0, Speicherdaten) werden nicht zu Mindestdruck")};
		double cav = 0.0;
		const double nan = std::numeric_limits<double>::quiet_NaN();
		QStringList wrong;
		if (std::isfinite(SimPhysics::oilNode(1e6, 1e-4, 0.0, nan, 7e8, 1e-5, 11300.0, cav))) wrong << QStringLiteral("Öl, Durchfluss NaN");
		if (std::isfinite(SimPhysics::oilNode(1e6, 0.0, 0.0, 1e-5, 7e8, 1e-5, 11300.0, cav))) wrong << QStringLiteral("Öl, Volumen 0");
		if (std::isfinite(SimPhysics::oilNode(1e6, 1e-4, 0.0, 1e-5, 0.0, 1e-5, 11300.0, cav))) wrong << QStringLiteral("Öl, Kompressionsmodul 0");
		if (std::isfinite(SimPhysics::gasNode(1e5, 0.0, 0.0, 1e-5, RT, 1e-5))) wrong << QStringLiteral("Luft, Volumen 0");
		if (std::isfinite(SimPhysics::gasNode(nan, 1e-3, 0.0, 1e-5, RT, 1e-5))) wrong << QStringLiteral("Luft, Druck NaN");
		if (std::isfinite(SimPhysics::oilNodeWithAccumulators(1e6, 1e-4, 0.0, 1e-5, 7e8, 1e-5, 11300.0,
				QVector<SimPhysics::GasCharge>{{1e6, 0.0, 1.4}}, cav))) wrong << QStringLiteral("Speicher, Gasvolumen 0");
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? QStringLiteral("6 Negativfälle liefern NaN; das Modell bricht dann mit Meldung ab")
								   : QStringLiteral("endlicher Wert bei: ") + wrong.join(QStringLiteral(", "));
		out << c;
	}
	{
		Check c{QStringLiteral("Einheiten nach Angabe, nicht nach Zahlenwert")};
		QStringList wrong;
		auto expect = [&](double value, const char *from, const char *to, double want) {
			const double got = SimUnits::convert(value, QString::fromLatin1(from), QString::fromLatin1(to));
			const bool ok = std::isnan(want) ? std::isnan(got) : std::abs(got - want) <= 1e-9 * std::abs(want);
			if (!ok) wrong << QStringLiteral("%1 %2 → %3 = %4 (soll %5)").arg(value).arg(QString::fromLatin1(from),
				QString::fromLatin1(to)).arg(got).arg(want);
		};
		const double nan = std::numeric_limits<double>::quiet_NaN();
		expect(6, "MPa", "Pa", 6e6);
		expect(60, "bar", "Pa", 6e6);
		expect(99, "MPa", "Pa", 99e6);
		expect(100, "MPa", "Pa", 100e6);   // was read as 100 bar before
		expect(100, "bar", "Pa", 100e5);
		expect(8, "cm3", "m3", 8e-6);
		expect(1450, "1/min", "1/s", 1450.0 / 60.0);
		expect(1, "l", "Pa", nan);         // different quantities
		expect(1, "psi", "Pa", nan);       // unknown unit
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? QStringLiteral("9 Fälle richtig (99 und 100 MPa beide als MPa; unbekannte oder "
													"unpassende Einheit = Fehler)")
								   : wrong.join(QStringLiteral("; "));
		out << c;
	}
	return out;
}

	// ---- end to end: real projects through the same SimulationModel as window and CLI

struct Input { double t; QString label; int kind; bool on; };   // kind 0 push button, 1 valve left, 2 valve right

struct ProjectRun {
	std::unique_ptr<QETProject> project;
	std::unique_ptr<SimulationModel> sim;
	QList<QPair<double, QString>> events;
	QString error;

	Element *element(const QString &label) const {
		for (Diagram *d : project->diagrams())
			for (Element *e : d->elements())
				if (e->elementInformations()[QStringLiteral("label")].toString() == label) return e;
		return nullptr;
	}
	Terminal *terminal(const QString &label, const QString &name) const {
		if (Element *e = element(label))
			for (Terminal *t : e->terminals())
				if (t->name() == name) return t;
		return nullptr;
	}
	double eventTime(const QString &text, double after = 0.0) const {
		for (const auto &e : events)
			if (e.first >= after - 1e-12 && e.second.contains(text)) return e.first;
		return std::numeric_limits<double>::quiet_NaN();
	}
	SimulationModel::CylinderState cylinder(const QString &label) const {
		for (const auto &c : sim->cylinders())
			if (c.label == label) return c;
		return SimulationModel::CylinderState{nullptr, label, std::nan(""), std::nan(""), 0.0};
	}
};

ProjectRun runProject(const QString &file, double supply_bar, QList<Input> input, double duration)
{
	ProjectRun r;
	if (!QFileInfo::exists(file)) {
		r.error = QStringLiteral("Testprojekt fehlt: %1").arg(QDir::toNativeSeparators(file));
		return r;
	}
	r.project = std::make_unique<QETProject>(file);
	if (r.project->state() != QETProject::Ok) {
		r.error = QStringLiteral("Testprojekt lässt sich nicht öffnen: %1").arg(QFileInfo(file).fileName());
		return r;
	}
	r.sim = std::make_unique<SimulationModel>(r.project->diagrams(), supply_bar);
	if (r.sim->failed()) {
		r.error = QStringLiteral("Modell: %1").arg(r.sim->failure());
		return r;
	}
	std::sort(input.begin(), input.end(), [](const Input &a, const Input &b) { return a.t < b.t; });
	int next = 0;
	const double dt = 1e-5;
	while (r.sim->time() < duration - 1e-12) {
		while (next < input.size() && input.at(next).t <= r.sim->time() + 1e-12) {
			const Input &in = input.at(next++);
			if (in.kind == 0) r.sim->pressButton(in.label, in.on);
			else r.sim->actuateValve(in.label, in.kind == 1, in.on);
		}
		r.sim->step(dt);
		for (const auto &m : r.sim->takeMessages()) r.events << qMakePair(m.time, m.text);
		if (r.sim->failed()) {
			r.error = QStringLiteral("Abbruch: %1").arg(r.sim->failure());
			break;
		}
	}
	return r;
}

	/// a copy of @a file in the temporary folder where attribute @a attr of the parameter @a param is @a value
QString variant(const QString &file, const QString &param, const QString &attr, const QString &value,
				const QString &name)
{
	QFile f(file);
	if (!f.open(QIODevice::ReadOnly)) return QString();
	QString text = QString::fromUtf8(f.readAll());
	const QRegularExpression tag(QStringLiteral("<parameter\\b[^>]*\\bname=\"%1\"[^>]*>").arg(param));
	const QRegularExpression at(QStringLiteral("\\b%1=\"[^\"]*\"").arg(attr));
	int changed = 0;
	QString result;
	int last = 0;
	auto it = tag.globalMatch(text);
	while (it.hasNext()) {
		const auto m = it.next();
		QString t = m.captured(0);
		if (!at.match(t).hasMatch()) continue;
		t.replace(at, QStringLiteral("%1=\"%2\"").arg(attr, value));
		result += text.mid(last, m.capturedStart() - last) + t;
		last = static_cast<int>(m.capturedEnd());
		++changed;
	}
	if (!changed) return QString();
	text = result + text.mid(last);
	const QString out = QDir(QDir::tempPath()).filePath(name);
	QFile o(out);
	if (!o.open(QIODevice::WriteOnly)) return QString();
	o.write(text.toUtf8());
	return out;
}

	/// characteristic value of the fluid symbol @a e in unit @a to
double symbolValue(Element *e, const QString &name, const QString &to)
{
	if (!e) return std::nan("");
	for (const FluidData::Parameter &p : e->elementData().m_fluid.m_parameters)
		if (p.name == name) return SimUnits::convert(p.value.toDouble(), p.unit, to);
	return std::nan("");
}

QList<Check> endToEnd(const QString &reference_dir)
{
	QList<Check> out;
	const QString dir = reference_dir.isEmpty() ? QString() : QDir(reference_dir).filePath(QStringLiteral("plaene"));
	auto path = [&](const char *f) { return QDir(dir).filePath(QString::fromLatin1(f)); };
	const QStringList names{
		QStringLiteral("Projekt Hydraulik: Ausfahrzeit = Hub · Kolbenfläche / Förderstrom"),
		QStringLiteral("Projekt Blindstopfen: B offen entlüftet, B verschlossen wirkt als Luftfeder"),
		QStringLiteral("Projekt Druckbegrenzung: Eingang 1, Rücklauf 2, öffnet erst über dem Einstelldruck"),
		QStringLiteral("Projekt GRAFCET: Schrittfolge, Parallelzweig, Synchronisation, Zykluszähler"),
		QStringLiteral("Projekt Speicher, Druckschalter, Wechsel- und Zweidruckventil"),
		QStringLiteral("Projekt mit ungültigen Daten: Lauf startet nicht, Fehler mit Bauteil")};
	if (dir.isEmpty() || !QFileInfo(dir).isDir()) {
		for (const QString &n : names) out << skip(n, QStringLiteral("keine Testprojekte gefunden (referenz/plaene)"));
		return out;
	}
	auto failedRun = [&](const QString &name, const ProjectRun &r) {
		Check c{name};
		c.detail = r.error;
		return c;
	};

	{   // 1. hydraulic cylinder: extend time from the symbol data
		ProjectRun r = runProject(path("hydraulik.qet"), 6.0, {{0.3, QStringLiteral("-SF1"), 0, true}}, 2.5);
		if (!r.error.isEmpty()) out << failedRun(names.at(0), r);
		else {
			Check c{names.at(0)};
			Element *cyl = nullptr, *pump = nullptr;
			for (Diagram *d : r.project->diagrams())
				for (Element *e : d->elements()) {
					const FluidData &f = e->elementData().m_fluid;
					if (f.m_kind == FluidData::Kind::Cylinder) cyl = e;
					if (f.m_function.contains(QLatin1String("pump"))) pump = e;
				}
			const double d = symbolValue(cyl, QStringLiteral("d_piston"), QStringLiteral("m"));
			const double stroke = symbolValue(cyl, QStringLiteral("xMax"), QStringLiteral("m"));
			const double q = symbolValue(pump, QStringLiteral("displacement"), QStringLiteral("m3"))
						   * symbolValue(pump, QStringLiteral("rpm"), QStringLiteral("1/s"));
			const double expected = stroke * M_PI * d * d / 4.0 / q;
			const double t0 = r.eventTime(QStringLiteral("schaltet in Stellung"), 0.3);
			const double t1 = r.eventTime(QStringLiteral("Endlage vorne"), 0.3);
			const double took = t1 - t0;
			c.passed = std::isfinite(took) && std::abs(took / expected - 1.0) <= 0.03;
			c.detail = QStringLiteral("ausgefahren in %1 s (Ventil schaltet bei %2 s), Handrechnung %3 s "
									  "(Ø %4 mm, Hub %5 mm, %6 l/min); Grenze ±3 %")
						   .arg(took, 0, 'f', 3).arg(t0, 0, 'f', 3).arg(expected, 0, 'f', 3)
						   .arg(d * 1000, 0, 'f', 0).arg(stroke * 1000, 0, 'f', 0).arg(q * 60000, 0, 'f', 2);
			out << c;
		}
	}
	{   // 2. plug: the same cylinder with port B open or closed
		const QList<Input> in{{0.2, QStringLiteral("-QM1"), 1, true}};
		ProjectRun open = runProject(path("zylinder_b_offen.qet"), 6.0, in, 1.5);
		ProjectRun plug = runProject(path("zylinder_b_blindstopfen.qet"), 6.0, in, 1.5);
		if (!open.error.isEmpty()) out << failedRun(names.at(1), open);
		else if (!plug.error.isEmpty()) out << failedRun(names.at(1), plug);
		else {
			Check c{names.at(1)};
			// open: B vents, no pressure in B. Plugged: the air in B (chamber, dead volume, line) is an
			// air spring, compressed isothermally; 6 bar on A still pushes it to the end, but slower and
			// with pressure left in B (about 3.8 bar for this cylinder)
			const double pb_open = open.sim->pressureBar(open.terminal(QStringLiteral("-MM1"), QStringLiteral("B")));
			const double pb = plug.sim->pressureBar(plug.terminal(QStringLiteral("-MM1"), QStringLiteral("B")));
			const double t_open = open.eventTime(QStringLiteral("-MM1 Endlage vorne"));
			const double t_plug = plug.eventTime(QStringLiteral("-MM1 Endlage vorne"));
			c.passed = std::isfinite(t_open) && std::abs(pb_open) < 0.05 && pb > 2.0
					   && (!std::isfinite(t_plug) || t_plug > t_open + 0.005);
			c.detail = QStringLiteral("B offen: vorne bei %1 s, Druck in B %2 bar; B verschlossen: %3, Druck in B %4 bar "
									  "(Luftfeder; Grenzen offen < 0,05 bar, verschlossen > 2 bar und langsamer)")
						   .arg(t_open, 0, 'f', 3).arg(pb_open, 0, 'f', 3)
						   .arg(std::isfinite(t_plug) ? QStringLiteral("vorne bei %1 s").arg(t_plug, 0, 'f', 3)
													  : QStringLiteral("nicht vorne"))
						   .arg(pb, 0, 'f', 2);
			out << c;
		}
	}
	{   // 3. relief valve 0.6 MPa between a 5 and an 8 bar supply and a cylinder
		ProjectRun low = runProject(path("druckbegrenzung.qet"), 5.0, {}, 1.0);
		ProjectRun high = runProject(path("druckbegrenzung.qet"), 8.0, {}, 1.0);
		if (!low.error.isEmpty()) out << failedRun(names.at(2), low);
		else if (!high.error.isEmpty()) out << failedRun(names.at(2), high);
		else {
			Check c{names.at(2)};
			const double pa_low = low.sim->pressureBar(low.terminal(QStringLiteral("-MM1"), QStringLiteral("A")));
			const double pa_high = high.sim->pressureBar(high.terminal(QStringLiteral("-MM1"), QStringLiteral("A")));
			const bool moved = std::isfinite(high.eventTime(QStringLiteral("-MM1 Endlage vorne")));
			// opens at 6 bar difference inlet - return, fully open 1 bar above: the return settles 8 - 6...7 bar
			c.passed = pa_low < 0.05 && moved && pa_high > 1.0 && pa_high < 2.1;
			c.detail = QStringLiteral("Versorgung 5 bar: Rücklauf %1 bar (zu); 8 bar: Rücklauf %2 bar%3 "
									  "(soll 1…2 bar: Einstelldruck 6 bar, Öffnungsbereich 1 bar)")
						   .arg(pa_low, 0, 'f', 2).arg(pa_high, 0, 'f', 2)
						   .arg(moved ? QStringLiteral(", Zylinder ausgefahren") : QStringLiteral(", Zylinder steht"));
			out << c;
		}
	}
	{   // 4. GRAFCET
		ProjectRun r = runProject(path("grafcet.qet"), 6.0,
			{{0.1, QStringLiteral("-SF1"), 0, true}, {0.2, QStringLiteral("-SF1"), 0, false}}, 2.5);
		if (!r.error.isEmpty()) out << failedRun(names.at(3), r);
		else {
			Check c{names.at(3)};
			QStringList steps;
			for (const auto &e : r.events)
				if (e.second.startsWith(QLatin1String("GRAFCET Schritt ")))
					steps << e.second.mid(16).section(QLatin1Char(' '), 0, 0);
			// 2 and 6 start together (parallel branch), their order inside one cycle is free
			QStringList norm = steps;
			for (int i = 0; i + 1 < norm.size(); ++i)
				if ((norm.at(i) == QLatin1String("6") && norm.at(i + 1) == QLatin1String("2")))
					norm.swapItemsAt(i, i + 1);
			const QString seq = norm.join(QLatin1Char(' '));
			const double t3 = r.eventTime(QStringLiteral("GRAFCET Schritt 3 aktiv"));
			const double t4 = r.eventTime(QStringLiteral("GRAFCET Schritt 4 aktiv"));
			const double cycles = r.sim->plcValue(QStringLiteral("ZYKLEN"));
			c.passed = seq == QLatin1String("0 1 2 6 3 7 4 5 0") && std::abs(t4 - t3 - 1.0) < 0.011 && cycles == 1.0;
			c.detail = QStringLiteral("Schritte %1 (soll 0 1 2 6 3 7 4 5 0), Synchronisation %2 s nach Schritt 3 "
									  "(soll 1 s), Zyklen %3").arg(seq).arg(t4 - t3, 0, 'f', 3).arg(cycles);
			out << c;
		}
	}
	{   // 5. accumulator charged by the pump, pressure switch, OR and AND valves
		ProjectRun r = runProject(path("speicher_druckschalter_logik.qet"), 6.0,
			{{0.1, QStringLiteral("-QM1"), 1, true}, {0.8, QStringLiteral("-QM2"), 1, true}}, 2.0);
		if (!r.error.isEmpty()) out << failedRun(names.at(4), r);
		else {
			Check c{names.at(4)};
			Element *acc = r.element(QStringLiteral("-CM1")), *pump = nullptr;
			for (Diagram *d : r.project->diagrams())
				for (Element *e : d->elements())
					if (e->elementData().m_fluid.m_function.contains(QLatin1String("pump"))) pump = e;
			const double v0 = symbolValue(acc, QStringLiteral("V0"), QStringLiteral("m3"));
			const double pre = symbolValue(acc, QStringLiteral("p0"), QStringLiteral("Pa")) + 1.013e5;
			const double n = symbolValue(acc, QStringLiteral("k"), QStringLiteral("1/s"));   // no unit: number
			const double kappa = std::isfinite(n) ? n : 1.4;
			const double q = symbolValue(pump, QStringLiteral("displacement"), QStringLiteral("m3"))
						   * symbolValue(pump, QStringLiteral("rpm"), QStringLiteral("1/s"));
			// gas law for the oil pumped in 2 s (the line oil takes a negligible share)
			const double expected = (pre * std::pow(v0 / (v0 - q * 2.0), kappa) - 1.013e5) / 1e5;
			double got = std::nan("");
			if (acc && !acc->terminals().isEmpty()) got = r.sim->pressureBar(acc->terminals().first());
			const double or_out = r.eventTime(QStringLiteral("-MM1 Endlage vorne"));
			const double and_out = r.eventTime(QStringLiteral("-MM2 Endlage vorne"));
			const bool sw = std::isfinite(r.eventTime(QStringLiteral("Druckschalter -BP1 schaltet ein")));
			c.passed = std::abs(got - expected) < 0.05 && sw && or_out < 0.8 && and_out > 0.8;
			c.detail = QStringLiteral("Speicher nach 2 s %1 bar (Gasgesetz %2 bar, Grenze 0,05 bar), Druckschalter %3; "
									  "ODER: -MM1 vorne bei %4 s (vor 0,8 s), UND: -MM2 vorne bei %5 s (nach 0,8 s)")
						   .arg(got, 0, 'f', 3).arg(expected, 0, 'f', 3)
						   .arg(sw ? QStringLiteral("ein") : QStringLiteral("aus"))
						   .arg(or_out, 0, 'f', 3).arg(and_out, 0, 'f', 3);
			out << c;
		}
	}
	{   // 6. invalid data: unknown unit, cylinder without stroke
		const QString hyd = path("hydraulik.qet");
		const QString unit = variant(hyd, QStringLiteral("pMax"), QStringLiteral("unit"), QStringLiteral("psi"),
									 QStringLiteral("qet_sim_einheit.qet"));
		const QString stroke = variant(hyd, QStringLiteral("xMax"), QStringLiteral("value"), QStringLiteral("0"),
									   QStringLiteral("qet_sim_hub.qet"));
		Check c{names.at(5)};
		if (unit.isEmpty() || stroke.isEmpty()) {
			c.detail = QStringLiteral("Testvarianten ließen sich nicht erzeugen (pMax/xMax nicht im Projekt gefunden)");
		} else {
			const ProjectRun a = runProject(unit, 6.0, {}, 0.1), b = runProject(stroke, 6.0, {}, 0.1);
			c.passed = a.error.contains(QLatin1String("psi")) && b.error.contains(QLatin1String("Zylinderdaten"));
			c.detail = QStringLiteral("Einheit psi: „%1“; Hub 0: „%2“").arg(a.error.left(140), b.error.left(140));
			QFile::remove(unit);
			QFile::remove(stroke);
		}
		out << c;
	}
	return out;
}

	/// mean of column @a col of @a rows between @a t0 and @a t1
double meanBetween(const QVector<QVector<double>> &rows, int col, double t0, double t1)
{
	double sum = 0.0;
	int n = 0;
	for (const auto &r : rows)
		if (r.at(0) >= t0 - 1e-12 && r.at(0) <= t1 + 1e-12) { sum += r.at(col); ++n; }
	return n ? sum / n : std::numeric_limits<double>::quiet_NaN();
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
			double cavity = 0.0;
			return SimPhysics::oilNode(p, 0.001, 0.0, (1e6 - p) / 1e12, 1e9, dt,
									   -std::numeric_limits<double>::infinity(), cavity);
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
			double cavity = 0.0;
			return SimPhysics::oilNode(p, 1e-3, 0.0, 1e-5, 1e9, dt, -std::numeric_limits<double>::infinity(), cavity);
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
				double cavity = 0.0;
				if (i < n)
					p = SimPhysics::oilNodeWithAccumulators(p, 1e-12, 0.0, 1e-5, 1e9, dt, 0.0,
						QVector<SimPhysics::GasCharge>{{1e6, 1e-3, 1.4}}, cavity);
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
			double cavity = 0.0;
			for (long long i = 0; i < n; ++i) p = SimPhysics::oilNodeWithAccumulators(p, 1e-4, 0.0, q, 7e8, dt, 0.0, g, cavity);
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
		struct Run {
			QVector<Sample> s;
			double balance = 0.0, pmin = 0.0, pmax = 0.0;
			double cav_a = 0.0, cav_b = 0.0, cav_s = 0.0, cav_max = 0.0;
		};
		auto run = [&](double dt) {
			Run r;
			const double bulk = 5e8, aa = 1e-3, ab = 7e-4, dead = 1e-4, floor = -0.9e5;
			Piston z;
			z.m = 50.0; z.stroke = 0.1; z.f_static = 20.0; z.f_slide = 20.0; z.damping = 5000.0; z.x = 0.02;
			double ps = 0.0, pa = 0.0, pb = 0.0, spool = 0.0;
			r.pmin = 0.0; r.pmax = 0.0;
			const long long n = std::llround(1.8 / dt), every = std::llround(0.001 / dt);
			for (long long i = 0; i <= n; ++i) {
				const double t = i * dt;
				if (i % every == 0) r.s << Sample{t, z.x, pa, pb};
				if (i == n) break;
				const double cmd = t < 0.7 ? 1.0 : (t < 0.9 ? 0.0 : -1.0);
				// the spool moves like SimulationModel's valves: linear over the switching time, here
				// 30 ms = three time constants of the reference spool (10 ms, then 95 % open)
				constexpr double T_SPOOL = 0.03;
				spool += qBound(-dt / T_SPOOL, cmd - spool, dt / T_SPOOL);
				const double open_p = std::max(spool, 0.0), open_m = std::max(-spool, 0.0);
				// pump 1e-4 m³/s, built-in pressure limit 5.1 MPa (full flow up to 5.0 MPa, as the reference relief valve)
				const double q_pump = 1e-4 * qBound(0.0, (5.1e6 - ps) / 1e5, 1.0);
				const double q_pa = open_p * SimPhysics::orificeFlow(ps - pa, kOrifice, DP_SMOOTH);
				const double q_bt = open_p * SimPhysics::orificeFlow(pb, kOrifice, DP_SMOOTH);
				const double q_pb = open_m * SimPhysics::orificeFlow(ps - pb, kOrifice, DP_SMOOTH);
				const double q_at = open_m * SimPhysics::orificeFlow(pa, kOrifice, DP_SMOOTH);
				SimPhysics::pistonStep(z, pa * aa - pb * ab, dt);
				const double va = dead + aa * z.x, vb = dead + ab * (z.stroke - z.x);
				const double ca0 = r.cav_a, cb0 = r.cav_b;
				const double pa_new = SimPhysics::oilNode(pa, va, aa * z.v, q_pa - q_at, bulk, dt, floor, r.cav_a);
				const double pb_new = SimPhysics::oilNode(pb, vb, -ab * z.v, q_pb - q_bt, bulk, dt, floor, r.cav_b);
				// volume balance of both chambers, also at the pressure floor:
				// inflow = compression + displacement - growth of the void
				r.balance = std::max({r.balance,
					std::abs((q_pa - q_at) * dt - (va / bulk * (pa_new - pa) + aa * z.v * dt - (r.cav_a - ca0))),
					std::abs((q_pb - q_bt) * dt - (vb / bulk * (pb_new - pb) - ab * z.v * dt - (r.cav_b - cb0)))});
				r.cav_max = std::max({r.cav_max, r.cav_a, r.cav_b});
				pa = pa_new;
				pb = pb_new;
				ps = SimPhysics::oilNode(ps, 1e-4, 0.0, q_pump - q_pa - q_pb, bulk, dt, floor, r.cav_s);
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
								  "Volumenbilanz beider Kammern %6 m³ je Schritt (Grenze 1e-12, auch an der Druckuntergrenze), "
								  "größter Hohlraum %7 cm³; dt halbiert: %8 mm / %9 s")
					   .arg(at(a.s, 0.699) * 1000, 0, 'f', 2).arg(drift * 1000, 0, 'f', 3).arg(t_back, 0, 'f', 3)
					   .arg(a.pmin / 1e5, 0, 'f', 2).arg(a.pmax / 1e5, 0, 'f', 1).arg(a.balance, 0, 'g', 2)
					   .arg(a.cav_max * 1e6, 0, 'g', 3)
					   .arg(at(b.s, 0.699) * 1000, 0, 'f', 2).arg(t_back2, 0, 'f', 3);
		c.passed = ok;
		checks << c;

		// comparison with the OpenModelica run: motion and chamber pressures
		const QString name = QStringLiteral("OpenModelica-Vergleich hydraulisch (Weg, Geschwindigkeiten, Kammerdrücke)");
		if (reference_dir.isEmpty()) {
			checks << skip(name, QStringLiteral("kein Ordner mit Referenzläufen gefunden"));
		} else {
			QVector<QVector<double>> ref;
			const QString error = readResult(QDir(reference_dir).filePath(QStringLiteral("OwnHydraulics.Examples.CylinderCycle_res.csv")),
				{QStringLiteral("cylinder.x"), QStringLiteral("cylinder.pressureA"), QStringLiteral("cylinder.pressureB")}, 1.8, &ref);
			Check v{name};
			if (!error.isEmpty()) {
				v.detail = error;
			} else {
				QVector<QVector<double>> own;
				for (const Sample &s : a.s) own << QVector<double>{s.t, s.x, s.pa, s.pb};
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
				bool same = std::abs(v_out / rv_out - 1.0) <= 0.03 && std::abs(v_in / rv_in - 1.0) <= 0.03
							&& std::abs(t_back - r_back) <= 0.05 * (r_back - 0.9);
				// chamber pressures, mean over windows of steady motion and of holding:
				// within 10 % of the reference or 1 bar, whichever is larger
				struct Window { const char *what; double t0, t1; };
				const QList<Window> windows{{"Ausfahren", 0.2, 0.6}, {"Halten", 0.8, 0.89}, {"Einfahren", 1.0, 1.4}};
				QStringList pressures;
				for (const Window &w : windows)
					for (int col : {2, 3}) {
						const double ours = meanBetween(own, col, w.t0, w.t1), theirs = meanBetween(ref, col, w.t0, w.t1);
						const double limit = std::max(0.1 * std::abs(theirs), 1e5);
						const bool fits = std::abs(ours - theirs) <= limit;
						same = same && fits;
						pressures << QStringLiteral("%1 %2 %3 bar (Ref. %4)%5").arg(QString::fromLatin1(w.what))
							.arg(col == 2 ? QStringLiteral("A") : QStringLiteral("B"))
							.arg(ours / 1e5, 0, 'f', 1).arg(theirs / 1e5, 0, 'f', 1)
							.arg(fits ? QString() : QStringLiteral(" ✗"));
					}
				v.passed = same;
				v.detail = QStringLiteral("Ausfahren %1 mm/s (Ref. %2), Einfahren %3 mm/s (Ref. %4), zurück bei %5 s (Ref. %6); "
										  "Kammerdrücke %7; Grenzen ±3 % Geschwindigkeit, ±5 % Rückhubzeit, Druck ±10 % bzw. 1 bar")
							   .arg(v_out * 1000, 0, 'f', 2).arg(rv_out * 1000, 0, 'f', 2)
							   .arg(v_in * 1000, 0, 'f', 2).arg(rv_in * 1000, 0, 'f', 2)
							   .arg(t_back, 0, 'f', 3).arg(r_back, 0, 'f', 3)
							   .arg(pressures.join(QStringLiteral(", ")));
			}
			checks << v;
		}
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

		c.passed = ok;
		checks << c;

		const QString name = QStringLiteral("OpenModelica-Vergleich pneumatisch (Hubzeiten, Weg, Kammerdrücke)");
		if (reference_dir.isEmpty()) {
			checks << skip(name, QStringLiteral("kein Ordner mit Referenzläufen gefunden"));
		} else {
			QVector<QVector<double>> ref;
			const QString error = readResult(QDir(reference_dir).filePath(QStringLiteral("OwnSim.Examples.CylinderCycle_res.csv")),
				{QStringLiteral("cylinder.x"), QStringLiteral("cylinder.chamberA.p"), QStringLiteral("cylinder.chamberB.p")}, 1.0, &ref);
			Check v{name};
			if (!error.isEmpty()) {
				v.detail = error;
			} else {
				QVector<Sample> r;
				for (const auto &row : ref) r << Sample{row.at(0), row.at(1), row.at(2), row.at(3)};
				const double r_out = firstTime(r, 0.0, [](const Sample &s) { return s.x >= 0.059; });
				const double r_back = firstTime(r, 0.4, [](const Sample &s) { return s.x <= 0.001; });
				double dx = 0.0, dp = 0.0;
				int compared = 0;
				for (const Sample &s : r) {
					const long long k = std::llround(s.t / 0.001);
					if (k < 0 || k >= a.size() || std::abs(a.at(k).t - s.t) > 1e-9) continue;
					++compared;
					dx = std::max(dx, std::abs(a.at(k).x - s.x));
					dp = std::max({dp, std::abs(a.at(k).pa - s.pa), std::abs(a.at(k).pb - s.pb)});
				}
				// travel time within 5 % of the stroke time, every 1-ms point: stroke 1 mm, pressure 0.1 bar
				const double tol_out = 0.05 * r_out, tol_back = 0.05 * (r_back - 0.4);
				v.passed = compared >= 900 && std::abs(t_out - r_out) <= tol_out && std::abs(t_back - r_back) <= tol_back
						   && dx <= 0.001 && dp <= 0.1e5;
				v.detail = QStringLiteral("aus bei %1 s (Ref. %2), zurück bei %3 s (Ref. %4), Grenze %5 / %6 ms; "
										  "an %7 Zeitpunkten größte Abweichung Weg %8 mm (Grenze 1), Druck %9 bar (Grenze 0,1)")
							   .arg(t_out, 0, 'f', 3).arg(r_out, 0, 'f', 3).arg(t_back, 0, 'f', 3).arg(r_back, 0, 'f', 3)
							   .arg(tol_out * 1000, 0, 'f', 1).arg(tol_back * 1000, 0, 'f', 1).arg(compared)
							   .arg(dx * 1000, 0, 'f', 2).arg(dp / 1e5, 0, 'f', 3);
			}
			checks << v;
		}
	}

	checks << physicsLimits() << endToEnd(reference_dir);

	int failed = 0, skipped = 0;
	for (const Check &c : checks) {
		out << (c.skipped ? "SKIP  " : (c.passed ? "OK    " : "FEHLER")) << "  " << c.name << "\n        " << c.detail << "\n";
		if (c.skipped) ++skipped;
		else if (!c.passed) ++failed;
	}
	const int run = static_cast<int>(checks.size()) - skipped;
	out << (failed ? QStringLiteral("%1 von %2 Prüfungen fehlgeschlagen").arg(failed).arg(run)
				   : QStringLiteral("Alle %1 Prüfungen bestanden").arg(run))
		<< (skipped ? QStringLiteral(", %1 übersprungen (keine Referenzdaten)").arg(skipped) : QString()) << "\n";
	out.flush();
	return failed ? 1 : (skipped ? 2 : 0);
}
