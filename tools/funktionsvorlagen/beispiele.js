// Zeichnet die Beispiel-Funktionsvorlagen und speichert jede als .qetmak.
// Aufruf über beispiele_erzeugen.py, das @ZIEL@ durch den Zielordner ersetzt.
// Nur Symbole der Standardsammlung (common://) und eigene Fork-Symbole
// (company:// = bauteile/nachgezeichnet), keine Kunden- oder Herstellerdateien.
//
// Raster: Alle Klemmen einer Strecke liegen auf denselben x-Werten wie die
// Anschlüsse der Geräte darüber und darunter, damit jede Leitung gerade ist.

var ZIEL = "@ZIEL@";
var C = "common://10_electric/10_allpole/";
var MSS = C + "200_fuses_protective_gears/12_magneto_thermal_circuit_breakers/dis_mag_term_3f-2.elmt";
var HAUPT = C + "310_relays_contactors_contacts/02_contacts_cross_referencing/02_power_contacts/com_puiss4.elmt";
var SPULE = C + "310_relays_contactors_contacts/01_coils/bobine3.elmt";
var MOTOR = C + "391_consumers_actuators/10_engines/moto_ac_3ph.elmt";
var KLEMME = C + "130_terminals_terminal_strips/borne_continuite2.elmt";
var VENTIL = C + "391_consumers_actuators/20_valves/magnet_valve_with_connector.elmt";
var LEUCHTE = C + "380_signaling_operating/11_optical_signaling/lampe2.elmt";
var TASTER = C + "380_signaling_operating/20_push_buttons/poussoir.elmt";
var LS1P = C + "200_fuses_protective_gears/11_circuit_breakers/disjonct-m_1f.elmt";
var DI = "company://Allgemein/SPS/SPS_Kanal_DI.elmt";
var DQ = "company://Allgemein/SPS/SPS_Kanal_DQ.elmt";
var SENSOR = "company://Allgemein/Sensoren/Sensor_3L_PNP_NO_oben.elmt";
var STECKDOSE = "company://Allgemein/Steckdosen/Schuko_1P_N_PE/Steckdose_1P+N+PE.elmt";

var F = 0;
var fehler = [];

function el(path, x, y, label) {
  var e = qet.addElement(F, path, x, y);
  if (!e) throw new Error("addElement " + path);
  if (label !== undefined) qet.setElementLabel(F, e, label);
  return e;
}
// Anschluss eines Elements über seine Lage relativ zum Element
function ti(e, dx, dy) {
  var g = qet.elementGeometry(F, e);
  var n = qet.elementTerminals(F, e).length;
  for (var i = 0; i < n; ++i) {
    var p = qet.terminalPosition(F, e, i);
    if (Math.abs(p.x - g.x - dx) < 0.6 && Math.abs(p.y - g.y - dy) < 0.6) return i;
  }
  throw new Error("kein Anschluss bei " + dx + "," + dy);
}
function w(a, adx, ady, b, bdx, bdy) {
  var t = ti(a, adx, ady);
  if (!qet.addConductor(F, a, t, b, ti(b, bdx, bdy)))
    fehler.push("Leitung " + a + " -> " + b);
  // ohne Drahtnummer: die vergibt die Nummerierung des Projekts
  else qet.setConductorProperty(F, a, t, "num", "");
}
// Index des Textes, der das Kennzeichen zeigt
function labelText(e) {
  var n = qet.elementTexts(F, e).length;
  for (var i = 0; i < n; ++i)
    if (qet.elementTextProperty(F, e, i, "info") === "label") return i;
  return -1;
}
function info(e, map) {
  for (var k in map) qet.setElementInfo(F, e, k, map[k]);
}
function klemme(x, y, label, artikel) {
  var k = el(KLEMME, x, y, label);
  // wie bei EPLAN: an der Klemme nur ihre Nummer, waagerecht neben dem
  // Klemmenkörper; den Leistennamen schreibt leiste() einmal links davor
  var t = labelText(k);
  if (t >= 0) {
    qet.setElementTextProperty(F, k, t, "source", "text");
    qet.setElementTextProperty(F, k, t, "text", label.split(":")[1]);
    qet.setElementTextProperty(F, k, t, "rotation", "0");
    qet.setElementTextProperty(F, k, t, "size", "7");
    qet.setElementTextProperty(F, k, t, "x", "4");
    qet.setElementTextProperty(F, k, t, "y", "-13");
  }
  info(k, {manufacturer: "WAGO", manufacturer_reference: artikel || "2002-1201",
           description: artikel === "2002-1207" ? "Schutzleiterklemme 2,5 mm²" : "Durchgangsklemme 2,5 mm²"});
  return k;
}
// der Name der Klemmleiste einmal links neben der ersten Klemme einer Gruppe
function leiste(x, y) {
  qet.addText(F, "-{X}", x - 34, y - 10);
}
function neuesBlatt(titel) {
  F = qet.addFolio();
  qet.setFolioTitle(F, titel);
}
function speichern(name, kategorie, beschreibung, parameter, tabellen) {
  var datei = qet.saveTemplate(F, name, {
    category: kategorie, description: beschreibung,
    parameters: parameter, tables: tabellen || [],
    path: ZIEL + "/" + kategorie + "/" + name + ".qetmak"});
  qet.log("VORLAGE " + (datei || ("FEHLER " + name)));
}

// ---------------------------------------------------------------------------
// 1. Motorabgang direkt (DOL): Motorschutzschalter, Schütz mit DQ-Ansteuerung,
//    Klemmen, Motorleitung, Motor. Die Leistung wählt Schutzschalter, Schütz
//    und Leitung aus der Tabelle (Richtwerte 400 V, 4-polig, bitte prüfen).
// ---------------------------------------------------------------------------
neuesBlatt("Vorlage Motorabgang DOL");
var x = 200;
var qa1 = el(MSS, x, 150, "QA1");
info(qa1, {manufacturer: "Siemens", manufacturer_reference: "{QA_ART}",
           description: "Motorschutzschalter {QA_I}"});
var km = el(HAUPT, x, 250);
var spule = el(SPULE, 400, 250, "QA2");
info(spule, {manufacturer: "Siemens", manufacturer_reference: "{KM_ART}",
             description: "Leistungsschütz {P_kW} kW AC-3, Spule 24 V DC"});
qet.linkElements(F, spule, F, km);
var dq = el(DQ, 400, 170);
info(dq, {plc_address: "{ADR}", function: "{BMK} Ein"});
w(dq, 0, 20, spule, 0, -20);
var k = [];
for (var i = 0; i < 3; ++i) {
  w(qa1, -20 + 20 * i, 40, km, -20 + 20 * i, -20);
  k.push(klemme(x - 20 + 20 * i, 330, "{X}:{XN" + (i ? "+" + i : "") + "}"));
  if (!i) leiste(x - 20, 330);
  w(km, -20 + 20 * i, 20, k[i], 0, -10);
}
var kpe = klemme(x + 40, 330, "{X}:{XN+3}", "2002-1207");
var m = el(MOTOR, x, 470, "{BMK}");
info(m, {description: "Drehstrommotor {P_kW} kW", function: "{TEXT}"});
for (i = 0; i < 3; ++i) w(k[i], 0, 10, m, -20 + 20 * i, -40);
w(kpe, 0, 10, m, 40, -40);
qet.addCable(F, "{W}", "{W_TYP}", ["1", "2", "3", "GNYE"], "", x - 35, 410, x + 55, 410);
qet.addText(F, "L1  L2  L3", x - 28, 80);
qet.addText(F, "PE", x + 34, 300);
qet.addText(F, "M", 395, 285);
speichern("Motorabgang DOL", "Motoren",
  "Direktanlauf: Motorschutzschalter, Schütz am SPS-Ausgang, Klemmen, Motorleitung und " +
  "Drehstrommotor. Die Leistung wählt Schutzschalter, Schütz und Leitung (Richtwerte 400 V).",
  [{name: "BMK", label: "Kennzeichen Motor", kind: "label", default: "MA1"},
   {name: "P_kW", label: "Leistung", unit: "kW", default: "4"},
   {name: "TEXT", label: "Funktion", default: "Antrieb"},
   {name: "ADR", label: "SPS-Ausgang", default: "%Q0.0"},
   {name: "X", label: "Klemmleiste", default: "X1"},
   {name: "XN", label: "erste Klemme", kind: "terminal", of: "X"},
   {name: "W", label: "Kabel", kind: "cable", default: "W1"}],
  [{key: "P_kW", columns: ["P_kW", "I_N", "QA_ART", "QA_I", "KM_ART", "W_TYP"], rows: [
    {P_kW: "0.37", I_N: "1.1 A", QA_ART: "3RV2011-1AA10", QA_I: "1,1...1,6 A", KM_ART: "3RT2015-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "0.55", I_N: "1.5 A", QA_ART: "3RV2011-1AA10", QA_I: "1,1...1,6 A", KM_ART: "3RT2015-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "0.75", I_N: "1.9 A", QA_ART: "3RV2011-1BA10", QA_I: "1,4...2 A", KM_ART: "3RT2015-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "1.1", I_N: "2.6 A", QA_ART: "3RV2011-1DA10", QA_I: "2,2...3,2 A", KM_ART: "3RT2015-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "1.5", I_N: "3.5 A", QA_ART: "3RV2011-1EA10", QA_I: "2,8...4 A", KM_ART: "3RT2015-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "2.2", I_N: "4.9 A", QA_ART: "3RV2011-1FA10", QA_I: "3,5...5 A", KM_ART: "3RT2015-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "3", I_N: "6.5 A", QA_ART: "3RV2011-1HA10", QA_I: "5,5...8 A", KM_ART: "3RT2015-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "4", I_N: "8.5 A", QA_ART: "3RV2011-1JA10", QA_I: "7...10 A", KM_ART: "3RT2016-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "5.5", I_N: "11.5 A", QA_ART: "3RV2011-1KA10", QA_I: "9...12,5 A", KM_ART: "3RT2017-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "7.5", I_N: "15.5 A", QA_ART: "3RV2011-4AA10", QA_I: "10...16 A", KM_ART: "3RT2018-1BB41", W_TYP: "Motorleitung 4G2,5"},
    {P_kW: "11", I_N: "22 A", QA_ART: "3RV2021-4CA10", QA_I: "18...25 A", KM_ART: "3RT2026-1BB40", W_TYP: "Motorleitung 4G4"},
    {P_kW: "15", I_N: "29 A", QA_ART: "3RV2021-4EA10", QA_I: "27...32 A", KM_ART: "3RT2027-1BB40", W_TYP: "Motorleitung 4G6"}]}]);

// ---------------------------------------------------------------------------
// 2. DI-Sensor mit Kabel: 3-Leiter-Sensor, Kabel, drei Klemmen, DI-Kanal.
// ---------------------------------------------------------------------------
neuesBlatt("Vorlage DI-Sensor mit Kabel");
x = 200;
var di = el(DI, x, 150);
info(di, {plc_address: "{ADR}", function: "{TEXT}"});
var kp = klemme(x - 20, 230, "{X}:{XN}");
leiste(x - 20, 230);
var ks = klemme(x, 230, "{X}:{XN+1}");
var km2 = klemme(x + 20, 230, "{X}:{XN+2}");
w(di, 0, 20, ks, 0, -10);
var b = el(SENSOR, x, 360, "{BMK}");
info(b, {description: "Näherungsschalter PNP Schließer", function: "{TEXT}",
         manufacturer_reference: "{B_ART}"});
w(kp, 0, 10, b, -20, -30);
w(ks, 0, 10, b, 0, -30);
w(km2, 0, 10, b, 20, -30);
qet.addCable(F, "{W}", "{W_TYP}", ["BN", "BK", "BU"], "", x - 35, 305, x + 35, 305);
qet.addText(F, "L+", x - 36, 196);
qet.addText(F, "M", x + 25, 196);
speichern("DI-Sensor mit Kabel", "Sensoren",
  "3-Leiter-Sensor (PNP) mit Anschlusskabel und drei Klemmen am SPS-Eingang. " +
  "Der Kanal wird mit dem SPS-Modul verknüpft, das die Adresse hat.",
  [{name: "BMK", label: "Kennzeichen", kind: "label", default: "BG1"},
   {name: "TEXT", label: "Funktion", default: "Endlage"},
   {name: "ADR", label: "SPS-Eingang", default: "%I0.0"},
   {name: "X", label: "Klemmleiste", default: "X2"},
   {name: "XN", label: "erste Klemme", kind: "terminal", of: "X"},
   {name: "W", label: "Kabel", kind: "cable", default: "W1"},
   {name: "W_TYP", label: "Kabeltyp", default: "PUR 3x0,34"},
   {name: "B_ART", label: "Sensor-Artikel", default: ""}]);

// ---------------------------------------------------------------------------
// 3. Ventil mit DQ-Kanal: DQ-Kanal, Klemmen, Kabel, Ventilstecker.
// ---------------------------------------------------------------------------
neuesBlatt("Vorlage Ventil mit DQ-Kanal");
x = 200;
var dq2 = el(DQ, x - 20, 150);
info(dq2, {plc_address: "{ADR}", function: "{TEXT}"});
var k1 = klemme(x - 20, 230, "{X}:{XN}");
leiste(x - 20, 230);
// Stecker: + links, - Mitte, PE rechts
var k2 = klemme(x, 230, "{X}:{XN+1}");
var kpe2 = klemme(x + 20, 230, "{X}:{XN+2}", "2002-1207");
w(dq2, 0, 20, k1, 0, -10);
var v = el(VENTIL, x, 360, "{BMK}");
info(v, {description: "Magnetventil 24 V DC", function: "{TEXT}", manufacturer_reference: "{V_ART}"});
w(k1, 0, 10, v, -20, -30);
w(k2, 0, 10, v, 0, -30);
w(kpe2, 0, 10, v, 20, -30);
qet.addCable(F, "{W}", "{W_TYP}", ["1", "2", "GNYE"], "", x - 35, 305, x + 35, 305);
qet.addText(F, "M", x - 5, 196);
qet.addText(F, "PE", x + 14, 196);
speichern("Ventil mit DQ-Kanal", "Ventile",
  "Magnetventil mit Gerätestecker am SPS-Ausgang: Klemmen, Kabel, Schutzleiter. " +
  "Der Kanal wird mit dem SPS-Modul verknüpft, das die Adresse hat.",
  [{name: "BMK", label: "Kennzeichen", kind: "label", default: "MB1"},
   {name: "TEXT", label: "Funktion", default: "Zylinder ausfahren"},
   {name: "ADR", label: "SPS-Ausgang", default: "%Q0.1"},
   {name: "X", label: "Klemmleiste", default: "X3"},
   {name: "XN", label: "erste Klemme", kind: "terminal", of: "X"},
   {name: "W", label: "Kabel", kind: "cable", default: "W1"},
   {name: "W_TYP", label: "Kabeltyp", default: "PUR 3x0,75"},
   {name: "V_ART", label: "Ventil-Artikel", default: ""}]);

// ---------------------------------------------------------------------------
// 4. Meldeleuchte am DQ-Kanal; die Farbe wählt den Artikel.
// ---------------------------------------------------------------------------
neuesBlatt("Vorlage Meldeleuchte DQ");
x = 200;
var dq3 = el(DQ, x, 150);
info(dq3, {plc_address: "{ADR}", function: "{TEXT}"});
var p = el(LEUCHTE, x, 230, "{BMK}");
info(p, {manufacturer: "Eaton", manufacturer_reference: "{P_ART}",
         description: "Leuchtmelder {FARBE}, LED 24 V", function: "{TEXT}"});
w(dq3, 0, 20, p, 0, -20);
qet.addText(F, "M", x + 6, 246);
speichern("Meldeleuchte DQ", "Meldungen",
  "Leuchtmelder am SPS-Ausgang. Die Farbe wählt Frontelement und LED-Element.",
  [{name: "BMK", label: "Kennzeichen", kind: "label", default: "PF1"},
   {name: "TEXT", label: "Funktion", default: "Betrieb"},
   {name: "FARBE", label: "Farbe", default: "grün"},
   {name: "ADR", label: "SPS-Ausgang", default: "%Q0.2"}],
  [{key: "FARBE", columns: ["FARBE", "P_ART"], rows: [
    {FARBE: "grün", P_ART: "M22-L-G + M22-A + M22-LED-G"},
    {FARBE: "gelb", P_ART: "M22-L-Y + M22-A + M22-LED-Y"},
    {FARBE: "rot", P_ART: "M22-L-R + M22-A + M22-LED-R"},
    {FARBE: "weiß", P_ART: "M22-L-W + M22-A + M22-LED-W"},
    {FARBE: "blau", P_ART: "M22-L-B + M22-A + M22-LED-B"}]}]);

// ---------------------------------------------------------------------------
// 5. Taster am DI-Kanal.
// ---------------------------------------------------------------------------
neuesBlatt("Vorlage Taster DI");
x = 200;
var di2 = el(DI, x, 150);
info(di2, {plc_address: "{ADR}", function: "{TEXT}"});
var s = el(TASTER, x, 230, "{BMK}");
var sg = qet.elementGeometry(F, s);
info(s, {manufacturer: "Eaton", manufacturer_reference: "M22-D-{FARBE_K} + M22-A + M22-K10",
         description: "Drucktaster {TEXT}", function: "{TEXT}"});
var oben = qet.terminalPosition(F, s, 0), unten = qet.terminalPosition(F, s, 1);
w(di2, 0, 20, s, oben.x - sg.x, oben.y - sg.y);
qet.addText(F, "L+", x + 6, unten.y + 4);
speichern("Taster DI", "Befehlsgeräte",
  "Drucktaster (Schließer) an L+ und am SPS-Eingang.",
  [{name: "BMK", label: "Kennzeichen", kind: "label", default: "SF1"},
   {name: "TEXT", label: "Funktion", default: "Start"},
   {name: "FARBE_K", label: "Farbe (Kurzzeichen)", default: "G", choices: ["G", "R", "W", "Y", "B", "S"]},
   {name: "ADR", label: "SPS-Eingang", default: "%I0.1"}]);

// ---------------------------------------------------------------------------
// 6. Steckdose 230 V mit Leitungsschutzschalter; die Absicherung wählt den Artikel.
// ---------------------------------------------------------------------------
neuesBlatt("Vorlage Steckdose 230 V");
x = 200;
var f1 = el(LS1P, x, 160, "FA1");
info(f1, {manufacturer: "Siemens", manufacturer_reference: "{LS_ART}",
          description: "Leitungsschutzschalter {LS} 1-polig"});
var kl = klemme(x, 250, "{X}:{XN}");
leiste(x, 250);
var kn = klemme(x + 20, 250, "{X}:{XN+1}");
var kpe3 = klemme(x + 40, 250, "{X}:{XN+2}", "2002-1207");
w(f1, 0, 20, kl, 0, -10);
var xd = el(STECKDOSE, x, 360, "{BMK}");
info(xd, {description: "Schutzkontaktsteckdose 230 V", function: "{TEXT}"});
qet.placeElementTexts(F, [xd], "left");
w(kl, 0, 10, xd, 0, 0);
w(kn, 0, 10, xd, 20, 0);
w(kpe3, 0, 10, xd, 40, 0);
qet.addCable(F, "{W}", "NYM-J 3x1,5", ["BN", "BU", "GNYE"], "", x - 15, 335, x + 55, 335);
qet.addText(F, "L", x - 16, 118);
qet.addText(F, "N", x + 16, 216);
qet.addText(F, "PE", x + 34, 216);
speichern("Steckdose 230 V", "Versorgung",
  "Schutzkontaktsteckdose mit Leitungsschutzschalter, Klemmen und Anschlussleitung.",
  [{name: "BMK", label: "Kennzeichen", kind: "label", default: "XD1"},
   {name: "TEXT", label: "Funktion", default: "Service-Steckdose"},
   {name: "LS", label: "Absicherung", default: "B16"},
   {name: "X", label: "Klemmleiste", default: "X4"},
   {name: "XN", label: "erste Klemme", kind: "terminal", of: "X"},
   {name: "W", label: "Kabel", kind: "cable", default: "W1"}],
  [{key: "LS", columns: ["LS", "LS_ART"], rows: [
    {LS: "B6", LS_ART: "5SL6106-6"}, {LS: "B10", LS_ART: "5SL6110-6"},
    {LS: "B13", LS_ART: "5SL6113-6"}, {LS: "B16", LS_ART: "5SL6116-6"},
    {LS: "C10", LS_ART: "5SL6110-7"}, {LS: "C16", LS_ART: "5SL6116-7"}]}]);

// ---------------------------------------------------------------------------
// 7. Motorabgang Stern-Dreieck (wie das Siemens-Schaltbild 3RA24): Motorschutz-
//    schalter auf I_N, dahinter Netzschütz QA2 und Dreieckschütz QA3, unter dem
//    Dreieckschütz das Sternschütz QA4 mit Sternbrücke; jedes Schütz an einem
//    SPS-Ausgang, zwei Motorleitungen, Motor mit sechs Anschlüssen.
//    Dreieck: L1-W2, L2-U2, L3-V2 (U1-W2, V1-U2, W1-V2). Netz- und Dreieckschütz
//    auf 0,58 I_N, Sternschütz auf 0,33 I_N (Richtwerte 400 V, 4-polig, prüfen).
//    Stern und Dreieck gegeneinander verriegeln (Programm und Hilfskontakte).
// ---------------------------------------------------------------------------
// die Leitung an Anschluss (dx, dy) von e: ihr waagerechtes Stück auf Höhe y
function ebene(e, dx, dy, y) {
  var t = ti(e, dx, dy);
  var seg = qet.conductorSegments(F, e, t);
  for (var s = 0; s < seg.length; ++s) {
    var m = /^(\d+): \(([-\d.]+),([-\d.]+)\)-\(([-\d.]+),([-\d.]+)\) horizontal movable/.exec(seg[s]);
    if (m) { qet.moveConductorSegment(F, e, t, parseInt(m[1]), 0, y - parseFloat(m[3])); return; }
  }
  fehler.push("kein waagerechtes Stück an " + e);
}
neuesBlatt("Vorlage Motorabgang Stern-Dreieck");
x = 200;
var qaS = el(MSS, x, 150, "QA1");
info(qaS, {manufacturer: "Siemens", manufacturer_reference: "{QA_ART}",
           description: "Motorschutzschalter {QA_I}"});
var kmN = el(HAUPT, x, 250);
var kmD = el(HAUPT, 420, 250);
var kmS = el(HAUPT, 620, 380);
var spN = el(SPULE, 780, 540, "QA2");
var spD = el(SPULE, 880, 540, "QA3");
var spS = el(SPULE, 980, 540, "QA4");
info(spN, {manufacturer: "Siemens", manufacturer_reference: "{KM_ART}",
           description: "Netzschütz {P_kW} kW Stern-Dreieck, Spule 24 V DC"});
info(spD, {manufacturer: "Siemens", manufacturer_reference: "{KM_ART}",
           description: "Dreieckschütz, Spule 24 V DC"});
info(spS, {manufacturer: "Siemens", manufacturer_reference: "{KS_ART}",
           description: "Sternschütz, Spule 24 V DC"});
qet.linkElements(F, spN, F, kmN);
qet.linkElements(F, spD, F, kmD);
qet.linkElements(F, spS, F, kmS);
var dqN = el(DQ, 780, 460);
var dqD = el(DQ, 880, 460);
var dqS = el(DQ, 980, 460);
info(dqN, {plc_address: "{ADR}", function: "{BMK} Netz"});
info(dqD, {plc_address: "{ADR+1}", function: "{BMK} Dreieck"});
info(dqS, {plc_address: "{ADR+2}", function: "{BMK} Stern"});
w(dqN, 0, 20, spN, 0, -20);
w(dqD, 0, 20, spD, 0, -20);
w(dqS, 0, 20, spS, 0, -20);
var kA = [], kB = [];
for (i = 0; i < 3; ++i) {
  w(qaS, -20 + 20 * i, 40, kmN, -20 + 20 * i, -20);
  // Dreieckschütz hinter dem Schutzschalter, je Phase eine eigene Höhe
  w(qaS, -20 + 20 * i, 40, kmD, -20 + 20 * ((i + 2) % 3), -20);
  ebene(kmD, -20 + 20 * ((i + 2) % 3), -20, 200 + 10 * i);
  kA.push(klemme(x - 20 + 20 * i, 330, "{X}:{XN" + (i ? "+" + i : "") + "}"));
  kB.push(klemme(400 + 20 * i, 480, "{X}:{XN+" + (4 + i) + "}"));
  w(kmN, -20 + 20 * i, 20, kA[i], 0, -10);
  w(kmD, -20 + 20 * i, 20, kB[i], 0, -10);
  // Sternschütz an U2 V2 W2 hinter dem Dreieckschütz
  w(kmD, -20 + 20 * i, 20, kmS, -20 + 20 * i, -20);
  ebene(kmS, -20 + 20 * i, -20, 300 + 10 * i);
}
leiste(x - 20, 330);
leiste(400, 480);
var kpeS = klemme(x + 40, 330, "{X}:{XN+3}", "2002-1207");
// Sternbrücke
w(kmS, -20, 20, kmS, 0, 20);
w(kmS, 0, 20, kmS, 20, 20);
var mS = el(C + "391_consumers_actuators/10_engines/induction_motor_6_terminals.elmt", x, 620, "{BMK}");
info(mS, {description: "Drehstrommotor {P_kW} kW, Stern-Dreieck-Anlauf", function: "{TEXT}"});
// das Kennzeichen links oben, frei von der PE-Leitung rechts
var tM = labelText(mS);
if (tM >= 0) {
  qet.setElementTextProperty(F, mS, tM, "x", "-80");
  qet.setElementTextProperty(F, mS, tM, "y", "-100");
}
for (i = 0; i < 3; ++i) {
  w(kA[i], 0, 10, mS, -20 + 20 * i, -100);
  w(kB[i], 0, 10, mS, -20 + 20 * i, 20);
}
w(kpeS, 0, 10, mS, 42, -11);
qet.addCable(F, "{W}", "{W_TYP}", ["1", "2", "3", "GNYE"], "", x - 35, 410, x + 55, 410);
qet.addCable(F, "{W+1}", "{W_TYP}", ["1", "2", "3"], "", 385, 545, 455, 545);
qet.addText(F, "L1  L2  L3", x - 28, 80);
qet.addText(F, "Netz", x + 32, 270);
qet.addText(F, "Dreieck", 452, 270);
qet.addText(F, "Stern", 652, 412);
speichern("Motorabgang Stern-Dreieck", "Motoren",
  "Stern-Dreieck-Anlauf: Motorschutzschalter, Netz- und Dreieckschütz dahinter, Sternschütz mit " +
  "Sternbrücke, jedes Schütz am SPS-Ausgang (ADR, ADR+1, ADR+2), zwei Motorleitungen, Motor mit " +
  "sechs Anschlüssen. Die Leistung wählt Schutzschalter, Schütze und Leitung (Richtwerte 400 V). " +
  "Stern und Dreieck gegeneinander verriegeln.",
  [{name: "BMK", label: "Kennzeichen Motor", kind: "label", default: "MA1"},
   {name: "P_kW", label: "Leistung", unit: "kW", default: "7.5"},
   {name: "TEXT", label: "Funktion", default: "Antrieb"},
   {name: "ADR", label: "SPS-Ausgang Netz", default: "%Q0.0"},
   {name: "X", label: "Klemmleiste", default: "X1"},
   {name: "XN", label: "erste Klemme", kind: "terminal", of: "X"},
   {name: "W", label: "Kabel", kind: "cable", default: "W1"}],
  [{key: "P_kW", columns: ["P_kW", "I_N", "QA_ART", "QA_I", "KM_ART", "KS_ART", "W_TYP"], rows: [
    {P_kW: "5.5", I_N: "11.5 A", QA_ART: "3RV2011-1KA10", QA_I: "9...12,5 A", KM_ART: "3RT2015-1BB41", KS_ART: "3RT2015-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "7.5", I_N: "15.5 A", QA_ART: "3RV2011-4AA10", QA_I: "10...16 A", KM_ART: "3RT2016-1BB41", KS_ART: "3RT2015-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "11", I_N: "22 A", QA_ART: "3RV2021-4CA10", QA_I: "18...25 A", KM_ART: "3RT2018-1BB41", KS_ART: "3RT2016-1BB41", W_TYP: "Motorleitung 4G1,5"},
    {P_kW: "15", I_N: "29 A", QA_ART: "3RV2021-4EA10", QA_I: "27...32 A", KM_ART: "3RT2025-1BB40", KS_ART: "3RT2017-1BB41", W_TYP: "Motorleitung 4G2,5"}]}]);

if (fehler.length) qet.log("FEHLER " + fehler.join("; "));
qet.log("FERTIG");
