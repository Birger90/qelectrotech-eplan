/*
	Copyright 2006-2025 The QElectroTech Team
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
#include "projectrevisions.h"

#include "../cli_export.h"
#include "../dataBase/projectdatabase.h"
#include "../diagram.h"
#include "../qetgraphicsitem/conductor.h"
#include "../qetgraphicsitem/diagramimageitem.h"
#include "../qetgraphicsitem/element.h"
#include "../qetgraphicsitem/independenttextitem.h"
#include "../qetgraphicsitem/qetshapeitem.h"
#include "../qetproject.h"
#include "../qetresult.h"
#include "../titleblockproperties.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDir>
#include <QDomDocument>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QTextDocumentFragment>
#include <QTextStream>
#include <QUndoStack>

#include <algorithm>

namespace QetRevision {

namespace {

bool looksLikeUuid(const QString &value)
{
	static const QRegularExpression re(QStringLiteral(
		"^\\{?[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\}?$"));
	return re.match(value).hasMatch();
}

/**
	The text of @p e as it looks: its tag, its attributes sorted, its text
	and children, without uuids (they say who it is, not what it looks
	like) and without @p skip_attributes and @p skip_children. The uuids
	met are added to @p uuids.
*/
void canonical(const QDomElement &e, const QSet<QString> &skip_attributes,
			   const QSet<QString> &skip_children, QString &out, QStringList &uuids)
{
	out += QLatin1Char('<') + e.tagName();
	const QDomNamedNodeMap attributes = e.attributes();
	QStringList pairs;
	for (int i = 0 ; i < attributes.count() ; ++i) {
		const QDomAttr a = attributes.item(i).toAttr();
		if (skip_attributes.contains(a.name())) continue;
		if (a.name() == QLatin1String("uuid") || looksLikeUuid(a.value())) {
			if (a.name() != QLatin1String("uuid")) uuids << a.name() + QLatin1Char('=') + a.value();
			continue;
		}
		pairs << a.name() + QStringLiteral("=\"") + a.value() + QLatin1Char('"');
	}
	pairs.sort();
	out += QLatin1Char(' ') + pairs.join(QLatin1Char(' ')) + QLatin1Char('>');
	for (QDomNode n = e.firstChild() ; !n.isNull() ; n = n.nextSibling()) {
		if (n.isElement()) {
			const QDomElement child = n.toElement();
			if (skip_children.contains(child.tagName())) continue;
			canonical(child, skip_attributes, skip_children, out, uuids);
		} else if (n.isText() || n.isCDATASection()) {
			const QString text = n.nodeValue().trimmed();
			if (looksLikeUuid(text)) uuids << text;
			else out += text;
		}
	}
	out += QStringLiteral("</") + e.tagName() + QLatin1Char('>');
}

QByteArray shortHash(const QString &text)
{
	return QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha1).left(10).toHex();
}

QString elementLabel(const QDomElement &element)
{
	for (QDomElement infos = element.firstChildElement(QStringLiteral("elementInformations"));
		 !infos.isNull() ; infos = infos.nextSiblingElement(QStringLiteral("elementInformations")))
		for (QDomElement info = infos.firstChildElement(QStringLiteral("elementInformation"));
			 !info.isNull() ; info = info.nextSiblingElement(QStringLiteral("elementInformation")))
			if (info.attribute(QStringLiteral("name")) == QLatin1String("label"))
				return info.text().trimmed();
	return QString();
}

QString shortType(const QString &type)
{
	QString name = type.section(QLatin1Char('/'), -1);
	if (name.endsWith(QLatin1String(".elmt"))) name.chop(5);
	return name;
}

QRectF around(qreal x, qreal y, qreal w = 40, qreal h = 40)
{
	return QRectF(x - w / 2, y - h / 2, w, h);
}

QString pointsText(const QPolygonF &polygon)
{
	QStringList points;
	for (const QPointF &p : polygon)
		points << QString::number(p.x(), 'f', 1) + QLatin1Char(',') + QString::number(p.y(), 'f', 1);
	return points.join(QLatin1Char(' '));
}

QPolygonF pointsFrom(const QString &text)
{
	QPolygonF polygon;
	for (const QString &point : text.split(QLatin1Char(' '), Qt::SkipEmptyParts))
		polygon << QPointF(point.section(QLatin1Char(','), 0, 0).toDouble(),
						   point.section(QLatin1Char(','), 1, 1).toDouble());
	return polygon;
}

QString rectText(const QRectF &r)
{
	return QStringLiteral("%1,%2,%3,%4").arg(r.x(), 0, 'f', 1).arg(r.y(), 0, 'f', 1)
			.arg(r.width(), 0, 'f', 1).arg(r.height(), 0, 'f', 1);
}

QRectF rectFrom(const QString &text)
{
	const QStringList v = text.split(QLatin1Char(','));
	if (v.size() != 4) return QRectF();
	return QRectF(v.at(0).toDouble(), v.at(1).toDouble(), v.at(2).toDouble(), v.at(3).toDouble());
}

/// The live items of @p diagram by uuid, with where they are drawn
struct LiveItems {
	QHash<QString, QRectF> rects;
	QHash<QString, QPolygonF> paths;
};

LiveItems liveItems(Diagram *diagram)
{
	LiveItems live;
	if (!diagram) return live;
	for (QGraphicsItem *item : diagram->items()) {
		if (auto *element = qgraphicsitem_cast<Element *>(item)) {
			live.rects.insert(element->uuid().toString(), element->sceneBoundingRect());
		} else if (auto *conductor = qgraphicsitem_cast<Conductor *>(item)) {
			const QString uuid = conductor->uuid().toString();
			const QPainterPath path = conductor->mapToScene(conductor->path());
			live.rects.insert(uuid, path.boundingRect());
			const QList<QPolygonF> polygons = path.toSubpathPolygons();
			if (!polygons.isEmpty()) live.paths.insert(uuid, polygons.first());
		} else if (auto *text = dynamic_cast<IndependentTextItem *>(item)) {
			live.rects.insert(text->uuid().toString(), text->sceneBoundingRect());
		} else if (auto *shape = dynamic_cast<QetShapeItem *>(item)) {
			live.rects.insert(shape->uuid().toString(), shape->sceneBoundingRect());
		} else if (auto *image = dynamic_cast<DiagramImageItem *>(item)) {
			live.rects.insert(image->uuid().toString(), image->sceneBoundingRect());
		}
	}
	return live;
}

/// The children of @p folio's <@p list> named @p tag
QList<QDomElement> children(const QDomElement &folio, const QString &list, const QString &tag)
{
	QList<QDomElement> found;
	for (QDomElement l = folio.firstChildElement(list) ; !l.isNull() ; l = l.nextSiblingElement(list))
		for (QDomElement e = l.firstChildElement(tag) ; !e.isNull() ; e = e.nextSiblingElement(tag))
			found << e;
	return found;
}

} // namespace

/**
	@brief fingerprint
	@return the fingerprint of one folio from its XML @p diagram (as
	Diagram::toXml() or a project file has it), @p index its place in the
	project; with @p live, where its items are drawn, else where their
	XML says (an estimate for symbols and wires)
*/
Fingerprint fingerprint(const QDomElement &diagram, int index, Diagram *live)
{
	Fingerprint fp;
	const QString folio = diagram.attribute(QStringLiteral("uuid"),
											QStringLiteral("folio-%1").arg(index));
	fp.folios << folio;
	const QString title = diagram.attribute(QStringLiteral("title"));
	fp.folio_names.insert(folio, QStringLiteral("%1%2").arg(index + 1)
						  .arg(title.isEmpty() ? QString() : QStringLiteral(" - ") + title));
	{
			//The title block, without what a revision itself writes
		QStringList pairs;
		const QDomNamedNodeMap attributes = diagram.attributes();
		static const QSet<QString> skip{QStringLiteral("indexrev"), QStringLiteral("date"),
										QStringLiteral("version"), QStringLiteral("order"),
										QStringLiteral("uuid"), QStringLiteral("auto_page_num")};
		for (int i = 0 ; i < attributes.count() ; ++i) {
			const QDomAttr a = attributes.item(i).toAttr();
			if (!skip.contains(a.name())) pairs << a.name() + QLatin1Char('=') + a.value();
		}
		pairs.sort();
		fp.folio_hashes.insert(folio, shortHash(pairs.join(QLatin1Char('\n'))));
	}
	const LiveItems items = liveItems(live);
	const auto rectOf = [&](const QString &uuid, const QRectF &estimate) {
		return items.rects.value(uuid, estimate);
	};

	QHash<QString, QString> element_keys;	// uuid -> key, for the wires
	QHash<QString, QString> element_names;
	static const QSet<QString> element_skip{QStringLiteral("z")};
	static const QSet<QString> element_skip_children{QStringLiteral("terminals")};
	for (const QDomElement &e : children(diagram, QStringLiteral("elements"), QStringLiteral("element"))) {
		Item item;
		item.kind = QLatin1Char('e');
		item.uuid = e.attribute(QStringLiteral("uuid"));
		item.folio = folio;
		item.folio_index = index;
		const QString label_text = elementLabel(e);
		const QString type = shortType(e.attribute(QStringLiteral("type")));
		item.key = QStringLiteral("%1|%2").arg(type, label_text.isEmpty()
			? e.attribute(QStringLiteral("x")) + QLatin1Char(',') + e.attribute(QStringLiteral("y"))
			: label_text);
		item.name = label_text.isEmpty() ? type : label_text;
		QString text;
		QStringList uuids;
		canonical(e, element_skip, element_skip_children, text, uuids);
		uuids.sort();
		item.hash = shortHash(text);
		item.ends = shortHash(uuids.join(QLatin1Char(' ')));
		item.rect = rectOf(item.uuid, around(e.attribute(QStringLiteral("x")).toDouble(),
											 e.attribute(QStringLiteral("y")).toDouble()));
		if (item.uuid.isEmpty()) item.uuid = QStringLiteral("e:%1:%2").arg(folio, item.key);
		element_keys.insert(e.attribute(QStringLiteral("uuid")), item.key);
		element_names.insert(e.attribute(QStringLiteral("uuid")), item.name);
		fp.items.insert(item.uuid, item);
	}

	static const QSet<QString> wire_skip{
		QStringLiteral("element1_label"), QStringLiteral("element2_label"),
		QStringLiteral("element1_name"), QStringLiteral("element2_name"),
		QStringLiteral("element1_linked"), QStringLiteral("element2_linked"),
		QStringLiteral("terminal1"), QStringLiteral("terminal2"),
		QStringLiteral("element1"), QStringLiteral("element2"), QStringLiteral("z")};
	for (const QDomElement &c : children(diagram, QStringLiteral("conductors"), QStringLiteral("conductor"))) {
		Item item;
		item.kind = QLatin1Char('c');
		item.uuid = c.attribute(QStringLiteral("uuid"));
		item.folio = folio;
		item.folio_index = index;
		QStringList ends, names;
		for (int n = 1 ; n <= 2 ; ++n) {
			const QString owner = c.attribute(QStringLiteral("element%1").arg(n));
			const QString terminal = c.attribute(QStringLiteral("terminalname%1").arg(n));
			ends << element_keys.value(owner, owner) + QLatin1Char(':') + terminal;
			names << element_names.value(owner, QStringLiteral("?"))
					 + (terminal.isEmpty() ? QString() : QLatin1Char(':') + terminal);
		}
		ends.sort();
		item.key = ends.join(QStringLiteral("--"));
		const QString number = c.attribute(QStringLiteral("num"));
		item.name = names.join(QStringLiteral(" - "))
				+ (number.isEmpty() || number == QLatin1String("_") ? QString()
																	 : QStringLiteral(" (%1)").arg(number));
		QString text;
		QStringList uuids;
		canonical(c, wire_skip, {}, text, uuids);
		item.hash = shortHash(text);
		QStringList refs;
		for (const QString &a : {QStringLiteral("element1"), QStringLiteral("terminal1"),
								 QStringLiteral("element2"), QStringLiteral("terminal2")})
			refs << c.attribute(a);
		const QString first = refs.at(0) + refs.at(1), second = refs.at(2) + refs.at(3);
		item.ends = shortHash(first < second ? first + second : second + first);
		item.rect = rectOf(item.uuid, around(c.attribute(QStringLiteral("x")).toDouble(),
											 c.attribute(QStringLiteral("y")).toDouble(), 20, 20));
		item.path = items.paths.value(item.uuid);
		if (item.uuid.isEmpty()) item.uuid = QStringLiteral("c:%1:%2").arg(folio, item.key);
		fp.items.insert(item.uuid, item);
	}

	const auto plain = [&](QChar kind, const QDomElement &e, const QString &key,
						   const QString &name, const QRectF &estimate) {
		Item item;
		item.kind = kind;
		item.uuid = e.attribute(QStringLiteral("uuid"));
		item.folio = folio;
		item.folio_index = index;
		item.key = key;
		item.name = name;
		QString text;
		QStringList uuids;
		canonical(e, {QStringLiteral("z")}, {}, text, uuids);
		item.hash = shortHash(text);
		item.rect = rectOf(item.uuid, estimate);
		if (item.uuid.isEmpty()) item.uuid = QStringLiteral("%1:%2:%3").arg(kind).arg(folio, key);
		fp.items.insert(item.uuid, item);
	};
	for (const QDomElement &t : children(diagram, QStringLiteral("inputs"), QStringLiteral("input"))) {
		QString shown = t.attribute(QStringLiteral("text"));
		if (shown.isEmpty()) shown = t.firstChildElement(QStringLiteral("text")).text();
		if (shown.contains(QLatin1Char('<')))
			shown = QTextDocumentFragment::fromHtml(shown).toPlainText();
		shown = shown.simplified();
		const qreal x = t.attribute(QStringLiteral("x")).toDouble(), y = t.attribute(QStringLiteral("y")).toDouble();
		plain(QLatin1Char('t'), t, shown + QLatin1Char('@') + t.attribute(QStringLiteral("x"))
			  + QLatin1Char(',') + t.attribute(QStringLiteral("y")),
			  shown.left(40), QRectF(x, y - 5, qMax<qreal>(20, shown.size() * 5), 12));
	}
	for (const QDomElement &s : children(diagram, QStringLiteral("shapes"), QStringLiteral("shape"))) {
		const QRectF r = QRectF(QPointF(s.attribute(QStringLiteral("x1")).toDouble(),
										s.attribute(QStringLiteral("y1")).toDouble()),
								QPointF(s.attribute(QStringLiteral("x2")).toDouble(),
										s.attribute(QStringLiteral("y2")).toDouble())).normalized();
		plain(QLatin1Char('s'), s, s.attribute(QStringLiteral("type")) + QLatin1Char('@') + rectText(r),
			  s.attribute(QStringLiteral("type")), r);
	}
	for (const QDomElement &i : children(diagram, QStringLiteral("images"), QStringLiteral("image"))) {
		const qreal x = i.attribute(QStringLiteral("x")).toDouble(), y = i.attribute(QStringLiteral("y")).toDouble();
		plain(QLatin1Char('i'), i, QStringLiteral("image@%1,%2").arg(x).arg(y),
			  QStringLiteral("image"), around(x, y, 60, 60));
	}
	return fp;
}

/**
	@brief fingerprint
	@return the fingerprint of every folio of @p project, each built
*/
Fingerprint fingerprint(QETProject *project)
{
	Fingerprint fp;
	if (!project) return fp;
	const QList<Diagram *> diagrams = project->diagrams();
	for (int i = 0 ; i < diagrams.size() ; ++i) {
		Diagram *diagram = diagrams.at(i);
		const QDomDocument xml = diagram->toXml();
		const Fingerprint one = fingerprint(xml.documentElement(), i, diagram);
		fp.folios << one.folios;
		fp.folio_names.insert(one.folio_names);
		fp.folio_hashes.insert(one.folio_hashes);
		fp.items.insert(one.items);
	}
	return fp;
}

/**
	@brief fingerprintOfFile
	@return the fingerprint of the project file @p path, opened for it
*/
Fingerprint fingerprintOfFile(const QString &path, QString *error)
{
	if (!QFileInfo::exists(path)) {
		if (error) *error = QObject::tr("%1 not found").arg(path);
		return {};
	}
	QETProject project(path);
	if (project.state() != QETProject::Ok) {
		if (error) *error = QObject::tr("%1 could not be opened").arg(path);
		return {};
	}
	return fingerprint(&project);
}

/**
	@brief compare
	@return what changed from @p before to @p now. Folios are paired by
	uuid, then those left by their place; items by uuid, then those left by
	their identity on their paired folio (alternative identification).
*/
QList<Change> compare(const Fingerprint &before, const Fingerprint &now)
{
	QList<Change> changes;
		//Folios: before's uuid -> now's uuid
	QHash<QString, QString> paired;
	const QSet<QString> now_folios(now.folios.cbegin(), now.folios.cend());
	QStringList before_left, now_left;
	for (const QString &f : before.folios)
		if (now_folios.contains(f)) paired.insert(f, f);
		else before_left << f;
	const QSet<QString> before_folios(before.folios.cbegin(), before.folios.cend());
	for (const QString &f : now.folios)
		if (!before_folios.contains(f)) now_left << f;
	for (int i = 0 ; i < qMin(before_left.size(), now_left.size()) ; ++i)
		paired.insert(before_left.at(i), now_left.at(i));

	QSet<QString> matched_now;
	QHash<QString, QString> matched;	// before uuid -> now uuid
	for (auto it = before.items.cbegin() ; it != before.items.cend() ; ++it) {
		const auto found = now.items.constFind(it.key());
		if (found != now.items.cend() && found->kind == it->kind) {
			matched.insert(it.key(), it.key());
			matched_now.insert(it.key());
		}
	}
		//Those left, by identity on the paired folio
	QMultiHash<QString, QString> by_key;
	for (auto it = now.items.cbegin() ; it != now.items.cend() ; ++it)
		if (!matched_now.contains(it.key()))
			by_key.insert(it->folio + QLatin1Char('|') + it->kind + QLatin1Char('|') + it->key, it.key());
	QSet<QString> by_identity;
	for (auto it = before.items.cbegin() ; it != before.items.cend() ; ++it) {
		if (matched.contains(it.key())) continue;
		const QString key = paired.value(it->folio) + QLatin1Char('|') + it->kind + QLatin1Char('|') + it->key;
		const QList<QString> candidates = by_key.values(key);
		for (const QString &candidate : candidates) {
			if (matched_now.contains(candidate)) continue;
			matched.insert(it.key(), candidate);
			matched_now.insert(candidate);
			by_identity.insert(it.key());
			break;
		}
	}

	for (auto it = before.items.cbegin() ; it != before.items.cend() ; ++it) {
		const auto m = matched.constFind(it.key());
		if (m == matched.cend()) {
			Change c;
			c.kind = Change::Removed;
			c.before = *it;
			changes << c;
			continue;
		}
		const Item &n = now.items[*m];
		const bool differs = n.hash != it->hash
				|| (!by_identity.contains(it.key()) && n.ends != it->ends);
		if (differs) {
			Change c;
			c.kind = Change::Changed;
			c.before = *it;
			c.now = n;
			changes << c;
		}
	}
	for (auto it = now.items.cbegin() ; it != now.items.cend() ; ++it) {
		if (matched_now.contains(it.key())) continue;
		Change c;
		c.kind = Change::Added;
		c.now = *it;
		changes << c;
	}
		//Folios: new, removed, or their title blocks changed
	QHash<QString, QString> reverse;
	for (auto it = paired.cbegin() ; it != paired.cend() ; ++it) reverse.insert(it.value(), it.key());
	for (const QString &f : now.folios) {
		Item item;
		item.kind = QLatin1Char('f');
		item.uuid = f;
		item.folio = f;
		item.folio_index = int(now.folios.indexOf(f));
		item.name = now.folio_names.value(f);
		Change c;
		if (!reverse.contains(f)) {
			c.kind = Change::Added;
		} else if (before.folio_hashes.value(reverse.value(f)) != now.folio_hashes.value(f)) {
			c.kind = Change::Changed;
			c.before = item;
		} else {
			continue;
		}
		c.now = item;
		changes << c;
	}
	for (const QString &f : before.folios) {
		if (paired.contains(f)) continue;
		Change c;
		c.kind = Change::Removed;
		c.before.kind = QLatin1Char('f');
		c.before.uuid = f;
		c.before.folio = f;
		c.before.folio_index = int(before.folios.indexOf(f));
		c.before.name = before.folio_names.value(f);
		changes << c;
	}
	std::stable_sort(changes.begin(), changes.end(), [](const Change &a, const Change &b) {
		if (a.item().folio_index != b.item().folio_index)
			return a.item().folio_index < b.item().folio_index;
		if ((a.item().kind == QLatin1Char('f')) != (b.item().kind == QLatin1Char('f')))
			return a.item().kind == QLatin1Char('f');
		return a.item().kind < b.item().kind;
	});
	return changes;
}

/**
	@brief encode
	One line per item, tab separated, compressed: what a project keeps of
	its last revision
*/
QByteArray encode(const Fingerprint &fingerprint)
{
	QStringList lines;
	const auto clean = [](QString s) {
		return s.replace(QLatin1Char('\t'), QLatin1Char(' ')).replace(QLatin1Char('\n'), QLatin1Char(' '));
	};
	for (const QString &f : fingerprint.folios)
		lines << QStringList{QStringLiteral("f"), f, clean(fingerprint.folio_names.value(f)),
							 QString::fromLatin1(fingerprint.folio_hashes.value(f))}
				 .join(QLatin1Char('\t'));
	QStringList keys = fingerprint.items.keys();
	keys.sort();
	for (const QString &k : std::as_const(keys)) {
		const Item &i = fingerprint.items[k];
		lines << QStringList{QString(i.kind), i.uuid, i.folio, QString::number(i.folio_index), clean(i.key),
							 QString::fromLatin1(i.hash), QString::fromLatin1(i.ends),
							 rectText(i.rect), pointsText(i.path), clean(i.name)}
				 .join(QLatin1Char('\t'));
	}
	return qCompress(lines.join(QLatin1Char('\n')).toUtf8(), 9).toBase64();
}

Fingerprint decode(const QByteArray &data)
{
	Fingerprint fp;
	if (data.trimmed().isEmpty()) return fp;
	const QString text = QString::fromUtf8(qUncompress(QByteArray::fromBase64(data.trimmed())));
	for (const QString &line : text.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
		const QStringList v = line.split(QLatin1Char('\t'));
		if (v.value(0) == QLatin1String("f")) {
			fp.folios << v.value(1);
			fp.folio_names.insert(v.value(1), v.value(2));
			fp.folio_hashes.insert(v.value(1), v.value(3).toLatin1());
			continue;
		}
		if (v.size() < 10 || v.at(0).isEmpty()) continue;
		Item i;
		i.kind = v.at(0).at(0);
		i.uuid = v.at(1);
		i.folio = v.at(2);
		i.folio_index = v.at(3).toInt();
		i.key = v.at(4);
		i.hash = v.at(5).toLatin1();
		i.ends = v.at(6).toLatin1();
		i.rect = rectFrom(v.at(7));
		i.path = pointsFrom(v.at(8));
		i.name = v.mid(9).join(QLatin1Char(' '));
		fp.items.insert(i.uuid, i);
	}
	return fp;
}

QString kindName(QChar kind)
{
	switch (kind.toLatin1()) {
		case 'e': return QObject::tr("Symbol");
		case 'c': return QObject::tr("Wire");
		case 't': return QObject::tr("Text");
		case 's': return QObject::tr("Shape");
		case 'i': return QObject::tr("Image");
		case 'f': return QObject::tr("Folio");
		default: return QString(kind);
	}
}

QString changeName(Change::Kind kind)
{
	switch (kind) {
		case Change::Added: return QObject::tr("added");
		case Change::Removed: return QObject::tr("removed");
		default: return QObject::tr("changed");
	}
}

QStringList describe(const QList<Change> &changes, const Fingerprint &now, const Fingerprint &before)
{
	QStringList lines;
	for (const Change &c : changes) {
		const Item &i = c.item();
		const QString folio = c.kind == Change::Removed ? before.folio_names.value(i.folio)
														: now.folio_names.value(i.folio);
		if (i.kind == QLatin1Char('f'))
			lines << QObject::tr("Folio %1: %2").arg(folio, c.kind == Change::Changed
													 ? QObject::tr("title block changed")
													 : changeName(c.kind));
		else
			lines << QObject::tr("Folio %1: %2 %3 %4").arg(folio, kindName(i.kind), i.name,
														   changeName(c.kind));
	}
	return lines;
}

QString nextIndex(const QString &index)
{
	const QString trimmed = index.trimmed();
	if (trimmed.isEmpty()) return QStringLiteral("A");
		//The last run of digits or letters counts up, the rest stays
	static const QRegularExpression digits(QStringLiteral("^(.*?)(\\d+)$"));
	const auto d = digits.match(trimmed);
	if (d.hasMatch()) {
		const QString number = d.captured(2);
		return d.captured(1) + QStringLiteral("%1").arg(number.toLongLong() + 1, int(number.size()),
														   10, QLatin1Char('0'));
	}
	static const QRegularExpression letters(QStringLiteral("^(.*?)([A-Za-z]+)$"));
	const auto l = letters.match(trimmed);
	if (l.hasMatch()) {
		QString word = l.captured(2);
		const bool lower = word.at(word.size() - 1).isLower();
		const QChar first = lower ? QLatin1Char('a') : QLatin1Char('A');
		const QChar last = lower ? QLatin1Char('z') : QLatin1Char('Z');
		qsizetype i = word.size() - 1;
		for ( ; i >= 0 ; --i) {
			if (word.at(i) != last) { word[i] = QChar(word.at(i).unicode() + 1); break; }
			word[i] = first;
		}
		if (i < 0) word.prepend(first);
		return l.captured(1) + word;
	}
	return trimmed + QStringLiteral("1");
}

} // namespace QetRevision

using namespace QetRevision;

namespace {
const QString &blockTag()
{
		//Was <revisions marks="…"> up to fork 1.52.0; that name is now QET's own
		//revision list (properties/revisionlist.h)
	static const QString tag = QStringLiteral("eplan_revisions");
	return tag;
}
bool startsWithTag(const QString &block, const QString &tag)
{
	return block.startsWith(QLatin1Char('<') + tag + QLatin1Char(' '))
			|| block.startsWith(QLatin1Char('<') + tag + QLatin1Char('>'))
			|| block.startsWith(QLatin1Char('<') + tag + QLatin1Char('/'));
}
bool isRevisionBlock(const QString &block)
{
	const QString b = block.trimmed();
	return startsWithTag(b, blockTag())
			|| (b.startsWith(QLatin1String("<revisions ")) && b.left(200).contains(QLatin1String(" marks=")));
}
} // namespace

ProjectRevisions::ProjectRevisions(QETProject *project) :
	QObject(project),
	m_project(project)
{
	if (m_project->undoStack())
		connect(m_project->undoStack(), &QUndoStack::indexChanged, this, [this]() { m_cache.clear(); });
}

void ProjectRevisions::reload()
{
	m_loaded = false;
	m_cache.clear();
	load();
	repaint();
}

/**
	Read the <revisions> block of the project, once
*/
void ProjectRevisions::load()
{
	if (m_loaded) return;
	m_loaded = true;
	m_entries.clear();
	m_baseline = Fingerprint();
	m_marks = false;
	for (const QString &block : m_project->dataBase()->foreignBlocks(
			 QString::fromLatin1(projectDataBase::project_owner))) {
		if (!isRevisionBlock(block)) continue;
		QDomDocument doc;
		if (!doc.setContent(block)) continue;
		const QDomElement root = doc.documentElement();
		m_marks = root.attribute(QStringLiteral("marks")) == QLatin1String("1");
		for (QDomElement r = root.firstChildElement(QStringLiteral("revision")) ; !r.isNull() ;
			 r = r.nextSiblingElement(QStringLiteral("revision"))) {
			Entry e;
			e.index = r.attribute(QStringLiteral("index"));
			e.date = r.attribute(QStringLiteral("date"));
			e.author = r.attribute(QStringLiteral("author"));
			e.text = r.attribute(QStringLiteral("text"));
			e.snapshot = r.attribute(QStringLiteral("snapshot"));
			e.pdf = r.attribute(QStringLiteral("pdf"));
			m_entries << e;
		}
		m_baseline = decode(root.firstChildElement(QStringLiteral("baseline")).text().toLatin1());
		break;
	}
}

/**
	Write the <revisions> block, keeping the project's other foreign blocks
*/
void ProjectRevisions::save()
{
	QDomDocument doc;
	QDomElement root = doc.createElement(blockTag());
	root.setAttribute(QStringLiteral("marks"), m_marks ? QStringLiteral("1") : QStringLiteral("0"));
	for (const Entry &e : std::as_const(m_entries)) {
		QDomElement r = doc.createElement(QStringLiteral("revision"));
		r.setAttribute(QStringLiteral("index"), e.index);
		r.setAttribute(QStringLiteral("date"), e.date);
		r.setAttribute(QStringLiteral("author"), e.author);
		r.setAttribute(QStringLiteral("text"), e.text);
		if (!e.snapshot.isEmpty()) r.setAttribute(QStringLiteral("snapshot"), e.snapshot);
		if (!e.pdf.isEmpty()) r.setAttribute(QStringLiteral("pdf"), e.pdf);
		root.appendChild(r);
	}
	if (!m_baseline.isEmpty()) {
		QDomElement b = doc.createElement(QStringLiteral("baseline"));
		b.appendChild(doc.createTextNode(QString::fromLatin1(encode(m_baseline))));
		root.appendChild(b);
	}
	doc.appendChild(root);
	QString text;
	QTextStream stream(&text);
	root.save(stream, -1);
	stream.flush();

	const QString owner = QString::fromLatin1(projectDataBase::project_owner);
	QStringList blocks = m_project->dataBase()->foreignBlocks(owner);
	blocks.erase(std::remove_if(blocks.begin(), blocks.end(), isRevisionBlock), blocks.end());
	blocks << text;
	m_project->dataBase()->storeForeignBlocks(owner, blocks);
	m_project->setModified(true);
	emit revisionsChanged();
}

QList<ProjectRevisions::Entry> ProjectRevisions::entries() { load(); return m_entries; }
bool ProjectRevisions::hasBaseline() { load(); return !m_baseline.isEmpty(); }
Fingerprint ProjectRevisions::baseline() { load(); return m_baseline; }
bool ProjectRevisions::marksShown() { load(); return m_marks; }

QString ProjectRevisions::lastIndex()
{
	load();
	return m_entries.isEmpty() ? QString() : m_entries.last().index;
}

/**
	@return the index to propose for the next revision: the one after the
	last, or after the folios' own when there is none yet
*/
QString ProjectRevisions::proposedIndex()
{
	load();
	if (!m_entries.isEmpty()) return nextIndex(m_entries.last().index);
	QString highest;
	for (Diagram *d : m_project->folios()) {
		const QString i = d->border_and_titleblock.exportTitleBlock().indexrev.trimmed();
		if (!i.isEmpty() && (i.size() > highest.size() || (i.size() == highest.size() && i > highest)))
			highest = i;
	}
	return highest.isEmpty() ? QStringLiteral("A") : nextIndex(highest);
}

void ProjectRevisions::setMarksShown(bool on)
{
	load();
	if (m_marks == on) return;
	m_marks = on;
	save();
	repaint();
}

void ProjectRevisions::invalidate()
{
	m_cache.clear();
}

void ProjectRevisions::repaint()
{
	for (Diagram *d : m_project->folios())
		if (d->isLoaded()) d->invalidate(QRectF(), QGraphicsScene::BackgroundLayer);
}

QList<Change> ProjectRevisions::changes()
{
	load();
	const Fingerprint before = m_comparing ? m_compare_baseline : m_baseline;
	if (before.isEmpty()) return {};
	return compare(before, fingerprint(m_project));
}

/**
	@return the changes on @p diagram since the last revision (or the
	state compared with), its folio paired by uuid; kept until the next edit
*/
QList<Change> ProjectRevisions::changesOn(Diagram *diagram)
{
	load();
	const Fingerprint &base = m_comparing ? m_compare_baseline : m_baseline;
	if (!diagram || base.isEmpty() || m_computing) return {};
	const QString folio = diagram->uuid().toString();
	const auto cached = m_cache.constFind(folio);
	if (cached != m_cache.cend()) return *cached;
	m_computing = true;
	const int index = m_project->folioIndex(diagram);
	const QDomDocument xml = diagram->toXml();
	const Fingerprint now = fingerprint(xml.documentElement(), index, diagram);
	Fingerprint before;
	QString before_folio = folio;
	if (!base.folios.contains(folio))
			//A comparison of two files: folios paired by their place
		before_folio = m_comparing ? base.folios.value(index) : QString();
	if (!before_folio.isEmpty()) {
		before.folios << before_folio;
		before.folio_names.insert(before_folio, base.folio_names.value(before_folio));
		before.folio_hashes.insert(before_folio, base.folio_hashes.value(before_folio));
		for (auto it = base.items.cbegin() ; it != base.items.cend() ; ++it)
			if (it->folio == before_folio) before.items.insert(it.key(), *it);
	}
	QList<Change> found = compare(before, now);
		//An item moved here unchanged from another folio is not new on it
	found.erase(std::remove_if(found.begin(), found.end(), [&](const Change &c) {
		return c.kind == Change::Added && c.now.kind != QLatin1Char('f')
				&& base.items.contains(c.now.uuid)
				&& base.items.value(c.now.uuid).hash == c.now.hash;
	}), found.end());
	m_cache.insert(folio, found);
	m_computing = false;
	return found;
}

QString ProjectRevisions::markLabel()
{
	return m_comparing ? m_compare_label : proposedIndex();
}

namespace {

/// A revision cloud around @p r: arcs bulging outwards
QPainterPath cloud(const QRectF &r, qreal arc)
{
	QPainterPath path;
	const QPointF corners[4] = {r.topLeft(), r.topRight(), r.bottomRight(), r.bottomLeft()};
	const QPointF outward[4] = {QPointF(0, -1), QPointF(1, 0), QPointF(0, 1), QPointF(-1, 0)};
	path.moveTo(corners[0]);
	for (int side = 0 ; side < 4 ; ++side) {
		const QPointF a = corners[side], b = corners[(side + 1) % 4];
		const qreal length = QLineF(a, b).length();
		const int n = qMax(1, qRound(length / arc));
		for (int k = 0 ; k < n ; ++k) {
			const QPointF p0 = a + (b - a) * (qreal(k) / n);
			const QPointF p1 = a + (b - a) * (qreal(k + 1) / n);
			const QPointF control = (p0 + p1) / 2 + outward[side] * (length / n) * 0.55;
			path.quadTo(control, p1);
		}
	}
	return path;
}

void triangle(QPainter *painter, const QPointF &at, const QString &label, const QColor &color)
{
	const qreal size = 14;
	QPolygonF t;
	t << QPointF(at.x(), at.y() - size * 0.9) << QPointF(at.x() + size / 2, at.y())
	  << QPointF(at.x() - size / 2, at.y());
	painter->setPen(QPen(color, 1.2));
	painter->setBrush(Qt::white);
	painter->drawPolygon(t);
	QFont font = painter->font();
	font.setPixelSize(7);
	font.setBold(true);
	painter->setFont(font);
	painter->drawText(QRectF(at.x() - size / 2, at.y() - size * 0.72, size, size * 0.72),
					  Qt::AlignHCenter | Qt::AlignBottom, label.left(3));
}

} // namespace

/**
	Draw the marks of @p diagram's changes: a cloud around a symbol, text
	or shape, a broad stroke along a wire, dashed and crossed where it was
	removed; a triangle with the revision beside each. A new folio gets
	one note instead.
*/
void ProjectRevisions::paint(Diagram *diagram, QPainter *painter)
{
	load();
	if (!(m_comparing ? m_compare_marks : m_marks)) return;
	const QList<Change> found = changesOn(diagram);
	if (found.isEmpty()) return;
	const QString label = markLabel();
	const QColor color(0xd0, 0x10, 0x10);
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	for (const Change &c : found) {
		if (c.item().kind != QLatin1Char('f') || c.kind != Change::Added) continue;
		const QRectF tb = diagram->border_and_titleblock.titleBlockRect();
		const QPointF at(tb.left() + 12, tb.top() - 6);
		triangle(painter, at, label, color);
		painter->setPen(color);
		QFont font = painter->font();
		font.setPixelSize(9);
		font.setBold(true);
		painter->setFont(font);
		painter->drawText(QPointF(at.x() + 12, at.y() - 2), tr("New folio"));
		painter->restore();
		return;
	}
	for (const Change &c : found) {
		const Item &i = c.item();
		if (i.kind == QLatin1Char('f') || i.rect.isNull()) continue;
		QPen pen(color, 1.4);
		if (c.kind == Change::Removed) pen.setStyle(Qt::DashLine);
		painter->setBrush(Qt::NoBrush);
		if (i.kind == QLatin1Char('c') && i.path.size() >= 2) {
			QPen stroke(QColor(color.red(), color.green(), color.blue(), 90), 7, pen.style(),
						Qt::RoundCap, Qt::RoundJoin);
			painter->setPen(stroke);
			painter->drawPolyline(i.path);
			const QPointF mid = (i.path.at(i.path.size() / 2 - 1) + i.path.at(i.path.size() / 2)) / 2;
			triangle(painter, mid + QPointF(9, -5), label, color);
			continue;
		}
		const QRectF r = i.rect.adjusted(-5, -5, 5, 5);
		painter->setPen(pen);
		painter->drawPath(cloud(r, 9));
		if (c.kind == Change::Removed) {
			painter->drawLine(r.topLeft() + QPointF(4, 4), r.bottomRight() - QPointF(4, 4));
			painter->drawLine(r.topRight() + QPointF(-4, 4), r.bottomLeft() + QPointF(4, -4));
		}
		triangle(painter, r.topRight() + QPointF(6, -2), label, color);
	}
	painter->restore();
}

QString ProjectRevisions::folder() const
{
	const QFileInfo info(m_project->filePath());
	return info.dir().filePath(info.completeBaseName() + QStringLiteral("_Revisionen"));
}

QString ProjectRevisions::snapshotPath(const Entry &entry) const
{
	if (entry.snapshot.isEmpty()) return QString();
	const QFileInfo info(m_project->filePath());
	return QDir::cleanPath(info.dir().filePath(entry.snapshot));
}

void ProjectRevisions::beginComparison(const Fingerprint &before, const QString &label)
{
	load();
	m_comparing = true;
	m_compare_baseline = before;
	m_compare_label = label;
	m_compare_marks = true;
	m_cache.clear();
	repaint();
}

void ProjectRevisions::endComparison()
{
	m_comparing = false;
	m_compare_baseline = Fingerprint();
	m_cache.clear();
	repaint();
}

bool ProjectRevisions::exportPdf(QETProject *project, const QString &path, QString *error)
{
	if (CLIExport::exportProjectPdf(*project, path) != 0) {
		if (error) *error = tr("The PDF %1 could not be written").arg(path);
		return false;
	}
	return true;
}

/**
	@brief ProjectRevisions::closeRevision
	Close a revision @p index with @p text, as EPLAN's "Revision erzeugen"
	and "Seiten abschließen" together: every folio's title block gets the
	index, the project its row in the revision table (rev1_* the newest);
	with @p write_pdf a PDF with the changes since the last revision marked;
	a snapshot of the project beside it; the fingerprint of what it is now,
	so that what changes next is marked; and the project saved (@p save_project).
*/
ProjectRevisions::CloseResult ProjectRevisions::closeRevision(const QString &index, const QString &text,
															   bool write_pdf, bool save_project)
{
	load();
	CloseResult result;
	const QString idx = index.trimmed();
	if (idx.isEmpty()) { result.error = tr("No revision index given"); return result; }
	if (m_project->isReadOnly()) { result.error = tr("The project is read-only"); return result; }
	if (m_project->filePath().isEmpty()) {
		result.error = tr("Save the project to a file first");
		return result;
	}
	for (const Entry &e : std::as_const(m_entries))
		if (e.index == idx) {
			result.error = tr("Revision %1 was closed already").arg(idx);
			return result;
		}

		//What changed since the last revision, before anything is stamped
	const QList<Change> found = m_baseline.isEmpty() ? QList<Change>() : changes();
	result.changes = int(found.size());

	Entry entry;
	entry.index = idx;
	entry.text = text.simplified();
	entry.date = QDate::currentDate().toString(Qt::ISODate);
	entry.author = qEnvironmentVariable("USERNAME", qEnvironmentVariable("USER"));
	m_entries << entry;

		//Title blocks: every folio and the default for new ones
	TitleBlockProperties def = m_project->defaultTitleBlockProperties();
	def.indexrev = idx;
	m_project->setDefaultTitleBlockProperties(def);
	for (Diagram *d : m_project->diagrams()) {
		TitleBlockProperties p = d->border_and_titleblock.exportTitleBlock();
		p.indexrev = idx;
		d->border_and_titleblock.importTitleBlock(p);
	}
		//The revision table, newest first: %{rev1_index} ... %{revN_text}
	DiagramContext properties = m_project->projectProperties();
	static const QRegularExpression table_key(QStringLiteral("^rev(\\d+|)_(index|date|text|author|count)$"));
	for (const QString &key : properties.keys())
		if (table_key.match(key).hasMatch()) properties.remove(key);
	for (int i = 0 ; i < m_entries.size() ; ++i) {
		const Entry &e = m_entries.at(m_entries.size() - 1 - i);
		const QString n = QString::number(i + 1);
		properties.addValue(QStringLiteral("rev%1_index").arg(n), e.index);
		properties.addValue(QStringLiteral("rev%1_date").arg(n),
							QDate::fromString(e.date, Qt::ISODate).toString(QStringLiteral("dd.MM.yyyy")));
		properties.addValue(QStringLiteral("rev%1_text").arg(n), e.text);
		properties.addValue(QStringLiteral("rev%1_author").arg(n), e.author);
	}
	properties.addValue(QStringLiteral("rev_index"), idx);
	properties.addValue(QStringLiteral("rev_date"), QDate::currentDate().toString(QStringLiteral("dd.MM.yyyy")));
	properties.addValue(QStringLiteral("rev_text"), entry.text);
	properties.addValue(QStringLiteral("rev_count"), QString::number(m_entries.size()));
	m_project->setProjectProperties(properties);

	QDir().mkpath(folder());
	const QFileInfo info(m_project->filePath());
	QString safe = idx;
	safe.replace(QRegularExpression(QStringLiteral("[^\\w\\-.]")), QStringLiteral("_"));
	const QString stem = info.completeBaseName() + QStringLiteral("_Rev_") + safe;

		//The PDF of the revision, what changed since the last one marked
	if (write_pdf) {
		const QString pdf = QDir(folder()).filePath(stem + QStringLiteral(".pdf"));
		m_comparing = true;
		m_compare_baseline = m_baseline;
		m_compare_label = idx;
		m_compare_marks = !m_baseline.isEmpty();
		m_cache.clear();
		QString error;
		const bool ok = exportPdf(m_project, pdf, &error);
		m_comparing = false;
		m_compare_baseline = Fingerprint();
		m_cache.clear();
		if (ok) {
			result.pdf = pdf;
			m_entries.last().pdf = info.dir().relativeFilePath(pdf);
		}
	}

		//What it is now, for the marks of what comes next; shown from
		//the first revision on, as EPLAN does
	if (m_entries.size() == 1) m_marks = true;
	m_baseline = fingerprint(m_project);
	const QString snapshot = QDir(folder()).filePath(stem + QStringLiteral(".qet"));
	m_entries.last().snapshot = info.dir().relativeFilePath(snapshot);
	save();
		//The snapshot carries the same block: its fingerprint is itself
	const QETResult written = m_project->writeCopyForOlderVersions(snapshot);
	if (!written.isOk()) {
		result.error = tr("The snapshot %1 could not be written: %2").arg(snapshot, written.errorMessage());
		return result;
	}
	result.snapshot = snapshot;
	if (save_project) {
		const QETResult saved = m_project->write();
		if (!saved.isOk()) {
			result.error = saved.errorMessage();
			return result;
		}
	}
	m_cache.clear();
	repaint();
	result.ok = true;
	return result;
}
