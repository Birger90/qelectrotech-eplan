// Builds one small test project for the checks of "Check plan" (sources/plancheck/logiccheck.cpp).
// tools/plancheck/pruefen_logik.py puts in front of this file: var CASE = "..."; var OUT = "...";
// The project the script runs on is an empty one; the result is saved to OUT.

var C = "common://10_electric/10_allpole/";
var COIL = C + "310_relays_contactors_contacts/01_coils/bobine3.elmt";
var NO = C + "310_relays_contactors_contacts/02_contacts_cross_referencing/01_auxiliary_contacts/con_simple.elmt";
var NC = C + "310_relays_contactors_contacts/02_contacts_cross_referencing/01_auxiliary_contacts/con_simple_nf.elmt";
	// 3 main contacts with the terminals 1-2, 3-4, 5-6 (the script's terminal
	// order is not the file's: always address a terminal by its name)
var PWR = C + "310_relays_contactors_contacts/02_contacts_cross_referencing/02_power_contacts/com_puiss_3P_inv.elmt";
var BTN = C + "380_signaling_operating/20_push_buttons/poussoir.elmt";
var BTN_NC = C + "380_signaling_operating/20_push_buttons/poussoir_nf.elmt";
var MCB1 = C + "200_fuses_protective_gears/11_circuit_breakers/disjonct-m_1f.elmt";
var MCB3 = C + "200_fuses_protective_gears/11_circuit_breakers/disjonct-m_3f.elmt";
	// an RCD (RCBO) drawn with the breaker symbol, for its named terminals IN.1/OUT.2
var RCD = MCB1;
var FEED = C + "110_network_supplies/src_1pn.elmt";
var SOCKET = C + "140_connectors_plugs/20_socket_outlets/pc_mono.elmt";
var MOTOR = C + "391_consumers_actuators/10_engines/moteur_tri_3.elmt";

var F = 0;   // the folio

function el(path, x, y, label) {
	var u = qet.addElement(F, path, x, y);
	if (!u) throw "cannot place " + path;
	if (label) qet.setElementLabel(F, u, label);
	return u;
}
	// index of the terminal called name ("IN.1", "A1")
function t(u, name) {
	var list = qet.elementTerminals(F, u);
	for (var i = 0; i < list.length; ++i)
		if (list[i].split(" ")[1] === name) return i;
	throw "no terminal " + name;
}
function w(a, ia, b, ib) {
	if (!qet.addConductor(F, a, ia, b, ib)) throw "cannot wire";
}
function link(coil, contact) {
	if (!qet.linkElements(F, coil, F, contact)) throw "cannot link";
}
function info(u, key, value) {
	if (!qet.setElementInfo(F, u, key, value)) throw "cannot set " + key;
}

	// the control fuse: its output is the plus rail
function controlFuse() {
	var f = el(MCB1, 60, 60, "-F9");
	return {u: f, i: t(f, "OUT.2")};
}

	// a coil branch: plus -> [stop NC] -> start || hold -> [NC of nc_of] [own NC] -> coil A1
function branch(plus, label, opts) {
	var x = opts.x;
	var coil = opts.coil || el(COIL, x, 520, label);
	var top = plus;
	if (opts.stop) {
		var s0 = el(BTN_NC, x, 140, "-S0" + label.substr(2));
		w(top.u, top.i, s0, 0);
		top = {u: s0, i: 1};
	}
	var node = null;
	if (opts.start) {
		var s1 = el(BTN, x, 240, "-S1" + label.substr(2));
		w(top.u, top.i, s1, 0);
		node = {u: s1, i: 1};
	}
	if (opts.hold) {
		var h = el(NO, x + 40, 240, null);
		link(coil, h);
		w(top.u, top.i, h, 0);
		if (node) w(node.u, node.i, h, 1); else node = {u: h, i: 1};
	}
	if (!node) node = top;
	if (opts.nc_of) {
		var nc = el(NC, x, 380, null);
		link(opts.nc_of, nc);
		w(node.u, node.i, nc, 0);
		node = {u: nc, i: 1};
	}
	if (opts.own_nc) {
		var own = el(NC, x, 380, null);
		link(coil, own);
		w(node.u, node.i, own, 0);
		node = {u: own, i: 1};
	}
	w(node.u, node.i, coil, t(coil, "A1"));
	return coil;
}

	// main contacts of a coil, inputs and outputs [{u, i}, ...] per pole
function poles(coil, x, ins, outs) {
	var p = el(PWR, x, 900, null);
	link(coil, p);
	for (var k = 0; k < 3; ++k) {
		if (ins[k]) w(ins[k].u, ins[k].i, p, t(p, String(2 * k + 1)));
		if (outs[k]) w(p, t(p, String(2 * k + 2)), outs[k].u, outs[k].i);
	}
	return p;
}
function pole(p, k, side) { return {u: p, i: t(p, String(2 * k + (side === "out" ? 2 : 1)))}; }

	// reversing contactors K1/K2; with1: NC of K1 in K2's circuit, with2: NC of K2 in K1's
function reversingCase(with1, with2) {
	var plus = controlFuse();
	var k1 = el(COIL, 200, 520, "-K1");
	var k2 = el(COIL, 400, 520, "-K2");
	branch(plus, "-K1", {coil: k1, stop: true, start: true, hold: true, nc_of: with2 ? k2 : null, x: 200});
	branch(plus, "-K2", {coil: k2, stop: true, start: true, hold: true, nc_of: with1 ? k1 : null, x: 400});
	w(k1, t(k1, "A2"), k2, t(k2, "A2"));
	var q = el(MCB3, 300, 760, "-Q1");
	info(q, "description", "C16");
	var l1 = {u: q, i: t(q, "OUT.2")}, l2 = {u: q, i: t(q, "OUT.4")}, l3 = {u: q, i: t(q, "OUT.6")};
	var p1 = poles(k1, 200, [l1, l2, l3], [null, null, null]);
		// K2 swaps L1 and L2 onto the same motor terminals
	poles(k2, 400, [l2, l1, l3], [pole(p1, 0, "out"), pole(p1, 1, "out"), pole(p1, 2, "out")]);
}

	// star-delta: K1 line, K2 star, K3 delta; interlocked: NC of K3 before K2, NC of K2 before K3
function starDeltaCase(interlocked) {
	var plus = controlFuse();
	var k1 = el(COIL, 200, 520, "-K1");
	var k2 = el(COIL, 400, 520, "-K2");
	var k3 = el(COIL, 600, 520, "-K3");
	branch(plus, "-K1", {coil: k1, stop: true, start: true, hold: true, x: 200});
	var h1 = el(NO, 300, 300, null);   // a further NO of K1 feeds K2 and K3
	link(k1, h1);
	w(plus.u, plus.i, h1, 0);
	branch({u: h1, i: 1}, "-K2", {coil: k2, nc_of: interlocked ? k3 : null, x: 400});
	branch({u: h1, i: 1}, "-K3", {coil: k3, nc_of: interlocked ? k2 : null, x: 600});
	w(k1, t(k1, "A2"), k2, t(k2, "A2"));
	w(k2, t(k2, "A2"), k3, t(k3, "A2"));
	var q = el(MCB3, 200, 760, "-Q1");
	info(q, "description", "C16");
	var l = [{u: q, i: t(q, "OUT.2")}, {u: q, i: t(q, "OUT.4")}, {u: q, i: t(q, "OUT.6")}];
		// K1: L -> U1 V1 W1; K3 (delta): U1 V1 W1 -> W2 U2 V2; K2 (star): U2 V2 W2 bridged
	var p1 = poles(k1, 200, l, [null, null, null]);
	var p3 = poles(k3, 600, [pole(p1, 0, "out"), pole(p1, 1, "out"), pole(p1, 2, "out")], [null, null, null]);
	var p2 = poles(k2, 400, [pole(p3, 1, "out"), pole(p3, 2, "out"), pole(p3, 0, "out")], [null, null, null]);
	w(p2, t(p2, "2"), p2, t(p2, "4"));
	w(p2, t(p2, "4"), p2, t(p2, "6"));
}

	// K5 holds itself, nothing in its path switches it off
function selfHoldCase() {
	branch(controlFuse(), "-K5", {start: true, hold: true, x: 200});
}

	// K4 fed through its own NC: on, off, on ...
function pumpCase() {
	branch(controlFuse(), "-K4", {own_nc: true, x: 200});
}

	// a 3-pole circuit-breaker with wires of @p section behind it
function breakerWithWires(designation, section, extra) {
	var q = el(MCB3, 200, 200, "-Q1");
	info(q, "description", designation);
	info(q, "manufacturer_reference", "TEST-" + designation);
	for (var key in extra) info(q, key, extra[key]);
	var m = el(MOTOR, 200, 500, null);   // the far end of the wires
	var outs = ["OUT.2", "OUT.4", "OUT.6"];
	for (var k = 0; k < 3; ++k) w(q, t(q, outs[k]), m, k);
	var ws = qet.conductorUuids(F);
	for (var j = 0; j < ws.length; ++j) qet.setConductorPropertyByUuid(F, ws[j], "conductor_section", section);
}

	// C32 on 2.5 mm², B1, PVC: Iz 21 A (B.52.4) -> too small
function cableCase() {
	breakerWithWires("C32", "2,5", {installation_method: "B1", insulation: "PVC", ambient_c: "30", grouping: "1"});
}
	// C16 on 2.5 mm², B1, PVC, 30 °C, alone: Iz 21 A -> fine
function cableOkCase() {
	breakerWithWires("C16", "2,5", {installation_method: "B1", insulation: "PVC", ambient_c: "30", grouping: "1"});
}
	// C16 on 2.5 mm², nothing else known: the report may not say "no finding"
function noDataCase() {
	breakerWithWires("C16", "2,5", {});
}

	// feed 10 kA, Zs 0.1 Ω; B16 of 6 kA; 150 m of 1.5 mm² behind it
function disconnectionCase() {
	var g = el(FEED, 60, 200, "-G1");
	info(g, "ik3_ka", "10");
	info(g, "zs_ohm", "0,1");
	info(g, "u0_v", "230");
	var q = el(MCB1, 300, 200, "-Q2");
	info(q, "description", "B16 6 kA");
	w(g, t(g, "L"), q, t(q, "IN.1"));
	var x = el(SOCKET, 300, 600, "-X1");
	w(q, t(q, "OUT.2"), x, 0);
	qet.setConductorProperty(F, q, t(q, "OUT.2"), "conductor_section", "1,5");
	if (!qet.addCable(F, "-W1", "NYM-J 3x1,5", ["BN", "BU", "GNYE"], "150 m", 250, 400, 350, 400))
		throw "cannot place the cable";
}

	// F1 300 mA (not S) in front of F2 30 mA in front of X1; X2 behind a breaker only
function rcdCase() {
	var g = el(FEED, 60, 200, "-G1");
	var q = el(MCB1, 300, 120, "-Q3");
	info(q, "description", "B16");
	w(g, t(g, "L"), q, t(q, "IN.1"));
	var f1 = el(RCD, 300, 300, "-F1");
	info(f1, "description", "RCBO B16");
	info(f1, "idn_ma", "300");
	var f2 = el(RCD, 300, 480, "-F2");
	info(f2, "description", "RCBO B16");
	info(f2, "idn_ma", "30");
	w(q, t(q, "OUT.2"), f1, t(f1, "IN.1"));
	w(f1, t(f1, "OUT.2"), f2, t(f2, "IN.1"));
	var x1 = el(SOCKET, 300, 680, "-X1");
	w(f2, t(f2, "OUT.2"), x1, 0);
	var q4 = el(MCB1, 600, 120, "-Q4");
	info(q4, "description", "B16");
	w(g, t(g, "L"), q4, t(q4, "IN.1"));
	var x2 = el(SOCKET, 600, 680, "-X2");
	w(q4, t(q4, "OUT.2"), x2, 0);
}

var cases = {
	wende_richtig: function() { reversingCase(true, true); },
	wende_ohne: function() { reversingCase(false, false); },
	wende_einseitig: function() { reversingCase(false, true); },
	stern_dreieck_richtig: function() { starDeltaCase(true); },
	stern_dreieck_falsch: function() { starDeltaCase(false); },
	selbsthaltung_ohne_aus: selfHoldCase,
	pumpen: pumpCase,
	kabel_zu_klein: cableCase,
	kabel_richtig: cableOkCase,
	ohne_angaben: noDataCase,
	abschaltung: disconnectionCase,
	fi: rcdCase
};
if (!cases[CASE]) throw "unknown case " + CASE;
cases[CASE]();
if (!qet.save(OUT)) throw "cannot save " + OUT;
