"""Einfache Gerätesymbole für Teile ohne Herstellerdaten (Black-Box mit beschrifteten Anschlüssen unten).

Regel „Symbolquellen-Reihenfolge“: nur, wenn es kein Herstellermakro und kein passendes Normsymbol gibt.
Ein Gerät = ein Symbol mit Artikeldaten; Anschlüsse im 20-px-Raster unten, Nummer + Bedeutung senkrecht.

    kasten("Fiessler/Sicherheitslichtgitter/ULVT/UE800-3.elmt", "UE800/3",
           [("7", "+24VDC"), ("6", "0V"), ("4", "OSSD2"), ("3", "OSSD1")],
           hersteller="Fiessler", artikel="UE800/3", beschreibung="Sicherheits-Lichtgitter Empfänger")
"""
import os
import uuid
from xml.sax.saxutils import escape

from .symbole import COMPANY


def kasten(rel, typ, anschluesse, hersteller="", artikel="", beschreibung="", abstand=20, ueberschreiben=False):
    """Schreibt das Symbol (falls noch nicht da) und gibt den company://-Pfad zurück."""
    pfad = os.path.join(COMPANY, rel)
    ziel = "company://" + rel.replace("\\", "/")
    if os.path.exists(pfad) and not ueberschreiben:
        return ziel
    n = len(anschluesse)
    breite = ((max(100, (n + 1) * abstand + 20)) + 9) // 10 * 10
    hoehe = 80
    teile = [f'<rect x="0" y="-{hoehe}" width="{breite - 20}" height="{hoehe}" '
             'style="line-style:normal;line-weight:normal;filling:none;color:black" antialias="false"/>',
             f'<text x="6" y="-{hoehe - 14}" size="9" text="{escape(typ)}" rotation="0" color="#000000" '
             'font="Liberation Sans,9,-1,5,50,0,0,0,0,0,Regular"/>']
    for i, (name, bedeutung) in enumerate(anschluesse):
        x = abstand * (i + 1)
        teile.append(f'<terminal uuid="{{{uuid.uuid4()}}}" name="{escape(name)}" x="{x}" y="0" orientation="s" '
                     'type="Generic"/>')
        teile.append(f'<text x="{x - 6}" y="-4" size="6" text="{escape(name)}" rotation="0" color="#000000" '
                     'font="Liberation Sans,6,-1,5,50,0,0,0,0,0,Regular"/>')
        if bedeutung:
            teile.append(f'<text x="{x + 3}" y="-6" size="5" text="{escape(bedeutung)}" rotation="270" '
                         'color="#000000" font="Liberation Sans,5,-1,5,50,0,0,0,0,0,Regular"/>')
    infos = "".join(f'<elementInformation name="{k}" show="1">{escape(v)}</elementInformation>'
                    for k, v in (("manufacturer", hersteller), ("manufacturer_reference", artikel),
                                 ("designation", artikel), ("description", beschreibung)) if v)
    label = ('<dynamic_text x="0" y="-104" z="2" text_width="-1" Halignment="AlignLeft" Valignment="AlignTop" '
             'frame="false" rotation="0" keep_visual_rotation="false" text_from="ElementInfo" '
             f'uuid="{{{uuid.uuid4()}}}" font="Liberation Sans,10,-1,5,50,0,0,0,0,0,Regular">'
             '<text></text><info_name>label</info_name></dynamic_text>')
    xml = (f'<definition version="0.100.0" type="element" link_type="simple" width="{breite}" height="{hoehe + 40}" '
           f'hotspot_x="0" hotspot_y="{hoehe + 30}">\n'
           f'<uuid uuid="{{{uuid.uuid4()}}}"/>\n<names><name lang="de">{escape(typ)}</name>'
           f'<name lang="en">{escape(typ)}</name></names>\n'
           f'<elementInformations>{infos}<elementInformation name="label" show="1"></elementInformation>'
           f'</elementInformations>\n<informations>Planvorlagen symbolbau.kasten (keine Herstellerdaten)</informations>\n'
           f'<description>{label}{"".join(teile)}</description>\n</definition>\n')
    os.makedirs(os.path.dirname(pfad), exist_ok=True)
    with open(pfad, "w", encoding="utf-8") as f:
        f.write(xml)
    return ziel


def polabstand(quelle, rel, faktor, namen=None, titel=None, ueberschreiben=False):
    """Normsymbol mit anderem Polabstand ableiten: alle x-Werte (Linien, Kreise, Anschlüsse) mal `faktor`.
    namen: {(x_neu, y): Anschlussname}. quelle: Dateipfad des Originals. Rückgabe company://-Pfad."""
    import re
    pfad = os.path.join(COMPANY, rel)
    ziel = "company://" + rel.replace("\\", "/")
    if os.path.exists(pfad) and not ueberschreiben:
        return ziel
    s = open(quelle, encoding="utf-8").read()
    d0, d1 = s.index("<description>"), s.index("</description>")

    def sx(m):
        return f'{m.group(1)}="{float(m.group(2)) * faktor:g}"'
    desc = re.sub(r'<(line|rect|ellipse|polygon|arc|circle|terminal)\b[^>]*>',
                  lambda m: re.sub(r'\b(x\d?)="(-?[\d.]+)"', sx, m.group(0)), s[d0:d1])
    desc = re.sub(r'(<(?:rect|ellipse)\b[^>]*\bwidth=")(-?[\d.]+)"',
                  lambda m: f'{m.group(1)}{float(m.group(2)) * faktor:g}"', desc)
    if namen:
        def tn(m):
            t = m.group(0)
            x = round(float(re.search(r' x="(-?[\d.]+)"', t).group(1)))
            y = round(float(re.search(r' y="(-?[\d.]+)"', t).group(1)))
            return re.sub(r'name="[^"]*"', f'name="{namen.get((x, y), "")}"', t) if (x, y) in namen else t
        desc = re.sub(r'<terminal\b[^>]*>', tn, desc)
    s = s[:d0] + desc + s[d1:]
    s = re.sub(r'width="(\d+)"', lambda m: f'width="{int(-(-int(m.group(1)) * faktor // 10) * 10)}"', s, count=1)
    s = re.sub(r'hotspot_x="(\d+)"', lambda m: f'hotspot_x="{int(round(int(m.group(1)) * faktor / 10) * 10)}"', s, count=1)
    s = re.sub(r'<uuid uuid="\{[^}]*\}"', f'<uuid uuid="{{{uuid.uuid4()}}}"', s, count=1)
    if titel:
        s = re.sub(r'(<name lang="de">)[^<]*', r'\g<1>' + titel, s)
    os.makedirs(os.path.dirname(pfad), exist_ok=True)
    open(pfad, "w", encoding="utf-8").write(s)
    return ziel


def kasten2(rel, typ, oben, unten, hersteller="", artikel="", beschreibung="", abstand=30, ueberschreiben=False):
    """Gerätekasten mit Anschlüssen oben und unten (z. B. Trafo: Primär oben, Sekundär unten).
    oben/unten: [(Name, Bedeutung)]; Anschlüsse im Abstand `abstand` ab x = abstand."""
    pfad = os.path.join(COMPANY, rel)
    ziel = "company://" + rel.replace("\\", "/")
    if os.path.exists(pfad) and not ueberschreiben:
        return ziel
    n = max(len(oben), len(unten))
    breite = ((n + 1) * abstand + 9) // 10 * 10
    hoehe = 80
    t = [f'<rect x="0" y="0" width="{breite}" height="{hoehe}" '
         'style="line-style:normal;line-weight:normal;filling:none;color:black" antialias="false"/>',
         f'<text x="6" y="{hoehe // 2 + 4}" size="8" text="{escape(typ)}" rotation="0" color="#000000" '
         'font="Liberation Sans,8,-1,5,50,0,0,0,0,0,Regular"/>']
    for liste, y, ri in ((oben, 0, "n"), (unten, hoehe, "s")):
        for i, (name, bed) in enumerate(liste):
            x = abstand * (i + 1)
            t.append(f'<terminal uuid="{{{uuid.uuid4()}}}" name="{escape(name)}" x="{x}" y="{y}" orientation="{ri}" '
                     'type="Generic"/>')
            ty, tb = (y + 10, y + 18) if ri == "n" else (y - 12, y - 4)
            t.append(f'<text x="{x + 3}" y="{ty}" size="6" text="{escape(name)}" '
                     'rotation="0" color="#000000" font="Liberation Sans,6,-1,5,50,0,0,0,0,0,Regular"/>')
            if bed:
                t.append(f'<text x="{x + 3}" y="{tb}" size="5" text="{escape(bed)}" '
                         'rotation="0" color="#000000" font="Liberation Sans,5,-1,5,50,0,0,0,0,0,Regular"/>')
    infos = "".join(f'<elementInformation name="{k}" show="1">{escape(v)}</elementInformation>'
                    for k, v in (("manufacturer", hersteller), ("manufacturer_reference", artikel),
                                 ("designation", artikel), ("description", beschreibung)) if v)
    label = ('<dynamic_text x="0" y="-30" z="2" text_width="-1" Halignment="AlignLeft" Valignment="AlignTop" '
             'frame="false" rotation="0" keep_visual_rotation="false" text_from="ElementInfo" '
             f'uuid="{{{uuid.uuid4()}}}" font="Liberation Sans,10,-1,5,50,0,0,0,0,0,Regular">'
             '<text></text><info_name>label</info_name></dynamic_text>')
    xml = (f'<definition version="0.100.0" type="element" link_type="simple" width="{breite + 20}" '
           f'height="{hoehe + 40}" hotspot_x="10" hotspot_y="30">\n'
           f'<uuid uuid="{{{uuid.uuid4()}}}"/>\n<names><name lang="de">{escape(typ)}</name>'
           f'<name lang="en">{escape(typ)}</name></names>\n'
           f'<elementInformations>{infos}<elementInformation name="label" show="1"></elementInformation>'
           f'</elementInformations>\n<informations>Planvorlagen symbolbau.kasten2 (keine Herstellerdaten)</informations>\n'
           f'<description>{label}{"".join(t)}</description>\n</definition>\n')
    os.makedirs(os.path.dirname(pfad), exist_ok=True)
    open(pfad, "w", encoding="utf-8").write(xml)
    return ziel


def spiegeln_y(quelle, rel, titel=None, ueberschreiben=False):
    """Kanalmakro senkrecht spiegeln (Anschluss oben statt unten): Linien, Rechtecke, Anschlüsse y -> -y,
    Texte zeilenweise mitgenommen (Oberkante neu = -(alte Oberkante) - 9), Lesrichtung bleibt."""
    import re
    pfad = os.path.join(COMPANY, rel)
    ziel = "company://" + rel.replace("\\", "/")
    if os.path.exists(pfad) and not ueberschreiben:
        return ziel
    s = open(quelle, encoding="utf-8").read()
    d0, d1 = s.index("<description>"), s.index("</description>")

    def neg(m):
        return f'{m.group(1)}="{-float(m.group(2)):g}"'

    def form(m):
        t = m.group(0)
        if t.startswith("<rect"):
            y = float(re.search(r'\by="(-?[\d.]+)"', t).group(1))
            h = float(re.search(r'\bheight="(-?[\d.]+)"', t).group(1))
            return re.sub(r'\by="(-?[\d.]+)"', f'y="{-(y + h):g}"', t)
        t = re.sub(r'\b(y\d?)="(-?[\d.]+)"', neg, t)
        return t.replace('orientation="s"', 'orientation="X"').replace('orientation="n"', 'orientation="s"') \
                .replace('orientation="X"', 'orientation="n"')

    # Zeilen des Kastens (waagerechte Linien, vor dem Spiegeln): Texte bleiben in ihrer Zeile, gleicher Abstand
    # zur Zeilenoberkante – sonst rutschen sie auf die Zeilenlinien (QET zeichnet dynamische Texte mit Rand)
    zeilen = sorted({float(k.group(1)) for k in re.finditer(r'<line [^>]*?y1="(-?[\d.]+)" y2="\1"', s[d0:d1])})

    def text(m):
        t = m.group(0)
        if t.startswith("<dynamic_text"):
            y = float(re.search(r'\by="(-?[\d.]+)"', t).group(1))
            gr = re.search(r'font="[^,"]*,([\d.]+)', t)
            hoehe = 3.2 * (float(gr.group(1)) if gr else 5.0)     # gerenderte Höhe samt Rand
            mitte = y + hoehe / 2
            zeile = [(a, b) for a, b in zip(zeilen, zeilen[1:]) if a <= mitte <= b]
            if zeile:
                a, b = zeile[0]
                neu = -b + (y - a)
            else:
                neu = -y - 10                    # außerhalb des Kastens (BMK): wie bisher
            return re.sub(r'\by="(-?[\d.]+)"', f'y="{neu:g}"', t, count=1)
        m_g = re.search(r'\bsize="([\d.]+)"', t) or re.search(r'font="[^,"]*,([\d.]+)', t)
        g = float(m_g.group(1)) if m_g else 6.0                # Grundlinie -> Oberkante: Schriftgröße
        return re.sub(r'\by="(-?[\d.]+)"', lambda k: f'y="{-float(k.group(1)) + g:g}"', t, count=1)
    desc = re.sub(r'<(line|rect|polygon|terminal|ellipse)\b[^>]*>', form, s[d0:d1])
    desc = re.sub(r'<(text|dynamic_text)\b[^>]*>', text, desc)
    s = s[:d0] + desc + s[d1:]
    hy = re.search(r'hotspot_y="(\d+)"', s)
    h = re.search(r'\bheight="(\d+)"', s)
    if hy and h:
        s = s.replace(hy.group(0), f'hotspot_y="{int(h.group(1)) - int(hy.group(1))}"', 1)
    s = re.sub(r'<uuid uuid="\{[^}]*\}"', f'<uuid uuid="{{{uuid.uuid4()}}}"', s, count=1)
    if titel:
        s = re.sub(r'(<name lang="de">)[^<]*', r'\g<1>' + titel, s)
    os.makedirs(os.path.dirname(pfad), exist_ok=True)
    open(pfad, "w", encoding="utf-8").write(s)
    return ziel


# Bekannte Fehler in Herstellermakros (Data Portal), beim Erzeugen der lesbaren Variante korrigiert:
# 6ES7136-6DC00-0CA0 ist das F-DQ 8x24VDC/0,5A PP HF, das Kanalmakro trägt aber den Text des 2-A-PPM-Moduls.
MAKRO_KORREKTUR = {"6ES7136-6DC00-0CA0_DQ-PP-M_B1.elmt": {"F-DQ 2X24VDC/2A PPM HF": "F-DQ 8X 24VDC/0.5A PP HF"}}


def lesbar(quelle_pfad, faktor=1.35, ueberschreiben=False, schrift=1.25):
    """Lesbare Variante eines (zu klein gezeichneten) Herstellermakros. Die Anschlüsse bleiben exakt an ihrer
    Stelle (Bausteine und Leitungen passen unverändert); der Kasten wächst um `faktor` nach außen: Koordinaten
    zwischen den Anschlüssen bleiben, alles außerhalb wird vom äußersten Anschluss weg gestreckt. So bleiben die
    Anschlussstummel auf den Anschlüssen. Schriften wachsen um `schrift` (etwas weniger als der Kasten, sonst
    laufen lange Zeilen wie „F-DI 8x24 VDC HF  ET 200SP“ über den Rand).
    quelle_pfad: company://…/Datei.elmt -> company://…/lesbar/Datei.elmt"""
    import re
    rel = quelle_pfad[len("company://"):]
    ordner, datei = os.path.split(rel)
    ziel_rel = f"{ordner}/lesbar/{datei}"
    ziel = os.path.join(COMPANY, ziel_rel)
    if os.path.exists(ziel) and not ueberschreiben:
        return "company://" + ziel_rel
    s = open(os.path.join(COMPANY, rel), encoding="utf-8").read()
    for alt, neu in MAKRO_KORREKTUR.get(datei, {}).items():
        s = s.replace(f'text="{alt}"', f'text="{neu}"')
    d0, d1 = s.index("<description>"), s.index("</description>")
    term = [(float(m.group(1)), float(m.group(2))) for m in
            re.finditer(r'<terminal [^>]*?x="(-?[\d.]+)" y="(-?[\d.]+)"', s[d0:d1])]
    if not term:
        term = [(0.0, 0.0)]
    tx0, tx1 = min(t[0] for t in term), max(t[0] for t in term)
    ty0, ty1 = min(t[1] for t in term), max(t[1] for t in term)

    def strecke(v, lo, hi):
        return lo + (v - lo) * faktor if v < lo else (hi + (v - hi) * faktor if v > hi else v)

    def form(m):
        t = m.group(0)
        if t.startswith("<terminal"):
            return t
        t = re.sub(r'\b(x\d?)="(-?[\d.]+)"', lambda k: f'{k.group(1)}="{strecke(float(k.group(2)), tx0, tx1):g}"', t)
        t = re.sub(r'\b(y\d?)="(-?[\d.]+)"', lambda k: f'{k.group(1)}="{strecke(float(k.group(2)), ty0, ty1):g}"', t)
        t = re.sub(r'\b(width|height|rx|ry|text_width)="(-?[\d.]+)"',
                   lambda k: f'{k.group(1)}="{float(k.group(2)) * faktor:g}"' if float(k.group(2)) > 0 else k.group(0), t)
        t = re.sub(r'\bsize="([\d.]+)"', lambda k: f'size="{float(k.group(1)) * schrift:.1f}"', t)
        return re.sub(r'(font="[^,"]*,)([\d.]+)', lambda k: f'{k.group(1)}{float(k.group(2)) * schrift:.1f}', t)
    desc = re.sub(r'<(line|rect|ellipse|polygon|arc|circle|text|dynamic_text|terminal)\b[^>]*>', form, s[d0:d1])
    # Polygone: Punkte stehen als x1/y1, x2/y2 … – oben mit erfasst
    # statische Texte (Grundlinie y) nicht auf waagerechten Linien: liegt eine Linie zwischen Ober- und Grundlinie,
    # den Text knapp unter diese Linie schieben (z. B. die Modulzeile „F-DI 8x24 VDC HF  ET 200SP“ ganz unten)
    waag = sorted({float(m.group(1)) for m in re.finditer(r'<line [^>]*?y1="(-?[\d.]+)" y2="\1"', desc)})

    def frei(m):
        t = m.group(0)
        y = re.search(r'\by="(-?[\d.]+)"', t)
        gr = re.search(r'\bsize="([\d.]+)"', t) or re.search(r'font="[^,"]*,([\d.]+)', t)
        if not (y and gr):
            return t
        yb, h = float(y.group(1)), 0.75 * float(gr.group(1))
        stoer = [w for w in waag if yb - h < w < yb - 0.3]
        if not stoer:
            return t
        neu_y = max(stoer) + 0.8 + h
        return re.sub(r'\by="(-?[\d.]+)"', f'y="{neu_y:g}"', t, count=1)
    desc = re.sub(r'<text\b[^>]*>', frei, desc)
    s = s[:d0] + desc + s[d1:]
    kopf = re.search(r'<definition [^>]*>', s).group(0)
    k = {n: float(re.search(rf'\b{n}="(-?[\d.]+)"', kopf).group(1)) for n in ("width", "height", "hotspot_x", "hotspot_y")}
    links, rechts = strecke(-k["hotspot_x"], tx0, tx1), strecke(k["width"] - k["hotspot_x"], tx0, tx1)
    oben, unten = strecke(-k["hotspot_y"], ty0, ty1), strecke(k["height"] - k["hotspot_y"], ty0, ty1)
    rund = lambda v: int(-(-v // 10) * 10)          # auf volle 10 aufrunden (QET-Raster)
    werte = {"width": rund(rechts - links), "height": rund(unten - oben), "hotspot_x": rund(-links), "hotspot_y": rund(-oben)}
    neu = re.sub(r'\b(width|height|hotspot_x|hotspot_y)="(-?[\d.]+)"', lambda m: f'{m.group(1)}="{werte[m.group(1)]}"', kopf)
    s = s.replace(kopf, neu, 1)
    s = re.sub(r'<uuid uuid="\{[^}]*\}"', f'<uuid uuid="{{{uuid.uuid4()}}}"', s, count=1)
    os.makedirs(os.path.dirname(ziel), exist_ok=True)
    open(ziel, "w", encoding="utf-8").write(s)
    return "company://" + ziel_rel


# ---------------------------------------------------------------------------------------------------------------
def wechsler_feld(art="P", ueberschreiben=False):
    """Feldschalter als Wechsler 11/12/14 mit Betätigungskasten (EPLAN-Darstellung wie im Original): 11 oben (Wurzel),
    12 (Öffner) links unten, 14 (Schließer) unten unter 11. art: "P" Druck, "T" Temperatur (ϑ), "S" Schwimmer.
    Ruhestellung: Kontakt liegt auf 12 (Öffner geschlossen)."""
    namen = {"P": "Druckschalter", "T": "Temperaturschalter", "S": "Schwimmerschalter"}
    rel = f"Allgemein/Sensoren/Wechsler_Plan/{namen[art]}_Wechsler_11-12-14_Plan.elmt"
    ziel = os.path.join(COMPANY, rel)
    if os.path.exists(ziel) and not ueberschreiben:
        return "company://" + rel
    st = 'style="line-style:normal;line-weight:normal;filling:none;color:black"'
    gs = 'style="line-style:dashed;line-weight:normal;filling:none;color:black"'

    def l(x1, y1, x2, y2, s=st):
        return f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" end1="none" end2="none" length1="1.5" length2="1.5" antialias="true" {s}/>'

    def t(x, y, s, g=6):
        return f'<text x="{x}" y="{y}" text="{s}" rotation="0" font="Liberation Sans,{g},-1,5,50,0,0,0,0,0,Regular" color="#000000"/>'
    teile = [l(0, 0, 0, 10), l(0, 10, -11, 26),                    # Wurzel 11 und Schaltglied (liegt auf 12)
             l(-14, 26, -10, 26), l(-10, 26, -10, 40),             # Öffner 12
             l(0, 24, 0, 40),                                       # Schließer 14
             l(-28, 18, -6, 18, gs),                                # Wirkverbindung
             l(-43, 11, -28, 11), l(-28, 11, -28, 25), l(-28, 25, -43, 25), l(-43, 25, -43, 11),
             t(3, 8, "11"), t(-21, 38, "12"), t(3, 38, "14")]
    if art == "P":
        teile.append(t(-38.5, 21.5, "P", 7))
    elif art == "T":
        teile.append(t(-38.5, 21.5, "ϑ", 8))
    else:                                                          # Schwimmer: Kreis auf Flüssigkeitsspiegel
        teile += [f'<ellipse x="-38.5" y="13.5" width="6" height="6" antialias="true" {st}/>', l(-41, 21.5, -30, 21.5)]
    uid = lambda: "{" + str(uuid.uuid4()) + "}"
    xml = f'''<definition version="0.100.0" type="element" link_type="simple" width="70" height="70" hotspot_x="50" hotspot_y="20">
    <uuid uuid="{uid()}"/>
    <names>
        <name lang="de">{namen[art]} Wechsler 11/12/14 (Plan)</name>
        <name lang="en">{ {"P": "Pressure", "T": "Temperature", "S": "Float"}[art]} switch changeover 11/12/14</name>
    </names>
    <informations>Erzeugt mit planvorlagen.symbolbau.wechsler_feld (Darstellung wie EPLAN-Original).</informations>
    <description>
        {"".join(teile)}
        <dynamic_text x="-62" y="-6" z="3" text_width="-1" Halignment="AlignLeft" Valignment="AlignTop" frame="false" rotation="0" keep_visual_rotation="false" text_from="ElementInfo" uuid="{uid()}" font="Liberation Sans,9,-1,5,50,0,0,0,0,0,Regular">
            <text></text>
            <info_name>label</info_name>
        </dynamic_text>
        <terminal uuid="{uid()}" name="11" x="0" y="0" orientation="n" type="Generic"/>
        <terminal uuid="{uid()}" name="12" x="-10" y="40" orientation="s" type="Generic"/>
        <terminal uuid="{uid()}" name="14" x="0" y="40" orientation="s" type="Generic"/>
    </description>
</definition>
'''
    os.makedirs(os.path.dirname(ziel), exist_ok=True)
    qd = os.path.join(os.path.dirname(ziel), "qet_directory")
    if not os.path.exists(qd):
        open(qd, "w", encoding="utf-8").write('<qet-directory>\n    <names>\n        <name lang="de">Wechsler (Plan)</name>\n'
                                              '        <name lang="en">Changeover (plan)</name>\n    </names>\n</qet-directory>\n')
    open(ziel, "w", encoding="utf-8").write(xml)
    return "company://" + rel
