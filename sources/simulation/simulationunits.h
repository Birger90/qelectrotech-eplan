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
#ifndef SIMULATIONUNITS_H
#define SIMULATIONUNITS_H

#include <QHash>
#include <QString>

#include <cmath>

/**
	Units of the characteristic values in the symbols (<parameter unit="MPa"/>):
	every value is converted by its unit, never guessed from its size.
*/
namespace SimUnits {

	/// factor to SI and the quantity of a unit; quantity empty when the unit is unknown
struct Unit { double factor; const char *quantity; };

inline Unit unit(const QString &u)
{
	static const QHash<QString, Unit> units {
		{QStringLiteral("Pa"), {1.0, "pressure"}}, {QStringLiteral("kPa"), {1e3, "pressure"}},
		{QStringLiteral("MPa"), {1e6, "pressure"}}, {QStringLiteral("bar"), {1e5, "pressure"}},
		{QStringLiteral("mbar"), {1e2, "pressure"}},
		{QStringLiteral("m3"), {1.0, "volume"}}, {QStringLiteral("l"), {1e-3, "volume"}},
		{QStringLiteral("dm3"), {1e-3, "volume"}}, {QStringLiteral("cm3"), {1e-6, "volume"}},
		{QStringLiteral("ml"), {1e-6, "volume"}},
		{QStringLiteral("m"), {1.0, "length"}}, {QStringLiteral("mm"), {1e-3, "length"}},
		{QStringLiteral("cm"), {1e-2, "length"}},
		{QStringLiteral("m2"), {1.0, "area"}}, {QStringLiteral("cm2"), {1e-4, "area"}},
		{QStringLiteral("mm2"), {1e-6, "area"}},
		{QStringLiteral("s"), {1.0, "time"}}, {QStringLiteral("ms"), {1e-3, "time"}},
		{QStringLiteral("min"), {60.0, "time"}},
		{QStringLiteral("1/s"), {1.0, "rate"}}, {QStringLiteral("1/min"), {1.0 / 60.0, "rate"}},
		{QStringLiteral("l/min"), {1e-3 / 60.0, "flow"}}, {QStringLiteral("m3/s"), {1.0, "flow"}},
		{QStringLiteral("N"), {1.0, "force"}}, {QStringLiteral("kN"), {1e3, "force"}},
		{QStringLiteral("kg"), {1.0, "mass"}}, {QStringLiteral("g"), {1e-3, "mass"}},
		{QStringLiteral("Nm"), {1.0, "torque"}}, {QStringLiteral("N.m"), {1.0, "torque"}},
		{QStringLiteral("N.m.s/rad"), {1.0, "rotary_damping"}}, {QStringLiteral("kg.m2"), {1.0, "inertia"}},
		{QStringLiteral("l/(min.bar)"), {1e-3 / 60.0 / 1e5, "leakage"}}, {QStringLiteral("m3/(s.Pa)"), {1.0, "leakage"}},
		{QStringLiteral("rad/s"), {1.0, "angular_velocity"}}, {QStringLiteral("deg"), {M_PI / 180.0, "angle"}},
		{QStringLiteral("rad"), {1.0, "angle"}}, {QStringLiteral("N.m/rad"), {1.0, "rotary_stiffness"}},
		{QStringLiteral("m/s"), {1.0, "velocity"}}, {QStringLiteral("mm/s"), {1e-3, "velocity"}},
		{QStringLiteral("N.s/m"), {1.0, "damping"}},
		{QStringLiteral("N/m"), {1.0, "stiffness"}}, {QStringLiteral("N/mm"), {1e3, "stiffness"}},
	};
	return units.value(u.trimmed(), Unit{std::nan(""), ""});
}

	/**
		@a value given in @a from converted to @a to (both of one quantity).
		@return NaN when a unit is unknown or the quantities differ
	*/
inline double convert(double value, const QString &from, const QString &to)
{
	const Unit a = unit(from), b = unit(to);
	if (!*a.quantity || !*b.quantity || QString::fromLatin1(a.quantity) != QString::fromLatin1(b.quantity))
		return std::nan("");
	return value * a.factor / b.factor;
}

}

#endif // SIMULATIONUNITS_H
