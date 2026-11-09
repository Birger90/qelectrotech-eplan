"""Symbolbibliotheken (*.sdb) und Seitengrafik (Instance.eod) eines entpackten EPLAN-.zw1 lesen.

Aufbau (empirisch, Projekt A / EPLAN 2026):
- Jede .sdb hat Symbol.eod (Klasse 20, Name in eb51), Variant.eod (Klasse 45, ed95 = Verweis aufs Symbol,
  edbe/edbf = Anschlusspunkt und -richtung, ed97 = Variantennummer) und Instance.eod mit den Grafikelementen.
- Grafikelemente (Klasse): 31 Linie (ec73/ec74), 34 Polylinie (eccd-Punkte), 30 Kreis/Bogen,
  32 Text, 89 Rechteck (f0d3/f0d4); Besitzer in ebf2 (Variante 45, Seite 4, Formular 51 …).
- Überlauf: Satz endet mit 0A; fehlt das zlib-Ende, liegt der Rest in Blob.eod desselben Ordners in
  256-Byte-Blöcken <Tabelle u16 BE><ID u32 BE><Nr u16 BE><Anzahl u16 BE><Flag u8><244 Byte><1 Byte>
  (im Projekt-Blob mit vorangestelltem 0A).
  Welche Blockgruppe passt, entscheidet die zlib-Prüfsumme.
Typen unbekannter Eigenschaften werden je Tabelle per Mehrheitsentscheid gelernt (zwei Durchgänge)."""
import collections
import itertools
import os
import struct
import sys
import zlib

HIER = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HIER)
from autodetect import detect, loeschmarke                     # noqa: E402
from eodlib import parse_stream, Fail, Budget                  # noqa: E402
from trace import load_types                                   # noqa: E402

_BLOB = {}


def _blob(ordner):
    if ordner not in _BLOB:
        p = os.path.join(ordner, "Blob.eod")
        b = open(p, "rb").read() if os.path.exists(p) else b""
        idx = collections.defaultdict(list)
        for o in range(0, len(b) - 255, 256):
            for hl in (0, 1):                     # Projekt-Blob mit 0A-Präfix, Bibliotheks-Blob ohne
                if hl and b[o] != 0x0A:
                    continue
                _, rid, nr, anz, _ = struct.unpack_from(">HIHHB", b, o + hl)
                if 0 < anz <= 400 and nr < anz:
                    idx[rid].append((anz, nr, b[o + hl + 11:o + hl + 255]))   # 244 Byte Nutzdaten
                    break
        _BLOB[ordner] = idx
    return _BLOB[ordner]


def _ueberlauf(ordner, rid, r, pz, csz):
    gruppen = collections.defaultdict(dict)
    for anz, nr, pl in _blob(ordner).get(rid, []):
        gruppen[anz].setdefault(nr, []).append(pl)
    for ende in (1, 5):
        teil = r[pz:len(r) - ende]
        for anz, g in sorted(gruppen.items()):
            if len(g) != anz:
                continue
            for kombi in itertools.islice(itertools.product(*[g[k] for k in range(anz)]), 64):
                d = zlib.decompressobj()
                try:
                    out = d.decompress(teil + b"".join(kombi))
                    if d.eof:
                        return out
                except zlib.error:
                    pass
    return None


def saetze(pfad):
    """(id, klasse, strom, inline) je Satz; strom None, wenn nicht rekonstruierbar."""
    t = detect(pfad) if os.path.exists(pfad) else None
    if t is None:
        return                                # leere Tabelle (kein lesbarer Satz)
    rs, pin, pz, _ = t
    d = open(pfad, "rb").read()
    ordner = os.path.dirname(pfad)
    marke = loeschmarke(pfad, rs)
    for i in range(len(d) // rs):
        r = d[i * rs:(i + 1) * rs]
        if marke is not None and r[marke] == 0:
            continue                          # D-ISAM-Satz gelöscht (Statusbyte 0, autodetect.loeschmarke)
        rid = struct.unpack(">I", r[:4])[0]
        cls = struct.unpack(">H", r[4:6])[0]
        if pin is not None and r[pin:pin + 8] == b"\x61\xea" + struct.pack("<HI", cls, rid):
            yield rid, cls, r[pin:], True
            continue
        csz = struct.unpack("<I", r[pz - 4:pz])[0]
        try:
            yield rid, cls, zlib.decompress(r[pz:pz + csz]), False
        except zlib.error:
            yield rid, cls, _ueberlauf(ordner, rid, r, pz, csz), False


def lesen(pfad, typen=None, budget=20000):
    """Tabelle parsen: {id: (klasse, props | None)}. Zwei Durchgänge mit Typ-Mehrheitsentscheid."""
    basis = dict(typen or load_types())
    roh = list(saetze(pfad))
    stimmen = collections.defaultdict(collections.Counter)
    for _, _, s, inl in roh:
        if s is None:
            continue
        try:
            _, nt = parse_stream(s, basis, learn=True, tail=8 if inl else 0, budget=budget)
            for k, v in nt.items():
                stimmen[k][v] += 1
        except (Fail, Budget):
            pass
    gelernt = dict(basis)
    gelernt.update({k: c.most_common(1)[0][0] for k, c in stimmen.items()})
    aus = {}
    for rid, cls, s, inl in roh:
        p = None
        if s is not None:
            for tt in (gelernt, basis):
                try:
                    p = parse_stream(s, tt, learn=True, tail=8 if inl else 0, budget=budget)[0]
                    break
                except (Fail, Budget):
                    continue
        aus[rid] = (cls, p)
    return aus, gelernt


def wert(props, pid, alle=False):
    w = [v for p, _, v in props or [] if p == pid]
    return w if alle else (w[0] if w else None)


def objekte(props, pid):
    return [v[1] for p, t, v in props or [] if p == pid and t == "obj"]
