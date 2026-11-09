"""Bausteine für Achsen und Signalkopplungen (Servoumrichter, Messsysteme, Endschalter, CNC <-> SPS).

Entstanden am Plan Projekt G (Blätter 40–48: =DC Y-Achse, =EA X-Achse), allgemein gehalten:

- `geraet_symbol`      Gerätekasten mit frei gelegten Anschlüssen oben/unten (Name intern, Nummer + Bedeutung sichtbar),
                       Benutzerfeldern und festen Texten – für Teilansichten ohne Herstellermakro (Steckerleisten von
                       Optionskarten, Encodereingängen, Kanälen fremder Steuerungen).
- `kanal_kasten`       Kanal einer fremden E/A-Baugruppe (z. B. Cybelec IOM-DI8O2-1A) im Stil der SPS-Kanalkästen:
                       Kopf (Typ, Hersteller), Kanal, Funktion, Stecker + Pin; Anschluss oben (Eingang) oder unten (Ausgang).
- `endschalter`        Positionsschalter mit Schließer und Öffner im Feld, je Kontakt Signal über Kabelader an eine
                       Initiatorenklemme, Versorgung über die zweite Ader an die „+“-Ebene (Plusschiene).
- `signalkopplung`     Ausgang -> Eingang zweier Steuerungen (CNC-DO -> SPS-DI, SPS-DQ -> CNC-DI), optional Abzweig
                       als Potentialpfeil zu einem weiteren Blatt.
- `geber_strecke`      Messsystem (Linearmaßstab/Encoder) über Stecker, Kabel und eine durchschleifende Baugruppe
                       (Sicherheitssteuerung) zum Encodereingang der Achssteuerung.
- `servoumrichter`     Einachsumrichter (SEW MOVIDRIVE MDX91A, Herstellermakro) mit Motorschutzschalter, Versorgung,
                       STO am F-DQ, Bremswiderstand mit Temperaturschalter, Servomotor mit Encoder, Störmelderelais.
- Hilfen: `pfeil_hoch`, `pfeil_rechts`, `pfeil_links`, `schiene_offen` (Schiene, die auf dem Blatt endet),
  `ader` (Aderfarbe/Querschnitt), `pfeilpaar` (Abbruchstelle auf demselben Blatt verknüpfen), `kontaktnummern`.

Kern (`blatt.py`, `bausteine.py`) bleibt unverändert; alles Zusätzliche liegt hier.
"""
import os
import uuid
from xml.sax.saxutils import escape

from . import symbole as S
from .blatt import BAND, raster, spalte_links, spalte_mitte, _dreh
from .bausteine import _infos, klemmen, kabel, sps_kanal, rahmen, funktion_bei, F_DQ_KANAL, POL
from .symbole import COMPANY

FONT = "Liberation Sans,{g},-1,5,50,0,0,0,0,0,Regular"
WECHSLER = S.U + "Allgemein/Schaltglieder/Wechsler_Original/Wechsler_Original.elmt"        # 1 oben, 2 unten, 4 rechts
LINEARMASSSTAB = S.U + "Allgemein/Messsysteme/Linearmassstab_TTL/Linearmassstab_TTL_mit_Kabel_1m_Plan.elmt"
RUNDSTECKER_12 = (S.U + "Allgemein/Steckverbinder/Rundsteckverbinder_12polig_Paar_Plan/"
                  "Rundsteckverbinder_12-polig_Stecker+Buchse_9_belegt_Plan.elmt")         # XA:n oben, XB:n unten
PE_GEHAEUSE = S.U + "Allgemein/Verbindungen/PE_Anschluss_Plan/PE-Anschluss_Gehaeuse_Plan.elmt"   # PE unten


# --- Symbole -----------------------------------------------------------------------------------------
def _txt(x, y, s, g, rot=0):
    return (f'<text x="{x:g}" y="{y:g}" size="{g}" text="{escape(s)}" rotation="{rot}" color="#000000" '
            f'font="{FONT.format(g=g)}"/>')


def _feld(x, y, g, breite=-1, ausrichtung="AlignLeft", info=None):
    quelle = (f'text_from="ElementInfo"', f'<text></text><info_name>{info}</info_name>') if info else \
        ('text_from="UserText"', '<text></text>')
    return (f'<dynamic_text x="{x:g}" y="{y:g}" z="2" text_width="{breite}" Halignment="{ausrichtung}" '
            f'Valignment="AlignTop" frame="false" rotation="0" keep_visual_rotation="false" {quelle[0]} '
            f'uuid="{{{uuid.uuid4()}}}" font="{FONT.format(g=g)}">{quelle[1]}</dynamic_text>')


def geraet_symbol(rel, titel, pins, breite, hoehe, texte=(), linien=(), felder=(), hersteller="", artikel="",
                  beschreibung="", label_xy=(0, -24), link_type="simple", ueberschreiben=False):
    """Gerätekasten 0..breite x 0..hoehe (Ursprung oben links) in die Firmensammlung schreiben.

    pins:   [(name, x, "o"|"u", nummer, bedeutung)] – Anschluss auf der Ober- oder Unterkante bei x; `name` ist
            der Anschlussname (eindeutig, z. B. "X52:6"), `nummer` die sichtbare Pinnummer, `bedeutung` klein senkrecht.
    texte:  [(x, y, text, größe)] feste Texte (y = Grundlinie); linien: [(x1, y1, x2, y2)];
    felder: [(x, y, größe, breite, ausrichtung)] Benutzerfelder in Reihenfolge (Blatt.setze(felder=[…])).
    Rückgabe: company://-Pfad. Bestehende Dateien bleiben (ueberschreiben=False)."""
    pfad = os.path.join(COMPANY, rel)
    ziel = "company://" + rel.replace("\\", "/")
    if os.path.exists(pfad) and not ueberschreiben:
        return ziel
    t = [f'<rect x="0" y="0" width="{breite}" height="{hoehe}" '
         'style="line-style:normal;line-weight:normal;filling:none;color:black" antialias="false"/>']
    for x1, y1, x2, y2 in linien:
        t.append(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" length1="1.5" length2="1.5" end1="none" '
                 'end2="none" style="line-style:normal;line-weight:thin;filling:none;color:black" antialias="false"/>')
    for name, x, seite, nummer, bed in pins:
        y, ri = (0, "n") if seite == "o" else (hoehe, "s")
        t.append(f'<ellipse x="{x - 2.5:g}" y="{y - 2.5:g}" width="5" height="5" '
                 'style="line-style:normal;line-weight:normal;filling:white;color:black" antialias="true"/>')
        t.append(f'<terminal uuid="{{{uuid.uuid4()}}}" name="{escape(name)}" x="{x}" y="{y}" orientation="{ri}" '
                 'type="Generic"/>')
        if nummer:
            t.append(_txt(x + 4, y + 10 if seite == "o" else y - 4, nummer, 6))
        if bed:
            lang = 3 * len(bed)
            yb = (y + 14 + lang) if seite == "o" else (y - 12)
            t.append(_txt(x - 2, yb, bed, 4, 270))
    for x, y, s, g in texte:
        t.append(_txt(x, y, s, g))
    dyn = [_feld(label_xy[0], label_xy[1], 10, info="label")]
    for x, y, g, b, a in felder:
        dyn.append(_feld(x, y, g, b, a))
    infos = "".join(f'<elementInformation name="{k}" show="1">{escape(v)}</elementInformation>'
                    for k, v in (("manufacturer", hersteller), ("manufacturer_reference", artikel),
                                 ("designation", artikel), ("description", beschreibung)) if v)
    oben = min(-30, label_xy[1] - 6)
    unten = max(hoehe + 10, label_xy[1] + 20)
    xml = (f'<definition version="0.100.0" type="element" link_type="{link_type}" width="{breite + 20}" '
           f'height="{(unten - oben + 9) // 10 * 10}" hotspot_x="10" hotspot_y="{-oben}">\n'
           f'<uuid uuid="{{{uuid.uuid4()}}}"/>\n<names><name lang="de">{escape(titel)}</name>'
           f'<name lang="en">{escape(titel)}</name></names>\n'
           f'<elementInformations>{infos}<elementInformation name="label" show="1"></elementInformation>'
           f'</elementInformations>\n<informations>Planvorlagen bausteine_achsen.geraet_symbol</informations>\n'
           f'<description>{"".join(dyn)}{"".join(t)}</description>\n</definition>\n')
    os.makedirs(os.path.dirname(pfad), exist_ok=True)
    with open(pfad, "w", encoding="utf-8") as f:
        f.write(xml)
    return ziel


KANAL_B, KANAL_H, KANAL_PIN = 140, 100, 40          # wie die SPS-Kanalkästen: Anschluss 40 px vom linken Rand


def kanal_kasten(rel, typ, hersteller, eingang, artikel="", beschreibung="", ueberschreiben=False):
    """Kanal einer E/A-Baugruppe ohne Herstellermakro als Kasten 140 x 100 (Anschluss „K“ bei x = 40).
    eingang=False (Ausgang): Kopf oben (Typ, Hersteller), Kanal, Funktion, unten Stecker + Pin, Anschluss unten.
    eingang=True: Anschluss oben mit Stecker + Pin, Funktion, Kanal, Fuß unten (Typ, Hersteller).
    Felder (Reihenfolge für setze): Kanal, Funktion, Stecker, Pin. Rückgabe company://-Pfad; der Anschluss liegt
    bei Einfügen mit anker="K" genau auf x/y."""
    b, h, p = KANAL_B, KANAL_H, KANAL_PIN
    if eingang:
        texte = [(3, h - 6, typ, 7), (b - 40, h - 6, hersteller, 4)]
        linien = [(0, 22, b, 22), (0, h - 18, b, h - 18)]
        felder = [(0, 52, 8, b, "AlignHCenter"), (0, 30, 8, b, "AlignHCenter"),
                  (p - 22, 3, 6, -1, "AlignLeft"), (p + 6, 3, 6, -1, "AlignLeft")]
        pins = [("K", p, "o", "", "")]
    else:
        texte = [(3, 12, typ, 7), (b - 40, 10, hersteller, 4)]
        linien = [(0, 18, b, 18), (0, h - 22, b, h - 22)]
        felder = [(0, 28, 8, b, "AlignHCenter"), (0, 46, 8, b, "AlignHCenter"),
                  (p - 22, h - 16, 6, -1, "AlignLeft"), (p + 6, h - 16, 6, -1, "AlignLeft")]
        pins = [("K", p, "u", "", "")]
    return geraet_symbol(rel, f"{typ} {'Eingang' if eingang else 'Ausgang'} (Kanal)", pins, b, h, texte, linien,
                         felder, hersteller=hersteller, artikel=artikel or typ, beschreibung=beschreibung,
                         label_xy=(0, h + 2) if eingang else (0, -22), ueberschreiben=ueberschreiben)


def makro_ohne_bruecken(quelle, rel, x_von, x_bis, y_bis=-20, texte=("WH",), titel=None, ueberschreiben=False):
    """Variante eines Herstellermakros ohne die vorgezeichneten Lieferbrücken (z. B. STO-Brücken am MOVIDRIVE X6):
    entfernt Linien, die zwischen x_von..x_bis liegen und bis y <= y_bis über die Anschlüsse reichen, sowie die
    Aderfarbtexte `texte` der Brücken. Nötig, wenn die Anschlüsse im Plan anders verdrahtet sind (STO am F-DQ).
    quelle: company://-Pfad des Makros; Rückgabe company://-Pfad der Variante (gleicher Typordner)."""
    import re
    pfad = os.path.join(COMPANY, rel)
    ziel = "company://" + rel.replace("\\", "/")
    if os.path.exists(pfad) and not ueberschreiben:
        return ziel
    s = open(S.datei(quelle), encoding="utf-8").read()

    def weg(m):
        t = m.group(0)
        if t.startswith("<line"):
            xs = [float(v) for v in re.findall(r'\bx[12]="(-?[\d.]+)"', t)]
            ys = [float(v) for v in re.findall(r'\by[12]="(-?[\d.]+)"', t)]
            if xs and ys and x_von <= min(xs) and max(xs) <= x_bis and min(ys) <= y_bis:
                return ""
        else:
            mx, my = re.search(r'\sx="(-?[\d.]+)"', t), re.search(r'\sy="(-?[\d.]+)"', t)
            mt = re.search(r'\stext="([^"]*)"', t)
            if mx and my and mt and mt.group(1) in texte and x_von <= float(mx.group(1)) <= x_bis \
                    and float(my.group(1)) <= y_bis + 5:
                return ""
        return t
    s = re.sub(r"<(?:line|text)\b[^>]*/>", weg, s)
    s = re.sub(r'<uuid uuid="\{[^}]*\}"', f'<uuid uuid="{{{uuid.uuid4()}}}"', s, count=1)
    if titel:
        s = re.sub(r'(<name lang="de">)[^<]*', r'\g<1>' + escape(titel), s)
    os.makedirs(os.path.dirname(pfad), exist_ok=True)
    open(pfad, "w", encoding="utf-8").write(s)
    return ziel


# --- kleine Hilfen -------------------------------------------------------------------------------------
def kanal(b, pfad, x, y, k, anker):
    """bausteine.sps_kanal, Kennzeichen aber links neben der Leitung (lange Kennzeichen wie „=SB-A4“ liegen sonst
    auf der Leitung zum Kanal)."""
    t = sps_kanal(b, pfad, x, y, k, anker)
    b.ops.append({"op": "set_element_text", "folio": b.folio, "element": t.ref, "index": 0, "property": "x",
                  "value": str(-10 - 6.5 * len(k.get("bmk", "")))})
    return t


def _pfeil_y(pfad, dreh, y):
    a = S.symbol(pfad).anschluss(0)
    dy = _dreh(a.x, a.y, dreh)[1]
    return raster(y - dy) + dy


def pfeil_hoch(b, name, x, y, text=True, melden=True):
    """Gehender Potentialpfeil nach oben: Leitung kommt von unten an (x, y), Pfeil zeigt hinauf.
    melden=False: nicht über den Namen blattübergreifend verknüpfen (Abbruchstelle auf demselben Blatt)."""
    t = b.setze(S.GEHEND, x, _pfeil_y(S.GEHEND, 270, y), anker=0, drehung=270)
    if melden:
        b.plan.quellen.append((b.folio, name, t))
    if text:
        b.text(name, x - 6, t.pos(0)[1] - 34, "funktion", drehung=270)
    return t


def pfeil_rechts(b, name, x, y, text=True):
    """Gehender Pfeil nach rechts: Leitung kommt von links an (x, y)."""
    x = raster(x + 13) - 13                       # Ursprung des Pfeils auf das Raster (Anschluss 13 px links)
    t = b.setze(S.GEHEND, x, y, anker=0)
    b.plan.quellen.append((b.folio, name, t))
    if text:
        b.text(name, x + 2, y - 20, "funktion")
    return t


def pfeil_links(b, name, x, y, text=True):
    """Kommender Pfeil von links: Potential kommt bei (x, y) herein, Leitung geht nach rechts weiter."""
    x = raster(x - 13) + 13                       # Ursprung auf das Raster (Anschluss 13 px rechts)
    t = b.setze(S.KOMMEND, x, y, anker=0)
    b.plan.ziele.append((b.folio, name, t))
    if text:
        b.text(name, x - 40 - 6 * len(name), y - 20, "funktion")
    return t


def pfeil_unten_kommend(b, name, x, y, text_rechts=True):
    """Kommender Pfeil von unten (zeigt hinauf), Name waagerecht klein rechts daneben (wenig Platz darunter)."""
    t = b.pfeil(name, x, y, "von_unten", text=False)
    if text_rechts:
        b.text(name, x + 6, y + 6, "verweis")
    return t


def pfeilpaar(b, gehend, kommend):
    """Abbruchstelle auf demselben Blatt (gehend -> kommend) verknüpfen."""
    b.plan.links.append((b.folio, gehend, b.folio, kommend))


def ader(b, x, y, *zeilen):
    """Aderfarbe/Querschnitt klein rechts neben eine senkrechte Leitung (schrankinterne Verdrahtung)."""
    for i, z in enumerate(zeilen):
        b.text(z, x + 4, y + 7 * i, 5)


def kontaktnummern(b, t, oben, unten, dx=6):
    """Kontaktnummern (13/14, 11/12 …) an einen Kontakt mit Anschluss oben (Index 0) und unten (Index 1)."""
    xo, yo = t.pos(0)
    xu, yu = t.pos(1)
    b.text(oben, xo + dx, yo + 2, 6)
    b.text(unten, xu + dx, yu - 10, 6)


def schiene_offen(b, name, y, x_start, xs, richtung="oben"):
    """Potential, das bei x_start von links kommt (Pfeil) und auf dem Blatt endet: Abzweige bei xs, der letzte als
    Ecke. richtung "oben": Geräte über der Schiene (Abgang nach oben), sonst darunter.
    Rückgabe {x: (Teil, Anschluss)} für die Leitungen zu den Geräten."""
    k = pfeil_links(b, name, x_start, y, text=False)
    b.text(name, x_start + 8, y - 22, "funktion")
    oben = richtung == "oben"
    pfad = S.ABZWEIG_OBEN if oben else S.ABZWEIG
    ab_an = "N" if oben else "S"
    vorher, an_v, raus = k, 0, {}
    xs = sorted(xs)
    for i, x in enumerate(xs):
        if i == len(xs) - 1:
            e = b.setze(S.ECKE, x, y, anker="N")
            b.draht(vorher, an_v, e, "W")
            raus[x] = (e, ab_an)
        else:
            sym = S.symbol(pfad)
            w, ab = sym.anschluss("W"), sym.anschluss(ab_an)
            t = b.setze(pfad, x - ab.x, y - w.y)
            b.draht(vorher, an_v, t, "W")
            vorher, an_v = t, "E"
            raus[x] = (t, ab_an)
    return raus


# --- Endschalter --------------------------------------------------------------------------------------
_POS_QUELLE = S.C + "390_sensors_instruments/41_limit_switches/fin_de_course_{}.elmt"


def positionsschalter(art, ueberschreiben=False):
    """Positionsschalter-Kontakt (IEC 60617, mechanisch betätigt) als eigenständiges Bauteil (nicht Slave, zeigt sein
    Kennzeichen selbst): QET-Normsymbol mit Anschlüssen auf dem 10-px-Raster (±20 statt ±21) und IEC-Namen
    13/14 (art "NO") bzw. 11/12 ("NC"); Kennzeichen links vom Kontakt. Rückgabe company://-Pfad."""
    import re
    no = art == "NO"
    rel = ("Allgemein/Schaltglieder/Positionsschalter_Plan/"
           + ("Positionsschalter_Schliesser_13-14_Plan.elmt" if no else "Positionsschalter_Oeffner_11-12_Plan.elmt"))
    pfad = os.path.join(COMPANY, rel)
    ziel = "company://" + rel
    if os.path.exists(pfad) and not ueberschreiben:
        return ziel
    s = open(S.datei(_POS_QUELLE.format("no" if no else "nf")), encoding="utf-8").read()
    oben, unten = ("13", "14") if no else ("11", "12")
    s = re.sub(r'<terminal uuid="\{[^}]*\}" name="" x="0" y="-21"',
               f'<terminal uuid="{{{uuid.uuid4()}}}" name="{oben}" x="0" y="-20"', s)
    s = re.sub(r'<terminal uuid="\{[^}]*\}" name="" x="0" y="21"',
               f'<terminal uuid="{{{uuid.uuid4()}}}" name="{unten}" x="0" y="20"', s)
    s = re.sub(r'(<dynamic_text x=")[^"]*(" y=")[^"]*', r'\g<1>-52\g<2>-6', s, count=1)
    s = re.sub(r'<uuid uuid="\{[^}]*\}"', f'<uuid uuid="{{{uuid.uuid4()}}}"', s, count=1)
    s = re.sub(r'hotspot_x="\d+"', 'hotspot_x="60"', s, count=1)
    s = re.sub(r'width="20"', 'width="80"', s, count=1)
    titel = f"Positionsschalter {'Schließer 13/14' if no else 'Öffner 11/12'} (Plan)"
    s = re.sub(r'(<name lang="de">)[^<]*', r'\g<1>' + titel, s)
    if 'link_type="simple"' not in s:
        s = s.replace("<definition ", '<definition link_type="simple" ', 1)
    s = s.replace("<informations>", "<informations>Planvorlagen bausteine_achsen.positionsschalter aus QET "
                                    "fin_de_course; ", 1)
    os.makedirs(os.path.dirname(pfad), exist_ok=True)
    open(pfad, "w", encoding="utf-8").write(s)
    return ziel


def endschalter(b, x, y, geraet, kontakte, kabel_feld, leiste, y_kabel, y_klemme, plus, ort=None, rahmen_xy=None):
    """Positionsschalter im Feld mit mehreren Kontakten (Schließer 13/14, Öffner 11/12) nebeneinander ab x.

    kontakte: [{"art": "NO"|"NC", "klemme": 24 | None, "adern": ("1", "2")}] – je Kontakt zwei Leiter: links das
    Signal (unterer Anschluss -> Ader -> Initiatorenklemme), rechts die Versorgung (oberer Anschluss oben herum ->
    Ader -> „+“-Ebene = Abgriff an der Plusschiene `plus`). klemme None: Kontakt nicht verdrahtet (nur gezeichnet).
    kabel_feld: {name, typ} – Adern in Kreuzungsreihenfolge aus den Kontakten.
    Rückgabe [Klemme oder None je Kontakt] für die Leitung zum Kanal/Relais (Klemme Anschluss 1 = unten)."""
    raus, adern, letzte_x, erste_x, teile = [], [], None, None, []
    for i, k in enumerate(kontakte):
        xs, xp = x + 120 * i, x + 120 * i + 30
        pfad = positionsschalter(k["art"])
        nr = ("13", "14") if k["art"] == "NO" else ("11", "12")
        t = b.setze(pfad, xs, y, anker=0, label=geraet["bmk"], infos=_infos(geraet) if i == 0 else None)
        if i > 0:                          # Kennzeichen nur am ersten Kontakt sichtbar (Datum bleibt im Bauteil)
            b.ops.append({"op": "delete_element_text", "folio": b.folio, "element": t.ref, "index": 0})
        kontaktnummern(b, t, *nr)
        teile.append(t)
        if k.get("klemme") is None:
            raus.append(None)
            continue
        kl = klemmen(b, leiste, [k["klemme"]], [xs], y_klemme)[0]
        b.draht(t, 1, kl, 0)
        y_oben = y - 40
        if isinstance(plus, dict):                 # Abgriffe einer Schiene, die auf dem Blatt endet (schiene_offen)
            a, an = plus[xp]
            y_s = a.pos(an)[1]
        else:
            a, an = b.abgriff(plus, xp, "oben")
            y_s = b.schienen[plus].y
        b.leitung(t, 0, a, an, ueber=[(xs, y_oben), (xp, y_oben)])
        b.text("+", xp + 4, y_s - 16, 6)
        adern += list(k.get("adern", ("", "")))
        letzte_x = xp
        erste_x = xs if erste_x is None else erste_x
        raus.append(kl)
    if len(teile) > 1:                     # Wirkverbindung der Kontakte
        i = b._id("mech")
        y_m = y + 20
        b.ops += [{"op": "add_shape", "folio": b.folio, "shape": "line", "x1": teile[0].pos(0)[0] - 6, "y1": y_m,
                   "x2": teile[-1].pos(0)[0] - 6, "y2": y_m, "id": i},
                  {"op": "set_shape", "folio": b.folio, "index": f"${i}", "property": "line-style", "value": "dashed"}]
    if kabel_feld and letzte_x:
        kabel(b, dict(kabel_feld, adern=adern), erste_x - 12, letzte_x + 12, y_kabel)
    if ort and rahmen_xy:
        rahmen(b, *rahmen_xy, ort)
    return raus


# --- Signalkopplung zweier Steuerungen ----------------------------------------------------------------
def signalkopplung(b, x, quelle, ziel, y_oben=230, y_unten=700, abzweig=None, funktion=None, ader_text=None):
    """Ein Signal von einem Ausgang (oben, Anschluss unten) zu einem Eingang (unten, Anschluss oben) auf x.

    quelle/ziel: {"art": "kasten", "pfad", "bmk", "felder": [...], "infos"} (fremde Baugruppe, kanal_kasten) oder
                 {"art": "sps", "pfad", "kanal": {...}} (SPS-Kanalmakro über bausteine.sps_kanal).
    abzweig: {"name": "SP", "y": 450} – T-Abzweig nach rechts auf einen gehenden Potentialpfeil (anderes Blatt).
    Rückgabe (Quelle, Ziel)."""
    def setze(d, y, anker_sps):
        if d["art"] == "sps":
            return kanal(b, d["pfad"], x, y, d["kanal"], anker_sps), anker_sps
        return b.setze(d["pfad"], x, y, anker="K", label=d.get("bmk", ""), infos=d.get("infos"),
                       felder=d.get("felder")), "K"
    q, q_an = setze(quelle, y_oben, "DQ")
    z, z_an = setze(ziel, y_unten, "DI")
    if abzweig:
        ya = raster(abzweig.get("y", (y_oben + y_unten) / 2))
        e = b.setze(S.ECKE, x, ya, anker="N")
        b.draht(q, q_an, e, "N")
        b.draht(e, "S", z, z_an)
        p = pfeil_rechts(b, abzweig["name"], x + 60, ya)
        b.draht(e, "E", p, 0)
    else:
        b.draht(q, q_an, z, z_an)
    if ader_text:
        ader(b, x, y_unten - 60, *ader_text)
    if funktion:
        funktion_bei(b, x + 30, funktion)
    return q, z


# --- Messsystem über Stecker, Kabel und durchschleifende Baugruppe -------------------------------------
def geber_strecke(b, x, geber, stecker, kabel1, durch, kabel2, eingang, ys, ort="+MASCHINE", funktion=None):
    """Linearmaßstab/Encoder mit Signalen nebeneinander (Abstand 30 px ab x):
        Geber (Anschlüsse unten) -> Rundstecker (oben Geräteseite, unten Kabelseite) -> Kabel 1 ->
        durchschleifende Baugruppe (Anschlüsse oben Eingang, unten Ausgang, z. B. Sicherheitssteuerung) ->
        Kabel 2 -> Encodereingang der Achssteuerung (Anschlüsse oben).
    geber/stecker/durch/eingang: {"pfad", "bmk", "pins": [Anschlussnamen in Signalreihenfolge], "infos", "label_xy"};
    durch["pins_aus"] = Ausgänge. Der Schirm (letzter Geberpin) endet am Stecker (Schirm auf Steckergehäuse).
    ys: {"geber", "stecker", "kabel1", "durch", "kabel2", "eingang"} – Anschlusslagen."""
    t = {}
    for key in ("geber", "stecker", "durch", "eingang"):
        d = locals()[key]
        t[key] = b.setze(d["pfad"], x, ys[key], anker=d["pins"][0], label=d.get("bmk", ""),
                         infos=d.get("infos"), label_xy=d.get("label_xy"))
    n = len(durch["pins"])
    for g, s in zip(geber["pins"], stecker["pins"]):
        b.draht(t["geber"], g, t["stecker"], s)
    for i in range(n):
        b.draht(t["stecker"], stecker["pins_aus"][i], t["durch"], durch["pins"][i])
        b.draht(t["durch"], durch["pins_aus"][i], t["eingang"], eingang["pins"][i])
    x_ende = x + 30 * (n - 1)
    kabel(b, kabel1, x - 12, x_ende + 12, ys["kabel1"])
    kabel(b, kabel2, x - 12, x_ende + 12, ys["kabel2"])
    if ort:
        rahmen(b, x - 70, ys["geber"] - 210, x + 30 * len(geber["pins"]) + 60, ys["stecker"] + 15, ort)
    if funktion:
        funktion_bei(b, x + 15 * (n - 1), funktion)
    return t


# --- Servoumrichter -----------------------------------------------------------------------------------
def servoumrichter(b, x, y, umrichter, schutz, motor, geber, bremswiderstand, versorgung, sto, binaer, relais,
                   kabel_motor, kabel_geber, kabel_br, kabel_temp, ort="+MASCHINE"):
    """Einachsumrichter mit Herstellermakro (Anschlüsse oben: Netz L1-L3, PE, X5 24V/GND, X6 STO, X22 Relais,
    X20 Binäreingänge; unten: Motor U/V/W, +R/-R, X15 Geber). x/y = Lage von L1.

    umrichter: {"pfad", "bmk", "infos", "pins": {Bedeutung: Anschluss}} – Bedeutungen L1 L2 L3 PE 24V GND STO_P
               STO_M DOR_C DOR_NC DI00 DI_GND U V W +R -R und geber_pins (Liste X15-Anschlüsse in Aderfolge).
    schutz: Motorschutzschalter {bmk, infos, daten}; versorgung: {"netz": ("L1","L2","L3"), "pe": "PE",
            "24V": "11L1+", "GND": "11M1"} – Potentialnamen der kommenden Pfeile.
    sto: SPS-Kanal (F-DQ PP/M) {bmk, start, n, adresse, symbol, funktion}.
    binaer: {"leiste": "-XG5.3", "klemmen": [1, 2], "plus": "4L8+", "gnd": "4M5"} – Temperaturschalter des
            Bremswiderstands über Klemmen an DI00, Versorgung vom Potential `plus`.
    relais: {"plus": "4L4+", "stoerung": "Servo_Stg"} – Störmelderelais X22: DOR-C an Plus, DOR-NC als Pfeil.
    motor/geber: {"pfad", "bmk", "infos", "pins"}; bremswiderstand: {"pfad", "bmk", "infos", "daten"}.
    Rückgabe dict der Teile (u. a. "schutz" als Master für Hilfskontakte auf anderen Blättern)."""
    P = umrichter["pins"]
    t = {}
    t["fu"] = b.setze(umrichter["pfad"], x, y, anker=P["L1"], label=umrichter["bmk"], infos=umrichter.get("infos"),
                      label_xy=umrichter.get("label_xy"))
    fu = t["fu"]

    def px(n):
        return fu.pos(P[n])[0]
    # Netz: Pfeile -> Motorschutzschalter -> L1..L3 (Polabstand 30 -> 20 über Winkel)
    x_fc = px("L2")
    y_fc = y - 230
    t["schutz"] = b.setze(S.MOTORSCHUTZ, x_fc, y_fc, anker="3", label=schutz["bmk"], infos=_infos(schutz))
    for i, z in enumerate(schutz.get("daten", [])):
        b.rechts_text(z, x_fc - POL - 44, y_fc + 18 + 9 * i, "daten")
    for name, pin in zip(versorgung["netz"], ("1", "3", "5")):
        xa = t["schutz"].pos(pin)[0]
        p = b.pfeil(name, xa, y_fc - 60, "von_oben")
        b.draht(p, 0, t["schutz"], pin)
    for i, (a, n) in enumerate((("2", "L1"), ("4", "L2"), ("6", "L3"))):
        xa, xn = t["schutz"].pos(a)[0], px(n)
        if xa == xn:
            b.draht(t["schutz"], a, fu, P[n])
        else:
            yj = y - 90 + 20 * (i // 2)
            b.leitung(t["schutz"], a, fu, P[n], ueber=[(xa, yj), (xn, yj)])
        ader(b, xn, y - 120, "BK", "2,5")
    p = b.pfeil(versorgung["pe"], px("PE"), y - 80, "von_oben")
    b.draht(p, 0, fu, P["PE"])
    for k in ("24V", "GND"):
        p = b.pfeil(versorgung[k], px(k), y - 80, "von_oben")
        b.draht(p, 0, fu, P[k])
    ader(b, px("24V"), y - 30, "DBU", "1")
    ader(b, px("GND"), y - 30, "DBUWH", "1")

    # STO am F-DQ (PP -> STO_P, M -> STO_M)
    x_sto = px("STO_P")
    t["sto"] = sps_kanal(b, F_DQ_KANAL, x_sto, y - 130, sto, "DQ-PP")
    b.draht(t["sto"], "DQ-PP", fu, P["STO_P"])
    xm, xn = t["sto"].pos("M")[0], px("STO_M")
    b.leitung(t["sto"], "M", fu, P["STO_M"], ueber=[(xm, y - 60), (xn, y - 60)])
    ader(b, x_sto, y - 40, "DBU", "0,5")

    # Störmelderelais X22: DOR-C an Plus (Schiene über dem Umrichter), DOR-NC als gehender Pfeil hinauf
    a, an = b.abgriff(relais["plus"], px("DOR_C"))
    b.draht(a, an, fu, P["DOR_C"])
    p = pfeil_hoch(b, relais["stoerung"], px("DOR_NC"), y - 40)
    b.draht(fu, P["DOR_NC"], p, 0)

    # Temperaturschalter Bremswiderstand -> Klemmen -> DI00; Versorgung von unten, GND des Eingangs von oben
    x2 = px("DI00")
    x1 = x2 + 30                                  # Versorgungsklemme rechts daneben (links liegt X22)
    y_k = y - 110
    k1, k2 = klemmen(b, binaer["leiste"], binaer["klemmen"], [x1, x2], y_k)
    b.draht(k2, 1, fu, P["DI00"])
    ader(b, x2, y - 40, "DBU", "0,5")
    pv = pfeil_unten_kommend(b, binaer["plus"], x1, y_k + 60)
    b.draht(k1, 1, pv, 0)
    xg = px("DI_GND")
    eg = b.setze(S.ECKE, xg + 30, y, anker="N")
    b.draht(fu, P["DI_GND"], eg, "W")
    pg = b.pfeil(binaer["gnd"], xg + 30, y - 80, "von_oben")
    b.draht(pg, 0, eg, "N")
    # Bremswiderstand unter dem Umrichter (um br["versatz"] nach links, damit X15 frei bleibt), Temperaturschalter
    # T1/T2 als Abbruchstelle hinauf zu den Klemmen
    br = bremswiderstand
    y_br = y + 300
    x_rb = px("+R") + br.get("versatz", 0)
    t["br"] = b.setze(br["pfad"], x_rb, y_br, anker=br["pins"]["RB1"], label=br["bmk"], infos=br.get("infos"),
                      label_xy=br.get("label_xy"))
    links = t["br"].pos(br["pins"]["T1"])[0] - 220          # linke Kastenkante (Makro: T1 220 px vom Rand)
    for i, z in enumerate(br.get("daten", [])):
        b.rechts_text(z, links - 6, y_br + 20 + 9 * i, "daten")
    for i, (a, z) in enumerate((("+R", "RB1"), ("-R", "RB2"))):
        xa, xz = px(a), t["br"].pos(br["pins"][z])[0]
        if xa == xz:
            b.draht(fu, P[a], t["br"], br["pins"][z])
        else:
            yj = y + 190 + 10 * i
            b.leitung(fu, P[a], t["br"], br["pins"][z], ueber=[(xa, yj), (xz, yj)])
    xpe = t["br"].pos(br["pins"]["PE"])[0]
    pe_br = b.pfeil(versorgung["pe"], xpe, y_br - 30, "von_oben")
    b.draht(pe_br, 0, t["br"], br["pins"]["PE"])
    kabel(b, kabel_br, x_rb - 12, xpe + 12, y_br - 10)
    for (pin, name), xk in zip(((br.get("an_klemme1", "T2"), br["temp"] + ".1"),
                                (br.get("an_klemme2", "T1"), br["temp"] + ".2")), (x1, x2)):
        xt = t["br"].pos(br["pins"][pin])[0]
        g = pfeil_hoch(b, name, xt, y_br - 60, text=False, melden=False)
        b.draht(t["br"], br["pins"][pin], g, 0)
        k = b.pfeil(name, xk, y_k - 110, "von_oben", text=False)
        b.draht(k, 0, k1 if xk == x1 else k2, 0)
        pfeilpaar(b, g, k)
    b.text(br["temp"], x2 - 4, y_k - 162, "funktion")
    b.text(br["temp"], t["br"].pos(br["pins"]["T1"])[0] - 4, y_br - 100, "funktion")
    if kabel_temp and x1 > x2:                    # Adern in Kreuzungsreihenfolge von links
        kabel_temp = dict(kabel_temp, adern=list(reversed(kabel_temp.get("adern", []))))
    kabel(b, kabel_temp, min(x1, x2) - 12, max(x1, x2) + 12, y_k - 70)

    # Motor U/V/W + PE über das Motorkabel
    y_m = y + 460
    m = motor
    t["motor"] = b.setze(m["pfad"], px("V"), y_m, anker=m["pins"]["V"], label=m["bmk"], infos=m.get("infos"),
                         label_xy=m.get("label_xy"))
    for i, n in enumerate(("U", "V", "W")):
        xa, xm = px(n), t["motor"].pos(m["pins"][n])[0]
        if xa == xm:
            b.draht(fu, P[n], t["motor"], m["pins"][n])
        else:
            yj = y_m - 60 + 10 * (i // 2)
            b.leitung(fu, P[n], t["motor"], m["pins"][n], ueber=[(xa, yj), (xm, yj)])
    xpe = t["motor"].pos(m["pins"]["PE"])[0]
    pe_m = b.setze(PE_GEHAEUSE, xpe, y + 260, anker="PE")
    b.draht(pe_m, "PE", t["motor"], m["pins"]["PE"])
    # Schutzleiter des Motorkabels endet auf dem Gehäuse-PE (Schirmklemme X2) des Umrichters – benennen,
    # sonst liest es sich als offenes Aderende
    b.text("X2 Gehäuse-PE", xpe + 8, y + 252, "daten")
    kabel(b, kabel_motor, px("U") - 12, xpe + 12, y_m - 110)
    # Geber (Steckeranschluss des Motors) an X15
    g = geber
    px_x15 = fu.pos(P["geber"][0])[0]
    t["geber"] = b.setze(g["pfad"], px_x15, y_m, anker=g["pins"][0], label=g["bmk"], infos=g.get("infos"),
                         label_xy=g.get("label_xy"))
    for a, z in zip(P["geber"], g["pins"]):
        b.draht(fu, a, t["geber"], z)
    kabel(b, kabel_geber, px_x15 - 12, fu.pos(P["geber"][-1])[0] + 12, y_m - 70)
    if ort:
        rahmen(b, fu.x - 120, y_m - 20, fu.pos(P["geber"][-1])[0] + 50, y_m + 95, ort)
    return t
