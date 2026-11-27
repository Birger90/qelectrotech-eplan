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
#include "simaticml.h"

#include <QDomDocument>
#include <QFile>
#include <QHash>
#include <QMap>
#include <QSet>

namespace {

QString local(const QDomElement &e)
{
	const QString n = e.localName().isEmpty() ? e.tagName() : e.localName();
	const int colon = int(n.indexOf(QLatin1Char(':')));
	return colon >= 0 ? n.mid(colon + 1) : n;
}

QList<QDomElement> children(const QDomElement &e, const QString &name = QString())
{
	QList<QDomElement> out;
	for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
		if (name.isEmpty() || local(c) == name) out << c;
	return out;
}

QDomElement child(const QDomElement &e, const QString &name)
{
	const QList<QDomElement> list = children(e, name);
	return list.isEmpty() ? QDomElement() : list.first();
}

	/// all descendants named @p name
void collect(const QDomElement &e, const QString &name, QList<QDomElement> &out)
{
	for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
		if (local(c) == name) out << c;
		collect(c, name, out);
	}
}

	/// whether @p e is one bracket from its first to its last character
bool enclosed(const QString &e)
{
	if (!e.startsWith(QLatin1Char('(')) || !e.endsWith(QLatin1Char(')'))) return false;
	int depth = 0;
	for (int i = 0; i < e.size(); ++i) {
		if (e.at(i) == QLatin1Char('(')) ++depth;
		else if (e.at(i) == QLatin1Char(')') && --depth == 0 && i < e.size() - 1) return false;
	}
	return true;
}

QString bracket(const QString &e)
{
	return (e.contains(QLatin1String(" OR ")) || e.contains(QLatin1String(" AND "))) && !enclosed(e)
		? QLatin1Char('(') + e + QLatin1Char(')') : e;
}

QString andOf(const QString &a, const QString &b)
{
	if (a == QLatin1String("FALSE") || b == QLatin1String("FALSE")) return QStringLiteral("FALSE");
	if (a == QLatin1String("TRUE") || a.isEmpty()) return b;
	if (b == QLatin1String("TRUE") || b.isEmpty()) return a;
	return bracket(a) + QStringLiteral(" AND ") + bracket(b);
}

QString notOf(const QString &a)
{
	if (a == QLatin1String("TRUE")) return QStringLiteral("FALSE");
	if (a == QLatin1String("FALSE")) return QStringLiteral("TRUE");
	return QStringLiteral("NOT ") + (a.contains(QLatin1Char(' ')) ? QLatin1Char('(') + a + QLatin1Char(')') : a);
}

	/// one LAD/FBD network
class Network
{
	public:
		Network(const QDomElement &flgnet, int number, QSet<QString> &instance_names,
				QMap<QString, QString> &declarations, QStringList &unsupported)
			: m_number(number), m_names(instance_names), m_decl(declarations), m_unsupported(unsupported)
		{
			const QDomElement parts = child(flgnet, QStringLiteral("Parts"));
			for (const QDomElement &p : children(parts)) {
				const QString uid = p.attribute(QStringLiteral("UId"));
				const QString kind = local(p);
				if (kind == QLatin1String("Access")) {
					m_access.insert(uid, accessText(p));
				} else if (kind == QLatin1String("Part")) {
					m_part.insert(uid, p.attribute(QStringLiteral("Name")));
					m_order << uid;
					for (const QDomElement &n : children(p, QStringLiteral("Negated")))
						m_negated[uid] << n.attribute(QStringLiteral("Name"));
					const QDomElement inst = child(p, QStringLiteral("Instance"));
					if (!inst.isNull()) {
						QStringList parts_of_name;
						for (const QDomElement &c : children(inst, QStringLiteral("Component")))
							parts_of_name << c.attribute(QStringLiteral("Name"));
						m_instance.insert(uid, parts_of_name.join(QLatin1Char('_')));
					}
				} else if (kind == QLatin1String("Call")) {
					const QDomElement info = child(p, QStringLiteral("CallInfo"));
					m_unsupported << QStringLiteral("network %1: call of %2 %3 not translated")
										 .arg(m_number).arg(info.attribute(QStringLiteral("BlockType")),
															info.attribute(QStringLiteral("Name")));
					m_calls << uid;
				}
			}
			const QDomElement wires = child(flgnet, QStringLiteral("Wires"));
			for (const QDomElement &w : children(wires, QStringLiteral("Wire"))) {
				const QList<QDomElement> ends = children(w);
				if (ends.size() < 2) continue;
				const QDomElement src = ends.first();
				Source s;
				const QString sk = local(src);
				if (sk == QLatin1String("Powerrail")) s.kind = Source::Power;
				else if (sk == QLatin1String("IdentCon")) {s.kind = Source::Ident; s.uid = src.attribute(QStringLiteral("UId"));}
				else if (sk == QLatin1String("NameCon")) {s.kind = Source::Pin; s.uid = src.attribute(QStringLiteral("UId")); s.pin = src.attribute(QStringLiteral("Name"));}
				else continue;
				for (int i = 1; i < ends.size(); ++i) {
					const QDomElement d = ends.at(i);
					const QString dk = local(d);
					if (dk == QLatin1String("NameCon"))
						m_source.insert(key(d.attribute(QStringLiteral("UId")), d.attribute(QStringLiteral("Name"))), s);
					else if (dk == QLatin1String("IdentCon") && s.kind == Source::Pin)
						m_targets[key(s.uid, s.pin)] << m_access.value(d.attribute(QStringLiteral("UId")));
				}
			}
		}

			/// the statements of this network, empty when nothing was translated
		QStringList statements()
		{
			QStringList pre, main;
			for (const QString &uid : std::as_const(m_order)) {
				const QString name = m_part.value(uid);
				if (name == QLatin1String("Coil") || name == QLatin1String("SCoil") || name == QLatin1String("RCoil")) {
					const QString target = m_access.value(m_source.value(key(uid, QStringLiteral("operand"))).uid);
					if (target.isEmpty()) continue;
					QString power = in(uid, QStringLiteral("in"));
					if (m_negated.value(uid).contains(QStringLiteral("operand")) && name == QLatin1String("Coil")) power = notOf(power);
					if (name == QLatin1String("Coil")) main << QStringLiteral("%1 := %2;").arg(target, power);
					else main << QStringLiteral("IF %1 THEN %2 := %3; END_IF;").arg(power, target,
								 name == QLatin1String("SCoil") ? QStringLiteral("TRUE") : QStringLiteral("FALSE"));
				} else if (name == QLatin1String("Sr") || name == QLatin1String("Rs")) {
					const QString op = m_access.value(m_source.value(key(uid, QStringLiteral("operand"))).uid);
					if (op.isEmpty()) continue;
					const bool sr = name == QLatin1String("Sr");
					const QString set = in(uid, sr ? QStringLiteral("s") : QStringLiteral("s1"));
					const QString reset = in(uid, sr ? QStringLiteral("r1") : QStringLiteral("r"));
					const QString s_line = QStringLiteral("IF %1 THEN %2 := TRUE; END_IF;").arg(set, op);
					const QString r_line = QStringLiteral("IF %1 THEN %2 := FALSE; END_IF;").arg(reset, op);
						// the input with the 1 is done last: it wins
					main << (sr ? s_line : r_line) << (sr ? r_line : s_line);
					assign(uid, QStringLiteral("q"), op, main);
				} else if (name == QLatin1String("PContact") || name == QLatin1String("NContact")
						   || name == QLatin1String("PBox") || name == QLatin1String("NBox")) {
					const QString e = edge(uid);
					const QString clk = name.endsWith(QLatin1String("Contact")) ? in(uid, QStringLiteral("operand"))
																				: in(uid, QStringLiteral("in"));
					pre << QStringLiteral("%1(CLK := %2);").arg(e, clk);
				} else if (name == QLatin1String("TON") || name == QLatin1String("TOF")) {
					const QString inst = instance(uid, name);
					pre << QStringLiteral("%1(IN := %2, PT := %3);").arg(inst, in(uid, QStringLiteral("IN")), in(uid, QStringLiteral("PT")));
					assign(uid, QStringLiteral("Q"), inst + QStringLiteral(".Q"), main);
					assign(uid, QStringLiteral("ET"), inst + QStringLiteral(".ET"), main);
				} else if (name == QLatin1String("CTU") || name == QLatin1String("CTD")) {
					const QString inst = instance(uid, name);
					if (name == QLatin1String("CTU"))
						pre << QStringLiteral("%1(CU := %2, R := %3, PV := %4);").arg(inst, in(uid, QStringLiteral("CU")),
								  in(uid, QStringLiteral("R")), in(uid, QStringLiteral("PV")));
					else
						pre << QStringLiteral("%1(CD := %2, LD := %3, PV := %4);").arg(inst, in(uid, QStringLiteral("CD")),
								  in(uid, QStringLiteral("LD")), in(uid, QStringLiteral("PV")));
					assign(uid, QStringLiteral("Q"), inst + QStringLiteral(".Q"), main);
					assign(uid, QStringLiteral("CV"), inst + QStringLiteral(".CV"), main);
				} else if (name == QLatin1String("Move")) {
					const QString en = in(uid, QStringLiteral("en"), QStringLiteral("TRUE"));
					const QString value = in(uid, QStringLiteral("in"));
					for (const QString &pin : {QStringLiteral("out1"), QStringLiteral("out")})
						for (const QString &t : m_targets.value(key(uid, pin)))
							main << guarded(en, QStringLiteral("%1 := %2;").arg(t, value));
				} else if (!arithmetic(name).isEmpty()) {
					const QString en = in(uid, QStringLiteral("en"), QStringLiteral("TRUE"));
					const QString value = QStringLiteral("(%1 %2 %3)").arg(in(uid, QStringLiteral("in1")), arithmetic(name),
																			  in(uid, QStringLiteral("in2")));
					for (const QString &t : m_targets.value(key(uid, QStringLiteral("out"))))
						main << guarded(en, QStringLiteral("%1 := %2;").arg(t, value));
				} else if (name == QLatin1String("Inc") || name == QLatin1String("Dec")) {
					const QString op = m_access.value(m_source.value(key(uid, QStringLiteral("operand"))).uid);
					if (op.isEmpty()) continue;
					main << guarded(in(uid, QStringLiteral("en"), QStringLiteral("TRUE")),
									QStringLiteral("%1 := %1 %2 1;").arg(op, name == QLatin1String("Inc") ? QStringLiteral("+") : QStringLiteral("-")));
				} else if (!known(name)) {
					m_unsupported << QStringLiteral("network %1: %2 not supported").arg(m_number).arg(name);
				}
			}
			return pre + main;
		}

	private:
		struct Source {enum Kind {None, Power, Ident, Pin} kind = None; QString uid, pin;};
		static QString key(const QString &uid, const QString &pin) {return uid + QLatin1Char('|') + pin;}

		static QString arithmetic(const QString &name)
		{
			if (name == QLatin1String("Add")) return QStringLiteral("+");
			if (name == QLatin1String("Sub")) return QStringLiteral("-");
			if (name == QLatin1String("Mul")) return QStringLiteral("*");
			if (name == QLatin1String("Div")) return QStringLiteral("/");
			return QString();
		}
		static QString comparison(const QString &name)
		{
			static const QHash<QString, QString> ops{{QStringLiteral("Eq"), QStringLiteral("=")},
				{QStringLiteral("Ne"), QStringLiteral("<>")}, {QStringLiteral("Lt"), QStringLiteral("<")},
				{QStringLiteral("Le"), QStringLiteral("<=")}, {QStringLiteral("Gt"), QStringLiteral(">")},
				{QStringLiteral("Ge"), QStringLiteral(">=")}};
			return ops.value(name);
		}
		static bool known(const QString &name)
		{
			static const QSet<QString> names{QStringLiteral("Contact"), QStringLiteral("O"), QStringLiteral("A"),
											 QStringLiteral("Eq"), QStringLiteral("Ne"), QStringLiteral("Lt"),
											 QStringLiteral("Le"), QStringLiteral("Gt"), QStringLiteral("Ge")};
			return names.contains(name);
		}

		QString accessText(const QDomElement &access)
		{
			const QString scope = access.attribute(QStringLiteral("Scope"));
			const QDomElement symbol = child(access, QStringLiteral("Symbol"));
			if (!symbol.isNull()) {
				QStringList parts;
				for (const QDomElement &c : children(symbol, QStringLiteral("Component"))) {
					parts << c.attribute(QStringLiteral("Name"));
					if (!children(c, QStringLiteral("Access")).isEmpty())
						m_unsupported << QStringLiteral("network %1: indexed access %2 read without its index")
											 .arg(m_number).arg(c.attribute(QStringLiteral("Name")));
				}
				return parts.join(QLatin1Char('.'));
			}
			const QDomElement constant = child(access, QStringLiteral("Constant"));
			if (!constant.isNull()) {
				const QString type = child(constant, QStringLiteral("ConstantType")).text().trimmed();
				QString value = child(constant, QStringLiteral("ConstantValue")).text().trimmed();
				if (type.compare(QLatin1String("Bool"), Qt::CaseInsensitive) == 0
					|| value.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
					|| value.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0)
					value = value.toUpper();
				return value;
			}
			const QDomElement address = child(access, QStringLiteral("Address"));
			if (!address.isNull()) {
				const QString area = address.attribute(QStringLiteral("Area"));
				const QString letter = area == QLatin1String("Input") ? QStringLiteral("I")
									 : area == QLatin1String("Output") ? QStringLiteral("Q")
									 : area == QLatin1String("Memory") ? QStringLiteral("M") : area.left(1);
				const int bits = address.attribute(QStringLiteral("BitOffset")).toInt();
				const QString type = address.attribute(QStringLiteral("Type")).toLower();
				if (type == QLatin1String("bool") || type.isEmpty())
					return QStringLiteral("%%1%2.%3").arg(letter).arg(bits / 8).arg(bits % 8);
				const QString size = type == QLatin1String("byte") ? QStringLiteral("B")
								   : (type == QLatin1String("dword") || type == QLatin1String("dint") || type == QLatin1String("real"))
										 ? QStringLiteral("D") : QStringLiteral("W");
				return QStringLiteral("%%1%2%3").arg(letter, size).arg(bits / 8);
			}
			m_unsupported << QStringLiteral("network %1: access of scope %2 not translated").arg(m_number).arg(scope);
			return QStringLiteral("0");
		}

			/// the value arriving at input @p pin of part @p uid
		QString in(const QString &uid, const QString &pin, const QString &absent = QStringLiteral("FALSE"))
		{
			const auto it = m_source.constFind(key(uid, pin));
			QString v;
			if (it == m_source.constEnd()) v = absent;
			else if (it->kind == Source::Power) v = QStringLiteral("TRUE");
			else if (it->kind == Source::Ident) v = m_access.value(it->uid);
			else v = out(it->uid, it->pin);
			if (m_negated.value(uid).contains(pin) && m_part.value(uid) != QLatin1String("Coil")) v = notOf(v);
			return v;
		}

			/// the value leaving output @p pin of part @p uid
		QString out(const QString &uid, const QString &pin)
		{
			if (++m_depth > 400) {--m_depth; return QStringLiteral("FALSE");}
			QString v;
			const QString name = m_part.value(uid);
			if (m_calls.contains(uid)) {
				v = QStringLiteral("FALSE");
			} else if (name == QLatin1String("Contact")) {
				v = andOf(in(uid, QStringLiteral("in")), in(uid, QStringLiteral("operand")));
			} else if (!comparison(name).isEmpty()) {
				v = andOf(in(uid, QStringLiteral("pre"), QStringLiteral("TRUE")),
						  QStringLiteral("(%1 %2 %3)").arg(in(uid, QStringLiteral("in1")), comparison(name), in(uid, QStringLiteral("in2"))));
			} else if (name == QLatin1String("O") || name == QLatin1String("A")) {
				QMap<int, QString> inputs;
				const QString head = uid + QStringLiteral("|in");
				for (auto it = m_source.constBegin(); it != m_source.constEnd(); ++it)
					if (it.key().startsWith(head)) inputs.insert(it.key().mid(head.size()).toInt(), it.key().mid(uid.size() + 1));
				QStringList terms;
				for (const QString &p : std::as_const(inputs)) terms << bracket(in(uid, p));
				if (name == QLatin1String("A")) {
					v = QStringLiteral("TRUE");
					for (const QString &t : std::as_const(terms)) v = andOf(v, t);
				} else {
					terms.removeAll(QStringLiteral("FALSE"));
					v = terms.contains(QStringLiteral("TRUE")) ? QStringLiteral("TRUE")
						: terms.isEmpty() ? QStringLiteral("FALSE")
						: terms.size() == 1 ? terms.first() : QLatin1Char('(') + terms.join(QStringLiteral(" OR ")) + QLatin1Char(')');
				}
			} else if (name == QLatin1String("Coil") || name == QLatin1String("SCoil") || name == QLatin1String("RCoil")) {
				v = in(uid, QStringLiteral("in"));
			} else if (name == QLatin1String("PContact") || name == QLatin1String("NContact")) {
				v = andOf(in(uid, QStringLiteral("pre"), QStringLiteral("TRUE")), edge(uid) + QStringLiteral(".Q"));
			} else if (name == QLatin1String("PBox") || name == QLatin1String("NBox")) {
				v = edge(uid) + QStringLiteral(".Q");
			} else if (name == QLatin1String("Sr") || name == QLatin1String("Rs")) {
				v = m_access.value(m_source.value(key(uid, QStringLiteral("operand"))).uid);
			} else if (name == QLatin1String("TON") || name == QLatin1String("TOF") || name == QLatin1String("CTU") || name == QLatin1String("CTD")) {
				v = instance(uid, name) + QLatin1Char('.') + (pin == QLatin1String("ET") || pin == QLatin1String("CV") ? pin : QStringLiteral("Q"));
			} else if (pin == QLatin1String("eno")) {
				v = in(uid, QStringLiteral("en"), QStringLiteral("TRUE"));
			} else if (name == QLatin1String("Move")) {
				v = in(uid, QStringLiteral("in"));
			} else if (!arithmetic(name).isEmpty()) {
				v = QStringLiteral("(%1 %2 %3)").arg(in(uid, QStringLiteral("in1")), arithmetic(name), in(uid, QStringLiteral("in2")));
			} else {
				v = QStringLiteral("FALSE");
			}
			--m_depth;
			return v;
		}

		void assign(const QString &uid, const QString &pin, const QString &value, QStringList &into)
		{
			for (const QString &t : m_targets.value(key(uid, pin)))
				if (!t.isEmpty() && t != value) into << QStringLiteral("%1 := %2;").arg(t, value);
		}

		static QString guarded(const QString &en, const QString &statement)
		{
			return en == QLatin1String("TRUE") ? statement : QStringLiteral("IF %1 THEN %2 END_IF;").arg(en, statement);
		}

		QString unique(QString name)
		{
			if (name.isEmpty()) name = QStringLiteral("I");
			QString n = name;
			for (int i = 2; m_names.contains(n.toUpper()) && !m_decl.contains(n); ++i) n = name + QStringLiteral("_%1").arg(i);
			m_names << n.toUpper();
			return n;
		}

		QString instance(const QString &uid, const QString &type)
		{
			if (m_given.contains(uid)) return m_given.value(uid);
			QString n = m_instance.value(uid);
			if (n.isEmpty()) n = QStringLiteral("%1_N%2_%3").arg(type).arg(m_number).arg(uid);
			if (!m_decl.contains(n)) n = unique(n);
			m_decl.insert(n, type);
			m_given.insert(uid, n);
			return n;
		}

		QString edge(const QString &uid)
		{
			const QString name = m_part.value(uid);
			const bool rising = name.startsWith(QLatin1Char('P'));
			return instance(uid, rising ? QStringLiteral("R_TRIG") : QStringLiteral("F_TRIG"));
		}

		int m_number = 0;
		int m_depth = 0;
		QSet<QString> &m_names;
		QMap<QString, QString> &m_decl;
		QStringList &m_unsupported;
		QHash<QString, QString> m_access, m_part, m_instance, m_given;
		QHash<QString, QSet<QString>> m_negated;
		QHash<QString, Source> m_source;
		QHash<QString, QStringList> m_targets;
		QStringList m_order;
		QSet<QString> m_calls;
};

} // namespace

SimaticMl::Result SimaticMl::toStructuredText(const QByteArray &xml)
{
	Result r;
	QDomDocument doc;
	if (!doc.setContent(xml, QDomDocument::ParseOption::UseNamespaceProcessing)) {
		r.error = QStringLiteral("not readable as XML");
		return r;
	}
	QDomElement block;
	for (QDomElement c = doc.documentElement().firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
		if (local(c).startsWith(QLatin1String("SW.Blocks."))) {block = c; break;}
	if (block.isNull()) {r.error = QStringLiteral("no SimaticML block (SW.Blocks.FC/FB/OB) in the file"); return r;}
	r.block = child(child(block, QStringLiteral("AttributeList")), QStringLiteral("Name")).text().trimmed();

	QList<QDomElement> units;
	collect(block, QStringLiteral("SW.Blocks.CompileUnit"), units);
	QSet<QString> names;
	QMap<QString, QString> declarations;
	QStringList body;
	for (const QDomElement &unit : std::as_const(units)) {
		++r.networks;
		const QDomElement attributes = child(unit, QStringLiteral("AttributeList"));
		const QString language = child(attributes, QStringLiteral("ProgrammingLanguage")).text().trimmed();
		const QDomElement source = child(attributes, QStringLiteral("NetworkSource"));
		const QDomElement flgnet = child(source, QStringLiteral("FlgNet"));
		body << QStringLiteral("(* network %1%2 *)").arg(r.networks)
					.arg(language.isEmpty() ? QString() : QStringLiteral(", ") + language);
		if (flgnet.isNull()) {
			if (!source.isNull() && !children(source).isEmpty())
				r.unsupported << QStringLiteral("network %1: %2 is not translated").arg(r.networks)
									 .arg(language.isEmpty() ? local(children(source).first()) : language);
			continue;
		}
		Network net(flgnet, r.networks, names, declarations, r.unsupported);
		const QStringList statements = net.statements();
		if (!statements.isEmpty()) ++r.translated;
		body << statements;
	}
	QString st = QStringLiteral("(* %1: translated from TIA Portal LAD/FBD (SimaticML) *)\n").arg(r.block);
	if (!declarations.isEmpty()) {
		st += QStringLiteral("VAR\n");
		for (auto it = declarations.constBegin(); it != declarations.constEnd(); ++it)
			st += QStringLiteral("\t%1 : %2;\n").arg(it.key(), it.value());
		st += QStringLiteral("END_VAR\n");
	}
	st += body.join(QLatin1Char('\n')) + QLatin1Char('\n');
	if (!r.unsupported.isEmpty()) {
		st += QStringLiteral("(* not translated:\n");
		for (const QString &u : std::as_const(r.unsupported)) st += QStringLiteral("   ") + u + QLatin1Char('\n');
		st += QStringLiteral("*)\n");
	}
	r.st = st;
	return r;
}

SimaticMl::Result SimaticMl::fileToStructuredText(const QString &path)
{
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) {
		Result r;
		r.error = f.errorString();
		return r;
	}
	return toStructuredText(f.readAll());
}
