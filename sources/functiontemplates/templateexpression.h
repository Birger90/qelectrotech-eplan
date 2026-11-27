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
#ifndef TEMPLATEEXPRESSION_H
#define TEMPLATEEXPRESSION_H

#include <QMap>
#include <QString>
#include <QStringList>

#include <functional>

/**
	@brief The small formula language of function templates: the
	conditions of their variants and their formulas (docs/konfigurator.md).

	@code
	P_kW > 5.5 and U = 400
	if(I_N = ''; lookup('motor'; P_kW; 'I_N'); I_N)
	'-QA' + text(NR; 0)
	@endcode

	Values are texts, as the placeholders are: a text which reads as a
	number (5.5 or 5,5) counts as one in arithmetic and comparison. A name
	stands for the value of that placeholder; one nobody gave is empty.
	true is "1", false is "0"; empty, "0", "false" and "nein" are false.
	Unlike FormulaBuilder (columns of a table, compiled to SQL) it is
	evaluated on the spot, on the values of one placing.
*/
namespace TemplateExpression
{
		/// What lookup('table'; value; 'column'[; 'key column']) answers:
		/// the row is picked by the table's key, or by @p key when given;
		/// the note says what the user should know (above the table, no row)
	using Lookup = std::function<QString(const QString &table, const QString &value,
										 const QString &column, const QString &key, QString *note)>;

	struct Context
	{
		const QMap<QString, QString> *values = nullptr;
		Lookup lookup;
			/// Notes of the lookups, for the report
		QStringList *notes = nullptr;
	};

		/// @return the value of @p expression; on a mistake an empty text
		/// and @p error says what is wrong where
	QString evaluate(const QString &expression, const Context &context, QString *error = nullptr);
		/// The truth of a value: empty, 0, false, nein, no are false
	bool isTrue(const QString &value);
		/// @p text as a number (comma or point, a unit after it allowed:
		/// "11,5 A"); false when it is none
	bool toNumber(const QString &text, double *value);
		/// @p value as a text without a needless .0
	QString fromNumber(double value);
}

#endif // TEMPLATEEXPRESSION_H
