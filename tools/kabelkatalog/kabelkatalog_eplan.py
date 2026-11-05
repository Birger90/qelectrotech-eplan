"""Kabelkatalog für QElectroTech (Typwahl, Ordner cable-catalogs) aus EPLAN-Artikeldaten der Hersteller.

Liest EPLAN-Artikel (partsmanagement-XML: eine Sammeldatei wie igus „IGUS_072019.xml“ oder ein Ordner einzelner
*.part.xml wie im LAPP-EDZ unter items/partxml) und schreibt je Kabelartikel eine Zeile der Kabeltypdatei:

    designation           Kabelbezeichnung (EPLAN Kabelbezeichnung, sonst Typnummer + Aufbau), z. B. „ÖLFLEX 191 25G1“
    cores                 Zahl der Adern (Funktionsschablonen Kategorie 4 = Ader, ohne Schirm)
    core_colors           Aderkennzeichnung in Reihenfolge, durch Komma getrennt (GNYE, 1, 2 … / WH, BN, GN …)
    section               Querschnitt der Adern in mm²
    color                 Mantelfarbe, falls angegeben
    manufacturer          Hersteller
    manufacturer_reference Bestellnummer
    description           Artikelbeschreibung (deutsch)

Ein Kabelartikel ist ein Artikel mit einer Funktionsschablone der Kategorie 400 (Kabeldefinition).

    python kabelkatalog_eplan.py <XML-Datei oder Ordner> <Ziel.csv>
"""
import csv
import io
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

SPALTEN = ["designation", "cores", "core_colors", "section", "outer_diameter", "chain", "sheath", "voltage",
           "bending_radius", "properties", "color", "manufacturer", "manufacturer_reference", "description"]
TITEL = ["Bezeichnung", "Anzahl der Adern", "Aderfarben", "Querschnitt", "Außendurchmesser", "Energiekette",
         "Mantel", "Nennspannung", "Biegeradius", "Eigenschaften", "Mantelfarbe", "Hersteller", "Artikelnummer",
         "Beschreibung"]
# Eigenschaften, die die Herstellertexte hergeben (Ölbeständigkeit steht in keinem der EPLAN-Pakete)
EIGENSCHAFTEN = [("energiekettentauglich", None),
                 ("halogenfrei", re.compile(r"halogenfrei|halogen.?free|LSZH|FRNC|\bH\b(?=[^a-z])", re.I)),
                 ("torsionsfest", re.compile(r"torsion", re.I)),
                 ("Roboter", re.compile(r"robot", re.I)),
                 ("UL/CSA", re.compile(r"\bUL\b|\bCSA\b|\bAWM\b", re.I)),
                 ("kältebeständig", re.compile(r"kälte|cold|low.?temp", re.I)),
                 ("ölbeständig", re.compile(r"ölbest|ölfest|oil.?resist", re.I)),
                 ("UV-beständig", re.compile(r"\bUV\b", re.I))]
# energiekettentauglich: igus chainflex immer; LAPP an der Serie (FD, CHAIN, ROBOT …) oder am Text
KETTE = re.compile(r"\bFD\b|\bCHAIN\b|ROBOT|TRAXLINE|chainflex|Schleppkette|Energiekette|e-kette|e-chain|"
                   r"energy chain|drag chain", re.I)
MANTEL = re.compile(r"\b(PUR|PVC|TPE|TPU|PE|FRNC|Gummi|Silikon|PTFE|FEP)\b", re.I)


def text(wert, sprache="de_DE"):
    """Mehrsprachiger EPLAN-Wert „de_DE@…;en_US@…;“ -> deutscher Text (sonst der erste)."""
    if not wert:
        return ""
    teile = [t for t in wert.split(";") if "@" in t]
    if not teile:
        return wert.strip()
    for t in teile:
        if t.startswith(sprache + "@"):
            return t.split("@", 1)[1].strip()
    return teile[0].split("@", 1)[1].strip()


def lesen(pfad):
    """XML einlesen; manche Herstellerdateien sind als UTF-8 erklärt, enthalten aber Windows-1252-Zeichen (®)."""
    roh = Path(pfad).read_bytes()
    try:
        s = roh.decode("utf-8")
    except UnicodeDecodeError:
        s = roh.decode("cp1252")
    return ET.fromstring(re.sub(r"^<\?xml[^>]*\?>", "", s.lstrip("﻿")))


def zahl(s):
    try:
        return float(str(s).replace(",", "."))
    except ValueError:
        return None


def kabel(part):
    """Zeile der Kabeltypdatei für einen Kabelartikel, sonst None."""
    variante = part.find("variant")
    if variante is None:
        return None
    vorlagen = variante.findall("functiontemplate")
    if not any(v.get("functiondefcategory") == "400" for v in vorlagen):
        return None
    adern = [v for v in vorlagen if v.get("functiondefcategory") == "4" and v.get("functiondefid") == "4"]
    farben = [(v.get("connectionDesignation") or "").strip() for v in adern]
    querschnitte = {zahl(v.get("terminalNr")) for v in adern if zahl(v.get("terminalNr"))}
    qs = variante.get("P_ARTICLE_CABLEWIRECROSSSECTION") or (
        str(max(querschnitte)).replace(".", ",") if querschnitte else "")
    bez = text(variante.get("P_ARTICLE_CABLEDESIGNATION"))
    typ = (part.get("P_ARTICLE_TYPENR") or "").strip()
    aufbau = (variante.get("P_ARTICLE_WIRECROSSSECTION_AND_DIAMETER") or "").strip()
    if not bez:
        bez = f"{typ} {aufbau}".strip() or text(part.get("P_ARTICLE_DESCR1"))
    notiz = text(part.get("P_ARTICLE_NOTE"))
    frei = " ".join(text(f.get("P_ARTICLE_FREE_DATA_VALUE")) for f in part.findall("freeproperty"))
    alles = " ".join([bez, typ, text(part.get("P_ARTICLE_DESCR1")), notiz, frei])
    d = re.search(r"[\d.,]+", variante.get("P_ARTICLE_OUTERDIAMETER") or "")
    durchmesser = zahl(d.group(0)) if d else None
    mantel = MANTEL.search(" ".join([text(part.get("P_ARTICLE_DESCR1")), frei, notiz]))
    return {"designation": bez,
            "outer_diameter": (f"{durchmesser:g}".replace(".", ",") if durchmesser else ""),
            "chain": "Ja" if KETTE.search(alles) else "Nein",
            "properties": ", ".join(
                ([n for n, r in EIGENSCHAFTEN if (r is None and KETTE.search(alles)) or (r and r.search(alles))])
                + (["geschirmt"] if "SH" in farben or re.search(r"geschirmt|shielded|screened|\bC[YP]?\b", bez)
                   else [])),
            "sheath": mantel.group(1).upper() if mantel else "",
            "voltage": (variante.get("P_ARTICLE_VOLTAGE") or "").strip(),
            "bending_radius": (variante.get("P_ARTICLE_BENDINGRADIUS") or "").strip(),
            "cores": str(len(adern) or variante.get("P_ARTICLE_CABLEWIRECOUNT") or ""),
            "core_colors": ",".join(farben),
            "section": (qs or "").replace(".", ","),
            "color": text(variante.get("P_ARTICLE_CABLECOLOR") or part.get("P_ARTICLE_CABLECOLOR")),
            "manufacturer": text(part.get("P_ARTICLE_MANUFACTURER")),
            "manufacturer_reference": (part.get("P_ARTICLE_ORDERNR") or "").strip(),
            "description": text(part.get("P_ARTICLE_DESCR1"))}


def quellen(pfad):
    p = Path(pfad)
    if p.is_dir():
        return sorted(f for f in p.glob("*.part.xml"))
    return [p]


def main(quelle, ziel):
    zeilen, gesehen = [], set()
    for datei in quellen(quelle):
        try:
            root = lesen(datei)
        except ET.ParseError as e:
            print("übersprungen:", datei.name, e)
            continue
        for part in root.iter("part"):
            z = kabel(part)
            if z and (z["manufacturer_reference"], z["designation"]) not in gesehen:
                gesehen.add((z["manufacturer_reference"], z["designation"]))
                zeilen.append(z)
    zeilen.sort(key=lambda z: [int(t) if t.isdigit() else t.lower() for t in re.split(r"(\d+)", z["designation"])])
    puffer = io.StringIO()
    w = csv.writer(puffer, delimiter=";", lineterminator="\r\n")
    w.writerow(TITEL)
    w.writerow(SPALTEN)
    for z in zeilen:
        w.writerow([z[s] for s in SPALTEN])
    Path(ziel).parent.mkdir(parents=True, exist_ok=True)
    Path(ziel).write_bytes(puffer.getvalue().encode("utf-8-sig"))   # Zeilenenden nicht noch einmal übersetzen
    print(f"{ziel}: {len(zeilen)} Kabeltypen")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
