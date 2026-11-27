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
#ifndef FLUIDBOMPROJECT_H
#define FLUIDBOMPROJECT_H

#include <QByteArray>

class QETProject;

namespace FluidBom
{
		/// The fluid parts list of @p project as CSV (FluidBom::csv()),
		/// gathered from every folio: the elements with fluid data and
		/// the conductors with a fluid line kind. The counts, when asked
		/// for, receive the parts, the lines and the records written.
	QByteArray projectCsv(QETProject &project, int *parts = nullptr,
						  int *lines = nullptr, int *rows = nullptr);
}

#endif // FLUIDBOMPROJECT_H
