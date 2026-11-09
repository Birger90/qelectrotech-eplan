"""Automatische Sichtprüfung eines exportierten Plans (PDF): Texte, die von Linien durchgestrichen werden, und Texte,
die sich überdecken. Ersetzt nicht das Ansehen, findet aber die häufigsten Mängel auf allen Blättern zuverlässig.

    python -m planvorlagen.pruefung plan.pdf [von-bis]

Ausgabe je Blatt: Text, Lage (Spalte 0–9) und Art des Mangels. Schriftfeld, Spaltenleiste und Tabellenblätter
(Listen) werden übersprungen – dort sind Linien unter/neben Texten gewollt."""
import sys

import pymupdf

RAND_UNTEN = 75          # pt: Schriftfeld
RAND_OBEN = 16           # pt: Spaltenleiste


MIN_PT = 5.5             # kleinere Schrift ist Innenleben von Herstellermakros (Pinnamen im Gerätekasten)


def _woerter(p):
    """Wörter mit Box, leicht geschrumpft (Unterlängen/Zeilenabstand sollen keine Treffer erzeugen)."""
    h = p.rect.height
    klein = []
    for blk in p.get_text("dict")["blocks"]:
        for ln in blk.get("lines", []):
            for sp in ln["spans"]:
                if sp["size"] < MIN_PT:
                    klein.append(pymupdf.Rect(sp["bbox"]))
    for x0, y0, x1, y1, w, *_ in p.get_text("words"):
        if y1 > h - RAND_UNTEN or y0 < RAND_OBEN:
            continue
        if any(k.contains(pymupdf.Point((x0 + x1) / 2, (y0 + y1) / 2)) for k in klein):
            continue
        dy, dx = (y1 - y0) * 0.28, min(0.8, (x1 - x0) * 0.1)
        yield w, pymupdf.Rect(x0 + dx, y0 + dy, x1 - dx, y1 - dy)


def _linien(p):
    for d in p.get_drawings():
        for it in d["items"]:
            if it[0] == "l":
                yield it[1], it[2]
            elif it[0] == "re":
                r = it[1]
                for a, b in ((r.tl, r.tr), (r.bl, r.br), (r.tl, r.bl), (r.tr, r.br)):
                    yield a, b


def _schneidet(r, a, b):
    """Gerade Linie a-b schneidet das Innere von r (waagerecht/senkrecht; schräge Linien grob über die Box)."""
    if abs(a.x - b.x) < 0.3:                                   # senkrecht
        return r.x0 < a.x < r.x1 and min(a.y, b.y) < r.y1 and max(a.y, b.y) > r.y0
    if abs(a.y - b.y) < 0.3:                                   # waagerecht
        return r.y0 < a.y < r.y1 and min(a.x, b.x) < r.x1 and max(a.x, b.x) > r.x0
    return False


def ist_liste(p):
    t = p.get_text()
    return any(k in t for k in ("Klemmenplan", "Steckerplan", "Kabelübersicht", "stückliste", "Inhaltsverzeichnis",
                                "Übersicht Anlagenkennzeichen", "Deckblatt", "Aufbau Schaltschrank"))


def spalte(p, x):
    return int(max(0, min(9, (x / p.rect.width) * 10)))


def pruefe_seite(p):
    funde = []
    woerter = list(_woerter(p))
    linien = [(a, b) for a, b in _linien(p) if abs(a.x - b.x) > 2 or abs(a.y - b.y) > 2]
    for w, r in woerter:
        if len(w) < 2:
            continue
        for a, b in linien:
            if _schneidet(r, a, b):
                funde.append(("Linie durch Text", w, r))
                break
    for i, (w1, r1) in enumerate(woerter):
        for w2, r2 in woerter[i + 1:]:
            s = r1 & r2
            if not s.is_empty and s.width > 1.0 and s.height > 1.0:
                funde.append(("Text auf Text", f"{w1} / {w2}", r1 | r2))
    return funde


def pruefe(pfad, von=1, bis=None):
    d = pymupdf.open(pfad)
    bis = bis or len(d)
    alle = {}
    for n in range(von, bis + 1):
        p = d[n - 1]
        if ist_liste(p):
            continue
        f = pruefe_seite(p)
        if f:
            alle[n] = [(art, w, spalte(p, r.x0)) for art, w, r in f]
    return alle


if __name__ == "__main__":
    pfad = sys.argv[1]
    von, bis = 1, None
    if len(sys.argv) > 2:
        a, _, b = sys.argv[2].partition("-")
        von, bis = int(a), int(b or a)
    ergebnis = pruefe(pfad, von, bis)
    gesamt = 0
    for seite, funde in ergebnis.items():
        print(f"Seite {seite}: {len(funde)}")
        for art, w, sp in funde:
            print(f"   Sp.{sp}  {art}: {w}")
        gesamt += len(funde)
    print("Summe:", gesamt)
