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
#include "simulationcatalog.h"
#include "simulationmodel.h"
#include "simulationphysics.h"
#include "simulationunits.h"
#include "../diagram.h"
#include "../fluid/fluiddata.h"
#include "../properties/elementdata.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/terminal.h"
#include "../qetproject.h"
#include "../qetapp.h"

#include <algorithm>
#include <memory>
#include <tuple>

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
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
	double stop_k = 0.0, stop_c = 0.0;
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

ProjectRun runProject(const QString &file, double supply_bar, QList<Input> input, double duration,
					  const std::function<void(SimulationModel &)> &each_step = nullptr, bool record = false)
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
	r.sim->setRecordFlows(record);
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
		if (each_step) each_step(*r.sim);
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

	/// a copy of @a file in the temporary folder where the fluid function @a from is @a to
QString functionVariant(const QString &file, const QString &from, const QString &to, const QString &name)
{
	QFile f(file);
	if (!f.open(QIODevice::ReadOnly)) return QString();
	QString text = QString::fromUtf8(f.readAll());
	const QString old = QStringLiteral("function=\"%1\"").arg(from);
	if (!text.contains(old)) return QString();
	text.replace(old, QStringLiteral("function=\"%1\"").arg(to));
	const QString out = QDir(QDir::tempPath()).filePath(name);
	QFile o(out);
	if (!o.open(QIODevice::WriteOnly)) return QString();
	o.write(text.toUtf8());
	return out;
}

	/// copy of @a file with every pair (old text, new text) replaced once; empty when one is missing
QString textVariant(const QString &file, const QList<QPair<QString, QString>> &pairs, const QString &name)
{
	QFile f(file);
	if (!f.open(QIODevice::ReadOnly)) return QString();
	QString text = QString::fromUtf8(f.readAll());
	for (const auto &pr : pairs) {
		const int at = static_cast<int>(text.indexOf(pr.first));
		if (at < 0) return QString();
		text.replace(at, pr.first.size(), pr.second);
	}
	const QString out = QDir(QDir::tempPath()).filePath(name);
	QFile o(out);
	if (!o.open(QIODevice::WriteOnly)) return QString();
	o.write(text.toUtf8());
	return out;
}

	/// every fluid function of the element collection in @a dir: {function: number of symbols}
QHash<QString, int> collectionFunctions(const QString &dir)
{
	QHash<QString, int> out;
	static const QRegularExpression fluid(QStringLiteral("<fluid\\b[^>]*\\bfunction=\"([^\"]*)\""));
	QDirIterator it(dir, {QStringLiteral("*.elmt")}, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QFile f(it.next());
		if (!f.open(QIODevice::ReadOnly)) continue;
		const auto m = fluid.match(QString::fromUtf8(f.readAll()));
		if (m.hasMatch()) out[m.captured(1)] += 1;
	}
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
		QStringLiteral("Projekt mit ungültigen Daten: Lauf startet nicht, Fehler mit Bauteil"),
		QStringLiteral("Modellkatalog: ohne Rechenmodell abgewiesen, Grafikbaustein ignoriert"),
		QStringLiteral("Messgrößen: Abgriff ohne Rückwirkung, Durchfluss bilanziert, Pumpe = Verdrängung · Drehzahl"),
		QStringLiteral("Netze, Stabilität, Ereignisse (R03): Widersprüche und Instabilität gemeldet, gleichzeitige Eingaben reproduzierbar"),
		QStringLiteral("Hydromotor (R08): Drehzahl, Moment, Lastsprung, Umkehr, Leistungsbilanz, Projekt"),
		QStringLiteral("Vakuum und Schwenk-/Greiferantriebe (R10): Saugen, Halten, Leckage, Last, Loslassen, Schwenkwinkel und -moment")};
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
	{   // 6. invalid data: units, values out of range, unreadable numbers; a failed model does not run
		struct Bad { const char *param, *attr, *value, *expect; };
		const QList<Bad> bad{
			{"pMax", "unit", "psi", "psi"},             // unknown unit
			{"rT", "unit", "psi", "rT"},                // rT in another unit
			{"rT", "value", "-0.1", "rT"},              // rT not positive
			{"d_piston", "value", "-40", "d_piston"},   // negative diameter (was squared to a valid area)
			{"d_piston", "value", "abc", "keine Zahl"}, // unreadable
			{"xMax", "value", "0", "xMax"},             // no stroke
			{"m_cyl", "value", "-2", "m_cyl"},          // negative mass
		};
		const QString hyd = path("hydraulik.qet");
		Check c{names.at(5)};
		QStringList wrong, seen;
		bool loop_ok = true;
		int i = 0;
		for (const Bad &b : bad) {
			const QString file = variant(hyd, QString::fromLatin1(b.param), QString::fromLatin1(b.attr),
										 QString::fromLatin1(b.value), QStringLiteral("qet_sim_ungueltig_%1.qet").arg(i++));
			if (file.isEmpty()) {
				if (QString::fromLatin1(b.param) == QLatin1String("m_cyl")) continue;   // the symbol has no m_cyl: nothing to vary
				wrong << QStringLiteral("%1 nicht im Projekt").arg(QString::fromLatin1(b.param));
				continue;
			}
			ProjectRun r = runProject(file, 6.0, {}, 0.1);
			if (!r.error.contains(QString::fromUtf8(b.expect)))
				wrong << QStringLiteral("%1 %2=%3: „%4“").arg(QString::fromLatin1(b.param), QString::fromLatin1(b.attr),
															  QString::fromLatin1(b.value), r.error.left(120));
			else
				seen << QStringLiteral("%1 %2").arg(QString::fromLatin1(b.param), QString::fromLatin1(b.value));
			// the failed model must not run: runUntil returns at once (it looped before 1.21.0)
			if (r.sim && r.sim->failed()) {
				r.sim->runUntil(0.01);
				loop_ok = loop_ok && r.sim->time() == 0.0;
			}
			QFile::remove(file);
		}
		c.passed = wrong.isEmpty() && loop_ok && seen.size() >= 6;
		c.detail = wrong.isEmpty()
			? QStringLiteral("abgewiesen mit Bauteil und Kennwert: %1; runUntil auf ungültigem Modell kehrt sofort zurück: %2")
				  .arg(seen.join(QStringLiteral(", ")), loop_ok ? QStringLiteral("ja") : QStringLiteral("nein"))
			: wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 7. model catalog (R00): a part without a model stops the run with its name and function, an
		//    unknown function as well (no guessed source, no linear piston for a muscle); a graphic part is ignored
		struct Case { const char *from, *to, *expect; };
		const QList<Case> cases{
			{"supply.pump", "vacuum.vacuum_efficiency_valve", "vacuum.vacuum_efficiency_valve hat kein Rechenmodell"},
			{"actuator.cyl_conf", "actuator.fluidic_muscle1", "actuator.fluidic_muscle1 hat kein Rechenmodell"},
			{"supply.tank", "unbekannt.pruefling", "steht nicht im Modellkatalog"},
			{"actuator.cyl_conf", "directional_valve.part", ""},
		};
		const QString hyd = path("hydraulik.qet");
		Check c{names.at(6)};
		QStringList wrong, seen;
		int i = 0;
		for (const Case &k : cases) {
			const QString file = functionVariant(hyd, QString::fromLatin1(k.from), QString::fromLatin1(k.to),
												 QStringLiteral("qet_sim_katalog_%1.qet").arg(i++));
			if (file.isEmpty()) { wrong << QStringLiteral("%1 nicht im Projekt").arg(QString::fromLatin1(k.from)); continue; }
			ProjectRun r = runProject(file, 6.0, {}, 0.05);
			const QString expect = QString::fromUtf8(k.expect);
			if (expect.isEmpty()) {
				if (!r.error.isEmpty()) wrong << QStringLiteral("Grafikbaustein: „%1“").arg(r.error.left(120));
				else seen << QStringLiteral("Grafikbaustein ignoriert");
			} else if (!r.error.contains(expect)) {
				wrong << QStringLiteral("%1 → %2: „%3“").arg(QString::fromLatin1(k.from), QString::fromLatin1(k.to), r.error.left(160));
			} else {
				seen << QString::fromLatin1(k.to) + QStringLiteral(" abgewiesen");
			}
			QFile::remove(file);
		}
		// every function of the installed collection is decided in the catalog
		// the collection installed with this program first (elements/ beside it), then the configured one
		QHash<QString, int> funcs;
		QString where;
		const QDir app(QCoreApplication::applicationDirPath());
		for (const QString &d : {app.filePath(QStringLiteral("elements")),
								 app.filePath(QStringLiteral("../share/qelectrotech/elements")),
								 QETApp::commonElementsDir()}) {
			if (!funcs.isEmpty() || !QFileInfo(d).isDir()) continue;
			funcs = collectionFunctions(d);
			where = QDir::toNativeSeparators(d);
		}
		QStringList missing;
		int symbols = 0;
		for (auto it = funcs.constBegin(); it != funcs.constEnd(); ++it) {
			symbols += it.value();
			if (!SimulationCatalog::find(it.key())) missing << it.key();
		}
		if (funcs.isEmpty()) wrong << QStringLiteral("keine Fluidsymbole in der Sammlung gefunden (%1)").arg(where);
		if (!missing.isEmpty()) wrong << QStringLiteral("nicht im Katalog: %1").arg(missing.join(QStringLiteral(", ")));
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty()
			? QStringLiteral("%1; Sammlung %5: %2 Fluidsymbole mit %3 Funktionen, alle im Katalog %4")
				  .arg(seen.join(QStringLiteral(", "))).arg(symbols).arg(funcs.size()).arg(SimulationCatalog::version(), where)
			: wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 9. nets, numerics and events (R03): contradicting sources and mixed media stop the run before it
		//    starts, a numerically unstable net stops it with a diagnosis, simultaneous inputs give the same
		//    run in any order
		Check c{names.at(8)};
		QStringList wrong, seen;
		const QString hyd = path("hydraulik.qet");
		auto expectError = [&](const QString &file, const QString &what, const QString &expect, double duration) {
			if (file.isEmpty()) { wrong << what + QStringLiteral(": Variante nicht erzeugt"); return; }
			ProjectRun r = runProject(file, 6.0, {{0.3, QStringLiteral("-SF1"), 0, true}}, duration);
			if (!r.error.contains(expect)) wrong << QStringLiteral("%1: „%2“").arg(what, r.error.left(160));
			else seen << what;
			QFile::remove(file);
		};
		// the pump as an ideal pressure source holds its tank port at 6 bar against the tank at 0 bar
		expectError(functionVariant(hyd, QStringLiteral("supply.pump"), QStringLiteral("supply.air_supply"),
									QStringLiteral("qet_sim_r03_quellen.qet")),
					QStringLiteral("widersprüchliche Quellen"), QStringLiteral("widersprüchliche Quellen"), 0.05);
		{   // one hydraulic port marked pneumatic
			QFile f(hyd);
			QString file;
			if (f.open(QIODevice::ReadOnly)) {
				QString text = QString::fromUtf8(f.readAll());
				const int at = static_cast<int>(text.indexOf(QStringLiteral("medium=\"hydraulic\"")));
				if (at >= 0) {
					text.replace(at, 18, QStringLiteral("medium=\"pneumatic\""));
					file = QDir(QDir::tempPath()).filePath(QStringLiteral("qet_sim_r03_medium.qet"));
					QFile o(file);
					if (o.open(QIODevice::WriteOnly)) o.write(text.toUtf8()); else file.clear();
				}
			}
			expectError(file, QStringLiteral("Luft und Öl in einem Netz"), QStringLiteral("Pneumatik- und Hydraulikanschlüsse"), 0.05);
		}
		// a valve resistance far too small for the step: the run stops with the net instead of computing nonsense
		expectError(variant(hyd, QStringLiteral("rT"), QStringLiteral("value"), QStringLiteral("0.0001"),
							QStringLiteral("qet_sim_r03_instabil.qet")),
					QStringLiteral("instabiles Netz"), QStringLiteral("Numerisch instabil im Netz an"), 0.5);
		// simultaneous inputs in either order: the same run, step for step
		const QString sp = path("speicher_druckschalter_logik.qet");
		QVector<double> trace1, trace2;
		auto tracer = [](QVector<double> &tr) {
			return [&tr](SimulationModel &s) {
				for (const auto &cyl : s.cylinders()) tr << cyl.x_mm;
			};
		};
		ProjectRun a1 = runProject(sp, 6.0, {{0.1, QStringLiteral("-QM1"), 1, true}, {0.1, QStringLiteral("-QM2"), 1, true}}, 1.0, tracer(trace1));
		ProjectRun a2 = runProject(sp, 6.0, {{0.1, QStringLiteral("-QM2"), 1, true}, {0.1, QStringLiteral("-QM1"), 1, true}}, 1.0, tracer(trace2));
		if (!a1.error.isEmpty() || !a2.error.isEmpty()) wrong << a1.error + a2.error;
		else if (trace1 != trace2 || trace1.isEmpty()) wrong << QStringLiteral("gleichzeitige Eingaben: Reihenfolge ändert den Lauf");
		else seen << QStringLiteral("gleichzeitige Eingaben in beiden Reihenfolgen gleich (%1 Werte)").arg(trace1.size());
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? QStringLiteral("erkannt: %1").arg(seen.join(QStringLiteral(", "))) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 11. vacuum, semi-rotary and gripper drives (R10). Plan vakuum.qet: source -GP1, 3/2 valve -QM1,
		//     ejector -UG1 (best vacuum p_vac -0.08 MPa at 0.5 MPa, suction 6 l/min), suction cup -UG2
		//     (Ø 20 mm, 0.1 kg), pressure switch -BP1 on the vacuum net
		Check c{names.at(10)};
		QStringList wrong, seen;
		const QString vak = path("vakuum.qet");
		const double area = M_PI * 0.02 * 0.02 / 4, pu = -0.08e6;
		const QList<Input> grip{{0.1, QStringLiteral("-QM1"), 1, true}, {1.0, QStringLiteral("-QM1"), 1, false}};
		auto sample = [](double at, QHash<QString, double> &into, const QStringList &ids) {
			return [at, &into, ids](SimulationModel &s) {
				if (std::abs(s.time() - at) < 5e-6)
					for (const QString &id : ids) into.insert(id, s.signalValue(id));
			};
		};
		const QStringList ids{QStringLiteral("p:-UG2.1"), QStringLiteral("p:-UG1.1"), QStringLiteral("Fhold:-UG2"), QStringLiteral("hold:-UG2")};
		auto near = [](double a, double b, double rel) { return std::isfinite(a) && std::abs(a - b) <= rel * std::abs(b); };
		// a) tight cup: best vacuum scaled with the supply, holding force (p_atm - p)·A, release after closing
		for (double bar : {6.0, 3.5}) {
			QHash<QString, double> v;
			ProjectRun pr = runProject(vak, bar, grip, 1.5, sample(0.95, v, ids));
			if (!pr.error.isEmpty()) { wrong << QStringLiteral("Vakuum %1 bar: %2").arg(bar).arg(pr.error); continue; }
			const double share = std::min(1.0, v.value(ids.at(1)) / 0.5e6), expect = pu * share;
			const double force = -v.value(ids.at(0)) * area;
			const double release = pr.eventTime(QStringLiteral("hält das Werkstück nicht mehr"), 1.0);
			if (!near(v.value(ids.at(0)), expect, 0.005)) wrong << QStringLiteral("Vakuum bei %1 bar: %2 bar statt %3").arg(bar).arg(v.value(ids.at(0)) / 1e5).arg(expect / 1e5);
			else if (!near(v.value(ids.at(2)), force, 1e-6) || v.value(ids.at(3)) != 1.0) wrong << QStringLiteral("Haltekraft %1 N statt %2").arg(v.value(ids.at(2))).arg(force);
			else if (!(release > 1.0 && release < 1.4)) wrong << QStringLiteral("Loslassen nach dem Schließen fehlt (%1 s)").arg(release);
			else seen << QStringLiteral("%1 bar: %2 bar (soll %3), %4 N, lässt bei %5 s los").arg(bar, 0, 'f', 1)
							 .arg(v.value(ids.at(0)) / 1e5, 0, 'f', 3).arg(expect / 1e5, 0, 'f', 3).arg(v.value(ids.at(2)), 0, 'f', 2).arg(release, 0, 'f', 3);
		}
		const QString cup = QStringLiteral("<fluid function=\"vacuum.suction_cup\" kind=\"other\" prefix=\"MM\"");
		auto cupVariant = [&](const QString &params, const char *name) {
			return textVariant(vak, {{cup + QStringLiteral("/>"), cup + QStringLiteral(">") + params + QStringLiteral("</fluid>")}},
							   QString::fromLatin1(name));
		};
		// b) leaking contact: equilibrium of the linear suction characteristic and the leakage (qLeak
		//    1.8 l/min at -0.6 bar through the ISO 6358 orifice, b = 0.3), solved by bisection
		{
			const QString file = cupVariant(QStringLiteral("<parameter name=\"qLeak\" unit=\"l/min\" value=\"1.8\"/>"), "qet_sim_r10_leck.qet");
			QHash<QString, double> v;
			ProjectRun pr = runProject(file, 6.0, grip, 1.5, sample(0.95, v, ids));
			if (!pr.error.isEmpty()) wrong << QStringLiteral("Leckage: %1").arg(pr.error);
			else {
				const double patm = 101300.0, b = 0.3;
				auto iso = [b](double r) { return r <= b ? 1.0 : std::sqrt(std::max(0.0, 1.0 - std::pow((r - b) / (1.0 - b), 2))); };
				const double pmin = pu * std::min(1.0, v.value(ids.at(1)) / 0.5e6);
				auto excess = [&](double g) {   // suction minus leakage in l/min, g gauge
					return 6.0 * (g - pmin) / (-pmin) - 1.8 * iso((patm + g) / patm) / iso((patm - 0.6e5) / patm);
				};
				double lo = pmin, hi = 0.0;
				for (int i = 0; i < 100; ++i) { const double m = 0.5 * (lo + hi); (excess(m) > 0 ? hi : lo) = m; }
				if (!near(v.value(ids.at(0)), lo, 0.005)) wrong << QStringLiteral("Leckage: %1 bar statt %2").arg(v.value(ids.at(0)) / 1e5).arg(lo / 1e5);
				else seen << QStringLiteral("Leckage 1,8 l/min: %1 bar (Gleichgewicht %2)").arg(v.value(ids.at(0)) / 1e5, 0, 'f', 3).arg(lo / 1e5, 0, 'f', 3);
			}
			QFile::remove(file);
		}
		// c) no workpiece: the open cup builds no vacuum worth the name and holds nothing
		{
			const QString file = cupVariant(QStringLiteral("<parameter name=\"object\" value=\"0\"/>"), "qet_sim_r10_frei.qet");
			QHash<QString, double> v;
			ProjectRun pr = runProject(file, 6.0, grip, 1.5, sample(0.95, v, ids));
			if (!pr.error.isEmpty()) wrong << QStringLiteral("ohne Werkstück: %1").arg(pr.error);
			else if (v.value(ids.at(3)) != 0.0 || v.value(ids.at(2)) != 0.0 || !(v.value(ids.at(0)) > -0.1e5)
					 || std::isfinite(pr.eventTime(QStringLiteral("hält das Werkstück"))))
				wrong << QStringLiteral("ohne Werkstück: %1 bar, Haltekraft %2 N").arg(v.value(ids.at(0)) / 1e5).arg(v.value(ids.at(2)));
			else seen << QStringLiteral("ohne Werkstück offen (%1 bar), hält nicht").arg(v.value(ids.at(0)) / 1e5, 0, 'f', 3);
			QFile::remove(file);
		}
		// d) a known load: 2 kg holds from (p_atm - p)·A = m·g on; with the safety factor 1.5 it never holds
		for (const char *s : {"1", "1.5"}) {
			const QString file = cupVariant(QStringLiteral("<parameter name=\"m\" unit=\"kg\" value=\"2\"/><parameter name=\"s\" value=\"%1\"/>")
												.arg(QString::fromLatin1(s)), "qet_sim_r10_last.qet");
			double onset = std::nan("");
			bool was = false;
			ProjectRun pr = runProject(file, 6.0, grip, 1.5, [&](SimulationModel &m) {
				const bool h = m.signalValue(QStringLiteral("hold:-UG2")) == 1.0;
				if (h && !was && !std::isfinite(onset)) onset = m.signalValue(QStringLiteral("Fhold:-UG2"));
				was = h;
			});
			const bool safe = QString::fromLatin1(s) == QLatin1String("1.5");
			if (!pr.error.isEmpty()) wrong << QStringLiteral("Last: %1").arg(pr.error);
			else if (safe && std::isfinite(onset)) wrong << QStringLiteral("Last mit s = 1,5 gehalten (%1 N)").arg(onset);
			else if (!safe && !near(onset, 2 * 9.81, 0.01)) wrong << QStringLiteral("Last 2 kg: hält ab %1 N statt 19,62 N").arg(onset);
			else seen << (safe ? QStringLiteral("2 kg mit s = 1,5: hält nicht (max. %1 N < 29,43 N)").arg(-pu * area, 0, 'f', 2)
							   : QStringLiteral("2 kg hält ab %1 N (m·g 19,62 N)").arg(onset, 0, 'f', 2));
			QFile::remove(file);
		}
		// e) vacuum switch: p0 -50 kPa, hysteresis 10 kPa: on when the net falls below -0.5 bar, off above -0.4 bar
		{
			const QString f1 = variant(vak, QStringLiteral("p0"), QStringLiteral("value"), QStringLiteral("-0.05"), QStringLiteral("qet_sim_r10_vs1.qet"));
			const QString file = f1.isEmpty() ? QString() : variant(f1, QStringLiteral("p_hyst"), QStringLiteral("value"), QStringLiteral("0.01"), QStringLiteral("qet_sim_r10_vs.qet"));
			double t_down = std::nan(""), t_up = std::nan("");
			ProjectRun pr = runProject(file, 6.0, grip, 1.5, [&](SimulationModel &m) {
				const double g = m.signalValue(QStringLiteral("p:-BP1.1"));
				if (!std::isfinite(t_down) && g <= -0.5e5) t_down = m.time();
				if (std::isfinite(t_down) && !std::isfinite(t_up) && m.time() > 1.0 && g > -0.4e5) t_up = m.time();
			});
			const double on = pr.eventTime(QStringLiteral("Vakuumschalter -BP1 schaltet ein"));
			const double off = pr.eventTime(QStringLiteral("Vakuumschalter -BP1 schaltet aus"));
			if (!pr.error.isEmpty() || file.isEmpty()) wrong << QStringLiteral("Vakuumschalter: %1").arg(file.isEmpty() ? QStringLiteral("Variante fehlt") : pr.error);
			else if (!(std::abs(on - t_down) <= 2e-5) || !(std::abs(off - t_up) <= 2e-5))
				wrong << QStringLiteral("Vakuumschalter ein %1 s (Druck %2 s), aus %3 s (Druck %4 s)").arg(on).arg(t_down).arg(off).arg(t_up);
			else seen << QStringLiteral("Vakuumschalter ein bei -0,5 bar (%1 s), aus über -0,4 bar (%2 s)").arg(on, 0, 'f', 4).arg(off, 0, 'f', 4);
			QFile::remove(f1);
			QFile::remove(file);
		}
		// f) semi-rotary drive in place of the cylinder of zylinder_b_offen.qet: 90°, M_6bar 2 N·m
		const QString cyl = path("zylinder_b_offen.qet");
		auto rotary = [&](const QString &load, const char *name) {
			return textVariant(cyl, {{QStringLiteral("function=\"actuator.cyl_conf\""), QStringLiteral("function=\"actuator.semi_rotary\"")},
									 {QStringLiteral("<parameter name=\"xMax\" unit=\"mm\" value=\"100\"/>"),
									  QStringLiteral("<parameter name=\"xMax\" unit=\"deg\" value=\"90\"/>")},
									 {QStringLiteral("<parameter name=\"xStart\" unit=\"mm\" value=\"0\"/>"),
									  QStringLiteral("<parameter name=\"xStart\" unit=\"deg\" value=\"0\"/>")},
									 {QStringLiteral("<parameter name=\"F_extern_const\" unit=\"N\" value=\"0\"/>"),
									  QStringLiteral("<parameter name=\"M_ext\" unit=\"N.m\" value=\"%1\"/>").arg(load)}},
							   QString::fromLatin1(name));
		};
		{
			const QString file = rotary(QStringLiteral("0"), "qet_sim_r10_schwenk.qet");
			QHash<QString, double> v;
			const QStringList rid{QStringLiteral("x:-MM1"), QStringLiteral("F:-MM1")};
			ProjectRun pr = runProject(file, 6.0, {{0.2, QStringLiteral("-QM1"), 1, true}}, 1.5, sample(1.4, v, rid), true);
			const double end = pr.eventTime(QStringLiteral("-MM1 Endlage vorne"));
			if (!pr.error.isEmpty() || file.isEmpty()) wrong << QStringLiteral("Schwenkantrieb: %1").arg(file.isEmpty() ? QStringLiteral("Variante fehlt") : pr.error);
			else if (!(end > 0.2) || std::abs(v.value(rid.at(0)) - M_PI / 2) > 1e-9 || !near(v.value(rid.at(1)), 2.0, 0.01))
				wrong << QStringLiteral("Schwenkantrieb: Endlage %1 s, Winkel %2 rad, Moment %3 N·m").arg(end).arg(v.value(rid.at(0))).arg(v.value(rid.at(1)));
			else seen << QStringLiteral("Schwenkantrieb 90° in %1 s, Moment in der Endlage %2 N·m (M_6bar 2)").arg(end - 0.2, 0, 'f', 3).arg(v.value(rid.at(1)), 0, 'f', 3);
			QFile::remove(file);
		}
		{   // a load torque above the drive torque at 6 bar: it does not turn
			const QString file = rotary(QStringLiteral("3"), "qet_sim_r10_schwenk_last.qet");
			QHash<QString, double> v;
			ProjectRun pr = runProject(file, 6.0, {{0.2, QStringLiteral("-QM1"), 1, true}}, 1.5, sample(1.4, v, {QStringLiteral("x:-MM1")}));
			if (!pr.error.isEmpty()) wrong << QStringLiteral("Schwenkantrieb mit Last: %1").arg(pr.error);
			else if (!(v.value(QStringLiteral("x:-MM1")) < 1e-6)) wrong << QStringLiteral("Schwenkantrieb dreht gegen 3 N·m (%1 rad)").arg(v.value(QStringLiteral("x:-MM1")));
			else seen << QStringLiteral("gegen 3 N·m Last steht er");
			QFile::remove(file);
		}
		{   // gripper (substitute model): closes against its return spring
			const QString file = functionVariant(cyl, QStringLiteral("actuator.cyl_conf"), QStringLiteral("actuator.gripper"), QStringLiteral("qet_sim_r10_greifer.qet"));
			ProjectRun pr = runProject(file, 6.0, {{0.2, QStringLiteral("-QM1"), 1, true}}, 1.5);
			if (!pr.error.isEmpty()) wrong << QStringLiteral("Greifer: %1").arg(pr.error);
			else if (!std::isfinite(pr.eventTime(QStringLiteral("-MM1 Endlage vorne")))) wrong << QStringLiteral("Greifer schließt nicht");
			else seen << QStringLiteral("Greifer schließt (%1 s)").arg(pr.eventTime(QStringLiteral("-MM1 Endlage vorne")) - 0.2, 0, 'f', 3);
			QFile::remove(file);
		}
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 10. hydraulic motor (R08) with the shaft model of the core (SimPhysics::Shaft), fed by an ideal flow
		//     source into an oil volume; GLEICHUNGEN 5: D 10 cm³/U, 15 l/min -> 1500 1/min; 100 bar <-> 15.9155 N·m.
		//     A viscous friction b damps the oil spring (else it rings forever); it is in the hand calculation.
		Check c{names.at(9)};
		QStringList wrong, seen;
		auto run = [](double (*q_in)(double), double (*tau_load)(double), double cleak, double b, double tau_c,
					  double t_end, double *dp_out, double *energy_error, double *e_in) {
			SimPhysics::Shaft s;
			s.D = 10e-6; s.alpha = 1.0; s.cleak = cleak; s.J = 1e-3; s.b = b; s.tau_c = tau_c;
			double pa = 0.0, cav = 0.0;
			const double dt = 1e-5, vol = 1e-4, bulk = 5e8;
			double win = 0, wload = 0, wfric = 0, wleak = 0;
			const double ek0 = 0.5 * s.J * s.omega * s.omega;
			const long long n = std::llround(t_end / dt);
			for (long long i = 0; i < n; ++i) {
				const double t = i * dt;
				const double dp = pa;                                   // B at the tank, 0 Pa gauge
				const double q = s.flow(dp);
				const double w0 = s.omega;
				const double tl = tau_load(t);
				const double fr = s.friction();
				s.step(dp, tl, dt);
				const double wm = 0.5 * (w0 + s.omega);
				win += dp * q * dt;
				wload += tl * wm * dt;
				wfric += fr * wm * dt;
				wleak += cleak * dp * dp * dt;
				pa = SimPhysics::oilNode(pa, vol, 0.0, q_in(t) - q, bulk, dt, -0.9e5, cav);
			}
			*dp_out = pa;
			const double dek = 0.5 * s.J * s.omega * s.omega - ek0;
			// the flow is taken at the start of the step: the balance holds to the order of dt
			*energy_error = std::abs(win - (wload + dek + wfric + wleak));
			*e_in = win;
			return s.omega;
		};
		const double D = 10e-6, b = 0.05, tau100 = D * 1e7 / (2 * M_PI), w_set = 2 * M_PI * 25.0;
		double dp = 0, err = 0, ein = 0;
		const double w1 = run([](double) { return 2.5e-4; }, [](double) { return 10e-6 * 1e7 / (2 * M_PI); }, 0, b, 0, 1.0, &dp, &err, &ein);
		const double dp1 = 2 * M_PI * (tau100 + b * w_set) / D;
		if (std::abs(w1 - w_set) > 0.005 * w_set || std::abs(dp - dp1) > 0.005 * dp1)
			wrong << QStringLiteral("stationär %1 1/min bei %2 bar (soll 1500, %3)").arg(w1 / (2 * M_PI) * 60).arg(dp / 1e5).arg(dp1 / 1e5);
		else seen << QStringLiteral("15 l/min → %1 1/min (soll 1500), %2 bar (soll %3: 100 bar für 15,9155 N·m + Reibung)")
						 .arg(w1 / (2 * M_PI) * 60, 0, 'f', 1).arg(dp / 1e5, 0, 'f', 2).arg(dp1 / 1e5, 0, 'f', 2);
		// load step: twice the torque from 0.5 s -> the pressure rises by 100 bar, the speed stays (flow fed)
		const double w2 = run([](double) { return 2.5e-4; }, [](double t) { return (t < 0.5 ? 1.0 : 2.0) * 10e-6 * 1e7 / (2 * M_PI); },
							  0, b, 0, 1.5, &dp, &err, &ein);
		const double dp2 = dp1 + 1e7;
		if (std::abs(w2 - w_set) > 0.005 * w_set || std::abs(dp - dp2) > 0.005 * dp2) wrong << QStringLiteral("Lastsprung %1 bar").arg(dp / 1e5);
		else seen << QStringLiteral("Lastsprung: %1 bar (soll %2), Drehzahl bleibt").arg(dp / 1e5, 0, 'f', 2).arg(dp2 / 1e5, 0, 'f', 2);
		// reversal: the supply changes to port B (A to the tank, as a reversing valve does): the shaft turns
		// backwards. (Sucking the flow out of A instead only reaches the cavitation limit: no reversal.)
		double w3 = 0.0;
		{
			SimPhysics::Shaft s;
			s.D = D; s.J = 1e-3; s.b = b;
			double pa = 0.0, pb = 0.0, ca = 0.0, cb = 0.0;
			const double dt = 1e-5;
			for (long long i = 0; i < 150000; ++i) {
				const bool back = i >= 50000;
				const double dpm = pa - pb, q = s.flow(dpm);
				s.step(dpm, 0.0, dt);
				pa = back ? 0.0 : SimPhysics::oilNode(pa, 1e-4, 0.0, 2.5e-4 - q, 5e8, dt, -0.9e5, ca);
				pb = back ? SimPhysics::oilNode(pb, 1e-4, 0.0, 2.5e-4 + q, 5e8, dt, -0.9e5, cb) : 0.0;
			}
			w3 = s.omega;
		}
		if (std::abs(w3 + w_set) > 0.005 * w_set) wrong << QStringLiteral("Umkehr %1 1/min").arg(w3 / (2 * M_PI) * 60);
		else seen << QStringLiteral("Umkehr (Zulauf B): %1 1/min").arg(w3 / (2 * M_PI) * 60, 0, 'f', 1);
		// power balance with leakage, viscous and dry friction and a load
		run([](double) { return 2.5e-4; }, [](double) { return 10.0; }, 1e-12, b, 0.2, 1.0, &dp, &err, &ein);
		if (!(err <= 0.01 * ein)) wrong << QStringLiteral("Leistungsbilanz Rest %1 J von %2 J").arg(err).arg(ein);
		else seen << QStringLiteral("Leistungsbilanz Δp·q = Last + ΔEkin + Reibung + Leckage, Rest %1 J von %2 J").arg(err, 0, 'g', 2).arg(ein, 0, 'f', 1);
		// end to end: the cylinder of the hydraulic project as a motor; speed = pump flow / displacement
		const QString file = functionVariant(path("hydraulik.qet"), QStringLiteral("actuator.cyl_conf"), QStringLiteral("motor.fixed_motor"),
											 QStringLiteral("qet_sim_r08_motor.qet"));
		double omega = std::nan(""), expect = std::nan("");
		ProjectRun pr = runProject(file, 6.0, {{0.3, QStringLiteral("-SF1"), 0, true}}, 1.5, [&](SimulationModel &s) {
			if (std::abs(s.time() - 1.4) < 5e-6) omega = s.signalValue(QStringLiteral("omega:-MM1"));
		});
		if (!pr.error.isEmpty()) wrong << QStringLiteral("Projekt: %1").arg(pr.error);
		else {
			for (Diagram *d : pr.project->diagrams())
				for (Element *e : d->elements())
					if (e->elementData().m_fluid.m_function == QLatin1String("supply.pump"))
						expect = 2 * M_PI * symbolValue(e, QStringLiteral("displacement"), QStringLiteral("m3"))
							   * symbolValue(e, QStringLiteral("rpm"), QStringLiteral("1/s")) / 8e-6;
			if (!(std::abs(std::abs(omega) - expect) <= 0.02 * expect))
				wrong << QStringLiteral("Projekt: ω %1 rad/s statt %2").arg(omega).arg(expect);
			else seen << QStringLiteral("Projekt: %1 rad/s = Förderstrom/Schluckvolumen (%2)").arg(std::abs(omega), 0, 'f', 1).arg(expect, 0, 'f', 1);
		}
		QFile::remove(file);
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 8. measurement contract (R01): reading does not change the run, flows balance over a part,
		//    the pump delivers displacement · speed below its limit, an unknown id gives no value
		Check c{names.at(7)};
		QStringList wrong;
		const QString hyd = path("hydraulik.qet");
		const QList<Input> in{{0.3, QStringLiteral("-SF1"), 0, true}};
		ProjectRun plain = runProject(hyd, 6.0, in, 0.6);
		double worst_balance = 0.0, pump_q = std::nan(""), valve_flow = 0.0;
		double expected_q = std::nan("");
		bool unknown_ok = true;
		ProjectRun probed = runProject(hyd, 6.0, in, 0.6, [&](SimulationModel &s) {
			// every signal read in every step: must not change anything
			QStringList qs;
			for (const auto &sig : s.signalList()) {
				bool ok = false;
				const double v = s.signalValue(sig.id, &ok);
				Q_UNUSED(v);
				if (sig.id.startsWith(QLatin1String("q:-QM1."))) qs << sig.id;
			}
			double sum = 0.0, mag = 0.0;
			for (const QString &id : qs) { const double v = s.signalValue(id); sum += v; mag += std::abs(v); }
			worst_balance = std::max(worst_balance, std::abs(sum));
			valve_flow = std::max(valve_flow, mag / 2);
			if (std::abs(s.time() - 0.45) < 5e-6) pump_q = -s.signalValue(QStringLiteral("q:-GP1.P"));
			bool ok = true;
			s.signalValue(QStringLiteral("p:-GIBT.ES_NICHT"), &ok);
			unknown_ok = unknown_ok && !ok;
		}, true);
		if (!plain.error.isEmpty() || !probed.error.isEmpty()) {
			c.detail = plain.error + probed.error;
		} else {
			const double x1 = plain.cylinder(QStringLiteral("-MM1")).x_mm, x2 = probed.cylinder(QStringLiteral("-MM1")).x_mm;
			if (x1 != x2) wrong << QStringLiteral("Abgriff verändert den Lauf: Weg %1 statt %2 mm").arg(x2).arg(x1);
			for (Diagram *d : probed.project->diagrams())
				for (Element *e : d->elements())
					if (e->elementData().m_fluid.m_function == QLatin1String("supply.pump"))
						expected_q = symbolValue(e, QStringLiteral("displacement"), QStringLiteral("m3"))
								   * symbolValue(e, QStringLiteral("rpm"), QStringLiteral("1/s"));
			if (!(valve_flow > 0)) wrong << QStringLiteral("kein Durchfluss am Ventil gemessen");
			else if (worst_balance > 1e-9 * valve_flow + 1e-15)
				wrong << QStringLiteral("Ventilbilanz %1 m³/s bei %2 m³/s Durchfluss").arg(worst_balance).arg(valve_flow);
			if (!(std::abs(pump_q - expected_q) <= 1e-6 * std::abs(expected_q)))
				wrong << QStringLiteral("Pumpe %1 m³/s statt %2 m³/s").arg(pump_q).arg(expected_q);
			if (!unknown_ok) wrong << QStringLiteral("unbekannte Messgröße lieferte einen Wert");
			c.passed = wrong.isEmpty();
			c.detail = wrong.isEmpty()
				? QStringLiteral("Weg mit und ohne Abgriff gleich (%1 mm), Ventilbilanz ≤ %2 m³/s bei %3 l/min, Pumpe %4 l/min "
								 "= Verdrängung · Drehzahl, unbekannte Kennung ohne Wert")
					  .arg(x1, 0, 'f', 3).arg(worst_balance, 0, 'g', 2).arg(valve_flow * 60000, 0, 'f', 2).arg(pump_q * 60000, 0, 'f', 3)
				: wrong.join(QStringLiteral("; "));
		}
		out << c;
	}
	return out;
}

using Curve = QVector<QPair<double, double>>;

	/**
		tube comparison (as csv-compare and the Modelica test tools use it):
		how far a point of @a a lies outside the tube around @a b, the tube
		being every point of b within ±@a dt in time widened by
		±max(@a rel·|b|, @a abs) in value. <= 0: every point of a is inside.
		A point of a with no point of b in its time window counts as outside.
		Run both ways (tubeBoth): a spike in either curve then fails.
	*/
	/// how far the points of @a a lie outside the tube around curve @a b: time ±dt, value ±max(rel·|b|, abs).
	/// @a b is a sampled continuous curve: between two samples it runs straight, so every value on a piece
	/// of it inside the time window counts (a fast drop between two samples is not a false excess).
double tubeExcess(const Curve &a, const Curve &b, double dt, double rel, double abs, double *at)
{
	if (b.isEmpty()) return std::numeric_limits<double>::infinity();
	double worst = -std::numeric_limits<double>::infinity();
	const double t0 = b.first().first, t1 = b.last().first;
	int lo = 0;
	auto valueAt = [](const QPair<double, double> &p, const QPair<double, double> &q, double t) {
		if (q.first <= p.first) return p.second;
		return p.second + (q.second - p.second) * (t - p.first) / (q.first - p.first);
	};
	for (const auto &pa : a) {
		if (pa.first < t0 || pa.first > t1) continue;          // compare where both curves exist
		while (lo > 0 && b.at(lo).first > pa.first - dt) --lo;
		while (lo + 1 < b.size() && b.at(lo + 1).first < pa.first - dt) ++lo;
		double best = std::numeric_limits<double>::infinity();
		for (int j = lo; j < b.size() && b.at(j).first <= pa.first + dt; ++j) {
			// the piece from sample j to j+1, cut to the window [pa.t - dt, pa.t + dt]
			const auto &p = b.at(j);
			const auto &q = j + 1 < b.size() ? b.at(j + 1) : b.at(j);
			const double s0 = std::max(p.first, pa.first - dt), s1 = std::min(q.first, pa.first + dt);
			if (s1 < s0) continue;
			const double v0 = valueAt(p, q, s0), v1 = valueAt(p, q, s1);
			const double lo_v = std::min(v0, v1), hi_v = std::max(v0, v1);
			const double nearest = std::clamp(pa.second, lo_v, hi_v);
			const double tol = std::max(rel * std::abs(nearest), abs);
			best = std::min(best, std::abs(pa.second - nearest) - tol);
		}
		if (best > worst) {
			worst = best;
			if (at) *at = pa.first;
		}
	}
	return worst;
}

double tubeBoth(const Curve &ours, const Curve &ref, double dt, double rel, double abs, double *at)
{
	double t_a = 0.0, t_b = 0.0;
	const double e1 = tubeExcess(ours, ref, dt, rel, abs, &t_a);
	const double e2 = tubeExcess(ref, ours, dt, rel, abs, &t_b);
	if (at) *at = e1 >= e2 ? t_a : t_b;
	return std::max(e1, e2);
}

Curve column(const QVector<QVector<double>> &rows, int col)
{
	Curve c;
	for (const auto &r : rows) c << qMakePair(r.at(0), r.at(col));
	return c;
}

	/// tube check of the curves @a cols (column, name, rel, abs, display scale, unit); @return texts, @a ok cleared on failure
QStringList tubeReport(const QVector<QVector<double>> &own, const QVector<QVector<double>> &ref, double dt,
					   const QList<std::tuple<int, QString, double, double, double, QString>> &cols, bool *ok)
{
	QStringList out;
	for (const auto &k : cols) {
		double at_t = 0.0;
		const double excess = tubeBoth(column(own, std::get<0>(k)), column(ref, std::get<0>(k)), dt,
									   std::get<2>(k), std::get<3>(k), &at_t);
		if (!(excess <= 0.0)) *ok = false;
		out << (excess <= 0.0 ? QStringLiteral("%1 im Schlauch").arg(std::get<1>(k))
							  : QStringLiteral("%1 %2 %3 außerhalb bei %4 s ✗").arg(std::get<1>(k))
									.arg(excess * std::get<4>(k), 0, 'g', 4).arg(std::get<5>(k)).arg(at_t, 0, 'f', 4));
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
			QVector<Sample> fine;                  // every step in the end-stop window 1.570 … 1.600 s
			double balance = 0.0, pmin = 0.0, pmax = 0.0;
			double cav_a = 0.0, cav_b = 0.0, cav_s = 0.0, cav_max = 0.0;
			// in the window: work of the contact force, penetration, energy of the contact
			// (spring energy at start and end, energy taken out by the stop's damping)
			double contact_work = 0.0, penetration = 0.0, spring0 = -1.0, spring1 = 0.0, dissipated = 0.0;
		};
		auto run = [&](double dt) {
			Run r;
			const double bulk = 5e8, aa = 1e-3, ab = 7e-4, dead = 1e-4, floor = -0.9e5;
			Piston z;
			z.m = 50.0; z.stroke = 0.1; z.f_static = 20.0; z.f_slide = 20.0; z.damping = 5000.0; z.x = 0.02;
			// the reference's end stop (R02): spring 1e9 N/m, one-sided damping 1e5 N·s/m
			z.stop_k = 1e9; z.stop_c = 1e5;
			double ps = 0.0, pa = 0.0, pb = 0.0, spool = 0.0;
			r.pmin = 0.0; r.pmax = 0.0;
			const long long n = std::llround(1.8 / dt), every = std::llround(0.001 / dt);
			for (long long i = 0; i <= n; ++i) {
				const double t = i * dt;
				if (i % every == 0) r.s << Sample{t, z.x, pa, pb};
				const bool window = t >= 1.570 - 1e-12 && t <= 1.600 + 1e-12;
				auto spring = [&]() {
					const double pen = std::max({0.0, -z.x, z.x - z.stroke});
					return 0.5 * z.stop_k * pen * pen;
				};
				if (window) {
					if (r.spring0 < 0) r.spring0 = spring();
					r.fine << Sample{t, z.x, pa, pb};
					r.spring1 = spring();
				}
				if (i == n) break;
				const double cmd = t < 0.7 ? 1.0 : (t < 0.9 ? 0.0 : -1.0);
				// the spool moves like SimulationModel's valves, a first-order servo with tau = switching
				// time / 3; a switching time of 30 ms gives tau = 10 ms, the reference spool
				constexpr double TAU_SPOOL = 0.03 / 3.0;
				spool += (cmd - spool) * (1.0 - std::exp(-dt / TAU_SPOOL));
				const double open_p = std::max(spool, 0.0), open_m = std::max(-spool, 0.0);
				// pump 1e-4 m³/s, built-in pressure limit 5.1 MPa (full flow up to 5.0 MPa, as the reference relief valve)
				const double q_pump = 1e-4 * qBound(0.0, (5.1e6 - ps) / 1e5, 1.0);
				const double q_pa = open_p * SimPhysics::orificeFlow(ps - pa, kOrifice, DP_SMOOTH);
				const double q_bt = open_p * SimPhysics::orificeFlow(pb, kOrifice, DP_SMOOTH);
				const double q_pb = open_m * SimPhysics::orificeFlow(ps - pb, kOrifice, DP_SMOOTH);
				const double q_at = open_m * SimPhysics::orificeFlow(pa, kOrifice, DP_SMOOTH);
				const double v_before = z.v, x_before = z.x;
				const double contact = SimPhysics::stopForce(z.x, z.v, z.stroke, z.stop_k, z.stop_c);
				SimPhysics::pistonStep(z, pa * aa - pb * ab, dt);
				if (window) {
					const double v_mid = 0.5 * (v_before + z.v);
					r.contact_work += contact * v_mid * dt;
					// the damping part of the contact force acts only while the piston moves into the stop
					const bool into = (x_before < 0 && v_before < 0) || (x_before > z.stroke && v_before > 0);
					if (into) r.dissipated += z.stop_c * std::abs(v_before) * std::abs(v_mid) * dt;
					r.penetration = std::max({r.penetration, -x_before, x_before - z.stroke});
				}
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
		// the elastic stop lets the piston enter by a fraction of a millimetre (reference 0.09 mm)
		bool ok = xmin >= -0.0005 && xmax <= 0.1005 && at(a.s, 0.699) >= 0.075 && at(a.s, 1.75) < 0.002
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
				// whole curves, switching transients, impacts and peaks included: tube ±5 ms, pressure
				// ±10 % or 1 bar, stroke ±2 mm, both ways (a spike in either curve fails). Since R02 the
				// end stop is the reference's contact model (spring and one-sided damping), so the impact
				// windows are no longer left out; they are reported on their own (peak, its time).
				QVector<QPair<double, double>> impacts;
				for (int i = 1; i < ref.size(); ++i) {
					const double x0 = ref.at(i - 1).at(1), x1 = ref.at(i).at(1);
					const double t = ref.at(i).at(0);
					// the first touch of an end only; the bounces after it stay in the tube
					if (((x0 > 0.0 && x1 <= 0.0) || (x0 < 0.1 && x1 >= 0.1))
						&& (impacts.isEmpty() || t > impacts.last().first + 0.05))
						impacts << qMakePair(t, t + 0.010);
				}
				const QStringList tubes = tubeReport(own, ref, 0.005, {
					std::make_tuple(1, QStringLiteral("Weg"), 0.0, 0.002, 1000.0, QStringLiteral("mm")),
					std::make_tuple(2, QStringLiteral("Druck A"), 0.10, 1e5, 1e-5, QStringLiteral("bar")),
					std::make_tuple(3, QStringLiteral("Druck B"), 0.10, 1e5, 1e-5, QStringLiteral("bar"))}, &same);
				// impact windows: pressure peak and its time, ours against the reference
				QStringList excluded;
				for (const auto &w : impacts) {
					double own_peak = 0.0, ref_peak = 0.0, own_t = 0.0, ref_t = 0.0;
					for (const auto &row : ref)
						if (row.at(0) >= w.first && row.at(0) <= w.second)
							for (int col : {2, 3}) if (std::abs(row.at(col)) > std::abs(ref_peak)) { ref_peak = row.at(col); ref_t = row.at(0); }
					for (const auto &row : own)
						if (row.at(0) >= w.first && row.at(0) <= w.second)
							for (int col : {2, 3}) if (std::abs(row.at(col)) > std::abs(own_peak)) { own_peak = row.at(col); own_t = row.at(0); }
					const bool fits = std::abs(own_peak - ref_peak) <= std::max(0.1 * std::abs(ref_peak), 1e5)
									  && std::abs(own_t - ref_t) <= 0.005;
					same = same && fits;
					excluded << QStringLiteral("Anschlag %1 s: Spitze %2 bar bei %3 s (Ref. %4 bar bei %5 s)%6")
						.arg(w.first, 0, 'f', 4).arg(own_peak / 1e5, 0, 'f', 1).arg(own_t, 0, 'f', 4)
						.arg(ref_peak / 1e5, 0, 'f', 1).arg(ref_t, 0, 'f', 4).arg(fits ? QString() : QStringLiteral(" ✗"));
				}
				v.passed = same;
				v.detail = QStringLiteral("Ausfahren %1 mm/s (Ref. %2), Einfahren %3 mm/s (Ref. %4), zurück bei %5 s (Ref. %6); "
										  "Kammerdrücke %7; ganze Kurven ohne Ausnahme: %8; Anschläge (Kontaktmodell wie Referenz): %9; Grenzen ±3 % Geschwindigkeit, ±5 % "
										  "Rückhubzeit, Druck ±10 % bzw. 1 bar (Mittel und Schlauch), Weg ±2 mm, Zeit ±5 ms")
							   .arg(v_out * 1000, 0, 'f', 2).arg(rv_out * 1000, 0, 'f', 2)
							   .arg(v_in * 1000, 0, 'f', 2).arg(rv_in * 1000, 0, 'f', 2)
							   .arg(t_back, 0, 'f', 3).arg(r_back, 0, 'f', 3)
							   .arg(pressures.join(QStringLiteral(", ")), tubes.join(QStringLiteral(", ")),
									excluded.isEmpty() ? QStringLiteral("keiner") : excluded.join(QStringLiteral(", ")));
			}
			checks << v;
		}

		// R02: the end stop against the OpenModelica run at 10 µs (same contact model and parameters).
		// Limits fixed before the comparison: whole curves in the window in a tube of ±0.5 ms with pressure
		// ±10 % or 1 bar and stroke ±0.05 mm, pressure peak ±5 % at ±1 ms, penetration ±0.02 mm, contact
		// work ±20 % or 0.1 J, contact energy balance (−work = Δ spring energy + damping loss) to 5 %,
		// peak within 2 % at half the step.
		const QString stop_name = QStringLiteral("Endanschlag hydraulisch (R02): Spitze, Nachschwingen, Eindringung, Kontaktarbeit");
		const QString fine_file = reference_dir.isEmpty() ? QString()
			: QDir(reference_dir).filePath(QStringLiteral("OwnHydraulics.Examples.CylinderCycle_fein.csv"));
		if (fine_file.isEmpty() || !QFileInfo::exists(fine_file)) {
			checks << skip(stop_name, QStringLiteral("feiner Referenzlauf fehlt (erzeuge_hydraulik_fein.py)"));
		} else {
			QVector<QVector<double>> ref;
			const QString error = readResult(fine_file, {QStringLiteral("cylinder.x"), QStringLiteral("cylinder.pressureA"),
				QStringLiteral("cylinder.pressureB"), QStringLiteral("cylinder.v"), QStringLiteral("cylinder.contact")}, 1.8, &ref);
			Check s{stop_name};
			if (!error.isEmpty()) {
				s.detail = error;
			} else {
				double t0 = std::nan("");
				for (int i = 1; i < ref.size() && std::isnan(t0); ++i)
					if (ref.at(i - 1).at(1) > 0.0 && ref.at(i).at(1) <= 0.0 && ref.at(i).at(0) > 1.0) t0 = ref.at(i).at(0);
				QVector<QVector<double>> rw, ow;
				double r_peak = 0, r_peak_t = 0, r_pen = 0, r_work = 0;
				for (int i = 0; i < ref.size(); ++i) {
					const auto &row = ref.at(i);
					if (!(row.at(0) >= t0 && row.at(0) <= t0 + 0.020)) continue;
					rw << QVector<double>{row.at(0), row.at(1), row.at(2), row.at(3)};
					for (int col : {2, 3}) if (std::abs(row.at(col)) > std::abs(r_peak)) { r_peak = row.at(col); r_peak_t = row.at(0); }
					r_pen = std::max(r_pen, -row.at(1));
					if (i + 1 < ref.size()) r_work += row.at(5) * row.at(4) * (ref.at(i + 1).at(0) - row.at(0));
				}
				// both runs from their own arrival at the end: the contact itself is compared; the arrival
				// differs by the small speed difference summed over the stroke (reported, limit 1 ms)
				auto arrival = [](const QVector<Sample> &f) {
					for (int i = 1; i < f.size(); ++i)
						if (f.at(i - 1).x > 0.0 && f.at(i).x <= 0.0) return f.at(i).t;
					return std::nan("");
				};
				const double o0 = arrival(a.fine), o0b = arrival(b.fine);
				const double shift = t0 - o0;                       // own time + shift = reference time
				double o_peak = 0, o_peak_t = 0, o2_peak = 0;
				for (const Sample &q : a.fine)
					if (q.t >= o0 && q.t <= o0 + 0.020) {
						ow << QVector<double>{q.t + shift, q.x, q.pa, q.pb};
						for (double pv : {q.pa, q.pb}) if (std::abs(pv) > std::abs(o_peak)) { o_peak = pv; o_peak_t = q.t + shift; }
					}
				for (const Sample &q : b.fine)
					if (q.t >= o0b && q.t <= o0b + 0.020)
						for (double pv : {q.pa, q.pb}) if (std::abs(pv) > std::abs(o2_peak)) o2_peak = pv;
				bool ok = !std::isnan(t0) && !std::isnan(o0) && !rw.isEmpty() && !ow.isEmpty() && std::abs(shift) <= 0.001;
				const QStringList tubes = tubeReport(ow, rw, 0.0005, {
					std::make_tuple(1, QStringLiteral("Weg"), 0.0, 0.00005, 1000.0, QStringLiteral("mm")),
					std::make_tuple(2, QStringLiteral("Druck A"), 0.10, 1e5, 1e-5, QStringLiteral("bar")),
					std::make_tuple(3, QStringLiteral("Druck B"), 0.10, 1e5, 1e-5, QStringLiteral("bar"))}, &ok);
				const bool peak_ok = std::abs(o_peak - r_peak) <= 0.05 * std::abs(r_peak) && std::abs(o_peak_t - r_peak_t) <= 0.001;
				const bool pen_ok = std::abs(a.penetration - r_pen) <= 2e-5;
				const bool work_ok = std::abs(a.contact_work - r_work) <= std::max(0.2 * std::abs(r_work), 0.1);
				// energy of the contact: what the contact force took out of the piston is stored in the spring or
				// lost in the stop's damping (to 5 %; the damping never pushes, so the loss is ≥ 0)
				const double stored = a.spring1 - std::max(a.spring0, 0.0);
				const double balance = -a.contact_work - (stored + a.dissipated);
				const bool energy_ok = a.dissipated >= 0 && std::abs(balance) <= 0.05 * std::max(std::abs(a.contact_work), 1e-3);
				const bool conv_ok = std::abs(o2_peak - o_peak) <= 0.02 * std::abs(o_peak);
				ok = ok && peak_ok && pen_ok && work_ok && energy_ok && conv_ok;
				s.passed = ok;
				s.detail = QStringLiteral("Ankunft am Hubende %22 s (Ref. %1 s, Unterschied %23 ms, Grenze 1 ms), Vergleich ab der "
										  "jeweiligen Ankunft 20 ms bis %2 s; %3; Spitze %4 bar bei %5 s (Ref. %6 bar bei %7 s)%8; Eindringung %9 mm "
										  "(Ref. %10 mm)%11; Kontaktarbeit %12 J (Ref. %13 J)%14; Kontaktbilanz: −Arbeit %15 J = "
										  "Federenergie %16 J + Dämpfung %17 J, Rest %18 J%19; halber Schritt: Spitze %20 bar%21")
							   .arg(t0, 0, 'f', 4).arg(t0 + 0.020, 0, 'f', 4).arg(tubes.join(QStringLiteral(", ")))
							   .arg(o_peak / 1e5, 0, 'f', 2).arg(o_peak_t, 0, 'f', 5).arg(r_peak / 1e5, 0, 'f', 2).arg(r_peak_t, 0, 'f', 5)
							   .arg(peak_ok ? QString() : QStringLiteral(" ✗"))
							   .arg(a.penetration * 1000, 0, 'f', 4).arg(r_pen * 1000, 0, 'f', 4)
							   .arg(pen_ok ? QString() : QStringLiteral(" ✗"))
							   .arg(a.contact_work, 0, 'f', 3).arg(r_work, 0, 'f', 3).arg(work_ok ? QString() : QStringLiteral(" ✗"))
							   .arg(-a.contact_work, 0, 'f', 4).arg(stored, 0, 'f', 4).arg(a.dissipated, 0, 'f', 4)
							   .arg(balance, 0, 'g', 2).arg(energy_ok ? QString() : QStringLiteral(" ✗"))
							   .arg(o2_peak / 1e5, 0, 'f', 2).arg(conv_ok ? QString() : QStringLiteral(" ✗"))
							   .arg(o0, 0, 'f', 4).arg(-shift * 1000, 0, 'f', 2);
			}
			checks << s;
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
				// travel time within 5 % of the stroke time, every 1-ms point: stroke 1 mm, pressure 0.1 bar;
				// every reference row (events and off-grid rows too): tube ±2 ms, stroke 1 mm, pressure 0.1 bar
				const double tol_out = 0.05 * r_out, tol_back = 0.05 * (r_back - 0.4);
				bool same = compared >= 900 && std::abs(t_out - r_out) <= tol_out && std::abs(t_back - r_back) <= tol_back
							&& dx <= 0.001 && dp <= 0.1e5;
				QVector<QVector<double>> own;
				for (const Sample &s : a) own << QVector<double>{s.t, s.x, s.pa, s.pb};
				const QStringList tubes = tubeReport(own, ref, 0.002, {
					std::make_tuple(1, QStringLiteral("Weg"), 0.0, 0.001, 1000.0, QStringLiteral("mm")),
					std::make_tuple(2, QStringLiteral("Druck A"), 0.0, 0.1e5, 1e-5, QStringLiteral("bar")),
					std::make_tuple(3, QStringLiteral("Druck B"), 0.0, 0.1e5, 1e-5, QStringLiteral("bar"))}, &same);
				v.passed = same;
				v.detail = QStringLiteral("aus bei %1 s (Ref. %2), zurück bei %3 s (Ref. %4), Grenze %5 / %6 ms; "
										  "an %7 Zeitpunkten größte Abweichung Weg %8 mm (Grenze 1), Druck %9 bar (Grenze 0,1); "
										  "ganze Kurven ±2 ms: %10")
							   .arg(t_out, 0, 'f', 3).arg(r_out, 0, 'f', 3).arg(t_back, 0, 'f', 3).arg(r_back, 0, 'f', 3)
							   .arg(tol_out * 1000, 0, 'f', 1).arg(tol_back * 1000, 0, 'f', 1).arg(compared)
							   .arg(dx * 1000, 0, 'f', 2).arg(dp / 1e5, 0, 'f', 3)
							   .arg(tubes.join(QStringLiteral(", ")));
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
