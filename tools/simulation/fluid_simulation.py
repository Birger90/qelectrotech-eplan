"""Pneumatik-Simulation eines QElectroTech-Projekts (Prototyp, ohne Oberfläche).

Liest ein .qet-Projekt, baut aus den Leitungen die Druckluftnetze und rechnet mit den Simulationsdaten der Symbole
(<fluid>: Stellungen, Schaltwege, Betätigung, Kennwerte):

- Netz = Knoten mit Volumen und Druck (isotherm, ideales Gas). Offene Anschlüsse ohne Leitung entlüften ins Freie.
- Druckluftquelle hält ihren Knoten auf dem Versorgungsdruck.
- Wegeventil: die Schaltwege der aktuellen Stellung verbinden Knoten; Durchfluss nach ISO 6358 aus dem
  Normalnenndurchfluss qNN (bei 6 bar Überdruck und 1 bar Druckabfall), kritisches Druckverhältnis b = 0,3.
  Schaltet nach Betätigung (Magnet, Steuerdruck, Hand) mit Schaltzeit, sonst Federrückstellung in die Ruhestellung.
- Zylinder: Kammern A (Kolben) und B (Ring) als veränderliche Volumen, Kraftbilanz mit Masse, Haft- und
  Gleitreibung, Feder und Endanschlägen.

Eingaben sind Ereignisse „Zeit Kennzeichen Seite an|aus“, z. B. „0.1 -QM1 links an“. Ausgabe: Zeitreihe (CSV),
Zustandsdiagramm (PNG) und Meldungen (Endlage erreicht, Ventil schaltet).

    python tools/simulation/fluid_simulation.py plan.qet --ereignis "0.1 -QM1 links an" \\
        --ereignis "1.0 -QM1 links aus" --dauer 2 --diagramm weg.png --csv werte.csv
"""
import argparse
import csv
import math
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

P_ATM = 1.013e5          # Pa
T = 293.15               # K
R = 287.0                # J/(kg K)
RHO_N = 1.1845           # kg/m³ Normdichte im technischen Normzustand von FluidSIM (20 °C, 1 bar)
B_KRIT = 0.4             # kritisches Druckverhältnis (FluidSIM: b = 0,4 für alle Komponenten)
V_LEITUNG = 5e-6         # m³ je Netz (Leitung, Verschraubungen)
V_TOT = 2e-6             # m³ Totvolumen je Zylinderkammer
P_STEUER = 1.5e5         # Pa Überdruck, ab dem ein Steuerdruck schaltet
# Hydraulik
E_OEL = 7e8              # Pa wirksamer Kompressionsmodul (Öl mit Schläuchen)
V_HYD = 1e-4             # m³ je Netz (Leitung)
DP_LAMINAR = 0.3e5       # Pa: darunter linearer Widerstand (vermeidet unendliche Steigung bei 0)


def bar(p):
    return (p - P_ATM) / 1e5


# ----------------------------------------------------------------------------------------------- Projekt lesen
class Bauteil:
    def __init__(self, uuid, label, fluid, anschluesse, typ="", infos=None, richtung=None, namen=None):
        self.uuid, self.label, self.fluid, self.anschluesse = uuid, label, fluid, anschluesse
        self.typ, self.infos = typ, infos or {}
        self.richtung = richtung or {}            # Anschluss-UUID -> n/s/e/w
        self.name = namen or ""
        self.links, self.link_type, self.art_infos = [], "", {}
        self.medium = ""
        self.kind = fluid.get("kind") if fluid is not None else None
        self.kw = {}
        if fluid is not None:
            for p in fluid.findall("parameter"):
                try:
                    self.kw[p.get("name")] = float(p.get("value"))
                except (TypeError, ValueError):
                    pass

    def wert(self, name, vorgabe):
        return self.kw.get(name, vorgabe)


def projekt_lesen(pfad):
    root = ET.parse(pfad).getroot()
    definitionen = {}
    col = root.find("collection")
    if col is not None:
        def gehe(knoten, weg):
            for k in knoten:
                if k.tag == "category":
                    gehe(k, weg + [k.get("name")])
                elif k.tag == "element":
                    definitionen["/".join(weg + [k.get("name")])] = k.find("definition")
        gehe(col, [])
    bauteile = {}
    leitungen = []
    for diagramm in root.iter("diagram"):
        for e in diagramm.iter("element"):
            schluessel = e.get("type", "").split("embed://", 1)[-1]
            d = definitionen.get(schluessel)
            if d is None:
                continue
            infos = {i.get("name"): (i.text or "").strip() for i in e.iter("elementInformation")}
            namen = {t.get("uuid"): t.get("name") or "" for t in d.iter("terminal")}
            richtung = {t.get("uuid"): t.get("orientation") for t in d.iter("terminal")}
            name = " ".join(n.text or "" for n in d.iter("name") if n.get("lang") in ("de", "en", "fr"))
            b = Bauteil(e.get("uuid"), infos.get("label", ""), d.find("fluid"), namen,
                        schluessel, infos, richtung, name)
            b.links = [l.get("uuid") for l in e.iter("link_uuid")]
            b.medium = next((x.get("medium") for x in d.iter("terminal") if x.get("medium")), "")
            b.link_type = d.get("link_type", "")
            b.art_infos = {k.get("name"): (k.text or "").strip() for k in d.iter("kindInformation")}
            bauteile[e.get("uuid")] = b
        for c in diagramm.iter("conductor"):
            leitungen.append(((c.get("element1"), c.get("terminal1")), (c.get("element2"), c.get("terminal2"))))
    return bauteile, leitungen


# ----------------------------------------------------------------------------------------------- Netz
class Netz:
    def __init__(self, bauteile, leitungen):
        self.eltern = {}
        self.bauteile = bauteile
        for b in bauteile.values():
            for tu in b.anschluesse:
                self.eltern[(b.uuid, tu)] = (b.uuid, tu)
        self.verbunden = set()
        for (e1, t1), (e2, t2) in leitungen:
            if e1 in bauteile and e2 in bauteile:
                a, z = (e1, t1), (e2, t2)
                if a in self.eltern and z in self.eltern:
                    self.eltern[self.wurzel(a)] = self.wurzel(z)
                    self.verbunden |= {a, z}
        wurzeln = sorted({self.wurzel(k) for k in self.eltern})
        self.index = {w: i for i, w in enumerate(wurzeln)}

    def wurzel(self, k):
        while self.eltern[k] != k:
            self.eltern[k] = self.eltern[self.eltern[k]]
            k = self.eltern[k]
        return k

    def knoten(self, uuid, anschluss):
        """Knotennummer eines Anschlusses (Name); None, wenn er offen ins Freie geht."""
        tu = next((u for u, n in self.bauteile[uuid].anschluesse.items() if n == anschluss), None)
        return self.knoten_t(uuid, tu)

    def knoten_t(self, uuid, tu):
        """Knotennummer eines Anschlusses (UUID); None ohne Leitung."""
        k = (uuid, tu)
        if k not in self.eltern or k not in self.verbunden:
            return None
        return self.index[self.wurzel(k)]


# ----------------------------------------------------------------------------------------------- Modelle
def massenstrom(p1, p2, c):
    """ISO 6358: Massenstrom von p1 nach p2 (kg/s), c = Leitwert in m³/(s·Pa) (Normvolumen)."""
    if p1 < p2:
        return -massenstrom(p2, p1, c)
    if p1 <= 0:
        return 0.0
    r = p2 / p1
    f = 1.0 if r <= B_KRIT else math.sqrt(max(0.0, 1.0 - ((r - B_KRIT) / (1.0 - B_KRIT)) ** 2))
    return c * p1 * f * RHO_N


def leitwert_aus_qnn(qnn_l_min):
    """Normalnenndurchfluss (l/min bei 6 bar Ü, 1 bar Druckabfall) -> Leitwert C."""
    p1 = P_ATM + 6e5
    r = (p1 - 1e5) / p1
    f = math.sqrt(1.0 - ((r - B_KRIT) / (1.0 - B_KRIT)) ** 2)
    return (qnn_l_min / 1000 / 60) / (p1 * f)


def volumenstrom(p1, p2, rt):
    """Hydraulik: Volumenstrom (m³/s) von p1 nach p2 durch einen Widerstand rT (bar·min²/l²): Δp = rT·Q²."""
    dp = p1 - p2
    if rt <= 0:
        rt = 1e-4
    k = 1.0 / (60000.0 * math.sqrt(1e5 * rt))      # wie SimPhysics::orificeFlow
    return k * dp / (dp * dp + DP_LAMINAR * DP_LAMINAR) ** 0.25


class Pumpe:
    """Hydraulikpumpe: Förderstrom = Verdrängung × Drehzahl, eingebaute Druckbegrenzung bei pMax."""

    def __init__(self, b):
        self.b = b
        self.q = b.wert("displacement", 8.0) * 1e-6 * b.wert("rpm", 1450) / 60
        pmax = b.wert("pMax", 6.0)
        self.p_max = P_ATM + (pmax * 1e6 if pmax < 100 else pmax * 1e5)   # MPa bzw. bar
        self.band = 1e5                     # wie im C++-Kern: volle Förderung bis pMax - 1 bar


class Stromventil:
    """Durchflussbauteile ohne Schaltstellungen. Pneumatik: Leitwert aus qNN, Hydraulik: Widerstand rT."""

    STANDARD_QNN = {"supply.silencer": 300.0, "shutoff_valve.quick_exhaust_valve": 300.0,
                    "shutoff_valve.check_valve": 150.0, "shutoff_valve.shutoff_valve": 200.0,
                    "shutoff_valve.three_way_valve": 200.0, "flow_control_valve.throttle_valve": 40.0,
                    "flow_control_valve.one_way_flow_control_valve": 40.0,
                    "pressure_valve.pressure_regulator": 200.0, "pressure_valve.pressure_relief_valve01": 200.0,
                    "flow_control_valve.flow_divider": 100.0}

    def __init__(self, b):
        self.b = b
        self.fkt = b.fluid.get("function", "")
        self.hydraulisch = b.medium == "hydraulic"
        self.c = leitwert_aus_qnn(b.wert("qNN", self.STANDARD_QNN.get(self.fkt, 100.0)))
        self.c_frei = leitwert_aus_qnn(150.0)
        self.rt = b.wert("rT", 1.0 if "throttle" in self.fkt or "flow_control" in self.fkt else 0.1)
        self.rt_frei = 0.05
        self.p0 = b.wert("p0", 0.0) * 1e6           # MPa -> Pa Überdruck
        self.dp_end = max(b.wert("dpEnd", 0.05) * 1e6, 0.1e5)
        param = {p.get("name"): p.get("value") for p in b.fluid.findall("parameter")}
        self.frei = (param.get("free_from"), param.get("free_to"))
        self.namen = list(b.anschluesse.values())

    def fluss(self, pa, pb, anteil=1.0, frei=False):
        if anteil <= 0:
            return 0.0
        if self.hydraulisch:
            rt = (self.rt_frei if frei else self.rt) / max(anteil, 1e-3) ** 2
            return volumenstrom(pa, pb, rt)
        return massenstrom(pa, pb, (self.c_frei if frei else self.c) * anteil)


class Ventil:
    def __init__(self, b):
        self.b = b
        pos = b.fluid.find("positions")
        self.stellungen = [[(w.get("from"), w.get("to")) for w in ps.findall("path") if w.get("to")]
                           for ps in pos.findall("position")]
        self.ruhe = int(pos.get("initial", "1")) - 1
        self.stellung = self.ziel = self.ruhe
        self.seiten = {a.get("side"): a for a in b.fluid.findall("actuation")}
        self.betaetigt = {"left": False, "right": False}
        self.umschalten_um = None
        self.umschalt_beginn = None
        self.c = leitwert_aus_qnn(b.wert("qNN", 60.0))
        self.rt = b.wert("rT", 0.1)
        self.hydraulisch = b.medium == "hydraulic"
        self.t_ein = b.wert("onDelay", 0.02)
        self.t_aus = b.wert("offDelay", 0.03)

    def soll(self):
        l, r = self.betaetigt["left"], self.betaetigt["right"]
        n = len(self.stellungen)
        if l and not r:
            return 0
        if r and not l:
            return n - 1
        if l and r:
            return self.stellung
        federn = any(a.get("spring") == "true" for a in self.seiten.values())
        return self.ruhe if federn else self.stellung   # ohne Feder (Raste, Impuls) bleibt sie


class Zylinder:
    def __init__(self, b):
        self.b = b
        c = b.fluid.find("cylinder")
        d = b.wert("d_piston", float(c.get("bore_mm", 20)) if c is not None else 20.0) / 1000
        s = b.wert("d_rod", float(c.get("rod_mm")) if c is not None and c.get("rod_mm") else 8.0) / 1000
        self.hub = b.wert("xMax", float(c.get("stroke_mm", 100)) if c is not None else 100.0) / 1000
        self.a_k = math.pi * d * d / 4
        self.a_r = self.a_k - math.pi * s * s / 4
        self.m = b.wert("m_cyl", 2.0 if b.medium == "hydraulic" else 0.1) + b.wert("m_ext", 0.0)
        # Reibung wie üblich ~5 % der Kolbenkraft bei 6 bar (Haften), Gleiten 70 % davon
        self.f_haft = b.wert("F_stat", 0.05 * self.a_k * 6e5)
        self.f_gleit = 0.7 * self.f_haft
        self.d_visk = 20.0
        self.feder = c is not None and c.get("spring") in ("rod", "piston")
        self.f_feder = 0.15 * self.a_k * 6e5 if self.feder else 0.0
        self.f_last = b.wert("F_extern_const", 0.0)
        self.x = b.wert("xStart", 0.0) / 1000
        self.v = 0.0


# ----------------------------------------------------------------------------------------------- Elektrik
def elektro_art(b):
    """Rolle eines Elektrosymbols aus seinem Sammlungspfad bzw. Namen."""
    weg, name = b.typ.lower(), b.name.lower()
    if b.link_type == "master" and b.art_infos.get("type") == "coil":
        return "relais", False
    if b.link_type == "slave" and b.art_infos.get("state") in ("NO", "NC"):
        return "relaiskontakt", b.art_infos.get("state") == "NC"
    oeffner = any(w in name for w in ("öffner", "oeffner", " nc", "(nc)", " nf")) \
        or weg.endswith(("_nf.elmt", "_nc.elmt"))
    if "voltage_current_sources" in weg or "spannungsquelle" in name or "voltage source" in name:
        return "quelle", False
    if "push_buttons" in weg or "taster" in name or "push-button" in name:
        return "taster", oeffner
    if "limit_switches" in weg or "endschalter" in name or "limitswitch" in name or "grenztaster" in name:
        return "endschalter", oeffner
    if "20_valves" in weg or "magnetventil" in name or "electrovanne" in name or "ventilspule" in name:
        return "spule", False
    return None, False


def zeit_aus_text(text, vorgabe=1.0):
    """„1 s“, „t=1,5s“, „500 ms“ -> Sekunden."""
    m = re.search(r"(\d+(?:[.,]\d+)?)\s*(ms|s)?\b", text or "")
    if not m:
        return vorgabe
    wert = float(m.group(1).replace(",", "."))
    return wert / 1000 if m.group(2) == "ms" else wert


def verzoegerung(b):
    """Art der Verzögerung eines Relais aus Name bzw. Kontaktart: 'ein', 'aus' oder None."""
    art = b.art_infos.get("type", "")
    name = b.name.lower()
    if art == "delayOn" or "ansprechverz" in name or "tempo_travail" in b.typ:
        return "ein"
    if art == "delayOff" or "abfallverz" in name or "rückfallverz" in name or "tempo_repos" in b.typ:
        return "aus"
    return None


class Elektrik:
    """Stromlaufplan als Schaltnetz: Kontakte offen/geschlossen, Spule an, wenn ein Anschluss mit Plus und der
    andere mit Minus verbunden ist. Gleichspannungsquelle: oberer Anschluss Plus, unterer Minus."""

    def __init__(self, netz, bauteile):
        self.netz = netz
        self.quellen, self.kontakte, self.spulen = [], [], []
        self.relais = {}            # uuid -> {"b", "knoten", "an", "ziel", "um"}
        self.gedrueckt = set()
        self.t_anzug, self.t_abfall = 0.010, 0.008
        for b in bauteile.values():
            art, oeffner = elektro_art(b)
            if art is None or b.fluid is not None:
                continue
            knoten = [(netz.knoten_t(b.uuid, tu), b.richtung.get(tu)) for tu in b.anschluesse]
            if art == "quelle":
                plus = [k for k, r in knoten if r in ("n", "w")]
                minus = [k for k, r in knoten if r in ("s", "e")]
                self.quellen.append((plus, minus))
            elif art in ("taster", "endschalter"):
                self.kontakte.append({"b": b, "art": art, "oeffner": oeffner, "knoten": [k for k, _ in knoten],
                                      "ausloeser": b.infos.get("function", "")})
            elif art == "spule":
                self.spulen.append({"b": b, "knoten": [k for k, _ in knoten], "an": False})
            elif art == "relais":
                text = b.infos.get("function", "") or b.infos.get("comment", "")
                self.relais[b.uuid] = {"b": b, "knoten": [k for k, _ in knoten], "an": False, "ziel": False,
                                       "um": None, "modus": verzoegerung(b), "zeit": zeit_aus_text(text),
                                       "an_seit": None, "aus_seit": None}
            elif art == "relaiskontakt":
                self.kontakte.append({"b": b, "art": art, "oeffner": oeffner, "knoten": [k for k, _ in knoten],
                                      "relais": b.links[0] if b.links else None, "modus": verzoegerung(b)})

    def geschlossen(self, k, endlagen, t=0.0):
        if k["art"] == "taster":
            betaetigt = k["b"].label in self.gedrueckt
        elif k["art"] == "relaiskontakt":
            r = self.relais.get(k["relais"])
            if not r:
                betaetigt = False
            else:
                modus = k["modus"] or r["modus"]
                if modus == "ein":
                    betaetigt = r["an"] and r["an_seit"] is not None and t - r["an_seit"] >= r["zeit"]
                elif modus == "aus":
                    betaetigt = r["an"] or (r["aus_seit"] is not None and t - r["aus_seit"] < r["zeit"])
                else:
                    betaetigt = r["an"]
        else:   # Endschalter: Funktionstext „-MM1 vorne“ bzw. „-MM1 hinten“
            teile = k["ausloeser"].split()
            betaetigt = len(teile) == 2 and endlagen.get(teile[0]) == teile[1]
        return betaetigt != k["oeffner"]

    def rechnen(self, endlagen, t=0.0, meldungen=None):
        eltern = {}

        def w(a):
            eltern.setdefault(a, a)
            while eltern[a] != a:
                eltern[a] = eltern[eltern[a]]
                a = eltern[a]
            return a
        for k in self.kontakte:
            kn = k["knoten"]
            if len(kn) >= 2 and None not in kn[:2] and self.geschlossen(k, endlagen, t):
                eltern[w(kn[0])] = w(kn[1])
        plus = {w(k) for p, m in self.quellen for k in p if k is not None}
        minus = {w(k) for p, m in self.quellen for k in m if k is not None}
        def bestromt(knoten):
            a, b = (knoten + [None, None])[:2]
            return a is not None and b is not None and (
                (w(a) in plus and w(b) in minus) or (w(a) in minus and w(b) in plus))
        for s in self.spulen:
            s["an"] = bestromt(s["knoten"])
        for r in self.relais.values():       # Relais ziehen mit Anzug-/Abfallzeit an bzw. ab
            soll = bestromt(r["knoten"])
            if soll != r["ziel"]:
                r["ziel"], r["um"] = soll, t + (self.t_anzug if soll else self.t_abfall)
            if r["um"] is not None and t >= r["um"]:
                if r["an"] != r["ziel"] and meldungen is not None:
                    meldungen.append((t, f"Relais {r['b'].label} {'zieht an' if r['ziel'] else 'fällt ab'}"))
                if r["an"] != r["ziel"]:
                    r["an_seit" if r["ziel"] else "aus_seit"] = t
                r["an"], r["um"] = r["ziel"], None
        return self.spulen


# ----------------------------------------------------------------------------------------------- Simulation
class Simulation:
    def __init__(self, pfad, p_versorgung_bar=6.0):
        bauteile, leitungen = projekt_lesen(pfad)
        self.netz = Netz(bauteile, leitungen)
        n = len(self.netz.index)
        self.p = [P_ATM] * n
        self.fest = {}
        self.ventile, self.zylinder, self.pumpen, self.meldungen = [], [], [], []
        self.stromventile = []
        p_vers = P_ATM + p_versorgung_bar * 1e5
        self.hyd = set()
        for b in bauteile.values():
            if b.medium == "hydraulic":
                for tu in b.anschluesse:
                    k = self.netz.knoten_t(b.uuid, tu)
                    if k is not None:
                        self.hyd.add(k)
        for b in bauteile.values():
            if b.fluid is None:
                continue
            fkt = b.fluid.get("function", "")
            if b.kind == "source" and fkt == "supply.tank":
                for tu in b.anschluesse:
                    k = self.netz.knoten_t(b.uuid, tu)
                    if k is not None:
                        self.fest[k] = P_ATM
            elif b.kind == "source" and b.medium == "hydraulic" and "pump" in fkt:
                self.pumpen.append(Pumpe(b))
            elif b.kind == "source":
                for tu in b.anschluesse:
                    k = self.netz.knoten_t(b.uuid, tu)
                    if k is not None:
                        self.fest[k] = p_vers
            elif b.kind == "valve" and b.fluid.find("positions") is not None:
                self.ventile.append(Ventil(b))
            elif b.kind == "cylinder":
                self.zylinder.append(Zylinder(b))
            elif b.kind in ("flow_control", "treatment") and fkt in Stromventil.STANDARD_QNN:
                self.stromventile.append(Stromventil(b))
        for k, p in self.fest.items():
            self.p[k] = p
        self.nach_label = {v.b.label: v for v in self.ventile}
        self.endlage = {z.b.label: None for z in self.zylinder}
        self.elektrik = Elektrik(self.netz, bauteile)
        self.spule_an = {}

    def spulen_auf_ventile(self, t):
        """Spule -QM1 (bzw. -QM1/14 links, -QM1/12 rechts) betätigt den Magneten des Ventils -QM1."""
        for s in self.elektrik.rechnen(self.endlage, t, self.meldungen):
            label = s["b"].label
            if label not in self.spule_an:
                self.spule_an[label] = s["an"]
            elif self.spule_an[label] != s["an"]:
                self.spule_an[label] = s["an"]
                self.meldungen.append((t, f"Spule {label} {'an' if s['an'] else 'aus'}"))
            ventil, _, seite = label.partition("/")
            v = self.nach_label.get(ventil)
            if v is None:
                continue
            if seite in ("14", "Y1", "1"):
                v.betaetigt["left"] = s["an"]
            elif seite in ("12", "Y2", "2"):
                v.betaetigt["right"] = s["an"]
            else:
                magnet = next((sd for sd, a in v.seiten.items() if "solenoid" in (a.get("types") or "")), "left")
                v.betaetigt[magnet] = s["an"]

    def kammer(self, z, name):
        return self.netz.knoten(z.b.uuid, name)

    def schritt(self, t, dt):
        n = len(self.p)
        mdot, dvdt = [0.0] * n, [0.0] * n            # Pneumatik: kg/s, Hydraulik: m³/s
        vol = [V_HYD if k in self.hyd else V_LEITUNG for k in range(n)]
        for pu in self.pumpen:
            kp, kt = self.netz.knoten(pu.b.uuid, "P"), self.netz.knoten(pu.b.uuid, "T")
            if kp is not None:
                anteil = min(1.0, max(0.0, (pu.p_max - self.p[kp]) / pu.band))
                mdot[kp] += pu.q * anteil
                if kt is not None:
                    mdot[kt] -= pu.q * anteil
        for v in self.ventile:
            for seite, a in v.seiten.items():
                if a.get("pilot") == "true" and "solenoid" not in (a.get("types") or ""):
                    k = self.netz.knoten(v.b.uuid, "14" if seite == "left" else "12")
                    if k is not None:
                        v.betaetigt[seite] = self.p[k] - P_ATM > P_STEUER
            ziel = v.soll()
            if ziel != v.ziel:
                v.ziel = ziel
                v.umschalt_beginn = t
                v.umschalten_um = t + (v.t_ein if ziel != v.ruhe else v.t_aus)
            if v.umschalten_um is not None and t >= v.umschalten_um:
                if v.stellung != v.ziel:
                    self.meldungen.append((t, f"{v.b.label} schaltet in Stellung {v.ziel + 1}"))
                v.stellung, v.umschalten_um = v.ziel, None
            # Schieber wie ein Stellglied erster Ordnung: neue Wege öffnen mit 1 - e^(-t/tau), tau = Schaltzeit/3,
            # alte schließen ebenso; wie im C++-Kern
            offen = 1.0
            if v.umschalten_um is not None and v.umschalten_um > v.umschalt_beginn:
                offen = min(1.0, max(0.0, 1.0 - math.exp(-3.0 * (t - v.umschalt_beginn) / (v.umschalten_um - v.umschalt_beginn))))
            if v.umschalten_um is not None and v.ziel != v.stellung:
                anteile = [(v.stellung, 1.0 - offen), (v.ziel, offen)]
            else:
                anteile = [(v.stellung, 1.0)]
            for stellung, anteil in anteile:
                if anteil <= 0.0:
                    continue
                for a, b in v.stellungen[stellung]:
                    ka, kb = self.netz.knoten(v.b.uuid, a), self.netz.knoten(v.b.uuid, b)
                    pa = self.p[ka] if ka is not None else P_ATM
                    pb = self.p[kb] if kb is not None else P_ATM
                    m = anteil * (volumenstrom(pa, pb, v.rt) if v.hydraulisch else massenstrom(pa, pb, v.c))
                    if ka is not None:
                        mdot[ka] -= m
                    if kb is not None:
                        mdot[kb] += m
        def druck(k):
            return self.p[k] if k is not None else P_ATM

        def buche(ka, kb, m):
            if ka is not None:
                mdot[ka] -= m
            if kb is not None:
                mdot[kb] += m

        for s in self.stromventile:
            kn = {n: self.netz.knoten(s.b.uuid, n) for n in s.namen}
            f = s.fkt
            if f == "supply.silencer":
                for k in kn.values():
                    buche(k, None, s.fluss(druck(k), P_ATM))
            elif f in ("flow_control_valve.throttle_valve", "shutoff_valve.shutoff_valve"):
                a, b = (s.namen + [None, None])[:2]
                if a and b:
                    buche(kn[a], kn[b], s.fluss(druck(kn[a]), druck(kn[b])))
            elif f in ("shutoff_valve.check_valve", "flow_control_valve.one_way_flow_control_valve"):
                von, nach = s.frei
                if von not in kn or nach not in kn:
                    continue
                pv, pn = druck(kn[von]), druck(kn[nach])
                if f == "flow_control_valve.one_way_flow_control_valve":
                    buche(kn[von], kn[nach], s.fluss(pv, pn))                    # Drossel in beide Richtungen
                entsperrt = False
                steuer = [n for n in s.namen if n not in (von, nach)]
                if steuer and kn.get(steuer[0]) is not None:                       # entsperrbar
                    entsperrt = druck(kn[steuer[0]]) - P_ATM > P_STEUER
                if entsperrt:
                    buche(kn[von], kn[nach], s.fluss(pv, pn, frei=True))
                elif pv - pn > s.p0:                                               # stetig ab Öffnungsdruck
                    buche(kn[von], kn[nach], s.fluss(pv - s.p0, pn, frei=True))
            elif f == "flow_control_valve.flow_divider":
                # Stromteiler: Eingang 1 bzw. P auf die übrigen Anschlüsse, je gleicher Leitwert
                ein = "1" if "1" in kn else ("P" if "P" in kn else s.namen[0])
                for ziel in s.namen:
                    if ziel != ein:
                        buche(kn.get(ein), kn.get(ziel), s.fluss(druck(kn.get(ein)), druck(kn.get(ziel))))
            elif f == "shutoff_valve.quick_exhaust_valve":
                ein, aus = ("1", "2") if "1" in kn else (s.namen + [None, None])[:2]
                p1, p2 = druck(kn.get(ein)), druck(kn.get(aus))
                if p1 >= p2:
                    buche(kn.get(ein), kn.get(aus), s.fluss(p1, p2, frei=True))
                else:
                    buche(kn.get(aus), None, s.fluss(p2, P_ATM))
            elif f == "pressure_valve.pressure_relief_valve01":
                k = next((k for k in kn.values() if k is not None), None)
                if k is not None:
                    anteil = min(1.0, max(0.0, (druck(k) - P_ATM - s.p0) / s.dp_end))
                    buche(k, None, s.fluss(druck(k), P_ATM, anteil))
            elif f == "pressure_valve.pressure_regulator":
                ein, aus = ("1", "2") if "1" in kn else (s.namen + [None, None])[:2]
                if kn.get(ein) is None or kn.get(aus) is None:
                    continue
                p2 = druck(kn[aus]) - P_ATM
                oeffnen = min(1.0, max(0.0, (s.p0 - p2) / s.dp_end))
                buche(kn[ein], kn[aus], s.fluss(druck(kn[ein]), druck(kn[aus]), oeffnen))
                entlueften = min(1.0, max(0.0, (p2 - s.p0 - s.dp_end) / s.dp_end))
                buche(kn[aus], None, s.fluss(druck(kn[aus]), P_ATM, entlueften))

        for z in self.zylinder:
            ka, kb = self.kammer(z, "A"), self.kammer(z, "B")
            pa = self.p[ka] if ka is not None else P_ATM
            pb = self.p[kb] if kb is not None else P_ATM
            f = (pa - P_ATM) * z.a_k - (pb - P_ATM) * z.a_r - z.f_last - z.f_feder
            if abs(z.v) < 1e-4 and abs(f) <= z.f_haft:
                z.v = 0.0
            else:
                richtung = z.v if abs(z.v) >= 1e-4 else f
                f_netto = f - math.copysign(z.f_gleit, richtung) - z.d_visk * z.v
                z.v += f_netto / z.m * dt
                z.x += z.v * dt
                if z.x <= 0.0:
                    z.x, z.v = 0.0, 0.0
                elif z.x >= z.hub:
                    z.x, z.v = z.hub, 0.0
            if ka is not None:
                vol[ka] += V_TOT + z.a_k * z.x
                dvdt[ka] += z.a_k * z.v
            if kb is not None:
                vol[kb] += V_TOT + z.a_r * (z.hub - z.x)
                dvdt[kb] -= z.a_r * z.v
            lage = "hinten" if z.x <= 0.0005 else ("vorne" if z.x >= z.hub - 0.0005 else None)
            if lage != self.endlage[z.b.label]:
                self.endlage[z.b.label] = lage
                if lage:
                    self.meldungen.append((t, f"{z.b.label} Endlage {lage}"))
        for k in range(n):
            if k in self.fest:
                continue
            if k in self.hyd:
                self.p[k] = max(P_ATM - 0.9e5, self.p[k] + E_OEL / vol[k] * (mdot[k] - dvdt[k]) * dt)
            else:
                self.p[k] = max(1000.0, self.p[k] + (R * T * mdot[k] - self.p[k] * dvdt[k]) / vol[k] * dt)

    def lauf(self, dauer, ereignisse, dt=1e-5, raster=0.005):
        ereignisse = sorted(ereignisse)
        zeilen, t, naechst, i = [], 0.0, 0.0, 0
        while t <= dauer + 1e-12:
            while i < len(ereignisse) and ereignisse[i][0] <= t:
                _, label, seite, an = ereignisse[i]
                if seite is None:       # Taster
                    (self.elektrik.gedrueckt.add if an else self.elektrik.gedrueckt.discard)(label)
                    self.meldungen.append((t, f"{label} {'gedrückt' if an else 'losgelassen'}"))
                else:
                    v = self.nach_label.get(label)
                    if v is None:
                        raise SystemExit(f"kein Ventil mit Kennzeichen {label}")
                    v.betaetigt[seite] = an
                    self.meldungen.append((t, f"{label} {'links' if seite == 'left' else 'rechts'} "
                                              f"{'betätigt' if an else 'losgelassen'}"))
                i += 1
            if self.elektrik.spulen or self.elektrik.relais:
                self.spulen_auf_ventile(t)
            if t >= naechst - 1e-12:
                z = {"t": round(t, 4)}
                for zy in self.zylinder:
                    z[f"{zy.b.label} x [mm]"] = round(zy.x * 1000, 2)
                    z[f"{zy.b.label} v [m/s]"] = round(zy.v, 3)
                    for k in ("A", "B"):
                        kn = self.kammer(zy, k)
                        z[f"{zy.b.label} p{k} [bar]"] = round(bar(self.p[kn]) if kn is not None else 0.0, 3)
                for v in self.ventile:
                    z[f"{v.b.label} Stellung"] = v.stellung + 1
                for k in self.elektrik.kontakte:
                    if k["art"] == "taster":
                        z[f"{k['b'].label} gedrückt"] = int(k["b"].label in self.elektrik.gedrueckt)
                for r in self.elektrik.relais.values():
                    z[f"{r['b'].label} angezogen"] = int(r["an"])
                for s in self.elektrik.spulen:
                    z[f"Spule {s['b'].label}"] = int(s["an"])
                zeilen.append(z)
                naechst += raster
            self.schritt(t, dt)
            t += dt
        return zeilen


def ereignis_lesen(text):
    """„0.1 -QM1 links an“ (Ventil von Hand) oder „0.1 -SF1 an“ (Taster drücken/loslassen)."""
    m = re.fullmatch(r"\s*([\d.]+)\s+(\S+)\s+(?:(links|rechts|left|right)\s+)?(an|aus|on|off)\s*", text)
    if not m:
        raise SystemExit(f"Ereignis nicht verstanden: {text!r} (Form: '0.1 -QM1 links an' oder '0.1 -SF1 an')")
    seite = None if m.group(3) is None else ("left" if m.group(3) in ("links", "left") else "right")
    return float(m.group(1)), m.group(2), seite, m.group(4) in ("an", "on")


def diagramm(zeilen, sim, pfad):
    """Zustandsdiagramm wie in FluidSIM: Weg, Kammerdrücke und Ventilstellung über der Zeit (PNG)."""
    import pymupdf
    t = [z["t"] for z in zeilen]
    spuren = []
    for zy in sim.zylinder:
        spuren.append((f"{zy.b.label} Weg [mm]", [[z[f"{zy.b.label} x [mm]"] for z in zeilen]], 0, zy.hub * 1000,
                       [(0, 0.67, 0.65)], False))
        spuren.append((f"{zy.b.label} Druck [bar]", [[z[f"{zy.b.label} pA [bar]"] for z in zeilen],
                                                     [z[f"{zy.b.label} pB [bar]"] for z in zeilen]],
                       0, max(7, max(max(z[f"{zy.b.label} pA [bar]"], z[f"{zy.b.label} pB [bar]"]) for z in zeilen)),
                       [(0.98, 0.44, 0.08), (0.32, 0.36, 0.59)], False))
    for v in sim.ventile:
        spuren.append((f"{v.b.label} Stellung", [[z[f"{v.b.label} Stellung"] for z in zeilen]],
                       1, len(v.stellungen), [(0.29, 0.3, 0.33)], True))
    for schluessel in zeilen[0]:
        if schluessel.endswith((" gedrückt", " angezogen")) or schluessel.startswith("Spule "):
            spuren.append((schluessel, [[z[schluessel] for z in zeilen]], 0, 1, [(0.0, 0.45, 0.52)], True))
    breite, hoehe_spur, links, oben = 900, 110, 150, 20
    doc = pymupdf.open()
    seite = doc.new_page(width=breite, height=oben + len(spuren) * hoehe_spur + 40)
    t_max = t[-1] or 1
    for i, (titel, reihen, lo, hi, farben, stufe) in enumerate(spuren):
        y0 = oben + i * hoehe_spur
        r = pymupdf.Rect(links, y0 + 10, breite - 20, y0 + hoehe_spur - 15)
        seite.draw_rect(r, color=(0.7, 0.7, 0.7), width=0.5)
        seite.insert_text((10, y0 + hoehe_spur / 2), titel, fontsize=9)
        seite.insert_text((links - 30, r.y1), f"{lo:g}", fontsize=7)
        seite.insert_text((links - 30, r.y0 + 7), f"{hi:g}", fontsize=7)
        for j in range(1, 10):
            x = r.x0 + r.width * j / 10
            seite.draw_line((x, r.y0), (x, r.y1), color=(0.9, 0.9, 0.9), width=0.3)
        for reihe, farbe in zip(reihen, farben):
            pkte = []
            for k, (tt, w) in enumerate(zip(t, reihe)):
                x = r.x0 + r.width * tt / t_max
                y = r.y1 - r.height * (w - lo) / ((hi - lo) or 1)
                if stufe and pkte:
                    pkte.append(pymupdf.Point(x, pkte[-1].y))
                pkte.append(pymupdf.Point(x, y))
            seite.draw_polyline(pkte, color=farbe, width=1.2)
    y = oben + len(spuren) * hoehe_spur
    for j in range(0, 11):
        x = links + (breite - 20 - links) * j / 10
        seite.insert_text((x - 8, y + 5), f"{t_max * j / 10:.2f}", fontsize=7)
    seite.insert_text((breite / 2 - 20, y + 20), "Zeit [s]", fontsize=8)
    seite.get_pixmap(dpi=110).save(str(pfad))


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("projekt", type=Path)
    ap.add_argument("--ereignis", action="append", default=[])
    ap.add_argument("--dauer", type=float, default=2.0)
    ap.add_argument("--druck", type=float, default=6.0, help="Versorgungsdruck in bar Überdruck")
    ap.add_argument("--csv", type=Path)
    ap.add_argument("--diagramm", type=Path)
    a = ap.parse_args()
    sim = Simulation(a.projekt, a.druck)
    print(f"{len(sim.netz.index)} Anschlussknoten, {len(sim.fest)} Quellen, {len(sim.ventile)} Ventile, "
          f"{len(sim.zylinder)} Zylinder")
    zeilen = sim.lauf(a.dauer, [ereignis_lesen(e) for e in a.ereignis])
    for t, m in sim.meldungen:
        print(f"{t:7.3f} s  {m}")
    if a.csv:
        with a.csv.open("w", newline="", encoding="utf-8-sig") as f:
            w = csv.DictWriter(f, fieldnames=list(zeilen[0]), delimiter=";")
            w.writeheader()
            w.writerows(zeilen)
    if a.diagramm:
        diagramm(zeilen, sim, a.diagramm)


if __name__ == "__main__":
    sys.exit(main())
