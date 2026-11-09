"""Excel je Projekt aus den CSV-Exporten."""
import sys, csv, json, re, os
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

def read(p):
    if not os.path.exists(p): return [], []
    with open(p, encoding='utf-8-sig') as f:
        r = list(csv.reader(f, delimiter=';'))
    return (r[0], r[1:]) if r else ([], [])

F = 'Arial'
HFILL = PatternFill('solid', start_color='1F3864')
def conv(v):
    if v is None: return ''
    if re.fullmatch(r'-?\d{1,9}', v) and not (len(v) > 1 and v.startswith('0')): return int(v)
    return v

def add_sheet(wb, name, path, widths=None):
    hdr, rows = read(path)
    ws = wb.create_sheet(name)
    if not hdr: ws['A1'] = 'keine Daten'; return ws, 0
    ws.append(hdr)
    for r in rows: ws.append([conv(v) for v in r])
    for row in ws.iter_rows(min_row=2):
        for c in row:
            if isinstance(c.value, str) and c.value.startswith('='):
                c.data_type = 's'   # Daten wie '=EA+CA/40' sind Text, keine Formel
    for c in ws[1]:
        c.font = Font(name=F, bold=True, color='FFFFFF'); c.fill = HFILL
        c.alignment = Alignment(wrap_text=True, vertical='top')
    for row in ws.iter_rows(min_row=2):
        for c in row: c.font = Font(name=F, size=10)
    ws.freeze_panes = 'A2'
    ws.auto_filter.ref = ws.dimensions
    for i, h in enumerate(hdr, 1):
        sample = [len(str(r[i-1])) for r in rows[:300] if i-1 < len(r)]
        w = min(60, max(10, len(h) * 0.9, (sorted(sample)[int(len(sample)*0.9)] if sample else 10) + 2))
        ws.column_dimensions[get_column_letter(i)].width = w
    ws.row_dimensions[1].height = 42
    return ws, len(rows)

def build(proj_dir, name, version, out):
    o = f'{proj_dir}/out'
    wb = Workbook(); info = wb.active; info.title = 'Info'
    sheets = [('Verbindungen', 'Verbindungen_aufgeloest.csv'), ('Klemmenplan', 'Klemmenplan.csv'),
              ('Kabeluebersicht', 'Kabeluebersicht.csv'), ('Kabelplan', 'Kabelplan.csv'),
              ('Stueckliste', 'Stueckliste_je_BMK.csv'), ('Stueckliste_summiert', 'Stueckliste_summiert.csv'),
              ('Funktionen', 'Funktionen_dekodiert.csv'), ('Seiten', 'Seiten_dekodiert.csv'), ('Netze', 'Netze.csv')]
    counts = {}
    for sn, fn in sheets:
        ws, n = add_sheet(wb, sn, f'{o}/{fn}'); counts[sn] = n
    # Spaltenbuchstaben für Formeln im Info-Blatt
    vh, vr = read(f'{o}/Verbindungen_aufgeloest.csv')
    col = lambda h: get_column_letter(vh.index(h) + 1)
    n = len(vr) + 1
    VA, VB = col('A_pruefung'), col('B_pruefung')
    BA, BB = col('A_berechnet'), col('B_berechnet')
    lines = [
        ('EPLAN-Projekt', name), ('EPLAN-Version (_BAKINFO.XML)', version),
        ('Quelle', 'unabhängige Dekodierung der .zw1-Sicherung, ohne EPLAN'), (None, None),
        ('Prüfung Verbindungen', None),
        ('Verbindungen gesamt', f'=COUNTA(Verbindungen!A2:A{n})'),
        ('beidseitig aufgelöst', f'=SUMPRODUCT((Verbindungen!{BA}2:{BA}{n}<>"")*(Verbindungen!{BB}2:{BB}{n}<>""))'),
        ('Enden mit EPLAN-eigenem Text', f'=COUNTIF(Verbindungen!{VA}2:{VA}{n},"<>kein EPLAN-Text vorhanden")+COUNTIF(Verbindungen!{VB}2:{VB}{n},"<>kein EPLAN-Text vorhanden")'),
        ('davon identisch mit Berechnung', f'=COUNTIF(Verbindungen!{VA}2:{VA}{n},"identisch mit EPLAN-Text")+COUNTIF(Verbindungen!{VB}2:{VB}{n},"identisch mit EPLAN-Text")'),
        ('Übereinstimmungsquote', '=IF(B8=0,0,B9/B8)'),
        (None, None), ('Umfang je Blatt (Zeilen)', None)]
    for sn, _ in sheets: lines.append((sn, f'=COUNTA({sn}!A2:A1048576)'))
    lines += [(None, None), ('Legende', None),
        ('BELEGT', 'Bedeutung der Eigenschaftsnummer laut EPLAN-API-Doku (eplan.help): z. B. 31002 Querschnitt, 31004 Verbindungsfarbe/-nummer, 20030 Klemmen-/Anschlussbezeichnung, 20040 Kabeltyp, 20042 Aderanzahl, 20043 Kabelquerschnitt, 22001 Artikelnummer, 22003 Bestellnummer, 22007 Hersteller'),
        ('GEPRÜFT', 'Spalten *_berechnet werden aus Rohverweisen berechnet und gegen EPLAN-eigene Texte (*_eplan_text) verglichen, Ergebnis in *_pruefung'),
        ('HYPOTHESE', 'Spalte „Menge (ef5b, Hypothese)“ in der Stückliste; Netze-Variante „plus gleichnamige IPoints“; Spalten mit „Bedeutung unbelegt“'),
        ('ABWEICHUNG', 'leeres Strukturpräfix („=“, „=+“) ist rein formal; „Verweis ins Leere“ = Objekt-ID fehlt im Archiv; vereinzelt veraltete EPLAN-Texte (Gerät im Archiv nicht vorhanden)'),
        ('Klemmenplan', 'enthält alle Funktionen mit Eigenschaft 20030 – kann neben Reihenklemmen auch Geräteanschlüsse mit Klemmenbezeichnung enthalten')]
    for i, (a, b) in enumerate(lines, 1):
        if a is None: continue
        info.cell(i, 1, a).font = Font(name=F, bold=b is None or a in ('BELEGT', 'GEPRÜFT', 'HYPOTHESE', 'ABWEICHUNG'))
        if b is not None:
            c = info.cell(i, 2, b); c.font = Font(name=F); c.alignment = Alignment(wrap_text=True, vertical='top')
    info['B10'].number_format = '0.0%'
    info.column_dimensions['A'].width = 34; info.column_dimensions['B'].width = 110
    assert info['A10'].value == 'Übereinstimmungsquote'
    wb.save(out)
    return counts

if __name__ == '__main__':
    proj_dir, name, version, out = sys.argv[1:5]
    print(build(proj_dir, name, version, out))
