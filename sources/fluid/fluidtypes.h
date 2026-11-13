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
#ifndef FLUIDTYPES_H
#define FLUIDTYPES_H

#include <QLineF>
#include <QList>
#include <QString>

/**
	@brief Words shared by everything that draws fluid power diagrams
	(pneumatics, hydraulics): what flows through a terminal, how an
	unconnected port is closed and what a line is used for.

	Each value has one fixed word in the files. The first value of each
	enum is the one an element or a project written before fluid power
	existed has: it is never written, so those files stay as they were.
*/
namespace Fluid
{
		/// What a terminal carries
	enum class Medium {
		Electric,   ///< a wire, as every terminal before
		Pneumatic,  ///< compressed air
		Hydraulic,  ///< oil
		Mechanic,   ///< a roller, a plunger: actuated by a piston rod
		Logic       ///< a mark only, no line (coil of a solenoid valve)
	};

		/// What sits on a port no line is connected to
	enum class PortEnd {
		None,       ///< nothing: an open port
		Silencer,   ///< air leaves through a silencer
		Plug        ///< closed with a blanking plug
	};

		/// What a fluid line is used for (ISO 1219-1 draws them apart)
	enum class LineKind {
		None,       ///< not a fluid line
		Working,    ///< working line, continuous
		Pilot,      ///< pilot (control) line, dashed
		Exhaust     ///< drain or exhaust line, dashed
	};

	QString mediumToString(Medium medium);
	Medium mediumFromString(const QString &string);

	QString portEndToString(PortEnd end);
	PortEnd portEndFromString(const QString &string);

	QString lineKindToString(LineKind kind);
	LineKind lineKindFromString(const QString &string);

		/// @return true when a line may join two terminals of these media
	bool canConnect(Medium a, Medium b);

		/// @return true when a port of this medium, closed by @a end and
		/// with @a line_count lines on it, is left open: air or oil
		/// would escape there
	bool isOpenPort(Medium medium, PortEnd end, int line_count);

		/// @return the strokes of the symbol that closes a port, in a
		/// frame of its own: the port at (0, 0), x pointing away from
		/// the element (where a line would go), y across. Empty for
		/// PortEnd::None.
	QList<QLineF> portEndLines(PortEnd end);
}

#endif // FLUIDTYPES_H
