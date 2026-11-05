# EPLAN Auswertungen, Formulare, Seiten und Normblätter

Was die offizielle EPLAN-Hilfe (Plattform 2.9 bis 2026) über Auswertungen (Listen wie Stückliste und
Inhaltsverzeichnis), Formulare mit Platzhaltern, Normblätter (Schriftfeld/Plankopf), Seitentypen und
Seitennummerierung sagt, und was daraus für den Nachbau in QElectroTech (Birgers Fork) folgt. Klemmen-, Kabel-
und Steckerauswertungen nur am Rande (eigene Referenzdateien). Inhalte in eigenen Worten; jede Aussage mit
Quelle; „Vermutung“ = nicht durch die Hilfe belegt. Stand der Recherche: 05.11.2026.

## Inhalt

1. Grundbegriffe
2. Auswertungstypen und Formular-Dateiendungen
3. Stückliste, Artikelsummenstückliste, Betriebsmittelliste
4. Inhaltsverzeichnis und Titel-/Deckblatt
5. Funktionsbezogene Auswertungen (Klemmen, Kabel, Stecker, SPS) – Kurzüberblick
6. Ausgabeformen: Auswertungsseiten und eingebettete Auswertungen
7. Auswertungsblöcke, Seitensortierung, Startseite, Unterseiten
8. Erzeugen, Aktualisieren, Einfrieren („online/offline“)
9. Auswertungsvorlagen und „Projekt auswerten“
10. Formulare: Aufbau, statisch/dynamisch, Bereiche, Seitenumbruch, Folgeseiten
11. Platzhaltertexte und Sondertexte
12. Folgeformulare
13. Einstellungen Anzeige/Ausgabe (Ersetzungszeichen, Einschränkungen)
14. Normblätter (Schriftfeld/Plankopf, Spalten, Zeilen)
15. Seitentypen
16. Seiteneigenschaften, Seitenname, Nummerierung
17. Filter-, Sortier- und Seitenumbruchschemata, manuelle Auswahl
18. Seitensortierung im Detail
19. Abläufe rund um Auswertungen (Ergänzungen)
20. Stückliste-Navigator und Artikel-Einstellungen
21. Bedeutung für den QET-Nachbau (Zusammenfassung)
22. Quellen

Siehe auch `formulare-platzhalter.md` (Formulareditor, Platzhalterelemente je Formulartyp, Seitenumbruchregeln,
Normblatteditor, Strukturkennzeichen-Verwaltung).

---

## 1. Grundbegriffe

- Auswertungen sind Abfragen auf die Projektdaten. EPLAN erzeugt sie automatisch und gibt sie als
  Auswertungsseiten oder in externe Dateien (z. B. Etiketten) aus. Eine Auswertung kann auch als eingebettete
  Auswertung in eine Projektseite gesetzt werden (z. B. Schaltschranklegende).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_start.htm
- Fehlen im Projekt die benötigten Objekte oder filtert ein Filter alles weg, entsteht keine Auswertungsseite.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_start.htm
- Auswertungsseiten zeigen das Ergebnis auf eigenen Seiten. Je Auswertungstyp wird ein Formular zugeordnet,
  dessen Platzhalter beim Auswerten mit Projektdaten gefüllt werden. EPLAN sortiert die Seiten in die
  Seitenstruktur ein; erkennbar am Seitentyp, z. B. „Klemmenplan (A)“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsseiten.htm
- Formulare geben Auswertungsergebnisse grafisch aus (welche Eigenschaften, Reihenfolge, Layout). Normblätter
  legen Schriftkopf, Blattgröße und Spalten-/Zeileneinteilung fest. Beide sind einzelne Dateien und werden im
  Formular- und Normblatteditor bearbeitet (temporäre Seite im Projekt).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_start.htm

**Bedeutung für QET:** Listen sind Daten, keine Zeichnung. Im Nachbau werden sie aus den QET-Projektdaten
(plus fehlende Angaben aus dem Original) generiert, damit Inhalt und Sprünge stimmen
(vgl. Skill `schaltplan-wissen`, Abschnitt 1).

## 2. Auswertungstypen und Formular-Dateiendungen

Auswertungen gliedern sich in funktionsbezogene Auswertungen, Auswertungsübersichten und grafische
Auswertungen (Ablaufdiagramm).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungstypen.htm

Auswertungsübersichten (Auszug, Endung = Formulardatei):

| Typ | Formular |
|---|---|
| Artikelstückliste | *.f01 |
| Artikelsummenstückliste | *.f02 |
| Betriebsmittelliste | *.f03 |
| Formulardokumentation | *.f04 |
| Inhaltsverzeichnis | *.f06 |
| Kabeldarstellungsplan | *.f08 |
| Kabelübersicht | *.f10 |
| Klemmenleistenübersicht | *.f14 |
| Normblattdokumentation | *.f15 |
| Potenzialübersicht | *.f16 |
| Revisionsübersicht | *.f17 |
| SPS-Kartenübersicht | *.f20 |
| Steckerübersicht | *.f23 |
| Strukturkennzeichenübersicht | *.f24 |
| Titel-/Deckblatt | *.f26 |
| Verbindungsliste | *.f27 |
| Hersteller-/Lieferantenliste | *.f31 |
| Montageliste | *.f32 |
| SPS-Adressübersicht | *.f48 |

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm

Funktionsbezogene (Auszug): Geräteanschlussplan *.f05, Kabelanschlussplan *.f07, Kabelplan *.f09,
Klemmenanschlussplan *.f11, Klemmenaufreihplan *.f12, Klemmenplan *.f13, Schaltschranklegende *.f18,
SPS-Diagramm *.f19, Steckeranschlussplan *.f21, Steckerplan *.f22, Symbolübersicht *.f25.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/formgeneratorgui_k_funktionsbezogen.htm

Der Formulartyp legt die Dateiendung fest; Standardablage `…\Formulare\<Firmenkennung>` bzw. Benutzer-
einstellung Verzeichnisse.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formulareerzeugen.htm

**Bedeutung für QET:** Den Typ eines Original-Blatts liest man am Seitentyp im Schriftfeld/Lesezeichen ab
(z. B. „Artikelstückliste“). Danach richtet sich der Generator, nicht nach der Optik.

## 3. Stückliste, Artikelsummenstückliste, Betriebsmittelliste

- **Artikelstückliste:** alle verwendeten Artikel, Artikel am Artikeldefinitionspunkt und Artikelplatzierungen
  ohne Artikel; **jeder Artikel einzeln**. Artikel mit Menge 0 erscheinen auch, sofern kein Filter sie ausschließt.
- **Artikelsummenstückliste:** gleicher Inhalt, aber **gleiche Artikel zusammengefasst und summiert**; Menge 0
  ebenfalls ohne Filter enthalten.
- **Betriebsmittelliste:** alle verwendeten Betriebsmittel; Kabelartikel können mit ausgegeben werden.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm
- Baugruppen auflösen / Vorplanungsartikel betreffen Artikelstückliste, Artikelsummenstückliste und teils
  Betriebsmittelliste; Standard = Einstellungen unter „Artikel“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_vorlagen.htm
- Bei der Artikelsummenstückliste kann „Variable Werte ersetzen durch“ die Werte eines Wertesatzes durch festen
  Text ersetzen (z. B. „Siehe Tabelle“), wenn die Formulareigenschaft dazu aktiv ist.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_einstellungenausgabe.htm
- Hat eine Funktion mehrere Darstellungsarten mit unterschiedlichen Daten, entscheidet eine feste Reihenfolge
  der Darstellungsarten, welche Werte in die Auswertung gehen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswertungen.htm
- Im PDF: Klick mittig auf das BMK einer Stücklistenzeile springt zur Hauptfunktion im Schaltplan.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm

**Bedeutung für QET:**
- Stückliste: eine Zeile je Artikel je Betriebsmittel (nicht summiert), BMK-Text mit `destination` = Bauteil
  (Hauptfunktion). Zubehörartikel eines Betriebsmittels (mehrere Artikel am selben BMK) sind eigene Zeilen.
- Artikelsummenstückliste: Summe je Bestellnummer aus der Stückliste berechnen, nicht abschreiben – dann
  stimmen Mengen nach Änderungen. Sprungziele der Summenzeilen im Original prüfen (laut Skill
  `qet-schaltplan-zeichnen` führt die Artikelsumme zur Stückliste).
- Artikel mit Menge 0 nicht stillschweigend weglassen, wenn das Original sie zeigt.
- Der Fork hat einen BOM-CSV-Export (`sources/bomexport.cpp`, Gruppierung möglich) – nützlich zum Gegenprüfen
  der generierten Stücklistenblätter.

## 4. Inhaltsverzeichnis und Titel-/Deckblatt

- Inhaltsverzeichnis (*.f06) = Inhaltsverzeichnis der Projektdokumentation; Titel-/Deckblatt (*.f26) = Titel-
  bzw. Deckblatt der Dokumentation. Beide sind Auswertungstypen, also automatisch erzeugte Seiten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm
- Das Inhaltsverzeichnis wird **immer zuletzt** ausgegeben (damit es alle anderen Auswertungsseiten enthält).
  Welche Seiten fehlen sollen, steuert man über Filter.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_einstellgrafikausgabe.htm
- Seitensortierung „Gesamt“ = ein Inhaltsverzeichnis für das ganze Projekt; „Anlage“ = eins je Anlage;
  Kombinationen (Anlage + Einbauort) erzeugen bei jedem Wechsel ein neues.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_seitensortierung.htm
- Teilausgabe (Zusatzformular, z. B. anlagenbezogen) gibt es nur für bestimmte Typen wie Inhaltsverzeichnis oder
  Kabelübersicht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_einstellgrafikausgabe.htm
- Per Projektfilter kann man je Projekt eine andere Titelblatt-Vorlage (anderes Formular) greifen lassen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsvorlagen.htm
- Spalteninhalt des Inhaltsverzeichnisses (Seitenname, Beschreibung, Datum …) legt das Formular fest; die Hilfe
  nennt keinen Standardinhalt. Vermutung: übliche Spalten = vollständiger Seitenname, Seitenbeschreibung,
  Zusatzfeld, Datum, Bearbeiter.
  Quelle (Lücke): https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm

**Bedeutung für QET:** Inhaltsverzeichnis aus den QET-Blattdaten generieren (Titel = Seitenbeschreibung,
Folio-Name, Anlage/Ort), **nach** allen anderen Blättern, je Zeile `destination` = Blatt-UUID. Seitensortierung
des Originals erkennen: ein Verzeichnis gesamt oder je Anlage (mehrere Blöcke mit Kopf „=Anlage“). Das Deckblatt
ist inhaltlich eine Auswertung aus Projekteigenschaften – in QET über Schriftfeld-/Projektvariablen füllen statt
Text abzuschreiben.

## 5. Funktionsbezogene Auswertungen – Kurzüberblick

- Funktionsbezogen = ein Betriebsmittel im Kopfbereich (z. B. Klemmenleiste), darunter seine Funktionen; je
  Betriebsmittel ein Block (Klemmenplan je Leiste, Steckerplan je Stecker, SPS-Diagramm je Karte; Kabelplan mit
  Kabelverbindungen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_funktionsbezogen.htm
- Mit „Zusammenfassen“ (nur dynamische Formulare) stehen mehrere Funktionen/Betriebsmittel auf einer Seite;
  „Mind.-Anzahl Auswertungszeilen“ verhindert, dass ein Block mit zu wenigen Zeilen am Seitenende beginnt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_einstellgrafikausgabe.htm
- „Manuelle Auswahl“ erlaubt funktionsbezogene Auswertungen nur für einzeln gewählte Betriebsmittel.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswertfestlegen.htm

Details: eigene Referenzdateien zu Klemmen, Kabeln, Steckern.

## 6. Ausgabeformen: Auswertungsseiten und eingebettete Auswertungen

- Dialog „Auswertung festlegen“: Ausgabeform **Seite** (Auswertungsseiten im Projekt) oder **Manuelle
  Platzierung** (eingebettete Auswertung auf der offenen Seite; nur bei geöffneter Seite im Grafischen Editor).
  „Aktuelle Seite“ wertet nur die Daten der aktuellen Seite aus. Quellprojekt wählbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswertfestlegen.htm
- Eingebettete Auswertungen werden mit aktualisiert; beim Kopieren von Betriebsmitteln/Makros bleibt der Bezug
  erhalten; sie lassen sich nicht einfrieren und werden direkt auf der Seite gelöscht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_ausgabeformen.htm
  und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswertungen.htm

**Bedeutung für QET:** Tabellen mitten auf einem Schaltplanblatt (z. B. Legende, Klemmenübersicht auf dem
Aufbaublatt) sind eingebettete Auswertungen – im Nachbau ebenfalls aus Daten erzeugen (QET-Tabellen/Text mit
`destination`), nicht als Grafik kopieren.

## 7. Auswertungsblöcke, Seitensortierung, Startseite, Unterseiten

- Ein **Auswertungsblock** ist eine Gruppe zusammengehöriger Auswertungsseiten. Ob es einen Block oder mehrere
  gibt, bestimmt die Seitensortierung; je Strukturkennzeichenebene ein Block. Eine Auswertungseinstellung gilt
  für genau einen Block. Ist die vorgeschlagene Startseite belegt, nimmt EPLAN die nächste freie.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsbloecke.htm
- Seitensortierung: projektweit, pro Kennzeichnungsblock, kombiniert, oder nach der Schaltzeichen-Eigenschaft
  „Ausgabegruppe“. Formulardokumentation nur „Gesamt“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_seitensortierung.htm
- Dialog <Auswertungstyp>: Startseite je Block (Strukturkennzeichen), vorgeschlagener Seitenname, „Am Ende
  anhängen“ (nur Vorlagen, hinter letzter Seite der Ebene, Reserveseiten ungenutzt), Zusatzfeld Blattnummer,
  automatische oder manuelle Seitenbeschreibung, „Start für alle Strukturkennzeichen übernehmen“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_struktur.htm
- Einstellungen „Ausgabe in Seiten“ (je Auswertungstyp):
  - Reihenfolge der Typen = Erzeugungsreihenfolge (beeinflusst Seitenfolge bei gleicher Startseite).
  - Formular, Seitensortierung, Teilausgabe, Zusammenfassen, Mind.-Anzahl Auswertungszeilen.
  - **Unterseite:** erste Seite eines Blocks auf Hauptseite, Folgeseiten als Unterseiten (1.1 oder 1.a/1.A);
    einseitige Blöcke ohne Unterseite, außer Variante „(ab Seite 1)“.
  - Freibleibende Seiten (Abstand zur letzten Projektseite), Runden (auf Vielfaches).
  - „Strukturkennzeichen bei Gleichheit ausblenden“: Nein / gegen die Seite / gegen Kopfobjekt / alles gegen
    Seite inkl. Kopfdaten – kürzt BMK auf Auswertungsseiten.
  - Abgleichen mit Stammdatenpool beim Öffnen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_einstellgrafikausgabe.htm

**Bedeutung für QET:**
- Seitennamen der Originalauswertungen genau übernehmen, inkl. Unterseiten (z. B. 101, 101.1 …) – sonst stimmen
  Inhaltsverzeichnis und Blattzählung nicht. Gleiche Blattanzahl wie das Original.
- BMKs auf Auswertungsseiten können gekürzt sein („bei Gleichheit ausblenden“). Beim Zuordnen von Listenzeilen
  zu Bauteilen fehlende =Anlage/+Ort aus dem Kopf bzw. der Seite ergänzen.

## 8. Erzeugen, Aktualisieren, Einfrieren („online/offline“)

- Auswertungen sind **Momentaufnahmen**: Sie ändern sich erst beim Aktualisieren. Manuell: im Seiten-Navigator
  eine Ebene markieren > Auswertungen > Aktualisieren (alle untergeordneten Auswertungsseiten und eingebetteten
  Auswertungen), oder in der Registerkarte Auswertungen. Es wird immer der ganze Block neu erstellt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswaktualisieren.htm
- Automatisch nur beim **Öffnen** einer Seite, wenn „Auswertungen beim Öffnen von Seiten aktualisieren“ (Benutzer
  > Darstellung > Allgemein) aktiv ist; Öffnen dauert dann länger.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_automatischaktualisieren.htm
- **Einfrieren:** Block aus der Verwaltung lösen, Seiten bleiben, werden nie mehr aktualisiert (eigenes Icon im
  Navigator). Neu erzeugen nur nach manuellem Löschen der alten Seiten, sonst entstehen zusätzliche Seiten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswerteinfrieren.htm
- Registerkarte Auswertungen: Baum (Projekt > Typ > Block > Seite, eingebettete gesondert), Kontextmenü Neu,
  Aktualisieren, Block löschen, Einfrieren, Vorlage erzeugen, Seite öffnen; Menü Einstellungen (Ausgabe in
  Seiten, Artikel, Anzeige/Ausgabe).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswertungen.htm
- „Online/Offline“ verwendet die Hilfe für **Prüfläufe**, nicht für Auswertungen: Online-Prüfung läuft während
  der Bearbeitung, Offline-Prüfung auf Befehl für Projekt, Strukturteile oder Seiten. Querverweise und
  Kontaktspiegel aktualisiert EPLAN laufend (Vermutung, nicht als „online“ belegt), Auswertungsseiten dagegen nur
  auf Befehl.
  Quelle: https://www.eplan.help/en-us/infoportal/content/Plattform/2026/Content/htm/msgmanagementgui_h_prueflaufonline.htm

**Bedeutung für QET:** Ein EPLAN-PDF kann veraltete Auswertungen enthalten (nicht aktualisiert oder eingefroren).
Weicht eine Liste vom Schaltplan ab, gilt der Schaltplan als führend (Inhalt vor Optik); Abweichung melden. Im
Nachbau Listen bei jedem Gesamt-Bau neu generieren (entspricht „Projekt auswerten“).

## 9. Auswertungsvorlagen und „Projekt auswerten“

- Vorlagen speichern die Einstellungen einer Auswertung (Formular, Filter, Sortierung, Startseite,
  Seitensortierung, Folgeformular, Teilausgabe …); mehrere Vorlagen je Typ möglich; Export/Import als XML.
  Erneutes Erzeugen aus einer Vorlage ersetzt die vorhandene Auswertung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsvorlagen.htm
- „Auswerten“ einer Vorlage wertet stets alle Vorlagen desselben Typs aus (sonst Überschreiben). Projektfilter
  machen Vorlagen bedingt („Vorlage aktiv“). Eigenschaften: Startseite, Filter-/Sortiereinstellung, „Neue Seite
  bei Änderung der Eigenschaft“, Seitenbeschreibung, Formular, Folgeformular.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_vorlagen.htm
- „Projekt auswerten“: erzeugt aus allen Vorlagen Auswertungen und aktualisiert alle bestehenden; fehlt eine
  Vorlage, bietet EPLAN an, die Auswertung zu löschen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_projektauswerten.htm
- Menüpfad 2.9: Dienstprogramme > Auswertungen > Erzeugen; 2026: Werkzeuge > Auswertungen > Erzeugen.
  Quelle: https://www.eplan.help/en-us/Infoportal/Content/Plattform/2026/Content/htm/formgeneratorgui_h_projektauswerten.htm

**Bedeutung für QET:** Das Gesamt-Skript ist das Gegenstück zu „Projekt auswerten“: jeder Listengenerator
entspricht einer Vorlage (fester Typ, Formular/Layout, Startseite, Sortierung). Diese Parameter je Generator
einmal aus dem Original ableiten und im Skript festhalten.

## 10. Formulare: Aufbau, statisch/dynamisch, Bereiche, Seitenumbruch, Folgeseiten

- Formulare bestehen aus statischen Elementen (Linien, Rechtecke, Bitmaps) und aus Platzhalter-/Sondertexten,
  die beim Auswerten gefüllt werden. Beim Erstellen gilt automatisch die Standardvorlage.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_grundlagen.htm
- **Formularhandhabung** statisch oder dynamisch:
  - statisch: gleiche Grafik auf jeder Seite, konstante Zeilenhöhe;
  - dynamisch: Grafik wird aufgebaut, dynamische Bereiche möglich, „Zeilenhöhe dynamisch anpassen“, Zusammenfassen
    mehrerer Funktionen auf einer Seite; leere Zeilen entfallen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_dynamformulare.htm
  und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_dynamischeformulare.htm
- **Dynamische Bereiche** (Einfügen > Dynamischer Bereich): Kopfbereich (je Betriebsmittel wiederholt; mit
  Zusammenfassen am Seitenanfang und vor jeder weiteren Funktion), Überschrift (je Funktion bzw. Gruppenkopf,
  optional je Spalte, mehrere über „Für die ersten x Sortierungseigenschaften Überschriften erzeugen“),
  Datenbereich (eine Zeile je Datensatz), Datenbereichsabschluss (z. B. Trennlinie vor nächster Funktion),
  Fußbereich (einmal je Seite). Nur die Höhe eines Bereichs zählt; jeder Bereichstyp nur einmal. Grafik und
  Platzhalter außerhalb der Bereiche erscheinen einmal je Seite mit den Daten des ersten Kopfobjekts der Seite.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_dynamformulare.htm
  und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_grafikendynamischplatzieren.htm
- **Seitenumbruch:** Zeilenanzahl × Zeilenhöhe = maximale Höhe des Auswertungsbereichs (gemessen ab Ende des
  Kopfbereichs, unsichtbare Grenze), z. B. 20 × 10 mm -> Umbruch nach 200 mm. Danach geht es in der nächsten
  Spalte bzw. auf der **Folgeseite** weiter; Kopf-/Fußbereich wiederholen sich dort.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_dynamformulare.htm
- **Spalten und Richtung:** Spaltenanzahl je Seite; Formularausrichtung zeilen- oder spaltenweise; Vorzeichen
  von Spaltenbreite/Zeilenhöhe bestimmen links/rechts bzw. oben/unten. Ohne „Kopfbereich bei neuer Spalte
  wiederholen“ passen in Folgespalten mehr Zeilen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_auswertungsrichtung.htm
- Bedingte Bereiche und Überschriften können bei Änderung bestimmter Eigenschaften neu ausgelöst werden (z. B.
  dicke Linie oder neue Überschrift bei jedem neuen BMK in der Betriebsmittelliste).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_bedingungen.htm
- Formulare lassen sich prüfen (Dienstprogramme > Formular prüfen), z. B. auf richtige Bereichsnutzung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_grafikendynamischplatzieren.htm

**Bedeutung für QET:**
- Listengenerator = dynamisches Formular nachbauen: feste Rahmengrafik + Kopf je Gruppe + n Datenzeilen je Seite
  (aus Original: Zeilenzahl, Zeilenhöhe, Spaltenzahl messen) + Umbruch auf Folgeblatt mit wiederholtem Kopf.
- Gruppenwechsel (neue Überschrift/neue Seite bei Änderung einer Eigenschaft, z. B. Anlage oder BMK) so setzen
  wie im Original erkennbar.
- Wiederholte gleiche Texte untereinander können durch ein Ersetzungszeichen ersetzt sein (Abschnitt 13) – beim
  Auslesen des Originals den Wert der Zeile darüber übernehmen.

## 11. Platzhaltertexte und Sondertexte

- **Platzhaltertexte** legen Position und Format einer Eigenschaft im Formular fest; beim Auswerten ersetzt EPLAN
  sie durch die Werte der Objekte (Bauteile, Seiten, Symbole). Einfügen: Einfügen > Platzhaltertext, Eigenschaft
  wählen (Liste hängt vom Formulartyp ab), Format, platzieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_platzhaltertexteeinfuegen.htm
- Arten: formulartypabhängige Platzhalter (z. B. Brücken im Klemmenplan, Abschirmungen und Kabeldefinitionslinien
  im Klemmenanschlussplan), Sondertexte (Projekt- und Seiteneigenschaften), Zeilen-/Spaltentexte für Normblätter.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_platzhalter.htm
- Zeilenumbrüche im Wert werden herausgefiltert, wenn der Platzhalter keinen Positionsrahmen hat oder „Umbrüche
  entfernen“ aktiv ist.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_platzhalter.htm
- **Sondertexte** (Einfügen > Sondertext > Seiten-/Projekteigenschaften; Formular-/Normblatteigenschaften;
  Spalten-/Zeilentext nur in Normblättern) werden durch den jeweiligen Eigenschaftswert ersetzt. Konkrete
  Beispiele (Seitenzahl, Datum …) nennt die Hilfe dort nicht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_sondertexteeinfuegen.htm
- Hyperlinks in Eigenschaftswerten werden auch über Platzhalter auf Auswertungsseiten als Link erkannt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_grafeditor_hyperlinkserkennung_248626.htm

**Bedeutung für QET:** Gegenstück sind QET-Schriftfeldvariablen (`%{…}`, z. B. `folio-id`, `folio-total`
aus `sources/bordertitleblock.cpp`) und Bauteilinformationen. Werte, die in EPLAN aus Eigenschaften kommen, in
QET als Projekt-/Folio-/Bauteildaten pflegen, nicht als festen Text.

## 12. Folgeformulare

- Für bestimmte Typen (z. B. Betriebsmittelliste, Kabeldarstellungsplan, Klemmenleistenübersicht) kann ein
  Folgeformular direkt nach dem Hauptformular ausgegeben werden – über einen Folgeformular-Einfügepunkt auf
  derselben Seite oder einer Folgeseite, mit eigenem Typ, Filter und Sortierung. Mehrere Einfügepunkte möglich,
  keine Verschachtelung. Alle Seiten tragen den Seitentyp des Hauptformulars; die Seiten eines Formularnamens
  bilden einen Auswertungsblock. Häufig als Folgeformular: Artikelstückliste/Artikelsummenstückliste.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_folgeformulare.htm

**Bedeutung für QET:** Steht im Original z. B. unter einem Klemmenplan eine Stückliste der Klemmen, ist das ein
Folgeformular – Daten gehören zur selben Leiste; Seitentyp bleibt der des Hauptplans.

## 13. Einstellungen Anzeige/Ausgabe

- „Identische Texte ersetzen durch“: wiederholte, untereinander stehende Texte werden durch ein Zeichen (z. B. `"`)
  ersetzt – nur bei Platzhaltern mit „Identischen Text ersetzen“. Korrektur/Ergänzung: Die Vorgabe ist laut Hilfe „=“,
  nicht für Inhaltsverzeichnisse (Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_platzierung.htm).
- Ausgabe von Kabeln/Klemmenleisten/Steckern einschränken (ab Mindestanzahl Verbindungen/Klemmen/Stifte).
- Quelle/Ziel von Kabelverbindungen aus Kabel oder Kabelverbindungen; Ersetzungszeichen für Einzel-/
  Kabelverbindungen in Kabeltabellen.
- Startseiten-Offset für Ausgabegruppen; Kennzeichnungsblock mit Ausgabegruppe füllen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_einstellungenausgabe.htm

**Bedeutung für QET:** Beim Auslesen von Originallisten `"` bzw. andere Wiederholzeichen als „wie oben“ auflösen.
Fehlt eine kleine Klemmenleiste/ein Kabel in der Liste, kann eine Mindestanzahl-Einschränkung die Ursache sein –
kein Fehler des Schaltplans.

## 14. Normblätter (Schriftfeld/Plankopf, Spalten, Zeilen)

- Ein Normblatt legt für eine Seite Schriftkopf, Blattgröße und die Einteilung in Zeilen und Spalten fest; dazu
  gehören Sondertexte für die Zeilen-/Spaltenbeschriftung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_normblaetter.htm
- Spalten-/Zeilentexte lassen sich automatisch in den Randbereichen neu platzieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_sondertexteeinfuegen.htm
- Normblatt ist in den Projekteinstellungen und je Seite setzbar; die Seite überschreibt das Projekt. Normblatt
  und Formular werden ins Projekt eingelagert. Papierformat und Ausrichtung ergeben sich aus dem Normblatt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_eigenschaften.htm
- Bearbeitete Normblätter/Formulare werden nicht automatisch ins Projekt übernommen (Stammdatenabgleich nötig).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_start.htm
- Raster der Seite kommt immer aus dem Normblatt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_h_seitenanlegen.htm

**Bedeutung für QET:** Normblatt = QET-Rahmen (Spalten/Zeilen in den Folio-Eigenschaften) + `.titleblock`. Die
Spaltenzahl und Spaltenbreite aus dem Original übernehmen; EPLAN zählt Spalten ab 0, QET standardmäßig ab 1
(Skill `qet-schaltplan-zeichnen`). Felder des Plankopfs (Seitenbeschreibung, Anlage, Ort, Blatt, Gesamtblatt,
Datum, Bearbeiter, Revision) als Variablen anlegen.

## 15. Seitentypen

- Jede Seite hat einen Seitentyp, der festlegt, wie sie ausgewertet wird. **Interaktive** Seiten (I) bearbeitet
  der Anwender (z. B. „Schaltplan allpolig (I)“), **automatische** (A) erzeugt EPLAN als Auswertung (z. B.
  „Artikelstückliste (A)“, Klemmen- und Kabelpläne).
- **Logikseiten** werten Betriebsmitteldaten aus und erzeugen Autoconnect-Linien; **Grafikseiten** nicht. Kein
  Autoconnect auf „Schaltschrankaufbau“, „Übersicht“ und „Vorplanung“.
- Seitentyp nachträglich änderbar (übrige Eigenschaften bleiben), außer „Fremddokument“.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_seitentypen.htm
und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pagebrowsergui_k_seitentypen.htm
- „Schaltplan einpolig“ ist ebenfalls interaktiv und für einpolige Darstellung/übergeordnete Funktionsübersichten
  gedacht; „allpolig“ hat die vollen Schaltplan-Bearbeitungsfunktionen.
  Quelle: https://www.eplan.help/en-us/Infoportal/Content/Plattform/2026/Content/htm/pagebrowsergui_k_seitentypen.htm
- Typreihenfolge im Seitenbaum (wenn nach Typ sortiert): Titelseite, Inhaltsverzeichnis, Schaltpläne (einpolig,
  allpolig, Fluid), Übersichten, Funktionsübersicht (Fluid), Schaltschrankaufbau, Grafik, Fremddokument,
  RI-Fließbild, Auswertungen (alphanumerisch).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_prinzip.htm
- Querverweise zwischen allpoliger und einpoliger Darstellung müssen in den Projekteinstellungen aktiviert werden;
  ein Paarquerverweis zu einpoliger Darstellung geht nicht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm

**Bedeutung für QET:**
- QET kennt keine Seitentypen. Seitentyp des Originals (aus Schriftfeld/Lesezeichen) als Folio-Eigenschaft bzw.
  Namenskonvention mitführen, damit Inhaltsverzeichnis und Lesezeichen „nach Seitentyp“ möglich sind.
- **Übersichtsseiten** (z. B. SPS-Übersicht) sind Darstellungsart „Übersicht“ desselben Betriebsmittels, nicht ein
  zweites Gerät: dort keine echten Leitungen ziehen; in QET Master/Slave bzw. Sprungkette zum Kanal
  (vgl. SPS-Querverweise: BMK, Anschluss, Funktionsdefinition gleich).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm
- Einpolige Seiten (Übersichtspläne) haben andere Querverweisregeln – keine Paarquerverweise erwarten.

## 16. Seiteneigenschaften, Seitenname, Nummerierung

- **Seitenname** = Name mit Trennzeichen und Unterseite ohne Strukturkennzeichen; **vollständiger Seitenname** =
  mit allen Strukturkennzeichen (z. B. =Anlage+Ort/Seite). **Unterseite**: bis vier Zeichen, mit Punkt
  getrennt, zum nachträglichen Einfügen. **Seitenbeschreibung**: mehrsprachig, Zeilenumbrüche erlaubt.
  Maßstab, Raster (Logikseiten 4 mm, Grafikseiten 1 mm), Normblatt, Formular.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_eigenschaften.htm
- **Zusatzfeld Blattnummer**: einmal je Seite und Strukturkennzeichen, bei Auswertungen ab Startnummer
  hochgezählt; Lücken/Doppelungen möglich, daher empfiehlt die Hilfe den Zähler **Seitenzahl** (ID 25020).
  Seitenanzahl/Seitenzähler je Eigenschaft (Position einer Seite in einer Gruppe gleicher Werte).
  Projektweit: Zeichen für Unterseiten, Mindestlänge des Seitennamens (führende Nullen).
  Automatisches Änderungsdatum und letzter Bearbeiter werden gesetzt, manuelles Änderungsdatum frei.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_eigenschaften.htm
- Neue Seite: vollständiger Seitenname (Vorschlag, [...] für Strukturkennzeichen), Seitentyp (interaktiv aus
  Liste, automatisch über eigenen Dialog), Seitenbeschreibung; Eigenschaften der markierten Seite werden
  übernommen; Ersteller automatisch.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_h_seitenanlegen.htm
  und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_d_seiteneigenschaften.htm
- **Nummerieren:** Seiten nachträglich mit Startwert und Schrittweite umbenennen/verschieben; EPLAN schlägt den
  nächsten freien Namen vor, Unterseiten werden hochgezählt, bei DIN-Projekten gruppenbezogen; bei
  Überschreibung Ergebnisvorschau. Umbenennen einer Seite lässt die Beschreibung unverändert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_h_seitennummerieren.htm
- Seitenreihenfolge für Ausgabe/PDF = Listendarstellung (oder Baum, wenn Projekteinstellung gesetzt).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_prinzip.htm

**Bedeutung für QET:**
- Querverweise in EPLAN (`/Seite.Spalte`, mit `=Anlage`) beziehen sich auf den **Seitennamen**, nicht auf die
  PDF-Seitennummer. Im Nachbau Folio-Namen = EPLAN-Seitenname (inkl. Unterseite, führende Nullen) setzen, sonst
  passen Verweise und Inhaltsverzeichnis nicht.
- „Blatt x von y“ im Plankopf: in EPLAN meist aus Seitenzähler/Seitenanzahl (Vermutung, je Normblatt
  verschieden); in QET `folio-id`/`folio-total` zählen Folios im ganzen Projekt – bei Zählung je Anlage eigene
  Variable pflegen.

## 17. Filter-, Sortier- und Seitenumbruchschemata, manuelle Auswahl

(Ergänzung aus der Hilfe-Durchsicht B6; Formular-Details in `formulare-platzhalter.md`.)

- Nach Wahl des Auswertungstyps öffnet sich „Einstellungen – <Auswertungstyp>“. **Auswertungsübersichten**: ein Filter,
  eine Sortierung, ein Schema „Neue Seite bei geändertem Eigenschaftswert“ und optional ein abweichendes Formular.
  Beispiel: Filter Anlage = FB3 und Einbauort = FT1 -> Inhaltsverzeichnis nur für Seiten in =FB3+FT1. Schemata werden
  benutzerbezogen gespeichert und beim Erzeugen mit der Auswertung abgelegt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_grafikausgabe.htm
- **Funktionsbezogene Auswertungen**: getrennte Gruppenfelder **Betriebsmittel** (Kopfobjekte, z. B. Klemmenleisten
  bzw. Symbolbibliotheken; nur ohne manuelle Auswahl) und **Funktionen** (z. B. Klemmen) jeweils mit Filter und
  Sortierung; „Neue Seite bei geändertem Eigenschaftswert“ gehört zu den Funktionen. Beispiele: Leisten der Anlage HF1,
  davon nur „PE-/PEN-Klemme, 4 Ziele“; Leisten nach Einbauort, Klemmen nach Montageort sortiert. „- Nicht aktiviert -“
  schaltet ein Schema ab.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_filtererweitert.htm
- **Sortierschema**: Kriterien nacheinander (z. B. Anlage, dann BMK-Kennbuchstabe, dann Einbauort). Für
  Artikelstückliste, Artikelsummenstückliste und Betriebsmittelliste ist auch **Funktionsdefinition (ID 20026)** als
  Sortierkriterium wählbar. **Seitenumbruchschema**: eine Eigenschaft; bei jedem Wertwechsel neue Seite (Beispiel
  Betriebsmittelliste: neue Seite beim Wechsel von Anlage EB1 zu FB1). Das gleiche Dialogmuster dient im
  Stückliste-Navigator als „Neue Nummerierung bei geänderter Eigenschaft“ für Positionsnummern.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_sortierung.htm
- **Manuelle Auswahl** (nur funktionsbezogen): Liste „Verfügbare“ (abhängig vom Typ: Betriebsmittel, Artikel oder
  Symbolbibliotheken) -> „Ausgewählte“; [Sortieren] sortiert alphanumerisch, sonst gilt die Auswahlreihenfolge.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswahl.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_funkbezogauswerterzeugen.htm
- **Verbindungsdiagramm** (Geräte-/Klemmen-/Steckeranschlussplan „für Verbindungsdiagramm“, manuelle Platzierung +
  manuelle Auswahl): Montageplatte wählen; Betriebsmittel erscheinen in der Reihenfolge wie auf der Montageplatte,
  ohne exakte Koordinaten; bereits ausgewertete können ausgeblendet werden.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswahlmontageplatte.htm

## 18. Seitensortierung im Detail

- Dialog „Seitensortierung – <Auswertungstyp>“ (Projekteinstellungen > Auswertungen > Ausgabe in Seiten, Spalte
  Seitensortierung, oder an der Vorlage). Arten:
  - **Aus Einstellungen** (nur an Vorlagen): Projekteinstellung verwenden.
  - **Strukturkennzeichen**: gewählte Kennzeichnungsblöcke; je **Kombination** ein Auswertungsblock, einsortiert unter
    der jeweiligen Struktur (Beispiel Anlage + Einbauort -> eigenes Inhaltsverzeichnis für =GB1+A1, =GD1+A2 …).
    Strukturtiefe je Block: „Vollständig“, „Hauptkennzeichen“ oder „Unterkennzeichen“.
  - **Gesamt**: ein Block über das ganze Projekt; zusätzlich gewählte Blöcke erzeugen **zusätzliche** Blöcke je
    Kombination. Bei funktionsbezogenen Auswertungen gibt es weder Gesamt noch Kennzeichnungsblock-Häkchen.
  - **Ausgabegruppe**: neuer Block bei jedem Wechsel der Schaltzeichen-Eigenschaft Ausgabegruppe (nicht kombinierbar).
  - **Objektkennzeichen**: Seitenstruktur nach DIN EN 61355-1 (Dokumentenart).
- **Quelle / Ziel verwenden** (nur Kabelplan, Kabelübersicht, Kabelanschlussplan, Verbindungsliste): Strukturkennzeichen
  von Quelle und Ziel statt des Kabel-BMK; Kabel zwischen unterschiedlichen Orten erscheinen in mehreren Blöcken.
- **Platzierung verwenden**: es zählen die Strukturkennzeichen der Seite, auf der das Betriebsmittel platziert ist,
  nicht die des Betriebsmittels.
- Verfügbare Arten hängen vom Typ ab (Formulardokumentation nur Gesamt).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_seitensortierung.htm
- Ablauf „ohne Vorlage“: [Einstellungen] > Ausgabe in Seiten > Seitensortierung wählen > Neu > Ausgabeform „Seite“ >
  Quellprojekt prüfen > Typ > Schemata > Startseite (Seitenname, Beschreibung, Blattnummer) **je Strukturkennzeichen**
  oder „Start für alle Strukturkennzeichen übernehmen“.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswerterzeugen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_funkbezogauswerterzeugen.htm

**Bedeutung für QET:** Liegt im Original je Einbauort ein eigener Klemmenplan-/Kabelplanblock (unter +S1, +S2 …), ist
das Seitensortierung „Strukturkennzeichen“. Ein Kabel, das in zwei Ortsblöcken auftaucht, deutet auf „Quelle/Ziel
verwenden“ – im Generator dann bewusst doppelt ausgeben, nicht als Fehler entfernen.

## 19. Abläufe rund um Auswertungen (Ergänzungen)

- **Formular zuordnen:** [Einstellungen] > Ausgabe in Seiten, Spalte Formular > „Durchsuchen“ (Formularverzeichnis aus
  den Einstellungen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswerten.htm
- **Abweichendes Formular** beim Erzeugen: wird am Auswertungsblock als Eigenschaft Formular mit „Aktualisieren mit
  diesem Formular“ gespeichert und beim Aktualisieren benutzt; beim Erzeugen einer Vorlage übernommen. **Auswerten von
  Vorlagen überschreibt** die Formulare der Blöcke. „Projekt auswerten“: eingebettete Auswertungen behalten ihr
  Formular; Auswertungsseiten mit abweichendem Formular bleiben nur, wenn es für den Typ keine Vorlage gibt oder die
  Vorlage dieses Formular trägt. **An Hauptfunktionen (Kabel-, Klemmenleisten-, Steckerdefinition …) eingetragene
  Formulare haben Vorrang vor allem anderen.**
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_updatewithformeigenschaft.htm
- **Vorlage erzeugen:** neu (Name automatisch „0001“; bei vorhandener Vorlage gleichen Typs Abfrage Überschreiben, bei
  „Nein“ zwei gleichnamige) oder aus bestehender Auswertung (Kontextmenü „Auswertungsvorlage erzeugen“). **Bedingte
  Vorlage** über Projektfilter: trifft er zu, ist „Vorlage aktiv“ gesetzt und Auswerten/Projekt auswerten erzeugt sie.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_vorlageerstellen.htm
- Aus Vorlage auswerten: auch bei einer markierten Vorlage werden alle Vorlagen dieses Typs ausgewertet.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswertungausvorlage.htm
- Export/Import von Vorlagen als *.xml (Vorgabeordner = Schemata-Verzeichnis).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_exportimport.htm
- **Löschen:** „Auswertungsblock löschen“ löscht immer alle Seiten des Blocks. Eingebettete Auswertung nur auf ihrer
  Seite löschbar (Bearbeiten > Löschen) – dann ist der Block weg, Auswertungsseiten bleiben.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswertungloeschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_eingbauswertungloeschen.htm
- **Eingefrorene Seiten:** Block wird gelöscht, Seiten bleiben (eigenes Icon), eingebettete Auswertungen nicht
  einfrierbar. Anzahl je Seitentyp in Projekteigenschaften > Statistik (Spalte Eingefrorene Seiten); Navigator-Filter
  auf Eigenschaft „Eingefrorene Auswertungsseite“.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_eingefrorseiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_eingefroreneseitenfiltern.htm
- **Geänderte Seiten finden:** Beim Aktualisieren ändert sich das automatische Änderungsdatum einer Auswertungsseite
  nur, wenn sich ihr Inhalt (Daten oder Formular) wirklich geändert hat -> Filter „Änderungsdatum (automatisch) >=
  Datum“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_geaenderteseitenfiltern.htm
- **Quellprojekt / Auswertungsprojekt:** Auswertungen eines anderen (Quell-)Projekts im geöffneten Projekt erzeugen;
  das Quellprojekt wird dabei lesend im Hintergrund geöffnet (dort kann weitergearbeitet werden). Formulare, Symbole
  und Einstellungen kommen immer aus dem Auswertungsprojekt. Seiteneigenschaft **Auswertung: Quellprojekt (ID 11067)**
  zeigt die Herkunft. „Auswertungsprojekt erzeugen“ kopiert Kopfdaten, ohne Seiten, mit eingetragenem Quellprojekt.
  Achtung: Verbindungen werden für die Verbindungsliste nur im normalen Projekt automatisch aktualisiert, nicht für ein
  Auswertungsprojekt.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_quellprojekt.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_inanderesprojekt.htm
- **Komplexe Geräte** (z. B. Mittelspannung): als Makro mit interner Verschaltung; Eigenschaftsplatzierungen an den
  Anschlüssen werden in der Betriebsmittelliste zu Platzhaltertexten -> Sprung zur Seite/zum Anschluss.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_komplexegeraete.htm
- **Grafische Auswertung**: Ablaufdiagramm (Typ Funktionsdiagramm, Formular Grafik *.f28), textorientiert oder
  symbolorientiert, mehrfach im Projekt möglich.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertunggrafisch.htm

**Bedeutung für QET:** Klemmenleisten oder Kabel mit eigenem Formular im Original (andere Optik als die übrigen) sind
kein Fehler – EPLAN nimmt das an der Leiste/am Kabel hinterlegte Formular vorrangig. Im Generator je Leiste/Kabel ein
Layout-Parameter vorsehen.

## 20. Stückliste-Navigator und Artikel-Einstellungen

- Die Stückliste ist **Projektdaten**, nicht Artikelverwaltung (Projektdaten > Geräte / Artikel > Stückliste-Navigator).
  Sie zeigt BMK mit und ohne Artikel, Artikel an Artikeldefinitionspunkten und Projektartikel; Daten werden beim Öffnen
  frisch gelesen. **Liste** = betriebsmittelorientiert (Spalten konfigurierbar), **Baum** = artikelorientiert (nur BMK
  mit Artikel; Ansichten Artikelnummer, Artikelnummer/Strukturkennzeichen, Hersteller, Lieferant,
  Stücklistengruppierung).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_d_artikelstueckliste.htm
- Kontextmenü: Projektartikel hinzufügen (ohne BMK, z. B. Beistellteile; alphabetisch im Baum), Artikel hinzufügen
  (an bestehende Artikel des BM angehängt), Artikel austauschen (Menge bleibt, Baugruppen nicht aufgelöst; K1 0815 ->
  0915), Löschen (nicht bei vorhandener Artikelplatzierung; die Position bleibt leer), Gerätedaten aktualisieren,
  Artikeldaten abgleichen (fehlende Artikel in der Datenbank anlegen), Export/Import (nur im selben Projekt).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_d_artikelstueckliste.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_artikelbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_artikelhinzufuegen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_projektartikelhinzufuegen.htm
- **Einstellungen: Artikel** (Projekteinstellungen > Auswertungen > Artikel): Baugruppen/Module auflösen bis Stufe
  (wirkt auf Artikelstückliste, Summenstückliste, Schaltschranklegende); **Artikel eines Betriebsmittels
  zusammenfassen** – Klemmenartikel zur Leiste, Aderartikel zum Kabel (Beispiel: eine Position „25 × UK5“ statt 25
  Einzelzeilen); „Artikel berücksichtigen“ je Artikelart (BM ohne Artikelnummer, Klemmen-, Leisten-, Kabel-, Stecker-,
  Kontakt-, Sammelschienen-, Verbindungs-, Vorplanungsartikel) – gilt meist auch für die Auswertungen und teils für
  die Betriebsmittelliste.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_d_paramartikelauswertung.htm
- **Verbindungsartikel** (an Verbindungsdefinitionspunkten): ein Artikel an einem Punkt mit mehreren Verbindungen geht
  an jede Verbindung; mehrere Punkte mit gleichem Artikel auf einer Verbindung zählen einmal; richtige Stückzahl erst
  nach Erzeugen der Verbindungsliste.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_artikelanverbindungen.htm
- **Montageplatte:** Artikelplatzierungen ohne Hauptfunktion definieren schon ein Gerät; ein Artikel darf trotzdem nur
  einmal in der Stückliste stehen. Platzierungen, deren Artikel an der Hauptfunktion fehlen, gelten als Fehler und
  werden zusätzlich ausgegeben (z. B. Hauptfunktion 0815;0915 + Platzierung 0715 -> Ausgabe 0815, 0915, 0715).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_k_artikelreferenzenausmontageplatte.htm
- **Positionsnummern:** reines Strukturmerkmal der Stückliste; Sortierschema, „Neue Nummerierung bei geänderter
  Eigenschaft“ (z. B. je Anlage oder Seite neu ab Startwert), Startwert, Schrittweite, Feldbreite, „Gleiche Nummer für
  identische Geräte“ (BM ohne Artikelnummer bekommen immer eine neue Nummer), Ergebnisvorschau. Eigenschaft
  **Positionsnummernliste (ID 20346)** zeigt alle Positionsnummern einer Funktion kommagetrennt (für Grafik oder
  Auswertung).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_d_positionsnummerierung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_positionenneunummerieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_d_nummerierungsvorschau.htm
- **Stücklistengruppe** (Artikelreferenzdaten): Gruppierung im Baum, als Filter und Spalte nutzbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_k_stuecklistengruppen.htm
- **Export/Import-Felder:** Export schreibt alle Artikel mit allen Feldern (unabhängig von Filtern), z. B.
  `P_ARTICLEREF_IDENTNAME` (BMK), `P_DESIGNATION_FULLPLANT`/`…FULLLOCATION` (Anlage/Ort), `P_FUNC_CODE`/`…COUNTER`
  (Kennbuchstabe/Zähler), `P_FUNC_COMPONENTTYPE` (Funktionsdefinition), `P_ARTICLEREF_PARTNO`, `…COUNT`,
  `…COUNT_TOTAL`, `…LENGTH_SUM`, `P_ARTICLE_ORDERNR`, `…TYPENR`, `…DESCR1–3`, `…MANUFACTURER`, `…SUPPLIER`,
  `REFERENCE_POS` (Reihenfolge der Artikel am BM). Zurückgeschrieben werden nur Artikelreferenzfelder (Artikelnummer,
  Variante, Datensatztyp, Menge, Funktionsgruppe, Teileart, Artikelzuordnung, Baugruppe, Positionsnummer, Zusatzfelder,
  externe Platzierung). Import mit unbekanntem BMK -> Projektartikel.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_k_felderexportdatei.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_stuecklistenexportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_stuecklistenimportieren.htm

**Bedeutung für QET:**
- Ob eine Original-Stückliste Klemmen einzeln oder als „n × Klemme“ an der Leiste führt, entscheidet „Artikel eines
  Betriebsmittels zusammenfassen“ – Zeilenbildung des Generators danach wählen.
- Projektartikel (Zeilen ohne BMK) und Montageplatten-Artikel ohne Hauptfunktion haben kein Sprungziel im Schaltplan;
  im Nachbau als Projektdaten führen, `destination` leer oder auf das Aufbaublatt.
- Der EPLAN-Stücklistenexport (Feldnamen `P_…`) ist ein brauchbares Importformat für Artikeldaten in den Fork, falls
  ein solcher Export vorliegt (**Vermutung**: Dateiformat je nach Auswahl Excel/CSV/XML, Hilfe nennt die Typen nicht).

## 21. Bedeutung für den QET-Nachbau (Zusammenfassung)

1. Jede Liste ist eine Auswertung: aus Daten generieren, bei jedem Gesamt-Bau neu (= Projekt auswerten).
2. Inhaltsverzeichnis zuletzt erzeugen, mit Sprung je Zeile auf das Blatt.
3. Stückliste: Zeile je Artikel je Betriebsmittel, BMK springt zur Hauptfunktion; Summenstückliste rechnen.
4. Seitennamen, Unterseiten und Blattanzahl exakt wie im Original (Verweise hängen daran).
5. Layout der Listen aus dem Formular ableiten: Kopf, Überschrift je Gruppe, Datenzeilen (Anzahl × Höhe = Umbruch),
   Fuß; Folgeseiten mit wiederholtem Kopf.
6. Gekürzte BMKs und Wiederholzeichen (`"`) beim Auslesen auflösen.
7. Seitentyp je Blatt mitführen (für Lesezeichen/Inhaltsverzeichnis), Übersichtsseiten nicht als eigene Geräte.
8. Weicht eine Originalliste vom Schaltplan ab: Auswertung war evtl. veraltet/eingefroren – Schaltplan führt.
9. Seitensortierung des Originals erkennen (Gesamt / je Strukturkennzeichen / Quelle-Ziel bei Kabeln) und
   nachbilden; ein Kabel darf dann in zwei Ortsblöcken stehen.
10. Stückliste: Zusammenfassung der Klemmen-/Aderartikel an Leiste/Kabel wie im Original; Projektartikel ohne BMK.
11. Seitenumbruchregeln: `formulare-platzhalter.md` Abschnitt 9.

## 22. Quellen

Gelesen und erreichbar:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungstypen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_funktionsbezogen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/formgeneratorgui_k_funktionsbezogen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsseiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_ausgabeformen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_seitensortierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsbloecke.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsvorlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswaktualisieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_automatischaktualisieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswerteinfrieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_projektauswerten.htm
- https://www.eplan.help/en-us/Infoportal/Content/Plattform/2026/Content/htm/formgeneratorgui_h_projektauswerten.htm (nur Suchtreffer-Auszug)
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswertungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_vorlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswertfestlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_struktur.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_einstellgrafikausgabe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_einstellungenausgabe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_platzhalter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_folgeformulare.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_dynamformulare.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_auswertungsrichtung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_grafikendynamischplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_platzhaltertexteeinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_sondertexteeinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formulareerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formulareigenschaftenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_bedingungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_auswertungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_dynamischeformulare.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_normblaetter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_seitentypen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pagebrowsergui_k_seitentypen.htm
- https://www.eplan.help/en-us/Infoportal/Content/Plattform/2026/Content/htm/pagebrowsergui_k_seitentypen.htm (nur Suchtreffer-Auszug)
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_eigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_d_seiteneigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_h_seitenanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_h_seitennummerieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_grafeditor_hyperlinkserkennung_248626.htm
- https://www.eplan.help/en-us/infoportal/content/Plattform/2026/Content/htm/msgmanagementgui_h_prueflaufonline.htm (nur Suchtreffer-Auszug)

Ergänzt in der Durchsicht B6 (lokal gelesene Hilfe 2.9):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_auswahlmontageplatte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_filtererweitert.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_grafikausgabe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_seitensortierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_sortierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswerten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswerterzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswertungausvorlage.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswertungloeschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_auswplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_eingbauswertungloeschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_eingefroreneseitenfiltern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_exportimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_funkbezogauswerterzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_geaenderteseitenfiltern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_inanderesprojekt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_vorlageerstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertunggrafisch.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_eingefrorseiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_komplexegeraete.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_quellprojekt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_updatewithformeigenschaft.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_d_artikelstueckliste.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_d_nummerierungsvorschau.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_d_paramartikelauswertung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_d_positionsnummerierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_artikelanverbindungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_artikelbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_artikelhinzufuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_positionenneunummerieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_projektartikelhinzufuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_stuecklistenexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_h_stuecklistenimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_k_artikelreferenzenausmontageplatte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_k_felderexportdatei.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partslistgui_k_stuecklistengruppen.htm

Nicht erreichbar (HTTP 404):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reportsgui_k_start.htm (geratener Name)
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reportgui_k_start.htm (geratener Name)

Fork-Code (gelesen, nicht geändert): `qelectrotech-eplan/sources/bomexport.cpp`, `bordertitleblock.cpp`.
