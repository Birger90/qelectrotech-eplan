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

}
