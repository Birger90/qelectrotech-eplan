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
#ifndef LOGICCHECK_H
#define LOGICCHECK_H

#include "plancheck.h"

#include <QSet>

/**
	@brief The PlanCheck::Logic namespace
	The checks of "Check plan" that read the plan as a circuit, not symbol by
	symbol (formulas and sources: tools/simulation/FORMELN.md, P01-P16):

	- checkControl(): the contactor logic as a graph of potentials. Contactors
	  whose main contacts short-circuit the supply when both are closed must
	  not be able to pull in together (EN 60204-1 9.3.4); an interlock in one
	  direction only, an interlock by push-buttons alone, a self-holding
	  without an off contact and a circuit with no stable state are reported.
	- checkPower(): the protective device against the current-carrying
	  capacity Iz of IEC 60364-5-52 annex B (when method and insulation are
	  given), the short-circuit current after IEC 60909-0 (simplified, only
	  with feed data), Zs × Ia ≤ U0 of IEC 60364-4-41 411.4.4, RCDs in series
	  and socket-outlets without an RCD of at most 30 mA (411.3.3).

	What cannot be decided from the plan is reported as a note with
	Finding::incomplete set ("Angabe fehlt"), never assumed.
*/
namespace PlanCheck
{
	namespace Logic
	{
		void checkControl(const QList<Diagram *> &diagrams, const QSet<Diagram *> &panel_folios,
						  QVector<Finding> &out);
		void checkPower(QETProject *project, const QList<Diagram *> &diagrams,
						const QSet<Diagram *> &panel_folios, QVector<Finding> &out);
	}
}

#endif // LOGICCHECK_H
