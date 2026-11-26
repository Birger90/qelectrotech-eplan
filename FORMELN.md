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

## Einrasten und Anreihen

| ID | Beziehung | Codestelle | Herkunft, Quelle | Grenze |
|---|---|---|---|---|
| E1 | Auf der Hutschiene sitzt das Gerät mit seinem Mittenversatz auf der Schienenmitte: Gerätemitte = Schienenmitte + Mittenversatz (`middle_offset_mm`, EPLAN „Mittenversatz“, ID 22215); ohne Angabe die Gerätemitte | `layout` `railSnap` | Q4 (Montagedaten: Verschiebung in der Frontansicht gegenüber der Tragschienenmitte); Vorzeichen Projektdefinition (positiv = Gerät höher), die Hilfe nennt keins | nur waagerechte Schienen |
| E2 | In 3D liegt die Rückseite eines Geräts auf der Schiene um die Aufklipshöhe (`snap_height_mm`, EPLAN ID 22211) hinter der Schienenoberkante, nicht tiefer als die Platte | `3d` `build` | Q4 (Bezug Schienenoberkante); Richtung Projektdefinition | – |
| E3 | Bündig anreihen: liegt eine freie Kante eines Nachbarn (Gerät auf derselben Schiene; frei auf der Platte auch Kanal oder C-Profil) höchstens 4 mm entfernt, rastet das Gerät an ihr an; frei auf der Platte richtet es sich außerdem an Ober-/Unterkante bzw. Seite des Nachbarn aus, wenn diese ebenso nahe liegt; nie auf ein anderes Teil | `layout` `railSnap`, `kFlushCatchMm` | Projektdefinition | Schienenanfang fängt weiter bis 15 mm |

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
