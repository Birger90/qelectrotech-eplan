"""EPLAN-Projektarchiv (.zw1) -> QElectroTech-Projekt (Import, Stufe 2: mit EPLAN-Symbolgrafik).

    python eplan_nach_qet.py "<archiv>.zw1" [ziel.qet] [Plannummer] [--pdf] [--oeffnen] [--dataportal] [--ohne-sammlung]

Was entsteht:
- je EPLAN-Seite ein QET-Blatt mit Titel und Struktur (=Anlage+Ort/Blatt);
- je Funktion das Symbol aus der im Archiv eingebetteten EPLAN-Symbolbibliothek (Bibliothek, Symbolnummer,
  Variante A–P) als QET-Element mit Anschlüssen an den EPLAN-Anschlusspunkten, gesetzt am EPLAN-Einfügepunkt;
  erzeugt einmal je Variante in elements-company/EPLAN-Import/<Bibliothek>/ (eplan_grafik.py);
- BMK wie EPLAN es zeigt (ohne =/+ der Seite bzw. des Strukturkastens, ohne Teil des Gerätekastens),
  Anschlussbezeichnungen, Klemmennummern, Aderfarben, Kabeltypen an der EPLAN-Textlage;
- Gerätekästen/Black Boxes, Strukturkästen (strichpunktiert) und Kabeldefinitionslinien aus der Ausdehnung
  der Funktion; freie Seitengrafik (Linien, Bögen, Rechtecke, Texte) und Winkel/Abzweige als QET-Formen;
- alle Verbindungen der Seite als QET-Leitungen zwischen den Symbolanschlüssen (aus Connection.eod);
- Unterbrechungspunkte (IPoint) als verknüpfte Potentialpfeile, Potentialpunkte (z. B. PE) als Punkte;
- danach Klemmenpläne, Kabelübersicht und Stückliste aus den EPLAN-Auswertungen (Tabellenblätter).
Funktionen ohne Symbol im Archiv erscheinen weiter als Anschlusspunkte. Grundlage: zw1_import.py (Dekoder).

Bauteile (eplan_bibliothek.py): hat die Funktion einen Artikel, kommt zuerst das Bauteil der Firmensammlung mit
gleicher Bestellnummer (Anschlüsse über den Namen, Reihenklemmen über die Lage); fehlt der Artikel, holt
--dataportal ihn aus dem EPLAN Data Portal; sonst wird das Schaltbild aus dem Archiv mit Artikeldaten als Bauteil
unter Hersteller/Kategorie/Bestellnummer in die Sammlung geschrieben. --ohne-sammlung schaltet das ab."""
import collections
import csv
import hashlib
import json
import math
import os
import pickle
import re
import sys

HIER = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HIER))                      # Daten\vorlagen
sys.path.insert(0, HIER)
from planvorlagen import Plan, listen as L                     # noqa: E402
from planvorlagen import symbole as S                          # noqa: E402
from zw1_import import importieren                             # noqa: E402
import eplan_grafik as G                                       # noqa: E402
import eod_symbole as E                                        # noqa: E402
import eplan_einstellungen as EST                               # noqa: E402
import eplan_bibliothek as BIB                                 # noqa: E402
import eplan_kontaktspiegel as KS                              # noqa: E402
import eplan_fktdef as FD                                      # noqa: E402

PUNKT = S.U + "Allgemein/Verbindungen/EPLAN_Anschluss.elmt"
# erzeugte Symbole (EPLAN-Bibliothekssymbole, keine Projektdaten) in der Firmensammlung
SYMBOLE = "EPLAN-Import"
VARIANTE = "ABCDEFGHIJKLMNOP"
# Eigenschaften, deren Texte an Symbolen gezeigt werden (EPLAN-Eigenschaftsnummern): Klemmennummer, Ader-
# farbe/-nummer, Querschnitt, Funktionstext, Technische Kenngrößen, Kabeltyp/-daten
ZEIGEN = {20030, 20100, 20400, 20901, 31004, 31002, 20011, 20027, 20031, 20040, 20041, 20042, 20043, 20045, 20046, 20047}
# Artikeleigenschaften (EPLAN-Nummer -> Spalte der Stückliste) für Eigenschaftstexte an Symbolen und Kästen
ARTIKEL_SPALTE = {22002: "Typnummer (22002)", 22003: "Bestellnummer (22003)", 22004: "Bezeichnung 1 (22004)",
                  22005: "Bezeichnung 2 (22005)", 22007: "Hersteller (22007)", 22017: "Technische Kenngrößen (22017)"}
K = 1560 / 420                    # px je mm (A3-Blatt 420 mm auf 10 Spalten à 156 px)
X0, H_MM = 20, 297
QET_EXE = os.environ.get("QET_EXE", os.path.join(os.path.expanduser("~"), "Documents", "Code Projekte",
                                                  "qelectrotech-eplan", "build", "deploy", "QElectroTech.exe"))
TITELBLOECKE = os.path.join(os.environ.get("APPDATA", ""), "QElectroTech", "QElectroTech", "titleblocks")


def _csv(out, name):
    return list(csv.DictReader(open(os.path.join(out, name), encoding="utf-8-sig"), delimiter=";"))


def _kv(props, pid):
    for p, t, v in props or []:
        if p == pid and t == "kv":
            return {str(k): s for k, _, s in v}
    return {}


def _wert(props, pid):
    for p, t, v in props or []:
        if p == pid:
            return v
    return None


MASSSTAB = {}            # Seite -> Maßstab (11048); Lagen maßstäblicher Seiten sind Modellmaße


def _pos(props):
    o = _wert(props, 61201)
    if o:
        for q, _, v in o[1]:
            if q == 60762:
                seite = _wert(props, 60402)
                s = MASSSTAB.get(seite[1], 1.0) if seite else 1.0
                return (v[0] / s, v[1] / s) if s != 1.0 else v
    return None


def _de(text):
    """'de_DE@Einspeisung;en_US@Power supply;' -> 'Einspeisung'"""
    m = re.search(r"de_DE@([^;]*)", text or "")
    return G.ohne_uebersetzungsmarke(m.group(1) if m else (text or "").split(";")[0].split("@")[-1])


def _px(p):
    return round(X0 + p[0] * K), round((H_MM - p[1]) * K)


def _verschoben(f, d):
    """Grundform um d (mm) verschieben (Symbolgrafik ohne Funktion auf die Seite setzen)."""
    def v(q):
        return (q[0] + d[0], q[1] + d[1])
    if f[0] == "L":
        return ("L", v(f[1]), v(f[2]), f[3])
    if f[0] == "A":
        return ("A", v(f[1])) + tuple(f[2:])
    if f[0] == "P":
        return ("P", [v(q) for q in f[1]]) + tuple(f[2:])
    return ("T", v(f[1])) + tuple(f[2:])


def _symbolpfad(gr, lib, sid, vid, var, cache, rolle=None):
    """QET-Element zur EPLAN-Symbolvariante (einmal je Import geschrieben); None ohne Grafik.
    rolle: Master/Slave für aufgeteilte Geräte (eigene Datei je Rolle)."""
    key = (lib, vid, rolle)
    if key not in cache:
        formen, an = gr.formen(lib, vid), gr.anschluesse(lib, vid)
        if not formen and not an:
            cache[key] = None
            return None
        name = gr.symbolname(lib, sid)
        endung = "" if not rolle else ("_M" if rolle[0] == "master" else "_S" + rolle[1])
        datei = re.sub(r"[^A-Za-z0-9_.+-]", "_", name) + "_" + VARIANTE[(var or 0) % 16] + endung + ".elmt"
        rel = f"{SYMBOLE}/{lib.replace('.sdb', '')}/{datei}"
        ziel = os.path.join(S.COMPANY, *rel.split("/"))
        os.makedirs(os.path.dirname(ziel), exist_ok=True)
        xml = G.element_xml(f"EPLAN {lib.replace('.sdb', '')} {name} {VARIANTE[(var or 0) % 16]}", formen, an,
                            f"Aus EPLAN-Symbolbibliothek {lib}, Symbol {name}, Variante {VARIANTE[(var or 0) % 16]} "
                            f"(eplan_nach_qet.py)")
        xml = G.mit_rolle(xml, rolle)
        if not os.path.exists(ziel) or open(ziel, encoding="utf-8").read() != xml:
            open(ziel, "w", encoding="utf-8").write(xml)
        cache[key] = (S.U + rel, an)
    return cache[key]


def _lagen(symbol, eigene):
    """Eigenschaftstexte einer Platzierung: Hat die Funktion eigene Textlagen, sind das ALLE sichtbaren Texte
    (EPLAN kopiert die Symbolvorgaben beim Einfügen in die Funktion; was dort fehlt, wurde entfernt) – vom Symbol
    bleiben nur die Texte je Anschluss (20028/20029). Ohne eigene Lagen gelten die Vorgaben des Symbols."""
    if not eigene:
        aus = G.Lagen(symbol)
        aus.relation = dict(getattr(symbol, "relation", {}))
        aus.boxbreite = dict(getattr(symbol, "boxbreite", {}))
        aus.boxart = dict(getattr(symbol, "boxart", {}))
        return aus
    aus = G.Lagen({k: v for k, v in symbol.items() if k[0] in (20028, 20029)})
    aus.update(eigene)
    aus.relation = dict(getattr(eigene, "relation", {}))
    aus.boxbreite = dict(getattr(eigene, "boxbreite", {}))
    aus.boxart = dict(getattr(eigene, "boxart", {}))
    return aus


DIAGNOSE = [t for t in os.environ.get("EPLAN_DIAGNOSE", "").split("|") if t]   # Texte, deren Herkunft gemeldet wird
KLEMMEN_BMK_VON_LINKS = os.environ.get("EPLAN_KLEMMEN_VON_LINKS", "1") == "1"
VORZEICHEN = {1000: "==", 1100: "=", 1300: "++", 1200: "+", 1500: "&", 1600: "#", 20012: "-", 20015: "."}


def bmk_aus_format(fmt, werte):
    """Sichtbares BMK nach dem an der Funktion gespeicherten Anzeigeformat (60145) mit den Werten (60144):
    Zahl = Wert der Eigenschaft, [ID] = Vorzeichen der Ebene (=, +, -, .), <…> = Gruppe, die nur erscheint, wenn
    ein Wert darin belegt ist. Beispiele: <20013><20014> -> „X6“, <[1100]1100><[20012]20013<20014>> -> „=SB-A4“."""
    def gruppe(i):
        aus, belegt = "", False
        while i < len(fmt) and fmt[i] != ">":
            c = fmt[i]
            if c == "<":
                t, b, i = gruppe(i + 1)
                if b:
                    aus, belegt = aus + t, True
                i += 1
            elif c == "[":
                j = fmt.index("]", i)
                aus += VORZEICHEN.get(int(fmt[i + 1:j]), "")
                i = j + 1
            elif c.isdigit():
                j = i
                while j < len(fmt) and fmt[j].isdigit():
                    j += 1
                w = werte.get(int(fmt[i:j]), "")
                if w:
                    aus, belegt = aus + w, True
                i = j
            else:
                aus += c
                i += 1
        return aus, belegt, i
    try:
        return gruppe(0)[0]
    except (ValueError, IndexError):
        return None


def _seitennr(s):
    """Seitenname wie EPLAN: Seitennummer (11012) und Seitenunterzähler (11013) -> „1“, „1.1“, „1.a“."""
    nr, sub = s.get("11012", ""), s.get("11013", "")
    return f"{nr}.{sub}" if sub else nr


def _seitenfolge(nr, anl, ort):
    """Sortierschlüssel: Seitennummer numerisch, dann Unterzähler (Zahl vor Buchstabe), dann Struktur."""
    haupt, _, sub = nr.partition(".")
    return (int(haupt) if haupt.isdigit() else 10 ** 6, (0, int(sub), "") if sub.isdigit() else (1, 0, sub),
            anl, ort)


def projektinfo(arbeit):
    """Projekteigenschaften aus ProjectInfo.xml des Archivs: {id: Text} (mehrsprachige Texte deutsch)."""
    pfad = os.path.join(arbeit, "x", "ProjectInfo.xml")
    if not os.path.exists(pfad):
        return {}
    import xml.etree.ElementTree as ET
    info = {}
    for e in ET.parse(pfad).getroot().iter("Property"):
        if e.get("index", "0") == "0" and e.text:
            info[e.get("id")] = _de(e.text)
    return info


def _svg_feld(titel, zeilen, lage, groesse, gt, gw):
    """Kleines SVG für ein Schriftfeld-Feld: Überschrift + Wertzeilen (Aufbau wie die KUNDE-Vorlage)."""
    from xml.sax.saxutils import escape
    x, y_titel, y_wert = lage
    t = [f'<text x="{x}" y="{y_titel}" font-family="Tahoma" font-size="{gt}" font-weight="normal" '
         f'text-anchor="start" fill="#000">{escape(titel)}</text>']
    for i, z in enumerate(zeilen[:3]):
        t.append(f'<text x="{x}" y="{y_wert + i * gw * 1.15:.1f}" font-family="Tahoma" font-size="{gw}" '
                 f'font-weight="normal" text-anchor="start" fill="#000">{escape(z)}</text>')
    return (f'<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.2" '
            f'baseProfile="tiny" width="{groesse[0]}" height="{groesse[1]}" '
            f'viewBox="0 0 {groesse[0]} {groesse[1]}">{"".join(t)}</svg>')


def _schriftfeld(nr, info=None):
    """Schriftfeld des Imports aus der KUNDE-Vorlage. Anlagenbezeichnung (10011), Auftragsnummer/Kommission (10014)
    und Zeichnungsnummer (10013) kommen aus den Projekteigenschaften des Archivs, nie aus der Vorlage (die Vorlage
    trägt diese Werte als SVG-Bilder; sie werden je Projekt neu erzeugt)."""
    import base64
    info = info or {}
    name = "KUNDE_" + nr.replace(" ", "_")
    ziel = os.path.join(TITELBLOECKE, name + ".titleblock")
    vorlage = os.path.join(TITELBLOECKE, "KUNDE_Projekt_G.titleblock")
    s = open(vorlage, encoding="utf-8").read().replace("KUNDE_Projekt_G", name)
    s = s.replace(">=%plant<", ">%plant<").replace(">+%locmach<", ">%locmach<")
    anlage = [z.strip() for z in (info.get("10011") or "").replace("\\n", "\n").split("\n") if z.strip()]
    felder = {"anlage.svg": _svg_feld("Anlagenbezeichnung", anlage, (2.9, 7.5, 17.7), (221.1, 45.2), 5.6, 8.8),
              "auftrag.svg": _svg_feld("Auftragsnr.", [info.get("10014", "")], (5.7, 7.3, 18.8), (102.1, 22.6),
                                       4.6, 8.8),
              "zeichnung.svg": _svg_feld("Zeichnungs-Nr.", [info.get("10013") or nr], (5.7, 8.7, 20.3),
                                         (102.1, 22.6), 4.6, 8.8)}
    for datei, svg in felder.items():
        b64 = base64.b64encode(svg.encode("utf-8")).decode("ascii")
        s = re.sub(r'(<logo name="' + re.escape(datei) + r'" type="svg" storage="base64">)[^<]*',
                   lambda m, b=b64: m.group(1) + b, s)
    open(ziel, "w", encoding="utf-8").write(s)
    return name


def _schriftfeld_leer():
    """Leeres QET-Schriftfeld (eine Zeile, kein Inhalt): Rahmen und Schriftfeld zeichnet der Import aus dem
    EPLAN-Normblatt des Projekts (Form.eod Klasse 21) als Seitengrafik."""
    name = "EPLAN_Normblatt_leer"
    ziel = os.path.join(TITELBLOECKE, name + ".titleblock")
    os.makedirs(TITELBLOECKE, exist_ok=True)
    open(ziel, "w", encoding="utf-8").write(
        f'<titleblocktemplate name="{name}">\n<information>Leer – Normblatt kommt aus dem EPLAN-Projekt</information>\n'
        '<logos/>\n<grid rows="1;" cols="r100%;">\n<field row="0" col="0" name="" displaylabel="false" align="left" '
        'valign="center" fontsize="6" hadjust="false"><value><translation lang="de"></translation></value>'
        '<label><translation lang="de"></translation></label></field>\n</grid>\n</titleblocktemplate>\n')
    return name


def _datum(ts):
    """EPLAN-Zeitstempel (Sekunden seit 1970) -> TT.MM.JJJJ (Datumsformat der Original-PDFs)."""
    import datetime
    try:
        return datetime.datetime.fromtimestamp(int(ts)).strftime("%d.%m.%Y")
    except (TypeError, ValueError, OSError):
        return ""


def _anschluss_schluessel(n):
    """Sortierung der Anschlussbezeichnungen: numerisch, alphanumerisch, leer (EPLAN-Hilfe
    xessettingsgui_k_querverweisarten)."""
    n = (n or "").strip()
    if not n:
        return (2, 0, "")
    return (0, int(n), "") if n.isdigit() else (1, 0, n)


def _label_als_text(t):
    """Kennzeichen eines Bauteils als Benutzertext statt Elementinformation „label“ (QET-eigener Querverweis aus)."""
    sym = S.symbol(t.pfad)
    if sym.label_lage is None:                        # Symbol ohne Info-Text „label“
        return
    idx = sym.label_index
    lab = next((op_.get("label", "") for op_ in t.blatt.ops if op_.get("op") == "set_label"
                and op_.get("element") == t.ref), "")
    for prop_, wert_ in (("source", "text"), ("text", lab)):
        t.blatt.plan.lagen.append({"op": "set_element_text", "folio": t.blatt.folio, "element": t.ref,
                                   "index": idx, "property": prop_, "value": wert_})


def _kontaktspiegel(P, F, funk, gr, einst, blatt, fn_seite, massstab, teile, extra, querverweis, uebersicht, PG,
                    stat):
    """EPLAN-Kontaktspiegel je Hauptfunktion mit DMContactImage (Klasse 100 an Funktion oder Symbolvariante) als
    Seitengrafik (eplan_kontaktspiegel). Einträge: Funktionsschablonen der Artikel (Article.eod Klasse 99, Reihenfolge
    der Gerätedefinition) – platziert = Kontakt gleichen BMKs mit gleichen Anschlüssen (mit Querverweis), sonst
    nicht platziert (ohne Querverweis); weitere platzierte Kontakte danach, Anschlüsse numerisch sortiert (Hilfe:
    „Gerätedefinition hat Vorrang“). QETs eigener Spiegel dieser Master wird abgeschaltet (Kennzeichen als
    Benutzertext statt Elementinformation: QET hängt den Querverweis nur an den Info-Text „label“)."""
    if os.environ.get("EPLAN_OHNE_KONTAKTSPIEGEL"):    # Vergleichslauf ohne Spiegel
        return
    vorlagen = {}                                       # Artikelnummer -> [(Index, fd, efc6, efc7, Anschlüsse)]
    for e_ in P.get("Article.eod", {}).values():
        pr_ = e_.get("props") or []
        kv_ = {t[0]: t[2] for x in pr_ if x[1] == "kv" for t in x[2] if t[1] == 0}
        if not kv_.get(22001):
            continue
        liste = []
        for q in pr_:
            if q[0] == 60005 and q[1] == "obj" and q[2][0] == 99:
                o = q[2][1]
                kv9 = {(k, i): w for x in o if x[1] == "kv" for k, i, w in x[2]}
                an_ = [kv9[(21000, i)] for i in sorted(i for k, i in kv9 if k == 21000)]
                liste.append((E.wert(o, 60004) or 0, E.wert(o, 61381), E.wert(o, 61382), E.wert(o, 61383), an_))
        # Artikelsatz je (Artikelnummer 62041/22001, Variante 62042); die Funktion wählt ihn über 20100/20101
        # (FUNC_ARTICLE_PARTNR / FUNC_ARTICLE_VARIANT)
        vorlagen[(kv_[22001], str(E.wert(pr_, 62042) or "1"))] = sorted(liste)
    # Symbol einer nicht platzierten Funktion: siehe KS.symbol_fuer
    nach_bmk_k = collections.defaultdict(list)
    for f_, e_ in F.items():
        if e_["cls"] == 17 and e_["props"] and fn_seite.get(f_) in blatt and _pos(e_["props"]):
            nach_bmk_k[funk.get(f_, {}).get("bmk", "")].append(f_)
    folge = {pid: i for i, pid in enumerate(blatt)}
    normbl = {}
    for fid, e in F.items():
        pr = e["props"]
        if e["cls"] != 17 or not pr or E.wert(pr, 0xeb16) is not None:
            continue
        pid = fn_seite.get(fid)
        p = _pos(pr)
        if pid not in blatt or not p or massstab.get(pid, 1.0) != 1.0:
            continue
        v = gr.variante(E.wert(pr, 0xef4d), E.wert(pr, 0xef4e), E.wert(pr, 0xef4f))
        o = KS.objekt(pr, gr.var[v[0]][v[2]][1] if v else None)
        if o is None:
            continue
        m = KS.maske(o, einst)
        if m & 0x2000800 or m & 0x60000 or m & 0xc400:
            stat["kontaktspiegel_nicht_umgesetzt"] += 1      # Formular/Tabelle/Drehung/Ausblenden/Linien offen
            continue
        bmk = funk.get(fid, {}).get("bmk", "")
        werte = {(k, i): w for k, i, w in (E.wert(pr, 0xea6b) or [])}
        roh_h = [n.strip() for n in (funk.get(fid, {}).get("anschluesse") or "").split("|")]
        kontakte = []
        for k_ in nach_bmk_k.get(bmk, []):
            pk = F[k_]["props"]
            if k_ == fid or E.wert(pk, 0xeb16) is None or E.wert(pk, 0xefc5) not in (201, 202, 203, 204) \
                    or uebersicht(F[k_]):
                continue
            kontakte.append(k_)
        kontakte.sort(key=lambda k_: (folge.get(fn_seite.get(k_), 0), _pos(F[k_]["props"])[0],
                                      -_pos(F[k_]["props"])[1]))

        def namen(k_):
            return [n.strip() for n in (funk.get(k_, {}).get("anschluesse") or "").split("|")]
        eintraege, frei = [], list(kontakte)
        for i in sorted(i for k, i in werte if k == 20100):
            for _, fd, c6, c7, an_ in vorlagen.get((werte[(20100, i)], str(werte.get((20101, i)) or "1")), []):
                if fd == E.wert(pr, 0xefc5) and an_ == roh_h[:len(an_)]:
                    continue                                    # Schablone der Hauptfunktion selbst
                treffer = next((k_ for k_ in frei if namen(k_)[:len(an_)] == an_
                                and E.wert(F[k_]["props"], 0xefc5) == fd), None)
                if treffer:
                    frei.remove(treffer)
                    eintraege.append(("platziert", treffer))
                elif fd in (201, 202, 203, 204):
                    eintraege.append(("frei", (fd, c6, c7, an_)))
        frei.sort(key=lambda k_: _anschluss_schluessel(namen(k_)[0] if namen(k_) else ""))
        eintraege += [("platziert", k_) for k_ in frei]
        liste = []
        for art, x in eintraege:
            if art == "platziert":
                pk = F[x]["props"]
                vk = gr.variante(E.wert(pk, 0xef4d), E.wert(pk, 0xef4e), E.wert(pk, 0xef4f))
                if not vk:
                    stat["kontaktspiegel_eintrag_ohne_symbol"] += 1
                    continue
                liste.append({"lib": vk[0], "sid": vk[1], "namen": namen(x), "qvw": querverweis(x, pid)})
            else:
                ls = KS.symbol_fuer(gr, v[0] if v else None, *x[:3])
                if not ls:
                    stat["kontaktspiegel_eintrag_ohne_symbol"] += 1
                    continue
                liste.append({"lib": ls[0], "sid": ls[1], "namen": list(x[3]), "qvw": ""})
        if not liste:
            continue
        nbn = _kv(PG[pid]["props"], 60011).get("11016") or einst.get("TrDMProject.Frame", "")
        if nbn not in normbl:
            normbl[nbn] = (gr.normblatt(nbn) or {}).get("kv", {})
        kv_s = {int(k): w for k, w in _kv(PG[pid]["props"], 60011).items() if k.isdigit()}
        O = KS.ursprung(o, m, p, kv_s, normbl[nbn])
        formen, ohne = KS.zeichnen(gr, m, O, liste, {k: G.text_de(w) for k, w in werte.items()})
        stat["kontaktspiegel_eintrag_ohne_symbol"] += ohne
        extra[pid] += formen
        stat["kontaktspiegel"] += 1
        t = teile.get(fid)
        if t and S.symbol(t.pfad).link_type == "master":
            lab = next((op_.get("label", "") for op_ in t.blatt.ops if op_.get("op") == "set_label"
                        and op_.get("element") == t.ref), "")
            for prop_, wert_ in (("source", "text"), ("text", lab)):
                t.blatt.plan.lagen.append({"op": "set_element_text", "folio": t.blatt.folio, "element": t.ref,
                                           "index": S.symbol(t.pfad).label_index, "property": prop_,
                                           "value": wert_})


def _fortschritt(prozent, schritt):
    """Eine Zeile für den Fortschrittsbalken des QET-Dialogs (Datei > EPLAN-Projekt importieren); der Dialog übersetzt den Schritt."""
    print(f"FORTSCHRITT {int(prozent)} {schritt}", flush=True)


def umsetzen(archiv, ziel_qet, plannr=None, arbeit=None, nur_seiten=None, neu_dekodieren=True, data_portal=False,
             bibliothek=True):
    """nur_seiten: Seitennummern (Text) für einen Probelauf ohne Listen; neu_dekodieren=False nutzt parsed.pkl;
    bibliothek: Bauteile der Firmensammlung nutzen und fehlende Schaltbilder dort ablegen;
    data_portal: fehlende Artikel vorher aus dem EPLAN Data Portal in die Sammlung holen."""
    _fortschritt(2, "lesen")
    plannr = plannr or os.path.splitext(os.path.basename(archiv))[0]
    arbeit = arbeit or os.path.join(os.path.dirname(os.path.abspath(ziel_qet)), "_eplan_import")
    out = (importieren(archiv, arbeit) if neu_dekodieren or not os.path.exists(os.path.join(arbeit, "parsed.pkl"))
           else os.path.join(arbeit, "out"))
    _fortschritt(15, "funktionen")
    P = pickle.load(open(os.path.join(arbeit, "parsed.pkl"), "rb"))
    F, PG, IP, PO = P["Function.eod"], P["Page.eod"], P.get("IPoint.eod", {}), P.get("Potential.eod", {})
    # 3D-Platzierungen (DMFunction3D, Montageaufbau) gehören nicht in den Schaltplan: erkennbar an Feldern, die laut
    # EPLAN-Metamodell nur DMFunction3D registriert (Klassen 170–185 in Projekt A/Phoenix, keine Schaltplanklasse trägt sie)
    nur_3d = {0xf412, 0xf413, 0xf414, 0xf417, 0xf418, 0xfa19}
    F = {k: v for k, v in F.items() if not (v["props"] and any(q[0] in nur_3d for q in v["props"]))}
    funk = {int(r["funktion_id"]): r for r in _csv(out, "Funktionen_dekodiert.csv") if int(r["funktion_id"]) in F}
    artikel = collections.defaultdict(list)
    for r in _csv(out, "Stueckliste_je_BMK.csv"):
        if r.get("Bestellnummer (22003)") and int(r["Funktion_id"]) in F:
            artikel[int(r["Funktion_id"])].append(r)

    _fortschritt(20, "seiten")
    # Seiten in Blattreihenfolge
    seiten = []
    for pid, e in PG.items():
        s = _kv(e["props"], 60049)
        if not s:
            continue
        nr = _seitennr(s)
        titel = _de(_kv(e["props"], 60011).get("11011", ""))
        seiten.append((pid, s.get("1100", ""), s.get("1200", ""), nr, titel))
    # Seitenreihenfolge wie der EPLAN-Seiten-Navigator: Strukturkennzeichen in der Reihenfolge der
    # Strukturkennzeichen-Verwaltung (Location.eod 60083, Hilfe pleditorgui_k_start), darin Seitenname
    reihe = {}
    for e_ in P.get("Location.eod", {}).values():
        pr_ = {q[0]: q[2] for q in (e_["props"] or []) if q[0] in (60081, 60082, 60083)}
        if 60082 in pr_ and isinstance(pr_.get(60083), float):
            reihe[(str(pr_.get(60081)), pr_[60082])] = pr_[60083]

    def rang(block, wert):
        return (-math.inf, "") if not wert else (reihe.get((block, wert), math.inf), wert)
    seiten.sort(key=lambda a: (rang("1100", a[1]), rang("1200", a[2]), _seitenfolge(a[3], a[1], a[2])))
    if nur_seiten:
        seiten = [s for s in seiten if s[3] in nur_seiten]

    # Formular der Auswertungsseite = Name 11015 + Seitentyp ea8f (= Formulartyp ede5 in Form.eod)
    formular = {pid: (_kv(e["props"], 60011).get("11015", ""), E.wert(e["props"], 0xea8f))
                     if _kv(e["props"], 60011).get("11015") else "" for pid, e in PG.items()}
    # Maßstab (11048, z. B. 10 = 1:10 im Schrankaufbau): Lagen auf solchen Seiten sind Modellmaße
    massstab = {}
    for pid_, e_ in PG.items():
        try:
            massstab[pid_] = float((_kv(e_["props"], 60011).get("11048") or "1").replace(",", ".")) or 1.0
        except ValueError:
            massstab[pid_] = 1.0
    MASSSTAB.clear()
    MASSSTAB.update(massstab)
    pinfo = projektinfo(arbeit)
    G.schrift_einstellen(EST.lesen(arbeit), pinfo.get("10043"))   # „Texte vergrößert“ (Kompatibilität)

    def strukturformat(fd):
        """Strukturformat des Projekts je Objektart (10055 allgemein, 10057 SPS, 10059 Klemmen, 10060 Stecker,
        10061 Kabel): welche Projektstrukturen (10002 Anlage, 10004 Einbauort) das BMK trägt."""
        k = {4: "10061", 3: "10057", 1: "10059"}.get((fd or 0) // 100, "10055")
        return pinfo.get(k) or pinfo.get("10055") or "<10002><10004>"

    def bmk_ident(bmk, fd):
        """BMK (identifizierend, 20005) im Strukturformat: Anlage/Ort nur, wenn das Format sie enthält; das
        Vorzeichen „-“ nur nach einer Projektstruktur (Projekt A-Kabel „W292“, Projekt B-Kabel „=CB-W1.1“)."""
        m = re.match(r"(=[^+\-]*)?(\+[^\-]*)?-?(.*)$", bmk or "")
        f_ = strukturformat(fd)
        teile = (m.group(1) or "" if "<10002>" in f_ else "") + (m.group(2) or "" if "<10004>" in f_ else "")
        return f"{teile}-{m.group(3)}" if teile and m.group(3) else (teile or m.group(3))
    plan = Plan(f"EPLAN-Import {plannr}", os.path.dirname(os.path.abspath(ziel_qet)), _schriftfeld_leer())
    blatt = {}
    for pid, anl, ort, nr, titel in seiten:
        blatt[pid] = plan.blatt(titel or f"Seite {nr}", "=" + anl if anl else "", "+" + ort if ort else "", nr)
        # Spaltenleiste zeichnet das EPLAN-Normblatt selbst (Platzhalter 19005)
        blatt[pid].ops.append({"op": "set_folio_border", "folio": blatt[pid].folio, "property": "display-columns",
                               "value": "false"})

    def im_blatt(p):
        return p and -5 <= p[0] <= 425 and 20 <= p[1] <= 300

    _fortschritt(25, "symbole")
    # Symbolbibliotheken und Seitengrafik
    gr = G.Grafik(os.path.join(arbeit, "x"))
    gr.bibliotheken_zuordnen([e["props"] for e in F.values() if e["props"]])
    cache, formen_je_blatt, texte_je_blatt = {}, collections.defaultdict(list), collections.defaultdict(list)
    bilder_je_blatt = collections.defaultdict(list)
    stat = collections.Counter()

    # Kästen (Gerätekasten/Black Box, Strukturkasten) und Linien (Kabeldefinition) aus der Ausdehnung
    # eb1e/eb1f (min/max relativ zum Einfügepunkt, mm); dienen auch dem sichtbaren BMK
    artikel_kv = {}                                    # Artikelnummer (22001) -> {Eigenschaft: Wert} (Index 0)
    for _, e_ in P.get("Article.eod", {}).items():
        kv_ = {t[0]: t[2] for x in (e_.get("props") or []) if x[1] == "kv" for t in x[2] if t[1] == 0}
        if kv_.get(22001):
            artikel_kv[kv_[22001]] = kv_

    def artikel_von(pr):
        nr_ = next((w for k, i, w in (E.wert(pr, 0xea6b) or []) if k == 20100 and i == 1 and w), "")
        return nr_, artikel_kv.get(nr_, {})

    def sichtbar_roh(fid):
        pr_ = F[fid]["props"]
        fmt_, kv_ = E.wert(pr_, 60145), E.wert(pr_, 60144)
        if isinstance(fmt_, str) and isinstance(kv_, list) and all(isinstance(t, tuple) for t in kv_):
            return bmk_aus_format(fmt_, {k: v for k, i, v in kv_ if i == 0}) or ""
        return funk.get(fid, {}).get("bmk", "")

    def panel_rolle(fid, pr):
        """Rolle im Fork-Panelmodell, nur aus EPLAN-Daten: Funktionsdefinition 7 = Montageplatte -> surface;
        Artikel-Produktgruppe 55 (Schaltschrank) / 49 (Gehäuse) -> enclosure; 53 Kabelkanäle -> duct;
        Produktuntergruppe 5/140 Tragschiene -> din_rail, 142 C-Profilschiene -> c_profile."""
        fd_ = E.wert(pr, 0xefc5)
        nr_, a_ = artikel_von(pr)
        pg_, ug_ = str(a_.get(22041, "")), str(a_.get(22028, ""))
        if pg_ in ("55", "49"):
            r_ = {"role": "enclosure"}
        elif fd_ == 7:
            r_ = {"role": "surface", "origin": "0"}
        elif pg_ == "53":
            r_ = {"role": "duct"}
        elif ug_ in ("5", "140"):
            r_ = {"role": "din_rail"}
        elif ug_ == "142":
            r_ = {"role": "c_profile"}
        else:
            return None
        bez_ = sichtbar_roh(fid)
        if bez_:
            r_["designation"] = bez_
        if nr_:
            r_["article"] = a_.get(22003) or nr_
            if a_.get(22007):
                r_["manufacturer"] = BIB.HERSTELLER.get(a_[22007], a_[22007])
        return r_
    kaesten = collections.defaultdict(list)            # seite -> [(x0, y0, x1, y1, struktur, gerät, klasse)]
    extra = collections.defaultdict(list)              # seite -> Grundformen
    box_rect = {}                                      # Funktion -> (x0, y0, x1, y1, seite, Index in extra)
    for fid, e in F.items():
        seite, p = _wert(e["props"], 60402), _pos(e["props"])
        if not seite or seite[1] not in blatt or not im_blatt(p):
            continue
        a_, b_ = E.wert(e["props"], 0xeb1e), E.wert(e["props"], 0xeb1f)          # Gerät, Kabel
        if E.wert(e["props"], 0xf0bf) or E.wert(e["props"], 0xf0c0):              # Strukturkasten
            a_, b_ = list(E.wert(e["props"], 0xf0bf) or (0.0, 0.0)), list(E.wert(e["props"], 0xf0c0) or (0.0, 0.0))
        if not (isinstance(a_, list) and isinstance(b_, list) and len(a_) == 2 and len(b_) == 2):
            continue
        if not all(abs(v) < 1e5 for v in a_ + b_):
            continue
        # Ausdehnung in Modellmaß zum Einfügepunkt in Modellmaß (die Formen werden danach als Ganzes auf Papiermaß
        # gebracht); Kästen für Lagevergleiche (kaesten, box_rect) in Papiermaß wie _pos
        s_m = massstab.get(seite[1], 1.0) or 1.0
        rx, ry = p[0] * s_m, p[1] * s_m
        X0, Y0, X1, Y1 = rx + a_[0], ry + a_[1], rx + b_[0], ry + b_[1]
        x0, y0, x1, y1 = X0 / s_m, Y0 / s_m, X1 / s_m, Y1 / s_m
        if X1 - X0 < 0.5 and Y1 - Y0 < 0.5:
            continue
        if X1 - X0 < 0.5 or Y1 - Y0 < 0.5:
            extra[seite[1]].append(("L", (X0, Y0), (X1, Y1), False))         # Kabel-/Schirmdefinitionslinie
            continue
        m = re.match(r"(=[^+\-]*)?(\+[^\-]*)?(-.*)?$", funk.get(fid, {}).get("bmk", ""))
        st, ge = ((m.group(1) or "") + (m.group(2) or ""), m.group(3) or "") if m else ("", "")
        kaesten[seite[1]].append((x0, y0, x1, y1, st, ge, e["cls"], fid))
        box_rect[fid] = (x0, y0, x1, y1, seite[1], len(extra[seite[1]]))
        rolle_ = panel_rolle(fid, e["props"])
        if rolle_:
            # Schrankaufbau (Fork-Panelmodell): Montageplatte -> Montagefläche, Schaltschrank -> Gehäuse
            attr_ = dict(rolle_)
            if attr_["role"] in ("surface", "enclosure"):
                attr_["mm_per_unit"] = f"{s_m / G.K:.6g}"
            extra[seite[1]].append(("R", (X0, Y0), (X1, Y1), attr_))
        else:
            extra[seite[1]].append(("P", [(X0, Y0), (X1, Y0), (X1, Y1), (X0, Y1), (X0, Y0)], True, False,
                                    e["cls"] == 88))
    info_seite = {pid: (anl, ort) for pid, anl, ort, _, _ in seiten}

    def sichtbar(bmk, pid, p, fid=None):
        """BMK wie EPLAN es zeigt. Vorrang: das an der Funktion gespeicherte Anzeigeformat (60145/60144, von EPLAN
        selbst berechnet); sonst Näherung: =Anlage/+Ort nur, wenn abweichend von Seite bzw. umgebendem
        Strukturkasten; Gerätekennzeichen ohne den Teil des umgebenden Gerätekastens."""
        pr_ = F[fid]["props"] if fid in F and F[fid]["props"] else None
        fmt_ = E.wert(pr_, 60145) if pr_ else None
        kv_ = E.wert(pr_, 60144) if pr_ else None
        if fmt_ and isinstance(fmt_, str) and isinstance(kv_, list) and all(isinstance(t, tuple) for t in kv_):
            w_ = bmk_aus_format(fmt_, {k: v for k, i, v in kv_ if i == 0})
            if w_ is not None:
                return w_
        m = re.match(r"(=[^+\-]*)?(\+[^\-]*)?(-.*)?$", bmk or "")
        if not m:
            return bmk
        an, ot, ge = m.group(1) or "", m.group(2) or "", m.group(3) or ""
        anl, ort = info_seite.get(pid, ("", ""))
        um = [k for k in kaesten.get(pid, []) if k[7] != fid and k[0] - 0.1 <= p[0] <= k[2] + 0.1
              and k[1] - 0.1 <= p[1] <= k[3] + 0.1]
        anl_soll = {"=" + anl} | {re.match(r"(=[^+]*)?", k[4]).group(0) for k in um if k[6] == 88}
        ort_soll = {"+" + ort} | {"+" + k[4].split("+", 1)[1] for k in um if k[6] == 88 and "+" in k[4]}
        if an in anl_soll:
            an = ""
        if ot in ort_soll:
            ot = ""
        for k in um:
            if k[6] != 88 and k[5] and k[4] + k[5] == (m.group(1) or "") + (m.group(2) or "") + ge:
                return ""                          # Funktion im eigenen Gerätekasten (SPS-Anschluss, Makro)
            if k[6] != 88 and k[5] and ge.startswith(k[5] + "-"):
                ge = ge[len(k[5]):]
                break
        return an + ot + ge

    def qv_relevant(f):
        """EsBaseFunctionXRefEvaluator::isRelevantForCrossReferences (EEsCrossReferenceu 0x180026530): 20021 der
        Funktion selbst – 1 nie, 2 immer; 0 automatisch: nur mit BMK und sichtbarem BMK (ESNameTools::hasInstanceName).
        Gilt für die eigene Funktion (EsStdFunctionXRefEvaluator::collectCrossreferencedObjects 0x18005aa20 prüft
        zuerst sich selbst) und für jedes Ziel (getXRefForGedAsString 0x18005c2b0 prüft jedes gesammelte Objekt).
        Nicht umgesetzt: param_3 (Ziel Kategorie 200 mit 20047 bzw. Darstellungsart 8 gilt ohne sichtbares BMK)."""
        pr_ = F[f]["props"] if f in F and F[f]["props"] else None
        if not pr_:
            return False
        try:
            z_ = int({(k_, i_): w_ for k_, i_, w_ in (E.wert(pr_, 0xea6b) or [])}.get((20021, 0)) or 0)
        except ValueError:
            z_ = 0
        if z_:
            return z_ == 2
        b_ = funk.get(f, {}).get("bmk", "")
        return bool(b_) and bool(sichtbar_gespeichert(b_, fn_seite.get(f), _pos(pr_) or (0, 0), f))

    def hat_spiegel(f):
        """Hauptfunktion mit EPLAN-Kontaktspiegel (DMContactImage, Klasse 100 an Funktion oder Symbolvariante)."""
        pr_ = F[f]["props"]
        v_ = gr.variante(E.wert(pr_, 0xef4d), E.wert(pr_, 0xef4e), E.wert(pr_, 0xef4f))
        return KS.objekt(pr_, gr.var[v_[0]][v_[2]][1] if v_ else None) is not None

    def qv_ziele(fid, bmk):
        """Ziele des Haupt-/Nebenfunktionsquerverweises (20300) im Schaltplan: andere platzierte Funktionen gleichen
        BMKs ohne Artikelplatzierungen (Funktionsdefinition 8, Schaltschrankaufbau) und in gleicher Darstellung
        (Übersicht/Schaltplan). Hauptfunktionen mit Kontaktspiegel zeigen keinen Text (Archivdaten Projekt A/Projekt E/Projekt B:
        Spulen mit Spiegel tragen den 20300-Text des Symbols, das Original zeigt dort nichts)."""
        pr_ = F[fid]["props"]
        ueb_ = uebersicht(F[fid])
        kand = [f for f in nach_bmk.get(bmk, []) if f != fid and _pos(F[f]["props"])
                and E.wert(F[f]["props"], 0xefc5) != 8 and uebersicht(F[f]) == ueb_]
        if E.wert(pr_, 0xeb16) is not None:
            return [f for f in kand if E.wert(F[f]["props"], 0xeb16) is None][:1]
        if hat_spiegel(fid):
            return []
        folge_ = {pid: i for i, (pid, *_r) in enumerate(seiten)}
        kand = [f for f in kand if E.wert(F[f]["props"], 0xeb16) is not None]
        kand.sort(key=lambda f: (folge_.get(fn_seite.get(f), 0), _pos(F[f]["props"])[0], -_pos(F[f]["props"])[1]))
        return kand

    def sichtbar_gespeichert(bmk, pid, p, fid):
        """Sichtbares BMK für die Querverweisanzeige „automatisch“ (20021 = 0): EPLAN speichert das berechnete
        sichtbare BMK als 60145/60144 nur, wenn es nicht leer ist (Archivdaten: ohne 60145 zeigt EPLAN in Projekt A/Projekt E/Projekt B
        516 von 527 Platzierungen keinen Querverweis); ohne 60145 also leer."""
        pr_ = F[fid]["props"] if fid in F and F[fid]["props"] else None
        if not pr_ or E.wert(pr_, 60145) is None:
            return ""
        return sichtbar(bmk, pid, p, fid)

    # Eigenschaftswerte je BMK (Zusatzfelder 20901 u. a. stehen an der Hauptfunktion, z. B. der SPS-Karte)
    je_bmk = collections.defaultdict(dict)
    je_bmk_fid = {}                                   # BMK -> Funktion mit Artikel (Hauptfunktion)
    # Hauptfunktion (ohne eb16) zuerst: nur sie trägt die Artikel des Betriebsmittels (Hilfe Betriebsmittel/Artikel);
    # Nebenfunktionen wie Artikelplatzierungen im Schrankaufbau (Funktionsdefinition 8) haben eigene Artikelreferenzen
    for fid, e in sorted(F.items(), key=lambda fe: E.wert(fe[1]["props"] or [], 0xeb16) is not None):
        b_ = funk.get(fid, {}).get("bmk", "")
        if artikel.get(fid):
            je_bmk_fid.setdefault(b_, fid)
        for k, i, w in (E.wert(e["props"] or [], 0xea6b) or []):
            je_bmk[b_].setdefault((k, i), w)
    # ermittelte symbolische Adresse (20201/20403) wie EPLAN: vom Anschluss über Verbindungen, durch
    # Reihenklemmen (Funktionsdefinition 100) hindurch, bis zum ersten anderen Gerät -> „BMK:Anschluss“
    nachbarn = collections.defaultdict(list)
    platziert = collections.defaultdict(set)    # Funktion -> Anschlussindizes mit platzierter Verbindung (Seite 60402)
    for r in _csv(out, "Verbindungen_aufgeloest.csv"):
        try:
            a = (int(r["A_objekt_id"] or 0), int(r["A_anschlussindex_0basiert"] or 0))
            z = (int(r["B_objekt_id"] or 0), int(r["B_anschlussindex_0basiert"] or 0))
        except ValueError:
            continue
        nachbarn[a].append((z, r["B_berechnet"]))
        nachbarn[z].append((a, r["A_berechnet"]))
        if r["seite_id"]:
            platziert[a[0]].add(a[1])
            platziert[z[0]].add(z[1])

    def ziel(fid, idx, anl, ort):
        gesehen, offen = {(fid, idx)}, [(fid, idx)]
        while offen:
            for (o, i), text in nachbarn.get(offen.pop(0), []):
                if (o, i) in gesehen:
                    continue
                gesehen.add((o, i))
                fe = F.get(o)
                if fe and fe["props"] and E.wert(fe["props"], 0xefc5) == 100:      # Reihenklemme: durchgehen
                    n = len((funk.get(o, {}).get("anschluesse") or "").split("|"))
                    offen += [(o, k) for k in range(n) if (o, k) not in gesehen]
                    gesehen.update((o, k) for k in range(n))
                    continue
                return re.sub(r"^=" + re.escape(anl) + r"(?=[+\-])", "", text or "")
        return ""

    # Symbolische Adresse (ermittelt) wie EPLAN (Abschnitt 19 „SPS-Zielverfolgung“): vom SPS-Anschluss über die
    # Verbindungen; an jeder erreichten Funktion gilt die Anschlusslogik (lokale Logik der Funktion, sonst die der
    # Funktionsdefinition, DMFunction::getLogicalDefinition). Signalart Sensor (Eingang DE/AE) bzw. Aktor (DA/AA)
    # = gefunden; sonst weiter über „Zielverfolgung (SPS) nach“ (eb07) zum anderen Anschluss; ohne Weiterleitung
    # Abbruch -> BMK der letzten Funktion (Hilfe plcgui_k_symbolischeadressen). Text = Endpunkt-Text ohne „:Anschluss“.
    fd_logik = FD.anschlusslogik(arbeit)
    stat["sps_logik_nicht_lesbar"] = fd_logik.get("_nicht_lesbar", 0)
    ip_gleich = collections.defaultdict(list)          # Abbruchstellen gleichen Namens sind verbunden
    for oid_, e_ in IP.items():
        t_ = {str(k_): w_ for k_, i_, w_ in (E.wert(e_["props"], 0xeaf0) or [])}
        n_ = "".join(t_.get(k_, "") for k_ in ("20012", "20013", "20014")) or \
            "".join(_kv(e_["props"], 60144).get(k_, "") for k_ in ("20013", "20014"))
        if n_:
            ip_gleich[n_].append(oid_)
    ip_von = {o_: n_ for n_, l_ in ip_gleich.items() for o_ in l_}

    def anschluss_logik(fid, idx):
        pr_ = F[fid]["props"]
        lok_ = [dict((a, b) for a, _, b in v[1]) for p, t, v in pr_ if p == 0xee86 and t == "obj"]
        lg_ = lok_ or fd_logik.get((E.wert(pr_, 0xefc5), E.wert(pr_, 0xefc6), E.wert(pr_, 0xefc7)), [])
        if 0 <= idx < len(lg_) and lg_[idx].get(0xeb01) not in ("n", "z"):
            return lg_[idx]
        var_ = [c for c in lg_ if c.get(0xeb01) in ("n", "z")]   # variable Definition: „z“ = letzter Anschluss
        n_ = len((funk.get(fid, {}).get("anschluesse") or "").split("|"))
        z_ = [c for c in var_ if c.get(0xeb01) == "z"]
        if z_ and idx == n_ - 1:
            return z_[0]
        return next((c for c in var_ if c.get(0xeb01) == "n"), {})

    def sps_ziel(fid, idx):
        """Endpunkt-Text des gefundenen bzw. letzten Geräts (ohne Anschluss) oder None (keine Suche möglich)."""
        sig_ = anschluss_logik(fid, idx).get(0xeb06)
        if sig_ not in FD.SIGNAL_EINGANG + FD.SIGNAL_AUSGANG:
            stat["sps_symb_ohne_signalart"] += 1
            return None
        such_ = FD.SIGNAL_SENSOR if sig_ in FD.SIGNAL_EINGANG else FD.SIGNAL_AKTOR
        gesehen_, offen_, letzt_ = {(fid, idx)}, [(fid, idx)], None
        while offen_:
            for (o, k), text in nachbarn.get(offen_.pop(0), []):
                if (o, k) in gesehen_:
                    continue
                gesehen_.add((o, k))
                if o in ip_von:                                       # Abbruchstelle: alle gleichnamigen
                    for o2 in ip_gleich[ip_von[o]]:
                        if (o2, 0) not in gesehen_:
                            gesehen_.add((o2, 0))
                            offen_.append((o2, 0))
                    continue
                if o not in F or not F[o]["props"]:
                    continue
                an_ = ((funk.get(o, {}).get("anschluesse") or "").split("|") + [""] * (k + 1))[k].strip()
                t_ = text or ""
                t_ = t_[:-len(an_) - 1] if an_ and t_.endswith(":" + an_) else t_
                letzt_ = t_
                c_ = anschluss_logik(o, k)
                if c_.get(0xeb06) == such_:
                    return t_
                w_ = c_.get(0xeb07) or 0
                if w_ > 0 and (o, w_ - 1) not in gesehen_:
                    gesehen_.add((o, w_ - 1))
                    offen_.append((o, w_ - 1))
                elif c_.get(0xeb0f):
                    stat["sps_symb_verweis_text"] += 1               # Verweis auf andere Funktion: nicht umgesetzt
        return letzt_

    _fortschritt(40, "portal" if data_portal else "sammlung")
    # Firmensammlung: vorhandene Bauteile zuerst; fehlende Artikel optional aus dem Data Portal holen
    sammlung = BIB.Sammlung() if bibliothek else None
    if sammlung is not None and data_portal:
        fehlt = {}
        for fid_, arts in artikel.items():
            fe = F.get(fid_)
            fd_ = E.wert(fe["props"], 0xefc5) if fe and fe["props"] else None
            for a in arts:
                b_ = a.get("Bestellnummer (22003)", "")
                if b_ and fd_ != 400 and not sammlung.hat(b_) and not BIB.ZUBEHOER.search(a.get("Bezeichnung 1 (22004)", "")):
                    fehlt[b_] = (b_, a.get("Hersteller (22007)", ""), BIB.kategorie(fd_))
        if fehlt:
            erg_dp = sammlung.data_portal(sorted(fehlt.values()), QET_EXE, os.path.join(arbeit, "data_portal.txt"))
            stat["data_portal_ok"] = sum(1 for v in erg_dp.values() if v.get("ok"))
            stat["data_portal_fehlt"] = len(fehlt) - stat["data_portal_ok"]
            sammlung = BIB.Sammlung()

    alle_seiten = {}
    for pid_, e_ in PG.items():
        k_ = _kv(e_["props"], 60049)
        if k_:
            alle_seiten[pid_] = (k_.get("1100", ""), k_.get("1200", ""), _seitennr(k_))
    pfadtexte = collections.defaultdict(list)          # Seite -> [(x, Text)] der Pfad-Funktionstexte (Ebene 110)
    for _, (c_, p_) in gr.seite.items():
        o_ = E.wert(p_, 0xebf2) if p_ else None
        if c_ in (30, 83) and o_ and o_[0] == 4 and E.wert(p_, 0xebfb) == 110:
            pfadtexte[o_[1]].append(((E.wert(p_, 0xec55) or (0.0, 0.0))[0], G.text_de(E.wert(p_, 0xec5f) or "")))
    nach_bmk = collections.defaultdict(list)
    for f_, e_ in F.items():
        if e_["cls"] == 17 and e_["props"] and E.wert(e_["props"], 60402):
            nach_bmk[funk.get(f_, {}).get("bmk", "")].append(f_)
    # Übersicht <-> Schaltplan gehören zusammen bei gleichem BMK, gleicher Funktionsdefinition, gleicher
    # Steckerbezeichnung (eb22) und gleicher Anschlussbezeichnung (Hilfe plcgui_k_arbeitsweise)
    def pin_schluessel(f_):
        r_ = funk.get(f_, {})
        return (r_.get("bmk", ""), (r_.get("anschluesse") or "").split("|")[0].strip(),
                (r_.get("steckerbez_eb22") or "").strip(), E.wert(F[f_]["props"], 0xefc5))
    nach_pin = collections.defaultdict(list)           # (BMK, erster Anschluss, Stecker, Fkt.-Def.) -> Funktionen
    for f_, e_ in F.items():
        if e_["props"] and E.wert(e_["props"], 60402):
            nach_pin[pin_schluessel(f_)].append(f_)

    def gegenstuecke(fid):
        """Alle Gegenstücke (EsPlcPairXRefEvaluator::getCounterparts liest die Relation 20305 der Funktion; hier wie
        EPLAN sie bildet: gleiches BMK, Anschluss, Stecker, Funktionsdefinition), andere Seite, mit Symbol zuerst."""
        k_ = pin_schluessel(fid) if fid in F and F[fid]["props"] else ("", "", "", None)
        if not k_[1]:
            return []
        kand = [f for f in nach_pin.get(k_, []) if f != fid and fn_seite.get(f) != fn_seite.get(fid)]
        kand.sort(key=lambda f: E.wert(F[f]["props"], 0xef4e) is None)
        return kand

    def gegenstueck(fid):
        """Dieselbe Funktion in anderer Darstellung (SPS-Übersicht <-> allpolig): gleiches BMK, Anschluss, Stecker
        und Funktionsdefinition, andere Seite; bevorzugt die Darstellung mit Symbol."""
        k_ = pin_schluessel(fid) if fid in F and F[fid]["props"] else ("", "", "", None)
        if not k_[1]:
            return None
        kand = [f for f in nach_pin.get(k_, []) if f != fid and fn_seite.get(f) != fn_seite.get(fid)]
        kand.sort(key=lambda f: E.wert(F[f]["props"], 0xef4e) is None)
        return kand[0] if kand else None

    def haupt_anderswo(fid, bmk):
        """Hauptfunktion (ohne eb16) gleichen BMKs auf einer anderen Seite (Querverweis eines Kastens)."""
        for f in nach_bmk.get(bmk, []):
            if f != fid and fn_seite.get(f) != fn_seite.get(fid) and E.wert(F[f]["props"], 0xeb16) is None                     and _pos(F[f]["props"]):
                return f
        return None

    def uebersicht(e):
        """Übersichtsdarstellung: ohne Symbol oder Übersichtssymbol (EPLAN-Name …OV, z. B. PLCCPOV)."""
        v_ = gr.variante(E.wert(e["props"], 0xef4d), E.wert(e["props"], 0xef4e), E.wert(e["props"], 0xef4f))
        return v_ is None or "OV" in gr.symbolname(v_[0], v_[1]).upper()

    einst = EST.lesen(arbeit)

    nb_cache = {}

    def pfad_spalte(pid_, x_, spalten=10, breite=420.0):
        """Pfad (Spalte) einer Lage wie die Spaltennummern des Normblatts (Seite 11016, sonst TrDMProject.Frame):
        Spaltenbreiten aus Form.eod (X 21509, Ursprung eb67), Anfangswert 12025, Stellen 12029 – dieselbe Rechnung
        wie die Spaltenleiste (nb_wert, Platzhalter 19005), eine Lage genau auf der Grenze gehört zur rechten Spalte
        (Original Projekt A =BD/14: Ziel x = 168 -> „/46/4“). Ohne Normblattdaten: 10 × 42 mm."""
        nbn = (_kv(PG[pid_]["props"], 60011).get("11016") if pid_ in PG else None) or einst.get("TrDMProject.Frame", "")
        if nbn not in nb_cache:
            nb_cache[nbn] = gr.normblatt(nbn) if nbn else None
        nb = nb_cache[nbn]
        if not nb or len(nb["spalten"][1]) < 2:
            return str(min(spalten - 1, max(0, int(x_ // (breite / spalten)))))
        x0, br = nb["spalten"]
        x, i = x0, 0
        while i < len(br) - 1 and x_ >= x + br[i]:
            x += br[i]
            i += 1
        return str(int(nb["kv"].get(12025) or 0) + i).zfill(int(nb["kv"].get(12029) or 1))

    def querverweis(f, von_pid, spalten=10, breite=420.0):
        """EPLAN-Querverweis wie EsXRefPlacement::getStandardXRefAsString (dekompiliert): Seitenname im Kontext
        (=Anlage+Ort nur, wenn abweichend) + Format.SeparatorForPage + Seite, dann SeparatorBetweenPageAndPath
        + Pfad (Spalte); ShowInBrackets setzt Klammern. Trennzeichen aus den Projekteinstellungen.
        Pfad = Spalte des Normblatts (pfad_spalte: Spaltenbreiten aus Form.eod, wie die Spaltenleiste)."""
        pid_ = fn_seite.get(f)
        if pid_ not in alle_seiten or not _pos(F[f]["props"]):
            return ""
        anl, ort, nr = alle_seiten[pid_]
        a0, o0, _ = alle_seiten.get(von_pid, ("", "", ""))
        sp = pfad_spalte(pid_, _pos(F[f]["props"])[0], spalten, breite)
        t_seite = einst.get("EsCrossReference.Format.SeparatorForPage", "/")
        t_pfad = einst.get("EsCrossReference.Format.SeparatorBetweenPageAndPath", "/")
        qv = f"{'=' + anl if anl != a0 else ''}{'+' + ort if ort != o0 else ''}{t_seite}{nr}{t_pfad}{sp}"
        return f"({qv})" if einst.get("EsCrossReference.Format.ShowInBrackets") == "1" else qv

    def pfadtext(f):
        pid_, p_ = fn_seite.get(f), _pos(F[f]["props"]) if f in F and F[f]["props"] else None
        if not p_ or not pfadtexte.get(pid_):
            return ""
        x_, t_ = min(pfadtexte[pid_], key=lambda xt: abs(xt[0] - p_[0]))
        return t_.replace("\n", " ") if abs(x_ - p_[0]) < 42 else ""

    artikel_daten = {}                                   # Artikelnummer (22001) -> {(Eigenschaft, Index): Wert}
    for _, e_ in P.get("Article.eod", {}).items():
        kv_ = [t for x in (e_.get("props") or []) if x[1] == "kv" for t in x[2]]
        d_ = {(t[0], t[1]): t[2] for t in kv_ if isinstance(t[2], str)}
        if d_.get((22001, 0)):
            artikel_daten[d_[(22001, 0)]] = d_

    gruppen_mit_grafik = {E.wert(p_, 0xebf1)[1] for _, (c_, p_) in gr.seite.items()
                          if p_ and E.wert(p_, 0xebf1) and E.wert(p_, 0xebf1)[0] == 26 and c_ not in (53, 26)}

    def hat_makrografik(pr):
        g_ = E.wert(pr, 0xebf1)
        return bool(g_ and g_[0] == 26 and g_[1] in gruppen_mit_grafik)

    def _texte(fid, e, p, b, lagen, an, roh, namen, klemme, spsan, werte, bmk, seite):
        werte_idx = {(k_, i_): w_ for k_, i_, w_ in (E.wert(e["props"], 0xea6b) or [])}
        # Eigenschaftstexte an der EPLAN-Lage (Anschlusstexte relativ zum Anschlusspunkt)
        beschr = [w for w in E.wert(e["props"], 0xeb1a, alle=True) if isinstance(w, str)]
        werte_k = {}
        # Querverweisanzeige 20021 (EPLAN FUNC_SUPPRESSCROSSREFERENCE): 0 automatisch (nur wenn das sichtbare BMK
        # nicht leer ist), 1 nie, 2 immer
        try:
            # 20021 ist eine Eigenschaft der Funktion selbst (nicht der Hauptfunktion): Archivdaten Projekt A/Projekt E/Projekt B –
            # Nebenfunktionen ohne eigenes 20021 zeigen den Verweis, auch wenn die Hauptfunktion 1 (nie) trägt
            qv_anzeige = int(werte_idx.get((20021, 0)) or 0)
        except ValueError:
            qv_anzeige = 0
        for (eig, idx), (tx, ty, h, ang, ausr, am, auf) in lagen.items():
            if eig == 20028 and not klemme and 1 <= idx <= len(roh):
                w = roh[idx - 1]
            elif eig == 20029 and 1 <= idx <= len(beschr):
                w = beschr[idx - 1]                      # EPLAN druckt einen führenden „\“ mit (\Channel)
            elif eig == 20038:
                w = " ".join(namen)
            elif eig == 20039:
                w = " ".join(x for x in beschr if x)
            elif eig == 20435:
                w = " ".join(namen)
            elif eig == 20301:
                # Übersichtsquerverweis (FUNC_PLCPAIRCROSSREFERENCE): EESInterpreteru 0x1800752a0 ->
                # EsPlcPairXRefEvaluator (EEsCrossReferenceu 0x180058ff0/0x180059190) prüft weder 20021 noch das
                # sichtbare BMK; relevant nach EsCrossReference.Plc.ShowPairCrossreferenceOnCircuit (Schaltplan)
                # bzw. …OnPlcOverview (Übersicht); Text = alle Gegenstücke, je Zeile ein Verweis
                ueb_ = uebersicht(e)
                if einst.get("EsCrossReference.Plc.ShowPairCrossreferenceOn" + ("PlcOverview" if ueb_ else "Circuit"),
                             "1") == "0":
                    w = ""
                else:
                    w = "\n".join(x for x in (querverweis(g_, seite[1]) for g_ in gegenstuecke(fid)) if x)
            elif eig in (20300, 20306) and not qv_relevant(fid):
                w = ""                                   # eigene Querverweisanzeige (20021) bzw. sichtbares BMK leer
            elif eig == 20306:                                       # Querverweis nur zur Hauptfunktion
                g_ = haupt_anderswo(fid, bmk)
                w = querverweis(g_, seite[1]) if g_ and qv_relevant(g_) else ""
            elif eig == 20300:
                g_ = gegenstueck(fid) or (haupt_anderswo(fid, bmk) if fid in box_rect else None)
                if g_ or fid in box_rect:
                    w = querverweis(g_, seite[1]) if g_ and qv_relevant(g_) else ""
                else:
                    # EsStdFunctionXRefEvaluator::getLogicalCrossReferences (0x18005bfe0): Nebenfunktion -> ihre
                    # Hauptfunktion (getDocumentationMainElement), Hauptfunktion -> alle Nebenfunktionen gleichen
                    # Namens und gleicher Darstellungsart (getSupplementFunctions 0x18005a7b0), je Ziel
                    # isRelevantForCrossReferences; ein Verweis je Zeile (NoOfCrossReferencesInBlock = 1)
                    w = "\n".join(x for x in (querverweis(z_, seite[1]) for z_ in qv_ziele(fid, bmk)
                                               if qv_relevant(z_)) if x)
            elif eig in (20201, 20403, 20404) and spsan:
                g_ = fid if nachbarn.get((fid, 0)) else (gegenstueck(fid) or fid)
                a0, o0, _ = alle_seiten.get(seite[1], ("", "", ""))
                manuell_ = werte_idx.get((20402, 0)) if eig == 20404 else None   # automatisch = manuell, sonst ermittelt
                w = sps_ziel(g_, 0) if eig in (20403, 20404) and not manuell_ else None
                if manuell_:
                    w = manuell_
                elif w is None:
                    w = ziel(g_, 0, a0, o0)
                    if eig == 20404:                   # symbolische Adresse: Gerät ohne Anschluss/Ort
                        w = re.sub(r":.*$", "", w)
                        w = re.sub(r"\+" + re.escape(o0) + r"(?=-)", "", w) if o0 else w
                else:
                    stat["sps_symb_ermittelt"] += 1
                    w = re.sub(r"^=" + re.escape(a0) + r"(?=[+\-])", "", w) if a0 else w
                    w = re.sub(r"\+" + re.escape(o0) + r"(?=-)", "", w) if o0 else w
            elif eig == 20031 and spsan:
                # Funktionstext (automatisch): eigener Funktionstext bzw. der des Gegenstücks, sonst Pfad-Funktionstext
                g_ = gegenstueck(fid)
                w = G.text_de(werte.get(20011) or "")
                if not w and g_:
                    w = G.text_de({k_: w_ for k_, i_, w_ in (E.wert(F[g_]["props"], 0xea6b) or [])}.get(20011) or "")
                w = (w or pfadtext(g_ or fid)).replace("\n", " ")
            elif eig == 20901:                                       # Zusatzfeld: genau dieser Index
                w = G.text_de(werte_idx.get((eig, idx)) or je_bmk[bmk].get((eig, idx)) or "")
            elif 22000 <= eig < 23000:
                # Artikeleigenschaft: idx = Artikel * 1000 + Eigenschaftsindex (eplan_grafik._index); Nebenfunktionen
                # zeigen die Artikel ihrer Hauptfunktion (gleiches BMK)
                arts_ = artikel.get(fid) or artikel.get(je_bmk_fid.get(bmk), [])
                k_, ei_ = (idx // 1000 - 1, idx % 1000) if idx >= 1000 else (max(0, idx - 1), 0)
                w = ""
                # Artikelnummer genau dieses Artikels: Funktion 20100[Index] (bzw. der Hauptfunktion), sonst Stückliste
                nr_ = werte_idx.get((20100, k_ + 1)) or je_bmk[bmk].get((20100, k_ + 1)) or ""
                if nr_ and nr_ in artikel_daten:
                    st_ = artikel_daten[nr_]
                    w = st_.get((eig, ei_)) or (st_.get((eig, 0)) if ei_ <= 1 else "") or ""
                elif 0 <= k_ < len(arts_):
                    st_ = artikel_daten.get(arts_[k_].get("Artikelnummer", ""), {})
                    w = st_.get((eig, ei_)) or (st_.get((eig, 0)) if ei_ <= 1 else "") or                         arts_[k_].get(ARTIKEL_SPALTE.get(eig, ""), "")
                w = G.text_de(w) if w and "@" in w else (w or "")
                if w in ("0", "0.0"):
                    w = ""
            elif eig == 20005:                           # BMK (identifizierend) im Strukturformat des Projekts
                w = bmk_ident(bmk, E.wert(e["props"], 0xefc5))
            elif eig == 20073:                           # Kabel: Typ, Aderanzahl x Querschnitt mit Einheit
                n_, q_ = werte.get(20042) or "", werte.get(20043) or ""
                ein_ = {"2": "sqmm", "3": "AWG", "4": "mm", "5": "KCM", "6": "MCM", "7": "Zoll", "8": '"',
                        "9": "inch", "10": "µm", "11": "kcmil", "12": "µm²"}.get(str(werte.get(20068) or "0"), "mm²")
                w = " ".join(t for t in (werte.get(20040) or "", f"{n_}x{q_} {ein_}" if n_ and q_ else "") if t)
            elif eig == 20006:                           # BMK (vollständig)
                w = bmk
            elif eig == 20053:                           # Kabel: Aderanzahl x Querschnitt („8x0,34“)
                n_, q_ = werte.get(20042) or "", werte.get(20043) or ""
                w = f"{n_}x{q_}" if n_ and q_ else (n_ or q_)
            elif eig == 20211:                           # BMK übergeordnet, ohne Projektstrukturen, mit Vorzeichen
                w = bmk[bmk.index("-"):] if "-" in bmk else ""
            elif eig == 20010 and klemme:
                # Klemme: sichtbares BMK, wo 20010 sichtbar platziert ist. Ohne von EPLAN gespeichertes sichtbares BMK
                # (60145) UND ohne eigene Platzierung übernimmt die Klemme das BMK von links (Hilfe) und zeigt keins
                # (Phoenix: nur Klemme 1 von -XD1 trägt 60145, die Folgeklemmen nicht)
                pr_k = F[fid]["props"] if fid in F else None
                ohne_fmt = not (pr_k and isinstance(E.wert(pr_k, 60145), str))
                eigene_k = (20010, 0) in G.textlagen(pr_k) if pr_k else False
                w = "" if (ohne_fmt and not eigene_k and KLEMMEN_BMK_VON_LINKS) else sichtbar(bmk, seite[1], p, fid)
            elif eig == 20010 and werte.get(20010):
                w = werte[20010]
            elif eig == 20010 and spsan:
                a0, o0, _ = alle_seiten.get(seite[1], ("", "", ""))
                w = re.sub(r"^=" + re.escape(a0) + r"(?=[+\-])", "", bmk) if a0 else bmk
                w = re.sub(r"\+" + re.escape(o0) + r"(?=-)", "", w) if o0 else w
            elif getattr(lagen, "relation", {}).get((eig, idx)) == 20064 and (eig, idx) in je_bmk[bmk]:
                w = G.text_de(je_bmk[bmk][(eig, idx)])         # Wert der Hauptfunktion (RelationType MainFunction)
            elif (eig, idx) in werte_idx or eig in werte:      # jede platzierte Eigenschaft mit Wert (z. B. 20406)
                w = G.text_de(werte_idx.get((eig, idx)) or werte.get(eig) or "")
            elif fid in box_rect and eig in ZEIGEN and (eig, 0) in je_bmk[bmk]:   # Kasten: Wert der Hauptfunktion
                w = G.text_de(je_bmk[bmk][(eig, 0)])
            else:
                w = ""
            # Ausrichtungsbox mit fester Breite (ee25 & 0x70 = 0x10): EPLAN bricht den Text um (GedFTextBreaker)
            if w and (getattr(lagen, "boxart", {}).get((eig, idx), 0) & 0x70) == 0x10 and \
                    getattr(lagen, "boxbreite", {}).get((eig, idx)):
                w = G.umbrechen(w, lagen.boxbreite[(eig, idx)], lagen[(eig, idx)][2])
            werte_k[(eig, idx)] = w or ""
        lage_k = {}
        # Das BMK (20010) zeichnet QET als Bauteilbeschriftung, nicht als Eigenschaftstext; für das Andocken zählt es
        # trotzdem als belegter Bezugstext (sonst landet der nächste Text der Kette genau auf dem BMK)
        stapel_w = dict(werte_k)
        if (20010, 0) in lagen and not stapel_w.get((20010, 0), "").strip() and not klemme and not spsan \
                and e["cls"] != 59:
            stapel_w[(20010, 0)] = sichtbar_gespeichert(bmk, seite[1], p, fid) or ""   # leeres BMK belegt keine Zeile

        def lage_von(key, tiefe=0):
            # gestapelte Texte: eine Zeile über dem Bezugstext, wenn dieser einen Wert hat
            if key in lage_k:
                return lage_k[key]
            tx, ty, h, ang, ausr, am, auf = lagen[key]
            if auf in lagen and tiefe < 12:
                bx, by = lage_von(auf, tiefe + 1)
                if getattr(auf, "richtung", 0) & 0x60:
                    # Waagerecht andocken (DockingStyle 32 rechts / 64 links, XGedBaseCalcDockPosition::
                    # positionTextBlock, Zweig (Stil & 0x60) != 0; setNew-/getNextPositionForDockingHorizontal):
                    # laufende Kante = Kante des Bezugstexts (Breite aus der Ausrichtungsspalte), weiter um
                    # Breite + 0,3 × Texthöhe des STARTtexts (this+0x68); leerer Text: Breite 0, kein Abstand.
                    # Der angedockte Text liegt mit seiner Kante an der laufenden Kante, gleiche Ankerhöhe.
                    def _breite(k_):
                        z_ = [z for z in stapel_w.get(k_, "").split("\n") if z.strip()]
                        return max((G.breite_px(z, lagen[k_][2] * G.TAHOMA_EM) for z in z_), default=0.0)
                    wurzel_ = auf
                    for _ in range(12):
                        if lagen.get(wurzel_) and lagen[wurzel_][6] in lagen:
                            wurzel_ = lagen[wurzel_][6]
                    sk_ = MASSSTAB.get(seite[1], 1.0) or 1.0                                   # Papier -> Modellmaß
                    anteil_ = (0.0, 0.5, 1.0)
                    wa_, wb_ = _breite(auf) * sk_, _breite(key) * sk_
                    links_a = bx - wa_ * anteil_[((lagen[auf][4] or 7) - 1) % 3]
                    abstand_ = 0.3 * lagen[wurzel_][2] * sk_ if wa_ > 1e-5 else 0.0
                    if getattr(auf, "richtung", 0) & 0x20:
                        links_b = links_a + wa_ + abstand_
                    else:
                        links_b = links_a - abstand_ - wb_
                    # längs der Schreibrichtung des Starttexts (positionTextBlock: Winkel aus dessen TextInfo +0x18)
                    d_ = links_b + wb_ * anteil_[((ausr or 7) - 1) % 3] - bx
                    w_ = lagen[wurzel_][3] or 0.0
                    lage_k[key] = (bx + d_ * math.cos(w_), by + d_ * math.sin(w_))
                    return lage_k[key]
                if stapel_w.get(auf, "").strip():
                    # Andockrichtung aus EPLAN (DockingStyle); ohne Angabe: nach oben, bei oben ausgerichtetem
                    # Bezugstext nach unten
                    r_ = getattr(auf, "richtung", 0)
                    # Richtung der Kette (Starttext): 16 darüber, 8 darunter
                    nach_unten = r_ == 8 or (not r_ and (lagen[auf][4] or 7) in (1, 2, 3))
                    # Abstand wie EPLAN (XGedBaseCalcDockPosition::getNextPositionForDockingVertical): Zeilenhöhe
                    # L = Zellhöhe der Schrift = 1,5 × Texthöhe (ETextGraphicsTools::setFont) je Zeile; der Anteil
                    # über/unter dem Anker hängt von der Ausrichtung ab (gestapelte Texte richten sich nach dem Bezug)
                    a_ = lagen[auf][4] or 7
                    zl_a = max(1, stapel_w.get(auf, "").count("\n") + 1)
                    zl_b = max(1, stapel_w.get(key, "").count("\n") + 1)
                    ueber_a, unter_a = G.zeilenanteile(a_, lagen[auf][2] * G.ZELLE * zl_a)
                    ueber_b, unter_b = G.zeilenanteile(a_, h * G.ZELLE * zl_b)
                    zeile_ = (unter_a + ueber_b if nach_unten else ueber_a + unter_b) * \
                        (MASSSTAB.get(seite[1], 1.0) or 1.0)                                  # Papier -> Modellmaß
                    # Abzug längs (-sin, cos) des Winkels des Starttexts (positionTextBlock 0x180267670 setzt
                    # this+0x28/0x30 = (-sin, cos) aus TextInfo +0x18 des ersten Blocktexts)
                    wurzel_ = auf
                    for _ in range(12):
                        if lagen.get(wurzel_) and lagen[wurzel_][6] in lagen:
                            wurzel_ = lagen[wurzel_][6]
                    w_ = lagen[wurzel_][3] or 0.0
                    zeile_ = -zeile_ if nach_unten else zeile_
                    bx, by = bx - zeile_ * math.sin(w_), by + zeile_ * math.cos(w_)
                lage_k[key] = (bx, by)
            else:
                if am and 1 <= key[1] <= len(an):
                    tx, ty = tx + an[key[1] - 1][0], ty + an[key[1] - 1][1]
                lage_k[key] = (tx, ty)
            return lage_k[key]
        gesetzt = set()
        sk = MASSSTAB.get(seite[1], 1.0) or 1.0                 # maßstäbliche Seite: Modellmaße -> Papier
        for key, w in werte_k.items():
            if w.strip():
                tx, ty = lage_von(key)
                tx, ty = tx / sk, ty / sk
                kenn = (w.strip(), round(p[0] + tx), round(p[1] + ty))
                if kenn in gesetzt:
                    continue
                gesetzt.add(kenn)
                h, ausr = lagen[key][2], lagen[key][4]
                if lagen[key][6] in lagen:                       # gestapelt: Ausrichtung des Bezugs
                    ausr = lagen[lagen[key][6]][4] or ausr
                # Lage in Modellmaß (/ Maßstab), Schrifthöhe von Eigenschaftstexten in Papiermaß (am Original: 1,5 mm)
                if G.PROTOKOLL is not None:
                    G.KONTEXT = {"art": "eig", "folio": b.folio, "fid": fid, "cls": e["cls"],
                                 "fd": E.wert(e["props"], 0xefc5), "var": E.wert(e["props"], 0xef4f), "eig": list(key),
                                 "ang_eplan": lagen[key][3], "auf": list(lagen[key][6]) if lagen[key][6] in lagen
                                 else None, "richtung": getattr(lagen[key][6], "richtung", 0),
                                 "box": getattr(lagen, "boxbreite", {}).get(key), "kasten": fid in box_rect,
                                 "im_kasten": fid in im_kasten, "sk": sk, "am": lagen[key][5]}
                # Textwinkel ee22 (DmgTextInfo +0x18, rad gegen den Uhrzeigersinn) wie bei freien Texten
                texte_je_blatt[b.folio].append(G.text_auf_blatt(w, (p[0] + tx, p[1] + ty), h, lagen[key][3] or 0.0,
                                                                 ausr))
                if DIAGNOSE and any(d_ in w for d_ in DIAGNOSE):
                    print(f"DIAG {w!r} fid {fid} eig {key} anker ({p[0] + tx:.1f},{p[1] + ty:.1f}) h {h} "
                          f"ausr {ausr} lage {lagen[key][:2]} auf {lagen[key][6]}")
        # Lage des BMK (20010) nach Andocken/Winkel: QET zeichnet es als Bauteilbeschriftung (Aufrufer)
        return lage_von((20010, 0)) if (20010, 0) in lagen else None

    pins = {}                              # (objekt_id, anschlussindex) -> (Teil, Anschluss)
    teile = {}                             # Funktion -> gesetztes Teil (für Master/Slave)
    fn_seite = {fid: E.wert(e["props"], 60402)[1] for fid, e in F.items()
                if e["props"] and E.wert(e["props"], 60402)}

    def roh_von(fid):
        return [n.strip() for n in (funk.get(fid, {}).get("anschluesse") or "").split("|")]

    _fortschritt(55, "kaesten")
    # Black Boxes: Gerätekasten (Funktionsdefinition 1303) bzw. SPS-Karte (301) samt allem, was in seiner
    # EPLAN-Makrogruppe steckt (Geräteanschlüsse, Kontakte, Grafik) -> EIN QET-Bauteil (Master)
    im_kasten, kasten_teil, kasten_gruppen = {}, {}, collections.defaultdict(set)
    makros = collections.defaultdict(list)            # (seite, Makrogruppe) -> Kästen
    for bfid, (x0, y0, x1, y1, pid, ex_i) in box_rect.items():
        e = F[bfid]
        ref = E.wert(e["props"], 0xebf1)
        if e["cls"] == 17 and E.wert(e["props"], 0xefc5) in (1303, 301) and ref and ref[0] == 26 and pid in blatt:
            makros[(pid, ref[1])].append(bfid)
    for (pid, g), kaesten_m in makros.items():
        kaesten_m.sort(key=lambda f: (box_rect[f][0], -box_rect[f][3]))
        bfid = kaesten_m[0]                            # Hauptkasten: links oben
        pr = F[bfid]["props"]
        p0 = _pos(pr)
        x0, y0, x1, y1 = box_rect[bfid][:4]
        mitglieder = [f for f, e2 in F.items() if f not in kaesten_m and fn_seite.get(f) == pid and e2["props"]
                      and _pos(e2["props"]) and g in gr.gruppen_kette(E.wert(e2["props"], 0xebf1))]
        rel = (-p0[0], -p0[1])
        formen = []
        for kf in kaesten_m:
            a0, b0, a1, b1 = box_rect[kf][:4]
            formen.append(("P", [(a0 + rel[0], b0 + rel[1]), (a1 + rel[0], b0 + rel[1]), (a1 + rel[0], b1 + rel[1]),
                                 (a0 + rel[0], b1 + rel[1]), (a0 + rel[0], b0 + rel[1])], True, False, False))
            pk = F[kf]["props"]
            vb = gr.variante(E.wert(pk, 0xef4d), E.wert(pk, 0xef4e), E.wert(pk, 0xef4f))
            if vb:
                dk = (_pos(pk)[0] + rel[0], _pos(pk)[1] + rel[1])
                formen += [_verschoben(f, dk) for f in gr.formen(vb[0], vb[2])]
            if kf != bfid:
                im_kasten[kf] = bfid
                extra[pid][box_rect[kf][5]] = None
        formen += [_verschoben(f, rel) for f in gr.seitenformen(pid, nur=g)]
        for i_, n_, v_, lage_ in gr.seitensymbole(pid, nur=g):
            vv = gr.variante(i_, n_, v_)
            if vv:
                formen += [_verschoben(f, (lage_[0] + rel[0], lage_[1] + rel[1])) for f in gr.formen(vv[0], vv[2])]
        bmk_b = funk.get(bfid, {}).get("bmk", "")
        ge_b = (re.match(r"(=[^+\-]*)?(\+[^\-]*)?(-.*)?$", bmk_b) or [None] * 4)[3] or ""
        anschl, namen_, zuord = [], [], {}
        for m in mitglieder:
            pm = _pos(F[m]["props"])
            d = (pm[0] + rel[0], pm[1] + rel[1])
            vm = gr.variante(E.wert(F[m]["props"], 0xef4d), E.wert(F[m]["props"], 0xef4e), E.wert(F[m]["props"], 0xef4f))
            pts = [(0.0, 0.0, 4)]
            if vm:
                formen += [_verschoben(f, d) for f in gr.formen(vm[0], vm[2])]
                pts = gr.anschluesse(vm[0], vm[2])
            rm = roh_von(m)
            ge_m = (re.match(r"(=[^+\-]*)?(\+[^\-]*)?(-.*)?$", funk.get(m, {}).get("bmk", "")) or [None] * 4)[3] or ""
            sub = ge_m[len(ge_b) + 1:] if ge_b and ge_m.startswith(ge_b + "-") else ""
            for i, (ax, ay, r_) in enumerate(pts):
                if i >= max(1, len(rm)):
                    break
                nm = (f"{sub}:" if sub else "") + (rm[i] if i < len(rm) and rm[i] else str(i + 1))
                while nm in namen_:
                    nm += "'"
                anschl.append((d[0] + ax, d[1] + ay, r_))
                namen_.append(nm)
                zuord[(m, i)] = nm
            if len(pts) == 1:
                for i in range(1, len(rm)):
                    zuord[(m, i)] = zuord[(m, 0)]
            im_kasten[m] = bfid
        kurz_b = sichtbar(bmk_b, pid, p0, bfid)
        xml = G.mit_rolle(G.element_xml(f"EPLAN Black Box {ge_b or bmk_b} ({pid}-{g})", formen, anschl,
                                        f"Black Box (Gerätekasten mit Makroinhalt) aus EPLAN-Projektimport, {bmk_b}",
                                        namen=namen_), ("master", "commutator"))
        sig = hashlib.md5(xml.encode("utf-8")).hexdigest()[:8]
        haupt_b = BIB.hauptartikel(artikel.get(bfid, []))
        for m in kaesten_m + mitglieder:
            haupt_b = haupt_b or BIB.hauptartikel(artikel.get(m, []))
        if sammlung is not None and haupt_b:
            pfad_b = sammlung.schaltbild(xml, haupt_b.get("Hersteller (22007)", ""), haupt_b["Bestellnummer (22003)"],
                                         haupt_b.get("Bezeichnung 1 (22004)", ""), 1303, "BlackBox", sig, namen_)
        else:
            rel_p = f"{SYMBOLE}/BlackBox/{re.sub(r'[^A-Za-z0-9_+-]', '_', bmk_b)[:40]}_{sig}.elmt"
            ziel_b = os.path.join(S.COMPANY, *rel_p.split("/"))
            os.makedirs(os.path.dirname(ziel_b), exist_ok=True)
            if not os.path.exists(ziel_b):
                open(ziel_b, "w", encoding="utf-8").write(xml)
            pfad_b = S.U + rel_p
        infos_b = {"function_bmk": bmk_b}
        if haupt_b:
            infos_b.update({"manufacturer": BIB.HERSTELLER.get(haupt_b.get("Hersteller (22007)", ""),
                                                               haupt_b.get("Hersteller (22007)", "")),
                            "manufacturer_reference": haupt_b.get("Bestellnummer (22003)", ""),
                            "designation": haupt_b.get("Bezeichnung 1 (22004)", "")})
        lagen_b0 = _lagen(gr.textlagen(vb[0], vb[2], pr) if vb else {}, G.textlagen(pr))
        tb = blatt[pid].setze(pfad_b, *_px(p0), label="" if {(20010, 0), (20211, 0), (20005, 0), (20006, 0)} & lagen_b0.keys() else kurz_b, infos=infos_b,
                              label_xy=(round((x0 - p0[0]) * K) + 2, round(-(y1 - p0[1]) * K) - 14))
        for k_, nm in zuord.items():
            pins[k_] = (tb, nm)
        # Eigenschaftstexte des Kastens selbst (Kartentyp, Zusatzfelder, Artikel, Querverweis …)
        werte_b = {}
        for k_, i_, w_ in (E.wert(pr, 0xea6b) or []):
            werte_b.setdefault(k_, w_)
        lagen_b = _lagen(gr.textlagen(vb[0], vb[2], pr) if vb else {}, G.textlagen(pr))
        # Artikeltexte ohne Lage und ohne Bezug: unten im Kasten, Index 1 zuunterst
        art_keys = sorted(k_ for k_, v_ in lagen_b.items() if 22000 <= k_[0] < 23000 and v_[6] is None
                          and abs(v_[0]) < 1e-6 and abs(v_[1]) < 1e-6)
        for j_, k_ in enumerate(art_keys):
            x_, y_, h_, a_, au_, am_, auf_ = lagen_b[k_]
            lagen_b[k_] = ((x0 + x1) / 2 - p0[0], y0 - p0[1] + 1.0 + j_ * h_ * 1.35, h_, a_, 8, am_, None)
        if (20010, 0) in lagen_b:
            werte_b[20010] = kurz_b                          # BMK als Text am Stapelanfang (wie EPLAN)
        _texte(bfid, F[bfid], p0, blatt[pid], lagen_b, [], roh_von(bfid), [n for n in roh_von(bfid) if n],
               False, False, werte_b, bmk_b, (4, pid))
        kasten_teil[bfid] = tb
        teile[bfid] = tb
        kasten_gruppen[pid].add(g)
        extra[pid][box_rect[bfid][5]] = None
        stat["blackbox"] += 1

    # Aufgeteilte Geräte: Hauptfunktion (ohne eb16) bzw. Black Box = Master, Kontakte als Nebenfunktion
    # (eb16 = 0, Funktionsdefinition 201 Schließer / 202 Öffner / 203–204 Wechsler) = Slaves gleichen BMKs
    rolle_von, master_von = {}, {}
    gruppen_bmk = collections.defaultdict(list)
    for fid, e in F.items():
        pr = e["props"]
        if e["cls"] != 17 or not pr or fid in im_kasten or fn_seite.get(fid) not in blatt:
            continue
        if E.wert(pr, 0xefc5) in (100, 101, 300, 301, 400, 401, 8, 1302, 15):
            continue
        gruppen_bmk[funk.get(fid, {}).get("bmk", "")].append(fid)
    box_bmk = {funk.get(bf, {}).get("bmk", ""): bf for bf in kasten_teil}
    for bmk_, fs in gruppen_bmk.items():
        if not bmk_:
            continue
        kont = [f for f in fs if E.wert(F[f]["props"], 0xeb16) is not None
                and E.wert(F[f]["props"], 0xefc5) in (201, 202, 203, 204)]
        haupt_ = [f for f in fs if E.wert(F[f]["props"], 0xeb16) is None and f not in kont]
        if not kont or not (haupt_ or bmk_ in box_bmk):
            continue
        if bmk_ in box_bmk:
            master_von[bmk_] = box_bmk[bmk_]
        else:
            m_ = haupt_[0]
            pm_ = F[m_]["props"]
            vm_ = gr.variante(E.wert(pm_, 0xef4d), E.wert(pm_, 0xef4e), E.wert(pm_, 0xef4f))
            sn = gr.symbolname(vm_[0], vm_[1]) if vm_ else ""
            fdm = E.wert(pm_, 0xefc5) or 0
            art_ = "coil" if fdm == 200 or sn.startswith("K") else ("protection" if fdm // 100 == 5 else "commutator")
            rolle_von[m_] = ("master", art_)
            master_von[bmk_] = m_
        for k_ in kont:
            pk = F[k_]["props"]
            vk = gr.variante(E.wert(pk, 0xef4d), E.wert(pk, 0xef4e), E.wert(pk, 0xef4f))
            sn = gr.symbolname(vk[0], vk[1]) if vk else ""
            zust = {201: "NO", 202: "NC"}.get(E.wert(pk, 0xefc5), "SW")
            rolle_von[k_] = ("slave", zust, "power" if sn[:2] in ("SL", "OL") else "simple")
    bmk_im_kasten = set()
    # links oben zuerst, damit das BMK eines Geräts im Kasten an der ersten Funktion steht
    _nf, _if = max(1, len(F)), 0
    for fid, e in sorted(F.items(), key=lambda fe: ((_pos(fe[1]["props"]) or (0, 0))[0] if fe[1]["props"] else 0,
                                                    -((_pos(fe[1]["props"]) or (0, 0))[1] if fe[1]["props"] else 0))):
        _if += 1
        if _if % 250 == 0:
            _fortschritt(60 + 20 * _if // _nf, f"setzen {_if} {_nf}")
        seite = _wert(e["props"], 60402)
        p = _pos(e["props"])
        if not seite or seite[1] not in blatt or not im_blatt(p) or fid in kasten_teil:
            continue                                   # Kästen sind schon als Black-Box-Bauteil gesetzt
        if (panel_rolle(fid, e["props"]) or {}).get("role") in ("surface", "enclosure"):
            continue                                   # Montagefläche/Gehäuse sind Formen des Panelmodells
        b = blatt[seite[1]]
        r = funk.get(fid, {})
        bmk = r.get("bmk", "")
        roh = [n.strip() for n in (r.get("anschluesse") or "").split("|")]
        namen = [n for n in roh if n]
        x, y = _px(p)
        werte = {}
        for k, i, w in (E.wert(e["props"], 0xea6b) or []):
            werte.setdefault(k, w)
        # Klemmen/Steckerstifte zeigen die Klemmennummer, das BMK steht an der Leisten-/Steckerdefinition;
        # Adern zeigen Farbe/Nummer, das Kabel-BMK steht an der Kabeldefinition
        klemme = bool(r.get("20030 Klemmen-/Anschlussbezeichnung")) or 20030 in werte
        # SPS-Anschlüsse (Funktionsdefinition 300) tragen kein BMK, es steht am SPS-Kasten
        spsan = E.wert(e["props"], 0xefc5) == 300
        # Sichtbares BMK nur, wenn EPLAN es gespeichert hat (60145/60144): ohne sichtbares BMK übernimmt die Funktion
        # das BMK von links bzw. vom Kasten und zeigt keins (Hilfe devicetaggui_k_bmkuebernahme); Archivdaten Projekt A/Projekt E/Projekt B:
        # 0 von 131 BMK-Platzierungen ohne 60145 stehen im Original (Nachbarpole, Slaves, -S331 Projekt E DC/33)
        kurz = "" if klemme or spsan or e["cls"] == 59 else sichtbar_gespeichert(bmk, seite[1], p, fid)
        art = artikel.get(fid, [])
        infos = {"function_bmk": bmk}
        if art:
            a = art[0]
            infos.update({"manufacturer": BIB.HERSTELLER.get(a.get("Hersteller (22007)", ""),
                                                             a.get("Hersteller (22007)", "")),
                          "manufacturer_reference": a.get("Bestellnummer (22003)", ""),
                          "designation": a.get("Bezeichnung 1 (22004)", "")})
        if E.wert(e["props"], 0xefc5) == 8:
            # Artikelplatzierung im Schrankaufbau: Umriss eb1e/eb1f (Modellmaß, Einfügepunkt = linke obere Ecke),
            # Tiefe 22014 des Artikels, Drehwinkel 20451 -> Infos des Fork-Panelmodells
            a8, b8 = E.wert(e["props"], 0xeb1e), E.wert(e["props"], 0xeb1f)
            if (isinstance(a8, list) and isinstance(b8, list) and len(a8) == 2 and len(b8) == 2
                    and all(isinstance(v_, (int, float)) and abs(v_) < 1e5 for v_ in a8 + b8)):
                if abs(b8[0] - a8[0]) > 0.1 and abs(b8[1] - a8[1]) > 0.1:
                    infos["width_mm"] = f"{abs(b8[0] - a8[0]):.6g}"
                    infos["height_mm"] = f"{abs(b8[1] - a8[1]):.6g}"
            nr8, a8k = artikel_von(e["props"])
            if a8k.get(22014) and str(a8k.get(22014)) not in ("0", "0.0"):
                infos["depth_mm"] = str(a8k[22014]).replace(",", ".")
            if nr8 and "manufacturer_reference" not in infos:
                infos["manufacturer_reference"] = a8k.get(22003) or nr8
        v = gr.variante(E.wert(e["props"], 0xef4d), E.wert(e["props"], 0xef4e), E.wert(e["props"], 0xef4f))
        var = E.wert(e["props"], 0xef4f) or 0
        if fid in im_kasten:                                     # steckt im Black-Box-Bauteil
            if True:
                an_k = gr.anschluesse(v[0], v[2]) if v else []
                lagen_k = _lagen(gr.textlagen(v[0], v[2], e["props"]) if v else {}, G.textlagen(e["props"]))
                _texte(fid, e, p, b, lagen_k, an_k, roh, namen, klemme, spsan, werte, bmk, seite)
                lx = lagen_k.get((20010, 0))
                schon_k = (im_kasten[fid], kurz)
                if kurz and lx and fid not in box_rect and schon_k not in bmk_im_kasten:
                    bmk_im_kasten.add(schon_k)
                    G.KONTEXT = {"art": "bmk_kasten", "folio": b.folio, "fid": fid}
                    texte_je_blatt[b.folio].append(G.text_auf_blatt(kurz, (p[0] + lx[0], p[1] + lx[1]), lx[2], 0.0,
                                                                     lx[4]))
            stat["im_kasten"] += 1
            continue
        sym = _symbolpfad(gr, *v[:3], var, cache, rolle_von.get(fid)) if v else None
        anschl_namen, quelle = None, "eplan"
        fd = E.wert(e["props"], 0xefc5)
        # Sondersymbole (Geräteanschluss, Black Box, Kabel …) sind kein Schaltbild eines Bauteils
        echt = bool(v) and v[0] != gr.zuordnung.get(0) and any(math.hypot(q[0], q[1]) > 0 for q in (sym or [None, []])[1])
        haupt = BIB.hauptartikel(art) if art else None
        if sym and echt and sammlung is not None and haupt and fd != 400:
            treffer = None
            for a_ in (art if var < 4 else []):                  # irgendein Artikel der Funktion in der Sammlung?
                if a_.get("Bestellnummer (22003)"):
                    treffer = sammlung.waehle(a_["Bestellnummer (22003)"], roh, klemme)
                    # Bauteil ohne alle platziert verbundenen EPLAN-Anschlüsse (z. B. Klemme X4_B: 6 Punkte inkl.
                    # Brückenpunkte links/rechts, Sammlungsklemme nur oben/unten) -> Schaltbild aus dem Archiv
                    if treffer and not platziert.get(fid, set()) <= set(treffer[1]):
                        treffer = None
                    if treffer:
                        break
            a = haupt
            best = a["Bestellnummer (22003)"]
            if treffer:
                # Bauteil der Sammlung: gedreht wie die EPLAN-Variante, ein Anschluss auf seinem EPLAN-Punkt
                lpfad, zuord = treffer
                k = min(zuord)
                an_e = sym[1]
                drehung = (360 - 90 * var) % 360
                ax, ay = _px((p[0] + an_e[k][0], p[1] + an_e[k][1])) if k < len(an_e) else (x, y)
                lagen_l = _lagen(gr.textlagen(v[0], v[2], e["props"]), G.textlagen(e["props"]))
                t = b.setze(lpfad, ax, ay, anker=zuord[k], label=kurz if (20010, 0) in lagen_l else "", infos=infos,
                            drehung=drehung)
                teile[fid] = t
                for i, nm in zuord.items():
                    pins[(fid, i)] = (t, nm)
                # Eigenschaftstexte an der EPLAN-Lage wie bei EPLAN-Symbolen (Klemmennummer, BMK, Artikel …)
                _texte(fid, e, p, b, lagen_l, an_e, roh, namen, klemme, spsan, werte, bmk, seite)
                stat["bibliothek"] += 1
                continue
            # Schaltbild aus dem Archiv als Bauteil in die Sammlung (Anschlussnamen = EPLAN-Anschlüsse)
            an_e = sym[1]
            anschl_namen = []
            for i in range(len(an_e)):
                nm = roh[i] if i < len(roh) and roh[i] and roh[i] not in anschl_namen else str(i + 1)
                anschl_namen.append(nm)
            sname = gr.symbolname(v[0], v[1])
            xml = G.mit_rolle(G.element_xml(f"{best} {sname} {VARIANTE[var % 16]} {'/'.join(anschl_namen)}", gr.formen(v[0], v[2]), an_e,
                                            f"Schaltbild aus EPLAN-Projektimport ({v[0]}, Symbol {sname}, Variante "
                                            f"{VARIANTE[var % 16]}), Artikel {best}", namen=anschl_namen),
                              rolle_von.get(fid))
            lpfad = sammlung.schaltbild(xml, a.get("Hersteller (22007)", ""), best, a.get("Bezeichnung 1 (22004)", ""),
                                        fd, sname, VARIANTE[var % 16], roh)
            sym = (lpfad, an_e)
            quelle = "archiv"
        if sym:
            pfad, an = sym
            lagen = _lagen(gr.textlagen(v[0], v[2], e["props"]), G.textlagen(e["props"]))
            lx = lagen.get((20010, 0))
            label_xy = label_pt = None
            # Eigenschaftstexte zuerst: liefert die Lage des BMK nach Andocken (angedocktes BMK steht nicht an der
            # gespeicherten Lage, ec57 Bit 2)
            bl_ = _texte(fid, e, p, b, lagen, an, roh, namen, klemme, spsan, werte, bmk, seite)
            if lx:
                ausr_l = (lagen[lx[6]][4] or lx[4]) if lx[6] in lagen else lx[4]   # angedockt: Ausrichtung des Bezugs
                lx = (*(bl_ or lx[:2]), lx[2], lx[3], ausr_l, *lx[5:])
                # Kennzeichen = QGraphicsTextItem: Lage = linke obere Ecke inkl. 4 px Rand; gedreht (ee22) um diese
                # Ecke wie freie Texte (text_auf_blatt)
                label_pt, px_, dx, dy = G._text_box(kurz or "X", lx[2], lx[4])
                ox_, oy_ = dx - G.QET_RAND, dy - G.QET_RAND - px_
                c_, s_ = math.cos(-(lx[3] or 0.0)), math.sin(-(lx[3] or 0.0))
                label_xy = (round(lx[0] * G.K + ox_ * c_ - oy_ * s_, 2),
                            round(-lx[1] * G.K + ox_ * s_ + oy_ * c_, 2))
            # BMK nur, wenn EPLAN es platziert hat (20010 in den Textlagen); Artikelplatzierungen im Schrankaufbau
            # tragen z. B. nur die Positionsnummer (20450)
            if lx and kurz and G.PROTOKOLL is not None:
                G.PROTOKOLL.append({"art": "bmk", "folio": b.folio, "fid": fid, "t": kurz, "x": p[0] + lx[0],
                                    "y": p[1] + lx[1], "h": lx[2], "ang": lx[3], "ausr": lx[4],
                                    "var": E.wert(e["props"], 0xef4f), "cls": e["cls"]})
            t = b.setze(pfad, x, y, label=kurz if lx else "", infos=infos,
                        label_xy=label_xy or (-6 * len(kurz) - 14, -18), label_pt=label_pt)
            if lx and kurz and lx[3]:
                b.plan.lagen.append({"op": "set_element_text", "folio": b.folio, "element": t.ref,
                                     "index": S.symbol(pfad).label_index, "property": "rotation",
                                     "value": G._f(-math.degrees(lx[3]) % 360)})
            teile[fid] = t
            nm_ = anschl_namen or [str(i + 1) for i in range(len(an))]
            for i in range(max(1, len(roh))):
                if i < len(an):
                    pins[(fid, i)] = (t, nm_[i])
                elif len(an) == 1:
                    pins[(fid, i)] = (t, nm_[0])              # alle Anschlüsse im selben Punkt (Ader, Schirm …)
            stat["archiv_in_sammlung" if quelle == "archiv" else "eplan_symbol"] += 1
            stat["symbol"] += 1
            continue
        stat["punkte"] += 1
        eigene_f = G.textlagen(e["props"])
        if eigene_f:
            # ohne Symbol, aber mit platzierten Eigenschaften (z. B. SPS-Übersicht: Adresse, Kanal, Querverweis,
            # Ziel, Funktionstext): Texte an der EPLAN-Lage, Anschluss als Punkt ohne Beschriftung
            _texte(fid, e, p, b, eigene_f, [], roh, namen, klemme, spsan, werte, bmk, seite)
            t = b.setze(PUNKT, x, y, anker=0, label="", infos=infos)
            for i in range(max(1, len(roh))):
                pins[(fid, i)] = (t, 0)
            continue
        if kurz:
            b.text(kurz, x + 4, y - 26, "daten")
        for i, n in enumerate(namen or [""]):
            t = b.setze(PUNKT, x + 20 * i, y, anker=0, label=n, infos=infos if i == 0 else {"function_bmk": bmk})
            pins[(fid, i)] = (t, 0)

    # Master/Slave verknüpfen (QET schreibt Querverweise und Kontaktspiegel selbst)
    for bmk_, mf in master_von.items():
        mt = teile.get(mf)
        if not mt or S.symbol(mt.pfad).link_type != "master":
            continue
        sl = [teile[k_] for k_ in gruppen_bmk[bmk_] if rolle_von.get(k_, ("",))[0] == "slave" and k_ in teile
              and S.symbol(teile[k_].pfad).link_type == "slave"]
        if sl:
            mt.blatt.verbinde(mt, *sl)
            stat["verknuepft"] += len(sl)
            # QET hängt an Master und Slaves eigene Querverweise (CrossRefItem bzw. Slave-Xref am Info-Text „label“,
            # DynamicElementTextItem::updateXref); EPLAN-Querverweise kommen als Eigenschaftstexte 20300/20301 bzw.
            # Kontaktspiegel. Kennzeichen daher als Benutzertext (Verknüpfung bleibt, QET zeichnet keinen Xref).
            if not os.environ.get("EPLAN_QET_XREF"):
                for t_ in [mt, *sl]:
                    _label_als_text(t_)
                    stat["qet_xref_aus"] += 1

    # EPLAN-Kontaktspiegel als Seitengrafik (QETs eigenen Spiegel dieser Master abschalten)
    _kontaktspiegel(P, F, funk, gr, einst, blatt, fn_seite, massstab, teile, extra, querverweis, uebersicht, PG,
                    stat)

    # Normblatt je Seite: 11016 der Seite, sonst Projekteinstellung TrDMProject.Frame
    normblaetter, nb_fehlt = {}, collections.Counter()
    folge = [s[0] for s in seiten]
    name_von = {s[0]: s[3] for s in seiten}

    def nb_wert(pid, eig, idx, quelle, lage, nb):
        kv_s = _kv(PG[pid]["props"], 60011) if pid in PG else {}
        st = _kv(PG[pid]["props"], 60049) if pid in PG else {}
        if quelle == 53 and eig == 19005:                       # Spaltennummer aus der Lage in der Spaltenleiste
            x0, br = nb["spalten"]
            x, i = x0, 0
            while i < len(br) - 1 and lage[0] > x + br[i]:
                x += br[i]
                i += 1
            start = int(nb["kv"].get(12025) or 0)              # ohne Wert 0 (Originale zeigen 0–9)
            return str(start + i).zfill(int(nb["kv"].get(12029) or 1))
        if quelle == 4:
            k = folge.index(pid) if pid in folge else -1
            if eig == 11000:
                return name_von.get(pid, "")                    # Seitenname ohne Strukturkennzeichen (Hilfe)
            if eig in (11040, 11041):
                j = k + (1 if eig == 11040 else -1)
                if not (0 <= j < len(folge) and k >= 0):
                    return ""
                # Struktur des Nachbarblatts nur, wenn sie abweicht (wie beim BMK; Projekt A-Original „=DA/28.1“ auf =DB)
                s_n = _kv(PG[folge[j]]["props"], 60049) if folge[j] in PG else {}
                vor = "".join(z + s_n[k_] for z, k_ in (("=", "1100"), ("+", "1200"))
                              if s_n.get(k_) and s_n.get(k_) != st.get(k_))
                return (vor + "/" if vor else "") + name_von.get(folge[j], "")
            if eig == 1140:
                return "=" + st["1100"] if st.get("1100") else ""
            if eig == 1240:
                return "+" + st["1200"] if st.get("1200") else ""
            if eig in (11021, 11024):
                return _datum(kv_s.get(str(eig)))
            return _de(kv_s.get(str(eig), ""))
        if quelle == 14:
            if eig in (10021, 10023, 10027, 10058):             # 10027 Tagesdatum: im Projekt gespeicherter Wert
                return _datum(pinfo.get(str(eig)))
            return _de(pinfo.get(str(eig), ""))
        if quelle == 21:                                        # Eigenschaft des Normblatts selbst (18041 Zusatzfeld)
            return _de(nb["kv_i"].get((eig, idx)) or "")
        nb_fehlt[(quelle, eig)] += 1
        return ""

    _fortschritt(82, "grafik")
    # freie Seitengrafik, Symbole ohne Funktion (Winkel, Abzweige …) und Verbindungsdefinitionspunkte
    for pid, b in blatt.items():
        nbn = _kv(PG[pid]["props"], 60011).get("11016") or einst.get("TrDMProject.Frame", "")
        if nbn not in normblaetter:
            normblaetter[nbn] = gr.normblatt(nbn)
        nb = normblaetter[nbn]
        if nb:
            fn = list(nb["formen"])
            for eig, idx, quelle, lage, h, ang, ausr in nb["platzhalter"]:
                w = nb_wert(pid, eig, idx, quelle, lage, nb)
                if w:
                    fn.append(("T", lage, w, h, ang, ausr))
            G.KONTEXT = {"art": "normblatt", "folio": b.folio}
            sh, inp = G.blatt_xml(fn)
            formen_je_blatt[b.folio] += sh
            texte_je_blatt[b.folio] += inp
            for x0, y0, x1, y1, datei in nb["bilder"]:
                bild = G.bild_xml(datei, x0, y0, x1, y1)
                if bild:
                    bilder_je_blatt[b.folio].append(bild)
        else:
            nb_fehlt[("Normblatt", nbn)] += 1
        ohne = kasten_gruppen.get(pid, set())
        formen = gr.seitenformen(pid, ohne=ohne) + [f for f in extra.get(pid, []) if f]
        formen += gr.masse(pid)                                     # Bemaßungen (Schrankaufbau)
        s_ = massstab.get(pid, 1.0)
        if s_ != 1.0:
            formen = [G.skaliert(f, s_) for f in formen]
        formen += gr.platzhaltertexte(pid)                          # Auswertungsinhalte, Deckblatt
        if formular.get(pid):
            formen += gr.formulargrafik(formular[pid])              # Tabellenlinien der Auswertungsseite
            fp_ = gr.formularplatzhalter(formular[pid])             # Kopf: Formularname, Bearbeiter, Datum …
            for eig, idx, quelle, lage, h, ang, ausr in (fp_[2] if fp_ else []):
                if quelle == 51:                                    # Eigenschaft des Formulars selbst
                    if eig == 18000:                                # FORMBASE_NAME = Name im Projekt (ede6)
                        w = fp_[0] or ""
                    elif eig in (14021, 14023):                     # Erstellungs-/Änderungsdatum (Zeitstempel)
                        w = _datum(fp_[1].get(eig))
                    else:
                        w = _de(fp_[1].get(eig) or "")
                else:
                    w = nb_wert(pid, eig, idx, quelle, lage, nb)
                if w:
                    formen.append(("T", lage, w, h, ang, ausr))
        for idx, nr, var, lage in gr.seitensymbole(pid, ohne=ohne):
            v = gr.variante(idx, nr, var)
            if v:
                formen += [_verschoben(f, (lage[0] / s_, lage[1] / s_)) for f in gr.formen(v[0], v[2])]
        G.KONTEXT = {"art": "seite", "folio": b.folio, "sk": s_}
        sh, inp = G.blatt_xml(formen)
        formen_je_blatt[b.folio] += sh
        texte_je_blatt[b.folio] += inp
        for x0, y0, x1, y1, datei in gr.seitenbilder(pid):
            bild = G.bild_xml(datei, x0 / s_, y0 / s_, x1 / s_, y1 / s_)
            if bild:
                bilder_je_blatt[b.folio].append(bild)
    for oid, (cls, p) in gr.seite.items():
        if cls != 130 or not p:
            continue
        seite = E.wert(p, 0xebf2)
        lage = None
        for o in E.objekte(p, 0xef11):
            lage = E.wert(o, 0xed5a) or lage
        if seite and seite[1] in blatt and lage and im_blatt(lage):
            t = blatt[seite[1]].setze(PUNKT, *_px(lage), anker=0, label="")
            for i in range(4):
                pins[(oid, i)] = (t, 0)

    _fortschritt(86, "unterbrechung")
    # Unterbrechungspunkte: Name = 20012+20013+20014 (z. B. „5L4+“); gleichnamige paarweise in Seitenfolge
    # (1↔2, 3↔4 …); Text wie EPLAN „Name / Querverweis“ (rechtsbündig: „Querverweis / Name“) an der Lage von 20010
    def ip_name(pr):
        t_ = {str(k_): w_ for k_, i_, w_ in (E.wert(pr, 0xeaf0) or [])}
        n_ = "".join(t_.get(k_, "") for k_ in ("20012", "20013", "20014"))
        return n_ or "".join(_kv(pr, 60144).get(k_, "") for k_ in ("20013", "20014")) or "?"

    def seitenfolge(pid_):
        a_, o_, nr_ = alle_seiten.get(pid_, ("", "", ""))
        return (int(nr_) if nr_.isdigit() else 10 ** 6, a_, o_)
    ip_alle = []
    for oid, e in IP.items():
        sei_, p_ = _wert(e["props"], 60402), _pos(e["props"])
        if sei_ and p_:
            ip_alle.append((oid, sei_[1], p_, ip_name(e["props"])))
    ip_kette = collections.defaultdict(list)
    for t_ in sorted(ip_alle, key=lambda t: (seitenfolge(t[1]), t[2][0], -t[2][1])):
        ip_kette[t_[3]].append(t_)
    ip_partner = {}
    for kette in ip_kette.values():
        for i in range(0, len(kette) - 1, 2):
            ip_partner[kette[i][0]], ip_partner[kette[i + 1][0]] = kette[i + 1], kette[i]

    def ort_verweis(pid_, x_, von_):
        anl, ort, nr = alle_seiten.get(pid_, ("", "", ""))
        a0, o0, _ = alle_seiten.get(von_, ("", "", ""))
        return f"{'=' + anl if anl != a0 else ''}{'+' + ort if ort != o0 else ''}/{nr}/{pfad_spalte(pid_, x_)}"
    erste = set()
    for oid, e in IP.items():
        seite, p = _wert(e["props"], 60402), _pos(e["props"])
        if not seite or seite[1] not in blatt or not im_blatt(p):
            continue
        b = blatt[seite[1]]
        n = ip_name(e["props"])
        x, y = _px(p)
        gehend = n in erste                 # erstes Vorkommen = Ziel des Folgenden (Kette über die Blätter)
        var_ip = (E.wert(e["props"], 0xef4f) or 0) % 4         # 0 rechts, 1 oben, 2 links, 3 unten
        senkrecht = var_ip in (1, 3)
        pfeil = (S.GEHEND_S if gehend else S.KOMMEND_S) if senkrecht else (S.GEHEND if gehend else S.KOMMEND)
        t = b.setze(pfeil, x, y, anker=0)
        (plan.quellen if gehend else plan.ziele).append((b.folio, n, t))
        erste.add(n)
        vip = gr.variante(E.wert(e["props"], 0xef4d), E.wert(e["props"], 0xef4e), E.wert(e["props"], 0xef4f))
        lg_ip = _lagen(gr.textlagen(vip[0], vip[2], e["props"]) if vip else {}, G.textlagen(e["props"]))
        l0 = lg_ip.get((20010, 0))
        if l0:
            # Text auf der offenen Seite des Pfeils (weg von der angeschlossenen Leitung), lesbar; Abstand wie im
            # Symbol. Leitung nach oben -> Text darunter, endet am Pfeil (rechtsbündig im gedrehten Text) usw.
            d_ = max(abs(l0[0]), abs(l0[1]), 2.0)
            pts_ = gr.anschluesse(vip[0], vip[2]) if vip else []
            benutzt = [i_ for i_ in range(len(pts_)) if nachbarn.get((oid, i_))] or [0]
            richt = pts_[benutzt[0]][2] if pts_ else (4 if var_ip == 3 else 1 if var_ip == 1 else 2 if var_ip == 0 else 8)
            ang_ip = math.pi / 2 if richt in (1, 4) else 0.0
            ox, oy, ausr_ip = {1: (0.0, -d_, 6), 4: (0.0, d_, 4), 2: (-d_, 0.0, 6), 8: (d_, 0.0, 4)}.get(richt, (l0[0], l0[1], l0[4] or 7))
        txt = n
        if oid in ip_partner:
            pa = ip_partner[oid]
            vw = ort_verweis(pa[1], pa[2][0], seite[1]).lstrip("/")    # an Pfeilen ohne führenden Schrägstrich
            txt = f"{vw} / {n}" if l0 and ausr_ip in (3, 6, 9, 12) else f"{n} / {vw}"
        if l0:
            G.KONTEXT = {"art": "ip", "folio": b.folio, "fid": oid}
            texte_je_blatt[b.folio].append(G.text_auf_blatt(txt, (p[0] + ox, p[1] + oy), l0[2], ang_ip, ausr_ip))
        else:
            b.text(txt, x + 6, y - 18, "daten")
        # Symbol BP (SPECIAL 8) hat zwei Anschlusspunkte im selben Punkt, entgegengesetzt gerichtet (Variant.eod
        # edbe/edbf + edbd); Connection.eod eb2c/eb2f nutzt beide (Index 1 z. B. Projekt A 127, Phoenix 95) -> beide auf
        # den einen Anschluss des QET-Pfeils
        for i_ in range(max(1, len(gr.anschluesse(vip[0], vip[2])) if vip else 1)):
            pins[(oid, i_)] = (t, 0)
    for oid, e in PO.items():
        seite, p = _wert(e["props"], 60402), _pos(e["props"])
        if not seite or seite[1] not in blatt or not im_blatt(p):
            continue
        b = blatt[seite[1]]
        x, y = _px(p)
        pins[(oid, 0)] = (b.setze(PUNKT, x, y, anker=0, label=_wert(e["props"], 61691) or ""), 0)

    _fortschritt(88, "verbindungen")
    # Verbindungen (nur innerhalb eines Blatts; blattübergreifend laufen sie über die Pfeile)
    gezeichnet = 0
    nicht = collections.Counter()          # Grund -> Anzahl nicht gezeichneter Verbindungen
    schon = set()
    for r in _csv(out, "Verbindungen_aufgeloest.csv"):
        # ohne Seite (Connection.eod ohne eb2a/60402) = nicht platziert: automatische Brücken zwischen Teilklemmen
        # (am letzten Anschluss), Verbindungen über Abbruchstellen u. ä. – EPLAN zeichnet sie nicht (Hilfe
        # connectionbrowsergui_k_verbindungseigenschaften, terminalgui_k_brueckenbildung)
        if not r["seite_id"]:
            nicht["ohne Seite"] += 1
            continue
        try:
            a = pins.get((int(r["A_objekt_id"] or 0), int(r["A_anschlussindex_0basiert"] or 0)))
            z = pins.get((int(r["B_objekt_id"] or 0), int(r["B_anschlussindex_0basiert"] or 0)))
        except ValueError:
            a = z = None
        if not a or not z:
            nicht["offenes Ende" if "0" in (r["A_objekt_id"] or "0", r["B_objekt_id"] or "0") else "Endpunkt fehlt"] += 1
            continue
        (ta, aa), (tz, az) = a, z
        paar = frozenset((a, z))
        if ta.blatt is not tz.blatt:
            nicht["blattübergreifend"] += 1
        elif a == z or paar in schon or ta.pos(aa) == tz.pos(az):
            nicht["doppelt/deckungsgleich"] += 1   # doppelte Verbindungen und deckungsgleiche Punkte lässt QET nicht zu
        else:
            ta.blatt.draht(ta, aa, tz, az)
            schon.add(paar)
            gezeichnet += 1
    fehlt = sum(nicht.values())

    _fortschritt(92, "listen")
    # Auswertungen aus EPLAN als Tabellenblätter
    n = 900
    if not nur_seiten:
        # alle Leisten in einer fortlaufenden Liste (Leiste:Klemme), sortiert – sonst je Leiste ein Blatt (QET max. 200)
        kl = sorted((r for r in _csv(out, "Klemmenplan.csv") if r["Klemme (20030)"]),
                    key=lambda r: (r["Leiste (BMK)"], L._sortkey(r["Klemme (20030)"])))
        zeilen = [[r["Ziele"], r.get("Farbe/Nr (31004)", ""), f'{r["Leiste (BMK)"]}:{r["Klemme (20030)"]}',
                   r.get("Anschluss", ""), r["Seite"]] for r in kl]
        n = L.seiten(plan, "Klemmenpläne", "ZKL", "", n, L.KLEMMEN_SPALTEN, zeilen)
        kab = [[r["Kabel (BMK)"], "", "", r["Typ (20040)"], r["Aderanzahl (20042)"], r["Querschnitt (20043)"]]
               for r in _csv(out, "Kabeluebersicht.csv")]
        n = L.seiten(plan, "Kabelübersicht", "ZKP", "", n, L.KABEL_SPALTEN, kab)
        st = [[r["BMKs"][:60], r["Bestellnummer (22003)"], r["Hersteller (22007)"], r["Bezeichnung 1 (22004)"],
               r["Anzahl Referenzen"]] for r in _csv(out, "Stueckliste_summiert.csv")]
        n = L.seiten(plan, "Stückliste", "ZST", "", n, L.STUECK_SPALTEN, st)

    _fortschritt(95, "schreiben")
    erg, _ = plan.erzeuge(os.path.basename(ziel_qet), pdf=False)
    # QETs eigene Verweistexte an den Pfeilen aus: der Querverweis steht als EPLAN-Text am Unterbrechungspunkt
    s_qet = open(erg, encoding="utf-8").read()
    open(erg, "w", encoding="utf-8").write(re.sub(r'<report label="[^"]*"', '<report label=""', s_qet))
    G.in_projekt_einfuegen(erg, {f: (formen_je_blatt.get(f, []), texte_je_blatt.get(f, []), bilder_je_blatt.get(f, []))
                                 for f in set(formen_je_blatt) | set(texte_je_blatt) | set(bilder_je_blatt)})
    bericht = (f"Seiten {len(seiten)}, Black Boxes {stat['blackbox']} (Funktionen darin {stat['im_kasten']}), "
               f"Kontakte mit Master verknüpft {stat['verknuepft']}, Bauteile aus der Sammlung {stat['bibliothek']}, "
               f"Schaltbilder aus dem Archiv "
               f"in die Sammlung {stat['archiv_in_sammlung']} (neue Dateien {len(sammlung.neu) if sammlung else 0}), "
               f"Data Portal geholt {stat['data_portal_ok']}/nicht gefunden {stat['data_portal_fehlt']}, "
               f"Funktionen mit EPLAN-Symbol {stat['symbol']}, nur als Anschlusspunkte "
               f"{stat['punkte']}, Symbolvarianten {sum(1 for v in cache.values() if v)} "
               f"(Variantensätze nicht lesbar {getattr(gr, 'varianten_unlesbar', 0)}), Anschlüsse {len(pins)}, "
               f"Verbindungen gezeichnet {gezeichnet}, nicht gezeichnet {fehlt} {dict(nicht)}, "
               f"Seitengrafik {sum(len(v) for v in formen_je_blatt.values())} Formen, "
               f"{sum(len(v) for v in bilder_je_blatt.values())} Bilder "
               f"(nicht auflösbar {sum(getattr(gr, 'bilder_fehlend', {}).values())}), offene Pfeile {len(plan.offen)}, "
               f"Normblätter {[k for k, v in normblaetter.items() if v]}, "
               f"Normblatt-Platzhalter ohne Wertquelle {dict(nb_fehlt)}")
    open(os.path.join(arbeit, "import_bericht.txt"), "w", encoding="utf-8").write(bericht + "\n")
    print(bericht)
    if G.PROTOKOLL is not None:
        with open(os.environ["EPLAN_TEXTPROTOKOLL"], "w", encoding="utf-8") as f_:
            for r_ in G.PROTOKOLL:
                f_.write(json.dumps(r_, ensure_ascii=False, default=str) + "\n")
    return erg


def standard_ziel(archiv):
    r"""<Ordner des Archivs>\QET\<Plannummer>_EPLAN_Import.qet"""
    nr = os.path.splitext(os.path.basename(archiv))[0]
    return os.path.join(os.path.dirname(os.path.abspath(archiv)), "QET", nr.replace(" ", "_") + "_EPLAN_Import.qet")


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    archiv = args[0]
    ziel = args[1] if len(args) > 1 else standard_ziel(archiv)
    os.makedirs(os.path.dirname(os.path.abspath(ziel)), exist_ok=True)
    erg = umsetzen(archiv, ziel, args[2] if len(args) > 2 else None,
                   arbeit=os.path.join(os.path.dirname(os.path.abspath(ziel)), "_eplan_import_" +
                                       os.path.splitext(os.path.basename(archiv))[0].replace(" ", "_")),
                   data_portal="--dataportal" in sys.argv, bibliothek="--ohne-sammlung" not in sys.argv)
    if "--pdf" in sys.argv:
        _fortschritt(97, "pdf")
        from qet_direkt import q, BIN
        print("pdf:", q.tool_export(BIN, erg, "pdf", erg.replace(".qet", ".pdf"), timeout=3600).get("ok"))
    _fortschritt(100, "fertig")
    print("fertig:", erg)
    if "--oeffnen" in sys.argv:
        import subprocess
        subprocess.Popen([os.path.join(os.path.expanduser("~"), "Documents", "Code Projekte", "qelectrotech-eplan",
                                       "build", "deploy", "QElectroTech.exe"), erg])
