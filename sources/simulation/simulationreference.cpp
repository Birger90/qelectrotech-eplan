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

#include <array>

#include "grafcet.h"
#include <QElapsedTimer>
#include "plcprogram.h"
#include "simulationcatalog.h"
#include "simulationmodel.h"
#include "simulationsignals.h"
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
#include <QDomDocument>
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

struct Input { double t; QString label; int kind; bool on; double value = 0.0; };   // kind 0 push button, 1 valve left, 2 valve right, 3 setpoint [V], 4 repair (R15)

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
			else if (in.kind == 3) r.sim->setSetpoint(in.label, in.value);
			else if (in.kind == 4) r.sim->repair(in.label);
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
	/// R15: a copy of @a file where the first element of the symbol type @a type_part carries the fault @a fault
QString withFault(const QString &file, const QString &type_part, const QString &fault, const QString &name)
{
	QFile f(file);
	if (!f.open(QIODevice::ReadOnly)) return QString();
	QString text = QString::fromUtf8(f.readAll());
	const int at = static_cast<int>(text.indexOf(type_part));
	if (at < 0) return QString();
	const QString tag = QStringLiteral("<elementInformations>");
	const int info = static_cast<int>(text.indexOf(tag, at));
	if (info < 0) return QString();
	text.insert(info + tag.size(), QStringLiteral("<elementInformation name=\"sim_fault\" show=\"0\">%1</elementInformation>").arg(fault));
	const QString out = QDir(QDir::tempPath()).filePath(name);
	QFile o(out);
	if (!o.open(QIODevice::WriteOnly)) return QString();
	o.write(text.toUtf8());
	return out;
}

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
	/// R11: path matrix, rest position and actuation of every valve symbol of a collection; @return problems
QStringList valveAudit(const QString &dir, int *valves, QHash<QString, int> *kinds)
{
	QStringList bad;
	QDirIterator it(dir, {QStringLiteral("*.elmt")}, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		const QString file = it.next();
		QFile f(file);
		if (!f.open(QIODevice::ReadOnly)) continue;
		QDomDocument doc;
		if (!doc.setContent(&f)) continue;
		const QDomElement root = doc.documentElement();
		const FluidData d = FluidData::fromElementDefinition(root);
		if (d.m_kind != FluidData::Kind::Valve) continue;
		++*valves;
		QSet<QString> ports;
		const QDomNodeList ts = root.elementsByTagName(QStringLiteral("terminal"));
		for (int i = 0; i < ts.size(); ++i) {
			const QDomElement te = ts.at(i).toElement();
			const QString medium = te.attribute(QStringLiteral("medium"));
			if (medium == QLatin1String("pneumatic") || medium == QLatin1String("hydraulic")) ports << te.attribute(QStringLiteral("name")).trimmed();
		}
		const QString who = QFileInfo(file).fileName();
		const int n = static_cast<int>(d.m_positions.size());
		if (n < 2) { bad << QStringLiteral("%1: %2 Stellung(en)").arg(who).arg(n); continue; }
		if (d.m_initial_position < 1 || d.m_initial_position > n) bad << QStringLiteral("%1: Ruhestellung %2 von %3").arg(who).arg(d.m_initial_position).arg(n);
		for (int k = 0; k < n; ++k)
			for (const FluidData::Path &pa : d.m_positions.at(k).paths) {
				if (!ports.contains(pa.from) || (!pa.to.isEmpty() && !ports.contains(pa.to)))
					bad << QStringLiteral("%1: Stellung %2, Weg %3→%4 ohne Anschluss").arg(who).arg(k + 1).arg(pa.from, pa.to.isEmpty() ? QStringLiteral("zu") : pa.to);
				else if (pa.from == pa.to) bad << QStringLiteral("%1: Stellung %2, Weg %3 auf sich selbst").arg(who).arg(k + 1).arg(pa.from);
			}
		if (d.m_left.isNull() && d.m_right.isNull()) bad << QStringLiteral("%1: keine Betätigung").arg(who);
		for (const FluidData::Actuation &a : {d.m_left, d.m_right}) {
			for (const QString &ty : a.types) (*kinds)[ty] += 1;
			if (a.spring) (*kinds)[QStringLiteral("Feder")] += 1;
			if (a.pilot) (*kinds)[QStringLiteral("Vorsteuerung")] += 1;
			if (a.air_spring) (*kinds)[QStringLiteral("Luftfeder")] += 1;
		}
	}
	return bad;
}

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
		QStringLiteral("Vakuum und Schwenk-/Greiferantriebe (R10): Saugen, Halten, Leckage, Last, Loslassen, Schwenkwinkel und -moment"),
		QStringLiteral("Leitungen, Kupplungen, Aufbereitung (R05): Rohr laminar/turbulent, beide Richtungen, Widerstand, Kupplung, Filter verstopft, Drehdurchführung"),
		QStringLiteral("Stetig-, Senkbrems- und Zuschaltventile (R07/R09): Kennlinie, Richtung, Sättigung, Überdeckung, Kerbe, Leckage, Sprungantwort, Steuerdruck, Druckwaagen, Schockblock, Folgeventil"),
		QStringLiteral("Analoge Signale und Regler (R06): Kalibrierung, Umkehr, Sättigung, Anti-Windup, Reset, Abtastverfeinerung, Druck- und Lageregelkreis"),
		QStringLiteral("Standardventile und Zylinderkräfte (R11): Pfadmatrizen der Sammlung, gleichzeitige Betätigung, Vorrang, Gewicht, Kraftprofil, Feder, Reibung"),
		QStringLiteral("Druckluftmotoren (R12): Leerlaufdrehzahl nach Druck, Nennpunkt aus den Nennwerten, beide Drehrichtungen"),
		QStringLiteral("Mehrstellungszylinder (R12): Zwischenstellung, Endlage, Halt im Rückhub, gemeinsamer Rückhub"),
		QStringLiteral("Mobilhydraulik (R13): LS-Bereitschaftsdruck, LS-Regelung lastunabhängig, Druckbegrenzung, 6/3-Handhebelventil, Belastungseinheit"),
		QStringLiteral("Lenkung und Sperrventile (R13): Lenkventil voll, halb, neutral, rechts; Vorsteuergerät; Sperrblock entsperrt, gesperrt, Durchfluss; Belastungseinheit 1; Speicher mit Sicherheitsblock"),
		QStringLiteral("LS-Steuerblock und Prioritätsventil (R13): Druckwaage hält den Strom lastunabhängig, Lastmeldung, Rückhub; Prioritätsventil teilt CF/EF"),
		QStringLiteral("GRAFCET-Struktur (R14): einschließender Schritt, Makroschritt, Zwangssteuerung, Instanzen, Analogwert"),
		QStringLiteral("Fehlerszenarien (R15): Taster, Spule mit Reparatur, klemmender Kolben, Rückschlagventil, Verstopfung, Leckage, Wiederholbarkeit"),
		QStringLiteral("Simulationsmodi (R16): bis zur nächsten Änderung, Einzelschritt, Teilprojekt (Blatt), Leistung"),
		QStringLiteral("Analoge Elektrik (R17, Stufe 1): RC, RL, RLC, Einweggleichrichter, Spannungsteiler, Leistungsbilanz"),
		QStringLiteral("Kopplung Stromkreis (R17, Stufe 2): Taster und Relaisspule mit R, L und Freilaufdiode, Relaiskontakt, SPS-Analogeingang und -ausgang"),
		QStringLiteral("Halbleiter (R17, Stufe 3): NPN aktiv und gesättigt, PNP, invertierender Operationsverstärker linear und begrenzt")};
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
		//    1.8 l/min at -0.6 bar through the ISO 6358 orifice, b = 0.4), solved by bisection
		{
			const QString file = cupVariant(QStringLiteral("<parameter name=\"qLeak\" unit=\"l/min\" value=\"1.8\"/>"), "qet_sim_r10_leck.qet");
			QHash<QString, double> v;
			ProjectRun pr = runProject(file, 6.0, grip, 1.5, sample(0.95, v, ids));
			if (!pr.error.isEmpty()) wrong << QStringLiteral("Leckage: %1").arg(pr.error);
			else {
				const double patm = 101300.0, b = 0.4;
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
	{   // 12. lines, couplings and treatment (R05, FORMELN.md L01–L05)
		Check c{names.at(11)};
		QStringList wrong, seen;
		auto near = [](double a, double b, double rel) { return std::isfinite(a) && std::abs(a - b) <= rel * std::abs(b); };
		// a) the pipe law against Hagen-Poiseuille and Darcy/Haaland written out here, both directions
		{
			SimPhysics::PipeFlow lam;
			lam.L = 10; lam.d = 0.01; lam.rho = 870; lam.mu = 870 * 22e-6;
			const double q1 = 1e-4, hp = 128 * lam.mu * lam.L * q1 / (M_PI * std::pow(lam.d, 4));
			SimPhysics::PipeFlow tur = lam;
			tur.L = 5; tur.d = 0.006; tur.mu = 870 * 5e-6;
			const double q2 = 3e-4, v = q2 / (M_PI * tur.d * tur.d / 4), re = tur.rho * v * tur.d / tur.mu;
			const double s = -1.8 * std::log10(std::pow(tur.eps / tur.d / 3.7, 1.11) + 6.9 / re);
			const double darcy = (1 / (s * s)) * tur.L / tur.d * tur.rho * v * v / 2;
			int bad = 0;
			double worst = 0;
			for (const SimPhysics::PipeFlow *pf : {&lam, &tur})
				for (double q : {1e-6, 5e-5, 2e-4, 6e-4, 1.5e-3})
					for (double sg : {1.0, -1.0}) {
						const double dq = pf->flow(pf->dp(sg * q));
						worst = std::max(worst, std::abs(dq - sg * q) / q);
						if (pf->dp(sg * q) * sg * q < 0) ++bad;                    // passive: dp·q >= 0
						if (std::abs(pf->dp(sg * q) + pf->dp(-sg * q)) > 1e-12 * std::abs(pf->dp(q))) ++bad;   // odd
					}
			// continuous at Re 2000 and 4000, rising everywhere
			auto atRe = [&](double rr) { return rr * tur.mu * M_PI * tur.d / (4 * tur.rho); };
			double jump = 0, prev = 0;
			for (double rr : {2000.0, 4000.0})
				jump = std::max(jump, std::abs(tur.dp(atRe(rr) * (1 + 1e-9)) - tur.dp(atRe(rr) * (1 - 1e-9))) / tur.dp(atRe(rr)));
			for (int i = 1; i <= 400; ++i) {
				const double d = tur.dp(atRe(12000.0 * i / 400));
				if (!(d > prev)) ++bad;
				prev = d;
			}
			if (!near(lam.dp(q1), hp, 1e-12) || !near(tur.dp(q2), darcy, 1e-12) || worst > 1e-9 || bad || jump > 1e-6)
				wrong << QStringLiteral("Rohrgesetz: laminar %1/%2 Pa, turbulent %3/%4 Pa, Umkehrfehler %5, Verstöße %6, Sprung %7")
							 .arg(lam.dp(q1)).arg(hp).arg(tur.dp(q2)).arg(darcy).arg(worst).arg(bad).arg(jump);
			else seen << QStringLiteral("Rohr laminar = Hagen-Poiseuille, turbulent (Re %1) = Darcy/Haaland, Umkehr ≤ %2, "
										"ungerade, passiv, stetig bei Re 2000/4000, steigend").arg(re, 0, 'f', 0).arg(worst, 0, 'g', 2);
		}
		// b) plan rohr.qet: pump -GP1 (11.6 l/min) -> part -HQ1 -> tank; the part becomes line, coupling or filter
		const QString rohr = path("rohr.qet");
		const QString filter = QStringLiteral("<fluid function=\"supply.filter\" kind=\"treatment\" prefix=\"HQ\"/>");
		auto part = [&](const QString &function, const QString &params, const char *name) {
			return textVariant(rohr, {{filter, QStringLiteral("<fluid function=\"%1\" kind=\"treatment\" prefix=\"HQ\">%2</fluid>").arg(function, params)}},
							   QString::fromLatin1(name));
		};
		auto pumpPressure = [&](const QString &file, double *q) {
			double pp = std::nan("");
			ProjectRun pr = runProject(file, 6.0, {}, 2.0, [&](SimulationModel &s) {
				if (std::abs(s.time() - 1.9) < 5e-6) pp = s.signalValue(QStringLiteral("p:-GP1.P"));
			});
			*q = std::nan("");
			if (pr.error.isEmpty())
				for (Diagram *d : pr.project->diagrams())
					for (Element *e : d->elements())
						if (e->elementData().m_fluid.m_function == QLatin1String("supply.pump"))
							*q = symbolValue(e, QStringLiteral("displacement"), QStringLiteral("m3")) * symbolValue(e, QStringLiteral("rpm"), QStringLiteral("1/s"));
			QFile::remove(file);
			return pr.error.isEmpty() ? pp : std::nan("");
		};
		const double rho = 870, nu = 22e-6;
		struct Case { const char *what, *function, *params, *file; std::function<double(double)> expect; double rel; };
		const QList<Case> cases{
			{"Rohr laminar (10 m, Ø 10 mm)", "connection.line",
			 "<parameter name=\"length\" unit=\"m\" value=\"10\"/><parameter name=\"d_inner\" unit=\"mm\" value=\"10\"/>", "qet_sim_r05_lam.qet",
			 [&](double q) { return 128 * rho * nu * 10 * q / (M_PI * std::pow(0.01, 4)); }, 0.01},
			{"Rohr turbulent (5 m, Ø 6 mm, 5 mm²/s)", "connection.line",
			 "<parameter name=\"length\" unit=\"m\" value=\"5\"/><parameter name=\"d_inner\" unit=\"mm\" value=\"6\"/><parameter name=\"nu\" unit=\"mm2/s\" value=\"5\"/>",
			 "qet_sim_r05_tur.qet",
			 [&](double q) {
				 const double d = 0.006, v = q / (M_PI * d * d / 4), re = v * d / 5e-6;
				 const double s = -1.8 * std::log10(std::pow(15e-6 / d / 3.7, 1.11) + 6.9 / re);
				 return (1 / (s * s)) * 5 / d * rho * v * v / 2;
			 }, 0.01},
			{"Schlauch als Widerstand rT 0,12", "supply.hose_line", "<parameter name=\"rT\" value=\"0.12\"/>", "qet_sim_r05_rt.qet",
			 [](double q) { return 0.12 * std::pow(q * 60000, 2) * 1e5; }, 0.01},
			{"Kupplung gekuppelt (ideal)", "connection.quick_coupling", "<parameter name=\"coupled\" value=\"1\"/>", "qet_sim_r05_k1.qet",
			 [](double) { return 0.0; }, 0.0},
			{"Kupplung getrennt, selbstdichtend", "connection.quick_coupling",
			 "<parameter name=\"coupled\" value=\"0\"/><parameter name=\"seal\" value=\"1\"/>", "qet_sim_r05_k0.qet",
			 [](double) { return 60e5; }, 0.017},
			{"Kupplung getrennt, offen (Öl tritt aus)", "connection.quick_coupling",
			 "<parameter name=\"coupled\" value=\"0\"/><parameter name=\"seal\" value=\"0\"/>", "qet_sim_r05_kx.qet",
			 [](double q) { return 0.05 * std::pow(q * 60000, 2) * 1e5; }, 0.01},
			// clean filter: below 0.5 bar (rT·Q² = 0.135 bar; the orifice is smoothed below p_s = 0.3 bar, H03)
			{"Filter sauber (rT 0,001)", "supply.filter", "<parameter name=\"rT\" value=\"0.001\"/>", "qet_sim_r05_f.qet",
			 [](double q) { return 0.001 * std::pow(q * 60000, 2) * 1e5; }, 0.0},
			{"Filter verstopft (rT_clogged 0,2)", "supply.filter",
			 "<parameter name=\"rT\" value=\"0.001\"/><parameter name=\"clogged\" value=\"1\"/><parameter name=\"rT_clogged\" value=\"0.2\"/>",
			 "qet_sim_r05_fz.qet", [](double q) { return 0.2 * std::pow(q * 60000, 2) * 1e5; }, 0.01},
		};
		for (const Case &k : cases) {
			const QString file = part(QString::fromLatin1(k.function), QString::fromLatin1(k.params), k.file);
			double q = 0;
			const double pp = file.isEmpty() ? std::nan("") : pumpPressure(file, &q);
			const double want = k.expect(q);
			const bool ok = k.rel > 0 ? near(pp, want, k.rel) : std::isfinite(pp) && std::abs(pp) < 0.5e5;
			if (!ok) wrong << QStringLiteral("%1: %2 bar statt %3").arg(QString::fromUtf8(k.what)).arg(pp / 1e5).arg(want / 1e5);
			else seen << QStringLiteral("%1: %2 bar (soll %3)").arg(QString::fromUtf8(k.what)).arg(pp / 1e5, 0, 'f', 3).arg(want / 1e5, 0, 'f', 3);
		}
		// c) pneumatics: rotary joint keeps its channels apart; a coupled quick connect passes, an open one vents
		{
			QHash<QString, double> v;
			const QStringList ids{QStringLiteral("p:-BP1.1"), QStringLiteral("p:-BP2.1"), QStringLiteral("p:-BP3.1"), QStringLiteral("p:-BP4.1")};
			ProjectRun pr = runProject(path("kupplung_drehdurchfuehrung.qet"), 6.0, {}, 0.5, [&](SimulationModel &s) {
				if (std::abs(s.time() - 0.45) < 5e-6) for (const QString &id : ids) v.insert(id, s.signalValue(id));
			});
			if (!pr.error.isEmpty()) wrong << QStringLiteral("Drehdurchführung/Kupplung: %1").arg(pr.error);
			else if (!near(v.value(ids[0]), 6e5, 0.002) || !(std::abs(v.value(ids[1])) < 100) || !near(v.value(ids[2]), 6e5, 0.002) || !(std::abs(v.value(ids[3])) < 100))
				wrong << QStringLiteral("Kanal 1 %1, Kanal 2 %2, gekuppelt %3, offen %4 bar").arg(v.value(ids[0]) / 1e5).arg(v.value(ids[1]) / 1e5)
							 .arg(v.value(ids[2]) / 1e5).arg(v.value(ids[3]) / 1e5);
			else seen << QStringLiteral("Drehdurchführung: Kanal 1 %1 bar, Kanal 2 %2 bar; Schnellanschluss gekuppelt %3 bar, getrennt %4 bar")
							 .arg(v.value(ids[0]) / 1e5, 0, 'f', 2).arg(v.value(ids[1]) / 1e5, 0, 'f', 2).arg(v.value(ids[2]) / 1e5, 0, 'f', 2).arg(v.value(ids[3]) / 1e5, 0, 'f', 2);
		}
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 13. proportional, counterbalance and sequence valves (R07/R09, FORMELN.md V05–V08). Pump -GP1 with
		//     11.6 l/min (pMax 60 bar) feeds -QM1; every other port goes to a tank, so the pump pressure is the
		//     pressure drop the valve needs for the pump flow
		Check c{names.at(12)};
		QStringList wrong, seen;
		auto near = [](double a, double b, double rel) { return std::isfinite(a) && std::abs(a - b) <= rel * std::abs(b); };
		auto run = [&](const QString &file, QList<Input> in, double at, const QStringList &ids, double *q) {
			QHash<QString, double> v;
			ProjectRun pr = runProject(file, 6.0, in, at + 0.05, [&](SimulationModel &s) {
				if (std::abs(s.time() - at) < 5e-6) for (const QString &id : ids) v.insert(id, s.signalValue(id));
			});
			*q = std::nan("");
			if (pr.error.isEmpty())
				for (Diagram *d : pr.project->diagrams())
					for (Element *e : d->elements())
						if (e->elementData().m_fluid.m_function == QLatin1String("supply.pump"))
							*q = symbolValue(e, QStringLiteral("displacement"), QStringLiteral("m3")) * symbolValue(e, QStringLiteral("rpm"), QStringLiteral("1/s"));
			if (!pr.error.isEmpty()) v.insert(QStringLiteral("error"), std::nan(""));
			QFile::remove(file);
			return v;
		};
		const QString valve = QStringLiteral("<fluid function=\"directional_valve.four_pins\" kind=\"valve\" prefix=\"QM\">");
		auto prop = [&](const QString &params, const char *name) {
			return textVariant(path("stetig.qet"), {{valve, QStringLiteral("<fluid function=\"proportional.directional_valve43_02\" kind=\"valve\" prefix=\"QM\">") + params}},
							   QString::fromLatin1(name));
		};
		auto par = [](const char *n, double v, const char *unit = "") {
			return QStringLiteral("<parameter name=\"%1\" unit=\"%2\" value=\"%3\"/>").arg(QString::fromLatin1(n), QString::fromLatin1(unit)).arg(v);
		};
		struct Case { const char *what; QString params; double volts; std::function<double(double)> expect; };
		// expected pump pressure [Pa] from the pump flow q [m3/s]: an edge open by area a needs dp = rT·Q²/a² (Q in l/min)
		auto edges = [](double rt, double area) { return [rt, area](double q) { return rt * std::pow(q * 60000 / area, 2) * 1e5; }; };
		const QList<Case> cases{
			{"P→A halb offen (5 V, Rechteck)", par("rT", 0.05, "bar.min2/l2") + par("lap", 0, "%"), 5.0, edges(0.05, 0.5)},
			{"P→B halb offen (−5 V)", par("rT", 0.05, "bar.min2/l2") + par("lap", 0, "%"), -5.0, edges(0.05, 0.5)},
			{"Sättigung (12 V = voll offen)", par("rT", 0.05, "bar.min2/l2") + par("lap", 0, "%"), 12.0, edges(0.05, 1.0)},
			{"Überdeckung 25 %, 2 V: zu", par("rT", 0.02, "bar.min2/l2") + par("lap", 25, "%"), 2.0, [](double) { return 60e5; }},
			{"Überdeckung 25 %, 5 V: x = 1/3", par("rT", 0.02, "bar.min2/l2") + par("lap", 25, "%"), 5.0, edges(0.02, 1.0 / 3)},
			{"Unterdeckung −20 %, 0 V: beide Kanten 1/6", par("rT", 0.02, "bar.min2/l2") + par("lap", -20, "%"), 0.0, edges(0.02, 2.0 / 6)},
			{"Dreieckkerbe, 5 V: Fläche x²", par("rT", 0.01, "bar.min2/l2") + par("lap", 0, "%") + par("notchType", 2), 5.0, edges(0.01, 0.25)},
			{"Mittelstellung, Leckage 0,3 l/(min·bar) je Kante", par("rT", 0.05, "bar.min2/l2") + par("lap", 0, "%") + par("GLeak", 0.3, "l/(min.bar)"),
			 0.0, [](double q) { return q * 60000 / (2 * 0.3) * 1e5; }},
		};
		int i = 0;
		for (const Case &k : cases) {
			double q = 0;
			const QString file = prop(k.params, qPrintable(QStringLiteral("qet_sim_r07_%1.qet").arg(i++)));
			const auto v = run(file, {{0.05, QStringLiteral("-QM1"), 3, true, k.volts}}, 0.9, {QStringLiteral("p:-GP1.P")}, &q);
			const double pp = v.value(QStringLiteral("p:-GP1.P"), std::nan("")), want = k.expect(q);
			const bool closed = want >= 59.9e5;
			const bool ok = closed ? (pp > 59e5 && pp < 60.01e5) : near(pp, want, 0.01);
			if (!ok) wrong << QStringLiteral("%1: %2 bar statt %3").arg(QString::fromUtf8(k.what)).arg(pp / 1e5).arg(want / 1e5);
			else seen << QStringLiteral("%1: %2 bar (soll %3)").arg(QString::fromUtf8(k.what)).arg(pp / 1e5, 0, 'f', 2)
							 .arg(closed ? QStringLiteral("59…60") : QString::number(want / 1e5, 'f', 2));
		}
		{   // step response: 0 -> 10 V at 0.5 s, tau 4 ms: s = 1 - e^-1 one time constant later
			double q = 0;
			const QString file = prop(par("rT", 0.05, "bar.min2/l2") + par("tau", 0.004, "s"), "qet_sim_r07_step.qet");
			const auto v = run(file, {{0.5, QStringLiteral("-QM1"), 3, true, 10.0}}, 0.504, {QStringLiteral("s:-QM1"), QStringLiteral("u:-QM1")}, &q);
			const double sv = v.value(QStringLiteral("s:-QM1"), std::nan("")), want = 1 - std::exp(-1.0);
			if (!(std::abs(sv - want) < 2e-3) || v.value(QStringLiteral("u:-QM1")) != 10.0)
				wrong << QStringLiteral("Sprungantwort: s = %1 statt %2").arg(sv).arg(want);
			else seen << QStringLiteral("Sprungantwort PT1: s(τ) = %1 (soll %2)").arg(sv, 0, 'f', 4).arg(want, 0, 'f', 4);
		}
		// b) plan lastventil.qet: -QM1 with P and X on the pump line, the third port on the tank
		const QString part = QStringLiteral("<fluid function=\"directional_valve.part\" kind=\"other\">");
		const QString portA = QStringLiteral("<terminal medium=\"hydraulic\" name=\"A\" orientation=\"n\" type=\"Generic\" uuid=\"{c29ef07e-b0f4-461c-ab62-b783dc560216}\"");
		const QString portT = QStringLiteral("<terminal medium=\"hydraulic\" name=\"T\" orientation=\"s\" type=\"Generic\" uuid=\"{b488c45e-d33f-4923-be1f-7b224e4bd3ba}\"");
		auto load = [&](const QString &function, const QString &params, bool a_to_t, const char *name) {
			QList<QPair<QString, QString>> pairs{{part, QStringLiteral("<fluid function=\"%1\" kind=\"other\">%2").arg(function, params)},
												 {portT, QString(portT).replace(QStringLiteral("name=\"T\""), QStringLiteral("name=\"X\""))}};
			if (a_to_t) pairs << qMakePair(portA, QString(portA).replace(QStringLiteral("name=\"A\""), QStringLiteral("name=\"T\"")));
			return textVariant(path("lastventil.qet"), pairs, QString::fromLatin1(name));
		};
		// equilibrium of the pump flow and the opening valve, solved by bisection: share(p)·√(p/rT) = Q (bar, l/min)
		auto solve = [](double q_lmin, double rt, std::function<double(double)> share) {
			double lo = 0, hi = 60;
			for (int n = 0; n < 200; ++n) { const double m = 0.5 * (lo + hi); (share(m) * std::sqrt(m / rt) > q_lmin ? hi : lo) = m; }
			return 0.5 * (lo + hi) * 1e5;
		};
		struct Load { const char *what, *function; QString params; bool a_to_t; std::function<double(double)> share; double rt; };
		const QList<Load> loads{
			{"Senkbremsventil mit Steuerdruck (ratio 3)", "pressure_valve.counterbalance_valve",
			 par("p0", 4, "MPa") + par("dpEnd", 1, "MPa") + par("rT_cb", 0.02, "bar.min2/l2") + par("ratio", 3), true,
			 [](double pb) { return std::clamp((4 * pb - 40) / 10, 0.0, 1.0); }, 0.02},
			{"Senkbremsventil ohne Steuerdruck (ratio 0)", "pressure_valve.counterbalance_valve",
			 par("p0", 4, "MPa") + par("dpEnd", 1, "MPa") + par("rT_cb", 0.02, "bar.min2/l2") + par("ratio", 0), true,
			 [](double pb) { return std::clamp((pb - 40) / 10, 0.0, 1.0); }, 0.02},
			{"Druckzuschaltventil (X am Pumpendruck, p0 30 bar)", "pressure_valve.pressure_sequence_valve02",
			 par("p0", 3, "MPa") + par("dpEnd", 0.7, "MPa") + par("rT", 0.1, "bar.min2/l2"), false,
			 [](double pb) { return std::clamp((pb - 30) / 7, 0.0, 1.0); }, 0.1},
		};
		i = 0;
		for (const Load &k : loads) {
			double q = 0;
			const QString file = load(QString::fromLatin1(k.function), k.params, k.a_to_t, qPrintable(QStringLiteral("qet_sim_r09_%1.qet").arg(i++)));
			const auto v = file.isEmpty() ? QHash<QString, double>{{QStringLiteral("error"), 0}} : run(file, {}, 1.4, {QStringLiteral("p:-GP1.P")}, &q);
			const double pp = v.value(QStringLiteral("p:-GP1.P"), std::nan("")), want = solve(q * 60000, k.rt, k.share);
			if (!near(pp, want, 0.005)) wrong << QStringLiteral("%1: %2 bar statt %3").arg(QString::fromUtf8(k.what)).arg(pp / 1e5).arg(want / 1e5);
			else seen << QStringLiteral("%1: %2 bar (soll %3)").arg(QString::fromUtf8(k.what)).arg(pp / 1e5, 0, 'f', 2).arg(want / 1e5, 0, 'f', 2);
		}
		{   // c) R09 rest (FORMELN.md V10–V13), plan druckwaagen.qet: four oil circuits, each with its own pump 11.6 l/min
			QHash<QString, double> v;
			const QStringList ids{QStringLiteral("p:-GP1.P"), QStringLiteral("p:-QN2.A"), QStringLiteral("p:-QN2.X"), QStringLiteral("p:-GP2.P"),
								  QStringLiteral("p:-QN3.A"), QStringLiteral("p:-GP3.P"), QStringLiteral("p:-GP4.P"), QStringLiteral("p:-QN4.B1")};
			ProjectRun pr = runProject(path("druckwaagen.qet"), 6.0, {}, 1.5001, [&](SimulationModel &s) {
				if (std::abs(s.time() - 1.5) < 5e-6) for (const QString &id : ids) v.insert(id, s.signalValue(id));
			});
			double q = 0;
			if (pr.error.isEmpty())
				for (Diagram *d : pr.project->diagrams())
					for (Element *e : d->elements())
						if (e->elementData().m_fluid.m_function == QLatin1String("supply.pump"))
							q = symbolValue(e, QStringLiteral("displacement"), QStringLiteral("m3")) * symbolValue(e, QStringLiteral("rpm"), QStringLiteral("1/s"));
			if (!pr.error.isEmpty()) wrong << QStringLiteral("druckwaagen.qet: %1").arg(pr.error);
			else {
				const double ql = q * 60000;
				auto bar = [&](const char *id) { return v.value(QLatin1String(id), std::nan("")) / 1e5; };
				// opener LS (p0 20 bar, dpEnd 5 bar, rT 0.05): relief P -> T with X at the tank
				const double ls = solve(ql, 0.05, [](double pb) { return std::clamp((pb - 20) / 5, 0.0, 1.0); }) / 1e5;
				if (!near(bar("p:-GP1.P"), ls, 0.005)) wrong << QStringLiteral("LS-Druckwaage: %1 bar statt %2").arg(bar("p:-GP1.P")).arg(ls);
				else seen << QStringLiteral("LS-Druckwaage: %1 bar (soll %2)").arg(bar("p:-GP1.P"), 0, 'f', 2).arg(ls, 0, 'f', 2);
				// upstream compensator (p0 4 bar, band 1.5 bar) before a throttle rT 1: holds pA - pX in its band
				const double dA = bar("p:-QN2.A") - bar("p:-QN2.X");
				if (!(dA >= 4.0 - 1e-6 && dA <= 5.5 && bar("p:-GP2.P") > bar("p:-QN2.A") + 10))
					wrong << QStringLiteral("vorgeschaltete Druckwaage: pA − pX %1 bar, Pumpe %2 bar").arg(dA).arg(bar("p:-GP2.P"));
				else seen << QStringLiteral("vorgeschaltete Druckwaage: pA − pX %1 bar (Band 4 … 5,5), Drossel %2 l/min").arg(dA, 0, 'f', 2).arg(std::sqrt(dA), 0, 'f', 2);
				// closer as pressure regulator (p0 10 bar, band 2 bar, X at A): pA in its band
				const double pa = bar("p:-QN3.A");
				if (!(pa >= 10.0 - 1e-6 && pa <= 12.0 && bar("p:-GP3.P") > pa + 10))
					wrong << QStringLiteral("Schließer als Druckregler: pA %1 bar, Pumpe %2 bar").arg(pa).arg(bar("p:-GP3.P"));
				else seen << QStringLiteral("Schließer als Druckregler: pA %1 bar (Band 10 … 12)").arg(pa, 0, 'f', 2);
				// shock block: relief A -> T (30 bar, band 38 bar, rT 0.08) and anti-cavitation T -> B (rT_cv 0.005)
				const double sh = solve(ql, 0.08, [](double pb) { return std::clamp((pb - 30) / 38, 0.0, 1.0); }) / 1e5;
				if (!near(bar("p:-GP4.P"), sh, 0.005)) wrong << QStringLiteral("Schockventil A: %1 bar statt %2").arg(bar("p:-GP4.P")).arg(sh);
				else seen << QStringLiteral("Schockventil A: %1 bar (soll %2)").arg(bar("p:-GP4.P"), 0, 'f', 2).arg(sh, 0, 'f', 2);
				// the check valve passes the suction flow q of -GP5: dp with orificeFlow(dp) = q (H03 incl. smoothing)
				const double k = 1.0 / (60000.0 * std::sqrt(1e5 * 0.005));
				double lo = 0, hi = 5e5;
				for (int n = 0; n < 200; ++n) { const double m = 0.5 * (lo + hi); (SimPhysics::orificeFlow(m, k, 0.3e5) > q ? hi : lo) = m; }
				const double cav = -0.5 * (lo + hi) / 1e5;
				if (!(std::abs(bar("p:-QN4.B1") - cav) < 0.01)) wrong << QStringLiteral("Nachsaugen B: %1 bar statt %2").arg(bar("p:-QN4.B1")).arg(cav);
				else seen << QStringLiteral("Nachsaugen B: %1 bar (soll %2)").arg(bar("p:-QN4.B1"), 0, 'f', 3).arg(cav, 0, 'f', 3);
			}
			// pneumatic sequence valve: 12 at the supply (6 bar) opens 1 -> 2, the receiver (1 l, precharge 4 bar) fills within 3 s;
			// with a switching pressure of 7 bar it stays closed
			auto receiver = [&](const QString &file) {
				double x = std::nan("");
				ProjectRun pf = runProject(file, 6.0, {}, 3.0001, [&](SimulationModel &s) {
					if (std::abs(s.time() - 3.0) < 5e-6) x = s.signalValue(QStringLiteral("p:-CM1.1")) / 1e5;
				});
				if (!pf.error.isEmpty()) wrong << QStringLiteral("%1: %2").arg(QFileInfo(file).fileName(), pf.error);
				return x;
			};
			const double open = receiver(path("folgeventil.qet"));
			const QString hold = textVariant(path("folgeventil.qet"), {{QStringLiteral("<parameter name=\"p0\" unit=\"bar\" value=\"3\"/>"),
																	   QStringLiteral("<parameter name=\"p0\" unit=\"bar\" value=\"7\"/>")}},
											 QStringLiteral("qet_sim_r09_folge.qet"));
			const double closed = hold.isEmpty() ? std::nan("") : receiver(hold);
			if (!hold.isEmpty()) QFile::remove(hold);
			if (!(std::abs(open - 6.0) < 0.05 && std::abs(closed - 4.0) < 0.01))
				wrong << QStringLiteral("Folgeventil: Behälter %1 bar (soll 6), mit 7 bar Schaltdruck %2 bar (soll 4)").arg(open).arg(closed);
			else seen << QStringLiteral("Folgeventil 3 bar: Behälter %1 bar, Schaltdruck 7 bar: %2 bar (bleibt zu)").arg(open, 0, 'f', 2).arg(closed, 0, 'f', 2);
		}
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 14. analog signals and controllers (R06, FORMELN.md A01–A10, V09): the blocks against closed solutions,
		//     then a pressure loop (PI, proportional relief valve) and a position loop (P, 4/3 valve, position sensor)
		Check c{names.at(13)};
		QStringList wrong, seen;
		auto expect = [&](const QString &what, double got, double want, double tol) {
			if (!(std::isfinite(got) && std::abs(got - want) <= tol)) wrong << QStringLiteral("%1: %2 statt %3").arg(what).arg(got).arg(want);
			else seen << QStringLiteral("%1 %2").arg(what).arg(got, 0, 'g', 5);
		};
		using namespace SimSignals;
		// A01 sensor characteristic: calibration, reversed signal, saturation outside the range
		expect(QStringLiteral("Kalibrierung 4 MPa →"), scale(4e6, 0, 10e6, 0, 10), 4.0, 1e-12);
		expect(QStringLiteral("Umkehr 2,5 MPa →"), scale(2.5e6, 0, 10e6, 10, 0), 7.5, 1e-12);
		expect(QStringLiteral("Sättigung 12 MPa →"), scale(12e6, 0, 10e6, 0, 10), 10.0, 1e-12);
		{   // A04 saturation and the unipolar range
			Pid a; a.kp = 2;
			expect(QStringLiteral("PID begrenzt e = 8 V, KP 2 →"), a.step(8, 0, 1e-4), 10.0, 1e-12);
			Pid b; b.umin = 0;
			expect(QStringLiteral("PID 0 … 10 V bei e = -5 →"), b.step(0, 5, 1e-4), 0.0, 1e-12);
		}
		{   // A04 anti-windup: 2 s saturated (r = 10, y = 0), then y = 11; time until the output leaves the limit
			auto unwind = [](double t_track) {
				Pid a; a.kp = 1; a.ki = 10; a.t_track = t_track;
				const double dt = 1e-4;
				for (int i = 0; i < 20000; ++i) a.step(10, 0, dt);
				for (int i = 0; i < 400000; ++i) if (a.step(10, 11, dt) < 10.0 - 1e-9) return i * dt;
				return 40.0;
			};
			const double aw = unwind(0.09), no = unwind(1e12);
			if (!(aw < 0.2 && no > 5.0)) wrong << QStringLiteral("Anti-Windup: %1 s, ohne %2 s").arg(aw).arg(no);
			else seen << QStringLiteral("Anti-Windup: aus der Begrenzung nach %1 s (ohne: %2 s)").arg(aw, 0, 'f', 4).arg(no, 0, 'f', 1);
			Pid a; a.kp = 1; a.ki = 10;
			for (int i = 0; i < 1000; ++i) a.step(5, 0, 1e-3);
			a.reset();
			expect(QStringLiteral("Reset: erster Wert KP·e ="), a.step(1, 0, 1e-3), 1.0, 1e-12);
		}
		{   // A04 sampling refinement: P control of an integrator y' = 2·u, r = 5: y(1 s) = 5·(1 - e^-2)
			auto run = [](double dt) {
				Pid a; a.kp = 1;
				double y = 0;
				const int n = qRound(1.0 / dt);
				for (int i = 0; i < n; ++i) y += dt * 2.0 * a.step(5, y, dt);
				return y;
			};
			const double exact = 5 * (1 - std::exp(-2.0)), e1 = std::abs(run(1e-3) - exact), e2 = std::abs(run(5e-4) - exact);
			if (!(e2 < e1 && e1 / e2 > 1.8 && e1 / e2 < 2.2 && e2 / exact < 1e-3))
				wrong << QStringLiteral("Abtastverfeinerung: Fehler %1 bei 1 ms, %2 bei 0,5 ms").arg(e1).arg(e2);
			else seen << QStringLiteral("Abtastverfeinerung: Fehler %1 → %2 (Ordnung 1)").arg(e1, 0, 'g', 3).arg(e2, 0, 'g', 3);
		}
		{   // A03 setpoint card: W1 = 2 V (R1 0.25 s/V), after 1 s W2 = -4 V (R2 0.1, R3 0.05 s/V)
			SetpointCard k; k.count = 2; k.w[0] = 2; k.w[1] = -4; k.ramp[0] = 0.25; k.ramp[1] = 0.1; k.ramp[2] = 0.05; k.t_switch = 1.0;
			QHash<int, double> at;
			const double dt = 1e-4;
			for (int i = 0; i <= 15000; ++i) {
				const double y = k.step(i * dt, dt, 0);
				if (i == 2500 || i == 9000 || i == 13000 || i == 15000) at.insert(i, y);
			}
			expect(QStringLiteral("Sollwertkarte t 0,25 s"), at.value(2500), 1.0, 2e-3);
			expect(QStringLiteral("t 0,9 s"), at.value(9000), 2.0, 1e-9);
			expect(QStringLiteral("t 1,3 s"), at.value(13000), -2.0, 4e-3);
			expect(QStringLiteral("t 1,5 s"), at.value(15000), -4.0, 1e-9);
		}
		{   // A02 function generator
			Waveform w; w.frequency = 2; w.amplitude = 5;
			expect(QStringLiteral("Sinus t T/4"), w.value(0.125), 5.0, 1e-9);
			w.form = 1; expect(QStringLiteral("Rechteck t 0,3 s"), w.value(0.3), -5.0, 1e-12);
			w.form = 3; expect(QStringLiteral("Dreieck t T/4"), w.value(0.125), 5.0, 1e-9);
			w.form = 4; w.offset = 2.5; expect(QStringLiteral("Konstant"), w.value(0.7), 2.5, 1e-12);
			w.form = 5; w.profile = {{0.0, 0.0}, {1.0, 10.0}}; expect(QStringLiteral("Profil t 0,5 s"), w.value(0.5), 5.0, 1e-9);
		}
		{   // A06 PT2, critically damped (d = 1): y(T) = K·(1 - 2/e)
			Pt2 g; g.t = 0.1; g.k = 2; g.d = 1;
			double y = 0;
			for (int i = 0; i < 10000; ++i) y = g.step(1.0, 1e-5);
			expect(QStringLiteral("PT2 y(T)"), y, 2 * (1 - 2 / std::exp(1.0)), 2e-4);
		}
		{   // A09 comparator 5 V ± 0.5 V, A10 amplifier jump current, A08 output range
			bool on = comparator(false, 5.4, 5, 1);
			const bool early = on;
			on = comparator(on, 5.5, 5, 1);
			const bool held = comparator(on, 4.6, 5, 1), off = comparator(on, 4.4, 5, 1);
			if (!(!early && on && held && !off)) wrong << QStringLiteral("Komparator-Hysterese falsch");
			else seen << QStringLiteral("Komparator aus 5,4 V, ein 5,5 V, hält 4,6 V, aus 4,4 V");
			expect(QStringLiteral("Verstärker 0,1 V, Sprung 10 % →"), amplifier(0.1, 0.1, false), 1.09, 1e-12);
			expect(QStringLiteral("2 Kanäle -3 V →"), amplifier(-3, 0, true), -3.0, 1e-12);
			expect(QStringLiteral("1 Kanal -3 V →"), amplifier(-3, 0, false), 0.0, 1e-12);
			expect(QStringLiteral("Bereich A1 15 V, A2 -10 V →"), supplyRange(true, 15, true, -10, 0.9, 1.5).limit, 9.0, 1e-12);
		}
		// plans: the loops close through the hydraulics
		auto sample = [&](const QString &file, double until, const QStringList &ids, QHash<QString, double> *peak) {
			QHash<QString, double> v;
			ProjectRun pr = runProject(file, 6.0, {}, until + 1e-4, [&](SimulationModel &s) {
				for (const QString &id : ids) {
					const double x = s.signalValue(id);
					if (peak && std::isfinite(x)) (*peak)[id] = std::max(peak->value(id, -1e300), x);
					if (std::abs(s.time() - until) < 5e-6) v.insert(id, x);
				}
			});
			if (!pr.error.isEmpty()) wrong << QStringLiteral("%1: %2").arg(QFileInfo(file).fileName(), pr.error);
			return v;
		};
		{
			const auto v = sample(path("druckregelung.qet"), 4.0, {QStringLiteral("p:-GP1.P"), QStringLiteral("u:-KF1")}, nullptr);
			if (!v.isEmpty()) {
				expect(QStringLiteral("Druckregelkreis PI, Sollwert 4 V: p(4 s) [bar]"), v.value(QStringLiteral("p:-GP1.P")) / 1e5, 40.0, 0.1);
				const double u = v.value(QStringLiteral("u:-KF1"));
				if (!(u > 0 && u < 10)) wrong << QStringLiteral("Druckregelkreis: Stellgröße %1 V in der Begrenzung").arg(u);
				else seen << QStringLiteral("Stellgröße %1 V").arg(u, 0, 'f', 3);
			}
		}
		{
			QHash<QString, double> peak;
			const auto v = sample(path("lageregelung.qet"), 2.0, {QStringLiteral("x:-MM1")}, &peak);
			if (!v.isEmpty()) {
				expect(QStringLiteral("Lageregelkreis P, Sollwert 6 V: x(2 s) [mm]"), v.value(QStringLiteral("x:-MM1")) * 1000, 120.0, 0.5);
				const double over = peak.value(QStringLiteral("x:-MM1")) * 1000 - 120.0;
				if (!(over < 2.0)) wrong << QStringLiteral("Lageregelkreis: Überschwingen %1 mm").arg(over);
				else seen << QStringLiteral("Überschwingen %1 mm").arg(std::max(over, 0.0), 0, 'f', 3);
			}
			// without enough supply (vMin 30 V > 24 V) the controller has no output: the cylinder stays
			const QString file = textVariant(path("lageregelung.qet"), {{QStringLiteral("vMin=21.6 V"), QStringLiteral("vMin=30 V")}},
											 QStringLiteral("qet_sim_r06_reset.qet"));
			if (file.isEmpty()) wrong << QStringLiteral("Variante ohne Versorgung nicht erzeugt");
			else {
				const auto w = sample(file, 1.0, {QStringLiteral("x:-MM1"), QStringLiteral("u:-KF1")}, nullptr);
				QFile::remove(file);
				if (!w.isEmpty()) {
					if (!(w.value(QStringLiteral("x:-MM1")) < 5e-4 && w.value(QStringLiteral("u:-KF1")) == 0.0))
						wrong << QStringLiteral("ohne Versorgung: x %1 mm, Y %2 V").arg(w.value(QStringLiteral("x:-MM1")) * 1000).arg(w.value(QStringLiteral("u:-KF1")));
					else seen << QStringLiteral("ohne Versorgung (vMin 30 V): Y 0 V, Zylinder steht");
				}
			}
		}
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 15. standard valves and cylinder forces (R11, FORMELN.md M08–M10, V14): every valve symbol of the
		//     collection, simultaneous actuation with and without a dominant side, the FluidSIM cylinder forces
		Check c{names.at(14)};
		QStringList wrong, seen;
		{
			QString where;
			QStringList bad;
			int valves = 0;
			QHash<QString, int> kinds;
			const QDir app(QCoreApplication::applicationDirPath());
			for (const QString &d : {app.filePath(QStringLiteral("elements")), app.filePath(QStringLiteral("../share/qelectrotech/elements")),
									 QETApp::commonElementsDir()}) {
				if (valves > 0 || !QFileInfo(d).isDir()) continue;
				bad = valveAudit(d, &valves, &kinds);
				where = QDir::toNativeSeparators(d);
			}
			if (valves == 0) wrong << QStringLiteral("keine Ventilsymbole gefunden");
			else if (!bad.isEmpty()) wrong << QStringLiteral("%1 von %2 Ventilsymbolen fehlerhaft: %3").arg(bad.size()).arg(valves).arg(bad.mid(0, 8).join(QStringLiteral("; ")));
			else {
				QStringList k;
				for (auto it = kinds.constBegin(); it != kinds.constEnd(); ++it) k << QStringLiteral("%1 %2").arg(it.key()).arg(it.value());
				k.sort();
				seen << QStringLiteral("%1 Ventilsymbole: Wege, Ruhestellung, Betätigung stimmig (%2)").arg(valves).arg(k.join(QStringLiteral(", ")));
			}
		}
		{   // simultaneous actuation of -QM1 (2 positions, rest 2): left 0.1 s, right too from 0.2 s, left off at 0.3 s
			auto positions = [&](const QString &file) {
				QList<double> at;
				ProjectRun pr = runProject(file, 6.0, {{0.1, QStringLiteral("-QM1"), 1, true}, {0.2, QStringLiteral("-QM1"), 2, true},
													   {0.3, QStringLiteral("-QM1"), 1, false}}, 0.4001, [&](SimulationModel &s) {
					for (const double t : {0.15, 0.25, 0.35})
						if (std::abs(s.time() - t) < 5e-6) at << s.signalValue(QStringLiteral("pos:-QM1"));
				});
				if (!pr.error.isEmpty()) wrong << QStringLiteral("%1: %2").arg(QFileInfo(file).fileName(), pr.error);
				return at;
			};
			auto list = [](const QList<double> &l) {
				QStringList s;
				for (double x : l) s << QString::number(x);
				return s.join(QLatin1Char(' '));
			};
			const QList<double> plain = positions(path("stetig.qet"));
			const QString dom = textVariant(path("stetig.qet"), {{QStringLiteral("<fluid function=\"directional_valve.four_pins\" kind=\"valve\" prefix=\"QM\">"),
																QStringLiteral("<fluid dominant=\"right\" function=\"directional_valve.four_pins\" kind=\"valve\" prefix=\"QM\">")}},
										  QStringLiteral("qet_sim_r11_vorrang.qet"));
			const QList<double> right = dom.isEmpty() ? QList<double>() : positions(dom);
			if (!dom.isEmpty()) QFile::remove(dom);
			if (plain != QList<double>{1, 1, 2} || right != QList<double>{1, 2, 2})
				wrong << QStringLiteral("gleichzeitige Betätigung: ohne Vorrang %1, Vorrang rechts %2 (soll 1 1 2 / 1 2 2)").arg(list(plain), list(right));
			else seen << QStringLiteral("gleichzeitig links + rechts: Stellung bleibt (1 1 2), mit Vorrang rechts 1 2 2");
		}
		{   // cylinder forces in hydraulik.qet: extending at the constant pump flow, the piston force difference
			// ΔpA·A_K - ΔpB·A_R at x = 100 mm to the plain plan is the added force (FORMELN.md M08–M10)
			struct Sample { double pa = std::nan(""), pb = std::nan(""), v = std::nan(""); };
			auto sample = [&](const QString &file) {
				Sample s0;
				bool done = false;
				ProjectRun pr = runProject(file, 6.0, {{0.1, QStringLiteral("-SF1"), 0, true}}, 2.0, [&](SimulationModel &s) {
					if (!done && s.signalValue(QStringLiteral("x:-MM1")) >= 0.1) {
						done = true;
						s0 = Sample{s.signalValue(QStringLiteral("p:-MM1.A")), s.signalValue(QStringLiteral("p:-MM1.B")), s.signalValue(QStringLiteral("v:-MM1"))};
					}
				});
				if (!pr.error.isEmpty()) wrong << QStringLiteral("%1: %2").arg(QFileInfo(file).fileName(), pr.error);
				return s0;
			};
			const double ak = M_PI * 0.04 * 0.04 / 4, ar = ak - M_PI * 0.02 * 0.02 / 4;
			const Sample base = sample(path("hydraulik.qet"));
			const QString cyl = QStringLiteral("<fluid function=\"actuator.cyl_conf\" kind=\"cylinder\" prefix=\"MM\">");
			const QString mext = QStringLiteral("<parameter name=\"m_ext\" unit=\"kg\" value=\"0\"/>");
			auto par = [](const char *n, const char *v, const char *u = "") {
				return QStringLiteral("<parameter name=\"%1\" unit=\"%2\" value=\"%3\"/>").arg(QLatin1String(n), QLatin1String(u), QLatin1String(v));
			};
			struct Case { const char *what; QList<QPair<QString, QString>> edit; std::function<double(const Sample &)> force; };
			const double g = 9.81, m_cyl = 2.0;   // the symbol gives no m_cyl: hydraulic default 2 kg (FORMELN.md Vorgaben)
			const double slide0 = 0.7 * 0.05 * ak * 6e5;   // default sliding friction, viscous default 20 N.s/m
			const QList<Case> cases{
				{"Gewicht 100 kg senkrecht (alpha 90°)", {{mext, par("m_ext", "100", "kg")}, {cyl, cyl + par("alpha", "90", "deg")}},
				 [&](const Sample &) { return (m_cyl + 100) * g; }},
				{"Kraftprofil 500 → 2500 N über den Hub (bei 100 mm 1500 N)", {{cyl, cyl + par("use_F_const", "False") + par("F_var_ex", "{0,500,200,2500}")}},
				 [](const Sample &) { return 1500.0; }},
				{"Feder rechts 4 … 10 bar (bei 100 mm 7 bar · A_K)", {{cyl, cyl + par("spring_right", "True") + par("p_spring_min", "4", "bar") + par("p_spring_max", "10", "bar")}},
				 [&](const Sample &) { return 7e5 * ak; }},
				{"Kolbenreibung F_B 300, F_M 200 N, F_V 1000 N·s/m", {{cyl, cyl + par("F_B", "300", "N") + par("F_M", "200", "N") + par("F_V", "1000", "N.s/m")}},
				 [&](const Sample &s) { return (200 - slide0) + (1000 - 20) * s.v; }},
				{"Gleitreibung der Last µ 0,2 bei 100 kg waagerecht", {{mext, par("m_ext", "100", "kg")}, {cyl, cyl + par("mu_M_ext", "0.2") + par("mu_B_ext", "0.25")}},
				 [&](const Sample &) { return 0.2 * 100 * g; }},
			};
			int i = 0;
			for (const Case &k : cases) {
				const QString file = textVariant(path("hydraulik.qet"), k.edit, QStringLiteral("qet_sim_r11_%1.qet").arg(i++));
				if (file.isEmpty()) { wrong << QStringLiteral("%1: Variante nicht erzeugt").arg(QString::fromUtf8(k.what)); continue; }
				const Sample s = sample(file);
				QFile::remove(file);
				const double got = (s.pa - base.pa) * ak - (s.pb - base.pb) * ar, want = k.force(s);
				if (!(std::isfinite(got) && std::abs(got - want) <= 0.02 * std::abs(want) + 2.0 && std::abs(s.v - base.v) < 0.02 * base.v))
					wrong << QStringLiteral("%1: %2 N statt %3 N (v %4 / %5 m/s)").arg(QString::fromUtf8(k.what)).arg(got).arg(want).arg(s.v).arg(base.v);
				else seen << QStringLiteral("%1: %2 N (soll %3)").arg(QString::fromUtf8(k.what)).arg(got, 0, 'f', 1).arg(want, 0, 'f', 1);
			}
		}
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 24. analog circuit (R17 stage 1, FORMELN.md E01–E05), plan analog.qet: 10 V step on R 1 kΩ + C 100 µF (τ 0.1 s),
		//     on R 10 Ω + L 1 H (τ 0.1 s), on R 10 Ω + L 0.1 H + C 100 µF (underdamped), 10 V 50 Hz on diode + R 1 kΩ,
		//     12 V on 100 Ω + 200 Ω
		Check c{names.at(23)};
		QStringList wrong, seen;
		double uc1 = std::nan(""), uc1b = std::nan(""), il2 = std::nan(""), uc3a = std::nan(""), uc3b = std::nan("");
		double ur4max = -1e9, ur4min = 1e9, ur6 = std::nan(""), pbal = std::nan("");
		auto sig = [](SimulationModel &m, const char *id) { return m.signalValue(QString::fromUtf8(id)); };
		ProjectRun r = runProject(path("analog.qet"), 6.0, {}, 0.3001, [&](SimulationModel &m) {
			const double tm = m.time();
			if (std::abs(tm - 0.1) < 5e-6) { uc1 = sig(m, "u:-C1"); il2 = sig(m, "i:-L2"); }
			if (std::abs(tm - 0.3) < 5e-6) {
				uc1b = sig(m, "u:-C1");
				ur6 = sig(m, "u:-R6");
				pbal = sig(m, "pel:-V5") + sig(m, "pel:-R5") + sig(m, "pel:-R6");
			}
			if (std::abs(tm - 0.005) < 5e-6) uc3a = sig(m, "u:-C3");
			if (std::abs(tm - 0.010) < 5e-6) uc3b = sig(m, "u:-C3");
			if (tm <= 0.04 + 1e-9) {
				const double u = sig(m, "u:-R4");
				ur4max = std::max(ur4max, u);
				ur4min = std::min(ur4min, u);
			}
		});
		if (!r.error.isEmpty()) wrong << r.error;
		auto near = [](double got, double want, double tol) { return std::isfinite(got) && std::abs(got - want) <= tol; };
		const double rc1 = 10 * (1 - std::exp(-1.0)), rc3 = 10 * (1 - std::exp(-3.0)), rl = 1 - std::exp(-1.0);
		if (!near(uc1, rc1, 0.01) || !near(uc1b, rc3, 0.01))
			wrong << QStringLiteral("RC: u_C %1 / %2 V statt %3 / %4").arg(uc1).arg(uc1b).arg(rc1).arg(rc3);
		else seen << QStringLiteral("RC: u_C(τ) %1 V (soll %2), u_C(3τ) %3 V (soll %4)").arg(uc1, 0, 'f', 4).arg(rc1, 0, 'f', 4).arg(uc1b, 0, 'f', 4).arg(rc3, 0, 'f', 4);
		if (!near(il2, rl, 0.001)) wrong << QStringLiteral("RL: i(τ) %1 A statt %2").arg(il2).arg(rl);
		else seen << QStringLiteral("RL: i(τ) %1 A (soll %2)").arg(il2, 0, 'f', 5).arg(rl, 0, 'f', 5);
		const double al = 10.0 / (2 * 0.1), w0 = 1 / std::sqrt(0.1 * 100e-6), wd = std::sqrt(w0 * w0 - al * al);
		auto rlc = [&](double tt) { return 10 * (1 - std::exp(-al * tt) * (std::cos(wd * tt) + al / wd * std::sin(wd * tt))); };
		// implicit Euler damps the oscillation by about ω₀²·Δt/2 per second (FORMELN.md E02): tolerance from that bound
		auto damp = [&](double tt) { return 10.0 * w0 * w0 * 1e-5 / 2 * tt + 0.005; };
		if (!near(uc3a, rlc(0.005), damp(0.005)) || !near(uc3b, rlc(0.010), damp(0.010)))
			wrong << QStringLiteral("RLC: u_C %1 / %2 V statt %3 / %4").arg(uc3a).arg(uc3b).arg(rlc(0.005)).arg(rlc(0.010));
		else seen << QStringLiteral("RLC (α 50/s, ω_d %1/s): u_C(5 ms) %2 V (soll %3), u_C(10 ms) %4 V (soll %5, Euler-Dämpfung bis %6 V)")
						 .arg(wd, 0, 'f', 1).arg(uc3a, 0, 'f', 3).arg(rlc(0.005), 0, 'f', 3).arg(uc3b, 0, 'f', 3).arg(rlc(0.010), 0, 'f', 3)
						 .arg(damp(0.010), 0, 'f', 3);
		// diode peak: 10 V = v_D + R·Is·(exp(v_D/V_T) − 1)
		const double vt = 1.380649e-23 * 293.15 / 1.602176634e-19;
		double lo = 0.0, hi = 1.0;
		for (int i = 0; i < 200; ++i) { const double mid = 0.5 * (lo + hi); ((mid + 1000 * 1e-12 * (std::exp(mid / vt) - 1) > 10.0) ? hi : lo) = mid; }
		const double peak = 10.0 - 0.5 * (lo + hi);
		if (!near(ur4max, peak, 0.02) || !(ur4min > -1e-3))
			wrong << QStringLiteral("Gleichrichter: Spitze %1 V statt %2, kleinster Wert %3 V").arg(ur4max).arg(peak).arg(ur4min);
		else seen << QStringLiteral("Einweggleichrichter: Spitze %1 V (soll 10 − U_D = %2), Sperrhalbwelle %3 V").arg(ur4max, 0, 'f', 3).arg(peak, 0, 'f', 3).arg(ur4min, 0, 'g', 3);
		if (!near(ur6, 8.0, 1e-6) || !near(pbal, 0.0, 1e-9))
			wrong << QStringLiteral("Teiler: %1 V statt 8, Leistungsbilanz %2 W").arg(ur6).arg(pbal);
		else seen << QStringLiteral("Spannungsteiler %1 V (soll 8), Leistungsbilanz Quelle + Widerstände %2 W").arg(ur6, 0, 'f', 6).arg(pbal, 0, 'g', 3);
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 25. coupling of the analog circuit (R17 stage 2, FORMELN.md E06–E08), plan kopplung.qet: 24 V, push button -S1
		//     (switch R_on 10 mΩ) on relay coil -K1 (R 240 Ω, L 2.4 H, pick-up 0.8·Un/R, drop-out 0.1·Un/R) with freewheeling
		//     diode -D1; the make contact of -K1 switches 10 V onto R2 1 kΩ; divider 1 kΩ/1 kΩ with AI -A1 (Rin 100 kΩ,
		//     0…10 V ↔ 0…27648) and AQ -A2 driven by the same variable onto R5 1 kΩ. Button pressed 0.1…0.3 s
		Check c{names.at(24)};
		QStringList wrong, seen;
		double ir2_before = std::nan(""), ir2_on = std::nan(""), ik1 = std::nan(""), ir2_after = std::nan("");
		double ai = std::nan(""), ua2 = std::nan(""), ur5 = std::nan("");
		auto sig = [](SimulationModel &m, const char *id) { return m.signalValue(QString::fromUtf8(id)); };
		ProjectRun r = runProject(path("kopplung.qet"), 6.0, {{0.1, QStringLiteral("-S1"), 0, true}, {0.3, QStringLiteral("-S1"), 0, false}},
								  0.4001, [&](SimulationModel &m) {
			const double tm = m.time();
			if (std::abs(tm - 0.09) < 5e-6) ir2_before = sig(m, "i:-R2");
			if (std::abs(tm - 0.29) < 5e-6) {
				ir2_on = sig(m, "i:-R2");
				ik1 = sig(m, "i:-K1");
				ai = sig(m, "plc:Messwert");
				ua2 = sig(m, "u:-A2");
				ur5 = sig(m, "u:-R5");
			}
			if (std::abs(tm - 0.39) < 5e-6) ir2_after = sig(m, "i:-R2");
		});
		if (!r.error.isEmpty()) wrong << r.error;
		auto near = [](double got, double want, double tol) { return std::isfinite(got) && std::abs(got - want) <= tol; };
		const double R = 240.0, L = 2.4, un = 24.0, ron = 0.01, i_on = 0.8 * un / R, i_off = 0.1 * un / R;
		const double tau = L / (R + ron), i_end = un / (R + ron);
		const double t_pick = 0.1 - tau * std::log(1.0 - i_on / i_end) + 0.010;     // + pull-in time of the relay
		// drop-out: the coil current decays through the diode, L·di/dt = −(R·i + n·V_T·ln(1 + i/Is)); RK4, 0.1 µs steps
		const double vt = 1.380649e-23 * 293.15 / 1.602176634e-19;
		auto didt = [&](double i) { return -(R * i + vt * std::log1p(i / 1e-12)) / L; };
		double i = i_end, tt = 0.0;
		const double h = 1e-7;
		while (i > i_off && tt < 0.1) {
			const double k1 = didt(i), k2 = didt(i + h / 2 * k1), k3 = didt(i + h / 2 * k2), k4 = didt(i + h * k3);
			i += h / 6 * (k1 + 2 * k2 + 2 * k3 + k4);
			tt += h;
		}
		const double t_drop = 0.3 + tt + 0.008;                                      // + drop-out time of the relay
		const double on_at = r.eventTime(QStringLiteral("Relais -K1 zieht an")), off_at = r.eventTime(QStringLiteral("Relais -K1 fällt ab"));
		if (!near(on_at, t_pick, 5e-5) || !near(off_at, t_drop, 5e-5))
			wrong << QStringLiteral("Relais: zieht an bei %1 s (soll %2), fällt ab bei %3 s (soll %4)").arg(on_at).arg(t_pick).arg(off_at).arg(t_drop);
		else seen << QStringLiteral("Relais zieht an bei %1 s (soll 0,1 + τ·ln(1/(1−Ion/I∞)) + 10 ms = %2), fällt ab bei %3 s "
									"(Freilauf über die Diode, soll %4)").arg(on_at, 0, 'f', 5).arg(t_pick, 0, 'f', 5).arg(off_at, 0, 'f', 5).arg(t_drop, 0, 'f', 5);
		if (!near(ik1, i_end, 1e-6)) wrong << QStringLiteral("Spulenstrom %1 A statt %2").arg(ik1).arg(i_end);
		else seen << QStringLiteral("Spulenstrom %1 A (soll Un/(R+Ron) = %2)").arg(ik1, 0, 'f', 6).arg(i_end, 0, 'f', 6);
		if (!(std::abs(ir2_before) < 1e-6) || !near(ir2_on, 10.0 / (1000.0 + ron), 1e-9) || !(std::abs(ir2_after) < 1e-6))
			wrong << QStringLiteral("Relaiskontakt: i(R2) %1 / %2 / %3 A").arg(ir2_before).arg(ir2_on).arg(ir2_after);
		else seen << QStringLiteral("Relaiskontakt: i(R2) vor %1 A, angezogen %2 A (soll 10 V/(1 kΩ+Ron)), abgefallen %3 A")
						 .arg(ir2_before, 0, 'g', 3).arg(ir2_on, 0, 'f', 7).arg(ir2_after, 0, 'g', 3);
		const double rp = 1000.0 * 1e5 / (1000.0 + 1e5), u4 = 10.0 * rp / (1000.0 + rp), raw = 27648.0 * u4 / 10.0;
		if (!near(ai, raw, 1e-3) || !near(ua2, u4, 1e-6) || !near(ur5, u4, 1e-6))
			wrong << QStringLiteral("SPS-Analog: Eingang %1 (soll %2), Ausgang %3 / %4 V (soll %5)").arg(ai).arg(raw).arg(ua2).arg(ur5).arg(u4);
		else seen << QStringLiteral("SPS-Analogeingang %1 (soll 27648·%2 V/10 V = %3, Rin belastet den Teiler), Ausgang %4 V an R5")
						 .arg(ai, 0, 'f', 2).arg(u4, 0, 'f', 5).arg(raw, 0, 'f', 2).arg(ur5, 0, 'f', 5);
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 26. semiconductors (R17 stage 3, FORMELN.md E09, E10), plan halbleiter.qet: NPN with Rc 1 kΩ on 10 V, base over
		//     470 kΩ (active) or 10 kΩ (saturated) from 5 V; PNP emitter on 10 V, base 470 kΩ and collector 1 kΩ to ground;
		//     inverting amplifier Ri 1 kΩ, Rf 10 kΩ, load 10 kΩ with 0.5 V (linear) and 2 V (limited). Expected values from
		//     the device equations solved by bisection, independent of the nodal solver
		Check c{names.at(25)};
		QStringList wrong, seen;
		ProjectRun r = runProject(path("halbleiter.qet"), 6.0, {}, 0.001);
		if (!r.error.isEmpty()) wrong << r.error;
		else {
			auto sig = [&](const QString &id) { return r.sim->signalValue(id); };
			const double vt = 1.380649e-23 * 293.15 / 1.602176634e-19, is = 1e-14, bf = 100, br = 1, k = 1 + 1 / br;
			auto ic = [&](double vbe, double vce) { return is * (std::exp(vbe / vt) - 1) - k * is * (std::exp((vbe - vce) / vt) - 1); };
			auto ib = [&](double vbe, double vce) { return is * (std::exp(vbe / vt) - 1) / bf + is * (std::exp((vbe - vce) / vt) - 1) / br; };
			// returns {Ic, Ib, Uce} for base supply vbb over rb and collector supply vcc over rc (magnitudes, PNP mirrored)
			auto bjt = [&](double vbb, double rb, double vcc, double rc) {
				auto vceOf = [&](double vbe) {
					double lo = -0.5, hi = vcc;
					for (int i = 0; i < 200; ++i) { const double m = 0.5 * (lo + hi); ((vcc - m) / rc - ic(vbe, m) > 0 ? lo : hi) = m; }
					return 0.5 * (lo + hi);
				};
				double lo = 0.0, hi = 1.0;
				for (int i = 0; i < 200; ++i) { const double m = 0.5 * (lo + hi); ((vbb - m) / rb - ib(m, vceOf(m)) > 0 ? lo : hi) = m; }
				const double vbe = 0.5 * (lo + hi), vce = vceOf(vbe);
				return std::array<double, 3>{ic(vbe, vce), ib(vbe, vce), vce};
			};
			auto rel = [](double got, double want, double tol) { return std::isfinite(got) && std::abs(got - want) <= tol * std::abs(want); };
			const struct { const char *label, *name; double vbb, rb, sign; } cases[] = {
				{"-Q1", "NPN aktiv", 5.0, 470e3, 1.0}, {"-Q2", "NPN gesättigt", 5.0, 10e3, 1.0}, {"-Q3", "PNP aktiv", 10.0, 470e3, -1.0}};
			for (const auto &cs : cases) {
				const auto want = bjt(cs.vbb, cs.rb, 10.0, 1000.0);
				const QString l = QString::fromLatin1(cs.label);
				const double gi = cs.sign * sig(QStringLiteral("i:") + l), gb = cs.sign * sig(QStringLiteral("ib:") + l), gu = cs.sign * sig(QStringLiteral("u:") + l);
				if (!rel(gi, want[0], 1e-6) || !rel(gb, want[1], 1e-6) || !rel(gu, want[2], 1e-6))
					wrong << QStringLiteral("%1: I_C %2 A, I_B %3 A, U_CE %4 V statt %5 / %6 / %7").arg(QString::fromUtf8(cs.name))
								 .arg(gi).arg(gb).arg(gu).arg(want[0]).arg(want[1]).arg(want[2]);
				else seen << QStringLiteral("%1: I_C %2 mA, I_B %3 µA (B = %4), U_CE %5 V (soll %6 mA / %7 µA / %8 V)")
								 .arg(QString::fromUtf8(cs.name)).arg(gi * 1e3, 0, 'f', 5).arg(gb * 1e6, 0, 'f', 4).arg(gi / gb, 0, 'f', 2)
								 .arg(gu, 0, 'f', 5).arg(want[0] * 1e3, 0, 'f', 5).arg(want[1] * 1e6, 0, 'f', 4).arg(want[2], 0, 'f', 5);
			}
			// op-amp: E = Vh·tanh(A·(v+ − v−)/Vh), output over Rout, Rin between the inputs; bisection on v−
			const double A = 1e5, rin = 1e6, rout = 1.0, vh = 15.0, ri = 1e3, rf = 10e3, rl = 10e3;
			auto opamp = [&](double vi) {
				auto vo = [&](double vn) { const double e = vh * std::tanh(A * (0.0 - vn) / vh);
					return (e / rout + vn / rf) / (1 / rout + 1 / rf + 1 / rl); };
				double lo = -2.0, hi = 2.0;
				for (int i = 0; i < 200; ++i) { const double m = 0.5 * (lo + hi);
					((vi - m) / ri + (vo(m) - m) / rf - m / rin > 0 ? lo : hi) = m; }
				return vo(0.5 * (lo + hi));
			};
			const double vo4 = sig(QStringLiteral("u:-N4")), vo5 = sig(QStringLiteral("u:-N5"));
			const double w4 = opamp(0.5), w5 = opamp(2.0);
			if (!rel(vo4, w4, 1e-6) || !rel(vo5, w5, 1e-6) || !(vo5 > -15.0))
				wrong << QStringLiteral("OPV: %1 / %2 V statt %3 / %4").arg(vo4).arg(vo5).arg(w4).arg(w5);
			else seen << QStringLiteral("OPV invertierend 0,5 V: %1 V (soll %2, ideal −5), 2 V: %3 V (soll %4, Grenze −15)")
							 .arg(vo4, 0, 'f', 6).arg(w4, 0, 'f', 6).arg(vo5, 0, 'f', 4).arg(w5, 0, 'f', 4);
		}
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 23. simulation modes (R16, FORMELN.md N10): run to the next state change on hydraulik.qet (valve switch, then end
		//     position at the times of the continuous run), single step of one control cycle, zwei_blaetter.qet run whole and
		//     folio 2 alone (the throttle circuit gives the same pressure), steps per second
		Check c{names.at(22)};
		QStringList wrong, seen;
		ProjectRun ref = runProject(path("hydraulik.qet"), 6.0, {{0.3, QStringLiteral("-SF1"), 0, true}}, 3.0);
		const double t_valve = ref.eventTime(QStringLiteral("schaltet in Stellung"), 0.3), t_end = ref.eventTime(QStringLiteral("Endlage vorne"), 0.3);
		QETProject proj(path("hydraulik.qet"));
		if (!ref.error.isEmpty() || proj.state() != QETProject::Ok) wrong << QStringLiteral("hydraulik.qet: %1").arg(ref.error);
		else {
			SimulationModel m(proj.diagrams(), 6.0);
			m.runUntil(0.3);
			m.pressButton(QStringLiteral("-SF1"), true);
			m.takeMessages();
			const bool c1 = m.runUntilChange(5.0);
			const double t1 = m.time();
			const QString e1 = m.takeMessages().value(0).text;
			const bool c2 = m.runUntilChange(5.0);
			const double t2 = m.time();
			bool c3 = false;
			double t_end_found = t2;
			for (const auto &msg : m.takeMessages())
				if (msg.text.contains(QLatin1String("Endlage vorne"))) { c3 = true; t_end_found = msg.time; }
			// later stops may be the relay or the coil first: go on change by change until the end position
			while (!c3 && m.runUntilChange(5.0))
				for (const auto &msg : m.takeMessages())
					if (msg.text.contains(QLatin1String("Endlage vorne"))) { c3 = true; t_end_found = msg.time; }
			if (!c1 || !c2 || !c3 || !(std::abs(t_end_found - t_end) < 1e-6) || t1 > t_valve + 1e-9 || t1 < 0.3)
				wrong << QStringLiteral("bis zur Änderung: erster Halt %1 s (%2), Endlage %3 s statt %4").arg(t1).arg(e1).arg(t_end_found).arg(t_end);
			else seen << QStringLiteral("bis zur Änderung: Halt bei %1 s („%2“), Ventil %3 s, Endlage %4 s wie im Dauerlauf")
							 .arg(t1, 0, 'f', 5).arg(e1).arg(t_valve, 0, 'f', 5).arg(t_end_found, 0, 'f', 5);
			const double before = m.time();
			m.runUntil(before + 0.010);
			if (!(std::abs(m.time() - before - 0.010) < 1e-9)) wrong << QStringLiteral("Einzelschritt %1 s statt 0,010").arg(m.time() - before);
			else seen << QStringLiteral("Einzelschritt 10 ms (%1 → %2 s)").arg(before, 0, 'f', 4).arg(m.time(), 0, 'f', 4);
		}
		QETProject two(path("zwei_blaetter.qet"));
		if (two.state() != QETProject::Ok || two.diagrams().size() != 2) wrong << QStringLiteral("zwei_blaetter.qet fehlt oder hat nicht zwei Blätter");
		else {
			auto run = [&](const QList<Diagram *> &ds, double *p2, double *p1, long long *steps, double *secs) {
				SimulationModel m(ds, 6.0);
				QElapsedTimer w;
				w.start();
				m.runUntil(1.0);
				*secs = w.nsecsElapsed() / 1e9;
				*steps = m.stepCount();
				*p2 = m.signalValue(QStringLiteral("p:-GZ2.P2")) / 1e5;
				*p1 = m.signalValue(QStringLiteral("p:-GZ1.P2")) / 1e5;
				return m.netCount();
			};
			double pa2 = 0, pa1 = 0, pb2 = 0, pb1 = 0, sa = 0, sb = 0;
			long long na = 0, nb = 0;
			const int nets_all = run(two.diagrams(), &pa2, &pa1, &na, &sa);
			const int nets_two = run({two.diagrams().at(1)}, &pb2, &pb1, &nb, &sb);
			if (!(std::abs(pa2 - pb2) < 1e-9) || std::isfinite(pb1) || !(nets_two < nets_all) || na != nb)
				wrong << QStringLiteral("Teilprojekt: Blatt 2 %1 bar (ganz %2), Blatt 1 im Teilprojekt %3, Netze %4/%5").arg(pb2).arg(pa2).arg(pb1).arg(nets_two).arg(nets_all);
			else seen << QStringLiteral("nur Blatt 2: %1 bar wie im ganzen Projekt, %2 statt %3 Netze, Blatt 1 nicht gerechnet; "
										"%4 Schritte/s (ganz %5)")
							 .arg(pb2, 0, 'f', 3).arg(nets_two).arg(nets_all).arg(nb / std::max(sb, 1e-9), 0, 'f', 0).arg(na / std::max(sa, 1e-9), 0, 'f', 0);
		}
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 22. faults (R15, FORMELN.md K16): hydraulik.qet (-SF1 at 0.3 s extends -MM1) with a fault set on one part, and
		//     fehler.qet: -RM1 check valve always closed (pump to its relief), -RM2 always open (fed backwards, free
		//     resistance 0.05), -RZ3 throttle clogged to 80 %, -MM4 load unit with an internal leak 0.5 l/(min.bar) A1.1 → A1.2
		Check c{names.at(21)};
		QStringList wrong, seen;
		const QString hyd = path("hydraulik.qet");
		const QList<Input> press{{0.3, QStringLiteral("-SF1"), 0, true}};
		auto ends = [](const ProjectRun &r) { return r.eventTime(QStringLiteral("Endlage vorne"), 0.0); };
		auto events = [](const ProjectRun &r) {
			QStringList s;
			for (const auto &e : r.events) s << QStringLiteral("%1 %2").arg(e.first, 0, 'f', 5).arg(e.second);
			return s.join(QLatin1Char('|'));
		};
		auto variant = [&](const char *type, const char *fault, const char *name, const QList<Input> &in) {
			const QString f = withFault(hyd, QString::fromUtf8(type), QString::fromUtf8(fault), QString::fromUtf8(name));
			ProjectRun r = f.isEmpty() ? ProjectRun{} : runProject(f, 6.0, in, 3.5);
			if (f.isEmpty()) r.error = QStringLiteral("Variante %1 nicht erzeugt").arg(QString::fromUtf8(name));
			if (!f.isEmpty()) QFile::remove(f);
			return r;
		};
		ProjectRun base = runProject(hyd, 6.0, press, 3.5);
		ProjectRun never = variant("poussoir.elmt", "schaltet_nicht", "qet_sim_r15_taster.qet", press);
		QList<Input> fix = press;
		fix << Input{1.0, QStringLiteral("-QM1"), 4, true};
		ProjectRun coil = variant("electrovanne.elmt", "spule_unterbrochen", "qet_sim_r15_spule.qet", fix);
		ProjectRun coil2 = variant("electrovanne.elmt", "spule_unterbrochen", "qet_sim_r15_spule2.qet", fix);
		ProjectRun stuck = variant("distributeur_4-2.elmt", "kolben_klemmt bis 1.2", "qet_sim_r15_kolben.qet", press);
		for (const ProjectRun *r : {&base, &never, &coil, &coil2, &stuck})
			if (!r->error.isEmpty()) wrong << r->error;
		const double t0 = ends(base);
		if (wrong.isEmpty()) {
			if (std::isfinite(ends(never)) || !std::isfinite(never.eventTime(QStringLiteral("Fehler -SF1: schaltet_nicht"), 0.0)))
				wrong << QStringLiteral("Taster schaltet nicht: Zylinder fährt trotzdem (%1 s)").arg(ends(never));
			else seen << QStringLiteral("Taster schaltet nicht: Zylinder bleibt hinten (ohne Fehler vorne bei %1 s)").arg(t0, 0, 'f', 3);
			const double dc = ends(coil) - t0, ds = ends(stuck) - t0;
			if (!(std::abs(dc - 0.7) <= 0.011) || !std::isfinite(coil.eventTime(QStringLiteral("-QM1 repariert"), 0.0)))
				wrong << QStringLiteral("Spule unterbrochen bis zur Reparatur bei 1 s: vorne %1 s später statt 0,7").arg(dc);
			else seen << QStringLiteral("Spule unterbrochen, repariert bei 1,0 s: %1 s später vorne (soll 0,7)").arg(dc, 0, 'f', 3);
			if (!(std::abs(ds - 0.9) <= 0.011) || !std::isfinite(stuck.eventTime(QStringLiteral("Fehler -QM1 aufgehoben"), 0.0)))
				wrong << QStringLiteral("Kolben klemmt bis 1,2 s: vorne %1 s später statt 0,9").arg(ds);
			else seen << QStringLiteral("Kolben klemmt bis 1,2 s: %1 s später (soll 0,9)").arg(ds, 0, 'f', 3);
			if (events(coil) != events(coil2))
				wrong << QStringLiteral("derselbe Fehler gibt einen anderen Verlauf");
			else seen << QStringLiteral("Wiederholung gleich (%1 Ereignisse)").arg(coil.events.size());
		}
		// fluid faults
		const double qf = 910 * 4.5055e-3, glf = 0.003333;
		const double ak = M_PI * 0.016 * 0.016 / 4;
		auto bisect = [](auto fn, double lo, double hi) {
			for (int i = 0; i < 200; ++i) { const double mid = 0.5 * (lo + hi); (fn(lo) * fn(mid) <= 0 ? hi : lo) = mid; }
			return 0.5 * (lo + hi);
		};
		auto h03 = [](double d, double rt) { return d / (std::sqrt(rt) * std::pow(d * d + 0.09, 0.25)); };
		const double relief = bisect([&](double pb) { return qf - glf * pb - (pb - 50) / 35 * std::sqrt(pb / 0.08); }, 50.0, 85.0);
		const double p_open = bisect([&](double pb) { return h03(pb, 0.05) - (qf - glf * pb); }, 0.0, 50.0);
		const double p_clog = bisect([&](double pb) { return 0.8 * h03(pb, 1.0) - (qf - glf * pb); }, 0.0, 50.0);
		double p1 = std::nan(""), p2 = std::nan(""), p3 = std::nan(""), p4 = std::nan(""), v4 = std::nan("");
		ProjectRun f = runProject(path("fehler.qet"), 0.0, {}, 2.9001, [&](SimulationModel &m) {
			if (std::abs(m.time() - 0.3) < 5e-6) { v4 = m.signalValue(QStringLiteral("v:-MM4")); p4 = m.signalValue(QStringLiteral("p:-GZ4.P2")) / 1e5; }
			if (std::abs(m.time() - 2.9) < 5e-6) {
				p1 = m.signalValue(QStringLiteral("p:-GZ1.P2")) / 1e5;
				p2 = m.signalValue(QStringLiteral("p:-GZ2.P2")) / 1e5;
				p3 = m.signalValue(QStringLiteral("p:-GZ3.P2")) / 1e5;
			}
		});
		if (!f.error.isEmpty()) wrong << f.error;
		auto near = [](double got, double want, double tol) { return std::isfinite(got) && std::abs(got - want) <= tol; };
		if (!near(p1, relief, 0.05)) wrong << QStringLiteral("Rückschlagventil gesperrt: %1 bar statt %2").arg(p1).arg(relief);
		else seen << QStringLiteral("Rückschlagventil immer gesperrt: Pumpe auf %1 bar (Druckbegrenzung %2)").arg(p1, 0, 'f', 2).arg(relief, 0, 'f', 2);
		if (!near(p2, p_open, 0.01)) wrong << QStringLiteral("Rückschlagventil offen: %1 bar statt %2").arg(p2).arg(p_open);
		else seen << QStringLiteral("immer geöffnet, rückwärts: %1 bar (soll %2)").arg(p2, 0, 'f', 3).arg(p_open, 0, 'f', 3);
		if (!near(p3, p_clog, 0.05)) wrong << QStringLiteral("Drossel verstopft: %1 bar statt %2").arg(p3).arg(p_clog);
		else seen << QStringLiteral("Drossel verstopft auf 80 %: %1 bar (soll %2, frei %3)").arg(p3, 0, 'f', 2).arg(p_clog, 0, 'f', 2)
						 .arg(bisect([&](double pb) { return h03(pb, 1.0) - (qf - glf * pb); }, 0.0, 50.0), 0, 'f', 2);
		const double balance = qf - glf * p4 - 0.5 * p4 - v4 * ak * 60000.0;
		if (!std::isfinite(balance) || std::abs(balance) > 0.01 * qf)
			wrong << QStringLiteral("Leckage: Bilanz %1 l/min (p %2 bar, v %3 m/s)").arg(balance).arg(p4).arg(v4);
		else seen << QStringLiteral("Leckage 0,5 l/(min·bar) bei %1 bar: %2 l/min am Kolben vorbei, v %3 m/s")
						 .arg(p4, 0, 'f', 2).arg(0.5 * p4, 0, 'f', 3).arg(v4, 0, 'f', 4);
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 21. GRAFCET structuring (R14, FORMELN.md D01–D05), plan grafcet_r14.qet; cycle 10 ms:
		//     G1: 0 → (0.1 s) enclosing 1 (G2: 21 ⇄ 22 every 0.1 s) → (0.55 s) macro M3 (E3 → 0.2 s → S3) → step 4 forces
		//     G5{52} for 0.3 s → 5 → (N ≥ 1.5) → 6; G5: 50 ⇄ 51 every 0.1 s, 52 → 50; G6: 1 ⇄ 4 every 0.25 s (step numbers
		//     of G1 again), N := N + 0.5 on activating G6 step 4
		ProjectRun r = runProject(path("grafcet_r14.qet"), 6.0, {}, 1.3);
		if (!r.error.isEmpty()) out << failedRun(names.at(20), r);
		else {
			Check c{names.at(20)};
			QStringList wrong, seen;
			auto times = [&](const QString &step) {
				QList<double> out;
				for (const auto &e : r.events)
					if (e.second == QStringLiteral("GRAFCET Schritt %1 aktiv").arg(step)) out << e.first;
				return out;
			};
			auto expect = [&](const QString &step, const QList<double> &want, const char *what) {
				const QList<double> got = times(step);
				bool ok = got.size() == want.size();
				for (int i = 0; ok && i < got.size(); ++i) ok = std::abs(got.at(i) - want.at(i)) <= 0.011;
				QStringList g;
				for (double x : got) g << QString::number(x, 'f', 2);
				if (!ok) wrong << QStringLiteral("%1: Schritt %2 bei %3 s").arg(QString::fromUtf8(what), step, g.join(QLatin1Char('/')));
				return ok;
			};
			bool all = true;
			all &= expect(QStringLiteral("22"), {0.2, 0.4, 0.6}, "eingeschlossen");
			all &= expect(QStringLiteral("21"), {0.1, 0.3, 0.5}, "Aktivierungsschritt");
			all &= expect(QStringLiteral("M3"), {0.65}, "Makroschritt");
			all &= expect(QStringLiteral("E3"), {0.65}, "Eingangsschritt");
			all &= expect(QStringLiteral("S3"), {0.85}, "Ausgangsschritt");
			all &= expect(QStringLiteral("G1.4"), {0.85}, "nach dem Makroschritt");
			all &= expect(QStringLiteral("51"), {0.1, 0.3, 0.5, 0.7, 1.25}, "G5 frei, nach dem Zwang weiter");
			all &= expect(QStringLiteral("52"), {0.85}, "G5 gezwungen");
			all &= expect(QStringLiteral("50"), {0.0, 0.2, 0.4, 0.6, 0.8, 1.15}, "G5 nach dem Zwang");
			all &= expect(QStringLiteral("G6.4"), {0.25, 0.75, 1.25}, "Instanz G6");
			all &= expect(QStringLiteral("6"), {1.25}, "Analogwert N");
			const double n = r.sim->plcValue(QStringLiteral("N"));
			if (!(std::abs(n - 1.5) < 1e-9)) { all = false; wrong << QStringLiteral("N %1 statt 1.5").arg(n); }
			const double forced = r.eventTime(QStringLiteral("GRAFCET G5 erzwungen: G5{52}"));
			if (!(std::abs(forced - 0.85) <= 0.011)) { all = false; wrong << QStringLiteral("Zwangssteuerung bei %1 s statt 0,85").arg(forced); }
			if (!r.sim->programErrors().isEmpty()) { all = false; wrong << r.sim->programErrors(); }
			c.passed = all;
			c.detail = all ? QStringLiteral("G2 läuft nur in Schritt 1 (21/22 bis 0,6 s, aus bei 0,65 s); M3 → E3 0,65 s → S3 0,85 s → 4; "
											"G5{52} von 0,85 bis 1,15 s, dann 50; G6 mit eigenen Schritten 1/4 (0,25/0,75/1,25 s); "
											"N = 1,5 → Schritt 6 bei 1,25 s")
						   : wrong.join(QStringLiteral("; "));
			out << c;
		}
	}
	{   // 20. LS control block and priority valve (R13, FORMELN.md V17, V18), plan lsblock.qet: the fixed pump P2 of -GZ1
		//     (relief 50 bar, variant 30 bar) feeds the block -QM1 and the joystick -KH1 (left +30 %: pilot 4.5 bar, stroke
		//     (4.5 − 2)/13); section 1 drives the load unit -MM1; -GZ2 feeds the priority valve -QV1, CF over -RZ1 (rT 1)
		//     to the air, EF and LS open; retracting (−30 %) with the relief at 200 bar over a 1 m stroke
		Check c{names.at(19)};
		QStringList wrong, seen;
		const double qf = 910 * 4.5055e-3, glf = 0.003333;
		const double ak = M_PI * 0.016 * 0.016 / 4, ar = ak - M_PI * 0.01 * 0.01 / 4;
		auto bisect = [](auto fn, double lo, double hi) {
			for (int i = 0; i < 200; ++i) { const double mid = 0.5 * (lo + hi); (fn(lo) * fn(mid) <= 0 ? hi : lo) = mid; }
			return 0.5 * (lo + hi);
		};
		auto h03 = [](double d, double rt) { return d / (std::sqrt(rt) * std::pow(d * d + 0.09, 0.25)); };
		const double x = (4.5 - 2.0) / 13.0;
		const double q_lo = x * std::sqrt(10.5 / 0.1), q_hi = x * std::sqrt(11.0 / 0.1);   // compensator band
		struct Sample { double v = std::nan(""), pa = std::nan(""), pls = std::nan(""), pp = std::nan(""), pcf = std::nan(""), pp2 = std::nan(""); };
		auto sig = [](SimulationModel &m, const char *id) { return m.signalValue(QString::fromUtf8(id)); };
		auto run = [&](const QString &file, double t_v) {
			Sample s;
			ProjectRun pr = runProject(file, 0.0, {}, 2.9001, [&](SimulationModel &m) {
				if (std::abs(m.time() - t_v) < 5e-6) {
					s.v = sig(m, "v:-MM1");
					s.pa = sig(m, "p:-MM1.A1.1") / 1e5;
					s.pls = sig(m, "p:-QM1.LS") / 1e5;
					s.pp = sig(m, "p:-GZ1.P2") / 1e5;
				}
				if (std::abs(m.time() - 2.9) < 5e-6) {
					s.pcf = sig(m, "p:-QV1.CF") / 1e5;
					s.pp2 = sig(m, "p:-GZ2.P2") / 1e5;
				}
			});
			if (!pr.error.isEmpty()) wrong << QStringLiteral("%1: %2").arg(QFileInfo(file).fileName(), pr.error);
			return s;
		};
		auto variant = [&](const QList<QPair<QString, QString>> &subst, const char *name, double t_v) {
			const QString f = textVariant(path("lsblock.qet"), subst, QString::fromUtf8(name));
			const Sample s = f.isEmpty() ? Sample{} : run(f, t_v);
			if (!f.isEmpty()) QFile::remove(f);
			return s;
		};
		const Sample a = run(path("lsblock.qet"), 0.3);
		const Sample lo = variant({{QStringLiteral("<parameter name=\"p0_prvFixed\" unit=\"MPa\" value=\"5\"/>"),
									QStringLiteral("<parameter name=\"p0_prvFixed\" unit=\"MPa\" value=\"3\"/>")}}, "qet_sim_r13_ls30.qet", 0.3);
		const Sample back = variant({{QStringLiteral("<parameter name=\"adjustmentLeft\" unit=\"%\" value=\"30\"/>"),
									  QStringLiteral("<parameter name=\"adjustmentLeft\" unit=\"%\" value=\"-30\"/>")},
									 {QStringLiteral("<parameter name=\"xStart\" unit=\"mm\" value=\"0\"/>"),
									  QStringLiteral("<parameter name=\"xStart\" unit=\"mm\" value=\"1000\"/>")},
									 {QStringLiteral("<parameter name=\"xMax\" unit=\"mm\" value=\"200\"/>"),
									  QStringLiteral("<parameter name=\"xMax\" unit=\"mm\" value=\"1000\"/>")},
									 // the return pressure builds up in the net volumes (V_HYD 0.1 l each): 1 m stroke, sampled at 1.5 s;
									 // retracting, the piston side returns 1.64 × the flow over rT_TAB/x²: about 100 bar on the rod
									 // side, so the compensator needs the relief at 200 bar to keep regulating
									 {QStringLiteral("<parameter name=\"p0_prvFixed\" unit=\"MPa\" value=\"5\"/>"),
									  QStringLiteral("<parameter name=\"p0_prvFixed\" unit=\"MPa\" value=\"20\"/>")}}, "qet_sim_r13_lsback.qet", 1.5);
		auto inBand = [&](double q) { return std::isfinite(q) && q >= q_lo * 0.999 && q <= q_hi * 1.001; };
		const double q1 = a.v * ak * 60000.0, q2 = lo.v * ak * 60000.0, q3 = -back.v * ar * 60000.0;
		if (!inBand(q1) || !inBand(q2) || !(std::abs(q1 - q2) <= 0.01 * q1) || !(a.pp - lo.pp > 15.0))
			wrong << QStringLiteral("Druckwaage: %1 l/min bei %2 bar, %3 l/min bei %4 bar, Band %5 … %6").arg(q1).arg(a.pp).arg(q2).arg(lo.pp).arg(q_lo).arg(q_hi);
		else seen << QStringLiteral("Scheibe 1 bei 30 %: %1 l/min (Pumpe %2 bar) und %3 l/min (Pumpe %4 bar), Band der Druckwaage %5 … %6")
						 .arg(q1, 0, 'f', 3).arg(a.pp, 0, 'f', 1).arg(q2, 0, 'f', 3).arg(lo.pp, 0, 'f', 1).arg(q_lo, 0, 'f', 3).arg(q_hi, 0, 'f', 3);
		if (!(std::abs(a.pls - a.pa) <= 0.1)) wrong << QStringLiteral("Lastmeldung: LS %1 bar, A1 %2 bar").arg(a.pls).arg(a.pa);
		else seen << QStringLiteral("LS %1 bar = Lastdruck A1 %2 bar").arg(a.pls, 0, 'f', 2).arg(a.pa, 0, 'f', 2);
		if (!inBand(q3)) wrong << QStringLiteral("Rückhub: %1 l/min außerhalb %2 … %3 (Pumpe %4 bar)").arg(q3).arg(q_lo).arg(q_hi).arg(back.pp);
		else seen << QStringLiteral("−30 %: %1 m/s, %2 l/min in die Stangenseite (Pumpe %3 bar)").arg(back.v, 0, 'f', 4).arg(q3, 0, 'f', 3).arg(back.pp, 0, 'f', 1);
		// priority valve: CF over rT 1 to the air, EF open; unknowns p_CF and p_P (H03 orifices)
		double want_p = std::nan("");
		const double want_cf = bisect([&](double pcf) {
			const double s = std::clamp(1.0 - (pcf - 8.0) / 2.4, 1e-6, 1.0 - 1e-6);
			const double q3c = h03(pcf, 1.0);
			const double pp = bisect([&](double pb) { return h03(pb - pcf, 0.9 / (s * s)) - q3c; }, pcf, pcf + 500.0);
			want_p = pp;
			return q3c + h03(pp, 0.9 / ((1 - s) * (1 - s))) - (qf - glf * pp);
		}, 0.0, 10.4);
		if (!(std::abs(a.pcf - want_cf) <= 0.05) || !(std::abs(a.pp2 - want_p) <= 0.1))
			wrong << QStringLiteral("Prioritätsventil: CF %1 bar statt %2, P %3 statt %4").arg(a.pcf).arg(want_cf).arg(a.pp2).arg(want_p);
		else seen << QStringLiteral("Prioritätsventil: CF %1 bar (soll %2), %3 l/min bevorzugt, P %4 bar (soll %5)")
						 .arg(a.pcf, 0, 'f', 2).arg(want_cf, 0, 'f', 2).arg(h03(a.pcf, 1.0), 0, 'f', 3).arg(a.pp2, 0, 'f', 2).arg(want_p, 0, 'f', 2);
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 19. steering and check valves (R13, FORMELN.md H10, H11, V16, M14), plan lenkung.qet: six circuits, each fed by
		//     the fixed pump P2 of an LS power unit (q = 910 1/min · 4.5055 cm³ − GL·p, relief 50 bar over 35 bar):
		//     -QZ1 steering unit on the load unit -MM1 (variants M_user 10, 5, 0, −10 N.m), -KH1 joystick left +50 %,
		//     right −100 % (pMax 15 bar) on sealed X ports, -RM1 unlocked (A2 and pilot B1 fed, A1 open), -RM2 locked
		//     (A2 fed, B1 sealed), -RM3 free flow A1 -> A2 (spring 2 bar), -MM2 load unit 1 with A1.2 sealed: its oil leaves
		//     over the cross relief (p0_prv 30 bar, dpEnd_prv 35.3 bar, rT_prv 0.09 + rT_cv 0.07) into the open A2.1;
		//     -CM1 accumulator with safety block (V0 0.05 l, precharge 10 bar, relief 30 bar over 38 bar, rT 0.17), shut
		//     (x = 1) in the neutral variant
		Check c{names.at(18)};
		QStringList wrong, seen;
		const double qf = 910 * 4.5055e-3, glf = 0.003333;   // l/min, l/(min.bar)
		const double ak = M_PI * 0.016 * 0.016 / 4, ar = ak - M_PI * 0.01 * 0.01 / 4;
		auto bisect = [](auto fn, double lo, double hi) {
			for (int i = 0; i < 200; ++i) { const double mid = 0.5 * (lo + hi); (fn(lo) * fn(mid) <= 0 ? hi : lo) = mid; }
			return 0.5 * (lo + hi);
		};
		// orifice as the model computes it (FORMELN.md H03, smoothed below p_s = 0.3 bar): q = Δp/(√rT·(Δp² + p_s²)^¼)
		auto h03 = [](double d, double rt) { return d / (std::sqrt(rt) * std::pow(d * d + 0.09, 0.25)); };
		auto orifice = [&](double p0, double rt) {   // p − p0 over the orifice passes q = qf − glf·p
			return bisect([&](double pb) { return h03(pb - p0, rt) - (qf - glf * pb); }, p0, p0 + 50.0);
		};
		const double relief = bisect([&](double pb) { return qf - glf * pb - (pb - 50) / 35 * std::sqrt(pb / 0.08); }, 50.0, 85.0);
		struct Sample {
			double v = std::nan(""), pp = std::nan(""), x3 = std::nan(""), phi = std::nan(""), x = std::nan("");
			double x11 = std::nan(""), x12 = std::nan(""), x21 = std::nan(""), x22 = std::nan("");
			double p3 = std::nan(""), p4 = std::nan(""), p5 = std::nan(""), v6 = std::nan(""), p6 = std::nan(""), d6 = std::nan("");
			double p7a = std::nan(""), p7 = std::nan("");
		};
		auto sig = [](SimulationModel &m, const char *id) { return m.signalValue(QString::fromUtf8(id)); };
		auto run = [&](const QString &file, double t_v) {
			Sample s;
			ProjectRun pr = runProject(file, 0.0, {}, 2.9001, [&](SimulationModel &m) {
				if (std::abs(m.time() - t_v) < 5e-6) {
					s.v = sig(m, "v:-MM1");
					s.pp = sig(m, "p:-GZ1.P2") / 1e5;
					s.x3 = sig(m, "x:-MM1");
					s.phi = sig(m, "phi:-QZ1");
					s.v6 = sig(m, "v:-MM2");
					s.p6 = sig(m, "p:-GZ6.P2") / 1e5;
					s.d6 = sig(m, "p:-MM2.A1.2") / 1e5;
				}
				if (std::abs(m.time() - 0.2) < 5e-6) s.p7a = sig(m, "p:-GZ7.P2") / 1e5;
				if (std::abs(m.time() - 2.9) < 5e-6) {
					s.x = sig(m, "x:-MM1");
					s.x11 = sig(m, "p:-KH1.X1.1") / 1e5;
					s.x12 = sig(m, "p:-KH1.X1.2") / 1e5;
					s.x21 = sig(m, "p:-KH1.X2.1") / 1e5;
					s.x22 = sig(m, "p:-KH1.X2.2") / 1e5;
					s.p3 = sig(m, "p:-GZ3.P2") / 1e5;
					s.p4 = sig(m, "p:-GZ4.P2") / 1e5;
					s.p5 = sig(m, "p:-GZ5.P2") / 1e5;
					s.p7 = sig(m, "p:-GZ7.P2") / 1e5;
				}
			});
			if (!pr.error.isEmpty()) wrong << QStringLiteral("%1: %2").arg(QFileInfo(file).fileName(), pr.error);
			return s;
		};
		auto variant = [&](const QList<QPair<QString, QString>> &subst, const char *name, double t_v) {
			const QString f = textVariant(path("lenkung.qet"), subst, QString::fromUtf8(name));
			const Sample s = f.isEmpty() ? Sample{} : run(f, t_v);
			if (!f.isEmpty()) QFile::remove(f);
			return s;
		};
		const QString mu = QStringLiteral("<parameter name=\"M_user\" unit=\"N.m\" value=\"%1\"/>");
		const QString x0 = QStringLiteral("<parameter name=\"xStart\" unit=\"mm\" value=\"%1\"/>");
		const Sample a = run(path("lenkung.qet"), 0.3);
		const Sample h = variant({{mu.arg(10), mu.arg(5)}}, "qet_sim_r13_halb.qet", 0.3);
		const Sample n = variant({{mu.arg(10), mu.arg(0)}, {QStringLiteral("<parameter name=\"x\" value=\"2\"/>"), QStringLiteral("<parameter name=\"x\" value=\"1\"/>")}},
								 "qet_sim_r13_neutral.qet", 0.3);
		const Sample r = variant({{mu.arg(10), mu.arg(-10)}, {x0.arg(0), x0.arg(200)}}, "qet_sim_r13_rechts.qet", 0.15);
		auto near = [](double got, double want, double tol) { return std::isfinite(got) && std::abs(got - want) <= tol; };
		// steering unit fully deflected: all the pump flow to L, the wheel turns by the metered volume
		const double v_full = (qf - glf * a.pp) / 60000.0 / ak, phi_want = 2 * M_PI * ak * a.x3 / 32e-6;
		if (!near(a.v, v_full, 0.01 * v_full) || !near(a.x, 0.2, 0.5e-3) || !near(a.phi, phi_want, 0.01 * phi_want))
			wrong << QStringLiteral("Lenkventil voll: v %1 m/s statt %2, Lenkrad %3 statt %4 rad, x %5 mm").arg(a.v).arg(v_full).arg(a.phi).arg(phi_want).arg(a.x * 1000);
		else seen << QStringLiteral("Lenkventil links voll: %1 m/s (soll q/A_k %2), Lenkrad %3 U (soll V/D %4)")
						 .arg(a.v, 0, 'f', 4).arg(v_full, 0, 'f', 4).arg(a.phi / (2 * M_PI), 0, 'f', 3).arg(phi_want / (2 * M_PI), 0, 'f', 3);
		// half: the neutral path P -> E (rT/0.5²) and the metering path share the pump flow
		const double q_e = h03(std::max(h.pp, 0.0), 0.1 / 0.25), q_l = h.v * ak * 60000.0;
		const double balance = qf - glf * h.pp - q_e - q_l;
		if (!std::isfinite(balance) || std::abs(balance) > 0.01 * qf || !(q_l > 0.1 && q_e > 0.1))
			wrong << QStringLiteral("Lenkventil halb: Bilanz %1 l/min (E %2, L %3, p %4 bar)").arg(balance).arg(q_e).arg(q_l).arg(h.pp);
		else seen << QStringLiteral("halb: E %1 + L %2 l/min = Pumpe %3").arg(q_e, 0, 'f', 3).arg(q_l, 0, 'f', 3).arg(qf - glf * h.pp, 0, 'f', 3);
		const double p_neutral = orifice(0.0, 0.1);
		if (!near(n.pp, p_neutral, 0.01) || !near(n.x, 0.0, 0.5e-3))
			wrong << QStringLiteral("Lenkventil neutral: p %1 bar statt %2, x %3 mm").arg(n.pp).arg(p_neutral).arg(n.x * 1000);
		else seen << QStringLiteral("neutral P → E: %1 bar (soll Blende H03 %2), Zylinder steht").arg(n.pp, 0, 'f', 3).arg(p_neutral, 0, 'f', 3);
		const double v_right = -(qf - glf * r.pp) / 60000.0 / ar;
		if (!near(r.v, v_right, 0.01 * std::abs(v_right)) || !near(r.x, 0.0, 0.5e-3))
			wrong << QStringLiteral("Lenkventil rechts: v %1 m/s statt %2, x %3 mm").arg(r.v).arg(v_right).arg(r.x * 1000);
		else seen << QStringLiteral("rechts voll: %1 m/s (soll −q/A_r %2)").arg(r.v, 0, 'f', 4).arg(v_right, 0, 'f', 4);
		// joystick: pressure reducing valves on dead ends
		if (!near(a.x11, 7.5, 0.05) || !near(a.x12, 0.0, 0.05) || !near(a.x21, 0.0, 0.05) || !near(a.x22, 15.0, 0.05))
			wrong << QStringLiteral("Vorsteuergerät: X1.1 %1, X1.2 %2, X2.1 %3, X2.2 %4 bar statt 7,5/0/0/15").arg(a.x11).arg(a.x12).arg(a.x21).arg(a.x22);
		else seen << QStringLiteral("Vorsteuergerät X1.1 %1 bar (soll 7,5), X2.2 %2 bar (soll 15), übrige %3/%4 bar")
						 .arg(a.x11, 0, 'f', 3).arg(a.x22, 0, 'f', 3).arg(a.x12, 0, 'f', 3).arg(a.x21, 0, 'f', 3);
		// pilot operated double check valve
		const double p_unlock = orifice(0.0, 0.093), p_free = orifice(2.0, 0.093);
		if (!near(a.p3, p_unlock, 0.01)) wrong << QStringLiteral("Sperrblock entsperrt: %1 bar statt %2").arg(a.p3).arg(p_unlock);
		else seen << QStringLiteral("Sperrblock entsperrt A2 → A1: %1 bar (soll Blende H03 %2)").arg(a.p3, 0, 'f', 3).arg(p_unlock, 0, 'f', 3);
		if (!near(a.p4, relief, 0.05)) wrong << QStringLiteral("Sperrblock gesperrt: %1 bar statt Druckbegrenzung %2").arg(a.p4).arg(relief);
		else seen << QStringLiteral("gesperrt: %1 bar (Druckbegrenzung %2)").arg(a.p4, 0, 'f', 2).arg(relief, 0, 'f', 2);
		if (!near(a.p5, p_free, 0.01)) wrong << QStringLiteral("Sperrblock Durchfluss: %1 bar statt %2").arg(a.p5).arg(p_free);
		else seen << QStringLiteral("Durchfluss A1 → A2: %1 bar (soll p0 + Blende H03 %2)").arg(a.p5, 0, 'f', 3).arg(p_free, 0, 'f', 3);
		// load unit 1: the rod chamber oil A_r·v crosses the relief valve and the check valve in series
		const double v6 = (qf - glf * a.p6) / 60000.0 / ak, qx = ar * a.v6 * 60000.0;
		const double d6 = bisect([&](double d) {
			const double s = std::clamp((d - 30.0) / 35.3, 1e-9, 1.0);
			return std::sqrt(d / (0.09 / (s * s) + 0.07)) - qx;
		}, 30.0, 65.3);
		if (!near(a.v6, v6, 0.01 * v6) || !near(a.d6, d6, 0.1))
			wrong << QStringLiteral("Belastungseinheit 1: v %1 m/s statt %2, Stangenraum %3 bar statt %4").arg(a.v6).arg(v6).arg(a.d6).arg(d6);
		else seen << QStringLiteral("Belastungseinheit 1: %1 m/s (soll q/A_k %2), Stangenraum %3 bar (soll %4)")
						 .arg(a.v6, 0, 'f', 4).arg(v6, 0, 'f', 4).arg(a.d6, 0, 'f', 2).arg(d6, 0, 'f', 2);
		// accumulator with safety block: charging dV/dt = q(p), p = p_pre·(V0/(V0 − V))^1.4; then the relief valve
		const double pre = 11.0, v0 = 0.05;   // bar absolute, l
		auto gas = [&](double v) { return pre * std::pow(v0 / (v0 - v), 1.4) - 1.0; };
		double vol = 0.0;
		for (int i = 0; i < 20000; ++i) vol += (qf - glf * gas(vol)) / 60.0 * (0.2 / 20000);
		const double p7a = gas(vol);
		const double p7 = bisect([&](double pb) { return qf - glf * pb - std::clamp((pb - 30.0) / 38.0, 0.0, 1.0) * std::sqrt(pb / 0.17); }, 30.0, 68.0);
		if (!near(a.p7a, p7a, 0.01 * p7a) || !near(a.p7, p7, 0.05) || !near(n.p7, relief, 0.05))
			wrong << QStringLiteral("Speicher mit Sicherheitsblock: Laden %1 bar statt %2, Begrenzung %3 statt %4, abgesperrt %5 statt %6")
						 .arg(a.p7a).arg(p7a).arg(a.p7).arg(p7).arg(n.p7).arg(relief);
		else seen << QStringLiteral("Speicher: nach 0,2 s %1 bar (soll Gasgesetz %2), Sicherheitsventil %3 bar (soll %4), abgesperrt %5 bar")
						 .arg(a.p7a, 0, 'f', 2).arg(p7a, 0, 'f', 2).arg(a.p7, 0, 'f', 2).arg(p7, 0, 'f', 2).arg(n.p7, 0, 'f', 2);
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 18. mobile hydraulics (R13, FORMELN.md H08, V15, M13), plan mobilhydraulik.qet (FluidSIM defaults of the
		//     power unit: n 910 1/min, VP 5.6044 cm³, FP 4.5055 cm³, p0 11.7 bar, dpEnd 0.8 bar, relief 50 bar / 35 bar):
		//     -GZ1 P1 closed: standby p = (p0 + dpEnd)/(1 + dpEnd·GL/q); -GZ2 P2 closed: q_FP − GL·p = (p − 50)/35·√(p/0.08);
		//     -GZ2 P1 → -RZ1 (rT 1) → load node (LS) → -RZ2 (rT 1, variant 2) → tank: Δp over -RZ1 = p0 + dpEnd·(1 − α),
		//     the flow does not depend on the load; -GZ1 P2 → -QM1 (+100 %, variant −100 %) → load unit -MM1 at q/A
		Check c{names.at(17)};
		QStringList wrong, seen;
		const double qv = 910 * 5.6044e-3, qf = 910 * 4.5055e-3, glv = 0.028333, glf = 0.003333;   // l/min, l/(min.bar)
		const double m0 = 11.7, band = 0.8;
		auto bisect = [](auto fn, double lo, double hi) {
			for (int i = 0; i < 200; ++i) { const double mid = 0.5 * (lo + hi); (fn(lo) * fn(mid) <= 0 ? hi : lo) = mid; }
			return 0.5 * (lo + hi);
		};
		const double standby = (m0 + band) / (1 + band * glv / qv);
		const double relief = bisect([&](double pb) { return qf - glf * pb - (pb - 50) / 35 * std::sqrt(pb / 0.08); }, 50.0, 85.0);
		auto loadPressure = [](double rt2, double q) {
			double px = rt2 * q * q;
			for (int i = 0; i < 50; ++i) px = rt2 * std::pow(q - std::sqrt(px / 2400), 2);
			return px;
		};
		auto lsWant = [&](double rt2, double *dp_out, double *px_out) {
			const double alpha = bisect([&](double a) {
				const double dp = m0 + band * (1 - a), q = std::sqrt(dp);
				return a * qv - glv * (loadPressure(rt2, q) + dp) - q;
			}, 0.0, 1.0);
			*dp_out = m0 + band * (1 - alpha);
			*px_out = loadPressure(rt2, std::sqrt(*dp_out));
		};
		struct Sample { double v = std::nan(""), pp2 = std::nan(""), x = std::nan(""), p1a = std::nan(""), p2b = std::nan(""), p1b = std::nan(""), pls = std::nan(""); };
		auto run = [&](const QString &file, double t_v) {
			Sample s;
			ProjectRun pr = runProject(file, 0.0, {}, 2.9001, [&](SimulationModel &m) {
				if (std::abs(m.time() - t_v) < 5e-6) { s.v = m.signalValue(QStringLiteral("v:-MM1")); s.pp2 = m.signalValue(QStringLiteral("p:-GZ1.P2")) / 1e5; }
				if (std::abs(m.time() - 2.9) < 5e-6) {
					s.x = m.signalValue(QStringLiteral("x:-MM1"));
					s.p1a = m.signalValue(QStringLiteral("p:-GZ1.P1")) / 1e5;
					s.p2b = m.signalValue(QStringLiteral("p:-GZ2.P2")) / 1e5;
					s.p1b = m.signalValue(QStringLiteral("p:-GZ2.P1")) / 1e5;
					s.pls = m.signalValue(QStringLiteral("p:-GZ2.LS")) / 1e5;
				}
			});
			if (!pr.error.isEmpty()) wrong << QStringLiteral("%1: %2").arg(QFileInfo(file).fileName(), pr.error);
			return s;
		};
		const Sample a = run(path("mobilhydraulik.qet"), 0.3);
		const QString load2 = textVariant(path("mobilhydraulik.qet"), {{QStringLiteral("<parameter name=\"rT\" unit=\"bar.min2/l2\" value=\"1.0\"/>"),
																	   QStringLiteral("<parameter name=\"rT\" unit=\"bar.min2/l2\" value=\"2.0\"/>")}},
										  QStringLiteral("qet_sim_r13_last.qet"));
		const Sample b = load2.isEmpty() ? Sample{} : run(load2, 0.3);
		if (!load2.isEmpty()) QFile::remove(load2);
		const QString back = textVariant(path("mobilhydraulik.qet"), {{QStringLiteral("<parameter name=\"xStep\" unit=\"%\" value=\"100\"/>"),
																	  QStringLiteral("<parameter name=\"xStep\" unit=\"%\" value=\"-100\"/>")},
																	 {QStringLiteral("<parameter name=\"xStart\" unit=\"mm\" value=\"0\"/>"),
																	  QStringLiteral("<parameter name=\"xStart\" unit=\"mm\" value=\"200\"/>")}},
										 QStringLiteral("qet_sim_r13_zurueck.qet"));
		const Sample r = back.isEmpty() ? Sample{} : run(back, 0.15);
		if (!back.isEmpty()) QFile::remove(back);
		auto near = [](double got, double want, double tol) { return std::isfinite(got) && std::abs(got - want) <= tol; };
		if (!near(a.p1a, standby, 0.05)) wrong << QStringLiteral("Bereitschaftsdruck %1 bar statt %2").arg(a.p1a).arg(standby);
		else seen << QStringLiteral("Bereitschaft %1 bar (soll %2)").arg(a.p1a, 0, 'f', 3).arg(standby, 0, 'f', 3);
		if (!near(a.p2b, relief, 0.05)) wrong << QStringLiteral("Druckbegrenzung %1 bar statt %2").arg(a.p2b).arg(relief);
		else seen << QStringLiteral("Druckbegrenzung %1 bar (soll %2)").arg(a.p2b, 0, 'f', 2).arg(relief, 0, 'f', 2);
		const Sample *ls[2] = {&a, &b};
		for (int i = 0; i < 2; ++i) {
			const double rt2 = i == 0 ? 1.0 : 2.0;
			double dp = 0, px = 0;
			lsWant(rt2, &dp, &px);
			const double got = ls[i]->p1b - ls[i]->pls;
			if (!near(got, dp, 0.05) || !near(ls[i]->pls, px, 0.1))
				wrong << QStringLiteral("LS-Regelung rT %1: Δp %2 / Last %3 bar statt %4 / %5").arg(rt2).arg(got).arg(ls[i]->pls).arg(dp).arg(px);
			else seen << QStringLiteral("LS Last %1 bar: Δp %2 bar (soll %3), %4 l/min").arg(ls[i]->pls, 0, 'f', 2).arg(got, 0, 'f', 3).arg(dp, 0, 'f', 3).arg(std::sqrt(got), 0, 'f', 3);
		}
		const double ak = M_PI * 0.016 * 0.016 / 4, ar = ak - M_PI * 0.01 * 0.01 / 4;
		const double v_out = (qf - glf * a.pp2) / 60000.0 / ak, v_in = -(qf - glf * r.pp2) / 60000.0 / ar;
		if (!near(a.v, v_out, 0.01 * std::abs(v_out)) || !near(a.x, 0.2, 0.5e-3))
			wrong << QStringLiteral("Ausfahren: v %1 m/s statt %2, x %3 mm").arg(a.v).arg(v_out).arg(a.x * 1000);
		else seen << QStringLiteral("Ausfahren +100 %: %1 m/s (soll q/A_k %2), Endlage 200 mm").arg(a.v, 0, 'f', 4).arg(v_out, 0, 'f', 4);
		if (!near(r.v, v_in, 0.01 * std::abs(v_in)) || !near(r.x, 0.0, 0.5e-3))
			wrong << QStringLiteral("Einfahren: v %1 m/s statt %2, x %3 mm").arg(r.v).arg(v_in).arg(r.x * 1000);
		else seen << QStringLiteral("−100 %: %1 m/s (soll −q/A_r %2), Endlage 0").arg(r.v, 0, 'f', 4).arg(v_in, 0, 'f', 4);
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 17. multi-position cylinder (R12, FORMELN.md M12), plan mehrstellungszylinder.qet, 6 bar, 3 s, xMax 200, xMax2 100 mm:
		//     -MM1 C1 → both at 100; -MM2 C2 → front 200, rear 0; -MM3 C1+C3 from the front → held at 100 (C1 force
		//     p·A_k beats C3 p·A_r); -MM4 C3 from 200/100 → both back to 0
		Check c{names.at(16)};
		QStringList wrong, seen;
		QHash<QString, QPair<double, double>> pos;
		const QStringList cyl{QStringLiteral("-MM1"), QStringLiteral("-MM2"), QStringLiteral("-MM3"), QStringLiteral("-MM4")};
		ProjectRun pr = runProject(path("mehrstellungszylinder.qet"), 6.0, {}, 3.0001, [&](SimulationModel &s) {
			if (std::abs(s.time() - 3.0) < 5e-6)
				for (const QString &z : cyl)
					pos.insert(z, {s.signalValue(QStringLiteral("x:") + z), s.signalValue(QStringLiteral("x1:") + z)});
		});
		if (!pr.error.isEmpty()) wrong << pr.error;
		const QHash<QString, QPair<double, double>> want{{cyl.at(0), {0.1, 0.1}}, {cyl.at(1), {0.2, 0.0}},
														 {cyl.at(2), {0.1, 0.1}}, {cyl.at(3), {0.0, 0.0}}};
		for (const QString &z : cyl) {
			const auto g = pos.value(z, {std::nan(""), std::nan("")}), w = want.value(z);
			const QString s = QStringLiteral("%1 %2/%3 mm").arg(z).arg(g.first * 1000, 0, 'f', 1).arg(g.second * 1000, 0, 'f', 1);
			if (!(std::abs(g.first - w.first) <= 0.5e-3 && std::abs(g.second - w.second) <= 0.5e-3))
				wrong << s + QStringLiteral(" statt %1/%2").arg(w.first * 1000).arg(w.second * 1000);
			else seen << s;
		}
		c.passed = wrong.isEmpty();
		c.detail = wrong.isEmpty() ? QStringLiteral("vorn/hinten: ") + seen.join(QStringLiteral("; ")) : wrong.join(QStringLiteral("; "));
		out << c;
	}
	{   // 16. compressed air motors (R12, FORMELN.md M11), plan druckluftmotor.qet: -MA1 fed at A, -MA2 at B.
		//     Linear characteristic from p_N 3 bar, n_N 975 1/min, M_N 0.62 N·m, n0/nN 2: ω₀ = 2·n_N·2π at p_N
		Check c{names.at(15)};
		QStringList wrong, seen;
		auto speeds = [&](const QString &file, double bar) {
			QPair<double, double> w{std::nan(""), std::nan("")};
			ProjectRun pr = runProject(file, bar, {}, 3.0001, [&](SimulationModel &s) {
				if (std::abs(s.time() - 3.0) < 5e-6) w = {s.signalValue(QStringLiteral("omega:-MA1")), s.signalValue(QStringLiteral("omega:-MA2"))};
			});
			if (!pr.error.isEmpty()) wrong << QStringLiteral("%1: %2").arg(QFileInfo(file).fileName(), pr.error);
			return w;
		};
		const double w0 = 2 * 975.0 / 60.0 * 2 * M_PI;
		const auto free6 = speeds(path("druckluftmotor.qet"), 6.0);
		const QString braked = textVariant(path("druckluftmotor.qet"), {{QStringLiteral("<parameter name=\"M_ext\" unit=\"N.m\" value=\"0\"/>"),
																	   QStringLiteral("<parameter name=\"M_ext\" unit=\"N.m\" value=\"0.62\"/>")}},
										   QStringLiteral("qet_sim_r12_nenn.qet"));
		const auto rated = braked.isEmpty() ? qMakePair(std::nan(""), std::nan("")) : speeds(braked, 3.0);
		if (!braked.isEmpty()) QFile::remove(braked);
		auto ok = [](double got, double want) { return std::isfinite(got) && std::abs(got - want) <= 0.005 * std::abs(want); };
		if (!(ok(free6.first, 2 * w0) && ok(free6.second, -2 * w0)))
			wrong << QStringLiteral("Leerlauf 6 bar: %1 / %2 rad/s statt ±%3").arg(free6.first).arg(free6.second).arg(2 * w0);
		else seen << QStringLiteral("Leerlauf 6 bar: %1 / %2 rad/s (soll ±%3, doppelt so schnell wie bei 3 bar)").arg(free6.first, 0, 'f', 1).arg(free6.second, 0, 'f', 1).arg(2 * w0, 0, 'f', 1);
		if (!(ok(rated.first, w0 / 2) && ok(rated.second, -w0 / 2)))
			wrong << QStringLiteral("Nennpunkt 3 bar, 0,62 N·m: %1 / %2 rad/s statt ±%3").arg(rated.first).arg(rated.second).arg(w0 / 2);
		else seen << QStringLiteral("Nennpunkt 3 bar, 0,62 N·m bremsend: %1 1/min (soll 975), links %2 1/min").arg(rated.first * 60 / (2 * M_PI), 0, 'f', 1).arg(rated.second * 60 / (2 * M_PI), 0, 'f', 1);
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
