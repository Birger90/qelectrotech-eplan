"""EPLAN-Symbolgrafik und Seitengrafik aus einem entpackten .zw1 nach QElectroTech.

- Symbolbibliotheken: je Funktion Bibliothek (ef4d), Symbolnummer (ef4e = eb52 im Symbol) und Variante
  (ef4f = ed96 der Variante, fehlt = A). Die Variante trägt die Anschlusspunkte (edbe Lage, edbf Richtung
  1 oben, 2 rechts, 4 unten, 8 links) und besitzt die Grafikelemente (Instance, ebf1 = Variante).
- Grafik: 31 Linie, 32 Bogen/Kreis, 34 Polylinie, 44 Spline, 89/128 Rechteck, 30/83 Text.
- Textausrichtung ee26 (am Original-PDF gemessen): 1–3 oben, 4–6 Mitte, 7–9 unten, 10–12 Grundlinie,
  jeweils links/mittig/rechts; fehlt = 7. Texte im Format „??_??@Text;“ oder „de_DE@…;en_US@…;“.
- Koordinaten in mm, Ursprung unten links, y nach oben.
Ergebnis: QET-Elementdateien je Symbolvariante (mit Anschlüssen) und Formen/Texte für die Seitengrafik."""
import collections
import html
import json
import math
import os
import re
import uuid

import eod_symbole as E

HIER = os.path.dirname(os.path.abspath(__file__))
K = 1560 / 420                    # px je mm
X0, H_MM = 20, 297
RICHTUNG = {1: "n", 2: "e", 4: "s", 8: "w"}
# Typen der Grafik-Eigenschaften (fest, damit der Mehrheitsentscheid sie nicht verbiegt)
# Schrift der Texte: EPLAN-Schrifttabelle (Firmeneinstellung GedViewer.Fonts, nicht im Projektarchiv); belegt durch die
# EPLAN-PDFs aller geprüften Projekte (eingebettete Schrift Tahoma = Schrift 0). Abweichend per EPLAN_SCHRIFT.
SCHRIFT = os.environ.get("EPLAN_SCHRIFT", "Tahoma")
# Diagnose: EPLAN_TEXTPROTOKOLL=<datei> -> jeder Blatttext mit Herkunft (KONTEXT setzt der Aufrufer) als JSON-Zeile
PROTOKOLL = [] if os.environ.get("EPLAN_TEXTPROTOKOLL") else None
KONTEXT = {}
# Schriftmaße. EPLAN (BaseTypes ETextGraphicsTools::setFont): Zellhöhe = 1,5 × Texthöhe = Ober- + Unterlänge der
# Schrift (Tahoma usWinAscent/Descent 2049/423 bei 2048 Einheiten) -> Geviert = 1,5·2048/2472 = 1,243 × Texthöhe;
# belegt am Original-PDF (2,5 mm -> 8,8 pt, 1,8 mm -> 6,34 pt, 1,5 mm -> 5,28 pt).
# QET (gemessen am eigenen PDF-Export): Pixelgröße = round(pt·4/3), ein Szenen-px je Geviert-px; QGraphicsTextItem
# (Blatttext, Bauteil-Kennzeichen) mit 4 px Rand: erster Buchstabe bei x+4, Grundlinie bei y+4+Oberlänge;
# Bauteiltexte <text> (ElementPictureFactory::parseText) ohne Rand, x/y = Grundlinie links bzw. mit
# anchor="alignment" Mitte/rechts; Laufweiten = Tahoma-hmtx × Pixelgröße.
TAHOMA_EM = 1.5 * 2048 / 2472
ZELLE = 1.5                     # Zellhöhe der Schrift / Texthöhe (ETextGraphicsTools::setFont)
TAHOMA_EM_MASSSTAB, ZELLE_MASSSTAB = TAHOMA_EM, ZELLE


def schrift_einstellen(einstellungen, programmversion):
    """Projekt-Kompatibilität „Texte vergrößert anzeigen“ (Projekteinstellung GedViewer.TypeFontCreation):
    0 = vergrößert wie alte Versionen, 1 = maßstabsgetreu. Bis EPLAN 2.9 setzte die Einstellung 0 die Schrift mit
    der Zellhöhe als Geviert (Geviert 1,5 × h statt 1,5 · 2048/2472 × h, alle Textmaße × 2472/2048 = +20,7 %); ab
    EPLAN 2023 gibt es den Schalter im Code nicht mehr (2025: PDF-Schriftgröße = getEplanToPdfSizeFactor, ohne
    Abfrage), alle Projekte maßstabsgetreu. Programmversion = ProjectInfo 10043 (zuletzt verwendete Version)."""
    global TAHOMA_EM, ZELLE
    try:
        alt = int(str(programmversion or "").split(".")[0]) < 2000
    except ValueError:
        alt = False
    v = 2472 / 2048 if alt and str(einstellungen.get("GedViewer.TypeFontCreation", "1")) == "0" else 1.0
    TAHOMA_EM, ZELLE = TAHOMA_EM_MASSSTAB * v, ZELLE_MASSSTAB * v
    return v
QET_RAND = 4
_LAUFWEITE = {}


def _laufweiten():
    if not _LAUFWEITE:
        from fontTools.ttLib import TTFont
        f = TTFont(os.path.join(os.environ.get("WINDIR", r"C:\Windows"), "Fonts", "tahoma.ttf"))
        upm, hm = f["head"].unitsPerEm, f["hmtx"]
        _LAUFWEITE.update({cp: hm[g][0] / upm for cp, g in f.getBestCmap().items()})
        _LAUFWEITE[None] = hm[".notdef"][0] / upm
    return _LAUFWEITE


def schrift_px(h_mm):
    """Pixelgröße der QET-Schrift, deren Geviert EPLANs Texthöhe h entspricht."""
    return max(1, round(h_mm * TAHOMA_EM * K))


def schrift_pt(h_mm):
    """Punktgröße für QET (Vielfaches von 0,75 pt, damit Qt genau schrift_px Pixel wählt)."""
    return schrift_px(h_mm) * 0.75


def breite_px(zeile, px):
    w = _laufweiten()
    return sum(w.get(ord(c), w[None]) for c in zeile) * px


def grundlinie_px(h_mm, ausr):
    """Lage der Grundlinie der ersten Zeile unter dem EPLAN-Anker (Szenen-px, y nach unten) je Ausrichtung:
    oben = Zellenoberkante, Mitte = Zellenmitte, unten = Zellenunterkante, 10–12 = Grundlinie."""
    zelle = ZELLE * h_mm * K
    oben = zelle * TAHOMA_OBEN
    a = ausr or 7
    if a in (1, 2, 3):
        return oben
    if a in (4, 5, 6):
        return oben - zelle / 2
    if a in (7, 8, 9):
        return oben - zelle
    return 0.0
GRAFIK_TYPEN = {0xec73: "p16", 0xec74: "p16", 0xec89: "d8", 0xec8a: "d8", 0xec8b: "p16", 0xec8c: "d8",
                0xeccd: "p16", 0xf0d3: "p16", 0xf0d4: "p16", 0xf0d5: "d8", 0xec5f: "str", 0xec5a: "s2",
                0xebfe: "d8", 0xebff: "i4", 0xee21: "d8", 0xee22: "d8", 0xee25: "s2", 0xee26: "b1",
                0xee27: "d8", 0xee28: "d8", 0xed96: "b1", 0xed97: "s2", 0xedbe: "p16", 0xedbf: "b1",
                0xedbd: "obj", 0xeb51: "str", 0xeb52: "s2", 0xef4d: "s2", 0xef4e: "s2", 0xef4f: "b1",
                0xf0e7: "str", 0xf141: "p16", 0xf144: "p16", 0xf147: "p16", 0xf148: "p16", 0xf149: "p16",
                0xf14a: "str"}
# Ebene (EPLAN-Layer-Nummer) -> (Schrifthöhe mm, Ausrichtung): Vorgaben für Texte ohne eigene Angaben
EBENEN = {}
# Ebenen, die nicht gedruckt werden (Layer.eod efef = 0 bzw. HIDDEN/VIS); gilt auch für Eigenschaftstexte (textlagen)
NICHT_GEDRUCKT = set()
# Farben (EPLAN 2025, REA): Farbnummer = Index der EPLAN-Farbtabelle (EColorTable, short). Gespeichert: Grafik ebfd
# (DmgVisibleInstance-Stift +0xe4, getColorId; ebfc = Linientyp +0xe6), Text ee24 im Textformat (DmgTextInfo +0x42),
# Ebene eff4 (DmgPersistentLayer::setColorId +0x11c, setzt Stift- und Textfarbe der Ebene zugleich). -16002 (gespeichert
# 49534) = „von Ebene“ (EColor::isColorDefinedByLayer, DmgPen::validateByLayer); das ist die Konstruktor-Vorgabe von
# Stift (DmgVisibleInstance) und Textformat (DmgTextInfo), also auch der Wert, wenn nichts gespeichert ist.
# Sonderfarben (EColorTable::getColor, calculateSpecialColorsForPrinter): -3 Hintergrund, -4 Hintergrund abgesetzt
# (Druck: Kanal - 30 bei hellem Hintergrund), -5 Gegenfarbe des Hintergrunds. PDF-Export „Schwarz-weiß“ (EPDFExportLogu:
# Einstellung OutputColor = 1 -> Farbtabelle „_BlackOnWhite“; EColorTableMap-Konstruktor: alle Tabellenfarben schwarz,
# Sonderfarben mit weißem Hintergrund) -> nur -3 weiß und -4 #E1E1E1 weichen von Schwarz ab (am Projekt A-Original bestätigt:
# Projektdatum Ebene 454, eff4 65532, Text #E1E1E1).
FARBE_VON_EBENE = 49534
EBENENFARBE = {}
# Ebene -> Linienbreite (Layer.eod eff5, mm): gilt für Grafik ohne eigene Breite (ebfe fehlt oder 0)
EBENENBREITE = {}


def linienbreite(p):
    """Linienbreite einer Grafik (mm): eigene Breite ebfe, sonst die der Ebene (eff5). Belegt am Original Projekt D:
    /1 Balken ebfe 2,0 -> PDF 1,99 mm, /21 ebfe 0,13 -> 0,129 mm, ohne ebfe auf Ebene 100 (0,25) -> 0,249 mm."""
    b = E.wert(p, 0xebfe)
    if not isinstance(b, (int, float)) or b <= 0:
        b = EBENENBREITE.get(E.wert(p, 0xebfb))
    return b if isinstance(b, (int, float)) and b > 0 else None


def textstil(fmt):
    """Schriftstil eines Textformats (ec59, Eigenschaft 60965): Bit 0 fett, Bit 1 kursiv (Original Projekt D:
    „KUNDE GmbH“ 16385 -> Tahoma-Bold, „SIEMENS“ 1 -> Tahoma-Bold, normale Texte 16384)."""
    f = E.wert(fmt, 60965) if fmt else None
    f = f if isinstance(f, int) else 0
    return {"fett": bool(f & 1), "kursiv": bool(f & 2)}


def _stil(f):
    """Zusatzangaben einer Grundform (letztes Feld, dict): breite (mm), fett, kursiv."""
    return f[-1] if f and f[0] != "R" and isinstance(f[-1], dict) else {}
SW_DRUCK = {65533: "#ffffff", 65532: "#e1e1e1"}


def farbe(farb_id, ebene):
    """EPLAN-Farbnummer (ggf. „von Ebene“) -> QET-Farbe wie im Schwarz-weiß-PDF; None = schwarz."""
    if farb_id is None or farb_id == FARBE_VON_EBENE:
        farb_id = EBENENFARBE.get(ebene)
    return SW_DRUCK.get(farb_id)
BIBLIOTHEKEN =("SPECIAL.sdb", "IEC_symbol.sdb", "IEC_single_symbol.sdb", "GRAPHICS.sdb")


def _seed():
    s = json.load(open(os.path.join(HIER, "daten_typen", "seed.json")))
    t = {int(k, 16): v for k, v in s.items()}
    t.update(GRAFIK_TYPEN)
    return t


def skaliert(f, s):
    """Grundform eines maßstäblichen Blatts (Modellmaße) auf Papiermaß: Lagen und Schrifthöhe /s."""
    if not s or s == 1:
        return f
    def q(p):
        return (p[0] / s, p[1] / s)
    if f[0] == "L":
        return ("L", q(f[1]), q(f[2])) + tuple(f[3:])
    if f[0] == "A":
        return ("A", q(f[1]), f[2] / s) + tuple(f[3:])
    if f[0] == "P":
        return ("P", [q(p) for p in f[1]]) + tuple(f[2:])
    if f[0] == "R":
        return ("R", q(f[1]), q(f[2]), f[3])
    return ("T", q(f[1]), f[2], f[3] / s) + tuple(f[4:])


def ohne_uebersetzungsmarke(s):
    """„{{Text}}“ = als nicht zu übersetzend gekennzeichneter Text (EPLAN-Hilfe translategui_k_mehrsprachigetexte:
    Darstellung im Dialog als {{+A}}); im Plan steht nur der Text."""
    return re.sub(r"\{\{(.*?)\}\}", r"\1", s) if s and "{{" in s else s


def text_de(s):
    """'??_??@Text;' / 'de_DE@Text;en_US@…;' -> 'Text'"""
    if not s:
        return ""
    for muster in (r"de_DE@([^;]*)", r"\?\?_\?\?@([^;]*)", r"##_##@([^;]*)"):
        m = re.search(muster, s)
        if m:
            return ohne_uebersetzungsmarke(m.group(1))
    m = re.match(r"[a-z]{2}_[A-Z]{2}@([^;]*)", s)
    return ohne_uebersetzungsmarke(m.group(1) if m else s)


def _ohne_klammern(t):
    """EPLAN-Makrotexte stehen als {{Text}} im Symbol."""
    return re.sub(r"\{\{([^{}]*)\}\}", lambda m: m.group(1), t)


# --- Grafikelemente -> Grundformen (mm, y nach oben) --------------------------------------------------

def form(cls, p, ausgeblendet=()):
    w = E.wert
    if w(p, 0xebfb) in ausgeblendet:
        return None
    gestrichelt = (w(p, 0xebfd) or 0) not in (0, 65531)
    fb = farbe(w(p, 0xebfd), w(p, 0xebfb))           # Stiftfarbe (letztes Tupelfeld; None = schwarz)
    st = {"breite": linienbreite(p)}
    if cls == 31:
        return ("L", w(p, 0xec73) or (0.0, 0.0), w(p, 0xec74) or (0.0, 0.0), gestrichelt, fb, st)
    if cls == 32:
        r = w(p, 0xec8c)
        if not isinstance(r, float) or r <= 0:
            return None
        return ("A", w(p, 0xec8b) or (0.0, 0.0), r, w(p, 0xec89) or 0.0, w(p, 0xec8a) or 0.0, gestrichelt, fb, st)
    if cls in (34, 44):
        pts = [v for v in w(p, 0xeccd, alle=True) if isinstance(v, tuple)]
        if len(pts) < 2:
            return None
        if cls == 44 and len(pts) == 4:                     # kubischer Bézier in 12 Stücken
            a, b, c, d = pts
            pts = [tuple((1 - t) ** 3 * a[i] + 3 * (1 - t) ** 2 * t * b[i] + 3 * (1 - t) * t * t * c[i] + t ** 3 * d[i]
                         for i in (0, 1)) for t in [k / 12 for k in range(13)]]
        geschlossen = len(pts) > 2 and math.dist(pts[0], pts[-1]) < 1e-6
        gefuellt = bool((w(p, 0xebf4) or 0) & 64) and geschlossen
        return ("P", pts, geschlossen, gefuellt, gestrichelt, fb, st)
    if cls in (89, 128):
        a, b = w(p, 0xf0d3), w(p, 0xf0d4)
        if not a or not b:
            return None
        ang = w(p, 0xf0d5) or 0.0
        ecken = [(a[0], a[1]), (b[0], a[1]), (b[0], b[1]), (a[0], b[1])]
        if ang:
            c, s = math.cos(ang), math.sin(ang)
            ecken = [(a[0] + (x - a[0]) * c - (y - a[1]) * s, a[1] + (x - a[0]) * s + (y - a[1]) * c) for x, y in ecken]
        return ("P", ecken + [ecken[0]], True, bool((w(p, 0xebf4) or 0) & 64), gestrichelt, fb, st)
    if cls in (30, 83, 52):            # 52 = ausgewerteter Platzhaltertext (Formular/Auswertung)
        t = _ohne_klammern(text_de(w(p, 0xec5f) or ""))
        # „&…“-Texte (Makro-/Artikelkürzel wie „&KU.M25“) liegen fast alle auf der Ebene 1028 „Grafik.Steuertext“
        # (Layer.eod efef = 0, nicht gedruckt) und entfallen über die Ebenenregel; auf druckenden Ebenen zeigt EPLAN
        # sie (Projekt E-Schrankaufbau, Ebene 1006: „&KU.M12“, „&D5,0“) -> kein Sonderfall für „&“
        if not t.strip():
            return None
        fmt = (E.objekte(p, 0xec59) or [[]])[0]
        eb = EBENEN.get(w(p, 0xebfb), (2.5, 7))
        # Ausrichtungsbox feste Breite (ee25 & 0x70 = 0x10, Breite ee27 = DmgTextInfo +0x28, setBox): EPLAN bricht um.
        # Freie Texte nur mit ebf4 Bit 0 (Archivbefund: alle umgebrochenen Seitentexte tragen ebf4 33/49, alle nicht
        # umgebrochenen kein ebf4; Bedeutung des Bits im EPLAN-Code noch nicht gefunden, Abschnitt 19 „Offen“)
        if ((w(fmt, 0xee25) or 0) & 0x70) == 0x10 and w(fmt, 0xee27) and \
                (cls == 52 or (w(p, 0xebf4) or 0) & 1):                        # auch ausgewertete Texte (52)
            t = umbrechen(t, w(fmt, 0xee27), w(fmt, 0xee21) or eb[0])
        return ("T", w(p, 0xec55) or (0.0, 0.0), t, w(fmt, 0xee21) or eb[0], w(fmt, 0xee22) or 0.0,
                w(fmt, 0xee26) or eb[1], farbe(w(fmt, 0xee24), w(p, 0xebfb)), textstil(fmt))
    return None


# --- Ausgabe als QET-Element (relativ zum Einfügepunkt) ------------------------------------------------

STIL = "line-style:{s};line-weight:{w};filling:{f};color:black"


def strichstaerke(breite):
    """EPLAN-Linienbreite (mm) -> QET-Strichstärke eines Elements (ElementPictureFactory: thin 0,5 px, normal 1 px,
    hight 2 px, eleve 5 px; 1 px = 1/K mm = 0,27 mm). Ohne Breite: thin wie bisher."""
    if not breite:
        return "thin"
    px = breite * K
    return "thin" if px < 0.7 else "normal" if px < 1.5 else "hight" if px < 3.5 else "eleve"


def schrift(pt, stil=None):
    """QFont-Text (family,pointSize,pixelSize,styleHint,weight,italic,underline,strikeOut,fixedPitch,rawMode,
    styleName) mit fett/kursiv aus dem EPLAN-Textformat."""
    stil = stil or {}
    fett, kursiv = bool(stil.get("fett")), bool(stil.get("kursiv"))
    if not fett and not kursiv:
        return f"{SCHRIFT},{_f(pt)},-1,5,50,0,0,0,0,0,Regular"
    # ohne Stilnamen: mit „Bold“ wählte Qt beim PDF-Export eine schräge Tahoma (gemessen); Gewicht/Kursiv genügen
    return f"{SCHRIFT},{_f(pt)},-1,5,{75 if fett else 50},{1 if kursiv else 0},0,0,0,0"


def _f(v):
    return f"{v:.2f}".rstrip("0").rstrip(".")


def _text_box(t, h_mm, ausr):
    """Lage der linken Grundlinie relativ zum Anker (px, y nach unten) für QET-Elementtexte."""
    """-> (pt, px, dx, dy): Versatz des linken Grundlinienpunkts der ersten (breitesten) Zeile."""
    px = schrift_px(h_mm)
    breite = max(breite_px(z, px) for z in t.split("\n"))
    spalte = ((ausr or 7) - 1) % 3
    return schrift_pt(h_mm), px, -breite * (0, 0.5, 1)[spalte], grundlinie_px(h_mm, ausr)


def mit_rolle(xml, rolle):
    """rolle: None | ('master', art) | ('slave', zustand, art) -> link_type und kindInformations (QET:
    Spule coil/Schutz protection/Schalter commutator; Kontakt state NO/NC/SW, type simple/power)."""
    if not rolle:
        return xml
    if rolle[0] == "master":
        kind = f'<kindInformation name="type">{rolle[1]}</kindInformation>'
        if rolle[1] == "plc" and len(rolle) > 2:
            # SPS-Karte: Kanaltabelle (ElementData::plcMasterDataFromXml) – je Kanal Art, Adresse, Funktionstext,
            # symbolische Adresse (comment) und Anschlüsse; Index = group_index der verknüpften Kanäle
            ios_ = "".join(
                f'<plcIO type="{html.escape(io_["type"], quote=True)}" address="{html.escape(io_["address"], quote=True)}" '
                f'functionText="{html.escape(io_["function"], quote=True)}" '
                f'comment="{html.escape(io_["comment"], quote=True)}" crossRef="" '
                f'terminalCount="{len(io_["terminals"])}">'
                + "".join(f"<terminal>{html.escape(t_)}</terminal>" for t_ in io_["terminals"]) + "</plcIO>"
                for io_ in rolle[2])
            kind += f'<plcMasterData rowHeight="8.00"><plcIOs>{ios_}</plcIOs></plcMasterData>'
    else:
        kind = (f'<kindInformation name="type">{rolle[2]}</kindInformation>'
                f'<kindInformation name="state">{rolle[1]}</kindInformation>'
                '<kindInformation name="number">1</kindInformation>')
    xml = xml.replace('link_type="simple"', f'link_type="{rolle[0]}"', 1)
    return xml.replace("    <description>", f"    <kindInformations>{kind}</kindInformations>\n    <description>", 1)


def element_xml(name, formen, anschluesse, beschreibung="", namen=None, infos=None):
    """formen in mm relativ zum Einfügepunkt; anschluesse = [(x_mm, y_mm, richtung)];
    namen: Anschlussnamen (sonst 1, 2, …); infos: Elementinformationen des Bauteils {Name: Wert}.
    Form ("D", Anker, Text, Höhe, Winkel, Ausrichtung, Info-Name): Text aus der Elementinformation (dynamic_text
    text_from="ElementInfo") an der Lage, die der Text als feste Form hätte; ein "D" mit Info „label“ ersetzt das
    Kennzeichen oben links."""
    teile, xs, ys, dyn = [], [0.0], [0.0], []
    mit_label = False

    def P(x, y):
        X, Y = x * K, -y * K
        xs.append(X)
        ys.append(Y)
        return X, Y
    for f in formen:
        if f[0] == "L":
            (x1, y1), (x2, y2) = P(*f[1]), P(*f[2])
            teile.append(f'<line x1="{_f(x1)}" y1="{_f(y1)}" x2="{_f(x2)}" y2="{_f(y2)}" length1="1.5" length2="1.5" '
                         f'end1="none" end2="none" style="{STIL.format(s="dashed" if f[3] else "normal", f="none", w=strichstaerke(_stil(f).get("breite")))}" '
                         f'antialias="true"/>')
        elif f[0] == "A":
            (cx, cy), r = f[1], f[2]
            P(cx - r, cy - r)
            P(cx + r, cy + r)
            X, Y = cx * K - r * K, -cy * K - r * K
            a0, a1 = math.degrees(f[3]), math.degrees(f[4])
            stil = STIL.format(s="dashed" if f[5] else "normal", f="none", w=strichstaerke(_stil(f).get("breite")))
            if abs(a1 - a0) < 1e-6 or abs(abs(a1 - a0) - 360) < 1e-6:
                teile.append(f'<ellipse x="{_f(X)}" y="{_f(Y)}" width="{_f(2 * r * K)}" height="{_f(2 * r * K)}" '
                             f'style="{stil}" antialias="true"/>')
            else:
                span = (a1 - a0) % 360 or 360
                teile.append(f'<arc x="{_f(X)}" y="{_f(Y)}" width="{_f(2 * r * K)}" height="{_f(2 * r * K)}" '
                             f'start="{_f(a0 % 360)}" angle="{_f(span)}" style="{stil}" antialias="true"/>')
        elif f[0] == "P":
            pts = [P(*q) for q in f[1]]
            if f[2]:
                pts = pts[:-1]
            attr = " ".join(f'x{i + 1}="{_f(x)}" y{i + 1}="{_f(y)}"' for i, (x, y) in enumerate(pts))
            teile.append(f'<polygon {attr} closed="{"true" if f[2] else "false"}" antialias="true" '
                         f'style="{STIL.format(s="dashed" if f[4] else "normal", f="black" if f[3] else "none", w=strichstaerke(_stil(f).get("breite")))}"/>')
        elif f[0] == "T":
            (x, y), t, h, ang, ausr = f[1], f[2], f[3], f[4], f[5]
            pt, px, _, dy = _text_box(t, h, ausr)
            spalte = ((ausr or 7) - 1) % 3
            ausr_attr = ('', ' Halignment="AlignHCenter" anchor="alignment"',
                         ' Halignment="AlignRight" anchor="alignment"')[spalte]
            X, Y = P(x, y)
            r = math.radians(-math.degrees(ang) % 360)
            c, s_ = math.cos(r), math.sin(r)
            for k, zeile in enumerate(t.split("\n")):
                ly = dy + k * ZELLE * h * K          # Zeilenabstand = Zellhöhe 1,5 × Texthöhe
                w = breite_px(zeile, px)
                tx, ty = X - ly * s_, Y + ly * c   # Grundlinienpunkt im gedrehten Textsystem
                teile.append(f'<text text="{html.escape(zeile, quote=True)}" x="{_f(tx)}" y="{_f(ty)}" '
                             f'rotation="{_f(-math.degrees(ang) % 360)}"{ausr_attr} '
                             f'font="{schrift(pt, _stil(f))}" color="#000000"/>')
                lx0 = -w * (0, 0.5, 1)[spalte]
                for ex, ey in ((lx0, -px), (lx0 + w, 0.3 * px)):
                    xs.append(tx + ex * c - ey * s_)
                    ys.append(ty + ex * s_ + ey * c)
        elif f[0] == "D":
            # Elementinformation als Text: QGraphicsTextItem, Lage = linke obere Ecke inkl. 4 px Rand, gedreht um diese
            # Ecke – dieselbe Rechnung wie text_auf_blatt (freie Texte), damit der Text dort steht, wo die feste Form
            # stand. Die Ausrichtung bleibt (QET hält beim Ändern des Werts die Kante bzw. Mitte fest).
            (x, y), t, h, ang, ausr, info_ = f[1], f[2], f[3], f[4], f[5], f[6]
            px, pt = schrift_px(h), schrift_pt(h)
            spalte = ((ausr or 7) - 1) % 3
            X, Y = P(x, y)
            w = breite_px(t, px)
            lx = -w * (0, 0.5, 1)[spalte] - QET_RAND
            ly = grundlinie_px(h, ausr) - QET_RAND - px
            r = -ang
            c, s_ = math.cos(r), math.sin(r)
            tx, ty = X + lx * c - ly * s_, Y + lx * s_ + ly * c
            for ex, ey in ((QET_RAND, QET_RAND), (QET_RAND + w, QET_RAND + 1.3 * px)):
                xs.append(tx + ex * c - ey * s_)
                ys.append(ty + ex * s_ + ey * c)
            halign = ("AlignLeft", "AlignHCenter", "AlignRight")[spalte]
            u_d = uuid.uuid5(uuid.NAMESPACE_URL, f"{name}/info/{info_}/{len(dyn)}")
            if info_ == "label":
                mit_label = True
            dyn.append(f'<dynamic_text x="{_f(tx)}" y="{_f(ty)}" z="3" text_width="-1" Halignment="{halign}" '
                       f'Valignment="AlignTop" frame="false" rotation="{_f(-math.degrees(ang) % 360)}" '
                       f'keep_visual_rotation="false" text_from="ElementInfo" uuid="{{{u_d}}}" '
                       f'font="{schrift(pt, _stil(f))}"><text>{html.escape(t)}</text>'
                       f'<info_name>{html.escape(info_)}</info_name></dynamic_text>')
    for i, (x, y, r) in enumerate(anschluesse):
        X, Y = P(x, y)
        u = uuid.uuid5(uuid.NAMESPACE_URL, f"{name}/{i}")
        nm = html.escape(namen[i] if namen and i < len(namen) else str(i + 1), quote=True)
        teile.append(f'<terminal uuid="{{{u}}}" name="{nm}" x="{_f(X)}" y="{_f(Y)}" '
                     f'orientation="{RICHTUNG.get(r, "s")}" type="Generic"/>')
    x0, x1 = math.floor(min(xs) / 10) * 10 - 10, math.ceil(max(xs) / 10) * 10 + 10
    y0, y1 = math.floor(min(ys) / 10) * 10 - 10, math.ceil(max(ys) / 10) * 10 + 10
    u = uuid.uuid5(uuid.NAMESPACE_URL, name)
    lab = uuid.uuid5(uuid.NAMESPACE_URL, name + "/label")
    ei = ""
    if infos:
        ei = ("    <elementInformations>" + "".join(
            f'<elementInformation name="{html.escape(k_, quote=True)}" show="1">{html.escape(str(v_))}'
            '</elementInformation>' for k_, v_ in infos.items()) + "</elementInformations>\n")
    label_dyn = "" if mit_label else (
        f'\n        <dynamic_text x="{x0 + 2}" y="{y0 - 2}" z="3" text_width="-1" Halignment="AlignLeft" '
        f'Valignment="AlignTop" frame="false" rotation="0" keep_visual_rotation="false" text_from="ElementInfo" '
        f'uuid="{{{lab}}}" font="{SCHRIFT},7,-1,5,50,0,0,0,0,0,Regular"><text></text>'
        f'<info_name>label</info_name></dynamic_text>')
    return (f'<definition version="0.100.0" type="element" link_type="simple" width="{x1 - x0}" height="{y1 - y0}" '
            f'hotspot_x="{-x0}" hotspot_y="{-y0}">\n'
            f'    <uuid uuid="{{{u}}}"/>\n    <names>\n        <name lang="de">{html.escape(name)}</name>\n'
            f'        <name lang="en">{html.escape(name)}</name>\n    </names>\n' + ei +
            f'    <informations>{html.escape(beschreibung)}</informations>\n    <description>\n        '
            + "\n        ".join(teile + dyn) + label_dyn +
            f'\n    </description>\n</definition>\n')


# --- Ausgabe als QET-Formen/Texte auf dem Blatt (Seitengrafik) -----------------------------------------

def _px(x, y):
    return X0 + x * K, (H_MM - y) * K


def _shape(typ, pts, gestrichelt=False, geschlossen=False, gefuellt=False, fb=None, breite=None):
    """breite: EPLAN-Linienbreite (mm) -> Stiftbreite in Szenen-px (K px je mm); ohne Breite 1 px wie bisher."""
    stil = "DashLine" if gestrichelt else "SolidLine"
    pen = f'<pen style="{stil}" color="{fb or "#000000"}" widthF="{_f(max(0.3, breite * K)) if breite else 1}"/>'
    brush = f'<brush style="{"SolidPattern" if gefuellt else "NoBrush"}" color="{fb or "#000000"}"/>'
    if typ == "Polygon":
        punkte = "".join(f'<point x="{_f(x)}" y="{_f(y)}"/>' for x, y in pts)
        return (f'<shape uuid="{{{uuid.uuid4()}}}" type="Polygon" is_movable="0" closed="{int(geschlossen)}" z="1">'
                f'{pen}{brush}<points>{punkte}</points></shape>')
    (x1, y1), (x2, y2) = pts
    return (f'<shape uuid="{{{uuid.uuid4()}}}" type="{typ}" is_movable="0" closed="0" x1="{_f(x1)}" y1="{_f(y1)}" '
            f'x2="{_f(x2)}" y2="{_f(y2)}" z="1">{pen}{brush}</shape>')


def _bogen_punkte(c, r, a0, a1, n=None):
    span = (a1 - a0) % (2 * math.pi) or 2 * math.pi
    n = n or max(6, int(span / (math.pi / 16)))
    return [(c[0] + r * math.cos(a0 + span * k / n), c[1] + r * math.sin(a0 + span * k / n)) for k in range(n + 1)]


def _input(t, x, y, pt, ang=0.0, fb=None, stil=None):
    font = schrift(pt, stil)
    inhalt = html.escape(t).replace("\n", "<br/>")
    if fb:                                  # <input> hat kein Farbattribut; QET liest den Text als HTML (setHtml)
        inhalt = f'<font color="{fb}">{inhalt}</font>'
    return (f'<input uuid="{{{uuid.uuid4()}}}" x="{_f(x)}" y="{_f(y)}" rotation="{_f(-math.degrees(ang) % 360)}" '
            f'font="{font}" text="{html.escape(inhalt, quote=True)}"/>')


def text_auf_blatt(t, anker, h_mm, ang, ausr, dx=0.0, dy=0.0, fb=None, stil=None):
    """Freier Text an EPLAN-Anker (mm) mit Ausrichtung -> <input> je Zeile. EPLAN richtet die ERSTE Zeile aus,
    weitere Zeilen laufen im Abstand 1,5 × Schrifthöhe nach unten (EPLAN: Zellhöhe der Schrift = 1,5 × Texthöhe,
    ETextGraphicsTools::setFont); jede Zeile wird für sich
    links/mittig/rechts ausgerichtet. QETs Textfeld: 4 px Rand, Grundlinie bei y + 4 + Oberlänge (gemessen)."""
    px, pt = schrift_px(h_mm), schrift_pt(h_mm)
    zeilen = t.split("\n")
    spalte = ((ausr or 7) - 1) % 3
    ax, ay = _px(anker[0] + dx, anker[1] + dy)
    ly0 = grundlinie_px(h_mm, ausr) - QET_RAND - px
    # Mehrzeilig: die Ausrichtung gilt für den ganzen Textblock (n Zellen à 1,5 h) – oben/Grundlinie: erste Zeile am
    # Anker, Mitte: Block mittig, unten: letzte Zeile am Anker (XGedBaseCalcDockPosition::
    # getNextPositionForDockingVertical 0x180268f80: Zeilen 2..n zählen bei 4–6 je zur Hälfte, bei 7–9 ganz über dem
    # Anker, sonst darunter)
    a_ = ausr or 7
    if a_ in (4, 5, 6, 7, 8, 9) and len(zeilen) > 1:
        ly0 -= (len(zeilen) - 1) * h_mm * ZELLE * K * (0.5 if a_ in (4, 5, 6) else 1.0)
    # gedrehter Text (EPLAN-Winkel gegen den Uhrzeigersinn): Versatz im Textsystem bestimmen und mitdrehen;
    # QET dreht ein Textfeld um seine linke obere Ecke (Uhrzeigersinn positiv, y nach unten)
    r = -ang
    c, s_ = math.cos(r), math.sin(r)
    if PROTOKOLL is not None:
        PROTOKOLL.append(dict(KONTEXT, t=t, x=anker[0] + dx, y=anker[1] + dy, h=h_mm, ang=ang, ausr=ausr))
    aus = []
    for k, z in enumerate(zeilen):
        if not z.strip():
            continue
        lx = -breite_px(z, px) * (0, 0.5, 1)[spalte] - QET_RAND
        ly = ly0 + k * h_mm * ZELLE * K          # Zeilenhöhe = Zellhöhe 1,5 × Texthöhe (ETextGraphicsTools::setFont)
        aus.append(_input(z, ax + lx * c - ly * s_, ay + lx * s_ + ly * c, pt, ang, fb, stil))
    return "".join(aus)


def _fb(f, i):
    """Farbe einer Grundform (letztes Feld aus form(); fehlt bei Formen anderer Herkunft -> schwarz)."""
    return f[i] if len(f) > i and isinstance(f[i], str) else None


def blatt_xml(formen, versatz=(0.0, 0.0)):
    """Grundformen (mm, absolut auf der Seite) -> (shapes, inputs) als XML-Strings."""
    shapes, inputs = [], []
    ox, oy = versatz
    for f in formen:
        if f[0] == "L":
            a, b = _px(f[1][0] + ox, f[1][1] + oy), _px(f[2][0] + ox, f[2][1] + oy)
            if math.dist(a, b) > 0.2:
                shapes.append(_shape("Line", [a, b], f[3], fb=_fb(f, 4), breite=_stil(f).get("breite")))
        elif f[0] == "A":
            c, r = (f[1][0] + ox, f[1][1] + oy), f[2]
            if abs(f[4] - f[3]) < 1e-6:
                p1, p2 = _px(c[0] - r, c[1] + r), _px(c[0] + r, c[1] - r)
                shapes.append(_shape("Ellipse", [p1, p2], f[5], fb=_fb(f, 6), breite=_stil(f).get("breite")))
            else:
                shapes.append(_shape("Polygon", [_px(*q) for q in _bogen_punkte(c, r, f[3], f[4])], f[5],
                                     fb=_fb(f, 6), breite=_stil(f).get("breite")))
        elif f[0] == "P":
            pts = [_px(x + ox, y + oy) for x, y in f[1]]
            if f[2]:
                pts = pts[:-1]
            shapes.append(_shape("Polygon", pts, f[4], f[2], f[3], fb=_fb(f, 5), breite=_stil(f).get("breite")))
        elif f[0] == "T":
            inputs.append(text_auf_blatt(f[2], (f[1][0] + ox, f[1][1] + oy), f[3], f[4], f[5], fb=_fb(f, 6),
                                         stil=_stil(f)))
        elif f[0] == "R":                                    # Rechteck mit Panel-Rolle (Fork-Schrankaufbau)
            a, b = _px(f[1][0] + ox, f[1][1] + oy), _px(f[2][0] + ox, f[2][1] + oy)
            p1 = (min(a[0], b[0]), min(a[1], b[1]))
            p2 = (max(a[0], b[0]), max(a[1], b[1]))
            attrs = " ".join(f'{k}="{html.escape(str(v), quote=True)}"' for k, v in f[3].items())
            shapes.append(_shape("Rectangle", [p1, p2]).replace("</shape>", f"<panel {attrs}/></shape>"))
    return shapes, inputs


def bild_xml(datei, x0, y0, x1, y1, max_px=1200):
    """Bild (Images-Ordner des Archivs) in das EPLAN-Rechteck (mm) einpassen -> <image> (PNG, base64)."""
    try:
        import base64
        import pymupdf
        pix = pymupdf.Pixmap(datei)
        while pix.width > max_px or pix.height > max_px:
            pix.shrink(1)
        daten = base64.b64encode(pix.tobytes("png")).decode()
    except Exception:
        return None
    links, oben = _px(min(x0, x1), max(y0, y1))
    rechts, unten = _px(max(x0, x1), min(y0, y1))
    skala = min((rechts - links) / pix.width, (unten - oben) / pix.height)
    # QET: Lage = linke obere Ecke des UNskalierten Bildes, skaliert wird um dessen Mitte
    x = (links + rechts) / 2 - pix.width / 2
    y = (oben + unten) / 2 - pix.height / 2
    return (f'<image uuid="{{{uuid.uuid4()}}}" x="{_f(x)}" y="{_f(y)}" z="0" rotation="0" size="{skala:.6f}" '
            f'is_movable="false">{daten}</image>')


def in_projekt_einfuegen(qet, je_blatt):
    """je_blatt: {folio_index: (shapes, inputs[, images])} in das gespeicherte .qet schreiben (nach der Erzeugung)."""
    s = open(qet, encoding="utf-8").read()
    teile = re.split(r"(<diagram [^>]*>)", s)
    folio = -1
    for k, teil in enumerate(teile):
        if teil.startswith("<diagram "):
            folio += 1
            continue
        if folio < 0 or folio not in je_blatt:
            continue
        shapes, inputs = je_blatt[folio][:2]
        bilder = je_blatt[folio][2] if len(je_blatt[folio]) > 2 else []
        ende = teil.find("</diagram>")
        kopf, rest = teil[:ende], teil[ende:]
        for tag, neu in (("shapes", shapes), ("inputs", inputs), ("images", bilder)):
            if not neu:
                continue
            # nur der Container des Blatts (Einrückung 8), nicht der gleichnamige eines Elements (Einrückung 16)
            m = re.search(r"\n {8}<" + tag + r"(/?)>", kopf)
            if m and m.group(1):
                kopf = kopf[:m.start()] + f"\n        <{tag}>" + "".join(neu) + f"</{tag}>" + kopf[m.end():]
            elif m:
                kopf = kopf[:m.end()] + "".join(neu) + kopf[m.end():]
            else:
                kopf += f"<{tag}>" + "".join(neu) + f"</{tag}>"
        teile[k] = kopf + rest
    open(qet, "w", encoding="utf-8").write("".join(teile))


# --- Bibliotheken und Seitengrafik laden ---------------------------------------------------------------

class Grafik:
    def __init__(self, x):
        self.x = x
        typen = _seed()
        self.sym, self.var, self.inst = {}, {}, {}
        self.libs = [b for b in BIBLIOTHEKEN if os.path.isdir(os.path.join(x, b))] + \
            sorted(d for d in os.listdir(x) if d.endswith(".sdb") and d not in BIBLIOTHEKEN)
        for lib in self.libs:
            self.sym[lib] = E.lesen(os.path.join(x, lib, "Symbol.eod"), typen)[0]
            self.var[lib] = E.lesen(os.path.join(x, lib, "Variant.eod"), typen)[0]
            inst = collections.defaultdict(list)
            for iid, (cls, p) in E.lesen(os.path.join(x, lib, "Instance.eod"), typen)[0].items():
                o = E.wert(p, 0xebf1) if p else None
                if o and o[0] == 45:
                    inst[o[1]].append((cls, p))
            self.inst[lib] = inst
        self.seite = E.lesen(os.path.join(x, "Instance.eod"), typen)[0]
        # ausgewertete Platzhaltertexte (Auswertungsseiten, Deckblatt) und Formulare (Name -> Formular-ID)
        pht = os.path.join(x, "PlaceHolderText.eod")
        self.platzhalter = E.lesen(pht, typen)[0] if os.path.exists(pht) else {}
        frm = os.path.join(x, "Form.eod")
        self.formulare, self.normblaetter, self.formularsatz = {}, {}, {}
        if os.path.exists(frm):
            for fid, (cls, p) in E.lesen(frm, typen)[0].items():
                # ede6 ist nur bei Formularsätzen der Name; andere Klassen (z. B. 169) tragen dort eine Eigenschaftsliste
                n = E.wert(p, 0xede6) if p else None
                if isinstance(n, str) and n:
                    # Formular = Name + Formulartyp (ede5; EPLAN: Dateiendung .fNN je Typ, gleicher Name in mehreren
                    # Typen möglich, z. B. KD_001 als Kabelübersicht 21 und Schaltschranklegende 29); die Seite trägt
                    # ihren Typ in ea8f, der Auswertungsblock in 61414
                    self.formulare.setdefault((n, E.wert(p, 0xede5)), fid)
                    self.formulare.setdefault(n, fid)
                    self.formularsatz[fid] = p
                    if cls == 21:                      # Normblatt (Plot frame)
                        self.normblaetter[n] = (fid, p)
        self.ausgeblendet = set()
        lay = os.path.join(x, "Layer.eod")
        if os.path.exists(lay):
            for _, (cls, p) in E.lesen(lay, typen)[0].items():
                if not p:
                    continue
                if (E.wert(p, 0xeff0) or "").upper() in ("HIDDEN", "VIS"):
                    self.ausgeblendet.add(E.wert(p, 0xeff2))
                # DmgPersistentLayer (EPLAN-Metamodell + EDmGraphicsu): efee = sichtbar (+0x100), efef = drucken
                # (+0x101), eff9 = gesperrt (+0x102); Konstruktor-Vorgabe sichtbar/drucken = 1, gespeichert wird nur
                # die Abweichung 0 (der Parser liest das bool-Byte 00 als leeren Text). Nachgebildet wird der Druck.
                fl = {q[0]: q[2] for q in p if q[0] == 0xefef}
                if 0xefef in fl and fl[0xefef] in ("", 0, False):
                    self.ausgeblendet.add(E.wert(p, 0xeff2))
                EBENEN[E.wert(p, 0xeff2)] = (E.wert(p, 0xee21) or 2.5, E.wert(p, 0xee26) or 7)
                EBENENFARBE[E.wert(p, 0xeff2)] = E.wert(p, 0xeff4)      # Farbe der Ebene (siehe farbe())
                if isinstance(E.wert(p, 0xeff5), (int, float)):
                    EBENENBREITE[E.wert(p, 0xeff2)] = E.wert(p, 0xeff5)  # Linienbreite der Ebene (mm)
        NICHT_GEDRUCKT.clear()
        NICHT_GEDRUCKT.update(self.ausgeblendet)
        # Symbolnummer (eb52) -> Satz-ID, Varianten je Symbol
        self.nummer = {lib: {(E.wert(p, 0xeb52) or 0): sid for sid, (c, p) in self.sym[lib].items() if p}
                       for lib in self.libs}
        self.varianten = collections.defaultdict(dict)
        self.varianten_unlesbar = 0                     # Variant.eod-Sätze ohne lesbaren Verweis (übersprungen)
        for lib in self.libs:
            for vid, (c, p) in self.var[lib].items():
                s_ = E.wert(p, 0xed95) if p else None    # Verweis (Klasse, Symbol-ID); sonst Satz nicht lesbar
                if isinstance(s_, tuple) and len(s_) > 1:
                    self.varianten[(lib, s_[1])][E.wert(p, 0xed96) or 0] = vid
                else:
                    self.varianten_unlesbar += 1
        self.zuordnung = {}

    def bibliotheken_zuordnen(self, funktionen):
        """Bibliotheksindex (ef4d) -> .sdb: die Bibliothek, deren Symbol die Funktionsdefinition am
        häufigsten trifft (je Projekt bestimmt, die Reihenfolge steht nicht im Archiv)."""
        treffer = collections.defaultdict(collections.Counter)
        for p in funktionen:
            idx, nr = E.wert(p, 0xef4d), E.wert(p, 0xef4e)
            if nr is None:
                continue
            fd = (E.wert(p, 0xefc5), E.wert(p, 0xefc6))
            for lib in self.libs:
                sid = self.nummer[lib].get(nr)
                if sid is None:
                    continue
                sp = self.sym[lib][sid][1]
                treffer[idx or 0][lib] += 1 + 5 * ((E.wert(sp, 0xefc5), E.wert(sp, 0xefc6)) == fd)
        for idx, c in treffer.items():
            self.zuordnung[idx] = c.most_common(1)[0][0]
        return self.zuordnung

    def variante(self, idx, nr, var):
        lib = self.zuordnung.get(idx or 0)
        if lib is None or nr is None:
            return None
        sid = self.nummer[lib].get(nr)
        if sid is None:
            return None
        vid = self.varianten[(lib, sid)].get(var or 0)
        if vid is None:
            return None
        return lib, sid, vid

    def symbolname(self, lib, sid):
        p = self.sym[lib][sid][1]
        return E.wert(p, 0xeb51) or str(sid)

    def anschluesse(self, lib, vid):
        """Anschlusspunkte der Variante in Reihenfolge; fehlt die Lage (edbe), liegt der Punkt im
        Einfügepunkt (z. B. Stecker-Stifte XBD/XSD). Ohne jeden Punkt (Geräteanschluss DC, Ader CDP):
        ein Punkt im Einfügepunkt."""
        p = self.var[lib][vid][1] or []
        pts = []
        if E.wert(p, 0xedbe) or E.wert(p, 0xedbf):
            pts.append((E.wert(p, 0xedbe) or (0.0, 0.0), E.wert(p, 0xedbf) or 4))
        for o in E.objekte(p, 0xedbd):
            if E.wert(o, 0xedbe) or E.wert(o, 0xedbf):
                pts.append((E.wert(o, 0xedbe) or (0.0, 0.0), E.wert(o, 0xedbf) or 4))
        if not pts:
            pts = [((0.0, 0.0), 4)]
        return [(q[0], q[1], r) for q, r in pts]

    def textlagen(self, lib, vid, ref=None):
        """Eigenschaftstexte der Variante: {(eigenschaft, index): (x, y, h, winkel, ausrichtung)}; ref = Eigenschaften
        der Platzierung (Funktion) -> nur die Texte ihrer Eigenschaftsanordnung (anordnung())."""
        return textlagen(self.var[lib][vid][1] or [], self.anordnung(lib, vid, ref) if ref else 0)

    def anordnung(self, lib, vid, ref, alternativ=False):
        """Gewählte Eigenschaftsanordnung einer Platzierung (EPLAN DMSymbolReference::getRealSelectedPropertyInstanceSet
        0x180887d30, ETrDMProjectu): ed78 (Member +0x143) & 0x7f, Bit 7 = eigene Texte an der Funktion
        (switchCurrentSelectedPropertySetToLocalInstances); ohne Wert die Vorgabe der Variante ed98 (+0x128,
        getDefaultPropSet; bei Projekteinstellung TrDMProject.SymbolReference.AlternativePropertySet ed99 +0x129),
        sonst die erste Anordnung des Symbols (DMSymbol::getFirstPropertyConfigurationInstanceSet: kleinster
        Index 1..63 der Eigenschaft 16050). 0 = keine Auswahl (alle Texte)."""
        r = (E.wert(ref, 0xed78) or 0) & 0x7F
        if r:
            return r
        vp = self.var[lib][vid][1] or []
        r = E.wert(vp, 0xed99 if alternativ else 0xed98) or 0
        if r:
            return r
        sid = E.wert(vp, 0xed95)
        sp = (self.sym[lib].get(sid[1], (None, []))[1] or []) if isinstance(sid, tuple) else []
        idx = [i for k, i, _ in (E.wert(sp, 0xea6b) or []) if k == 16050 and 0 < i < 64]
        return min(idx) if idx else 0

    def formen(self, lib, vid):
        return [f for f in (form(c, p, self.ausgeblendet) for c, p in self.inst[lib].get(vid, [])) if f]

    # --- Makrogruppen (Seiten-Instance Klasse 26): ebf1 = Elterngruppe bzw. Seite -------------------
    def _eltern(self):
        if not hasattr(self, "_gruppe"):
            self._gruppe = {oid: E.wert(p, 0xebf1) for oid, (cls, p) in self.seite.items() if p and cls == 26}
        return self._gruppe

    def gruppen_kette(self, ref):
        """Alle Makrogruppen (IDs), in denen ein Objekt mit Besitzer ref (ebf1) steckt, innen nach außen."""
        eltern, kette = self._eltern(), []
        while ref and ref[0] == 26 and ref[1] not in kette and len(kette) < 32:
            kette.append(ref[1])
            ref = eltern.get(ref[1])
        return kette

    def seitenformen(self, seite_id, ohne=(), nur=None):
        """Grafik der Seite; ohne = Makrogruppen, deren Grafik ausgelassen wird; nur = nur diese Gruppe."""
        out = []
        for _, (cls, p) in self.seite.items():
            if p and E.wert(p, 0xebf2) == (4, seite_id):
                kette = self.gruppen_kette(E.wert(p, 0xebf1))
                if nur is not None and nur not in kette:
                    continue
                if ohne and any(g in ohne for g in kette):
                    continue
                f = form(cls, p, self.ausgeblendet)
                if f:
                    out.append(f)
        return out

    def masse(self, seite_id):
        """Bemaßungen (Klasse 97): Messpunkte f141/f144, Maßlinie f147/f148, Text f149 (Wert f14a, sonst Abstand)."""
        out = []
        for _, (cls, p) in self.seite.items():
            if not p or cls != 97 or E.wert(p, 0xebf2) != (4, seite_id):
                continue
            a, b, c, d, t = (E.wert(p, k) for k in (0xf141, 0xf144, 0xf147, 0xf148, 0xf149))
            if not all(isinstance(q, tuple) for q in (a, b, c, d, t)):
                continue
            wert_ = E.wert(p, 0xf14a)
            if not wert_:
                wert_ = f"{math.dist(a, b) if abs(a[0] - b[0]) > 1e-6 and abs(a[1] - b[1]) > 1e-6 else abs(a[0] - b[0]) + abs(a[1] - b[1]):.1f}".rstrip("0").rstrip(".")
            wert_ = wert_.replace(".", ",")                       # deutsches Dezimalkomma wie im EPLAN-Ausdruck
            fmt = (E.objekte(p, 0xf112) or [[]])[0]
            out += [("L", a, c, False), ("L", b, d, False), ("L", c, d, False),
                    ("T", t, wert_, 2.5, 0.0, E.wert(fmt, 0xee26) or 8)]
        return out

    def platzhaltertexte(self, seite_id):
        """Von EPLAN ausgewertete Texte einer Seite (Klemmenplan-, Kabel-, Stücklisteninhalte …). Zeilen früherer
        Auswertungsläufe (gleiche Lage wie die gültige Zeile) sind in PlaceHolderText.eod gelöschte D-ISAM-Sätze
        (Statusbyte 0) und kommen schon beim Lesen nicht mehr an (autodetect.loeschmarke, FORMATBESCHREIBUNG
        Abschnitt 19 „Satzstatus“). Die Prüfung auf eine fehlende Gruppe (Besitzer ebf1 = Klasse 3) greift danach
        in Projekt A/Projekt E/Projekt B nicht mehr; sie bleibt als Schutz für Archive ohne lesbare .eox."""
        out = []
        for _, (cls, p) in self.platzhalter.items():
            if p and E.wert(p, 0xebf2) == (4, seite_id) and E.wert(p, 0xec5f):
                b_ = E.wert(p, 0xebf1)
                if b_ and b_[0] == 3 and self.seite.get(b_[1], (None,))[0] != 3:
                    continue
                f = form(cls, p, self.ausgeblendet)
                if f:
                    out.append(f)
        return out

    def formulargrafik(self, name, ab_y=20.0):
        """Grafik des Formulars (Tabellenlinien einer Auswertungsseite) ohne Schriftfeldbereich (QET-Schriftfeld).
        name = Formularname oder (Name, Formulartyp); ohne passenden Typ gilt der erste Satz des Namens."""
        fid = self.formulare.get(name)
        if fid is None and isinstance(name, tuple):
            fid = self.formulare.get(name[0])
        if fid is None:
            return []
        out = []
        for _, (cls, p) in self.seite.items():
            if p and E.wert(p, 0xebf2) == (51, fid) and cls != 53:
                f = form(cls, p, self.ausgeblendet)
                if not f:
                    continue
                ys = [q[1] for q in (f[1] if f[0] == "P" else [f[1]] + ([f[2]] if f[0] == "L" else []))]
                if min(ys) >= ab_y:
                    out.append(f)
        return out

    def formularbilder(self, name, ab_y=20.0):
        """Bilder des Formulars (Klasse 91, Eigner (51, id)), z. B. das Firmenlogo des Deckblatts (Projekt D:
        „$(IMG)\\Logo Kunde.jpg“ im Formular KUNDE_002) -> [(x0, y0, x1, y1, datei)]."""
        fid = self.formulare.get(name)
        if fid is None and isinstance(name, tuple):
            fid = self.formulare.get(name[0])
        if fid is None:
            return []
        out = []
        for _, (cls, p) in self.seite.items():
            if not p or cls != 91 or E.wert(p, 0xebf2) != (51, fid) or E.wert(p, 0xebf6) == 1 \
                    or E.wert(p, 0xebfb) in self.ausgeblendet:
                continue
            a, b = E.wert(p, 0xf0d3), E.wert(p, 0xf0d4)
            datei = self.bilddatei(E.wert(p, 0xf0e7) or "")
            if a and b and datei and min(a[1], b[1]) >= ab_y:
                out.append((a[0], a[1], b[0], b[1], datei))
        return out

    def formularplatzhalter(self, name, ab_y=20.0):
        """Platzhalter des Formulars selbst (Instance.eod Klasse 53, Eigner (51, id)) – Kopfzeilen, Formularname,
        Bearbeiter/Datum, Seiten- und Projekteigenschaften; EPLAN speichert für sie keinen ausgewerteten Text
        (PlaceHolderText.eod hat nur Zeileninhalte). -> (name, kv, [(eig, idx, quelle, lage, h, winkel, ausr)])
        mit kv = Eigenschaften des Formulars (ea6b), Quelle ee17: 4 Seite, 14 Projekt, 51 Formular."""
        fid = self.formulare.get(name)
        if fid is None and isinstance(name, tuple):
            fid = self.formulare.get(name[0])
        if fid is None:
            return None
        p = self.formularsatz.get(fid) or []
        kv = {k: v for k, _, v in (E.wert(p, 0xea6b) or [])}
        out = []
        for _, (cls, q) in self.seite.items():
            if not q or cls != 53 or E.wert(q, 0xebf2) != (51, fid) or E.wert(q, 0xebf6) == 1                     or E.wert(q, 0xebfb) in self.ausgeblendet:
                continue
            lage = E.wert(q, 0xec55) or (0.0, 0.0)
            if lage[1] < ab_y:
                continue
            fmt = (E.objekte(q, 0xec59) or [[]])[0]
            eb = EBENEN.get(E.wert(q, 0xebfb), (2.5, 7))
            out.append((E.wert(q, 0xed63), E.wert(q, 0xed64) or 0, E.wert(q, 0xee17), lage,
                        E.wert(fmt, 0xee21) or eb[0], E.wert(fmt, 0xee22) or 0.0, E.wert(fmt, 0xee26) or eb[1]))
        return E.wert(p, 0xede6), kv, out

    def normblatt(self, name):
        """Normblatt (Form.eod Klasse 21) aus seinen eigenen Grafikobjekten (Instance.eod, Eigner (21, id)):
        formen = Linien/Rechtecke/feste Texte; bilder = [(x0, y0, x1, y1, datei)]; platzhalter = Eigenschaftstexte
        (Klasse 53: ed63 Eigenschaft, ed64 Index, ee17 Quelle 4 Seite / 14 Projekt / 53 Spalte, ec55 Lage,
        ec59 Format); spalten = (x0, [Breiten]) aus dem Zahlen-Array 81 03 (X 21509) und eb67; kv = ea6b
        (12025 Anfangswert Spalte, 12029 Stellen, 12060 Kontaktspiegelabstand …)."""
        if name not in self.normblaetter:
            return None
        fid, p = self.normblaetter[name]
        breiten = next((q[2][2] for q in p if q[0] == 0x0381 and q[2][0] == 21509), [])
        ursprung = E.wert(p, 0xeb67)
        x0 = ursprung[0] if isinstance(ursprung, tuple) else 0.0
        kv = {k: v for k, _, v in (E.wert(p, 0xea6b) or [])}
        kv_i = {(k, i): v for k, i, v in (E.wert(p, 0xea6b) or [])}
        formen, bilder, platzhalter = [], [], []
        for _, (cls, q) in self.seite.items():
            if not q or E.wert(q, 0xebf2) != (21, fid) or E.wert(q, 0xebf6) == 1 \
                    or E.wert(q, 0xebfb) in self.ausgeblendet:
                continue
            if cls == 53:
                fmt = (E.objekte(q, 0xec59) or [[]])[0]
                eb = EBENEN.get(E.wert(q, 0xebfb), (2.5, 7))
                platzhalter.append((E.wert(q, 0xed63), E.wert(q, 0xed64) or 0, E.wert(q, 0xee17),
                                    E.wert(q, 0xec55) or (0.0, 0.0), E.wert(fmt, 0xee21) or eb[0],
                                    E.wert(fmt, 0xee22) or 0.0, E.wert(fmt, 0xee26) or eb[1]))
            elif cls == 91:
                a, b, n = E.wert(q, 0xf0d3), E.wert(q, 0xf0d4), E.wert(q, 0xf0e7) or ""
                datei = self.bilddatei(n)
                if a and b and datei:
                    bilder.append((a[0], a[1], b[0], b[1], datei))
            else:
                f = form(cls, q, self.ausgeblendet)
                if f:
                    formen.append(f)
        return {"formen": formen, "bilder": bilder, "platzhalter": platzhalter, "spalten": (x0, breiten), "kv": kv,
                "kv_i": kv_i}

    def bilddatei(self, n):
        """Bildpfad f0e7 („$(IMG)\\a.png“, „$(MD_IMG)\\News 2027\\b.png“) -> Datei im Images-Ordner des Archivs.
        Belegt an den EES-Archiven: $(IMG)-Dateien ohne Unterordner liegen direkt in Images; alles andere legt das
        Archiv in Ordner „<hex>“ bzw. „M<hex>“ (gleicher Hex-Name = gleicher Quellordner, z. B. $(MD_IMG)-Wurzel
        immer bc146770). Die Hash-Funktion ist nicht bekannt, daher Zuordnung (Variable, Unterordner) -> Hex aus den
        Dateien selbst: der einzige Hex-Name, unter dem alle Bilder dieses Ordners liegen. Sonst None (gezählt)."""
        img = os.path.join(self.x, "Images")
        if not hasattr(self, "_bildordner"):
            self.bilder_fehlend = collections.Counter()
            gruppen = collections.defaultdict(set)
            for _, (cls, p) in self.seite.items():
                m = re.match(r"^\$\(?([A-Z_]*IMG)\)?[\\/](.*)$", (E.wert(p, 0xf0e7) or "") if p and cls == 91 else "",
                             re.I)                      # Windows-Pfad: Groß/klein egal („$(md_img)\pxc“)
                if m:
                    *sub, f = re.split(r"[\\/]", m.group(2))
                    gruppen[(m.group(1).upper(), "\\".join(sub).lower())].add(f)
            hexe = {d[1:] if d.startswith("M") else d for d in (os.listdir(img) if os.path.isdir(img) else [])
                    if os.path.isdir(os.path.join(img, d))}
            self._bildordner = {}
            for g, fs in gruppen.items():
                k = [h for h in hexe if all(os.path.exists(os.path.join(img, "M" + h, f))
                                            or os.path.exists(os.path.join(img, h, f)) for f in fs)]
                def inhalt(h, f):
                    for v in ("M" + h, h):
                        if os.path.exists(os.path.join(img, v, f)):
                            return open(os.path.join(img, v, f), "rb").read()
                # mehrere Ordner nur dann eindeutig, wenn die Bilder darin byte-gleich sind (Archiv-Doppel)
                if len(k) == 1 or (k and all(len({inhalt(h, f) for h in k}) == 1 for f in fs)):
                    self._bildordner[g] = sorted(k)[0]
        # „$IMG\Datei“ ohne Klammern (ältere Schreibweise, Projekt D: Schrankaufbau /65, Formular) = „$(IMG)\Datei“
        m = re.match(r"^\$\(?([A-Z_]*IMG)\)?[\\/](.*)$", n or "", re.I)
        if not m:
            self.bilder_fehlend[n or "(leer)"] += 1
            return None
        *sub, f = re.split(r"[\\/]", m.group(2))
        if m.group(1).upper() == "IMG" and not sub and os.path.exists(os.path.join(img, f)):
            return os.path.join(img, f)
        h = self._bildordner.get((m.group(1).upper(), "\\".join(sub).lower()))
        for v in (("M" + h, h) if h else ()):
            if os.path.exists(os.path.join(img, v, f)):
                return os.path.join(img, v, f)
        self.bilder_fehlend[n] += 1
        return None

    def seitenbilder(self, seite_id):
        """Bilder (Klasse 91) einer Seite: [(x0, y0, x1, y1, datei)] mit Datei aus <archiv>/Images (bilddatei)."""
        out = []
        for _, (cls, p) in self.seite.items():
            if p and cls == 91 and E.wert(p, 0xebf2) == (4, seite_id):
                a, b = E.wert(p, 0xf0d3), E.wert(p, 0xf0d4)
                datei = self.bilddatei(E.wert(p, 0xf0e7))
                if a and b and datei:
                    out.append((a[0], a[1], b[0], b[1], datei))
        return out

    def seitensymbole(self, seite_id, ohne=(), nur=None):
        """Symbolplatzierungen ohne Funktion (Klasse 42: Winkel, Abzweige, Sondersymbole) einer Seite."""
        out = []
        for _, (cls, p) in self.seite.items():
            if p and cls == 42 and E.wert(p, 0xebf2) == (4, seite_id):
                kette = self.gruppen_kette(E.wert(p, 0xebf1))
                if (nur is not None and nur not in kette) or (ohne and any(g in ohne for g in kette)):
                    continue
                lage = None
                for o in E.objekte(p, 0xef11):
                    lage = E.wert(o, 0xed5a) or lage
                if lage:
                    out.append((E.wert(p, 0xef4d), E.wert(p, 0xef4e), E.wert(p, 0xef4f), lage))
        return out


# Tahoma (Firmenschrift): Ober-/Unterlänge 2049/423 von 2472 Einheiten der Zelle (Schriftdatei tahoma.ttf, hhea)
TAHOMA_OBEN = 2049 / 2472


def zeilenanteile(ausr, zeile):
    """(über, unter) dem Anker für einen Textblock der Höhe zeile (EPLAN-Ausrichtung 1–12): oben 0/zeile,
    Mitte je die Hälfte, unten zeile/0, Grundlinie Ober-/Unterlänge der Schrift."""
    a = (ausr or 7)
    if a in (1, 2, 3):
        return 0.0, zeile
    if a in (4, 5, 6):
        return zeile / 2, zeile / 2
    if a in (7, 8, 9):
        return zeile, 0.0
    return zeile * TAHOMA_OBEN, zeile * (1 - TAHOMA_OBEN)


def umbrechen(text, breite, h):
    """Zeilenumbruch in fester Breite wie EPLAN (EGedBaseLogu GedFTextBreaker, Umbruchart ee25 & 0x70 = 0x10 ->
    breakFixedWidth 0x1801d0de0 / breakLine_FixWidth 0x1801d2970): je Absatz (Zeilenwechsel); kein Umbruch, wenn
    Texthöhe × 5 × Zeichenzahl < Breite oder die Zeile passt; sonst bricht die Zeile am letzten Worttrenner
    (Leerzeichen, „;“, Leerraum; nicht das geschützte Leerzeichen, isWordSeparator 0x1801d26d0), bis zu dem der
    Text in die Breite passt, mindestens am ersten; der Trenner entfällt. Wörter werden nicht geteilt.
    Laufweite = Tahoma-hmtx × Geviert 1,243 × h (ETextGraphicsTools::getTextExtent mit der Schrift aus setFont)."""
    px = h * TAHOMA_EM

    def trenner(c):
        return c != "\u00a0" and (c in " ;" or c.isspace())
    aus = []
    for absatz in text.split("\n"):
        zeilen, rest = [], absatz
        while rest and h * 5 * len(rest) >= breite and breite_px(rest, px) > breite:
            pos = [i for i, c in enumerate(rest) if trenner(c)]
            if not pos:
                break
            passt = [i for i in pos if breite_px(rest[:i], px) <= breite]
            i = passt[-1] if passt else pos[0]
            zeilen.append(rest[:i])
            rest = rest[i + 1:]
        zeilen.append(rest)
        aus.append("\n".join(zeilen))
    return "\n".join(aus)


class Lagen(dict):
    """Textlagen; .relation = {(Eigenschaft, Index): ed65} – woher der Wert kommt (EPLAN PropertyPlacement.
    RelationType: 0 Funktion, 20064 Hauptfunktion, 20190 Artikelreferenz, 20191 Artikel, 20192/20193
    Artikelreferenz/Artikel der Hauptfunktion, 20194/20195 Anschluss-Artikel)."""
    relation = {}
    boxbreite = {}                     # (Eigenschaft, Index) -> Breite der Ausrichtungsbox (ee27, mm): Umbruch
    boxart = {}                        # (Eigenschaft, Index) -> ee25 (DmgTextInfo +0x44, Umbruchart & 0x70)
    stil = {}                          # (Eigenschaft, Index) -> {"fett", "kursiv"} (textstil)


class Bezug(tuple):
    """(Eigenschaft, Index) des Texts, an den ein Text angedockt ist; .richtung = EPLAN-DockingStyle-Bits des
    Kette = DockingStyle des Starttexts (DmgBaseText::getDockingDirection): 16 = darüber, 8 = darunter,
    32 rechts, 64 links."""
    richtung = 0


def _index(o):
    """Index eines Eigenschaftstexts. Artikeleigenschaften (22xxx): ed66 = welcher Artikel (1-basiert), ed64 = Index
    der Eigenschaft (z. B. 22051 „Attribute“ 1..n) -> kodiert als Artikel * 1000 + Eigenschaftsindex."""
    eig = E.wert(o, 0xed63) or 0
    if 22000 <= eig < 23000:
        return (E.wert(o, 0xed66) or 1) * 1000 + (E.wert(o, 0xed64) or 0)
    return E.wert(o, 0xed64) or E.wert(o, 0xed66) or 0


def textlagen(props, satz=0):
    """Eigenschaftstexte (ea65, Klasse 53) -> {(eigenschaft, index): (x, y, h, winkel, ausrichtung, am_anschluss,
    gestapelt_auf)}.
    - satz = gewählte Eigenschaftsanordnung der Platzierung (Grafik.anordnung): ein Text mit ee18 (SymRefConfiguration,
      DMContextPropertyInstance +0x1ca) ≠ 0 gilt nur in dieser Anordnung (DMSymbolReference::
      isPropertyInstanceConfigurationVisible 0x180888d90: sichtbar, wenn ee18 = 0, Auswahl = 0 oder ee18 = Auswahl);
      Texte ohne ee18 (v. a. Anschlusstexte) gelten in jeder Anordnung.
    - ec55 Lage, ec56 Bezug auf einen anderen Eintrag (65535 = keiner); Bezug OHNE eigene Lage = EPLAN stapelt den
      Text eine Zeile über den Bezugstext (leere Texte belegen keine Zeile) -> gestapelt_auf = (eigenschaft, index);
      ec57 Bit 2 zeigt nur den Bezug an (kein Ausblenden);
    - Anschlusstexte (20028/20029, Index = Anschluss 1-basiert) liegen relativ zu ihrem Anschlusspunkt;
    - ee19 1/2 = Lage für waagerechten/senkrechten Pfad: genommen wird die für senkrechte Pfade (2)."""
    roh, kasten = {}, None
    for o in E.objekte(props, 0xea65):
        eig = E.wert(o, 0xed63)
        konf = E.wert(o, 0xee18)
        if eig and satz and isinstance(konf, int) and konf and konf != satz:
            continue                     # Text einer anderen Eigenschaftsanordnung
        if eig:
            roh[E.wert(o, 0xea64)] = o
        elif E.wert(o, 0xf0d3) and E.wert(o, 0xf0d4):          # Kasten (Klasse 89) des Geräts/der Karte
            kasten = (E.wert(o, 0xf0d3), E.wert(o, 0xf0d4))
    if kasten is None:
        a, b = E.wert(props, 0xeb1e), E.wert(props, 0xeb1f)
        if a and b and max(abs(v) for v in (*a, *b)) < 1e6:
            kasten = (a, b)

    def anker(o):
        """ee1a = Bezugspunkt (EPLAN-API ContextPropertyPlacement.Enums.BasePoint): 0 Einfügepunkt, 1 links oben,
        2 rechts oben, 3 links unten, 4 rechts unten, 5 oben Mitte, 6 unten Mitte, 7 Mitte links, 8 Mitte rechts,
        9 Mitte; 13-15 Kabel, 16 Anschluss, 17 feste Y-Lage (ohne Kasten: Einfügepunkt)."""
        k = E.wert(o, 0xee1a)
        if not kasten or k not in range(1, 10):
            return 0.0, 0.0
        (xa, ya), (xb, yb) = kasten
        x0, x1, y0, y1 = min(xa, xb), max(xa, xb), min(ya, yb), max(ya, yb)
        xm, ym = (x0 + x1) / 2, (y0 + y1) / 2
        return {1: (x0, y1), 2: (x1, y1), 3: (x0, y0), 4: (x1, y0), 5: (xm, y1), 6: (xm, y0), 7: (x0, ym),
                8: (x1, ym), 9: (xm, ym)}[k]

    def angedockt(o):
        """ec57 Bit 2 = Text ist an seinen Bezugstext angedockt (EPLAN „IsDocked“): die eigene Lage ec55 gilt nicht,
        der Text folgt dem Bezugstext eine Zeile weiter – auch Anschlussbeschreibungen 20029/20039 (die frühere Ausnahme
        war am PDF geschärft, ohne Code-Beleg; getTextBlocks nimmt jeden Text mit DockingStyle-Bit 2 in den Block)."""
        return bool((E.wert(o, 0xec57) or 0) & 2)

    def kettenrichtung(i):
        """Andockrichtung einer Textkette = DockingStyle des STARTtexts (ohne Bezug): 16 DockToTop -> angedockte
        Texte darüber, 8 DockToBottom -> darunter, 32/64 rechts/links; ohne Richtungsbit gilt 8 (EPLAN
        DmgBaseText::getDockingDirection für den Master). Am Original-PDF geprüft (SPS-Kanal 20211 ec57=20 ->
        darüber, Kabel 20005 ec57=4 -> darunter)."""
        for _ in range(12):
            bez_ = E.wert(roh[i], 0xec56)
            if not (bez_ and bez_[0] != 65535 and bez_[2] in roh):
                break
            i = bez_[2]
        return ((E.wert(roh[i], 0xec57) or 0) & 0x78) or 8

    def lage(i, tiefe=0):
        o = roh[i]
        x, y = E.wert(o, 0xec55) or (0.0, 0.0)
        bez = E.wert(o, 0xec56)
        if bez and bez[0] != 65535 and bez[2] in roh and tiefe < 8:
            bx, by, am = lage(bez[2], tiefe + 1)
            if angedockt(o):
                return bx, by, am                    # angedockt: gespeicherte Lage veraltet, EPLAN stapelt
            return x + bx, y + by, am
        ax, ay = anker(o)
        return x + ax, y + ay, E.wert(o, 0xed63) in (20028, 20029)
    lagen, relation, boxbreite, boxart, stil = {}, {}, {}, {}, {}
    for i, o in roh.items():
        if E.wert(o, 0xebf6) == 1:
            continue                  # ebf6 = Placement.Visibility (EPLAN-API): 1 unsichtbar (0 sichtbar, 2 nach Ebene)
        if E.wert(o, 0xebf6) in (None, 2) and E.wert(o, 0xebfb) in NICHT_GEDRUCKT and not any(
                (E.wert(q, 0xec56) or (65535,))[0] != 65535 and (E.wert(q, 0xec56) or (0, 0, None))[2] == i
                for q in roh.values()):
            continue                  # Ebene druckt nicht (Layer.eod efef = 0), z. B. Artikel-Dokumente 22149 ff.
        eig, pfad = E.wert(o, 0xed63), E.wert(o, 0xee19)
        idx = _index(o)
        alt = lagen.get((eig, idx))
        if alt and pfad == 1 and alt[-1] != 1:
            continue
        fmt = (E.objekte(o, 0xec59) or [[]])[0]
        x, y, am = lage(i)
        eb = EBENEN.get(E.wert(o, 0xebfb), (2.5, None))              # Vorgabe der Ebene, wenn der Text keine hat
        bez, auf = E.wert(o, 0xec56), None
        if bez and bez[0] != 65535 and bez[2] in roh and (E.wert(o, 0xec55) is None or angedockt(o)):
            po = roh[bez[2]]
            auf = Bezug((E.wert(po, 0xed63), _index(po)))
            auf.richtung = kettenrichtung(i)
        if E.wert(o, 0xed65):
            relation[(eig, idx)] = E.wert(o, 0xed65)
        if any(textstil(fmt).values()):
            stil[(eig, idx)] = textstil(fmt)
        if E.wert(fmt, 0xee27):
            boxbreite[(eig, idx)] = E.wert(fmt, 0xee27)
            boxart[(eig, idx)] = E.wert(fmt, 0xee25) or 0
        lagen[(eig, idx)] = (x, y, E.wert(fmt, 0xee21) or eb[0], E.wert(fmt, 0xee22) or 0.0,
                             E.wert(fmt, 0xee26) or eb[1], am, auf, pfad)
    aus = Lagen({k: v[:7] for k, v in lagen.items()})
    aus.relation = relation
    aus.boxbreite = boxbreite
    aus.boxart = boxart
    aus.stil = stil
    return aus
