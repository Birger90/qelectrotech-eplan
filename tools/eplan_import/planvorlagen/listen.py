"""Auswertungen wie EPLAN, erzeugt aus einem fertigen QET-Projekt (nicht aus dem Original):
Inhaltsverzeichnis, Strukturkennzeichen, Klemmen-/Steckerpläne, Kabelübersicht, Artikelstückliste,
Artikelsummenstückliste.

    daten = auswerten("plan.qet")          # Blätter, Geräte, Klemmen mit Zielen, Kabel
    listen.klemmenplaene(plan, daten, nummer_ab=100)
Ein Plan wird dazu zweimal gebaut: erst der Schaltplan (auswerten), dann alles samt Listen (projekt.py).
"""
import math
import re
import xml.etree.ElementTree as ET
from collections import defaultdict, OrderedDict

from .blatt import SPALTE_B, X0

DURCHLASS = ("Ecke_Plan", "Abzweig_", "/Verweise/", "Querschnitt-Strich")   # Leitung läuft hindurch
ZEILEN, Y0, ZH = 38, 70, 23


def _info(e, name):
    for i in e.iter("elementInformation"):
        if i.get("name") == name:
            return (i.text or "").strip()
    return ""


def auswerten(qet):
    """Liest das Projekt: Blätter, Elemente, Verbindungsgraph (Knoten = (Element, Anschluss)), Kabel."""
    root = ET.parse(qet).getroot()
    blaetter, elemente = [], {}
    kanten, knoten_je_element, leiter, kabel_adern = defaultdict(set), defaultdict(set), {}, {}
    diagramme = root.findall("diagram")
    for f, d in enumerate(diagramme):
        blaetter.append({"index": f, "uuid": d.get("uuid"), "nr": d.get("folio", ""), "anlage": d.get("plant", ""),
                         "ort": d.get("locmach", ""), "titel": d.get("title", "")})
        for e in d.iter("element"):
            x = float(e.get("x", 0))
            elemente[e.get("uuid")] = {
                "folio": f, "blatt": d.get("folio", ""), "anlage": d.get("plant", ""), "ort": d.get("locmach", ""),
                "typ": e.get("type", ""), "label": _info(e, "label"), "leiste": _info(e, "terminal_strip"),
                "hersteller": _info(e, "manufacturer"), "bestellnr": _info(e, "manufacturer_reference"),
                "bezeichnung": _info(e, "description"), "spalte": max(0, int((x - X0) // SPALTE_B)),
                "links": [lk.get("uuid") for lk in e.iter("link_uuid")], "anschlussnamen": {}}
        for c in d.iter("conductor"):
            a = (c.get("element1"), c.get("terminal1"))
            b = (c.get("element2"), c.get("terminal2"))
            kanten[a].add((b, c.get("uuid")))
            kanten[b].add((a, c.get("uuid")))
            knoten_je_element[a[0]].add(a)
            knoten_je_element[b[0]].add(b)
            leiter[c.get("uuid")] = (a, b)
            for k, nm in ((a, c.get("terminalname1")), (b, c.get("terminalname2"))):
                if k[0] in elemente:
                    elemente[k[0]]["anschlussnamen"][k[1]] = nm or ""
    kabel = []
    for cab in root.iter("cable"):
        adern = [co.get("conductor") for co in cab.findall("core")]
        for a in adern:
            kabel_adern[a] = cab.get("designation", "")
        teil = cab.find("part")
        folio = next((b["index"] for b in blaetter if teil is not None and b["uuid"] == teil.get("diagram")), None)
        kabel.append({"name": cab.get("designation", ""), "typ": cab.get("type", ""), "adern": cab.get("cores", ""),
                      "folio": folio, "leiter": adern})
    return {"blaetter": blaetter, "elemente": elemente, "kanten": kanten, "knoten": knoten_je_element,
            "leiter": leiter, "kabel_adern": kabel_adern, "kabel": kabel}


def _durchlass(e):
    return any(k in e.get("typ", "") for k in DURCHLASS)


def _name(e, anschluss):
    """Ziel wie im Klemmenplan: Klemme „-X1:5“, Gerät „-K1:A1“."""
    if e.get("leiste"):
        return f"{e['leiste']}:{e.get('label', '')}"
    an = e.get("anschlussnamen", {}).get(anschluss, "")
    return f"{e.get('label', '')}:{an}" if an else e.get("label", "")


def ziele(daten, start, ohne_leiter=None):
    """Direktes Ziel wie im EPLAN-Klemmen-/Kabelplan: das nächste Nicht-Durchlass-Element, das vom Knoten `start`
    über Leitungen erreichbar ist (durch Ecken, Abzweige und Folgeverweise hindurch). Breitensuche nach Abstand:
    nur die Ziele mit dem kleinsten Abstand zählen – nicht das ganze Potential. Rückgabe (Zielnamen, Kabelname
    der ersten Kabelader oder "")."""
    from collections import deque
    el, ka, kn = daten["elemente"], daten["kanten"], daten["knoten"]
    gesehen, offen, treffer, kabel = {start}, deque([(start, 0)]), {}, ""
    beste = None
    e0 = el.get(start[0], {})
    if _durchlass(e0):                      # Start auf Ecke/Abzweig/Pfeil: auch dessen andere Anschlüsse/Partner
        weiter = set(kn.get(start[0], ()))
        for lk in e0.get("links", []):
            weiter |= kn.get(lk, set())
        for k in weiter - gesehen:
            gesehen.add(k)
            offen.append((k, 0))
    while offen:
        knoten, tiefe = offen.popleft()
        if beste is not None and tiefe >= beste:
            break
        for (nach, lt) in ka.get(knoten, ()):
            if lt == ohne_leiter:                     # Kabelende: nicht über die eigene Ader zur Gegenseite
                continue
            kabel = kabel or daten["kabel_adern"].get(lt, "")
            if nach in gesehen:
                continue
            gesehen.add(nach)
            e = el.get(nach[0], {})
            if _durchlass(e):
                weiter = set(kn.get(nach[0], ()))
                for lk in e.get("links", []):
                    weiter |= kn.get(lk, set())
                for k in weiter - gesehen:
                    gesehen.add(k)
                    offen.append((k, tiefe + 1))
            elif nach[0] != start[0]:
                name = _name(e, nach[1])
                if name:
                    treffer[name] = tiefe
                    beste = tiefe + 1 if beste is None else beste
    t = sorted(treffer)
    return t[:2], kabel


# --- Tabellenblätter ------------------------------------------------------------------------------
def tabelle(b, spalten, zeilen):
    """spalten: [(Titel, x_links, breite)], zeilen: [[Text …]] – Kopf + Zeilen mit Linien."""
    x1, x2 = spalten[0][1], spalten[-1][1] + spalten[-1][2]

    def linie(xa, ya, xb, yb):
        b.ops.append({"op": "add_shape", "folio": b.folio, "shape": "line", "x1": xa, "y1": ya, "x2": xb, "y2": yb,
                      "id": b._id("l")})
    for titel, x, w in spalten:
        b.text(titel, x + 4, Y0 + 4, 8)
    linie(x1, Y0, x2, Y0)
    linie(x1, Y0 + ZH, x2, Y0 + ZH)
    for r, z in enumerate(zeilen):
        y = Y0 + ZH * (r + 1)
        for (titel, x, w), wert in zip(spalten, z):
            if wert not in (None, ""):
                b.text(str(wert)[: max(4, int(w / 5.0))], x + 4, y + 5, 7)
        linie(x1, y + ZH, x2, y + ZH)
    y_ende = Y0 + ZH * (len(zeilen) + 1)
    for titel, x, w in spalten:
        linie(x, Y0, x, y_ende)
    linie(x2, Y0, x2, y_ende)


def seiten(plan, titel, anlage, ort, nummer_ab, spalten, zeilen):
    """Zeilen auf Blätter verteilen; Rückgabe der nächsten freien Blattnummer."""
    n = nummer_ab
    for i in range(0, max(1, len(zeilen)), ZEILEN):
        tabelle(plan.blatt(titel, anlage, ort, n), spalten, zeilen[i:i + ZEILEN])
        n += 1
    return n


def anzahl_seiten(zeilen):
    return max(1, math.ceil(len(zeilen) / ZEILEN))


# Inhaltsverzeichnis --------------------------------------------------------------------------------
INHALT_SPALTEN = [("Anlage", 40, 100), ("Ort", 140, 100), ("Blatt", 240, 70), ("Seitenbenennung", 310, 900)]


def inhalt_zeilen(eintraege):
    """eintraege: [(anlage, ort, nr, titel)] in Blattreihenfolge."""
    return [[a and "=" + a, o and "+" + o, n, t] for a, o, n, t in eintraege]


# Strukturkennzeichen -----------------------------------------------------------------------------
STRUKTUR_SPALTEN = [("Anlagenkennzeichen", 40, 160), ("Beschreibung", 200, 520), ("Ortskennzeichen", 720, 160),
                    ("Beschreibung", 880, 600)]


def struktur_zeilen(anlagen, orte):
    a, o = list(anlagen.items()), list(orte.items())
    return [[(a[i][0] and "=" + a[i][0]) if i < len(a) else "", a[i][1] if i < len(a) else "",
             (o[i][0] and "+" + o[i][0]) if i < len(o) else "", o[i][1] if i < len(o) else ""]
            for i in range(max(len(a), len(o)))]


# Klemmen- und Steckerpläne -------------------------------------------------------------------------
KLEMMEN_SPALTEN = [("Ziel extern", 40, 360), ("Kabel", 400, 110), ("Klemme", 510, 90), ("Ziel intern", 600, 700),
                   ("Seite/Pfad", 1300, 140)]


def _sortkey(nr):
    m = re.match(r"([A-Za-z]*)(\d*)(.*)", str(nr))
    return (m.group(1), int(m.group(2)) if m.group(2) else 0, m.group(3))


def klemmen_zeilen(daten):
    """Je Leiste die Klemmen/Pins mit Zielen; extern = Seite, über die ein Kabel geht."""
    leisten = defaultdict(list)
    for u, e in daten["elemente"].items():
        if not e.get("leiste"):
            continue
        seiten_ = [ziele(daten, k) for k in sorted(daten["knoten"].get(u, ()))]
        extern = next((s for s in seiten_ if s[1]), ([], ""))
        intern = sorted({z for s in seiten_ if s is not extern for z in s[0]})
        nr = e.get("label", "")
        leisten[e["leiste"]].append((_sortkey(nr), [", ".join(extern[0]), extern[1], nr, ", ".join(intern),
                                                     f"/{e['blatt']}/{e['spalte']}"]))
    return OrderedDict((k, [z for _, z in sorted(v, key=lambda a: a[0])])
                       for k, v in sorted(leisten.items(), key=lambda kv: _sortkey(kv[0].lstrip("-"))))


# Kabelübersicht ----------------------------------------------------------------------------------
KABEL_SPALTEN = [("Kabel", 40, 120), ("von", 160, 380), ("nach", 540, 380), ("Kabeltyp", 920, 300),
                 ("Adern", 1220, 80), ("Seite", 1300, 120)]


def kabel_zeilen(daten):
    """Je Kabelname eine Zeile (auf mehreren Blättern gezeichnete Kabel zusammengefasst): Geräte an beiden
    Enden, Typ, Aderzahl, Seiten. Enden an Potentialschienen (viele Ziele) zählen nicht als Gerät."""
    kabel = OrderedDict()
    for k in daten["kabel"]:
        if not k["name"] or k["name"] == "W":
            continue
        z = kabel.setdefault(k["name"], {"enden": [set(), set()], "typ": k["typ"], "adern": 0, "seiten": []})
        z["adern"] += int(k["adern"] or 0)
        if k["folio"] is not None:
            z["seiten"].append(daten["blaetter"][k["folio"]]["nr"])
        for lt in k["leiter"]:
            for seite, knoten in enumerate(daten["leiter"].get(lt, ())):
                e = daten["elemente"].get(knoten[0], {})
                # Kabelende = das Gerät, auf dem die Ader landet; nur über Ecken/Abzweige hinweg weitersuchen
                ziel = [_name(e, knoten[1])] if not _durchlass(e) else ziele(daten, knoten, ohne_leiter=lt)[0]
                if not any(ziel) and re.search(r"PE|terre|Erde", e.get("typ", "")):
                    ziel = ["PE"]                       # Ader endet auf einem Schutzleiter-/Erdsymbol
                z.setdefault("paar", {}).setdefault(lt, [set(), set()])[seite].update(
                    n for n in (t.split(":")[0] for t in ziel if t) if n)
    # Seiten je Ader ausrichten: Leitungen sind mal vom Gerät zur Klemme, mal umgekehrt gezeichnet
    for z in kabel.values():
        a_seite, b_seite = z["enden"]
        for x, y in z.pop("paar", {}).values():
            if (x & b_seite) or (y & a_seite):
                x, y = y, x
            a_seite |= x
            b_seite |= y
    zeilen = []
    for name, z in kabel.items():
        a, b = (sorted(e) for e in z["enden"])
        if set(a) & set(b) and len(a) > len(b):          # Enden ordnen: Klemmleisten zuerst
            a, b = b, a
        zeilen.append((_sortkey(name[1:]), [name, ", ".join(a), ", ".join(b), z["typ"], z["adern"],
                                             ", ".join(f"/{s}" for s in sorted(set(z["seiten"]), key=_sortkey))]))
    return [z for _, z in sorted(zeilen, key=lambda a: a[0])]


# Stücklisten -------------------------------------------------------------------------------------
STUECK_SPALTEN = [("Betriebsmittel", 40, 220), ("Bestellnummer", 260, 260), ("Hersteller", 520, 200),
                  ("Bezeichnung", 720, 600), ("Platzierung", 1320, 120)]
SUMME_SPALTEN = [("Bestellnummer", 40, 300), ("Menge", 340, 80), ("Hersteller", 420, 240), ("Bezeichnung", 660, 800)]


KANAL = ("_Kanal", "_B1.elmt", "_C1.elmt", "_D1.elmt", "_E1.elmt")   # Kanal-/Teilmakros eines Geräts


# einheitliche Herstellernamen in den Listen (Artikeldaten kommen aus verschiedenen Quellen)
HERSTELLER = {"CYBELEC S.A.": "Cybelec", "SIEMENS": "Siemens", "Phoenix": "Phoenix Contact"}


def stueck_zeilen(daten):
    """Je Gerät (Kennzeichen + Bestellnummer) eine Zeile; Klemmen/Pins nicht einzeln. Kanäle eines Moduls zählen
    zum Modul: ist das Modul selbst gesetzt (Übersicht, Station), gilt dessen Anlage, sonst die erste Fundstelle."""
    geraete = OrderedDict()
    for e in sorted(daten["elemente"].values(), key=lambda e: (any(k in e.get("typ", "") for k in KANAL),
                                                              e.get("folio", 0))):
        if not e.get("bestellnr") or e.get("leiste"):
            continue
        # Kennzeichen zerlegen: „=SB-A3“ (Kanal mit Anlage) und „-A3“ auf einem =SB-Blatt sind dasselbe Gerät
        m = re.match(r"^(=[^+\-]+)?(\+[^-]+)?(-.+)?$", e.get("label", "") or "")
        bmk = (m.group(3) if m else "") or ""
        if not bmk:
            continue                                        # ohne Betriebsmittelkennzeichen keine Stücklistenzeile
        anlage = (m.group(1) or "=" + e.get("anlage", ""))[1:]
        ort = (m.group(2) or "+" + e.get("ort", ""))[1:]
        schluessel = (anlage, bmk, e["bestellnr"])
        if schluessel in geraete:
            continue
        geraete[schluessel] = (e, ort)
    zeilen = []
    for (anlage, bmk, nr), (e, ort) in geraete.items():
        zeilen.append((anlage, _sortkey(bmk.lstrip("-")),
                       [f"={anlage}+{ort}{bmk}", nr, HERSTELLER.get(e.get("hersteller", ""), e.get("hersteller", "")),
                        e.get("bezeichnung", ""),
                        f"/{e['blatt']}/{e['spalte']}"]))
    return [z for *_, z in sorted(zeilen, key=lambda a: (a[0], a[1]))]


def summen_zeilen(stueck):
    summe = OrderedDict()
    for z in stueck:
        summe.setdefault((z[1], z[2]), [0, z[3]])
        summe[(z[1], z[2])][0] += 1
    return [[k[0], v[0], k[1], v[1]] for k, v in sorted(summe.items())]
