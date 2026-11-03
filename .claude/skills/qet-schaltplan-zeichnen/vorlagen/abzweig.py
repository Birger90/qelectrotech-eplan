"""EPLAN-Abzweig (T-Knoten mit 45°-Schräge) statt QETs Verbindungspunkt, Symbole aus symbole_aus_pdf.sh.

Arten nach Lage des Abzweigs und der Schräge: „unten_links“ (durchgehende Schiene, Schräge von links nach
unten), „unten_rechts“ (Ecke links->unten, Schräge von unten nach rechts), „oben_rechts“ (Ecke links->oben,
Schräge von oben nach rechts), „rechts_unten“ (senkrecht durchlaufend, Schräge von unten nach rechts).
Die Anschlüsse liegen 5,7 pt (7,5 px) vom Knoten entfernt. ECKE ist eine Leitungsecke ohne Punkt mit
denselben Anschluss-Indizes wie QETs Verbindungspunkt (N, W, E, S = 0-3).

Hängt nur vom Blatt-Index 0 ab, damit auch blatt08_einspeisung es importieren kann.
"""
F = 0
PFAD = "custom://Allgemein/Verbindungen/Abzweig_Plan/Abzweig_{}_Plan.elmt"
ECKE = "custom://Allgemein/Verbindungen/Abzweig_Plan/Ecke_Plan.elmt"
D = 7.5   # Abstand Anschluss <-> Knoten (px)
# Anschluss-Index je Richtung (oben->unten, links->rechts) und Lage des ersten Anschlusses zum Knoten
ANSCHLUSS = {"unten_links": {"W": 0, "E": 1, "S": 2}, "unten_rechts": {"W": 0, "E": 1, "S": 2},
             "oben_rechts": {"N": 0, "W": 1, "E": 2}, "rechts_unten": {"N": 0, "E": 1, "S": 2}}
URSPRUNG = {"unten_links": (-D, 0), "unten_rechts": (-D, 0), "oben_rechts": (0, -D), "rechts_unten": (0, 0)}


def _element(ident, pfad, x, y):
    return [{"op": "add_element", "folio": F, "path": pfad, "x": x, "y": y, "id": ident},
            {"op": "set_label", "folio": F, "element": f"${ident}", "label": ""}]


def abzweig(ident, art, x, y):
    """Abzweig am Knoten x, y (px); Anschlüsse über an(art, Richtung)."""
    dx, dy = URSPRUNG[art]
    return _element(ident, PFAD.format(art), round(x + dx, 1), round(y + dy, 1))


def ecke(ident, x, y):
    """Leitungsecke ohne Punkt am Knoten x, y (px)."""
    return _element(ident, ECKE, x, y)


def an(art, richtung):
    return ANSCHLUSS[art][richtung]
