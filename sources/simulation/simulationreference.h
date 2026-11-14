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
#ifndef SIMULATIONREFERENCE_H
#define SIMULATIONREFERENCE_H

#include <QString>
#include <QTextStream>

/**
	Runs the reference cases of the simulation (analytical solutions, PLC
	counter, cylinder cycle) with the integration steps of SimulationModel.
	@param reference_dir folder with OpenModelica result files for the
	comparison, may be empty
	@return 0 when every check passed
*/
int runSimulationReference(const QString &reference_dir, QTextStream &out);

#endif // SIMULATIONREFERENCE_H
