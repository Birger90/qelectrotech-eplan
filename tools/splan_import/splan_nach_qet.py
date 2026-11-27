"""sPlan-8-Plan (.spl8) -> QElectroTech-Projekt (.qet), neu und aufgeräumt gezeichnet.

Ablauf je Blatt:
1. `splan8.lese` liest die Datei (FORMATBESCHREIBUNG_SPL8.md),
2. `splan_analyse.analysiere` macht daraus Bauteile mit Anschlüssen, Netze und Potentialnamen,
3. hier: Funktion erkennen (Kontakt, Spule, Klemme, Schutzschalter, Motor, Gerät …), Symbol wählen,
   Bauteile in Bänder legen (Schienen oben/unten, Schutz -> Schalten -> Steuerung -> Klemmen -> Feld),
   Leitungen als echte QET-Leiter zwischen Anschlüssen, Potentiale als Schienen oder Verweispfeile,
   Spule <-> Kontakte als Master/Slave (über die sPlan-Parent-ID oder gleiches Kennzeichen).
Die Lage aus sPlan bestimmt nur die Reihenfolge (links -> rechts), nicht die Koordinaten.

Aufruf:
    python splan_nach_qet.py <plan.spl8> [<ziel.qet>] [--pdf] [--vergleich <original.pdf>]
Ohne Ziel: <Ordner des Plans>\\QET\\<Name>.qet. Daneben Offene_Fragen.txt.
FORTSCHRITT-Zeilen speisen den Fortschrittsbalken des QET-Dialogs (Datei > sPlan-Plan importieren).
"""
import hashlib
import io
import os
import re
import sys

HIER = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HIER)
sys.path.insert(0, os.path.join(HIER, "..", "eplan_import"))

import splan8                                    # noqa: E402
import splan_analyse as A                        # noqa: E402
import makros as M                               # noqa: E402
from planvorlagen import symbole as S            # noqa: E402
from planvorlagen import symbolbau               # noqa: E402
from planvorlagen.blatt import Plan, raster      # noqa: E402

TITELBLOECKE = os.path.join(os.environ.get("APPDATA", ""), "QElectroTech", "QElectroTech", "titleblocks")
C = "common://10_electric/10_allpole/"
MOTOR_1 = C + "391_consumers_actuators/10_engines/moteur_mono.elmt"
MOTOR_DC = C + "391_consumers_actuators/10_engines/moteur_dc.elmt"
NTC = C + "395_electronics_semiconductors/01_resistors/ntc.elmt"
KASTEN_ORDNER = "sPlan-Import"                    # company://sPlan-Import/… : Gerätekästen ohne Herstellerdaten

ZUORDNUNG = M.Zuordnung()                         # sPlan-Bauteil -> Herstellermakro (makros.py), je Lauf geladen
X_LINKS, X_RECHTS = 110, 1480                     # nutzbare Breite (Schienenpfeile links/rechts frei)
RAIL_UNTEN_Y = 900

POT_UNTEN = re.compile(r"^(GND|0\s?V|M|N|PE|X\d+:\s?M|\d?M)\b", re.I)
POT_NAME = re.compile(r"^(\+?\d+([.,]\d+)?\s?V(DC|AC)?|L[123+]?|N|PE|M|GND|0\s?V|L\d?\s?\d+\s?V|X\d+:\s?M)\b", re.I)


def _fortschritt(prozent, schritt):
    print(f"FORTSCHRITT {int(prozent)} {schritt}", flush=True)


# --- Funktion erkennen ---------------------------------------------------------------------------------
def klassifiziere(bt):
    """Funktion des Bauteils aus Bibliotheksname, Wert und Kontakten (belegt am sPlan-Bauteil)."""
    sym, wert = bt.symbol.lower(), bt.wert.lower()
    namen = {a.name for a in bt.anschluesse}
    n = len(bt.anschluesse)
    if bt.ist_bild or bt.ist_gruppe:
        return "kasten"
    if "klemmen" in sym:
        return "klemmen"
    if "klemme" in sym:
        return "klemme"
    if sym.startswith("schliesser") or sym.startswith("schließer"):
        return "schliesser" if n <= 2 else "kasten"
    if sym.startswith("öffner") or sym.startswith("oeffner"):
        return "oeffner" if n <= 2 else "kasten"
    if ("schütz" in wert or "relais" in wert) and n <= 2 and namen <= {"A1", "A2"}:
        return "spule"
    if "leistungsschalter" in sym and n <= 6:
        return "ls3"
    if (sym.startswith("abb fi") or "fi-schutz" in " ".join(t for t, _ in bt.texte).lower()) and n <= 4:
        return "fi"
    if "kippschalter" in sym and n <= 2:
        return "schalter"
    if "ntc" in sym and n <= 2:
        return "ntc"
    if "wechselstrom" in sym and n <= 2:
        return "motor1"
    if "gleichstrom" in sym and n <= 2:
        return "motor_dc"
    return "kasten"


BAND = {"ls3": 0, "fi": 0, "schliesser": 1, "oeffner": 1, "schalter": 1, "spule": 2, "kasten": 2, "ntc": 2,
        "klemme": 3, "klemmen": 3, "motor1": 4, "motor_dc": 4}


def _sicher(s):
    return re.sub(r"[^A-Za-z0-9ÄÖÜäöüß_.+-]+", "_", s).strip("_") or "Geraet"


def kasten_symbol(bt):
    """Gerätekasten mit den Anschlüssen des sPlan-Bauteils: oben/unten nach ihrer Lage im Original, Reihenfolge
    von links nach rechts. Gleiche Anschlussliste -> gleiche Datei."""
    oben = sorted([a for a in bt.anschluesse if a.seite in ("oben", "links")], key=lambda a: (a.pos[0], a.pos[1]))
    unten = sorted([a for a in bt.anschluesse if a.seite in ("unten", "rechts")], key=lambda a: (a.pos[0], a.pos[1]))
    typ = bt.wert if bt.wert and not bt.wert.startswith("<") else (bt.symbol if bt.symbol not in ("Bild", "Gruppe")
                                                                  else "Gerät")
    if bt.ist_bild:
        typ = "Gerät (Bild in sPlan)"
    typ = typ[:28]
    schl = "|".join([typ, ",".join(a.name for a in oben), ",".join(a.name for a in unten)])
    h = hashlib.md5(schl.encode("utf-8")).hexdigest()[:8]
    rel = f"{KASTEN_ORDNER}/{_sicher(bt.symbol or 'Geraet')}_{h}.elmt"
    hersteller = next((t for t, _ in bt.texte if t in ("Weidmüller", "Meanwell", "Beckhoff", "Leadshine", "Weintek",
                                                       "Micro-Epsilon", "Simco", "ABB")), "")
    pfad = symbolbau.kasten2(rel, typ, [(a.name, "") for a in oben], [(a.name, "") for a in unten],
                             hersteller=hersteller, beschreibung=bt.symbol, abstand=30)
    return pfad, {id(a): a.name for a in oben + unten}


def _nach_seite(bt, sym):
    """Anschlüsse eines Normsymbols den sPlan-Anschlüssen zuordnen: gleicher Name, sonst gleiche Seite in
    Reihenfolge von links nach rechts."""
    zu, frei = {}, list(sym.anschluesse)
    for a in bt.anschluesse:
        t = next((s for s in frei if s.name and s.name == a.name), None)
        if t:
            zu[id(a)] = t.index
            frei.remove(t)
    for seite, richt in ((("oben", "links"), "n"), (("unten", "rechts"), "s")):
        rest = sorted([a for a in bt.anschluesse if id(a) not in zu and a.seite in seite], key=lambda a: a.pos)
        kand = sorted([s for s in frei if s.richtung == richt], key=lambda s: s.x)
        for a, s in zip(rest, kand):
            zu[id(a)] = s.index
            frei.remove(s)
    for a in bt.anschluesse:                       # Rest (mehr Anschlüsse auf einer Seite als das Symbol hat)
        if id(a) not in zu and frei:
            zu[id(a)] = frei.pop(0).index
    return zu


def symbol_fuer(bt, art):
    """(Pfad, {id(Anschluss): Anschluss im Symbol (Name oder Index)})"""
    norm = {"schliesser": S.SCHLIESSER, "oeffner": S.OEFFNER, "schalter": S.SCHLIESSER, "spule": S.SPULE,
            "klemme": S.KLEMME, "ls3": S.MOTORSCHUTZ, "fi": S.FI_LS, "ntc": NTC, "motor1": MOTOR_1,
            "motor_dc": MOTOR_DC}
    if art in norm:
        pfad = norm[art]
        zu = _nach_seite(bt, S.symbol(pfad))
        if len(zu) == len(bt.anschluesse):
            return pfad, zu
    return kasten_symbol(bt)


def _sym_box(pfad):
    """Lage des Symbols relativ zum Einfügepunkt: (x0, y0, x1, y1) – aus dem tatsächlich Gezeichneten. Der
    deklarierte Rahmen (width/height) ist bei Herstellermakros oft viel größer (Kanalmakro S7-1200: 140 × 140 px
    deklariert, 100 × 70 px gezeichnet) und kostet sonst je Zeile ein Drittel der Blatthöhe."""
    import xml.etree.ElementTree as ET
    r = ET.parse(S.datei(pfad)).getroot()
    hx, hy = float(r.get("hotspot_x", 0)), float(r.get("hotspot_y", 0))
    dekl = (-hx, -hy, float(r.get("width", 0)) - hx, float(r.get("height", 0)) - hy)
    xs, ys = [], []

    def f(e, k, v=0.0):
        try:
            return float(e.get(k, v))
        except (TypeError, ValueError):
            return v
    for e in r.iter():
        t = e.tag
        if t == "line":
            xs += [f(e, "x1"), f(e, "x2")]
            ys += [f(e, "y1"), f(e, "y2")]
        elif t in ("rect", "ellipse", "arc"):
            xs += [f(e, "x"), f(e, "x") + f(e, "width")]
            ys += [f(e, "y"), f(e, "y") + f(e, "height")]
        elif t == "circle":
            xs += [f(e, "x"), f(e, "x") + f(e, "diameter")]
            ys += [f(e, "y"), f(e, "y") + f(e, "diameter")]
        elif t == "polygon":
            k = 1
            while e.get(f"x{k}") is not None:
                xs.append(f(e, f"x{k}"))
                ys.append(f(e, f"y{k}"))
                k += 1
        elif t == "text":
            xs += [f(e, "x"), f(e, "x") + 4 * len(e.get("text", ""))]
            ys += [f(e, "y") - 8, f(e, "y")]
        elif t == "dynamic_text":
            w = f(e, "text_width", -1)
            xs += [f(e, "x"), f(e, "x") + (w if w > 0 else 30)]
            ys += [f(e, "y"), f(e, "y") + 12]
        elif t == "terminal":
            xs.append(f(e, "x"))
            ys.append(f(e, "y"))
    if not xs:
        return dekl
    g = (min(xs) - 4, min(ys) - 4, max(xs) + 4, max(ys) + 4)
    # nie größer als deklariert (Texte am Rand werden sonst überschätzt)
    return (max(g[0], dekl[0]), max(g[1], dekl[1]), min(g[2], dekl[2]), min(g[3], dekl[3]))


# --- ein Blatt zeichnen --------------------------------------------------------------------------------
class Geraet:
    def __init__(self, bt, art, pfad, zu, label):
        self.bt, self.art, self.pfad, self.zu, self.label = bt, art, pfad, zu, label
        self.box = _sym_box(pfad)
        self.teil, self.leiste, self.x, self.y = None, None, 0, 0
        self.infos, self.adressen = {}, {}         # Artikeldaten aus der Zuordnung; Kanal: id(Anschluss) -> Adresse

    def anschluss(self, a):
        return self.zu[id(a)]

    def pin_dx(self, a):
        return S.symbol(self.pfad).anschluss(self.zu[id(a)]).x


def _ist_schiene(nz):
    if not nz.name:
        return False
    lang = any(abs(p[0] - q[0]) > 1500 for ln in nz.leitungen for p, q in zip(ln["pts"], ln["pts"][1:]))
    return bool(lang or (POT_NAME.match(nz.name) and len(nz.anschluesse) >= 3))


def _schiene_unten(nz):
    if POT_UNTEN.match(nz.name):
        return True
    ys = [p[1] for ln in nz.leitungen for p in ln["pts"]]
    return bool(ys) and sum(ys) / len(ys) > 1100


def zeichne_blatt(plan, inh, nummer, fragen, kontakte_global):
    b = plan.blatt(inh.titel or f"Blatt {nummer}", "", "", nummer)
    geraete = []
    for bt in inh.bauteile:
        if not bt.anschluesse and not bt.bez and not bt.wert:
            continue                                # reine Zeichnung ohne Anschluss (Rahmen, Logo …)
        art = klassifiziere(bt)
        regel = ZUORDNUNG.finde(bt, int(nummer))
        if regel and regel.kanaele:
            # Herstellermakros: je Gruppe von Anschlüssen ein Makro (SPS: Kanalmakro je Ein-/Ausgang, EPLAN-Stil);
            # alle Makros eines Geräts bilden eine Einheit wie eine Klemmleiste
            gr, rest = M.gruppen(bt, regel, fragen, nummer)
            for pfad, liste in gr:
                teil = A.Bauteil.__new__(A.Bauteil)
                teil.__dict__.update(bt.__dict__)
                teil.anschluesse = [a for a, _, _ in liste]
                teil.id = teil.parent = 0
                g = Geraet(teil, "kanal", pfad, {id(a): an for a, an, _ in liste}, bt.bez)
                g.leiste, g.infos = ("SPS", bt.bez, id(bt)), regel.infos()
                g.adressen = {id(a): adr for a, _, adr in liste}
                geraete.append(g)
            if rest:
                fragen.append(f"Blatt {nummer}: {bt.bez or bt.symbol}: Anschlüsse ohne Makro in der Zuordnung "
                              f"({', '.join(a.name for a in rest)}) – als Kasten gezeichnet.")
                teil = A.Bauteil.__new__(A.Bauteil)
                teil.__dict__.update(bt.__dict__)
                teil.anschluesse = rest
                pfad, zu = kasten_symbol(teil)
                g = Geraet(teil, "kasten", pfad, zu, bt.bez)
                g.infos = regel.infos()
                geraete.append(g)
            continue
        if art == "klemmen":                        # mehrstöckig: je Ebene eine Reihenklemme
            for e in sorted({getattr(a, "ebene", 1) for a in bt.anschluesse}):
                teil = A.Bauteil.__new__(A.Bauteil)
                teil.__dict__.update(bt.__dict__)
                teil.anschluesse = [a for a in bt.anschluesse if getattr(a, "ebene", 1) == e]
                pfad, zu = symbol_fuer(teil, "klemme")
                g = Geraet(teil, "klemme", pfad, zu, str(e))
                g.leiste = _leiste(bt)
                geraete.append(g)
            continue
        pfad, zu = symbol_fuer(bt, art)
        if pfad.startswith("company://" + KASTEN_ORDNER):
            art = "kasten"
        g = Geraet(bt, art, pfad, zu, bt.bez)
        if regel:                                   # Normsymbol oder Kasten, aber mit Artikeldaten
            g.infos = regel.infos()
        if art == "klemme":
            g.leiste = _leiste(bt)
        geraete.append(g)

    # Schienen: oben (+ Potentiale), unten (0 V, M, N, PE)
    schienen = [nz for nz in inh.netze if _ist_schiene(nz)]
    namen_frei = {}
    for nz in schienen:                             # gleicher Name zweimal auf dem Blatt: zweite bekommt Zusatz
        namen_frei[nz.name] = namen_frei.get(nz.name, 0) + 1
        if namen_frei[nz.name] > 1:
            nz.namen.insert(0, f"{nz.name} ({namen_frei[nz.name]})")
    oben = [nz for nz in schienen if not _schiene_unten(nz)]
    unten = [nz for nz in schienen if _schiene_unten(nz)]
    for k, nz in enumerate(oben):
        b.schiene(nz.name, y=80 + 20 * k)
    for k, nz in enumerate(unten):
        b.schiene(nz.name, y=RAIL_UNTEN_Y - 20 * k)
    y_start = 80 + 20 * len(oben) + 70
    y_ende = (RAIL_UNTEN_Y - 20 * len(unten)) - 40

    # Bänder; in jedem Band Reihenfolge wie im Original (links -> rechts, dann oben -> unten)
    platz = {}                                      # id(Anschluss) -> (x, y)
    # Zeilen in der Reihenfolge des Originals (oben -> unten): was sich in sPlan in der Höhe überdeckt, steht in
    # einer Zeile; eine Klemmleiste ist immer eine geschlossene Einheit. So bleibt der Signalfluss des Zeichners
    # erhalten, die Lage selbst kommt aus dem Raster.
    einheiten = {}
    for g in geraete:
        reihe = getattr(g.bt, "_reihe", None)
        if g.art == "kanal":                       # Makros eines Geräts bleiben zusammen (Reihenfolge der Zuordnung)
            schl = ("K", g.leiste)
        else:
            schl = ("L", g.leiste or reihe) if g.art == "klemme" and (g.leiste or reihe) else ("G", id(g))
        einheiten.setdefault(schl, []).append(g)
    einheiten = list(einheiten.values())
    for eh in einheiten:
        eh.sort(key=lambda g: (_num(g.bt.bez), _num(g.label)) if g.art == "klemme" else 0)
    # Kanalmakros einer Baugruppe: in Stücke teilen, die in eine Zeile passen (sonst läuft eine CPU mit 20 Kanälen
    # über den Blattrand); die Stücke stehen in derselben Zeilengruppe und brechen wie Einheiten um
    geteilt = []
    for eh in einheiten:
        if eh[0].art != "kanal":
            geteilt.append(eh)
            continue
        # Eingänge (Leitung nach oben) und Ausgänge (nach unten) in getrennte Zeilen: eine gemischte Zeile braucht
        # oben und unten Platz für Verweispfeile und frisst ein ganzes Blatt
        def _ri(g):
            return S.symbol(g.pfad).anschluss(g.zu[id(g.bt.anschluesse[0])]).richtung
        eh = sorted(eh, key=lambda g: 0 if _ri(g) == "n" else 1)
        stueck, breite = [], 0
        for g in eh:
            g.ri = _ri(g)
            w = g.box[2] - g.box[0] + 20
            if stueck and (breite + w > X_RECHTS - X_LINKS - 40 or _ri(g) != _ri(stueck[-1])):
                geteilt.append(stueck)
                stueck, breite = [], 0
            stueck.append(g)
            breite += w
        geteilt.append(stueck)
    einheiten = geteilt

    def yb(eh):
        return min(g.bt.bbox[1] for g in eh), max(g.bt.bbox[3] for g in eh)
    einheiten.sort(key=lambda eh: (sum(yb(eh)) / 2, min(g.bt.bbox[0] for g in eh)))
    roh = []
    for eh in einheiten:
        y0, y1 = yb(eh)
        if roh and y0 < roh[-1][1] - 0.25 * (y1 - y0):     # überdeckt die laufende Zeile
            roh[-1][0].append(eh)
            roh[-1][1] = max(roh[-1][1], y1)
        else:
            roh.append([[eh], y1])

    def schwerpunkt(eh):
        """mittlere Lage der schon gesetzten Partner (weniger Kreuzungen), sonst Lage im Original"""
        xs = [platz[id(b_)][0] for g in eh for a in g.bt.anschluesse
              for b_ in (a.netz.anschluesse if a.netz else []) if id(b_) in platz and b_.bt is not g.bt]
        return sum(xs) / len(xs) if xs else X_LINKS + min(g.bt.bbox[0] for g in eh) * 0.5

    zeilen = []
    for gruppe, _ in roh:
        gruppe.sort(key=lambda eh: min(g.bt.bbox[0] for g in eh))
        if zeilen:
            gruppe.sort(key=schwerpunkt)
        zeile, x = [], X_LINKS
        breiten = [sum(g.box[2] - g.box[0] + ({"klemme": 10, "kanal": 20}.get(g.art, 60)) for g in eh) + 30 for eh in gruppe]
        for k, eh in enumerate(gruppe):
            breite = breiten[k]
            rest = sum(breiten[k + 1:])             # was in dieser Zeile noch Platz braucht
            andere_richtung = eh[0].art == "kanal" and any(
                z.art == "kanal" and z.ri != eh[0].ri for z in zeile)
            if zeile and (x + breite > X_RECHTS or andere_richtung):
                zeilen.append(zeile)
                zeile, x = [], X_LINKS
            if eh[0].art == "klemme" and eh[0].leiste:
                x += 50                             # Platz für den Leistennamen links
            for g in eh:
                luecke = {"klemme": 10, "kanal": 20}.get(g.art) or 60 + (int(len(g.bt.wert) * 4.5)
                                                            if g.art != "kasten" and g.bt.wert else 0)
                ziel = None                         # unter/über dem Partner ausrichten (senkrechte Leitung)
                if g.art not in ("klemme", "kanal"):
                    for a in g.bt.anschluesse:
                        for b_ in (a.netz.anschluesse if a.netz else []):
                            if id(b_) in platz and b_.bt is not g.bt:
                                ziel = platz[id(b_)][0] - g.pin_dx(a)
                                break
                        if ziel is not None:
                            break
                gx = x - g.box[0]
                if ziel is not None and ziel >= gx and ziel + g.box[2] + rest <= X_RECHTS:
                    gx = ziel
                g.x = gx = raster(gx)
                zeile.append(g)
                x = gx + g.box[2] + luecke
                for a in g.bt.anschluesse:
                    platz[id(a)] = (gx + g.pin_dx(a), 0)
            x += 30
        if zeile:
            zeilen.append(zeile)

    zeile_von = {}
    for r, zeile in enumerate(zeilen):
        for g in zeile:
            zeile_von[id(g)] = r
    teil_von = {}
    for g in geraete:
        for a in g.bt.anschluesse:
            teil_von[id(a)] = g

    def richtung(a):
        g = teil_von[id(a)]
        return S.symbol(g.pfad).anschluss(g.anschluss(a)).richtung

    def frei_rechts(a, zeile):
        """Platz rechts neben dem Anschluss bis zum nächsten Anschluss gleicher Richtung in der Zeile (px)"""
        ax, r = platz[id(a)][0], richtung(a)
        xs = [platz[id(b_)][0] for g in zeile for b_ in g.bt.anschluesse
              if id(b_) in platz and b_ is not a and richtung(b_) == r and platz[id(b_)][0] > ax + 1]
        return (min(xs) if xs else X_RECHTS + 60) - ax - 14

    def pfeile(zeile, richt):
        """Platz für Verweispfeile nach oben/unten: Pfeil + Querverweis; der Name steht waagerecht rechts daneben
        (umbrochen auf den freien Platz), nur wo kein Platz ist, senkrecht wie bisher (frisst viel Höhe)."""
        senk, zeilen_n = 0, 0
        for g in zeile:
            for a in g.bt.anschluesse:
                if a.netz and a.netz.namen and a.netz not in schienen and richtung(a) == richt:
                    z = _umbruch(a.netz.name, frei_rechts(a, zeile))
                    if z is None:
                        senk = max(senk, len(a.netz.name))
                    else:
                        zeilen_n = max(zeilen_n, len(z))
        if senk:
            return 60 + 7 * min(senk, 20)
        return 0 if not zeilen_n else 110 + 10 * max(0, zeilen_n - 4)

    # Verbindungen je Netz als kürzester Baum (nicht als Kette), dann Kanäle zwischen den Zeilen belegen:
    # jede Leitung bekommt eine eigene Höhe (Spur), damit parallele Leitungen nie aufeinander liegen.
    paare = []
    for nz in inh.netze:
        if nz in schienen:
            continue
        pins = [a for a in nz.anschluesse if id(a) in teil_von]
        if len(pins) < 2:
            continue

        def schaetz(a):
            r = zeile_von[id(teil_von[id(a)])]
            return platz[id(a)][0], r * 400 + (300 if richtung(a) == "s" else 0)
        im_baum, rest = [pins[0]], pins[1:]
        while rest:
            d, p, q = min(((abs(schaetz(p)[0] - schaetz(q)[0]) + abs(schaetz(p)[1] - schaetz(q)[1]), p, q)
                           for p in im_baum for q in rest), key=lambda t: t[0])
            paare.append((p, q))
            im_baum.append(q)
            rest.remove(q)
    kanal_von, spuren = {}, {}
    for p, q in paare:
        rp, rq = zeile_von[id(teil_von[id(p)])], zeile_von[id(teil_von[id(q)])]
        dp, dq = richtung(p), richtung(q)
        if rp > rq:
            p, q, rp, rq, dp, dq = q, p, rq, rp, dq, dp
        k = None
        if dp == "s" and dq == "n" and rq == rp + 1:
            k = rp
        elif dp == dq == "s" and rp == rq:
            k = rp
        elif dp == dq == "n" and rp == rq:
            k = rp - 1
        x1, x2 = platz[id(p)][0], platz[id(q)][0]
        if k is None or (abs(x1 - x2) < 1 and dp != dq):
            continue
        lo, hi = min(x1, x2) - 5, max(x1, x2) + 5
        belegt = spuren.setdefault(k, [])
        t = 0
        while any(t == s and not (hi < l2 or lo > h2) for s, l2, h2 in belegt):
            t += 1
        belegt.append((t, lo, hi))
        kanal_von[(id(p), id(q))] = (k, t)

    def n_spuren(k):
        return 1 + max((t for t, _, _ in spuren.get(k, [])), default=-1)

    # Zeilen von oben nach unten; passt eine Zeile nicht mehr aufs Blatt, geht es auf einem Folgeblatt weiter
    # (wie EPLAN: nichts wird zusammengeschoben, Verbindungen zum Folgeblatt laufen über Verweispfeile)
    y = y_start + n_spuren(-1) * 10
    kanal_y = {-1: y_start}
    seite_von, seite = {}, 0
    for r, zeile in enumerate(zeilen):
        h_oben = max(-g.box[1] for g in zeile)
        h_unten = max(g.box[3] for g in zeile)
        for versuch in (0, 1):
            gy = raster(y + h_oben + 30 + pfeile(zeile, "n"))
            ky = raster(gy + h_unten + 20 + pfeile(zeile, "s"))
            if versuch or ky + n_spuren(r) * 10 <= y_ende or not any(v == seite for v in seite_von.values()):
                break
            seite += 1                              # Folgeblatt
            y = y_start + n_spuren(r - 1) * 10
        seite_von[r] = seite
        if os.environ.get("SPLAN_DEBUG"):
            print("ZEILE", nummer, r, seite, len(zeile), "y", y, "oben", h_oben, pfeile(zeile, "n"), "unten", h_unten, pfeile(zeile, "s"), "spuren", n_spuren(r), "ende", y_ende, [(g.pfad.split("/")[-1][-14:], g.box[1], g.box[3]) for g in zeile])
        for g in zeile:
            g.y, g.seite = gy, seite
        kanal_y[r] = ky
        y = kanal_y[r] + n_spuren(r) * 10 + 20
    blaetter = [b] + [plan.blatt(f"{inh.titel or 'Blatt ' + nummer} ({k + 1})", "", "", f"{nummer}.{k}")
                      for k in range(1, seite + 1)]
    if seite:
        fragen.append(f"Blatt {nummer}: Inhalt passt nicht auf ein Blatt – auf {seite + 1} Blätter verteilt "
                      f"(Verbindungen zwischen ihnen als Verweispfeile).")
        for k, bb in enumerate(blaetter[1:], 1):    # Schienen auf jedem Folgeblatt
            for j, nz in enumerate(oben):
                bb.schiene(nz.name, y=80 + 20 * j)
            for j, nz in enumerate(unten):
                bb.schiene(nz.name, y=RAIL_UNTEN_Y - 20 * j)

    def kanal_hoehe(k, r):
        """Spur-y für Kanal k, gesehen von Zeile r (oberster Kanal eines Folgeblatts liegt unter den Schienen)"""
        return kanal_y[k] if k >= 0 and seite_von.get(k) == seite_von.get(r) else y_start

    # setzen
    for g in geraete:
        infos = {"description": " ".join(g.bt.beschreibung)[:120]} if g.bt.beschreibung else {}
        if g.bt.zusatz:
            infos["designation"] = " ".join(g.bt.zusatz)[:80]
        infos.update(g.infos)
        felder = None
        if g.art == "kanal":
            # Kanalmakro: Adresse, symbolische Adresse (sPlan kennt keine), Funktion = Ziel der Leitung
            a = g.bt.anschluesse[0]
            ziel = ""
            if a.netz:
                ziel = a.netz.name or ", ".join(str(x) for x in a.netz.anschluesse if x.bt.bez != g.bt.bez)
            adr = " ".join(v for v in (g.adressen.get(id(x), "") for x in g.bt.anschluesse) if v)
            felder = [adr, "", ziel if len(g.bt.anschluesse) == 1 else ""]
            infos.pop("description", None)
        b = blaetter[getattr(g, "seite", 0)]
        g.blatt = b
        g.teil = b.setze(g.pfad, g.x, g.y, label=g.label, infos=infos, felder=felder)
        for a in g.bt.anschluesse:
            platz[id(a)] = g.teil.pos(g.anschluss(a))
        if g.art in ("schliesser", "oeffner", "schalter", "ls3", "fi", "ntc", "motor1", "motor_dc", "spule"):
            # Anschlussnummern aus sPlan an das Normsymbol schreiben (wo das Symbol sie nicht selbst zeigt)
            for a in g.bt.anschluesse:
                s = S.symbol(g.pfad).anschluss(g.anschluss(a))
                if s.name == a.name or not a.name or re.match(r"^.+\.\d+$", a.name):
                    continue
                px, py = g.teil.pos(g.anschluss(a))
                b.text(a.name, px + 3, py + (-14 if s.richtung == "n" else 2), "klemme")
        if g.bt.wert and g.art not in ("kasten", "kanal") and g.bt.wert.lower() not in ("schütz", "motor", "schliesser", "öffner"):
            b.text(g.bt.wert, g.x + g.box[2] + 4, g.y + g.box[1] + 12, "daten")
        if g.bt.beschreibung and g.art not in ("klemme", "kanal"):
            b.text(" ".join(g.bt.beschreibung)[:40], g.x + g.box[0], g.y + g.box[3] + 6, "daten")
        if g.bt.id:
            kontakte_global.append((b, g))
    # Leistennamen links vor die erste Klemme jeder Leiste
    gesehen = set()
    for g in sorted([g for g in geraete if g.art == "klemme"], key=lambda g: (g.y, g.x)):
        if (g.leiste, g.y) not in gesehen:
            gesehen.add((g.leiste, g.y))
            if g.leiste:
                g.blatt.rechts_text(g.leiste, g.x - 14, g.y - 8, "label")
            else:
                fragen.append(f"Blatt {nummer}: Klemmen {g.bt.bez}… ohne erkennbaren Leistennamen.")

    teil_von = {}
    for g in geraete:
        for a in g.bt.anschluesse:
            teil_von[id(a)] = g

    def _pfeil_an(g, a, name, extras=()):
        b = g.blatt
        px, py = g.teil.pos(g.anschluss(a))
        nach_oben = S.symbol(g.pfad).anschluss(g.anschluss(a)).richtung == "n"
        richt = "von_oben" if nach_oben else "ab"
        ay = py - 40 if nach_oben else py + 40
        z = _umbruch(name, frei_rechts(a, zeilen[zeile_von[id(g)]]))
        t = b.pfeil(name, px, ay, richt, text=z is None)
        if z is not None:                           # waagerecht rechts neben dem Pfeil, oben bündig
            y0 = ay - 12 - 10 * (len(z) - 1) if nach_oben else ay + 4
            for k, zeile_ in enumerate(z):
                b.text(zeile_, px + 6, y0 + 10 * k, "verweis")
        b.draht(t, 0, g.teil, g.anschluss(a))
        for k, extra in enumerate(extras):
            if z is not None:                       # unter (bzw. über) den Namenszeilen weiter
                b.text(extra, px + 6, (y0 - 10 * (k + 1)) if nach_oben else (y0 + 10 * (len(z) + k)), "verweis")
            else:
                b.text(extra, px + 6, ay + (-30 if nach_oben else 20), "verweis")


    # Leitungen
    for nz in inh.netze:
        pins = [a for a in nz.anschluesse if id(a) in teil_von]
        if nz in schienen:
            for a in pins:
                g = teil_von[id(a)]
                px, py = g.teil.pos(g.anschluss(a))
                richt = S.symbol(g.pfad).anschluss(g.anschluss(a)).richtung
                if (nz in unten and richt == "n") or (nz not in unten and richt == "s"):
                    # Anschluss zeigt von der Schiene weg: neben dem Kasten vorbei statt durch das Gerät
                    xr = raster(g.x + g.box[2] + 10)
                    yb = py - 20 if richt == "n" else py + 20
                    t, an = g.blatt.abgriff(nz.name, xr, "oben" if nz in unten else "unten")
                    g.blatt.leitung(t, an, g.teil, g.anschluss(a), ueber=[(xr, yb), (px, yb)])
                    continue
                t, an = g.blatt.abgriff(nz.name, px, "oben" if nz in unten else "unten")
                g.blatt.draht(t, an, g.teil, g.anschluss(a))
            weitere = [x for x in dict.fromkeys(nz.namen[1:]) if x != nz.name]
            b = blaetter[0]
            if weitere:
                s = b.schienen[nz.name]
                x0 = min([px for px, _ in s.abgriffe] + [s.x1]) if s.anfang is False else s.x1
                b.text("= " + ", ".join(weitere), x0 + 8 + 9 * len(nz.name), s.y - 20, "verweis")
            continue
        if not pins:
            if nz.namen:
                fragen.append(f"Blatt {nummer}: Potential „{nz.name}“ ist gezeichnet, hängt aber an keinem Bauteil.")
            continue
        pins.sort(key=lambda a: platz[id(a)])
        if nz.namen:
            # Verweispfeil am ersten Anschluss: nach oben weg, wenn der Anschluss oben liegt, sonst nach unten
            a = pins[0]
            g = teil_von[id(a)]
            _pfeil_an(g, a, nz.name, nz.namen[1:])

    for p, q in paare:
        g1, g2 = teil_von[id(p)], teil_von[id(q)]
        if g1.blatt is not g2.blatt:
            # über die Blattgrenze: auf beiden Seiten ein Verweispfeil gleichen Namens (verknüpft beim Erzeugen)
            name = p.netz.name if p.netz and p.netz.name else min(str(p), str(q))
            _pfeil_an(g1, p, name)
            _pfeil_an(g2, q, name)
            continue
        b = g1.blatt
        k = kanal_von.get((id(p), id(q))) or kanal_von.get((id(q), id(p)))
        if k is None:
            b.draht(g1.teil, g1.anschluss(p), g2.teil, g2.anschluss(q))
            continue
        yt = kanal_hoehe(k[0], zeile_von[id(g1)]) + 10 * k[1]
        x1, x2 = platz[id(p)][0], platz[id(q)][0]
        if abs(x1 - x2) < 1:
            b.draht(g1.teil, g1.anschluss(p), g2.teil, g2.anschluss(q))
        else:
            b.leitung(g1.teil, g1.anschluss(p), g2.teil, g2.anschluss(q), ueber=[(x1, yt), (x2, yt)])

    # freie Texte ohne Bauteil: unten als Hinweis (nichts geht verloren)
    b = blaetter[0]
    rest = [t["text"].replace("\n", " ").strip() for t in inh.texte if t["text"].strip()]
    if rest:
        zeile, yy = "", 948
        for t in rest:
            if zeile and len(zeile) + len(t) > 170:
                b.text(zeile, 40, yy, "daten")
                zeile, yy = "", yy + 11
            zeile = (zeile + "  ·  " if zeile else "Texte aus sPlan: ") + t
        if zeile:
            b.text(zeile, 40, yy, "daten")
    return b, geraete, [bi for bi in inh.bilder if not bi.get("_geraet")]


ZEICHEN_PX = 4.3                                   # Breite eines Zeichens der Größe „verweis“ (7 pt)


def _umbruch(text, breite):
    """Text auf Zeilen der Breite `breite` (px) umbrechen; None, wenn nicht einmal 6 Zeichen nebeneinander passen."""
    n = int(breite / ZEICHEN_PX)
    if n < 6:
        return None
    zeilen, z = [], ""
    for w in text.split():
        while len(w) > n:                           # überlange Wörter hart teilen
            if z:
                zeilen.append(z)
                z = ""
            zeilen.append(w[:n])
            w = w[n:]
        if z and len(z) + 1 + len(w) > n:
            zeilen.append(z)
            z = w
        else:
            z = (z + " " + w) if z else w
    if z:
        zeilen.append(z)
    return zeilen or [""]


def _num(s):
    m = re.match(r"\d+", s or "")
    return int(m.group(0)) if m else 0


def _leiste(bt):
    for t in bt.beschreibung:
        m = re.match(r"^-?(X\d+[\w.]*)$", t.strip())
        if m:
            return m.group(1)
    return getattr(bt, "_leiste", None)


def leisten_zuordnen(inh):
    """Klemmenleistenname (X1, X2 …) steht in sPlan als freier Text neben der Leiste. Klemmen einer Reihe
    (gleiche Höhe, lückenlos nebeneinander) bilden eine Leiste; der Name ist der X-Text am nächsten an ihr."""
    kl = sorted([bt for bt in inh.bauteile if A.ist_klemme(bt)], key=lambda bt: (round(bt.mitte[1] / 50), bt.bbox[0]))
    reihen, akt = [], []
    for bt in kl:
        if akt and (abs(bt.mitte[1] - akt[-1].mitte[1]) > 40 or bt.bbox[0] - akt[-1].bbox[2] > 25):
            reihen.append(akt)
            akt = []
        akt.append(bt)
    if akt:
        reihen.append(akt)
    kand = [(t["text"].strip(), A._mitte(t["pts"]), t) for t in inh.texte if re.match(r"^-?X\d+[\w.]*$",
                                                                                         t["text"].strip())]
    kand += [(s.strip(), bt.mitte, None) for bt in kl for s in bt.beschreibung if re.match(r"^-?X\d+[\w.]*$",
                                                                                         s.strip())]
    for k, r in enumerate(reihen):
        for bt in r:
            bt._reihe = f"Reihe{k + 1}"
        x0, y0 = r[0].bbox[0], min(b.bbox[1] for b in r)
        best = None
        for name, m, t in kand:
            d = abs(m[0] - x0) + abs(m[1] - y0)
            if m[0] <= r[-1].bbox[2] + 20 and d < 400 and (best is None or d < best[0]):
                best = (d, name, t)
        if best:
            name = best[1].lstrip("-")
            for bt in r:
                bt._leiste = name
                bt.beschreibung = [s for s in bt.beschreibung if s.strip().lstrip("-") != name]
            if best[2] is not None and best[2] in inh.texte:
                inh.texte.remove(best[2])


def _bild_png(o, ordner, name):
    from PIL import Image
    os.makedirs(ordner, exist_ok=True)
    pfad = os.path.join(ordner, name + ".png")
    Image.open(io.BytesIO(o["bmp"])).save(pfad)
    return pfad


def doku_blatt(plan, titel, nummer, bilder, texte, bilder_ordner):
    """Blatt ohne Schaltungsinhalt (Datenblattauszüge, Fotos): Bilder und Texte maßstäblich übernehmen
    (0,1 mm -> px: Faktor 0,5; A4-Inhalt auf dem A3-Blatt)."""
    from PIL import Image
    b = plan.blatt(titel, "", "", nummer)
    f = 0.5
    for k, o in enumerate(bilder):
        x0, y0, x1, y1 = A._bbox(o["pts"])
        try:
            png = _bild_png(o, bilder_ordner, f"blatt{nummer}_bild{k + 1}")
        except Exception as e:                     # noqa: BLE001
            plan.fragen.append(f"Blatt {nummer}: Bild {k + 1} nicht lesbar ({e}).")
            continue
        w = Image.open(png).size[0]
        ident = f"img{nummer.replace('.', '_')}_{k}"
        b.ops.append({"op": "add_image", "folio": b.folio, "file": png, "x": round(60 + x0 * f, 1),
                      "y": round(40 + y0 * f, 1), "id": ident})
        faktor = (x1 - x0) * f / max(1, w)
        if abs(faktor - 1) > 0.01:
            b.ops.append({"op": "scale_image", "folio": b.folio, "index": f"${ident}", "factor": round(faktor, 4)})
    for t in texte:
        if t.get("drehung", 0) not in (0, 90, -90):
            continue
        x, y = t["pts"][0]
        groesse = max(5, min(14, round(t.get("hoehe", 30) * f * 0.55)))
        dreh = {0: 0, 90: 90, -90: 270}[t.get("drehung", 0)]
        for i, z in enumerate(t["text"].split("\n")):
            if z.strip():
                b.text(z, 60 + x * f, 40 + y * f + i * groesse * 1.4, groesse, drehung=dreh)
    return b


def schriftfeld():
    name = "sPlan_Import"
    os.makedirs(TITELBLOECKE, exist_ok=True)

    def feld(r, c, wert, groesse=9, label="", rs=0, cs=0):
        lab = f'<label><translation lang="de">{label}</translation></label>'
        return (f'<field row="{r}" col="{c}" rowspan="{rs}" colspan="{cs}" name="" displaylabel="'
                f'{"true" if label else "false"}" align="left" valign="center" fontsize="{groesse}" hadjust="true">'
                f'<value><translation lang="de">{wert}</translation></value>{lab}</field>\n')
    xml = ('<titleblocktemplate name="sPlan_Import">\n<information>Schriftfeld des sPlan-Imports</information>\n'
           '<logos/>\n<grid rows="18;18;18" cols="300;700;300;r100%">\n'
           + feld(0, 0, "%author", 8, "Bearbeiter") + feld(1, 0, "%date", 8, "Datum")
           + feld(2, 0, "%{saveddate-de}", 7, "gespeichert")
           + feld(0, 1, "%projecttitle", 10, "Projekt") + feld(1, 1, "%title", 11, "Blatt", rs=1)
           + feld(0, 2, "%plant", 8, "Anlage") + feld(1, 2, "%locmach", 8, "Ort")
           + feld(2, 2, "Quelle: sPlan 8", 7)
           + feld(0, 3, "%folio", 11, "Blatt", rs=1) + feld(2, 3, "von %{folio-total}", 8)
           + '</grid>\n</titleblocktemplate>\n')
    open(os.path.join(TITELBLOECKE, name + ".titleblock"), "w", encoding="utf-8").write(xml)
    return name


def metadaten(blatt):
    """Schriftfeldtexte des sPlan-Rahmens (unterhalb der Zeichenfläche): Bezeichnung, Name, Datum."""
    texte = [o for o in blatt.objekte if o["art"] == "text" and o["pts"] and o["pts"][0][1] > A.RAND[3]]
    werte = [t["text"].strip() for t in texte]
    feste = {"Datum", "Name", "Änderungen", "gez.:", "gepr.:", "Norm:", "Bezeichnung", "Zeichnungs-Nr.:", "Blatt",
             "von", "Bezeichnung:"}
    datum = next((w for w in werte if re.match(r"^\d{1,2}\.\d{1,2}\.\d{2,4}$", w)), "")
    name = next((w for w in werte if re.match(r"^[A-ZÄÖÜ]\.\s?[A-Za-zäöüß]+$", w)), "")
    rest = [w for w in werte if w not in feste and w not in (datum, name) and not w.isdigit() and len(w) > 3]
    return {"titel": max(rest, key=len) if rest else "", "datum": datum, "autor": name}


def verknuepfe(kontakte_global, fragen):
    """Spule (Master) <-> Kontakte (Slave): sPlan-Parent-ID, sonst gleiches Kennzeichen (Schütz K3 / Schließer K3)."""
    masters, nach_label = {}, {}
    for b, g in kontakte_global:
        if g.art == "spule":
            masters[g.bt.id] = g
            nach_label.setdefault(g.label, g)
    for b, g in kontakte_global:
        if g.art not in ("schliesser", "oeffner") or S.symbol(g.pfad).link_type != "slave":
            continue
        m = (masters.get(g.bt.parent) if g.bt.parent else None) or nach_label.get(g.label)
        if m is None:
            fragen.append(f"Kontakt {g.label} (Blatt {b.nummer}): keine Spule {g.label} im Plan gefunden.")
            continue
        b.verbinde(m.teil, g.teil)


def umsetzen(quelle, ziel, pdf=False, vergleich=None, makros=None, nur_vorlage=False):
    global ZUORDNUNG
    _fortschritt(2, "lesen")
    pr = splan8.lese(quelle)
    ordner = os.path.dirname(os.path.abspath(ziel))
    os.makedirs(ordner, exist_ok=True)
    if nur_vorlage:
        inhalte = [(i, A.analysiere(bl, i)) for i, bl in enumerate(pr.blaetter, 1)]
        p = M.vorlage_schreiben(makros or M.standardpfad(ziel).replace("_makros.json", "_makros_vorlage.json"),
                                inhalte)
        _fortschritt(100, "fertig")
        return p
    ZUORDNUNG = M.lade_fuer(ziel, makros)
    meta = metadaten(pr.blaetter[0])
    plan = Plan(meta["titel"] or os.path.splitext(os.path.basename(quelle))[0], ordner, schriftfeld(),
                autor=meta["autor"] or "")
    plan.fragen = []
    bilder_ordner = os.path.join(ordner, os.path.splitext(os.path.basename(ziel))[0] + "_bilder")
    kontakte_global, zuordnung = [], []
    n = len(pr.blaetter)
    for i, bl in enumerate(pr.blaetter, 1):
        _fortschritt(5 + 70 * i / n, "blatt")
        inh = A.analysiere(bl, i)
        leisten_zuordnen(inh)
        fr = [f"Blatt {i} ({bl.name}): {f}" for f in inh.fragen]
        if not any(bt.anschluesse for bt in inh.bauteile):
            zuordnung.append((i, bl.name, len(plan.blaetter)))
            doku_blatt(plan, bl.name, str(i), inh.bilder, inh.texte, bilder_ordner)
            fr.append(f"Blatt {i} ({bl.name}): keine Schaltung (nur Bilder/Texte) – als Dokumentationsblatt "
                      f"übernommen, Bilder und Texte maßstäblich.")
        else:
            zuordnung.append((i, bl.name, len(plan.blaetter)))
            _, _, bilder = zeichne_blatt(plan, inh, str(i), fr, kontakte_global)
            if bilder:
                doku_blatt(plan, f"{bl.name} – Bilder", f"{i}.1", bilder, [], bilder_ordner)
        plan.fragen += fr
    verknuepfe(kontakte_global, plan.fragen)
    plan.artikel = [r for r in ZUORDNUNG.regeln if r.benutzt]
    for r in ZUORDNUNG.unbenutzt():
        plan.fragen.append(f"Zuordnung {r.wenn} passt auf kein Bauteil – Eintrag prüfen.")
    _fortschritt(80, "schreiben")
    erg, r = plan.erzeuge(os.path.basename(ziel), pdf=False)
    _fortschritt(88, "pruefen")
    plan.layout = layout_nachbessern(erg)
    fragen_schreiben(os.path.join(ordner, "Offene_Fragen.txt"), quelle, plan, meta)
    if pdf:
        _fortschritt(95, "pdf")
        from qet_direkt import q, BIN
        print("pdf:", q.tool_export(BIN, erg, "pdf", erg.replace(".qet", ".pdf"), timeout=1800).get("ok"))
        if vergleich:
            vergleich_pdf(vergleich, erg.replace(".qet", ".pdf"), zuordnung,
                          os.path.join(ordner, os.path.splitext(os.path.basename(ziel))[0] + "_Vergleich.pdf"))
    _fortschritt(100, "fertig")
    return erg


def layout_nachbessern(qet, runden=2):
    """QETs Layoutprüfung (qet_layout_check) laufen lassen und ihre Korrekturen (Rasterversatz, Ausrichtung)
    übernehmen. Rückgabe: Kennzahlen der letzten Prüfung."""
    from qet_direkt import q, BIN
    r = {}
    for _ in range(runden):
        r = q.tool_layout_check(BIN, qet, limit=2000)
        fixes = r.get("fixes") or []
        if not fixes:
            break
        tmp = qet[:-4] + "_nachbessern.qet"
        e = q.tool_edit(BIN, qet, fixes, tmp, timeout=1800)
        if not e.get("ok"):
            if os.path.exists(tmp):
                os.remove(tmp)
            break
        os.replace(tmp, qet)
    r = q.tool_layout_check(BIN, qet, limit=50)
    print("layout:", r.get("score"), r.get("summary"))
    return r


def fragen_schreiben(pfad, quelle, plan, meta):
    z = [f"Offene Fragen zum sPlan-Import von {os.path.basename(quelle)}",
         f"Projekt: {meta['titel']}  ·  Bearbeiter: {meta['autor']}  ·  Datum im Original: {meta['datum']}",
         "", "Diese Punkte konnte der Import nicht eindeutig aus der sPlan-Datei ableiten. Die genannte Annahme ist",
         "im QET-Projekt umgesetzt – bitte prüfen und dort korrigieren.", ""]
    lay = getattr(plan, "layout", None) or {}
    if lay.get("summary"):
        sm = lay["summary"]
        z += [f"Layoutprüfung (qet_layout_check): {lay.get('score')}/100 – {sm.get('straight_wires')} von "
              f"{sm.get('wires')} Leitungen gerade, {sm.get('labels_on_wires')} Texte auf Leitungen, "
              f"{sm.get('overlaps')} Überdeckungen.", ""]
    z += ["- " + f for f in plan.fragen]
    artikel = getattr(plan, "artikel", [])
    if artikel:
        z += ["", "Artikel aus der Zuordnungsdatei (sPlan kennt keine Artikel; Begründung je Gerät – bitte bestätigen):"]
        z += [f"- {r.wenn}: {r.hersteller} {r.artikel} {r.typ} – {r.beleg or 'ohne Beleg'}" for r in artikel]
    offen = getattr(plan, "offen", [])
    if offen:
        z += ["", "Verweispfeile ohne Gegenstück gleichen Namens auf einem anderen Blatt (das Ziel steht als Text",
              "am Pfeil, z. B. „X4:1“ = Leiste X4 Klemme 1):"]
        z += [f"- Blatt {plan.blaetter[f].nummer}: {nm}" for f, nm in offen]
    open(pfad, "w", encoding="utf-8").write("\n".join(z) + "\n")


def vergleich_pdf(original, neu, zuordnung, ziel):
    """Original-Seite links, neues Blatt rechts (wie die Vergleichs-PDFs der EPLAN-Nachbauten)."""
    import pymupdf as fitz
    o, nn = fitz.open(original), fitz.open(neu)
    out = fitz.open()
    for i, name, folio in zuordnung:
        if i - 1 >= len(o) or folio >= len(nn):
            continue
        r1, r2 = o[i - 1].rect, nn[folio].rect
        h2 = r2.height * r1.width / r2.width
        p = out.new_page(width=r1.width * 2 + 20, height=max(r1.height, h2) + 30)
        p.insert_text((10, 18), f"Original sPlan Blatt {i} ({name})", fontsize=11)
        p.insert_text((r1.width + 30, 18), "QET neu", fontsize=11)
        p.show_pdf_page(fitz.Rect(0, 25, r1.width, 25 + r1.height), o, i - 1)
        p.show_pdf_page(fitz.Rect(r1.width + 20, 25, 2 * r1.width + 20, 25 + h2), nn, folio)
    out.save(ziel)
    print("vergleich:", ziel)


if __name__ == "__main__":
    argv = sys.argv[1:]
    opt = {}
    for name in ("--vergleich", "--makros"):
        if name in argv:
            k = argv.index(name)
            opt[name] = argv[k + 1]
            del argv[k:k + 2]
    args = [a for a in argv if not a.startswith("--")]
    quelle = args[0]
    ziel = args[1] if len(args) > 1 else os.path.join(
        os.path.dirname(os.path.abspath(quelle)), "QET", _sicher(os.path.splitext(os.path.basename(quelle))[0]) + ".qet")
    print("fertig:", umsetzen(quelle, ziel, pdf="--pdf" in argv, vergleich=opt.get("--vergleich"),
                              makros=opt.get("--makros"), nur_vorlage="--makros-vorlage" in argv))
