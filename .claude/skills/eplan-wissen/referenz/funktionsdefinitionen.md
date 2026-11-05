# EPLAN-Wissen: Funktionen, Funktionsdefinitionen, Funktionsdefinitionsbibliothek

Zusammenfassung der EPLAN-Plattform-Hilfe 2.9 zu Funktionen, Funktionsdefinitionen (Logik), Anschlusslogik,
Betriebsmittelaufbau sowie zum Austausch eigener Kennbuchstaben- und Anschlussbezeichnungssätze. Zweck: im
QET-Fork beim Import (`sources\import\edz\`, `sources\import\eplandxf\`) und beim Verknüpfen den richtigen
Elementtyp und die richtigen Anschlussbezeichnungen ableiten. „Vermutung“ = eigener Schluss, „Fork-Code“ = aus dem
Quelltext. Ergänzt `makros-artikel.md` (Abschnitt 9) und `artikelverwaltung.md` (Schablonen, Geräteauswahl).

## Inhalt
1. Trennung Logik/Grafik, Funktion, Schaltzeichen
2. Betriebsmittel, Bauteil, Haupt- und Nebenfunktion
3. Funktionsdefinition: Eigenschaften und Bibliotheksaufbau
4. Allgemeine Funktionsdefinitionen und Austauschbarkeit
5. Anschlusslogik (Anschlussart, Potenzial, Zielverfolgung …)
6. Variable Anschlusszahl (n/z), einpolige Definitionen
7. Funktionsdefinition oder Symbol tauschen, Funktionen einfügen, Abbuchen
8. Kennbuchstabensätze
9. Anschlussbezeichnungs- und -beschreibungssätze
10. Austausch (XML-Export/-Import) und Aktualisieren der Bibliothek
11. Zuordnung zu QET-Elementtypen (Empfehlung)
12. Quellen

---

## 1. Trennung Logik/Grafik, Funktion, Schaltzeichen

- EPLAN trennt **logische Daten** (Funktionsdefinitionen) von **grafischen** (Symbolen); das ermöglicht
  Normunabhängigkeit und beide Arbeitsweisen: grafikorientiert (Symbole/Makros/Seiten kopieren – die Symbole der
  Bibliotheken bringen ihre Funktionsdefinition mit) und logikorientiert (Geräte, Navigatoren, Abbuchen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_start.htm
- **Funktion** = Objekt mit zugewiesener Funktionsdefinition, bestehend aus einer Gruppe von Anschlüssen; kleinste
  logische Einheit des Datenmodells. Anschlüsse sind in die Funktion eingebettet, die **Anschlusslogik**
  (Potenzialtrennung, SPS-Zielverfolgung …) wird innerhalb der Funktion verwaltet. Eine Funktion gehört zu genau
  einem Bauteil, ein Bauteil hat eine oder mehrere Funktionen (Hilfsblock 2S+1Ö = 3 Funktionen).
- **Schaltzeichen** = platzierte Funktion mit Symbol; **einem Schaltzeichen ist genau eine Funktion** zugeordnet.
  Im Plan platziert man Funktionen, nicht Bauteile. Nicht platzierte Funktionen (ohne Symbol) gibt es in den
  Navigatoren.
- Beispiel der Hilfe: Schütz aus Spule mit 4 Schließern + Hilfsblock mit 4 Öffnern = 9 Funktionen mit je zwei
  Anschlüssen.
- Daten der Funktionsdefinition sind **Vorbelegung**, an jedem Schaltzeichen überschreibbar (z. B. Klemme nicht
  signalweiterleitend, Wechsler-Zielverfolgung zur geöffneten Seite, Schaltzeichen signaltrennend = Grenze für die
  Verbindungsnummerierung).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_prinzip.htm

**Bedeutung für QET:** Ein QET-Element entspricht einem **Schaltzeichen** (Symbol + eine Funktion). Ein
mehrteiliges EPLAN-Gerät (Schütz mit Kontakten) ist in QET Master + Slaves; die Zahl der Slaves entspricht der Zahl
der platzierten Kontaktfunktionen, nicht der Bauteile.

## 2. Betriebsmittel, Bauteil, Haupt- und Nebenfunktion

- **Betriebsmittel** = logisch zusammenwirkende Einheit mit BMK (M1, K1, X1, XS1, W1); **alle Funktionen mit
  gleichem BMK gehören zum selben Betriebsmittel**. Eine davon ist die **Hauptfunktion**; nur an ihr werden Artikel
  hinterlegt (Ausnahmen: Klemmen – nur an Hauptklemmen –, Steckerkontakte, Sammelschienenanschlüsse,
  Verbindungen). Schablonen der Artikel erscheinen im BMK-Navigator am Betriebsmittel.
- **Bauteil** = nicht weiter zerlegbarer Teil (Motor, Klemme, Stecker mit mehreren Stiften – ohne Buchsen –,
  Hilfsblock, Kabel), normalerweise ein Artikel; nur für Artikelverwaltung und Geräte relevant, nicht für das
  logische Verhalten (Kopieren, Löschen, Abbuchen).
- Beispiel K1 aus der Hilfe: Spule A1;A2 (Hauptfunktion), Leistungsschließer 1;2 3;4 5;6, Schließer 13;14,
  Öffner 21;22 (nicht platziert), Öffner 58;59. Artikel: Leistungsschütz (Schablonen Spule + 3 Leistungsschließer)
  und Hilfsschalterblock (13;14, 21;22, 31;32, 43;44). 31;32 und 43;44 erscheinen als **freie Schablonen**
  (noch platzierbar); 58;59 passt zu keiner Schablone → Prüflauf meldet es.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_betriebsmittel.htm
- Schütz-Beispiel zur Rollenverteilung: Spule = in der Regel Hauptfunktion, Leistungs- und Hilfskontakte =
  Nebenfunktionen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_prinzip.htm

**Bedeutung für QET:**
- QET-Master = EPLAN-Hauptfunktion (trägt Artikel und Kontaktspiegel), QET-Slave = Nebenfunktion mit gleichem BMK.
  Das Verknüpfungsskript sollte nach **gleichem vollständigem BMK** gruppieren und je Gruppe genau einen Master
  erwarten.
- Bei Klemmen trägt nur die **Hauptklemme** den Artikel; bei Steckern die Steckerdefinition – das deckt sich mit
  der Klemmen-/Stecker-Logik in `klemmen.md`.
- Die Prüfregel „Funktion passt zu keiner Schablone“ ist direkt als Plausibilitätsprüfung nutzbar: ein Slave mit
  Anschlussbezeichnungen, die in den EDZ-Funktionsschablonen des Masters fehlen, ist verdächtig (Empfehlung).

## 3. Funktionsdefinition: Eigenschaften und Bibliotheksaufbau

- Die Funktionsdefinition legt das **Standardverhalten** fest, insbesondere die Behandlung in **Auswertungen**
  (Kontakte erscheinen im Kontaktspiegel, Klemmen im Klemmenplan …) und Logik (z. B. signaltrennend) – teils für die
  ganze Funktion, überwiegend je Anschluss.
- Eigenschaften: **Name** (z. B. „Leistungsschließer“, „Spule Wechselstrom“), **Beschreibung**, **Anzahl der
  Anschlüsse** (≥ 1; Sonderfunktionen wie Gerätekasten 0; variabel „n“), je Definition **10
  Anschlussbezeichnungs- und 10 Anschlussbeschreibungsvorschläge**.
- **Bibliothek**: hierarchisch **Gewerk → Bereich → Kategorie → Gruppe → Funktionsdefinition**; wird von EPLAN
  geliefert und kann **nicht selbst erstellt oder erweitert** werden.
  - Gewerk: technischer Zweck (Elektrotechnik, Hydraulik …).
  - Bereich: z. B. „Spulen und Kontakte“, „Motoren“.
  - **Kategorie**: bestimmt das **Systemverhalten** und die verfügbaren Eigenschaften (z. B. SPS-Adresse nur bei
    SPS-Funktionen, Spule: Spannung nur bei Spulen).
  - **Gruppe**: Grundfunktion der Kategorie, z. B. „Schließer (2 Anschlüsse)“.
  - Funktionsdefinition: konkrete Art, z. B. „Leistungsschließer“.
  - Beispiel: Elektrotechnik / Spulen und Kontakte / Schließer / Schließer (2 Anschlüsse) / Leistungsschließer.
- An Artikeln eingetragene Funktionsdefinitionen ermöglichen die Geräteauswahl; EPLAN bietet dann passende Symbole an.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_funktionsdefinitionen.htm
- Auswahldialog: Baum Gewerk/Bereich/Kategorie/Gruppe/Definition; Option **Anschlusslogik der Funktion
  beibehalten** (benutzerdefinierte Logik bleibt beim Wechsel). Registerkarte Attribute (nur Anzeige):
  **Hauptfunktion**, **Sicherheitsfunktion**, **Signaltrennend** (aus bei Klemmen, Steckerkontakten),
  **Netzverbindend**, **Kennbuchstabe**, Beschreibung, Zusatzeigenschaften (Klemme, SPS). Registerkarte
  Anschlussdaten (nur Anzeige): siehe Abschnitt 5.
- Sicherheitsfunktion und Schablonen: eine Sicherheits-Schablone passt zu Sicherheits- und Normalfunktion; eine
  **Sicherheitsfunktion im Projekt passt nicht** auf eine Normal-Schablone → Meldung **007001** („mehr Funktionen
  als vom Artikel bereitgestellt“).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_d_funktionsdefinitionen.htm
- Der Funktionsdefinitions-Auswahldialog ist auch erreichbar aus Funktionsschablonen der Artikelverwaltung, den
  Symboleigenschaften im Symboleditor und der **Funktionsdefinitionszuordnung** einer Symbolbibliothek.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_d_funktionsdefinitionen.htm
- Glossar-Hierarchie und SPS-Beispiele: `makros-artikel.md` Abschnitt 9.

**Bedeutung für QET:** Die **Kategorie** ist der Schlüssel für den QET-Elementtyp (Abschnitt 11). Im EDZ steht am
`<functiontemplate>` `functiondefcategory`, `functiondefgroup`, `functiondefid` (CSV-/Tabellenfelder laut
Artikelverwaltung, `artikelverwaltung.md` Abschnitt 3); der Fork liest bisher nur `functiondefinition` als
Gruppenschlüssel (Fork-Code `edzpart.cpp`). Im `.ema` liest `eplanmacroreader` P20013 (Kennbuchstaben), P20027
(Technische Kenngrößen), P20100 (BMK), A185 (Anschlussbezeichnung) und P20406 (Anschlussbeschreibung) (Fork-Code);
die Funktionsdefinition des Objekts wird nicht ausgewertet (offene Lücke).

## 4. Allgemeine Funktionsdefinitionen und Austauschbarkeit

- Viele Gruppen haben eine **allgemeine** Definition (z. B. „Klemme, allgemein, 2 Anschlüsse“, „Schließer,
  allgemein“) für die Projektierung ohne feststehenden Artikel. **Alle Definitionen einer Gruppe sind
  austauschbar.** Bei der Geräteauswahl passt die allgemeine zu allen genauen Definitionen derselben Gruppe
  (Klemme allgemein ↔ N-Klemme ↔ PE-Klemme). In Funktionsschablonen der Artikel nur **genaue** Definitionen
  verwenden.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_funktionsdefinitionen.htm

**Bedeutung für QET:** Für das Verknüpfen reicht der Gruppenvergleich: ein „Schließer, allgemein“ im Plan passt zu
jeder Schließer-Schablone des Artikels. Beim Nachbau aus PDF ist die genaue Definition meist unbekannt; die Gruppe
ergibt sich aus dem Symbol (Schließer/Öffner/Wechsler/Spule).

## 5. Anschlusslogik

- Dialog **Anschlusslogik** (Symbol-/Funktionsdaten > [Logik]): eine Spalte je Anschluss; Abweichungen von der
  Funktionsdefinition gelb; „Auf Standard zurücksetzen“.
- **Anschlussart**: Undefiniert, Ader/Draht (mit Kabelanschluss = Kabelader), **Brücke** (z. B. Klemmenbrücken,
  Leistungsbrücken; Ader/Draht wird über Verbindungsdefinitionspunkt zur Brücke), **Direktanschluss** (Karte auf
  Baugruppenträger, Sicherung auf Schiene, Stift ↔ Buchse), Lichtwellenleiter, Funkübertragung, Nicht elektrisch
  allg., Funktionaler Anschluss (Software, nicht in Auswertungen), Fluid, **Intern** (im Bauteil),
  Verfahrenstechnik, **Stegbrücke**.
- **Kabelanschluss** (für automatische Kabelerzeugung), Druck-/Steueranschluss (Fluid).
- **Potenzial weiterleiten nach** (Anschlussnummern), **Potenzialtyp** (Undefiniert, L, N, PE, PEN, +, M, -, SH),
  **Verbraucher/Quelle nach**, **Signalart** (Sensor, Aktor, Anschlussversorgung, Geräteversorgung, Quelle,
  Digital-/Analogein-/-ausgang, Bus-Quelle/-Eingang/-Ausgang, Druck-/Sauganschluss …), **Zielverfolgung (SPS)
  nach** (bestimmt Reihenfolge der automatischen symbolischen Adresse), **Anzahl Ziele**, **Anzahl Stegbrücken**,
  **Intern/extern** (wirkt auf Geräteanschlussplan), Eigensicherheit möglich, **Gleiche Anschlussbezeichnung
  erlaubt** (sonst innerhalb eines Betriebsmittels eindeutig; bei mehreren Darstellungsarten nicht aktivieren),
  **Symbolanschluss** (Zuordnung Funktions- zu Symbolanschluss, Standard 1→1).
- Weiterleitung an **andere Funktionen mit gleichem BMK** (Geräteanschlüsse, SPS-Anschlüsse, Teilklemmen – bei
  Teilklemmen nur innerhalb derselben Etage) per Anschlussverweis: Ziffern in Anführungszeichen `"4"`, Buchstaben
  ohne `B`, anderer Stecker `"X2:2"`, Ziel ohne Steckerbezeichnung `":2"`; bei mehrpoligen Anschlüssen zuerst
  interne Anschlussnummer: `2,"4"`. Prüflauf **007027** findet falsche Verweise; bei Klemmen gilt `"X2:2"` als
  einfache Bezeichnung.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_d_anschlusslogik.htm
- Anschlüsse Funktion ↔ Symbol: bei gleicher Zahl automatisch, auch für Schließer- oder Öffnersymbol, das einen
  **Wechsler** darstellt; hat die Funktion mehr Anschlüsse als das Symbol, Zuordnung manuell.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_h_anschlussdatenbearbeiten.htm
- Die Anschlussdaten der Definition (Anschlussart, Kabelanschluss, Potenzial, Signalart, Zielverfolgung, Anzahl
  Ziele/Stegbrücken, intern/extern, Eigensicherheit, gleiche Bezeichnung) sind fest; Änderungen nur am Schaltzeichen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_d_funktionsdefinitionen.htm

**Bedeutung für QET:**
- **Potenzial weiterleiten** und **Zielverfolgung (SPS)** sind der Kern der EPLAN-Potenzial- und Signalverfolgung.
  QET-Klemmen und -Stecker leiten (über Leiter) immer weiter, Kontakte nicht. Für ein Verknüpfungsskript, das
  Potenziale über Geräte hinweg verfolgt, gilt: Klemmen/Stecker = durchleitend, Spulen/Kontakte = trennend
  (Funktionsattribut „signaltrennend“).
- **Intern/extern** steuert, welche Seite eines Geräteanschlusses im Geräteanschlussplan als extern gilt; beim
  Nachbau von Klemmen-/Anschlussplänen beachten.
- Der Potenzialtyp PE/PEN/SH entscheidet auch über die Schablonen-Passung von Kabeladern
  (`artikelverwaltung.md` Abschnitt 13).
- Ein Schließersymbol kann einen Wechsler darstellen: im PDF sichtbar als Kontakt mit nur zwei Anschlüssen, aber
  dreistelliger Bezeichnung im Kontaktspiegel – nicht als Fehler werten (Vermutung aus der Regel).

## 6. Variable Anschlusszahl (n/z), einpolige Definitionen

- Definitionen mit **„n“** Anschlüssen: Anzahl folgt dem **Symbol**; ist ein Anschluss **„z“** definiert, bekommt der
  **letzte** Anschluss dessen Daten, alle anderen die von „n“. Bei nicht platzierten Funktionen Anzahl selbst
  eintragen. Logik (Potenzialweiterleitung, Zielverfolgung) muss dann an der Funktion eingetragen werden. Jede
  Kategorie hat variable Funktionen.
- Es gibt eigene **einpolige** Funktionsdefinitionen (auch variabel); **Klemmen und Stecker haben keine**
  einpoligen, dort werden die normalen verwendet.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_k_anschluss.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_funktionsdefinitionen.htm

**Bedeutung für QET:** Gerätekästen und Geräteanschlüsse mit „n“ Anschlüssen (Black Box, SPS-Kasten) bekommen
ihre Anschlusszahl vom Symbol bzw. Makro – der Fork muss sie also aus den Anschlusspunkten (O17/A185) des Makros
zählen, nicht aus der Definition (passt zum heutigen Vorgehen, Fork-Code `eplanmacroreader`).

## 7. Funktionsdefinition oder Symbol tauschen, Funktionen einfügen, Abbuchen

- **Funktionsdefinition tauschen**: nur Definitionen mit **gleich vielen oder mehr** Anschlüssen als das Symbol
  (Schließersymbol → „Wechsler“ möglich); weniger Anschlüsse → Symbol tauschen (neues Symbol bringt neue Funktion).
  Auch per Suchen und Ersetzen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_h_funktionsdeftauschen.htm
- Beim Tausch bleiben eingegebene Daten (Funktionstext, BMK …), die **Anschlusslogik wird von der neuen Definition
  überschrieben** (außer mit „Anschlusslogik beibehalten“). Nicht mehr verfügbare Eigenschaften bleiben verborgen
  erhalten, bis zum Komprimieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_prinzip.htm
- Symbol tauschen über Symbol-/Funktionsdaten > Nummer/Name.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_h_symboltauschen.htm
- **Funktionen einfügen**: über Symbol (Symbol + Funktion werden zum Schaltzeichen gruppiert), über Makro (mit
  Funktionsdefinition oder **eingelagerter spezieller Logikdefinition**), über ein Gerät (Funktion des Artikels
  wählen, passende Symbole werden angeboten; das ganze Gerät entsteht) oder direkt im Navigator (platziert oder nicht).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_h_funktioneneinfuegen.htm
- Eigenschaften ändern: Vorbelegung überschreibbar, zusätzliche Eigenschaften hinzufügbar; Anschlussbezeichnung
  aus den vordefinierten Sätzen wählen oder frei eingeben; Vorbelegung ist der **erste** Satz.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_h_funktionaendern.htm
- **Abbuchen** (Sichtbares BMK > [...]): ist nur der Name markiert, wird nur das BMK übernommen; ist eine Funktion
  markiert, werden alle schreibbaren Eigenschaften dieser Funktion übernommen (außer Symbol-/Funktionsdaten,
  Anzeige und darstellungsbezogene Einstellungen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_prinzip.htm

**Bedeutung für QET:** Makros können eine **eingelagerte Logikdefinition** statt einer Bibliotheksdefinition
tragen; dann stehen Anschlusseigenschaften im `.ema` selbst (Vermutung: als Anschluss-Attribute neben A185, nicht
geprüft). Das Abbuchen entspricht in QET dem Zuordnen eines Slaves zu einem vorhandenen Master über das BMK.

## 8. Kennbuchstabensätze

- Je Funktionsdefinition ein Kennbuchstabe pro Satz; mitgeliefert für **IEC, IEC 61346, IEC 81346, NFPA, GB/T
  5094** (IEC nicht änderbar); eigene Sätze für Normen, Länder, Kunden; Kennbuchstaben auch mehrstellig (KT, KTP);
  mehrere je Zelle mit `;`, der **erste** ist Vorbelegung beim Einfügen.
- Auswahl im Projekt: Betriebsmittel > Nummerierung (online) > **Aktueller Kennbuchstabensatz** und
  **Kennbuchstaben-Unterklassen**; vorher Stammdaten-Projektabgleich. Beim Erstellen einer Funktion wird der
  Kennbuchstabe ins BMK übernommen (manuell änderbar) und **fest eingetragen, nicht referenziert**.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_k_kennbuchstaben.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_h_eigenekennbuchstaben.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_d_kennbuchstaben.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_d_neuerkennbuchstabensatz.htm

**Bedeutung für QET:** Der Kennbuchstabe im importierten Element (`label` „-K“, aus P20013, Fork-Code) ist nur
Vorbelegung. Pläne nach IEC 81346 nutzen andere Buchstaben als ältere IEC-Pläne (z. B. Schütz -Q statt -K);
beim Nachbau immer das BMK aus dem PDF übernehmen, nie den Import-Vorschlag (siehe `kennzeichnung.md`).

## 9. Anschlussbezeichnungs- und -beschreibungssätze

- Je Funktionsdefinition und Anschluss bis zu **10 Vorschläge** für Anschlussbezeichnung (identifizierend) und
  Anschlussbeschreibung (nicht identifizierend); Zeilen je Anschluss mit Nummer oder „n“/„z“ und Potenzialtyp.
  Dienstprogramme > Stammdaten > Anschlussbezeichnung/-beschreibung.
- Neue Funktion: Anschlussbezeichnung mit „1“ vorbelegt bzw. erster Satz; Auswahl als Liste (Schließer: 13,14 /
  23,24 / 33,34). **Die Satzliste liegt an der Funktionsdefinition, die gewählte Bezeichnung fest an der Funktion**
  (nicht referenziert).
- Die **Logikinformationen** stehen in derselben Datei wie die Funktionsdefinitionen, sind von EPLAN vorgegeben und
  nicht änderbar; für Sonderlogik eigene Symbolmakros bauen.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_k_anschluss.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_h_eigenanschlussbezeichnungen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_d_anschlussbezeichnungen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_k_start.htm

**Bedeutung für QET:** Anschlussbezeichnungen sind an jeder Funktion **fest** gespeichert; im `.ema` steht also die
tatsächliche Bezeichnung (A185) – keine Auflösung über die Bibliothek nötig (passt zum Fork-Code). Die
10er-Vorschlagssätze wären für einen QET-Kontakt-Dialog nützlich (Schließer 13-14/23-24/…; Öffner 21-22/31-32/…).

## 10. Austausch (XML-Export/-Import) und Aktualisieren der Bibliothek

- Eigene Kennbuchstaben- sowie Anschlussbezeichnungs-/-beschreibungssätze werden **funktionsdefinitionsbezogen** in
  **XML** exportiert und beim Import den Definitionen wieder zugeordnet (Sicherung, Austausch). Kennbuchstaben:
  markierte Spalten exportieren; Anschlussdaten: immer alle Sätze.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_h_eigenesaetzeexportieren.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_h_eigenesaetzeimportieren.htm
- Import der Anschlussdaten **überschreibt alle** Daten der gefundenen Definitionen; neu hinzugekommene Definitionen
  haben danach keine Werte.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_d_anschlussbezeichnungen.htm
- **Bibliothek aktualisieren** (neue Version/Update): 1. eigene Sätze exportieren, 2. Funktionsdefinitionsbibliothek
  über Abgleich Systemstammdaten ↔ EPLAN-Originalstammdaten aktualisieren, 3. eigene Sätze importieren
  (überschreibt EPLAN-Vorgaben), 4. eigene Werte an neuen Definitionen nachtragen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_h_fktdefaktualisieren.htm
- Die Artikelverwaltung verwendet eine einstellbare **Funktionsdefinitionsbibliothek für die Schablonenauswahl**.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_einstellungenartverw.htm

**Bedeutung für QET:** Die Funktionsdefinitionsbibliothek selbst ist nicht offen dokumentiert und nicht erweiterbar;
für den Fork bleibt nur, die Definitionen über ihre Namen/IDs aus EDZ und `.ema` zu lesen und auf eine eigene
Zuordnungstabelle (Abschnitt 11) abzubilden.

## 11. Zuordnung zu QET-Elementtypen (Empfehlung, keine Hilfe-Aussage)

| EPLAN-Kategorie / Gruppe (Beispiele) | Rolle in EPLAN | QET-Elementtyp |
|---|---|---|
| Spule (Schütz, Relais), Motor, allgemeines Gerät, Gerätekasten | meist Hauptfunktion, trägt Artikel | `master` (bzw. `simple` ohne Slaves) |
| Schließer, Öffner, Wechsler, Leistungsschließer | Nebenfunktion | `slave` (Kontaktart NO/NC/SW) |
| Klemme (mit Etage/Hauptklemme) | Hauptklemme trägt Artikel | `terminal` |
| Steckerdefinition / Stift / Buchse | Stecker = Bauteil aus Stiften | `simple`/`terminal` je Stift |
| SPS-Kasten, SPS-Anschluss E/A, Multifunktion, Versorgung | Kasten = Hauptfunktion, Anschlüsse = Kanäle | Kasten `master`, Kanal-Element des Moduls |
| Abbruchstelle, Potenzialanschluss | keine Gerätefunktion | `next_report`/`previous_report` bzw. Potenzial-Element |
| Gruppe „Grafik“ | ohne Logik/Auswertung | reines Grafik-Element (`thumbnail`/ohne Anschlüsse) |

Grundlage: Rollenregeln aus Abschnitt 1–3 (Quellen dort), SPS-Kategorien aus `makros-artikel.md` Abschnitt 9 und
`sps.md`. Ausnahmen der Artikelzuordnung (Klemmen, Steckerkontakte) aus
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_betriebsmittel.htm

## 12. Quellen

Funktionen und Funktionsdefinitionen:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_betriebsmittel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_k_funktionsdefinitionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_d_funktionsdefinitionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_d_anschlusslogik.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_h_anschlussdatenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_h_funktionaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_h_funktioneneinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_h_funktionsdeftauschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xfctdefbrowsergui_h_symboltauschen.htm

Datenaustausch der Funktionsdefinitionsbibliothek:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_k_anschluss.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_k_kennbuchstaben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_h_eigenanschlussbezeichnungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_h_eigenekennbuchstaben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_h_eigenesaetzeexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_h_eigenesaetzeimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_h_fktdefaktualisieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_d_anschlussbezeichnungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_d_kennbuchstaben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fctdeflibdataexchangegui_d_neuerkennbuchstabensatz.htm

Bezug Artikelverwaltung:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_einstellungenartverw.htm
