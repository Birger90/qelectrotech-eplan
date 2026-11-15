# Stand der Simulations-Roadmap (R00–R19)

Arbeitsauftrag: externes Paket „Claude_Simulation_Roadmap“ (Roadmap R00–R19, Funktionsmatrix mit 94 Zeilen, geprüfte
Basis 220ccf2). Hier steht je Paket, was belegt umgesetzt ist, womit es geprüft wird und was offen bleibt. Ein Paket
gilt erst als abgeschlossen, wenn seine Prüfungen im echten Kern (`--simulate-reference`, `SimulationModel` über den
Projektlader) bestehen. Für reale Genauigkeitszusagen fehlen weiterhin unabhängige Messdaten.

| Paket | Stand | Belege | Matrix |
|---|---|---|---|
| R00 Modellkatalog | **umgesetzt** (Fork 1.23.0) | Prüfung „Modellkatalog“: Vakuumerzeuger, Greifer, unbekannte Funktion vor dem Lauf abgewiesen; Grafikbaustein ohne Fehler; 437/437 Sammlungssymbole (43 Funktionen) im Katalog 1.0; `--simulate-catalog` | K02, Q01 (Katalogteil), Q05 |
| R01 Messgrößen | **umgesetzt** (Fork 1.23.0): Drücke (Über-/Absolutdruck), gerichtete Durchflüsse je Anschluss, Zylinder x/v/F, Ventilstellung, Laufidentität und Status | Prüfung „Messgrößen“: Abgriff ohne Rückwirkung, Ventilbilanz, Pumpenstrom = Verdrängung · Drehzahl, unbekannte Kennung ohne Wert | K10, K11 (ohne Winkel/Leistung/Energie), K12, K15 (Laufidentität) |
| R02 Endlagen, Drucktransienten | offen | Vergleichslücke 10 ms nach dem Anschlag besteht weiter und wird im Prüfstand genannt | H20, H21, M02, Q02 |
| R03 Anfangszustand, Numerik | teilweise (Konvergenz analytischer Fälle, Abbruch bei Fehlern) | GasCharge/HydraulicCharge mit Verfeinerung, runUntil-Fehlerpfad | K03–K05, Q04 |
| R04 Vergleichsbestand | teilweise (OpenModelica pneumatisch/hydraulisch, 6 Projekte, 1136 FluidSIM-Beispiele als Rauchprüfung) | `--simulate-reference`, `fluidsim_batch.py` | K15, Q01–Q05 |
| R05 Leitungen, Aufbereitung | teilweise: Aufbereitung als Durchgangswiderstand, Leitung/Kupplung ideal | Katalog | H06–H08, H25, P12 |
| R06–R19 | offen | – | siehe Matrix |

Nicht unterstützte Funktionen stehen mit Grund im Katalog (`--simulate-catalog`, Rolle `unsupported`); ein Projekt
mit ihnen startet nicht.
