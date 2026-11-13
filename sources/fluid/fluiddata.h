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
#ifndef FLUIDDATA_H
#define FLUIDDATA_H

#include <QDomElement>
#include <QList>
#include <QString>
#include <QStringList>

/**
	@brief The FluidData class
	What a fluid power element is, beyond its drawing: the positions of
	a directional valve and how each side is actuated, or the build of
	a cylinder. Stored in the element definition as one <fluid> block.

	An element without that block has a null FluidData and nothing is
	written for it: elements made before stay byte for byte the same.

	The values needed to compute flows (m_flow_*) are only kept here;
	nothing reads them yet.
*/
class FluidData
{
	public:
		enum class Kind {
			None,
			Valve,       ///< directional control valve
			Cylinder,
			Source,      ///< compressed air supply, compressor, tank
			Treatment,   ///< filter, regulator, lubricator, service unit
			FlowControl, ///< throttle, one-way flow control, check valve
			LogicValve,  ///< shuttle (OR), dual pressure (AND), quick exhaust
			Sensor,      ///< pressure switch, gauge
			Other
		};

		enum class CylinderAction { Double, Single };
		enum class SpringSide { None, Rod, Piston };
		enum class Cushioning { None, Fixed, Adjustable };

			/// One flow path of a valve position: port @a from is
			/// joined to port @a to. An empty @a to means @a from is
			/// blocked in that position.
		struct Path {
			QString from;
			QString to;
			bool operator==(const Path &o) const
			{ return from == o.from && to == o.to; }
		};

			/// One position (one square of the symbol), left to right
		struct Position {
			QList<Path> paths;
			bool operator==(const Position &o) const
			{ return paths == o.paths; }
		};

			/// How one side of a valve is actuated
		struct Actuation {
				/// manual, button, lever, pedal, detent, plunger,
				/// roller, idle_roller, solenoid, pneumatic
			QStringList types;
			bool spring     = false; ///< spring return
			bool pilot      = false; ///< piloted
			bool air_spring = false; ///< pneumatic spring
			bool isNull() const
			{ return types.isEmpty() && !spring && !pilot && !air_spring; }
			bool operator==(const Actuation &o) const
			{
				return types == o.types && spring == o.spring
					&& pilot == o.pilot && air_spring == o.air_spring;
			}
		};

		static constexpr int max_positions = 4;

		FluidData() = default;

		bool isNull() const { return m_kind == Kind::None; }
		bool isValid() const;

		QDomElement toXml(QDomDocument &document) const;
		bool fromXml(const QDomElement &fluid_element);
		static FluidData fromElementDefinition(const QDomElement &definition);

		bool operator==(const FluidData &other) const;
		bool operator!=(const FluidData &other) const
		{ return !(*this == other); }

		static QString kindToString(Kind kind);
		static Kind kindFromString(const QString &string);

		static QString xmlTagName() { return QStringLiteral("fluid"); }

	public:
		Kind m_kind = Kind::None;

			// valve
		QList<Position> m_positions;
			/// Position at rest, counted from 1 at the left
		int m_initial_position = 1;
		Actuation m_left;
		Actuation m_right;
			/// Which side wins when both signals are on: "", "left", "right"
		QString m_dominant;

			// cylinder
		CylinderAction m_action     = CylinderAction::Double;
		SpringSide     m_spring     = SpringSide::None;
		Cushioning     m_cushioning = Cushioning::None;
		bool   m_through_rod    = false;
		bool   m_magnet_piston  = false;
		double m_bore_mm   = 0.0;
		double m_rod_mm    = 0.0;
		double m_stroke_mm = 0.0;

			// flow values, kept for a later simulation; 0 = not given
		double m_flow_qn_nl_min = 0.0; ///< standard nominal flow rate, l/min
		double m_flow_c         = 0.0; ///< sonic conductance C (ISO 6358)
		double m_flow_b         = 0.0; ///< critical pressure ratio b (ISO 6358)
};

#endif // FLUIDDATA_H
