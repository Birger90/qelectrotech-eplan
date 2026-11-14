# Referenzlauf für `--simulate-reference`

`OwnSim.Examples.CylinderCycle_res.csv`: Ergebnis des Referenzmodells `OwnSim.Examples.CylinderCycle`
(doppeltwirkender Zylinder, Hub 60 mm, Kolbenfläche 4 cm², bewegte Masse 0,5 kg, zwei 3/2-Ventile mit
Leitwert 2e-9 kg/(s Pa), Versorgung 6 bar abs, Umschaltung bei 0,4 s), gerechnet mit OpenModelica 1.27.1,
DASSL, Toleranz 1e-9. Hier stehen nur die Spalten Zeit, Weg und Kammerdrücke (SI, absolut); die
Originaldatei hatte die SHA-256-Prüfsumme `a724a44179291617ea67f5f3725ce1899c8fc9edfb98d4d081914c4dce3b375c`.

```
QElectroTech --simulate-reference tools/simulation/referenz
```
