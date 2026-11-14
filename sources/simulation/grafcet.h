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
#ifndef GRAFCET_H
#define GRAFCET_H

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <functional>
#include <memory>

class Diagram;
class Element;
class PlcProgram;
class Terminal;

/**
	@brief The Grafcet class
	Runs the GRAFCET charts drawn with the GRAFCET symbols of the collection
	(20_logic/2020_flow_chart/202020_grafcet) by the rules of IEC 60848:

	- the initial steps are active at the start;
	- a transition is enabled when all steps before it are active and fires
	  when its receptivity is true; all fireable transitions fire at once,
	  the steps before them are deactivated, the steps after them activated
	  (activation wins);
	- this repeats until the situation is stable (transient evolution);
	  continuous actions act in the stable situation only, stored actions
	  also on transient steps.

	Steps carry their number as label (variable X1, activity time X1.T in s).
	The receptivity is the label of the transition, an expression of the
	PLC variables (DI0, symbols), step variables, element labels (-BG1 =
	the push button or limit switch is actuated) and times (2s/X1). Actions:
	continuous "DQ0, -QM1" (true while the step is active, -QM1 / -QM1/14 /
	-QM1/12 actuate a valve), conditional (condition above the box), stored
	on activation, on deactivation, on event (Structured Text statements such
	as "Z := Z + 1").
*/
class Grafcet
{
	public:
		Grafcet(const QList<Diagram *> &diagrams, const std::function<int(Terminal *)> &node);
		~Grafcet();

		static bool isGrafcet(const Element *element);
			/// variable of an element label in receptivities and actions: "-QM1/14" -> "BMK_QM1__14"
		static QString labelVariable(const QString &label);
			/// GRAFCET notation to Structured Text: -BG1, 2s/X1, t/X1/2s, ·, ∧, ∨, ¬, /a, =1
		static QString toStructuredText(const QString &text);

		bool isEmpty() const { return m_steps.isEmpty(); }
		QStringList errors() const { return m_errors; }
		int stepCount() const { return static_cast<int>(m_steps.size()); }
		int transitionCount() const { return static_cast<int>(m_transitions.size()); }

			/// one cycle at @a time: reads and writes @a vars, appends messages
		void cycle(double time, QHash<QString, double> &vars, QStringList *messages);

		QList<Element *> activeSteps() const;

	private:
		struct Step;
		struct Transition;
		struct Action;
		void activate(Step *s, double time, QHash<QString, double> &vars, QStringList *messages);
		void deactivate(Step *s, double time, QHash<QString, double> &vars);

		QList<Step *> m_steps;
		QList<Transition *> m_transitions;
		QList<Action *> m_actions;
		QStringList m_continuous;        // variables written by continuous actions
		QStringList m_errors;
		bool m_started = false;
		bool m_unstable_reported = false;
};

#endif // GRAFCET_H
