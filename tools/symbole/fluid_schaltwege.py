"""Schaltwege der Wegeventile aus FluidSIM 6 übernehmen und in die eigenen ISO-1219-Symbole eintragen.

FluidSIM setzt jedes Wegeventil aus Stellungsbausteinen zusammen (Symbolebenen `B<Anschlüsse>_<Nr>`, z. B. B4_1
„alles gesperrt“, B4_8 „gekreuzt“). Jeder Baustein besteht aus Linien mit Pfeil- oder Sperrende
(`end-cap="arrow1"` / `"ortho1"`), Polylinien und Verbindungspunkten. Daraus folgt eindeutig, welcher Anschluss
in welcher Stellung mit welchem verbunden ist und in welche Richtung. Die Anschlussnamen (P, A, B, T bzw. 1–5)
stehen an den Anschlusspunkten der Ebene `CON<n>`.

Übernommen werden nur diese Funktionsdaten, keine Grafik: Das Skript sucht zu jedem Ventil der Sammlung das
FluidSIM-Bauteil mit gleichem Namen, wählt bei mehreren Varianten (z. B. gespiegelt) die, deren Stellungen zur
gezeichneten Grafik passen (Anzahl schräger Linien je Kästchen), und schreibt die Wege als
`<path from=".." to=".."/>` (ohne `to`: gesperrt) in die vorhandenen `<position>`-Einträge.
Alles, was nicht eindeutig zuzuordnen ist, steht im Bericht und bleibt unverändert.

    python fluid_schaltwege.py --fluidsim "C:/Program Files (x86)/Didactic/FluidSIM6" bauteile/fluid --nur-zeigen
    python fluid_schaltwege.py --fluidsim ... bauteile/fluid --bericht bericht.csv
"""
import argparse
import csv
import gzip
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from fluidsim_symbole import Uebersetzung  # noqa: E402

# Reihenfolge für ungerichtete Verbindungen: von der Versorgung zum Verbraucher bzw. zum Ablass
RANG = {"P": 0, "1": 0, "X": 1, "Y": 1, "A": 2, "B": 2, "2": 2, "4": 2, "C": 2, "D": 2,
        "T": 4, "R": 4, "S": 4, "3": 4, "5": 4, "L": 5}


def rang(p):
    return RANG.get(p, 3), p


# ----------------------------------------------------------------------------------------------- FluidSIM

def baustein_wege(ebene, anschluesse):
    """Wege eines Stellungsbausteins. anschluesse: {(x, oben): Name} in Baustein-Koordinaten.
    Rückgabe: Liste (von, nach) und Menge gesperrter Anschlüsse."""
    segmente, pfeile, sperren = [], set(), set()
    for e in ebene:
        if e.tag == "line":
            x1, y1 = float(e.get("x1", 0)), float(e.get("y1", 0))
            x2, y2 = float(e.get("x2", e.get("x1", 0))), float(e.get("y2", e.get("y1", 0)))
            if e.get("end-cap") == "ortho1":
                sperren.add((x1, y1))
                continue
            segmente.append(((x1, y1), (x2, y2)))
            if e.get("end-cap") == "arrow1":
                pfeile.add((x2, y2))
            if e.get("start-cap") == "arrow1":
                pfeile.add((x1, y1))
        elif e.tag == "polyline":
            pk = [tuple(float(v) for v in p.split(",")) for p in e.get("points", "").split()]
            segmente += list(zip(pk, pk[1:]))

    def auf(p, s):
        (ax, ay), (bx, by) = s
        if min(ax, bx) - 1e-6 <= p[0] <= max(ax, bx) + 1e-6 and min(ay, by) - 1e-6 <= p[1] <= max(ay, by) + 1e-6:
            return abs((bx - ax) * (p[1] - ay) - (by - ay) * (p[0] - ax)) < 1e-6
        return False
    n = len(segmente)
    eltern = list(range(n))

    def wurzel(i):
        while eltern[i] != i:
            eltern[i] = eltern[eltern[i]]
            i = eltern[i]
        return i
    for i in range(n):
        for j in range(n):
            if i != j and any(auf(p, segmente[j]) for p in segmente[i]):
                eltern[wurzel(i)] = wurzel(j)

    def anschluss_an(p):
        if abs(abs(p[1]) - 12) > 0.5:
            return None
        return anschluesse.get((round(p[0]), p[1] < 0))
    gruppen = {}
    for i, s in enumerate(segmente):
        for p in s:
            a = anschluss_an(p)
            if a:
                gruppen.setdefault(wurzel(i), {})[a] = p
    wege = []
    for g in gruppen.values():
        if len(g) < 2:
            continue
        ziele = [a for a, p in g.items() if p in pfeile]
        quellen = [a for a in g if a not in ziele]
        if ziele and quellen:
            for q in sorted(quellen, key=rang):
                for z in sorted(ziele, key=rang):
                    wege.append((q, z))
        else:   # ungerichtet oder beidseitig: von der Versorgung aus, sonst zum Ablass hin
            namen = sorted(g, key=rang)
            if rang(namen[0])[0] == 0 or rang(namen[-1])[0] < 4:
                wege += [(namen[0], z) for z in namen[1:]]
            else:
                wege += [(q, namen[-1]) for q in namen[:-1]]
    verbunden = {a for w in wege for a in w}
    gesperrt = {anschluesse[(round(x), y < 0)] for x, y in sperren if (round(x), y < 0) in anschluesse}
    gesperrt |= set(anschluesse.values()) - verbunden
    return wege, gesperrt - verbunden


def schraeg_je_stellung(ebenen):
    out = []
    for ebene in ebenen:
        k = 0
        for e in ebene:
            if e.tag == "line" and e.get("end-cap") != "ortho1":
                x1, x2 = float(e.get("x1", 0)), float(e.get("x2", e.get("x1", 0)))
                if abs(x2 - x1) > 1:
                    k += 1
        out.append(k)
    return out


def fluidsim_ventile(fluidsim: Path):
    w = ET.fromstring(gzip.decompress((fluidsim / "sym" / "sym_fs.lib").read_bytes()))
    sprache = Uebersetzung(fluidsim / "db" / "lang.db")
    ventile = {}
    for f in w.iter("file"):
        comp = next((c for c in f.iter("component") if "Directional" in (c.get("object-ref") or "")), None)
        if comp is None:
            continue
        sym = next((y for y in f.iter("symbol") if y.get("ident") == comp.get("symbol-ref")), None)
        if sym is None:
            continue
        ebenen = [l for l in sym.findall("symbol-layer") if re.match(r"^B\d+_\d+$", l.get("ident") or "")]
        con = next((l for l in sym.findall("symbol-layer") if re.match(r"^CON\d+$", l.get("ident") or "")), None)
        if not ebenen or con is None:
            continue
        ebenen.sort(key=lambda l: float(l.get("ref-x", 0)))
        anschluesse = {(round(float(c.get("cx"))), float(c.get("cy")) < 0): c.get("designation") or c.get("ident")
                       for c in con.iter("connector")}
        werte = {c.get("ident"): c.get("value") for c in comp if c.tag in ("const", "parameter")}
        try:
            ruhe = int(float(werte.get("posNoInit", "0")))
        except ValueError:
            ruhe = 0
        stellungen = [baustein_wege(e, anschluesse) for e in ebenen]
        eig = f.find("section[@type='properties']")
        de, en = sprache.namen(eig.get("name", "")) if eig is not None else ("", "")
        ventile.setdefault(de.strip().lower(), []).append({
            "datei": f.get("name"), "de": de, "en": en, "stellungen": stellungen,
            "bausteine": [e.get("ident") for e in ebenen], "schraeg": schraeg_je_stellung(ebenen),
            "ruhe": ruhe, "anschluesse": sorted(set(anschluesse.values()))})
    return ventile


# ----------------------------------------------------------------------------------------------- Sammlung

def schraeg_in_grafik(desc, n):
    """Schräge Linien je Kästchen in der Grafik (größtes Rechteck = Ventilkörper, in n gleiche Felder)."""
    rects = [e for e in desc if e.tag == "rect"]
    if not rects:
        return None
    koerper = max(rects, key=lambda e: float(e.get("width")) * float(e.get("height")))
    x0, y0 = float(koerper.get("x")), float(koerper.get("y"))
    b, h = float(koerper.get("width")), float(koerper.get("height"))
    if b < h * n * 0.8:
        return None
    feld = b / n
    k = [0] * n
    fluss = [False] * n
    for e in desc:
        # gefüllte Pfeilspitze (kleines geschlossenes Dreieck) im Feld = dort ist ein Durchflussweg gezeichnet
        if e.tag == "polygon" and "filling:black" in (e.get("style") or "") and e.get("x4") is None:
            px = [float(e.get(f"x{i}")) for i in (1, 2, 3)]
            py = [float(e.get(f"y{i}")) for i in (1, 2, 3)]
            mx, my = sum(px) / 3, sum(py) / 3
            if x0 <= mx <= x0 + b and y0 <= my <= y0 + h and max(px) - min(px) < 8:
                i = int((mx - x0) // feld)
                if 0 <= i < n:
                    fluss[i] = True
        if e.tag != "line":
            continue
        x1, y1, x2, y2 = (float(e.get(a)) for a in ("x1", "y1", "x2", "y2"))
        if abs(x2 - x1) < 1 or abs(y2 - y1) < 1:
            continue
        mx, my = (x1 + x2) / 2, (y1 + y2) / 2
        if not (x0 <= mx <= x0 + b and y0 <= my <= y0 + h):
            continue
        i = int((mx - x0) // feld)
        if 0 <= i < n:
            k[i] += 1
    return k, fluss


def text_ersetzen(text, stellungen):
    """<positions>-Block im Dateitext ersetzen, Rest der Datei Byte für Byte unverändert lassen."""
    m = re.search(r"([ \t]*)<positions([^>]*)>(.*?)</positions>", text, re.S)
    if not m:
        return None
    einzug = m.group(1)
    zeilen = [f"{einzug}<positions{m.group(2)}>"]
    for wege, gesperrt in stellungen:
        teile = [f'<path from="{v}" to="{z}" />' for v, z in wege]
        teile += [f'<path from="{a}" />' for a in sorted(gesperrt, key=rang)]
        zeilen.append(f"{einzug}    <position>{''.join(teile)}</position>")
    zeilen.append(f"{einzug}</positions>")
    return text[:m.start()] + "\n".join(zeilen) + text[m.end():]


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--fluidsim", type=Path, required=True)
    ap.add_argument("ordner", type=Path)
    ap.add_argument("--nur-zeigen", action="store_true")
    ap.add_argument("--bericht", type=Path)
    a = ap.parse_args()
    fs = fluidsim_ventile(a.fluidsim)
    zeilen = []
    for datei in sorted(a.ordner.rglob("*.elmt")):
        text = datei.read_text(encoding="utf-8")
        d = ET.fromstring(text)
        fluid = d.find("fluid")
        if fluid is None or fluid.get("kind") != "valve" or fluid.find("positions") is None:
            continue
        n = len(fluid.find("positions").findall("position"))
        ruhe = int(fluid.find("positions").get("initial", "1"))
        name = next((x.text or "" for x in d.iter("name") if x.get("lang") == "de"), "")
        ports = sorted({t.get("name") for t in d.iter("terminal") if t.get("name")})
        rel = datei.relative_to(a.ordner).as_posix()
        kand = [k for k in fs.get(name.strip().lower(), []) if len(k["stellungen"]) == n]
        # direkt aus FluidSIM umgewandelte Symbole heißen wie ihre Vorlage (z. B. el6.elmt <- el6.circ)
        kand = [k for k in kand if Path(k["datei"]).stem == datei.stem] or kand
        grund, wahl = "", None
        if not kand:
            grund = "kein FluidSIM-Ventil mit diesem Namen und dieser Stellungszahl"
        else:
            # Steueranschlüsse (10, 12, 14, X, Y …) sitzen in FluidSIM an der Betätigung, nicht am Ventilkörper
            steuer = {"10", "12", "14", "81", "82", "84", "91", "X", "Y", "Z"}
            passt = lambda k: set(k["anschluesse"]) <= set(ports) and set(ports) - set(k["anschluesse"]) <= steuer
            kand = [k for k in kand if passt(k)] or kand
            if not passt(kand[0]):
                grund = f"Anschlüsse passen nicht: Symbol {ports}, FluidSIM {kand[0]['anschluesse']}"
            else:
                # Vorrang hat die gezeichnete Grafik (in welchem Kästchen ein Durchfluss gezeichnet ist),
                # danach die Ruhestellung, danach die schrägen Linien je Kästchen
                g = schraeg_in_grafik(d.find("description"), n)
                grafik = g[0] if g else None
                fluss = g[1] if g else None
                flussgleich = [k for k in kand if fluss is None or [bool(w) for w, _ in k["stellungen"]] == fluss]
                flussgleich = [k for k in flussgleich if not k["ruhe"] or k["ruhe"] == ruhe] or flussgleich
                passend = [k for k in flussgleich if grafik is None or k["schraeg"] == grafik] or flussgleich
                varianten = {tuple(map(str, k["stellungen"])) for k in passend}
                alle = {tuple(map(str, k["stellungen"])) for k in kand}
                if not flussgleich:
                    varianten = set()
                    grund = (f"Grafik widerspricht FluidSIM: Durchfluss gezeichnet je Kästchen {fluss}, "
                             f"FluidSIM {[[bool(w) for w, _ in k['stellungen']] for k in kand]}")
                elif len(varianten) == 1:
                    wahl = passend[0]
                elif len(alle) == 1:
                    wahl = kand[0]
                    grund = f"Grafik weicht ab (schräg je Feld {grafik}, FluidSIM {kand[0]['schraeg']})"
                else:
                    wahl = sorted(passend or kand, key=lambda k: k["datei"])[0]
                    grund = (f"Variante nicht sicher, gewählt {wahl['datei']} aus "
                             f"{sorted(k['datei'] for k in kand)} (Grafik schräg je Feld {grafik})")
        if wahl:
            if wahl["ruhe"] and wahl["ruhe"] != ruhe:
                grund = f"Ruhestellung weicht ab (Symbol {ruhe}, FluidSIM {wahl['ruhe']})"
            wege_txt = " | ".join(
                f"{i + 1}{' R' if i + 1 == ruhe else ''}: " + ", ".join(
                    [f"{v}->{z}" for v, z in w] + [f"{g} gesperrt" for g in sorted(s, key=rang)])
                for i, (w, s) in enumerate(wahl["stellungen"]))
            zeilen.append((rel, name, "eingetragen", wahl["datei"], wege_txt, grund))
            if not a.nur_zeigen:
                neu = text_ersetzen(text, wahl["stellungen"])
                if neu:
                    datei.write_text(neu, encoding="utf-8", newline="")
        else:
            zeilen.append((rel, name, "offen", "", "", grund))
    ok = sum(1 for z in zeilen if z[2] == "eingetragen")
    for z in zeilen:
        print(f"{z[2]:11} {z[1][:55]:55} {z[3]:18} {z[4] or z[5]}  {z[5] if z[4] else ''}")
    print(f"\n{ok} von {len(zeilen)} Ventilen mit Schaltwegen" + (" (nur gezeigt)" if a.nur_zeigen else ""))
    if a.bericht:
        with a.bericht.open("w", encoding="utf-8-sig", newline="") as f:
            w = csv.writer(f, delimiter=";")
            w.writerow(["Datei", "Name", "Status", "FluidSIM-Vorlage", "Schaltwege", "Hinweis"])
            w.writerows(zeilen)


if __name__ == "__main__":
    main()
