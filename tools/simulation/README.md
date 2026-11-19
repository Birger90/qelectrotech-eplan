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
| Druckluftquelle | hält den Versorgungsdruck (Fenster: *Versorgungsdruck*); mit eigenem Nenndruck `pNom` (aus FluidSIM übernommen) diesen |
| Hydraulikpumpe | Verstellgrad `alpha` × Verdrängung × Drehzahl minus Leckage `GL`·Δp, eingebaute Druckbegrenzung: volle Förderung bis 1 bar unter `pMax`, bei `pMax` null |
| Hydrospeicher (`supply.accumulator`, Öl) | Gasvolumen `V0`, Vorfülldruck `p0`, Polytropenexponent `k`; unter dem Vorfülldruck leer. Der Druck im Netz wird implizit aus der Volumenbilanz gelöst, die Ölmenge bleibt bei jedem Zeitschritt erhalten |
| Druckluftbehälter (`supply.accumulator`, Luft) | Volumen `V0` im Netz, Anfangsdruck `p0`; ein endlicher Vorrat, keine Quelle |
| Druckschalter (`sensor.pressure_switch`) | schaltet bei `p0` ein, `p_hyst` darunter aus; Kontakte mit Funktionstext „`-BP1`“, im GRAFCET `-BP1` |
| Wechselventil, Zweidruckventil | Ausgang 2 vom Eingang mit höherem (ODER) bzw. niedrigerem Druck (UND) |
| Zeitverzögerungsventil | schaltet 1 → 2, wenn Steueranschluss 3 `t_delay` lang Druck hat; sonst entlüftet 2 |
| Tank | drucklos |
| Hydromotor (`motor.fixed_motor`, `motor.variable_motor`) | Welle: q = α·D·ω/2π + C·Δp (A → B), τ = α·D·Δp/2π, J·dω/dt = τ − Lastmoment − f·ω − M_fric·tanh(ω/0,1); Kennwerte `displacement`, `alpha`, `GLeak`, `I`/`J`, `f`, `M_fric`, `M_load`/`M_ext`; Messgrößen `omega:`, `phi:`, `tau:`, `P:`. Druckluftmotoren: kein Modell |
| Wegeventil | Schaltwege der Stellung; Luft nach ISO 6358 aus `qNN`, Öl mit Widerstand `rT` (Δp = rT·Q², um Δp = 0 stetig geglättet: Q ∝ Δp/(Δp² + 0,3 bar²)^¼); Schaltzeiten `onDelay`/`offDelay`: der Schieber bewegt sich wie ein Stellglied erster Ordnung (Zeitkonstante = Schaltzeit/3, nach der Schaltzeit 95 % offen), die Wege der alten Stellung schließen, die der neuen öffnen; Feder in die Ruhestellung, ohne Feder bleibt die Stellung; Steuerdruck an 14/12 |
| Rückschlag-, Drosselrückschlagventil | freie Richtung `free_from` → `free_to` (aus der Zeichnung); öffnet stetig ab dem Öffnungsdruck `p0`; entsperrbar über Steueranschluss |
| Drossel, Absperrventil, Schalldämpfer, Schnellentlüftung | Leitwert aus `qNN` bzw. `rT` |
| Druckbegrenzung, Druckregler | Einstelldruck `p0`, Öffnungsbereich `dpEnd`; Druckbegrenzung von Eingang 1 (P) in den Rücklauf 2 (T), ohne Rücklaufleitung ins Freie, öffnet nach dem Unterschied Eingang – Rücklauf |
| Zylinder | Endanschlag hart oder elastisch (`stop_k` N/m, `stop_c` N·s/m: Feder mit einseitiger Dämpfung wie die OpenModelica-Referenz, Kolben darf eindringen und prellen); Kraftbilanz: Drücke × Flächen, Masse, Haft-/Gleitreibung (mit `F_ref`/`v_ref` aus FluidSIM viskos F_ref/v_ref, sonst 20 N·s/m), Feder, Last, Endanschläge; meldet Unterdruck im Öl (Kavitation wird nicht gerechnet) ; R11: Einbauwinkel `alpha` (Gewicht m·g·sin α gegen das Ausfahren), äußere Masse `m_ext` mit Haft-/Gleitreibung `mu_B_ext`/`mu_M_ext`, hydraulische Kolbenreibung `F_B`/`F_M`/`F_V`, Federkennlinie `p_spring_min`/`p_spring_max` mit `spring_left`/`spring_right`, Kraftprofil `F_var_ex`/`F_var_re` {x mm, F N, …} bei `use_F_const` False; gleichzeitige Betätigung eines Ventils: Vorrang `dominant`, sonst bleibt es stehen |
| Spannungsquelle | oberer Anschluss Plus, unterer Minus |
| Taster, Endschalter | Schließer/Öffner aus dem Symbol; Wahlschalter (Rastung, `sim_role` `switch`) schaltet bei jedem Druck um; Endschalter auch über einen Hubbereich „`-MM1 0..2`“ (mm) |
| Handventil mit Wahlschalter | nur auf einer Seite von Hand betätigt, ohne Feder: jeder Druck schaltet um, das Ventil bleibt stehen |
| Rollenhebel-, Stößelventil | Betätigung `roller`/`plunger` mit Funktionstext „`-MM1 0..2`“: betätigt, solange der Kolben im Bereich steht |
| GRAFCET-SPS (`sim_role` `grafcet_io`) | Elementinformation `sim_io` „`I0=-KF1;Q0=-MB1`“: Eingang wahr, solange sein Anschluss Plus hat; Ausgangsanschluss hat Plus, solange die Variable wahr ist |
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

## Analoge Elektrik (R17 Stufe 1, Fork 1.41.0)

Ein Stromkreis aus Bauteilen mit `sim_role` = `circuit` wird nach Kirchhoff gerechnet (modifizierte Knotenanalyse, implizites
Euler-Verfahren, Formeln E01–E05). Je Bauteil `sim_block` und `sim_params` (SI-Vorsätze p n u µ m k M G):

| sim_block | Bauteil | sim_params |
|---|---|---|
| `R`, `C`, `L` | Widerstand, Kondensator, Spule | `R=1k`, `C=100u`, `L=100m` |
| `V`, `I` | Spannungs-, Stromquelle (erster Anschluss + bzw. „+“, „P“, „1“) | `V=10` (Gleich), `V=10;f=50;offset=0;phase=0` (Sinus) |
| `D` | Diode (Anode = erster Anschluss bzw. „A“) | `Is=1e-12;n=1` |
| `GND` | Masse (0 V), mindestens eine je Stromkreis | – |

Messgrößen `u:-R1` (Spannung erster gegen zweiten Anschluss), `i:-R1` (Strom vom ersten zum zweiten), `pel:-R1` (aufgenommene
Leistung, Quellen negativ).

**Stufe 2 (Fork 1.42.0): Kopplung an die Schaltlogik** (Formeln E06–E08, Prüfplan `kopplung.qet`):

| Teil | wird im Stromkreis | Angaben |
|---|---|---|
| Taster, Grenztaster, Relaiskontakt mit beiden Anschlüssen auf Stromkreisnetzen | Schalter R_on / R_off | `sim_contact` „Ron=10m;Roff=1G“ (Vorgabe) |
| Relaisspule, Ventilmagnet mit beiden Anschlüssen auf Stromkreisnetzen | R und L in Reihe; Anzug bei I_an, Abfall bei I_ab | `sim_coil` „R=240;L=2.4;Un=24;on=0.8;off=0.1“ oder „Ion=…;Ioff=…“ – R, L, Un aus dem Datenblatt |
| `sim_block` `AI` | Eingangswiderstand, Spannung → Variable | `sim_params` „Rin=100k;Umin=0;Umax=10;raw=27648“, `sim_var` „Messwert“ |
| `sim_block` `AQ` | Spannungsquelle aus der Variable | `sim_params` „Umin=0;Umax=10;raw=27648“, `sim_var` |

Kontakte heißen im Stromkreis „-K1/14“ (Anschluss der Schaltseite) bzw. „-K1/Schließer“; `plc:Messwert` liest die Variable.
Die Schaltlogik sieht den Spulenstrom des vorigen Rechenschritts (10 µs bei der Referenz).

**Stufe 3 (Fork 1.43.0): Halbleiter** (Formeln E09, E10, Prüfplan `halbleiter.qet`):

| sim_block | Bauteil | Anschlüsse | sim_params (Vorgabe = Projektfestlegung) | Messgrößen |
|---|---|---|---|---|
| `QN`, `QP` | NPN-, PNP-Transistor (Ebers-Moll) | C, B, E | `Is=1e-12;n=1;BF=100;BR=1` | `u:` U_CE, `i:` I_C, `ib:` I_B |
| `OP` | Operationsverstärker | +, −, OUT | `A=1e5;Rin=1M;Rout=1;Vmax=15;Vmin=-15` | `u:` Ausgang gegen Masse, `i:` Ausgangsstrom, `ib:` Eingangsstrom |

Ohne Frequenzgang, Anstiegsrate, Early-Effekt und Kapazitäten; die Begrenzung des OPV ist ein Kennwert, keine Versorgung.

**Stufe 4 (Fork 1.44.0): Schutzorgane** (Formel E11, Prüfplan `schutz.qet`):

| sim_block | Bauteil | Vorgaben (Projektfestlegung im Normband) | Pflicht |
|---|---|---|---|
| `F` | Sicherung gG | kf = 1,6, τ = I²t/(kf·In)² | `In`, `I2t` (Schmelzintegral, Datenblatt) |
| `MCB_B`, `MCB_C`, `MCB_D` | Leitungsschutzschalter | kf = 1,45, τ = 60 s, Im = 5 / 10 / 20 | `In` |
| `MPCB` | Motorschutzschalter Klasse 10 | kf = 1,125, τ = 280 s, Im = 13 | `In` (Einstellstrom Ir) |

Alle mit `R` (Vorgabe 10 mΩ), überschreibbar `kf`, `tau`, `Im`. Ausgelöst: 1 GΩ, Meldung „… löst aus (thermisch|magnetisch, … A)“;
`trip:-F1` (1/0), `w:-F1` (thermischer Zustand A²s). Zurücksetzen mit dem Ereignis „-F1 reparieren“.
Prüfplan `analog.qet`.

## Simulationsmodi und Leistung (R16, Fork 1.40.0)

- Simulationsfenster: **Start**, **Pause**/**Fortsetzen**, **Stop** (Ende; der nächste Start beginnt von vorn), **Schritt 10 ms**
  (ein Steuerzyklus), **Bis zur nächsten Änderung** (bis Ventil, Endlage, Relais, Schritt … umschalten, höchstens 10 s),
  **Nur aktuelles Blatt**.
- Kommandozeile: `--folios 2` bzw. `1,3` oder `2-4` (nur diese Blätter), `--until-change` (Halt bei der ersten Zustandsänderung),
  `--measure`/`--csv`/`--raster` (Messabgriffe wie bisher), `--meta` mit `wallTimeS`, `steps`, `stepsPerSecond`, `fluidNets`, `folios`.
- Formel N10, Prüfplan `zwei_blaetter.qet`.

## Fehlerszenarien (R15, Fork 1.39.0)

Ein Defekt steht als Elementinformation `sim_fault` am Bauteil, Schreibweise „`<Fehler> [Wert] [ab t] [bis t]`“, mehrere durch
`;` getrennt (Fehlermodelle wie in FluidSIM, Formel K16):

| Fehler | Bauteil | Wirkung |
|---|---|---|
| `kolben_klemmt` | Wegeventil | behält seine Stellung |
| `spule_unterbrochen` | Ventil- oder Relaisspule | ohne Wirkung |
| `interne_leckage 0.5` | Teil mit zwei Fluidanschlüssen (Zylinder …) | Öl G·Δp in l/(min·bar), Luft Normalstrom in l/min, zwischen den ersten beiden Anschlüssen |
| `verstopft 0.2` | Drossel, Filter, Ventil mit Widerstand | Strom × Restanteil (ohne Wert 1 %) |
| `immer_gesperrt`, `immer_geoeffnet` | Rückschlag-, Drosselrückschlagventil | sperrt immer bzw. ist in beide Richtungen offen |
| `schaltet_nicht`, `schaltet_immer` | Taster, Grenztaster, Druckschalter | Kontakt nie bzw. immer betätigt |

`ab 0.5 bis 2` begrenzt den Fehler zeitlich; das Ereignis `1.0 -QM1 reparieren` (`--event`, MCP `qet_simulate`) hebt alle
Fehler des Bauteils auf. Meldungen „Fehler -QM1: kolben_klemmt“, „Fehler -QM1 aufgehoben“, „-QM1 repariert“; Messgröße
`fault:-QM1` (1 wirksam). Prüfplan `fehler.qet`.

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

Gliederung (R14, Fork 1.38.0; Schreibweise wie FluidSIM):

| Mittel | Eintrag | Wirkung |
|---|---|---|
| Teil-GRAFCET | Elementinformation `sim_grafcet` = `G5` an einem Schritt jeder Kette | gleich benannte Ketten bilden G5; ohne Namen: globaler GRAFCET |
| Schrittnamen | `X3` im eigenen Teil-GRAFCET, `G5.X3` von außen, `XG5` (G5 hat einen aktiven Schritt) | gleiche Nummern in verschiedenen Teil-GRAFCETs sind erlaubt |
| Einschließender Schritt | Symbol „Einschließender Schritt“, `sim_encloses` = `G2 G3` (ohne Angabe `G<Nummer>`) | aktiviert die Aktivierungsschritte (Anfangsschritt-Symbol oder `sim_activation` = 1), deaktiviert beim Verlassen alles darin |
| Makroschritt | Symbol „Makro Schritt“, Kennzeichen `M3`; Expansion mit Schritten `E3` … `S3` | Transition nach M3 erst mit aktivem S3 |
| Zwangssteuerung | kontinuierliche Aktion `G5{8, 9}`, `G5{}`, `G5{*}`, `G5{INIT}` | G5 in dieser Situation, entwickelt sich nicht, solange der Schritt aktiv ist |

Prüfplan `grafcet_r14.qet`.

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

## Modellkatalog (welches Bauteil wie gerechnet wird)

Jede Fluidfunktion hat genau eine Entscheidung im versionierten Katalog (`sources/simulation/simulationcatalog.cpp`,
`--simulate-catalog` gibt ihn als JSON aus): **physical** (ein Rechenmodell des Kerns, mit Anschlüssen, Kennwerten,
Grenzen und den Prüfungen, die es abdecken), **ideal_connection** (verlustfreie Verbindung: Leitung, Kupplung,
Verteiler), **display** (Messstelle ohne Rückwirkung; Durchflussmesser verbinden ihre Anschlüsse), **graphic**
(Zeichenbaustein) oder **unsupported**. Ein Bauteil ohne Rechenmodell oder mit einer Funktion, die nicht im Katalog
steht, verhindert den Start mit Kennzeichen und Funktion – die Simulation errät kein Modell aus Art oder Symbol
(eine unbekannte Quelle ist keine Druckquelle, ein Greifer kein Linearzylinder). Filter, Wartungseinheit, Öler,
Trockner, Kühler, Heizer und Anlaufventil laufen als Durchgangswiderstand (`passthrough`, ohne Wärme- oder
Filterwirkung). Ein pneumatischer „pump“ ist ein Verdichter und damit ideale Druckquelle.

## Leitungen, Kupplungen, Aufbereitung (R05)

- **Leitung/Schlauch** (`connection.line`, `supply.hose_line`): ohne Kennwerte ideal (Anschlüsse ein Netz), mit `rT`
  (bar·min²/l²) bzw. `qNN` (l/min) als Widerstand, mit `length` und `d_inner` als Rohr: Δp = 128·μ·L·q/(π·d⁴) bis
  Re 2000, Darcy mit Haaland-Reibzahl ab Re 4000 (`roughness`, Vorgabe 15 µm), dazwischen stetig. Öl nach `nu`
  (Vorgabe 22 mm²/s, HLP 22) und `rho` (870 kg/m³), Luft mit Dichte beim Mitteldruck. Keine Fluidträgheit.
- **Drehdurchführung**: Anschlüsse auf gleicher Höhe im Symbol bilden je einen Kanal (1–4, 2–5, 3–6).
- **Schnellkupplung** (`connection.quick_coupling`): `coupled` 1/0, `seal` 1 (selbstdichtend) oder 0 (offen,
  entlüftet). Der Zustand gilt für den ganzen Lauf.
- **Filter, Wartungseinheit, Öler …**: Durchgangswiderstand; `clogged` 1 mit `qNN_clogged` (Vorgabe 10 l/min) bzw.
  `rT_clogged` (Vorgabe 1000·rT). Keine Filter-, Kühl- oder Heizwirkung.

Nicht unterstützt (Start abgewiesen): Stetig-/Proportionalventile, Muskel-, Lade- und Druckluftmotorantriebe,
Load-Sensing-Pumpen, Folge-, Senkbrems- und Kompensatorventile, Vakuum-Sparventil, Ventilgruppen und pneumatische
Zeitglieder/Zähler – siehe Roadmap (R07–R13).

Herkunft, Beleg und Grenze jeder Formel stehen im [Formelregister](FORMELN.md).

## Stetig-, Senkbrems- und Zuschaltventile (R07/R09)

- **4/3-Stetigventil** (`proportional.directional_valve43_01/_02`): Anschlüsse P, A, B, T. Sollwert `u` in Volt
  (−10 … 10) als Kennwert oder Ereignis `--event "0.5 -QM1 sollwert 5"`; der Schieber folgt als PT1 (`tau`, Vorgabe
  4 ms beim Regelventil 02, 10 ms beim 01). Positiv öffnet P→A und B→T, negativ P→B und A→T. Überdeckung `lap`
  bzw. `lapP`/`lapT` in % (negativ = Unterdeckung), Steuerkerbe `notchType` 1 Rechteck (linear) oder 2 Dreieck
  (progressiv), `rT` je voll offener Kante, `GLeak` je Kante. Messgrößen `u:<Ventil>` [V], `s:<Ventil>` [−1 … 1].
  Mit einem gekoppelten lagegeregelten Magneten oder Verstärker (R06) kommt der Sollwert aus dem Signalpfad.
- **Senkbremsventil** (`pressure_valve.counterbalance_valve`, P, T, X): öffnet P→T, wenn
  (p_P − p_T) + `ratio`·(p_X − p_T) den Öffnungsdruck `p0` erreicht, voll offen nach `dpEnd`; Rückschlagventil T→P
  (`rT_cv`), Leckage `GLeak`.
- **Druckzuschaltventil** (`pressure_valve.pressure_sequence_valve01/02`, P, A, X): öffnet P→A ab Steuerdruck `p0`
  an X (ohne X an P).
- **Druckwaagen** (R09, `pressure_valve.pressure_compensator_*`): druckabhängiger Widerstand im Hauptstrom (`rT` Öl,
  `qNN` Luft), Solldruck `p0`, Regelbereich `dpEnd`. Schließer 01/02 (Strom B ↔ A, Steuerung X − XS; mit X an A ein
  Druckregler) schließen ab p0 und sind bei p0 + dpEnd zu, Öffner 01/02 öffnen ab p0. Vorgeschaltet (P → A) hält
  p_A − p_X, nachgeschaltet (P → A) öffnet, wenn p_P den Lastmeldedruck X um p0 übersteigt, Open Center LS öffnet
  P → T ab p_X + p0.
- **Schock- und Nachsaugblock** (`pressure_valve.shock_and_anti_cavitation_valve`): A1 = A2 und B1 = B2 sind
  durchgehend; A und B werden gegen T begrenzt (`p0_prvA/B`, `dpEndA/B`, `rT_prvA/B`), fällt A oder B unter den
  Tankdruck, saugt das Rückschlagventil (`rT_cv`) nach.
- **Pneumatisches Folgeventil** (`pressure_valve.pressure_sequence_valve`, 1, 2, 12): öffnet 1 → 2, sobald der Druck an
  12 den Schaltdruck `p0` erreicht (Übergang 0,1 bar); geschlossen ist 2 abgesperrt.
- Prüfpläne `druckwaagen.qet` (LS-Druckwaage, vorgeschaltete Druckwaage vor einer Drossel, Schließer als Druckregler,
  Schockblock mit Begrenzung und Nachsaugen) und `folgeventil.qet`.
- Weiter nicht gerechnet (Start abgewiesen): LS-/Mobilventilblöcke, MPYE, Druckwaage `closer03` (ohne Kennwerte),
  Vorsteuergeber Joystick.

## Analoge Signale und Regler (R06)

Elektrische Teile mit reellen Werten. Sie tragen die Bauteilinformationen `sim_role` = `analog`, `sim_block`
(FluidSIM-Klasse ohne „ET.“), `sim_params` (Kennwerte mit Einheit, z. B. `KP=1;KI=5 1/s;KD=0 ms`) und `sim_link`
(Kopplung); der FluidSIM-Import schreibt alle vier. Anschlüsse heißen wie am FluidSIM-Bauteil.

- **Signalvertrag:** Jedes elektrische Netz hat ein Potential gegen 0 V: ein Analogausgang, sonst das Potential der
  Quelle (`sim_volt` an `supply+`/`supply-`, ohne Angabe 24 V bzw. 0 V). Die Bausteine rechnen stetig im Modellschritt
  (10 µs), ein Baustein nach denen, die seine Eingänge treiben. SPS und GRAFCET lesen die Analogwerte nicht (ihr
  10-ms-Zyklus sieht nur digitale Eingänge). Zwei Ausgänge auf einem Netz sind ein Modellfehler. Ströme,
  Widerstände und Lastrückwirkung werden nicht gerechnet.
- **Quellen:** Funktionsgenerator (`Source.WaveformGenerator02`, Ausgang S; `waveForm` 1 Rechteck, 2 Sinus,
  3 Dreieck, 4 Konstant = `yOffset`, 5 Profil), Sollwertkarte (`Source.SetPointGenerator`, Q; W1 … W8, Rampen
  R1 … R4 in s/V, Modus 1 Umschaltzeit, 2 Weiterschalten, 3 Auswahl über I1 … I3 ≥ 15 V).
- **Regler:** PID (`Controller.ControllerPID`: W, X → Y, `KP`, `KI` [1/s], `KD` [ms], `yOffset`, `outputMode`
  0 = ±10 V, 1 = 0 … 10 V; Anti-Windup; ohne Versorgung ≥ 21,6 V Ausgang 0 und Neustart), Zustandsregler
  (`ControllerS`), PT2-Glied, I-Glied, Differenzverstärker (Bereich aus den Potentialen an A1/A2), Komparator
  (U → S mit Hysterese; S schaltet die Versorgung, z. B. auf ein Relais).
- **Stellglieder:** Proportional-Verstärker 1/2 Kanäle (U → gekoppeltes Ventil, Sprungstrom `jumpCurrentRel`),
  lagegeregelter Proportional-Ventilmagnet (`Relay.ValveContSolenoid`: Spannung an C ist der Ventilsollwert).
  Kopplung `sim_link`: Verstärker `A=-QM1/Y2;B=-QM1/Y1` bzw. `A=-QN1/S`, Magnet `C=-QM1/S`. Vorzeichen wie in FluidSIM:
  ein positiver Sollwert öffnet P→A (Y2 bzw. positive Spannung an S), Y1 öffnet P→B (der Ventilsollwert `u:` zählt positiv für P→A). Gekoppelt werden
  4/3-Stetigventile und das Proportional-Druckbegrenzungsventil (`proportional.pressure_relief_valve`, A → B,
  Öffnungsdruck `p_start` bei 0 V bis `p_end` bei 10 V, `dpEnd`, `rT`).
- **Sensoren:** Analog-Drucksensor (elektrischer Teil `Sensor.PressureSensorHyd/Pneu`, Y; liest den Fluidteil
  `proportional.pressure_sensor` gleicher Kennzeichnung bzw. `sim_link P=-BP1` mit `p_start`, `p_end`, `U_start`,
  `U_end`), Durchflusssensor (`Sensor.FlowSensorHyd` mit `proportional.flow_sensor`: Blende `rT`, `mode` 1 Betrag,
  2 mit Richtung), Wegmesssystem (`Sensor.PositionSensor`, S; `sim_link X=-MM1`, `U_start`/`U_end` für ein- und
  ausgefahren), Voltmeter (Anzeige p − n).
- **Messgrößen:** `u:<Baustein>` Ausgangsspannung [V], `e:<Regler>` Regelabweichung [V], `qs:<Durchflusssensor>`
  Durchfluss [m³/s]; die Ventile weiter `u:` und `s:`.
- **Anschlussnamen:** Stetigventile, Analogsensoren und Analogbausteine heißen beim Import wie am FluidSIM-Bauteil;
  bei gespiegelten Symbolen weicht die Beschriftung davon ab (A trägt „B“, P trägt „T“). Der Durchflussmesser
  `sensor.flowmeter_2` ist jetzt eine messende Blende (rT 0,001) statt einer bloßen Verbindung.
- **Prüfpläne:** `druckregelung.qet` (PI-Regler hält 40 bar über das Proportional-Druckbegrenzungsventil),
  `lageregelung.qet` (P-Regler fährt den Zylinder über Wegmesssystem und 4/3-Regelventil auf 120 mm). Die
  Prüfsymbole darin sind eigene Kästen mit benannten Anschlüssen.
- Nicht gerechnet: Potentiometer, Oszilloskop, Strom- und Widerstandsmesser (ohne Wirkung), MPYE und
  Proportional-Verstellpumpe, analoge SPS-Ein-/Ausgänge.

## Druckluftmotoren (R12)

- **Druckluftmotor mit Mittelauspuff** (`actuator.motor_with_mid_exhaust`, A, B, EX): aus den FluidSIM-Nennwerten
  `p_N`, `rpm_N`, `M_N`, `n0_nN` eine lineare Momentenkennlinie (Stillstandsmoment M_N·k/(k − 1), Leerlauf k·n_N bei
  p_N), Moment proportional zur Druckdifferenz A − B, Trägheit `I`, Fremdmoment `M_ext` bremsend (`torqueType` 1) oder
  antreibend (2). Die Luft strömt vom höheren der Anschlüsse A/B zum Auspuff EX (Verbrauch Dichte · Schluckvolumen ·
  Drehzahl). Messgrößen wie beim Hydromotor: `omega:`, `phi:`, `tau:`, `P:`.
- **Druckluftmotor** (`actuator.rotary_motor` pneumatisch): Schluckvolumen `displacement` je Umdrehung, viskose
  Reibung `f`, Trägheit `I`; Luft von A nach B.
- Prüfplan `druckluftmotor.qet` (eigenes Prüfsymbol, Motor rechts- und linksherum).
- **Mehrstellungszylinder** (`actuator.multi_position_cylinder`, C1, C2, C3; Fork 1.34.0): zwei Kolben gleichen
  Durchmessers hintereinander. Der hintere (C1, Hub `xMax2`, Standard `xMax`/2) schiebt den vorderen mit seiner Stange in
  die Zwischenstellung, C2 fährt den vorderen allein bis `xMax`, C3 ist seine Stangenseite. Kontakt einseitig, Stoß
  plastisch; Startlagen `xStart`, `xStart1`; Messgrößen `x:` (vorderer Kolben) und `x1:` (hinterer). Prüfplan
  `mehrstellungszylinder.qet` (Formel M12).
- Nicht gerechnet: Muskeln (Kennlinie nur aus dem Datenblatt), Belastungseinheit (R13), Druckübersetzer (FluidSIM gibt
  keine Kennwerte, nur Datenblattgrößen). Tandem- und Teleskopzylinder gibt es in FluidSIM nicht als Bauteil.

## Mobilhydraulik (R13, Fork 1.35.0)

- **LS-Aggregat** (`supply.load_sensing_pump_unit`, P1 Verstellpumpe, P2 Konstantpumpe, LS): Verdrängung `displacementVP`/
  `displacementFP` · `rpm`; LS-Regler α = 1 − (p_P1 − p_LS − `p0`)/`dpEnd` als PT1 (`T_PT1`), Druckbegrenzung je Pumpe
  (`p0_prvLS`/`p0_prvFixed`, Band `dpEnd_prv*` in bar, `rT_prv*`), Leckage `GL_VP`/`GL_FP`, LS-Entlastung `rT_LS`. Tank-,
  Leck- und Lüftungsanschlüsse (T, L, LA) dürfen offen bleiben. Formel H08.
- **6/3-Handhebelventil** (`proportional.directional_valve63_with_hand_lever`): `xStep` in % (+: P2 → B, A → T2; −: P2 → A,
  B → T2; 0: Umlauf P1 → T1), Widerstände `rT_P1_T1`, `rT_P2_AB`, `rT_T2_AB`, Rückschlagventil `rT_CV`, Schieber `T_2`.
  Formel V15.
- **Belastungseinheit** (`actuator.loading_unit_2`, A1.1, A1.2, A2.1, A2.2): zwei gegenüberliegende Zylinder, Masse `m`,
  Reibung `F_B`/`F_M`/`F_V`, Einbauwinkel `alpha`. Formel M13.
- Prüfplan `mobilhydraulik.qet`.
- **Lenkventil** (`supply.steering_unit_1/2`, Fork 1.36.0; P, T, L, R, E): Handmoment `M_user` (+ links) gegen `M_userMax`
  öffnet das Drehventil (Kanten `rT`/Anteil²), neutral P → E; Lenkradwinkel `phi:` aus dem Dosierstrom und `displacement`.
  Formel H10.
- **Vorsteuergerät** (`pressure_valve.pilot_valves_joystick`; P, T, X1.1, X1.2, X2.1, X2.2): `adjustmentLeft`/`adjustmentRight`
  in %, + regelt X g.1, − X g.2 auf |Auslenkung|·`pMax`. Formel H11.
- **Sperrblock** (`shutoff_valve.double_non_return_valve01/02`; A1 → A2, B1 → B2): `p0` Feder, `ratio` Entsperrkolben, `rT`.
  Formel V16.
- **Belastungseinheit 1** (`actuator.loading_unit_1`): wie Belastungseinheit 2, Stangenräume entlasten sich über `p0_prv`,
  `dpEnd_prv`, `rT_prv`, `rT_cv` ineinander. Formel M14.
- **Membranspeicher mit Sicherheitsblock** (`supply.accumulator` mit `p0_accu`; P, T): Kugelhahn `x` (2 offen), Druckbegrenzung
  `p0_prv` zum Tank. Formel H12.
- Prüfplan `lenkung.qet`.
- **LS-Steuerblock** (`proportional.mobile_valve_block_ls`, Fork 1.37.0; P, T, LS, je Scheibe A g, B g, Vorsteuerung C g, D g):
  Vorsteuerdruck zwischen `pS_min` und `pS_max` (C → P→A, D → P→B) oder Handhebel `xStepLeft`/`xStepRight`, Druckwaage `p0_pc`,
  `dpEnd_pc`, `rT_pc`, Kanten `rT_PAB`, `rT_TAB`, Wechselventil `rT_SV`. Formel V17.
- **Prioritätsventil LS** (`proportional.priority_valve_ls`; P, CF, EF, LS): `p0`, `dpEnd`, `rT_CF`, `rT_EF`. Formel V18.
- Prüfplan `lsblock.qet`.

## Vakuum, Schwenk- und Greiferantriebe (R10)

- **Ejektor** (`vacuum.vacuum_generator`): Anschluss 1 Versorgung, 2 Vakuum. Luftverbrauch `qn_ac` bei 600 kPa (wie
  FluidSIM), optimaler Versorgungsdruck `dp_max`; Anteil s = min(1, p₁/dp_max). Saugstrom q = q_max·(p₂ − p_atm − pu_min·s)/|pu_min|
  (q_max aus `qns_max`): null beim besten Vakuum pu_min·s, q_max·s gegen die Atmosphäre; ohne Versorgung derselbe
  lineare Weg zurück an die Luft. Über `dp_max` kein Abfall des Vakuums (nicht modelliert). Abblasen über ein
  eigenes Ventil auf das Vakuumnetz wird als normaler Netzzufluss gerechnet; ein Abblasanschluss des Ejektors fehlt.
- **Sauger** (`vacuum.suction_cup`): Volumen π d²/4 · 5 mm; Haltekraft (p_atm − p)·π d²/4, hält bei ≥ s·m·g
  (Kennwerte `d`, `m`, `s`); `qLeak` ist die Leckage bei −0,6 bar (ISO-6358-Blende), `object` 0 = kein Werkstück
  (offen, 50 l/min bei 0,1 bar für Ø 20 mm). Messgrößen `Fhold:<Sauger>` [N], `hold:<Sauger>` [1].
- **Vakuumschalter**: Druckschalter mit negativem `p0`; ein unter p0, aus über p0 + `p_hyst`.
- **Schwenkantrieb** (`actuator.semi_rotary`): wie der Zylinder, aber Winkel. Schluckvolumen je Radiant = Moment je
  Pascal = `M_6bar`/6 bar auf beiden Seiten, `xMax`/`xStart` in Grad, Trägheit `I`, viskose Reibung `f`,
  Haftmoment `M_stat` (Vorgabe 2 % von M_6bar), Lastmoment `M_ext`, Anschläge wie beim Zylinder. `x:` in rad,
  `v:` in rad/s, `F:` ist das Druckmoment in N·m; Wegmarken („-MM1 89..91“) am Schwenkantrieb gelten in Grad.
- **Greifer** (`actuator.gripper`): Ersatzmodell einfachwirkender Zylinder mit Rückstellfeder, Greifkraft = `F:`;
  keine Backenkinematik.

## Netze, Numerik und Ereignisse

Vor dem Lauf gemeldet: zwei Bauteile, die ein Netz auf verschiedenen festen Drücken halten (z. B. Druckquelle und
Tank an derselben Leitung), Luft- und Ölanschlüsse oder Fluid- und elektrische Anschlüsse in einem Netz. Gerechnet
wird mit festem Schritt 10 µs (expliziter Euler, Netze mit Hydrospeicher implizit); schwingt ein Netzdruck von
Schritt zu Schritt, ohne abzuklingen, bricht der Lauf mit dem Netz und dem Hinweis ab, dass Volumen oder Widerstand
für diesen Schritt zu klein sind. Ereignisse (Taster, Ventile von Hand) wirken ab dem ersten Schritt, dessen Zeit
nicht kleiner ist als ihre; gleichzeitige Ereignisse in der angegebenen Reihenfolge, das Ergebnis hängt davon nicht ab.
Gültige Kennwertbereiche: alle Längen, Flächen, Volumen, Massen > 0 bzw. ≥ 0 (Anfangsweg, Last), Zeiten ≥ 0,
Leitwerte qNN > 0, Widerstände rT > 0 und groß genug für den Schritt (sonst Abbruch wie oben).

## Messgrößen und Ergebnisse

`--simulate plan.qet --signals` listet alle Messgrößen als JSON (Kennung, Größe, SI-Einheit, Bezug, Bauteil,
Anschluss). Kennungen: `p:<Bauteil>.<Anschluss>` Überdruck in Pa (über P_ATM = 101300 Pa), `pa:` absolut,
`q:` Durchfluss **in das Bauteil hinein** an diesem Anschluss (Luft kg/s, Öl m³/s; positiv = hinein), `x:`/`v:`
Zylinderweg in m (0 = eingefahren) und Geschwindigkeit in m/s (positiv ausfahrend), `F:` Druckkraft auf den
Kolben in N (ohne Reibung, Last, Feder), `pos:` Ventilstellung (1 … n von links). `--measure all` oder
`--measure p:-QM1.4,q:-QM1.4` schreibt sie in die CSV (Kopf „Kennung [Einheit]“), `--raster 0.001` setzt das
Ausgaberaster (der Integrationsschritt bleibt 10 µs), `--meta lauf.json` schreibt Programm- und Katalogfassung,
SHA-256 des Projekts, Versorgungsdruck, Schrittweiten, Eingaben, Status und erreichte Endzeit. Eine unbekannte
Kennung bricht mit Meldung ab (keine Spalte voller Nullen); Messen verändert den Lauf nicht (Prüfung
„Messgrößen“). Die Ausgabe endet mit `Status: ok|abgebrochen, Endzeit … s`.

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
| OpenModelica-Vergleich pneumatisch / hydraulisch | Weg und Kammerdrücke gegen die Referenzläufe | pneumatisch an jedem ms Weg ≤ 1 mm, Druck ≤ 0,1 bar; hydraulisch Geschwindigkeiten ±3 %, Kammerdrücke beim Ausfahren, Halten und Einfahren ±10 % bzw. 1 bar; ganze Kurven im Schlauch (±5 ms, Druck ±10 % bzw. 1 bar, Weg ±2 mm, beide Richtungen; nur 10 ms nach dem Aufschlag am Anschlag ausgenommen, weil die Referenz dort elastisch prellt); ungültige oder fehlende Referenzdatei = FEHLER |
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
- **Endschalter → Zylinder:** Funktionstext des Schalters „`-MM1 vorne`“ bzw. „`-MM1 hinten`“ oder ein Hubbereich
  „`-MM1 0..2`“ in mm (so übernimmt der FluidSIM-Import die Wegmarken des Wegmaßstabs).
- **Ventilspule ohne Ventilkennzeichen:** über die Verknüpfung (Spule als Slave des Ventils), Seite aus der
  Elementinformation `valve_side` (`left`/`right`); ein Spulenkennzeichen in einer GRAFCET-Aktion (`-MB1`) betätigt
  diese Seite.
- **Rolle eines elektrischen Bauteils (`sim_role`):** eine Elementinformation `sim_role` legt fest, wie ein elektrisches Bauteil
  gerechnet wird, statt es am Symbolnamen zu erkennen: `supply+`/`supply-` (alle Anschlüsse Plus bzw. Minus),
  `voltage` (Anschlüsse p/+ und n/−), `button`, `switch` (Wahlschalter), `limit`, jeweils mit `_nc` als Öffner,
  `solenoid`, `grafcet_io`. Der FluidSIM-Import setzt sie.
- **Kennwerte** stehen als `<parameter>` im Symbol und lassen sich je Bauteil anpassen. Umgerechnet wird nach der
  Einheit (`unit="MPa"`, `bar`, `l`, `cm3`, `1/min` …), nie nach der Größe der Zahl; eine unbekannte oder unpassende
  Einheit, unlesbare Zahlen und Werte außerhalb ihres Bereichs (Durchmesser, Hub, Masse, Zeiten, Volumen ≤ 0, Kolbenstange
  nicht kleiner als Kolben; `rT` nur in bar.min2/l2, `k` ohne Einheit) sind Fehler: die Simulation startet dann
  nicht und nennt das Bauteil. Ein Rechenfehler (nicht endlicher Wert) bricht den Lauf mit Meldung ab;
  `--simulate` endet dann mit Rückgabewert 3 und `Model error:` auf der Fehlerausgabe.

## Grenzen

Die Modelle sind eigene, aus der Fachphysik; sie sind gegen Handrechnungen geprüft (Hydraulik: Geschwindigkeit =
Förderstrom/Fläche), gegen analytische Lösungen und einen OpenModelica-Lauf (siehe oben), nicht Wert für Wert gegen
FluidSIM; die 1136 FluidSIM-Beispiele laufen über `tools/fluidsim_import/fluidsim_batch.py` als Rauchprüfung (Import,
Modell, Bewegung). Nicht simuliert: Makroschritte und einschließende Schritte (laufen als normale Schritte), Zwangssteuerung, analoge SPS-Ein-/Ausgänge, Proportionalventile
mit Stellsignal.
