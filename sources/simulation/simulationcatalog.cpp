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
		      "d_piston, d_rod, xMax, xStart [m]; m_cyl, m_ext [kg]; F_stat, F_extern_const [N]; F_ref [N]/v_ref [m/s]; stop_k [N/m], stop_c [N.s/m]",
		      "Endanschläge hart oder elastisch (stop_k, stop_c: Feder mit einseitiger Dämpfung), Haft-/Gleitreibung, viskose Dämpfung, Feder als Konstantkraft; keine hydraulische Endlagendämpfung, keine Leckage",
		      "CylinderCycle, Hydraulik-CylinderCycle, OpenModelica-Vergleich pneumatisch/hydraulisch, Endanschlag hydraulisch (R02), Projekt Hydraulik, Projekt Blindstopfen");
	for (const char *f : {"motor.fixed_motor", "motor.variable_motor", "actuator.rotary_motor"})
		e << physical(f, "hydraulic_motor", "hydraulic", "A (Zulauf), B (Ablauf)",
			      "displacement [m3/U], alpha (-1 … 1), GLeak [m3/(s.Pa)], I bzw. J [kg.m2], f [N.m.s/rad], M_fric [N.m], M_load bzw. M_ext [N.m]",
			      "Welle mit Trägheit, viskoser und geglätteter trockener Reibung, Leckage A→B; als Druckluftmotor (R12): Luftverbrauch ρ·D·ω/2π, ohne Leckage",
			      "Hydromotor (R08)");
	for (const char *f : {"actuator.semi_rotary", "actuator.semi_rotary_motor2"})
		e << physical(f, "rotary_actuator", "both", "A, B",
			      "M_6bar [N.m] Moment bei 6 bar, xMax, xStart [deg], I [kg.m2], f [N.m.s/rad], M_stat, M_ext [N.m]; stop_k [N.m/rad], stop_c [N.m.s/rad]",
			      "Drehanalogon des Zylinders: Schluckvolumen je Radiant M_6bar/6 bar beidseitig, Endanschläge hart oder elastisch, keine Endlagendämpfung; Wegmarken in Grad (Sensor ±1°)",
			      "Vakuum und Schwenk-/Greiferantriebe (R10)");
	e << physical("actuator.gripper", "linear_cylinder", "pneumatic", "A",
		      "d_piston, xMax [m], m_cyl [kg], F_stat [N] wie Zylinder",
		      "Ersatzmodell: einfachwirkender Zylinder mit Rückstellfeder, Greifkraft = Kolbenkraft (F:); keine Backenkinematik, kein Werkstückanschlag",
		      "Vakuum und Schwenk-/Greiferantriebe (R10)");
	e << physical("actuator.multi_position_cylinder", "linear_cylinder", "pneumatic", "C1 (hinterer Kolben), C2 (vorderer Kolben), C3 (Stangenseite)",
		      "d_piston, d_rod, xMax (Gesamthub), xMax2 (Zwischenhub, Standard xMax/2), xStart, xStart1 [m]; F_Extern [N]; Masse und Reibung wie Zylinder",
		      "zwei Kolben gleichen Durchmessers hintereinander, der hintere schiebt den vorderen mit seiner Stange (Kontakt einseitig, Stoß plastisch); "
		      "C2 wirkt auch auf das Stangenende des hinteren Kolbens; Stangenraum des hinteren Kolbens entlüftet; beide Kolben gleiche Masse und Reibung",
		      "Mehrstellungszylinder (R12)");
	e << physical("actuator.pressure_intensifier01", "pressure_intensifier", "hydraulic", "IN (Zulauf), R (Rücklauf), H (Hochdruck)",
		      "r, p_max_in [Pa], q_max_in, q_max_hi [l/min] (alle Pflicht, aus dem Datenblatt), G_leak [m3/(s.Pa)]",
		      "Mittelwertmodell des oszillierenden Pumpkolbens: q_H linear bis zum Stillstand bei r·p_IN − (r − 1)·p_R, Zulauf r·q_H, "
		      "Rücklauf (r − 1)·q_H; Bypass IN→H und entsperrbares Rückschlagventil H→IN (Steuerdruck an R) als Rückschlagventile; "
		      "keine Hubpulsation, keine Umsteuerverluste",
		      "Druckübersetzer (R12)");
	e << physical("actuator.motor_with_mid_exhaust", "air_motor", "pneumatic", "A, B (Zulauf je Drehrichtung), EX Mittelauspuff",
		      "p_N [Pa], rpm_N [1/min], M_N [N.m], n0_nN, I [kg.m2], M_ext [N.m], torqueType (1 bremsend, 2 antreibend)",
		      "lineare Momentenkennlinie aus den Nennwerten, Moment ∝ Druckdifferenz, Luftverbrauch ρ·D·ω/2π zum Auspuff; ohne Leckage und Anlaufverhalten der Lamellen",
		      "Druckluftmotoren (R12)");
	e << physical("actuator.loading_unit_2", "linear_cylinder", "hydraulic", "A1.1, A1.2 (Zylinder 1 Kolben-/Stangenseite), A2.1, A2.2 (Zylinder 2 Stangen-/Kolbenseite)",
		      "d_piston, d_rod, xMax, xStart [m]; m [kg]; alpha; F_B, F_M [N], F_V [N.s/m] wie Zylinder (R11)",
		      "zwei gegenüberliegende Zylinder auf einer Stange und einer Masse; Reibung nach R11, keine Endlagendämpfung; v_M (Stribeck) nicht gerechnet",
		      "Mobilhydraulik (R13)");
	e << physical("actuator.loading_unit_1", "linear_cylinder", "hydraulic", "A1.1, A1.2 (Zylinder 1 Kolben-/Stangenseite), A2.1, A2.2 (Zylinder 2 Stangen-/Kolbenseite)",
		      "wie Belastungseinheit 2; Druckbegrenzung zwischen den Stangenseiten p0_prv, dpEnd_prv [Pa], rT_prv, Rückschlagventil rT_cv [bar.min2/l2]",
		      "wie Belastungseinheit 2; Stangenräume A1.2 und A2.1 entlasten sich über Druckbegrenzungs- und Rückschlagventil in Reihe ineinander (Zwischenknoten nicht eigens gerechnet)",
		      "Mobilhydraulik (R13)");
	e << physical("actuator.fluidic_muscle1", "muscle", "pneumatic", "A",
		      "muscleType (1 DMSP-10, 2 DMSP-20, 3 DMSP-40), len [m], F_extern_const [N] (negativ streckt), m_ext [kg], mu_B_ext, mu_M_ext, alpha; F_V [N.s/m]",
		      "Kraft-Weg-Kennlinien aus dem Festo-Katalog DMSP (linear zwischen den Druckkurven), Volumen über virtuelle Arbeit; "
		      "negative Last streckt (Vorzeichen wie FluidSIM); Anschläge an Vorspanngrenze und max. Kontraktion; keine Hysterese, MAS ohne Kennlinie",
		      "Muskel (R12)");
	for (const char *f : {"actuator.fluidic_muscle2", "actuator.fluidic_muscle3"})
		e << other(f, Role::Unsupported, "",
			   "Muskel mit doppeltwirkendem Zylinder: FluidSIM dokumentiert die Kopplung (Zylinderhub zu Muskelkontraktion, Nullpunkt) nicht");
	// ---------------------------------------------------------- Wegeventile
	for (const char *f : {"directional_valve.two_pins", "directional_valve.three_pins", "directional_valve.four_pins",
			       "directional_valve.five_pins"})
		e << physical(f, "directional_valve", "both", "1…5, P, A, B, T, X, Y; Steueranschlüsse 12/14", VALVE_PARAMS, VALVE_LIMITS,
			      "CylinderCycle, Hydraulik-CylinderCycle, Projekt GRAFCET, Projekt Speicher");
	e << other("directional_valve.part", Role::Graphic, "", "Zeichenbaustein (Betätigung, Stellung) ohne eigene Funktion");
	for (const char *f : {"proportional.directional_valve43_01", "proportional.directional_valve43_02"})
		e << physical(f, "proportional_valve", "hydraulic", "P, A, B, T; Sollwert über gekoppelten Magneten (S) oder Verstärker (Y1/Y2, R06)",
			      "u [V] Sollwert -10 … 10, tau [s], lap bzw. lapP/lapT [%] Überdeckung, notchType (1 Rechteck, 2 Dreieck), rT [bar.min2/l2] je Kante, GLeak [l/(min.bar)] je Kante",
			      "Schieber PT1 ohne Hysterese und Strömungskraft; Kanten als Blenden mit Fläche ∝ x bzw. x²; Sollwert aus dem Signalpfad (R06: positiver Sollwert, Y2 bzw. S → P→A; Y1 → P→B), sonst Kennwert u oder Ereignis",
			      "Stetig-, Senkbrems- und Zuschaltventile (R07/R09)");
	e << physical("proportional.directional_valve63_with_hand_lever", "mobile_valve63", "hydraulic", "P1, T1 (Umlauf), P2 (über Rückschlagventil), T2, A, B",
		      "xStep [%] Auslenkung (−100 … 100), rT_P1_T1, rT_P2_AB, rT_T2_AB, rT_CV [bar.min2/l2], GLeak [l/(min.bar)], T_2 [s]",
		      "Umlauf P1→T1 mit Öffnung 1 − |x|, Arbeitskanten mit |x| (Blenden rT/Anteil²), Rückschlagventil in Reihe; Schieber PT1, Auslenkung fest für den Lauf",
		      "Mobilhydraulik (R13)");
	e << physical("proportional.mobile_valve_block_ls", "ls_valve_block", "hydraulic",
		      "P, T, LS; je Scheibe g: A g, B g, Vorsteuerung C g (X g.1) → P→A, D g (X g.2) → P→B",
		      "rT_PAB, rT_TAB, rT_pc, rT_SV [bar.min2/l2]; pS_min, pS_max [Pa] Vorsteuerbereich; p0_pc, dpEnd_pc [Pa] Druckwaage; xStepLeft/xStepRight [%] Handhebel",
		      "Schieberweg aus dem Vorsteuerdruck (bzw. Handhebel), Druckwaage quasistatisch in Reihe zur Messkante, geschlossene Mitte, Wechselventile melden den höchsten Lastdruck an LS; ohne Schieberdynamik, F_manual nicht ausgewertet",
		      "Mobilhydraulik LS (R13)");
	e << physical("proportional.priority_valve_ls", "priority_valve_ls", "hydraulic", "P, CF (bevorzugt), EF (Überschuss), LS",
		      "p0 [Pa] Federvorspannung, dpEnd [Pa], rT_CF, rT_EF [bar.min2/l2]",
		      "Kolben quasistatisch aus p_CF − p_LS gegen die Feder; Dämpfungsdrosseln rT_L, rT_R (dynamisch) nicht gerechnet",
		      "Mobilhydraulik LS (R13)");
	e << other("proportional.mpye", Role::Unsupported, "", "pneumatisches Stetigventil (Roadmap R07)");
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
	for (const char *f : {"shutoff_valve.double_non_return_valve01", "shutoff_valve.double_non_return_valve02"})
		e << physical(f, "double_check_valve", "hydraulic", "A1 → A2, B1 → B2 (frei), A2 → A1 entsperrt über B1, B2 → B1 über A1",
			      "rT [bar.min2/l2], ratio Flächenverhältnis des Entsperrkolbens, p0 [Pa] Federvorspannung (02), dpEnd [Pa] Öffnungsbereich (Vorgabe 0,5 bar)",
			      "entsperrt bei ratio·(p_Steuer − p_ein) ≥ (p_aus − p_ein) + p0, voll offen über dpEnd; ohne Kolbendynamik und Leckage",
			      "Mobilhydraulik (R13)");
	for (const char *f : {"shutoff_valve.pneumatic_counter",
			       "shutoff_valve.timer1", "shutoff_valve.timer2", "shutoff_valve.three_way_valve"})
		e << other(f, Role::Unsupported, "", "kein eigenes Modell");
	// ---------------------------------------------------------- Druckventile
	e << physical("pressure_valve.pressure_relief_valve01", "pressure_relief_valve", "both", "1/P → 2/T/3", "p0, dpEnd [Pa], rT/qNN", "direktgesteuert, ohne Dynamik", "Projekt Druckbegrenzung");
	e << physical("pressure_valve.pressure_regulator", "pressure_regulator", "both", "1/P → 2/A, Entlüftung", "p0, dpEnd [Pa], qNN/rT", "ohne Dynamik und Pilotdrain", "");
	e << physical("pressure_valve.counterbalance_valve", "counterbalance_valve", "hydraulic", "P, T, X",
		      "p0 [Pa] Öffnungsdruck, dpEnd [Pa], rT_cb bzw. rT, ratio Flächenverhältnis, rT_cv Rückschlagventil, GLeak [l/(min.bar)]",
		      "öffnet P→T bei (pP − pT) + ratio·(pX − pT) ≥ p0, Rückschlag T→P frei; ohne Kolbendynamik", "Stetig-, Senkbrems- und Zuschaltventile (R07/R09)");
	for (const char *f : {"pressure_valve.pressure_sequence_valve01", "pressure_valve.pressure_sequence_valve02"})
		e << physical(f, "sequence_valve", "hydraulic", "P, A, X (ohne X: Vorsteuerung aus P)",
			      "p0 [Pa] Öffnungsdruck, dpEnd [Pa], rT", "öffnet P→A ab Steuerdruck p0, ohne Leckölanschluss und Dynamik",
			      "Stetig-, Senkbrems- und Zuschaltventile (R07/R09)");
	// R09 (FORMELN.md V10–V13): Druckwaagen als druckabhängige Widerstände, Schock-/Nachsaugblock, pneumatisches Folgeventil
	for (const char *f : {"pressure_valve.pressure_compensator_closer01", "pressure_valve.pressure_compensator_closer02"})
		e << physical(f, "pressure_compensator", "both", "A ↔ B, Steuerung X − XS",
			      "p0 [Pa] Solldruck, dpEnd [Pa] Druckregelbereich, rT (Öl) bzw. qNN (Luft)",
			      "Schließer: voll offen bis p_X − p_XS = p0, zu bei p0 + dpEnd; ohne Kolbendynamik", "Stetig-, Senkbrems- und Zuschaltventile (R07/R09)");
	for (const char *f : {"pressure_valve.pressure_compensator_opener01", "pressure_valve.pressure_compensator_opener02"})
		e << physical(f, "pressure_compensator", "both", "A ↔ B, Steuerung X − XS",
			      "p0 [Pa] Solldruck, dpEnd [Pa] Druckregelbereich, rT (Öl) bzw. qNN (Luft)",
			      "Öffner: zu bis p_X − p_XS = p0, voll offen bei p0 + dpEnd; ohne Kolbendynamik", "Stetig-, Senkbrems- und Zuschaltventile (R07/R09)");
	for (const char *f : {"pressure_valve.pressure_compensator_upstream_1", "pressure_valve.pressure_compensator_upstream_2"})
		e << physical(f, "pressure_compensator", "hydraulic", "P → A, Steuerung A − X",
			      "p0 [Pa] Regeldifferenz, dpEnd [Pa], rT", "vorgeschaltete Druckwaage (Schließer): hält p_A − p_X bei p0; ohne Dynamik",
			      "Stetig-, Senkbrems- und Zuschaltventile (R07/R09)");
	for (const char *f : {"pressure_valve.pressure_compensator_downstream_1", "pressure_valve.pressure_compensator_downstream_2"})
		e << physical(f, "pressure_compensator", "hydraulic", "P → A, Steuerung P − X",
			      "p0 [Pa], dpEnd [Pa], rT", "nachgeschaltete Druckwaage (Öffner): öffnet, wenn p_P den Lastmeldedruck X um p0 übersteigt; ohne Dynamik",
			      "Stetig-, Senkbrems- und Zuschaltventile (R07/R09)");
	for (const char *f : {"pressure_valve.pressure_compensator_opener_ls_1", "pressure_valve.pressure_compensator_opener_ls_2"})
		e << physical(f, "pressure_compensator", "hydraulic", "P → T, Steuerung P − X",
			      "p0 [Pa] Öffnungsdruck, dpEnd [Pa], rT", "Open-Center-LS-Druckwaage: öffnet P→T bei p_P ≥ p_X + p0; ohne Dynamik",
			      "Stetig-, Senkbrems- und Zuschaltventile (R07/R09)");
	e << physical("pressure_valve.shock_and_anti_cavitation_valve", "shock_valve", "hydraulic", "A1 = A2, B1 = B2 (durchgehend), T",
		      "p0_prvA/p0_prvB [Pa], dpEndA/dpEndB [Pa], rT_prvA/rT_prvB, rT_cv [bar.min2/l2]",
		      "begrenzt A und B gegen T wie ein Druckbegrenzungsventil, saugt über Rückschlagventile von T nach; ohne Dynamik",
		      "Stetig-, Senkbrems- und Zuschaltventile (R07/R09)");
	e << physical("pressure_valve.pressure_sequence_valve", "sequence_valve", "pneumatic", "1 → 2, Steuerung 12",
		      "p0 [Pa] Schaltdruck an 12, qNN [l/min]", "öffnet 1→2 ab p12 ≥ p0 (Übergang 0,1 bar), sonst 2 abgesperrt; ohne Rückstellhysterese",
		      "Stetig-, Senkbrems- und Zuschaltventile (R07/R09)");
	e << physical("pressure_valve.pilot_valves_joystick", "pilot_joystick", "hydraulic", "P, T, X1.1, X1.2 (linke Gruppe), X2.1, X2.2 (rechte Gruppe)",
		      "adjustmentLeft/adjustmentRight [%] (−100 … 100), pMax [Pa]; Projektwerte rT 0,1 bar.min2/l2, dpEnd 0,5 bar",
		      "je Gruppe ein Druckminderventil P → X mit |Auslenkung|·pMax (+: X g.1, −: X g.2), Rest zu T; Auslenkung fest für den Lauf, ohne Dynamik",
		      "Mobilhydraulik (R13)");
	e << other("pressure_valve.pressure_compensator_closer03", Role::Unsupported, "", "Druckwaage ohne Kennwerte (closer03) (Roadmap R13)");
	// ---------------------------------------------------------- Versorgung
	e << physical("supply.air_supply", "pressure_source", "pneumatic", "alle", "pNom [Pa] (sonst Versorgungsdruck des Laufs)", "ideale Quelle ohne Liefergrenze", "CylinderCycle");
	e << physical("supply.tank", "tank", "hydraulic", "alle", "", "drucklos", "Hydraulik-CylinderCycle");
	for (const char *f : {"supply.variable_pump", "supply.pump01"})
		e << physical(f, "fixed_displacement_pump", "hydraulic", "P, T",
			      "displacement [m3], rpm [1/s], alpha (0 … 1), GL [m3/(s.Pa)], pMax [Pa]",
			      "Verstellgrad als fester Kennwert (kein Stellregler), eingebaute Begrenzung, Leckage P→T", "Hydromotor (R08)");
	e << physical("supply.pump", "fixed_displacement_pump", "both", "P, T (Hydraulik; vereinfachtes FluidSIM-Aggregat nur P2, FluidSIM Pump01 A Druck, B Saugen); alle (Pneumatik)",
		      "Hydraulik: displacement [m3], rpm [1/s], alpha, GL [m3/(s.Pa)], pMax [Pa]; Pneumatik (Verdichter): pNom [Pa]",
		      "Hydraulik: eingebaute Begrenzung (1 bar Band), keine Leckage, kein Antriebsmoment; Pneumatik: Verdichter als ideale Druckquelle",
		      "PumpFill, Hydraulik-CylinderCycle, CylinderCycle");
	e << physical("supply.accumulator", "accumulator", "both", "erster Anschluss; Speicher mit Sicherheitsblock: P, T",
		      "V0 [m3], p0 [Pa], k; mit Sicherheitsblock p0_accu [Pa] Vorfülldruck, p0_prv [Pa] Druckbegrenzung, x (2 offen, 1 abgesperrt, 0 entleert)",
		      "Gasgesetz polytrop; Luft: Behälter; Sicherheitsblock: Druckbegrenzung zum Tank mit Band und Widerstand des FluidSIM-Druckbegrenzungsventils (3,8 MPa, 0,17), Kugelhahn fest für den Lauf",
		      "AccumulatorCharge, Projekt Speicher, Lenkung und Sperrventile (R13)");
	for (const char *f : {"supply.filter", "supply.dryer", "supply.lubricator", "supply.water_separator", "supply.service_unit",
			       "supply.cooler", "supply.heater", "supply.temperature_control", "supply.startup_valve"})
		e << physical(f, "passthrough", "both", "erste zwei Anschlüsse",
			      "qNN [l/min] bzw. rT (Vorgabe 1000 l/min bzw. 0,01 bar.min2/l2); clogged 1 = verstopft mit qNN_clogged (Vorgabe 10 l/min) bzw. rT_clogged (Vorgabe 1000·rT)",
			      PASS_LIMITS, "Leitungen, Kupplungen, Aufbereitung (R05)");
	for (const char *f : {"connection.line", "supply.hose_line"})
		e << physical(f, "line", "both", "zwei Anschlüsse; Drehdurchführung paarweise je Kanal (gleiche Höhe im Symbol)",
			      "ohne Angaben ideal; rT [bar.min2/l2] bzw. qNN [l/min]: Widerstand; length, d_inner [m], roughness [m], nu [m2/s], rho [kg/m3]: Rohr",
			      "Rohr: Hagen-Poiseuille/Darcy mit Haaland-Reibzahl, Übergang 2000–4000 linear, Volumen je zur Hälfte an die Enden; Luft isotherm mit Dichte beim Mitteldruck (nur bis etwa Mach 0,3); keine Fluidträgheit, keine Wärme",
			      "Leitungen, Kupplungen, Aufbereitung (R05)");
	e << physical("connection.quick_coupling", "coupling", "both", "alle Anschlüsse",
		      "coupled (1 gekuppelt, 0 getrennt), seal (1 selbstdichtend, 0 offen), Verlust wie Leitung (rT, qNN oder Geometrie)",
		      "Zustand fest für den Lauf; getrennt und selbstdichtend: beide Hälften zu; getrennt ohne Dichtung: beide Hälften entlüften",
		      "Leitungen, Kupplungen, Aufbereitung (R05)");
	e << other("supply.manifold_c8", Role::Ideal, "ideal_connection", "verlustfreier Verteiler");
	e << physical("supply.load_sensing_pump_unit", "ls_pump_unit", "hydraulic", "P1 (LS-Verstellpumpe), P2 (Konstantpumpe), LS; T, L, LA zum Tank",
		      "rpm, displacementVP/FP [cm3], p0 [MPa] Federvorspannung (Regeldruckdifferenz), dpEnd [bar] Regelbereich, T_PT1 [s], p0_prvLS/p0_prvFixed, dpEnd_prv* [bar], rT_prv*, rT_LS, GL_VP/GL_FP [l/(min.bar)]",
		      "LS-Regler proportional mit PT1 (kein Schwenkwinkelmodell), je Pumpe Druckbegrenzung als stetige Blende und Leckage zum Tank, LS-Leitung über rT_LS entlastet; Drehzahl fest",
		      "Mobilhydraulik (R13)");
	for (const char *f : {"supply.steering_unit_1", "supply.steering_unit_2"})
		e << physical(f, "steering_unit", "hydraulic", "P, T, L, R, E (Neutralumlauf)",
			      "M_user [N.m] Handmoment (+: links), M_userMax [N.m], rT [bar.min2/l2], displacement [m3] Dosierpumpe",
			      "Drehventil öffnet mit M_user/M_userMax (Kanten rT/Anteil²), Neutral P → E; Lenkradwinkel aus dem Dosierstrom; Moment fest für den Lauf, revolutionMax und xStep nicht ausgewertet",
			      "Mobilhydraulik (R13)");
	for (const char *f : {"vacuum.vacuum_efficiency_valve",
			       "valve_group.control_block_for_two_hand_start", "valve_group.taa_01", "valve_group.tab_01",
			       "valve_group.timer3", "valve_group.timer4", "other.misc"})
		e << other(f, Role::Unsupported, "", "kein Modell (Vakuum-Sparventil, Ventilgruppen); keine Überdruckquelle als Ersatz");
	// ---------------------------------------------------------- Vakuum (R10)
	for (const char *f : {"vacuum.vacuum_generator", "vacuum.vacuum_generator_05_h", "vacuum.vacuum_generator_05_l"})
		e << physical(f, "vacuum_generator", "pneumatic", "1 (Versorgung), 2 (Vakuum)",
			      "pu_min [Pa] bestes Vakuum, dp_max [Pa] optimaler Versorgungsdruck, qn_ac [l/min] Luftverbrauch bei 600 kPa, qns_max [l/min] Saugvolumenstrom",
			      "Ejektor mit linearer Saugkennlinie zwischen pu_min (kein Fluss) und Atmosphäre (qns_max), beide mit dem Versorgungsdruck bis dp_max skaliert; über dp_max kein Abfall des Vakuums",
			      "Vakuum und Schwenk-/Greiferantriebe (R10)");
	for (const char *f : {"vacuum.suction_cup", "vacuum.vacuum_suction_cup"})
		e << physical(f, "suction_cup", "pneumatic", "erster Anschluss",
			      "d [m] Durchmesser, m [kg] Werkstückmasse, s Sicherheitsfaktor, qLeak [l/min] Leckage bei -0,6 bar, object (1 = Werkstück liegt an)",
			      "Haltekraft (p_atm - p)·π d²/4 gegen s·m·g, Volumen 5 mm tief; ohne Werkstück offen (50 l/min bei 0,1 bar); keine Verformung, keine Querkräfte",
			      "Vakuum und Schwenk-/Greiferantriebe (R10)");
	e << physical("vacuum.vacuum_switch", "pressure_switch", "pneumatic", "erster Anschluss", "p0 [Pa] (negativ), p_hyst [Pa]",
		      "schaltet unter p0 ein, über p0 + p_hyst aus; ohne Schaltzeit", "Vakuum und Schwenk-/Greiferantriebe (R10)");
	// ---------------------------------------------------------- Messen
	e << physical("sensor.pressure_switch", "pressure_switch", "both", "erster Anschluss", "p0, p_hyst [Pa]", "ohne Schaltzeit", "Projekt Speicher");
	for (const char *f : {"sensor.manometer", "measure.indicator", "sensor.pressure_sensor", "sensor.pressure_sensor1_a1_d",
			       "sensor.pressure_sensor1_a2_d", "sensor.pressure_sensor2_d", "proportional.pressure_sensor",
			       "proportional.pressure_sensor02", "sensor.flowmeter", "sensor.flowmeter03",
			       "sensor.flow_sensor1_a2_d"})
		e << other(f, Role::Display, "measuring_point", "Messstelle ohne Rückwirkung (Durchflussmesser verbinden ihre Anschlüsse); "
				   "das Spannungssignal liefert der elektrische Teil (proportional.pressure_sensor/-02: p_start, p_end, U_start, U_end, R06)");
	for (const char *f : {"proportional.flow_sensor", "sensor.flowmeter_2"})
		e << physical(f, "flow_sensor", "hydraulic", "A → B",
		      "rT [bar.min2/l2] (Standard 0,001), mode (1 Richtung ignorieren, 2 berücksichtigen), q_start, q_end [l/min], U_start, U_end [V]",
		      "Blende rT zwischen A und B, gemessen wird ihr Durchfluss; Durchflussmesser mit Schluckvolumen (displacement, f, GLeak) als Hydromotor ohne Trägheit (R13, H09); Spannung über den elektrischen Teil (R06)",
		      "Analoge Signale und Regler (R06)");
	e << physical("proportional.pressure_relief_valve", "prop_relief_valve", "hydraulic", "A (P) → B (T), Sollwert über Verstärker",
		      "p_start, p_end [Pa] Öffnungsdruck bei 0 und 10 V, dpEnd [Pa] Druckregelbereich, rT [bar.min2/l2], u [V] ohne Verstärker",
		      "direktgesteuert, Öffnungsdruck linear im Sollwert 0 … 10 V, ohne Magnet- und Kolbendynamik",
		      "Analoge Signale und Regler (R06)");
	// ---------------------------------------------------------- analoge Signalbausteine (R06): elektrische Teile,
	// sim_role "analog" mit sim_block (FluidSIM-Klasse), sim_params, sim_link; kein <fluid>-Block
	for (const auto &b : std::initializer_list<std::pair<const char *, const char *>>{
			 {"signal.waveform_generator", "Funktionsgenerator (A02): Rechteck, Sinus, Dreieck, Konstant, Profil; Ausgang S bzw. p"},
			 {"signal.set_point_generator", "Sollwertkarte (A03): W1 … W8, Rampen R1 … R4, Modus Zeit/Weiterschalten/extern I1 … I3; Ausgang Q"},
			 {"signal.controller_pid", "PID-Regler (A04): W, X → Y, KP, KI, KD, Offset, Bereich ±10 V oder 0 … 10 V, Anti-Windup"},
			 {"signal.controller_s", "Zustandsregler (A05): W, X → Y aus Abweichung, Geschwindigkeit, Beschleunigung"},
			 {"signal.controller_pt2", "PT2-Glied (A06): u → y, Bereich aus A1/A2 (A08)"},
			 {"signal.controller_integrator", "I-Glied (A07): u → y, Bereich aus A1/A2 (A08)"},
			 {"signal.differential_amplifier", "Differenzverstärker: y = u - x, Bereich aus A1/A2 (A08)"},
			 {"signal.comparator_card", "Komparator (A09): U → S mit Hysterese, S schaltet die Versorgung"},
			 {"signal.prop_amplifier", "Proportional-Verstärker 1/2 Kanäle (A10): U → gekoppeltes Stetigventil, Sprungstrom"},
			 {"signal.valve_cont_solenoid", "Proportional-Ventilmagnet, lagegeregelt: Spannung an C → Sollwert des Ventils"},
			 {"signal.pressure_sensor", "Analog-Drucksensor, elektrischer Teil (A01): Druck des Fluidteils gleicher Kennzeichnung → Y"},
			 {"signal.flow_sensor", "Analog-Durchflusssensor, elektrischer Teil (A01): Durchfluss des Fluidteils → Y"},
			 {"signal.position_sensor", "Wegmesssystem (A01): Kolbenweg des Zylinders mit pos_label → S"},
			 {"signal.voltmeter", "Voltmeter: Anzeige p - n"}})
		e << physical(b.first, "analog_block", "electric", "siehe Baustein", "siehe tools/simulation/README.md, Abschnitt R06",
			      b.second, "Analoge Signale und Regler (R06)");
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
