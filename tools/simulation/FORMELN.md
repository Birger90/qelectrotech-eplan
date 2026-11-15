# Formelregister des Rechenkerns

Stand Fork 1.27.0. Hier steht jede Beziehung, die `sources/simulation/` rechnet, mit Codestelle, Herkunft, Beleg,
Gültigkeitsgrenze und der Prüfung, die sie abdeckt. Die F-Nummern verweisen auf das Quellenregister des externen
Pakets „Claude_Simulation_Roadmap“ (`FORMELQUELLEN.md`, Stand 15.11.2026), die S-Nummern auf dessen Primärquellen.
QS-Nummern sind Quellen, die nur dieser Kern braucht; sie stehen am Ende.

**Herkunftsarten** (wie im Paket): **Literaturformel** = fachlich belegt; **eigene Ableitung** = nachvollziehbare
Umformung der genannten Grundlagen; **Modellnäherung/Projektdefinition** = ausdrücklich eigene Wahl. Ein Beleg für
einen Grundsatz belegt nicht unsere Parameter. **Vorgabewerte sind Projektdefinitionen, keine Messwerte**
(Abschnitt „Vorgaben“); für Genauigkeitszusagen fehlen weiterhin unabhängige Messdaten.

Codestellen: `phys` = `simulationphysics.h`, `model` = `simulationmodel.cpp`, `units` = `simulationunits.h`,
`ref` = `simulationreference.cpp`. Prüfungen = Namen aus `--simulate-reference`.

## Einheiten, Netze, Messgrößen

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| K01 | p_abs = p_gauge + P_ATM, P_ATM = 101300 Pa; Umrechnung nur über die angegebene Einheit | `units` `convert`, `model` `paramIn` | F01 | Definition; S01 | P_ATM ist Projektvorgabe (Paket: 100000 Pa) | „Einheiten nach Angabe“ |
| K02 | ein Netz = ein Druck; Σ Massen- bzw. Volumenströme im Knoten | `model` `buildNets`, `book` | F02 | Anschlussvertrag; S02 | Luft kg/s, Öl m³/s getrennt; gemischte Netze abgewiesen | „Netze, Stabilität, Ereignisse (R03)“ |
| K03 | Messgrößen p, pa, q (in das Bauteil hinein), x, v, F, pos, ω, φ, τ, P, Fhold, hold | `model` `signalList`, `signalValue` | F41, F49 | eigene Schnittstellendefinition; S02 | Abgriff rückwirkungsfrei; q nur bei `setRecordFlows` | „Messgrößen“ |
| K04 | Kennwertbereiche (Volumen, Massen > 0, Zeiten ≥ 0 …), Abbruch bei NaN | `model` `inRange`, `fail` | F42 | eigene Abnahmeentscheidung; S31 | keine Literaturwerte | „Ungültige Werte“, „Projekt mit ungültigen Daten“ |

## Pneumatik

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| P01 | isotherme Kammer: V·dp/dt + p·dV/dt = R·T·Σṁ, R = 287 J/(kg·K), T = 293,15 K | `phys` `gasNode` | F03, F04 | eigene Ableitung; S03, S04 | konstante Temperatur, keine Wärmebilanz; p ≥ 1000 Pa abs | GasFill, GasCharge, CylinderCycle |
| P02 | Durchfluss nach ISO 6358: ṁ = C·p₁·ρ_N für r = p₂/p₁ ≤ b, sonst zusätzlich ·√(1 − ((r − b)/(1 − b))²); b = 0,3 | `model` `massFlow` | (F26–F28 als Düsengrundlage) | Literaturformel; QS01; ρ_N = 1,185 kg/m³ (ANR) QS02 | b fest 0,3 für alle Bauteile, T₁ = T_N, Exponent 0,5 | GasCharge, OpenModelica-Vergleich pneumatisch |
| P03 | Leitwert aus dem Normalnenndurchfluss: C = qNN/(p₁·f(r)) mit p₁ = 7,013 bar abs, Δp = 1 bar | `model` `conductanceFromQnn` | – | eigene Ableitung aus P02 und QS03 | qNN in Normliter/min | CylinderCycle |
| P04 | Leitwert für einen Durchfluss q bei p₁ → p₂: C = q/(p₁·f(p₂/p₁)) | `model` `conductanceAt` | – | eigene Ableitung aus P02 | wie P02 | Vakuum (R10) |
| P05 | Druckluftbehälter: zusätzliches Volumen, Anschlüsse über großen Leitwert verbunden | `model` (accumulator, Luft) | F04 | Modellnäherung | keine Wärme, kein Druckverlust im Behälter | Projekt Speicher |
| P06 | Schalldämpfer, offene Anschlüsse: Leitwert gegen Atmosphäre | `model` `m_silencer_c` | F10, P02 | Modellnäherung | Geräusch nicht | Projekt Blindstopfen |

## Hydraulik

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| H01 | Ölknoten: V/E·dp/dt = Σq − dV/dt, E = 7·10⁸ Pa | `phys` `oilNode` | F18 | eigene Näherungsableitung; S14, S04 | konstantes E, einphasig | HydraulicCharge, PumpFill |
| H02 | Hohlraum: unter P_ATM − 0,9 bar fehlendes Öl als Volumen bilanziert, beim Zufluss zuerst gefüllt | `phys` `oilNode` (cavity) | F42 | Modellnäherung | kein Dampfdruck, keine Entgasung | „Unterdruck im Öl“ |
| H03 | Blende geglättet: q = k·Δp/(Δp² + p_s²)^¼, k aus rT (Δp = rT·Q²), p_s = 0,3 bar | `phys` `orificeFlow`, `model` `volumeFlow` | F11, F52 | Literatur-Regularisierung mit eigener Skalierung; S10, S11; rT nach QS04 | p_s bestimmt die Kleinsignalkennlinie, keine Viskosität | Orifice |
| H04 | Hydrospeicher polytrop: Öl(p) = V₀ − V₀·(p_pre/p)^(1/n); Volumenbilanz implizit (Bisektion) | `phys` `accumulatorOil`, `oilNodeWithAccumulators` | F21, F22 | Literatur-Modellansatz; S15, S21; implizite Lösung eigene Wahl | n fest, keine Wärmebilanz | AccumulatorCharge |
| H05 | Pumpe: q = α·D·n − GL·Δp, volle Förderung bis p_max − 1 bar, linear bis 0 bei p_max | `model` `Pump`, Schritt | F12, F16 | Literaturgrundlage (S05) + eigene Begrenzungskennlinie | Band 1 bar ist Projektdefinition | PumpFill, Hydromotor (R08) |
| H06 | Hydromotor: q = α·D·ω/2π + C·Δp, τ = α·D·Δp/2π, J·dω/dt = τ − τ_L − b·ω − τ_c·tanh(ω/0,1) | `phys` `Shaft` | F12, F14, F08 | Literaturformel (S05, S29) + Reibungsnäherung (S07) | Glättung 0,1 rad/s ist Projektdefinition, keine Haftphase | Hydromotor (R08) |
| H07 | Leistung: P_hyd = Δp·q, P_Welle = τ·ω, P_Leck = C·Δp² | `model` `signalValue` (P:) | F15 | Leistungsableitung; S05, S29 | – | Hydromotor (R08) |

## Mechanik

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| M01 | Zylinder: F = p_A·A_K − p_B·A_R − F_Last − F_Feder; m·dv/dt = F − F_Reibung + F_Kontakt; A_K = πd²/4, A_R = A_K − πd_s²/4 | `model` Schritt, `phys` `pistonStep` | F06, F07 | Literaturgrundlage mit eigener Vorzeichenableitung; S05, S06 | Feder als konstante Kraft (Projektdefinition) | CylinderCycle, Projekt Hydraulik |
| M02 | Haften/Gleiten: \|v\| < 10⁻⁴ m/s und \|F\| ≤ F_stat → v = 0; sonst Gleitreibung und viskose Dämpfung | `phys` `pistonStep` | F53, F08 | Literatur-Modellfamilie mit eigener Zustandsfestlegung; S09 | Schwelle 10⁻⁴ ist Projektdefinition | CylinderCycle |
| M03 | viskose Dämpfung aus dem FluidSIM-Bezug: d = F_ref/v_ref | `model` (F_ref, v_ref) | F08 | Projektdefinition (Kennwertkonvention FluidSIM) | nur lineare Dämpfung | Batch FluidSIM |
| M04 | elastischer Anschlag: x < 0: F = −k·x − c·min(v,0); x > L: F = −k·(x − L) − c·max(v,0); ohne k harter Anschlag | `phys` `stopForce`, `pistonStep` | F09, F43 | eigenes Kontaktmodell; S08 | Kraftsprung bei Kontaktbeginn | Endanschlag hydraulisch (R02) |
| M05 | Kammervolumen V_A = V_tot + A_K·x, V_B = V_tot + A_R·(L − x) | `model` Schritt | F06 | geometrische Ableitung; S05 | V_tot = 2 cm³ ist Projektdefinition | Hydraulik-CylinderCycle (Volumenbilanz) |
| M06 | Schwenkantrieb als Drehanalogon: τ = (p_A − p_B)·D_θ, D_θ = M_6bar/6 bar [m³/rad], I·dω/dt = τ − M_ext − f·ω (Haften wie M02); V_A = V_tot + D_θ·θ | `model` (rotary_actuator), `phys` `pistonStep` | F14, F36 | eigene Ableitung (S29): Arbeit p·dV = τ·dθ ergibt D_θ = τ/Δp | ideale Wandlung, keine Leckage, keine Endlagendämpfung | Vakuum und Schwenk-/Greiferantriebe (R10) |
| M07 | Greifer: Ersatz durch M01 mit Rückstellfeder, Greifkraft = Kolbenkraft | `model` (actuator.gripper) | F07 | Modellnäherung | keine Backenkinematik (F36 nicht angewandt) | R10 |

## Ventile

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| V01 | Schieber: Öffnung 1 − e^(−3t/T_schalt) der neuen Stellung, die alte schließt gleich; Durchfluss linear in der Öffnung | `model` Ventilschritt | F13, F17 | PT1 aus S22, Pfadmatrix eigene Bauteilspezifikation | keine Überdeckung, keine Leckage | CylinderCycle |
| V02 | Rückschlag-, Begrenzungs-, Druckregelventil: Öffnung linear zwischen p₀ und p₀ + Δp_end | `model` FlowValve-Schritt | F16 | eigenes Referenzmodell; S04 | keine Kegelmasse, Hysterese, Pilotdynamik | Projekt Druckbegrenzung, Projekt Speicher |
| V03 | Wechsel-/Zweidruckventil: Eingang mit höherem bzw. niedrigerem Druck auf Ausgang 2 | `model` FlowValve-Schritt | – | Projektdefinition (logische Funktion) | keine Zwischenstellung | Projekt Speicher |
| V04 | Steueranschluss schaltet ab p_gauge > 1,5 bar | `model` `P_PILOT` | – | Projektdefinition | keine Steuerfläche, keine Kraftbilanz | CylinderCycle |

## Vakuum (R10)

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| U01 | Ejektor-Anteil s = min(1, p₁,gauge/dp_max); Luftverbrauch über P04 mit qn_ac bei dp_max | `model` Ejector | F35 | Modellnäherung | über dp_max kein Abfall | R10 (3,5 bar) |
| U02 | Saugstrom ṁ = ṁ_max·(p₂ − P_ATM − pu_min·s)/\|pu_min\|, ṁ_max = qns_max·ρ_N | `model` Ejector | F35 | Modellnäherung (Paket: Ejektorfluss ist ein eigenes, zu identifizierendes Kennfeld) | linear, nicht an Messkennlinien identifiziert | R10 (−0,800/−0,555 bar, Leckgleichgewicht) |
| U03 | Haltekraft F = (P_ATM − p)·π·d²/4, hält bei F ≥ s·m·g | `model` SuctionCup | F35 | Bilanz-/Kraftableitung; S03, S05 | kein Haft-/Sicherheitsmodell, keine Querkräfte | R10 (25,13 N, 2 kg, s = 1,5) |
| U04 | Saugerleckage über P04: qLeak bei −0,6 bar; ohne Werkstück 50 l/min bei 0,1 bar (Ø 20 mm, mit d skaliert) | `model` SuctionCup | F35, P02 | Projektdefinition | Werte gewählt, nicht gemessen | R10 (Leckage, ohne Werkstück) |
| U05 | Vakuumschalter: ein bei p ≤ p₀ (< 0), aus bei p > p₀ + p_hyst | `model` PressureSwitch | – | Projektdefinition | ohne Schaltzeit | R10 (Vakuumschalter) |

## Numerik, Elektrik, Steuerung

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| N01 | expliziter Euler dt = 10 µs; Mechanik halbimplizit (v zuerst) | `model` `step`, `phys` `pistonStep` | F44 (Vergleich), F51 | eigene Wahl; S32 | steife Netze werden diagnostiziert, nicht gelöst | GasCharge (Ordnung 1), Konvergenzprüfungen |
| N02 | Schwingungswächter: 200 Schritte mit wechselndem Vorzeichen und \|Δp_k\| ≥ 0,9·\|Δp_k−1\| → Abbruch | `phys` `OscillationWatch` | F42 | Projektdefinition | Diagnose, keine Lösung | R03 |
| N03 | Relais: Anzug 10 ms, Abfall 8 ms; SPS und GRAFCET alle 10 ms | `model` `T_PULL_IN`, `T_DROP_OUT` | F40 | Projektdefinition; S31, S35 | – | PeriodicCounter, Projekt GRAFCET |
| N04 | Fehlermaße der Vergleiche: Schlauch um die stückweise lineare Gegenkurve, Grenzen je Prüfung | `ref` | F51 | eigene Abnahmeentscheidung | Grenzen stehen in jeder Prüfung | alle Vergleichsprüfungen |
| N05 | Handrechnungen der Prüfungen (Ausfahrzeit = Hub·A/Q, Gleichgewicht Saugstrom = Leckage per Bisektion, Δp = 2π(τ + bω)/D …) | `ref` | F50 | eigene Ableitung aus den Zeilen oben | – | jeweilige Prüfung |

## Vorgaben (Projektdefinitionen, keine Messwerte)

V_LINE 5 cm³ je Luftnetz, V_HYD 0,1 l je Ölnetz, V_tot 2 cm³ je Zylinderkammer, E_OIL 7·10⁸ Pa, b = 0,3,
p_s = 0,3 bar, Pilotschwelle 1,5 bar, Haftkraft 5 % von A_K·6 bar, Gleitreibung 70 % der Haftkraft, Federkraft
15 % von A_K·6 bar, Haftmoment des Schwenkantriebs 2 % von M_6bar, Pumpenband 1 bar, Saugervolumen 5 mm tief,
offener Sauger 50 l/min bei 0,1 bar, Leckagebezug −0,6 bar, Relaiszeiten 10/8 ms. Kennwerte aus FluidSIM-Bauteilen
(qNN, rT, M_6bar, I, f, pu_min, qn_ac, qns_max …) sind Angaben der Beispielbauteile, nicht von uns gemessen.

## Zusätzliche Quellen

- **QS01** – ISO 6358-1:2013, Pneumatic fluid power – Determination of flow-rate characteristics of components using
  compressible fluids – Part 1: General rules and test methods for steady-state flow. Kennlinie mit Schallleitwert C
  und kritischem Druckverhältnis b (Ellipse, Exponent 0,5). Fundstelle nach Normtitel; die Abschnittsnummer ist hier
  nicht am Normtext geprüft.
- **QS02** – ISO 8778:2003, Pneumatic fluid power – Standard reference atmosphere (ANR: 20 °C, 100 kPa, 65 % relative
  Feuchte; Dichte ≈ 1,185 kg/m³).
- **QS03** – Normalnenndurchfluss qNN (Festo/FluidSIM-Kennwert, VDI 3290): Durchfluss bei 6 bar Eingangsüberdruck und
  1 bar Druckabfall in Normliter/min. Übernommene Kennwertkonvention, kein eigener Messwert.
- **QS04** – FluidSIM-Kennwert rT (hydraulischer Widerstand, Δp = rT·Q², Einheit bar·min²/l² bzw. MPa·min²/l²):
  Kennwertkonvention der Beispielbauteile, beim Import übernommen.

Eine neue Formel bekommt hier ihre Zeile mit Codestelle, Herkunft, Beleg, Grenze und Prüfung, bevor sie freigegeben wird.
