"""QET-Standardsymbole für Hydraulik und Pneumatik (elements/30_hydraulic, elements/50_pneumatic) um
Simulationsdaten erweitern. Die Grafik bleibt unverändert.

Je Symbol:
- Anschlussnamen: aus der Beschriftung neben dem Anschluss (1, 2, 12, P, A …), sonst nach Norm aus der Lage;
- <fluid kind=… function=…> mit Medium, Kennbuchstaben und Kennwerten;
- Wegeventile: Stellungen und Schaltwege aus der Zeichnung gelesen (Pfeile, Sperrzeichen, Verbindungslinien
  in jedem Kästchen), Ruhestellung = Kästchen mit den Anschlüssen, Betätigung links/rechts (Magnet, Feder,
  Vorsteuerung, Hand);
- Zylinder: Wirkungsweise, Feder, Kolben-/Stangendurchmesser und Hub als Startwerte.

Quelle ist die unveränderte Standardsammlung (Submodul elements/), Ziel bauteile/fluid mit denselben Pfaden;
Build und Deploy legen bauteile/fluid über die Standardsammlung.

    python tools/symbole/qet_fluid_daten.py                 # schreibt nach bauteile/fluid/
    python tools/symbole/qet_fluid_daten.py --nur-zeigen --bericht bericht.csv
"""
import argparse
import csv
import math
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
BASIS = [REPO / "elements/30_hydraulic", REPO / "elements/50_pneumatic"]

# Reihenfolge für ungerichtete Verbindungen: Versorgung -> Verbraucher -> Ablass
RANG = {"P": 0, "1": 0, "X": 1, "Y": 1, "A": 2, "B": 2, "2": 2, "4": 2, "C": 2,
        "T": 3, "R": 3, "S": 3, "3": 3, "5": 3, "L": 4}
WORT = {2: "two", 3: "three", 4: "four", 5: "five", 6: "six", 8: "eight"}

# --------------------------------------------------------------------------- Kennwerte (Startwerte)
# Dieselben Kennwertnamen wie die Simulationsbibliothek von FluidSIM, Werte = übliche Vorgaben.
VENTIL = [("qNN", "60", "l/min"), ("onDelay", "0.02", "s"), ("offDelay", "0.03", "s"),
          ("F_spring", "20", "N"), ("F_EL", "70", "N"), ("F_ER", "70", "N"),
          ("areaPilotL", "1.4", "cm2"), ("areaPilotR", "1.4", "cm2")]
VENTIL_HYD = [("rT", "0.1", "bar.min2/l2"), ("onDelay", "0.02", "s"), ("offDelay", "0.03", "s"),
              ("F_spring", "20", "N"), ("F_EL", "70", "N"), ("F_ER", "70", "N")]


def zylinder_kw(medium, action, feder):
    d, s, h = ("20", "8", "100") if medium == "pneumatic" else ("40", "20", "200")
    kw = [("d_piston", d, "mm"), ("d_rod", s, "mm"), ("xMax", h, "mm"), ("xStart", "0", "mm"),
          ("F_extern_const", "0", "N"), ("m_ext", "0", "kg"), ("len_damp", "10", "mm")]
    if feder:
        kw += [("p_spring_min", "0.5", "bar"), ("p_spring_max", "1", "bar")]
    return kw


# --------------------------------------------------------------------------- Geometrie
def fz(e, k, d=0.0):
    v = e.get(k)
    return float(v) if v not in (None, "") else d


def polypunkte(e):
    p, i = [], 1
    while e.get(f"x{i}") is not None:
        p.append((fz(e, f"x{i}"), fz(e, f"y{i}")))
        i += 1
    return p


def strecken(desc):
    """Alle Linienstücke (x1, y1, x2, y2, quelle) – Linien und Polygonkanten."""
    out = []
    for e in desc:
        if e.tag == "line":
            out.append((fz(e, "x1"), fz(e, "y1"), fz(e, "x2"), fz(e, "y2"), e))
        elif e.tag == "polygon":
            p = polypunkte(e)
            for a, b in zip(p, p[1:]):
                out.append((a[0], a[1], b[0], b[1], e))
            if e.get("closed", "true") != "false" and len(p) > 2:
                out.append((p[-1][0], p[-1][1], p[0][0], p[0][1], e))
    return out


def rechtecke(desc):
    """Achsparallele Rechtecke: <rect> und rechteckige Polygone -> (x0, y0, x1, y1)."""
    out = []
    for e in desc:
        if e.tag == "rect":
            x, y, w, h = fz(e, "x"), fz(e, "y"), fz(e, "width"), fz(e, "height")
            out.append((min(x, x + w), min(y, y + h), max(x, x + w), max(y, y + h)))
        elif e.tag == "polygon":
            p = polypunkte(e)
            if len(p) < 4:
                continue
            xs = [x for x, _ in p]
            ys = [y for _, y in p]
            ax, bx, ay, by = min(xs), max(xs), min(ys), max(ys)
            if all(min(abs(x - ax), abs(x - bx)) < 0.6 for x in xs) \
                    and all(min(abs(y - ay), abs(y - by)) < 0.6 for y in ys) and bx - ax > 6 and by - ay > 6:
                out.append((ax, ay, bx, by))
    return out


def ist_pfeilspitze(e):
    if e.tag != "polygon" or "filling:black" not in (e.get("style") or ""):
        return False
    p = polypunkte(e)
    if not 3 <= len(p) <= 5:
        return False
    xs, ys = [x for x, _ in p], [y for _, y in p]
    return max(xs) - min(xs) < 9 and max(ys) - min(ys) < 9


# --------------------------------------------------------------------------- Anschlussnamen
def kurztexte(desc):
    out = []
    for e in desc:
        if e.tag == "dynamic_text" and e.get("text_from") == "UserText":
            t = (e.findtext("text") or "").strip()
        elif e.tag == "text":
            t = (e.get("text") or "").strip()
        else:
            continue
        if t and len(t) <= 4 and re.fullmatch(r"[0-9A-Za-z./]+", t):
            out.append([t.upper(), fz(e, "x"), fz(e, "y")])
    return out


STEUER = {"10", "12", "14", "81", "82", "84", "91", "X", "Y", "Z", "X1", "X2", "Y1", "Y2"}


def namen_aus_texten(terms, desc):
    """Beschriftung den Anschlüssen zuordnen. In der QET-Sammlung steht die Nummer eines senkrechten Anschlusses
    links neben seiner Leitung (Textanker ≈ x − 7); Steueranschlüsse (10, 12, 14, X, Y …) gehören nur an
    seitliche Anschlüsse, alle anderen nur an obere/untere."""
    texte = kurztexte(desc)
    passt = lambda t, s: (s in STEUER) == (t["o"] in ("e", "w"))
    paare = []
    for i, t in enumerate(terms):
        for j, (s, x, y) in enumerate(texte):
            if not passt(t, s):
                continue
            if t["o"] in ("n", "s") and abs(x + 7 - t["x"]) < 4 and abs(y - t["y"]) < 16:
                paare.append((abs(x + 7 - t["x"]) * 3 + abs(y - t["y"]) * 0.3, i, j))
            else:
                d = math.hypot(x + 2 - t["x"], y + 3 - t["y"])
                if d < 16:
                    paare.append((d + 50, i, j))
    paare.sort()
    vergeben_t, vergeben_s = set(), set()
    for d, i, j in paare:
        if i in vergeben_t or j in vergeben_s:
            continue
        terms[i]["name"] = texte[j][0]
        vergeben_t.add(i)
        vergeben_s.add(j)


# --------------------------------------------------------------------------- Wegeventile
def ventil_zeile(desc, terms):
    """Kästchenreihe des Ventils: Liste (x0, y0, x1, y1) von links nach rechts."""
    rs = rechtecke(desc)
    senk = [t for t in terms if t["o"] in ("n", "s")]
    linien = [s for s in strecken(desc) if abs(s[0] - s[2]) < 0.3]
    zellen = []
    for (x0, y0, x1, y1) in rs:
        if not 10 <= y1 - y0 <= 40:
            continue
        # Trennlinie: senkrecht über die volle Kästchenhöhe, aber nicht darüber hinaus (sonst Anschlussleitung)
        teiler = sorted({round(s[0], 1) for s in linien
                         if x0 + 2 < s[0] < x1 - 2 and abs(min(s[1], s[3]) - y0) <= 0.8
                         and abs(max(s[1], s[3]) - y1) <= 0.8})
        kanten = [x0] + teiler + [x1]
        for a, b in zip(kanten, kanten[1:]):
            zellen.append((round(a, 1), round(y0, 1), round(b, 1), round(y1, 1)))
    zellen = sorted(set(zellen))
    beste = []
    for z in zellen:
        reihe = sorted(w for w in zellen if abs(w[1] - z[1]) < 1 and abs(w[3] - z[3]) < 1
                       and 0.6 < (w[2] - w[0]) / (z[2] - z[0]) < 1.6)
        ketten, kette = [], []
        for w in reihe:
            if kette and abs(w[0] - kette[-1][2]) < 1.2:
                kette.append(w)
            else:
                if kette:
                    ketten.append(kette)
                kette = [w]
        if kette:
            ketten.append(kette)
        for k in ketten:
            treffer = sum(1 for t in senk if k[0][0] - 0.5 <= t["x"] <= k[-1][2] + 0.5)
            if treffer and len(k) > len(beste):
                beste = k
    return beste


def clip(ax, ay, bx, by, x0, y0, x1, y1):
    """Strecke auf ein Rechteck kürzen (Liang-Barsky); None, wenn außerhalb."""
    dx, dy = bx - ax, by - ay
    u0, u1 = 0.0, 1.0
    for p, q in ((-dx, ax - x0), (dx, x1 - ax), (-dy, ay - y0), (dy, y1 - ay)):
        if abs(p) < 1e-9:
            if q < 0:
                return None
            continue
        r = q / p
        if p < 0:
            u0 = max(u0, r)
        else:
            u1 = min(u1, r)
        if u0 > u1:
            return None
    return ax + u0 * dx, ay + u0 * dy, ax + u1 * dx, ay + u1 * dy


def zelle_wege(desc, zelle, ports, alle):
    x0, y0, x1, y1 = zelle
    eps = 1.6

    def innen(x, y, rand=eps):
        return x0 - rand <= x <= x1 + rand and y0 - rand <= y <= y1 + rand

    def port_an(x, y):
        if abs(y - y0) < 4:
            rand = "n"
        elif abs(y - y1) < 4:
            rand = "s"
        else:
            return None
        off = x - x0
        kand = [p for p in ports if p["o"] == rand and abs(p["off"] - off) < 3.6]
        return min(kand, key=lambda p: abs(p["off"] - off))["name"] if kand else None

    spitzen = [e for e in desc if ist_pfeilspitze(e) and all(innen(x, y, 2.5) for x, y in polypunkte(e))]
    quer = [s for s in alle if abs(s[1] - s[3]) < 0.3 and 0.5 < abs(s[2] - s[0]) <= 7
            and innen(s[0], s[1], 0) and innen(s[2], s[3], 0) and s[4] not in spitzen]
    sperren = set()
    stummel = set()
    for (ax, ay, bx, by, e) in quer:
        mx, my = (ax + bx) / 2, ay
        for s in alle:
            cx, cy, dx, dy, f = s
            if abs(cx - dx) < 0.3 and abs(cx - mx) < 1.2 and (abs(cy - my) < 1.2 or abs(dy - my) < 1.2):
                andere = dy if abs(cy - my) < 1.2 else cy
                rand = y0 if andere < my else y1
                p = port_an(mx, rand)
                if p:
                    sperren.add(p)
                    stummel.add(id(s))
    weg = []
    for s in alle:
        ax, ay, bx, by, e = s
        if e in spitzen or id(s) in stummel or s in quer:
            continue
        if math.hypot(bx - ax, by - ay) < 0.5:
            continue
        if (abs(ay - by) < 0.3 and (abs(ay - y0) < 0.6 or abs(ay - y1) < 0.6)) or \
                (abs(ax - bx) < 0.3 and (abs(ax - x0) < 0.6 or abs(ax - x1) < 0.6)):
            continue
        if not (innen(ax, ay) and innen(bx, by)):
            # durchgehende Anschlussleitung: auf das Kästchen kürzen
            k = clip(ax, ay, bx, by, x0, y0, x1, y1)
            if not k or math.hypot(k[2] - k[0], k[3] - k[1]) < 2:
                continue
            ax, ay, bx, by = k
        weg.append([ax, ay, bx, by, e])
    pfeile = set()
    for t in weg:
        ax, ay, bx, by, e = t
        for s in spitzen:
            p = polypunkte(s)
            cx, cy = sum(x for x, _ in p) / len(p), sum(y for _, y in p) / len(p)
            da, db = math.hypot(cx - ax, cy - ay), math.hypot(cx - bx, cy - by)
            if min(da, db) < 6:
                fern = (bx, by) if da < db else (ax, ay)
                spitze = max(p, key=lambda q: math.hypot(q[0] - fern[0], q[1] - fern[1]))
                if da < db:
                    t[0], t[1], t[2], t[3] = bx, by, spitze[0], spitze[1]
                else:
                    t[2], t[3] = spitze
                pfeile.add(id(t))
                break
    eltern = {id(t): id(t) for t in weg}

    def wurzel(a):
        while eltern[a] != a:
            eltern[a] = eltern[eltern[a]]
            a = eltern[a]
        return a

    pkt = []
    for t in weg:
        pkt += [(t[0], t[1], id(t)), (t[2], t[3], id(t))]
    for i, (xa, ya, ta) in enumerate(pkt):
        for (xb, yb, tb) in pkt[i + 1:]:
            if ta != tb and math.hypot(xa - xb, ya - yb) < 1.6:
                eltern[wurzel(ta)] = wurzel(tb)
        for t in weg:
            if id(t) == ta:
                continue
            ax, ay, bx, by = t[:4]
            lg = math.hypot(bx - ax, by - ay)
            if lg < 1:
                continue
            u = ((xa - ax) * (bx - ax) + (ya - ay) * (by - ay)) / lg ** 2
            if 0 < u < 1 and abs((bx - ax) * (ay - ya) - (ax - xa) * (by - ay)) / lg < 1.2:
                eltern[wurzel(ta)] = wurzel(id(t))
    gruppen = {}
    for t in weg:
        gruppen.setdefault(wurzel(id(t)), []).append(t)
    wege = []
    for teile in gruppen.values():
        enden = []
        for t in teile:
            for (x, y, kopf) in ((t[0], t[1], False), (t[2], t[3], id(t) in pfeile)):
                p = port_an(x, y)
                if p:
                    enden.append((p, kopf))
        namen = sorted({p for p, _ in enden}, key=lambda n: (RANG.get(n, 2), n))
        if len(namen) < 2:
            continue
        ziele = list(dict.fromkeys(p for p, k in enden if k))
        quellen = [n for n in namen if n not in ziele]
        if ziele and quellen:
            for z in ziele:
                for q in quellen:
                    wege.append((q, z))
        else:
            for n in namen[1:]:
                wege.append((namen[0], n))
    verbunden = {p for w in wege for p in w}
    gesperrt = [p["name"] for p in ports if p["name"] not in verbunden]
    return wege, gesperrt


def betaetigung(desc, zeile, seite, name_en):
    """Typen der Betätigung links/rechts neben der Kästchenreihe."""
    x0, x1 = zeile[0][0], zeile[-1][2]
    y0, y1 = zeile[0][1], zeile[0][3]
    aussen = (lambda x: x <= x0 + 0.5) if seite == "left" else (lambda x: x >= x1 - 0.5)
    weit = (lambda x: x < x0 - 1) if seite == "left" else (lambda x: x > x1 + 1)
    els = []
    for e in desc:
        if e.tag == "line":
            pts = [(fz(e, "x1"), fz(e, "y1")), (fz(e, "x2"), fz(e, "y2"))]
        elif e.tag == "polygon":
            pts = polypunkte(e)
        elif e.tag in ("rect", "ellipse", "circle", "arc"):
            pts = [(fz(e, "x"), fz(e, "y"))]
        else:
            continue
        if pts and all(aussen(x) for x, _ in pts) and any(weit(x) for x, _ in pts)                 and all(y0 - 15 <= y <= y1 + 15 for _, y in pts):
            els.append((e, pts))
    feder = any(e.tag == "polygon" and e.get("closed") == "false" and len(pts) >= 5
                and sum(1 for a, b in zip(pts, pts[1:]) if abs(a[1] - b[1]) > 2) >= 4 for e, pts in els)
    schraeg = [pts for e, pts in els if e.tag == "line"
               and abs(pts[0][0] - pts[1][0]) > 1.5 and abs(pts[0][1] - pts[1][1]) > 1.5]
    if len(schraeg) >= 3:
        feder = True
    kaesten = [e for e, pts in els if e.tag == "rect" or (e.tag == "polygon" and len(pts) >= 4
                                                          and e.get("closed") != "false")]
    dreiecke = [e for e, pts in els if e.tag == "polygon" and len(pts) == 3]
    typen = []
    if kaesten and (schraeg or any(e.tag == "line" for e, _ in els)):
        typen.append("solenoid")
    if dreiecke:
        typen.append("pilot")
    if any(e.tag in ("ellipse", "circle", "arc") for e, _ in els):
        typen.append("roller" if ("roll" in name_en or "mech" in name_en) else "manual")
    elif els and not typen and not feder:
        typen.append("manual")
    return typen, feder


def ventil_daten(desc, terms, name_en, medium):
    zeile = ventil_zeile(desc, terms)
    if len(zeile) < 2:
        return None, "keine Kästchenreihe erkannt"
    ruhe = next((i for i, z in enumerate(zeile)
                 if any(z[0] - 0.5 <= t["x"] <= z[2] + 0.5 for t in terms if t["o"] in ("n", "s"))), None)
    if ruhe is None:
        return None, "Ruhekästchen nicht gefunden"
    rz = zeile[ruhe]
    if any(not rz[0] - 0.5 <= t["x"] <= rz[2] + 0.5 for t in terms if t["o"] in ("n", "s")):
        return None, "Anschlüsse an mehreren Kästchen gezeichnet (Ruhestellung unklar)"
    ports = [{"name": t["name"], "o": t["o"], "off": t["x"] - rz[0]} for t in terms if t["o"] in ("n", "s")]
    if len({p["name"] for p in ports}) != len(ports):
        return None, "Anschlussnamen nicht eindeutig"
    st = strecken(desc)
    stellungen = [zelle_wege(desc, z, ports, st) for z in zeile]
    if not any(w for w, _ in stellungen):
        return None, "keine Schaltwege erkannt"
    xml = [f'<positions initial="{ruhe + 1}">']
    for wege, gesperrt in stellungen:
        xml.append("    <position>" + "".join(f'<path from="{a}" to="{b}"/>' for a, b in wege)
                   + "".join(f'<path from="{g}"/>' for g in gesperrt) + "</position>")
    xml.append("</positions>")
    # Die Pneumatik-Ventile der Sammlung nennen ihre Betätigung im Namen: "5/2e [elec_pneu]-[spring]"
    m = re.search(r"\[([^\]]*)\]-\[([^\]]*)\]", name_en)
    for s, kennung in zip(("left", "right"), (m.groups() if m else (None, None))):
        if kennung is not None:
            teile = set(kennung.split("_"))
            typen = [t for k, t in (("elec", "solenoid"), ("man", "manual"), ("mech", "roller")) if k in teile]
            if "pneu" in teile:
                typen.append("pilot")
            feder = bool(teile & {"spring", "airspring"})
        else:
            typen, feder = betaetigung(desc, zeile, s, name_en)
        if typen or feder:
            pilot = ' pilot="true"' if "pilot" in typen else ""
            xml.append(f'<actuation side="{s}" types="{",".join(t for t in typen if t != "pilot")}" '
                       f'spring="{"true" if feder else "false"}"{pilot}/>')
    n = len(ports)
    kw = list(VENTIL if medium == "pneumatic" else VENTIL_HYD)
    kw += [("posNoMax", str(len(zeile)), ""), ("posNoInit", str(ruhe + 1), "")]
    info = " | ".join(f"{i + 1}{'R' if i == ruhe else ''}: " + ",".join(f"{a}>{b}" for a, b in w)
                      + (" /" + ",".join(g) if g else "") for i, (w, g) in enumerate(stellungen))
    return {"kind": "valve", "prefix": "QM", "function": f"directional_valve.{WORT.get(n, str(n))}_pins",
            "inner": xml, "kw": kw, "info": info}, ""


# --------------------------------------------------------------------------- Zuordnung der Bauart
def bauart(rel, name_de, name_en):
    r = rel.lower()
    n = (name_de + " " + name_en).lower()
    datei = r.rsplit("/", 1)[-1]
    regeln = [
        ("teil", "single_parts" in r),
        ("ventil", "/502010_control/" in r or "/31_control_valves/" in r or "/502030_prop/" in r),
        ("motor", "cylinder_motor" in r or "moteur" in r),
        ("greifer", "gripper" in r),
        ("schwenk", "cylinder_rot" in r),
        ("zylinder", "cylinder" in r or "verin" in r),
        ("pumpe", "pomp" in r or "air_generation" in r or "vacuum_pump" in r or "source_energie" in r),
        ("speicher", "bache_pressurisee" in r or "storage" in r or "speicher" in n),
        ("tank", "bache" in r or "tank" in r),
        ("druckminderer", "pressure_regulator" in r),
        ("schalldaempfer", "silencer" in r or "sil_vacuum" in r),
        ("schnellentlueftung", "purge_rapide" in r),
        ("stromteiler", "diviseur" in r or "flow_divider" in r),
        ("wechselventil", "selector" in r or "selecteur" in r),
        ("zweidruckventil", "double_valve" in r or "soupape_double" in r),
        ("zeitglied", "timer" in r or "idozito" in r),
        ("dbv", "soupape_tare" in r or "relief" in r),
        ("rueckschlag", "clapet" in r),
        ("drossel", "reducteur_debit" in r or "picv" in r),
        ("kupplung", "quick_release" in r),
        ("absperr", "vanne" in r),
        ("leitung", "pipes_connections" in r),
        ("wartungseinheit", "combined_unit" in r),
        ("filter", "filtre" in r or ("treatment" in r and "filter" in n)),
        ("trockner", "trockner" in n),
        ("wasserabscheider", "wasserabscheid" in n),
        ("oeler", "öler" in n),
        ("kuehler", "exchang" in r or "kühl" in n),
        ("heizer", "erhitz" in n),
        ("temperaturregler", "temperaturregel" in n),
        ("druckschalter", "pressure_switch" in r or "pressostat" in r or datei == "50402020_pressure_sensor.elmt"),
        ("drucksensor", "pressure_sensor" in r),
        ("vakuumerzeuger", "ejecteur" in r),
        ("sauger", "ventouse" in r),
        ("messgeraet", "indicator" in r or "indicateur" in r),
    ]
    return next((art for art, ja in regeln if ja), "sonstig")


FUNKTION = {   # Bauart -> (kind, Kennbuchstaben, function)
    "pumpe": ("source", "GP", "supply.pump"), "motor": ("other", "MA", "motor.fixed_motor"),
    "speicher": ("source", "CM", "supply.accumulator"), "tank": ("source", "CM", "supply.tank"),
    "druckminderer": ("treatment", "RN", "pressure_valve.pressure_regulator"),
    "schalldaempfer": ("treatment", "RZ", "supply.silencer"),
    "schnellentlueftung": ("flow_control", "RM", "shutoff_valve.quick_exhaust_valve"),
    "stromteiler": ("flow_control", "RZ", "flow_control_valve.flow_divider"),
    "wechselventil": ("logic_valve", "KH", "logic.shuttle_valve"),
    "zweidruckventil": ("logic_valve", "KH", "logic.dual_pressure_valve"),
    "zeitglied": ("logic_valve", "KH", "logic.time_delay_valve"),
    "dbv": ("flow_control", "RN", "pressure_valve.pressure_relief_valve01"),
    "rueckschlag": ("flow_control", "RM", "shutoff_valve.check_valve"),
    "drossel": ("flow_control", "RZ", "flow_control_valve.throttle_valve"),
    "kupplung": ("other", "XM", "connection.quick_coupling"),
    "absperr": ("flow_control", "QM", "shutoff_valve.shutoff_valve"),
    "leitung": ("other", "", "connection.line"),
    "wartungseinheit": ("treatment", "HQ", "supply.service_unit"),
    "filter": ("treatment", "HQ", "supply.filter"), "trockner": ("treatment", "HQ", "supply.dryer"),
    "wasserabscheider": ("treatment", "HQ", "supply.water_separator"),
    "oeler": ("treatment", "HQ", "supply.lubricator"), "kuehler": ("treatment", "EQ", "supply.cooler"),
    "heizer": ("treatment", "EQ", "supply.heater"), "temperaturregler": ("treatment", "EQ", "supply.temperature_control"),
    "druckschalter": ("sensor", "BP", "sensor.pressure_switch"),
    "drucksensor": ("sensor", "BP", "sensor.pressure_sensor"),
    "vakuumerzeuger": ("source", "GP", "vacuum.vacuum_generator"), "sauger": ("other", "MM", "vacuum.suction_cup"),
    "messgeraet": ("sensor", "PG", "measure.indicator"),
    "greifer": ("cylinder", "MM", "actuator.gripper"), "schwenk": ("cylinder", "MM", "actuator.semi_rotary"),
    "sonstig": ("other", "", "other.misc"), "teil": ("other", "", "directional_valve.part"),
}
KW = {
    "pumpe": [("displacement", "8", "cm3"), ("rpm", "1450", "1/min"), ("pMax", "6", "MPa")],
    "motor": [("displacement", "8", "cm3"), ("M_load", "0", "Nm")],
    "speicher": [("V0", "1", "l"), ("p0", "0.4", "MPa"), ("k", "1.4", "")],
    "tank": [("V0", "10", "l")],
    "druckminderer": [("p0", "0.4", "MPa"), ("dpEnd", "0.05", "MPa")],
    "dbv": [("p0", "0.6", "MPa"), ("dpEnd", "0.1", "MPa")],
    "rueckschlag": [("p0", "0", "MPa")],
    "drossel": [("rT", "1", "bar.min2/l2")],
    "absperr": [("rT", "0.1", "bar.min2/l2")],
    "schnellentlueftung": [("qNN", "300", "l/min")],
    "schalldaempfer": [("qNN", "300", "l/min")],
    "zeitglied": [("t_delay", "1", "s")],
    "druckschalter": [("p0", "0.4", "MPa"), ("p_hyst", "0.05", "MPa")],
    "drucksensor": [("p_min", "0", "MPa"), ("p_max", "1", "MPa")],
    "vakuumerzeuger": [("p_vac", "-0.08", "MPa")],
}


def standardnamen(terms, art, medium):
    """Anschlüsse ohne Beschriftung nach Lage benennen; doppelte Namen eindeutig machen."""
    frei = [t for t in terms if not t["name"]]
    pneu = medium == "pneumatic"
    if art in ("ventil", "teil"):
        unten = sorted([t for t in frei if t["o"] == "s"], key=lambda t: t["x"])
        oben = sorted([t for t in frei if t["o"] == "n"], key=lambda t: t["x"])
        nu = {1: ["1"], 2: ["1", "3"], 3: ["5", "1", "3"]} if pneu else {1: ["P"], 2: ["P", "T"], 3: ["T", "P", "T2"]}
        no = {1: ["2"], 2: ["4", "2"]} if pneu else {1: ["A"], 2: ["A", "B"]}
        for t, n in zip(unten, nu.get(len(unten), [f"U{i + 1}" for i in range(len(unten))])):
            t["name"] = n
        for t, n in zip(oben, no.get(len(oben), [f"O{i + 1}" for i in range(len(oben))])):
            t["name"] = n
        for t in frei:
            if t["o"] in ("e", "w") and not t["name"]:
                t["name"] = ("14" if t["o"] == "w" else "12") if pneu else ("X" if t["o"] == "w" else "Y")
    elif art == "pumpe" and not pneu:
        for t in frei:
            t["name"] = {"n": "P", "s": "T"}.get(t["o"], "L")
    elif art in ("zylinder", "greifer", "schwenk", "motor"):
        for t, n in zip(sorted(frei, key=lambda t: (t["x"], t["y"])), ["A", "B", "C", "D", "E", "F"]):
            t["name"] = n
    else:
        s = sorted(frei, key=lambda t: (0 if t["o"] in ("s", "w") else 1, t["x"], t["y"]))
        for t, n in zip(s, (["1", "2", "3", "4", "5", "6"] if pneu else ["P", "A", "T", "B", "L", "X"])):
            t["name"] = n
    for i, t in enumerate(terms):
        if not t["name"]:
            t["name"] = str(i + 1)
    gesehen = {}
    for t in terms:
        if t["name"] in gesehen:
            gesehen[t["name"]] += 1
            t["name"] = f"{t['name']}.{gesehen[t['name']]}"
        else:
            gesehen[t["name"]] = 1


def zylinder_daten(rel, name_de, name_en, medium, anschluesse=2):
    r, n = rel.lower(), (name_de + " " + name_en).lower()
    einfach = "single" in r or "simple" in r or "einfach" in n or "single" in n or anschluesse == 1
    feder = "ressort" in r or "spring" in n or "feder" in n or (einfach and medium == "pneumatic")
    action = "single" if einfach else "double"
    d, s, h = ("20", "8", "100") if medium == "pneumatic" else ("40", "20", "200")
    stange = "" if ("rodless" in r or "stangenlos" in n) else f' rod_mm="{s}"'
    feder_attr = ' spring="rod"' if feder else ""
    cyl = f'<cylinder action="{action}"{feder_attr} bore_mm="{d}"{stange} stroke_mm="{h}"/>'
    return {"kind": "cylinder", "prefix": "MM", "function": "actuator.cyl_conf", "inner": [cyl],
            "kw": zylinder_kw(medium, action, feder), "info": cyl}


def baustein_daten(desc, terms):
    """Einzelnes Ventilkästchen (Baustein zum Zusammensetzen): eine Stellung mit ihren Wegen."""
    senk = [t for t in terms if t["o"] in ("n", "s")]
    kaesten = [r for r in rechtecke(desc) if 10 <= r[3] - r[1] <= 40
               and all(r[0] - 0.5 <= t["x"] <= r[2] + 0.5 for t in senk)]
    daten = {"kind": "other", "prefix": "", "function": "directional_valve.part", "inner": [], "kw": [], "info": ""}
    if not kaesten or not senk:
        return daten
    z = min(kaesten, key=lambda r: r[2] - r[0])
    ports = [{"name": t["name"], "o": t["o"], "off": t["x"] - z[0]} for t in senk]
    wege, gesperrt = zelle_wege(desc, z, ports, strecken(desc))
    daten["inner"] = ['<positions initial="1">',
                      "    <position>" + "".join(f'<path from="{a}" to="{b}"/>' for a, b in wege)
                      + "".join(f'<path from="{g}"/>' for g in gesperrt) + "</position>", "</positions>"]
    daten["info"] = "1: " + ",".join(f"{a}>{b}" for a, b in wege) + (" /" + ",".join(gesperrt) if gesperrt else "")
    return daten


# --------------------------------------------------------------------------- von Hand festgelegt
# Diese Hydraulikventile haben an beiden Kästchen Anschlüsse bzw. eine Mittelstellung ohne Kästchenrahmen;
# die Wege sind an der Zeichnung abgelesen. Anschlüsse: (x, y) -> Name; Anschlüsse am betätigten Kästchen
# (nicht Ruhestellung) bekommen ".2" und gehören zu keinem Weg.
VON_HAND = {
    "30_hydraulic/31_control_valves/11_2-2/distributeur_2-2.elmt": {
        "namen": {(10, 15): "P", (10, -15): "A", (-10, 15): "P.2", (-10, -15): "A.2"},
        "stellungen": [([], ["P", "A"]), ([("P", "A")], [])], "ruhe": 2},
    "30_hydraulic/31_control_valves/11_2-2/electrovanne_2_2_nf.elmt": {
        "namen": {(10, 14): "P", (10, -15): "A", (-10, 14): "P.2", (-10, -15): "A.2"},
        "stellungen": [([("P", "A")], []), ([], ["P", "A"])], "ruhe": 2},
    "30_hydraulic/31_control_valves/13_4-2/distributeur_4-2-centre_ouvert.elmt": {
        "namen": {(10, 19): "P", (20, 19): "T", (10, -19): "A", (20, -19): "B"},
        "stellungen": [([("P", "A"), ("B", "T")], []), ([("P", "A"), ("P", "B"), ("P", "T")], [])], "ruhe": 2},
}


def von_hand(rel, terms, medium):
    v = VON_HAND[rel]
    for t in terms:
        t["name"] = v["namen"][(round(t["x"]), round(t["y"]))]
    xml = [f'<positions initial="{v["ruhe"]}">']
    for wege, gesperrt in v["stellungen"]:
        xml.append("    <position>" + "".join(f'<path from="{a}" to="{b}"/>' for a, b in wege)
                   + "".join(f'<path from="{g}"/>' for g in gesperrt) + "</position>")
    xml += ["</positions>", '<actuation side="left" types="solenoid" spring="false"/>',
            '<actuation side="right" types="" spring="true"/>']
    n = len({x for x in v["namen"].values() if "." not in x})
    kw = list(VENTIL_HYD) + [("posNoMax", str(len(v["stellungen"])), ""), ("posNoInit", str(v["ruhe"]), "")]
    info = " | ".join(f"{i + 1}{'R' if i + 1 == v['ruhe'] else ''}: " + ",".join(f"{a}>{b}" for a, b in w)
                      + (" /" + ",".join(g) if g else "") for i, (w, g) in enumerate(v["stellungen"]))
    return {"kind": "valve", "prefix": "QM", "function": f"directional_valve.{WORT[n]}_pins",
            "inner": xml, "kw": kw, "info": info}


# --------------------------------------------------------------------------- Datei
def medium_von(rel):
    return "hydraulic" if rel.startswith("30_hydraulic") else "pneumatic"


def bearbeiten(pfad, rel):
    text = pfad.read_text(encoding="utf-8")
    d = ET.fromstring(text)
    desc = d.find("description")
    name_de = next((x.text or "" for x in d.iter("name") if x.get("lang") == "de"), "")
    name_en = next((x.text or "" for x in d.iter("name") if x.get("lang") == "en"), "")
    medium = medium_von(rel)
    terms = [{"e": t, "x": fz(t, "x"), "y": fz(t, "y"), "o": t.get("orientation"), "name": ""}
             for t in desc.iter("terminal")]
    art = bauart(rel, name_de, name_en)
    namen_aus_texten(terms, desc)
    standardnamen(terms, art, medium)
    hinweis = ""
    if rel in VON_HAND:
        daten = von_hand(rel, terms, medium)
    elif art == "ventil":
        daten, hinweis = ventil_daten(desc, terms, name_en.lower(), medium)
        if daten is None:
            daten = {"kind": "valve", "prefix": "QM",
                     "function": f"directional_valve.{WORT.get(len(terms), 'n')}_pins",
                     "inner": [], "kw": [], "info": ""}
    elif art == "zylinder":
        daten = zylinder_daten(rel, name_de, name_en, medium, len(terms))
    elif art == "teil" and terms:
        daten = baustein_daten(desc, terms)
    else:
        kind, prefix, fkt = FUNKTION[art]
        n = (name_de + " " + name_en).lower()
        if art == "absperr" and len(terms) == 3:
            fkt = "shutoff_valve.three_way_valve"
        elif art == "pumpe" and ("verstell" in n or "variable" in n):
            fkt = "supply.variable_pump"
        elif art == "motor" and ("variable" in n or "verstell" in n):
            fkt = "motor.variable_motor"
        daten = {"kind": kind, "prefix": prefix, "function": fkt, "inner": [], "kw": KW.get(art, []), "info": ""}
        if kind == "cylinder":
            daten["inner"] = ['<cylinder action="double"/>']
    daten["medium"] = medium
    return text, terms, art, daten, hinweis, name_de or name_en


def schreiben(text, terms, daten):
    def ersetze_term(m):
        tag = m.group(0)
        uuid = re.search(r'uuid="([^"]+)"', tag).group(1)
        t = next(t for t in terms if t["e"].get("uuid") == uuid)
        tag = re.sub(r'\sname="[^"]*"', "", tag)
        tag = tag.replace("<terminal", f'<terminal name="{t["name"]}"', 1)
        if "medium=" not in tag:
            tag = re.sub(r"\s*/>$", f' medium="{daten["medium"]}"/>', tag)
        return tag

    text = re.sub(r"<terminal\b[^>]*/>", ersetze_term, text)
    pre = f' prefix="{daten["prefix"]}"' if daten["prefix"] else ""
    teile = [f'<fluid kind="{daten["kind"]}"{pre} function="{daten["function"]}">']
    teile += ["    " + x for x in daten["inner"]]
    teile += [f'    <parameter name="{n}" value="{v}"' + (f' unit="{u}"' if u else "") + "/>"
              for n, v, u in daten["kw"]]
    teile.append("</fluid>")
    block = "\n    ".join(teile)
    text = re.sub(r"\s*<fluid\b.*?</fluid>", "", text, flags=re.S)
    return text.replace("</names>", "</names>\n    " + block, 1)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--nur-zeigen", action="store_true")
    ap.add_argument("--bericht", type=Path)
    ap.add_argument("--nur", help="nur Pfade mit diesem Teilstring")
    ap.add_argument("--ziel", type=Path, default=REPO / "bauteile/fluid")
    a = ap.parse_args()
    zeilen = []
    for basis in BASIS:
        for p in sorted(basis.rglob("*.elmt")):
            rel = p.relative_to(REPO / "elements").as_posix()
            if a.nur and a.nur not in rel:
                continue
            text, terms, art, daten, hinweis, name = bearbeiten(p, rel)
            zeilen.append([rel, name, art, daten["kind"], daten["function"],
                           " ".join(f"{t['name']}({t['o']})" for t in terms), daten["info"], hinweis])
            if not a.nur_zeigen:
                out = a.ziel / rel
                out.parent.mkdir(parents=True, exist_ok=True)
                out.write_text(schreiben(text, terms, daten), encoding="utf-8", newline="")
    ventile = [z for z in zeilen if z[2] == "ventil"]
    ok = [z for z in ventile if z[6]]
    print(f"{len(zeilen)} Symbole, davon {len(ventile)} Wegeventile, {len(ok)} mit Schaltwegen")
    for z in ventile:
        if not z[6]:
            print("  offen:", z[0], "-", z[7])
    if a.bericht:
        with a.bericht.open("w", encoding="utf-8-sig", newline="") as f:
            w = csv.writer(f, delimiter=";")
            w.writerow(["Datei", "Name", "Bauart", "kind", "function", "Anschlüsse", "Stellungen/Daten", "Hinweis"])
            w.writerows(zeilen)


if __name__ == "__main__":
    sys.exit(main())
