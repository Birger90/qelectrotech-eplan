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
#include "wirenumberpattern.h"

#include "../QPropertyUndoCommand/qpropertyundocommand.h"
#include "../diagram.h"
#include "../diagramcontext.h"
#include "../diagramposition.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/terminal.h"
#include "../qetproject.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <QUndoCommand>
#include <algorithm>

namespace {

const QString key_active  = QStringLiteral("wire_number_pattern_active");
const QString key_pattern = QStringLiteral("wire_number_pattern");
const QString key_scope   = QStringLiteral("wire_number_pattern_scope");
const QString key_reserve = QStringLiteral("wire_number_pattern_reserve");
const QString key_order   = QStringLiteral("wire_number_pattern_order");
const QString key_mode    = QStringLiteral("wire_number_pattern_mode");
const QString key_start   = QStringLiteral("wire_number_pattern_start");

	/// {name} or {name:option}
const QRegularExpression &tokenRe()
{
	static const QRegularExpression re(QStringLiteral("\\{([A-Za-z]+)(?::([0-9]+|[Aa]))?\\}"));
	return re;
}

QString canonical(const QString &name)
{
	const QString n = name.toLower();
	if (n == QLatin1String("seite") || n == QLatin1String("page")) return QStringLiteral("page");
	if (n == QLatin1String("spalte") || n == QLatin1String("column")) return QStringLiteral("column");
	if (n == QLatin1String("zeile") || n == QLatin1String("row")) return QStringLiteral("row");
	if (n == QLatin1String("schrank") || n == QLatin1String("cabinet")) return QStringLiteral("cabinet");
	if (n == QLatin1String("anlage") || n == QLatin1String("plant")) return QStringLiteral("plant");
	if (n == QLatin1String("n")) return QStringLiteral("n");
	return QString();
}

QString scopeText(WireNumberPattern::Scope s)
{
	switch (s) {
		case WireNumberPattern::Scope::Cabinet: return QStringLiteral("cabinet");
		case WireNumberPattern::Scope::Project: return QStringLiteral("project");
		default: return QStringLiteral("folio");
	}
}

WireNumberPattern::Scope scopeOf(const QString &t, WireNumberPattern::Scope fallback)
{
	const QString s = t.toLower();
	if (s == QLatin1String("folio") || s == QLatin1String("seite") || s == QLatin1String("page"))
		return WireNumberPattern::Scope::Folio;
	if (s == QLatin1String("cabinet") || s == QLatin1String("schrank"))
		return WireNumberPattern::Scope::Cabinet;
	if (s == QLatin1String("project") || s == QLatin1String("projekt"))
		return WireNumberPattern::Scope::Project;
	return fallback;
}

	/// Where a wire starts on its folio: the upper, then the left, of its
	/// ends -- by rows; the left, then the upper, by columns
QPointF startOf(const Conductor *wire, WireNumberPattern::Order order)
{
	const QPointF a = wire->terminal1 ? wire->terminal1->dockConductor() : QPointF();
	const QPointF b = wire->terminal2 ? wire->terminal2->dockConductor() : QPointF();
	const bool a_first = order == WireNumberPattern::Order::Rows
			? (a.y() < b.y() || (a.y() == b.y() && a.x() <= b.x()))
			: (a.x() < b.x() || (a.x() == b.x() && a.y() <= b.y()));
	return a_first ? a : b;
}

bool before(const QPointF &a, const QPointF &b, WireNumberPattern::Order order)
{
	if (order == WireNumberPattern::Order::Rows) {
		if (a.y() != b.y()) return a.y() < b.y();
		return a.x() < b.x();
	}
	if (a.x() != b.x()) return a.x() < b.x();
	return a.y() < b.y();
}

QString folioText(Diagram *d)
{
	const QString f = d->border_and_titleblock.finalfolio().trimmed();
	return f.isEmpty() ? QString::number(d->folioIndex() + 1) : f;
}

QString stripped(QString text, QChar prefix)
{
	text = text.trimmed();
	while (text.startsWith(prefix)) text.remove(0, 1);
	return text.trimmed();
}

	/// A regular expression matching every text @p pattern can give
QRegularExpression matcherOf(const QString &pattern)
{
	QString re = QStringLiteral("^");
	int last = 0;
	auto it = tokenRe().globalMatch(pattern);
	while (it.hasNext()) {
		const auto m = it.next();
		re += QRegularExpression::escape(pattern.mid(last, m.capturedStart() - last));
		re += canonical(m.captured(1)) == QLatin1String("n")
				? QStringLiteral("(?:[0-9]+|[A-Za-z]+)") : QStringLiteral(".*?");
		last = m.capturedEnd();
	}
	re += QRegularExpression::escape(pattern.mid(last)) + QStringLiteral("$");
	return QRegularExpression(re);
}

}

QString WireNumberPattern::letters(int n)
{
	QString out;
	while (n > 0) {
		--n;
		out.prepend(QChar('A' + n % 26));
		n /= 26;
	}
	return out;
}

QString WireNumberPattern::format(const QString &pattern, const Context &context, int n)
{
	QString out;
	int last = 0;
	auto it = tokenRe().globalMatch(pattern);
	while (it.hasNext()) {
		const auto m = it.next();
		out += pattern.mid(last, m.capturedStart() - last);
		last = m.capturedEnd();
		const QString name = canonical(m.captured(1));
		const QString option = m.captured(2);
		if (name == QLatin1String("page")) out += context.page;
		else if (name == QLatin1String("column")) out += context.column;
		else if (name == QLatin1String("row")) out += context.row;
		else if (name == QLatin1String("cabinet")) out += context.cabinet;
		else if (name == QLatin1String("plant")) out += context.plant;
		else if (name == QLatin1String("n")) {
			if (option == QLatin1String("A")) out += letters(n);
			else if (option == QLatin1String("a")) out += letters(n).toLower();
			else out += QStringLiteral("%1").arg(n, option.isEmpty() ? 1 : option.toInt(), 10, QLatin1Char('0'));
		}
		else out += m.captured(0);
	}
	out += pattern.mid(last);
	return out;
}

QString WireNumberPattern::problem(const QString &pattern)
{
	bool counter = false;
	auto it = tokenRe().globalMatch(pattern);
	while (it.hasNext()) {
		const auto m = it.next();
		const QString name = canonical(m.captured(1));
		if (name.isEmpty())
			return QCoreApplication::translate("WireNumberPattern", "Unknown field %1").arg(m.captured(0));
		if (name == QLatin1String("n")) counter = true;
	}
	if (!counter)
		return QCoreApplication::translate("WireNumberPattern", "The pattern needs the counter {n}");
	return QString();
}

WireNumberPattern::Settings WireNumberPattern::settings(QETProject *project)
{
	Settings s;
	if (!project) return s;
	const DiagramContext p = project->projectProperties();
	QVariantMap map;
	const QString prefix = QStringLiteral("wire_number_pattern_");
	for (const QString &k : {key_active, key_scope, key_reserve, key_order, key_mode, key_start})
		if (p.contains(k)) map.insert(k.mid(prefix.size()), p.value(k));
	if (p.contains(key_pattern)) map.insert(QStringLiteral("pattern"), p.value(key_pattern));
	return fromMap(map, s);
}

void WireNumberPattern::setSettings(QETProject *project, const Settings &s)
{
	if (!project || project->isReadOnly()) return;
	DiagramContext p = project->projectProperties();
	const DiagramContext old = p;
	const QVariantMap m = toMap(s);
	p.addValue(key_active, s.active ? QStringLiteral("true") : QStringLiteral("false"));
	p.addValue(key_pattern, s.pattern);
	p.addValue(key_scope, m.value(QStringLiteral("scope")).toString());
	p.addValue(key_reserve, QString::number(s.reserve));
	p.addValue(key_order, m.value(QStringLiteral("order")).toString());
	p.addValue(key_mode, m.value(QStringLiteral("mode")).toString());
	p.addValue(key_start, QString::number(s.start));
	if (p == old) return;
	project->setUnshownProjectProperties(p);
	project->setModified(true);
}

QVariantMap WireNumberPattern::toMap(const Settings &s)
{
	QVariantMap m;
	m.insert(QStringLiteral("active"), s.active);
	m.insert(QStringLiteral("pattern"), s.pattern);
	m.insert(QStringLiteral("scope"), scopeText(s.scope));
	m.insert(QStringLiteral("reserve"), s.reserve);
	m.insert(QStringLiteral("order"), s.order == Order::Rows ? QStringLiteral("rows") : QStringLiteral("columns"));
	m.insert(QStringLiteral("mode"), s.mode == Mode::Recalculate ? QStringLiteral("recalculate") : QStringLiteral("fixed"));
	m.insert(QStringLiteral("start"), s.start);
	return m;
}

WireNumberPattern::Settings WireNumberPattern::fromMap(const QVariantMap &m, const Settings &base)
{
	Settings s = base;
	if (m.contains(QStringLiteral("active"))) {
		const QString a = m.value(QStringLiteral("active")).toString().toLower();
		s.active = a == QLatin1String("true") || a == QLatin1String("1");
	}
	if (!m.value(QStringLiteral("pattern")).toString().isEmpty())
		s.pattern = m.value(QStringLiteral("pattern")).toString();
	if (m.contains(QStringLiteral("scope")))
		s.scope = scopeOf(m.value(QStringLiteral("scope")).toString(), s.scope);
	if (m.contains(QStringLiteral("reserve")))
		s.reserve = qMax(0, m.value(QStringLiteral("reserve")).toInt());
	if (m.contains(QStringLiteral("order"))) {
		const QString o = m.value(QStringLiteral("order")).toString().toLower();
		if (o == QLatin1String("rows") || o == QLatin1String("zeilen")) s.order = Order::Rows;
		else if (o == QLatin1String("columns") || o == QLatin1String("spalten")) s.order = Order::Columns;
	}
	if (m.contains(QStringLiteral("mode"))) {
		const QString o = m.value(QStringLiteral("mode")).toString().toLower();
		if (o == QLatin1String("recalculate") || o == QLatin1String("neu") || o == QLatin1String("mitlaufend"))
			s.mode = Mode::Recalculate;
		else if (o == QLatin1String("fixed") || o == QLatin1String("fest") || o == QLatin1String("gesperrt"))
			s.mode = Mode::Fixed;
	}
	if (m.contains(QStringLiteral("start")))
		s.start = qMax(0, m.value(QStringLiteral("start")).toInt());
	return s;
}

WireNumberPattern::Plan WireNumberPattern::plan(QETProject *project, const Settings &s,
												const QList<Conductor *> &only, bool renumber)
{
	Plan result;
	if (!project || !problem(s.pattern).isEmpty()) return result;
	const QList<Diagram *> folios = project->diagrams();
	project->buildFolios(folios);
	const QRegularExpression matcher = matcherOf(s.pattern);
	static const QRegularExpression autoLike(QStringLiteral("^[0-9]+(?:[.:/-][0-9]+)*$"));
	const QSet<Conductor *> only_set(only.begin(), only.end());

	struct Potential {
		QList<Conductor *> wires;
		Diagram *folio = nullptr;
		QPointF start;
		QString text;
		bool keep = false;
	};
	QList<Potential> potentials;
	QSet<Conductor *> seen;
	for (Diagram *d : folios) {
		QList<Conductor *> wires = d->conductors();
		std::sort(wires.begin(), wires.end(), [&s](Conductor *a, Conductor *b) {
			return before(startOf(a, s.order), startOf(b, s.order), s.order);
		});
		for (Conductor *wire : std::as_const(wires)) {
			if (seen.contains(wire)) continue;
			QSet<Conductor *> group = wire->relatedPotentialConductors(true);
			group.insert(wire);
			seen.unite(group);
			Potential p;
			p.wires = QList<Conductor *>(group.begin(), group.end());
			p.folio = d;
			p.start = startOf(wire, s.order);
			bool frozen = false, touched = only_set.isEmpty();
			for (Conductor *w : std::as_const(p.wires)) {
					//"_" is what a new wire carries before anything numbers it
				const QString text = w->properties().text.trimmed();
				if (p.text.isEmpty() && text != QLatin1String("_")) p.text = text;
				if (w->isFreezeLabel()) frozen = true;
				if (only_set.contains(w)) touched = true;
			}
				//A number the pattern does not give was typed by hand
				//("L1", "+24V"): it stays, as a frozen wire does
			if (!touched || frozen)
				p.keep = true;
			else if (renumber && !only_set.isEmpty())
				p.keep = false;
			else if (s.mode == Mode::Fixed)
				p.keep = !p.text.isEmpty();
			else
					//Recalculate: a number the pattern gives, or one of digits and
					//separators only (a number given before, by another pattern),
					//is numbered anew; a name stays (L1, +24V, PE)
				p.keep = !p.text.isEmpty() && !matcher.match(p.text).hasMatch()
						&& !autoLike.match(p.text).hasMatch();
			potentials << p;
		}
	}
	result.potentials = int(potentials.size());

	QSet<QString> used;
	for (const Potential &p : std::as_const(potentials))
		if (p.keep && !p.text.isEmpty()) used.insert(p.text);

		//The first number of each folio: after the numbers of the folios
		//before it in its scope and their reserve
	QHash<QString, int> next_base;
	int i = 0;
	while (i < potentials.size()) {
		Diagram *folio = potentials.at(i).folio;
		int end = i;
		while (end < potentials.size() && potentials.at(end).folio == folio) ++end;
		QString key;
		if (s.scope == Scope::Cabinet) key = stripped(folio->border_and_titleblock.locmach(), QLatin1Char('+'));
		const int base = s.scope == Scope::Folio ? s.start : next_base.value(key, s.start);
		int n = base;
		for (int k = i; k < end; ++k) {
			const Potential &p = potentials.at(k);
			if (p.keep) { ++result.kept; continue; }
			const DiagramPosition pos = folio->convertPosition(p.start);
			Context c;
			c.page = folioText(folio);
			c.column = pos.number() ? QString::number(pos.number()) : QString();
			c.row = pos.letter();
			c.cabinet = stripped(folio->border_and_titleblock.locmach(), QLatin1Char('+'));
			c.plant = stripped(folio->border_and_titleblock.plant(), QLatin1Char('='));
			QString text;
			for (int guard = 0; guard < 100000; ++guard) {
				text = format(s.pattern, c, n++);
				if (!used.contains(text)) break;
			}
			used.insert(text);
			if (text == p.text) continue;
			Change change;
			change.wires = p.wires;
			change.old_text = p.text;
			change.new_text = text;
			result.changes << change;
		}
		if (s.scope != Scope::Folio)
			next_base.insert(key, qMax(n, base + (end - i)) + s.reserve);
		i = end;
	}
	return result;
}

QUndoCommand *WireNumberPattern::command(const Plan &plan, QUndoCommand *parent)
{
	if (plan.changes.isEmpty()) return nullptr;
	auto *macro = new QUndoCommand(QCoreApplication::translate("WireNumberPattern",
															  "Number wires by pattern"), parent);
	for (const Change &change : plan.changes)
		for (Conductor *wire : change.wires) {
			ConductorProperties old_p = wire->properties();
			ConductorProperties new_p = old_p;
			new_p.text = change.new_text;
				//The number is the pattern's, not a formula's to work out again
			new_p.m_formula.clear();
			QVariant o, n;
			o.setValue(old_p);
			n.setValue(new_p);
			new QPropertyUndoCommand(wire, "properties", o, n, macro);
		}
	return macro;
}
