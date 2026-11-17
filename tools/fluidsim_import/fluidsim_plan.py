"""FluidSIM-6-Schaltungen (.circ) als QElectroTech-Projekt (.qet) übernehmen.

Eine .circ ist gzip-gepacktes XML (Beschreibung: PLAN-PNEUMATIK.md, Abschnitt 1); jede wird ein Blatt.
Übernommen werden:
- jedes Bauteil mit Anschlüssen (Pneumatik, Elektrik, Hydraulik) als eingebettetes QET-Bauteil mit EIGENER
  Grafik nach DIN ISO 1219-1 / IEC 60617 (eigene_grafik.py; aus der Datei nur Anschlusslagen und Daten, keine
  Festo-Grafik – so kann jeder übernehmen, auch ohne FluidSIM; --fluidsim-grafik übernimmt sie für den
  Eigengebrauch mit Lizenz), mit Drehung, Spiegelung und Maßstab der Schaltung,
  deutschem Namen (FluidSIMs eigene Übersetzung) und, bei Zylindern, Kolben-/Stangendurchmesser und Hub;
- die Kennzeichnung (-MM1, -QM1, -MB1 …), Beschreibung und technische Werte als Bauteilinformation
  (description, comment), die in FluidSIM sichtbaren Werte (24 V, p = 6 bar …) als Text am Bauteil;
- jede Leitung als QET-Leitung zwischen denselben Anschlüssen (QET legt den Weg neu); Verzweigungen werden zu
  Leitungen am selben Netz;
- Schalldämpfer und Blindstopfen als Abschluss des Anschlusses, an dem sie sitzen;
- Spule ↔ Ventil (Verweis am Ventilanschluss oder Magnetkennzeichnung an der Betätigung AEML/AEMR) mit der
  Seite (valve_side) und Relaisspule ↔ Kontakte (gleiche Kennzeichnung) als Master/Slave, auch über Blattgrenzen;
- für die Simulation: die Schaltwege jeder Ventilstellung aus FluidSIMs Stellungsbausteinen (B<n>_<nr>),
  gespiegelte Ventile mit getauschten Stellungen und Seiten; Funktion und alle Kennwerte der Schaltung mit
  Einheit (<parameter>), FluidSIM-Klassen unter dem Namen, den der Simulationskern kennt (ALIAS, UMBAU);
  elektrische Rollen (Elementinformation sim_role: supply+/-, voltage, button, switch, limit, solenoid,
  grafcet_io); Endschalter und Rollenhebelventile über die Wegmarken des Wegmaßstabs (function „-MM1 0..2“);
  fehlende Kennzeichnungen von Ventilen, Zylindern und Tastern (-QM1, -MM1, -SF1 …);
- GRAFCET (Schritte, Transitionen, Aktionen, Verzweigungen, GRAFCET-SPS mit ihren E/A-Namen), siehe
  fluidsim_grafcet.py;
- ältere Dateien mit gepackten Koordinaten (Linien als Hex-Text), siehe fluidsim_symbole.entpacken.

Nicht übernommen: Rahmen und Schriftfeld, Tabellen, Zustandsdiagramme, Stücklisten, Digitaltechnik (LOGO),
Teil-GRAFCETs und Zwangssteuerungen. Artikeldaten (Hersteller, Bestellnummer) speichert FluidSIM nicht.

    python fluidsim_plan.py SCHALTUNG.circ [WEITERE.circ …] AUSGABE.qet [--qet build/deploy/QElectroTech.exe]
    python fluidsim_plan.py ORDNER_MIT_CIRC AUSGABE.qet          # jede .circ ein Blatt, nach Namen sortiert
    python fluidsim_plan.py SCHALTUNG.circ AUSGABE.qet --bericht bericht.json   # Zahlen, Taster, Handventile
    python fluidsim_plan.py --pruefe-drehung "C:/Program Files (x86)/Didactic/FluidSIM6/circ"

In QElectroTech: Datei > FluidSIM-Schaltung importieren (sources/import/fluidsim/fluidsimimport.cpp).
Prüfung über alle Beispielschaltungen: fluidsim_batch.py.

Das Projekt schreibt QElectroTech selbst (über --run, wie das MCP qet_edit); dafür setzt das Werkzeug für
diesen einen Aufruf QET_ENABLE_SCRIPTING=1.
"""
import argparse
import gzip
import hashlib
import json
import math
import os
import re
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

HIER = Path(__file__).resolve().parent
REPO = HIER.parents[1]


def _erster(*pfade):
    """Der erste vorhandene Pfad: im Quellbaum (tools/…, misc/…) oder neben einem installierten
    QElectroTech (fluidsim_import/symbole, mcp, elements)."""
    return next((p for p in pfade if p.exists()), pfade[0])


SYMBOLE = _erster(HIER / "symbole", REPO / "tools" / "symbole")
MCP = _erster(REPO / "misc" / "qet-mcp", HIER.parent / "mcp")
GRAFCET_SAMMLUNG = _erster(REPO / "bauteile" / "grafcet", HIER.parent / "elements")
sys.path.insert(0, str(SYMBOLE))
import fluidsim_symbole as fs  # noqa: E402
import fluid_kennwerte  # noqa: E402
import fluid_schaltwege  # noqa: E402
sys.path.insert(0, str(HIER))
import fluidsim_grafcet  # noqa: E402
import eigene_grafik  # noqa: E402

FLUIDSIM = Path("C:/Program Files (x86)/Didactic/FluidSIM6")
AUSLASSEN = ("Grafcet.", "Digital.")  # Objektarten, die keine Bauteile eines Fluid-/Elektroplans sind


def lesen(pfad: Path):
    roh = pfad.read_bytes()
    if roh[:2] == b"\x1f\x8b":
        roh = gzip.decompress(roh)
    return fs.entpacken(ET.fromstring(roh))


def abschnitt(wurzel, art):
    return next((s for s in wurzel.findall("section") if s.get("type") == art), None)


def objekte(wurzel):
    """Alle Schaltungsobjekte mit Symbol, auch in Gruppen."""
    co = abschnitt(wurzel, "circuit-objects")
    if co is None:
        return []
    return [o for o in co.iter() if o.tag in ("component", "terminal") and o.get("symbol-ref")]


def anschluss_lage(symbole, objekt, anschluss):
    """Lage eines Anschlusses eines Objekts auf dem Blatt (FluidSIM-Einheiten), oder None."""
    ebene_name, ident = fs.anschluss_schluessel(objekt, anschluss)
    symbol = symbole.get(objekt.get("symbol-ref"))
    if symbol is None:
        return None
    for eb in symbol.findall("symbol-layer"):
        if eb.get("ident", "") != ebene_name:
            continue
        for con in eb.findall("connector"):
            if con.get("ident") == ident:
                lx = fs.zahl(eb, "ref-x") + fs.zahl(eb, "scale-x", 1.0) * fs.zahl(con, "cx")
                ly = fs.zahl(eb, "ref-y") + fs.zahl(eb, "scale-y", 1.0) * fs.zahl(con, "cy")
                q = fs.Trafo.von(objekt).punkt(lx, ly)
                m = fs.mitte(objekt)
                return (m[0] + q[0], m[1] + q[1])
    return None


def pruefe_drehung(ordner: Path):
    """Zählt für SINN = +1 und -1, wie viele Leitungen an gedrehten Bauteilen genau passen."""
    treffer = {1: 0, -1: 0}
    gesamt = 0
    for f in sorted(ordner.rglob("*.circ")):
        try:
            w = lesen(f)
        except Exception:
            continue
        grafik = abschnitt(w, "symbol-graphics")
        if grafik is None:
            continue
        symbole = {s.get("ident"): s for s in grafik.findall("symbol")}
        nach_id = {o.get("ident"): o for o in objekte(w)}
        for v in w.iter("connect"):
            a, b = v.get("con1", ""), v.get("con2", "")
            oa, _, xa = a.partition(".")
            ob, _, xb = b.partition(".")
            if oa not in nach_id or ob not in nach_id:
                continue
            if not (fs.zahl(nach_id[oa], "rotation") or fs.zahl(nach_id[ob], "rotation")):
                continue
            pkte = (v.get("points") or "").split()
            if not pkte:
                continue
            lx, ly = (float(z) for z in pkte[-1].split(","))
            gesamt += 1
            for sinn in (1, -1):
                fs.Trafo.SINN = sinn
                pa, pb = anschluss_lage(symbole, nach_id[oa], xa), anschluss_lage(symbole, nach_id[ob], xb)
                if pa and pb and math.hypot(pa[0] + lx - pb[0], pa[1] + ly - pb[1]) < 1.0:
                    treffer[sinn] += 1
    fs.Trafo.SINN = -1
    print(f"Leitungen an gedrehten Bauteilen: {gesamt}; passend mit Uhrzeigersinn: {treffer[1]}, "
          f"gegen den Uhrzeigersinn: {treffer[-1]}")
    return treffer


def dateiname(name, saat):
    """Lesbarer Name der eingebetteten Bauteildatei: deutscher Name und ein Stück der Prüfsumme."""
    kurz = re.sub(r"[^\w\-]+", "_", name, flags=re.UNICODE).strip("_")[:40] or "Bauteil"
    return f"{kurz}_{saat[-8:]}.elmt"


def sichtbare_werte(objekt, textfelder):
    """Die in FluidSIM am Bauteil angezeigten Parameter (^#…, z. B. „p0 = 6 bar“) mit ihrem Textfeld."""
    out = []
    for a in objekt.iter("attribute"):
        name, ref = a.get("name", ""), a.get("display-ref")
        if not name.startswith("^#") or not ref:
            continue
        tf = textfelder.get(ref)
        wert = (tf.get("value") if tf is not None else None) or a.get("value") or ""
        if wert.strip() and tf is not None and tf.get("visible") != "false":
            out.append((wert.strip(), tf))
    return out


def ventil_stellungen(symbol):
    """Schaltwege je Stellung aus den Stellungsbausteinen B<n>_<nr> des Ventilsymbols, links nach rechts."""
    ebenen = [l for l in symbol.findall("symbol-layer") if re.match(r"^B\d+_\d+$", l.get("ident") or "")]
    con = next((l for l in symbol.findall("symbol-layer") if re.match(r"^CON\d+$", l.get("ident") or "")), None)
    if not ebenen or con is None:
        return None
    ebenen.sort(key=lambda l: float(l.get("ref-x", 0)))
    anschluesse = {(round(float(c.get("cx"))), float(c.get("cy")) < 0): c.get("designation") or c.get("ident")
                   for c in con.iter("connector")}
    return [fluid_schaltwege.baustein_wege(e, anschluesse) for e in ebenen]


def kennwerte(objekt):
    """Die einstellbaren und festen Werte des Bauteils dieser Schaltung: [(name, wert, einheit)]."""
    out = []
    for e in objekt:
        if e.tag not in ("parameter", "const") or not e.get("ident"):
            continue
        ident, wert = e.get("ident"), e.get("value", "")
        if (not wert or wert.endswith(".pic") or ".pic," in wert or ident == "picture" or "label" in ident.lower()
                or ident.endswith("_err") or ident == "error_mode"):
            continue
        out.append((ident, wert, e.get("unit", "")))
    return out


def simulationsdaten(text, symbol, objekt):
    """Schaltwege, Funktion und Kennwerte in den <fluid>-Block der umgewandelten Datei eintragen.

    Ventile: die Stellungen folgen der Zeichnung von links nach rechts. Ist das Bauteil in der Schaltung
    gespiegelt oder um 180° gedreht, steht die linke FluidSIM-Stellung rechts: dann werden Stellungen,
    Ruhestellung und Betätigungsseiten getauscht, damit ein Klick auf die linke Hälfte links betätigt."""
    if '<fluid kind="valve"' in text:
        stellungen = ventil_stellungen(symbol)
        if stellungen:
            tr = fs.Trafo.von(objekt)
            if (tr.sx < 0) != (tr.rot == 180):
                stellungen = stellungen[::-1]
                n = len(stellungen)
                text = re.sub(r'<positions initial="(\d+)"',
                              lambda m: f'<positions initial="{n + 1 - int(m.group(1))}"', text, count=1)
                text = (text.replace('side="left"', 'side="__r"').replace('side="right"', 'side="left"')
                        .replace('side="__r"', 'side="right"'))
            text = fluid_schaltwege.text_ersetzen(text, stellungen) or text
    if "<fluid" in text:
        fkt, werte = simulationsfunktion(objekt, symbol, kennwerte(objekt))
        text = fluid_kennwerte.eintragen(text, fkt, werte) or text
        ref = objekt.get("object-ref", "")
        for teil, art, namen in UMBAU:
            if teil in ref:
                if art:
                    text = re.sub(r'(<fluid\b[^>]*\bkind=")[a-z_]+(")', r"\g<1>%s\g<2>" % art, text, count=1)
                for alt, neu in namen.items():
                    text = re.sub(r'(<terminal\b[^>]*\bname=")%s(")' % re.escape(alt), r"\g<1>%s\g<2>" % neu, text)
                break
    return text


# Bauteile, die der Simulationskern als andere Art oder mit anderen Anschlussnamen rechnet:
# (Teil der FluidSIM-Klasse, neue Art oder "", {Anschlussname: neuer Name})
UMBAU = [
    ("Pneu.Supply.AirServiceUnit", "flow_control", {"C1": "1", "C2": "2"}),
    ("Pneu.Supply.Filter", "flow_control", {"C1": "1", "C2": "2"}),
    ("Pneu.Supply.Lubricator", "flow_control", {"C1": "1", "C2": "2"}),
    ("Hyd.Supply.Filter", "flow_control", {}),
    ("Hyd.Supply.Distributor", "flow_control", {}),
    ("Hyd.Supply.SimpleFlowSource", "", {"A": "P"}),
    # ohne Anschlussbezeichnung im Symbol heißen die Anschlüsse wie am Bauteil (A, B): Eingang 1, Ausgang 2
    ("PressureReducingValve", "", {"A": "1", "B": "2"}),
    ("Pneu.FlowValve.ThrottleValve", "", {"A": "1", "B": "2"}),
]


# FluidSIM-Bauteilklasse -> Funktion, unter der der Simulationskern das Bauteil rechnet
ALIAS = [
    ("Proportional.PressureReliefValve", "proportional.pressure_relief_valve"),   # R06, vor dem Druckbegrenzungsventil
    ("ThrottleCheckValve", "flow_control_valve.one_way_flow_control_valve"),
    ("CheckValvePilotToOpen", "shutoff_valve.check_valve"),
    ("CheckValve", "shutoff_valve.check_valve"),
    ("FlowValve.ThrottleValve", "flow_control_valve.throttle_valve"),
    ("FlowControlValve2Way", "flow_control_valve.throttle_valve"),
    ("FlowValve.ShutoffValve", "shutoff_valve.shutoff_valve"),
    ("ShutoffValve.ShuttleValve", "logic.shuttle_valve"),
    ("ShutoffValve.TwoPressureValve", "logic.dual_pressure_valve"),
    ("ShutoffValve.QuickExhaust", "shutoff_valve.quick_exhaust_valve"),
    ("PressureReliefValve", "pressure_valve.pressure_relief_valve01"),
    ("PressureReducingValve", "pressure_valve.pressure_regulator"),
    ("PressureRegulator", "pressure_valve.pressure_regulator"),
    ("Supply.PumpBlock", "supply.pump"),
    ("Supply.SimpleFlowSource", "supply.pump"),
    ("FlowValve.FlowDivider", "flow_control_valve.flow_divider"),
    ("NozzleVariable", "flow_control_valve.throttle_valve"),
    ("Supply.Accumulator", "supply.accumulator"),
    ("Pneu.Supply.AirServiceUnit", "pressure_valve.pressure_regulator"),
    ("Pneu.Supply.Filter", "flow_control_valve.throttle_valve"),
    ("Pneu.Supply.Lubricator", "flow_control_valve.throttle_valve"),
    ("Hyd.Supply.Filter", "flow_control_valve.throttle_valve"),
    ("Hyd.Supply.Distributor", "flow_control_valve.flow_divider"),
]


def simulationsfunktion(objekt, symbol, werte):
    """Funktion und Kennwerte für den Simulationskern: FluidSIM-Klassennamen, die er unter einem anderen Namen
    kennt, und Werte, die er anders erwartet (Einheit MPa·min²/l², Öffnung „level“ in %, Durchflussrichtung).

    Durchflussrichtung der Rückschlag- und Drosselrückschlagventile aus der Geometrie: frei ist die Richtung
    von der Spitze des Ventilsitzes zur Kugel, bei allen FluidSIM-Symbolen vom Symbolanschluss 2 zu 1 (beim
    entsperrbaren vom Anschluss A zu B). Das deckt sich mit den Beispielschaltungen, in denen das
    Drosselrückschlagventil zu 304 von 334 mit Anschluss 1 am Zylinder sitzt (Abluftdrosselung)."""
    ref = objekt.get("object-ref", "")
    fkt = fluid_kennwerte.funktion(ref)
    for teil, ziel in ALIAS:
        if teil in ref:
            fkt = ziel
            break
    w = {n: (v, u) for n, v, u in werte}
    out = []
    for n, v, u in werte:
        if u == "MPa.min2/l2":
            try:
                v, u = repr(float(v) * 10.0), "bar.min2/l2"
            except ValueError:
                pass
        out.append((n, v, u))

    def setze(name, wert, einheit=""):
        out[:] = [x for x in out if x[0] != name] + [(name, wert, einheit)]

    def zahl(name, vorgabe):
        try:
            return float(w[name][0])
        except (KeyError, ValueError):
            return vorgabe

    def bezeichnung(ident):
        con = next((c for c in symbol.iter("connector") if c.get("ident") == ident and c.get("type") != "electric-label"), None)
        return (con.get("designation") or "") if con is not None else ""

    level = zahl("level", 100.0)
    offen = max(level, 1.0) / 100.0
    if fkt in ("flow_control_valve.one_way_flow_control_valve", "flow_control_valve.throttle_valve"):
        if "qNN_n" in w:
            setze("qNN", repr(zahl("qNN_n", 85.0) * offen), "l/min")
        rt = next((n for n in ("rT_orifice", "rT") if n in w), None)
        if rt:
            rt_bar = dict((n, v) for n, v, _ in out)[rt]
            setze("rT", repr(float(rt_bar) / offen ** 2), "bar.min2/l2")
    if fkt == "shutoff_valve.shutoff_valve" and "level" in w and level <= 0:
        setze("rT", "1e9", "bar.min2/l2")          # Öffnung 0 %: geschlossen
    if fkt in ("flow_control_valve.one_way_flow_control_valve", "shutoff_valve.check_valve"):
        if "PilotToOpen" in ref:
            von, nach = "A", "B"
        else:
            von, nach = bezeichnung("2"), bezeichnung("1")
        if von and nach:
            setze("free_from", von)
            setze("free_to", nach)
    if fkt == "pressure_valve.pressure_relief_valve01" and "p0_prv" in w:
        setze("p0", w["p0_prv"][0], w["p0_prv"][1])
    if fkt == "supply.pump" and "p0" in w and "pMax" not in w:
        setze("pMax", w["p0"][0], w["p0"][1])
    if fkt == "supply.pump" and "q0" in w:
        # einfache Stromquelle: Förderstrom q0 als „Verdrängung q0 je Umdrehung bei 1/min“
        setze("displacement", w["q0"][0], "l" if w["q0"][1] == "l/min" else "")
        setze("rpm", "1", "1/min")
    if fkt == "supply.accumulator" and "vol" in w and "V0" not in w:
        setze("V0", w["vol"][0], w["vol"][1])
        if ref.startswith("Pneu."):
            setze("p0", "0", "bar")                # Druckluftbehälter: anfangs drucklos
    if fkt == "pressure_valve.pressure_regulator" and "p0_r" in w:
        setze("p0", w["p0_r"][0], w["p0_r"][1])
        setze("qNN", w.get("qNN_r", ("750", ""))[0], "l/min")
    if fkt == "flow_control_valve.throttle_valve" and "qNN_f" in w and "qNN" not in w:
        setze("qNN", w["qNN_f"][0], "l/min")
    return fkt, out


def magnete(symbol, objekt):
    """Ventilmagnete am Ventil: [(Seite, Kennzeichnung)] aus den Ebenen AEML…/AEMR… (Anschluss electric-label).
    Bei gespiegeltem oder um 180° gedrehtem Ventil sind die Seiten wie in der Zeichnung getauscht."""
    tr = fs.Trafo.von(objekt)
    tausch = (tr.sx < 0) != (tr.rot == 180)
    out = []
    for eb in symbol.findall("symbol-layer"):
        name = eb.get("ident") or ""
        if not name.startswith(("AEML", "AEMR")):
            continue
        seite = "left" if name.startswith("AEML") else "right"
        if tausch:
            seite = "right" if seite == "left" else "left"
        for con in eb.iter("connector"):
            if con.get("type") == "electric-label" and con.get("designation"):
                out.append((seite, con.get("designation")))
    return out


def elektro_rolle(objekt, typ, o):
    """Rolle eines elektrischen Bauteils für die Simulation (Elementinformation sim_role), oder ""."""
    if objekt in fs.ANALOG:
        return "analog"
    if objekt.startswith(("ET.Source.Pot24V", "ET.Source.L1", "ET.Source.L2", "ET.Source.L3")):
        return "supply+"
    if objekt.startswith(("ET.Source.Pot0V", "ET.Source.Ground")) or objekt == "ET.Source.N":
        return "supply-"
    if objekt == "ET.Source.Pot":
        try:
            return "supply+" if float(fluidsim_grafcet.werte(o).get("pot", "0")) > 0 else "supply-"
        except ValueError:
            return ""
    if objekt.startswith("ET.Source.ConstantVoltage"):
        return "voltage"
    if typ == "contact-manual" and "Changeover" not in objekt:
        art = "switch" if "Detent" in objekt and "NoDetent" not in objekt else "button"
        return art + ("_nc" if "Break" in objekt else "")
    if typ == "solenoid-valve-coil":
        return "solenoid"
    if objekt.startswith(("ET.PLC.GrafcetIO", "ET.PLC.GrafcetPLC")):
        return "grafcet_io"
    return ""


def wegmarken(alle, zylinder, schwenk=None):
    """Wegmarken der Wegmaßstäbe (Mech.FS3.DistanceRule): {Kennzeichnung: "-MM1 lo..hi"} (mm), jeweils am
    Zylinder, dem der Maßstab am nächsten liegt. Schwenkantriebe (R10) tragen ihre Marken selbst (label1..3,
    x11..x32 in Grad): der Sensor erfasst ±1°, eine Marke am Ende des Schwenkwinkels reicht bis in den
    Anschlag."""
    out = {}
    for o in alle:
        lbl = (schwenk or {}).get(o.get("ident"))
        if not lbl:
            continue
        w = fluidsim_grafcet.werte(o)
        try:
            xmax = float(w.get("xMax", "180"))
        except ValueError:
            xmax = 180.0
        for i in range(1, 7):
            name = w.get(f"label{i}", "").strip()
            try:
                lo, hi = float(w.get(f"x{i}1", "-1")), float(w.get(f"x{i}2", "-1"))
            except ValueError:
                continue
            if name and lo >= 0 and hi >= lo:
                lo2 = 0.0 if lo - 1.0 <= 0 else lo - 1.0
                hi2 = xmax + 1.0 if hi + 1.0 >= xmax else hi + 1.0
                out[name] = f"{lbl} {lo2:g}..{hi2:g}"
    for o in alle:
        if o.get("object-ref") != "Mech.FS3.DistanceRule" or not zylinder:
            continue
        m = fs.mitte(o)
        lbl = min(zylinder, key=lambda z: math.hypot(z[1][0] - m[0], z[1][1] - m[1]))[0]
        w = fluidsim_grafcet.werte(o)
        for i in range(1, 7):
            name = w.get(f"label{i}", "").strip()
            try:
                lo, hi = float(w.get(f"x{i}1", "-1")), float(w.get(f"x{i}2", "-1"))
            except ValueError:
                continue
            if name and lo >= 0 and hi >= lo:
                out[name] = f"{lbl} {lo:g}..{hi:g}"
    return out


def plan(circs, sammlung: Path, sprache, fluidsim_grafik=False):
    """Bauteile in sammlung schreiben; @return die Operationen für qet_edit und eine Zusammenfassung."""
    ops = []
    zahl = {"bauteile": 0, "leitungen": 0, "verknuepfungen": 0, "texte": 0, "ausgelassen": 0, "kennzeichnungen": 0,
            "rollen": 0, "schritte": 0, "transitionen": 0, "aktionen": 0}
    offen = []
    taster, handventile, ventile = [], [], []
    # alle Kennzeichnungen aus FluidSIM vorab, damit eine vergebene keiner späteren gleicht
    vergeben = set()
    for circ in circs:
        for o in objekte(lesen(circ)):
            k = o.find("dki-data/designation")
            if k is not None and k.get("value") and not k.get("value").startswith("?"):
                vergeben.add(k.get("value"))
    bauteile = []   # dict: folio, id, label, link, klasse, ventil_ziele [(comp-ref, label)], ident

    for blatt, circ in enumerate(circs):
        w = lesen(circ)
        grafik = abschnitt(w, "symbol-graphics")
        if grafik is None:
            continue
        ops.append({"op": "set_folio_border", "folio": blatt, "property": "preset", "value": "a3-landscape"})
        # leere Leitungsbeschriftungen nicht zeigen (sonst steht an jeder Leitung ein Strich)
        ops.append({"op": "set_conductor_default", "folio": blatt, "property": "displaytext", "value": "false"})
        symbole = {s.get("ident"): s for s in grafik.findall("symbol")}
        alle = objekte(w)
        verbindungen = list(w.iter("connect"))
        nach_id = {o.get("ident"): o for o in alle}
        co = abschnitt(w, "circuit-objects")
        textfelder = {t.get("ident"): t for t in co.iter("text-box")} if co is not None else {}
        hier = {}       # FluidSIM-Ident -> Bauteil dieses Blatts
        zylinder = []   # (Kennzeichnung, Mitte)
        schwenk = {}    # ident des Schwenkantriebs -> Kennzeichnung (R10)
        kontakte = []   # (Bauteil, Klasse, Objekt, Kennzeichnung) für Endschalter über Wegmarken

        for o in alle:
            objekt = o.get("object-ref") or ""
            if o.tag == "component" and (not objekt or objekt in fs.ABSCHLUSS or objekt.startswith(AUSLASSEN)
                                         or objekt.endswith("Interface.MultiPin") or objekt.startswith("Mech.")):
                # Verzweigungen werden Leitungen am selben Netz, Wegmaßstäbe Wegmarken (siehe unten)
                continue
            beschr = o.find("dki-data/description")
            en = (beschr.get("value") if beschr is not None else "") or objekt
            de = sprache.deutsch(en) if en else ""
            if not en:
                # Klemmen einer Klemmleiste haben keine Beschreibung
                en, de = ("Terminal", "Klemme") if o.tag == "terminal" else ("FluidSIM", "FluidSIM")
            # Eigene Grafik statt der FluidSIM-Symbole (eigene_grafik.py); nur die Anschlüsse und Daten bleiben
            quelle = symbole
            if not fluidsim_grafik and o.get("symbol-ref") in symbole:
                quelle = dict(symbole)
                quelle[o.get("symbol-ref")] = eigene_grafik.ersetzen(
                    symbole[o.get("symbol-ref")], o, fs.klasse(o), ventil_stellungen(symbole[o.get("symbol-ref")]))
            # Zwei Durchläufe: die UUIDs hängen am Inhalt, so werden gleiche Symbole ein Bauteil
            probe = fs.bauteil_umwandeln(quelle, o, alle, verbindungen, (de, en), circ.name, "probe", lage=True)
            if probe is None or not probe["uuids"]:
                zahl["ausgelassen"] += 1
                continue
            saat = "plan/" + hashlib.sha1(probe["text"].encode("utf-8")).hexdigest()
            erg = fs.bauteil_umwandeln(quelle, o, alle, verbindungen, (de, en), circ.name, saat, lage=True)
            erg["text"] = simulationsdaten(erg["text"], symbole[o.get("symbol-ref")], o)
            if not fluidsim_grafik:
                erg["text"] = re.sub(r"<informations>.*?</informations>",
                                     "<informations>Eigene Grafik nach DIN ISO 1219-1 / IEC 60617 (eigene_grafik.py); "
                                     "Anschlüsse und Kennwerte aus der FluidSIM-Schaltung "
                                     + fs.escape(circ.name) + " (" + fs.escape(objekt) + ").</informations>",
                                     erg["text"], count=1, flags=re.S)
            datei = sammlung / dateiname(de, saat)
            datei.write_text(erg["text"], encoding="utf-8", newline="\n")
            m = fs.mitte(o)
            x = round((m[0] + erg["anker"][0]) * fs.K, 1)
            y = round((m[1] + erg["anker"][1]) * fs.K, 1)
            oid = f"b{blatt}_{o.get('ident')}"
            ops.append({"op": "add_element", "id": oid, "folio": blatt, "path": f"common://{datei.name}",
                        "x": x, "y": y})
            el = "$" + oid
            kenn = o.find("dki-data/designation")
            label = kenn.get("value", "") if kenn is not None else ""
            if label.startswith("?"):
                label = ""
            art = re.search(r'<fluid kind="(valve|cylinder)"', erg["text"])
            schwenkt = "SemiRotaryMotor" in o.get("object-ref", "")   # Schwenkantrieb: wie ein Zylinder (R10)
            rolle = elektro_rolle(objekt, erg["klasse"], o)
            # Rollenhebelventile tragen ihre Kennzeichnung (= Wegmarke) an der Betätigung (mechanical-label)
            marke = next((c.get("designation") for c in symbole[o.get("symbol-ref")].iter("connector")
                          if c.get("type") == "mechanical-label" and c.get("designation")), "")
            if not label and art and art.group(1) == "valve" and marke:
                label = marke
            stetig = objekt.startswith("Hyd.Proportional.") and "Valve" in objekt
            if not label and (art or schwenkt or stetig or rolle == "analog" or rolle.startswith(("button", "switch"))):
                # Die Simulation bedient Ventile und Taster und meldet Zylinder über ihre Kennzeichnung: ohne
                # eine aus FluidSIM bekommen sie die nächste freie nach DIN EN 81346-2 (-QM1, -MM1, -SF1 …);
                # Analogbausteine (R06): Quellen -GF, Sensoren -BG, Regler und Verstärker -KF
                vorsilbe = ("-QM" if stetig else "-GF" if objekt.startswith("ET.Source.") else "-BG" if objekt.startswith("ET.Sensor.")
                            else "-KF" if rolle == "analog" else "-MM" if schwenkt else "-SF" if not art
                            else "-QM" if art.group(1) == "valve" else "-MM")
                k = 1
                while f"{vorsilbe}{k}" in vergeben:
                    k += 1
                label = f"{vorsilbe}{k}"
                zahl["kennzeichnungen"] += 1
            if label:
                vergeben.add(label)
                ops.append({"op": "set_label", "folio": blatt, "element": el, "label": label})
            ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "description", "value": de})
            if label and rolle.startswith(("button", "switch")):
                taster.append(label)
            if label and art and art.group(1) == "valve":
                ventile.append(label)
                if re.search(r'<actuation [^>]*types="[^"]*manual', erg["text"]):
                    handventile.append(label)
            if rolle:
                ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "sim_role", "value": rolle})
                zahl["rollen"] += 1
            if rolle in ("supply+", "supply-"):
                # R06: das Potential der Quelle, damit Analogbausteine Spannungen lesen (±15 V an A1/A2 …)
                pot = ("24" if objekt.startswith("ET.Source.Pot24V") else "0" if objekt.startswith(("ET.Source.Pot0V", "ET.Source.Ground"))
                       else fluidsim_grafcet.werte(o).get("pot") if objekt == "ET.Source.Pot" else None)
                if pot not in (None, ""):
                    ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "sim_volt", "value": pot})
            if rolle == "analog":
                # R06: FluidSIM-Klasse und alle Kennwerte mit Einheit („KP=1;KI=0 1/s;KD=0 ms“)
                kw = [f"{e.get('ident')}={e.get('value')}" + (f" {e.get('unit')}" if e.get("unit") else "")
                      for e in o if e.tag in ("const", "parameter") and e.get("ident") and e.get("ident") != "picture"
                      and e.get("value") not in (None, "") and ";" not in e.get("value")]
                ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "sim_block", "value": objekt[3:]})
                ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "sim_params", "value": ";".join(kw)})
            if rolle == "grafcet_io":
                # Ein- und Ausgänge mit den Namen, die der GRAFCET verwendet (LabelIn0 = -KF1 …)
                ww = fluidsim_grafcet.werte(o)
                zeilen = [f"I{i}={ww[f'LabelIn{i}']}" for i in range(16) if ww.get(f"LabelIn{i}", "").strip()]
                zeilen += [f"Q{i}={ww[f'LabelOut{i}']}" for i in range(16) if ww.get(f"LabelOut{i}", "").strip()]
                ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "sim_io", "value": ";".join(zeilen)})
            if art and art.group(1) == "cylinder" and label:
                zylinder.append((label, fs.mitte(o)))
            if "SemiRotaryMotor" in o.get("object-ref", "") and label:
                schwenk[o.get("ident")] = label
            if erg["klasse"] in ("make-contact", "break-contact", "proximity-switch") and label:
                kontakte.append((el, erg["klasse"], objekt, label))
            if art and art.group(1) == "valve" and (marke or label) and re.search(r'types="[^"]*roller', erg["text"]):
                kontakte.append((el, "valve", objekt, marke or label))
            # Ein Wert, der wie die Kennzeichnung lautet („24 V“ an der Spannungsquelle), steht schon da
            werte = [(v, tf) for v, tf in sichtbare_werte(o, textfelder) if v.replace(" ", "") != label.replace(" ", "")]
            if werte:
                ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "comment",
                            "value": "; ".join(v for v, _ in werte)})
            for wert, tf in werte:
                # In Bauteilkoordinaten: das Bauteil steht mit seinem Ursprung bei (x, y)
                ops.append({"op": "add_element_text", "folio": blatt, "element": el, "source": "text",
                            "value": wert, "x": round(fs.zahl(tf, "x1") * fs.K - x, 1),
                            "y": round(fs.zahl(tf, "y1") * fs.K - y, 1)})
                zahl["texte"] += 1
            ziele = []      # (comp-ref, Kennzeichnung, Seite)
            seiten = dict((lbl, s) for s, lbl in magnete(symbole[o.get("symbol-ref")], o))
            tr = fs.Trafo.von(o)
            tausch = (tr.sx < 0) != (tr.rot == 180)
            for con in o.findall("connector"):
                art = con.find("link-props/type")
                ziel = con.find("link-props/target")
                if art is not None and ziel is not None and art.get("value") == "solenoid-valve-coil-target":
                    seite = "left" if (con.get("con-ref") or "").startswith("AEML") else (
                        "right" if (con.get("con-ref") or "").startswith("AEMR") else "")
                    if seite and tausch:
                        seite = "right" if seite == "left" else "left"
                    ziele.append((ziel.get("comp-ref", ""), ziel.get("value", ""), seite))
            for lbl, s in seiten.items():
                if not any(z[1] == lbl for z in ziele):
                    ziele.append(("", lbl, s))
            b = {"folio": blatt, "id": oid, "label": label, "link": erg["link"], "klasse": erg["klasse"],
                 "uuids": erg["uuids"], "ziele": ziele, "ident": o.get("ident")}
            bauteile.append(b)
            hier[o.get("ident")] = b
            zahl["bauteile"] += 1

        # Endschalter: Kontakte, deren Kennzeichnung eine Wegmarke an einem Zylinder ist
        marken = wegmarken(alle, zylinder, schwenk)
        for el, klasse, objekt, label in kontakte:
            if label in marken and klasse == "valve":
                ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "function", "value": marken[label]})
                zahl["rollen"] += 1
            elif label in marken:
                ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "sim_role",
                            "value": "limit_nc" if klasse == "break-contact" else "limit"})
                ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "function", "value": marken[label]})
                zahl["rollen"] += 1

        # R06: Kopplungen der Analogbausteine als sim_link, über die Kennzeichnungen, die die Teile hier tragen:
        # Ventilmagnet „C=-QM1/S“, Verstärker „A=-QM1/Y1;B=-QM1/Y2“, Sensor „P=-BP1“, Wegmesssystem „X=-MM1“
        def kz(ident):
            b = hier.get(ident)
            return b["label"] if b else ""

        def ziel(c):
            z = c.find("link-props/target")
            return z if z is not None and z.get("comp-ref") else None

        def verstaerker_ausgang(amp, spule):
            """Ausgang A/B des Verstärkers, dessen Marke die Spule `spule` (-MB1) nennt, oder ""."""
            ausgaenge = [c for c in amp.findall("connector") if c.get("ident") in ("A", "B")]
            # zuerst der eindeutige Verweis am Verstärker („-QM1.-MB1“), dann die Beschriftung seiner Marke
            for c in ausgaenge:
                z = c.find("link-props/target")
                if z is not None and z.get("value", "").split(".")[-1] == spule:
                    return c.get("ident")
            sym = symbole.get(amp.get("symbol-ref"))
            for c in ausgaenge:
                nr = (c.get("con-ref") or "").rpartition(".")[2]
                if sym is not None and any(s.get("ident") == nr and s.get("designation") == spule
                                           for s in sym.iter("connector") if s.get("type") == "electric-label"):
                    return c.get("ident")
            return ""

        kopplung = {}
        verstaerker_fest = set()   # (Verstärker, Ausgang) aus dem Verweis am Verstärker
        # zwei Durchläufe: erst die Verweise an den Verstärkern, dann die an den Ventilen
        reihenfolge = sorted(alle, key=lambda o: 0 if (o.get("object-ref") or "").startswith("ET.Controller.PropAmplifier") else 1)
        for o in reihenfolge:
            ref, ident = o.get("object-ref") or "", o.get("ident")
            if ident not in hier:
                continue
            for c in o.findall("connector"):
                z = ziel(c)
                partner = nach_id.get(z.get("comp-ref")) if z is not None else None
                if partner is None or partner.get("ident") not in hier:
                    continue
                pref, spule = partner.get("object-ref") or "", z.get("value", "").split(".")[-1]
                if "Proportional." in ref and pref == "ET.Relay.ValveContSolenoid":
                    kopplung.setdefault(partner.get("ident"), {})["C"] = f"{kz(ident)}/{c.get('ident')}"
                elif "Proportional." in ref and pref.startswith("ET.Controller.PropAmplifier"):
                    # ohne Verweis wie in allen FluidSIM-Beispielen: Ausgang A an Y2, B an Y1 (FORMELN.md A10)
                    aus = verstaerker_ausgang(partner, spule) or ("A" if c.get("ident") == "Y2" else "B")
                    if (partner.get("ident"), aus) not in verstaerker_fest:
                        kopplung.setdefault(partner.get("ident"), {})[aus] = f"{kz(ident)}/{c.get('ident')}"
                elif ref.startswith("ET.Controller.PropAmplifier") and "Proportional." in pref:
                    vc = next((x.get("ident") for x in partner.findall("connector")
                               if ziel(x) is not None and ziel(x).get("value", "").split(".")[-1] == spule), None)
                    vc = vc or ("S" if any(x.get("ident") == "S" for x in partner.findall("connector"))
                                else "Y1" if c.get("ident") == "B" else "Y2")
                    # der Verweis am Verstärker selbst gilt vor dem am Ventil
                    kopplung.setdefault(ident, {})[c.get("ident")] = f"{kz(partner.get('ident'))}/{vc}"
                    verstaerker_fest.add((ident, c.get("ident")))
                elif ref.startswith("ET.Sensor.") and ref in fs.ANALOG and kz(partner.get("ident")):
                    kopplung.setdefault(ident, {})["P"] = kz(partner.get("ident"))
        for o in alle:
            # Wegmesssystem: der Zylinder nennt es in pos_label
            if o.get("object-ref") != "ET.Sensor.PositionSensor" or o.get("ident") not in hier:
                continue
            k = o.find("dki-data/designation")
            name = k.get("value", "") if k is not None else ""
            for z in alle:
                if name and z.get("ident") in hier and fluidsim_grafcet.werte(z).get("pos_label", "").strip() == name:
                    kopplung.setdefault(o.get("ident"), {})["X"] = kz(z.get("ident"))
        for ident, k in kopplung.items():
            ops.append({"op": "set_info", "folio": blatt, "element": "$" + hier[ident]["id"], "key": "sim_link",
                        "value": ";".join(f"{a}={b}" for a, b in sorted(k.items()))})
            zahl["rollen"] += 1

        # GRAFCET dieses Blatts
        g_ops, g_zahl, g_offen = fluidsim_grafcet.umwandeln(blatt, alle, verbindungen, sammlung, fs.K, fs.mitte)
        ops += g_ops
        for k in ("schritte", "transitionen", "aktionen"):
            zahl[k] += g_zahl[k]
        offen += [f"{circ.name}: {x}" for x in g_offen]

        # Das ganze Blatt so verschieben, dass es im Rahmen beginnt: FluidSIM-Schaltungen liegen teils bei
        # negativen Koordinaten oder höher als das QET-Blatt. Texte stehen relativ zum Bauteil und gehen mit.
        hier_ops = [op for op in ops if op["op"] == "add_element" and op["folio"] == blatt]
        if hier_ops:
            rand = 50.0
            dx = max(0.0, rand - min(op["x"] for op in hier_ops))
            dy = max(0.0, rand - min(op["y"] for op in hier_ops))
            for op in hier_ops:
                op["x"] = round(op["x"] + dx, 1)
                op["y"] = round(op["y"] + dy, 1)

        # Leitungen dieses Blatts
        def anschluss_uuid(bezug):
            oid, _, x = bezug.partition(".")
            b = hier.get(oid)
            if b is None:
                return None, None
            return b["id"], b["uuids"].get(fs.anschluss_schluessel(nach_id[oid], x))

        # Verzweigungen (Interface.MultiPin) sind in QET kein Bauteil: alle Anschlüsse, die über eine oder
        # mehrere Verzweigungen zusammenhängen, bilden ein Netz und werden der Reihe nach verbunden
        def ist_verzweigung(bezug):
            o = nach_id.get(bezug.partition(".")[0])
            return o is not None and (o.get("object-ref") or "").endswith("Interface.MultiPin")

        eltern = {}

        def wurzel(k):
            while eltern.setdefault(k, k) != k:
                eltern[k] = eltern[eltern[k]]
                k = eltern[k]
            return k

        im_netz = set()
        paare = []
        for v in verbindungen:
            a, b2 = v.get("con1", ""), v.get("con2", "")
            va, vb = ist_verzweigung(a), ist_verzweigung(b2)
            if not va and not vb:
                paare.append((a, b2))
                continue
            ka = a.partition(".")[0] if va else a
            kb = b2.partition(".")[0] if vb else b2
            eltern[wurzel(ka)] = wurzel(kb)
            im_netz.update((ka, kb))

        def lage(bezug):
            oid, _, x = bezug.partition(".")
            return anschluss_lage(symbole, nach_id[oid], x) or (0.0, 0.0)

        netze = {}
        for k in im_netz:
            if k not in nach_id:          # nicht die Verzweigung selbst
                netze.setdefault(wurzel(k), []).append(k)
        for glieder in netze.values():
            glieder.sort(key=lambda bz: (round(lage(bz)[1]), lage(bz)[0]))
            paare += list(zip(glieder, glieder[1:]))

        for a, b2 in paare:
            ea, ta = anschluss_uuid(a)
            eb, tb = anschluss_uuid(b2)
            if ea and eb and ta and tb:
                ops.append({"op": "add_conductor", "folio": blatt, "from": "$" + ea,
                            "from_terminal": "{" + ta + "}", "to": "$" + eb, "to_terminal": "{" + tb + "}"})
                zahl["leitungen"] += 1

    # Verknüpfungen, auch über Blattgrenzen: zuerst über den Verweis im selben Blatt, sonst über die Kennzeichnung
    verknuepft = set()

    def verbinde(master, slave, seite=""):
        if slave["id"] in verknuepft:
            return
        verknuepft.add(slave["id"])
        if seite:
            ops.append({"op": "set_info", "folio": slave["folio"], "element": "$" + slave["id"],
                        "key": "valve_side", "value": seite})
        # Vom Slave aus: QETs linkElements setzt die Verknüpfungen des ersten Elements auf genau das zweite;
        # vom Master aus würde jede weitere Spule (zweiter Magnet, weitere Kontakte) die vorige ersetzen
        ops.append({"op": "link_elements", "folio": slave["folio"], "element": "$" + slave["id"],
                    "to_folio": master["folio"], "to": "$" + master["id"]})
        zahl["verknuepfungen"] += 1

    spulen = [b for b in bauteile if b["link"] == "slave" and b["klasse"] == "solenoid-valve-coil"]
    for ventil in (b for b in bauteile if b["link"] == "master" and b["ziele"]):
        for comp_ref, ziel_label, seite in ventil["ziele"]:
            spule = next((s for s in spulen if s["folio"] == ventil["folio"] and comp_ref and s["ident"] == comp_ref), None)
            if spule is None and ziel_label:
                spule = next((s for s in spulen if s["label"] == ziel_label and s["id"] not in verknuepft
                              and s["folio"] == ventil["folio"]), None) or next(
                    (s for s in spulen if s["label"] == ziel_label and s["id"] not in verknuepft), None)
            if spule is not None:
                verbinde(ventil, spule, seite)
    relais = [b for b in bauteile if b["link"] == "master" and b["klasse"] == "solenoid-coil" and b["label"]]
    for kontakt in (b for b in bauteile if b["link"] == "slave" and b["klasse"] in fs.KONTAKT and b["label"]):
        spule = next((r for r in relais if r["label"] == kontakt["label"]), None)
        if spule is not None:
            verbinde(spule, kontakt)
    zahl["offen"] = offen
    zahl["taster"] = sorted(set(taster))
    zahl["handventile"] = sorted(set(handventile))
    zahl["ventile"] = sorted(set(ventile))
    return ops, zahl


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("pfade", nargs="*", type=Path, help=".circ-Dateien oder ein Ordner, zuletzt AUSGABE.qet")
    ap.add_argument("--qet", type=Path, default=_erster(HIER.parent / "QElectroTech.exe", REPO / "build" / "deploy" / "QElectroTech.exe"))
    ap.add_argument("--fluidsim", type=Path, default=FLUIDSIM, help="FluidSIM-Installation (für db/lang.db)")
    ap.add_argument("--titel", default=None)
    ap.add_argument("--pruefe-drehung", type=Path, metavar="ORDNER")
    ap.add_argument("--nur-zeigen", action="store_true", help="Operationen als JSON ausgeben, nichts schreiben")
    ap.add_argument("--fluidsim-grafik", action="store_true",
                    help="die Grafik der FluidSIM-Symbole übernehmen (nur mit eigener FluidSIM-Lizenz, nicht weitergeben)")
    ap.add_argument("--bericht", type=Path, help="Zusammenfassung als JSON (Zahlen, Taster, Handventile, offene Punkte)")
    a = ap.parse_args(argv)
    a.qet = a.qet.resolve()      # QElectroTech läuft unter qet_mcp in einem anderen Arbeitsordner

    if a.pruefe_drehung:
        pruefe_drehung(a.pruefe_drehung)
        return
    pfade = list(a.pfade)
    ausgabe = None
    if pfade and pfade[-1].suffix.lower() == ".qet":
        ausgabe = pfade.pop()
    circs = []
    for p in pfade:
        circs += sorted(p.glob("*.circ")) if p.is_dir() else [p]
    if not circs or (ausgabe is None and not a.nur_zeigen):
        ap.error("SCHALTUNG.circ (oder Ordner) und AUSGABE.qet angeben")

    sprache = fs.Uebersetzung(a.fluidsim / "db" / "lang.db")
    with tempfile.TemporaryDirectory(prefix="fluidsim-plan-") as tmp:
        sammlung = Path(tmp) / "elements"
        sammlung.mkdir()
        g_quelle = GRAFCET_SAMMLUNG / fluidsim_grafcet.G
        g_ziel = sammlung / fluidsim_grafcet.G
        g_ziel.mkdir(parents=True)
        for f in g_quelle.glob("*.elmt"):
            (g_ziel / f.name).write_bytes(f.read_bytes())
        ops, zahl = plan(circs, sammlung, sprache, a.fluidsim_grafik)
        if a.nur_zeigen:
            print(json.dumps(ops, indent=1, ensure_ascii=False))
            print(zahl)
            return
        sys.path.insert(0, str(MCP))
        try:
            import qet_mcp  # noqa: E402
        except SyntaxError as e:
            sys.exit(f"misc/qet-mcp/qet_mcp.py lässt sich nicht laden: {e}")
        os.environ["QET_ENABLE_SCRIPTING"] = "1"
        leer = Path(tmp) / "leer.qet"
        titel = a.titel or (circs[0].stem if len(circs) == 1 else circs[0].parent.name)
        qet_mcp.tool_project_new(str(a.qet), str(leer), title=titel, folios=[c.stem for c in circs],
                                 elements_dir=str(sammlung))
        if ops:
            erg = qet_mcp.tool_edit(str(a.qet), str(leer), ops, str(ausgabe), elements_dir=str(sammlung),
                                    timeout=600)
        else:
            # nichts Übernehmbares (z. B. nur Digitaltechnik): das leere Projekt mit seinen Blättern
            Path(ausgabe).write_bytes(leer.read_bytes())
            erg = {"saved": True, "operations": []}
            zahl["offen"].append("keine übernehmbaren Bauteile (Digitaltechnik/LOGO wird nicht übernommen)")
        fehler = [o for o in erg.get("operations", []) if not o.get("succeeded")]
        print(f"{ausgabe}: {len(circs)} Blatt/Blätter, {zahl['bauteile']} Bauteile, {zahl['leitungen']} Leitungen, "
              f"{zahl['verknuepfungen']} Verknüpfungen, {zahl['texte']} Werte als Text, "
              f"{zahl['kennzeichnungen']} Kennzeichnungen vergeben, {zahl['rollen']} Simulationsrollen, "
              f"GRAFCET {zahl['schritte']} Schritte/{zahl['transitionen']} Transitionen/{zahl['aktionen']} Aktionen, "
              f"{zahl['ausgelassen']} Objekte ohne Anschluss ausgelassen; {len(fehler)} Operationen fehlgeschlagen.")
        for o in fehler[:10]:
            print("  FEHLER", json.dumps(o, ensure_ascii=False))
        for x in zahl["offen"]:
            print("  nicht übernommen:", x)
        if not erg.get("saved"):
            sys.exit("Projekt nicht gespeichert: " + json.dumps(erg, ensure_ascii=False)[:2000])
        print(f"fertig: {ausgabe}")
        if a.bericht:
            a.bericht.write_text(json.dumps(dict(zahl, fehlops=len(fehler), fehler=fehler[:10]), ensure_ascii=False,
                                            indent=1), encoding="utf-8")


if __name__ == "__main__":
    main()
