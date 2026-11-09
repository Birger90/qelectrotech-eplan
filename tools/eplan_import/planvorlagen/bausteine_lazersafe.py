"""Bausteine für Geräte mit Steckern (Sicherheitssteuerung LazerSafe PCSS, Controller, Auswertegeräte).

Wie EPLAN: das Gerät ist EIN Bauteil – die Anschlussübersicht (Master, Art SPS) mit allen Steckern und Artikeldaten.
Auf den Funktionsblättern steht nur ein Rahmen des Geräts mit den benutzten Steckern; jeder benutzte Anschluss ist ein
Gerätepin (Slave) mit Stecker-Pin, Kanalname und Signal und hängt am Master. Die Querverweise der Übersicht
(„/53/2“ je Pin) entstehen am Ende aus den gesetzten Gerätepins (`uebersicht_verweise(plan)`), nicht von Hand.

    pfad = uebersicht_symbol("LazerSafe/…/PCSS-A0-Plus_Uebersicht.elmt", "LazerSafe PCSS-A0 Plus", STECKER, …)
    geraet_uebersicht(b49, pfad, {"bmk": "-A490", …}, 140, 120)            # Blatt Anschlussübersicht
    geraet_rahmen(b, "-A490", 180, 560, 980, 820, ["LazerSafe", "PCSS-A0 Plus"])
    pins = stecker(b, "-A490", "CN3", [("1", 300, "X00", ""), …], 560, "oben")
    …
    uebersicht_verweise(plan)                                                  # vor plan.erzeuge

Weitere Bausteine: Wechsler eines Relais (`wechsler`, `kontakt_zwischen`), Relais am DQ (`relais_am_dq`),
Geräteversorgung über Stecker (`geraet_versorgung`), serielle Schnittstelle über Kabel zu einem anderen Gerät
(`schnittstelle_kabel`), Feldgerät (Lichtschranke, Sensor mit Rundstecker) über Kabel/Klemmenkasten an einen Stecker
(`feldgeraet_an_stecker`), Gerätepin an SPS-Kanal (`pin_an_kanal`).
"""
import os
import uuid
from xml.sax.saxutils import escape

from .blatt import X0, SPALTE_B, raster, _dreh
from . import symbole as S
from .bausteine import _infos, kabel, klemmen, rahmen, sps_kanal

PIN = {"oben": "company://Allgemein/Verbindungen/Geraetepin_Plan/Geraetepin_oben_Plan.elmt",     # Leitung von oben
       "unten": "company://Allgemein/Verbindungen/Geraetepin_Plan/Geraetepin_unten_Plan.elmt"}   # Leitung nach unten
WECHSLER = "company://Allgemein/Schaltglieder/Wechsler_Original/Wechsler_Original_30.elmt"   # 1 oben, 2 Öffner, 4 Schließer +30
PE_GEHAEUSE = "company://Allgemein/Verbindungen/PE_Anschluss_Plan/PE-Anschluss_Gehaeuse_Plan.elmt"
RESERVE = "company://Allgemein/Verbindungen/Reserveader_Plan/Reserveader_Ende_unten_Plan.elmt"   # freies Aderende
PIN_PITCH = 30
SPULE_LABEL = (-64, -8)             # Kennzeichen links neben die Spule (rechts steht der Kontaktspiegel)
SCHRIFT = "Liberation Sans,{},-1,5,50,0,0,0,0,0,Regular"


def spalte(x):
    """Spaltennummer (ab 0, wie die Plan-Verweise) eines x auf dem Blatt."""
    return int((x - X0) // SPALTE_B)


def _register(plan):
    if not hasattr(plan, "geraete"):
        plan.geraete = {}
    return plan.geraete


# --- Gerätesymbol (Anschlussübersicht) -----------------------------------------------------------------
def uebersicht_symbol(rel, titel, stecker, breite, hoehe, hersteller="", artikel="", bestellnr="", beschreibung="",
                      extra=(), ueberschreiben=False):
    """Anschlussübersicht eines Geräts als EIN Symbol (Master, Art SPS) in die Firmensammlung schreiben.

    stecker: [{"name": "CN3", "x": 60, "y": 60, "pins": [("12", "X11"), …, ("1", "X00")], "unter": "005 RX"}]
      Pins von oben nach unten (20 px Raster); Anschluss je Pin rechts am Steckerkasten, Name „CN3:12“.
    extra: [(x, y, breite, hoehe, text)] weitere Kästen ohne Anschlüsse (Erweiterungsbus …).
    Gehäuse = Rechteck 0/0 … breite/hoehe, Gerätename unten links. Rückgabe company://-Pfad."""
    pfad = os.path.join(S.COMPANY, rel)
    ziel = "company://" + rel.replace("\\", "/")
    if os.path.exists(pfad) and not ueberschreiben:
        return ziel
    stil = "line-style:normal;line-weight:normal;filling:none;color:black"
    t = [f'<rect x="0" y="0" width="{breite}" height="{hoehe}" style="{stil}" antialias="false"/>']

    def text(x, y, s, g, rot=0):
        t.append(f'<text x="{x}" y="{y}" size="{g}" text="{escape(s)}" rotation="{rot}" color="#000000" '
                 f'font="{SCHRIFT.format(g)}"/>')
    for st in stecker:
        x, y, pins = st["x"], st["y"], st["pins"]
        h = 20 * len(pins) + 10
        t.append(f'<rect x="{x}" y="{y}" width="40" height="{h}" style="{stil}" antialias="false"/>')
        text(x, y - 5, st["name"], 9)
        for k, (nr, sig) in enumerate(pins):
            py = y + 15 + 20 * k
            t.append(f'<ellipse x="{x + 27}" y="{py - 3}" width="6" height="6" style="{stil}" antialias="true"/>')
            t.append(f'<line x1="{x + 33}" y1="{py}" x2="{x + 40}" y2="{py}" end1="none" end2="none" length1="1.5" '
                     f'length2="1.5" style="{stil}" antialias="false"/>')
            text(x + 3, py + 3, nr, 6)
            if sig:
                text(x + 12, py - 4, sig, 4)
            t.append(f'<terminal uuid="{{{uuid.uuid4()}}}" name="{escape(st["name"])}:{escape(nr)}" x="{x + 40}" '
                     f'y="{py}" orientation="e" type="Generic"/>')
        if st.get("unter"):
            text(x, y + h + 10, st["unter"], 6)
    for x, y, w, h, s in extra:
        t.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" style="{stil}" antialias="false"/>')
        text(x + 3, y + 12, s, 6)
    for i, z in enumerate(titel.split("\n")):
        text(20, hoehe - 70 + 18 * i, z, 14)
    infos = "".join(f'<elementInformation name="{k}" show="1">{escape(v)}</elementInformation>'
                    for k, v in (("manufacturer", hersteller), ("manufacturer_reference", bestellnr),
                                 ("designation", artikel), ("description", beschreibung)) if v)
    label = ('<dynamic_text x="-50" y="-4" z="2" text_width="-1" Halignment="AlignLeft" Valignment="AlignTop" '
             'frame="false" rotation="0" keep_visual_rotation="false" text_from="ElementInfo" '
             f'uuid="{{{uuid.uuid4()}}}" font="{SCHRIFT.format(10)}"><text></text><info_name>label</info_name>'
             '</dynamic_text>')
    xml = (f'<definition version="0.100.0" type="element" link_type="master" width="{breite + 80}" '
           f'height="{hoehe + 40}" hotspot_x="60" hotspot_y="30">\n'
           f'<uuid uuid="{{{uuid.uuid4()}}}"/>\n<names><name lang="de">{escape(titel.replace(chr(10), " "))} '
           f'(Anschlussübersicht)</name><name lang="en">{escape(titel.replace(chr(10), " "))} (overview)</name></names>\n'
           f'<elementInformations>{infos}<elementInformation name="label" show="1"></elementInformation>'
           '</elementInformations>\n<kindInformations><kindInformation name="type">plc</kindInformation>'
           '</kindInformations>\n<informations>Planvorlagen bausteine_lazersafe.uebersicht_symbol</informations>\n'
           f'<description>{label}{"".join(t)}</description>\n</definition>\n')
    os.makedirs(os.path.dirname(pfad), exist_ok=True)
    with open(pfad, "w", encoding="utf-8") as f:
        f.write(xml)
    qd = os.path.join(os.path.dirname(pfad), "qet_directory")
    if not os.path.exists(qd):
        name = os.path.basename(os.path.dirname(pfad))
        with open(qd, "w", encoding="utf-8") as f:
            f.write(f'<qet-directory>\n    <names>\n        <name lang="de">{escape(name)}</name>\n'
                    f'        <name lang="en">{escape(name)}</name>\n    </names>\n</qet-directory>\n')
    return ziel


def geraet_uebersicht(b, pfad, geraet, x, y, fremd=None):
    """Anschlussübersicht (Master) aufs Blatt setzen. fremd: {("CN13", "9"): "=DC/41/4"} – Verweise auf Blätter
    anderer Bereiche, die dieser Plan nicht selbst zeichnet."""
    infos = _infos(geraet)
    if geraet.get("bestellnr"):
        infos["manufacturer_reference"] = geraet["bestellnr"]
    t = b.setze(pfad, x, y, label=geraet["bmk"], infos=infos)
    _register(b.plan)[geraet["bmk"]] = {"master": t, "pins": {}, "fremd": dict(fremd or {})}
    return t


def uebersicht_verweise(plan, groesse=6):
    """Je benutzten Anschluss der Übersicht den Verweis „/Blatt/Spalte“ rechts neben den Pin schreiben (wie EPLAN),
    links oben am Gerät die Liste der Blätter mit Teilansichten. Aufrufen, wenn alle Blätter gebaut sind."""
    for bmk, g in _register(plan).items():
        m = g["master"]
        b = m.blatt
        ziele = {k: f"/{n}/{c}" for k, (n, c) in g["pins"].items()}
        ziele.update(g["fremd"])
        for (st, pin), s in sorted(ziele.items()):
            try:
                px, py = m.pos(f"{st}:{pin}")
            except (KeyError, IndexError):
                continue
            b.text(s, px + 4, py - 7, groesse)
        blaetter = sorted({n for n, _ in g["pins"].values()}, key=lambda n: int(n) if n.isdigit() else 0)
        for i, n in enumerate(blaetter):
            b.text(f"/{n}/0", m.x - 50, m.y + 14 + 9 * i, groesse)


# --- Teilansicht auf Funktionsblättern ------------------------------------------------------------------------
def geraet_rahmen(b, bmk, x1, y1, x2, y2, zeilen=(), text_xy=None, verweis=None):
    """Rahmen eines Geräts auf einem Funktionsblatt (wie die EPLAN-Teilansicht): Kennzeichen links oben außen,
    Gerätename (zeilen) in den Rahmen; verweis = Blatt der Übersicht („/49/0“)."""
    i = b._id("geraet")
    b.ops.append({"op": "add_shape", "folio": b.folio, "shape": "rectangle", "x1": x1, "y1": y1, "x2": x2, "y2": y2,
                  "id": i})
    b.rechts_text(bmk, x1 - 6, y1 + 2, "label")
    g = _register(b.plan).get(bmk)
    if verweis is None and g:                       # Verweis auf die Anschlussübersicht („/49/0“)
        verweis = f"/{g['master'].blatt.nummer}/{spalte(g['master'].x)}"
    if verweis:
        b.rechts_text(verweis, x1 - 6, y1 + 16, "daten")
    tx, ty = text_xy or ((x1 + x2) / 2, y2 - 16 * len(zeilen) - 12)
    for k, z in enumerate(zeilen):                 # Größe wie Funktionstexte (gleiche Texte, gleiche Größe)
        b.text(z, tx - 0.3 * 10 * len(z), ty + 15 * k, "funktion")


def geraetepin(b, bmk, st, pin, x, y, richtung="oben", kanal="", signal=""):
    """Ein Anschluss des Geräts als Gerätepin (Slave des Übersicht-Masters): Anschluss bei x/y, Leitung von oben
    („oben“) oder nach unten („unten“); Felder Pin, Kanal (X00), Signal. Kennzeichen nur als Datum."""
    t = b.setze(PIN[richtung], x, y, anker=0, label=bmk, felder=[str(pin), kanal, signal])
    b.ops += [{"op": "set_element_text", "folio": b.folio, "element": t.ref, "index": 0, "property": "source",
               "value": "text"},
              {"op": "set_element_text", "folio": b.folio, "element": t.ref, "index": 0, "property": "text",
               "value": ""}]
    g = _register(b.plan).get(bmk)
    if g:
        g["pins"][(st, str(pin))] = (b.nummer, spalte(x))
        b.verbinde(g["master"], t)
    return t


def stecker(b, bmk, name, pins, y, richtung="oben"):
    """Stecker eines Geräts in seinem Rahmen: Kasten mit Steckername, je Pin ein Gerätepin.
    pins: [(pin, x, kanal, signal)]; richtung "oben" = Kasten unter der Kante y (Leitungen kommen von oben),
    "unten" = Kasten über y (Leitungen gehen nach unten). Rückgabe {pin: Teil}."""
    xs = [p[1] for p in pins]
    x1, x2 = min(xs) - 40, max(xs) + 20
    y1, y2 = (y, y + 40) if richtung == "oben" else (y - 40, y)
    i = b._id("stecker")
    b.ops.append({"op": "add_shape", "folio": b.folio, "shape": "rectangle", "x1": x1, "y1": y1, "x2": x2, "y2": y2,
                  "id": i})
    b.text(name, x1 + 4, y1 + 12, "label")
    return {str(p): geraetepin(b, bmk, name, p, x, y, richtung, k, s) for p, x, k, s in pins}


# --- Relaiskontakte ------------------------------------------------------------------------------------------
def wechsler(b, x, y, master, bmk, nummern=("11", "12", "14")):
    """Wechsler eines Relais (Slave der Spule `master`, auch auf anderem Blatt): Wurzel bei x/y, Öffner darunter
    (Anschluss "2"), Schließer 30 px rechts ("4"). Kontaktnummern als Texte an die Anschlüsse."""
    w = b.setze(WECHSLER, x, y, anker="1", label=bmk)
    b.text(nummern[0], x + 4, y - 4, "daten")
    b.text(nummern[1], x - 14, y + 20, "daten")
    b.text(nummern[2], x + 34, y + 20, "daten")
    if master is not None:
        b.verbinde(master, w)
    return w


def kontakt_zwischen(b, a, an_a, z, an_z, master, bmk, nummern, schliesser, x_kontakt, y_kontakt, y_hin=None,
                     y_weg=None):
    """Wechsler zwischen zwei Anschlüssen: von (a, an_a) zur Wurzel, vom Öffner bzw. Schließer zu (z, an_z).
    Liegen die Anschlüsse nicht senkrecht in Linie, laufen die Leitungen waagerecht auf y_hin / y_weg."""
    w = wechsler(b, x_kontakt, y_kontakt, master, bmk, nummern)
    aus = "4" if schliesser else "2"
    ax = a.pos(an_a)[0]
    if abs(ax - x_kontakt) < 0.5:
        b.draht(a, an_a, w, "1")
    else:
        b.leitung(a, an_a, w, "1", ueber=[(ax, y_hin), (x_kontakt, y_hin)])
    ox = w.pos(aus)[0]
    zx = z.pos(an_z)[0]
    if abs(zx - ox) < 0.5:
        b.draht(w, aus, z, an_z)
    else:
        b.leitung(w, aus, z, an_z, ueber=[(ox, y_weg), (zx, y_weg)])
    return w


def wechsler_an_di(b, x, plus, master, bmk, kanal, nummern=("11", "12", "14"), schliesser=True, y_kontakt=None,
                   y_kanal=None, pfad=None):
    """Rückmeldung eines Relais an einen DI: Plusschiene -> Wechsler (Schließer oder Öffner) -> DI-Kanal bei x.
    Beim Schließer sitzt die Wurzel 30 px links vom Kanal, beim Öffner darüber."""
    xw = x - PIN_PITCH if schliesser else x
    a, an = b.abgriff(plus, xw)
    y_kontakt = y_kontakt or b.schienen[plus].y + 40
    ch = sps_kanal(b, pfad or S.DI_KANAL, x, y_kanal or y_kontakt + 120, kanal, "DI")
    return kontakt_zwischen(b, a, an, ch, "DI", master, bmk, nummern, schliesser, xw, y_kontakt)


def kanal_mit_oeffner(b, x, kanal, master, bmk, nummern=("21", "22", "24"), y_kanal=820, pfad=None):
    """F-DI mit Geberversorgung, Schleife VS -> Öffner eines Relais -> DI (z. B. E-Stop über Relaiskontakt).
    Der Kontakt sitzt rechts neben dem Kanal, die Leitungen laufen über und unter ihm herum."""
    ch = sps_kanal(b, pfad or S.F_DI_VS_KANAL, x, y_kanal, kanal, "DI")
    xk = x + 80
    return kontakt_zwischen(b, ch, "DI", ch, "VS", master, bmk, nummern, False, xk, y_kanal - 120,
                            y_hin=y_kanal - 150, y_weg=y_kanal - 60), ch


def _spule(b, x, y_a1, spule):
    """Relaisspule (bobine3, Master) mit A1 bei x/y_a1. Felder der Spule: 0 = „A2“, 1 = „A1“, 2 = Kennzeichen –
    setze() belegt die Benutzerfelder ab Index 1, daher A1 mitgeben; Kennzeichen links neben die Spule, rechts
    zeichnet QET den Kontaktspiegel."""
    # Benutzerfelder der Spule in Dateireihenfolge: zuerst das untere (A2), dann das obere (A1)
    # label_xy über setze: wird nach den Verknüpfungen gesetzt (link_elements setzt die Lage sonst zurück)
    sp = b.setze(S.SPULE, x, y_a1, anker="A1", label=spule["bmk"], infos=_infos(spule), felder=["A2", "A1"],
                 label_xy=SPULE_LABEL)
    return sp


F_DQ_KANAL = "company://Siemens/SPS/6ES7136-6DC00-0CA0/lesbar/6ES7136-6DC00-0CA0_DQ-PP-M_B1.elmt"   # DQ-PP, M unten


def relais_am_fdq(b, x, y_dq, spule, kanal):
    """Koppelrelais am sicheren Ausgang (F-DQ PP/M) in frei wählbarer Höhe: DQ-PP -> A1, A2 -> M desselben Kanals
    (Rückleitung rechts herum). Wie bausteine.schuetz_an_fdq, aber Kennzeichen über dem Kanalkasten."""
    fdq = sps_kanal(b, F_DQ_KANAL, x, y_dq, kanal, "DQ-PP")
    b.ops.append({"op": "set_element_text", "folio": b.folio, "element": fdq.ref, "index": 0, "property": "y",
                  "value": "-108"})
    sp = _spule(b, x, y_dq + 60, spule)
    b.draht(fdq, "DQ-PP", sp, "A1")
    y_r = sp.pos("A2")[1] + 20
    b.leitung(sp, "A2", fdq, "M", ueber=[(x, y_r), (x + 40, y_r), (x + 40, y_dq + 30), (x + 20, y_dq + 30)])
    return sp


def relais_am_dq(b, x, y_dq, spule, kanal, minus, y_spule=None):
    """Koppelrelais am Standard-DQ: DQ-Kanal (Kasten über dem Anschluss) -> A1, A2 -> Minusschiene (abgriff oben).
    spule: {bmk, artikel, hersteller, bezeichnung}. Rückgabe Spule (Master der Wechsler)."""
    dq = sps_kanal(b, S.DQ_KANAL, x, y_dq, kanal, "DQ")
    sp = _spule(b, x, y_spule or y_dq + 60, spule)
    b.draht(dq, "DQ", sp, "A1")
    a, an = b.abgriff(minus, x, "oben")
    b.draht(sp, "A2", a, an)
    return sp


def spule_zwischen(b, a, an_a, z, an_z, spule, y_spule, y_rueck):
    """Relaisspule zwischen zwei Geräteausgängen (z. B. PCSS Y00H -> A1, A2 -> Y00L): A1 senkrecht unter a,
    A2 über y_rueck waagerecht zurück zu z."""
    x = a.pos(an_a)[0]
    sp = _spule(b, x, y_spule, spule)
    b.draht(a, an_a, sp, "A1")
    b.leitung(sp, "A2", z, an_z, ueber=[(x, y_rueck), (z.pos(an_z)[0], y_rueck)])
    return sp


# --- Potentiale -----------------------------------------------------------------------------------------------
def pfeil_auf(b, name, x, y, text=True):
    """Gehender Verweispfeil nach oben (Leitung kommt von unten, z. B. Kabel zu einem Gerät auf anderem Blatt)."""
    a = S.symbol(S.GEHEND).anschluss(0)
    dy = _dreh(a.x, a.y, 270)[1]
    y = raster(y - dy) + dy
    t = b.setze(S.GEHEND, x, y, anker=0, drehung=270)
    b.plan.quellen.append((b.folio, name, t))
    if text:
        b.text(name, x - 6, y - 40, "funktion", drehung=270)
    return t


def geraet_versorgung(b, bmk, name, plus_pins, minus_pins, y_pins, plus, minus, kontakt=None, pe=None, y_pfeil=None):
    """Spannungsversorgung eines Geräts über einen Stecker: Potentialpfeil plus (von oben) -> optional Schließer
    eines Relais -> erster Plus-Pin, Brücken zu den weiteren; Pfeil minus -> erster Minus-Pin, Brücken.
    plus_pins/minus_pins: [(pin, x, signal)]; kontakt: {"master", "bmk", "nummern"} (Schließer, 30 px links vom Pin);
    pe: {"x", "text"} – Schutzleiter gestrichelt aufs Gehäuse. Rückgabe {pin: Teil}."""
    pins = stecker(b, bmk, name, [(p, x, "", s) for p, x, s in plus_pins + minus_pins], y_pins, "oben")
    y_pfeil = y_pfeil or y_pins - 380
    p0 = plus_pins[0]
    if kontakt:
        xk = p0[1] - PIN_PITCH
        pf = b.pfeil(plus, xk, y_pfeil, "von_oben")
        w = wechsler(b, xk, y_pfeil + 60, kontakt.get("master"), kontakt["bmk"], kontakt.get("nummern", ("11", "12", "14")))
        b.draht(pf, 0, w, "1")
        b.draht(w, "4", pins[str(p0[0])], 0)
    else:
        pf = b.pfeil(plus, p0[1], y_pfeil, "von_oben")
        b.draht(pf, 0, pins[str(p0[0])], 0)
    pm = b.pfeil(minus, minus_pins[0][1], y_pfeil, "von_oben")
    b.draht(pm, 0, pins[str(minus_pins[0][0])], 0)
    for gruppe in (plus_pins, minus_pins):              # Brücken im Stecker
        for (p1, _, _), (p2, _, _) in zip(gruppe, gruppe[1:]):
            b.draht(pins[str(p1)], 0, pins[str(p2)], 0)
    if pe:
        g = b.setze(PE_GEHAEUSE, pe["x"], y_pins + 10, anker="PE", drehung=180, label="")
        pp = b.pfeil("PE", pe["x"], y_pins - 200, "von_oben")
        b.draht(pp, 0, g, "PE", gestrichelt=True)
        if pe.get("text"):
            b.text(pe["text"], pe["x"] + 6, y_pins - 130, "daten")
    return pins


def schnittstelle_kabel(b, bmk, name, pins, y_pins, kabel_daten, ziel, y_kabel=None, zusatz=""):
    """Serielle Schnittstelle (RS-232 …) über ein Kabel zu einem Gerät auf einem anderen Blatt: Stecker mit den
    benutzten Pins [(pin, x, kanal, signal)], je Ader ein gehender Pfeil „<ziel> <signal>“ nach oben,
    Kabel (mit Aderfarben in Pin-Reihenfolge) quer darüber. zusatz: Text über den Pfeilen (z. B. „=SC COM2“)."""
    st = stecker(b, bmk, name, pins, y_pins, "oben")
    y_pf = y_pins - 260
    xs = []
    for p, x, k, s in pins:
        pf = pfeil_auf(b, f"{ziel} {s}", x, y_pf)
        b.draht(st[str(p)], 0, pf, 0)
        xs.append(x)
    kabel(b, kabel_daten, min(xs) - 12, max(xs) + 12, y_kabel or y_pins - 120)
    if zusatz:
        b.text(zusatz, min(xs) - 10, y_pf - 150, "daten")
    return st


# --- Feldgeräte über Kabel an Gerätestecker ------------------------------------------------------------------
def feldgeraet_an_stecker(b, geraet, x0, y_geraet, adern, strecke, bmk_ziel, stecker_name, y_stecker,
                          funktion=None):
    """Feldgerät (Kasten mit Anschlüssen unten, symbolbau.kasten im 30-px-Raster) über Kabel und optional einen
    Klemmenkasten an einen Stecker eines Geräts (Gerätepins von oben).
    adern: in Pin-Reihenfolge des Feldgeräts [(geraetepin, zielpin | None, kanal, signal)]; zielpin None = Ader
      ohne Ziel (Schirm/Reserve) -> freies Aderende unter dem ersten Kabel.
    strecke: von oben nach unten [{"art": "kabel", name, typ, adern, laenge}, {"art": "klemmen", "leiste",
      "nummern": [...] (für die Adern mit Ziel), "ort": "+KK1"}, …]."""
    g = b.setze(geraet["pfad"], x0, y_geraet, anker=adern[0][0], label=geraet["bmk"], infos=_infos(geraet))
    oben = {p: (g, p) for p, *_ in adern}                     # aktueller Startpunkt je Ader
    xs = {p: g.pos(p)[0] for p, *_ in adern}
    y, jobs, erstes_kabel = y_geraet + 60, [], True
    for st in strecke:
        if st["art"] == "kabel":
            mit = [p for p, *_ in adern] if erstes_kabel else [p for p, z, *_ in adern if z is not None]
            jobs.append((st, min(xs[p] for p in mit) - 12, max(xs[p] for p in mit) + 12, y))
            if erstes_kabel:                                   # Adern ohne Ziel enden unter dem ersten Kabel
                for p, z, *_ in adern:
                    if z is None:
                        e = b.setze(RESERVE, xs[p], y + 30, anker="X", label="")
                        b.draht(oben[p][0], oben[p][1], e, "X")
                        oben.pop(p)
                erstes_kabel = False
            y += 80
        elif st["art"] == "klemmen":
            mit = [p for p, z, *_ in adern if z is not None]
            kl = klemmen(b, st["leiste"], st["nummern"], [xs[p] for p in mit], y)
            for p, k in zip(mit, kl):
                b.draht(oben[p][0], oben[p][1], k, 0)
                oben[p] = (k, 1)
            if st.get("ort"):
                rahmen(b, min(xs[p] for p in mit) - 80, y - 30, max(xs[p] for p in mit) + 40, y + 40, st["ort"])
            y += 80
    pins = stecker(b, bmk_ziel, stecker_name, [(z, xs[p], k, s) for p, z, k, s in adern if z is not None],
                   y_stecker, "oben")
    for p, z, *_ in adern:
        if z is not None:
            b.draht(oben[p][0], oben[p][1], pins[str(z)], 0)
    for st, x1, x2, yk in jobs:                                # Kabel zuletzt (sucht seine Adern)
        kabel(b, st, x1, x2, yk)
    if funktion:
        from .bausteine import funktion_bei
        funktion_bei(b, (min(xs.values()) + max(xs.values())) / 2, funktion)
    return {"geraet": g, "pins": pins}


def pin_an_kanal(b, pin_teil, an, kanal, pfad, y_kanal, anker="DI"):
    """Geräteausgang (Gerätepin, Leitung nach unten) senkrecht an einen SPS-Eingangskanal."""
    x = pin_teil.pos(an)[0]
    ch = sps_kanal(b, pfad, x, y_kanal, kanal, anker)
    b.draht(pin_teil, an, ch, anker)
    return ch
