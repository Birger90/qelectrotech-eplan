#!/usr/bin/env python3
"""Pneumatik- und Hydrauliksymbole nach ISO 1219-1, eigene Zeichnung des Forks.

Jedes Symbol wird hier aus den Grundelementen der Norm zusammengesetzt (Rastermodul, Ventilkästchen je
Schaltstellung, Durchflusspfeile, Sperren, Betätigungen, Zylinder, Kreise für Pumpen und Motoren); keine
Geometrie stammt aus einer fremden Bibliothek. Die Bauteile tragen die Daten, mit denen QElectroTech
Fluidpläne prüft: <fluid> (Art, Kennbuchstabe nach IEC 81346-2, Schaltstellungen, Betätigungen,
Zylinderdaten), an jedem Anschluss Medium, Portnummer und Abschluss. Magnetventile sind Master, die
Ventilspule ist ihr Slave.

    python iso1219_symbole.py --ziel bauteile/fluid

Die UUIDs leiten sich aus dem Dateinamen ab: jeder Lauf erzeugt dieselben Dateien.
"""
import argparse
import math
import os
import uuid as uuidlib
from xml.sax.saxutils import escape

M = 10          # Rastermodul in px: ein Ventilkästchen ist 4 M breit, 2 M hoch
NS = uuidlib.UUID("6f1a6a55-7c43-4b8e-9a3e-1219f1a1d000")
STIL = "line-style:normal;line-weight:normal;filling:none;color:black"
STIL_GEFUELLT = "line-style:normal;line-weight:thin;filling:black;color:black"
STIL_STRICH = "line-style:dashed;line-weight:normal;filling:none;color:black"


def uid(*teile):
    return "{" + str(uuidlib.uuid5(NS, "/".join(teile))) + "}"


def z(v):
    """Zahl ohne überflüssige Nachkommastellen"""
    v = round(float(v), 2)
    return str(int(v)) if v == int(v) else str(v)


class Symbol:
    """Sammelt die Grafik eines Bauteils; Koordinaten relativ zum Einfügepunkt (0, 0)."""

    def __init__(self):
        self.teile = []         # XML-Zeilen der Grafik
        self.anschluesse = []   # (name, x, y, ausrichtung, medium, abschluss)
        self.texte = []         # (text, x, y)
        self.punkte = [(0, 0)]

    def _p(self, *xy):
        self.punkte.extend(xy)

    def linie(self, x1, y1, x2, y2, stil=STIL):
        self._p((x1, y1), (x2, y2))
        self.teile.append(f'<line x1="{z(x1)}" y1="{z(y1)}" x2="{z(x2)}" y2="{z(y2)}" end1="none" end2="none" '
                          f'length1="1.5" length2="1.5" style="{stil}" antialias="false"/>')

    def kasten(self, x, y, b, h, stil=STIL):
        self._p((x, y), (x + b, y + h))
        self.teile.append(f'<rect x="{z(x)}" y="{z(y)}" width="{z(b)}" height="{z(h)}" rx="0" ry="0" '
                          f'style="{stil}" antialias="false"/>')

    def kreis(self, mx, my, r, stil=STIL):
        self._p((mx - r, my - r), (mx + r, my + r))
        self.teile.append(f'<ellipse x="{z(mx - r)}" y="{z(my - r)}" width="{z(2 * r)}" height="{z(2 * r)}" '
                          f'style="{stil}" antialias="true"/>')

    def bogen(self, mx, my, r, start, laenge):
        self._p((mx - r, my - r), (mx + r, my + r))
        self.teile.append(f'<arc x="{z(mx - r)}" y="{z(my - r)}" width="{z(2 * r)}" height="{z(2 * r)}" '
                          f'start="{z(start)}" angle="{z(laenge)}" style="{STIL}" antialias="true"/>')

    def zug(self, punkte, geschlossen=False, stil=STIL):
        self._p(*punkte)
        attr = " ".join(f'x{i + 1}="{z(x)}" y{i + 1}="{z(y)}"' for i, (x, y) in enumerate(punkte))
        self.teile.append(f'<polygon {attr} closed="{"true" if geschlossen else "false"}" style="{stil}" '
                          f'antialias="true"/>')

    def dreieck(self, spitze, richtung, laenge=4.0, breite=3.2, gefuellt=True):
        """Pfeil- oder Druckdreieck: Spitze und Richtung (Einheitsvektor)"""
        (sx, sy), (dx, dy) = spitze, richtung
        bx, by = sx - dx * laenge, sy - dy * laenge
        nx, ny = -dy * breite / 2, dx * breite / 2
        self.zug([(sx, sy), (bx + nx, by + ny), (bx - nx, by - ny)], True,
                 STIL_GEFUELLT if gefuellt else STIL)

    def pfeil(self, x1, y1, x2, y2):
        """Durchflussweg mit Pfeil am Ende"""
        lg = math.hypot(x2 - x1, y2 - y1)
        dx, dy = (x2 - x1) / lg, (y2 - y1) / lg
        self.linie(x1, y1, x2 - dx * 3.5, y2 - dy * 3.5)
        self.dreieck((x2, y2), (dx, dy))

    def text(self, t, x, y):
        self.texte.append((t, x, y))
        self._p((x, y))

    def anschluss(self, name, x, y, ausrichtung, medium="pneumatic", abschluss=""):
        self.anschluesse.append((name, x, y, ausrichtung, medium, abschluss))
        self._p((x, y))

    def xml(self, datei, namen, fluid, link_type="simple", kind_infos=""):
        xs = [p[0] for p in self.punkte]
        ys = [p[1] for p in self.punkte]
        x0 = int((min(xs) - 12) // 10 * 10)
        y0 = int((min(ys) - 18) // 10 * 10)
        x1 = int(-(-(max(xs) + 6) // 10) * 10)
        y1 = int(-(-(max(ys) + 6) // 10) * 10)
        zeilen = [f'<definition version="0.90" type="element" link_type="{link_type}" width="{x1 - x0}" '
                  f'height="{y1 - y0}" hotspot_x="{-x0}" hotspot_y="{-y0}">',
                  f'    <uuid uuid="{uid(datei)}"/>', "    <names>"]
        for sprache, text in namen.items():
            zeilen.append(f'        <name lang="{sprache}">{escape(text)}</name>')
        zeilen.append("    </names>")
        if kind_infos:
            zeilen.append(kind_infos)
        zeilen.append(fluid)
        zeilen.append("    <informations>Author: Fork qelectrotech-eplan, drawn after ISO 1219-1 "
                      "(tools/symbole/iso1219_symbole.py)\nLicense: see "
                      "http://qelectrotech.org/wiki/doc/elements_license</informations>")
        zeilen.append("    <description>")
        zeilen += ["        " + t for t in self.teile]
        zeilen.append(f'        <dynamic_text x="{x0 + 2}" y="{y0 + 1}" z="30" text_width="-1" Halignment="AlignLeft" '
                      f'Valignment="AlignTop" frame="false" rotation="0" keep_visual_rotation="false" '
                      f'text_from="ElementInfo" uuid="{uid(datei, "label")}" '
                      f'font="Liberation Sans,7,-1,5,0,0,0,0,0,0,Regular"><text></text><info_name>label</info_name>'
                      f'</dynamic_text>')
        for i, (t, x, y) in enumerate(self.texte):
            zeilen.append(f'        <dynamic_text x="{z(x)}" y="{z(y)}" z="31" text_width="-1" Halignment="AlignLeft" '
                          f'Valignment="AlignTop" frame="false" rotation="0" keep_visual_rotation="false" '
                          f'text_from="UserText" uuid="{uid(datei, "text", str(i))}" '
                          f'font="Liberation Sans,4,-1,5,0,0,0,0,0,0,Regular"><text>{escape(t)}</text></dynamic_text>')
        for name, x, y, ausr, medium, abschluss in self.anschluesse:
            ende = f' port_end="{abschluss}"' if abschluss else ""
            zeilen.append(f'        <terminal uuid="{uid(datei, "port", name)}" name="{escape(name)}" x="{z(x)}" '
                          f'y="{z(y)}" orientation="{ausr}" type="Generic" medium="{medium}"{ende}/>')
        zeilen += ["    </description>", "</definition>", ""]
        return "\n".join(zeilen)


# --- Wegeventile ---------------------------------------------------------------------------------------

B, H = 4 * M, 2 * M     # Kästchen einer Schaltstellung: 40 × 20, Oberkante y = -H, Unterkante y = 0

# Lage der Anschlüsse im Kästchen (x im Kästchen, unten/oben) je Bauart
LAGE = {
    "2/2": {"1": (20, "u"), "2": (20, "o")},
    "3/2": {"1": (10, "u"), "3": (30, "u"), "2": (10, "o")},
    "4/2": {"1": (10, "u"), "3": (30, "u"), "4": (10, "o"), "2": (30, "o")},
    "5/2": {"5": (10, "u"), "1": (20, "u"), "3": (30, "u"), "4": (10, "o"), "2": (30, "o")},
    "5/3": {"5": (10, "u"), "1": (20, "u"), "3": (30, "u"), "4": (10, "o"), "2": (30, "o")},
    "H4/2": {"P": (10, "u"), "T": (30, "u"), "A": (10, "o"), "B": (30, "o")},
    "H4/3": {"P": (10, "u"), "T": (30, "u"), "A": (10, "o"), "B": (30, "o")},
    "H2/2": {"P": (20, "u"), "A": (20, "o")},
    "H3/2": {"P": (10, "u"), "T": (30, "u"), "A": (10, "o")},
}


def zeichne_stellung(s, x0, lage, wege, gesperrt):
    """Ein Kästchen: Pfeile für die Wege, T-Sperren für gesperrte Anschlüsse"""
    s.kasten(x0, -H, B, H)

    def punkt(port):
        x, seite = lage[port]
        return x0 + x, (0 if seite == "u" else -H)

    for von, nach in wege:
        (ax, ay), (bx, by) = punkt(von), punkt(nach)
        if ay == by:    # beide unten oder beide oben: Verbindung ohne Pfeil
            yy = ay - 7 if ay == 0 else ay + 7
            s.zug([(ax, ay), (ax, yy), (bx, yy), (bx, by)])
        else:
            s.pfeil(ax, ay, bx, by)
    for port in gesperrt:
        x, y = punkt(port)
        innen = y - 6 if y == 0 else y + 6
        s.linie(x, y, x, innen)
        s.linie(x - 3, innen, x + 3, innen)


def feder(s, x, my, r):
    """Federrückstellung: Zickzack von der Ventilkante x nach außen (r = ±1)"""
    pkte = [(x, my)]
    for i in range(1, 6):
        pkte.append((x + r * 2.4 * i, my + (-4 if i % 2 else 4)))
    pkte.append((x + r * 14, my))
    s.zug(pkte)


def betaetigung(s, art, x, links, mit_feder=False):
    """Betätigung an der Außenkante x des Ventils; mit Feder sitzt die Betätigung im oberen Drittel"""
    r = -1 if links else 1
    my = -H / 2
    if mit_feder:
        feder(s, x, -5 if art else my, r)
        my = -H + 4
    if not art:
        return
    lo, hi = (x - 10, x) if links else (x, x + 10)
    if art == "solenoid":
        h = 6 if mit_feder else 12
        s.kasten(lo, my - h / 2, 10, h)
        s.linie(lo, my + h / 2, hi, my - h / 2)
    elif art == "pneumatic":
        s.linie(x + r * 12, my, x + r * 4.5, my, STIL_STRICH)
        s.dreieck((x, my), (-r, 0), 4.5, 4, gefuellt=False)
    elif art == "button":
        s.linie(x, my, x + r * 6, my)
        s.bogen(x + r * 8, my, 3, 90 if links else 270, 180)
    elif art == "lever":
        s.linie(x, my, x + r * 4, my)
        s.linie(x + r * 4, my, x + r * 10, my - 7)
        s.kreis(x + r * 10.8, my - 8, 1.5, STIL_GEFUELLT)
    elif art == "pedal":
        s.linie(x, my, x + r * 5, my)
        s.linie(x + r * 5, my + 4, x + r * 11, my - 4)
    elif art == "plunger":
        s.linie(x, my, x + r * 8, my)
        s.linie(x + r * 8, my - 3, x + r * 8, my + 3)
    elif art in ("roller", "idle_roller"):
        s.linie(x, my, x + r * 6, my)
        s.kreis(x + r * 9, my, 3)
        if art == "idle_roller":
            s.linie(x + r * 6, my - 4, x + r * 12, my + 4)
    elif art == "detent":
        s.linie(x, my, x + r * 6, my)
        s.zug([(x + r * 3, my - 5), (x + r * 6, my - 2), (x + r * 9, my - 5)])
    elif art == "manual":
        s.linie(x, my, x + r * 6, my)
        s.linie(x + r * 6, my - 4, x + r * 6, my + 4)


BET_DE = {"solenoid": "Magnet", "pneumatic": "pneumatisch", "button": "Taster", "lever": "Hebel",
          "pedal": "Pedal", "plunger": "Stößel", "roller": "Rolle", "idle_roller": "Leerrücklaufrolle",
          "detent": "Raste", "manual": "Hand", "": ""}
BET_EN = {"solenoid": "solenoid", "pneumatic": "pilot", "button": "push button", "lever": "lever",
          "pedal": "pedal", "plunger": "plunger", "roller": "roller", "idle_roller": "idle return roller",
          "detent": "detent", "manual": "manual", "": ""}


def ventil(art, stellungen, links, rechts, medium="pneumatic", feder_links=False, feder_rechts=False,
           praefix="QM", abschluss=None):
    """Wegeventil. stellungen: (wege, gesperrt) von links nach rechts; die Anschlüsse sitzen am Kästchen
    der Ruhestellung (rechts bei zwei, Mitte bei drei Stellungen), das bei x = 0 … B liegt."""
    lage = LAGE[art]
    n = len(stellungen)
    grund = n - 1 if n == 2 else n // 2
    s = Symbol()
    x_start = -grund * B
    for i, (wege, gesperrt) in enumerate(stellungen):
        zeichne_stellung(s, x_start + i * B, lage, wege, gesperrt)
    betaetigung(s, links, x_start, True, feder_links)
    betaetigung(s, rechts, x_start + n * B, False, feder_rechts)
    abschluss = abschluss or {}
    for port, (x, seite) in lage.items():
        if seite == "u":
            s.linie(x, 0, x, M)
            s.anschluss(port, x, M, "s", medium, abschluss.get(port, ""))
            s.text(port, x + 1.5, 1.5)
        else:
            s.linie(x, -H, x, -H - M)
            s.anschluss(port, x, -H - M, "n", medium, abschluss.get(port, ""))
            s.text(port, x + 1.5, -H - 8)
    pos = []
    for wege, gesperrt in stellungen:
        pfade = "".join(f'<path from="{a}" to="{b}"/>' for a, b in wege)
        pfade += "".join(f'<path from="{p}"/>' for p in gesperrt)
        pos.append(f"            <position>{pfade}</position>")

    def act(seite, typ, mit_feder):
        return f'        <actuation side="{seite}" types="{typ}"{" spring=\"true\"" if mit_feder else ""}/>'
    fluid = (f'    <fluid kind="valve" prefix="{praefix}">\n        <positions initial="{grund + 1}">\n'
             + "\n".join(pos) + "\n        </positions>\n"
             + act("left", links, feder_links) + "\n" + act("right", rechts, feder_rechts) + "\n    </fluid>")
    magnete = (links == "solenoid") + (rechts == "solenoid")
    if not magnete:
        return s, fluid, "simple", ""
    kind = ("    <kindInformations>\n        <kindInformation name=\"type\">coil</kindInformation>\n"
            f"        <kindInformation name=\"max_slaves\">{magnete}</kindInformation>\n    </kindInformations>")
    return s, fluid, "master", kind


def bet_text(links, rechts, fl, fr, sprache):
    t, fe = (BET_DE, "Feder") if sprache == "de" else (BET_EN, "spring")
    l = t[links] + (("/" if links else "") + fe if fl else "")
    r = t[rechts] + (("/" if rechts else "") + fe if fr else "")
    return f"{l} – {r}" if l and r else (l or r)


PNEU_2 = {   # (Bauart, [Stellung links, Stellung rechts = Ruhestellung])
    "2/2 NC": ("2/2", [([("1", "2")], []), ([], ["1", "2"])]),
    "2/2 NO": ("2/2", [([], ["1", "2"]), ([("1", "2")], [])]),
    "3/2 NC": ("3/2", [([("1", "2")], ["3"]), ([("2", "3")], ["1"])]),
    "3/2 NO": ("3/2", [([("2", "3")], ["1"]), ([("1", "2")], ["3"])]),
    "4/2": ("4/2", [([("1", "4"), ("2", "3")], []), ([("1", "2"), ("4", "3")], [])]),
    "5/2": ("5/2", [([("1", "4"), ("2", "3")], ["5"]), ([("1", "2"), ("4", "5")], ["3"])]),
}
PNEU_3 = {   # Mittelstellung der 5/3-Wegeventile
    "Mitte gesperrt": (([], ["1", "2", "3", "4", "5"]), "closed centre", "gesperrt"),
    "Mitte entlüftet": (([("4", "5"), ("2", "3")], ["1"]), "exhausted centre", "entlueftet"),
    "Mitte belüftet": (([("1", "4"), ("1", "2")], ["3", "5"]), "pressurised centre", "belueftet"),
}
PNEU_BET = [  # (links, rechts, Feder links, Feder rechts)
    ("solenoid", "", False, True), ("solenoid", "solenoid", False, False), ("pneumatic", "", False, True),
    ("pneumatic", "pneumatic", False, False), ("button", "", False, True), ("lever", "detent", False, False),
    ("pedal", "", False, True), ("roller", "", False, True), ("idle_roller", "", False, True),
    ("plunger", "", False, True), ("manual", "", False, True),
]


def pneumatik_ventile():
    for kurz, (art, stellungen) in PNEU_2.items():
        for links, rechts, fl, fr in PNEU_BET:
            if art == "2/2" and links == "lever":
                continue
            if art in ("4/2", "5/2"):
                praefix = "QM"
            elif links in ("roller", "idle_roller", "plunger"):
                praefix = "BG"
            elif links in ("button", "lever", "pedal", "manual"):
                praefix = "SJ"
            else:
                praefix = "QM"
            de = (kurz.replace(" NC", "-Wegeventil, Ruhestellung gesperrt").replace(" NO", "-Wegeventil, Ruhestellung offen")
                  if " " in kurz else kurz + "-Wegeventil") + ", " + bet_text(links, rechts, fl, fr, "de")
            en = (kurz.replace(" NC", " way valve, normally closed").replace(" NO", " way valve, normally open")
                  if " " in kurz else kurz + " way valve") + ", " + bet_text(links, rechts, fl, fr, "en")
            s, fluid, link, kind = ventil(art, stellungen, links, rechts, feder_links=fl, feder_rechts=fr,
                                          praefix=praefix, abschluss={"3": "silencer", "5": "silencer"})
            datei = f"wv_{kurz.replace('/', '').replace(' ', '_').lower()}_{links}_{rechts or 'feder'}"
            yield "Wegeventile", datei, s, {"de": de, "en": en}, fluid, link, kind
    aussen_l = ([("1", "4"), ("2", "3")], ["5"])
    aussen_r = ([("1", "2"), ("4", "5")], ["3"])
    for kurz, (mitte, en_mitte, slug) in PNEU_3.items():
        for typ in ("solenoid", "pneumatic", "lever"):
            federn = typ != "lever"
            s, fluid, link, kind = ventil("5/3", [aussen_l, mitte, aussen_r], typ, typ, feder_links=federn,
                                          feder_rechts=federn, abschluss={"3": "silencer", "5": "silencer"})
            de = f"5/3-Wegeventil, {kurz}, {BET_DE[typ]}, " + ("federzentriert" if federn else "Raste")
            en = f"5/3 way valve, {en_mitte}, {BET_EN[typ]}, " + ("spring centred" if federn else "detent")
            yield "Wegeventile", f"wv_53_{slug}_{typ}", s, {"de": de, "en": en}, fluid, link, kind


HYD_3 = {   # Mittelstellung der 4/3-Wegeventile
    "gesperrt": ([], ["P", "T", "A", "B"], "closed centre", "gesperrt"),
    "Umlauf": ([("P", "T")], ["A", "B"], "tandem centre", "umlauf"),
    "Schwimmstellung": ([("A", "T"), ("B", "T")], ["P"], "float centre", "schwimm"),
    "offen": ([("P", "T"), ("A", "B")], [], "open centre", "offen"),
}


def hydraulik_ventile():
    aussen_l = ([("P", "B"), ("A", "T")], [])
    aussen_r = ([("P", "A"), ("B", "T")], [])
    for links, rechts, fl, fr in [("solenoid", "", False, True), ("solenoid", "solenoid", False, False),
                                  ("lever", "", False, True)]:
        s, fluid, link, kind = ventil("H4/2", [aussen_l, aussen_r], links, rechts, medium="hydraulic",
                                      feder_links=fl, feder_rechts=fr)
        de = f"4/2-Wegeventil, {bet_text(links, rechts, fl, fr, 'de')}"
        en = f"4/2 way valve, {bet_text(links, rechts, fl, fr, 'en')}"
        yield "Wegeventile", f"hwv_42_{links}_{rechts or 'feder'}", s, {"de": de, "en": en}, fluid, link, kind
    for kurz, (wege, gesperrt, en_mitte, slug) in HYD_3.items():
        for typ in ("solenoid", "lever"):
            s, fluid, link, kind = ventil("H4/3", [aussen_l, (wege, gesperrt), aussen_r], typ, typ,
                                          medium="hydraulic", feder_links=True, feder_rechts=True)
            de = f"4/3-Wegeventil, Mittelstellung {kurz}, {BET_DE[typ]}, federzentriert"
            en = f"4/3 way valve, {en_mitte}, {BET_EN[typ]}, spring centred"
            yield "Wegeventile", f"hwv_43_{slug}_{typ}", s, {"de": de, "en": en}, fluid, link, kind
    for art, st, de0, en0 in [("H2/2", [([("P", "A")], []), ([], ["P", "A"])], "2/2-Wegeventil", "2/2 way valve"),
                              ("H3/2", [([("P", "A")], ["T"]), ([("A", "T")], ["P"])], "3/2-Wegeventil",
                               "3/2 way valve")]:
        for typ in ("solenoid", "lever"):
            s, fluid, link, kind = ventil(art, st, typ, "", medium="hydraulic", feder_rechts=True,
                                          praefix="QM" if typ == "solenoid" else "SJ")
            yield "Wegeventile", f"hwv_{art[1:].replace('/', '')}_{typ}", s, \
                {"de": f"{de0}, {BET_DE[typ]}/Feder", "en": f"{en0}, {BET_EN[typ]}/spring"}, fluid, link, kind


# --- Zylinder, Motoren, Pumpen ---------------------------------------------------------------------------

def zylinder(medium, einfach=False, daempfung=False, durchgehend=False, magnet=False):
    s = Symbol()
    L, Hz, kolben = 60, 20, 20
    s.kasten(0, -Hz, L, Hz)
    s.linie(kolben, -Hz, kolben, 0)
    s.linie(kolben, -Hz / 2, L + 20, -Hz / 2)
    if durchgehend:
        s.linie(kolben, -Hz / 2, -20, -Hz / 2)
    if einfach:
        pkte = [(kolben + 2, -Hz / 2)]
        for i in range(1, 8):
            pkte.append((kolben + 2 + i * 4.5, -Hz / 2 + (-6 if i % 2 else 6)))
        pkte.append((L - 2, -Hz / 2))
        s.zug(pkte)
        s.linie(50, 0, 50, 3)       # Entlüftung der Federseite
    if daempfung:
        s.kasten(kolben - 6, -Hz / 2 - 3, 4, 6)
        s.kasten(kolben + 2, -Hz / 2 - 3, 4, 6)
        s.linie(kolben - 10, -3, kolben - 3, -10)
        s.dreieck((kolben - 3, -10), (0.707, -0.707), 2.5, 2.2)
    if magnet:
        s.kasten(kolben - 1.5, -Hz + 2, 3, Hz - 4, STIL_GEFUELLT)
    s.linie(10, 0, 10, M)
    s.anschluss("1", 10, M, "s", medium)
    s.text("1", 11.5, 1.5)
    if not einfach:
        s.linie(50, 0, 50, M)
        s.anschluss("2", 50, M, "s", medium)
        s.text("2", 51.5, 1.5)
    attr = f'action="{"single" if einfach else "double"}"'
    if einfach:
        attr += ' spring="rod"'
    if daempfung:
        attr += ' cushioning="adjustable"'
    if durchgehend:
        attr += ' through_rod="true"'
    if magnet:
        attr += ' magnet_piston="true"'
    return s, f'    <fluid kind="cylinder" prefix="MM">\n        <cylinder {attr}/>\n    </fluid>'


def pumpe_motor(medium, motor=False, verstellbar=False, zwei_richtungen=False):
    """Kreis mit Dreieck(en): Spitze nach außen = Pumpe/Verdichter, nach innen = Motor;
    gefüllt für Hydraulik, offen für Pneumatik"""
    s = Symbol()
    r = 12
    s.kreis(0, -r, r)
    gefuellt = medium == "hydraulic"
    for dy in [-1] + ([1] if zwei_richtungen else []):
        if motor:
            s.dreieck((0, -r + dy * (r - 6)), (0, -dy), 5, 6, gefuellt)
        else:
            s.dreieck((0, -r + dy * (r - 0.5)), (0, dy), 5, 6, gefuellt)
    if verstellbar:
        s.linie(-r - 3, 3, r + 3, -2 * r - 3)
        s.dreieck((r + 3, -2 * r - 3), (0.62, -0.78), 4, 3)
    s.linie(r, -r, r + 8, -r)               # Welle
    s.linie(r + 8, -r - 2, r + 8, -r + 2)
    s.linie(0, 0, 0, M)
    s.linie(0, -2 * r, 0, -2 * r - 8)
    if medium == "pneumatic":
        unten, oben = "1", "2"
    else:
        unten, oben = ("A", "B") if motor else ("S", "P")
    s.anschluss(unten, 0, M, "s", medium)
    s.anschluss(oben, 0, -2 * r - 8, "n", medium)
    return s


def elektromotor():
    s = Symbol()
    s.kreis(0, -12, 12)
    s.text("M", -3, -17)
    s.linie(12, -12, 20, -12)
    for x, n in ((-6, "1"), (6, "2")):
        s.linie(x, -2, x, M)
        s.anschluss(n, x, M, "s", "electric")
    return s


# --- Sperr-, Strom- und Druckventile, Versorgung, Aufbereitung --------------------------------------------

def senkrecht(s, medium, unten, oben, halb=20):
    s.linie(0, halb, 0, halb + M)
    s.linie(0, -halb, 0, -halb - M)
    s.anschluss(unten, 0, halb + M, "s", medium)
    s.anschluss(oben, 0, -halb - M, "n", medium)


def ports(medium):
    return ("1", "2") if medium == "pneumatic" else ("A", "B")


def rueckschlag(medium, mit_feder=False):
    s = Symbol()
    s.linie(0, 20, 0, 4)
    s.kreis(0, 0, 4)
    s.zug([(-7, -6), (0, 4), (7, -6)])
    s.linie(0, -4, 0, -20)
    if mit_feder:
        s.zug([(5, -4), (8, -6), (2, -8), (8, -10), (2, -12), (5, -14)])
    senkrecht(s, medium, *ports(medium))
    return s


def drossel(medium, verstellbar=False, mit_rueckschlag=False):
    s = Symbol()
    s.linie(0, 20, 0, -20)
    s.bogen(-14, 0, 12, -40, 80)
    s.bogen(14, 0, 12, 140, 80)
    if verstellbar:
        s.linie(-10, 10, 10, -10)
        s.dreieck((10, -10), (0.707, -0.707), 4, 3)
    if mit_rueckschlag:
        s.linie(0, 14, 16, 14)
        s.linie(0, -14, 16, -14)
        s.linie(16, 14, 16, 5)
        s.kreis(16, 2, 3)
        s.zug([(11, -4), (16, 5), (21, -4)])
        s.linie(16, -1, 16, -14)
    senkrecht(s, medium, *ports(medium))
    return s


def wechselventil(und=False):
    s = Symbol()
    s.kasten(-20, -8, 40, 16)
    if und:
        s.kasten(-12, -5, 6, 10)
        s.kasten(6, -5, 6, 10)
    else:
        s.kreis(0, 0, 4)
        s.zug([(-12, -8), (-6, 0), (-12, 8)])
        s.zug([(12, -8), (6, 0), (12, 8)])
    s.linie(-20, 0, -30, 0)
    s.linie(20, 0, 30, 0)
    s.linie(0, -8, 0, -20)
    s.anschluss("1.1", -30, 0, "w")
    s.anschluss("1.2", 30, 0, "e")
    s.anschluss("2", 0, -20, "n")
    s.text("1", -28, 1.5)
    s.text("1", 25, 1.5)
    s.text("2", 1.5, -18)
    return s


def schnellentlueft():
    s = Symbol()
    s.kasten(-20, -8, 40, 16)
    s.kreis(-4, 0, 4)
    s.zug([(-12, -8), (-6, 0), (-12, 8)])
    s.linie(4, 0, 12, 0)
    s.dreieck((19, 0), (1, 0), 4, 3, gefuellt=False)
    s.linie(-20, 0, -30, 0)
    s.linie(0, -8, 0, -20)
    s.linie(14, 8, 14, 20)
    s.anschluss("1", -30, 0, "w")
    s.anschluss("2", 0, -20, "n")
    s.anschluss("3", 14, 20, "s", "pneumatic", "silencer")
    return s


def druckventil(medium, regler=False, folge=False):
    """Druckbegrenzungs-, Druckregel- oder Zuschaltventil: Kästchen mit Pfeil, einstellbare Feder,
    gestrichelte Steuerleitung vom Eingang (Begrenzung, Zuschaltung) bzw. vom Ausgang (Regler)"""
    s = Symbol()
    s.kasten(-10, -20, 20, 40)
    s.pfeil(-3 if regler else 3, 20, -3 if regler else 3, -20)
    s.zug([(10, 0), (13, -3), (16, 3), (19, -3), (22, 3), (24, 0)])
    s.linie(14, 8, 24, -8)
    s.dreieck((24, -8), (0.53, -0.85), 3, 2.5)
    y_steuer = -26 if regler else 26
    s.zug([(0, y_steuer), (-18, y_steuer), (-18, 0), (-14.5, 0)], stil=STIL_STRICH)
    s.dreieck((-10, 0), (1, 0), 4.5, 4, gefuellt=False)
    if medium == "pneumatic":
        unten, oben = "1", "2"
    else:
        unten, oben = "P", ("A" if regler or folge else "T")
    senkrecht(s, medium, unten, oben)
    return s


def quelle(medium):
    s = Symbol()
    s.kreis(0, 0, 8)
    s.dreieck((0, -8), (0, -1), 5, 6, gefuellt=medium == "hydraulic")
    s.linie(0, -8, 0, -20)
    s.anschluss("1" if medium == "pneumatic" else "P", 0, -20, "n", medium)
    return s


def speicher(medium):
    s = Symbol()
    if medium == "pneumatic":
        s.kasten(-12, -16, 24, 32)
        s.bogen(0, -16, 12, 0, 180)
        s.linie(0, 16, 0, 20)
        s.anschluss("1", 0, 20, "s", medium)
    else:
        s.bogen(0, -6, 10, 0, 180)
        s.linie(-10, -6, -10, 6)
        s.linie(10, -6, 10, 6)
        s.bogen(0, 6, 10, 180, 180)
        s.linie(-10, 0, 10, 0)
        s.linie(0, 16, 0, 20)
        s.anschluss("P", 0, 20, "s", medium)
    return s


def tank():
    s = Symbol()
    s.zug([(-14, -6), (-14, 6), (14, 6), (14, -6)])
    s.linie(0, 2, 0, -20)
    s.anschluss("T", 0, -20, "n", "hydraulic")
    return s


def raute(s, mx=0, my=0, r=14):
    s.zug([(mx, my - r), (mx + r, my), (mx, my + r), (mx - r, my), (mx, my - r)], True)


def aufbereitung(medium, art):
    s = Symbol()
    raute(s)
    if art in ("filter", "abscheider"):
        s.linie(-14, 0, 14, 0, STIL_STRICH)
        if art == "abscheider":
            s.linie(-6, 8, 6, 8)
            s.linie(0, 14, 0, 18)
            s.dreieck((0, 22), (0, 1), 4, 4)
    elif art == "oeler":
        s.linie(-6, 4, 6, 4)
        s.linie(0, 4, 0, -4)
        s.dreieck((0, -8), (0, -1), 4, 3)
    elif art == "kuehler":
        s.linie(-14, 0, 14, 0)
        s.dreieck((-6, -4), (0, -1), 4, 4)
        s.dreieck((6, -4), (0, -1), 4, 4)
    elif art == "trockner":
        s.linie(-14, 0, 14, 0)
        s.kasten(-4, 4, 8, 4)
    s.linie(-14, 0, -24, 0)
    s.linie(14, 0, 24, 0)
    ein, aus = ports(medium)
    s.anschluss(ein, -24, 0, "w", medium)
    s.anschluss(aus, 24, 0, "e", medium)
    return s


def wartungseinheit():
    s = Symbol()
    s.kasten(-30, -14, 60, 28, STIL_STRICH)
    raute(s, -14, 0, 9)
    s.linie(-23, 0, -5, 0, STIL_STRICH)
    s.kasten(4, -8, 10, 16)
    s.pfeil(9, 8, 9, -8)
    s.kreis(22, -3, 4)
    s.linie(-30, 0, -40, 0)
    s.linie(30, 0, 40, 0)
    s.anschluss("1", -40, 0, "w")
    s.anschluss("2", 40, 0, "e")
    return s


def manometer(medium):
    s = Symbol()
    s.kreis(0, -10, 10)
    s.linie(-6, -4, 5, -15)
    s.dreieck((6, -16), (0.707, -0.707), 3.5, 3)
    s.linie(0, 0, 0, M)
    s.anschluss("1" if medium == "pneumatic" else "P", 0, M, "s", medium)
    return s


def druckschalter():
    s = Symbol()
    s.kasten(-12, -24, 24, 18)
    s.zug([(0, 0), (0, -2), (-18, -2), (-18, -10), (-16.5, -10)], stil=STIL_STRICH)
    s.dreieck((-12, -10), (1, 0), 4.5, 3.5, gefuellt=False)
    s.linie(-4, -10, 4, -20)
    s.linie(0, 0, 0, M)
    s.anschluss("1", 0, M, "s", "pneumatic")
    for x, n in ((-6, "13"), (6, "14")):
        s.linie(x, -24, x, -34)
        s.anschluss(n, x, -34, "n", "electric")
    return s


def schalldaempfer():
    s = Symbol()
    s.kasten(-6, -4, 12, 8)
    for x in (-3, 0, 3):
        s.linie(x, -4, x, 4)
    s.linie(0, -4, 0, -14)
    s.anschluss("1", 0, -14, "n")
    return s


def ventilspule():
    s = Symbol()
    s.kasten(-10, -5, 20, 10)
    s.linie(-10, 5, 10, -5)
    s.linie(0, -5, 0, -15)
    s.linie(0, 5, 0, 15)
    s.anschluss("A1", 0, -15, "n", "electric")
    s.anschluss("A2", 0, 15, "s", "electric")
    return s


# --- Zusammenstellen ------------------------------------------------------------------------------------

def einfach(kind, praefix):
    return f'    <fluid kind="{kind}" prefix="{praefix}"/>' if praefix else f'    <fluid kind="{kind}"/>'


def alle():
    """(Bereich, Ordner, Datei, Symbol, Namen, fluid, link_type, kindInformations)"""
    for ordner, datei, s, namen, fluid, link, kind in pneumatik_ventile():
        yield "pneumatik", ordner, datei, s, namen, fluid, link, kind
    for ordner, datei, s, namen, fluid, link, kind in hydraulik_ventile():
        yield "hydraulik", ordner, datei, s, namen, fluid, link, kind
    for medium, bereich in (("pneumatic", "pneumatik"), ("hydraulic", "hydraulik")):
        zyl = [("zyl_einfach_feder", dict(einfach=True), "Zylinder, einfachwirkend, Federrückstellung",
                "Single acting cylinder, spring return"),
               ("zyl_doppelt", {}, "Zylinder, doppeltwirkend", "Double acting cylinder"),
               ("zyl_doppelt_daempfung", dict(daempfung=True),
                "Zylinder, doppeltwirkend, einstellbare Endlagendämpfung", "Double acting cylinder, adjustable cushioning"),
               ("zyl_doppelt_durchgehend", dict(durchgehend=True),
                "Zylinder, doppeltwirkend, durchgehende Kolbenstange", "Double acting cylinder, through rod")]
        if medium == "pneumatic":
            zyl.append(("zyl_doppelt_magnet", dict(magnet=True), "Zylinder, doppeltwirkend, Magnetkolben",
                        "Double acting cylinder, magnetic piston"))
        for datei, kw, de, en in zyl:
            s, fluid = zylinder(medium, **kw)
            yield bereich, "Antriebe", datei, s, {"de": de, "en": en}, fluid, "simple", ""
        maschinen = [("motor_eine_richtung", dict(motor=True), "Motor, eine Drehrichtung", "Motor, one direction"),
                     ("motor_zwei_richtungen", dict(motor=True, zwei_richtungen=True), "Motor, zwei Drehrichtungen",
                      "Motor, two directions")]
        if medium == "hydraulic":
            maschinen += [("motor_verstellbar", dict(motor=True, verstellbar=True), "Motor, verstellbar",
                           "Variable displacement motor"),
                          ("pumpe_konstant", {}, "Pumpe, konstant", "Fixed displacement pump"),
                          ("pumpe_verstellbar", dict(verstellbar=True), "Pumpe, verstellbar",
                           "Variable displacement pump")]
        else:
            maschinen.append(("verdichter", {}, "Verdichter", "Compressor"))
        for datei, kw, de, en in maschinen:
            s = pumpe_motor(medium, **kw)
            motor = kw.get("motor", False)
            yield bereich, "Antriebe" if motor else "Versorgung", datei, s, {"de": de, "en": en}, \
                einfach("other" if motor else "source", "MM" if motor else "GP"), "simple", ""
        teile = [("rueckschlag", rueckschlag(medium), "Rückschlagventil", "Check valve", "flow_control", "RM",
                  "Sperr- und Stromventile"),
                 ("rueckschlag_feder", rueckschlag(medium, mit_feder=True), "Rückschlagventil, federbelastet",
                  "Check valve, spring loaded", "flow_control", "RM", "Sperr- und Stromventile"),
                 ("drossel", drossel(medium), "Drosselventil, konstant", "Fixed throttle", "flow_control", "RZ",
                  "Sperr- und Stromventile"),
                 ("drossel_einstellbar", drossel(medium, verstellbar=True), "Drosselventil, einstellbar",
                  "Adjustable throttle", "flow_control", "RZ", "Sperr- und Stromventile"),
                 ("drosselrueckschlag", drossel(medium, verstellbar=True, mit_rueckschlag=True),
                  "Drosselrückschlagventil", "One-way flow control valve", "flow_control", "RZ",
                  "Sperr- und Stromventile"),
                 ("druckbegrenzung", druckventil(medium), "Druckbegrenzungsventil", "Pressure relief valve",
                  "flow_control", "QN", "Druckventile"),
                 ("druckregel", druckventil(medium, regler=True), "Druckregelventil", "Pressure regulator",
                  "treatment" if medium == "pneumatic" else "flow_control", "QN", "Druckventile"),
                 ("zuschalt", druckventil(medium, folge=True), "Zuschaltventil (Folgeventil)", "Sequence valve",
                  "flow_control", "QN", "Druckventile"),
                 ("manometer", manometer(medium), "Manometer", "Pressure gauge", "sensor", "PG", "Messgeräte"),
                 ("quelle", quelle(medium), "Druckluftquelle" if medium == "pneumatic" else "Hydraulikquelle",
                  "Compressed air supply" if medium == "pneumatic" else "Hydraulic power supply", "source", "GP",
                  "Versorgung"),
                 ("speicher", speicher(medium), "Druckluftbehälter" if medium == "pneumatic" else "Hydrospeicher",
                  "Air reservoir" if medium == "pneumatic" else "Hydraulic accumulator", "source", "CM", "Versorgung"),
                 ("filter", aufbereitung(medium, "filter"), "Filter", "Filter", "treatment", "HQ", "Aufbereitung")]
        for datei, s, de, en, kind, praefix, ordner in teile:
            yield bereich, ordner, datei, s, {"de": de, "en": en}, einfach(kind, praefix), "simple", ""
    pneu = [("wechselventil", wechselventil(), "Wechselventil (ODER)", "Shuttle valve (OR)", "logic_valve", "KH",
             "Sperr- und Stromventile"),
            ("zweidruckventil", wechselventil(und=True), "Zweidruckventil (UND)", "Dual pressure valve (AND)",
             "logic_valve", "KH", "Sperr- und Stromventile"),
            ("schnellentlueftung", schnellentlueft(), "Schnellentlüftungsventil", "Quick exhaust valve",
             "logic_valve", "KH", "Sperr- und Stromventile"),
            ("abscheider", aufbereitung("pneumatic", "abscheider"), "Filter mit Wasserabscheider",
             "Filter with water separator", "treatment", "HQ", "Aufbereitung"),
            ("oeler", aufbereitung("pneumatic", "oeler"), "Öler", "Lubricator", "treatment", "AZ", "Aufbereitung"),
            ("trockner", aufbereitung("pneumatic", "trockner"), "Lufttrockner", "Air dryer", "treatment", "HQ",
             "Aufbereitung"),
            ("wartungseinheit", wartungseinheit(), "Wartungseinheit (Filter, Regler, Öler)",
             "Air service unit (filter, regulator, lubricator)", "treatment", "AZ", "Aufbereitung"),
            ("schalldaempfer", schalldaempfer(), "Schalldämpfer", "Silencer", "other", "", "Aufbereitung"),
            ("druckschalter", druckschalter(), "Druckschalter, pneumatisch-elektrisch",
             "Pressure switch, pneumatic-electric", "sensor", "BP", "Messgeräte")]
    for datei, s, de, en, kind, praefix, ordner in pneu:
        yield "pneumatik", ordner, datei, s, {"de": de, "en": en}, einfach(kind, praefix), "simple", ""
    for datei, s, de, en, kind, praefix, ordner in [
            ("tank", tank(), "Tank", "Tank", "source", "CM", "Versorgung"),
            ("kuehler", aufbereitung("hydraulic", "kuehler"), "Kühler", "Cooler", "treatment", "EP", "Aufbereitung")]:
        yield "hydraulik", ordner, datei, s, {"de": de, "en": en}, einfach(kind, praefix), "simple", ""
    yield "hydraulik", "Antriebe", "elektromotor", elektromotor(), {"de": "Elektromotor", "en": "Electric motor"}, \
        einfach("other", "MA"), "simple", ""
    kind = ("    <kindInformations>\n        <kindInformation name=\"type\">simple</kindInformation>\n"
            "        <kindInformation name=\"state\">Other</kindInformation>\n"
            "        <kindInformation name=\"number\">1</kindInformation>\n    </kindInformations>")
    for bereich in ("pneumatik", "hydraulik"):
        yield bereich, "Wegeventile", "ventilspule", ventilspule(), {"de": "Ventilspule", "en": "Valve solenoid coil"}, \
            '    <fluid kind="solenoid"/>', "slave", kind


ORDNER_EN = {"Wegeventile": "Directional valves", "Antriebe": "Actuators", "Versorgung": "Supply",
             "Sperr- und Stromventile": "Non-return and flow valves", "Druckventile": "Pressure valves",
             "Aufbereitung": "Air and fluid treatment", "Messgeräte": "Measuring devices"}
BEREICH = {"pneumatik": ("50_pneumatic", "5095_ISO1219", "Pneumatik nach ISO 1219", "Pneumatics after ISO 1219"),
           "hydraulik": ("30_hydraulic", "3095_ISO1219", "Hydraulik nach ISO 1219", "Hydraulics after ISO 1219")}


def verzeichnis(pfad, de, en):
    os.makedirs(pfad, exist_ok=True)
    with open(os.path.join(pfad, "qet_directory"), "w", encoding="utf-8", newline="\n") as f:
        f.write(f'<qet-directory>\n    <names>\n        <name lang="de">{escape(de)}</name>\n'
                f'        <name lang="en">{escape(en)}</name>\n    </names>\n</qet-directory>\n')


def ordnername(text):
    for a, b in (("ä", "ae"), ("ö", "oe"), ("ü", "ue"), ("ß", "ss"), (" ", "_")):
        text = text.replace(a, b)
    return text


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--ziel", required=True, help="Ordner, darunter 50_pneumatic/… und 30_hydraulic/…")
    a = ap.parse_args()
    anzahl, gesehen = {}, set()
    for bereich, ordner, datei, s, namen, fluid, link, kind in alle():
        oben, unter, de, en = BEREICH[bereich]
        basis = os.path.join(a.ziel, oben, unter)
        verzeichnis(basis, de, en)
        pfad = os.path.join(basis, ordnername(ordner))
        verzeichnis(pfad, ordner, ORDNER_EN[ordner])
        if (bereich, datei) in gesehen:
            raise SystemExit(f"doppelter Dateiname {bereich}/{datei}")
        gesehen.add((bereich, datei))
        with open(os.path.join(pfad, datei + ".elmt"), "w", encoding="utf-8", newline="\n") as f:
            f.write(s.xml(f"{bereich}/{datei}", namen, fluid, link, kind))
        anzahl[bereich] = anzahl.get(bereich, 0) + 1
    print(", ".join(f"{k}: {v} Bauteile" for k, v in anzahl.items()), "->", a.ziel)


if __name__ == "__main__":
    main()
