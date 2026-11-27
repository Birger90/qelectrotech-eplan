# EPLAN-Import (.zw1 und .epdz → QElectroTech)

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
python eplan_zw1/eplan_nach_qet.py "<Export>.epdz" [ziel.qet] [--pdf] [--oeffnen]
```

Eine `.epdz` (EPLAN-Export für eVIEW/eBUILD) reicht `eplan_nach_qet.py` an `epdz_nach_qet.py` weiter. Die
Export-Datei enthält keine Symbolbibliothek, sondern je Seite ein SVG, in dem jede Funktion eine Gruppe
`Id17_<Nummer>` ist, und `manifest.db` mit den EPLAN-Eigenschaften und der Verbindungsliste. Daraus entsteht:
- je Funktion ein Bauteil mit ihrer gezeichneten Grafik (`elements-company/EPLAN-Import/EPDZ/`); Anschluss ist jeder
  Punkt, an dem eine Verbindungslinie auf das Symbol trifft (Ecke, Strich oder Kreisrand), benannt über den
  Anschlusstext, sonst über die Verbindungsliste; Geräteanschlusspunkte („-PF1:1“) gehören zum Gerät;
- Leitungen aus der Verbindungsliste (Ende „BMK:Anschluss“, auch mit Stecker), wo beide Enden im selben gezeichneten
  Netz liegen; Netzteile ohne Listeneintrag über den nächsten Anschluss. Jede Leitung läuft über die Ecken der
  EPLAN-Linie (`set_conductor_path`), Abzweig-Schrägen werden Ecken;
- Unterbrechungspunkte als verknüpfte Verweispfeile: die Sprungziele bilden je Potential eine Kette über die
  Seiten, Paare sind aufeinanderfolgende Glieder auf verschiedenen Seiten;
- Rahmen, Schriftfeld, Struktur-/Ortskästen und Farb-/Querschnittangaben als Blattgrafik; Geräteliste,
  Verbindungsliste (CSV) und 3D-Bauräume (`.e3d`) neben dem Projekt; Bericht in `_epdz_import_<Name>/import_bericht.txt`.

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
