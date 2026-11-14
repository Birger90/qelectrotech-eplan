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
	QString number, var;
	int in = 0, out = 0, act = 0;       // net keys
	bool initial = false, active = false;
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
	QStringList outputs;                 // continuous actions
	std::shared_ptr<PlcProgram> program;  // stored actions
	std::shared_ptr<PlcProgram> condition;
};

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

Grafcet::Grafcet(const QList<Diagram *> &diagrams, const std::function<int(Terminal *)> &node)
{
	// net keys: the simulation's nets; open terminals get keys of their own
	QHash<Terminal *, int> key;
	int next_open = -2;
	QList<QPair<Element *, Kind>> elements;
	QList<Terminal *> all;
	for (Diagram *d : diagrams)
		for (Element *e : d->elements()) {
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
	QHash<QString, Step *> by_number;
	for (const auto &ek : elements) {
		Element *e = ek.first;
		switch (ek.second) {
			case Kind::InitialStep:
			case Kind::Step: {
				auto *s = new Step;
				s->e = e;
				s->initial = ek.second == Kind::InitialStep;
				s->number = stepNumber(firstText(e));
				if (s->number.isEmpty()) {
					s->number = QStringLiteral("?%1").arg(m_steps.size() + 1);
					m_errors << QStringLiteral("GRAFCET: Schritt ohne Nummer (Kennzeichen)");
				}
				s->var = PlcProgram::normalized(QStringLiteral("X") + s->number);
				if (by_number.contains(s->number))
					m_errors << QStringLiteral("GRAFCET: Schritt %1 gibt es mehrmals").arg(s->number);
				by_number.insert(s->number, s);
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
				t->condition = std::make_shared<PlcProgram>();
				QString error;
				if (t->text.isEmpty())
					m_errors << QStringLiteral("GRAFCET: Transition ohne Weiterschaltbedingung (gilt als 1)");
				if (!t->condition->compileExpression(toStructuredText(t->text), &error)) {
					m_errors << QStringLiteral("GRAFCET: Transition „%1“: %2").arg(t->text, error);
					t->condition.reset();
				}
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
				const QString text = boxText(e);
				QString error;
				if (a->kind == Kind::Action || a->kind == Kind::ConditionalAction) {
					const QStringList parts = text.split(QRegularExpression(QStringLiteral("[,;\\n]")), Qt::SkipEmptyParts);
					for (const QString &raw : parts) {
						const QString part = toStructuredText(raw).trimmed();
						if (part.contains(QLatin1String(":=")))
							m_errors << QStringLiteral("GRAFCET: kontinuierliche Aktion „%1“ nennt nur Variablen; "
													   "Zuweisungen gehören in eine speichernde Aktion").arg(text);
						else if (!part.isEmpty())
							a->outputs << PlcProgram::normalized(part);
					}
				} else {
					a->program = std::make_shared<PlcProgram>();
					QString source = toStructuredText(text);
					if (!source.trimmed().endsWith(QLatin1Char(';'))) source += QLatin1Char(';');
					if (!a->program->compile(source, &error)) {
						m_errors << QStringLiteral("GRAFCET: Aktion „%1“: %2").arg(text, error);
						a->program.reset();
					}
				}
				if (a->kind == Kind::ConditionalAction || a->kind == Kind::ActionOnEvent) {
					const QString cond = textAbove(e);
					a->condition = std::make_shared<PlcProgram>();
					if (cond.isEmpty())
						m_errors << QStringLiteral("GRAFCET: Aktion „%1“ ohne Bedingung über dem Kasten").arg(text);
					if (!a->condition->compileExpression(toStructuredText(cond), &error)) {
						m_errors << QStringLiteral("GRAFCET: Bedingung „%1“: %2").arg(cond, error);
						a->condition.reset();
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

	// the chart: steps before and after every transition
	for (Transition *t : m_transitions) {
		for (Step *s : m_steps) {
			if (s->out == t->in) t->before << s;
			if (s->in == t->out) t->after << s;
		}
		for (const Jump &j : jumps)
			if (j.in == t->out) {
				if (Step *s = by_number.value(j.target)) {
					if (!t->after.contains(s)) t->after << s;
				} else {
					m_errors << QStringLiteral("GRAFCET: Sprung zu Schritt %1, den es nicht gibt").arg(j.target);
				}
			}
		if (t->before.isEmpty())
			m_errors << QStringLiteral("GRAFCET: Transition „%1“ hat keinen Schritt davor").arg(t->text);
		if (t->after.isEmpty())
			m_errors << QStringLiteral("GRAFCET: Transition „%1“ hat keinen Schritt danach").arg(t->text);
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
		if (!a->step) m_errors << QStringLiteral("GRAFCET: Aktion „%1“ gehört zu keinem Schritt").arg(boxText(a->e));
		for (const QString &o : std::as_const(a->outputs))
			if (!m_continuous.contains(o)) m_continuous << o;
	}
	bool any_initial = false;
	for (Step *s : m_steps) any_initial = any_initial || s->initial;
	if (!m_steps.isEmpty() && !any_initial)
		m_errors << QStringLiteral("GRAFCET: kein Anfangsschritt (Doppelquadrat)");
}

Grafcet::~Grafcet()
{
	qDeleteAll(m_steps);
	qDeleteAll(m_transitions);
	qDeleteAll(m_actions);
}

void Grafcet::activate(Step *s, double time, QHash<QString, double> &vars, QStringList *messages)
{
	s->active = true;
	s->since = time;
	vars.insert(s->var, 1.0);
	vars.insert(s->var + QStringLiteral(".T"), 0.0);
	if (messages) *messages << QStringLiteral("GRAFCET Schritt %1 aktiv").arg(s->number);
	for (Action *a : std::as_const(m_actions))
		if (a->step == s && a->kind == Kind::ActionOnActivation && a->program) a->program->cycle(time, vars);
}

void Grafcet::deactivate(Step *s, double time, QHash<QString, double> &vars)
{
	s->active = false;
	vars.insert(s->var, 0.0);
	vars.insert(s->var + QStringLiteral(".T"), 0.0);
	for (Action *a : std::as_const(m_actions))
		if (a->step == s && a->kind == Kind::ActionOnDeactivation && a->program) a->program->cycle(time, vars);
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
		for (Step *s : std::as_const(m_steps))
			if (s->initial) activate(s, time, vars, messages);
	}
	for (Step *s : std::as_const(m_steps))
		if (s->active) vars.insert(s->var + QStringLiteral(".T"), time - s->since);

	bool stable = false;
	for (int round = 0; round < 100 && !stable; ++round) {
		// every receptivity is evaluated each cycle, so edges are seen even before the transition is enabled
		for (Transition *t : std::as_const(m_transitions))
			t->value = t->condition ? t->condition->evaluate(time, vars) : 0.0;
		QList<Transition *> fire;
		for (Transition *t : std::as_const(m_transitions)) {
			if (t->before.isEmpty() || t->value == 0.0) continue;
			bool enabled = true;
			for (Step *s : std::as_const(t->before)) enabled = enabled && s->active;
			if (enabled) fire << t;
		}
		if (fire.isEmpty()) {
			stable = true;
			break;
		}
		// all fireable transitions fire at once; a step both left and entered stays active
		QSet<Step *> off, on;
		for (Transition *t : std::as_const(fire)) {
			for (Step *s : std::as_const(t->before)) off << s;
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
}

QList<Element *> Grafcet::activeSteps() const
{
	QList<Element *> out;
	for (Step *s : m_steps)
		if (s->active) out << s->e;
	return out;
}
