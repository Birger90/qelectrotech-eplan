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
#include "grafcet.h"

#include "plcprogram.h"
#include "../diagram.h"
#include "../qet.h"
#include "../qetgraphicsitem/dynamicelementtextitem.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/elementtextitemgroup.h"
#include "../qetgraphicsitem/terminal.h"

#include <QFileInfo>
#include <QLineF>
#include <QRegularExpression>
#include <QSet>

#include <cmath>

namespace {

enum class Kind { None, InitialStep, Step, Action, ConditionalAction, ActionOnActivation, ActionOnDeactivation,
				  ActionOnEvent, Transition, Bar, Junction, Jump };

Kind kindOf(const Element *e)
{
	const QString path = e->location().toString();
	if (!path.contains(QLatin1String("202020_grafcet"))) return Kind::None;
	const QString file = QFileInfo(path).fileName();
	if (file.startsWith(QLatin1String("20202000"))) return Kind::InitialStep;
	if (file.startsWith(QLatin1String("2020200"))) return Kind::Step;       // step, enclosing step, macro step
	if (file.startsWith(QLatin1String("20202020"))) return Kind::Action;
	if (file.startsWith(QLatin1String("20202021"))) return Kind::ConditionalAction;
	if (file.startsWith(QLatin1String("20202022"))) return Kind::ActionOnActivation;
	if (file.startsWith(QLatin1String("20202023"))) return Kind::ActionOnDeactivation;
	if (file.startsWith(QLatin1String("20202024"))) return Kind::ActionOnEvent;
	if (file.startsWith(QLatin1String("2020206"))) return Kind::Transition;
	if (file.startsWith(QLatin1String("2020207"))) return Kind::Bar;        // parallel branch, synchronization
	if (file.startsWith(QLatin1String("2020208"))) return Kind::Junction;
	if (file.startsWith(QLatin1String("2020209"))) return Kind::Jump;
	return Kind::None;
}

QString label(const Element *e)
{
	return e->elementInformations()[QStringLiteral("label")].toString().trimmed();
}

struct UserText { QString text; double y; };

	/// texts typed into the symbol (not the label), with their height in the symbol
QList<UserText> userTexts(Element *e)
{
	QList<UserText> out;
	auto add = [&](DynamicElementTextItem *t, double dy) {
		if (t->textFrom() == DynamicElementTextItem::UserText && !t->text().trimmed().isEmpty())
			out << UserText{t->text().trimmed(), t->pos().y() + dy};
	};
	for (DynamicElementTextItem *t : e->dynamicTextItems()) add(t, 0.0);
	for (ElementTextItemGroup *g : e->textGroups())
		for (DynamicElementTextItem *t : g->texts()) add(t, g->pos().y());
	return out;
}

	/// text inside the action box (between -18 and 8 in the symbol), else the label
QString boxText(Element *e)
{
	for (const UserText &t : userTexts(e))
		if (t.y >= -18 && t.y <= 8) return t.text;
	return label(e);
}

	/// condition or event written above the action box
QString textAbove(Element *e)
{
	for (const UserText &t : userTexts(e))
		if (t.y < -18) return t.text;
	return QString();
}

QString firstText(Element *e)
{
	if (!label(e).isEmpty()) return label(e);
	const QList<UserText> t = userTexts(e);
	return t.isEmpty() ? QString() : t.first().text;
}

QString stepNumber(const QString &text)
{
	QString n = text.trimmed();
	n.remove(QLatin1Char(' '));
	if (n.startsWith(QLatin1Char('X'), Qt::CaseInsensitive) && n.size() > 1) n.remove(0, 1);
	return n.toUpper();
}

QString replaceAll(const QString &s, const QRegularExpression &re,
				   const std::function<QString(const QRegularExpressionMatch &)> &with)
{
	QString out;
	int last = 0;
	auto it = re.globalMatch(s);
	while (it.hasNext()) {
		const auto m = it.next();
		out += s.mid(last, m.capturedStart() - last) + with(m);
		last = static_cast<int>(m.capturedEnd());
	}
	return out + s.mid(last);
}

double seconds(const QString &value, const QString &unit)
{
	const double v = QString(value).replace(QLatin1Char(','), QLatin1Char('.')).toDouble();
	if (unit.compare(QLatin1String("ms"), Qt::CaseInsensitive) == 0) return v / 1000.0;
	if (unit.compare(QLatin1String("min"), Qt::CaseInsensitive) == 0) return v * 60.0;
	return v;
}

}

struct Grafcet::Step {
	Element *e = nullptr;
	QString number, var, shown;
	int in = 0, out = 0, act = 0;       // net keys
	bool initial = false, active = false;
	bool enclosing = false, macro = false;
	int part = -1;                      // partial grafcet (by name; the unnamed chains form the global grafcet)
	int comp = -1;                      // connected chain
	QStringList encloses;               // R14: names of the partial grafcets this enclosing step encloses
	Step *macro_in = nullptr, *macro_out = nullptr;   // R14: input step E… and output step S… of a macro step
	int expansion = -1;                 // R14: chain of the macro expansion
	double since = 0.0;
};

struct Grafcet::Transition {
	Element *e = nullptr;
	QString text;
	int in = 0, out = 0;
	std::shared_ptr<PlcProgram> condition;
	QList<Step *> before, after;
	double value = 0.0;
};

struct Grafcet::Action {
	Element *e = nullptr;
	Kind kind = Kind::Action;
	int key = 0;
	Step *step = nullptr;
	QString text;
	QStringList outputs;                 // continuous actions
	std::shared_ptr<PlcProgram> program;  // stored actions
	std::shared_ptr<PlcProgram> condition;
	QString condition_text;
	// R14 forcing order G…{…}: target partial grafcet, mode and the step numbers of the forced situation
	QString force_target;
	enum class Force { None, Situation, Freeze, Init } force = Force::None;
	QStringList force_steps;
};

struct Grafcet::Partial {
	QString name;
	QList<Step *> steps;
	Step *encloser = nullptr;            // R14: the enclosing step, if this partial grafcet is enclosed
	bool frozen = false;                 // R14: forced in this cycle, it does not evolve
	QString forced_as;                   // last forced situation, for the message
};

namespace {
	/// the elements of a folio in a fixed order (by uuid): the scene does not give them in the same order on every
	/// load, and the model must be built the same way every time, so that a run is reproducible (roadmap R03)
QList<Element *> sortedElements(Diagram *d)
{
	QList<Element *> out = d->elements();
	std::sort(out.begin(), out.end(), [](const Element *a, const Element *b) { return a->uuid() < b->uuid(); });
	return out;
}

QString info(const Element *e, const char *key)
{
	return e->elementInformations()[QLatin1String(key)].toString().trimmed();
}
}

bool Grafcet::isGrafcet(const Element *element)
{
	return element && kindOf(element) != Kind::None;
}

QString Grafcet::labelVariable(const QString &lbl)
{
	QString s = lbl.trimmed();
	if (s.startsWith(QLatin1Char('-'))) s.remove(0, 1);
	s.replace(QLatin1Char('/'), QStringLiteral("__"));
	s.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_.]")), QStringLiteral("_"));
	return PlcProgram::normalized(QStringLiteral("BMK_") + s);
}

QString Grafcet::toStructuredText(const QString &text)
{
	QString s = text.trimmed();
	if (s.isEmpty() || s == QLatin1String("=1") || s == QLatin1String("1")) return QStringLiteral("TRUE");
	// times: 2s/X1, 500 ms/X3, t/X1/2s
	static const QRegularExpression after_step(QStringLiteral(
		"(\\d+(?:[.,]\\d+)?)\\s*(ms|s|min)\\s*/\\s*[Xx]\\s*([A-Za-z0-9_]+)"));
	s = replaceAll(s, after_step, [](const QRegularExpressionMatch &m) {
		return QStringLiteral("(X%1.T >= %2)").arg(stepNumber(m.captured(3)))
			.arg(seconds(m.captured(1), m.captured(2)), 0, 'g', 12);
	});
	static const QRegularExpression t_step(QStringLiteral(
		"\\bt\\s*/\\s*[Xx]\\s*([A-Za-z0-9_]+)\\s*/\\s*(\\d+(?:[.,]\\d+)?)\\s*(ms|s|min)"));
	s = replaceAll(s, t_step, [](const QRegularExpressionMatch &m) {
		return QStringLiteral("(X%1.T >= %2)").arg(stepNumber(m.captured(1)))
			.arg(seconds(m.captured(2), m.captured(3)), 0, 'g', 12);
	});
	// IEC 60848 operators
	s.replace(QStringLiteral("·"), QStringLiteral(" AND "));
	s.replace(QStringLiteral("∧"), QStringLiteral(" AND "));
	s.replace(QStringLiteral("∨"), QStringLiteral(" OR "));
	s.replace(QStringLiteral("¬"), QStringLiteral(" NOT "));
	static const QRegularExpression slash_not(QStringLiteral("(^|[\\s(])/(?=\\s*[-A-Za-z_%(])"));
	s = replaceAll(s, slash_not, [](const QRegularExpressionMatch &m) { return m.captured(1) + QStringLiteral("NOT "); });
	// element labels: -BG1, -QM1/14
	static const QRegularExpression element(QStringLiteral(
		"(^|[\\s(,;&=<>*+!\\x{2191}\\x{2193}])-([A-Za-z][A-Za-z0-9_.]*(?:/[A-Za-z0-9_.]+)?)"));
	s = replaceAll(s, element, [](const QRegularExpressionMatch &m) {
		return m.captured(1) + labelVariable(QStringLiteral("-") + m.captured(2));
	});
	return s;
}

QString Grafcet::localized(const QString &structured, int part) const
{
	// R14: a step number used in more than one partial grafcet is a variable of its own per partial grafcet;
	// inside a partial grafcet X3 means its own step 3
	// a step of another partial grafcet: G1.X2 (FluidSIM notation)
	static const QRegularExpression cross(QStringLiteral("(?<![A-Za-z0-9_.])[Gg]([A-Za-z0-9_]+)\\.[Xx]([A-Za-z0-9]+)(?![A-Za-z0-9_])"));
	const QString crossed = replaceAll(structured, cross, [&](const QRegularExpressionMatch &m) {
		const QString g = QStringLiteral("G") + m.captured(1).toUpper(), n = m.captured(2).toUpper();
		for (const Partial *p : m_parts)
			if (p->name == g)
				for (const Step *s : p->steps)
					if (s->number == n) return s->var;
		return m.captured(0);
	});
	if (part < 0 || part >= m_parts.size()) return crossed;
	static const QRegularExpression step_ref(QStringLiteral("(?<![A-Za-z0-9_.])[Xx]([A-Za-z0-9]+)(?![A-Za-z0-9_])"));
	return replaceAll(crossed, step_ref, [&](const QRegularExpressionMatch &m) {
		const QString n = m.captured(1).toUpper();
		for (const Step *s : m_parts.at(part)->steps)
			if (s->number == n) return s->var;
		return m.captured(0);
	});
}

Grafcet::Grafcet(const QList<Diagram *> &diagrams, const std::function<int(Terminal *)> &node)
{
	// net keys: the simulation's nets; open terminals get keys of their own
	QHash<Terminal *, int> key;
	int next_open = -2;
	QList<QPair<Element *, Kind>> elements;
	QList<Terminal *> all;
	for (Diagram *d : diagrams)
		for (Element *e : sortedElements(d)) {
			const Kind k = kindOf(e);
			if (k == Kind::None) continue;
			elements << qMakePair(e, k);
			for (Terminal *t : e->terminals()) {
				const int n = node(t);
				key.insert(t, n >= 0 ? n : next_open--);
				all << t;
			}
		}
	if (elements.isEmpty()) return;

	QHash<int, int> parent;
	std::function<int(int)> root = [&](int a) {
		while (parent.value(a, a) != a) {
			const int up = parent.value(a, a);
			parent[a] = parent.value(up, up);
			a = parent[a];
		}
		return a;
	};
	auto join = [&](int a, int b) {
		const int ra = root(a), rb = root(b);
		if (ra != rb) parent.insert(ra, rb);
	};
	// branch bars and junctions connect all their terminals
	for (const auto &ek : elements)
		if (ek.second == Kind::Bar || ek.second == Kind::Junction) {
			const QList<Terminal *> ts = ek.first->terminals();
			for (int i = 1; i < ts.size(); ++i) join(key.value(ts.at(0)), key.value(ts.at(i)));
		}
	// symbols placed terminal on terminal are connected too
	for (int i = 0; i < all.size(); ++i)
		for (int j = i + 1; j < all.size(); ++j)
			if (all.at(i)->diagram() == all.at(j)->diagram()
				&& QLineF(all.at(i)->dockConductor(), all.at(j)->dockConductor()).length() < 2.0)
				join(key.value(all.at(i)), key.value(all.at(j)));

	auto terminalKey = [&](Element *e, Qet::Orientation o) {
		for (Terminal *t : e->terminals())
			if (t->orientation() == o) return root(key.value(t));
		return next_open--;     // no such terminal: a key nothing matches
	};

	struct Jump { int in; QString target; };
	QList<Jump> jumps;
	for (const auto &ek : elements) {
		Element *e = ek.first;
		switch (ek.second) {
			case Kind::InitialStep:
			case Kind::Step: {
				auto *s = new Step;
				s->e = e;
				s->initial = ek.second == Kind::InitialStep || info(e, "sim_initial") == QLatin1String("1")
							 || info(e, "sim_activation") == QLatin1String("1");
				const QString file = QFileInfo(e->location().toString()).fileName();
				s->enclosing = file.startsWith(QLatin1String("20202002"));
				s->macro = file.startsWith(QLatin1String("20202003"));
				s->number = stepNumber(firstText(e));
				if (s->number.isEmpty()) {
					s->number = QStringLiteral("?%1").arg(m_steps.size() + 1);
					m_errors << QStringLiteral("GRAFCET: Schritt ohne Nummer (Kennzeichen)");
				}
				const QString enc = info(e, "sim_encloses");
				if (!enc.isEmpty()) s->encloses = enc.split(QRegularExpression(QStringLiteral("[,; ]+")), Qt::SkipEmptyParts);
				s->in = terminalKey(e, Qet::North);
				s->out = terminalKey(e, Qet::South);
				s->act = terminalKey(e, Qet::East);
				m_steps << s;
				break;
			}
			case Kind::Transition: {
				auto *t = new Transition;
				t->e = e;
				t->text = firstText(e);
				t->in = terminalKey(e, Qet::North);
				t->out = terminalKey(e, Qet::South);
				if (t->text.isEmpty())
					m_errors << QStringLiteral("GRAFCET: Transition ohne Weiterschaltbedingung (gilt als 1)");
				m_transitions << t;
				break;
			}
			case Kind::Action:
			case Kind::ConditionalAction:
			case Kind::ActionOnActivation:
			case Kind::ActionOnDeactivation:
			case Kind::ActionOnEvent: {
				auto *a = new Action;
				a->e = e;
				a->kind = ek.second;
				a->key = terminalKey(e, Qet::West);
				a->text = boxText(e);
				if (a->kind == Kind::ConditionalAction || a->kind == Kind::ActionOnEvent) {
					a->condition_text = textAbove(e);
					if (a->condition_text.isEmpty())
						m_errors << QStringLiteral("GRAFCET: Aktion „%1“ ohne Bedingung über dem Kasten").arg(a->text);
				}
				// R14 forcing order: G5{8, 9}, G5{}, G5{*} (freeze), G5{INIT}
				static const QRegularExpression forcing(QStringLiteral("^\\s*(G[A-Za-z0-9_]+)\\s*\\{([^}]*)\\}\\s*$"));
				const auto fm = forcing.match(a->text);
				if (a->kind == Kind::Action && fm.hasMatch()) {
					a->force_target = fm.captured(1).toUpper();
					const QString inner = fm.captured(2).trimmed();
					if (inner == QLatin1String("*")) a->force = Action::Force::Freeze;
					else if (inner.compare(QLatin1String("INIT"), Qt::CaseInsensitive) == 0) a->force = Action::Force::Init;
					else {
						a->force = Action::Force::Situation;
						for (const QString &n : inner.split(QRegularExpression(QStringLiteral("[,; ]+")), Qt::SkipEmptyParts))
							a->force_steps << stepNumber(n);
					}
				}
				m_actions << a;
				break;
			}
			case Kind::Jump:
				for (Terminal *t : e->terminals()) jumps << Jump{root(key.value(t)), stepNumber(firstText(e))};
				break;
			default:
				break;
		}
	}

	// the chart: steps before and after every transition (jumps later, inside their partial grafcet)
	for (Transition *t : m_transitions)
		for (Step *s : m_steps) {
			if (s->out == t->in) t->before << s;
			if (s->in == t->out) t->after << s;
		}

	// R14 partial grafcets: the connected parts of the chart (steps joined by transitions); a jump stays inside the
	// partial grafcet of its transition
	QHash<Step *, Step *> up;
	std::function<Step *(Step *)> top = [&](Step *s) { while (up.value(s, s) != s) s = up.value(s, s); return s; };
	auto unite = [&](Step *a, Step *b) { Step *ra = top(a), *rb = top(b); if (ra != rb) up.insert(ra, rb); };
	for (Transition *t : m_transitions) {
		QList<Step *> both = t->before + t->after;
		for (int i = 1; i < both.size(); ++i) unite(both.at(0), both.at(i));
	}
	auto partOfTransition = [&](Transition *t) { return !t->before.isEmpty() ? top(t->before.first()) : nullptr; };
	// a jump: the step of that number in the partial grafcet of its transition, else the only one in the project
	for (Transition *t : m_transitions)
		for (const Jump &j : jumps) {
			if (j.in != t->out) continue;
			Step *found = nullptr;
			int count = 0;
			for (Step *s : m_steps)
				if (s->number == j.target) {
					++count;
					if (!found || (partOfTransition(t) && top(s) == partOfTransition(t))) found = s;
				}
			if (count > 1 && !(found && partOfTransition(t) && top(found) == partOfTransition(t))) found = nullptr;
			if (found) {
				if (!t->after.contains(found)) t->after << found;
				if (!t->before.isEmpty()) unite(t->before.first(), found);
			} else {
				m_errors << QStringLiteral("GRAFCET: Sprung zu Schritt %1, den es nicht gibt").arg(j.target);
			}
		}
	// chains, then partial grafcets: all chains with the same name sim_grafcet (FluidSIM: the steps inside one
	// Teil-GRAFCET frame) form one partial grafcet; the chains without a name form the global grafcet (name empty)
	QHash<Step *, int> index;
	QList<QString> comp_name;
	for (Step *s : m_steps) {
		Step *r = top(s);
		if (!index.contains(r)) {
			index.insert(r, static_cast<int>(comp_name.size()));
			comp_name << QString();
		}
		s->comp = index.value(r);
		const QString n = info(s->e, "sim_grafcet").toUpper();
		if (n.isEmpty()) continue;
		if (!comp_name.at(s->comp).isEmpty() && comp_name.at(s->comp) != n)
			m_errors << QStringLiteral("GRAFCET: Kette mit zwei Teilgrafcet-Namen (%1, %2)").arg(comp_name.at(s->comp), n);
		comp_name[s->comp] = n;
	}
	for (Step *s : m_steps) {
		const QString n = comp_name.at(s->comp);
		int p = -1;
		for (int i = 0; i < m_parts.size(); ++i)
			if (m_parts.at(i)->name == n) p = i;
		if (p < 0) {
			p = static_cast<int>(m_parts.size());
			m_parts << new Partial;
			m_parts.last()->name = n;
		}
		s->part = p;
		m_parts.at(p)->steps << s;
	}
	auto partByName = [&](const QString &n) {
		for (int i = 0; i < m_parts.size(); ++i)
			if (!n.isEmpty() && m_parts.at(i)->name.compare(n, Qt::CaseInsensitive) == 0) return i;
		return -1;
	};
	// step variables: X<n> in the global grafcet and when the number is unique, else <G>_X<n> (instances, R14)
	QHash<QString, int> uses;
	for (Step *s : m_steps) ++uses[s->number];
	for (Step *s : m_steps) {
		const bool unique = uses.value(s->number) == 1 || m_parts.at(s->part)->name.isEmpty();
		s->var = PlcProgram::normalized(unique ? QStringLiteral("X") + s->number
											   : m_parts.at(s->part)->name + QStringLiteral("_X") + s->number);
		s->shown = unique ? s->number : m_parts.at(s->part)->name + QLatin1Char('.') + s->number;
	}
	for (Partial *p : m_parts) {
		QSet<QString> seen;
		for (const Step *s : p->steps) {
			if (seen.contains(s->number))
				m_errors << QStringLiteral("GRAFCET: Schritt %1 gibt es mehrmals in %2")
								.arg(s->number, p->name.isEmpty() ? QStringLiteral("dem globalen GRAFCET") : p->name);
			seen << s->number;
		}
	}
	// R14 enclosing steps: sim_encloses names the enclosed partial grafcets (default G<number of the step>);
	// in an enclosed partial grafcet the initial steps are its activation steps (IEC 60848: marked *)
	for (Step *s : m_steps) {
		if (!s->enclosing) continue;
		if (s->encloses.isEmpty()) s->encloses << QStringLiteral("G") + s->number;
		for (const QString &n : std::as_const(s->encloses)) {
			const int p = partByName(n);
			if (p < 0 || p == s->part) {
				m_errors << QStringLiteral("GRAFCET: einschließender Schritt %1 – Teilgrafcet %2 fehlt").arg(s->shown, n);
				continue;
			}
			if (m_parts.at(p)->encloser)
				m_errors << QStringLiteral("GRAFCET: Teilgrafcet %1 ist zweimal eingeschlossen").arg(n);
			m_parts.at(p)->encloser = s;
			bool any = false;
			for (const Step *x : m_parts.at(p)->steps) any = any || x->initial;
			if (!any) m_errors << QStringLiteral("GRAFCET: eingeschlossener Teilgrafcet %1 ohne Aktivierungsschritt (Anfangsschritt-Symbol)").arg(n);
		}
	}
	// R14 macro steps M<n>: expansion from the input step E<n> to the output step S<n>
	for (Step *s : m_steps) {
		if (!s->macro) continue;
		QString digits = s->number;
		if (digits.startsWith(QLatin1Char('M'))) digits.remove(0, 1);
		for (Step *x : m_steps) {
			if (x->number == QStringLiteral("E") + digits) s->macro_in = x;
			if (x->number == QStringLiteral("S") + digits) s->macro_out = x;
		}
		if (!s->macro_in || !s->macro_out || s->macro_in->comp != s->macro_out->comp) {
			m_errors << QStringLiteral("GRAFCET: Makroschritt %1 ohne Expansion E%2 … S%2").arg(s->shown, digits);
			s->macro_in = s->macro_out = nullptr;
			continue;
		}
		s->expansion = s->macro_in->comp;
		for (const Step *x : std::as_const(m_steps))
			if (x->comp == s->expansion && x->initial)
				m_errors << QStringLiteral("GRAFCET: Expansion von %1 hat einen Anfangsschritt").arg(s->shown);
	}

	// receptivities and actions, each in its partial grafcet
	for (Transition *t : m_transitions) {
		if (t->before.isEmpty())
			m_errors << QStringLiteral("GRAFCET: Transition „%1“ hat keinen Schritt davor").arg(t->text);
		if (t->after.isEmpty())
			m_errors << QStringLiteral("GRAFCET: Transition „%1“ hat keinen Schritt danach").arg(t->text);
		const int part = t->before.isEmpty() ? -1 : t->before.first()->part;
		t->condition = std::make_shared<PlcProgram>();
		QString error;
		if (!t->condition->compileExpression(localized(toStructuredText(t->text), part), &error)) {
			m_errors << QStringLiteral("GRAFCET: Transition „%1“: %2").arg(t->text, error);
			t->condition.reset();
		}
	}
	for (Action *a : m_actions) {
		for (Step *s : m_steps)
			if (s->act == a->key) a->step = s;
		if (!a->step) {
			// beside a step without a wire: the nearest step to the left on the same height
			const QPointF p = a->e->scenePos();
			double best = 1e9;
			for (Step *s : m_steps) {
				const QPointF q = s->e->scenePos();
				if (s->e->diagram() == a->e->diagram() && std::abs(q.y() - p.y()) < 25 && q.x() < p.x()
					&& p.x() - q.x() < best) {
					best = p.x() - q.x();
					a->step = s;
				}
			}
		}
		if (!a->step) m_errors << QStringLiteral("GRAFCET: Aktion „%1“ gehört zu keinem Schritt").arg(a->text);
		const int part = a->step ? a->step->part : -1;
		QString error;
		if (a->force != Action::Force::None) {
			const int p = partByName(a->force_target);
			if (p < 0) m_errors << QStringLiteral("GRAFCET: Zwangssteuerung „%1“ – Teilgrafcet %2 fehlt").arg(a->text, a->force_target);
			else if (p == part) m_errors << QStringLiteral("GRAFCET: Zwangssteuerung „%1“ zwingt den eigenen Teilgrafcet").arg(a->text);
			else
				for (const QString &n : std::as_const(a->force_steps)) {
					bool ok = false;
					for (const Step *s : m_parts.at(p)->steps) ok = ok || s->number == n;
					if (!ok) m_errors << QStringLiteral("GRAFCET: Zwangssteuerung „%1“ – Schritt %2 nicht in %3").arg(a->text, n, a->force_target);
				}
		} else if (a->kind == Kind::Action || a->kind == Kind::ConditionalAction) {
			const QStringList parts = a->text.split(QRegularExpression(QStringLiteral("[,;\\n]")), Qt::SkipEmptyParts);
			for (const QString &raw : parts) {
				const QString item = toStructuredText(raw).trimmed();
				if (item.contains(QLatin1String(":=")))
					m_errors << QStringLiteral("GRAFCET: kontinuierliche Aktion „%1“ nennt nur Variablen; "
											   "Zuweisungen gehören in eine speichernde Aktion").arg(a->text);
				else if (!item.isEmpty())
					a->outputs << PlcProgram::normalized(item);
			}
		} else {
			a->program = std::make_shared<PlcProgram>();
			QString source = localized(toStructuredText(a->text), part);
			if (!source.trimmed().endsWith(QLatin1Char(';'))) source += QLatin1Char(';');
			if (!a->program->compile(source, &error)) {
				m_errors << QStringLiteral("GRAFCET: Aktion „%1“: %2").arg(a->text, error);
				a->program.reset();
			}
		}
		if (a->kind == Kind::ConditionalAction || a->kind == Kind::ActionOnEvent) {
			a->condition = std::make_shared<PlcProgram>();
			if (!a->condition->compileExpression(localized(toStructuredText(a->condition_text), part), &error)) {
				m_errors << QStringLiteral("GRAFCET: Bedingung „%1“: %2").arg(a->condition_text, error);
				a->condition.reset();
			}
		}
		for (const QString &o : std::as_const(a->outputs))
			if (!m_continuous.contains(o)) m_continuous << o;
	}
	bool any_initial = false;
	for (Step *s : m_steps) any_initial = any_initial || (s->initial && !m_parts.at(s->part)->encloser);
	if (!m_steps.isEmpty() && !any_initial)
		m_errors << QStringLiteral("GRAFCET: kein Anfangsschritt (Doppelquadrat)");
}

Grafcet::~Grafcet()
{
	qDeleteAll(m_steps);
	qDeleteAll(m_transitions);
	qDeleteAll(m_actions);
	qDeleteAll(m_parts);
}

void Grafcet::activate(Step *s, double time, QHash<QString, double> &vars, QStringList *messages)
{
	if (s->active) return;
	s->active = true;
	s->since = time;
	vars.insert(s->var, 1.0);
	if (!m_parts.at(s->part)->name.isEmpty()) vars.insert(PlcProgram::normalized(QStringLiteral("X") + m_parts.at(s->part)->name), 1.0);
	vars.insert(s->var + QStringLiteral(".T"), 0.0);
	if (messages) *messages << QStringLiteral("GRAFCET Schritt %1 aktiv").arg(s->shown);
	for (Action *a : std::as_const(m_actions))
		if (a->step == s && a->kind == Kind::ActionOnActivation && a->program) a->program->cycle(time, vars);
	// R14: an enclosing step activates the activation steps of its enclosed partial grafcets, a macro step its input step
	for (Partial *p : std::as_const(m_parts))
		if (p->encloser == s)
			for (Step *x : std::as_const(p->steps))
				if (x->initial) activate(x, time, vars, messages);
	if (s->macro_in) activate(s->macro_in, time, vars, messages);
}

void Grafcet::deactivate(Step *s, double time, QHash<QString, double> &vars)
{
	if (!s->active) return;
	s->active = false;
	vars.insert(s->var, 0.0);
	bool any = false;
	for (const Step *x : std::as_const(m_parts.at(s->part)->steps)) any = any || x->active;
	if (!m_parts.at(s->part)->name.isEmpty()) vars.insert(PlcProgram::normalized(QStringLiteral("X") + m_parts.at(s->part)->name), any ? 1.0 : 0.0);
	vars.insert(s->var + QStringLiteral(".T"), 0.0);
	for (Action *a : std::as_const(m_actions))
		if (a->step == s && a->kind == Kind::ActionOnDeactivation && a->program) a->program->cycle(time, vars);
	// R14: leaving an enclosing step deactivates everything it encloses, leaving a macro step its expansion
	for (Partial *p : std::as_const(m_parts))
		if (p->encloser == s)
			for (Step *x : std::as_const(p->steps)) deactivate(x, time, vars);
	if (s->expansion >= 0)
		for (Step *x : std::as_const(m_steps))
			if (x->comp == s->expansion) deactivate(x, time, vars);
}

bool Grafcet::force(double time, QHash<QString, double> &vars, QStringList *messages)
{
	// R14 forcing orders of the active steps, applied before the evolution: the forced partial grafcet takes the
	// given situation and does not evolve in this cycle (IEC 60848 forcing: G5{8}, G5{}, G5{*}, G5{INIT})
	bool changed = false;
	for (Action *a : std::as_const(m_actions)) {
		if (a->force == Action::Force::None || !a->step || !a->step->active) continue;
		Partial *target = nullptr;
		for (Partial *p : std::as_const(m_parts))
			if (p->name == a->force_target) target = p;
		if (!target) continue;
		target->frozen = true;
		if (a->force == Action::Force::Freeze) continue;
		QSet<Step *> want;
		for (Step *s : std::as_const(target->steps))
			if (a->force == Action::Force::Init ? s->initial : a->force_steps.contains(s->number)) want << s;
		const QString as = a->text.trimmed();
		if (target->forced_as != as && messages) *messages << QStringLiteral("GRAFCET %1 erzwungen: %2").arg(target->name, as);
		target->forced_as = as;
		for (Step *s : std::as_const(target->steps))
			if (s->active && !want.contains(s)) { deactivate(s, time, vars); changed = true; }
		for (Step *s : std::as_const(target->steps))
			if (!s->active && want.contains(s)) { activate(s, time, vars, messages); changed = true; }
	}
	return changed;
}

void Grafcet::cycle(double time, QHash<QString, double> &vars, QStringList *messages)
{
	if (m_steps.isEmpty()) return;
	if (!m_started) {
		m_started = true;
		for (Step *s : std::as_const(m_steps)) {
			vars.insert(s->var, 0.0);
			vars.insert(s->var + QStringLiteral(".T"), 0.0);
		}
		for (const Partial *p : std::as_const(m_parts))
			if (!p->name.isEmpty()) vars.insert(PlcProgram::normalized(QStringLiteral("X") + p->name), 0.0);
		// enclosed partial grafcets and macro expansions start with their enclosing (macro) step only
		for (Step *s : std::as_const(m_steps))
			if (s->initial && !m_parts.at(s->part)->encloser) activate(s, time, vars, messages);
	}
	for (Step *s : std::as_const(m_steps))
		if (s->active) vars.insert(s->var + QStringLiteral(".T"), time - s->since);

	bool stable = false;
	for (int round = 0; round < 100 && !stable; ++round) {
		for (Partial *p : std::as_const(m_parts)) p->frozen = false;
		const bool forced = force(time, vars, messages);
		// every receptivity is evaluated each cycle, so edges are seen even before the transition is enabled
		for (Transition *t : std::as_const(m_transitions))
			t->value = t->condition ? t->condition->evaluate(time, vars) : 0.0;
		QList<Transition *> fire;
		for (Transition *t : std::as_const(m_transitions)) {
			if (t->before.isEmpty() || t->value == 0.0) continue;
			if (m_parts.at(t->before.first()->part)->frozen) continue;
			bool enabled = true;
			// a macro step lets its transition fire only when the output step of its expansion is active
			for (Step *s : std::as_const(t->before))
				enabled = enabled && s->active && (!s->macro_out || s->macro_out->active);
			if (enabled) fire << t;
		}
		if (fire.isEmpty() && !forced) {
			stable = true;
			break;
		}
		// R14: a transition inside a partial grafcet whose enclosing (macro) step is left in this round does not fire
		QSet<Step *> leaving;
		for (Transition *t : std::as_const(fire))
			for (Step *s : std::as_const(t->before)) leaving << s;
		for (Transition *t : std::as_const(fire))
			for (Step *s : std::as_const(t->after)) leaving.remove(s);
		for (int i = static_cast<int>(fire.size()) - 1; i >= 0; --i) {
			const Step *first = fire.at(i)->before.first();
			bool drop = m_parts.at(first->part)->encloser && leaving.contains(m_parts.at(first->part)->encloser);
			for (const Step *m : std::as_const(leaving))
				drop = drop || (m->expansion >= 0 && m->expansion == first->comp && !fire.at(i)->before.contains(m->macro_out));
			if (drop) fire.removeAt(i);
		}
		// all fireable transitions fire at once; a step both left and entered stays active
		QSet<Step *> off, on;
		for (Transition *t : std::as_const(fire)) {
			for (Step *s : std::as_const(t->before)) {
				off << s;
				if (s->macro_out) off << s->macro_out;
			}
			for (Step *s : std::as_const(t->after)) on << s;
		}
		for (Step *s : std::as_const(m_steps))
			if (off.contains(s) && !on.contains(s)) deactivate(s, time, vars);
		for (Step *s : std::as_const(m_steps))
			if (on.contains(s) && !s->active) activate(s, time, vars, messages);
	}
	if (!stable && !m_unstable_reported) {
		m_unstable_reported = true;
		if (messages)
			*messages << QStringLiteral("GRAFCET schaltet ohne Ende weiter (Schleife, deren Bedingungen alle erfüllt sind)");
	}

	// actions of the stable situation
	for (Action *a : std::as_const(m_actions))
		if (a->kind == Kind::ActionOnEvent && a->step && a->step->active && a->program && a->condition
			&& a->condition->evaluate(time, vars) != 0.0)
			a->program->cycle(time, vars);
	QHash<QString, bool> want;
	for (const QString &o : std::as_const(m_continuous)) want.insert(o, false);
	for (Action *a : std::as_const(m_actions)) {
		if (a->outputs.isEmpty() || !a->step || !a->step->active) continue;
		if (a->kind == Kind::ConditionalAction && (!a->condition || a->condition->evaluate(time, vars) == 0.0))
			continue;
		for (const QString &o : std::as_const(a->outputs)) want.insert(o, true);
	}
	for (auto it = want.constBegin(); it != want.constEnd(); ++it) vars.insert(it.key(), it.value() ? 1.0 : 0.0);
	// forced partial grafcets that are free again
	for (Partial *p : std::as_const(m_parts))
		if (!p->frozen) p->forced_as.clear();
}

QList<Element *> Grafcet::activeSteps() const
{
	QList<Element *> out;
	for (Step *s : m_steps)
		if (s->active) out << s->e;
	return out;
}
