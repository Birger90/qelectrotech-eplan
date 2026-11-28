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
#include "productrules.h"

#include "functiontemplate.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

#include <algorithm>

namespace
{
	QMap<QString, QString> valuesOf(const QJsonValue &value)
	{
		QMap<QString, QString> map;
		const QJsonObject object = value.toObject();
		for (auto it = object.constBegin() ; it != object.constEnd() ; ++it)
			map.insert(it.key(), it.value().isString() ? it.value().toString()
													   : it.value().toVariant().toString());
		return map;
	}

		/// What a template knows by name: parameters, table columns,
		/// formulas, the placeholders of its drawing
	QStringList known(const FunctionTemplate &tpl)
	{
		QStringList list;
		for (const auto &p : tpl.parameters()) list << p.name;
		for (const auto &t : tpl.tables()) list << t.columns;
		for (const auto &f : tpl.formulas()) list << f.name;
		if (tpl.hasDrawing()) list << FunctionTemplate::placeholdersIn(tpl.document().documentElement());
		return list;
	}

	constexpr int MaxNumbers = 5000;
}

ProductRules ProductRules::fromFile(const QString &path, QString *error)
{
	ProductRules rules;
	rules.m_path = QFileInfo(path).absoluteFilePath();
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		if (error) *error = tr("The rules %1 cannot be read.").arg(QDir::toNativeSeparators(path));
		return rules;
	}
	QJsonParseError parse;
	const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parse);
	if (parse.error != QJsonParseError::NoError || !document.isObject()) {
		if (error) *error = tr("The rules %1 are no valid JSON: %2").arg(QDir::toNativeSeparators(path), parse.errorString());
		return rules;
	}
	const QJsonObject root = document.object();
	rules.m_family = root.value(QStringLiteral("family")).toString().trimmed();
	rules.m_name = root.value(QStringLiteral("name")).toString();
	const QString separator = root.value(QStringLiteral("separator")).toString();
	if (!separator.isEmpty()) rules.m_separator = separator;
	for (const QJsonValue &s : root.value(QStringLiteral("segments")).toArray()) {
		const QJsonObject o = s.toObject();
		Segment segment;
		segment.name = o.value(QStringLiteral("name")).toString().trimmed();
		segment.label = o.value(QStringLiteral("label")).toString(segment.name);
		segment.optional = o.value(QStringLiteral("optional")).toBool();
		const QJsonObject codes = o.value(QStringLiteral("codes")).toObject();
		for (auto it = codes.constBegin() ; it != codes.constEnd() ; ++it) {
			Code code;
			code.code = it.key().trimmed();
			code.label = it.value().toObject().value(QStringLiteral("label")).toString(code.code);
			code.values = valuesOf(it.value().toObject().value(QStringLiteral("values")));
			segment.codes << code;
		}
		if (!segment.name.isEmpty()) rules.m_segments << segment;
	}
	for (const QJsonValue &r : root.value(QStringLiteral("rules")).toArray()) {
		const QJsonObject o = r.toObject();
		Rule rule;
		rule.when = valuesOf(o.value(QStringLiteral("when")));
		rule.templ = o.value(QStringLiteral("template")).toString().trimmed();
		rule.variant = o.value(QStringLiteral("variant")).toString().trimmed();
		rule.values = valuesOf(o.value(QStringLiteral("values")));
		rules.m_rules << rule;
	}
	rules.m_valid = !rules.m_family.isEmpty() && !rules.m_segments.isEmpty();
	if (!rules.m_valid && error)
		*error = tr("The rules %1 name no family or no segment.").arg(QDir::toNativeSeparators(path));
	return rules;
}

QList<ProductRules> ProductRules::available()
{
	QList<ProductRules> list;
	QSet<QString> families;
	for (const QString &dir : FunctionTemplate::directories()) {
		QDirIterator it(dir, {QStringLiteral("*.product.json")}, QDir::Files, QDirIterator::Subdirectories);
		QStringList files;
		while (it.hasNext()) files << it.next();
		std::sort(files.begin(), files.end());
		for (const QString &file : std::as_const(files)) {
			const ProductRules rules = fromFile(file);
				//The user's own folder comes first: its family wins
			if (rules.isValid() && !families.contains(rules.family().toUpper())) {
				families.insert(rules.family().toUpper());
				list << rules;
			}
		}
	}
	return list;
}

ProductRules ProductRules::forNumber(const QString &number)
{
	for (const ProductRules &rules : available()) {
		const QString head = number.trimmed().section(rules.m_separator, 0, 0);
		if (head.trimmed().compare(rules.m_family, Qt::CaseInsensitive) == 0) return rules;
	}
	return ProductRules();
}

ProductRules::Decoded ProductRules::decodeNumber(const QString &number)
{
	const ProductRules rules = forNumber(number);
	if (!rules.isValid()) {
		Decoded d;
		d.number = number.trimmed();
		d.error = tr("No product family has rules for %1.").arg(d.number);
		return d;
	}
	return rules.decode(number);
}

ProductRules::Decoded ProductRules::decode(const QString &number) const
{
	Decoded d;
	d.number = number.trimmed();
	const QStringList parts = d.number.split(m_separator);
	if (parts.isEmpty() || parts.first().trimmed().compare(m_family, Qt::CaseInsensitive) != 0) {
		d.error = tr("%1 is no number of the family %2.").arg(d.number, m_family);
		return d;
	}
	const QStringList rest = parts.mid(1);
	if (rest.size() > m_segments.size()) {
		d.error = tr("%1 has more segments than the family %2 knows (%3).")
				.arg(d.number, m_family).arg(m_segments.size());
		return d;
	}
	for (int i = 0 ; i < m_segments.size() ; ++i) {
		const Segment &segment = m_segments.at(i);
		const QString code = i < rest.size() ? rest.at(i).trimmed() : QString();
		if (i >= rest.size() && !segment.optional) {
			d.error = tr("%1 lacks the segment %2.").arg(d.number, segment.label);
			return d;
		}
		const Code *found = nullptr;
		for (const Code &c : segment.codes)
			if (c.code.compare(code, Qt::CaseInsensitive) == 0) { found = &c; break; }
		if (!found) {
			if (code.isEmpty() && segment.optional) {
				d.codes.insert(segment.name, QString());
				continue;
			}
				//Never guessed: a code the rules do not offer is an error
			QStringList offered;
			for (const Code &c : segment.codes) offered << (c.code.isEmpty() ? tr("(none)") : c.code);
			d.error = tr("%1: the code %2 is not offered for %3 (offered: %4).")
					.arg(d.number, code.isEmpty() ? tr("(none)") : code, segment.label, offered.join(QStringLiteral(", ")));
			return d;
		}
		d.codes.insert(segment.name, found->code);
		for (auto it = found->values.cbegin() ; it != found->values.cend() ; ++it) {
			if (d.values.contains(it.key()) && d.values.value(it.key()) != it.value()) {
				d.error = tr("%1: the codes set %2 to %3 and to %4.")
						.arg(d.number, it.key(), d.values.value(it.key()), it.value());
				return d;
			}
			d.values.insert(it.key(), it.value());
		}
	}

		//The rules whose codes are all in the number; the general ones
		//(no codes) only when no other applies
	QList<int> applied;
	for (int i = 0 ; i < m_rules.size() ; ++i) {
		const Rule &rule = m_rules.at(i);
		if (rule.when.isEmpty()) continue;
		bool all = true;
		for (auto it = rule.when.cbegin() ; it != rule.when.cend() && all ; ++it)
			all = d.codes.contains(it.key())
				  && d.codes.value(it.key()).compare(it.value(), Qt::CaseInsensitive) == 0;
		if (all) applied << i;
	}
	if (applied.isEmpty())
		for (int i = 0 ; i < m_rules.size() ; ++i)
			if (m_rules.at(i).when.isEmpty()) applied << i;
	if (applied.isEmpty()) {
		d.error = tr("%1: no rule of the family %2 applies.").arg(d.number, m_family);
		return d;
	}
	for (int i : std::as_const(applied)) {
		const Rule &rule = m_rules.at(i);
		const auto take = [&](QString *field, const QString &value, const QString &what) {
			if (value.isEmpty()) return true;
			if (!field->isEmpty() && field->compare(value, Qt::CaseInsensitive) != 0) {
				d.error = tr("%1: the rules disagree on the %2: %3 or %4.").arg(d.number, what, *field, value);
				return false;
			}
			*field = value;
			return true;
		};
		if (!take(&d.templ, rule.templ, tr("template")) || !take(&d.variant, rule.variant, tr("variant")))
			return d;
		for (auto it = rule.values.cbegin() ; it != rule.values.cend() ; ++it) {
			if (d.values.contains(it.key()) && d.values.value(it.key()) != it.value()) {
				d.error = tr("%1: the rules set %2 to %3 and to %4.")
						.arg(d.number, it.key(), d.values.value(it.key()), it.value());
				return d;
			}
			d.values.insert(it.key(), it.value());
		}
	}
	if (d.templ.isEmpty()) {
		d.error = tr("%1: the rules which apply name no template.").arg(d.number);
		return d;
	}
	if (!d.variant.isEmpty()) d.values.insert(QStringLiteral("VARIANTE"), d.variant);
	d.rules = applied;
	d.ok = true;
	return d;
}

QList<ProductRules::Finding> ProductRules::check(int *numbers) const
{
	QList<Finding> findings;
	QSet<QString> said;
	const auto add = [&](Finding::Severity severity, const QString &text) {
		if (said.contains(text)) return;
		said.insert(text);
		findings << Finding{severity, text};
	};

		//What the file says, before any number
	for (int i = 0 ; i < m_rules.size() ; ++i) {
		const Rule &rule = m_rules.at(i);
		if (rule.templ.isEmpty() && rule.variant.isEmpty() && rule.values.isEmpty())
			add(Finding::Warning, tr("Rule %1 says nothing.").arg(i + 1));
		for (auto it = rule.when.cbegin() ; it != rule.when.cend() ; ++it) {
			const Segment *segment = nullptr;
			for (const Segment &s : m_segments) if (s.name == it.key()) segment = &s;
			if (!segment) {
				add(Finding::Error, tr("Rule %1 names the segment %2, which the family has not.").arg(i + 1).arg(it.key()));
				continue;
			}
			bool offered = it.value().isEmpty() && segment->optional;
			for (const Code &c : segment->codes) offered |= c.code.compare(it.value(), Qt::CaseInsensitive) == 0;
			if (!offered)
				add(Finding::Error, tr("Rule %1 names the code %2 of %3, which the segment does not offer.")
					.arg(i + 1).arg(it.value(), segment->label));
		}
	}
	for (const Segment &segment : m_segments)
		for (const Code &code : segment.codes)
			for (auto it = code.values.cbegin() ; it != code.values.cend() ; ++it)
				if (it.value().trimmed().isEmpty())
					add(Finding::Warning, tr("The code %1 of %2 sets %3 to nothing.")
						.arg(code.code, segment.label, it.key()));

		//Every number the rules allow, as the generator would draw it
	QList<QStringList> choices;
	for (const Segment &segment : m_segments) {
		QStringList codes;
		for (const Code &c : segment.codes) codes << c.code;
		if (segment.optional && !codes.contains(QString())) codes << QString();
		choices << codes;
	}
	QList<int> at(choices.size(), 0);
	QSet<int> used;
	QHash<QString, FunctionTemplate> templates;
	int count = 0;
	bool done = choices.isEmpty();
	for (const QStringList &c : std::as_const(choices)) done |= c.isEmpty();
	while (!done && count < MaxNumbers) {
		QStringList parts{m_family};
		bool gap = false;
		int last = -1;
		for (int i = 0 ; i < choices.size() ; ++i)
			if (!choices.at(i).at(at.at(i)).isEmpty()) last = i;
		for (int i = 0 ; i <= last ; ++i) {
			const QString code = choices.at(i).at(at.at(i));
				//An empty code in the middle cannot be written
			if (code.isEmpty()) gap = true;
			parts << code;
		}
		if (!gap) {
			++count;
			const QString number = parts.join(m_separator);
			const Decoded d = decode(number);
			if (!d.ok) {
				add(Finding::Error, d.error);
			} else {
				for (int r : d.rules) used.insert(r);
				if (!templates.contains(d.templ)) templates.insert(d.templ, FunctionTemplate::find(d.templ));
				const FunctionTemplate tpl = templates.value(d.templ);
				if (!tpl.isValid()) {
					add(Finding::Error, tr("The template %1 the rules name is not there.").arg(d.templ));
				} else {
					QStringList notes;
					QString chain, error;
					const QMap<QString, QString> resolved = tpl.resolve(d.values, nullptr, &notes);
					const FunctionTemplate drawn = tpl.chosen(resolved, &chain, &notes, &error);
					if (!drawn.isValid()) {
						add(Finding::Error, tr("%1: %2").arg(number, error));
					} else {
						if (!d.variant.isEmpty()
								&& !chain.split(QStringLiteral(" > ")).contains(d.variant, Qt::CaseInsensitive))
							add(Finding::Error, tr("%1: the variant %2 is no variant of %3 (taken: %4).")
								.arg(number, d.variant, tpl.name(), chain));
						const QStringList names = known(tpl) + known(drawn);
						for (auto it = d.values.cbegin() ; it != d.values.cend() ; ++it)
							if (it.key() != QLatin1String("VARIANTE") && !names.contains(it.key(), Qt::CaseInsensitive))
								add(Finding::Warning, tr("%1 is no value of %2: it is not used.").arg(it.key(), drawn.name()));
						for (const QString &note : std::as_const(notes))
							add(Finding::Warning, tr("%1: %2").arg(number, note));
					}
				}
			}
		}
			//The next combination
		int i = int(choices.size()) - 1;
		for (; i >= 0 ; --i) {
			if (++at[i] < choices.at(i).size()) break;
			at[i] = 0;
		}
		done = i < 0;
	}
	if (!done)
		add(Finding::Note, tr("Only the first %1 numbers were tried.").arg(MaxNumbers));
	for (int i = 0 ; i < m_rules.size() ; ++i)
		if (!used.contains(i))
			add(Finding::Note, tr("Rule %1 applies to no number.").arg(i + 1));
	if (numbers) *numbers = count;
	return findings;
}

QString ProductRules::severityName(Finding::Severity severity)
{
	switch (severity) {
		case Finding::Error: return tr("error");
		case Finding::Warning: return tr("warning");
		case Finding::Note: return tr("note");
	}
	return QString();
}
