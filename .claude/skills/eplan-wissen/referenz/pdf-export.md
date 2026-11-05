# EPLAN PDF-Export – Navigation, Sprünge, Einstellungen

Was die offizielle EPLAN-Hilfe (Plattform 2.9 bis 2026) über den PDF-Export sagt und was das für den
Nachbau in QElectroTech (Birgers Fork) heißt. Inhalte sind in eigenen Worten zusammengefasst. Jede Aussage
nennt ihre Quelle. „Vermutung“ heißt: nicht durch die Hilfe belegt.
Stand der Recherche: 05.11.2026.

## Inhalt

1. Grundprinzip
2. Dreifachsprung und Sprungfolge
3. Betriebsmittel-Seiten (BM-Seiten) und Betriebsmittelbaum
4. Sprünge aus Auswertungen
5. Querverweis- und Unterbrechungspunkt-Sprünge
6. Sprungziel-Markierung (roter Blinkrahmen) und Zoom
7. Lesezeichen, Seitenbaum und Texte im PDF
8. Hyperlinks
9. Ausgabesprachen
10. PDF/A und Schreibschutz
11. Dialog PDF-Export und Registerkarten (Export, Allgemein, BM-Seiten, BM-Eigenschaften)
12. Seitenreihenfolge im PDF
13. API/Automatisierung
14. Neuerungen nach Version
15. Ausgabegröße, Druckränder, Einstellungsschema, PDF-Kommentare
16. Stand im QET-Fork und offene Punkte
17. Quellen

Drucken, DXF/DWG und EDA-Export: siehe `dxf-dwg-druck.md`.

---

## 1. Grundprinzip

- Beim PDF-Export bleiben logische Elemente und Verknüpfungen des Projekts erhalten. Darum eignet sich das PDF
  zur Archivierung. Über Lesezeichen springt man zu den Seiten, und man kann zwischen verknüpften Elementen
  navigieren. Die Funktion ist nur in bestimmten Ausbaustufen verfügbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_start.htm
- Es entsteht immer genau eine PDF-Datei, egal ob einzelne Seiten oder das ganze Projekt gewählt sind.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_export.htm
- Zum Betrachten wird Adobe Acrobat Reader ab Version 10.0 verlangt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_h_seitenexportieren.htm

**Bedeutung für QET:** Ziel ist ein Gesamt-PDF mit funktionierender Navigation, kein Satz Einzel-PDFs. Ein
reines Bild-PDF (PDF-Kopie) erfüllt das nicht, weil nichts springt.

## 2. Dreifachsprung und Sprungfolge

- Standardmäßig bekommt jedes Schaltzeichen drei Sprungbereiche:
  - **Mitte:** Bei einer Hauptfunktion geht es zur BM-Seite des Betriebsmittels. Bei einer querverwiesenen
    Nebenfunktion (z. B. Kontakt) geht es zur Hauptfunktion (z. B. Spule).
  - **Links:** vorwärts in der Sprungfolge.
  - **Rechts:** rückwärts in der Sprungfolge.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- In die Sprungfolge eines Betriebsmittels gehören alle Darstellungsarten, Platzierungen und Einträge in
  Auswertungen. Die Reihenfolge der Kette ist die Reihenfolge der Platzierungen im Bereich „Verweise“ der
  BM-Seite. (Englische Fassung: „All representation types, placements, and report entries … are integrated“.)
  Quelle: https://www.eplan.help/en-us/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
  und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/pdfexportgui_k_grundlagen.htm
- Der Sprungbereich ist gegenüber früheren Versionen größer: Er umfasst auch die platzierten Eigenschaftstexte
  des Schaltzeichens (z. B. BMK-Text).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- Bedienung im PDF: Linke Maustaste gedrückt halten zeigt den Sprungbereich als markierte Fläche. Wer die Maus
  mit gedrückter Taste wegzieht, bricht den Sprung ab.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pdfexportgui_k_grundlagen.htm
- Mit „Vereinfachte Sprungfunktion“ (Registerkarte Allgemein) entstehen nur einfache Sprünge statt der
  dreiteiligen. Das verkleinert die Datei.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_allgemein.htm
- Was die Mitte einer Hauptfunktion tut, wenn keine BM-Seiten erzeugt werden, sagt die Hilfe nicht.
  Vermutung (aus früherer Linkanalyse eines EPLAN-PDFs): Sprung auf sich selbst bzw. kein Ziel.
  Quelle (Lücke festgestellt): https://www.eplan.help/en-us/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm

**Bedeutung für QET:**
- Der Fork setzt den Dreifachsprung bereits um: Bauteil-Informationen `jump_next` (linkes Drittel),
  `jump_main` (Mitte, Fallback `destination`) und `jump_prev` (rechtes Drittel). Ohne next/prev nimmt das
  Hauptziel die ganze Fläche ein. Quelle: Fork `sources/pdf_links.cpp` (`injectCrossRefLinks`) und
  `sources/qetgraphicsitem/element.cpp` (`Element::jumpUuid`).
- Das Verknüpfungsskript muss pro Betriebsmittel eine **geordnete Kette** bauen: alle Platzierungen (Spule,
  Kontakte, Übersichtsdarstellung, SPS-Kanal) plus Auswertungszeilen (Stückliste, Klemmenplan, Kabelplan …),
  in der Reihenfolge, die EPLAN im Bereich „Verweise“ hätte. Danach je Glied `jump_next` = nächstes Glied,
  `jump_prev` = voriges Glied. Ob die Kette am Ende umläuft, sagt die Hilfe nicht. Vermutung: nein.
- Mitte: Nebenfunktion -> Hauptfunktion. Hauptfunktion -> BM-Seite, falls der Nachbau BM-Seiten erzeugt.
  Sonst die Mitte der Hauptfunktion leer lassen oder auf die erste Auswertungszeile legen (Vermutung, gegen
  das Original-PDF prüfen).
- Der Sprungbereich muss die Bauteiltexte einschließen. Der Fork nimmt `boundingRect()` des Elements.
  Ob die beweglichen Texte (DynamicElementTextItem) darin liegen, ist zu prüfen.
- Prüfen: Original-PDF mit pymupdf `page.get_links()` lesen. Drei nebeneinander liegende Linkflächen auf einem
  Bauteil = Dreifachsprung, Ziele vergleichen.

## 3. Betriebsmittel-Seiten (BM-Seiten) und Betriebsmittelbaum

- Zusätzlich zu den Projektseiten kann EPLAN Seiten mit Betriebsmitteldaten anhängen (Eigenschaften,
  Artikeldaten). Welche Eigenschaften erscheinen und wie sie heißen, ist einstellbar. Die Texte sind
  mehrsprachig.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- Einschalten: Einstellung „Betriebsmittel-Seiten erzeugen“ unter Optionen (ab 2025: Datei) > Einstellungen >
  Benutzer > Schnittstellen > PDF-Export, Registerkarte BM-Seiten. Ist sie aus, erscheinen die Betriebsmittel
  nur im Betriebsmittelbaum des PDFs, ohne eigene Datenseiten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_bmseiten.htm
  und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/pdfexportgui_k_grundlagen.htm
- **Betriebsmittelbaum** im PDF (Lesezeichen-artig, aufgebaut wie der Betriebsmittel-Navigator): oben das
  sichtbare BMK, darunter die Darstellungsarten (z. B. allpolig, Übersicht), ganz unten die Funktionen. Klick auf
  eine Funktion zeigt deren Platzierung im Schaltplan. Klick auf BMK oder Darstellungsart springt zu den Daten
  des Betriebsmittels.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/pdfexportgui_k_grundlagen.htm
- **Aufbau einer BM-Seite:** Überschrift = vollständiges BMK, darunter drei Bereiche: BM-Eigenschaften,
  Artikeleigenschaften, Verweise. Im Bereich Verweise stehen Platzierungen (Darstellungsarten der Funktionen)
  und Auswertungsseiten. Die Reihenfolge der Bereiche ist per Pfeiltasten änderbar, jeder Bereich abschaltbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_bmseiten.htm
- Sprünge: BMK-Überschrift der BM-Seite -> Schaltzeichen der Hauptfunktion; Mitte der Hauptfunktion -> BM-Seite;
  Einträge unter „Verweise“ -> jeweilige Platzierung (z. B. Nebenfunktion).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/pdfexportgui_k_grundlagen.htm
- **Betriebsmittel-Filter** (Tabelle, Zeilen UND-verknüpft) begrenzt die BM-Seiten, z. B. nur Kabel.
  Kontrollkästchen „Verweise zu Funktionsdarstellungen ausgeben“ und „Verweise zu Auswertungen ausgeben“ steuern
  den Inhalt des Bereichs Verweise.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_bmseiten.htm
- Eigenschaften, die leer sind, den Wert 0 haben oder am Betriebsmittel nicht existieren, werden auf der BM-Seite
  weggelassen. Die Namen der Eigenschaften sind editierbar und mehrsprachig. Bei Artikeleigenschaften:
  „Baugruppen auflösen“ und „Module auflösen“ (Standard aus) samt „Bis zur Stufe“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_bmeigenschaften.htm

**Bedeutung für QET:**
- Ob das Vorlage-PDF BM-Seiten hat, sieht man am Seitenende des PDFs (Seiten mit BMK-Überschrift und den drei
  Bereichen) und an Linkzielen hinter der letzten Planseite. Hat das Original keine, braucht der Nachbau keine.
- Falls gewünscht: BM-Seiten wären im Fork neue Blätter (z. B. Tabelle je Betriebsmittel) mit Texten, deren
  `destination` auf die Platzierungen zeigt. Leere Werte und 0 nicht ausgeben. Vermutung: Für die
  Nachbau-Praxis reicht meist der Dreifachsprung ohne BM-Seiten.
- Einen Betriebsmittelbaum als PDF-Lesezeichen erzeugt der Fork nicht (siehe Abschnitt 15).

## 4. Sprünge aus Auswertungen

- Von Auswertungen in den Schaltplan: z. B. in der Artikelstückliste mittig auf das BMK klicken -> Hauptfunktion
  im Schaltplan.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- Auswertungseinträge sind Glieder der Sprungfolge des Betriebsmittels (Abschnitt 2). Von welchem Eintrag zu
  welchem Ziel genau gesprungen wird (Klemmenplan, Kabelplan), beschreibt die Hilfe nicht einzeln; genannt ist
  nur die Artikelstückliste.
  Quelle: https://www.eplan.help/en-us/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- Querverweise auf Betriebsmittellisten: Eine Hauptfunktion kann einen Verweis auf die Betriebsmittelliste
  anzeigen, wenn ihre Eigenschaft „Querverweisanzeige“ auf „Immer anzeigen“ steht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm
- Hyperlinks in Eigenschaftswerten werden auch auf Auswertungsseiten (über Platzhalter) als Link erkannt und ins
  PDF übernommen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_grafeditor_hyperlinkserkennung_248626.htm

**Bedeutung für QET:**
- Listen (Inhaltsverzeichnis, Stückliste, Klemmen-, Kabelplan) als echte QET-Blätter erzeugen, nicht als
  PDF-Kopie, sonst gibt es keine Sprünge. Jede Zeile bekommt einen Text mit `destination` (Blatt- oder
  Bauteil-UUID), der Fork macht daraus einen Link mit /FitR auf das Bauteil. Das ist bereits Arbeitsweise laut
  Skill `qet-schaltplan-zeichnen` (Abschnitt „Sprungziele wie EPLAN“).
- Klickfläche in Stücklisten: das BMK in der Zeilenmitte (EPLAN: „mittig auf das BMK“). Der ganze BMK-Text als
  Linkfläche genügt.
- Die Stücklistenzeile ist zugleich Kettenglied: Wer die Kette vollständig nachbaut, hängt die Zeile in
  `jump_next/jump_prev` der Bauteile ein. Freie Texte haben im Fork aber nur ein Ziel (keine drei Bereiche).
  Ob das reicht, gegen das Original prüfen.

**Beobachtet am Original-PDF (pymupdf `get_links()`, EPLAN-Plan mit 133 Seiten, 05.11.2026):** schließt die
Lücke „von welchem Eintrag zu welchem Ziel“:
- Klemmen-/Steckerplan: Funktionstext, Klemmen-/Pinnummer, Brückenmarke und Pfad einer Zeile sind EIN Link
  vorwärts in der Sprungfolge. Hat die Leiste eine Folgeseite, zeigt er dorthin (Seite 84 -> 85 -> 86 -> 87);
  auf der letzten bzw. einzigen Seite in den Stromlauf (Kette läuft also um, zurück zur Hauptfunktion).
  Die Zielspalten (Ziel extern/intern) sind KEINE Links.
- Steckerplan: nächstes Glied ist der Stücklisteneintrag des Steckers (Seite 103 -> 131); hat der Stecker keinen
  Stücklisteneintrag, geht es in den Stromlauf.
- Stückliste: SPS-Baugruppen springen auf das Stationsblatt (Hauptfunktion mit Artikel), nicht in die
  Kartenübersicht.
- Deckblatt, Ausführungsdaten, Strukturkennzeichenübersicht: keine Links.
- Ortskasten (z. B. +MASCHINE): die ganze Kastenfläche ist EIN Link zur nächsten Seite mit einem Kasten gleicher
  =Anlage+Ort, am Ende zurück zur ersten (=DB+MASCHINE 30 -> 31 -> 32 -> 33 -> 34 -> 30). Gerätekästen von
  Teilansichten eines Geräts verketten ebenso von Seite zu Seite.
- SPS-Kanal: Neben dem Sprung zur Kartenübersicht ist der Stationsverweis („=SB/55/5“) ein eigener Link aufs
  Stationsblatt.
- Artikelsummenstückliste: je Zeile ist nur die Menge ein Link, Ziel = erste Stücklistenseite mit dem Artikel.
  Die Artikelsumme ist das LETZTE Glied der Sprungfolge eines Geräts; die Kette läuft um, daher springt die
  Hauptfunktion im Schaltplan rückwärts in die Artikelsumme (-K268 -> Artikelsumme), ebenso Teile, die nur im
  Schrankaufbau stehen. Aufbau-Platzierungen springen vorwärts in die Stückliste.
- Reihenfolge der Artikelsumme im Original: Hersteller, dann Bestellnummer (nicht nur Bestellnummer).

## 5. Querverweis- und Unterbrechungspunkt-Sprünge

- Querverweise sind im PDF Sprünge: Mitte einer querverwiesenen Nebenfunktion -> Hauptfunktion; Verweise auf
  BM-Seiten -> Platzierungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- Ob ein angezeigter Querverweistext (z. B. „/12.3“ im Kontaktspiegel) oder ein Unterbrechungspunkt-Querverweis
  im PDF klickbar ist, beschreibt die Hilfe **nicht** ausdrücklich. Beobachtung aus der Linkanalyse von
  EPLAN-PDFs (laut Skill `qet-schaltplan-zeichnen`): Querverweise und Pfeile tragen Links. Das bestätigt die Hilfe
  nur indirekt (größerer Sprungbereich inkl. Eigenschaftstexte).
  Quelle (Lücke): https://www.eplan.help/en-us/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- Begriffe: Die deutsche Hilfe 2.9 nennt die Potenzial-/Signalfortführung „Abbruchstellen“ (früher
  „Potenzialpfeile“). Sie führen Autoconnect-Linien über Spalten oder Seiten fort.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_abbruchstellen.htm
- Abbruchstellen mit gleichem vollständigem BMK bilden Querverweise, in zwei Formen:
  - **Sternquerverweis:** Eine Abbruchstelle ist „Sternquelle“ (Kontrollkästchen in den Eigenschaften). Alle
    anderen gleichen Namens verweisen auf sie. Sie zeigt eine formatierbare Liste aller anderen.
  - **Kettenquerverweis** (Standard, wenn „Sternquelle“ aus): paarweise, die erste verweist auf die zweite, die
    dritte auf die vierte usw., also von Seite zu Seite.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm
  und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_abbruchstellenquerverweiseerzeugen.htm
- Anzeigeformat der Abbruchstellen-Querverweise (Projekteinstellung Querverweise / Kontaktspiegel >
  Abbruchstellen): zeilen-/spaltenweise Anordnung und Anzahl je Zeile/Spalte; „Im QVW nur Seitennamen anzeigen“
  (aus 7.8/L- wird 7/L-); „Strukturkennzeichen vollständig anzeigen“ (z. B. =AA1+BC1/2.5); Zielanzeige an Paar-
  und Sternquerverweisen; Trennzeichen zwischen Name und Querverweis.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektabbruchstellen.htm
- Betriebsmittel-Querverweise: Hauptfunktion verweist auf alle Nebenfunktionen und umgekehrt. Reihenfolge:
  zuerst Nebenfunktionen aus der Gerätedefinition, dann die übrigen; numerische Anschlüsse vor alphanumerischen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm

**Bedeutung für QET:**
- QET verknüpft Folio-Reports nur 1:1 (gehend <-> kommend). Das entspricht dem EPLAN-**Kettenquerverweis**
  (Paare 1-2, 3-4 …). Ein Sternquerverweis (eine Quelle, viele Ziele) ist in QET so nicht abbildbar. Vermutung
  für den Nachbau: Sternquelle als freier Text mit mehreren Zieltexten (je ein Text mit `destination`).
- Im Fork führt ein verknüpfter Pfeil (`jumpTarget()`) mit seiner ganzen Fläche zum Gegenstück, nicht nur sein
  Label (Quelle: Fork `sources/pdf_links.cpp`). Das passt zur EPLAN-Beobachtung.
- Kontaktspiegel-/Querverweiseinträge (CrossRefItem) werden im Fork je Kontakt zu Links (Quelle: Fork
  `sources/pdf_links.cpp`).
- Reihenfolge der Kette (Abschnitt 2) an der EPLAN-Querverweisreihenfolge ausrichten: Gerätedefinition zuerst,
  numerische vor alphanumerischen Anschlüssen.

## 6. Sprungziel-Markierung (roter Blinkrahmen) und Zoom

- Nach einem Sprung markiert ein roter, kurz blinkender Rahmen das Ziel (Betriebsmittel auf Schaltplanseiten
  oder BM-Seiten). Eingeführt mit der Neuerung „Markierungen für Sprungziele im PDF“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_seiten_markierungenpdf_241404.htm
- Mit derselben Neuerung: „Zoom verwenden“ ist standardmäßig aus, Zoomstufe von 50 mm auf 120 mm angehoben.
  Sprünge zwischen Haupt- und Nebenfunktionen von 2D-Montageplatten auf Seiten vom Typ „Schaltschrankaufbau“ sind
  möglich. Eigenschaftsanordnungen von Bauteilen im Bauraum sind im exportierten 3D erkennbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_seiten_markierungenpdf_241404.htm
- Keine roten Rahmen bei PDF/A und (ab den neueren Hilfeständen ausdrücklich) bei aktiviertem Schreibschutz.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/pdfexportgui_k_grundlagen.htm
  und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pdfexportgui_r_allgemein.htm
- „Zoom verwenden“ an: Sprung zeigt das Schaltzeichen zentriert in einem Zoomfenster; aus: ganze Seite.
  „Zoomstufe“ in mm = Höhe des Ausschnitts, kleiner = stärker vergrößert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_allgemein.htm
- Hinweis aus der Praxis: Nach dem Zusammenführen eines EPLAN-PDFs mit einem Deckblatt in Acrobat funktionieren
  Sprünge noch, die roten Blinkmarkierungen aber nicht mehr (Community-Beitrag, keine offizielle Aussage).
  Quelle: https://community.adobe.com/t5/acrobat-discussions/pdf-zusammenf%C3%BChren-eplan-links-verschwinden/m-p/13502452

**Bedeutung für QET:**
- Der Fork springt mit /FitR auf das Ziel (Rahmen mit 25 px Rand, mindestens 160 px). Das entspricht eher
  „Zoom verwenden = an“. Seit der Neuerung ist EPLANs Standard aber **ganze Seite** (Zoom aus) plus roter Rahmen.
  Quelle: Fork `sources/pdf_links.cpp` (`frameRectPdf`, `convertUriToGoTo`).
- Den Blinkrahmen kennt der Fork nicht. Vermutung zur Technik bei EPLAN: JavaScript- oder
  Anzeige-/Ausblende-Aktionen an Annotationen (daher bei PDF/A und Schreibschutz abgeschaltet und nach
  Acrobat-Zusammenführung verloren). Nicht belegt; vor einem Nachbau ein EPLAN-PDF auf `/JavaScript`, `/Hide`
  bzw. zusätzliche Annotationen am Ziel untersuchen.
- Optional im Fork: Schalter „Sprung = ganze Seite (/Fit) statt /FitR“, um EPLAN-Standard nachzubilden.

## 7. Lesezeichen, Seitenbaum und Texte im PDF

- Aus den Lesezeichen springt man zu den Projektseiten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_start.htm
- **Seitenbaum nach** (Registerkarte Allgemein): Seiten im Lesezeichen-Seitenbaum entweder nach
  **Strukturkennzeichen** (Anlage, Ort …) oder nach **Seitentyp** geordnet.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_allgemein.htm
- **Texte in der PDF-Datei** (Tabelle, Registerkarte Allgemein): Eigenschaften und feste Begriffe, die als
  Lesezeichen erscheinen (z. B. Seitentypnamen, Bereichsnamen der BM-Seiten). Spalte „Name“ editierbar. Ausgabe
  in der PDF-Ausgabesprache; fehlt der Text dort, nimmt EPLAN die erste vorhandene Sprache.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_allgemein.htm
- Neben dem Seitenbaum gibt es den Betriebsmittelbaum (Abschnitt 3).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/pdfexportgui_k_grundlagen.htm
- Den genauen Text eines Seiten-Lesezeichens (z. B. Seitenname + Seitenbeschreibung) nennt die Hilfe nicht.
  Vermutung: Strukturebene als Knoten, darunter je Seite Seitenname und Seitenbeschreibung. Im Vorlage-PDF mit
  pymupdf `doc.get_toc()` nachsehen.
  Quelle (Lücke): https://www.eplan.help/en-us/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm

**Bedeutung für QET:**
- Der Fork-Export schreibt nach Code-Durchsicht keine Lesezeichen (/Outlines) – in `cli_export.cpp`,
  `pdf_links.cpp` keine Fundstelle. Für EPLAN-Nähe nachrüsten: nach dem Export Lesezeichenbaum anlegen
  (z. B. per pymupdf `doc.set_toc()` als Nachbearbeitung oder im Fork nach `convertUriToGoTo`), Ebene 1 =
  Strukturkennzeichen (=Anlage/+Ort) bzw. Seitentyp, Ebene 2 = Blatt „Seitenname Beschreibung“. Texte aus
  `get_toc()` des Originals übernehmen.
- Zweiter Baum „Betriebsmittel“ (BMK > Darstellungsart > Funktion) wäre ebenfalls per `set_toc()` mit Zielen auf
  die Bauteilpositionen machbar (Vermutung, nicht umgesetzt).

## 8. Hyperlinks

- Hyperlinks fügt man im Grafischen Editor ein (Einfügen > Grafik > Hyperlink, Registerkarte Hyperlink). Ziele:
  Datei, Internetseite, Mail-Adresse (`mailto:`), auch Pfadvariable. Projektseiten als Ziel nennt die Hilfe
  nicht. Bei Dateien fragt EPLAN, ob die Datei ins Fremddokument-Verzeichnis kopiert wird.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_hyperlinks.htm
  und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/gededitgui_r_hyperlink.htm
- Internetadressen immer mit `http://` oder `https://` beginnen, damit sie auch im PDF-Export funktionieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/gededitgui_r_hyperlink.htm
- Steht ein Hyperlink als Wert einer Eigenschaft, erkennt EPLAN den platzierten Eigenschaftstext automatisch als
  Link, auch auf Auswertungsseiten über Platzhalter. Im exportierten PDF ist er ebenfalls ein Link.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_grafeditor_hyperlinkserkennung_248626.htm
- „Extern verlinkte Dokumente ins Ausgabeverzeichnis kopieren“ (Allgemein): kopiert Fremd-/verlinkte Dokumente,
  die weder eingelagert noch im Standardverzeichnis liegen, ins Zielverzeichnis; sonst werden sie ignoriert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_allgemein.htm

**Bedeutung für QET:** Externe Links (Datenblatt-URLs) im Original mit `get_links()` als Art „uri“ erkennbar.
Der Fork wandelt beim Export alle URI-Links in interne GoTo um (`convertUriToGoTo`). Ob echte externe URLs dabei
erhalten bleiben, ist zu prüfen (Vermutung: nur URIs auf die eigene Datei mit `#page=` werden umgeschrieben).

## 9. Ausgabesprachen

- Dialog „PDF-Ausgabesprachen“: Optionen > Einstellungen > Projekte > „Projekt“ > Verwaltung >
  PDF-Ausgabesprachen oder im PDF-Export unter [Einstellungen] > Ausgabesprachen.
  - **Projekt-Anzeigesprachen:** Das PDF erhält die Anzeigesprachen des Projekts.
  - **Alternative Sprachen:** nur die gewählten Übersetzungssprachen, Reihenfolge per Pfeil. Ist die Liste leer
    (z. B. Sprache entfernt), gelten wieder die Anzeigesprachen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_projekteinstellungen.htm
- Lesezeichen- und BM-Seiten-Texte sind mehrsprachig hinterlegt und folgen der Ausgabesprache.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- API: Parameter LANGUAGE übersetzt das Projekt vor dem Export (z. B. `de_DE`).
  Quelle: https://www.eplan.help/en-US/Infoportal/content/api/2026/export.html

**Bedeutung für QET:** Mehrsprachige Texte sieht man im Original als übereinanderstehende Zeilen gleicher
Bedeutung. Der Nachbau übernimmt die Sprachen so, wie sie im Vorlage-PDF stehen; QET hat keine
Ausgabesprachen-Umschaltung.

## 10. PDF/A und Schreibschutz

- „Archivierungsformat PDF/A“ speichert als **PDF/A-1b**. Modelle (3D) und verlinkte Dokumente fallen weg, rote
  Sprungrahmen sind nicht möglich.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_allgemein.htm
- „Schreibgeschützt“: PDF in Adobe Acrobat nicht mehr änderbar; ab neueren Hilfeständen ausdrücklich ebenfalls
  ohne rote Rahmen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pdfexportgui_r_allgemein.htm
- „Schnelle Web-Anzeige“: erste Seiten erscheinen, während der Rest lädt (linearisiert).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_allgemein.htm

**Bedeutung für QET:** Der Fork setzt eine Dokument-ID aus dem Inhalt (für reproduzierbare PDFs); laut
Code-Kommentar schreibt Qt die ID nur bei PDF/A (Quelle: Fork `sources/cli_export.cpp`). Ob das Vorlage-PDF
PDF/A ist, steht in dessen XMP-Metadaten (`pdfaid:part`). Ist es PDF/A, hat es keine Blinkrahmen – dann ist auch
im Nachbau keiner nötig.

## 11. Dialog PDF-Export und Registerkarten

**Dialog PDF-Export** (Seite > Exportieren > PDF; 2026: Datei > Exportieren > Projektdaten > Seite > PDF):
- Quelle (Projekt oder Seiten, nicht editierbar), Schema (Einstellungssatz, über [...] bearbeitbar), PDF-Datei
  (Standard: Verzeichnis DOC im Projekt, Name = Projektname, Pfadvariablen möglich).
- Ausgabe: Farbig („Farbig auf Weiß“), Schwarz-Weiß (Bilder behalten Farben), Graustufen. Neu in 2026:
  „Weiße grafische Elemente invertieren“ (nur bei Schwarz-Weiß; EPLAN-Farben 8, 255, 263, 264, 266 werden
  schwarz).
- Druckränder verwenden (Rand zum Lochen), 3D-Modell ausgeben (Modelle als Zusatzseiten am Ende), Seiten mit
  Modell-Ansichten ausgeben, Anwenden auf gesamtes Projekt.
- [Einstellungen]: Ausgabesprachen, Ausgabegröße (Papierformat), Druckränder.
- Große Seiten in Originalgröße können Probleme machen; importierte DXF-Blöcke vorher auflösen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_export.htm
und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pdfexportgui_d_export.htm

**Registerkarte Export** (Benutzer > Schnittstellen > PDF-Export):
- Seitenfilter (Schema aus dem Seiten-Navigator; fehlt er später, bricht der Export ab; „-Nicht aktiviert-“ =
  ungefiltert).
- Exportmedium Datenträger (Ausgabeverzeichnis) oder E-Mail (gepackt, Teilgröße 1–10 MB oder unbegrenzt,
  Betreff „<Dateiname> n/m“, MAPI-Client nötig).
- Dateinamen aus Seitennamen oder aus Eigenschaften (2026 zusätzlich Bauraumeigenschaften); Unterverzeichnisse
  keine / aus Seitenbaum / aus Eigenschaften (Formatelement „Verzeichnistrenner“).
- Ausgabe farbig/SW/Graustufen, Druckränder.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_export.htm
und https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pdfexportgui_r_export.htm

**Registerkarte Allgemein:** Zoom verwenden, Zoomstufe, Vereinfachte Sprungfunktion, Schnelle Web-Anzeige,
Schreibgeschützt, Extern verlinkte Dokumente kopieren, Archivierungsformat PDF/A, Seitenbaum nach
(Strukturkennzeichen/Seitentyp), Texte in der PDF-Datei (Details in den Abschnitten 2, 6, 7, 10).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_allgemein.htm

**Registerkarte BM-Seiten:** BM-Seiten erzeugen, Betriebsmittel-Filter, Bereiche (Reihenfolge, an/aus),
Verweise zu Funktionsdarstellungen/Auswertungen ausgeben (Abschnitt 3).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_bmseiten.htm

**Registerkarten BM-Eigenschaften / Artikeleigenschaften:** Liste der ausgegebenen Eigenschaften mit
editierbarem Namen; Baugruppen/Module auflösen bis Stufe (Abschnitt 3).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_bmeigenschaften.htm

**Ablauf** Seiten markieren > Export > Schema wählen > Datei > Medium > Ausgabe > [Einstellungen] prüfen > OK.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_h_seitenexportieren.htm

## 12. Seitenreihenfolge im PDF

- Die Reihenfolge der **Listendarstellung** im Seiten-Navigator bestimmt standardmäßig die Seitenausgabe (Druck,
  PDF) und das Blättern. Soll die Baumreihenfolge gelten, muss die Projekteinstellung „Darstellungsreihenfolge
  auch für Seitenausgaben nutzen“ aktiv sein. Manuelle Seitensortierung ist möglich.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_prinzip.htm

**Bedeutung für QET:** QET exportiert in Folio-Reihenfolge des Projekts. Die Blattfolge des Originals (inkl.
Auswertungen hinten, Inhaltsverzeichnis vorn) muss im Gesamt-Skript nachgebildet werden; Seitenzahlen im
Fork-PDF sind 1-basierte Positionen (`pageMap`, Quelle: Fork `sources/cli_export.cpp`).

## 13. API/Automatisierung

- Aktion `export` mit TYPE=PDFPROJECTSCHEME (ganzes Projekt) oder PDFPAGESSCHEME (Seiten über PAGENAMEn/SELn),
  EXPORTFILE, EXPORTSCHEME (leer = zuletzt benutztes Schema), BLACKWHITE (0 Farbe, 1 SW, 2 Graustufen,
  3 Weiß invertiert), USEZOOM, ZOOMLEVEL (1–3500 mm), USESIMPLELINK (1 = einfache Links, 0 = Dreifachsprung),
  FASTWEBVIEW, READONLYEXPORT, USEPRINTMARGINS, LANGUAGE, EXPORTMODEL.
  Quelle: https://www.eplan.help/en-US/Infoportal/content/api/2026/export.html

## 14. Neuerungen nach Version

| Stand | Neu/anders | Quelle |
|---|---|---|
| Neuerung „Markierungen für Sprungziele“ (in 2.9-Hilfe enthalten, genaue Version nicht genannt) | roter Blinkrahmen; Zoom standardmäßig aus; Zoomstufe 50 -> 120 mm; Sprünge an 2D-Montageplatten; Eigenschaftsanordnungen im 3D | https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_seiten_markierungenpdf_241404.htm |
| ab 2.9 (gegenüber „früheren Versionen“) | Sprungbereich inkl. platzierter Eigenschaftstexte | https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm |
| 2025/2026 | Menüpfad „Datei > Einstellungen“ statt „Optionen > Einstellungen“; Schreibschutz unterdrückt rote Rahmen ausdrücklich | https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/pdfexportgui_k_grundlagen.htm |
| 2026 | „Weiße grafische Elemente invertieren“; Dateinamen auch aus Bauraumeigenschaften; Pfad „Datei > Exportieren > Projektdaten > Seite > PDF“ | https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pdfexportgui_d_export.htm |

## 15. Ausgabegröße, Druckränder, Einstellungsschema, PDF-Kommentare

(Ergänzung aus der Hilfe-Durchsicht B6.)

- **Eine Datei:** Beim PDF-Export von einzelnen Seiten oder ganzem Projekt entsteht immer genau **eine** PDF-Datei.
  Das Schema (Benutzer > Schnittstellen > PDF-Export) bündelt die Registerkarten Export, Allgemein, BM-Seiten,
  BM-/Artikeleigenschaften.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_einstellexport.htm
- **Ausgabegröße** ([Einstellungen] > Ausgabegröße bzw. Station > Grafische Bearbeitung > PDF-Ausgabegröße):
  **Originalgröße** (Standard) = Papierformat aus dem Normblatt der Seite; **Auf Seite skalieren** = auf ein gewähltes
  Format, Seitenverhältnis bleibt, Seiten werden **nicht gedreht** (A4-quer-Seite in A4-hoch -> auf A5-Größe
  verkleinert). Breite/Höhe überschreibbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_einstellausgabegroesse.htm
- Wählbare Formate: A0 841×1189, A1 594×841, A2 420×594, A3 297×420, A4 210×297, A5 148×210 mm, ANSI A (Letter)
  216×279, ANSI B (Ledger/Tabloid) 279×432, ANSI C 432×559, ANSI D 559×864, ANSI E 864×1118 mm.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_auswahlausgabeformat.htm
- **Druckränder** im PDF-Export (Links/Rechts/Oben/Unten, Druckposition) sind dieselben Werte wie unter Projekt >
  Drucken > [Einstellungen]; Änderungen hier wirken dort mit.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_druckraender.htm
- **PDF-Kommentare:** Kommentare aus einer (geprüften) PDF-Datei lassen sich ins Projekt importieren und im
  Kommentare-Navigator (Seite > Kommentare-Navigator) verwalten: Baum nach Seitenstruktur, Liste konfigurierbar,
  „Gehe zu (Grafik)“ springt zum Kommentar auf der Seite, Status in den Eigenschaften änderbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_kommentarnavigator.htm

**Bedeutung für QET:**
- Die Seitengröße im EPLAN-PDF ist bei „Originalgröße“ das Normblattformat (meist A3 quer). Weicht die MediaBox des
  Originals davon ab (z. B. A4 mit skaliertem Inhalt), wurde „Auf Seite skalieren“ benutzt – für den Nachbau zählt das
  Normblattformat, nicht die PDF-Größe.
- Druckränder verschieben im PDF nur den Inhalt (Lochrand); Koordinaten aus dem Original-PDF dann um den Rand
  korrigieren, bevor man Positionen ins QET-Raster überträgt (**Vermutung**, am Original prüfen: gleichmäßiger Versatz
  aller Seiten).
- PDF-Kommentare (Rückläufer vom Kunden) sind Annotationen, keine Zeichnungsinhalte – beim Auslesen eines Original-
  PDFs ignorieren bzw. als Revisionshinweise sammeln.

## 16. Stand im QET-Fork und offene Punkte

Vorhanden (Code-Durchsicht `sources/pdf_links.cpp`, `sources/cli_export.cpp`, `sources/qetgraphicsitem/element.cpp`):
- Dreifachsprung über `jump_next`/`jump_main`/`jump_prev` (Drittel der Bauteilfläche).
- Verknüpfte Folio-Reports: ganze Pfeilfläche springt zum Gegenstück.
- Kontaktspiegel/Querverweise (CrossRefItem) je Kontakt als Link.
- Kabel-Querverweise und Kabellabel mit `destination` -> Zielblatt; Klemmenleisten-Pläne -> Bauteil.
- Freie Texte mit `destination` -> Blatt (/Fit) oder Bauteil (/FitR).
- Interne GoTo-Aktionen (funktionieren in jedem Viewer), sortierte Linkausgabe (reproduzierbar).

Fehlt gegenüber EPLAN:
- Lesezeichen (Seitenbaum, Betriebsmittelbaum).
- Roter Blinkrahmen am Ziel; Standardsprung „ganze Seite“.
- BM-Seiten.
- Sternquerverweis an Abbruchstellen (QET nur 1:1).

Aufgaben für das Verknüpfungsskript (`verknuepfen.py`):
1. Je Betriebsmittel geordnete Sprungkette bilden (Platzierungen + Auswertungszeilen, Reihenfolge wie EPLAN
   „Verweise“) und `jump_next/jump_prev/jump_main` setzen.
2. Nebenfunktionen: `jump_main` = Hauptfunktion.
3. Listenzeilen: `destination` = Hauptfunktion (Stückliste) bzw. Klemme/Kabel.
4. Nach dem Export Linkzahlen je Art gegen das Original (`get_links()`) vergleichen.

## 17. Quellen

Gelesen und erreichbar:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- https://www.eplan.help/en-us/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/pdfexportgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pdfexportgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_h_seitenexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_export.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pdfexportgui_d_export.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_export.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pdfexportgui_r_export.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_allgemein.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/pdfexportgui_r_allgemein.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_bmseiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_r_bmeigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_projekteinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_seiten_markierungenpdf_241404.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_hyperlinks.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/gededitgui_r_hyperlink.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_grafeditor_hyperlinkserkennung_248626.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_abbruchstellenquerverweiseerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektabbruchstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_abbruchstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_abbruchstellenquerverweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_prinzip.htm
- https://www.eplan.help/en-US/Infoportal/content/api/2026/export.html
- https://community.adobe.com/t5/acrobat-discussions/pdf-zusammenf%C3%BChren-eplan-links-verschwinden/m-p/13502452 (nur Suchtreffer-Auszug, inoffiziell)

Ergänzt in der Durchsicht B6 (lokal gelesene Hilfe 2.9):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_einstellexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_einstellausgabegroesse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_auswahlausgabeformat.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pdfexportgui_d_kommentarnavigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_druckraender.htm

Nicht erreichbar (HTTP 404):
- https://www.eplan.help/help/platform/2.8/de-de/help/Content/htm/pdfexportgui_k_grundlagen.htm
- https://www.eplan.help/help/platform/2.8/en-us/help/Content/htm/pdfexportgui_k_grundlagen.htm
- https://eplan.help/en-us/Infoportal/Content/Plattform/2.8/Content/htm/pdfexportgui_k_grundlagen.htm

Fork-Code (gelesen, nicht geändert): `qelectrotech-eplan/sources/pdf_links.cpp`, `pdf_links.h`,
`cli_export.cpp`, `qetgraphicsitem/element.cpp`.
