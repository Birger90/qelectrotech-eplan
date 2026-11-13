"""Pneumatik- und Hydraulikbauteile für die Fluidtechnik nachrüsten.

Die Bauteile der QET-Sammlung (`elements/50_pneumatic`, `elements/30_hydraulic`) haben Anschlüsse ohne Namen und
ohne Medium: Für QElectroTech sind es elektrische Anschlüsse, eine Leitung zu einem pneumatischen Anschluss geht
nicht. Dieses Werkzeug schreibt eine Kopie jedes Bauteils, in der

- jeder Anschluss das Medium trägt (`medium="pneumatic"` bzw. `"hydraulic"`),
- jeder Anschluss den Namen der Portnummer bekommt, die neben ihm gezeichnet ist (1, 2, 3, 4, 5, 12, 14, P, …),
- das Bauteil eine neue UUID hat (es ist ein anderes Bauteil als das Original).

Die Grafik bleibt unverändert. Die Portnummern werden allen Anschlüssen gemeinsam zugeordnet, mit der kleinsten
Summe der Abstände: Beim 5/2-Ventil steht die „1“ näher an Anschluss 5 als die „5“, die gemeinsame Zuordnung
trifft trotzdem richtig. Ein Anschluss ohne Text in der Nähe bleibt ohne Namen.

    python fluid_nachruesten.py elements/50_pneumatic --ziel ZIELORDNER
    python fluid_nachruesten.py elements/30_hydraulic --ziel ZIELORDNER --medium hydraulic
    python fluid_nachruesten.py elements/50_pneumatic --nur-zeigen

Zum Benutzen den Zielordner in die Benutzersammlung legen (Datenordner\\elements).
"""
import argparse
import re
import shutil
import sys
import uuid
import xml.etree.ElementTree as ET
from pathlib import Path

# Was als Portbezeichnung gilt: Zahlen nach ISO 5599-3 (1, 2, 12, 14, 82 …) und die Buchstaben nach ISO 1219
PORT = re.compile(r"^(\d{1,3}|[PRSTABXYZL]\d?|EX)$")
# Weiter weg zählt ein Text nicht mehr als Bezeichnung eines Anschlusses (px)
MAX_ABSTAND = 16.0
# Steueranschlüsse (12, 14, 82, 84 …) liegen an der Seite eines Ventils, die Arbeitsanschlüsse 1–5 oben und unten.
# Eine zweistellige Zahl an einem Anschluss, der nach oben oder unten zeigt, ist meist die Beschriftung der
# Betätigungsseite daneben: sie kostet so viel mehr.
AUFSCHLAG_STEUERNAME = 10.0


def schriftgroesse(font: str, vorgabe: float = 9.0) -> float:
    """Punktgröße aus einem QFont-String wie „Liberation Sans,3,-1,5,…“."""
    teile = (font or "").split(",")
    try:
        return float(teile[1])
    except (IndexError, ValueError):
        return vorgabe


def textmitten(d):
    """(Mitte x, Mitte y, Text) aller Texte, die eine Portbezeichnung sein können."""
    out = []
    for dt in d.iter("dynamic_text"):
        if dt.get("text_from") != "UserText":
            continue
        s = (dt.findtext("text") or "").strip()
        if not PORT.match(s):
            continue
        g = schriftgroesse(dt.get("font"), float(dt.get("size", 9)))
        # dynamic_text: x/y ist die linke obere Ecke des Textfelds
        x, y = float(dt.get("x")), float(dt.get("y"))
        out.append((x + 0.3 * g * len(s), y + 0.7 * g, s))
    for t in d.iter("text"):
        s = (t.get("text") or "").strip()
        if not t.get("x") or not PORT.match(s):
            continue
        g = float(t.get("size", 9))
        # statischer Text: x/y ist die Grundlinie links
        x, y = float(t.get("x")), float(t.get("y"))
        out.append((x + 0.3 * g * len(s), y - 0.35 * g, s))
    return out


def eindeutig(namen):
    """Doppelte Namen eindeutig machen: zweimal „1“ wird „1.1“ und „1.2“; leere Namen bleiben leer.

    Ein Wechselventil hat nach ISO 1219 zwei Eingänge „1“. Gezeichnet bleibt das so, aber QET verlangt
    unterscheidbare Anschlussnamen (Verdrahtungsliste, IEC 61666; der Bauteil-Editor speichert sonst nicht).
    """
    zahl = {}
    for n in namen:
        if n:
            zahl[n] = zahl.get(n, 0) + 1
    gesehen = {}
    out = []
    for n in namen:
        if n and zahl[n] > 1:
            gesehen[n] = gesehen.get(n, 0) + 1
            out.append(f"{n}.{gesehen[n]}")
        else:
            out.append(n)
    return out


def kosten_fuer(anschluss, text):
    """Abstand von Anschluss (x, y, Richtung) zu Text (x, y, Name), mit Aufschlag für einen Steuernamen oben/unten."""
    ax, ay, richtung = anschluss
    tx, ty, name = text
    k = ((ax - tx) ** 2 + (ay - ty) ** 2) ** 0.5
    if richtung in ("n", "s") and re.fullmatch(r"\d{2}", name):
        k += AUFSCHLAG_STEUERNAME
    return k


def zuordnen(anschluesse, texte):
    """Index des Texts je Anschluss (oder None): kleinste Summe der Abstände, jeder Text höchstens einmal.

    Ein Anschluss ohne passenden Text kostet MAX_ABSTAND; so lohnt sich eine Zuordnung nur, wenn sie näher ist.
    """
    n, m = len(anschluesse), len(texte)
    kosten = [[kosten_fuer(a, t) for t in texte] for a in anschluesse]
    if m > 16:   # zu viele Texte für die vollständige Suche: lieber keine Namen als falsche
        return [None] * n
    memo = {}

    def beste(i, belegt):
        if i == n:
            return 0.0, ()
        schluessel = (i, belegt)
        if schluessel in memo:
            return memo[schluessel]
        rest, wahl = beste(i + 1, belegt)
        ergebnis = (rest + MAX_ABSTAND, (None,) + wahl)
        for j in range(m):
            if belegt & (1 << j) or kosten[i][j] >= MAX_ABSTAND:
                continue
            rest, wahl = beste(i + 1, belegt | (1 << j))
            if rest + kosten[i][j] < ergebnis[0]:
                ergebnis = (rest + kosten[i][j], (j,) + wahl)
        memo[schluessel] = ergebnis
        return ergebnis

    return list(beste(0, 0)[1])


def nachruesten(text: str, medium: str):
    """Neuer Dateitext und die vergebenen Namen. Nur die <terminal>-Zeilen und die UUID ändern sich."""
    d = ET.fromstring(text)
    anschluesse = [(float(t.get("x")), float(t.get("y")), t.get("orientation", "n")) for t in d.iter("terminal")]
    texte = textmitten(d)
    wahl = zuordnen(anschluesse, texte)
    namen = eindeutig([texte[j][2] if j is not None else "" for j in wahl])

    zaehler = iter(range(len(anschluesse)))

    def anschluss(mm):
        tag = mm.group(0)
        i = next(zaehler)
        if namen[i] and re.search(r'\bname=""', tag):
            tag = re.sub(r'\bname=""', 'name="%s"' % namen[i], tag, count=1)
        if " medium=" not in tag:
            tag = re.sub(r"\s*/>$", ' medium="%s"/>' % medium, tag)
        return tag

    neu = re.sub(r"<terminal\b[^>]*/>", anschluss, text)
    # Neue UUID, aus der alten abgeleitet: bei jedem Lauf dieselbe, aber eine andere als die des Originals
    alt = re.search(r'<uuid uuid="\{([^}]*)\}"', text)
    neue = uuid.uuid5(uuid.NAMESPACE_URL, "qet-fluid-nachruesten/" + (alt.group(1).lower() if alt else text))
    neu = re.sub(r'<uuid uuid="\{[^}]*\}"\s*/>', '<uuid uuid="{%s}"/>' % neue, neu, count=1)
    return neu, namen


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("quelle", type=Path, help="Ordner der Sammlung oder eine .elmt-Datei")
    ap.add_argument("--ziel", type=Path, help="Ordner für die nachgerüsteten Kopien")
    ap.add_argument("--medium", default="pneumatic", choices=["pneumatic", "hydraulic"])
    ap.add_argument("--nur-zeigen", action="store_true", help="nur anzeigen, was vergeben würde")
    a = ap.parse_args(argv)
    if not a.nur_zeigen and not a.ziel:
        ap.error("--ziel fehlt (oder --nur-zeigen)")

    dateien = [a.quelle] if a.quelle.is_file() else sorted(a.quelle.rglob("*.elmt"))
    basis = a.quelle.parent if a.quelle.is_file() else a.quelle
    gesamt = benannt = ohne = 0
    for f in dateien:
        text = f.read_text(encoding="utf-8")
        neu, namen = nachruesten(text, a.medium)
        gesamt += len(namen)
        benannt += sum(1 for n in namen if n)
        ohne += 0 if any(namen) else 1
        if a.nur_zeigen:
            print(f"{f.relative_to(basis)}: {' '.join(n or '-' for n in namen)}")
            continue
        ziel = a.ziel / f.relative_to(basis)
        ziel.parent.mkdir(parents=True, exist_ok=True)
        ziel.write_text(neu, encoding="utf-8", newline="\n")
    if not a.nur_zeigen and a.quelle.is_dir():
        # Ordnernamen der Sammlung mitnehmen
        for q in a.quelle.rglob("qet_directory"):
            z = a.ziel / q.relative_to(basis)
            z.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(q, z)
    print(f"{len(dateien)} Bauteile, {gesamt} Anschlüsse, davon {benannt} benannt; "
          f"{ohne} Bauteile ohne Portnummern (Medium trotzdem gesetzt).")


if __name__ == "__main__":
    main()
