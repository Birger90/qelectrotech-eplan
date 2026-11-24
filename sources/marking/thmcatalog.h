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
#ifndef THMCATALOG_H
#define THMCATALOG_H

#include <QByteArray>
#include <QList>
#include <QString>

/**
	@brief The MultiMark article variants of a Weidmüller catalog file
	(my_cat*.mca) the user chooses; nothing of it is part of QElectroTech.

	Read: the root multiprint_catalog, in it the cat_item whose title is
	"MultiMark", and below it every element that carries a material
	number (materialno) together with its name and description. Each of
	the three may be an attribute or a child element of the same name,
	since only these names, not the exact form, are known; see
	docs/beschriftung-export.md.
*/
namespace ThmCatalog
{
	struct Variant
	{
		QString material_no;
		QString name;
		QString description;
	};

	QList<Variant> parse(const QByteArray &xml, QString *error = nullptr);
	QList<Variant> read(const QString &path, QString *error = nullptr);
		/// "materialno – name"
	QString title(const Variant &variant);
}

#endif // THMCATALOG_H
