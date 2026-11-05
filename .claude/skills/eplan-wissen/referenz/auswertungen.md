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
17. Bedeutung für den QET-Nachbau (Zusammenfassung)
18. Quellen

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
  ersetzt – nur bei Platzhaltern mit „Identischen Text ersetzen“.
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

## 17. Bedeutung für den QET-Nachbau (Zusammenfassung)

1. Jede Liste ist eine Auswertung: aus Daten generieren, bei jedem Gesamt-Bau neu (= Projekt auswerten).
2. Inhaltsverzeichnis zuletzt erzeugen, mit Sprung je Zeile auf das Blatt.
3. Stückliste: Zeile je Artikel je Betriebsmittel, BMK springt zur Hauptfunktion; Summenstückliste rechnen.
4. Seitennamen, Unterseiten und Blattanzahl exakt wie im Original (Verweise hängen daran).
5. Layout der Listen aus dem Formular ableiten: Kopf, Überschrift je Gruppe, Datenzeilen (Anzahl × Höhe = Umbruch),
   Fuß; Folgeseiten mit wiederholtem Kopf.
6. Gekürzte BMKs und Wiederholzeichen (`"`) beim Auslesen auflösen.
7. Seitentyp je Blatt mitführen (für Lesezeichen/Inhaltsverzeichnis), Übersichtsseiten nicht als eigene Geräte.
8. Weicht eine Originalliste vom Schaltplan ab: Auswertung war evtl. veraltet/eingefroren – Schaltplan führt.

## 18. Quellen

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

Nicht erreichbar (HTTP 404):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reportsgui_k_start.htm (geratener Name)
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reportgui_k_start.htm (geratener Name)

Fork-Code (gelesen, nicht geändert): `qelectrotech-eplan/sources/bomexport.cpp`, `bordertitleblock.cpp`.
