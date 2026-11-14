"""Funktion und Kennwerte der Fluidbauteile aus FluidSIM 6 übernehmen (für eine spätere Simulation).

Zu jedem Symbol in bauteile/fluid wird das FluidSIM-Bauteil mit gleichem deutschem Namen gesucht. Übernommen
werden nur Daten, keine Grafik:
- function="…" am <fluid>-Block: die Bauteilklasse aus FluidSIM ohne Herstellerbezug, z. B.
  `pressure_valve.relief_valve`, `shutoff_valve.check_valve`, `supply.pump`;
- je Kennwert ein <parameter name="…" value="…" unit="…"/>: die einstellbaren Werte (`parameter`) und die festen
  Modellwerte (`const`) von FluidSIM, z. B. Öffnungsdruck, Förderstrom, Federkraft, Kolbendurchmesser.

Bestehende Kennwerte werden ersetzt, alles andere im <fluid>-Block (Stellungen, Schaltwege, Betätigung,
Zylinderdaten) bleibt. Symbole ohne passendes FluidSIM-Bauteil bleiben unverändert und stehen im Bericht.

    python fluid_kennwerte.py --fluidsim "C:/Program Files (x86)/Didactic/FluidSIM6" bauteile/fluid --nur-zeigen
    python fluid_kennwerte.py --fluidsim ... bauteile/fluid --bericht kennwerte.csv
"""
import argparse
import csv
import gzip
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from xml.sax.saxutils import quoteattr

sys.path.insert(0, str(Path(__file__).resolve().parent))
from fluidsim_symbole import Uebersetzung  # noqa: E402

NEBEN = ("Supply.Silencer", "Supply.BlindPlug", "TextBox")


def funktion(objekt_ref):
    """Hyd.PressureValve.ReliefValve -> pressure_valve.relief_valve"""
    teile = objekt_ref.split(".")
    if teile and teile[0] in ("Hyd", "Pneu", "ET"):
        teile = teile[1:]
    return ".".join(re.sub(r"(?<=[a-z0-9])(?=[A-Z])", "_", t).lower() for t in teile)


def fluidsim_bauteile(fluidsim: Path):
    w = ET.fromstring(gzip.decompress((fluidsim / "sym" / "sym_fs.lib").read_bytes()))
    sprache = Uebersetzung(fluidsim / "db" / "lang.db")
    out = {}
    for f in w.iter("file"):
        eig = f.find("section[@type='properties']")
        if eig is None:
            continue
        comps = [c for c in f.iter("component")
                 if c.get("object-ref") and not any(n in c.get("object-ref") for n in NEBEN)]
        if not comps:
            continue
        c = comps[0]
        werte = []
        for e in c:
            if e.tag in ("parameter", "const") and e.get("ident"):
                ident, wert = e.get("ident"), e.get("value", "")
                # Bildverweise und Beschriftungen von FluidSIM sind keine Kennwerte
                if not wert or wert.endswith(".pic") or ".pic," in wert or ident == "picture"                         or "label" in ident.lower():
                    continue
                werte.append((ident, wert, e.get("unit", "")))
        de, _ = sprache.namen(eig.get("name", ""))
        out.setdefault(de.strip().lower(), []).append(
            {"datei": f.get("name"), "funktion": funktion(c.get("object-ref")), "werte": werte,
             "medium": {"Hyd": "hydraulic", "Pneu": "pneumatic"}.get(c.get("object-ref").split(".")[0], "")})
    return out


def eintragen(text, fkt, werte):
    """function-Attribut setzen und <parameter>-Kinder im <fluid>-Block ersetzen; Rest unverändert."""
    m = re.search(r"([ \t]*)<fluid\b([^>]*?)(/?)>", text)
    if not m:
        return None
    einzug, attrs, leer = m.group(1), m.group(2), m.group(3)
    attrs = re.sub(r'\s+function="[^"]*"', "", attrs).rstrip()
    attrs += f" function={quoteattr(fkt)}"
    zeilen = [f"{einzug}    <parameter name={quoteattr(n)} value={quoteattr(v)}"
              + (f" unit={quoteattr(u)}" if u else "") + " />" for n, v, u in werte]
    if leer:
        neu = f"{einzug}<fluid{attrs}>\n" + "\n".join(zeilen) + f"\n{einzug}</fluid>"
        return text[:m.start()] + neu + text[m.end():]
    ende = text.index("</fluid>", m.end())
    innen = re.sub(r"\n[ \t]*<parameter\b[^>]*/>", "", text[m.end():ende]).rstrip(" \t")
    if not innen.endswith("\n"):
        innen += "\n"
    neu = f"{einzug}<fluid{attrs}>" + innen + "\n".join(zeilen) + f"\n{einzug}"
    return text[:m.start()] + neu + text[ende:]


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--fluidsim", type=Path, required=True)
    ap.add_argument("ordner", type=Path)
    ap.add_argument("--nur-zeigen", action="store_true")
    ap.add_argument("--bericht", type=Path)
    a = ap.parse_args()
    fs = fluidsim_bauteile(a.fluidsim)
    zeilen = []
    for datei in sorted(a.ordner.rglob("*.elmt")):
        text = datei.read_text(encoding="utf-8")
        d = ET.fromstring(text)
        rel = datei.relative_to(a.ordner).as_posix()
        name = next((x.text or "" for x in d.iter("name") if x.get("lang") == "de"), "")
        if d.find("fluid") is None:
            zeilen.append((rel, name, "offen", "", "", "kein <fluid>-Block"))
            continue
        medium = next((t.get("medium") for t in d.iter("terminal") if t.get("medium")), "")
        kand = [k for k in fs.get(name.strip().lower(), []) if not k["medium"] or not medium or k["medium"] == medium]
        if not kand:
            zeilen.append((rel, name, "offen", "", "", "kein FluidSIM-Bauteil mit diesem Namen"))
            continue
        # direkt aus FluidSIM umgewandelte Symbole heißen wie ihre Vorlage (z. B. el6.elmt <- el6.circ)
        kand = [x for x in kand if Path(x["datei"] or "").stem == datei.stem] or kand
        k = kand[0]
        hinweis = ""
        if len({(x["funktion"], tuple(x["werte"])) for x in kand}) > 1:
            hinweis = f"mehrere Varianten, genommen {k['datei']} aus {[x['datei'] for x in kand]}"
        werte_txt = ", ".join(f"{n}={v}{(' ' + u) if u else ''}" for n, v, u in k["werte"])
        zeilen.append((rel, name, "eingetragen", k["funktion"], werte_txt, hinweis))
        if not a.nur_zeigen:
            neu = eintragen(text, k["funktion"], k["werte"])
            if neu:
                datei.write_text(neu, encoding="utf-8", newline="")
    ok = sum(1 for z in zeilen if z[2] == "eingetragen")
    for z in zeilen:
        print(f"{z[2]:11} {z[1][:50]:50} {z[3][:40]:40} {z[5]}")
    print(f"\n{ok} von {len(zeilen)} Symbolen mit Funktion und Kennwerten" + (" (nur gezeigt)" if a.nur_zeigen else ""))
    if a.bericht:
        with a.bericht.open("w", encoding="utf-8-sig", newline="") as f:
            w = csv.writer(f, delimiter=";")
            w.writerow(["Datei", "Name", "Status", "Funktion", "Kennwerte", "Hinweis"])
            w.writerows(zeilen)


if __name__ == "__main__":
    main()
