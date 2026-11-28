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
#include "functiontemplate.h"
#include "templateexpression.h"

#include "../ElementsCollection/elementslocation.h"
#include "../ElementsCollection/xmlelementcollection.h"
#include "../NameList/nameslist.h"
#include "../TerminalStrip/drawnterminals.h"
#include "../TerminalStrip/terminalstripautoimport.h"
#include "../cable/addcablecommand.h"
#include "../cable/cable.h"
#include "../cable/cablecopy.h"
#include "../cable/cablepart.h"
#include "../diagram.h"
#include "../diagramcommands.h"
#include "../diagramcontent.h"
#include "../diagramevent/macrolabels.h"
#include "../properties/elementdata.h"
#include "../qetapp.h"
#include "../qetgraphicsitem/element.h"
#include "../qetproject.h"
#include "../undocommand/linkelementcommand.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QRegularExpression>
#include <QScopedPointer>
#include <QSettings>
#include <QTextStream>
#include <QUuid>

#include <algorithm>

namespace
{
	const char *const BlockTag = "function_template";

	QList<QDomElement> childrenOf(const QDomElement &parent, const QString &tag)
	{
		QList<QDomElement> list;
		for (QDomElement e = parent.firstChildElement(tag) ; !e.isNull()
			 ; e = e.nextSiblingElement(tag))
			list << e;
		return list;
	}

	QDomElement diagramOf(const QDomElement &root)
	{
		return root.firstChildElement(QStringLiteral("diagram_content"))
				.firstChildElement(QStringLiteral("diagram"));
	}

		/// {NAME}, {NAME+2}, {NAME-1}
	const QRegularExpression &placeholderPattern()
	{
		static const QRegularExpression pattern(
					QStringLiteral("\\{([A-Za-z_][A-Za-z0-9_]*)(?:([+-])(\\d+))?\\}"));
		return pattern;
	}

	const QRegularExpression &uuidPattern()
	{
		static const QRegularExpression pattern(
					QStringLiteral("\\{[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-"
								   "[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\}"));
		return pattern;
	}

	bool number(const QString &text, double *value)
	{
		QString t = text.trimmed();
		t.replace(QLatin1Char(','), QLatin1Char('.'));
		bool ok = false;
		*value = t.toDouble(&ok);
		return ok;
	}

	QString escaped(const QString &text)
	{
		return QString(text).replace(QLatin1Char('&'), QStringLiteral("&amp;"))
				.replace(QLatin1Char('<'), QStringLiteral("&lt;"))
				.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
	}

		/// Calls @p f on every element node below and including @p e
	template <typename F>
	void walk(QDomElement e, F f)
	{
		f(e);
		for (QDomElement child = e.firstChildElement() ; !child.isNull()
			 ; child = child.nextSiblingElement())
			walk(child, f);
	}

		/// Every attribute and text of @p root, passed through @p change
	template <typename F>
	void rewrite(QDomElement root, F change)
	{
		walk(root, [&](QDomElement e) {
			const QDomNamedNodeMap attributes = e.attributes();
			for (int i = 0 ; i < attributes.count() ; ++i) {
				QDomAttr attribute = attributes.item(i).toAttr();
				const QString before = attribute.value();
				const QString after = change(e, attribute.name(), before);
				if (after != before) attribute.setValue(after);
			}
			for (QDomNode child = e.firstChild() ; !child.isNull() ; child = child.nextSibling()) {
				if (!child.isText()) continue;
				QDomText text = child.toText();
				const QString before = text.data();
				const QString after = change(e, QString(), before);
				if (after != before) text.setData(after);
			}
		});
	}

		/// Put the element definitions a template carries into the
		/// embedded collection of @p project (as DiagramEventAddMacro does)
	void embedCollection(QETProject *project, const QDomElement &root)
	{
		XmlElementCollection *collection = project->embeddedElementCollection();
		for (const QDomElement &element : childrenOf(root.firstChildElement(QStringLiteral("collection")),
													 QStringLiteral("element"))) {
			const QString path = element.attribute(QStringLiteral("path"));
			const QDomElement definition = element.firstChildElement(QStringLiteral("definition"));
			if (path.isEmpty() || definition.isNull()) continue;
			const int slash = path.lastIndexOf(QLatin1Char('/'));
			const QString dir_path = slash >= 0 ? path.left(slash) : QString();
			const QString file_name = slash >= 0 ? path.mid(slash + 1) : path;
			QString current;
			for (const QString &part : dir_path.split(QLatin1Char('/'), Qt::SkipEmptyParts)) {
				const QString parent = current;
				if (!current.isEmpty()) current += QLatin1Char('/');
				current += part;
				if (current == QLatin1String("import")) continue;
				collection->createDir(parent, part, NamesList());
			}
			collection->addElementDefinition(dir_path, file_name, definition);
		}
	}

		/// %I0.0, E0.0, I 0.0 are one address; A is Q in German
	QString plainAddress(QString address)
	{
		address = address.trimmed().toUpper();
		address.remove(QLatin1Char('%'));
		address.remove(QLatin1Char(' '));
		if (address.startsWith(QLatin1Char('E'))) address[0] = QLatin1Char('I');
		else if (address.startsWith(QLatin1Char('A'))) address[0] = QLatin1Char('Q');
		return address;
	}

	bool sameAddress(const QString &channel, const ElementData::PlcIO &io)
	{
		const QString wanted = plainAddress(channel);
		const QString offered = plainAddress(io.address);
		if (wanted.isEmpty() || offered.isEmpty()) return false;
		if (wanted == offered) return true;
			//An IO saved as "1.0" says by its type whether it is I or Q
		if (offered.at(0).isDigit() && !wanted.at(0).isDigit()) {
			const bool input = io.type == ElementData::EntreeDigitale
					|| io.type == ElementData::EntreeAnalogique
					|| io.type == ElementData::EntreeUniverselle;
			const QChar letter = wanted.at(0);
			int digits_at = 1;
			while (digits_at < wanted.size() && !wanted.at(digits_at).isDigit()) ++digits_at;
			if (wanted.mid(digits_at) == offered) {
				if (letter == QLatin1Char('I')) return input;
				if (letter == QLatin1Char('Q')) return !input;
			}
		}
		return false;
	}

	QString fileNameFor(const QString &name)
	{
		QString file = name.trimmed();
		static const QRegularExpression bad(QStringLiteral("[\\\\/:*?\"<>|]"));
		file.replace(bad, QStringLiteral("_"));
		return file.isEmpty() ? QStringLiteral("Vorlage") : file;
	}

		/// The XML of @p items, as the clipboard holds it, cable lines included
	QDomDocument selectionXml(Diagram *folio, QList<QGraphicsItem *> items)
	{
		const QList<QGraphicsItem *> selected = folio->selectedItems();
		if (items.isEmpty()) {
			for (QGraphicsItem *item : folio->items())
				if (!item->parentItem() && (item->flags() & QGraphicsItem::ItemIsSelectable))
					items << item;
		}
		folio->clearSelection();
		for (QGraphicsItem *item : std::as_const(items)) item->setSelected(true);
		QDomDocument document = folio->toXml(false, true);
		CableCopy::write(folio, document);
		folio->clearSelection();
		for (QGraphicsItem *item : selected) item->setSelected(true);
		return document;
	}
}

/**
	@brief FunctionTemplate::fromFile
	@return the template stored in @p path; not valid when it cannot be read
*/
FunctionTemplate FunctionTemplate::fromFile(const QString &path, QString *error)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		if (error) *error = tr("The file %1 cannot be read.").arg(QDir::toNativeSeparators(path));
		return FunctionTemplate();
	}
	QDomDocument document;
	if (!document.setContent(&file)) {
		if (error) *error = tr("The file %1 is no valid XML.").arg(QDir::toNativeSeparators(path));
		return FunctionTemplate();
	}
	FunctionTemplate tpl = fromDocument(document, QFileInfo(path).absoluteFilePath());
	if (!tpl.isValid() && error)
		*error = tr("The file %1 is no template.").arg(QDir::toNativeSeparators(path));
	return tpl;
}

FunctionTemplate FunctionTemplate::fromDocument(const QDomDocument &document, const QString &path)
{
	FunctionTemplate tpl;
	tpl.m_document = document;
	tpl.m_path = path;
	const QDomElement root = document.documentElement();
	tpl.m_has_drawing = !diagramOf(root).isNull();
		//A typical made of variants only has no drawing of its own
	const bool variants = !root.firstChildElement(QLatin1String(BlockTag))
			.firstChildElement(QStringLiteral("variant")).isNull();
	tpl.m_valid = root.tagName() == QLatin1String("qet_macro") && (tpl.m_has_drawing || variants);
	if (tpl.m_valid) tpl.readBlock();
	return tpl;
}

void FunctionTemplate::readBlock()
{
	const QDomElement block = m_document.documentElement().firstChildElement(QLatin1String(BlockTag));
	m_has_block = !block.isNull();
	m_name = block.attribute(QStringLiteral("name"));
	if (m_name.isEmpty()) m_name = QFileInfo(m_path).completeBaseName();
	m_category = block.attribute(QStringLiteral("category"));
	if (m_category.isEmpty() && !m_path.isEmpty()) {
		const QString folder = QFileInfo(m_path).dir().dirName();
		m_category = folder.compare(QLatin1String("macros"), Qt::CaseInsensitive) == 0
				? QString() : folder;
	}
	m_description = block.firstChildElement(QStringLiteral("description")).text().trimmed();
	if (m_description.isEmpty()) m_description = block.attribute(QStringLiteral("description"));

	for (const QDomElement &p : childrenOf(block, QStringLiteral("parameter"))) {
		Parameter parameter;
		parameter.name = p.attribute(QStringLiteral("name")).trimmed();
		if (parameter.name.isEmpty()) continue;
		parameter.label = p.attribute(QStringLiteral("label"), parameter.name);
		parameter.defaultValue = p.attribute(QStringLiteral("default"));
		parameter.kind = p.attribute(QStringLiteral("kind"), QStringLiteral("text"));
		parameter.unit = p.attribute(QStringLiteral("unit"));
		parameter.of = p.attribute(QStringLiteral("of"));
		parameter.help = p.attribute(QStringLiteral("help"));
		const QString choices = p.attribute(QStringLiteral("choices"));
		if (!choices.isEmpty()) parameter.choices = choices.split(QLatin1Char(';'), Qt::SkipEmptyParts);
		parameter.io = p.attribute(QStringLiteral("io")).trimmed().toUpper();
		parameter.count = qMax(1, p.attribute(QStringLiteral("count"), QStringLiteral("1")).toInt());
		m_parameters << parameter;
	}
		//A ?name value of a plain macro (MacroLabels, upstream's way of
		//asking) is a text parameter of the same form, so a template
		//never asks twice
	if (m_has_block && m_has_drawing) {
		QStringList asked;
		rewrite(diagramOf(m_document.documentElement()), [&](const QDomElement &, const QString &, const QString &value) {
			if (value.contains(QLatin1Char('?')))
				for (const QString &name : MacroLabels::namedValues(value))
					if (!asked.contains(name.mid(1))) asked << name.mid(1); // without its ?
			return value;
		});
		for (const QString &name : std::as_const(asked)) {
			bool known = false;
			for (const Parameter &p : std::as_const(m_parameters)) known |= p.name == name;
			if (known) continue;
			Parameter parameter;
			parameter.name = name;
			parameter.label = name;
			parameter.kind = QStringLiteral("text");
			m_parameters << parameter;
			m_question_names << name;
		}
	}
	for (const QDomElement &t : childrenOf(block, QStringLiteral("table"))) {
		Table table;
		table.name = t.attribute(QStringLiteral("name"));
		table.key = t.attribute(QStringLiteral("key"));
		if (table.key.isEmpty()) continue;
		table.columns << table.key;
			//A table of a data sheet or a standard, as a CSV beside the
			//template: its rows come before the ones written in the block
		table.file = t.attribute(QStringLiteral("file"));
		if (!table.file.isEmpty()) {
			const QString path = QFileInfo(table.file).isAbsolute() || m_path.isEmpty()
					? table.file : QFileInfo(m_path).dir().filePath(table.file);
			QStringList columns;
			for (const auto &row : readCsv(path, &columns)) {
				if (row.contains(table.key)) table.rows << row;
			}
			for (const QString &c : std::as_const(columns))
				if (!table.columns.contains(c)) table.columns << c;
		}
		for (const QDomElement &r : childrenOf(t, QStringLiteral("row"))) {
			QMap<QString, QString> row;
			const QDomNamedNodeMap attributes = r.attributes();
			for (int i = 0 ; i < attributes.count() ; ++i) {
				const QDomAttr a = attributes.item(i).toAttr();
				row.insert(a.name(), a.value());
				if (!table.columns.contains(a.name())) table.columns << a.name();
			}
			if (row.contains(table.key)) table.rows << row;
		}
		m_tables << table;
	}
	for (const QDomElement &f : childrenOf(block, QStringLiteral("formula"))) {
		Formula formula;
		formula.name = f.attribute(QStringLiteral("name")).trimmed();
		formula.expression = f.attribute(QStringLiteral("expr"));
		if (formula.expression.isEmpty()) formula.expression = f.text().trimmed();
		if (!formula.name.isEmpty() && !formula.expression.trimmed().isEmpty()) m_formulas << formula;
	}
	for (const QDomElement &v : childrenOf(block, QStringLiteral("variant"))) {
		Variant variant;
		variant.name = v.attribute(QStringLiteral("name"));
		variant.condition = v.attribute(QStringLiteral("when"));
		variant.target = v.attribute(QStringLiteral("template")).trimmed();
		if (variant.name.isEmpty()) variant.name = QFileInfo(variant.target).completeBaseName();
		m_variants << variant;
	}
}

QList<QMap<QString, QString>> FunctionTemplate::readCsv(const QString &path, QStringList *columns,
														  QString *error)
{
	QList<QMap<QString, QString>> rows;
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		if (error) *error = tr("The file %1 cannot be read.").arg(QDir::toNativeSeparators(path));
		return rows;
	}
	QString text = QString::fromUtf8(file.readAll());
	if (text.startsWith(QChar(0xFEFF))) text.remove(0, 1);
	const QStringList lines = text.split(QLatin1Char('\n'));
	QStringList header;
	QChar separator = QLatin1Char(';');
	for (QString line : lines) {
		line.remove(QLatin1Char('\r'));
		if (line.trimmed().isEmpty() || line.trimmed().startsWith(QLatin1Char('#'))) continue;
		if (header.isEmpty() && !line.contains(QLatin1Char(';')))
			separator = line.contains(QLatin1Char('\t')) ? QLatin1Char('\t') : QLatin1Char(',');
			//Cells in quotes may hold the separator
		QStringList cells;
		QString cell;
		bool quoted = false;
		for (int i = 0 ; i < line.size() ; ++i) {
			const QChar c = line.at(i);
			if (c == QLatin1Char('"')) {
				if (quoted && i + 1 < line.size() && line.at(i + 1) == QLatin1Char('"')) { cell += c; ++i; }
				else quoted = !quoted;
			} else if (c == separator && !quoted) {
				cells << cell.trimmed();
				cell.clear();
			} else {
				cell += c;
			}
		}
		cells << cell.trimmed();
		if (header.isEmpty()) {
			header = cells;
			continue;
		}
		QMap<QString, QString> row;
		for (int i = 0 ; i < header.size() && i < cells.size() ; ++i)
			if (!header.at(i).isEmpty()) row.insert(header.at(i), cells.at(i));
		rows << row;
	}
	if (columns) *columns = header;
	return rows;
}

bool FunctionTemplate::isExample() const
{
	const QString examples = examplesDirectory();
	return !examples.isEmpty() && !m_path.isEmpty()
			&& QFileInfo(m_path).absoluteFilePath().startsWith(examples, Qt::CaseInsensitive);
}

QStringList FunctionTemplate::choicesFor(const Parameter &parameter) const
{
	if (!parameter.choices.isEmpty()) return parameter.choices;
	QStringList choices;
	for (const Table &table : m_tables) {
		if (table.key != parameter.name) continue;
		for (const auto &row : table.rows) choices << row.value(table.key);
	}
	return choices;
}

/**
	@brief FunctionTemplate::rowFor
	A number takes the smallest key which is at least as big, what
	EPLAN calls the next bigger size: a 3 kW motor takes the 4 kW row.
	Above the last row the last is taken, and @p note says so. Any
	other text has to match a key.
*/
int FunctionTemplate::rowFor(const Table &table, const QString &value, QString *note)
{
	double wanted = 0;
	if (number(value, &wanted)) {
		int best = -1;
		double best_key = 0;
		int biggest = -1;
		double biggest_key = 0;
		for (int i = 0 ; i < table.rows.size() ; ++i) {
			double key = 0;
			if (!number(table.rows.at(i).value(table.key), &key)) continue;
			if (key >= wanted - 1e-9 && (best < 0 || key < best_key)) {
				best = i;
				best_key = key;
			}
			if (biggest < 0 || key > biggest_key) {
				biggest = i;
				biggest_key = key;
			}
		}
		if (best >= 0) return best;
		if (biggest >= 0) {
			if (note) *note = tr("%1 = %2 is larger than the table goes (%3): its last row is used.")
							  .arg(table.key, value, table.rows.at(biggest).value(table.key));
			return biggest;
		}
	}
	for (int i = 0 ; i < table.rows.size() ; ++i)
		if (table.rows.at(i).value(table.key).trimmed().compare(value.trimmed(), Qt::CaseInsensitive) == 0)
			return i;
	if (note) *note = tr("The table for %1 holds no row for %2.").arg(table.key, value);
	return -1;
}

QMap<QString, QString> FunctionTemplate::proposals(Diagram *folio) const
{
	return resolve(QMap<QString, QString>(), folio, nullptr);
}

QMap<QString, QString> FunctionTemplate::resolve(const QMap<QString, QString> &given,
												 Diagram *folio, QStringList *notes) const
{
	QMap<QString, QString> values;
	QETProject *project = folio ? folio->project() : nullptr;

		//The labels asked for are proposed together, so that two of them
		//never get the same next free one
	QList<MacroLabels::Field> labels;
	for (const Parameter &p : m_parameters) {
		if (p.kind != QLatin1String("label") || given.contains(p.name)) continue;
		MacroLabels::Field field;
		field.kind = MacroLabels::Field::Label;
		field.saved = p.defaultValue.trimmed().isEmpty() ? p.name : p.defaultValue.trimmed();
		field.value = field.saved;
		labels << field;
	}
	if (folio && !labels.isEmpty()) MacroLabels::propose(labels, folio);

	int next_label = 0;
	for (const Parameter &p : m_parameters) {
		if (given.contains(p.name)) {
			values.insert(p.name, given.value(p.name));
			continue;
		}
		if (p.kind == QLatin1String("label")) {
			values.insert(p.name, labels.value(next_label++).value);
		} else if (p.kind == QLatin1String("cable")) {
				//The next cable designation nobody uses yet: W1, W2, ...
				//(a project with a cable numbering renumbers it anyway)
			QString value = p.defaultValue.trimmed().isEmpty() ? QStringLiteral("W1") : p.defaultValue.trimmed();
			if (project) {
				QSet<QString> taken;
				for (Cable *cable : project->cables()) taken.insert(cable->designation().trimmed());
				static const QRegularExpression last_number(QStringLiteral("(\\d+)(\\D*)$"));
				for (int guard = 0 ; taken.contains(value) && guard < 10000 ; ++guard) {
					const auto match = last_number.match(value);
					value = match.hasMatch()
							? value.left(match.capturedStart(1))
							  + QString::number(match.captured(1).toLongLong() + 1) + match.captured(2)
							: value + QStringLiteral("1");
				}
			}
			values.insert(p.name, value);
		} else if (p.kind == QLatin1String("address")) {
				//The next free channel of its kind the PLC modules offer
			const QString address = project ? nextFreeAddress(project, p.io, p.count) : QString();
			values.insert(p.name, address.isEmpty() ? p.defaultValue : address);
		} else if (p.kind == QLatin1String("terminal")) {
			const QString strip = values.contains(p.of) ? values.value(p.of)
														: given.value(p.of, p.of);
			if (project && !strip.trimmed().isEmpty())
				values.insert(p.name, QString::number(DrawnTerminals::nextNumber(project, strip)));
			else
				values.insert(p.name, p.defaultValue.isEmpty() ? QStringLiteral("1") : p.defaultValue);
		} else {
			values.insert(p.name, p.defaultValue);
		}
	}
		//Values given for something which is no parameter, a column of a
		//table for instance, override it
	for (auto it = given.cbegin() ; it != given.cend() ; ++it)
		values.insert(it.key(), it.value());

	for (const Table &table : m_tables) {
			//A table only formulas look up in (its key no value) is left alone
		if (values.value(table.key).trimmed().isEmpty()) continue;
		QString note;
		const int row = rowFor(table, values.value(table.key), &note);
		if (!note.isEmpty() && notes) notes->append(note);
		if (row < 0) continue;
		const auto &cells = table.rows.at(row);
		for (auto it = cells.cbegin() ; it != cells.cend() ; ++it) {
			if (it.key() == table.key || given.contains(it.key())) continue;
			bool is_parameter = false;
			for (const Parameter &p : m_parameters) is_parameter |= p.name == it.key();
			if (!is_parameter) values.insert(it.key(), it.value());
		}
	}

		//The formulas, in file order: each sees the values before it. A
		//value given for its name stands; the formula fills what is missing
	TemplateExpression::Context context;
	context.values = &values;
	context.notes = notes;
	context.lookup = [this](const QString &table, const QString &value, const QString &column,
							const QString &key, QString *note) {
		return lookup(table, value, column, key, note);
	};
	for (const Formula &f : m_formulas) {
		if (!given.value(f.name).trimmed().isEmpty()) continue;
		QString error;
		const QString value = TemplateExpression::evaluate(f.expression, context, &error);
		if (!error.isEmpty()) {
			if (notes) notes->append(tr("Formula %1 of %2: %3").arg(f.name, m_name, error));
			continue;
		}
		values.insert(f.name, value);
	}
	return values;
}

QString FunctionTemplate::lookup(const QString &table_name, const QString &value, const QString &column,
								 const QString &key, QString *note) const
{
	for (const Table &table : m_tables) {
		if (table.name.compare(table_name, Qt::CaseInsensitive) != 0) continue;
		Table picked = table;
		if (!key.isEmpty()) picked.key = key;
		QString row_note;
		const int row = rowFor(picked, value, &row_note);
		if (row < 0) {
			if (note) *note = tr("%1: %2").arg(table_name, row_note);
			return QString();
		}
		const auto &cells = picked.rows.at(row);
		if (!row_note.isEmpty() && note) *note = tr("%1: %2").arg(table_name, row_note);
			//A key column I_max of a table with I_min: the value has to
			//lie in the range, not only below its top
		if (picked.key.endsWith(QLatin1String("_max"), Qt::CaseInsensitive)) {
			const QString lower = picked.key.left(picked.key.size() - 4) + QStringLiteral("_min");
			double v = 0, least = 0;
			if (cells.contains(lower) && TemplateExpression::toNumber(value, &v)
					&& TemplateExpression::toNumber(cells.value(lower), &least) && v < least - 1e-9 && note)
				*note = tr("%1: %2 = %3 is below the range of the row picked (%4 ... %5).")
						.arg(table_name, picked.key.left(picked.key.size() - 4), value,
							 cells.value(lower), cells.value(picked.key));
		}
		if (!cells.contains(column)) {
			if (note) *note = tr("The table %1 has no column %2.").arg(table_name, column);
			return QString();
		}
		return cells.value(column);
	}
	if (note) *note = tr("The template %1 has no table %2.").arg(m_name, table_name);
	return QString();
}

FunctionTemplate FunctionTemplate::chosen(const QMap<QString, QString> &resolved, QString *chain,
										  QStringList *notes, QString *error) const
{
	FunctionTemplate current = *this;
	QMap<QString, QString> values = resolved;
	QStringList taken;
	QStringList visited{QFileInfo(m_path).absoluteFilePath()};
	for (int depth = 0 ; ; ++depth) {
		if (current.m_variants.isEmpty()) break;
		if (depth >= 8) {
			if (error) *error = tr("The variants of %1 go deeper than eight levels.").arg(m_name);
			return FunctionTemplate();
		}
		TemplateExpression::Context context;
		context.values = &values;
		context.notes = notes;
		context.lookup = [&current](const QString &table, const QString &value, const QString &column,
									const QString &key, QString *note) {
			return current.lookup(table, value, column, key, note);
		};
			//A variant asked for by name (VARIANTE: from a product number
			//or a column of the table) is taken whatever its condition
		const QString wanted = values.value(QStringLiteral("VARIANTE")).trimmed();
		const Variant *match = nullptr;
		if (!wanted.isEmpty())
			for (const Variant &v : std::as_const(current.m_variants))
				if (v.name.compare(wanted, Qt::CaseInsensitive) == 0) { match = &v; break; }
		for (const Variant &v : std::as_const(current.m_variants)) {
			if (match) break;
			if (v.condition.trimmed().isEmpty()) { match = &v; break; }
			QString e;
			const QString truth = TemplateExpression::evaluate(v.condition, context, &e);
			if (!e.isEmpty()) {
				if (notes) notes->append(tr("Condition of the variant %1 of %2: %3").arg(v.name, current.m_name, e));
				continue;
			}
			if (TemplateExpression::isTrue(truth)) { match = &v; break; }
		}
		if (!match) {
			if (current.m_has_drawing) break;
			if (error) *error = tr("No variant of %1 fits.").arg(current.m_name);
			return FunctionTemplate();
		}
		taken << match->name;
		if (match->target.isEmpty()) {
			if (current.m_has_drawing) break; // the template's own drawing
			if (error) *error = tr("The variant %1 of %2 names no template.").arg(match->name, current.m_name);
			return FunctionTemplate();
		}
			//A target beside this template first, then in the template
			//folders, then any by its name
		const QString target = match->target;
		FunctionTemplate next;
		QStringList candidates;
		if (!current.m_path.isEmpty()) {
			const QDir here = QFileInfo(current.m_path).dir();
			candidates << here.filePath(target) << here.filePath(target + QStringLiteral(".qetmak"));
		}
		for (const QString &dir : directories())
			candidates << QDir(dir).filePath(target) << QDir(dir).filePath(target + QStringLiteral(".qetmak"));
		for (const QString &candidate : std::as_const(candidates))
			if (QFileInfo(candidate).isFile()) { next = fromFile(candidate); if (next.isValid()) break; }
		if (!next.isValid()) next = find(target);
		if (!next.isValid()) {
			if (error) *error = tr("The variant %1 of %2 names %3, which is no template.")
							.arg(match->name, current.m_name, target);
			return FunctionTemplate();
		}
		const QString path = QFileInfo(next.m_path).absoluteFilePath();
		if (visited.contains(path, Qt::CaseInsensitive)) {
			if (error) *error = tr("The variants of %1 lead back to %2.").arg(m_name, next.m_name);
			return FunctionTemplate();
		}
		visited << path;
		current = next;
			//The next one's own tables and formulas, for its variants
		values = current.resolve(values, nullptr, nullptr);
	}
	if (chain) *chain = taken.join(QStringLiteral(" > "));
	return current;
}

QString FunctionTemplate::substitute(const QString &text, const QMap<QString, QString> &values,
									 QSet<QString> *unknown)
{
	if (!text.contains(QLatin1Char('{'))) return text;
	QString result;
	int last = 0;
	auto it = placeholderPattern().globalMatch(text);
	while (it.hasNext()) {
		const auto match = it.next();
		const QString name = match.captured(1);
		if (!values.contains(name)) {
			if (unknown) unknown->insert(name);
			continue;
		}
		QString value = values.value(name);
		if (match.capturedLength(3)) {
			const qlonglong offset = match.captured(3).toLongLong()
					* (match.captured(2) == QLatin1String("-") ? -1 : 1);
			static const QRegularExpression last_number(QStringLiteral("(\\d+)(\\D*)$"));
			const auto number_match = last_number.match(value);
			QString address;
				//A PLC channel counts in bits: %Q0.7 + 1 is %Q1.0
			if (addressPlus(value, int(offset), &address)) {
				value = address;
			} else if (number_match.hasMatch()) {
				value = value.left(number_match.capturedStart(1))
						+ QString::number(number_match.captured(1).toLongLong() + offset)
						+ number_match.captured(2);
			} else if (unknown) {
				unknown->insert(match.captured(0).mid(1, match.capturedLength(0) - 2));
			}
		}
		result += text.mid(last, match.capturedStart(0) - last) + value;
		last = match.capturedEnd(0);
	}
	result += text.mid(last);
	return result;
}

QStringList FunctionTemplate::placeholdersIn(const QDomElement &root)
{
	QStringList names;
	rewrite(root, [&](const QDomElement &, const QString &, const QString &value) {
		if (!value.contains(QLatin1Char('{'))) return value;
		auto it = placeholderPattern().globalMatch(value);
		while (it.hasNext()) {
			const QString name = it.next().captured(1);
			if (!names.contains(name)) names << name;
		}
		return value;
	});
	return names;
}

QDomDocument FunctionTemplate::instantiate(const QMap<QString, QString> &resolved,
										   Diagram *folio, QStringList *notes) const
{
	QDomDocument out;
	if (!m_valid) return out;
	const QDomElement root = m_document.documentElement();

		//The labels the template holds as they are, without a
		//placeholder, go on to the next free one, as for any template
	QList<MacroLabels::Field> fixed;
	for (const MacroLabels::Field &field : MacroLabels::fields(root))
		if (field.kind == MacroLabels::Field::Label && !field.saved.contains(QLatin1Char('{')))
			fixed << field;
	if (folio && !fixed.isEmpty()) MacroLabels::propose(fixed, folio);

	out.appendChild(out.importNode(diagramOf(root), true));
	QDomElement diagram = out.documentElement();

		//Fresh identities: every symbol, wire and text of the copy is one
		//of its own, so the template can be placed again and again; the
		//links between its symbols and the ends of its wires follow
	QHash<QString, QString> renewed;
	walk(diagram, [&](QDomElement e) {
		static const QStringList owners{QStringLiteral("element"), QStringLiteral("conductor"),
										QStringLiteral("input"), QStringLiteral("shape"),
										QStringLiteral("image"), QStringLiteral("dimension")};
		if (!owners.contains(e.tagName()) || !e.hasAttribute(QStringLiteral("uuid"))) return;
		const QString old_uuid = e.attribute(QStringLiteral("uuid"));
		if (QUuid(old_uuid).isNull() || renewed.contains(old_uuid)) return;
		renewed.insert(old_uuid, QUuid::createUuid().toString());
	});
	QSet<QString> unknown;
	QMap<QString, QString> html_values;
	for (auto it = resolved.cbegin() ; it != resolved.cend() ; ++it)
		html_values.insert(it.key(), escaped(it.value()));
	rewrite(diagram, [&](const QDomElement &e, const QString &attribute, const QString &value) {
		QString changed = value;
		if (changed.contains(QLatin1Char('{'))) {
			if (!renewed.isEmpty()) {
				QString with_uuids;
				int last = 0;
				auto it = uuidPattern().globalMatch(changed);
				while (it.hasNext()) {
					const auto match = it.next();
					with_uuids += changed.mid(last, match.capturedStart(0) - last)
							+ renewed.value(match.captured(0), match.captured(0));
					last = match.capturedEnd(0);
				}
				changed = with_uuids + changed.mid(last);
			}
			const bool html = e.tagName() == QLatin1String("input") && attribute == QLatin1String("text");
			changed = substitute(changed, html ? html_values : resolved, &unknown);
		}
		if (!m_question_names.isEmpty() && changed.contains(QLatin1Char('?'))) {
			const bool html = e.tagName() == QLatin1String("input") && attribute == QLatin1String("text");
			QHash<QString, QString> asked;
			for (const QString &name : m_question_names)
				asked.insert(QLatin1Char('?') + name, html ? html_values.value(name) : resolved.value(name));
			changed = MacroLabels::withValues(changed, asked);
		}
		if (e.tagName() == QLatin1String("element") && attribute == QLatin1String("type")
				&& changed.startsWith(QLatin1String("macro://")))
			changed.replace(0, 8, QStringLiteral("embed://"));
		return changed;
	});
	if (folio && !fixed.isEmpty()) MacroLabels::apply(diagram, fixed);

	if (notes && !unknown.isEmpty()) {
		QStringList names(unknown.cbegin(), unknown.cend());
		std::sort(names.begin(), names.end());
		for (QString &name : names) name = QLatin1Char('{') + name + QLatin1Char('}');
		notes->append(tr("No value for %1: left as it is.").arg(names.join(QStringLiteral(", "))));
	}
	return out;
}

QImage FunctionTemplate::preview(const QMap<QString, QString> &values, const QSize &max) const
{
	if (!m_valid) return QImage();
	if (!m_has_drawing) {
		const QMap<QString, QString> all = resolve(values, nullptr);
		QMap<QString, QString> resolved;
		for (auto it = all.cbegin() ; it != all.cend() ; ++it)
			if (!it.value().trimmed().isEmpty()) resolved.insert(it.key(), it.value());
		const FunctionTemplate target = chosen(resolved);
		return target.isValid() ? target.preview(resolved, max) : QImage();
	}
	QScopedPointer<QETProject> project(new QETProject());
	embedCollection(project.data(), m_document.documentElement());
	Diagram *folio = project->addNewDiagram();
	if (!folio) return QImage();
	folio->setDisplayGrid(false);
	QDomDocument document = instantiate(resolve(values, nullptr), nullptr);
	QList<CablePart *> cables;
	folio->fromXml(document, QPointF(), false, nullptr, &cables, nullptr);
	folio->refreshContents();

	QRectF rect;
	for (QGraphicsItem *item : folio->items())
		if (!item->parentItem() && item->isVisible())
			rect = rect.united(item->sceneBoundingRect());
	if (rect.isEmpty()) return QImage();
	rect.adjust(-10, -10, 10, 10);
		//An empty @p max: the drawing at its own size, for the cursor
	const QSize size = max.isEmpty() ? rect.size().toSize()
									 : rect.size().toSize().scaled(max, Qt::KeepAspectRatio)
									   .boundedTo(rect.size().toSize() * 2);
	QImage image(size.expandedTo(QSize(1, 1)), QImage::Format_ARGB32_Premultiplied);
	image.fill(Qt::white);
	QPainter painter(&image);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);
	folio->render(&painter, QRectF(QPointF(0, 0), QSizeF(image.size())), rect);
	painter.end();
	return image;
}

FunctionTemplate::Placed FunctionTemplate::place(Diagram *folio, const QMap<QString, QString> &given,
												 QPointF pos) const
{
	Placed placed;
	placed.folio = folio;
	if (!m_valid) {
		placed.error = tr("This is no valid template.");
		return placed;
	}
	if (!folio || !folio->project()) {
		placed.error = tr("There is no folio to place the template on.");
		return placed;
	}
	QETProject *project = folio->project();
	if (folio->isReadOnly() || project->isReadOnly()) {
		placed.error = tr("The project is read-only.");
		return placed;
	}
	project->buildFolioToShow(folio);

	if (!m_variants.isEmpty()) {
			//A typical: its values made, then the variant they choose
			//placed with all of them
		QStringList notes;
		const QMap<QString, QString> values = resolve(given, folio, &notes);
		QString chain, error;
		const FunctionTemplate target = chosen(values, &chain, &notes, &error);
		if (!target.isValid()) {
			placed.error = error;
			placed.notes = notes;
			return placed;
		}
		FunctionTemplate drawn = target;
		drawn.m_variants.clear();	// chosen already
			//An empty value is none: the variant fills it from its own
			//tables (a current left empty, the article of its row)
		QMap<QString, QString> passed;
		for (auto it = values.cbegin() ; it != values.cend() ; ++it)
			if (!it.value().trimmed().isEmpty()) passed.insert(it.key(), it.value());
		Placed result = drawn.place(folio, passed, pos);
		result.notes = notes + result.notes;
		result.variant = chain;
		return result;
	}
	placed.placedTemplate = m_path;

	placed.values = resolve(given, folio, &placed.notes);
	QDomDocument document = instantiate(placed.values, folio, &placed.notes);
	embedCollection(project, m_document.documentElement());

		//What the paste and the cable numbering would wipe out: the
		//designation the template gives its cables, the PLC address of
		//its channels (a pasted PLC channel which is free starts clean)
	QStringList designations;
	for (QDomElement c = document.documentElement().firstChildElement(QStringLiteral("cables"))
		 .firstChildElement(QStringLiteral("cable")) ; !c.isNull() ; c = c.nextSiblingElement(QStringLiteral("cable")))
		if (!c.attribute(QStringLiteral("x1")).isEmpty())
			designations << c.attribute(QStringLiteral("designation")).trimmed();

	DiagramContent content;
	QList<CablePart *> cables;
	folio->fromXml(document, pos, false, &content, &cables, nullptr);
	folio->refreshContents();
	for (int i = 0 ; i < cables.size() && i < designations.size() ; ++i) {
		Cable *cable = cables.at(i) ? cables.at(i)->cable() : nullptr;
		if (!cable || designations.at(i).isEmpty() || designations.at(i).contains(QLatin1Char('{'))) continue;
		cable->setDesignation(designations.at(i));
		cable->setDesignationByHand(true);
	}
	QHash<Element *, QString> addresses;
	for (Element *element : std::as_const(content.m_elements)) {
		const QString address = element->elementInformations()
				.value(QStringLiteral("plc_address")).toString().trimmed();
		if (!address.isEmpty() && element->linkType() == Element::Slave)
			addresses.insert(element, address);
	}
	const QList<CableCopy::Wired> wired = CableCopy::wire(
				folio, cables, content.conductors(DiagramContent::AnyConductor));
	if (!content.count() && wired.isEmpty()) {
		placed.error = tr("The template holds nothing to place.");
		return placed;
	}

		//The paste keeps the labels the template was filled with, and
		//numbers nothing itself
	QSettings settings;
	const bool erase = settings.value(QStringLiteral("diagramcommands/erase-label-on-copy"), true).toBool();
	settings.setValue(QStringLiteral("diagramcommands/erase-label-on-copy"), false);

	QUndoStack &stack = folio->undoStack();
	stack.beginMacro(tr("Insert function template %1").arg(m_name));
	auto *paste = new PasteDiagramCommand(folio, content);
	paste->setAutoNumbering(false);
	stack.push(paste);
	for (const CableCopy::Wired &line : wired) {
		if (!line.part) continue;
		stack.push(new AddCableCommand(line.part->cable(), line.part.data(), folio, true, line.taken));
	}
	CableCopy::repoint(wired);
		//Terminals labelled "X1:3" go into their strip right away
	TerminalStripAutoImport::importFreeTerminals(project);
	linkPlcChannels(folio, addresses, &placed.notes);
	stack.endMacro();
	settings.setValue(QStringLiteral("diagramcommands/erase-label-on-copy"), erase);

	placed.elements = content.m_elements;
	placed.ok = true;
	return placed;
}

FunctionTemplate::Placed FunctionTemplate::placeOnNewFolio(QETProject *project,
														   const QMap<QString, QString> &given) const
{
	Placed placed;
	if (!project || project->isReadOnly()) {
		placed.error = tr("The project is read-only.");
		return placed;
	}
	Diagram *folio = project->addNewDiagram();
	if (!folio) {
		placed.error = tr("No folio could be added.");
		return placed;
	}
	placed = place(folio, given, QPointF());
	if (placed.ok) {
		TitleBlockProperties properties = folio->border_and_titleblock.exportTitleBlock();
		QString title = m_name;
		for (const Parameter &p : m_parameters)
			if (p.kind == QLatin1String("label") && !placed.values.value(p.name).isEmpty()) {
				title += QStringLiteral(" -") + placed.values.value(p.name);
				break;
			}
		properties.title = title;
		folio->border_and_titleblock.importTitleBlock(properties);
	}
	return placed;
}

/**
	@brief FunctionTemplate::linkPlcChannels
	A PLC channel of the template whose plc_address one of the PLC
	modules of the project offers is linked to that module, on that
	channel: "Ventil mit DQ-Kanal" placed for %Q2.3 lands on the module
	which has Q2.3.
*/
void FunctionTemplate::linkPlcChannels(Diagram *folio, const QHash<Element *, QString> &addresses,
									   QStringList *notes)
{
	QETProject *project = folio->project();
	QList<Element *> masters;
	bool scanned = false;
	for (auto it = addresses.cbegin() ; it != addresses.cend() ; ++it) {
		Element *channel = it.key();
		const QString address = it.value();
		if (!channel->linkedElements().isEmpty()) continue;
		if (!scanned) {
			scanned = true;
			for (Diagram *d : project->folios()) {
				project->buildFolioToShow(d);
				for (QGraphicsItem *item : d->items()) {
					auto *element = qgraphicsitem_cast<Element *>(item);
					if (!element) continue;
					const ElementData e = element->elementData();
					if (e.m_type == ElementData::Master && e.m_master_type == ElementData::PLC)
						masters << element;
				}
			}
		}
		bool linked = false;
		bool occupied = false;
		for (Element *master : std::as_const(masters)) {
			const auto ios = master->elementData().plcMasterData().ios;
			for (int i = 0 ; i < ios.size() && !linked ; ++i) {
				if (!sameAddress(address, ios.at(i))) continue;
					//A channel another symbol is already on stays its own
				bool taken = false;
				for (Element *other : master->linkedElements())
					taken |= master->groupIndexForElement(other) == i;
				if (taken) {
					if (notes) notes->append(tr("PLC channel %1 of %2 is already used: the channel is left unlinked.")
											 .arg(address, master->elementInformations()
												  .value(QStringLiteral("label")).toString()));
					occupied = true;
					continue;
				}
				auto *command = new LinkElementCommand(channel);
				command->setLink(master);
				command->setGroupIndex(i);
				folio->undoStack().push(command);
				linked = channel->linkedElements().contains(master);
				if (linked && notes)
					notes->append(tr("PLC channel %1 linked to %2.")
								  .arg(address, master->elementInformations()
									   .value(QStringLiteral("label")).toString()));
			}
			if (linked) break;
		}
		if (linked) continue;
			//Unlinked, it keeps showing the address it was given
		DiagramContext informations = channel->elementInformations();
		informations.addValue(QStringLiteral("plc_address"), address);
		channel->setElementInformations(informations);
		if (notes && !masters.isEmpty() && !occupied)
			notes->append(tr("No PLC module of the project offers %1: the channel is left unlinked.")
						  .arg(address));
	}
}

QString FunctionTemplate::userDirectory()
{
	return QDir(QETApp::userMacrosDir()).filePath(QStringLiteral("Funktionsvorlagen"));
}

QString FunctionTemplate::examplesDirectory()
{
	const QString app = QCoreApplication::applicationDirPath();
	const QStringList candidates{
		app + QStringLiteral("/funktionsvorlagen"),
		app + QStringLiteral("/../funktionsvorlagen"),
		app + QStringLiteral("/../share/qelectrotech/funktionsvorlagen"),
		app + QStringLiteral("/../Resources/funktionsvorlagen"),
	};
	for (const QString &candidate : candidates)
		if (QFileInfo(candidate).isDir()) return QDir(candidate).absolutePath();
	return QString();
}

QStringList FunctionTemplate::directories()
{
	QStringList dirs;
	const auto add = [&](const QString &dir) {
		if (dir.isEmpty() || !QFileInfo(dir).isDir()) return;
		const QString absolute = QDir(dir).absolutePath();
		for (const QString &known : std::as_const(dirs))
			if (absolute.startsWith(known + QLatin1Char('/'), Qt::CaseInsensitive)
					|| absolute.compare(known, Qt::CaseInsensitive) == 0)
				return;
		dirs << absolute;
	};
		//Every template of the user, function templates and plain ones
	add(QETApp::userMacrosDir());
	for (const QString &dir : qEnvironmentVariable("QET_FUNCTION_TEMPLATES_DIR")
		 .split(QDir::listSeparator(), Qt::SkipEmptyParts))
		add(dir);
	add(examplesDirectory());
	return dirs;
}

QList<FunctionTemplate> FunctionTemplate::available()
{
	QList<FunctionTemplate> list;
	for (const QString &dir : directories()) {
		QDirIterator it(dir, {QStringLiteral("*.qetmak")}, QDir::Files, QDirIterator::Subdirectories);
		QStringList files;
		while (it.hasNext()) files << it.next();
		std::sort(files.begin(), files.end());
		for (const QString &file : std::as_const(files)) {
			FunctionTemplate tpl = fromFile(file);
			if (tpl.isValid()) list << tpl;
		}
	}
	return list;
}

FunctionTemplate FunctionTemplate::find(const QString &name_or_path)
{
	const QString wanted = name_or_path.trimmed();
	if (wanted.isEmpty()) return FunctionTemplate();
	if (QFileInfo(wanted).isFile()) return fromFile(wanted);
	const QList<FunctionTemplate> all = available();
	QString relative = QDir::fromNativeSeparators(wanted);
	if (!relative.endsWith(QLatin1String(".qetmak"), Qt::CaseInsensitive))
		relative += QStringLiteral(".qetmak");
	for (const FunctionTemplate &tpl : all)
		if (QDir::fromNativeSeparators(tpl.path()).endsWith(QLatin1Char('/') + relative, Qt::CaseInsensitive))
			return tpl;
	for (const FunctionTemplate &tpl : all)
		if (tpl.name().compare(wanted, Qt::CaseInsensitive) == 0)
			return tpl;
	for (const FunctionTemplate &tpl : all)
		if ((tpl.category() + QLatin1Char('/') + tpl.name()).compare(wanted, Qt::CaseInsensitive) == 0)
			return tpl;
	return FunctionTemplate();
}

QDomElement FunctionTemplate::blockXml(QDomDocument &document, const SaveOptions &options)
{
	QDomElement block = document.createElement(QLatin1String(BlockTag));
	block.setAttribute(QStringLiteral("version"), 1);
	block.setAttribute(QStringLiteral("name"), options.name);
	if (!options.category.isEmpty()) block.setAttribute(QStringLiteral("category"), options.category);
	if (!options.description.isEmpty()) {
		QDomElement description = document.createElement(QStringLiteral("description"));
		description.appendChild(document.createTextNode(options.description));
		block.appendChild(description);
	}
	for (const Parameter &p : options.parameters) {
		QDomElement e = document.createElement(QStringLiteral("parameter"));
		e.setAttribute(QStringLiteral("name"), p.name);
		if (!p.label.isEmpty() && p.label != p.name) e.setAttribute(QStringLiteral("label"), p.label);
		if (!p.kind.isEmpty() && p.kind != QLatin1String("text")) e.setAttribute(QStringLiteral("kind"), p.kind);
		e.setAttribute(QStringLiteral("default"), p.defaultValue);
		if (!p.unit.isEmpty()) e.setAttribute(QStringLiteral("unit"), p.unit);
		if (!p.of.isEmpty()) e.setAttribute(QStringLiteral("of"), p.of);
		if (!p.help.isEmpty()) e.setAttribute(QStringLiteral("help"), p.help);
		if (!p.choices.isEmpty()) e.setAttribute(QStringLiteral("choices"), p.choices.join(QLatin1Char(';')));
		if (!p.io.isEmpty()) e.setAttribute(QStringLiteral("io"), p.io);
		if (p.count > 1) e.setAttribute(QStringLiteral("count"), p.count);
		block.appendChild(e);
	}
	for (const Table &t : options.tables) {
		QDomElement e = document.createElement(QStringLiteral("table"));
		if (!t.name.isEmpty()) e.setAttribute(QStringLiteral("name"), t.name);
		e.setAttribute(QStringLiteral("key"), t.key);
		for (const auto &row : t.rows) {
			QDomElement r = document.createElement(QStringLiteral("row"));
				//In the column order of the table, the key first
			QStringList columns = t.columns;
			for (auto it = row.cbegin() ; it != row.cend() ; ++it)
				if (!columns.contains(it.key())) columns << it.key();
			if (columns.removeAll(t.key)) columns.prepend(t.key);
			for (const QString &c : std::as_const(columns))
				if (row.contains(c)) r.setAttribute(c, row.value(c));
			e.appendChild(r);
		}
		block.appendChild(e);
	}
	return block;
}

QList<FunctionTemplate::Parameter> FunctionTemplate::parametersFor(Diagram *folio,
																   const QList<QGraphicsItem *> &items)
{
	QList<Parameter> list;
	if (!folio) return list;
	const QDomDocument document = selectionXml(folio, items);
	for (const QString &name : placeholdersIn(document.documentElement())) {
		Parameter p;
		p.name = name;
		const QString upper = name.toUpper();
		if (upper.startsWith(QLatin1String("BMK"))) {
			p.kind = QStringLiteral("label");
			p.label = tr("Device tag");
			p.defaultValue = QStringLiteral("?1");
		} else if (upper.startsWith(QLatin1String("ADR"))) {
			p.label = tr("PLC address");
			p.defaultValue = QStringLiteral("%I0.0");
		} else if (upper.startsWith(QLatin1String("P_"))) {
			p.label = tr("Power");
			p.unit = name.mid(2);
		} else {
			p.label = name;
		}
		list << p;
	}
	return list;
}

QString FunctionTemplate::save(Diagram *folio, const QList<QGraphicsItem *> &items,
							   const SaveOptions &options, QString *error)
{
	if (!folio || !folio->project()) {
		if (error) *error = tr("There is no folio to save from.");
		return QString();
	}
	if (options.name.trimmed().isEmpty()) {
		if (error) *error = tr("The template needs a name.");
		return QString();
	}
	folio->project()->buildFolioToShow(folio);
	QDomDocument content = selectionXml(folio, items);
	if (content.documentElement().firstChildElement(QStringLiteral("elements")).isNull()) {
		if (error) *error = tr("The selection holds no symbol.");
		return QString();
	}

	QDomDocument macro;
	QDomElement root = macro.createElement(QStringLiteral("qet_macro"));
	macro.appendChild(root);
	root.appendChild(blockXml(macro, options));
	QDomElement collection = macro.createElement(QStringLiteral("collection"));
	root.appendChild(collection);

		//Each symbol carries its definition, so that the template places
		//on a computer which has not got the collection it came from
	QSet<QString> done;
	QDomNodeList nodes = content.elementsByTagName(QStringLiteral("element"));
	for (int i = 0 ; i < nodes.count() ; ++i) {
		QDomElement node = nodes.at(i).toElement();
		const QString type = node.attribute(QStringLiteral("type"));
		if (type.isEmpty()) continue;
		ElementsLocation location(type, folio->project());
		const QString path = location.collectionPath(false);
		node.setAttribute(QStringLiteral("type"), QStringLiteral("macro://") + path);
		if (done.contains(path)) continue;
		done.insert(path);
		const QDomElement definition = location.xml();
		if (definition.isNull()) continue;
		QDomElement entry = macro.createElement(QStringLiteral("element"));
		entry.setAttribute(QStringLiteral("path"), path);
		entry.appendChild(macro.importNode(definition, true));
		collection.appendChild(entry);
	}
	QDomElement container = macro.createElement(QStringLiteral("diagram_content"));
	root.appendChild(container);
	container.appendChild(macro.importNode(content.documentElement(), true));

	QString path = options.path;
	if (path.isEmpty()) {
		QDir dir(userDirectory());
		if (!options.category.trimmed().isEmpty())
			dir.setPath(dir.filePath(fileNameFor(options.category)));
		path = dir.filePath(fileNameFor(options.name) + QStringLiteral(".qetmak"));
	}
	QDir().mkpath(QFileInfo(path).absolutePath());
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		if (error) *error = tr("The file %1 cannot be written.").arg(QDir::toNativeSeparators(path));
		return QString();
	}
	QTextStream out(&file);
	out.setEncoding(QStringConverter::Utf8);
	out << macro.toString(4);
	return QFileInfo(path).absoluteFilePath();
}

bool FunctionTemplate::addressPlus(const QString &address, int offset, QString *result)
{
		//%I0.0, %Q2.7, E1.3, I 0.0: a byte and a bit of eight
	static const QRegularExpression pattern(
				QStringLiteral("^(\\s*%?\\s*[A-Za-z]{1,2}\\s*)(\\d+)\\.([0-7])(\\s*)$"));
	const QRegularExpressionMatch match = pattern.match(address);
	if (!match.hasMatch()) return false;
	const long long bit = match.captured(2).toLongLong() * 8 + match.captured(3).toInt() + offset;
	if (bit < 0) return false;
	if (result)
		*result = match.captured(1) + QString::number(bit / 8) + QLatin1Char('.')
				+ QString::number(bit % 8) + match.captured(4);
	return true;
}

QString FunctionTemplate::addressLetter(const QString &io)
{
	const QString kind = io.trimmed().toUpper();
	return kind == QLatin1String("DQ") || kind == QLatin1String("AQ") || kind == QLatin1String("DO")
			|| kind == QLatin1String("AO") ? QStringLiteral("Q") : QStringLiteral("I");
}

QString FunctionTemplate::nextFreeAddress(QETProject *project, const QString &io, int count)
{
	if (!project) return QString();
	const QString kind = io.trimmed().toUpper();
	const bool analog = kind.startsWith(QLatin1Char('A'));
	const bool output = addressLetter(kind) == QLatin1String("Q");
	QStringList offered;	// in the order of the modules
	QSet<QString> taken;
	for (Diagram *d : project->diagrams()) {
		project->buildFolioToShow(d);
		for (QGraphicsItem *item : d->items()) {
			auto *element = qgraphicsitem_cast<Element *>(item);
			if (!element) continue;
			const ElementData data = element->elementData();
			if (data.m_type == ElementData::Master && data.m_master_type == ElementData::PLC) {
				const auto ios = data.plcMasterData().ios;
				QSet<int> used;
				for (Element *other : element->linkedElements()) used.insert(element->groupIndexForElement(other));
				for (int i = 0 ; i < ios.size() ; ++i) {
					const auto t = ios.at(i).type;
					const bool is_output = t == ElementData::SortieDigitale || t == ElementData::SortieAnalogique
							|| t == ElementData::SortieUniverselle;
					const bool is_analog = t == ElementData::EntreeAnalogique || t == ElementData::SortieAnalogique;
					if (is_output != output || (!kind.isEmpty() && is_analog != analog)) continue;
					QString address = plainAddress(ios.at(i).address);
					if (!address.isEmpty() && address.at(0).isDigit())
						address.prepend(addressLetter(kind));
					offered << address;
					if (used.contains(i)) taken.insert(address);
				}
			} else if (element->linkType() == Element::Slave) {
				const QString address = element->elementInformations()
						.value(QStringLiteral("plc_address")).toString();
				if (!address.trimmed().isEmpty()) taken.insert(plainAddress(address));
			}
		}
	}
	for (const QString &first : std::as_const(offered)) {
		bool free = !taken.contains(first);
		for (int k = 1 ; k < count && free ; ++k) {
			QString next;
			free = addressPlus(first, k, &next) && offered.contains(next) && !taken.contains(next);
		}
		if (free) return QLatin1Char('%') + first;
	}
	return QString();
}

QRectF FunctionTemplate::extent(const QMap<QString, QString> &values) const
{
	if (!m_valid) return QRectF();
	if (!m_has_drawing) {
		const QMap<QString, QString> all = resolve(values, nullptr);
		QMap<QString, QString> resolved;
		for (auto it = all.cbegin() ; it != all.cend() ; ++it)
			if (!it.value().trimmed().isEmpty()) resolved.insert(it.key(), it.value());
		const FunctionTemplate target = chosen(resolved);
		return target.isValid() ? target.extent(resolved) : QRectF();
	}
	QScopedPointer<QETProject> project(new QETProject());
	embedCollection(project.data(), m_document.documentElement());
	Diagram *folio = project->addNewDiagram();
	if (!folio) return QRectF();
	QDomDocument document = instantiate(resolve(values, nullptr), nullptr);
	QList<CablePart *> cables;
	folio->fromXml(document, QPointF(), false, nullptr, &cables, nullptr);
	folio->refreshContents();
	QRectF rect;
	for (QGraphicsItem *item : folio->items()) {
		if (item->parentItem() || !item->isVisible()) continue;
		if (!qgraphicsitem_cast<Element *>(item) && !(item->flags() & QGraphicsItem::ItemIsSelectable)) continue;
		rect = rect.united(item->sceneBoundingRect());
	}
	return rect;
}
