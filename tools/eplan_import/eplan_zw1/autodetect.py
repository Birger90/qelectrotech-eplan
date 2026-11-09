"""Satzaufbau einer EPLAN-Tabelle (.eod): (Satzlänge, Lage Inline-Strom, Lage zlib-Strom, Satzanzahl).

Satzlänge: steht im Kopf der Indexdatei <name>.eox (Kennung fe 53), Bytes 0x0D–0x0E (Big Endian) + 5 – belegt an
allen Tabellen von sieben Projekten (z. B. Connection 963+5 = 968, Instance 210+5 = 215, Function 1316+5 = 1321,
Plc 859+5 = 864). Die frühere Suche nach der kleinsten passenden Satzlänge griff bei wenigen Sätzen oder leeren
Sätzen am Anfang daneben (Projekt E Plc 3480 = 4 × 870, damit 15 von 20 Sätzen verloren) – sie bleibt nur als Rückfall
ohne .eox. Inline-Strom beginnt mit 61 ea <Klasse LE> <ID LE>; der zlib-Strom liegt in allen Tabellen genau
8 Bytes dahinter.
"""
import struct, os
from collections import Counter


def satzlaenge(path):
    eox = os.path.splitext(path)[0] + ".eox"
    if os.path.exists(eox):
        k = open(eox, "rb").read(16)
        if len(k) >= 15 and k[:2] == b"\xfe\x53":
            return struct.unpack(">H", k[13:15])[0] + 5
    return None


def loeschmarke(path, rs):
    """Lage des Satzstatus-Bytes im Satz (= Satzlänge aus .eox) oder None, wenn nicht belegt.

    EPLAN speichert jede Tabelle als D-ISAM-Datei (EDisam96u.dll, C-ISAM-Aufbau): Satz = Nutzdaten (Satzlänge aus
    .eox) + 1 Statusbyte + 4 Byte. Statusbyte 0x0A = Satz gültig, 0x00 = Satz gelöscht/frei. Belege
    (FORMATBESCHREIBUNG Abschnitt 19, Satzstatus): isDelRecUnsafe -> 0x18000d6c0 schreibt 1 Byte 0 an
    (Satznr-1)*Satzabstand + Satzlänge und hängt die Satznummer an die Freiliste (.eox-Kopf 0x19); isRebuildFree
    (0x18000a9c0) baut die Freiliste aus genau den Sätzen mit Statusbyte 0 neu auf; 0x180001e10 liefert nur Sätze
    mit Statusbyte 0x0A. Gilt nur, wenn der Satzabstand aus dem .eox-Kopf stammt (sonst None, nichts gefiltert)."""
    r = satzlaenge(path)
    return r - 5 if r and r == rs else None


def _suche(d):
    L = len(d)
    for r in range(16, min(L, 20000) + 1):
        if L % r:
            continue
        n = L // r; ok = 0; chk = min(n, 50)
        for i in range(chk):
            rr = d[i*r:(i+1)*r]
            k = b'\x61\xea' + rr[4:6][::-1] + rr[0:4][::-1]
            if rr.find(k) >= 0 or b'\x78\xda' in rr:
                ok += 1
        if ok == chk:
            return r
    return None


def detect(path):
    d = open(path, 'rb').read(); L = len(d)
    if L < 16:
        return None
    r = satzlaenge(path)
    if not r or L % r:
        r = _suche(d)
    if not r:
        return None
    pc = Counter(); zc = Counter()
    for i in range(L // r):
        rr = d[i*r:(i+1)*r]
        k = b'\x61\xea' + rr[4:6][::-1] + rr[0:4][::-1]
        p = rr.find(k)
        if p >= 0:
            pc[p] += 1
        else:
            # zlib-Kopf: 78 DA (bis EPLAN 2026) oder 78 9C (EPLAN 2027, News-Projekt), 78 01/5E der Vollständigkeit halber
            zs = [z for z in (rr.find(h) for h in (b'\x78\xda', b'\x78\x9c', b'\x78\x01', b'\x78\x5e')) if z >= 0]
            if zs:
                zc[min(zs)] += 1
    pin = pc.most_common(1)[0][0] if pc else None
    pz = zc.most_common(1)[0][0] if zc else None
    if pin is not None:
        pz = pin + 8                      # belegt: zlib-Strom immer 8 Byte hinter der Inline-Position
    elif pz is not None:
        pin = pz - 8
    if pin is None:
        return None                       # Tabelle ohne lesbaren Satz (alle leer/gelöscht)
    return r, pin, pz, L // r


if __name__ == '__main__':
    import sys
    base = sys.argv[1]
    for f in ['Connection.eod','Function.eod','Page.eod','Article.eod','ArticleRef.eod','IPoint.eod','Relation.eod','Potential.eod','Plc.eod','Location.eod','Instance.eod']:
        print(f, detect(os.path.join(base, f)))
