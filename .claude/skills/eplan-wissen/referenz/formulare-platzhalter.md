# EPLAN Formulare, Platzhalter, Normblätter und Strukturkennzeichen-Verwaltung

Wie EPLAN (Plattform 2.9, Hilfe gelesen am 05.11.2026) Auswertungsformulare aufbaut: Formulareditor, Platzhalterelemente
und Platzhaltertexte, dynamische und bedingte Bereiche, Formulargrafiken (Brücken, Kabellinien, Schräubchen),
Spezialformulare für Anschluss-, Klemmen-, Stecker- und Kabelpläne, Seitenumbruch, Normblatteditor und die
Strukturkennzeichen-Verwaltung. Ergänzt `auswertungen.md` (dort: Auswertungstypen, Blöcke, Aktualisieren, Grundzüge
dynamischer Formulare). Eigene Worte; jede Aussage mit Quelle; „Vermutung“ = nicht durch die Hilfe belegt.

## Inhalt

1. Formulareditor: Aufbau, Formulartyp, Bedienung
2. Platzhalterelemente und Platzhaltertexte (Prinzip, Index, Kopfdaten)
3. Eigenschaften eines Platzhaltertexts (Format, Summierung, Ersetzen, QR-Code, Positionsrahmen)
4. Dynamische Bereiche, bedingte Bereiche, Überschriften, Zusammenfassen
5. Automatische Seitenbeschreibung
6. Bedingte Formulare und Folgeformulare
7. Formulargrafiken (Index/Funktionsdefinition/Eigenschaftswert zu Grafik, Brücken, Kabellinien, Gabeln)
8. Spezialformulare: Anschlusspläne, Klemmen-/Steckerpläne mit Kabeltabellen, Kabeldarstellungsplan, Summenstückliste
9. Seitenumbruch – allgemein und für Klemmen- und Kabelpläne (Antwort auf die offene Frage)
10. Platzhalterelemente je Formulartyp (Tabelle)
11. Normblatteditor (Schriftfeld, Spalten/Zeilen, Pfadnummerierung)
12. Strukturkennzeichen-Verwaltung („pleditor“)
13. Bedeutung für den QET-Nachbau (Zusammenfassung)
14. Ergänzung: Ersetzungstexte und identische Funktionstexte
15. Quellen

---

## 1. Formulareditor: Aufbau, Formulartyp, Bedienung

- Aufruf: Dienstprogramme > Stammdaten > Formular > Neu / Öffnen / Kopieren. Das Formular wird als temporäre Seite
  geöffnet; Eigenschaften über Kontextmenü > Eigenschaften im Seiten-Navigator.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_formulareigenschaften.htm
- **Formulartyp** (z. B. Klemmenanschlussplan, Inhaltsverzeichnis) trägt eine interne ID. Sie bestimmt, welche
  Platzhaltertexte im Formular wählbar sind, und welche Formulare EPLAN für einen Auswertungstyp anbietet (eine
  Betriebsmittelliste bekommt nur Betriebsmittellisten-Formulare). Feste Werte wie Firmenkennung und Änderungsdatum
  sind nicht änderbar, Beschreibung und Version frei. Alle Formulareigenschaften sind setzbar, sinnvoll aber nur
  passend zur Formularhandhabung (dynamische Eigenschaften wie „Zeilenhöhe dynamisch anpassen“ nutzen statischen
  Formularen nichts).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_formulareigenschaften.htm
- **Statische Bestandteile** (Rahmen, Linien, Rechtecke, Bitmaps/Logo, feste Texte) beeinflussen die Auswertungsdaten
  nicht und werden wie Grafik auf Schaltplanseiten bearbeitet (Einfügen > Grafik …; Text per Drag & Drop aus Office).
  **Platzhaltertexte** markieren die Position eines Felds eines Datensatzes; sie werden je Objekt ausgewertet und mit
  festem Vorschub in X/Y vervielfältigt (Vorschub = Formulareigenschaften **Spaltenbreite** und **Zeilenhöhe**). Sie
  werden nur beim Auswertungslauf gefüllt, nicht laufend. **Sondertexte** sind Platzhalter für Projekt- bzw.
  Seiteneigenschaften.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formularebearbeiten.htm
- Sondertexte: Einfügen > Sondertext > Projekteigenschaften / Seiteneigenschaften / Formulareigenschaften (im
  Formular) bzw. Normblatteigenschaften (im Normblatt). Angeboten werden nur Eigenschaften, die für den Formulartyp
  bzw. das Normblatt zugelassen sind; Format, Sprache und Rahmen sind einstellbar.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_sondertextformularnormblatteigenschaft.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_sondertextprojekteigenschaft.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_sondertextseiteneigenschaft.htm
- Anzeigehilfen: Ansicht > Platzhalter ausblenden; Ansicht > **Eigenschaften als Zahlen** (Platzhalter zeigen ihre
  internen Eigenschafts-IDs, auch im Druck und PDF-Export des Formulars); Ansicht > Schaltplanpfade (Pfadeinteilung
  als Linien).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_platzhaltertexteanzeigen.htm
- Kopieren = unter anderem Namen/Ort speichern (Dienstprogramme > Stammdaten > Formular > Kopieren), die Kopie öffnet
  sich zur Bearbeitung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formularekopieren.htm
- Prüfen: Dienstprogramme > Formular prüfen; Meldungen in den Systemmeldungen, als Protokolldatei speicherbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formularepruefen.htm
- Dokumentieren: Auswertungstypen **Formulardokumentation** und **Normblattdokumentation** geben Übersichten aller im
  Projekt verwendeten Formulare/Normblätter aus (mit Inhaltsverzeichnis und Änderungsinformationen möglich).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formularedokumentieren.htm

## 2. Platzhalterelemente und Platzhaltertexte

- Ein Platzhaltertext = **Platzhalterelement** + **Eigenschaft**. Das Element bestimmt, von welchem Objekt die
  Eigenschaft gelesen wird (z. B. Klemme, Klemmenleiste, Ziel extern Stufe 1). Welche Elemente es gibt, hängt vom
  Formulartyp ab. Im Dialog „Platzhaltertexte – <Formulartyp>“ stehen links die Elemente, rechts deren Eigenschaften
  (Filter und Kategorie schränken ein, z. B. Kategorie „Revision“ für Ersteller/Datum).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/placeholder_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_platzhaltertexteauswahl.htm
- **Indiziert:** Das Element lässt sich mit Index [n] wählen (Plus-Zeichen im Baum), z. B. „Artikeldaten [3]“ = dritter
  Artikel am Betriebsmittel, oder das n-te Ziel. Ohne Index werden die Werte **aller** Objekte ausgegeben (z. B. alle
  Artikelnummern). Achtung: Der Index zählt die tatsächlich vorhandenen Einträge, nicht die Zeilennummer in der
  Artikel-Registerkarte (Artikel in Zeile 1 und 20 -> zweiter Artikel = Index 2).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/placeholder_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_platzhaltertexteauswahl.htm
- **Kopfdaten:** Element ist für den Kopfbereich gedacht (im Klemmenplan z. B. die Klemmenleiste).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/placeholder_k_prinzip.htm
- Zeilenumbrüche im Wert entfallen, wenn der Platzhalter keinen Positionsrahmen hat oder „Umbrüche entfernen“ (unter
  Rahmen) aktiv ist.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_platzhaltertextmittabs.htm

## 3. Eigenschaften eines Platzhaltertexts (Registerkarte Platzierung / Format)

- **Eigenschaft** bzw. **Eigenschaften formatiert / berechnet** (Formatdialog mit Formatelementen, auch Rechnen mit
  „Berechnung“, Zahlen und Operatoren). **Aktion**: kundenspezifische Erweiterung am Platzhalter. **Als QR-Code
  anzeigen**: der ermittelte Wert wird beim Auswerten als QR-Code ausgegeben.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_platzierung.htm
- **Summierung** je Bereich:

  | Art | Überschrift | Datenbereichsabschluss | Fußbereich / frei / statisch |
  |---|---|---|---|
  | Zwischensumme | – | Summe von Überschrift bis Abschluss | Summe der aktuellen Seite |
  | Zwischensumme + Übertrag | – | wie oben + Übertrag des vorigen Abschlusses | Seitensumme + Übertrag der Vorseite |
  | Übertrag | Ergebnis der Vorseite | Ergebnis des vorigen Abschlusses | Ergebnis der Vorseite |
  | Gesamt | ganze Auswertung | ganze Auswertung | ganze Auswertung |

  Kopf- und Datenbereich: nur „Übertrag“ (Kopf: aus vorigem Abschluss) bzw. gar nicht. Unzulässige Kombination wird
  beim Schließen auf „Nicht summieren“ zurückgesetzt. Beispiel: Gesamtmenge im Fußbereich mit „Gesamt“ -> Endsumme
  auf der letzten Seite; Seitensumme am Ende jeder Seite nur mit „Zwischensumme + Übertrag“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_platzierung.htm
- **Eigenschaft in einer Zeile summieren:** summiert die Werte zusammengefasster Objekte (Formulareigenschaft „In einer
  Zeile zusammenfassen nach“); muss dann auch am Summen-Platzhalter gesetzt sein.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_platzierung.htm
- **Identischen Text ersetzen:** gleiche, untereinander stehende Texte werden durch die Zeichenkette aus Einstellungen >
  Projekte > Auswertungen > Anzeige / Ausgabe ersetzt; **Standard ist „=“**. Nicht für Inhaltsverzeichnisse. Für
  Funktionstexte hat die Kompatibilitätseinstellung „Auswertungen: Identische Funktionstexte ersetzen“ Vorrang.
  Korrektur zu `auswertungen.md` Abschnitt 13: dort ist `"` als Beispiel genannt; die Hilfe nennt „=“ als Vorgabe.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_platzierung.htm
- **Alle Verbindungen anzeigen** (nur Platzhalter „Kabeltabellen-Datenbereich …“ in Klemmen-/Steckerplänen): zeigt
  zusätzlich Einzelverbindungen; nur solche, die über Verbindungs- oder Potenzialdefinitionspunkt definiert sind.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_platzierung.htm
- **Positionsrahmen:** Grafikplatzhalter (Brücken, Kabellinien, BM-Kästen, Gabeln, Symbole) brauchen eine im Formular
  festgelegte Ausdehnung und Ausrichtung (Registerkarte Format > Positionsrahmen); für Text zusätzlich Textrahmen-
  Ausrichtung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formularfunction_k_bboxplaceholder.htm
- **Dynamische Textboxen:** Text bricht innerhalb der Box an Leerzeichen, Satzzeichen und Bindestrich um; vorhandene
  Zeilenumbrüche bleiben; innerhalb eines Worts nur am Bindestrich.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formularfunction_k_dynamictextboxes.htm
- Registerkarte **Platzhaltertexte** eines dynamischen Bereichs: Liste aller Platzhalter des Bereichs; Option Position
  beibehalten / Platzhaltertexte einpassen (letzter Rahmen füllt Restbreite) / gleichmäßig verteilen (rastergerecht).
  Platzhalter ohne Positionsrahmen behalten immer ihre Position.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_platzhaltertexte.htm

## 4. Dynamische Bereiche, bedingte Bereiche, Überschriften, Zusammenfassen

(Grundaufbau Kopf/Überschrift/Daten/Abschluss/Fuß siehe `auswertungen.md` Abschnitt 10.)

- **Bedingter Bereich** (Einfügen > Dynamischer Bereich > Bedingter Bereich, Registerkarte Bedingungen): wird ausgegeben
  - bei Änderung einer gewählten Eigenschaft (z. B. dicke Linie vor jedem neuen BMK in der Betriebsmittelliste; in
    Verbindungslisten auch Blockeigenschaften, z. B. Leerzeile),
  - beim Objektwechsel (z. B. grauer Balken vor jeder Klemme),
  - am ersten / an jedem Spaltenanfang, an jedem / am letzten Spaltenende (bei Zusammenfassen bezogen auf das
    Kopfobjekt, z. B. die Leiste),
  - nach jeder x-ten Zeile; „Höhe anpassen“ skaliert auf die Höhe der vorigen Datenzeile.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_bedingungen.htm
- **Überschriften aus Sortierung:** Formulareigenschaft „Für die ersten x Sortierungseigenschaften Überschriften
  erzeugen“ (ID 13060). 0 = keine, 1 = erste Sortiereigenschaft, 2 = erste und zweite … Beispiel: Sortierung
  Anlage + Einbauort + BMK, Wert 1 -> neue Überschrift je Anlage, darunter nach Ort und BMK sortiert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_hauptueberschriftenwiederholen.htm
- **In einer Zeile zusammenfassen nach** (ID 13111): identische Daten für gewählte Eigenschaften in eine Zeile.
  Trennzeichen Standard „;“ zwischen Werten und „...“ für zusammengefasste Bereiche (eigene Eigenschaften für beide
  Trennzeichen). Beispiel Inhaltsverzeichnis nach Seitentyp: `=EB3+ET1/1...=EB3+ET1/3;=EB3+ET1/5...`. Mit
  Überschriften (ID 13060) wird nur innerhalb einer Überschrift zusammengefasst. Mehrsprachige Texte nur, wenn völlig
  gleich. Ist für Artikelstücklisten „Artikel eines Betriebsmittels zusammenfassen“ gesetzt und wird nach
  Artikelnummer zusammengefasst, kommen die Artikel trotzdem einzeln.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_eigenschaftenzusammenfassen.htm

## 5. Automatische Seitenbeschreibung

- Formulareigenschaft „Format für automatische Seitenbeschreibung“ setzt die Seitenbeschreibung erzeugter
  Auswertungsseiten aus Formatelementen zusammen: Seiten-/Formulartyp (Nr. 1), Formulareigenschaften (9), Name des
  Kopfobjekts (2), weitere Eigenschaften des Kopfobjekts (10), Name des **ersten** (3) und **letzten Datenobjekts**
  (4) der Seite, Trennzeichen (5), Trennzeichen zwischen Kopfobjekten (11, immer am Listenende). Gespeichert als
  `<Nr>;<Anzahl/Länge/ID>|…`, z. B. `2;1|9;<18000,0>|5;4`.
- Beispiel der Hilfe: Klemmenplan mit 30 Klemmen je Seite -> erstes Datenobjekt Seite 1 = Klemme 1, Seite 2 = Klemme 31;
  letztes = 30 bzw. 60.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_formatfuerautomatseitenbeschreibung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_seiteninfosautomatischvergeben.htm

**Bedeutung für QET:** Seitenbeschreibungen wie „-X1 1 … 30“ im Original sind meist automatisch (Kopfobjekt + erstes/
letztes Datenobjekt). Im Generator aus dem tatsächlichen Seiteninhalt bilden, nicht abschreiben – dann stimmen sie
nach Änderungen.

## 6. Bedingte Formulare und Folgeformulare

- **Bedingte Formulare** (Einfügen > Bedingte Formulare, nur dynamische Formulare): Unterformulare gleichen Typs mit
  Filterbedingung (Zeilen UND-verknüpft, mehrere Werte mit „;“ = ODER, Platzhalter `*`/`?`). Je Objekt werden alle
  passenden Unterformulare in Dialogreihenfolge ausgewertet, dabei **nur deren dynamische Bereiche**; Formular-
  eigenschaften, Grafik außerhalb von Bereichen und Kopfbereiche kommen vom Hauptformular. Passt keins, gilt das
  Hauptformular. Nur einstufig. Unterformulare werden mit eingelagert.
- Nicht möglich bei: Formular-/Normblattdokumentation, Geräte-, Kabel-, Klemmen-, Steckeranschlussplan,
  Revisionsübersicht, Titel-/Deckblatt, Kabeldarstellungsplan.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_bedingteformulare.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_bedingteformulare.htm
- **Folgeformulareinfügepunkt** (Einfügen > Folgeformulareinfügepunkt): Auswertungstyp des Folgeformulars (meist
  Artikelsummenstückliste oder Artikelliste), Formular, Filter, Sortierung (bei funktionsbezogenen Typen die
  Funktionskriterien). **Neue Seite** aktiv -> erst Hauptauswertung, Folgeformular ab nächster Seite; aus -> direkt am
  Einfügepunkt auf derselben Seite. Typwechsel löscht die übrigen Einstellungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_folgeformulareinfuegepunkt.htm

## 7. Formulargrafiken

- Zwei Arten: **erzeugte Grafik** (nur Brückengrafik in Klemmenplan, Klemmenstückliste, Klemmenanschlussplan) und
  **Symbolgrafik** (in jedem Plan, Eigenschaft Symbolgrafik ID 13049: dieselbe Grafik wie am Bauteil).
  Brücken entstehen so: An der Basis der „Brückenbeinchen“ stehen Platzhalter „Grafik für …brücken“; beim Auswerten
  werden dort Klemmeneigenschaften mit Brückeninformation eingesetzt und in einem zweiten Lauf durch Brückengrafik
  ersetzt (Ausprägung aus Platzhalter- und Formulareigenschaften). Platzhalter z. B. „Grafik für Steg-, Einlege- und
  Drahtbrücken“, „Grafik für interne Drahtbrücken“, „Grafik für Stegbrücken“.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formularfunction_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formulargrafikeneinfuegen.htm
- **Zuordnung Index zu Grafik [1–100]:** feste Grafik je Index (z. B. Schräubchen je Klemme im Klemmenanschlussplan,
  feste Grafik je Kabelverbindung). Wert: `<Index>;<Bibliothek>;<Symbolnr>;<Variante>|…`, z. B.
  `1;SPECIAL;307;0|2;IEC_EPLAN_D;12;1`.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_allggrafikeninformularendefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formularfunction_h_insfixedgraphicsintoconndiagrams.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_zuordnungindexzugrafik.htm
- **Zuordnung Funktionsdefinition zu Grafik:** Grafik je Funktionsdefinition, z. B. eigenes Symbol für PE-Klemmen.
  Wert: `<Kategorie>;<Gruppe>;<Funktionsdefinition>;<Bibliothek>;<Symbolnr>;<Variante>|…`.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_grafikenfunktionsdefinformularendefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_zuordnungfunktionsdefzugrafik.htm
- **Zuordnung Eigenschaft / Wert zu Grafik [1–10]** (Formular und Normblatt): Grafik, wenn eine Eigenschaft einen
  bestimmten Wert hat (z. B. Klemmeneigenschaften „Grafikdarstellung in Auswertungen [1]/[2]“, oder Seiteneigenschaft
  Benutzer-Zusatzfeld 1 für eine Signatur im Normblatt). Wert:
  `<EigenschaftsID>;<Kategorie>;<Wert>;<Bibliothek>;<Symbolnr>;<Variante>|…`, z. B. `20400;0;SIE;SPECIAL;310;0`.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_grafikeneigenschaftswerteinformularendefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_zuordnungeigenschaftwertzugrafik.htm
- Alle Zuordnungen funktionieren nur mit **eingelagerten** Symbolbibliotheken; fehlt die Bibliothek im Zielprojekt,
  fehlen die Symbole in der Auswertung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_zuordnungindexzugrafik.htm
- **Verbindungsgabeln** (Eigenschaft Verbindungsgrafik, ID 13048, Element „Kabel intern/extern Stufe (1–3)“):
  Verbindungen nebeneinanderliegender Bauteile werden zu einer zweiseitigen Gabel mit so vielen Zinken wie
  Einzelverbindungen zusammengefasst; Ausdehnung = Positionsrahmen, Zinkenhöhe = Schriftgröße, Richtung = Formular-
  ausrichtung. **Kabellinien** (Kabelgrafik, ID 13107): Positionsrahmen Höhe 0 -> senkrecht zur Verbindung, sonst
  schräg; benachbarte Verbindungen desselben Kabels werden verschmolzen. Beides nicht in Formularen mit mehreren
  Spalten **und** Zeilen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formularfunction_h_insforksandcablesintoconndiagrams.htm

## 8. Spezialformulare

### 8.1 Anschlusspläne (Geräte-, Klemmen-, Kabel-, Steckeranschlussplan)

- Je Anschluss eines Objekts die Zielinformationen; **mehrstufig** möglich (Ziele über Klemmen oder weiterführende
  Betriebsmittel hinweg). Ziele von Klemmen/Steckerkontakten aus internen/externen Anschlüssen, bei anderen
  Betriebsmitteln aus „Zielverfolgung (SPS) nach“ der Anschlusslogik. Die Verfolgung bricht ab, wenn ein Betriebsmittel
  erneut gefunden wird (so erscheinen nicht gebrückte PE-Klemmen nicht mehrfach). Platzhalter gibt es für Stufe 1–3,
  weitere Stufen duplizieren Stufe 1.
- Formulareigenschaften: Anzahl Stufen intern/extern (ID 13077/13079), Stufenabstand intern/extern (13078/13080),
  Alle Ziele anzeigen (13084; bis Stufe 1 alle Ziele, danach nur die ersten; mehrere Ziele -> schräge Verbindungslinie),
  Ziele tabellarisch intern/extern (13086/13087), an Zielen nur verbundene/angeschlossene Anschlüsse (13085),
  Zielverfolgung über alle Betriebsmittel (13117; letztes Ziel = erstes BM ohne Weiterleitung), nur angeschlossene
  Anschlüsse des Geräts (13121, nur Geräteanschlussplan).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_anschlussplan.htm

### 8.2 Klemmen- und Steckerpläne mit Kabeltabellen

- Eine oder mehrere **Kabeltabellen** zeigen, über welche Kabelader eine Klemme/ein Steckerkontakt mit dem Ziel
  verbunden ist, nach Kabeln sortiert. Üblich: zwei Tabellen, **links extern, rechts intern**. Auch Tabellen je
  **Formularposition** A, B, C, D oder paarweise (A+C, B+D); die Formularposition wird an der Klemme (Registerkarte
  Verbindungen) eingetragen.
- Kabeltabelle = Kopfbereich (Kabeleigenschaften: BMK, Typ, Gegenziel …) + Datenbereich (Ader-Eigenschaften, z. B.
  Farbe/Nummer). Kopf parallel oder **quer** (90°) zum Datenbereich (Formulareigenschaft „Kabeltabelle: Kopfbereich
  quer zum Datenbereich“); Abstände „Kabeltabelle: Abstand Datenbereich/Kopfbereich …“, positive Werte in
  Formularrichtung.
- **Einzelverbindungen** (keine Kabeladern) mit „Alle Verbindungen anzeigen“: sie füllen die Spalten von der
  entgegengesetzten Seite (Kabel z. B. von rechts, Einzelverbindungen von links). Statt Kabeltyp zeigt der Kopf
  automatisch Querschnitt, Einheit und Farbe; im Datenbereich ein Ersetzungszeichen (z. B. „*“, Einstellung
  Anzeige/Ausgabe). Nur Einzelverbindungen, deren Querschnitt (ID 31002), Einheit (31059), Farbe (31004) und Bemerkung
  (20045) **von den Projektvorgaben abweichen**, erscheinen; gleiche Kopfwerte teilen sich eine Spalte.
- **Seitenumbruch der Kabeltabelle:** Ist keine Kabelspalte mehr frei, beginnt eine neue Auswertungsseite.
  Quelle (alle Punkte): https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_klemmenplan.htm

### 8.3 Kabeldarstellungsplan

- Tabelle mit Kabeldaten + optional gezeichnetes Standardkabel; je Standardkabel ein eigenes Formular. Kabel mit
  gleichem eingetragenen Kabeldarstellungsplan-Formular bilden **einen** Auswertungsblock; Sortierung nur innerhalb des
  Blocks. Kopfdaten = grafisch erstes Kabel; Kabel nach Reihenfolge im Schaltplan, nicht platzierte nach BMK hinten.
- Folgeformular Artikelsummenstückliste (gleiche oder Folgeseite) mit Platzhalter **Kabellängensumme** (Länge aller
  Kabel des Blocks mit diesem Kabelartikel, jedes Kabel einmal; Kabelartikel = Produktgruppe Kabel, Untergruppe
  Undefiniert). Reicht der Platz nicht, kommen erst alle Kabeldarstellungsseiten, dann alle Stücklistenseiten.
- Formulareigenschaft **Verbindungen**: Aderzuordnung als Tupel `Quelle;Ziel;Farbe/Nr` getrennt durch `|`, z. B.
  `1;2;rot|2;3;|3;4;blau` (Schreibweise der Hilfe weicht leicht ab).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_kdpformular.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_verbindungen.htm

### 8.4 Artikelsummenstückliste: Verpackungseinheiten

- Summiert gleiche Artikel; Menge 0 wird ohne Filter ausgegeben. Mehrsprachige Eigenschaften ohne Format werden nicht
  zusammengefasst; formatiert einsprachig schon, mit „Alle Anzeigesprachen“ nicht.
- Packungszahl per Berechnung: Gesamtmenge (Artikelreferenz) / Menge je Verpackung (Artikel) + 0,4999, 0 Nachkomma-
  stellen (7 Lampen, 5 je Packung -> 2).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_artikelsummenstueckliste.htm

## 9. Seitenumbruch – allgemein und für Klemmen- und Kabelpläne

**Antwort auf die offene Frage im SKILL.md (soweit die Hilfe es hergibt).** Eine Seite wird umgebrochen, wenn

1. der Auswertungsbereich voll ist: Zeilenanzahl × Zeilenhöhe ab Ende des Kopfbereichs (bei mehreren Spalten erst
   nächste Spalte, dann Folgeseite; Kopf/Fuß wiederholen sich). Quelle:
   https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_dynamformulare.htm
2. ein neues **Kopfobjekt** beginnt (neue Klemmenleiste / neues Kabel = neuer Block auf neuer Seite), es sei denn
   „Zusammenfassen“ (nur dynamische Formulare) ist aktiv; dann verhindert „Mind.-Anzahl Auswertungszeilen“, dass ein
   Block mit zu wenig Platz am Seitenende beginnt. Quelle:
   https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_einstellgrafikausgabe.htm
3. sich der Wert der Eigenschaft im Schema **„Neue Seite bei geändertem Eigenschaftswert“** ändert (Seitenumbruch-
   schema, beim funktionsbezogenen Typ im Gruppenfeld Funktionen; z. B. Anlage: hinter dem letzten Objekt von =EB1
   neue Seite für =FB1). Quellen:
   https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_filtererweitert.htm,
   https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_sortierung.htm
4. im Klemmen-/Steckerplan die **Kabelspalten** der Kabeltabelle aufgebraucht sind (neues Kabel ohne freie Spalte ->
   neue Seite). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_klemmenplan.htm
5. ein **Folgeformular** mit „Neue Seite“ folgt bzw. beim Kabeldarstellungsplan erst alle Haupt-, dann alle
   Folgeformularseiten kommen. Quellen:
   https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_folgeformulareinfuegepunkt.htm,
   https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_kdpformular.htm
6. eine neue **Strukturkennzeichen-Kombination** laut Seitensortierung beginnt (neuer Auswertungsblock). Bei Kabelplan,
   Kabelübersicht, Kabelanschlussplan und Verbindungsliste mit „Quelle / Ziel verwenden“ zählen die Kennzeichen von
   Quelle/Ziel, und ein Kabel zwischen zwei Orten erscheint in **beiden** Blöcken. Quelle:
   https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_seitensortierung.htm

Zeilenbedarf je Klemme: Im Klemmen-/Klemmenanschlussplan mit „Anschlussbezogene Ausgabe“ entsteht **je Anschlusspaar
eine Zeile**; Klemmen mit mehr als zwei Anschlüssen werden auf mehrere Zeilen verteilt, unbeschaltete Anschlüsse
erscheinen mit. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/placeholder_o_main.htm

Nicht in der Hilfe: ob EPLAN eine Mehrstockklemme oder die Zeilen einer Klemme über ein Seitenende hinweg trennt, und
die konkrete Zeilenzahl der Standardformulare. **Vermutung:** Die Zeilenzahl kommt allein aus dem Formular (Zeilen ×
Zeilenhöhe); deshalb im Nachbau Zeilen je Seite und erste Klemme je Seite aus dem Original-PDF messen.

## 10. Platzhalterelemente je Formulartyp

Zusammenfassung der Übersichtstabelle (`placeholder_o_main`, 597 Einträge, 2.9). Kürzel: [i] = indizierbar,
[K] = für Kopfdaten. In fast allen Typen zusätzlich: **Datensatz** (allgemeine Datensatzeigenschaften, z. B. laufende
Zeilennummer), **PLT-Stelle / PLT-Stellenfunktion** und **Vorplanungssegment** (hier weggelassen).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/placeholder_o_main.htm

| Typ | Formular | Wichtige Elemente und ihre Bedeutung |
|---|---|---|
| Artikelstückliste / Artikelsummenstückliste | f01 / f02 | Artikel (eingelagerte Artikeldaten), Artikelreferenz (am Betriebsmittel änderbare und berechnete Werte, z. B. Gesamtmenge), Artikelreferenz Hersteller/Lieferant, Artikelplatzierung [i] (Darstellungsarten zusammengefasst), Betriebsmittel (dem der Artikel gehört), Montageplatte [i] |
| Betriebsmittelliste | f03 | Betriebsmittelkennzeichen (Strukturkennzeichen und BMK-Teile), Funktion [i] (n-te Funktion), Artikel/Artikelreferenz [i] (n-ter Artikel), Angeschlossener SPS-Anschluss [i], Gerätegruppe (weitere Funktionen) [i] (ohne Index alle außer der aktuellen) |
| Formular-/Normblattdokumentation | f04 / f15 | Formular bzw. Normblatt |
| Geräteanschlussplan | f05 | Betriebsmittel [K], Artikel/Artikelreferenz [iK], Funktion, Anschluss extern/intern (n-ter Anschluss), Angeschlossener SPS-Anschluss, Stufen-Elemente (s. u.) |
| Inhaltsverzeichnis | f06 | Projekt, Seite |
| Kabelanschlussplan | f07 | Kabel [K], Kabelartikel/-Artikelreferenz [iK] (Hauptartikel+Zubehör, getrennt „Zuordnung Quelle/Ziel“), Kabelverbindung (Adern), Stufen-Elemente |
| Kabeldarstellungsplan / Kabelplan / Kabelübersicht | f08 / f09 / f10 | Kabel (im Kabelplan [K]), Kabelartikel… [i], Betriebsmittel (Quelle)/(Ziel) (nach Regeln aus den Adern bestimmt), Artikel/Artikelreferenz (Quelle)/(Ziel) [i]; nur Kabelplan: Kabelverbindung (Adern) und Verbindungsartikel… [i] |
| Klemmenanschlussplan | f11 | Klemmenleiste [K], Klemmenleistenartikel… [iK], Klemme, Klemmenartikel… [i], Klemmenanschluss extern/intern, Angeschlossener SPS-Anschluss, Stufen-Elemente |
| Klemmenaufreihplan | f12 | Klemmenleiste [K], Klemme, Klemmen-/Leistenartikel… [i], zusätzlich „… nach Produktuntergruppen“ (alle Artikel nach Untergruppe sortiert) |
| Klemmenplan | f13 | Klemmenleiste [K], Klemme, Hauptklemme (+Artikel), Klemmenanschluss extern/intern (nur mit „Anschlussbezogene Ausgabe“, je Anschlusspaar eine Zeile), Ziel/Verbindung/Verbindung-Kabel extern/intern [i] (n-tes Ziel), … über Anschluss [i] (n = Anschlussnummer in Symbolreihenfolge), … über Formularposition [i], Kabeltabellen-Kopfbereich (parallel) / -Kopfbereich extern/intern/über Formularposition (gedreht) [iK], Kabeltabellen-Kopfbereich: Gegenziel (alle Gegenziele eines Kabels, „;“-getrennt), Kabeltabellen-Datenbereich extern/intern/über Formularposition [i], Angeschlossener SPS-Anschluss |
| Klemmenleistenübersicht | f14 | Klemmenleiste, Klemmenleistenartikel [i], Ziel extern/intern (alle unterschiedlichen Ziel-BMK der Leiste) |
| Potenzialübersicht | f16 | Potenzial |
| Revisionsübersicht | f17 | Projekt, Seite (die in der Revision enthaltenen Eigenschaften) |
| Schaltschranklegende | f18 | Montageplatte [K], Montageplattenartikel [iK], 3D-Modell-Ansicht [K], Artikelplatzierung, Artikel, Artikelreferenz, Betriebsmittel (Hauptfunktion des Artikels) |
| SPS-Diagramm | f19 | SPS-Karte [K], Kommunikationseinheit [iK], Artikel [iK], SPS-Anschluss, Ziel [i] (Funktion am n-ten Anschluss), Angeschlossener Sensor/Aktor, Verbindung [i], Verbindung/Kabel [i] |
| SPS-Kartenübersicht | f20 | SPS-Karte, SPS-Anschluss [i], Kommunikationseinheit [i], Artikel [i], Angeschlossener Sensor/Aktor pro Anschluss [i] |
| Steckeranschlussplan | f21 | Stecker [K], Steckergegenstück [K] (zugeordnete Steckdose/Kupplung bei getrennter Stift/Buchse-Darstellung), Steckerkontakt, Artikel [iK], Stufen-Elemente |
| Steckerplan | f22 | wie Klemmenplan, dazu Stecker [K], Stecker-/Steckerkontaktartikel, und alles getrennt für **Stiftseite** und **Buchsenseite** [K] (Kontakt, Ziel, Verbindung, Kabel, Kabeltabellen-Datenbereich, SPS-Anschluss, Artikel) |
| Steckerübersicht | f23 | Stecker, Steckergegenstück, Artikel [i], Ziel extern/intern (alle unterschiedlichen Ziele) |
| Strukturkennzeichenübersicht | f24 | je Kennzeichnungsblock ein Element (Funktionale Zuordnung, Anlage, Anlagennummer, Aufstellungsort, Einbauort, Dokumentenart, Benutzerdefiniert, Produkt), je Block eine Spalte; oder „Strukturkennzeichen“ (alle Blöcke in einer Spalte; nicht mit den Einzelblöcken mischen) |
| Symbolübersicht | f25 | Symbolbibliothek [K], Symbol, Varianten (alle oder A–I) |
| Verbindungsliste | f27 | Verbindung, Kabel (zugehöriges Kabel), Betriebsmittel (Quelle)/(Ziel) – **grafisch erstes BMK ist immer die Quelle** –, Verbindungsartikel… [i], 3D-Artikelplatzierung (Quelle)/(Ziel) |
| Projektoptionen- / Platzhalterobjektübersicht | f29 / f30 | Projektoption, Platzhalterobjekt, Wertesatztabelle (Wertesatznamen und mögliche Werte indiziert), Betriebsmittel, Seite |
| Hersteller-/Lieferantenliste | f31 | Hersteller / Lieferant |
| Montageliste | f32 | Artikel, Artikelreferenz, Artikelplatzierung, Betriebsmittel – jeweils als Daten- und als Kopfbereich-Variante |
| SPS-Adressübersicht | f48 | SPS-Adressen (Werte der Zuordnungslisten-Zeile), SPS-Anschluss [i] |
| Weitere | f34–f47, f49, f51 | Topologie (Streckenliste/-plan, verlegte Kabel), RI-Fließbild (Rohrleitungsübersicht), Vorplanung (Struktursegmente, Planungsobjekte, Segmentvorlagen, Rohrklassen, Stoffe), Artikelgruppenplan f44, Betriebsmittelplan f45, Paket-/Leitungsplan f46, Durchbruchlegende f47 |

**Stufen-Elemente der Anschlusspläne** (f05, f07, f11, f21, jeweils intern und extern, Stufe 1–3 [i]): Ziel (die
angeschlossene Funktion der n-ten Stufe), Ziel Anschluss (deren Anschluss), Ziel Gegenanschluss (der weiterführende
Anschluss), Verbindung (Ader/Verbindung der Stufe), Kabel (Kabel der Stufe); dazu **Letztes internes/externes Ziel**
(zuletzt gefundene Funktion der Zielverfolgung).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/placeholder_o_main.htm

Die einzelnen Eigenschaften je Element (z. B. BMK, Funktionstext, Anschlussbezeichnung) stehen nicht in dieser
Übersicht, sondern im Eigenschaftenverzeichnis (`eigenschaften-ids.md`).

**Bedeutung für QET:** Ein Listengenerator im Fork braucht dieselbe Objektsicht: je Formulartyp ein Kopfobjekt und
Datenobjekte, dazu Hilfsobjekte (n-tes Ziel, n-ter Artikel, Kabel der Ader). Für den Klemmenplan heißt das: Leiste
(Kopf) -> Klemmen (Zeilen) -> je Seite externe/interne Ziele, Verbindung und Kabel; Kabel als Spaltenkopf der
Kabeltabelle mit Gegenziel. Sprungziel einer Zeile ist das Objekt hinter dem Element (Klemme, Ziel-BMK, Kabel).

## 11. Normblatteditor

- Neu: Dienstprogramme > Stammdaten > Normblatt > Neu; Standardordner `…\Normblätter\<Firmenkennung>` oder Benutzer-
  einstellung Verzeichnisse. Danach Dialog Normblatteigenschaften (Kategorie wählen, Werte setzen, weitere
  Eigenschaften über Neu). Der **Nullpunkt** wird als roter Kreis gezeigt und ist nicht verschiebbar.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaettererzeugen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblatteigenschaftenbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_normblatteigenschaften.htm
- Bestandteile: statische Grafik (Rahmen, Logo) und – wichtiger – **Sondertexte** für Projekt- und Seiteneigenschaften
  (Schriftfeld) sowie **Zeilen- und Spaltentexte** (Pfadbeschriftung), die auch in Auswertungen (Querverweise) zählen.
  Ansicht > Schaltplanpfade zeigt die Pfadeinteilung. Kopieren wie bei Formularen.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaetterbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaetterkopieren.htm
- **Spalten-/Zeilennummerierung** (Normblatteigenschaften, Kategorie Einstellungen): Anfangswert Spalte (ID 12025) und
  Zeile (12026) – Standard **0**, links nach rechts bzw. oben nach unten; Spaltennummerierungsformat (12011) und
  Zeilennummerierungsformat (12009) – leer = keine Nummer; alphanumerisch braucht eine Zahl als Anfangswert (0 und 1
  -> „A“, 2 -> „B“ …); Seitenausrichtung quer/hoch.
- **Pfadnummerierung** (Projekteinstellungen > Verwaltung > Seiten): seitenbezogen (jede Seite ab Startwert),
  projektbezogen (über alle Seiten hochgezählt; alle Seiten zählen, außer bei „Spaltenbezeichnung auf nicht-logischen
  Seiten anzeigen“) oder je Strukturkennzeichen. Übernahme mit Dienstprogramme > Pfadbezeichnungen aktualisieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaetterpfadeaktualisieren.htm
- Dienstprogramme > Spalten- und Zeilentext neu platzieren: löscht alle Pfadtexte und setzt sie exakt auf die Ränder
  der Spalten/Zeilen (letzte Formatierung).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaetterpfadeposneuplatzieren.htm
- **GOST:** invertierte Spalten- (ID 12035, rechts nach links) und Zeilennummerierung (12036, unten nach oben), dazu
  Auswertungsrichtung (12103) „Senkrecht“, BMK-Übernahme-Suchrichtung GOST (12104), Formate 12009/12011 numerisch/
  alphanumerisch; projektweise zählt nur die senkrechte Nummerierung durch.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_pfadnummgost.htm
- Eigenschaft/Wert zu Grafik gibt es auch im Normblatt (z. B. Signaturbild abhängig von Benutzer-Zusatzfeld 1).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_zuordnungeigenschaftwertzugrafik.htm

**Bedeutung für QET:** Spaltennummer im Querverweis = Spaltentext des Normblatts. Bei Anfangswert 0 beginnt EPLAN mit
Spalte 0; ein Original mit „1“ in der ersten Spalte hat Anfangswert 1. Im QET-Rahmen Spaltenzahl, Breite und
Startnummer genau so setzen (QET zählt ab 1 – Offset ggf. über Folio-Einstellung „Spalten ab“ bzw. eigene
Beschriftung, **Vermutung**: Fork-Code prüfen). Projektbezogene Pfadnummerierung (fortlaufend über Seiten) ist selten,
aber dann tragen Querverweise keine Seitennummer-Logik wie gewohnt – am Original prüfen.

## 12. Strukturkennzeichen-Verwaltung („pleditor“)

Korrektur zur Auftragsbeschreibung: Die Hilfeseiten `pleditorgui_*` beschreiben nicht einen Plotrahmen-Editor, sondern
die **Strukturkennzeichen-Verwaltung** (Projektdaten > Strukturkennzeichen-Verwaltung).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_k_start.htm

- Hauptaufgabe: **Reihenfolge** der Strukturkennzeichen festlegen. Sie sind nicht alphanumerisch sortiert, sondern
  folgen der Reihenfolge in der Kennzeichen-Datenbank; die Seitenreihenfolge im Seiten-Navigator richtet sich nach der
  Spalte „Zeile“. Umsortieren wirkt auf Seitenfolge, Querverweise und Verbindungen (Sicherheitsabfrage, nicht
  rückgängig machbar).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzanlegen.htm
- Kennzeichnungsblöcke und Vorzeichen: Funktionale Zuordnung `==` (oder `#`, `≠`), Anlage `=`, Aufstellungsort `++`,
  Einbauort `+`, Dokumentenart `&` (oder Leerzeichen), Benutzerdefiniert frei. Alle für Seiten, alle außer
  Dokumentenart auch für Betriebsmittel; frei kombinierbar; Blöcke dürfen leer bleiben (Eintrag „<Leeres
  Kennzeichen>“).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_k_grundlagen.htm
- Neue Kennzeichen (neue Seite, neues BMK, Makro einfügen) werden je Benutzereinstellung **alphabetisch einsortiert**
  oder **am Ende angefügt**; Großbuchstabenkonvertierung des Projekts gilt.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_d_einstellungenkennzeichen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzanlegenbeineuerseite.htm
- Spalten der Liste: Bezeichnung, **Strukturbeschreibung** (übersetzbar, mit Zeilenumbruch, in Listen/Auswertungen
  ausgebbar), ursprüngliche Bezeichnung, Verwendung („?“ bis „Verwendung prüfen“; „Nein“ = unbenutzt, „*“ = nur
  Unterkennzeichen benutzt), Status (Warnung bei Doppelungen). Schutz konfigurierbar; Tabellen Excel-kompatibel.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_d_kennzeichen.htm
- Umbenennen ändert Seiten und BMK, Position bleibt; Hauptkennzeichen mit Unterkennzeichen (ANL1, ANL1.1) – Abfrage, ob
  die Unterkennzeichen mitgehen. Umbenennen auf ein vorhandenes Kennzeichen = **Zusammenführen** (abgebrochen, wenn
  gleichnamige Seiten entstünden).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzbearbeiten.htm
- Löschen nur unbenutzter Kennzeichen (oder „Alles aufräumen“); Löschen von Seiten entfernt Kennzeichen nicht aus der
  Datenbank. Suchen/Ersetzen nur spaltenweise abwärts, nur Bezeichnung und Strukturbeschreibung ersetzbar, nur
  identifizierendes BMK. Syntaxprüfung über Projekteinstellung BMK-Syntaxüberprüfung.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzloeschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzsuchen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzpruefen.htm
- Platzhalter für BMK auf Auswertungsseiten ändern sich nach Umbenennen erst beim Aktualisieren der Auswertungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzanlegen.htm
- Auswahldialog <Kennzeichnungsblock> bzw. Objektkennzeichen (DIN EN 61355-1-Seitenstruktur): zeigt Kennzeichen mit
  Strukturbeschreibung, filterbar.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_d_kennzeichnungsblock.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_d_neueskennzeichen.htm

**Bedeutung für QET:** Die Reihenfolge der Anlagen/Orte im EPLAN-Seitenbaum, im Inhaltsverzeichnis und in
Strukturkennzeichenübersichten ist die Datenbank-Reihenfolge, nicht alphabetisch. Im Gesamt-Skript die Reihenfolge
aus dem Original übernehmen (Lesezeichen/Inhaltsverzeichnis) und nie neu sortieren. Strukturbeschreibungen (z. B.
Klartext zu `+S1`) gehören zentral in die Projektdaten, nicht als Text auf jedes Blatt.

## 13. Bedeutung für den QET-Nachbau (Zusammenfassung)

1. Generator = Formular: Kopfobjekt + Datenobjekte + Hilfsobjekte wie in Abschnitt 10; Layoutparameter (Zeilen,
   Zeilenhöhe, Spalten, Kabelspalten, extern links/intern rechts) aus dem Original messen.
2. Seitenumbruch nach den sechs Regeln in Abschnitt 9; erste/letzte Klemme je Seite aus dem Original als Prüfwert.
3. Wiederholzeichen: Standard „=“ (Identischen Text ersetzen), in Kabeltabellen „*“ für Einzelverbindungen – beim
   Auslesen auflösen.
4. Einzelverbindungen in der Kabeltabelle nur, wenn sie von den Projektvorgaben abweichen; fehlende Spalten sind kein
   Fehler.
5. Verbindungsliste: grafisch erstes BMK = Quelle. Kabelpläne mit „Quelle/Ziel verwenden“ können ein Kabel zweimal
   zeigen.
6. Summen/Überträge in Stücklisten-Fußzeilen sind Rechenwerte – berechnen.
7. Strukturkennzeichen-Reihenfolge = Original-Reihenfolge.

## 14. Ergänzung: Ersetzungstexte und identische Funktionstexte
**Ersetzungstexte**: Tabelle, die Werte einer Blockeigenschaft (Projekteigenschaft „Blockeigenschaft: Ersetzungstext [n]“) oder einer formatierten Eigenschaft im Formular („Eigenschaften formatiert: Ersetzungstext [n]“) durch andere Texte ersetzt – z. B. Zahlen oder Wahr/Falsch durch aussagekräftige, mehrsprachige Texte. Aktiviert wird sie im Dialog Format: Blockeigenschaft über das Feld Ersetzungstext = „Tabelle n verwenden“.
- Spalten: *Operator* (Vergleich oder „Regulärer Ausdruck“), *Quelltext* (Groß-/Kleinschreibung zählt), *Ausgabetext*, *Fortsetzen* (mehrere reguläre Ausdrücke nacheinander anwenden; Standard aus = nur einmal).
- Quell-/Ausgabetexte sind mehrsprachig, werden aber von „Bei der Eingabe übersetzen“ nicht erfasst – nur manuell übersetzbar.
- Beispiel 1: Hauptfunktion (ID 20122) liefert „X“ bzw. leer → Tabelle „= X → Hauptfunktion“, „= (leer) → Nebenfunktion“. Da Blockeigenschaft 1 bei vielen BM zur Standard-Eigenschaftsplatzierung gehört, erscheint der Text direkt am Schaltzeichen.
- Beispiel 2: Technische Kenngrößen (ID 20027) „Spannung 24 V“ → nur Zahl: Regex `^[^0-9]*` und `[^0-9]*$` jeweils durch nichts ersetzen (mit Fortsetzen) → „24“, „12“, „2,5“.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_ersetzungstexte.htm

**Identische Funktionstexte ersetzen** (Kompatibilitätsoption): untereinander wiederholte Funktionstexte auf Auswertungsseiten werden durch ein Ersetzungszeichen (z. B. `"`) ersetzt; das Zeichen kommt aus Auswertungen > Anzeige/Ausgabe „Identische Texte ersetzen durch“.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektkompatibilitaet.htm

**Bedeutung für QET**: Steht im EPLAN-PDF am Schaltzeichen „Hauptfunktion“/„Nebenfunktion“ oder eine bloße Zahl, ist das oft ein ersetzter Blockeigenschaftswert – im Nachbau den Anzeigetext übernehmen, den Quellwert (ID) aber als Elementinfo führen. Wiederholungszeichen `"` in Listen bedeutet „wie Zeile darüber“.

## 15. Quellen

Formulareditor und Formularfunktionen:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_bedingteformulare.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_eigenschaftenzusammenfassen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_folgeformulareinfuegepunkt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_formatfuerautomatseitenbeschreibung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_formulareigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_normblatteigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_platzhaltertexteauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_platzhaltertextmittabs.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_sondertextformularnormblatteigenschaft.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_sondertextprojekteigenschaft.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_sondertextseiteneigenschaft.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_verbindungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_zuordnungeigenschaftwertzugrafik.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_zuordnungfunktionsdefzugrafik.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_d_zuordnungindexzugrafik.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_allggrafikeninformularendefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_artikelsummenstueckliste.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formularebearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formularedokumentieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formularekopieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formularepruefen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_formulargrafikeneinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_grafikeneigenschaftswerteinformularendefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_grafikenfunktionsdefinformularendefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_hauptueberschriftenwiederholen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_platzhaltertexteanzeigen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_seiteninfosautomatischvergeben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_anschlussplan.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_bedingteformulare.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_kdpformular.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_klemmenplan.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_bedingungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_platzhaltertexte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_r_platzierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_dynamformulare.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formularfunction_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formularfunction_k_bboxplaceholder.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formularfunction_k_dynamictextboxes.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formularfunction_h_insfixedgraphicsintoconndiagrams.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formularfunction_h_insforksandcablesintoconndiagrams.htm

Platzhalter:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/placeholder_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/placeholder_o_main.htm

Auswertungseinstellungen (Seitenumbruch):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_filtererweitert.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_sortierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_seitensortierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_d_einstellgrafikausgabe.htm

Normblatt:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaettererzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaetterbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaetterkopieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblatteigenschaftenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaetterpfadeaktualisieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaetterpfadeposneuplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_k_pfadnummgost.htm

Strukturkennzeichen-Verwaltung:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_d_einstellungenkennzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_d_kennzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_d_kennzeichnungsblock.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_d_neueskennzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzanlegenbeineuerseite.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzloeschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzpruefen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_h_kennzsuchen.htm

Zusatz-Dialogseiten (Ergänzung Abschnitt 14):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_ersetzungstexte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektkompatibilitaet.htm
