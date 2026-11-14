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
	A small subset of IEC 61131-3 Structured Text to simulate the program of
	a PLC drawn in a project:

	@code
	VAR T1 : TON; T2 : TOF; Z1 : CTU; END_VAR
	Lauf := (DI0 OR Lauf) AND NOT DI1 AND NOT T1.Q;   (* self-holding *)
	T1(IN := DI2, PT := T#1s);
	Z1(CU := DI3, R := DI1, PV := 3);
	IF Z1.Q THEN DQ1 := TRUE; ELSE DQ1 := FALSE; END_IF;
	DQ0 := Lauf;
	@endcode

	Boolean variables need no declaration; names are compared without case
	and without a leading '%'. Operators: NOT, AND (&), XOR, OR, brackets,
	TRUE/FALSE. Function blocks: TON, TOF (IN, PT, .Q, .ET), CTU (CU, R, PV,
	.Q, .CV). Comments (* ... *) and // ... are ignored.
*/
class PlcProgram
{
	public:
		PlcProgram();
		~PlcProgram();

			/// @return false and an error text with line when the source has a fault
		bool compile(const QString &source, QString *error = nullptr);
			/// runs the program once; @a vars are read and written
		void cycle(double time, QHash<QString, bool> &vars);
		bool isEmpty() const { return m_statements.isEmpty(); }

		static QString normalized(const QString &name);

		struct Block;
		struct Parser;

	private:
		QVector<std::function<void()>> m_statements;
		QHash<QString, std::shared_ptr<Block>> m_blocks;
		QHash<QString, bool> *m_vars = nullptr;
		double m_time = 0.0;
};

#endif // PLCPROGRAM_H
