---
name: qet-simulation
description: Pläne in QElectroTech (EPLAN-Fork) simulieren – Pneumatik, Hydraulik, elektrische Steuerung, SPS-Programme in Strukturiertem Text und GRAFCET nach IEC 60848; Zuordnungen im Plan (Spule → Ventil, Endschalter → Zylinder, Druckschalter), Kennwerte, Ereignisse, `--simulate`, `--simulate-reference`, MCP `qet_simulate`; wie der Rechenkern geprüft wird und welche Fehler schon gefunden wurden. Immer verwenden, wenn ein Plan simuliert, ein SPS-Programm oder GRAFCET für die Simulation geschrieben, die Simulation erweitert oder ihre Physik geprüft wird, auch bei „läuft der Zylinder aus?“, „schaltet die Schrittkette?“, „stimmt die Hydraulik?“.
---

# Simulation in QElectroTech (EPLAN-Fork)

Stand 15.11.2026, Fork 1.24.0. Roadmap-Stand: `tools/simulation/ROADMAP_STAND.md`. Anleitung für Anwender: `tools/simulation/README.md` (Modelltabelle, Konventionen,
Prüffälle). Hier steht, wie man damit arbeitet und was beim Bauen gelernt wurde. Neue Erkenntnisse unten unter
„Gelernt“ nachtragen – sofort, nicht erst am Ende.

## Schnellstart

```
QElectroTech --simulate plan.qet --event "0.2 -SF1 an" --event "0.4 -SF1 aus" --duration 3
QElectroTech --simulate plan.qet --event "0.5 -QM1 links an" --csv weg.csv --png bilder --at 1.0
QElectroTech --simulate-reference            # Rechenkern prüfen (Referenzläufe liegen beim Programm)
```

Per MCP: `qet_simulate` (project, events, duration, pressure, csv, png_dir, at) liefert `events`, `variables`
(SPS/GRAFCET am Ende), `program_errors`; `qet_simulate_reference` liefert jede Prüfung mit `passed`. SPS-Programm
schreiben: `qet_edit` → `set_info` key `sim_program` an der SPS-Baugruppe (Master mit E/A-Tabelle).
In QET: Anzeige → Simulation (Start/Pause/Stopp, Zeitlupe, Taster/Ventile per Maus, Liste *SPS-Variablen*).

## Zuordnungen im Plan

| Was | Wie |
|---|---|
| Ventilspule → Ventil | Kennzeichen der Spule = Ventil (`-QM1`), zwei Magnete `-QM1/14` (links), `-QM1/12` (rechts) |
| Endschalter → Zylinder | Funktionstext des Schalters „`-MM1 vorne`“ / „`-MM1 hinten`“ |
| Druckschalterkontakt → Druckschalter | Funktionstext „`-BP1`“ (Druckschalter `sensor.pressure_switch`, `p0`, `p_hyst`) |
| Zeitrelais | Zeit im Funktionstext der Spule („1 s“, „500 ms“) |
| Zähler | `comptage.elmt`, Vorwahl im Funktionstext, Kontakte `act_comptage` mit gleichem Kennzeichen, Reset `-PF1/R` |
| SPS-Kanal | verknüpfter SPS-Slave; Adresse und Richtung aus Kanal-Info `plc_address`/`plc_type`, sonst E/A-Tabelle |
| GRAFCET | Schrittnummer, Weiterschaltbedingung, Aktionstext = Kennzeichen; `-BG1`/`-QM1` direkt oder SPS-Variablen |
| Kennwerte | `<parameter>` im Symbol (`qNN`, `rT`, `onDelay`, `d_piston`, `pMax`, `V0`, `p0` …) |
| Endschalter/Rollenventil über Hub | Funktionstext „`-MM1 0..2`“ (mm, FluidSIM-Wegmarke) |
| Spule ohne Ventilkennzeichen | Verknüpfung Spule (Slave) → Ventil, Seite in `valve_side`; `-MB1` in GRAFCET-Aktion betätigt sie |
| Rolle eines E-Bauteils | Elementinformation `sim_role`: `supply+`/`supply-`, `voltage`, `button`, `switch`, `limit` (+`_nc`), `solenoid`, `grafcet_io` (+ `sim_io` „I0=-KF1;Q0=-MB1“) – vor jeder Namenserkennung |
| FluidSIM-Schaltung | Datei → FluidSIM-Schaltung importieren, `tools/fluidsim_import/fluidsim_plan.py`, MCP `qet_fluidsim_import` (Skill `fluidtechnik`) |

## Rechenmodell (Kurzfassung)

Expliziter Euler, dt = 10 µs, Netze aus den Leitungen (Union-Find, Folioverweise verbinden), Elektrik jeden Schritt,
SPS und GRAFCET alle 10 ms. Gemeinsame Integrationsschritte in `sources/simulation/simulationphysics.h` – `step()`
und der Prüfstand benutzen dieselben Funktionen, sonst prüft der Prüfstand nur sich selbst.

- Luft isotherm (R·T fest), Durchfluss ISO 6358 aus `qNN` (b = 0,4 und ρ_N = 1,1845 kg/m³ wie FluidSIM; Belege in `tools/simulation/FORMELN.md`); Druckluftbehälter = Volumen, keine Quelle.
- Öl: Kompressionsmodul 7e8 Pa, Netzvolumen 0,1 l; Widerstand `rT` als geglättete Wurzelkennlinie
  q = k·Δp/(Δp² + 0,3 bar²)^¼ (gleiche Kleinsignal-Steigung wie die frühere lineare Zone → gleich stabil).
- Netze mit Hydrospeicher: Druck **implizit** aus der Volumenbilanz (Bisektion bis zum letzten Bit).
- Unter 0,11 bar abs reißt das Öl ab: **Hohlraum bilanziert** (`cavity`), beim Nachströmen zuerst gefüllt.
- Nicht endliche Werte → NaN durchreichen, Lauf bricht ab (`failed()`), CLI Rückgabewert 3, `Model error:`.
- Freie Anschlüsse: offen = Atmosphäre, `port_end="plug"` = dichtes eigenes Volumen, Schalldämpfer = Widerstand.
- Wegeventil: Schieber als Stellglied erster Ordnung, τ = Schaltzeit/3 (C++ und Python gleich).
- Kennwerte über `paramIn` nach `unit` mit `Range` (Positive/NonNegative), `rT`/`k` über `paramNative`;
  jeder Fehler landet in `modelErrors()`, der Lauf startet nicht. `runUntil` hört bei `failed()` auf.
- Pumpe: volle Förderung bis 1 bar unter `pMax`, bei `pMax` null. Rückschlagventil öffnet stetig ab `p0`.
- Zylinder: Kraftbilanz, Haft-/Gleitreibung, harte Endanschläge; Unterdruck im Öl wird gemeldet, nicht gerechnet.
  R11: alles, was dem Ausfahren entgegenwirkt (Last/Kraftprofil, Feder, Gewicht), liefert `Cylinder::opposing()`;
  FluidSIM-Kennwerte alpha, m_ext, mu_B_ext/mu_M_ext, F_B/F_M/F_V, spring_left/right, p_spring_min/max, F_var_ex/re.
  Prüfung per Kraftdifferenz ΔpA·A_K − ΔpB·A_R bei x = 100 mm gegen den Grundfall (`hydraulik.qet`, textVariant).
  `valveAudit` prüft Wege/Ruhestellung/Betätigung aller Ventilsymbole; Vorrang `dominant` in `Valve::wanted`.
- Hydromotor (R08) über `SimPhysics::Shaft`; Schwenkantrieb (R10) ist ein `Cylinder` mit `rotary` (Winkel in rad,
  Flächen = M_6bar/6 bar, Masse = Trägheit, Wegmarken in Grad über `markPos()`).
- Leitungen (R05): `lineMode` wählt ideal / Widerstand / Rohr nach den Kennwerten; ideale Leitungen werden in
  `buildNets` verbunden (Drehdurchführung je Kanal über die Anschlusshöhe: `e->mapFromScene(t->dockConductor())`,
  nicht `t->pos()`, das ist immer 0). Prüfpläne `rohr.qet` (Bauteil -HQ1 per `textVariant` austauschbar) und
  `kupplung_drehdurchfuehrung.qet`.
- Stetigventile (R07): `PropValve`, Schieber per `SimPhysics::pt1`, Kanten per `edgeOpening`; Sollwert über
  `setSetpoint` (CLI `sollwert`, Prüfstand `Input` kind 3 mit value). Senkbrems-/Zuschaltventil, Druckwaagen (gemeinsamer Zweig
  `pressure_compensator_*`, Schließer/Öffner über die Steuerdifferenz), Schockblock (A1 = A2 in `buildNets`) und
  pneumatisches Folgeventil laufen als `FlowValve`-Zweige; Prüfpläne `druckwaagen.qet`, `folgeventil.qet`. Prüfpläne `stetig.qet` (Ventil per `textVariant` zum Stetigventil) und `lastventil.qet`
  (Anschlüsse per eindeutigem Terminal-Tag mit UUID umbenannt, nicht per erstem Treffer).
- Analoge Signale (R06): Bausteine in `simulationsignals.h` (reine Funktionen, testbar ohne Plan), im Modell
  `Analog` mit `linkAnalog` (Kennwerte, Kopplung, Auswertereihenfolge) und `analog(dt)` direkt nach `electrics()`;
  Potential je Netz über `volt()` (Analogausgang > `sim_volt` der Quelle > 24 V an Plus). Formel-IDs A00–A12 und V09
  (nicht S…, das sind Paketquellen). Prüfpläne `druckregelung.qet`, `lageregelung.qet` aus eigenen Prüfsymbolen
  (Testsammlung `90_signal`, Erzeuger `erzeuge_r06.py` im Scratchpad-Muster); Reset-Variante per `textVariant`
  auf `vMin=21.6 V` → `vMin=30 V`.
- Druckluftmotoren (R12): `Motor` mit `air` = true, gleiche `SimPhysics::Shaft`; Massenstrom ρ·D·|ω|/2π vom höheren
  Anschluss zu EX; Prüfplan `druckluftmotor.qet`, Nennpunkt per textVariant auf M_ext.
- Mehrstellungszylinder (R12, 1.34.0): normaler `Cylinder` mit `multi` = true, Schritt in `multiStep` (zwei Kolben,
  in Kontakt ein Körper, sonst getrennt, Stoß plastisch); Anschlüsse C1/C2/C3, Messgröße `x1:`; Prüfplan
  `mehrstellungszylinder.qet` mit drei Prüfsymbolen (Startlagen 0/0, 200/0, 200/100).
- Mobilhydraulik (R13, 1.35.0): `LsUnit` (LS-Aggregat, Pumpen als Quellen in P1/P2, Rücklauf/Leckage nach außen, LS-Regler als PT1), `Valve63` (6/3-Handhebelventil, Kanten rT/Anteil²), Belastungseinheit als `Cylinder` mit `loading` (vier Kammern). Prüfplan `mobilhydraulik.qet`, Varianten per textVariant auf Lastdrossel rT 1.0 → 2.0 und xStep/xStart.
- Lenkung und Sperrventile (R13, 1.36.0): `Steering` (Orbitrol, M_user > 0 = links, Lenkrad `phi:`), `Joystick` (Druckminderventile an X g.1/X g.2), Sperrblock als FlowValve (`shutoff_valve.double_non_return_valve01/02`), Belastungseinheit 1 (`cross`), Speicher mit Sicherheitsblock (`Accumulator::block`, erkannt an `p0_accu`). Pumpe sucht P, sonst P2 (vereinfachtes Aggregat) bzw. A/B (Pump01). Signale `p:Teil.Anschluss` dürfen Punkte im Anschlussnamen haben. Prüfplan `lenkung.qet`.
- Vakuum (R10): Ejektor mit stetiger linearer Saugkennlinie, Sauger mit Haltekraft gegen s·m·g, Vakuumschalter =
  Druckschalter mit negativem p0. Prüfplan `referenz/plaene/vakuum.qet`; Varianten im Prüfstand per `textVariant`
  (Parameter in den eingebetteten `<fluid>`-Block schreiben, selbstschließendes Tag beachten).
- SPS: ST-Teilmenge (BOOL/INT, IF/ELSIF, CASE, TON/TOF/CTU/CTD/R_TRIG/F_TRIG/SR/RS, R_EDGE/↑), alle Werte `double`.
- GRAFCET (`sources/simulation/grafcet.*`): IEC-60848-Schaltregeln, gleichzeitiges Schalten, flüchtige Entwicklung,
  kontinuierliche Aktionen nur in stabiler Situation; läuft vor den ST-Programmen im 10-ms-Zyklus.

## Prüfen (jede Änderung am Kern)

1. Bauen, `tools/deploy_lokal.ps1`.
2. `--simulate-reference`: alle 22 Prüfungen bestanden, kein SKIP (analytisch mit Konvergenz, OpenModelica-Vergleich
   mit Drücken, Hohlraum, Negativfälle, Einheiten, sechs `.qet`-Projekte durch `SimulationModel`). Eine neue
   Funktion bekommt ein Projekt in `tools/simulation/referenz/plaene` (Erzeuger `_test/sim/plaene_referenz.py`).
3. Gegenprobe C++ ↔ Python-Prototyp (`tools/simulation/fluid_simulation.py`) auf den Testplänen: 0 Abweichungen.
   Physik-Änderungen in beiden Programmen gleich machen, sonst zeigt die Gegenprobe nichts.
4. Testpläne (Pendel, Selbsthaltung, Hydraulik, Drossel, Zeitrelais, Stromteiler, SPS, Zähler, Schrittkette,
   GRAFCET, neue Bauteile) simulieren und das Ergebnis gegen eine Handrechnung halten.
5. Jedes Bild, das gezeigt wird, vorher durch `qet_layout_check` (alle Blätter) und eine 300-dpi-Sicht
   (Skill `qet-schaltplan-zeichnen`).
6. README (`README.md`, `README.de.md`), `tools/simulation/README.md`, `CHANGELOG-FORK.md` und diesen Skill mitziehen.

## Gelernt

- **Neue Physik braucht einen Fall, der sie an ihrer Grenze trifft.** Der Speicher-Referenzfall startete oberhalb
  des Vorfülldrucks und bestand; erst Laden ab 1 bar zeigte, dass der explizite Schritt beim Überschreiten der
  Vorfüllung überschießt (2 % zu viel Druck, Öl „aus dem Nichts“). Lösung: implizit aus der Volumenbilanz.
- **Eine Abweichung im Vergleich erst nachrechnen, dann anpassen.** Der hydraulische Rückhub war 4 % zu langsam;
  Ursache war das 5-bar-Band der Pumpe (nahm schon bei 46 bar Förderstrom weg), nicht das Ventil.
- Speicher und Druckbehälter haben `kind="source"` – nicht als feste Druckquelle behandeln.
- SPS-Kanal vor Relaiskontakt erkennen (sonst wird er als Kontakt gerechnet); der Gruppenindex der Verknüpfung
  steht am Master, die Adresse am Kanal (`plc_address`).
- Endschalter-Zuordnung über den Funktionstext, nicht über die Lage.
- Gleichzeitige Meldungen (gleiche ms) haben keine feste Reihenfolge: Vergleiche sortieren; Python-Ausgaben mit
  `PYTHONIOENCODING=utf-8` lesen, sonst zählen Umlaute als Abweichung.
- CLI- und Prüfstandsausgabe werden vom MCP-Server zerlegt (`parse_simulation_output`, `parse_reference_output`) –
  Ausgabeformat nicht ändern, ohne die Parser und ihre Tests anzupassen.
- Die QET-GRAFCET-Originalsymbole sind für saubere Pläne unbrauchbar (Nummer neben dem Kasten, Platzhalter als
  ' und _); die Fork-Symbole verwenden. Aktion ohne Leitung direkt auf den Schritt legen – die Simulation verbindet
  deckungsgleiche Anschlüsse.
- Referenzpaket (Modelica/OpenModelica): nicht als zweites Backend nachbauen, sondern seine Fälle mit denselben
  Parametern gegen den eigenen Kern rechnen; Referenz-CSVs reduziert unter `tools/simulation/referenz` mit
  SHA-256 des Originals.
- Beim Schreiben von C++/PowerShell über Shell-Heredocs gehen Backslashes verloren (`\n`, `\r`, `\20`): Patch-Skripte
  als Datei schreiben und Strings mit `chr(92)` prüfen.

- **Ein Prüfstand, der den Kern nur nachbaut, prüft nicht das Programm.** Externe Prüfung 14.11.2026: Vergleich
  ohne Drücke, falsche Referenzdaten akzeptiert, Grenzfall (Druckuntergrenze) übersprungen. Darum jetzt
  Ende-zu-Ende-Projekte durch `SimulationModel` und Gegenproben mit absichtlich falschen Daten.
- **Druckgrenzen nie mit `std::max` „reparieren“**: verliert Volumen und versteckt NaN. Bilanzieren oder abbrechen.
- **Eine falsche Erwartung im Test ist kein Kernfehler**: Blindstopfen-Zylinder fuhr trotzdem aus – nachgerechnet
  (isotherme Luftfeder 33 → 7 cm³ = 3,85 bar), Kriterium korrigiert, nicht den Kern.
- **Haltedrücke hängen an der Schieberbewegung**: sprunghaftes Schließen gab 6,3/8,8 statt 8,6/12,3 bar; mit
  Überblendung über die Schaltzeit 8,5/12,1 bar. Testparameter (30 ms = 3 τ) vorher aus der Physik festlegen.
- Offener Fluidanschluss hat 0 bar Überdruck, nicht „nicht definiert“ (`pressureBar`).

- **Mittelwerte decken Spitzen nicht ab**: zweite Prüfung 15.11.2026 schob eine Druckspitze ein, die Fenster-
  mittel nicht sahen. Ganze Kurven im Schlauch vergleichen, in beide Richtungen. Der Schlauch fand dann echte
  Unterschiede: 7 ms späterer Anschlag (linearer statt exponentieller Schieber → Schieber erster Ordnung) und
  das Prellen des elastischen Referenzanschlags (bleibt als offen genannte Ausnahme von 10 ms).
- **Prüfen vor Umrechnen**: Durchmesser vor dem Quadrieren, Einheit vor dem Wert. Jede Kennwert-Lesestelle mit
  Bereich; keine stillen Vorgaben bei unlesbaren Werten.
- **Fehlerzustand in jeder Schleife abfragen**: `step()` ohne Zeitfortschritt + `while (t < ziel)` = Endlosschleife.
- Gegenproben des Prüfers (Manipulationen) im eigenen Prüfstand nachbilden, nicht nur seine Dateien einmal laufen lassen.

- **Fremde Pläne über die Masse prüfen**: alle 1136 FluidSIM-Beispiele importieren und simulieren
  (`fluidsim_batch.py`) fand in einem Lauf, was Einzeltests nie zeigten: gepackte Koordinaten (357 Dateien ohne
  Schaltwege), andere Klassennamen (Drosselrückschlagventil lief als „nichts“ = gesperrt), Steueranschluss „12“
  am 3/2-Ventil, Rollenventile, Wahlschalter ohne Rückweg. Ergebnis je Schaltung als CSV, nach Ursachen gruppieren.
- **Plausibel heißt nicht richtig**: 100 mm Hub in 27 ms war rechnerisch möglich (1000 l/min, 0,14 kg), aber
  FluidSIMs Reibungsmodell (F_ref bei v_ref) fehlte. Geschwindigkeiten gegen die Erfahrung halten (≈ 1 m/s).
- **Erkennung über Namen bricht bei fremden Bibliotheken**: Quellen, Taster, Spulen aus FluidSIM wurden nicht
  erkannt. Darum `sim_role` – der Import sagt, was ein Bauteil ist, statt dass der Kern es rät.
- **Modell nur aus dem Katalog** (R00, `simulationcatalog.cpp`): neue Fluidfunktion = neuer Katalogeintrag mit Rolle,
  Modell, Grenzen und Prüfung; ohne Eintrag startet der Lauf nicht. Nie wieder aus `kind` raten („source“ wurde
  sonst zur Druckquelle, auch Kühler und Vakuumerzeuger). Pneumatischer `supply.pump` = Verdichter.
- **Messgrößen** (R01): `signalList()/signalValue()`, CLI `--signals/--measure/--raster/--meta`; Durchflüsse nur mit
  `setRecordFlows(true)` (sonst NaN, keine erfundenen Nullen); Abgriff darf den Lauf nicht ändern (Prüfung).
- **OpenModelica 1.26.3** ist installiert (`C:\Program Files\OpenModelica1.26.3-64bit\bin\omc.exe`). Neue
  Referenzläufe mit eigenem Skript neben `tools/simulation/referenz` (Muster `erzeuge_hydraulik_fein.py`: Modelle aus
  dem Referenzpaket in einen Temp-Ordner, `simulate(...)` mit `variableFilter`, reduzierte CSV + Herkunfts-JSON).
- **Zeitversatz von Vorgängen sauber trennen**: Unser Kolben kam 0,75 ms früher am Anschlag an (Geschwindigkeit
  +0,23 % über 0,67 s). Für den Kontakt selbst beide Läufe an der Ankunft ausrichten und den Versatz eigens begrenzen;
  die ganzen Kurven bleiben in absoluter Zeit verglichen.
- **Schlauch mit verbundenen Punkten**: Abtastpunkte allein lassen schnelle Abfälle zwischen zwei Punkten als
  Abweichung erscheinen. Gegenkurve stückweise linear; danach die Spitzen-Gegenprobe wiederholen.
- **Bilanz statt Tautologie**: ΔEkin gegen die Arbeit aus der Geschwindigkeitsänderung prüft nichts. Kontaktbilanz
  −W_Kontakt = ΔE_Feder + ∫c·v²dt prüft Modell und Integration wirklich.
- Der Python-Prototyp kennt die FluidSIM-Erweiterungen (`sim_role`, Wahlschalter, Hubbereich-Auslöser, Wechsler,
  `pNom`, `F_ref`/`v_ref`) nicht; die Gegenprobe läuft nur auf Plänen ohne sie. Neue Physik dort nachziehen, sobald
  ein Testplan sie benutzt.
- Batch-Läufe nicht während eines Deploys starten: der Lauf benutzt `build/deploy/QElectroTech.exe`; bauen geht
  (`build/`), deployen erst danach.

- **Sonderfall weg, Kennlinie stetig**: Ein Ejektor-Rückströmterm „nur ohne Versorgung“ verfälschte das Vakuum bei
  Teilversorgung (−0,35 statt −0,56 bar). Die lineare Saugkennlinie hat für s → 0 von selbst den Rückweg zur Luft.
- **Prüfpläne aus der Fork-Sammlung**: `bauteile/fluid` liegt außerhalb des MCP-Arbeitsbereichs; für `qet_edit` in
  `Schaltplaene/_test/fluid_elements` kopieren und als `elements_dir` (`common://`) angeben.

- **Formelbeleg**: Jede neue Formel des Kerns bekommt vor der Freigabe eine Zeile in `tools/simulation/FORMELN.md`
  (Codestelle, Herkunftsart, F-/S-ID des Roadmap-Quellenregisters, Grenze, Prüfung). Gewählte Vorgaben als
  Projektdefinition kennzeichnen, nie als Messwert oder Literaturwert ausgeben.

## Offen

Schieberdynamik mit Überdeckung/Kennlinie (heute lineare Überblendung), thermische Pneumatik, Leitungsträgheit,
Kavitation, Proportionalventile, Greiferkinematik, Abblasen am Ejektor, analoge SPS-Werte, Makroschritte/Zwangssteuerung im GRAFCET,
Vergleich mit FluidSIM, Simulationsfenster im laufenden QET mit Bedienung angesehen.
