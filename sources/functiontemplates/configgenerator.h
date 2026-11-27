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
#ifndef CONFIGGENERATOR_H
#define CONFIGGENERATOR_H

#include <QCoreApplication>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

class QETProject;

/**
	@brief The configurator: a plan made from a table of the objects of a
	machine, after EPLAN EEC Pro (docs/konfigurator.md).

	Each row of the table (Excel or CSV) is an object -- a drive, a valve,
	a sensor -- with the function template ("typical") it is drawn with and
	its values. The template's variants pick what is drawn, its formulas
	fill in what follows from the values, the generator counts device tags
	and PLC channels on. It draws the folios, a PLC overview whose modules
	the channels in the circuits are linked to, and the order list, the
	terminal plan and the cable list.

	Generated again from a changed table into the same project, it keeps
	what did not change, draws again what changed and was not changed by
	hand since, and leaves a folio changed by hand alone: the new one goes
	beside it and the report names the changes (unless overwrite).
*/
class ConfigGenerator
{
	Q_DECLARE_TR_FUNCTIONS(ConfigGenerator)

	public:
		struct Row
		{
			int line = 0;					///< in the file, from 1
			QMap<QString, QString> cells;	///< column name as written -> value
		};

		struct Table
		{
			QStringList columns;
			QList<Row> rows;
			QString error;
		};

		struct Options
		{
				/// Replace folios changed by hand as well
			bool overwrite = false;
		};

		struct Result
		{
			bool ok = false;
			QString error;
			QString target;
			QStringList report;		///< one line each, what happened
			QStringList conflicts;	///< folios kept because they were changed by hand
			int objects = 0;
			int created = 0;		///< folios drawn new
			int replaced = 0;		///< drawn again
			int kept = 0;			///< left as they were
			int removed = 0;
			int folios = 0;			///< folios of the project in the end
		};

			/// The table of @p path: .xlsx (first sheet) or CSV
		static Table readTable(const QString &path);

			/// Generate @p table into @p target (made when it does not exist)
			/// and save it
		static Result generate(const QString &table, const QString &target, const Options &options);
		static Result generate(const QString &table, const QString &target);
			/// Generate @p table into @p project, which is not saved
		static Result generate(const Table &table, QETProject *project, const QString &source,
							   const Options &options);

			/// The meta columns every table may have, in lower case
		static QStringList metaColumns();
};

#endif // CONFIGGENERATOR_H
