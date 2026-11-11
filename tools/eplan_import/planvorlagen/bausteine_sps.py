"""SPS-Bausteine: Netzwerkübersicht, SPS-Station (ET 200SP), Kartenversorgung, Modul- und Geräteübersichten.

Die Übersichtsblätter entstehen aus Daten, nicht aus dem Original:

* Kanäle: `kanaele_der_bereiche(module)` lässt die Projektbereiche (projekt_<x>.blaetter) trocken auf einem
  Hilfsplan laufen und zeichnet dabei jeden Aufruf von `bausteine.sps_kanal` auf (Adresse, Symbol, Funktion,
  Anlage/Blatt/Spalte des Kanals). Dazu kommt die E/A-Liste (`ea_liste`, SPS-Diagramm.xlsx) für Adressen, die
  noch kein Bereich zeichnet. `kanaltabelle` führt beides zusammen (Plan vor Liste).
* Module: Liste von dicts {bmk, art ("DI", "DQ", "F-DI", "F-DQ"), artikel, bu (BaseUnit), start (Byte),
  bit0 (erstes Bit, F-DQ: 1), titel}. Daraus Station (`sps_station`), Versorgung (`kartenversorgung`) und
  Übersicht (`modul_uebersicht`, Siemens-Makro `…_Uebersicht_…_A4`).

Verweise („=DA/30/5“, „/58/2“) werden aus der Lage der gezeichneten Teile berechnet (`verweis`), nicht
abgeschrieben. Neue Symbole (Versorgungskästen, Netzwerkkästen) schreiben `versorgung_variante` und
`netzwerk_kasten` in die Firmensammlung (nur, wenn noch nicht vorhanden).
"""
import contextlib
import importlib
import os
import re
import uuid
import zipfile
import xml.etree.ElementTree as ET
from xml.sax.saxutils import escape

from . import bausteine as B
from . import symbole as S
from .blatt import Plan, X0, SPALTE_B, spalte_links, raster

SIE = U = "company://"
S7 = {  # Übersichts-, Versorgungs- und Aufbaumakros je Artikel
    "6ES7131-6BH01-0BA0": "Siemens/SPS/6ES7131-6BH01-0BA0/6ES7131-6BH01-0BA0",
    "6ES7132-6BH00-0AA0": "Siemens/Steuerungen/6ES7132-6BH00-0AA0/6ES7132-6BH00-0AA0",
    "6ES7136-6BA01-0CA0": "Siemens/SPS/6ES7136-6BA01-0CA0/6ES7136-6BA01-0CA0",
    "6ES7136-6DC00-0CA0": "Siemens/SPS/6ES7136-6DC00-0CA0/6ES7136-6DC00-0CA0",
    "6ES7512-1SM03-0AB0": "Siemens/Steuerungen/6ES7512-1SM03-0AB0/6ES7512-1SM03-0AB0",
    "6ES7512-1SK01-0AB0": "Siemens/SPS/6ES7512-1SK01-0AB0/6ES7512-1SK01-0AB0",
    "6ES7193-6AR00-0AA0": "Siemens/SPS/6ES7193-6AR00-0AA0/6ES7193-6AR00-0AA0",
    "6AV2123-3GB32-0AW0": "Siemens/Steuerungen/6AV2123-3GB32-0AW0/6AV2123-3GB32-0AW0",
}
UEBERSICHT = {"DI": "_Uebersicht_DI0-0VDC_A4.elmt", "F-DI": "_Uebersicht_DI0-0VDC_A4.elmt",
              "DQ": "_Uebersicht_DQ0-0VDC_A4.elmt", "F-DQ": "_Uebersicht_DQ-PP0-0VDC_A4.elmt"}
KANAELE = {"DI": 16, "DQ": 16, "F-DI": 8, "F-DQ": 8}
KANAL_SYMBOL = re.compile(r"_(DI|DQ|DI-AUX|DI-VS|DQ-PP-M|DQ-PP|DQ-AUX)_[A-Z]\d\.elmt$|_Kanal")


def pfad(artikel, endung):
    return U + S7[artikel] + endung


# --------------------------------------------------------------------------------------------------- Verweise
def spalte(x):
    """EPLAN-Spalte (0…9) eines Punkts auf dem Blatt."""
    return max(0, min(9, int((x - X0) // SPALTE_B)))


def verweis(ziel_anlage, ziel_blatt, x, von_anlage):
    """„=DA/30/5“ bzw. „/58/2“, wenn das Ziel in derselben Anlage liegt (wie EPLAN)."""
    vor = "" if ziel_anlage == von_anlage else f"={ziel_anlage}"
    return f"{vor}/{ziel_blatt}/{spalte(x)}"


def ziel_kurz(symbol, ort):
    """Symbolische Adresse ohne den eigenen Einbauort: „=DA+CA-FC311“ auf einem +CA-Blatt -> „=DA-FC311“."""
    return symbol.replace(f"+{ort.lstrip('+')}-", "-") if symbol else ""


# --------------------------------------------------------------------------------------------------- Kanaldaten
@contextlib.contextmanager
def _aufzeichnen(liste):
    """bausteine.sps_kanal vorübergehend ersetzen: jeden Kanal mit Blattdaten in `liste` schreiben."""
    orig = B.sps_kanal

    def sps_kanal(b, pfad_, x, y, kanal, anker):
        t = orig(b, pfad_, x, y, kanal, anker)
        if kanal.get("adresse"):
            liste.append({"adresse": kanal["adresse"], "bmk": kanal.get("bmk", ""),
                          "symbol": kanal.get("symbol", ""), "funktion": kanal.get("funktion", ""),
                          "anlage": b.anlage, "blatt": b.nummer, "x": x, "quelle": "Plan"})
        return t
    B.sps_kanal = sps_kanal
    try:
        yield
    finally:
        B.sps_kanal = orig


def _aus_ops(plan, liste):
    """Kanäle, die nicht über sps_kanal entstanden sind, aus den Operationen lesen (Kanalmakro + Felder)."""
    bekannt = {k["adresse"] for k in liste}
    for b in plan.blaetter:
        teile, felder = {}, {}
        for op in b.ops:
            if op["op"] == "add_element" and KANAL_SYMBOL.search(op["path"]):
                teile[op["id"]] = op
            elif op["op"] == "set_element_text" and op.get("property") == "text":
                felder.setdefault(op["element"].lstrip("$"), {})[op["index"]] = op["value"]
            elif op["op"] == "set_label":
                felder.setdefault(op["element"].lstrip("$"), {})["label"] = op["label"]
        for ident, op in teile.items():
            f = felder.get(ident, {})
            adr = f.get(1, "")
            if re.fullmatch(r"[EA]\d+\.[0-7]", adr) and adr not in bekannt:
                bekannt.add(adr)
                liste.append({"adresse": adr, "bmk": f.get("label", ""), "symbol": f.get(2, ""),
                              "funktion": f.get(3, ""), "anlage": b.anlage, "blatt": b.nummer, "x": op["x"],
                              "quelle": "Plan"})


def kanaele_der_bereiche(module, ordner):
    """Trockenlauf der Bereichsmodule (Liste von Modulnamen, z. B. ["projekt_ba", …]) auf einem Hilfsplan.
    Rückgabe (Kanalliste, Fehlerliste). Ein fehlerhafter Bereich bricht den Lauf nicht ab."""
    liste, fehler = [], []
    for name in module:
        plan = Plan("Trockenlauf", ordner, "x")
        teil = []
        try:
            m = importlib.import_module(name)
            with _aufzeichnen(teil):
                m.blaetter(plan)
        except Exception as e:                                 # noqa: BLE001 – Bereich eines anderen Agenten
            fehler.append(f"{name}: {type(e).__name__}: {e}")
        _aus_ops(plan, teil)
        liste += teil
    return liste, fehler


def _xlsx_zeilen(datei):
    """Erste Tabelle einer .xlsx als Zeilenlisten (ohne openpyxl)."""
    ns = {"m": "http://schemas.openxmlformats.org/spreadsheetml/2006/main"}
    z = zipfile.ZipFile(datei)
    strings = []
    if "xl/sharedStrings.xml" in z.namelist():
        for si in ET.fromstring(z.read("xl/sharedStrings.xml")).findall("m:si", ns):
            strings.append("".join(t.text or "" for t in si.iter(f"{{{ns['m']}}}t")))
    blatt = sorted(n for n in z.namelist() if n.startswith("xl/worksheets/sheet"))[0]
    zeilen = []
    for row in ET.fromstring(z.read(blatt)).iter(f"{{{ns['m']}}}row"):
        werte = {}
        for c in row.findall("m:c", ns):
            spalte_ = re.match(r"[A-Z]+", c.get("r")).group(0)
            v = c.find("m:v", ns)
            if c.get("t") == "s" and v is not None:
                werte[spalte_] = strings[int(v.text)]
            elif c.get("t") == "inlineStr":
                werte[spalte_] = "".join(t.text or "" for t in c.iter(f"{{{ns['m']}}}t"))
            elif v is not None:
                werte[spalte_] = v.text
        zeilen.append(werte)
    return zeilen


def ea_liste(datei):
    """E/A-Liste (SPS-Diagramm.xlsx): Spalten SPS-Adresse, Symbolische Adresse, Funktionstext."""
    kopf, liste = None, []
    for z in _xlsx_zeilen(datei):
        if kopf is None:
            if any(str(v).startswith("SPS-Adresse") for v in z.values()):
                kopf = {str(v).split()[0]: k for k, v in z.items()}
            continue
        adr = (z.get(kopf.get("SPS-Adresse"), "") or "").strip()
        sym = (z.get(kopf.get("Symbollische", kopf.get("Symbolische")), "") or "").strip()
        fkt = (z.get(kopf.get("Funktionstext"), "") or "").strip()
        if re.fullmatch(r"[EA]\d+\.[0-7]", adr) and (sym or fkt):
            liste.append({"adresse": adr, "symbol": sym, "funktion": fkt, "quelle": "E/A-Liste"})
    return liste


def _anlage(symbol):
    m = re.match(r"=([A-Z]+)", symbol or "")
    return m.group(1) if m else ""


def kanaltabelle(plan_kanaele, ea=(), verworfen=None):
    """Adresse -> Kanaldaten. Gezeichnete Kanäle haben Vorrang. Die E/A-Liste füllt nur Lücken von Anlagen,
    die noch kein Bereich zeichnet: ist eine Anlage gezeichnet, gilt ihr Planstand – was die (ältere) Liste
    dort zusätzlich nennt, wird nicht übernommen, sondern in `verworfen` gemeldet."""
    gezeichnet = {k["anlage"] for k in plan_kanaele}
    t = {}
    for k in plan_kanaele:
        t[k["adresse"]] = k
    for k in ea:
        if not k.get("symbol") or k["adresse"] in t:   # Zeilen ohne Symbol (z. B. „Kartenversorgung“) überspringen
            continue
        if _anlage(k["symbol"]) in gezeichnet:
            if verworfen is not None:
                verworfen.append(k)
            continue
        t[k["adresse"]] = k
    return t


def adressen(modul):
    """Adressen eines Moduls in Kanalreihenfolge (F-DQ zählt ab Bit 1: A120.1 = DQ0)."""
    a = "E" if modul["art"] in ("DI", "F-DI") else "A"
    n0 = modul["start"] * 8 + modul.get("bit0", 0)
    return [f"{a}{(n0 + i) // 8}.{(n0 + i) % 8}" for i in range(KANAELE[modul["art"]])]


# --------------------------------------------------------------------------------------------------- Symbole
def versorgung_variante(quelle, rel, ersetzen, artikel, beschreibung="", zusatz="", ueberschreiben=False):
    """Versorgungskasten eines anderen Moduls aus einem vorhandenen „…_Versorgung_Original“ ableiten:
    Texte ersetzen ({alt: neu}), Artikeldaten setzen, optional zusätzliche Grafik/Anschlüsse (`zusatz`, XML)
    einfügen. Schreibt nur, wenn die Datei fehlt. Rückgabe company://-Pfad."""
    ziel = U + rel.replace("\\", "/")
    datei = S.datei(ziel)
    if os.path.exists(datei) and not ueberschreiben:
        return ziel
    s = open(S.datei(quelle), encoding="utf-8").read()
    for alt, neu in ersetzen.items():
        s = s.replace(f'text="{alt}"', f'text="{escape(neu)}"')
    alt_art = re.search(r'name="manufacturer_reference" show="1">([^<]*)<', s).group(1)
    s = s.replace(alt_art, artikel)
    if beschreibung:
        s = re.sub(r'(name="description" show="1">)[^<]*', r"\g<1>" + escape(beschreibung), s)
    s = re.sub(r'<uuid uuid="\{[^}]*\}"', f'<uuid uuid="{{{uuid.uuid4()}}}"', s, count=1)
    s = re.sub(r'(<name lang="(?:de|en)">)[^<]*', r"\g<1>" + artikel + " Versorgung (Plan)", s)
    s = re.sub(r"<informations>[^<]*</informations>",
               "<informations>Planvorlagen bausteine_sps.versorgung_variante</informations>", s)
    if zusatz:
        s = s.replace("</description>", zusatz + "</description>")
    os.makedirs(os.path.dirname(datei), exist_ok=True)
    open(datei, "w", encoding="utf-8").write(s)
    return ziel


def anschluss_xml(name, x, y, texte=(), richtung="n"):
    """Zusätzlicher Anschluss im Stil der Versorgungskästen (Stummel + Kreis + Beschriftung)."""
    t = [f'<line x1="{x}" y1="0" x2="{x}" y2="11.25" end1="none" end2="none" length1="1.5" length2="1.5" '
         'antialias="false" style="line-style:normal;line-weight:normal;filling:none;color:black"/>',
         f'<ellipse x="{x - 3.75}" y="11.25" width="7.5" height="7.5" antialias="true" '
         'style="line-style:normal;line-weight:normal;filling:none;color:black"/>']
    for i, (s, g) in enumerate(texte):
        t.append(f'<text x="{x + 6.73}" y="{18.6 + 7.8 * i}" text="{escape(s)}" rotation="0" '
                 f'font="Liberation Sans,{g},-1,5,50,0,0,0,0,0,Regular" color="#000000"/>')
    t.append(f'<terminal name="{escape(name)}" orientation="{richtung}" type="Generic" uuid="{{{uuid.uuid4()}}}" '
             f'x="{x}" y="{y}"/>')
    return "".join(t)


def netzwerk_kasten(rel, zeilen, ports, seite="e", hersteller="", artikel="", beschreibung="", breite=280,
                    ueberschreiben=False):
    """Netzwerkteilnehmer als Kasten mit Ports (Ethernet/CAN) für Übersichtsblätter. Die Herstellermakros
    („…_Einpolig“) haben keine Anschlüsse; hier liegt je Port ein Anschluss mit Kreis und Name.
    zeilen: [(Text, Größe, x, y)]; ports: [Name]; seite "e" (rechts, 40 px Raster) oder "n" (oben, 60 px)."""
    ziel = U + rel.replace("\\", "/")
    datei = S.datei(ziel)
    if os.path.exists(datei) and not ueberschreiben:
        return ziel
    n = len(ports)
    hoehe = max(100, 40 * n + 40) if seite == "e" else 120
    if seite == "n":
        breite = max(breite, 60 * n + 60)
    st = 'style="line-style:normal;line-weight:normal;filling:none;color:black"'
    t = [f'<rect x="0" y="0" width="{breite}" height="{hoehe}" antialias="false" {st}/>']
    for s, g, x, y in zeilen:
        t.append(f'<text x="{x}" y="{y}" text="{escape(s)}" rotation="0" color="#000000" '
                 f'font="Liberation Sans,{g},-1,5,50,0,0,0,0,0,Regular"/>')
    for i, name in enumerate(ports):
        if seite == "e":
            x, y = breite, 30 + 40 * i
            t.append(f'<ellipse x="{x - 20}" y="{y - 4}" width="8" height="8" antialias="true" {st}/>')
            t.append(f'<line x1="{x - 12}" y1="{y}" x2="{x}" y2="{y}" end1="none" end2="none" length1="1.5" '
                     f'length2="1.5" antialias="false" {st}/>')
            t.append(f'<text x="{x - 22 - 4.2 * len(name)}" y="{y + 3}" text="{escape(name)}" rotation="0" '
                     'color="#000000" font="Liberation Sans,6,-1,5,50,0,0,0,0,0,Regular"/>')
            t.append(f'<text x="{x - 20}" y="{y + 13}" text="PN" rotation="0" color="#000000" '
                     'font="Liberation Sans,5,-1,5,50,0,0,0,0,0,Regular"/>')
        else:
            x, y = 60 * (i + 1), 0
            t.append(f'<ellipse x="{x - 4}" y="12" width="8" height="8" antialias="true" {st}/>')
            t.append(f'<line x1="{x}" y1="0" x2="{x}" y2="12" end1="none" end2="none" length1="1.5" '
                     f'length2="1.5" antialias="false" {st}/>')
            t.append(f'<text x="{x - 4.2 * len(name) / 2}" y="30" text="{escape(name)}" rotation="0" '
                     'color="#000000" font="Liberation Sans,6,-1,5,50,0,0,0,0,0,Regular"/>')
        t.append(f'<terminal name="{escape(name)}" orientation="{seite}" type="Generic" uuid="{{{uuid.uuid4()}}}" '
                 f'x="{x}" y="{y}"/>')
    infos = "".join(f'<elementInformation name="{k}" show="1">{escape(v)}</elementInformation>'
                    for k, v in (("manufacturer", hersteller), ("manufacturer_reference", artikel),
                                 ("designation", artikel), ("description", beschreibung)) if v)
    label = ('<dynamic_text x="-130" y="-2" z="2" text_width="120" Halignment="AlignRight" Valignment="AlignTop" '
             'frame="false" rotation="0" keep_visual_rotation="false" text_from="ElementInfo" '
             f'uuid="{{{uuid.uuid4()}}}" font="Liberation Sans,10,-1,5,50,0,0,0,0,0,Regular">'
             '<text></text><info_name>label</info_name></dynamic_text>') if seite == "e" else (
             '<dynamic_text x="0" y="' + str(hoehe + 2) + '" z="2" text_width="' + str(breite) + '" '
             'Halignment="AlignHCenter" Valignment="AlignTop" frame="false" rotation="0" keep_visual_rotation="false" '
             f'text_from="ElementInfo" uuid="{{{uuid.uuid4()}}}" font="Liberation Sans,10,-1,5,50,0,0,0,0,0,Regular">'
             '<text></text><info_name>label</info_name></dynamic_text>')
    titel = zeilen[0][0] if zeilen else artikel
    rand = 140 if seite == "e" else 10           # links Platz für das Kennzeichen (nur bei Ports rechts)
    xml = (f'<definition version="0.100.0" type="element" link_type="simple" width="{breite + rand + 10}" '
           f'height="{hoehe + 40}" hotspot_x="{rand}" hotspot_y="20">\n<uuid uuid="{{{uuid.uuid4()}}}"/>\n'
           f'<names><name lang="de">{escape(titel)} Netzwerk</name><name lang="en">{escape(titel)} network</name>'
           f'</names>\n<elementInformations>{infos}<elementInformation name="label" show="1"></elementInformation>'
           '</elementInformations>\n<informations>Planvorlagen bausteine_sps.netzwerk_kasten</informations>\n'
           f'<description>{label}{"".join(t)}</description>\n</definition>\n')
    os.makedirs(os.path.dirname(datei), exist_ok=True)
    open(datei, "w", encoding="utf-8").write(xml)
    return ziel


def _infos(artikel, hersteller="Siemens", bezeichnung=""):
    return {"manufacturer": hersteller, "manufacturer_reference": artikel, "designation": artikel,
            "description": bezeichnung}


# --------------------------------------------------------------------------------------------------- Netzwerk
def netzwerk(b, teilnehmer, verbindungen):
    """Netzwerkübersicht. teilnehmer: {key: {pfad, x, y, anker, label, infos, texte:[(s, dx, dy, größe)]}};
    verbindungen: [((key, port), (key, port), [(x, y) Eckpunkte])]. Rückgabe {key: Teil}."""
    t = {}
    for k, d in teilnehmer.items():
        t[k] = b.setze(d["pfad"], d["x"], d["y"], anker=d.get("anker"), label=d.get("label", ""),
                       infos=d.get("infos"), label_xy=d.get("label_xy"))
        for s, dx, dy, g in d.get("texte", ()):
            b.text(s, d["x"] + dx, d["y"] + dy, g)
    for (ka, pa), (kb, pb), ueber in verbindungen:
        # Die Ecke (Plan) hat W/E 0,1 px unter N: eine Ecke, deren waagerechter Nachbar ein Geräteanschluss ist,
        # um 0,1 anheben, damit die Leitung gerade läuft (zwischen zwei Ecken bleibt sie auf dem Raster)
        punkte = [t[ka].pos(pa)] + list(ueber) + [t[kb].pos(pb)]
        neu = []
        for i, (x, y) in enumerate(ueber, start=1):
            waag = [j for j in (i - 1, i + 1) if abs(punkte[j][1] - y) < 0.5]
            an_geraet = any(j in (0, len(punkte) - 1) for j in waag)
            neu.append((x, y - 0.1 if an_geraet else y))
        b.leitung(t[ka], pa, t[kb], pb, ueber=neu)
    return t


# --------------------------------------------------------------------------------------------------- Station
STATION_MODUL = 40            # Raster der Module (Modul 15 mm = 37,5 px, auf das 10-px-Raster gelegt)


def sps_station(b, x, y, cpu, module, titel_y=None):
    """SPS-Station von vorn: CPU mit Busadapter (Aufbaumakro A6), Module mit BaseUnit (B6), Servermodul (CPU-B6).
    Über jedem Modul der Titel senkrecht (Digital Input …). Rückgabe {bmk: (x Mitte)} für Verweise."""
    lage = {}
    unten = titel_y or y - 34                  # senkrechte Titel enden über den Kennzeichen
    b.setze(pfad(cpu["artikel"], "_Aufbau_A6.elmt"), x, y, label=cpu["bmk"], infos=_infos(cpu["artikel"]))
    lage[cpu["bmk"]] = x + 120
    if cpu.get("titel"):
        b.text(cpu["titel"], x + 120, unten, 10, drehung=270)
    xm = x + 250
    for m in module:
        b.setze(pfad(m["artikel"], "_Aufbau_B6.elmt"), xm, y, label=m["bmk"], infos=_infos(m["artikel"]))
        lage[m["bmk"]] = xm + STATION_MODUL / 2
        b.text(m["titel"], xm + 12, unten, 10, drehung=270)
        xm += STATION_MODUL
    b.setze(pfad(cpu["artikel"], "_Aufbau_B6.elmt"), xm, y, infos=_infos("6ES7193-6PA00-0AA0"))   # Servermodul
    return lage


# --------------------------------------------------------------------------------------------------- Versorgung
def kartenversorgung(b, x, karte, potentiale, y_pfeil=600, y_karte=800, funktion=None, pe=None):
    """Versorgungskasten einer Karte (Symbol karte["pfad"], Anschluss anschluss[0] bei x) mit Potentialpfeilen von
    oben: potentiale = [(Anschluss, Potentialname)]. pe = (Anschluss, Text) für FE/PE gestrichelt.
    Rückgabe Teil."""
    erster = potentiale[0][0]
    k = b.setze(karte["pfad"], x, y_karte, anker=erster, label=karte["bmk"], infos=karte.get("infos"))
    for k_info, wert in (karte.get("felder_info") or {}).items():
        b.ops.append({"op": "set_info", "folio": b.folio, "element": k.ref, "key": k_info, "value": wert})
    for an, name in potentiale:
        p = b.pfeil(name, k.pos(an)[0], y_pfeil, "von_oben")
        b.draht(p, 0, k, an)
    if pe:
        an, txt = pe
        xp = k.pos(an)[0]
        p = b.pfeil("PE", xp, y_karte - 120, "von_oben")
        b.draht(p, 0, k, an, gestrichelt=True)
        if txt:
            b.text(txt, xp + 4, y_karte - 70, 5)
    if funktion:
        B.funktion_bei(b, x + 40, funktion)
    return k


# --------------------------------------------------------------------------------------------------- Übersichten
def _felder(pfad_):
    """Benutzerfelder des Übersichtsmakros: [(Index, Anschlussname, Spalte 0=Ziel/1=Funktion)]."""
    d = ET.parse(S.datei(pfad_)).getroot()
    ans = [(a.get("name"), float(a.get("y"))) for a in d.iter("terminal") if a.get("y") is not None]
    out = []
    for i, t in enumerate(d.iter("dynamic_text")):
        if t.get("text_from") != "UserText":
            continue
        ty = float(t.get("y"))
        name = min(ans, key=lambda a: abs(a[1] - (ty + 7.59)))
        if abs(name[1] - (ty + 7.59)) > 5:
            name = min(ans, key=lambda a: abs(a[1] - (ty + 9)))
        out.append((i, name[0], 0 if float(t.get("x")) < 225 else 1))
    return out


# Feldlage (px ab Anschluss): Adresse, Verweis, Ziel (x, Breite), Funktion (x, Breite)
FELDER_BREIT = {"adresse": 14, "verweis": 46, "ziel": (100, 90), "funktion": (195, 230)}
KOPF_LABEL = (-120, -90)     # Kennzeichen über den Kopfzeilen (Verweis auf die Station, Modulart)
FELDER_SCHMAL = {"adresse": 14, "verweis": 42, "ziel": (80, 60), "funktion": (142, 104)}


def _feld(b, t, index, x=None, breite=None, text=None):
    for k, v in (("x", x), ("width", breite), ("text", text)):
        if v is not None:
            b.ops.append({"op": "set_element_text", "folio": b.folio, "element": t.ref, "index": index,
                          "property": k, "value": str(v)})


def modul_uebersicht(b, x, y, modul, kanaele, versorgung=None, lage=FELDER_BREIT, kopf=()):
    """Siemens-Übersichtsmakro eines E/A-Moduls (Anschluss 1 bei x/y) mit den Kanaldaten je Klemme:
    Adresse (Text rechts am Anschluss), Verweis auf den Kanal (=DA/30/5), Ziel (=DA-QA301) und Funktionstext.
    kanaele: {Adresse: {symbol, funktion, anlage, blatt, x}}; versorgung: (Verweis, Funktionstext) für L+/M.
    kopf: Zeilen unter dem Kennzeichen (Verweis auf die Station, Modulart)."""
    p = pfad(modul["artikel"], UEBERSICHT[modul["art"]])
    infos = _infos(modul["artikel"], bezeichnung=modul.get("titel", ""))
    t = b.setze(p, x, y, anker="1", label=modul["bmk"], infos=infos, label_xy=KOPF_LABEL)
    for i, z in enumerate(kopf):
        b.text(z, x - 120, y - 75 + 9 * i, 6)
    # Kanal n -> Anschluss n+1 (F-DQ: Anschlüsse 1…8 = DQ-PP0…7)
    zeile = {str(i + 1): a for i, a in enumerate(adressen(modul))}
    for index, an, sp in _felder(p):
        if an in zeile:
            k = kanaele.get(zeile[an])
            wert = (ziel_kurz(k["symbol"], b.ort) if sp == 0 else k["funktion"]) if k else ""
        elif an in ("L+", "M") and versorgung:
            wert = "" if sp == 0 else versorgung[1]
        else:
            wert = ""
        fx, fb = lage["ziel"] if sp == 0 else lage["funktion"]
        _feld(b, t, index, x=fx, breite=fb, text=wert)
    for an, adr in zeile.items():
        ay = t.pos(an)[1]
        b.text(adr, x + lage["adresse"], ay - 9, 6)
        k = kanaele.get(adr)
        if k and k.get("blatt"):
            b.text(verweis(k["anlage"], k["blatt"], k["x"], b.anlage), x + lage["verweis"], ay - 7.5, 5)
    if versorgung:
        for an in ("L+", "M"):
            b.text(versorgung[0], x + lage["verweis"], t.pos(an)[1] - 7.5, 5)
    return t


def geraet_uebersicht(b, pfad_, x, y, anker, label, infos, zeilen, lage=FELDER_BREIT, kopf=(), feld_x=None):
    """Geräteübersicht (CPU, Busadapter, Panel): zeilen = {Anschluss: (Verweis, Ziel, Funktion)}.
    Makros mit nur einem Feld je Anschluss (Panel) bekommen dort die Funktion; der Verweis steht als Text."""
    t = b.setze(pfad_, x, y, anker=anker, label=label, infos=infos, label_xy=KOPF_LABEL)
    for i, z in enumerate(kopf):
        b.text(z, x - 120, y - 75 + 9 * i, 6)
    felder = _felder(pfad_)
    je_an = {}
    for index, an, sp in felder:
        je_an.setdefault(an, []).append((index, sp))
    for an, liste in je_an.items():
        vw, ziel, fkt = zeilen.get(an, ("", "", ""))
        einzel = len(liste) == 1
        for index, sp in liste:
            if einzel:
                _feld(b, t, index, x=feld_x[1] if feld_x else None, breite=180, text=fkt)
            else:
                fx, fb = lage["ziel"] if sp == 0 else lage["funktion"]
                _feld(b, t, index, x=fx, breite=fb, text=ziel if sp == 0 else fkt)
    for an, (vw, ziel, fkt) in zeilen.items():
        if vw:
            xv = x + (feld_x[0] if feld_x else lage["verweis"])
            b.text(vw, xv, t.pos(an)[1] - 7.5, 5)
    return t
