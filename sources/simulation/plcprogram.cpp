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
#include "plcprogram.h"

#include <QRegularExpression>
#include <QSet>
#include <QStringList>

#include <cmath>

using Expr = std::function<double()>;

struct PlcProgram::Block {
	QString type;                       // TON, TOF, CTU, CTD, R_TRIG, F_TRIG, SR, RS
	QHash<QString, Expr> in;            // inputs by formal name
	bool q = false, last = false;
	double since = -1.0;                // TON: IN high since; TOF: IN low since
	double et = 0.0;
	double cv = 0.0;

	double input(const QString &name) const {
		const Expr e = in.value(name);
		return e ? e() : 0.0;
	}
	static QStringList inputsOf(const QString &type) {
		if (type == QLatin1String("TON") || type == QLatin1String("TOF")) return {"IN", "PT"};
		if (type == QLatin1String("CTU")) return {"CU", "R", "PV"};
		if (type == QLatin1String("CTD")) return {"CD", "LD", "PV"};
		if (type == QLatin1String("SR")) return {"S1", "R"};
		if (type == QLatin1String("RS")) return {"S", "R1"};
		return {"CLK"};                 // R_TRIG, F_TRIG
	}
	void run(double t) {
		if (type == QLatin1String("TON")) {
			const bool on = input("IN") != 0.0;
			const double pt = input("PT");
			if (on && since < 0) since = t;
			if (!on) since = -1;
			et = on ? qMin(t - since, pt) : 0.0;
			q = on && t - since >= pt - 1e-9;
		} else if (type == QLatin1String("TOF")) {
			const bool on = input("IN") != 0.0;
			const double pt = input("PT");
			if (on) { q = true; since = -1; et = 0.0; }
			else {
				if (last) since = t;
				et = since >= 0 ? qMin(t - since, pt) : 0.0;
				q = since >= 0 && t - since < pt - 1e-9;
			}
			last = on;
		} else if (type == QLatin1String("CTU")) {
			const bool cu = input("CU") != 0.0;
			if (input("R") != 0.0) cv = 0;
			else if (cu && !last) ++cv;
			q = cv >= input("PV");
			last = cu;
		} else if (type == QLatin1String("CTD")) {
			const bool cd = input("CD") != 0.0;
			if (input("LD") != 0.0) cv = input("PV");
			else if (cd && !last) --cv;
			q = cv <= 0;
			last = cd;
		} else if (type == QLatin1String("R_TRIG") || type == QLatin1String("F_TRIG")) {
			const bool clk = input("CLK") != 0.0;
			q = type == QLatin1String("R_TRIG") ? (clk && !last) : (!clk && last);
			last = clk;
		} else if (type == QLatin1String("SR")) {
			q = input("S1") != 0.0 || (q && input("R") == 0.0);
		} else if (type == QLatin1String("RS")) {
			q = input("R1") == 0.0 && (input("S") != 0.0 || q);
		}
	}
};

namespace {
struct Token { QString text; int line; };

const QStringList &blockTypes() {
	static const QStringList t{"TON", "TOF", "CTU", "CTD", "R_TRIG", "F_TRIG", "SR", "RS"};
	return t;
}
const QStringList &valueTypes() {
	static const QStringList t{"BOOL", "INT", "DINT", "SINT", "UINT", "UDINT", "WORD", "BYTE",
							   "REAL", "LREAL", "TIME"};
	return t;
}
bool isIntegerType(const QString &t) {
	return t != QLatin1String("BOOL") && t != QLatin1String("REAL") && t != QLatin1String("LREAL")
		   && t != QLatin1String("TIME");
}
const QSet<QString> &keywords() {
	static const QSet<QString> k{"IF", "THEN", "ELSIF", "ELSE", "END_IF", "CASE", "OF", "END_CASE",
								 "VAR", "END_VAR", "AND", "OR", "XOR", "NOT", "MOD", "TRUE", "FALSE"};
	return k;
}
inline double truth(bool b) { return b ? 1.0 : 0.0; }
}

struct PlcProgram::Parser {
	PlcProgram *prog;
	QVector<Token> tokens;
	int pos = 0;
	QString error;
	QSet<QString> integers;             // variables declared as an integer type
	QSet<QString> booleans;             // variables declared as BOOL

	bool atEnd() const { return pos >= tokens.size(); }
	QString peek(int ahead = 0) const {
		return pos + ahead < tokens.size() ? tokens.at(pos + ahead).text.toUpper() : QString();
	}
	int line() const {
		if (tokens.isEmpty()) return 1;
		return tokens.at(qMin(pos, int(tokens.size()) - 1)).line;
	}
	bool fail(const QString &what) {
		if (error.isEmpty()) error = QStringLiteral("Zeile %1: %2").arg(line()).arg(what);
		return false;
	}
	bool expect(const QString &t) {
		if (peek() == t.toUpper()) { ++pos; return true; }
		return fail(QStringLiteral("„%1“ erwartet, gefunden „%2“")
					.arg(t, atEnd() ? QStringLiteral("Ende") : tokens.at(pos).text));
	}
	static bool isName(const QString &t) {
		static const QRegularExpression re(QStringLiteral("^%?[A-Za-z_][A-Za-z0-9_.]*$"));
		return re.match(t).hasMatch() && !keywords().contains(t.toUpper());
	}
	static bool isNumber(const QString &t) {
		bool ok = false;
		t.toDouble(&ok);
		return ok;
	}
	static double timeLiteral(const QString &t, bool *ok) {
		static const QRegularExpression whole(QStringLiteral("^(?:T|TIME)#((?:\\d+(?:\\.\\d+)?(?:MS|S|M|H|D))+)$"),
											  QRegularExpression::CaseInsensitiveOption);
		static const QRegularExpression part(QStringLiteral("(\\d+(?:\\.\\d+)?)(MS|S|M|H|D)"),
											 QRegularExpression::CaseInsensitiveOption);
		const auto m = whole.match(t);
		*ok = m.hasMatch();
		if (!*ok) return 0.0;
		double sum = 0.0;
		auto it = part.globalMatch(m.captured(1));
		while (it.hasNext()) {
			const auto p = it.next();
			const double v = p.captured(1).toDouble();
			const QString u = p.captured(2).toUpper();
			if (u == QLatin1String("MS")) sum += v / 1000.0;
			else if (u == QLatin1String("M")) sum += v * 60.0;
			else if (u == QLatin1String("H")) sum += v * 3600.0;
			else if (u == QLatin1String("D")) sum += v * 86400.0;
			else sum += v;
		}
		return sum;
	}

	// ---------------------------------------------------------------- expressions
	// precedence from low to high: OR, XOR, AND, comparison, + -, * / MOD, unary
	Expr expr() {
		Expr left = xorExpr();
		while (left && peek() == QLatin1String("OR")) {
			++pos;
			Expr right = xorExpr();
			if (!right) return {};
			left = [left, right]() { return truth(left() != 0.0 || right() != 0.0); };
		}
		return left;
	}
	Expr xorExpr() {
		Expr left = andExpr();
		while (left && peek() == QLatin1String("XOR")) {
			++pos;
			Expr right = andExpr();
			if (!right) return {};
			left = [left, right]() { return truth((left() != 0.0) != (right() != 0.0)); };
		}
		return left;
	}
	Expr andExpr() {
		Expr left = comparison();
		while (left && (peek() == QLatin1String("AND") || peek() == QLatin1String("&"))) {
			++pos;
			Expr right = comparison();
			if (!right) return {};
			left = [left, right]() { return truth(left() != 0.0 && right() != 0.0); };
		}
		return left;
	}
	Expr comparison() {
		Expr left = additive();
		static const QStringList ops{"=", "<>", "<", ">", "<=", ">="};
		while (left && ops.contains(peek())) {
			const QString op = peek();
			++pos;
			Expr right = additive();
			if (!right) return {};
			if (op == QLatin1String("="))
				left = [left, right]() { return truth(std::fabs(left() - right()) < 1e-9); };
			else if (op == QLatin1String("<>"))
				left = [left, right]() { return truth(std::fabs(left() - right()) >= 1e-9); };
			else if (op == QLatin1String("<"))
				left = [left, right]() { return truth(left() < right() - 1e-9); };
			else if (op == QLatin1String(">"))
				left = [left, right]() { return truth(left() > right() + 1e-9); };
			else if (op == QLatin1String("<="))
				left = [left, right]() { return truth(left() <= right() + 1e-9); };
			else
				left = [left, right]() { return truth(left() >= right() - 1e-9); };
		}
		return left;
	}
	Expr additive() {
		Expr left = multiplicative();
		while (left && (peek() == QLatin1String("+") || peek() == QLatin1String("-"))) {
			const bool plus = peek() == QLatin1String("+");
			++pos;
			Expr right = multiplicative();
			if (!right) return {};
			if (plus) left = [left, right]() { return left() + right(); };
			else left = [left, right]() { return left() - right(); };
		}
		return left;
	}
	Expr multiplicative() {
		Expr left = unary();
		while (left && (peek() == QLatin1String("*") || peek() == QLatin1String("/")
						|| peek() == QLatin1String("MOD"))) {
			const QString op = peek();
			++pos;
			Expr right = unary();
			if (!right) return {};
			if (op == QLatin1String("*")) left = [left, right]() { return left() * right(); };
			else if (op == QLatin1String("/"))
				left = [left, right]() { const double r = right(); return r == 0.0 ? 0.0 : left() / r; };
			else
				left = [left, right]() { const double r = right(); return r == 0.0 ? 0.0 : std::fmod(left(), r); };
		}
		return left;
	}
	Expr unary() {
		if (peek() == QLatin1String("NOT")) {
			++pos;
			Expr inner = unary();
			if (!inner) return {};
			return [inner]() { return truth(inner() == 0.0); };
		}
		if (peek() == QLatin1String("-")) {
			++pos;
			Expr inner = unary();
			if (!inner) return {};
			return [inner]() { return -inner(); };
		}
		return primary();
	}
	Expr primary() {
		if (atEnd()) { fail(QStringLiteral("Ausdruck erwartet")); return {}; }
		const QString t = tokens.at(pos).text;
		const QString up = t.toUpper();
		if (up == QLatin1String("(")) {
			++pos;
			Expr e = expr();
			if (!e || !expect(QStringLiteral(")"))) return {};
			return e;
		}
		if (up == QLatin1String("TRUE")) { ++pos; return []() { return 1.0; }; }
		if (up == QLatin1String("FALSE")) { ++pos; return []() { return 0.0; }; }
		bool ok = false;
		const double time = timeLiteral(t, &ok);
		if (ok) { ++pos; return [time]() { return time; }; }
		if (isNumber(t)) {
			const double n = t.toDouble();
			++pos;
			return [n]() { return n; };
		}
		if (!isName(t)) { fail(QStringLiteral("unerwartet: „%1“").arg(t)); return {}; }
		++pos;
		const QString name = PlcProgram::normalized(t);
		const int dot = static_cast<int>(name.lastIndexOf(QLatin1Char('.')));
		if (dot > 0) {
			const QString inst = name.left(dot), field = name.mid(dot + 1);
			if (prog->m_blocks.contains(inst)) {
				std::shared_ptr<Block> b = prog->m_blocks.value(inst);
				if (field == QLatin1String("Q") || field == QLatin1String("Q1"))
					return [b]() { return truth(b->q); };
				if (field == QLatin1String("CV")) return [b]() { return b->cv; };
				if (field == QLatin1String("ET")) return [b]() { return b->et; };
				fail(QStringLiteral("unbekannter Ausgang %1").arg(t));
				return {};
			}
		}
		if (prog->m_blocks.contains(name)) {
			fail(QStringLiteral("%1 ist ein Baustein: Ausgang wie %1.Q angeben").arg(t));
			return {};
		}
		PlcProgram *p = prog;
		return [p, name]() { return p->m_vars ? p->m_vars->value(name, 0.0) : 0.0; };
	}

	// ---------------------------------------------------------------- statements
	/// a CASE label starts here: 1:  1,2:  1..3:  -1:
	bool atCaseLabel() const {
		int i = 0;
		if (peek() == QLatin1String("-")) i = 1;
		if (!isNumber(peek(i))) return false;
		const QString next = peek(i + 1);
		return next == QLatin1String(":") || next == QLatin1String(",") || next == QLatin1String("..");
	}

	bool block(QVector<std::function<void()>> &out, const QStringList &until, bool stop_at_label = false) {
		while (!atEnd() && !until.contains(peek())) {
			if (stop_at_label && atCaseLabel()) break;
			if (!statement(out)) return false;
		}
		return true;
	}

	bool integerLiteral(long long *value) {
		bool negative = false;
		if (peek() == QLatin1String("-")) { negative = true; ++pos; }
		bool ok = false;
		const long long v = atEnd() ? 0 : tokens.at(pos).text.toLongLong(&ok);
		if (!ok) return fail(QStringLiteral("ganze Zahl als CASE-Marke erwartet"));
		++pos;
		*value = negative ? -v : v;
		return true;
	}

	bool caseStatement(QVector<std::function<void()>> &out) {
		++pos;                          // CASE
		Expr selector = expr();
		if (!selector || !expect(QStringLiteral("OF"))) return false;
		struct Branch { QVector<QPair<long long, long long>> ranges; QVector<std::function<void()>> body; };
		QVector<Branch> branches;
		QVector<std::function<void()>> otherwise;
		while (!atEnd() && peek() != QLatin1String("ELSE") && peek() != QLatin1String("END_CASE")) {
			if (!atCaseLabel()) return fail(QStringLiteral("CASE-Marke wie „1:“ erwartet"));
			Branch br;
			while (true) {
				long long lo = 0, hi = 0;
				if (!integerLiteral(&lo)) return false;
				hi = lo;
				if (peek() == QLatin1String("..")) {
					++pos;
					if (!integerLiteral(&hi)) return false;
				}
				br.ranges << qMakePair(lo, hi);
				if (peek() != QLatin1String(",")) break;
				++pos;
			}
			if (!expect(QStringLiteral(":"))) return false;
			if (!block(br.body, {QStringLiteral("ELSE"), QStringLiteral("END_CASE")}, true)) return false;
			branches << br;
		}
		if (peek() == QLatin1String("ELSE")) {
			++pos;
			if (!block(otherwise, {QStringLiteral("END_CASE")})) return false;
		}
		if (!expect(QStringLiteral("END_CASE"))) return false;
		if (peek() == QLatin1String(";")) ++pos;
		out << [selector, branches, otherwise]() {
			const long long v = std::llround(selector());
			for (const Branch &br : branches)
				for (const auto &r : br.ranges)
					if (v >= r.first && v <= r.second) {
						for (const auto &s : br.body) s();
						return;
					}
			for (const auto &s : otherwise) s();
		};
		return true;
	}

	bool ifStatement(QVector<std::function<void()>> &out) {
		++pos;                          // IF
		QVector<QPair<Expr, QVector<std::function<void()>>>> arms;
		QVector<std::function<void()>> otherwise;
		const QStringList ends{QStringLiteral("ELSIF"), QStringLiteral("ELSE"), QStringLiteral("END_IF")};
		while (true) {
			Expr cond = expr();
			if (!cond || !expect(QStringLiteral("THEN"))) return false;
			QVector<std::function<void()>> body;
			if (!block(body, ends)) return false;
			arms << qMakePair(cond, body);
			if (peek() != QLatin1String("ELSIF")) break;
			++pos;
		}
		if (peek() == QLatin1String("ELSE")) {
			++pos;
			if (!block(otherwise, {QStringLiteral("END_IF")})) return false;
		}
		if (!expect(QStringLiteral("END_IF"))) return false;
		if (peek() == QLatin1String(";")) ++pos;
		out << [arms, otherwise]() {
			for (const auto &arm : arms)
				if (arm.first() != 0.0) {
					for (const auto &s : arm.second) s();
					return;
				}
			for (const auto &s : otherwise) s();
		};
		return true;
	}

	bool statement(QVector<std::function<void()>> &out) {
		const QString up = peek();
		if (up == QLatin1String(";")) { ++pos; return true; }
		if (up == QLatin1String("IF")) return ifStatement(out);
		if (up == QLatin1String("CASE")) return caseStatement(out);
		if (atEnd() || !isName(tokens.at(pos).text)) return fail(QStringLiteral("Anweisung erwartet"));
		const QString name = PlcProgram::normalized(tokens.at(pos).text);
		++pos;
		if (peek() == QLatin1String(":=")) {
			if (prog->m_blocks.contains(name))
				return fail(QStringLiteral("einem Baustein kann nichts zugewiesen werden: %1").arg(name));
			++pos;
			Expr e = expr();
			if (!e || !expect(QStringLiteral(";"))) return false;
			PlcProgram *p = prog;
			const bool as_int = integers.contains(name), as_bool = booleans.contains(name);
			out << [p, name, e, as_int, as_bool]() {
				if (!p->m_vars) return;
				double v = e();
				if (as_int) v = std::trunc(v);
				else if (as_bool) v = truth(v != 0.0);
				p->m_vars->insert(name, v);
			};
			return true;
		}
		if (peek() == QLatin1String("(")) {
			if (!prog->m_blocks.contains(name))
				return fail(QStringLiteral("%1 ist nicht als Baustein (%2) deklariert")
							.arg(name, blockTypes().join(QStringLiteral(", "))));
			std::shared_ptr<Block> b = prog->m_blocks.value(name);
			const QStringList allowed = Block::inputsOf(b->type);
			++pos;
			while (!atEnd() && peek() != QLatin1String(")")) {
				const QString arg = peek();
				++pos;
				if (!allowed.contains(arg))
					return fail(QStringLiteral("%1 hat keinen Eingang %2 (nur %3)")
								.arg(b->type, arg, allowed.join(QStringLiteral(", "))));
				if (!expect(QStringLiteral(":="))) return false;
				Expr e = expr();
				if (!e) return false;
				b->in.insert(arg, e);
				if (peek() == QLatin1String(",")) ++pos;
			}
			if (!expect(QStringLiteral(")")) || !expect(QStringLiteral(";"))) return false;
			PlcProgram *p = prog;
			out << [p, b]() { b->run(p->m_time); };
			return true;
		}
		return fail(QStringLiteral("„:=“ oder „(“ erwartet nach %1").arg(name));
	}

	/// VAR a, b : BOOL; Schritt : INT := 0; T1 : TON; END_VAR
	bool declarations() {
		while (peek() == QLatin1String("VAR") || peek() == QLatin1String("VAR_GLOBAL")) {
			++pos;
			while (!atEnd() && peek() != QLatin1String("END_VAR")) {
				QStringList names;
				while (true) {
					if (atEnd() || !isName(tokens.at(pos).text)) return fail(QStringLiteral("Variablenname erwartet"));
					names << PlcProgram::normalized(tokens.at(pos).text);
					++pos;
					if (peek() != QLatin1String(",")) break;
					++pos;
				}
				if (!expect(QStringLiteral(":"))) return false;
				const QString type = peek();
				++pos;
				if (blockTypes().contains(type)) {
					for (const QString &n : names) {
						auto b = std::make_shared<Block>();
						b->type = type;
						prog->m_blocks.insert(n, b);
					}
				} else if (valueTypes().contains(type)) {
					for (const QString &n : names) {
						if (type == QLatin1String("BOOL")) booleans.insert(n);
						else if (isIntegerType(type)) integers.insert(n);
					}
					if (peek() == QLatin1String(":=")) {
						++pos;
						Expr e = expr();       // a constant: names read as 0 here
						if (!e) return false;
						const double v = e();
						for (const QString &n : names) prog->m_initial.insert(n, v);
					}
				} else {
					return fail(QStringLiteral("Typ %1 wird nicht unterstützt (%2, %3)")
								.arg(type, valueTypes().join(QStringLiteral(", ")),
									 blockTypes().join(QStringLiteral(", "))));
				}
				if (!expect(QStringLiteral(";"))) return false;
			}
			if (!expect(QStringLiteral("END_VAR"))) return false;
			if (peek() == QLatin1String(";")) ++pos;
		}
		return true;
	}
};

PlcProgram::PlcProgram() = default;
PlcProgram::~PlcProgram() = default;

QString PlcProgram::normalized(const QString &name)
{
	QString n = name.trimmed().toUpper();
	n.remove(QLatin1Char(' '));
	if (n.startsWith(QLatin1Char('%'))) n.remove(0, 1);
	return n;
}

bool PlcProgram::compile(const QString &source, QString *error)
{
	m_statements.clear();
	m_blocks.clear();
	m_initial.clear();
	m_initialised = false;
	QString src = source;
	// keep line numbers: a comment becomes as many line breaks as it spans
	static const QRegularExpression block_comment(QStringLiteral("\\(\\*[\\s\\S]*?\\*\\)"));
	auto it = block_comment.globalMatch(src);
	QVector<QPair<int, int>> spans;
	while (it.hasNext()) { const auto m = it.next(); spans << qMakePair(int(m.capturedStart()), int(m.capturedLength())); }
	for (int i = spans.size() - 1; i >= 0; --i) {
		const QString text = src.mid(spans.at(i).first, spans.at(i).second);
		src.replace(spans.at(i).first, spans.at(i).second, QString(text.count(QLatin1Char('\n')), QLatin1Char('\n')));
	}
	src.remove(QRegularExpression(QStringLiteral("//[^\\n]*")));
	QVector<Token> tokens;
	static const QRegularExpression tok(QStringLiteral(
		"(:=|<=|>=|<>|\\.\\.|(?:T|TIME)#(?:[0-9.]+(?:ms|s|m|h|d))+|%?[A-Za-z_][A-Za-z0-9_.]*"
		"|[0-9]+(?:\\.[0-9]+)?(?:[eE][-+]?[0-9]+)?|[();,:&=<>+\\-*/])"),
		QRegularExpression::CaseInsensitiveOption);
	const QStringList lines = src.split(QLatin1Char('\n'));
	for (int i = 0; i < lines.size(); ++i) {
		auto t = tok.globalMatch(lines.at(i));
		while (t.hasNext()) tokens << Token{t.next().captured(1), i + 1};
	}
	Parser p{this, tokens};
	if (p.declarations())
		p.block(m_statements, {});
	if (!p.error.isEmpty()) {
		m_statements.clear();
		m_blocks.clear();
		m_initial.clear();
		if (error) *error = p.error;
		return false;
	}
	return true;
}

void PlcProgram::cycle(double time, QHash<QString, double> &vars)
{
	if (!m_initialised) {
		for (auto it = m_initial.constBegin(); it != m_initial.constEnd(); ++it)
			vars.insert(it.key(), it.value());
		m_initialised = true;
	}
	m_vars = &vars;
	m_time = time;
	for (const auto &s : std::as_const(m_statements)) s();
	for (auto it = m_blocks.constBegin(); it != m_blocks.constEnd(); ++it) {
		const Block &b = *it.value();
		const bool counter = b.type == QLatin1String("CTU") || b.type == QLatin1String("CTD");
		const bool timer = b.type == QLatin1String("TON") || b.type == QLatin1String("TOF");
		const bool flipflop = b.type == QLatin1String("SR") || b.type == QLatin1String("RS");
		vars.insert(it.key() + (flipflop ? QStringLiteral(".Q1") : QStringLiteral(".Q")), b.q ? 1.0 : 0.0);
		if (counter) vars.insert(it.key() + QStringLiteral(".CV"), b.cv);
		if (timer) vars.insert(it.key() + QStringLiteral(".ET"), b.et);
	}
	m_vars = nullptr;
}
