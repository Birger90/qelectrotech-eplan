"""WSCAD-Projekt (Ordner mit Database/WSCAD6.db und .wsELD-Seiten) -> bearbeitbares QElectroTech-Projekt.

    python wscad_nach_qet.py "<WSCAD-Projektordner>" [ziel.qet] [--pdf] [--ohne-sammlung]

Auch über Datei > WSCAD-Projekt importieren (QET-Dialog, FORTSCHRITT-Zeilen speisen den Balken).

Übersetzung (Formatbeschreibung: FORMATBESCHREIBUNG_WSCAD.md, jede Regel am Demoprojekt von WSCAD ELECTRIX 2025
gegen dessen PDF-Export geprüft):
- je Schaltplanseite (.wsELD) ein Blatt; Rahmen und Schriftfeld (<Template>) und freie Seitengrafik als Blattgrafik,
  Schriftfeldwerte (Seitentitel, Blatt, Anlage =, Ort +) zusätzlich in die Blatteigenschaften;
- jedes Symbol mit Betriebsmittel (<SL hid> = Symbol.SymbolID der Datenbank) und Anschlüssen wird ein QET-Bauteil
  mit seiner WSCAD-Grafik, seinen Texten (BMK als Kennzeichen, Anschlussnummern, Funktionstext, Querverweis)
  und Anschlüssen in WSCAD-Richtung (connDir 1 oben, 2 rechts, 4 unten, 8 links); Artikel und Hersteller als
  Elementinformationen;
- Leitungen: die Verbindungsliste der Datenbank (Connection: Symbol/Anschluss -> Symbol/Anschluss, je Seite) wird
  als QET-Leiter gezeichnet; der Weg folgt den gezeichneten Leitungen der Seite (WSCAD verbindet Anschlüsse, die
  sich in Anschlussrichtung gegenüberstehen; Winkel/T-Symbole (SL-Art 10) und Leitungszüge (<CN>) leiten weiter).
  Farbe, Querschnitt und Potential der Verbindung an den Leiter;
- Pfeile mit Ziel (SL-Art 14, Abbruchsymbol) werden verknüpfte QET-Verweise (gehend/kommend nach dem Pfeiltext,
  Partner = nächster gleichnamiger Pfeil); Spule (Art 2) bzw. Master (Art 7) und Kontakte (Art 3) desselben BMK
  werden Master/Slave; WSCADs Querverweistexte bleiben, QETs eigene sind aus (wie beim EPLAN-Import).
- Artikel, die die Firmensammlung noch nicht kennt (Index nach Bestellnummer, eplan_bibliothek.Sammlung), kommen als
  sauberes Bauteil nach Hersteller/Kategorie/Bestellnummer (--ohne-sammlung: nicht).
Daneben <ziel>_Geraete.csv und <ziel>_Verbindungen.csv (Excel-tauglich).
"""
import base64
import collections
import csv
import hashlib
import math
import os
import re
import sys

HIER = os.path.dirname(os.path.abspath(__file__))
EPLAN = os.path.join(HIER, "..", "eplan_import")
sys.path.insert(0, HIER)
sys.path.insert(0, EPLAN)
sys.path.insert(0, os.path.join(EPLAN, "eplan_zw1"))

import wscad as W                                              # noqa: E402
from planvorlagen import Plan                                  # noqa: E402
from planvorlagen import symbole as S                         # noqa: E402
import eplan_grafik as G                                       # noqa: E402
import eplan_bibliothek as BIB                                 # noqa: E402

ORDNER = "WSCAD-Import"                       # erzeugte Schaltbilder in der Firmensammlung
TOL = 0.02                                    # mm: Punkte gelten als gleich
# WSCAD-Schriftgröße (size, Vorgabe 2,5 mm) -> Geviert im PDF 0,933 × size (gemessen: 3,75 -> 3,5; 2,5 -> 2,33);
# eplan_grafik rechnet mit EPLAN-Texthöhe h (Geviert = TAHOMA_EM × h)
HOEHE_JE_SIZE = 0.933 / G.TAHOMA_EM
SIZE_VORGABE = 2.5
ZEILE_JE_SIZE = 1.0                           # Zeilenabstand mehrzeiliger Texte (PDF: 2,49 mm bei 2,5 mm)
ZEILE_DATENBLOCK = 1.2                        # Zeilen eines Datenblocks (PDF: 3,0 mm bei 2,5 mm)
# WSCAD zeichnet Rahmen und Schriftfeld selbst (A4/A3 in echten mm): QET-Szene maßstäblich (96 dpi) ab dem
# Blattursprung, nicht wie beim EPLAN-Import 420 mm auf zehn Spalten à 156 px
K_WSCAD, X0_WSCAD = 96 / 25.4, 0
# QET zeichnet Rahmen und Schriftfeld ab Szene (5, 5) (Diagram::margin); der PDF-Export beginnt dort. Der WSCAD-
# Blattursprung liegt daher bei 5 px (gemessen: ohne Versatz alles 1,32 mm nach links oben)
RAND_PX = 5.0
GEGEN = {1: 4, 4: 1, 2: 8, 8: 2}
SCHRITT = {1: (0, -1), 2: (1, 0), 4: (0, 1), 8: (-1, 0)}       # WSCAD: y nach unten


SCHRIFT_WSCAD = "Roboto Mono"                # WSCAD-PDF: eingebettete Schrift RobotoMono-Regular


def schrift_wie_wscad():
    """Texte in WSCADs Schrift Roboto Mono (PDF-Export des Demoprojekts) setzen, mit deren Laufweiten für die
    Ausrichtung; ist sie nicht installiert (Windows-Schriften, Benutzerschriften), bleibt Tahoma."""
    kandidaten = [os.path.join(os.environ.get("WINDIR", r"C:\Windows"), "Fonts", "RobotoMono-Regular.ttf"),
                  os.path.join(os.environ.get("LOCALAPPDATA", ""), "Microsoft", "Windows", "Fonts",
                               "RobotoMono-Regular.ttf"),
                  os.path.expanduser("~/.local/share/fonts/RobotoMono-Regular.ttf"),
                  "/Library/Fonts/RobotoMono-Regular.ttf"]
    datei = next((k for k in kandidaten if os.path.isfile(k)), None)
    if not datei:
        print("Roboto Mono nicht installiert – Texte in Tahoma (Breiten weichen vom WSCAD-PDF ab)")
        return False
    from fontTools.ttLib import TTFont
    f = TTFont(datei)
    upm, hm = f["head"].unitsPerEm, f["hmtx"]
    G._LAUFWEITE.clear()
    G._LAUFWEITE.update({cp: hm[g][0] / upm for cp, g in f.getBestCmap().items()})
    G._LAUFWEITE[None] = hm[".notdef"][0] / upm
    G.SCHRIFT = SCHRIFT_WSCAD
    return True


def _fortschritt(prozent, schritt):
    print(f"FORTSCHRITT {int(prozent)} {schritt}", flush=True)


def _zahl(v, vorgabe=0.0):
    try:
        return float(v)
    except (TypeError, ValueError):
        return vorgabe


def _punkte(s):
    z = [float(v) for v in (s or "").replace(",", " ").split()]
    return list(zip(z[0::2], z[1::2]))


def _stil(s):
    return dict(kv.split(":", 1) for kv in (s or "").split(";") if ":" in kv)


def _art(sl):
    """SL type „14.3“ -> (14, 3)."""
    a, _, b = (sl.get("type") or "0").partition(".")
    return int(_zahl(a)), int(_zahl(b))


def _schluessel(p):
    """Punkt auf 1/100 mm (WSCAD speichert zwei Nachkommastellen; Pfeile liegen 0,01 mm versetzt – ein Raster
    von TOL kippte bei x/0,02 = n,5 je nach Gleitkommarest)."""
    return (round(p[0] * 100), round(p[1] * 100))


def _ausrichtung(stil):
    """WSCAD-align (am PDF geeicht: Bit 2 rechts, Bit 4 Mitte; Bit 8 unten, nur Bit 16 Mitte, sonst oben; ohne
    align: Grundlinie links) -> (Spalte 0/1/2, Zeile 'oben'|'mitte'|'unten'|'grund')."""
    if "align" not in stil:
        return 0, "grund"
    a = int(_zahl(stil.get("align")))
    spalte = 1 if a & 4 else (2 if a & 2 else 0)
    zeile = "unten" if a & 8 else ("mitte" if a & 16 else "oben")
    return spalte, zeile


def _ausr(spalte, zeile):
    """-> EPLAN-Ausrichtung 1–12 für eplan_grafik (1–3 oben, 4–6 Mitte, 7–9 unten, 10–12 Grundlinie). WSCAD
    „unten“ liegt am PDF auf der Grundlinie (Anker 0,86–0,92 der Zeilenhöhe)."""
    return {"oben": 1, "mitte": 4, "unten": 10, "grund": 10}[zeile] + spalte


def _zeilen(t):
    """WSCAD-Zeilenwechsel: \\a bzw. \\n als Zeichenfolge, CR/LF; leere Zwischenzeilen (CR LF doppelt) entfallen."""
    t = (t or "").replace("\\a", "\n").replace("\\n", "\n").replace("\r\n", "\n").replace("\r", "\n")
    return "\n".join(z for z in t.split("\n") if z.strip())


EIGENE_STRUKTUR = [""]                        # „=A1+ST“ der Seite, die gerade umgesetzt wird


def _ohne_eigene_struktur(qv):
    """Querverweis „=A1+ST/CE:1.3“ auf derselben Struktur zeigt WSCAD als „/CE:1.3“ (PDF)."""
    e = EIGENE_STRUKTUR[0]
    return qv[len(e):] if e and qv.startswith(e + "/") else qv


def sps_adresse(wert, sprache="DE"):
    """WSPlcChannelAddress „0#|#True#|#True#|#EN|T|I|L|DE|T|E#|##|#0#|#7#|#1#|#.#|#“ -> „E0.7“: Felder #|#:
    1 Bitadressierung, 3 Präfix je Sprache (Sprache|T|Präfix, mit |L| getrennt), 5 Byte, 6 Bit, 8 Trenner
    (PDF: „E0.7“, analog „AEW0“ ohne Bit)."""
    f = (wert or "").split("#|#")
    if len(f) < 9:
        return wert or ""
    praefix = {}
    for eintrag in f[3].split("|L|"):
        teile = eintrag.split("|")
        if len(teile) >= 3:
            praefix[teile[0]] = teile[2]
    vor = praefix.get(sprache) or praefix.get("DE") or next(iter(praefix.values()), "")
    bit = f[6] if f[1] == "True" and f[6].lstrip("-").isdigit() and int(f[6]) >= 0 else ""
    return f"{vor}{f[5]}" + (f"{f[8]}{bit}" if bit != "" else "")


def _wert(p):
    """Angezeigter Wert einer Eigenschaft <P>: BMKZ prd, FTexte Text der Sprache, SPS-Adresse, sonst val."""
    if p is None:
        return ""
    if p.get("prd") is not None:
        return p.get("prd") or ""
    if p.get("type") == "WSPlcChannelAddress":
        return sps_adresse(p.get("val"))
    w = _zeilen(W.text_de(p) if p.find("T") is not None else (p.get("val") or ""))
    return _ohne_eigene_struktur(w) if p.get("id") == "QV" else w


def _text_wert(eltern, xpath):
    """PHL xpath="../P[@id='X']" -> angezeigter Wert."""
    m = re.match(r"\.\./P\[@id='([^']+)'\]", xpath or "")
    if not m or eltern is None:
        return ""
    # mehrere gleichnamige Eigenschaften (Querverweis mit lineid 0, 1 …, anzL="2") untereinander
    return "\n".join(w for w in (_wert(p) for p in eltern.findall(f"P[@id='{m.group(1)}']")) if w)


# --- Grafik ------------------------------------------------------------------------------------------------------

class Abbild:
    """WSCAD-Koordinaten (mm, y nach unten, relativ zu ox/oy) -> eplan_grafik-Koordinaten (mm, y nach oben):
    für ein Bauteil relativ zum Einfügepunkt (spiegeln an y), für das Blatt absolut (H - y)."""

    def __init__(self, ox, oy, hoehe=None):
        self.ox, self.oy, self.hoehe = ox, oy, hoehe

    def __call__(self, x, y):
        x, y = x + self.ox, y + self.oy
        return (x, self.hoehe - y) if self.hoehe is not None else (x, -y)


def _strich(e):
    lt = e.get("lt")
    gestrichelt = lt not in (None, "", "0", "1")
    breite = _zahl(e.get("dk"), 0.0) or None
    return gestrichelt, ({"breite": breite} if breite else {})


def _bogen(start, spanne):
    """WSCAD-Bogen (GDI: Startwinkel und Spanne in Grad, im Uhrzeigersinn bei y nach unten) -> Form „A“
    (Bogenmaß, gegen den Uhrzeigersinn bei y nach oben): von -(start+spanne) bis -start."""
    return math.radians(-(start + spanne)), math.radians(-start)


def formen(knoten, ab, texte=True):
    """Grundformen eines WSCAD-Knotens (Kinder line, polyline, polygon, rect, kreis, text) -> eplan_grafik-Formen."""
    aus = []
    for e in knoten:
        tag = e.tag
        if tag == "line":
            g, st = _strich(e)
            aus.append(("L", ab(_zahl(e.get("x1")), _zahl(e.get("y1"))), ab(_zahl(e.get("x2")), _zahl(e.get("y2"))),
                        g, None, st))
        elif tag in ("polyline", "polygon"):
            pts = [ab(x, y) for x, y in _punkte(e.get("points"))]
            if len(pts) < 2:
                continue
            g, st = _strich(e)
            zu = tag == "polygon"
            if zu:
                pts = pts + [pts[0]]
            aus.append(("P", pts, zu, zu and e.get("fill") == "1", g, None, st))
        elif tag == "rect":
            x, y, w, h = (_zahl(e.get(k)) for k in ("x", "y", "wd", "hg"))
            pts = [ab(x, y), ab(x + w, y), ab(x + w, y + h), ab(x, y + h), ab(x, y)]
            g, st = _strich(e)
            aus.append(("P", pts, True, e.get("fill") == "1", g, None, st))
        elif tag == "kreis":
            c = ab(_zahl(e.get("cx")), _zahl(e.get("cy")))
            r = _zahl(e.get("r"))
            g, st = _strich(e)
            if e.get("as") is not None:
                a0, a1 = _bogen(_zahl(e.get("as")), _zahl(e.get("ag")))
            elif e.get("type") == "KreisH":
                # Halbkreis: qd = Viertel-Bits (1: 0–90°, 2: 90–180°, 4: 180–270°, 8: 270–360°, GDI-Winkel)
                start = {3: 0, 6: 90, 12: 180, 9: 270}.get(int(_zahl(e.get("qd"), 3)), 0)
                a0, a1 = _bogen(start, 180)
            else:
                a0 = a1 = 0.0
            aus.append(("A", c, r, a0, a1, g, None, st))
        elif tag == "text" and texte:
            aus += text_formen(W.text_de(e.find("P")), _zahl(e.get("x")), _zahl(e.get("y")), _stil(e.get("style")), ab)
    return aus


def text_formen(t, x, y, stil, ab, box=None):
    """Text an WSCAD-Anker (lokal) -> Formen „T“ (eine je Zeile, Zeilenabstand 1,2 × size)."""
    t = _zeilen(t)
    if not t.strip():
        return []
    size = _zahl(stil.get("size"), SIZE_VORGABE)
    spalte, zeile = _ausrichtung(stil)
    if box and "align" in stil:                   # Platzhalter mit Rahmen (Schriftfeld): Anker im Rahmen
        w, h = box
        x += (0, w / 2, w)[spalte]
        y += {"oben": 0, "mitte": h / 2, "unten": h, "grund": h}[zeile]
    winkel = _zahl(stil.get("winkel"))
    zeilen = t.split("\n")
    n = len(zeilen)
    dz = ZEILE_JE_SIZE * size
    # Block um den Anker: oben -> erste Zeile am Anker, Mitte -> Block mittig, unten -> letzte Zeile am Anker
    # mehrzeilig: erste Zeile am Anker, auch bei „unten“ (PDF: „Einspeisung/HV Schaltraum/Feld 5“, align 30,
    # erste Grundlinie am Anker); nur „Mitte“ mittet den Block
    vor = {"oben": 0, "grund": 0, "mitte": (n - 1) / 2, "unten": 0}[zeile]
    c, s_ = math.cos(math.radians(winkel)), math.sin(math.radians(winkel))
    aus = []
    for k, z in enumerate(zeilen):
        d = (k - vor) * dz                        # Versatz quer zur Schrift (nach „unten“ im Textsystem)
        # gedrehte Schrift (WSCAD-Winkel gegen den Uhrzeigersinn, y nach unten): „unten“ im Textsystem
        # ist (sin w, cos w)
        lx, ly = x + d * s_, y + d * c
        aus.append(("T", ab(lx, ly), z, size * HOEHE_JE_SIZE, math.radians(winkel), _ausr(spalte, zeile), None, {}))
    return aus


def kasten(text, ph, st):
    """Platzhalter mit Rahmen (wd/hg): tc:5 schneidet am Rahmenende ab (PDF: „Überwachung MSS Transpor…“ in
    35 mm), tc:6 bricht an Wortgrenzen in Zeilen der Rahmenbreite um (SPS-Kanal: Kommentar zweizeilig mittig).
    -> (Text, Rahmen oder None)."""
    if not ph.get("wd"):
        return text, None
    breite, size = _zahl(ph.get("wd")), _zahl(st.get("size"), SIZE_VORGABE)
    box = (breite, _zahl(ph.get("hg")))
    if st.get("tc") == "6":
        return _umbrochen(text, breite, size), box
    if st.get("tc") == "5" or st.get("pl") == "1":
        return _gekuerzt(text, breite, size), box
    return text, box


def _umbrochen(text, breite_mm, size):
    px = G.schrift_px(size * HOEHE_JE_SIZE)
    aus = []
    for absatz in _zeilen(text).split("\n"):
        zeile = ""
        for wort in absatz.split():
            probe = (zeile + " " + wort).strip()
            if zeile and G.breite_px(probe, px) / G.K > breite_mm:
                aus.append(zeile)
                zeile = wort
            else:
                zeile = probe
        if zeile:
            aus.append(zeile)
    return "\n".join(aus)


def _gekuerzt(text, breite_mm, size):
    """Einzeiliger Platzhalter mit Rahmen (pl:1): WSCAD schneidet am Rahmenende ab (PDF: „Überwachung MSS
    Transpor…“ in 35 mm)."""
    px = G.schrift_px(size * HOEHE_JE_SIZE)
    zeile = _zeilen(text).split("\n")[0] if text else ""
    while zeile and G.breite_px(zeile, px) / G.K > breite_mm:
        zeile = zeile[:-1]
    return zeile


def platzhalter(eltern, ab_fn, mit_bmk=True):
    """Sichtbare Platzhalter (PHL) eines Knotens -> Formen; BMKZ als Kennzeichen (Form „D“ label)."""
    aus = []
    for ph in eltern.findall("PHL"):
        if ph.get("hide") is not None:
            continue
        st = _stil(ph.get("style"))
        wert = _text_wert(eltern, ph.get("xpath"))
        if not wert:
            continue
        wert, box = kasten(wert, ph, st)
        f = text_formen(wert, _zahl(ph.get("x")), _zahl(ph.get("y")), st, ab_fn, box=box)
        if mit_bmk and st.get("wstype") == "BMKZ" and len(f) == 1:
            t = f[0]
            # QET hält bei rechtsbündigen/mittigen Info-Texten die rechte Kante/Mitte fest, wenn der Wert gesetzt
            # wird – gerechnet ab dem leeren Text des frisch gesetzten Bauteils (gemessen: „-X00“ wanderte um seine
            # Breite nach links). Im Schaltbild daher um die (halbe) Breite in Schreibrichtung versetzt ablegen.
            spalte = ((t[5] or 7) - 1) % 3
            w = G.breite_px(t[2], G.schrift_px(t[3])) / G.K * (0, 0.5, 1)[spalte]
            (ax, ay), ang = t[1], t[4]
            f = [("D", (ax + w * math.cos(ang), ay + w * math.sin(ang)), t[2], t[3], t[4], t[5], "label")]
        aus += f
    return aus


def datenblock(sl, db, ab_fn):
    """DataBlock params="QV,False;Funktionstext,True;…": Zeilen der genannten Eigenschaften (True = ausgeblendet),
    leere entfallen, untereinander ab dem Anker."""
    zeilen = []
    for teil in (db.get("params") or "").split(";"):
        name, _, versteckt = teil.partition(",")
        if versteckt.strip() == "True" or not name:
            continue
        if name == "QV":
            q = sl.find("QV/P[@id='QV']")
            wert = _ohne_eigene_struktur(q.get("val", "")) if q is not None else ""
        else:
            wert = _text_wert(sl, f"../P[@id='{name}']")
        zeilen += [z for z in _zeilen(wert).split("\n") if z.strip()]
    if not zeilen:
        return []
    st = _stil(db.get("style"))
    size = _zahl(st.get("size"), SIZE_VORGABE)
    x, y = _zahl(db.get("x")), _zahl(db.get("y"))
    st_oben = dict(st, align=str(int(_zahl(st.get("align"))) & ~24))
    aus = []
    for k, z in enumerate(zeilen):
        aus += text_formen(z, x, y + k * ZEILE_DATENBLOCK * size, st_oben, ab_fn)
    return aus


def _unter(knoten, tag):
    """Nachfahren mit diesem Tag, ohne in eingebettete Symbole (<SL> im <SL>, z. B. den Kontaktspiegel einer
    Spule) hinabzusteigen – die haben eigene, absolute Lage und gehören nicht zum Bauteil."""
    for k in knoten:
        if k.tag == tag:
            yield k
        if k.tag != "SL":
            yield from _unter(k, tag)


def pins(sl):
    """Alle Anschlüsse eines Symbols (auch in PinGP geschachtelt): (Knoten, x, y, Richtung)."""
    aus = []
    for p in _unter(sl, "Pin"):
        x, y = _punkte(p.get("pos"))[0] if p.get("pos") else (0.0, 0.0)
        r = int(_zahl(p.get("connDir") or p.get("dir"), 0))
        aus.append((p, x, y, r if r in (1, 2, 4, 8) else 0))
    return aus


def pin_texte(sl, ab_fn):
    """Anschlussnummer (nr an posNR) und -name (nm an posNM), sofern nicht ausgeblendet."""
    aus = []
    vpins = []
    for v in _unter(sl, "VPin"):                     # virtuelle Anschlüsse (SPS-Kanalliste): nur ihre Texte
        x, y = _punkte(v.get("pos"))[0] if v.get("pos") else (0.0, 0.0)
        vpins.append((v, x, y, 0))
    for p, px, py, _ in pins(sl) + vpins:
        for wert, lage, stil, aus_ in (("nr", "posNR", "styleNR", "hideNR"), ("nm", "posNM", "styleNM", "hideNM")):
            t = p.get(wert)
            if not t or p.get(aus_) is not None or not p.get(lage):
                continue
            dx, dy = _punkte(p.get(lage))[0]
            aus += text_formen(t, px + dx, py + dy, _stil(p.get(stil)), ab_fn)
    return aus


def bild(e, ox, oy, hoehe, arbeit):
    """<image> (Base64, PNG/JPEG) -> QET-Blattbild über eplan_grafik.bild_xml."""
    try:
        roh = base64.b64decode(e.text or "")
    except ValueError:
        return None
    if len(roh) < 8:
        return None
    endung = ".png" if roh[:4] == b"\x89PNG" else ".jpg"
    datei = os.path.join(arbeit, hashlib.sha1(roh).hexdigest()[:12] + endung)
    if not os.path.exists(datei):
        with open(datei, "wb") as f:
            f.write(roh)
    x, y, w, h = (_zahl(e.get(k)) for k in ("x", "y", "wd", "hg"))
    faktor = _zahl(e.get("faktor"), 1.0) or 1.0         # angezeigte Größe = wd/hg × faktor (Rittal-Logo 0,06)
    w, h = w * faktor, h * faktor
    m = RAND_PX / G.K
    return G.bild_xml(datei, ox + x + m, hoehe - (oy + y) - m, ox + x + w + m, hoehe - (oy + y + h) - m)


# --- Seite --------------------------------------------------------------------------------------------------------

def _dateiname(text):
    return re.sub(r"[^A-Za-z0-9ÄÖÜäöüß_.-]+", "_", text or "Symbol").strip("_")[:50] or "Symbol"


class Seite:
    """Eine Schaltplanseite: Symbole, Anschlusspunkte und das gezeichnete Leitungsnetz (für die Leiterwege)."""

    def __init__(self, info, file_):
        self.info, self.f = info, file_
        hdr = file_.find("Header")
        ps = hdr.find("PaperSize") if hdr is not None else None
        b, h = _punkte(ps.get("val"))[0] if ps is not None and ps.get("val") else (info["breite"], info["hoehe"])
        self.breite, self.hoehe = b or 297.0, h or 210.0
        self.symbole = file_.findall("SL")
        self.anschl = {}                         # Punktschlüssel -> [(sl, index, Richtung)]
        self.weiter = collections.defaultdict(set)   # Netzknoten: Punktschlüssel -> Nachbarn (Leitungsstücke)
        self.durch = set()                       # Punkte, über die ein Leiterweg laufen darf (Winkel, T, Leitungszug)
        self.punkt = {}
        self.je_hid = {}                         # SymbolID -> (Ursprung, [(x, y) der Anschlüsse relativ])
        for sl in self.symbole:
            ox, oy = _zahl(sl.get("x")), _zahl(sl.get("y"))
            a, _ = _art(sl)
            ps_ = pins(sl)
            hid = int(_zahl(sl.get("hid")))
            if hid:
                self.je_hid[hid] = ((ox, oy), [(px, py) for _, px, py, _ in ps_])
            ks = []
            for i, (_, px, py, r) in enumerate(ps_):
                p = (ox + px, oy + py)
                k = _schluessel(p)
                ks.append(k)
                self.punkt[k] = p
                self.anschl.setdefault(k, []).append((sl, i, r))
                if a == 10:
                    self.durch.add(k)
            if a == 10:                          # Winkel/T/Brücke: alle Anschlüsse untereinander verbunden
                for k1 in ks:
                    for k2 in ks:
                        if k1 != k2:
                            self._kante(k1, k2)
        self._gegenueber()
        for cn in file_.findall("CN"):           # gezeichnete Leitungszüge
            for pl in cn.findall("polyline"):
                pts = _punkte(pl.get("points"))
                for p, q in zip(pts, pts[1:]):
                    kp, kq = _schluessel(p), _schluessel(q)
                    self.punkt[kp], self.punkt[kq] = p, q
                    self._kante(kp, kq)
                for p in pts[1:-1]:
                    self.durch.add(_schluessel(p))

    def anschlusslage(self, hid, pin_db):
        """Lage (mm, Blatt) des Datenbank-Anschlusses (PinID, (Name, Nummer, x, y relativ zur Symbolbasis)):
        die PinID zählt die <Pin> des Symbols in Dateireihenfolge (am Demoprojekt 752 von 752 Symbolen mit gleich
        vielen Anschlüssen); sonst der Anschluss gleicher relativer Lage. Die Symbolbasis der Datenbank (xBase)
        weicht bei gedrehten Pfeilen ab, daher der Ursprung aus der Seite."""
        if hid not in self.je_hid:
            return None
        (ox, oy), rel = self.je_hid[hid]
        pid, (_, _, rx, ry) = pin_db
        if not rel:
            return None
        if 1 <= pid <= len(rel) and math.dist(rel[pid - 1], (rx, ry)) <= 0.05:
            px, py = rel[pid - 1]
        else:
            px, py = min(rel, key=lambda q: math.dist(q, (rx, ry)))
        return (ox + px, oy + py)

    def _kante(self, a, b):
        self.weiter[a].add(b)
        self.weiter[b].add(a)

    def _gegenueber(self):
        """WSCAD verbindet einen Anschluss mit dem nächsten Anschluss in seiner Richtung, der zu ihm zeigt."""
        alle = [(k, r) for k, lst in self.anschl.items() for _, _, r in lst if r]
        for k, r in alle:
            p = self.punkt[k]
            dx, dy = SCHRITT[r]
            best = None
            for k2, r2 in alle:
                if r2 != GEGEN[r] or k2 == k:
                    continue
                q = self.punkt[k2]
                if dx and abs(q[1] - p[1]) <= TOL and (q[0] - p[0]) * dx > TOL:
                    d = abs(q[0] - p[0])
                elif dy and abs(q[0] - p[0]) <= TOL and (q[1] - p[1]) * dy > TOL:
                    d = abs(q[1] - p[1])
                else:
                    continue
                if best is None or d < best[0]:
                    best = (d, k2)
            if best:
                self._kante(k, best[1])

    def weg(self, a, b):
        """Kürzester gezeichneter Weg von Anschlusspunkt a nach b (Punkte), nur über Winkel/T/Leitungszüge."""
        ka, kb = _schluessel(a), _schluessel(b)
        if ka == kb:
            return [a]
        vor = {ka: None}
        offen = collections.deque([ka])
        while offen:
            k = offen.popleft()
            if k == kb:
                break
            if k != ka and k not in self.durch:
                continue
            for n in self.weiter.get(k, ()):
                if n not in vor:
                    vor[n] = k
                    offen.append(n)
        if kb not in vor:
            return None
        pfad, k = [], kb
        while k is not None:
            pfad.append(self.punkt[k])
            k = vor[k]
        return pfad[::-1]


def _senkrecht(a, b):
    return abs(a[0] - b[0]) <= TOL


def _waagerecht(a, b):
    return abs(a[1] - b[1]) <= TOL


def ecken(pfad):
    """Weg -> Eckpunkte zwischen den Enden für einen QET-Leiter (nur waagerecht/senkrecht). WSCADs Winkel-
    symbole haben ihre Anschlüsse 1,25 mm vor der Ecke ((0;1,25)-(0;0)-(1,25;0)): das schräge Stück zwischen
    zwei Anschlüssen wird zur Ecke im Schnittpunkt der Nachbarstrecken; gerade Durchgänge fallen weg."""
    if not pfad or len(pfad) < 2:
        return []
    P = [pfad[0]]
    for p in pfad[1:]:
        if math.dist(p, P[-1]) > TOL:
            P.append(p)
    Q = [P[0]]
    for i in range(1, len(P)):
        a, b = Q[-1], P[i]
        if not (_senkrecht(a, b) or _waagerecht(a, b)):
            vor = Q[-2] if len(Q) > 1 else None
            nach = P[i + 1] if i + 1 < len(P) else None
            if vor is not None:
                c = (a[0], b[1]) if _senkrecht(vor, a) else (b[0], a[1])
            elif nach is not None:
                c = (b[0], a[1]) if _senkrecht(b, nach) else (a[0], b[1])
            else:
                c = (a[0], b[1])
            Q.append(c)
        Q.append(b)
    aus = []
    for a, b, c in zip(Q, Q[1:], Q[2:]):
        if (_senkrecht(a, b) and _senkrecht(b, c)) or (_waagerecht(a, b) and _waagerecht(b, c)):
            continue
        aus.append(b)
    return aus


# --- Rollen (Master/Slave) -----------------------------------------------------------------------------------------

def _bmk(sl):
    p = sl.find("P[@id='BMKZ']")
    return (p.get("prd") or "") if p is not None else ""


def _name(sl):
    rn = sl.find("P[@id='ReferenzName']")
    return (rn.get("val") if rn is not None and rn.get("val") else W.text_de(sl.find("P[@id='BtName']"))) or ""


def rollen(seiten, sym_db):
    """Spule (Art 2) bzw. Master (Art 7) und Kontakte (Art 3) mit demselben vollständigen BMK -> Master/Slave."""
    je_bmk = collections.defaultdict(list)
    for se in seiten:
        for sl in se.symbole:
            hid = int(_zahl(sl.get("hid")))
            a, _ = _art(sl)
            if hid and a in (2, 3, 7) and hid in sym_db and pins(sl):
                je_bmk[sym_db[hid]["bmk"]].append((a, hid, sl))
    rolle = {}
    for bmk, lst in je_bmk.items():
        haupt = [x for x in lst if x[0] in (2, 7)]
        kont = [x for x in lst if x[0] == 3]
        if not haupt or not kont or bmk.endswith("-"):
            continue
        a, hid, sl = haupt[0]
        n = _name(sl).lower()
        rolle[hid] = ("master", "protection" if a == 7 or "schutz" in n or "automat" in n else "coil")
        for _, h2, s2 in kont:
            n2 = _name(s2).lower()
            zustand = "NC" if re.search(r"(-o$|öffner|_o$)", n2) else ("NO" if re.search(r"(-s$|schlie)", n2)
                                                                         else "SW")
            rolle[h2] = ("slave", zustand, "simple")
    return rolle, je_bmk


# --- Umsetzen ------------------------------------------------------------------------------------------------------

def _schriftfeld_leer():
    from eplan_nach_qet import _schriftfeld_leer as leer
    return leer()


def _px(p):
    """WSCAD-Blattpunkt (mm, y nach unten) -> QET-Szene (px)."""
    return round(G.X0 + RAND_PX + p[0] * G.K, 1), round(RAND_PX + p[1] * G.K, 1)


def _schreibe_element(xml, name, stat):
    sig = hashlib.sha1(re.sub(r'uuid="\{[^}]*\}"', "", xml).encode("utf-8")).hexdigest()[:10]
    rel = f"{ORDNER}/{_dateiname(name)}_{sig}.elmt"
    ziel = os.path.join(S.COMPANY, *rel.split("/"))
    if not os.path.exists(ziel):
        os.makedirs(os.path.dirname(ziel), exist_ok=True)
        with open(ziel, "w", encoding="utf-8") as f:
            f.write(xml)
        stat["schaltbilder_neu"] += 1
    return S.U + rel


def _anschluss_index(t, lokal):
    """QET-Index des Anschlusses an der lokalen WSCAD-Lage (mm, y nach unten) eines Bauteils."""
    rx, ry = lokal[0] * G.K, lokal[1] * G.K
    return min(t.symbol.anschluesse, key=lambda a: math.hypot(a.x - rx, a.y - ry)).index


def _infos(sl):
    infos = {}
    art = _wert(sl.find("P[@id='Artikel']")).strip()
    her = _wert(sl.find("P[@id='Hersteller']")).strip()
    if art and art.lower() != "bestand":
        infos["manufacturer_reference"] = art
    if her:
        infos["manufacturer"] = her
    bt = _wert(sl.find("P[@id='BtName']"))
    if bt:
        infos["designation"] = bt.replace("\n", " ")
    ft = _wert(sl.find("P[@id='Funktionstext']"))
    if ft:
        infos["function"] = ft.replace("\n", " ")
    return infos


def symbol_texte(sl, ab, mit_bmk=True):
    """Texte eines Symbols relativ zu seinem Ursprung: Platzhalter, Querverweis, Datenblock, Kanalgruppen,
    Anschlussnummern."""
    fo = platzhalter(sl, ab, mit_bmk=mit_bmk)
    # Querverweis: zeigt ihn der Datenblock („QV,False“), zeichnet WSCAD den QV-Platzhalter (Vorgabelage etwa
    # (-40; -45)) nicht (PDF: -Q52 „/10.6“ nur im Datenblock)
    qv_im_block = any(re.search(r"(^|;)QV,False", db.get("params") or "") for db in sl.findall("DataBlock"))
    for q in ([] if qv_im_block else sl.findall("QV")):
        fo += platzhalter(q, ab, mit_bmk=False)
    for db in sl.findall("DataBlock"):
        fo += datenblock(sl, db, ab)
    for gp in _unter(sl, "PinGP"):               # Kontakt- und SPS-Kanalgruppen: Platzhalter und Texte
        fo += platzhalter(gp, ab, mit_bmk=False)
        for q in gp.findall("QV"):
            fo += platzhalter(q, ab, mit_bmk=False)
        for ph in gp.findall("PH"):
            st = _stil(ph.get("style"))
            wert, box = kasten(_wert(ph.find("P")), ph, st)
            fo += text_formen(wert, _zahl(ph.get("x")), _zahl(ph.get("y")), st, ab, box=box)
    fo += pin_texte(sl, ab)
    return fo


def symbolgrafik(sl, H, arbeit):
    """Symbol ohne Bauteil (Grafik, Kontaktspiegel …) als Blattgrafik: (Formen, Bilder)."""
    ox, oy = _zahl(sl.get("x")), _zahl(sl.get("y"))
    ab = Abbild(ox, oy, H)
    fo = formen(sl, ab) + symbol_texte(sl, ab, mit_bmk=False)
    bilder = [bx for bx in (bild(im, ox, oy, H, arbeit) for im in _unter(sl, "image")) if bx]
    return fo, bilder


# --- Bauteile in die Firmensammlung ------------------------------------------------------------------------------

# WSCAD schreibt Hersteller uneinheitlich („SIEMENS“, „Siemens AG“, „PHOENIX“): Kurzformen -> Ordnername
HERSTELLER_ALIAS = {"phoenix": "Phoenix Contact", "weidmueller": "Weidmüller", "lapp kabel": "Lapp", "lapp": "Lapp", "wöhner": "Wöhner",
                    "endress hauser": "Endress+Hauser", "schneider electric": "Schneider Electric"}
RECHTSFORM = re.compile(r"\b(gmbh|ag|kg|co\.?|& co\.? kg|interface|se|inc\.?|ltd\.?)\b|[&+]", re.I)
# SL-Art -> Kategorieordner wie in der Sammlung (eplan_bibliothek.KATEGORIE)
KATEGORIE_ART = {4: "Klemmen", 12: "Steckverbinder", 19: "Steckverbinder", 15: "SPS", 16: "SPS"}
KATEGORIE_NAME = [(re.compile(r"schütz|schuetz|relais|pnoz|sicherheits", re.I), "Relais, Schütze"),
                  (re.compile(r"automat|schutzschalter|leistungsschalter|sicherung|fi[-/ ]|rcbo|mcb", re.I),
                   "Schutzeinrichtungen"),
                  (re.compile(r"taster|schalter|not-aus|sensor|näherung|druckschalter|positionsschalter", re.I),
                   "Sensorik, Schalter und Taster"),
                  (re.compile(r"sitop|netzgerät|stromversorgung|usv|netzteil", re.I), "Stromversorgungen"),
                  (re.compile(r"motor", re.I), "Antriebe"),
                  (re.compile(r"leucht|lampe|signals", re.I), "Befehlsgeräte"),
                  (re.compile(r"klemme", re.I), "Klemmen"),
                  (re.compile(r"cpu|simatic|sps|eingabe|ausgabe", re.I), "SPS")]
# keine eigenen Bauteile: Verbindungssymbole, Pfeile, Kabelader, Kabelschirm, Leitungsattribut, Kontaktspiegel
OHNE_BAUTEIL = {9, 10, 11, 14, 20, 23}


def hersteller_ordner(name, vorhanden):
    """WSCAD-Herstellername -> Ordner der Sammlung (vorhandener Ordner gleichen Namens bevorzugt)."""
    roh = re.sub(r"\s+", " ", RECHTSFORM.sub(" ", name or "")).strip(" .,")
    schluessel = roh.lower()
    if schluessel in HERSTELLER_ALIAS:
        roh = HERSTELLER_ALIAS[schluessel]
    for v in vorhanden:
        if v.lower() == roh.lower() or v.lower().replace(" ", "") == roh.lower().replace(" ", ""):
            return v
    for v in vorhanden:                          # „Phoenix“ -> „Phoenix Contact“
        if len(roh) >= 4 and v.lower().startswith(roh.lower() + " "):
            return v
    return roh.title() if roh.isupper() else roh or "Sonstige"


def kategorie_wscad(sl):
    a, _ = _art(sl)
    if a in KATEGORIE_ART:
        return KATEGORIE_ART[a]
    n = _name(sl) + " " + _wert(sl.find("P[@id='BtName']"))
    for rx, kat in KATEGORIE_NAME:
        if rx.search(n):
            return kat
    return "Geräte"


def _qet_directory(ordner, wurzel):
    """qet_directory je neuem Ordner (Name de/en wie der Ordner), wie die übrigen Importe."""
    pfad = wurzel
    for teil in ordner.split("/"):
        pfad = os.path.join(pfad, teil)
        datei = os.path.join(pfad, "qet_directory")
        if os.path.isdir(pfad) and not os.path.exists(datei):
            import html as _h
            with open(datei, "w", encoding="utf-8") as f:
                f.write(f'<qet-directory>\n    <names>\n        <name lang="de">{_h.escape(teil)}</name>\n'
                        f'        <name lang="en">{_h.escape(teil)}</name>\n    </names>\n</qet-directory>\n')


def in_sammlung(sl, sammlung, stat):
    """Artikel des Symbols, den die Firmensammlung noch nicht kennt, als Bauteil ablegen: Hersteller/Kategorie/
    Bestellnummer (vorhandener Ordner der Bestellnummer hat Vorrang), Datei <Bestellnr>_WSCAD-Schaltbild_<Symbol>.
    Das Schaltbild trägt nur, was zum Bauteil gehört (Grafik, Anschlüsse mit Nummern, Kennzeichen), keine
    Plan-Texte (Querverweis, Funktionstext); Artikeldaten als Elementinformationen."""
    a, _ = _art(sl)
    art = _wert(sl.find("P[@id='Artikel']")).strip()
    if a in OHNE_BAUTEIL or not art or art.lower() == "bestand" or not pins(sl):
        return None
    if sammlung.hat(art) and art not in stat_neu_artikel:
        stat["artikel_vorhanden"] += 1
        return None
    vorhanden = [d for d in os.listdir(sammlung.wurzel) if os.path.isdir(os.path.join(sammlung.wurzel, d))]
    # Hersteller am Symbol, sonst aus der Artikeldatenbank des Projekts bzw. einem anderen Symbol desselben Artikels
    her = hersteller_ordner(_wert(sl.find("P[@id='Hersteller']")) or HERSTELLER_JE_ARTIKEL.get(BIB.norm(art), ""),
                            vorhanden)
    typ = re.sub(r'[<>:"/\\|?*]', "_", art)
    ordner = sammlung.ordner.get(BIB.norm(art), "").replace(os.sep, "/") or f"{her}/{kategorie_wscad(sl)}/{typ}"
    ab = Abbild(0.0, 0.0)
    fo = formen(sl, ab) + [f for f in platzhalter(sl, ab) if f[0] == "D"] + pin_texte(sl, ab)
    ps = pins(sl)
    an = [(px, -py, r or 4) for _, px, py, r in ps]
    namen = [p.get("nr") or p.get("nm") or str(i + 1) for i, (p, _, _, _) in enumerate(ps)]
    bez = _wert(sl.find("P[@id='BtName']")).replace("\n", " ")
    infos = {"manufacturer": her, "manufacturer_reference": art, "designation": bez or art}
    xml = G.element_xml(f"{art} {_name(sl)}".strip(), fo, an,
                        "Aus einem WSCAD-Projekt übernommen (wscad_nach_qet.py)", namen=namen, infos=infos)
    datei = f"{re.sub(r'[^A-Za-z0-9_+.-]', '_', art)}_WSCAD-Schaltbild_{_dateiname(_name(sl))}.elmt"
    ziel = os.path.join(sammlung.wurzel, *ordner.split("/"), datei)
    if os.path.exists(ziel):
        return None
    os.makedirs(os.path.dirname(ziel), exist_ok=True)
    with open(ziel, "w", encoding="utf-8") as f:
        f.write(xml)
    _qet_directory(ordner, sammlung.wurzel)
    stat_neu_artikel.add(art)
    stat["bauteile_in_sammlung"] += 1
    sammlung.neu.add(f"{ordner}/{datei}")
    return f"{ordner}/{datei}"


stat_neu_artikel = set()                         # in diesem Lauf angelegte Artikel (weitere Symbolarten dazu)
HERSTELLER_JE_ARTIKEL = {}                       # normierte Bestellnummer -> Hersteller (article.db, Symbole)


def bauteil(sl, b, rolle_, stat):
    """Symbol als QET-Bauteil: eigene Grafik, Texte, Anschlüsse; gesetzt am Symbolursprung (SL x/y)."""
    ox, oy = _zahl(sl.get("x")), _zahl(sl.get("y"))
    ab = Abbild(0.0, 0.0)
    fo = formen(sl, ab) + symbol_texte(sl, ab)
    ps = pins(sl)
    an = [(px, -py, r or 4) for _, px, py, r in ps]
    namen = [p.get("nr") or p.get("nm") or str(i + 1) for i, (p, _, _, _) in enumerate(ps)]
    name = _name(sl) or "Symbol"
    xml = G.element_xml(f"WSCAD {name}", fo, an, "Aus einem WSCAD-Projekt übernommen (wscad_nach_qet.py)",
                        namen=namen)
    hid = int(_zahl(sl.get("hid")))
    xml = G.mit_rolle(xml, rolle_.get(hid))
    if not any(f[0] == "D" for f in fo):      # kein sichtbares BMK: Kennzeichen nur als Information
        xml = re.sub(r'\n\s*<dynamic_text [^>]*text_from="ElementInfo"[^>]*><text></text>'
                     r'<info_name>label</info_name></dynamic_text>', "", xml)
    pfad = _schreibe_element(xml, name, stat)
    t = b.setze(pfad, *_px((ox, oy)), label=_bmk(sl), infos=_infos(sl), label_xy=())
    t.wx, t.wy = ox, oy
    stat["bauteile"] += 1
    stat["anschluesse"] += len(ps)
    return t


def verweis(sl, b, gehend, stat):
    """Pfeil mit Ziel (Art 14) als QET-Verweiselement; Pfeiltext und Querverweis kommen als Blatttext dazu."""
    ox, oy = _zahl(sl.get("x")), _zahl(sl.get("y"))
    fo = formen(sl, Abbild(0.0, 0.0), texte=False)
    ps = pins(sl)
    an = [(px, -py, r or 4) for _, px, py, r in ps[:1]]
    xml = G.element_xml("WSCAD Pfeil " + ("gehend" if gehend else "kommend"), fo, an,
                        "Aus einem WSCAD-Projekt übernommen (wscad_nach_qet.py)", namen=["1"])
    xml = xml.replace('link_type="simple"', 'link_type="%s"' % ("next_report" if gehend else "previous_report"), 1)
    xml = re.sub(r'\n\s*<dynamic_text [^>]*text_from="ElementInfo"[^>]*><text></text>'
                 r'<info_name>label</info_name></dynamic_text>', "", xml)
    pfad = _schreibe_element(xml, "Pfeil_" + ("gehend" if gehend else "kommend"), stat)
    t = b.setze(pfad, *_px((ox, oy)), label="", label_xy=())
    t.wx, t.wy = ox, oy
    stat["pfeile"] += 1
    return t


def _ist_gehend(sl):
    """„Pfeil: Richtung rechts (mit Ziel)“ geht weg, „Pfeil: Richtung von links (mit Ziel)“ kommt an."""
    return " von " not in (" " + _name(sl).lower() + " ")


QV_ZIEL = re.compile(r"^\s*(=[^+/]*)?(\+[^/]*)?/(\d+)\.")


def pfeilpaare(seiten):
    """Pfeile mit Ziel über alle Seiten paaren, bevor gezeichnet wird. Partner = gleicher Leitungsname auf einer
    anderen Seite; Vorrang: beide Querverweise zeigen aufeinander („/2.0“ = Blatt 2 derselben Struktur,
    „=A1+P1/3.2“ eine andere), dann der Querverweis dieses Pfeils, dann das nächste spätere, dann das letzte frühere
    Blatt. Gehend ist der Pfeil auf dem früheren Blatt, kommend der auf dem späteren – die Pfeilrichtung im
    Symbolnamen taugt nicht (Projekte ohne Namen; alle „Richtung rechts“). -> (Paare [(sl, sl)], {id(sl): gehend})."""
    pf = []
    for si, se in enumerate(seiten):
        werte = _blatt_aus_schriftfeld(se.f.find("Template"))
        nummer = werte.get("Blattnummer") or str(si + 1)
        anlage, ort = se.info["anlage"] or "", se.info["ort"] or ""
        for sl in se.symbole:
            if _art(sl)[0] != 14 or not pins(sl):
                continue
            name = _wert(sl.find("P[@id='Leitungsname']"))
            q = sl.find("QV/P[@id='QV']")
            m = QV_ZIEL.match(q.get("val", "") if q is not None else "")
            ziel = None
            if m:
                ziel = ((m.group(1) or "=" + anlage).lstrip("="), (m.group(2) or "+" + ort).lstrip("+"), m.group(3))
            pf.append({"sl": sl, "seite": si, "name": name, "wo": (anlage, ort, nummer), "ziel": ziel})
    paare, frei = [], set(range(len(pf)))
    for i, a in enumerate(pf):
        if i not in frei or not a["name"]:
            continue
        kand = [j for j in frei if j != i and pf[j]["name"] == a["name"] and pf[j]["seite"] != a["seite"]]
        if not kand:
            continue

        def rang(j):
            b = pf[j]
            hin, her = a["ziel"] == b["wo"], b["ziel"] == a["wo"]
            spaeter = b["seite"] > a["seite"]
            return (not (hin and her), not hin, not spaeter, abs(b["seite"] - a["seite"]))
        j = min(kand, key=rang)
        frei -= {i, j}
        paare.append((a, pf[j]) if a["seite"] < pf[j]["seite"] else (pf[j], a))
    richtung = {}
    for a, b in paare:
        richtung[id(a["sl"])], richtung[id(b["sl"])] = True, False
    return [(a["sl"], b["sl"]) for a, b in paare], richtung, len(frei)


class _Vereinigung:
    def __init__(self):
        self.p = {}

    def finde(self, a):
        self.p.setdefault(a, a)
        while self.p[a] != a:
            self.p[a] = self.p[self.p[a]]
            a = self.p[a]
        return a

    def vereine(self, a, b):
        a, b = self.finde(a), self.finde(b)
        if a != b:
            self.p[b] = a


def _blatt_aus_schriftfeld(tp):
    werte = {}
    if tp is not None:
        for ph in tp.findall("PH"):
            p = ph.find("P")
            if p is not None:
                werte[p.get("id")] = _wert(p)
    return werte


def umsetzen(ordner, ziel_qet, pdf=False, bibliothek=True):
    _fortschritt(2, "lesen")
    os.environ.setdefault("QET_COMPANY_ELEMENTS_DIR", S.COMPANY)
    G.K, G.X0 = K_WSCAD, X0_WSCAD
    schrift_wie_wscad()
    P = W.Projekt(ordner)
    sym_db = P.symbole()
    pins_db = P.anschluesse()
    arbeit = os.path.join(os.path.dirname(os.path.abspath(ziel_qet)),
                          "_wscad_import_" + re.sub(r"\W+", "_", os.path.basename(os.path.normpath(ordner))))
    os.makedirs(arbeit, exist_ok=True)
    stat, nicht = collections.Counter(), collections.Counter()

    _fortschritt(6, "seiten")
    seiten = []
    for s in P.schaltplanseiten():
        try:
            seiten.append(Seite(s, W.seite_xml(s["datei"])))
        except Exception as e:                   # Seite nicht lesbar: melden, Rest weiter
            print(f"Seite {s['name']} nicht lesbar: {e}")
            nicht["seite_unlesbar"] += 1
    if not seiten:
        raise SystemExit("keine lesbare Schaltplanseite (.wsELD) im Projekt")
    rolle, je_bmk = rollen(seiten, sym_db)
    HERSTELLER_JE_ARTIKEL.clear()
    HERSTELLER_JE_ARTIKEL.update({BIB.norm(k): v for k, v in W.artikel_hersteller(ordner).items()})
    for se in seiten:
        for sl in se.symbole:
            art_, her_ = _wert(sl.find("P[@id='Artikel']")), _wert(sl.find("P[@id='Hersteller']"))
            if art_ and her_:
                HERSTELLER_JE_ARTIKEL.setdefault(BIB.norm(art_), her_)

    plan = Plan(os.path.basename(os.path.normpath(ordner)), os.path.dirname(os.path.abspath(ziel_qet)),
                _schriftfeld_leer())
    teil_je_hid, je_blatt, verweis_ops, teil_je_pfeil = {}, {}, [], {}
    pfeil_paare_sl, pfeil_gehend, pfeil_allein = pfeilpaare(seiten)
    sammlung = None
    if bibliothek:
        try:
            sammlung = BIB.Sammlung()
        except Exception as e:                   # Sammlung nicht lesbar: dann ohne Ablage
            print("Firmensammlung nicht lesbar:", e)

    for si, se in enumerate(seiten):
        _fortschritt(10 + 70 * si / max(1, len(seiten)), "seiten")
        tp = se.f.find("Template")
        werte = _blatt_aus_schriftfeld(tp)
        EIGENE_STRUKTUR[0] = f"={se.info['anlage']}+{se.info['ort']}" if se.info["anlage"] or se.info["ort"] else ""
        b = plan.blatt(werte.get("Dateiinhalt", "").replace("\n", " ") or se.info["name"],
                       (werte.get("Funktion") or se.info["anlage"]).lstrip("="),
                       (werte.get("Ort") or se.info["ort"]).lstrip("+"), werte.get("Blattnummer") or str(si + 1))
        b.ops.append({"op": "set_folio_border", "folio": b.folio, "property": "preset",
                      "value": "a3-landscape" if se.breite > 400 else "a4-landscape"})
        b.ops.append({"op": "set_folio_border", "folio": b.folio, "property": "display-columns", "value": "false"})
        H = se.hoehe
        G.H_MM = H                               # eplan_grafik: y nach oben ab der Seitenhöhe (Bilder, Texte)
        blatt_ab = Abbild(0.0, 0.0, H)
        formen_b, bilder_b = [], []
        # Rahmen und Schriftfeld (WSCAD-Vorlage der Seite) als Blattgrafik
        if tp is not None:
            formen_b += formen(tp, blatt_ab)
            for ph in tp.findall("PH"):
                st = _stil(ph.get("style"))
                wert, box = kasten(_wert(ph.find("P")), ph, st)
                formen_b += text_formen(wert, _zahl(ph.get("x")), _zahl(ph.get("y")), st, blatt_ab, box=box)
            for lg in tp.findall("logo"):
                for im in lg.iter("image"):
                    if not _zahl(im.get("wd")):         # ältere Projekte (SUITE 2018): Bild ohne Maß, Rahmen am Logo
                        for k in ("x", "y", "wd", "hg"):
                            im.set(k, lg.get(k, "0"))
                    bx = bild(im, 0.0, 0.0, H, arbeit)          # Lage im Logo absolut (= Logo x/y)
                    if bx:
                        bilder_b.append(bx)
        # freie Seitengrafik und -texte
        formen_b += formen(se.f, blatt_ab)
        for cl in se.f.findall("Cableline"):
            formen_b.append(("L", blatt_ab(_zahl(cl.get("x1")), _zahl(cl.get("y1"))),
                             blatt_ab(_zahl(cl.get("x2")), _zahl(cl.get("y2"))), cl.get("lt") not in (None, "0", "1"),
                             None, {}))
        for im in se.f.findall("image"):
            bx = bild(im, 0.0, 0.0, H, arbeit)
            if bx:
                bilder_b.append(bx)

        for sl in se.symbole:
            a, _ = _art(sl)
            ox, oy = _zahl(sl.get("x")), _zahl(sl.get("y"))
            ab_sl = Abbild(ox, oy, H)
            hid = int(_zahl(sl.get("hid")))
            ps = pins(sl)
            if a == 10:                          # Winkel/T/Brücke: die Leitung zeichnet der Leiter
                continue
            if a == 23:                          # Leitungsattribut (Drahtnummer, Potential): Daten am Leiter
                stat["leitungsattribut"] += 1
                continue
            if a == 14 and ps:
                gehend = pfeil_gehend.get(id(sl), _ist_gehend(sl))
                t = verweis(sl, b, gehend, stat)
                teil_je_pfeil[id(sl)] = t
                if hid:
                    teil_je_hid[hid] = t
                formen_b += platzhalter(sl, ab_sl, mit_bmk=False)
                for q in sl.findall("QV"):
                    formen_b += platzhalter(q, ab_sl, mit_bmk=False)
                continue
            # eingebettete Symbole (Kontaktspiegel der Spule, SL-Art 9): eigene, absolute Lage -> Blattgrafik
            for innen in sl.findall("SL"):
                f_, b_ = symbolgrafik(innen, H, arbeit)
                formen_b += f_
                bilder_b += b_
                stat["kontaktspiegel"] += 1
            if ps and hid:
                teil_je_hid[hid] = bauteil(sl, b, rolle, stat)
                if sammlung is not None:
                    in_sammlung(sl, sammlung, stat)
                for im in _unter(sl, "image"):   # Bilder im Symbol (Herstellerlogo): QET-Schaltbilder haben
                    bx = bild(im, ox, oy, H, arbeit)    # keine Bilder, daher als Blattbild an derselben Stelle
                    if bx:
                        bilder_b.append(bx)
                continue
            # ohne Anschluss bzw. ohne Betriebsmittel: Blattgrafik
            f_, b_ = symbolgrafik(sl, H, arbeit)
            formen_b += f_
            bilder_b += b_
            stat["grafik_symbol"] += 1
        se.blatt = b
        je_blatt[b.folio] = (formen_b, bilder_b)

    # Verweispaare (vorab über alle Seiten gepaart, pfeilpaare)
    _fortschritt(82, "verweise")
    paare = [(teil_je_pfeil[id(x)], teil_je_pfeil[id(y)]) for x, y in pfeil_paare_sl
             if id(x) in teil_je_pfeil and id(y) in teil_je_pfeil]
    nicht["pfeil_ohne_partner"] += pfeil_allein

    # Leitungen aus der Verbindungsliste der Datenbank. WSCAD führt Brücken (Klemmen, Verteiler) als vollständiges
    # Netz (jede mit jeder); gezeichnet wird je Blatt ein Spannbaum: zuerst Verbindungen mit gezeichnetem Weg,
    # dann die kürzesten, eine nur, wenn ihre Enden noch nicht verbunden sind.
    _fortschritt(86, "verbindungen")
    seite_je_id = {se.info["id"]: se for se in seiten}
    kandidaten = collections.defaultdict(list)
    for v in P.verbindungen():
        se = seite_je_id.get(v["PageID"])
        if se is None:
            nicht["verbindung_ohne_schaltplanseite"] += 1
            continue
        enden = []
        for sid, pin in ((v["SymbolID1"], v["Pin1"]), (v["SymbolID2"], v["Pin2"])):
            t = teil_je_hid.get(sid)
            pd = pins_db.get((sid, pin))
            lage = se.anschlusslage(sid, (pin, pd)) if pd is not None else None
            if t is None or lage is None or t.blatt is not se.blatt:
                enden.append(None)
                continue
            enden.append((t, lage))
        if None in enden:
            nicht["ende_ohne_bauteil"] += 1
            continue
        (ta, pa), (tb, pb) = enden
        if math.dist(pa, pb) <= TOL:
            nicht["deckungsgleich"] += 1
            continue
        ia = _anschluss_index(ta, (pa[0] - ta.wx, pa[1] - ta.wy))
        ib = _anschluss_index(tb, (pb[0] - tb.wx, pb[1] - tb.wy))
        if ta is tb and ia == ib:
            continue
        w = se.weg(pa, pb)
        kandidaten[se.blatt.folio].append((w is None, math.dist(pa, pb), ta, ia, tb, ib, w, v))
    gezeichnet = 0
    grad, ende_teil, werte_je_ende = collections.Counter(), {}, {}
    netz = _Vereinigung()
    for folio in sorted(kandidaten):
        for ohne_weg, _, ta, ia, tb, ib, w, v in sorted(kandidaten[folio], key=lambda k: (k[0], k[1])):
            b = ta.blatt
            ea, eb = (b.folio, ta.ref, ia), (b.folio, tb.ref, ib)
            if netz.finde(ea) == netz.finde(eb):
                stat["doppelt_verbunden"] += 1
                continue
            b.draht(ta, ia, tb, ib)
            gezeichnet += 1
            if w:
                ek = ecken(w)
                if ek:
                    b.ops.append({"op": "set_conductor_path", "folio": b.folio, "from": ta.ref,
                                  "from_terminal": ta.an(ia).uuid, "to": tb.ref, "to_terminal": tb.an(ib).uuid,
                                  "corners": ";".join("%g,%g" % _px(e) for e in ek)})
                    stat["leiterweg"] += 1
            else:
                stat["leiterweg_ohne"] += 1
            grad[ea] += 1
            grad[eb] += 1
            netz.vereine(ea, eb)
            ende_teil[ea], ende_teil[eb] = (b, ta, ia), (b, tb, ib)
            c = {"farbe": (v["Colorcode"] or "").strip(), "querschnitt": (v["Dimension"] or "").strip(),
                 "potential": (v["Potential"] or "").strip()}
            if any(c.values()):
                werte_je_ende.setdefault(ea, c)

    # Farbe, Querschnitt, Potential gelten in QET für das ganze Potential und müssen an beiden Seiten eines
    # Verweispaars gleich sein, sonst verknüpft QET die Pfeile nicht: ein Wert je Potential über die Verweispaare
    # hinweg, gesetzt je Blattnetz an einem Anschluss mit genau einer Leitung (wie beim EPLAN-Import)
    ueber = _Vereinigung()
    for e in ende_teil:
        ueber.vereine(e, netz.finde(e))
    for ta, tb in paare:
        ea, eb = (ta.blatt.folio, ta.ref, 0), (tb.blatt.folio, tb.ref, 0)
        if ea in ende_teil and eb in ende_teil:
            ueber.vereine(ea, eb)
    je_netz, je_pot = collections.defaultdict(list), collections.defaultdict(set)
    for e in ende_teil:
        je_netz[netz.finde(e)].append(e)
        je_pot[ueber.finde(e)].add(netz.finde(e))
    unsicher = set()
    for netze_ in je_pot.values():
        c = next((werte_je_ende[e] for n in netze_ for e in je_netz[n] if e in werte_je_ende), None)
        if not c:
            continue
        for n in netze_:
            ende = next((e for e in je_netz[n] if grad[e] == 1), None)
            if ende is None:
                nicht["netz_ohne_ende"] += 1
                unsicher.add(n)
                continue
            b, t, i = ende_teil[ende]
            for prop, wert in (("conductor_color", c["farbe"]), ("conductor_section", c["querschnitt"]),
                               ("function", c["potential"])):
                if wert:
                    b.ops.append({"op": "set_conductor", "folio": b.folio, "element": t.ref,
                                  "terminal": t.an(i).uuid, "property": prop, "value": wert})
    # Verweise zuletzt verknüpfen (nach den Kennzeichen-Lagen): ein abgelehntes Paar hält sonst alles Weitere auf
    for ta, tb in paare:
        ea, eb = (ta.blatt.folio, ta.ref, 0), (tb.blatt.folio, tb.ref, 0)
        if netz.finde(ea) in unsicher or netz.finde(eb) in unsicher:
            nicht["verweis_ohne_gleiche_werte"] += 1
            continue
        verweis_ops.append({"op": "link_elements", "folio": ta.blatt.folio, "element": ta.ref,
                            "to_folio": tb.blatt.folio, "to": tb.ref})
        stat["verweise_verknuepft"] += 1

    # Master/Slave verknüpfen (WSCADs Querverweistexte bleiben, QETs eigene aus)
    from eplan_nach_qet import _label_als_text
    for hid, r_ in rolle.items():
        if r_[0] != "master" or hid not in teil_je_hid:
            continue
        mt = teil_je_hid[hid]
        sl_ = [teil_je_hid[h2] for _, h2, _ in je_bmk.get(sym_db[hid]["bmk"], [])
               if rolle.get(h2, ("",))[0] == "slave" and h2 in teil_je_hid]
        if sl_:
            mt.blatt.verbinde(mt, *sl_)
            stat["verknuepft"] += len(sl_)
            for t_ in [mt, *sl_]:
                _label_als_text(t_)

    _fortschritt(92, "schreiben")
    plan.lagen += verweis_ops
    erg, r = plan.erzeuge(os.path.basename(ziel_qet), pdf=False)
    with open(erg, encoding="utf-8") as f:
        s_qet = f.read()
    with open(erg, "w", encoding="utf-8") as f:
        f.write(re.sub(r'<report label="[^"]*"', '<report label=""', s_qet))
    einfuegen = {}
    for folio, (formen_b, bilder_b) in je_blatt.items():
        G.H_MM = seiten[folio].hoehe            # Blattgrafik: y nach oben ab der Seitenhöhe
        shapes, inputs = G.blatt_xml(formen_b, versatz=(RAND_PX / G.K, -RAND_PX / G.K))
        einfuegen[folio] = (shapes, inputs, bilder_b)
    G.in_projekt_einfuegen(erg, einfuegen)

    _fortschritt(96, "listen")
    listen(P, sym_db, ziel_qet)
    fehl = [o for o in r.get("operations", []) if not o.get("succeeded")]
    bericht = (f"Seiten {len(seiten)}, Bauteile {stat['bauteile']} (neue Schaltbilder {stat['schaltbilder_neu']}), "
               f"Anschlüsse {stat['anschluesse']}, Leitungen {gezeichnet} (doppelte der Liste weggelassen "
               f"{stat['doppelt_verbunden']}, Weg wie gezeichnet {stat['leiterweg']}, "
               f"ohne gezeichneten Weg {stat['leiterweg_ohne']}), nicht gezeichnet {dict(nicht)}, Verweispfeile "
               f"{stat['pfeile']} (verknüpft {stat['verweise_verknuepft']} Paare), Master/Slave {stat['verknuepft']}, "
               f"Grafiksymbole {stat['grafik_symbol']}, neue Bauteile in der Firmensammlung {stat['bauteile_in_sammlung']} "
               f"(Artikel schon vorhanden {stat['artikel_vorhanden']}), Leitungsattribute {stat['leitungsattribut']}, "
               f"fehlgeschlagene Operationen {len(fehl)}")
    with open(os.path.join(arbeit, "import_bericht.txt"), "w", encoding="utf-8") as f:
        f.write(bericht + "\n")
    print(bericht)
    if pdf:
        from qet_direkt import q, BIN
        print("pdf:", q.tool_export(BIN, erg, "pdf", erg.replace(".qet", ".pdf")).get("ok"))
    return erg


def listen(P, sym_db, ziel_qet):
    """Geräte- und Verbindungsliste (CSV, Excel-tauglich) neben dem Projekt."""
    basis = os.path.splitext(ziel_qet)[0]
    art = P.artikel()
    seite = {s["id"]: s["name"] for s in P.seiten}
    with open(basis + "_Geraete.csv", "w", encoding="utf-8-sig", newline="") as f:
        w = csv.writer(f, delimiter=";")
        w.writerow(["BMK", "Artikel", "Hersteller", "Seite"])
        gesehen = set()
        for s in sorted(sym_db.values(), key=lambda s: s["bmk"] or ""):
            b = s["bmk"] or ""
            if not b or b.endswith("-") or b in gesehen:
                continue
            gesehen.add(b)
            a = art.get(s["eltern"], {}) or art.get(s["id"], {})
            w.writerow([b, a.get("artikel", ""), a.get("hersteller", ""), seite.get(s["seite"], "")])
    pins_db = P.anschluesse()

    def ende(sid, pin):
        s = sym_db.get(sid, {})
        pd = pins_db.get((sid, pin), ("", "", 0, 0))
        return f"{s.get('bmk', '')}:{pd[1] or pd[0]}"
    with open(basis + "_Verbindungen.csv", "w", encoding="utf-8-sig", newline="") as f:
        w = csv.writer(f, delimiter=";")
        w.writerow(["Quelle", "Ziel", "Potential", "Farbe", "Querschnitt", "Seite"])
        for v in P.verbindungen():
            w.writerow([ende(v["SymbolID1"], v["Pin1"]), ende(v["SymbolID2"], v["Pin2"]), v["Potential"] or "",
                        v["Colorcode"] or "", v["Dimension"] or "", seite.get(v["PageID"], "")])


def standard_ziel(ordner):
    name = os.path.basename(os.path.normpath(ordner))
    return os.path.join(os.path.abspath(ordner), "QET", re.sub(r"\s+", "_", name) + "_WSCAD_Import.qet")


def projektordner(pfad):
    """Ordner, Seitendatei oder Database/WSCAD6.db -> Projektordner."""
    if os.path.isfile(pfad):
        pfad = os.path.dirname(os.path.abspath(pfad))
        if os.path.basename(pfad).lower() == "database":
            pfad = os.path.dirname(pfad)
    return pfad


def haupt(argv):
    args = [a for a in argv if not a.startswith("--")]
    if not args:
        print(__doc__)
        return None
    ordner = projektordner(args[0])
    ziel = args[1] if len(args) > 1 else standard_ziel(ordner)
    os.makedirs(os.path.dirname(os.path.abspath(ziel)), exist_ok=True)
    erg = umsetzen(ordner, ziel, pdf="--pdf" in argv, bibliothek="--ohne-sammlung" not in argv)
    _fortschritt(100, "fertig")
    print("fertig:", erg)
    return erg


if __name__ == "__main__":
    haupt(sys.argv[1:])
