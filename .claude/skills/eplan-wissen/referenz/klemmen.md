# EPLAN: Klemmen, Klemmenleisten und Stecker

Referenz für den inhaltlich korrekten Nachbau von EPLAN-Klemmenleisten, Mehrstockklemmen, Brücken, Klemmenplänen und Steckern in QElectroTech (Fork). Grundlage ist die offizielle EPLAN-Hilfe (Plattform 2.9, abgeglichen mit 2023 bis 2026). Alles ist in eigenen Worten zusammengefasst. Jede Aussage nennt ihre Quelle. Abschnitte „Bedeutung für QET“ übertragen das auf den Fork. Mit **(Vermutung)** markierte Stellen sind nicht aus der Hilfe belegt.

Die Struktur der Hilfe ist von 2.9 bis 2026 gleich (gleiche Themenseiten unter `terminalgui_*`, `plugsgui_*`). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/terminalgui_k_start.htm

## Inhalt

1. Grundbegriffe und Funktionsumfang
2. Klemmenleistendefinition
3. Klemme: Eigenschaften (Registerkarte Klemme)
4. Hauptklemme, Nebenklemme, eigenständige Bereiche
5. Mehrstockklemmen (Etagen)
6. Potenzial-/Verteilerklemmen und Teilklemmen
7. Brücken (Steg-, Draht-, Einlege-, Schaltbrücke)
8. Sortierung (Geräteposition, Sortierkennung)
9. Nummerierung
10. Innen/Außen (intern/extern) und Verbindungen der Klemme
11. Dialog „Klemmenleiste bearbeiten“ und Klemmenleisten-Navigator
12. Auswertungen: Klemmenplan, Klemmenanschlussplan, Formulare
13. Stecker
14. Neuerungen neuerer Versionen
15. Ergänzung: Dialog „Klemmenleiste bearbeiten“ im Detail
16. Klemmenleisten-Navigator, Verschieben, Klemmen erzeugen
17. Zubehör (aufreihbar / nicht aufreihbar)
18. Teilklemmen: Einstellungen, Einfügen, Optimieren
19. Angeschlossene SPS-Adresse an Klemmen und Steckerkontakten
20. Ergänzung Stecker: Navigator, Definitionen, Bearbeiten
21. Seitenumbruch im Klemmenplan (Lücke)
22. Ergänzung: Funktionen per Nummerierungsmuster, Mehrstock-Etagen, Stecker-Spalten, Kompatibilitätsoptionen
23. Quellen

---

## 1. Grundbegriffe und Funktionsumfang

- EPLAN bietet: vordefinierte Klemmen/Leisten aus einer Funktionsdefinitionsauswahl, Haupt- und Nebenklemmen (mehrere getrennte Klemmen in einem Gehäuse), bis zu 50 Artikel je Klemme oder Leiste, Nummerierungsschemata, mehrere Sortierarten, automatische und manuelle Brücken, Potenzialtypen, Mehrstockklemmen mit wählbarer Etagenzahl, verteilte Darstellung über Teilklemmen sowie Import/Export von Klemmenleisten. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_start.htm
- Die Klemmenleiste ist die **Hauptfunktion** des Betriebsmittels, die einzelnen Klemmen sind Haupt- oder Nebenklemmen (ein Betriebsmittel hat nur eine Hauptfunktion). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_hauptklemmen.htm
- Eine Klemme darf in mehreren Darstellungsarten vorkommen, aber je Darstellungsart nur einmal; Inkonsistenzen meldet die Projektprüfung. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwaltung.htm
- Die **Funktionsdefinition** einer Klemme gibt maximale Zielanzahl, Stegbrückenmöglichkeiten, Potenzialtyp und Potenzialweiterleitung vor; diese Werte sind in der Anschlusslogik einzeln änderbar. Die **Klemmenausführung** (Durchgangs-, Trenn-, Schalt-, Diodenklemme …) ist eine beschreibende Eigenschaft für Auswertungen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwaltung.htm

**Bedeutung für QET:** Der Fork bildet das als `TerminalStrip` (Leiste) → `PhysicalTerminal` (Klemmengerät) → `RealTerminal` (eine Etage/ein Klemmpunkt) ab. Die EPLAN-Klemmenausführung entspricht grob `ElementData::TerminalType` (Generic, Fuse, Sectional = Trennklemme, Diode, Ground = PE, …), der Potenzialtyp grob `TerminalFunction` (Generic, Phase, Neutral) bzw. dem Projekt-Potenzialtyp am Leiter. Quelle: `sources/TerminalStrip/physicalterminal.h`, `sources/properties/elementdata.h` (Fork).

## 2. Klemmenleistendefinition

- Die Klemmenleistendefinition identifiziert die Leiste und trägt deren Daten und Leistenartikel. Sie kann platziert oder nicht platziert sein; kopieren lässt sie sich nur, wenn sie platziert ist. Nummeriert wird sie über die normale BMK-Nummerierung (nicht über die Klemmennummerierung). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwaltungleisten.htm
- **Vordefinierte** Leisten haben eine Klemmenleistendefinition mit Aufbau und Eigenschaften. **Nicht vordefinierte** Leisten entstehen allein durch das BMK beim Einfügen einer Klemme; sie erscheinen in Auswertungen, haben aber keine Eigenschaften (nachrüstbar über eine Definition). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmenleistenerstlbearb.htm
- Felder der Registerkarte Klemmenleistendefinition: sichtbares/vollständiges BMK, **Klemmenplanformular** und **Anschlussplanformular** (haben Vorrang vor den Auswertungseinstellungen), Funktionstext, Montageort (beschreibend), Hauptfunktion, „Hauptklemmen nicht verwenden“ sowie Unterdrückungen „Keine Ausgabe im Klemmen-/Steckerplan“, „… im Klemmen-/Steckeranschlussplan“, „… in der Klemmenleisten-/Steckerübersicht“, „… im Klemmenaufreihplan“. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_klemmenleisten.htm
- Beim Einfügen einer Klemme mit bekanntem BMK wird sie dieser Leiste zugeordnet; ein neues BMK erzeugt eine neue Leiste ohne Definition. Liegt in Suchrichtung schon eine Klemme, übernimmt die neue deren BMK (Umschalt kehrt das um). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmenzeichnen.htm
- Kompatibilitätsmodus (wie vor EPLAN 2.0): ganze Leiste ist ein Bereich, Aufbau über eine Gerätedefinition an der Leistendefinition; nur ohne Hauptklemmen oder mit „Hauptklemmen nicht verwenden“, nicht empfohlen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwaltungleisten.htm

**Bedeutung für QET:** Die Leiste entspricht einem `TerminalStrip` mit Name = BMK (z. B. „-X1“) und gegebenenfalls Anlage/Ort. Das Kennzeichen jeder Klemme muss im Bauteil stehen (Element-Information `label`), sonst ordnet weder der Klemmleisten-Editor noch eine Auswertung die Klemme zu. Eine eigene „Klemmenleistendefinition“ als Symbol gibt es in QET nicht. Steht sie im EPLAN-Plan, wird sie als Text/Rahmen nachgezeichnet. Die Ausgabe-Unterdrückungen muss der eigene Klemmenplan-Generator beachten. (Vermutung: im EPLAN-PDF sieht man sie nur daran, dass für die Leiste kein Klemmenplan erzeugt wurde.)

## 3. Klemme: Eigenschaften (Registerkarte Klemme)

- **Bezeichnung** = Position der Klemme in der Leiste (Eigenschaft Klemmen-/Steckerkontaktbezeichnung). Ein Doppelpunkt im sichtbaren BMK (`-X1:5`) schreibt den Teil hinter dem Doppelpunkt in dieses Feld. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_klemmen.htm
- **Anschlussbezeichnung/-beschreibung**: Anzeige am Symbol über „Anschlussbezeichnungen (alle)“, ID 20038. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_klemmen.htm
- **Etage**: 0 = einfache Klemme, > 0 = Mehrstockklemme, 1 = unterste Etage. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_klemmen.htm
- **Stegbrücke**: Automatisch, Automatisch Brückenanfang, Automatisch Brückenende, Keine automatische Brücke. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_klemmen.htm
- **Hauptklemme** (Kontrollkästchen), **Teilklemme** (Kontrollkästchen; dann keine Registerkarte Verbindungen), **Funktionsdefinition**, **Klemmenausführung**, **Gleiche Bezeichnung erlaubt** (unterdrückt Meldungen bei Doppelbezeichnungen, typisch PE/PEN; nicht bei Platzierung in mehreren Darstellungsarten aktivieren), **Klemme: Geräteposition**. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_klemmen.htm
- Die Klemmenbezeichnung kann nach DIN EN ISO 81714-2 gedreht dargestellt werden (Projekteinstellung „Anschlussbezeichnung gedreht darstellen“; gilt auch für Steckerkontakte). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwaltung.htm

**Bedeutung für QET:** Klemmennummer = Bauteil-Kennzeichen (`label`), nicht freier Text. Die Etage entspricht dem Level in `PhysicalTerminal` (Index 0 = hinten/Montageplatte). Die Richtung ist also umgekehrt zu einer „Etage 1 = unten“-Lesart, und bei der Übernahme muss man die Reihenfolge prüfen **(Vermutung zur Zuordnung, Quelle: Kommentar in `sources/TerminalStrip/physicalterminal.h`)**. Doppelte PE-Bezeichnungen sind in EPLAN zulässig und dürfen in QET nicht „korrigiert“ werden.

## 4. Hauptklemme, Nebenklemme, eigenständige Bereiche

- Eine neu eingefügte Klemme ist standardmäßig Hauptklemme. Nebenklemme entsteht durch Abwählen von „Hauptklemme“; sie gehört zur Hauptklemme mit gleicher Geräteposition. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_hauptklemmen.htm
- Ein **eigenständiger Bereich** reicht von einer Hauptklemme bis zur nächsten. Klemmen vor der ersten Hauptklemme bilden einen Bereich, der über die Leistendefinition bestimmt wird. Beispiel der Hilfe: Klemmen 1–7, Hauptklemmen 3 und 6 → Bereiche {1,2}, {3,4,5}, {6,7}. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_hauptklemmen.htm
- Ein Artikel an der Hauptklemme überlagert mit seinen Funktionsschablonen die Hauptklemme und alle Nebenklemmen gleicher Geräteposition. Die Zuweisungsreihenfolge innerhalb des Bereichs bestimmt die Sortierkennung. Artikel nur an Hauptklemmen (Voraussetzung für Sichtbarkeit im 3D-Montageaufbau und für externe Klemmenkonfiguratoren). Empfehlung: Leiste entweder ganz mit oder ganz ohne Hauptklemmen aufbauen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_hauptklemmen.htm
- Gerätedefinitionen (Klemmenaufbau) werden in der Artikelverwaltung über Funktionsschablonen angelegt. Die Schablone der Hauptklemme steht oben. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwaltungleisten.htm

**Bedeutung für QET:** Haupt-/Nebenklemme entspricht: ein `PhysicalTerminal` mit mehreren `RealTerminal`. Der Artikel (Bestellnummer, Typ) gehört an das Klemmengerät, nicht an jede Etage. Im Klemmenplan erscheint der Klemmentyp daher nur in der Zeile der Hauptklemme. Steht der Klemmentyp nicht im Stromlaufplan, kommt er aus dem Original-Klemmenplan des EPLAN-PDFs.

## 5. Mehrstockklemmen (Etagen)

- Eine Mehrstockklemme besteht aus mehreren übereinanderliegenden Etagen. Jede Klemme hat eine Etageninformation. Etagen können verschiedene Funktionen haben, je einen eigenen Potenzialtyp und jeweils eigene (oder keine) Stegbrückenanschlüsse. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_mehrstockklemmen.htm
- Zugehörigkeit: gleiches BMK, in der Sortierung direkt aufeinanderfolgend, Etagenwerte stetig steigend oder fallend, gleiche Geräteposition. Klemmen aus verschiedenen Bereichen bilden nie eine Mehrstockklemme. Genau eine Hauptklemme je Mehrstockklemme. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_mehrstockklemmen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_hauptklemmen.htm
- Hauptklemme ist die unterste **oder** oberste Etage und trägt den Artikel. Bei oberster Etage dreht EPLAN die Schablonenreihenfolge selbst um (Etage 1 → aufsteigend, Etage > 1 → absteigend). Die Hauptklemme soll der ersten Schablone zugeordnet sein, sonst meldet der Prüflauf. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_mehrstockklemmen.htm
- Erzeugen im Dialog „Klemmenleiste bearbeiten“: „Mehrstockklemme erzeugen“ fasst alle markierten Klemmen zusammen, „Mehrstockklemmen erzeugen“ gruppiert nach vorgegebener Etagenzahl (8 Klemmen, 3 Etagen → 2 Mehrstockklemmen, 2 Klemmen bleiben einzeln). Etagen werden absteigend vergeben, die oberste wird Hauptklemme. „Trennen“ löst wieder auf. Manuell: Etage > 0, gleiche Geräteposition, Sortierkennung, Hauptklemme an-/abwählen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_mehrstockklemmenarbeit.htm
- Nummerierung: Eigenschaft „Klemmenleisten: Gleiche Bezeichnungen innerhalb von Mehrstockklemmen zulassen“. Ist sie aktiv, bekommen beim Schema „Numerisch“ alle Etagen dieselbe Bezeichnung (Beispiel der Hilfe: 3, 4+, 5- → 1, 1+, 1-). Bei „Seitenorientiert“ bestimmt die grafisch erste Klemme die Bezeichnung. „Alphabetische Bestandteile beibehalten“ hat Vorrang. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_mehrstockklemmen.htm
- Im Klemmenplan zeigt Spalte A die Klemmenbezeichnungen mittig, links die externen und rechts die internen Anschlüsse mit Anschlussbezeichnung. Spalte B zeigt die Stegbrücken. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_mehrstockklemmen.htm
- TechTipp „Klemmen mit variablem Aufbau“ (Plattform 2026): Basisklemme einfügen, im Eigenschaftendialog als Hauptklemme markieren und dann im Dialog „Klemmenleiste bearbeiten“ zu einer Mehrstockklemme gruppieren. Die Reihenfolge in der Leiste bestimmt die Ausgabe im Klemmenplan (nur über Suchergebnis-Auszug belegt, PDF nicht vollständig lesbar). Quelle: https://www.eplan.help/techtipps/de-DE/Klemmen/TechTip-Terminals-with-variable-structure.pdf

**Bedeutung für QET:** EPLAN-Etagen „5“, „5+“, „5-“, „5PE“ derselben Mehrstockklemme gehören zu **einem** `PhysicalTerminal` mit mehreren Levels. Im Klemmenplan bekommt jede Etage eine eigene Zeile, in der Reihenfolge der Funktionsschablonen des Artikels (nicht alphabetisch). Diese Reihenfolge liest man aus dem Original-Klemmenplan ab. Im Klemmleisten-Editor des Forks setzt man den Level über „changeterminallevel“ (UndoCommand vorhanden). Die Klemmenbezeichnung der Etagen darf gleich sein; das Vorzeichen (+/-/PE) steht dann als Bezeichnungsanteil.

## 6. Potenzial-/Verteilerklemmen und Teilklemmen

- **Potenzialklemmen** verteilen ein Potenzial. Ihre Etagen werden in EPLAN nicht projektiert. Sie sind *eine* Klemmenfunktion mit n Anschlüssen auf Etage 0. Es gibt Symbole mit 3 bis 8 Anschlüssen. Anschlussbezeichnungen (üblich a, b, c, d …) sind Pflicht, damit sie in Auswertungen erscheinen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_mehrstockklemmen.htm
- **Teilklemmen** stellen die Anschlüsse einer Klemme verteilt dar (dort, wo sie gebraucht werden). Zusammengehörig sind sie bei gleicher Funktionsdefinition, gleichem BMK, gleicher Klemmenbezeichnung und gleicher Etage. Der **Teilklemmenindex** unterscheidet Teilklemmen gleicher Bezeichnung. Ziele werden nur an Anschlüssen mit Anschlussbezeichnung ausgewertet. In Auswertungen werden Teilklemmen zu einer Klemme zusammengefasst. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verteiltedarstellungklemmen.htm
- **Verteilerklemmen** werden mit der Funktionsdefinition „Klemme, variabel“ (beliebig viele Anschlüsse) gebaut. Damit das Potenzial über alle Anschlüsse läuft, wird „Potenzial weiterleiten nach“ gesetzt: am ersten Anschluss die Nummer des letzten, am letzten die des ersten (20 Anschlüsse: 1 → „20“, 20 → „1“). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verteiltedarstellungklemmen.htm
- Mit der Projekteinstellung „Zusammengehörige Teilklemmen automatisch verbinden“ werden Teilklemmen beim Aktualisieren der Verbindungen intern verbunden, auch ohne gezeichnete Linie. Diese **internen Brücken** liegen am letzten Anschluss der Klemmenfunktion, und die Reihenfolge im Navigator ist maßgeblich. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_brueckenbildung.htm
- **Teilklemmenquerverweis** (ID 20252): Die erste Teilklemme (Hauptklemme bzw. die mit der kleinsten Anschlussbezeichnung) verweist auf alle anderen, diese verweisen zurück. Standardebene EPLAN444. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verteiltedarstellungklemmen.htm
- **PE-Klemmen**: Zwischen zwei PE-/PEN-Klemmen entsteht immer eine Direktverbindung ohne Ziele; automatische Stegbrücken werden für PE/PEN nicht vorgeschlagen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwendungbruecken.htm . Bei Funktionsdefinitionen mit **Schienenkontakt** (z. B. PE-Klemme) werden diese Direktanschlüsse in Auswertungen nicht ausgegeben. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_auswertungklemmen.htm

**Bedeutung für QET:** Eine Verteiler-/Potenzialklemme mit a–h ist ein Klemmengerät mit einer Bezeichnung. In QET entweder ein Bauteil mit n Anschlüssen (Anschlussnamen a, b, c …) oder verteilte Einzelelemente mit gleichem Kennzeichen; dann müssen sie im Klemmenplan zu einer Zeile zusammengefasst werden. Teilklemmen-Querverweise des PDFs bildet man über die Fork-Sprungziele (`destination`) bzw. Master/Slave-Verweise ab, nicht als abgeschriebenen Text. Abgezweigte Schienenpunkte (Etagenpunkte) ordnet man der nummerierten Klemme zu.

## 7. Brücken

- Brückentypen (Funktionsdefinition der Verbindung): Brücke allgemein, **Drahtbrücke** (entfernte Klemmen), **Einlegebrücke** (kammförmig, benachbarte Klemmen, an den Anschlüssen befestigt), **Stegbrücke** (Metallsteg im Klemmenschacht, benachbarte Klemmen), **Schaltbrücke** (schaltbar). Die Verbindungsdefinition hat Vorrang vor der Klemmeneinstellung. Vorschlag: benachbart → Stegbrücke (nicht PE/PEN), nicht benachbart → Drahtbrücke. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwendungbruecken.htm
- „Sattelbrücke“ und „Steckbrücke“ kommen in der Hilfe als Begriffe nicht vor. **(Vermutung)** Steck-/Sattelbrücken des Herstellers entsprechen in EPLAN der Steg- bzw. Einlegebrücke. Quelle (Fehlen der Begriffe): https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwendungbruecken.htm
- Stegbrücken werden für **interne und externe** Stegbrückenanschlüsse getrennt gebildet. Automatische Stegbrücken entstehen nur zwischen direkt benachbarten, verbundenen Klemmen mit gleicher Funktionsdefinition („Klemme, allgemein“ brückt mit allen), bei Mehrstockklemmen nur auf gleicher Etage, mit mindestens einer Stegbrückenmöglichkeit und ohne aufreihbares Zubehör dazwischen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwendungbruecken.htm
- **Manuelle Stegbrücken** gehen auch über nicht benachbarte Klemmen, ohne gezeichnete Verbindung; die Klemmen dazwischen bleiben ungebrückt. Gespeichert an der Anfangsklemme in ID 20350 (intern) / 20351 (extern) in der Notation „Schritt Klemme/Schritt Etage“, z. B. „2/0;2/0“ = X1:1–X1:3–X1:5. Die Brücken sind geräteorientiert (Mehrstockklemme zählt als ein Gerät). Ohne gefundenes Ende gilt die Brücke als defekt. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwendungbruecken.htm
- **Schaltbrücke**: verbindet schaltbar mit der *nachfolgenden* Klemme. Nur „Geschlossen“ wirkt auf Logik/Potenziale. Getrennt für intern/extern (Eigenschaft Schaltbrücke (extern)/(intern)). **Trennklemmen**: Eigenschaften „Schaltsperre“ (ID 20293) und „Klemme geöffnet“ (ID 20232). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwendungbruecken.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_brueckenarbeiten.htm
- Darstellung im Dialog: automatische Stegbrücken rot, manuelle schwarz. Stegbrücken-Anschlusspunkte als Kreis, Direktverbindungen als Rechteck (auch in Auswertungen). Über einen Verbindungsdefinitionspunkt lässt sich der Typ umstellen (z. B. Drahtbrücke → Einlegebrücke). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwendungbruecken.htm
- **Brückenoptimierung** (Projekteinstellung) erzeugt möglichst Steg- statt Drahtbrücken, unabhängig von der gezeichneten Paarung; gezeichnete Verbindungen zeigen nur, *welche* Teilklemmen verbunden sind. Stegbrücken laufen an der ersten Teilklemme vom letzten zum vorletzten Anschluss. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_brueckenbildung.htm

**Bedeutung für QET:** Der Fork kennt `TerminalStripBridge` mit Farbe, `automatic()` (aus der Zeichnung übernommen, schreibgeschützt) und `track()` = Nut 1–4 („groove“): Zwei Brücken über dieselben Klemmen liegen nebeneinander in verschiedenen Nuten. EPLAN-Brückengruppen (z. B. 4-7-10 und 5-8-11) brauchen also je eine eigene Spur, sonst wirken sie wie eine Brücke. Interne und externe Stegbrücke sind in EPLAN getrennt. Im QET-Klemmenplan werden sie auf der richtigen Seite gezeichnet (intern rechts, extern links wie in EPLAN-Spalte B). Drahtbrücken sind normale Leiter zwischen Klemmen derselben Leiste. Im Klemmenplan erscheinen sie als Ziel der jeweils anderen Klemme. Quelle Fork: `sources/TerminalStrip/terminalstripbridge.h`, `sources/TerminalStrip/terminalstrip.h`.

## 8. Sortierung

- Reihenfolge auf der Leiste: 1. **Geräteposition** (ID 20367; alle Klemmen eines Geräts gleich), 2. **Sortierkennung** (ID 20809; nur innerhalb gleicher Geräteposition), 3. Klemmenbezeichnung als Rückfall. Klemmengeräte (Mehrstockklemmen) bleiben beim Sortieren erhalten. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmensort.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwaltung.htm
- Sortierarten der Schaltfläche [Sortieren]: **Sortierung löschen** (entfernt Geräteposition/Sortierkennung, löst Geräte auf, sortiert nach Bezeichnung; Klemmen mit „Sortierung (grafisch)“ folgen der Grafik), **Numerisch** (nur mit Ziffer beginnende, Rest bleibt stehen), **Alphanumerisch** (erst Zahlenwert, dann Buchstaben), **Seitenorientiert** (grafische Reihenfolge nach Normblattlogik, nicht platzierte vorn), **Nach externen Kabeln** (gleiches externes Kabel nebeneinander), **Nach Brücken** (verbundene hintereinander, Stegbrücken neu). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- Beispiele der Hilfe: numerisch 2,3,4,A10,PE,1,10A,10,5,PE → 1,2,3,A10,PE,4,5,10A,10,PE; alphanumerisch → 1,2,3,4,5,10,10A,A10,PE,PE. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- Tipp: Erst Sortierung löschen, dann trennen. Sonst werden Klemmen evtl. wieder zusammengefasst. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm

**Bedeutung für QET:** Die Klemmenreihenfolge im EPLAN-Klemmenplan ist die gespeicherte Leistenreihenfolge, **nicht** zwingend numerisch. Deshalb übernimmt man die Reihenfolge aus dem Original-Klemmenplan (PE zwischen Nummern, „10A“ vor „10“ bei numerischer Sortierung usw.), statt sie in QET neu zu sortieren. Im Fork-Klemmleisten-Editor gilt die Reihenfolge der `PhysicalTerminal`s in der Leiste.

## 9. Nummerierung

- Klemmen werden über die eigene **Klemmennummerierung** bezeichnet (Projektdaten > Klemmenleisten > Klemmen nummerieren); die BMK-Nummerierung betrifft nur das Leisten-BMK. Beim Kopieren nummeriert EPLAN gemäß Online-Nummerierung. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_arbeitsweise.htm
- Dialog „Klemmen nummerieren“: Schema; Reihenfolge **Wie Sortierung / Seitenorientiert / Kabelorientiert / Etagenorientiert (nur Klemmen) / Seitenorientiert (alternativ)**; Umfang (markierte / ausgewählte Leisten / alle); Startwert, Schrittweite, Präfix/Suffix. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalplugnumberinggui_d_klemmennummerieren.htm
- **PE/PEN, N, SH getrennt** einstellbar: Nicht verändern / Nummerieren / Nicht verändern, Reihenfolge berücksichtigen (Beispiel: 1, 4, PE, 8, 2 → 1, 2, PE, 4, 5). Klemmen mit „Gleiche Bezeichnung erlaubt“: nicht verändern / gleich / einzeln nummerieren. Alphabetische Klemmen: nicht verändern / Buchstabenanteil beibehalten / überschreiben. Weitere Optionen: „Änderung der Sortierung zulassen“, „Nur gekennzeichnete nummerieren“ (Bezeichnung beginnt mit „?“ oder leer) mit „Lücken füllen“, „Durch SPS beeinflusste mit nummerieren“. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalplugnumberinggui_d_klemmennummerieren.htm
- Nummerierungsschema: Standardformat „Nach Online-Nummerierungsformat“ oder „Nach Gerätedefinition“ (Bezeichnungen aus den Funktionsschablonen) oder eine freie Bezeichnungstabelle (z. B. A001–A100 per Zwischenablage). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalplugnumberinggui_d_nummerierungsschema.htm

**Bedeutung für QET:** Beim Nachbau keine eigene Nummerierung laufen lassen. Die Bezeichnungen wörtlich aus dem PDF übernehmen, auch PE-Klemmen mit gleicher Bezeichnung und Lücken. Eine Klemme „?“ oder ohne Nummer im Original ist ein Hinweis auf eine Nummerierungslücke, nicht auf einen Lesefehler.

## 10. Innen/Außen (intern/extern) und Verbindungen der Klemme

- Standard: Bauteile **unterhalb** der Klemme im Strompfad = **externe** Ziele, **oberhalb** = **interne** Ziele. Ansicht > Externe Ziele zeigt sie mit Pfeilen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_auswertungklemmen.htm
- Registerkarte Verbindungen (je Anschluss mit Anschlussart „Ader/Draht“ eine Unterkarte): Spalten Ziel, **Anschluss (Intern/Extern)**, **Kabelanschluss** (Verbindung gilt als Kabelverbindung, wird bei automatischer Kabelerzeugung berücksichtigt), **Form. pos.** (als wievieltes Ziel A/B/C/D in Auswertungen). Dazu Anzahl Ziele und Potenzialtyp (-, +, L, M, N, Undefiniert, PE, PEN, SH). Kontextmenü „Klemme: Intern/extern tauschen“ im Leistendialog tauscht Anschluss und Form. pos. des 1. und 2. Symbolanschlusses. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_verbindungenklemmen.htm
- „Anschluss: Intern/extern tauschen“ wirkt auf einzelne Anschlüsse, „Klemme: Intern/extern tauschen“ auf alle Ziele der markierten Klemmen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm

**Bedeutung für QET:** Regel für den Klemmenplan-Generator: Der obere Anschluss des Klemmensymbols ist intern (Schrankseite), der untere extern (Feld/Kabel), solange die Klemme im üblichen senkrechten Strompfad steht. Bei gedrehten Klemmen oder getauschter Seite gilt der Original-Klemmenplan. An einer Mehrstock-Etage steht das Feldgerät extern, intern oft nur die Brücke. Kabel hängen fast immer an der externen Seite (EPLAN-Sortierung „Nach externen Kabeln“ setzt das voraus).

## 11. Dialog „Klemmenleiste bearbeiten“ und Klemmenleisten-Navigator

- Drei Bearbeitungsorte: Eigenschaftendialog, **Klemmenleisten-Navigator** (alle Leisten/Klemmen, geräteorientierte Ansicht zeigt Klemmengeräte), **Klemmenleiste bearbeiten** (alle Klemmen einer Leiste mit Zielen und Brücken). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_arbeitsweise.htm
- Spalten (schemaabhängig): Status, Zeile (Klemmengerät = Block, nur gemeinsam verschiebbar), **Ziel (extern/intern)**, **Brücken (extern/intern)** (Drahtbrücken), **Stegbrücken (extern/intern)**, **Kabel (extern/intern)** (Kabel-BMK plus Aderfarbe/-nummer), **Platzierung (extern/intern)**, Hauptklemme, Etage, Klemmenbezeichnung, Vorschau (extern links, intern rechts, Etagen eingerückt, Potenzialfarben), Anschlussbezeichnungen, Anschlussquerschnitt, Funktionsdefinition, Potenzialtyp, Klemme mit LED, Klemme geöffnet, Schaltbrücke, Schaltsperre, **SPS-Adresse**, Artikelnummer [1]–[10]. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- Kontextmenü: Neue Funktionen, Funktionsschablonen erzeugen/löschen, aufreihbares Zubehör hinzufügen/sichtbar/löschen bzw. davor/dahinter, Ansicht: Anschlussorientiert, Klemmen nummerieren, Verschieben (Lücken), Intern/extern tauschen, Geräteauswahl. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm

**Bedeutung für QET:** Der Fork-Klemmleisten-Editor (`TerminalStrip/ui/terminalstripeditor*`, `terminalstripmodel`) hat Spalten für Level, Kennzeichen, Xref, Kabel, Ader, Schirm, Farbe, Querschnitt, Breite und Katalogeintrag (`terminal_cable`, `terminal_cable_wire`, `terminal_shield`, `terminal_color`, `terminal_section`, `terminal_width`, `terminal_catalog_entry`, `terminal_separator`). EPLAN-Spalten „Kabel (extern)“ ergeben sich dort aus der Kabelzuordnung (`automaticCableTarget()`, sonst Handeingabe). Aufreihbares Zubehör (Abschlussplatte, Trennplatte) entspricht `terminal_separator` bzw. einem Ersatzeintrag. Quelle: `sources/TerminalStrip/realterminal.h` (Fork).

## 12. Auswertungen: Klemmenplan, Klemmenanschlussplan, Formulare

- Auswertungstypen mit Klemmenbezug: **Klemmenplan** (Klemmen einer Leiste, je Leiste ein Plan, Aufbau plus Verdrahtung), **Klemmenanschlussplan** (gleiche Daten, andere Formulare, mehrere Ebenen, Reihenfolge = Leistenreihenfolge, zeigt angeschlossene Ziele), **Klemmenaufreihplan** (Liste, je Leiste), **Klemmenleistenübersicht** (Liste aller Leisten, Formular *.f14). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_funktionsbezogen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungstypen.htm
- Funktionsbezogene Auswertungen: Im **Kopf** steht das Betriebsmittel (Leiste), im **Datenbereich** stehen die Funktionen (Klemmen). Erzeugung: Seitensortierung je Auswertungstyp (Einstellungen > Ausgabe in Seiten: Gesamt oder nach Anlage/Einbauort), Ausgabeform „Seite“, manuelle Auswahl der Leisten, optional Schemata für **Filter, Sortierung und Seitenumbruch**, Startseite/Blattnummer je Strukturkennzeichen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_funkbezogauswerterzeugen.htm
- Seitensortierung legt fest, je welche Strukturkennzeichenebene (oder je Wechsel der Eigenschaft „Ausgabegruppe“) ein eigener **Auswertungsblock** entsteht. Seitentyp z. B. „Klemmenplan (A)“. Ist die Startseite belegt, nimmt EPLAN die nächste freie. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_seitensortierung.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsseiten.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsbloecke.htm
- Formular je Leiste: Klemmenplan- und Anschlussplanformular an der Leistendefinition gehen vor den Projekteinstellungen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_klemmenleisten.htm
- Formulareigenschaft **„Anschlussbezogene Ausgabe“**: eine Zeile je Anschlusspaar extern/intern; Klemmen mit > 2 Anschlüssen auf mehrere Zeilen; Anschlüsse ohne Ziel entfallen. Ohne diese Eigenschaft wird zielbezogen ausgegeben (Platzhalter „Klemmenanschluss intern/extern“). Stegbrücken: anschlussbezogen nur an Nicht-Stegbrückenanschlüssen und nur, wenn auf derselben Seite ein Nicht-Stegbrückenanschluss ausgegeben wird; Stegbrücken zwischen intern und extern nie; zielbezogen alle. Direktanschlüsse erscheinen wie normale Anschlüsse (außer Schienenkontakte). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_auswertungklemmen.htm
- Kabelzuordnung im Klemmenplan: Der Platzhalter „Verbindung / Kabel über Anschluss“ gibt Kabel (BMK, Ader) je Anschluss aus. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_auswertungklemmen.htm . Die Spalte „Kabel (extern/intern)“ zeigt Kabel-BMK und Verbindungsfarbe/-nummer. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- Der genaue **Seitenumbruch** (Zeilen je Formular, Umbruch an ganzen Klemmen) ist in den gelesenen Hilfeseiten nicht beschrieben, es gibt nur den Hinweis auf ein Seitenumbruch-Schema. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_funkbezogauswerterzeugen.htm

**Bedeutung für QET:** Klemmenpläne entstehen im Fork als echte Blätter (Generator), damit Sprungziele (Klemme → Gerät) funktionieren. Der Aufbau folgt dem Formular des Originals: Kopf = Leiste (BMK, Funktionstext, Ort), Daten = je Klemme bzw. je Etage eine Zeile, extern links und intern rechts, Kabelspalten (Kabel-BMK als Spaltenkopf, Ader je Zeile), Brückenspalte. Beim Seitenumbruch an derselben Klemme teilen wie das Original (an der ersten Klemme jeder Originalseite). Eine feste Zeilenzahl ist unzuverlässig, weil Mehrstockklemmen nicht über Seiten getrennt werden **(Vermutung, aus dem Original-PDF ableiten)**. Leisten mit „Keine Ausgabe im Klemmenplan“ bekommen kein Blatt. Die Blattbenennung (Seitentyp, Blattnummer im Block) übernimmt man aus dem PDF.

## 13. Stecker

- EPLAN fasst Stecker, Kupplungen und Steckdosen unter **„Stecker“** zusammen. Ein Stecker ist die Kombination mehrerer **Steckerkontakte** (Stifte oder Buchsen). Stecker und Gegenstück können getrennt oder gemeinsam verwaltet werden. Kontakte können Artikel tragen, und die Kontaktreihenfolge ist festlegbar. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_k_start.htm
- **Gemeinsame Verwaltung** (eine Funktion je Kontakt): Anschluss 1 = Buchse, Anschluss 2 = Stift. Im Steckerplan stehen Buchsen und Stecker auf getrennten Auswertungsseiten. **Getrennte Verwaltung** (zwei Funktionen): Stift und Buchse bilden einen Kontakt bei **gleichem BMK und gleicher Steckerkontaktbezeichnung** (XS1:1 ↔ XS1:1). Für Stift- und Buchsenseite ist je eine passende Steckerdefinition nötig. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_k_verwaltungstecker.htm
- Gerätesteckverbindungen: eine Seite hat ein Geräte-BMK (A), die andere ein Stecker-BMK (X). Geschachtelte BMK (-A-X) sind möglich. Unterschiedliche BMK für Stift/Buchse über Symbole mit **Direktanschluss** (Direktverbindung hat Vorrang vor der Zuordnung über die Bezeichnung; mehrere Stecker können in ein gemeinsames Gegenstück stecken). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_k_verwaltungstecker.htm
- **Steckerdefinition**: Typ, Kontaktanzahl, „Querverweis auf Steckergegenstück“ (nicht bei kombinierter Stift/Buchse-Funktion). Die Steckerkontaktbezeichnung ist Pflicht. Fertig konfigurierte Stecker tragen alle Artikel an der Definition. Bei frei konfigurierbaren steht an der Definition nur das Gehäuse, die Kontaktartikel stehen am Kontakt. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_k_arbeitsweise.htm
- Registerkarte Steckerkontakt: Bezeichnung (Doppelpunkt-Regel wie Klemme), Anschlussbezeichnung, **Zuordnung zu Stift-/Buchsenseite** (Automatisch/Stiftseite/Buchsenseite), Funktionstext, Stegbrücke (Keine/Manuell Anfang/Ende/Mitte), Funktionsdefinition. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_steckerkontakt.htm
- Einfügen: wie Klemmen (BMK-Übernahme in Suchrichtung, „-X?“ am Cursor, Umschalt kehrt um). Steckerdefinition über Einfügen > Steckerdefinition. Mit Einfügen > Gerät werden Definition und Kontakte nacheinander platziert (Taste N wechselt die Funktion). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_h_steckerkontaktezeichnen.htm
- Dialog „Stecker bearbeiten“: je Paar zwei grafisch verbundene Zeilen, Spalten je Buchsen- und Stiftseite (Ziel, Funktionstext, Platzierung, Kabel mit Aderfarbe/-nummer, Stegbrücken). Sortierung: löschen / Numerisch / Alphanumerisch / Nach Position / Nach externen Kabeln. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectormanagementgui_d_steckerbearbeiten.htm
- Auswertungen: **Steckerplan** (Kontakte je Stecker, Aufbau und Verdrahtung), **Steckeranschlussplan** (andere Formulare, mehrere Ebenen, Reihenfolge = Kontaktreihenfolge, mit Zielen), **Steckerübersicht** (*.f23, Liste aller Stecker). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_funktionsbezogen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm
- Steckerkontakte werden mit demselben Dialog wie Klemmen nummeriert („Steckerkontakte nummerieren“). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalplugnumberinggui_d_klemmennummerieren.htm

**Bedeutung für QET:** Stecker in QET als Steckerelemente mit Kennzeichen = Stecker-BMK und Pinbeschriftung = Kontaktbezeichnung. Stift und Buchse eines Paares tragen dasselbe Kennzeichen und dieselbe Nummer. Wo EPLAN eine Übersicht (alle Pins) und Teilansichten (benutzte Pins) zeigt, verknüpft man sie wie SPS-Übersicht und Kanal (Master/Slave über das vollständige Kennzeichen). Ob ein EPLAN-Steckerplan Buchsen und Stifte auf getrennten Seiten zeigt, verrät, ob kombiniert verwaltet wurde. In diesem Fall die QET-Steckerplanblätter ebenso trennen. Ein eigener Stecker-„Editor“ wie die Klemmleiste fehlt im Fork **(Vermutung, nicht geprüft)**. Dann bleibt nur, den Steckerplan per Generator aus den Elementen zu erzeugen.

## 14. Neuerungen neuerer Versionen

- **2023:** Neuer Dialog „Kabel bearbeiten“ mit grafischen Spalten wie in „Klemmenleiste bearbeiten“ (siehe kabel.md). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2023/Content/htm/news_p_kabel_kabelbearbeiten_355040.htm
- **2024, „Klemmenleiste bearbeiten“:** „Neue Klemmen (Geräte)“ direkt im Dialog (unter der markierten Klemme), Zubehör und Mehrstockklemmen getrennt abtrennbar, neue Symbolleisten-Schaltflächen für Zubehör und Mehrstock-Trennen, Kontextmenü **„Klemmenleiste prüfen“** mit Spalten „Prüflauf: Status“/„Prüflauf: Meldungstext“, **„Gehe zu (Grafik)“** springt zum Objekt im Grafischen Editor. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2024/Content/htm/news_p_klemmen_klemmenbearbeiten_389642.htm
- **2026:** Die Themenstruktur der Klemmenhilfe ist unverändert. Der TechTipp „Klemmen mit variablem Aufbau“ (Basisklemme plus Zusatzklemmen als Mehrstockklemme) ist neu. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/terminalgui_k_start.htm , https://www.eplan.help/techtipps/de-DE/Klemmen/TechTip-Terminals-with-variable-structure.pdf

## 15. Ergänzung: Dialog „Klemmenleiste bearbeiten“ im Detail

- Welche Spalten der Dialog in welcher Reihenfolge zeigt, steht in einem **Schema** (Dialog „Einstellungen: Spaltenkonfiguration (Klemmen)“, Aufruf über [...] neben „Schema“). Schemata werden **benutzerbezogen** gespeichert und gehen nicht mit dem Projekt mit (Export nötig). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_schemaspaltenkonfiguration.htm
- Weitere Spalten mit fachlicher Wirkung (Ergänzung zu Abschnitt 11):
  - **Status**: platziert oder nicht, überlagert eine Funktionsschablone des Artikels, freie Schablonen übrig. Daran sieht man, ob der Artikel zur Mehrstockklemme passt.
  - **Klemme geöffnet**: trennt die klemmeninterne Verbindung intern↔extern, die Potenzialweiterleitung ist unterbrochen. Zurückschreibbar.
  - **Schaltbrücke (extern/intern)** und **Schaltsperre**: zurückschreibbar.
  - **SPS-Adresse**: aus „Angeschlossene SPS-Adresse (automatisch)“.
  - **Potenzialtyp der Anschlüsse**: „Undefiniert“, wenn die Anschlüsse verschiedene Typen haben.
  - **Beschriftungsart**: unterscheidet Klemmen für verschiedene Etikettengrößen.
  - **Grafikdarstellung in Auswertungen [1]–[10]**: Werte, die beim Erzeugen eines Klemmenplans über eine Zuordnungstabelle am Formular durch Grafiken ersetzt werden.
  - Weiter: Blockeigenschaften [1]–[10], Meldungen, Artikelnummer und Typnummer [1]–[10] (Artikel nur an Hauptklemmen), Anschlussbezeichnungen/-beschreibungen (alle) (änderbar und zurückschreibbar), Platzierung (extern/intern) (Fundort des angeschlossenen Anschlusses, auch bei Teilklemmen).

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- **Stegbrücken-Spalten**: Ein kleiner Punkt zeigt, ob die Klemme überhaupt einen Stegbrückenanschluss hat. Die Spalte „Stegbrücken“ ohne Zusatz zeigt alle Stegbrücken, egal auf welcher Seite. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- **Manuelle Stegbrücke erzeugen (extern/intern)** ist nur aktiv, wenn die markierten Klemmen die passende Stegbrückenmöglichkeit haben. Vorhandene manuelle Stegbrücken in der Markierung werden zu einer durchgehenden Brücke zusammengefasst, zusätzliche Knoten werden eingefügt. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- **Trennen und Geräteposition**:
  - „Sortierung löschen“ löscht die Geräteposition.
  - „Trennen“ gibt jeder Klemme eine eigene Geräteposition.
  - Ist die Geräteposition leer, fasst EPLAN aufeinanderfolgende Klemmen von einer Hauptklemme bis zur nächsten zu einem Gerät zusammen.
  - „Mehrstockklemme(n) erzeugen“ funktioniert nur, wenn „Hauptklemmen nicht verwenden“ an der Leiste aus ist.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- **Ansicht: Anschlussorientiert** blendet Zeilen für alle Anschlüsse ein, auch für unbeschaltete. Das entspricht dem Inhalt anschlussorientierter Auswertungen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- „Klemme: Intern/extern tauschen“: Die Anzeige von Steg- und Brückenanschlüssen aktualisiert sich erst nach [Übernehmen]. Vor einer „Geräteauswahl“ (Artikel für Hauptklemme oder Etagen suchen) erst übernehmen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm

**Bedeutung für QET:** „Klemme geöffnet“ (offene Trennklemme) unterbricht in EPLAN das Potenzial durch die Klemme. Ein Potenzial- oder Durchgangsprüfer im Fork darf an einer als geöffnet gekennzeichneten Trennklemme also nicht weiterlaufen **(Vermutung zur Umsetzung; im Fork ist dafür kein Feld bekannt)**. Grafiken im EPLAN-Klemmenplan (Symbole für LED-, Sicherungs- oder Trennklemmen in einer Spalte) kommen aus „Grafikdarstellung in Auswertungen“. Beim Nachbau gehören sie in die Klemmenplanzeile, nicht in den Stromlaufplan.

## 16. Klemmenleisten-Navigator, Verschieben, Klemmen erzeugen

- Navigator (Projektdaten > Klemmenleisten > Navigator): Baum je Leiste (unter dem BMK die Leistendefinition und die Klemmen) und Liste. Zwei Ansichten: **BMK-orientiert** oder **geräteorientiert**. Geräteorientiert fasst Klemmen gleicher Geräteposition zu einem Klemmengerät zusammen (z. B. Mehrstockklemme). Dafür darf das Gerät nur **eine** Hauptklemme haben. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_d_klemmennavigator.htm
- Kontextmenü „**Neue Klemmen (Geräte)**“: erzeugt mehrere Klemmen über einen Artikel. Je Klemmengerät wird die zuerst erzeugte Klemme Hauptklemme und bekommt den Artikel. Die Funktionsdefinitionen kommen aus den Funktionsschablonen des Artikels. „**Neue Funktionen**“ erzeugt mehrere Funktionen gleicher Funktionsdefinition nach einem Nummerierungsmuster. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_d_klemmennavigator.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmenerstlbearb.htm
- „**Zuweisen**“: überträgt eine (nicht platzierte) Funktion auf ein Schaltzeichen. Die Funktion muss mindestens so viele Anschlüsse haben wie das Schaltzeichen. Löschen im Navigator entfernt auch die Platzierung und kann Verbindungen verändern. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_d_klemmennavigator.htm
- Klemmen lassen sich vorab im Navigator (ohne Grafik) anlegen und später platzieren oder per Drag & Drop auf eine Seite ziehen. Eine neue Klemme ist Hauptklemme. Für eine Nebenklemme das Kästchen abwählen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmenerstlbearb.htm
- **Verschieben** (Kontextmenü im Leistendialog): verschiebt die markierten **und alle folgenden Klemmen bis zum Leistenende** um n Positionen. Dabei ändern sich **Klemmenbezeichnung und Sortierkennung**. So entstehen Lücken oder schließen sich wieder. Negativer Wert = nach oben/hinten, positiver = nach unten/vorne. Mehrstockklemmen bleiben erhalten, maßgeblich ist die unterste Etage. Wer die Etagen einzeln verschieben will, trennt die Mehrstockklemme vorher. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalplugcommongui_d_verschieben.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmenverschieb.htm
- **Klemmenleistenaufbau über Gerätedefinition** (nur aus Kompatibilitätsgründen; nur ohne Hauptklemmen bzw. mit „Hauptklemmen nicht verwenden“):
  - Funktionsschablonen an der Leistendefinition legen den Aufbau fest. Löschen von Klemmen ändert ihn nicht: Eine gelöschte Klemme bleibt in der Leiste und kann wieder platziert werden.
  - Jede Schablone wird über Klemmenbezeichnung, Funktionsdefinition und Etage identifiziert. Eine leere Bezeichnung passt zu jeder Klemme.
  - Weitergabe durch Kopieren der Leistendefinition oder per Makro.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_funktionsschabloneerzeugen.htm

**Bedeutung für QET:** Klemmen ohne Grafik (nur im Navigator angelegt) erscheinen im EPLAN-Klemmenplan, aber auf keinem Stromlaufplanblatt. Beim Nachbau sind sie Zeilen ohne Sprungziel (Spalte „Platzierung“ leer). Im Fork lassen sie sich nur als Klemme im Klemmleisten-Editor ohne Bauteil oder als Bauteil auf einem Hilfsblatt führen **(Vermutung; prüfen, ob der Fork-Editor Klemmen ohne Element erlaubt)**. Lücken in der Nummerierung (z. B. 1–5, 8–12) stammen oft aus „Verschieben“ und sind gewollt.

## 17. Zubehör (aufreihbar / nicht aufreihbar)

- **Nicht aufreihbares Zubehör** (Klemmenschild, Leistenschild, Abdeckung, Querverbinder, Schaltbrücke, Schaltsperre, Steckaufsatz, Trennwand, Testzubehör …) hängt als Artikel an der Klemme oder an der Leistendefinition. Es erscheint **nicht** im Klemmenplan, nur in der Spalte „Vorschau Zubehör“. Kennzeichen: In der Artikelverwaltung ist „Aufreihbar“ aus. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_klemmenzubehoer.htm
- **Aufreihbares Zubehör** (Tragschiene, Endwinkel, Abschluss(platte), Trennwand …) sitzt auf der Leiste vor oder hinter einer Klemme. Der Artikel steht an der Hauptklemme, ohne Hauptklemmen an der Leistendefinition. „Aufreihbar“ ist für Abschluss, Endwinkel und Trennwand voreingestellt. Klemmen sind immer aufreihbar. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_klemmenzubehoer.htm
- In klemmenbezogenen Auswertungen erzeugt die Formulareigenschaft **„Aufreihbares Zubehör anzeigen“** eigene Zeilen für das Zubehör. Es steht dann nicht mehr in der Klemmenzeile. Ausnahme: Die Platzhalter „Klemmen-Artikelreferenz nach Produktuntergruppen“ zeigen es immer in der Klemmenzeile. Je Klemme oder Leistendefinition gibt es höchstens **50 Artikelreferenzen** (1 Hauptartikel + 49 Zubehör). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_klemmenzubehoer.htm
- Im Leistendialog:
  - „Aufreihbares Zubehör sichtbar“ zeigt das Zubehör als schraffierte Zeile.
  - „Hinzufügen“ fügt es unter dem Cursor ein. „Nachfolgend“ entscheidet, ob es vor oder hinter der Hauptklemme liegt (Pfeil in Spalte „Zeile“).
  - Nach [Übernehmen] bilden Klemme und Zubehör einen Block. Die Reihenfolge wird als Sortierkennung in der Artikeleigenschaft **„Einsortierung“** abgelegt.
  - Löschen: den Block erst trennen, dann die Zubehörzeile löschen.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_leistenzubehoererstlloesch.htm
- Aufreihbares Zubehör zwischen zwei Klemmen verhindert die automatische Stegbrücke (siehe Abschnitt 7). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwendungbruecken.htm

**Bedeutung für QET:** Zeilen wie „Endplatte“, „Trennplatte“ oder „Endhalter“ im EPLAN-Klemmenplan sind aufreihbares Zubehör mit eigener Zeile: keine Klemme, keine Nummer, keine Ziele. Im Fork entspricht das am ehesten `terminal_separator` bzw. einem Trenn- oder Platzhaltereintrag zwischen den `PhysicalTerminal`s **(Vermutung zur Zuordnung)**. Eine Trennplatte zwischen zwei Klemmen erklärt, warum dort keine Steckbrücke durchläuft.

## 18. Teilklemmen: Einstellungen, Einfügen, Optimieren

- Projekteinstellung „Klemmen (Teilklemmen)“:
  - Standard-Funktionsdefinition für Teilklemmen.
  - **„Platzieren ab Anschlusszahl“** (1, 2, 3, 4, 6, 8, niemals): Ab so vielen *bezeichneten* Anschlüssen wird eine Klemme beim Einfügen als Gerät oder aus dem Navigator als Teilklemmen platziert, sonst als Ganzes.
  - „Zusammengehörige Teilklemmen automatisch verbinden“: sollte an sein, sonst keine Potenzialweiterleitung. Ist die Option aus, meldet ein Prüflauf unverbundene zusammengehörige Teilklemmen.
  - „Brückenoptimierung“.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_d_projekteinstellungenklemmen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_teilklemmenzeichnen.htm
- Beispiel der Hilfe zur Brückenoptimierung: Drei Teilklemmen 1, 2, 3 sind mehrfach grafisch verbunden. Ergebnis: zwei Stegbrücken (1–2, 2–3) und fünf interne Brücken (1:a–1:b, 2:a–2:b, 2:b–2:c, 3:a–3:b, 3:b–3:c), auch ohne gezeichnete Linie zwischen 2:b und 2:c. Die Art der Brücke bestimmt die Anschlussart des letzten Klemmenanschlusses (normalerweise „Stegbrücke“). Ohne Brückenoptimierung entstehen die Brücken aus den gezeichneten Verbindungen. **Netzbasierte Verbindungen** und Brückenoptimierung schließen sich aus. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_d_projekteinstellungenklemmen.htm
- Einfügen > Teilklemme: Mindestens das BMK muss eingetragen sein.
  - Bekanntes BMK → bestehende Leiste. Neues BMK → neue Leiste ohne Definition.
  - Bekannte Klemmenbezeichnung → Teil derselben Klemme.
  - Die Anschlussbezeichnungen der Teilklemmen sind nur sichtbar und druckbar, wenn die Ebene **EPLAN430** (Eigenschaftsplatzierung Klemmen-Anschlussbezeichnungen) sichtbar und druckbar ist (Eigenschaft Anschlussbezeichnung des Symbols, ID 20028).

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_teilklemmenzeichnen.htm
- Einfügen als Gerät: Die erste Teilklemme wird Hauptklemme. Je Anschlussbezeichnung entsteht eine eigene Teilklemme, leere Bezeichnungen werden übersprungen. [N] blättert durch die Funktionen, [Leertaste] wechselt zum nächsten Klemmengerät, [Tab] dreht den Anschluss. Tipp der Hilfe: Symbole verwenden, bei denen die Anschlüsse für interne Verbindungen nicht gezeichnet sind. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_teilklemmenzeichnen.htm
- **Teilklemmen optimieren** (Projektdaten > Klemmenleisten):
  - Vergibt Anschlussbezeichnungen für jeden Anschluss mit mindestens einem Draht (Steg- und interne Brücken zählen nicht).
  - Fasst Teilklemmen zu vollständigen Klemmenfunktionen zusammen und setzt „Hauptklemme“ an der ersten Teilklemme.
  - Artikel je Klemme, PE-, N- und SH-Klemme: Die erste Funktionsschablone überschreibt **alle** Eigenschaften, auch mit Leerwerten. Artikel mit Klemmenbezeichnung in der Schablone werden ignoriert.
  - „Vorhandene Klemmenbezeichnungen beibehalten“: sammelt verbundene Teilklemmen gleicher Bezeichnung, bis die Anschlusszahl des Artikels erreicht ist, und vergibt einen Teilklemmenindex. Ohne diese Option vergibt EPLAN neue Bezeichnungen nach der Online-Nummerierung (Beispiel: 8 Teilklemmen -X1, Funktionsdefinition mit 3 Anschlüssen → Klemmen 1, 2, 3).
  - Weitere Optionen: Anschlussbezeichnungen bzw. Teilklemmenindex beibehalten; „Zusammenlegen von miteinander verbundenen Klemmen zulassen“ (legt verbundene Klemmen nach Brücken nebeneinander; PE/PEN nur mit „Auch PE-Klemmen“).

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_d_teilklemmenoptimieren.htm

**Bedeutung für QET:** Teilklemmen erkennt man im PDF an gleicher Klemmenbezeichnung mit verschiedenen Anschlussbezeichnungen (a, b, c) an verschiedenen Stellen. Im Fork ist das ein Klemmeneintrag mit mehreren Bauteilen gleichen Kennzeichens. Die interne Verbindung a–b–c muss als Potenzial durchgehen, auch ohne gezeichnete Linie, denn EPLAN erzeugt dort interne Brücken. Ein Verknüpfungsskript muss diese Teile daher als ein Potenzial behandeln. Sind im PDF keine Anschlussbezeichnungen sichtbar, war vermutlich EPLAN430 ausgeblendet. Die Bezeichnungen stehen dann nur im Klemmenplan.

## 19. Angeschlossene SPS-Adresse an Klemmen und Steckerkontakten

- Klemmen und Steckerkontakte können im Klemmen- oder Steckerplan die SPS-Adresse des angeschlossenen SPS-Anschlusses zeigen. EPLAN trägt sie in **„Angeschlossene SPS-Adresse (automatisch)“** ein. Eine manuelle Angabe in **„Angeschlossene SPS-Adresse“** hat Vorrang und beendet die Suche. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_ermittlungspsadresse.htm
- Suchreihenfolge: erst die **internen** Ziele, dann die **externen**, jeweils rekursiv weiter über angeschlossene Klemmen und Steckerkontakte. Erst wenn dort nichts gefunden wird, folgen die Ziele über interne und externe Brücken. Ändert sich der SPS-Anschluss, aktualisiert sich der Wert selbst. Tipp der Hilfe: Die interne Seite der Klemme Richtung SPS legen, dann gibt es keine Doppeldeutigkeiten. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_ermittlungspsadresse.htm

**Bedeutung für QET:** Eine SPS-Adresse (z. B. „%I1.3“) an einer Klemme im EPLAN-Klemmenplan stammt aus dieser rekursiven Suche. Ein QET-Klemmenplan-Generator geht entsprechend vor: Vom Klemmenanschluss aus sucht er erst über die interne Seite (Schrankseite) zum SPS-Kanal-Bauteil, auch über weitere Klemmen hinweg, und nimmt die Adresse aus dem Kanal (Formel/Label des SPS-Elements). Brücken kommen erst zuletzt dran, sonst bekommen gebrückte Potenzialklemmen falsche Adressen. Eine manuell gesetzte Adresse im Original gewinnt.

## 20. Ergänzung Stecker: Navigator, Definitionen, Bearbeiten

- Stecker-Navigator (Projektdaten > Stecker > Navigator): Baum je Stecker mit Steckerdefinitionen und Kontakten. Die Liste zeigt u. a. „Zuordnung zu Stecker (Stift-/Buchsenseite)“. „Steckerdefinition erzeugen“ bietet **Stifte und Buchsen / Nur Stifte / Nur Buchsen**. Ein Betriebsmittel hat **höchstens drei** Steckerdefinitionen (Stift, Buchse, Stift-Buchse-Kombination). „Steckerkontakt erzeugen“ bietet Stift und Buchse / Stift / Buchse. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_d_steckernavigator.htm
- Gemeinsame Verwaltung → Untermenü „Stifte und Buchsen“. Getrennte Verwaltung → je Seite „Buchsen“ bzw. „Stifte“. Ein Stecker, der nur über das BMK eines Kontakts entsteht (ohne Definition), ist „nicht vordefiniert“: in Auswertungen sichtbar, aber ohne Eigenschaften. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_h_steckerleisteerstlbearb.htm
- Dialog „Stecker bearbeiten“:
  - Besteht der Stecker aus mehreren Definitionen (getrennte Stift-/Buchsenseite), bearbeitet man nur die **markierte** Definition.
  - Bearbeitbar sind die Felder der Seite, von der aus der Dialog aufgerufen wurde (Stift oder Buchse).
  - Spalten je Buchsen- und Stiftseite: Ziel, Kabel (BMK + Farbe/Nummer), Funktionstext des Ziels, Platzierung (Seitenname + Spalte), Brücken, Kontaktbezeichnung, Potenzialtyp, Funktionsdefinition, Verbindungseigenschaften (Farbe/Nummer bis Verbindungsbezeichnung).

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectormanagementgui_d_steckerbearbeiten.htm
- Reihenfolge der Steckerkontakte: Zeilen markieren (Strg für mehrere) und mit den Pfeilen verschieben, oder [Sortieren]. „Nach Position“ = grafische Reihenfolge nach Normblattlogik, nicht platzierte zuerst. „Sortierung löschen“ löscht nur die Sortierkennung (anders als bei Klemmen keine Geräteposition). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_h_steckerkontaktesortieren.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectormanagementgui_d_steckerbearbeiten.htm

**Bedeutung für QET:** Ein EPLAN-Steckerplan zeigt die gespeicherte Kontaktreihenfolge, und die muss nicht numerisch sein. Deshalb die Reihenfolge aus dem Original übernehmen. Die Spalte „Platzierung“ (Seite und Spalte des Ziels) im Steckerplan wird im QET-Nachbau ein Sprungziel zum Gerät.

## 21. Seitenumbruch im Klemmenplan (Lücke)

**Nachtrag:** Die Umbruchregeln (voller Datenbereich, neues Kopfobjekt/neue Leiste, Schema „Neue Seite bei
geändertem Eigenschaftswert“, keine freie Kabelspalte mehr, Folgeformular, neuer Strukturkennzeichen-Block) stehen
jetzt in [formulare-platzhalter.md](formulare-platzhalter.md) Abschnitt 9, mit Quellen aus den Formulareditor-Seiten.
Offen bleibt nur, ob Mehrstockklemmen über ein Seitenende getrennt werden.

- Die Klemmen-, Stecker- und Kabelseiten der Hilfe nennen keine Regel, wann ein Klemmenplan auf die nächste Seite umbricht. Die Beschriftungsseiten erwähnen „Seitenumbruch“ nur für Exportvorlagen, nicht für Klemmenpläne. Weiterhin gilt: den Umbruch aus dem Original-PDF ablesen (siehe Abschnitt 12). Quelle (Fehlen der Regel): https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_auswertungklemmen.htm

## 22. Ergänzung: Funktionen per Nummerierungsmuster, Mehrstock-Etagen, Stecker-Spalten, Kompatibilitätsoptionen
**Dialog Funktionen erzeugen** (Navigator > BMK markieren > „Neue Funktionen“): erzeugt mehrere Funktionen nach Nummerierungsmuster – nur für Klemmen, Steckerkontakte, Spulen-/Relaiskontakte, Geräte- und SPS-Anschlüsse und Sammelschienen; Funktionen ohne Anschlüsse (z. B. „SPS-Anschluss, intern, allgemein“) gehen nicht.
- Bei Klemmen, Stecker- und Spulenkontakten bestimmt das Muster die **Klemmen-/Kontaktbezeichnung**, bei allen anderen die **Anschlussbezeichnungen**.
- Anzahl neuer Funktionen = Werte des Musters ÷ Anschlüsse je Funktion: Klemme (2 Anschlüsse) „1-10“ → 10 Klemmen 1…10; Leistungsschließer (2 Anschlüsse) „1-6“ → 3 Schließer `1¶2`, `3¶4`, `5¶6`; „1a,1b,2a-8b“ → 16 Klemmen 1a…8b.
- **Mehrstockklemme** mit „Anzahl Etagen“: aus 16 Klemmen werden 8 Zweistock-Klemmen. *Etagenorientiert* = Etage 2 erhält die erste Hälfte fortlaufend (1a[2] … 4b[2]), Etage 1 die zweite (5a[1] … 8b[1]); *klemmenweise* = je Klemme wechseln die Werte zwischen den Etagen (1a[2], 1b[1], 2a[2], 2b[1] …). Die Etage steht in eckigen Klammern hinter der Bezeichnung.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_nummusteranschluesse.htm

**Spaltenkonfiguration „Stecker bearbeiten“**: Schemata für die Spalten sind **benutzerbezogen** gespeichert; mit dem Projekt reisen sie nur, wenn man sie exportiert und separat weitergibt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectormanagementgui_d_schemaspaltenkonfiguration.htm

**Kompatibilitätsoptionen mit Klemmenbezug** (Projekt > Verwaltung > Kompatibilität, Details in einstellungen-oberflaeche.md):
- „Klemmen als Hauptfunktion zulassen“ entspricht dem Verhalten vor 2.6; empfohlen ist nur die Eigenschaft **Hauptklemme**.
- „Registerkarte ‚Verbindungen‘ bei Klemmen und Steckerkontakten verwenden“: Ziele werden abweichend von der Grafik ausgewertet; im Dialog Klemmenleiste bearbeiten tauscht „Anschluss: Intern/extern tauschen“ dann die **Ziele**, nicht die Anschlüsse. Ausgeschaltet entsprechen Auswertungen immer der Anschlusslogik im Schaltplan.
- „Teilklemmen bei Export/Import von Kabelbaumdaten getrennt behandeln“: an = eine Zeile je Teilklemme (Verhalten vor 2.8), aus = eine Zeile je Klemme.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektkompatibilitaet.htm

**Bedeutung für QET**: Beim Import von Mehrstockklemmen die Etagennummer aus dem Suffix `[n]` lesen und nicht aus der Reihenfolge ableiten – je nach Option „etagenorientiert“ liegen fortlaufende Bezeichnungen auf derselben Etage oder wechseln die Etage. Auswertungen im Fork sollten ohne Sonder-Ziele aus der Grafik (Anschlusslogik) ableiten, wie es EPLAN bei ausgeschalteter Kompatibilitätsoption tut.

## 23. Quellen

- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwaltungleisten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwaltung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_arbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_hauptklemmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_mehrstockklemmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_mehrstockklemmenarbeit.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verwendungbruecken.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_brueckenarbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_brueckenbildung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_verteiltedarstellungklemmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmenzeichnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmenleistenerstlbearb.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmensort.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_auswertungklemmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_klemmenleisten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_klemmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_verbindungenklemmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalplugnumberinggui_d_klemmennummerieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalplugnumberinggui_d_nummerierungsschema.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungstypen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_funktionsbezogen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_funkbezogauswerterzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_seitensortierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsseiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsbloecke.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_k_arbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_k_verwaltungstecker.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_h_steckerkontakteerstlbearb.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_h_steckerkontaktezeichnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_steckerkontakt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectormanagementgui_d_steckerbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2023/Content/htm/news_p_kabel_kabelbearbeiten_355040.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2024/Content/htm/news_p_klemmen_klemmenbearbeiten_389642.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/terminalgui_k_start.htm
- https://www.eplan.help/techtipps/de-DE/Klemmen/TechTip-Terminals-with-variable-structure.pdf
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_schemaspaltenkonfiguration.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_d_klemmennavigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmenerstlbearb.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_klemmenverschieb.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalplugcommongui_d_verschieben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_funktionsschabloneerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_klemmenzubehoer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_leistenzubehoererstlloesch.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_d_projekteinstellungenklemmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_h_teilklemmenzeichnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_d_teilklemmenoptimieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalgui_k_ermittlungspsadresse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_d_steckernavigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_h_steckerleisteerstlbearb.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plugsgui_h_steckerkontaktesortieren.htm
- Fork-Quellcode (nur gelesen): `C:\Users\test\Documents\Code Projekte\qelectrotech-eplan\sources\TerminalStrip\` (physicalterminal.h, realterminal.h, terminalstrip.h, terminalstripbridge.h), `sources\properties\elementdata.h`

Zusatz-Dialogseiten (Ergänzung Abschnitt 22):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_nummusteranschluesse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectormanagementgui_d_schemaspaltenkonfiguration.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektkompatibilitaet.htm
