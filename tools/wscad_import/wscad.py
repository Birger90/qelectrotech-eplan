"""WSCAD-Projekt lesen (ELECTRIX/SUITE ab 6.x, belegt an WSCAD ELECTRIX 2025).

Ein WSCAD-Projekt ist ein Ordner:
- Database/WSCAD6.db (SQLite): Seiten (Page), Symbole mit BMK und Lage (Symbol), Anschlüsse (Pin),
  Verbindungen Symbol/Anschluss -> Symbol/Anschluss (Connection), Potentiale, Kabeladern (Center),
  Texte (String, Data) – die logische Sicht, die WSCAD für Auswertungen führt;
- je Seite eine Datei <Struktur>_<Name>.<Nr>.ws<Art> (wsELD = Schaltplan, wsFLD Fluid, wsTEC Klemmenplan …).

Seitendatei (FORMATBESCHREIBUNG_WSCAD.md): 4 Byte Dateikennung, 4 Byte Version, 4 Byte Zähler, 5 Abschnitte
à 19 Byte (benutzt, Index, Art, Versatz, Länge, Zeit). Art 2 (Seite) und 5 (Info) sind gepackt: 4 Byte Version,
4 Byte Länge, Schlüssellänge, IV-Länge, Schlüssel, IV, 3 Prüfbytes, dahinter AES-CBC (PKCS7) über GZip. Schlüssel
und IV stehen im Abschnitt selbst. Ausgepackt: 4 Byte Version + UTF-8-XML <WS><Solution><Project><File>.
"""
import gzip
import os
import sqlite3
import struct
import xml.etree.ElementTree as ET

ABSCHNITT_SEITE, ABSCHNITT_INFO = 2, 5


def _aes_cbc_entschluesseln(schluessel, iv, daten):
    try:
        from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
        d = Cipher(algorithms.AES(schluessel), modes.CBC(iv)).decryptor()
        roh = d.update(daten) + d.finalize()
    except ImportError:                                  # pycryptodome als Ersatz
        from Crypto.Cipher import AES
        roh = AES.new(schluessel, AES.MODE_CBC, iv).decrypt(daten)
    n = roh[-1]
    if not 1 <= n <= 16 or roh[-n:] != bytes([n]) * n:
        raise ValueError("Auffüllung (PKCS7) ungültig")
    return roh[:-n]


def auspacken(abschnitt):
    """Gepackter Abschnitt -> Bytes (4 Byte Version + Inhalt)."""
    kl, il = abschnitt[8], abschnitt[9]
    schluessel = abschnitt[10:10 + kl]
    iv = abschnitt[10 + kl:10 + kl + il]
    beginn = 10 + kl + il + 3
    return gzip.decompress(_aes_cbc_entschluesseln(schluessel, iv, abschnitt[beginn:]))


def abschnitte(datei):
    """{Art: Bytes} der benutzten Abschnitte einer Seitendatei, gepackte schon ausgepackt."""
    with open(datei, "rb") as f:
        d = f.read()
    aus = {}
    for i in range(5):
        e = d[12 + 19 * i:12 + 19 * (i + 1)]
        if len(e) < 19 or not e[0]:
            continue
        art = e[2]
        versatz, laenge = struct.unpack("<II", e[3:11])
        roh = d[versatz:versatz + laenge]
        if art in (ABSCHNITT_SEITE, ABSCHNITT_INFO):
            roh = auspacken(roh)
        aus[art] = roh
    return aus


def seite_xml(datei):
    """<File>-Element der Seite (Header, Template, SL-Symbole, CN-Verbindungen, freie Grafik)."""
    roh = abschnitte(datei)[ABSCHNITT_SEITE]
    return ET.fromstring(roh[4:]).find(".//File")


def info_xml(datei):
    a = abschnitte(datei)
    return ET.fromstring(a[ABSCHNITT_INFO][4:]).find(".//File") if ABSCHNITT_INFO in a else None


def text_de(p, sprache="DE"):
    """Text einer FTexte-Eigenschaft <P><T id="DE">…</T></P>: gewünschte Sprache, sonst DE, sonst die erste."""
    if p is None:
        return ""
    ts = {t.get("id"): (t.text or "") for t in p.findall("T")}
    if not ts:
        return p.get("val", "") or ""
    return ts.get(sprache) or ts.get("DE") or next(iter(ts.values()))


class Projekt:
    """Projektordner: Datenbank und Seitendateien."""

    def __init__(self, ordner):
        self.ordner = ordner
        db = os.path.join(ordner, "Database", "WSCAD6.db")
        if not os.path.exists(db):
            raise FileNotFoundError(f"kein WSCAD-Projekt (Database/WSCAD6.db fehlt): {ordner}")
        self.db = sqlite3.connect(f"file:{db}?mode=ro", uri=True)
        dateien = {n.upper(): n for n in os.listdir(ordner)}
        self.seiten = []
        for pid, name, art, ref, w, h, asp_f, asp_l in self.db.execute(
                "SELECT PageID, FileName, FileType, Ref, Width, Height, AspF, AspL FROM Page"):
            pfad = dateien.get((name or "").upper())
            self.seiten.append({"id": pid, "datei": os.path.join(ordner, pfad) if pfad else None, "name": name,
                                "art": art, "ref": ref, "breite": w, "hoehe": h, "anlage": asp_f or "",
                                "ort": asp_l or ""})
        # Reihenfolge: Seitengruppen (Struktur + Seitenart) in der Reihenfolge ihrer ersten PageID (so wie WSCAD
        # sie angelegt hat, = Reihenfolge im PDF-Export), darin nach Seitennummer
        erste = {}
        for s in sorted(self.seiten, key=lambda s: s["id"]):
            erste.setdefault(_seitenfolge(s["name"])[0], s["id"])
        self.seiten.sort(key=lambda s: (erste[_seitenfolge(s["name"])[0]], _seitenfolge(s["name"])[1]))

    def schaltplanseiten(self):
        return [s for s in self.seiten if (s["art"] or "").lower() == "wseld" and s["datei"]]

    def symbole(self):
        """SymbolID -> Zeile (BMK, PageID, x, y, ItemType)."""
        return {r[0]: {"id": r[0], "eltern": r[1], "typ": r[2], "bmk": r[3], "seite": r[4], "x": r[5], "y": r[6]}
                for r in self.db.execute("SELECT SymbolID, ParentID, ItemType, BMK, PageID, x, y FROM Symbol")}

    def anschluesse(self):
        """(SymbolID, PinID) -> (Name, Nummer, x, y) relativ zum Symbol."""
        return {(r[0], r[1]): (r[2] or "", r[3] or "", r[4], r[5])
                for r in self.db.execute("SELECT SymbolID, PinID, Name, Number, x, y FROM Pin")}

    def verbindungen(self):
        spalten = ("SymbolID1", "Pin1", "SymbolID2", "Pin2", "PageID", "Colorcode", "Dimension", "Potential",
                   "WireName", "WireType", "CableID")
        return [dict(zip(spalten, r)) for r in self.db.execute(f"SELECT {', '.join(spalten)} FROM Connection")]

    def artikel(self):
        """SymbolID -> Artikelnummer(n) aus der Datentabelle (Typ 57 = Artikel, 63 = Hersteller)."""
        aus = {}
        for sid, typ, wert in self.db.execute("SELECT ID, Type, Data FROM Data WHERE Type IN (57, 63)"):
            aus.setdefault(sid, {})[{57: "artikel", 63: "hersteller"}[typ]] = wert
        return aus


def artikel_hersteller(ordner):
    """article.db des Projekts: Bestellnummer -> Herstellername (article.artManufacturer = adress.adrAutoValue)."""
    db = os.path.join(ordner, "article.db")
    if not os.path.isfile(db):
        return {}
    try:
        c = sqlite3.connect(f"file:{db}?mode=ro", uri=True)
        aus = {nr: name for nr, name in c.execute(
            "SELECT a.artNumber, d.adrName FROM article a JOIN adress d ON a.artManufacturer = d.adrAutoValue")
            if nr and name}
        c.close()
        return aus
    except sqlite3.Error:
        return {}


def _seitenfolge(name):
    """=A1+ST_PLAN.0003.WSELD -> (=A1+ST_PLAN, 3): Gruppen wie im Dateinamen, darin nach Nummer."""
    stamm, _, rest = (name or "").rpartition(".")
    basis, _, nr = stamm.rpartition(".")
    return (basis.upper(), int(nr) if nr.isdigit() else 0)
