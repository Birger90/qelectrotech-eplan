# EPLAN-Import (.zw1 → QElectroTech)

Liest ein gepacktes EPLAN-Projekt (.zw1) rein lokal und erzeugt daraus ein QElectroTech-Projekt (.qet)
mit Seiten, Normblatt, Symbolen, Verbindungen, Querverweisen, Kontaktspiegeln und Black Boxes.
Jede Übersetzungsregel ist belegt: aus dem EPLAN-Programmcode, den Archivdaten oder der EPLAN-Hilfe
(siehe `eplan_zw1/FORMATBESCHREIBUNG_EOD.md`).

## Aufbau

- `eplan_zw1/` – Dekoder der EPLAN-Datenbankdateien (`eodlib`, `autodetect`, `parseall`, `export*`),
  Grafik/Schrift (`eplan_grafik`), Bauteilsammlung (`eplan_bibliothek`), Hauptprogramm `eplan_nach_qet.py`.
- `eplan_zw1/daten_typen/` – Typtabellen des EPLAN-Metamodells.
- `planvorlagen/` – Bausteine und Blatt-API, mit denen der Import (und eigene Vorlagen) Seiten zeichnet.

## Aufruf

```
python eplan_zw1/eplan_nach_qet.py "<Projekt>.zw1" [--pdf] [--oeffnen] [--dataportal]
```

Voraussetzungen: Python 3.11, 7-Zip, der QElectroTech-Fork aus diesem Repository (gebaut) und
`.claude/skills/qet-schaltplan-zeichnen/vorlagen/qet_direkt.py`.

Pfade über Umgebungsvariablen (sonst die Vorgaben im Code):

| Variable | Bedeutung |
|---|---|
| `QET_EXE` | QElectroTech.exe des Forks |
| `QET_DATEN` | Datenordner (enthält `elements-company`) |
| `QET_COMPANY_ELEMENTS_DIR` | Firmensammlung der Bauteile |
| `QET_COMMON_ELEMENTS_DIR` | `elements` der QET-Installation |
| `QET_DIREKT_DIR` | Ordner mit `qet_direkt.py` |
| `SIEBENZIP` | `7z.exe` |

Herstellerdaten (Data Portal, Makros, STEP) gehören nicht in dieses Repository; sie liegen in der
eigenen Firmensammlung.
