"""Leser für sPlan-8-Projekte (Abacom, Endung .spl8).

Das Format ist nicht veröffentlicht. Alle Regeln hier sind am Dateiinhalt belegt, siehe FORMATBESCHREIBUNG_SPL8.md
(Abschnittsnummern stehen an den Funktionen). Die Datei ist ein unkomprimierter Delphi-Strom (little endian):
Zahlen int32/float32, Texte als int32-Zeichenzahl + UTF-16LE, Schriftnamen als int32 + Pascal-Kurzstring.
Koordinaten in 0,1 mm, Ursprung oben links, y nach unten.

    from splan8 import lese
    projekt = lese("plan.spl8")
    for blatt in projekt.blaetter:
        for o in blatt.objekte: ...

Jedes Objekt ist ein dict mit mindestens "art" und "pts" (Liste (x, y) in 0,1 mm).
"""
import math
import struct
import zlib

MAGIC = b"\x07SPLAN80"

# Objektarten (Typbyte, §3)
ARTEN = {1: "ellipse", 2: "rechteck", 3: "text", 4: "linie", 5: "polygon", 6: "gruppe", 7: "bauteil",
         8: "punkt", 9: "bild", 10: "mengentext", 11: "kontakt", 12: "bezier"}


class FormatFehler(ValueError):
    pass


class _Leser:
    def __init__(self, b, p=0):
        self.b, self.p = b, p

    def u8(self):
        v = self.b[self.p]
        self.p += 1
        return v

    def i32(self):
        v = struct.unpack_from("<i", self.b, self.p)[0]
        self.p += 4
        return v

    def f32(self):
        v = struct.unpack_from("<f", self.b, self.p)[0]
        self.p += 4
        return v

    def ws(self):
        n = self.i32()
        if n < 0 or n > 200000:
            raise FormatFehler(f"Textlänge {n} bei 0x{self.p - 4:x}")
        v = self.b[self.p:self.p + 2 * n].decode("utf-16le", "replace")
        self.p += 2 * n
        return v

    def roh(self, n):
        v = self.b[self.p:self.p + n]
        if len(v) < n:
            raise FormatFehler(f"Dateiende bei 0x{self.p:x}")
        self.p += n
        return v

    def punkte(self, n):
        return [(self.f32(), self.f32()) for _ in range(n)]


def _kopf(r):
    """§4 Gemeinsamer Objektkopf nach dem Typbyte (23 Byte): Strich (3), 11 × 0, Füllfarbe, Linienfarbe, Flag."""
    h = r.roh(3)
    r.roh(11)
    fuell = r.roh(4)
    farbe = r.roh(4)
    flag = r.u8()
    return {"strich": h[1], "breite": h[2], "fuellung": fuell.hex(), "farbe": farbe.hex(), "flag": flag}


def _textkoerper(r):
    """§5 Text: Kopf, Rahmen aus 4 Ecken, Text, Schrift, Attribute, GUID."""
    o = _kopf(r)
    o["pts"] = r.punkte(4)
    o["text"] = r.ws()
    r.i32()
    n = r.u8()
    o["schrift"] = r.roh(n).decode("latin1")
    attr = r.roh(14)
    o["schriftgrad"] = attr[5]
    o["link"] = r.ws()                                # aktive Verknüpfung (URL/Datei), sonst leer
    o["guid"] = r.roh(16).hex()
    r.roh(17)
    r.roh(5)
    p = o["pts"]
    o["drehung"] = round(math.degrees(math.atan2(p[1][1] - p[0][1], p[1][0] - p[0][0])))
    # Schrifthöhe = Rahmenhöhe senkrecht zur Leserichtung (0,1 mm)
    o["hoehe"] = math.hypot(p[3][0] - p[0][0], p[3][1] - p[0][1])
    return o


def _text_mit_wert(r):
    """§5 Freier Text: enthält er eine Variable (<PAGENO>, <PARENT_ID> …), folgt der ausgewertete Text."""
    o = _textkoerper(r)
    o["vorlage"] = o["text"]
    if "<" in o["text"]:
        o["text"] = r.ws()
    return o


def _objekt(r, tiefe=0):
    start, t = r.p, r.u8()
    art = ARTEN.get(t)
    if art is None:
        raise FormatFehler(f"unbekannter Objekttyp 0x{t:02x} bei 0x{start:x}")
    if t in (4, 12):                                  # §6 Linienzug / Bezier
        o = _kopf(r)
        n = r.i32()
        o["pts"] = r.punkte(n + 1)
        tr = r.roh(21)
        o["pfeil_anfang"], o["pfeil_ende"] = tr[5], tr[6]
    elif t == 5:                                      # §7 Polygon (geschlossen)
        o = _kopf(r)
        n = r.i32()
        o["pts"] = r.punkte(n + 1)
        r.roh(27)
    elif t == 1:                                      # §7 Ellipse/Kreis als Punktfolge
        o = _kopf(r)
        n = r.i32()
        o["pts"] = r.punkte(n + 1)
        r.roh(47)
    elif t == 2:                                      # §7 Rechteck: 4 Ecken + Schlusspunkt
        o = _kopf(r)
        o["pts"] = r.punkte(5)
        r.roh(31)
    elif t == 3:
        o = _text_mit_wert(r)
    elif t == 11:                                     # §5 Kontakt = Text + Kontaktname
        o = _textkoerper(r)
        o["name"] = r.ws()
    elif t == 8:                                      # §8 Lötpunkt
        o = _kopf(r)
        o["pts"] = r.punkte(1)
    elif t == 9:                                      # §9 Bild: Rahmen + zlib(BMP)
        o = _kopf(r)
        o["pts"] = r.punkte(4)
        d = zlib.decompressobj()
        rest = r.b[r.p:]
        daten = d.decompress(rest)
        r.p += len(rest) - len(d.unused_data)
        o["bmp"] = daten[4:]
        r.roh(4)
    elif t == 10:                                     # §5 Mengentext (mehrzeilig)
        o = _kopf(r)
        o["pts"] = r.punkte(4)
        r.u8()
        r.i32()
        n = r.u8()
        o["schrift"] = r.roh(n).decode("latin1")
        r.roh(9)
        o["zeilen"] = [r.ws() for _ in range(r.i32())]
        o["text"] = "\n".join(o["zeilen"])
        r.roh(7)
        p = o["pts"]
        o["drehung"] = round(math.degrees(math.atan2(p[1][1] - p[0][1], p[1][0] - p[0][0])))
        o["hoehe"] = math.hypot(p[3][0] - p[0][0], p[3][1] - p[0][1]) / max(1, len(o["zeilen"]))
    else:                                             # §10 Gruppe (6) / Bauteil (7)
        n = r.i32()
        o = {"kinder": [_objekt(r, tiefe + 1) for _ in range(n + 1)]}
        if t == 7:
            o["bez_text"] = _textkoerper(r)
            o["bez"] = r.ws()
            o["wert_text"] = _textkoerper(r)
            o["wert"] = o["wert_text"]["text"]
            if "<" in o["wert"]:                      # Wert mit Variable: ausgewerteter Text folgt
                o["wert"] = r.ws()
            o["schalter"] = r.roh(4).hex()
            o["symbol"] = r.ws()                      # Name des Bibliotheksbauteils
            o["bez_vorlage"] = r.ws()
            o["nummer"] = r.i32()
            r.f32()
            r.f32()
            r.i32()
            r.u8()
            o["zusatz"] = [r.ws() for _ in range(4)]
            o["ist_parent"] = r.u8()
            o["id"] = r.i32() & 0xFFFFFFFF
            o["parent"] = r.i32() & 0xFFFFFFFF
            r.u8()
    o["art"], o["typ"], o["pos"] = art, t, start
    o.setdefault("pts", [])
    return o


class Blatt:
    def __init__(self, name, breite_mm, hoehe_mm, objekte):
        self.name, self.breite_mm, self.hoehe_mm, self.objekte = name, breite_mm, hoehe_mm, objekte


class Projekt:
    def __init__(self, kopf, blaetter):
        self.kopf, self.blaetter = kopf, blaetter


def _naechstes_blatt(b, p, grenze=600):
    """§11 Blattfuß (Raster-, Druck-, Ansichtseinstellungen, Länge je Version verschieden) überspringen: der nächste
    Blattkopf beginnt mit Name (int32 + UTF-16) und Blattmaß in mm (int32, int32)."""
    for q in range(p, min(len(b) - 12, p + grenze)):
        n = struct.unpack_from("<i", b, q)[0]
        if not 0 < n <= 120:
            continue
        e = q + 4 + 2 * n
        try:
            name = b[q + 4:e].decode("utf-16le")
        except UnicodeDecodeError:
            continue
        if not all(c.isprintable() for c in name) or e + 8 > len(b):
            continue
        w, h = struct.unpack_from("<ii", b, e)
        if 50 <= w <= 3000 and 50 <= h <= 3000:
            return q
    return None


def walk(o):
    """Objekt und alle Kinder (Gruppen, Bauteile) der Reihe nach."""
    yield o
    for k in o.get("kinder", []):
        yield from walk(k)


def lese(pfad_oder_bytes):
    b = pfad_oder_bytes if isinstance(pfad_oder_bytes, (bytes, bytearray)) else open(pfad_oder_bytes, "rb").read()
    if b[:8] != MAGIC:
        raise FormatFehler("keine sPlan-8-Datei (Kennung \\x07SPLAN80 fehlt)")
    r = _Leser(b, 16)                                 # §2 Dateikopf: Kennung + 8 Byte Version
    kopf = {"u0": r.u8(), "s1": r.ws(), "s2": r.ws(), "i1": r.i32(), "i2": r.i32()}
    blaetter = []
    while True:                                       # §3 Blätter
        name = r.ws()
        w, h = r.i32(), r.i32()
        r.i32(), r.i32(), r.i32(), r.f32()            # Raster, Bildlauf x/y, Zoom
        n = r.i32()
        objekte = [_objekt(r) for _ in range(n + 1)]
        blaetter.append(Blatt(name, w, h, objekte))
        q = _naechstes_blatt(b, r.p)
        if q is None:
            break
        r.p = q
    return Projekt(kopf, blaetter)


if __name__ == "__main__":
    import collections
    import sys
    pr = lese(sys.argv[1])
    for i, bl in enumerate(pr.blaetter, 1):
        c = collections.Counter(o["art"] for o in bl.objekte)
        print(i, bl.name, f"{bl.breite_mm}x{bl.hoehe_mm}", dict(c))
