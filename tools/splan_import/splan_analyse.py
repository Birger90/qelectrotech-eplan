"""sPlan-Blatt -> Inhalt: Bauteile mit Anschlüssen, Netze, Potentialnamen, freie Texte, Bilder.

sPlan kennt keine Anschlüsse und keine Netze (Hilfe „Was ist ein Bauteil?“: ein Bauteil ist eine Gruppe von
Zeichenelementen mit Bezeichner und Wert; „Lötpunkte“ zeigen an, ob sich kreuzende Linien verbunden sind).
Die Verbindungen werden deshalb aus der Geometrie abgeleitet (FORMATBESCHREIBUNG_SPL8.md §12):

- Leitung = Linienzug auf oberster Ebene (nicht Teil eines Bauteils).
- Zwei Leitungen sind verbunden, wenn ein Ende der einen auf der anderen liegt (T-Stoß, Ecke) oder ein Lötpunkt
  auf beiden liegt. Reine Kreuzungen ohne Lötpunkt sind nicht verbunden.
- Anschluss = Leitungsende, das auf der Zeichnung eines Bauteils endet. Name: Kontakt des Bauteils
  (sPlan-„Kontakt“), sonst der nächste Text im Bauteil, sonst die Lage (oben/unten).
- Potentialname = Text am Pfeilende einer Leitung (Pfeil als Linienende §6 oder als kleines Dreieck §7).
Alles, was nicht eindeutig ist, landet in `fragen` (wird zu Offene_Fragen.txt).
"""
import math
import re

from splan8 import walk

TOL = 3.0                 # 0,3 mm: Punkt liegt auf Linie
RAND = (75, 75, 2865, 1752)   # Zeichenfläche: innerer Rahmen liegt bei 7,2 mm, Schriftfeld ab 175 mm (0,1 mm)


def _abst_seg(p, a, b):
    ax, ay = a
    bx, by = b
    dx, dy = bx - ax, by - ay
    l2 = dx * dx + dy * dy
    t = 0 if l2 == 0 else max(0.0, min(1.0, ((p[0] - ax) * dx + (p[1] - ay) * dy) / l2))
    return math.hypot(p[0] - ax - t * dx, p[1] - ay - t * dy)


def _segmente(o):
    p = o["pts"]
    if o["art"] in ("linie", "bezier"):
        return list(zip(p, p[1:]))
    if o["art"] in ("rechteck", "polygon", "ellipse"):
        return list(zip(p, p[1:] + p[:1]))
    return []


def _bbox(pts):
    xs, ys = [p[0] for p in pts], [p[1] for p in pts]
    return (min(xs), min(ys), max(xs), max(ys))


def _mitte(pts):
    b = _bbox(pts)
    return ((b[0] + b[2]) / 2, (b[1] + b[3]) / 2)


def im_rahmen(o):
    """Rahmen, Spalten-/Zeilenbeschriftung und Schriftfeld liegen außerhalb der Zeichenfläche."""
    pts = [q for k in walk(o) for q in k["pts"]]
    if not pts:
        return False
    def draussen(p):
        return p[0] <= RAND[0] + 5 or p[0] >= RAND[2] - 5 or p[1] <= RAND[1] + 5 or p[1] >= RAND[3] - 5
    if o["art"] in ("linie", "polygon"):
        # Rahmenlinien (auch die Zickzack-Teilung der Spalten) liegen samt Segmentmitten im Randstreifen
        mitten = [((a[0] + b[0]) / 2, (a[1] + b[1]) / 2) for a, b in zip(pts, pts[1:])]
        return all(draussen(p) for p in pts + mitten)
    # Flächen (Bild, Rechteck, Gruppe, Text): Rahmen nur, wenn die Mitte im Randstreifen liegt
    return draussen(_mitte(pts))


def sauber(s):
    """Steuerzeichen (sPlan speichert z. B. das Byte 07 als Bibliotheksnamen) sind in XML nicht erlaubt."""
    return re.sub(r"[\x00-\x08\x0b-\x1f]", "", s or "").strip()


class Bauteil:
    def __init__(self, o, nr):
        self.o, self.nr = o, nr
        self.bez = sauber(o.get("bez"))
        self.wert = sauber(o.get("wert"))
        self.symbol = sauber((o.get("symbol") or "").split("\r")[0])
        self.id, self.parent = o.get("id", 0), o.get("parent", 0)
        self.ist_parent = bool(o.get("ist_parent"))
        self.ist_gruppe = False
        self.ist_bild = False
        self.segs = [s for k in walk(o) if k is not o for s in _segmente(k)]
        pts = [q for k in walk(o) if k is not o and k["art"] not in ("text", "kontakt") for q in k["pts"]]
        self.bbox = _bbox(pts) if pts else _bbox(o["bez_text"]["pts"] if "bez_text" in o else [(0, 0)])
        self.kontakte = [(k.get("name", ""), k["text"], _mitte(k["pts"])) for k in walk(o) if k["art"] == "kontakt"]
        self.texte = []
        for k in walk(o):
            if k["art"] in ("text", "mengentext") and k is not o:
                self.texte += _text_teile(k)
        self.zusatz = [z for z in o.get("zusatz", []) if z]
        self.anschluesse = []     # [Anschluss]
        self.beschreibung = []    # freie Texte daneben
        self.leiste = None        # Klemmenleiste (bei Klemmen)
        self.art = None           # Funktion (klassifiziere)

    def beruehrt(self, p):
        """Abstand des Punkts zur Bauteilzeichnung (0 = auf einer Linie oder im Rahmen)."""
        d = min((_abst_seg(p, a, b) for a, b in self.segs), default=1e9)
        x0, y0, x1, y1 = self.bbox
        if x0 - 1 <= p[0] <= x1 + 1 and y0 - 1 <= p[1] <= y1 + 1:
            d = min(d, 1.0)
        return d

    @property
    def mitte(self):
        return ((self.bbox[0] + self.bbox[2]) / 2, (self.bbox[1] + self.bbox[3]) / 2)

    def __repr__(self):
        return f"<{self.bez or '?'} {self.symbol!r} {len(self.anschluesse)}A>"


def _text_teile(t):
    """Text eines Bauteils in Anschlussnamen zerlegen: „PE I L I N“ (Klemmenbeschriftung eines Netzteils) wird zu
    PE, L, N an ihrer Lage im Text; ein Wort bleibt ein Wort (Lage = Textmitte)."""
    txt = t["text"]
    worte = [(m.start(), m.group(0)) for m in re.finditer(r"\S+", txt)]
    worte = [(k, w) for k, w in worte if w not in ("I", "|")]
    if len(worte) <= 1 or "\n" in txt or len(worte) > 12:
        return [(txt, _mitte(t["pts"]))]
    p0, p1, p3 = t["pts"][0], t["pts"][1], t["pts"][3]
    n = max(1, len(txt))
    erg = []
    for k, w in worte:
        f = (k + len(w) / 2) / n
        erg.append((w, (p0[0] + (p1[0] - p0[0]) * f + (p3[0] - p0[0]) / 2,
                        p0[1] + (p1[1] - p0[1]) * f + (p3[1] - p0[1]) / 2)))
    return erg


def seite(bt, p):
    cx, cy = bt.mitte
    x0, y0, x1, y1 = bt.bbox
    dx, dy = p[0] - cx, p[1] - cy
    rx, ry = abs(dx) / max(1, x1 - x0), abs(dy) / max(1, y1 - y0)
    return ("unten" if dy > 0 else "oben") if ry >= rx else ("rechts" if dx > 0 else "links")


def ist_klemme(bt):
    return bool(re.search(r"klemme", bt.symbol, re.I))


def _gleicher_anschluss(bt, a, e):
    """Klemmen: alle Leitungen einer Seite landen auf derselben Schraube; mehrstöckige „Klemmen“ je Spalte."""
    if ist_klemme(bt):
        if (a.pos[1] < bt.mitte[1]) != (e[1] < bt.mitte[1]):       # Reihenklemme: nur oben/unten
            return False
        if re.search(r"klemmen", bt.symbol, re.I):
            return abs(a.pos[0] - e[0]) <= 8
        return True
    return math.dist(a.pos, e) <= 4


class Anschluss:
    def __init__(self, bt, pos):
        self.bt, self.pos, self.name, self.netz = bt, pos, "", None
        self.seite = ""           # oben/unten/links/rechts relativ zum Bauteil
        self.leitung = None

    def __repr__(self):
        return f"{self.bt.bez}:{self.name}"


class Netz:
    def __init__(self, nr):
        self.nr, self.leitungen, self.anschluesse, self.namen, self.pfeile = nr, [], [], [], []

    @property
    def name(self):
        return self.namen[0] if self.namen else ""


class Inhalt:
    """Ergebnis je Blatt."""

    def __init__(self, nummer, titel):
        self.nummer, self.titel = nummer, titel
        self.bauteile, self.netze, self.texte, self.bilder, self.grafik = [], [], [], [], []
        self.fragen = []


def _union(parent, a, b):
    a, b = _wurzel(parent, a), _wurzel(parent, b)
    if a != b:
        parent[b] = a


def _wurzel(parent, a):
    while parent[a] != a:
        parent[a] = parent[parent[a]]
        a = parent[a]
    return a


def analysiere(blatt, nummer):
    inh = Inhalt(nummer, blatt.name)
    leit, kleine_dreiecke, punkte, freie_texte = [], [], [], []
    for o in blatt.objekte:
        if im_rahmen(o):
            continue
        a = o["art"]
        if a == "bauteil":
            inh.bauteile.append(Bauteil(o, len(inh.bauteile)))
        elif a == "linie":
            leit.append(o)
        elif a == "polygon" and max(_bbox(o["pts"])[2] - _bbox(o["pts"])[0],
                                    _bbox(o["pts"])[3] - _bbox(o["pts"])[1]) <= 30:
            kleine_dreiecke.append(o)
        elif a == "punkt":
            punkte.append(o["pts"][0])
        elif a in ("text", "mengentext"):
            if o["text"].strip():
                freie_texte.append(o)
        elif a == "bild":
            inh.bilder.append(o)
        else:                                      # Gruppen (Clips), Rechtecke, Ellipsen, große Polygone
            inh.grafik.append(o)

    # Gruppen ohne Bauteil-Daten (Clips, z. B. Ventilteile) zählen als unbenanntes Bauteil
    for g in [g for g in inh.grafik if g["art"] == "gruppe"]:
        bt = Bauteil(dict(g, bez="", wert="", symbol="Gruppe"), len(inh.bauteile))
        bt.ist_gruppe = True
        inh.bauteile.append(bt)
    inh.grafik = [g for g in inh.grafik if g["art"] != "gruppe"]

    # Eingefügte Bilder (Gerätefotos, Relais als Bild), auf denen Leitungen enden, sind Geräte ohne Daten
    for b in inh.bilder:
        bt = Bauteil(dict(b, kinder=[dict(art="rechteck", pts=b["pts"] + b["pts"][:1], kinder=[])],
                          bez="", wert="", symbol="Bild"), len(inh.bauteile))
        bt.ist_bild = True
        b["_bauteil"] = bt
        inh.bauteile.append(bt)

    # --- Leitungen verbinden -------------------------------------------------------------------------
    n = len(leit)
    parent = list(range(n))
    segs = [_segmente(o) for o in leit]
    enden = [(o["pts"][0], o["pts"][-1]) for o in leit]
    for i in range(n):
        for e in enden[i]:
            for j in range(n):
                if i != j and any(_abst_seg(e, a, b) <= TOL for a, b in segs[j]):
                    _union(parent, i, j)
    for p in punkte:
        auf = [i for i in range(n) if any(_abst_seg(p, a, b) <= TOL for a, b in segs[i])]
        for i in auf[1:]:
            _union(parent, auf[0], i)
        if not auf:
            inh.fragen.append(f"Lötpunkt bei {p[0] / 10:.0f}/{p[1] / 10:.0f} mm liegt auf keiner Leitung.")

    # --- Pfeilenden --------------------------------------------------------------------------------
    pfeil_enden = []           # (leitung, punkt, vorletzter punkt)
    for i, o in enumerate(leit):
        if o.get("pfeil_anfang"):
            pfeil_enden.append((i, o["pts"][0], o["pts"][1]))
        if o.get("pfeil_ende"):
            pfeil_enden.append((i, o["pts"][-1], o["pts"][-2]))
    for d in kleine_dreiecke:
        m = _mitte(d["pts"])
        best = min(((math.dist(m, e), i, k) for i in range(n) for k, e in enumerate(enden[i])), default=None)
        if best and best[0] <= 25:
            i, k = best[1], best[2]
            if not any(j == i and math.dist(p, enden[i][k]) < 1 for j, p, _ in pfeil_enden):
                pfeil_enden.append((i, enden[i][k], leit[i]["pts"][1 if k == 0 else -2]))

    # --- Anschlüsse ----------------------------------------------------------------------------------
    def auf_leitung(e, i):
        return any(j != i and any(_abst_seg(e, a, b) <= TOL for a, b in segs[j]) for j in range(n))

    for i in range(n):
        for e in enden[i]:
            kand = sorted(((bt.beruehrt(e), bt.nr, bt) for bt in inh.bauteile), key=lambda t: t[:2])
            if not kand or kand[0][0] > TOL:
                continue
            bt = kand[0][2]
            vorh = next((a for a in bt.anschluesse if _gleicher_anschluss(bt, a, e)), None)
            if vorh is None:
                vorh = Anschluss(bt, e)
                bt.anschluesse.append(vorh)
            if vorh.leitung is None:
                vorh.leitung = i
            else:
                _union(parent, vorh.leitung, i)

    # --- Netze -------------------------------------------------------------------------------------
    gruppen = {}
    for i in range(n):
        gruppen.setdefault(_wurzel(parent, i), []).append(i)
    for k, idx in enumerate(gruppen.values()):
        nz = Netz(k)
        nz.leitungen = [leit[i] for i in idx]
        inh.netze.append(nz)
        for i in idx:
            leit[i]["_netz"] = nz
    for bt in inh.bauteile:
        for a in bt.anschluesse:
            a.netz = leit[a.leitung]["_netz"]
            a.netz.anschluesse.append(a)

    # --- Texte zuordnen: Pfeiltexte, Anschlussnamen, Beschreibung, frei --------------------------
    benutzt = set()

    def naechster_text(p, maxd):
        best = None
        for t in freie_texte:
            if id(t) in benutzt:
                continue
            q = t["pts"]
            d = min(_abst_seg(p, a, b) for a, b in zip(q, q[1:] + q[:1]))
            if d <= maxd and (best is None or d < best[0]):
                best = (d, t)
        return best[1] if best else None

    # Leitungsenden, die einen Namen tragen können: Pfeilenden und freie Enden (kein Bauteil, keine Leitung)
    namens_enden = [(i, p, vor, True) for i, p, vor in pfeil_enden]
    for i in range(n):
        for k, e in enumerate(enden[i]):
            if auf_leitung(e, i) or any(math.dist(e, p) < 2 for _, p, _ in pfeil_enden) or any(
                    math.dist(a.pos, e) <= 4 for bt in inh.bauteile for a in bt.anschluesse):
                continue
            namens_enden.append((i, e, leit[i]["pts"][1 if k == 0 else -2], False))
    # global nach Abstand zuordnen (eng stehende Pfeile: jeder Text gehört genau einer Leitung, §12)
    # Längs der Leitung stehende Texte: Bei eng stehenden Pfeilen liegt ein Text oft näher an der Nachbarleitung als
    # an seiner eigenen. Der Zeichner setzt aber alle Texte einer Gruppe gleich neben ihre Leitung (gleicher Versatz
    # quer zur Leitung, Seite und Abstand je Blatt verschieden). Deshalb: je Text den Versatz wählen, den die meisten
    # Nachbartexte ebenfalls zu einer ihrer Leitungen haben, und daran messen. Belegt in Projekt A: Signalboard-
    # Ausgänge „A8:Step/A8:Dir/A7:Step/A7:Dir“ (Grundlinie an der Leitung) und Netzteil „K1:3/X4:1“ (Oberkante an
    # der Leitung) kommen beide richtig heraus.
    paare, laengs = [], []
    for ni, (i, p, vor, pfeil) in enumerate(namens_enden):
        senkrecht = abs(p[0] - vor[0]) < abs(p[1] - vor[1])
        for ti, t in enumerate(freie_texte):
            if t["art"] != "text" or (not pfeil and re.match(r"^-?X\d+[\w.]*$", t["text"].strip())):
                continue                           # Leistenname (X1) ist kein Potentialname
            q = t["pts"]
            d = min(_abst_seg(c, p, vor) for c in q)
            d = min(d, min(_abst_seg(x, a, b) for x in (p, vor) for a, b in zip(q, q[1:] + q[:1])))
            parallel = (abs(t["drehung"]) == 90) == senkrecht
            grenze = (140 if pfeil else 40) if parallel else 40
            if d > grenze:
                continue
            if not parallel:
                paare.append((d + 30, ni, ti))
                continue
            k = 0 if senkrecht else 1               # Achse quer zur Leitung
            c = _mitte(q)
            lo, hi = sorted((p[1 - k], vor[1 - k]))
            laengs.append((ni, ti, p[k] - c[k], max(0.0, lo - c[1 - k], c[1 - k] - hi), d))
    TOL_VERSATZ = 12          # 1,2 mm: gleicher Versatz Text <-> Leitung
    versatz = {}
    for ti in {ti for _, ti, _, _, _ in laengs}:
        c0 = _mitte(freie_texte[ti]["pts"])
        nachbarn = {tj for _, tj, _, _, _ in laengs
                    if freie_texte[tj]["drehung"] == freie_texte[ti]["drehung"]
                    and math.dist(_mitte(freie_texte[tj]["pts"]), c0) <= 300}
        eigene = [s for _, tj, s, _, _ in laengs if tj == ti]
        best = None
        for s0 in eigene:
            treffer = sum(1 for tj in nachbarn if any(abs(s - s0) <= TOL_VERSATZ for _, tk, s, _, _ in laengs if tk == tj))
            schl = (-treffer, abs(s0))
            if best is None or schl < best[0]:
                best = (schl, s0)
        versatz[ti] = best[1]
    for ni, ti, s, entlang, d in laengs:
        paare.append((abs(s - versatz[ti]) + entlang + 0.01 * d, ni, ti))
    paare.sort()
    vergeben_e, vergeben_t = set(), set()
    for d, ni, ti in paare:
        if ni in vergeben_e or ti in vergeben_t:
            continue
        vergeben_e.add(ni)
        vergeben_t.add(ti)
        i, p, vor, pfeil = namens_enden[ni]
        t = freie_texte[ti]
        benutzt.add(id(t))
        nz = leit[i]["_netz"]
        nz.namen.append(t["text"].strip())
        nz.pfeile.append((p, vor, t["text"].strip()))
    for ni, (i, p, vor, pfeil) in enumerate(namens_enden):
        if pfeil and ni not in vergeben_e:
            nz = leit[i]["_netz"]
            nz.pfeile.append((p, vor, ""))
            inh.fragen.append(f"Pfeil bei {p[0] / 10:.0f}/{p[1] / 10:.0f} mm ohne Beschriftung "
                              f"(Netz an {', '.join(map(str, nz.anschluesse)) or 'keinem Bauteil'}).")

    inh.bauteile = [bt for bt in inh.bauteile if not bt.ist_bild or bt.anschluesse]
    # Bauteil ohne Bezeichner, aber mit Kennzeichen als Text im Bauteil (z. B. „A8“ im Treiberbild): übernehmen
    kennz = re.compile(r"^-?[A-Z]{1,3}\d+(\.\d+)?$")
    for bt in inh.bauteile:
        if not bt.bez and not bt.ist_bild:
            t = next((t for t, _ in bt.texte if kennz.match(t.strip())), None)
            if t:
                bt.bez = t.strip()
                inh.fragen.append(f"Bauteil „{bt.symbol}“ hat in sPlan keinen Bezeichner; Kennzeichen {bt.bez} aus "
                                  f"dem Text im Bauteil übernommen.")
    for b in inh.bilder:
        if b["_bauteil"] in inh.bauteile:
            b["_geraet"] = True
    for bt in inh.bauteile:
        if bt.ist_bild:
            x0, y0, x1, y1 = bt.bbox
            kand = []
            for t in freie_texte:
                if id(t) in benutzt or not re.match(r"^-?[A-Z]{1,3}\d+(\.\d+)?$", t["text"].strip()):
                    continue
                m = _mitte(t["pts"])
                kand.append((math.hypot(max(x0 - m[0], 0, m[0] - x1), max(y0 - m[1], 0, m[1] - y1)), id(t), t))
            kand.sort(key=lambda k: k[:2])
            if kand and kand[0][0] <= 80:
                bt.bez = kand[0][2]["text"].strip()
                benutzt.add(id(kand[0][2]))
            inh.fragen.append(f"{bt.bez or 'Gerät'} ist in sPlan nur als Bild eingefügt: Anschlussnamen stehen im Bild "
                              f"und wurden nach Lage nummeriert – bitte prüfen.")
    for bt in inh.bauteile:
        _anschlussnamen(bt, inh)
    # Beschreibungstexte (Gerätename, Leistenname) nahe am Bauteil
    for t in freie_texte:
        if id(t) in benutzt:
            continue
        m = _mitte(t["pts"])
        kand = []
        for bt in inh.bauteile:
            x0, y0, x1, y1 = bt.bbox
            dx = max(x0 - m[0], 0, m[0] - x1)
            dy = max(y0 - m[1], 0, m[1] - y1)
            kand.append((math.hypot(dx, dy), bt.nr, bt))
        kand.sort(key=lambda k: k[:2])
        if kand and kand[0][0] <= 80:
            kand[0][2].beschreibung.append(t["text"].strip())
            benutzt.add(id(t))
        else:
            inh.texte.append(t)
    return inh


def _anschlussnamen(bt, inh):
    for a in bt.anschluesse:
        a.seite = seite(bt, a.pos)
        if ist_klemme(bt):
            a.seite = "oben" if a.pos[1] < bt.mitte[1] else "unten"
    if ist_klemme(bt):
        # Reihenklemme: oben 1, unten 2; mehrstöckig („Klemmen“): Spalte 1, 2, 3 … von links, Seite als Zusatz
        if re.search(r"klemmen", bt.symbol, re.I):
            xs = sorted({round(a.pos[0] / 8) for a in bt.anschluesse})
            for a in bt.anschluesse:
                a.ebene = xs.index(round(a.pos[0] / 8)) + 1
                a.name = f"{a.ebene}.{'1' if a.seite in ('oben', 'links') else '2'}"
        else:
            for a in bt.anschluesse:
                a.name = "1" if a.seite in ("oben", "links") else "2"
        return
    vergeben = set()
    for a in sorted(bt.anschluesse, key=lambda a: a.pos):
        cands = [(math.dist(a.pos, p), k, txt) for k, (nm, txt, p) in enumerate(bt.kontakte)
                 if ("k", k) not in vergeben]
        cands += [(math.dist(a.pos, p) + 5, 1000 + k, txt) for k, (txt, p) in enumerate(bt.texte)
                  if ("t", k) not in vergeben and 0 < len(txt.strip()) <= 8]
        cands.sort(key=lambda c: c[:2])
        if cands and cands[0][0] <= 90:
            a.name = cands[0][2].strip()
            k = cands[0][1]
            vergeben.add(("k", k) if k < 1000 else ("t", k - 1000))
    ohne = [a for a in bt.anschluesse if not a.name]
    if ohne:
        oben = sorted([a for a in ohne if a.seite in ("oben", "links")], key=lambda a: a.pos)
        unten = sorted([a for a in ohne if a.seite in ("unten", "rechts")], key=lambda a: a.pos)
        if "klemme" in bt.symbol.lower():
            for a in oben:
                a.name = "1"
            for a in unten:
                a.name = "2"
        else:
            for k, a in enumerate(oben + unten, 1):
                a.name = str(k)
            if not bt.ist_gruppe and not bt.ist_bild and len(bt.anschluesse) > 2:
                inh.fragen.append(f"{bt.bez or bt.symbol}: {len(ohne)} Anschlüsse ohne Beschriftung im sPlan-"
                                  f"Bauteil, im Import nummeriert ({', '.join(a.name for a in oben + unten)}).")
    gesehen = {}
    for a in bt.anschluesse:
        a.name = sauber(" ".join(a.name.split()))
        if a.name in gesehen:
            gesehen[a.name] += 1
            a.name = f"{a.name}.{gesehen[a.name]}"
        else:
            gesehen[a.name] = 1
