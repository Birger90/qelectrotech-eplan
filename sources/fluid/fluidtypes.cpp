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
#include "fluidtypes.h"

namespace Fluid
{

QString mediumToString(Medium medium)
{
	switch (medium) {
		case Medium::Electric:  return QStringLiteral("electric");
		case Medium::Pneumatic: return QStringLiteral("pneumatic");
		case Medium::Hydraulic: return QStringLiteral("hydraulic");
		case Medium::Mechanic:  return QStringLiteral("mechanic");
		case Medium::Logic:     return QStringLiteral("logic");
	}
	return QStringLiteral("electric");
}

/**
	@return the medium @a string names. An empty or unknown word is
	Electric: that is what every older terminal is.
*/
Medium mediumFromString(const QString &string)
{
	if (string == QLatin1String("pneumatic")) return Medium::Pneumatic;
	if (string == QLatin1String("hydraulic")) return Medium::Hydraulic;
	if (string == QLatin1String("mechanic"))  return Medium::Mechanic;
	if (string == QLatin1String("logic"))     return Medium::Logic;
	return Medium::Electric;
}

QString portEndToString(PortEnd end)
{
	switch (end) {
		case PortEnd::None:     return QStringLiteral("none");
		case PortEnd::Silencer: return QStringLiteral("silencer");
		case PortEnd::Plug:     return QStringLiteral("plug");
	}
	return QStringLiteral("none");
}

PortEnd portEndFromString(const QString &string)
{
	if (string == QLatin1String("silencer")) return PortEnd::Silencer;
	if (string == QLatin1String("plug"))     return PortEnd::Plug;
	return PortEnd::None;
}

QString lineKindToString(LineKind kind)
{
	switch (kind) {
		case LineKind::None:    return QStringLiteral("none");
		case LineKind::Working: return QStringLiteral("working");
		case LineKind::Pilot:   return QStringLiteral("pilot");
		case LineKind::Exhaust: return QStringLiteral("exhaust");
	}
	return QStringLiteral("none");
}

LineKind lineKindFromString(const QString &string)
{
	if (string == QLatin1String("working")) return LineKind::Working;
	if (string == QLatin1String("pilot"))   return LineKind::Pilot;
	if (string == QLatin1String("exhaust")) return LineKind::Exhaust;
	return LineKind::None;
}

/**
	@return true when a line may join two terminals of media @a a and
	@a b: only like with like, and never a mark-only terminal.
*/
bool canConnect(Medium a, Medium b)
{
	if (a == Medium::Logic || b == Medium::Logic)
		return false;
	return a == b;
}

bool isOpenPort(Medium medium, PortEnd end, int line_count)
{
	if (medium != Medium::Pneumatic && medium != Medium::Hydraulic)
		return false;
	return end == PortEnd::None && line_count == 0;
}

/**
	The shapes follow the connector terminators of FluidSIM 6 (folder
	"Silencers and blind plugs"), at the small size, so that they fit
	on the 10 px grid of a sheet:
	- silencer: a stem, a box with three hatch strokes, a small arrow
	  for the air that leaves;
	- plug: a stem closed by a bar across it.
*/
QList<QLineF> portEndLines(PortEnd end)
{
	QList<QLineF> lines;
	switch (end) {
		case PortEnd::None:
			break;
		case PortEnd::Silencer:
				// stem
			lines << QLineF(0, 0, 5, 0);
				// box, 10 long and 6 wide
			lines << QLineF(5, -3, 15, -3) << QLineF(15, -3, 15, 3)
				  << QLineF(15, 3, 5, 3) << QLineF(5, 3, 5, -3);
				// hatch, alternately from each side to the middle
			lines << QLineF(8.5, 0, 8.5, 3) << QLineF(11, -3, 11, 0)
				  << QLineF(13.5, 0, 13.5, 3);
				// air leaving
			lines << QLineF(15, -1.5, 18.5, 0) << QLineF(18.5, 0, 15, 1.5);
			break;
		case PortEnd::Plug:
			lines << QLineF(0, 0, 5, 0) << QLineF(5, -4, 5, 4);
			break;
	}
	return lines;
}

}
