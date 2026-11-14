---
name: qet-simulation
description: Pläne in QElectroTech (EPLAN-Fork) simulieren – Pneumatik, Hydraulik, elektrische Steuerung, SPS-Programme in Strukturiertem Text und GRAFCET nach IEC 60848; Zuordnungen im Plan (Spule → Ventil, Endschalter → Zylinder, Druckschalter), Kennwerte, Ereignisse, `--simulate`, `--simulate-reference`, MCP `qet_simulate`; wie der Rechenkern geprüft wird und welche Fehler schon gefunden wurden. Immer verwenden, wenn ein Plan simuliert, ein SPS-Programm oder GRAFCET für die Simulation geschrieben, die Simulation erweitert oder ihre Physik geprüft wird, auch bei „läuft der Zylinder aus?“, „schaltet die Schrittkette?“, „stimmt die Hydraulik?“.
---

# Simulation in QElectroTech (EPLAN-Fork)

Stand 14.11.2026, Fork 1.20.0. Anleitung für Anwender: `tools/simulation/README.md` (Modelltabelle, Konventionen,
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

## Rechenmodell (Kurzfassung)

Expliziter Euler, dt = 10 µs, Netze aus den Leitungen (Union-Find, Folioverweise verbinden), Elektrik jeden Schritt,
SPS und GRAFCET alle 10 ms. Gemeinsame Integrationsschritte in `sources/simulation/simulationphysics.h` – `step()`
und der Prüfstand benutzen dieselben Funktionen, sonst prüft der Prüfstand nur sich selbst.

- Luft isotherm (R·T fest), Durchfluss ISO 6358 aus `qNN` (b = 0,3); Druckluftbehälter = Volumen, keine Quelle.
- Öl: Kompressionsmodul 7e8 Pa, Netzvolumen 0,1 l; Widerstand `rT` als geglättete Wurzelkennlinie
  q = k·Δp/(Δp² + 0,3 bar²)^¼ (gleiche Kleinsignal-Steigung wie die frühere lineare Zone → gleich stabil).
- Netze mit Hydrospeicher: Druck **implizit** aus der Volumenbilanz (Bisektion bis zum letzten Bit).
- Unter 0,11 bar abs reißt das Öl ab: **Hohlraum bilanziert** (`cavity`), beim Nachströmen zuerst gefüllt.
- Nicht endliche Werte → NaN durchreichen, Lauf bricht ab (`failed()`), CLI Rückgabewert 3, `Model error:`.
- Freie Anschlüsse: offen = Atmosphäre, `port_end="plug"` = dichtes eigenes Volumen, Schalldämpfer = Widerstand.
- Wegeventil: Schieber bewegt sich über die Schaltzeit (lineare Überblendung der Stellungen).
- Kennwerte über `paramIn` nach `unit` (`simulationunits.h`); unbekannte Einheit = Modellfehler.
- Pumpe: volle Förderung bis 1 bar unter `pMax`, bei `pMax` null. Rückschlagventil öffnet stetig ab `p0`.
- Zylinder: Kraftbilanz, Haft-/Gleitreibung, harte Endanschläge; Unterdruck im Öl wird gemeldet, nicht gerechnet.
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

## Offen

Schieberdynamik mit Überdeckung/Kennlinie (heute lineare Überblendung), thermische Pneumatik, Leitungsträgheit,
Kavitation, Hydromotoren, Proportionalventile, analoge SPS-Werte, Makroschritte/Zwangssteuerung im GRAFCET,
Vergleich mit FluidSIM, Simulationsfenster im laufenden QET mit Bedienung angesehen.
