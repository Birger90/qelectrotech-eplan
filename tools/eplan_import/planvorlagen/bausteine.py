"""Funktionsbausteine: wiederkehrende Technik als Vorlage, gefüllt mit Projektdaten.

Jeder Baustein bekommt ein Blatt und eine Startspalte und belegt eine feste Zahl Spalten (BREITE). Er legt
seine Symbole in die Bänder des Rasters (blatt.BAND), hängt sich an die Potentialschienen des Blatts und
verknüpft Spule/Kontakte als Master/Slave. Alles Projektspezifische kommt als Parameter:

    geraet = {"bmk": "-QA318", "artikel": "3RT2016-1BB41", "hersteller": "Siemens", "daten": ["24VDC/4kW"]}
    kanal  = {"bmk": "-A2", "adresse": "E20.3", "symbol": "=DA+CA-FC311", "funktion": "Filtermotor Störung"}

Neue Bausteine: dieselbe Form (blatt, spalte, **daten) -> dict der gesetzten Teile, Breite in BREITE eintragen.
"""
import re
from .blatt import BAND, spalte_mitte, spalte_links, SPALTE_B as SPALTE
from . import symbole as S

BREITE = {"motorabgang_direkt": 4, "schalter_am_di": 1}
POL = 30                      # Polabstand der Leistungssymbole


def _infos(g):
    """Artikeldaten eines Geräts für set_info."""
    if not g or not g.get("artikel"):
        return {}
    return {"manufacturer": g.get("hersteller", ""), "manufacturer_reference": g.get("artikel", ""),
            "designation": g.get("artikel", ""), "description": g.get("bezeichnung", "")}


def _kennzeichen(b, g, x_ende, y):
    """Technische Daten links neben ein Symbol (das Kennzeichen zeigt das Bauteil selbst)."""
    for i, z in enumerate(g.get("daten", [])):
        b.rechts_text(z, x_ende, y + 9 * i, "daten")


def _links(spalte):
    """Leistungsteil: Mitte 120 px rechts vom Spaltenanfang (links Platz für Kennzeichen und Daten)."""
    return spalte_links(spalte) + 120            # spalte_links liegt auf dem Raster


def klemmen(b, leiste, nummern, xs, y, ort_text=True, pe=()):
    """Je Leiter eine Klemme der Leiste bei (x, y); Kennzeichen der Leiste links daneben.
    pe: Positionen (0-basiert) der Schutzleiterklemmen oder True für alle – sie werden als PE-Reihenklemme
    (Tragschienenkontakt, Klemmentyp „ground“) gesetzt; Klemmen mit der Nummer „PE“ ebenso."""
    # Leiste als Info „terminal_strip“: Klemmen- und Steckerpläne entstehen daraus (listen.py)
    def ist_pe(i, n):
        return pe is True or i in pe or str(n).upper() == "PE"
    teile = [b.setze(S.KLEMME_PE if ist_pe(i, n) else S.KLEMME, x, y, anker=0, label=str(n), label_xy=(4, -18.5),
                     infos={"terminal_strip": leiste})
             for i, (n, x) in enumerate(zip(nummern, xs))]
    if ort_text:
        b.rechts_text(leiste, min(xs) - 12, y + 4, "label")
    return teile


def kabel(b, k, x1, x2, y):
    """Kabel quer über die Leitungen bei y; k = {name, typ, adern:[farben], laenge, schirm}.
    schirm: True = Schirm gezeichnet (Ellipse um die Adern, „SH“); "PE" = zusätzlich rechts auf PE aufgelegt
    (EPLAN-Darstellung; QET-Kabel kennen keinen Schirm, daher als Grafik)."""
    if not k:
        return
    b.ops.append({"op": "add_cable", "folio": b.folio, "designation": k["name"], "type": k.get("typ", ""),
                  "core_colors": k.get("adern", []), "length": k.get("laenge", ""),
                  "x1": round(x1, 1), "y1": round(y, 1), "x2": round(x2, 1), "y2": round(y, 1)})
    # geschirmte Leitungstypen (LiYCY, …CY, …CP) bekommen ihren Schirm auch ohne Angabe gezeichnet
    schirm = k.get("schirm", bool(re.search(r"LiYCY|C[YP]|[0-9]\s*C[YP]| CY ", " " + k.get("typ", "") + " ")))
    if schirm:
        k = dict(k, schirm=schirm)
        i = b._id("schirm")
        b.ops += [{"op": "add_shape", "folio": b.folio, "shape": "ellipse", "x1": x1 + 6, "y1": y - 7,
                   "x2": x2 - 4, "y2": y + 7, "id": i},
                  {"op": "set_shape", "folio": b.folio, "index": f"${i}", "property": "line-style", "value": "dashed"}]
        b.text("SH", x2 + 2, y - 16, "daten")
        if k["schirm"] == "PE":
            j = b._id("schirm")
            b.ops.append({"op": "add_shape", "folio": b.folio, "shape": "line", "x1": x2 - 4, "y1": y,
                          "x2": x2 + 16, "y2": y, "id": j})
            j2 = b._id("schirm")
            b.ops.append({"op": "add_shape", "folio": b.folio, "shape": "line", "x1": x2 + 16, "y1": y,
                          "x2": x2 + 16, "y2": y + 19, "id": j2})
            b.setze(S.ERDE, x2 + 16, y + 30, anker="PE")


def kanal_nr(adresse, start):
    """Kanal im Modul aus der Adresse: E20.5 bei Modulanfang 20 -> 5 (8 Kanäle je Byte)."""
    byte, bit = adresse[1:].split(".")
    return (int(byte) - start) * 8 + int(bit)


def sps_kanal(b, pfad, x, y, kanal, anker):
    """Kanalmakro des Moduls. Felder: Adresse, Symbol, Funktion (kurz, passt in den Kasten),
    Kanal (DI5) und Modulklemme (6) – beide aus Adresse und Modulanfang (kanal['start'])."""
    art = "DI" if kanal.get("adresse", "E").startswith("E") else "DQ"
    pin = klemme = ""
    n = None
    if "start" in kanal and kanal.get("adresse"):
        # Kanalnummer aus der Adresse; Module mit versetzter Zählung (F-DQ: A120.1 = DQ0) geben "n" selbst an
        n = kanal.get("n", kanal_nr(kanal["adresse"], kanal["start"]))
        pin, klemme = f"{art}{n}", str(n + 1)
    # Kennzeichen über den Kasten, neben die Leitung (DI: Kasten unter dem Anschluss, DQ: darüber)
    # lesbare Makros (symbolbau.lesbar) tragen das Kennzeichen selbst über dem Kasten
    label_xy = None if "/lesbar/" in pfad else ((-34, -22.5) if (art == "DI" or "_oben_Kanal" in pfad) else (-34, -92))
    felder = [kanal.get("adresse", ""), kanal.get("symbol", ""),
              _kurz(kanal.get("kanaltext") or kanal.get("funktion", ""))]
    if pfad == S.DI_AUX_KANAL:                  # DI-AUX: Kanal, AUX, Klemme DI, Klemme AUX
        felder += [pin, "AUX", klemme, kanal.get("aux_klemme", "")]
    elif pfad == S.F_DI_VS_KANAL:               # F-DI 8x: DIn an Klemme n+1, VSn an Klemme n+9
        felder += [pin, f"VS{n}", klemme, str(n + 9)]
    elif pfad.endswith("_DQ-PP-M_B1.elmt"):     # F-DQ 8x PP: DQn an Klemme n+1, Mn an Klemme n+9
        felder += [f"DQ{n}", f"M{n}", klemme, str(n + 9)]
    else:
        felder += [pin, klemme]
    return b.setze(pfad, x, y, anker=anker, label=kanal.get("bmk", ""), label_xy=label_xy, felder=felder)


KANALTEXT_MAX = 26          # Zeichen, die in eine Zeile des Kanalkastens passen


def _kurz(text):
    """Kanaltext auf eine Zeile kürzen: erst Wörter von vorn weglassen, dann abschneiden."""
    w = text.split()
    while len(" ".join(w)) > KANALTEXT_MAX and len(w) > 1:
        w = w[1:]
    return " ".join(w)[:KANALTEXT_MAX]


# ---------------------------------------------------------------------------------------------------
def motorabgang_direkt(b, spalte, schutz, schuetz, motor, klemmleiste, klemmen_nr, kabel_motor=None,
                       dq_ein=None, di_stoerung=None, di_ein=None, schienen=("L1", "L2", "L3", "PE"),
                       plus="L+", minus="M", funktion=("Antrieb",)):
    """Drehstrommotor direkt: Motorschutzschalter -> Schütz -> Klemmen -> Motor (Spalte `spalte`),
    Schützspule am DQ (Spalte +1), Rückmeldungen Schutzschalter/Schütz an DI (Spalten +2, +3).

    schutz/schuetz/motor: Gerätedaten {bmk, artikel, hersteller, daten:[…]}; klemmen_nr: 3 (+PE) Nummern;
    dq_ein, di_stoerung, di_ein: SPS-Kanäle {bmk, adresse, symbol, funktion, kurz}."""
    cx = _links(spalte)
    xs = [cx - POL, cx, cx + POL]
    t = {}
    # Leistungsteil
    t["schutz"] = b.setze(S.MOTORSCHUTZ, cx, BAND["schutz"], anker="3", label=schutz["bmk"], infos=_infos(schutz))
    _kennzeichen(b, schutz, cx - POL - 44, BAND["schutz"] + 18)
    for i, (s, x) in enumerate(zip(schienen[:3], xs)):
        a, an = b.abgriff(s, x)
        b.draht(a, an, t["schutz"], ("1", "3", "5")[i])
    t["haupt"] = b.setze(S.HAUPTKONTAKT, cx, BAND["schalten"], anker=1, label=schuetz["bmk"], label_xy=(44, -9.5))
    for i in range(3):
        b.draht(t["schutz"], ("2", "4", "6")[i], t["haupt"], i)
    kl = klemmen(b, klemmleiste, klemmen_nr[:3], xs, BAND["klemme"])
    for i in range(3):
        b.draht(t["haupt"], 3 + i, kl[i], 0)
    t["motor"] = b.setze(S.MOTOR3, cx, BAND["feld"], anker="V1", label=motor["bmk"], infos=_infos(motor),
                         label_xy=(-62, 26))
    _kennzeichen(b, motor, cx - POL - 6, BAND["feld"] + 42)
    for i, n in enumerate(("U1", "V1", "W1")):
        b.draht(kl[i], 1, t["motor"], n)
    if len(klemmen_nr) > 3 and len(schienen) > 3:
        xpe = cx + 2 * POL
        kpe = klemmen(b, klemmleiste, [klemmen_nr[3]], [xpe], BAND["klemme"], ort_text=False, pe=True)[0]
        a, an = b.abgriff(schienen[3], xpe)
        b.draht(a, an, kpe, 0)
        b.draht(kpe, 1, t["motor"], "PE")
    kabel(b, kabel_motor, xs[0] - 12, cx + 2 * POL + 12, BAND["klemme"] + 50)

    # Ansteuerung: DQ -> Spule A1, A2 -> Minus
    xq = spalte_mitte(spalte + 1)
    t["spule"] = b.setze(S.SPULE, xq, BAND["steuer"] + 40, anker="A1", label=schuetz["bmk"], infos=_infos(schuetz))
    _kennzeichen(b, schuetz, xq - 24, BAND["steuer"] + 62)
    if dq_ein:
        t["dq"] = sps_kanal(b, S.DQ_KANAL, xq, BAND["steuer"] - 40, dq_ein, "DQ")
        b.draht(t["dq"], "DQ", t["spule"], "A1")
    a, an = b.abgriff(minus, xq, "oben")
    b.draht(t["spule"], "A2", a, an)
    b.verbinde(t["spule"], t["haupt"])

    # Rückmeldungen: Plus -> Hilfskontakt -> DI
    for k, (geraet_t, kanal, bmk) in enumerate(((t["schutz"], di_stoerung, schutz["bmk"]),
                                                (t["spule"], di_ein, schuetz["bmk"]))):
        if not kanal:
            continue
        xd = spalte_mitte(spalte + 2 + k)
        a, an = b.abgriff(plus, xd)
        hk = b.setze(S.SCHLIESSER, xd, BAND["schalten"], anker=0, label=bmk)
        b.draht(a, an, hk, 0)
        di = sps_kanal(b, S.DI_KANAL, xd, BAND["klemme"] - 40, kanal, "DI")
        b.draht(hk, 1, di, "DI")
        b.verbinde(geraet_t, hk)
    b.funktion(spalte, spalte + 1, list(funktion))
    if di_stoerung:
        b.funktion(spalte + 2, spalte + 2, [di_stoerung.get("kurz", "Störung")])
    if di_ein:
        b.funktion(spalte + 3, spalte + 3, [di_ein.get("kurz", "Ein")])
    return t


def schalter_am_di(b, spalte, geraet, klemmleiste, klemmen_nr, kanal, kabel_feld=None, plus="L+",
                   funktion=None, kanal2=None, funktion2=None):
    """Schalter/Melder im Feld an einen DI. Oben das Feldgerät, darunter Kabel und Klemmen
    (links Plus, rechts Signal), dann die Plusschiene, unten der DI-Kanal:
        Plus -> Klemme 1 -> Kontakt 13/14 -> Klemme 2 -> DI.
    kanal2: zweiter Schaltpunkt desselben Geräts (Spalte +1, dritte Klemme), gleiche Versorgung."""
    x = spalte_mitte(spalte)
    xp, xs = x - 20, x + 20
    y_oben, y_kontakt, y_klemme = BAND["schutz"] - 60, BAND["schutz"] - 40, BAND["schalten"] + 20
    t = {}
    t["kontakt"] = b.setze(S.SCHALTER_FELD, xs, y_kontakt, anker="13", label=geraet["bmk"],
                           infos=_infos(geraet), label_xy=(8, 22))
    k1, k2 = klemmen(b, klemmleiste, klemmen_nr[:2], [xp, xs], y_klemme)
    ecke1 = b.setze(S.ECKE, xp, y_oben, anker="N")
    ecke2 = b.setze(S.ECKE, xs, y_oben, anker="N")
    b.draht(k1, 0, ecke1, "S")
    b.draht(ecke1, "E", ecke2, "W")
    b.draht(ecke2, "S", t["kontakt"], "13")
    b.draht(t["kontakt"], "14", k2, 0)
    adern = [k2]
    if kanal2:
        x2 = spalte_mitte(spalte + 1) + 20
        t["kontakt2"] = b.setze(S.SCHALTER_FELD, x2, y_kontakt, anker="13", label=geraet["bmk"],
                                label_xy=(8, 22))
        k3 = klemmen(b, klemmleiste, [klemmen_nr[2]], [x2], y_klemme, ort_text=False)[0]
        b.leitung(ecke2, "E", t["kontakt2"], "13", ueber=[(x2, y_oben)])
        b.draht(t["kontakt2"], "14", k3, 0)
        adern.append(k3)
        di2 = sps_kanal(b, S.DI_KANAL, x2, BAND["klemme"] - 40, kanal2, "DI")
        b.draht(k3, 1, di2, "DI")
        b.funktion(spalte + 1, spalte + 1, list(funktion2 or [kanal2.get("funktion", "")]))
    # Kabel nach den Leitungen: es sucht seine Adern unter der Linie
    kabel(b, kabel_feld, xp - 12, (x2 if kanal2 else xs) + 12, BAND["schalten"] - 30)
    a, an = b.abgriff(plus, xp, "oben")
    b.draht(k1, 1, a, an)
    di = sps_kanal(b, S.DI_KANAL, xs, BAND["klemme"] - 40, kanal, "DI")
    b.draht(k2, 1, di, "DI")
    b.funktion(spalte, spalte, list(funktion or [kanal.get("funktion", "")]))
    return t


def _drei_eck_motor(b, cx, ausgaenge, klemmleiste, klemmen_nr, kabel1, kabel2, motor):
    """Motor im Dreieck über zwei Klemmensätze: T1->U1+W2, T2->V1+U2, T3->W1+V2 (Standard-Dreieck).
    ausgaenge: [(Teil, Anschluss)] der drei Phasen; klemmen_nr: 8 Nummern (1-3, PE, 5-7, PE)."""
    xs1 = [cx - POL, cx, cx + POL]
    xs2 = [cx + 3 * POL, cx + 4 * POL, cx + 5 * POL]
    yk = BAND["klemme"]
    k1 = klemmen(b, klemmleiste, klemmen_nr[0:3], xs1, yk)
    k2 = klemmen(b, klemmleiste, klemmen_nr[4:7], xs2, yk, ort_text=False)
    # zweiter Satz in der Reihenfolge W2, U2, V2: Abzweig je Phase über eine Ecke (T-Punkt)
    ziel2 = {0: 0, 1: 1, 2: 2}
    for i, (t, an) in enumerate(ausgaenge):
        y_ab = yk - 80 + 10 * i
        e = b.setze(S.ECKE, xs1[i], y_ab, anker="N")
        b.draht(t, an, e, "N")
        b.draht(e, "S", k1[i], 0)
        b.leitung(e, "E", k2[ziel2[i]], 0, ueber=[(xs2[ziel2[i]], y_ab)])
    m = b.setze(S.MOTOR6, cx, BAND["feld"], anker="V1", label=motor["bmk"], infos=_infos(motor), label_xy=(-62, 26))
    _kennzeichen(b, motor, cx - POL - 34, BAND["feld"] + 50)
    for i, n in enumerate(("U1", "V1", "W1")):
        b.draht(k1[i], 1, m, n)
    # zweiter Satz unten herum an W2, U2, V2 (Dreieck)
    y_unten = BAND["feld"] + 80
    for i, n in enumerate(("W2", "U2", "V2")):
        xm = m.pos(n)[0]
        yu = y_unten + 10 * i               # 5->W2 oben, 6->U2, 7->V2 darunter: nur eine Kreuzung
        b.leitung(k2[i], 1, m, n, ueber=[(xs2[i], yu), (xm, yu)])
    if len(klemmen_nr) >= 8:
        for xpe, nr in ((cx + 2 * POL, klemmen_nr[3]), (cx + 6 * POL, klemmen_nr[7])):
            kpe = klemmen(b, klemmleiste, [nr], [xpe], yk, ort_text=False, pe=True)[0]
            a, an = b.abgriff("PE", xpe)
            b.draht(a, an, kpe, 0)
            b.draht(kpe, 1, m, "PE")      # beide Kabel-PE an den Motor (jede Zuleitung führt ihren Schutzleiter)
    kabel(b, kabel1, xs1[0] - 12, cx + 2 * POL + 12, yk + 50)
    kabel(b, kabel2, xs2[0] - 12, cx + 6 * POL + 12, yk + 70)     # tiefer: Texte nicht übereinander
    return m


def pumpe_sanftstarter(b, spalte, schutz, starter, motor, reset, klemmleiste, klemmen_nr, kabel1=None,
                       kabel2=None, dq_ein=None, dq_reset=None, di_laeuft=None, di_bypass=None, di_ok=None,
                       versorgung="L+", minus="M", melde_plus="L+", funktion=("Antrieb",), laeuft_abgang=None):
    """Drehstrommotor am Sanftstarter (Siemens 3RW40) im Dreieck.
    Spalten: +0/+1 Leistung (Schutzschalter, Sanftstarter, zwei Klemmensätze, Motor), +2 DQ Ein,
    +3 DQ Reset mit Relais, +4..+6 Rückmeldungen läuft / bypassed / i.O.
    Steuerung: A1 über den Öffner des Reset-Relais (Reset = kurz spannungslos), Eingang 1 vom DQ,
    A2 an Minus. Rückmeldung i.O. = Störkontakt 95/96 in Reihe mit Hilfskontakt 13/14 des Schutzschalters."""
    cx = _links(spalte)
    xs = [cx - POL, cx, cx + POL]
    t = {}
    t["schutz"] = b.setze(S.MOTORSCHUTZ, cx, BAND["schutz"], anker="3", label=schutz["bmk"], infos=_infos(schutz))
    _kennzeichen(b, schutz, cx - POL - 44, BAND["schutz"] + 18)
    for i, (s, x) in enumerate(zip(("L1", "L2", "L3"), xs)):
        a, an = b.abgriff(s, x)
        b.draht(a, an, t["schutz"], ("1", "3", "5")[i])
    y_st = BAND["schalten"]
    t["starter"] = b.setze(S.SANFTSTARTER_3RW40, cx, y_st, anker="3/L2", label=starter["bmk"], infos=_infos(starter),
                           label_xy=(-90, 20))
    for i, n in enumerate(("1/L1", "3/L2", "5/L3")):
        b.draht(t["schutz"], ("2", "4", "6")[i], t["starter"], n)
    t["motor"] = _drei_eck_motor(b, cx, [(t["starter"], n) for n in ("2/T1", "4/T2", "6/T3")],
                                 klemmleiste, klemmen_nr, kabel1, kabel2, motor)

    # A1 über den Öffner des Reset-Relais, A2 an Minus
    xa1 = t["starter"].pos("A1")[0]
    a, an = b.abgriff(versorgung, xa1)
    t["reset_oe"] = b.setze(S.OEFFNER, xa1, BAND["schutz"] - 20, anker=0, label=reset["bmk"])
    b.draht(a, an, t["reset_oe"], 0)
    b.draht(t["reset_oe"], 1, t["starter"], "A1")
    a, an = b.abgriff(minus, xa1, "oben")
    b.draht(t["starter"], "A2", a, an)
    # Eingang 1 vom DQ (eigene Spalte, über Ecken)
    xin = t["starter"].pos("1")[0]
    if dq_ein:
        xq = spalte_mitte(spalte + 2)
        y_q = BAND["schalten"] - 30          # Kasten über dem Anschluss, unterhalb der Schienen
        t["dq"] = sps_kanal(b, S.DQ_KANAL, xq, y_q, dq_ein, "DQ")
        y_e = t["starter"].pos("1")[1] - 20
        b.leitung(t["dq"], "DQ", t["starter"], "1", ueber=[(xq, y_e), (xin, y_e)])
    # Reset-Relais am DQ
    xr = spalte_mitte(spalte + 3)
    if dq_reset:
        t["dq_reset"] = sps_kanal(b, S.DQ_KANAL, xr, BAND["schalten"] - 30, dq_reset, "DQ")
    t["reset"] = b.setze(S.SPULE, xr, BAND["schalten"] + 20, anker="A1", label=reset["bmk"], infos=_infos(reset))
    _kennzeichen(b, reset, xr - 24, BAND["schalten"] + 42)
    if dq_reset:
        b.draht(t["dq_reset"], "DQ", t["reset"], "A1")
    a, an = b.abgriff(minus, xr, "oben")
    b.draht(t["reset"], "A2", a, an)
    b.verbinde(t["reset"], t["reset_oe"])

    # Rückmeldungen
    y_k = BAND["steuer"] + 120               # unter den Schienen Minus und Melde-Plus (Raster!)
    y_di = BAND["klemme"] + 10
    for k, (kanal, kontakt, master) in enumerate(((di_laeuft, S.SCHLIESSER, "starter"),
                                                  (di_bypass, S.SCHLIESSER, "starter"),
                                                  (di_ok, S.OEFFNER, "starter"))):
        if not kanal:
            continue
        xd = spalte_mitte(spalte + 4 + k)
        a, an = b.abgriff(melde_plus, xd)
        hk = b.setze(kontakt, xd, y_k, anker=0, label=starter["bmk"])
        b.draht(a, an, hk, 0)
        b.verbinde(t[master], hk)
        letzter = hk
        if k == 2:                       # i.O.: Störkontakt 95/96 in Reihe mit Schutzschalter 13/14
            hs = b.setze(S.SCHLIESSER, xd, y_k + 60, anker=0, label=schutz["bmk"])
            b.draht(hk, 1, hs, 0)
            b.verbinde(t["schutz"], hs)
            letzter = hs
        di = sps_kanal(b, S.DI_KANAL, xd, y_di, kanal, "DI")
        b.draht(letzter, 1, di, "DI")
        if k == 0 and laeuft_abgang:     # Laufmeldung zusätzlich an ein anderes Gerät (z. B. HYD-Ein an LazerSafe)
            pf = b.pfeil(laeuft_abgang, xd - 50, y_k + 40, "ab")          # links neben dem Kontakt, über dem Kasten
            b.draht(hk, 1, pf, 0)
        b.funktion(spalte + 4 + k, spalte + 4 + k, [kanal.get("kurz", kanal.get("funktion", ""))])
    b.funktion(spalte, spalte + 1, list(funktion))
    if dq_ein:
        b.funktion(spalte + 2, spalte + 2, [dq_ein.get("kurz", "Ein")])
    if dq_reset:
        b.funktion(spalte + 3, spalte + 3, [dq_reset.get("kurz", "Reset")])
    return t


def rahmen(b, x1, y1, x2, y2, ort):
    """Strichpunkt-Rahmen eines anderen Einbauorts (+NSHV, +MASCHINE) mit Ortskennzeichen."""
    i = b._id("rahmen")
    b.ops += [{"op": "add_shape", "folio": b.folio, "shape": "rectangle", "x1": x1, "y1": y1, "x2": x2, "y2": y2, "id": i},
              {"op": "set_shape", "folio": b.folio, "index": f"${i}", "property": "line-style", "value": "dashdot"}]
    # Ortskennzeichen senkrecht am linken Rahmenrand (wie EPLAN): dort laufen keine Leitungen
    b.text(ort, x1 + 3, min(y2 - 4, y1 + 10 + 6.6 * len(ort)), "label", drehung=270)


def einspeisung_hauptschalter(b, spalte, hauptschalter, wandler, adapter, adapter_nr, zuleitung, zuleitung_nr,
                              kabel_zu=None, quelle="+NSHV", quelle_text=(), schienen=("L1", "L2", "L3", "PE")):
    """Einspeisung von unten: Quelle (+NSHV) -> Kabel -> Klemmen -> Hauptschalter -> Stromwandler je Phase ->
    Adapterklemmen -> Sammelschiene (Schienen oben). wandler: 3 Geräte {bmk, artikel, daten} (oder leer).
    Rückgabe enthält die Wandler-Teile für den Messbaustein (Sekundäranschlüsse 1 = k, 2 = l)."""
    cx = _links(spalte)
    xs = [cx - POL, cx, cx + POL]
    t = {"wandler": []}
    ka = klemmen(b, adapter[0], adapter_nr, xs, BAND["schiene"] + 120)
    for i, s in enumerate(schienen[:3]):
        a, an = b.abgriff(s, xs[i])
        b.draht(a, an, ka[i], 0)
    vorher = [(k, 1) for k in ka]
    for i, w in enumerate(wandler or []):
        y = BAND["schutz"] + 10 + 55 * i                      # versetzt, damit die Sekundärleitungen frei laufen
        # Beschriftung links an allen Phasenleitungen vorbei (sonst streichen L1/L2 die Texte durch)
        tw = b.setze(S.STROMWANDLER, xs[i], y, anker=0, label=w["bmk"], infos=_infos(w), label_xy=(-58 - POL * i, -6))
        _kennzeichen(b, w, xs[i] - 22 - POL * i, y + 14)
        b.draht(vorher[i][0], vorher[i][1], tw, 0)
        vorher[i] = (tw, 3)
        t["wandler"].append(tw)
    y_hs = BAND["klemme"] - 120
    t["hs"] = b.setze(S.LASTTRENNSCHALTER, cx, y_hs, anker="3", label=hauptschalter["bmk"], infos=_infos(hauptschalter))
    _kennzeichen(b, hauptschalter, cx - POL - 34, y_hs + 30)
    for i, n in enumerate(("1", "3", "5")):
        b.draht(vorher[i][0], vorher[i][1], t["hs"], n)
    kz = klemmen(b, zuleitung, zuleitung_nr[:3], xs, BAND["klemme"])
    for i, n in enumerate(("2", "4", "6")):
        b.draht(t["hs"], n, kz[i], 0)
    # Quelle als Klemmen des fremden Orts, darunter Rahmen
    yq = BAND["feld"] + 40
    kq = klemmen(b, "", ["L1", "L2", "L3"], xs, yq, ort_text=False)
    for i in range(3):
        b.draht(kz[i], 1, kq[i], 0)
    # N (5-Leiter-Einspeisung): eigene Klemme (Trennklemme), ungeschaltet auf die N-Schiene – N nie aus PE bilden
    mit_n = len(zuleitung_nr) > 4 and len(schienen) > 4
    xe = cx + 2 * POL
    if mit_n:
        kn = klemmen(b, zuleitung, [zuleitung_nr[4]], [xe], BAND["klemme"], ort_text=False)[0]
        a, an = b.abgriff(schienen[4], xe)
        b.draht(a, an, kn, 0)
        kqn = klemmen(b, "", ["N"], [xe], yq, ort_text=False)[0]
        b.draht(kn, 1, kqn, 0)
        xe += POL
    if len(zuleitung_nr) > 3:
        xpe = xe
        kpe = klemmen(b, zuleitung, [zuleitung_nr[3]], [xpe], BAND["klemme"], ort_text=False, pe=True)[0]
        a, an = b.abgriff(schienen[3], xpe)
        b.draht(a, an, kpe, 0)
        kqpe = klemmen(b, "", ["PE"], [xpe], yq, ort_text=False)[0]
        b.draht(kpe, 1, kqpe, 0)
    kabel(b, kabel_zu, xs[0] - 12, xe + 12, BAND["klemme"] + 60)
    rahmen(b, xs[0] - 50, yq - 30, xe + 60, yq + 70, quelle)
    for i, z in enumerate(quelle_text):
        b.text(z, xs[0] - 40, yq + 20 + 12 * i, "daten")
    b.funktion(spalte, spalte + 1, ["Hauptschalter"])
    return t


def energiemessung(b, spalte, messgeraet, absicherung, messklemmen, wandler, schienen=("L1", "L2", "L3", "N"),
                   versorgung=("L+", "M"), funktion=("Spannungsmessung",)):
    """Energiemessgerät (Siemens PAC3220): Spannungspfad über Absicherung und Trennklemmen an V1-V3, N an VN;
    Strompfade von den Wandlern (k/l) über Wandlerklemmen an IL1k..IL3l; Versorgung L/N von Schienen.
    messklemmen: (Leiste, [Nummern Strom 1-6], [Nummern Spannung 7-10])."""
    leiste, nr_strom, nr_spannung = messklemmen
    xf = _links(spalte)
    t = {}
    t["schutz"] = b.setze(S.MOTORSCHUTZ, xf, BAND["schutz"], anker="3", label=absicherung["bmk"],
                          infos=_infos(absicherung))
    _kennzeichen(b, absicherung, xf - POL - 44, BAND["schutz"] + 18)
    for i, s in enumerate(schienen[:3]):
        a, an = b.abgriff(s, xf - POL + POL * i)
        b.draht(a, an, t["schutz"], ("1", "3", "5")[i])
    # Messgerät: V2 unter der Mitte der Absicherung
    sym = S.symbol(S.PAC3220)
    y_pac = BAND["steuer"] + 20
    t["pac"] = b.setze(S.PAC3220, xf, y_pac, anker="V2", label=messgeraet["bmk"], infos=_infos(messgeraet),
                       label_xy=(-80, -20))
    xv = [t["pac"].pos(n)[0] for n in ("V1", "V2", "V3", "VN")]
    ys = BAND["schalten"] + 10
    ks = klemmen(b, leiste, nr_spannung, xv, ys)
    for i, n in enumerate(("2", "4", "6")):
        xa = t["schutz"].pos(n)[0]
        if xa == xv[i]:
            b.draht(t["schutz"], n, ks[i], 0)
        else:
            yj = BAND["schalten"] - 40 + 10 * i
            b.leitung(t["schutz"], n, ks[i], 0, ueber=[(xa, yj), (xv[i], yj)])
    for i, n in enumerate(("V1", "V2", "V3", "VN")):
        b.draht(ks[i], 1, t["pac"], n)
    a, an = b.abgriff(schienen[3], xv[3])
    b.draht(a, an, ks[3], 0)
    # Versorgung
    for n, s in zip(("L", "N"), versorgung):
        a, an = b.abgriff(s, t["pac"].pos(n)[0])
        b.draht(a, an, t["pac"], n)
    # Strompfade: Wandler k/l -> Wandlerklemmen (unter dem Gerät) -> ILnk/ILnl
    namen = ["IL1k", "IL1l", "IL2k", "IL2l", "IL3k", "IL3l"]
    xil = [t["pac"].pos(n)[0] for n in namen]
    yw = BAND["klemme"] + 20
    kw = klemmen(b, leiste, nr_strom, xil, yw)
    for i, n in enumerate(namen):
        b.draht(kw[i], 0, t["pac"], n)
    for i, w in enumerate(wandler):
        for j, (sek, k) in enumerate(((1, kw[2 * i]), (2, kw[2 * i + 1]))):
            n = 2 * i + j
            xw, yw_s = w.pos(sek)
            # eigene senkrechte Bahn je Ader rechts neben Phasen und PE, waagerecht unter den Wandlerklemmen
            x_lauf = wandler[-1].pos(0)[0] + 2 * POL + 20 + 10 * n
            y_lauf = BAND["klemme"] + 50 + 10 * n
            b.leitung(w, sek, k, 1, ueber=[(x_lauf, yw_s), (x_lauf, y_lauf), (k.pos(1)[0], y_lauf)])
    b.funktion(spalte, spalte + 3, list(funktion))
    return t


# --- 24 V: Netzteil, Selektivmodul, Verteilung ------------------------------------------------------
def netzteil_3ph(b, spalte, schutz, netzteil, schienen=("L1", "L2", "L3", "PE"), plus="L+", minus="M"):
    """Drehstrom-Netzteil (SITOP 6EP3437): Motorschutzschalter -> L1-L3, PE; +1/+2 auf Plus, -1/-2 auf Minus
    (Schienen unter dem Netzteil). Die Polabstände (30 -> 20 px) gleichen kurze Winkel aus."""
    cx = _links(spalte)
    t = {}
    t["schutz"] = b.setze(S.MOTORSCHUTZ, cx, BAND["schutz"], anker="3", label=schutz["bmk"], infos=_infos(schutz))
    _kennzeichen(b, schutz, cx - POL - 44, BAND["schutz"] + 18)
    for i, s in enumerate(schienen[:3]):
        a, an = b.abgriff(s, cx - POL + POL * i)
        b.draht(a, an, t["schutz"], ("1", "3", "5")[i])
    y_nt = BAND["schalten"] + 20
    t["nt"] = b.setze(S.NETZTEIL_3P_40A, cx, y_nt, anker="L2", label=netzteil["bmk"], infos=_infos(netzteil),
                      label_xy=(-90, -20))
    _kennzeichen(b, netzteil, cx - 50, y_nt + 30)
    for i, (a_fc, a_nt) in enumerate((("2", "L1"), ("4", "L2"), ("6", "L3"))):
        xa, xn = t["schutz"].pos(a_fc)[0], t["nt"].pos(a_nt)[0]
        if xa == xn:
            b.draht(t["schutz"], a_fc, t["nt"], a_nt)
        else:
            yj = y_nt - 40 + 10 * (i // 2)
            b.leitung(t["schutz"], a_fc, t["nt"], a_nt, ueber=[(xa, yj), (xn, yj)])
    xpe = t["nt"].pos("PE")[0]
    xbahn = cx + 2 * POL + 20                         # rechts am Schutzschalter vorbei
    a, an = b.abgriff(schienen[3], xbahn)
    b.leitung(a, an, t["nt"], "PE", ueber=[(xbahn, y_nt - 20), (xpe, y_nt - 20)])
    for n in ("+1", "+2"):
        a, an = b.abgriff(plus, t["nt"].pos(n)[0], "oben")
        b.draht(t["nt"], n, a, an)
    for n in ("-1", "-2"):
        a, an = b.abgriff(minus, t["nt"].pos(n)[0], "oben")
        b.draht(t["nt"], n, a, an)
    # 0 V geerdet (EN 60204-1: geerdeter Steuerstromkreis, Erdschluss löst aus statt unerkannt zu bleiben):
    # -3 über GNYE an PE – wie im Original (6 mm²)
    erde = b.setze(S.ERDE, t["nt"].pos("-3")[0], y_nt + 230, anker="PE")      # unter den Schienen L+/M
    b.draht(t["nt"], "-3", erde, "PE")
    b.text("0V geerdet", t["nt"].pos("-3")[0] + 8, y_nt + 196, "daten")
    b.funktion(spalte, spalte + 1, ["Netzteil", "24V DC"])
    return t


def selektivmodul(b, spalte, modul, ausgaenge, meldung=None, plus="L+", minus_name="1M4", ausgaenge_text=None,
                  funktion=None, minus_kommend=False):
    """SITOP SEL1200 (4 oder 8 Kanäle): Eingang 24V1/24V2 von der Plusschiene oben, Ausgänge 1..n als
    Potentialpfeile nach unten (Namen in `ausgaenge`, z. B. ["1L+", …]), 0V1 als Pfeil `minus_name`,
    Sammelmeldung 13/14 an einen DI-AUX-Kanal. ausgaenge_text: Funktionstext je Ausgang."""
    n = len(ausgaenge)
    pfad = S.SEL1200[8 if n > 4 else 4]
    x0 = spalte_links(spalte) + 180                   # 24V1 / Ausgang 1; Meldekontakte 160 px links davon
    y_in = BAND["schutz"]
    t = {"modul": b.setze(pfad, x0, y_in, anker="24V1", label=modul["bmk"], infos=_infos(modul))}
    _kennzeichen(b, modul, x0 - 190, y_in + 10)
    for n_in in ("24V1", "24V2"):
        a, an = b.abgriff(plus, t["modul"].pos(n_in)[0])
        b.draht(a, an, t["modul"], n_in)
    y_pf = t["modul"].pos("1")[1] + 50
    for i, name in enumerate(ausgaenge):
        x = t["modul"].pos(str(i + 1))[0]
        p = b.pfeil(name, x, y_pf, "ab")
        b.draht(t["modul"], str(i + 1), p, 0)
    x0v = t["modul"].pos("0V1")[0]
    # 0V1: beim ersten Modul gehend, beim weiteren kommt dasselbe Potential vom ersten Modul (minus_kommend)
    p = b.pfeil(minus_name, x0v, y_pf, "von_unten" if minus_kommend else "ab")
    b.draht(t["modul"], "0V1", p, 0)
    if meldung:
        x13 = t["modul"].pos("13")[0]
        di = sps_kanal(b, S.DI_AUX_KANAL, x13, BAND["klemme"], meldung, "DI")
        b.draht(t["modul"], "13", di, "DI")
        b.leitung(t["modul"], "14", di, "AUX")
    for i, zeilen in enumerate(ausgaenge_text or []):
        funktion_bei(b, t["modul"].pos(str(i + 1))[0], zeilen)
    b.funktion(spalte, spalte, list(funktion or [meldung.get("funktion", "") if meldung else ""]))
    return t


def funktion_bei(b, x, zeilen):
    """Funktionstext unten, mittig unter x (für Bausteine, deren Funktionen nicht spaltenweise liegen)."""
    g = 10
    for i, z in enumerate(zeilen if isinstance(zeilen, (list, tuple)) else [zeilen]):
        b.text(z, x - 0.31 * g * len(z), BAND["text"] + 14 * i, "funktion")


def _abgaenge(b, von, an_von, x, y_t, namen):
    """Von (von, an_von) senkrecht nach unten auf y_t, dort je Abgang ein Pfeil nach unten (20 px Raster),
    verbunden über Ecken (T-Punkte). Name mit „^“: Einspeisung von unten statt Abgang."""
    vorher, an_v = von, an_von
    for i, name in enumerate(namen):
        xi = x + 20 * i
        # "^name": hier kommt das Potential an (Einspeisung von unten), sonst Abgang nach unten
        richtung, name = ("von_unten", name[1:]) if name.startswith("^") else ("ab", name)
        if len(namen) == 1:
            pa = b.pfeil(name, xi, y_t + 40, richtung)
            b.draht(vorher, an_v, pa, 0)
            return
        e = b.setze(S.ECKE, xi, y_t, anker="N")
        b.draht(vorher, an_v, e, "N" if i == 0 else "W")
        pa = b.pfeil(name, xi, y_t + 40, richtung)
        b.draht(e, "S", pa, 0)
        vorher, an_v = e, "E"


def potential_verteilung(b, spalte, quelle, leiste, nummer, abgaenge, y=None, funktion=None):
    """Verteilung eines Potentials über eine Klemme: Pfeil `quelle` von oben -> Klemme -> Abzweige nach unten
    (je Abgang ein Potentialpfeil, Name z. B. "1L1+")."""
    x = spalte_links(spalte) + 50
    y = y or BAND["schutz"]
    p = b.pfeil(quelle, x, y - 40, "von_oben")
    k = klemmen(b, leiste, [nummer], [x], y)[0]
    b.draht(p, 0, k, 0)
    if abgaenge:
        _abgaenge(b, k, 1, x, y + 40, abgaenge)
    if funktion:
        b.funktion(spalte, spalte, list(funktion))
    return k


def minus_verteilung(b, spalte, schiene, leiste, nummer, abgaenge, y=None):
    """Minus-Verteilung: Abgriff an der durchgehenden Schiene -> Klemme -> Abzweige nach unten."""
    x = spalte_links(spalte) + 50               # Abstand zum Einspeisepfeil der Schiene
    s = b.schienen[schiene]
    y = y or s.y + 40
    a, an = b.abgriff(schiene, x)
    k = klemmen(b, leiste, [nummer], [x], y)[0]
    b.draht(a, an, k, 0)
    if abgaenge:
        _abgaenge(b, k, 1, x, y + 40, abgaenge)
    return k


# --- Sicherheitstechnik ---------------------------------------------------------------------------
def sicherheitskreis_2kanal(b, spalte, geraet, symbol, kanaele, feld=None, ort="+MASCHINE", funktion=None):
    """Zweikanaliger Öffnerkreis (Not-Halt, Schutztür …) an F-DI mit Geberversorgung: je Kanal
    VS -> Öffner -> DI. symbol: S.NOTHALT_2NC / S.SCHUTZTUER_2NC (11/12 Kanal 1, 21/22 Kanal 2).
    feld: {"leiste", "klemmen": [DI1, VS1, DI2, VS2], "kabel": {...}} – Gerät im Feld über Kabel und Klemmen;
    ohne feld sitzt das Gerät im Schrank und ist direkt verdrahtet."""
    x1 = spalte_mitte(spalte) - 20
    y_k = BAND["schutz"]
    t = {"geraet": b.setze(symbol, x1, y_k, anker="11", label=geraet["bmk"], infos=_infos(geraet))}
    y_box = BAND["klemme"] + 80                 # Kastenkopf (Kennzeichen) frei von den Klemmen
    y_top = y_k - 30
    for k, (oben, unten) in enumerate((("11", "12"), ("21", "22"))):
        xd = t["geraet"].pos(oben)[0]
        xv = xd + 20
        di = sps_kanal(b, S.F_DI_VS_KANAL, xd, y_box, kanaele[k], "DI")
        e1 = b.setze(S.ECKE, xd, y_top, anker="N")
        e2 = b.setze(S.ECKE, xv, y_top, anker="N")
        b.draht(t["geraet"], oben, e1, "S")
        b.draht(e1, "E", e2, "W")
        if feld:
            kd, kv = klemmen(b, feld["leiste"], feld["klemmen"][2 * k:2 * k + 2], [xd, xv], BAND["klemme"],
                             ort_text=(k == 0))
            b.draht(t["geraet"], unten, kd, 0)
            b.draht(kd, 1, di, "DI")
            b.draht(e2, "S", kv, 0)
            b.draht(kv, 1, di, "VS")
        else:
            b.draht(t["geraet"], unten, di, "DI")
            b.draht(e2, "S", di, "VS")
        b.funktion(spalte + k, spalte + k, list(funktion or [geraet.get("text", "")]) + [f"(Kanal {k + 1})"])
    if feld:
        x_ende = t["geraet"].pos("21")[0] + 20
        kabel(b, feld.get("kabel"), x1 - 12, x_ende + 12, BAND["klemme"] - 120)
        rahmen(b, x1 - 70, y_top - 30, x_ende + 30, y_k + 60, ort)
    return t


def lichtgitter(b, spalte, sender, empfaenger, kanaele, plus, minus, leiste, klemmen_nr, kabel_s=None, kabel_e=None,
                ort="+MASCHINE", funktion=None):
    """Sicherheits-Lichtgitter (Sender + Empfänger) im Feld: +24 V/0 V über Klemmen von Potentialpfeilen,
    OSSD1/OSSD2 des Empfängers an zwei F-DI. sender/empfaenger: {bmk, artikel, hersteller, pfad}; Anschlüsse
    über die Namen in den Symbolen (+24VDC, 0V, OSSD1, OSSD2, FE). klemmen_nr: [S+, S0, E+, E0, OSSD1, OSSD2]."""
    x0 = spalte_links(spalte) + 40
    y_g = BAND["schutz"] - 20
    ts = b.setze(sender["pfad"], x0, y_g, label=sender["bmk"], infos=_infos(sender))
    te = b.setze(empfaenger["pfad"], x0 + 2 * SPALTE, y_g, label=empfaenger["bmk"], infos=_infos(empfaenger))
    pins = {id(ts): sender["pins"], id(te): empfaenger["pins"]}     # Bedeutung -> Anschlussname

    def an(t, bedeutung):
        return pins[id(t)][bedeutung]
    y_kl = BAND["klemme"] - 60
    paare = [(ts, "+24VDC"), (ts, "0V"), (te, "+24VDC"), (te, "0V"), (te, "OSSD1"), (te, "OSSD2")]
    kl = []
    for i, ((t, bed), nr) in enumerate(zip(paare, klemmen_nr)):
        x = t.pos(an(t, bed))[0]
        k = klemmen(b, leiste, [nr], [x], y_kl, ort_text=(i == 0))[0]
        b.draht(t, an(t, bed), k, 0)
        kl.append((k, bed))
    for t in (ts, te):                       # Funktionserde FE über die GNYE-Ader auf eine PE-Reihenklemme
        if "FE" in pins[id(t)]:
            kfe = klemmen(b, leiste, ["PE"], [t.pos(an(t, "FE"))[0]], y_kl, ort_text=False, pe=True)[0]
            b.draht(t, an(t, "FE"), kfe, 0)
    y_pf = BAND["klemme"] + 10
    for k, bed in kl:
        if bed in ("+24VDC", "0V"):
            name = plus if bed == "+24VDC" else minus
            p = b.pfeil(name, k.pos(1)[0], y_pf, "von_unten")
            b.draht(k, 1, p, 0)
    # OSSD-Kanäle je in eine eigene Spalte rechts neben den Empfänger (Kästen 150 px breit)
    for j, (k, bed) in enumerate([x for x in kl if x[1].startswith("OSSD")]):
        xbox = spalte_mitte(spalte + 3 + j) - 20
        di = sps_kanal(b, S.F_DI_KANAL, xbox, BAND["klemme"] + 60, kanaele[j], "DI")
        y_r = BAND["klemme"] + 20 + 10 * j
        b.leitung(k, 1, di, "DI", ueber=[(k.pos(1)[0], y_r), (xbox, y_r)])
        funktion_bei(b, xbox + 40, [kanaele[j].get("kurz") or kanaele[j].get("funktion", "")])
    xs_s = [ts.pos(p)[0] for p in pins[id(ts)].values()]
    kabel(b, kabel_s, min(xs_s) - 12, max(xs_s) + 12, y_kl - 60)
    kabel(b, kabel_e, te.pos(an(te, "+24VDC"))[0] - 12, te.pos(an(te, "OSSD1"))[0] + 12, y_kl - 60)
    rahmen(b, x0 - 30, y_g - 140, te.x + 160, y_g + 30, ort)
    b.funktion(spalte, spalte + 2, list(funktion or ["Sicherheitslichtgitter"]))
    return {"sender": ts, "empfaenger": te}


# --- 230 V: Steuertrafo, Abgänge, Meldekontakte ----------------------------------------------------
def steuertrafo(b, spalte, schutz, trafo, erde, schienen=("L1", "L2"), sekundaer=("TL230", "TN230"),
                funktion=("Steuertrafo", "Steuerspannung 230V AC")):
    """Einphasen-Steuertrafo: Schutzschalter (drei Strombahnen in Reihe) an zwei Phasen -> Primär (trafo["primaer"] = 2 Anschlussnamen) ->
    Sekundär (trafo["sekundaer"] = [N-Seite, L-Seite]) über Klemmen `erde` = (Leiste, [N, L, PE]) an die
    Sekundärschienen; die N-Seite ist über die PE-Klemme geerdet."""
    cx = _links(spalte)
    t = {"schutz": b.setze(S.MOTORSCHUTZ, cx, BAND["schutz"], anker="3", label=schutz["bmk"], infos=_infos(schutz))}
    _kennzeichen(b, schutz, cx - POL - 44, BAND["schutz"] + 18)
    # Einphasige Last am 3RV2 (Phasenausfallerkennung): alle drei Strombahnen in Reihe (Siemens-Handbuch 3RV2):
    # erste Phase -> 1, Brücke 2-3, 4 -> Primär 1; zweite Phase -> 5, 6 -> Primär 2
    a, an = b.abgriff(schienen[0], cx - POL)
    b.draht(a, an, t["schutz"], "1")
    a, an = b.abgriff(schienen[1], cx + POL)
    b.draht(a, an, t["schutz"], "5")
    y2, y3 = t["schutz"].pos("2")[1], t["schutz"].pos("3")[1]
    b.leitung(t["schutz"], "2", t["schutz"], "3", ueber=[(cx - POL, y2 + 10), (cx - POL // 2, y2 + 10),
                                                        (cx - POL // 2, y3 - 10), (cx, y3 - 10)])
    p1, p2 = trafo["primaer"]
    # Kasten beginnt 30 px links von Primär 1: BMK und Daten links daneben, nicht auf der Zuleitung
    t["trafo"] = b.setze(trafo["pfad"], cx, BAND["schalten"], anker=p1, label=trafo["bmk"], infos=_infos(trafo),
                         label_xy=(-8 - 6.2 * len(trafo["bmk"]), 4))
    _kennzeichen(b, trafo, cx - 36, BAND["schalten"] + 50)
    b.draht(t["schutz"], "4", t["trafo"], p1)
    b.draht(t["schutz"], "6", t["trafo"], p2)
    leiste, nr = erde
    sn, sl = trafo["sekundaer"]
    xn, xl = t["trafo"].pos(sn)[0], t["trafo"].pos(sl)[0]
    kn, kl = klemmen(b, leiste, nr[:2], [xn, xl], BAND["klemme"] - 60)
    b.draht(t["trafo"], sn, kn, 0)
    b.draht(t["trafo"], sl, kl, 0)
    kpe = klemmen(b, leiste, [nr[2]], [xn], BAND["klemme"] + 40, ort_text=False, pe=True)[0]
    b.text("PE", xn + 8, BAND["klemme"] + 70, "daten")
    # Sekundärschienen rechts oben: unten herum, dann hinauf (Abgang des Abzweigs nach unten)
    x_ziel = spalte_links(spalte + 2) + 20
    for i, (k, s) in enumerate(((kl, sekundaer[0]), (kn, sekundaer[1]))):
        xs = x_ziel + POL * i
        a, an = b.abgriff(s, xs)
        y_u = BAND["klemme"] - 20 + 10 * i
        if k is kn:                       # N-Seite: Abzweig zur PE-Klemme (Erdung)
            e = b.setze(S.ECKE, k.pos(1)[0], y_u, anker="N")
            b.draht(k, 1, e, "N")
            b.draht(e, "S", kpe, 0)
            b.leitung(e, "E", a, an, ueber=[(xs, y_u)])
        else:
            b.leitung(k, 1, a, an, ueber=[(k.pos(1)[0], y_u), (xs, y_u)])
    b.funktion(spalte, spalte + 1, list(funktion))
    return t


VERBRAUCHER = {"steckdose": (S.STECKDOSE, "L", "N", "PE"), "geraet": (None, "L", "N", "PE"),
               "leuchte": (S.LEUCHTE, "A1", "A2", None), "luefter": (S.LUEFTER, "L", "N", "PE")}


def abgang_230v(b, spalte, schutz, verbraucher, klemmleiste, schienen=("TL230", "TN230"), pe="PE", schalter=None):
    """230-V-Abgang: Schutzorgan (schutz["art"] "fi_ls" = FI/LS 1P+N, "ls" = LS 1P, N direkt) -> optional
    Schalter/Thermostat in L -> ein oder mehrere Verbraucher, je mit Klemmen L/N/PE, Kabel und Funktionstext.
    verbraucher: [{bmk, art: steckdose|geraet|leuchte|luefter, pfad (für geraet), klemmen: [L, N, PE],
    kabel, funktion: [...], daten, ort}]. Rückgabe {"schutz": Teil} für Meldekontakte auf anderen Blättern."""
    xl = spalte_links(spalte) + 60
    xn = xl + POL
    t = {}
    art = schutz.get("art", "fi_ls")
    tl, an_l = b.abgriff(schienen[0], xl)
    tn, an_n = b.abgriff(schienen[1], xn)
    if art == "fi_ls":
        # BMK links neben den Schalter: rechts laufen N und die PE-Abzweigleitung (Kontaktspiegel hängt darunter)
        t["schutz"] = b.setze(S.FI_LS, xl, BAND["schutz"], anker="1", label=schutz["bmk"], infos=_infos(schutz),
                              label_xy=(-15 - 8 - 6.2 * len(schutz["bmk"]), -40))
        b.draht(tl, an_l, t["schutz"], "1")
        b.draht(tn, an_n, t["schutz"], "N")
        l_von, n_von = (t["schutz"], "2"), (t["schutz"], "N.")
    else:
        t["schutz"] = b.setze(S.LS_1P, xl, BAND["schutz"], anker="1", label=schutz["bmk"], infos=_infos(schutz))
        b.draht(tl, an_l, t["schutz"], "1")
        l_von, n_von = (t["schutz"], "2"), (tn, an_n)
    _kennzeichen(b, schutz, xl - 30, BAND["schutz"] + 30)
    if schalter:
        pfad = S.THERMOSTAT if schalter.get("art") == "thermostat" else S.SCHALTER_1NO
        t["schalter"] = b.setze(pfad, xl, BAND["schalten"], anker="13", label=schalter["bmk"], infos=_infos(schalter))
        _kennzeichen(b, schalter, xl - 44, BAND["schalten"] + 30)
        b.draht(l_von[0], l_von[1], t["schalter"], "13")
        l_von = (t["schalter"], "14")
    y_v = BAND["steuer"] + 40
    vorher_l, vorher_n = l_von, n_von
    for i, v in enumerate(verbraucher):
        x = spalte_links(spalte + i) + 60
        if len(verbraucher) > 1:
            el = b.setze(S.ECKE, x, y_v, anker="N")
            en = b.setze(S.ECKE, x + POL, y_v + 10, anker="N")
            b.draht(vorher_l[0], vorher_l[1], el, "N" if i == 0 else "W")
            b.draht(vorher_n[0], vorher_n[1], en, "N" if i == 0 else "W")
            vorher_l, vorher_n = (el, "E"), (en, "E")
            ziel_l, ziel_n = (el, "S"), (en, "S")
        else:
            ziel_l, ziel_n = vorher_l, vorher_n
        kl_l, kl_n, kl_pe = klemmen(b, klemmleiste, v["klemmen"], [x, x + POL, x + 2 * POL], BAND["klemme"],
                                    ort_text=(i == 0), pe=(2,))
        b.draht(ziel_l[0], ziel_l[1], kl_l, 0)
        b.draht(ziel_n[0], ziel_n[1], kl_n, 0)
        a, an = b.abgriff(pe, x + 2 * POL)
        b.draht(a, an, kl_pe, 0)
        pfad, pl, pn, ppe = VERBRAUCHER[v["art"]]
        pfad = v.get("pfad") or pfad
        y_f = BAND["feld"] + 20
        if v["art"] == "luefter":
            g = b.setze(pfad, x + POL, y_f, anker="N", label=v["bmk"], infos=_infos(v))
            b.leitung(kl_l, 1, g, "L", ueber=[(x, y_f - 30), (x - POL, y_f - 30)])
            b.draht(kl_n, 1, g, "N")
            b.leitung(kl_pe, 1, g, "PE", ueber=[(x + 2 * POL, y_f - 20), (g.pos("PE")[0], y_f - 20)])
        elif v["art"] == "leuchte":
            g = b.setze(pfad, x, y_f, anker="A1", label=v["bmk"], infos=_infos(v), label_xy=(-64, -8))
            b.draht(kl_l, 1, g, "A1")
            b.leitung(kl_n, 1, g, "A2", ueber=[(x + POL, y_f + 60), (x, y_f + 60)])
            # Leuchte Schutzklasse I: PE-Ader an den Gehäuse-PE (im Original W10x 3G1,5 mit GNYE)
            erde = b.setze(S.ERDE, x + 2 * POL, y_f + 30, anker="PE")
            b.draht(kl_pe, 1, erde, "PE")
        else:
            g = b.setze(pfad, x, y_f, anker=pl, label=v["bmk"], infos=_infos(v))
            b.draht(kl_l, 1, g, pl)
            b.draht(kl_n, 1, g, pn)
            b.draht(kl_pe, 1, g, ppe)
        _kennzeichen(b, v, x - 34, y_f + 40)
        kabel(b, v.get("kabel"), x - 12, x + 2 * POL + 12, BAND["klemme"] + 50)
        if v.get("ort"):
            rahmen(b, x - 70, BAND["klemme"] + 70, x + 2 * POL + 60, BAND["feld"] + 130, v["ort"])
        b.funktion(spalte + i, spalte + i, list(v.get("funktion", [])))
    return t


def meldekontakt_an_di(b, spalte, master, bmk, kanal, plus="L+", funktion=None, anschluesse=("13", "14")):
    """Hilfskontakt (13/14) eines Geräts von einem anderen Blatt an einen DI, verknüpft als Nebenfunktion."""
    x = spalte_mitte(spalte)
    a, an = b.abgriff(plus, x)
    hk = b.setze(S.SCHLIESSER, x, BAND["schalten"], anker=0, label=bmk)
    # Anschlussbezeichnungen wie am Gerät (das QET-Kontaktsymbol trägt keine)
    b.text(anschluesse[0], x - 14, BAND["schalten"] - 24, "daten")       # links: rechts steht das BMK
    b.text(anschluesse[1], x - 14, BAND["schalten"] + 16, "daten")
    b.draht(a, an, hk, 0)
    b.verbinde(master, hk)
    di = sps_kanal(b, S.DI_KANAL, x, BAND["klemme"], kanal, "DI")
    b.draht(hk, 1, di, "DI")
    b.funktion(spalte, spalte, list(funktion or [kanal.get("funktion", "")]))
    return hk


# --- Bediengeräte und Feldkontakte über Kabel/Stecker an Kanäle --------------------------------------
STECKERSTIFT = "company://Allgemein/Steckverbinder/Steckerstift_Plan/Steckerstift_Plan.elmt"   # o oben, u unten
DQ_OBEN = "company://Siemens/Steuerungen/6ES7132-6BH00-0AA0/lesbar/6ES7132-6BH00-0AA0_DQ_oben_Kanal.elmt"


def kontakte_an_kanaele(b, spalte, geraet, paare, strecke=(), ort=None, rahmen_bis=None):
    """Allgemeiner Baustein für Bediengeräte, Fußschalter, Taster, Leuchtmelder und Steckerbrücken.

    geraet: {"pfad", "bmk", artikel …} – Gerätesymbol mit Anschlüssen unten (symbolbau.kasten) oder None.
    paare: je Kanal eine Spalte (ab `spalte`):
        {"art": "fdi", "di": "13", "vs": "14", "kanal": {...}}         F-DI mit Geberversorgung VS
        {"art": "di",  "di": "14", "vs": "13", "kanal": {...}, "plus": "4L6+"}   DI, Versorgung vom Potential
        {"art": "dq",  "di": "X1", "vs": "X2", "kanal": {...}, "minus": "4M6"}   Leuchte am DQ
        {"art": "bruecke", "kanal": {...}}                              Brücke im Stecker (Steckerüberwachung)
        {"art": "reserve"}                                              nur Übergänge
      dazu "funktion": [...] (Funktionstext der Spalte).
      "vs_gemeinsam": True – die Versorgung dieses Kontakts ist im Gerät auf die des ersten Paars gebrückt
      (z. B. Wahlschalter: Wurzeln 2-6-4 gebrückt, eine Ader von VS0); der VS-Kanal bleibt frei.
    strecke: Übergänge von oben nach unten, je Paar zwei Leiter (links Signal, rechts Versorgung):
        {"art": "kabel", "name", "typ", "adern"}, {"art": "stecker"|"klemmen", "leiste", "pins": [(a, b) je Paar]}.
    Leitungen vom Gerät fächern treppenförmig in die Spalten auf (keine Kreuzungen)."""
    n = len(paare)
    xs = [spalte_mitte(spalte + i) - 20 for i in range(n)]
    y_dev = BAND["schutz"] - 30
    t = {}
    oben = {}                                           # je Paar/Ader: (Teil, Anschluss) als Startpunkt nach unten
    if geraet:
        t["geraet"] = b.setze(geraet["pfad"], xs[0] - 20, y_dev, label=geraet["bmk"], infos=_infos(geraet))
        y0 = y_dev + 40 + 20 * n
        vorige_vs = None
        for i, p in enumerate(paare):
            if p.get("art") in ("bruecke", "reserve") or "di" not in p:
                continue
            if p.get("vs_gemeinsam") and vorige_vs:
                # Brücke im Gerät: VS-Anschluss an den des vorigen Paars, dicht unter den Geräteanschlüssen
                yb = t["geraet"].pos(p["vs"])[1] + 10
                b.leitung(t["geraet"], p["vs"], t["geraet"], vorige_vs, ueber=[(t["geraet"].pos(p["vs"])[0], yb),
                                                                              (t["geraet"].pos(vorige_vs)[0], yb)])
            vorige_vs = p["vs"]
            for k, (pin, x) in enumerate(((p["di"], xs[i]), (p["vs"], xs[i] + 20))):
                if k == 1 and (p.get("vs_gemeinsam") or (p.get("art") == "di" and not p.get("plus"))):
                    continue
                xp = t["geraet"].pos(pin)[0]
                if xp == x:
                    oben[(i, k)] = (t["geraet"], pin)
                else:
                    y = y0 - 20 * i - 10 * k
                    e = b.setze(S.ECKE, x, y, anker="N")
                    b.leitung(t["geraet"], pin, e, "W" if xp < x else "E", ueber=[(xp, y)])
                    oben[(i, k)] = (e, "S")
        y_s = y0 + 40
    else:
        y_s = y_dev + 60
    letzte, kabel_jobs = {}, []
    for st in strecke:
        if st["art"] == "kabel":
            kabel_jobs.append((st, y_s))
            y_s += 50
            continue
        pfad = STECKERSTIFT if st["art"] == "stecker" else S.KLEMME
        oben_an, unten_an = ("o", "u") if st["art"] == "stecker" else (0, 1)
        for i, pins in enumerate(st["pins"]):
            if not pins:
                continue
            els = []
            for k, pin in enumerate(pins):
                if pin is None:
                    els.append(None)
                    continue
                el = b.setze(pfad, xs[i] + 20 * k, y_s, anker=oben_an, label=str(pin),
                             infos={"terminal_strip": st["leiste"]})
                vorher = letzte.get((i, k)) or oben.pop((i, k), None)
                if vorher:
                    b.draht(vorher[0], vorher[1], el, oben_an)
                els.append(el if not vorher else None)
                letzte[(i, k)] = (el, unten_an)
            # Brücke im Stecker: die beiden ersten Pins eines Brückenpaars oben verbinden
            if paare[i].get("art") == "bruecke" and len(els) == 2 and els[0] and els[1]:
                e1 = b.setze(S.ECKE, xs[i], y_s - 30, anker="N")
                e2 = b.setze(S.ECKE, xs[i] + 20, y_s - 30, anker="N")
                b.draht(e1, "E", e2, "W")
                b.draht(e1, "S", els[0], oben_an)
                b.draht(e2, "S", els[1], oben_an)
        # am Symbol nur das Kennzeichen (-XS221); der Ort (+S1/+CA) steht im Rahmen bzw. in der Info terminal_strip
        b.rechts_text(re.sub(r"^\+[^-]+", "", st["leiste"]), xs[0] - 10, y_s + 6, "label")
        y_s += 60
    # Kanäle bzw. Potentiale unten
    y_ch = BAND["klemme"] + 80
    for i, p in enumerate(paare):
        art = p.get("art", "fdi")
        quelle = {k: letzte.get((i, k)) or oben.get((i, k)) for k in (0, 1)}
        if art in ("fdi", "bruecke"):
            ch = sps_kanal(b, S.F_DI_VS_KANAL, xs[i], y_ch, p["kanal"], "DI")
            if quelle[0]:
                b.draht(quelle[0][0], quelle[0][1], ch, "DI")
            if quelle[1]:
                b.draht(quelle[1][0], quelle[1][1], ch, "VS")
        elif art == "di":
            ch = sps_kanal(b, S.DI_KANAL, xs[i], y_ch, p["kanal"], "DI")
            b.draht(quelle[0][0], quelle[0][1], ch, "DI")
            if p.get("plus") and quelle[1]:      # ohne "plus": Versorgung kommt über eine Brücke im Gerät/Pult
                pf = b.pfeil(p["plus"], xs[i] + 20, y_s + 20, "von_unten")
                b.draht(quelle[1][0], quelle[1][1], pf, 0)
        elif art == "dq":
            ch = sps_kanal(b, DQ_OBEN, xs[i], y_ch, p["kanal"], "DQ")
            b.draht(quelle[0][0], quelle[0][1], ch, "DQ")
            pf = b.pfeil(p["minus"], xs[i] + 20, y_s + 20, "ab")
            b.draht(quelle[1][0], quelle[1][1], pf, 0)
        if p.get("funktion"):
            b.funktion(spalte + i, spalte + i, list(p["funktion"]))
    for st, y in kabel_jobs:                          # Kabel zuletzt: es sucht die Adern unter seiner Linie
        kabel(b, st, xs[0] - 12, xs[-1] + 32, y)
    if ort:
        rahmen(b, xs[0] - 60, y_dev - 120, (rahmen_bis or xs[-1] + 60), y_dev + 50, ort)
    return t


# --- Ventile: Schütz am F-DQ, Ventil mit Stellungsrückmeldung, Proportionalventil -----------------
F_DQ_KANAL = "company://Siemens/SPS/6ES7136-6DC00-0CA0/lesbar/6ES7136-6DC00-0CA0_DQ-PP-M_B1.elmt"   # DQ-PP, M unten


def schuetz_an_fdq(b, spalte, schuetz, kanal, funktion=None):
    """Koppelschütz am sicheren Ausgang (F-DQ PP/M): DQ-PP -> A1, A2 -> M desselben Kanals.
    Rückgabe der Spule (Master für die Schließer in `ventil_mit_rueckmeldung`)."""
    x = spalte_mitte(spalte) - 20
    y_q = BAND["schutz"] - 30
    fdq = sps_kanal(b, F_DQ_KANAL, x, y_q, kanal, "DQ-PP")
    # BMK rechts neben die Rückleitung zu M (x+40): QET hängt den Kontaktspiegel unter das BMK
    sp = b.setze(S.SPULE, x, y_q + 60, anker="A1", label=schuetz["bmk"], infos=_infos(schuetz), label_xy=(48, -9))
    _kennzeichen(b, schuetz, x - 24, y_q + 82)
    b.draht(fdq, "DQ-PP", sp, "A1")
    y_r = sp.pos("A2")[1] + 20
    b.leitung(sp, "A2", fdq, "M", ueber=[(x, y_r), (x + 40, y_r), (x + 40, y_q + 20), (x + 20, y_q + 20)])
    b.funktion(spalte, spalte, list(funktion or ["Ansteuerung", kanal.get("funktion", "")]))
    return sp


def ventil_mit_rueckmeldung(b, spalte, spule, schuetz_bmk, kontakt, ventil, sensor, plus, minus, xg6, kabel_v, xg5,
                            kabel_s, kanal, sensor_plus="4L3+", sensor_minus="4M3", funktion_v=None,
                            funktion_s=None, ort="+MASCHINE"):
    """Magnetventil über einen Schließer des Koppelschützes (kontakt = (an, ab) Anschlussnummern) an Plus/Minus,
    Klemmen xg6 = (Leiste, [+, -, PE]) und Kabel; daneben der Näherungsschalter der Ventilstellung
    (Anschlüsse unten: 1 + BN, 3 - BU, 2 Signal WH, 4 Reserve BK) über INI-Kabel und Initiatorenklemmen
    xg5 = (Leiste, [Signal, Reserve]) an einen DI, Versorgung von sensor_plus/sensor_minus (Schienen darunter)."""
    xv = spalte_links(spalte) + 60
    t = {}
    a, an = b.abgriff(plus, xv)
    # BMK links: rechts läuft im Polabstand die Minusleitung
    t["kontakt"] = b.setze(S.SCHLIESSER, xv, BAND["schutz"] + 30, anker=0, label=schuetz_bmk,
                           label_xy=(-14 - 6.2 * len(schuetz_bmk), -2))
    b.text(kontakt[0], xv + 6, BAND["schutz"] + 6, "daten")
    b.text(kontakt[1], xv + 6, BAND["schutz"] + 46, "daten")
    b.draht(a, an, t["kontakt"], 0)
    b.verbinde(spule, t["kontakt"])
    k_p, k_m, k_pe = klemmen(b, xg6[0], xg6[1], [xv, xv + POL, xv + 2 * POL], BAND["schalten"], pe=(2,))
    b.draht(t["kontakt"], 1, k_p, 0)
    a, an = b.abgriff(minus, xv + POL)
    b.draht(a, an, k_m, 0)
    # BMK links neben den Kasten (darüber laufen die Leitungen zu x1/x2/PE)
    t["ventil"] = b.setze(ventil["pfad"], xv, BAND["steuer"] + 20, anker="x1", label=ventil["bmk"], infos=_infos(ventil),
                          label_xy=(-8 - 6.2 * len(ventil["bmk"]), 18))
    _kennzeichen(b, ventil, xv - 40, BAND["steuer"] + 70)
    b.draht(k_p, 1, t["ventil"], "x1")
    b.draht(k_m, 1, t["ventil"], "x2")
    b.draht(k_pe, 1, t["ventil"], "PE", gestrichelt=True)
    kabel(b, kabel_v, xv - 12, xv + 2 * POL + 12, BAND["schalten"] + 50)
    xs = spalte_links(spalte + 1) + 60
    y_s = BAND["steuer"] + 100
    t["sensor"] = b.setze(sensor["pfad"], xs, y_s, anker="1", label=sensor["bmk"], infos=_infos(sensor))
    y_k = BAND["klemme"] - 20
    # Signal ist Pin 2 (WH, Schließer), Pin 4 (BK) liegt auf Reserve – wie im Original; Klemmen in x-Reihenfolge
    k_res, k_sig = klemmen(b, xg5[0], [xg5[1][1], xg5[1][0]], [t["sensor"].pos("4")[0], t["sensor"].pos("2")[0]], y_k)
    b.draht(t["sensor"], "2", k_sig, 0)
    b.draht(t["sensor"], "4", k_res, 0)
    for pin, schiene in (("1", sensor_plus), ("3", sensor_minus)):
        xp = t["sensor"].pos(pin)[0]
        a, an = b.abgriff(schiene, xp, "oben")
        b.draht(t["sensor"], pin, a, an)
    di = sps_kanal(b, S.DI_KANAL, k_sig.pos(1)[0], BAND["klemme"] + 80, kanal, "DI")
    b.draht(k_sig, 1, di, "DI")
    # Kabel weiter links beginnen: der Kabelname darf nicht auf die Aderfarbe der ersten Ader fallen
    kabel(b, kabel_s, xs - 40, t["sensor"].pos("2")[0] + 12, y_s + 20)
    if ort:
        # Rahmen unter dem Ventilkabel (dessen Typtext steht bis ≈ 20 px darunter) und links mit Platz für Daten
        rahmen(b, xv - 90, BAND["steuer"] - 30, xs + 120, y_s + 10, ort)
    b.funktion(spalte, spalte, list(funktion_v or [ventil.get("funktion", "Ventil")]))
    b.funktion(spalte + 1, spalte + 1, list(funktion_s or ["Rückmeldung", kanal.get("funktion", "")]))
    return t


def proportionalventil(b, spalte, ventil, sollwert, plus, minus, leiste, klemmen_nr, kabel_v=None, ort="+MASCHINE",
                       funktion=None):
    """Proportionalventil mit Stecker A..F/PE (A 24 V, B GND, C GND Istwert, D Sollwert, E GND Sollwert,
    F Istwert): Versorgung von plus/minus auf A/B, Sollwert vom Analogausgang (sollwert = {"pfad", "bmk", "ao",
    "gnd"} – Kanalsymbol mit Anschlüssen unten) auf D/E; alle Adern über Klemmen `leiste` und Kabel."""
    pins = ["A", "B", "C", "D", "E", "F", "PE"]
    x0 = spalte_links(spalte) + 60
    xs = [x0 + 20 * i for i in range(len(pins))]
    y_v = BAND["feld"] - 20
    t = {"ventil": b.setze(ventil["pfad"], x0, y_v, anker="A", label=ventil["bmk"], infos=_infos(ventil))}
    y_k = BAND["klemme"] - 60
    kl = klemmen(b, leiste, klemmen_nr, xs, y_k, pe=(pins.index("PE"),))
    for k, p in zip(kl, pins):
        b.draht(k, 1, t["ventil"], p)
    for k, schiene in ((kl[0], plus), (kl[1], minus)):
        a, an = b.abgriff(schiene, k.pos(0)[0])
        b.draht(a, an, k, 0)
    so = b.setze(sollwert["pfad"], xs[3] - 20, BAND["schutz"], label=sollwert["bmk"])
    if sollwert.get("stecker"):                     # Steckerbezeichnung am Gerät (z. B. X1 bzw. X3 der Achskarte)
        b.text(sollwert["stecker"], so.pos(sollwert["ao"])[0] - 40, so.pos(sollwert["ao"])[1] - 14, "label")
    b.draht(so, sollwert["ao"], kl[3], 0)
    b.draht(so, sollwert["gnd"], kl[4], 0)
    kabel(b, kabel_v, xs[0] - 12, xs[-1] + 12, y_k + 50)
    if ort:
        rahmen(b, x0 - 60, y_k + 80, xs[-1] + 80, y_v + 120, ort)
    b.funktion(spalte, spalte + 1, list(funktion or [ventil.get("funktion", "Proportionalventil")]))
    return t


# ---------------------------------------------------------------------------------------------------------------
def wechsler_am_di(b, spalte, geraet, art, klemmleiste, klemmen_nr, kanal, kabel_feld=None, plus="L+", minus=None,
                   funktion=None, kanal2=None, funktion2=None):
    """Feldschalter mit Wechsler 11/12/14 an einen DI (EPLAN-Darstellung wie im Original Projekt G, Blatt 32):
        12 (Öffner, drahtbruchsicher) -> Ader 1 -> Klemme klemmen_nr[0] -> DI
        14 (Schließer)               -> Ader 2 -> Klemme klemmen_nr[1] -> Reserve bzw. kanal2 (zweiter DI)
        11 (Wurzel)                  -> Ader 3 -> Plusschiene
        PE des Geräts                -> GNYE  -> PE-Reihenklemme.
    art: "P" Druck, "T" Temperatur, "S" Schwimmer (Betätigungssymbol). Spalte = 156 px; kanal2 belegt die Folgespalte."""
    from .symbolbau import wechsler_feld
    x = spalte_mitte(spalte)
    x12, x11 = x - 40, x - 30
    xr, xp, xpe = x, x + 20, x + 40          # eng: rechts beginnt schon die Klemmenbeschriftung der Folgespalte
    y_k, y_top, y_m = BAND["schutz"] - 60, BAND["schutz"] - 90, BAND["schutz"] + 10
    y_kl = BAND["schalten"] + 20
    t = {"kontakt": b.setze(wechsler_feld(art), x11, y_k, anker="11", label=geraet["bmk"], infos=_infos(geraet))}
    k_sig, k_res = klemmen(b, klemmleiste, klemmen_nr[:2], [x12, xr], y_kl)
    b.draht(t["kontakt"], "12", k_sig, 0)
    b.leitung(t["kontakt"], "14", k_res, 0, ueber=[(x11, y_m), (xr, y_m)])
    a, an = b.abgriff(plus, xp, "oben")
    b.leitung(t["kontakt"], "11", a, an, ueber=[(x11, y_top), (xp, y_top)])
    erde = b.setze(S.ERDE, xpe, y_k + 20, anker="PE", drehung=180)
    k_pe = klemmen(b, klemmleiste, ["PE"], [xpe], y_kl, ort_text=False, pe=True)[0]
    b.draht(erde, "PE", k_pe, 0, gestrichelt=True)
    kabel(b, kabel_feld, x12 - 12, xpe + 12, BAND["schalten"] - 30)
    di = sps_kanal(b, S.DI_KANAL, x12, BAND["klemme"] - 40, kanal, "DI")
    b.draht(k_sig, 1, di, "DI")
    if kanal2:
        x2 = spalte_mitte(spalte + 1) - 40
        di2 = sps_kanal(b, S.DI_KANAL, x2, BAND["klemme"] - 40, kanal2, "DI")
        b.leitung(k_res, 1, di2, "DI", ueber=[(xr, BAND["klemme"] - 90), (x2, BAND["klemme"] - 90)])
        b.funktion(spalte + 1, spalte + 1, list(funktion2 or [kanal2.get("funktion", "")]))
    b.funktion(spalte, spalte, list(funktion or [kanal.get("funktion", "")]))
    return t
