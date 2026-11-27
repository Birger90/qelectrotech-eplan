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
#ifndef SIMATICML_H
#define SIMATICML_H

#include <QByteArray>
#include <QString>
#include <QStringList>

/**
	@brief A TIA Portal block in ladder (LAD) or function block diagram
	(FBD), exported as SimaticML XML (TIA V17 to V21), turned into the
	Structured Text the simulation runs (PlcProgram).

	A LAD/FBD network is stored as a net list (FlgNet): data (Access),
	instructions (Part), block calls (Call) and wires from one source to
	any number of sinks. The power flow is folded into boolean
	expressions, network by network:
	- Contact (negated: NOT), comparisons (Eq Ne Lt Le Gt Ge), the
	  junctions O and A, PContact/NContact and PBox/NBox (as R_TRIG and
	  F_TRIG instances);
	- Coil, SCoil, RCoil (negated coil too), Sr/Rs (the input with the 1 wins);
	- the boxes Move, Add, Sub, Mul, Div, Inc, Dec, TON, TOF, CTU, CTD
	  (with en/eno; outputs wired to variables are assigned).
	Everything else (TP, Calculate, user block calls, SCL/STL/GRAPH
	networks, system functions) is not translated: it is listed in
	@p unsupported and left as a comment, never dropped silently.

	Names: the components of a symbol joined with dots ("Data.Motor_On"),
	an absolute address as %I1.0 / %QW4; instances (timers, edges)
	get simple names (Timers_CircuitProtection).

	The format is read after the SimaticML reading guide and the
	instruction catalog of simaticml-decoder
	(https://github.com/Czarnak/simaticml-decoder, MIT); the code is our
	own.
*/
namespace SimaticMl
{
	struct Result {
		QString st;              ///< the program: VAR ... END_VAR and the statements
		QString block;           ///< name of the block
		int networks = 0, translated = 0;
		QStringList unsupported; ///< "network 3: TP not supported", one line each
		QString error;           ///< not a SimaticML block at all
	};
	Result toStructuredText(const QByteArray &xml);
	Result fileToStructuredText(const QString &path);
}

#endif // SIMATICML_H
