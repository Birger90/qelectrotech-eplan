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

#include <QHash>
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
			///The strip the terminal goes into, "X1"; empty: Settings::strip
		QString strip;
			///The number the terminal gets, "3" for "X1:3". Rows of one
			///strip with the same number are the levels of one terminal.
		QString number;
			///The level of its terminal, 0 = lowest (0 for a terminal of one level)
		int level = 0;
			///"Phoenix Contact 3213946"; empty: Settings::article
		QString article;
			///A protective earth wire
		bool pe = false;
	};

	struct Settings
	{
		QString strip;			///< "X1", for rows which name none
		QString element_path;	///< collection path of the terminal symbol
		QString article;		///< "WAGO 2002-1201": manufacturer, a space, the reference
		QString pe_article;		///< article of a PE terminal
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
		///The level of a terminal of several levels which is PE, -1 when none
	int peLevelOf(const QString &article);
		///What the level of a terminal is called: "PE", "L", "N" where
		///the type says so, "1", "2", ... otherwise, empty for one level
	QString levelName(const QString &article, int level);
		///The article a row gets when nobody chose one: the terminal's
		///own article, the PE article for PE wires unless the terminal
		///has a PE level
	QString defaultArticle(const Row &row, const Settings &settings);
		///Number the rows strip by strip: every strip from @p first (its
		///next free number when it has no entry there). Rows of one strip
		///with an article of several levels fill the levels of one terminal
		///in turn, a PE wire the PE level; a wire whose level is taken
		///opens the next terminal.
	void numberRows(QETProject *project, QList<Row> &rows,
					const QHash<QString, int> &first = {});

		///Put the terminals down, one undo step, and file them into the strip
	Result place(Diagram *diagram, const QLineF &line,
				 const QList<Row> &rows, const Settings &settings);
}

#endif // DRAWNTERMINALS_H
