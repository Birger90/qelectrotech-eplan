# sPlan-Import (.spl8 → QElectroTech)

Liest einen Schaltplan aus sPlan 8 (Abacom) und zeichnet ihn als QElectroTech-Projekt neu: gleiche Geräte, gleiche
Verbindungen, aber im Raster, mit geraden Leitungen und Normsymbolen. Aufruf aus QET: **Datei > sPlan-Plan
importieren (.spl8)…**, oder direkt:

```
python splan_nach_qet.py <plan.spl8> [<ziel.qet>] [--pdf] [--vergleich <original.pdf>]
```

Ohne Ziel landet das Projekt in `<Ordner des Plans>\QET\<Name>.qet`. Daneben entstehen `Offene_Fragen.txt`
(was der Plan nicht eindeutig hergibt), mit `--pdf` das PDF und mit `--vergleich` ein PDF „Original | neu“ je Blatt.
Voraussetzungen: Python 3.11+ mit Pillow und pymupdf, der gebaute Fork und `qet_direkt.py` (wie beim EPLAN-Import).

## Aufbau

| Datei | Aufgabe |
|---|---|
| `splan8.py` | Leser des Binärformats (siehe `FORMATBESCHREIBUNG_SPL8.md`) |
| `splan_analyse.py` | Geometrie → Bauteile mit Anschlüssen, Netze, Potentialnamen, offene Fragen |
| `splan_nach_qet.py` | Funktion erkennen, Symbol wählen, Blatt neu anordnen, QET-Projekt schreiben, Layout prüfen |

Gezeichnet wird mit der Blatt-API aus `../eplan_import/planvorlagen` (Raster, Schienen, Abgriffe, Verweispfeile,
Master/Slave).

## Regeln beim Neuzeichnen

- **Symbole**: Schließer/Öffner → IEC-Kontakt (Slave), Schütz → Spule (Master), Reihenklemme → Klemme (mehrstöckige
  „Klemmen“ je Ebene), Leistungsschalter → 3-poliger Schutzschalter, FI → FI/LS, Motor (Wechsel-/Gleichstrom), NTC.
  Alles andere (Netzteile, SPS-Module, Treiber, Geräte als Bild) wird ein beschrifteter Gerätekasten mit den
  Anschlüssen des sPlan-Bauteils, abgelegt unter `company://sPlan-Import/`.
- **Anordnung**: Potentiale wie 24 VDC, 48 VDC, L1 → Schiene oben; GND, 0 V, M, N, PE → Schiene unten. Gleichnamige
  Schienen verschiedener Blätter werden Folgeverweise. Geräte in Zeilen in der Reihenfolge des Originals
  (oben → unten, links → rechts), Klemmleisten geschlossen und nach Nummer, jedes Gerät möglichst senkrecht über
  oder unter seinem Partner.
- **Leitungen**: je Netz ein kürzester Baum; zwischen zwei Zeilen bekommt jede Leitung eine eigene Spur (keine
  aufeinanderliegenden Leiter). Potentialnamen ohne Schiene werden Verweispfeile mit dem Ziel als Text.
- **Querverweise**: Spule ↔ Kontakte über die sPlan-Parent-ID, sonst über gleiches Kennzeichen (auch blattübergreifend).
- **Nichts geht verloren**: Texte ohne Zuordnung stehen unten auf dem Blatt; Blätter ohne Schaltung (Datenblätter,
  Fotos) werden Dokumentationsblätter; Bilder neben einer Schaltung bekommen ein eigenes Blatt.
- Zum Schluss `qet_layout_check` und Übernahme seiner Korrekturen.
