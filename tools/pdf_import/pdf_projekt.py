# SPDX-License-Identifier: GPL-2.0-or-later
"""Einen ganzen Schaltplan aus einem Vektor-PDF eines fremden CAD (SEE Electrical, EPLAN,
WSCAD, ...) übernehmen: alle Seiten auf einmal, nicht nur ein Blatt.

Je Seite setzt tools/symbole/pdf_blatt.py die Zeichnung um (Leitungen werden echte Leiter,
zusammenhängende Grafik wird Bauteil mit Anschlüssen an den Leitungsenden). Dieser Lauf
legt darüber, was ein Projekt braucht:

- die **Leiterstärke** des Dokuments, aus der häufigsten Strichstärke langer waagerechter
  und senkrechter Linien (jedes CAD zeichnet Leiter anders dick), statt fester EPLAN-Werte;
- **Schaltplanseiten** von Deckblatt, Inhaltsverzeichnis und Listen getrennt: eine Seite
  mit zu wenigen Leitern wird übersprungen;
- **Blattnummer und Titel** aus dem Schriftfeld (unterer Rand), ein Titelpräfix wie
  "Q1.1 Einspeisung" als Ort +Q1.1;
- die **Farbe** jedes Leiters aus der Zeichenfarbe als Kürzel nach IEC 60757 (BU, RD, ...),
  gestrichelt als GNYE;
- die **Drahtnummer** aus dem Text, der am Leiter steht (200.01, L1-1.1);
- das **BMK** jedes Bauteils aus seinen Texten (-K1, =A1+Q1-2K1);
- **Unterbrechungspunkte**: Leitungsenden mit einem Verweis Blatt.Spalte (/12.3) daneben,
  über Namen oder gegenseitigen Verweis gepaart.

Was bleibt (wie bei jedem Import aus einer Zeichnung): ob ein Bauteil Spule oder Kontakt
ist, weiß die Grafik nicht; Kontaktspiegel entstehen erst mit den Verknüpfungen. Ein
gescanntes PDF (Bild) geht nicht.

    python pdf_projekt.py plan.pdf --nur-analyse          # Leiterstärke und Seitenarten
    python pdf_projekt.py plan.pdf --ziel SAMMLUNG --ordner "Import/Plan" --aus OUT

Ausgabe: je Schaltplanseite OUT/seite_<n>.json (wie pdf_blatt.py, dazu "page", je Leiter
"colour" und "number", je Bauteil "label") und OUT/projekt.json mit Seiten, übersprungenen
Seiten, Unterbrechungspunkten und Paaren.
"""
import argparse
import collections
import json
import math
import re
import sys
from pathlib import Path

import pymupdf

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "symbole"))
import pdf_blatt  # noqa: E402

# Kürzel nach IEC 60757 und die Zeichenfarbe, die ein CAD dafür nimmt (RGB 0..1)
IEC60757 = [
    ("BK", (0.0, 0.0, 0.0)), ("BN", (0.6, 0.3, 0.0)), ("RD", (1.0, 0.0, 0.0)),
    ("OG", (1.0, 0.5, 0.0)), ("YE", (1.0, 1.0, 0.0)), ("GN", (0.0, 0.6, 0.0)),
    ("BU", (0.0, 0.0, 1.0)), ("VT", (0.5, 0.0, 0.5)), ("GY", (0.5, 0.5, 0.5)),
    ("WH", (1.0, 1.0, 1.0)), ("PK", (1.0, 0.6, 0.7)), ("TQ", (0.0, 0.8, 0.8)),
]
LABEL_RE = re.compile(r"^(?:[=+][\w.]+)*-\d*[A-Z]{1,3}\d+[A-Za-z]?(?:[.:]\d+)*$")
WIRE_NO_RE = re.compile(r"^(?:\d+[.\-/]\d+|L\d(?:-\d+(?:\.\d+)*)?|N|PE|\+?\d+V\w*|0V|M)$")
REF_RE = re.compile(r"^/?(\d+)[./](\d+)$")
LOC_RE = re.compile(r"^([A-Z]+\d+(?:\.\d+)*)\s+(.+)$")


def colour_code(rgb):
    if not rgb:
        return ""
    best, dist = "", 9.0
    for code, ref in IEC60757:
        d = sum((a - b) ** 2 for a, b in zip(rgb, ref))
        if d < dist:
            best, dist = code, d
    return best if dist < 0.12 else ""


def wire_width(doc, pages):
    """Die häufigste Strichstärke langer waagerechter und senkrechter Linien, ±15 %."""
    counts = collections.Counter()
    for i in pages:
        for p in doc[i].get_drawings():
            w = round(p.get("width") or 0, 2)
            if not 0.1 <= w <= 3:
                continue
            for item in p["items"]:
                if item[0] != "l":
                    continue
                a, b = item[1], item[2]
                if (abs(a.x - b.x) < 0.05 or abs(a.y - b.y) < 0.05) and math.dist((a.x, a.y), (b.x, b.y)) > 20:
                    counts[w] += 1
    if not counts:
        return None
    width = counts.most_common(1)[0][0]
    return (width * 0.85, width * 1.15)


def segment_colours(page):
    """Zeichenfarbe je Linie, nach ihren gerundeten Enden."""
    out = {}
    for p in page.get_drawings():
        for item in p["items"]:
            if item[0] == "l":
                a, b = item[1], item[2]
                key = tuple(sorted([(round(a.x, 1), round(a.y, 1)), (round(b.x, 1), round(b.y, 1))]))
                out[key] = p.get("color")
    return out


NUMBER_RE = re.compile(r"\d+(?:\.\d+)?[a-z]?")


def low_texts(page, texts, share):
    h = page.rect.height
    return [t for t in texts if t["bbox"][1] > h * (1 - share)]


def title_block(page, texts, share, common=frozenset()):
    """Blattnummer, Titel und Ort aus dem Schriftfeld: den Texten im unteren Teil. Die
    Blattnummer ist eine Zahl dort, die nicht auf vielen Seiten gleich steht (@p common:
    Zeichnungsnummer, Datum, Revision)."""
    low = low_texts(page, texts, share)
    number = ""
    for t in sorted(low, key=lambda t: (-t["bbox"][0], -t["size"])):
        if NUMBER_RE.fullmatch(t["t"]) and t["t"] not in common:
            number = t["t"]
            break
    words = [t for t in low if len(t["t"]) > 3 and not re.fullmatch(r"[\d./:\-]+", t["t"])]
    title = max(words, key=lambda t: (t["size"], len(t["t"])))["t"] if words else ""
    location = ""
    m = LOC_RE.match(title)
    if m:
        location, title = "+" + m.group(1), m.group(2)
    return number, title, location


def near(text, p, tol):
    x0, y0, x1, y1 = text["bbox"]
    return x0 - tol <= p[0] <= x1 + tol and y0 - tol <= p[1] <= y1 + tol


def centre(text):
    x0, y0, x1, y1 = text["bbox"]
    return ((x0 + x1) / 2, (y0 + y1) / 2)


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("pdf")
    ap.add_argument("--ziel", help="Sammlung, in die die Bauteile kommen")
    ap.add_argument("--ordner", default="Import/PDF", help="Unterordner in der Sammlung")
    ap.add_argument("--aus", help="Ordner für die JSON-Dateien")
    ap.add_argument("--seiten", help="nur diese Seiten (ab 1), z. B. 3-10,12")
    ap.add_argument("--leiter", help="Leiterstärke von-bis in pt, z. B. 0.45-0.6 (sonst erkannt)")
    ap.add_argument("--min-leiter", type=int, default=3, help="weniger Leiter: keine Schaltplanseite")
    ap.add_argument("--rand", type=float, default=8.0, help="Abstand des Inhalts vom Blattrand (pt)")
    ap.add_argument("--schriftfeld", type=float, default=0.13, help="Anteil der Blatthöhe unten für das Schriftfeld")
    ap.add_argument("--nur-analyse", action="store_true", help="nichts schreiben, nur Seiten auswerten")
    a = ap.parse_args()

    doc = pymupdf.open(a.pdf)
    pages = list(range(len(doc)))
    if a.seiten:
        pages = []
        for part in a.seiten.split(","):
            lo, _, hi = part.partition("-")
            pages += list(range(int(lo) - 1, min(int(hi or lo), len(doc))))
    if a.leiter:
        lo, hi = (float(v) for v in a.leiter.split("-"))
        pdf_blatt.WIRE_W = (lo, hi)
    else:
        found = wire_width(doc, pages)
        if not found:
            sys.exit("keine Vektorlinien gefunden: gescanntes PDF?")
        pdf_blatt.WIRE_W = found
    print(f"Leiterstärke {pdf_blatt.WIRE_W[0]:.2f}–{pdf_blatt.WIRE_W[1]:.2f} pt")
    write = not a.nur_analyse
    if write and not (a.ziel and a.aus):
        sys.exit("--ziel und --aus fehlen (oder --nur-analyse)")
    out = Path(a.aus) if a.aus else None
    if out and write:
        out.mkdir(parents=True, exist_ok=True)

    # Zahlen, die im Schriftfeld vieler Seiten gleich stehen, sind keine Blattnummer
    seen = collections.Counter()
    for i in pages:
        r = doc[i].rect
        low = low_texts(doc[i], pdf_blatt.load(doc[i], (r.x0 + a.rand, r.y0 + a.rand, r.x1 - a.rand,
                                                        r.y1 - a.rand))[3], a.schriftfeld)
        seen.update({t["t"] for t in low if NUMBER_RE.fullmatch(t["t"])})
    common = frozenset(v for v, n in seen.items() if n > max(2, len(pages) // 3))

    project = {"pdf": Path(a.pdf).name, "wire_width": list(pdf_blatt.WIRE_W), "pages": [], "skipped": [],
               "reports": [], "pairs": []}
    for i in pages:
        page = doc[i]
        r = page.rect
        area = (r.x0 + a.rand, r.y0 + a.rand, r.x1 - a.rand, r.y1 - a.rand)
        texts = pdf_blatt.load(page, area)[3]
        number, title, location = title_block(page, texts, a.schriftfeld, common)
        drawing = (area[0], area[1], area[2], r.y0 + r.height * (1 - a.schriftfeld))
        try:
            wires, clusters, cbox, ctexts, free_texts, pins, nodes, ends, frames, images = pdf_blatt.build(page, drawing)
        except Exception as exc:     # eine kaputte Seite hält den Lauf nicht an
            project["skipped"].append({"page": i + 1, "why": f"Fehler: {exc}"})
            continue
        if len(wires) < a.min_leiter:
            project["skipped"].append({"page": i + 1, "why": f"{len(wires)} Leiter", "title": title})
            print(f"Seite {i + 1}: übersprungen ({len(wires)} Leiter) {title!r}")
            continue

        colours = segment_colours(page)
        wire_info = []
        for w in wires:
            key = tuple(sorted([(round(w.a[0], 1), round(w.a[1], 1)), (round(w.b[0], 1), round(w.b[1], 1))]))
            code = "GNYE" if w.dashed else colour_code(colours.get(key))
            number_text = ""
            for t in free_texts:
                if WIRE_NO_RE.match(t["t"]) and pdf_blatt.dist_ps(centre(t), w.a, w.b) < 6:
                    number_text = t["t"]
                    break
            wire_info.append({"colour": code, "number": number_text})
        labels = [next((t["t"] for t in tl if LABEL_RE.match(t["t"])), "") for tl in ctexts]

        # Verweise an Leitungsenden: Unterbrechungspunkte
        for w in wires:
            for p in (w.a, w.b):
                ref = next((t for t in free_texts if REF_RE.match(t["t"]) and near(t, p, 14)), None)
                if not ref:
                    continue
                m = REF_RE.match(ref["t"])
                name = next((t["t"] for t in free_texts if t is not ref and WIRE_NO_RE.match(t["t"])
                             and near(t, p, 14)), "")
                project["reports"].append({"page": i + 1, "number": number or str(i + 1), "at": list(p),
                                           "name": name, "target_page": m.group(1), "target_column": m.group(2)})

        entry = {"page": i + 1, "number": number or str(i + 1), "title": title, "location": location,
                 "wires": len(wires), "devices": len(clusters),
                 "coloured": sum(1 for x in wire_info if x["colour"] not in ("", "BK")),
                 "numbered": sum(1 for x in wire_info if x["number"]), "labels": [x for x in labels if x]}
        if write:
            elems = pdf_blatt.write_elements(clusters, cbox, ctexts, pins, f"{a.ordner}/Seite {entry['number']}",
                                             a.ziel, f"Seite {entry['number']}")
            data = {"page": entry,
                    "elements": [{"file": el["file"], "origin": el["origin"], "label": labels[k]}
                                 for k, el in enumerate(elems)],
                    "nodes": [{"p": list(p), "dirs": dirs} for p, dirs in nodes], "wires": [],
                    "texts": free_texts, "frames": frames, "images": images}
            for w, info in zip(wires, wire_info):
                ends_w = []
                for e, p in enumerate((w.a, w.b)):
                    ref = ends[(id(w), e)]
                    if ref[0] == "c":
                        _, ci, q, d = ref
                        ends_w.append({"element": ci,
                                       "terminal": elems[ci]["idx"][(round(q[0], 2), round(q[1], 2), d)]})
                    else:
                        ends_w.append({"node": ref[1], "dir": ref[2]})
                data["wires"].append({"a": list(w.a), "b": list(w.b), "dashed": w.dashed, "ends": ends_w,
                                      "colour": info["colour"], "number": info["number"]})
            (out / f"seite_{i + 1}.json").write_text(json.dumps(data, ensure_ascii=False), encoding="utf-8")
            entry["json"] = f"seite_{i + 1}.json"
        project["pages"].append(entry)
        print(f"Seite {i + 1}: Blatt {entry['number']} {title!r} {location} – {len(wires)} Leiter "
              f"({entry['numbered']} mit Nummer, {entry['coloured']} farbig), {len(clusters)} Bauteile, "
              f"{len(entry['labels'])} mit BMK")

    # Unterbrechungspunkte paaren: gleicher Name, oder sie zeigen aufeinander
    reps = project["reports"]
    used = set()
    for x, rx in enumerate(reps):
        if x in used:
            continue
        for y in range(x + 1, len(reps)):
            ry = reps[y]
            if y in used or ry["page"] == rx["page"]:
                continue
            same_name = rx["name"] and rx["name"] == ry["name"]
            points_at = rx["target_page"] == ry["number"] and ry["target_page"] == rx["number"]
            if same_name or points_at:
                project["pairs"].append([x, y])
                used |= {x, y}
                break
    numbers = {p["number"] for p in project["pages"]}
    for k, rep in enumerate(reps):
        rep["paired"] = k in used
        rep["target_missing"] = rep["target_page"] not in numbers
    print(f"{len(project['pages'])} Schaltplanseiten, {len(project['skipped'])} übersprungen, "
          f"{len(reps)} Unterbrechungspunkte, {len(project['pairs'])} Paare, "
          f"{len(reps) - len(used)} ohne Gegenstück")
    if out and write:
        (out / "projekt.json").write_text(json.dumps(project, ensure_ascii=False, indent=1), encoding="utf-8")


if __name__ == "__main__":
    main()
