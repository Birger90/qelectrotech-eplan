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
#include "simulationmodel.h"
#include "simulationphysics.h"

#include "plcprogram.h"

#include "../diagram.h"
#include "../fluid/fluiddata.h"
#include "../fluid/fluidtypes.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/terminal.h"

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
constexpr double RHO_N = 1.185;         // kg/m³ (ANR)
constexpr double B_CRIT = 0.3;          // critical pressure ratio
constexpr double V_LINE = 5e-6;         // m³ per pneumatic net
constexpr double V_DEAD = 2e-6;         // m³ per cylinder chamber
constexpr double P_PILOT = 1.5e5;       // Pa gauge that switches a pilot
constexpr double E_OIL = 7e8;           // Pa effective bulk modulus
constexpr double V_HYD = 1e-4;          // m³ per hydraulic net
constexpr double DP_LAMINAR = 0.3e5;    // Pa
constexpr double T_PULL_IN = 0.010;     // s relay
constexpr double T_DROP_OUT = 0.008;    // s relay

double massFlow(double p1, double p2, double c)
{
	if (p1 < p2) return -massFlow(p2, p1, c);
	if (p1 <= 0) return 0.0;
	const double r = p2 / p1;
	const double f = r <= B_CRIT ? 1.0
		: std::sqrt(std::max(0.0, 1.0 - std::pow((r - B_CRIT) / (1.0 - B_CRIT), 2)));
	return c * p1 * f * RHO_N;
}

double conductanceFromQnn(double qnn_l_min)
{
	const double p1 = P_ATM + 6e5;
	const double r = (p1 - 1e5) / p1;
	const double f = std::sqrt(1.0 - std::pow((r - B_CRIT) / (1.0 - B_CRIT), 2));
	return (qnn_l_min / 1000.0 / 60.0) / (p1 * f);
}

double volumeFlow(double p1, double p2, double rt)
{
	const double dp = p1 - p2;
	if (rt <= 0) rt = 1e-4;
	const double dp_bar = std::abs(dp) / 1e5;
	const double lam = DP_LAMINAR / 1e5;
	const double q = dp_bar > lam ? std::sqrt(dp_bar / rt) : std::sqrt(lam / rt) * dp_bar / lam;
	return std::copysign(q / 60000.0, dp);
}

QString label(Element *e) { return e->elementInformations()[QStringLiteral("label")].toString(); }
QString info(Element *e, const char *key) { return e->elementInformations()[QLatin1String(key)].toString(); }

double param(const FluidData &f, const QString &name, double fallback)
{
	bool ok = false;
	const double v = f.parameter(name).toDouble(&ok);
	return ok ? v : fallback;
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

// ------------------------------------------------------------------------- components
struct SimulationModel::Valve {
	Element *e = nullptr;
	QString label;
	QList<QList<QPair<QString, QString>>> positions;
	int rest = 0, position = 0, target = 0;
	FluidData::Actuation side[2];
	bool actuated[2] = {false, false};
	double switch_at = -1;
	double c = 0, rt = 0.1, t_on = 0.02, t_off = 0.03;
	bool hydraulic = false;

	int wanted() const {
		const bool l = actuated[0], r = actuated[1];
		if (l && !r) return 0;
		if (r && !l) return static_cast<int>(positions.size()) - 1;
		if (l && r) return position;
		const bool spring = side[0].spring || side[1].spring || side[0].air_spring || side[1].air_spring;
		return spring ? rest : position;
	}
};

struct SimulationModel::Cylinder {
	Element *e = nullptr;
	QString label;
	double a_k = 0, a_r = 0, stroke = 0.1, m = 0.1, f_static = 0, f_slide = 0, damping = 20;
	double f_spring = 0, f_load = 0, x = 0, v = 0;
};

struct SimulationModel::Pump {
	Element *e = nullptr;
	double q = 0, p_max = 0, band = 5e5;
};

struct SimulationModel::FlowValve {
	Element *e = nullptr;
	QString function;
	QStringList ports;
	QString free_from, free_to;
	bool hydraulic = false;
	double c = 0, c_free = 0, rt = 0.1, rt_free = 0.05, p0 = 0, dp_end = 0.5e5;

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
	QString trigger;       // limit switch: "-MM1 vorne"
	Relay *relay = nullptr;
	QString mode;
};

struct SimulationModel::Coil {
	Element *e = nullptr;
	QString label;
	int n1 = -1, n2 = -1;
	bool on = false, known = false;
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
	qDeleteAll(m_flow_valves);
	qDeleteAll(m_contacts);
	qDeleteAll(m_relays);
	qDeleteAll(m_coils);
	qDeleteAll(m_counters);
	qDeleteAll(m_plc_channels);
	qDeleteAll(m_plcs);
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
		for (Element *e : d->elements())
			for (Terminal *t : e->terminals())
				if (!parent.contains(t)) parent.insert(t, t);
	}
	for (Diagram *d : diagrams) {
		for (Conductor *c : d->conductors()) {
			join(c->terminal1, c->terminal2);
			connected << c->terminal1 << c->terminal2;
		}
		// folio reports join the nets on both pages
		for (Element *e : d->elements()) {
			if (!(e->linkType() & Element::AllReport)) continue;
			for (Element *other : e->linkedElements())
				if (!e->terminals().isEmpty() && !other->terminals().isEmpty()) {
					join(e->terminals().first(), other->terminals().first());
					connected << e->terminals().first() << other->terminals().first();
				}
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
	m_p.fill(P_ATM, static_cast<int>(index.size()));
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
		{QStringLiteral("flow_control_valve.flow_divider"), 100}};

	for (Diagram *d : diagrams) {
		for (Element *e : d->elements()) {
			const ElementData data = e->elementData();
			const FluidData &f = data.m_fluid;
			const QString fn = f.m_function;
			const bool hyd = medium(e) == Fluid::Medium::Hydraulic;
			if (!f.isNull() && f.m_kind != FluidData::Kind::Solenoid) {
				if (f.m_kind == FluidData::Kind::Source && fn == QLatin1String("supply.tank")) {
					for (Terminal *t : e->terminals())
						if (node(t) >= 0) m_fixed.insert(node(t), P_ATM);
				} else if (f.m_kind == FluidData::Kind::Source && hyd && fn.contains(QLatin1String("pump"))) {
					auto *p = new Pump;
					p->e = e;
					p->q = param(f, QStringLiteral("displacement"), 8.0) * 1e-6
						 * param(f, QStringLiteral("rpm"), 1450) / 60.0;
					const double pmax = param(f, QStringLiteral("pMax"), 6.0);
					p->p_max = P_ATM + (pmax < 100 ? pmax * 1e6 : pmax * 1e5);
					m_pumps << p;
				} else if (f.m_kind == FluidData::Kind::Source) {
					for (Terminal *t : e->terminals())
						if (node(t) >= 0) m_fixed.insert(node(t), p_supply);
				} else if (f.m_kind == FluidData::Kind::Valve && !f.m_positions.isEmpty()) {
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
					v->c = conductanceFromQnn(param(f, QStringLiteral("qNN"), 60.0));
					v->rt = param(f, QStringLiteral("rT"), 0.1);
					v->t_on = param(f, QStringLiteral("onDelay"), 0.02);
					v->t_off = param(f, QStringLiteral("offDelay"), 0.03);
					v->hydraulic = hyd;
					m_valves << v;
				} else if (f.m_kind == FluidData::Kind::Cylinder) {
					auto *c = new Cylinder;
					c->e = e;
					c->label = label(e);
					const double dp = param(f, QStringLiteral("d_piston"), f.m_bore_mm > 0 ? f.m_bore_mm : 20.0) / 1000.0;
					const double dr = param(f, QStringLiteral("d_rod"), f.m_rod_mm > 0 ? f.m_rod_mm : 8.0) / 1000.0;
					c->stroke = param(f, QStringLiteral("xMax"), f.m_stroke_mm > 0 ? f.m_stroke_mm : 100.0) / 1000.0;
					c->a_k = M_PI * dp * dp / 4;
					c->a_r = c->a_k - M_PI * dr * dr / 4;
					c->m = param(f, QStringLiteral("m_cyl"), hyd ? 2.0 : 0.1) + param(f, QStringLiteral("m_ext"), 0.0);
					c->f_static = param(f, QStringLiteral("F_stat"), 0.05 * c->a_k * 6e5);
					c->f_slide = 0.7 * c->f_static;
					c->f_spring = f.m_spring != FluidData::SpringSide::None ? 0.15 * c->a_k * 6e5 : 0.0;
					c->f_load = param(f, QStringLiteral("F_extern_const"), 0.0);
					c->x = param(f, QStringLiteral("xStart"), 0.0) / 1000.0;
					m_cylinders << c;
					m_end_position.insert(c->label, QString());
				} else if (flow_qnn.contains(fn)) {
					auto *fv = new FlowValve;
					fv->e = e;
					fv->function = fn;
					for (Terminal *t : e->terminals()) fv->ports << t->name();
					fv->free_from = f.parameter(QStringLiteral("free_from"));
					fv->free_to = f.parameter(QStringLiteral("free_to"));
					fv->hydraulic = hyd;
					fv->c = conductanceFromQnn(param(f, QStringLiteral("qNN"), flow_qnn.value(fn)));
					fv->c_free = conductanceFromQnn(150);
					fv->rt = param(f, QStringLiteral("rT"), fn.contains(QLatin1String("flow_control")) ? 1.0 : 0.1);
					fv->p0 = param(f, QStringLiteral("p0"), 0.0) * 1e6;
					fv->dp_end = std::max(param(f, QStringLiteral("dpEnd"), 0.05) * 1e6, 0.1e5);
					m_flow_valves << fv;
				}
				continue;
			}

			// electric parts
			const QString path = e->location().toString().toLower();
			const QString name = e->name().toLower();
			QList<int> n;
			for (Terminal *t : e->terminals()) n << node(t);
			while (n.size() < 2) n << -1;
			const bool nc_name = name.contains(QStringLiteral("öffner")) || name.contains(QLatin1String(" nc"))
				|| name.contains(QLatin1String("(nc)")) || path.endsWith(QLatin1String("_nf.elmt"))
				|| path.endsWith(QLatin1String("_nc.elmt"));
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
	// relay contacts find their relay through the link
	for (Contact *c : m_contacts) {
		if (c->kind != QLatin1String("relay")) continue;
		const auto linked = c->e->linkedElements();
		for (Relay *r : m_relays)
			if (!linked.isEmpty() && linked.first() == r->e) c->relay = r;
	}
	for (auto it = m_fixed.constBegin(); it != m_fixed.constEnd(); ++it)
		m_p[it.key()] = it.value();
	electrics();
}

// ------------------------------------------------------------------------- electrics
bool SimulationModel::contactClosed(const Contact &c) const
{
	bool actuated = false;
	if (c.kind == QLatin1String("button")) {
		actuated = m_pressed.contains(label(c.e));
	} else if (c.kind == QLatin1String("limit")) {
		const QStringList parts = c.trigger.split(QLatin1Char(' '), Qt::SkipEmptyParts);
		actuated = parts.size() == 2 && m_end_position.value(parts.at(0)) == parts.at(1);
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
	if (m_sources.isEmpty() && m_plc_channels.isEmpty()) return;
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
	if (!m_plcs.isEmpty() && m_time >= m_next_plc_cycle - 1e-12) {
		m_next_plc_cycle = m_time + 0.010;
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

// ------------------------------------------------------------------------- step
void SimulationModel::step(double dt)
{
	const int n = static_cast<int>(m_p.size());
	QVector<double> flow(n, 0.0), dvdt(n, 0.0), vol(n, V_LINE);
	for (int k = 0; k < n; ++k)
		if (m_hydraulic.contains(k)) vol[k] = V_HYD;
	auto p = [&](int k) { return k >= 0 ? m_p.at(k) : P_ATM; };
	auto book = [&](int a, int b, double m) {
		if (a >= 0) flow[a] -= m;
		if (b >= 0) flow[b] += m;
	};

	electrics();

	for (Pump *pu : m_pumps) {
		const int kp = node(pu->e, QStringLiteral("P")), kt = node(pu->e, QStringLiteral("T"));
		if (kp < 0) continue;
		const double share = qBound(0.0, (pu->p_max - m_p.at(kp)) / pu->band, 1.0);
		flow[kp] += pu->q * share;
		if (kt >= 0) flow[kt] -= pu->q * share;
	}

	for (Valve *v : m_valves) {
		for (int s = 0; s < 2; ++s) {
			const auto &a = v->side[s];
			if (a.pilot && !a.types.contains(QLatin1String("solenoid"))) {
				const int k = node(v->e, s == 0 ? QStringLiteral("14") : QStringLiteral("12"));
				if (k >= 0) v->actuated[s] = m_p.at(k) - P_ATM > P_PILOT;
			}
		}
		const int want = v->wanted();
		if (want != v->target) {
			v->target = want;
			v->switch_at = m_time + (want != v->rest ? v->t_on : v->t_off);
		}
		if (v->switch_at >= 0 && m_time >= v->switch_at) {
			if (v->position != v->target)
				m_messages << Message{m_time, QStringLiteral("%1 schaltet in Stellung %2").arg(v->label).arg(v->target + 1)};
			v->position = v->target;
			v->switch_at = -1;
		}
		for (const auto &path : v->positions.at(v->position)) {
			const int ka = node(v->e, path.first), kb = node(v->e, path.second);
			const double m = v->hydraulic ? volumeFlow(p(ka), p(kb), v->rt) : massFlow(p(ka), p(kb), v->c);
			book(ka, kb, m);
		}
	}

	for (FlowValve *s : m_flow_valves) {
		QHash<QString, int> kn;
		for (const QString &port : s->ports) kn.insert(port, node(s->e, port));
		const QString &fn = s->function;
		if (fn == QLatin1String("supply.silencer")) {
			for (int k : kn) book(k, -1, s->flow(p(k), P_ATM));
		} else if (fn == QLatin1String("flow_control_valve.throttle_valve")
				   || fn == QLatin1String("shutoff_valve.shutoff_valve")) {
			if (s->ports.size() >= 2) {
				const int a = kn.value(s->ports.at(0)), b = kn.value(s->ports.at(1));
				book(a, b, s->flow(p(a), p(b)));
			}
		} else if (fn == QLatin1String("shutoff_valve.check_valve")
				   || fn == QLatin1String("flow_control_valve.one_way_flow_control_valve")) {
			if (!kn.contains(s->free_from) || !kn.contains(s->free_to)) continue;
			const int a = kn.value(s->free_from), b = kn.value(s->free_to);
			if (fn == QLatin1String("flow_control_valve.one_way_flow_control_valve"))
				book(a, b, s->flow(p(a), p(b)));
			bool unlocked = false;
			for (const QString &port : s->ports)
				if (port != s->free_from && port != s->free_to && kn.value(port) >= 0)
					unlocked = p(kn.value(port)) - P_ATM > P_PILOT;
			if (p(a) - p(b) > s->p0 || unlocked)
				book(a, b, s->flow(p(a), p(b), 1.0, true));
		} else if (fn == QLatin1String("flow_control_valve.flow_divider")) {
			// inlet 1 (or P) feeds every other port through equal conductances
			const QString in_port = kn.contains(QStringLiteral("1")) ? QStringLiteral("1")
				: (kn.contains(QStringLiteral("P")) ? QStringLiteral("P") : s->ports.value(0));
			const int in = kn.value(in_port, -1);
			for (const QString &port : s->ports)
				if (port != in_port)
					book(in, kn.value(port), s->flow(p(in), p(kn.value(port))));
		} else if (fn == QLatin1String("shutoff_valve.quick_exhaust_valve")) {
			const int in = kn.value(QStringLiteral("1"), -1), out = kn.value(QStringLiteral("2"), -1);
			if (p(in) >= p(out)) book(in, out, s->flow(p(in), p(out), 1.0, true));
			else book(out, -1, s->flow(p(out), P_ATM));
		} else if (fn == QLatin1String("pressure_valve.pressure_relief_valve01")) {
			int k = -1;
			for (int x : kn) if (x >= 0) { k = x; break; }
			if (k >= 0)
				book(k, -1, s->flow(p(k), P_ATM, qBound(0.0, (p(k) - P_ATM - s->p0) / s->dp_end, 1.0)));
		} else if (fn == QLatin1String("pressure_valve.pressure_regulator")) {
			const int in = kn.value(QStringLiteral("1"), kn.value(QStringLiteral("P"), -1));
			const int out = kn.value(QStringLiteral("2"), kn.value(QStringLiteral("A"), -1));
			if (in < 0 || out < 0) continue;
			const double p2 = p(out) - P_ATM;
			book(in, out, s->flow(p(in), p(out), qBound(0.0, (s->p0 - p2) / s->dp_end, 1.0)));
			book(out, -1, s->flow(p(out), P_ATM, qBound(0.0, (p2 - s->p0 - s->dp_end) / s->dp_end, 1.0)));
		}
	}

	for (Cylinder *z : m_cylinders) {
		const int ka = node(z->e, QStringLiteral("A")), kb = node(z->e, QStringLiteral("B"));
		const double f = (p(ka) - P_ATM) * z->a_k - (p(kb) - P_ATM) * z->a_r - z->f_load - z->f_spring;
		SimPhysics::pistonStep(*z, f, dt);
		if (ka >= 0) { vol[ka] += V_DEAD + z->a_k * z->x; dvdt[ka] += z->a_k * z->v; }
		if (kb >= 0) { vol[kb] += V_DEAD + z->a_r * (z->stroke - z->x); dvdt[kb] -= z->a_r * z->v; }
		const QString end = z->x <= 0.0005 ? QStringLiteral("hinten")
			: (z->x >= z->stroke - 0.0005 ? QStringLiteral("vorne") : QString());
		if (end != m_end_position.value(z->label)) {
			m_end_position.insert(z->label, end);
			if (!end.isEmpty())
				m_messages << Message{m_time, QStringLiteral("%1 Endlage %2").arg(z->label, end)};
		}
	}

	for (int k = 0; k < n; ++k) {
		if (m_fixed.contains(k) || m_electric.contains(k)) continue;
		if (m_hydraulic.contains(k))
			m_p[k] = SimPhysics::oilNode(m_p.at(k), vol.at(k), dvdt.at(k), flow.at(k), E_OIL, dt, P_ATM - 0.9e5);
		else
			m_p[k] = SimPhysics::gasNode(m_p.at(k), vol.at(k), dvdt.at(k), flow.at(k), R_AIR * T_AIR, dt);
	}
	m_time += dt;
}

void SimulationModel::runUntil(double time, double dt)
{
	while (m_time + dt <= time + 1e-12)
		step(dt);
}

// ------------------------------------------------------------------------- input / state
void SimulationModel::pressButton(const QString &lbl, bool pressed)
{
	if (pressed == m_pressed.contains(lbl)) return;
	if (pressed) m_pressed.insert(lbl); else m_pressed.remove(lbl);
	m_messages << Message{m_time, QStringLiteral("%1 %2").arg(lbl, pressed ? QStringLiteral("gedrückt")
																		 : QStringLiteral("losgelassen"))};
}

bool SimulationModel::actuateValve(const QString &lbl, bool left, bool on)
{
	for (Valve *v : m_valves)
		if (v->label == lbl) {
			v->actuated[left ? 0 : 1] = on;
			m_messages << Message{m_time, QStringLiteral("%1 %2 %3").arg(lbl,
				left ? QStringLiteral("links") : QStringLiteral("rechts"),
				on ? QStringLiteral("betätigt") : QStringLiteral("losgelassen"))};
			return true;
		}
	return false;
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
		out << CylinderState{z->e, z->label, z->x * 1000.0, z->stroke * 1000.0, z->v};
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
		.arg(m_counters.size()).arg(m_plcs.size()).arg(m_plc_channels.size());
}

QList<QPair<Element *, QString>> SimulationModel::elementStates() const
{
	QList<QPair<Element *, QString>> out;
	for (Cylinder *z : m_cylinders)
		out << qMakePair(z->e, QStringLiteral("%1: %2 / %3 mm%4").arg(z->label)
			.arg(z->x * 1000.0, 0, 'f', 0).arg(z->stroke * 1000.0, 0, 'f', 0)
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
	return out;
}

double SimulationModel::plcValue(const QString &name) const
{
	return m_plc_vars.value(PlcProgram::normalized(name), 0.0);
}

double SimulationModel::pressureBar(Terminal *t) const
{
	const int k = node(t);
	if (k < 0 || m_electric.contains(k)) return std::numeric_limits<double>::quiet_NaN();
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
