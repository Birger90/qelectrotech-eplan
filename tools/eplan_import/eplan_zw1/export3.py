import os
"""Kabelübersicht und Kabelplan (Ader -> Quelle/Ziel)."""
import sys, os, csv, collections, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from resolve3 import *          # P, F, C, pd, kvd, bmk
OUT = os.environ.get('EOD_EXPORT', 'out')
conn = {int(r['connection_id']): r for r in csv.DictReader(open(f'{OUT}/Verbindungen_aufgeloest.csv', encoding='utf-8-sig'), delimiter=';')}

# Kabelobjekte: Funktionen mit 20040 (Kabeltyp) oder 20042 (Aderanzahl), oder Ziel von ee67-Rückverweisen
ader_by_cable = collections.defaultdict(list)
aders = {}
for fid, e in F.items():
    if e['cls'] != 0x3b or not e['props']: continue
    d = pd(e)
    cab = d[0xee67][0][1] if 0xee67 in d else None
    k6 = kvd(d, 0xea6b)
    aders[fid] = {'kabel': cab, 'ader': k6.get((31004, 0), ''), 'querschnitt': k6.get((31002, 0), ''), 'bmk': bmk(fid)[0]}
    if cab: ader_by_cable[cab].append(fid)

cables = set(ader_by_cable)
for fid, e in F.items():
    if not e['props']: continue
    k6 = kvd(pd(e), 0xea6b)
    if (20040, 0) in k6 or (20042, 0) in k6: cables.add(fid)

# Verbindung -> Ader (eb33)
conn_by_ader = collections.defaultdict(list)
for cid, e in C.items():
    d = pd(e)
    if 0xeb33 in d: conn_by_ader[d[0xeb33][0][1]].append(cid)

uebersicht = []; plan = []; chk = collections.Counter()
for cab in sorted(cables, key=lambda x: bmk(x)[0] if x in F and F[x]['props'] else ''):
    e = F.get(cab)
    if not e or not e['props']: chk['Kabelobjekt nicht dekodiert'] += 1; continue
    d = pd(e); k6 = kvd(d, 0xea6b); b = bmk(cab)[0]
    al = ader_by_cable.get(cab, [])
    belegt = sum(1 for a in al if conn_by_ader.get(a))
    soll = k6.get((20042, 0), '')
    if soll.isdigit():
        chk['Aderobjekte <= 20042' if len(al) <= int(soll) else 'Aderobjekte > 20042 (Widerspruch)'] += 1
    b = b or '(ohne BMK)'
    uebersicht.append({'Kabel (BMK)': b, 'Typ (20040)': k6.get((20040, 0), ''), 'Aderanzahl (20042)': soll,
                       'Querschnitt (20043)': k6.get((20043, 0), ''), 'Spannung (20044)': k6.get((20044, 0), ''),
                       'Aderobjekte im Archiv': len(al), 'davon mit Verbindung': belegt, 'Funktion_id': cab})
    for a in sorted(al, key=lambda x: (len(aders[x]['ader']), aders[x]['ader'])):
        cs = conn_by_ader.get(a) or [None]
        for cid in cs:
            r = conn.get(cid, {})
            plan.append({'Kabel (BMK)': b, 'Typ (20040)': k6.get((20040, 0), ''), 'Ader (31004)': aders[a]['ader'],
                         'Querschnitt (31002)': aders[a]['querschnitt'] or r.get('31002 Querschnitt/Durchmesser', ''),
                         'Quelle': r.get('A_berechnet', ''), 'Ziel': r.get('B_berechnet', ''),
                         'Seite': r.get('seite', ''), 'Verbindungs-ID': cid or '', 'Ader_id': a})
            chk['Ader mit Verbindung' if cid else 'Ader ohne Verbindung (Reserve/unbelegt)'] += 1
for name, rows in (('Kabeluebersicht.csv', uebersicht), ('Kabelplan.csv', plan)):
    with open(f'{OUT}/{name}', 'w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()) if rows else ['leer'], delimiter=';'); w.writeheader(); w.writerows(rows)
summ = {'kabel': len(uebersicht), 'adern_zeilen': len(plan), 'pruefung': dict(chk)}
json.dump(summ, open(f'{OUT}/Zusammenfassung_Kabel.json', 'w'), indent=1, ensure_ascii=False)
print(json.dumps(summ, ensure_ascii=False))
