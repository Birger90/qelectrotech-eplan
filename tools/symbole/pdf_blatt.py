"""Ein ganzes Schaltplanblatt aus einem Vektor-PDF (EPLAN) in QElectroTech-Operationen umsetzen.

Leitungen (waagerechte/senkrechte Linien in Leiterstärke) werden zu echten Leitern, Ecken und Abzweige zu
Knoten, alles übrige Zusammenhängende (Geräte, Kästen, Kabelbilder, Pfeile) zu Bauteilen, die genau dort
Anschlüsse bekommen, wo eine Leitung anstößt. Freie Texte, strichpunktierte Ortsrahmen und Rasterbilder werden
mit ausgegeben. Ergebnis: Bauteile in der Sammlung und eine JSON-Datei, aus der ein Blattskript die
tool_edit-Operationen baut.

    python pdf_blatt.py plan.pdf 36 --ziel SAMMLUNG --ordner "Plaene/Plan/Blatt 36" --json blatt36.json
"""
import argparse
import json
import math
import sys
from pathlib import Path

import pymupdf

sys.path.insert(0, str(Path(__file__).resolve().parent))
from nachzeichnen_symbole import Elem, info, STYLE, DASHED  # noqa: E402
from pdf_symbol import segments, S, THIN  # noqa: E402

WIRE_W = (0.6, 0.9)                # Strichstärke von Leitungen (pt)
PE_DASH = "5.66929"                # Strichmuster gestrichelter Leitungen (PE)
FRAME_DASH = "9.04825"             # Strich-Punkt der Ortsrahmen
TOL = 0.35                         # Berührung Leitung/Leitung (pt)
TOL_G = 0.7                        # Berührung Leitung/Grafik (pt)
TOL_C = 0.9                        # Abstand, bis zu dem Grafikteile ein Bauteil bilden (pt)
OFF = {"n": (0, -0.1), "w": (-0.1, 0), "e": (0.1, 0), "s": (0, 0.1)}   # px, damit Anschlüsse unterscheidbar bleiben


def dist_ps(p, a, b):
    """Abstand Punkt p zur Strecke a-b."""
    dx, dy = b[0] - a[0], b[1] - a[1]
    L = dx * dx + dy * dy
    t = 0 if L == 0 else max(0, min(1, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / L))
    return math.hypot(p[0] - a[0] - t * dx, p[1] - a[1] - t * dy)


class Wire:
    def __init__(self, a, b, dashed):
        if (a[0], a[1]) > (b[0], b[1]):
            a, b = b, a
        self.a, self.b, self.dashed = a, b, dashed
        self.horiz = abs(a[1] - b[1]) < 0.05

    def interior(self, p, tol=TOL):
        """p liegt auf der Leitung, aber nicht an einem Ende."""
        return dist_ps(p, self.a, self.b) < tol and math.dist(p, self.a) > tol and math.dist(p, self.b) > tol


class Graphic:
    """Grafikteil: Polylinie (Linien, Kurven, Rechtecke) oder gefüllte Fläche."""
    def __init__(self, pts, style, filled=False, fill=None):
        self.pts, self.style, self.filled, self.fill = pts, style, filled, fill
        xs, ys = [p[0] for p in pts], [p[1] for p in pts]
        self.bbox = (min(xs), min(ys), max(xs), max(ys))

    def dist(self, p):
        if len(self.pts) == 1:
            return math.dist(p, self.pts[0])
        return min(dist_ps(p, a, b) for a, b in zip(self.pts, self.pts[1:]))

    def sample(self):
        out = []
        for a, b in zip(self.pts, self.pts[1:]):
            n = max(1, int(math.dist(a, b)))
            out += [(a[0] + (b[0] - a[0]) * i / n, a[1] + (b[1] - a[1]) * i / n) for i in range(n + 1)]
        return out or list(self.pts)

    def near(self, other, tol):
        a, b = self.bbox, other.bbox
        if a[0] - tol > b[2] or b[0] - tol > a[2] or a[1] - tol > b[3] or b[1] - tol > a[3]:
            return False
        return any(other.dist(p) <= tol for p in self.sample()) or any(self.dist(p) <= tol for p in other.sample())


def load(page, area):
    """Leitungskandidaten, Grafik, Rahmenlinien, Texte und Bilder des Inhaltsbereichs."""
    x0, y0, x1, y1 = area
    inside = lambda p: x0 <= p[0] <= x1 and y0 <= p[1] <= y1   # noqa: E731
    wires, graphics, frames = [], [], []
    for p in page.get_drawings():
        dashes = p.get("dashes") or "[] 0"
        dashed = dashes not in ("[] 0", "[] 0.0")
        width = p.get("width") or 0
        filled = bool(p.get("fill")) and p.get("type") in ("f", "fs")
        segs = [[(pt.x, pt.y) for pt in seg] for seg in segments(p)]
        if filled and segs and all(len(sg) == 2 for sg in segs):
            segs = [[segs[0][0]] + [sg[1] for sg in segs]]
        for seg in segs:
            if not all(inside(q) for q in seg):
                continue
            if dashed and FRAME_DASH in dashes:
                frames += [[a, b] for a, b in zip(seg, seg[1:])]
                continue
            if filled:
                graphics.append(Graphic(seg, "fill", True, p.get("fill")))
                continue
            style = DASHED if dashed else (THIN if width < 0.5 else STYLE)
            pe = dashed and PE_DASH in dashes
            if len(seg) > 2:                                   # Kurve oder Rechteck: ein Grafikteil
                graphics.append(Graphic(seg, style))
                continue
            a, b = seg
            axis = abs(a[0] - b[0]) < 0.05 or abs(a[1] - b[1]) < 0.05
            if axis and WIRE_W[0] <= width <= WIRE_W[1] and (not dashed or pe) and math.dist(a, b) > 1.5:
                wires.append(Wire(a, b, dashed))
            elif math.dist(a, b) > 0.02:
                graphics.append(Graphic([a, b], style))
    texts = []
    for blk in page.get_text("dict")["blocks"]:
        for line in blk.get("lines", []):
            d = line["dir"]
            for s in line["spans"]:
                t = s["text"].strip()
                bb = s["bbox"]
                if not t or not (x0 <= bb[0] and bb[2] <= x1 and y0 <= bb[1] and bb[3] <= y1):
                    continue
                if abs(d[0]) > 0.01 and abs(d[1]) > 0.01:          # schräger Stempel (Druckdatum)
                    continue
                texts.append({"t": t, "bbox": list(bb), "dir": list(d), "size": s["size"], "bold": bool(s["flags"] & 16)})
    images = [{"xref": i["xref"], "bbox": list(i["bbox"])} for i in page.get_image_info(xrefs=True)
              if x0 <= i["bbox"][0] and i["bbox"][2] <= x1 and y0 <= i["bbox"][1] and i["bbox"][3] <= y1]
    return wires, graphics, frames, texts, images


def merge_collinear(wires):
    """Gerade, sich berührende Stücke derselben Linie zusammenfassen."""
    groups = {}
    for w in wires:
        key = ("h", round(w.a[1], 1), w.dashed) if w.horiz else ("v", round(w.a[0], 1), w.dashed)
        groups.setdefault(key, []).append(w)
    out = []
    for (o, c, dashed), ws in groups.items():
        iv = sorted((w.a[0], w.b[0]) if o == "h" else (w.a[1], w.b[1]) for w in ws)
        cur = list(iv[0])
        merged = []
        for s0, s1 in iv[1:]:
            if s0 <= cur[1] + 0.05:
                cur[1] = max(cur[1], s1)
            else:
                merged.append(cur)
                cur = [s0, s1]
        merged.append(cur)
        out += [Wire((s0, c), (s1, c), dashed) if o == "h" else Wire((c, s0), (c, s1), dashed) for s0, s1 in merged]
    return out


def rectangles(wires):
    """Leitungskandidaten, die Kästen bilden (vier Seiten), sind Grafik."""
    hs = [w for w in wires if w.horiz]
    vs = [w for w in wires if not w.horiz]
    box = set()

    def has_v(x, ya, yb):
        return [v for v in vs if abs(v.a[0] - x) < 0.2 and v.a[1] <= ya + 0.2 and v.b[1] >= yb - 0.2]
    for i, h1 in enumerate(hs):
        for h2 in hs[i + 1:]:
            if abs(h1.a[0] - h2.a[0]) < 0.2 and abs(h1.b[0] - h2.b[0]) < 0.2 and abs(h1.a[1] - h2.a[1]) > 3:
                ya, yb = sorted((h1.a[1], h2.a[1]))
                left, right = has_v(h1.a[0], ya, yb), has_v(h1.b[0], ya, yb)
                if left and right:
                    box.update({id(h1), id(h2), id(left[0]), id(right[0])})
    return box


def build(page, area):
    wires, graphics, frames, texts, images = load(page, area)
    wires = merge_collinear(wires)
    box = rectangles(wires)
    graphics += [Graphic([w.a, w.b], STYLE) for w in wires if id(w) in box]
    wires = [w for w in wires if id(w) not in box]

    def touches_graphic(p):
        return any(g.bbox[0] - TOL_G <= p[0] <= g.bbox[2] + TOL_G and g.bbox[1] - TOL_G <= p[1] <= g.bbox[3] + TOL_G
                   and g.dist(p) <= TOL_G for g in graphics)

    def touches_wire(p, me):
        return any(w is not me and (math.dist(p, w.a) < TOL or math.dist(p, w.b) < TOL or w.interior(p)) for w in wires)

    # Leitungen mit freiem Ende (Kabellinien, Schirme, Striche) sind Grafik, bis nichts mehr wechselt
    changed = True
    while changed:
        changed = False
        keep = []
        for w in wires:
            if any(not touches_wire(p, w) and not touches_graphic(p) for p in (w.a, w.b)):
                graphics.append(Graphic([w.a, w.b], DASHED if w.dashed else STYLE))
                changed = True
            else:
                keep.append(w)
        wires = keep

    # Leitungen an Knoten aufteilen: Ende einer Leitung oder einer Abzweig-Schräge im Inneren einer anderen
    points = [p for w in wires for p in (w.a, w.b)]
    for g in graphics:
        if not g.filled and len(g.pts) == 2 and math.dist(*g.pts) < 12 and abs(g.pts[0][0] - g.pts[1][0]) > 0.05 \
                and abs(g.pts[0][1] - g.pts[1][1]) > 0.05:
            points += g.pts
    split = True
    while split:
        split = False
        for p in points:
            hit = next((w for w in wires if w.interior(p)), None)
            if hit:
                wires.remove(hit)
                wires += [Wire(hit.a, p, hit.dashed), Wire(p, hit.b, hit.dashed)]
                split = True

    # Grafik zu Bauteilen bündeln (Union-Find)
    n = len(graphics)
    parent = list(range(n))

    def find(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i
    order = sorted(range(n), key=lambda i: graphics[i].bbox[0])
    for k, i in enumerate(order):
        gi = graphics[i]
        for j in order[k + 1:]:
            gj = graphics[j]
            if gj.bbox[0] - TOL_C > gi.bbox[2]:
                break
            if find(i) != find(j) and gi.near(gj, TOL_C):
                parent[find(i)] = find(j)
    groups = {}
    for i in range(n):
        groups.setdefault(find(i), []).append(graphics[i])
    clusters = list(groups.values())
    cbox = []
    for c in clusters:
        cbox.append((min(g.bbox[0] for g in c), min(g.bbox[1] for g in c), max(g.bbox[2] for g in c), max(g.bbox[3] for g in c)))

    # Texte im Kasten eines Bauteils gehören dazu (kleinster passender Kasten), alle anderen bleiben frei
    ctexts = [[] for _ in clusters]
    free_texts = []
    for t in texts:
        bb = t["bbox"]
        cx, cy = (bb[0] + bb[2]) / 2, (bb[1] + bb[3]) / 2
        best = None
        for i, b in enumerate(cbox):
            if b[0] <= cx <= b[2] and b[1] <= cy <= b[3] and (b[2] - b[0]) > 8 and (b[3] - b[1]) > 8:
                a_ = (b[2] - b[0]) * (b[3] - b[1])
                if best is None or a_ < best[1]:
                    best = (i, a_)
        (ctexts[best[0]] if best else free_texts).append(t)

    def cluster_at(p):
        for i, c in enumerate(clusters):
            b = cbox[i]
            if b[0] - TOL_G <= p[0] <= b[2] + TOL_G and b[1] - TOL_G <= p[1] <= b[3] + TOL_G \
                    and any(g.dist(p) <= TOL_G for g in c):
                return i
        return None

    def direction(p, w):
        q = w.b if math.dist(p, w.a) < TOL else w.a
        if w.horiz:
            return "e" if q[0] > p[0] else "w"
        return "s" if q[1] > p[1] else "n"

    pins = {i: [] for i in range(len(clusters))}        # Bauteil -> [(Punkt, Richtung)]
    nodes = []                                          # Ecken/Abzweige ohne Grafik: (Punkt, Richtungen)
    ends = {}                                           # (id(Leitung), Ende) -> Bezug
    at = {}
    for w in wires:
        for e, p in enumerate((w.a, w.b)):
            at.setdefault((round(p[0], 1), round(p[1], 1)), []).append((w, e, p))
    for lst in at.values():
        p = lst[0][2]
        ci = cluster_at(p)
        if ci is not None:
            for w, e, q in lst:
                d = direction(q, w)
                pins[ci].append((p, d))
                ends[(id(w), e)] = ("c", ci, p, d)
        else:
            k = len(nodes)
            dirs = []
            for w, e, q in lst:
                d = direction(q, w)
                dirs.append(d)
                ends[(id(w), e)] = ("k", k, d)
            nodes.append((p, dirs))
    return wires, clusters, cbox, ctexts, free_texts, pins, nodes, ends, frames, images


def fuellung(rgb):
    """PDF-Füllfarbe auf eine QET-Füllung abbilden (Listenköpfe sind hellgrau, Symbole schwarz)."""
    if not rgb:
        return "black"
    hell = sum(rgb) / len(rgb)
    if hell > 0.97:
        return "white"
    if hell > 0.75:
        return "lightgray"
    if hell > 0.35:
        return "gray"
    return "black"


def add_text(e, t, T):
    x0, y0, x1, y1 = t["bbox"]
    d = t["dir"]
    rot = -math.degrees(math.atan2(d[1], d[0]))
    size = max(4, round(t["size"] * S) - 1)
    n = len(t["t"])
    room = (x1 - x0 if abs(rot) < 1 else y1 - y0) * S
    if n and any(ch.isalnum() for ch in t["t"]) and 0.68 * size * n > room:
        size = max(4, int(room / (0.68 * n)))
    base = (x0, y1 - 0.2 * t["size"]) if abs(rot) < 1 else (x1 - 0.2 * t["size"], y1)
    tx, ty = T(base)
    e.text(tx, ty, t["t"], size=size, rot=-rot, bold=t["bold"])


def write_elements(clusters, cbox, ctexts, pins, ordner, ziel, name):
    """Bauteil-Dateien schreiben; je Bauteil Datei, Ursprung (pt) und Anschluss-Index je (x, y, Richtung)."""
    out = []
    levels = [v for v in ordner.split("/") if v]
    for i, c in enumerate(clusters):
        plist = sorted(set((round(p[0], 2), round(p[1], 2), d) for p, d in pins[i]))
        ox, oy = (plist[0][0], plist[0][1]) if plist else (cbox[i][0], cbox[i][1])

        def T(q, ox=ox, oy=oy):
            return ((q[0] - ox) * S, (q[1] - oy) * S)
        e = Elem(f"{name} Teil {i + 1}", "-", info("", "", "", "aus dem Plan-PDF übernommen"))
        for g in c:
            if len(g.pts) > 2 and (g.filled or g.style == DASHED):
                q = [T(p) for p in g.pts]
                e._box(*q)
                style = STYLE.replace("filling:none", f"filling:{fuellung(g.fill)}") if g.filled else DASHED
                e.parts.append(("polygon", {**{f"x{k + 1}": f"{x:.2f}" for k, (x, _) in enumerate(q)},
                                            **{f"y{k + 1}": f"{y:.2f}" for k, (_, y) in enumerate(q)},
                                            "closed": "true" if g.filled else "false", "antialias": "true",
                                            "style": style}))
            else:
                for a, b in zip(g.pts, g.pts[1:]):
                    (x1, y1), (x2, y2) = T(a), T(b)
                    if math.hypot(x2 - x1, y2 - y1) > 0.05:
                        e.line(x1, y1, x2, y2, STYLE if g.style == "fill" else g.style)
        for t in ctexts[i]:
            add_text(e, t, T)
        terms, key = [], {}
        for x, y, d in plist:
            tx, ty = T((x, y))
            q = (round(tx + OFF[d][0], 1), round(ty + OFF[d][1], 1), d)
            key[(x, y, d)] = q
            terms.append(q)
        terms = sorted(set(terms), key=lambda q: (q[1], q[0]))
        for k, (tx, ty, d) in enumerate(terms):
            e.term(tx, ty, d, f"P{k + 1}")
        if not terms:
            e.term(0, 0, "n", "P1")           # Bauteil ohne Leitung bekommt einen Anschluss, sonst lehnt QET ab
        path = e.write(Path(ziel), levels, f"{name} Teil {i + 1}")
        out.append({"file": Path(path).name, "origin": [ox, oy],
                     "idx": {k: terms.index(v) for k, v in key.items()}})
    return out


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    ap = argparse.ArgumentParser()
    ap.add_argument("pdf")
    ap.add_argument("seite", type=int, help="Seitenindex ab 0")
    ap.add_argument("--ziel", required=True)
    ap.add_argument("--ordner", required=True)
    ap.add_argument("--name", default="Blatt")
    ap.add_argument("--json", required=True)
    ap.add_argument("--bereich", default="4,33,1191,800", help="Inhaltsbereich x0,y0,x1,y1 (pt)")
    a = ap.parse_args()
    page = pymupdf.open(a.pdf)[a.seite]
    area = tuple(map(float, a.bereich.split(",")))
    wires, clusters, cbox, ctexts, free_texts, pins, nodes, ends, frames, images = build(page, area)
    elems = write_elements(clusters, cbox, ctexts, pins, a.ordner, a.ziel, a.name)
    data = {"ordner": a.ordner, "elements": [{"file": el["file"], "origin": el["origin"]} for el in elems],
            "nodes": [{"p": list(p), "dirs": dirs} for p, dirs in nodes], "wires": [], "texts": free_texts,
            "frames": frames, "images": images}
    for w in wires:
        ends_w = []
        for e, p in enumerate((w.a, w.b)):
            ref = ends[(id(w), e)]
            if ref[0] == "c":
                _, ci, q, d = ref
                ends_w.append({"element": ci, "terminal": elems[ci]["idx"][(round(q[0], 2), round(q[1], 2), d)]})
            else:
                ends_w.append({"node": ref[1], "dir": ref[2]})
        data["wires"].append({"a": list(w.a), "b": list(w.b), "dashed": w.dashed, "ends": ends_w})
    Path(a.json).write_text(json.dumps(data, ensure_ascii=False), encoding="utf-8")
    print(f"{len(elems)} Bauteile, {len(nodes)} Knoten, {len(wires)} Leitungen, {len(free_texts)} Texte, "
          f"{len(frames)} Rahmenlinien, {len(images)} Bilder")


if __name__ == "__main__":
    main()
