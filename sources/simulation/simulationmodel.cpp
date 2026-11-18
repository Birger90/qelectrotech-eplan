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
#include <algorithm>
#include "simulationmodel.h"
#include "simulationphysics.h"
#include "simulationsignals.h"
#include "simulationunits.h"
#include "grafcet.h"
#include "simulationcatalog.h"

#include "plcprogram.h"

#include "../diagram.h"
#include "../fluid/fluiddata.h"
#include "../fluid/fluidtypes.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/terminal.h"

#include <QMap>
#include <QRegularExpression>
#include <cmath>
#include <functional>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

constexpr double P_ATM = 1.013e5;       // Pa
constexpr double T_AIR = 293.15;        // K
constexpr double R_AIR = 287.0;         // J/(kg K)
// normal density of the normal litres in qNN, qn_ac, qns_max, qLeak: the "technical normal state" of the
// FluidSIM flow model (20 °C, 100000 Pa, rho0 = 1.1845 kg/m³), where all these values come from (FORMELN.md P02, QS05)
constexpr double RHO_N = 1.1845;        // kg/m³
constexpr double B_CRIT = 0.4;          // critical pressure ratio, as FluidSIM assumes for all components (QS05)
constexpr double V_LINE = 5e-6;         // m³ per pneumatic net
constexpr double V_DEAD = 2e-6;         // m³ per cylinder chamber
constexpr double P_PILOT = 1.5e5;       // Pa gauge that switches a pilot
constexpr double E_OIL = 7e8;           // Pa effective bulk modulus
constexpr double V_HYD = 1e-4;          // m³ per hydraulic net
constexpr double DP_LAMINAR = 0.3e5;    // Pa
constexpr double P_CAVITATION = P_ATM - 0.9e5;   // Pa abs: lowest oil pressure the model allows
constexpr double T_PULL_IN = 0.010;     // s relay
constexpr double T_DROP_OUT = 0.008;    // s relay

// ISO 6358 ellipse, b = B_CRIT (tools/simulation/FORMELN.md P02)
double massFlow(double p1, double p2, double c)
{
	if (p1 < p2) return -massFlow(p2, p1, c);
	if (p1 <= 0) return 0.0;
	const double r = p2 / p1;
	const double f = r <= B_CRIT ? 1.0
		: std::sqrt(std::max(0.0, 1.0 - std::pow((r - B_CRIT) / (1.0 - B_CRIT), 2)));
	return c * p1 * f * RHO_N;
}

// conductance from the normal nominal flow at 6 -> 5 bar (FORMELN.md P03)
double conductanceFromQnn(double qnn_l_min)
{
	const double p1 = P_ATM + 6e5;
	const double r = (p1 - 1e5) / p1;
	const double f = std::sqrt(1.0 - std::pow((r - B_CRIT) / (1.0 - B_CRIT), 2));
	return (qnn_l_min / 1000.0 / 60.0) / (p1 * f);
}

	/**
		how a line, hose or coupling is computed (R05, FORMELN.md L04): Open = an uncoupled coupling
		(no connection), Pipe = geometry given (length, d_inner), Resistance = rT or qNN given,
		Ideal = nothing given (lossless, as FluidSIM treats plain connections)
	*/
enum class LineMode { Ideal, Resistance, Pipe, Open };

LineMode lineMode(const FluidData &f)
{
	if (f.m_function == QLatin1String("connection.quick_coupling") && f.parameter(QStringLiteral("coupled")).trimmed() == QLatin1String("0"))
		return LineMode::Open;
	if (!f.parameter(QStringLiteral("length")).isEmpty() && !f.parameter(QStringLiteral("d_inner")).isEmpty())
		return LineMode::Pipe;
	if (!f.parameter(QStringLiteral("rT")).isEmpty() || !f.parameter(QStringLiteral("qNN")).isEmpty())
		return LineMode::Resistance;
	return LineMode::Ideal;
}

	/// fluid ports of @a e in pairs by their height in the symbol (rotary joint: 1-4, 2-5, 3-6); empty when
	/// the ports do not form pairs (distributor: all one net)
QList<QPair<Terminal *, Terminal *>> channelPairs(Element *e)
{
	QMap<int, QList<Terminal *>> rows;
	int n = 0;
	for (Terminal *t : e->terminals()) {
		if (t->medium() != Fluid::Medium::Pneumatic && t->medium() != Fluid::Medium::Hydraulic) continue;
		rows[qRound(e->mapFromScene(t->dockConductor()).y())] << t;   // height in the symbol, before rotation
		++n;
	}
	QList<QPair<Terminal *, Terminal *>> out;
	if (n <= 2) return out;
	for (const auto &row : std::as_const(rows)) {
		if (row.size() != 2) return {};
		out << qMakePair(row.first(), row.last());
	}
	return out;
}

	/// conductance that passes @a q_l_min (normal litres per minute) from @a p1 to @a p2 (absolute) (FORMELN.md P04)
double conductanceAt(double q_l_min, double p1, double p2)
{
	const double r = p2 / p1;
	const double f = r <= B_CRIT ? 1.0 : std::sqrt(std::max(1e-6, 1.0 - std::pow((r - B_CRIT) / (1.0 - B_CRIT), 2)));
	return (q_l_min / 1000.0 / 60.0) / (p1 * f);
}

// oil orifice from rT, dp = rT Q² (FORMELN.md H03)
double volumeFlow(double p1, double p2, double rt)
{
	const double dp = p1 - p2;
	if (rt <= 0) rt = 1e-4;
	// rT in bar/(l/min)²: dp = rT Q² for |dp| >> DP_LAMINAR, smooth and odd around dp = 0
	return SimPhysics::orificeFlow(dp, 1.0 / (60000.0 * std::sqrt(1e5 * rt)), DP_LAMINAR);
}

QString label(Element *e) { return e->elementInformations()[QStringLiteral("label")].toString(); }
	/// R06: parts whose flow A → B is measured (an orifice rT): analog flow sensor and the FluidSIM flowmeter with output
bool isFlowSensor(const QString &fn)
{
	return fn == QLatin1String("proportional.flow_sensor") || fn == QLatin1String("sensor.flowmeter_2");
}
QString info(Element *e, const char *key) { return e->elementInformations()[QLatin1String(key)].toString(); }

	/**
		characteristic value @a name in unit @a to, converted by the unit the
		symbol gives (<parameter unit="MPa"/>); a value without unit is taken
		as @a to. An unreadable number or an unknown or unfitting unit is an
		error of the model (@a errors), and @a fallback is used.
	*/
enum class Range { Any, NonNegative, Positive };

bool inRange(double v, Range r)
{
	return std::isfinite(v) && (r == Range::Any || (r == Range::NonNegative ? v >= 0.0 : v > 0.0));
}

QString rangeText(Range r)
{
	return r == Range::Positive ? QStringLiteral("muss größer als 0 sein")
		: (r == Range::NonNegative ? QStringLiteral("darf nicht negativ sein") : QStringLiteral("muss endlich sein"));
}

	/**
		characteristic value without conversion, in its own unit @a native
		(e.g. rT in bar.min2/l2, the exponent k without unit): a different
		unit, an unreadable number or a value outside @a range is an error.
	*/
double paramNative(const FluidData &f, const QString &name, double fallback, const QString &native,
				   QStringList *errors, const QString &who, Range range)
{
	for (const FluidData::Parameter &p : f.m_parameters) {
		if (p.name != name) continue;
		if (!p.unit.isEmpty() && p.unit != native) {
			*errors << QStringLiteral("%1: Kennwert %2 hat die Einheit „%3“, erwartet wird %4")
						   .arg(who, name, p.unit, native.isEmpty() ? QStringLiteral("keine Einheit") : native);
			return fallback;
		}
		bool ok = false;
		const double v = QString(p.value).replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok);
		if (!ok || !inRange(v, range)) {
			*errors << QStringLiteral("%1: Kennwert %2 = „%3“ %4").arg(who, name, p.value,
				ok ? rangeText(range) : QStringLiteral("ist keine Zahl"));
			return fallback;
		}
		return v;
	}
	return fallback;
}

double paramIn(const FluidData &f, const QString &name, double fallback, const QString &to,
			   QStringList *errors, const QString &who, Range range = Range::Any)
{
	for (const FluidData::Parameter &p : f.m_parameters) {
		if (p.name != name) continue;
		bool ok = false;
		const double v = QString(p.value).replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok);
		if (!ok) {
			*errors << QStringLiteral("%1: Kennwert %2 = „%3“ ist keine Zahl").arg(who, name, p.value);
			return fallback;
		}
		const double r = SimUnits::convert(v, p.unit.isEmpty() ? to : p.unit, to);
		if (std::isnan(SimUnits::convert(1.0, p.unit.isEmpty() ? to : p.unit, to))) {
			*errors << QStringLiteral("%1: Kennwert %2 hat die Einheit „%3“, erwartet wird %4")
						   .arg(who, name, p.unit, to);
			return fallback;
		}
		if (!inRange(r, range)) {
			*errors << QStringLiteral("%1: Kennwert %2 = %3 %4 %5").arg(who, name, p.value, p.unit, rangeText(range));
			return fallback;
		}
		return r;
	}
	return fallback;
}

Fluid::Medium medium(Element *e)
{
	for (Terminal *t : e->terminals())
		if (t->medium() == Fluid::Medium::Pneumatic || t->medium() == Fluid::Medium::Hydraulic)
			return t->medium();
	return Fluid::Medium::Electric;
}

double timeFromText(const QString &text, double fallback = 1.0)
{
	static const QRegularExpression re(QStringLiteral("(\\d+(?:[.,]\\d+)?)\\s*(ms|s)?\\b"));
	const auto m = re.match(text);
	if (!m.hasMatch()) return fallback;
	QString number = m.captured(1);
	number.replace(QLatin1Char(','), QLatin1Char('.'));
	const double v = number.toDouble();
	return m.captured(2) == QLatin1String("ms") ? v / 1000.0 : v;
}

} // namespace

namespace {
	/// the elements of a folio in a fixed order (by uuid): the scene does not give them in the same order on every
	/// load, and the model must be built the same way every time, so that a run is reproducible (roadmap R03)
QList<Element *> sortedElements(Diagram *d)
{
	QList<Element *> out = d->elements();
	std::sort(out.begin(), out.end(), [](const Element *a, const Element *b) { return a->uuid() < b->uuid(); });
	return out;
}
}

// ------------------------------------------------------------------------- components
struct SimulationModel::Valve {
	Element *e = nullptr;
	QString label;
	QList<QList<QPair<QString, QString>>> positions;
	int rest = 0, position = 0, target = 0;
	FluidData::Actuation side[2];
	bool actuated[2] = {false, false};
	bool forced[2] = {false, false};      // actuated by a GRAFCET action (-QM1, -QM1/14, -QM1/12)
	double switch_at = -1, switch_start = -1;   // the spool moves from position to target in between
	double c = 0, rt = 0.1, t_on = 0.02, t_off = 0.03;
	bool hydraulic = false;
	// selector switch: actuated by hand on one side only, without any spring (FluidSIM "mit Wahlschalter").
	// Each press switches between the rest position and the actuated one and the valve stays there.
	int toggle_side = -1;
	bool latched = false;
	int dominant = -1;          // R11: side that wins when both are actuated (FluidSIM/QET "dominant"), -1: none, the valve stays
	// roller lever / plunger: actuated by a cylinder inside a stroke range (FluidSIM distance rule)
	QString trigger_cylinder;
	double trigger_lo = 0, trigger_hi = -1;
	int trigger_side = 0;

	int last() const { return static_cast<int>(positions.size()) - 1; }
	int wanted() const {
		if (toggle_side >= 0 && !forced[0] && !forced[1]) {
			const int on = toggle_side == 0 ? 0 : last();
			const int off = rest != on ? rest : last() - on;
			return latched ? on : off;
		}
		const bool l = actuated[0] || forced[0], r = actuated[1] || forced[1];
		if (l && !r) return 0;
		if (r && !l) return static_cast<int>(positions.size()) - 1;
		if (l && r) return dominant == 0 ? 0 : dominant == 1 ? last() : position;
		const bool spring = side[0].spring || side[1].spring || side[0].air_spring || side[1].air_spring;
		return spring ? rest : position;
	}
};

struct SimulationModel::Cylinder {
	Element *e = nullptr;
	QString label;
	double a_k = 0, a_r = 0, stroke = 0.1, m = 0.1, f_static = 0, f_slide = 0, damping = 20;
	double stop_k = 0, stop_c = 0;   // elastic end stop (R02); 0: hard stop
	double f_spring = 0, f_load = 0, x = 0, v = 0;
	// R11 (FORMELN.md M08–M10): weight m·g·sin(alpha) against extending, spring characteristic, force profile
	double weight = 0;                          // N, against extending
	int spring_dir = 0;                         // characteristic: +1 pushes out (left), -1 pushes back (right), 0 constant f_spring
	double spring_f0 = 0, spring_f1 = 0;        // spring force at x = 0 and at the stroke [N]
	QVector<QPair<double, double>> load_ex, load_re;   // force profile (x [m], F [N]) extending / retracting, against extending
	static double profileAt(const QVector<QPair<double, double>> &pts, double x) {
		if (pts.isEmpty()) return 0.0;
		if (x <= pts.first().first) return pts.first().second;
		for (int i = 1; i < pts.size(); ++i)
			if (x <= pts.at(i).first) {
				const auto &a = pts.at(i - 1), &b = pts.at(i);
				return b.first > a.first ? a.second + (b.second - a.second) * (x - a.first) / (b.first - a.first) : b.second;
			}
		return pts.last().second;
	}
		/// all forces in the direction against extending except friction: load (constant or profile), spring, weight
	double opposing() const {
		const auto &pts = v >= 0 ? load_ex : load_re;
		const double load = pts.isEmpty() ? f_load : profileAt(pts, x);
		double spring = f_spring;
		if (spring_dir != 0) spring = -spring_dir * (spring_f0 + (spring_f1 - spring_f0) * std::clamp(x / stroke, 0.0, 1.0));
		return load + spring + weight;
	}
	// R10 semi-rotary drive: x, stroke in rad, v in rad/s, a_k = a_r displacement per radian [m³/rad]
	// (= torque per pascal), m the moment of inertia [kg·m²], forces are torques [N·m]
	bool rotary = false;
	/// position compared with a marker "-MM1 lo..hi": metres, for a rotary drive degrees / 1000
	double markPos() const { return rotary ? x * 180.0 / M_PI / 1000.0 : x; }
};

	/// line with geometry (R05): Darcy/Hagen-Poiseuille between two ports
	/// proportional 4/3 valve (R07): spool s follows the setpoint u/10 V as a first-order lag
struct SimulationModel::PropValve {
	Element *e = nullptr;
	QString label;
	double u = 0.0, s = 0.0;               // setpoint [V], spool position (-1 … 1)
	double tau = 0.01;                     // time constant of the spool [s]
	double lap_p = 0.0, lap_t = 0.0;       // relative overlap of the P and the T edges
	double rt = 1.0;                       // resistance of one fully open edge [bar.min2/l2]
	double gleak = 0.0;                    // leakage of each edge [m3/(s.Pa)]
	bool triangular = false;               // notch: triangular (area ∝ x²) or rectangular (∝ x)
};

struct SimulationModel::Pipe {
	Element *e = nullptr;
	QString a, b;                 // port names
	SimPhysics::PipeFlow flow;
	bool hydraulic = false;
};

	/// vacuum generator (ejector, R10): supply 1, vacuum port 2, exhaust to the air
struct SimulationModel::Ejector {
	Element *e = nullptr;
	QString label;
	double pu_min = -0.085e6;     // best vacuum (gauge, < 0) at the optimal supply pressure
	double dp_opt = 0.5e6;        // optimal supply pressure (gauge)
	double c_drive = 0;           // conductance of the driving nozzle (air consumption)
	double q_max = 0;             // suction mass flow against the atmosphere at dp_opt [kg/s]
};

	/// suction cup (R10): a small volume, sealed by a workpiece or open to the air
struct SimulationModel::SuctionCup {
	Element *e = nullptr;
	QString label;
	int node = -1;
	double area = 0, weight = 0, safety = 1.0;    // effective area [m²], weight force m·g [N]
	double c_leak = 0, c_open = 0;
	bool object = true;           // a workpiece lies on the cup
	bool held = false;
	double force = 0;             // holding force (p_atm - p)·A [N]
};

struct SimulationModel::Pump {
	Element *e = nullptr;
	double q = 0, p_max = 0, band = 1e5;          // relief characteristic: full flow up to p_max - band
	double alpha = 1.0, cleak = 0.0;              // setting of a variable pump, leakage P -> T (R08)
	double last_q = 0.0, last_dp = 0.0;           // delivered flow and pressure rise of the last step
};

struct SimulationModel::Motor {
	Element *e = nullptr;
	QString label;
	SimPhysics::Shaft shaft;
	double tau_load = 0.0;                        // constant load torque against positive rotation
	double last_dp = 0.0;
	bool air = false;                             // R12: compressed air motor (mass flow from the inlet)
	QString exhaust;                              // mid exhaust port (EX), empty: the air leaves at the other port
};

struct SimulationModel::FlowValve {
	Element *e = nullptr;
	QString function;
	QStringList ports;
	QString free_from, free_to;
	bool hydraulic = false;
	double c = 0, c_free = 0, rt = 0.1, rt_free = 0.05, p0 = 0, dp_end = 0.5e5;
	double t_delay = 1.0, since = -1.0;         // time delay valve
	double ratio = 3.0, gleak = 0.0;            // counterbalance valve: pilot area ratio, leakage P -> T (R09)
	double u = 0.0, p_lo = 0.0, p_hi = 0.0;     // proportional relief valve: setpoint [V], opening at 0 and 10 V (R06)
	double last_q = 0.0;                        // flow sensor: flow A -> B of the last step [m3/s] (R06)
	double p0_b = 0.0, dp_end_b = 0.5e5, rt_b = 0.1;   // shock valve: relief of side B (R09)

	double flow(double pa, double pb, double share = 1.0, bool free = false) const {
		if (share <= 0) return 0.0;
		if (hydraulic) return volumeFlow(pa, pb, (free ? rt_free : rt) / std::pow(std::max(share, 1e-3), 2));
		return massFlow(pa, pb, (free ? c_free : c) * share);
	}
};

struct SimulationModel::Relay {
	Element *e = nullptr;
	QString label;
	int n1 = -1, n2 = -1;
	bool on = false, target = false;
	double switch_at = -1, on_since = -1, off_since = -1, delay = 1.0;
	QString mode;          // "on", "off" or empty
};

struct SimulationModel::Contact {
	Element *e = nullptr;
	QString kind;          // button, limit, relay
	bool nc = false;
	int n1 = -1, n2 = -1;
	QString trigger;       // limit switch: "-MM1 vorne", or a stroke range "-MM1 0..2" (mm)
	Relay *relay = nullptr;
	QString mode;
	bool latching = false; // selector switch: each press switches it over and it stays
};

struct SimulationModel::Coil {
	Element *e = nullptr;
	QString label;
	int n1 = -1, n2 = -1;
	bool on = false, known = false;
	Valve *valve = nullptr;   // the valve it is linked to (master), if not found by its label
	int side = -1;            // 0 left, 1 right, -1 the valve's only solenoid
};

	/// the inputs and outputs of a GRAFCET controller (FluidSIM "GRAFCET-SPS"): each input is true
	/// while its terminal carries plus, each output terminal carries plus while its variable is true
struct SimulationModel::GrafcetIo {
	Element *e = nullptr;
	QList<QPair<int, QString>> inputs, outputs;   // net, variable
	QList<bool> value;                            // per output
};

struct SimulationModel::Counter {
	Element *e = nullptr;
	QString label;
	int n1 = -1, n2 = -1;
	QList<int> nodes;             // every terminal: a counter symbol may have several
	int preset = 1, count = 0;
	bool last = false;
	bool reset_coil = false;      // label ends with "/R": resets the counter of that label
};

struct SimulationModel::Accumulator {
	Element *e = nullptr;
	QString label;
	int node = -1;
	double v0 = 1e-3, p_pre = 0.0, n = 1.4;    // gas volume at precharge, precharge absolute, exponent
};

struct SimulationModel::PressureSwitch {
	Element *e = nullptr;
	QString label;
	int node = -1;
	double p_on = 4e5, p_off = 3.5e5;          // gauge
	bool on = false;
	bool vacuum = false;                       // vacuum switch (R10): on below p_on, off above p_off
};

struct SimulationModel::PlcChannel {
	Element *e = nullptr;
	bool output = false;
	QStringList names;            // variable names: address and symbol
	QList<int> signal, plus, minus;
	bool value = false;
};

struct SimulationModel::Plc {
	Element *e = nullptr;
	QString label;
	std::shared_ptr<PlcProgram> program;
};

	/// analog signal block (R06, FORMELN.md A01–A10): a FluidSIM electric part with real valued ports
struct SimulationModel::Analog {
	Element *e = nullptr;
	QString label, block;             // block: FluidSIM class without "ET.", e.g. "Controller.ControllerPID"
	QHash<QString, int> port;         // port name (upper case) -> net
	QHash<QString, QPair<double, QString>> par;   // FluidSIM characteristic values with their unit
	QList<QPair<QString, QString>> links;          // own port -> "-QM1/Y1", "-MM1", "-BP1"
	QStringList inputs, outputs;      // ports it reads / drives
	double out = 0.0;                 // output voltage
	double vmin = 0.0;                // least supply voltage
	bool on = false;                  // comparator
	SimSignals::Waveform wave;
	SimSignals::SetpointCard card;
	SimSignals::Pid pid;
	SimSignals::StateController sc;
	SimSignals::Pt2 pt2;
	SimSignals::Integrator integ;
	double efficiency = 0.9, jump = 0.0;
	bool two_channel = false;
	// linked parts (linkAnalog)
	QList<QPair<PropValve *, double>> valves;      // valve and sign: connector (Y2: -1) times channel (B: -1)
	QList<FlowValve *> reliefs;
	Cylinder *cylinder = nullptr;
	int sensor_node = -1;
	FlowValve *flow_sensor = nullptr;
	double s0 = 0, s1 = 1, u0 = 0, u1 = 10;        // sensor characteristic (SI)
	bool signed_flow = false;

	double value(const QString &name, double fallback, const QString &to = QString()) const {
		const auto it = par.constFind(name);
		if (it == par.constEnd() || !std::isfinite(it->first)) return fallback;
		if (to.isEmpty() || it->second.isEmpty()) return it->first;
		const double v = SimUnits::convert(it->first, it->second, to);
		return std::isfinite(v) ? v : fallback;
	}
	bool has(const QString &p) const { return port.value(p, -1) >= 0; }
};

// ------------------------------------------------------------------------- construction
SimulationModel::SimulationModel(const QList<Diagram *> &diagrams, double supply_bar)
{
	buildNets(diagrams);
	buildComponents(diagrams, supply_bar);
}

SimulationModel::~SimulationModel()
{
	qDeleteAll(m_valves);
	qDeleteAll(m_cylinders);
	qDeleteAll(m_pumps);
	qDeleteAll(m_motors);
	qDeleteAll(m_flow_valves);
	qDeleteAll(m_contacts);
	qDeleteAll(m_relays);
	qDeleteAll(m_coils);
	qDeleteAll(m_grafcet_io);
	qDeleteAll(m_counters);
	qDeleteAll(m_plc_channels);
	qDeleteAll(m_plcs);
	qDeleteAll(m_accumulators);
	qDeleteAll(m_pressure_switches);
	qDeleteAll(m_ejectors);
	qDeleteAll(m_pipes);
	qDeleteAll(m_prop_valves);
	qDeleteAll(m_cups);
	qDeleteAll(m_analog);
}

void SimulationModel::buildNets(const QList<Diagram *> &diagrams)
{
	QHash<Terminal *, Terminal *> parent;
	std::function<Terminal *(Terminal *)> root = [&](Terminal *t) {
		while (parent.value(t, t) != t) {
			Terminal *up = parent.value(t, t);
			parent[t] = parent.value(up, up);
			t = parent[t];
		}
		return t;
	};
	auto join = [&](Terminal *a, Terminal *b) {
		if (!a || !b) return;
		Terminal *ra = root(a), *rb = root(b);
		if (ra != rb) parent.insert(ra, rb);
	};
	QSet<Terminal *> connected;
	for (Diagram *d : diagrams) {
		for (Element *e : sortedElements(d))
			for (Terminal *t : e->terminals())
				if (!parent.contains(t)) parent.insert(t, t);
	}
	for (Diagram *d : diagrams) {
		for (Conductor *c : d->conductors()) {
			join(c->terminal1, c->terminal2);
			connected << c->terminal1 << c->terminal2;
		}
		// folio reports join the nets on both pages
		for (Element *e : sortedElements(d)) {
			if (!(e->linkType() & Element::AllReport)) continue;
			for (Element *other : e->linkedElements())
				if (!e->terminals().isEmpty() && !other->terminals().isEmpty()) {
					join(e->terminals().first(), other->terminals().first());
					connected << e->terminals().first() << other->terminals().first();
				}
		}
	}
	// ideal connections (catalog role) and in-line measuring points join all their fluid ports
	for (Diagram *d : diagrams)
		for (Element *e : sortedElements(d)) {
			const SimulationCatalog::Entry *ce = SimulationCatalog::find(e->elementData().m_fluid.m_function);
			const bool line = ce && (ce->model == QLatin1String("line") || ce->model == QLatin1String("coupling"));
			if (!ce || (ce->role != SimulationCatalog::Role::Ideal && ce->role != SimulationCatalog::Role::Display
						&& !(line && lineMode(e->elementData().m_fluid) == LineMode::Ideal))) continue;
			Terminal *first = nullptr;
			bool any = false;
			const auto pairs = line ? channelPairs(e) : QList<QPair<Terminal *, Terminal *>>();
			for (Terminal *t : e->terminals()) {
				if (t->medium() != Fluid::Medium::Pneumatic && t->medium() != Fluid::Medium::Hydraulic) continue;
				any = any || connected.contains(t);
				if (!pairs.isEmpty()) continue;
				if (!first) first = t; else join(first, t);
			}
			for (const auto &pr : pairs) join(pr.first, pr.second);   // R05: rotary joint, one net per channel
			if (any)
				for (Terminal *t : e->terminals())
					if (t->medium() == Fluid::Medium::Pneumatic || t->medium() == Fluid::Medium::Hydraulic) connected << t;
		}
	// R09: the shock and anti-cavitation block sits in the work lines: A1 = A2 and B1 = B2 are one net each
	for (Diagram *d : diagrams)
		for (Element *e : sortedElements(d)) {
			if (e->elementData().m_fluid.m_function != QLatin1String("pressure_valve.shock_and_anti_cavitation_valve")) continue;
			for (const auto &pr : {qMakePair(QStringLiteral("A1"), QStringLiteral("A2")), qMakePair(QStringLiteral("B1"), QStringLiteral("B2"))}) {
				Terminal *a = nullptr, *b = nullptr;
				for (Terminal *t : e->terminals()) {
					if (t->name().trimmed() == pr.first) a = t;
					if (t->name().trimmed() == pr.second) b = t;
				}
				if (!a || !b) continue;
				join(a, b);
				if (connected.contains(a) || connected.contains(b)) connected << a << b;
			}
		}
	QHash<Terminal *, int> index;
	for (auto it = parent.constBegin(); it != parent.constEnd(); ++it) {
		Terminal *t = it.key();
		if (!connected.contains(t)) continue;   // an open port has no node
		Terminal *r = root(t);
		if (!index.contains(r)) index.insert(r, static_cast<int>(index.size()));
		m_node_of.insert(t, index.value(r));
	}
	// a free fluid port is open to the air unless the symbol closes it with a plug (own, sealed net)
	// or vents it through a silencer (own net, restricted to the air)
	int nets = static_cast<int>(index.size());
	for (auto it = parent.constBegin(); it != parent.constEnd(); ++it) {
		Terminal *t = it.key();
		if (connected.contains(t)) continue;
		const Fluid::Medium m = t->medium();
		if (m != Fluid::Medium::Pneumatic && m != Fluid::Medium::Hydraulic) continue;
		const Fluid::PortEnd end = t->portEnd();
		if (end == Fluid::PortEnd::Plug) {
			m_node_of.insert(t, nets++);
		} else if (end == Fluid::PortEnd::Silencer) {
			m_silenced << nets;
			m_node_of.insert(t, nets++);
		}
	}
	m_p.fill(P_ATM, nets);
	m_watch.fill(SimPhysics::OscillationWatch{}, nets);
	// R03: one net with pneumatic and hydraulic ports (or fluid and electric ones) is a drawing error
	{
		QHash<int, QSet<int>> media;
		QHash<int, QString> where;
		for (auto it = m_node_of.constBegin(); it != m_node_of.constEnd(); ++it) {
			const Fluid::Medium m = it.key()->medium();
			const int kind = m == Fluid::Medium::Pneumatic ? 1 : m == Fluid::Medium::Hydraulic ? 2 : m == Fluid::Medium::Electric ? 3 : 0;
			if (!kind) continue;
			media[it.value()] << kind;
			if (Element *e = it.key()->parentElement()) {
				const QString l = e->elementInformations()[QStringLiteral("label")].toString();
				where[it.value()] = l.isEmpty() ? e->name() : l;
			}
		}
		for (auto it = media.constBegin(); it != media.constEnd(); ++it)
			if (it.value().size() > 1)
				m_model_errors << QStringLiteral("Netz an %1: verbindet %2 – Leitung prüfen").arg(where.value(it.key()),
					it.value().contains(3) ? QStringLiteral("Fluid- und elektrische Anschlüsse")
										   : QStringLiteral("Pneumatik- und Hydraulikanschlüsse"));
	}
	for (auto it = m_node_of.constBegin(); it != m_node_of.constEnd(); ++it) {
		const Fluid::Medium m = it.key()->medium();
		if (m == Fluid::Medium::Hydraulic) m_hydraulic << it.value();
		if (m == Fluid::Medium::Electric) m_electric << it.value();
	}
}

int SimulationModel::node(Terminal *t) const { return t ? m_node_of.value(t, -1) : -1; }

int SimulationModel::node(Element *e, const QString &port) const
{
	for (Terminal *t : e->terminals())
		if (t->name() == port) return node(t);
	return -1;
}

void SimulationModel::buildComponents(const QList<Diagram *> &diagrams, double supply_bar)
{
	const double p_supply = P_ATM + supply_bar * 1e5;
	static const QHash<QString, double> flow_qnn {
		{QStringLiteral("supply.silencer"), 300}, {QStringLiteral("shutoff_valve.quick_exhaust_valve"), 300},
		{QStringLiteral("shutoff_valve.check_valve"), 150}, {QStringLiteral("shutoff_valve.shutoff_valve"), 200},
		{QStringLiteral("flow_control_valve.throttle_valve"), 40},
		{QStringLiteral("flow_control_valve.one_way_flow_control_valve"), 40},
		{QStringLiteral("pressure_valve.pressure_regulator"), 200},
		{QStringLiteral("pressure_valve.pressure_relief_valve01"), 200},
		{QStringLiteral("flow_control_valve.flow_divider"), 100},
		{QStringLiteral("logic.shuttle_valve"), 150}, {QStringLiteral("logic.dual_pressure_valve"), 150},
		{QStringLiteral("logic.time_delay_valve"), 100},
		{QStringLiteral("pressure_valve.counterbalance_valve"), 200},
		{QStringLiteral("pressure_valve.pressure_sequence_valve01"), 200},
		{QStringLiteral("pressure_valve.pressure_sequence_valve02"), 200},
		{QStringLiteral("proportional.pressure_relief_valve"), 200}, {QStringLiteral("proportional.flow_sensor"), 1000},
		{QStringLiteral("sensor.flowmeter_2"), 1000},
		{QStringLiteral("pressure_valve.pressure_compensator_closer01"), 300}, {QStringLiteral("pressure_valve.pressure_compensator_closer02"), 300},
		{QStringLiteral("pressure_valve.pressure_compensator_opener01"), 300}, {QStringLiteral("pressure_valve.pressure_compensator_opener02"), 300},
		{QStringLiteral("pressure_valve.pressure_compensator_upstream_1"), 300}, {QStringLiteral("pressure_valve.pressure_compensator_upstream_2"), 300},
		{QStringLiteral("pressure_valve.pressure_compensator_downstream_1"), 300}, {QStringLiteral("pressure_valve.pressure_compensator_downstream_2"), 300},
		{QStringLiteral("pressure_valve.pressure_compensator_opener_ls_1"), 300}, {QStringLiteral("pressure_valve.pressure_compensator_opener_ls_2"), 300},
		{QStringLiteral("pressure_valve.shock_and_anti_cavitation_valve"), 300}, {QStringLiteral("pressure_valve.pressure_sequence_valve"), 100}};

	for (Diagram *d : diagrams) {
		for (Element *e : sortedElements(d)) {
			const ElementData data = e->elementData();
			const FluidData &f = data.m_fluid;
			const QString fn = f.m_function;
			const bool hyd = medium(e) == Fluid::Medium::Hydraulic;
			if (!f.isNull() && f.m_kind != FluidData::Kind::Solenoid) {
				// R03: two parts holding one net at different fixed pressures contradict each other
				auto fixPressure = [&](int k, double p, const QString &who) {
					if (m_fixed.contains(k) && std::abs(m_fixed.value(k) - p) > 1.0) {
						m_model_errors << QStringLiteral("Netz an %1: widersprüchliche Quellen – %2 hält %3 bar, %4 hält %5 bar")
											  .arg(netName(k), m_fixed_by.value(k)).arg((m_fixed.value(k) - P_ATM) / 1e5, 0, 'f', 2)
											  .arg(who).arg((p - P_ATM) / 1e5, 0, 'f', 2);
						return;
					}
					m_fixed.insert(k, p);
					m_fixed_by.insert(k, who);
				};
				// the model comes from the catalog (R00), never from the kind or the symbol: an unknown
				// function or one without a model stops the run before it starts
				const SimulationCatalog::Entry *ce = SimulationCatalog::find(fn);
				const QString part = label(e).isEmpty() ? e->name() : label(e);
				if (!ce) {
					m_model_errors << QStringLiteral("%1: Funktion „%2“ steht nicht im Modellkatalog %3 – kein Rechenmodell")
										  .arg(part, fn.isEmpty() ? QStringLiteral("(keine)") : fn, SimulationCatalog::version());
					continue;
				}
				if (ce->role == SimulationCatalog::Role::Unsupported) {
					m_model_errors << QStringLiteral("%1: %2 hat kein Rechenmodell (%3)").arg(part, fn, ce->limits);
					continue;
				}
				if (ce->role != SimulationCatalog::Role::Physical) continue;   // ideal, display, graphic
				QString model = ce->model;
				// a pneumatic "pump" is a compressor: an ideal pressure source (catalog entry supply.pump)
				if (model == QLatin1String("fixed_displacement_pump") && !hyd) model = QStringLiteral("pressure_source");
				if (model == QLatin1String("line") || model == QLatin1String("coupling")) {
					// R05 (FORMELN.md L04): the mode follows the given values; ideal lines were joined in buildNets
					const LineMode lm = lineMode(f);
					if (lm == LineMode::Ideal) continue;
					if (lm == LineMode::Open) {
						// uncoupled: a self-sealing coupling (seal 1, default) closes both halves, else both vent
						if (f.parameter(QStringLiteral("seal")).trimmed() == QLatin1String("0"))
							for (Terminal *t : e->terminals())
								if (node(t) >= 0) m_silenced << node(t);
						continue;
					}
					if (lm == LineMode::Resistance) {
						model = QStringLiteral("passthrough");
					} else {
						auto *pi = new Pipe;
						pi->e = e;
						pi->hydraulic = hyd;
						QStringList ports;
						for (Terminal *t : e->terminals())
							if (t->medium() == Fluid::Medium::Pneumatic || t->medium() == Fluid::Medium::Hydraulic) ports << t->name();
						pi->a = ports.value(0);
						pi->b = ports.value(1);
						const QString m = QStringLiteral("m");
						pi->flow.L = paramIn(f, QStringLiteral("length"), 1.0, m, &m_model_errors, part, Range::Positive);
						pi->flow.d = paramIn(f, QStringLiteral("d_inner"), 0.01, m, &m_model_errors, part, Range::Positive);
						pi->flow.eps = paramIn(f, QStringLiteral("roughness"), 15e-6, m, &m_model_errors, part, Range::NonNegative);
						// oil HLP 22 (FluidSIM reference oil): 22 mm²/s, 870 kg/m³; air 1.81e-5 Pa·s at 20 °C (FORMELN.md L01)
						const double rho = paramIn(f, QStringLiteral("rho"), 870.0, QStringLiteral("kg/m3"), &m_model_errors, part, Range::Positive);
						const double nu = paramIn(f, QStringLiteral("nu"), 22e-6, QStringLiteral("m2/s"), &m_model_errors, part, Range::Positive);
						pi->flow.rho = hyd ? rho : 1.2;
						pi->flow.mu = hyd ? rho * nu : 1.81e-5;
						const int ka = node(e, pi->a), kb = node(e, pi->b);
						const double volume = pi->flow.area() * pi->flow.L;   // the fluid in the line, half to each end
						if (ka >= 0) m_extra_volume[ka] += volume / 2;
						if (kb >= 0) m_extra_volume[kb] += volume / 2;
						m_pipes << pi;
						continue;
					}
				}
				if (model == QLatin1String("proportional_valve")) {
					// R07 (FORMELN.md V05, V06): FluidSIM 4/3 proportional valve; a linked solenoid or amplifier (R06,
					// linkAnalog) sets u every step, else the setpoint is the value u or a setpoint event
					if (!hyd) {
						m_model_errors << QStringLiteral("%1: Stetigventil nur hydraulisch gerechnet").arg(part);
						continue;
					}
					auto *v = new PropValve;
					v->e = e;
					v->label = label(e);
					v->u = paramIn(f, QStringLiteral("u"), 0.0, QStringLiteral("V"), &m_model_errors, part);
					v->tau = paramIn(f, QStringLiteral("tau"), fn.endsWith(QLatin1String("43_02")) ? 0.004 : 0.01,
									 QStringLiteral("s"), &m_model_errors, part, Range::NonNegative);
					const double lap = paramNative(f, QStringLiteral("lap"), 0.0, QStringLiteral("%"), &m_model_errors, part, Range::Any) / 100.0;
					v->lap_p = paramNative(f, QStringLiteral("lapP"), lap * 100.0, QStringLiteral("%"), &m_model_errors, part, Range::Any) / 100.0;
					v->lap_t = paramNative(f, QStringLiteral("lapT"), lap * 100.0, QStringLiteral("%"), &m_model_errors, part, Range::Any) / 100.0;
					if (!(v->lap_p > -1.0 && v->lap_p < 1.0 && v->lap_t > -1.0 && v->lap_t < 1.0))
						m_model_errors << QStringLiteral("%1: Überdeckung muss zwischen -100 und 100 % liegen").arg(part);
					v->rt = paramNative(f, QStringLiteral("rT"), 1.0, QStringLiteral("bar.min2/l2"), &m_model_errors, part, Range::Positive);
					v->gleak = paramIn(f, QStringLiteral("GLeak"), 0.0, QStringLiteral("m3/(s.Pa)"), &m_model_errors, part, Range::NonNegative);
					// FluidSIM notchType: 1 rectangular, 2 triangular (default of the 4/3 valve 01)
					v->triangular = paramNative(f, QStringLiteral("notchType"), fn.endsWith(QLatin1String("43_02")) ? 1.0 : 2.0,
												QString(), &m_model_errors, part, Range::Any) > 1.5;
					v->s = qBound(-1.0, v->u / 10.0, 1.0);
					m_prop_valves << v;
					continue;
				}
				if (model == QLatin1String("tank")) {
					for (Terminal *t : e->terminals())
						if (node(t) >= 0) fixPressure(node(t), P_ATM, part);
				} else if (model == QLatin1String("fixed_displacement_pump")) {
					auto *p = new Pump;
					p->e = e;
					const QString who = label(e).isEmpty() ? e->name() : label(e);
					p->q = paramIn(f, QStringLiteral("displacement"), 8e-6, QStringLiteral("m3"), &m_model_errors, who, Range::NonNegative)
						 * paramIn(f, QStringLiteral("rpm"), 1450.0 / 60.0, QStringLiteral("1/s"), &m_model_errors, who, Range::NonNegative);
					p->p_max = P_ATM + paramIn(f, QStringLiteral("pMax"), 6e6, QStringLiteral("Pa"), &m_model_errors, who, Range::Positive);
					// R08: setting of a variable pump (0 … 1) and leakage from P to T
					p->alpha = paramNative(f, QStringLiteral("alpha"), 1.0, QString(), &m_model_errors, who, Range::NonNegative);
					if (p->alpha > 1.0) m_model_errors << QStringLiteral("%1: Verstellgrad alpha %2 größer als 1").arg(who).arg(p->alpha);
					p->cleak = paramIn(f, QStringLiteral("GL"), 0.0, QStringLiteral("m3/(s.Pa)"), &m_model_errors, who, Range::NonNegative);
					m_pumps << p;
				} else if ((model == QLatin1String("hydraulic_motor") && !hyd) || model == QLatin1String("air_motor")) {
					// R12 (FORMELN.md M11): compressed air motor as a shaft (M01/M06 family). With FluidSIM rated values
					// (p_N, rpm_N, M_N, n0_nN = k) a linear characteristic: stall torque M_N·k/(k − 1), no-load speed
					// k·n_N at p_N; displacement per radian = stall torque / p_N, viscous friction = stall torque / ω₀
					auto *mo = new Motor;
					mo->e = e;
					mo->label = part;
					mo->air = true;
					const QString who = part;
					auto &s = mo->shaft;
					s.alpha = 1.0;
					if (fn == QLatin1String("actuator.motor_with_mid_exhaust")) {
						const double pn = paramIn(f, QStringLiteral("p_N"), 0.3e6, QStringLiteral("Pa"), &m_model_errors, who, Range::Positive);
						const double nn = paramIn(f, QStringLiteral("rpm_N"), 975.0 / 60.0, QStringLiteral("1/s"), &m_model_errors, who, Range::Positive);
						const double mn = paramIn(f, QStringLiteral("M_N"), 0.62, QStringLiteral("N.m"), &m_model_errors, who, Range::Positive);
						const double k = paramNative(f, QStringLiteral("n0_nN"), 2.0, QString(), &m_model_errors, who, Range::Positive);
						if (!(k > 1.0)) m_model_errors << QStringLiteral("%1: n0_nN %2 muss größer als 1 sein").arg(who).arg(k);
						const double stall = mn * k / std::max(k - 1.0, 1e-3);
						s.D = 2.0 * M_PI * stall / pn;
						s.b = stall / (k * 2.0 * M_PI * nn);
						mo->exhaust = QStringLiteral("EX");
					} else {
						s.D = paramIn(f, QStringLiteral("displacement"), 1e-4, QStringLiteral("m3"), &m_model_errors, who, Range::Positive);
						s.b = paramIn(f, QStringLiteral("f"), 3.0, QStringLiteral("N.m.s/rad"), &m_model_errors, who, Range::NonNegative);
					}
					s.J = paramIn(f, QStringLiteral("I"), 2e-3, QStringLiteral("kg.m2"), &m_model_errors, who, Range::Positive);
					const double mext = paramIn(f, QStringLiteral("M_ext"), 0.0, QStringLiteral("N.m"), &m_model_errors, who);
					// FluidSIM torqueType 1: braking (against the rotation, like dry friction), 2: driving
					if (paramNative(f, QStringLiteral("torqueType"), 1.0, QString(), &m_model_errors, who, Range::Any) > 1.5) mo->tau_load = -mext;
					else s.tau_c = std::abs(mext);
					m_motors << mo;
				} else if (model == QLatin1String("hydraulic_motor")) {
					auto *mo = new Motor;
					mo->e = e;
					mo->label = part;
					const QString who = part;
					auto &s = mo->shaft;
					s.D = paramIn(f, QStringLiteral("displacement"), 8e-6, QStringLiteral("m3"), &m_model_errors, who, Range::Positive);
					s.alpha = paramNative(f, QStringLiteral("alpha"), 1.0, QString(), &m_model_errors, who, Range::Any);
					if (std::abs(s.alpha) > 1.0) m_model_errors << QStringLiteral("%1: Verstellgrad alpha %2 außerhalb -1 … 1").arg(who).arg(s.alpha);
					s.cleak = paramIn(f, QStringLiteral("GLeak"), 0.0, QStringLiteral("m3/(s.Pa)"), &m_model_errors, who, Range::NonNegative);
					s.J = paramIn(f, f.parameter(QStringLiteral("I")).isEmpty() ? QStringLiteral("J") : QStringLiteral("I"), 1e-3,
								  QStringLiteral("kg.m2"), &m_model_errors, who, Range::Positive);
					s.b = paramIn(f, QStringLiteral("f"), 1e-3, QStringLiteral("N.m.s/rad"), &m_model_errors, who, Range::NonNegative);
					s.tau_c = paramIn(f, QStringLiteral("M_fric"), 0.0, QStringLiteral("N.m"), &m_model_errors, who, Range::NonNegative);
					mo->tau_load = paramIn(f, f.parameter(QStringLiteral("M_ext")).isEmpty() ? QStringLiteral("M_load") : QStringLiteral("M_ext"),
										   0.0, QStringLiteral("N.m"), &m_model_errors, who);
					m_motors << mo;
				} else if (model == QLatin1String("accumulator")) {
					// a finite volume, not an ideal source: hydraulic accumulator with gas precharge,
					// compressed air receiver with its volume
					QList<int> ks;
					for (Terminal *t : e->terminals())
						if (node(t) >= 0) ks << node(t);
					const QString who = label(e).isEmpty() ? e->name() : label(e);
					const double v0 = paramIn(f, QStringLiteral("V0"), 1e-3, QStringLiteral("m3"), &m_model_errors, who, Range::Positive);
					const double p0 = paramIn(f, QStringLiteral("p0"), 0.4e6, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative);
					if (ks.isEmpty()) continue;
					if (hyd) {
						auto *a = new Accumulator;
						a->e = e;
						a->label = label(e);
						a->node = ks.first();
						a->v0 = v0;
						a->p_pre = P_ATM + p0;
						a->n = paramNative(f, QStringLiteral("k"), 1.4, QString(), &m_model_errors, who, Range::Positive);
						m_accumulators << a;
					} else {
						m_extra_volume[ks.first()] += v0;
						for (int k : ks) m_p[k] = P_ATM + p0;
						for (int i = 1; i < ks.size(); ++i) m_vessel_links << qMakePair(ks.first(), ks.at(i));
					}
				} else if (model == QLatin1String("pressure_source")) {
					// a source with its own nominal pressure (FluidSIM "pNom", taken over on import) keeps it;
					// the others supply the pressure set for the whole simulation
					double p = p_supply;
					if (!f.parameter(QStringLiteral("pNom")).isEmpty()) {
						const QString who = label(e).isEmpty() ? e->name() : label(e);
						p = P_ATM + paramIn(f, QStringLiteral("pNom"), supply_bar * 1e5, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative);
					}
					for (Terminal *t : e->terminals())
						if (node(t) >= 0) fixPressure(node(t), p, part);
				} else if (model == QLatin1String("directional_valve") && f.m_positions.isEmpty()) {
					m_model_errors << QStringLiteral("%1: Wegeventil ohne Stellungen und Schaltwege").arg(part);
				} else if (model == QLatin1String("directional_valve")) {
					auto *v = new Valve;
					v->e = e;
					v->label = label(e);
					for (const auto &pos : f.m_positions) {
						QList<QPair<QString, QString>> paths;
						for (const auto &path : pos.paths)
							if (!path.to.isEmpty()) paths << qMakePair(path.from, path.to);
						v->positions << paths;
					}
					v->rest = v->position = v->target
						= qBound(0, f.m_initial_position - 1, static_cast<int>(f.m_positions.size()) - 1);
					v->side[0] = f.m_left;
					v->side[1] = f.m_right;
					v->dominant = f.m_dominant == QLatin1String("left") ? 0 : f.m_dominant == QLatin1String("right") ? 1 : -1;
					{
						const auto handOnly = [](const FluidData::Actuation &a) {
							return a.types == QStringList{QStringLiteral("manual")} && !a.spring && !a.air_spring && !a.pilot;
						};
						if (v->positions.size() >= 2) {
							if (handOnly(f.m_left) && f.m_right.isNull()) v->toggle_side = 0;
							else if (handOnly(f.m_right) && f.m_left.isNull()) v->toggle_side = 1;
						}
					}
					{
						// "-MM1 0..2": actuated while the piston of -MM1 is between 0 and 2 mm
						const QStringList parts = info(e, "function").split(QLatin1Char(' '), Qt::SkipEmptyParts);
						const int dots = parts.size() == 2 ? static_cast<int>(parts.at(1).indexOf(QLatin1String(".."))) : -1;
						bool ok1 = false, ok2 = false;
						if (dots > 0) {
							v->trigger_lo = parts.at(1).left(dots).toDouble(&ok1) / 1000.0;
							v->trigger_hi = parts.at(1).mid(dots + 2).toDouble(&ok2) / 1000.0;
						}
						if (ok1 && ok2) {
							v->trigger_cylinder = parts.at(0);
							const auto mech = [](const FluidData::Actuation &a) {
								for (const QString &ty : a.types)
									if (ty == QLatin1String("roller") || ty == QLatin1String("idle_roller") || ty == QLatin1String("plunger"))
										return true;
								return false;
							};
							v->trigger_side = mech(f.m_left) || !mech(f.m_right) ? 0 : 1;
						}
					}
					const QString who = v->label.isEmpty() ? e->name() : v->label;
					v->c = conductanceFromQnn(paramIn(f, QStringLiteral("qNN"), 60.0, QStringLiteral("l/min"), &m_model_errors, who, Range::Positive));
					v->rt = paramNative(f, QStringLiteral("rT"), 0.1, QStringLiteral("bar.min2/l2"), &m_model_errors, who, Range::Positive);
					v->t_on = paramIn(f, QStringLiteral("onDelay"), 0.02, QStringLiteral("s"), &m_model_errors, who, Range::NonNegative);
					v->t_off = paramIn(f, QStringLiteral("offDelay"), 0.03, QStringLiteral("s"), &m_model_errors, who, Range::NonNegative);
					v->hydraulic = hyd;
					m_valves << v;
				} else if (model == QLatin1String("rotary_actuator")) {
					// R10 semi-rotary drive (FORMELN.md M06): the rotary analogue of the cylinder. M_6bar is the torque at
					// 6 bar, so the displacement per radian (= torque per pascal) is M_6bar / 6 bar on
					// both sides; angle and stroke in rad, inertia I as mass, viscous friction f
					auto *c = new Cylinder;
					c->e = e;
					c->rotary = true;
					c->label = label(e);
					const QString who = c->label.isEmpty() ? e->name() : c->label;
					const QString rad = QStringLiteral("rad"), nm = QStringLiteral("N.m");
					const double m6 = paramIn(f, QStringLiteral("M_6bar"), 2.0, nm, &m_model_errors, who, Range::Positive);
					c->a_k = c->a_r = m6 / 6e5;
					c->stroke = paramIn(f, QStringLiteral("xMax"), M_PI, rad, &m_model_errors, who, Range::Positive);
					c->x = paramIn(f, QStringLiteral("xStart"), 0.0, rad, &m_model_errors, who, Range::NonNegative);
					c->m = paramIn(f, QStringLiteral("I"), 2.5e-3, QStringLiteral("kg.m2"), &m_model_errors, who, Range::Positive);
					c->damping = paramIn(f, QStringLiteral("f"), 0.1, QStringLiteral("N.m.s/rad"), &m_model_errors, who, Range::NonNegative);
					c->f_static = paramIn(f, QStringLiteral("M_stat"), 0.02 * m6, nm, &m_model_errors, who, Range::NonNegative);
					c->f_slide = 0.7 * c->f_static;
					c->f_load = paramIn(f, QStringLiteral("M_ext"), 0.0, nm, &m_model_errors, who);
					c->stop_k = paramIn(f, QStringLiteral("stop_k"), 0.0, QStringLiteral("N.m/rad"), &m_model_errors, who, Range::NonNegative);
					c->stop_c = paramIn(f, QStringLiteral("stop_c"), 0.0, QStringLiteral("N.m.s/rad"), &m_model_errors, who, Range::NonNegative);
					m_cylinders << c;
					m_end_position.insert(c->label, QString());
				} else if (model == QLatin1String("vacuum_generator")) {
					// R10 ejector: the driving air 1 -> exhaust sucks the vacuum port 2. Linear suction
					// characteristic between the best vacuum pu_min (no flow) and the atmosphere (qns_max),
					// both scaled with the supply pressure up to the optimum dp_max
					auto *v = new Ejector;
					v->e = e;
					v->label = label(e);
					const QString who = v->label.isEmpty() ? e->name() : v->label;
					const QString pa = QStringLiteral("Pa"), lmin = QStringLiteral("l/min");
					v->pu_min = paramIn(f, f.parameter(QStringLiteral("pu_min")).isEmpty() ? QStringLiteral("p_vac") : QStringLiteral("pu_min"), -0.085e6, pa, &m_model_errors, who);
					v->dp_opt = paramIn(f, QStringLiteral("dp_max"), 0.5e6, pa, &m_model_errors, who, Range::Positive);
					const double qac = paramIn(f, QStringLiteral("qn_ac"), 11.6, lmin, &m_model_errors, who, Range::Positive);
					const double qs = paramIn(f, QStringLiteral("qns_max"), 6.0, lmin, &m_model_errors, who, Range::Positive);
					if (!(v->pu_min < 0) || v->pu_min <= -P_ATM)
						m_model_errors << QStringLiteral("%1: Vakuum pu_min %2 kPa muss zwischen 0 und -101 kPa liegen").arg(who).arg(v->pu_min / 1e3);
					v->c_drive = conductanceAt(qac, P_ATM + 6e5, P_ATM);   // FluidSIM: "Luftverbrauch bei 600 kPa" (QS05)
					v->q_max = qs / 60000.0 * RHO_N;
					m_ejectors << v;
				} else if (model == QLatin1String("suction_cup")) {
					// R10 suction cup: holding force (p_atm - p)·π d²/4 against the weight m·g with the
					// safety factor s; leakage qLeak at -0.6 bar when a workpiece lies on it, else open
					auto *s = new SuctionCup;
					s->e = e;
					s->label = label(e);
					for (Terminal *t : e->terminals())
						if (node(t) >= 0) { s->node = node(t); break; }
					const QString who = s->label.isEmpty() ? e->name() : s->label;
					const double d = paramIn(f, QStringLiteral("d"), 0.02, QStringLiteral("m"), &m_model_errors, who, Range::Positive);
					s->area = M_PI * d * d / 4;
					s->weight = 9.81 * paramIn(f, QStringLiteral("m"), 0.1, QStringLiteral("kg"), &m_model_errors, who, Range::NonNegative);
					s->safety = paramNative(f, QStringLiteral("s"), 1.0, QString(), &m_model_errors, who, Range::Positive);
					const double leak = paramIn(f, QStringLiteral("qLeak"), 0.0, QStringLiteral("l/min"), &m_model_errors, who, Range::NonNegative);
					s->c_leak = leak > 0 ? conductanceAt(leak, P_ATM, P_ATM - 0.6e5) : 0.0;
					s->c_open = conductanceAt(50.0 * d / 0.02, P_ATM, P_ATM - 0.1e5);   // open cup: 50 l/min at 0.1 bar (d 20 mm)
					s->object = paramNative(f, QStringLiteral("object"), 1.0, QString(), &m_model_errors, who, Range::NonNegative) > 0.5;
					if (s->node >= 0) m_extra_volume[s->node] += s->area * 0.005;   // cup volume: 5 mm deep
					m_cups << s;
				} else if (model == QLatin1String("linear_cylinder")) {
					auto *c = new Cylinder;
					c->e = e;
					c->label = label(e);
					const QString who = c->label.isEmpty() ? e->name() : c->label;
					const QString m = QStringLiteral("m");
					const double dp = paramIn(f, QStringLiteral("d_piston"), (f.m_bore_mm > 0 ? f.m_bore_mm : 20.0) / 1000.0, m, &m_model_errors, who, Range::Positive);
					const double dr = paramIn(f, QStringLiteral("d_rod"), (f.m_rod_mm > 0 ? f.m_rod_mm : 8.0) / 1000.0, m, &m_model_errors, who, Range::NonNegative);
					if (std::isfinite(dp) && std::isfinite(dr) && dp > 0 && dr >= dp)
						m_model_errors << QStringLiteral("%1: Kolbenstange Ø %2 mm nicht kleiner als Kolben Ø %3 mm")
											  .arg(who).arg(dr * 1000).arg(dp * 1000);
					c->stroke = paramIn(f, QStringLiteral("xMax"), (f.m_stroke_mm > 0 ? f.m_stroke_mm : 100.0) / 1000.0, m, &m_model_errors, who, Range::Positive);
					c->a_k = M_PI * dp * dp / 4;
					c->a_r = c->a_k - M_PI * dr * dr / 4;
					const QString kg = QStringLiteral("kg"), newton = QStringLiteral("N");
					c->m = paramIn(f, QStringLiteral("m_cyl"), hyd ? 2.0 : 0.1, kg, &m_model_errors, who, Range::NonNegative)
						 + paramIn(f, QStringLiteral("m_ext"), 0.0, kg, &m_model_errors, who, Range::NonNegative);
					c->f_static = paramIn(f, QStringLiteral("F_stat"), 0.05 * c->a_k * 6e5, newton, &m_model_errors, who, Range::NonNegative);
					c->f_slide = 0.7 * c->f_static;
					c->f_spring = f.m_spring != FluidData::SpringSide::None ? 0.15 * c->a_k * 6e5 : 0.0;
					c->f_load = paramIn(f, QStringLiteral("F_extern_const"), 0.0, newton, &m_model_errors, who);
					c->x = paramIn(f, QStringLiteral("xStart"), 0.0, m, &m_model_errors, who, Range::NonNegative);
					// elastic end stops (R02): stiffness [N/m] and damping [N.s/m]; without them hard stops
					c->stop_k = paramIn(f, QStringLiteral("stop_k"), 0.0, QStringLiteral("N/m"), &m_model_errors, who, Range::NonNegative);
					c->stop_c = paramIn(f, QStringLiteral("stop_c"), 0.0, QStringLiteral("N.s/m"), &m_model_errors, who, Range::NonNegative);
					// FluidSIM "automatic friction": a viscous force F_ref at the speed v_ref (F_ref is the piston
					// force at the reference pressure, so the free cylinder runs at about v_ref there)
					if (!f.parameter(QStringLiteral("F_ref")).isEmpty() && !f.parameter(QStringLiteral("v_ref")).isEmpty()) {
						const double fr = paramIn(f, QStringLiteral("F_ref"), 0.0, newton, &m_model_errors, who, Range::NonNegative);
						const double vr = paramIn(f, QStringLiteral("v_ref"), 1.0, QStringLiteral("m/s"), &m_model_errors, who, Range::Positive);
						if (fr > 0 && vr > 0) c->damping = fr / vr;
					}
					// R11 (FORMELN.md M08–M10): the FluidSIM cylinder model. Weight m·g·sin(alpha) against extending; friction
					// of the external mass mu·m_ext·g·|cos(alpha)|; hydraulic piston friction F_B (breakaway), F_M (Coulomb),
					// F_V (viscous); spring pressure p_spring_min at x = 0 … p_spring_max at the stroke on the piston area;
					// force profile F_var_ex/F_var_re {x [mm], F [N], …} instead of the constant force (use_F_const False)
					{
						const auto yes = [&](const char *n) { return f.parameter(QLatin1String(n)).trimmed().compare(QLatin1String("true"), Qt::CaseInsensitive) == 0; };
						const double alpha = paramIn(f, QStringLiteral("alpha"), 0.0, QStringLiteral("rad"), &m_model_errors, who);
						const double g = 9.81;
						const double m_ext = paramIn(f, QStringLiteral("m_ext"), 0.0, kg, &m_model_errors, who, Range::NonNegative);
						c->weight = c->m * g * std::sin(alpha);
						if (!f.parameter(QStringLiteral("F_B")).isEmpty()) {
							c->f_static = paramIn(f, QStringLiteral("F_B"), c->f_static, newton, &m_model_errors, who, Range::NonNegative);
							c->f_slide = paramIn(f, QStringLiteral("F_M"), 0.7 * c->f_static, newton, &m_model_errors, who, Range::NonNegative);
							c->damping = paramIn(f, QStringLiteral("F_V"), c->damping, QStringLiteral("N.s/m"), &m_model_errors, who, Range::NonNegative);
						}
						const double normal = m_ext * g * std::abs(std::cos(alpha));
						c->f_static += paramNative(f, QStringLiteral("mu_B_ext"), 0.0, QString(), &m_model_errors, who, Range::NonNegative) * normal;
						c->f_slide += paramNative(f, QStringLiteral("mu_M_ext"), 0.0, QString(), &m_model_errors, who, Range::NonNegative) * normal;
						c->f_static = std::max(c->f_static, c->f_slide);
						if (!f.parameter(QStringLiteral("spring_left")).isEmpty() || !f.parameter(QStringLiteral("spring_right")).isEmpty()) {
							c->f_spring = 0.0;
							c->spring_dir = yes("spring_left") ? 1 : yes("spring_right") ? -1 : 0;
							c->spring_f0 = c->a_k * paramIn(f, QStringLiteral("p_spring_min"), 0.4e5, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative);
							c->spring_f1 = c->a_k * paramIn(f, QStringLiteral("p_spring_max"), 1e5, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative);
						}
						if (f.parameter(QStringLiteral("use_F_const")).trimmed().compare(QLatin1String("false"), Qt::CaseInsensitive) == 0) {
							auto profile = [&](const char *n, QVector<QPair<double, double>> *out) {
								QString s = f.parameter(QLatin1String(n));
								s.remove(QLatin1Char('{')).remove(QLatin1Char('}'));
								const QStringList v = s.split(QLatin1Char(','), Qt::SkipEmptyParts);
								for (int i = 0; i + 1 < v.size(); i += 2) {
									bool ok1 = false, ok2 = false;
									const double x = v.at(i).trimmed().toDouble(&ok1) / 1000.0, F = v.at(i + 1).trimmed().toDouble(&ok2);
									if (!ok1 || !ok2) {
										m_model_errors << QStringLiteral("%1: Kraftprofil %2 „%3“ ist keine Zahlenliste").arg(who, QLatin1String(n), f.parameter(QLatin1String(n)));
										out->clear();
										return;
									}
									*out << qMakePair(x, F);
								}
								std::sort(out->begin(), out->end(), [](const QPair<double, double> &a, const QPair<double, double> &b) { return a.first < b.first; });
							};
							profile("F_var_ex", &c->load_ex);
							profile("F_var_re", &c->load_re);
							if (c->load_re.isEmpty()) c->load_re = c->load_ex;
						}
					}
					// R10 gripper: substitute model, a single-acting cylinder with return spring; the jaw
					// force is the piston force (signal F:)
					if (fn == QLatin1String("actuator.gripper") && c->f_spring <= 0) c->f_spring = 0.15 * c->a_k * 6e5;
					m_cylinders << c;
					m_end_position.insert(c->label, QString());
				} else if (model == QLatin1String("pressure_switch")) {
					auto *s = new PressureSwitch;
					s->e = e;
					s->label = label(e);
					for (Terminal *t : e->terminals())
						if (node(t) >= 0) { s->node = node(t); break; }
					const QString who = s->label.isEmpty() ? e->name() : s->label;
					s->p_on = paramIn(f, QStringLiteral("p0"), 0.4e6, QStringLiteral("Pa"), &m_model_errors, who);
					const double hyst = std::max(paramIn(f, QStringLiteral("p_hyst"), 0.05e6, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative), 1e3);
					// a negative switching point is a vacuum switch (R10): on below it, off above it + hysteresis
					s->vacuum = s->p_on < 0;
					s->p_off = s->vacuum ? s->p_on + hyst : s->p_on - hyst;
					m_pressure_switches << s;
				} else if (flow_qnn.contains(fn) || model == QLatin1String("passthrough")) {
					// passthrough (filter, service unit, cooler …): a resistance between the first two ports
					auto *fv = new FlowValve;
					fv->e = e;
					const bool pass = model == QLatin1String("passthrough");
					fv->function = pass ? QStringLiteral("flow_control_valve.throttle_valve") : fn;
					for (Terminal *t : e->terminals()) fv->ports << t->name();
					fv->free_from = f.parameter(QStringLiteral("free_from"));
					fv->free_to = f.parameter(QStringLiteral("free_to"));
					fv->hydraulic = hyd;
					const QString who = label(e).isEmpty() ? e->name() : label(e);
					fv->c = conductanceFromQnn(paramIn(f, QStringLiteral("qNN"), pass ? 1000.0 : flow_qnn.value(fn), QStringLiteral("l/min"), &m_model_errors, who, Range::Positive));
					fv->c_free = conductanceFromQnn(150);
					fv->rt = paramNative(f, QStringLiteral("rT"), pass ? 0.01 : fn.contains(QLatin1String("flow_control")) ? 1.0 : 0.1,
										 QStringLiteral("bar.min2/l2"), &m_model_errors, who, Range::Positive);
					// R05: a clogged filter or treatment part (FORMELN.md L05): FluidSIM fault model "Bauteil verstopft"
					// with a given restricted normal flow (default 10 l/min); oil: rT_clogged (default 1000·rT)
					if (pass && f.parameter(QStringLiteral("clogged")).trimmed() == QLatin1String("1")) {
						fv->c = conductanceFromQnn(paramIn(f, QStringLiteral("qNN_clogged"), 10.0, QStringLiteral("l/min"), &m_model_errors, who, Range::Positive));
						fv->rt = paramNative(f, QStringLiteral("rT_clogged"), 1000.0 * fv->rt, QStringLiteral("bar.min2/l2"), &m_model_errors, who, Range::Positive);
					}
					fv->p0 = paramIn(f, QStringLiteral("p0"), 0.0, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative);
					fv->dp_end = std::max(paramIn(f, QStringLiteral("dpEnd"), 0.05e6, QStringLiteral("Pa"), &m_model_errors, who, Range::Positive), 0.1e5);
					fv->t_delay = paramIn(f, QStringLiteral("t_delay"), 1.0, QStringLiteral("s"), &m_model_errors, who, Range::NonNegative);
					if (fn == QLatin1String("pressure_valve.pressure_sequence_valve")) {
						// R09 (FORMELN.md V13): FluidSIM pneumatic sequence valve, switching pressure p0 at 12 (default 3 bar)
						fv->p0 = paramIn(f, QStringLiteral("p0"), 3e5, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative);
						fv->dp_end = std::max(paramIn(f, QStringLiteral("dpEnd"), 0.1e5, QStringLiteral("Pa"), &m_model_errors, who, Range::Positive), 0.1e5);
					}
					if (fn == QLatin1String("pressure_valve.shock_and_anti_cavitation_valve")) {
						// R09 (FORMELN.md V12): two reliefs A → T and B → T, check valves T → A and T → B (FluidSIM defaults)
						fv->p0 = paramIn(f, QStringLiteral("p0_prvA"), 3e6, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative);
						fv->dp_end = std::max(paramIn(f, QStringLiteral("dpEndA"), 3.8e6, QStringLiteral("Pa"), &m_model_errors, who, Range::Positive), 0.1e5);
						fv->rt = paramNative(f, QStringLiteral("rT_prvA"), 0.08, QStringLiteral("bar.min2/l2"), &m_model_errors, who, Range::Positive);
						fv->p0_b = paramIn(f, QStringLiteral("p0_prvB"), 3e6, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative);
						fv->dp_end_b = std::max(paramIn(f, QStringLiteral("dpEndB"), 3.8e6, QStringLiteral("Pa"), &m_model_errors, who, Range::Positive), 0.1e5);
						fv->rt_b = paramNative(f, QStringLiteral("rT_prvB"), 0.08, QStringLiteral("bar.min2/l2"), &m_model_errors, who, Range::Positive);
						fv->rt_free = paramNative(f, QStringLiteral("rT_cv"), 0.06, QStringLiteral("bar.min2/l2"), &m_model_errors, who, Range::Positive);
					}
					if (fn == QLatin1String("proportional.pressure_relief_valve")) {
						// R06 (FORMELN.md V09): opening pressure p_start at 0 V … p_end at 10 V, FluidSIM default 0 … 10 MPa
						fv->p_lo = paramIn(f, QStringLiteral("p_start"), 0.0, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative);
						fv->p_hi = paramIn(f, QStringLiteral("p_end"), 10e6, QStringLiteral("Pa"), &m_model_errors, who, Range::NonNegative);
						fv->dp_end = std::max(paramIn(f, QStringLiteral("dpEnd"), 2e6, QStringLiteral("Pa"), &m_model_errors, who, Range::Positive), 0.1e5);
						fv->u = paramIn(f, QStringLiteral("u"), 0.0, QStringLiteral("V"), &m_model_errors, who);
					}
					if (isFlowSensor(fn))
						fv->rt = paramNative(f, QStringLiteral("rT"), 0.001, QStringLiteral("bar.min2/l2"), &m_model_errors, who, Range::Positive);
					if (fn == QLatin1String("pressure_valve.counterbalance_valve")) {
						// R09 (FORMELN.md V07): FluidSIM names the resistances rT_cb (valve) and rT_cv (check valve)
						fv->rt = paramNative(f, QStringLiteral("rT_cb"), fv->rt, QStringLiteral("bar.min2/l2"), &m_model_errors, who, Range::Positive);
						fv->rt_free = paramNative(f, QStringLiteral("rT_cv"), 0.1, QStringLiteral("bar.min2/l2"), &m_model_errors, who, Range::Positive);
						fv->ratio = paramNative(f, QStringLiteral("ratio"), 3.0, QString(), &m_model_errors, who, Range::NonNegative);
						fv->gleak = paramIn(f, QStringLiteral("GLeak"), 0.0, QStringLiteral("m3/(s.Pa)"), &m_model_errors, who, Range::NonNegative);
					}
					m_flow_valves << fv;
				}
				continue;
			}

			// GRAFCET charts are run by Grafcet, not as electric parts
			if (Grafcet::isGrafcet(e)) continue;

			// electric parts
			const QString path = e->location().toString().toLower();
			const QString name = e->name().toLower();
			QList<int> n;
			for (Terminal *t : e->terminals()) n << node(t);
			while (n.size() < 2) n << -1;
			const bool nc_name = name.contains(QStringLiteral("öffner")) || name.contains(QLatin1String(" nc"))
				|| name.contains(QLatin1String("(nc)")) || path.endsWith(QLatin1String("_nf.elmt"))
				|| path.endsWith(QLatin1String("_nc.elmt"));
			// an explicit role (written by the FluidSIM import) comes before guessing from the symbol
			const QString role = info(e, "sim_role");
			if (!role.isEmpty()) {
				if (role == QLatin1String("supply+") || role == QLatin1String("supply-")) {
					QList<int> all;
					for (int k : n) if (k >= 0) all << k;
					// R06: the potential of the supply (FluidSIM "pot"), default 24 V and 0 V
					bool ok = false;
					const double v = info(e, "sim_volt").replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok);
					for (int k : all) m_source_volt.insert(k, ok ? v : (role == QLatin1String("supply+") ? 24.0 : 0.0));
					if (role == QLatin1String("supply+")) m_sources << qMakePair(all, QList<int>{});
					else m_sources << qMakePair(QList<int>{}, all);
				} else if (role == QLatin1String("voltage")) {
					QList<int> plus, minus;
					for (Terminal *t : e->terminals()) {
						const QString tn = t->name().toLower();
						(tn == QLatin1String("p") || tn == QLatin1String("+") || tn == QLatin1String("l+") ? plus : minus) << node(t);
					}
					m_sources << qMakePair(plus, minus);
				} else if (role.startsWith(QLatin1String("button")) || role.startsWith(QLatin1String("switch"))
						   || role.startsWith(QLatin1String("limit"))) {
					auto *c = new Contact;
					c->e = e;
					c->kind = role.startsWith(QLatin1String("limit")) ? QStringLiteral("limit") : QStringLiteral("button");
					c->latching = role.startsWith(QLatin1String("switch"));
					c->nc = role.endsWith(QLatin1String("_nc"));
					c->n1 = n.at(0); c->n2 = n.at(1);
					c->trigger = info(e, "function");
					m_contacts << c;
				} else if (role == QLatin1String("solenoid")) {
					auto *c = new Coil;
					c->e = e;
					c->label = label(e);
					c->n1 = n.at(0); c->n2 = n.at(1);
					const QString s = info(e, "valve_side");
					c->side = s == QLatin1String("left") ? 0 : s == QLatin1String("right") ? 1 : -1;
					m_coils << c;
				} else if (role == QLatin1String("analog")) {
					// R06: analog block; ports by name, characteristic values "KP=1;KI=0 1/s", links "A=-QM1/Y1"
					auto *a = new Analog;
					a->e = e;
					a->label = label(e).isEmpty() ? e->name() : label(e);
					a->block = info(e, "sim_block").trimmed();
					for (Terminal *t : e->terminals())
						if (node(t) >= 0) a->port.insert(t->name().trimmed().toUpper(), node(t));
					for (const QString &item : info(e, "sim_params").split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
						const int eq = static_cast<int>(item.indexOf(QLatin1Char('=')));
						if (eq <= 0) continue;
						const QString key = item.left(eq).trimmed(), rest = item.mid(eq + 1).trimmed();
						if (rest.startsWith(QLatin1Char('{'))) {   // a list ("{0,10}"): kept as text
							a->par.insert(key, qMakePair(std::nan(""), rest));
							continue;
						}
						const QStringList vu = rest.split(QLatin1Char(' '), Qt::SkipEmptyParts);
						const QString num = vu.value(0);
						bool ok = false;
						double v = QString(num).replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok);
						if (!ok) v = num.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0 ? 1.0
								   : num.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0 ? 0.0 : std::nan("");
						if (!std::isfinite(v))
							m_model_errors << QStringLiteral("%1: Kennwert %2 = „%3“ ist keine Zahl").arg(a->label, key, num);
						a->par.insert(key, qMakePair(v, vu.mid(1).join(QLatin1Char(' '))));
					}
					for (const QString &item : info(e, "sim_link").split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
						const int eq = static_cast<int>(item.indexOf(QLatin1Char('=')));
						if (eq > 0) a->links << qMakePair(item.left(eq).trimmed().toUpper(), item.mid(eq + 1).trimmed());
					}
					m_analog << a;
				} else if (role == QLatin1String("grafcet_io")) {
					auto *io = new GrafcetIo;
					io->e = e;
					QHash<QString, QString> names;
					for (const QString &line : info(e, "sim_io").split(QRegularExpression(QStringLiteral("[;\\n]")), Qt::SkipEmptyParts)) {
						const int eq = static_cast<int>(line.indexOf(QLatin1Char('=')));
						if (eq > 0) names.insert(line.left(eq).trimmed().toUpper(), line.mid(eq + 1).trimmed());
					}
					for (Terminal *t : e->terminals()) {
						const QString tn = t->name().trimmed().toUpper();
						const QString var = names.value(tn);
						if (var.isEmpty() || node(t) < 0) continue;
						const QString v = var.startsWith(QLatin1Char('-')) ? Grafcet::labelVariable(var) : PlcProgram::normalized(var);
						if (tn.startsWith(QLatin1Char('I')) || tn.startsWith(QLatin1Char('E'))) io->inputs << qMakePair(node(t), v);
						else { io->outputs << qMakePair(node(t), v); io->value << false; }
					}
					m_grafcet_io << io;
				}
				continue;
			}
			if (data.m_type == ElementData::Master && data.m_master_type == ElementData::PLC) {
				// a PLC (module): its program is the element information "sim_program"
				const QString source = info(e, "sim_program");
				if (!source.trimmed().isEmpty()) {
					auto *plc = new Plc;
					plc->e = e;
					plc->label = label(e);
					plc->program = std::make_shared<PlcProgram>();
					QString error;
					if (!plc->program->compile(source, &error))
						m_program_errors << QStringLiteral("%1: %2").arg(plc->label, error);
					m_plcs << plc;
				}
				// its supply terminals feed the channels
				for (Terminal *t : e->terminals()) {
					const QString tn = t->name().toUpper();
					if (tn == QLatin1String("L+") || tn == QLatin1String("24V") || tn == QLatin1String("VS"))
						m_sources << qMakePair(QList<int>{node(t)}, QList<int>{});
					else if (tn == QLatin1String("M") || tn == QLatin1String("0V") || tn == QLatin1String("L-"))
						m_sources << qMakePair(QList<int>{}, QList<int>{node(t)});
				}
				continue;
			}
			if (data.m_type == ElementData::Slave && data.m_slave_type == ElementData::PLCSlave) {
				// a PLC channel: its IO entry in the linked module gives address and direction
				auto *ch = new PlcChannel;
				ch->e = e;
				const auto linked = e->linkedElements();
				Element *master = linked.isEmpty() ? nullptr : linked.first();
				// the link keeps the IO index on the module; the channel also carries address and type
				int io = master ? master->groupIndexForElement(e) : -1;
				if (master && io < 0) io = e->groupIndexForElement(master);
				const QString address = info(e, "plc_address");
				const QString io_type = info(e, "plc_type").toLower();
				if (!address.isEmpty()) {
					ch->names << PlcProgram::normalized(address);
					ch->output = io_type.contains(QLatin1String("output")) || io_type.contains(QLatin1String("ausgang"));
				} else if (master && io >= 0 && io < master->elementData().plcMasterData().ios.size()) {
					const ElementData::PlcIO entry = master->elementData().plcMasterData().ios.at(io);
					ch->output = entry.type == ElementData::SortieDigitale || entry.type == ElementData::SortieAnalogique
								 || entry.type == ElementData::SortieUniverselle;
					if (!entry.address.isEmpty()) ch->names << PlcProgram::normalized(entry.address);
					if (!entry.functionText.isEmpty())
						ch->names << PlcProgram::normalized(QString(entry.functionText).replace(QLatin1Char(' '), QLatin1Char('_')));
				}
				// a channel takes the label of its module, so the label only names it when nothing else does
				if (ch->names.isEmpty() && !label(e).isEmpty()) ch->names << PlcProgram::normalized(label(e));
				for (Terminal *t : e->terminals()) {
					const QString tn = t->name().toUpper();
					const int k = node(t);
					if (k < 0) continue;
					if (tn == QLatin1String("VS") || tn == QLatin1String("L+") || tn == QLatin1String("24V"))
						ch->plus << k;
					else if (tn == QLatin1String("M") || tn == QLatin1String("0V") || tn == QLatin1String("L-"))
						ch->minus << k;
					else if (tn != QLatin1String("AUX"))
						ch->signal << k;
				}
				m_plc_channels << ch;
				continue;
			}
			if (path.contains(QLatin1String("comptage")) || name.contains(QStringLiteral("zähler"))) {
				const QString lbl = label(e);
				if (name.contains(QStringLiteral("betätigt")) || path.contains(QLatin1String("act_comptage"))) {
					auto *c = new Contact;
					c->e = e;
					c->kind = QStringLiteral("counter");
					c->nc = nc_name;
					c->n1 = n.at(0); c->n2 = n.at(1);
					c->trigger = lbl;
					m_contacts << c;
				} else {
					auto *z = new Counter;
					z->e = e;
					z->reset_coil = lbl.endsWith(QLatin1String("/R"));
					z->label = z->reset_coil ? lbl.chopped(2) : lbl;
					z->n1 = n.at(0); z->n2 = n.at(1);
					z->nodes = n;
					z->preset = std::max(1, int(timeFromText(info(e, "function"), 1.0)));
					m_counters << z;
				}
				continue;
			}
			if (data.m_type == ElementData::Master && data.m_master_type == ElementData::Coil) {
				auto *r = new Relay;
				r->e = e;
				r->label = label(e);
				r->n1 = n.at(0); r->n2 = n.at(1);
				const QString text = info(e, "function").isEmpty() ? info(e, "comment") : info(e, "function");
				r->delay = timeFromText(text);
				if (name.contains(QLatin1String("ansprechverz")) || path.contains(QLatin1String("tempo_travail")))
					r->mode = QStringLiteral("on");
				else if (name.contains(QLatin1String("abfallverz")) || name.contains(QStringLiteral("rückfallverz"))
						 || path.contains(QLatin1String("tempo_repos")))
					r->mode = QStringLiteral("off");
				m_relays << r;
			} else if (data.m_type == ElementData::Slave && data.m_slave_state == ElementData::SW) {
				// changeover contact: common (…1), break (…2) and make (…4) side, as an NC and an NO contact
				int common = -1, brk = -1, make = -1;
				QList<int> rest;
				for (Terminal *t : e->terminals()) {
					const QString tn = t->name().trimmed();
					static const QRegularExpression digits(QStringLiteral("^\\d+$"));
					if (!digits.match(tn).hasMatch()) { if (node(t) >= 0 && tn.isEmpty()) rest << node(t); continue; }
					if (tn.endsWith(QLatin1Char('1')) && common < 0) common = node(t);
					else if (tn.endsWith(QLatin1Char('2')) && brk < 0) brk = node(t);
					else if (tn.endsWith(QLatin1Char('4')) && make < 0) make = node(t);
					else rest << node(t);
				}
				if (common < 0 && !rest.isEmpty()) common = rest.takeFirst();
				if (brk < 0 && !rest.isEmpty()) brk = rest.takeFirst();
				if (make < 0 && !rest.isEmpty()) make = rest.takeFirst();
				for (const bool nc : {true, false}) {
					auto *c = new Contact;
					c->e = e;
					c->kind = QStringLiteral("relay");
					c->nc = nc;
					c->n1 = common; c->n2 = nc ? brk : make;
					if (data.m_slave_type == ElementData::DelayOn) c->mode = QStringLiteral("on");
					if (data.m_slave_type == ElementData::DelayOff) c->mode = QStringLiteral("off");
					m_contacts << c;
				}
			} else if (data.m_type == ElementData::Slave
					   && (data.m_slave_state == ElementData::NO || data.m_slave_state == ElementData::NC)) {
				auto *c = new Contact;
				c->e = e;
				c->kind = QStringLiteral("relay");
				c->nc = data.m_slave_state == ElementData::NC;
				c->n1 = n.at(0); c->n2 = n.at(1);
				if (data.m_slave_type == ElementData::DelayOn) c->mode = QStringLiteral("on");
				if (data.m_slave_type == ElementData::DelayOff) c->mode = QStringLiteral("off");
				m_contacts << c;
			} else if (path.contains(QLatin1String("voltage_current_sources")) || name.contains(QLatin1String("spannungsquelle"))
					   || name.contains(QLatin1String("voltage source"))) {
				QList<int> plus, minus;
				for (Terminal *t : e->terminals()) {
					const auto o = t->orientation();
					(o == Qet::North || o == Qet::West ? plus : minus) << node(t);
				}
				m_sources << qMakePair(plus, minus);
			} else if (path.contains(QLatin1String("push_buttons")) || name.contains(QLatin1String("taster"))
					   || name.contains(QLatin1String("push-button"))) {
				auto *c = new Contact;
				c->e = e;
				c->kind = QStringLiteral("button");
				c->nc = nc_name;
				c->n1 = n.at(0); c->n2 = n.at(1);
				m_contacts << c;
			} else if (path.contains(QLatin1String("limit_switches")) || name.contains(QLatin1String("endschalter"))
					   || path.contains(QLatin1String("pressostat")) || name.contains(QStringLiteral("druckschalter"))
					   || name.contains(QLatin1String("limitswitch")) || name.contains(QLatin1String("grenztaster"))) {
				auto *c = new Contact;
				c->e = e;
				c->kind = QStringLiteral("limit");
				c->nc = nc_name;
				c->n1 = n.at(0); c->n2 = n.at(1);
				c->trigger = info(e, "function");
				m_contacts << c;
			} else if (path.contains(QLatin1String("20_valves")) || name.contains(QLatin1String("magnetventil"))
					   || name.contains(QLatin1String("electrovanne")) || name.contains(QLatin1String("ventilspule"))) {
				auto *c = new Coil;
				c->e = e;
				c->label = label(e);
				c->n1 = n.at(0); c->n2 = n.at(1);
				m_coils << c;
			}
		}
	}
	// valve solenoids without the valve's label find it through the link
	for (Coil *c : m_coils) {
		bool by_label = false;
		const int slash = static_cast<int>(c->label.indexOf(QLatin1Char('/')));
		const QString base = slash >= 0 ? c->label.left(slash) : c->label;
		for (Valve *v : m_valves) by_label = by_label || (!base.isEmpty() && v->label == base);
		if (by_label) continue;
		for (Element *other : c->e->linkedElements())
			for (Valve *v : m_valves)
				if (v->e == other) c->valve = v;
	}
	// relay contacts find their relay through the link
	for (Contact *c : m_contacts) {
		if (c->kind != QLatin1String("relay")) continue;
		const auto linked = c->e->linkedElements();
		for (Relay *r : m_relays)
			if (!linked.isEmpty() && linked.first() == r->e) c->relay = r;
	}
	for (auto it = m_fixed.constBegin(); it != m_fixed.constEnd(); ++it)
		m_p[it.key()] = it.value();
	m_vessel_c = conductanceFromQnn(3000);
	m_silencer_c = conductanceFromQnn(300);
	for (const Cylinder *z : std::as_const(m_cylinders)) {
		const QString who = z->label.isEmpty() ? z->e->name() : z->label;
		if (z->rotary) {
			if (!(z->stroke > 0) || !(z->m > 0) || !(z->a_k > 0) || !std::isfinite(z->x) || z->x < 0 || z->x > z->stroke
				|| !std::isfinite(z->f_load) || !(z->f_static >= 0))
				m_model_errors << QStringLiteral("%1: ungültige Schwenkantriebsdaten (Winkel %2°, Trägheit %3 kg·m², "
					"Moment bei 6 bar %4 N·m, Startwinkel %5°)").arg(who).arg(z->stroke * 180 / M_PI).arg(z->m)
					.arg(z->a_k * 6e5).arg(z->x * 180 / M_PI);
			continue;
		}
		if (!(z->stroke > 0) || !(z->m > 0) || !(z->a_k > 0) || !(z->a_r > 0) || !std::isfinite(z->x)
			|| z->x < 0 || z->x > z->stroke || !std::isfinite(z->f_load) || !(z->f_static >= 0))
			m_model_errors << QStringLiteral("%1: ungültige Zylinderdaten (Hub %2 mm, Masse %3 kg, Kolbenfläche %4 cm², "
				"Ringfläche %5 cm², Startweg %6 mm)").arg(who).arg(z->stroke * 1000).arg(z->m)
				.arg(z->a_k * 1e4).arg(z->a_r * 1e4).arg(z->x * 1000);
	}
	for (const Accumulator *a : std::as_const(m_accumulators))
		if (!(a->v0 > 0) || !(a->p_pre > 0) || !(a->n > 0))
			m_model_errors << QStringLiteral("%1: ungültige Speicherdaten (Volumen %2 l, Vorfülldruck %3 bar, k %4)")
				.arg(a->label).arg(a->v0 * 1000).arg((a->p_pre - P_ATM) / 1e5).arg(a->n);
	for (const Pump *pu : std::as_const(m_pumps))
		if (!(pu->q >= 0) || !(pu->p_max > P_ATM))
			m_model_errors << QStringLiteral("%1: ungültige Pumpendaten").arg(label(pu->e).isEmpty() ? pu->e->name() : label(pu->e));
	if (!m_model_errors.isEmpty()) {
		m_failed = true;
		m_failure = m_model_errors.join(QStringLiteral("; "));
	}
	linkAnalog(diagrams);
	if (!m_model_errors.isEmpty() && !m_failed) {
		m_failed = true;
		m_failure = m_model_errors.join(QStringLiteral("; "));
	}
	m_grafcet = std::make_unique<Grafcet>(diagrams, [this](Terminal *t) { return node(t); });
	if (m_grafcet->isEmpty()) m_grafcet.reset();
	else m_program_errors << m_grafcet->errors();
	electrics();
}

// ------------------------------------------------------------------------- electrics
bool SimulationModel::contactClosed(const Contact &c) const
{
	bool actuated = false;
	if (c.kind == QLatin1String("button")) {
		actuated = c.latching ? m_latched.contains(label(c.e)) : m_pressed.contains(label(c.e));
	} else if (c.kind == QLatin1String("limit")) {
		const QStringList parts = c.trigger.split(QLatin1Char(' '), Qt::SkipEmptyParts);
		actuated = parts.size() == 2 && m_end_position.value(parts.at(0)) == parts.at(1);
		const int dots = parts.size() == 2 ? static_cast<int>(parts.at(1).indexOf(QLatin1String(".."))) : -1;
		if (dots > 0) {
			// a stroke range in mm (FluidSIM distance rule): actuated while the piston is inside
			bool ok1 = false, ok2 = false;
			const double lo = parts.at(1).left(dots).toDouble(&ok1) / 1000.0;
			const double hi = parts.at(1).mid(dots + 2).toDouble(&ok2) / 1000.0;
			for (const Cylinder *z : m_cylinders)
				if (ok1 && ok2 && z->label == parts.at(0)) actuated = z->markPos() >= lo - 1e-9 && z->markPos() <= hi + 1e-9;
		}
		if (parts.size() == 1)          // function text "-BP1": contact of that pressure switch
			for (const PressureSwitch *s : m_pressure_switches)
				if (s->label == parts.at(0)) actuated = s->on;
	} else if (c.kind == QLatin1String("counter")) {
		for (const Counter *z : m_counters)
			if (!z->reset_coil && z->label == c.trigger) actuated = z->count >= z->preset;
	} else if (c.relay) {
		const Relay *r = c.relay;
		const QString mode = c.mode.isEmpty() ? r->mode : c.mode;
		if (mode == QLatin1String("on"))
			actuated = r->on && r->on_since >= 0 && m_time - r->on_since >= r->delay;
		else if (mode == QLatin1String("off"))
			actuated = r->on || (r->off_since >= 0 && m_time - r->off_since < r->delay);
		else
			actuated = r->on;
	}
	return actuated != c.nc;
}

void SimulationModel::electrics()
{
	if (m_sources.isEmpty() && m_plc_channels.isEmpty() && !m_grafcet) return;
	QHash<int, int> parent;
	std::function<int(int)> root = [&](int a) {
		while (parent.value(a, a) != a) {
			const int up = parent.value(a, a);
			parent[a] = parent.value(up, up);
			a = parent[a];
		}
		return a;
	};
	for (Contact *c : m_contacts)
		if (c->n1 >= 0 && c->n2 >= 0 && contactClosed(*c)) {
			const int ra = root(c->n1), rb = root(c->n2);
			if (ra != rb) parent.insert(ra, rb);
		}
	QSet<int> plus, minus;
	for (const auto &s : m_sources) {
		for (int x : s.first) if (x >= 0) plus << root(x);
		for (int x : s.second) if (x >= 0) minus << root(x);
	}
	// R06: a comparator that is on puts its supply onto its output S (state of the last step)
	for (const Analog *a : std::as_const(m_analog))
		if (a->on && a->has(QStringLiteral("S"))) plus << root(a->port.value(QStringLiteral("S")));
	{
		QHash<int, double> pot;
		for (auto it = m_source_volt.constBegin(); it != m_source_volt.constEnd(); ++it) pot.insert(root(it.key()), it.value());
		m_net_volt.clear();
		for (int x : m_electric) {
			const int r = root(x);
			m_net_volt.insert(x, pot.contains(r) ? pot.value(r) : plus.contains(r) ? 24.0 : 0.0);
		}
	}
	for (const GrafcetIo *io : std::as_const(m_grafcet_io))
		for (int i = 0; i < io->outputs.size(); ++i)
			if (io->value.at(i)) plus << root(io->outputs.at(i).first);
	for (const PlcChannel *ch : m_plc_channels) {
		for (int x : ch->plus) plus << root(x);
		for (int x : ch->minus) minus << root(x);
		if (ch->output && ch->value)
			for (int x : ch->signal) plus << root(x);
	}
	// PLC: read the inputs, run the programs every 10 ms, write the outputs
	for (PlcChannel *ch : m_plc_channels) {
		if (ch->output) continue;
		bool on = false;
		for (int x : ch->signal) on = on || plus.contains(root(x));
		ch->value = on;
		for (const QString &nm : ch->names) m_plc_vars.insert(nm, on ? 1.0 : 0.0);
	}
	if ((!m_plcs.isEmpty() || m_grafcet) && m_time >= m_next_plc_cycle - 1e-12) {
		m_next_plc_cycle = m_time + 0.010;
		for (const GrafcetIo *io : std::as_const(m_grafcet_io))
			for (const auto &in : io->inputs) m_plc_vars.insert(in.second, plus.contains(root(in.first)) ? 1.0 : 0.0);
		if (m_grafcet) {
			// labels of push buttons and limit switches: actuated or not
			for (Contact *c : m_contacts)
				if ((c->kind == QLatin1String("button") || c->kind == QLatin1String("limit")) && !label(c->e).isEmpty())
					m_plc_vars.insert(Grafcet::labelVariable(label(c->e)), (contactClosed(*c) != c->nc) ? 1.0 : 0.0);
			for (const PressureSwitch *s : std::as_const(m_pressure_switches))
				if (!s->label.isEmpty()) m_plc_vars.insert(Grafcet::labelVariable(s->label), s->on ? 1.0 : 0.0);
			QStringList texts;
			m_grafcet->cycle(m_time, m_plc_vars, &texts);
			for (const QString &text : std::as_const(texts)) m_messages << Message{m_time, text};
			// valves named in actions: -QM1 (its solenoid), -QM1/14 (left), -QM1/12 (right)
			for (Valve *v : m_valves) {
				const QString base = Grafcet::labelVariable(v->label);
				const bool any = m_plc_vars.value(base, 0.0) != 0.0;
				const bool l = m_plc_vars.value(base + QStringLiteral("__14"), 0.0) != 0.0;
				const bool r = m_plc_vars.value(base + QStringLiteral("__12"), 0.0) != 0.0;
				const int single = v->side[1].types.contains(QLatin1String("solenoid"))
								   && !v->side[0].types.contains(QLatin1String("solenoid")) ? 1 : 0;
				v->forced[0] = l || (any && single == 0);
				v->forced[1] = r || (any && single == 1);
			}
			// a valve solenoid named in an action (-MB1) actuates its side of the linked valve
			for (const Coil *c : std::as_const(m_coils)) {
				if (!c->valve || c->label.isEmpty()) continue;
				if (m_plc_vars.value(Grafcet::labelVariable(c->label), 0.0) == 0.0) continue;
				Valve *v = c->valve;
				const int single = v->side[1].types.contains(QLatin1String("solenoid"))
								   && !v->side[0].types.contains(QLatin1String("solenoid")) ? 1 : 0;
				v->forced[c->side >= 0 ? c->side : single] = true;
			}
			for (GrafcetIo *io : std::as_const(m_grafcet_io))
				for (int i = 0; i < io->outputs.size(); ++i)
					io->value[i] = m_plc_vars.value(io->outputs.at(i).second, 0.0) != 0.0;
		}
		for (Plc *plc : m_plcs) plc->program->cycle(m_time, m_plc_vars);
		for (PlcChannel *ch : m_plc_channels) {
			if (!ch->output) continue;
			bool on = false;
			for (const QString &nm : ch->names) on = on || m_plc_vars.value(nm, 0.0) != 0.0;
			if (on != ch->value) {
				ch->value = on;
				m_messages << Message{m_time, QStringLiteral("SPS-Ausgang %1 %2").arg(ch->names.value(0),
					on ? QStringLiteral("an") : QStringLiteral("aus"))};
			}
		}
	}
	m_plus_nets.clear();
	for (int x : m_electric)
		if (plus.contains(root(x))) m_plus_nets << x;
	auto powered = [&](int a, int b) {
		return a >= 0 && b >= 0 && ((plus.contains(root(a)) && minus.contains(root(b)))
									|| (minus.contains(root(a)) && plus.contains(root(b))));
	};
	for (Relay *r : m_relays) {
		const bool want = powered(r->n1, r->n2);
		if (want != r->target) {
			r->target = want;
			r->switch_at = m_time + (want ? T_PULL_IN : T_DROP_OUT);
		}
		if (r->switch_at >= 0 && m_time >= r->switch_at) {
			if (r->on != r->target) {
				(r->target ? r->on_since : r->off_since) = m_time;
				m_messages << Message{m_time, QStringLiteral("Relais %1 %2").arg(r->label,
					r->target ? QStringLiteral("zieht an") : QStringLiteral("fällt ab"))};
			}
			r->on = r->target;
			r->switch_at = -1;
		}
	}
	for (Counter *z : m_counters) {
		// counts when any of its terminals has plus and another one minus
		bool has_plus = false, has_minus = false;
		for (int x : z->nodes) {
			if (x < 0) continue;
			has_plus = has_plus || plus.contains(root(x));
			has_minus = has_minus || minus.contains(root(x));
		}
		const bool on = has_plus && has_minus;
		if (z->reset_coil) {
			if (on)
				for (Counter *o : m_counters)
					if (!o->reset_coil && o->label == z->label) o->count = 0;
		} else if (on && !z->last) {
			++z->count;
			m_messages << Message{m_time, QStringLiteral("Zähler %1: %2 von %3").arg(z->label).arg(z->count).arg(z->preset)};
		}
		z->last = on;
	}
	for (Coil *c : m_coils) {
		const bool on = powered(c->n1, c->n2);
		if (c->known && on != c->on)
			m_messages << Message{m_time, QStringLiteral("Spule %1 %2").arg(c->label,
				on ? QStringLiteral("an") : QStringLiteral("aus"))};
		c->on = on;
		c->known = true;
		if (Valve *v = c->valve) {
			const int single = v->side[1].types.contains(QLatin1String("solenoid"))
							   && !v->side[0].types.contains(QLatin1String("solenoid")) ? 1 : 0;
			v->actuated[c->side >= 0 ? c->side : single] = on;
			continue;
		}
		const int slash = static_cast<int>(c->label.indexOf(QLatin1Char('/')));
		const QString valve = slash >= 0 ? c->label.left(slash) : c->label;
		const QString side = slash >= 0 ? c->label.mid(slash + 1) : QString();
		for (Valve *v : m_valves) {
			if (v->label != valve) continue;
			if (side == QLatin1String("14") || side == QLatin1String("Y1") || side == QLatin1String("1"))
				v->actuated[0] = on;
			else if (side == QLatin1String("12") || side == QLatin1String("Y2") || side == QLatin1String("2"))
				v->actuated[1] = on;
			else
				v->actuated[v->side[1].types.contains(QLatin1String("solenoid"))
							&& !v->side[0].types.contains(QLatin1String("solenoid")) ? 1 : 0] = on;
		}
	}
}


// ------------------------------------------------------------------------- analog signals (R06)
double SimulationModel::volt(int k) const
{
	if (k < 0) return 0.0;
	const auto it = m_volt.constFind(k);
	if (it != m_volt.constEnd()) return it.value();
	return m_net_volt.value(k, 0.0);
}

void SimulationModel::linkAnalog(const QList<Diagram *> &diagrams)
{
	if (m_analog.isEmpty()) return;
	static const QStringList blocks{
		QStringLiteral("Source.WaveformGenerator"), QStringLiteral("Source.WaveformGenerator02"),
		QStringLiteral("Source.SetPointGenerator"), QStringLiteral("Controller.ControllerPID"),
		QStringLiteral("Controller.ControllerS"), QStringLiteral("Controller.ControllerPT2"),
		QStringLiteral("Controller.ControllerIntegrator"), QStringLiteral("Controller.DifferentialAmplifier"),
		QStringLiteral("Controller.ComparatorCard"), QStringLiteral("Controller.PropAmplifier1Ch"),
		QStringLiteral("Controller.PropAmplifier2Ch"), QStringLiteral("Relay.ValveContSolenoid"),
		QStringLiteral("Sensor.PressureSensorHyd"), QStringLiteral("Sensor.PressureSensorPneu"),
		QStringLiteral("Sensor.FlowSensorHyd"), QStringLiteral("Sensor.PositionSensor"), QStringLiteral("Sensor.Voltmeter")};
	// fluid parts the sensors read, by label
	QHash<QString, Element *> fluid_by_label;
	for (Diagram *d : diagrams)
		for (Element *e : sortedElements(d))
			if (!e->elementData().m_fluid.isNull() && !label(e).isEmpty()) fluid_by_label.insert(label(e), e);
	const QString V = QStringLiteral("V"), s = QStringLiteral("s");
	for (Analog *a : std::as_const(m_analog)) {
		const QString &b = a->block;
		if (!blocks.contains(b)) {
			m_model_errors << QStringLiteral("%1: Signalbaustein „%2“ hat kein Rechenmodell").arg(a->label, b.isEmpty() ? QStringLiteral("(keiner)") : b);
			continue;
		}
		auto link = [&](const QString &p) {
			for (const auto &l : std::as_const(a->links)) if (l.first == p) return l.second;
			return QString();
		};
		const bool unipolar = a->value(QStringLiteral("outputMode"), 0.0) > 0.5;   // FluidSIM: 0 → ±10 V, 1 → 0 … 10 V
		if (b.startsWith(QLatin1String("Source.WaveformGenerator"))) {
			auto &w = a->wave;
			w.form = qRound(a->value(QStringLiteral("waveForm"), 2.0));
			w.frequency = a->value(QStringLiteral("frequency"), 1.0, QStringLiteral("Hz"));
			w.amplitude = a->value(QStringLiteral("amplitude"), 5.0, V);
			w.offset = a->value(QStringLiteral("yOffset"), 0.0, V);
			w.vmin = a->value(QStringLiteral("vMin"), -30.0, V);
			w.vmax = a->value(QStringLiteral("vMax"), 30.0, V);
			w.repeat = a->value(QStringLiteral("repeat"), 0.0) > 0.5;
			const auto list = [&](const char *n) {
				QList<double> out;
				QString t = a->par.value(QLatin1String(n)).second;
				t.remove(QLatin1Char('{')).remove(QLatin1Char('}'));
				for (const QString &x : t.split(QLatin1Char(','), Qt::SkipEmptyParts)) out << x.trimmed().toDouble();
				return out;
			};
			const QList<double> px = list("profile_x"), py = list("profile_y");
			for (int i = 0; i < std::min(px.size(), py.size()); ++i) w.profile.emplace_back(px.at(i), py.at(i));
			a->outputs << (a->has(QStringLiteral("S")) ? QStringLiteral("S") : QStringLiteral("P"));
			if (a->has(QStringLiteral("N"))) a->inputs << QStringLiteral("N");
		} else if (b == QLatin1String("Source.SetPointGenerator")) {
			auto &c = a->card;
			c.mode = qRound(a->value(QStringLiteral("mode"), 1.0));
			c.count = qRound(a->value(QStringLiteral("numOfSetPoints"), 1.0));
			for (int i = 0; i < 8; ++i) c.w[i] = a->value(QStringLiteral("W%1").arg(i + 1), 0.0, V);
			for (int i = 0; i < 4; ++i) c.ramp[i] = a->value(QStringLiteral("R%1").arg(i + 1), 0.0);
			c.t_switch = a->value(QStringLiteral("switchingTime"), 1.0, s);
			a->vmin = a->value(QStringLiteral("vMin"), 20.0, V);
			a->inputs << QStringLiteral("I1") << QStringLiteral("I2") << QStringLiteral("I3");
			a->outputs << QStringLiteral("Q");
		} else if (b == QLatin1String("Controller.ControllerPID")) {
			auto &c = a->pid;
			c.kp = a->value(QStringLiteral("KP"), 1.0);
			c.ki = a->value(QStringLiteral("KI"), 0.0, QStringLiteral("1/s"));
			c.kd = a->value(QStringLiteral("KD"), 0.0, s);
			c.offset = a->value(QStringLiteral("yOffset"), 0.0, V);
			c.umin = unipolar ? 0.0 : -10.0;
			c.umax = 10.0;
			// FORMELN.md A04: derivative filter Td/10, tracking time 0.9·Ti (Modelica LimPID Nd = 10, Ni = 0.9)
			c.tau_d = c.kp > 0 && c.kd > 0 ? std::max(c.kd / c.kp / 10.0, 1e-4) : 1e-3;
			c.t_track = c.ki > 0 ? std::max(0.9 * (c.kp > 0 ? c.kp / c.ki : 1.0 / c.ki), 1e-4) : 0.1;
			a->vmin = a->value(QStringLiteral("vMin"), 21.6, V);
			a->inputs << QStringLiteral("W") << QStringLiteral("X");
			a->outputs << QStringLiteral("Y");
		} else if (b == QLatin1String("Controller.ControllerS")) {
			auto &c = a->sc;
			c.kx = a->value(QStringLiteral("Kx"), 1.0);
			c.kv = a->value(QStringLiteral("Kv"), 0.0, s);
			c.ka = a->value(QStringLiteral("Ka"), 0.0, QStringLiteral("s2"));
			c.gain = a->value(QStringLiteral("gain"), 1.0);
			c.offset = a->value(QStringLiteral("yOffset"), 0.0, V);
			c.umin = unipolar ? 0.0 : -10.0;
			a->vmin = a->value(QStringLiteral("vMin"), 21.6, V);
			a->inputs << QStringLiteral("W") << QStringLiteral("X");
			a->outputs << QStringLiteral("Y");
		} else if (b == QLatin1String("Controller.ControllerPT2") || b == QLatin1String("Controller.ControllerIntegrator")
				   || b == QLatin1String("Controller.DifferentialAmplifier")) {
			a->pt2.t = std::max(a->value(QStringLiteral("T"), 0.01, s), 1e-5);
			a->pt2.k = a->value(QStringLiteral("K"), 1.0);
			a->pt2.d = std::max(a->value(QStringLiteral("d"), 1.0), 1e-5);
			a->integ.ki = a->value(QStringLiteral("Ki"), 100.0, QStringLiteral("1/s"));
			a->efficiency = a->value(QStringLiteral("Efficiency"), 0.9);
			a->vmin = a->value(QStringLiteral("vMinSupplyA1"), 1.5, V);
			a->inputs << QStringLiteral("U") << QStringLiteral("A1") << QStringLiteral("A2");
			if (b.endsWith(QLatin1String("DifferentialAmplifier"))) a->inputs << QStringLiteral("X");
			a->outputs << QStringLiteral("Y");
		} else if (b == QLatin1String("Controller.ComparatorCard")) {
			a->vmin = a->value(QStringLiteral("vMin"), 13.0, V);
			a->inputs << QStringLiteral("U");
			a->outputs << QStringLiteral("S");
		} else if (b.startsWith(QLatin1String("Controller.PropAmplifier")) || b == QLatin1String("Relay.ValveContSolenoid")) {
			const bool coil = b == QLatin1String("Relay.ValveContSolenoid");
			a->two_channel = b.endsWith(QLatin1String("2Ch"));
			a->jump = a->value(QStringLiteral("jumpCurrentRel"), 0.0) / 100.0;
			a->vmin = a->value(QStringLiteral("vMin"), 20.0, V);
			a->inputs << (coil ? QStringLiteral("C") : QStringLiteral("U"));
			// links "A=-QM1/Y1" (amplifier outputs) or "C=-QM1/S" (solenoid): the valve and its connector
			for (const auto &l : std::as_const(a->links)) {
				const QString target = l.second.section(QLatin1Char('/'), 0, 0), conn = l.second.section(QLatin1Char('/'), 1).toUpper();
				// sign of the spool (s > 0: P→A, FORMELN.md A10): in FluidSIM a positive setpoint opens P→A. The
				// right solenoid Y2 of the 4/3 valve 01 pushes the spool left and brings the box P→A, B→T, Y1 the
				// crossed one; a positive voltage at S (valve 02) opens P→A. A two channel amplifier drives A for
				// positive and B for negative inputs, so B wired to Y1 gives the same sign as A wired to Y2
				const double sign = (conn == QLatin1String("Y1") ? -1.0 : 1.0) * (a->two_channel && l.first == QLatin1String("B") ? -1.0 : 1.0);
				bool found = false;
				for (PropValve *v : std::as_const(m_prop_valves))
					if (v->label == target) {
						bool seen = false;
						for (const auto &pv : std::as_const(a->valves)) seen = seen || pv.first == v;
						if (!seen) a->valves << qMakePair(v, sign);
						found = true;
					}
				for (FlowValve *fv : std::as_const(m_flow_valves))
					if (fv->function == QLatin1String("proportional.pressure_relief_valve") && label(fv->e) == target) {
						if (!a->reliefs.contains(fv)) a->reliefs << fv;
						found = true;
					}
				// a part with that label but without a model (MPYE …) is already reported by the catalog
				if (!found && !fluid_by_label.contains(target))
					m_model_errors << QStringLiteral("%1: gekoppeltes Stetigventil „%2“ nicht gefunden").arg(a->label, target);
			}
		} else if (b.startsWith(QLatin1String("Sensor.PressureSensor")) || b == QLatin1String("Sensor.FlowSensorHyd")) {
			const bool flow = b == QLatin1String("Sensor.FlowSensorHyd");
			QString target = link(QStringLiteral("P"));
			if (target.isEmpty()) target = a->label;
			Element *fe = fluid_by_label.value(target);
			const FluidData f = fe ? fe->elementData().m_fluid : FluidData();
			if (!fe || !(flow ? isFlowSensor(f.m_function) : f.m_function.startsWith(QLatin1String("proportional.pressure_sensor")))) {
				m_model_errors << QStringLiteral("%1: hydraulischer bzw. pneumatischer Teil „%2“ des Sensors fehlt").arg(a->label, target);
				continue;
			}
			const QString who = a->label;
			if (flow) {
				for (FlowValve *fv : std::as_const(m_flow_valves)) if (fv->e == fe) a->flow_sensor = fv;
				a->s0 = paramIn(f, QStringLiteral("q_start"), 0.0, QStringLiteral("m3/s"), &m_model_errors, who);
				a->s1 = paramIn(f, QStringLiteral("q_end"), 10.0 / 60000.0, QStringLiteral("m3/s"), &m_model_errors, who);
				a->signed_flow = paramNative(f, QStringLiteral("mode"), 1.0, QString(), &m_model_errors, who, Range::Any) > 1.5;
			} else {
				for (Terminal *t : fe->terminals())
					if (isFluid(t) && node(t) >= 0) { a->sensor_node = node(t); break; }
				const bool pneu = b.endsWith(QLatin1String("Pneu"));
				a->s0 = paramIn(f, QStringLiteral("p_start"), 0.0, QStringLiteral("Pa"), &m_model_errors, who);
				a->s1 = paramIn(f, QStringLiteral("p_end"), pneu ? 1e6 : 10e6, QStringLiteral("Pa"), &m_model_errors, who);
			}
			a->u0 = paramIn(f, QStringLiteral("U_start"), 0.0, V, &m_model_errors, who);
			a->u1 = paramIn(f, QStringLiteral("U_end"), 10.0, V, &m_model_errors, who);
			a->outputs << QStringLiteral("Y");
		} else if (b == QLatin1String("Sensor.PositionSensor")) {
			const QString target = link(QStringLiteral("X"));
			for (Cylinder *z : std::as_const(m_cylinders)) if (z->label == target) a->cylinder = z;
			if (!a->cylinder) {
				m_model_errors << QStringLiteral("%1: Zylinder „%2“ des Wegmesssystems nicht gefunden").arg(a->label, target);
				continue;
			}
			a->u0 = a->value(QStringLiteral("U_start"), 0.0, V);
			a->u1 = a->value(QStringLiteral("U_end"), 10.0, V);
			a->vmin = a->value(QStringLiteral("vMinAB"), 13.0, V);
			a->outputs << QStringLiteral("S");
		} else if (b == QLatin1String("Sensor.Voltmeter")) {
			a->inputs << QStringLiteral("P") << QStringLiteral("N");
		}
	}
	// two outputs on one net contradict each other
	QHash<int, QString> driver;
	for (const Analog *a : std::as_const(m_analog))
		for (const QString &o : a->outputs) {
			const int k = a->port.value(o, -1);
			if (k < 0) continue;
			if (driver.contains(k))
				m_model_errors << QStringLiteral("Netz an %1: zwei Analogausgänge treiben dasselbe Netz (%2 und %3)")
									  .arg(netName(k), driver.value(k), a->label);
			driver.insert(k, a->label);
		}
	// order of evaluation (FORMELN.md A00): a block after the blocks that drive its inputs; in a pure
	// algebraic loop the first block of the plan reads the value of the last step
	QList<Analog *> todo = m_analog, done;
	QSet<int> ready;
	while (!todo.isEmpty()) {
		int pick = -1;
		for (int i = 0; i < todo.size() && pick < 0; ++i) {
			bool free = true;
			for (const QString &in : std::as_const(todo.at(i)->inputs)) {
				const int k = todo.at(i)->port.value(in, -1);
				if (k >= 0 && driver.contains(k) && !ready.contains(k)) free = false;
			}
			if (free) pick = i;
		}
		if (pick < 0) pick = 0;
		Analog *a = todo.takeAt(pick);
		for (const QString &o : std::as_const(a->outputs)) ready << a->port.value(o, -1);
		done << a;
	}
	m_analog = done;
}

void SimulationModel::analog(double dt)
{
	if (m_analog.isEmpty()) return;
	for (Analog *a : m_analog) {
		const QString &b = a->block;
		auto V = [&](const char *p) { return volt(a->port.value(QLatin1String(p), -1)); };
		auto has = [&](const char *p) { return a->has(QLatin1String(p)); };
		const double supply = std::abs(V("SP") - V("SN"));
		const bool powered = supply >= a->vmin - 1e-9;
		double y = 0.0;
		if (b.startsWith(QLatin1String("Source.WaveformGenerator"))) {
			y = a->wave.value(m_time) + (has("N") ? V("N") : 0.0);
		} else if (b == QLatin1String("Source.SetPointGenerator")) {
			if (!powered) a->card.reset();
			else y = a->card.step(m_time, dt, (V("I1") >= 15.0 ? 1 : 0) + (V("I2") >= 15.0 ? 2 : 0) + (V("I3") >= 15.0 ? 4 : 0));
		} else if (b == QLatin1String("Controller.ControllerPID")) {
			// without its supply the controller has no output and starts again from zero (reset)
			if (!powered) a->pid.reset();
			else y = a->pid.step(V("W"), V("X"), dt);
		} else if (b == QLatin1String("Controller.ControllerS")) {
			if (!powered) a->sc.reset();
			else y = a->sc.step(V("W"), V("X"), dt);
		} else if (b == QLatin1String("Controller.ControllerPT2") || b == QLatin1String("Controller.ControllerIntegrator")
				   || b == QLatin1String("Controller.DifferentialAmplifier")) {
			const SimSignals::SupplyRange r = SimSignals::supplyRange(has("A1"), V("A1"), has("A2"), V("A2"), a->efficiency, a->vmin);
			if (r.pass) y = r.pass_value;
			else if (r.limit > 0) {
				if (b.endsWith(QLatin1String("PT2"))) y = std::clamp(a->pt2.step(V("U"), dt), -r.limit, r.limit);
				else if (b.endsWith(QLatin1String("Integrator"))) y = a->integ.step(V("U"), dt, r.limit);
				else y = std::clamp(V("U") - V("X"), -r.limit, r.limit);
			} else {
				a->pt2.y = a->pt2.yd = 0.0;
				a->integ.y = 0.0;
			}
		} else if (b == QLatin1String("Controller.ComparatorCard")) {
			const bool on = powered && SimSignals::comparator(a->on, V("U"), a->value(QStringLiteral("vSet"), 5.0, QStringLiteral("V")),
															   a->value(QStringLiteral("hyst"), 0.0, QStringLiteral("V")));
			if (on != a->on)
				m_messages << Message{m_time, QStringLiteral("Komparator %1 %2 bei %3 V").arg(a->label,
					on ? QStringLiteral("schaltet ein") : QStringLiteral("schaltet aus")).arg(V("U"), 0, 'f', 2)};
			a->on = on;
			y = on ? V("SP") : 0.0;
		} else if (b.startsWith(QLatin1String("Controller.PropAmplifier")) || b == QLatin1String("Relay.ValveContSolenoid")) {
			const bool coil = b == QLatin1String("Relay.ValveContSolenoid");
			const double supply_ab = coil ? std::abs(V("A") - V("B")) : supply;
			if (supply_ab >= a->vmin - 1e-9)
				y = coil ? std::clamp(V("C"), -10.0, 10.0) : SimSignals::amplifier(V("U"), a->jump, a->two_channel);
			for (const auto &pv : std::as_const(a->valves)) pv.first->u = y * pv.second;
			for (FlowValve *fv : std::as_const(a->reliefs)) fv->u = y;
		} else if (b.startsWith(QLatin1String("Sensor.PressureSensor"))) {
			if (a->sensor_node >= 0) y = SimSignals::scale(m_p.at(a->sensor_node) - P_ATM, a->s0, a->s1, a->u0, a->u1);
		} else if (b == QLatin1String("Sensor.FlowSensorHyd")) {
			const double q = a->flow_sensor ? a->flow_sensor->last_q : 0.0;
			y = SimSignals::scale(a->signed_flow ? q : std::abs(q), a->s0, a->s1, a->u0, a->u1);
		} else if (b == QLatin1String("Sensor.PositionSensor")) {
			if (a->cylinder && std::abs(V("A") - V("B")) >= a->vmin - 1e-9)
				y = a->u0 + (a->u1 - a->u0) * a->cylinder->x / a->cylinder->stroke;
		} else if (b == QLatin1String("Sensor.Voltmeter")) {
			y = V("P") - V("N");
		}
		a->out = y;
		for (const QString &o : std::as_const(a->outputs)) {
			const int k = a->port.value(o, -1);
			if (k >= 0) m_volt.insert(k, y);
		}
	}
}

// ------------------------------------------------------------------------- step
void SimulationModel::fail(const QString &why)
{
	if (m_failed) return;
	m_failed = true;
	m_failure = why;
	m_messages << Message{m_time, QStringLiteral("Abbruch: %1").arg(why)};
}

QString SimulationModel::netName(int k) const
{
	// a cylinder port first, else any labelled element on the net
	for (const Cylinder *z : m_cylinders)
		for (const QString &port : {QStringLiteral("A"), QStringLiteral("B")})
			if (node(z->e, port) == k) return QStringLiteral("%1 Anschluss %2").arg(z->label, port);
	for (auto it = m_node_of.constBegin(); it != m_node_of.constEnd(); ++it)
		if (it.value() == k && it.key()->parentElement() && !label(it.key()->parentElement()).isEmpty())
			return QStringLiteral("%1 Anschluss %2").arg(label(it.key()->parentElement()), it.key()->name());
	return QStringLiteral("Netz %1").arg(k);
}

double SimulationModel::cavity(Terminal *t) const
{
	return m_cavity.value(node(t), 0.0);
}

void SimulationModel::step(double dt)
{
	if (m_failed) return;
	const int n = static_cast<int>(m_p.size());
	QVector<double> flow(n, 0.0), dvdt(n, 0.0), vol(n, V_LINE);
	for (int k = 0; k < n; ++k)
		if (m_hydraulic.contains(k)) vol[k] = V_HYD;
	for (auto it = m_extra_volume.constBegin(); it != m_extra_volume.constEnd(); ++it)
		if (it.key() >= 0 && it.key() < n) vol[it.key()] += it.value();
	auto p = [&](int k) { return k >= 0 ? m_p.at(k) : P_ATM; };
	if (m_record_flows) m_port_flow.clear();
	// flow m from net a through a part into net b; with @a e recorded as flow into the part at a, out at b
	auto book = [&](int a, int b, double m, Element *e = nullptr) {
		if (a >= 0) flow[a] -= m;
		if (b >= 0) flow[b] += m;
		if (m_record_flows && e) {
			if (a >= 0) m_port_flow[qMakePair(e, a)] += m;
			if (b >= 0) m_port_flow[qMakePair(e, b)] -= m;
		}
	};

	electrics();
	analog(dt);

	for (Pump *pu : m_pumps) {
		const int kp = node(pu->e, QStringLiteral("P")), kt = node(pu->e, QStringLiteral("T"));
		if (kp < 0) continue;
		const double share = qBound(0.0, (pu->p_max - m_p.at(kp)) / pu->band, 1.0);
		// R08: setting alpha and leakage back from P to T (q = alpha·D·n - Cleak·dp)
		const double dp = m_p.at(kp) - p(kt);
		const double q = pu->alpha * pu->q * share - pu->cleak * dp;
		pu->last_q = q;
		pu->last_dp = dp;
		flow[kp] += q;
		if (kt >= 0) flow[kt] -= q;
		if (m_record_flows) {
			m_port_flow[qMakePair(pu->e, kp)] -= q;
			if (kt >= 0) m_port_flow[qMakePair(pu->e, kt)] += q;
		}
	}

	// R08: hydraulic motors, flow from A to B turns the shaft forward
	for (Motor *mo : m_motors) {
		const int ka = node(mo->e, QStringLiteral("A")), kb = node(mo->e, QStringLiteral("B"));
		const double dp = p(ka) - p(kb);
		if (mo->air) {
			// R12 (FORMELN.md M11): air from the port with the higher pressure, mass flow = inlet density ·
			// displacement · speed, out through the mid exhaust (open or silenced) or the other port
			const int in = p(ka) >= p(kb) ? ka : kb;
			const int out = mo->exhaust.isEmpty() ? (in == ka ? kb : ka) : node(mo->e, mo->exhaust);
			if (in >= 0) book(in, out, p(in) / (R_AIR * T_AIR) * mo->shaft.D * std::abs(mo->shaft.omega) / (2.0 * M_PI), mo->e);
			mo->shaft.step(dp, mo->tau_load, dt);
			mo->last_dp = dp;
			continue;
		}
		book(ka, kb, mo->shaft.flow(dp), mo->e);
		mo->shaft.step(dp, mo->tau_load, dt);
		mo->last_dp = dp;
	}

	for (Valve *v : m_valves) {
		if (!v->trigger_cylinder.isEmpty())
			for (const Cylinder *z : std::as_const(m_cylinders))
				if (z->label == v->trigger_cylinder)
					v->actuated[v->trigger_side] = z->markPos() >= v->trigger_lo - 1e-9 && z->markPos() <= v->trigger_hi + 1e-9;
		for (int s = 0; s < 2; ++s) {
			const auto &a = v->side[s];
			if (a.pilot && !a.types.contains(QLatin1String("solenoid"))) {
				// pilot port 14 left, 12 right; a valve piloted on one side only may name it either way
				// (ISO 5599: a 3/2 valve's pilot 12 connects 1 to 2)
				int k = node(v->e, s == 0 ? QStringLiteral("14") : QStringLiteral("12"));
				if (k < 0 && !v->side[1 - s].pilot) k = node(v->e, s == 0 ? QStringLiteral("12") : QStringLiteral("14"));
				if (k >= 0) v->actuated[s] = m_p.at(k) - P_ATM > P_PILOT;
			}
		}
		const int want = v->wanted();
		if (want != v->target) {
			v->target = want;
			v->switch_start = m_time;
			v->switch_at = m_time + (want != v->rest ? v->t_on : v->t_off);
		}
		if (v->switch_at >= 0 && m_time >= v->switch_at) {
			if (v->position != v->target)
				m_messages << Message{m_time, QStringLiteral("%1 schaltet in Stellung %2").arg(v->label).arg(v->target + 1)};
			v->position = v->target;
			v->switch_at = -1;
		}
		// while switching the spool moves like a first-order servo (FORMELN.md V01): the paths of the new position open
		// as 1 - e^(-t/tau) with tau = switching time / 3 (95 % open when the switching time is over,
		// then fully), those of the old one close alike; flow is linear in the opening
		double opened = 1.0;
		if (v->switch_at >= 0 && v->switch_at > v->switch_start)
			opened = qBound(0.0, 1.0 - std::exp(-3.0 * (m_time - v->switch_start) / (v->switch_at - v->switch_start)), 1.0);
		auto flowThrough = [&](int pos, double share) {
			if (share <= 0.0) return;
			for (const auto &path : v->positions.at(pos)) {
				const int ka = node(v->e, path.first), kb = node(v->e, path.second);
				const double m = v->hydraulic ? volumeFlow(p(ka), p(kb), v->rt) : massFlow(p(ka), p(kb), v->c);
				book(ka, kb, share * m, v->e);
			}
		};
		if (v->switch_at >= 0 && v->target != v->position) {
			flowThrough(v->position, 1.0 - opened);
			flowThrough(v->target, opened);
		} else {
			flowThrough(v->position, 1.0);
		}
	}

	for (FlowValve *s : m_flow_valves) {
		QHash<QString, int> kn;
		for (const QString &port : s->ports) kn.insert(port, node(s->e, port));
		const QString &fn = s->function;
		if (fn == QLatin1String("supply.silencer")) {
			for (int k : kn) book(k, -1, s->flow(p(k), P_ATM), s->e);
		} else if (fn == QLatin1String("flow_control_valve.throttle_valve")
				   || fn == QLatin1String("shutoff_valve.shutoff_valve")) {
			if (s->ports.size() >= 2) {
				const int a = kn.value(s->ports.at(0)), b = kn.value(s->ports.at(1));
				book(a, b, s->flow(p(a), p(b)), s->e);
			}
		} else if (fn == QLatin1String("shutoff_valve.check_valve")
				   || fn == QLatin1String("flow_control_valve.one_way_flow_control_valve")) {
			if (!kn.contains(s->free_from) || !kn.contains(s->free_to)) continue;
			const int a = kn.value(s->free_from), b = kn.value(s->free_to);
			if (fn == QLatin1String("flow_control_valve.one_way_flow_control_valve"))
				book(a, b, s->flow(p(a), p(b)), s->e);
			bool unlocked = false;
			for (const QString &port : s->ports)
				if (port != s->free_from && port != s->free_to && kn.value(port) >= 0)
					unlocked = p(kn.value(port)) - P_ATM > P_PILOT;
			if (unlocked)
				book(a, b, s->flow(p(a), p(b), 1.0, true), s->e);
			else if (p(a) - p(b) > s->p0)    // flow grows from zero at the cracking pressure
				book(a, b, s->flow(p(a) - s->p0, p(b), 1.0, true), s->e);
		} else if (fn == QLatin1String("flow_control_valve.flow_divider")) {
			// inlet 1 (or P) feeds every other port through equal conductances
			const QString in_port = kn.contains(QStringLiteral("1")) ? QStringLiteral("1")
				: (kn.contains(QStringLiteral("P")) ? QStringLiteral("P") : s->ports.value(0));
			const int in = kn.value(in_port, -1);
			for (const QString &port : s->ports)
				if (port != in_port)
					book(in, kn.value(port), s->flow(p(in), p(kn.value(port))), s->e);
		} else if (fn == QLatin1String("logic.shuttle_valve") || fn == QLatin1String("logic.dual_pressure_valve")) {
			// output 2; OR: the input with the higher pressure feeds it, AND: the one with the lower
			const int out = kn.value(QStringLiteral("2"), -1);
			QList<int> ins;
			for (const QString &port : s->ports)
				if (port != QLatin1String("2") && kn.value(port, -1) >= 0) ins << kn.value(port);
			if (out < 0 || ins.isEmpty()) continue;
			int in = ins.first();
			const bool shuttle = fn == QLatin1String("logic.shuttle_valve");
			for (int k : ins)
				if (shuttle ? p(k) > p(in) : p(k) < p(in)) in = k;
			book(in, out, s->flow(p(in), p(out)), s->e);
		} else if (fn == QLatin1String("logic.time_delay_valve")) {
			// 3/2 valve opens 1 -> 2 when the control port 3 has held pressure for t_delay; else 2 exhausts
			const int in = kn.value(QStringLiteral("1"), -1), out = kn.value(QStringLiteral("2"), -1);
			const int ctl = kn.value(QStringLiteral("3"), -1);
			const bool pilot = ctl >= 0 && p(ctl) - P_ATM > P_PILOT;
			if (pilot && s->since < 0) s->since = m_time;
			if (!pilot) s->since = -1;
			const bool open = pilot && m_time - s->since >= s->t_delay;
			if (out < 0) continue;
			if (open) book(in, out, s->flow(p(in), p(out)), s->e);
			else book(out, -1, s->flow(p(out), P_ATM), s->e);
		} else if (fn == QLatin1String("shutoff_valve.quick_exhaust_valve")) {
			const int in = kn.value(QStringLiteral("1"), -1), out = kn.value(QStringLiteral("2"), -1);
			if (p(in) >= p(out)) book(in, out, s->flow(p(in), p(out), 1.0, true), s->e);
			else book(out, -1, s->flow(p(out), P_ATM), s->e);
		} else if (fn == QLatin1String("pressure_valve.pressure_relief_valve01")) {
			// inlet 1 (P), return 2 (T, 3); without a return line it vents to the air. It opens on the
			// difference inlet - return and passes the same flow from the inlet into the return.
			const QString in_port = kn.contains(QStringLiteral("1")) ? QStringLiteral("1") : QStringLiteral("P");
			const int in = kn.value(in_port, -1);
			int ret = -1;
			for (const QString &port : {QStringLiteral("2"), QStringLiteral("T"), QStringLiteral("3")})
				if (kn.value(port, -1) >= 0) { ret = kn.value(port); break; }
			if (in >= 0)
				book(in, ret, s->flow(p(in), p(ret), qBound(0.0, (p(in) - p(ret) - s->p0) / s->dp_end, 1.0)), s->e);
		} else if (fn == QLatin1String("proportional.pressure_relief_valve")) {
			// R06 (FORMELN.md V09): A (P) inlet, B (T) return; opening pressure from the setpoint 0 … 10 V
			const int a = kn.value(QStringLiteral("A"), kn.value(QStringLiteral("P"), -1));
			const int b = kn.value(QStringLiteral("B"), kn.value(QStringLiteral("T"), -1));
			const double p0 = s->p_lo + (s->p_hi - s->p_lo) * qBound(0.0, s->u / 10.0, 1.0);
			if (a >= 0) book(a, b, s->flow(p(a), p(b), qBound(0.0, (p(a) - p(b) - p0) / s->dp_end, 1.0)), s->e);
		} else if (isFlowSensor(fn)) {
			// R06: orifice rT between A and B, its flow is the measured value
			const int a = kn.value(QStringLiteral("A"), -1), b = kn.value(QStringLiteral("B"), -1);
			s->last_q = (a >= 0 || b >= 0) ? s->flow(p(a), p(b)) : 0.0;
			book(a, b, s->last_q, s->e);
		} else if (fn == QLatin1String("pressure_valve.counterbalance_valve")) {
			// R09 (FORMELN.md V07): opens P -> T when (pP - pT) + ratio·(pX - pT) reaches p0, fully over dpEnd;
			// the check valve lets T -> P pass freely, leakage P -> T
			const int kp = kn.value(QStringLiteral("P"), -1), kt = kn.value(QStringLiteral("T"), -1);
			const int kx = kn.value(QStringLiteral("X"), -1);
			const double drive = (p(kp) - p(kt)) + s->ratio * (kx >= 0 ? p(kx) - p(kt) : 0.0);
			const double share = qBound(0.0, (drive - s->p0) / s->dp_end, 1.0);
			if (kp >= 0 || kt >= 0)
				book(kp, kt, s->flow(p(kp), p(kt), share) + s->gleak * (p(kp) - p(kt)), s->e);
			if (p(kt) > p(kp)) book(kt, kp, s->flow(p(kt), p(kp), 1.0, true), s->e);
		} else if (fn == QLatin1String("pressure_valve.pressure_sequence_valve01")
				   || fn == QLatin1String("pressure_valve.pressure_sequence_valve02")) {
			// R09 (FORMELN.md V08): opens P -> A when the pilot X (gauge) reaches p0, fully over dpEnd;
			// without a pilot line the inlet P is the pilot
			const int kp = kn.value(QStringLiteral("P"), -1), ka = kn.value(QStringLiteral("A"), -1);
			const int kx = kn.value(QStringLiteral("X"), -1);
			const double pilot = (kx >= 0 ? p(kx) : p(kp)) - P_ATM;
			if (kp >= 0 || ka >= 0) book(kp, ka, s->flow(p(kp), p(ka), qBound(0.0, (pilot - s->p0) / s->dp_end, 1.0)), s->e);
		} else if (fn.startsWith(QLatin1String("pressure_valve.pressure_compensator_"))) {
			// R09 (FORMELN.md V10, V11): pressure compensator = resistance whose share follows a pressure difference d;
			// a closer is fully open up to d = p0 and closed at p0 + dpEnd, an opener closed up to p0, open at p0 + dpEnd
			auto port = [&](const char *a, const char *b = nullptr) {
				const int k = kn.value(QLatin1String(a), -1);
				return k >= 0 || !b ? k : kn.value(QLatin1String(b), -1);
			};
			const QString kind = fn.mid(static_cast<int>(qstrlen("pressure_valve.pressure_compensator_")));
			int in = -1, out = -1;
			double d = 0;
			bool closer = false;
			if (kind.startsWith(QLatin1String("closer")) || kind.startsWith(QLatin1String("opener0"))) {
				in = port("B", "1"); out = port("A", "2");
				d = p(port("X", "3")) - p(port("XS", "4"));
				closer = kind.startsWith(QLatin1String("closer"));
			} else if (kind.startsWith(QLatin1String("upstream"))) {
				in = port("P"); out = port("A");
				d = p(out) - p(port("X"));
				closer = true;
			} else if (kind.startsWith(QLatin1String("downstream"))) {
				in = port("P"); out = port("A");
				d = p(in) - p(port("X"));
			} else {   // opener_ls: P -> T
				in = port("P"); out = port("T");
				d = p(in) - p(port("X"));
			}
			const double share = closer ? qBound(0.0, 1.0 - (d - s->p0) / s->dp_end, 1.0) : qBound(0.0, (d - s->p0) / s->dp_end, 1.0);
			if (in >= 0 || out >= 0) book(in, out, s->flow(p(in), p(out), share), s->e);
		} else if (fn == QLatin1String("pressure_valve.shock_and_anti_cavitation_valve")) {
			// R09 (FORMELN.md V12): A (= A1 = A2) and B relieve to T above their opening pressure, T feeds A or B
			// through a check valve when the work line falls below the tank pressure (anti-cavitation)
			const int ka = kn.value(QStringLiteral("A1"), kn.value(QStringLiteral("A2"), -1));
			const int kb = kn.value(QStringLiteral("B1"), kn.value(QStringLiteral("B2"), -1));
			const int kt = kn.value(QStringLiteral("T"), -1);
			auto relief = [&](int k, double p0, double band, double rt) {
				if (k < 0) return;
				const double share = qBound(0.0, (p(k) - p(kt) - p0) / band, 1.0);
				if (share > 0) book(k, kt, volumeFlow(p(k), p(kt), rt / std::pow(std::max(share, 1e-3), 2)), s->e);
				if (p(kt) > p(k)) book(kt, k, volumeFlow(p(kt), p(k), s->rt_free), s->e);
			};
			relief(ka, s->p0, s->dp_end, s->rt);
			relief(kb, s->p0_b, s->dp_end_b, s->rt_b);
		} else if (fn == QLatin1String("pressure_valve.pressure_sequence_valve")) {
			// R09 (FORMELN.md V13): pneumatic sequence valve opens 1 -> 2 when the pilot 12 (gauge) reaches p0
			const int k1 = kn.value(QStringLiteral("1"), kn.value(QStringLiteral("C1"), -1));
			const int k2 = kn.value(QStringLiteral("2"), kn.value(QStringLiteral("C2"), -1));
			const int k12 = kn.value(QStringLiteral("12"), kn.value(QStringLiteral("C12"), -1));
			const double share = k12 >= 0 ? qBound(0.0, (p(k12) - P_ATM - s->p0) / s->dp_end, 1.0) : 0.0;
			if (k1 >= 0 || k2 >= 0) book(k1, k2, s->flow(p(k1), p(k2), share), s->e);
		} else if (fn == QLatin1String("pressure_valve.pressure_regulator")) {
			const int in = kn.value(QStringLiteral("1"), kn.value(QStringLiteral("P"), -1));
			const int out = kn.value(QStringLiteral("2"), kn.value(QStringLiteral("A"), -1));
			if (in < 0 || out < 0) continue;
			const double p2 = p(out) - P_ATM;
			book(in, out, s->flow(p(in), p(out), qBound(0.0, (s->p0 - p2) / s->dp_end, 1.0)), s->e);
			book(out, -1, s->flow(p(out), P_ATM, qBound(0.0, (p2 - s->p0 - s->dp_end) / s->dp_end, 1.0)), s->e);
		}
	}

	for (PropValve *v : m_prop_valves) {   // FORMELN.md V05, V06
		v->s = SimPhysics::pt1(v->s, qBound(-1.0, v->u / 10.0, 1.0), v->tau, dt);
		const int kp = node(v->e, QStringLiteral("P")), ka = node(v->e, QStringLiteral("A"));
		const int kb = node(v->e, QStringLiteral("B")), kt = node(v->e, QStringLiteral("T"));
		auto edge = [&](int a, int b, double x) {
			if (a < 0 && b < 0) return;
			const double area = v->triangular ? x * x : x;
			double q = v->gleak * (p(a) - p(b));
			if (area > 1e-9) q += volumeFlow(p(a), p(b), v->rt / (area * area));
			book(a, b, q, v->e);
		};
		edge(kp, ka, SimPhysics::edgeOpening(v->s, v->lap_p));
		edge(kb, kt, SimPhysics::edgeOpening(v->s, v->lap_t));
		edge(kp, kb, SimPhysics::edgeOpening(-v->s, v->lap_p));
		edge(ka, kt, SimPhysics::edgeOpening(-v->s, v->lap_t));
	}
	for (const Pipe *pi : std::as_const(m_pipes)) {   // FORMELN.md L01–L03
		const int ka = node(pi->e, pi->a), kb = node(pi->e, pi->b);
		if (ka < 0 && kb < 0) continue;
		if (pi->hydraulic) {
			book(ka, kb, pi->flow.flow(p(ka) - p(kb)), pi->e);
		} else {
			SimPhysics::PipeFlow g = pi->flow;   // air: density at the mean pressure (isothermal)
			g.rho = 0.5 * (p(ka) + p(kb)) / (R_AIR * T_AIR);
			book(ka, kb, g.rho * g.flow(p(ka) - p(kb)), pi->e);
		}
	}
	for (const Ejector *v : std::as_const(m_ejectors)) {   // FORMELN.md U01, U02
		const int k1 = node(v->e, QStringLiteral("1")), k2 = node(v->e, QStringLiteral("2"));
		const double share = k1 >= 0 ? qBound(0.0, (p(k1) - P_ATM) / v->dp_opt, 1.0) : 0.0;
		if (k1 >= 0) book(k1, -1, massFlow(p(k1), P_ATM, v->c_drive), v->e);
		if (k2 < 0) continue;
		// suction: zero at the best vacuum p_atm + pu_min·share, q_max·share against the atmosphere. The
		// slope q_max/|pu_min| does not depend on the supply, so undriven (share 0) the vacuum port is the
		// same linear path back to the air: one continuous characteristic
		book(k2, -1, v->q_max * (p(k2) - P_ATM - v->pu_min * share) / (-v->pu_min), v->e);
	}
	for (const SuctionCup *s : std::as_const(m_cups)) {
		if (s->node < 0) continue;
		const double c = s->object ? s->c_leak : s->c_open;
		if (c > 0) book(s->node, -1, massFlow(p(s->node), P_ATM, c), s->e);
	}

	for (Cylinder *z : m_cylinders) {
		const int ka = node(z->e, QStringLiteral("A")), kb = node(z->e, QStringLiteral("B"));
		const double f = (p(ka) - P_ATM) * z->a_k - (p(kb) - P_ATM) * z->a_r - z->opposing();
		if (m_record_flows) m_cylinder_force.insert(z->e, (p(ka) - P_ATM) * z->a_k - (p(kb) - P_ATM) * z->a_r);
		SimPhysics::pistonStep(*z, f, dt);
		if (ka >= 0) { vol[ka] += V_DEAD + z->a_k * z->x; dvdt[ka] += z->a_k * z->v; }
		if (kb >= 0) { vol[kb] += V_DEAD + z->a_r * (z->stroke - z->x); dvdt[kb] -= z->a_r * z->v; }
		// an elastic stop may not push the piston through the dead volume of a chamber
		if (z->stop_k > 0 && (V_DEAD + z->a_k * z->x <= 0 || V_DEAD + z->a_r * (z->stroke - z->x) <= 0) && !m_failed)
			fail(QStringLiteral("%1: Endanschlag zu weich, Kolben dringt durch das Totvolumen (stop_k erhöhen)").arg(z->label));
		const QString end = z->x <= 0.0005 ? QStringLiteral("hinten")
			: (z->x >= z->stroke - 0.0005 ? QStringLiteral("vorne") : QString());
		if (end != m_end_position.value(z->label)) {
			m_end_position.insert(z->label, end);
			if (!end.isEmpty())
				m_messages << Message{m_time, QStringLiteral("%1 Endlage %2").arg(z->label, end)};
		}
	}

	for (int k : std::as_const(m_silenced))
		book(k, -1, m_hydraulic.contains(k) ? volumeFlow(p(k), P_ATM, 0.05) : massFlow(p(k), P_ATM, m_silencer_c));
	// all ports of a compressed air receiver are one vessel
	for (const auto &link : std::as_const(m_vessel_links))
		book(link.first, link.second, massFlow(p(link.first), p(link.second), m_vessel_c));
	QHash<int, QVector<SimPhysics::GasCharge>> gas;
	for (const Accumulator *a : std::as_const(m_accumulators))
		gas[a->node] << SimPhysics::GasCharge{a->p_pre, a->v0, a->n};

	const QVector<double> before = m_p;
	for (int k = 0; k < n; ++k) {
		if (m_fixed.contains(k) || m_electric.contains(k)) continue;
		if (m_hydraulic.contains(k) && gas.contains(k))
			m_p[k] = SimPhysics::oilNodeWithAccumulators(m_p.at(k), vol.at(k), dvdt.at(k), flow.at(k), E_OIL, dt,
														 P_CAVITATION, gas.value(k), m_cavity[k]);
		else if (m_hydraulic.contains(k))
			m_p[k] = SimPhysics::oilNode(m_p.at(k), vol.at(k), dvdt.at(k), flow.at(k), E_OIL, dt, P_CAVITATION,
										 m_cavity[k]);
		else
			m_p[k] = SimPhysics::gasNode(m_p.at(k), vol.at(k), dvdt.at(k), flow.at(k), R_AIR * T_AIR, dt);
	}
	// R03: a net swinging from step to step is numerically unstable: stop with a diagnosis (FORMELN.md N02)
	for (int k = 0; k < n && !m_failed; ++k) {
		if (m_fixed.contains(k) || m_electric.contains(k) || k >= m_watch.size()) continue;
		const double dp = m_p.at(k) - before.at(k);
		if (m_watch[k].update(dp, std::max(1e4, 1e-3 * m_p.at(k))))
			fail(QStringLiteral("Numerisch instabil im Netz an %1: der Druck schwingt von Schritt zu Schritt (zuletzt %2 bar je "
								"Schritt). Das Netzvolumen ist für die Durchflüsse zu klein bzw. ein Widerstand zu klein "
								"(Kennwerte qNN/rT prüfen).").arg(netName(k)).arg(dp / 1e5, 0, 'g', 3));
	}
	// a computing fault ends the run with a message instead of a plausible looking value
	for (int k = 0; k < n && !m_failed; ++k)
		if (!std::isfinite(m_p.at(k)))
			fail(QStringLiteral("Rechenfehler: Druck nicht endlich im Netz an %1").arg(netName(k)));
	for (const Cylinder *z : std::as_const(m_cylinders))
		if (!m_failed && (!std::isfinite(z->x) || !std::isfinite(z->v)))
			fail(QStringLiteral("Rechenfehler: Weg oder Geschwindigkeit von %1 nicht endlich").arg(z->label));
	if (m_failed) return;
	// oil torn off below the lowest pressure: a void, kept in the volume balance and refilled first
	for (auto it = m_cavity.constBegin(); it != m_cavity.constEnd(); ++it) {
		const bool open = it.value() > 0.0;
		if (open == m_cavity_open.contains(it.key())) continue;
		if (open) {
			m_cavity_open.insert(it.key());
			m_messages << Message{m_time, QStringLiteral("Unterdruck an %1: Öl reißt ab, Hohlraum entsteht "
				"(bilanziert; Dampfdruck und Ausgasen nicht gerechnet – Nachsaugen oder Gegenhalteventil vorsehen)")
				.arg(netName(it.key()))};
		} else {
			m_cavity_open.remove(it.key());
			m_messages << Message{m_time, QStringLiteral("Hohlraum an %1 wieder gefüllt").arg(netName(it.key()))};
		}
	}
	for (PressureSwitch *s : m_pressure_switches) {
		if (s->node < 0) continue;
		const double g = m_p.at(s->node) - P_ATM;
		const bool on = s->vacuum ? (s->on ? g < s->p_off : g <= s->p_on) : (s->on ? g > s->p_off : g >= s->p_on);
		if (on != s->on) {
			s->on = on;
			m_messages << Message{m_time, QStringLiteral("%1 %2 %3 bei %4 bar").arg(
				s->vacuum ? QStringLiteral("Vakuumschalter") : QStringLiteral("Druckschalter"), s->label,
				on ? QStringLiteral("schaltet ein") : QStringLiteral("schaltet aus")).arg(g / 1e5, 0, 'f', 2)};
		}
	}
	// R10 suction cups: holding force against the weight with the safety factor (FORMELN.md U03)
	for (SuctionCup *s : m_cups) {
		if (s->node < 0) continue;
		s->force = s->object ? std::max(0.0, (P_ATM - m_p.at(s->node)) * s->area) : 0.0;
		const bool held = s->object && s->force > 0.0 && s->force >= s->safety * s->weight;
		if (held != s->held) {
			s->held = held;
			m_messages << Message{m_time, held
				? QStringLiteral("Sauger %1 hält das Werkstück (%2 N bei %3 bar)").arg(s->label).arg(s->force, 0, 'f', 2)
					  .arg((m_p.at(s->node) - P_ATM) / 1e5, 0, 'f', 2)
				: QStringLiteral("Sauger %1 hält das Werkstück nicht mehr (%2 N, nötig %3 N)").arg(s->label)
					  .arg(s->force, 0, 'f', 2).arg(s->safety * s->weight, 0, 'f', 2)};
		}
	}
	m_time += dt;
}

void SimulationModel::runUntil(double time, double dt)
{
	// a failed run does not advance the time: stop instead of looping
	while (!m_failed && m_time + dt <= time + 1e-12)
		step(dt);
}

// ------------------------------------------------------------------------- input / state
void SimulationModel::pressButton(const QString &lbl, bool pressed)
{
	bool latching = false;
	for (const Contact *c : std::as_const(m_contacts))
		latching = latching || (c->latching && label(c->e) == lbl);
	if (latching) {
		// selector switch: a press switches it over
		if (pressed && !m_pressed.contains(lbl)) {
			if (m_latched.contains(lbl)) m_latched.remove(lbl); else m_latched.insert(lbl);
			m_messages << Message{m_time, QStringLiteral("%1 %2").arg(lbl, m_latched.contains(lbl)
				? QStringLiteral("eingeschaltet") : QStringLiteral("ausgeschaltet"))};
		}
		if (pressed) m_pressed.insert(lbl); else m_pressed.remove(lbl);
		return;
	}
	if (pressed == m_pressed.contains(lbl)) return;
	if (pressed) m_pressed.insert(lbl); else m_pressed.remove(lbl);
	m_messages << Message{m_time, QStringLiteral("%1 %2").arg(lbl, pressed ? QStringLiteral("gedrückt")
																		 : QStringLiteral("losgelassen"))};
}

bool SimulationModel::setSetpoint(const QString &lbl, double volts)
{
	for (FlowValve *s : m_flow_valves)
		if (s->function == QLatin1String("proportional.pressure_relief_valve") && label(s->e) == lbl) {
			if (s->u != volts) m_messages << Message{m_time, QStringLiteral("%1 Sollwert %2 V").arg(lbl).arg(volts, 0, 'g', 4)};
			s->u = volts;
			return true;
		}
	for (PropValve *v : m_prop_valves)
		if (v->label == lbl) {
			if (v->u != volts) m_messages << Message{m_time, QStringLiteral("%1 Sollwert %2 V").arg(lbl).arg(volts, 0, 'g', 4)};
			v->u = volts;
			return true;
		}
	return false;
}

bool SimulationModel::actuateValve(const QString &lbl, bool left, bool on)
{
	for (Valve *v : m_valves)
		if (v->label == lbl) {
			if (v->toggle_side >= 0) {
				// selector switch: a press on either half switches it over, releasing does nothing
				if (!on) return true;
				v->latched = !v->latched;
				m_messages << Message{m_time, QStringLiteral("%1 umgeschaltet").arg(lbl)};
				return true;
			}
			v->actuated[left ? 0 : 1] = on;
			m_messages << Message{m_time, QStringLiteral("%1 %2 %3").arg(lbl,
				left ? QStringLiteral("links") : QStringLiteral("rechts"),
				on ? QStringLiteral("betätigt") : QStringLiteral("losgelassen"))};
			return true;
		}
	return false;
}

// ------------------------------------------------------------------------- measurement (R01)
namespace {
QString partName(Element *e)
{
	const QString l = label(e);
	return l.isEmpty() ? e->name() : l;
}

QString portName(Terminal *t, int index)
{
	const QString n = t->name().trimmed();
	return n.isEmpty() ? QStringLiteral("#%1").arg(index + 1) : n;
}
}

QList<SimulationModel::Signal> SimulationModel::signalList() const
{
	QList<Signal> out;
	QSet<Element *> fluid_parts;
	for (const Valve *v : m_valves) fluid_parts << v->e;
	for (const Pump *pu : m_pumps) fluid_parts << pu->e;
	for (const FlowValve *s : m_flow_valves) fluid_parts << s->e;
	for (const PropValve *v : m_prop_valves) fluid_parts << v->e;
	for (const Pipe *pi : m_pipes) fluid_parts << pi->e;
	for (const Ejector *v : m_ejectors) fluid_parts << v->e;
	QSet<Element *> seen;
	for (auto it = m_node_of.constBegin(); it != m_node_of.constEnd(); ++it) {
		Element *e = it.key()->parentElement();
		if (!e || seen.contains(e)) continue;
		seen << e;
		const QList<Terminal *> ts = e->terminals();
		for (int i = 0; i < ts.size(); ++i) {
			Terminal *u = ts.at(i);
			if (!isFluid(u) || node(u) < 0) continue;
			const QString part = partName(e), port = portName(u, i);
			const QString at = part + QLatin1Char('.') + port;
			out << Signal{QStringLiteral("p:") + at, QStringLiteral("pressure"), QStringLiteral("Pa"),
						  QStringLiteral("gauge (über P_ATM = 101300 Pa)"), part, port};
			out << Signal{QStringLiteral("pa:") + at, QStringLiteral("pressure"), QStringLiteral("Pa"),
						  QStringLiteral("absolute"), part, port};
			if (fluid_parts.contains(e)) {
				const bool oil = m_hydraulic.contains(node(u));
				out << Signal{QStringLiteral("q:") + at, oil ? QStringLiteral("volume_flow") : QStringLiteral("mass_flow"),
							  oil ? QStringLiteral("m3/s") : QStringLiteral("kg/s"),
							  QStringLiteral("into_part (positiv: in das Bauteil hinein)"), part, port};
			}
		}
	}
	for (const PropValve *v : m_prop_valves) {
		out << Signal{QStringLiteral("u:") + v->label, QStringLiteral("voltage"), QStringLiteral("V"),
					  QStringLiteral("Sollwert -10 … 10 V, positiv: P→A"), v->label, QString()};
		out << Signal{QStringLiteral("s:") + v->label, QStringLiteral("state"), QStringLiteral("1"),
					  QStringLiteral("Schieberweg -1 … 1, positiv: P→A, B→T"), v->label, QString()};
	}
	for (const Analog *a : m_analog)
		out << Signal{QStringLiteral("u:") + a->label, QStringLiteral("voltage"), QStringLiteral("V"),
					  a->block.endsWith(QLatin1String("Voltmeter")) ? QStringLiteral("Anzeige p - n")
					  : QStringLiteral("Ausgang %1 gegen 0 V").arg(a->outputs.value(0, QStringLiteral("–"))), a->label, QString()};
	for (const Analog *a : m_analog)
		if (a->block.endsWith(QLatin1String("ControllerPID")) || a->block.endsWith(QLatin1String("ControllerS")))
			out << Signal{QStringLiteral("e:") + a->label, QStringLiteral("voltage"), QStringLiteral("V"),
						  QStringLiteral("Regelabweichung W - X"), a->label, QString()};
	for (const FlowValve *s : m_flow_valves)
		if (isFlowSensor(s->function) && !label(s->e).isEmpty())
			out << Signal{QStringLiteral("qs:") + label(s->e), QStringLiteral("volume_flow"), QStringLiteral("m3/s"),
						  QStringLiteral("Durchfluss A → B des Durchflusssensors"), label(s->e), QString()};
	for (const SuctionCup *s : m_cups) {
		out << Signal{QStringLiteral("Fhold:") + s->label, QStringLiteral("force"), QStringLiteral("N"),
					  QStringLiteral("Haltekraft (p_atm - p)·A, 0 ohne Werkstück"), s->label, QString()};
		out << Signal{QStringLiteral("hold:") + s->label, QStringLiteral("state"), QStringLiteral("1"),
					  QStringLiteral("1 = hält (Haltekraft ≥ s·m·g)"), s->label, QString()};
	}
	for (const Cylinder *z : m_cylinders) {
		if (z->rotary) {
			out << Signal{QStringLiteral("x:") + z->label, QStringLiteral("angle"), QStringLiteral("rad"),
						  QStringLiteral("0 = Ausgangslage"), z->label, QString()};
			out << Signal{QStringLiteral("v:") + z->label, QStringLiteral("angular_velocity"), QStringLiteral("rad/s"),
						  QStringLiteral("positiv: Druck an A dreht vorwärts"), z->label, QString()};
			out << Signal{QStringLiteral("F:") + z->label, QStringLiteral("torque"), QStringLiteral("N.m"),
						  QStringLiteral("Druckmoment, positiv: vorwärts (ohne Reibung und Last)"), z->label, QString()};
			continue;
		}
		out << Signal{QStringLiteral("x:") + z->label, QStringLiteral("position"), QStringLiteral("m"),
					  QStringLiteral("0 = eingefahren"), z->label, QString()};
		out << Signal{QStringLiteral("v:") + z->label, QStringLiteral("velocity"), QStringLiteral("m/s"),
					  QStringLiteral("positiv: ausfahrend"), z->label, QString()};
		out << Signal{QStringLiteral("F:") + z->label, QStringLiteral("force"), QStringLiteral("N"),
					  QStringLiteral("Druckkraft auf den Kolben, positiv: ausfahrend (ohne Reibung, Last, Feder)"), z->label, QString()};
	}
	for (const Motor *mo : m_motors) {
		out << Signal{QStringLiteral("omega:") + mo->label, QStringLiteral("angular_velocity"), QStringLiteral("rad/s"),
					  QStringLiteral("positiv: Durchfluss von A nach B"), mo->label, QString()};
		out << Signal{QStringLiteral("phi:") + mo->label, QStringLiteral("angle"), QStringLiteral("rad"),
					  QStringLiteral("0 beim Start"), mo->label, QString()};
		out << Signal{QStringLiteral("tau:") + mo->label, QStringLiteral("torque"), QStringLiteral("N.m"),
					  QStringLiteral("hydraulisches Moment alpha·D·Δp/2π, positiv: antreibend"), mo->label, QString()};
		out << Signal{QStringLiteral("P:") + mo->label, QStringLiteral("power"), QStringLiteral("W"),
					  QStringLiteral("Wellenleistung tau·omega"), mo->label, QString()};
	}
	for (const Pump *pu : m_pumps) {
		const QString l = label(pu->e).isEmpty() ? pu->e->name() : label(pu->e);
		out << Signal{QStringLiteral("P:") + l, QStringLiteral("power"), QStringLiteral("W"),
					  QStringLiteral("hydraulisch abgegebene Leistung Δp·q"), l, QString()};
	}
	for (const Valve *v : m_valves)
		out << Signal{QStringLiteral("pos:") + v->label, QStringLiteral("state"), QStringLiteral("1"),
					  QStringLiteral("Stellung 1 … n (links nach rechts)"), v->label, QString()};
	return out;
}

double SimulationModel::signalValue(const QString &id, bool *ok) const
{
	if (ok) *ok = false;
	const int colon = static_cast<int>(id.indexOf(QLatin1Char(':')));
	if (colon <= 0) return std::nan("");
	const QString kind = id.left(colon), rest = id.mid(colon + 1);
	if (kind == QLatin1String("x") || kind == QLatin1String("v") || kind == QLatin1String("F")) {
		for (const Cylinder *z : m_cylinders)
			if (z->label == rest) {
				if (kind == QLatin1String("F") && !m_cylinder_force.contains(z->e)) return std::nan("");
				if (ok) *ok = true;
				return kind == QLatin1String("x") ? z->x : kind == QLatin1String("v") ? z->v : m_cylinder_force.value(z->e);
			}
		return std::nan("");
	}
	if (kind == QLatin1String("omega") || kind == QLatin1String("phi") || kind == QLatin1String("tau") || kind == QLatin1String("P")) {
		for (const Motor *mo : m_motors)
			if (mo->label == rest) {
				if (ok) *ok = true;
				const double tau = mo->shaft.torque(mo->last_dp);
				return kind == QLatin1String("omega") ? mo->shaft.omega : kind == QLatin1String("phi") ? mo->shaft.phi
					 : kind == QLatin1String("tau") ? tau : tau * mo->shaft.omega;
			}
		if (kind == QLatin1String("P"))
			for (const Pump *pu : m_pumps)
				if ((label(pu->e).isEmpty() ? pu->e->name() : label(pu->e)) == rest) {
					if (ok) *ok = true;
					return pu->last_dp * pu->last_q;
				}
		return std::nan("");
	}
	if (kind == QLatin1String("Fhold") || kind == QLatin1String("hold")) {
		for (const SuctionCup *s : m_cups)
			if (s->label == rest) {
				if (ok) *ok = true;
				return kind == QLatin1String("hold") ? (s->held ? 1.0 : 0.0) : s->force;
			}
		return std::nan("");
	}
	if (kind == QLatin1String("u") || kind == QLatin1String("s")) {
		for (const PropValve *v : m_prop_valves)
			if (v->label == rest) { if (ok) *ok = true; return kind == QLatin1String("u") ? v->u : v->s; }
		if (kind == QLatin1String("u"))
			for (const Analog *a : m_analog)
				if (a->label == rest) { if (ok) *ok = true; return a->out; }
		return std::nan("");
	}
	if (kind == QLatin1String("e")) {
		for (const Analog *a : m_analog)
			if (a->label == rest) { if (ok) *ok = true; return a->block.endsWith(QLatin1String("ControllerS")) ? a->sc.e : a->pid.e; }
		return std::nan("");
	}
	if (kind == QLatin1String("qs")) {
		for (const FlowValve *s : m_flow_valves)
			if (isFlowSensor(s->function) && label(s->e) == rest) { if (ok) *ok = true; return s->last_q; }
		return std::nan("");
	}
	if (kind == QLatin1String("pos")) {
		for (const Valve *v : m_valves)
			if (v->label == rest) { if (ok) *ok = true; return v->position + 1; }
		return std::nan("");
	}
	const int dot = static_cast<int>(rest.lastIndexOf(QLatin1Char('.')));
	if (dot <= 0) return std::nan("");
	const QString part = rest.left(dot), port = rest.mid(dot + 1);
	QSet<Element *> seen;
	for (auto it = m_node_of.constBegin(); it != m_node_of.constEnd(); ++it) {
		Element *e = it.key()->parentElement();
		if (!e || seen.contains(e) || partName(e) != part) continue;
		seen << e;
		const QList<Terminal *> ts = e->terminals();
		for (int i = 0; i < ts.size(); ++i) {
			Terminal *u = ts.at(i);
			const int k = node(u);
			if (!isFluid(u) || k < 0 || portName(u, i) != port) continue;
			if (kind == QLatin1String("p")) { if (ok) *ok = true; return m_p.at(k) - P_ATM; }
			if (kind == QLatin1String("pa")) { if (ok) *ok = true; return m_p.at(k); }
			if (kind == QLatin1String("q")) {
				if (!m_record_flows) return std::nan("");     // not recorded: no invented 0
				if (ok) *ok = true;
				return m_port_flow.value(qMakePair(e, k), 0.0);
			}
		}
	}
	return std::nan("");
}

QStringList SimulationModel::buttons() const
{
	QStringList out;
	for (Contact *c : m_contacts)
		if (c->kind == QLatin1String("button") && !label(c->e).isEmpty()) out << label(c->e);
	out.removeDuplicates();
	return out;
}

QStringList SimulationModel::valves() const
{
	QStringList out;
	for (Valve *v : m_valves) out << v->label;
	return out;
}

QList<SimulationModel::CylinderState> SimulationModel::cylinders() const
{
	QList<CylinderState> out;
	for (Cylinder *z : m_cylinders)
		out << CylinderState{z->e, z->label, z->rotary ? z->x * 180.0 / M_PI : z->x * 1000.0,
							 z->rotary ? z->stroke * 180.0 / M_PI : z->stroke * 1000.0, z->v};
	return out;
}

QList<SimulationModel::ValveState> SimulationModel::valveStates() const
{
	QList<ValveState> out;
	for (Valve *v : m_valves)
		out << ValveState{v->e, v->label, v->position + 1, static_cast<int>(v->positions.size())};
	return out;
}

QList<SimulationModel::Message> SimulationModel::takeMessages()
{
	QList<Message> out;
	out.swap(m_messages);
	return out;
}

QString SimulationModel::summary() const
{
	return QStringLiteral("%1 Netze, %2 Quellen/Tanks, %3 Pumpen, %4 Wegeventile, %5 Stromventile, "
						  "%6 Zylinder, %7 Kontakte, %8 Relais, %9 Ventilspulen")
		.arg(m_p.size()).arg(m_fixed.size()).arg(m_pumps.size()).arg(m_valves.size())
		.arg(m_flow_valves.size()).arg(m_cylinders.size()).arg(m_contacts.size())
		.arg(m_relays.size()).arg(m_coils.size())
		+ QStringLiteral(", %1 Zähler, %2 SPS-Programme, %3 SPS-Kanäle")
		.arg(m_counters.size()).arg(m_plcs.size()).arg(m_plc_channels.size())
		+ QStringLiteral(", %1 Hydrospeicher, %2 Druckschalter").arg(m_accumulators.size()).arg(m_pressure_switches.size())
		+ (m_grafcet ? QStringLiteral(", GRAFCET mit %1 Schritten und %2 Transitionen")
						   .arg(m_grafcet->stepCount()).arg(m_grafcet->transitionCount())
					 : QString());
}

QList<QPair<Element *, QString>> SimulationModel::elementStates() const
{
	QList<QPair<Element *, QString>> out;
	for (Cylinder *z : m_cylinders)
		out << qMakePair(z->e, QStringLiteral("%1: %2 / %3 %4%5").arg(z->label)
			.arg(z->rotary ? z->x * 180.0 / M_PI : z->x * 1000.0, 0, 'f', 0)
			.arg(z->rotary ? z->stroke * 180.0 / M_PI : z->stroke * 1000.0, 0, 'f', 0)
			.arg(z->rotary ? QStringLiteral("°") : QStringLiteral("mm"))
			.arg(z->v > 1e-3 ? QStringLiteral("  >>") : (z->v < -1e-3 ? QStringLiteral("  <<") : QString())));
	for (Valve *v : m_valves)
		out << qMakePair(v->e, QStringLiteral("%1: Stellung %2").arg(v->label).arg(v->position + 1));
	for (Relay *r : m_relays)
		out << qMakePair(r->e, r->on ? QStringLiteral("%1: angezogen").arg(r->label) : QString());
	for (Coil *c : m_coils)
		out << qMakePair(c->e, c->on ? QStringLiteral("%1: an").arg(c->label) : QString());
	for (Counter *z : m_counters)
		if (!z->reset_coil)
			out << qMakePair(z->e, QStringLiteral("%1: %2 / %3").arg(z->label).arg(z->count).arg(z->preset));
	for (PlcChannel *ch : m_plc_channels)
		out << qMakePair(ch->e, ch->value ? QStringLiteral("%1 = 1").arg(ch->names.value(0)) : QString());
	for (PressureSwitch *s : m_pressure_switches)
		out << qMakePair(s->e, s->on ? QStringLiteral("%1: ein").arg(s->label) : QString());
	for (Analog *a : m_analog)
		out << qMakePair(a->e, QStringLiteral("%1: %2 V").arg(a->label).arg(a->out, 0, 'f', 2));
	for (Accumulator *a : m_accumulators)
		out << qMakePair(a->e, QStringLiteral("%1: %2 bar").arg(a->label)
			.arg((m_p.value(a->node, P_ATM) - P_ATM) / 1e5, 0, 'f', 1));
	return out;
}

QList<Element *> SimulationModel::activeSteps() const
{
	return m_grafcet ? m_grafcet->activeSteps() : QList<Element *>();
}

double SimulationModel::plcValue(const QString &name) const
{
	return m_plc_vars.value(PlcProgram::normalized(name), 0.0);
}

double SimulationModel::pressureBar(Terminal *t) const
{
	const int k = node(t);
	if (k < 0) {
		// an open fluid port is at the air: 0 bar gauge; not fluid: NaN
		const bool fluid = t && (t->medium() == Fluid::Medium::Pneumatic || t->medium() == Fluid::Medium::Hydraulic);
		return fluid ? 0.0 : std::numeric_limits<double>::quiet_NaN();
	}
	if (m_electric.contains(k)) return std::numeric_limits<double>::quiet_NaN();
	return (m_p.at(k) - P_ATM) / 1e5;
}

bool SimulationModel::energized(Terminal *t) const
{
	const int k = node(t);
	return k >= 0 && m_plus_nets.contains(k);
}

bool SimulationModel::isFluid(Terminal *t) const
{
	return t && (t->medium() == Fluid::Medium::Pneumatic || t->medium() == Fluid::Medium::Hydraulic);
}

bool SimulationModel::isElectric(Terminal *t) const
{
	return t && t->medium() == Fluid::Medium::Electric;
}
