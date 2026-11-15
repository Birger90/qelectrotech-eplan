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
#include "simulationcatalog.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
using Role = SimulationCatalog::Role;
using Entry = SimulationCatalog::Entry;

Entry physical(const char *f, const char *model, const char *domain, const char *ports,
	       const char *params, const char *limits, const char *tests)
{
	return Entry{QString::fromUtf8(f), Role::Physical, QString::fromUtf8(model), QString::fromUtf8(domain),
		     QString::fromUtf8(ports), QString::fromUtf8(params), QString::fromUtf8(limits),
		     QString::fromUtf8(tests)};
}

Entry other(const char *f, Role role, const char *model, const char *limits)
{
	return Entry{QString::fromUtf8(f), role, QString::fromUtf8(model), QStringLiteral("both"), QString(),
		     QString(), QString::fromUtf8(limits), QString()};
}

const char VALVE_PARAMS[] = "qNN [l/min] (Luft), rT [bar.min2/l2] (Öl), onDelay, offDelay [s]; Stellungen mit Schaltwegen";
const char VALVE_LIMITS[] = "Schieber als Stellglied 1. Ordnung, keine Über-/Unterdeckung, keine Leckage, keine Proportionalfunktion";
const char PASS_LIMITS[]  = "nur Durchgangswiderstand; Filterwirkung, Ölung, Trocknung, Sanftanlauf und Wärme werden nicht gerechnet";

QList<Entry> build()
{
	QList<Entry> e;
	// ---------------------------------------------------------- Antriebe
	e << physical("actuator.cyl_conf", "linear_cylinder", "both", "A, B",
		      "d_piston, d_rod, xMax, xStart [m]; m_cyl, m_ext [kg]; F_stat, F_extern_const [N]; F_ref [N]/v_ref [m/s]",
		      "harte Endanschläge, Haft-/Gleitreibung, viskose Dämpfung, Feder als Konstantkraft; keine Endlagendämpfung, keine Leckage",
		      "CylinderCycle, Hydraulik-CylinderCycle, OpenModelica-Vergleich pneumatisch/hydraulisch, Projekt Hydraulik, Projekt Blindstopfen");
	for (const char *f : {"actuator.gripper", "actuator.semi_rotary", "actuator.fluidic_muscle1", "actuator.fluidic_muscle3",
			       "actuator.loading_unit_2", "actuator.motor_with_mid_exhaust", "actuator.rotary_motor",
			       "actuator.semi_rotary_motor2", "motor.fixed_motor", "motor.variable_motor"})
		e << other(f, Role::Unsupported, "", "Dreh-, Greifer-, Muskel- und Motorantriebe brauchen eigene Mechanik (Roadmap R08, R10, R12); kein Linearkolben als Ersatz");
	// ---------------------------------------------------------- Wegeventile
	for (const char *f : {"directional_valve.two_pins", "directional_valve.three_pins", "directional_valve.four_pins",
			       "directional_valve.five_pins"})
		e << physical(f, "directional_valve", "both", "1…5, P, A, B, T, X, Y; Steueranschlüsse 12/14", VALVE_PARAMS, VALVE_LIMITS,
			      "CylinderCycle, Hydraulik-CylinderCycle, Projekt GRAFCET, Projekt Speicher");
	e << other("directional_valve.part", Role::Graphic, "", "Zeichenbaustein (Betätigung, Stellung) ohne eigene Funktion");
	for (const char *f : {"proportional.directional_valve43_01", "proportional.directional_valve43_02",
			       "proportional.directional_valve63_with_hand_lever", "proportional.mobile_valve_block_ls",
			       "proportional.mpye"})
		e << other(f, Role::Unsupported, "", "Stetigventile mit analogem Sollwert (Roadmap R07)");
	// ---------------------------------------------------------- Strom-, Sperr-, Logikventile
	e << physical("flow_control_valve.throttle_valve", "orifice", "both", "erste zwei Anschlüsse", "qNN [l/min] bzw. rT", "kein druckkompensierter Stromregler", "Orifice");
	e << physical("shutoff_valve.shutoff_valve", "orifice", "both", "erste zwei Anschlüsse", "qNN bzw. rT", "Stellung fest (Öffnung über qNN/rT)", "Orifice");
	e << physical("supply.silencer", "orifice_to_air", "pneumatic", "alle", "qNN [l/min]", "Geräusch nicht", "Projekt Blindstopfen");
	e << physical("shutoff_valve.check_valve", "check_valve", "both", "free_from → free_to, Steueranschluss", "p0 [Pa] Öffnungsdruck, qNN/rT", "Öffnung stetig ab p0, keine Dynamik der Kugel", "Projekt Speicher");
	e << physical("flow_control_valve.one_way_flow_control_valve", "throttle_check_valve", "both", "free_from → free_to", "qNN/rT Drossel, Rückschlag frei", "keine Druckkompensation", "");
	e << physical("flow_control_valve.flow_divider", "flow_divider", "both", "1 bzw. P → übrige", "qNN/rT", "gleiche Leitwerte, keine Druckkompensation", "");
	e << physical("logic.shuttle_valve", "shuttle_valve", "pneumatic", "Eingänge, Ausgang 2", "qNN", "", "Projekt Speicher");
	e << physical("logic.dual_pressure_valve", "dual_pressure_valve", "pneumatic", "Eingänge, Ausgang 2", "qNN", "", "Projekt Speicher");
	e << physical("logic.time_delay_valve", "time_delay_valve", "pneumatic", "1, 2, Steuerung 3", "t_delay [s], qNN", "Verzögerung als feste Zeit, kein Speichervolumen", "");
	e << physical("shutoff_valve.quick_exhaust_valve", "quick_exhaust_valve", "pneumatic", "1, 2", "qNN", "", "");
	for (const char *f : {"shutoff_valve.double_non_return_valve02", "shutoff_valve.pneumatic_counter",
			       "shutoff_valve.timer1", "shutoff_valve.timer2", "shutoff_valve.three_way_valve"})
		e << other(f, Role::Unsupported, "", "kein eigenes Modell");
	// ---------------------------------------------------------- Druckventile
	e << physical("pressure_valve.pressure_relief_valve01", "pressure_relief_valve", "both", "1/P → 2/T/3", "p0, dpEnd [Pa], rT/qNN", "direktgesteuert, ohne Dynamik", "Projekt Druckbegrenzung");
	e << physical("pressure_valve.pressure_regulator", "pressure_regulator", "both", "1/P → 2/A, Entlüftung", "p0, dpEnd [Pa], qNN/rT", "ohne Dynamik und Pilotdrain", "");
	for (const char *f : {"pressure_valve.counterbalance_valve", "pressure_valve.pilot_valves_joystick",
			       "pressure_valve.pressure_compensator_closer01", "pressure_valve.pressure_compensator_closer02",
			       "pressure_valve.pressure_compensator_downstream_1", "pressure_valve.pressure_compensator_opener_ls_1",
			       "pressure_valve.pressure_compensator_upstream_1", "pressure_valve.pressure_compensator_upstream_2",
			       "pressure_valve.pressure_sequence_valve", "pressure_valve.pressure_sequence_valve01",
			       "pressure_valve.pressure_sequence_valve02", "pressure_valve.shock_and_anti_cavitation_valve"})
		e << other(f, Role::Unsupported, "", "Last-, Folge- und Kompensatorventile (Roadmap R09, R13)");
	// ---------------------------------------------------------- Versorgung
	e << physical("supply.air_supply", "pressure_source", "pneumatic", "alle", "pNom [Pa] (sonst Versorgungsdruck des Laufs)", "ideale Quelle ohne Liefergrenze", "CylinderCycle");
	e << physical("supply.tank", "tank", "hydraulic", "alle", "", "drucklos", "Hydraulik-CylinderCycle");
	e << physical("supply.pump", "fixed_displacement_pump", "both", "P, T (Hydraulik); alle (Pneumatik)",
		      "Hydraulik: displacement [m3], rpm [1/s], pMax [Pa]; Pneumatik (Verdichter): pNom [Pa]",
		      "Hydraulik: eingebaute Begrenzung (1 bar Band), keine Leckage, kein Antriebsmoment; Pneumatik: Verdichter als ideale Druckquelle",
		      "PumpFill, Hydraulik-CylinderCycle, CylinderCycle");
	e << physical("supply.accumulator", "accumulator", "both", "erster Anschluss", "V0 [m3], p0 [Pa], k", "Gasgesetz polytrop; Luft: Behälter", "AccumulatorCharge, Projekt Speicher");
	for (const char *f : {"supply.filter", "supply.dryer", "supply.lubricator", "supply.water_separator", "supply.service_unit",
			       "supply.cooler", "supply.heater", "supply.temperature_control", "supply.startup_valve"})
		e << physical(f, "passthrough", "both", "erste zwei Anschlüsse", "qNN [l/min] bzw. rT (Vorgabe 1000 l/min bzw. 0,01 bar.min2/l2)", PASS_LIMITS, "Katalog");
	for (const char *f : {"connection.line", "connection.quick_coupling", "supply.manifold_c8", "supply.hose_line"})
		e << other(f, Role::Ideal, "ideal_connection", "verlustfreie Verbindung; Kupplung immer gekuppelt, kein Rohrmodell (Roadmap R05)");
	for (const char *f : {"supply.variable_pump", "supply.pump01", "supply.load_sensing_pump_unit", "supply.steering_unit_1",
			       "vacuum.vacuum_generator", "vacuum.vacuum_generator_05_h", "vacuum.vacuum_generator_05_l",
			       "vacuum.suction_cup", "vacuum.vacuum_suction_cup", "vacuum.vacuum_efficiency_valve", "vacuum.vacuum_switch",
			       "valve_group.control_block_for_two_hand_start", "valve_group.taa_01", "valve_group.tab_01",
			       "valve_group.timer3", "valve_group.timer4", "other.misc"})
		e << other(f, Role::Unsupported, "", "kein Modell (Verstellpumpen R08, Vakuum R10, Ventilgruppen); keine Überdruckquelle als Ersatz");
	// ---------------------------------------------------------- Messen
	e << physical("sensor.pressure_switch", "pressure_switch", "both", "erster Anschluss", "p0, p_hyst [Pa]", "ohne Schaltzeit", "Projekt Speicher");
	for (const char *f : {"sensor.manometer", "measure.indicator", "sensor.pressure_sensor", "sensor.pressure_sensor1_a1_d",
			       "sensor.pressure_sensor1_a2_d", "sensor.pressure_sensor2_d", "proportional.pressure_sensor",
			       "proportional.pressure_sensor02", "sensor.flowmeter", "sensor.flowmeter_2", "sensor.flowmeter03",
			       "sensor.flow_sensor1_a2_d", "proportional.flow_sensor"})
		e << other(f, Role::Display, "measuring_point", "Messstelle ohne Rückwirkung (Durchflussmesser verbinden ihre Anschlüsse); Ausgangssignal nicht gerechnet (Roadmap R06)");
	return e;
}
}

QString SimulationCatalog::version() { return QStringLiteral("1.0"); }

const QList<SimulationCatalog::Entry> &SimulationCatalog::entries()
{
	static const QList<Entry> all = build();
	return all;
}

const SimulationCatalog::Entry *SimulationCatalog::find(const QString &function)
{
	static const QHash<QString, int> index = []() {
		QHash<QString, int> h;
		for (int i = 0; i < entries().size(); ++i) h.insert(entries().at(i).function, i);
		return h;
	}();
	const int i = index.value(function.trimmed(), -1);
	return i < 0 ? nullptr : &entries().at(i);
}

QString SimulationCatalog::roleName(Role role)
{
	switch (role) {
		case Role::Physical:    return QStringLiteral("physical");
		case Role::Ideal:       return QStringLiteral("ideal_connection");
		case Role::Display:     return QStringLiteral("display");
		case Role::Graphic:     return QStringLiteral("graphic");
		case Role::Unsupported: return QStringLiteral("unsupported");
	}
	return QString();
}

QByteArray SimulationCatalog::json()
{
	QJsonArray list;
	for (const Entry &e : entries()) {
		QJsonObject o;
		o.insert(QStringLiteral("function"), e.function);
		o.insert(QStringLiteral("simulationRole"), roleName(e.role));
		o.insert(QStringLiteral("model"), e.model);
		o.insert(QStringLiteral("domain"), e.domain);
		o.insert(QStringLiteral("ports"), e.ports);
		o.insert(QStringLiteral("parameters"), e.parameters);
		o.insert(QStringLiteral("limits"), e.limits);
		o.insert(QStringLiteral("tests"), e.tests);
		list.append(o);
	}
	QJsonObject root;
	root.insert(QStringLiteral("catalogVersion"), version());
	root.insert(QStringLiteral("entries"), list);
	return QJsonDocument(root).toJson(QJsonDocument::Indented);
}
