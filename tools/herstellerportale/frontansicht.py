"""Aus einem Aufbau-Bauteil, das der DXF-Import aus einem ganzen Massbild (Blatt 1:1 mit
mehreren Ansichten) gebaut hat, die Frontansicht herausloesen.
Aufruf: frontansicht.py <ein.elmt> <aus.elmt> <Breite_mm laut Artikeldaten> <Hoehe_mm>
Die Ansichten sind zusammenhaengende Liniengruppen; genommen wird die Gruppe, deren Hoehe zur
Artikelhoehe passt und deren Breite der Artikelbreite am naechsten liegt. Die Masse im Bauteil
(width_mm/height_mm) stammen aus der Zeichnung (Huellrechteck der Ansicht, 2,5 px je mm)."""
import sys, math
import xml.etree.ElementTree as ET
src, dst, want_w, want_h = sys.argv[1], sys.argv[2], float(sys.argv[3]), float(sys.argv[4])
K = 2.5
tree = ET.parse(src); d = tree.getroot(); desc = d.find('description')
def box(e):
    f = lambda a: float(e.get(a))
    if e.tag == 'line': return (min(f('x1'), f('x2')), min(f('y1'), f('y2')), max(f('x1'), f('x2')), max(f('y1'), f('y2')))
    if e.tag in ('ellipse', 'arc', 'rect'): return (f('x'), f('y'), f('x') + f('width'), f('y') + f('height'))
    if e.tag == 'circle': return (f('x'), f('y'), f('x') + f('diameter'), f('y') + f('diameter'))
    if e.tag == 'polygon':
        xs = []; ys = []; i = 1
        while e.get('x%d' % i) is not None: xs.append(f('x%d' % i)); ys.append(f('y%d' % i)); i += 1
        return (min(xs), min(ys), max(xs), max(ys))
    return None
items = [(e, box(e)) for e in desc if box(e) is not None]
sheet_w = max(b[2] for _, b in items) - min(b[0] for _, b in items)
# Gruppen ueber sich beruehrende Huellrechtecke (Raster, damit es schnell bleibt)
parent = list(range(len(items)))
def find(i):
    while parent[i] != i: parent[i] = parent[parent[i]]; i = parent[i]
    return i
T = 0.6; C = 20.0; grid = {}
for i, (_, b) in enumerate(items):
    if b[2] - b[0] > 0.5 * sheet_w: continue        # Blattrahmen, Schriftfeld
    for gx in range(int((b[0] - T) // C), int((b[2] + T) // C) + 1):
        for gy in range(int((b[1] - T) // C), int((b[3] + T) // C) + 1):
            grid.setdefault((gx, gy), []).append(i)
for cell in grid.values():
    for a in range(len(cell)):
        ba = items[cell[a]][1]
        for c in range(a + 1, len(cell)):
            bb = items[cell[c]][1]
            if ba[0] - T <= bb[2] and bb[0] - T <= ba[2] and ba[1] - T <= bb[3] and bb[1] - T <= ba[3]:
                ra, rc = find(cell[a]), find(cell[c])
                if ra != rc: parent[ra] = rc
groups = {}
for i, (_, b) in enumerate(items):
    if b[2] - b[0] > 0.5 * sheet_w: continue
    g = groups.setdefault(find(i), [1e9, 1e9, -1e9, -1e9, []])
    g[0] = min(g[0], b[0]); g[1] = min(g[1], b[1]); g[2] = max(g[2], b[2]); g[3] = max(g[3], b[3]); g[4].append(i)
# Gruppen, die ganz in einer anderen liegen, gehoeren zu deren Ansicht
gl = sorted(groups.values(), key=lambda g: -(g[2] - g[0]) * (g[3] - g[1]))
views = []
for g in gl:
    for v in views:
        if g[0] >= v[0] - 1 and g[1] >= v[1] - 1 and g[2] <= v[2] + 1 and g[3] <= v[3] + 1:
            v[4].extend(g[4]); break
    else:
        views.append(g)
cand = []
for v in views:
    w, h = (v[2] - v[0]) / K, (v[3] - v[1]) / K
    print('Ansicht %.1f x %.1f mm, %d Objekte' % (w, h, len(v[4])))
    if abs(h - want_h) <= 0.15 * want_h: cand.append((abs(w - want_w), v))
if not cand: sys.exit('keine Ansicht mit passender Hoehe')
cand.sort(key=lambda c: c[0]); v = cand[0][1]
w_mm, h_mm = (v[2] - v[0]) / K, (v[3] - v[1]) / K
print('gewaehlt: %.2f x %.2f mm (Artikeldaten %.0f x %.0f)' % (w_mm, h_mm, want_w, want_h))
keep = set(id(items[i][0]) for i in v[4])
dx, dy = -v[0], -v[1]
for e in list(desc):
    if e.tag == 'dynamic_text':
        e.set('x', '0'); e.set('y', '-20'); continue
    if id(e) not in keep: desc.remove(e); continue
    for a in list(e.attrib):
        if a in ('x', 'x1', 'x2') or (a[0] == 'x' and a[1:].isdigit()): e.set(a, '%.2f' % (float(e.get(a)) + dx))
        if a in ('y', 'y1', 'y2') or (a[0] == 'y' and a[1:].isdigit()): e.set(a, '%.2f' % (float(e.get(a)) + dy))
up = lambda x: int(math.ceil(x / 10.0) * 10)
d.set('hotspot_x', '10'); d.set('hotspot_y', '30')
d.set('width', str(up(v[2] - v[0] + 20))); d.set('height', str(up(v[3] - v[1] + 40)))
infos = d.find('elementInformations')
for name, val in (('width_mm', w_mm), ('height_mm', h_mm)):
    for i in list(infos):
        if i.get('name') == name: infos.remove(i)
    n = ET.SubElement(infos, 'elementInformation', {'name': name, 'show': '0'}); n.text = ('%.4g' % val).replace('.', ',')
tree.write(dst, encoding='utf-8', xml_declaration=False)
