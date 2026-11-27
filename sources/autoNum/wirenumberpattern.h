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
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef WIRENUMBERPATTERN_H
#define WIRENUMBERPATTERN_H

#include <QHash>
#include <QList>
#include <QString>
#include <QVariantMap>

class Conductor;
class Diagram;
class QETProject;
class QUndoCommand;

/**
	@brief The WireNumberPattern namespace
	Wire numbers from a pattern one can read, beside QElectroTech's own
	numberings (NumerotationContext): "{page}.{n:2}" gives 200.01, 200.02 ...
	on folio 200.

	Tokens, German or English:
	- {seite} {page}      the folio number (title block field "folio")
	- {spalte} {column}   the column of the start of the wire
	- {zeile} {row}       the row letter of the start of the wire
	- {schrank} {cabinet} the location of the folio (+), without the +
	- {anlage} {plant}    the plant of the folio (=), without the =
	- {n}                 the counter; {n:3} at least 3 digits (001);
						  {n:A} or {n:a} in letters (A ... Z, AA ...)

	The counter starts again on each folio, in each cabinet, or runs over
	the whole project (Scope). A reserve leaves numbers free after each
	folio, so that a wire drawn later on folio 1 still gets a number of
	folio 1 while the next folios keep theirs; once the reserve is used
	up the counter jumps the numbers the next folios use. A folio is
	numbered by columns (left to right, then top down) or by rows.

	Fixed mode keeps every number a wire has and gives the wires without
	one the first free number of their folio -- also when a new wire is
	drawn (ConductorAutoNumerotation). Recalculate gives every wire its
	number anew, in order; a number that the pattern does not give (typed
	by hand: "L1", "+24V") and a frozen wire are kept.

	One number per potential: the wires joined through terminals, junctions
	and folio reports carry one number, as everywhere in QElectroTech. The
	settings are kept with the project (project properties
	"wire_number_pattern*").
*/
namespace WireNumberPattern
{
	enum class Scope { Folio, Cabinet, Project };
	enum class Order { Columns, Rows };
	enum class Mode { Fixed, Recalculate };

	struct Settings {
		bool active = false;               ///< new wires take their number from the pattern
		QString pattern = QStringLiteral("{seite}.{n:2}");
		Scope scope = Scope::Folio;
		int reserve = 0;                   ///< numbers kept free after each folio
		Order order = Order::Columns;
		Mode mode = Mode::Fixed;
		int start = 1;                     ///< the first number of a counter
	};

	Settings settings(QETProject *project);
	void setSettings(QETProject *project, const Settings &settings);
	QVariantMap toMap(const Settings &settings);
	Settings fromMap(const QVariantMap &map, const Settings &base = Settings());

		/// What is written where the tokens of a pattern stand
	struct Context {
		QString page, column, row, cabinet, plant;
	};
		/// @p pattern with its tokens replaced, the counter @p n
	QString format(const QString &pattern, const Context &context, int n);
		/// "A", "B" ... "Z", "AA" for 1, 2 ... 26, 27
	QString letters(int n);
		/// A problem of @p pattern (an unknown token, no {n}), empty if none
	QString problem(const QString &pattern);

		/// One potential and the number it gets
	struct Change {
		QList<Conductor *> wires;
		QString old_text;
		QString new_text;
	};
	struct Plan {
		QList<Change> changes;
		int potentials = 0;    ///< potentials looked at
		int kept = 0;          ///< potentials that keep their number
	};
		/// What numbering the wires of @p project by @p settings would do,
		/// nothing changed yet. @p only: the potentials of these wires only
		/// (a wire just drawn), empty for all. @p renumber: those
		/// potentials get a new number even when they have one (a number
		/// given twice).
	Plan plan(QETProject *project, const Settings &settings,
			  const QList<Conductor *> &only = {}, bool renumber = false);
		/// One undo step that does @p plan, nullptr when it changes nothing
	QUndoCommand *command(const Plan &plan, QUndoCommand *parent = nullptr);
}

#endif // WIRENUMBERPATTERN_H
