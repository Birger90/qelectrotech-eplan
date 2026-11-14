# Referenzläufe für `--simulate-reference`

`OwnSim.Examples.CylinderCycle_res.csv`: Ergebnis des Referenzmodells `OwnSim.Examples.CylinderCycle`
(doppeltwirkender Zylinder, Hub 60 mm, Kolbenfläche 4 cm², bewegte Masse 0,5 kg, zwei 3/2-Ventile mit
Leitwert 2e-9 kg/(s Pa), Versorgung 6 bar abs, Umschaltung bei 0,4 s), gerechnet mit OpenModelica 1.27.1,
DASSL, Toleranz 1e-9. Hier stehen nur die Spalten Zeit, Weg und Kammerdrücke (SI, absolut); die
Originaldatei hatte die SHA-256-Prüfsumme `a724a44179291617ea67f5f3725ce1899c8fc9edfb98d4d081914c4dce3b375c`.

```
QElectroTech --simulate-reference tools/simulation/referenz
```

`OwnHydraulics.Examples.CylinderCycle_res.csv`: hydraulischer Zyklus aus dem Referenzpaket 2.0 – Pumpe 1e-4 m³/s,
Druckbegrenzung 5 MPa, 4/3-Ventil (Blenden 3 mm², Schieber-Zeitkonstante 10 ms; +1 bis 0,7 s, Mitte bis 0,9 s,
dann −1), Differentialzylinder Hub 100 mm, Flächen 10/7 cm², Totvolumen je 0,1 l, Masse 50 kg, Öl 5e8 Pa.
Spalten Zeit, Weg, Kammerdrücke, Versorgungsdruck, Schieberstellung (Überdruck in Pa). SHA-256 der Originaldatei:
`09feba3bfb67678c7010a13f95e4785d5e0d536ec4876be717180752e6372308`.

Ohne Ordnerangabe sucht `--simulate-reference` die Dateien neben dem Programm (`simulation/referenz`), dorthin legen
Installation, macOS-Bündel und `tools/deploy_lokal.ps1` sie ab.
