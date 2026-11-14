"""Katalog der Fluidsymbole als PDF: je Symbol die Zeichnung und alle Daten (für die Neuzeichnung).

Je Symbol: Zeichnung mit Anschlusspunkten und -namen (rot), deutscher Name, Datei, Art und Funktion, Anschlüsse
(Name, Lage, Richtung, Medium), bei Wegeventilen die Stellungen mit Schaltwegen und die Betätigung, bei Zylindern
die Maße, dazu die Kennwerte für die Simulation.

    python tools/symbole/fluid_katalog.py bauteile/fluid Katalog.pdf
"""
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

import pymupdf

ART = {"valve": "Wegeventil", "cylinder": "Zylinder", "source": "Versorgung/Quelle", "treatment": "Aufbereitung",
       "flow_control": "Strom-/Sperrventil", "logic_valve": "Logikventil", "sensor": "Messgerät/Sensor",
       "other": "Sonstiges", "solenoid": "Ventilmagnet"}
RICHTUNG = {"n": "oben", "s": "unten", "e": "rechts", "w": "links"}
BEREICH = {"30_hydraulic": "Hydraulik", "50_pneumatic": "Pneumatik"}


def zahlen(e, *k):
    return [float(e.get(x, 0)) for x in k]


def grenzen(desc):
    xs, ys = [], []
    for e in desc:
        t = e.tag
        if t == "line":
            x1, y1, x2, y2 = zahlen(e, "x1", "y1", "x2", "y2"); xs += [x1, x2]; ys += [y1, y2]
        elif t in ("rect", "ellipse", "arc"):
            x, y, w, h = zahlen(e, "x", "y", "width", "height"); xs += [x, x + w]; ys += [y, y + h]
        elif t == "polygon":
            i = 1
            while e.get(f"x{i}") is not None:
                xs.append(float(e.get(f"x{i}"))); ys.append(float(e.get(f"y{i}"))); i += 1
        elif t == "terminal":
            xs.append(float(e.get("x"))); ys.append(float(e.get("y")))
    return (min(xs), min(ys), max(xs), max(ys)) if xs else None


def zeichne(page, desc, rect):
    g = grenzen(desc)
    if not g:
        return
    bx0, by0, bx1, by1 = g
    s = min(rect.width / max(bx1 - bx0, 1), rect.height / max(by1 - by0, 1), 2.2)
    ox = rect.x0 + (rect.width - (bx1 - bx0) * s) / 2
    oy = rect.y0 + (rect.height - (by1 - by0) * s) / 2
    tf = lambda x, y: pymupdf.Point(ox + (x - bx0) * s, oy + (y - by0) * s)
    for e in desc:
        t = e.tag
        stil = e.get("style") or ""
        fill = (0, 0, 0) if "filling:black" in stil else None
        sh = page.new_shape()
        if t == "line":
            sh.draw_line(tf(*zahlen(e, "x1", "y1")), tf(*zahlen(e, "x2", "y2")))
        elif t in ("rect", "ellipse", "arc"):
            x, y, w, h = zahlen(e, "x", "y", "width", "height")
            r = pymupdf.Rect(tf(x, y), tf(x + w, y + h))
            sh.draw_rect(r) if t == "rect" else sh.draw_oval(r)
        elif t == "polygon":
            p, i = [], 1
            while e.get(f"x{i}") is not None:
                p.append(tf(float(e.get(f"x{i}")), float(e.get(f"y{i}")))); i += 1
            if e.get("closed") == "true":
                p.append(p[0])
            sh.draw_polyline(p)
        else:
            continue
        sh.finish(color=(0, 0, 0), fill=fill, width=0.5, dashes="[2 1.5] 0" if "dash" in stil else None)
        sh.commit()
    for e in desc:
        if e.tag == "terminal":
            q = tf(float(e.get("x")), float(e.get("y")))
            page.draw_circle(q, 1.0, color=(0.8, 0, 0), fill=(0.8, 0, 0))
            if e.get("name"):
                page.insert_text(pymupdf.Point(q.x + 1.5, q.y - 1.5), e.get("name"), fontsize=5.5, color=(0.8, 0, 0))


def texte(d, rel):
    name = next((x.text for x in d.iter("name") if x.get("lang") == "de"), "") or ""
    zeilen = [("b", name), ("g", rel)]
    fl = d.find("fluid")
    if fl is None:
        zeilen.append(("t", "Art: keine Fluiddaten"))
    else:
        zeilen.append(("t", f"Art: {ART.get(fl.get('kind'), fl.get('kind'))}   Funktion: {fl.get('function') or '-'}"
                       + (f"   Kennbuchstaben: {fl.get('prefix')}" if fl.get("prefix") else "")))
    anschl = [f"{t.get('name') or '?'} ({RICHTUNG.get(t.get('orientation'), t.get('orientation'))}, "
              f"{t.get('x')}|{t.get('y')})" for t in d.find("description").iter("terminal")]
    medium = next((t.get("medium") for t in d.iter("terminal") if t.get("medium")), "")
    zeilen.append(("t", f"Anschlüsse [{medium}]: " + ", ".join(anschl)))
    if fl is not None:
        pos = fl.find("positions")
        if pos is not None:
            ruhe = pos.get("initial", "1")
            for i, p in enumerate(pos.findall("position"), 1):
                wege = [f"{w.get('from')}→{w.get('to')}" if w.get("to") else f"{w.get('from')} gesperrt"
                        for w in p.findall("path")]
                zeilen.append(("t", f"Stellung {i}{' (Ruhe)' if str(i) == ruhe else ''}: "
                               + (", ".join(wege) or "keine Schaltwege")))
            for a in fl.findall("actuation"):
                teile = [a.get("types") or ""] + (["Feder"] if a.get("spring") == "true" else []) \
                    + (["vorgesteuert"] if a.get("pilot") == "true" else [])
                zeilen.append(("t", f"Betätigung {'links' if a.get('side') == 'left' else 'rechts'}: "
                               + (", ".join(x for x in teile if x) or "-")))
        z = fl.find("cylinder")
        if z is not None:
            zeilen.append(("t", "Zylinder: " + ", ".join(f"{k}={v}" for k, v in z.attrib.items())))
        kw = [f"{p.get('name')}={p.get('value')}{(' ' + p.get('unit')) if p.get('unit') else ''}"
              for p in fl.findall("parameter")]
        if kw:
            zeilen.append(("k", "Kennwerte: " + ", ".join(kw)))
    return zeilen


def maengel(d):
    """Zeichenfehler an den Anschlüssen: schräge Leitung, Leitung auf der Körperkante, freies Ende, ohne Namen,
    Anschluss nicht auf dem 10er-Raster, Wegeventil-Anschlüsse nicht am Ruhekästchen."""
    desc = d.find("description")
    terms = [(float(t.get("x")), float(t.get("y")), t.get("name") or "?") for t in desc if t.tag == "terminal"]
    rects = [tuple(float(e.get(k)) for k in ("x", "y", "width", "height")) for e in desc if e.tag == "rect"]
    linien = [tuple(float(e.get(a)) for a in ("x1", "y1", "x2", "y2")) for e in desc if e.tag == "line"]
    out = []
    schraeg, kante, frei = set(), set(), set()
    for tx, ty, n in terms:
        an = [l for l in linien if (abs(l[0] - tx) < .3 and abs(l[1] - ty) < .3) or (abs(l[2] - tx) < .3 and abs(l[3] - ty) < .3)]
        for x1, y1, x2, y2 in an:
            if abs(x1 - x2) > .5 and abs(y1 - y2) > .5:
                schraeg.add(n)
            for rx, ry, rw, rh in rects:
                if abs(x1 - x2) < .3 and (abs(x1 - rx) < .3 or abs(x1 - rx - rw) < .3) and min(y1, y2) < ry + rh and max(y1, y2) > ry + 1:
                    kante.add(n)
                if abs(y1 - y2) < .3 and (abs(y1 - ry) < .3 or abs(y1 - ry - rh) < .3) and min(x1, x2) < rx + rw and max(x1, x2) > rx + 1:
                    kante.add(n)
        if not an and not any(e.tag in ("polygon", "ellipse", "rect") for e in desc):
            frei.add(n)
    if schraeg:
        out.append("schräge Anschlussleitung bei " + ", ".join(sorted(schraeg)))
    if kante:
        out.append("Anschlussleitung liegt auf der Körperkante bei " + ", ".join(sorted(kante)))
    if terms and any(n == "?" for *_, n in terms):
        out.append("Anschlüsse ohne Namen")
    if any(abs(tx / 10 - round(tx / 10)) > .01 or abs(ty / 10 - round(ty / 10)) > .01 for tx, ty, _ in terms):
        out.append("Anschluss nicht auf dem 10er-Raster")
    fl = d.find("fluid")
    if fl is not None and fl.find("positions") is not None and rects:
        n = len(fl.find("positions").findall("position"))
        ruhe = int(fl.find("positions").get("initial", "1"))
        kx, _, kb, _ = max(rects, key=lambda r: r[2] * r[3])
        w = kb / n
        felder = {int((tx - kx) // w) + 1 for tx, ty, nm in terms if nm != "?"}
        if felder and felder != {ruhe}:
            out.append("Anschlüsse nicht am Kästchen der Ruhestellung")
        if not any(p.find("path") is not None for p in fl.find("positions").findall("position")):
            out.append("keine Schaltwege")
    return out


def main():
    ordner, ziel = Path(sys.argv[1]), sys.argv[2]
    dateien = sorted(ordner.rglob("*.elmt"))
    doc = pymupdf.open()
    # Mängelliste vorne
    liste = []
    for p in dateien:
        d = ET.parse(p).getroot()
        m = maengel(d)
        if m:
            name = next((x.text for x in d.iter("name") if x.get("lang") == "de"), p.name)
            liste.append((p.relative_to(ordner).as_posix().split("/")[0], name, "; ".join(m)))
    B, H, rand = 595, 842, 28
    seite, y = None, 0
    for i, (ber, name, m) in enumerate(liste):
        if seite is None or y > H - rand:
            seite = doc.new_page(width=B, height=H)
            seite.insert_text((rand, rand), f"Mängelliste – {len(liste)} Symbole neu zu zeichnen (Stand automatisch geprüft)",
                              fontsize=10, color=(0.75, 0, 0))
            y = rand + 18
        seite.insert_text((rand, y), f"{BEREICH.get(ber, ber)[:4]}  {name[:58]}", fontsize=6.5)
        seite.insert_text((rand + 235, y), m[:95], fontsize=6.2, color=(0.75, 0, 0))
        y += 9.2
    B, H, rand, pro_seite = 595, 842, 28, 6
    zh = (H - 2 * rand - 20) / pro_seite
    seite, k, bereich_alt = None, 0, None
    for p in dateien:
        rel = p.relative_to(ordner).as_posix()
        bereich = rel.split("/")[0]
        if seite is None or k >= pro_seite or bereich != bereich_alt:
            seite, k = doc.new_page(width=B, height=H), 0
            seite.insert_text((rand, rand), f"Fluidsymbole nach ISO 1219 – {BEREICH.get(bereich, bereich)}",
                              fontsize=10)
            seite.insert_text((B - rand - 40, rand), f"Seite {doc.page_count}", fontsize=7)
        bereich_alt = bereich
        d = ET.parse(p).getroot()
        y0 = rand + 12 + k * zh
        seite.draw_line((rand, y0), (B - rand, y0), color=(0.75, 0.75, 0.75), width=0.4)
        zeichne(seite, d.find("description"), pymupdf.Rect(rand + 4, y0 + 8, rand + 150, y0 + zh - 8))
        tx, ty = rand + 160, y0 + 11
        zeilen = texte(d, rel)
        m = maengel(d)
        if m:
            zeilen.insert(2, ("m", "MANGEL: " + "; ".join(m)))
        for art, text in zeilen:
            gr = {"b": 8.5, "g": 5.5}.get(art, 6.3)
            farbe = (0.45, 0.45, 0.45) if art == "g" else (0.8, 0, 0) if art == "m" else (0, 0, 0)
            breite = int((B - rand - tx) / (gr * 0.47))
            while text and ty < y0 + zh - 4:
                stueck, text = text[:breite], text[breite:]
                if text and " " in stueck:
                    cut = stueck.rfind(" ")
                    stueck, text = stueck[:cut], stueck[cut + 1:] + text
                seite.insert_text((tx, ty), stueck, fontsize=gr, color=farbe)
                ty += gr + 2.2
            if text:
                seite.insert_text((tx, ty - gr - 2.2), " …", fontsize=gr)
        k += 1
    doc.save(ziel, garbage=3, deflate=True)
    print(len(dateien), "Symbole,", doc.page_count, "Seiten")


if __name__ == "__main__":
    main()
