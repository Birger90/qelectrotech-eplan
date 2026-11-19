# Formelregister des Rechenkerns

Stand Fork 1.33.0 (18.11.2026). Hier steht jede Beziehung, die `sources/simulation/` rechnet, mit Codestelle,
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
| K16 | Fehlermodelle (Elementinformation sim_fault „<Fehler> [Wert] [ab t] [bis t]“, wirksam im Zeitfenster bis zur Reparatur): Kolben klemmt – Wegeventil behält seine Stellung; Spule unterbrochen – Ventil- bzw. Relaisspule ohne Wirkung; interne Leckage – zusätzlicher Strom zwischen den ersten beiden Fluidanschlüssen, Öl q = G·Δp (Vorgabe 0,1 l/(min·bar)), Luft Leitwert eines Normalstroms (Vorgabe 10 l/min); verstopft – Strom des Ventils × Restanteil (Vorgabe 1 %); Rückschlagventil immer gesperrt/geöffnet; Bauteil schaltet nicht/immer (Taster, Grenztaster, Druckschalter) | `model` `buildFaults`, `fault`, `repair` | F42 | Fehlerarten und Reparatur nach QS05 („Fehlermodelle und Fehlerkonfigurationen“, „Fehlerkonfiguration erstellen“, „Reparaturen durchführen“, Bauteilseiten: Kolben klemmt, Interne Leckage, Bauteil verstopft, Relaisspule unterbrochen, Rückschlagventil immer gesperrt/geöffnet, Bauteil schaltet nicht/immer); Zeitfenster und Vorgabewerte Projektfestlegung | ein Fehler je Art und Bauteil, Konfiguration am Bauteil statt als benannte Fehlerkonfiguration; Kurzschluss, Glühfaden und elektrische Werte erst mit R17 | Fehlerszenarien (R15) |

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
| H05 | Pumpe: q = α·D·n − GL·Δp, volle Förderung bis p_max − 1 bar, linear bis 0 bei p_max; Anschlüsse P (Druck) und T (Saugen), vereinfachtes FluidSIM-Aggregat nur P2 (saugt intern), FluidSIM `Pump01` A (Druck, Pfeil) und B (Saugen) | `model` `Pump`, Schritt | F12, F16 | Literaturgrundlage (S05 S. 13–15) + eigene Begrenzungskennlinie; Anschlüsse nach Symbolgeometrie (Fork 1.36.0: vorher lieferten P2- und A/B-Pumpen nichts) | Band 1 bar ist Projektdefinition | PumpFill, Hydromotor (R08) |
| H06 | Hydromotor: q = α·D·ω/2π + C·Δp, τ = α·D·Δp/2π, J·dω/dt = τ − τ_L − b·ω − τ_c·tanh(ω/0,1) | `phys` `Shaft` | F12, F14, F08 | Literaturformel (S05 S. 13–15, S29) + Reibungsnäherung (S07) | Glättung 0,1 rad/s ist Projektdefinition, keine Haftphase | Hydromotor (R08) |
| H07 | Leistung: P_hyd = Δp·q, P_Welle = τ·ω, P_Leck = C·Δp² | `model` `signalValue` (P:) | F15 | Leistungsableitung; S05, S29 | – | Hydromotor (R08) |
| H08 | LS-Aggregat: Verstellpumpe q₁ = α·D_VP·n − G_VP·p₁ mit α* = clamp(1 − (p₁ − p_LS − p₀)/Δp_end, 0, 1) und dα/dt = (α* − α)/T_PT1; Konstantpumpe q₂ = D_FP·n − G_FP·p₂; je Pumpe Druckbegrenzung q = s·√(Δp/r_T), s = clamp((p − p₀,DBV)/Δp_end,DBV, 0, 1); LS-Leitung entlastet über r_T,LS zum Tank | `model` `LsUnit` | F02, F05 | Aufbau und Kennwerte nach QS05 („Hydraulikaggregat mit LS-Verstell- und Konstantpumpenkombination“: Drehzahl, Verdrängungsvolumen, Öffnungsdruck, Federvorspannung, Leckage, Druckregelbereich, Zeitkonstante); Regler als proportionale Rückstellung über den Regelbereich eigene Ableitung; einheitenlose FluidSIM-Bänder (dpEnd, dpEnd_prv*) als bar gelesen (Projektfestlegung: dpEnd_prv der übrigen Druckbegrenzungsventile ist 3,5 MPa = 35 bar) | kein Schwenkwinkel- und Stellkolbenmodell, Drehzahl fest, Rückläufe und Leckage direkt zum Tank | Mobilhydraulik (R13) |
| H09 | Durchflussmesser als Hydromotor (FluidSIM `sensor.flowmeter_2` mit Schluckvolumen): quasistationär τ = D·Δp/2π = f·ω, ω = 2π·q/D, also q = D²/(4π²·f)·Δp; Leckage G_Leak·Δp parallel; gemessen wird der Motorstrom | `model` FlowValve (`gmeter`) | H05, M06 | Aufbau nach QS05 („Durchflussmesser: besteht aus einem Hydromotor, der mit einem Drehzahlmessgerät verbunden ist“; Schluckvolumen 8,2 cm³, Reibung 0,0108 N·m·s/rad, Trägheit 0,0001 kg·m², Leckage 0,0126 l/(min·bar)); Gleichgewicht eigene Ableitung aus H05 | Trägheit nicht gerechnet (Zeitkonstante I·4π²·f/… vernachlässigt), keine Impulszählung; ohne Schluckvolumen bleibt die Blende rT (R06) | Mobilhydraulik (R13) |
| H10 | Lenkventil (Orbitrol): Drehventil-Anteil a = M_user/M_userMax (−1 … 1); Neutralumlauf P → E q = √(Δp·(1 − |a|)²/r_T); a > 0 (links) P → L und R → T, a < 0 (rechts) P → R und L → T, je q = √(Δp·a²/r_T); Lenkradwinkel φ̇ = sign(a)·2π·q_L/D (Dosierpumpe) | `model` `Steering` | V01, H03 | Aufbau nach QS05 („Lenkventil (Orbitrol)“: Proportionaldrehventil, Pumpensteuerventil, Dosierpumpe; Drehventil öffnet proportional zum Drehmoment und zur Drehrichtung; Kennwerte Drehmoment, Verdrängungsvolumen, Widerstand) und Symbolgeometrie (Schichten Neutral/Left/Right: neutral P → E, links P → L und R → T, rechts überkreuzt); Vorzeichen M_user > 0 = links Projektfestlegung | Moment fest für den Lauf, keine Rückwirkung der Lenkkraft auf das Moment, revolutionMax, dMax und xStep nicht ausgewertet, ohne Leckage | Lenkung und Sperrventile (R13) |
| H11 | Vorsteuergerät 2×2: je Gruppe g Sollwert p_s = |a_g|·p_max am Anschluss X g.1 (a_g > 0) bzw. X g.2 (a_g < 0) als Druckminderventil, Zulauf P → X mit Anteil clamp((p_s − p_X,T)/Δp_end, 0, 1), Entlastung X → T mit clamp((p_X,T − p_s)/Δp_end, 0, 1), Blende r_T/Anteil²; der andere Anschluss und beide in Ruhe offen zu T | `model` `Joystick` | V01, H03 | Aufbau nach QS05 („Vorsteuergerät, 2x2-kanalig“: je Kanal zwei Druckventile, die Wippe betätigt eines, das den Druck am Steueranschluss proportional zur Auslenkung regelt; Kennwerte maximaler Druck, Auslenkung links/rechts −100 … 100 %); Zuordnung + → X g.1 Projektfestlegung; r_T 0,1 bar·min²/l² und Δp_end 0,5 bar Projektwerte (FluidSIM nennt keine) | Auslenkung fest für den Lauf, Rückschlagventil im Tankanschluss nicht gerechnet, ohne Dynamik | Lenkung und Sperrventile (R13) |
| H12 | Membranspeicher mit Sicherheitsblock: Speicher H04 mit Vorfülldruck p0_accu an P, solange der Kugelhahn offen ist (x = 2; 1 abgesperrt, 0 entleert: ohne Wirkung auf P); Druckbegrenzung Speicher → T mit Anteil clamp((p − p_T − p0_prv)/Δp_end, 0, 1), q = √(Δp·Anteil²/r_T) | `model` `Accumulator` (`block`) | H04, H03 | Aufbau und Kennwerte nach QS05 („Membranspeicher mit Sicherheitsblock“: speichert den Druck, mit Druckbegrenzungsventil gegen Überdruck gesichert; Vorspanndruck, Volumen, Öffnungsdruck) und Symbolgeometrie (3/3-Kugelhahn mit Schieberschicht ×24 px: offen, zu, Speicher → T); Δp_end 3,8 MPa und r_T 0,17 vom FluidSIM-Druckbegrenzungsventil 01 übernommen (Projektfestlegung) | Kugelhahn fest für den Lauf, Entleeren nicht als Strom zum Tank gerechnet | Lenkung und Sperrventile (R13) |

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
| M08 | Gewichtskraft F_W = m·g·sin α gegen das Ausfahren (m = m_cyl + m_ext, α Einbauwinkel, g = 9,81 m/s²) | `model` Cylinder `weight`, `opposing` | F06 | Literaturformel nach QS05 („Die dem Ausfahren entgegenwirkende Gewichtskraft“: F_W = m·g·sin α); S06 (Masse) | Drehen des Symbols ändert α nicht (wie FluidSIM) | Standardventile und Zylinderkräfte (R11) |
| M09 | Reibung nach FluidSIM: hydraulische Kolbenreibung F_B (Losbrechkraft), F_M (Coulomb), F_V (viskos, N·s/m) statt der Vorgaben aus M02/M03; äußere Masse zusätzlich Haftreibung µ_B·m_ext·g·\|cos α\| und Gleitreibung µ_C·m_ext·g·\|cos α\|; Haftgrenze ≥ Gleitreibung | `model` Cylinder | F07, F08 | nach QS05 („Geschwindigkeitsabhängige Reibung“: Haftreibung, Newton'sche und Coulomb'sche Gleitreibung des Kolbens und der externen Masse, F_B = µ_B·m·g·cos α, F_C = µ_C·m·g·cos α; „Konfigurierbarer Zylinder“: Losbrechkraft, Coulomb'sche Reibkraft, viskose Reibung); Reibungsfamilie S07 | Stribeck-Übergang (tv, tf, v_M) nicht nachgebildet: Sprung von Haft- auf Gleitreibung; pneumatische Zylinder weiter M02/M03 (F_stat, F_ref/v_ref) | R11 |
| M10 | Federkennlinie F_S(x) = A_K·(p_min + (p_max − p_min)·x/x_max) (Federdruck bei x = 0 und bei x_max, `spring_right` gegen das Ausfahren, `spring_left` in Ausfahrrichtung); Kraftprofil F_E(x) stückweise linear aus F_var_ex (Ausfahren, v ≥ 0) bzw. F_var_re (Einfahren), Punkte {x [mm], F [N], …}, außerhalb der Endwert, gegen das Ausfahren | `model` Cylinder `opposing`, `profileAt` | F06, F41 | nach QS05 („Konfigurierbarer Zylinder“: Federdruck bei x = 0 und x = x_max; „Kraftprofil“: Stützpunkte über die Kolbenstellung, getrennt für Aus- und Einfahren; „Die vom Anwender vorgegebene Kraft“: wirkt dem Ausfahren entgegen) | Federdruck auf die Kolbenfläche bezogen und Einheit N der Profilkraft sind Annahmen (die Hilfe nennt sie nicht); ohne spring_left/right bleibt die Vorgabe 15 % von A_K·6 bar | R11 |
| M11 | Druckluftmotor als Welle (M06/H-Motor-Familie): τ = D_θ·(p_A − p_B) − b·ω − τ_Last, J·dω/dt = τ; mit Nennwerten (p_N, n_N, M_N, k = n₀/n_N): Stillstandsmoment M_S = M_N·k/(k − 1), D_θ = M_S/p_N, b = M_S/(k·2π·n_N), also n = n_N bei p_N und M = M_N, n₀ = k·n_N im Leerlauf; Luftverbrauch ṁ = ρ(p_Zulauf)·D_θ·\|ω\| vom höheren der Anschlüsse A/B zum Mittelauspuff EX; bremsendes Fremdmoment als trockene Reibung (torqueType 1), antreibendes als Lastmoment (2) | `model` Motor (`air`), `phys` `Shaft` | F09, F10 | eigene Ableitung aus der linearen Kennlinie (Herleitung unten); Nennwerte und Parameter nach QS05 („Druckluftmotor mit Mittelauspuff“: Nenndruck, Nenndrehzahl, Nennmoment, n₀:n_N, Trägheitsmoment, Fremdmoment bremsend/antreibend); Wellengleichung S29 | Kennlinie linear und Moment proportional zum Druck (reale Lamellenmotoren weichen ab); keine Leckage, kein Anlaufmoment der Lamellen; ohne Nennwerte (Pneu.RotaryMotor) gelten displacement, f, I | Druckluftmotoren (R12) |
| M12 | Mehrstellungszylinder als zwei Kolben gleicher Fläche A_k hintereinander: hinterer Kolben (C1, Hub x₁ ≤ x_max2) m·ẍ₁ = p_C1·A_k − p_C2·A_St − F_R,1 + N, vorderer Kolben (C2/C3, x ≤ x_max) m·ẍ = p_C2·A_k − p_C3·(A_k − A_St) − F_ext − F_R,2 − N, Kontaktkraft N ≥ 0 nur bei x = x₁; in Kontakt ein Körper (2m, Reibung und Dämpfung addiert, Grenze x_max2), getrennt sobald einer allein vom anderen wegläuft; Stoß plastisch (gleiche Massen: gemeinsame Geschwindigkeit (v + v₁)/2); Volumen V_C1 = A_k·x₁, V_C2 = A_k·x − A_St·x₁, V_C3 = (A_k − A_St)·(x_max − x) | `model` Cylinder (`multi`), `multiStep` | M01, F01 | Aufbau nach QS05 („Mehrstellungszylinder, doppeltwirkend“: zwei Zylinder gleichen Kolbendurchmessers aneinandergereiht, kürzerer Hub = halber längerer, Parameter Kolben-/Stangendurchmesser, Gesamthub, Kolbenstellung, Zwischenstellung, externe Kraft) und Symbolgeometrie (Trennwand mit Stangendurchführung, Anschlüsse C1 hinter dem hinteren, C2 hinter dem vorderen Kolben, C3 Stangenseite); Kräftegleichgewicht eigene Ableitung | Stangenraum des hinteren Kolbens entlüftet angenommen (Symbol ohne Anschluss); beide Kolben mit Masse und Reibung des Zylinders (Projektfestlegung, FluidSIM nennt keine eigenen Werte); keine Endlagendämpfung | Mehrstellungszylinder (R12) |
| M13 | Belastungseinheit: zwei gegenüberliegende Zylinder auf einer Stange, m·ẍ = p_A1.1·A_k − p_A1.2·A_r + p_A2.1·A_r − p_A2.2·A_k − F_Reib (R11: F_B, F_M, F_V) − m·g·sin α; Volumen A_k·x, A_r·(x_max − x), A_r·x, A_k·(x_max − x) | `model` Cylinder (`loading`) | M01, M08 | Aufbau nach QS05 („Belastungseinheit/Zylinderlastsimulator“: zwei Zylinder stehen sich gegenüber, Kennwerte Kolben-/Stangendurchmesser, Hub, Einbauwinkel, bewegte Masse, Losbrech-, Coulomb- und viskose Reibung) und Symbolgeometrie (Anschlüsse A1.1/A2.2 Kolbenseiten, A1.2/A2.1 Stangenseiten) | Stribeck-Geschwindigkeit v_M nicht gerechnet; Belastungseinheit 1 mit eingebautem Druckbegrenzungsventil noch nicht | Mobilhydraulik (R13) |
| M14 | Belastungseinheit 1: M13 und eine Querentlastung der Stangenräume A1.2 ↔ A2.1: vom höheren zum niedrigeren Druck über Druckbegrenzungs- und Rückschlagventil in Reihe, Anteil s = clamp((|Δp| − p0_prv)/Δp_end,prv, 0, 1), q = √(|Δp|/(r_T,prv/s² + r_T,cv)) | `model` Cylinder (`cross`) | M13, H03 | Aufbau nach QS05 („Belastungseinheit/Zylinderlastsimulator“, Öffnungsdruck 12 MPa) und Symbolgeometrie (je ein Druckbegrenzungsventil von A1.2 und A2.1 auf einen gemeinsamen Knoten, Rückschlagventile von dort in beide Stangenräume); Kennwerte rT_prv, dpEnd_prv, rT_cv aus FluidSIM | Zwischenknoten nicht eigens gerechnet (Öffnung aus der Gesamtdifferenz, Rückschlagventil als Widerstand in Reihe) | Lenkung und Sperrventile (R13) |

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
| V09 | Proportional-Druckbegrenzungsventil: Öffnungsdruck p₀ = p_start + (p_end − p_start)·clamp(u/10 V, 0, 1); öffnet A→B mit Anteil clamp((p_A − p_B − p₀)/Δp_end, 0, 1), Blende rT/Anteil² (H03) | `model` FlowValve (prop. relief) | F16, F17 | eigenes Referenzmodell; Kennwerte nach QS05 („Proportional-Druckbegrenzungsventil“: Ansteuerung 0 … 10 V, min./max. Öffnungsdruck bei 0/10 V, Druckregelbereich, Widerstand der voll offenen Kante) | direktgesteuert, ohne Magnet- und Kolbendynamik; Vorsteuerung fehlt | Analoge Signale und Regler (R06) |
| V10 | Druckwaage Schließer/Öffner (zwei Anschlüsse im Hauptstrom, Steuerdifferenz d): Schließer Anteil clamp(1 − (d − p₀)/Δp_end, 0, 1), Öffner clamp((d − p₀)/Δp_end, 0, 1); Hauptstrom als Blende rT/Anteil² (Öl, H03) bzw. ISO-6358-Leitwert·Anteil (Luft, P02). d: Schließer/Öffner 01/02 p_X − p_XS (Strom B ↔ A), vorgeschaltet p_A − p_X (Schließer, P → A), nachgeschaltet p_P − p_X (Öffner, P → A), Open Center LS p_P − p_X (Öffner, P → T) | `model` FlowValve (pressure compensator) | F16 | eigenes Referenzmodell nach QS05 („Druckwaage (Schließer)/(Öffner)“: schließt bzw. öffnet, wenn die Druckdifferenz den Solldruck überschreitet; „vorgeschaltet“: hält p_A − p_X konstant; „Open Center Load Sensing“: öffnet P → T ab p₀ über p_X); Lage des Regelbereichs (Schließer erst ab p₀ schließend) Projektdefinition | keine Kolbenmasse und Dämpfung, keine Strömungskraft; nachgeschaltete Druckwaage: Wirkrichtung aus der LS-Funktion abgeleitet (Hilfe nennt nur die Verwendung) | Stetig-, Senkbrems- und Zuschaltventile (R07/R09) |
| V11 | Druckwaage als Regler (Prüffälle): mit einer Drossel zwischen A und X bzw. mit X an A stellt sich d im Band [p₀, p₀ + Δp_end] ein, solange die Pumpe mehr fördert, als die Drossel bei p₀ abführt | `ref` | F16, F50 | eigene Ableitung aus V10 (Herleitung unten) | Lage im Band hängt vom Pumpendruck ab | R07/R09 |
| V12 | Schock- und Nachsaugblock: A1 = A2 und B1 = B2 je ein Netz; A und B öffnen nach T mit Anteil clamp((p − p_T − p₀)/Δp_end, 0, 1), Blende rT_prv/Anteil²; fällt p unter p_T, speist das Rückschlagventil T → A bzw. B (rT_cv) | `model` FlowValve (shock), `buildNets` | F16, F11 | eigenes Referenzmodell nach QS05 („Schock- und Nachsaugblock“: begrenzt den Druck von A oder B gegenüber T; Öffnungsdruck, Druckregelbereich und Widerstand je Gruppe); Nachsaugfunktion aus dem Namen und den Rückschlagwiderständen der Bibliothek (rT_cv) | keine Dynamik der Druckbegrenzer (Schockspitzen nur über die Netzsteifigkeit) | R07/R09 |
| V13 | Pneumatisches Folgeventil: 1 → 2 mit Anteil clamp((p₁₂,gauge − p₀)/0,1 bar, 0, 1), ISO-6358-Leitwert aus qNN (P02/P03); geschlossen ist 2 abgesperrt | `model` FlowValve (pneumatic sequence) | F16 | nach QS05 („Druckschaltventil (Folgeventil)“: übersteigt der Druck an 12 den Einstellwert, öffnet der Durchfluss von 1 nach 2); Übergangsbreite 0,1 bar Projektdefinition | keine Rückstellhysterese, keine Entlüftung von 2 | R07/R09 |
| V14 | Gleichzeitige Betätigung beider Seiten: ohne Vorrang bleibt der Schieber, wo er ist; mit `dominant` left/right geht er in die Stellung dieser Seite | `model` `Valve::wanted` | F40 | Projektdefinition; Vorrang ist eine Eigenschaft des Ventilsymbols (QET-Fluiddaten „Which side wins when both signals are on“) | kein Kräftevergleich der Betätigungen | R11 |
| V15 | 6/3-Handhebelventil (Open Center): Umlauf P1→T1 q = √(Δp·(1 − |x|)²/r_T,P1T1); x > 0: P2→B, A→T2, x < 0: P2→A, B→T2 mit q = √(Δp·|x|²/r_T); Zulauf über das Rückschlagventil in Reihe (r_T,CV + r_T,P2AB/|x|², nur vorwärts); Leckage A, B → T2; Schieber dx/dt = (x_Hebel − x)/T_2 | `model` `Valve63` | V01, H03 | Anschlüsse, Kennwerte und Schaltwege nach QS05 („6/3-Wege-Proportional Handhebelventil“: Auslenkung ±100 %, Widerstände P1→T1, P2→A,B, T2→A,B, Leckage, Rückschlagventil) und Symbolgeometrie (Schieberstellungen); Fläche linear in |x| eigene Annahme | Auslenkung fest für den Lauf (kein Handhebel im Lauf), keine Über-/Unterdeckung, keine Strömungskräfte | Mobilhydraulik (R13) |
| V16 | Entsperrbares Doppelrückschlagventil (Sperrblock): A1 → A2 und B1 → B2 frei ab der Federvorspannung p₀, q = √((Δp − p₀)/r_T); Entsperren der Gegenrichtung, wenn ratio·(p_Steuer − p_ein) ≥ (p_aus − p_ein) + p₀ (Steuerkolben mit Flächenverhältnis ratio gegen Kugel und Feder), Anteil clamp(Überschuss/Δp_end, 0, 1), q = √(Δp·Anteil²/r_T) in beide Richtungen; A wird von B1, B von A1 entsperrt | `model` FlowValve | V01, H03 | Kennwerte nach QS05 (Widerstand, Flächenverhältnis 3,3, Federvorspannung 0,2 MPa bei Ausführung 02) und Symbolgeometrie (Kugeln zwischen A1/A2 und B1/B2, Steuerleitung der linken Kugel aus B1, der rechten aus A1); Kräftegleichgewicht eigene Ableitung; Δp_end 0,5 bar Projektwert | ohne Kolbendynamik, Leckage und Vorentlastung | Lenkung und Sperrventile (R13) |
| V17 | LS-Steuerblock, je Scheibe g: Schieberweg x = clamp((p_C − pS_min)/(pS_max − pS_min)) − clamp((p_D − pS_min)/(pS_max − pS_min)) bzw. Handhebel xStep; x > 0 P → A, B → T, x < 0 P → B, A → T; vorgeschaltete Druckwaage quasistatisch in Reihe zur Messkante: Δp = q²·(r_T,pc/s² + r_T,PAB/x²) mit s = clamp(1 − (q²·r_T,PAB/x² − p0_pc)/dpEnd_pc), nach q gelöst (Bisektion), damit q ≈ |x|·√(p0_pc/r_T,PAB) lastunabhängig, solange das Gefälle reicht; Rücklauf q = √(Δp·x²/r_T,TAB); geschlossene Mitte; Wechselventile: höchster Lastdruck → LS über r_T,SV, nur vorwärts | `model` `LsBlock` | V10, H03 | Aufbau nach QS05 („Load-Sensing Steuerblock“: zwei Scheiben mit je einem Proportionalwegeventil, dem eine Druckwaage vorgeschaltet ist, die Scheibe verhält sich wie ein Proportionalstromregler) und Symbolgeometrie (Vorsteueranschlüsse C (X g.1), D (X g.2), Wechselventile SV1/SV2 zum LS-Anschluss); Kennwerte rT_PAB, rT_TAB, pS_min, pS_max, p0_pc, dpEnd_pc, rT_pc, rT_SV aus FluidSIM; C → P→A Projektfestlegung | ohne Schieberdynamik und Leckage, F_manual nicht ausgewertet, Druckwaage ohne eigenen Zwischenknoten | LS-Steuerblock und Prioritätsventil (R13) |
| V18 | Prioritätsventil LS: Kolbenanteil s = clamp(1 − (p_CF − p_LS − p0)/dpEnd, 0, 1); P → CF q = √(Δp·s²/r_T,CF), P → EF q = √(Δp·(1 − s)²/r_T,EF) | `model` `PriorityValve` | V10, H03 | Aufbau nach QS05 („Prioritätsventil LS, dynamisch“: CF wird bevorzugt versorgt; Federvorspannung, Druckregelbereich, Widerstände P-CF, P-EF) | quasistatisch: Dämpfungsdrosseln rT_L, rT_R und die dynamische Steuerölströmung nicht gerechnet | LS-Steuerblock und Prioritätsventil (R13) |

## Analoge Signale und Regler (R06)

Elektrische Teile mit reellen Werten (`sim_role` „analog“, Klasse in `sim_block`, Kennwerte in `sim_params`, Kopplung
in `sim_link`; der FluidSIM-Import schreibt sie). Codestellen: `sig` = `simulationsignals.h`, `model` `analog`, `linkAnalog`.

| ID | Beziehung | Codestelle | Paket | Herkunft, Belege | Grenze | Prüfung |
|---|---|---|---|---|---|---|
| A00 | Signalvertrag: Potential eines Netzes gegen 0 V = Analogausgang, sonst Potential der verbundenen Quelle (`sim_volt`, ohne Angabe 24/0 V), sonst 24 V an einem Plus-Netz, sonst 0. Bausteine sind stetig und laufen mit dem Modellschritt (keine eigene Abtastung); ausgewertet nach den Bausteinen, die ihre Eingänge treiben; eine reine algebraische Schleife liest dort den Wert des letzten Schritts. SPS und GRAFCET sehen die Analogwerte nicht (10-ms-Zyklus nur digital). Zwei Ausgänge auf einem Netz sind ein Modellfehler | `model` `volt`, `linkAnalog`, `analog` | F40, F41 | Projektdefinition; S02 (Potentialgleichheit an Verbindungen), S35 (sample/pre als Abgrenzung zur Abtastung) | keine Ströme, keine Lastrückwirkung: Ausgänge ideale Spannungsquellen, Eingänge hochohmig; kein Kirchhoff-Netz | Analoge Signale und Regler (R06) |
| A01 | Sensorkennlinie y = U₀ + clamp((v − v₀)/(v₁ − v₀), 0, 1)·(U₁ − U₀) für Druck (Überdruck), Durchfluss (Richtung ignoriert: \|q\|, berücksichtigt: q) und Kolbenweg (v = x/Hub, Bereich 0 … 1) | `sig` `scale`, `model` `analog` | F41 | affine Kalibrierung F41; Bereichsgrenze und Kennwerte nach QS05 („Analog-Drucksensor“: nur Drücke im Bereich; „Analog-Durchflussmesser“: Flussrichtung ignorieren/berücksichtigen; „Wegmesssystem“: Spannung eingefahren/ausgefahren, mind. 13 V) | keine Sensordynamik, kein Rauschen; der Drucksensor prüft seine Versorgung nicht | R06 (Kalibrierung, Umkehr, Sättigung) |
| A02 | Funktionsgenerator: y = clamp(Versatz + A·f(t), U_min, U_max), f = Rechteck (+1 in der ersten Halbperiode), sin(2π f t), Dreieck (Start 0, Scheitel bei T/4), konstant 0; Profil stückweise linear, mit „Schleife“ periodisch | `sig` `Waveform` | F41 | eigene Ableitung der Kodierung waveForm 1–5 (Herleitung unten); Signalformen und Bereiche nach QS05 („Funktionsgenerator“) | Profil ohne Amplitude und Versatz (Projektdefinition); Phase 0 bei t = 0 | R06 (Sinus, Rechteck, Dreieck, Konstant, Profil) |
| A03 | Sollwertkarte: W1 … W8, Rampe R [s/V] (0 = Sprung): R1 steigend ab 0 V, R2 fallend bis 0 V, R3 fallend unter 0 V, R4 steigend bis 0 V, Wechsel der Rampe bei 0 V; Modus 1 nach Umschaltzeit weiter, 2 nach Erreichen weiter, 3 Auswahl W = I1 + 2·I2 + 4·I3 (Eingang ≥ 15 V) | `sig` `SetpointCard` | F41 | Verhalten nach QS05 („Sollwertkarte“: Rampen, drei Modi, Bittabelle, 15 V); Modusnummern 1–3 in der Reihenfolge der Hilfe (Annahme, gestützt auf TP701_03: Modus 1 mit Umschaltzeit) | Versorgung ≥ 20 V ist Projektdefinition; ohne Versorgung Ausgang 0 und Neustart | R06 (Rampen, Umschaltung) |
| A04 | PID-Regler (Parallelform): e = W − X; τ_d·ż = X − z; u_raw = K_P·e + I − K_D·(X − z)/τ_d + Offset; u = clamp(u_raw, u_min, 10 V), u_min = −10 V oder 0; İ = K_I·e + (u − u_raw)/T_track; ohne K_I kein Integrator; ohne Versorgung ≥ vMin (21,6 V) Ausgang 0, I = 0, z neu (Reset) | `sig` `Pid`, `model` `linkAnalog` | F37, F17 | eigene Spezifikation der Paket-Variante F37 mit b = 1 und Ableitung des Messwerts (wd = 0 wie die LimPID-Vorgabe, S23); Parallelform aus den Einheiten K_I [1/s], K_D [ms] (Herleitung unten); Bereiche, Offset, Versorgung nach QS05 („PID-Regler“) | τ_d = T_d/10 (T_d = K_D/K_P, in Anlehnung an LimPID Nd = 10, S23) und T_track = 0,9·T_i (T_i = K_P/K_I, wie LimPID Ni = 0,9, S23) sind Projektdefinitionen; die innere Struktur des FluidSIM-Reglers ist nicht dokumentiert (Ableitung könnte auf e wirken); kein Hand-/Automatikbetrieb, keine stoßfreie Umschaltung | R06 (Sättigung, Anti-Windup, Reset, Abtastverfeinerung, beide Regelkreise) |
| A05 | Zustandsregler: v = ẋ, a = v̇ je über ein Filter erster Ordnung τ_f; u = clamp(K·(K_x·(W − X) − K_v·v − K_a·a) + Offset) | `sig` `StateController` | F37, F17 | eigene Spezifikation; Größen und Einheiten nach QS05 („Zustandsregler“: Abweichungsverstärkung, Geschwindigkeitsdämpfung [ms], Beschleunigungsdämpfung [ms²], Gesamtverstärkung) | τ_f = 1 ms Projektdefinition; Gewichtung im FluidSIM-Regler nicht dokumentiert | Signalbaustein ohne eigenen Prüffall |
| A06 | PT2-Glied T²·ÿ + 2·d·T·ẏ + y = K·u (halbimplizit, Teilschritte ≤ T/20), Ausgang im Bereich A08 | `sig` `Pt2` | F17 | Literaturform QS16 (k/((s/w)² + 2D·s/w + 1), T = 1/w); Kennwerte nach QS05 („PT2-Glied“) | – | R06 (y(T) = K·(1 − 2/e)) |
| A07 | I-Glied ẏ = K_I·u, gespeicherter Wert begrenzt auf ±Bereich A08 | `sig` `Integrator` | F17 | Literatur/Projektdefinition; QS05 („I-Glied“: Integrierbeiwert, Bereich aus A1/A2) | – | Signalbaustein ohne eigenen Prüffall |
| A08 | Ausgangsbereich der Bausteine mit A1/A2: beide ≥ U_min → ±η·min(\|U_A1\|, \|U_A2\|); nur einer angeschlossen → dessen Potential am Ausgang; sonst 0 | `sig` `supplyRange` | F17 | nach QS05 („I-Glied“, „PT2-Glied“, „Differenzverstärker“: betragsmäßig kleinster Wert, Wirkungsgrad, Mindestpotential, Durchreichen) | innere Widerstände R_In, R_Out nicht gerechnet | R06 (A1 15 V, A2 −10 V → ±9 V) |
| A09 | Komparator: ein bei U ≥ U_soll + H/2, aus bei U < U_soll − H/2; ein: Ausgang S = Potential an SP, im Stromlaufplan wie ein Plus-Netz (Stand des letzten Schritts) | `sig` `comparator`, `model` `electrics` | F41 | nach QS05 („Komparator“: Einschaltwert Sollwert + ½ Hysterese, Ausschaltwert Sollwert − ½ Hysterese, Mindestspannung 13 V) | ein Schritt Verzug zur Elektrik | R06 (Hysterese) |
| A10 | Proportional-Verstärker: Sollwert am Ventil u_V = ±10 V·(j + (1 − j)·min(\|U\|, 10 V)/10 V), 0 bei U = 0; 1 Kanal nur U > 0; 2 Kanäle A für U > 0, B für U < 0. Kopplung: Vorzeichen des Ventilanschlusses (Y2 und S positiv = P→A, Y1 negativ = P→B) mal Kanal (B negativ) | `sig` `amplifier`, `model` `linkAnalog` | F41 | nach QS05 („Proportional-Verstärker, 1/2 Kanäle“: 0 … 10 V bzw. ±10 V, Sprungstrom relativ zum Maximalstrom gegen Überdeckung, Versorgung 24 V); Vorzeichen aus Symbolzeichnung und Beispielschaltungen (Herleitung unten) | Strom, Spulenwiderstand und Magnetdynamik nicht gerechnet (FluidSIM passt den Strom ans Ventil an, der Kern bildet die Spannung direkt ab) | R06 (Sprungstrom, Kanäle) |
| A11 | Proportional-Ventilmagnet, lagegeregelt: u_V = clamp(U_C, −10, 10 V) (positive Spannung an S → P→A, A10) bei Versorgung \|U_A − U_B\| ≥ 20 V, sonst 0 (Ventil in Mittelstellung) | `model` `analog` | F41 | nach QS05 („Proportional-Ventilmagnet, lagegeregelt“: Spannungssignal gibt die Schieberposition vor, Regel- und Verstärkerteil im Ventil) | wie A10; die Schieberdynamik ist V05 | R06 (Lageregelkreis, Reset) |
| A12 | Durchflusssensor (hydraulischer Teil): Blende rT zwischen A und B (H03), ihr Durchfluss ist der Messwert von A01 | `model` FlowValve (flow sensor) | F11 | Modellnäherung; QS05 („Analog-Durchflussmesser“: hydraulischer Widerstand, Vorgabe 0,001 bar·min²/l²) | – | Signalbaustein ohne eigenen Prüffall |

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
| N06 | Teil-GRAFCET: alle Ketten mit derselben Elementinformation sim_grafcet (FluidSIM: Schritte im Rahmen G12) bilden einen Teil-GRAFCET, die übrigen den globalen GRAFCET; Schrittvariable X<n> im eigenen Teil-GRAFCET, von außen G<name>.X<n>, bei mehrfach vergebener Nummer intern G<name>_X<n>; XG<name> = 1, solange ein Schritt aktiv ist | `grafcet` `localized`, Konstruktor | F40 | QS05 (FluidSIM-Hilfe, Kapitel GRAFCET: „Einschließender Schritt“, „Makroschritte“, „Zwangssteuernde Befehle“, „Variablennamen“, „Schritt“; Beispiele Grafcet11–17) | Schrittnamen müssen im Teil-GRAFCET eindeutig sein | GRAFCET-Struktur (R14) |
| N07 | Einschließender Schritt: aktiviert die Aktivierungsschritte (Anfangsschritt-Symbol, sim_activation, FluidSIM „Aktivierungsverbindung“) der in sim_encloses genannten Teil-GRAFCETs; beim Deaktivieren werden alle ihre Schritte deaktiviert; Transitionen eines eingeschlossenen Teil-GRAFCETs schalten nicht im selben Durchlauf, in dem der einschließende Schritt verlassen wird | `grafcet` `activate`, `deactivate`, `cycle` | F40 | QS05 (FluidSIM-Hilfe, Kapitel GRAFCET: „Einschließender Schritt“, „Makroschritte“, „Zwangssteuernde Befehle“, „Variablennamen“, „Schritt“; Beispiele Grafcet11–17); Gleichzeitigkeitsregel eigene Festlegung | – | GRAFCET-Struktur (R14) |
| N08 | Makroschritt M<n>: Aktivieren aktiviert den Eingang E<n>; die Transition nach M<n> ist erst freigegeben, wenn der Ausgang S<n> aktiv ist, ihr Schalten deaktiviert M<n> und S<n> (die ganze Expansion) | `grafcet` | F40 | QS05 (FluidSIM-Hilfe, Kapitel GRAFCET: „Einschließender Schritt“, „Makroschritte“, „Zwangssteuernde Befehle“, „Variablennamen“, „Schritt“; Beispiele Grafcet11–17) | Expansion ohne Anfangsschritt, E<n> und S<n> in einer Kette | GRAFCET-Struktur (R14) |
| N09 | Zwangssteuernder Befehl (kontinuierliche Aktion) G<name>{8, 9}: genau diese Schritte aktiv; {} leere Situation; {*} einfrieren; {INIT} Anfangssituation; solange der Schritt aktiv ist, entwickelt sich der gezwungene Teil-GRAFCET nicht; der Zwang wird in jedem Durchlauf vor dem Schalten gesetzt | `grafcet` `force` | F40 | QS05 (FluidSIM-Hilfe, Kapitel GRAFCET: „Einschließender Schritt“, „Makroschritte“, „Zwangssteuernde Befehle“, „Variablennamen“, „Schritt“; Beispiele Grafcet11–17) | keine Prüfung der Hierarchie gegen gegenseitigen Zwang | GRAFCET-Struktur (R14) |

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
- **A02 (Kodierung):** Die Hilfe nennt die Signaltypen in der Reihenfolge Rechteck, Sinus, Dreieck, Konstant, Profil
  mit Standard „Sinus“; die Bibliothek `sym_fs.lib` hat waveForm = 2 als Vorgabe. Also zählt die Kodierung ab 1
  (1 Rechteck … 5 Profil). Die PID-Bereichswahl (outputMode 0 = −10 … 10 V, 1 = 0 … 10 V) folgt ebenso der
  Reihenfolge der Hilfe mit Vorgabe 0 = „−10 … 10 V“.
- **A04 (Parallelform):** K_I hat die Einheit 1/s, K_D Millisekunden. Nur in der Parallelform u = K_P·e + K_I·∫e dt +
  K_D·ė ergeben alle drei Glieder Volt (V·1/s·s = V, s·V/s = V); in der Reihenform K_P·(e + ∫e/T_i + T_d·ė) hätte
  K_I die Einheit einer Zeit. Daher T_i = K_P/K_I und T_d = K_D/K_P für die Filter- und Nachführzeiten.
- **A04 (Prüffall Abtastverfeinerung):** P-Regler an einem Integrator ẏ = 2·u: ẏ = 2·K_P·(r − y), also
  y(t) = r·(1 − e^(−2 K_P t)). Der explizite Euler hat den globalen Fehler O(dt): Halbierung von dt halbiert den Fehler.
- **A03 (Prüffall):** 0 → 2 V mit R1 = 0,25 s/V dauert 0,5 s (bei 0,25 s 1 V); nach 1 s von 2 V über R2 = 0,1 s/V
  in 0,2 s auf 0 V, dann mit R3 = 0,05 s/V in 0,1 s auf −2 V (bei 1,3 s).
- **A10/A11 (Vorzeichen):** In FluidSIM öffnet ein positiver Sollwert P→A. Belege: Im Symbol des 4/3-Ventils 01
  liegt rechts der Kasten P→A, B→T (die Schieberebene `Slide` verschiebt um PIC_x·24); der rechte Magnet Y2 drückt den
  Schieber nach links und bringt diesen Kasten vor die Anschlüsse. In allen Beispielen hängt Verstärkerausgang A
  (positiver Sollwert) an Y2, der Zylinder startet eingefahren. Beim Regelventil 02 zeigt TP511_15 (PID: W Sollwert,
  X Wegmesssystem, Y auf den lagegeregelten Magneten, A an der Kolbenseite), dass positive Spannung an S P→A öffnet;
  die Hilfe („Regelung in der Hydraulik“) fährt den Zylinder mit dem Wegmesssystem direkt am Magneten ein, weil dort
  Aggregat und Tank vertauscht sind. Gerechnet wird mit den Anschlussnamen des FluidSIM-Bauteils: gespiegelt beschriftet
  FluidSIM die Anschlüsse um (der Anschluss A trägt dann den Text „B“, P den Text „T“), die Simulation nutzt die Namen.
- **V11 (Druckwaage als Regler):** Schließer mit X an A: solange d = p_A − p_XS < p₀, ist das Ventil voll offen und die
  Pumpe (11,6 l/min) drückt mehr durch, als die Drossel rT = 1 bei p₀ abführt (√(10 bar/1) = 3,2 l/min); p_A steigt
  also über p₀. Ab p₀ + Δp_end ist es zu und p_A fällt über die Drossel. Dazwischen liegt das einzige Gleichgewicht,
  also p_A ∈ [p₀, p₀ + Δp_end]. Für die vorgeschaltete Druckwaage gilt dasselbe mit d = p_A − p_X über der Drossel.
- **V12 (Prüffall Nachsaugen):** Die Pumpe -GP5 saugt q aus dem Netz B, nachfließen kann nur T → B über rT_cv; im
  Gleichgewicht ist q = orificeFlow(p_T − p_B) (H03 mit p_s = 0,3 bar), nach p_B per Bisektion gelöst.
- **M11 (Nennpunkt):** Lineare Kennlinie M(n) = M_S·(1 − n/n₀) mit n₀ = k·n_N. Aus M(n_N) = M_N folgt
  M_S = M_N·k/(k − 1); als Welle ist M_S = D_θ·p_N (Stillstand) und M_S = b·ω₀ (Leerlauf), daraus D_θ und b. Bei
  anderem Druck p skaliert das Stillstandsmoment mit p und die Leerlaufdrehzahl ω₀·p/p_N (Prüffall: 6 bar doppelt so
  schnell wie 3 bar; Nennpunkt 3 bar mit 0,62 N·m bremsend genau 975 1/min).
- **M12 (Haltepunkte):** C1 allein: der hintere Kolben schiebt den vorderen bis x_max2 und bleibt dort (Zwischenstellung), sofern C3 ein Luftpolster hat (im Prüfplan Blindstopfen); sind C2 und C3 offen, läuft der vordere Kolben mit seinem Schwung über x_max2 hinaus (Prüflauf zuerst: 117 mm), wie ein realer Zylinder ohne Gegendruck. C2 allein: der vordere Kolben fährt allein bis x_max, der hintere bleibt hinten. C1 und C3 zugleich vom vorderen Ende aus: der hintere Kolben fährt bis x_max2, der vordere fährt zurück, bis er an der Stange anliegt; dort hält ihn p·A_k > p·(A_k − A_St), also steht der Zylinder in der Zwischenstellung (Zwischenhalt im Rückhub „mit entsprechender Ansteuerung“). C3 allein: beide gemeinsam zurück.
- **H08 (Prüffall LS):** Bei geschlossenem P1 hält nur die Leckage die Pumpe auf; aus p = p₀ + Δp_end·(1 − α) und α·q = G·p folgt p = (p₀ + Δp_end)/(1 + Δp_end·G/q) = 12,44 bar. Hinter einer Messdrossel r_T stellt der Regler Δp = p₀ + Δp_end·(1 − α) ein; damit ist q = √(Δp/r_T) fast unabhängig vom Lastdruck (Prüfplan: Lastdrossel 1 und 2 bar·min²/l², Volumenstrom gleich bis auf die kleine Änderung von α).
- **H10 (Prüffall Lenkung):** Voll ausgelenkt fließt der ganze Pumpenstrom über L in die Belastungseinheit (v = q/A_k, rechts −q/A_r), die Dosierpumpe dreht das Lenkrad um V/D·2π; halb ausgelenkt teilt sich der Strom auf E und L (Bilanz q_Pumpe = q_H03(p, r_T/0,25) + A_k·v); neutral p aus der Blende H03 (bei 1,7 bar 1,5 % über r_T·q², Glättung p_s = 0,3 bar).
- **V16 (Prüffall Sperrblock):** Mit A2 und B1 am selben Pumpendruck p ist der Überschuss 3,3·p − p − 2 bar > Δp_end, das Ventil also ganz offen und p folgt aus der Blende H03; ohne Steuerdruck sperrt es, die Pumpe geht auf ihre Druckbegrenzung; vorwärts p = p₀ + Blendendruck.
- **V17 (Prüffall LS-Steuerblock):** 30 % Vorsteuerung (4,5 bar) ergibt x = 2,5/13; die Druckwaage hält 2,01 l/min bei 52,6 und 33,4 bar Pumpendruck (Band x·√(10,5/0,1) … x·√(11/0,1) = 1,971 … 2,017 l/min). Beim Einfahren drückt die Rücklaufkante das 1,64-fache Volumen der Kolbenseite: der Stangenraum braucht rund 100 bar, die Pumpe daher 200 bar; der Rückstaudruck baut sich in den Netzvolumen (V_HYD 0,1 l) langsam auf, gemessen wird nach 1,5 s auf 1 m Hub.
- **V18 (Prüffall Prioritätsventil):** CF über r_T 1 ins Freie, EF offen: p_CF, p_P aus den drei H03-Blenden und der Pumpenbilanz.
- **R06 (Druckregelkreis):** Mit PI-Anteil verschwindet die bleibende Regelabweichung: der Sensor (0 … 10 MPa →
  0 … 10 V) meldet beim Sollwert 4 V genau 40 bar. Die Stellgröße stellt sich auf den Öffnungsdruck ein, bei dem
  V09 die Pumpenförderung abführt (rund 2,8 V).

## Abweichungen von FluidSIM (bekannt, nicht nachgebildet)

- Zylinderfeder: FluidSIM führt „Federdruck bei x = 0“ und „bei x = x_max“ (lineare Kennlinie); der Kern rechnet eine
  konstante Federkraft (15 % von A_K·6 bar). Endlagendämpfung („Dämpfungslänge“) und Zylinderleckage fehlen (QS05).
- Sauger: FluidSIM senkt und hebt per Klick („Position“, Vorgabe 0 = angehoben); der Kern nimmt `object` 1
  (Werkstück liegt an) als Vorgabe. Der Saugvolumen-Kennwert des Ejektors mit Abstoßeinrichtung („Volumen“) und das
  Abstoßen fehlen.
- Pilotventile: FluidSIM rechnet eine Kraftbilanz aus Steuerfläche und Feder; der Kern schaltet ab 1,5 bar (V04).
- Gaskonstante 287 statt 288 J/(kg·K) in den Kammern (0,35 %).
- Analoge Elektrik (R06): FluidSIM rechnet Ströme und Widerstände der Karten (R, iOutMax, R_In, R_Out) und passt den
  Magnetstrom ans Ventil an; der Kern führt nur Potentiale (A00, A10). Potentiometer, Oszilloskop, Strom- und
  Widerstandsmesser haben kein Modell und bleiben ohne Wirkung; Spannungsquellen mit Wechselspannung (L1 … L3) gelten
  als 24 V.

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
R06: Quellen ohne Potentialangabe 24 V bzw. 0 V, Versorgungsschwelle der Sollwertkarte 20 V, Ableitungsfilter τ_d =
T_d/10 (ohne D-Anteil 1 ms), Nachführzeit 0,9·T_i (ohne I-Anteil 0,1 s), Ableitungsfilter des Zustandsreglers 1 ms,
Profil des Funktionsgenerators ohne Amplitude/Versatz. Die übrigen Schwellen (15 V Sollwertkarte, 13 V Komparator und
Wegmesssystem, 20 V Verstärker und Ventilmagnet, 21,6 V Regler, 1,5 V und η = 0,9 der TP-1013-Glieder) sind
FluidSIM-Vorgaben (QS05), nicht von uns gemessen.
R09: Druckwaagen-Schließer schließt erst ab p₀ (Band darüber), Übergangsbreite des pneumatischen Folgeventils
0,1 bar, Normal-Nenndurchfluss pneumatischer Druckwaagen ohne Angabe 300 l/min (FluidSIM-Vorgabe der Bibliothek).

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
- **QS16** – Modelica Standard Library 4.0.0, Blocks.Continuous.SecondOrder
  (https://doc.modelica.org/Modelica%204.0.0/Resources/helpWSM/Modelica/Modelica.Blocks.Continuous.SecondOrder.html):
  y/u = k/((s/w)² + 2·D·(s/w) + 1), Parameter k, w, D. Zugriff 17.11.2026.
- **S23** (Paket) – MSL 4.0.0, Blocks.Continuous.LimPID
  (https://doc.modelica.org/Modelica%204.0.0/Resources/helpWSM/Modelica/Modelica.Blocks.Continuous.LimPID.html):
  Nd = 10 („je größer Nd, desto idealer das D-Glied“), Ni = 0,9 („Ni·Ti ist die Zeitkonstante der
  Anti-Windup-Kompensation“), wd = 0 (Sollwertgewicht im D-Anteil). Die Filterformel T_d/Nd steht nicht auf der Seite.
  Zugriff 17.11.2026.

Eine neue Formel bekommt hier ihre Zeile mit Codestelle, Herkunft, Beleg, Grenze und Prüfung, bevor sie freigegeben
wird; eine neue Quelle bekommt eine QS-Nummer mit Fundstelle und Zugriffsdatum.
