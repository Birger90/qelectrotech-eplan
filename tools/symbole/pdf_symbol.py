"""Symbol aus einem Vektor-PDF (z. B. EPLAN-Schaltplan) als QElectroTech-Bauteil übernehmen.

Ein Ausschnitt einer PDF-Seite wird 1:1 übernommen: Linien, Rechtecke, Bézierkurven (als Polylinie),
gefüllte Flächen, gestrichelte Linien und Texte. Leitungen, die aus dem Ausschnitt herauslaufen,
bleiben draußen (nur Teile, die ganz im Ausschnitt liegen). Die Anschlüsse werden angegeben:
Name, Lage im PDF (pt) und Richtung (n/s/e/w).

Maßstab: das EPLAN-Raster 4 mm wird zu 15 px, 8 mm zu 30 px; Anschlüsse liegen damit im
10-px-Raster von QElectroTech, wenn sie im PDF auf dem 8-mm-Raster liegen.

    python pdf_symbol.py plan.pdf 8 --clip 95,195,190,262 --list
    python pdf_symbol.py plan.pdf 8 --clip 95,195,190,262 --pin 1:127.7,200:n --pin N:150.4,200:n \
        --pin 2:127.7,255:s --pin N.:150.4,255:s --skip -F91 --ziel SAMMLUNG \
        --ordner Siemens/Schutzeinrichtungen/5SU1356-6KK06 --name "5SU1356-6KK06 Schaltbild" \
        --hersteller Siemens --bestell 5SU1356-6KK06 --beschreibung "FI/LS 1P+N B6A 30mA"
"""
import argparse
import math
import sys
from pathlib import Path

import pymupdf

sys.path.insert(0, str(Path(__file__).resolve().parent))
from nachzeichnen_symbole import Elem, info, STYLE, DASHED  # noqa: E402

S = 15 / (4 / 25.4 * 72)  # px je pt: 4 mm -> 15 px
THIN = STYLE.replace("line-weight:normal", "line-weight:thin")


def bezier(p0, p1, p2, p3, n=8):
    pts = []
    for i in range(n + 1):
        t = i / n
        a, b, c, d = (1 - t) ** 3, 3 * t * (1 - t) ** 2, 3 * t * t * (1 - t), t ** 3
        pts.append(pymupdf.Point(a * p0.x + b * p1.x + c * p2.x + d * p3.x,
                                 a * p0.y + b * p1.y + c * p2.y + d * p3.y))
    return pts


def segments(path):
    """Alle Teilstücke eines PDF-Pfads als Punktfolgen."""
    for it in path["items"]:
        kind = it[0]
        if kind == "l":
            yield [it[1], it[2]]
        elif kind == "c":
            yield bezier(it[1], it[2], it[3], it[4])
        elif kind == "re":
            r = it[1]
            yield [r.tl, r.tr, r.br, r.bl, r.tl]
        elif kind == "qu":
            q = it[1]
            yield [q.ul, q.ur, q.lr, q.ll, q.ul]


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    ap = argparse.ArgumentParser()
    ap.add_argument("pdf")
    ap.add_argument("seite", type=int, help="Seitenindex ab 0")
    ap.add_argument("--clip", required=True, help="x0,y0,x1,y1 in pt")
    ap.add_argument("--list", action="store_true", help="nur Inhalt des Ausschnitts zeigen")
    ap.add_argument("--pin", action="append", default=[], help="Name:x,y:Richtung (pt)")
    ap.add_argument("--skip", action="append", default=[], help="Text nicht übernehmen (Kennzeichen)")
    ap.add_argument("--ziel")
    ap.add_argument("--ordner", default="")
    ap.add_argument("--name", default="Symbol")
    ap.add_argument("--kennzeichen", default="-K")
    ap.add_argument("--hersteller", default="")
    ap.add_argument("--bestell", default="")
    ap.add_argument("--beschreibung", default="")
    ap.add_argument("--raster", type=float, default=10,
                    help="Anschlüsse auf dieses Raster runden (px); 0 = genau wie im PDF")
    a = ap.parse_args()

    # Git-Bash wandelt Argumente wie „/38/5“ in Windows-Pfade; „//38/5“ übersteht das und meint „/38/5“
    a.skip = [s[1:] if s.startswith("//") else s for s in a.skip]
    page = pymupdf.open(a.pdf)[a.seite]
    clip = pymupdf.Rect(*map(float, a.clip.split(",")))
    inside = pymupdf.Rect(clip.x0 - 0.3, clip.y0 - 0.3, clip.x1 + 0.3, clip.y1 + 0.3)
    paths = page.get_drawings()
    texts = []
    for b in page.get_text("dict", clip=clip)["blocks"]:
        for line in b.get("lines", []):
            for s in line["spans"]:
                if s["text"].strip() and s["text"].strip() not in a.skip and inside.contains(pymupdf.Rect(s["bbox"])):
                    texts.append((s, line["dir"]))

    if a.list:
        for p in paths:
            for seg in segments(p):
                if all(inside.contains(pt) for pt in seg):
                    print(p.get("type"), "fill" if p.get("fill") else "", round(p.get("width") or 0, 2),
                          p.get("dashes"), [(round(pt.x, 1), round(pt.y, 1)) for pt in seg[:: max(1, len(seg) // 3)]])
        for s, d in texts:
            print("T", s["text"], [round(v, 1) for v in s["bbox"]], round(s["size"], 1))
        return

    pins = []
    for spec in a.pin:
        name, xy, ori = spec.rsplit(":", 2)
        x, y = map(float, xy.split(","))
        pins.append((name, x, y, ori))
    ox, oy = (pins[0][1], pins[0][2]) if pins else (clip.x0, clip.y0)
    T = lambda pt: ((pt.x - ox) * S, (pt.y - oy) * S)  # noqa: E731

    e = Elem(a.name, a.kennzeichen, info(a.hersteller, a.bestell, a.bestell, a.beschreibung))
    # kurze Stücke, die nur halb im Ausschnitt liegen, gehören meist zum Symbol (Kontaktschräge):
    # Ausschnitt dann zu knapp -> melden
    for p in paths:
        for seg in segments(p):
            drin = [inside.contains(pt) for pt in seg]
            if any(drin) and not all(drin) and math.dist(seg[0], seg[-1]) < 15:
                print(f"Warnung: Linie {[(round(pt.x, 1), round(pt.y, 1)) for pt in (seg[0], seg[-1])]} "
                      f"ragt aus dem Ausschnitt ({a.name})", file=sys.stderr)
    for p in paths:
        dashed = bool(p.get("dashes")) and p["dashes"] not in ("[] 0", "[] 0.0")
        style = DASHED if dashed else (THIN if (p.get("width") or 0) < 0.5 else STYLE)
        filled = bool(p.get("fill")) and p.get("type") in ("f", "fs")
        segs = list(segments(p))
        if filled and segs and all(len(sg) == 2 for sg in segs):
            # EPLAN füllt Flächen als Folge einzelner Linienstücke: zu einem Umriss zusammensetzen
            umriss = [segs[0][0]] + [sg[1] for sg in segs]
            segs = [umriss]
        for seg in segs:
            if not all(inside.contains(pt) for pt in seg):
                continue
            if filled and len(seg) > 2:
                pts = [T(pt) for pt in seg]
                e._box(*pts)
                e.parts.append(("polygon", {**{f"x{i + 1}": f"{x:.2f}" for i, (x, _) in enumerate(pts)},
                                            **{f"y{i + 1}": f"{y:.2f}" for i, (_, y) in enumerate(pts)},
                                            "closed": "true", "antialias": "true",
                                            "style": STYLE.replace("filling:none", "filling:black")}))
            elif dashed and len(seg) > 2:
                # gestrichelte Bögen als ein Linienzug, sonst fängt das Strichmuster in jedem Stück neu an
                pts = [T(pt) for pt in seg]
                e._box(*pts)
                e.parts.append(("polygon", {**{f"x{i + 1}": f"{x:.2f}" for i, (x, _) in enumerate(pts)},
                                            **{f"y{i + 1}": f"{y:.2f}" for i, (_, y) in enumerate(pts)},
                                            "closed": "false", "antialias": "true", "style": style}))
            else:
                for p1, p2 in zip(seg, seg[1:]):
                    (x1, y1), (x2, y2) = T(p1), T(p2)
                    if math.hypot(x2 - x1, y2 - y1) > 0.05:
                        e.line(x1, y1, x2, y2, style)
    for s, d in texts:
        x0, y0, x1, y1 = s["bbox"]
        rot = -math.degrees(math.atan2(d[1], d[0]))
        size = max(4, round(s["size"] * S) - 1)
        # QETs Liberation Sans läuft breiter als die PDF-Schrift (EPLAN: Tahoma): Text nicht breiter
        # werden lassen als im Original, sonst ragt er über Rahmen und Nachbartexte
        n = len(s["text"].strip())
        room = (x1 - x0 if abs(rot) < 1 else y1 - y0) * S
        # nur Schrift, keine Zeichen-Grafik (EPLAN setzt z. B. Schalterstellungen als ■-Glyphen)
        if n and any(c.isalnum() for c in s["text"]) and 0.68 * size * n > room:
            size = max(4, int(room / (0.68 * n)))
        base = pymupdf.Point(x0, y1 - 0.2 * s["size"]) if abs(rot) < 1 else pymupdf.Point(x1 - 0.2 * s["size"], y1)
        tx, ty = T(base)
        e.text(tx, ty, s["text"].strip(), size=size, rot=-rot, bold=bool(s["flags"] & 16))  # 16 = fett
    for name, x, y, ori in pins:
        tx, ty = T(pymupdf.Point(x, y))
        if a.raster:
            tx, ty = round(tx / a.raster) * a.raster, round(ty / a.raster) * a.raster
        e.term(round(tx, 1), round(ty, 1), ori, name)

    levels = [v for v in a.ordner.split("/") if v]
    out = e.write(Path(a.ziel), levels, a.name)
    print(out)


if __name__ == "__main__":
    main()
