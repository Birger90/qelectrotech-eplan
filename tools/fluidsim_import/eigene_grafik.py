"""Eigene Grafik für Bauteile aus FluidSIM-Schaltungen (statt der FluidSIM-Symbole).

Aus der .circ werden nur Daten genommen: Lage und Namen der Anschlüsse, die Ebenennamen (welche Betätigung, welche
Stellung), die Schaltwege der Stellungen, die Ausdehnung des Bauteils, seine Klasse und Kennwerte (Feder links/
rechts). Gezeichnet wird alles neu nach DIN ISO 1219-1 bzw. IEC 60617 aus einfachen Grundelementen: Ventilkästen
mit Pfeilen und Sperren, Betätigungen (Magnet, Taster, Rolle, Vorsteuerung, Feder), Zylinder, Quellen, Tank,
Messgeräte, Schaltkontakte, Spulen, Leuchten, Potentiale, Zweipole (Widerstand, Kondensator, Diode, LED,
Sicherung, Gleichspannungsquelle), Bipolartransistor, Signalgenerator/Oszilloskop; jede andere Klasse als Kasten mit ihrem Namen. So enthält
ein übernommenes Projekt keine Festo-Grafik, und jeder kann eine Schaltung übernehmen, auch ohne FluidSIM.

ersetzen() gibt ein <symbol> im FluidSIM-Format zurück: dieselben Ebenen (Namen, Sichtbarkeit, Lage) mit nur ihren
Anschlüssen, dazu eine Ebene "EIGEN" mit der neuen Grafik. Der Umwandler (fluidsim_symbole.bauteil_umwandeln)
zeichnet es wie jedes Symbol, mit Drehung, Spiegelung und den Anschluss-UUIDs der Schaltung.
"""
import math
import re
import xml.etree.ElementTree as ET

SCHWARZ = "#FF000000"
BETAETIGUNG = ("AEM", "AMAN", "AMECH", "APNEU", "PLT")


def _z(e, name, vorgabe=0.0):
    v = e.get(name)
    try:
        return float(v) if v not in (None, "") else vorgabe
    except ValueError:
        return vorgabe


class Stift:
    """Sammelt Grundelemente im Koordinatensystem des Symbols (FluidSIM-Einheiten, y nach unten)."""

    def __init__(self):
        self.ebene = ET.Element("symbol-layer", {"ident": "EIGEN"})

    def linie(self, x1, y1, x2, y2, ende=None, strich=None):
        a = {"x1": repr(float(x1)), "y1": repr(float(y1)), "x2": repr(float(x2)), "y2": repr(float(y2))}
        if ende:
            a["end-cap"] = ende
        if strich:
            a["style"] = strich
        ET.SubElement(self.ebene, "line", a)

    def zug(self, punkte, gefuellt=False):
        a = {"points": " ".join(f"{float(x)!r},{float(y)!r}" for x, y in punkte)}
        if gefuellt:
            a["fill-color"] = SCHWARZ
        ET.SubElement(self.ebene, "polyline", a)

    def kasten(self, x, y, b, h):
        ET.SubElement(self.ebene, "rect", {"x1": repr(float(x)), "y1": repr(float(y)),
                                           "width": repr(float(b)), "height": repr(float(h))})

    def kreis(self, cx, cy, r, gefuellt=False):
        a = {"cx": repr(float(cx)), "cy": repr(float(cy)), "r": repr(float(r))}
        if gefuellt:
            a["fill-color"] = SCHWARZ
        ET.SubElement(self.ebene, "circle", a)

    def text(self, x, y, inhalt, hoehe=6):
        t = ET.SubElement(self.ebene, "text", {"x1": repr(float(x)), "y1": repr(float(y)), "height": repr(float(hoehe))})
        t.text = inhalt

    def feder(self, x1, y, x2, h=4):
        n = 6
        pk = [(x1, y)] + [(x1 + (x2 - x1) * i / n, y + (h if i % 2 else -h)) for i in range(1, n)] + [(x2, y)]
        self.zug(pk)


def anschluesse(symbol):
    """[(Ebene, Anschluss-Element, x, y)] in Symbolkoordinaten, nur echte Anschlüsse (keine Beschriftungen)."""
    out = []
    for eb in symbol.findall("symbol-layer"):
        if eb.get("visible") == "false":
            continue
        ox, oy = _z(eb, "ref-x"), _z(eb, "ref-y")
        sx, sy = _z(eb, "scale-x", 1.0), _z(eb, "scale-y", 1.0)
        for c in eb.iter("connector"):
            if c.get("type") in ("pneumatic", "hydraulic", "electric"):
                out.append((eb, c, ox + sx * _z(c, "cx"), oy + sy * _z(c, "cy")))
    return out


def umriss(symbol):
    """Ausdehnung der Originalgrafik (nur die Grenzen) als (x0, y0, x1, y1)."""
    xs, ys = [], []
    for eb in symbol.findall("symbol-layer"):
        if eb.get("visible") == "false":
            continue
        ox, oy = _z(eb, "ref-x"), _z(eb, "ref-y")
        sx, sy = _z(eb, "scale-x", 1.0), _z(eb, "scale-y", 1.0)

        def p(x, y):
            xs.append(ox + sx * x)
            ys.append(oy + sy * y)
        for e in eb:
            if e.tag == "line":
                x1, y1 = _z(e, "x1"), _z(e, "y1")
                p(x1, y1)
                p(_z(e, "x2", x1), _z(e, "y2", y1))
            elif e.tag == "rect":
                p(_z(e, "x1"), _z(e, "y1"))
                p(_z(e, "x1") + _z(e, "width"), _z(e, "y1") + _z(e, "height"))
            elif e.tag == "circle":
                r = _z(e, "r")
                p(_z(e, "cx") - r, _z(e, "cy") - r)
                p(_z(e, "cx") + r, _z(e, "cy") + r)
            elif e.tag == "polyline":
                for pk in (e.get("points") or "").split():
                    a, b = pk.split(",")
                    p(float(a), float(b))
            elif e.tag == "connector":
                p(_z(e, "cx"), _z(e, "cy"))
    if not xs:
        return (-12.0, -12.0, 12.0, 12.0)
    return (min(xs), min(ys), max(xs), max(ys))


def _stummel(s, anschl, x0, y0, x1, y1):
    """Anschlussleitung vom Anschlusspunkt bis an den Kasten (x0..x1, y0..y1)."""
    for _, _, cx, cy in anschl:
        if cy >= y1:
            s.linie(cx, cy, cx, y1)
        elif cy <= y0:
            s.linie(cx, cy, cx, y0)
        elif cx <= x0:
            s.linie(cx, cy, x0, cy)
        elif cx >= x1:
            s.linie(cx, cy, x1, cy)


# ---------------------------------------------------------------------------------------------- Wegeventil
def _ventil(s, symbol, komponente, stellungen):
    bloecke = sorted((eb for eb in symbol.findall("symbol-layer") if re.match(r"^B\d+_\d+$", eb.get("ident") or "")),
                     key=lambda eb: _z(eb, "ref-x"))
    con = next((eb for eb in symbol.findall("symbol-layer") if re.match(r"^CON\d+$", eb.get("ident") or "")), None)
    if not bloecke or con is None:
        return False
    ports = {}      # Name -> (x im Baustein, oben?)
    for c in con.iter("connector"):
        ports[c.get("designation") or c.get("ident")] = (_z(c, "cx") + _z(con, "ref-x"), _z(c, "cy") < 0)
    breite, hoehe = 36.0, 24.0
    for eb in bloecke:
        r = eb.find("rect")
        if r is not None:
            breite, hoehe = _z(r, "width", 36.0), _z(r, "height", 24.0)
            break
    halb = hoehe / 2
    oy = _z(bloecke[0], "ref-y")
    links = min(_z(eb, "ref-x") for eb in bloecke)
    rechts = max(_z(eb, "ref-x") for eb in bloecke) + breite
    # die Anschlüsse sitzen an der Ruhestellung: deren Baustein ist der Bezug der Anschluss-x-Lagen
    try:
        ruhe = int(float(next((e.get("value") for e in komponente if e.get("ident") == "posNoInit"), "1")))
    except ValueError:
        ruhe = 1
    bezug = _z(bloecke[min(max(ruhe, 1), len(bloecke)) - 1], "ref-x")
    for i, eb in enumerate(bloecke):
        bx = _z(eb, "ref-x")
        dx = bx - bezug
        s.kasten(bx, oy - halb, breite, hoehe)
        wege, gesperrt = stellungen[i] if stellungen and i < len(stellungen) else ([], set())
        for von, nach in wege:
            if von not in ports or nach not in ports:
                continue
            (xa, oa), (xb, ob) = ports[von], ports[nach]
            ya = oy - halb if oa else oy + halb
            yb = oy - halb if ob else oy + halb
            if oa == ob:            # beide auf einer Seite: über die Kastenmitte
                s.zug([(dx + xa, ya), (dx + xa, oy), (dx + xb, oy)])
                s.linie(dx + xb, oy, dx + xb, yb, ende="arrow1")
            else:
                s.linie(dx + xa, ya, dx + xb, yb, ende="arrow1")
        for name in gesperrt:
            if name in ports:
                x, oben = ports[name]
                y = oy - halb if oben else oy + halb
                s.linie(dx + x, y, dx + x, y + (5 if oben else -5), ende="ortho1")
    for c in con.iter("connector"):
        cx, cy = _z(con, "ref-x") + _z(c, "cx"), _z(con, "ref-y") + _z(c, "cy")
        s.linie(cx, cy, cx, oy - halb if cy < oy else oy + halb)
    # Steueranschlüsse (12, 14 …) an den Betätigungsebenen: Leitung bis an die Betätigung
    for eb, c, cx, cy in anschluesse(symbol):
        name = eb.get("ident") or ""
        if eb is con or re.match(r"^B\d+_\d+$", name):
            continue
        kante = links if cx < (links + rechts) / 2 else rechts
        ziel_x = kante - 6 if kante == links else kante + 6
        s.zug([(cx, cy), (cx, oy), (ziel_x, oy)] if abs(cy - oy) > 0.5 else [(cx, cy), (ziel_x, oy)])
    # Betätigungen links und rechts, nach außen gestapelt
    namen = [eb.get("ident") or "" for eb in symbol.findall("symbol-layer") if eb.get("visible") != "false"]
    werte = {e.get("ident"): e.get("value") for e in komponente if e.tag in ("const", "parameter")}
    for seite, kante, r in (("L", links, -1), ("R", rechts, 1)):
        x = kante
        for n in namen:
            vorsilbe = next((v for v in BETAETIGUNG if n.startswith(v)), None)
            if vorsilbe is None or n[len(vorsilbe):len(vorsilbe) + 1] != seite:
                continue
            if vorsilbe == "AEM":
                s.kasten(min(x, x + 8 * r), oy - 6, 8, 12)
                s.linie(min(x, x + 8 * r), oy + 6, max(x, x + 8 * r), oy - 6)
                x += 8 * r
            elif vorsilbe == "AMAN":
                s.linie(x, oy, x + 8 * r, oy)
                s.linie(x + 8 * r, oy - 5, x + 8 * r, oy + 5)
                x += 8 * r
            elif vorsilbe == "AMECH":
                s.linie(x, oy, x + 5 * r, oy)
                s.kreis(x + 8 * r, oy, 3)
                x += 11 * r
            else:                   # pneumatisch / vorgesteuert
                s.zug([(x, oy), (x + 6 * r, oy - 4), (x + 6 * r, oy + 4), (x, oy)], gefuellt=True)
                x += 6 * r
        if werte.get("spring" + seite) == "True":
            s.feder(x, oy, x + 12 * r)
    return True


# ---------------------------------------------------------------------------------------------- weitere Klassen
def _zylinder(s, anschl, u, einfach):
    x0, y0, x1, y1 = u
    mitte = (y0 + y1) / 2
    koerper = x0 + (x1 - x0) * 0.72
    hoch = min(y1 - y0, 24.0)
    oben, unten = mitte - hoch / 2, mitte + hoch / 2
    s.kasten(x0, oben, koerper - x0, hoch)
    kolben = x0 + (koerper - x0) * 0.25
    s.linie(kolben, oben, kolben, unten)
    s.kasten(kolben, mitte - 1.5, x1 - kolben, 3)
    if einfach:
        s.feder(kolben + 2, mitte - hoch / 4, koerper - 2, 3)
    for _, _, cx, cy in anschl:
        s.linie(cx, cy, cx, unten if cy > mitte else oben)


def _quelle(s, anschl, u, hydraulik):
    x0, y0, x1, y1 = u
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    r = max(min(x1 - x0, y1 - y0) / 2 - 2, 5)
    s.kreis(cx, cy, r)
    dreieck = [(cx, cy - r + 1), (cx - 3, cy - r + 6), (cx + 3, cy - r + 6), (cx, cy - r + 1)]
    s.zug(dreieck, gefuellt=hydraulik)
    for _, _, ax, ay in anschl:
        if ay < cy - r:
            s.linie(ax, ay, ax, cy - r)
        elif ay > cy + r:
            s.linie(ax, ay, ax, cy + r)
        elif ax < cx:
            s.linie(ax, ay, cx - r, ay)
        else:
            s.linie(ax, ay, cx + r, ay)


def _tank(s, anschl, u):
    x0, y0, x1, y1 = u
    s.zug([(x0, y0), (x0, y1), (x1, y1), (x1, y0)])
    for _, _, ax, ay in anschl:
        s.linie(ax, ay, ax, y1 - 3)


def _messgeraet(s, anschl, u):
    x0, y0, x1, y1 = u
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    r = max(min(x1 - x0, y1 - y0) / 2 - 1, 5)
    s.kreis(cx, cy, r)
    s.linie(cx - r * 0.6, cy + r * 0.6, cx + r * 0.6, cy - r * 0.6, ende="arrow1")
    for _, _, ax, ay in anschl:
        s.linie(ax, ay, cx, cy + r if ay > cy else cy - r)


def _kasten_mit_name(s, anschl, u, name):
    x0, y0, x1, y1 = u
    if len(anschl) >= 2:
        # Kasten zwischen den Anschlüssen statt der ganzen Originalausdehnung
        xs, ys = [a[2] for a in anschl], [a[3] for a in anschl]
        x0, x1 = max(x0, min(xs) + 4), min(x1, max(xs) - 4)
        y0, y1 = max(y0, min(ys) + 4), min(y1, max(ys) - 4)
        if x1 - x0 < 16:
            x0, x1 = (min(xs) + max(xs)) / 2 - 12, (min(xs) + max(xs)) / 2 + 12
        if y1 - y0 < 16:
            y0, y1 = (min(ys) + max(ys)) / 2 - 8, (min(ys) + max(ys)) / 2 + 8
    pad = 0
    if x1 - x0 < 12:
        x0, x1 = (x0 + x1) / 2 - 6, (x0 + x1) / 2 + 6
    if y1 - y0 < 12:
        y0, y1 = (y0 + y1) / 2 - 6, (y0 + y1) / 2 + 6
    s.kasten(x0, y0, x1 - x0, y1 - y0)
    if name:
        s.text(x0 + 2, (y0 + y1) / 2 + 2, name[:16], 5)
    _stummel(s, anschl, x0, y0, x1, y1)


def _kontakt(s, anschl, art, betaetigt):
    """Schaltkontakt zwischen dem oberen und dem unteren Anschluss (IEC 60617), Wechsler mit drittem Anschluss."""
    if len(anschl) < 2:
        return False
    pk = sorted(anschl, key=lambda a: a[3])
    oben, unten = pk[0], pk[-1]
    x, xu = oben[2], unten[2]
    ya, yb = oben[3], unten[3]
    m1, m2 = ya + (yb - ya) * 0.35, ya + (yb - ya) * 0.65
    s.linie(x, ya, x, m1)
    s.linie(xu, yb, xu, m2)
    if art == "break":
        s.linie(x, m1, x + 6, m1)
        s.linie(xu, m2, x + 7, m1 - 2)
    else:
        s.linie(xu, m2, xu - 7, m1 - 1)
    for a in pk[1:-1]:
        s.linie(a[2], a[3], a[2], m1)
    if betaetigt:
        m = (m1 + m2) / 2
        s.linie(xu - 4, m, xu - 12, m, strich="dash")
        s.linie(xu - 12, m - 4, xu - 12, m + 4)
    return True


def _spule(s, anschl, ventil):
    if len(anschl) < 2:
        return False
    pk = sorted(anschl, key=lambda a: a[3])
    x = pk[0][2]
    m = (pk[0][3] + pk[-1][3]) / 2
    s.kasten(x - 8, m - 5, 16, 10)
    if ventil:
        s.linie(x - 8, m + 5, x + 8, m - 5)
    s.linie(x, pk[0][3], x, m - 5)
    s.linie(pk[-1][2], pk[-1][3], pk[-1][2], m + 5)
    return True


def _lampe(s, anschl):
    if len(anschl) < 2:
        return False
    pk = sorted(anschl, key=lambda a: a[3])
    x = pk[0][2]
    m = (pk[0][3] + pk[-1][3]) / 2
    s.kreis(x, m, 6)
    s.linie(x - 4.2, m - 4.2, x + 4.2, m + 4.2)
    s.linie(x - 4.2, m + 4.2, x + 4.2, m - 4.2)
    s.linie(x, pk[0][3], x, m - 6)
    s.linie(pk[-1][2], pk[-1][3], pk[-1][2], m + 6)
    return True


def _potential(s, anschl, text):
    if not anschl:
        return False
    _, _, x, y = anschl[0]
    yb = y - 8 if y >= 0 else y + 8
    s.linie(x, y, x, yb)
    s.linie(x - 6, yb, x + 6, yb)
    s.text(x + 8, yb + 2, text, 6)
    return True


# ---------------------------------------------------------------------------------------------- Elektrik
ZWEIPOL = ("Resistor", "Potentiometer", "Capacitor", "CapacitorPol", "Diode", "LED", "Fuse", "ConstantVoltage")
KREISQUELLE = ("WaveformGenerator", "Oscilloscope")


def _zweipol(s, anschl, objekt):
    """Zweipol nach IEC 60617 zwischen den Anschlüssen p und n: Widerstand, Potentiometer, Kondensator,
    Diode/LED, Sicherung, Gleichspannungsquelle; Zuleitungen bis an den Körper."""
    el = [a for a in anschl if a[1].get("type") == "electric"]
    if len(el) < 2:
        return False
    plus = next((a for a in el if a[1].get("designation") == "p"), el[0])
    minus = next((a for a in el if a[1].get("designation") == "n" and a is not plus),
                 next(a for a in el if a is not plus))
    ax = Achse((plus[2], plus[3]), (minus[2], minus[3]))
    teil = objekt.split(".")[-1]

    def zu(u):
        return ax.p(u)
    if teil.startswith(("Capacitor", "ConstantVoltage")):
        L = 1.5
    elif teil.startswith(("Diode", "LED")):
        L = min(4.0, ax.halb - 1)
    else:
        L = max(min(9.0, ax.halb - 2), 3.0)
    s.linie(plus[2], plus[3], *zu(-L))
    s.linie(*zu(L), minus[2], minus[3])
    if teil.startswith(("Resistor", "Potentiometer", "Fuse")):
        s.zug([ax.p(-L, -3.5), ax.p(L, -3.5), ax.p(L, 3.5), ax.p(-L, 3.5), ax.p(-L, -3.5)])
        if teil.startswith("Fuse"):
            s.linie(*ax.p(-L), *ax.p(L))
        if teil.startswith("Potentiometer"):
            schleifer = [a for a in el if a is not plus and a is not minus]
            if schleifer:
                w = schleifer[0]
                ziel = min((ax.p(0, -3.5), ax.p(0, 3.5)), key=lambda q: (q[0] - w[2]) ** 2 + (q[1] - w[3]) ** 2)
                s.linie(w[2], w[3], *ziel, ende="arrow1")
    elif teil.startswith("Capacitor"):
        for u in (-1.5, 1.5):
            s.linie(*ax.p(u, -6), *ax.p(u, 6))
        if teil == "CapacitorPol":
            q = ax.p(-5, -7)
            s.text(q[0] - 2, q[1] + 2, "+", 5)
    elif teil.startswith("ConstantVoltage"):
        s.linie(*ax.p(-1.5, -7), *ax.p(-1.5, 7))
        s.linie(*ax.p(1.5, -3.5), *ax.p(1.5, 3.5))
        q = ax.p(-5, -8)
        s.text(q[0] - 2, q[1] + 2, "+", 5)
    else:   # Diode, LED: Dreieck in Durchlassrichtung p -> n, Strich an der Kathode
        s.zug([ax.p(-L, -4.5), ax.p(-L, 4.5), ax.p(L, 0), ax.p(-L, -4.5)])
        s.linie(*ax.p(L, -4.5), *ax.p(L, 4.5))
        if teil.startswith("LED"):
            for u in (-1.5, 1.5):
                a, b = ax.p(u, -6), ax.p(u + 4, -11)
                s.linie(*a, *b, ende="arrow1")
    return True


def _kreisquelle(s, anschl, u, objekt):
    """Signalgenerator bzw. Oszilloskop: Kreis mit Sinus (IEC 60617), Zuleitungen bis an den Kreis."""
    el = [a for a in anschl if a[1].get("type") == "electric"]
    if not el:
        return False
    x0, y0, x1, y1 = u
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    r = max(min(x1 - x0, y1 - y0) / 2 * 0.55, 6)
    s.kreis(cx, cy, r)
    s.zug([(cx - r * 0.6 + r * 1.2 * i / 12, cy - r * 0.35 * math.sin(i / 12 * 2 * 3.14159))
           for i in range(13)])
    for _, _, ax_, ay in el:
        dx, dy = ax_ - cx, ay - cy
        lang = (dx * dx + dy * dy) ** 0.5 or 1.0
        if lang > r:
            s.linie(ax_, ay, cx + dx / lang * r, cy + dy / lang * r)
    return True


def _transistor(s, anschl, pnp):
    """Bipolartransistor (IEC 60617): Basisbalken, Kollektor und Emitter schräg, Pfeil am Emitter (NPN
    heraus, PNP hinein), Kreis; Anschlüsse B, C, E."""
    el = {a[1].get("designation"): a for a in anschl if a[1].get("type") == "electric"}
    if not all(k in el for k in ("B", "C", "E")):
        return False
    b, c, e = el["B"], el["C"], el["E"]
    bx = b[2] + (c[2] - b[2]) * 0.55
    cy, ey = c[3], e[3]
    my = (cy + ey) / 2
    h = abs(ey - cy) * 0.22
    s.linie(b[2], b[3], bx, b[3])
    s.linie(bx, my - h, bx, my + h)
    kx = c[2]
    s.linie(bx, my - h * 0.5, kx, my - h * 1.6)
    s.linie(kx, my - h * 1.6, kx, cy)
    if pnp:
        s.linie(kx, my + h * 1.6, bx, my + h * 0.5, ende="arrow1")
    else:
        s.linie(bx, my + h * 0.5, kx, my + h * 1.6, ende="arrow1")
    s.linie(kx, my + h * 1.6, kx, ey)
    r = max(abs(kx - bx), h * 1.8) * 0.95
    s.kreis((bx + kx) / 2, my, r)
    return True


class Achse:
    """Örtliche Koordinaten längs der Verbindung zweier Anschlüsse: u längs (von a nach b), v quer."""

    def __init__(self, a, b):
        self.m = ((a[0] + b[0]) / 2, (a[1] + b[1]) / 2)
        dx, dy = b[0] - a[0], b[1] - a[1]
        lang = (dx * dx + dy * dy) ** 0.5 or 1.0
        self.d = (dx / lang, dy / lang)
        self.n = (-self.d[1], self.d[0])
        self.halb = lang / 2

    def p(self, u, v=0.0):
        return (self.m[0] + u * self.d[0] + v * self.n[0], self.m[1] + u * self.d[1] + v * self.n[1])


def _rueckschlag(s, ax, u0, v=0.0):
    """Kugel mit Sitz; frei in +u-Richtung (von der Sitzspitze zur Kugel)."""
    k = ax.p(u0 + 2, v)
    s.kreis(k[0], k[1], 3)
    s.zug([ax.p(u0 - 2, v - 4), ax.p(u0 - 5, v), ax.p(u0 - 2, v + 4)])


def _drossel(s, ax, u0=0.0, v=0.0):
    s.zug([ax.p(u0 - 5, v - 3), ax.p(u0, v - 1), ax.p(u0 + 5, v - 3)])
    s.zug([ax.p(u0 - 5, v + 3), ax.p(u0, v + 1), ax.p(u0 + 5, v + 3)])


def _inline(s, anschl, objekt):
    """Bauteile zwischen zwei Anschlüssen: Rückschlag-, Drossel-, Drosselrückschlag-, Absperrventil, Filter."""
    if len(anschl) != 2 and not ("PilotToOpen" in objekt and len(anschl) == 3):
        return False
    nach_ident = {a[1].get("ident"): a for a in anschl}
    if "PilotToOpen" in objekt:
        a, b = nach_ident.get("1"), nach_ident.get("3")       # A -> B frei, X steuert
    else:
        a, b = nach_ident.get("2"), nach_ident.get("1")       # frei von Anschluss 2 nach 1
    if a is None or b is None:
        a, b = anschl[0], anschl[1]
    ax = Achse((a[2], a[3]), (b[2], b[3]))
    r = min(14.0, ax.halb - 1)
    s.linie(a[2], a[3], *ax.p(-r))
    s.linie(b[2], b[3], *ax.p(r))
    if "ThrottleCheck" in objekt:
        s.linie(*ax.p(-r), *ax.p(-6))
        s.linie(*ax.p(6), *ax.p(r))
        _drossel(s, ax)
        s.zug([ax.p(-r + 2), ax.p(-r + 2, 9), ax.p(-6, 9)])
        s.zug([ax.p(6, 9), ax.p(r - 2, 9), ax.p(r - 2)])
        _rueckschlag(s, ax, 0, 9)
        s.linie(*ax.p(-6, -6), *ax.p(6, 6), ende="arrow1")
    elif "CheckValve" in objekt:
        s.linie(*ax.p(-r), *ax.p(-5))
        _rueckschlag(s, ax, 0)
        s.linie(*ax.p(5), *ax.p(r))
        if "PilotToOpen" in objekt:
            x = next((c for c in anschl if c[1].get("ident") == "2"), None)
            if x is not None:
                s.linie(x[2], x[3], *ax.p(0, 8), strich="dash")
    elif "ShutoffValve" in objekt:
        s.zug([ax.p(-r), ax.p(-r, -4), ax.p(r, 4), ax.p(r), ax.p(r, -4), ax.p(-r, 4), ax.p(-r)])
    elif "Filter" in objekt:
        s.zug([ax.p(-r), ax.p(0, -r * 0.8), ax.p(r), ax.p(0, r * 0.8), ax.p(-r)])
        s.linie(*ax.p(0, -r * 0.8), *ax.p(0, r * 0.8), strich="dash")
    else:                           # Drossel / Stromregelventil
        s.linie(*ax.p(-r), *ax.p(-5))
        _drossel(s, ax)
        s.linie(*ax.p(5), *ax.p(r))
        s.linie(*ax.p(-6, -6), *ax.p(6, 6), ende="arrow1")
    return True


def _druckventil(s, anschl, regler):
    """Druckbegrenzungs- bzw. Druckregelventil: Kasten mit Pfeil und Feder, Anschlüsse ein/aus."""
    if len(anschl) < 2:
        return False
    xs, ys = [a[2] for a in anschl], [a[3] for a in anschl]
    cx, cy = (min(xs) + max(xs)) / 2, (min(ys) + max(ys)) / 2
    s.kasten(cx - 9, cy - 9, 18, 18)
    if regler:
        s.linie(cx - 3, cy + 9, cx - 3, cy - 9, ende="arrow1")
    else:
        s.linie(cx - 3, cy + 7, cx + 3, cy - 7, ende="arrow1")
    s.feder(cx + 9, cy, cx + 19, 3)
    s.linie(cx - 9, cy + 6, cx - 15, cy + 6, strich="dash")
    for _, _, ax_, ay in anschl:
        if abs(ay - cy) >= abs(ax_ - cx):
            s.linie(ax_, ay, ax_, cy - 9 if ay < cy else cy + 9)
        else:
            s.linie(ax_, ay, cx - 9 if ax_ < cx else cx + 9, ay)
    return True


def _logikventil(s, anschl, und):
    """Wechsel- (ODER) bzw. Zweidruckventil (UND): Kasten zwischen den Eingängen, Ausgang in der Mitte."""
    if len(anschl) != 3:
        return False
    aus = next((a for a in anschl if (a[1].get("designation") or "") == "2"), None) or anschl[-1]
    ein = [a for a in anschl if a is not aus]
    x0, x1 = min(a[2] for a in ein), max(a[2] for a in ein)
    y = sum(a[3] for a in ein) / 2
    s.kasten(x0 + 6, y - 6, x1 - x0 - 12, 12)
    m = (x0 + x1) / 2
    s.kreis(m, y, 3, gefuellt=und)
    s.linie(x0, y, x0 + 6, y)
    s.linie(x1 - 6, y, x1, y)
    for a in ein:
        if abs(a[3] - y) > 0.5:
            s.linie(a[2], a[3], a[2], y)
    s.zug([(aus[2], aus[3]), (aus[2], y - 6 if aus[3] < y else y + 6)])
    return True


def kurzname(objekt):
    teil = (objekt.split(".")[-1] if objekt else "") or "?"
    return re.sub(r"(?<=[a-z])(?=[A-Z])", " ", teil)


# ---------------------------------------------------------------------------------------------- Einstieg
def ersetzen(symbol, komponente, klasse="", stellungen=None):
    """Neues <symbol>: Ebenen mit nur ihren Anschlüssen, dazu die eigene Grafik in der Ebene EIGEN."""
    objekt = komponente.get("object-ref") or ""
    neu = ET.Element("symbol", dict(symbol.attrib))
    for eb in symbol.findall("symbol-layer"):
        kopie = ET.SubElement(neu, "symbol-layer", dict(eb.attrib))
        for c in eb.iter("connector"):
            kopie.append(c)
    anschl = anschluesse(symbol)
    u = umriss(symbol)
    s = Stift()
    gezeichnet = False
    if "DirectionalValve" in objekt or "Valve.Directional" in objekt:
        gezeichnet = _ventil(s, symbol, komponente, stellungen)
    elif "Actuator" in objekt and "CYL" in objekt.upper():
        fluid = [a for a in anschl if a[1].get("type") != "electric"]
        _zylinder(s, fluid, u, len(fluid) == 1)
        gezeichnet = True
    elif objekt == "Pneu.Supply.AirSupply" or objekt.endswith(("SimpleFlowSource", "PumpBlock01")):
        _quelle(s, anschl, u, objekt.startswith("Hyd."))
        gezeichnet = True
    elif objekt.endswith("Supply.Tank"):
        _tank(s, anschl, u)
        gezeichnet = True
    elif ".Sensor.Manometer" in objekt or objekt.endswith(("Voltmeter", "AmpereMeter")):
        _messgeraet(s, anschl, u)
        gezeichnet = True
    elif objekt.startswith(("ET.Source.Pot", "ET.Source.Ground")):
        gezeichnet = _potential(s, anschl, "0V" if ("0V" in objekt or "Ground" in objekt) else "24V")
    elif klasse in ("make-contact", "break-contact", "changeover-contact", "contact-manual", "proximity-switch"):
        art = "break" if ("Break" in objekt or klasse == "break-contact") else "make"
        gezeichnet = _kontakt(s, anschl, art, klasse == "contact-manual")
    elif klasse in ("solenoid-coil", "solenoid-valve-coil", "solenoid-coil-clpc"):
        gezeichnet = _spule(s, anschl, klasse != "solenoid-coil")
    elif klasse == "indicator-light":
        gezeichnet = _lampe(s, anschl)
    elif re.search(r"CheckValve|ThrottleValve|FlowControlValve2Way|FlowValve\.ShutoffValve|Supply\.Filter", objekt):
        gezeichnet = _inline(s, anschl, objekt)
    elif re.search(r"PressureReliefValve|PressureReducingValve|PressureRegulator", objekt):
        gezeichnet = _druckventil(s, anschl, "Relief" not in objekt)
    elif "ShuttleValve" in objekt or "TwoPressureValve" in objekt:
        gezeichnet = _logikventil(s, anschl, "TwoPressure" in objekt)
    elif objekt.startswith("ET.") and objekt.split(".")[-1].startswith(ZWEIPOL):
        gezeichnet = _zweipol(s, anschl, objekt)
    elif objekt.startswith("ET.Semiconductor.") and objekt.split(".")[-1].startswith(("NPN", "PNP")):
        gezeichnet = _transistor(s, anschl, objekt.split(".")[-1].startswith("PNP"))
    elif objekt.startswith("ET.") and objekt.split(".")[-1].startswith(KREISQUELLE):
        gezeichnet = _kreisquelle(s, anschl, u, objekt)
    if not gezeichnet:
        _kasten_mit_name(s, anschl, u, kurzname(objekt))
    neu.append(s.ebene)
    return neu
