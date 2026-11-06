# EPLAN-Wissen: Artikelverwaltung, Artikelauswahl, Import/Export, Ersatzteillogik, Data Portal

Zusammenfassung der EPLAN-Plattform-Hilfe 2.9 zur Artikeldatenbank (Struktur, Felder, Zubehör,
Funktionsschablonen, Module), zur Artikel- und Geräteauswahl, zu Import/Export (CSV, Text, XML, EDZ, BMEcat), zur
Logik nicht platzierter Funktionen („Ersatzteile“/Gerätedefinitionen) und zu Data Portal/eServices. Zweck: den
Bauteil-Import im QET-Fork (`sources\import\edz\`, `sources\import\eplandxf\`) an die EPLAN-Datenstruktur anlehnen
und beim Nachbau Artikeldaten richtig zuordnen. Grundlagen (Makro-Zuordnung, Funktionsschablonen-Kurzfassung)
stehen in `makros-artikel.md`; „Vermutung“ markiert eigene Schlüsse, „Fork-Code“ Aussagen aus dem Quelltext.

## Inhalt
1. Grundprinzip: zwei Artikeldatenbanken
2. Baum, Artikeltypen, Varianten, Gewerke
3. Interne Tabellen und Feldnamen
4. Zubehör, Zubehörlisten, Zubehörplatzierung
5. Baugruppen und Module
6. Import/Export: Formate, Konfigurationsdateien (CSV/Text)
7. Kennungen: Datensatztyp, Produktgruppen, Pflege
8. Baugruppen, Funktionsschablonen zuweisen, Funktionsschablonen summieren
9. Registerkarten des Artikels: Regeln mit Wirkung
10. Import, Export, Feldzuordnung, geschützte Felder
11. Artikelauswahl und Geräteauswahl: Prinzip
12. Übertragene Daten, Geräteauswahl-Regeln, Konflikte
13. Funktionspool, Gerätedefinition, nicht platzierte Funktionen („Reservebauteile“)
14. Stammdaten: Einlagerung, Abgleich, Datenablage
15. Pro Panel: Bohrbilder aus Maschinendaten (knapp)
16. BMEcat- und eCl@ss-Import
17. eServices-Dialoge (Format, Klemmen erzeugen, Eigenschaften, Schutz) und Data Portal
18. Ergänzung: Artikelstammdaten-Navigator, Artikel aus Schaltplan erzeugen, Data-Portal-Einstellungen
99. Quellen

---

## 1. Grundprinzip: zwei Artikeldatenbanken

- Wie alle Stammdaten werden Artikel **ins Projekt eingelagert**: es gibt die zentrale Artikeldatenbank und die
  Projekt-Artikeldatenbank mit nur den verbauten Artikeln. Im Projekt wird ein Artikel **einmal eingelagert und
  über die Artikelnummer referenziert** (10 Verwendungen = 1 Datensatz). Abgleich mit der zentralen Datenbank ist
  jederzeit möglich.
- Baum- oder Listenansicht; Liste zeigt nur gemeinsame Eigenschaften (Artikelnummer, Produktgruppe …), keine
  Hersteller/Lieferanten/Kunden. Die **Baumkonfiguration** erlaubt eigene Strukturknoten mit Auswahlkriterien
  (Suche nach technischen Merkmalen statt Artikelnummer).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_artverwnutzen.htm
- Datenbank: Microsoft Access oder SQL Server; mehrsprachige Texte, mehrere Währungen, Preisumrechnung,
  Import/Export von Herstellerdaten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_start.htm

**Bedeutung für QET:** QET lagert Artikeldaten je Element-Instanz in `elementInformations` ein (keine zentrale
Projekt-Artikeltabelle). Ein Nachbau-Skript sollte deshalb pro Artikelnummer **einen** Datensatz (Hersteller,
Bestellnummer, Typ, Bezeichnung) führen und identisch an alle Instanzen schreiben, damit die Stückliste gleiche
Artikel zusammenfasst (Vermutung/Empfehlung).

## 2. Baum, Artikeltypen, Varianten, Gewerke

- Baumebenen: Artikel (konfigurierbar), Zubehörliste, Zubehörplatzierung, Bohrbild, Anschlussbild, Kunde,
  Hersteller/Lieferant. Je Gewerk drei Artikeltypen: **Einzelteil, Baugruppe, Modul**; Kabel und Gehäuse sind
  Einzelteile; nicht zuordenbare Artikel ins Gewerk **Mechanik** (auch Pro-Panel-Daten).
- Neuer Artikel auf Produktgruppenebene heißt `Neu_<n>`, auf Artikelebene `<Artikelnummer>_<n>`.
- **Variante**: ein Zeichen 0–9 oder A–Z; Artikelnummer + Variante eindeutig.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelanlegen.htm
- Kopieren/Einfügen erzeugt `<Nummer>_1`, `_2`; es werden nur Daten übernommen, die der Zieltyp speichern kann.
  Varianten lassen sich nicht einzeln kopieren (immer der ganze Artikel). „Einfügen technische Daten“ und
  „Einfügen Funktionsschablonen“ übertragen gezielt (ersetzen vorhandene). Mehrfachauswahl zeigt nur gleiche oder
  leere Felder und ändert sie global. Artikel lassen sich auch in der Artikelauswahl anlegen, wenn „Änderung während
  Auswahl erlaubt“ aktiv ist.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelbearbeiten.htm
- Gewerke im CSV als Einzelflags: Elektrotechnik, Fluid, Mechanik, Verfahrenstechnik, Hydraulik, Pneumatik,
  Schmierung, Kühlung, Kühlschmiermittel, Gastechnik, Fluid (undefiniert); Produkt-Obergruppe/-Gruppe/-Untergruppe
  ordnen den Artikel ein.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvartikel.htm

**Bedeutung für QET:** Die Fork-Ordnerregel Hersteller/Kategorie/Typ entspricht grob Produktgruppe/-untergruppe.
Der DXF-Paketimport nimmt schon `Product group` aus `commercialdata.csv` als Kategorie (Fork-Code
`eplanpartimporter.cpp`). Für EDZ wäre analog `P_ARTICLE_PRODUCTGROUP`/`…SUBGROUP` aus `part.xml` zu lesen
(Vermutung: Attributnamen nach dem Muster der schon gelesenen `P_ARTICLE_*`, nicht geprüft).

## 3. Interne Tabellen und Feldnamen

Die Hilfe nennt Tabellen und interne Feldnamen der Artikeldatenbank (zugleich CSV-Feldnamen). Wichtigste Auswahl
(max. Länge in Klammern):

| Tabelle | Inhalt | wichtige Felder |
|---|---|---|
| `tblPart` | Artikelstamm | `partnr` (255), `variant` (3), `erpnr`, `typenr`, `manufacturer` (30), `supplier` (30), `ordernr` (255), `description1..3`, `note`, `uniqueid` (50, eindeutige ID für das Data Portal), `producttopgroup`/`productgroup`/`productsubgroup`, `discontinued`, Preise, `identcode` (Strichcode), `certificate*`, Maße `weight/width/height/depth`, `graphicmacro` (Grafikmakro inkl. Verzeichnis), `picturefile`, `ecabinetmacro` (Textur), `groupsymbolmacro` (= Feld **Makro**), `terminal` (Anschlussbild), `reportid` (Kennung für Auswertungen), `externaldocument1..3`, `wear`/`spare`/`maintenance`/`lifetime`/`usage` |
| `tblVariant` | variantenbezogen | `connectiondesignation` (Anschlussbezeichnungen), `technicalcharacteristics`, `assemblyspreaded` |
| `tblFunctionTemplate` | Funktionsschablonen | `pos`, `functiondefid`, `functiondefcategory`, `functiondefgroup`, `connectionDesignation` (255), `connectiondescription` (255), `characteristics` (Kenngröße), `terminalNr` (Feld „Bezeichnung“), `idx1`/`idx2`, `nesteddevicetag` (Steckerbezeichnung), `description` (Klemmenbeschreibung), `variant`, `intrinsicsafety`, `safetyrelevant`, `symbol`, `symbolmacro` |
| `tblFreeProperty` | freie Eigenschaften | `description.n`, `val.n`, `unit.n` (n = 1…1000) |
| `tblAttribute` | Attribute | `val.n` (1…100) |
| `tblAccessoryPosition` | Zubehör am Artikel | `necessary.n`, `partnr.n`, `variant.n`, `parttype.n` |
| `tblAccessoryList(Position)` | Zubehörlisten | `name`, `description`; `partnr.n`, `variant.n`, `placement.n`, `pos.n` |
| `tblTerminal` / `tblTerminalPosition` | Anschlussbild | `name`, `designation`, Standard-Verbindungsende/Zusatzlänge/Richtung; je Anschluss `pos`, `name` (Anschlussbezeichnung), `xpos/ypos/zpos`, `direction`, `additionallength`, `type` |
| `tblConstruction` / `tblDrillingPosition` | Bohrbild | `name`, `designation`, `variant`; je Durchbruch `type`, `subtype`, `outlinename`, `xpos/ypos`, `angle`, `dimension1..3`, Wiederholabstand |
| `tblModule`, `tblAssembly` | Modul-/Baugruppenpositionen | `partnr`, `variant`, `parentvariant`, Modul zusätzlich `devicetag` (BMK) und `moduleid` (BMK-ID, 16) |
| `tblAddress` | Hersteller/Lieferant/Kunde | `shortName` (30), `longName` (70) … |
| `tblGrid` | Dokumente 4–20, Symbole 1–20 | `<Eigenschaftsnummer>.<Index>`, Dokument-Eigenschaften ab **22235** |

- **Achtung-Regel:** `tblFunctionTemplate` wird **nur als Ganzes** importiert/exportiert; fehlt beim Import ein
  Feld (z. B. auskommentiert), wird der vorhandene Wert gelöscht. Sicherheitskennwerte gehen nicht per CSV (keine
  1:1-Beziehung).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_feldlaengenfelder.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvartikel.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvanschluesse.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvkonstruktion.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvzubehoer.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvadressen.htm
- Feldlängen technischer Eigenschaften (Auszug): SPS-Typbezeichnung 20, Gerätebeschreibung: Dateiname 50,
  Adressbereich 40, Klemmen Strom/Spannung IEC/UL/CSA je 8, Kabeltyp 40, Verbindungsanzahl 6, Kabel/Paket:
  Bezeichnung in der Grafik 100, Stecker-Eigenschaften 10–30, TemplateReference 255.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_feldlaengeneigenschaften.htm

**Bedeutung für QET:** Die Attributnamen im EDZ-`part.xml` folgen offenbar denselben Feldern: der Fork liest am
`<functiontemplate>` schon `connectionDesignation`, `connectiondescription`, `terminalNr`, `functiondefinition`
(Fork-Code `edzpart.cpp`). Nach dieser Tabelle lohnen zusätzlich: `functiondefcategory`/`functiondefgroup`
(→ QET-Elementtyp, siehe `funktionsdefinitionen.md`), `idx1`/`idx2` und `nesteddevicetag` (Stecker/Untergerät),
`characteristics` (Kenngröße, z. B. Schaltvermögen), `pos` (Reihenfolge = Kanalreihenfolge bei SPS). `terminalNr`
heißt in der Oberfläche „Bezeichnung“ – der Fork nutzt es richtig als Gruppenschlüssel. Am Artikel:
`groupsymbolmacro` (= Makro) zeigt auf das Hauptmakro im EDZ; `uniqueid` ist der Data-Portal-Schlüssel.

## 4. Zubehör, Zubehörlisten, Zubehörplatzierung

- Jeder Einzelteil-, Baugruppen- oder Modulartikel ist entweder **Hauptartikel** oder **Zubehör** („Artikel ist
  Zubehör“). Zubehör kann selbst kein Zubehör erhalten. Am Hauptartikel je Zubehörzeile „Erforderlich“ oder
  optional. In der Geräteauswahl erscheinen nur Hauptartikel; nach der Wahl wird das passende Zubehör bestimmt.
- **Zubehörliste** = Liste alternativer Zubehörteile mit eindeutigem Namen; enthält nur als Zubehör markierte
  Artikel (gleicher Artikel mehrfach erlaubt). Beispiel der Hilfe: Leuchttaster-Grundkörper mit erforderlicher
  Liste farbiger Kappen – genau eine muss gewählt werden.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_zubehoerlisten.htm

**Bedeutung für QET:** Pflichtzubehör (Kappe, BaseUnit, Beschriftungsschild) gehört in die Stückliste, aber nicht
als eigenes Schaltplan-Element. In QET bei Bedarf als zusätzliche Artikelzeile im Element führen (Vermutung: QET
kennt nur ein Artikelfeld-Set je Element; mehrere Artikel ggf. über Zusatzfelder oder Kommentar).

## 5. Baugruppen und Module

- **Modul** = Artikel mit Liste von Unterartikeln (Modulpositionen), identifiziert über **untergeordnetes BMK oder
  BMK-ID**; kann Artikel, Baugruppen und Module enthalten (z. B. Motorsteuerung im 19"-Rack). Varianten der
  Unterartikel werden bei Geräteauswahl und Auswertungen berücksichtigt.
- Vorgehen: zuerst Makro bauen (empfohlen im Makroprojekt); alle Betriebsmittel des Moduls **innerhalb eines
  Gerätekastens** geschachtelt, der Kasten gehört zum Makro; BMK-IDs im Makro eindeutig und an Haupt- **und**
  Nebenfunktionen gesetzt. Am Modul Feld **Makro** (Technische Daten) Pflicht. BMK-IDs machen Makros unabhängig von
  der Kennzeichnungsnorm. Einfügen wie ein Gerät (Einfügen > Gerät).
- Mechanische Module (Pro Panel): nur Einzelartikel, 3D-Makro im Feld Grafikmakro, nur als Einheit platzierbar;
  Längen variabler Teile landen in Teilmenge/Länge (ID 31008).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_module.htm

**Bedeutung für QET:** Ein Modul (z. B. SPS-Kopf mit Steckmodulen, Antrieb mit Optionskarten) ist in EPLAN **ein**
Gerätekasten mit geschachtelten Betriebsmitteln. Im QET-Nachbau: Master-Element für den Kasten, Unterbetriebsmittel
mit untergeordnetem BMK (z. B. `-A1-K1`); die BMK-ID ist ein stabiler Schlüssel, um Makro-Teile den Unterartikeln
zuzuordnen (Vermutung, falls im `.ema` vorhanden).

## 6. Import/Export: Formate, Konfigurationsdateien (CSV/Text)

- Konfigurationsdatei (`*.cfg`) steuert CSV-, ECAD- und Text-Import/-Export. Kopf: `separator=,`,
  `skipLeadIn=<n>` (zählt alle Kopfzeilen, mit `header` und `fieldTitle=1` mindestens 2), `rowsPerRecord`,
  `header=…`, `fieldTitle=1`, `trim=1`, `convert-region-dependent=1` (Zahlen nach Gebietsschema, „1.200,00“),
  optional `textquote=…`; danach `table=<Tabelle>` und je Feld `feld=[<Zeile>!#<Spalte>]`, feste Texte mit `+`
  verknüpfbar (`partnr="Loz."+[1!#4]`), Festbreite `[Spalte,Länge]`. Mehrere Bereiche (Adressen, Artikel,
  Anschlussbilder, Bohrbilder, Zubehörlisten, -platzierungen) dürfen nacheinander in einer CFG stehen.
- Mehrsprachige Felder: `description1="DE_de@"+[1!#26]+";EN_us@"+[1!#27]+";"` – also Format
  `<Sprache>@<Text>;` je Sprache. Freie Eigenschaften mit Zeilennummer: `description.1`, `val.1`, `unit.1`.
- Mitgeliefert: `…\Artikel\EPLAN\csvimportexport.cfg`; für Text `textimportexport.cfg` unter
  `\Artikel\<Kundenkennung>` (muss für Anschluss-/Bohrbilder ggf. erweitert werden).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_steuerdateiimportexport.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelexportieren.htm
- Export: Teilmengen über [Extras] > Exportieren oder markierte Datensätze über das Kontextmenü (Knoten nehmen alle
  Unterdatensätze mit); Gesamtdatei oder einzelne Dateien; Auswahl nach Datensatztyp und Gewerk; EDZ mit
  „Referenzierte Daten vollständig exportieren“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelexportieren.htm

**Bedeutung für QET:** Mehrsprachige EPLAN-Texte haben das Muster `de_DE@Text;en_US@Text;` – genau das löst der
Fork mit `pickLang`/`multilangNames` auf (Fork-Code `edzpart.cpp`). Die Groß-/Kleinschreibung der Sprachkennung
variiert in der Hilfe (`DE_de`, `EN_us`); der Parser sollte sie ohne Beachtung der Schreibung vergleichen
(Vermutung/Empfehlung).

## 7. Kennungen: Datensatztyp, Produktgruppen, Pflege

- Datensatztyp im CSV-Feld **`class`** (sonst geht er verloren): 0 Undefiniert, **1 Einzelteil, 4 Baugruppe,
  5 Modul**, 6 Bohrbild, 7 Anschlussbild, 11 Kunde, 12 Hersteller/Lieferant. Artikelnummer ist über Gewerke und
  Produktobergruppen hinweg eindeutig. In Ober-, Gruppe und Untergruppe gibt es „Ungültig“ (-1) und „Undefiniert“
  (0) für fehlgeschlagene Importe. Gewerk-Flags: 1 = gesetzt, 0 = nicht.
- Produktgruppen der Obergruppe Elektrotechnik (interner Wert, Auszug): 1 Allgemeine, 2 Relais/Schütze (Untergr.
  2 Schütze, 3 Hilfsblock), 3 Klemmen (4 Klemme, 5 Tragschiene, 6 Endwinkel, 7 Klemmenschild, 10 Querverbinder …,
  219 Schaltbrücke), 4 Stecker (215 Stifteinsatz, 216 Buchseneinsatz …), 5 Umformer, 6 Schutzeinrichtungen (189 FU-,
  190 FI-Schutzschalter, 191 Motorschutzschalter, 192 Schmelzsicherung, 193 Schutzschalter, 194 Sicherungsautomat,
  201 Schutzrelais), 7 Röhren/Halbleiter, 8 Meldeeinrichtungen (185 Anzeigegerät, 186 Meldeleuchte …), 9 Motoren,
  10 Messgeräte, 11 Widerstände, 12 Sensorik/Schalter/Taster (202 analoger, 203 digitaler Sensor, 204 Endschalter,
  206 Lichtschranke, 207 Näherungsschalter, 208 Schalter/Taster), 13 Transformatoren, 15 elektrisch betätigte
  mechanische Einrichtungen, 18 Kondensatoren, 19 Logikbauteile, 20 Spannungsquelle/Generator, 21 Induktivitäten,
  22 Verstärker/Regler, 23 Starkstrom-Schaltgeräte, 24 Abschlüsse/Filter, 25 Übertragungswege, **26 SPS** (152/153
  Eingangskarte analog/digital, 154/155 Ausgangskarte analog/digital), **29 Kabel** (184 vorkonfektioniert).
  Weitere Obergruppen: Fluid, Mechanik (u. a. Gehäuse, Kabelkanäle, Sammelschienen, Schaltschrank),
  Verfahrenstechnik.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_importexport.htm
- Filtern: Volltextfilter (Teilstring, mit UND/ODER) und feldbasierter Filter (exakter Feldinhalt; Werte mit
  Einheiten ohne Einheit eingeben; Dateinamen mit Pfad und Endung; Dokumentfelder enthalten
  `<Datei>??_??@<Text>;` und brauchen `*`). Feldkriterien sind UND-verknüpft, ODER explizit.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelfiltern.htm
- Baumkonfiguration: Hauptknoten mit Filter und Struktureigenschaften (Beispiel Kabel → Hersteller → Querschnitt →
  Aderzahl → Artikelnummer).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_baumkonfiguration.htm
- Liste und Baum sind synchron; Filter wirken in beiden, in der Liste nur auf Artikeldaten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_listenutzen.htm
- Kurznamen von Herstellern/Lieferanten/Kunden müssen eindeutig sein (`tblAddress.shortName`, max. 30).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_kundenbearbeiten.htm
- Mehrsprachig: Bezeichnung 1–3, Beschreibung, Texte der freien Eigenschaften; Übersetzung über Wörterbuch,
  Quellsprache wählbar (oder Dialogsprache `##_##`).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_mehrsprachigeartikelverwalten.htm
- Preise: Preiseinheit, Mengeneinheit, Menge/Verpackung, EK je Preiseinheit oder je Verpackung (jeweils eine
  berechenbar), zwei Währungen mit Wechselkurs aus den Benutzereinstellungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_preisewaehrungenbearbeiten.htm

**Bedeutung für QET:** Die Produktgruppe ist der zuverlässigste Hinweis auf den Bauteiltyp beim Import
(26/152–155 → SPS-Kanal-Elemente, 3 → Klemme, 29 → Kabel, 6 → Schutzorgan). Der Fork könnte die Kategorie-Ordner
daraus ableiten statt aus Freitext (Vermutung/Empfehlung). Der Hersteller-Kurzname (z. B. „SIE“, „PXC“) ist der
EPLAN-Schlüssel; `manufacturerName()` im Fork übersetzt ihn schon in einen Anzeigenamen (Fork-Code).

## 8. Baugruppen, Funktionsschablonen zuweisen, Funktionsschablonen summieren

- **Baugruppe** = Artikel aus mehreren Artikeln **eines** Betriebsmittels (Taster = Kontakt + Halter + Druckknopf),
  eigene Artikelnummer, verschachtelbar; „Baugruppe verteilt platzieren“ für den 3D-Aufbau.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_baugruppenanlegen.htm
- **Funktionsschablonen zuweisen** (2.9 unter [Extras] bzw. Kontextmenü): prüft Feld Makro (Technische Daten),
  durchsucht das Makro beginnend mit der Hauptfunktion und füllt die Schablonen mit den Funktionsdaten; Artikel an
  der Hauptfunktion im Makro werden ignoriert. Danach **Artikelabgleich** im Projekt, damit die Schablonen am
  Betriebsmittel „überlagert“ werden. Fehler (kein Makro, keine oder mehrere Hauptfunktionen) → Systemmeldung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_funktionsschablonenzuweisen.htm
- Modulpositionen: je Zeile **entweder BMK oder BMK-ID**; das Betriebsmittel muss im Modulmakro vorkommen; der Artikel
  der Zeile wird beim Einfügen diesem Betriebsmittel zugewiesen. **[Extras] > Funktionsschablonen summieren**
  trägt die Schablonen aller Unterartikel in die Schablonen des Moduls ein. Modul einfügen: Einfügen > Gerät,
  Modulartikel steht dann am Gerätekasten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_moduleanlegen.htm
- Zubehörliste anlegen: Artikelauswahl ist auf „Zubehör“ vorgefiltert; danach Artikeldaten abgleichen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_zubehoerlisteerstellen.htm

**Bedeutung für QET:** Funktionsschablonen und Makro hängen in EPLAN eng zusammen: die Schablonen werden aus dem
Makro gewonnen. Liefert ein EDZ beides, sollten Anschlussbezeichnungen im `.ema` und in den `functiontemplate`s
übereinstimmen – eine einfache Plausibilitätsprüfung für den Fork-Import (Vermutung/Empfehlung). Bei Modulen sind
die Schablonen die Summe der Unterartikel, d. h. ein Modul-EDZ enthält viele Pins, die zu mehreren Geräten gehören.

## 9. Registerkarten des Artikels: Regeln mit Wirkung

- **Allgemein**: Artikelnummer identifizierend (bis 255 Zeichen; mit Varianten Nummer+Variante), empfohlene
  Nummernorganisation z. B. `Hersteller.Gruppe.LfdNr` (`SIE.3TB.0001`); ERP-Nummer ebenfalls eindeutig (bis 50);
  **Typnummer nicht eindeutig** (v. a. Kabel), Bezeichnung 1–3 werden nicht ausgewertet, aber in Stücklisten
  ausgegeben; Hersteller/Lieferant per Kurzname (unbekannter Kurzname legt leeren Adressdatensatz an);
  „Auslaufartikel“ filterbar. Produktgruppe aus festen Listen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_allgemein.htm
- **Technische Daten**: Technische Kenngrößen (z. B. Spulenspannung, Schaltleistung) werden aus der **ersten
  Hauptschablone** übernommen (dann gesperrt) und bei der Artikelauswahl übertragen; Gruppennummer, **Teileart**
  (max. 10 Zeichen, z. B. Leistungs-/Hilfsschütz), **Funktionsgruppe** (max. 30, Filter); Feld **Makro**
  (Symbolmakro oder Kontur-Extrusion; Pflicht bei Modulen). Artikel mit Makro sind aus der Geräteliste per
  Drag & Drop platzierbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_technischedaten.htm
- **Funktionsschablonen** (Ergänzung zu `makros-artikel.md` Abschnitt 11): Grundlage für den **Gerätevergleich**
  (gewähltes Gerät gegen projektiertes); verglichen wird genau das, was in der Schablone steht. Spalten hängen von
  der Funktionsdefinition ab, Reihenfolge fest. Anschlussinformationen der ersten Schablone zählen nur bei
  mehreren Varianten. Hilfsblock-Varianten mit „13;14“ bzw. „23;24“ als Artikelvarianten. Bei Anschlussbildern über
  mehrere Varianten: alle Anschlüsse ins Anschlussbild, in den Varianten keine zusätzlichen. Die Spalte
  **Beschreibung** wird nicht an die Funktion übertragen, aber in Navigatoren in eckigen Klammern hinter der
  Funktionsdefinition angezeigt. Mehrfachauswahl → schreibgeschützt. Module: keine Schablonen (ausgegraut).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_funktionsschablone.htm
- Mechanik-Artikel haben statt Funktionsschablonen die Registerkarte **Funktionsdefinition** (Funktionsdefinition +
  Bauteil = Produktgruppe in Mechanik, wirkt auf die 3D-Artikelplatzierung).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_funktionsdefinition.htm
- **Modul**: je Position BMK **oder** BMK-ID (Werte 1–4); beide leer → Meldung. Längenanteil für Kabel/Leitungen
  wandert beim Auflösen in **Teilmenge/Länge**; Module in Stücklisten nur mit „Module auflösen“ aufgelöst
  (Einstellung je Modul überschreibbar).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_modul.htm
- **Baugruppe**: Positionsnummer/Zusatztext aus der Vorplanung; Längenanteile wie beim Modul.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_baugruppe.htm
- **Zubehör**: Datensatztyp („Einzelteil“/„Zubehörliste“) automatisch; Zubehörplatzierung nur für Einzelteile.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_zubehoer.htm
- **Anschlussbild** (eigener Datensatz, am Artikel in Technische Daten referenziert): Standard-Anschlussausführung,
  Anschlussmaß (z. B. M6, 4,8x0,5), Zusatzlänge, Verlegerichtung; Einzelanschlüsse mit X/Y/Z. Gilt für
  Artikelplatzierungen ohne eigene Anschlüsse. Empfohlene **Anschlussausführungen nach eCl@ss**: Schraub-,
  Zugfeder-, Push-in-, Durchdring-, Schneidklemm-, Steck-, Steckklemm-, Bolzen-, Flachsteck-, Crimp-, Band-,
  Schienen-, Lötfahnen-, Wickelanschluss (ältere Bezeichnungen nur noch zur Kompatibilität).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_anschluesse.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_einstanschlussausf.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_anschlussklemmen.htm
- **Daten für Auswertungen**: „Kennung für Auswertungen“ (einsprachig) wählt bedingte Formulare/Unterformulare;
  Symbole im Format `<Bibliothek>;<Zuordnung>;<Symbolnummer>;<Variante>` (Beispiel `IEC_SYMBOL;1;50;0`).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_datenauswertung.htm
- **Eigenschaften**: produktgruppenspezifische Eigenschaften als Baum; **dunkelgrau** = Eigenschaft passt nicht zur
  Produktgruppe (löschbar). **Attribute** sind nicht mehrsprachig (max. 80 Zeichen), **freie Eigenschaften** nur noch
  zur Kompatibilität – empfohlen sind **benutzerdefinierte Eigenschaften** (Schema-basiert, für alle Varianten
  gleich). **Dokumente**: bis zu 20 Dateien/Hyperlinks.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_eigenschaften.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_attribute.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_zusatztexte.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_konfigeigenschaften.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_dokumente.htm
- **Sicherheitskennwerte** (Elektro/Fluid): Anwendungsfälle in bis zu 5 Ebenen (z. B. Antrieb mit/ohne Bremse,
  1-/2-kanalig), je Fall Funktionsbereich Eingang/Logik/Ausgang und Kennwerte PL, MTTF(d), SIL CL, MTBF, PFHd u. a.;
  Import im XML-Format **VDMA 66413**.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_sicherheitskennwerte.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_anwendungsfall.htm
- Bohrbild, Fertigung, Montagedaten, Montageplatten, Türen, Tragschienen, Sammelschienen, Sperrflächen,
  Zubehörplatzierung: Aufbau-/Pro-Panel-Daten (Maße in mm, Bezugspunkte, Durchbrüche ab Nullpunkt unten links);
  für den Schaltplannachbau ohne Bedeutung.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_bohrbild.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_montagedaten.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_fertigung.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_konstruktion.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_montageplatten.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_platzierung.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_sammelschiene.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_sammelschienen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_sperrflaeche.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_tragschienen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_tueren.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_zubehoerartikel.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_zubehoerplatzierung.htm

**Bedeutung für QET:** Abbildung EPLAN → QET-`elementInformations` (Empfehlung): `manufacturer` ← Hersteller
(Langname, Kurzname als Zusatz), `manufacturer_reference` ← Bestellnummer, Artikelnummer `partnr` in ein eigenes
Feld (z. B. `supplier_reference` oder Infofeld „Artikelnummer“), `designation`/`description` ← Bezeichnung 1,
Typnummer als Zusatzinfo, technische Kenngröße der Hauptschablone (Spulenspannung, Schaltleistung) als sichtbarer
Funktionstext. Die Beschreibung der Funktionsschablone gehört **nicht** an den Kontakt im Plan. Welche
QET-Infofeldnamen der Fork genau nutzt, ist im Fork-Code zu prüfen (offen).

## 10. Import, Export, Feldzuordnung, geschützte Felder

- Export-Dateitypen: **CSV** (getrennt: Adressen, Anschlüsse, Artikel, Bohrbilder, Zubehörlisten,
  Zubehörplatzierungen), **Text**, **XML**, **EDZ** (Data-Portal-Austauschformat). Einzelne Dateien heißen wie
  Artikelnummer bzw. Name. Datensatztyp „Eigenschaftskonfiguration“ (benutzerdefinierte Eigenschaften) nur bei EDZ
  und XML.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artikelexport.htm
- Import: Dateityp (u. a. CSV, Text, XML, EDZ, **VDMA 66413** mit Zuordnung der identifizierenden Eigenschaft),
  Mehrfachauswahl, **Feldzuordnung**-Schema (Quellnummer → Zielnummer für freie Eigenschaften/Attribute), Modus
  nur neue / nur vorhandene aktualisieren / beides. Attribute und freie Eigenschaften werden **ergänzt**,
  Funktionsschablonen **ersetzt**.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artikelimport.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelimportieren.htm
- Dialog **Feldzuordnung / Felder schützen** (auch beim Data-Portal-Import): Felder und benutzerdefinierte
  Eigenschaften, die beim Import **nicht** aktualisiert werden; Kontrollkästchen **Funktionsschablonen schützen**
  (Schablonen werden dann nie importiert).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_feldzuordnung.htm
- Einstellungen der Artikelverwaltung: Funktionsdefinitionsbibliothek für die Schablonenauswahl, Sortierung
  alphabetisch/alphanumerisch, **Maße intern immer mm/kg** (Maßsystem für Import/Export wählbar), zwei Währungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_einstellungenartverw.htm
- Volltextsuche der Artikelverwaltung: Teilwort, Groß/klein egal; `"…"` exakte Folge; Leerzeichen = ODER;
  ` +Wort` = UND (nur SQL Server; Access behandelt es als ODER); keine Platzhalter; Suchindex muss erzeugt werden;
  Kunden/Hersteller werden nicht durchsucht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artverw.htm
- **Automatikfilter** der Artikelauswahl: nur Artikel mit gleicher Funktionsdefinition / gleichem Gewerk / gleichem
  Makro wie das markierte Betriebsmittel.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_automatikfilter.htm
- Gewerk/Untergewerk: zusätzliche Gewerke je Artikel, eigene Untergewerke anlegbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_gewerk.htm
- Baumkonfiguration-Hauptknoten: Datensatztyp, Filter, Struktureigenschaften.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_baumkonfiguration.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_baumkonfigurationhauptknoten.htm
- Preise/Währungen: siehe Abschnitt 7.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_preiseberechnen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_waehrungumrechnen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_preise.htm

**Bedeutung für QET:** Das Import-Verhalten „Funktionsschablonen ersetzen, freie Eigenschaften ergänzen“ ist ein
gutes Vorbild für das Neu-Importieren eines Bauteils im Fork: Pins (Schablonen) komplett aus der neuen Quelle,
Zusatzinfos zusammenführen, vom Nutzer geschützte Felder (z. B. eigene Beschreibung) nicht überschreiben
(Empfehlung). Maße in EDZ-/CSV-Daten sind mm.

## 11. Artikelauswahl und Geräteauswahl: Prinzip

- **Artikelauswahl** = Wahl nach kaufmännischen Daten (Datenquelle: interne Datenbank, Access, SQL Server oder
  API), **Geräteauswahl** = Wahl nach technischen Merkmalen, immer aus der EPLAN-eigenen Datenbank; danach kann eine
  externe Quelle die Daten ergänzen („Artikeldaten ergänzen über“, getrennt einstellbar). Nach dem Einfügen eines
  Makros sind Artikel nur als Referenz da und müssen ergänzt werden.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_k_start.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_artikeldatenergaenzen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_konfigapiartikel.htm
- Speicherorte: bis **1.000 Projektartikel** (Beistellteile, Installationsmaterial), bis **50 Artikel je
  Hauptfunktion**, ebenso je **Artikeldefinitionspunkt**. Artikel stehen **nur an der Hauptfunktion**, auch für alle
  Nebenfunktionen des Betriebsmittels. Welche Artikeleigenschaften eingelagert werden, ist konfigurierbar (z. B.
  Preise nicht an Endkunden); „Eingelagerte Eigenschaften löschen“ entfernt sie gezielt.
- **Abgleich**: Projekt abgleichen (Dialog), aktualisieren (alle veralteten eingelagerten Artikel ersetzen und
  Gerätedaten an die Hauptfunktionen übertragen), vervollständigen, beim Öffnen aktualisieren. Artikelnummern und
  Artikelreferenzdaten bleiben dabei unverändert. Bei geänderten Schablonen ggf. „Gerätedaten aktualisieren“ im
  Stückliste-Navigator.
- **Artikel aus Schaltplandaten erzeugen**: nur an einer Hauptfunktion mit genau einer Artikelreferenz (Menge 1);
  erzeugt Funktionsschablonen für alle Funktionen des Betriebsmittels; Funktionsgruppe wird „NEW“.
- Artikel mit Menge 0 werden in Stücklisten immer ausgegeben (Filter nötig). Baugruppen können beim Einfügen
  bis zu einer Stufe aufgelöst werden (danach nicht mehr als Baugruppe auswertbar). Hinzufügen eines Artikels hängt
  seine Schablonen an das Betriebsmittel, Entfernen entfernt sie wieder (Multifunktionsrelais über Varianten).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_k_prinzip.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikelausschaltplan.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikeldateneinlagern.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_eingelagerteartikeleigenschaftenloeschen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_artikeleigenschaftenloeschen.htm
- **Artikelabgleich-Dialog**: Liste eingelagerte Artikel ↔ Systemartikel mit Status (Neuer, Identisch, Nicht
  eingelagert …); Übertragung ins System legt nur fehlende Artikel an, vorhandene werden **nicht** aktualisiert.
  Artikel werden ins Projekt kopiert, nicht referenziert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsextensiongui_d_artikelabgleich.htm
- **Artikeldefinitionspunkt**: hängt Artikel ohne Betriebsmittel an eine Stelle im Plan (bis 50), mit sichtbarem/
  vollständigem BMK für die strukturbezogene Stückliste; kein Betriebsmittel (nicht im BMK-Navigator).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_adpzeichnen.htm

**Bedeutung für QET:** Im EPLAN-PDF trägt nur das Hauptfunktions-Symbol (Spule, Kasten, Motor) die
Artikeldaten; Kontakte erben sie. In QET deshalb Artikeldaten nur am **Master**-Element, Slaves ohne eigene
Bestellnummer, sonst doppelt in der Stückliste. Mehrere Artikel an einem Betriebsmittel (Sicherung = Einsatz +
Sockel + Passring) sind in QET nur über Zusatzfelder abbildbar (offen). Artikeldefinitionspunkte (Montagematerial
ohne Symbol) im Nachbau als reines Info-Element oder gar nicht – sie erscheinen nur in Stücklisten.

## 12. Übertragene Daten, Geräteauswahl-Regeln, Konflikte

- Beim Zuordnen gehen Daten des **ersten** Artikels an die Funktion (Artikel-ID → Funktions-ID):
  Spule: Spannung 22218 → 20054; Schaltleistung 22072 → Kontakt: Schaltleistung 20055; SPS-Typbezeichnung 22105 →
  20416; Gerätebeschreibung: Dateiname 22037 → 20415; Objektbeschreibung 22038 → 20417; Version 22104 → 20418;
  Adressbereich 22106 → 20432 (2: 22261 → 20299); Bus-Koppler/Kopfstation 22019 → 20164; CPU 22020 → 20167;
  Spannungsversorgung 22052 → 20184; Bus-Verteiler 22053 → 20189; Stationstyp 22269 → 20409; **Technische Kenngrößen
  22017 → 20027**; Anschlussquerschnitt 22036 → 20057; Anzahl Steckerkontakte 22035 → 20056. Kabel: Typ 22030 →
  20040, Querschnitt 22032 → 20043, Einheit 22068 → 20068, Verbindungsanzahl 22031 → 20042, Spannung 22033 → 20044,
  Bezeichnung in der Grafik 22064 → 20067, Kabeldarstellungsplanformular 22034 → 20092, Länge 22055 → 20041.
  Außerdem alle Eigenschaften der **Hauptschablone** und die Artikeldatenzuweisungen des Makrokastens. Technische
  Kenngrößen des Artikels nur, wenn die Hauptschablone keine hat.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_k_uebertragenedaten.htm
- Artikelauswahl am Eigenschaftendialog: Daten werden bei späteren Artikeländerungen **nicht** nachgeführt. Weicht
  Vorhandenes ab (Kenngrößen, Anschlussbezeichnung, Kabeltyp), erscheint der **Konflikt**-Dialog (Beibehalten /
  Überschreiben / Anhängen je Eigenschaft). Technische Eigenschaften überträgt nur die **erste Artikelzeile**;
  für weitere Zeilen nur Artikelreferenzdaten. Neue Zuweisung immer in eine leere Zeile, sonst Überschreiben.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikelauswahl.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_konflikte.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_artikelauswahl.htm
- **Artikelreferenzdaten** = an die Funktion kopierte Artikeldaten, teils editierbar; Abweichung vom Artikel gelb.
  Leeres Feld wird beim Übernehmen wieder aus dem Artikel gefüllt; identische Referenzdaten werden bei neuer Auswahl
  gelöscht. Eigenschaft **Artikelzuordnung**: Hauptartikel / Zubehör / Artikel Quelle / Artikel Ziel (z. B.
  Kabelschuhe am Kabelende; für Auswertungen und automatische Kabelauswahl). „Artikel ist Bestandteil eines
  Moduls“ unterdrückt ihn in der Stückliste. Zeilenreihenfolge = Reihenfolge in Auswertungen.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikeldatenanschaltzeichen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_r_artikel.htm
- **Geräteauswahl**: Liste Hauptartikel (passend, kein Zubehör), Liste Zubehör (zum markierten Hauptartikel;
  Pflichtzubehör sofort wählbar, Zubehörlisten über Unterdialog), Vorschau, welche Funktion welche Schablone
  bekommt und welche frei bleiben. Daten der Schablonen gehen an Haupt- **und Nebenfunktionen**; ein vorhandener
  Artikel wird ohne Konfliktdialog überschrieben; am Schaltzeichen eingetragene Daten (z. B.
  Anschlussbezeichnungen) bleiben, Abweichungen meldet der Prüflauf. Beispiel der Hilfe: Schütz-Hauptartikel mit
  drei Leistungsschließern + Hilfsblock-Zubehör (2 Öffner oder 2 Schließer).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_geraeteauswahl.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_geraeteauswahl.htm
- **Passungsregel**: zuerst identische Funktionen; sonst allgemeine Funktion (z. B. „Schließer, allgemein“); ein
  Betriebsmittel mit allgemeiner Funktion bekommt alle Geräte mit einer Schablone **derselben Gruppe**. Gruppe
  „Grafik“ → alle Artikel mit Grafik-Schablone, ohne Schablonen und Zubehör (so werden Mechanik-Artikel wählbar).
  Reine Definitionsfunktionen (Gerätekasten, SPS-Kasten, Klemmenleisten-, Stecker-, Kabeldefinition, Abschirmung)
  an einem sonst leeren Betriebsmittel: nur Artikel, die genau diese Schablone haben (leerer SPS-Kasten → nur
  Artikel mit Schablone „SPS-Kasten“). Übernommen werden Kenngrößen, Funktionsgruppe, Artikeltyp, Teileart,
  Schützvariante, Baugruppe, Produktuntergruppe.
- Modul an Gerätekasten: EPLAN betrachtet alle Funktionen im Kasten **und in allen querverwiesenen Gerätekästen**
  und verteilt die Modulartikel auf passende Hauptfunktionen; Baugruppe an Gerätekasten bleibt am Kasten.
- Zuordnungsfälle: Einzelsymbol (Motor), **verteiltes Betriebsmittel** (mehrere Symbole, gleiches BMK, z. B. Schütz,
  vorkonfektioniertes Kabel), Modul (mehrere Symbole, verschiedene BMK, z. B. U1-K1…K3, U1 trägt den Modulartikel).
- Blockbearbeitung aus dem Eigenschaftendialog schreibt den Artikel an alle markierten Symbole (Kriterium nur das
  erste); aus dem Stückliste-Navigator nacheinander.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_k_vorgehensweise.htm
- Einstellungen Geräteauswahl: Standard bietet nur Artikel mit **passendem oder größerem Schablonensatz**.
  Automatische Geräteauswahl (Dialog nur bei Fehler/Überbelegung oder fehlendem Artikel; ergänzt fehlende
  technische Daten), **Nur Hauptfunktion berücksichtigen** (für modulare Geräte), **Freie Funktionen minimieren**
  (max. Anzahl unbenutzter Schablonen), Anzeige auch bei nur einem Treffer, alte Artikel überschreiben,
  **Symbol bei wechselnder Funktion tauschen** (Symbol aus der Schablone, sonst Standard-Platzierungssymbol),
  Auswahlkriterien je Produktgruppe: aktivierte Eigenschaften bleiben an der Funktion, deaktivierte werden von der
  Schablone überschrieben. Kabel: Potenzialtyp berücksichtigt nur N, PE/PEN, SH.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_einstellungengeraeteauswahl.htm
- Filter: feldbasiert (exakt), Volltext, **Automatikfilter** (Eigenschaften des Betriebsmittels, z. B.
  Funktionsdefinition; wirkt nur auf Hauptartikel, nicht auf Zubehör; Platzhalter erlaubt), **Artikelvorauswahl**
  (projektbezogene Artikelliste, exportierbar), Kabelauswahl mit Aderfilter; **Artikelvorfilterung** (Benutzer,
  „%“ in der Titelleiste). Filter wirken nur auf Felder der Artikelverwaltung, nicht auf Funktionsdefinitionen.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_k_filtervorauswahl.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_geraetefilter.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikelvorauswahl.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_artikelvorauswahl.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsextensiongui_d_artikelvorfilterung.htm
- Datenquelle: Benutzer-Schema (Access `.mdb`, SQL Server, API; optional Schemaauswahl bei jeder Auswahl;
  „Änderung während Auswahl erlaubt“), Projekt-Einstellung „Projekt“ = nur eingelagerte Artikel (eigene
  Projektartikeldatenbank, „Aktualisieren“). Neue Geräte im BMK-Navigator über „Neues Gerät“ (Beispiel: Schütz
  unter Einbauort → K1 mit allen Funktionen); Stückliste-Navigator: Artikel hinzufügen/austauschen.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_datenauswahlbenutzer.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_datenauswahlprojekt.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_datenquelleauswahl.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikelauswahlnavigator.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_adresseauswaehlen.htm

**Bedeutung für QET:**
- Die ID-Paare sind direkt nutzbar, wenn EPLAN-Exporte (Betriebsmittelliste, XML) ausgewertet werden: am Symbol
  heißen die Werte 20027 (Technische Kenngrößen), 20054 (Spulenspannung), 20055 (Schaltleistung), 20416
  (SPS-Typbezeichnung), 20040/20042/20043 (Kabeltyp/Aderzahl/Querschnitt). Ein Verknüpfungs- oder Importskript
  liest also die **Funktions-IDs**, nicht die Artikel-IDs (Abgleich mit `eigenschaften-ids.md`).
- „Verteiltes Betriebsmittel“ = QET-Master mit Slaves gleichen Labels; „Modul“ = mehrere Master unter einem
  Kasten mit untergeordnetem BMK. Für den Nachbau gilt die Passungsregel sinngemäß: ein Kontakt-Slave passt zu
  einem Master, wenn dessen Artikel eine Schablone **derselben Funktionsdefinition** mit **gleichen
  Anschlussbezeichnungen** hat – das ist ein prüfbares Kriterium für ein Verknüpfungsskript (Vermutung/Empfehlung).
- „Symbol bei wechselnder Funktion tauschen“ bestätigt: das Symbol folgt der Funktionsdefinition, nicht umgekehrt.

## 13. Funktionspool, Gerätedefinition, nicht platzierte Funktionen („Reservebauteile“)

- **Funktionspool**: ein Gerät besteht aus beliebig vielen Funktionen (Schütz = Spule + Schließer …); Funktion und
  Schaltzeichen stehen 1:1. Pool entsteht über Geräteauswahl oder nicht platzierte Funktionen. Analog:
  **Klemmenpool** einer Leiste (nicht gezeichnete Klemmen können verdrahtet, gebrückt und mit Kabeladern belegt
  werden; Mehrstockklemme = mehrere Funktionen) und **Kabelverbindungspool** (freie Adern; Belegung platziert ↔
  platziert, nicht platziert ↔ nicht platziert, platziert ↔ nicht platziert). Ebenso nicht platzierte Verdrahtungen
  (Brücken auf nicht platzierten Klemmen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_grundlagen.htm
- **Gerätedefinition (Typ)** = Liste von Funktionsschablonen, gehört zur **Hauptfunktion** (wird mit ihr kopiert und
  gelöscht). Nur mit Gerätedefinition kann EPLAN prüfen, ob gezeichnete Funktionen zum Gerät passen. Beispiel der
  Hilfe: Hilfsschütz 1S+1Ö, gezeichnet Spule + 1 Schließer = in Ordnung (Öffner frei); Spule + 2 Schließer = Fehler.
  Beispiel Stecker: fünf gezeichnete Buchsen A1, A10, A100, A1000, A2 werden nach Zuweisen eines 7-poligen Steckers
  automatisch auf A1–A5 umnummeriert; die Zuordnung platziert ↔ Pool folgt den Anschlussbezeichnungen, widersprüchliche
  Änderungen erzeugen eine Fehlermeldung.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_start.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_geraetedefinitionen.htm
- **Schablonen-Eigenschaften**: Funktionsdefinition, Anschlussbezeichnung (z. B. `13¶14`), Klemmenbezeichnung,
  Index 1/2, Technische Kenngrößen. **Überlagerungsregel**: ist eine Schablonen-Eigenschaft gefüllt, muss die
  Funktion exakt denselben Wert haben; eine leere Schablonen-Eigenschaft passt zu jedem Wert; eine leere
  Funktions-Eigenschaft passt nur zu leerer Schablone. **Überschreibregel** (Geräteauswahl): gefüllte
  Schablonenwerte gehen an die Funktion, leere ändern nichts. Kabel: Potenzialtyp PE/PEN/SH muss auf beiden Seiten
  exakt gleich sein, alle anderen Potenzialtypen passen immer.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_funktionsschablonen.htm
- Nicht platzierte Funktionen tragen alle Daten, aber **keine Grafik** (kein Symbol, keine Lage); sie können mit
  platzierten und nicht platzierten Funktionen verbunden werden.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_datenvonnichtplatziertenfunktionen.htm
- **Reservebauteile** = nicht gezeichnete Geräte (Lagerersatzteile), optional in der Stückliste; sie sollen **kein
  BMK** bekommen, weil beim Projektabschluss nirgends gezeichnete BMK als Fehler gelten. Elemente auf Schaltplan-,
  Übersichts- oder Montageseiten sind keine Reserven.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_arbeitsweise.htm
- **Platzierung löschen (Zurückbuchen)**: Symbol weg, Funktion bleibt nicht platziert mit allen Daten (Darstellungsart
  bleibt). Zurückbuchbar: Schaltplan ein-/allpolig, Übersicht, Paarquerverweis, RI-Fließbild; nicht: grafische,
  externe, Artikelplatzierungen, Nicht-Funktionen (Winkel, T-Stücke, Abbruchstellen). Nur im Schaltplanprojekt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_h_funktionenzurueckbuchen.htm
- Kopieren: BMK muss danach neu nummeriert werden; Kopie im BMK-Navigator verliert die Platzierung; Schablonen
  selbst sind nicht kopierbar (nur mit der Hauptfunktion). Löschen: gelöschte Gerätefunktion bleibt in der
  Gerätedefinition; Löschen der Hauptfunktion löscht das Gerät; nicht platzierte Funktionen nur im BMK-Navigator
  oder beim Komprimieren löschbar; Löschen der Gerätedefinition berührt die Funktionen nicht.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_kopiermodifunktionen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_loeschmodifunktionen.htm

**Bedeutung für QET:**
- QET hat keine nicht platzierten Funktionen. Kontaktspiegel und Klemmenpläne im EPLAN-PDF zeigen aber auch **freie**
  (nicht platzierte) Kontakte und Klemmen. Im Nachbau: der Kontaktspiegel am Master muss die vollständige
  Gerätedefinition (z. B. alle 4 Hilfskontakte) zeigen, auch wenn nur zwei als Slave gezeichnet sind – im Fork
  über die Kontaktliste des Master-Elements aus den Funktionsschablonen der EDZ (Vermutung/Empfehlung).
- Die Überlagerungsregel ist eine präzise Vorlage für den Abgleich Slave ↔ Schablone im Verknüpfungsskript:
  gefüllte Schablonenfelder exakt vergleichen, leere als Joker.
- Freie Kabeladern und nicht gezeichnete Klemmen erscheinen nur in Kabel- und Klemmenplänen. Diese Auswertungen
  dürfen sie im QET-Nachbau nicht verlieren (siehe `kabel.md`, `klemmen.md`).

## 14. Stammdaten: Einlagerung, Abgleich, Datenablage

- Stammdaten = Symbole, Normblätter, Formulare, Makros, Projektvorlagen/Basisprojekte, Fremdsprachendatenbank,
  Artikel inkl. Gerätedefinitionen, **Funktionsdefinitionen**, Bilder, DXF/DWG, Rechtedatenbank; Verzeichnisse unter
  Benutzer > Verwaltung > Verzeichnisse.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_k_start.htm
- Prinzip: Stammdaten werden ins Projekt **kopiert** (eingelagert), im Projekt wird referenzierend auf die Kopie
  zugegriffen (ein Symbol nur einmal in der Projektbibliothek). Abgleich beim Öffnen/Sichern/Wiederherstellen
  optional; bearbeitet wird nur im Stammdatenpool (Editoren).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_k_grundlagen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_d_einstellungenstammdatenallg.htm
- Datenablage: Makros, Formulare, Normblätter sind **Einzeldateien im XML-Textformat**; Symbolbibliotheken mehrere
  Binärdateien in eigener Verzeichnisstruktur. Makros und Fremdsprachen werden nicht als Kopie eingelagert,
  sondern beim Einfügen aufgelöst bzw. wortweise eingelagert. Kein automatischer Abgleich Einzeldatei ↔
  Stammdatenprojekt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_k_datenablage.htm
- Makros: beim Einfügen aufgelöst, danach nicht von Einzelobjekten zu unterscheiden; im Makro ist der **Name der
  Symbolbibliothek** hinterlegt (für die Vorschau). Artikel aus Makros werden **nicht** automatisch eingelagert
  (Aktuelles Projekt aktualisieren; überschreibt aber alle Projektartikel), Normblätter/Formulare über „Projekt
  vervollständigen“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_makrosverwalten.htm
- Symbolbibliotheken: die Symbolauswahl zeigt nur eingelagerte Bibliotheken, identifiziert **über den Namen ohne
  Pfad**; referenziert auch über Formulareigenschaften (Zuordnung Eigenschaft/Funktionsdefinition/Index zu Grafik).
  Beim Aktualisieren wird Kompatibilität (z. B. Anschlussbezeichnungen) geprüft, sonst Abbruch. Dateinamen sind
  beim Abgleich nicht groß/klein-sensitiv.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_d_einstellungensymbolbibl.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_stammdatenabgleichen.htm
- Artikel: Projekt-Artikeldatenbank getrennt; Abgleich ins System nur für fehlende Artikel („System
  vervollständigen“), System wird durch neuere Projektartikel nicht überschrieben.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_artikeldatenverwalten.htm
- Vervollständigen/Aktualisieren (Ergebnis in Systemmeldungen), Formulare je Auswertungstyp einlagern, Verwaltung je
  Stammdatentyp.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_stammdatenprojektvervollstaendigen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_formulareinstellungenfestlegen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_stammdatenverwalten.htm

**Bedeutung für QET:** Das Muster „Stammdaten ins Projekt kopieren“ hat QET schon (eingebettete Sammlung im
`.qet`). Die Hausregel „Programm nicht ungefragt starten“ und die AppData-Virtualisierung betreffen genau diese
Ablage: importierte Elemente müssen dort liegen, wo QET sie sieht. Weil EPLAN-Makros die Symbolbibliothek nur per
Name referenzieren, braucht der Fork für Symbolverweise eine eigene Zuordnung Bibliotheksname → QET-Element (offen).

## 15. Pro Panel: Bohrbilder aus Maschinendaten (knapp)

- Bohrbilder aus Rittal Perforex/Secarex (`*.PBT`, `*.PVB` Bauteile, `*.PKU` Konturen) oder aus DXF/DWG (z. B.
  Steinhauer, CAD) per Importschema und **Bohrbildkasten** im Makroprojekt; Ebenen **EPLAN802** Umriss, **803**
  Bohrungen, **804** Gewinde, **805** Konturen; Nullpunkt unten links. Grafische Definition auf 1:1-Grafikseite.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/propanelmasterdata_k_start.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/propanelmasterdata_h_bohrbildausdxfdaten.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/propanelmasterdata_h_bohrbildausncdaten.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/propanelmasterdata_h_bohrbildgrafischdefinieren.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/propanelmasterdata_h_bohrbildkasteneinfuegen.htm

**Bedeutung für QET:** Beim DXF-Import aus Herstellerpaketen gehören Elemente auf Bohrbild-Ebenen (802–805) und
Aufbau-Darstellungen nicht in ein Schaltplan-Element (Vermutung: der Fork filtert bisher nach Darstellungsart,
nicht nach Ebene – prüfen).

## 16. BMEcat- und eCl@ss-Import

- eCl@ss = branchenübergreifende Klassifikation für Warengruppen und -merkmale; Import über das XML-Format
  **BMEcat 2005.1** (mehrsprachig). Im Importdialog je eCl@ss-Version ein Dateityp; ein **Schema** ordnet
  eCl@ss-Klassen und -Merkmale den EPLAN-Produktgruppen und Artikeleigenschaften zu.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_k_start.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_d_einstellungeneclass.htm
- **Klassenzuordnung**: Segment/Hauptgruppe/Gruppe/Untergruppe → standardmäßig Attribute [1]–[4]; Anwendungsklassen
  aus Segment 19 (IKT), 23 (Maschinenelemente), **27 (Elektro-, Automatisierungs-, Prozessleittechnik)**, 37
  (Rohrleitungstechnik); je 8-stellige Klassen-ID Zuordnung zu Produktober-/-gruppe/-untergruppe (Vorgabe
  Elektrotechnik / Allgemeine / Undefiniert).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_r_klassenzuordnung.htm
- **Merkmalzuordnung**: Merkmale „Advanced“ (Baum) oder „Basic“ (flach) → Artikeleigenschaft **oder**
  benutzerdefinierte Eigenschaft; Bedingungen (z. B. Dokumentpfad nur, wenn „Typ des Dokumentes = 2D-Grafik“).
  Merkmale sind nicht klassenabhängig zuordenbar (ID eindeutig). Mehrere Merkmale auf eine Eigenschaft → erste
  gewinnt, Systemmeldung. Mehrsprachig in den Sprachen des Katalogs; Einheit wird angehängt (bei freien
  Eigenschaften ins Einheitenfeld); feste Einheiten ggf. umrechnen. **Artikelnummer muss gefüllt sein.**
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_r_Merkmalzuordnung.htm
- **Kardinalität** (z. B. Zahl der Anschlüsse = wiederholter Merkmalblock): einem Merkmal mehrere EPLAN-Eigenschaften
  zuordnen (z. B. Attribute 10–20), Werte werden der Reihe nach verteilt, Überzählige ignoriert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_k_mappingdetails.htm
- **BMEcat-Feldzuordnung**: feste BMEcat-Felder ohne eCl@ss-Merkmal direkt auf Artikeleigenschaften (keine
  benutzerdefinierten/freien Eigenschaften).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_r_BMEcatFeldzuordnung.htm
- **Einstellungen**: **Hersteller-Kurzname** wird der Artikelnummer vorangestellt (`ABC` + `4711` → `ABC.4711`),
  auch Anschluss-/Bohrbildnamen; für spätere Data-Portal-Zuordnung **dreistelliges Kürzel** nötig. Optional
  Anschluss- und Bohrbilder automatisch erzeugen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_r_Eigenschaften.htm

**Bedeutung für QET:** Die EPLAN-Artikelnummer folgt dem Muster `<3-stelliges Herstellerkürzel>.<Bestellnummer>`
(z. B. `SIE.3RT2015-1BB41`, `PXC.3211757`). Der Fork kann daraus Hersteller und Bestellnummer zurückgewinnen, wenn
nur die Artikelnummer vorliegt (z. B. aus einer EPLAN-Stückliste im PDF), und umgekehrt beim Data-Portal-Abruf die
Artikelnummer bilden (Vermutung, deckt sich mit `manufacturerName()` im Fork-Code). Herstellerdaten nach BMEcat/
eCl@ss wären eine weitere Importquelle ohne Grafik (nur Artikeldaten und Anschlusszahl) – Stufe 2 der Hausregel.

## 17. eServices-Dialoge (Format, Klemmen erzeugen, Eigenschaften, Schutz) und Data Portal

- **Klemmen (Geräte) erzeugen** (Klemmenleisten-Navigator): erzeugt aus den **Funktionsschablonen eines
  Klemmenartikels** mehrere Klemmen nach **Nummerierungsmuster**; je Gerät wird die zuerst erzeugte Klemme
  Hauptklemme und trägt den Artikel. Beispiel der Hilfe: 4-Stock-Klemme mit Schablonen Etage 4 (mit Steg, 2 Anschl.),
  3, 2, 1 (PE mit Schienenkontakt); Muster `1,1+,1-,1PE,2-5PE` → 5 Mehrstockklemmen (20 Funktionen) 1/1+/1-/1PE …
  5/5+/5-/5PE; Zuordnung in Schablonen-Reihenfolge; Option etagenorientiert nummerieren. Artikel muss mindestens
  eine Klemmen-Schablone haben.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_klemmengeraeteerzeugen.htm
- **Benutzerdefinierte Eigenschaften** (Projekt oder Artikeldatenbank): identifizierender Name mit mindestens einem
  Punkt, nicht mit „EPLAN“ beginnend, Zeichen `.-_&`, Punkte = Baumebenen, Empfehlung Firmenkennung voran
  (`Firma.Gruppe.Eigenschaft`), nicht änderbar; Anzeigename mehrsprachig; **Zuordnung** (fest bei Erstellung):
  Abbruchstelle, Artikelreferenz (erzeugt indizierte Eigenschaft an allen artikeltragenden Objekten), Funktion,
  Projekt, Seite, Verbindung …; XML-Import/-Export.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_konfigeigenschaften.htm
- Eigenschaftsanordnungen: für Schaltzeichen je **Symbolvariante**, für 3D-Platzierungen je Funktionsdefinition.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_konfigeigenschaftsanordnungen.htm
- **Schutz**: Änderungsschutz (ID 3014, hierarchisch 3015), automatisch oder manuell; **Geräteschutz (ID 20475)**
  schützt vordefinierte Gerätefunktionen vor grafischen Änderungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_schutzkonfig.htm
- **Format-Dialog** (Blockeigenschaften, Baumformate): Gruppen Funktion, Artikelreferenz, Artikel, Hauptfunktion,
  Hauptfunktion des übergeordneten BM, andere Darstellungsarten, **andere Funktionen des BM** (erster Eintrag =
  Hauptfunktion), Gerätegruppe, **Ziel über Anschluss und Zielnummer** (Ziele semikolongetrennt), Verbindung über
  Anschluss (Anschluss 1 = Quelle, 2 = Ziel). Formatoptionen: Mindeststellen/Füllzeichen, **Mit übergeordneter
  Ebene vergleichen** (Strukturkennzeichen gleich der Umgebung ausblenden, auch BMK ID 20020), ausblenden wenn leer
  oder bestimmter Wert; Kommentar wird nicht ausgegeben.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_format.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatblockeigenschaft.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatkommentar.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formateigenschaft.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatartikeldatenzuweisung.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatberechnung.htm
- Format-Elemente der Online-Nummerierung und SPS-Adressen: Zähler mit Mindeststellen/Füllzeichen/Startwert,
  **alphanumerischer Zähler** (Zeichen „AKS“ → A, K, S, AA, AK …), Zahlensystem oktal/dezimal/hex mit Anfangs- und
  Endwert, Trennzeichen (Leerzeichen n-fach oder anderes Zeichen).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatelement.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatzaehler.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formattrennzeichen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formattrennzklein.htm
- Sonstige (knapp): Bearbeitungsbereiche (dürfen sich für Teilprojekte nicht überlappen, Bearbeiter aus der
  Rechteverwaltung), externe Dateien ins Projekt kopieren, Rückgängig-Liste (bis Projekt schließen).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_bearbeitungsbereichdefinieren.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_bearbeiterauswaehlen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_insprojektkopieren.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_rueckgaengigliste.htm
- **Data Portal in der Artikelverwaltung (2.9)**: Kontextmenü „Aktualisieren über Data Portal“ für markierte Artikel
  (Anmeldung + Lizenz **Data Portal Professional** nötig); beim Data-Portal-Import gilt der Dialog „Felder schützen“
  (Abschnitt 10); EDZ ist das Austauschformat; `uniqueid` ist die interne Data-Portal-ID; Funktionsschablonen
  zuweisen ist nicht rückgängig zu machen. Weiteres zum Data Portal (Import-Modi, DXF-Download, API):
  `makros-artikel.md` Abschnitt 14.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artverw.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_feldzuordnung.htm

**Bedeutung für QET:**
- Klemmen-Nummerierungsmuster `1,1+,1-,1PE,2-5PE` ist das EPLAN-Vorbild für Mehrstockklemmen-Bezeichnungen;
  ein Fork-Werkzeug „Klemmen erzeugen“ sollte dieselbe Syntax verstehen (Liste mit Komma, Bereich mit `-`,
  Etagen-Suffixe wiederholt) (Empfehlung).
- „Ziel über Anschluss“, „andere Funktionen des BM“ zeigen, wie EPLAN Blockeigenschaften (z. B. Klemmentexte mit
  Zielbezeichnung) berechnet – Vorlage für ein QET-Skript, das Zieltexte aus der Verbindung ableitet.
- Benutzerdefinierte Eigenschaften im EDZ (`Firma.X.Y`) sind keine EPLAN-IDs; der Fork sollte sie als
  Zusatzinfo ablegen statt zu verwerfen (Vermutung/Empfehlung).

## 18. Ergänzung: Artikelstammdaten-Navigator, Artikel aus Schaltplan erzeugen, Data-Portal-Einstellungen
**Artikelstammdaten-Navigator** (Dienstprogramme > Artikel): Überblick über die Artikelstammdaten, Platzieren per Drag & Drop.
- Feldbasierter Filter wirkt auf Baum und Liste; Volltextfilter durchsucht alle Felder, im Baum auch die Knoten Zubehörliste, Zubehörplatzierung, Bohrbild, Anschlussbild; braucht beim ersten Mal einen Suchindex ([Extras] > Suchindex aktualisieren).
- Die Liste zeigt nur Eigenschaften, die **alle** Artikel haben; „Kombination“ = Baum als Vorauswahl + Liste darunter. Die Registerkarte Info zeigt Datenbankname/-typ und Mengen (Artikel, Hersteller, Bohrbilder, Zubehörlisten).
- „Gerät einfügen“: vor dem Platzieren [Rücktaste] öffnet den Dialog Gerät einfügen (Makroauswahl, Darstellungsart). „Zuweisen“: Artikel bleibt am Cursor und kann mehreren Schaltzeichen zugewiesen werden. Drag & Drop mit [Strg]+[Umschalt] auf ein Schaltzeichen fügt den Artikel hinzu; hat das BM schon Artikel, wird nach Ersetzen oder Ergänzen gefragt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/PartBrowserGui_d_artikelnavigator.htm

**Artikel erzeugen** (Kontextmenü an einer Hauptfunktion mit mindestens einer Artikelreferenz): legt aus den Schaltplandaten einen Artikel an. Vorgeschlagen wird die **erste** Artikelreferenz als Artikelnummer (änderbar; existiert sie schon, kommt eine Meldung); Produktober-/Produkt-/Untergruppe sind aus der Funktionsdefinition vorbelegt; optional öffnet danach die Artikelverwaltung mit dem neuen Artikel.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsextensiongui_d_artikelerzeugen.htm

**Data Portal – Benutzereinstellungen** (Optionen > Einstellungen > Benutzer > Verwaltung > Data Portal):
- *Portal*: Portal-URL ist vorbelegt, Timeout in Sekunden; Benutzername/Passwort sollen nur für das Data Portal verwendet werden; [Konto erstellen] registriert.
- *Optionen*: „Heruntergeladene Dateien überprüfen“ prüft, ob Zusatzdokumente Dritter gegenüber dem Serverstand verändert wurden.
- *Info*: Version und Status des Add-ons; [Prüfen] testet die Verbindung und zeigt die Portalversion.
- Proxy und Verbindungstest für alle Internet-Komponenten liegen unter „Internetverbindungen“ (siehe einstellungen-oberflaeche.md).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.edata_r_portal.htm · https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.edata_r_optionen.htm · https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.edata_r_info.htm

**Bedeutung für QET**: „Artikel erzeugen“ entspricht im Fork dem Zurückschreiben eines Elements mit Artikeldaten in die Benutzersammlung; die erste Artikelreferenz ist die führende Nummer. Der Data-Portal-Zugang selbst ist für QET irrelevant (der Fork importiert heruntergeladene EDZ/Makros).

## 18a. Ergänzung: Data Portal und Artikelverwaltung laut Student Handbook

Quelle: EPLAN Education Handbook „Student Handbook“, Stand 16.12.2021,
https://www.eplan.com.tr/fileadmin/eplan_content_international/tr/data/Student_Handbook.pdf

- Data Portal (Stand 2021): über eine Million Datensätze von über 300 Herstellern, mit 2D- und 3D-Makros, Eigenschaften,
  Bildern und Dokumenten; Geräte direkt in den Schaltplan oder zuerst in die Artikelverwaltung übernehmen. (S. 23–25)
- Kabel aus dem Data Portal bringen die Aderdaten mit; Kabeltyp tauschen = Kabelartikel ersetzen. (S. 26)
- Funktionstexte hängen am Gerät und erscheinen in Auswertungen (z. B. Geräteliste neben dem BMK); normale Texte sind
  nicht mit Geräten verknüpft. (S. 27)
- Auswertungen lassen sich filtern und sortieren, u. a. SPS-Kartenübersicht mit freien Anschlüssen; jede Auswertung ist
  als XML, Text oder Excel exportierbar (z. B. für Beschriftungsmaschinen). (S. 44–45)

## 99. Quellen
Artikelverwaltung:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_artverwnutzen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvartikel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_feldlaengenfelder.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvanschluesse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvkonstruktion.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvzubehoer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_konfigurationcsvadressen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_feldlaengeneigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_zubehoerlisten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_module.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_steuerdateiimportexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_importexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelfiltern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_baumkonfiguration.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_listenutzen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_kundenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_mehrsprachigeartikelverwalten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_preisewaehrungenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_baugruppenanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_funktionsschablonenzuweisen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_moduleanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_zubehoerlisteerstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_allgemein.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_technischedaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_funktionsschablone.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_funktionsdefinition.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_modul.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_baugruppe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_zubehoer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_anschluesse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_einstanschlussausf.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_anschlussklemmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_datenauswertung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_eigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_attribute.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_zusatztexte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_konfigeigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_dokumente.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_sicherheitskennwerte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_anwendungsfall.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_bohrbild.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_montagedaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_fertigung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artikelexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artikelimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_feldzuordnung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_einstellungenartverw.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artverw.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_automatikfilter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_gewerk.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_baumkonfiguration.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_baumkonfigurationhauptknoten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_preiseberechnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_waehrungumrechnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_preise.htm

Artikel- und Geräteauswahl:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_artikeldatenergaenzen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_konfigapiartikel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikelausschaltplan.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikeldateneinlagern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_eingelagerteartikeleigenschaftenloeschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_artikeleigenschaftenloeschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_adpzeichnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_k_uebertragenedaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikelauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_konflikte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_artikelauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikeldatenanschaltzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_r_artikel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_geraeteauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_geraeteauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_k_vorgehensweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_einstellungengeraeteauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_k_filtervorauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_geraetefilter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikelvorauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_artikelvorauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_datenauswahlbenutzer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_datenauswahlprojekt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_datenquelleauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_h_artikelauswahlnavigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partselectiongui_d_adresseauswaehlen.htm

Artikelabgleich/-vorfilterung:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsextensiongui_d_artikelabgleich.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsextensiongui_d_artikelvorfilterung.htm

Funktionspool / Reservebauteile:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_geraetedefinitionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_funktionsschablonen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_datenvonnichtplatziertenfunktionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_arbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_h_funktionenzurueckbuchen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_kopiermodifunktionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_loeschmodifunktionen.htm

Stammdaten:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_d_einstellungenstammdatenallg.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_k_datenablage.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_makrosverwalten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_d_einstellungensymbolbibl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_stammdatenabgleichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_artikeldatenverwalten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_stammdatenprojektvervollstaendigen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_formulareinstellungenfestlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmasterdatasettingsgui_h_stammdatenverwalten.htm

Pro Panel: Bohrbilder:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/propanelmasterdata_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/propanelmasterdata_h_bohrbildausdxfdaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/propanelmasterdata_h_bohrbildausncdaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/propanelmasterdata_h_bohrbildgrafischdefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/propanelmasterdata_h_bohrbildkasteneinfuegen.htm

BMEcat/eCl@ss-Import:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_d_einstellungeneclass.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_r_klassenzuordnung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_r_Merkmalzuordnung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_k_mappingdetails.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_r_BMEcatFeldzuordnung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportbmecatgui_r_Eigenschaften.htm

eServices-Dialoge:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_klemmengeraeteerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_konfigeigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_konfigeigenschaftsanordnungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_schutzkonfig.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_format.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatblockeigenschaft.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatkommentar.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formateigenschaft.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatartikeldatenzuweisung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatberechnung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatelement.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formatzaehler.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formattrennzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_formattrennzklein.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_bearbeitungsbereichdefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_bearbeiterauswaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_insprojektkopieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_rueckgaengigliste.htm

Nachtrag:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_konstruktion.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_montageplatten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_platzierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_sammelschiene.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_sammelschienen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_sperrflaeche.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_tragschienen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_tueren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_zubehoerartikel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_zubehoerplatzierung.htm

Zusatz-Dialogseiten (Ergänzung Abschnitt 18):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/PartBrowserGui_d_artikelnavigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsextensiongui_d_artikelerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.edata_r_portal.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.edata_r_optionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.edata_r_info.htm
