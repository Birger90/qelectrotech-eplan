"""Firmensammlung (elements-company) für den EPLAN-Import: vorhandene Bauteile zuerst, fehlende ergänzen.

Reihenfolge je Funktion mit Artikel (Regel „Herstellersymbole zuerst“):
1. Bauteil der Sammlung mit gleicher Bestellnummer, dessen Schaltbild-Anschlüsse genau die Anschlüsse der
   Funktion tragen (Zuordnung über den Namen); Reihenklemmen (link_type terminal, o/u) über die Lage.
2. Fehlt der Artikel ganz: optional Data Portal (Fork-Skript-API importPartFromDataPortal) in die Sammlung.
3. Sonst: Schaltbild aus der EPLAN-Symbolvariante des Archivs + Artikeldaten, abgelegt unter
   Hersteller/Kategorie/Bestellnummer (bzw. im schon vorhandenen Ordner des Artikels) – die Sammlung wächst mit.
Kabel (Kabeldefinition) und reines Zubehör bekommen kein Symbol."""
import collections
import html
import json
import os
import re
import subprocess
import tempfile

from planvorlagen import symbole as S

# EPLAN-Herstellerkürzel (22007) -> Ordnername der Sammlung
HERSTELLER = {"SIE": "Siemens", "WEI": "Weidmüller", "RIT": "Rittal", "PXC": "Phoenix Contact",
              "MURR": "Murrelektronik", "P&O": "Petzold&Otto", "CYB": "Cybelec", "HAR": "HARTING",
              "HELU": "Helukabel", "RuH": "RuH", "Schmer": "Schmersal", "IFM": "ifm", "FIN": "Finder",
              "FIES": "Fiessler", "K&N": "Kraus und Naimer", "IRT": "IRT", "LAZ": "LazerSafe",
              "MCT": "METZ CONNECT", "LAPP": "Lapp", "SICK": "SICK", "BOS": "Bosch Rexroth",
              "SEW": "SEW-EURODRIVE", "WAG": "WAGO", "ELM": "Elmeko"}
# Funktionsdefinition (efc5) der Hauptfunktion -> Kategorie
KATEGORIE = {100: "Klemmen", 101: "Klemmen", 102: "Klemmen", 201: "Befehlsgeräte", 202: "Befehlsgeräte",
             203: "Befehlsgeräte", 204: "Befehlsgeräte", 300: "Steuerungen", 301: "Schutzeinrichtungen",
             302: "Schutzeinrichtungen", 303: "Schutzeinrichtungen", 400: "Kabel", 500: "Steckverbinder",
             501: "Steckverbinder", 502: "Schaltgeräte", 1302: "Geräte", 1303: "Geräte", 1305: "Geräte"}


ZUBEHOER = re.compile(r"verbindungsbaustein|kennzeichnung|schild|halter|kabelverschraubung|verschraubung|adapter|"
                      r"abdeckung|endplatte|endhalter|brücke|querverbinder|beschriftung|gegenmutter|zubehör|betätiger|klemmenblock|klemmenabdeckung|hilfsschalterblock", re.I)


def kategorie(fd):
    if fd in KATEGORIE:
        return KATEGORIE[fd]
    return {1: "Klemmen", 2: "Befehlsgeräte", 3: "Steuerungen"}.get((fd or 0) // 100, "Sonstiges")


def hauptartikel(arts):
    """Bei mehreren Artikeln einer Funktion das Gerät, nicht das Zubehör."""
    for a in arts:
        if a.get("Bestellnummer (22003)") and not ZUBEHOER.search(a.get("Bezeichnung 1 (22004)", "")):
            return a
    return None


def norm(nr):
    return re.sub(r"[\s.\-/]", "", (nr or "").upper())


class Sammlung:
    def __init__(self, wurzel=None):
        self.wurzel = wurzel or S.COMPANY
        self.index = collections.defaultdict(list)       # Bestellnr -> [(rel, link_type, anschlüsse, aufbau)]
        self.ordner = {}                                   # Bestellnr -> vorhandener Ordner (rel)
        for d, _, fs in os.walk(self.wurzel):
            rel_d = os.path.relpath(d, self.wurzel)
            if rel_d.split(os.sep)[0] in ("EPLAN-Import", "Plaene"):
                continue
            for f in fs:
                if not f.endswith(".elmt"):
                    continue
                s = open(os.path.join(d, f), encoding="utf-8", errors="replace").read()
                m = re.search(r'name="manufacturer_reference"[^>]*>([^<]*)<', s)
                if not m:
                    continue
                n = norm(html.unescape(m.group(1)))
                self.ordner.setdefault(n, rel_d)
                namen = [html.unescape(t) for t in re.findall(r'<terminal [^>]*?name="([^"]*)"', s)]
                lt = (re.search(r'link_type="([^"]*)"', s) or [None, "simple"])[1]
                aufbau = "aufbau" in f.lower() or "schrankplan" in f.lower()
                self.index[n].append((os.path.join(rel_d, f).replace(os.sep, "/"), lt, namen, aufbau))
        self.neu, self.genutzt, self.portal = set(), collections.Counter(), {}

    def hat(self, bestellnr):
        return norm(bestellnr) in self.index

    def waehle(self, bestellnr, pins, klemme):
        """Bauteil der Sammlung für eine Funktion: (company://pfad, {pin_index: anschlussname}) oder None."""
        belegt = [(i, p) for i, p in enumerate(pins) if p]
        for rel, lt, namen, aufbau in self.index.get(norm(bestellnr), []):
            if aufbau or not namen or "EPLAN-Schaltbild" in rel:
                continue
            if belegt and sorted(p for _, p in belegt) == sorted(namen) and len(set(namen)) == len(namen):
                self.genutzt[rel] += 1
                return S.U + rel, {i: p for i, p in belegt}
            if klemme and lt == "terminal" and len(namen) == 2 and len(pins) >= 2:
                sym = S.symbol(S.U + rel)
                oben, unten = sorted(sym.anschluesse, key=lambda a: a.y)[:2]
                self.genutzt[rel] += 1
                return S.U + rel, {0: oben.name, 1: unten.name}
        return None

    def ziel_ordner(self, hersteller, bestellnr, fd):
        n = norm(bestellnr)
        if n in self.ordner:
            return self.ordner[n].replace(os.sep, "/")
        her = HERSTELLER.get(hersteller, hersteller or "Sonstige")
        kat = kategorie(fd)
        typ = re.sub(r'[<>:"/\\|?*]', "_", bestellnr.strip())
        return f"{her}/{kat}/{typ}"

    def schaltbild(self, xml, hersteller, bestellnr, bezeichnung, fd, symbolname, variante, pins):
        """EPLAN-Schaltbild als Bauteil in die Sammlung schreiben; liefert company://pfad."""
        ordner = self.ziel_ordner(hersteller, bestellnr, fd)
        typ = re.sub(r'[<>:"/\\|?*\s]', "_", bestellnr.strip())
        sig = re.sub(r"[^A-Za-z0-9+]", "", "-".join(p for p in pins if p))[:24] or "x"
        datei = f"{typ}_EPLAN-Schaltbild_{re.sub(r'[^A-Za-z0-9_+-]', '_', symbolname)}_{variante}_{sig}.elmt"
        info = ('    <elementInformations>\n'
                f'        <elementInformation name="manufacturer" show="1">{html.escape(HERSTELLER.get(hersteller, hersteller))}</elementInformation>\n'
                f'        <elementInformation name="manufacturer_reference" show="1">{html.escape(bestellnr)}</elementInformation>\n'
                f'        <elementInformation name="designation" show="1">{html.escape(bestellnr)}</elementInformation>\n'
                f'        <elementInformation name="description" show="1">{html.escape(bezeichnung)}</elementInformation>\n'
                '    </elementInformations>\n')
        xml = xml.replace("    <informations>", info + "    <informations>", 1)
        ziel = os.path.join(self.wurzel, *ordner.split("/"), datei)
        os.makedirs(os.path.dirname(ziel), exist_ok=True)
        if not os.path.exists(ziel) or open(ziel, encoding="utf-8").read() != xml:
            open(ziel, "w", encoding="utf-8").write(xml)
            self.neu.add(f"{ordner}/{datei}")
        return S.U + f"{ordner}/{datei}"

    def data_portal(self, artikel, qet_exe, protokoll=None, timeout=1800):
        """artikel: [(bestellnr, herstellerkürzel, kategorie)] über die Fork-Skript-API in die Sammlung holen
        (Token aus den QET-Einstellungen). Liefert {bestellnr: Ergebnis}."""
        if not artikel:
            return {}
        js = ("var J=" + json.dumps([[b, HERSTELLER.get(h, h), k] for b, h, k in artikel], ensure_ascii=False) +
              ";\nfor (var i=0;i<J.length;i++){ var r=qet.importPartFromDataPortal(J[i][0],J[i][1]," +
              json.dumps(self.wurzel.replace("\\", "/")) + ",J[i][2]); qet.log('IMP\\t'+J[i][0]+'\\t'+r); }\n")
        with tempfile.TemporaryDirectory(prefix="eplan-dp-") as tmp:
            skript = os.path.join(tmp, "dp.js")
            open(skript, "w", encoding="utf-8").write(js)
            # ohne Fenster (qoffscreen.dll liegt in build/deploy/platforms); Skript-Zeitgrenze für viele Artikel
            env = dict(os.environ, QET_ENABLE_SCRIPTING="1", QT_QPA_PLATFORM="offscreen",
                       QET_SCRIPT_TIMEOUT_MS=str(timeout * 1000))
            leer = os.path.join(tmp, "leer.qet")                 # --run braucht ein Projekt
            from qet_direkt import q, BIN
            q.tool_project_new(BIN, leer, title="Data Portal", folios=1, overwrite=True)
            r = subprocess.run([qet_exe, "--run", skript, leer], env=env, capture_output=True, text=True,
                               encoding="utf-8", errors="replace", timeout=timeout)
        aus = {}
        for zeile in (r.stdout + r.stderr).splitlines():
            if "IMP\t" not in zeile:
                continue
            teile = zeile.split("IMP\t", 1)[1].split("\t", 1)
            if len(teile) == 2:
                try:
                    aus[teile[0]] = json.loads(teile[1])
                except ValueError:
                    aus[teile[0]] = {"ok": False, "error": teile[1]}
        if protokoll:
            open(protokoll, "w", encoding="utf-8").write(r.stdout[-20000:] + r.stderr[-5000:])
        self.portal = aus
        return aus
