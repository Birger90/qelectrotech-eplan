# Simulation von Pneumatik, Hydraulik und Steuerung

QElectroTech simuliert ein Projekt mit den Simulationsdaten der Fluidsymbole (`<fluid>`: Stellungen, Schaltwege,
Betätigung, Kennwerte) und der elektrischen Steuerung.

- **In QET:** Menü *Anzeige → Simulation*. *Start* rechnet in Echtzeit (oder Zeitlupe). Leitungen unter Druck werden
  blau (Luft) bzw. rot (Öl) eingefärbt, stromführende Drähte rot; neben Zylindern stehen Weg und Richtung, neben
  Wegeventilen die Stellung. Taster und Ventile bedient man im Fenster (Knopf gedrückt halten) oder direkt im
  Plan: Maustaste auf dem Taster gedrückt halten; auf einem Wegeventil betätigt ein Klick in die linke bzw. rechte
  Hälfte diese Seite (Handhilfsbetätigung). *Stopp* entfernt die Anzeige; das Projekt wird nicht verändert.
- **Ohne Oberfläche:** `QElectroTech --simulate plan.qet --event "0.2 -SF1 an" --event "0.3 -SF1 aus"
  --duration 2 --csv weg.csv` gibt alle Ereignisse mit Zeit aus (Ventil schaltet, Endlage erreicht, Relais zieht
  an …). Ventil von Hand: `--event "0.2 -QM1 links an"`. Mit `--png ordner --at 0.5` entstehen Bilder aller
  Blätter mit der Simulationsanzeige zu diesem Zeitpunkt.
- **Prototyp in Python** (Referenz, mit Diagramm): `tools/simulation/fluid_simulation.py`.

## Was gerechnet wird

| Bauteil | Modell |
|---|---|
| Leitungsnetz | Knoten mit Volumen; Luft isotherm (ideales Gas), Öl mit Kompressionsmodul. Freier Anschluss: offen = ins Freie, Blindstopfen (`port_end="plug"`) = verschlossen (eigenes, dichtes Volumen), Schalldämpfer = über dessen Widerstand ins Freie |
| Unterdruck im Öl | unter 0,11 bar abs reißt das Öl ab: der fehlende Inhalt wird als Hohlraum bilanziert und beim Nachströmen zuerst wieder gefüllt (Meldung „Hohlraum“); Dampfdruck und Ausgasen werden nicht gerechnet |
| Druckluftquelle | hält den Versorgungsdruck (Fenster: *Versorgungsdruck*) |
| Hydraulikpumpe | Verdrängung × Drehzahl, eingebaute Druckbegrenzung: volle Förderung bis 1 bar unter `pMax`, bei `pMax` null |
| Hydrospeicher (`supply.accumulator`, Öl) | Gasvolumen `V0`, Vorfülldruck `p0`, Polytropenexponent `k`; unter dem Vorfülldruck leer. Der Druck im Netz wird implizit aus der Volumenbilanz gelöst, die Ölmenge bleibt bei jedem Zeitschritt erhalten |
| Druckluftbehälter (`supply.accumulator`, Luft) | Volumen `V0` im Netz, Anfangsdruck `p0`; ein endlicher Vorrat, keine Quelle |
| Druckschalter (`sensor.pressure_switch`) | schaltet bei `p0` ein, `p_hyst` darunter aus; Kontakte mit Funktionstext „`-BP1`“, im GRAFCET `-BP1` |
| Wechselventil, Zweidruckventil | Ausgang 2 vom Eingang mit höherem (ODER) bzw. niedrigerem Druck (UND) |
| Zeitverzögerungsventil | schaltet 1 → 2, wenn Steueranschluss 3 `t_delay` lang Druck hat; sonst entlüftet 2 |
| Tank | drucklos |
| Wegeventil | Schaltwege der Stellung; Luft nach ISO 6358 aus `qNN`, Öl mit Widerstand `rT` (Δp = rT·Q², um Δp = 0 stetig geglättet: Q ∝ Δp/(Δp² + 0,3 bar²)^¼); Schaltzeiten `onDelay`/`offDelay`: der Schieber bewegt sich in dieser Zeit, die Wege der alten Stellung schließen, die der neuen öffnen (linear); Feder in die Ruhestellung, ohne Feder bleibt die Stellung; Steuerdruck an 14/12 |
| Rückschlag-, Drosselrückschlagventil | freie Richtung `free_from` → `free_to` (aus der Zeichnung); öffnet stetig ab dem Öffnungsdruck `p0`; entsperrbar über Steueranschluss |
| Drossel, Absperrventil, Schalldämpfer, Schnellentlüftung | Leitwert aus `qNN` bzw. `rT` |
| Druckbegrenzung, Druckregler | Einstelldruck `p0`, Öffnungsbereich `dpEnd`; Druckbegrenzung von Eingang 1 (P) in den Rücklauf 2 (T), ohne Rücklaufleitung ins Freie, öffnet nach dem Unterschied Eingang – Rücklauf |
| Zylinder | Kraftbilanz: Drücke × Flächen, Masse, Haft-/Gleitreibung, Feder, Last, Endanschläge; meldet Unterdruck im Öl (Kavitation wird nicht gerechnet) |
| Spannungsquelle | oberer Anschluss Plus, unterer Minus |
| Taster, Endschalter | Schließer/Öffner aus dem Symbol |
| Relais | Spule (Master) und verknüpfte Kontakte (Slaves), Anzug 10 ms, Abfall 8 ms |
| Zeitrelais | ansprech- bzw. abfallverzögerte Spule oder Kontakte; Zeit im Funktionstext der Spule („1 s“, „500 ms“) |

| Zähler | Zählersymbol zählt jede steigende Flanke; Vorwahl im Funktionstext („3“); Kontakte mit gleichem Kennzeichen schalten bei Erreichen; Rücksetzen über ein zweites Zählersymbol `-PF1/R` |
| SPS | Kanäle (verknüpfte SPS-Slaves) lesen bzw. schreiben ihre Adresse aus der E/A-Tabelle (z. B. `DI0`, `DQ0`); Programm alle 10 ms |

## SPS-Programm

Das Programm steht an der SPS-Baugruppe (Element-Information `sim_program`) und wird im Simulationsfenster mit
*SPS-Programm …* bearbeitet. Sprache: Teil von Strukturiertem Text (IEC 61131-3):

```
VAR T1 : TON; Z1 : CTU; END_VAR
Lauf := (DI0 OR Lauf) AND NOT DI1 AND NOT T1.Q AND NOT Z1.Q;   (* Selbsthaltung *)
T1(IN := DI2, PT := T#1s);
Z1(CU := DI2, R := DI1, PV := 3);
DQ0 := Lauf;
```

Ablaufsteuerungen (statt GRAFCET) als Schrittkette mit `CASE`, hier A+ B+ A- B- mit zwei Zylindern
(Testplan `test_schrittkette`):

```
VAR Schritt : INT := 0; F1 : R_TRIG; END_VAR
F1(CLK := DI0);                                   (* Start: steigende Flanke *)
CASE Schritt OF
  0: IF F1.Q AND DI1 AND DI3 THEN Schritt := 1; END_IF;
  1: IF DI2 THEN Schritt := 2; END_IF;            (* A+ bis -MM1 vorne *)
  2: IF DI4 THEN Schritt := 3; END_IF;            (* B+ *)
  3: IF DI1 THEN Schritt := 4; END_IF;            (* A- *)
  4: IF DI3 THEN Schritt := 0; END_IF;            (* B- *)
END_CASE;
DQ0 := Schritt = 1 OR Schritt = 2;
DQ1 := Schritt = 2 OR Schritt = 3;
```

Variablen sind die Adressen und Symbole (Funktionstext) der E/A-Tabelle; Merker brauchen keine Deklaration.
Werte sind Zahlen (FALSE = 0, TRUE = 1, Zeiten in Sekunden); `VAR` deklariert Bausteine, `INT`/`BOOL` und
Startwerte (`Schritt : INT := 0`).

| Sprachmittel | Umfang |
|---|---|
| Anweisungen | `:=`, `IF … THEN … ELSIF … ELSE … END_IF`, `CASE … OF 1: … 2, 3: … 4..6: … ELSE … END_CASE` |
| Operatoren | `NOT`, `AND` (`&`), `XOR`, `OR`, `= <> < > <= >=`, `+ - * /`, `MOD`, Klammern |
| Konstanten | `TRUE`, `FALSE`, Zahlen, Zeiten `T#1s`, `T#500ms`, `T#1m30s` |
| Bausteine | `TON`, `TOF` (IN, PT → Q, ET), `CTU` (CU, R, PV → Q, CV), `CTD` (CD, LD, PV → Q, CV), `R_TRIG`, `F_TRIG` (CLK → Q), `SR` (S1, R → Q1), `RS` (S, R1 → Q1) |

Programme aller SPS-Baugruppen eines Projekts teilen sich die Variablen. Das Simulationsfenster zeigt sie
während des Laufs in der Liste *SPS-Variablen* (Ein- und Ausgänge, Merker, Schritt, Bausteinausgänge);
`--simulate` gibt am Ende die Werte aus.

## GRAFCET (IEC 60848)

Ein GRAFCET wird mit den GRAFCET-Symbolen der Standardsammlung gezeichnet (*Logik → Ablaufplan → GRAFCET*; die Fork
liefert sie neu gezeichnet: Schrittkasten 40 × 40 mit mittiger Nummer, Anschlüsse im 10er-Raster auf der Kante, Zweige
im Abstand 200, Generator `tools/symbole/grafcet_symbole.py`, Ablage `bauteile/grafcet`) und
in der Simulation nach IEC 60848 ausgeführt, alle 10 ms (zusammen mit der SPS, vor deren Programm). Aktive
Schritte bekommen im Plan eine Marke (Punkt und grüner Rahmen), die Variablenliste zeigt `X1`, `X1.T` usw.

| Symbol | Eintrag | Bedeutung |
|---|---|---|
| Anfangsschritt, Schritt | Kennzeichen = Schrittnummer (`0`, `1`, …) | Variable `X1` (aktiv), `X1.T` (Aktivzeit in s) |
| Transition | Kennzeichen = Weiterschaltbedingung | z. B. `-BG2`, `↑-SF1 · -BG1`, `DI0 AND NOT DI1`, `2s/X3`, `t/X3/500ms`, `=1` |
| Kontinuierliche Aktion | Kennzeichen (steht mittig im Kasten); Anschluss direkt an den Schritt legen | Variablen, solange der Schritt aktiv ist: `-QM1, DQ0` |
| Kontinuierliche Aktion mit Bedingung | Bedingung über dem Kasten | nur wenn die Bedingung erfüllt ist |
| Speichernde Aktion bei Aktivierung / Deaktivierung | Strukturierter Text | z. B. `Zyklen := Zyklen + 1` |
| Speichernde Aktion bei Ereignis | Ereignis über dem Kasten | z. B. `↑-BG4` |
| Parallelverzweigung, Synchronisierung | – | UND-Verzweigung und -Zusammenführung; Anschluss oben bzw. unten in der Mitte, Zweige bei ±100 |
| Sprung | Kennzeichen = Zielschritt | Rücksprung, z. B. zum Anfangsschritt `0` |

Ablauf nach der Norm: Eine Transition ist freigegeben, wenn alle Schritte davor aktiv sind, und schaltet, wenn
ihre Bedingung erfüllt ist. Alle schaltbaren Transitionen schalten gleichzeitig (Aktivieren hat Vorrang vor
Deaktivieren), das wiederholt sich bis zur stabilen Situation. Kontinuierliche Aktionen wirken nur in der stabilen
Situation, speichernde auch auf flüchtigen Schritten. ODER-Verzweigungen sind einfache Leitungsabzweige.

Namen in Bedingungen und Aktionen:

- **SPS-Variablen** wie im SPS-Programm (`DI0`, Symbole, Merker); so läuft das GRAFCET „in der SPS“ und schaltet
  über `DQ0` die Spulen.
- **Kennzeichen** direkt, ohne SPS: `-SF1` (Taster gedrückt), `-BG1` (Endschalter betätigt); in Aktionen `-QM1`
  (Magnet des Ventils), `-QM1/14` (links), `-QM1/12` (rechts).
- Operatoren: UND `·` oder `AND`, ODER `OR` oder `∨` (`+` bleibt Addition), NICHT `NOT`, `¬` oder `/a`; Flanken `↑a`, `↓a`, `R_EDGE(a)`,
  `F_EDGE(a)`; Zeiten `2s/X3` (seit 2 s aktiv).

Fehler im GRAFCET (Transition ohne Schritt davor, Sprung ins Leere, unlesbare Bedingung …) stehen beim Start im
Simulationsfenster bzw. bei `--simulate` auf der Fehlerausgabe. Testplan: `test_grafcet` (zwei Zylinder,
Parallelverzweigung, Wartezeit, Sprung, Zykluszähler).

## Prüfung gegen Referenzfälle

`QElectroTech --simulate-reference [ordner]` rechnet mit denselben Integrationsschritten wie die Simulation
(`sources/simulation/simulationphysics.h`) Referenzfälle mit bekannter Lösung, Parameter wie in den
Modelica-Referenzmodellen (Luft isotherm, R = 287,05 J/(kg K), T = 300 K, linearer Ventilleitwert):

| Fall | Erwartung | Kriterium |
|---|---|---|
| GasFill | p = 1 bar + 861,15 Pa/s · t | Fehler ≤ 0,05 Pa |
| GasCharge | p = 2 bar − 1 bar · e^(−t/0,2 s) | Fehler ≤ 5 Pa, Konvergenz 1. Ordnung bei halbem Zeitschritt |
| HydraulicCharge | p = 10 bar · (1 − e^(−t/1 s)) | Fehler ≤ 5 Pa, Konvergenz 1. Ordnung |
| PeriodicCounter | SPS-Zähler zählt alle 0,1 s, 9 bei 0,95 s | exakter Wert und Zeitpunkt |
| SPS-Sprachumfang | ELSIF, CASE mit Bereich und ELSE, MOD, INT-Division, SR, RS, F_TRIG, TOF, CTD; Syntaxfehler | 14 Handrechnungen, Fehler mit richtiger Zeile |
| GRAFCET-Schreibweise | `2s/X3`, `t/X1/500ms`, `-BG1 · /-BG2`, `-QM1/14 ∨ DI0`, `=1`, Flanke `↑-SF1` | 8 Fälle |
| PumpFill | p = 10⁷ Pa/s · t | Fehler ≤ 0,1 Pa |
| Orifice | Blende vorwärts, rückwärts, Gleichdruck | Wert wie OpenModelica auf 1e-12 m³/s, Verlustleistung ≥ 0, ungerade |
| AccumulatorCharge | p = 10 bar · (1 l/(1 l − Q·t))^1,4; außerdem Laden ab 1 bar über den Vorfülldruck | Fehler ≤ 0,1 Pa bzw. 1 Pa |
| Hydraulik-CylinderCycle | Pumpe, 4/3-Ventil, Differentialzylinder: aus, halten, zurück | Weg −0,1…100,1 mm, vor 0,7 s ≥ 75 mm, Haltedrift < 2 mm, nach 1,75 s < 2 mm, Drücke > −0,98 bar und < 100 bar, Volumenbilanz; mit Referenzlauf: Aus-/Einfahrgeschwindigkeit ±3 %, Rückhubzeit ±5 % |
| CylinderCycle | Ausfahren vor 0,4 s, Rückhub bis 1 s | Weg −2…62 mm, vorne ≥ 59 mm vor 0,4 s, nach 0,9 s < 1 mm; Hubzeiten ±1 ms bei halbem Zeitschritt; mit Ordner: Hubzeiten ±5 % gegen den OpenModelica-Lauf |
| OpenModelica-Vergleich pneumatisch / hydraulisch | Weg und Kammerdrücke gegen die Referenzläufe | pneumatisch an jedem ms Weg ≤ 1 mm, Druck ≤ 0,1 bar; hydraulisch Geschwindigkeiten ±3 %, Kammerdrücke beim Ausfahren, Halten und Einfahren ±10 % bzw. 1 bar; ungültige oder fehlende Referenzdatei = FEHLER |
| Unterdruck im Öl | 10 cm³ aus 0,1 l abziehen und wieder zuführen | Bilanz je Schritt, Druck nie unter der Grenze, danach wieder Ausgangsdruck |
| Ungültige Werte, Einheiten | NaN, Volumen 0, Speicherdaten; MPa/bar/cm³/1/min, unbekannte Einheit | NaN statt Mindestdruck; 99 und 100 MPa beide als MPa |
| Projekte (Ende zu Ende, `referenz/plaene`) | Hydraulik, Blindstopfen, Druckbegrenzung, GRAFCET, Speicher/Druckschalter/Logikventile, ungültige Daten | durch dasselbe `SimulationModel` wie Fenster und CLI, jeweils gegen Handrechnung oder erwartete Folge |

Fehlt der Ordner, erscheinen die Vergleiche als `SKIP` (nie als bestanden; Rückgabewert 2). Der Ordner enthält die Ergebnisdateien der OpenModelica-Läufe (DASSL, Toleranz 1e-9) für den pneumatischen und den
hydraulischen Zyklus; sie liegen unter `tools/simulation/referenz/` und werden mit dem Programm installiert, so dass
`--simulate-reference` ohne Ordner sie findet. Über MCP: `qet_simulate` und `qet_simulate_reference`. Der Zylinder dort hat elastische
Anschläge und eine geglättete Reibung, unserer harte Anschläge und Haft-/Gleitreibung; deshalb wird die Hubzeit
verglichen, nicht jeder Punkt.

## Zuordnungen im Plan

- **Ventilspule → Ventil:** Die Spule trägt das Kennzeichen des Ventils, z. B. `-QM1`. Bei zwei Magneten
  `-QM1/14` (links) und `-QM1/12` (rechts).
- **Endschalter → Zylinder:** Funktionstext des Schalters „`-MM1 vorne`“ bzw. „`-MM1 hinten`“.
- **Kennwerte** stehen als `<parameter>` im Symbol und lassen sich je Bauteil anpassen. Umgerechnet wird nach der
  Einheit (`unit="MPa"`, `bar`, `l`, `cm3`, `1/min` …), nie nach der Größe der Zahl; eine unbekannte oder unpassende
  Einheit und unmögliche Werte (Hub, Masse, Fläche, Speichervolumen ≤ 0) sind Fehler: die Simulation startet dann
  nicht und nennt das Bauteil. Ein Rechenfehler (nicht endlicher Wert) bricht den Lauf mit Meldung ab;
  `--simulate` endet dann mit Rückgabewert 3 und `Model error:` auf der Fehlerausgabe.

## Grenzen

Die Modelle sind eigene, aus der Fachphysik; sie sind gegen Handrechnungen geprüft (Hydraulik: Geschwindigkeit =
Förderstrom/Fläche), gegen analytische Lösungen und einen OpenModelica-Lauf (siehe oben), noch nicht gegen FluidSIM. Nicht simuliert: Makroschritte und einschließende Schritte (laufen als normale Schritte), Zwangssteuerung, analoge SPS-Ein-/Ausgänge, Proportionalventile
mit Stellsignal.
