import os
"""Export der unabhängig dekodierten Verbindungs-, Funktions- und Seitendaten."""
import sys, csv, collections, pickle, json, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from resolve3 import *          # C, F, I, Pot, P, pd, kvd, bmk, pins, resolve_ep, SIDES

OUT = os.environ.get('EOD_EXPORT', 'out'); os.makedirs(OUT, exist_ok=True)
_ip = os.environ.get('EOD_INS', 'ins130.pkl')
INS = pickle.load(open(_ip, 'rb')) if os.path.exists(_ip) else {}
PG = P['Page.eod']

# EPLAN-Eigenschaftsnummern mit Bedeutung laut EPLAN-API-Doku (eplan.help) - nur belegte
DOC = {31000: 'Länge (Wert)', 31001: 'Längeneinheit', 31002: 'Querschnitt/Durchmesser',
       31004: 'Verbindungsfarbe/-nummer', 31005: 'Autoconnecting line: Farbe',
       31006: 'Potenzialtyp', 31018: 'Autoconnecting line: Musterlänge',
       31048: 'Typbezeichnung', 31059: 'Einheit Querschnitt', 20030: 'Klemmen-/Anschlussbezeichnung'}

def page_name(pid):
    e = PG.get(pid)
    if not e or not e['props']: return ''
    d = {}
    for p, t, v in e['props']:
        if t == 'kv' and p == 0xea91:
            for k, i, s in v: d[k] = s
    return ('=' + d[1100] if 1100 in d else '') + ('+' + d[1200] if 1200 in d else '') + ('/' + d[11012] if 11012 in d else '')

def ipoint_name(tid):
    e = I.get(tid)
    if not e or not e['props']: return None
    fd = pd(e); kv = kvd(fd, 0xeaf6)
    s = ('=' + kv[(1100, 0)] if (1100, 0) in kv else '') + ('+' + kv[(1200, 0)] if (1200, 0) in kv else '')
    return s + '-' + kv.get((20013, 0), '') + kv.get((20014, 0), '')

def ins_text(tid):
    p = INS.get(tid)
    if not p: return None
    for pid, t, v in p:
        if t == 'kv' and pid == 0xea6b:
            for k, i, s in v:
                if k == 19071: return s
    return None

rows = []; stat = collections.Counter()
for cid in sorted(C):
    e = C[cid]; d = pd(e)
    kv = kvd(d, 0xea6b); kvb = kvd(d, 0xeaf6)
    pg = d[0xeb2a][0][1] if 0xeb2a in d else (d[0xebf2][0][1] if 0xebf2 in d else None)
    row = {'connection_id': cid, 'seite': page_name(pg) if pg else '', 'seite_id': pg or '',
           'hinweis': '' if (pg and pg in PG) else 'Seite fehlt im Archiv (verwaiste Verbindung)'}
    for s, lab in (('A', 'A'), ('B', 'B')):
        ep = resolve_ep(d, s)
        k = ep['kind']; calc = ep.get('calc')
        if k == 'IPoint': calc = ipoint_name(ep['refid'])
        if k.startswith('Instance'): t = ins_text(ep['refid']); calc = ('[Objekt Klasse 130, Text 19071=%s]' % t) if t else None
        txt = ep['txt']
        if txt is None: ver = 'kein EPLAN-Text vorhanden'
        elif calc == txt: ver = 'identisch mit EPLAN-Text'
        else: ver = 'ABWEICHUNG'
        stat[(s, k, ver)] += 1
        row.update({f'{lab}_typ': k, f'{lab}_objekt_id': ep.get('refid', ''), f'{lab}_anschlussindex_0basiert': ep.get('pinidx', ''),
                    f'{lab}_berechnet': calc or '', f'{lab}_eplan_text': txt or '', f'{lab}_pruefung': ver})
    for num, name in DOC.items():
        if num < 31000: continue
        row[f'{num} {name}'] = kv.get((num, 0), '')
    row['Verbindungs-BMK (eaf6, Bedeutung unbelegt)'] = ('-' + kvb.get((20013, 0), '') + kvb.get((20014, 0), '')) if kvb else ''
    rows.append(row)

with open(f'{OUT}/Verbindungen_aufgeloest.csv', 'w', newline='', encoding='utf-8-sig') as f:
    w = csv.DictWriter(f, fieldnames=list(rows[0].keys()), delimiter=';'); w.writeheader(); w.writerows(rows)

# Funktionen
frows = []
for fid in sorted(F):
    e = F[fid]
    if not e['props']:
        frows.append({'funktion_id': fid, 'klasse': e['cls'], 'status': 'nicht dekodiert: ' + str(e['err'])}); continue
    b, fd = bmk(fid); pl = pins(fd); k6 = kvd(fd, 0xea6b)
    pg = fd[0xebf2][0][1] if 0xebf2 in fd else None
    frows.append({'funktion_id': fid, 'klasse': e['cls'], 'status': 'dekodiert', 'bmk': b,
                  'anschluesse': ' | '.join(pl), 'anzahl_anschluesse': len(pl),
                  'steckerbez_eb22': fd[0xeb22][0] if 0xeb22 in fd else '',
                  '20030 Klemmen-/Anschlussbezeichnung': k6.get((20030, 0), ''),
                  '20406 (Bedeutung unbelegt)': k6.get((20406, 0), ''),
                  '20468 (Makro-Pfad, Bedeutung unbelegt)': k6.get((20468, 0), ''),
                  'seite': page_name(pg) if pg else '', 'seite_id': pg or ''})
keys = []
for r in frows:
    for k in r:
        if k not in keys: keys.append(k)
with open(f'{OUT}/Funktionen_dekodiert.csv', 'w', newline='', encoding='utf-8-sig') as f:
    w = csv.DictWriter(f, fieldnames=keys, delimiter=';'); w.writeheader(); w.writerows(frows)

# Seiten
prow = [{'seite_id': pid, 'seite': page_name(pid), 'status': 'dekodiert' if PG[pid]['props'] else 'nicht dekodiert'} for pid in sorted(PG)]
with open(f'{OUT}/Seiten_dekodiert.csv', 'w', newline='', encoding='utf-8-sig') as f:
    w = csv.DictWriter(f, fieldnames=['seite_id', 'seite', 'status'], delimiter=';'); w.writeheader(); w.writerows(prow)

# Netze: Union-Find nur über Connection-Datensätze (Fakt). Variante 2 zusätzlich: gleichnamige IPoints verbunden (Hypothese)
def nets(merge_ip):
    par = {}
    def f(x):
        par.setdefault(x, x)
        while par[x] != x: par[x] = par[par[x]]; x = par[x]
        return x
    def u(a, b): par[f(a)] = f(b)
    label = {}
    for r in rows:
        nodes = []
        for s in 'AB':
            if r[f'{s}_objekt_id'] == '': continue
            if r[f'{s}_typ'] == 'IPoint' and merge_ip and r[f'{s}_berechnet']:
                n = ('IP', r[f'{s}_berechnet'])
            else:
                n = (r[f'{s}_typ'], r[f'{s}_objekt_id'], r[f'{s}_anschlussindex_0basiert'])
            label[n] = r[f'{s}_berechnet'] or str(n); nodes.append(n)
        if len(nodes) == 2: u(*nodes)
        elif nodes: f(nodes[0])
    groups = collections.defaultdict(list)
    for n in par: groups[f(n)].append(label[n])
    return groups
g1 = nets(False); g2 = nets(True)
with open(f'{OUT}/Netze.csv', 'w', newline='', encoding='utf-8-sig') as f:
    w = csv.writer(f, delimiter=';'); w.writerow(['variante', 'netz_nr', 'anzahl_knoten', 'knoten'])
    for name, g in (('nur Verbindungen (Fakt)', g1), ('plus gleichnamige IPoints (Hypothese)', g2)):
        for i, (k, v) in enumerate(sorted(g.items(), key=lambda x: -len(x[1]))):
            w.writerow([name, i + 1, len(v), ' | '.join(sorted(set(map(str, v))))])

summary = {'stat': {' / '.join(k): v for k, v in sorted(stat.items())},
           'netze_fakt': len(g1), 'netze_mit_ipoint_hypothese': len(g2),
           'funktionen_dekodiert': sum(1 for r in frows if r['status'] == 'dekodiert'), 'funktionen_gesamt': len(frows),
           'seiten_dekodiert': sum(1 for r in prow if r['status'] == 'dekodiert')}
json.dump(summary, open(f'{OUT}/Zusammenfassung.json', 'w'), indent=1, ensure_ascii=False)
print(json.dumps(summary, indent=1, ensure_ascii=False))
