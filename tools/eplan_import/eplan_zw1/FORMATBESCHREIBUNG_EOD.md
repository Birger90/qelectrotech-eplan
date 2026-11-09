# EPLAN .zw1 / EOD – unabhängig dekodierte Struktur (13 Projekte, EPLAN 2.7 / 2.9 / 2023 / 2026)

Grundlage: die 234 entpackten Originaldateien (SHA-256 gegen das mitgelieferte Manifest geprüft: 234/234 identisch).
Projektversion laut `_BAKINFO.XML`: prodversion 2026.0.3.25480, appvariant Basic.
Alle Angaben gelten empirisch für diesen Datenbestand. Ob sie für andere EPLAN-Versionen gelten, ist nicht geprüft.

Kennzeichnung: **FAKT** = durch Code am Datenbestand nachgewiesen · **BELEGT** = Bedeutung durch die EPLAN-API-Doku (eplan.help) gestützt · **HYPOTHESE** = abgeleitet, nicht unabhängig bestätigt.

## 1. Datensatzkopf (alle EOD-Tabellen mit fester Satzlänge)
- FAKT: Satzlänge ist tabellenfest. Dateigröße / Satzlänge ergibt in allen 60 Tabellen mit Daten einen Rest von 0, und alle IDs sind eindeutig.
- FAKT: Bytes 0–3 = Objekt-ID (u32 Big Endian), Bytes 4–5 = Objektklasse (u16 BE). Eine Tabelle kann mehrere Klassen enthalten, z. B. Function.eod: 0x11, 0x3B, 0xAA, 0xAF, 0xB4 …
- FAKT: Die letzten 5 Bytes jedes Satzes sind ein Trailer `0A 00 00 00 xx`.

## 2. Eigenschaftsblock
- FAKT: Jeder Datensatz enthält genau einen Eigenschaftsblock an tabellenfester Position:
  - inline: Der Block beginnt direkt mit `61 EA <Klasse u16 LE> <ID u32 LE>` (Selbstverweis).
  - zlib: 9 Byte Kopf (Flag, u32, u32 = komprimierte Länge), danach ein zlib-Strom.
- Positionen (inline / zlib-Daten): Connection 375/383, Function 328/336, Page 129/137, Article 135/143, ArticleRef 422/430, IPoint 173/181, Relation 39/47, Potential 108/116, Plc 725/733, Location 111/119, Instance 60/68, Message 101/109, PlaceHolderText 48/56.
- FAKT: Flag 1 = Überlauf. Der Datensatz enthält nur den Anfang des zlib-Stroms (bis 5 Byte vor Satzende). Der Rest liegt in Blob.eod desselben Ordners, in 256-Byte-Blöcken:
  `0A | Tabelle u16 BE | Objekt-ID u32 BE | Blocknummer u16 BE | Blockanzahl u16 BE | 01 | 244 Byte Nutzdaten`.
  Zusammengesetzt wird nach Blocknummer. Akzeptiert wird nur bei zlib-Ende mit gültiger Adler-32-Prüfsumme. In Projekt Projekt A wurden so alle 483 Überlauf-Datensätze vollständig rekonstruiert.

## 3. Eigenschaftsstrom
Folge von `<Property-ID u16 LE><Wert>`. Ende bei ID 0x0000. Der Typ hängt von der ID ab. Die Typzuordnung wurde per Backtracking-Parser mit Mehrheitsentscheid über alle Datensätze ermittelt (`types.json`).

| Typ | Kodierung |
|---|---|
| b1 / s2 / i4 / i8 | 1 / 2 / 4 / 8 Byte LE |
| str | UTF-8, nullterminiert |
| ref | Objektverweis: Klasse u16 + ID u32 (LE) |
| rp | Anschlussverweis: Klasse u16 + ID u32 + Anschlussindex u32 |
| d8 / p16 | double / Punkt (2× double) |
| kv | Liste `(EPLAN-Eigenschaftsnr. u16, Index u16, str)`, Ende 0xFFFF |
| obj | Unterobjekt: Klasse u16, Eigenschaften bis 0xFFFF |
| dN | n× double ohne Längenpräfix (eb1e/eb1f), n per Backtracking |
| 0x0380 | Array: u32, u8, Anzahl u32, Anzahl× str (z. B. Anschlussbezeichnungen) |
| 0xFFFF auf oberster Ebene | einmaliger Abschnittstrenner (vor den Anschlussdaten) |

- BELEGT: Die Schlüssel in den kv-Listen sind die offiziellen EPLAN-Eigenschaftsnummern, z. B. 31002 Querschnitt, 31004 Verbindungsfarbe/-nummer, 31006 Potenzialtyp, 20030 Klemmen-/Anschlussbezeichnung (eplan.help API).
- FAKT: Mit dieser Grammatik vollständig bis zum Endmarker parsebar (Rest nur Nullbytes):
  - Connection: 1504/1504
  - Relation: 318/318
  - Location: 31/31
  - Plc: 20/20
  - Function: 3227/3261
  - Page: 131/133
  - ArticleRef: 776/777
  - IPoint: 264/303
  - Article: 1075/1259 (Rest: abgebrochener zlib-Strom)
  - Instance: nur 60 benötigte Objekte geparst, 50 davon erfolgreich

## 4. Verbindungen (Connection.eod, Klasse 0x12)
- FAKT: eb2b = Objektverweis Seite A, eb2e = Seite B. eb2c / eb2f = 0-basierter Anschlussindex in die Anschlussliste der Funktion; fehlt das Feld, gilt 0.
- FAKT: fa03 / fa04 sind EPLAN-eigene Endpunkt-Texte (A/B). Sie sind nur bei einem Teil der Verbindungen gespeichert und dienen hier als unabhängige Referenz.
- FAKT: Endpunkt-Text = BMK (`=`1100[`.`1101] `+`1200[`.`1201] `-`[20012-Wert]20013+20014 [`.`20015] [`-`20017+20018]) [`:` 20447 + (20406 | 20030 | eb22)] [`:` Anschluss].
  - Abgleich: 2.088 von 2.109 Enden mit EPLAN-Text sind identisch (99,0 %).
  - Verbindungen mit beiden Texten: 1.028 von 1.049 beidseitig identisch.
- Verweisziele: Function (Klasse 17), IPoint (70), Potential (93, Name in f0fb), Instance-Objekt Klasse 130 (Text in Eigenschaft 19071, Bedeutung unbelegt).

## 5. Funktionen (Function.eod)
- FAKT: eaf6 = Bezeichnungsteile (kv). eaf1 = Bezeichnungsschablone, z. B. `[20012]<20013><20014><[20015]20015>`. ee2c/ee2d = Kennbuchstabe/Zählnummer (redundant zu 20013/20014).
- FAKT: Anschlüsse liegen nach dem Trenner: eb19 = Anschlussbezeichnung (je Anschluss ein Eintrag) oder Array 0x0380. eb1a = zweite Textliste je Anschluss (HYPOTHESE: Anschlussbeschreibung).
- FAKT: ebf2 = Verweis auf die Seite (Klasse 4). ee36…ee40 = Verweise auf Location (Klasse 6). ea65 = Unterobjekte Klasse 0x35 je Anschlusspunkt, mit Koordinaten in ec55.

## 6. Seiten (Page.eod)
- FAKT: ea91 kv: 1100 / 1200 / 11012 ergeben `=Anlage+Ort/Seitennummer`. 11012 als Seitennummer ist HYPOTHESE, gestützt durch den fortlaufenden Wert und eead.

## 7. Offen / nicht dekodiert
- 34 Funktionsdatensätze (Klasse 0xB4 u. a., 19 davon mit abgebrochenem zlib-Strom)
- 184 Artikel mit ausgelagerten Daten
- Blob/XBlob-Struktur
- Bedeutung vieler Property-IDs ohne kv-Nummer (eaxx–faxx)
- Formatierung leerer Strukturkennzeichen („=+“) in 13 Fällen
- Steckerbezeichnung X1 bei −A20 (2 Fälle)
- Instance-Klasse-130-Objekte
- Kabel-/Aderzuordnung

## 8. Satzlängen-Autoerkennung und Versionen
- FAKT: Satzlänge steht im Kopf der Indexdatei `.eox` (Bytes 0x0D–0x0E BE + 5, siehe Abschnitt 19); `autodetect.py` liest sie dort und sucht nur ohne `.eox` nach einer passenden Länge. Blockposition: Selbstverweis 61 EA an fester Stelle, zlib-Strom 8 Bytes dahinter.
- FAKT: Die Satzlängen unterscheiden sich je nach Version und Projekt, z. B. Connection 980 (2026) und 892 (2.7). Der Aufbau des Eigenschaftsstroms ist in 2.7, 2023 und 2026 gleich.
- Validierung über 5 Projekte: 10.517 Verbindungsenden mit EPLAN-eigenem Text, davon
  - 10.328 zeichengleich,
  - 182 nur mit abweichendem leerem Strukturpräfix („=“, „=+“),
  - 7 echte Abweichungen: 6 Verweise auf eine im Archiv nicht vorhandene Funktions-ID und 1 Fall in EPLAN 2.7, in dem 20447 nicht angezeigt wird.
- Offen: Funktionsklassen 0xAF, 0xAA, 0xB4 und 0xB3 in Projekt A (232 Objekte, 3D-/Montage- und Grafikobjekte). Keine Verbindung verweist auf sie.

## 9. Artikel und Artikelreferenzen (Stückliste)
- BELEGT: Article.eod, kv ea6b mit EPLAN-Artikeleigenschaften: 22001 Artikelnummer, 22003 Bestellnummer, 22004/22005 Bezeichnung 1/2, 22007 Hersteller, 22017 Technische Kenngrößen, 22023 Satzart, 22056 ERP-Nummer (eplan.help, ArticlePropertyList).
- FAKT: Alle Sätze der Klasse 0x75 haben 22001 (Projekt A: 996/996). Die anderen Klassen sind andere Satzarten laut 22023, zum Beispiel Herstelleradressen, Zubehörlisten oder Bohrbilder.
- FAKT: ArticleRef.eod enthält:
  - ef57 = Verweis auf die Funktion
  - ef59 = Artikelnummer (Text)
  - ef5c = BMK-Teile (kv)
  - ef60 = Seitenverweis
- HYPOTHESE: ef5b = Menge. Die Werte sind meist 1, vereinzelt 2, 3 oder 8. Nicht gegen EPLAN geprüft.
- Konsistenzprüfung: Artikelnummer aus ArticleRef im Artikelstamm gefunden
  - Projekt A: 576 von 586
  - Projekt B: 550 von 550
  - Projekt C: 667 von 673
  - Projekt D: 582 von 588
  - Projekt E: 545 von 558
  Bei den übrigen fehlt der Stammsatz im Archiv (FAKT).
- FAKT: In Projekt E verweisen 38 Artikelsätze auf Blob-Fortsetzungen, die im Archiv nicht enthalten sind. Sie sind nicht rekonstruierbar.

## 10. Klemmenplan
- Grundlage: Funktionen mit der Eigenschaft 20030 (Klemmen-/Anschlussbezeichnung, BELEGT). Je Anschluss werden alle Verbindungen mit Ziel, Farbe/Nummer (31004) und Querschnitt (31002) aufgelistet.
- Die Zuordnung „Leiste = BMK der Funktion“ entspricht der EPLAN-Textform `BMK:Klemme:Anschluss`, die in Abschnitt 4 gegen die EPLAN-Texte validiert ist.

## 11. Kabel und Adern
- BELEGT: Das Kabelobjekt ist eine Funktion mit 20040 Kabeltyp, 20042 Aderanzahl, 20043 Querschnitt und 20044 Spannung (eplan.help, Properties.Cable).
- FAKT: Aderobjekte haben die Klasse 0x3B in Function.eod. ee67 = Rückverweis auf das Kabel, 31004 = Aderfarbe/-nummer, 31002 = Querschnitt.
- FAKT: Connection eb33 = Verweis auf die Ader. Daraus ergibt sich der Kabelplan: Kabel → Ader → Quelle/Ziel.
- Konsistenzprüfung: Die Zahl der Aderobjekte ist kleiner oder gleich 20042 bei 500 Kabeln. 4 Kabel haben mehr Aderobjekte als 20042 angibt, als Widerspruch ausgewiesen.
- FAKT: Kabel ohne BMK im Archiv erscheinen als „(ohne BMK)“.

## 12. Validierung über 6 Projekte (Stand Abschluss)
- 15.236 Verbindungsenden mit EPLAN-eigenem Text:
  - 14.918 identisch
  - 225 nur mit leerem Strukturpräfix
  - 56 mit Verweis auf eine im Archiv fehlende Objekt-ID
  - 37 abweichend
- Von den 37 Abweichungen: 36 in Projekt F, wo der EPLAN-Text ein anderes Gerät nennt als der referenzierte Datensatz. HYPOTHESE: veraltete EPLAN-Texte. Gestützt dadurch, dass ein Teil der Texte `=SB+CA-A19` nennt und ein Gerät A19 im Archiv nicht existiert.
- Die übrige Abweichung (Projekt B, EPLAN 2.7): EPLAN zeigt den Steckerpräfix 20447 nicht an.

## 13. Erweiterung auf 13 Projekte (inkl. EPLAN 2.9)
- FAKT: Ein Wert in 20012 ist ein eigenes Präfix, z. B. `-12X1`. Die Schablone `<20012>` setzt den Wert ein, `[20012]` nur das Trennzeichen „-“.
- FAKT: In EPLAN-Verbindungstexten fehlt das „-“, wenn das BMK kein Strukturkennzeichen hat. Beobachtet 13 von 13 Fällen, alle EPLAN 2.9. Die Regel gilt nur für Endpunkt-Texte.
- BELEGT/FAKT: Typnummer = 22002. Abgleich mit EPLAN-BMK-Exporten: 522 von 528 gleich.
- FAKT: Verwaiste Verbindungen verweisen auf Funktionen und Seiten, die im Archiv fehlen. Sie sind in der Spalte `hinweis` markiert. Zu einem solchen Objekt (Projekt D) enthält Message.eod einen EPLAN-Eintrag der Kategorie „TerminalLog“.
- Validierung über alle 13 Projekte:
  - 18997 Verbindungen
  - 27628 Enden mit EPLAN-Text, davon:
    - 26811 identisch
    - 335 nur leeres Präfix
    - 428 Verweis ins Leere
    - 54 abweichend
  - Die abweichenden Fälle sind überwiegend Texte, die einen anderen Ort bzw. ein anderes Gerät nennen als der in sich widerspruchsfreie Datensatz. HYPOTHESE: veraltete EPLAN-Texte.

## 14. Abgleich mit EPLAN-Berichtsexporten und Schaltplan-PDF
- Exporte: BMK-Liste, Klemmenplan (KLP) und Kabelübersicht als xlsx/xls aus den Projektordnern. Sie sind teils Monate älter als die ZW1.
- Ergebnis je Projekt: `groundtruth.json`.
- Schiedsrichter bei Abweichungen: die Schaltplanseiten des zeitgleich mit der ZW1 erzeugten PDFs. Berichtsseiten im PDF sind ausgenommen, weil EPLAN-Auswertungen Momentaufnahmen sind; sie trugen z. B. das Datum 08.09.2025, während die ZW1 vom Mai 2026 stammt.
- Ergebnis Schiedsrichter: Wenn der Schaltplan genau einer Seite recht gibt, war es immer die ZW1-Auswertung (31 Kabeltypen, 36 Aderzahl/Querschnitt-Angaben). Fehlende Kabel bzw. Klemmen gibt es im aktuellen ZW1-Stand nicht mehr. Details: `pdfcheck.json`.

## 15. Symbolgrafik, Seitengrafik, Kästen und Eigenschaftstexte (Stufe 2, Claude, Projekt A)
Gelesen mit `eod_symbole.py` (Satzleser mit Blob-Überlauf und Typ-Mehrheitsentscheid je Tabelle) und
`eplan_grafik.py`. Grundlage nur Typen aus `seed.json` plus feste Grafiktypen; die Typvotes der Haupttabellen
verfälschen die Instance-Tabellen (z. B. ec00, ec5f) und werden dort nicht benutzt.
- FAKT: Überlauf in Blob.eod: 256-Byte-Block `<Tabelle u16 BE><ID u32 BE><Nr u16 BE><Anzahl u16 BE><Flag u8>`,
  dann **244 Byte** Nutzdaten und 1 Füllbyte; im Projekt-Blob zusätzlich ein führendes 0A. Der Satzrest endet vor
  dem letzten Byte (0A). Damit lesbar: SPECIAL Symbol 356/356, IEC_symbol Symbol 932/932, Variant 7877/7890,
  Seiten-Instance 58761/59495.
- FAKT: Symbolbibliothek `*.sdb`: Symbol.eod Klasse 20 (eb51 Name, eb52 Symbolnummer, efc5/efc6/efc7
  Funktionsdefinition, 60011 kv 16011 Beschreibung); Variant.eod Klasse 45 (ed95 → Symbol, ed96 Variante 0–15 = A–P,
  fehlt = A; Anschlusspunkte: edbe Lage + edbf Richtung 1 oben/2 rechts/4 unten/8 links, weitere in edbd-Objekten;
  fehlt edbe, liegt der Punkt im Einfügepunkt); Instance.eod: Grafik mit ebf1 = (45, Variante).
- FAKT: Funktion → Symbol: ef4d Bibliotheksindex, ef4e Symbolnummer (= eb52), ef4f Variante. In Projekt A:
  0 = SPECIAL.sdb, 1/2 = IEC_symbol.sdb (Zuordnung je Projekt über die beste Übereinstimmung der
  Funktionsdefinition bestimmt; die Bibliotheksreihenfolge steht nicht im Archiv). Gleiches gilt für Seiten-
  Instance Klasse 42 (Symbolplatzierungen ohne Funktion: Winkel, Abzweige).
- FAKT Grafikklassen: 31 Linie (ec73/ec74, fehlt = 0/0), 32 Bogen/Kreis (ec8b Mitte, ec8c Radius, ec89/ec8a
  Start-/Endwinkel rad, gegen den Uhrzeigersinn, beide 0 = Vollkreis), 34 Polylinie (eccd-Punkte; ebf4 Bit 64 =
  gefüllt), 44 Spline (4 Bézier-Punkte), 89/128 Rechteck (f0d3/f0d4, f0d5 Drehung), 30/83 Text (ec55 Anker,
  ec59{ee21 Höhe mm, ee22 Winkel rad, ee26 Ausrichtung}, ec5f Text „??_??@…;“ bzw. „de_DE@…;“), 91 Bild
  (f0e7 `$IMG\…`), 26 Gruppe/Makrokasten ohne Geometrie. ebfb = Ebene (Layer.eod eff2), ebfd Linientyp.
- BELEGT am PDF: Textausrichtung ee26: 1–3 oben, 4–6 Mitte, 7–9 unten (Unterkante), 10–12 Grundlinie, je
  links/mittig/rechts; fehlt = 7. Das EPLAN-PDF hat 1,06 mm Rand (Seite 422,5 × 304,5 mm).
- FAKT Ausdehnung: Gerätekasten/Black Box und Kabeldefinitionslinie in eb1e/eb1f (dN, [x, y] min/max relativ
  zum Einfügepunkt; Höhe 0 = Linie), Strukturkasten (Klasse 88) in f0bf/f0c0 (p16 min/max).
- FAKT Eigenschaftstexte (ea65, Klasse 53, an Variante und Funktion; die Funktion schlägt die Variante):
  ed63 Eigenschaft, ed64 Index (bei 20028/20029 der Anschluss, 1-basiert), ec55 Lage, ec56 Bezug auf einen anderen
  Eintrag ((65535, 2147483647, 0) = keiner), ec57 Bit 2 = ausgeblendet (Anschlussbeschreibungen 20029/20039
  zeigt EPLAN trotzdem, z. B. DI3/VS3 im SPS-Kasten), ee19 1/2 = Lage für waagerechten/
  senkrechten Pfad. 20028/20029 liegen relativ zu ihrem Anschlusspunkt (am Öffner der Variante C geprüft:
  21 unten, 22 oben wie im PDF).
- HYPOTHESE (am PDF gestützt): Klemmen/Steckerstifte zeigen nur 20030, das BMK steht an der Leisten-/
  Steckerdefinition (TDEF/PDEF); SPS-Anschlüsse (Funktionsdefinition 300) zeigen kein BMK; Funktionen im eigenen
  Gerätekasten ebenso.
- FAKT SPS-Kasten: 20400 SPS-Adresse (kv), 20038/20039 alle Anschlussbezeichnungen/-beschreibungen, 20901 Zusatzfelder
  (Index 15/16 = Kartentext/Baureihe) an der Hauptfunktion gleichen BMKs, 20201 Blockeigenschaft = ermittelte
  symbolische Adresse 20403: über die Verbindungen durch Reihenklemmen (Funktionsdefinition 100) bis zum ersten
  anderen Gerät („+FP1-XB180:1“, am PDF bestätigt).
- Offen: Bilder (Klasse 91) einbetten, Querverweistexte (/Seite.Spalte), Pfad-/Formularrahmen (Besitzer 51),
  Objekte mit Besitzer Klasse 21, genaue Sichtbarkeit einzelner Eigenschaftstexte.

## 16. Makrogruppen, Black Boxes, Haupt-/Nebenfunktionen (Stufe 3)
- FAKT: Seiten-Instance Klasse 26 = Makrogruppe (Gruppe eines eingefügten Makros), ebf1 = Elterngruppe bzw. Seite;
  Grafik und Funktionen tragen in ebf1 ihre innerste Gruppe (ebf2 = Seite). Ein Makro = alles unter derselben Gruppe.
- FAKT: Gerätekasten/Black Box = Funktion mit Funktionsdefinition 1303 (SPS-Karte 301) und Ausdehnung eb1e/eb1f;
  darin Geräteanschlusspunkte (1302, Symbole DCP…) mit Anschlussbezeichnung, Kontakte, Grafik, Texte. Ein Makro
  kann mehrere Kästen haben (Sanftstarter: Leistungs-, Steuer-, Thermistorteil in je eigener Gruppe).
- FAKT: Hauptfunktion = Funktion ohne eb16, Nebenfunktion = eb16 0 (Kontakte, Geräteanschlüsse, SPS-Anschlüsse);
  der Artikel hängt an der Hauptfunktion; zusammen gehören sie über das gleiche BMK (Querverweis/Kontaktspiegel).
- FAKT: Kontaktart aus der Funktionsdefinition: 201 Schließer, 202 Öffner, 203/204 Wechsler; 200 Spule;
  5xx Schutzorgane (Leitungsschutz, Motorschutz), 600/601 Befehlsgeräte.
- Bildpfade: `$IMG\…` und `$(IMG)\…` (beide vorkommend); Bilder der Formulare (Besitzer 51) sind Logos.
- Import: Kasten + Gruppeninhalt -> EIN QET-Bauteil (Master, Anschlüsse „Gerät:Anschluss“), mit Artikel als
  `<Bestellnr>_EPLAN-Schaltbild_BlackBox_…` in die Sammlung; Hauptfunktion -> Master (coil/protection/commutator),
  Kontakte -> Slave (NO/NC/SW), per link_elements verknüpft. Element-UUIDs müssen je Datei eindeutig sein (Name),
  sonst zeichnet QET alle gleichnamigen Elemente mit derselben Grafik. Bilder bleiben freie Bilder auf dem Blatt
  (QET-Elemente können keine Bilder enthalten).

## 17. Eigenschaftstexte, Auswertungen, Maßstab, Abbruchstellen (Stufe 4, am Original-PDF geprüft)
Prüfwerkzeug: `pdf_vergleich.py <original.pdf> <import.pdf>`: je Blatt Anteil der Originaltexte an gleicher
Stelle (±4 mm) und Anteil zusätzlicher Texte; Blattzuordnung über das Schriftfeld (=Anlage +Ort Blatt).
- **Textvorgaben aus der Ebene** (Layer.eod: eff2 Nummer, ee21 Höhe, ee26 Ausrichtung): Texte ohne eigene Höhe/
  Ausrichtung erben sie von ihrer Ebene (ebfb). Beispiel: Ebene 110 „Pfad-Funktionstexte“ = oben mittig.
- **Mehrzeilige Texte:** EPLAN richtet die ERSTE Zeile aus, weitere Zeilen laufen im Abstand 1,48 × Schrifthöhe
  nach unten; jede Zeile wird für sich links/mittig/rechts ausgerichtet.
- **Bezüge zwischen Eigenschaftstexten** (ec56 ohne ec55): EPLAN stapelt den Text eine Zeile neben den Bezugstext,
  leere Texte belegen keine Zeile. Richtung: Bezug oben ausgerichtet (1–3) -> darunter, sonst darüber
  (z. B. Aderfarbe „DBU“ über Querschnitt „0,5“; „-A9“ über „=SB+CA/55/6“). ec57 Bit 2 markiert nur den Bezug
  (kein Ausblenden); Bit 64 = in derselben Zeile anhängen (Abbruchstellen: „Name / Verweis“).
- **Index:** ed64, bei Artikeleigenschaften (22xxx) ed66 (Artikelposition 1, 2 …). Zusatzfelder 20901 genau nach
  Index (13/15/16 = Hersteller/Kartentyp/Baureihe); Werte an der Hauptfunktion gleichen BMKs.
- **Ermittelte Eigenschaften** (stehen nicht im Archiv, nachgebildet):
  - 20300/20301 Querverweis „=Anlage+Ort/Seite/Spalte“ (Anlage/Ort nur wenn abweichend; Spalte = x // 42 mm bei
    10 Spalten); 20301 (Übersichtsquerverweis) nur in der Übersichtsdarstellung (ohne Symbol bzw. Symbolname …OV);
    20306 zur Hauptfunktion (ohne eb16) gleichen BMKs.
  - Gegenstück: gleiches BMK und Anschluss auf anderer Seite (SPS-Übersicht <-> allpolig).
  - 20201/20403/20404 symbolische Adresse: über die Verbindungen durch Reihenklemmen (Funktionsdefinition 100)
    bis zum ersten anderen Gerät; 20404 ohne Anschluss/Ort.
  - 20031 Funktionstext (automatisch): eigener Funktionstext 20011 bzw. der des Gegenstücks, sonst der
    Pfad-Funktionstext (Ebene 110) derselben Spalte.
  - 20038/20039 alle Anschlussbezeichnungen/-beschreibungen; 20435 Anschlussbezeichnung (Übersicht).
- **SPS-Anschluss (300) im Schaltplan:** trägt BMK des Kastens (20010) und Hauptfunktions-Querverweis (20306) an
  der eigenen Lage; Klemmen/Steckerstifte zeigen 20030 statt BMK; im Gerätekasten erscheint ein BMK nur einmal.
- **Auswertungsseiten:** Seite kv 11015 = Formularname -> Form.eod (ede6 Name, ede5 Typ) -> Formulargrafik
  (Instance, Besitzer (51, Formular)); Inhalte = PlaceHolderText.eod (Klasse 52, Besitzer Seite, ec5f fertiger Text,
  ee0d/ee0f Eigenschaften). Damit Klemmen-, Kabel-, Stücklistenseiten wie im Original (94–96 % gleiche Texte).
- **Maßstab:** Seite kv 11048 (z. B. 10 = 1:10, Schrankaufbau). Alle Lagen UND Schrifthöhen auf solchen Seiten
  sind Modellmaße -> durch den Maßstab teilen.
- **Bemaßung** (Instance Klasse 97): f141/f144 Messpunkte, f147/f148 Maßlinie, f149 Textlage, f14a Text (fehlt ->
  Abstand, mit Dezimalkomma).
- **Texte „&…“** sind interne Verweise (Makro-/Artikelkürzel); gedruckt nach ihrer Ebene (meist 1028 „Grafik.Steuertext“,
  nicht druckend – siehe Abschnitt 19); „{{…}}“ = Makrotext.
- **Bilder:** Pfad `$IMG\…` oder `$(IMG)\…`; QET setzt Bilder über den Mittelpunkt des UNskalierten Bildes.
- **Abbruchstellen (IPoint):** Name = 20012+20013+20014 (z. B. „5L4+“); gleichnamige paarweise in Seitenfolge
  (1↔2, 3↔4); Symbol SPECIAL „BP“, Variante %4: 0 rechts, 1 oben, 2 links, 3 unten (4–7 gleich). Text auf der
  offenen Seite (weg von der angeschlossenen Leitung), senkrecht von unten nach oben lesbar; „Name / Verweis“,
  rechtsbündig „Verweis / Name“; Verweis an Pfeilen ohne führenden Schrägstrich („12/2“, „=SB/57/1“).
- **QET-Fallen beim Nachbearbeiten des .qet:** Texte nur in den `<inputs>` des Blatts (Einrückung 8), nicht in den
  gleichnamigen eines Elements; Element-UUIDs je Datei eindeutig (Name), sonst gleiche Grafik für alle.
- **Stand Projekt A-Testprojekt:** 73,5 % der Originaltexte an gleicher Stelle über 118 Blätter (vor Abbruchstellen-
  und SPS-Kasten-Regeln), 30 Blätter ≥ 90 %. Offen: Positionsnummern im Aufbau (EPLAN-berechnet), perspektivische
  Makros (LazerSafe), Kabeltexte, Strukturkasten-Beschriftung senkrecht.

## 18. Verallgemeinerung auf weitere Projekte (Stufe 5, drei Projekte am Original-PDF geprüft)

- **Projektdaten fürs Schriftfeld:** `x/ProjectInfo.xml` (Property id/index/type): 10011 Projektbeschreibung
  (Anlagenbezeichnung, mehrzeilig, „??_??@…;“), 10013 Projektnummer (Zeichnungsnummer), 10014 Kommission
  (Auftragsnummer), 10015–10017 Firma/Adresse, 10020/10021 Ersteller/Datum. Nie Werte aus einer Schriftfeld-Vorlage
  übernehmen (die KUNDE-Vorlage trägt sie als SVG-Bild -> je Projekt neu erzeugen).
- **Seitenname:** 11012 Seitennummer + 11013 Seitenunterzähler -> „1“, „1.1“, „1.a“; Sortierung numerisch, dann
  Unterzähler. Leere Anlage/Ort: kein einzelnes „=“/„+“ im Schriftfeld. 11050 = „Automatikseite geändert“ (keine
  Ausblendung), 11006 = automatisch erzeugt.
- **Bezugspunkt von Eigenschaftstexten an Kästen (ee1a):** ec55 ist relativ zu einem Punkt des Geräte-/Kartenkastens
  (Klasse 89 f0d3/f0d4, sonst eb1e/eb1f): 1 links oben, 2 rechts oben, 3 links unten, 4 rechts unten, 5 oben Mitte,
  6 Mitte, 7 rechts Mitte, 8 links Mitte, 9 unten Mitte. Belegt an SPS-Kartenkästen (Artikeltexte unten links/
  rechts im Kasten); 5–9 aus wenigen Fällen abgeleitet. Ohne ee1a/16 = Einfügepunkt.
- **Artikeleigenschaften am Symbol (22xxx):** ed66 = welcher Artikel (1-basiert), ed64 = Index der Eigenschaft
  (z. B. 22051 „Attribute“ 1..n); Werte direkt aus Article.eod (kv 60011, Schlüssel 22001 = Artikelnummer), nicht
  nur aus den Stücklistenspalten (z. B. 22105 Kurzbezeichnung „DE 16x24V DC ST“, 22269 Baureihe). Nebenfunktionen
  (Kartenkasten je Kanal) zeigen die Artikel ihrer Hauptfunktion; „0“ = leer.
- **Weitere BMK-Texte:** 20211 BMK übergeordnet ohne Projektstrukturen („-A4“ an jedem Kanalkasten), 20005 BMK
  identifizierend (Anlage+BMK ohne Ort, z. B. Kabel „=CB-W1.1“), 20006 vollständig; 20053 Kabel „Aderanzahl x
  Querschnitt“ aus 20042/20043 („8x0,34“).
- **Überbleibsel:** zwei Adern (Klasse 59) mit gleichem Symbol an derselben Stelle (z. B. alte Farbcodes „BN“ und
  neue „br“): sichtbar ist die mit berechneter Ausdehnung (eb1e), sonst die höhere ID.
- **Vergleichsergebnis:** Projekt A 78,0 -> 79,8 % (133 statt 118 zugeordnete Blätter dank Unterzähler), Projekt B (EPLAN 2024) 75,1 % vorher, Projekt C (EPLAN 2020) 61,8 -> 64,6 %. Regel „nur eigene Textlagen, vom Symbol nur 20028/20029“ an A und C bestätigt (beide besser). Offen: Listenseiten in C um ca. 8 mm versetzt; Querverweis 20300 an Kartenkästen-Nebenfunktionen druckt EPLAN nicht (Regel unbekannt).

## 19. Belegte Regeln aus EPLAN 2025 (REA/Ghidra, .NET-Metadaten, Projekteinstellungen) – Stand 08.11.2026

Grundsatz: Der Import ist ein Compiler. Jede Regel braucht einen Beleg (EPLAN-Code, API-Enum/Doku, gespeicherte
Daten, EPLAN-Hilfe); am PDF abgemessene Näherungen sind nicht erlaubt (wieder entfernt: Kontaktspiegel-Geometrie,
„erste Klemme“, Aderauswahl, Textumbruch-Zeichenbreite).

**Belegt**
- ee1a = ContextPropertyPlacement.Enums.BasePoint: 0 Einfügepunkt, 1 links oben, 2 rechts oben, 3 links unten,
  4 rechts unten, 5 oben Mitte, 6 unten Mitte, 7 Mitte links, 8 Mitte rechts, 9 Mitte, 13–15 Kabel, 16 Anschluss,
  17 feste Y-Lage (DataModel-Enum).
- ec57 = DockingStyle (DmgBaseText, dekompiliert): Bits 8/16/32/64 Richtung, Bit 128 Block zentriert, Bits 256–2048
  Anzahl zentrierter Texte; Bit 2 = angedockt (gespeicherte Lage ec55 gilt dann nicht). Richtung der Kette = Starttext
  (ohne Bit: 8 = darunter, DmgBaseText::getDockingDirection), 16 = darüber (am Original geprüft).
- ed65 = PropertyPlacement.RelationType: 0 Funktion, 20064 Hauptfunktion, 20190/20191 Artikelreferenz/Artikel,
  20192/20193 der Hauptfunktion, 20194–20197 Anschluss/Segment. ed66 = Artikelindex -> 20100[Index].
- ee27/ee28 (ec59) = Breite/Höhe der Ausrichtungsbox (TextBase.AlignmentBoxWidth/Height).
- 20021 FUNC_SUPPRESSCROSSREFERENCE: 0 automatisch (nur bei sichtbarem BMK), 1 nie, 2 immer.
- 60145/60144 (eaf1/eaf0) = sichtbares BMK: Anzeigeformat + Werte, von EPLAN gespeichert (<id> Wert, [id] Vorzeichen,
  <…> Gruppe nur bei belegtem Wert).
- Projekteinstellungen: x/Settings.xdb/Setting.eod (eplan_einstellungen.py; Felder ffdf Name, fff2 Wert, fff4 Einheit),
  z. B. EsCrossReference.Format.SeparatorForPage „/“, SeparatorBetweenPageAndPath „/“, SeparatorBetweenPathAndPos „:“,
  EsCrossReference.ContactImage.ContactorDisplayBitMask 262 (= ContactImage.Enums.DisplayMaskType: 256 ShowSymbols
  + 4 ShowFirstArticleUpper + 2 ShowVertical), MotorSwitchDisplayBitMask, TableWidth/TableHeight.
- Strukturformate je Objektart: ProjectInfo.xml 10050–10064 (10055 allgemein, 10057 SPS, 10059 Klemmen, 10060 Stecker,
  10061 Kabel).
- Kontaktspiegel: DMContactImage (Klasse 100, f169 Anzeigemaske, f16a Versatz); Zeichnung über die Einstellung
  ContactorDisplayForm bzw. eigenes Formular (XClSContactImageService::createContactImageView); Kontaktliste aus den
  Funktionsschablonen des Artikels (Article.eod Klasse 99, fd + 21000[1/2]). Normblatt (Form.eod, z. B.
  „Kunde_FN1_001“) trägt 12060 Kontaktspiegelabstand im Pfad (Projekt A: 60), 12061 Verschiebung, 12059 am Schaltzeichen.
- Kontaktspiegel umgesetzt (eplan_kontaktspiegel.py, Aufruf eplan_nach_qet._kontaktspiegel), Belege EEServicesLogu.erx
  (XEsContactImageInterface.cpp, EPLAN 2025) und ETrDMProjectu.erx:
  * Ablage: Klasse-100-Objekt als Platzierung ea65 der FUNKTION (vorrangig) oder der SYMBOLVARIANTE (z. B. Spule K:
    60411 Ebene 460, f16a (−5; 3,5)). Gezeichnet nur an Hauptfunktionen (eb16 fehlt). Felder nach Typ eindeutig den
    Membern des Konstruktors DMContactImage (0x18060c720) zugeordnet: f169 long +0x100 Maske, f16a EPoint +0xf0 Lage,
    f16b bool +0x104 automatisch ausrichten (Vorgabe 1, gespeichert nur die 0), f16c Text +0x120 Formular.
  * Maske (0x180303520): f169 Bit 1 -> eigene Maske; sonst ContactorDisplayBitMask, bei Bit 0x10 (Motorschutz)
    MotorSwitchDisplayBitMask | 0x10. Projekt A: Spulen 262, Schutzschalter 800|0x10.
  * Lage (0x180302730, computeAutoAlignLocation 0x18060cce0): f16b = 0 -> Funktion + f16a; sonst im Pfad
    (x + 12061, y = 12060 absolut), am Schaltzeichen (x + 12059, y + 12061); Werte Seite → Normblatt → 70 bzw. 24.
  * Einträge (0x180300750/0x1803011c0): Kontakte des Geräts; Symbol = eigenes Symbol des Kontakts, Variante 16
    (waagerecht, Anschlüsse ±2,8) bzw. A bei Motorschutz (0x180305080). Nicht platzierte Funktionen aus den
    Funktionsschablonen des Artikels: Satz je (62041 Artikelnummer, 62042 Variante) = Funktion 20100/20101
    (FUNC_ARTICLE_PARTNR/FUNC_ARTICLE_VARIANT; Projekt A-Q261.1: Variante 11 -> 11.1/11.2/11.4 wie im Original).
    Reihenfolge Gerätedefinition, weitere platzierte Kontakte danach numerisch (Hilfe querverweisarten).
  * Eintrag (0x180305080/0x180305880/0x180301ab0): Hülle = Symbolgrafik ∪ Anschlusstexte (20028 der Variante,
    Schrift nach Ebene). Maske & 0x412 = 2: Text am Anschlusspunkt, Richtung 8 -> x−1 Ausrichtung 6, 2 -> x+1/4,
    1 -> 8, 4 -> 2; sonst Lage der Platzierung. Senkrecht (Bit 2): Symbol mittig x = 0, Hüllen ohne Abstand
    untereinander ab 0 nach unten; waagerecht: nebeneinander, Abstand 2 mm, Symbol mittig y = 0.
  * Querverweis (0x180301bf0/0x1803061b0): Ebene 440, Schrift nach Ebene, Bit 0x20 -> 90° gedreht, Bit 0x10000
    -> × 0,8 (0x180306dd0); senkrecht x = rechte Hülle + 1 (Bit 0x200 links − 1), y = Eintrag + unterster Anschluss
    mit Richtung 2 (< 0); waagerecht x = Eintragsmitte, y = obere Hülle + 1 (Bit 0x200 untere − 1). Ausrichtung:
    Maske & 0x22 = 0 -> 0x100 ? (0x200 ? 2 : 8) : 5; = 0x22 -> 0x100 ? (0x200 ? 8 : 2) : 5; sonst 0x100 ?
    ((0x200)|2)·2 : 5. Ist der Text höher/breiter als der Eintrag, rücken Eintrag und Folgeeinträge um die halbe
    Differenz. Text nur bei nicht leerem Querverweis (nicht platzierte Kontakte: keiner).
  * Kopf/Fuß (0x180302930/0x180302d50): Kopf ab (0,0) nach oben (senkrecht, Ausrichtung 2) bzw. links (waagerecht,
    − Breite − 1, Ausrichtung 4): Bit 8 20200[2], 0x400000 20100[2], 0x100000 20200[1], 4 20100[1]; Fuß ab Cursor:
    0x80000 20100[1], 0x200000 20200[1], 0x800000 20100[2], 0x1000000 20200[2]. Ebene 480 (20100) / 481 (20200).
  * Text der Schnittstelle (+0x68): DmgTextInfo(Höhe −16002 = nach Ebene, Winkel 0) im Konstruktor 0x1802fe2c0,
    sonst nirgends gesetzt.
  * QET: der eigene Querverweis eines Masters hängt nur am Info-Text „label“ (DynamicElementTextItem::updateXref);
    bei Mastern mit EPLAN-Spiegel wird das Kennzeichen deshalb als Benutzertext gesetzt (Verknüpfung bleibt).
- Querverweise (EEsCrossReferenceu.erx / EESInterpreteru.erx, EPLAN 2025, Stand 09.11.2026):
  * Übersichtsquerverweis 20301 (FUNC_PLCPAIRCROSSREFERENCE): Interpreter EESInterpreteru 0x1800752a0 ->
    EsPlcPairXRefEvaluator::isCrossReferenceRelevant (0x180058ff0) + getXRefForGedAsString (0x180059190). Prüft
    WEDER 20021 noch das sichtbare BMK. Relevant auf Schaltplanseiten (isLogicalPageDocType) bei
    EsCrossReference.Plc.ShowPairCrossreferenceOnCircuit, in der Übersicht (Darstellungsart 3) bei …OnPlcOverview;
    …AtSupply = 0 schließt Versorgungsanschlüsse aus (Signal der Anschlusspunkte; Projekt A/Projekt E/Projekt B: alle drei = 1).
    Text = Präfix + getStandardXRefAsString je Gegenstück (getCounterparts: Relation 20305), mehrere durch
    Zeilenumbruch. Vorher nur in der Übersicht und mit 20021 -> SPS-Kanäle im Schaltplan ohne „=SC+BP1/3/6“ usw.
  * Haupt-/Nebenfunktionsquerverweis 20300: EsStdFunctionXRefEvaluator. collectCrossreferencedObjects (0x18005aa20)
    prüft zuerst die EIGENE Funktion mit isRelevantForCrossReferences(eigene, 0, false) (0x180026530): 20021 der
    Funktion selbst (0x4e35) – 1 nie, 2 immer, 0 nur mit BMK und sichtbarem BMK (ESNameTools::hasInstanceName).
    getXRefForGedAsString (0x18005c2b0) prüft jedes Ziel ebenso. 20021 wird also NICHT von der Hauptfunktion geerbt
    (vorher über je_bmk geerbt – Projekt A/Projekt E/Projekt B: 39 Nebenfunktionen ohne eigenes 20021 zeigten im Original den Verweis,
    obwohl irgendeine Funktion des BMK 1 trug). Zusätzlich vor den Zielen: Manueller Querverweis 20302 (in Projekt A/Projekt E/
    Projekt B nicht belegt, nicht umgesetzt).
    getLogicalCrossReferences (0x18005bfe0): Nebenfunktion -> Hauptfunktion (getDocumentationMainElement),
    Hauptfunktion -> getSupplementFunctions (0x18005a7b0: alle Funktionen gleichen Namens außer sich selbst, gleiche
    Darstellungsart, platziert, relevant, ohne Paarquerverweis). Ausgabe: buildXRefFromList mit
    Function.NoOfCrossReferencesInBlock (1) / ShowCrossReferenceBlockAsRow, an der Funktion 20049/20050/20198.
  * Sichtbares BMK „leer“ für die Relevanz = kein gespeichertes 60145 (Archivdaten: Funktionen ohne 60145 zeigen in
    Projekt A/Projekt E/Projekt B bei 524 von 535 20300-Platzierungen (20021 = 0) keinen Verweis; die 11 übrigen sind v. a. Nachbarpole,
    deren Verweis am ersten Pol steht). hasInstanceName selbst nicht dekompiliert.
  * Pfad = Spalte des Normblatts wie die Spaltenleiste (Form.eod Spaltenbreiten X 21509, Ursprung eb67, 12025,
    12029; Projekt A/Projekt E/Projekt B „Kunde_FN1_001“: 10 × 42 mm) – auch für Abbruchstellen. Lage genau auf einer Spaltengrenze
    zählt zur rechten Spalte (wie bisher x // 42; am Original Projekt A =BD/14, =DA/28, =EB/46 kontrolliert, im Code
    nicht belegt).
  * QET-eigene Verweise („[1-2] /15/1“ am Master = CrossRefItem mit Anschlussnamen, Slave-Xref „/%F/%c“) hängen an
    der Elementinformation „label“; bei allen verknüpften Mastern und Slaves wird das Kennzeichen als Benutzertext
    gesetzt (Schalter EPLAN_QET_XREF=1 behält QETs Verweise). EPLANs Text kommt aus 20300 an der EPLAN-Lage.
  * Hauptfunktionen mit Kontaktspiegel (Klasse 100) zeigen keinen 20300-Text (Archivdaten: Spulen tragen den Text
    aus der Symbolvorlage, das Original zeigt dort nichts) – Code-Stelle der Unterdrückung nicht gefunden.
- Schrankaufbau -> Fork-Panelmodell: Funktionsdefinition 7 = Montageplatte -> <panel role="surface">; Artikel-
  Produktgruppe 22041: 55 Schaltschrank/49 Gehäuse -> enclosure, 53 Kabelkanäle -> duct; Untergruppe 22028: 5/140
  Tragschiene -> din_rail, 142 C-Profil -> c_profile; Funktionsdefinition 8 = Artikelplatzierung -> Element mit
  width_mm/height_mm (eb1e/eb1f, Modellmaß, Einfügepunkt links oben), depth_mm (22014), Drehung 20451,
  Legendenposition 20450, Anfasspunkt 20208/20209 (20339–20341 aus Makro). mm je QET-Einheit = Seitenmaßstab / K.
- 1:n-Maßstab (11048): Eigenschaftstexte haben Papier-Schrifthöhe; Lagen in Modellmaß.
- Placement.Visibility ebf6 = 1: unsichtbar, nicht ausgeben.
- Schrift: Zellhöhe = 1,5 × Texthöhe (ETextGraphicsTools::setFont, CreateFont cHeight) = Ober- + Unterlänge der
  Schrift (Tahoma usWin 2049/423 bei 2048) -> Geviert 1,243 × Texthöhe; am Original-PDF bestätigt (2,5 mm = 8,8 pt,
  1,8 mm = 6,34 pt, 1,5 mm = 5,28 pt). Zeilenabstand = Zellhöhe. Gestapelte Texte (Andocken): Abstand aus den
  Zellanteilen je Ausrichtung (XGedBaseCalcDockPosition::getNextPositionForDockingVertical).
- Schrift „vergrößert“ (Kompatibilität „Texte vergrößert anzeigen“, Projekteinstellung GedViewer.TypeFontCreation,
  umgesetzt: eplan_grafik.schrift_einstellen, Aufruf in eplan_nach_qet nach projektinfo). Belege: (a) EPLAN-Code 2025
  EBaseTypesu ETextGraphicsTools::getEplanToPdfSizeFactor 0x1801eb070 / _calcFontSizesInternal 0x1801eb480:
  CreateFontW(1000 = Zellhöhe), Faktor = 1,5 × (tmHeight − tmInternalLeading)/tmHeight = 1,5 × Geviert/Zelle
  (Tahoma 1,243; Vorgabe ohne Schrift 1,35, ohne Metrik 1,26), vom PDF-Export (EPDFExportLogu 0x180010df0) je Schrift
  ohne jede Einstellungsabfrage benutzt; „TypeFontCreation“ kommt in keiner 2025-Binärdatei vor. (b) EPLAN-Hilfe 2.9
  Projekteinstellungen Verwaltung > Kompatibilität: „Texte vergrößert anzeigen“ = Texte wie in alten Versionen um ca.
  20 % größer, Textlängen und Zeilenabstände anders (xessettingsgui_d_einstellungenprojektkompatibilitaet.htm).
  (c) Archivdaten aller 17 Archive mit Original-PDF unter dem Kundenordner: alle mit EPLAN 2.7/2.9 erzeugten PDFs
  (PDFlib 9.x, 10 Projekte) mit TypeFontCreation 0 drucken Geviert 1,5 × h (2,5 mm -> 10,63 pt, 1,8 -> 7,65), mit
  1 Geviert 1,243 × h (8,8/6,34 pt) – ohne Ausnahme, auch bei gleicher Version 2.7.3 bzw. 2.9.4 und gleichem
  Bearbeiter; alle PDFs aus EPLAN 2023/2026 (PDFlib 10) drucken 1,243 × h auch bei TypeFontCreation 0 (Schalter
  entfallen, siehe a). Alle Textmaße (Formulare/Normblatt eingeschlossen) sind um 2472/2048 = 1,207 größer, auch der
  Zeilenabstand mehrzeiliger Texte (1,8 h statt 1,5 h). Regel: TypeFontCreation = 0 und Programmversion (ProjectInfo
  10043) < 2023 -> Geviert und Zellhöhe × 2472/2048 (Geviert = 1,5 h = alte Zellhöhe), sonst maßstabsgetreu.
  Nicht dekompiliert (keine 2.x-Binärdateien vorhanden): die alte Zeichenroutine selbst; dass das Andocken
  gestapelter Texte die vergrößerte Zelle nimmt, folgt aus „Zeilenabstände anders“ (umgesetzt: gleiche Zelle).
  Wirkung (gleicher Code bis auf die Regel): Projekt C 83,3 -> 95,0 % Treffer, zusätzlich 16,9 -> 6,2 %; A und B
  unverändert (93,0 % / 91,5 %).
- Satzlänge jeder Tabelle: Kopf der Indexdatei .eox (fe 53 …), Bytes 0x0D–0x0E Big Endian + 5 (alle Tabellen in sieben
  Projekten). Die frühere Suche nach der kleinsten teilbaren Länge lag bei Projekt E Plc daneben (3480 = 4 × 870).
- Formular-Name ede6 nur als Text gültig; Form.eod-Klasse 169 trägt an der Stelle eine Eigenschaftsliste.
- Normblatt = Form.eod Klasse 21 (Klasse 51 = Formulare). Satz: ea63 s2 1, Zahlen-Array 81 03 (Kopf wie 80 03:
  u32 X = 21509, u8, u32 Anzahl, Anzahl × d8) = Spaltenbreiten (FN1_001/Kunde: 10 × 42, EES: 10 × 40), eb67 p16 und
  eb66 d8 (FN1: (0, 21)/267, Kunde: (0, 16)/276, EES: (12, 16)/276 – Bedeutung offen, x = Beginn der Spaltenleiste
  passt), ede6 Name, ea6b Eigenschaften 12006 ff. (12025 Anfangswert Spalte, 12029 Stellen, 12033/12034 Maße,
  12060 Kontaktspiegelabstand), ea65 → f0d4 Blattgröße. Grafik des Normblatts = Instance.eod mit Eigner (21, id):
  Linien/Rechtecke/Texte wie Seitengrafik, Bild (Klasse 91, Logo), Platzhalter Klasse 53 (ed63 Eigenschaft, ed64
  Index, ee17 Quelle 4 Seite / 14 Projekt / 53 Spalte, ec55 Lage, ec59 Format).
- Normblatt einer Seite: Seiteneigenschaft 11016 (PAGE_FORMPLOT), sonst Projekteinstellung TrDMProject.Frame.
- Platzhalter: 11000 Seitenname = ohne Strukturkennzeichen (EPLAN-Hilfe); 11040/11041 Folge-/Rückblatt, Struktur nur
  wenn abweichend (Projekt A-Original „=DA/28.1“ auf einer =DB-Seite); 1140/1240 Anlage/Ort mit Vorzeichen; 11020–11024
  Ersteller/Datum der Seite (Zeitstempel → TT.MM.JJJJ wie im Original); 10027 Tagesdatum = im Projekt gespeicherter
  Wert (Original zeigt ihn, nicht das heutige Datum); 19005 Spaltennummer = Anfangswert + Spaltenindex aus der Lage.
- Bilder (Instance.eod Klasse 91, f0e7 Pfad): „$(IMG)\datei“ ohne Unterordner liegt im Archiv direkt in Images\;
  alles mit Unterordner und alles unter $(MD_IMG) liegt in Images\<hex> bzw. Images\M<hex> (gleiches Hex = gleicher
  Quellordner; $(MD_IMG)-Wurzel in allen EES-Archiven bc146770, News 2027: „EES“ -> M84070225, „News 2027“ ->
  Me0c0baec; M-Präfix hängt nicht allein an MD_IMG: Energie legt $(MD_IMG)\EPLAN.jpg in bc146770 ohne M). Umsetzung
  eplan_grafik.Grafik.bilddatei: Hex je (Variable, Unterordner) = einziger Ordner, der alle Bilder dieses Unterordners
  enthält (mehrere nur, wenn die Dateien darin byte-gleich sind – Archiv-Doppel, Energie 5 Fälle; „wago\2787“ hat
  zwei verschiedene Dateien und bleibt offen); nicht auflösbar wird gezählt (Importbericht). Variable und Unterordner
  stehen oft klein („$(md_img)\pxc“, Siemens 78 Seitenbilder) – Windows-Pfad, Vergleich ohne Groß/klein.
- „{{Text}}“ in Texten (Sprache ??_??) = als nicht zu übersetzend gekennzeichneter Text (EPLAN-Hilfe
  translategui_k_mehrsprachigetexte: Darstellung im Dialog als {{+A}}); im Plan nur „Text“ (ohne_uebersetzungsmarke).
  Betroffen u. a. Seitenbeschreibungen „{{Eplan Disclaimer}}“, Energie „{{NYY-J /1x50 RM BK}}“ (151×).
- Location.eod (Klasse 6, Strukturkennzeichen): 60081 Kennzeichnungsblock (1100 =, 1200 +, 1201 Unterebene Ort,
  1500 &), 60082 Kennzeichen, 60083 d8 = Reihenfolge der Strukturkennzeichen-Verwaltung („Zeile“), NICHT
  alphabetisch (Phoenix: GAA1, GAB1…GAB4, XDA1…XDA8, MAA1, MAA2, B, XGA, K; Stacking: … GQA, K1, K2, F …).
  EPLAN-Hilfe pleditorgui_k_start: die Seitenreihenfolge im Seiten-Navigator richtet sich nach dieser Reihenfolge.
  Umgesetzt: Sortierung Anlage (1100) → Ort (1200) nach 60083, dann Seitenname. Am Original geprüft (Nachbarpaare in
  Originalfolge): Projekt E 115 → 125 von 127, Projekt B 90 → 115 von 117, Projekt A unverändert 131 von 132.
- zlib-Kopf: bis EPLAN 2026 78 DA, EPLAN 2027 (News-Projekt) 78 9C. Position des zlib-Stroms = Inline-Position + 8
  (alle Tabellen), nicht mehr über die Kopfsuche bestimmt.
- Formular einer Auswertungsseite = Name (Seite 11015) + Formulartyp: Form.eod ede5 = Typ (z. B. 21 Kabelübersicht,
  29 Schaltschranklegende, 31 Strukturkennzeichenübersicht, 14 Inhaltsverzeichnis, 22 Artikelstückliste); die Seite
  trägt ihren Typ in ea8f, der Auswertungsblock (Klasse 75) in 61414, die Zeilengruppe (Instance Klasse 3) in ea77.
  Gleicher Name in mehreren Typen ist normal (KD_001 als Typ 21/14/29/31/17/30). Hilfe
  formeditorgui_d_formulareigenschaften (Formulartyp = interne ID) und formgeneratorgui_k_auswertungstypen
  (Dateiendung .fNN je Typ). Vorher: erster Satz des Namens -> Legende zeigte den Kabelübersicht-Kopf.
- Platzhaltertexte (PlaceHolderText.eod), deren Besitzer ebf1 eine Zeilengruppe (Klasse 3) ist, die in Instance.eod
  nicht mehr existiert, sind Reste früherer Auswertungsläufe: sie liegen auf denselben Zeilenlagen wie die gültige
  Zeile (Beispiel Artikelstückliste: Gruppe fehlt, „1547610000“ auf 235/210,5 unter „6ES7 512-1SK01-0AB0“ der
  vorhandenen Gruppe). Nicht gezeichnet. Projekt B 574, Projekt E 1246, Projekt A 0 solcher Texte. Stand 09.11.2026: alle diese Texte
  (und ihre Gruppen) sind gelöschte D-ISAM-Sätze (siehe „Satzstatus“) und werden schon beim Lesen verworfen; die
  Gruppenprüfung greift danach in Projekt B/Projekt A/Projekt E nicht mehr.
- Satzstatus und .eox-Aufbau (alle Tabellen): EPLAN speichert jede Tabelle mit D-ISAM (EDisam96u.dll, Exporte
  isOpen/isRead/isWrite/isDelRec/isRebuildFree …), Dateiformat C-ISAM. **.eod** = Datensätze fester Länge, Satz n
  (1-basiert) bei (n-1)·Satzabstand; Satz = Nutzdaten (Satzlänge) + 1 Statusbyte + 4 Byte. Statusbyte **0x0A =
  gültig, 0x00 = gelöscht/frei**. **.eox** = Indexdatei aus 4-KB-Knoten, Knoten k (1-basiert) bei (k-1)·4096. Kopf
  (Knoten 1, Big Endian): 0x00 fe 53, 0x06 Knotengröße-1 (0x0fff), 0x08 Anzahl Schlüssel, 0x0D Satzlänge,
  0x0F Knoten der Schlüsselbeschreibung (2), 0x19 erster Knoten der Freiliste der Datensätze, 0x1D erster Knoten der
  Freiliste der Indexknoten, 0x21 nächste Satznummer (= Satzanzahl), 0x25 letzter Knoten (= Dateigröße/4096),
  0x29 Transaktionszähler, 0x2D eindeutige ID. Freilisten-Knoten: u16 belegte Bytes (0x0ffa = voll), u32 nächster
  Knoten der Kette (0 = Ende), dann u32-**Satznummern (1-basiert)** der freien Sätze. Übrige Knoten = B-Baum-Knoten
  der Schlüssel (nicht weiter gebraucht). Belege EDisam96u.dll (EPLAN 2025): isDelRecUnsafe (0x18000dd40) ruft
  0x18000d6c0 = schreibe 1 Byte 0 an Satzabstand(+0x1c)·(n-1) + Satzlänge(+0x18), danach Satznummer n big-endian an
  den Freilisten-Knoten anhängen (Kopf u16 Füllstand, ab Byte 6 je 4 Byte, neuer Knoten bei vollem); isRebuildFree
  (0x18000a9c0) liest für n = Satzanzahl … 1 dasselbe Byte und nimmt genau die Sätze mit 0 in die Freiliste;
  0x180001e10 (bei isDumpIndex) reicht nur Sätze mit Byte == 0x0A weiter. Daten: in allen festen Tabellen von
  Projekt B/Projekt A/Projekt E sind die Statusbytes nur 0x0A/0x00, und die Freiliste ist gleich der Menge der Sätze mit 0
  (Ausnahmen nur zusätzliche 0-Sätze außerhalb der Liste: Projekt B Function 4, Instance 72; Projekt E Function 1, Instance 43 –
  isRebuildFree würde sie aufnehmen). Blob/XBlob und die Bibliotheken (*.sdb, FctDefLib.fdb) haben andere
  Satzabstände (Dateigröße kein Vielfaches) und werden nicht gefiltert. Befund des Vorgängers aufgeklärt: Satzplätze
  waren 0-basiert gezählt, die Freiliste enthält 1-basierte Satznummern (Knoten „0x0e“ = Knoten 15 der Kette
  117 -> 15 -> 217 -> 199; die „gültigen“ 5517/5776 sind Satznummern = Plätze 5516/5775, beide gelöscht; die „in keiner
  Liste“ veralteten 2985/8701/8972 stehen als 2986/8702/8973 in der Liste). Umsetzung: autodetect.loeschmarke;
  eod_symbole.saetze, eodlib.records und eplan_einstellungen.lesen überspringen Sätze mit Statusbyte 0.
  Gelöschte Sätze Projekt B/Projekt A/Projekt E: PlaceHolderText 3167/0/7614, Instance 8717/21/10840, Page 32/0/70, Function 52/0/64,
  Article 47/1/216, Setting 41/0/18 (u. a.). Projekt E: 6 Instance-IDs liegen gelöscht und gültig vor, bei einer stand der
  gelöschte Satz später in der Datei und überschrieb bisher den gültigen. Inhaltsverzeichnis Projekt B: alle 119 im
  Original vorhandenen Zeilen haben 0x0A, alle 37 veralteten 0x00. Wirkung (isolierter Lauf, Treffer unverändert
  Projekt A 93,0 / Projekt E 91,4 / Projekt B 83,3 %): zusätzlich/falsch Projekt A 5,2 -> 5,1 %, Projekt E 13,5 -> 6,8 %, Projekt B 20,1 -> 16,9 %;
  Geisterseiten (gelöschte Page-Sätze) entfallen: Projekt E 239 -> 168, Projekt B 195 -> 163 PDF-Seiten, alle Seiten mit
  Kennung entsprechen jetzt genau dem Original; gelöschte Notizen/Hinweistexte (z. B. Projekt A =EA/41) entfallen.
- Eigenschaftstexte auf nicht druckenden Ebenen (Layer.eod efef = 0, siehe Ebenen) werden nicht gezeichnet, wenn
  die Platzierung keine eigene Sichtbarkeit trägt (ebf6 fehlt/2 = nach Ebene) und kein anderer Text an ihnen andockt
  (Projekt B/Projekt E: Artikel-Dokumente 22149 ff. auf Ebene 1075/1076 „Dokumente_De/_En“: Handbuch_De, $(MD_DOCUMENTS)…).
- Werte der Hauptfunktion (je BMK, auch Artikel 20100[n] für Nebenfunktionen/Kästen): zuerst die Funktion ohne eb16
  (Hilfe Betriebsmittel: nur die Hauptfunktion trägt Artikel; Nebenfunktionen zeigen deren Artikel). Vorher gewann
  die Reihenfolge in Function.eod, z. B. die Artikelplatzierung im Schrankaufbau (Funktionsdefinition 8, eb16 = 0,
  nur BaseUnit 6ES7 193-6BP20-0BA0) -> SPS-Kanäle zeigten 22105 der BaseUnit statt „DE 16x24V DC ST“.
- Gegenstück Übersicht <-> Schaltplan (Funktionstext 20031, Querverweise 20300/20301): gleiches BMK, gleiche
  Funktionsdefinition, gleiche Steckerbezeichnung (eb22) und gleiche Anschlussbezeichnung (Hilfe
  plcgui_k_arbeitsweise). Vorher ohne Stecker -> Stecker X2/X3/X4 eines Geräts mit gleicher Pinnummer vertauschten
  Funktionstexte.
  Wirkung der fünf Punkte (isoliert gemessen, übriger Code gleich): Projekt B Treffer 76,6 -> 78,6 %, zusätzlich
  25,6 -> 22,4 %; Projekt A 88,8 -> 88,9 %, 8,2 -> 7,6 %; Projekt E 86,5 -> 86,9 %, 16,6 -> 15,4 %; kein Blatt schlechter.
- **SPS-Zielverfolgung / symbolische Adresse 20403/20404 (09.11.2026).** Nicht gespeichert: Function.eod der
  SPS-Anschlüsse trägt weder 20402 (manuell) noch 20403/20404; Plc.eod (Klasse 243) nur Struktur/Karte,
  PlcIoEntries.eod leer; der Text „=CB+S1-S13“ steht nur als Endpunkt-Text in Connection.eod. EPLAN berechnet ihn:
  DMPlcIoEntryCompound::getSymbolicAddressCalculated (ETrDMProjectu 0x1807ca680) ruft einen registrierten Rechner
  (Objekt +0xc0, vtable +0xa0) – dessen Rumpf (EPlcLogu) nicht dekompiliert; die Regel stammt aus der Hilfe
  (plcgui_k_symbolischeadressen: Sensor am Eingang bzw. Aktor am Ausgang über die Signalart der Anschlusslogik,
  dessen BMK; kein Fund -> BMK der letzten Funktion, an der die Suche abbrach), die Daten dazu sind belegt:
  - Funktionsdefinitionen liegen im Archiv: x/FctDefLib.fdb/FctDef.eod (Klasse 60, 1160 Sätze, Schlüssel
    efc5/efc6/efc7, Name ee72, je Anschluss ein ee86-Objekt Klasse 16 = DMConnectionPoint). Überlauf in
    FctDefLib.fdb/Blob.eod: 256-Byte-Blöcke, Kopf 11 Byte (Tabelle u16 BE, ID u32 BE, Nr u16 BE, Anzahl u16 BE, Flag),
    letztes Byte je Block und je Satz kein Strominhalt (zlib-Adler-32 stimmt, 1160/1160 Sätze). eodlib.blob_continue
    (Kopf/Ende 5 Byte) passt hier nicht -> eigener Leser eplan_fktdef.py.
  - DMConnectionPoint-Felder: ETrDMMetau.erx (Registrierfunktion 0x1800d3920: Member-Offset je Eigenschafts-ID) und
    ETrDMProjectu.erx (Getter lesen denselben Offset): eb01 +0x48 getInternalDesignation, eb02 +0x70 getType,
    eb03 +0x78 PhysPotentialTransfer, eb04 +0xa0 getPotentialType, eb05 +0xa8 getConsumerSources, eb06 +0xd0
    getSignal, eb07 +0xd2 getPlcTrackingOneBased (Zielverfolgung (SPS) nach, 1-basiert), eb0f +0xd8 PlcTracking als
    Text (setPlcTracking setzt +0xd2 = 0), eb08 +0x100 MaxConnectionCount, eb09 +0x102 JumperBarCount, eb0a +0x104
    TerminalTarget, eb0b +0x105 Eigensicherheit, eb0c +0x71 PortType, eb0d +0x106 Kabelanschluss, eb0e +0x107
    gleiche Bezeichnung erlaubt, eb10 +0x108 SrcOrDest. Typ eb01 = EString (Function.eod-Typtabelle liest s2 49 = „1“).
  - Signalart = Eplan.EplApi.DataModel.Pin.SignalType (EEplan.EplApi.DataModelu.dll, Konstanten): 0 Neutral,
    1 Sensor, 2 Actor, 3 ConnectionSupply, 4 DeviceSupply, 5 Source, 6 DigitalInput, 7 DigitalOutput, 8 AnalogInput,
    9 AnalogOutput, 10 BusSource, 11 BusIn, 12 BusOut, 13 PressurePort, 14 SuctionPort, 15 BusEnd. Gegenprobe an den
    Definitionen: Schließer/Öffner/Wechsler 1, Spule/Steckdose 2, „SPS-Anschluss … Digitaleingang“ 6, DA 7, AE 8,
    AA 9, Bus-Eingang 11, Zylinder 13. Klemmen/Stecker: eb07 1->2, 2->1, 3->4, 4->3 (= Hilfe 004018/004019).
  - Wirksame Logik einer Funktion: DMFunction::getLogicalDefinition (0x180667380): hat die Funktion eine lokale
    Logik (eigene ee86, hasLocalLogicalDefinition) gilt nur diese, sonst die der Funktionsdefinition (+0x1a0).
    DMFunction::getPlcTracking (0x180677d80) nimmt den Anschluss param_1 (0-basiert) dieser Logik und dessen
    PlcTracking (convertConnPoints).
  - Angezeigter Text = Endpunkt-Text der Funktion (Abschnitt 1 FAKT „Endpunkt-Text“) ohne „:Anschluss“, also mit
    Steckerbezeichnung/Steckerkontakt: Kontrolle Projekt B =SB+CA/8 „=SC+BP1-A12:X2“, =SB+CA/9 „=EA-XSUB:15“.
  Umgesetzt (eplan_nach_qet.sps_ziel): Start am SPS-Anschluss (Signalart 6/8 sucht 1, 7/9 sucht 2), Breitensuche
  über Connection.eod; Abbruchstellen gleichen Namens verbunden; je erreichtem Anschluss: Signalart = gesucht ->
  Treffer; sonst eb07 > 0 -> weiter am Anschluss eb07 derselben Funktion; sonst Ende dieses Zweigs; ohne Treffer
  die zuletzt erreichte Funktion. 20404 = 20402 (manuell), sonst ermittelt; Anlage/Ort der Seite wie bisher
  ausgeblendet. Nicht umgesetzt (gezählt): Start ohne Signalart 6–9 (lokale Logik ohne eb06, sps_symb_ohne_signalart
  -> alter Weg), Text-Weiterleitung eb0f auf andere Funktion (sps_symb_verweis_text), 20196 „ohne Sensor-/Aktor-
  Suche“ (in Projekt B/Projekt A/Projekt E nicht gesetzt), Reihenfolge bei Verzweigungen (EPLAN-Rechner nicht dekompiliert).
  Wirkung (gleicher übriger Code, nur sps_ziel ein/aus): Projekt B 80,4 -> 80,8 %, zusätzlich 21,3 -> 21,0 %; Projekt A
  90,5 -> 90,8 %, 6,5 -> 6,2 %; Projekt E 88,4 -> 88,6 %, 14,1 -> 14,1 %; 19 SPS-Blätter besser, keins schlechter.
  Offen: SPS-Ausgang ohne Adresse direkt an einem SPS-Eingang (Projekt A =CB+CA/22: A665 -> A3, Projekt E =CB+CA/21): Import
  zeigt das Ziel-BMK (Abbruch), das Original nichts – Regel (Adresse leer? SPS-Anschluss als Ziel?) nicht belegt.

- Formular-Platzhalter (09.11.2026): Formulare (Form.eod Klasse 51) tragen eigene Platzhalter als Instance.eod
  Klasse 53 mit Eigner (51, Formular-ID) – ed63 Eigenschaft, ed64 Index, ee17 Quelle (4 Seite, 14 Projekt, 51 Formular),
  ec55 Lage, ec59 Format – wie die Normblatt-Platzhalter (Klasse 21). Für sie speichert EPLAN keinen ausgewerteten
  Text: PlaceHolderText.eod (Klasse 52, ee0d = Eigenschaft) enthält in Projekt B/Projekt A/Projekt E nur Zeileninhalte (Besitzer
  Zeilengruppe Klasse 3 bzw. Seite beim Inhaltsverzeichnis), nie 18000/14022/14023/11015 oder Projekteigenschaften.
  Gefüllt werden sie deshalb beim Zeichnen wie die Normblatt-Platzhalter: Quelle 4/14 über nb_wert (Seiten-/
  Projekteigenschaften, z. B. Deckblatt Typ 17: 10011, 10013, 10025 …; Kopf 11015 Formularname der Seite), Quelle 51 =
  Eigenschaft des Formulars (ea6b): 14020/14022 Ersteller/letzter Bearbeiter (Text), 14021/14023 Erstellungs-/
  Änderungsdatum (Zeitstempel -> TT.MM.JJJJ wie 11021/11024), 18000 FORMBASE_NAME (Hilfe eigenschaften-ids:
  „Name (Formular, Normblatt …)“) = Name des Formulars im Projekt (ede6, über den die Seite es per 11015 referenziert).
  Der in ea6b gespeicherte 18000-Wert ist der Name der Vorlage, aus der das Formular kopiert wurde (Projekt B/Projekt A/Projekt E:
  KD_F01_002 -> „F01_002“, F21_004 -> „F21_001“, KUNDE_002 -> „F26_001“); Kontrolle am Original: Kopf zeigt
  „KD_F01_002“, „06.08.2020“ (14023 = 1596696482), „XYZ“ (14022). Lagen unter 20 mm (QET-Schriftfeld) entfallen
  wie bei formulargrafik. Umsetzung eplan_grafik.Grafik.formularplatzhalter + eplan_nach_qet (nach formulargrafik).
- Ausgewertete Platzhaltertexte (Klasse 52) mit Ausrichtungsbox fester Breite (ec59 ee27, Umbruchart ee25 & 0x70 =
  0x10) brechen wie Eigenschaftstexte um (umbrechen, GedFTextBreaker): z. B. Hersteller 22222 „… GmbH & Co. KG“
  (Box 47,5 mm) und Bezeichnung 22004 (131,5 mm) in Stücklisten. Vorher nur für Klasse 30/83.
  Wirkung (isoliert, gleicher übriger Code, eigene Elementsammlung, Vergleich mit fester Abbildung der Grundlinie):
  Projekt A 89,9 -> 90,5 %, zusätzlich 7,0 -> 6,9 %; Projekt E 87,6 -> 88,2 %, 14,9 -> 14,7 %; Projekt B 79,5 -> 80,0 % (eigene
  Abbildung 80,2 %), 22,1 -> 21,9 %; kein Blatt schlechter (mit eigener Abbildung je Lauf verschieben sich
  Grenzfälle um 4 mm – Messartefakt der gemeinsamen Abbildung). Deckblatt Projekt A 55,7 -> 98,4 %, Projekt E 51,5 -> 98,5 %,
  Projekt B 31,7 -> 54,0 %; alle „F13_001/XYZ/06.08.2020/KD_F01_002/KUNDE_001“-Fehlwörter weg.
- Projekt B-Original (PDFlib 9, ältere EPLAN-Version): Schriftgröße im PDF = 1,5 × Texthöhe (2,5 mm -> 10,63 pt, 5 mm ->
  21,26 pt), Projekt A/Projekt E (PDFlib 10): 1,243 × Texthöhe (8,8 pt). Längere Projekt B-Texte sind im Original ~21 % breiter;
  daher die vielen „verschoben“ bei linksbündigen Titeln/Zeilen (Inhaltsverzeichnis 4,9 mm, Deckblatt bis 29 mm).
  Kein Importfehler; welche Einstellung/Version das steuert, ist nicht belegt.

**QET-Seite (gemessen am eigenen PDF-Export, kein EPLAN-Wert)**
- Pixelgröße = round(pt · 4/3), 1 Szenen-px je Geviert-px; Punktgröße deshalb als Vielfaches von 0,75 pt setzen.
- Blatttext <input> und Bauteil-Kennzeichen (QGraphicsTextItem): 4 px Rand, erster Buchstabe x+4,
  Grundlinie y + 4 + Oberlänge. Bauteiltext <text> (ElementPictureFactory::parseText): x/y = Grundlinie ohne Rand,
  mit anchor="alignment" + Halignment = Mitte/rechts. Laufweiten = Tahoma-hmtx × Pixelgröße (fontTools).

- Typtabelle der Eigenschaften aus dem EPLAN-Metamodell (ETrDMMetau.erx, REA/capstone): jede Klasse registriert
  ihre Felder über DMMetaBase::addSimpleData(ID, DataType, bool), addReferenceData, setReference2(ID, T*) und
  DbData-Unterklassen (RTTI DbSimpleData<DataType, DbDataValueProxyPtrToMember<Klasse, C++-Typ>>), teils über lokale
  Vorlagen-Helfer. DataType: 1 bool, 2 char, 3 short, 4 long, 6 double, 7 EString, 10 DbObjectId, 13 EPoint,
  14 ETime, 17 EPropertyList, 25 EMD5Hash; EPoint3D = 3 × d8. 883 IDs, davon 747 eindeutig abbildbar ->
  daten_typen/eplan_metamodell_typen.json. Der Typ ist klassenabhängig (Arc ec8c double, anderswo EPoint) und der
  Scan erfasst nicht jede Klasse; eine globale Überschreibung kostete Projekt A 4 Punkte (11 312 Bögen weg). Deshalb liest
  der Parser zweistufig: erst exakt wie bisher, nur bei Scheitern mit dem EPLAN-Typ als zweitem Versuch und bis zu
  6 Abschnitten – alle bisher lesbaren Sätze bleiben bitgleich (gegen HEAD geprüft). Am Byte belegt: ea9f/eaa3 Text (erlernt s2), DMConnectionPoint eb03 Text „2,3,4“ (erlernt b1),
  f412 EPoint3D 24 Byte (erlernt q32). Neue Parsertypen p24 (EPoint3D), h16 (MD5/GUID).
- Abschnitte: je Vererbungsstufe ein Abschnitt, getrennt durch ff ff, auch leer (ff ff ff ff); bis zu 6 erlaubt.
- Wirkung (zweistufig): Phoenix Function.eod 673 → 1234 von 1562 lesbar, Page.eod 24 → 98 von 98; Projekt A Function
  3011 → 3228, Instance 36 615 → 56 590, Page 131 → 133 (alle früher lesbaren Sätze bitgleich). Import: Phoenix
  492 → 568 Funktionen mit Symbol, 219 → 291 Verbindungen; Projekt A 88,7 %, Projekt E 86,4 % unverändert, Projekt B 76,3 → 76,8 %.
- 3D-Platzierungen (DMFunction3D, Klassen 170–185) erkennbar an Feldern nur dieser Klasse (f412–f418, fa19);
  der Compiler nimmt sie aus der Schaltplanverarbeitung.

- EOD-Klassennummer → EPLAN-Klasse: ETrDMMetau.erx exportiert je Klasse ??_F<Klasse>Meta (Hülle, ruft den
  Meta-Konstruktor mit der Klassennummer in edx). Belegt u. a. 17 DMFunction, 18 DMConnection, 88 DMLocationBox,
  53 DMContextPropertyInstance, 160 DMFunction3D, 173 DMCabDuct, 174 DMCabBusBar, 180 DMCabComponent,
  185 DMCabRoutingSegment (98 Klassen mit Nummer). Vererbungsketten aus MSVC-RTTI (937 Klassen, z. B. DMFunction3D →
  DMFunction → DMFunctionBase → DMSymbolReference → DmgBaseReference → DmgInstance → DMPropertyUser → DMBase).
  Gesichert in daten_typen/eplan_metamodell_klassen.json (Felder je Klasse über den Aufrufgraph; unvollständig,
  z. B. ee86 bei DMFunction fehlt). Noch nicht im Parser verwendet.

- Klemmen-BMK: EPLAN speichert das sichtbare BMK (60145/60144) nur an Klemmen, die es zeigen; Folgeklemmen ohne
  60145 und ohne eigene 20010-Platzierung übernehmen es von links und zeigen keins (Phoenix: nur Klemme 1 von -XD1).
  Am Original bestätigt (weniger Überzähliges, keine Treffer verloren).
- BMK-Beschriftung aller übrigen Funktionen (Stand 09.11.2026): gezeichnet nur mit gespeichertem sichtbarem BMK
  (60145/60144, sichtbar_gespeichert – dieselbe Regel wie für die Querverweis-Relevanz). Beleg: Hilfe
  devicetaggui_k_bmkuebernahme („Funktionen ohne sichtbares BMK übernehmen das BMK der nächsten Funktion in
  Suchrichtung“ – sie zeigen also keins) und Archivdaten: von den als Beschriftung gezeichneten BMK ohne 60145 steht
  im Original keines (Projekt A 0/41, Projekt E 0/45, Projekt B 0/49 – mit und ohne eigene 20010-Platzierung, alle Nebenfunktionen:
  Nachbarpole 201/202, Geräteanschlüsse 1302, Schirme 900; z. B. Projekt E DC/33 -S331 Kontakt 73804 + Geräteanschluss 74705
  neben der Master-Beschriftung 73803). Gilt auch für BMK-Texte in Black Boxes (Projekt E -LP/-RP, Projekt A -A480) und die
  Andockkette (leeres BMK belegt keine Zeile). Klemmen behalten ihre Regel (ohne 60145 MIT eigener Platzierung zeigen
  sie das BMK über den Eigenschaftstext). Die Projekt A-116 Funktionen mit eigener Platzierung ohne 60145 (SPS-Anschlüsse
  300: 44, Geräteanschlüsse 1302: 53, Klemmen 13 u. a.) laufen nicht über die Beschriftung und sind unverändert. Ergebnis: Projekt A 93,0/5,2
  -> 93,0/5,1 %, Projekt E 91,4/13,5 -> 91,5/13,3 %, Projekt B 83,3/20,1 -> 83,3/19,9 %, kein Blatt schlechter.
- Texte „&…“ (Stand 09.11.2026): kein Sonderfall. Alle „&…“-Seitentexte in Projekt A/Projekt E/Projekt B (Projekt E 120, Projekt A 78, Projekt B 214)
  liegen auf Ebene 1028 „Grafik.Steuertext“ bzw. 1090 (Layer.eod efef = 0, nicht gedruckt – belegte Ebenenregel)
  außer fünf auf Ebene 1006 (gedruckt): Projekt E =ZAP/71 „&KU.M12“, 4 × „&D5,0“ – genau diese zeigt das Original, sonst
  keines. Die frühere Regel „& wird nicht gedruckt“ war eine Näherung und ist entfernt.
- Andocken: das BMK (20010) zählt als belegter Bezugstext, auch wenn QET es als Bauteilbeschriftung zeichnet
  (sonst lag der nächste Kettentext, z. B. In = 100A, -WE1/N-Schiene, genau darauf).
- **Verbindungen zeichnen (Stand 09.11.2026):** Gezeichnet wird nur eine Verbindung mit Seite (Connection.eod
  eb2a/ebf2 = 60402). Verbindungen ohne Seite sind nicht platziert (Hilfe: Verwaltungsarten „Nicht-platziert“,
  „Brücke (automatisch)“, connectionbrowsergui_k_verbindungseigenschaften; zusammengehörige Teilklemmen werden
  „auch ohne gezeichnete Linie“ am letzten Anschluss verbunden, terminalgui_k_brueckenbildung). Daten: sie tragen
  statt eb2a die Felder eb39/eb3a; Phoenix: 69 Ketten XD1:1 a→b→…→f am Index 18 = letzter von 19 Anschlüssen;
  Projekt A: XSUB402:1→M (Original Blatt 40 ohne diese Linie). Platzierte Verbindungen enden in Projekt A nie auf verschiedenen
  Blättern (blattübergreifend nur über Abbruchstellen).
- **Abbruchstelle = zwei Anschlüsse:** Symbol BP (SPECIAL.sdb Nr. 8), Variant.eod: zwei Punkte im Einfügepunkt,
  entgegengesetzt (A: Richtung 2/8, B: 1/4 bei (0, −1)). Connection.eod eb2c/eb2f nutzt beide Indizes (Projekt A 154
  Verbindungen an Index 1, Phoenix 95) -> beide auf den einen Anschluss des QET-Pfeils.
- **Sammlungsbauteil nur, wenn es alle platziert verbundenen Anschlüsse hat:** Klemme X4_B (IEC_symbol 1422) hat
  6 Punkte (a/c oben, b/d unten, 4/5 links/rechts = Brückenpunkte); die Sammlungsklemme (nur oben/unten) verlor die
  Brücken und c/d-Anschlüsse an X5P/X5M (Projekt A 66 Verbindungen). Fehlen Indizes -> Schaltbild aus dem Archiv.
- Ergebnis Importbericht: Projekt A gezeichnet 1141 -> 1332, nicht gezeichnet 172 = 166 ohne Seite + 6 offenes Ende
  (eb29 = 3, kein zweiter Verweis); Phoenix 291 (davon 69 falsch, ohne Seite) -> 317, nicht gezeichnet 180 ohne Seite
  + 2 blattübergreifend; Projekt E 1070 -> 1263, nicht gezeichnet 191 = 184 ohne Seite + 7 offenes Ende. Textabgleich
  unverändert (Projekt A 89,7 %, Projekt E 87,6 %); Projekt A-Blatt 13 zeigt jetzt Brücken und Pfeilleitungen.

- Andocken, Blockbildung (EGedBaseLogu XGedBaseCalcDockPosition::getTextBlocks 0x180266b10, REA): in einen Block
  kommen Texte mit DockingStyle-Bit 1, 2 oder 4; Kette = Master (getDirectMaster) -> angedockte Texte, der oberste
  Master kommt immer dazu (FUN_180266930 läuft die Masterkette hoch). positionTextBlock (0x180267670) nimmt den Stil
  des ERSTEN Blocktexts (Starttext): (Stil & 0x60) == 0 -> senkrecht, Bit 16 gesetzt = darüber, sonst darunter
  (getNextPositionForDockingVertical, param bool = Bit 16 nicht gesetzt -> Abzug längs (-sin, cos)). Ohne jeden
  DockingStyle (Konstruktoren von DmgBaseText setzen +0x170 = 0, EDmGraphicsu 0x180036060/160/500) gilt also
  „senkrecht darunter“ – so wie kettenrichtung() mit Vorgabe 8.
- Waagerecht andocken: (Stil & 0x60) != 0 -> Bit 32 rechts, Bit 64 links (Stil & 0x20). Startkante = Anker des Starttexts
  +/- Breite je Ausrichtungsspalte (links 0, Mitte 1/2, rechts 1) + 0,3 × Texthöhe des Starttexts (this+0x68 =
  TextInfo+0x10 × 0,3); jeder weitere Text liegt mit seiner Kante an der laufenden Kante (setNewPosForDockingHorizontal
  0x180269630: Ausrichtung links/Mitte/rechts -> Anker = Kante + 0 / B/2 / B), weiter um Breite + 0,3 × h
  (getNextPositionForDockingHorizontal 0x1802691c0); leerer Text: Breite 0, kein Abstand. Alle Blocktexte übernehmen
  die Ausrichtung des Starttexts (XGedDlText+0xe6 = Master). Phoenix SPECIAL.sdb Symbol 307 (Variante 1962): 20010
  ec57 = 36 (32 + 4), 20099 = 38, 20011 = 34 -> „N-Schiene“ steht RECHTS neben -WE1 (bisher fälschlich darüber, weil
  32 wie „nicht 8“ = nach oben behandelt wurde). Umgesetzt in eplan_nach_qet.lage_von (Breite = Tahoma-hmtx × Geviert
  1,243 h; Rahmenabstand getHorizontalDistanceOfTextFrame und Textdrehung nicht nachgebildet).
- Farben (REA, EBaseTypesu/EDmGraphicsu/ETrDMMetau/EPDFExportLogu): Farbnummer = Index der EColorTable (short).
  Gespeichert: Grafik ebfd -> DmgVisibleInstance-Stift +0xe4 (getColorId@DmgVisibleInstance; ebfc +0xe6 Linientyp,
  ebfe +0xd8 Breite, ebff +0xe0, ec00 +0xe8), Text ee24 -> DmgTextInfo +0x42 (getColorId@DmgTextInfo; ee21 +0x10 Höhe,
  ee22 +0x18 Winkel, ee23 +0x40, ee25 +0x44, ee26 +0x46 Ausrichtung, ee29 +0x47), Ebene eff4 -> DmgPersistentLayer
  +0x11c (setColorId schreibt Stift- und Textfarbe der Ebene, +0x11c und +0x172; eff3 +0x11e, eff5 +0x110, eff6 +0x118).
  „Von Ebene“ = -16002 (gespeichert 49534; EColor::isColorDefinedByLayer, DmgPen::validateByLayer ersetzt Farbe,
  Linientyp, Breite) und ist die Konstruktor-Vorgabe von Stift (DmgVisibleInstance +0xe4/+0xe6 = 0xc17ec17e) und
  Textformat (DmgTextInfo +0x42; auch Höhe -16002.0 -> Ebenenhöhe). Tabelle: 0–255 AutoCAD-Palette (acadrgb),
  256–265 = ACAD 0–9, 266–281 E21-Farben (Tabellen BonW/WonB/BonG/Printer); Sonderfarben (EColorTable::getColor):
  -3 Hintergrund, -4 Hintergrund abgesetzt (sub 0x18009d660: hell -> je Kanal -30 beim Druck, -24 am Bildschirm),
  -5 Gegenfarbe (ACAD 7), -6…-9 Revisionsmarken (Kopien der Einträge 1/4/3/4). PDF-Export: Einstellung OutputColor
  (EPDFExportLogu 0x180028acf) = 1 -> Farbtabelle „_BlackOnWhite“: EColorTableMap-Konstruktor (EBaseTypesu
  0x1800a3e7a) setzt ALLE Tabellenfarben schwarz, Sonderfarben mit weißem Hintergrund und Druckerversatz -> -3 weiß,
  -4 #E1E1E1, -5 schwarz. Projekt A-Original (Schwarz-weiß-PDF): Texte 7120× #000000, 120× #E1E1E1 (Projektdatum, Ebene 454
  eff4 65532), Linien 570× #E1E1E1. Umgesetzt (eplan_grafik.farbe): Farbe auflösen (None/49534 -> eff4 der Ebene),
  dann 65533 -> #FFFFFF, 65532 -> #E1E1E1, sonst schwarz; als letztes Feld der Grundformen, Blatt-Shapes (pen/brush
  color) und Blatttexte (<input> als <font color>). Gilt für die Ausgabeart Schwarz-weiß; die Einstellung selbst steht
  im PDF-Schema des Benutzers, nicht im Projekt.
- Linientyp ist ebfc (+0xe6), nicht ebfd: form() wertet für „gestrichelt“ noch ebfd (Farbe!) aus – falsch, aber ohne
  Beleg für die Linientyp-Nummern (0, 2, Ebene eff3 1/3) noch nicht ersetzt (siehe Offen).

- **Lage von Eigenschafts- und freien Texten (Stand 09.11.2026, „verschobene“ Wörter; A/B/C = die drei Prüfprojekte, Original-PDF 2026/2024/2021):** Diagnose
  `EPLAN_TEXTPROTOKOLL=<datei>` schreibt je Blatttext Herkunft, Anker, Winkel, Ausrichtung, Bezug, Box (eplan_grafik.
  PROTOKOLL/KONTEXT, eplan_nach_qet._texte); Auswertung je Ursache mit einem Skript, das jedes verschobene Original-
  wort dem Protokollsatz des nächsten gleichen Import-Worts zuordnet. Belegt und umgesetzt:
  * Textwinkel ee22 (DmgTextInfo +0x18) gilt auch für Eigenschaftstexte (bisher fest 0; Projekt A 1372, B 2184, C 1050
    Platzierungen mit 90°). Andocken längs (-sin, cos) bzw. (cos, sin) des Winkels des Starttexts (positionTextBlock
    0x180267670: this+0x28/0x30 = (-sin, cos) aus TextInfo +0x18 des ersten Blocktexts).
  * BMK (20010) als QET-Bauteilbeschriftung: Lage nach Andocken (angedocktes BMK, ec57 Bit 2, stand an der Lage
    des Bezugs), Ausrichtung des Bezugs, Winkel als Beschriftungsdrehung (set_element_text rotation, um die linke
    obere Ecke wie freie Texte), z. B. Ortskennzeichen „+MASCHINE“ am Strukturkasten (vorher dx −7, dy +10).
  * Umbruch in der Ausrichtungsbox: ee25 = DmgTextInfo +0x44 (GedFTextInFrameFitting::actualizeTextBreaker 0x1801d5710:
    Umbruchart = +0x44 & 0x70), Breite ee27 = TextInfo +0x28 (DmgTextInfo::setBox). Art 0x10 = feste Breite ->
    GedFTextBreaker::breakFixedWidth 0x1801d0de0 / breakLine_FixWidth 0x1801d2970: je Absatz; kein Umbruch, wenn
    h·5·Zeichen < Breite oder die Zeile passt, sonst am letzten Worttrenner, bis zu dem der Text passt (mindestens am
    ersten); Trenner (isWordSeparator 0x1801d26d0) = Leerzeichen, „;“, Leerraum, nicht das geschützte Leerzeichen;
    Wörter werden nicht geteilt; Laufweite Tahoma-hmtx × 1,243 h. Umgesetzt eplan_grafik.umbrechen (Eigenschafts-
    texte und freie Texte), z. B. „Ventilentstörmodul 24VAC/DC/10VA/W“ (Box 32) zweizeilig (vorher dx −19), Pfad-
    Funktionstexte („RM Antrieb Ölkühler / eingeschaltet“). Andere Umbrucharten (0x20/0x30/0x50/…) nicht umgesetzt.
  * Mehrzeilige Texte: die Ausrichtung gilt für den Block (n Zellen à 1,5 h): oben/Grundlinie erste Zeile am Anker,
    Mitte Block mittig, unten letzte Zeile am Anker (getNextPositionForDockingVertical 0x180268f80: Zeilen 2..n bei
    4–6 je halb, bei 7–9 ganz über dem Anker); vorher immer erste Zeile am Anker (Projekt A „Hinweis: …“ dy −11,4).
  * Anschlussbeschreibungen 20029/20039 mit ec57 Bit 2 docken an wie jeder Text (getTextBlocks 0x180266b10: Bit 1/2/4
    -> Block); die frühere Ausnahme (eigene Lage ec55 relativ zum Bezug) war am PDF geschärft, ohne Code-Beleg.
    Beispiel B: SPS-Kanal „DI12“ unter 20038 (vorher dx +4, dy −3…−5).
  Wirkung (gleicher Code bis auf diese Punkte, Basis 8903cd5, globale Abbildung je Projekt): A Treffer 20683 ->
  20918 (89,9 -> 90,9 %), verschoben 876 -> 653, zusätzlich 1561 -> 1337; B 19759 -> 20153 (87,6 -> 89,4 %),
  verschoben 1230 -> 853, zusätzlich 3472 -> 3092; C 15849 -> 16077 (79,5 -> 80,7 %), verschoben 3091 -> 2869,
  zusätzlich 4517 -> 4302 (gezählt sind alle verschobenen Wörter, nicht nur die sechs im Bericht je Blatt).
  Einzelblätter mit 1–2 Wörtern weniger sind Wörter an der 4-mm-Grenze (die Abbildung verschiebt sich um Zehntel);
  echte Rückschritte: Kabel -W95 (A Blatt 9/16, Textsatz ee18 – erledigt, siehe nächster Punkt), C: ein angedockter
  dreizeiliger Text Mitte (Schrift des C-Originals, siehe Offen).
- **Eigenschaftsanordnungen (Textsätze ee18, Stand 09.11.2026, REA EPLAN 2025):** Eine Symbolvariante trägt die Texte
  aller vordefinierten Eigenschaftsanordnungen (Hilfe: „Eigenschaftsanordnung“ je Symbolvariante, Namen = Symbol-
  Eigenschaft 16050 SYMB_PROPERTYINSTANCESET, Index 1…63, z. B. CABDL 1 „Links oben, 0°, Bemerkung“, 3 „Links mittig,
  90°, Bemerkung“ … 30; PLCCPOV 1 waagerecht / 2 senkrecht / 3 180° / 4 270°). Belegt:
  * Felder (ETrDMMetau, Meta-Konstruktor 0x1800e93d0): DMContextPropertyInstance ee17 = +0x1c8 ContextType (u16),
    ee18 = +0x1ca SymRefConfiguration, ee19 = +0x1cb ProjectConfiguration, ee1a = +0x1cc ReferencePoint, ee1b = +0x1d0.
    DMSymbolReference (0x1802a5e20): ed78 = +0x143 (u8), ed79 = +0x140, ed7a = +0x142. DMSymbolVariant (0x1802ad410):
    ed98 = +0x128 (getDefaultPropSet), ed99 = +0x129 (getAlternativeDefaultPropSet).
  * Auswahl je Platzierung (ETrDMProjectu DMSymbolReference::getRealSelectedPropertyInstanceSet 0x180887d30):
    ed78 & 0x7f, wenn ≠ 0; sonst bei Projekteinstellung TrDMProject.SymbolReference.AlternativePropertySet (A/B/C: 0)
    ed99, sonst ed98; ist das 0 (oder bei Alternativ ed99 = 0): DMSymbol::getFirstPropertyConfigurationInstanceSet
    0x1808795d0 = kleinster Index 1…63, für den 16050 existiert. Bit 7 von ed78 (setSelectedPropertyInstanceSet
    0x180887c70 erhält es, getSelectedPropertyInstanceSet 0x180887d10 blendet es aus) = Texte liegen als eigene
    Instanzen an der Funktion (switchCurrentSelectedPropertySetToLocalInstances) – Archivbefund A/B/C: ed78 ≥ 128
    nur bei Funktionen mit eigenen Texten, ed78 < 128 nie mit eigenen Texten und immer eine vorhandene Anordnung.
  * Sichtbarkeit (DMSymbolReference::isPropertyInstanceConfigurationVisible 0x180888d90, DMContextPropertyInstance::
    isConfigurationVisibleForGed 0x18060e510): Text sichtbar, wenn (ee19 = 0 oder Projektanordnung = 0 oder gleich)
    und (ee18 = 0 oder Auswahl = 0 oder ee18 = Auswahl). Texte ohne ee18 (fast nur Anschlusstexte 20028/20029/
    20038/20039) gelten in jeder Anordnung. Eigene Texte der Funktionen passen in A/B/C alle zur Auswahl.
  Umgesetzt: Grafik.anordnung / Grafik.textlagen(lib, vid, ref) / textlagen(props, satz) in eplan_grafik.py, Aufrufe
  mit den Funktionseigenschaften in eplan_nach_qet.py (Black Box, Kasten-Mitglieder, Geräte, Anschlüsse,
  Unterbrechungspunkte); bisher galt je (Eigenschaft, Index) der letzte Satz. Beispiel A Blatt 9: Kabel -W95 (ed78 = 3)
  jetzt senkrecht „Links mittig, 90°“ (ÖLFLEX/CLASSIC/G/-W95 vorher 4–10 mm daneben). Wirkung (isoliert, gleicher Code
  bis auf diesen Punkt): A Treffer 21391 -> 21406, verschoben 674 -> 659, zusätzlich 1115 -> 1024; B 20628 -> 20641,
  886 statt 903, 1483 -> 1394; C 18934 -> 18939, 561 -> 551, 1262 -> 1234 (globale Abbildung; Blätter mit −1…−3
  sind 4-mm-Grenzwörter, je Blatt mit eigener Abbildung unverändert). Nicht umgesetzt: ee19 (Projektanordnung;
  bisher Vorrang ee19 = 2, Quelle des Projektwerts – IDmgDrawingContext +0x78 – nicht gefunden).

**Offen (nicht belegt, nicht umgesetzt)**
- Verschobene Wörter, Rest nach Ursache (Stand 09.11.2026; A/B/C wie oben): (1) erledigt: Schriftgröße 1,5 × h in Projekt C =
  Kompatibilität „Texte vergrößert“ (GedViewer.TypeFontCreation 0 unter EPLAN 2.x), siehe Abschnitt Schrift oben.
  (2) Kabeltexte: Auswahl des Textsatzes (ee18) ERLEDIGT (Belegt, „Eigenschaftsanordnungen“). Die meisten Kabel
  (A 74, B 91, C 71 CABDL) haben eigene Texte an der Funktion (ed78 ≥ 128) und waren davon nicht betroffen; ihr
  Rest-Versatz (z. B. B =EA/36 -W61 „UNITRONIC“ dx −9 dy +7, W6x dy +4) kommt aus der Andock-Kette mit gemischten
  Winkeln (Start 90° mit ec55, angedockte Texte 0°) bzw. den Kabel-Bezugspunkten ee1a 13–15 (CableUpperLeft/
  CableCenter/CableLowerRight, EPLAN-API), die textlagen als Einfügepunkt nimmt – Code dafür nicht dekompiliert.
  (3) Umbruch freier Texte zusätzlich nur bei Platzierungsflag ebf4 Bit 0 (Archivbefund in allen drei Projekten:
  umgebrochene Seitentexte tragen ebf4 33/49, nicht umgebrochene – auch zu breite wie „Schutzgitter hinten“ in Box 24
  – kein ebf4; Bit 0 oder 5, Bedeutung im Code nicht gefunden: Registrierung von 0xebf4 fehlt im Metamodell-Scan).
  (4) Verschiebungen > 20 mm sind fast nur kurze Wörter (Ziffern, Pinnamen), deren richtiges Gegenstück im Import
  fehlt – in Wahrheit „fehlend“, nicht Lagefehler. (5) Ausrichtungsbox mit Bezugspunkt: ob die Box (ee27/ee28) die
  Lage verschiebt („Positionsrahmen hat Vorrang vor Ausrichtung“, Hilfe) ist nicht dekompiliert (B: SPS-Übersicht
  1:1,5 „Versorgung 24V DC …“ dx +5).
- Querverweise: (1) Darstellungsart/Seitentyp je Funktion (DMFunctionBase::getPlacementType) nicht gelesen –
  angenähert über uebersicht(); damit auch EsCrossReferenceSettings::getFunction_CrossreferenceDocTypes (welche
  Seitentypen ein Verweis erreicht, z. B. Schrankaufbau Funktionsdefinition 8) nicht ausgewertet: Artikelplatzierungen
  werden als Ziel ausgeschlossen. (2) Gegenstücke 20301 aus Relation 20305 – im Archiv nicht gefunden, gebildet nach
  Hilfe (BMK, Anschluss, Stecker, Funktionsdefinition); Projekt E FE/50 (Funktionsdefinition 1302, Bus-Anschlüsse) liefert
  falsche Zuordnung (Schnittstellenname fehlt im Schlüssel). (3) Reihenfolge mehrerer Ziele
  (getAllFunctionsWithSameNameAndExpand) nicht belegt – Seitenfolge, dann Lage. (4) SPS-Kästen fd 301 als
  Nebenfunktion (z. B. Projekt B =SC+BP1-A12, Projekt E/Projekt A =SC+CA-A665): Befund 09.11.2026 widerlegt – das Original ZEIGT den
  Verweis zur Hauptfunktion (Projekt E EA/34 „=SC/66/5“ bei y 75,8, Projekt B =DC/1 „=SC+BP1/1/1“, Projekt E 34 von 36, Projekt A 28 von 31,
  Projekt B 57 von 82 Kasten-Verweisen am Original gefunden). Die „überzähligen“ des Querverweis-Orakels sind Lagefehler:
  am gedrehten BMK (ee22 = 90°) angedockte Verweise liegen 3–7 mm versetzt (Andocken an gedrehten Bezugstext nicht
  dekompiliert), bzw. ein darüber gestapelter Funktionstext verschiebt die Kette (Projekt E =SB/60). Kein Code geändert.
  Echt ohne Verweis im Original bleiben nur Geräteanschlüsse 1303 (Projekt E -T410/-T440). (5) Kabel-Hauptfunktion
  listet in EPLAN den Schirm (fd 401) auf derselben Seite nicht (Projekt B =DC-W9). (6) param_3 von
  isRelevantForCrossReferences (Kategorie 200 mit 20047, Darstellungsart 8) nicht umgesetzt.
  (7) Abbruchstellen: Partner der Kette weicht in Projekt A/Projekt B ab (z. B. Projekt A =BD/10 L1: Original =EB/44/2, Import
  =BA/9/9); Sortierkennung 24809 vermutlich ef75 (61301, Wert 101) – Zuordnung und Kettenlogik
  (EsInterruptionPointXRefEvaluator::getDestination) noch nicht dekompiliert. Spalte „10“ im Original (Projekt A 13/10)
  liegt außerhalb der 10 Normblattspalten – Regel offen.
- Überzählige Texte ohne Beleg (Stand nach den Formular-/Platzhalter-/Ebenen-Regeln): (1) Inhaltsverzeichnis: zwei
  Zeilensätze auf denselben Lagen, Besitzer jeweils die Seite (Klasse 4), einer verweist (ee14) auf eine andere Seite
  mit veralteten Werten (Datum 11024 ≠ Seitenwert) – woran EPLAN den gültigen erkennt, nicht gefunden. Befund
  09.11.2026 (Projekt B =AB/1.2, 1.3): auch einzelne Spaltentexte und ganze Zeilen ohne Gegenstück sind veraltet (z. B.
  1240 „+CA“/„+BP1“ auf =AB/1.2 Zeile =ZSA 1.a, Original leer; =AB/1.3 ab y 112 Original leer). Satzinhalt und
  Satzkopf (u16 bei Byte 18 = Änderungszähler? gültig 2–12, veraltet 0–2) trennen nicht; kein Verweis auf die Texte
  aus Instance/Page/Form; Seite 2837 trägt den Auswertungsblock 75 (61415 Liste der Inhaltsverzeichnisseiten, 61412
  Bereich) ohne Zeilenliste. In PlaceHolderText.eox gibt es 4 KB-Knoten mit reinen u32-Satzplatzlisten (Kopf u16
  0x0ffa, z. B. Knoten 0xc6/0xd8/0x0e/0x74): Knoten 0xc6 enthält 471 der 574 verwaisten Zeilengruppentexte und die
  veralteten Inhaltsverzeichnis-Plätze 7517, 8947, 8973, 8975, 8979, keinen geprüften gültigen – vermutlich
  Freispeicherliste; Knoten 0x0e enthält aber auch gültige Plätze (5517, 5776), veraltete Plätze 2985/8701/8972
  stehen in keiner Liste. ERLEDIGT 09.11.2026: die veralteten Sätze sind gelöschte D-ISAM-Sätze (Statusbyte 0),
  siehe Belegt „Satzstatus und .eox-Aufbau“; die Widersprüche oben kamen von 0-basiert gezählten Plätzen gegen
  1-basierte Satznummern der Freiliste. Die abgeleitete Regel „erste in Lesefolge“ wird nicht gebraucht und ist
  nicht umgesetzt. (2) Symbolische
  Adresse 20404: ERLEDIGT 09.11.2026 (Belegt, „SPS-Zielverfolgung“); offen bleibt nur der Rumpf des EPLAN-Rechners
  (Reihenfolge bei Verzweigungen, Text-Weiterleitung eb0f, Start ohne Signalart).
  (3) Schrankaufbau: 20100/20200/20450 an Artikelplatzierungen (ec57 Bit 0x80, Ebenen 490–493 druckend) zeigt
  der Import, das Original nicht. (4) Der Vergleich (pdf_vergleich) entfernt Wasserzeichen-Wörter (≥ 90 % der
  Blätter, z. B. Datum 27.01.2021) nur aus dem Original, nicht aus dem Import -> je Blatt 1 Scheintreffer
  „zusätzlich“ (Projekt B 119, Projekt E 128).
- Linientypen: ebfc/eff3-Nummer -> Strichmuster (EPLAN-Linientyptabelle) suchen; bis dahin bleibt „gestrichelt“ an
  ebfd (= Farbe) – z. B. Projekt A-Rechtecke mit Farbe 65532 erscheinen gestrichelt.
- Farbige PDF-Ausgabe (OutputColor 0/2: Tabelle „White“ + Benutzer-Farbschema USER.GedViewer.ColorSchema.White,
  Graustufen) nicht nachgebildet; Sonderfarben -6…-9 (Revisionsmarken) nicht umgesetzt (in Projekt A/Phoenix nicht benutzt).
- Elementgrafik (Symbole .elmt) bleibt schwarz (QET-Elementstil kennt nur Farbnamen); Projekt A/Phoenix-Bibliotheken nutzen
  keine Sonderfarbe. Linien aus anderen Modulen (_verschoben in eplan_nach_qet/eplan_kontaktspiegel) verlieren die Farbe.
- Restliche unlesbare Funktionssätze (Phoenix 328) sind ausschließlich Schaltschrank-3D-Klassen (180, 185, 173, 174)
  – für den Schaltplan nicht nötig. Typ je (Klasse, ID) im Parser erst, wenn die Feldsammlung je Klasse vollständig
  ist (Basisklassen-Registrierung über andere Wege). Ausweichtypen per Rückverfolgung bleiben verworfen.
- Sichtbar/Drucken je Ebene: BELEGT (REA). Layer.eod-Sätze sind DmgPersistentLayer; das Metamodell registriert
  efee bool → Member +0x100, efef bool → +0x101, eff9 bool → +0x102. EDmGraphicsu.erx: isVisible liest +0x100,
  isVisibleOnPrinter +0x101, isLocked +0x102; der Konstruktor setzt +0x100/+0x101 = 1, +0x102 = 0 – gespeichert wird
  nur die Abweichung (bool 0, vom Parser als leerer Text gelesen). Weitere Felder: eff0 Name, eff1 Beschreibung,
  eff2 Nummer, eff3/eff4 short (eff4 Farbe), eff5 double Linienbreite, eff6 float, eff8 Textformat, effa/effb uchar.
  Umgesetzt: Ebenen mit efef = 0 werden nicht gezeichnet (Nachbildung des Drucks). Projekt A/Projekt E/Projekt B: 40–50 solche Ebenen,
  Ergebnis unverändert (Projekt A 88,7, Projekt E 86,4, Projekt B 76,8 %); EES: Designkit des Normblatts (833–840) verschwindet.
- 12025 ohne Wert: Spaltenzählung ab 0 angenommen (Projekt A/Projekt E/Projekt B-Originale zeigen 0–9), Vorgabewert nicht belegt.
- Kontaktspiegel (Rest): Symbol eines NICHT platzierten Kontakts – EPLAN sucht per DMSymbolLibrary::getSymbol(
  Funktionsdefinition) in der Bibliothek der Hauptfunktion (belegt), welches von mehreren passenden Symbolen kommt,
  ist nicht belegt (Funktion liegt in TrDMProjectu, nicht dekompiliert); umgesetzt: kleinste Symbolnummer eb52
  (Projekt A: SL/S/O wie im Original). Hülle eines DmgText nicht dekompiliert: angenommen Zellhöhe 1,5 × h und
  Tahoma-Laufweite (Projekt A-Zeilenabstand der Einträge damit 3,1 mm, Original ≈ 3,35 mm – Abweichung offen).
  Nicht umgesetzt (in Projekt A/Projekt E/Projekt B ungenutzt, werden gezählt: kontaktspiegel_nicht_umgesetzt): Formular 0x2000000,
  Tabelle 0x800, Drehung 0x20000/0x40000, Anschlusstexte ausblenden 0x4000/0x8000, Verlängerungslinien 0x400,
  Einträge ohne Symbol (EPLAN zeichnet ein 2-mm-Kreuz). DMFrame +0x178 (Konstruktor-Vorgabe 1) entscheidet in
  computeAutoAlignLocation zwischen Pfad-Lage und Zeilen-Lage (checkForLadder) – die Eigenschaft dahinter ist
  nicht gefunden, umgesetzt nur der Fall 1. Kontaktspiegel an Nebenfunktionen (Projekt A 48 SPS-Anschlüsse mit eigenem
  Klasse-100-Objekt) werden nicht gezeichnet (Hilfe zu 20314: Spiegel an der Hauptfunktion).
- Verlegte 3D-Verbindungen (Energie-Beispiel, 8 von ~2800): Strom ab eb2c falsch zerlegt (x/y/z-Wegliste) –
  Eigenschaftstyp fehlt; bisher als „Anschlussindex nicht lesbar“ übersprungen.
- Klemmen-BMK bei versetzten Klemmen (welche zeigt es) – EsFunctionPlacementTools (ESBase).
- Platzierungen ohne Makro: EPLAN zeichnet Rechteck mit BMK/Legendenposition (Hilfe), Lage der Texte unbekannt.
- Kanäle/Schienen als reine Grafik (z. B. Projekt A „&KU.180X80“): ohne Artikel kein Beleg für eine Rolle.
- Hash-Funktion der Images-Ordnernamen (bc146770 usw.): CRC32/FNV/djb2/Java/MD5 über Unterordner-Namen (mit/ohne
  Variable, Groß/klein, UTF-8/UTF-16) getroffen nichts; vermutlich über den absoluten Quellpfad. Deshalb Zuordnung über
  die Dateien (siehe Belegt).
- Variant.eod: ed95 in einigen Sätzen nicht als (Klasse, ID) lesbar (Importbericht „Variantensätze nicht lesbar“:
  Siemens 177, Phoenix 219, News27 217, Stacking 217, Energie 175; v. a. SPECIAL.sdb, HYD1/HYD2, LUB1) – der
  Typ-Mehrheitsentscheid zerlegt diese Sätze falsch (ed95 als int/str/Liste); übersprungen und gezählt.
- Function.eod: Klassen 170/175/180/185 (Klemmen, Anschlüsse, SPS-Kanäle u. ä.) scheitern im Parser (parseall
  „fail“) massenhaft: Phoenix 646 von 1562 Sätzen (41 %), Siemens 344/2018, Stacking 501/3221, Energie 1574/7641.
  Folge: Klemmensymbole und Verbindungen fehlen (Phoenix-Klemmenseiten zeigen nur Texte). Ursache = unbekannte
  Eigenschaftstypen in diesen Klassen; nächster Schritt: Abbruchstelle (letzte Eigenschafts-ID) je Klasse bestimmen.
