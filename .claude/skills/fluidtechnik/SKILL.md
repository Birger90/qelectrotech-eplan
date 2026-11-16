---
name: fluidtechnik
description: Pneumatik- und Hydraulikpläne in QElectroTech (EPLAN-Fork) nach Vorbild FluidSIM 6 – Bauteile mit Medium und Portnummern, Schalldämpfer/Blindstopfen, Leitungsarten, Spule ↔ Ventil und Relais ↔ Kontakt verknüpfen, Kennbuchstaben (QM, MM, MB …), Prüfung offener Anschlüsse, FluidSIM-Symbole und -Schaltungen (.circ) übernehmen. Immer verwenden, wenn ein Pneumatik-, Hydraulik- oder Elektropneumatikplan in QET entsteht, geprüft oder aus FluidSIM übernommen wird, bei Fragen zu Ventilen, Zylindern, Wegeventil-Anschlüssen (1, 2, 3, 4, 5, 12, 14) oder zum FluidSIM-Dateiformat.
---

# Fluidtechnik (Pneumatik, Hydraulik) in QElectroTech

Stand 14.11.2026, Fork ab 1.13.0 (`main`). Was hier steht, ist gebaut und geprüft; Unbelegtes ist als offen
markiert. Plan und Format: `PLAN-PNEUMATIK.md`. Neue Erkenntnisse unten unter „Gelernt“ nachtragen.
**Simulation** (Physik, SPS, GRAFCET, Prüfstand): eigener Skill `qet-simulation`.

Ein QET ohne die Fluid-Funktionen dieses Forks lädt die Bauteile, behandelt alle Anschlüsse aber als elektrisch und
zeichnet keine Schalldämpfer.

## Datenmodell (sources/fluid/)

| Wo | Attribut | Werte |
|---|---|---|
| `<terminal>` im .elmt | `medium` | `pneumatic`, `hydraulic`, `mechanic`, `logic`; fehlt = elektrisch |
| `<terminal>` | `port_end` | `silencer`, `plug`; wird am freien Anschluss gezeichnet, verschwindet mit Leitung |
| `<terminal>` | `name` | Portnummer: `1`, `2`, `4`, `12` …; doppelte als `1.1`, `1.2` (QET verlangt eindeutige Namen) |
| Leitung | `fluid_line` | `working`, `pilot`, `exhaust`; setzt im Leitungs-Fenster auch das Strichmuster |
| `<definition>` | `<fluid kind=…>` | `valve`, `cylinder`, `source`, `treatment`, `flow_control`, `logic_valve`, `sensor`, `solenoid`, `other` |
| `<fluid>` | `prefix` | Kennbuchstaben-Vorschlag beim Einfügen, wenn `qet_labels.xml` für den Ordner keinen hat |

- Eine Leitung verbindet nur gleiches Medium (`Terminal::canBeLinkedTo`); zwei elektrische immer.
- Ventil: `<positions initial="2"><position/>…</positions>`, `<actuation side="left" types="solenoid,manual" spring="true" pilot="true"/>`.
  Zylinder: `<cylinder action="single|double" spring="rod|piston" bore_mm= rod_mm= stroke_mm=/>`.
- Ventil mit Magnet = **Master** (`kindInformation type=coil`, `max_slaves` = Zahl der Magnete),
  Ventilspule = **Slave** (`state=Other`, `<fluid kind="solenoid"/>`). Der Querverweis am Ventil zeigt die
  Spule mit dem Magnetsymbol (Kasten mit Diagonale). Relaisspule = Master, Kontakte = Slave (NO/NC/SW).
- Eine Spule als Slave zeigt die Kennzeichnung ihres Masters (`-QM1`); ihre eigene (`-MB1`) bleibt in den Daten.

## Bauteile

| Ordner der Standardsammlung | Inhalt |
|---|---|
| `50_pneumatic/…`, `30_hydraulic/…` | die 437 QET-Symbole, Grafik unverändert, mit Simulationsdaten: benannte Anschlüsse mit Medium, `<fluid function=…>`, Stellungen und Schaltwege aller 172 Wegeventile und 30 Ventilbausteine, Betätigung, Zylinderdaten, Kennwerte als `<parameter>` (Ablage `bauteile/fluid`, gleiche Pfade; Generator `tools/symbole/qet_fluid_daten.py`) |
| `20_logic/2020_flow_chart/202020_grafcet` | GRAFCET-Symbole, im Raster neu gezeichnet (`bauteile/grafcet`, `tools/symbole/grafcet_symbole.py`) |
| `bauteile/pneumatik/` (Repo) | Beispiel: 5/2-Magnetventil (Master) und Ventilspule (Slave) |

FluidSIM-Symbole gehören nur in die Firmensammlung, nie in den öffentlichen Fork (Lizenz). `tools/deploy_lokal.ps1`
legt `bauteile/fluid` und `bauteile/grafcet` über die Sammlung. Ältere Werkzeuge von Hand:

    python tools/symbole/fluid_nachruesten.py elements/50_pneumatic --ziel <Sammlung>/50_pneumatic
    python tools/symbole/fluidsim_symbole.py --fluidsim "C:/Program Files (x86)/Didactic/FluidSIM6" --bereich ~FolderPneumatic --ziel <Ordner>

Portnummern (so in den FluidSIM-Symbolen und QET-Ventilen): `1` Versorgung, `2`/`4` Arbeitsanschlüsse,
`3`/`5` Entlüftung, `12`/`14` Steueranschlüsse (stehen an der Betätigungsseite, nicht oben/unten).

## Kennbuchstaben (Vorschlag, aus FluidSIMs eigener Kennzeichnung gezählt)

Gezählt über alle 1072 FluidSIM-Beispiele (`kennbuchstaben()` in `fluidsim_symbole.py`):

| Bauart | Buchst. | Anteil |
|---|---|---|
| Zylinder | MM | 98–100 % |
| 5/2-, 5/3-Wegeventil | QM | 90 % |
| 3/2-, 4/2-Wegeventil | SJ | 55 % (je nach Rolle auch QM, BG) |
| Ventilspule | MB | 98 % |
| Drosselrückschlagventil | RZ | 98 % |
| Rückschlagventil | RM | 95–100 % |
| Wechsel-, Zweidruckventil, Schnellentlüftung | KH | 87–100 % |
| Druckregel-, Druckbegrenzungs-, Zuschaltventil | QN | 95–100 % |
| Manometer | PG | 86–100 % |
| Drucksensor | BP | 97–100 % |
| Näherungsschalter | BG | 100 % |
| Wartungseinheit | AZ | 100 % |
| Speicher, Tank | CM | 100 % |
| Druckluftquelle, Pumpe | GP | 75–100 % |
| Filter (Hydraulik) | HQ | 100 % |
| Vakuum | UQ | 100 % |
| Relais | KF | 99 % |
| Taster | SF | 86–100 % |

Ordnervorgaben: `tools/symbole/qet_labels/50_pneumatic.xml`, `30_hydraulic.xml` (Deploy legt sie
nur an, wenn noch keine `qet_labels.xml` da ist). ISO/IEC 81346-2 selbst liegt nicht vor.

## Zeichnen per MCP / Skript

- Bauteile: `add_element` mit `common://50_pneumatic/5090_FluidSIM/Pneumatic/Directional_valves/Solenoid_operated/567199.elmt` usw.
- Anschlüsse über ihre UUID ansprechen (`from_terminal: "{…}"`), nicht über den Index.
- **Verknüpfen immer vom Slave aus**: `link_elements` mit `element` = Spule/Kontakt, `to` = Ventil/Relais.
  Andersherum ersetzt jede weitere Spule die vorige (QETs linkElements setzt die Verknüpfungen des
  ersten Elements auf genau das zweite).
- Leitungsart (`fluid_line`) kann `set_conductor` noch nicht setzen; im Leitungs-Fenster von Hand.
- Prüfen: Menü Projekt → „Fluidtechnik prüfen…“ (offene Fluidanschlüsse ohne Leitung/Abschluss,
  Magnetventil mit weniger Spulen als Magneten, Spule ohne Ventil).

## FluidSIM-Schaltungen übernehmen

    python tools/fluidsim_import/fluidsim_plan.py SCHALTUNG.circ AUSGABE.qet
    python tools/fluidsim_import/fluidsim_plan.py ORDNER_MIT_CIRC AUSGABE.qet   # jede .circ ein Blatt

Übernimmt Bauteile mit Lage (Drehung, Spiegelung), Kennzeichnung, deutsche Namen, sichtbare Werte als
Text, Zylindermaße, Leitungen (Verzweigungen werden Leitungen am selben Anschluss), Schalldämpfer/
Blindstopfen, Spule ↔ Ventil und Relais ↔ Kontakte (auch über Blätter). Läuft über QETs Skript-
Schnittstelle (`misc/qet-mcp/qet_mcp.py`, setzt `QET_ENABLE_SCRIPTING=1`).

Seit 1.22.0 mit **eigener Grafik** (`tools/fluidsim_import/eigene_grafik.py`: Ventilkästen aus den Schaltwegen,
Betätigungen aus den Ebenennamen, Zylinder, Quelle, Tank, Messgerät, Rückschlag-/Drossel-/Absperrventil längs der
Anschlussachse, Druck- und Logikventile, Kontakte, Spulen, Leuchten, Potentiale; sonst Kasten mit Klassenname).
Aus der .circ nur Anschlusslagen und Daten – jeder kann übernehmen, ohne FluidSIM und ohne Festo-Grafik im Projekt.
`--fluidsim-grafik` nimmt die Originalgrafik (nur Eigengebrauch mit Lizenz). Und sofort simulierbar (auch: Datei → „FluidSIM-Schaltung importieren (.circ)…“, MCP
`qet_fluidsim_import`, Gesamtprüfung `tools/fluidsim_import/fluidsim_batch.py --ausgabe ORDNER`; 15.11.2026: 1136/1136 übernommen, 0 Modellfehler, Zylinder bewegt sich in 276/461):
- **Schaltwege** je Stellung aus den Stellungsbausteinen `B<n>_<nr>` des Ventilsymbols (nach `ref-x`),
  Anschlussnamen aus `CON<n>`; gespiegelt (`scale-x=-1`) oder 180° gedreht → Stellungen, Ruhestellung
  und Seiten tauschen (Klick links = linke Hälfte der Zeichnung).
- **Kennwerte** der Schaltung (`<parameter>`/`<const>` mit Einheit) in den `<fluid>`-Block; FluidSIM-
  Klassen unter dem Namen des Simulationskerns (`ALIAS`/`UMBAU` in `fluidsim_plan.py`), z. B.
  `ThrottleCheckValve` → `one_way_flow_control_valve` (frei Anschluss 2 → 1 = Abluftdrosselung, 304 von
  334 Beispielen mit Anschluss 1 am Zylinder), `level` = Öffnung in %, `MPa.min2/l2` → ×10 bar.
- **Elektrik** über Elementinformation `sim_role`: `supply+`/`supply-` (Pot24V/Pot0V/Ground), `voltage`,
  `button`/`switch` (mit Rastung = Wahlschalter)/`_nc`, `limit` (+ `function` „-MM1 0..2“ aus dem
  Wegmaßstab `Mech.FS3.DistanceRule`: label1..6, x11/x12 … in mm, am nächstgelegenen Zylinder),
  `solenoid` (+ `valve_side`), `grafcet_io` (+ `sim_io` „I0=-KF1;Q0=-MB1“ aus LabelIn/LabelOut).
  Rollenhebelventile bekommen dieselbe Wegmarke als `function`.
- **Magnet ↔ Ventil**: neben `link-props/target` steht die Spulenkennzeichnung in der Betätigungsebene
  `AEML…`/`AEMR…` als `<connector type="electric-label" designation="-MB1">`.
- **GRAFCET** (`fluidsim_grafcet.py`): Grafcet.Step (flags initial), Transition (condition: `*`→`·`,
  `+`→`∨`, `RE`→`↑`, `.signal` weg), Action (actionType 0 N, 1 bedingt, 2/3 speichernd bei Aktivierung/
  Deaktivierung, 4 beim Auslösen ≈ 2, 5 bei Ereignis, 6 Zwangssteuerung – nicht), MultiPin/SyncRule.
Nicht übernommen: Rahmen, Tabellen, Zustandsdiagramme, Digitaltechnik (LOGO), Teil-GRAFCETs,
Artikeldaten (FluidSIM speichert keine), Leitungswege (QET legt sie neu).

## FluidSIM-Format (Kurzfassung, ausführlich in PLAN-PNEUMATIK.md)

- `.circ`, `.sym`, `.lib`, `db/lang.db`: gzip-gepacktes XML. Installation `C:\Program Files (x86)\Didactic\FluidSIM6`.
- Symbol: `<symbol>` → `<symbol-layer ident ref-x ref-y scale-x>` → `line` (fehlendes x2/y2 = wie x1/y1),
  `polyline`, `rect`, `circle`, `arc`, `text`, `<connector type="pneumatic" designation="4" cx cy>`.
  Koordinaten relativ zur Mitte des Objekts (`x1 + width/2`). Raster 12 = QET 10 (Faktor 10/12).
- Bauteil: `<component object-ref="Pneu.DirectionalValve.FivePins" rotation scale-x>` mit
  `dki-data/designation`, `classification-data/type` (`solenoid-coil`, `make-contact` …), Parametern.
  **Drehung gegen den Uhrzeigersinn** (an 993 Leitungen gemessen). Bauteile können in `<group>` stecken.
- Leitung: `<connect con1="Oxx.C4" con2="Oyy.A" points="relativ zu con1">`; `C4` → `con-ref` → `Ebene.Ident`.
- Verzweigung: `Interface.MultiPin`. Schalldämpfer: eigenes Bauteil, Anschluss liegt auf dem Ventilanschluss.
- Spule ↔ Ventil: am Ventilanschluss `link-props/type = solenoid-valve-coil-target`, `target value="-MB1" comp-ref`.
- **Gepackte Koordinaten** (357 ältere Beispiele): `<line>` / `<polyline count=n>` ohne Attribute, Text =
  Hex von little-endian-Doubles, erst alle x, dann alle y (Linie: x1, x2, y1, y2). `fluidsim_symbole.entpacken`
  schreibt sie in Attribute um – ohne das stehen alle Linien auf 0 und kein Ventil hat Schaltwege.
- Anschlüsse ohne `designation` (Zylinder, Quellen, Spulen, GRAFCET-SPS): Name = Anschlussname am Bauteil
  (`<connector ident="A" con-ref="CON1.1">`), so heißen Zylinderanschlüsse A/B und SPS-Anschlüsse I0/Q0.
- Pneumatische Betätigung heißt `APNEUL`/`APNEUR` (wie `PLT…` Vorsteuerung), mechanische `AMECHL/R` (Rolle).
  3/2-Ventile nennen ihren einzigen Steueranschluss oft „12“ (ISO 5599: verbindet 1 mit 2).
- Zylinder: `F_ref`/`v_ref` (auto_friction) = viskose Reibung F_ref/v_ref; ohne sie fährt ein leichter
  Zylinder an einem 1000-l/min-Ventil unrealistisch schnell (100 mm in 27 ms statt ≈ 130 ms).

## Gelernt

- SVG-Export zeichnet die Bauteilgrafik mit Faktor 0,75, Anschlüsse und Leitungen nicht: Leitungen enden
  im SVG neben dem Symbol. Zum Ansehen PDF oder PNG exportieren (`--export-pdf`, `--export-png`).
- Bewegliche Texte (`dynamic_text`) rückt QET beim Einfügen von der Grafik weg; Portnummern deshalb als
  feste `<text>` (y = Grundlinie, `size`).
- Portnummern der QET-Ventile stehen links neben ihrer Linie: Zuordnung Text → Anschluss nur gemeinsam
  (kleinste Abstandssumme), sonst bekommt Anschluss 5 die „1“.
- Manche FluidSIM-Symbole zeichnen nur in einer ausgeblendeten Ebene (Spule im Leiterdiagramm).
- `qet_mcp.py` aus 1.9.0 hatte einen Syntaxfehler (unmaskierte Anführungszeichen); behoben in f726acb7a.
- Vakuum und Schwenkantriebe (Fork 1.27.0, R10): FluidSIM `VacuumGenerator_05H/_05L` → `vacuum.vacuum_generator_05_h/_l`
  (Anschluss 1 Versorgung, 2 Vakuum), `VacuumSuctionCup` → Sauger (Haltekraft gegen s·m·g), `VacuumSwitch` →
  Druckschalter mit negativem p0, `SemiRotaryMotor2` → Schwenkantrieb; dessen eingebaute Sensoren (label1–3,
  x11–x32 in Grad) werden Wegmarken „-MM1 89..91“ (±1°), ohne FluidSIM-Kennzeichnung bekommt er -MM n.
  `VacuumEfficiencyValve` (Sparventil) hat noch kein Modell.
- Leitungen und Kupplungen (Fork 1.28.0, R05): FluidSIM `Hyd.Supply.HoseLine` kommt als `supply.hose_line` mit `rT`
  (Widerstand); eine Leitung mit `length` und `d_inner` rechnet als Rohr. Schnellkupplungen tragen `coupled`/`seal`
  (Sammlung: „…offen“ = getrennt; Schnellanschluss ohne, Schnellkupplung mit Rückschlagventil). Verstopfte Filter über
  `clogged`.

## Offen

Ventil- und Zylinder-Editor (Symbol zur Laufzeit), Wegmaßstab, Stückliste mit Schläuchen,
`fluid_line` in Skript/MCP, ISO 1219-1 und 81346-2 im Wortlaut. Simulation: siehe `qet-simulation`.
