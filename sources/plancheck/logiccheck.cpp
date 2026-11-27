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
// SPDX-License-Identifier: GPL-2.0-or-later
#include "logiccheck.h"

#include "../cable/cable.h"
#include "../diagram.h"
#include "../diagramcontent.h"
#include "../duplicatelabels.h"
#include "../qet.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/terminal.h"
#include "../qetgraphicsitem/terminalelement.h"
#include "../qetproject.h"

#include <QHash>
#include <QLocale>
#include <QMap>
#include <QRegularExpression>

#include <algorithm>
#include <cmath>
#include <functional>

namespace {

using PlanCheck::Finding;
using Severity = ProjectCheck::Severity;

QString labelOf(Element *e)
{
	return DuplicateLabels::displayedLabel(e).simplified();   // "EDF↵Type S" in one line
}

QString shownOf(Element *e)
{
	const QString label = labelOf(e);
	return label.isEmpty() ? e->name() : label;
}

QString infoOf(Element *e, const char *key)
{
	return e->elementInformations().value(QLatin1String(key)).toString().trimmed();
}

	/// a number at the start of @p text ("16", "0,35 Ohm"), 0 when none
double leadingNumber(QString text)
{
	text = text.trimmed();
	text.replace(QLatin1Char(','), QLatin1Char('.'));
	static const QRegularExpression re(QStringLiteral("^\\+?\\d+(?:\\.\\d+)?"));
	const QRegularExpressionMatch m = re.match(text);
	return m.hasMatch() ? m.captured(0).toDouble() : 0;
}

double valueOf(Element *e, const char *key)
{
	return leadingNumber(infoOf(e, key));
}

QString fmt(double v, int digits = 3)
{
	return QLocale().toString(v, 'g', digits);
}

bool isPanelDevice(Element *e)
{
	return !e->elementInformations().value(QStringLiteral("panel_of")).toString().isEmpty();
}

	/// the texts of a device a datum may be written in
QString describedOf(Element *e)
{
	return QStringList{infoOf(e, "description"), infoOf(e, "designation"), infoOf(e, "comment"),
					   infoOf(e, "manufacturer_reference"), labelOf(e)}.join(QLatin1Char(' '));
}

void add(QVector<Finding> &out, Severity severity, const char *rule, Diagram *folio,
		 const QString &label, const QString &text, const QList<QUuid> &items, bool incomplete = false)
{
	Finding f;
	f.severity = severity;
	f.rule = QString::fromLatin1(rule);
	f.eplan = QStringLiteral("");
	f.folio = folio;
	f.label = label;
	f.text = text;
	f.items = items;
	f.incomplete = incomplete;
	out << f;
}

	/// a terminal of a strip drawn as a plain symbol: named so in any of the
	/// collection's languages, or kept in its folder of terminals
bool isPlainTerminal(Element *e)
{
	static const QRegularExpression re(QStringLiteral(
		"^(klemme|klemmblock|reihenklemme|durchgangsklemme|terminal|terminal block|borne|borne continuit[ée])\\b"),
		QRegularExpression::CaseInsensitiveOption);
	return e->terminals().size() >= 2
		&& (e->name().trimmed().contains(re)
			|| e->location().toString().contains(QLatin1String("130_terminals_terminal_strips")));
}

/* ---- the potentials of the plan ------------------------------------- */

	/// Wires, junctions, folio reports and the two sides of a terminal
	/// block make one potential (net); every other symbol is a device
class Nets
{
	public:
		Nets(const QList<Diagram *> &diagrams, const QSet<Diagram *> &panel)
		{
			for (Diagram *d : diagrams) {
				if (panel.contains(d)) continue;
				for (Conductor *w : d->content().conductors()) {
					unite(w->terminal1, w->terminal2);
					m_wires << w;
				}
				for (Element *e : d->elements()) {
					if (isPanelDevice(e)) continue;
					const ElementData data = e->elementData();
					if (data.m_type == ElementData::Thumbnail || data.m_type == ElementData::ConductorDefinition)
						continue;
					const QList<Terminal *> own = e->terminals();
					if ((data.m_type & (ElementData::AllReport | ElementData::Junction | ElementData::Terminal))
						|| dynamic_cast<TerminalElement *>(e)) {
						for (int i = 1; i < own.size(); ++i) unite(own.first(), own.at(i));
						if (data.m_type & ElementData::AllReport)
							for (Element *other : e->linkedElements())
								if (!own.isEmpty() && !other->terminals().isEmpty())
									unite(own.first(), other->terminals().first());
						continue;
					}
						// a terminal drawn with an old or plain symbol (no
						// link type: "Borne continuité", "Klemmblock"): its
						// terminals one above the other are one potential
					if (data.m_type == ElementData::Simple && isPlainTerminal(e)) {
						QMap<int, QList<Terminal *>> column;
						for (Terminal *t : own) column[qRound(e->mapFromScene(t->dockConductor()).x())] << t;
						for (const QList<Terminal *> &c : std::as_const(column))
							for (int i = 1; i < c.size(); ++i) unite(c.first(), c.at(i));
						continue;
					}
					m_devices << qMakePair(d, e);
				}
			}
			for (Conductor *w : std::as_const(m_wires)) {
				const int n = of(w->terminal1);
				m_net_wires[n] << w;
				const ConductorProperties p = w->properties();
				for (const QString &name : {p.m_potential_type, p.m_function, p.m_tension_protocol})
					if (!name.trimmed().isEmpty()) m_names[n] << name.trimmed();
			}
		}

		int of(Terminal *t)
		{
			if (!t) return -1;
			Terminal *r = root(t);
			auto it = m_id.find(r);
			if (it == m_id.end()) it = m_id.insert(r, m_count++);
			return it.value();
		}

		QList<QPair<Diagram *, Element *>> m_devices;
		QList<Conductor *> m_wires;
		QHash<int, QList<Conductor *>> m_net_wires;
		QHash<int, QStringList> m_names;

	private:
		Terminal *root(Terminal *t)
		{
			Terminal *p = m_parent.value(t, t);
			if (p == t) return t;
			Terminal *r = root(p);
			m_parent.insert(t, r);
			return r;
		}
		void unite(Terminal *a, Terminal *b)
		{
			if (!a || !b) return;
			Terminal *ra = root(a), *rb = root(b);
			if (ra != rb) m_parent.insert(ra, rb);
		}
		QHash<Terminal *, Terminal *> m_parent;
		QHash<Terminal *, int> m_id;
		int m_count = 0;
};

/* ---- contacts and their terminals ------------------------------------ */

	/// two terminals of a device that a contact or a pole joins; kind 1
	/// make (NO), 2 break (NC), 0 not known; directed: a is the input
struct Pair { Terminal *a = nullptr; Terminal *b = nullptr; int kind = 0; bool main = false; bool directed = false; };

	/// the kind of a contact from its function digits (EN 50005, IEC 60947-1
	/// annex L): 1-2 and 5-6 break, 3-4 and 7-8 make; 0 for others
int kindOfDigits(int f1, int f2)
{
	if ((f1 == 1 && f2 == 2) || (f1 == 5 && f2 == 6)) return 2;
	if ((f1 == 3 && f2 == 4) || (f1 == 7 && f2 == 8)) return 1;
	return 0;
}

QList<Pair> pairsOf(Element *e)
{
	const QList<Terminal *> ts = e->terminals();
	QList<Pair> out;
	static const QRegularExpression two(QStringLiteral("^(\\d)(\\d)$"));
	static const QRegularExpression one(QStringLiteral("^([1-8])$"));
		// "IN.1", "OUT.2": the number after the point
	auto nameOf = [](Terminal *t) { return t->name().trimmed().toUpper().section(QLatin1Char('.'), -1); };
		// where a terminal is in the symbol itself, whatever its rotation
	auto local = [e](Terminal *t) { return e->mapFromScene(t->dockConductor()); };

	if (ts.size() == 2) {
		Pair p;
		p.a = ts.at(0);
		p.b = ts.at(1);
		const QRegularExpressionMatch ma = two.match(nameOf(p.a)), mb = two.match(nameOf(p.b));
		const QRegularExpressionMatch sa = one.match(nameOf(p.a)), sb = one.match(nameOf(p.b));
		if (ma.hasMatch() && mb.hasMatch() && ma.captured(1) == mb.captured(1)) {
			int f1 = ma.captured(2).toInt(), f2 = mb.captured(2).toInt();
			if (f1 > f2) {std::swap(p.a, p.b); std::swap(f1, f2);}
			p.kind = kindOfDigits(f1, f2);
		} else if (sa.hasMatch() && sb.hasMatch()) {
			int a = sa.captured(1).toInt(), b = sb.captured(1).toInt();
			if (a > b) {std::swap(p.a, p.b); std::swap(a, b);}
			p.main = true;
			p.directed = a % 2 == 1 && b == a + 1;
		} else if (nameOf(p.a).isEmpty() && nameOf(p.b).isEmpty()) {
				// no names: the upper one is the input
			if (local(p.a).y() > local(p.b).y()) std::swap(p.a, p.b);
			p.directed = true;
		}
		out << p;
		return out;
	}

	QHash<QString, Terminal *> by_name;
	QList<Terminal *> neutral;
	for (Terminal *t : ts) {
		const QString n = nameOf(t);
		if (n == QLatin1String("N") || n == QLatin1String("N'") || n == QLatin1String("N1") || n == QLatin1String("N2"))
			neutral << t;
		by_name.insert(n, t);
	}
		// auxiliary contacts: x1-x2, x3-x4, x5-x6, x7-x8; changeover x1-x2 and x1-x4
	for (int ord = 0; ord <= 9; ++ord)
		for (int f : {1, 3, 5, 7}) {
			Terminal *a = by_name.value(QStringLiteral("%1%2").arg(ord).arg(f));
			Terminal *b = by_name.value(QStringLiteral("%1%2").arg(ord).arg(f + 1));
			if (a && b) {
				Pair p; p.a = a; p.b = b; p.kind = kindOfDigits(f, f + 1);
				out << p;
			}
			if (f == 1 && a && !by_name.value(QStringLiteral("%1%2").arg(ord).arg(3))) {
				if (Terminal *c = by_name.value(QStringLiteral("%1%2").arg(ord).arg(4))) {
					Pair p; p.a = a; p.b = c; p.kind = 1;
					out << p;
				}
			}
		}
		// poles: 1-2, 3-4, 5-6, 7-8 and L1-T1, L2-T2, L3-T3
	for (int a = 1; a <= 7; a += 2) {
		Terminal *in = by_name.value(QString::number(a)), *o = by_name.value(QString::number(a + 1));
		if (in && o) {Pair p; p.a = in; p.b = o; p.main = true; p.directed = true; out << p;}
	}
	for (int k = 1; k <= 3; ++k) {
		Terminal *in = by_name.value(QStringLiteral("L%1").arg(k)), *o = by_name.value(QStringLiteral("T%1").arg(k));
		if (in && o) {Pair p; p.a = in; p.b = o; p.main = true; p.directed = true; out << p;}
	}
	if (neutral.size() == 2) {Pair p; p.a = neutral.at(0); p.b = neutral.at(1); p.main = true; out << p;}

		// terminals left without a pair (no names, as most symbols of the
		// collection): one above the other in the symbol, the upper one the
		// input
	QSet<Terminal *> paired;
	for (const Pair &p : std::as_const(out)) paired << p.a << p.b;
	QMap<int, QList<Terminal *>> by_x;
	for (Terminal *t : ts)
		if (!paired.contains(t)) by_x[qRound(local(t).x())] << t;
	for (const QList<Terminal *> &column : std::as_const(by_x)) {
		if (column.size() != 2) continue;
		Pair p;
		p.a = column.at(0);
		p.b = column.at(1);
		if (local(p.a).y() > local(p.b).y()) std::swap(p.a, p.b);
		p.directed = true;
		out << p;
	}
	return out;
}

	/// the letter of the kind of a device: "-S3" S, "=A1+B2-K4" K
QChar letterOf(Element *e)
{
	QString label = labelOf(e);
	const int dash = int(label.lastIndexOf(QLatin1Char('-')));
	if (dash >= 0) label = label.mid(dash + 1);
	for (const QChar ch : label)
		if (ch.isLetter()) return ch.toUpper();
	return QChar();
}

bool looksBreak(Element *e)
{
	const QString name = e->name().toLower();
	const QString path = e->location().toString().toLower();
	return name.contains(QStringLiteral("öffner")) || name.contains(QLatin1String(" nc")) || name.contains(QLatin1String("(nc)"))
		|| name.contains(QLatin1String("break")) || path.endsWith(QLatin1String("_nf.elmt")) || path.endsWith(QLatin1String("_nc.elmt"));
}

bool looksMake(Element *e)
{
	const QString name = e->name().toLower();
	const QString path = e->location().toString().toLower();
	return name.contains(QStringLiteral("schließer")) || name.contains(QLatin1String("schliesser")) || name.contains(QLatin1String(" no")) || name.contains(QLatin1String("(no)"))
		|| name.contains(QLatin1String("make")) || path.endsWith(QLatin1String("_no.elmt")) || path.endsWith(QLatin1String("_na.elmt"));
}

bool isFuse(Element *e)
{
		// the name of an old embedded symbol may be missing: its file or
		// folder names it too ("zabezpieczenia/odlacznik_bez_1f.elmt")
	static const QRegularExpression re(QStringLiteral("fusible|fusibile|sicherung|bezpiecz|\\bfuse|odlacznik_bez"),
									   QRegularExpression::CaseInsensitiveOption);
	return e->name().contains(re) || e->location().toString().contains(re);
}

bool isRcd(Element *e);

	/// a circuit-breaker or RCD drawn with a plain symbol (link type
	/// "simple", as int_diff5 "Disjoncteur differentiel 1P+N"): a protective
	/// device all the same
bool isPlainProtection(Element *e)
{
	if (e->elementData().m_type != ElementData::Simple || e->terminals().size() < 2) return false;
	static const QRegularExpression re(QStringLiteral(
		"disjonct|circuit.?breaker|leitungsschutz|schutzschalter|motorschutz|protective_gears|residual_current"),
		QRegularExpression::CaseInsensitiveOption);
	return isRcd(e) || e->name().contains(re) || e->location().toString().contains(re);
}

int plcRow(Element *card, Element *channel)
{
	const int row = card->groupIndexForElement(channel);
	return row >= 0 ? row : channel->groupIndexForElement(card);
}

/* ---- the circuit: branches between potentials ------------------------- */

	/// a switching path between two nets; var -1 always closed (a pole of
	/// a protective device, a fuse), else closed while var is true (make)
	/// or false (break, neg)
struct Branch {
	int a = -1, b = -1;
	int var = -1;
	bool neg = false;
	bool timer = false;     ///< a delayed contact of a timer relay
	bool main = false;      ///< a main contact (pole), for the power circuit
	bool directed = false;  ///< a is the input, b the output
	Element *e = nullptr;
	Diagram *d = nullptr;
};

struct Var {
	QString name;
	Element *coil = nullptr;   ///< the coil of a contactor or relay, else an input
	bool plc = false;
};

struct CoilInfo { Diagram *d; Element *e; int var; int n1; int n2; };

class Circuit
{
	public:
		explicit Circuit(Nets &nets) : m_nets(nets)
		{
				// coils first: their contacts point to them
			for (const auto &de : nets.m_devices) {
				Element *e = de.second;
				const ElementData data = e->elementData();
				if (data.m_type != ElementData::Master || data.m_master_type != ElementData::Coil) continue;
				Terminal *a1 = nullptr, *a2 = nullptr;
				for (Terminal *t : e->terminals()) {
					const QString n = t->name().trimmed().toUpper();
					if (n == QLatin1String("A1")) a1 = t;
					else if (n == QLatin1String("A2")) a2 = t;
				}
				if ((!a1 || !a2) && e->terminals().size() == 2) {
					a1 = e->terminals().at(0);
					a2 = e->terminals().at(1);
				}
				if (!a1 || !a2) continue;
				const int var = varFor(QStringLiteral("coil:") + e->uuid().toString(), shownOf(e), e);
				m_coil_var.insert(e, var);
				m_coils << CoilInfo{de.first, e, var, nets.of(a1), nets.of(a2)};
			}

			QSet<int> explicit_plus, explicit_minus, fed;
				// protective devices and fuses: their input and output nets,
				// sorted out below (a supply only when nothing switches in front)
			QList<QPair<QSet<int>, QSet<int>>> feeders;
			for (const auto &de : nets.m_devices) {
				Diagram *d = de.first;
				Element *e = de.second;
				const ElementData data = e->elementData();
				const QList<Terminal *> own = e->terminals();
				auto branch = [&](const Pair &p, int var, bool neg, bool timer, bool main) {
					Branch b;
					b.a = nets.of(p.a);
					b.b = nets.of(p.b);
					if (b.a < 0 || b.b < 0 || b.a == b.b) return;
					b.var = var; b.neg = neg; b.timer = timer; b.main = main || p.main;
					b.directed = p.directed; b.e = e; b.d = d;
					m_branches << b;
				};
				auto freeVar = [&](int i) {
					return varFor(QStringLiteral("free:%1:%2").arg(e->uuid().toString()).arg(i), shownOf(e));
				};

				if (data.m_type == ElementData::Slave && data.m_slave_type == ElementData::PLCSlave) {
					const QList<Element *> linked = e->linkedElements();
					if (linked.isEmpty()) continue;
					Element *card = linked.first();
					const int row = plcRow(card, e);
					const auto ios = card->elementData().plcMasterData().ios;
					if (row < 0 || row >= ios.size()) continue;
					const auto type = ios.at(row).type;
					if (type != ElementData::SortieDigitale && type != ElementData::SortieUniverselle) continue;
					const int var = varFor(QStringLiteral("plc:") + e->uuid().toString(), shownOf(e));
					m_vars[var].plc = true;
					for (Terminal *t : own) m_plc_source.insert(nets.of(t), var);
					continue;
				}
				if (data.m_type == ElementData::Slave) {
					const QList<Element *> linked = e->linkedElements();
					if (linked.isEmpty()) continue;
					Element *master = linked.first();
					const int var = m_coil_var.contains(master)
						? m_coil_var.value(master)
						: varFor(QStringLiteral("in:") + master->uuid().toString(), shownOf(master));
					const bool timer = data.m_slave_type == ElementData::DelayOn || data.m_slave_type == ElementData::DelayOff
									   || data.m_slave_type == ElementData::delayOnOff;
					const bool power = data.m_slave_type == ElementData::Power;
					int i = 0;
					for (const Pair &p : pairsOf(e)) {
						int kind = p.kind;
						if (!kind && data.m_slave_state == ElementData::NO) kind = 1;
						if (!kind && data.m_slave_state == ElementData::NC) kind = 2;
						if (kind) branch(p, var, kind == 2, timer, power);
						else branch(p, freeVar(i), false, false, power);
						++i;
					}
					continue;
				}
				if ((data.m_type == ElementData::Master && data.m_master_type == ElementData::Protection)
					|| isPlainProtection(e)) {
					m_protection << de;
					QSet<int> ins, outs;
					for (const Pair &p : pairsOf(e)) {
						branch(p, -1, false, false, true);
						if (p.directed) {ins << nets.of(p.a); outs << nets.of(p.b);}
					}
					if (outs.isEmpty())
						for (Terminal *t : own) (PlanCheck::isOutput(t, own) ? outs : ins) << nets.of(t);
					feeders << qMakePair(ins, outs);
					continue;
				}
				if (data.m_type == ElementData::Master && data.m_master_type == ElementData::Commutator) {
					const int var = varFor(QStringLiteral("in:") + e->uuid().toString(), shownOf(e));
					int i = 0;
					for (const Pair &p : pairsOf(e)) {
						int kind = p.kind;
						if (!kind) kind = looksBreak(e) ? 2 : (looksMake(e) ? 1 : 0);
						if (kind) branch(p, var, kind == 2, false, true);
						else branch(p, freeVar(i), false, false, true);
						++i;
					}
					continue;
				}
				if (data.m_type != ElementData::Simple) continue;

				const QString role = infoOf(e, "sim_role");
				if (role == QLatin1String("supply+") || role == QLatin1String("supply-")) {
					for (Terminal *t : own) (role.endsWith(QLatin1Char('+')) ? explicit_plus : explicit_minus) << nets.of(t);
					continue;
				}
				if (role == QLatin1String("voltage")) {
					for (Terminal *t : own) {
						const QString tn = t->name().trimmed().toLower();
						(tn == QLatin1String("p") || tn == QLatin1String("+") || tn == QLatin1String("l+")
							 ? explicit_plus : explicit_minus) << nets.of(t);
					}
					continue;
				}
				const QChar letter = letterOf(e);
				if (role.startsWith(QLatin1String("button")) || role.startsWith(QLatin1String("switch"))
					|| role.startsWith(QLatin1String("limit")) || letter == QLatin1Char('S') || letter == QLatin1Char('B')) {
					const int var = varFor(QStringLiteral("in:") + e->uuid().toString(), shownOf(e));
					int i = 0;
					for (const Pair &p : pairsOf(e)) {
						int kind = p.kind;
						if (role.endsWith(QLatin1String("_nc"))) kind = 2;
						else if (!role.isEmpty() && !kind) kind = 1;
						if (!kind && own.size() == 2) kind = looksBreak(e) ? 2 : (looksMake(e) ? 1 : 0);
						if (kind) branch(p, var, kind == 2, false, false);
						else branch(p, freeVar(i), false, false, false);
						++i;
					}
					continue;
				}
				if (own.size() == 2 && (letter == QLatin1Char('F') || isFuse(e))) {
					Pair p; p.a = own.at(0); p.b = own.at(1);
					branch(p, -1, false, false, true);
						// no direction: either side may be the input
					feeders << qMakePair(QSet<int>{nets.of(own.at(0))}, QSet<int>{nets.of(own.at(1))});
					feeders << qMakePair(QSet<int>{nets.of(own.at(1))}, QSet<int>{nets.of(own.at(0))});
				}
			}

				// a protective device feeds when no relay contact and no coil is at its
				// input: one drawn inside a control path (the auxiliary contact
				// of a motor protection, Q4 in a coil circuit) is no supply
			QSet<int> switched_or_coil;
			for (const Branch &b : std::as_const(m_branches))
				if (b.var >= 0 && m_vars.at(b.var).coil) switched_or_coil << b.a << b.b;
			for (const CoilInfo &c : std::as_const(m_coils)) switched_or_coil << c.n1 << c.n2;
			for (const auto &f : std::as_const(feeders)) {
				bool clean = true;
				for (int n : f.first) if (switched_or_coil.contains(n)) clean = false;
				if (clean) fed += f.second;
			}

				// the rails of the control circuit: given ones (sim_role,
				// names of the potentials) first, else the outputs of the
				// protective devices and the common return of the coils
			static const QRegularExpression plus_name(QStringLiteral(
				"^(L\\+|\\+|\\+?24 ?V( ?DC)?|\\+?230 ?V( ?AC)?|L1|L2|L3|L|P)$"), QRegularExpression::CaseInsensitiveOption);
			static const QRegularExpression minus_name(QStringLiteral(
				"^(M|0 ?V( ?DC)?|N|L-|-|GND)$"), QRegularExpression::CaseInsensitiveOption);
			for (auto it = nets.m_names.cbegin(); it != nets.m_names.cend(); ++it)
				for (const QString &name : it.value()) {
					if (name.contains(plus_name)) explicit_plus << it.key();
					if (name.contains(minus_name)) explicit_minus << it.key();
				}
			m_given = !explicit_plus.isEmpty() || !explicit_minus.isEmpty();
			m_fed = fed;
			m_fed.remove(-1);
			m_plus = explicit_plus.isEmpty() ? fed : explicit_plus;
			m_minus = explicit_minus;
			if (m_minus.isEmpty()) {
					// every common return: a net with two coils or more and no
					// contact (which side of a coil is A2 is not always named);
					// a single coil: its side without a contact
				QSet<int> switched;
				for (const Branch &b : std::as_const(m_branches)) switched << b.a << b.b;
				QHash<int, int> count;
				for (const CoilInfo &c : std::as_const(m_coils)) {
					++count[c.n1];
					if (c.n2 != c.n1) ++count[c.n2];
				}
				for (auto it = count.cbegin(); it != count.cend(); ++it)
					if (it.key() >= 0 && it.value() >= 2 && !switched.contains(it.key())) m_minus << it.key();
				if (m_coils.size() == 1) {
					const CoilInfo &c = m_coils.first();
					if (c.n2 >= 0 && !switched.contains(c.n2)) m_minus << c.n2;
					else if (c.n1 >= 0 && !switched.contains(c.n1)) m_minus << c.n1;
				}
			}
			m_plus.subtract(m_minus);
			m_plus.remove(-1);

			for (int i = 0; i < m_branches.size(); ++i) {
				m_adj[m_branches.at(i).a] << i;
				m_adj[m_branches.at(i).b] << i;
			}
		}

		int varFor(const QString &key, const QString &name, Element *coil = nullptr)
		{
			auto it = m_var_of.find(key);
			if (it != m_var_of.end()) return it.value();
			m_vars << Var{name, coil, false};
			m_var_of.insert(key, int(m_vars.size()) - 1);
			return int(m_vars.size()) - 1;
		}

		Nets &m_nets;
		QVector<Branch> m_branches;
		QVector<Var> m_vars;
		QVector<CoilInfo> m_coils;
		QHash<Element *, int> m_coil_var;
		QHash<QString, int> m_var_of;
		QSet<int> m_plus, m_minus;
		QSet<int> m_fed;        ///< outputs of protective devices and fuses
		bool m_given = false;   ///< rails given (sim_role, names of potentials)
		QHash<int, int> m_plc_source;   ///< net of a PLC output -> its variable
		QList<QPair<Diagram *, Element *>> m_protection;
		QHash<int, QVector<int>> m_adj;  ///< net -> branches
};

/* ---- the condition of a coil as a sum of products --------------------- */

	/// a product of literals (var * 2 + neg), sorted; via: the contacts
struct Term {
	QVector<int> lits;
	QVector<Element *> via;
	bool timer = false;
};

bool addLit(Term &t, int lit)
{
	if (t.lits.contains(lit ^ 1)) return false;
	if (!t.lits.contains(lit)) {
		t.lits << lit;
		std::sort(t.lits.begin(), t.lits.end());
	}
	return true;
}

bool joinTerms(Term &t, const Term &o)
{
	for (int lit : o.lits)
		if (!addLit(t, lit)) return false;
	for (Element *e : o.via)
		if (!t.via.contains(e)) t.via << e;
	t.timer = t.timer || o.timer;
	return true;
}

	/// the coil circuits: paths through at most kMaxDepth branches, at
	/// most kMaxTerms products; beyond, the coil is "not followed"
constexpr int kMaxDepth = 24;
constexpr int kMaxTerms = 512;

class CoilLogic
{
	public:
		explicit CoilLogic(Circuit &c) : m_c(c) {}

			/// every simple path from @p start to a net of @p targets, not
			/// through @p forbidden; toward the plus rail a PLC output counts
		QList<Term> paths(int start, const QSet<int> &targets, const QSet<int> &forbidden,
						  bool to_plus, bool *overflow)
		{
			QList<Term> out;
			QSet<int> visited;
			Term cur;
			std::function<void(int, int)> walk = [&](int net, int depth) {
				if (*overflow) return;
				if (targets.contains(net)) {out << cur; return;}
				if (forbidden.contains(net)) return;
				if (to_plus && m_c.m_plc_source.contains(net)) {
					Term t = cur;
					if (addLit(t, m_c.m_plc_source.value(net) * 2)) out << t;
				}
				if (out.size() > kMaxTerms || depth > kMaxDepth) {*overflow = true; return;}
				visited << net;
				for (int i : m_c.m_adj.value(net)) {
					const Branch &b = m_c.m_branches.at(i);
					const int other = b.a == net ? b.b : b.a;
					if (visited.contains(other)) continue;
					const Term keep = cur;
					if (b.var >= 0) {
						if (!addLit(cur, b.var * 2 + (b.neg ? 1 : 0))) {cur = keep; continue;}
						if (!cur.via.contains(b.e)) cur.via << b.e;
						cur.timer = cur.timer || b.timer;
					}
					walk(other, depth + 1);
					cur = keep;
				}
				visited.remove(net);
			};
			if (start >= 0) walk(start, 0);
			return out;
		}

		static QList<Term> product(const QList<Term> &x, const QList<Term> &y)
		{
			QList<Term> out;
			for (const Term &a : x)
				for (const Term &b : y) {
					Term t = a;
					if (joinTerms(t, b)) out << t;
				}
			return out;
		}

			/// drop every product that holds a smaller one
		static QList<Term> minimal(QList<Term> terms)
		{
			std::sort(terms.begin(), terms.end(), [](const Term &a, const Term &b) {return a.lits.size() < b.lits.size();});
			QList<Term> kept;
			for (const Term &t : std::as_const(terms)) {
				bool covered = false;
				for (const Term &k : std::as_const(kept))
					if (std::includes(t.lits.begin(), t.lits.end(), k.lits.begin(), k.lits.end())) {covered = true; break;}
				if (!covered) kept << t;
			}
			return kept;
		}

			/// when coil @p i carries the voltage; false when it could not
			/// be followed to the end
		bool condition(int i, QList<Term> *out)
		{
			const CoilInfo &coil = m_c.m_coils.at(i);
			bool overflow = false;
				// rails not given: a coil side right on a common return or on
				// the output of a protective device (a fused neutral, 230N) is
				// this coil's return; the other side must reach another rail
			if (!m_c.m_given) {
				int ret = -1;
				if (m_c.m_minus.contains(coil.n2) || m_c.m_fed.contains(coil.n2)) ret = coil.n2;
				else if (m_c.m_minus.contains(coil.n1) || m_c.m_fed.contains(coil.n1)) ret = coil.n1;
				if (ret >= 0) {
					QSet<int> targets = m_c.m_plus + m_c.m_minus + m_c.m_fed;
					targets.remove(ret);
					*out = minimal(paths(ret == coil.n2 ? coil.n1 : coil.n2, targets, {ret}, true, &overflow));
					return !overflow;
				}
			}
			const QList<Term> a = product(paths(coil.n1, m_c.m_plus, m_c.m_minus, true, &overflow),
										  paths(coil.n2, m_c.m_minus, m_c.m_plus, false, &overflow));
			const QList<Term> b = product(paths(coil.n1, m_c.m_minus, m_c.m_plus, false, &overflow),
										  paths(coil.n2, m_c.m_plus, m_c.m_minus, true, &overflow));
			*out = minimal(a + b);
			return !overflow;
		}

	private:
		Circuit &m_c;
};

	/// the devices where the circuit of @p coil ends without a rail: wired
	/// to it through contacts, but neither a contact, a coil nor a supply
	/// (a safety relay, a converter): the reader learns where data is missing
QStringList boundaryOf(Circuit &c, const CoilInfo &coil)
{
	QSet<Element *> known;
	for (const Branch &b : std::as_const(c.m_branches)) known << b.e;
	for (auto it = c.m_coil_var.cbegin(); it != c.m_coil_var.cend(); ++it) known << it.key();
	const QSet<int> rails = c.m_plus + c.m_minus + c.m_fed;
	QSet<int> seen;
	QList<int> todo;
	for (int n : {coil.n1, coil.n2})
		if (n >= 0 && !rails.contains(n)) {seen << n; todo << n;}
	while (!todo.isEmpty()) {
		const int n = todo.takeLast();
		for (int i : c.m_adj.value(n)) {
			const Branch &b = c.m_branches.at(i);
			const int other = b.a == n ? b.b : b.a;
			if (!seen.contains(other) && !rails.contains(other)) {seen << other; todo << other;}
		}
	}
	QStringList out;
	for (const auto &de : std::as_const(c.m_nets.m_devices)) {
		Element *e = de.second;
		if (known.contains(e) || e->terminals().size() < 2) continue;
		bool touches = false;
		for (Terminal *t : e->terminals()) if (seen.contains(c.m_nets.of(t))) touches = true;
		const QString name = shownOf(e);
		if (touches && !out.contains(name)) out << name;
		if (out.size() >= 4) break;
	}
	return out;
}

	/// whether every product holds @p lit
bool everyTermHas(const QList<Term> &terms, int lit)
{
	if (terms.isEmpty()) return false;
	for (const Term &t : terms)
		if (!t.lits.contains(lit)) return false;
	return true;
}

	/// "S2 actuated, K1 off": the state a product asks for
QString stateText(const Circuit &c, const QVector<int> &lits, const QSet<int> &skip)
{
	QStringList parts;
	for (int lit : lits) {
		const int var = lit / 2;
		if (skip.contains(var)) continue;
		const Var &v = c.m_vars.at(var);
		const bool on = !(lit & 1);
		if (v.coil || v.plc)
			parts << (on ? ProjectCheck::tr("%1 on") : ProjectCheck::tr("%1 off")).arg(v.name);
		else
			parts << (on ? ProjectCheck::tr("%1 actuated") : ProjectCheck::tr("%1 not actuated")).arg(v.name);
	}
	return parts.isEmpty() ? ProjectCheck::tr("the supply alone") : parts.join(QStringLiteral(", "));
}

/* ---- the power circuit: which contactors must exclude each other ------- */

class Dsu
{
	public:
		int find(int x)
		{
			const int p = m_p.value(x, x);
			if (p == x) return x;
			const int r = find(p);
			m_p.insert(x, r);
			return r;
		}
		void unite(int a, int b) {a = find(a); b = find(b); if (a != b) m_p.insert(a, b);}
	private:
		QHash<int, int> m_p;
};

	/// the pairs of contactors (coil variables) whose main contacts, both
	/// closed, join two potentials of the supply that neither joins alone
QList<QPair<int, int>> exclusivePairs(Circuit &c)
{
	QHash<int, QVector<int>> main_of;   // coil var -> main branches
	for (int i = 0; i < c.m_branches.size(); ++i) {
		const Branch &b = c.m_branches.at(i);
		if (b.var >= 0 && b.main && c.m_vars.at(b.var).coil) main_of[b.var] << i;
	}
	QList<int> contactors = main_of.keys();
	std::sort(contactors.begin(), contactors.end());
	if (contactors.size() < 2) return {};

		// the supply: the nets of protective devices, else the inputs of
		// the main contacts
	QSet<int> sources;
	for (const auto &de : std::as_const(c.m_protection))
		for (Terminal *t : de.second->terminals()) sources << c.m_nets.of(t);
	QSet<int> main_nets;
	for (const auto &list : std::as_const(main_of))
		for (int i : list) main_nets << c.m_branches.at(i).a << c.m_branches.at(i).b;
	Dsu all;
	for (const Branch &b : std::as_const(c.m_branches)) all.unite(b.a, b.b);
	QSet<int> main_roots;
	for (int n : std::as_const(main_nets)) main_roots << all.find(n);
	QList<int> src;
	for (int s : std::as_const(sources))
		if (main_roots.contains(all.find(s))) src << s;
	if (src.isEmpty())
		for (const auto &list : std::as_const(main_of))
			for (int i : list)
				if (c.m_branches.at(i).directed && !src.contains(c.m_branches.at(i).a)) src << c.m_branches.at(i).a;

		// the potentials of the supply: with every contactor open, the parts
		// of the power circuit that hold a source and no output of a main
		// contact (behind a contactor is not supply: two contactors in a row
		// join nothing). Two of them joined is a short circuit.
	Dsu open;
	for (const Branch &b : std::as_const(c.m_branches))
		if (b.var < 0 || !c.m_vars.at(b.var).coil) open.unite(b.a, b.b);
	QSet<int> behind;
	for (const auto &list : std::as_const(main_of))
		for (int i : list) behind << open.find(c.m_branches.at(i).b);
	QList<int> supply;   // one net per potential of the supply
	QSet<int> supply_roots;
	for (int s : std::as_const(src)) {
		const int root = open.find(s);
		if (behind.contains(root) || supply_roots.contains(root)) continue;
		supply_roots << root;
		supply << s;
	}
	if (supply.size() < 2) return {};
	auto shorts = [&](const QSet<int> &closed) {
		Dsu d;
		for (const Branch &b : std::as_const(c.m_branches))
			if (b.var < 0 || !c.m_vars.at(b.var).coil || (b.main && closed.contains(b.var))) d.unite(b.a, b.b);
		for (int i = 0; i < supply.size(); ++i)
			for (int j = i + 1; j < supply.size(); ++j)
				if (d.find(supply.at(i)) == d.find(supply.at(j))) return true;
		return false;
	};
		// line contactors: a main contact on the supply itself; they may be
		// on while the pair is checked (the line contactor of a star-delta
		// starter), every other contactor is off
	QList<int> line;
	for (int var : std::as_const(contactors))
		for (int i : main_of.value(var))
			if ((supply_roots.contains(open.find(c.m_branches.at(i).a)) || supply_roots.contains(open.find(c.m_branches.at(i).b)))
				&& !line.contains(var))
				line << var;

	QList<QPair<int, int>> out;
	for (int x = 0; x < contactors.size(); ++x)
		for (int y = x + 1; y < contactors.size(); ++y) {
			const int A = contactors.at(x), B = contactors.at(y);
			if (all.find(c.m_branches.at(main_of.value(A).first()).a) != all.find(c.m_branches.at(main_of.value(B).first()).a))
				continue;   // apart in the power circuit
			QList<int> others;
			for (int var : std::as_const(line))
				if (var != A && var != B && all.find(c.m_branches.at(main_of.value(var).first()).a)
												== all.find(c.m_branches.at(main_of.value(A).first()).a))
					others << var;
			if (others.size() > 8) others = others.mid(0, 8);
			bool must = false;
			for (quint32 mask = 0; mask < (1u << others.size()) && !must; ++mask) {
				QSet<int> o;
				for (int k = 0; k < others.size(); ++k) if ((mask >> k) & 1u) o << others.at(k);
				const QSet<int> with_a = o + QSet<int>{A}, with_b = o + QSet<int>{B};
				must = shorts(with_a + with_b) && !shorts(with_a) && !shorts(with_b);
			}
			if (must) out << qMakePair(A, B);
		}
	return out;
}

} // namespace

/* ---- the control circuit ---------------------------------------------- */

void PlanCheck::Logic::checkControl(const QList<Diagram *> &diagrams, const QSet<Diagram *> &panel_folios,
									QVector<Finding> &out)
{
	Nets nets(diagrams, panel_folios);
	Circuit c(nets);
	if (c.m_coils.isEmpty()) return;
	CoilLogic logic(c);

		// coils with contacts: the others are no part of a logic
	QSet<int> used_vars;
	for (const Branch &b : std::as_const(c.m_branches)) if (b.var >= 0) used_vars << b.var;
	const QList<QPair<int, int>> pairs = exclusivePairs(c);
	QSet<int> in_pairs;
	for (const auto &p : pairs) in_pairs << p.first << p.second;

	if (c.m_given ? (c.m_plus.isEmpty() || c.m_minus.isEmpty()) : (c.m_fed.isEmpty() && c.m_plus.isEmpty())) {
		QList<QUuid> items;
		QStringList names;
		for (const CoilInfo &coil : std::as_const(c.m_coils))
			if (used_vars.contains(coil.var)) {items << coil.e->uuid(); names << c.m_vars.at(coil.var).name;}
		if (!items.isEmpty())
			add(out, Severity::Info, "control_data_missing", c.m_coils.first().d, names.mid(0, 6).join(QStringLiteral(", ")),
				ProjectCheck::tr("Supply of the control circuit not found (no potential L+ and M, no sim_role "
								 "supply+/supply- and no protective device in front): interlocks, self-holding "
								 "and stability of %n coil(s) are not checked", nullptr, int(items.size())),
				items, true);
		return;
	}

	QHash<int, int> coil_of_var;
	QVector<QList<Term>> cond(c.m_coils.size());
	QVector<bool> known(c.m_coils.size(), false);
	for (int i = 0; i < c.m_coils.size(); ++i) {
		coil_of_var.insert(c.m_coils.at(i).var, i);
		if (!used_vars.contains(c.m_coils.at(i).var) && !in_pairs.contains(c.m_coils.at(i).var)) continue;
		known[i] = logic.condition(i, &cond[i]);
		if (!known.at(i))
			add(out, Severity::Info, "control_data_missing", c.m_coils.at(i).d, c.m_vars.at(c.m_coils.at(i).var).name,
				ProjectCheck::tr("Coil circuit too large to follow (more than %1 paths or %2 contacts in a row): "
								 "not checked").arg(kMaxTerms).arg(kMaxDepth),
				{c.m_coils.at(i).e->uuid()}, true);
	}

		// interlocks of contactors that must exclude each other
	for (const auto &pair : pairs) {
		const int ia = coil_of_var.value(pair.first, -1), ib = coil_of_var.value(pair.second, -1);
		if (ia < 0 || ib < 0 || !known.at(ia) || !known.at(ib)) continue;
		const CoilInfo &A = c.m_coils.at(ia);
		const CoilInfo &B = c.m_coils.at(ib);
		const QString na = c.m_vars.at(A.var).name, nb = c.m_vars.at(B.var).name;
		const QList<QUuid> items{A.e->uuid(), B.e->uuid()};
		if (cond.at(ia).isEmpty() || cond.at(ib).isEmpty()) {
			const CoilInfo &lost = cond.at(ia).isEmpty() ? A : B;
			const QStringList ends = boundaryOf(c, lost);
			add(out, Severity::Info, "control_data_missing", A.d, na + QStringLiteral(" / ") + nb,
				ends.isEmpty()
					? ProjectCheck::tr("%1 is never supplied from the rails found: the interlock of %2 and %3 is not checked")
						  .arg(c.m_vars.at(lost.var).name, na, nb)
					: ProjectCheck::tr("%1 is never supplied: its circuit ends at %4, a device with no known switching "
									   "function; give the supply as wire function L+ and M or with sim_role. The "
									   "interlock of %2 and %3 is not checked")
						  .arg(c.m_vars.at(lost.var).name, na, nb, ends.join(QStringLiteral(", "))),
				items, true);
			continue;
		}
		bool both = false;
		Term example;
		for (const Term &ta : cond.at(ia)) {
			for (const Term &tb : cond.at(ib)) {
				Term t = ta;
				if (joinTerms(t, tb) && addLit(t, A.var * 2) && addLit(t, B.var * 2)) {example = t; both = true; break;}
			}
			if (both) break;
		}
		const bool a_has = everyTermHas(cond.at(ia), B.var * 2 + 1);
		const bool b_has = everyTermHas(cond.at(ib), A.var * 2 + 1);
		if (both) {
			add(out, Severity::Error, "interlock_missing", A.d, na + QStringLiteral(" / ") + nb,
				ProjectCheck::tr("%1 and %2 can pull in at the same time (e.g. with %3), but their main contacts "
								 "then short-circuit the supply: each needs a normally closed contact of the other "
								 "in its coil circuit (EN 60204-1 9.3.4)")
					.arg(na, nb, stateText(c, example.lits, {A.var, B.var})),
				items);
		} else if (a_has != b_has) {
			const QString lacking = a_has ? nb : na, other = a_has ? na : nb;
			add(out, Severity::Warning, "interlock_one_way", (a_has ? B : A).d, na + QStringLiteral(" / ") + nb,
				ProjectCheck::tr("%1 and %2 are interlocked in one direction only: the coil circuit of %3 has no "
								 "normally closed contact of %4, so %3 pulls in while %4 is still on (EN 60204-1 9.3.4)")
					.arg(na, nb, lacking, other),
				items);
		} else if (!a_has && !b_has) {
			add(out, Severity::Warning, "interlock_buttons_only", A.d, na + QStringLiteral(" / ") + nb,
				ProjectCheck::tr("%1 and %2 are kept apart only by push-buttons or switches, not by normally closed "
								 "contacts of each other: a welded contact lets both be on (EN 60204-1 9.3.4)")
					.arg(na, nb),
				items);
		}
	}

		// self-holding without an off contact
	for (int i = 0; i < c.m_coils.size(); ++i) {
		if (!known.at(i)) continue;
		const CoilInfo &coil = c.m_coils.at(i);
		for (const Term &t : cond.at(i))
			if (t.lits.size() == 1 && t.lits.first() == coil.var * 2) {
				QList<QUuid> items{coil.e->uuid()};
				for (Element *e : t.via) items << e->uuid();
				add(out, Severity::Error, "self_hold_no_off", coil.d, c.m_vars.at(coil.var).name,
					ProjectCheck::tr("Self-holding without an off contact: %1 holds itself through its own normally "
									 "open contact and nothing in that path can switch it off")
						.arg(c.m_vars.at(coil.var).name),
					items);
				break;
			}
	}

		// stability: every group of coils that act on each other must have
		// a state that keeps itself, for every state of its inputs
	const int n = int(c.m_coils.size());
	QVector<QSet<int>> dep(n);
	for (int i = 0; i < n; ++i) {
		if (!known.at(i)) continue;
		for (const Term &t : cond.at(i))
			for (int lit : t.lits)
				if (coil_of_var.contains(lit / 2)) dep[i] << coil_of_var.value(lit / 2);
	}
		// strongly connected parts (Tarjan)
	QVector<int> index(n, -1), low(n, 0);
	QVector<bool> on_stack(n, false);
	QVector<int> stack;
	int counter = 0;
	QList<QVector<int>> groups;
	std::function<void(int)> strong = [&](int v) {
		index[v] = low[v] = counter++;
		stack << v;
		on_stack[v] = true;
		for (int w : dep.at(v)) {
			if (index.at(w) < 0) {strong(w); low[v] = qMin(low.at(v), low.at(w));}
			else if (on_stack.at(w)) low[v] = qMin(low.at(v), index.at(w));
		}
		if (low.at(v) == index.at(v)) {
			QVector<int> g;
			int w;
			do {w = stack.takeLast(); on_stack[w] = false; g << w;} while (w != v);
			groups << g;
		}
	};
	for (int v = 0; v < n; ++v) if (index.at(v) < 0 && known.at(v)) strong(v);

	for (const QVector<int> &g : std::as_const(groups)) {
		if (g.size() == 1 && !dep.at(g.first()).contains(g.first())) continue;
		bool timed = false;
		QSet<int> inside, inputs;
		for (int i : g) inside << c.m_coils.at(i).var;
		for (int i : g)
			for (const Term &t : cond.at(i)) {
				timed = timed || t.timer;
				for (int lit : t.lits) if (!inside.contains(lit / 2)) inputs << lit / 2;
			}
		if (timed) continue;   // a timer in the loop: a flasher on purpose
		QList<int> in_list(inputs.begin(), inputs.end());
		std::sort(in_list.begin(), in_list.end());
		const CoilInfo &first = c.m_coils.at(g.first());
		if (g.size() > 10 || in_list.size() > 14) {
			add(out, Severity::Info, "control_data_missing", first.d, c.m_vars.at(first.var).name,
				ProjectCheck::tr("Loop of %1 coil(s) with %2 input(s) too large to check for a stable state: not checked")
					.arg(g.size()).arg(in_list.size()),
				{first.e->uuid()}, true);
			continue;
		}
		QHash<int, int> bit;   // var -> bit in the state of the loop
		for (int k = 0; k < g.size(); ++k) bit.insert(c.m_coils.at(g.at(k)).var, k);
		QHash<int, int> in_bit;
		for (int k = 0; k < in_list.size(); ++k) in_bit.insert(in_list.at(k), k);
		auto holds = [&](const Term &t, quint32 state, quint32 in) {
			for (int lit : t.lits) {
				const int var = lit / 2;
				const bool v = bit.contains(var) ? ((state >> bit.value(var)) & 1u) : ((in >> in_bit.value(var)) & 1u);
				if (v == bool(lit & 1)) return false;
			}
			return true;
		};
		for (quint32 in = 0; in < (1u << in_list.size()); ++in) {
			bool stable = false;
			for (quint32 s = 0; s < (1u << g.size()) && !stable; ++s) {
				bool fixed = true;
				for (int k = 0; k < g.size() && fixed; ++k) {
					bool on = false;
					for (const Term &t : cond.at(g.at(k))) if (holds(t, s, in)) {on = true; break;}
					fixed = on == bool((s >> k) & 1u);
				}
				stable = fixed;
			}
			if (stable) continue;
			QVector<int> lits;
			for (int k = 0; k < in_list.size(); ++k) lits << in_list.at(k) * 2 + (((in >> k) & 1u) ? 0 : 1);
			QList<QUuid> items;
			QStringList names;
			for (int i : g) {items << c.m_coils.at(i).e->uuid(); names << c.m_vars.at(c.m_coils.at(i).var).name;}
			add(out, Severity::Warning, "circuit_unstable", first.d, names.join(QStringLiteral(", ")),
				ProjectCheck::tr("No stable state with %1: the coil(s) %2 switch on and off without end (pumping)")
					.arg(stateText(c, lits, {}), names.join(QStringLiteral(", "))),
				items);
			break;
		}
	}
}

/* ---- the power circuit ------------------------------------------------ */

namespace {

	/// current-carrying capacity of copper, 30 °C air, IEC 60364-5-52 annex
	/// B tables B.52.2 (PVC, 2 loaded), B.52.4 (PVC, 3 loaded), B.52.3 (XLPE,
	/// 2 loaded), B.52.5 (XLPE, 3 loaded); columns A1, A2, B1, B2, C
const double kSections[] = {1.5, 2.5, 4, 6, 10, 16, 25, 35, 50, 70, 95, 120};
const double kIz[4][12][5] = {
	{ // PVC, 2 loaded
	{14.5, 14, 17.5, 16.5, 19.5}, {19.5, 18.5, 24, 23, 27}, {26, 25, 32, 30, 36}, {34, 32, 41, 38, 46},
	{46, 43, 57, 52, 63}, {61, 57, 76, 69, 85}, {80, 75, 101, 90, 112}, {99, 92, 125, 111, 138},
	{119, 110, 151, 133, 168}, {151, 139, 192, 168, 213}, {182, 167, 232, 201, 258}, {210, 192, 269, 232, 299}},
	{ // PVC, 3 loaded
	{13.5, 13, 15.5, 15, 17.5}, {18, 17.5, 21, 20, 24}, {24, 23, 28, 27, 32}, {31, 29, 36, 34, 41},
	{42, 39, 50, 46, 57}, {56, 52, 68, 62, 76}, {73, 68, 89, 80, 96}, {89, 83, 110, 99, 119},
	{108, 99, 134, 118, 144}, {136, 125, 171, 149, 184}, {164, 150, 207, 179, 223}, {188, 172, 239, 206, 259}},
	{ // XLPE, 2 loaded
	{19, 18.5, 23, 22, 24}, {26, 25, 31, 30, 33}, {35, 33, 42, 40, 45}, {45, 42, 54, 51, 58},
	{61, 57, 75, 69, 80}, {81, 76, 100, 91, 107}, {106, 99, 133, 119, 138}, {131, 121, 164, 146, 171},
	{158, 145, 198, 175, 209}, {200, 183, 253, 221, 269}, {241, 220, 306, 265, 328}, {278, 253, 354, 305, 382}},
	{ // XLPE, 3 loaded
	{17, 16.5, 20, 19.5, 22}, {23, 22, 28, 26, 30}, {31, 30, 37, 35, 40}, {40, 38, 48, 44, 52},
	{54, 51, 66, 60, 71}, {73, 68, 88, 80, 96}, {95, 89, 117, 105, 119}, {117, 109, 144, 128, 147},
	{141, 130, 175, 154, 179}, {179, 164, 222, 194, 229}, {216, 197, 269, 233, 278}, {249, 227, 312, 268, 322}},
};
const char *const kMethods[] = {"A1", "A2", "B1", "B2", "C"};

	/// IEC 60364-5-52 table B.52.14 (air), PVC and XLPE, 25 to 60 °C
const double kTemps[] = {25, 30, 35, 40, 45, 50, 55, 60};
const double kTempFactor[2][8] = {
	{1.06, 1.00, 0.94, 0.87, 0.79, 0.71, 0.61, 0.50},
	{1.04, 1.00, 0.96, 0.91, 0.87, 0.82, 0.76, 0.71}};

	/// IEC 60364-5-52 table B.52.17 row 1 (bunched in air, on a surface,
	/// embedded or enclosed): circuits -> factor
const int kGroups[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 16, 20};
const double kGroupFactor[] = {1.00, 0.80, 0.70, 0.65, 0.60, 0.57, 0.54, 0.52, 0.50, 0.45, 0.41, 0.38};

	/// the column of a reference method, -1 when not in the tables
int methodColumn(const QString &text)
{
	const QString t = text.trimmed().toUpper();
	for (int i = 0; i < 5; ++i) if (t == QLatin1String(kMethods[i])) return i;
	return -1;
}

	/// 0 PVC, 1 XLPE (EPR), -1 not known: written out or from the type
	/// code (DIN VDE 0292: NYM, NYY Y = PVC, N2X 2X = XLPE; HD 361 H05V,
	/// H07V V = PVC)
int insulationOf(const QString &text)
{
	static const QRegularExpression xlpe(QStringLiteral("\\b(XLPE|VPE|EPR)\\b|\\bN2X"), QRegularExpression::CaseInsensitiveOption);
	static const QRegularExpression pvc(QStringLiteral("\\bPVC\\b|\\bNY[A-Z]*\\b|\\bH0[357]V"), QRegularExpression::CaseInsensitiveOption);
	if (text.contains(xlpe)) return 1;
	if (text.contains(pvc)) return 0;
	return -1;
}

QString methodOf(const QString &text)
{
	static const QRegularExpression re(QStringLiteral(
		"(?:verlegeart|installation method|method)\\s*[:=]?\\s*(A1|A2|B1|B2|C|D1|D2|E|F|G)\\b"),
		QRegularExpression::CaseInsensitiveOption);
	const QRegularExpressionMatch m = re.match(text);
	return m.hasMatch() ? m.captured(1).toUpper() : QString();
}

double ambientOf(const QString &text)
{
	static const QRegularExpression re(QStringLiteral("(?:umgebung|ambient)\\D{0,12}?(\\d+(?:[.,]\\d+)?)\\s*°?\\s*C\\b"),
									   QRegularExpression::CaseInsensitiveOption);
	const QRegularExpressionMatch m = re.match(text);
	return m.hasMatch() ? leadingNumber(m.captured(1)) : 0;
}

int groupingOf(const QString &text)
{
	static const QRegularExpression re(QStringLiteral("(?:häufung|grouping)\\s*[:=]?\\s*(\\d+)"),
									   QRegularExpression::CaseInsensitiveOption);
	const QRegularExpressionMatch m = re.match(text);
	return m.hasMatch() ? m.captured(1).toInt() : 0;
}

	/// length of a cable in m, 0 when none
double lengthOf(const QString &text)
{
	static const QRegularExpression number(QStringLiteral("(\\d+(?:[.,]\\d+)?)"));
	const QRegularExpressionMatch m = number.match(text);
	if (!m.hasMatch()) return 0;
	double v = leadingNumber(m.captured(1));
	if (text.contains(QLatin1String("mm"), Qt::CaseInsensitive)) v /= 1000;
	else if (text.contains(QLatin1String("cm"), Qt::CaseInsensitive)) v /= 100;
	return v;
}

	/// IEC 60898-1 table 2: the upper limit of instantaneous tripping in
	/// multiples of In; 0 for another characteristic
double instantFactor(QChar ch)
{
	switch (ch.toUpper().unicode()) {
		case 'B': return 5;
		case 'C': return 10;
		case 'D': return 20;
		default: return 0;
	}
}

QChar characteristicOf(Element *e)
{
	const QString own = infoOf(e, "characteristic");
	if (!own.isEmpty()) return own.at(0).toUpper();
	static const QRegularExpression curve(QStringLiteral("(?<![A-Za-z0-9])([BCDKZ])\\s?\\d+(?:[.,]\\d+)?\\s?A?(?![A-Za-z0-9]|[.,]\\d)"));
	const QRegularExpressionMatch m = curve.match(describedOf(e));
	return m.hasMatch() ? m.captured(1).at(0) : QChar();
}

double breakingCapacityOf(Element *e)
{
	for (const char *key : {"icu_ka", "icn_ka", "breaking_capacity_ka"}) {
		const double v = valueOf(e, key);
		if (v > 0) return v * 1000;
	}
	static const QRegularExpression ka(QStringLiteral("(\\d+(?:[.,]\\d+)?)\\s?kA\\b"));
	const QRegularExpressionMatch m = ka.match(describedOf(e));
	return m.hasMatch() ? leadingNumber(m.captured(1)) * 1000 : 0;
}

bool isRcd(Element *e)
{
	if (QET::infoFlagIsTrue(infoOf(e, "rcd")) || valueOf(e, "idn_ma") > 0) return true;
	static const QRegularExpression re(QStringLiteral("\\b(FI|RCD|RCCB|RCBO|FI/LS)\\b|fehlerstrom|residual|diff[ée]rentiel|differential"),
									   QRegularExpression::CaseInsensitiveOption);
	if ((describedOf(e) + QLatin1Char(' ') + e->name()).contains(re)) return true;
		// the name of an old embedded symbol may be missing: its folder or
		// file names it ("interrupteurs_differentiels/int_diff1.elmt")
	static const QRegularExpression path(QStringLiteral("differenti|int_?diff|residual_current|fehlerstrom|/ddr\\d"),
										 QRegularExpression::CaseInsensitiveOption);
	return e->location().toString().contains(path);
}

	/// IΔn in A, 0 when not given
double ratedResidualOf(Element *e)
{
	const double own = valueOf(e, "idn_ma");
	if (own > 0) return own / 1000;
	static const QRegularExpression ma(QStringLiteral("(\\d+(?:[.,]\\d+)?)\\s?mA\\b"));
	const QRegularExpressionMatch m = ma.match(describedOf(e) + QLatin1Char(' ') + e->name());
	return m.hasMatch() ? leadingNumber(m.captured(1)) / 1000 : 0;
}

bool isSelectiveRcd(Element *e)
{
	if (infoOf(e, "rcd_type").contains(QLatin1Char('S'), Qt::CaseInsensitive)) return true;
	static const QRegularExpression re(QStringLiteral("selektiv|selective|s[ée]lectif|\\btyp(e)?\\s*S\\b|\\[S\\]"),
									   QRegularExpression::CaseInsensitiveOption);
	return (describedOf(e) + QLatin1Char(' ') + e->name()).contains(re);
}

bool isSocket(Element *e)
{
	if (QET::infoFlagIsTrue(infoOf(e, "socket"))) return true;
	static const QRegularExpression re(QStringLiteral("steckdose|socket|outlet|prise de courant|\\bprise\\b"),
									   QRegularExpression::CaseInsensitiveOption);
	return e->name().contains(re) || e->location().toString().contains(re);
}

	/// copper at 70 °C in Ω·mm²/m: ρ20 of IEC 60228 (1/58), α 0.00393/K
const double kRho70 = 0.017241 * (1.0 + 0.00393 * (70.0 - 20.0));

struct Feed {
	Element *e = nullptr;
	Diagram *d = nullptr;
	double ik3 = 0;   ///< A, 0 when not known
	double zs = 0;    ///< loop impedance at the origin in Ω, 0 when not known
	double u0 = 0;    ///< V line to earth, 0 when not known
	QString how;      ///< how ik3 was found
	QSet<int> reach;  ///< nets it feeds
};

struct Device { Diagram *d; Element *e; double in; QSet<int> ins; QSet<int> outs; };

} // namespace

void PlanCheck::Logic::checkPower(QETProject *project, const QList<Diagram *> &diagrams,
								  const QSet<Diagram *> &panel_folios, QVector<Finding> &out)
{
	if (!project) return;
	Nets nets(diagrams, panel_folios);
	Circuit c(nets);

		// which cable a wire is a core of
	QHash<QUuid, Cable *> cable_of;
	for (Cable *cable : project->cables())
		for (const CableCore &core : cable->usedCores()) cable_of.insert(core.conductor, cable);

		// the power flow: from the input of a pole to its output, both ways
		// through the others (every contact closed)
	QHash<int, QVector<int>> next, back;
	for (const Branch &b : std::as_const(c.m_branches)) {
		next[b.a] << b.b;
		back[b.b] << b.a;
		if (!b.directed) {next[b.b] << b.a; back[b.a] << b.b;}
	}
	auto flood = [](const QSet<int> &start, const QHash<int, QVector<int>> &edges) {
		QSet<int> seen = start;
		QList<int> todo(start.begin(), start.end());
		while (!todo.isEmpty()) {
			const int n = todo.takeLast();
			for (int m : edges.value(n)) if (!seen.contains(m)) {seen << m; todo << m;}
		}
		return seen;
	};

		// the feeds
	QList<Feed> feeds;
	for (const auto &de : std::as_const(nets.m_devices)) {
		Element *e = de.second;
		const ElementData data = e->elementData();
		const bool protective = data.m_type == ElementData::Master && data.m_master_type == ElementData::Protection;
		const double ik3 = valueOf(e, "ik3_ka"), ik1 = valueOf(e, "ik1_ka"), sk = valueOf(e, "sk_mva"),
			sr = valueOf(e, "sr_kva"), ukr = valueOf(e, "ukr_pct"), zs = valueOf(e, "zs_ohm"),
			un = valueOf(e, "un_v"), u0 = valueOf(e, "u0_v");
		if (!(ik3 > 0 || ik1 > 0 || sk > 0 || sr > 0 || (zs > 0 && !protective))) continue;
		Feed f;
		f.e = e;
		f.d = de.first;
		f.u0 = u0 > 0 ? u0 : (un > 0 ? un / std::sqrt(3.0) : 0);
			// IEC 60909-0 table 1: low voltage, tolerance +10 %
		const double cmax = valueOf(e, "c_max") > 0 ? valueOf(e, "c_max") : 1.10;
		if (ik3 > 0) {
			f.ik3 = ik3 * 1000;
			f.how = ProjectCheck::tr("given");
		} else if (sr > 0 && ukr > 0 && un > 0) {
			const double zt = ukr / 100.0 * un * un / (sr * 1000);
			const double zq = sk > 0 ? cmax * un * un / (sk * 1e6) : 0;
			f.ik3 = cmax * un / (std::sqrt(3.0) * (zq + zt));
			f.how = sk > 0 ? ProjectCheck::tr("transformer and network, IEC 60909-0")
						   : ProjectCheck::tr("transformer on a network of unlimited power, IEC 60909-0");
		} else if (sk > 0 && un > 0) {
			f.ik3 = sk * 1e6 / (std::sqrt(3.0) * un);
			f.how = ProjectCheck::tr("network, IEC 60909-0");
		}
		if (zs > 0 && !protective) f.zs = zs;
		else if (ik1 > 0 && f.u0 > 0) f.zs = f.u0 / (ik1 * 1000);
		QSet<int> start;
		for (Terminal *t : e->terminals()) start << nets.of(t);
		f.reach = flood(start, next);
		feeds << f;
		if (f.ik3 > 0)
			add(out, Severity::Info, "short_circuit", f.d, shownOf(e),
				ProjectCheck::tr("Short-circuit current at the feed Ik3 = %1 kA (%2, c = %3)")
					.arg(fmt(f.ik3 / 1000), f.how, fmt(cmax)),
				{e->uuid()});
	}

	QList<Device> devices;
	for (const auto &de : std::as_const(c.m_protection)) {
		Element *e = de.second;
		Device dev{de.first, e, PlanCheck::ratedCurrentOf(e), {}, {}};
		for (const Pair &p : pairsOf(e)) {
			if (!p.directed) continue;
			dev.ins << nets.of(p.a);
			dev.outs << nets.of(p.b);
		}
		if (dev.ins.isEmpty())
			for (Terminal *t : e->terminals()) (PlanCheck::isOutput(t, e->terminals()) ? dev.outs : dev.ins) << nets.of(t);
		devices << dev;
	}
	if (devices.isEmpty()) return;

	QList<QUuid> no_method, no_env, no_icu, no_loop;
	QStringList loop_missing;
	bool any_with_current = false;

	for (const Device &dev : std::as_const(devices)) {
		Element *e = dev.e;
		const QString shown = shownOf(e);
		const bool rcd = isRcd(e);
		const QChar curve = characteristicOf(e);
		const bool overcurrent = dev.in > 0 && !(rcd && curve.isNull());
		if (overcurrent) any_with_current = true;

			// the wires and cables behind it
		double section = 0;
		QList<Cable *> cables;
		QString cable_text;
		for (int n : dev.outs)
			for (Conductor *w : nets.m_net_wires.value(n)) {
				const double s = PlanCheck::sectionOf(w->properties().m_wire_section);
				if (s > 0 && (section <= 0 || s < section)) section = s;
				cable_text += QLatin1Char(' ') + w->properties().m_conductor_type;
				if (Cable *cable = cable_of.value(w->uuid()))
					if (!cables.contains(cable)) {
						cables << cable;
						cable_text += QLatin1Char(' ') + cable->type() + QLatin1Char(' ') + cable->comment()
									  + QLatin1Char(' ') + cable->installation();
					}
			}

			// 1. In <= Iz after IEC 60364-5-52 annex B
		if (overcurrent && section > 0) {
			const QString own_method = infoOf(e, "installation_method");
			const QString method = own_method.isEmpty() ? methodOf(cable_text) : own_method.toUpper();
			const QString own_ins = infoOf(e, "insulation");
			const int insulation = insulationOf(own_ins.isEmpty() ? cable_text : own_ins);
			const int col = methodColumn(method);
			if (method.isEmpty() || insulation < 0) {
				no_method << e->uuid();
			} else if (col < 0) {
				add(out, Severity::Info, "cable_capacity", dev.d, shown,
					ProjectCheck::tr("Installation method %1 is not in the tables used (A1, A2, B1, B2, C): "
									 "In ≤ Iz not checked").arg(method),
					{e->uuid()}, true);
			} else if (section < 1.5 - 1e-9 || section > 120 + 1e-9) {
				add(out, Severity::Info, "cable_capacity", dev.d, shown,
					ProjectCheck::tr("%1 mm² is outside the tables used (1.5 to 120 mm²): In ≤ Iz not checked")
						.arg(fmt(section)), {e->uuid()}, true);
			} else {
				int row = 0;
				for (int i = 0; i < 12; ++i) if (section + 1e-9 >= kSections[i]) row = i;
				const int loaded = dev.outs.size() >= 3 ? 3 : 2;
				const int table = insulation * 2 + (loaded == 3 ? 1 : 0);
				double iz = kIz[table][row][col];
				const double own_amb = valueOf(e, "ambient_c");
				const double ambient = own_amb > 0 ? own_amb : ambientOf(cable_text);
				const int own_grp = int(valueOf(e, "grouping"));
				const int grouping = own_grp > 0 ? own_grp : groupingOf(cable_text);
				double k_temp = 1, k_group = 1;
				bool usable = true;
				if (ambient > 0) {
					int t = -1;
					for (int i = 0; i < 8; ++i) if (t < 0 && ambient <= kTemps[i] + 1e-9) t = i;
					if (t < 0) usable = false; else k_temp = kTempFactor[insulation][t];
				}
				if (grouping > 0) {
					int g = -1;
					for (int i = 0; i < 12; ++i) if (g < 0 && grouping <= kGroups[i]) g = i;
					if (g < 0) usable = false; else k_group = kGroupFactor[g];
				}
				if (!usable) {
					add(out, Severity::Info, "cable_capacity", dev.d, shown,
						ProjectCheck::tr("Ambient temperature or grouping outside the tables (60 °C, 20 circuits): "
										 "In ≤ Iz not checked"), {e->uuid()}, true);
				} else {
					iz *= k_temp * k_group;
					if (dev.in > iz + 1e-9)
						add(out, Severity::Warning, "cable_capacity", dev.d, shown,
							ProjectCheck::tr("In = %1 A is above Iz = %2 A of %3 mm² (IEC 60364-5-52 annex B: method %4, "
											 "%5, %6 loaded conductors, factor %7 for temperature and grouping)")
								.arg(fmt(dev.in), fmt(iz), fmt(section), method,
									 insulation ? QStringLiteral("XLPE") : QStringLiteral("PVC"))
								.arg(loaded).arg(fmt(k_temp * k_group)),
							{e->uuid()});
					else if (ambient <= 0 || grouping <= 0)
						no_env << e->uuid();
				}
			}
		}

			// which feeds reach this device
		QList<const Feed *> mine;
		for (const Feed &f : std::as_const(feeds)) {
			bool hit = false;
			for (int n : dev.ins) if (f.reach.contains(n)) hit = true;
			if (hit) mine << &f;
		}

			// 2. breaking capacity against Ik3 at the feed
		double ik3 = 0;
		for (const Feed *f : std::as_const(mine)) ik3 = qMax(ik3, f->ik3);
		if (ik3 > 0 && overcurrent) {
			const double icu = breakingCapacityOf(e);
			if (icu <= 0) no_icu << e->uuid();
			else if (icu < ik3 - 1e-9)
				add(out, Severity::Error, "short_circuit", dev.d, shown,
					ProjectCheck::tr("Breaking capacity %1 kA is below the short-circuit current %2 kA at the feed "
									 "(IEC 60909-0, simplified: cables in front neglected, the highest value)")
						.arg(fmt(icu / 1000), fmt(ik3 / 1000)),
					{e->uuid()});
		}

			// 3. disconnection: Zs × Ia <= U0 (IEC 60364-4-41 411.4.4)
		Cable *worst = nullptr;
		double worst_r = 0, worst_len = 0, worst_s = 0, worst_spe = 0;
		static const QRegularExpression type_section(
			QStringLiteral("\\d+\\s*[xXG×]\\s*(\\d+(?:[.,]\\d+)?)(?:\\s*/\\s*(\\d+(?:[.,]\\d+)?))?"));
		for (Cable *cable : std::as_const(cables)) {
			const double len = lengthOf(cable->length());
			if (len <= 0) continue;
			double s = 0;
			for (const CableCore &core : cable->usedCores())
				for (int n : dev.outs)
					for (Conductor *w : nets.m_net_wires.value(n))
						if (w->uuid() == core.conductor) {
							const double ws = PlanCheck::sectionOf(w->properties().m_wire_section);
							if (ws > 0 && (s <= 0 || ws < s)) s = ws;
						}
			const QRegularExpressionMatch tm = type_section.match(cable->type());
			if (s <= 0 && tm.hasMatch()) s = leadingNumber(tm.captured(1));
			if (s <= 0) continue;
			const double spe = tm.hasMatch() && !tm.captured(2).isEmpty() ? leadingNumber(tm.captured(2)) : s;
			const double r = kRho70 * len * (1.0 / s + 1.0 / spe);
			if (r > worst_r) {worst = cable; worst_r = r; worst_len = len; worst_s = s; worst_spe = spe;}
		}
		if (worst && overcurrent) {
			const double ia_own = valueOf(e, "ia_a");
			const double ia = ia_own > 0 ? ia_own : instantFactor(curve) * dev.in;
			double u0 = valueOf(e, "u0_v");
			if (u0 <= 0) for (const Feed *f : std::as_const(mine)) if (f->u0 > 0) u0 = f->u0;
			const double zs_dev = valueOf(e, "zs_ohm");
			double zs_origin = zs_dev;
			bool complete = zs_dev > 0;
			if (!complete && mine.size() == 1 && mine.first()->zs > 0) {
				zs_origin = mine.first()->zs;
					// a cable with a length in front: its impedance is not known here
				bool cable_in_front = false;
				for (int n : dev.ins)
					for (Conductor *w : nets.m_net_wires.value(n))
						if (Cable *cable = cable_of.value(w->uuid()))
							if (lengthOf(cable->length()) > 0 && !cables.contains(cable)) cable_in_front = true;
				complete = !cable_in_front;
			}
			QStringList missing;
			if (ia <= 0) missing << ProjectCheck::tr("Ia (characteristic B, C, D or ia_a)");
			if (u0 <= 0) missing << ProjectCheck::tr("U0 (u0_v, or un_v of the feed)");
			if (ia > 0 && u0 > 0) {
				const double zs = zs_origin + worst_r;
				const QString name = worst->designation().trimmed().isEmpty() ? worst->type() : worst->designation().trimmed();
				if (zs * ia > u0 * (1 + 1e-9)) {
					const double reach = (u0 / ia - zs_origin) / (kRho70 * (1.0 / worst_s + 1.0 / worst_spe));
					add(out, Severity::Error, "loop_impedance", dev.d, shown,
						ProjectCheck::tr("No disconnection in time: Zs × Ia = %1 Ω × %2 A = %3 V is above U0 = %4 V "
										 "(cable %5, %6 m of %7 mm², copper 70 °C; Zs at the origin %8 Ω); this "
										 "section reaches %9 m (IEC 60364-4-41 411.4.4)")
							.arg(fmt(zs, 3), fmt(ia), fmt(zs * ia), fmt(u0), name, fmt(worst_len), fmt(worst_s),
								 fmt(zs_origin, 3), QLocale().toString(qMax(0.0, std::floor(reach)), 'f', 0)),
						{e->uuid()});
				} else if (!complete) {
					missing << ProjectCheck::tr("loop impedance at the device (zs_ohm)");
				}
			}
			if (!missing.isEmpty()) {
				no_loop << e->uuid();
				loop_missing << QStringLiteral("%1: %2").arg(shown, missing.join(QStringLiteral(", ")));
			}
		}
	}

		// RCDs: in series, and in front of the socket-outlets
	QList<const Device *> rcds;
	for (const Device &dev : std::as_const(devices)) if (isRcd(dev.e)) rcds << &dev;
	for (const Device *up : std::as_const(rcds)) {
		const QSet<int> below = flood(up->outs, next);
		for (const Device *down : std::as_const(rcds)) {
			if (down == up) continue;
			bool behind = false;
			for (int n : down->ins) if (below.contains(n)) behind = true;
			if (!behind) continue;
				// not the other way round as well (no direction known)
			const QSet<int> below_down = flood(down->outs, next);
			bool back_again = false;
			for (int n : up->ins) if (below_down.contains(n)) back_again = true;
			if (back_again) continue;
			const double i_up = ratedResidualOf(up->e), i_down = ratedResidualOf(down->e);
			const bool known = i_up > 0 && i_down > 0;
			if (known && isSelectiveRcd(up->e) && i_up >= 3 * i_down - 1e-12) continue;
				// unknown IΔn behind a type S: most likely selective, a datum is
				// missing; unknown behind any other: worth a look
			add(out, !known && isSelectiveRcd(up->e) ? Severity::Info : Severity::Warning,
				"rcd_in_series", down->d, shownOf(down->e),
				known
					? ProjectCheck::tr("RCD behind RCD %1: not selective, the one in front must be of type S with "
									   "IΔn of at least 3 × %2 mA (IEC 60364-5-53 535.3)")
						  .arg(shownOf(up->e), fmt(i_down * 1000))
					: ProjectCheck::tr("RCD behind RCD %1: selectivity not decided, type S or IΔn not given "
									   "(rcd_type, idn_ma; IEC 60364-5-53 535.3)").arg(shownOf(up->e)),
				{down->e->uuid(), up->e->uuid()}, !known);
		}
	}
	for (const auto &de : std::as_const(nets.m_devices)) {
		Element *e = de.second;
		if (!isSocket(e)) continue;
		QSet<int> start;
		for (Terminal *t : e->terminals()) start << nets.of(t);
		const QSet<int> above = flood(start, back);
		bool ok = false, unknown = false;
		QList<QUuid> found;
		for (const Device *r : std::as_const(rcds)) {
			bool feeds_it = false;
			for (int n : r->outs) if (above.contains(n)) feeds_it = true;
			if (!feeds_it) continue;
			found << r->e->uuid();
			const double idn = ratedResidualOf(r->e);
			if (idn <= 0) unknown = true;
			else if (idn <= 0.030 + 1e-12) ok = true;
		}
		if (ok) continue;
			// the supply of the socket not followed back to any protective
			// device: what is in front of it is not known
		bool reached = !found.isEmpty();
		for (const Device &dev : std::as_const(devices))
			for (int n : dev.outs) if (above.contains(n)) reached = true;
		if (!reached) {
			add(out, Severity::Info, "socket_without_rcd", de.first, shownOf(e),
				ProjectCheck::tr("Supply of the socket-outlet not followed back to a protective device: an RCD of "
								 "at most 30 mA in front is not checked (IEC 60364-4-41 411.3.3)"),
				{e->uuid()}, true);
			continue;
		}
		if (unknown)
			add(out, Severity::Info, "socket_without_rcd", de.first, shownOf(e),
				ProjectCheck::tr("Socket-outlet behind an RCD without IΔn (idn_ma): 30 mA not checked "
								 "(IEC 60364-4-41 411.3.3)"),
				QList<QUuid>{e->uuid()} + found, true);
		else
			add(out, Severity::Warning, "socket_without_rcd", de.first, shownOf(e),
				found.isEmpty()
					? ProjectCheck::tr("Socket-outlet without an RCD in front: additional protection by an RCD of at "
									   "most 30 mA is required (IEC 60364-4-41 411.3.3)")
					: ProjectCheck::tr("Socket-outlet behind an RCD of more than 30 mA: additional protection by an RCD "
									   "of at most 30 mA is required (IEC 60364-4-41 411.3.3)"),
				QList<QUuid>{e->uuid()} + found);
	}

		// what was not checked for want of data, one note each
	Diagram *first = devices.first().d;
	if (!no_method.isEmpty())
		add(out, Severity::Info, "cable_capacity", first, QStringLiteral("IEC 60364-5-52"),
			ProjectCheck::tr("%n protective device(s) without installation method or insulation of their wires: "
							 "In ≤ Iz after IEC 60364-5-52 not checked (installation_method A1, A2, B1, B2 or C "
							 "and insulation PVC or XLPE, at the device or in the cable)", nullptr, int(no_method.size())),
			no_method, true);
	if (!no_env.isEmpty())
		add(out, Severity::Info, "cable_capacity", first, QStringLiteral("IEC 60364-5-52"),
			ProjectCheck::tr("%n protective device(s): ambient temperature or grouping not given (ambient_c, grouping); "
							 "Iz taken at 30 °C without grouping, a higher temperature or more circuits lower it",
							 nullptr, int(no_env.size())),
			no_env, true);
	if (feeds.isEmpty() && any_with_current)
		add(out, Severity::Info, "feed_data_missing", nullptr, QString(),
			ProjectCheck::tr("No feed data in the plan (ik3_ka, ik1_ka, zs_ohm, sk_mva or sr_kva with ukr_pct and un_v "
							 "at the supply): short-circuit current and disconnection are not checked"),
			{}, true);
	if (!no_icu.isEmpty())
		add(out, Severity::Info, "short_circuit", first, QStringLiteral("IEC 60909"),
			ProjectCheck::tr("%n protective device(s) without breaking capacity (icu_ka, or \"6 kA\" in the designation): "
							 "not checked against the short-circuit current", nullptr, int(no_icu.size())),
			no_icu, true);
	if (!no_loop.isEmpty())
		add(out, Severity::Info, "loop_impedance", first, QStringLiteral("IEC 60364-4-41"),
			ProjectCheck::tr("Disconnection not decided, missing: %1").arg(loop_missing.join(QStringLiteral("; "))),
			no_loop, true);
}
