import os
"""Stückliste und Klemmenplan aus dem dekodierten Bestand."""
import sys, os, csv, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from resolve3 import *          # P, F, C, pd, kvd, bmk, pins, resolve_ep
OUT = os.environ.get('EOD_EXPORT', 'out'); os.makedirs(OUT, exist_ok=True)
A = P['Article.eod']; AR = P['ArticleRef.eod']; PG = P['Page.eod']

def ml(s, lang='de_DE'):
    """EPLAN-Mehrsprachentext 'de_DE@..;en_US@..;' -> deutscher Text, sonst erster."""
    if not s or '@' not in s: return s or ''
    parts = [p for p in s.split(';') if '@' in p]
    for p in parts:
        if p.startswith(lang + '@'): return p.split('@', 1)[1]
    return parts[0].split('@', 1)[1] if parts else s

def page_name(pid):
    e = PG.get(pid)
    if not e or not e['props']: return ''
    d = {}
    for p, t, v in e['props']:
        if t == 'kv' and p == 0xea91:
            for k, i, s in v: d[k] = s
    return ('=' + d[1100] if 1100 in d else '') + ('+' + d[1200] if 1200 in d else '') + ('/' + d[11012] if 11012 in d else '')

# Artikelstamm
stamm = {}; stamm_stat = collections.Counter()
for e in A.values():
    if not e['props']: stamm_stat['nicht dekodiert (' + str(e.get('err') or e.get('mode')) + ')'] += 1; continue
    k = kvd(pd(e), 0xea6b)
    pn = k.get((22001, 0))
    if not pn: stamm_stat['andere Satzart, Satzart 22023=%s' % k.get((22023, 0), '?')] += 1; continue
    stamm_stat['mit Artikelnummer'] += 1
    stamm[pn] = {'Artikelnummer (22001)': pn, 'Typnummer (22002)': k.get((22002, 0), ''), 'Bestellnummer (22003)': k.get((22003, 0), ''),
                 'Hersteller (22007)': k.get((22007, 0), ''), 'Bezeichnung 1 (22004)': ml(k.get((22004, 0), '')),
                 'Bezeichnung 2 (22005)': ml(k.get((22005, 0), '')), 'Technische Kenngrößen (22017)': k.get((22017, 0), ''),
                 'ERP-Nummer (22056)': k.get((22056, 0), '')}

rows = []; st = collections.Counter()
for e in AR.values():
    if not e['props']: st['Referenz nicht dekodiert'] += 1; continue
    d = pd(e)
    pn = d[0xef59][0] if 0xef59 in d else ''
    fid = d[0xef57][0][1] if 0xef57 in d else None
    b = bmk(fid)[0] if fid in F and F[fid]['props'] else ''
    pg = d[0xef60][0][1] if 0xef60 in d else None
    s = stamm.get(pn, {})
    if not pn: st['Referenz ohne Artikelnummer'] += 1
    elif s: st['Artikel im Stamm gefunden'] += 1
    else: st['Artikelnummer NICHT im Stamm'] += 1
    rows.append({'BMK (aus Funktion)': b, 'Funktion_id': fid or '', 'Seite': page_name(pg) if pg else '',
                 'Artikelnummer': pn, 'Menge (ef5b, Hypothese)': d[0xef5b][0] if 0xef5b in d else '',
                 'Stammsatz gefunden': 'ja' if s else ('-' if not pn else 'nein'),
                 **{k: v for k, v in s.items() if k != 'Artikelnummer (22001)'}})
rows.sort(key=lambda r: (r['BMK (aus Funktion)'], r['Artikelnummer']))
keys = []
for r in rows:
    for k in r:
        if k not in keys: keys.append(k)
with open(f'{OUT}/Stueckliste_je_BMK.csv', 'w', newline='', encoding='utf-8-sig') as f:
    w = csv.DictWriter(f, fieldnames=keys, delimiter=';'); w.writeheader(); w.writerows(rows)

agg = collections.OrderedDict()
for r in rows:
    if not r['Artikelnummer']: continue
    a = agg.setdefault(r['Artikelnummer'], {'Artikelnummer': r['Artikelnummer'], 'Anzahl Referenzen': 0, 'Summe Menge (Hypothese)': 0, 'BMKs': set(), **{k: v for k, v in stamm.get(r['Artikelnummer'], {}).items() if k != 'Artikelnummer (22001)'}})
    a['Anzahl Referenzen'] += 1
    a['Summe Menge (Hypothese)'] += int(r['Menge (ef5b, Hypothese)'] or 0)
    a['BMKs'].add(r['BMK (aus Funktion)'])
agl = []
for a in agg.values():
    a['BMKs'] = ', '.join(sorted(x for x in a['BMKs'] if x)); agl.append(a)
agl.sort(key=lambda a: a['Artikelnummer'])
keys = []
for r in agl:
    for k in r:
        if k not in keys: keys.append(k)
with open(f'{OUT}/Stueckliste_summiert.csv', 'w', newline='', encoding='utf-8-sig') as f:
    w = csv.DictWriter(f, fieldnames=keys, delimiter=';'); w.writeheader(); w.writerows(agl)

# Klemmenplan: Funktionen mit Eigenschaft 20030 (Klemmen-/Anschlussbezeichnung)
conn = list(csv.DictReader(open(f'{OUT}/Verbindungen_aufgeloest.csv', encoding='utf-8-sig'), delimiter=';'))
ziel = collections.defaultdict(list)
for r in conn:
    for s, o in (('A', 'B'), ('B', 'A')):
        if r[f'{s}_typ'] == 'Funktion' and r[f'{s}_objekt_id']:
            ziel[(int(r[f'{s}_objekt_id']), int(r[f'{s}_anschlussindex_0basiert'] or 0))].append(
                (r[f'{o}_berechnet'] or '[' + r[f'{o}_typ'] + ']', r['31004 Verbindungsfarbe/-nummer'], r['31002 Querschnitt/Durchmesser'], r['connection_id']))
krows = []
for fid, e in F.items():
    if not e['props']: continue
    d = pd(e); k6 = kvd(d, 0xea6b)
    if (20030, 0) not in k6: continue
    b, _ = bmk(fid); pl = pins(d)
    pg = d[0xebf2][0][1] if 0xebf2 in d else None
    for i, pn in enumerate(pl or ['']):
        z = ziel.get((fid, i), [])
        krows.append({'Leiste (BMK)': b, 'Klemme (20030)': k6[(20030, 0)], 'Anschlussindex': i, 'Anschluss': pn,
                      'Seite': page_name(pg) if pg else '', 'Funktion_id': fid,
                      'Anzahl Verbindungen': len(z),
                      'Ziele': ' | '.join(x[0] for x in z), 'Farbe/Nr (31004)': ' | '.join(x[1] for x in z),
                      'Querschnitt (31002)': ' | '.join(x[2] for x in z), 'Verbindungs-IDs': ' | '.join(x[3] for x in z)})
def nk(s):
    import re
    return [int(t) if t.isdigit() else t for t in re.split(r'(\d+)', s)]
krows.sort(key=lambda r: (r['Leiste (BMK)'], nk(r['Klemme (20030)']), r['Anschlussindex']))
with open(f'{OUT}/Klemmenplan.csv', 'w', newline='', encoding='utf-8-sig') as f:
    w = csv.DictWriter(f, fieldnames=list(krows[0].keys()) if krows else ['leer'], delimiter=';'); w.writeheader(); w.writerows(krows)

summ = {'artikelstamm': dict(stamm_stat), 'artikelreferenzen': dict(st), 'stueckliste_positionen': len(rows),
        'artikel_summiert': len(agl), 'klemmen_zeilen': len(krows),
        'klemmen_funktionen': len({r['Funktion_id'] for r in krows}), 'leisten': len({r['Leiste (BMK)'] for r in krows})}
json.dump(summ, open(f'{OUT}/Zusammenfassung_Stueckliste_Klemmen.json', 'w'), indent=1, ensure_ascii=False)
print(json.dumps(summ, ensure_ascii=False))
