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
#ifndef FLUIDBOM_H
#define FLUIDBOM_H

#include "fluiddata.h"
#include "fluidtypes.h"

#include <QByteArray>
#include <QList>
#include <QString>

/**
	@brief Parts list of the fluid power side of a project: valves,
	cylinders, sources... with their characteristics, and the lines
	(hoses, pipes) counted by kind and size.

	The functions here only group and write: what is in the project is
	gathered by the caller (cli_export.cpp), so they can be tested
	without a project.

	A diagram is not drawn to scale, so a line's length is what was
	typed into it (ConductorProperties::m_length). The list adds up the
	lengths of each kind and size, says how many lines have none, and
	leaves the length to be measured when no line of it has one.
*/
namespace FluidBom
{
		/// One fluid element of the project
	struct Part {
		QString name;          ///< the element's name
		QString kind;          ///< kindText() of its fluid data
		QString details;       ///< details() of its fluid data
		QString manufacturer;  ///< element information "manufacturer"
		QString reference;     ///< element information "manufacturer_reference"
		QString label;         ///< its label (-QM1), may be empty
	};

		/// One fluid line (conductor) of the project
	struct Line {
		Fluid::LineKind kind = Fluid::LineKind::None;
		QString section;       ///< the conductor's section field (hose size)
		QString length;        ///< its length as typed in, may be empty
	};

	QString kindText(FluidData::Kind kind);
	QString lineKindText(Fluid::LineKind kind);
	QString details(const FluidData &data);

		/// Reads a length as typed into a line: a number with "," or "."
		/// for decimals and a unit mm, cm or m; without a unit, metres.
		/// @return false when @p text is empty or not such a length
	bool lengthInMm(const QString &text, double *mm);

		/// The list as CSV (BomExport::csvRecord): a header, then one
		/// record per identical part, then one per kind and size of
		/// line with the sum of their lengths. @p rows, when given,
		/// receives the number of records after the header.
	QByteArray csv(const QList<Part> &parts, const QList<Line> &lines,
				   int *rows = nullptr);
}

#endif // FLUIDBOM_H
