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
#include "configgenerator.h"

#include "functiontemplate.h"
#include "productrules.h"
#include "templateexpression.h"

#include "../ElementsCollection/elementslocation.h"
#include "../ElementsCollection/xmlelementcollection.h"
#include "../NameList/nameslist.h"
#include "../container/qetzip.h"
#include "../dataBase/projectdatabase.h"
#include "../diagram.h"
#include "../TerminalStrip/terminalstrip.h"
#include "../cable/cable.h"
#include "../qetgraphicsitem/conductor.h"
#include "../factory/elementfactory.h"
#include "../genericdevice/genericdevice.h"
#include "../properties/elementdata.h"
#include "../qetapp.h"
#include "../qetgraphicsitem/ViewItem/projectdbmodel.h"
#include "../qetgraphicsitem/ViewItem/qetgraphicstableitem.h"
#include "../qetgraphicsitem/element.h"
#include "../qetproject.h"
#include "../qetresult.h"
#include "../qetversion.h"
#include "../revision/projectrevisions.h"
#include "../titleblockproperties.h"
#include "../utils/qetutils.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>

#include <algorithm>
#include <cmath>

namespace
{
	const QString &blockTag()
	{
		static const QString tag = QStringLiteral("configurator");
		return tag;
	}

		/// Where the generic PLC modules go in the embedded collection
	const QString &moduleDir()
	{
		static const QString dir = QStringLiteral("import/Konfigurator");
		return dir;
	}

	constexpr int ChannelsPerModule = 16;
		/// The channel table beside a generic module: address, function,
		/// cross reference, one row of 20 px beside each channel
	constexpr int TableRow = 20;
	constexpr int TableAddress = 45;
	constexpr int TableFunction = 130;
	constexpr int TableCrossRef = 85;
	constexpr int TableGap = 20;
	constexpr int TableWidth = TableAddress + TableFunction + TableCrossRef;

		/// The table of a generic module made readable: rows of a channel
		/// pitch in line with the channels, the columns a PLC overview needs
	void tableBesideChannels(QDomElement definition)
	{
		QDomDocument document = definition.ownerDocument();
		QDomElement plc = definition.firstChildElement(QStringLiteral("kindInformations"))
				.firstChildElement(QStringLiteral("plcMasterData"));
		if (plc.isNull()) return;
		plc.setAttribute(QStringLiteral("rowHeight"), QString::number(TableRow, 'f', 2));
		QDomElement widths = document.createElement(QStringLiteral("columnWidths"));
		const QList<QPair<int, int>> width_list{{1, TableAddress}, {2, TableFunction}, {4, TableCrossRef}};
		for (const auto &w : width_list) {
			QDomElement c = document.createElement(QStringLiteral("column"));
			c.setAttribute(QStringLiteral("index"), w.first);
			c.setAttribute(QStringLiteral("width"), w.second);
			widths.appendChild(c);
		}
		plc.insertBefore(widths, plc.firstChild());
		QDomElement shown = document.createElement(QStringLiteral("columnVisibility"));
		for (int i = 0 ; i < 5 ; ++i) {
			QDomElement c = document.createElement(QStringLiteral("column"));
			c.setAttribute(QStringLiteral("index"), i);
			c.setAttribute(QStringLiteral("visible"), i == 0 || i == 3 ? QStringLiteral("false") : QStringLiteral("true"));
			shown.appendChild(c);
		}
		plc.insertBefore(shown, plc.firstChild());

		QDomElement description = definition.firstChildElement(QStringLiteral("description"));
		qreal first = 1e9;
		for (QDomElement t = description.firstChildElement(QStringLiteral("terminal")) ; !t.isNull()
			 ; t = t.nextSiblingElement(QStringLiteral("terminal")))
			first = qMin(first, t.attribute(QStringLiteral("y")).toDouble());
		QDomElement table = description.firstChildElement(QStringLiteral("plc_table"));
		if (!table.isNull() && first < 1e9)
				//The header is a row and 2 px high, then row i beside channel i
			table.setAttribute(QStringLiteral("y"), first - TableRow / 2 - (TableRow + 2));
	}

	struct ManifestObject
	{
		QString name;
		QString hash;
		QString templ;		///< category/name of the template drawn
		QString variant;
		QString folio;		///< uuid
		QMap<QString, QString> stable;	///< device tags and addresses it got
	};

	struct ManifestFolio
	{
		QString uuid;
		QString kind;		///< plan, kept, plc, list
		QStringList objects;
		QByteArray fingerprint;
	};

	struct Manifest
	{
		bool present = false;
		QString source;
		QMap<QString, ManifestObject> objects;
		QList<ManifestFolio> folios;
	};

	bool isBlock(const QString &block)
	{
		const QString b = block.trimmed();
		return b.startsWith(QLatin1Char('<') + blockTag() + QLatin1Char(' '))
				|| b.startsWith(QLatin1Char('<') + blockTag() + QLatin1Char('>'))
				|| b.startsWith(QLatin1Char('<') + blockTag() + QLatin1Char('/'));
	}

	Manifest loadManifest(QETProject *project)
	{
		Manifest m;
		for (const QString &block : project->dataBase()->foreignBlocks(
				 QString::fromLatin1(projectDataBase::project_owner))) {
			if (!isBlock(block)) continue;
			QDomDocument doc;
			if (!doc.setContent(block)) continue;
			const QDomElement root = doc.documentElement();
			m.present = true;
			m.source = root.attribute(QStringLiteral("source"));
			for (QDomElement o = root.firstChildElement(QStringLiteral("object")) ; !o.isNull()
				 ; o = o.nextSiblingElement(QStringLiteral("object"))) {
				ManifestObject object;
				object.name = o.attribute(QStringLiteral("name"));
				object.hash = o.attribute(QStringLiteral("hash"));
				object.templ = o.attribute(QStringLiteral("template"));
				object.variant = o.attribute(QStringLiteral("variant"));
				object.folio = o.attribute(QStringLiteral("folio"));
				for (QDomElement v = o.firstChildElement(QStringLiteral("value")) ; !v.isNull()
					 ; v = v.nextSiblingElement(QStringLiteral("value")))
					object.stable.insert(v.attribute(QStringLiteral("name")), v.attribute(QStringLiteral("value")));
				m.objects.insert(object.name, object);
			}
			for (QDomElement f = root.firstChildElement(QStringLiteral("folio")) ; !f.isNull()
				 ; f = f.nextSiblingElement(QStringLiteral("folio"))) {
				ManifestFolio folio;
				folio.uuid = f.attribute(QStringLiteral("uuid"));
				folio.kind = f.attribute(QStringLiteral("kind"));
				folio.fingerprint = f.firstChildElement(QStringLiteral("fingerprint")).text().toLatin1();
				for (QDomElement o = f.firstChildElement(QStringLiteral("object")) ; !o.isNull()
					 ; o = o.nextSiblingElement(QStringLiteral("object")))
					folio.objects << o.attribute(QStringLiteral("name"));
				m.folios << folio;
			}
			break;
		}
		return m;
	}

	void saveManifest(QETProject *project, const Manifest &m)
	{
		QDomDocument doc;
		QDomElement root = doc.createElement(blockTag());
		root.setAttribute(QStringLiteral("version"), 1);
		root.setAttribute(QStringLiteral("source"), m.source);
		root.setAttribute(QStringLiteral("date"),
						  QDateTime::currentDateTime().toString(Qt::ISODate));
		for (const ManifestObject &o : m.objects) {
			QDomElement e = doc.createElement(QStringLiteral("object"));
			e.setAttribute(QStringLiteral("name"), o.name);
			e.setAttribute(QStringLiteral("hash"), o.hash);
			e.setAttribute(QStringLiteral("template"), o.templ);
			if (!o.variant.isEmpty()) e.setAttribute(QStringLiteral("variant"), o.variant);
			e.setAttribute(QStringLiteral("folio"), o.folio);
			for (auto it = o.stable.cbegin() ; it != o.stable.cend() ; ++it) {
				QDomElement v = doc.createElement(QStringLiteral("value"));
				v.setAttribute(QStringLiteral("name"), it.key());
				v.setAttribute(QStringLiteral("value"), it.value());
				e.appendChild(v);
			}
			root.appendChild(e);
		}
		for (const ManifestFolio &f : m.folios) {
			QDomElement e = doc.createElement(QStringLiteral("folio"));
			e.setAttribute(QStringLiteral("uuid"), f.uuid);
			e.setAttribute(QStringLiteral("kind"), f.kind);
			for (const QString &name : f.objects) {
				QDomElement o = doc.createElement(QStringLiteral("object"));
				o.setAttribute(QStringLiteral("name"), name);
				e.appendChild(o);
			}
			if (!f.fingerprint.isEmpty()) {
				QDomElement p = doc.createElement(QStringLiteral("fingerprint"));
				p.appendChild(doc.createTextNode(QString::fromLatin1(f.fingerprint)));
				e.appendChild(p);
			}
			root.appendChild(e);
		}
		doc.appendChild(root);
		QString text;
		QTextStream stream(&text);
		root.save(stream, -1);
		stream.flush();

		const QString owner = QString::fromLatin1(projectDataBase::project_owner);
		QStringList blocks = project->dataBase()->foreignBlocks(owner);
		blocks.erase(std::remove_if(blocks.begin(), blocks.end(), isBlock), blocks.end());
		blocks << text;
		project->dataBase()->storeForeignBlocks(owner, blocks);
		project->setModified(true);
	}

	QetRevision::Fingerprint fingerprintOf(QETProject *project, Diagram *folio)
	{
		project->buildFolioToShow(folio);
		const QDomDocument document = folio->toXml();
		return QetRevision::fingerprint(document.documentElement(), project->folioIndex(folio), folio);
	}

	QString cellOf(const ConfigGenerator::Row &row, const QString &name)
	{
		for (auto it = row.cells.cbegin() ; it != row.cells.cend() ; ++it)
			if (it.key().trimmed().compare(name, Qt::CaseInsensitive) == 0) return it.value().trimmed();
		return QString();
	}

		/// "%Q1.3": its letter and its bit, counted from %Q0.0
	bool addressBit(const QString &address, QString *letter, int *bit)
	{
		static const QRegularExpression pattern(
					QStringLiteral("^\\s*%?\\s*([A-Za-z])\\s*(\\d+)\\.([0-7])\\s*$"));
		const QRegularExpressionMatch match = pattern.match(address);
		if (!match.hasMatch()) return false;
		QString l = match.captured(1).toUpper();
		if (l == QLatin1String("E")) l = QStringLiteral("I");
		if (l == QLatin1String("A")) l = QStringLiteral("Q");
		*letter = l;
		*bit = match.captured(2).toInt() * 8 + match.captured(3).toInt();
		return true;
	}

	QString addressOf(const QString &letter, int bit)
	{
		return QStringLiteral("%%1%2.%3").arg(letter).arg(bit / 8).arg(bit % 8);
	}

		/// The kinds of parameter the generator counts on and keeps from
		/// one run to the next: device tag, PLC channel, terminal, cable
	bool counted(const QString &kind)
	{
		return kind == QLatin1String("label") || kind == QLatin1String("address")
				|| kind == QLatin1String("terminal") || kind == QLatin1String("cable");
	}

	QString templateId(const FunctionTemplate &tpl)
	{
		return tpl.category().isEmpty() ? tpl.name() : tpl.category() + QLatin1Char('/') + tpl.name();
	}

	QString shortHash(const QString &text)
	{
		return QString::fromLatin1(QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha1)
								   .toHex().left(16));
	}

		/// One object of the table, planned
	struct Plan
	{
		ConfigGenerator::Row row;
		QString object;
		FunctionTemplate typical;	///< what the table names
		FunctionTemplate drawn;		///< what its variants choose
		QString variant;
		QMap<QString, QString> given;
		QString hash;
		QString group;
		QString plant;
		QString location;
		QRectF extent;
		bool stable_template = false;	///< drawn with the same template as last time
	};

	struct Unit
	{
		QString group;
		QList<int> plans;
		qreal width = 0;
		Diagram *folio = nullptr;	///< kept or drawn
		Diagram *replaces = nullptr;
		Diagram *beside = nullptr;	///< the folio changed by hand it does not replace
		bool keep = false;
		QStringList objects(const QList<Plan> &all) const
		{
			QStringList list;
			for (int i : plans) list << all.at(i).object;
			return list;
		}
	};

	QStringList sorted(QStringList list)
	{
		std::sort(list.begin(), list.end());
		return list;
	}

		/// Before a folio goes: its symbols out of their links and terminal
		/// strips, its cable sections and cores out of their cables -- what
		/// the project keeps beside the folios would point at nothing
	void release(QETProject *project, Diagram *folio)
	{
		QSet<QUuid> conductors;
		QSet<TerminalStrip *> filled;
		for (TerminalStrip *strip : project->terminalStrip())
			if (strip->physicalTerminalCount() > 0) filled.insert(strip);
			//Gathered first: unlinking deletes the cross references, items
			//of the folio too
		QList<Element *> elements;
		for (QGraphicsItem *item : folio->items()) {
			if (auto *element = qgraphicsitem_cast<Element *>(item)) elements << element;
			else if (auto *conductor = qgraphicsitem_cast<Conductor *>(item)) conductors.insert(conductor->uuid());
		}
		QSet<TerminalStrip *> changed;
		for (Element *element : std::as_const(elements)) {
			if (!element->linkedElements().isEmpty()) element->unlinkAllElements();
			for (TerminalStrip *strip : project->terminalStrip())
				if (strip->removeTerminal(element)) changed.insert(strip);
		}
			//Removing a terminal does not tell the database, as an edit of
			//the strip does: the lists read the strips from it
		for (TerminalStrip *strip : std::as_const(changed)) project->dataBase()->storeTerminalStrip(strip);
		const QUuid folio_uuid = folio->uuid();
		for (Cable *cable : project->cables()) {
			QSet<QUuid> parts;
			for (const CablePartData &part : cable->parts())
				if (part.diagram == folio_uuid) parts.insert(part.uuid);
			for (const CableCore &core : cable->usedCores())
				if (parts.contains(core.part) || conductors.contains(core.conductor)) cable->removeCore(core.core);
			for (const QUuid &part : std::as_const(parts)) cable->removePart(part);
			if (cable->parts().isEmpty()) project->removeCable(cable);
		}
			//A strip emptied by this, not one the user keeps empty
		for (TerminalStrip *strip : project->terminalStrip())
			if (filled.contains(strip) && strip->physicalTerminalCount() == 0) project->removeTerminalStrip(strip);
	}

	QString labelOf(Element *element)
	{
		return element->elementInformations().value(QStringLiteral("label")).toString().trimmed();
	}

	void setTitle(Diagram *folio, const QString &title, const QString &plant, const QString &location)
	{
		TitleBlockProperties properties = folio->border_and_titleblock.exportTitleBlock();
		properties.title = title;
		if (!plant.isNull()) properties.plant = plant;
		if (!location.isNull()) properties.locmach = location;
		folio->border_and_titleblock.importTitleBlock(properties);
	}

		/// A list of the project database as tables on folios of their
		/// own, as many as its rows need. @return its rows
	int addList(QETProject *project, const QString &title, const QString &identifier,
				const QString &query, QList<Diagram *> *folios)
	{
		Diagram *folio = project->addNewDiagram();
		if (!folio) return 0;
		setTitle(folio, title, QString(), QString());
		*folios << folio;
		auto *model = new ProjectDBModel(project, project);
		model->setIdentifier(identifier);
		model->setQuery(query);
			//What the table dialog always sets: a table saved without an
			//alignment reads it back as -1 and draws no text. Saved by its
			//enum name, so one flag, not a combination
		const int align = int(Qt::AlignLeft);
		const QString margins = QETUtils::marginsToString(QMargins(5, 2, 5, 2));
		model->setData(model->index(0, 0), align, Qt::TextAlignmentRole);
		model->setData(model->index(0, 0), QETApp::diagramTextsFont(9), Qt::FontRole);
		model->setData(model->index(0, 0), margins, Qt::UserRole + 1);
		model->setHeaderData(0, Qt::Horizontal, align, Qt::TextAlignmentRole);
		QFont header_font = QETApp::diagramTextsFont(9);
		header_font.setBold(true);
		model->setHeaderData(0, Qt::Horizontal, header_font, Qt::FontRole);
		model->setHeaderData(0, Qt::Horizontal, margins, Qt::UserRole + 1);
		auto *table = new QetGraphicsTableItem();
		table->setTableName(title);
		table->setModel(model);
		folio->addItem(table);
		table->setPos(50, 50);
		QetGraphicsTableItem::adjustTableToFolio(table);
		int shown = table->displayNRow();
		int number = 2;
		QetGraphicsTableItem *previous = table;
		while (shown > 0 && model->rowCount() > shown && number < 100) {
			Diagram *next = project->addNewDiagram();
			if (!next) break;
			setTitle(next, QStringLiteral("%1 %2").arg(title).arg(number), QString(), QString());
			*folios << next;
			auto *more = new QetGraphicsTableItem();
			more->setPreviousTable(previous);
			more->setTableName(title);
			next->addItem(more);
			more->setPos(50, 50);
			QetGraphicsTableItem::adjustTableToFolio(more);
			if (more->displayNRow() <= 0) break;
			shown += more->displayNRow();
			previous = more;
			++number;
		}
		return model->rowCount();
	}
}

QStringList ConfigGenerator::metaColumns()
{
	return {QStringLiteral("objekt"), QStringLiteral("typical"), QStringLiteral("anlage"),
			QStringLiteral("ort"), QStringLiteral("blatt"), QStringLiteral("produkt")};
}

ConfigGenerator::Table ConfigGenerator::readTable(const QString &path)
{
	Table table;
	QFile probe(path);
	if (!probe.open(QIODevice::ReadOnly)) {
		table.error = tr("The table %1 cannot be read.").arg(QDir::toNativeSeparators(path));
		return table;
	}
	const QByteArray head = probe.read(2);
	probe.close();

	if (head == "PK") {
			//An Excel workbook: its first sheet
		QList<QetZip::Entry> entries;
		QString error;
		if (!QetZip::read(path, &entries, &error)) {
			table.error = tr("The workbook %1 cannot be read: %2").arg(QDir::toNativeSeparators(path), error);
			return table;
		}
		QHash<QString, QByteArray> files;
		for (const auto &e : entries) files.insert(e.name, e.data);
		const auto parsed = [&](const QString &name) {
			QDomDocument doc;
			doc.setContent(files.value(name));
			return doc;
		};
		QStringList shared;
		{
			const QDomNodeList sis = parsed(QStringLiteral("xl/sharedStrings.xml")).elementsByTagName(QStringLiteral("si"));
			for (int i = 0 ; i < sis.count() ; ++i) {
				QString text;
				const QDomNodeList ts = sis.at(i).toElement().elementsByTagName(QStringLiteral("t"));
				for (int k = 0 ; k < ts.count() ; ++k) text += ts.at(k).toElement().text();
				shared << text;
			}
		}
		QHash<QString, QString> targets;
		{
			const QDomNodeList rs = parsed(QStringLiteral("xl/_rels/workbook.xml.rels"))
					.elementsByTagName(QStringLiteral("Relationship"));
			for (int i = 0 ; i < rs.count() ; ++i) {
				const QDomElement r = rs.at(i).toElement();
				QString target = r.attribute(QStringLiteral("Target"));
				if (target.startsWith(QLatin1Char('/'))) target.remove(0, 1);
				else target.prepend(QStringLiteral("xl/"));
				targets.insert(r.attribute(QStringLiteral("Id")), target);
			}
		}
		QString part;
		{
			const QDomNodeList sheets = parsed(QStringLiteral("xl/workbook.xml")).elementsByTagName(QStringLiteral("sheet"));
			if (sheets.count()) {
				const QDomElement s = sheets.at(0).toElement();
				QString id = s.attribute(QStringLiteral("r:id"));
				if (id.isEmpty())
					id = s.attributeNS(QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"),
									   QStringLiteral("id"));
				part = targets.value(id);
			}
		}
		if (part.isEmpty() || !files.contains(part)) {
			table.error = tr("The workbook %1 has no sheet.").arg(QDir::toNativeSeparators(path));
			return table;
		}
		const QDomNodeList rows = parsed(part).elementsByTagName(QStringLiteral("row"));
		QMap<int, QString> header;
		for (int r = 0 ; r < rows.count() ; ++r) {
			const QDomElement row = rows.at(r).toElement();
			QMap<int, QString> cells;
			const QDomNodeList cs = row.elementsByTagName(QStringLiteral("c"));
			int next = 0;
			for (int c = 0 ; c < cs.count() ; ++c) {
				const QDomElement cell = cs.at(c).toElement();
				const QString ref = cell.attribute(QStringLiteral("r"));
				int col = 0;
				if (ref.isEmpty()) {
					col = next;
				} else {
					for (const QChar ch : ref) {
						if (!ch.isLetter()) break;
						col = col * 26 + (ch.toUpper().unicode() - 'A' + 1);
					}
					--col;
				}
				next = col + 1;
				const QString type = cell.attribute(QStringLiteral("t"));
				QString text;
				if (type == QLatin1String("inlineStr")) {
					const QDomNodeList ts = cell.elementsByTagName(QStringLiteral("t"));
					for (int i = 0 ; i < ts.count() ; ++i) text += ts.at(i).toElement().text();
				} else {
					const QString v = cell.firstChildElement(QStringLiteral("v")).text();
					text = type == QLatin1String("s") ? shared.value(v.toInt()) : v;
				}
				cells.insert(col, text.trimmed());
			}
			bool empty = true;
			for (const QString &t : std::as_const(cells)) empty &= t.isEmpty();
			if (empty) continue;
			if (cells.first().startsWith(QLatin1Char('#'))) continue;
			if (header.isEmpty()) {
				for (auto it = cells.cbegin() ; it != cells.cend() ; ++it)
					if (!it.value().isEmpty()) header.insert(it.key(), it.value());
				table.columns = header.values();
				continue;
			}
			Row line;
			line.line = row.attribute(QStringLiteral("r"), QString::number(r + 1)).toInt();
			for (auto it = header.cbegin() ; it != header.cend() ; ++it)
				line.cells.insert(it.value(), cells.value(it.key()));
			table.rows << line;
		}
	} else {
		QString error;
		const auto rows = FunctionTemplate::readCsv(path, &table.columns, &error);
		if (!error.isEmpty()) {
			table.error = error;
			return table;
		}
		int line = 1;
		for (const auto &cells : rows) {
			Row row;
			row.line = ++line;
			row.cells = cells;
			table.rows << row;
		}
	}
	if (table.columns.isEmpty()) table.error = tr("The table %1 is empty.").arg(QDir::toNativeSeparators(path));
	return table;
}

ConfigGenerator::Result ConfigGenerator::generate(const QString &table_path, const QString &target)
{
	return generate(table_path, target, Options());
}

ConfigGenerator::Result ConfigGenerator::generate(const QString &table_path, const QString &target,
												  const Options &options)
{
	Result result;
	result.target = QFileInfo(target).absoluteFilePath();
	const Table table = readTable(table_path);
	if (!table.error.isEmpty()) {
		result.error = table.error;
		return result;
	}
	const bool exists = QFileInfo::exists(target);
	QScopedPointer<QETProject> project(exists ? new QETProject(target) : new QETProject());
	if (exists && project->state() != QETProject::Ok) {
		result.error = tr("The project %1 cannot be opened.").arg(QDir::toNativeSeparators(target));
		return result;
	}
	if (!exists) project->setTitle(QFileInfo(target).completeBaseName());
	result = generate(table, project.data(), QFileInfo(table_path).fileName(), options);
	result.target = QFileInfo(target).absoluteFilePath();
	if (!result.ok) return result;
	if (exists) {
			//What was there before, beside it
		const QFileInfo info(target);
		const QString backup = info.absoluteDir().filePath(info.completeBaseName()
														   + QStringLiteral(".vor-konfigurator.") + info.suffix());
		QFile::remove(backup);
		QFile::copy(target, backup);
		result.report << tr("The project as it was: %1").arg(QDir::toNativeSeparators(backup));
	}
	project->setFilePath(result.target);
	const QETResult written = project->write();
	if (!written.isOk()) {
		result.ok = false;
		result.error = tr("The project cannot be written: %1").arg(written.errorMessage());
	}
	return result;
}

ConfigGenerator::Result ConfigGenerator::generate(const Table &table, QETProject *project,
												  const QString &source, const Options &options)
{
	Result r;
	if (!project) {
		r.error = tr("There is no project to generate into.");
		return r;
	}
	if (project->isReadOnly()) {
		r.error = tr("The project is read-only.");
		return r;
	}
	const Manifest old = loadManifest(project);
	if (!old.present && !project->diagrams().isEmpty())
		r.report << tr("The project was not made by the configurator: its %1 folios stay as they are, "
					   "the generated ones come after them.").arg(project->diagrams().size());

		//1. Each row: its template, the variant its values choose, the
		//   values themselves
	const QStringList meta = metaColumns();
	QList<Plan> plans;
	QSet<QString> names;
	QHash<QString, QRectF> extents;
	const auto known = [](const FunctionTemplate &tpl) {
		QStringList list;
		for (const auto &p : tpl.parameters()) list << p.name;
		for (const auto &t : tpl.tables()) list << t.columns;
		for (const auto &f : tpl.formulas()) list << f.name;
		if (tpl.hasDrawing()) list << FunctionTemplate::placeholdersIn(tpl.document().documentElement());
		return list;
	};
	const auto label_parameter = [](const FunctionTemplate &tpl) {
		for (const auto &p : tpl.parameters())
			if (p.kind == QLatin1String("label")) return p.name;
		return QString();
	};
	for (const Row &row : table.rows) {
		const QString object = cellOf(row, QStringLiteral("objekt"));
		const QString typical_cell = cellOf(row, QStringLiteral("typical"));
		const QString product = cellOf(row, QStringLiteral("produkt"));
		if (object.isEmpty() && typical_cell.isEmpty() && product.isEmpty()) continue;
		if (object.isEmpty()) {
			r.report << tr("Row %1: no object name, left out.").arg(row.line);
			continue;
		}
		if (names.contains(object.toLower())) {
			r.report << tr("Row %1: the object %2 is there twice, left out.").arg(row.line).arg(object);
			continue;
		}
		Plan plan;
		plan.row = row;
		plan.object = object;
			//A product number brings its template, variant and values; the
			//columns of the row go before them
		ProductRules::Decoded decoded;
		if (!product.isEmpty()) {
			decoded = ProductRules::decodeNumber(product);
			if (!decoded.ok) {
				r.report << tr("Row %1 (%2): %3").arg(row.line).arg(object, decoded.error);
				continue;
			}
		}
		const QString typical = typical_cell.isEmpty() ? decoded.templ : typical_cell;
		if (!typical_cell.isEmpty() && !decoded.templ.isEmpty()
				&& FunctionTemplate::find(typical_cell).path() != FunctionTemplate::find(decoded.templ).path())
			r.report << tr("Row %1 (%2): the column Typical (%3) goes before the template of the product number (%4).")
						.arg(row.line).arg(object, typical_cell, decoded.templ);
		plan.typical = FunctionTemplate::find(typical);
		if (!plan.typical.isValid()) {
			r.report << tr("Row %1 (%2): there is no function template %3, left out.")
						.arg(row.line).arg(object, typical);
			continue;
		}
		plan.plant = cellOf(row, QStringLiteral("anlage"));
		plan.location = cellOf(row, QStringLiteral("ort"));
		while (plan.plant.startsWith(QLatin1Char('='))) plan.plant.remove(0, 1);
		while (plan.location.startsWith(QLatin1Char('+'))) plan.location.remove(0, 1);

			//The columns are the placeholders, whatever their case
		const auto map_values = [&](const FunctionTemplate &a, const FunctionTemplate &b, QStringList *unknown) {
			const QStringList placeholders = known(a) + known(b);
			QMap<QString, QString> values;
			for (auto it = row.cells.cbegin() ; it != row.cells.cend() ; ++it) {
				const QString column = it.key().trimmed();
				if (column.isEmpty() || meta.contains(column.toLower()) || it.value().trimmed().isEmpty()) continue;
				QString name;
				for (const QString &n : placeholders)
					if (n.compare(column, Qt::CaseInsensitive) == 0) { name = n; break; }
				if (name.isEmpty() && column.compare(QLatin1String("BMK"), Qt::CaseInsensitive) == 0) {
					name = label_parameter(b);
					if (name.isEmpty()) name = label_parameter(a);
				}
				if (name.isEmpty()) {
					name = column;
					if (unknown) *unknown << column;
				}
				values.insert(name, it.value().trimmed());
			}
			return values;
		};

		QStringList notes;
		QString chain, error;
		const auto with_product = [&decoded](QMap<QString, QString> values) {
			for (auto it = decoded.values.cbegin() ; it != decoded.values.cend() ; ++it)
				if (!values.contains(it.key())) values.insert(it.key(), it.value());
			return values;
		};
		plan.given = with_product(map_values(plan.typical, plan.typical, nullptr));
		plan.drawn = plan.typical.chosen(plan.typical.resolve(plan.given, nullptr), &chain, nullptr, &error);
		if (!plan.drawn.isValid()) {
			r.report << tr("Row %1 (%2): %3").arg(row.line).arg(object, error);
			continue;
		}
		QStringList unknown;
		plan.given = with_product(map_values(plan.typical, plan.drawn, &unknown));
			//The object names the function, when nothing else does
		bool text_formula = false;
		for (const auto &f : plan.typical.formulas()) text_formula |= f.name == QLatin1String("TEXT");
		for (const auto &f : plan.drawn.formulas()) text_formula |= f.name == QLatin1String("TEXT");
		for (const auto &p : plan.drawn.parameters())
			if (p.name == QLatin1String("TEXT") && !plan.given.contains(p.name) && !text_formula)
				plan.given.insert(p.name, object);
		plan.drawn = plan.typical.chosen(plan.typical.resolve(plan.given, nullptr, &notes), &chain, &notes, &error);
		if (!plan.drawn.isValid()) {
			r.report << tr("Row %1 (%2): %3").arg(row.line).arg(object, error);
			continue;
		}
		names.insert(object.toLower());
		plan.variant = chain;
		for (const QString &u : std::as_const(unknown))
			r.report << tr("Row %1 (%2): the column %3 is no value of %4, it is passed on as it is.")
						.arg(row.line).arg(object, u, templateId(plan.drawn));
		for (const QString &n : std::as_const(notes))
			r.report << tr("Row %1 (%2): %3").arg(row.line).arg(object, n);

		QStringList cells;
		for (auto it = row.cells.cbegin() ; it != row.cells.cend() ; ++it)
			if (!it.value().trimmed().isEmpty())
				cells << it.key().trimmed().toLower() + QLatin1Char('=') + it.value().trimmed();
		cells.sort();
		plan.hash = shortHash(cells.join(QLatin1Char('\n')));

		const QString sheet = cellOf(row, QStringLiteral("blatt"));
		plan.group = sheet.isEmpty()
				? QStringLiteral("auto|%1|%2|%3").arg(templateId(plan.drawn), plan.plant, plan.location)
				: QStringLiteral("blatt|%1").arg(sheet);
		const QString extent_key = plan.drawn.path();
		if (!extents.contains(extent_key)) extents.insert(extent_key, plan.drawn.extent(plan.given));
		plan.extent = extents.value(extent_key);
		plans << plan;
	}
	r.objects = plans.size();
	if (plans.isEmpty()) {
		r.error = tr("The table names no object to draw.");
		return r;
	}

		//2. Device tags and PLC channels: the ones of last time stay,
		//   the new ones are counted on
	QHash<QString, QSet<int>> used;	// letter -> bits
	for (Plan &plan : plans) {
		const auto it = old.objects.constFind(plan.object);
		plan.stable_template = it != old.objects.cend() && it->templ == templateId(plan.drawn);
		for (const auto &p : plan.drawn.parameters()) {
			if (!counted(p.kind)) continue;
			if (!plan.given.value(p.name).isEmpty()) {
				if (p.kind == QLatin1String("address")) {
					QString letter; int bit = 0;
					if (addressBit(plan.given.value(p.name), &letter, &bit))
						for (int k = 0 ; k < p.count ; ++k) used[letter].insert(bit + k);
				}
				continue;
			}
			if (!plan.stable_template) continue;
			const QString value = it->stable.value(p.name);
			if (value.isEmpty()) continue;
			if (p.kind == QLatin1String("address")) {
				if (it->stable.value(p.name + QStringLiteral("#count")).toInt() != p.count) continue;
				QString letter; int bit = 0;
				if (!addressBit(value, &letter, &bit)) continue;
				bool free = true;
				for (int k = 0 ; k < p.count ; ++k) free &= !used.value(letter).contains(bit + k);
				if (!free) continue;
				for (int k = 0 ; k < p.count ; ++k) used[letter].insert(bit + k);
			}
			plan.given.insert(p.name, value);
		}
	}
	for (Plan &plan : plans) {
		for (const auto &p : plan.drawn.parameters()) {
			if (p.kind != QLatin1String("address") || !plan.given.value(p.name).isEmpty()) continue;
			if (p.io.startsWith(QLatin1Char('A'))) {
				r.report << tr("%1: analog channels (%2) are not counted by the configurator yet; "
							   "give the address in the table.").arg(plan.object, p.io);
				continue;
			}
			const QString letter = FunctionTemplate::addressLetter(p.io);
			QSet<int> &bits = used[letter];
			int first = 0;
			for (;; ++first) {
				bool free = true;
				for (int k = 0 ; k < p.count && free ; ++k) free = !bits.contains(first + k);
				if (free) break;
			}
			for (int k = 0 ; k < p.count ; ++k) bits.insert(first + k);
			plan.given.insert(p.name, addressOf(letter, first));
		}
	}

		//3. The folios: an object with a big drawing has one of its own,
		//   small ones of the same kind and place share one
	const BorderProperties border = project->defaultBorderProperties();
	const qreal inner_width = border.columns_count * border.columns_width;
	const qreal gap = border.columns_width;
	QList<Unit> units;
	for (int i = 0 ; i < plans.size() ; ++i) {
		const Plan &plan = plans.at(i);
		const qreal width = plan.extent.width();
		if (!units.isEmpty() && units.last().group == plan.group
				&& units.last().width + gap + width <= inner_width - gap) {
			units.last().plans << i;
			units.last().width += gap + width;
			continue;
		}
		Unit unit;
		unit.group = plan.group;
		unit.plans << i;
		unit.width = width;
		units << unit;
	}

		//4. What became of the folios of last time
	QHash<QString, Diagram *> by_uuid;
	for (Diagram *d : project->diagrams()) by_uuid.insert(d->uuid().toString(), d);
	QHash<QString, QList<QetRevision::Change>> changes;	// folio uuid
	QHash<QString, QetRevision::Fingerprint> before;
	QHash<QString, QString> folio_kind;
	for (const ManifestFolio &f : old.folios) {
		Diagram *d = by_uuid.value(f.uuid);
		if (!d) continue;
		folio_kind.insert(f.uuid, f.kind);
		const QetRevision::Fingerprint fp = QetRevision::decode(f.fingerprint);
		before.insert(f.uuid, fp);
		if (fp.isEmpty()) continue;
		const QetRevision::Fingerprint now = fingerprintOf(project, d);
		const QList<QetRevision::Change> list = QetRevision::compare(fp, now);
		if (!list.isEmpty()) {
			changes.insert(f.uuid, list);
			if (f.kind == QLatin1String("plc"))
				r.report << tr("The PLC overview (folio %1) was changed by hand; modules are only added to it.")
							.arg(project->folioIndex(d) + 1);
		}
	}
	const auto describe = [&](Diagram *d) {
		const QString uuid = d->uuid().toString();
		return QetRevision::describe(changes.value(uuid), fingerprintOf(project, d), before.value(uuid));
	};

	QSet<QString> matched;
	QList<Diagram *> kept_by_hand;	// changed by hand, left as they are
	for (Unit &unit : units) {
		const QStringList objects = sorted(unit.objects(plans));
		for (const ManifestFolio &f : old.folios) {
			if (f.kind != QLatin1String("plan") || matched.contains(f.uuid)) continue;
			if (sorted(f.objects) != objects) continue;
			Diagram *d = by_uuid.value(f.uuid);
			if (!d) continue;
			matched.insert(f.uuid);
			bool same = true;
			for (int i : unit.plans) {
				const Plan &plan = plans.at(i);
				const auto it = old.objects.constFind(plan.object);
				same &= it != old.objects.cend() && it->hash == plan.hash && plan.stable_template
						&& it->variant == plan.variant;
			}
			if (same) {
				unit.keep = true;
				unit.folio = d;
				++r.kept;
			} else if (!changes.contains(f.uuid) || options.overwrite) {
				unit.replaces = d;
			} else {
				unit.beside = d;
				kept_by_hand << d;
				r.conflicts << tr("Folio %1 (%2) was changed by hand since it was generated; it is kept, "
								  "the new one follows it with [new] in its title:")
							   .arg(project->folioIndex(d) + 1).arg(objects.join(QStringLiteral(", ")));
				for (const QString &c : describe(d)) r.conflicts << QStringLiteral("  ") + c;
			}
			break;
		}
	}
	QList<Diagram *> to_remove;
	QList<Diagram *> plc_folios;
	for (const ManifestFolio &f : old.folios) {
		Diagram *d = by_uuid.value(f.uuid);
		if (!d) {
			if (f.kind == QLatin1String("plan"))
				r.report << tr("A generated folio of %1 was deleted by hand.").arg(f.objects.join(QStringLiteral(", ")));
			continue;
		}
		if (f.kind == QLatin1String("plc")) { plc_folios << d; continue; }
		if (f.kind == QLatin1String("list")) { to_remove << d; continue; }
		if (f.kind == QLatin1String("kept")) {
				//Left from a conflict before: overwrite clears it away
			if (options.overwrite) {
				to_remove << d;
				++r.removed;
			} else if (!kept_by_hand.contains(d)) {
				kept_by_hand << d;
				r.conflicts << tr("Folio %1 is still there from an earlier conflict; delete it when its changes are taken over.")
							   .arg(project->folioIndex(d) + 1);
			}
			continue;
		}
		if (matched.contains(f.uuid)) continue;
			//Its objects are gone or on other folios now
		if (changes.contains(f.uuid) && !options.overwrite) {
			kept_by_hand << d;
			r.conflicts << tr("Folio %1 (%2) is no longer generated but was changed by hand; it is kept.")
						   .arg(project->folioIndex(d) + 1).arg(f.objects.join(QStringLiteral(", ")));
		} else {
			to_remove << d;
			++r.removed;
		}
	}
	for (const Unit &unit : std::as_const(units))
		if (unit.replaces) to_remove << unit.replaces;
	for (Diagram *d : std::as_const(to_remove)) {
		project->buildFolioToShow(d);
		release(project, d);
		project->removeDiagram(d);
	}

		//5. The PLC: a generic module of 16 channels for each pair of
		//   bytes used, unless a module of the project offers them
	QHash<QString, QSet<int>> wanted;	// letter -> module numbers
	for (const Plan &plan : std::as_const(plans))
		for (const auto &p : plan.drawn.parameters()) {
			if (p.kind != QLatin1String("address")) continue;
			QString letter; int bit = 0;
			if (!addressBit(plan.given.value(p.name), &letter, &bit)) continue;
			for (int k = 0 ; k < p.count ; ++k) wanted[letter].insert((bit + k) / ChannelsPerModule);
		}
	QSet<QString> present_modules;	// "I0"
	QSet<QString> offered;	// "I0.0", by modules of the user
	QSet<QString> labels;
	for (Diagram *d : project->diagrams()) {
		project->buildFolioToShow(d);
		for (QGraphicsItem *item : d->items()) {
			auto *element = qgraphicsitem_cast<Element *>(item);
			if (!element) continue;
			labels.insert(labelOf(element).toUpper());
			const ElementData data = element->elementData();
			if (data.m_type != ElementData::Master || data.m_master_type != ElementData::PLC) continue;
			static const QRegularExpression generic(QStringLiteral("Konfigurator/SPS_([IQ])_(\\d+)\\.elmt$"));
			const QRegularExpressionMatch match = generic.match(element->location().toString());
			if (match.hasMatch()) {
				present_modules.insert(match.captured(1) + match.captured(2));
				continue;
			}
			for (const auto &io : data.plcMasterData().ios) {
				QString address = io.address.trimmed().toUpper().remove(QLatin1Char('%')).remove(QLatin1Char(' '));
				if (address.startsWith(QLatin1Char('E'))) address[0] = QLatin1Char('I');
				if (address.startsWith(QLatin1Char('A'))) address[0] = QLatin1Char('Q');
				offered.insert(address);
			}
		}
	}
	XmlElementCollection *collection = project->embeddedElementCollection();
	if (collection && !collection->exist(moduleDir())) {
		NamesList dir_names;
		dir_names.addName(QStringLiteral("en"), QStringLiteral("Configurator"));
		dir_names.addName(QStringLiteral("de"), QStringLiteral("Konfigurator"));
		collection->createDir(QStringLiteral("import"), QStringLiteral("Konfigurator"), dir_names);
	}
	const GenericDevice::Fonts fonts{QETApp::diagramTextsFont(7), QETApp::dynamicTextsItemFont(9)};
	int next_kf = 1;
	Diagram *plc_folio = plc_folios.isEmpty() ? nullptr : plc_folios.last();
		//Modules side by side, each its box and its table wide: the slots
		//of the last PLC folio already taken by modules
	int plc_slot = 0;
	if (plc_folio) {
		for (QGraphicsItem *item : plc_folio->items())
			if (auto *element = qgraphicsitem_cast<Element *>(item))
				if (element->location().toString().contains(QLatin1String("Konfigurator/SPS_"))) ++plc_slot;
	}
	int modules_made = 0;
	for (const QString &letter : {QStringLiteral("I"), QStringLiteral("Q")}) {
		QList<int> numbers = wanted.value(letter).values();
		std::sort(numbers.begin(), numbers.end());
		for (int number : std::as_const(numbers)) {
			if (present_modules.contains(letter + QString::number(number))) continue;
			bool by_user = false;
			for (int k = 0 ; k < ChannelsPerModule && !by_user ; ++k)
				by_user = offered.contains(addressOf(letter, number * ChannelsPerModule + k).mid(1));
			if (by_user) continue;
			if (!collection) break;
			const bool input = letter == QLatin1String("I");
			GenericDevice::Spec spec;
			const QString first = addressOf(letter, number * ChannelsPerModule).mid(1);
			const QString last = addressOf(letter, number * ChannelsPerModule + ChannelsPerModule - 1).mid(1);
			spec.name = QStringLiteral("SPS %1 16 %2-%3").arg(input ? QStringLiteral("DI") : QStringLiteral("DQ"),
															   first, last);
			spec.description = input ? tr("Digital input module, 16 channels (generic)")
									 : tr("Digital output module, 16 channels (generic)");
			spec.reference = input ? QStringLiteral("DI 16") : QStringLiteral("DQ 16");
			GenericDevice::Slots channels;
			for (int k = 0 ; k < ChannelsPerModule ; ++k)
				channels << GenericDevice::Slot::terminal(addressOf(letter, number * ChannelsPerModule + k).mid(1));
			spec.sides[GenericDevice::Left] = channels;
			spec.pitch = TableRow;
			spec.io[GenericDevice::Left] = input ? GenericDevice::Io::DigitalIn : GenericDevice::Io::DigitalOut;
			QDomDocument doc;
			const QDomElement definition = GenericDevice::toDefinition(
						spec, fonts, doc, QetVersion::currentVersion().toString());
			tableBesideChannels(definition);
			const QString file = QStringLiteral("SPS_%1_%2.elmt").arg(letter).arg(number);
			if (!collection->exist(moduleDir() + QLatin1Char('/') + file))
				collection->addElementDefinition(moduleDir(), file, definition);
			const ElementsLocation location(moduleDir() + QLatin1Char('/') + file, project);
			int state = 0;
			Element *element = ElementFactory::Instance()->createElement(location, nullptr, &state);
			if (state || !element) {
				delete element;
				r.report << tr("The PLC module %1 cannot be made.").arg(spec.name);
				continue;
			}
			while (labels.contains(QStringLiteral("KF%1").arg(next_kf))) ++next_kf;
			const QString label = QStringLiteral("KF%1").arg(next_kf);
			labels.insert(label);
			DiagramContext informations = element->elementInformations();
			informations.addValue(QStringLiteral("label"), label);
			informations.addValue(QStringLiteral("description"), spec.description);
			element->setElementInformations(informations);

				//Box, gap, table, a column: whole columns, so every module
				//starts on a column like the first
			const qreal box = definition.attribute(QStringLiteral("width")).toDouble();
			const qreal step = std::ceil((box + TableGap + TableWidth + gap) / border.columns_width)
					* border.columns_width;
			const int per_folio = qMax(1, int((inner_width - border.columns_width) / step));
			if (!plc_folio || plc_slot >= per_folio) {
				plc_folio = project->addNewDiagram();
				setTitle(plc_folio, tr("PLC overview"), QString(), QString());
				plc_folios << plc_folio;
				plc_slot = 0;
			}
			const QRectF inside = plc_folio->border_and_titleblock.insideBorderRect();
			const QRectF shape = element->boundingRect();
			const qreal x = inside.left() + border.columns_width + plc_slot * step;
			const QPointF pos = Diagram::snapToGrid(QPointF(x - shape.left(),
															inside.top() + 2 * border.rows_height - shape.top()));
			element->setPos(pos);
			plc_folio->addItem(element);
			++plc_slot;
			++modules_made;
			r.report << tr("PLC module %1: %2 (%3 ... %4)").arg(label, spec.reference, first, last);
		}
	}

		//"PLC channel %Q0.0 linked to KF2." for every channel says nothing
		//the PLC overview does not show
	const QString link = QCoreApplication::translate("FunctionTemplate", "PLC channel %1 linked to %2.");
	const QRegularExpression linked(
				QStringLiteral("^") + QRegularExpression::escape(link.section(QStringLiteral("%1"), 0, 0))
				+ QStringLiteral(".*") + QRegularExpression::escape(
					link.section(QStringLiteral("%1"), 1).section(QStringLiteral("%2"), 0, 0))
				+ QStringLiteral(".*"));

		//6. Draw the folios which are new or changed
	QHash<QString, Diagram *> folio_of;	// object -> folio
	QHash<QString, QMap<QString, QString>> values_of;
	for (Unit &unit : units) {
		if (unit.keep) {
			for (int i : unit.plans) folio_of.insert(plans.at(i).object, unit.folio);
			continue;
		}
		Diagram *d = project->addNewDiagram();
		if (!d) {
			r.error = tr("No folio could be added.");
			return r;
		}
		unit.folio = d;
		const Plan &first = plans.at(unit.plans.first());
		setTitle(d, first.typical.name(), first.plant, first.location);
		const QRectF inside = d->border_and_titleblock.insideBorderRect();
		qreal cursor = 0;
		QStringList titles;
		for (int n = 0 ; n < unit.plans.size() ; ++n) {
			const Plan &plan = plans.at(unit.plans.at(n));
			QPointF pos;
			if (n > 0) {
					//Beside the one before, on the next column
				qreal x = cursor + gap;
				x = inside.left() + std::ceil((x - inside.left()) / border.columns_width) * border.columns_width;
				pos = QPointF(x, plan.extent.top());
			}
			const FunctionTemplate::Placed placed = plan.typical.place(d, plan.given, pos);
			if (!placed.ok) {
				r.report << tr("%1: %2").arg(plan.object, placed.error);
				continue;
			}
			cursor = n == 0 ? plan.extent.right() : pos.x() + plan.extent.width();
			folio_of.insert(plan.object, d);
			values_of.insert(plan.object, placed.values);
			for (const QString &note : placed.notes)
				if (!linked.match(note).hasMatch()) r.report << tr("%1: %2").arg(plan.object, note);
			QString label;
			for (const auto &p : plan.drawn.parameters())
				if (p.kind == QLatin1String("label")) { label = placed.values.value(p.name); break; }
			titles << (label.isEmpty() ? plan.object : QStringLiteral("-%1 %2").arg(label, plan.object));
		}
		const QString title = (unit.plans.size() == 1
							   ? first.typical.name() + QLatin1Char(' ') + titles.join(QString())
							   : first.typical.name() + QStringLiteral(": ") + titles.join(QStringLiteral(", ")))
				+ (unit.beside ? tr(" [new]") : QString());
		setTitle(d, title, first.plant, first.location);
		if (unit.replaces) ++r.replaced;
		else ++r.created;
	}

		//A channel left unlinked before -- drawn while its module channel
		//was still taken by a folio now gone -- is linked now
	for (const Unit &unit : std::as_const(units)) {
		if (!unit.folio) continue;
		QHash<Element *, QString> open;
		for (QGraphicsItem *item : unit.folio->items()) {
			auto *element = qgraphicsitem_cast<Element *>(item);
			if (!element || element->linkType() != Element::Slave || !element->linkedElements().isEmpty()) continue;
			const QString address = element->elementInformations().value(QStringLiteral("plc_address")).toString().trimmed();
			if (!address.isEmpty()) open.insert(element, address);
		}
		if (open.isEmpty()) continue;
		QStringList notes;
		FunctionTemplate::linkPlcChannels(unit.folio, open, &notes);
		for (const QString &note : std::as_const(notes))
			if (!linked.match(note).hasMatch()) r.report << note;
	}

		//The channel table of each generic module says what is on each
		//channel: the function its channel symbol in the circuit carries
	for (Diagram *d : std::as_const(plc_folios)) {
		for (QGraphicsItem *item : d->items()) {
			auto *module = qgraphicsitem_cast<Element *>(item);
			if (!module || !module->location().toString().contains(QLatin1String("Konfigurator/SPS_"))) continue;
			ElementData data = module->elementData();
			auto master = data.plcMasterData();
			bool changed = false;
			for (int i = 0 ; i < master.ios.size() ; ++i) {
				QString text;
				for (Element *channel : module->linkedElements())
					if (module->groupIndexForElement(channel) == i)
						text = channel->elementInformations().value(QStringLiteral("function")).toString().trimmed();
				if (master.ios[i].functionText != text) {
					master.ios[i].functionText = text;
					changed = true;
				}
			}
			if (!changed) continue;
			data.setPlcMasterData(master);
			module->setElementData(data);
		}
	}

		//7. The order: the folios not generated first, then the plan in
		//   the order of the table, the ones kept by hand, the PLC
	QList<Diagram *> order;
	QSet<Diagram *> generated;
	for (const Unit &unit : std::as_const(units)) {
		if (unit.folio) generated.insert(unit.folio);
		if (unit.beside) generated.insert(unit.beside);
	}
	for (Diagram *d : std::as_const(kept_by_hand)) generated.insert(d);
	for (Diagram *d : std::as_const(plc_folios)) generated.insert(d);
	for (Diagram *d : project->diagrams())
		if (!generated.contains(d)) order << d;
	for (const Unit &unit : std::as_const(units)) {
		if (unit.beside && !order.contains(unit.beside)) order << unit.beside;
		if (unit.folio && !order.contains(unit.folio)) order << unit.folio;
	}
	for (Diagram *d : std::as_const(kept_by_hand)) if (!order.contains(d)) order << d;
	for (Diagram *d : std::as_const(plc_folios)) if (!order.contains(d)) order << d;
	for (int i = 0 ; i < order.size() ; ++i) {
		const int from = project->folioIndex(order.at(i));
		if (from >= 0 && from != i) project->diagramOrderChanged(from, i);
	}

		//A folio drawn beside one changed by hand keeps its [new] until
		//no such folio is left (deleted by hand, or overwritten)
	if (kept_by_hand.isEmpty()) {
		for (const Unit &unit : std::as_const(units)) {
			if (!unit.keep || !unit.folio) continue;
			const QString title = unit.folio->border_and_titleblock.exportTitleBlock().title;
			if (title.endsWith(tr(" [new]")))
				setTitle(unit.folio, title.left(title.size() - tr(" [new]").size()), QString(), QString());
		}
	}

		//8. The lists, from the project database, at the end; rebuilt all
		//over, folios were taken out
	project->dataBase()->updateDBForce();
	QList<Diagram *> list_folios;
	const int parts = addList(project, tr("Order list"), QStringLiteral("konfigurator_bestellliste"),
			QStringLiteral(
				"SELECT COALESCE(structure_id, '') AS \"%1\", COALESCE(description, '') AS \"%2\", "
				"CASE WHEN COALESCE(articles, '') = '' THEN '%3' ELSE articles END AS \"%4\", "
				"COALESCE(folio_number, '') AS \"%5\" FROM qet_device_list ORDER BY structure_id")
				.arg(tr("Device"), tr("Description"), tr("article open"), tr("Article"), tr("Folio")),
			&list_folios);
	const int terminals = addList(project, tr("Terminal plan"), QStringLiteral("konfigurator_klemmenplan"),
			QStringLiteral(
				"SELECT COALESCE(strip_id, '') AS \"%1\", COALESCE(label, '') AS \"%2\", "
				"COALESCE(connected_to, '') AS \"%3\", COALESCE(folio_number, '') AS \"%4\" "
				"FROM qet_terminals ORDER BY strip_id, strip_uuid, position, level")
				.arg(tr("Strip"), tr("Terminal"), tr("Connected"), tr("Folio")),
			&list_folios);
	const int cables = addList(project, tr("Cable list"), QStringLiteral("konfigurator_kabelliste"),
			QStringLiteral(
				"SELECT COALESCE(structure_id, '') AS \"%1\", COALESCE(type, '') AS \"%2\", "
				"COALESCE(cores, '') AS \"%3\", COALESCE(folio_number, '') AS \"%4\" "
				"FROM qet_cables ORDER BY structure_id")
				.arg(tr("Cable"), tr("Type"), tr("Cores"), tr("Folio")),
			&list_folios);
	r.report << tr("Lists: %1 devices, %2 terminals, %3 cables.").arg(parts).arg(terminals).arg(cables);
	project->dataBase()->updateDBForce();

		//9. What this run made, for the next one
	Manifest m;
	m.present = true;
	m.source = source;
	for (const Unit &unit : std::as_const(units)) {
		for (int i : unit.plans) {
			const Plan &plan = plans.at(i);
			ManifestObject o;
			if (unit.keep) {
				o = old.objects.value(plan.object);
			} else {
				o.name = plan.object;
				o.hash = plan.hash;
				o.templ = templateId(plan.drawn);
				o.variant = plan.variant;
				const QMap<QString, QString> values = values_of.value(plan.object);
				for (const auto &p : plan.drawn.parameters()) {
					if (!counted(p.kind)) continue;
					const QString value = values.value(p.name, plan.given.value(p.name));
					if (value.isEmpty()) continue;
					o.stable.insert(p.name, value);
					if (p.kind == QLatin1String("address"))
						o.stable.insert(p.name + QStringLiteral("#count"), QString::number(p.count));
				}
			}
			Diagram *d = folio_of.value(plan.object);
			o.folio = d ? d->uuid().toString() : QString();
			m.objects.insert(o.name, o);
		}
	}
	for (const Unit &unit : std::as_const(units)) {
		if (!unit.folio) continue;
		ManifestFolio f;
		f.uuid = unit.folio->uuid().toString();
		f.kind = QStringLiteral("plan");
		f.objects = unit.objects(plans);
		bool found = false;
		if (unit.keep)
			for (const ManifestFolio &o : old.folios)
				if (o.uuid == f.uuid) { f.fingerprint = o.fingerprint; found = true; }
		if (!found) f.fingerprint = QetRevision::encode(fingerprintOf(project, unit.folio));
		m.folios << f;
	}
	for (Diagram *d : std::as_const(kept_by_hand)) {
		ManifestFolio f;
		f.uuid = d->uuid().toString();
		f.kind = QStringLiteral("kept");
		m.folios << f;
	}
	for (Diagram *d : std::as_const(plc_folios)) {
		ManifestFolio f;
		f.uuid = d->uuid().toString();
		f.kind = QStringLiteral("plc");
		f.fingerprint = QetRevision::encode(fingerprintOf(project, d));
		m.folios << f;
	}
	for (Diagram *d : std::as_const(list_folios)) {
		ManifestFolio f;
		f.uuid = d->uuid().toString();
		f.kind = QStringLiteral("list");
		m.folios << f;
	}
	saveManifest(project, m);

	r.folios = project->diagrams().size();
	r.report.prepend(tr("%1 objects: %2 folios drawn new, %3 drawn again, %4 kept, %5 removed, %6 conflicts.")
					 .arg(r.objects).arg(r.created).arg(r.replaced).arg(r.kept).arg(r.removed)
					 .arg(r.conflicts.isEmpty() ? 0 : kept_by_hand.size()));
	for (const Plan &plan : std::as_const(plans)) {
		Diagram *d = folio_of.value(plan.object);
		r.report << tr("%1: %2%3, folio %4").arg(plan.object, templateId(plan.drawn),
			plan.variant.isEmpty() ? QString() : QStringLiteral(" (") + plan.variant + QLatin1Char(')'))
					.arg(d ? project->folioIndex(d) + 1 : 0);
	}
	r.ok = true;
	return r;
}
