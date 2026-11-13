"""FluidSIM-6-Symbole in QElectroTech-Bauteile umwandeln.

Liest die Symbolbibliothek einer FluidSIM-6-Installation (`sym/sym_fs.lib`, gzip-gepacktes XML) und schreibt je
Bibliotheksbauteil eine `.elmt`-Datei, in Ordnern wie in FluidSIM. Die Namen kommen aus FluidSIMs eigener
Übersetzung (`db/lang.db`, Deutsch und Englisch).

Übernommen werden:
- die Grafik in Ruhestellung (Linien, Polylinien, Rechtecke, Kreise, Bögen, Pfeile und Sperrstriche an
  Linienenden, gestrichelte Steuerleitungen); Maßstab 10/12, so dass FluidSIMs Anschlussraster 12 auf das
  QET-Raster 10 fällt;
- jeder Anschluss als QET-Anschluss mit Name (Portnummer 1, 2, 3 …), Medium und Richtung;
- Schalldämpfer und Blindstopfen, die in der Vorlage an einem Anschluss hängen, als Abschluss des Anschlusses;
- bei Wegeventilen Stellungen, Ruhestellung und Betätigung als `<fluid>`-Block; ein Ventil mit Magnet wird
  Master (Spule verknüpfbar), die Ventilspule (ET.Relay.ValveSolenoid) Slave;
- die Kennzeichnung als Text rechts neben dem Symbol (der Querverweis einer Spule kommt darunter).

Nicht übernommen: Simulationswerte, Animationsebenen (die Ruhestellung wird gezeichnet), Texte mit
FluidSIM-Platzhaltern.

    python fluidsim_symbole.py --fluidsim "C:/Program Files (x86)/Didactic/FluidSIM6" --ziel ZIELORDNER
    python fluidsim_symbole.py --fluidsim ... --ziel ZIELORDNER --bereich ~FolderPneumatic --nur-zeigen

Die Symbole gehören Festo Didactic. Laut Birger (13.11.2026) liegt eine Freigabe ohne Einschränkung vor;
die erzeugten Dateien nennen die Quelle in <informations>.
"""
import argparse
import gzip
import math
import re
import sys
import uuid
import xml.etree.ElementTree as ET
from pathlib import Path
from xml.sax.saxutils import escape, quoteattr

K = 10.0 / 12.0              # FluidSIM-Einheit -> QET-Pixel
PFEIL_LAENGE = 4.5           # Pfeilspitze, FluidSIM-Einheiten
PFEIL_BREITE = 2.0           # halbe Breite der Pfeilspitze
SPERR_BREITE = 3.0           # halbe Breite des Sperrstrichs (ortho)
STRICHART = {"dash": "dashed", "dot": "dotted", "dashdot": "dashdotted"}
ABSCHLUSS = {"Pneu.Supply.Silencer": "silencer", "Pneu.Supply.BlindPlug": "plug"}


def zahl(e, name, vorgabe=0.0):
    v = e.get(name)
    return float(v) if v not in (None, "") else vorgabe


def fmt(v):
    return ("%.2f" % v).rstrip("0").rstrip(".").replace("-0", "0") if abs(v) > 1e-9 else "0"


def stil(e, gefuellt=None):
    linie = STRICHART.get(e.get("style", ""), "normal")
    fuellung = "none"
    farbe = (e.get("fill-color") or "").upper()
    if gefuellt:
        fuellung = gefuellt
    elif farbe == "#FF000000":
        fuellung = "black"
    elif farbe == "#FFFFFFFF":
        fuellung = "white"
    return f"line-style:{linie};line-weight:normal;filling:{fuellung};color:black"


class Uebersetzung:
    def __init__(self, pfad: Path):
        self.texte = {}
        if not pfad.exists():
            return
        wurzel = ET.fromstring(gzip.decompress(pfad.read_bytes()))
        for t in wurzel.iter("text"):
            ident = t.get("ident")
            if ident:
                self.texte[ident] = (t.get("DE") or "", t.get("ENUS") or t.get("ENGB") or "")

    def namen(self, ident):
        de, en = self.texte.get(ident, ("", ""))
        roh = ident.lstrip("~").replace("_", " ")
        return (de or en or roh), (en or de or roh)


class Zeichner:
    """Sammelt die Grafik eines Symbols in FluidSIM-Koordinaten und schreibt sie als QET-XML."""

    def __init__(self):
        self.teile = []        # (art, daten)
        self.anschluesse = []  # dict
        self.linien = []       # Endpunkte aller Linien, für die Richtung der Anschlüsse

    # --- Grundformen -------------------------------------------------------------------------------------
    def linie(self, a, b, e, start=None, ende=None):
        self.teile.append(("line", (a, b, stil(e))))
        self.linien.append((a, b))
        for kappe, spitze, von in ((ende, b, a), (start, a, b)):
            if kappe in ("arrow1", "arrow2"):
                self.pfeil(spitze, von)
            elif kappe == "ortho1":
                self.sperre(spitze, von)

    def pfeil(self, spitze, von):
        dx, dy = spitze[0] - von[0], spitze[1] - von[1]
        l = math.hypot(dx, dy) or 1.0
        ux, uy = dx / l, dy / l
        bx, by = spitze[0] - ux * PFEIL_LAENGE, spitze[1] - uy * PFEIL_LAENGE
        pkte = [spitze, (bx - uy * PFEIL_BREITE, by + ux * PFEIL_BREITE),
                (bx + uy * PFEIL_BREITE, by - ux * PFEIL_BREITE)]
        self.teile.append(("polygon", (pkte, True, "line-style:normal;line-weight:thin;filling:black;color:black")))

    def sperre(self, spitze, von):
        dx, dy = spitze[0] - von[0], spitze[1] - von[1]
        l = math.hypot(dx, dy) or 1.0
        ux, uy = dx / l, dy / l
        a = (spitze[0] - uy * SPERR_BREITE, spitze[1] + ux * SPERR_BREITE)
        b = (spitze[0] + uy * SPERR_BREITE, spitze[1] - ux * SPERR_BREITE)
        self.teile.append(("line", (a, b, "line-style:normal;line-weight:normal;filling:none;color:black")))

    def polylinie(self, pkte, e, geschlossen=False):
        if len(pkte) < 2:
            return
        if e.get("fill-color"):
            geschlossen = True
        self.teile.append(("polygon", (pkte, geschlossen, stil(e))))
        for a, b in zip(pkte, pkte[1:]):
            self.linien.append((a, b))
        if e.get("start-cap") in ("arrow1", "arrow2"):
            self.pfeil(pkte[0], pkte[1])
        if e.get("end-cap") in ("arrow1", "arrow2"):
            self.pfeil(pkte[-1], pkte[-2])

    def rechteck(self, x, y, b, h, e):
        self.teile.append(("rect", ((x, y), (x + b, y + h), stil(e))))
        for a, c in (((x, y), (x + b, y)), ((x + b, y), (x + b, y + h)),
                     ((x + b, y + h), (x, y + h)), ((x, y + h), (x, y))):
            self.linien.append((a, c))

    def kreis(self, m, r, e, sx=1.0):
        self.teile.append(("ellipse", (m, r * sx, r, stil(e))))

    def bogen(self, m, r, start, ende, e):
        # FluidSIM: start/end in Grad. Angenommen: gegen den Uhrzeigersinn wie auf dem Bildschirm (wie Qt).
        # NICHT gegen FluidSIM geprüft; die Bögen der Pneumatik-Bibliothek sind meist symmetrisch zur Achse,
        # dort fällt eine falsche Richtung nicht auf.
        sweep = (ende - start) % 360 or 360
        self.teile.append(("arc", (m, r, start, sweep, stil(e))))
        for kappe, winkel, richtung in ((e.get("end-cap"), ende, 1), (e.get("start-cap"), start, -1)):
            if kappe in ("arrow1", "arrow2"):
                w = math.radians(winkel)
                spitze = (m[0] + r * math.cos(w), m[1] - r * math.sin(w))
                # Tangente in Laufrichtung des Bogens
                t = (-math.sin(w) * richtung, -math.cos(w) * richtung)
                self.pfeil(spitze, (spitze[0] - t[0], spitze[1] - t[1]))

    def text(self, p, inhalt, hoehe):
        self.teile.append(("text", (p, inhalt, hoehe)))

    # --- Ausgabe -----------------------------------------------------------------------------------------
    def richtung(self, p):
        """n/s/e/w: von der Linie weg, die am Anschluss endet; sonst vom Mittelpunkt des Symbols weg."""
        for a, b in self.linien:
            for ende, anderes in ((a, b), (b, a)):
                if math.hypot(ende[0] - p[0], ende[1] - p[1]) < 0.6:
                    dx, dy = ende[0] - anderes[0], ende[1] - anderes[1]
                    if abs(dx) < 1e-6 and abs(dy) < 1e-6:
                        continue
                    if abs(dy) >= abs(dx):
                        return "s" if dy > 0 else "n"
                    return "e" if dx > 0 else "w"
        xs = [q[0] for a, b in self.linien for q in (a, b)] or [p[0]]
        ys = [q[1] for a, b in self.linien for q in (a, b)] or [p[1]]
        mx, my = (min(xs) + max(xs)) / 2, (min(ys) + max(ys)) / 2
        dx, dy = p[0] - mx, p[1] - my
        if abs(dy) >= abs(dx):
            return "s" if dy > 0 else "n"
        return "e" if dx > 0 else "w"

    def xml(self, kennzeichen_text=True):
        # Ursprung: erster Fluidanschluss, so liegen die Anschlüsse auf dem QET-Raster
        anker = next((a["p"] for a in self.anschluesse), None)
        if anker is None:
            pkte = [q for _, d in self.teile for q in punkte(d)] or [(0.0, 0.0)]
            anker = (min(q[0] for q in pkte), min(q[1] for q in pkte))
        ox, oy = anker

        def t(p):
            return ((p[0] - ox) * K, (p[1] - oy) * K)

        zeilen = []
        alle = []
        for art, d in self.teile:
            if art == "line":
                a, b, s = d
                a, b = t(a), t(b)
                alle += [a, b]
                zeilen.append(f'<line x1="{fmt(a[0])}" y1="{fmt(a[1])}" x2="{fmt(b[0])}" y2="{fmt(b[1])}" '
                              f'end1="none" end2="none" length1="1.5" length2="1.5" style="{s}" antialias="true"/>')
            elif art == "polygon":
                pkte, zu, s = d
                pkte = [t(p) for p in pkte]
                alle += pkte
                attr = " ".join(f'x{i}="{fmt(p[0])}" y{i}="{fmt(p[1])}"' for i, p in enumerate(pkte, 1))
                zu_attr = "" if zu else ' closed="false"'
                zeilen.append(f'<polygon {attr}{zu_attr} style="{s}" antialias="true"/>')
            elif art == "rect":
                a, b, s = d
                a, b = t(a), t(b)
                x, y = min(a[0], b[0]), min(a[1], b[1])
                w, h = abs(b[0] - a[0]), abs(b[1] - a[1])
                alle += [a, b]
                zeilen.append(f'<rect x="{fmt(x)}" y="{fmt(y)}" width="{fmt(w)}" height="{fmt(h)}" rx="0" ry="0" '
                              f'style="{s}" antialias="true"/>')
            elif art == "ellipse":
                m, rx, ry, s = d
                m = t(m)
                rx, ry = rx * K, ry * K
                alle += [(m[0] - rx, m[1] - ry), (m[0] + rx, m[1] + ry)]
                zeilen.append(f'<ellipse x="{fmt(m[0] - rx)}" y="{fmt(m[1] - ry)}" width="{fmt(2 * rx)}" '
                              f'height="{fmt(2 * ry)}" style="{s}" antialias="true"/>')
            elif art == "arc":
                m, r, start, sweep, s = d
                m = t(m)
                r *= K
                alle += [(m[0] - r, m[1] - r), (m[0] + r, m[1] + r)]
                zeilen.append(f'<arc x="{fmt(m[0] - r)}" y="{fmt(m[1] - r)}" width="{fmt(2 * r)}" '
                              f'height="{fmt(2 * r)}" start="{fmt(start)}" angle="{fmt(sweep)}" style="{s}" '
                              f'antialias="true"/>')
            elif art == "text":
                p, inhalt, hoehe = d
                p = t(p)
                groesse = max(4, round(hoehe * K * 0.75))
                alle.append(p)
                # FluidSIM: x1/y1 oben links; QET-Text: y ist die Grundlinie
                zeilen.append(f'<text x="{fmt(p[0])}" y="{fmt(p[1] + hoehe * K * 0.8)}" size="{groesse}" '
                              f'color="#000000" text={quoteattr(inhalt)}/>')

        for a in self.anschluesse:
            p = t(a["p"])
            alle.append(p)
            if a["name"]:
                # Portnummer sichtbar wie in FluidSIM: neben der Anschlusslinie, zwischen Anschluss und Körper.
                # Fester Text (Teil der Zeichnung, y = Grundlinie), damit QET ihn nicht vom Symbol wegrückt.
                r = a["richtung"]
                breite = 2.6 * len(a["name"])
                tx, ty = {"n": (p[0] + 1.5, p[1] + 5), "s": (p[0] + 1.5, p[1] - 2),
                          "w": (p[0] + 1, p[1] - 1.5), "e": (p[0] - 1 - breite, p[1] - 1.5)}[r]
                zeilen.append(f'<text x="{fmt(tx)}" y="{fmt(ty)}" size="4" color="#000000" '
                              f'text={quoteattr(a["name"])}/>')
            attr = [f'uuid="{{{uuid.uuid4()}}}"', f'name={quoteattr(a["name"])}', f'x="{fmt(round(p[0], 1))}"',
                    f'y="{fmt(round(p[1], 1))}"', f'orientation="{a["richtung"]}"', 'type="Generic"']
            if a["medium"] != "electric":
                attr.append(f'medium="{a["medium"]}"')
            if a.get("abschluss"):
                attr.append(f'port_end="{a["abschluss"]}"')
            zeilen.append("<terminal " + " ".join(attr) + "/>")

        xs = [p[0] for p in alle] or [0.0]
        ys = [p[1] for p in alle] or [0.0]
        if kennzeichen_text:
            # Rechts neben dem Symbol, oben: QET setzt den Querverweis einer Spule (Slave) unter die
            # Kennzeichnung, so überdeckt er das Symbol nicht
            lx, ly = max(xs) + 4, min(ys) - 2
            xs.append(lx + 30)
            zeilen.insert(0, (
                f'<dynamic_text x="{fmt(lx)}" y="{fmt(ly)}" z="3" text_width="-1" '
                f'Halignment="AlignLeft" Valignment="AlignTop" frame="false" rotation="0" '
                f'keep_visual_rotation="false" text_from="ElementInfo" uuid="{{{uuid.uuid4()}}}" '
                f'font="Liberation Sans,7,-1,5,0,0,0,0,0,0,Regular"><text></text><info_name>label</info_name>'
                f'</dynamic_text>'))
            ys.append(ly)
        x0 = math.floor(min(xs) / 10) * 10
        y0 = math.floor(min(ys) / 10) * 10
        x1 = math.ceil(max(xs) / 10) * 10
        y1 = math.ceil(max(ys) / 10) * 10
        return zeilen, (max(10, x1 - x0), max(10, y1 - y0), -x0, -y0)


def punkte(d):
    out = []
    for v in d if isinstance(d, tuple) else ():
        if isinstance(v, tuple) and len(v) == 2 and all(isinstance(z, float) for z in v):
            out.append(v)
        elif isinstance(v, list):
            out += [q for q in v if isinstance(q, tuple)]
    return out


def ebene_lesen(z: Zeichner, ebene, anschluss_info):
    """Zeichnet eine <symbol-layer> in z. anschluss_info: {(ebene, ident): Abschluss}."""
    if ebene.get("visible") == "false":
        return
    ox, oy = zahl(ebene, "ref-x"), zahl(ebene, "ref-y")
    sx = zahl(ebene, "scale-x", 1.0)
    sy = zahl(ebene, "scale-y", 1.0)
    name = ebene.get("ident", "")

    def p(x, y):
        return (ox + sx * x, oy + sy * y)

    for e in ebene:
        if e.tag == "line":
            x1, y1 = zahl(e, "x1"), zahl(e, "y1")
            x2 = zahl(e, "x2", x1) if e.get("x2") is not None else x1
            y2 = zahl(e, "y2", y1) if e.get("y2") is not None else y1
            z.linie(p(x1, y1), p(x2, y2), e, e.get("start-cap"), e.get("end-cap"))
        elif e.tag == "polyline":
            pkte = []
            for paar in (e.get("points") or "").split():
                a, b = paar.split(",")
                pkte.append(p(float(a), float(b)))
            z.polylinie(pkte, e)
        elif e.tag == "rect":
            x, y = p(zahl(e, "x1"), zahl(e, "y1"))
            z.rechteck(x, y, zahl(e, "width") * sx, zahl(e, "height") * sy, e)
        elif e.tag == "circle":
            z.kreis(p(zahl(e, "cx"), zahl(e, "cy")), zahl(e, "r") * sy, e, sx / sy if sy else 1.0)
        elif e.tag == "arc":
            z.bogen(p(zahl(e, "cx"), zahl(e, "cy")), zahl(e, "r") * sy, zahl(e, "start"), zahl(e, "end"), e)
        elif e.tag == "text":
            inhalt = (e.text or "").strip()
            if inhalt and not inhalt.startswith("?") and "~" not in inhalt:
                z.text(p(zahl(e, "x1"), zahl(e, "y1")), inhalt, zahl(e, "height", 8))
        elif e.tag == "connector":
            art = e.get("type")
            if e.get("visible") == "false" or art not in ("pneumatic", "electric", "hydraulic"):
                continue
            z.anschluesse.append({
                "p": p(zahl(e, "cx"), zahl(e, "cy")),
                "name": e.get("designation") or "",
                "medium": art,
                "abschluss": anschluss_info.get((name, e.get("ident"))),
                "schluessel": (name, e.get("ident")),
            })


def hauptbauteil(datei):
    """Das Bauteil der Vorlage, das kein Abschluss und kein Text ist."""
    kandidaten = [c for c in datei.iter("component") if c.get("object-ref") not in ABSCHLUSS]
    return kandidaten[0] if kandidaten else None


def werte(komponente):
    out = {}
    for c in komponente:
        if c.tag in ("const", "parameter") and c.get("ident"):
            out[c.get("ident")] = c.get("value", "")
    return out


def fluid_block(objekt, w, ebenen_namen):
    """<fluid>-Block nach der Bauart der Vorlage, oder ""."""
    if objekt.startswith("Pneu.DirectionalValve") or objekt.startswith("Pneu.Valve.Directional"):
        try:
            anzahl = int(float(w.get("posNoMax", "0")))
            ruhe = int(float(w.get("posNoInit", "1")))
        except ValueError:
            return "", 0
        if not 1 <= anzahl <= 4:
            return "", 0
        ruhe = min(max(ruhe, 1), anzahl)
        seiten = []
        magnete = 0
        for seite, kenn in (("left", "L"), ("right", "R")):
            arten = []
            if any(n.startswith("AEM" + kenn) for n in ebenen_namen):
                arten.append("solenoid")
                magnete += 1
            if any(n.startswith("AMAN" + kenn) for n in ebenen_namen):
                arten.append("manual")
            attr = f'side="{seite}" types="{",".join(arten)}"'
            if w.get("spring" + kenn) == "True":
                attr += ' spring="true"'
            if any(n.startswith("PLT" + kenn) for n in ebenen_namen):
                attr += ' pilot="true"'
            if arten or "spring" in attr or "pilot" in attr:
                seiten.append(f"        <actuation {attr}/>")
        stellungen = "\n".join("            <position/>" for _ in range(anzahl))
        return (f'    <fluid kind="valve">\n        <positions initial="{ruhe}">\n{stellungen}\n'
                f'        </positions>\n' + "\n".join(seiten) + "\n    </fluid>\n"), magnete
    if objekt.startswith("Pneu.Actuator") and "CYL" in objekt.upper():
        return '    <fluid kind="cylinder">\n        <cylinder action="double"/>\n    </fluid>\n', 0
    if objekt == "ET.Relay.ValveSolenoid":
        return '    <fluid kind="solenoid"/>\n', 0
    for vorsilbe, art in (("Pneu.Supply", "source"), ("Pneu.Maintenance", "treatment"),
                          ("Pneu.FlowControl", "flow_control"), ("Pneu.Sensor", "sensor"),
                          ("Pneu.Measuring", "sensor")):
        if objekt.startswith(vorsilbe):
            return f'    <fluid kind="{art}"/>\n', 0
    return "", 0


def umwandeln(datei, namen, quelle_name):
    """Text der .elmt für eine <file> der Bibliothek, oder None."""
    grafik = datei.find("section[@type='symbol-graphics']")
    haupt = hauptbauteil(datei)
    if grafik is None or haupt is None:
        return None
    symbol = next((s for s in grafik.findall("symbol") if s.get("ident") == haupt.get("symbol-ref")), None)
    if symbol is None:
        return None

    # Abschlüsse: welcher Anschluss des Hauptbauteils hängt an einem Schalldämpfer oder Blindstopfen
    abschluss_von = {c.get("ident"): ABSCHLUSS[c.get("object-ref")]
                     for c in datei.iter("component") if c.get("object-ref") in ABSCHLUSS}
    verweis = {c.get("ident"): c.get("con-ref", "") for c in haupt.findall("connector")}
    anschluss_info = {}
    for v in datei.iter("connect"):
        enden = [v.get("con1", ""), v.get("con2", "")]
        for a, b in (enden, enden[::-1]):
            komp_a, _, ans_a = a.partition(".")
            komp_b = b.partition(".")[0]
            if komp_a == haupt.get("ident") and komp_b in abschluss_von and ans_a in verweis:
                ebene, _, ident = verweis[ans_a].rpartition(".")
                anschluss_info[(ebene, ident)] = abschluss_von[komp_b]

    # Meist hängt der Abschluss ohne Leitung am Anschluss: sein Anschlusspunkt liegt auf dem des Ventils.
    # Symbolkoordinaten sind relativ zur Mitte des Bauteils.
    def mitte(c):
        return (zahl(c, "x1") + zahl(c, "width") / 2, zahl(c, "y1") + zahl(c, "height") / 2)

    symbole = {s.get("ident"): s for s in grafik.findall("symbol")}
    hm = mitte(haupt)
    abschluss_punkte = []
    for c in datei.iter("component"):
        if c.get("object-ref") not in ABSCHLUSS or c.get("symbol-ref") not in symbole:
            continue
        cm = mitte(c)
        for eb in symbole[c.get("symbol-ref")].findall("symbol-layer"):
            for con in eb.findall("connector"):
                px = cm[0] + zahl(eb, "ref-x") + zahl(con, "cx") - hm[0]
                py = cm[1] + zahl(eb, "ref-y") + zahl(con, "cy") - hm[1]
                abschluss_punkte.append((px, py, ABSCHLUSS[c.get("object-ref")]))

    z = Zeichner()
    ebenen_namen = []
    for ebene in symbol.findall("symbol-layer"):
        if ebene.get("visible") != "false":
            ebenen_namen.append(ebene.get("ident", ""))
        ebene_lesen(z, ebene, anschluss_info)
    for a in z.anschluesse:
        if not a.get("abschluss"):
            for px, py, art in abschluss_punkte:
                if math.hypot(a["p"][0] - px, a["p"][1] - py) < 1.5:
                    a["abschluss"] = art
    if not z.teile:
        return None
    for a in z.anschluesse:
        a["richtung"] = z.richtung(a["p"])
    # Fluidanschlüsse zuerst: sie bestimmen das Raster
    z.anschluesse.sort(key=lambda a: a["medium"] == "electric")

    objekt = haupt.get("object-ref", "")
    fluid, magnete = fluid_block(objekt, werte(haupt), ebenen_namen)
    zeilen, (breite, hoehe, hx, hy) = z.xml()

    link = "simple"
    art = ""
    if magnete:
        link = "master"
        art = ('    <kindInformations>\n        <kindInformation name="type">coil</kindInformation>\n'
               f'        <kindInformation name="max_slaves">{magnete}</kindInformation>\n    </kindInformations>\n')
    elif objekt == "ET.Relay.ValveSolenoid":
        link = "slave"
        art = ('    <kindInformations>\n        <kindInformation name="type">simple</kindInformation>\n'
               '        <kindInformation name="state">Other</kindInformation>\n'
               '        <kindInformation name="number">1</kindInformation>\n    </kindInformations>\n')

    de, en = namen
    return (f'<definition version="0.90" type="element" link_type="{link}" width="{breite}" height="{hoehe}" '
            f'hotspot_x="{hx}" hotspot_y="{hy}">\n'
            f'    <uuid uuid="{{{uuid.uuid4()}}}"/>\n'
            f'    <names>\n        <name lang="de">{escape(de)}</name>\n'
            f'        <name lang="en">{escape(en)}</name>\n    </names>\n'
            f'{art}{fluid}'
            f'    <informations>Quelle: FluidSIM 6, Bibliothek von Festo Didactic ({escape(quelle_name)}, '
            f'{escape(objekt)}). Umgewandelt mit tools/symbole/fluidsim_symbole.py.</informations>\n'
            f'    <description>\n' + "\n".join("        " + zl for zl in zeilen) + "\n    </description>\n"
            f'</definition>\n')


def ordnername(name):
    name = re.sub(r"[^\w\- ]+", "", name.lstrip("~")).strip()
    return re.sub(r"\s+", "_", name) or "Ordner"


def ordner_datei(namen):
    de, en = namen
    return (f'<qet-directory>\n    <names>\n        <name lang="de">{escape(de)}</name>\n'
            f'        <name lang="en">{escape(en)}</name>\n    </names>\n</qet-directory>\n')


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--fluidsim", type=Path, required=True, help="Installationsordner von FluidSIM 6")
    ap.add_argument("--ziel", type=Path, help="Ordner für die Bauteile")
    ap.add_argument("--bereich", default="~FolderPneumatic",
                    help="Ordner der Bibliothek, z. B. ~FolderPneumatic (Vorgabe), ~FolderHydraulic, alle")
    ap.add_argument("--nur-zeigen", action="store_true")
    a = ap.parse_args(argv)
    if not a.nur_zeigen and not a.ziel:
        ap.error("--ziel fehlt (oder --nur-zeigen)")

    lib = a.fluidsim / "sym" / "sym_fs.lib"
    wurzel = ET.fromstring(gzip.decompress(lib.read_bytes()))
    sprache = Uebersetzung(a.fluidsim / "db" / "lang.db")
    oben = wurzel.find("section")
    if a.bereich == "alle":
        start = [oben]
    else:
        start = [s for s in oben.iter("section") if s.get("name") == a.bereich]
        if not start:
            sys.exit(f"Bereich {a.bereich} nicht gefunden")

    zahl_ok = zahl_leer = 0

    def gehe(abschnitt, pfad):
        nonlocal zahl_ok, zahl_leer
        namen = sprache.namen(abschnitt.get("name", ""))
        hier = pfad / ordnername(namen[1])
        if not a.nur_zeigen:
            hier.mkdir(parents=True, exist_ok=True)
            (hier / "qet_directory").write_text(ordner_datei(namen), encoding="utf-8", newline="\n")
        for datei in abschnitt.findall("file"):
            eig = datei.find("section[@type='properties']")
            ident = eig.get("name", "") if eig is not None else ""
            bnamen = sprache.namen(ident) if ident else (datei.get("name"), datei.get("name"))
            text = umwandeln(datei, bnamen, datei.get("name", ""))
            if text is None:
                zahl_leer += 1
                continue
            zahl_ok += 1
            stamm = Path(datei.get("name", "symbol")).stem
            if a.nur_zeigen:
                print(f"{hier / stamm}.elmt  {bnamen[0]}")
            else:
                (hier / f"{stamm}.elmt").write_text(text, encoding="utf-8", newline="\n")
        for unter in abschnitt.findall("section"):
            if unter.get("type") == "circuits":
                gehe(unter, hier)

    ziel = a.ziel or Path("FluidSIM")
    for s in start:
        gehe(s, ziel)
    print(f"{zahl_ok} Bauteile geschrieben, {zahl_leer} Vorlagen ohne Symbol übersprungen.", file=sys.stderr)


if __name__ == "__main__":
    main()
