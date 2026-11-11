"""Gerätesymbol im EPLAN-Stil für Geräte ohne Herstellermakro: Kasten mit frei gesetzten Anschlüssen.

Anders als `symbolbau.kasten2` (fester Anschlussabstand) legt der Zeichner jeden Anschluss selbst auf die Spalte, in
der sein Strompfad läuft – so bleiben alle Leitungen senkrecht (Skill qet-schaltplan-zeichnen: „Gerätesymbol
anpassen statt schräge Leitungen hinnehmen“). Anschlussnamen stehen senkrecht im Kasten, damit sie auch im
20-px-Raster nicht ineinanderlaufen.

    geraet("Leadshine/Antriebe/EM542S/EM542S_Plan.elmt", "EM542S", breite=340,
           oben=[("ALM", 40), ("ENA+", 100)], unten=[("GND", 40), ("A+", 240)],
           hersteller="Leadshine", artikel="EM542S", beschreibung="Schrittmotortreiber")
"""
import os
import sys
import uuid
from xml.sax.saxutils import escape

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "eplan_import"))
from planvorlagen.symbole import COMPANY          # noqa: E402

STIL = 'style="line-style:normal;line-weight:normal;filling:none;color:black" antialias="false"'


def geraet(rel, typ, breite, oben=(), unten=(), hoehe=110, hersteller="", artikel="", beschreibung="",
           ueberschreiben=False):
    """oben/unten: [(Name, x)] mit x ab linker Kastenkante (Vielfache von 10). Einfügepunkt = linke obere Ecke.
    Bestehende Dateien bleiben (ihre Anschluss-UUIDs stecken in fertigen Plänen), außer mit ueberschreiben."""
    pfad = os.path.join(COMPANY, rel)
    ziel = "company://" + rel.replace("\\", "/")
    if os.path.exists(pfad) and not ueberschreiben:
        return ziel
    t = [f'<rect x="0" y="0" width="{breite}" height="{hoehe}" {STIL}/>']
    for liste, y, ri in ((oben, 0, "n"), (unten, hoehe, "s")):
        for name, x in liste:
            t.append(f'<terminal uuid="{{{uuid.uuid4()}}}" name="{escape(name)}" x="{x}" y="{y}" '
                     f'orientation="{ri}" type="Generic"/>')
            # senkrecht, von unten nach oben zu lesen; oben beginnt der Name an der Kante, unten endet er dort
            ty = y + 6 + 4.2 * len(name) if ri == "n" else y - 6
            t.append(f'<text x="{x + 3}" y="{ty:.0f}" text="{escape(name)}" rotation="270" color="#000000" '
                     'font="Liberation Sans,6,-1,5,50,0,0,0,0,0,Regular"/>')
    infos = "".join(f'<elementInformation name="{k}" show="1">{escape(v)}</elementInformation>'
                    for k, v in (("manufacturer", hersteller), ("manufacturer_reference", artikel),
                                 ("designation", typ), ("description", beschreibung)) if v)
    label = ('<dynamic_text x="-2" y="-26" z="2" text_width="-1" Halignment="AlignLeft" Valignment="AlignTop" '
             'frame="false" rotation="0" keep_visual_rotation="false" text_from="ElementInfo" '
             f'uuid="{{{uuid.uuid4()}}}" font="Liberation Sans,10,-1,5,50,0,0,0,0,0,Regular">'
             '<text></text><info_name>label</info_name></dynamic_text>')
    xml = (f'<definition version="0.100.0" type="element" link_type="simple" width="{breite + 20}" '
           f'height="{hoehe + 40}" hotspot_x="10" hotspot_y="30">\n'
           f'<uuid uuid="{{{uuid.uuid4()}}}"/>\n<names><name lang="de">{escape(typ)}</name>'
           f'<name lang="en">{escape(typ)}</name></names>\n'
           f'<elementInformations>{infos}<elementInformation name="label" show="1"></elementInformation>'
           f'</elementInformations>\n<informations>splan_import.geraetekasten (kein Herstellermakro)</informations>\n'
           f'<description>{label}{"".join(t)}</description>\n</definition>\n')
    os.makedirs(os.path.dirname(pfad), exist_ok=True)
    with open(pfad, "w", encoding="utf-8") as f:
        f.write(xml)
    return ziel
