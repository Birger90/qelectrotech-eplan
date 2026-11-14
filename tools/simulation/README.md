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
  an …). Ventil von Hand: `--event "0.2 -QM1 links an"`.
- **Prototyp in Python** (Referenz, mit Diagramm): `tools/simulation/fluid_simulation.py`.

## Was gerechnet wird

| Bauteil | Modell |
|---|---|
| Leitungsnetz | Knoten mit Volumen; Luft isotherm (ideales Gas), Öl mit Kompressionsmodul; offener Anschluss = ins Freie |
| Druckluftquelle | hält den Versorgungsdruck (Fenster: *Versorgungsdruck*) |
| Hydraulikpumpe | Verdrängung × Drehzahl, eingebaute Druckbegrenzung bei `pMax` |
| Tank | drucklos |
| Wegeventil | Schaltwege der Stellung; Luft nach ISO 6358 aus `qNN`, Öl mit Widerstand `rT` (Δp = rT·Q²); Schaltzeiten `onDelay`/`offDelay`; Feder in die Ruhestellung, ohne Feder bleibt die Stellung; Steuerdruck an 14/12 |
| Rückschlag-, Drosselrückschlagventil | freie Richtung `free_from` → `free_to` (aus der Zeichnung); entsperrbar über Steueranschluss |
| Drossel, Absperrventil, Schalldämpfer, Schnellentlüftung | Leitwert aus `qNN` bzw. `rT` |
| Druckbegrenzung, Druckregler | Einstelldruck `p0`, Öffnungsbereich `dpEnd` |
| Zylinder | Kraftbilanz: Drücke × Flächen, Masse, Haft-/Gleitreibung, Feder, Last, Endanschläge |
| Spannungsquelle | oberer Anschluss Plus, unterer Minus |
| Taster, Endschalter | Schließer/Öffner aus dem Symbol |
| Relais | Spule (Master) und verknüpfte Kontakte (Slaves), Anzug 10 ms, Abfall 8 ms |
| Zeitrelais | ansprech- bzw. abfallverzögerte Spule oder Kontakte; Zeit im Funktionstext der Spule („1 s“, „500 ms“) |

## Zuordnungen im Plan

- **Ventilspule → Ventil:** Die Spule trägt das Kennzeichen des Ventils, z. B. `-QM1`. Bei zwei Magneten
  `-QM1/14` (links) und `-QM1/12` (rechts).
- **Endschalter → Zylinder:** Funktionstext des Schalters „`-MM1 vorne`“ bzw. „`-MM1 hinten`“.
- **Kennwerte** stehen als `<parameter>` im Symbol und lassen sich je Bauteil anpassen.

## Grenzen

Die Modelle sind eigene, aus der Fachphysik; sie sind gegen Handrechnungen geprüft (Hydraulik: Geschwindigkeit =
Förderstrom/Fläche), noch nicht gegen FluidSIM. Noch nicht simuliert: Stromteiler, Zähler, SPS/GRAFCET.
