# Datenblattkurven für die Simulation

Kennlinien werden nie abgeschätzt, sondern aus den Vektorpfaden der Hersteller-PDFs gelesen.

- `dmsp_kennlinien.py DMSP_EN.PDF dmsp.json` liest die Kraft-Weg-Diagramme des Festo-Katalogs „Fluidic Muscle DMSP“
  (Stand 2026/01, S. 14/15): Achsen über die Teilstriche, Druckkurven in Zeichenreihenfolge, geprüft auf steigende
  Kraft mit dem Druck. Quelle: https://ftp.festo.com/public/PNEUMATIC/SOFTWARE_SERVICE/Documentation/2026/PT/DMSP_EN.PDF
- `erzeuge_muskelkopf.py dmsp.json ../../../sources/simulation/simulationmuscle.h` schreibt die Tabellen samt
  Datenblattwerten (I.D., Gewichte, Vorspannung, Kontraktion, Druck, Aufweitung) als C++-Kopf.

Das PDF selbst liegt nicht im Repository (Urheberrecht Festo); siehe FORMELN.md QS17.
