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
#ifndef PLCPROGRAM_H
#define PLCPROGRAM_H

#include <QHash>
#include <QString>
#include <QVector>
#include <functional>
#include <memory>

/**
	@brief The PlcProgram class
	A subset of IEC 61131-3 Structured Text to simulate the program of a PLC
	drawn in a project, enough for logic and step sequences:

	@code
	VAR Schritt : INT := 0; T1 : TON; F1 : R_TRIG; END_VAR
	F1(CLK := DI0);
	CASE Schritt OF
	  0: IF F1.Q THEN Schritt := 1; END_IF;
	  1: DQ0 := TRUE; IF DI2 THEN Schritt := 2; END_IF;
	  2: T1(IN := TRUE, PT := T#1s);
	     IF T1.Q THEN DQ0 := FALSE; Schritt := 0; END_IF;
	END_CASE;
	@endcode

	Values are numbers; FALSE = 0, TRUE = 1, times are seconds. Variables need
	no declaration (VAR ... END_VAR declares function blocks, INT variables and
	start values); names are compared without case and without a leading '%'.
	Operators: NOT, AND (&), XOR, OR, = <> < > <= >=, + - * / MOD, brackets.
	Statements: :=, IF/ELSIF/ELSE/END_IF, CASE/OF/ELSE/END_CASE (labels 1, 2,
	3..5). Function blocks: TON, TOF (IN, PT, .Q, .ET), CTU (CU, R, PV, .Q,
	.CV), CTD (CD, LD, PV, .Q, .CV), R_TRIG, F_TRIG (CLK, .Q), SR (S1, R, .Q1),
	RS (S, R1, .Q1). Comments (* ... *) and // ... are ignored.
*/
class PlcProgram
{
	public:
		PlcProgram();
		~PlcProgram();

			/// @return false and an error text with line when the source has a fault
		bool compile(const QString &source, QString *error = nullptr);
			/// runs the program once; @a vars are read and written
		void cycle(double time, QHash<QString, double> &vars);
		bool isEmpty() const { return m_statements.isEmpty() && !m_expression; }

			/// a single expression (GRAFCET receptivity, action condition); also R_EDGE(x), F_EDGE(x), ↑x, ↓x
		bool compileExpression(const QString &source, QString *error = nullptr);
			/// value of the expression compiled with compileExpression(); edges compare with the previous call
		double evaluate(double time, QHash<QString, double> &vars);

		static QString normalized(const QString &name);

		struct Block;
		struct Parser;

	private:
		QVector<std::function<void()>> m_statements;
		QHash<QString, std::shared_ptr<Block>> m_blocks;
		std::function<double()> m_expression;
		QHash<QString, double> m_initial;
		bool m_initialised = false;
		QHash<QString, double> *m_vars = nullptr;
		double m_time = 0.0;
};

#endif // PLCPROGRAM_H
