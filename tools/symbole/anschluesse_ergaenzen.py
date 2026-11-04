"""Anschlüsse in importierte Herstellersymbole ergänzen.

Die Einpolig-/Netzwerkmakros aus den EPLAN-DXF-Daten (z. B. Siemens) zeichnen jeden Port als kleinen Kreis mit einer kurzen
Anschlusslinie, enthalten aber keine QElectroTech-Anschlüsse – man kann keine Leitung anschließen. Dieses Werkzeug lässt
die Grafik des Herstellers unverändert und setzt an das freie Ende jeder Anschlusslinie einen Anschluss. Der Name wird aus
den Texten über dem Kreis gebildet (z. B. „X1“ und „P1“ -> „X1:P1“).

    python anschluesse_ergaenzen.py SYMBOL.elmt [...]            # schreibt SYMBOL_Plan.elmt daneben
    python anschluesse_ergaenzen.py SYMBOL.elmt --nur-zeigen     # nur anzeigen, was ergänzt würde
"""
import argparse
import re
import uuid
import xml.etree.ElementTree as ET
from pathlib import Path

KREIS = 5.0          # Durchmesser der Port-Kreise in den Siemens-Makros (px)
TOL = 0.6


def ports(d):
    """(Mitte x, y, Anschluss x, y, Richtung) je Port-Kreis mit anliegender Linie."""
    out = []
    linien = [tuple(float(l.get(k)) for k in ("x1", "y1", "x2", "y2")) for l in d.iter("line")]
    for e in d.iter("ellipse"):
        w, h = float(e.get("width")), float(e.get("height"))
        if abs(w - KREIS) > TOL or abs(h - KREIS) > TOL:
            continue
        cx, cy, r = float(e.get("x")) + w / 2, float(e.get("y")) + h / 2, w / 2
        for x1, y1, x2, y2 in linien:
            for (ax, ay), (bx, by) in (((x1, y1), (x2, y2)), ((x2, y2), (x1, y1))):
                if abs(((ax - cx) ** 2 + (ay - cy) ** 2) ** 0.5 - r) < TOL:
                    richtung = "s" if by > ay + TOL else "n" if by < ay - TOL else "e" if bx > ax else "w"
                    out.append((cx, cy, bx, by, richtung))
                    break
            else:
                continue
            break
    return out


def name(d, cx, cy):
    """Bezeichnung aus den Texten über dem Kreis: Stecker (X1) vor Port (P1), „Ethernet“ weglassen."""
    teile = []
    for t in d.findall("text"):                     # nur feste Texte, nicht die <text> in dynamic_text
        x, y, s = float(t.get("x")), float(t.get("y")), t.get("text", "").strip()
        if abs(x - cx) < 8 and cy - 20 < y < cy and s and s.lower() != "ethernet":
            teile.append((y, s))
    teile.sort()
    return ":".join(s for _, s in teile) or f"P{round(cx)}"


def ergaenzen(pfad: Path, nur_zeigen=False):
    baum = ET.parse(pfad)
    d = baum.getroot()
    beschr = d.find("description")
    if beschr.find("terminal") is not None:
        print(f"{pfad.name}: hat schon Anschlüsse, unverändert")
        return None
    # Platzhalter der Herstellerdaten („…“ als Benutzertext) zeigen in QET nur Punkte an
    for dt in list(beschr.findall("dynamic_text")):
        if dt.get("text_from") == "UserText" and (dt.findtext("text") or "").strip() in ("…", "..."):
            beschr.remove(dt)
    gefunden = ports(beschr)
    for cx, cy, x, y, r in gefunden:
        n = name(beschr, cx, cy)
        print(f"{pfad.name}: {n} bei {x:g}/{y:g} ({r})")
        if not nur_zeigen:
            ET.SubElement(beschr, "terminal", {"name": n, "orientation": r, "type": "Generic",
                                               "uuid": "{%s}" % uuid.uuid4(), "x": f"{x:g}", "y": f"{y:g}"})
    if nur_zeigen or not gefunden:
        return None
    ziel = pfad.with_name(pfad.stem + "_Plan.elmt")
    d.find("uuid").set("uuid", "{%s}" % uuid.uuid5(uuid.NAMESPACE_URL, ziel.as_posix()))
    for n in d.iter("name"):
        n.text = re.sub(r"\s*$", " (Plan)", n.text or "")
    info = d.find("informations")
    if info is not None:
        info.text = (info.text or "") + "; Anschlüsse ergänzt (tools/symbole/anschluesse_ergaenzen.py)"
    baum.write(ziel, encoding="utf-8", xml_declaration=False)
    return ziel


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("symbole", nargs="+", type=Path)
    ap.add_argument("--nur-zeigen", action="store_true")
    a = ap.parse_args()
    for p in a.symbole:
        z = ergaenzen(p, a.nur_zeigen)
        if z:
            print("->", z)
