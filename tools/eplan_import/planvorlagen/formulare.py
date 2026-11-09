"""Formularblätter (Deckblatt, Ausführungsdaten): Texte, Linien, Kästen, Ankreuzfelder in pt-Lagen eines
Firmenformulars (A3 quer, wie EPLAN-Formular), umgerechnet ins QET-Blatt. Inhalte sind Projektdaten."""

S = 1.3229                         # px je pt


def X(pt):
    return round(10 + pt * S, 1)


def Y(pt):
    return round(990 + (pt - 760.1) * S, 1)


def text(b, x, y, groesse, s):
    b.text(s, X(x), Y(y), groesse)


def linie(b, x1, y1, x2, y2, breite=0.4):
    i = b._id("l")
    b.ops += [{"op": "add_shape", "folio": b.folio, "shape": "line", "x1": X(x1), "y1": Y(y1), "x2": X(x2), "y2": Y(y2),
               "id": i},
              {"op": "set_shape", "folio": b.folio, "index": f"${i}", "property": "width", "value": str(round(breite * S, 2))}]


def kasten(b, x1, y1, x2, y2, breite=0.4):
    i = b._id("r")
    b.ops += [{"op": "add_shape", "folio": b.folio, "shape": "rectangle", "x1": X(x1), "y1": Y(y1), "x2": X(x2),
               "y2": Y(y2), "id": i},
              {"op": "set_shape", "folio": b.folio, "index": f"${i}", "property": "width", "value": str(round(breite * S, 2))}]


def ankreuz(b, x, y, an):
    kasten(b, x, y, x + 14, y + 17)
    if an:
        text(b, x + 3.0, y + 1.5, 8.8, "X")


def bild(b, datei, x_pt, y_pt, breite_px, hoehe_px, faktor):
    """Bild mit linker oberer Ecke bei (x_pt, y_pt); QET skaliert um die Bildmitte, daher vorab versetzt."""
    i = b._id("bild")
    b.ops += [{"op": "add_image", "folio": b.folio, "file": datei, "id": i,
               "x": round(X(x_pt) - breite_px * (1 - faktor) / 2, 1), "y": round(Y(y_pt) - hoehe_px * (1 - faktor) / 2, 1)},
              {"op": "scale_image", "folio": b.folio, "index": f"${i}", "factor": faktor}]


def tabelle_aus_daten(b, texte, linien, felder):
    """Formular aus Datenlisten: texte [(x, y, Größe, Text)], linien [(x1, y1, x2, y2, Breite)],
    felder [(x, y, angekreuzt)] – Lagen in pt."""
    for x, y, g, s, *_ in texte:
        text(b, x, y, g, s)
    for x1, y1, x2, y2, w in linien:
        linie(b, x1, y1, x2, y2, w)
    for x, y, an in felder:
        ankreuz(b, x, y, an)
