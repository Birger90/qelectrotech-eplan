"""GRAFCET-Symbole (IEC 60848) sauber neu zeichnen, unter den Pfaden der Standardsammlung.

Die Symbole der QElectroTech-Sammlung (20_logic/2020_flow_chart/202020_grafcet) haben die Schrittnummer neben dem
Kasten, Anschlüsse 5 px neben der Grafik ohne Linie (QET zeichnet den Anschlussstrich im Export nicht), leere
Platzhaltertexte, die als ' und _ erscheinen, und Verzweigungsbalken ohne Anschluss oben bzw. unten. Dieser Generator
schreibt Ersatzsymbole nach bauteile/grafcet/<gleicher Pfad>:

- alle Anschlüsse im 10er-Raster auf der Kante der Grafik (Schrittkasten 40 x 40), keine Stummel;
- Schrittnummer (Kennzeichen) mittig im Kasten, Weiterschaltbedingung (Kennzeichen) rechts neben der Transition,
  Aktionstext (Kennzeichen) mittig im Aktionskasten; Bedingung bzw. Ereignis als Text über dem Kasten;
- Parallelverzweigung mit Anschluss oben und zwei Zweigen im Abstand 200, Synchronisierung spiegelbildlich, so dass ein
  Schritt mit Aktionskasten im linken Zweig den rechten Zweig nicht berührt.

Namen (alle Sprachen) und UUID kommen aus dem Original. Aufruf: python tools/symbole/grafcet_symbole.py
"""
import pathlib
import re
import uuid

REPO = pathlib.Path(__file__).resolve().parents[2]
REL = pathlib.Path("20_logic/2020_flow_chart/202020_grafcet")
SRC = REPO / "elements" / REL
DST = REPO / "bauteile" / "grafcet" / REL

STIL = "line-style:normal;line-weight:normal;filling:none;color:black"
VOLL = "line-style:normal;line-weight:normal;filling:black;color:black"
SCHRIFT_NR = "Liberation Sans,9,-1,5,50,0,0,0,0,0,Regular"
SCHRIFT = "Liberation Sans,8,-1,5,50,0,0,0,0,0,Regular"

# Ein leerer Text hat in QET einen Rahmen von 8 px Breite; die Höhe einer Zeile 8/9 pt mit Rand. Der Anker
# (Mitte bzw. linke Kante) bleibt beim Schreiben stehen; diese Werte setzen ihn auf die gewünschte Stelle.
LEER_B = 8.0
LEER_H = {SCHRIFT_NR: 22.0, SCHRIFT: 21.0}


def linie(x1, y1, x2, y2):
    return (f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" end1="none" end2="none" length1="1.5" length2="1.5" '
            f'style="{STIL}" antialias="false"/>')


def rechteck(x, y, b, h, voll=False):
    return (f'<rect x="{x}" y="{y}" width="{b}" height="{h}" rx="0" ry="0" style="{VOLL if voll else STIL}" '
            f'antialias="false"/>')


def polygon(punkte, voll=True):
    attrs = " ".join(f'x{i + 1}="{x}" y{i + 1}="{y}"' for i, (x, y) in enumerate(punkte))
    return f'<polygon {attrs} closed="true" style="{VOLL if voll else STIL}" antialias="true"/>'


def anschluss(x, y, o, name):
    return f'<terminal uuid="{{{uuid.uuid4()}}}" name="{name}" x="{x}" y="{y}" orientation="{o}" type="Generic"/>'


def text(x, y, mitte=True, quelle="ElementInfo", schrift=SCHRIFT):
    """Text mit Anker (x, y): mitte = waagerecht und senkrecht mittig, sonst linke Kante, senkrecht mittig."""
    h = LEER_H[schrift]
    px = x - LEER_B / 2 if mitte else x
    py = y - h / 2
    ha = "AlignHCenter" if mitte else "AlignLeft"
    info = "<info_name>label</info_name>" if quelle == "ElementInfo" else ""
    return (f'<dynamic_text x="{px:g}" y="{py:g}" z="5" text_width="-1" Halignment="{ha}" Valignment="AlignVCenter" '
            f'frame="false" rotation="0" keep_visual_rotation="false" text_from="{quelle}" '
            f'uuid="{{{uuid.uuid4()}}}" font="{schrift}"><text></text>{info}</dynamic_text>')


def schritt_grundform(doppelt=False, makro=False, einschliessend=False):
    # Kasten 40 x 40: die Anschlüsse liegen auf der Kante, unbenutzte Anschlüsse zeigen keinen Stummel
    teile = [rechteck(-20, -20, 40, 40)]
    if doppelt:
        teile.append(rechteck(-16, -16, 32, 32))
    if makro:
        teile += [linie(-20, -15, 20, -15), linie(-20, 15, 20, 15)]
    if einschliessend:
        teile += [linie(-20, -13, -13, -20), linie(13, -20, 20, -13), linie(-20, 13, -13, 20), linie(13, 20, 20, 13)]
    teile += [anschluss(0, -20, "n", "in"), anschluss(0, 20, "s", "out"), anschluss(20, 0, "e", "action"),
              text(0, 0, schrift=SCHRIFT_NR)]
    return teile, (40, 40, 20, 20)


def transition():
    teile = [linie(0, -10, 0, 10), rechteck(-12, -2, 24, 4, voll=True),
             anschluss(0, -10, "n", "in"), anschluss(0, 10, "s", "out"), text(18, 0, mitte=False)]
    return teile, (130, 20, 20, 10)


def aktion(art):
    """art: 0 kontinuierlich, 1 mit Bedingung, 2 bei Aktivierung, 3 bei Deaktivierung, 4 bei Ereignis."""
    teile = [linie(0, 0, 10, 0), rechteck(10, -15, 140, 30), anschluss(0, 0, "w", "step"), text(80, 0)]
    if art in (1, 4):
        # Bedingung bzw. Ereignis: kurzer Strich auf dem Kasten, Text darüber (Benutzertext)
        teile += [linie(80, -15, 80, -25), text(84, -32, mitte=False, quelle="UserText")]
    if art == 2:      # speichernd bei Aktivierung: Pfeil nach oben an der linken Kastenkante
        teile += [linie(22, -15, 22, -27), polygon([(22, -29), (19, -23), (25, -23)])]
    if art == 3:      # bei Deaktivierung: Pfeil nach unten
        teile += [linie(22, -27, 22, -19), polygon([(22, -15), (19, -21), (25, -21)])]
    if art == 4:      # bei Ereignis: Pfeil nach oben am Strich
        teile += [polygon([(80, -27), (77, -21), (83, -21)])]
    # angemeldete Größe = gezeichnete Größe (Kasten 30 hoch, Pfeil bis -29, Text über dem Kasten bis -40)
    oben = {0: 15, 1: 40, 2: 30, 3: 30, 4: 40}[art]
    return teile, (150, oben + 15, 0, oben)


def parallel():
    teile = [linie(-110, 0, 110, 0), linie(-110, 3, 110, 3), linie(0, -10, 0, 0),
             linie(-100, 3, -100, 10), linie(100, 3, 100, 10),
             anschluss(0, -10, "n", "in"), anschluss(-100, 10, "s", "1"), anschluss(100, 10, "s", "2"),
             anschluss(-110, 0, "w", "w"), anschluss(110, 0, "e", "e")]
    return teile, (240, 20, 120, 10)


def synchronisierung():
    teile = [linie(-110, -3, 110, -3), linie(-110, 0, 110, 0), linie(0, 0, 0, 10),
             linie(-100, -10, -100, -3), linie(100, -10, 100, -3),
             anschluss(-100, -10, "n", "1"), anschluss(100, -10, "n", "2"), anschluss(0, 10, "s", "out"),
             anschluss(-110, 0, "w", "w"), anschluss(110, 0, "e", "e")]
    return teile, (240, 20, 120, 10)


def sprung():
    teile = [linie(0, -20, 0, 0), polygon([(0, 0), (-4, -8), (4, -8)]), anschluss(0, -20, "n", "in"),
             text(10, -6, mitte=False)]
    return teile, (60, 30, 10, 20)


SYMBOLE = {
    "20202000_step.elmt": lambda: schritt_grundform(doppelt=True),
    "20202001_step.elmt": lambda: schritt_grundform(),
    "20202002_step.elmt": lambda: schritt_grundform(einschliessend=True),
    "20202003_step.elmt": lambda: schritt_grundform(makro=True),
    "20202020_action.elmt": lambda: aktion(0),
    "20202021_action.elmt": lambda: aktion(1),
    "20202022_action.elmt": lambda: aktion(2),
    "20202023_action.elmt": lambda: aktion(3),
    "20202024_action.elmt": lambda: aktion(4),
    "20202060_transition.elmt": transition,
    "20202070_parallel_branch.elmt": parallel,
    "20202071_synchronization.elmt": synchronisierung,
    "20202090_jump.elmt": sprung,
}


def main():
    DST.mkdir(parents=True, exist_ok=True)
    for name, bauen in SYMBOLE.items():
        alt = (SRC / name).read_text(encoding="utf-8")
        namen = re.search(r"<names>.*?</names>", alt, re.S).group(0)
        kennung = re.search(r'<uuid uuid="(\{[^}]+\})"/>', alt).group(1)
        teile, (b, h, hx, hy) = bauen()
        inhalt = "\n        ".join(teile)
        xml = (f'<definition version="0.100" type="element" link_type="simple" width="{b}" height="{h}" '
               f'hotspot_x="{hx}" hotspot_y="{hy}">\n'
               f'    <uuid uuid="{kennung}"/>\n    {namen}\n'
               '    <informations>Author: The QElectroTech team; redrawn on the 10 grid (QElectroTech-EPLAN)\n'
               'License: see http://qelectrotech.org/wiki/doc/elements_license</informations>\n'
               f'    <description>\n        {inhalt}\n    </description>\n</definition>\n')
        (DST / name).write_text(xml, encoding="utf-8")
        print("geschrieben:", name)


if __name__ == "__main__":
    main()
