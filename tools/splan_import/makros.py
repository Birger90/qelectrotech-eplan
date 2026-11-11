"""Zuordnung sPlan-Bauteil -> Herstellermakro (EPLAN-Stil).

sPlan kennt keine Artikel: Ein Bauteil ist eine Gruppe von Strichen mit einem Bibliotheksnamen („CPU“, „Netzteil“).
Welches Gerät es wirklich ist, steht als Aufdruck im Symbol („230VAC“, „RELAIS OUTPUTS“, „Weidmüller PRO MAX“,
„Em542S“) oder nur im Bild – und der Bibliotheksname kann sogar falsch sein (Treiber „SMCI33-2“ mit Aufdruck
„Leadshine Em542S“). Diese Entscheidung trifft deshalb ein Mensch oder die KI, mit Beleg, und schreibt sie in eine
Zuordnungsdatei (JSON). Der Import übersetzt sie dann wie ein Compiler: gleicher Plan + gleiche Zuordnung = gleiches
Ergebnis. `--makros-vorlage` schreibt das Gerüst mit allen Geräten, ihren Aufdrucken und Anschlüssen.

Aufbau der Datei:
{
  "pfade":    {"S7AC": "company://Siemens/SPS/6ES7212-1BE40-0XB0/6ES7212-1BE40-0XB0_"},   # Abkürzungen
  "vorlagen": {"CPU1212C-AC": { …wie ein Gerät, ohne „wenn“… }},                           # wiederverwendbar
  "geraete": [
    {"wenn": {"blatt": 6, "bmk": "A2"},                  # alle Angaben müssen passen; ohne blatt: jedes Blatt
                                                         # weitere: "symbol" (Bibliotheksname), "text" (Aufdruck)
     "vorlage": "CPU1212C-AC",                           # optional, eigene Angaben überschreiben die Vorlage
     "hersteller": "Siemens", "artikel": "6ES7212-1BE40-0XB0", "typ": "CPU 1212C AC/DC/Relais",
     "beleg": "Aufdruck 230VAC, RELAIS OUTPUTS",         # warum dieser Artikel (steht in Offene_Fragen.txt)
     "norm": true,                                       # Normsymbol behalten, nur Artikeldaten setzen
     "kanaele": {                                        # sonst: je sPlan-Anschluss das Makro (EPLAN-Kanalmakro)
        ".0": {"makro": "S7AC:DI_E1", "anschluss": "DI", "adresse": "%I0.0"},
        "L+": {"makro": "S7AC:L1-FE_A1", "anschluss": "L1", "gruppe": "netz"},   # gleiche Gruppe = ein Makro
        "1.2": {"aus": true, "grund": "PROFINET-Kabel, kein Klemmanschluss"}}}  # bewusst nicht gezeichnet
  ]
}
"""
import json
import os
import re


class Regel:
    def __init__(self, d, pfade):
        self.d, self.pfade = d, pfade
        self.wenn = d.get("wenn", {})
        self.hersteller, self.artikel = d.get("hersteller", ""), d.get("artikel", "")
        self.typ, self.beleg = d.get("typ", ""), d.get("beleg", "")
        self.norm = bool(d.get("norm"))
        self.kanaele = d.get("kanaele") or {}
        self.benutzt = 0

    def pfad(self, kurz):
        """„S7AC:DI_E1“ -> company://…/6ES7212-1BE40-0XB0_DI_E1.elmt"""
        if "://" not in kurz and ":" in kurz:
            k, rest = kurz.split(":", 1)
            if k in self.pfade:
                kurz = self.pfade[k] + rest
        return kurz if kurz.endswith(".elmt") else kurz + ".elmt"

    def infos(self):
        i = {}
        if self.hersteller:
            i["manufacturer"] = self.hersteller
        if self.artikel:
            i["manufacturer_reference"] = self.artikel
        if self.typ:
            i["designation"] = self.typ
        return i

    def passt(self, bt, blatt):
        w = self.wenn
        if "blatt" in w and int(w["blatt"]) != blatt:
            return False
        if "bmk" in w and w["bmk"] != bt.bez:
            return False
        if "symbol" in w and w["symbol"].lower() != (bt.symbol or "").lower():
            return False
        if "wert" in w and w["wert"].lower() != (bt.wert or "").lower():
            return False
        if "text" in w and not any(w["text"].lower() in t.lower() for t, _ in bt.texte):
            return False
        return bool(w)


class Zuordnung:
    def __init__(self, pfad=None):
        self.regeln, self.quelle = [], pfad
        if not pfad:
            return
        with open(pfad, encoding="utf-8") as f:
            daten = json.load(f)
        pfade = daten.get("pfade", {})
        vorlagen = daten.get("vorlagen", {})
        for g in daten.get("geraete", []):
            if "vorlage" in g:
                basis = dict(vorlagen[g["vorlage"]])
                kan = dict(basis.get("kanaele") or {})
                kan.update(g.get("kanaele") or {})      # Gerät ergänzt/überschreibt einzelne Anschlüsse
                basis.update(g)
                basis["kanaele"] = kan
                g = basis
            self.regeln.append(Regel(g, pfade))

    def finde(self, bt, blatt):
        for r in self.regeln:
            if r.passt(bt, blatt):
                r.benutzt += 1
                return r
        return None

    def unbenutzt(self):
        return [r for r in self.regeln if not r.benutzt]


def gruppen(bt, regel, fragen, nummer):
    """sPlan-Anschlüsse des Bauteils auf Makros verteilen.
    Ergebnis: [(makropfad, [(Anschluss, Makroanschluss, adresse)])] in Reihenfolge der Zuordnungsdatei, dazu die
    Anschlüsse ohne Eintrag (die bleiben in einem Restkasten, damit keine Verbindung verloren geht)."""
    nach_name = {}
    for a in bt.anschluesse:
        nach_name.setdefault(a.name, []).append(a)
    gruppen_, reihenfolge = {}, []
    for name, k in regel.kanaele.items():
        for a in nach_name.pop(name, []):
            if k.get("aus"):
                fragen.append(f"Blatt {nummer}: {bt.bez}:{name} nicht gezeichnet – {k.get('grund', 'laut Zuordnung')}.")
                continue
            schl = (regel.pfad(k["makro"]), k.get("gruppe") or f"_{name}_{id(a)}")
            if schl not in gruppen_:
                gruppen_[schl] = []
                reihenfolge.append(schl)
            gruppen_[schl].append((a, k["anschluss"], k.get("adresse", "")))
    rest = [a for liste in nach_name.values() for a in liste]
    return [(schl[0], gruppen_[schl]) for schl in reihenfolge], rest


def vorlage_schreiben(pfad, inhalte):
    """Gerüst der Zuordnungsdatei: jedes Gerät mit Bibliotheksname, Wert, Aufdrucken und Anschlüssen samt Ziel."""
    geraete = []
    for nummer, inh in inhalte:
        for bt in inh.bauteile:
            if not bt.anschluesse:
                continue
            namen = {a.name for a in bt.anschluesse}
            geraete.append({
                "wenn": {"blatt": nummer, "bmk": bt.bez} if bt.bez else {"blatt": nummer, "symbol": bt.symbol},
                "_sPlan": {"symbol": bt.symbol, "wert": bt.wert,
                           "aufdrucke": [t for t, _ in bt.texte if t not in namen][:40],
                           "anschluesse": {a.name: (a.netz.name or ", ".join(
                               str(b) for b in a.netz.anschluesse if b is not a)) if a.netz else ""
                               for a in bt.anschluesse}},
                "hersteller": "", "artikel": "", "typ": "", "beleg": "", "kanaele": {}})
    with open(pfad, "w", encoding="utf-8") as f:
        json.dump({"pfade": {}, "vorlagen": {}, "geraete": geraete}, f, ensure_ascii=False, indent=1)
    return pfad


def standardpfad(ziel_qet):
    return re.sub(r"\.qet$", "", ziel_qet) + "_makros.json"


def lade_fuer(ziel_qet, ausdruecklich=None):
    p = ausdruecklich or standardpfad(ziel_qet)
    return Zuordnung(p if os.path.exists(p) else None)
