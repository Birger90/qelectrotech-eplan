"""DWG lesen (AutoCAD R2004–R2010, AC1018/AC1024) und als DXF schreiben, ohne Fremdsoftware.

Nach der öffentlichen Formatbeschreibung der Open Design Alliance („Open Design Specification for .dwg files“):
Dateikopf (mit festem Zufallsstrom verschleiert), Seitenverzeichnis, Abschnittsverzeichnis, LZ77-artig gepackte
Datenseiten, Objektverzeichnis (AcDb:Handles) und die bitweise codierten Objekte (AcDb:AcDbObjects). Gelesen wird,
was ein Symbol braucht: Linien, Kreise, Bögen, Ellipsen, Polylinien, Volltonflächen, Texte, Attribute und Blöcke
(Einfügungen). Alles andere wird gezählt und übersprungen.

    python dwg_lesen.py DATEI.dwg [ZIEL.dxf] [--liste]
"""
import math
import struct
import sys


class DwgFehler(Exception):
    pass


# ---------------------------------------------------------------------------------------------- Dateistruktur

def _entschleiern(daten):
    """Der R2004-Dateikopf (0x6C Byte ab 0x80) ist mit einem festen Pseudozufallsstrom XOR-verknüpft."""
    seed, aus = 1, bytearray()
    for b in daten:
        seed = (seed * 0x343FD + 0x269EC3) & 0xFFFFFFFF
        aus.append(b ^ ((seed >> 16) & 0xFF))
    return bytes(aus)


def entpacken(src, groesse):
    """R2004-Kompression (LZ77-Variante mit Opcodes 0x10–0xFF, Ende 0x11)."""
    dst = bytearray()
    i = 0

    def byte():
        nonlocal i
        b = src[i]
        i += 1
        return b

    def literal_laenge():
        b = byte()
        if 0x01 <= b <= 0x0F:
            return b + 3, 0
        if b == 0:
            total = 0x0F
            b = byte()
            while b == 0:
                total += 0xFF
                b = byte()
            return total + b + 3, 0
        return 0, b                                 # Opcode folgt sofort

    def lange_laenge():
        b = byte()
        total = 0
        if b == 0:
            total = 0xFF
            b = byte()
            while b == 0:
                total += 0xFF
                b = byte()
        return total + b

    def zwei_byte_versatz():
        a, b = byte(), byte()
        return (a >> 2) | (b << 6), a & 0x03

    n, op = literal_laenge()
    dst += src[i:i + n]
    i += n
    while i < len(src) and len(dst) < groesse:
        if op == 0:
            op = byte()
        if op >= 0x40:
            anzahl = ((op & 0xF0) >> 4) - 1
            op2 = byte()
            versatz = (op2 << 2) | ((op & 0x0C) >> 2)
            if op & 0x03:
                lit, op = op & 0x03, 0
            else:
                lit, op = literal_laenge()
        elif 0x21 <= op <= 0x3F or op == 0x20 or 0x12 <= op <= 0x1F or op == 0x10:
            if 0x21 <= op <= 0x3F:
                anzahl = op - 0x1E
                versatz, lit = zwei_byte_versatz()
            elif op == 0x20:
                anzahl = lange_laenge() + 0x21
                versatz, lit = zwei_byte_versatz()
            elif op == 0x10:
                anzahl = lange_laenge() + 9
                versatz, lit = zwei_byte_versatz()
                versatz += 0x3FFF
            else:
                anzahl = (op & 0x0F) + 2
                versatz, lit = zwei_byte_versatz()
                versatz += 0x3FFF
            if lit:
                op = 0
            else:
                lit, op = literal_laenge()
        elif op == 0x11:
            break
        else:
            raise DwgFehler(f"unbekannter Opcode 0x{op:02x} bei {i}")
        q = len(dst) - versatz - 1
        if q < 0:
            raise DwgFehler("Rückverweis vor den Anfang")
        for k in range(anzahl):
            dst.append(dst[q + k])
        dst += src[i:i + lit]
        i += lit
    return bytes(dst[:groesse])


def abschnitte(daten):
    """{Abschnittsname: entpackte Daten} einer R2004+-Datei."""
    version = daten[:6].decode("ascii")
    if version not in ("AC1018", "AC1024", "AC1027", "AC1032"):
        raise DwgFehler(f"DWG-Version {version} wird nicht unterstützt (nur 2004–2018)")
    kopf = _entschleiern(daten[0x80:0x80 + 0x6C])
    if not kopf.startswith(b"AcFssFcAJMB"):
        raise DwgFehler("Dateikopf nicht lesbar")
    seitenkarte_id, = struct.unpack_from("<I", kopf, 0x50)
    seitenkarte_adr, = struct.unpack_from("<Q", kopf, 0x54)
    abschnittskarte_id, = struct.unpack_from("<I", kopf, 0x5C)

    def systemseite(adresse):
        typ, roh, gepackt, art, _ = struct.unpack_from("<5I", daten, adresse)
        inhalt = daten[adresse + 20:adresse + 20 + gepackt]
        return entpacken(inhalt, roh) if art == 2 else inhalt

    # Seitenverzeichnis: Seitennummer -> Dateiadresse (Seiten liegen hintereinander ab 0x100)
    karte = systemseite(seitenkarte_adr + 0x100)
    seiten, adr, p = {}, 0x100, 0
    while p + 8 <= len(karte):
        nr, groesse = struct.unpack_from("<iI", karte, p)
        p += 8
        if nr < 0:
            p += 16                                  # Lücke: Eltern, links, rechts, 0
        else:
            seiten[nr] = adr
        adr += groesse
    if seitenkarte_id not in seiten or abschnittskarte_id not in seiten:
        raise DwgFehler("Seitenverzeichnis unvollständig")

    info = systemseite(seiten[abschnittskarte_id])
    anzahl, = struct.unpack_from("<I", info, 0)
    p, ergebnis = 20, {}
    for _ in range(anzahl):
        gesamt, seitenzahl, max_roh, _, gepackt, _, verschl = struct.unpack_from("<QIIIIII", info, p)
        name = info[p + 32:p + 96].split(b"\0", 1)[0].decode("latin-1")
        p += 96
        puffer = bytearray(gesamt)
        for _ in range(seitenzahl):
            seite, groesse, start = struct.unpack_from("<IIQ", info, p)
            p += 16
            a = seiten[seite]
            # Datenseitenkopf (32 Byte) ist mit 0x4164536b ^ Adresse verknüpft
            maske = 0x4164536B ^ a
            werte = [w ^ maske for w in struct.unpack_from("<8I", daten, a)]
            roh_gepackt = werte[2]
            inhalt = daten[a + 32:a + 32 + roh_gepackt]
            block = entpacken(inhalt, max_roh) if gepackt == 2 else inhalt
            ende = min(start + len(block), gesamt)
            puffer[start:ende] = block[:ende - start]
        if name:
            ergebnis[name] = bytes(puffer)
    return version, ergebnis


# ---------------------------------------------------------------------------------------------- Bitstrom

class Bits:
    """Bitweise Lesen nach den DWG-Datentypen (B, BB, BS, BL, BD, DD, RD, RC, RS, RL, MC, MS, H, …)."""

    def __init__(self, daten, bit=0):
        self.d = daten
        self.p = bit

    def B(self):
        b = (self.d[self.p >> 3] >> (7 - (self.p & 7))) & 1
        self.p += 1
        return b

    def bits(self, n):
        w = 0
        for _ in range(n):
            w = (w << 1) | self.B()
        return w

    def RC(self):
        if self.p & 7 == 0:
            b = self.d[self.p >> 3]
            self.p += 8
            return b
        return self.bits(8)

    def roh(self, n):
        if n < 0 or (self.p >> 3) + n > len(self.d):
            raise IndexError("Länge über das Datenende")
        if self.p & 7 == 0:
            a = self.p >> 3
            self.p += 8 * n
            return bytes(self.d[a:a + n])
        return bytes(self.RC() for _ in range(n))

    def RS(self):
        return struct.unpack("<h", self.roh(2))[0]

    def RSu(self):
        return struct.unpack("<H", self.roh(2))[0]

    def RL(self):
        return struct.unpack("<i", self.roh(4))[0]

    def RD(self):
        return struct.unpack("<d", self.roh(8))[0]

    def BB(self):
        return self.bits(2)

    def BS(self):
        c = self.BB()
        if c == 0:
            return self.RS()
        if c == 1:
            return self.RC()
        return 0 if c == 2 else 256

    def BL(self):
        c = self.BB()
        if c == 0:
            return self.RL()
        if c == 1:
            return self.RC()
        return 0

    def BLL(self):
        n = self.bits(3)
        w = 0
        for i in range(n):
            w |= self.RC() << (8 * i)
        return w

    def BD(self):
        c = self.BB()
        if c == 0:
            return self.RD()
        return (None, 1.0, 0.0, None)[c]

    def DD(self, vorgabe):
        c = self.BB()
        if c == 0:
            return vorgabe
        b = bytearray(struct.pack("<d", vorgabe))
        if c == 1:
            b[0:4] = self.roh(4)
        elif c == 2:
            b[4:6] = self.roh(2)
            b[0:4] = self.roh(4)
        else:
            return self.RD()
        return struct.unpack("<d", bytes(b))[0]

    def BT(self):
        return 0.0 if self.B() else self.BD()

    def BE(self):
        return (0.0, 0.0, 1.0) if self.B() else (self.BD(), self.BD(), self.BD())

    def P2RD(self):
        return (self.RD(), self.RD())

    def P3BD(self):
        return (self.BD(), self.BD(), self.BD())

    def MC(self, vorzeichen=True):
        w, s = 0, 0
        while True:
            b = self.RC()
            if b & 0x80:
                w |= (b & 0x7F) << s
                s += 7
                continue
            if vorzeichen:
                neg = b & 0x40
                w |= (b & 0x3F) << s
                return -w if neg else w
            w |= (b & 0x7F) << s
            return w

    def MS(self):
        w, s = 0, 0
        while True:
            lo, hi = self.RC(), self.RC()
            v = lo | (hi << 8)
            if v & 0x8000:
                w |= (v & 0x7FFF) << s
                s += 15
                continue
            return w | (v << s)

    def H(self, bezug=0):
        """Handle-Verweis; relative Codes (6, 8, A, C) beziehen sich auf das eigene Handle."""
        code, n = self.bits(4), self.bits(4)
        w = 0
        for _ in range(n):
            w = (w << 8) | self.RC()
        return {6: bezug + 1, 8: bezug - 1, 0xA: bezug + w, 0xC: bezug - w}.get(code, w)

    def CMC(self):
        """Farbe ab R2004 (ENC-ähnlich, wie im Objekt gespeichert)."""
        self.BS()
        self.BL()
        f = self.RC()
        if f & 1:
            self.TV()
        if f & 2:
            self.TV()

    def ENC(self):
        idx = self.BS()
        if idx:
            f = idx >> 8
            if f & 0x80:
                self.BL()
            if f & 0x20:
                self.BL()
        return idx & 0x1FF


class Objektbits(Bits):
    """Objektdaten ab R2007: Texte stehen in einem eigenen Strom am Ende der Daten."""

    def __init__(self, daten, start_bit, datenbits):
        super().__init__(daten, start_bit)
        self.texte = None
        s = start_bit + datenbits - 1                 # letztes Datenbit: „hat Texte“
        t = Bits(daten, s)
        if t.B():
            s -= 16
            t.p = s
            groesse = t.RSu()
            if groesse & 0x8000:
                s -= 16
                t.p = s
                groesse = (groesse & 0x7FFF) | (t.RSu() << 15)
            self.texte = Bits(daten, s - groesse)

    def TV(self):
        if self.texte is None:
            return ""
        n = self.texte.BS()
        return self.texte.roh(2 * n).decode("utf-16-le", "replace").rstrip("\0")


# ---------------------------------------------------------------------------------------------- Objekte

def objektverzeichnis(handles):
    """AcDb:Handles: {Handle: Versatz in AcDb:AcDbObjects}."""
    karte, p = {}, 0
    while p + 2 <= len(handles):
        groesse = struct.unpack_from(">H", handles, p)[0]
        if groesse <= 2:
            break
        b = Bits(handles, (p + 2) * 8)
        ende = p + groesse
        h = adr = 0
        while (b.p >> 3) < ende:
            h += b.MC(vorzeichen=False)
            adr += b.MC()
            karte[h] = adr
        p = ende + 2                                  # CRC
    return karte


FESTE_TYPEN = {1: "TEXT", 2: "ATTRIB", 3: "ATTDEF", 4: "BLOCK", 5: "ENDBLK", 6: "SEQEND", 7: "INSERT", 8: "MINSERT",
               10: "VERTEX_2D", 11: "VERTEX_3D", 15: "POLYLINE_2D", 16: "POLYLINE_3D", 17: "ARC", 18: "CIRCLE",
               19: "LINE", 27: "POINT", 31: "SOLID", 34: "VIEWPORT", 35: "ELLIPSE", 36: "SPLINE", 44: "MTEXT",
               48: "BLOCK_CONTROL", 49: "BLOCK_HEADER", 50: "LAYER_CONTROL", 51: "LAYER", 77: "LWPOLYLINE",
               78: "HATCH"}
ENTITAETEN = {1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 15, 16, 17, 18, 19, 27, 31, 34, 35, 36, 44, 77, 78}


class Objekt:
    def __init__(self, handle, typ):
        self.handle, self.typ, self.name = handle, typ, FESTE_TYPEN.get(typ, f"TYP{typ}")
        self.werte = {}
        self.eigentuemer = None
        self.verweise = []                            # weitere Handles aus dem Handle-Strom


def objekt_lesen(daten, adresse, version, handle_erwartet):
    b = Bits(daten, adresse * 8)
    groesse = b.MS()
    handlebits = b.MC(vorzeichen=False) if version >= "AC1024" else 0
    start = b.p
    datenbits = groesse * 8 - handlebits
    o = Objektbits(daten, start, datenbits)
    if version >= "AC1024":
        c = o.BB()
        typ = o.RC() if c == 0 else (o.RC() + 0x1F0 if c == 1 else o.RSu())
    else:
        typ = o.BS()
        datenbits = o.RL()
    ob = Objekt(o.H(), typ)
    h = Bits(daten, start + datenbits)                 # Handle-Strom
    # erweiterte Daten (EED)
    n = o.BS()
    while n:
        o.H()
        o.roh(n)
        n = o.BS()
    if typ in ENTITAETEN:
        if o.B():                                     # Grafik vorhanden
            g = o.BLL() if version >= "AC1024" else o.RL()
            o.roh(g)
        modus = o.BB()
        reaktoren = o.BL()
        xdic_fehlt = o.B()
        if version < "AC1018":
            o.B()                                     # nolinks (bis R2000)
        ob.werte["farbe"] = o.ENC()
        o.BD()                                        # Linientypfaktor
        lt, ps = o.BB(), o.BB()                       # Linientyp-, Plotstil-Merker (3 = Verweis im Handle-Strom)
        mat = o.BB() if version >= "AC1021" else 0    # Material
        if version >= "AC1021":
            o.RC()                                    # Schatten
        stile = [o.B(), o.B(), o.B()] if version >= "AC1024" else []   # Visualstile
        ob.werte["unsichtbar"] = o.BS() & 1
        o.RC()                                        # Linienstärke
        ob.werte["modus"] = modus                     # 0 = Eigentümer im Handle-Strom, 1 = Papier-, 2 = Modellbereich
        try:
            _entitaet(o, ob, version)
        except (IndexError, struct.error):
            ob.werte["fehler"] = True
        # Handle-Strom: Eigentümer (nur Modus 0), Reaktoren, XDic, Layer, Linientyp, Material, Plotstil, Visualstile,
        # danach die Verweise des Elements selbst (z. B. Blockkopf einer Einfügung)
        try:
            if modus == 0:
                ob.eigentuemer = h.H(ob.handle)
            for _ in range(reaktoren):
                h.H(ob.handle)
            if not xdic_fehlt:
                h.H(ob.handle)
            ob.werte["layer"] = h.H(ob.handle)
            for merker in (lt == 3, mat == 3, ps == 3, *stile):
                if merker:
                    h.H(ob.handle)
            ob.verweise = _rest_handles(h, ob, datenbits + start + handlebits)
        except (IndexError, struct.error):
            pass
    else:
        reaktoren = o.BL()
        xdic_fehlt = o.B()
        try:
            _objekt(o, ob, version)
            ob.eigentuemer = h.H(ob.handle)
            for _ in range(reaktoren):
                h.H(ob.handle)
            if not xdic_fehlt:
                h.H(ob.handle)
            ob.verweise = _rest_handles(h, ob, datenbits + start + handlebits)
        except (IndexError, struct.error):
            ob.werte["fehler"] = True
    return ob


def _rest_handles(h, ob, ende_bit):
    aus = []
    while h.p + 8 <= ende_bit:
        try:
            aus.append(h.H(ob.handle))
        except (IndexError, struct.error):
            break
    return aus


def _text(o, w):
    df = o.RC()
    w["z"] = 0.0 if df & 1 else o.RD()
    w["p"] = o.P2RD()
    if not df & 2:
        w["ausrichtung"] = (o.DD(w["p"][0]), o.DD(w["p"][1]))
    o.BE()
    o.BT()
    w["schraeg"] = 0.0 if df & 4 else o.RD()
    w["drehung"] = 0.0 if df & 8 else o.RD()
    w["hoehe"] = o.RD()
    w["breite"] = 1.0 if df & 16 else o.RD()
    w["text"] = o.TV()
    w["generation"] = 0 if df & 32 else o.BS()
    w["h_ausr"] = 0 if df & 64 else o.BS()
    w["v_ausr"] = 0 if df & 128 else o.BS()


def _entitaet(o, ob, version):
    w, t = ob.werte, ob.typ
    if t == 19:                                       # LINE
        z0 = o.B()
        x1 = o.RD(); x2 = o.DD(x1); y1 = o.RD(); y2 = o.DD(y1)
        if not z0:
            o.RD(); o.DD(0.0)
        w["a"], w["b"] = (x1, y1), (x2, y2)
    elif t in (17, 18):                               # ARC, CIRCLE
        w["m"] = o.P3BD()
        w["r"] = o.BD()
        o.BT(); o.BE()
        if t == 17:
            w["w1"], w["w2"] = o.BD(), o.BD()
    elif t in (1, 2, 3):                              # TEXT, ATTRIB, ATTDEF
        _text(o, w)
        if t in (2, 3):
            if version >= "AC1024":
                o.RC()
            w["tag"] = o.TV()
            o.BS()
            w["attr_flags"] = o.RC()
            o.B()
            if t == 3:
                w["prompt"] = o.TV()
    elif t in (7, 8):                                 # INSERT, MINSERT
        w["p"] = o.P3BD()
        df = o.BB()
        if df == 3:
            s = (1.0, 1.0, 1.0)
        elif df == 1:
            s = (1.0, o.DD(1.0), o.DD(1.0))
        elif df == 2:
            x = o.RD()
            s = (x, x, x)
        else:
            x = o.RD()
            s = (x, o.DD(x), o.DD(x))
        w["s"] = s
        w["drehung"] = o.BD()
        o.P3BD()
        w["hat_attribute"] = o.B()
        if w["hat_attribute"]:
            w["attribute_anzahl"] = o.BL()
        if t == 8:
            w["spalten"], w["zeilen"] = o.BS(), o.BS()
            w["dx"], w["dy"] = o.BD(), o.BD()
    elif t == 77:                                     # LWPOLYLINE
        f = o.BS()
        w["breite"] = o.BD() if f & 4 else 0.0
        if f & 8:
            o.BD()
        if f & 2:
            o.BD()
        if f & 1:
            o.P3BD()
        n = o.BL()
        nb = o.BL() if f & 16 else 0
        nv = o.BL() if f & 1024 else 0
        nw = o.BL() if f & 32 else 0
        pts = []
        for i in range(n):
            if i == 0:
                pts.append(o.P2RD())
            else:
                pts.append((o.DD(pts[-1][0]), o.DD(pts[-1][1])))
        w["punkte"] = pts
        w["bulges"] = [o.BD() for _ in range(nb)]
        for _ in range(nv):
            o.BL()
        for _ in range(nw):
            o.BD(); o.BD()
        w["geschlossen"] = bool(f & 512)
    elif t == 15:                                     # POLYLINE_2D
        w["flags"] = o.BS()
        o.BS(); o.BD(); o.BD(); o.BT(); o.BD(); o.BE()
        w["anzahl"] = o.BL()
    elif t == 10:                                     # VERTEX_2D
        f = o.RC()
        w["p"] = o.P3BD()
        sw = o.BD()
        if sw >= 0:
            o.BD()
        w["bulge"] = o.BD()
        if version >= "AC1024":
            o.BL()
        o.BD()
    elif t == 35:                                     # ELLIPSE
        w["m"] = o.P3BD()
        w["achse"] = o.P3BD()
        o.P3BD()
        w["verhaeltnis"] = o.BD()
        w["w1"], w["w2"] = o.BD(), o.BD()
    elif t == 31:                                     # SOLID
        o.BT()
        o.BD()
        w["ecken"] = [o.P2RD() for _ in range(4)]
        o.BE()
    elif t == 44:                                     # MTEXT
        w["p"] = o.P3BD()
        o.P3BD()
        w["x_richtung"] = o.P3BD()
        w["breite"] = o.BD()
        o.BD()                                        # Rechteckhöhe (R2007+)
        w["hoehe"] = o.BD()
        w["anhang"] = o.BS()
        o.BS(); o.BD(); o.BD()
        w["text"] = o.TV()
    elif t == 4:                                      # BLOCK
        w["name"] = o.TV()


def _objekt(o, ob, version):
    w = ob.werte
    if ob.typ == 49:                                  # BLOCK_HEADER
        w["name"] = o.TV()
        o.B()                                         # 64-Merker
        if version < "AC1021":
            o.BS()                                    # xref-Index (ab R2007 entfallen)
        o.B()
        w["anonym"] = o.B()
        w["hat_attribute"] = o.B()
        w["xref"] = o.B()
        o.B()
        o.B()
        w["anzahl"] = o.BL() if not w["xref"] else 0
        w["basis"] = o.P3BD()
        o.TV()
        while o.RC():
            pass
        o.TV()
        n = o.BL()
        o.roh(n)
        o.BS(); o.B(); o.RC()
    elif ob.typ == 51:                                # LAYER
        w["name"] = o.TV()


def lesen(pfad):
    """(version, {handle: Objekt}) einer DWG-Datei."""
    version, ab = abschnitte(open(pfad, "rb").read())
    karte = objektverzeichnis(ab["AcDb:Handles"])
    daten = ab["AcDb:AcDbObjects"]
    objekte = {}
    for h, adr in karte.items():
        try:
            ob = objekt_lesen(daten, adr, version, h)
        except (IndexError, struct.error) as e:
            continue
        objekte[ob.handle] = ob
    return version, objekte


# ---------------------------------------------------------------------------------------------- DXF

def _name(n):
    """Blocknamen kommen mit XML-Maskierung (_x002d_ = „-“)."""
    import re
    n = re.sub(r"_x([0-9a-fA-F]{4})_", lambda m: chr(int(m.group(1), 16)), n)
    return re.sub(r'[<>/\\":;?*|=`,]', "_", n)      # in DXF-Blocknamen nicht erlaubt


def nach_dxf(objekte, ziel):
    """Geometrie und Texte als DXF (R2010) schreiben: Blöcke wie in der DWG, Modellbereich mit den Einfügungen."""
    import ezdxf
    doc = ezdxf.new("R2010")
    kopf = {h: o for h, o in objekte.items() if o.typ == 49}
    namen = {}
    for h, o in kopf.items():
        roh = o.werte.get("name", "").lower()
        n = _name(o.werte.get("name", ""))
        if roh.startswith("*model_space"):
            namen[h] = None                           # Modellbereich
        elif roh.startswith("*paper_space"):
            namen[h] = False                          # Papierbereich: weglassen
        else:
            namen[h] = n
            doc.blocks.new(n, base_point=o.werte.get("basis", (0, 0, 0))[:2])
    modell = doc.modelspace()
    zaehler = {"geschrieben": 0, "übersprungen": 0}
    for o in objekte.values():
        if o.typ not in ENTITAETEN or o.typ in (4, 5, 6) or o.werte.get("fehler") or o.werte.get("unsichtbar"):
            continue
        modus = o.werte.get("modus")
        if modus == 2:
            raum = modell
        elif modus == 0 and namen.get(o.eigentuemer):
            raum = doc.blocks.get(namen[o.eigentuemer])
        elif modus == 0 and o.eigentuemer in namen and namen[o.eigentuemer] is None:
            raum = modell
        else:
            continue
        if _schreiben(raum, o, namen):
            zaehler["geschrieben"] += 1
        else:
            zaehler["übersprungen"] += 1
    doc.saveas(ziel)
    return zaehler


def _schreiben(raum, o, namen):
    w, t = o.werte, o.typ
    if t == 19:
        raum.add_line(w["a"], w["b"])
    elif t == 18:
        raum.add_circle(w["m"][:2], w["r"])
    elif t == 17:
        raum.add_arc(w["m"][:2], w["r"], math.degrees(w["w1"]), math.degrees(w["w2"]))
    elif t == 35:
        raum.add_ellipse(w["m"], w["achse"], w["verhaeltnis"], w["w1"], w["w2"])
    elif t == 77:
        b = w["bulges"] + [0.0] * (len(w["punkte"]) - len(w["bulges"]))
        raum.add_lwpolyline([(x, y, 0, 0, bu) for (x, y), bu in zip(w["punkte"], b)], format="xyseb",
                            close=w["geschlossen"])
    elif t == 31:
        e = w["ecken"]
        raum.add_solid([e[0], e[1], e[2], e[3]])
    elif t in (1, 2, 3):
        if t == 3:
            raum.add_attdef(w.get("tag", ""), insert=w["p"], text=w.get("text", ""),
                            dxfattribs={"height": w["hoehe"], "rotation": math.degrees(w["drehung"])})
        else:
            attr = {"height": w["hoehe"], "rotation": math.degrees(w["drehung"]), "width": w["breite"],
                    "halign": w["h_ausr"], "valign": w["v_ausr"]}
            if w["h_ausr"] or w["v_ausr"]:
                attr["align_point"] = w.get("ausrichtung", w["p"])
            raum.add_text(w.get("text", ""), dxfattribs=dict(attr, insert=w["p"]))
    elif t == 44:
        raum.add_mtext(w.get("text", ""), dxfattribs={"insert": w["p"], "char_height": w["hoehe"],
                                                      "width": w["breite"], "attachment_point": w["anhang"] or 1})
    elif t in (7, 8):
        block = namen.get(o.verweise[0]) if o.verweise else None
        if not block:
            return False
        raum.add_blockref(block, w["p"], dxfattribs={"xscale": w["s"][0], "yscale": w["s"][1],
                                                     "rotation": math.degrees(w["drehung"])})
    else:
        return False
    return True


if __name__ == "__main__":
    import collections
    v, obj = lesen(sys.argv[1])
    print(v, len(obj), "Objekte:", dict(collections.Counter(o.name for o in obj.values()
                                                            if o.typ in ENTITAETEN).most_common()))
    if len(sys.argv) > 2:
        print(nach_dxf(obj, sys.argv[2]), "->", sys.argv[2])
