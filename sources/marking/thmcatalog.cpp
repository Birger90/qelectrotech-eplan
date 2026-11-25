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
#include "thmcatalog.h"

#include <QCoreApplication>
#include <QFile>
#include <QSet>
#include <QXmlStreamReader>

namespace {

	/// An element being read, and what was found in it so far
struct Frame
{
	bool inside = false;   ///< below the MultiMark cat_item
	QString material_no;
	QString name;
	QString description;
};

bool is(const QStringView &name, const char *wanted)
{
	return name.compare(QLatin1String(wanted), Qt::CaseInsensitive) == 0;
}

} // namespace

QList<ThmCatalog::Variant> ThmCatalog::parse(const QByteArray &xml, QString *error)
{
	QList<Variant> variants;
	QSet<QString> seen;
	QXmlStreamReader r(xml);
	QList<Frame> stack;
	bool root_seen = false;
	bool multimark_seen = false;
	while (!r.atEnd()) {
		r.readNext();
		if (r.isStartElement()) {
			const QStringView name = r.name();
			if (stack.isEmpty()) {
				if (!is(name, "multiprint_catalog")) {
					if (error) *error = QCoreApplication::translate("ThmCatalog", "Not a Weidmüller catalog: the root element is <%1>, not <multiprint_catalog>.").arg(name.toString());
					return {};
				}
				root_seen = true;
			}
			const bool parent_inside = !stack.isEmpty() && stack.last().inside;
				//A field of the element above, written as child element
			if (parent_inside && (is(name, "materialno") || is(name, "name")
								  || is(name, "description"))) {
				const QString text = r.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
				Frame &parent = stack.last();
				if (is(name, "materialno")) parent.material_no = text;
				else if (is(name, "name")) parent.name = text;
				else parent.description = text;
				continue;
			}
			Frame f;
			const auto a = r.attributes();
			const bool multimark = is(name, "cat_item")
					&& a.value(QLatin1String("title")).trimmed().compare(QLatin1String("MultiMark"), Qt::CaseInsensitive) == 0;
			multimark_seen = multimark_seen || multimark;
			f.inside = parent_inside || multimark;
			if (f.inside) {
				for (const QXmlStreamAttribute &attr : a) {
					const QStringView n = attr.name();
					if (is(n, "materialno")) f.material_no = attr.value().trimmed().toString();
					else if (is(n, "name")) f.name = attr.value().trimmed().toString();
					else if (is(n, "description")) f.description = attr.value().trimmed().toString();
				}
			}
			stack << f;
		} else if (r.isEndElement() && !stack.isEmpty()) {
			const Frame f = stack.takeLast();
			if (f.inside && !f.material_no.isEmpty() && !seen.contains(f.material_no)) {
				seen.insert(f.material_no);
				variants << Variant{f.material_no, f.name, f.description};
			}
		}
	}
	if (r.hasError()) {
		if (error) *error = QCoreApplication::translate("ThmCatalog", "The catalog cannot be read (line %1): %2")
				.arg(r.lineNumber()).arg(r.errorString());
		return {};
	}
	if (error) {
		if (!root_seen) *error = QCoreApplication::translate("ThmCatalog", "The catalog file is empty.");
		else if (!multimark_seen) *error = QCoreApplication::translate("ThmCatalog", "The catalog has no entry \"MultiMark\".");
		else if (variants.isEmpty()) *error = QCoreApplication::translate("ThmCatalog", "The entry \"MultiMark\" of the catalog lists no material numbers.");
		else error->clear();
	}
	return variants;
}

QList<ThmCatalog::Variant> ThmCatalog::read(const QString &path, QString *error)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		if (error) *error = QCoreApplication::translate("ThmCatalog", "Cannot open %1: %2").arg(path, file.errorString());
		return {};
	}
	return parse(file.readAll(), error);
}

QString ThmCatalog::title(const Variant &v)
{
	if (v.name.isEmpty()) return v.material_no;
	return QStringLiteral("%1 – %2").arg(v.material_no, v.name);
}
