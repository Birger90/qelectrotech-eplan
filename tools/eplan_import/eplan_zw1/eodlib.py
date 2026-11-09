import os
"""EOD-Eigenschaftsblock-Parser (empirisch abgeleitet, EPLAN 2026 .zw1-Backup).
Rekordkopf: [0:4] ID (u32 BE), [4:6] Klassen-ID (u16 BE).
Eigenschaftsblock an tabellenfester Position:
  inline: Bytes ab Position p (beginnt mit 61 EA <Klasse LE u16> <ID LE u32>)
  zlib:   9 Byte Kopf (Flag, u32, u32 LE = ?, komprimierte Länge), dann zlib
Eigenschaftsstrom: u16 LE Property-ID, Wert typabhängig; Ende = ID 0x0000.
"""
import struct, zlib, os, sys
sys.setrecursionlimit(100000)

BASE = os.environ.get('EOD_BASE', '.')
TABLES = {  # datei: (satzgroesse, offset inline-start, offset zlib-start)
 'Connection.eod': (980, 375, 383), 'Function.eod': (1333, 328, 336),
 'Page.eod': (1134, 129, 137), 'Article.eod': (1940, 135, 143),
 'ArticleRef.eod': (667, 422, 430), 'IPoint.eod': (518, 173, 181),
 'Relation.eod': (86, 39, 47), 'Potential.eod': (513, 108, 116),
 'Plc.eod': (870, 725, 733), 'Location.eod': (1116, 111, 119),
 'Instance.eod': (215, 60, 68), 'Message.eod': (236, 101, 109),
 'PlaceHolderText.eod': (233, 48, 56),
}

_BLOB = {}
def blob_continue(fname, r, pz, csz):
    """Überlaufende zlib-Daten: Fortsetzung in Blob.eod desselben Ordners suchen.
    Kandidaten = Fundstellen der Objekt-ID (u32 BE) im Blob; akzeptiert nur bei zlib-EOF mit gültiger Adler-32."""
    bp = os.path.join(BASE, os.path.dirname(fname), 'Blob.eod')
    if bp not in _BLOB:
        _BLOB[bp] = open(bp, 'rb').read() if os.path.exists(bp) else b''
    blob = _BLOB[bp]
    part = r[pz:len(r)-5]; need = csz - len(part)
    if need <= 0 or not blob: return None
    rid = r[0:4]
    # Variante 1: verkettete 256-Byte-Blöcke: 0A | Tabelle u16 BE | ID u32 BE | Nr u16 BE | Anzahl u16 BE | Flag | Nutzdaten
    chunks = {}; start = 0
    while True:
        h = blob.find(rid, start)
        if h < 0: break
        c = h - 3
        if c >= 0 and h + 9 <= len(blob):
            seq, cnt = struct.unpack_from('>HH', blob, h+4)
            if 0 < cnt <= 64 and seq < cnt and blob[h+8] == 1:
                chunks.setdefault(cnt, {}).setdefault(seq, []).append(blob[c+12:c+256])
        start = h + 1
    import itertools
    for cnt, ch in chunks.items():
        if len(ch) != cnt: continue
        combos = itertools.product(*[ch[i] for i in range(cnt)])
        for n_try, combo in enumerate(combos):
            if n_try > 256: break
            payload = b''.join(combo)
            d = zlib.decompressobj()
            try:
                out = d.decompress(part + payload[:need])
                if d.eof: return out
            except Exception: pass
    start = 0
    while True:
        h = blob.find(rid, start)
        if h < 0: return None
        for k in range(4, 24):
            o = h + k; d = zlib.decompressobj()
            try:
                out = d.decompress(part + blob[o:o+need])
                if d.eof and not d.unused_data: return out
            except Exception: pass
        start = h + 1

def records(fname):
    if os.environ.get('EOD_AUTO'):
        from autodetect import detect
        t = detect(os.path.join(BASE, fname))
        if t is None:
            return                        # leere Tabelle
        rs, pin, pz, _ = t
    else:
        rs, pin, pz = TABLES[fname]
    d = open(os.path.join(BASE, fname), 'rb').read()
    from autodetect import loeschmarke
    marke = loeschmarke(os.path.join(BASE, fname), rs)
    for i in range(len(d) // rs):
        r = d[i*rs:(i+1)*rs]
        if marke is not None and r[marke] == 0:
            continue                      # D-ISAM-Satz gelöscht (Statusbyte 0, autodetect.loeschmarke)
        rid = struct.unpack('>I', r[0:4])[0]; cls = struct.unpack('>H', r[4:6])[0]
        key = b'\x61\xea' + struct.pack('<H', cls) + struct.pack('<I', rid)
        if r[pin:pin+8] == key:
            stream = r[pin:]; mode = 'inline'
        else:
            csz = struct.unpack('<I', r[pz-4:pz])[0]
            try:
                stream = zlib.decompress(r[pz:pz+csz]); mode = 'zlib'
            except Exception as e:
                stream = None; mode = 'ERR:' + str(e)
                cont = blob_continue(fname, r, pz, csz)
                if cont is not None: stream = cont; mode = 'zlib+blob'
        yield dict(idx=i, id=rid, cls=cls, raw=r, stream=stream, mode=mode)

CANDS = ['s2', 'str', 'ref', 'b1', 'i4', 'kv', 'obj', 'd8', 'p16', 'rp', 'q32', 'arr2', 'arrd', 'blob']
# Nur zur Untersuchung (EOD_AUSWEICHTYPEN=1): Phoenix Function.eod 673 -> 1396 „lesbare“ Sätze, aber 560 der neuen
# enthalten Unsinnswerte (Fehllesungen). Belegte Typen je Eigenschaft fehlen noch (EPLAN-Persistenzschicht).
AUSWEICHTYPEN = os.environ.get('EOD_AUSWEICHTYPEN', '0') == '1'

# Belegte Typen aus dem EPLAN-Metamodell (daten_typen/eplan_metamodell_typen.json, Herkunft dort): bei unbekannten
# IDs erster Typ, bei bekannten zweiter Versuch (Typ ist klassenabhängig, der Scan deckt nicht jede Klasse ab).
EPLAN_TYP = {}
if os.environ.get('EOD_EPLAN_TYPEN', '1') == '1':
    _p = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'daten_typen', 'eplan_metamodell_typen.json')
    if os.path.exists(_p):
        import json as _json
        EPLAN_TYP = {int(k, 16): v['typen'][0] for k, v in _json.load(open(_p, encoding='utf-8'))['typen'].items()
                     if len(v['typen']) == 1}
_STRUKTUR = {'dN', 'arrd', 'arr2', 'rp', 'blob'}       # q32 nicht: f412 erlernt q32, belegt EPoint3D (24 Byte)
ABSCHNITTE = int(os.environ.get('EOD_ABSCHNITTE', '6'))   # Abschnittstrenner ff ff je Satz (bisher fest 1)

class Fail(Exception): pass

def rd(buf, pos, t):
    """liest Wert vom Typ t; liefert (wert, neue_pos) oder wirft Fail. obj wird extern behandelt."""
    L = len(buf)
    if t == 'b1':
        if pos+1 > L: raise Fail
        return buf[pos], pos+1
    if t == 's2':
        if pos+2 > L: raise Fail
        return struct.unpack_from('<H', buf, pos)[0], pos+2
    if t == 'i4':
        if pos+4 > L: raise Fail
        return struct.unpack_from('<i', buf, pos)[0], pos+4
    if t == 'ref':
        if pos+6 > L: raise Fail
        c, i = struct.unpack_from('<HI', buf, pos); return (c, i), pos+6
    if t == 'i8':
        if pos+8 > L: raise Fail
        return struct.unpack_from('<q', buf, pos)[0], pos+8
    if t == 'rp':
        if pos+10 > L: raise Fail
        c, i, k = struct.unpack_from('<HII', buf, pos); return (c, i, k), pos+10
    if t == 'd8':
        if pos+8 > L: raise Fail
        return struct.unpack_from('<d', buf, pos)[0], pos+8
    if t == 'blob':
        if pos+4 > L: raise Fail
        n = struct.unpack_from('<I', buf, pos)[0]
        if pos+4+n > L: raise Fail
        return buf[pos+4:pos+4+n], pos+4+n
    if t == 'q32':
        if pos+32 > L: raise Fail
        return struct.unpack_from('<4d', buf, pos), pos+32
    if t == 'p16':
        if pos+16 > L: raise Fail
        return struct.unpack_from('<dd', buf, pos), pos+16
    if t == 'h16':                     # EMD5Hash / EGuid (EPLAN-Metamodell, z. B. ed1b/ed1c): 16 Byte roh
        if pos+16 > L: raise Fail
        return buf[pos:pos+16].hex(), pos+16
    if t == 'p24':                     # EPoint3D (EPLAN-Metamodell, z. B. f413/f414)
        if pos+24 > L: raise Fail
        return struct.unpack_from('<3d', buf, pos), pos+24
    if t == 'str':
        e = buf.find(b'\x00', pos)
        if e < 0: raise Fail
        s = buf[pos:e]
        try: s = s.decode('utf-8')
        except UnicodeDecodeError: raise Fail
        if any(ord(ch) < 32 and ch not in '\t\r\n' for ch in s): raise Fail
        return s, e+1
    if t == 'kv':
        out = []
        while True:
            if pos+2 > L: raise Fail
            k = struct.unpack_from('<H', buf, pos)[0]
            if k == 0xffff: return out, pos+2
            if pos+4 > L: raise Fail
            ix = struct.unpack_from('<H', buf, pos+2)[0]
            s, pos = rd(buf, pos+4, 'str'); out.append((k, ix, s))
    if t == 'arr2':
        out = []
        while True:
            if pos+2 > L: raise Fail
            v = struct.unpack_from('<H', buf, pos)[0]; pos += 2
            if v == 0xffff: return out, pos
            out.append(v)
    if t == 'arrd':
        out = []
        while True:
            if pos+2 > L: raise Fail
            if buf[pos:pos+2] == b'\xff\xff': return out, pos+2
            v, pos = rd(buf, pos, 'd8'); out.append(v)
    raise Fail

class Budget(Exception): pass

def parse_stream(buf, types, learn=True, budget=200000, tail=0):
    """Zweistufig: erst exakt das bisherige Verfahren (ein Abschnittstrenner, nur erlernte Typen); nur wenn das
    scheitert, mit belegten EPLAN-Typen als zweitem Versuch und bis zu ABSCHNITTE Trennern. So bleiben alle bisher
    lesbaren Sätze bitgleich."""
    try:
        return _parse_stream(buf, types, learn, budget, tail, erweitert=False)
    except (Fail, Budget):
        if not (EPLAN_TYP or ABSCHNITTE > 1):
            raise
        return _parse_stream(buf, types, learn, budget, tail, erweitert=True)


def _parse_stream(buf, types, learn=True, budget=200000, tail=0, erweitert=False):
    """Parst Eigenschaftsstrom; bei unbekannten IDs wird (falls learn) per Backtracking ein Typ gesucht.
    Gibt (props, neue_typen) zurück; props = Liste (pid, typ, wert) mit verschachtelten obj."""
    steps = [0]
    sep = [0]
    newt = {}
    def T(pid):
        return types.get(pid) or newt.get(pid)
    def seq(pos, term, acc, cont):
        steps[0] += 1
        if steps[0] > budget: raise Budget
        if pos + 2 > len(buf): raise Fail
        pid = struct.unpack_from('<H', buf, pos)[0]
        if pid == term:
            return cont(pos+2, acc)
        if pid == 0xffff and term == 0 and sep[0] < (ABSCHNITTE if erweitert else 1):
            # je Vererbungsstufe ein Abschnitt, getrennt durch ff ff (auch leere: ff ff ff ff); neuere Klassen
            # (Phoenix 170–185) haben bis zu vier
            sep[0] += 1
            try:
                return seq(pos+2, term, acc + [(0xffff, 'sep', None)], cont)
            except Fail:
                sep[0] -= 1; raise
        if pid in (0, 0xffff): raise Fail
        if pid == 0x0380:
            # Array-Kodierung: 80 03 | u32 X | u8 | u32 Anzahl | Anzahl x nullterminierter String
            if pos+11 > len(buf): raise Fail
            x = struct.unpack_from('<I', buf, pos+2)[0]; n = struct.unpack_from('<I', buf, pos+7)[0]
            if n > 512: raise Fail
            p2 = pos+11; vals = []
            for _ in range(n):
                v, p2 = rd(buf, p2, 'str'); vals.append(v)
            return seq(p2, term, acc + [(0x0380, 'arrS', (x, buf[pos+6], vals))], cont)
        if pid == 0x0381:
            # Zahlen-Array, gleicher Kopf wie 80 03: 81 03 | u32 X | u8 | u32 Anzahl | Anzahl x d8
            # (belegt im Normblatt-Satz, Form.eod Klasse 21: X = 0x5405, 10 x 42,0 = Spaltenbreiten FN1_001)
            if pos+11 > len(buf): raise Fail
            x = struct.unpack_from('<I', buf, pos+2)[0]; n = struct.unpack_from('<I', buf, pos+7)[0]
            if n > 4096 or pos+11+8*n > len(buf): raise Fail
            vals = list(struct.unpack_from('<%dd' % n, buf, pos+11))
            return seq(pos+11+8*n, term, acc + [(0x0381, 'arrD', (x, buf[pos+6], vals))], cont)
        t = T(pid)
        cands = [t] if t else (CANDS if learn else [])
        e_ = EPLAN_TYP.get(pid)
        if erweitert and t and e_ and e_ != t:
            # Typ je ID hängt von der Klasse ab (Arc ec8c double, anderswo EPoint): erlernter Typ zuerst, der belegte
            # EPLAN-Typ als zweiter Versuch – bisher lesbare Sätze bleiben unverändert
            cands = [t, e_]
        if t and learn and AUSWEICHTYPEN:
            # Typ je ID gilt nicht in jedem Objekt (Phoenix: eb03 in ee86-Anschlussobjekten Text „2,3,4“ statt b1):
            # scheitert der bekannte Typ, die übrigen probieren. Bisher lesbare Sätze bleiben gleich (bekannter Typ zuerst).
            cands = [t] + [c for c in CANDS if c != t]
        for c in cands:
            added = False
            if not t:
                newt[pid] = c; added = True
            try:
                if c == 'obj':
                    if pos+4 > len(buf): raise Fail
                    oc = struct.unpack_from('<H', buf, pos+2)[0]
                    if pos+6 <= len(buf):
                        nx = struct.unpack_from('<H', buf, pos+4)[0]
                        if not (nx >= 0xe000 or nx in (0x0380, 0x0381)): raise Fail
                    def after_obj(p2, sub, pid=pid, oc=oc):
                        return seq(p2, term, acc + [(pid, 'obj', (oc, sub))], cont)
                    return seq(pos+4, 0xffff, [], after_obj)
                if c == 'dN':
                    prevn = None
                    if pid == 0xeb1f:
                        for a in acc:
                            if a[0] == 0xeb1e: prevn = len(a[2])
                    ks = [prevn] if prevn is not None else range(0, 33)
                    for k in ks:
                        p2 = pos+2+8*k
                        if p2+2 > len(buf): break
                        nx = struct.unpack_from('<H', buf, p2)[0]
                        if not (nx >= 0xe000 or nx == 0): continue
                        v = list(struct.unpack_from('<%dd' % k, buf, pos+2))
                        if not all(x == 0.0 or abs(x) == float('inf') or 1e-300 < abs(x) < 1e7 or abs(abs(x) - 1.7976931348623204e+52) < 1e40 for x in v):
                            continue
                        try:
                            return seq(p2, term, acc + [(pid, c, v)], cont)
                        except Fail:
                            continue
                    raise Fail
                v, p2 = rd(buf, pos+2, c)
                if p2+2 <= len(buf):
                    nx = struct.unpack_from('<H', buf, p2)[0]
                    if not (nx >= 0xe000 or nx == 0 or nx in (0x0380, 0x0381)): raise Fail
                return seq(p2, term, acc + [(pid, c, v)], cont)
            except Fail:
                if added: del newt[pid]
                continue
        raise Fail
    def done(pos, acc):
        if any(buf[pos:len(buf)-tail]): raise Fail
        return acc
    props = seq(0, 0, [], done)
    return props, dict(newt)
