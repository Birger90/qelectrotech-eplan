# Plan: Pneumatik-Schaltpläne in QET (Stufe 1)

Stand: 13.11.2026 · Branch `feature/pneumatik` (von `main`, Fork 1.9.0)

Ziel von Stufe 1: Pneumatikpläne sauber **zeichnen**, mit Anschlüssen, Leitungsarten,
Ventil- und Zylinderbaukasten, Stückliste und Verknüpfung Spule ↔ Ventil.
Noch **keine Simulation** (Stufe 2), aber das Datenmodell wird schon dafür angelegt.

Vorbild: FluidSIM 6, Handbuch Kap. 4.6, 4.8, 4.11, 4.12, 6.7, 8.8, 8.9, 11.6.

---

## 1. Datenquellen für Symbole und Makros

| Quelle | Inhalt | Ins Repo? | Status |
|---|---|---|---|
| A. QET-Sammlung `elements/50_pneumatic` | 406 Dateien (Ventile, Zylinder, Wartung, Sensoren) | ist schon drin | Anschlüsse ohne Namen, keine Schaltstellungen |
| B. Eigener Baukasten nach ISO 1219-1 | Ventilkörper, Betätigungen, Zylinder aus Bausteinen | **ja** | neu |
| C. FluidSIM-Bibliothek (Umwandler) | Festo-Symbole | ja (laut Birger, 13.11.2026: Festo-Freigabe ohne Einschränkung) | Format gesichtet (13.11.2026), siehe unten |
| D. Herstellermakros SMC/Festo (EDZ, ZW1) | Teile mit Artikeldaten | nein, nur lokal | Nutzungsbedingungen offen |
| E. SMC-Symbole DXF/SVG | Einzelsymbole | nein, nur lokal | Nutzungsbedingungen offen |

Stand zu C (FluidSIM 6.3 installiert unter `C:\Program Files (x86)\Didactic\FluidSIM6`, gesichtet 13.11.2026):
- `.circ`, `.sym`, `.lib` sind **gzip-gepacktes XML** (UTF-8 mit BOM), Wurzel `<fluidtools2>` bzw. `<fluidtools4>`.
- Beispiele: 2290 Schaltungen in `circ/` und `circ_tp/` (pneu, hyd, et, GRAFCET, Digital, Lehrgänge TP101 …).
- Bibliothek `sym/sym_fs.lib`: 873 Vorlagen, davon 156 Pneumatik.
- Symbol: `<symbol>` → `<symbol-layer ref-x ref-y>` mit `line`, `polyline`, `rect`, `circle`, `arc`, `text` und
  `<connector type="pneumatic" ident designation="4" cx cy>`; Raster 6, Einheit vermutlich mm.
- Bauteil: `<component object-ref="Pneu.DirectionalValve.FivePins">` mit `<dki-data><designation value="-QM1"/>`,
  `<classification-data><type value="directional-valve"/>` und Simulationswerten in `<const>`/`<parameter>`.
- Leitung: `<line object-ref="Pneu.Supply.LongLine"/>` + `<connect con1="O46478.C4" con2="O46503.A" points="…"/>`.
- Schalldämpfer, Entlüftung, Blindstopfen, Verschraubung sind in FluidSIM **eigene Bauteile** mit einem Anschluss
  (Ordner „Silencers and blind plugs“, `Pneu.Supply.Silencer`, `Pneu.Supply.BlindPlug`). In QET bleiben sie
  Abschluss am Anschluss; der Umwandler setzt beim Import den Abschluss am angeschlossenen Port.
- Spule ↔ Ventil: am Ventil-Anschluss `Y1` steht `<link-props><type value="solenoid-valve-coil-target"/>
  <target value="-MB1" comp-ref="…"/>`; die Spule ist `ET.Relay.ValveSolenoid`. Das Ventil hält den Verweis.
- Offen: ein eigener Plan als Datei **und** als PDF/SVG zum Maßstabsabgleich (Raster 6 ↔ QET-Raster 10).

---

## 2. Datenmodell (Grundlage für alles)

### 2.1 Anschlüsse
- Neues Terminal-Attribut `medium`: `electric` (Standard, alte Dateien unverändert) | `pneumatic` | `hydraulic` | `mechanic` | `logic`.
- Anschlussname nach ISO 5599-3 / ISO 1219: `1`, `2`, `3`, `4`, `5`, `12`, `14`, `82`, `84`, `X`, `Y`, `Z` …
- Verbindungsabschluss je Anschluss: keiner | Schalldämpfer | Blindstopfen (FluidSIM 4.8).
- Leitung nur zwischen gleichem Medium (Prüfung im Verbinden, abschaltbar).

### 2.2 Leitungen (Conductor)
- Leitungsart: Arbeitsleitung (durchgezogen) | Steuerleitung (gestrichelt) | Abluft/Leck (gestrichelt fein).
  QET hat schon Strichmuster (`conductordashpattern`), dazu kommt die Art als Eigenschaft.
- Optional Schlauchdaten für die Stückliste: Außen-Ø, Länge, Material (kommt aus dem Klemmen-/Kabelkatalog-Muster).

### 2.3 Fluidbauteil-Beschreibung (neu, im `.elmt` als `<fluid>`-Block)
```
<fluid kind="valve|cylinder|source|treatment|throttle|check|sensor|…">
  <!-- Ventil -->
  <positions initial="2">          <!-- 1…4 Schaltstellungen -->
    <position body="…"/>           <!-- Wege je Stellung, z. B. 1-2, 3 gesperrt -->
  </positions>
  <actuation side="left"  types="manual,detent" spring="0" pilot="0" airspring="0"/>
  <actuation side="right" types="solenoid"      spring="1" pilot="1"/>
  <!-- Zylinder -->
  <cylinder action="double|single" spring="none|rod|piston" cushioning="none|fixed|adjustable"
            bore_mm="" rod_mm="" stroke_mm=""/>
  <!-- Kennwerte für Stufe 2, hier nur speichern -->
  <params qn_nl_min="" c="" b="" />
</fluid>
```
Fehlt der Block, ist das Bauteil ein normales QET-Bauteil (alte Sammlungen bleiben gültig).

### 2.4 Verknüpfungen (FluidSIM „Marken“)
- Spule ↔ Ventilseite: QET Master/Slave nutzen. Ventil = Master mit Slots `links`/`rechts`, Spule = Slave.
- Wegmaßstab am Zylinder ↔ Endschalter/Rollenventil: neues Bauteil „Wegmaßstab“ mit Liste (Marke, von mm, bis mm).
- Querverweise wie bei Spule/Kontakt.

---

## 3. Baukasten nach ISO 1219-1 (Quelle B)

Generator `tools/symbole/fluid_baukasten.py` (Muster: `nachzeichnen_symbole.py`), erzeugt `.elmt` aus einer Beschreibung:
- Ventilkörper je Stellung: gesperrt, Durchfluss, über Kreuz, parallel, Mittelstellungen (gesperrt, entlüftet, belüftet).
- Betätigungen: Taster, Hebel, Pedal, Raste, Stößel, Rolle, Rollenhebel (Leerrücklauf), Magnet, Magnet+Vorsteuerung, pneumatisch, Feder, Luftfeder, Handhilfsbetätigung.
- Wegeventile: 2/2, 3/2, 4/2, 5/2, 4/3, 5/3 in allen Kombinationen der Betätigungen (Liste, nicht alle Permutationen).
- Zylinder: einfach-/doppeltwirkend, Feder, durchgehende Stange, Dämpfung fest/einstellbar, Magnetkolben, Wegmaßstab.
- Übrige: Druckluftquelle, Kompressor, Behälter, Filter, Regler, Wartungseinheit, Öler, Drossel, Drosselrückschlag, Rückschlag, entsperrbar, Wechsel-/Zweidruckventil, Schnellentlüfter, Druckschalter, Manometer, Schalldämpfer, Blindstopfen, Vakuumerzeuger, Sauger, Greifer, Schwenkantrieb, Druckluftmotor.

Die genaue Liste wird gegen ISO 1219-1 abgeglichen. **Offen: Norm liegt nicht vor (kostenpflichtig, DIN Media).**

Zusätzlich: bestehende `50_pneumatic`-Bauteile per Skript mit Anschlussnamen und `<fluid>`-Block nachrüsten (`tools/symbole/anschluesse_ergaenzen.py` erweitern).

---

## 4. Editoren in QET

1. **Ventil konfigurieren** (FluidSIM 4.11): Dialog wählt Stellungen, Körper, Betätigung je Seite, Ruhestellung, dominierendes Signal → Symbol wird zur Laufzeit erzeugt und im Projekt gespeichert (eingebettetes Element).
2. **Zylinder konfigurieren** (FluidSIM 4.12): Typ, Feder, Kolben, Dämpfung, Stange, Kolbenstellung 0/25/50/75/100 %.
3. **Anschluss-Dialog**: Kennzeichnung, Abschluss (Schalldämpfer/Blindstopfen).
4. **Leitungsart** im Leitungs-Seitenfenster.

Alles über `QUndoCommand` (Merge-IDs in `dev_doc/ID_of_QUndoCommand.txt` eintragen).

---

## 5. Prüfungen und Auswertungen

- Zeichnung prüfen: offene pneumatische Anschlüsse ohne Abschluss, Medium gemischt, Ventilseite ohne Spule, Spule ohne Ventil (FluidSIM 6.7).
- Stückliste: Fluidbauteile mit Hersteller/Bestellnummer, Schläuche als Meterware.
- Kennzeichnung nach DIN EN 81346-2 (z. B. `-MM1` Zylinder, `-QM1` Ventil, `-MB1` Spule, `-BG1` Sensor) als Vorschlag beim Einfügen.

---

## 6. Import

- **EPLAN ZW1**: prüfen, ob Fluid-Seiten (Festo-Projekte, Seitentyp Fluid, Kennung `&MFS`/`&MFA` in Festo-Beispielen) schon ankommen. Testdatei nötig.
- **FluidSIM** (Quelle C): erst nach Sichtung der Beispieldateien. Ergebnis: Umwandler `tools/fluidsim_import/`, Format-Beschreibung wie `FORMATBESCHREIBUNG_EOD.md`.

---

## 7. Schnittstellen

- Skript-API: `qet.addFluidValve(...)`, `qet.addCylinder(...)`, `qet.setLineKind(...)`.
- MCP: Operationen `add_fluid_part`, `configure_valve`, `configure_cylinder`, `link_coil`.

---

## 8. Reihenfolge (Commits)

1. Datenmodell: Terminal `medium`, Abschluss, Leitungsart, `<fluid>`-Block lesen/schreiben + Tests (`tests/qttest`). **Erledigt** (`sources/fluid/`, `tst_fluiddata`). Noch ohne Oberfläche; Medium-Prüfung beim Verbinden folgt mit Schritt 4.
2. Baukasten-Generator + erste 40 Symbole. **Ersetzt durch Quelle C:** `tools/symbole/fluidsim_symbole.py` wandelt die FluidSIM-Bibliothek um (150 Pneumatiksymbole, Ruhestellung, Portnummern, Abschlüsse, Master/Slave). Ein eigener ISO-Baukasten bleibt offen, solange ISO 1219-1 nicht vorliegt. Offen am Umwandler: Drehrichtung der Bögen nicht gegen FluidSIM geprüft; Hydraulik/Elektrik-Bereiche nicht gesichtet; die erzeugten Bauteile sind (noch) nicht im Repo, nur das Skript.
3. Nachrüsten `50_pneumatic` (Anschlussnamen). **Erledigt:** `tools/symbole/fluid_nachruesten.py` (Medium an allen Anschlüssen, Portnummern bei 199 von 343 Bauteilen, alle 113 Wegeventile vollständig; `30_hydraulic` nur Medium, dort sind keine Portnummern gezeichnet).
4. Ventil- und Zylindereditor. **Teil erledigt:** Leitungsart im Leitungs-Fenster, Medium und Abschluss im Anschluss-Editor, Leitungen nur zwischen gleichem Medium, Schalldämpfer/Blindstopfen am freien Anschluss gezeichnet. Offen: Ventil-/Zylindereditor.
5. Verknüpfung Spule ↔ Ventil, Wegmaßstab. **Teil erledigt:** Ventil = Master, Spule = Slave mit `<fluid kind="solenoid"/>`, Magnetsymbol im Querverweis, Beispielbauteile in `bauteile/pneumatik/`. Hinweis: Als Slave zeigt die Spule die Kennzeichnung des Ventils (`-QM1`), nicht ein eigenes `-MB1` wie FluidSIM. Offen: Wegmaßstab.
6. Prüfungen, Stückliste, Kennzeichnung. **Teil erledigt:** Projekt → „Fluidtechnik prüfen…“ (offene Anschlüsse, Ventil ohne Spule, Spule ohne Ventil). Offen: Stückliste, Kennzeichnungsvorschlag.
7. Skript/MCP.
8. ZW1-Fluid-Test, danach FluidSIM-Umwandler.

Jede Änderung am bestehenden Verhalten: Einstellung im Einstellungsdialog, Standard aus (AGENTS.md).

---

## 9. Prüfung von außen

- Testplan (Zylinder + 5/2-Magnetventil + Drosselrückschlag + Endschalter) in QET und FluidSIM zeichnen, PDFs nebeneinander.
- Symbole gegen ISO 1219-1 abgleichen.
- Alte Projekte öffnen: unverändert (Regressionstest).

## 10. Offen / nicht verifiziert

- FluidSIM-Dateiformat: Aufbau bekannt (Abschnitt 1), Maßstab und Simulationswerte noch nicht geprüft.
- ISO 1219-1 Volltext.
- Nutzungsbedingungen SMC/Festo-Makros außerhalb von EPLAN.
- Ob der ZW1-Import Fluid-Seiten schon verarbeitet.
