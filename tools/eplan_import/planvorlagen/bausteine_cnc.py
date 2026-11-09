"""Bausteine für CNC-Steuerungen mit E/A-Stationen (Cybelec CybIO, Bedienterminal VisiTouch Pac).

Technik, die sich in Pressenplänen wiederholt (Projekt G =SC, Projekt A =SC):

- Stationsübersicht: Module einer E/A-Station nebeneinander (Kennzeichen, Typ senkrecht darüber).
- Modulübersicht: je Modul ein Bauteil (Master) mit allen Steckern und Pins; daneben je belegtem Pin das Ziel
  (Blatt der Funktion), die SPS-Adresse des Partners und der Funktionstext – das sind Plandaten, sie kommen als
  Belegung aus dem Projekt, nicht aus dem Symbol.
- Kartenversorgung: Versorgungsstecker der Module von Potentialpfeilen, Weiterverbindung zum Nachbarmodul.
- Bedienterminal: Geräteübersicht im Ortsrahmen des Bedienpults; Versorgung über Klemmen, Kabel, Filter.

Cybelec liefert keine EPLAN-Daten. Die Symbole entstehen daher hier aus den Gerätedaten (Pinbelegung je Modultyp,
MODULTYPEN) in der Firmensammlung `Cybelec/Steuerungen/<Typ>/…` (Regel Symbolquellen: Herstellermakro > Normsymbol >
eigener Kasten). Erzeugt wird nur, wenn die Datei fehlt (ueberschreiben=True erzwingt es).

    from planvorlagen import bausteine_cnc as C
    C.station(b, 0, [{"bmk": "-A1", "typ": "IOM-CPU-6C"}, …])
    C.modul_uebersicht(b, 0, {"bmk": "-A2", "typ": "IOM-DI8O2-1A"}, {"X1:1": ("=DC/44", "A60.0", "Permanente Regelung")})
"""
import os
import uuid
from xml.sax.saxutils import escape

from .blatt import BAND, raster, spalte_links
from .symbole import COMPANY
from . import symbole as S
from .bausteine import kabel, klemmen, funktion_bei, rahmen

CYB = "Cybelec"
NEU = os.environ.get("CNC_SYMBOLE_NEU") == "1"     # Symbole neu schreiben (nach Änderungen an den Generatoren)
_GESCHRIEBEN = set()                                # je Lauf jede Datei nur einmal (UUIDs bleiben stabil)
FONT = 'font="Liberation Sans,{g},-1,5,50,0,0,0,0,0,Regular"'

# --- Gerätedaten je Modultyp (Herstellerdokumentation CybIO, Pinbelegung der Stecker) -----------------------------
_DIO_X1 = ["DI1", "DI2", "DI3", "DI4", "0V I/O", "DI5", "DI6", "DI7", "DI8", "0V I/O", "DO1", "DO2", "DO3", "DO4"]
_DIO_X3 = ["0V I/O", "DO5", "DO6", "0V I/O", "DO7", "DO8", "0V I/O", "0V I/O", "DO9", "DO10", "0V I/O", "DO11",
           "DO12", "0V I/O"]
_VERS = ["+24V I/O", "0V I/O", "0V I/O", "+24V I/O"]
_AX = ["+5V ENC{n}", "0V ENC{n}", "A{n}+", "A{n}-", "B{n}+", "B{n}-", "Z{n}+", "Z{n}-", "AO {n}+", "AO {n}-", "AGND",
       "DI{a}", "DI{b}", "DO{n}"]

MODULTYPEN = {
    "IOM-CPU-6C": {"artikel": "IOM - CPU - 6C", "bezeichnung": "Feldbus-Koppler CANopen", "breite": 160,
                   "stecker": [("X1", ["+24 VDC", "0V", "PE", "DAUX"])], "versorgung": "X1",
                   "kurz": "IOM-CPU-6C"},
    "IOM-DI8O2-1A": {"artikel": "IOM-DI8O2-1A", "bezeichnung": "Digitalmodul 8xDE/12x DA", "breite": 80,
                     "stecker": [("X1", _DIO_X1), ("X2", _VERS), ("X3", _DIO_X3)], "versorgung": "X2",
                     "kurz": "IOM-DI8O12"},
    "IOM-AI6O4-1A": {"artikel": "IOM-AI6O4 - 1A", "bezeichnung": "Analogmodul 6xAE/4x AA", "breite": 80,
                     "stecker": [("X1", ["+10VREF", "AGND", "AI1+", "AI1-", "AGND", "AI2+", "AI2-", "AGND", "AI3+",
                                         "AI3-", "AGND", "AI4+", "AI4-", "AGND"]),
                                 ("X3", ["AI5+", "AI5-", "AGND", "AI6+", "AI6-", "AGND", "AO1", "AGND", "AO2", "AGND",
                                         "AO3", "AGND", "AO4", "AGND"])],
                     "versorgung": None, "kurz": "IOM-AI6O4"},
    "IOM-AXIS-2A/MO": {"artikel": "IOM-AXIS-2A/MO", "bestellnr": "N10.01.0193",
                       "bezeichnung": "Axis Controller Module (2 axis)", "breite": 80,
                       "stecker": [("X1", [p.format(n=1, a=1, b=2) for p in _AX]), ("X2", _VERS),
                                   ("X3", [p.format(n=2, a=3, b=4) for p in _AX])],
                       "versorgung": "X2", "kurz": "Dual Axis IOM-AXIS-2A"},
}


def _ordner(typ):
    return typ.replace("/", "-").replace(" ", "")


def _infos_xml(infos):
    return "".join(f'<elementInformation name="{k}" show="1">{escape(v)}</elementInformation>'
                   for k, v in infos.items() if v)


def _label(x, y, g=10):
    return (f'<dynamic_text x="{x}" y="{y}" z="2" text_width="-1" Halignment="AlignLeft" Valignment="AlignTop" '
            f'frame="false" rotation="0" keep_visual_rotation="false" text_from="ElementInfo" uuid="{{{uuid.uuid4()}}}" '
            f'{FONT.format(g=g)}><text></text><info_name>label</info_name></dynamic_text>')


def _t(x, y, s, g, rot=0):
    return (f'<text x="{x:g}" y="{y:g}" size="{g}" text="{escape(s)}" rotation="{rot}" color="#000000" '
            f'{FONT.format(g=g)}/>')


def _rect(x, y, w, h, stil="normal"):
    return (f'<rect x="{x:g}" y="{y:g}" width="{w:g}" height="{h:g}" '
            f'style="line-style:{stil};line-weight:normal;filling:none;color:black" antialias="false"/>')


def _line(x1, y1, x2, y2):
    return (f'<line x1="{x1:g}" y1="{y1:g}" x2="{x2:g}" y2="{y2:g}" length1="1.5" length2="1.5" end1="none" '
            'end2="none" style="line-style:normal;line-weight:normal;filling:none;color:black" antialias="false"/>')


def _kreis(x, y, r=3):
    return (f'<ellipse x="{x - r:g}" y="{y - r:g}" width="{2 * r:g}" height="{2 * r:g}" '
            'style="line-style:normal;line-weight:normal;filling:none;color:black" antialias="true"/>')


def _term(name, x, y, ori):
    return (f'<terminal uuid="{{{uuid.uuid4()}}}" name="{escape(name)}" x="{x:g}" y="{y:g}" orientation="{ori}" '
            'type="Generic"/>')


def _schreibe(rel, titel, teile, breite, hoehe, hx, hy, link_type="simple", infos=None, info_text=""):
    pfad = os.path.join(COMPANY, rel)
    if rel in _GESCHRIEBEN:
        return
    _GESCHRIEBEN.add(rel)
    xml = (f'<definition version="0.100.0" type="element" link_type="{link_type}" width="{breite}" height="{hoehe}" '
           f'hotspot_x="{hx}" hotspot_y="{hy}">\n<uuid uuid="{{{uuid.uuid4()}}}"/>\n'
           f'<names><name lang="de">{escape(titel)}</name><name lang="en">{escape(titel)}</name></names>\n'
           f'<elementInformations>{_infos_xml(infos or {})}<elementInformation name="label" show="1">'
           f'</elementInformation></elementInformations>\n'
           f'<informations>Planvorlagen bausteine_cnc ({escape(info_text or "aus Gerätedaten erzeugt")})</informations>\n'
           f'<description>{"".join(teile)}</description>\n</definition>\n')
    os.makedirs(os.path.dirname(pfad), exist_ok=True)
    open(pfad, "w", encoding="utf-8").write(xml)


def _ziel(rel):
    return "company://" + rel.replace("\\", "/")


# --- Symbole ---------------------------------------------------------------------------------------------------
PIN_RASTER = 20          # senkrechter Abstand der Modulpins
UEB_BREITE = 330         # Breite des Übersichtskastens (Pins links, Ziel-/Funktionsspalten rechts)
X_PIN = 100              # Anschluss (rechts am Steckerkasten, Richtung Ost)


def modul_symbol(typ, ueberschreiben=NEU):
    """Modulübersicht (Master): Kasten mit Bezeichnung/Typ, je Stecker ein Pinblock; Anschlüsse „X1:1“ … rechts am
    Block (20-px-Raster). Ohne Artikeldaten – die trägt die Stationsansicht (eine Stücklistenzeile je Modul)."""
    d = MODULTYPEN[typ]
    rel = f"{CYB}/Steuerungen/{_ordner(typ)}/{_ordner(typ)}_Uebersicht.elmt"
    if os.path.exists(os.path.join(COMPANY, rel)) and not ueberschreiben:
        return _ziel(rel)
    teile = [_label(-46, -2), _t(8, 14, d["bezeichnung"], 8), _t(8, 26, d["artikel"], 8)]
    y = 40
    for name, pins in d["stecker"]:
        teile.append(_t(44, y + 14, name, 7))
        yb = y + 20
        teile.append(_rect(40, yb, 50, PIN_RASTER * len(pins)))
        for i, p in enumerate(pins):
            py = yb + 10 + PIN_RASTER * i
            teile += [_t(43, py - 2, p, 4), _t(73, py + 6, str(i + 1), 5), _kreis(86, py), _line(89, py, X_PIN, py),
                      _term(f"{name}:{i + 1}", X_PIN, py, "e")]
        y = yb + PIN_RASTER * len(pins)
    h = y + 20
    teile.insert(0, _rect(0, 0, UEB_BREITE, h))
    _schreibe(rel, f"{typ} Modulübersicht", teile, UEB_BREITE + 60, h + 20, 50, 10, "master",
              {"description": d["bezeichnung"]}, "Pinbelegung aus der CybIO-Dokumentation")
    return _ziel(rel)


def stations_symbol(typ, hoehe=470, ueberschreiben=NEU):
    """Stationsansicht eines Moduls: Kasten in Modulbreite, Typ senkrecht darüber, Kennzeichen oben links."""
    d = MODULTYPEN[typ]
    rel = f"{CYB}/Steuerungen/{_ordner(typ)}/{_ordner(typ)}_Station.elmt"
    if os.path.exists(os.path.join(COMPANY, rel)) and not ueberschreiben:
        return _ziel(rel)
    w = d["breite"]
    teile = [_label(0, -22), _rect(0, 0, w, hoehe), _t(w / 2 + 4, -36, d["kurz"], 8, 270)]
    if d.get("bestellnr"):
        teile.append(_t(w / 2 - 18, hoehe / 2, d["bestellnr"], 5))
    _schreibe(rel, f"{typ} Stationsansicht", teile, w + 20, hoehe + 200, 10, 190, "simple",
              info_text="Stationsansicht")
    return _ziel(rel)


def geraetekasten(rel, titel, breite, hoehe, pins, gruppen=(), typtext="", infos=None, link_type="simple",
                  label="unten", ueberschreiben=NEU):
    """Allgemeiner Gerätekasten mit Pins oben/unten (Teilansicht eines Geräts, Filter, Versorgungsstecker).
    pins: [(Anschlussname, Anzeige, Bedeutung, x, "oben"|"unten")]; gruppen: [(Text, x, "oben"|"unten")] (X1, J2 …).
    Anschlüsse auf dem 10-px-Raster, Pin als kleiner Kreis auf der Kastenkante, Bedeutung senkrecht im Kasten.
    label: Lage des Kennzeichens – "unten" (unter der linken Kastenecke, Module in einer Reihe) oder "links"."""
    if os.path.exists(os.path.join(COMPANY, rel)) and not ueberschreiben:
        return _ziel(rel)
    teile = [_label(-60, 2) if label == "links" else _label(0, hoehe + 2), _rect(0, 0, breite, hoehe)]
    for name, anz, bed, x, seite in pins:
        oben = seite == "oben"
        y = 0 if oben else hoehe
        teile += [_kreis(x, y + (4 if oben else -4)), _line(x, y, x, y + (1 if oben else -1)),
                  _term(name, x, y, "n" if oben else "s"), _t(x + 3, y - 3 if oben else y + 9, anz, 6)]
        if bed:
            teile.append(_t(x + 2, (y + 9 + 3.2 * len(bed)) if oben else y - 9, bed, 4, 270))
    for text, x, seite in gruppen:
        teile.append(_t(x, 14 if seite == "oben" else hoehe - 6, text, 8))
    if typtext:
        teile.append(_t(6, hoehe - 6, typtext, 8))
    if label == "links":                     # Kasten (Größe) wie gezeichnet, damit Nachbarn nicht „überlappen“
        _schreibe(rel, titel, teile, breite + 80, hoehe + 30, 70, 10, link_type, infos)
    else:
        _schreibe(rel, titel, teile, breite + 10, hoehe + 30, 5, 10, link_type, infos)
    return _ziel(rel)


def versorgung_symbol(typ, ueberschreiben=NEU):
    """Versorgungsstecker eines Moduls (X1 bzw. X2) als Teilansicht, Pins oben im 30-px-Abstand."""
    d = MODULTYPEN[typ]
    st = d["versorgung"]
    pins = dict(d["stecker"])[st]
    rel = f"{CYB}/Steuerungen/{_ordner(typ)}/{_ordner(typ)}_Versorgung_{st}.elmt"
    return geraetekasten(rel, f"{typ} Versorgung {st}", 150, 80,
                         [(f"{st}:{i + 1}", str(i + 1), p, 30 * (i + 1), "oben") for i, p in enumerate(pins)],
                         typtext=f"{st}", ueberschreiben=ueberschreiben)


def pe_punkt(ueberschreiben=NEU):
    """PE-Anschlusspunkt (Gehäuse/Montageplatte), Leitung von oben: Kreis mit Querstrich und „PE“."""
    rel = "Allgemein/Verbindungen/PE_Anschluss_Plan/PE-Punkt_von_oben.elmt"
    if os.path.exists(os.path.join(COMPANY, rel)) and not ueberschreiben:
        return _ziel(rel)
    teile = [_term("PE", 0, 0, "n"), _line(0, 0, 0, 7), _kreis(0, 10), _line(-14, 10, 14, 10), _t(4, 24, "PE", 6)]
    _schreibe(rel, "PE-Punkt (Leitung von oben)", teile, 40, 40, 20, 10, "simple", info_text="PE-Anschlusspunkt")
    return _ziel(rel)


# --- Bausteine -------------------------------------------------------------------------------------------------
def station(b, spalte, module, y=300, artikel=True):
    """Stationsübersicht: Module nebeneinander ab Spalte `spalte` (Breite je Typ). Die Stationsansicht trägt die
    Artikeldaten (eine Stücklistenzeile je Modul). module: [{"bmk": "-A1", "typ": "IOM-CPU-6C"}, …]."""
    x = spalte_links(spalte) + 120
    teile = []
    for m in module:
        d = MODULTYPEN[m["typ"]]
        infos = ({"manufacturer": "CYBELEC S.A.", "manufacturer_reference": d.get("bestellnr") or d["artikel"],
                  "designation": d["artikel"], "description": d["bezeichnung"]} if artikel else None)
        teile.append(b.setze(stations_symbol(m["typ"]), x, y, label=m["bmk"], infos=infos))
        x += d["breite"]
    return teile


def modul_uebersicht(b, spalte, modul, belegung, y=80, einstellungen=()):
    """Modulübersicht (Master) mit Ziel, Partneradresse und Funktion je belegtem Pin.
    belegung: {"X1:1": (Ziel, Adresse, Funktion)} – Ziel z. B. "=DC/44" (Blatt der Funktion), Adresse der SPS-Partner
    ("A60.0", "E40.0" oder ""). einstellungen: Textzeilen unter den Steckern (DIP-Schalter, Knotennummer)."""
    x0 = raster(spalte_links(spalte) + 110)
    t = b.setze(modul_symbol(modul["typ"]), x0, y, label=modul["bmk"])
    for pin, (ziel, adr, fkt) in belegung.items():
        px, py = t.pos(pin)
        b.text(ziel, px + 8, py - 8, 6)
        if adr:
            b.text(adr, px + 52, py - 8, 6)
        b.text(fkt, px + 84, py - 8, 6)
    sym = S.symbol(modul_symbol(modul["typ"]))
    y_end = y + max(a.y for a in sym.anschluesse) + 40
    for i, z in enumerate(einstellungen):
        b.text(z, x0 + 40, y_end + 12 * i, 6)
    return t


def _ader(b, x, y, text):
    """Aderkennung/Querschnitt („DBU 0,75“) klein und senkrecht rechts neben der Leitung."""
    b.text(text, x + 3, y, 5, drehung=270)


def kartenversorgung(b, spalte, module, y=820, y_pfeil=640):
    """Versorgung der Module einer Station: je Modul optional Plus/Minus/PE von Potentialpfeilen an Pin 1/2/3 und/oder
    Weiterverbindung von einem Nachbarmodul (Brücke 0 V -> Pin 2, +24 V -> Pin 1, über Ecken oberhalb geführt).
    module: [{"bmk", "typ", "plus": "3L1+", "minus": "3M1", "pe": "PE" | None, "von": Index des Speisemoduls,
    "adern": ("DBU 0,75", "DBUWH 0,75", "GNYE 1,5"), "funktion": [Zeilen]}] – Module ohne Versorgungsstecker
    (Analogmodul) fehlen in der Liste."""
    teile = []
    x = raster(spalte_links(spalte) + 50)
    for i, m in enumerate(module):
        d = MODULTYPEN[m["typ"]]
        st = d["versorgung"]
        t = b.setze(versorgung_symbol(m["typ"]), x, y, anker=f"{st}:1", label=m["bmk"])
        teile.append(t)
        adern = m.get("adern", ("DBU 0,75", "DBUWH 0,75", "GNYE 1,5"))
        for k, name in enumerate((m.get("plus"), m.get("minus"), m.get("pe"))):
            if not name:
                continue
            px, py = t.pos(f"{st}:{k + 1}")
            p = b.pfeil(name, px, y_pfeil, "von_oben")
            b.draht(p, 0, t, f"{st}:{k + 1}")
            _ader(b, px, py - 40, adern[k])
        if m.get("von") is not None:
            q = teile[m["von"]]
            qs = MODULTYPEN[module[m["von"]]["typ"]]["versorgung"]
            # +24 V (Pin 4) -> Pin 1 tiefer, 0 V (Pin 3) -> Pin 2 höher: so kreuzen sich die Brücken nicht
            for q_pin, z_pin, h in ((4, 1, 40), (3, 2, 60)):
                qx, qy = q.pos(f"{qs}:{q_pin}")
                zx, _ = t.pos(f"{st}:{z_pin}")
                b.leitung(q, f"{qs}:{q_pin}", t, f"{st}:{z_pin}", ueber=[(qx, qy - h), (zx, qy - h)])
        if m.get("funktion"):
            funktion_bei(b, x + 45, m["funktion"])
        x += 160 if i % 2 == 0 else 150         # Modulraster ≈ eine Spalte, Pins bleiben auf dem 10-px-Raster
    return teile


def bedienterminal(b, geraet, x, y, ort="+BP", rahmen_xy=None, verweise=None, daten=()):
    """Bedienterminal als Geräteübersicht (Master) im Ortsrahmen des Bedienpults.
    geraet = {"bmk", "pfad", "artikel", "hersteller", "bezeichnung", "beschreibung"}; verweise = {Anschluss: Text}
    senkrecht über dem Anschluss (wo die Leitung weitergeht, z. B. "/68" für die Versorgung, "=SA/55" fürs Netz)."""
    infos = {"manufacturer": geraet.get("hersteller", ""), "manufacturer_reference": geraet.get("artikel", ""),
             "designation": geraet.get("bezeichnung", ""), "description": geraet.get("beschreibung", "")}
    t = b.setze(geraet["pfad"], x, y, anker=geraet.get("anker"), label=geraet["bmk"], infos=infos)
    if rahmen_xy:
        rahmen(b, *rahmen_xy, ort)
    for an, s in (verweise or {}).items():
        px, py = t.pos(an)
        b.text(s, px - 7, py - 12, 6, drehung=270)
    for i, z in enumerate(daten):
        b.rechts_text(z, t.pos(0)[0] - 66, y + 30 + 9 * i, "daten")
    return t


def terminal_versorgung(b, x0, terminal, filt, klemmleiste, klemmen_nr, kabel1, kabel2, plus, minus,
                        ort="+BP", y_pfeil=180, y_kl=440, y_bp=600, y_filter=660, y_term=860, pe="PE",
                        rs232=None, kabel_pe=None, x_pe=None, rahmen_bis=None, abstand_io=270, daten=()):
    """Versorgung eines Bedienterminals mit getrennten Kreisen „Panel“ (über Netzfilter) und „I/O“:
    plus/minus von oben -> Klemmen n+3/n+4 (I/O) und mit Abzweig auf n/n+1 (Panel); je Kreis ein Kabel mit PE
    (Klemme n+2 bzw. n+5 -> PE-Punkt im Pult). Panel: Klemme -> Filter J1:1/2, Filter J2:1-4 -> Terminal X1:1-4.
    I/O: Klemmen -> Terminal X2 (+24 V, 0 V). rs232: {"pins": [(Anschluss, Ader)], "name": "COM2", "pfeile": [Namen]}
    kommt über Potentialpfeile (Kabel auf dem Blatt der Gegenstelle). kabel_pe: Erdungsleitung PE -> Pult.
    Lage: x0 = erste Klemme; Terminal/Filter mit Anker auf ihre Pins, damit alle Leitungen senkrecht laufen."""
    xs = [x0, x0 + 30, x0 + 60, x0 + abstand_io, x0 + abstand_io + 30, x0 + abstand_io + 60]
    kl = klemmen(b, klemmleiste, klemmen_nr, xs, y_kl, pe=(2, 5))       # n+2, n+5: PE-Klemmen A2C 2.5 PE
    for k, nr in zip(kl, klemmen_nr):
        if nr in (klemmen_nr[2], klemmen_nr[5]):
            b.ops.append({"op": "set_info", "folio": b.folio, "element": k.ref, "key": "designation",
                          "value": "A2C 2.5 PE"})
        else:
            b.ops.append({"op": "set_info", "folio": b.folio, "element": k.ref, "key": "designation", "value": "A2C 2.5"})
    # Einspeisung: Pfeile über den I/O-Klemmen, Abzweig (Ecke) nach links auf die Panel-Klemmen
    for name, k_io, k_pan, h in ((plus, kl[3], kl[0], 0), (minus, kl[4], kl[1], 40)):
        px = k_io.pos(0)[0]
        p = b.pfeil(name, px, y_pfeil, "von_oben")
        ye = y_pfeil + 80 + h
        e = b.setze(S.ECKE, px, ye, anker="N")
        b.draht(p, 0, e, "N")
        b.draht(e, "S", k_io, 0)
        b.leitung(e, "W", k_pan, 0, ueber=[(k_pan.pos(0)[0], ye)])
    y_kab = y_kl + 70
    # Panelkreis: Filter, Terminal X1
    f = b.setze(filt["pfad"], xs[0], y_filter, anker="J1:1", label=filt["bmk"], infos=filt.get("infos"))
    b.draht(kl[0], 1, f, "J1:1")
    b.draht(kl[1], 1, f, "J1:2")
    t = b.setze(terminal["pfad"], xs[0], y_term, anker="X1:1", label=terminal["bmk"])
    for i, z in enumerate(daten):
        b.rechts_text(z, xs[0] - 34, y_term + 22 + 9 * i, "daten")
    for i in range(1, 5):
        b.draht(f, f"J2:{i}", t, f"X1:{i}")
    # I/O-Kreis
    b.draht(kl[3], 1, t, "X2:1")
    b.draht(kl[4], 1, t, "X2:4")
    # PE der Kabel auf PE-Punkte im Pult (zwischen Kabel und Gerät)
    for k in (kl[2], kl[5]):
        pp = b.setze(pe_punkt(), k.pos(1)[0], y_bp + 20, anker="PE")
        b.draht(k, 1, pp, "PE", gestrichelt=True)
    # RS-232 von der Gegenstelle (Kabel dort gezeichnet), je Ader ein Pfeil mit Aderfarbe
    if rs232:
        for (an, ader), name in zip(rs232["pins"], rs232["pfeile"]):
            px = t.pos(an)[0]
            p = b.pfeil(name, px, y_kl - 60, "von_oben", text=False)
            b.draht(p, 0, t, an)
            b.text(ader, px + 3, y_term - 34, 6)
        xa = t.pos(rs232["pins"][0][0])[0]
        b.text(rs232["name"], xa - 6, y_kl - 120, "label")
        if rs232.get("text"):
            b.text(rs232["text"], xa - 6, y_kl - 106, 6)
    # Erdung des Pults
    if kabel_pe:
        xp = x_pe or (t.pos("X2:8")[0] + 120)
        p = b.pfeil(pe, xp, y_pfeil + 120, "von_oben")
        pp = b.setze(pe_punkt(), xp, y_term, anker="PE")
        b.draht(p, 0, pp, "PE", gestrichelt=True)
    # Kabel erst nach den Leitungen (QET sucht die Adern unter der Kabellinie)
    kabel(b, kabel1, xs[0] - 12, xs[2] + 12, y_kab)
    kabel(b, kabel2, xs[3] - 12, xs[5] + 12, y_kab)
    if kabel_pe:
        kabel(b, kabel_pe, xp - 12, xp + 12, y_kab)
    if rahmen_bis:
        rahmen(b, x0 - 140, y_bp, rahmen_bis, y_term + 110, ort)
    return {"klemmen": kl, "filter": f, "terminal": t}
