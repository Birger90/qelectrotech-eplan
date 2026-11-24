"""DMSP-Kraftkennlinien F(h, p) exakt aus den Vektorpfaden des Festo-Katalogs DMSP_EN.PDF (2026/01, S. 14/15) lesen.

Achsen: Teilstriche (0,232-pt-Linien, 5,56 pt lang) am unteren bzw. linken Rand, Werte laut Diagramm. Kurven: 0,926-pt-
Linien, Druckzuordnung über das Strichmuster der Legende (gleiche Reihenfolge 0, 1, 2 … bar).
Aufruf: python -I dmsp_kennlinien.py DMSP_EN.PDF ausgabe.json
"""
import json
import sys
import pymupdf

# (Seite, Typ, x-Teilung, y-Teilung, y-Bereich der Grafik in pt)
GRAFIKEN = [
    (13, "DMSP-5", list(range(-5, 30, 5)), list(range(0, 201, 50)), (250, 400)),
    (13, "DMSP-10", list(range(-5, 30, 5)), list(range(0, 801, 200)), (420, 560)),
    (14, "DMSP-20", list(range(-5, 35, 5)), list(range(0, 2001, 500)), (120, 270)),
    (14, "DMSP-40", list(range(-5, 35, 5)), list(range(0, 8001, 2000)), (290, 440)),
]


def pfadpunkte(g):
    pts = []
    for it in g["items"]:
        if it[0] != "l":
            raise ValueError("Kurve mit Nicht-Linien-Element")
        a, b = it[1], it[2]
        if not pts or (abs(pts[-1][0] - a.x) > 1e-3 or abs(pts[-1][1] - a.y) > 1e-3):
            pts.append((a.x, a.y))
        pts.append((b.x, b.y))
    return pts


def muster(g, breite):
    """Strichmuster in Linienbreiten, gerundet (Legende 1,7 pt, Kurven 0,926 pt)."""
    s = (g.get("dashes") or "[] 0").split("]")[0].strip("[ ")
    return tuple(round(float(v) / breite, 1) for v in s.split()) if s else ()


def linear(ticks, werte):
    """Ausgleichsgerade Wert = a + b·Koordinate über alle Teilstriche (kleinste Quadrate)."""
    n = len(ticks)
    mx, my = sum(ticks) / n, sum(werte) / n
    b = sum((t - mx) * (w - my) for t, w in zip(ticks, werte)) / sum((t - mx) ** 2 for t in ticks)
    rest = max(abs(my + b * (t - mx) - w) for t, w in zip(ticks, werte))
    return (lambda t: my + b * (t - mx)), rest


def main():
    d = pymupdf.open(sys.argv[1])
    out = {}
    for seite, typ, xt, yt, (y0, y1) in GRAFIKEN:
        dr = [g for g in d[seite].get_drawings() if g["rect"].y0 >= y0 - 20 and g["rect"].y1 <= y1 + 20]
        duenn = [g for g in dr if g.get("width") and abs(g["width"] - 0.232) < 0.01 and len(g["items"]) == 1]
        vert = sorted({round(g["items"][0][1].x, 2) for g in duenn
                       if abs(g["rect"].width) < 0.01 and 5.0 < g["rect"].height < 6.0})
        hori = sorted({round(g["items"][0][1].y, 2) for g in duenn
                       if abs(g["rect"].height) < 0.01 and 5.0 < g["rect"].width < 6.0}, reverse=True)
        if len(vert) != len(xt) or len(hori) != len(yt):
            raise SystemExit(f"{typ}: {len(vert)} x-Teilstriche (soll {len(xt)}), {len(hori)} y-Teilstriche (soll {len(yt)})")
        fx, rx = linear(vert, xt)
        fy, ry = linear(hori, yt)
        dick = [g for g in dr if g.get("width") and abs(g["width"] - 0.926) < 0.01 and g["rect"].x1 < 280]
        kurven = [g for g in dick if g["rect"].width > 1.0]          # ohne die senkrechte Vorspanngrenze
        legende = [g for g in d[seite].get_drawings() if g.get("width") and abs(g["width"] - 1.7) < 0.01
                   and g["rect"].x0 > 280 and y0 - 20 <= g["rect"].y0 <= y1 + 20]
        legende.sort(key=lambda g: g["rect"].y0)
        if len(kurven) != len(legende):
            raise SystemExit(f"{typ}: {len(kurven)} Kurven, {len(legende)} Legendeneinträge")
        # Zuordnung in Zeichenreihenfolge (= Legendenreihenfolge 0, 1, 2 … bar); die Strichmuster der Legende sind
        # nicht maßstäblich gleich. Prüfung: bei jeder Kontraktion steigt die Kraft mit dem Druck
        res = {}
        for bar, k in enumerate(kurven):
            res[str(bar)] = [[round(fx(x), 4), round(fy(y), 2)] for x, y in pfadpunkte(k)]
        def f_at(c, h):
            for (h0, f0), (h1, f1) in zip(c, c[1:]):
                if min(h0, h1) <= h <= max(h0, h1) and h1 != h0:
                    return f0 + (f1 - f0) * (h - h0) / (h1 - h0)
            return None
        for bar in range(1, len(kurven)):
            for h, F in res[str(bar - 1)]:
                g = f_at(res[str(bar)], h)
                if g is not None and g < F - 1e-6 * max(yt) and F < 0.999 * max(yt):
                    raise SystemExit(f"{typ}: {bar} bar unter {bar - 1} bar bei h = {h} % ({g} < {F} N)")
        out[typ] = {"kurven_bar": res, "achse_rest_x_prozent": round(rx, 4), "achse_rest_y_N": round(ry, 3)}
    json.dump(out, open(sys.argv[2], "w", encoding="utf-8"), indent=1)
    for t, v in out.items():
        print(t, {b: (len(c), c[0], c[-1]) for b, c in v["kurven_bar"].items()}, v["achse_rest_x_prozent"], v["achse_rest_y_N"])


main()
