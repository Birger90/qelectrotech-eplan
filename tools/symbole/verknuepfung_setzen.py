"""Verknüpfungsart (Master/Slave) an Bauteilen einer Sammlung setzen, damit QElectroTech Querverweise bildet.

Importierte Herstellersymbole (EPLAN-DXF/EDZ) kommen als „simple“ an. Für Querverweise braucht QElectroTech:
Spule = Master (coil), Kontakt = Slave, SPS-Baugruppe (Übersicht) = Master (plc), SPS-Kanal = Slave (plc).
Die Rolle wird am Dateinamen erkannt; Geräte mit eigenen Kontakten (Taster, Schalter, Thermostat) bleiben „simple“.

    python verknuepfung_setzen.py SAMMLUNG [--nur-zeigen]
"""
import argparse
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

# Übersichten nur bei E/A-Baugruppen (6ES713x): CPU, BusAdapter und Panel haben keine Kanäle.
# „mit_Hilfskontakten“ ist ein ganzes Gerät (Motorschutzschalter), kein einzelner Kontakt.
# (Muster im Dateinamen, Ordner-Muster, link_type, kindInformations)
REGELN = [
    (r"_Spule", r"", "master", {"type": "coil"}),
    # Wendestarter 3RM1: Steuerteil (C1) trägt die Querverweise, Hauptstromkreis (B1) ist sein Leistungskontakt
    (r"^3RM1.*_C1", r"", "master", {"type": "coil"}),
    (r"^3RM1.*_B1", r"", "slave", {"type": "power", "state": "NO", "number": "3"}),
    (r"Motorschutzschalter_mit_Hilfskontakten", r"", "master", {"type": "protection"}),
    (r"_Uebersicht", r"/SPS/6ES713", "master", {"type": "plc"}),
    (r"_Kanal_", r"/SPS/", "slave", {"type": "plc", "state": "NO", "number": "1"}),
    (r"Hauptkontakte", r"", "slave", {"type": "power", "state": "NO", "number": "3"}),
    (r"Hauptkontakt_", r"", "slave", {"type": "power", "state": "NO", "number": "1"}),
    (r"Hilfskontakt(?!en)", r"", "slave", {"type": "simple", "state": "NO", "number": "1"}),
    (r"Fehlerkontakt|Wechsler", r"Relais|Schuetze|Schütze|Zeitrelais|Sanftstarter|Schaltglieder", "slave",
     {"type": "simple", "state": "SW", "number": "1"}),
    (r"Schliesser_1-2_3-4", r"Relais", "slave", {"type": "simple", "state": "NO", "number": "2"}),
    (r"Schliesser_\d", r"Schaltglieder", "slave", {"type": "simple", "state": "NO", "number": "1"}),
]


def regel(pfad: Path):
    name, ordner = pfad.name, pfad.as_posix()
    for muster, ort, art, kind in REGELN:
        if re.search(muster, name) and re.search(ort, ordner):
            return art, kind
    return None


def setzen(pfad: Path, art, kind):
    baum = ET.parse(pfad)
    d = baum.getroot()
    if d.get("link_type") == art:
        return False
    d.set("link_type", art)
    alt = d.find("kindInformations")
    if alt is not None:
        d.remove(alt)
    ki = ET.Element("kindInformations")
    for k, v in kind.items():
        e = ET.SubElement(ki, "kindInformation", name=k)
        e.text = v
    # wie QElectroTech: direkt nach den Namen bzw. Bauteilinformationen
    pos = [i for i, c in enumerate(d) if c.tag in ("names", "elementInformations")]
    d.insert(pos[-1] + 1 if pos else 0, ki)
    baum.write(pfad, encoding="utf-8", xml_declaration=False)
    return True


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    ap = argparse.ArgumentParser()
    ap.add_argument("sammlung")
    ap.add_argument("--nur-zeigen", action="store_true")
    a = ap.parse_args()
    for pfad in sorted(Path(a.sammlung).rglob("*.elmt")):
        r = regel(pfad)
        if r and (a.nur_zeigen or setzen(pfad, *r)):
            print(f"{r[0]:6} {r[1]['type']:7} {pfad.relative_to(a.sammlung)}")


if __name__ == "__main__":
    main()
