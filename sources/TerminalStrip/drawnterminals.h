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
#ifndef DRAWNTERMINALS_H
#define DRAWNTERMINALS_H

#include <QLineF>
#include <QList>
#include <QPointF>
#include <QString>
#include <QStringList>

class Conductor;
class Diagram;
class QETProject;

/**
	@brief Terminals laid down by a line drawn across conductors.

	The counterpart of "draw a cable": the user draws one straight line
	over the wires which go through a terminal strip, and every wire the
	line crosses gets a terminal at the crossing. The terminal symbol is
	turned so its two connection points lie on the wire, the wire is
	broken there and runs through the terminal, and the terminal is
	labelled "<strip>:<number>" so that the terminal strip of the project
	is built from it straight away (TerminalStripAutoImport).
*/
namespace DrawnTerminals
{
	struct Row
	{
		Conductor *conductor = nullptr;
			///Scene coordinates where the line crosses the conductor
		QPointF position;
			///The number the terminal gets, "3" for "X1:3"
		QString number;
			///A protective earth terminal: gets the PE article
		bool pe = false;
	};

	struct Settings
	{
		QString strip;			///< "X1"
		QString element_path;	///< collection path of the terminal symbol
		QString article;		///< "WAGO 2002-1201": manufacturer, a space, the reference
		QString pe_article;		///< article of a PE terminal
			///Levels of one terminal: with 3 (an installation terminal
			///PE/L/N) every 3 wires share one terminal, one article, one
			///number, and pe_article is not used
		int levels = 1;
	};

	struct Result
	{
		int placed = 0;			///< terminals put on the folio
		int unconnected = 0;	///< terminals which did not land on their wire
		QStringList problems;	///< what went wrong, for the user
	};

		///The rows of a line: one per conductor, in the direction it was drawn
	QList<Row> rowsFor(Diagram *diagram, const QLineF &line);
		///The strip names the project already uses, sorted
	QStringList stripNames(QETProject *project);
		///The first "X<n>" no strip and no terminal of the project uses yet
	QString nextFreeStrip(QETProject *project);
		///The number after the highest one the strip already holds
	int nextNumber(QETProject *project, const QString &strip);
		///Whether a conductor looks like a protective earth wire
	bool looksLikePe(const Conductor *conductor);
		///What the dialog shows for a conductor: its number, potential or colour
	QString conductorText(const Conductor *conductor);
		///The symbol used when the user never chose one
	QString defaultElementPath();
		///How many levels the article of the catalogue has (1 when unknown)
	int levelsOf(const QString &article);
		///Number the rows from @p first: one number per terminal, that is
		///per @p levels rows
	void numberRows(QList<Row> &rows, int first, int levels);

		///Put the terminals down, one undo step, and file them into the strip
	Result place(Diagram *diagram, const QLineF &line,
				 const QList<Row> &rows, const Settings &settings);
}

#endif // DRAWNTERMINALS_H
