"""Projekteinstellungen eines EPLAN-Archivs lesen (x/Settings.xdb/Setting.eod).

Aufbau eines Eintrags (am Archiv bestimmt): Satz beginnt mit ff ff 61 ea; danach Felder <Kennung u16 LE><Wert>.
  ff df  Name der Einstellung (z. B. „EsCrossReference.ContactImage.ContactorDisplayBitMask“), nullterminiert
  ff f2  Wert als Text (z. B. „262“, „16“), nullterminiert
  ff f4  Einheit/Bereich (z. B. „0.1/32767“), nullterminiert
Weitere Felder (ff d7, d8, d9, e2, e4, f9) sind Kennungen/Typ und werden hier nicht gebraucht.

    python eplan_einstellungen.py <Arbeitsordner mit x/> [Filter]
"""
import os
import sys


def _text(b, i):
    j = b.find(b"\x00", i)
    return b[i:j].decode("utf-8", "replace") if j >= 0 else ""


def lesen(arbeit):
    """{Name: Wert} aller Projekteinstellungen (bei mehrfachem Vorkommen der letzte Datensatz)."""
    pfad = os.path.join(arbeit, "x", "Settings.xdb", "Setting.eod")
    if not os.path.exists(pfad):
        return {}
    b = open(pfad, "rb").read()
    out = {}
    start = 0
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from autodetect import satzlaenge, loeschmarke
    rs = satzlaenge(pfad)
    marke = loeschmarke(pfad, rs) if rs and len(b) % rs == 0 else None
    while True:
        i = b.find(b"\xff\xff\x61\xea", start)
        if i < 0:
            break
        nxt = b.find(b"\xff\xff\x61\xea", i + 4)
        satz = b[i:nxt if nxt > 0 else len(b)]
        start = i + 4
        if marke is not None and b[i // rs * rs + marke] == 0:
            continue                      # D-ISAM-Satz gelöscht (Statusbyte 0, autodetect.loeschmarke)
        n = satz.find(b"\xdf\xff")
        if n < 0:
            continue
        name = _text(satz, n + 2)
        if not name or not all(32 <= ord(c) < 127 for c in name):
            continue
        w = satz.find(b"\xf2\xff", n + 2 + len(name))
        if w >= 0:
            out[name] = _text(satz, w + 2)
    return out


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    e = lesen(sys.argv[1])
    filt = sys.argv[2] if len(sys.argv) > 2 else ""
    for k in sorted(e):
        if filt.lower() in k.lower():
            print(f"{k} = {e[k]}")
    print(len(e), "Einstellungen")
