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
#ifndef PRODUCTRULES_H
#define PRODUCTRULES_H

#include <QCoreApplication>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

/**
	@brief The rules of a product family: a product number such as
	MS-400-7K5-D taken apart into its segments, each code of a segment a
	set of values, the codes together choosing the function template, its
	variant and more values (docs/konfigurator.md, "Produktnummer").

	@code
	{
	  "family": "MS", "name": "Motorstarter", "separator": "-",
	  "segments": [
	    {"name": "U", "label": "Spannung", "codes": {"400": {"label": "400 V", "values": {"U": "400"}}}},
	    {"name": "P", "label": "Leistung", "codes": {"7K5": {"values": {"P_kW": "7.5"}}}},
	    {"name": "OPT", "label": "Anlauf", "optional": true,
	     "codes": {"": {"label": "nach Leistung"}, "D": {"label": "Direktstart"}}}
	  ],
	  "rules": [
	    {"template": "Typicals/Antrieb"},
	    {"when": {"OPT": "D"}, "template": "Typicals/Antrieb", "variant": "Direktstart"}
	  ]
	}
	@endcode

	A file <family>.product.json in a folder of function templates. A
	number is taken apart only by its rules: a code no segment offers is an
	error, never a guess, so a number always makes the same circuit. A
	rule applies when each of its "when" codes is in the number; the rules
	without "when" apply when no other does. Two rules which apply and say
	different things are an error, not an order.
*/
class ProductRules
{
	Q_DECLARE_TR_FUNCTIONS(ProductRules)

	public:
		struct Code
		{
			QString code;
			QString label;
			QMap<QString, QString> values;	///< its value set
		};

		struct Segment
		{
			QString name;
			QString label;
			bool optional = false;	///< may be left off at the end of a number
			QList<Code> codes;
		};

		struct Rule
		{
			QMap<QString, QString> when;	///< segment -> code
			QString templ;
			QString variant;
			QMap<QString, QString> values;
		};

		struct Decoded
		{
			bool ok = false;
			QString error;
			QString number;
			QString templ;
			QString variant;
			QMap<QString, QString> values;		///< with VARIANTE when a rule names one
			QMap<QString, QString> codes;		///< segment -> code
			QList<int> rules;					///< the rules applied, by index
		};

		struct Finding
		{
			enum Severity { Error, Warning, Note };
			Severity severity = Error;
			QString text;
		};

		static ProductRules fromFile(const QString &path, QString *error = nullptr);
			/// Every rules file in the folders of function templates
		static QList<ProductRules> available();
			/// The rules of the family @p number begins with
		static ProductRules forNumber(const QString &number);
			/// Take @p number apart by the rules of its family
		static Decoded decodeNumber(const QString &number);

		bool isValid() const {return m_valid;}
		QString family() const {return m_family;}
		QString name() const {return m_name;}
		QString path() const {return m_path;}
		QList<Segment> segments() const {return m_segments;}
		QList<Rule> rules() const {return m_rules;}

		Decoded decode(const QString &number) const;
			/// The family's templates checked against its rules: every
			/// number the rules allow, worked through to the drawing.
			/// @p numbers counts the numbers tried
		QList<Finding> check(int *numbers = nullptr) const;

		static QString severityName(Finding::Severity severity);

	private:
		QString m_path;
		QString m_family;
		QString m_name;
		QString m_separator = QStringLiteral("-");
		QList<Segment> m_segments;
		QList<Rule> m_rules;
		bool m_valid = false;
};

#endif // PRODUCTRULES_H
