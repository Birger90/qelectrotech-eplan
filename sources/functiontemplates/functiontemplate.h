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
#ifndef FUNCTIONTEMPLATE_H
#define FUNCTIONTEMPLATE_H

#include <QCoreApplication>
#include <QDomDocument>
#include <QImage>
#include <QHash>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QSet>
#include <QSize>
#include <QString>
#include <QStringList>

class Diagram;
class Element;
class QETProject;
class QGraphicsItem;

/**
	@brief A function template: a reusable piece of circuit with
	placeholders, "Motorabgang DOL", "DI-Sensor mit Kabel".

	It is a template (.qetmak, the file "Create a template" writes) with
	one block more, which says what is asked when it is placed:

	@code
	<qet_macro>
	  <function_template version="1" name="Motorabgang DOL" category="Motor"
	                     description="...">
	    <parameter name="BMK" label="Kennzeichen" kind="label" default="MA1"/>
	    <parameter name="P_kW" label="Leistung" unit="kW" default="4"/>
	    <parameter name="X" label="Klemmleiste" default="X1"/>
	    <parameter name="XN" label="erste Klemme" kind="terminal" of="X"/>
	    <table name="motor" key="P_kW">
	      <row P_kW="4" QA_ART="3RV2011-1JA10" QA_I="7...10 A"/>
	    </table>
	  </function_template>
	  <collection>...</collection>
	  <diagram_content><diagram>...</diagram></diagram_content>
	</qet_macro>
	@endcode

	Everything of the drawing may hold a placeholder, {BMK}: a label, a
	text, an element information (article data), a wire number, the
	type of a cable. {XN+2} is the value counted on by two. A table picks
	its row by its key, the smallest one which is at least the value
	(a 3 kW motor takes the 4 kW row), and its columns are placeholders
	too: the article of the breaker follows the power.

	Placing it is pasting it: the same undo step, the cable lines with
	it and fresh identities, so the template can be placed again and
	again. Its symbols keep their links to one another; the labels it
	holds without a placeholder are counted on to the next free one, as
	for any template; terminals labelled "X1:3" are filed into their
	strip; a PLC channel whose plc_address one of the PLC modules of the
	project offers is linked to that module.
*/
class FunctionTemplate
{
	Q_DECLARE_TR_FUNCTIONS(FunctionTemplate)

	public:
		struct Parameter
		{
			QString name;			///< BMK: what {BMK} stands for
			QString label;			///< what the form shows, "Kennzeichen"
			QString defaultValue;	///< MA1
			QString kind;			///< text, label, terminal, cable, number
			QString unit;			///< kW
			QString of;				///< for a terminal: the parameter naming its strip
			QString help;			///< a tool tip
			QStringList choices;	///< offered values; a table key offers its rows
		};

		struct Table
		{
			QString name;
			QString key;			///< the parameter which picks the row
			QStringList columns;	///< in file order, the key first
			QList<QMap<QString, QString>> rows;
		};

		struct Placed
		{
			bool ok = false;
			QString error;
			Diagram *folio = nullptr;
			QList<Element *> elements;
			QStringList notes;				///< what the user should know
			QMap<QString, QString> values;	///< every placeholder as it was filled
		};

		struct SaveOptions
		{
			QString name;
			QString category;
			QString description;
			QList<Parameter> parameters;
			QList<Table> tables;
				/// where to write; empty: userDirectory()/category/name.qetmak
			QString path;
		};

		FunctionTemplate() = default;

		static FunctionTemplate fromFile(const QString &path, QString *error = nullptr);
		static FunctionTemplate fromDocument(const QDomDocument &document,
											 const QString &path = QString());

		bool isValid() const {return m_valid;}
			/// A template with a function_template block, not a bare one
		bool hasParameters() const {return m_has_block;}
		bool isExample() const;
		QString path() const {return m_path;}
		QString name() const {return m_name;}
		QString category() const {return m_category;}
		QString description() const {return m_description;}
		QList<Parameter> parameters() const {return m_parameters;}
		QList<Table> tables() const {return m_tables;}
		QDomDocument document() const {return m_document;}
			/// The choices a parameter offers, its own or the rows of its table
		QStringList choicesFor(const Parameter &parameter) const;

			/// The value of each parameter as it would be proposed on
			/// @p folio: the default, a label counted on to the next free
			/// one, a terminal number after the last of its strip
		QMap<QString, QString> proposals(Diagram *folio) const;
			/// @p given completed: the proposals for what is missing,
			/// then the columns of the rows the tables pick
		QMap<QString, QString> resolve(const QMap<QString, QString> &given,
									   Diagram *folio,
									   QStringList *notes = nullptr) const;
			/// The template's drawing with every placeholder filled in
			/// and fresh identities, ready for Diagram::fromXml()
		QDomDocument instantiate(const QMap<QString, QString> &resolved,
								 Diagram *folio,
								 QStringList *notes = nullptr) const;
			/// A picture of the drawing filled with @p values
		QImage preview(const QMap<QString, QString> &values, const QSize &max) const;
			/// Put it on @p folio, its top left corner at @p pos (a null
			/// @p pos keeps the places it was saved at), one undo step
		Placed place(Diagram *folio, const QMap<QString, QString> &given,
					 QPointF pos = QPointF()) const;
			/// Put it on a new folio at the end of @p project
		Placed placeOnNewFolio(QETProject *project, const QMap<QString, QString> &given) const;

			/// Where "Save as function template" writes
		static QString userDirectory();
			/// The examples which come with QElectroTech
		static QString examplesDirectory();
			/// Every directory searched, the user's own first
		static QStringList directories();
		static QList<FunctionTemplate> available();
			/// A template by its path, its file name or its name
		static FunctionTemplate find(const QString &name_or_path);

			/// The placeholders {NAME} @p root holds, each once, in order
		static QStringList placeholdersIn(const QDomElement &root);
			/// @p text with every placeholder @p values knows filled in
		static QString substitute(const QString &text,
								  const QMap<QString, QString> &values,
								  QSet<QString> *unknown = nullptr);
			/// The row of @p table for @p value; -1 when none fits
		static int rowFor(const Table &table, const QString &value, QString *note = nullptr);

			/// Write @p items of @p folio as a function template.
			/// @return the file written, empty on failure
		static QString save(Diagram *folio, const QList<QGraphicsItem *> &items,
							const SaveOptions &options, QString *error = nullptr);
			/// The parameters to propose for @p items: one per placeholder
		static QList<Parameter> parametersFor(Diagram *folio, const QList<QGraphicsItem *> &items);

	private:
		void readBlock();
		static QDomElement blockXml(QDomDocument &document, const SaveOptions &options);
		static void linkPlcChannels(Diagram *folio, const QHash<Element *, QString> &addresses,
									QStringList *notes);

		QDomDocument m_document;
			/// Parameters made from a ?name value (MacroLabels), not {NAME}
		QStringList m_question_names;
		QString m_path;
		QString m_name;
		QString m_category;
		QString m_description;
		QList<Parameter> m_parameters;
		QList<Table> m_tables;
		bool m_valid = false;
		bool m_has_block = false;
};

#endif // FUNCTIONTEMPLATE_H
