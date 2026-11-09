"""Import gegen das EPLAN-Original-PDF prüfen: je Blatt, welche Texte des Originals an derselben Stelle stehen.

    python pdf_vergleich.py <original.pdf> <import.pdf> [bericht.txt] [--seiten 1-30]

- Seiten werden über die Reihenfolge zugeordnet (der Import legt je EPLAN-Seite ein Blatt in Seitenfolge an).
- Wörter beider PDFs in mm; die Abbildung Import -> Original (Maßstab + Versatz je Achse) wird aus Wörtern
  bestimmt, die auf beiden Seiten genau einmal vorkommen (robust: Median).
- Treffer: gleiches Wort im Umkreis von 4 mm. Ergebnis je Blatt: Anteil gefundener Wörter, fehlende Wörter,
  verschobene Wörter (gleiches Wort nur weiter weg); Schriftfeld (untere 20 mm) ausgenommen.
Benötigt PyMuPDF (Python 3.11 dieses Rechners)."""
import collections
import statistics
import sys

import pymupdf

MM = 25.4 / 72


def woerter(seite, rand_unten=20.0, rand_oben=12.0, rand_seite=6.0):
    """Wörter (Text, x, y in mm, y nach oben) ohne Schriftfeld, Spaltenleiste und seitliche Vermerke."""
    h, b = seite.rect.height * MM, seite.rect.width * MM
    aus = []
    for x0, y0, x1, y1, w, *_ in seite.get_text("words"):
        w = w.strip()
        if not w or len(w) > 60:
            continue
        cx, cy = (x0 + x1) / 2 * MM, h - (y0 + y1) / 2 * MM
        if rand_unten < cy < h - rand_oben and rand_seite < cx < b - rand_seite:
            aus.append((w, cx, cy))
    return aus


def kennung(seite):
    """(=Anlage, +Ort, Blattnummer) aus dem Schriftfeld rechts unten (EPLAN und KUNDE-Schriftfeld des Imports)."""
    h, b = seite.rect.height * MM, seite.rect.width * MM
    feld = [(w[4], w[0] * MM, h - w[3] * MM) for w in seite.get_text("words")
            if w[0] * MM > b - 60 and h - w[3] * MM < 25]
    anl = next((w for w, *_ in feld if w.startswith("=") and "/" not in w), "")
    ort = next((w for w, *_ in feld if w.startswith("+") and "/" not in w), "")
    blatt = [(x, y) for w, x, y in feld if w.startswith("Blatt")]
    nr = ""
    if blatt:
        bx, by = blatt[0]
        unter = [(by - y, w) for w, x, y in feld if y < by and abs(x - bx) < 12 and w.strip()]
        nr = min(unter)[1] if unter else ""
    return anl, ort, nr


def abbildung(orig, imp):
    """Import-mm -> Original-mm: x' = ax*x + bx, y' = ay*y + by aus eindeutigen Wörtern."""
    zo = collections.Counter(w for w, *_ in orig)
    zi = collections.Counter(w for w, *_ in imp)
    o = {w: (x, y) for w, x, y in orig if zo[w] == 1 and len(w) > 2}
    i = {w: (x, y) for w, x, y in imp if zi[w] == 1 and len(w) > 2}
    paare = [(i[w], o[w]) for w in o if w in i]
    if len(paare) < 4:
        return None

    def fit(k):
        """Maßstab und Versatz je Achse per Rastersuche: die meisten Wortpaare innerhalb 1,5 mm."""
        bestes = (-1, 1.0, 0.0)
        for i in range(0, 161):
            a = 0.92 + i * 0.001
            b = statistics.median(q[k] - a * p[k] for p, q in paare)
            n = sum(1 for p, q in paare if abs(q[k] - (a * p[k] + b)) < 1.5)
            if n > bestes[0]:
                bestes = (n, a, b)
        return bestes[1], bestes[2]
    return fit(0), fit(1)


def vergleiche(orig_pdf, imp_pdf, seiten=None, radius=4.0):
    do, di = pymupdf.open(orig_pdf), pymupdf.open(imp_pdf)
    imp_von = {}
    for k in range(len(di)):
        imp_von.setdefault(kennung(di[k]), k)
    paare = []
    for k in range(len(do)):
        if seiten and (k + 1) not in seiten:
            continue
        kn = kennung(do[k])
        if kn[2] and kn in imp_von:
            paare.append((k, imp_von[kn], kn))
    # eine Abbildung für alle Blätter (gleiches Papier): eindeutige Wörter aller Blattpaare
    alle_o, alle_i = [], []
    for ko, ki, kn in paare:
        tag = "/".join(kn) + "§"
        alle_o += [(tag + w, x, y) for w, x, y in woerter(do[ko])]
        alle_i += [(tag + w, x, y) for w, x, y in woerter(di[ki])]
    abb = abbildung(alle_o, alle_i)
    je_seite = collections.Counter(w for ko, _, _ in paare for w in {w for w, *_ in woerter(do[ko])})
    wasserzeichen = {w for w, c in je_seite.items() if len(paare) > 5 and c >= 0.9 * len(paare)}
    erg = []
    for ko, ki, kn in paare:
        orig, imp = woerter(do[ko]), woerter(di[ki], 0.0, 0.0, 0.0)
        orig = [o for o in orig if o[0] not in wasserzeichen]
        # Import-Ränder erst nach der Abbildung in Originalkoordinaten abschneiden (Seitenformate verschieden)
        if abb is not None:
            (ax_, bx_), (ay_, by_) = abb
            ho, bo = do[ko].rect.height * MM, do[ko].rect.width * MM
            imp = [(w, x, y) for w, x, y in imp
                   if 20.0 < ay_ * y + by_ < ho - 12.0 and 6.0 < ax_ * x + bx_ < bo - 6.0]
        if not orig:
            continue
        if abb is None:
            erg.append((ko + 1, 0.0, len(orig), [w for w, *_ in orig][:15], [], "keine Zuordnung"))
            continue
        (ax, bx), (ay, by) = abb
        im = collections.defaultdict(list)
        for w, x, y in imp:
            im[w].append((ax * x + bx, ay * y + by))
        treffer, fehlt, verschoben = 0, [], []
        for w, x, y in orig:
            orte = im.get(w, [])
            if any(abs(x - u) <= radius and abs(y - v) <= radius for u, v in orte):
                treffer += 1
            elif orte:
                d = min(((x - u) ** 2 + (y - v) ** 2) ** 0.5 for u, v in orte)
                verschoben.append((w, round(d, 1)))
            else:
                fehlt.append(w)
        om = collections.defaultdict(list)
        for w, x, y in orig:
            om[w].append((x, y))
        extra = [w for w, x, y in imp if not any(abs(ax * x + bx - u) <= radius and abs(ay * y + by - v) <= radius
                                                 for u, v in om.get(w, []))]
        erg.append((ko + 1, treffer / len(orig), len(orig), fehlt, verschoben, " ".join(kn), extra, len(imp)))
    return erg


def bericht(erg):
    zeilen = []
    gesamt_t = sum(q * n for _, q, n, *_ in erg)
    gesamt_n = sum(n for _, _, n, *_ in erg)
    ex_n = sum(len(e[6]) for e in erg if len(e) > 6)
    im_n = sum(e[7] for e in erg if len(e) > 7)
    zeilen.append(f"Gesamt: {gesamt_t / max(1, gesamt_n):.1%} der Originaltexte an gleicher Stelle "
                  f"({len(erg)} Blätter, {gesamt_n} Wörter); zusätzlich/falsch im Import: {ex_n} von {im_n} "
                  f"({ex_n / max(1, im_n):.1%})")
    haeufig_ex = collections.Counter(w for e in erg if len(e) > 6 for w in e[6])
    zeilen.append("Häufig zusätzlich: " + ", ".join(f"{w}×{c}" for w, c in haeufig_ex.most_common(20)))
    haeufig = collections.Counter(w for e in erg for w in e[3])
    zeilen.append("Häufig fehlend: " + ", ".join(f"{w}×{c}" for w, c in haeufig.most_common(25)))
    for nr, q, n, fehlt, versch, hinweis, *_ in sorted(erg, key=lambda e: e[1]):
        zeilen.append(f"Blatt {nr:>3}: {q:6.1%} von {n:>4}  fehlt: {' '.join(fehlt[:12])}"
                      f"{' …' if len(fehlt) > 12 else ''}  verschoben: "
                      f"{' '.join(f'{w}({d})' for w, d in versch[:6])} {hinweis}")
    return "\n".join(zeilen)


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    seiten = None
    if "--seiten" in sys.argv:
        bereich = sys.argv[sys.argv.index("--seiten") + 1]
        a, _, b = bereich.partition("-")
        seiten = set(range(int(a), int(b or a) + 1))
        args = [x for x in args if x != bereich]
    text = bericht(vergleiche(args[0], args[1], seiten))
    if len(args) > 2:
        open(args[2], "w", encoding="utf-8").write(text + "\n")
    print(text[:6000])
