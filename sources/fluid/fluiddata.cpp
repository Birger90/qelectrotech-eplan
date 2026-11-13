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
#include "fluiddata.h"

namespace
{
	QString boolText(bool b)
	{
		return b ? QStringLiteral("true") : QStringLiteral("false");
	}

	bool textBool(const QString &s)
	{
		return s == QLatin1String("true");
	}

		/// A number that reads back as the very same double
	QString numberText(double d)
	{
		return QString::number(d, 'g', 17);
	}

	double textNumber(const QDomElement &e, const QString &name)
	{
		bool ok = false;
		const double d = e.attribute(name).toDouble(&ok);
		return (ok && d > 0.0) ? d : 0.0;
	}

	void actuationToXml(QDomDocument &document, QDomElement &parent,
						const QString &side, const FluidData::Actuation &a)
	{
		if (a.isNull())
			return;
		auto e = document.createElement(QStringLiteral("actuation"));
		e.setAttribute(QStringLiteral("side"), side);
		e.setAttribute(QStringLiteral("types"),
					   a.types.join(QLatin1Char(',')));
		if (a.spring)
			e.setAttribute(QStringLiteral("spring"), boolText(true));
		if (a.pilot)
			e.setAttribute(QStringLiteral("pilot"), boolText(true));
		if (a.air_spring)
			e.setAttribute(QStringLiteral("air_spring"), boolText(true));
		parent.appendChild(e);
	}

	FluidData::Actuation actuationFromXml(const QDomElement &e)
	{
		FluidData::Actuation a;
		a.types = e.attribute(QStringLiteral("types"))
				  .split(QLatin1Char(','), Qt::SkipEmptyParts);
		a.spring     = textBool(e.attribute(QStringLiteral("spring")));
		a.pilot      = textBool(e.attribute(QStringLiteral("pilot")));
		a.air_spring = textBool(e.attribute(QStringLiteral("air_spring")));
		return a;
	}
}

/**
	@return true when the data can be drawn and saved: a valve has one
	to max_positions positions and rests in one of them.
	Null data is valid.
*/
bool FluidData::isValid() const
{
	if (m_kind == Kind::Valve) {
		if (m_positions.isEmpty() || m_positions.size() > max_positions)
			return false;
		if (m_initial_position < 1
			|| m_initial_position > m_positions.size())
			return false;
		if (!m_dominant.isEmpty()
			&& m_dominant != QLatin1String("left")
			&& m_dominant != QLatin1String("right"))
			return false;
	}
	return true;
}

/**
	@return the <fluid> element of this data, or a null element when
	the data is null. Only what differs from the defaults is written.
*/
QDomElement FluidData::toXml(QDomDocument &document) const
{
	if (isNull())
		return QDomElement();

	auto root = document.createElement(xmlTagName());
	root.setAttribute(QStringLiteral("kind"), kindToString(m_kind));
	if (!m_prefix.isEmpty())
		root.setAttribute(QStringLiteral("prefix"), m_prefix);

	if (m_kind == Kind::Valve) {
		auto positions = document.createElement(QStringLiteral("positions"));
		positions.setAttribute(QStringLiteral("initial"), m_initial_position);
		for (const Position &p : m_positions) {
			auto pos = document.createElement(QStringLiteral("position"));
			for (const Path &path : p.paths) {
				auto xp = document.createElement(QStringLiteral("path"));
				xp.setAttribute(QStringLiteral("from"), path.from);
				if (!path.to.isEmpty())
					xp.setAttribute(QStringLiteral("to"), path.to);
				pos.appendChild(xp);
			}
			positions.appendChild(pos);
		}
		root.appendChild(positions);
		actuationToXml(document, root, QStringLiteral("left"), m_left);
		actuationToXml(document, root, QStringLiteral("right"), m_right);
		if (!m_dominant.isEmpty())
			root.setAttribute(QStringLiteral("dominant"), m_dominant);
	}

	if (m_kind == Kind::Cylinder) {
		auto cyl = document.createElement(QStringLiteral("cylinder"));
		cyl.setAttribute(QStringLiteral("action"),
			m_action == CylinderAction::Single ? QStringLiteral("single")
											   : QStringLiteral("double"));
		if (m_spring != SpringSide::None)
			cyl.setAttribute(QStringLiteral("spring"),
				m_spring == SpringSide::Rod ? QStringLiteral("rod")
											: QStringLiteral("piston"));
		if (m_cushioning != Cushioning::None)
			cyl.setAttribute(QStringLiteral("cushioning"),
				m_cushioning == Cushioning::Fixed
					? QStringLiteral("fixed") : QStringLiteral("adjustable"));
		if (m_through_rod)
			cyl.setAttribute(QStringLiteral("through_rod"), boolText(true));
		if (m_magnet_piston)
			cyl.setAttribute(QStringLiteral("magnet_piston"), boolText(true));
		if (m_bore_mm > 0.0)
			cyl.setAttribute(QStringLiteral("bore_mm"), numberText(m_bore_mm));
		if (m_rod_mm > 0.0)
			cyl.setAttribute(QStringLiteral("rod_mm"), numberText(m_rod_mm));
		if (m_stroke_mm > 0.0)
			cyl.setAttribute(QStringLiteral("stroke_mm"),
							 numberText(m_stroke_mm));
		root.appendChild(cyl);
	}

	if (m_flow_qn_nl_min > 0.0 || m_flow_c > 0.0 || m_flow_b > 0.0) {
		auto params = document.createElement(QStringLiteral("params"));
		if (m_flow_qn_nl_min > 0.0)
			params.setAttribute(QStringLiteral("qn_nl_min"),
								numberText(m_flow_qn_nl_min));
		if (m_flow_c > 0.0)
			params.setAttribute(QStringLiteral("c"), numberText(m_flow_c));
		if (m_flow_b > 0.0)
			params.setAttribute(QStringLiteral("b"), numberText(m_flow_b));
		root.appendChild(params);
	}

	return root;
}

/**
	Load from a <fluid> element. A null element, an unknown kind or
	data that is not valid leaves this object null.
	@return true when fluid data was read
*/
bool FluidData::fromXml(const QDomElement &fluid_element)
{
	*this = FluidData();
	if (fluid_element.isNull() || fluid_element.tagName() != xmlTagName())
		return false;

	FluidData d;
	d.m_kind = kindFromString(fluid_element.attribute(QStringLiteral("kind")));
	if (d.m_kind == Kind::None)
		return false;
	d.m_prefix = fluid_element.attribute(QStringLiteral("prefix")).trimmed();

	if (d.m_kind == Kind::Valve) {
		const auto positions =
			fluid_element.firstChildElement(QStringLiteral("positions"));
		d.m_initial_position =
			positions.attribute(QStringLiteral("initial"), QStringLiteral("1"))
			.toInt();
		for (auto pos = positions.firstChildElement(QStringLiteral("position"));
			 !pos.isNull();
			 pos = pos.nextSiblingElement(QStringLiteral("position"))) {
			Position p;
			for (auto xp = pos.firstChildElement(QStringLiteral("path"));
				 !xp.isNull();
				 xp = xp.nextSiblingElement(QStringLiteral("path"))) {
				const QString from = xp.attribute(QStringLiteral("from"));
				if (from.isEmpty())
					continue;
				p.paths << Path{from, xp.attribute(QStringLiteral("to"))};
			}
			d.m_positions << p;
		}
		for (auto a = fluid_element.firstChildElement(QStringLiteral("actuation"));
			 !a.isNull();
			 a = a.nextSiblingElement(QStringLiteral("actuation"))) {
			const QString side = a.attribute(QStringLiteral("side"));
			if (side == QLatin1String("left"))
				d.m_left = actuationFromXml(a);
			else if (side == QLatin1String("right"))
				d.m_right = actuationFromXml(a);
		}
		d.m_dominant = fluid_element.attribute(QStringLiteral("dominant"));
	}

	if (d.m_kind == Kind::Cylinder) {
		const auto cyl =
			fluid_element.firstChildElement(QStringLiteral("cylinder"));
		d.m_action = cyl.attribute(QStringLiteral("action"))
					 == QLatin1String("single")
				? CylinderAction::Single : CylinderAction::Double;
		const QString spring = cyl.attribute(QStringLiteral("spring"));
		if (spring == QLatin1String("rod"))
			d.m_spring = SpringSide::Rod;
		else if (spring == QLatin1String("piston"))
			d.m_spring = SpringSide::Piston;
		const QString cushioning = cyl.attribute(QStringLiteral("cushioning"));
		if (cushioning == QLatin1String("fixed"))
			d.m_cushioning = Cushioning::Fixed;
		else if (cushioning == QLatin1String("adjustable"))
			d.m_cushioning = Cushioning::Adjustable;
		d.m_through_rod =
			textBool(cyl.attribute(QStringLiteral("through_rod")));
		d.m_magnet_piston =
			textBool(cyl.attribute(QStringLiteral("magnet_piston")));
		d.m_bore_mm   = textNumber(cyl, QStringLiteral("bore_mm"));
		d.m_rod_mm    = textNumber(cyl, QStringLiteral("rod_mm"));
		d.m_stroke_mm = textNumber(cyl, QStringLiteral("stroke_mm"));
	}

	const auto params = fluid_element.firstChildElement(QStringLiteral("params"));
	if (!params.isNull()) {
		d.m_flow_qn_nl_min = textNumber(params, QStringLiteral("qn_nl_min"));
		d.m_flow_c         = textNumber(params, QStringLiteral("c"));
		d.m_flow_b         = textNumber(params, QStringLiteral("b"));
	}

	if (!d.isValid())
		return false;
	*this = d;
	return true;
}

/**
	@return the fluid data of the element whose <definition> is given,
	null when it has none
*/
FluidData FluidData::fromElementDefinition(const QDomElement &definition)
{
	FluidData d;
	d.fromXml(definition.firstChildElement(xmlTagName()));
	return d;
}

int FluidData::solenoidSides() const
{
	if (m_kind != Kind::Valve)
		return 0;
	const QString solenoid = QStringLiteral("solenoid");
	return (m_left.types.contains(solenoid) ? 1 : 0)
		+ (m_right.types.contains(solenoid) ? 1 : 0);
}

bool FluidData::operator==(const FluidData &o) const
{
	return m_kind == o.m_kind
		&& m_prefix == o.m_prefix
		&& m_positions == o.m_positions
		&& m_initial_position == o.m_initial_position
		&& m_left == o.m_left
		&& m_right == o.m_right
		&& m_dominant == o.m_dominant
		&& m_action == o.m_action
		&& m_spring == o.m_spring
		&& m_cushioning == o.m_cushioning
		&& m_through_rod == o.m_through_rod
		&& m_magnet_piston == o.m_magnet_piston
		&& m_bore_mm == o.m_bore_mm
		&& m_rod_mm == o.m_rod_mm
		&& m_stroke_mm == o.m_stroke_mm
		&& m_flow_qn_nl_min == o.m_flow_qn_nl_min
		&& m_flow_c == o.m_flow_c
		&& m_flow_b == o.m_flow_b;
}

QString FluidData::kindToString(Kind kind)
{
	switch (kind) {
		case Kind::None:        return QString();
		case Kind::Valve:       return QStringLiteral("valve");
		case Kind::Cylinder:    return QStringLiteral("cylinder");
		case Kind::Source:      return QStringLiteral("source");
		case Kind::Treatment:   return QStringLiteral("treatment");
		case Kind::FlowControl: return QStringLiteral("flow_control");
		case Kind::LogicValve:  return QStringLiteral("logic_valve");
		case Kind::Sensor:      return QStringLiteral("sensor");
		case Kind::Other:       return QStringLiteral("other");
		case Kind::Solenoid:    return QStringLiteral("solenoid");
	}
	return QString();
}

FluidData::Kind FluidData::kindFromString(const QString &string)
{
	if (string == QLatin1String("valve"))        return Kind::Valve;
	if (string == QLatin1String("cylinder"))     return Kind::Cylinder;
	if (string == QLatin1String("source"))       return Kind::Source;
	if (string == QLatin1String("treatment"))    return Kind::Treatment;
	if (string == QLatin1String("flow_control")) return Kind::FlowControl;
	if (string == QLatin1String("logic_valve"))  return Kind::LogicValve;
	if (string == QLatin1String("sensor"))       return Kind::Sensor;
	if (string == QLatin1String("other"))        return Kind::Other;
	if (string == QLatin1String("solenoid"))     return Kind::Solenoid;
	return Kind::None;
}
