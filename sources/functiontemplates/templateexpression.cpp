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
#include "templateexpression.h"

#include <QCoreApplication>
#include <QList>
#include <QRegularExpression>

#include <cmath>

namespace TemplateExpression
{

namespace
{
	struct Token
	{
		enum Kind { End, Number, Text, Name, Op, Open, Close, Separator };
		Kind kind = End;
		QString text;
		int pos = 0;
	};

	QString tr(const char *text)
	{
		return QCoreApplication::translate("TemplateExpression", text);
	}

	QList<Token> tokens(const QString &s, QString *error)
	{
		QList<Token> list;
		int i = 0;
		const int n = s.size();
		while (i < n) {
			const QChar c = s.at(i);
			if (c.isSpace()) { ++i; continue; }
			Token t;
			t.pos = i;
			if (c.isDigit() || (c == QLatin1Char('.') && i + 1 < n && s.at(i + 1).isDigit())) {
				int j = i;
				bool decimal = false;
				while (j < n) {
					const QChar d = s.at(j);
					if (d.isDigit()) { ++j; continue; }
						//5.5 and 5,5: a comma only directly between digits,
						//so that max(a, 2) keeps its separator
					if (!decimal && (d == QLatin1Char('.') || d == QLatin1Char(','))
							&& j + 1 < n && s.at(j + 1).isDigit() && j > i) {
						decimal = true;
						++j;
						continue;
					}
					break;
				}
				t.kind = Token::Number;
				t.text = s.mid(i, j - i).replace(QLatin1Char(','), QLatin1Char('.'));
				i = j;
			} else if (c == QLatin1Char('\'') || c == QLatin1Char('"')) {
				int j = i + 1;
				QString text;
				while (j < n && s.at(j) != c) text += s.at(j++);
				if (j >= n) {
					if (error) *error = tr("Text opened at %1 is not closed.").arg(i + 1);
					return {};
				}
				t.kind = Token::Text;
				t.text = text;
				i = j + 1;
			} else if (c.isLetter() || c == QLatin1Char('_')) {
				int j = i;
				while (j < n && (s.at(j).isLetterOrNumber() || s.at(j) == QLatin1Char('_'))) ++j;
				t.kind = Token::Name;
				t.text = s.mid(i, j - i);
				i = j;
			} else if (c == QLatin1Char('(')) {
				t.kind = Token::Open; t.text = c; ++i;
			} else if (c == QLatin1Char(')')) {
				t.kind = Token::Close; t.text = c; ++i;
			} else if (c == QLatin1Char(';') || c == QLatin1Char(',')) {
				t.kind = Token::Separator; t.text = c; ++i;
			} else {
				static const QStringList two{QStringLiteral("=="), QStringLiteral("!="), QStringLiteral("<>"),
											 QStringLiteral("<="), QStringLiteral(">="), QStringLiteral("&&"),
											 QStringLiteral("||")};
				const QString pair = s.mid(i, 2);
				if (two.contains(pair)) {
					t.text = pair;
					i += 2;
				} else if (QStringLiteral("+-*/^=<>!≤≥").contains(c)) {
					t.text = c;
					++i;
				} else {
					if (error) *error = tr("Unknown sign '%1' at %2.").arg(c).arg(i + 1);
					return {};
				}
				t.kind = Token::Op;
			}
			list << t;
		}
		Token end;
		end.pos = n;
		list << end;
		return list;
	}

	class Parser
	{
			//The messages under the namespace's name, as tokens() writes them
		Q_DECLARE_TR_FUNCTIONS(TemplateExpression)

		public:
			Parser(const QList<Token> &tokens, const Context &context) :
				m_tokens(tokens), m_context(context) {}

			QString run(QString *error)
			{
				const QString value = orExpr();
				if (m_error.isEmpty() && peek().kind != Token::End)
					fail(tr("Unexpected '%1' at %2.").arg(peek().text).arg(peek().pos + 1));
				if (error) *error = m_error;
				return m_error.isEmpty() ? value : QString();
			}

		private:
			const Token &peek() const { return m_tokens.at(qMin(m_at, int(m_tokens.size()) - 1)); }
			Token take() { const Token t = peek(); if (m_at < m_tokens.size() - 1) ++m_at; return t; }
			void fail(const QString &text) { if (m_error.isEmpty() && !m_quiet) m_error = text; }

			bool isWord(const char *word) const
			{
				return peek().kind == Token::Name
						&& peek().text.compare(QLatin1String(word), Qt::CaseInsensitive) == 0;
			}
			bool isOp(const char *op) const
			{
				return peek().kind == Token::Op && peek().text == QString::fromUtf8(op);
			}

			static QString boolean(bool b) { return b ? QStringLiteral("1") : QStringLiteral("0"); }

			QString orExpr()
			{
				QString left = andExpr();
				while (m_error.isEmpty() && (isWord("or") || isWord("oder") || isOp("||"))) {
					take();
					const QString right = andExpr();
					left = boolean(isTrue(left) || isTrue(right));
				}
				return left;
			}

			QString andExpr()
			{
				QString left = notExpr();
				while (m_error.isEmpty() && (isWord("and") || isWord("und") || isOp("&&"))) {
					take();
					const QString right = notExpr();
					left = boolean(isTrue(left) && isTrue(right));
				}
				return left;
			}

			QString notExpr()
			{
				if (isWord("not") || isWord("nicht") || isOp("!")) {
					take();
					return boolean(!isTrue(notExpr()));
				}
				return comparison();
			}

			QString comparison()
			{
				const QString left = sum();
				static const QStringList ops{QStringLiteral("="), QStringLiteral("=="), QStringLiteral("!="),
											 QStringLiteral("<>"), QStringLiteral("<"), QStringLiteral("<="),
											 QStringLiteral(">"), QStringLiteral(">="), QStringLiteral("≤"),
											 QStringLiteral("≥")};
				if (peek().kind != Token::Op || !ops.contains(peek().text)) return left;
				const QString op = take().text;
				const QString right = sum();
				double a = 0, b = 0;
				int order;
				if (toNumber(left, &a) && toNumber(right, &b))
					order = std::fabs(a - b) < 1e-9 ? 0 : (a < b ? -1 : 1);
				else
					order = QString::compare(left.trimmed(), right.trimmed(), Qt::CaseInsensitive);
				if (op == QLatin1String("=") || op == QLatin1String("==")) return boolean(order == 0);
				if (op == QLatin1String("!=") || op == QLatin1String("<>")) return boolean(order != 0);
				if (op == QLatin1String("<")) return boolean(order < 0);
				if (op == QLatin1String(">")) return boolean(order > 0);
				if (op == QLatin1String("<=") || op == QStringLiteral("≤")) return boolean(order <= 0);
				return boolean(order >= 0);
			}

			QString sum()
			{
				QString left = product();
				while (m_error.isEmpty() && (isOp("+") || isOp("-"))) {
					const QString op = take().text;
					const QString right = product();
					double a = 0, b = 0;
					const bool numbers = toNumber(left, &a) && toNumber(right, &b);
					if (op == QLatin1String("+")) {
							//With a text, + joins
						left = numbers ? fromNumber(a + b) : left + right;
					} else {
						if (!numbers) {
							fail(tr("'%1 - %2' is no subtraction of numbers.").arg(left, right));
							return QString();
						}
						left = fromNumber(a - b);
					}
				}
				return left;
			}

			QString product()
			{
				QString left = power();
				while (m_error.isEmpty() && (isOp("*") || isOp("/"))) {
					const QString op = take().text;
					const QString right = power();
					double a = 0, b = 0;
					if (!toNumber(left, &a) || !toNumber(right, &b)) {
						fail(tr("'%1 %2 %3' needs two numbers.").arg(left, op, right));
						return QString();
					}
					if (op == QLatin1String("/")) {
						if (b == 0) { fail(tr("Division by zero.")); return QString(); }
						left = fromNumber(a / b);
					} else {
						left = fromNumber(a * b);
					}
				}
				return left;
			}

			QString power()
			{
				const QString left = unary();
				if (!isOp("^")) return left;
				take();
				const QString right = unary();
				double a = 0, b = 0;
				if (!toNumber(left, &a) || !toNumber(right, &b)) {
					fail(tr("'%1 ^ %2' needs two numbers.").arg(left, right));
					return QString();
				}
				return fromNumber(std::pow(a, b));
			}

			QString unary()
			{
				if (isOp("-")) {
					take();
					const QString value = unary();
					double a = 0;
					if (!toNumber(value, &a)) { fail(tr("'-%1' is no number.").arg(value)); return QString(); }
					return fromNumber(-a);
				}
				if (isOp("+")) { take(); return unary(); }
				return primary();
			}

			QString primary()
			{
				const Token t = take();
				switch (t.kind) {
					case Token::Number: return t.text;
					case Token::Text: return t.text;
					case Token::Open: {
						const QString value = orExpr();
						if (peek().kind != Token::Close) {
							fail(tr("')' missing at %1.").arg(peek().pos + 1));
							return QString();
						}
						take();
						return value;
					}
					case Token::Name: {
						if (peek().kind == Token::Open) return call(t);
						if (t.text.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
								|| t.text.compare(QLatin1String("wahr"), Qt::CaseInsensitive) == 0)
							return QStringLiteral("1");
						if (t.text.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0
								|| t.text.compare(QLatin1String("falsch"), Qt::CaseInsensitive) == 0)
							return QStringLiteral("0");
						return m_context.values ? m_context.values->value(t.text) : QString();
					}
					case Token::End:
						fail(tr("The formula ends too early."));
						return QString();
					default:
						fail(tr("Unexpected '%1' at %2.").arg(t.text).arg(t.pos + 1));
						return QString();
				}
			}

			QString call(const Token &name)
			{
				take(); // (
				const QString f = name.text.toLower();
					//has(NAME) asks about the name itself, not its value
				if (f == QLatin1String("has")) {
					const Token arg = take();
					if (arg.kind != Token::Name || peek().kind != Token::Close) {
						fail(tr("has() takes one name."));
						return QString();
					}
					take();
					return boolean(m_context.values && !m_context.values->value(arg.text).trimmed().isEmpty());
				}
				QStringList args;
				const bool branch = f == QLatin1String("if") || f == QLatin1String("wenn");
				if (peek().kind != Token::Close) {
					for (;;) {
							//if(): the branch not taken is read, not worked out:
							//no lookup, no note, no mistake of its own
						const bool skip = branch && args.size() >= 1
								&& (args.size() == 1) != isTrue(args.at(0));
						if (skip) ++m_quiet;
						args << orExpr();
						if (skip) --m_quiet;
						if (!m_error.isEmpty()) return QString();
						if (peek().kind == Token::Separator) { take(); continue; }
						break;
					}
				}
				if (peek().kind != Token::Close) {
					fail(tr("')' missing after the arguments of %1 at %2.").arg(name.text).arg(peek().pos + 1));
					return QString();
				}
				take();
				return function(f, name.text, args);
			}

			bool numbers(const QString &f, const QStringList &args, QList<double> *out)
			{
				for (const QString &a : args) {
					double v = 0;
					if (!toNumber(a, &v)) {
						fail(tr("%1() needs numbers, not '%2'.").arg(f, a));
						return false;
					}
					*out << v;
				}
				return true;
			}

			bool count(const QString &f, const QStringList &args, int least, int most)
			{
				if (args.size() < least || args.size() > most) {
					fail(least == most ? tr("%1() takes %2 arguments.").arg(f).arg(least)
									   : tr("%1() takes %2 to %3 arguments.").arg(f).arg(least).arg(most));
					return false;
				}
				return true;
			}

			QString function(const QString &f, const QString &shown, const QStringList &args)
			{
				QList<double> v;
				if (f == QLatin1String("if") || f == QLatin1String("wenn")) {
					if (!count(shown, args, 2, 3)) return QString();
					return isTrue(args.at(0)) ? args.at(1) : args.value(2);
				}
				if (f == QLatin1String("lookup")) {
					if (!count(shown, args, 3, 4)) return QString();
					if (m_quiet) return QString();
					if (!m_context.lookup) { fail(tr("There is no table to look up.")); return QString(); }
					QString note;
					const QString value = m_context.lookup(args.at(0), args.at(1), args.at(2), args.value(3), &note);
					if (!note.isEmpty() && m_context.notes) m_context.notes->append(note);
					return value;
				}
				if (f == QLatin1String("min") || f == QLatin1String("max")) {
					if (!count(shown, args, 1, 64) || !numbers(shown, args, &v)) return QString();
					double r = v.first();
					for (double x : std::as_const(v)) r = f == QLatin1String("min") ? qMin(r, x) : qMax(r, x);
					return fromNumber(r);
				}
				if (f == QLatin1String("round")) {
					if (!count(shown, args, 1, 2) || !numbers(shown, args, &v)) return QString();
					const double k = std::pow(10.0, v.value(1, 0));
					return fromNumber(std::round(v.at(0) * k) / k);
				}
				if (f == QLatin1String("ceil") || f == QLatin1String("floor") || f == QLatin1String("abs")
						|| f == QLatin1String("sqrt")) {
					if (!count(shown, args, 1, 1) || !numbers(shown, args, &v)) return QString();
					const double x = v.at(0);
					if (f == QLatin1String("sqrt") && x < 0) { fail(tr("sqrt() of a negative number.")); return QString(); }
					return fromNumber(f == QLatin1String("ceil") ? std::ceil(x)
									  : f == QLatin1String("floor") ? std::floor(x)
									  : f == QLatin1String("abs") ? std::fabs(x) : std::sqrt(x));
				}
				if (f == QLatin1String("text")) {
					if (!count(shown, args, 1, 2)) return QString();
					double x = 0, digits = 0;
					if (!toNumber(args.at(0), &x)) return args.at(0);
					if (args.size() > 1 && !toNumber(args.at(1), &digits)) {
						fail(tr("text() needs a number of digits."));
						return QString();
					}
						//The German way, as the folio shows numbers: 5,5
					return QString::number(x, 'f', int(digits)).replace(QLatin1Char('.'), QLatin1Char(','));
				}
				fail(tr("Unknown function %1().").arg(shown));
				return QString();
			}

			QList<Token> m_tokens;
			int m_at = 0;
			int m_quiet = 0;	///< inside a branch of if() not taken
			const Context &m_context;
			QString m_error;
	};
}

QString evaluate(const QString &expression, const Context &context, QString *error)
{
	QString e;
	const QList<Token> list = tokens(expression, &e);
	if (!e.isEmpty()) {
		if (error) *error = e;
		return QString();
	}
	Parser parser(list, context);
	return parser.run(error);
}

bool isTrue(const QString &value)
{
	const QString v = value.trimmed().toLower();
	if (v.isEmpty() || v == QLatin1String("false") || v == QLatin1String("falsch")
			|| v == QLatin1String("nein") || v == QLatin1String("no"))
		return false;
	double d = 0;
	if (toNumber(v, &d)) return d != 0;
	return true;
}

bool toNumber(const QString &text, double *value)
{
		//"5,5", "11.5 A", "7,5kW": the number, a unit of letters after it
	static const QRegularExpression pattern(
				QStringLiteral("^\\s*([-+]?(?:\\d+(?:[.,]\\d+)?|[.,]\\d+)(?:[eE][-+]?\\d+)?)\\s*([A-Za-z%\u00b5\u00b0\u03a9]{0,4})\\s*$"));
	const QRegularExpressionMatch match = pattern.match(text);
	if (!match.hasMatch()) return false;
	QString t = match.captured(1);
	t.replace(QLatin1Char(','), QLatin1Char('.'));
	bool ok = false;
	const double d = t.toDouble(&ok);
	if (ok && value) *value = d;
	return ok;
}

QString fromNumber(double value)
{
	if (std::fabs(value - std::round(value)) < 1e-9 && std::fabs(value) < 1e15)
		return QString::number(qlonglong(std::llround(value)));
	QString s = QString::number(value, 'f', 6);
	while (s.endsWith(QLatin1Char('0'))) s.chop(1);
	if (s.endsWith(QLatin1Char('.'))) s.chop(1);
	return s;
}

} // namespace TemplateExpression
