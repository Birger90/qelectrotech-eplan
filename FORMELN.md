# Formelregister Schaltschrankaufbau

Hier steht jede Beziehung, die der Schaltschrankaufbau (`sources/panel/`) rechnet, mit Codestelle, Herkunft und
Grenze. Das Formelregister des Simulationskerns liegt in `tools/simulation/FORMELN.md`.

**Herkunftsarten**: **Literaturformel** = in der genannten Quelle so angegeben; **eigene Ableitung** = nachvollziehbare
Umformung der Grundlagen; **Projektdefinition** = eigene Wahl, die Quelle nennt nur die Grundlage. Vorgabewerte sind
Projektdefinitionen, keine Messwerte. Alle Ergebnisse sind Richtwerte für die Planung, kein Erwärmungsnachweis.

Codestellen: `climate` = `panelclimate.cpp`, `wiring` = `panelwiring.cpp`, `layout` = `panellayout.cpp`,
`3d` = `panel3dview.cpp`, `edz` = `sources/import/edz/edzpart.cpp`.

## Klima des Schranks (Klimaberechnung)

| ID | Beziehung | Codestelle | Herkunft, Quelle | Grenze |
|---|---|---|---|---|
| K1 | Verlustleistung je Gerät P_i aus der Elementinformation `power_loss_w` (W): am Aufbaugerät, sonst in seiner Definition, sonst am Schaltplangerät gleicher Kennzeichnung (`panel_of`); beim EDZ-Import aus `P_ARTICLE_POWERDISSIPATION` („Max. Verlustleistung“, EPLAN-ID 22074, Text mit Einheit, z. B. „3,3 W“) | `climate` `ownPowerLoss`, `compute`; `edz` `wattsAttr` | Projektdefinition; Eigenschaft nach Q4 | Klemmenleisten zählen nicht (wenige mW je Klemme) |
| K2 | P = g · Σ P_i + P_zus mit Gleichzeitigkeitsfaktor g (Vorgabe 1) und zusätzlicher Verlustleistung P_zus (Verdrahtung, Schienen) | `climate` `evaluate` | Literaturformel nach Q4 (Geräteverlustleistung = max. Verlustleistung × Gleichzeitigkeitsfaktor) | g und P_zus schätzt der Planer |
| K3 | wirksame Oberfläche A_e = Σ A_0 · b mit Flächenfaktor b: Dach frei 1,4, Dach abgedeckt 0,7, Seitenfläche frei 0,9, Seitenfläche verdeckt (Wand, Nachbarschrank) 0,5, Boden 0 | `climate` `effectiveArea` | Literaturformel, Q1 (Tabelle Flächenfaktor b) | Maße in m: Breite B, Höhe H, Tiefe T des Schranks |
| K4 | Aufstellarten 1–7 als Flächen: 1 Einzelschrank frei A = 1,8·H·(B+T) + 1,4·B·T; 2 Einzelschrank an der Wand A = 1,4·B·(H+T) + 1,8·T·H; 3 Anfang/Ende einer Reihe frei A = 1,4·T·(H+B) + 1,8·B·H; 4 Anfang/Ende an der Wand A = 1,4·H·(B+T) + 1,4·B·T; 5 Mitte einer Reihe frei A = 1,8·B·H + 1,4·B·T + T·H; 6 Mitte an der Wand A = 1,4·B·(H+T) + T·H; 7 Mitte an der Wand, Dach abgedeckt A = 1,4·B·H + 0,7·B·T + T·H | `climate` `effectiveArea` | eigene Ableitung aus K3; gleichlautend in Q2 | – |
| K5 | Schrankmaße aus dem Schrank (Form mit Rolle Gehäuse um die Platte, Tiefe aus dessen `depth`); ohne Schrank Platte + 100 mm je Richtung und Tiefe = tiefstes Gerät + 100 mm, mindestens 300 mm (als „geschätzt“ gemeldet); im Dialog überschreibbar | `climate` `compute` | Projektdefinition | Schätzung nur als Notbehelf |
| K6 | Wärmeabgabe der Wände Q_S = k · A_e · ΔT; Wärmedurchgangszahl k: Stahlblech lackiert 5,5 W/(m²·K) (Vorgabe), Edelstahl 4,5, Aluminium 12, Aluminium doppelwandig 4,5, Kunststoff 3,5 | `climate` `evaluate`, `kMaterials` | Literaturformel und Richtwerte, Q2 | ruhende Luft außen, gleichmäßig verteilte Verlustleistung |
| K7 | Übertemperatur ohne Klimatisierung ΔT = P / (k · A_e); Innentemperatur T_i = T_u + ΔT | `climate` `evaluate` | eigene Ableitung aus K6 (Q_S = P im Gleichgewicht) | mittlere Lufttemperatur; die Schichtung (oben wärmer, Q1 rechnet ΔT in halber und voller Höhe) fehlt |
| K8 | abzuführende Wärme Q_K = P − k · A_e · (T_soll − T_u); Q_K ≤ 0: keine Kühlung nötig | `climate` `evaluate` | Literaturformel, Q2 | T_soll = zulässige Innentemperatur (Vorgabe 35 °C), T_u = Umgebung (Vorgabe 25 °C) |
| K9 | Filterlüfter V̇ = f · Q_K / (T_soll − T_u) mit f = 3,1 m³·K/(W·h); Luft/Luft-Wärmetauscher q = Q_K / (T_soll − T_u) in W/K; sonst Kühlgerät Q_K | `climate` `evaluate`, `kAirFactor` | Literaturformel, Q2 (f für 0–100 m über NN); f = 3600 s/h / (ρ · c_p) = 3600 / (1,16 kg/m³ · 1005 J/(kg·K)) ≈ 3,09 (eigene Ableitung, Q3) | nur wenn T_u < T_soll; in größerer Höhe ist f größer (dünnere Luft) |

## Kanalfüllgrad

| ID | Beziehung | Codestelle | Herkunft, Quelle | Grenze |
|---|---|---|---|---|
| W1 | Welche Geräte einer Montageplatte ein Potential verbindet: Anschlüsse über Leiter, Verbindungspunkte und Folgeverweise zu Netzen zusammengefasst; ein Kontakt gehört zu seinem Gerät (Master), eine Klemme zu ihrer Klemmenleiste; das Aufbaugerät wird über `panel_of` (=Anlage+Ort-BMK) gefunden, sonst über die Kennzeichnung | `wiring` `route` | Projektdefinition | Geräte außerhalb der Platte (Motoren, Feldgeräte) und Kabel zählen nicht |
| W2 | Ein Netz mit n Geräten braucht n − 1 Leitungen: die kürzesten (minimaler Spannbaum nach Prim über die Weglängen durch die Kanäle) | `wiring` `route` | Projektdefinition (übliche Verdrahtung von Gerät zu Gerät) | wirkliche Reihenfolge der Verdrahtung kann abweichen |
| W3 | Weg einer Leitung: vom Gerät in den nächstgelegenen Kanal, entlang der Kanalmittellinien bis zu den Stellen, an denen Kanäle sich berühren oder kreuzen, auf dem kürzesten Weg (Dijkstra), in den Kanal nächst dem anderen Gerät | `wiring` `route`, `Graph` | Projektdefinition | ohne verbundenen Kanal: „ohne Kanalweg“ |
| W4 | Platzbedarf je Leitung A_L = d²; Außendurchmesser d aus dem Querschnitt (größter am Netz eingetragener Querschnitt, sonst 1,5 mm²) nach Richtwerten einadriger PVC-Aderleitungen: 0,5 → 2,1; 0,75 → 2,3; 1 → 2,5; 1,5 → 3,1; 2,5 → 3,7; 4 → 4,4; 6 → 4,9; 10 → 6,3; 16 → 7,5; 25 → 9,4; 35 → 10,8; 50 → 13,0; 70 → 15,0; 95 → 17,0; 120 → 19,0 mm, dazwischen linear | `wiring` `outerDiameter` | Richtwerte (Projektdefinition), gerundet aus Herstellerangaben zu H05V-K/H07V-K; die Normgrenzen stehen in Q5 | das Quadrat d² rechnet die Zwickel zwischen runden Adern mit |
| W5 | Füllgrad eines Kanals = Σ A_L der Leitungen im vollsten Abschnitt / Innenquerschnitt (B − 4 mm) · (H − 4 mm) (Wand und Boden je 2 mm); Befund ab 70 %, orange ab 56 % (80 % der Grenze) | `wiring` `warnings`, `kWarnPercent` | Projektdefinition (übliche Planungsreserve von 30 % für Nachverdrahtung) | Kanalhöhe aus `depth` des Kanals, Vorgabe 60 mm |
| W6 | Leitung ohne Kanalweg (Platte ohne Kanäle, oder kein Kanal verbindet die Geräte): Weg auf einem Raster von 25 mm über die Platte (bei sehr großen Platten gröber), Rasterpunkte innerhalb eines Geräts gesperrt, Anschluss am freien Rasterpunkt nächst der Gerätekante; Kosten je Schritt 1 · Raster, entlang schon verlegter Leitungen 0,35 · Raster, je Ecke + 0,5 · Raster (Dijkstra mit Richtung); Paare nach Prim über den rechtwinkligen Abstand der Gerätemitten; Länge = Schritte · Raster + rechtwinkliger Abstand Gerätemitte → Anschlusspunkt an beiden Enden | `wiring` `FreeGrid::lay`, `kGridMm`, `kBundleCost` | Projektdefinition, Idee aus OpenECraft #275 (Bündel entlang der Rückwand) | keine Biegeradien, keine Trennung Leistung/Signal; von Geräten eingeschlossene Geräte: „ohne Weg“ |
| W7 | Bündeldurchmesser D = √(Σ d² / 0,75): Σ d² der Leitungen eines Rasterabschnitts (W4), Packungsdichte 0,75 angenommen | `wiring` `FreeGrid::bundles` | Projektdefinition (Richtwert) | – |
| W9 | Drahtliste: jede Leitung aus W2 als Zeile in der Reihenfolge, in der Prim sie findet (das Netz wächst vom ersten Gerät aus, das nächstgelegene Gerät zuerst); Länge = Weglänge aus W3 bzw. W6 | `wiring` `route`, `PanelWireRun` | Projektdefinition (Verdrahtungsreihenfolge Gerät für Gerät) | Reihenfolge innerhalb eines Geräts (welcher Anschluss zuerst) nicht bestimmt |
| W10 | Zuschnitt = ⌈(Weglänge + 2 · Reserve je Ende) / 10 mm⌉ · 10 mm; Reserve vom Aufrufer (Vorgabe 0) | `wiring` `PanelWireRun::cutLength` | Projektdefinition | ohne Weg: 0 |
| W11 | Aderendhülse mit Kunststoffkragen (DIN 46228-4) je Querschnitt, Typ Phoenix Contact AI (Querschnitt mm² – Hülsenlänge mm): 0,25 – 8; 0,34 – 8; 0,5 – 8; 0,75 – 8; 1 – 8; 1,5 – 8; 2,5 – 8; 4 – 10; 6 – 12; 10 – 12; 16 – 12; 25 – 16; 35 – 16 | `wiring` `PanelWireRun::ferrule` | Herstellerkatalog Phoenix Contact, Typbezeichnung „AI <Querschnitt>-<Länge>“ | andere Querschnitte: leer; Farbe (Farbcode des Herstellers) nicht angegeben |
| W8 | Kabelbinder: je gerader Strecke, auf der mindestens zwei Leitungen zusammenliegen, max(1, round(L / 150 mm)) | `wiring` `FreeGrid::bundles`, `kTieSpacingMm` | Projektdefinition (übliche Bindeabstände 100–200 mm) | Ecken und Abzweige ohne eigenen Binder |

## Einrasten und Anreihen

| ID | Beziehung | Codestelle | Herkunft, Quelle | Grenze |
|---|---|---|---|---|
| E1 | Auf der Hutschiene sitzt das Gerät mit seinem Mittenversatz auf der Schienenmitte: Gerätemitte = Schienenmitte + Mittenversatz (`middle_offset_mm`, EPLAN „Mittenversatz“, ID 22215); ohne Angabe die Gerätemitte | `layout` `railSnap` | Q4 (Montagedaten: Verschiebung in der Frontansicht gegenüber der Tragschienenmitte); Vorzeichen Projektdefinition (positiv = Gerät höher), die Hilfe nennt keins | nur waagerechte Schienen |
| E2 | In 3D liegt die Rückseite eines Geräts auf der Schiene um die Aufklipshöhe (`snap_height_mm`, EPLAN ID 22211) hinter der Schienenoberkante, nicht tiefer als die Platte | `3d` `build` | Q4 (Bezug Schienenoberkante); Richtung Projektdefinition | – |
| E3 | Bündig anreihen: liegt eine freie Kante eines Nachbarn (Gerät auf derselben Schiene; frei auf der Platte auch Kanal oder C-Profil) höchstens 4 mm entfernt, rastet das Gerät an ihr an; frei auf der Platte richtet es sich außerdem an Ober-/Unterkante bzw. Seite des Nachbarn aus, wenn diese ebenso nahe liegt; nie auf ein anderes Teil | `layout` `railSnap`, `kFlushCatchMm` | Projektdefinition | Schienenanfang fängt weiter bis 15 mm |

## Leitungsquerschnitt zur Absicherung (Plan prüfen)

Codestelle: `sources/plancheck/plancheck.cpp` (`checkWireProtection`, `capacityOf`, `ratedCurrentOf`).

| ID | Beziehung | Codestelle | Herkunft, Quelle | Grenze |
|---|---|---|---|---|
| F1 | Bedingung In ≤ Iz für jede Leitung hinter dem Schutzorgan | `checkWireProtection` | Literaturformel, Q6 (Ib ≤ In ≤ Iz; bei Leitungsschutzschaltern ist I2 = 1,45·In, damit gilt I2 ≤ 1,45·Iz von selbst) | Ib (Betriebsstrom) und Kurzschlussschutz werden nicht geprüft |
| F2 | Iz(30 °C) für Kupfer, PVC 70 °C, Verlegeart B1 (Aderleitungen im Rohr/Kanal), 3 belastete Adern: 1,5 → 15,5 A; 2,5 → 21; 4 → 28; 6 → 36; 10 → 50; 16 → 68; 25 → 89; 35 → 110; 50 → 134; 70 → 171; 95 → 207; 120 → 239 | `capacityOf` | Literaturwerte, Q7 (1,5–4 mm² am Auszug geprüft); ab 6 mm² Werte von IEC 60364-5-52 Tabelle B.52.4 Spalte B1, **nicht am Dokument geprüft** | zwischen Tabellenwerten der kleinere |
| F3 | 0,75 mm² → 6 A, 1 mm² → 10 A (30 °C, mehradrig, 3 belastete Adern) | `capacityOf` | Literaturwerte, Q8 (Tabelle 12-1, Kategorie B); B1 nennt keine Werte unter 1,5 mm² | konservativ |
| F4 | Iz(40 °C) = 0,87 · Iz(30 °C) | `capacityOf` | Umrechnungsfaktor für 70 °C Leitertemperatur bei 40 °C Umgebung, Q8 (Tabelle 12-2) | Schrankinnentemperatur fest 40 °C wie EN 60204-1; Häufung nicht berücksichtigt |
| F5 | Bemessungsstrom: `rated_current_a` → `In=` aus `sim_params` → Siemens-Bestellnummer `5S[YLJP]` + Pole + zwei Ziffern In (5SY4310-7 → 10 A) → größter Strom im Text („C16“, „B 10 A“, „8A“) von Beschreibung, Bezeichnung, Kommentar, Kennzeichen | `ratedCurrentOf` | Projektdefinition; Siemens-Bestellschlüssel 5SY/5SL | Motorschutzschalter mit Einstellbereich: der größte Wert im Text |

## Spannungsfall (Plan prüfen)

| ID | Beziehung | Codestelle | Herkunft, Quelle | Grenze |
|---|---|---|---|---|
| V1 | ΔU = k · I · R mit R = ρ_70 · L / S; k = 2 bei Wechsel- oder Gleichstrom (Hin- und Rückleiter), k = √3 bei Drehstrom; cos φ = 1, Reaktanz vernachlässigt | `checkWireProtection` (Teil Spannungsfall) | IEC 60364-5-52 Anhang G (Q9); Umsetzung nach KoperCad `novo-checks/src/calc/drop.rs` (Q10) | rein ohmsch; bei großen Querschnitten (> 50 mm²) und cos φ < 1 zu klein |
| V2 | ρ_70 = 0,017241 · (1 + 0,00393 · (70 − 20)) Ω·mm²/m = 0,02063 Ω·mm²/m (Kupfer bei 70 °C Leitertemperatur, PVC) | dto. | Q9, Q10 | – |
| V3 | Strom I = Bemessungsstrom In des vorgeschalteten Schutzorgans (F5); Länge L aus der Kabellänge (Zahl, Einheit m, sonst mm/cm umgerechnet); Querschnitt S = kleinster Leiterquerschnitt der Adern, sonst aus dem Kabeltyp („5G2,5“, „4x1,5“) | dto. | Projektdefinition (Auslegung auf In, nicht auf den Laststrom) | ohne Länge, Querschnitt oder Schutzorgan keine Prüfung |
| V4 | Nennspannung U: Elementinfo `nominal_voltage_v` des Schutzorgans, sonst 400 V bei drei oder mehr abgehenden Potentialen (Drehstrom), sonst 230 V | dto. | **Vorgabe** (Projektdefinition) | 24-V-Kreise brauchen `nominal_voltage_v` = 24 |
| V5 | Grenze: Elementinfo `voltage_drop_limit_pct` des Schutzorgans, sonst **3 % (Vorgabe)**; Warnung über der Grenze, Fehler über dem Doppelten; größte Länge L_max = Grenze / ΔU% · L | dto. | Vorgabe; IEC 60364-5-52 G nennt 3 % (Beleuchtung) und 5 % (übrige Verbraucher) am öffentlichen Netz (Q9) | – |

## Sicherheitsfunktion, Performance Level (Skript `safetyFunction`)

| ID | Beziehung | Codestelle | Herkunft, Quelle | Grenze |
|---|---|---|---|---|
| S1 | MTTFd eines Bauteils = B10d / (0,1 · n_op); n_op = Tage · Stunden · 3600 / Taktzeit | `Iso13849::Component::years`, `operationsPerYear` | ISO 13849-1 Anhang C.2 (Q11), Umsetzung nach KoperCad `novo-safety` (Q10) | – |
| S2 | Kanal: 1/MTTFd = Σ 1/MTTFd_i, höchstens 100 Jahre; zwei ungleiche Kanäle (Kat. 3, 4): MTTFd = 2/3 · (M1 + M2 − 1/(1/M1 + 1/M2)); Stufen niedrig 3–10, mittel 10–30, hoch ≥ 30 Jahre | `channelMttfd`, `calculate` | ISO 13849-1 Anhang D (Q11) | unter 3 Jahren unzulässig |
| S3 | DCavg-Stufen: keine < 60 %, niedrig ≥ 60 %, mittel ≥ 90 %, hoch ≥ 99 %; CCF ≥ 65 Punkte für Kat. 2–4 | `dcRank`, `calculate` | ISO 13849-1 Tab. 5, Anhang F (Q11) | – |
| S4 | PFHd: Kat. B/1 λd = 1/(MTTFd · 8760 h); Kat. 2 λd · (1 − DC); Kat. 3/4 2 · a · b · t_ce + β · λd mit β = 0,02 (CCF erfüllt, sonst 0,1), T_M = 20 Jahre, MTTR = 8 h | `pfhdModel` | **Näherung** aus KoperCad (Q10), **nicht** Tabelle K.1 der Norm | Planungshilfe; Nachweis mit SISTEMA oder Tabelle K.1 |
| S5 | PL aus PFHd: e < 10⁻⁷, d < 10⁻⁶, c < 3·10⁻⁶, b < 10⁻⁵, a < 10⁻⁴; begrenzt durch die Kategorie (B → b, 1 → c, 2/3 → d, 4 → e); Reihenschaltung: PFHd addieren, kleinster PL begrenzt | `fromPfhd`, `evaluate` | ISO 13849-1 Tab. 2 und Abschnitt 6.3 (Q11) | – |
| S6 | Erforderlicher PL aus dem Risikographen: S1F1P1 a, S1F1P2 b, S1F2P1 b, S1F2P2 c, S2F1P1 c, S2F1P2 d, S2F2P1 d, S2F2P2 e | `requiredPl` | ISO 13849-1 Anhang A (Q11) | – |

## Quellen

- Q1: IEC/TR 60890:2022 bzw. DIN VDE 0660-507, *A method of temperature-rise verification of low-voltage switchgear
  and controlgear assemblies by calculation* – Flächenfaktor b je Fläche (Tabelle zur wirksamen Kühlfläche A_e);
  Verfahren der Erwärmungsberechnung nach DIN EN 61439-1, Abschnitt 10.10.
- Q2: Rittal, *Technisches Systemhandbuch / Handbuch Klimatisierung* – Q_S = k · A · ΔT, Flächenformeln der
  Aufstellarten 1–7 nach IEC/TR 60890, Wärmedurchgangszahlen k, Lüfterformel V̇ = f · Q_V / ΔT mit f = 3,1 m³·K/(W·h)
  bis 100 m über NN.
- Q3: Stoffwerte trockener Luft bei etwa 30 °C und 1013 hPa: ρ ≈ 1,16 kg/m³, c_p ≈ 1005 J/(kg·K) (VDI-Wärmeatlas,
  Stoffwerte von Luft).
- Q4: EPLAN-Hilfe, Artikelverwaltung, Registerkarte Montagedaten und Thermische Auslegung:
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_montagedaten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_verlustleistungprinzip.htm
- Q5: DIN EN 50525-2-31, Kabel und Leitungen bis 450/750 V – Einadrige Leitungen ohne Mantel mit thermoplastischer
  PVC-Isolierung (H05V-K, H07V-K): Grenzwerte des mittleren Außendurchmessers.
- Q6: DIN VDE 0100-430 / IEC 60364-4-43 (Schutz bei Überstrom) und DIN EN 60204-1 Abschnitt 7.2 und Tabelle 6
  (Strombelastbarkeit in Maschinen, 40 °C).
- Q7: DIN VDE 0298-4, Tabelle 3, Auszug in Voltimum, *Querschnittsbemessung von Leitungen und Kabeln – Teil 1*,
  Tabelle 6 (Verlegeart B1: 1,5 mm² 15,5 A, 2,5 mm² 21 A, 4 mm² 28 A bei 3 belasteten Adern),
  https://content.voltimum.com/de/137631/201108022505.pdf
- Q8: Lapp, Technische Tabellen T12, Tabelle 12-1 (DIN VDE 0298-4 Tabelle 11, 30 °C) und Tabelle 12-2
  (Umrechnungsfaktor 40 °C bei 70 °C Leitertemperatur: 0,87).
- Q9: IEC 60364-5-52 / DIN VDE 0100-520, Anhang G (Spannungsfall in Verbraucheranlagen).
- Q10: KoperCad, https://github.com/Herepiggypiggypiggy/KoperCad (MIT), `crates/novo-checks/src/calc/drop.rs` und `crates/novo-safety/src/` (pl.rs, subsystem.rs, components.rs).
- Q11: DIN EN ISO 13849-1:2023, Sicherheitsbezogene Teile von Steuerungen – Allgemeine Gestaltungsleitsätze (Anhänge A, C, D, F, K).
