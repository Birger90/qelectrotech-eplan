# Formelregister des Rechenkerns

Stand Fork 1.29.0 (16.11.2026). Hier steht jede Beziehung, die `sources/simulation/` rechnet, mit Codestelle,
Herkunft, Beleg, Gültigkeitsgrenze und der Prüfung, die sie abdeckt. Jede Zeile hat mindestens einen Beleg; alle
Belege stehen mit Fundstelle am Ende. F-Nummern verweisen auf das Quellenregister des externen Pakets
„Claude_Simulation_Roadmap“ (`FORMELQUELLEN.md`, Stand 15.11.2026), S-Nummern auf dessen Primärquellen (hier mit
Fundstelle übernommen), QS-Nummern auf Quellen, die nur dieser Kern braucht.

**Herkunftsarten** (wie im Paket): **Literaturformel** = fachlich belegt; **eigene Ableitung** = nachvollziehbare
Umformung der genannten Grundlagen (Herleitung unten); **Modellnäherung/Projektdefinition** = ausdrücklich eigene
Wahl, der Beleg nennt nur Grundlage oder Vergleich. Ein Beleg für einen Grundsatz belegt nicht unsere Parameter.
**Vorgabewerte sind Projektdefinitionen, keine Messwerte** (Abschnitt „Vorgaben“). Für Genauigkeitszusagen fehlen
weiterhin unabhängige Messdaten.

Codestellen: `phys` = `simulationphysics.h`, `model` = `simulationmodel.cpp`, `units` = `simulationunits.h`,
`ref` = `simulationreference.cpp`. Prüfungen = Namen aus `--simulate-reference`.

## Bezugszustände und Zeichen

| Größe | Wert im Kern | Bezug | Beleg |
|---|---|---|---|
| Umgebungsdruck P_ATM | 101300 Pa | alle Überdrücke (`p:`) beziehen sich darauf | Projektdefinition; S01 (Einheit), F01 |
| Normdichte ρ_N der Normliter (qNN, qn_ac, qns_max, qLeak) | 1,1845 kg/m³ | technischer Normzustand von FluidSIM: 20 °C, 100000 Pa, R_N = 288 J/(kg·K) | QS05 („Durchflussmodell nach ISO 6358“) |
| Gas in Kammern | R = 287 J/(kg·K), T = 293,15 K (isotherm) | – | S03; FluidSIM nennt R_N = 288 (Unterschied 0,35 %) |
| kritisches Druckverhältnis b | 0,4 für alle pneumatischen Widerstände | wie FluidSIM | QS05 („Durchflussmodell der Komponenten mit konstantem Querschnitt“) |
| Zeichen | Durchfluss `q:` positiv **in** das Bauteil; Zylinderweg 0 = eingefahren; Moment/Winkel positiv = Druck an A | – | F02, F41; S02 |
| Einheiten | SI intern; Kennwerte nach ihrer `unit` umgerechnet, nie nach Zahlenwert | – | S01; F01 |

## Einheiten, Netze, Messgrößen

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| K01 | p_abs = p_gauge + P_ATM; Umrechnung nur über die angegebene Einheit | `units` `convert`, `model` `paramIn` | F01 | Definition; S01 | P_ATM 101300 Pa ist Projektvorgabe (Paket: 100000 Pa) | „Einheiten nach Angabe“ |
| K02 | ein Netz = ein Druck; Σ Massen- bzw. Volumenströme im Knoten = Speicherung | `model` `buildNets`, `book` | F02 | Anschlussvertrag; S02, S04 | Luft kg/s, Öl m³/s getrennt; gemischte Netze abgewiesen | „Netze, Stabilität, Ereignisse (R03)“ |
| K03 | Messgrößen p, pa, q, x, v, F, pos, ω, φ, τ, P, Fhold, hold | `model` `signalList`, `signalValue` | F41, F49 | eigene Schnittstellendefinition; S02 | Abgriff rückwirkungsfrei; q nur bei `setRecordFlows` | „Messgrößen“ |
| K04 | Kennwertbereiche (Volumen, Massen > 0, Zeiten ≥ 0 …), Abbruch bei NaN | `model` `inRange`, `fail` | F42 | eigene Abnahmeentscheidung; S31 | keine Literaturwerte | „Ungültige Werte“, „Projekt mit ungültigen Daten“ |

## Pneumatik

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| P01 | isotherme Kammer: V·dp/dt + p·dV/dt = R·T·Σṁ | `phys` `gasNode` | F03, F04 | eigene Ableitung; S03, S04 | konstante Temperatur, keine Wärmebilanz; p ≥ 1000 Pa abs | GasFill, GasCharge, CylinderCycle |
| P02 | Durchfluss nach ISO 6358: ṁ = C·p₁·ρ_N für r = p₂/p₁ ≤ b, sonst zusätzlich ·√(1 − ((r − b)/(1 − b))²) (Ellipse, m = 0,5) | `model` `massFlow` | F26–F28 (Düsengrundlage) | Literaturformel; QS01 (Anhang I, Formeln I.2, I.3, m = 0,5 elliptisch), QS05 | b = 0,4 fest, Temperaturfaktor √(T₀/T₁) = 1, kein Öffnungsdruck Δp_c | GasCharge, CylinderCycle |
| P03 | Leitwert aus dem Normal-Nenndurchfluss: C = qNN/(p₁·φ(r)) mit p₁ = 6 bar + P_ATM, p₂ = 5 bar + P_ATM | `model` `conductanceFromQnn` | – | eigene Ableitung (unten); Messbedingung QS03, Umrechnung QS05 | qNN in Normliter/min (ρ_N wie oben) | CylinderCycle |
| P04 | Leitwert für einen Durchfluss q bei p₁ → p₂: C = q/(p₁·φ(p₂/p₁)) | `model` `conductanceAt` | – | eigene Ableitung aus P02 | wie P02 | Vakuum (R10) |
| P05 | Druckluftbehälter: zusätzliches Volumen, Anschlüsse über großen Leitwert verbunden | `model` (accumulator, Luft) | F04 | Modellnäherung; S04 | keine Wärme, kein Druckverlust im Behälter | Projekt Speicher |
| P06 | Schalldämpfer, offene Anschlüsse: Leitwert gegen Atmosphäre (P02) | `model` `m_silencer_c` | F10 | Modellnäherung; QS05 (Schalldämpfer-qnN gegen p_amb, Ausnahme 1 in QS03) | Geräusch nicht | Projekt Blindstopfen |

## Leitungen, Kupplungen, Aufbereitung (R05)

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| L01 | laminar (Re ≤ 2000): Δp = 128·μ·L·q/(π·d⁴), Re = ρ·v·d/μ, v = 4q/(πd²); Öl μ = ρ·ν mit ν = 22 mm²/s, ρ = 870 kg/m³; Luft μ = 1,81·10⁻⁵ Pa·s | `phys` `PipeFlow::dp`, `model` (line) | F24 | Literaturformel; S12 (Poiseuille, Viskositätstabelle); ν nach QS14 (VG 22) und QS05 (Bezugsöl HLP 22) | Newtonsches Fluid, ausgebildete Strömung im Kreisrohr; ρ 870 kg/m³ ist Projektdefinition | Leitungen, Kupplungen, Aufbereitung (R05) |
| L02 | turbulent (Re ≥ 4000): Δp = λ·L/d·ρ·v·\|v\|/2, 1/√λ = −1,8·log₁₀((ε/d/3,7)^1,11 + 6,9/Re); 2000 < Re < 4000: λ linear in Re zwischen 64/2000 und λ(4000) | `phys` `PipeFlow::dp`, `lambdaTurbulent` | F25 | Literaturformel; S13 (Darcy-Weisbach, Übergangsbereich 2000/4000), QS13 (Haaland) | Haaland-Näherung (±2 % zu Colebrook laut QS13); ε = 15 µm ist Projektdefinition; keine Höhenänderung, keine Fluidträgheit | R05 |
| L03 | Umkehrung q(Δp) durch Bisektion zwischen 0 und dem laminaren Fluss (obere Schranke, da λ ≥ 64/Re); ungerade in Δp; Luft: ṁ = ρ_m·q mit ρ_m = p_m/(R·T), p_m = (p₁ + p₂)/2; Leitungsvolumen π d²/4·L je zur Hälfte an die Enden | `phys` `PipeFlow::flow`, `model` Schritt | F02, F03, F18 | eigene Ableitung (unten); S03, S17 (Kompressibilität unter Mach 0,3 vernachlässigbar) | Luft nur bis etwa Mach 0,3, isotherm | R05 |
| L04 | Modellwahl je Leitung: ohne Angaben ideal (Anschlüsse ein Netz), mit rT/qNN Widerstand (P02/H03), mit length und d_inner Rohr (L01–L03); Kupplung: coupled 0 = getrennt, seal 1 = beide Hälften dicht, seal 0 = beide Hälften entlüften (Schalldämpferleitwert); Drehdurchführung: Anschlüsse gleicher Höhe je ein Kanal | `model` `lineMode`, `channelPairs`, `buildNets` | F10, F13 | Projektdefinition; QS05 (Schlauchleitung mit hydraulischem Widerstand, einfache Verbindungen ohne Druckabfall); Symbolbedeutung QS15 | Kupplungszustand fest für den Lauf; Kanalzuordnung aus der Symbolgeometrie | R05 |
| L05 | verstopftes Bauteil der Aufbereitung: clogged 1 → qNN_clogged (Vorgabe 10 l/min), Öl rT_clogged (Vorgabe 1000·rT) | `model` passthrough | F16 | Projektdefinition nach dem FluidSIM-Fehlermodell „Bauteil verstopft“ (QS05: eingeschränkter Normal-Nenndurchfluss, Vorgabe 10 l/min) | Faktor 1000 für Öl ist gewählt, nicht belegt; keine Differenzdruckanzeige | R05 |

## Hydraulik

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| H01 | Ölknoten: V/E·dp/dt = Σq − dV/dt, E = 7·10⁸ Pa | `phys` `oilNode` | F18 | eigene Näherungsableitung; S14, S04 | konstantes E, einphasig | HydraulicCharge, PumpFill |
| H02 | Hohlraum: unter P_ATM − 0,9 bar fehlendes Öl als Volumen bilanziert, beim Zufluss zuerst gefüllt | `phys` `oilNode` (cavity) | F42 | Modellnäherung; S14 (Gasanteile), S04 | kein Dampfdruck, keine Entgasung | „Unterdruck im Öl“ |
| H03 | Blende geglättet: q = k·Δp/(Δp² + p_s²)^¼ mit k = 1/(60000·√(10⁵·rT)), p_s = 0,3 bar; für \|Δp\| ≫ p_s gilt Δp = rT·Q² | `phys` `orificeFlow`, `model` `volumeFlow` | F11, F52 | Literatur-Regularisierung mit eigener Skalierung; S10 (regRoot), S11; rT nach QS05 | p_s bestimmt die Kleinsignalkennlinie, keine Viskosität | Orifice |
| H04 | Hydrospeicher polytrop: Öl(p) = V₀ − V₀·(p_pre/p)^(1/n); Volumenbilanz implizit (Bisektion) | `phys` `accumulatorOil`, `oilNodeWithAccumulators` | F21, F22 | Literatur-Modellansatz; S15, S21; implizite Lösung eigene Wahl (S32) | n fest, keine Wärmebilanz | AccumulatorCharge |
| H05 | Pumpe: q = α·D·n − GL·Δp, volle Förderung bis p_max − 1 bar, linear bis 0 bei p_max | `model` `Pump`, Schritt | F12, F16 | Literaturgrundlage (S05 S. 13–15) + eigene Begrenzungskennlinie | Band 1 bar ist Projektdefinition | PumpFill, Hydromotor (R08) |
| H06 | Hydromotor: q = α·D·ω/2π + C·Δp, τ = α·D·Δp/2π, J·dω/dt = τ − τ_L − b·ω − τ_c·tanh(ω/0,1) | `phys` `Shaft` | F12, F14, F08 | Literaturformel (S05 S. 13–15, S29) + Reibungsnäherung (S07) | Glättung 0,1 rad/s ist Projektdefinition, keine Haftphase | Hydromotor (R08) |
| H07 | Leistung: P_hyd = Δp·q, P_Welle = τ·ω, P_Leck = C·Δp² | `model` `signalValue` (P:) | F15 | Leistungsableitung; S05, S29 | – | Hydromotor (R08) |

## Mechanik

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| M01 | Zylinder: F = p_A·A_K − p_B·A_R − F_Last − F_Feder; m·dv/dt = F − F_Reibung + F_Kontakt; A_K = πd²/4, A_R = A_K − πd_s²/4 | `model` Schritt, `phys` `pistonStep` | F06, F07 | Literaturgrundlage mit eigener Vorzeichenableitung; S05 S. 7–9, S06; theoretische Kolbenkraft wie QS05 („Kolbenkraft“) | Feder als konstante Kraft (FluidSIM: Federdruck bei x = 0 und x_max, siehe Abweichungen) | CylinderCycle, Projekt Hydraulik |
| M02 | Haften/Gleiten: \|v\| < 10⁻⁴ m/s und \|F\| ≤ F_stat → v = 0; sonst Gleitreibung 0,7·F_stat und viskose Dämpfung | `phys` `pistonStep` | F53, F08 | Literatur-Modellfamilie mit eigener Zustandsfestlegung; S09 | Schwelle 10⁻⁴ m/s und Faktor 0,7 sind Projektdefinitionen | CylinderCycle |
| M03 | viskose Dämpfung aus dem FluidSIM-Bezug: d = F_ref/v_ref („Reibung bei dp_ref“, „Referenzgeschwindigkeit“) | `model` (F_ref, v_ref) | F08 | Modellnäherung; Kennwerte nach QS05 (Zylinderreibung); Reibungsfamilie S07 | nur lineare Dämpfung, Losbrechkraft separat (F_stat) | Batch FluidSIM |
| M04 | elastischer Anschlag: x < 0: F = −k·x − c·min(v,0); x > L: F = −k·(x − L) − c·max(v,0); ohne k harter Anschlag | `phys` `stopForce`, `pistonStep` | F09, F43 | eigenes Kontaktmodell; S08, S30 | Kraftsprung bei Kontaktbeginn | Endanschlag hydraulisch (R02) |
| M05 | Kammervolumen V_A = V_tot + A_K·x, V_B = V_tot + A_R·(L − x) | `model` Schritt | F06 | geometrische Ableitung; S05 | V_tot = 2 cm³ ist Projektdefinition | Hydraulik-CylinderCycle (Volumenbilanz) |
| M06 | Schwenkantrieb als Drehanalogon: τ = (p_A − p_B)·D_θ, D_θ = M_6bar/6 bar [m³/rad], I·dω/dt = τ − M_ext − f·ω (Haften wie M02); V_A = V_tot + D_θ·θ | `model` (rotary_actuator), `phys` `pistonStep` | F14, F36 | eigene Ableitung (unten); S29; Kennwerte M_6bar, I, f, M_ext nach QS05 (Schwenkzylinder) | ideale Wandlung, keine Leckage, keine Endlagendämpfung | Vakuum und Schwenk-/Greiferantriebe (R10) |
| M07 | Greifer: Ersatz durch M01 mit Rückstellfeder, Greifkraft = Kolbenkraft | `model` (actuator.gripper) | F07 | Modellnäherung; S05, S06 | keine Backenkinematik (F36 nicht angewandt) | R10 |

## Ventile

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| V01 | Schieber: Öffnung 1 − e^(−3t/T_schalt) der neuen Stellung, die alte schließt gleich; Durchfluss linear in der Öffnung; Stell-/Rückstellzeit aus `onDelay`/`offDelay` | `model` Ventilschritt | F13, F17 | PT1 aus S22, Pfadmatrix eigene Bauteilspezifikation; Stellzeiten nach QS05 (Wegeventil: Stellzeit 0,02 s, Rückstellzeit 0,03 s) | keine Überdeckung, keine Leckage; τ = T_schalt/3 ist Projektdefinition | CylinderCycle |
| V02 | Rückschlag-, Begrenzungs-, Druckregelventil: Öffnung linear zwischen p₀ und p₀ + Δp_end | `model` FlowValve-Schritt | F16 | eigenes Referenzmodell; S04; Verhalten wie QS05 („Komponenten mit druckabhängigem Querschnitt“: C proportional zum Weg gegen die Feder) | keine Kegelmasse, Hysterese, Pilotdynamik | Projekt Druckbegrenzung, Projekt Speicher |
| V03 | Wechselventil (ODER): der Eingang mit höherem Druck strömt nach 2; Zweidruckventil (UND): der mit niedrigerem | `model` FlowValve-Schritt | – | Projektdefinition der logischen Funktion; QS05 (Folien „Wechselventil“, „Zweidruckventil“); Vergleich QS12 | keine Zwischenstellung, kein Umschaltdruck | Projekt Speicher |
| V04 | Steueranschluss schaltet ab p_gauge > 1,5 bar | `model` `P_PILOT` | – | Projektdefinition; Größenordnung QS10 (Steuerdruck je Typ 1,5 … 3 bar); FluidSIM-Symbole: F_spring/areaPilot = 20 N/1,4 cm² = 1,43 bar | keine Steuerflächen-Kraftbilanz | CylinderCycle |

## Stetig-, Senkbrems- und Zuschaltventile (R07/R09)

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| V05 | Schieber s ∈ [−1, 1] folgt dem Sollwert u/10 V als PT1: s ← z + (s − z)·e^(−dt/τ), z = clamp(u/10, −1, 1); Kantenöffnung x = clamp((±s − ü)/(1 − ü), 0, 1) mit relativer Überdeckung ü (> 0 Überdeckung, < 0 Unterdeckung), getrennt für P- und T-Kanten | `phys` `pt1`, `edgeOpening`, `model` PropValve | F13, F17 | PT1 aus S22 (exakte Lösung für konstantes Ziel), Kantenmatrix und Überdeckung eigene Bauteilspezifikation nach QS05 („4/3-Wege-Proportionalventil“: ±10 V, Mittelstellung bei 0 V sperrt, Überdeckung relativ zum Schieberweg) | keine Hysterese, keine Strömungskraft, kein Lageregelkreis; τ = 4 ms (Regelventil, QS05: Stellzeit < 12 ms für 0–100 %) bzw. 10 ms sind Projektdefinitionen | Stetig-, Senkbrems- und Zuschaltventile (R07/R09) |
| V06 | Kante als Blende (H03) mit Fläche a = x (Rechteckkerbe) bzw. a = x² (Dreieckkerbe), rT bezogen auf die voll offene Kante: Δp = rT·Q²/a²; Leckage je Kante q = G·Δp | `model` PropValve-Schritt | F11, F52 | Modellnäherung; QS05 (Rechteck linear, Dreieck progressiv; Widerstand je voll offener Kante, Leckage je Kante) | a = x² ist die einfachste progressive Form, nicht die gemessene Kerbengeometrie | R07/R09 |
| V07 | Senkbremsventil: öffnet P→T mit Anteil clamp(((p_P − p_T) + ratio·(p_X − p_T) − p₀)/Δp_end, 0, 1); Rückschlag T→P frei (rT_cv); Leckage G·(p_P − p_T) | `model` FlowValve (counterbalance) | F16 | eigenes Referenzmodell (F16) mit Steuerflächenverhältnis; Kennwerte nach QS05 („Senkbremsventil“: Öffnungsdruck, Druckregelbereich, Flächenverhältnis, Rückschlagventil) | keine Kolbenmasse, keine Dämpfung (Schwingneigung realer Senkbremsventile fehlt) | R07/R09 |
| V08 | Druckzuschaltventil: öffnet P→A mit Anteil clamp((p_X,gauge − p₀)/Δp_end, 0, 1); ohne X steuert P | `model` FlowValve (sequence 01/02) | F16 | eigenes Referenzmodell; Funktion nach QS05 („Druckzuschaltventil“: öffnet bei Solldruck an X) | kein Leckölanschluss, keine Dynamik | R07/R09 |

## Vakuum (R10)

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| U01 | Ejektor-Anteil s = min(1, p₁,gauge/dp_max); Luftverbrauch über P04 mit qn_ac bei 600 kPa | `model` Ejector | F35 | Modellnäherung; Kennwerte nach QS05 (Vakuumsaugdüse: „Maximales Vakuum“, „Betriebsdruck für max. Vakuum“, „Luftverbrauch bei 600 kPa“, „Max. Saugvolumenstrom“) | über dp_max kein Abfall des Vakuums | R10 (3,5 bar) |
| U02 | Saugstrom ṁ = ṁ_max·(p₂ − P_ATM − pu_min·s)/\|pu_min\|, ṁ_max = qns_max·ρ_N | `model` Ejector | F35 | Modellnäherung: Endpunkte wie QS07 (P_max bei Q = 0, Q_max bei offenem Anschluss), Linearität eigene Wahl; Paket F35: Ejektorfluss ist ein eigenes, zu identifizierendes Kennfeld | nicht an Messkennlinien identifiziert; Herstellerkurven gelten bei festem Versorgungsdruck (QS07: 0,45 MPa) | R10 (−0,800/−0,555 bar) |
| U03 | Haltekraft F = (P_ATM − p)·π·d²/4, hält bei F ≥ s·m·g | `model` SuctionCup | F35 | Bilanz-/Kraftableitung; S03, S05; Formel und Sicherheitsfaktor wie QS08 (W = P·S·0,1·1/t) und QS09 (F_H, S); Kennwerte d, m, s nach QS05 (Vakuumsaugnapf) | keine Beschleunigung a (QS09: m·(g + a)), keine Querkräfte, keine Reibung bei senkrechtem Ansaugen | R10 (25,13 N, 2 kg, s = 1,5) |
| U04 | Saugerleckage über P04: qLeak bei −0,6 bar; ohne Werkstück 50 l/min bei 0,1 bar (Ø 20 mm, linear mit d) | `model` SuctionCup | F35 | Projektdefinition; Gleichgewicht Leckage = Saugstrom wie QS06; Kennwert „Leckage“ nach QS05 (ohne Bezugsdruck angegeben) | Bezugsdruck −0,6 bar und offener Leitwert sind gewählt, nicht gemessen | R10 (Leckage, ohne Werkstück) |
| U05 | Vakuumschalter: ein bei p ≤ p₀ (< 0), aus bei p > p₀ + p_hyst | `model` PressureSwitch | – | Projektdefinition; Verhalten nach QS05 (Druckschalter: als Vakuumschalter „betätigt, wenn der Schaltdruck unterschritten wird“) | ohne Schaltzeit; Hysterese mindestens 1 kPa (FluidSIM-Vorgabe 0) | R10 (Vakuumschalter) |

## Numerik, Elektrik, Steuerung

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| N01 | expliziter Euler dt = 10 µs; Mechanik halbimplizit (v zuerst) | `model` `step`, `phys` `pistonStep` | F44 (Vergleich), F51 | eigene Wahl; S32 | steife Netze werden diagnostiziert, nicht gelöst | GasCharge (Ordnung 1), Konvergenzprüfungen |
| N02 | Schwingungswächter: 200 Schritte mit wechselndem Vorzeichen und \|Δp_k\| ≥ 0,9·\|Δp_k−1\| → Abbruch | `phys` `OscillationWatch` | F42 | Projektdefinition; S32 (Stabilität expliziter Verfahren) | Diagnose, keine Lösung | R03 |
| N03 | Relais: Anzug 10 ms, Abfall 8 ms; SPS und GRAFCET alle 10 ms | `model` `T_PULL_IN`, `T_DROP_OUT` | F40 | Projektdefinition; Größenordnung QS11 (Industrierelais 7/3 ms); Zyklussemantik S31, S35 | keine Prellzeit | PeriodicCounter, Projekt GRAFCET |
| N04 | Fehlermaße der Vergleiche: Schlauch um die stückweise lineare Gegenkurve, Grenzen je Prüfung | `ref` | F51 | eigene Abnahmeentscheidung; S33 (stückweise Interpolation/Quadratur) | Grenzen stehen in jeder Prüfung | alle Vergleichsprüfungen |
| N05 | Handrechnungen der Prüfungen (Ausfahrzeit = Hub·A/Q, Gleichgewicht Saugstrom = Leckage per Bisektion, Δp = 2π(τ + bω)/D …) | `ref` | F50 | eigene Ableitung aus den Zeilen oben; S05, S29 | – | jeweilige Prüfung |

## Herleitungen der eigenen Ableitungen

- **P03/P04:** P02 mit φ(r) = 1 für r ≤ b und √(1 − ((r − b)/(1 − b))²) sonst; am Messpunkt ṁ = qNN·ρ_N (Normliter in
  m³/s mal Normdichte) gleichsetzen: C = (qNN/60000)/(p₁·φ(p₂/p₁)). Für qNN gilt p₁ = 7,013 bar abs, p₂ = 6,013 bar abs
  (QS03: Eingangsdruck 6 bar, Ausgangsdruck 5 bar, Überdrücke).
- **H03:** Für \|Δp\| ≫ p_s ist q = k·√Δp; mit Δp = rT·Q² (Q in l/min, rT in bar·min²/l², Δp in bar) folgt
  q[m³/s] = √(Δp[Pa]/(10⁵·rT))/60000, also k = 1/(60000·√(10⁵·rT)).
- **M06:** Arbeit des Fluids p·dV gleich mechanischer Arbeit τ·dθ (S29) ergibt dV/dθ = τ/Δp = D_θ. Aus dem Kennwert
  „Moment bei 6 bar“ folgt D_θ = M_6bar/6·10⁵ Pa; Kammervolumen und Bewegungsgleichung dann wie M01/M05 mit θ statt x.
- **U02:** Lineare Gerade durch (p₂ = P_ATM + pu_min·s, ṁ = 0) und (p₂ = P_ATM, ṁ = ṁ_max·s). Ihre Steigung
  ṁ_max/\|pu_min\| hängt nicht von s ab; für s → 0 bleibt deshalb derselbe lineare Rückweg zur Luft (stetig, ohne
  Fallunterscheidung).
- **L03:** Δp(q) ist für jedes Re streng steigend (laminar linear, turbulent ∝ λ·q², λ fällt langsamer als 1/q), also
  eindeutig umkehrbar. Weil λ ≥ 64/Re überall gilt, ist Δp(q) ≥ Δp_laminar(q) und damit q(Δp) ≤ Δp·π d⁴/(128 μ L):
  Startintervall der Bisektion.
- **V05/V06:** Bei Unterdeckung ü < 0 sind in Mittelstellung beide P-Kanten mit x = −ü/(1 − ü) offen (−20 % → 1/6);
  zwei gleiche Blenden parallel wirken wie eine mit doppelter Fläche, also Δp = rT·Q²/(2x)². Bei Überdeckung ü > 0
  bleibt die Kante bis |s| = ü zu (Totzone), dann x = (|s| − ü)/(1 − ü).
- **V07 (Prüffall):** Mit X am Pumpendruck ist der Antrieb (1 + ratio)·p; Gleichgewicht Anteil(p)·√(p/rT) = Q
  (bar, l/min) per Bisektion.
- **N05 (Leckgleichgewicht):** 6·(g − p_min)/\|p_min\| = 1,8·φ((P_ATM + g)/P_ATM)/φ((P_ATM − 0,6 bar)/P_ATM) in l/min,
  nach g gelöst (Bisektion, 100 Schritte). Die Normdichte kürzt sich heraus.

## Abweichungen von FluidSIM (bekannt, nicht nachgebildet)

- Zylinderfeder: FluidSIM führt „Federdruck bei x = 0“ und „bei x = x_max“ (lineare Kennlinie); der Kern rechnet eine
  konstante Federkraft (15 % von A_K·6 bar). Endlagendämpfung („Dämpfungslänge“) und Zylinderleckage fehlen (QS05).
- Sauger: FluidSIM senkt und hebt per Klick („Position“, Vorgabe 0 = angehoben); der Kern nimmt `object` 1
  (Werkstück liegt an) als Vorgabe. Der Saugvolumen-Kennwert des Ejektors mit Abstoßeinrichtung („Volumen“) und das
  Abstoßen fehlen.
- Pilotventile: FluidSIM rechnet eine Kraftbilanz aus Steuerfläche und Feder; der Kern schaltet ab 1,5 bar (V04).
- Gaskonstante 287 statt 288 J/(kg·K) in den Kammern (0,35 %).

## Befunde der Quellenprüfung (15.11.2026)

- **b = 0,4 statt 0,3:** FluidSIM nimmt für alle Komponenten b = 0,4 an (QS05). Der Kern und der Python-Prototyp
  hatten 0,3; seit 1.27.1 gilt 0,4. Weil C aus qNN am Messpunkt 6 → 5 bar bestimmt wird, ändern sich nur die
  Durchflüsse abseits dieses Punkts; die Prüfprojekte verschieben sich um wenige Millisekunden (Blindstopfen
  0,262 → 0,264 s).
- **Normdichte 1,1845 kg/m³:** FluidSIM rechnet mit dem technischen Normzustand (20 °C, 1 bar). Festo-Katalogwerte des
  Normal-Nenndurchflusses beziehen sich dagegen auf DIN 1343 (0 °C, 1,01325 bar, ρ ≈ 1,292 kg/m³; QS03, QS04). Wer
  Katalogwerte statt FluidSIM-Werten einträgt, bekommt rund 9 % zu wenig Massenstrom.
- **Luftverbrauch des Ejektors** gilt bei 600 kPa (QS05), nicht beim Betriebsdruck für maximales Vakuum; seit 1.27.1 so.
- **Sicherheitsfaktoren der Hersteller** sind viel größer als 1: SMC 4 waagerecht, 8 senkrecht (QS08), Festo 1,5 mit
  Beschleunigung (QS09). Der Kern übernimmt `s` aus dem Bauteil (FluidSIM-Vorgabe 1).
- **ISO 6358:** Die Ellipsenform (m = 0,5) und die Bereichsgrenze r ≤ b sind am Normtext von ISO 6358-1:2013/Amd 1:2020,
  Anhang I, belegt (Formeln I.2, I.3). Die Grundformel des Hauptteils (ṁ = C·p₁·ρ₀·√(T₀/T₁)) ist über QS05 belegt, nicht
  am Normtext selbst geprüft.

## Vorgaben (Projektdefinitionen, keine Messwerte)

V_LINE 5 cm³ je Luftnetz, V_HYD 0,1 l je Ölnetz, V_tot 2 cm³ je Zylinderkammer, E_OIL 7·10⁸ Pa, p_s = 0,3 bar,
Pilotschwelle 1,5 bar, Haftkraft 5 % von A_K·6 bar, Gleitreibung 70 % der Haftkraft, Federkraft 15 % von A_K·6 bar,
Haftmoment des Schwenkantriebs 2 % von M_6bar, Schieberzeitkonstante 4/10 ms, Öldichte 870 kg/m³, Rauheit 15 µm, Verstopfungsfaktor Öl 1000,
Leitwert entlüftender Kupplungshälften wie Schalldämpfer, Pumpenband 1 bar, Saugervolumen 5 mm tief, offener Sauger 50 l/min bei
0,1 bar, Leckagebezug −0,6 bar, Hysterese mindestens 1 kPa, Relaiszeiten 10/8 ms, Ventil-τ = Schaltzeit/3. Kennwerte
aus FluidSIM-Bauteilen (qNN, rT, M_6bar, I, f, pu_min, qn_ac, qns_max, d, m, s …) sind Angaben der Beispielbauteile,
nicht von uns gemessen.

## Primärquellen des Pakets (hier verwendet)

Fundstellen wie im Quellenregister des Pakets, Zugriff dort geprüft am 15.11.2026.

- **S01** – BIPM: SI Brochure, 9. Auflage (https://www.bipm.org/en/publications/si-brochure). Kapitel 2 und 4:
  SI-Einheiten, Vorsätze, zugelassene Einheiten.
- **S02** – Modelica Language Specification 3.6, Verbindungen
  (https://specification.modelica.org/maint/3.6/connectors-and-connections.html). Abschnitt 9.2: Potentialgleichheit
  und Flusssumme.
- **S03** – OpenStax University Physics 2, Abschnitt 2.1 (https://openstax.org/books/university-physics-volume-2/pages/2-1-molecular-model-of-an-ideal-gas):
  ideale Gasgleichung, spezifisches R.
- **S04** – Modelica Standard Library 4.0.0, Fluid-Bilanzen
  (https://doc.modelica.org/Modelica%204.0.0/Resources/helpWSM/Modelica/Modelica.Fluid.UsersGuide.ComponentDefinition.BalanceEquations.html):
  Mass, Momentum, Energy balance.
- **S05** – H. Hatami / Bosch Rexroth: Hydraulic Formulary, 10.01.2013
  (https://www.btpco.com/download/training/Boschrexroth/hyd_formelsammlung_en.pdf). Gedruckte S. 7–9 (Kräfte), 13–15
  (Pumpen/Motoren), 20 (Rohr).
- **S06** – MSL 4.0.0, Translational.Components.Mass
  (https://doc.modelica.org/Modelica%204.0.0/Resources/helpWSM/Modelica/Modelica.Mechanics.Translational.Components.Mass.html).
- **S07** – MathWorks, Translational Friction (https://www.mathworks.com/help/simscape/ref/translationalfriction.html):
  viskoser und geglätteter Coulomb-Anteil.
- **S08** – MSL 4.0.0, ElastoGap
  (https://doc.modelica.org/Modelica%204.0.0/Resources/helpWSM/Modelica/Modelica.Mechanics.Translational.Components.ElastoGap.html).
- **S09** – MSL 4.0.0, MassWithStopAndFriction
  (https://doc.modelica.org/Modelica%204.0.0/Resources/helpWSM/Modelica/Modelica.Mechanics.Translational.Components.MassWithStopAndFriction.html):
  Haftphase, Losbrechen.
- **S10** – MSL 4.0.0, Fluid.Utilities.regRoot
  (https://raw.githubusercontent.com/modelica/ModelicaStandardLibrary/v4.0.0/Modelica/Fluid/Utilities.mo):
  y = x/(x² + δ²)^0,25.
- **S11** – OpenStax University Physics 1, Abschnitt 14.6 Bernoulli
  (https://openstax.org/books/university-physics-volume-1/pages/14-6-bernoullis-equation).
- **S14** – N. D. Manring, R. C. Fales: Hydraulic Control Systems, 2. Aufl., Wiley 2020
  (https://catalogimages.wiley.com/images/db/pdf/9781119416470.excerpt.pdf). Kapitel 1, Abschnitte 1.2–1.3.
- **S15** – MathWorks, Gas-Charged Accumulator (IL) (https://www.mathworks.com/help/hydro/ref/gaschargedaccumulatoril.html).
- **S21** – OpenStax University Physics 2, Abschnitt 3.6 adiabatische Prozesse
  (https://openstax.org/books/university-physics-volume-2/pages/3-6-adiabatic-processes-for-an-ideal-gas).
- **S22** – MSL 4.0.0, Blocks.Continuous.FirstOrder
  (https://doc.modelica.org/Modelica%204.0.0/Resources/helpWSM/Modelica/Modelica.Blocks.Continuous.FirstOrder.html).
- **S29** – OpenStax University Physics 1, Abschnitt 10.8 Rotationsarbeit und -leistung
  (https://openstax.org/books/university-physics-volume-1/pages/10-8-work-and-power-for-rotational-motion).
- **S30** – OpenStax University Physics 1, Abschnitt 9.4 Stoßarten
  (https://openstax.org/books/university-physics-volume-1/pages/9-4-types-of-collisions).
- **S31** – Modelica Language Specification 3.6, Gleichungen (https://specification.modelica.org/maint/3.6/equations.html).
  Abschnitte 8.3.5–8.3.8, 8.5, 8.6.
- **S32** – OpenModelica Users Guide 1.27.1, Solving Modelica Models
  (https://openmodelica.org/doc/OpenModelicaUsersGuide/v1.27.1/solving.html): Integrationsverfahren.
- **S33** – NIST DLMF 3.5 Quadratur (https://dlmf.nist.gov/3.5). Gleichungen 3.5.1, 3.5.2.
- **S35** – Modelica Language Specification 3.6, Operatoren
  (https://specification.modelica.org/maint/3.6/operators-and-expressions.html). Abschnitt 3.7: sample, pre.

## Zusätzliche Quellen dieses Kerns

Zugriff am 15.11.2026, sofern nicht anders vermerkt.

- **QS01** – ISO 6358-1:2013/Amd 1:2020, Pneumatic fluid power – Determination of flow-rate characteristics of
  components using compressible fluids – Part 1, Amendment 1: Effective conductance. Anhang I.1 (Kenngrößen C, b, m,
  Δp_c), I.2 (Formel I.2: gesperrte Strömung für p₂/p₁ ≤ b; Formel I.3: unterkritisch), I.3 (m = 0,5 elliptisch).
  Leseprobe: https://cdn.standards.iteh.ai/samples/75902/c0a8174bb17f4a00938cf3815ffda5c2/ISO-6358-1-2013-Amd-1-2020.pdf
- **QS02** – ISO 8778:2003, Pneumatic fluid power – Standard reference atmosphere (ANR: 20 °C, 100 kPa, 65 % rel.
  Feuchte). https://standards.iteh.ai/catalog/standards/iso/e5cdf05c-8c45-46dd-8564-301e2d1c13bc/iso-8778-2003 –
  nur zur Abgrenzung; der Kern nutzt den Normzustand von QS05.
- **QS03** – Festo: Technische Information (TECHINFO_DE), S. 7 „Normalnenndurchfluss“: Bezug DIN 1343, Prüfmedium
  Luft 20 ± 3 °C, Eingangsdruck 6 bar, Ausgangsdruck 5 bar; Ausnahmen Schalldämpfer (gegen p_amb), Niederdruck,
  Druckregelventile; Drücke als Überdruck.
  https://ftp.festo.com/Public/PNEUMATIC/SOFTWARE_SERVICE/Documentation/2023/DE/TECHINFO_DE.PDF
- **QS04** – DIN 1343:1990-01, Referenzzustand, Normzustand, Normvolumen; Begriffe und Werte (273,15 K, 101325 Pa).
  https://www.din.de/en/innovation-and-research/hydrogen/standards-search/din-1343-1187740
- **QS05** – Festo Didactic: FluidSIM 6 Hilfe (Installation `FluidSIM6/html/de/html.db`, Stand 20.03.2026). Kapitel
  „Mathematische Modelle“: „Durchflussmodell nach ISO 6358“ (technischer Normzustand T_N = 293,15 K, p_N = 100000 Pa,
  R_N = 288 J/(kg·K), ρ₀ = 1,1845 kg/m³; Leitwert C, kritisches Druckverhältnis b), „Durchflussmodell der Komponenten
  mit konstantem Querschnitt“ (Normal-Nenndurchfluss intern in C umgerechnet, b = 0,4 für alle Komponenten, C
  proportional zum Öffnungsgrad), „Komponenten mit druckabhängigem Querschnitt“; Bauteilseiten „Vakuumsaugdüse“,
  „Vakuumsaugnapf“, „Schwenkzylinder“, „Druckschalter, Vakuumschalter“, „Zylinder, doppeltwirkend“ (Zylinderreibung,
  Federdruck, Dämpfungslänge), „Wegeventil“ (Stellzeit, Rückstellzeit), Drossel-/Druckventilseiten (hydraulischer
  Widerstand in bar·min²/l²); Folien „Wechselventil“, „Zweidruckventil“, „Kolbenkraft“.
- **QS06** – SMC: Model Selection, Selection of Vacuum Ejector and Vacuum Switching Valve, S. 33 „Leakage Volume during
  Workpiece Adsorption“: Leckage = Saugstrom der Ejektorkennlinie beim gemessenen Vakuum.
  https://www.smcworld.com/assets/products/pickup/en-jp/vacuum_device/select/6-4-p0033-vacuum_en.pdf
- **QS07** – SMC: ZH-X267 Datenblatt (Ejektor), „Flow rate characteristics“: P_max bei Saugstrom 0, Q_max bei offenem
  Sauganschluss; Kurven bei 0,45 MPa Versorgung. https://static.smc.eu/pdf/ZH-X267_EU.pdf
- **QS08** – SMC: Model Selection, Vacuum pads, theoretische Hebekraft W = P·S·0,1·1/t (P in kPa, S in cm²),
  Sicherheitsfaktor t ≥ 4 waagerecht, ≥ 8 senkrecht.
  https://cdn.smcworld.com/assets/products/pickup/en-jp/vacuum_device/select/4-P0878_en.pdf
- **QS09** – Festo: Poster Vakuumgreifen (Sauggreiferauslegung): theoretische Haltekraft F_H, Sicherheitsfaktor
  S = 1,5, Beschleunigung a und Not-Aus beachten.
  https://www.festo.com/fox/net/supportportal/Downloads/9429/9240/Poster_VacuumGripping_de.pdf – Datei am 15.11.2026
  nicht abrufbar (HTTP 403), Inhalt über den Suchauszug; vor einer Freigabe am Original prüfen.
- **QS10** – Festo: Datenblätter Magnetventil VUVG, z. B. 574445 (Steuerdruck 0,15 … 0,8 MPa), 574365 (2 … 8 bar),
  574347 (3 … 8 bar): Mindeststeuerdruck hängt vom Typ ab.
  https://www.festo.com/de/de/a/download-document/datasheet/574445
- **QS11** – Finder: Serie 40, Datenblatt (Ansprech-/Rückfallzeit 7/3 ms, empfindliche Spulen bis 12/4 ms).
  https://static6.arrow.com/aropdfconversion/6b3a67a5f9ae5019535b36a903d247ecb4925eaf/s40en.pdf
- **QS12** – MathWorks: Shuttle Valve (IL) (https://www.mathworks.com/help/hydro/ref/shuttlevalveil.html): Vergleichsmodell
  „höherer Eingangsdruck auf den Ausgang“.

- **QS13** – S. E. Haaland: Simple and Explicit Formulas for the Friction Factor in Turbulent Pipe Flow. Journal of
  Fluids Engineering 105 (1983), Nr. 1, S. 89–90, doi:10.1115/1.3240948. Explizite Näherung der Colebrook-Gleichung.
- **QS14** – ISO 3448:1992, Industrial liquid lubricants – ISO viscosity classification: VG 22 = 22 mm²/s bei 40 °C
  (Bereich 19,8 … 24,2 mm²/s). Titel und Klasse nach Normtitel, Normtext hier nicht eingesehen.
- **QS15** – ISO 1219-1:2012, Fluid power systems and components – Graphical symbols and circuit diagrams – Part 1:
  Schnellkupplungen gekuppelt/getrennt, mit und ohne Rückschlagventil. Symboltabelle nicht am Normtext geprüft; die
  Zuordnung „offen = getrennt“ folgt den Symbolnamen der Sammlung (`50101530` … `50101535`).

Eine neue Formel bekommt hier ihre Zeile mit Codestelle, Herkunft, Beleg, Grenze und Prüfung, bevor sie freigegeben
wird; eine neue Quelle bekommt eine QS-Nummer mit Fundstelle und Zugriffsdatum.
