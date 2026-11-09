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
- FAKT: Satzlänge und Blockposition werden je Tabelle automatisch erkannt (`autodetect.py`): Der Selbstverweis 61 EA muss in jedem Satz an derselben Stelle stehen. Für Projekt A stimmt das Ergebnis mit den manuell ermittelten Werten überein.
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
- Von den 37 Abweichungen: 36 in Projekt F, wo der EPLAN-Text ein anderes Gerät nennt als der referenzierte Datensatz. HYPOTHESE: veraltete EPLAN-Texte. Gestützt dadurch, dass ein Teil der Texte `einem Gerät` nennt und ein Gerät A19 im Archiv nicht existiert.
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
## 15. Symbolgrafik, Seitengrafik, Kästen und Eigenschaftstexte (Stufe 2)
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
- FAKT: Funktion → Symbol: ef4d Bibliotheksindex, ef4e Symbolnummer (= eb52), ef4f Variante. Im Beispielprojekt:
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
- Werkzeug: `Daten\vorlagen\eplan_zw1\eplan_nach_qet.py` (Import mit Symbolen), `EPLAN-Import.cmd` (Drag & Drop);
  Probelauf einzelner Seiten: `umsetzen(..., nur_seiten={"18", "26"}, neu_dekodieren=False)`.
- Bauteile beim Import (`eplan_bibliothek.py`): Funktion mit Artikel → zuerst Bauteil der Firmensammlung gleicher
  Bestellnummer (Anschlüsse über den Namen, Reihenklemmen o/u über die Lage; gedreht nach Variante A–D, gespiegelte
  Varianten E–H nicht); fehlt der Artikel → Data Portal (`--dataportal`, Fork-API `importPartFromDataPortal`); sonst
  Schaltbild aus der Archiv-Symbolvariante + Artikeldaten als `<Bestellnr>_EPLAN-Schaltbild_<Symbol>_<Var>_<Anschlüsse>.elmt`
  unter Hersteller/Kategorie/Bestellnr. Sondersymbole (SPECIAL: Geräteanschluss, Black Box, Kabel) werden kein Bauteil.
  Offen: Black-Box-Geräte (Makrografik + Geräteanschlüsse) als ein Bauteil zusammensetzen; Geräte, deren
  Funktionen zusammen ein Sammlungsbauteil ergeben (Gruppenzuordnung).

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
- **Texte „&…“** sind interne Verweise (Makro-/Artikelkürzel) und werden nicht gedruckt; „{{…}}“ = Makrotext.
- **Bilder:** Pfad `$IMG\…` oder `$(IMG)\…`; QET setzt Bilder über den Mittelpunkt des UNskalierten Bildes.
- **Abbruchstellen (IPoint):** Name = 20012+20013+20014 (z. B. „5L4+“); gleichnamige paarweise in Seitenfolge
  (1↔2, 3↔4); Symbol SPECIAL „BP“, Variante %4: 0 rechts, 1 oben, 2 links, 3 unten (4–7 gleich). Text auf der
  offenen Seite (weg von der angeschlossenen Leitung), senkrecht von unten nach oben lesbar; „Name / Verweis“,
  rechtsbündig „Verweis / Name“; Verweis an Pfeilen ohne führenden Schrägstrich („12/2“, „=SB/57/1“).
- **QET-Fallen beim Nachbearbeiten des .qet:** Texte nur in den `<inputs>` des Blatts (Einrückung 8), nicht in den
  gleichnamigen eines Elements; Element-UUIDs je Datei eindeutig (Name), sonst gleiche Grafik für alle.
- **Stand Testprojekt:** 73,5 % der Originaltexte an gleicher Stelle über 118 Blätter (vor Abbruchstellen-
  und SPS-Kasten-Regeln), 30 Blätter ≥ 90 %. Offen: Positionsnummern im Aufbau (EPLAN-berechnet), perspektivische
  Makros (LazerSafe), Kabeltexte, Strukturkasten-Beschriftung senkrecht.

## 18. Verallgemeinerung auf weitere Projekte (Stufe 5, drei Projekte am Original-PDF geprüft)

- **Projektdaten fürs Schriftfeld:** `x/ProjectInfo.xml` (Property id/index/type): 10011 Projektbeschreibung
  (Anlagenbezeichnung, mehrzeilig, „??_??@…;“), 10013 Projektnummer (Zeichnungsnummer), 10014 Kommission
  (Auftragsnummer), 10015–10017 Firma/Adresse, 10020/10021 Ersteller/Datum. Nie Werte aus einer Schriftfeld-Vorlage
  übernehmen (die Firmenvorlage trägt sie als SVG-Bild -> je Projekt neu erzeugen).
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
  „<Firma>_FN1_001“) trägt 12060 Kontaktspiegelabstand im Pfad (Projekt A: 60), 12061 Verschiebung, 12059 am Schaltzeichen.
- Schrankaufbau -> Fork-Panelmodell: Funktionsdefinition 7 = Montageplatte -> <panel role="surface">; Artikel-
  Produktgruppe 22041: 55 Schaltschrank/49 Gehäuse -> enclosure, 53 Kabelkanäle -> duct; Untergruppe 22028: 5/140
  Tragschiene -> din_rail, 142 C-Profil -> c_profile; Funktionsdefinition 8 = Artikelplatzierung -> Element mit
  width_mm/height_mm (eb1e/eb1f, Modellmaß, Einfügepunkt links oben), depth_mm (22014), Drehung 20451,
  Legendenposition 20450, Anfasspunkt 20208/20209 (20339–20341 aus Makro). mm je QET-Einheit = Seitenmaßstab / K.
- 1:n-Maßstab (11048): Eigenschaftstexte haben Papier-Schrifthöhe; Lagen in Modellmaß.

**Offen (nicht belegt, nicht umgesetzt)**
- Kontaktspiegel-Zeichnung (Zeilenraster, Symbolgröße, Spalten) – im EPLAN-Code suchen.
- Zeilenabstand gestapelter Texte (bisher 1,35 × Schrifthöhe aus älterer Sitzung) – EPLAN RowSpacing/Schrift.
- Klemmen-BMK bei versetzten Klemmen (welche zeigt es) – EsFunctionPlacementTools (ESBase).
- Platzierungen ohne Makro: EPLAN zeichnet Rechteck mit BMK/Legendenposition (Hilfe), Lage der Texte unbekannt.
- Kanäle/Schienen als reine Grafik (z. B. „&KU.180X80“): ohne Artikel kein Beleg für eine Rolle.
