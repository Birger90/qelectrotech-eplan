"""Anschlusslogik der Funktionsdefinitionen aus dem Archiv (x/FctDefLib.fdb/FctDef.eod) lesen.

Belegt (FORMATBESCHREIBUNG_EOD.md Abschnitt 19, „SPS-Zielverfolgung“):
- FctDef.eod: Sätze Klasse 60, Funktionsdefinition = efc5/efc6/efc7 (Kategorie, Gruppe, Nummer) wie an der Funktion;
  ee86 = je Anschluss ein DMConnectionPoint (Klasse 16) in Anschlussreihenfolge.
- zlib-Überlauf liegt in FctDefLib.fdb/Blob.eod in 256-Byte-Blöcken: Kopf 11 Byte (Tabelle u16 BE, Objekt-ID u32 BE,
  Blocknummer u16 BE, Blockanzahl u16 BE, Flag), Nutzdaten bis Byte 255; das letzte Byte eines Blocks und des Satzes
  gehört nicht zum Strom (geprüft über die Adler-32-Prüfsumme des zlib-Stroms, alle 1160 Sätze lesbar).
- DMConnectionPoint (ETrDMMetau.erx-Registrierung -> Member-Offset, ETrDMProjectu.erx-Getter auf denselben Offset):
  eb01 0x48 Anschlussbezeichnung (getInternalDesignation), eb02 0x70 Anschlussart (getType), eb03 0x78 Potenzial
  weiterleiten nach, eb04 0xa0 Potenzialtyp, eb05 0xa8 Verbraucher/Quelle nach, eb06 0xd0 Signalart (getSignal),
  eb07 0xd2 Zielverfolgung (SPS) nach, 1-basiert (getPlcTrackingOneBased), eb0f 0xd8 Zielverfolgung als Text
  (getPlcTracking, Verweis auf andere Funktion), eb08 0x100 Anzahl Ziele, eb09 0x102 Anzahl Stegbrücken.
- Signalart (Eplan.EplApi.DataModel.Pin.SignalType): 0 neutral, 1 Sensor, 2 Aktor, 3 Anschlussversorgung,
  4 Geräteversorgung, 5 Quelle, 6 DE, 7 DA, 8 AE, 9 AA, 10 Bus-Quelle, 11 Bus-Eingang, 12 Bus-Ausgang, 13 Druck-,
  14 Sauganschluss, 15 Busende.
"""
import collections
import json
import os
import pickle
import struct
import zlib

import eodlib
from autodetect import detect
from trace import load_types

SIGNAL_SENSOR, SIGNAL_AKTOR = 1, 2
SIGNAL_EINGANG, SIGNAL_AUSGANG = (6, 8), (7, 9)       # DE/AE suchen Sensoren, DA/AA Aktoren


def _typen():
    t = load_types()
    seed = {int(k, 16): v for k, v in json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                                                   "daten_typen", "seed.json"))).items()}
    t2 = dict(t)
    t2.update(eodlib.EPLAN_TYP)                     # belegte Metamodell-Typen (eb01 EString usw.)
    t2.update({0xee71: "str", 0xee72: "str"})       # Namen der Definition (mehrsprachig)
    return (t2, t, seed)


def _saetze(ordner, name):
    pfad = os.path.join(ordner, name)
    t = detect(pfad) if os.path.exists(pfad) else None
    if t is None:
        return
    rs, pin, pz, _ = t
    blob = open(os.path.join(ordner, "Blob.eod"), "rb").read() if os.path.exists(os.path.join(ordner, "Blob.eod")) else b""
    bloecke = collections.defaultdict(dict)
    for k in range(0, len(blob) - 255, 256):
        tab, oid, nr, anz, _f = struct.unpack(">HIHHB", blob[k:k + 11])
        bloecke[(oid, anz, tab)][nr] = blob[k + 11:k + 255]
    d = open(pfad, "rb").read()
    for ix in range(len(d) // rs):
        r = d[ix * rs:(ix + 1) * rs]
        rid = struct.unpack(">I", r[0:4])[0]
        cls = struct.unpack(">H", r[4:6])[0]
        key = b"\x61\xea" + struct.pack("<H", cls) + struct.pack("<I", rid)
        if r[pin:pin + 8] == key:
            yield rid, cls, r[pin:], 8
            continue
        csz = struct.unpack("<I", r[pz - 4:pz])[0]
        try:
            yield rid, cls, zlib.decompress(r[pz:pz + csz]), 0
            continue
        except zlib.error:
            pass
        teil = r[pz:-1]
        for (oid, anz, _tab), ch in bloecke.items():
            if oid != rid or len(ch) != anz:
                continue
            z = zlib.decompressobj()
            try:
                strom = z.decompress(teil + b"".join(ch[i] for i in range(anz)))
            except zlib.error:
                continue
            if z.eof:
                yield rid, cls, strom, 0
                break


def anschlusslogik(arbeit):
    """{(efc5, efc6, efc7): [ {Eigenschaft: Wert} je Anschluss ]} aus x/FctDefLib.fdb; zwischengespeichert in
    arbeit/fktdef_logik.pkl. Leeres dict, wenn das Archiv keine Funktionsdefinitionen enthält."""
    cache = os.path.join(arbeit, "fktdef_logik.pkl")
    ordner = os.path.join(arbeit, "x", "FctDefLib.fdb")
    quelle = os.path.join(ordner, "FctDef.eod")
    if os.path.exists(cache) and (not os.path.exists(quelle) or os.path.getmtime(cache) >= os.path.getmtime(quelle)):
        return pickle.load(open(cache, "rb"))
    erg, nicht_lesbar = {}, 0
    typen = _typen()
    for rid, cls, strom, tail in _saetze(ordner, "FctDef.eod") or []:
        props = None
        for ty in typen:
            try:
                props, _ = eodlib.parse_stream(strom, ty, learn=True, tail=tail, budget=50000)
                break
            except (eodlib.Fail, eodlib.Budget):
                continue
        if props is None:
            nicht_lesbar += 1
            continue
        g = {p: v for p, t, v in props if p in (0xefc5, 0xefc6, 0xefc7)}
        erg[(g.get(0xefc5), g.get(0xefc6), g.get(0xefc7))] = \
            [dict((a, b) for a, _, b in v[1]) for p, t, v in props if p == 0xee86 and t == "obj"]
    erg["_nicht_lesbar"] = nicht_lesbar
    pickle.dump(erg, open(cache, "wb"))
    return erg
