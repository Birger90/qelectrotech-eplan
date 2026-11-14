"""Anschlüsse umgewandelter Fluidsymbole auf das QET-Raster (10) bringen, ohne die Grafik zu verbiegen.

Zwei Schritte je Symbol:
1. Das ganze Symbol verschieben, so dass möglichst alle Anschlüsse in ihrer Querrichtung auf dem Raster liegen
   (senkrechte Anschlüsse n/s: x, waagerechte e/w: y).
2. Die Längsrichtung jedes Anschlusses richtet der Anschlussstummel: der Anschluss rückt nach außen auf das
   nächste Raster, die Anschlussleitung wird bis dorthin verlängert (bzw. neu gezogen).

Keine Haken, keine Knicke: Ein Anschluss, dessen Querrichtung auch nach dem Verschieben neben dem Raster liegt,
wird nicht angefasst und steht im Bericht.

    python fluid_raster_einpassen.py ORDNER [--bericht datei.csv]
"""
import argparse
import csv
import math
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

R = 10.0
TOL = 0.6

KOORD = {
    "line": [("x1", "y1"), ("x2", "y2")],
    "rect": [("x", "y")], "ellipse": [("x", "y")], "arc": [("x", "y")], "circle": [("x", "y")],
    "text": [("x", "y")], "dynamic_text": [("x", "y")], "terminal": [("x", "y")], "input": [("x", "y")],
}


def f(v):
    s = f"{v:.2f}".rstrip("0").rstrip(".")
    return "0" if s in ("-0", "") else s


def auf_raster(v):
    return abs(v / R - round(v / R)) < 1e-6


def punkte(e):
    if e.tag == "polygon":
        i = 1
        while e.get(f"x{i}") is not None:
            yield (f"x{i}", f"y{i}")
            i += 1
    else:
        yield from KOORD.get(e.tag, [])


def verschiebe(desc, dx, dy):
    for e in desc.iter():
        for ax, ay in punkte(e):
            if e.get(ax) is None:
                continue
            e.set(ax, f(float(e.get(ax)) + dx))
            e.set(ay, f(float(e.get(ay)) + dy))


def beste_verschiebung(werte):
    if not werte:
        return 0.0
    kand = {0.0} | {round(-(v % R), 4) for v in werte} | {round(R - (v % R), 4) % R for v in werte}
    return min(kand, key=lambda d: (sum(not auf_raster(v + d) for v in werte), min(abs(d), abs(d - R))))


def grenzen(desc):
    xs, ys = [], []
    for e in desc.iter():
        if e.tag in ("rect", "ellipse", "arc"):
            x, y, w, h = (float(e.get(k)) for k in ("x", "y", "width", "height"))
            xs += [x, x + w]; ys += [y, y + h]
        elif e.tag == "circle":
            x, y, d = float(e.get("x")), float(e.get("y")), float(e.get("diameter"))
            xs += [x, x + d]; ys += [y, y + d]
        else:
            for ax, ay in punkte(e):
                if e.get(ax) is not None:
                    xs.append(float(e.get(ax))); ys.append(float(e.get(ay)))
    return min(xs), min(ys), max(xs), max(ys)


def einpassen(text):
    """Gibt (neuer_text, offene_anschluesse, geaendert) zurück."""
    root = ET.fromstring(text)
    desc = root.find("description")
    terms = [t for t in desc.iter("terminal")]
    if not terms:
        return text, [], False
    senk = [float(t.get("x")) for t in terms if t.get("orientation") in ("n", "s")]
    waag = [float(t.get("y")) for t in terms if t.get("orientation") in ("e", "w")]
    dx = beste_verschiebung(senk) if senk else beste_verschiebung([float(t.get("x")) for t in terms])
    dy = beste_verschiebung(waag) if waag else beste_verschiebung([float(t.get("y")) for t in terms])
    geaendert = bool(dx or dy)
    verschiebe(desc, dx, dy)

    offen = []
    linien = [e for e in desc if e.tag == "line"]
    for t in terms:
        o = t.get("orientation")
        x, y = float(t.get("x")), float(t.get("y"))
        quer, laengs = (x, y) if o in ("n", "s") else (y, x)
        if not auf_raster(quer):
            offen.append(f"{t.get('name') or '?'}({o})")
            continue
        if auf_raster(laengs):
            continue
        aussen = {"s": math.ceil, "e": math.ceil, "n": math.floor, "w": math.floor}[o]
        neu = aussen(laengs / R) * R
        nx, ny = (x, neu) if o in ("n", "s") else (neu, y)
        stummel = False
        for ln in linien:
            for ax, ay in (("x1", "y1"), ("x2", "y2")):
                if abs(float(ln.get(ax)) - x) < TOL and abs(float(ln.get(ay)) - y) < TOL:
                    andere = ("x2", "y2") if ax == "x1" else ("x1", "y1")
                    gerade = (abs(float(ln.get(andere[0])) - x) < TOL) if o in ("n", "s") \
                        else (abs(float(ln.get(andere[1])) - y) < TOL)
                    if gerade:
                        ln.set(ax, f(nx)); ln.set(ay, f(ny)); stummel = True
        if not stummel:
            neu_ln = ET.SubElement(desc, "line", {
                "x1": f(x), "y1": f(y), "x2": f(nx), "y2": f(ny), "end1": "none", "end2": "none",
                "length1": "1.5", "length2": "1.5",
                "style": "line-style:normal;line-weight:normal;filling:none;color:black", "antialias": "false"})
            linien.append(neu_ln)
        t.set("x", f(nx)); t.set("y", f(ny))
        geaendert = True

    if not geaendert:
        return text, offen, False
    x0, y0, x1, y1 = grenzen(desc)
    gx0, gy0 = math.floor(x0 / R) * R, math.floor(y0 / R) * R
    gx1, gy1 = math.ceil(x1 / R) * R, math.ceil(y1 / R) * R
    root.set("width", f(gx1 - gx0)); root.set("height", f(gy1 - gy0))
    root.set("hotspot_x", f(-gx0)); root.set("hotspot_y", f(-gy0))
    ET.indent(root, "    ")
    return "<?xml version='1.0' encoding='utf-8'?>\n" + ET.tostring(root, encoding="unicode") + "\n", offen, True


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("ordner", type=Path)
    ap.add_argument("--bericht", type=Path)
    a = ap.parse_args()
    zeilen, n_aend, n_offen = [], 0, 0
    for p in sorted(a.ordner.rglob("*.elmt")):
        t = p.read_text(encoding="utf-8")
        neu, offen, aend = einpassen(t)
        if aend:
            p.write_text(neu, encoding="utf-8", newline="")
            n_aend += 1
        if offen:
            n_offen += 1
            zeilen.append([p.relative_to(a.ordner).as_posix(), " ".join(offen)])
    print(f"{n_aend} Symbole eingepasst, {n_offen} mit Anschlüssen quer neben dem Raster")
    if a.bericht:
        with open(a.bericht, "w", newline="", encoding="utf-8-sig") as fh:
            w = csv.writer(fh, delimiter=";")
            w.writerow(["Datei", "Anschlüsse neben dem Raster"])
            w.writerows(zeilen)


if __name__ == "__main__":
    sys.exit(main())
