# Änderungen im Fork

Neuere Versionen stehen oben. Das Schema erklärt [VERSIONIERUNG.md](VERSIONIERUNG.md). Die Änderungen des
Original-QElectroTech stehen in `ChangeLog*`.

## 1.51.0 – 24.11.2026 – Sprungziel im PDF markiert

- PDF-Export: Das Ziel eines Sprungs (Potentialpfeil, Querverweis, Kontaktspiegel, Dreifachsprung, Kabel- und
  Klemmenverweis, Text mit Ziel-Bauteil) bekommt wie bei EPLAN ab 2.9 einen roten, blinkenden Rahmen – gebaut wie in
  EPLANs eigenen PDFs: Die Link-Aktion ist GoTo (Seite + /FitR wie bisher) → `/Next` JavaScript
  `highlight(Seitenindex, [x1, y1, x2, y2]);`. Das Dokument-Skript `generalScript` unter `/Names /JavaScript` legt
  per `addField` ein Schaltflächenfeld mit rotem Rand über das Ziel, lässt es alle 500 ms blinken und entfernt es
  nach 5 s; ein neuer Sprung entfernt den alten Rahmen vorher. Vorab liegt nichts im PDF (kein Formular, keine
  versteckte Annotation). Läuft überall, wo PDF-JavaScript läuft (Acrobat/Reader; Chrome und Edge zeigen es beim
  EPLAN-PDF). Sprünge auf ganze Blätter bekommen keinen Rahmen.
- Neuer Schalter `--pdf-a` für `--export-pdf`: PDF/A-1b, dann wie bei EPLAN ohne Rahmen und ohne JavaScript.
- Test `tst_pdfjumpmark` prüft die Aktionskette und den PDF/A-Fall.

## 1.50.0 – 24.11.2026 – Weitergabe in der Firma, Simulation vollständig

### Simulation: closer03

- Druckwaage `closer03` = FluidSIM „2/2-Wege-Ventil, hydraulisch betätigt, in Ruhestellung geöffnet“: 2 → 3 offen, Druck an 1
  schließt über Solldruck (0,28 MPa) und Druckregelbereich (0,04 MPa), Widerstand 0,00362, Leckage 2 → 3; Formel V26, Prüfung
  „closer03“ mit Plan `closer03.qet`. Damit hat jede Funktion der FluidSIM-Bibliothek ein Rechenmodell.

### Weitergabe in der Firma

- Verteilordner (SharePoint-Bibliothek „QET-Fork“, per OneDrive synchronisiert) für Kollegen ohne Git: das
  Programm findet ihn selbst (Einstellung, `QET_DISTRIBUTION_DIR` oder OneDrive-Ordner mit `version.json`).
- Update-Prüfung beim Start und unter Hilfe > *Nach Updates suchen...*: neuere Version installieren (schließt
  QElectroTech mit Rückfrage, installiert, startet neu), später oder überspringen.
- Hilfe > *Fehler oder Wunsch melden...*: Art, Titel, Text, Bildschirmfoto und Diagnosebericht als Ordner in
  `Meldungen\`; ohne Verbindung lokal und beim nächsten Start nachgereicht.
- Firmensammlung aus `Bauteile\` des Verteilordners (bleibt dauerhaft auf dem Rechner); eine selbst gewählte
  Firmensammlung bleibt eingestellt.
- `tools/verteilung/`: `Installieren.cmd`/`.ps1` (ohne Adminrechte nach `%LOCALAPPDATA%\QET-Fork`, vorige
  Version bleibt als `Programm.alt`, Verknüpfungen), `veroeffentlichen.ps1` (Programmpaket, Bauteile,
  `version.json`), Anleitung `tools/verteilung/README.md`.

## 1.49.1 – 24.11.2026 – Ein Prüfpanel

„Plan prüfen“ des Forks und das Prüfpanel des Originals (#1878) sind jetzt ein einziges Panel.

- Grundlage ist das Prüfpanel des Originals (*Projekt > Projekt prüfen*, Knopf in der Werkzeugleiste neben
  „PDF exportieren“). Die Regeln von „Plan prüfen“ stehen dort als weitere Kategorien: Kontakte, Spulen und
  Abbruchstellen ohne Gegenstück, SPS-Kanäle ohne Adresse, Kabel ohne Ziel, Klemmen ohne Klemmleiste, Geräte ohne
  Kennzeichen oder Artikel, offene Anschlüsse, Schaltschrankaufbau.
- Jeder Befund hat eine Schwere (Fehler, Warnung, Hinweis) mit Symbol; der Tooltip nennt die EPLAN-Meldungsnummer.
  Eine Auswahl über der Liste zeigt alle Befunde, nur Fehler und Warnungen oder nur Fehler. Doppelklick öffnet das
  Blatt und markiert das Betroffene; „Prüfungen ausführen“ prüft neu.
- Keine doppelten Meldungen mehr: doppelte Kennzeichen, Potenzialtypen und Gerätetypen kommen nur aus den Prüfungen
  des Originals (Klemmen einer Leiste und Aufbau-Kopien eines Geräts zählen dort als Hinweis, nicht als Fehler). Ein
  Symbol ganz ohne Leitung steht nur unter „Symbole ohne Leitung“, offene Anschlüsse nur an Symbolen mit Leitung.
- Menüeintrag und Andockfenster „Plan prüfen“ entfallen. Gespeicherte Werkzeugleisten behalten ihren Knopf, er führt
  jetzt zum gemeinsamen Panel.
- `--check-project` und `qet.checkProject()` nutzen dieselben Regeln wie das Panel. Jeder Eintrag hat zusätzlich
  `category` und `categoryTitle`; `check` bleibt der Regelname. Das Panel prüft nicht mehr nach jeder Änderung von
  selbst (wie im Original): nach einer Änderung neu prüfen.
- Die Meldungstexte der Fork-Regeln sind jetzt auf Deutsch übersetzt (vorher fand lupdate sie nicht und sie blieben
  englisch).

## 1.49.0 – 23.11.2026

Sieben neue Funktionen für den Alltag und das Original-QElectroTech bis 23.11.2026. Alle ohne Oberfläche geprüft (Build, Referenzlauf 52/52, Befehle an Testprojekten); die Dialoge sind noch nicht von Hand angesehen. Hinweis: Das Original bringt jetzt ein eigenes Prüfpanel mit; „Plan prüfen“ des Forks steht vorerst daneben und wird zusammengelegt.

### Upstream 20.11.2026 (nachgezogen bis 23.11.2026)

Original-QElectroTech bis `64715e035` (20.11.2026) eingearbeitet, 32 Commits ohne Merges, ohne Textkonflikte:

- Neuer Elementtyp **Leitungsabzweig** (`link_type="junction"`, #1500): Ecke, T-Abzweig, Kreuzung als reiner
  Verbindungspunkt. Alle Anschlüsse liegen auf einem Potential, der Abzweig bekommt kein BMK, keine Infoseite und
  erscheint nicht in der Stückliste und in keiner Klemmenleiste. Die Rollen (`junction_source`,
  `junction_target_1..3`) stehen in der Symboldefinition; Verdrahtungsliste, Kabelzuordnung und Klemmenleisten
  verfolgen die Leitung über die Kette bis zum echten Ende.
- Gerätenavigator zeigt die freien Teile eines Geräts und platziert sie; Präfixdialog bearbeitet auch die
  `qet_labels.xml` der Firmensammlung; die automatische Leitungserzeugung bleibt im Projekt gespeichert (#1920);
  Eigenschaftsdialoge auf macOS verschiebbar.

Danach bis `8d817fac8` (23.11.2026), 79 Commits ohne Merges: Prüfpanel, das alle Projektprüfungen auf einmal
ausführt (#1878 B1), mit „Offene Leitungsenden“ und „Symbole ohne Leitung“; Projekt > Anlagen und Orte (mit
Umbenennen); Projekt > Auswertungen mit Schrankplatzliste; Stecker als Kopf mit Pins, Pinplan und Gerätetyp;
Klemmenleisten mit Art (Klemmenleiste, Stecker, Buchse); Hersteller und Bestellnummer an Kabeln; Fenster
„Artikel suchen“, Abfrage von BMK und Artikel beim Platzieren; Strukturkasten-Namen als eigener Text;
Ordnerbaum der Blätter; Seitenverweise mit Gegenstelle und wählbarer Verkettung; Kennzeichen-Anzeige einstellbar;
Präfixdateien mit beliebiger Wurzel und Kurzpfaden; `elements` auf 661f5e9ce.
Konflikte (alle „beides behalten“): `qet_compilation_vars.cmake` (Fluidprüfung + offene Leitungsenden),
`cable.cpp` (Kabelziel `destination` des Fork + Hersteller/Bestellnummer), `qetdiagrameditor.{h,cpp}`
(Assistent- und Simulationsfenster + Prüfpanel, Fluidprüfung + Leitungsenden-Prüfung im Menü Projekt);
`qet_de.ts` aus Upstream übernommen, lupdate, 950 Texte aus der Fork-Fassung, 222 neue übersetzt.

Anpassungen im Fork:

- Simulation: ein Abzweig ist ein Knoten. Vorher hätte er das Netz an seinen Anschlüssen getrennt. Neue Prüfung
  „Leitungsabzweig“ mit dem Plan `zylinder_abzweig.qet` (erzeugt von `referenz/erzeuge_abzweig.py`): Druck an
  beiden Abzweiganschlüssen gleich dem der Quelle, Endlage wie ohne Abzweig.
- `qet_mcp.py`: `junction` als `link_type` bei `qet_element_build` und `qet_element_search`.
- `lang/qet_de.ts`: per lupdate auf Stand gebracht, alle 355 offenen Texte übersetzt (auch ältere, die bisher fehlten).

### Projektsperre für SharePoint, OneDrive und Netzlaufwerke

Baut auf der Sperrdatei aus Upstream auf (`.~<Projekt>.lock` neben dem Projekt, JSON mit Benutzer, Rechner,
Prozess, Programmversion, „seit“ und Herzschlag jede Minute; wird beim Schließen gelöscht):

- Meldung beim Öffnen „<Projekt> ist seit … bei <Benutzer> auf <Rechner> geöffnet (zuletzt aktiv vor n Minuten)“
  mit Programmversion. Wahl: *Nur lesen öffnen* (Vorgabe), *Trotzdem bearbeiten* (fragt mit einer deutlichen
  Warnung nach), *Abbrechen*.
- Verwaiste Sperren: Kam seit 10 Minuten kein Herzschlag, bietet die Meldung *Sperre übernehmen* als Vorgabe an
  (Absturz, Rechner aus, Netz weg). Die Sperre einer abgestürzten Sitzung auf dem eigenen Rechner wird wie bisher
  ohne Frage übernommen, jetzt mit Hinweis in der Statuszeile.
- Übernimmt jemand anderes das Projekt, wird es hier sofort schreibgeschützt (Meldung, Statuszeile); eigene
  Änderungen gehen mit *Speichern unter* in eine andere Datei. Speichern auf eine Datei, die gerade jemand anderes
  hält, verweigert das Programm auch beim automatischen Speichern.
- OneDrive-/SharePoint-Konfliktkopien („Projekt-<Rechnername>.qetz“) neben dem Projekt werden beim Öffnen gemeldet:
  einmal je Kopie als Meldung (*Neueste Kopie öffnen*, *Ordner zeigen*), solange sie da sind in der Statuszeile.
- Nur lesen heißt jetzt wirklich nur lesen: *Speichern* ist bei schreibgeschützten Projekten aus, Strg+S und
  Speichern beim Schließen führen zu *Speichern unter*, und `QETProject::write()` schreibt die Datei nie – auch
  nicht beim automatischen Speichern, per Skript oder MCP, und auch dann nicht, wenn das Dateisystem es erlauben
  würde (Projekt einer neueren Version, Stand aus dem Verlauf). Fehler behoben: Ein abgewiesenes Speichern hat den
  Rückgängig-Stapel trotzdem als gespeichert markiert.
- Ohne Oberfläche: `--lock-status <Projekt> [--json]` zeigt Zustand (frei, gehalten, verwaist, abgestürzt), Halter
  und Konfliktkopien; Rückgabewert 0 frei, 1 gehalten, 3 verwaist/abgestürzt.
- Tests: `tst_projectlock` um verwaiste Sperre, gemeldete Absturzsperre, Konfliktkopien und zwei echte Prozesse
  erweitert (20 bestanden).

### Revisionsverwaltung

Einfache Revisionsverwaltung nach EPLAN-Vorbild (Skill `eplan-wissen`, `revision.md`):

- Projekt > Revision > *Revision abschließen…* fragt nur Index (Vorschlag) und Kurztext. Danach: Index in jedem
  Schriftfeld und in der Vorgabe für neue Blätter, Revisionstabelle als Projektvariablen `rev1_*`…`revN_*`
  (neueste zuerst) und `rev_index`, `rev_date`, `rev_text`, `rev_count`, PDF mit den markierten Änderungen seit der
  vorigen Revision und Momentaufnahme `.qet` in `<Name>_Revisionen/`, Projekt gespeichert.
- Fingerabdruck jedes Symbols, jeder Leitung, jedes Textes, jeder Form und jedes Bildes (ohne UUIDs, Klemmen-IDs,
  abgeleitete Beschriftungen) im Block `<revisions>` des Projekts; den behalten `.qet`, `.qetz` und `.qetdb` unverändert
  (Fremdblöcke). Zuordnung über UUID, danach wie EPLANs alternative Identifizierung.
- *Änderungen markieren* (an/aus, wird mitgespeichert): Revisionswolke mit Dreieck und Index, Strich entlang
  geänderter Leitungen, Gelöschtes gestrichelt und durchgekreuzt, Hinweis „Neues Blatt“; gezeichnet im Hintergrund
  des Blatts, also auch im Druck und PDF.
- *Revisionen vergleichen…*: Liste mit Sprung und PDF mit Markierung für zwei beliebige Stände.
- Neues Schriftfeld *revision* mit Tabelle der letzten drei Revisionen.
- CLI `--close-revision`, `--revisions`, `--revision-marks`, `--compare-revisions`; Skript-API `closeRevision`,
  `revisions`, `revisionChanges`, `nextRevisionIndex`, `revisionMarks`, `setRevisionMarks`, `exportRevisionPdf`.

### Beschriftungsexport

- Menü Projekt > *Beschriftung exportieren…*: Draht-, Klemmen-, Geräte- und Kabelschilder für die
  Beschriftungsprogramme Phoenix Contact MARKING system / PROJECT complete, Weidmüller M-Print PRO und WAGO Smart
  Script, dazu eine neutrale CSV mit allen Spalten. Auswahl nach Art, Blattbereich und Anlage/Ort; vollständiges
  oder kurzes Kennzeichen; ein Schild je Draht-/Kabelende oder eines. Die Zahl der Schilder steht im Dialog, bevor
  die Datei geschrieben wird.
- Die Daten kommen aus den Datenbank-Sichten `qet_wires`, `qet_terminals`, `qet_device_list` und `qet_cables`
  (Drahtnummern, Klemmleisten, Kennzeichen, Kabel), also aus denselben Zeilen wie Klemmenplan und Kabelliste.
- Keiner der drei Hersteller veröffentlicht ein festes Spaltenschema. Jede Datei ist deshalb eine beschriebene CSV
  nach dem belegten Import: bei Phoenix eine Spalte „Material“ für das Material mapping (bei Klemmen die
  Artikelnummer); bei Weidmüller der VK-Import mit Spalte „Referenz“, die nie leer ist, und „Anzahl“, als
  Windows-Text; bei WAGO eine Zeile je Schild für die Vorlagenzuordnung. Spalten und Belegstellen stehen in
  `docs/beschriftung-export.md`.
- CLI `--export-marking <projekt> <datei> [--format …] [--kinds …] [--folios N-M] [--plant …] [--location …]
  [--short] [--one-end]`, Skript-API `qet.exportMarking(datei, format, optionen)` und `qet.markings(optionen)`.

### Plan prüfen

- Neuer Knopf **Plan prüfen** (Menü *Projekt* und Werkzeugleiste): ein Prüflauf wie in EPLAN, der alle Prüfungen
  an einem Ort sammelt. Ein Andockfenster listet jede Meldung mit Schwere (Fehler, Warnung, Hinweis), Blatt,
  Kennzeichen, Text und EPLAN-Meldungsnummer; Filter „Fehler und Warnungen“ / „Nur Fehler“; ein Klick öffnet das
  Blatt und markiert Gerät, Leitung oder Kabeladern. Ist ein Lauf schnell (unter 0,8 s), prüft das Fenster nach
  jeder Änderung von selbst neu, sonst über „Erneut prüfen“.
- Geprüft wird: offene Anschlüsse (005026, an Klemmen 005053), offene Fluidanschlüsse, doppelte Kennzeichen
  (017005) und Leiternummern, Geräte ohne Artikel (007017) oder Kennzeichen (017001), Klemmen ohne Klemmleiste
  (001010, je Leiste zusammengefasst), Kontakte ohne Spule bzw. Hauptfunktion (007005), Spulen ohne Kontakt (007009),
  Abbruchstellen ohne Gegenstück (011002) oder Anschluss (011003), Kabel ohne Ziel/ohne Kennzeichen/mit verwaister
  Ader (003005), SPS-Kanäle ohne Karte oder Adresse (004023), Potenzialtypen (005016), Gerätetypen (007001/007002)
  und auf Aufbaublättern die Aufbauprüfung. Wiederverwendet werden die vorhandenen Einzelprüfungen (doppelte
  Kennzeichen, Fluid, Potenzialtypen, Gerätetypen, Aufbauprüfung); Aufbaublätter werden nicht als Schaltplan geprüft.
- Skript-API `qet.checkProject(folio = -1)` (Liste mit severity, check, eplan, folio, folioTitle, label, text, items)
  und CLI `--check-project <projekt.qet> [bericht.json]` (JSON; Rückgabewert 3, wenn Fehler gefunden wurden).

### Funktionsvorlagen

Wiederverwendbare Schaltungsteile mit Platzhaltern, ähnlich EPLAN-Makros mit Wertesätzen:

- Seitenleiste *Anzeige > Funktionsvorlagen*: Vorlagen nach Kategorie mit Suche und Vorschau, ein Formular mit den
  wenigen Werten der Vorlage, *Einfügen* (folgt der Maus, jeder Klick setzt eine weitere) und *Auf neues Blatt*. Die
  Werte der Auswahltabelle (z. B. Artikel von Schutzschalter und Schütz) stehen vor dem Einfügen unter dem Formular.
- Format: `.qetmak` mit Block `<function_template>` (Parameter mit Art `label`, `terminal`, `cable`, Text; Tabellen
  mit Schlüssel). Platzhalter `{NAME}` und `{NAME+n}` in Kennzeichen, Texten, Elementinformationen, Drahtnummern,
  Kabeltyp. Zahlen wählen die nächstgrößere Tabellenzeile, Texte die gleiche.
- Einfügen wie Einfügen aus der Zwischenablage: ein Rückgängig-Schritt, neue Identitäten, Kabellinien mit Adern,
  Verknüpfungen der Vorlage bleiben erhalten; feste Kennzeichen zählen weiter, Klemmen `X1:3` kommen sofort in ihre
  Klemmleiste, ein SPS-Kanal mit `plc_address` wird mit dem SPS-Modul verknüpft, das die Adresse hat.
- Kontextmenü *Als Funktionsvorlage speichern…* (ersetzt *Vorlage erstellen*): Name, Kategorie, Beschreibung, die
  gefundenen Platzhalter mit Beschriftung, Art und Vorgabe, optional eine aus Excel eingefügte Auswahltabelle. Ablage
  im Vorlagenordner des Nutzers (`<macros>/Funktionsvorlagen`). Eine Vorlage mit Parametern, die aus der Sammlung
  gezogen wird, fragt ihre Werte im selben Formular ab.
- Sechs Beispiele ohne Kundendaten in `funktionsvorlagen/`: Motorabgang DOL (Leistungstabelle 0,37–15 kW),
  DI-Sensor mit Kabel, Ventil mit DQ-Kanal, Meldeleuchte DQ (Farbtabelle), Taster DI, Steckdose 230 V
  (Absicherungstabelle). Neue Fork-Symbole: allgemeiner DI-/DQ-Kanal, 3-Leiter-Sensor mit Anschlüssen oben.
- Skript-API: `listTemplates()`, `templateInfo()`, `insertTemplate()`, `saveTemplate()`. MCP: `qet_edit`-Operationen
  `insert_template` und `save_template`; der Vorlagenordner des Nutzers wird an den Headless-Lauf weitergegeben.

### Einkaufsliste

Stückliste für Einkauf und ERP (Menü Projekt > *Einkaufsliste exportieren…*, CLI `--export-purchase`,
`--fetch-commercial`, Skript-API `purchaseList`, `exportPurchaseList`, `fetchCommercialData`, `purchasePriceListPath`):

- Eine Zeile je Artikel, gruppiert nach Lieferant (sonst Hersteller), Summe je Gruppe und Gesamtsumme; Geräte ohne
  Artikel am Ende mit „Artikel offen“. Excel-Mappe (`.xlsx`, Preise nachtragbar, Bestellmenge/Gesamtpreis/Summen
  als Formeln) oder CSV, dazu `<Name>_ERP.csv` als neutrales Importformat.
- Kaufmännische Daten: EAN/GTIN und Lebenszyklus aus dem EPLAN Data Portal (ohne Token), VPE und Mengeneinheit aus
  den EDZ-Paketen in `herstellerdaten`, zwischengespeichert in `einkauf/artikeldaten.csv`. Preise, Lieferzeiten und
  Lieferanten aus der eigenen Preisliste `einkauf/preisliste.csv`, die alles andere überschreibt.
- Geprüft am Live-Portal: Weder `/parts` noch die kaufmännische CSV mit Token (`/download/commercial_data`) liefern
  Preis, Lieferzeit oder VPE; EDZ-Preisfelder stehen bei den Herstellern auf 0. Solche Werte bleiben leer.
- `DataPortalPart` liest zusätzlich `identcode` und `lifecycle`.

### SPS-Abgleich mit TIA Portal

- Neues Fenster *Projekt > SPS-Variablen und TIA Portal…*: alle Kanäle der SPS-Module (E/A-Tabellen) so, wie sie nach
  TIA Portal gehen – Adresse, Name, Datentyp (Bool für DI/DQ, Int für AI/AQ), Kommentar, Modul, Blatt; Klick auf eine
  Zeile zeigt den Kanal.
- *Für TIA exportieren…* schreibt eine PLC-Variablentabelle: `.xlsx` im Format, das der Import der Variablentabelle
  erwartet (STEP-7-Hilfe V20/V21 „Format of the export file (*.xlsx)“: Blatt „PLCTags“ mit Name, Path, Data Type,
  Logical Address, Comment, Hmi Visible/Accessible/Writeable, Typeobject ID, Version ID; Blatt „TagTableProperties“;
  Dokumenteigenschaft TIA_Version = 1.0, ohne die TIA „Missing version ID“ meldet) oder SimaticML-`.xml`
  (`SW.Tags.PlcTagTable`) für TIA Portal Openness.
- Adressen: EPLAN-Adressen in deutscher oder internationaler Mnemonik (E 50.0, A1.3, PEW256, %QW64) werden zu
  %I50.0, %Q1.3, %IW256 …; Kanäle, die nur ihren Kanal nennen (DI0, DQ3, AI1 wie die Module der Sammlung), werden ab
  der Startadresse des Moduls gezählt – Elementinformation `plc_start_address` (Eingänge) bzw. `plc_start_address_2`
  (Ausgänge) wie EPLANs Startadresse/Startadresse 2, sonst die nächsten freien Bytes in Blattreihenfolge.
- Name = symbolische Adresse des Kanals, sonst Modul + Kanal (A3_DI0); doppelte Namen bekommen _2 (TIA verlangt
  eindeutige Namen je CPU ohne Groß-/Kleinschreibung).
- *Mit TIA-Tabelle abgleichen…* liest eine aus TIA exportierte Tabelle (`.xlsx`, auch ältere mit Blatt „PLC Tags“,
  oder SimaticML-`.xml`) und zeigt je Kanal: gleich, Name abweichend, Kommentar abweichend, Adresse abweichend
  (gleicher Name), fehlt in TIA, nur in TIA. Merker- und DB-Variablen werden gezählt, nicht verglichen.
  *Namen und Kommentare übernehmen* schreibt TIA-Name als symbolische Adresse und TIA-Kommentar als Funktionstext in
  die Kanäle mit gleicher Adresse (ein Rückgängig-Schritt). *Bericht speichern…* als CSV.
- CLI: `--export-tia-tags <projekt> <datei.xlsx|.xml> [--tia-table NAME] [--tia-culture de-DE]`,
  `--compare-tia-tags <projekt> <datei> [bericht.csv|-] [--apply neu.qet]`. Skript-API: `qet.exportTiaTags(datei,
  tabelle)`, `qet.compareTiaTags(datei)` (CSV-Zeilen), `qet.applyTiaTags(datei)`.
- Geprüft in TIA Portal V21 an einem leeren Testprojekt (S7-1511): der SimaticML-Export lässt sich per Openness
  importieren (32 von 32 Variablen mit Name, Adresse, Datentyp, Kommentar), ein Openness-Export aus TIA wird
  eingelesen und eine dort umbenannte Variable erkannt. Die `.xlsx` ist gegen die Formatbeschreibung der Hilfe
  geprüft; ihr Import geht nur über die Oberfläche von TIA und ist noch nicht von Hand ausprobiert.

## 1.48.0 – 20.11.2026

Die letzten FluidSIM-Bauteile ohne Modell, in FluidSIM 6.3 selbst untersucht:

- Taktstufen-Bausteine TAA und TAB: Innenschaltung aus dem FluidSIM-Symbol (Speicher-Impulsventil, UND über X, ODER mit
  L), Grundstellung an TP102_04 beobachtet; Formel V25, Prüfung „Taktstufen-Kette“ (Plan `taktkette.qet`).
- Muskel mit einem oder zwei Führungszylindern (FluidicMuscle2/3): Zylinderweg = xStart − Kontraktion, Zylinderdruck an
  AC streckt den Muskel (an TP220_13 beobachtet); Messgröße `xz:`; Formel M16, Prüfung „Muskel mit Führungszylindern“
  (Plan `muskel_zylinder.qet`).
- Damit rechnen alle FluidSIM-Beispielschaltungen ohne Modellfehler.

## 1.47.0 – 20.11.2026

FluidSIM-Bauteile, die bisher den Start verhinderten (FluidSIM-Gesamtlauf: 35 Schaltungen mit Modellfehler):

- Proportional-Wegeventil MPYE (pneumatisch 5/3, Sollwert 0 … 10 V über den lagegeregelten Proportionalmagneten):
  Querschnitt nach der Festo-Kennlinie q(U), aus dem Katalog-PDF gelesen; Formel V19.
- Pneumatische Timer (in Ruhe gesperrt/offen), Verzögerungsventile mit Speicher und Düse, pneumatischer Vorwahlzähler,
  Zweihandsteuerblock (0,5-s-Fenster) und Vakuumsaugventil nach der FluidSIM-Hilfe; Formeln V20–V24.
- Neue Prüfungen „Proportional-Wegeventil MPYE“ und „Pneumatische Zeit-, Zähl- und Sicherheitsbausteine“ (Pläne
  `mpye.qet`, `pneu_logik.qet`). Weiter abgewiesen mit Begründung: Taktstufen TAA/TAB (Verschaltung nicht
  dokumentiert), Druckwaage `closer03` (ohne Kennwerte und Hilfeseite, in keinem Beispiel), Muskel mit Zylinder.

## 1.46.1 – 20.11.2026

- Pneumatischer Muskel: Vorzeichen der externen Kraft wie in FluidSIM (in FluidSIM 6.3 an TP220_14 geprüft: −750 N
  streckt den Muskel). In 1.46.0 wirkte eine negative Kraft fälschlich in Kontraktionsrichtung. Prüfplan `muskel.qet`
  mit −674/−400/−2000 N; Befund und FluidSIM-Vergleich (FluidSIM 9,45 %, Festo-Katalogkurve 10,65 %) in FORMELN.md M15.

## 1.46.0 – 19.11.2026

Simulations-Roadmap R12 (Sonderantriebe) fertig: pneumatischer Muskel und hydraulischer Druckübersetzer mit
Herstellerdaten:

- Pneumatischer Muskel (FluidSIM `FluidicMuscle1`, Typ DMSP-10/20/40, Länge, externe Last): Zugkraft aus den
  Kraft-Weg-Kurven des Festo-Katalogs DMSP (exakt aus den Vektorpfaden des PDFs gelesen), Volumen über virtuelle Arbeit,
  Anschläge an Vorspanngrenze und max. Kontraktion; Messgrößen `x:` (Kontraktion), `h:` (%), `v:`, `F:`. MAS und Muskel
  mit Zylinder bleiben mit Begründung abgewiesen.
- Hydraulischer Druckübersetzer (FluidSIM `PressureIntensifier01`, IN/R/H, Kennwerte `r`, `p_max_in`, `q_max_in`,
  `q_max_hi` aus dem Datenblatt Pflicht, `G_leak`): Bypass, Pumpkolben bis r·p_IN, entsperrbares Rückschlagventil;
  Messgröße `qh:`.
- Formeln M15, H13, Quellen QS17 (Festo DMSP), QS18 (Scanwill MP-F); neue Prüfungen „Pneumatischer Muskel (R12)“
  (Festo-Auslegungsbeispiel 674 N/609 N → 10 mm, Vorspannung, Volumen gegen die Aufweitung laut Datenblatt) und
  „Druckübersetzer (R12)“ (Bypass, Enddruck 4·p_IN, Druckaufbau nach der Kennlinie).
- `tools/simulation/datenblaetter/`: Leser der Festo-Kennlinien und Erzeuger von `simulationmuscle.h`.

## 1.45.0 – 19.11.2026

Simulations-Roadmap R17 (analoge Elektrik), Stufe 5: elektrische Maschinen – damit ist R17 vollständig:

- Gleichstrommotor (`sim_block` MDC, `sim_params` „Ra=…;La=…;k=…;J=…;b=…;ML=…“): Ankerkreis mit Gegenspannung und Welle
  mit Trägheit, Reibung und Lastmoment im Kirchhoff-Stromkreis.
- Asynchronmotor mit Käfigläufer (`sim_block` MASM, Anschlüsse U/V/W bzw. L1/L2/L3, „Rs=…;Rr=…;Lls=…;Llr=…;Lm=…;p=…;J=…;ML=…“):
  Raumzeigermodell am Drehstromnetz (drei Sinusquellen), Hochlauf und Schlupf unter Last.
- Messgrößen `omega:` (rad/s), `n:` (1/min), `tau:` (Moment) für beide Motoren; alle Kennwerte aus Datenblatt bzw.
  Ersatzschaltbild (keine Vorgaben).
- Formeln E12, E13; neue Prüfung „Elektrische Maschinen (R17, Stufe 5)“ mit `motoren.qet`: Gleichstrom-Hochlauf gegen
  Runge-Kutta und den stationären Endwert, Asynchronmotor gegen das Strang-Ersatzschaltbild (Drehzahl, Moment, Strom).

## 1.44.0 – 19.11.2026

Simulations-Roadmap R17 (analoge Elektrik), Stufe 4: Schutzorgane:

- Sicherung (`sim_block` F, `sim_params` „In=…;I2t=…“ mit dem Schmelzintegral aus dem Datenblatt), Leitungsschutzschalter
  MCB_B, MCB_C, MCB_D (IEC 60898-1) und Motorschutzschalter MPCB (Klasse 10, IEC 60947-4-1) im Kirchhoff-Stromkreis:
  thermisches Gedächtnis mit einer Zeitkonstante und magnetische Schnellauslösung; Kennwerte `tau`, `kf`, `Im`, `R`
  überschreibbar. Meldung „Leitungsschutzschalter -Q1 löst aus (thermisch, … A)“, Signale `trip:` und `w:`.
- Zurücksetzen (neue Sicherung, Schalter wieder ein) mit dem bestehenden Ereignis „-F1 reparieren“.
- Formel E11; neue Prüfung „Schutzorgane (R17, Stufe 4)“ mit `schutz.qet` an den Normprüfpunkten (1,13 / 1,45 / 2,55·In,
  Magnetgrenzen, Klasse 10 bei 7,2·Ir) und Sicherungstausch.
- Offen in R17: Stufe 5 (Motoren).

## 1.43.0 – 19.11.2026

Simulations-Roadmap R17 (analoge Elektrik), Stufe 3: Transistor und Operationsverstärker:

- Bipolartransistor im Kirchhoff-Stromkreis: `sim_block` QN (NPN) bzw. QP (PNP), Anschlüsse C, B, E, Ebers-Moll mit
  `sim_params` „Is=1e-14;n=1;BF=100;BR=1“ (Vorgaben sind Projektfestlegungen, Datenblattwerte eintragen). Messgrößen
  `u:` = U_CE, `i:` = I_C, neu `ib:` = I_B.
- Operationsverstärker: `sim_block` OP, Anschlüsse +, −, OUT, `sim_params` „A=1e5;Rin=1M;Rout=1;Vmax=15;Vmin=-15“
  (Leerlaufverstärkung mit weicher Begrenzung). `u:` = Ausgangsspannung.
- Formeln E09, E10; neue Prüfung „Halbleiter (R17, Stufe 3)“ mit `halbleiter.qet`: NPN aktiv und gesättigt, PNP,
  invertierender Verstärker linear und in der Begrenzung, jeweils gegen eine unabhängige Lösung der Bauteilgleichungen.
- Offen in R17: Stufe 4 (Schutzorgane), 5 (Motoren).

## 1.42.0 – 19.11.2026

Simulations-Roadmap R17 (analoge Elektrik), Stufe 2: Kopplung an die Schaltlogik:

- Kontakte im Stromkreis: liegen beide Anschlüsse eines Tasters, Grenztasters oder Relaiskontakts auf Netzen des
  Kirchhoff-Stromkreises, schaltet der Kontakt dort einen Widerstand (R_on 10 mΩ, R_off 1 GΩ, änderbar mit `sim_contact`).
  Messgrößen `u:`/`i:` unter „-K1/Schließer“ bzw. „-K1/14“.
- Relais- und Ventilspulen im Stromkreis: R und L in Reihe aus `sim_coil` („R=240;L=2.4;Un=24“), Anzug und Abfall nach dem
  Spulenstrom mit Hysterese (Vorgabe 0,8 bzw. 0,1 des Nennstroms oder „Ion/Ioff“), danach Ansprech- und Abfallzeit wie bisher.
  Freilaufdioden verlängern den Abfall wie im echten Kreis.
- SPS-Analogeingang `sim_block` AI (Eingangswiderstand Rin, Normierung „Umin/Umax/raw“, z. B. 0…10 V ↔ 0…27648) und
  -ausgang AQ (Spannungsquelle aus einer Variablen), Variable aus `sim_var`. Neues Signal `plc:<Variable>`.
- Formeln E06–E08; neue Prüfung „Kopplung Stromkreis (R17, Stufe 2)“ mit `kopplung.qet`: Anzugszeit gegen die e-Funktion,
  Abfallzeit über die Freilaufdiode gegen eine unabhängige Runge-Kutta-Rechnung, Relaiskontakt im zweiten Kreis,
  Analogeingang gegen den belasteten Teiler, Analogausgang.
- Offen in R17: Stufe 3 (Transistor, Operationsverstärker), 4 (Schutzorgane), 5 (Motoren).

## 1.41.0 – 19.11.2026

Simulations-Roadmap R17 (analoge Elektrik), Stufe 1; R18 und R19 entschieden:

- Analoger Stromkreis nach Kirchhoff (modifizierte Knotenanalyse, implizites Euler-Verfahren): Bauteile mit Elementinformation
  `sim_role` = `circuit`, `sim_block` = R, C, L, V (Spannungsquelle), I (Stromquelle), D (Diode), GND (Masse) und
  `sim_params` mit SI-Vorsätzen („R=1k“, „C=100u“, „V=10;f=50“, „Is=1e-12;n=1“). Messgrößen `u:`, `i:`, `pel:` je Bauteil.
  Formeln E01–E05.
- Neue Prüfung „Analoge Elektrik (R17, Stufe 1)“ mit `analog.qet`: RC- und RL-Glied gegen die e-Funktion, gedämpfter
  Reihenschwingkreis gegen die geschlossene Lösung, Einweggleichrichter gegen die Shockley-Gleichung, Spannungsteiler mit
  Leistungsbilanz.
- Stufe 1 steht neben der Schaltlogik: der Stromkreis treibt noch keine Relais, Spulen oder SPS-Eingänge; Transistoren,
  Verstärker, Schutzorgane und elektrische Maschinen folgen in eigenen Stufen (`tools/simulation/ROADMAP_STAND.md`).
- R18 (OPC UA, echte E/A) für die lokale Offline-Simulation ausgeklammert, R19 (Zusatzphysik, FMI) ohne Umsetzungsauftrag,
  solange kein Beleg oder Bedarf vorliegt – beides wie in der Roadmap vorgesehen, mit Begründung im Stand.

## 1.40.0 – 19.11.2026

Simulations-Roadmap R16 (Simulationsmodi, Messabgriffe, Leistung):

- Simulationsfenster: „Schritt 10 ms“ (ein Steuerzyklus) und „Bis zur nächsten Änderung“ (höchstens 10 s), Ankreuzfeld „Nur
  aktuelles Blatt“; Pause/Fortsetzen und Stop/Neustart (Rücksetzen) wie bisher.
- Kommandozeile `--simulate`: `--folios 1,3` bzw. `2-4` rechnet nur diese Blätter, `--until-change` hält bei der ersten
  Zustandsänderung; `--meta` nennt Rechenzeit, Schritte, Schritte je Sekunde, Zahl der Fluidnetze und die Blätter.
- Kern: `runUntilChange()`, `stepCount()`, `netCount()`. Formel N10.
- Neue Prüfung „Simulationsmodi (R16)“ mit `zwei_blaetter.qet`: Halt an der Ventilumschaltung, Endlage zur selben Zeit wie im
  Dauerlauf, Einzelschritt 10 ms, Blatt 2 allein mit demselben Druck wie im ganzen Projekt.
- Das Simulationsfenster selbst ist nicht im Programm durchgeklickt (Bedienung ungeprüft), der Kern dahinter ist geprüft.

## 1.39.0 – 19.11.2026

Simulations-Roadmap R15 (Fehlerszenarien und Reparatur, Fehlermodelle wie FluidSIM):

- Elementinformation `sim_fault` am Bauteil: `kolben_klemmt`, `spule_unterbrochen`, `interne_leckage [l/(min·bar) bzw. l/min]`,
  `verstopft [Restanteil]`, `immer_gesperrt`, `immer_geoeffnet`, `schaltet_nicht`, `schaltet_immer`, jeweils mit `ab t` und
  `bis t` (Sekunden); mehrere durch „;“ getrennt. Unpassende Fehler (etwa „Kolben klemmt“ an einem Zylinder) halten den Lauf mit
  Meldung an. Formel K16.
- Reparatur: Ereignis `1.0 -QM1 reparieren` (Kommandozeile, MCP `qet_simulate`) hebt die Fehler des Bauteils auf; Meldungen
  „Fehler -QM1: …“, „Fehler -QM1 aufgehoben“, „-QM1 repariert“; Messgröße `fault:-QM1`.
- Neue Prüfung „Fehlerszenarien (R15)“ mit `fehler.qet` und Varianten von `hydraulik.qet`: Taster ohne Wirkung, Spule bis zur
  Reparatur unterbrochen (0,7 s später vorne), klemmender Kolben bis 1,2 s (0,9 s später), gleicher Verlauf bei Wiederholung,
  Rückschlagventil gesperrt (Pumpe auf Druckbegrenzung) und offen (rückwärts durchströmt), Drossel auf 80 % verstopft,
  Leckage am Kolben vorbei aus der Strombilanz.
- FluidSIM-Beispiele unverändert (1100 ohne Modellfehler, 1131 ohne Programmfehler, 296 bewegte Zylinder).

## 1.38.0 – 18.11.2026

Simulations-Roadmap R14 (GRAFCET-Struktur nach IEC 60848, Schreibweise wie FluidSIM):

- Teil-GRAFCETs: Ketten mit derselben Elementinformation `sim_grafcet` (G1, G12 …) bilden einen Teil-GRAFCET, die übrigen den
  globalen GRAFCET. Schrittnummern dürfen sich zwischen Teil-GRAFCETs wiederholen (Instanzen); `X3` meint den eigenen Schritt,
  `G5.X3` den eines anderen, `XG5` ist 1, solange G5 einen aktiven Schritt hat. Formel N06.
- Einschließender Schritt (`sim_encloses` = „G2 G3“): aktiviert die Aktivierungsschritte der eingeschlossenen Teil-GRAFCETs und
  deaktiviert sie beim Verlassen. Formel N07.
- Makroschritt `M3` mit Expansion `E3` … `S3`. Formel N08.
- Zwangssteuernde Befehle `G5{8, 9}`, `G5{}`, `G5{*}`, `G5{INIT}` als kontinuierliche Aktion. Formel N09.
- FluidSIM-Import übernimmt Teil-GRAFCET-Rahmen, einschließende Schritte, Aktivierungsverbindungen, Makroschritte,
  zwangssteuernde Befehle und Sprungziele von Transitionen; die Beispiele Grafcet11, 12, 14, 15, 17 laufen jetzt ohne
  Programmfehler.
- Neue Prüfung „GRAFCET-Struktur (R14)“ mit `grafcet_r14.qet`.
- FluidSIM-Beispiele: 1100 von 1136 ohne Modellfehler, 1131 ohne Programmfehler (vorher 1126); keine Schaltung verschlechtert.

## 1.37.0 – 18.11.2026

Simulations-Roadmap R13 (mobile Hydraulik) abgeschlossen:

- Load-Sensing-Steuerblock (FluidSIM `MobileValveBlockLS`, zwei Scheiben): Schieberweg aus dem Vorsteuerdruck C/D zwischen
  pS_min und pS_max (oder Handhebel), vorgeschaltete Druckwaage je Scheibe hält den Strom lastunabhängig, Wechselventile melden
  den höchsten Lastdruck an LS. Formel V17.
- Prioritätsventil LS: CF wird bevorzugt versorgt, der Rest geht nach EF. Formel V18.
- Neue Prüfung „LS-Steuerblock und Prioritätsventil (R13)“ mit `lsblock.qet`: gleicher Strom bei 52,6 und 33,4 bar Pumpendruck
  im Band der Druckwaage, LS = Lastdruck, Rückhub, Prioritätsventil gegen die Handrechnung.
- FluidSIM-Beispiele: 1100 von 1136 rechnen (vorher 1095), alle 27 TP800-Übungen; keine Schaltung verschlechtert.

## 1.36.0 – 18.11.2026

Simulations-Roadmap R13 (mobile Hydraulik), zweiter Teil:

- Lenkventil (Orbitrol, FluidSIM `SteeringUnit_1/2`): Drehventil öffnet mit dem Handmoment, neutral P → E, links P → L /
  R → T, rechts überkreuzt; die Dosierpumpe dreht das Lenkrad (Signal `phi:`). Formel H10.
- Vorsteuergerät 2×2 (Joystick): je Gruppe ein Druckminderventil mit |Auslenkung|·pMax am Steueranschluss. Formel H11.
- Entsperrbares Doppelrückschlagventil (Sperrblock, Ausführungen 01 und 02): frei ab der Federvorspannung, Entsperren über
  das Flächenverhältnis des Steuerkolbens. Formel V16.
- Belastungseinheit 1 mit Querentlastung der Stangenräume über Druckbegrenzungs- und Rückschlagventil. Formel M14.
- Membranspeicher mit Sicherheitsblock: Vorfülldruck `p0_accu`, Kugelhahn `x`, Druckbegrenzung zum Tank. Formel H12.
- Behoben: Das vereinfachte FluidSIM-Antriebsaggregat (ein Anschluss P2) und die FluidSIM-Pumpe `Pump01` (A/B) lieferten
  keinen Strom, weil die Pumpe nur die Anschlüsse P und T kannte; betroffen waren 11 TP800-Übungen und 5 weitere Beispiele.
- Drucksignale an Anschlüssen mit Punkt im Namen (`p:-MM1.A1.2`, `p:-KH1.X1.1`) lassen sich jetzt abfragen.
- Neue Prüfung „Lenkung und Sperrventile (R13)“ mit `lenkung.qet` (sieben Kreise, vier Varianten).
- FluidSIM-Beispiele: 1095 von 1136 rechnen (vorher 1088), 22 von 27 TP800-Übungen; bewegte Zylinder 296 (vorher 291),
  keine Schaltung verschlechtert. Die Lenkübungen stehen, weil FluidSIM sie mit Handmoment 0 speichert (gelenkt wird dort
  im Lauf).
- Noch abgewiesen: LS-Steuerblock (`MobileValveBlockLS`), Prioritätsventil LS.

## 1.35.0 – 18.11.2026

Simulations-Roadmap R13 (mobile Hydraulik), erster Teil:

- Hydraulikaggregat mit LS-Verstell- und Konstantpumpe (FluidSIM `LoadSensingPumpUnit`, P1, P2, LS): der LS-Regler hält
  die Druckdifferenz P1 − LS auf der Federvorspannung (11,7 bar, Regelbereich 0,8 bar, PT1 20 ms), Bereitschaftsdruck bei
  geschlossenem P1, Druckbegrenzung je Pumpe, Leckage, LS-Leitung über rT_LS entlastet.
- 6/3-Wege-Proportional-Handhebelventil (Open Center): Umlauf P1 → T1 in der Mitte, Auslenkung ±100 % öffnet P2 → B / A → T2
  bzw. P2 → A / B → T2, Zulauf über das Lasthalte-Rückschlagventil, Schieber mit Zeitkonstante.
- Belastungseinheit (`LoadingUnit_2`): zwei gegenüberliegende Zylinder auf einer Stange mit FluidSIM-Reibung.
- Neue Prüfung „Mobilhydraulik (R13)“ mit `mobilhydraulik.qet`: Bereitschaftsdruck 12,44 bar, Druckbegrenzung der
  Konstantpumpe, LS-Regelung bei doppeltem Lastwiderstand mit gleichem Volumenstrom, Belastungseinheit über das
  Handhebelventil aus- und einfahrend mit q/A. Formelregister H08, V15, M13.
- Durchflussmesser mit Schluckvolumen (FluidSIM `Flowmeter_2`) rechnet jetzt als Hydromotor mit Reibung und Leckage
  (Δp = (2π/D)²·f·q, Formel H09) statt als fast widerstandslose Blende; die Blende machte LS-Kreise numerisch instabil.
- Noch abgewiesen: Lenkventil (Orbitrol), LS-Steuerblock, Vorsteuergerät (Joystick), Belastungseinheit 1 mit Druckbegrenzung.
- FluidSIM-Batch: 1088 von 1136 Beispielen starten (vorher 1071), davon 16 der 27 TP800-Übungen (Mobilhydraulik);
  keine Schaltung verschlechtert, bewegte Zylinder unverändert 291.

## 1.34.0 – 18.11.2026

Simulations-Roadmap R12 (Sonderantriebe), zweiter Teil: Mehrstellungszylinder.

- Mehrstellungszylinder (FluidSIM `MultiPositionCylinder`, Anschlüsse C1, C2, C3): zwei Kolben gleichen Durchmessers
  hintereinander, der hintere schiebt den vorderen mit seiner Stange bis zur Zwischenstellung (`xMax2`, Standard halber
  Gesamthub). Kontakt einseitig, Stoß plastisch, C2 wirkt auch auf das Stangenende des hinteren Kolbens; Startlagen
  `xStart`/`xStart1`, externe Kraft `F_Extern`. Neue Messgröße `x1:` (hinterer Kolben), Meldung „Zwischenstellung“.
- Neue Prüfung „Mehrstellungszylinder (R12)“ mit `mehrstellungszylinder.qet`: C1 → Zwischenstellung (C3 mit Blindstopfen als
  Luftpolster; ohne Gegendruck läuft der vordere Kolben mit seinem Schwung darüber hinaus), C2 → vorne,
  C1 + C3 → Halt im Rückhub in der Zwischenstellung, C3 → beide zurück. Formelregister M12.
- Druckübersetzer bleibt abgewiesen, jetzt mit Grund: FluidSIM nennt nur die Datenblattgrößen (r, p_max, q_max, G_leak)
  ohne Standardwerte und ohne Innenschaltung. Tandem- und Teleskopzylinder sind in FluidSIM keine Bauteile.
- FluidSIM-Batch: kein Beispiel verwendet Mehrstellungszylinder oder Druckübersetzer; Ergebnis unverändert.

## 1.33.0 – 18.11.2026

Simulations-Roadmap R12 (Sonderantriebe), erster Teil: Druckluftmotoren.

- Druckluftmotor mit Mittelauspuff (FluidSIM `MotorWithMidExhaust`): lineare Momentenkennlinie aus Nenndruck,
  Nenndrehzahl, Nennmoment und n₀/n_N, Moment proportional zur Druckdifferenz, Luftverbrauch zum Auspuff, Fremdmoment
  bremsend oder antreibend; einfacher Druckluftmotor aus Schluckvolumen, Reibung und Trägheit (vorher abgewiesen).
- Neue Prüfung „Druckluftmotoren (R12)“ mit `druckluftmotor.qet`: Leerlauf bei 6 bar doppelt so schnell wie bei 3 bar,
  Nennpunkt 3 bar mit 0,62 N·m bremsend genau 975 1/min, rechts- wie linksherum. Formelregister M11.
- Muskeln und Belastungseinheit bleiben abgewiesen: die Muskelkennlinie braucht Herstellerdaten (DMSP), die
  Belastungseinheit gekoppelte Zylinder (R13).
- FluidSIM-Batch: 1071 von 1136 Beispielen starten (vorher 1064): sieben Übungen aus TP101/TP201 mit Druckluftmotor;
  bewegte Zylinder unverändert 291.

## 1.32.0 – 18.11.2026

Simulations-Roadmap R11 (Standardventile und Zylinderkräfte):

- Zylinder nach dem FluidSIM-Kräftegleichgewicht: Einbauwinkel (Gewicht m·g·sin α gegen das Ausfahren), äußere Masse
  mit Haft- und Gleitreibung (µ·m·g·|cos α|), hydraulische Kolbenreibung aus Losbrechkraft, Coulomb'scher und viskoser
  Reibung, Federkennlinie aus dem Federdruck bei x = 0 und x_max, Kraftprofil über den Kolbenweg getrennt für Aus- und
  Einfahren. Zylinder ohne diese Kennwerte rechnen wie bisher.
- Gleichzeitige Betätigung beider Ventilseiten: der im Symbol festgelegte Vorrang (`dominant`) gewinnt, ohne Vorrang
  bleibt der Schieber stehen (vorher wurde der Vorrang nicht ausgewertet).
- Neue Prüfung „Standardventile und Zylinderkräfte (R11)“: alle 172 Ventilsymbole der Sammlung (Wege nur zwischen
  vorhandenen Anschlüssen, gültige Ruhestellung, Betätigung vorhanden), Stellungsfolge bei gleichzeitiger Betätigung
  mit und ohne Vorrang, und als Kraftdifferenz beim Ausfahren gegen die Handrechnung: Gewicht 1000,6 N, Kraftprofil
  1500,0 N, Feder 879,6 N, Kolbenreibung 324,4 N, Gleitreibung der Last 196,2 N. 32 Prüfungen, alle bestanden.
  Formelregister M08–M10, V14.
- FluidSIM-Batch: 1064 von 1136 Beispielen starten (wie 1.31.0), in 291 bewegen sich Zylinder (vorher 298). Die
  Verschiebung kommt von der FluidSIM-Kolbenreibung: In 16 Übungen (z. B. TP501_07 mit offenem Umgehungs-Absperrventil,
  0,47 bar am Kolben) reicht die Kraft nicht mehr für die Losbrechkraft von 21 N, wie in FluidSIM; vorher genügten die
  zu kleinen Vorgabewerte (6 N). Neu bewegen sich 9 Übungen mit hängender Last oder Gewicht (z. B. TP501_16, α = 270°).

## 1.31.0 – 18.11.2026

Simulations-Roadmap R09 vervollständigt (Regel- und Lastventile):

- Druckwaagen (FluidSIM `PressureCompensator*`, Öl und Luft): Schließer und Öffner 01/02 (Hauptstrom B ↔ A, Steuerung
  X − XS; mit X an A ein Druckregler), vorgeschaltete Druckwaage (hält p_A − p_X), nachgeschaltete Druckwaage (öffnet
  ab p_X + p0) und Open-Center-LS-Druckwaage (P → T ab p_X + p0); Solldruck, Regelbereich, Widerstand aus FluidSIM.
- Schock- und Nachsaugblock: A1/A2 und B1/B2 durchgehend, Druckbegrenzung von A und B gegen T, Nachsaugen über die
  Rückschlagventile.
- Pneumatisches Folgeventil (1 → 2 ab Schaltdruck an 12).
- FluidSIM-Import: diese Ventile übernehmen die Anschlussnamen des Bauteils (gespiegelte Symbole sind umbeschriftet).
- Prüfung „Stetig-, Senkbrems- und Zuschaltventile (R07/R09)“ erweitert um die Pläne `druckwaagen.qet` und
  `folgeventil.qet`: LS-Druckwaage 22,72 bar und Schockventil 48,00 bar wie das Gleichgewicht der
  Handrechnung, vorgeschaltete Druckwaage hält 5,38 bar über der Drossel (Band 4 … 5,5), Schließer als Druckregler
  11,69 bar (Band 10 … 12), Nachsaugen −0,728 bar wie der Druckabfall des Rückschlagventils, Folgeventil schaltet bei
  3 bar durch und bleibt bei 7 bar zu. 31 Prüfungen, alle bestanden. Formelregister V10–V13.
- FluidSIM-Batch: 1064 von 1136 Beispielen starten (vorher 1051), in 298 bewegen sich Zylinder (vorher 286): alle
  Folgeventil-Übungen aus TP101, TP701_10 mit Druckwaage, die Druckwaagen-Shows; in TP800 bleiben nur LS-Pumpe,
  Lenkung und 6/3-Handhebelventil ohne Modell (Roadmap R13).

## 1.30.0 – 17.11.2026

Simulations-Roadmap R06 (analoge Signale und Regler):

- Signalvertrag: jedes elektrische Netz hat ein Potential gegen 0 V (Analogausgang, sonst Potential der Quelle,
  `sim_volt`, ohne Angabe 24/0 V). Die Bausteine rechnen stetig im Modellschritt, jeder nach denen, die seine Eingänge
  treiben; SPS und GRAFCET lesen die Analogwerte nicht. Zwei Ausgänge auf einem Netz sind ein Modellfehler.
- Neue Bausteine (`sources/simulation/simulationsignals.h`): Funktionsgenerator (Rechteck, Sinus, Dreieck, Konstant,
  Profil), Sollwertkarte (W1 … W8, Rampen R1 … R4, drei Modi), PID-Regler (Parallelform, Messwertableitung mit Filter,
  Anti-Windup durch Rückführung, Bereich ±10 V oder 0 … 10 V, Reset ohne Versorgung), Zustandsregler, PT2-Glied,
  I-Glied, Differenzverstärker (Bereich aus A1/A2), Komparator mit Hysterese, Proportional-Verstärker 1/2 Kanäle mit
  Sprungstrom, lagegeregelter Proportional-Ventilmagnet, Analog-Druck-, Durchfluss- und Wegsensor, Voltmeter.
- 4/3-Stetigventile bekommen ihren Sollwert jetzt aus dem gekoppelten Magneten oder Verstärker; neu gerechnet werden
  das Proportional-Druckbegrenzungsventil (Öffnungsdruck 0 … 10 V) und der hydraulische Teil des Durchflusssensors.
- FluidSIM-Import: Analogbausteine mit Rolle, Klasse, allen Kennwerten und Kopplung (Ventil-S/Y1/Y2 ↔ Magnet bzw.
  Verstärker, Zylinder ↔ Wegmesssystem, elektrischer ↔ fluidischer Sensorteil), Potentiale der Spannungsquellen,
  Anschlüsse mit dem FluidSIM-Namen. Das Proportional-Druckbegrenzungsventil war vorher ein normales DBV mit falschen
  Anschlussnamen und damit ohne Wirkung. Gespiegelte Stetigventile hatten seit R07 vertauschte Anschlüsse A/B und P/T
  (FluidSIM beschriftet gespiegelte Symbole um); jetzt gelten die Bauteilnamen. Vorzeichen wie FluidSIM: ein positiver
  Sollwert öffnet P→A (Y2 bzw. S), Y1 öffnet P→B. Der Durchflussmesser `sensor.flowmeter_2` misst jetzt (Blende rT 0,001).
- Neue Prüfung „Analoge Signale und Regler (R06)“: Kalibrierung, Umkehr, Sättigung, Anti-Windup (sofort aus der
  Begrenzung, ohne 18,9 s), Reset, Abtastverfeinerung (Fehler 0,00135 → 0,00068, Ordnung 1), Sollwertkarte,
  Signalformen, PT2, Komparator, Verstärker; Pläne `druckregelung.qet` (PI-Regler hält 40,00 bar, Stellgröße 2,84 V)
  und `lageregelung.qet` (P-Regler fährt auf 120,0 mm ohne Überschwingen; ohne Versorgung steht der Zylinder).
  31 Prüfungen, alle bestanden. Formelregister A00–A12 und V09.
- FluidSIM-Batch: 1051 von 1136 Beispielen starten (wie 1.29.0), in 286 bewegen sich Zylinder (vorher 285). Die
  Lageregelkreise aus TP511 regeln jetzt wirklich (TP511_15 folgt dem Sollwert 8 V/2 V auf 160/40 mm), die
  Verstärkerbeispiele aus TP701/TP702 steuern ihr Stetigventil über Sollwertkarte und Verstärker.

## 1.29.0 – 16.11.2026

Simulations-Roadmap R07/R09 (Stetig- und Lastventile) und Hauptrepo nachgezogen:

- Hauptrepo (QElectroTech upstream) mit 181 Commits eingearbeitet; der Gerätenavigator kommt jetzt von Upstream
  (Lazy Loading, Strukturkästen, Zustand beim Neuladen), Fork-Funktionen bleiben erhalten (`--check-elements --grid`
  neben EPLAN-Import und Portalen, Symboltausch und verschobener Einfügepunkt).
- 4/3-Wege-Stetigventile (FluidSIM `DirectionalValve43_01/_02`): Schieber als PT1 zum Sollwert ±10 V, getrennte
  Überdeckung der P- und T-Kanten (auch Unterdeckung), Rechteck- oder Dreieckkerbe, Widerstand je voll offener Kante,
  Leckage je Kante. Sollwert als Kennwert `u` oder Ereignis `--event "0.5 -QM1 sollwert 5"`; der analoge Signalpfad
  (S, Y1/Y2) kommt mit R06. Messgrößen `u:` (Sollwert) und `s:` (Schieberweg).
- Senkbremsventil (öffnet bei (p_P − p_T) + Flächenverhältnis·(p_X − p_T) ≥ p₀, Rückschlag T→P) und
  Druckzuschaltventile 01/02 (öffnen P→A ab Steuerdruck an X).
- Neue Prüfung „Stetig-, Senkbrems- und Zuschaltventile (R07/R09)“ mit den Plänen `stetig.qet` und `lastventil.qet`:
  halb offen in beiden Richtungen 26,91 bar, Sättigung 6,73 bar, Überdeckung 25 % (zu bei 2 V, x = 1/3 bei 5 V),
  Unterdeckung −20 %, Dreieckkerbe, Leckage 19,33 bar, Sprungantwort s(τ) = 0,6321, Senkbremsventil mit/ohne
  Steuerdruck 11,22/42,52 bar, Zuschaltventil 34,38 bar – alles wie die Handrechnung. 30 Prüfungen, alle bestanden.
  Formelregister V05–V08.
- FluidSIM-Batch: 1051 von 1136 Beispielen starten (vorher 1020), in 285 bewegen sich Zylinder (vorher 269).

## 1.28.0 – 16.11.2026

Simulations-Roadmap R05 (Leitungen, Kupplungen, Aufbereitung):

- Leitungen und Schläuche (`connection.line`, `supply.hose_line`) mit Modellwahl: ohne Angaben ideal (wie FluidSIM bei
  einfachen Verbindungen), mit `rT`/`qNN` als Widerstand (FluidSIM-Schlauchleitung), mit `length` und `d_inner` als Rohr:
  Hagen-Poiseuille laminar, Darcy mit Haaland-Reibzahl turbulent, Übergang 2000–4000 stetig, beide Richtungen, Volumen
  der Leitung an den Enden; Öl HLP 22 (22 mm²/s, 870 kg/m³) oder `nu`/`rho`, Luft isotherm bei Mitteldruck.
- Schnellkupplungen (`connection.quick_coupling`) mit Zustand `coupled` und `seal`: gekuppelt ideal oder mit Verlust,
  getrennt selbstdichtend (beide Hälften zu) oder offen (beide Hälften entlüften). Die Symbole „Schnellanschluss/
  Schnellkupplung offen“ der Sammlung sind jetzt getrennt; vorher verbanden sie fälschlich beide Hälften.
- Drehdurchführung: Anschlüsse gleicher Höhe bilden je einen Kanal; vorher flossen alle Kanäle in ein Netz.
- Verstopftes Bauteil der Aufbereitung (`clogged`, wie FluidSIM-Fehlermodell „Bauteil verstopft“): `qNN_clogged`
  (Vorgabe 10 l/min) bzw. `rT_clogged`.
- Messgröße `q:` jetzt auch an Leitungen und Vakuumerzeugern.
- Neue Prüfung „Leitungen, Kupplungen, Aufbereitung (R05)“ mit den Plänen `rohr.qet` und
  `kupplung_drehdurchfuehrung.qet`: Rohrgesetz gegen Hagen-Poiseuille und Darcy/Haaland (ungerade, passiv, stetig,
  steigend), Rohr laminar 1,508 bar und turbulent 6,045 bar wie die Handrechnung, Schlauch rT 0,12 → 16,15 bar,
  Kupplung gekuppelt/getrennt dicht/getrennt offen, Filter verstopft 26,91 bar, Drehdurchführung Kanal 2 drucklos.
  29 Prüfungen, alle bestanden. Formelregister L01–L05 mit Belegen (Haaland 1983, ISO 3448, ISO 1219-1).

## 1.27.1 – 15.11.2026

Formelbelege (zweite Fassung des Roadmap-Pakets mit Quellenregister):

- `tools/simulation/FORMELN.md` vollständig: jede Formel des Kerns mit Beleg und Fundstelle, Bezugszustände,
  Herleitungen der eigenen Ableitungen, bekannte Abweichungen von FluidSIM, Befunde der Quellenprüfung, alle
  verwendeten Primärquellen (Paket S01–S35, eigene QS01–QS12: ISO 6358-1 Amd 1, Festo, DIN 1343, FluidSIM-Hilfe,
  SMC, Finder, MathWorks). Codestellen tragen die Register-ID.
- Korrektur nach der FluidSIM-Hilfe (aus ihr stammen alle Kennwerte): kritisches Druckverhältnis b = 0,4 statt 0,3,
  Normdichte 1,1845 kg/m³ (technischer Normzustand 20 °C, 1 bar), Luftverbrauch des Ejektors bei 600 kPa. Kern und
  Python-Prototyp gleich geändert (Gegenprobe 0 Abweichungen); pneumatische Zeiten verschieben sich um wenige
  Millisekunden (Blindstopfen 0,262 → 0,264 s, R10 bei 3,5 bar −0,555 → −0,557 bar). 28 Prüfungen, alle bestanden.

## 1.27.0 – 15.11.2026

Simulations-Roadmap R10 (Vakuum, Schwenk- und Greiferantriebe):

- Vakuumerzeuger/Ejektor (`vacuum.vacuum_generator`, FluidSIM `VacuumGenerator_05H/_05L`): Versorgung 1, Vakuum 2,
  Luftverbrauch `qn_ac` aus der Versorgung, lineare Saugkennlinie zwischen dem besten Vakuum `pu_min` (bzw. `p_vac`)
  und der Atmosphäre (`qns_max`), beide mit dem Versorgungsdruck bis zum Optimum `dp_max` skaliert; ohne Versorgung
  derselbe Weg zurück an die Luft (eine stetige Kennlinie).
- Sauger (`vacuum.suction_cup`, FluidSIM `VacuumSuctionCup`): Volumen, Haltekraft (p_atm − p)·π d²/4 gegen s·m·g,
  Leckage `qLeak` bei −0,6 bar, ohne Werkstück (`object` 0) offen; Meldungen „hält das Werkstück“ / „hält nicht mehr“,
  Messgrößen `Fhold:` und `hold:`.
- Vakuumschalter (`vacuum.vacuum_switch`, Druckschalter mit negativem p0): schaltet unter p0 ein, über p0 + p_hyst aus.
- Schwenkantrieb (`actuator.semi_rotary`, FluidSIM `SemiRotaryMotor2`) als Drehanalogon des Zylinders: Schluckvolumen je
  Radiant M_6bar/6 bar, Winkel in rad (Anzeige in Grad), Trägheit, viskose Reibung, Lastmoment, harte oder elastische
  Anschläge; `x:`/`v:`/`F:` als Winkel, Winkelgeschwindigkeit, Moment. Der Import übernimmt die eingebauten Sensoren
  (label1–3, x11–x32) als Wegmarken in Grad (Sensor ±1°).
- Greifer (`actuator.gripper`) als dokumentiertes Ersatzmodell: einfachwirkender Zylinder mit Rückstellfeder.
- Neue Prüfung „Vakuum und Schwenk-/Greiferantriebe (R10)“ mit eigenem Plan `referenz/plaene/vakuum.qet`: −0,800 bar bei
  6 bar, −0,555 bar bei 3,5 bar (Skalierung), Haltekraft 25,13 N, Leckage im Gleichgewicht (−0,562 bar), ohne
  Werkstück offen, 2 kg hält ab m·g, mit s = 1,5 nicht, Vakuumschalter genau an seinen Schaltpunkten, Loslassen,
  Schwenkantrieb 90° mit 2,000 N·m in der Endlage, steht gegen 3 N·m Last, Greifer schließt. 28 Prüfungen, alle bestanden.
- Formelregister `tools/simulation/FORMELN.md`: jede Beziehung des Rechenkerns mit Codestelle, Herkunft (Literatur,
  eigene Ableitung, Modellnäherung), Beleg (Quellenregister des Roadmap-Pakets bzw. ISO 6358/8778), Grenze und Prüfung;
  gewählte Vorgabewerte ausdrücklich als Projektdefinition, nicht als Messwert.
- FluidSIM-Batch: 1020 von 1136 Beispielen starten (vorher 989).

## 1.26.0 – 15.11.2026

Simulations-Roadmap R08 (Pumpen und Hydromotoren):

- Hydromotor mit Welle (`motor.fixed_motor`, `motor.variable_motor`, FluidSIM `actuator.rotary_motor`): Schluckvolumen,
  Verstellgrad α (−1 … 1), Leckage A→B, Trägheit, viskose und geglättete trockene Reibung, Lastmoment;
  q = α·D·ω/2π + C·Δp, τ = α·D·Δp/2π, J·dω/dt = τ − τ_Last − τ_Reibung (`SimPhysics::Shaft`, von Modell und
  Prüfstand gemeinsam benutzt). Druckluftmotoren werden weiter abgewiesen.
- Pumpen mit Verstellgrad α und Leckage P→T (`GL`); Verstellpumpe und FluidSIM-`Pump01` jetzt mit Modell.
- Messgrößen `omega:`, `phi:`, `tau:`, `P:` (Motor) und `P:` (abgegebene Pumpenleistung).
- Reproduzierbarkeit: das Modell wird in fester Reihenfolge der Bauteile aufgebaut (QElectroTech liefert die
  Elemente eines Blatts nicht bei jedem Laden gleich); vertauschte gleichzeitige Eingaben ergeben eine bytegleiche CSV.
- Neue Prüfung „Hydromotor (R08)“: 15 l/min → 1500 1/min, Druck = 2π·(Last + Reibung)/D, Lastsprung +100 bar bei
  gleicher Drehzahl, Umkehr über B −1500 1/min, Leistungsbilanz Δp·q = Last + ΔEkin + Reibung + Leckage (Rest
  0,028 J von 2780 J), Projekt: Motordrehzahl = Förderstrom/Schluckvolumen. 27 Prüfungen, alle bestanden.

## 1.25.0 – 15.11.2026

Simulations-Roadmap R03 (Anfangszustand, Numerik, Ereignisse):

- Widersprüchliche Netze werden vor dem Lauf gemeldet: zwei Bauteile halten ein Netz auf verschiedenen festen
  Drücken (mit beiden Kennzeichen und Drücken), Luft- und Ölanschlüsse oder Fluid- und elektrische Anschlüsse in
  einem Netz.
- Schwingungswächter je Netz: schwingt ein Druck von Schritt zu Schritt ohne abzuklingen (expliziter Schritt zu groß
  für Netzvolumen und Widerstand), bricht der Lauf nach 200 Schritten mit Netz und Hinweis ab, statt Unsinn zu
  rechnen (z. B. rT = 1e-4 bar·min²/l² am Hydraulikventil).
- Ereignisse zur selben Zeit behalten die Reihenfolge der Eingabe (stabile Sortierung); gleichzeitige Eingaben
  ergeben in jeder Reihenfolge denselben Lauf. GRAFCET-Entwicklung bleibt auf 100 Runden begrenzt, alle Schleifen
  enden bei Fehlern.
- Neue Prüfung „Netze, Stabilität, Ereignisse (R03)“; `--simulate-reference`: 26 Prüfungen, alle bestanden.

## 1.24.0 – 15.11.2026

Simulations-Roadmap R02 (Endanschlag und Drucktransienten), geprüft gegen OpenModelica 1.26.3 (jetzt installiert):

- Elastischer Endanschlag im Zylindermodell: Federsteifigkeit `stop_k` [N/m] und einseitige Dämpfung `stop_c`
  [N·s/m] wie das Kontaktmodell der Referenz (Dämpfung wirkt nur beim Eindringen, kann also nur Energie
  entziehen); ohne die Kennwerte bleibt der harte Anschlag. Ein zu weicher Anschlag, der den Kolben durch das
  Totvolumen drückt, bricht den Lauf mit Meldung ab.
- Feiner Referenzlauf `OwnHydraulics.Examples.CylinderCycle_fein.csv` (10 µs, DASSL 1e-9) mit Erzeuger
  `erzeuge_hydraulik_fein.py` und Herkunftsdatei; Referenzpaket-Eigenprüfung (Integrität, Generator, Hydraulik)
  mit omc bestanden.
- Neue Prüfung „Endanschlag hydraulisch (R02)“: Spitze 51,01 bar (Ref. 51,01) zur selben Zeit (±0,02 ms),
  Eindringung 0,0224 mm (Ref. 0,0226), Kontaktarbeit −0,444 J (Ref. −0,459), Kontaktbilanz bis 0,1 mJ, gleiche
  Spitze bei halbem Schritt. Die 10-ms-Ausnahme nach dem Anschlag im hydraulischen Vergleich ist entfernt.
- Schlauchvergleich verbindet die Abtastpunkte der Gegenkurve (vorher galt ein schneller Abfall zwischen zwei
  Punkten fälschlich als Abweichung); die Gegenprobe mit einer eingeschmuggelten Druckspitze fällt weiter durch.
- `--simulate-reference`: 25 Prüfungen, alle bestanden.

## 1.23.0 – 15.11.2026

Simulations-Roadmap (externer Auftrag, Pakete R00 und R01):

- **R00 Modellkatalog** (`simulationcatalog.cpp`, `--simulate-catalog`, Katalog 1.0): jede der 98 bekannten
  Fluidfunktionen mit Rolle (physical, ideal_connection, display, graphic, unsupported), Modell, Anschlüssen,
  Kennwerten, Grenzen und abdeckenden Prüfungen. Das Modell kommt nur noch aus dem Katalog, nicht aus der Art:
  unbekannte Quellen sind keine Druckquellen mehr, Greifer und Schwenkantriebe keine Linearzylinder; Bauteile
  ohne Modell oder mit unbekannter Funktion verhindern den Start mit Kennzeichen und Funktion. Leitungen,
  Kupplungen und Verteiler sind ideale Verbindungen, Messgeräte Messstellen; Aufbereitung als Durchgangswiderstand.
  Prüfung „Modellkatalog“: Vakuumerzeuger, Greifer und unbekannte Funktion abgewiesen, Grafikbaustein ignoriert,
  alle 437 Fluidsymbole der Sammlung (43 Funktionen) entschieden. FluidSIM-Beispiele: 989 von 1136 starten, 147
  werden mit genanntem fehlendem Modell abgewiesen (vorher liefen sie mit erfundener Ersatzphysik).
- **R01 Messgrößen**: `SimulationModel::signalList()/signalValue()`, CLI `--signals`, `--measure`, `--raster`,
  `--meta`; Drücke mit Bezug, gerichtete Durchflüsse je Bauteilanschluss, Zylinderweg, -geschwindigkeit,
  -druckkraft, Ventilstellung; Laufidentität und Status (Programm, Katalog, Projekt-SHA-256, Eingaben,
  Schrittweiten, Endzeit, Fehler). Prüfung „Messgrößen“: Abgriff ohne Rückwirkung, Ventilbilanz ≤ 1e-19 m³/s,
  Pumpe = Verdrängung · Drehzahl, unbekannte Kennung ohne Wert. MCP `qet_simulate`: `measure`, `raster`, `meta`,
  `signals`, Status mit Endzeit.
- `--simulate-reference`: 24 Prüfungen, alle bestanden.

## 1.22.0 – 15.11.2026

FluidSIM-Schaltungen werden simulierbar übernommen, mit eigener Grafik:

- **Eigene Symbole statt FluidSIM-Grafik** (`tools/fluidsim_import/eigene_grafik.py`): aus der .circ kommen nur
  Anschlusslagen, Ebenennamen, Schaltwege und Kennwerte; gezeichnet wird nach DIN ISO 1219-1 / IEC 60617 (Ventil-
  kästen mit Pfeilen und Sperren, Magnet/Taster/Rolle/Vorsteuerung/Feder, Zylinder, Quelle, Tank, Messgerät,
  Rückschlag-, Drossel-, Drosselrückschlag-, Absperr-, Druck- und Logikventile, Kontakte, Spulen, Leuchten,
  Potentiale; sonst Kasten mit Klassenname). Jeder kann übernehmen, ohne FluidSIM, ohne Festo-Grafik im Projekt.
- **Datei > FluidSIM-Schaltung importieren (.circ)…**, MCP `qet_fluidsim_import`, Prüfung aller Beispiele mit
  `tools/fluidsim_import/fluidsim_batch.py`.
- Übernommen für die Simulation: Schaltwege jeder Ventilstellung (auch gespiegelt), alle Kennwerte mit Einheit,
  FluidSIM-Klassen unter den Namen des Kerns, elektrische Rollen (`sim_role`), Endschalter und Rollenventile aus
  den Wegmarken des Wegmaßstabs, Magnet ↔ Ventil mit Seite, GRAFCET mit GRAFCET-SPS (`sim_io`), ältere Dateien mit
  gepackten Koordinaten, fehlende Kennzeichnungen (-QM, -MM, -SF).
- Simulationskern: `sim_role` vor der Namenserkennung; Wechsler (Relaiskontakt mit drei Anschlüssen);
  Wahlschalter und Handventil mit Rastung; Endschalter und Rollenventile über einen Hubbereich „-MM1 0..2“;
  Spule → Ventil über die Verknüpfung (`valve_side`), Spulenkennzeichen in GRAFCET-Aktionen; GRAFCET-SPS-E/A;
  eigener Nenndruck einer Quelle (`pNom`); viskose Zylinderreibung aus `F_ref`/`v_ref`; Steueranschluss 12 bzw. 14
  bei einseitig vorgesteuerten Ventilen; Ereignisse mit Leerzeichen im Kennzeichen („0.3 Reset PB2 an“).
- Prüfung: alle 22 Referenzprüfungen bestanden, Python-Gegenprobe 0 Abweichungen, 9 Tests der Importbausteine.
  Alle 1136 FluidSIM-Beispiele: Import ohne Fehler (0 fehlgeschlagene Operationen, 0 Modellfehler), 45 mit GRAFCET
  (10 mit offenen Punkten: Teil-GRAFCETs mit gleichen Schrittnummern, Zeit auf Kennzeichen „3s/-BG2“); in 276 von
  461 Schaltungen mit Zylinder bewegt sich einer allein durch Drücken jedes Tasters und Betätigen jedes Ventils
  (die übrigen brauchen eine bestimmte Bedienfolge, Analogtechnik oder Bauteile ohne Modell).

## 1.21.0 – 15.11.2026

Nach dem Freigabetest des Stands fce93f6 (vier offene Punkte, alle bestätigt und behoben):

- Ein ungültiges oder abgebrochenes Modell läuft nicht mehr endlos: `runUntil` hört beim Fehler auf, das
  Simulationsfenster startet ein ungültiges Modell gar nicht und setzt einen abgebrochenen Lauf nicht fort.
- Kennwerte mit festem Wertebereich (Durchmesser, Hub, Massen, Zeiten, Drücke, Volumen, Durchflüsse); Durchmesser
  werden vor dem Quadrieren geprüft (vorher wurde −40 mm zu einer gültigen Fläche), Kolbenstange kleiner als Kolben.
  `rT` nur in `bar.min2/l2`, der Exponent `k` nur ohne Einheit; unlesbare Werte sind Fehler statt stiller Vorgabe.
- Wegeventile: der Schieber öffnet wie ein Stellglied erster Ordnung (Zeitkonstante = Schaltzeit / 3).
- `--simulate-reference`: ganze Kurven im Schlauchvergleich (±5 ms, Druck ±10 % bzw. 1 bar, Weg ±2 mm, in beide
  Richtungen) – eine einzelne Druckspitze in der Referenz fällt jetzt auf. Ausgenommen und mit Abweichung genannt
  werden nur 10 ms nach dem Aufschlag am Anschlag (Referenz elastisch und prellend, unser Anschlag hart).
  Sieben Varianten ungültiger Daten und `runUntil` auf ungültigem Modell als Projekttest.

## 1.20.0 – 14.11.2026

Nach einer externen Prüfung des Stands 5be3d74 (sechs Befunde, alle bestätigt und behoben):

- Öl unter der Druckuntergrenze geht nicht mehr verloren: der fehlende Inhalt wird als Hohlraum bilanziert und beim
  Nachströmen zuerst gefüllt (Meldung „Hohlraum“), auch in Netzen mit Speicher.
- Nicht endliche Werte werden nicht mehr zum Mindestdruck: die Rechnung bricht mit Meldung und Netzbezug ab;
  unmögliche Zylinder-, Speicher- und Pumpendaten verhindern den Start. `--simulate` meldet `Model error:` und endet
  mit Rückgabewert 3, das Simulationsfenster zeigt den Fehler rot.
- Blindstopfen an freien Anschlüssen verschließen (eigenes, dichtes Volumen), Schalldämpfer entlüften über ihren
  Widerstand; vorher waren beide offen.
- Druckbegrenzungsventil mit Eingang 1 und Rücklauf 2 (öffnet nach dem Unterschied, gleicher Strom in den Rücklauf),
  statt eines zufälligen Anschlusses gegen Atmosphäre.
- Kennwerte nach ihrer Einheit (`MPa`, `bar`, `l`, `cm3`, `1/min` …) statt nach der Größe der Zahl (vorher wurde
  `pMax` ab 100 als bar gelesen).
- Wegeventile: der Schieber bewegt sich während der Schaltzeit (alte Wege schließen, neue öffnen) statt zu springen –
  damit stimmen die eingeschlossenen Drücke beim Halten mit OpenModelica überein.
- `--simulate-reference` (22 Prüfungen): Druckvergleiche gegen OpenModelica, geprüfte Referenzdateien (falsche oder
  fehlende Daten = FEHLER, fehlender Ordner = SKIP), Hohlraum- und Negativfälle, Einheiten und sechs Projekte, die
  durch dasselbe `SimulationModel` wie Fenster und CLI laufen (`tools/simulation/referenz/plaene`).
- MCP: `qet_simulate` liefert `model_errors`, `qet_simulate_reference` kennt `SKIP`.

## 1.19.0 – 14.11.2026

- GRAFCET-Symbole neu gezeichnet (gleiche Pfade, `bauteile/grafcet`, Generator `tools/symbole/grafcet_symbole.py`):
  Schrittnummer mittig im 40er-Kasten, Weiterschaltbedingung rechts neben der Transition, Aktionstext mittig im Kasten,
  alle Anschlüsse im 10er-Raster auf der Kante (keine Stummel, keine leeren Platzhaltertexte mehr), Parallelverzweigung
  und Synchronisierung mit Anschluss in der Mitte – gerade Leitungen ohne Knicke. Die Markierung aktiver Schritte passt
  zum neuen Kasten.

## 1.18.0 – 14.11.2026

- Simulation nach dem Referenzpaket 2.0 (Hydraulik) verbessert und gegen dessen OpenModelica-Läufe geprüft:
  Ölwiderstände mit stetig geglätteter Wurzelkennlinie, Rückschlagventile öffnen stetig ab dem Öffnungsdruck,
  Pumpe fördert voll bis 1 bar unter `pMax`.
- Neu gerechnet: Hydrospeicher (Gasgesetz mit Vorfülldruck, Druck implizit aus der Volumenbilanz – vorher als
  feste Druckquelle behandelt), Druckluftbehälter als endliches Volumen, Druckschalter mit Hysterese (Kontakte und
  GRAFCET), Wechsel- und Zweidruckventil, Zeitverzögerungsventil; Warnung bei Unterdruck im Öl.
- `--simulate-reference`: 11 Prüfungen, neu Pumpe, Blende, Hydrospeicher und hydraulischer Zylinderzyklus
  (Geschwindigkeiten auf 0,3 % wie OpenModelica); die Referenzläufe werden mit dem Programm installiert.
- MCP-Server: Werkzeuge `qet_simulate` (Plan simulieren mit Ereignissen, liefert Ereignisse, SPS-/GRAFCET-Variablen,
  Fehler, CSV, Bilder) und `qet_simulate_reference`.

## 1.17.0 – 14.11.2026

- GRAFCET nach IEC 60848: Mit den GRAFCET-Symbolen der Standardsammlung gezeichnete Ablaufpläne laufen in der
  Simulation – Anfangsschritt, Schritt, Transition, Parallelverzweigung und Synchronisierung, ODER-Verzweigung,
  Sprung; kontinuierliche Aktion (auch mit Bedingung), speichernde Aktionen bei Aktivierung, Deaktivierung und
  Ereignis; Schaltregeln der Norm mit gleichzeitigem Schalten und flüchtiger Entwicklung. Weiterschaltbedingungen
  mit SPS-Variablen oder direkt mit Kennzeichen (`-BG1`), Zeiten (`2s/X3`), Flanken (`↑-SF1`), `·`, `∨`, `/`;
  Aktionen schalten SPS-Variablen oder Ventile (`-QM1`, `-QM1/14`). Aktive Schritte bekommen im Plan eine Marke,
  `X1`/`X1.T` stehen in der Variablenliste. Fehler im GRAFCET werden beim Start gemeldet.
- SPS-Ausdrücke mit Flanken `R_EDGE(…)`, `F_EDGE(…)`, `↑`, `↓`.
- `--simulate-reference` prüft zusätzlich die GRAFCET-Schreibweise.

## 1.16.0 – 14.11.2026

- SPS-Programm für Ablaufsteuerungen: Zahlenwerte (`INT`, Startwerte in `VAR`), Vergleiche und Rechnen
  (`= <> < > <= >= + - * / MOD`), `ELSIF`, Schrittketten mit `CASE … OF` (Marken, Listen, Bereiche, `ELSE`),
  Bausteine `R_TRIG`, `F_TRIG`, `SR`, `RS`, `CTD`, Ausgänge `.ET`/`.CV` als Zahl; Fehler mit Zeilennummer.
- Simulationsfenster: Liste *SPS-Variablen* mit den aktuellen Werten (Ein-/Ausgänge, Merker, Schritt,
  Bausteinausgänge); `--simulate` gibt sie am Ende aus.
- `--simulate-reference [ordner]`: Prüfstand für den Rechenkern mit denselben Integrationsschritten
  (`simulationphysics.h`) – Gasbefüllung, Gasladung und Hydraulikladung gegen die analytische Lösung samt
  Konvergenz bei halbem Zeitschritt, SPS-Zähler und Sprachumfang, Zylinderzyklus gegen einen OpenModelica-Lauf
  (`tools/simulation/referenz`): Hubzeiten gleich auf 1 ms, Weg auf 0,13 mm.

## 1.15.0 – 14.11.2026

- Simulation mit SPS: Kanäle lesen und schreiben ihre Adresse aus der E/A-Tabelle, das Programm in Strukturiertem
  Text (IEC-61131-3-Teilmenge mit TON, TOF, CTU, IF) steht an der SPS-Baugruppe und wird im Simulationsfenster
  bearbeitet (*SPS-Programm …*, mit Rückgängig).
- Zähler: Zählersymbol mit Vorwahl im Funktionstext, Kontakte mit gleichem Kennzeichen, Rücksetzen über `…/R`.
- Taster und Wegeventile lassen sich während der Simulation direkt im Plan bedienen (Maustaste halten).
- `--simulate … --png ordner --at t`: Bilder der Blätter mit Simulationsanzeige; Zustände (Zylinderweg,
  Ventilstellung, Relais, Zähler, SPS-Ein-/Ausgänge) stehen rechts neben dem Symbol.

## 1.14.0 – 14.11.2026

- Simulation von Pneumatik, Hydraulik und elektrischer Steuerung (Menü *Anzeige → Simulation*): Leitungen unter
  Druck werden eingefärbt, stromführende Drähte rot, Zylinderweg und Ventilstellung stehen am Symbol; Taster und
  Ventile werden im Fenster bedient, Echtzeit oder Zeitlupe. Das Projekt bleibt unverändert.
- Gerechnet wird aus den Simulationsdaten der Symbole: Druckluft nach ISO 6358 (qNN), Öl mit Widerstand rT und
  Kompressionsmodul, Wegeventile mit Schaltwegen, Schaltzeiten, Feder und Steuerdruck; Rückschlag-, Drossel-,
  Drosselrückschlag-, Absperr-, Schnellentlüftungs-, Druckbegrenzungs- und Druckregelventil, Stromteiler,
  Schalldämpfer; Druckluftquelle, Pumpe mit Maximaldruck, Tank; Zylinder mit Kraftbilanz, Reibung, Feder,
  Endlagen. Elektrik: Spannungsquelle, Taster, Endschalter, Relais mit verknüpften Kontakten, Zeitrelais,
  Ventilspulen (Zuordnung über Kennzeichen, siehe `tools/simulation/README.md`).
- `--simulate plan.qet --event "0.2 -SF1 an" --duration 2 --csv weg.csv`: dieselbe Simulation ohne Oberfläche.
- Rückschlag- und Drosselrückschlagventile tragen ihre freie Durchflussrichtung (`free_from`/`free_to`, aus der
  Zeichnung gelesen).

## 1.13.0 – 14.11.2026

- Pneumatik und Hydraulik: Die Symbole der QElectroTech-Standardsammlung (437, Grafik unverändert) tragen
  Simulationsdaten und ersetzen die eigenen ISO-1219-Symbole aus 1.12.0. Jedes Symbol hat benannte Anschlüsse
  mit Medium, Art, Funktion, Kennbuchstaben und Kennwerte (Startwerte); alle 172 Wegeventile und 30
  Ventilbausteine haben Stellungen, Schaltwege, Ruhestellung und Betätigung, gelesen aus der Zeichnung
  (`tools/symbole/qet_fluid_daten.py`, Ablage `bauteile/fluid` mit den Pfaden der Standardsammlung).
- Fluiddaten: `<fluid function="…">` und `<parameter name value unit>` für Kennwerte (`FluidData`).
- `--check-elements` liest den `<fluid>`-Block mit dem Fluidmodell und meldet Schaltwege auf unbekannte
  Anschlüsse.
- Werkzeuge für FluidSIM-Bibliotheken (nur zur eigenen Verwendung, nicht im Fork abgelegt):
  `fluid_raster_einpassen.py` (Anschlüsse aufs Raster ohne Haken), `fluid_schaltwege.py`, `fluid_kennwerte.py`,
  `fluid_katalog.py`.

## 1.12.0 – 13.11.2026

- Pneumatik und Hydraulik: die eigenen, neu gezeichneten Symbole nach ISO 1219 (322 Bauteile) ersetzen die
  generierten Symbole aus 1.11.0; der Generator `tools/symbole/iso1219_symbole.py` entfällt.
- `tools/deploy_lokal.ps1` erzeugt keine FluidSIM-Symbole mehr und räumt frühere Stände aus der
  Standardsammlung, sodass dort nur die eigenen Symbole stehen.

## 1.11.1 – 13.11.2026

- macOS: die Klemmen-Vorschau kompiliert mit Clang (mehrdeutiger Name `TerminalStripDrawer`); der Mac-Build
  in GitHub Actions meldet alle Fehler auf einmal.

## 1.11.0 – 13.11.2026

- Eigene Pneumatik- und Hydrauliksymbole nach ISO 1219-1 (143 Bauteile), fest in der Standardsammlung unter
  `50_pneumatic/5095_ISO1219` und `30_hydraulic/3095_ISO1219`: Wegeventile 2/2, 3/2, 4/2, 5/2 und 5/3 (Mitte
  gesperrt, entlüftet, belüftet) mit Magnet, Steuerdruck, Taster, Hebel/Raste, Pedal, Rolle, Leerrücklaufrolle,
  Stößel und Hand; hydraulische 4/2- und 4/3-Wegeventile (Mitte gesperrt, Umlauf, Schwimmstellung, offen);
  Zylinder (einfach-/doppeltwirkend, Dämpfung, durchgehende Stange, Magnetkolben), Motoren, Pumpen, Verdichter,
  Rückschlag-, Drossel-, Drosselrückschlag-, Wechsel-, Zweidruck- und Schnellentlüftungsventil,
  Druckbegrenzungs-, Druckregel- und Zuschaltventil, Quelle, Speicher, Tank, Filter, Abscheider, Öler,
  Trockner, Wartungseinheit, Kühler, Manometer, Druckschalter, Schalldämpfer, Elektromotor und Ventilspule.
- Gezeichnet mit `tools/symbole/iso1219_symbole.py` aus den Grundelementen der Norm, ohne fremde Geometrie.
  Jedes Bauteil trägt Medium, Portnummer und Abschluss je Anschluss, Kennbuchstaben (IEC 81346-2) und die
  Schaltstellungen; Magnetventile sind Master ihrer Ventilspule. Die FluidSIM-Umwandlung bleibt ein
  Werkzeug für Besitzer einer FluidSIM-Lizenz, ihre Symbole kommen nicht ins Repository.

## 1.10.0 – 13.11.2026

### Klemmleiste zeichnen: Zuordnung je Leitung

- Jede gekreuzte Leitung bekommt im Dialog ihre eigene Zeile mit Leiste, Klemmennummer, Etage und Artikel.
  Leitungen mit gleicher Leiste und Nummer teilen sich eine Mehrstockklemme. „Eigene Leiste“ legt die
  ausgewählten Leitungen auf die nächste freie Leiste (z. B. N auf X2, PE auf X3).
- PE kommt nur bei Klemmen mit PE-Etage auf die Klemme (Typ „PE/L/N“ oder Schutzleiterklemme); eine
  Etagenklemme ohne PE (z. B. PTI 2,5-L/N) gibt PE-Leitungen eine eigene Klemme mit dem PE-Artikel.
- Etagen heißen PE, L, N, wenn der Typ sie nennt; Leitungen ohne Nummer zeigen ihre Enden; die gewählte
  Leitung wird im Plan markiert; die Vorschau zeigt die Leiste der gewählten Zeile.

### macOS

- Herstellerportal-Konten im macOS-Schlüsselbund (unter Windows weiter in der Anmeldeinformationsverwaltung).
- Aus dem Finder gestartet findet QElectroTech Python, 7-Zip und Claude Code aus Homebrew, MacPorts und
  `~/.local/bin`.
- Claude-Assistent und EPLAN-Projektimport werden im App-Bundle (`Contents/Resources`) gefunden und von
  `misc/MacQetDeploy_arm64_cmake.sh` mitgeliefert; Open CASCADE (STEP-Modelle) wird eingebunden, wenn installiert.
- EPLAN-Projektimport: 7-Zip als `7z`, `7zz` oder `7za`; QElectroTech gibt den Schriftfeld-Ordner mit.
- qet-mcp liest das Data-Portal-Token auch aus den macOS-Einstellungen.
- Neuer GitHub-Workflow `macos-build.yml`: Mac-Build mit 3D-Ansicht, Open CASCADE und 3D-Maus.

### Pneumatik und Hydraulik nach Vorbild FluidSIM 6 (Stufe 1: zeichnen, noch keine Simulation)

Plan: `PLAN-PNEUMATIK.md`.

- Anschlüsse tragen ein Medium (elektrisch, pneumatisch, hydraulisch, mechanisch, Marke) und einen Abschluss
  (keiner, Schalldämpfer, Blindstopfen); beides im Anschluss-Editor. Eine Leitung verbindet nur Anschlüsse
  mit gleichem Medium. Elektrische Anschlüsse und alte Dateien bleiben unverändert.
- Leitungsart im Leitungs-Fenster: Arbeitsleitung, Steuerleitung, Abluft/Leckleitung (`fluid_line`).
- Schalldämpfer und Blindstopfen werden am freien Anschluss gezeichnet (Form wie in FluidSIM) und verschwinden,
  sobald eine Leitung angeschlossen ist.
- Spule ↔ Ventil über Master/Slave: Ventil = Master (`<fluid kind="valve">`), Ventilspule = Slave
  (`<fluid kind="solenoid"/>`). Der Querverweis am Ventil zeigt die Spule mit dem Magnetsymbol nach ISO 1219.
- Projekt → „Fluidtechnik prüfen…“: offene pneumatische/hydraulische Anschlüsse ohne Leitung und Abschluss,
  Magnetventile ohne verknüpfte Spule, Spulen ohne Ventil. Ein Klick wählt das Bauteil auf seinem Blatt.
- Beispielbauteile in `bauteile/pneumatik/`: 5/2-Magnetventil mit Federrückstellung (Master, Schalldämpfer an
  3 und 5) und Ventilspule (Slave).
- `tools/symbole/fluidsim_symbole.py` wandelt die Symbolbibliothek einer FluidSIM-6-Installation in
  QET-Bauteile um (150 Pneumatiksymbole): Grafik in Ruhestellung, Anschlüsse mit Portnummer, Medium und
  Richtung, Schalldämpfer/Blindstopfen der Vorlage als Abschluss, deutsche Namen aus FluidSIM, Magnetventile als
  Master, Ventilspule als Slave.
- `tools/symbole/fluid_nachruesten.py` rüstet die QET-Sammlung `50_pneumatic` (und `30_hydraulic`) nach:
  Medium an jedem Anschluss, Portnummern aus den gezeichneten Texten (alle 113 Wegeventile vollständig).
  Ohne Nachrüsten lassen sich diese Bauteile nicht mit pneumatischen Anschlüssen verbinden.
- FluidSIM-Schaltungen übernehmen: `tools/fluidsim_import/fluidsim_plan.py SCHALTUNG.circ AUSGABE.qet` macht aus
  einer FluidSIM-6-Schaltung ein QET-Projekt: Bauteile mit Lage (Drehung, Spiegelung), Kennzeichnung, Leitungen
  (Verzweigungen werden zu Leitungen am selben Anschluss), Schalldämpfer/Blindstopfen, Spule ↔ Ventil verknüpft.
  Mehrere .circ (oder ein Ordner) werden mehrere Blätter. Bauteile tragen den deutschen Namen aus FluidSIM,
  Beschreibung und technische Werte als Bauteilinformation, die sichtbaren Werte (p = 6 bar …) als Text,
  Zylinder Durchmesser, Hub und Bauart. Relaisspulen und ihre Kontakte (Schließer, Öffner, Wechsler) werden
  verknüpft, Ventile mit mehreren Magneten mit allen Spulen, auch über Blattgrenzen. Rahmen, Tabellen,
  Zustandsdiagramme und GRAFCET bleiben weg; Artikeldaten speichert FluidSIM nicht. Alle 1072 Beispiele
  von FluidSIM (Pneumatik, Elektrik, Hydraulik) lassen sich umwandeln.
- Die FluidSIM-Hydraulikbibliothek (172 Bauteile) kommt beim Deploy nach `30_hydraulic/3090_FluidSIM`.
- Kennbuchstaben beim Einfügen (81346-2) für Pneumatik und Hydraulik: ein Fluid-Bauteil kann seinen Vorschlag
  mitbringen (`<fluid prefix="QM">`), er gilt, wenn `qet_labels.xml` für den Ordner keinen hat. Die Buchstaben
  folgen der Kennzeichnung in FluidSIMs Beispielen (Zylinder MM, 5/2-Ventil QM, Spule MB, Drossel RZ …);
  die QET-Ordner `50_pneumatic`/`30_hydraulic` bekommen beim Deploy eine `qet_labels.xml`, wenn sie keine haben.
- Skill `fluidtechnik` (`.claude/skills/fluidtechnik/`): Datenmodell, Bauteile, Kennbuchstaben, Zeichnen per
  MCP, FluidSIM-Import und -Format, Fallen.
- `tools/deploy_lokal.ps1` ersetzt in der Standardsammlung `50_pneumatic` und `30_hydraulic` durch die
  nachgerüsteten Bauteile und legt die FluidSIM-Symbole unter `50_pneumatic/5090_FluidSIM` ab (wenn FluidSIM 6
  installiert ist). Doppelte Portnummern (zwei Eingänge „1“ am Wechselventil) heißen als Anschluss „1.1“/„1.2“,
  gezeichnet bleibt „1“. UUIDs sind bei jedem Lauf gleich.

## 1.9.1 – 13.11.2026

- qet-mcp: Syntaxfehler in der Beschreibung von `add_terminals` behoben (der MCP-Server startete seit 1.9.0 nicht).
- Abgleich mit dem Original-QElectroTech (Stand 13.11.2026): Strukturkästen, Ansichten für Betriebsmittel, Kabel,
  Klemmen, Potentiale und SPS-E/A, Artikel-Register am Bauteil, PDF-Ränder unter Windows u. a.
- Kabelliste: die Korrektur „Kabelende an der Klemmleiste“ kommt jetzt aus dem Original.

## 1.9.0 – 13.11.2026

- Neues Werkzeug „Klemmleiste zeichnen“ (Werkzeugleiste neben „Kabel zeichnen“): ein Strich quer über Leitungen
  setzt auf jede Leitung eine Klemme, trennt die Leitung dort auf und behält Drahtnummer und Farbe auf beiden
  Seiten. Der Vorschlag (Leiste, erste Nummer, Symbol, Artikel, PE-Artikel) lässt sich vor dem Einfügen ändern;
  die Klemmen kommen als `X1:n` in die Klemmleiste. Skript `qet.addTerminals(...)`, MCP-Operation `add_terminals`.
- Klemmenkatalog mit Filter (Hersteller, Art, Querschnitt, Anschlussart), maßstäblicher Vorschau, Produktfoto
  und Suche im EPLAN Data Portal. Eigene Kataloge als CSV in `<Datenordner>/terminal-catalogs/`.
- Mehrstockklemmen: eine Klemme mit mehreren Etagen (z. B. L/N/PE) nimmt mehrere Leitungen auf; die Etagen werden
  in der Klemmleiste zu einer Klemme zusammengefasst.
- Kabelliste: alle Klemmen einer Klemmleiste zählen als ein Kabelende (`X1` statt `X1:1` → `X1:2`).

## 1.8.0 – 12.11.2026

- Herstellerportale: Keyence. Benutzername und Passwort für keyence.de im Windows-Anmeldeinformationsspeicher
  (Dialog Herstellerportale, `--portal-status`, `--portal-login keyence`, `--portal-logout keyence`).
  keyence.de meldet nur über die eigene Webseite an, QElectroTech sendet das Passwort nicht.
- `--fetch-parts` übernimmt den CAD-Download aus dem Browser: ZIP oder STEP in `herstellerdaten/Keyence/<Nr>/`
  wird entpackt und als `3D/Keyence/<Nr>.stp` abgelegt (STEP aus ZIP-Archiven auch für andere Hersteller).
  Fehlt die Datei, nennt es den Suchlink auf keyence.de und den Zielordner. Herstellerkürzel `KEY`.

## 1.7.1 – 11.11.2026

- EPLAN-Import: symbolische Adressen wie EPLAN (Zielverfolgung bis Sensor/Aktor bzw. Steckerklemme ohne
  Weiterleitung, vollständiger Text; nur für SPS-Anschlüsse mit SPS-Adresse).
- EPLAN-Import: Herstellername (22222) aus dem Herstellersatz, BMK am SPS-Kanal nur, wo EPLAN es sichtbar gespeichert hat.
- EPLAN-Import: Pfad-Funktionstext über den Pfad (Spalte des Normblatts) statt über einen Abstand; Übersichten
  zeigen den Funktionstext des Gegenstücks.
- EPLAN-Import: Normblatt-Platzhalter ohne Zeilen, die unter den Blattrand laufen (wie EPLAN).

## 1.7.0 – 11.11.2026

- EPLAN-Import: Bilder aus Formularen (z. B. Firmenlogo auf dem Deckblatt) und Bilder mit der alten Pfadschreibweise
  `$IMG\…` kommen mit (Testprojekt: 201 Bilder, 0 nicht auflösbar).
- EPLAN-Import: Linienbreite wie in EPLAN (eigene Breite, sonst Breite der Ebene) für Blattgrafik und
  Bauteilgrafik; dicke Balken und Rahmen sind wieder dick.
- EPLAN-Import: fette und kursive Texte (Textformat 60965) bleiben fett/kursiv.

## 1.6.0 – 11.11.2026

- Claude-Assistent: Bilder und PDFs mitschicken. Ins Eingabefeld einfügen (Strg+V, z. B. ein Bildschirmfoto),
  Dateien hineinziehen oder mit der Büroklammer wählen; die Anhänge stehen als kleine Knöpfe über dem Feld und
  verschwinden per Klick wieder. Bilder werden auf höchstens 2000 px verkleinert (Fotos als JPEG), PDFs bis
  20 MB, höchstens 10 Anhänge je Frage. Geht mit API-Schlüssel und über das Claude-Konto.

## 1.5.0 – 11.11.2026

- EPLAN-Import: Auch Steuerungen anderer Hersteller (z. B. Cybelec) werden SPS-Master. Ihre Kanaladresse steht in
  EPLAN oft nicht als SPS-Adresse, sondern in der Anschlussbezeichnung bzw. -beschreibung (DO5.8, DI1.2); der
  Import liest sie von dort.
- EPLAN-Import: Funktionstexte der Kanäle (z. B. „Hand von CNC“) kommen mit – eigener Text, sonst der
  Pfad-Funktionstext wie in EPLAN – und stehen in der Kanaltabelle der Karte.
- Adresse, symbolische Adresse und Funktionstext eines verknüpften Kanals kommen nur aus der Kanaltabelle der Karte:
  Beim Wechsel auf einen anderen Kanal wandern die Texte mit.
- Nach jedem Import liegt `sps_bericht.txt` im Arbeitsordner: welche Kanäle nicht verknüpft sind und warum.

## 1.4.0 – 11.11.2026

- Claude-Assistent über das eigene Claude-Konto (Pro/Max) statt eines API-Schlüssels: keine API-Kosten, die
  Nutzung zählt gegen das Abo. Die Einrichtung hat dafür einen Knopf „Mit Claude verbinden“: Er installiert das
  Python-Paket `claude-agent-sdk` (es bringt sein Claude Code mit) und öffnet die Claude-Anmeldung im Browser.
  Danach wechselt das Fenster zum Chat. Die Brücke startet dazu mit `--konto`; Claude bekommt nur die
  QET-Werkzeuge, keine Datei-, Shell- oder Webwerkzeuge, und ein API-Schlüssel aus der Umgebung wird
  ausgeblendet. „Stattdessen einen API-Schlüssel verwenden“ schaltet zurück.
- Nur in eigenen Builds mit `-DQET_CLAUDE_KONTO=ON` (dort voreingestellt): Anthropic erlaubt Programmen nicht
  ohne Freigabe, die claude.ai-Anmeldung anzubieten. Ohne den Schalter bleibt es beim API-Schlüssel.

## 1.3.0 – 11.11.2026

- EPLAN-Import: SPS-Karten werden SPS-Master mit Kanaltabelle (Adresse, Funktionstext, symbolische Adresse,
  Klemme je Kanal, Art DI/DQ/AI/AQ aus EPLAN), SPS-Kanäle werden SPS-Slaves und sind beim Import schon mit
  ihrer Karte und Kanalzeile verknüpft. Damit lässt sich ein importierter Kanal im Fenster Auswahleigenschaften
  > SPS auf einen anderen freien Kanal legen. Kanäle ohne Karte im Projekt bleiben SPS-Slaves zum Verknüpfen
  von Hand. Versorgungsanschlüsse ohne Adresse (L+, M, 1A …) stehen nicht in der Kanaltabelle.

## 1.2.0 – 11.11.2026

- Neuer Knopf „Claude“ (Symbol und Text) ganz vorn in der Hauptwerkzeugleiste. Er öffnet und schließt den
  Claude-Assistenten rechts. Wer seine Hauptwerkzeugleiste schon angepasst hatte, bekommt den Knopf einmal
  vorn eingefügt; danach lässt er sich wie jeder andere Knopf verschieben oder entfernen.
- Geführte Einrichtung im Assistenten statt des Einstellungsdialogs: API-Schlüssel (mit Link zu
  console.anthropic.com und „Prüfen“, das den Schlüssel mit einer kurzen, kostenlosen Anfrage testet und erst dann
  in der Windows-Anmeldeinformationsverwaltung speichert), Python und das Paket „anthropic“ (Knopf
  „Jetzt installieren“ ruft pip auf und zeigt die Ausgabe). Jeder Schritt bekommt einen grünen Haken oder ein rotes
  Kreuz; sind alle grün, wechselt das Fenster von selbst zum Chat. „Einstellungen“ im Chat führt zurück.

## 1.1.2 – 11.11.2026

- EPLAN-Import: Ein SPS-Kanal ist jetzt EIN Bauteil – Kanalkasten, Kopfzeile (Karte, Typ), Anschluss, Klemme,
  Kanal, Adresse, symbolische Adresse und Funktionstext zusammen. Vorher kam nur ein Anschlusspunkt als Bauteil,
  der Rest als lose Texte und Formen aufs Blatt. Gleiches für Gerätekästen/SPS-Kartenübersichten (Black Box):
  die Texte ihrer Zeilen stehen im Bauteil.

## 1.1.1 – 11.11.2026

- Das Claude-Fenster heißt jetzt „Claude-Assistent“ (Ansicht > Claude-Assistent), das Protokoll des Direktmodus
  „Direktmodus“. Vorher hießen beide „Assistent“ und hatten dieselbe Kennung, sodass QET ihre Plätze beim Start
  vertauschen konnte.

## 1.1.0 – 11.11.2026

- Datei > EPLAN-Projekt importieren zeigt einen grünen Fortschrittsbalken (0–100 %) und den laufenden Schritt.
  Das Import-Programm meldet dazu Zeilen `FORTSCHRITT <prozent> <schritt>`.
- Deutsche Oberfläche vollständig: alle 913 offenen Texte übersetzt (EPLAN-Import, Assistent, Data Portal,
  Gerätenavigator, Brücken u. a.), drei falsche alte Übersetzungen korrigiert.
- `tools/deploy_lokal.ps1` bringt Programm, Übersetzungen und Python-Teile nach build\deploy.
  Vorher kam nur die exe dorthin, deshalb waren neue Fenster englisch.

## 1.0.1 – 11.11.2026

- Die exe trägt wieder das QElectroTech-Programmsymbol (Explorer, Desktop-Verknüpfung, Taskleiste).

## 1.0.0 – 10.11.2026

Erste gezählte Version. Sie fasst alle Fork-Funktionen zusammen, die vorher in einzelnen Branches lagen,
auf dem Stand von QElectroTech 0.200.1-dev.

**Neu in dieser Version**
- Eingebauter Claude-Assistent als Andockfenster (Ansicht > Assistent). Er arbeitet über `mcp/qet-assistent/bruecke.py`,
  der Schlüssel liegt in der Windows-Anmeldeinformationsverwaltung.
- Datei > EPLAN-Projekt importieren (.zw1). Ein Dialog startet den Import und öffnet das Ergebnis.
- SPS-Kanäle: Modul und Kanal im Fenster Auswahleigenschaften wählen, allgemeiner Ein-/Ausgangskanal. SPS-Module
  bekommen ihre Kanaltabelle schon beim Import.
- Die Fork-Version steht im Info-Dialog und bei `--version`.

**Bauteil-Import**
- EPLAN-Makros und EDZ-Pakete: Grafik aus DXF, Schaltplan aus `.ema`, Anschlüsse und Artikeldaten, 2D-Montageaufbau
  als `<Typ>_Aufbau.elmt`.
- DWG-Zeichnungen (AutoCAD 2004–2018) direkt, Artikeldaten aus WSCAD `.wspak`.
- EPLAN Data Portal: Suche, Filter, Bildvorschau, Import, auch per Skript (`importPartFromDataPortal`).
- Herstellerportale: Konten im Windows-Anmeldeinformationsspeicher, fehlende Bauteile automatisch holen
  (`--fetch-parts`).

**Schaltplan wie EPLAN**
- Dreifachsprung am Bauteil und im PDF, Sprungziele für Texte und Bauteile, verknüpfte Folioverweise als Link.
- Gefüllte Folioverweis-Pfeile, Gerätenavigator mit Filterschemata, SPS-E/A-Tabelle, Kontaktgruppen.
- Kabeltyp-Fenster mit Herstellerkatalogen, Typ eines platzierten Elements tauschen.

**Schaltschrankaufbau**
- Montageflächen, Hutschienen, Kabelkanäle, Klemmenleisten, Tür und Seitenwände, Bohrbild, DXF-Fertigungsexport.
- 3D-Ansicht mit STEP-Modellen der Hersteller, glTF-Export, Aufbauprüfung, Aufbau aus EPLAN-Projekten.

**Werkzeuge und Skills**
- MCP-Server `qet_mcp` mit Live-Modus, Skript-API, Data-Portal-Suche.
- Skills `qet-schaltplan-zeichnen`, `eplan-wissen` und `bauteile-herunterladen` unter `.claude/skills/`.
