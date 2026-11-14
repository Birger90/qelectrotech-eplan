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
#include <QStringList>

struct PlcProgram::Block {
	QString type;                 // TON, TOF, CTU
	std::function<bool()> in, reset;
	double pt = 0.0;
	int pv = 1;
	bool q = false, last_in = false;
	double since = -1.0;          // TON: IN high since; TOF: IN low since
	int cv = 0;
};

namespace {
struct Token { QString text; int line; };
}

struct PlcProgram::Parser {
	PlcProgram *prog;
	QVector<Token> tokens;
	int pos = 0;
	QString error;

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
		return re.match(t).hasMatch();
	}

	// ---------------------------------------------------------------- expressions
	std::function<bool()> expr() {
		auto left = xorExpr();
		while (left && peek() == QLatin1String("OR")) {
			++pos;
			auto right = xorExpr();
			if (!right) return {};
			left = [left, right]() { return left() || right(); };
		}
		return left;
	}
	std::function<bool()> xorExpr() {
		auto left = andExpr();
		while (left && peek() == QLatin1String("XOR")) {
			++pos;
			auto right = andExpr();
			if (!right) return {};
			left = [left, right]() { return left() != right(); };
		}
		return left;
	}
	std::function<bool()> andExpr() {
		auto left = notExpr();
		while (left && (peek() == QLatin1String("AND") || peek() == QLatin1String("&"))) {
			++pos;
			auto right = notExpr();
			if (!right) return {};
			left = [left, right]() { return left() && right(); };
		}
		return left;
	}
	std::function<bool()> notExpr() {
		if (peek() == QLatin1String("NOT")) {
			++pos;
			auto inner = notExpr();
			if (!inner) return {};
			return [inner]() { return !inner(); };
		}
		return primary();
	}
	std::function<bool()> primary() {
		if (atEnd()) { fail(QStringLiteral("Ausdruck erwartet")); return {}; }
		const QString t = tokens.at(pos).text;
		const QString up = t.toUpper();
		if (up == QLatin1String("(")) {
			++pos;
			auto e = expr();
			if (!e || !expect(QStringLiteral(")"))) return {};
			return e;
		}
		if (up == QLatin1String("TRUE")) { ++pos; return []() { return true; }; }
		if (up == QLatin1String("FALSE")) { ++pos; return []() { return false; }; }
		bool number = false;
		const double n = t.toDouble(&number);
		if (number) { ++pos; return [n]() { return n != 0.0; }; }
		if (!isName(t)) { fail(QStringLiteral("unerwartet: „%1“").arg(t)); return {}; }
		++pos;
		const QString name = PlcProgram::normalized(t);
		const int dot = static_cast<int>(name.lastIndexOf(QLatin1Char('.')));
		if (dot > 0) {
			const QString inst = name.left(dot), field = name.mid(dot + 1);
			if (prog->m_blocks.contains(inst)) {
				std::shared_ptr<Block> b = prog->m_blocks.value(inst);
				if (field == QLatin1String("Q")) return [b]() { return b->q; };
				if (field == QLatin1String("CV")) return [b]() { return b->cv > 0; };
				if (field == QLatin1String("ET")) return [b]() { return b->since >= 0; };
				fail(QStringLiteral("unbekannter Ausgang %1").arg(t));
				return {};
			}
		}
		PlcProgram *p = prog;
		return [p, name]() { return p->m_vars ? p->m_vars->value(name, false) : false; };
	}

	// ---------------------------------------------------------------- statements
	static double timeLiteral(const QString &t, bool *ok) {
		static const QRegularExpression re(QStringLiteral("^(?:T|TIME)#(\\d+(?:\\.\\d+)?)(MS|S|M|H)$"),
										   QRegularExpression::CaseInsensitiveOption);
		const auto m = re.match(t);
		*ok = m.hasMatch();
		if (!*ok) return 0.0;
		const double v = m.captured(1).toDouble();
		const QString u = m.captured(2).toUpper();
		if (u == QLatin1String("MS")) return v / 1000.0;
		if (u == QLatin1String("M")) return v * 60.0;
		if (u == QLatin1String("H")) return v * 3600.0;
		return v;
	}

	bool block(QVector<std::function<void()>> &out, const QStringList &until) {
		while (!atEnd() && !until.contains(peek()))
			if (!statement(out)) return false;
		return true;
	}

	bool statement(QVector<std::function<void()>> &out) {
		const QString up = peek();
		if (up == QLatin1String(";")) { ++pos; return true; }
		if (up == QLatin1String("IF")) {
			++pos;
			auto cond = expr();
			if (!cond || !expect(QStringLiteral("THEN"))) return false;
			QVector<std::function<void()>> then_part, else_part;
			if (!block(then_part, {QStringLiteral("ELSE"), QStringLiteral("END_IF")})) return false;
			if (peek() == QLatin1String("ELSE")) {
				++pos;
				if (!block(else_part, {QStringLiteral("END_IF")})) return false;
			}
			if (!expect(QStringLiteral("END_IF"))) return false;
			if (peek() == QLatin1String(";")) ++pos;
			out << [cond, then_part, else_part]() {
				for (const auto &s : (cond() ? then_part : else_part)) s();
			};
			return true;
		}
		if (atEnd() || !isName(tokens.at(pos).text)) return fail(QStringLiteral("Anweisung erwartet"));
		const QString name = PlcProgram::normalized(tokens.at(pos).text);
		++pos;
		if (peek() == QLatin1String(":=")) {
			++pos;
			auto e = expr();
			if (!e || !expect(QStringLiteral(";"))) return false;
			PlcProgram *p = prog;
			out << [p, name, e]() { if (p->m_vars) p->m_vars->insert(name, e()); };
			return true;
		}
		if (peek() == QLatin1String("(")) {
			if (!prog->m_blocks.contains(name))
				return fail(QStringLiteral("%1 ist nicht als TON, TOF oder CTU deklariert").arg(name));
			std::shared_ptr<Block> b = prog->m_blocks.value(name);
			++pos;
			while (!atEnd() && peek() != QLatin1String(")")) {
				const QString arg = peek();
				++pos;
				if (!expect(QStringLiteral(":="))) return false;
				if (arg == QLatin1String("PT")) {
					bool ok = false;
					b->pt = timeLiteral(atEnd() ? QString() : tokens.at(pos).text, &ok);
					if (!ok) return fail(QStringLiteral("Zeit wie T#1s erwartet"));
					++pos;
				} else if (arg == QLatin1String("PV")) {
					bool ok = false;
					b->pv = atEnd() ? 0 : tokens.at(pos).text.toInt(&ok);
					if (!ok) return fail(QStringLiteral("Zahl für PV erwartet"));
					++pos;
				} else if (arg == QLatin1String("IN") || arg == QLatin1String("CU")) {
					b->in = expr();
					if (!b->in) return false;
				} else if (arg == QLatin1String("R")) {
					b->reset = expr();
					if (!b->reset) return false;
				} else {
					return fail(QStringLiteral("unbekannter Eingang %1").arg(arg));
				}
				if (peek() == QLatin1String(",")) ++pos;
			}
			if (!expect(QStringLiteral(")")) || !expect(QStringLiteral(";"))) return false;
			PlcProgram *p = prog;
			out << [p, b]() {
				const bool in = b->in ? b->in() : false;
				const double t = p->m_time;
				if (b->type == QLatin1String("TON")) {
					if (in && b->since < 0) b->since = t;
					if (!in) b->since = -1;
					b->q = in && t - b->since >= b->pt - 1e-9;
				} else if (b->type == QLatin1String("TOF")) {
					if (in) { b->q = true; b->since = -1; }
					else {
						if (b->last_in) b->since = t;
						b->q = b->since >= 0 && t - b->since < b->pt;
					}
				} else {    // CTU
					if (b->reset && b->reset()) b->cv = 0;
					else if (in && !b->last_in) ++b->cv;
					b->q = b->cv >= b->pv;
				}
				b->last_in = in;
			};
			return true;
		}
		return fail(QStringLiteral("„:=“ oder „(“ erwartet nach %1").arg(name));
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
	QString src = source;
	src.remove(QRegularExpression(QStringLiteral("\\(\\*[\\s\\S]*?\\*\\)")));
	src.remove(QRegularExpression(QStringLiteral("//[^\\n]*")));
	QVector<Token> tokens;
	static const QRegularExpression tok(QStringLiteral(
		"(:=|(?:T|TIME)#[0-9.]+(?:ms|s|m|h)|%?[A-Za-z_][A-Za-z0-9_.]*|[0-9]+(?:\\.[0-9]+)?|[();,:&])"),
		QRegularExpression::CaseInsensitiveOption);
	const QStringList lines = src.split(QLatin1Char('\n'));
	for (int i = 0; i < lines.size(); ++i) {
		auto it = tok.globalMatch(lines.at(i));
		while (it.hasNext()) tokens << Token{it.next().captured(1), i + 1};
	}
	Parser p{this, tokens};
	while (p.peek() == QLatin1String("VAR")) {
		++p.pos;
		while (!p.atEnd() && p.peek() != QLatin1String("END_VAR")) {
			const QString name = normalized(p.tokens.at(p.pos).text);
			++p.pos;
			if (!p.expect(QStringLiteral(":"))) break;
			const QString type = p.peek();
			++p.pos;
			if (type != QLatin1String("TON") && type != QLatin1String("TOF") && type != QLatin1String("CTU")
				&& type != QLatin1String("BOOL")) {
				p.fail(QStringLiteral("Typ %1 wird nicht unterstützt (BOOL, TON, TOF, CTU)").arg(type));
				break;
			}
			if (type != QLatin1String("BOOL")) {
				auto b = std::make_shared<Block>();
				b->type = type;
				m_blocks.insert(name, b);
			}
			if (p.peek() == QLatin1String(";")) ++p.pos;
		}
		if (!p.error.isEmpty() || !p.expect(QStringLiteral("END_VAR"))) break;
		if (p.peek() == QLatin1String(";")) ++p.pos;
	}
	if (p.error.isEmpty())
		p.block(m_statements, {});
	if (!p.error.isEmpty()) {
		m_statements.clear();
		if (error) *error = p.error;
		return false;
	}
	return true;
}

void PlcProgram::cycle(double time, QHash<QString, bool> &vars)
{
	m_vars = &vars;
	m_time = time;
	for (const auto &s : std::as_const(m_statements)) s();
	for (auto it = m_blocks.constBegin(); it != m_blocks.constEnd(); ++it)
		vars.insert(it.key() + QStringLiteral(".Q"), it.value()->q);
	m_vars = nullptr;
}
