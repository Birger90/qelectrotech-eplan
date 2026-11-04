"""Bauteil aus einer DWG-Datei (z. B. WSCAD Universe „Download DWG“) als QElectroTech-Element.

Die DWG wird mit dwg_lesen.py gelesen. Übernommen wird der im Modellbereich eingefügte Block samt verschachtelter
Blöcke. Anschlüsse erkennt das Werkzeug wie in WSCAD gezeichnet: ein kurzer Stich (bis 5 mm) beginnt auf dem
Gerätekasten und zeigt nach außen; der Anschluss liegt am äußeren Ende, sein Name ist der Text direkt daneben
(z. B. „X3.1“), die Funktion (z. B. „24V“) bleibt als Text im Symbol. Artikeldaten kommen aus der .wspak, wenn
angegeben (Data/part.json: Hersteller, Bestellnummer, Bezeichnung).

    python dwg_symbol.py SCHALTBILD.dwg --ziel SAMMLUNG --ordner WAGO/Feldbus/750-362 --name "750-362 Schaltbild" \
        [--wspak PAKET.wspak] [--raster 5]
"""
import argparse
import json
import math
import sys
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "symbole"))
import dwg_lesen  # noqa: E402
from nachzeichnen_symbole import Elem, info  # noqa: E402


def _trafo(einfuegung, basis):
    """Abbildung Blockkoordinaten -> übergeordnete Koordinaten einer Einfügung."""
    (px, py, _), (sx, sy, _), r = einfuegung["p"], einfuegung["s"], einfuegung["drehung"]
    bx, by = basis[:2]
    c, s = math.cos(r), math.sin(r)

    def f(x, y):
        x, y = (x - bx) * sx, (y - by) * sy
        return px + c * x - s * y, py + s * x + c * y
    return f, r


def elemente(objekte):
    """Alle zeichenbaren Elemente des im Modellbereich eingefügten Blocks, in Modellkoordinaten:
    [(Objekt, Abbildung, Drehung)]."""
    kopf = {h: o for h, o in objekte.items() if o.typ == 49}
    inhalt = {}
    for o in objekte.values():
        if o.typ in dwg_lesen.ENTITAETEN and o.werte.get("modus") == 0 and o.eigentuemer in kopf:
            inhalt.setdefault(o.eigentuemer, []).append(o)
    aus = []

    def block(h, f, r, tiefe):
        for o in inhalt.get(h, []):
            if o.werte.get("fehler") or o.werte.get("unsichtbar"):
                continue
            if o.typ in (7, 8) and o.verweise and tiefe < 8:
                g, r2 = _trafo(o.werte, kopf[o.verweise[0]].werte.get("basis", (0, 0, 0)))
                block(o.verweise[0], lambda x, y, g=g, f=f: f(*g(x, y)), r + r2, tiefe + 1)
            elif o.typ not in (4, 5, 6):
                aus.append((o, f, r))

    for o in objekte.values():
        if o.typ in (7, 8) and o.werte.get("modus") == 2 and o.verweise and o.verweise[0] in kopf:
            f, r = _trafo(o.werte, kopf[o.verweise[0]].werte.get("basis", (0, 0, 0)))
            block(o.verweise[0], f, r, 0)
    return aus


def _segmente(o, f):
    """Linienzüge eines Elements als Punktlisten (Modellkoordinaten, Bögen angenähert)."""
    w, t = o.werte, o.typ
    if t == 19:
        return [[f(*w["a"]), f(*w["b"])]]
    if t == 77:
        p, b = w["punkte"], w["bulges"] + [0.0] * len(w["punkte"])
        if w["geschlossen"]:
            p = p + p[:1]
        pts = [f(*p[0])]
        for i in range(len(p) - 1):
            (x1, y1), (x2, y2), bu = p[i], p[i + 1], b[i]
            if abs(bu) > 1e-9:                         # Bogensegment (Bulge = tan(Winkel/4))
                a = 4 * math.atan(bu)
                d = math.hypot(x2 - x1, y2 - y1)
                r = d / (2 * math.sin(a / 2))
                mx, my = (x1 + x2) / 2, (y1 + y2) / 2
                h = r * math.cos(a / 2)
                nx, ny = -(y2 - y1) / d, (x2 - x1) / d
                cx, cy = mx + nx * h, my + ny * h
                a0 = math.atan2(y1 - cy, x1 - cx)
                for k in range(1, 9):
                    pts.append(f(cx + abs(r) * math.cos(a0 + a * k / 8), cy + abs(r) * math.sin(a0 + a * k / 8)))
            else:
                pts.append(f(x2, y2))
        return [pts]
    if t == 31:
        e = w["ecken"]
        return [[f(*e[0]), f(*e[1]), f(*e[3]), f(*e[2]), f(*e[0])]]
    if t in (17, 18):
        (cx, cy, _), r = w["m"], w["r"]
        a0, a1 = (w["w1"], w["w2"]) if t == 17 else (0.0, 2 * math.pi)
        if a1 < a0:
            a1 += 2 * math.pi
        n = max(8, int((a1 - a0) / (math.pi / 16)))
        return [[f(cx + r * math.cos(a0 + (a1 - a0) * k / n), cy + r * math.sin(a0 + (a1 - a0) * k / n))
                 for k in range(n + 1)]]
    return []


def anschluesse(teile):
    """Anschlüsse: kurze Stiche, die mit einem Ende auf dem größten geschlossenen Umriss liegen."""
    umrisse = [_segmente(o, f)[0] for o, f, _ in teile if o.typ == 77 and o.werte["geschlossen"]]
    if not umrisse:
        return []
    kasten = max(umrisse, key=lambda p: (max(x for x, _ in p) - min(x for x, _ in p)) *
                 (max(y for _, y in p) - min(y for _, y in p)))
    x0, x1 = min(x for x, _ in kasten), max(x for x, _ in kasten)
    y0, y1 = min(y for _, y in kasten), max(y for _, y in kasten)

    def auf_rand(x, y):
        return (abs(y - y1) < 0.05 or abs(y - y0) < 0.05) and x0 - 0.05 <= x <= x1 + 0.05 or \
               (abs(x - x0) < 0.05 or abs(x - x1) < 0.05) and y0 - 0.05 <= y <= y1 + 0.05

    texte = [(o.werte["text"], f(*o.werte["p"])) for o, f, _ in teile if o.typ == 1]
    aus = []
    for o, f, _ in teile:
        for seg in _segmente(o, f):
            if len(seg) != 2 or o.typ == 77 and o.werte["geschlossen"]:
                continue
            (ax, ay), (bx, by) = seg
            if math.hypot(bx - ax, by - ay) > 5.01:
                continue
            if auf_rand(bx, by) and not auf_rand(ax, ay):
                (ax, ay), (bx, by) = (bx, by), (ax, ay)
            if not auf_rand(ax, ay) or auf_rand(bx, by):
                continue
            richtung = "n" if by > ay + 1e-6 else "s" if by < ay - 1e-6 else "e" if bx > ax else "w"
            # Name: nächster Text am Stich, der wie eine Anschlussbezeichnung aussieht (ohne Leerzeichen)
            kandidaten = sorted((math.hypot(tx - (ax + bx) / 2, ty - (ay + by) / 2), s)
                                for s, (tx, ty) in texte if s.strip() and " " not in s.strip())
            name = kandidaten[0][1] if kandidaten and kandidaten[0][0] < 3 else f"{len(aus) + 1}"
            aus.append((name, bx, by, richtung, seg))
    return aus


def symbol(dwg, name, wspak=None, raster=5.0):
    version, objekte = dwg_lesen.lesen(dwg)
    teile = elemente(objekte)
    if not teile:
        raise dwg_lesen.DwgFehler("kein eingefügter Block im Modellbereich")
    pins = anschluesse(teile)
    daten = {}
    if wspak:
        z = zipfile.ZipFile(wspak)
        daten = json.loads(z.read("Data/part.json").decode("utf-8-sig"))
    inf = info(daten.get("manufacturerName", ""), daten.get("number", ""), daten.get("typeName", ""),
               (daten.get("translateName") or {}).get("De") or daten.get("descriptionOne", ""))
    e = Elem(name, "-K", inf)
    k = 10.0 / raster                                 # ein Anschlussraster = 10 px (QElectroTech-Raster)
    ox, oy = (pins[0][1], pins[0][2]) if pins else (0.0, 0.0)

    def T(x, y):
        return round((x - ox) * k, 2), round(-(y - oy) * k, 2)

    for o, f, r in teile:
        if o.typ in (1, 2, 3):
            s = o.werte.get("text", "") if o.typ != 3 else ""
            if not s.strip() or s.strip() in ("...", "…") or s.strip() == "-K":
                continue                              # Platzhalter und Kennzeichen (kommt als Datenfeld)
            w = o.werte
            # Einfügepunkt = Anfang der Grundlinie; bei ausgerichteten Texten hat AutoCAD ihn schon berechnet,
            # QElectroTech setzt Texte ebenso an der Grundlinie an
            x, y = f(*w["p"])
            rot = math.degrees(w["drehung"] + r)
            groesse = max(4, round(w["hoehe"] * k * 1.1))
            tx, ty = T(x, y)
            e.text(tx, ty, s, size=groesse, rot=-rot)
        else:
            for seg in _segmente(o, f):
                pts = [T(*p) for p in seg]
                for (x1, y1), (x2, y2) in zip(pts, pts[1:]):
                    if math.hypot(x2 - x1, y2 - y1) > 0.05:
                        e.line(x1, y1, x2, y2)
    for nm, x, y, ori, _ in pins:
        tx, ty = T(x, y)
        e.term(tx, ty, ori, nm)
    return e, pins


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    ap = argparse.ArgumentParser()
    ap.add_argument("dwg")
    ap.add_argument("--ziel", required=True)
    ap.add_argument("--ordner", default="")
    ap.add_argument("--name", required=True)
    ap.add_argument("--wspak")
    ap.add_argument("--raster", type=float, default=5.0, help="Anschlussraster in mm (WSCAD 5, EPLAN 4)")
    a = ap.parse_args()
    e, pins = symbol(a.dwg, a.name, a.wspak, a.raster)
    print("Anschlüsse:", ", ".join(p[0] for p in pins) or "keine")
    print(e.write(Path(a.ziel), [v for v in a.ordner.split("/") if v], a.name))


if __name__ == "__main__":
    main()
