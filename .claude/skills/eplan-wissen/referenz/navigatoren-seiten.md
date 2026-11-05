# EPLAN: Seiten-Navigator, Seiteneigenschaften, Seitentypen, Suchen/Ersetzen, Projektstruktur-Verwaltung

Wie EPLAN Seiten verwaltet (Seitenname, Unterseiten, Seitentypen, Eigenschaften, Sortierung, Pfadnummerierung),
wie Seiten kopiert und strukturell angepasst werden, wie Suchen/Ersetzen und die „Gehe zu“-Sprünge arbeiten und was
beim Kopieren zwischen unterschiedlichen Projektstrukturen passiert. Grundlage: EPLAN-Plattform-Hilfe 2.9, in eigenen
Worten. URL-Präfix aller Quellen, sofern nicht anders: `https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/`
(genannt wird nur der Dateiname). Betriebsmittel-Navigator: `betriebsmittel.md` Abschnitt 17; Kennzeichnung:
`kennzeichnung.md`. „Vermutung“ = nicht durch die Hilfe belegt.

## Inhalt

1. Seiten-Navigator: Baum und Liste
2. Seiteneigenschaften und Seitenname
3. Seitentypen (Logik/Grafik, interaktiv/automatisch)
4. Seiten erstellen, öffnen, löschen
5. Seiten nummerieren, sortieren, Pfadnummerierung
6. Seiten kopieren und Struktur anpassen
7. Kopieren zwischen unterschiedlichen Projektstrukturen
8. Seitenfilter
9. Fremddokumente
10. Suchen und Ersetzen
11. Ergebnislisten und „Gehe zu“
12. Projektstruktur-Verwaltung (Baumdarstellung, benutzerdefinierte Strukturen)
13. Bedeutung für QET (Zusammenfassung)
14. Quellen

---

## 1. Seiten-Navigator: Baum und Liste

- Zeigt alle geöffneten Projekte, ihre Strukturkennzeichen und Seiten; Seiten öffnen, erstellen, kopieren, löschen,
  Eigenschaften bearbeiten, importieren/exportieren, sichern, nummerieren, drucken. Stammdaten werden nicht angezeigt.
  Bearbeitungen nur innerhalb **eines** Projekts gleichzeitig. Typische Projektgröße laut Hilfe ~300 Seiten.
  Quelle: pagebrowsergui_k_start.htm, pagebrowsergui_k_prinzip.htm
- **Baum**: erste Ebene Projekte, darunter Seiten gemäß **Seitenstruktur**; standardmäßig nur die **identifizierenden**
  und verwendeten Strukturkennzeichen; beschreibende und unbenutzte nicht. Optional oberste Ebene nach **Seitentyp** in
  fester Reihenfolge: Titel/Deckblatt, Inhaltsverzeichnis, Schaltplan einpolig, Schaltplan allpolig, Schaltplan Fluid,
  Übersicht, Funktionsübersicht (Fluid), Schaltschrankaufbau, Grafik, Fremddokument, RI-Fließbild, Auswertungen
  (alphanumerisch). Unterkennzeichen in einer Ebene oder als Unterebenen. Geöffnete Seiten fett; Name direkt editierbar.
  Quelle: pagebrowsergui_k_prinzip.htm, projectstructure_h_kennzeichenausblenden.htm
- **Liste**: alle Seiten in Reihenfolge, Spalten frei konfigurierbar (alle Seiteneigenschaften), einige Felder direkt
  editierbar (Seitenbeschreibung, Zeichnungsnummer …), manuelle Seitensortierung möglich.
- **Wichtig:** Maßgeblich für Drucken, PDF-Export und Blättern ist standardmäßig die Reihenfolge der **Listendarstellung**;
  die Baumreihenfolge gilt dafür nur mit der Projekteinstellung „Darstellungsreihenfolge auch für Seitenausgaben nutzen“.
  Quelle: pagebrowsergui_k_prinzip.htm
- Kontextmenü u. a.: Neu, Öffnen, In neuem Fenster öffnen, Ausschneiden/Kopieren/Einfügen, Löschen, Umbenennen,
  Manuelle Seitensortierung (nur Liste). Drag & Drop verschiebt, mit Strg kopiert. Quelle: pagebrowsergui_d_seitenuebersicht.htm,
  pagebrowsergui_h_seitennummerieren.htm
- Grafische Vorschau (Ansicht > Grafische Vorschau): mehrere markierte Seiten nebeneinander; Doppelklick öffnet, mit
  Umschalt in neuem Fenster. Quelle: pagebrowsergui_h_seitenvorschau.htm

## 2. Seiteneigenschaften und Seitenname

- **Seitenname**: kompletter Name mit Trennzeichen und Unterseite, **ohne** Strukturkennzeichen; beliebige Ziffern/
  Buchstaben. **Unterseite** durch **Punkt** getrennt (bei mehreren Punkten gilt der erste von links), vier Stellen
  vorgesehen. Höchstlänge: Seitenname **32**, Unterseite **16** Zeichen (sonst Sortierung nicht gewährleistet).
  Quelle: pagebrowsergui_k_eigenschaften.htm, pagebrowsergui_k_start.htm
- **Seitenname (vollständig)** = alle Strukturkennzeichen (identifizierend und beschreibend) + Seitenname;
  **Seitenname (identifizierend)** = nur identifizierende + Seitenname. Quelle: pagebrowsergui_k_eigenschaften.htm
- **Zeichen für Unterseite** (Projekteinstellung): keine / Kleinbuchstaben (Standard: `1.a, 1.b`) / Großbuchstaben
  (`1.A`) / Zahlen (`1.1`); Änderung im bestehenden Projekt ändert die Reihenfolge nicht. **Min. Länge des
  Seitennamens**: führende Nullen bei der Ausgabe (Standard 0; manuell eingetippte führende Nullen werden dann
  unterdrückt). Quelle: pagebrowsergui_k_eigenschaften.htm, pagebrowsergui_d_projekteinstellungen.htm
- Weitere Eigenschaften: **Seitentyp**, **Seitenbeschreibung** (mehrsprachig, Zeilenumbrüche erlaubt), **Zusatzfeld
  Blattnummer** (einmal je Seite mit gleichem Strukturkennzeichen; bei Auswertungen ab Startnummer automatisch
  hochgezählt; kann Lücken/Doppelte haben), **Zähler Seitenzahl (ID 25020)** für eine echte fortlaufende Seitennummer,
  Maßstab, Raster (Logik 4 mm, Grafik 1 mm; Fluid 2 mm, RI 2,5 mm), Normblatt (Seiteneinstellung überschreibt Projekt;
  bestimmt Papierformat/Ausrichtung), Formular, automatisches Änderungsdatum und letzter Bearbeiter (nicht editierbar),
  manuelles Änderungsdatum, Ersteller. „Seitenanzahl/-namen pro Eigenschaft“ zählt Seiten mit gleichem Wert einer
  gewählten Eigenschaft (Ergebnis in Seitenanzahl/Seitennamen/Seitenzähler pro Eigenschaft). Quelle: pagebrowsergui_k_eigenschaften.htm,
  pagebrowsergui_d_projekteinstellungen.htm, pagebrowsergui_h_seitenanlegen.htm
- Dialog **Vollständiger Seitenname**: Tabelle der Kennzeichnungsblöcke der Seitenstruktur mit Vorzeichen und Wert
  (Auswahl vorhandener Kennzeichen über [...]), darunter Seitenname. Vorschlag: erste Seite „1“, sonst nächster freier
  Name; bei DIN gruppenweise, bei DIN/JIC mit Unternummer, wenn die aktuelle Seite eine hat (Zahlen: 4 → 5;
  Kleinbuchstaben: 5.a → 5.b). Quelle: pagebrowsergui_d_seitenname.htm
- **Ändern identifizierender Seiteneigenschaften** (z. B. Anlage) benennt alle BMK auf der Seite mit gleicher Anlage mit
  um. Quelle: pagebrowsergui_h_eigenschaftenbearbeiten.htm

**Bedeutung für QET:** QET-Folio-Felder: `folio` = Seitenname inkl. Unterseite (`27.a`), `plant`/`locmach` =
identifizierende Strukturkennzeichen, Titel = Seitenbeschreibung. Zur Sortierung nicht der Folio-Index, sondern die
Reihenfolge der Seiten im Original (Inhaltsverzeichnis / PDF-Reihenfolge = Listendarstellung). Führende Nullen im
Original-Seitennamen (`001`) können Ausgabeformat sein („Min. Länge“) – Vermutung: als angezeigter Text übernehmen, für
Vergleiche numerisch normalisieren.

## 3. Seitentypen (Logik/Grafik, interaktiv/automatisch)

- **Logikseiten** werten Betriebsmitteldaten aus und erzeugen Autoconnect-Linien (außer Schaltschrankaufbau, Übersicht,
  Vorplanung: kein Autoconnecting). **Grafikseiten** nur Grafik; dort platzierte Schaltzeichen sind grafisch (nicht
  ausgewertet). **Interaktiv (I)** = selbst gezeichnet (Schaltplan, Grafik …), **automatisch (A)** = Auswertungen
  (Klemmenplan, Kabelplan, Artikelstückliste …). Seitentyp änderbar, alle anderen Eigenschaften bleiben; Ausnahme
  Fremddokument (nicht mehr änderbar). Seitentypen beschleunigen Auswertungen (irrelevante Seiten werden übersprungen).
  Quelle: pagebrowsergui_k_seitentypen.htm, devicetaggui_k_grafischeschaltzeichen.htm

**Bedeutung für QET:** QET kennt keine Seitentypen. Für den Nachbau (Vermutung): Seitentyp je Folio in einem
Zusatzfeld (z. B. Folio-Eigenschaft oder Titelblock-Variable) führen, damit Skripte Logikseiten (Schaltplan einpolig/
allpolig) von Auswertungsseiten (Klemmenplan, Stückliste, Inhaltsverzeichnis) und Grafikseiten unterscheiden; Elemente
auf Grafik- und Auswertungsseiten nie in Querverweise/Stückliste aufnehmen.

## 4. Seiten erstellen, öffnen, löschen

- Seite > Neu: übernimmt Eigenschaften der markierten Seite (bei markierter Strukturebene die der ersten Seite darunter);
  Raster aus dem Normblatt; [Felder leeren] leert alles außer Seitentyp, Namensvorschlag, Maßstab, Raster; [Übernehmen]
  erzeugt je Klick eine Seite; doppelte Namen werden abgewiesen. Bei Pfadnummerierung projektweise/je Strukturkennzeichen
  Abfrage, ob Pfadbezeichnungen aktualisiert werden. Quelle: pagebrowsergui_h_seitenanlegen.htm, pagebrowsergui_d_seiteneigenschaften.htm
- Löschen einer Seite entfernt **platzierte** Bauteile vollständig; **nicht-platzierte** bleiben (geschützte Geräte
  bleiben als nicht-platzierte Funktionen, siehe `betriebsmittel.md` Abschnitt 12). Quelle: pagebrowsergui_h_seitenanlegen.htm
- Öffnen als Registerkarte; „In neuem Fenster öffnen“; Blättern mit Bild auf/ab in der Reihenfolge der Seiten (bei
  aktivem Filter nur durch gefilterte Seiten). Mehrbenutzer: eine Seite ist gesperrt, solange jemand darauf arbeitet.
  Änderungen werden beim Seitenwechsel automatisch gespeichert. Quelle: pagebrowsergui_h_seitenoeffnen.htm, pagebrowsergui_k_filter.htm

## 5. Seiten nummerieren, sortieren, Pfadnummerierung

- **Seiten nummerieren**: Startnummer, Schrittweite, „Nummerierung strukturbezogen“ (DIN, je Strukturkennzeichen),
  „Abstand beibehalten“ (1, 2, 3, 8 ab 2 → 2, 3, 4, 9), „Text beibehalten“ (alphabetischer Teil bleibt), Unterseiten
  beibehalten / durchnummerieren (je Hauptseite ab 1) / in Hauptseiten umwandeln. Kollisionen zeigt die Ergebnisvorschau
  mit Sternchen und rot; erst ohne Sternchen übernehmbar. Umbenennen einer Einzelseite behält die Beschreibung.
  Quelle: pagebrowsergui_h_seitennummerieren.htm, pagebrowsergui_d_seitennummerierung.htm, pagebrowsergui_d_seitennummerierungsvorschau.htm
- **Manuelle Seitensortierung** (Projekteinstellung + Dialog mit Pfeiltasten, „Standardsortierung“ setzt zurück): wirkt
  auf Blättern, Drucken, **Querverweise**; nur auf die Liste, nicht den Baum. Quelle: pagebrowsergui_d_projekteinstellungen.htm,
  pagebrowsergui_d_manuelleseitensortierung.htm
- **Pfadnummerierung**: seitenweise (je Seite ab Startwert) / projektweise (über alle **Logikseiten** fortlaufend) / je
  Strukturkennzeichen (nicht bei Gesamtnummerierung); Fremddokumente immer ausgenommen. Zusätze: **Nur Logikseiten**
  (andere seitenweise), **Lücken in Seitennummern berücksichtigen** (fehlende Seiten × Pfade des Standardnormblatts frei
  lassen: Seiten 1, 2, 5 mit 10 Pfaden → 0–9, 10–19, 40–49), **Bei Unterseiten Unternummern erzeugen** (Seite 1.A:
  `0.A … 9.A`, Hauptnummern von der Hauptseite). Quelle: pagebrowsergui_d_projekteinstellungen.htm

**Bedeutung für QET:** Die Spaltennummern am Blattrand des EPLAN-PDF sind Pfadbezeichnungen und hängen von dieser
Einstellung ab (meist seitenweise 0–9 oder 1–10; projektweise fortlaufend möglich). Querverweise im Original
(`/27.3`) verweisen auf Seite + **Pfadbezeichnung**, nicht auf eine geometrische Spalte. QET-Folio-Raster (Spaltenanzahl,
Startwert, Beschriftung) muss die Pfadbezeichnungen des Originals reproduzieren; bei projektweiser Nummerierung eigene
Spaltenbeschriftung je Folio setzen (Vermutung: über Titelblock-/Raster-Offset, falls der Fork das kann).

## 6. Seiten kopieren und Struktur anpassen

- Innerhalb eines Projekts (Bearbeiten > Kopieren/Einfügen, Drag & Drop mit Strg) oder zwischen Projekten (Seite >
  Kopieren von/in, zwei Bäume, Filter, Vorschau bis 12 Seiten). Danach Dialog **Struktur anpassen**: Registerkarte
  **Seiten** (Quelle/Ziel mit Strukturkennzeichen, Seitennummer, Blattnummer, Beschreibung; Ziel editierbar,
  „Überschreiben“, „Seitenname automatisch“ = nächsthöhere Nummer in der Teilstruktur, [Nummerieren], [Zurücksetzen] =
  Originalstruktur, z. B. für Mehrseitenmakros) und Registerkarte **BMK** (sichtbare BMK mit Strukturkennzeichen ändern;
  neue Kennzeichen werden alphabetisch oder am Ende einsortiert). [Extras] > **Strukturen bearbeiten**: Haupt- und bis zu
  9 Unterkennzeichen je Block in einem Arbeitsgang (Kopieren per Strg+C/V). Anschließend ggf. Dialog Einfügemodus für
  die BMK-Nummerierung. Gleiches gilt beim Einfügen von **Seitenmakros**. Quelle: pagebrowsergui_h_seitenkopieren.htm,
  pagebrowsergui_h_seitenkopierenprojekte.htm, pagebrowsergui_h_seitenstrukturanpassen.htm, pagebrowsergui_r_seitenstruktur.htm,
  pagebrowsergui_r_ortkaestenstruktur.htm, pagebrowsergui_d_strukturenbearbeiten.htm, pagebrowsergui_d_seitenkopieren.htm

## 7. Kopieren zwischen unterschiedlichen Projektstrukturen

- Seiten in ein Projekt mit flacherer Struktur: Blöcke, die im Ziel **beschreibend** sind, bleiben; Blöcke, die im Ziel
  **nicht verfügbar** sind, werden entfernt (z. B. Anlage+Einbauort → Gesamtnummerierung: Strukturkennzeichen weg).
  Wird ein Block im Ziel nur beschreibend, können Seitennamen ihre Eindeutigkeit verlieren → neu nummerieren.
- Betriebsmittel (auch Kabel, Abbruchstellen, Geräte- und Strukturkästen): **sichtbares BMK bleibt immer erhalten**;
  ins identifizierende Kennzeichen kommen nur die Blöcke der Zielstruktur.
  Quelle: projectstructure_k_verhkopinbmubauteilen.htm, pagebrowsergui_h_seitenstrukturanpassen.htm

## 8. Seitenfilter

- Feldbasierter Filter (Schema) für Baum und Liste aller geöffneten Projekte; Blättern, Mehrfach-Bearbeiten, Drucken
  (Option „Markierung“) und Export (ohne „Anwenden auf gesamtes Projekt“) nur für gefilterte Seiten. Kriterien:
  Seiteneigenschaften, **Enthält Ergebnisse in der Suchergebnisliste (ID 11045)**, und **Funktion auf Seite**
  (Strukturkennzeichen, Darstellungsart, Gewerk, Funktionsdefinition – z. B. alle Seiten mit PE-Klemmen einer bestimmten
  Funktionsdefinition oder mit Funktionen am Einbauort B4). Leere Strukturebenen verschwinden aus dem Baum. Neu
  erzeugte Seiten erscheinen nur, wenn sie den Filter erfüllen. Quelle: pagebrowsergui_k_filter.htm

## 9. Fremddokumente

- Seitentyp **Fremddokument** bindet Word/PDF u. a. ein (Pfad in den Seiteneigenschaften; ohne Pfad = Projektverzeichnis,
  Kopie nach `…\<Projekt>\DOC` wird angeboten). Wie normale Seiten verwaltet (sortieren, nummerieren, eigenes Normblatt –
  wirkt nur beim **PDF-Export**), im Inhaltsverzeichnis immer **eine** Seite, kein Durchblättern, kein Maßstab. Druck über
  das zugehörige Programm, immer am Ende in getrennten Druckjobs, ohne Normblatt und ohne Mitzählen der Seitennummern;
  nicht in der Druckvorschau; Hyperlink-Dokumente nur über „Begleitdokumente drucken“. Pfadnummerierung ignoriert sie.
  Quelle: pagebrowsergui_k_fremddokumente.htm, pagebrowsergui_d_projekteinstellungen.htm

**Bedeutung für QET:** Im EPLAN-PDF eingebettete Fremddokumente (Datenblätter) erscheinen als eine Seite mit Normblatt;
beim Nachbau als eigenes Folio mit Bild/Verweis oder weglassen und im Inhaltsverzeichnis vermerken.

## 10. Suchen und Ersetzen

- Suchbar: Texte, BMK, Symbole, Kabel, Kabelverbindungen, Potenziale, Makros, Stammdaten, Seiten- und Projektkopfdaten,
  Eigenschaften – im ganzen Projekt oder auf Seiten; auch unsichtbare Objekte und Ebenen. Gesucht wird in allen
  platzierbaren Objekten (Seiten, Bauräume), in Eigenschaften platzierter und nicht-platzierter Objekte, in Seiten-,
  Bauraum- und Projektdaten (jeweils ohne deren Namen). Zusammenhängende Begriffe nur in genau dieser Reihenfolge.
  Quelle: searchandreplacegui_k_start.htm, searchandreplacegui_k_prinzip.htm
- Dialog Suchen: Filter für Funktionen (Darstellungsart, Funktionsdefinition), Groß-/Kleinschreibung, nur ganze Texte,
  **Suchen in** (u. a. BMK/Bezeichnung: allgemeine BM inkl. Geräte-/SPS-Kästen und Übersichtsseiten, Klemmen als
  `BMK:Klemme` z. B. `-X1:12`, Stecker `BMK:Stift`, Geräteanschlüsse `BMK:Anschluss` …), **Suchen auf** (Seitentypen).
  Quelle: searchandreplacegui_d_suchen.htm, searchandreplacegui_h_suchen.htm
- **BMK-Suche und Struktur**: Strukturelemente werden nur beachtet, wenn sie im Suchbegriff stehen – dann muss vom BMK
  nach links eine **lückenlose Teilstruktur** mit Vorzeichen angegeben werden, ohne Platzhalter (bei Struktur `==`, `=`,
  `++`, `+`: erlaubt `+O-K1`, `++OO+O-K1`, `=A++OO+O-K1`; **nicht** `=A-K1`). Ersetzen ändert nur das sichtbare BMK ohne
  Strukturteile; Funktionen mit **BMK-Übernahme** (sichtbares BMK leer) werden weder gefunden noch ersetzt.
  Quelle: searchandreplacegui_k_prinzip.htm
- **Platzhalter** wie MS Access: `*` beliebig viele (nur am Anfang/Ende), `?` ein Zeichen, `[ae]` eines aus Menge,
  `[!ae]` keines davon, `[a-c]` Bereich, `#` eine Ziffer; Sonderzeichen `* ? # [ -` in eckige Klammern (`[?]`).
  Quelle: searchandreplacegui_k_platzhalter.htm
- **Ersetzen** in markierten Ergebniseinträgen (Suchen nach / Ersetzen durch, Wörterbuch; Übersetzungen bleiben
  unverändert). Ergebnisse per Eigenschaftendialog/Blockbearbeitung bearbeitbar (Seiten-, Projektdaten und
  Schaltplanobjekte nicht gemischt). **Auswahl synchronisieren** markiert dasselbe Objekt in allen offenen Navigatoren.
  Quelle: searchandreplacegui_h_ersetzen.htm, searchandreplacegui_d_ersetzen.htm, searchandreplacegui_h_bearbeiten.htm,
  searchandreplacegui_h_auswahlsynchronisieren.htm

## 11. Ergebnislisten und „Gehe zu“

- Zwei Listen im Dialog Suchergebnisse: **Ergebnisliste** (sammelt, wird nicht automatisch geleert) und **Gehe zu**
  (wird vor jedem Füllen geleert). Eintrag: Objektart, Objektname + **erste Anschlussbezeichnung** (`-K1:A1`), Eigenschaft,
  Wert; Reihenfolge = **grafische Reihenfolge** (Seite 1, Seite 2 …, nicht-platzierte, Projekt), eine Zeile je Sprache.
  Eingetragen werden Texte, Funktionen (sichtbares BMK), Abbruchstellen (Name), VDP (Verbindungsbezeichnung),
  PDP (Potenzialname), Platzhaltertexte von Auswertungen; nicht: Strukturkästen, Winkel/T/Brücken/Kreuzungen, Grafik,
  Verbindungen, Montageplatten, Makrokästen, Platzhalterobjekte. Nächster/Vorheriger Eintrag springt in Plan-Reihenfolge.
  Quelle: searchandreplacegui_k_ergebnislisten.htm, searchandreplacegui_k_eintraegeergebnisliste.htm, searchandreplacegui_d_suchergebnisse.htm
- **Gehe zu (querverwiesene)**: Ausgangsobjekt + alle mit ihm querverwiesenen + deren Querverweise (Kontakt → Spule und
  **alle anderen Kontakte**), auch auf einpoligen, Übersichts- und Schaltschrankaufbau-Seiten und Platzhaltertexte auf
  Auswertungsseiten; bei Abbruchstellen **alle zusammenhängenden**, nicht nur das Paar. **Gehe zu (alle Darstellungsarten)**:
  alle Darstellungen der Funktion (einpolig, allpolig, Paarquerverweis, Übersicht, Auswertung). **Gehe zu
  Artikelauswertungen**: Stellen in Artikel-(summen)stücklisten. **Gehe zu (Gegenstück)** (nur Grafikeditor): bei
  Paarquerverweisen zum Gegenstück (SPS, Abbruchstelle), bei Nebenfunktionen zur **Hauptfunktion**. **Gehe zu (Grafik)**:
  zum platzierten Objekt; bei Verbindungen zum grafisch ersten VDP, sonst zur Quellfunktion; auf Auswertungsseiten zu den
  querverwiesenen Objekten (z. B. Inhaltsverzeichnis → Seite); Strg + Mauszeiger zeigt Hand, wenn ein Sprung möglich ist;
  auch in andere (ggf. erst zu öffnende) Projekte. Quelle: searchandreplacegui_k_ergebnislisten.htm

**Bedeutung für QET:** Diese Sprungziele sind dieselben, die EPLAN im PDF als Links ausgibt (siehe `pdf-export.md`,
Dreifachsprung): Kontakt → Spule (Gegenstück), Abbruchstelle → Gegenstück, Inhaltsverzeichnis-/Auswertungszeile →
Objekt. Für den QET-PDF-Export: je Element ein Linkziel am Einfügepunkt; Verbindungen zielen auf ihre Drahtnummer
(VDP) bzw. Quellfunktion; Auswertungszeilen verlinken auf das Objekt, nicht nur auf die Seite.

## 12. Projektstruktur-Verwaltung (Baumdarstellung, benutzerdefinierte Strukturen)

- Projektstruktur = Seiten- + Betriebsmittelstruktur aus vordefinierten oder benutzerdefinierten Schemata; je Block
  identifizierend/beschreibend/nicht verfügbar; exportier-/importierbar; erweiterte Referenzkennzeichnung (EN 81346,
  Produktkennzeichen) und Seitenkennzeichnung nach DIN EN 61355-1 (Objektkennzeichen). Details: `kennzeichnung.md`.
  Quelle: projectstructure_k_start.htm
- Benutzerdefinierte Struktur (Projekt > Eigenschaften > Struktur, [...] neben Seiten- bzw. BM-Struktur; Reihenfolge per
  Pfeiltasten). Vor-/Trennzeichen (über [Weitere] > Erweiterte Projektstrukturen) nur für Blöcke, die identifizierend
  oder beschreibend sind: Funktionale Zuordnung `==`/`#`/`≠` mit `.` oder `==`; Anlage `=`; Aufstellungsort `++`;
  Einbauort `+` (jeweils Trenner `.` oder Wiederholung); Anlagennummer, Benutzerdefiniert frei; Dokumentenart Leerzeichen
  oder `&`. Verboten: `/`, `\`, `?`, `:`. Quelle: projectstructure_h_frhierarchienerstellen.htm
- **Baumdarstellung** in allen Navigatoren (Projekt > Darstellung > Baumstruktur (Seiten) bzw. analog): Blöcke ein-/
  ausblenden, zusätzliche Eigenschaften (Seitentyp, BMK: Kennbuchstabe …) einblenden, Reihenfolge ändern. Standard: nur
  identifizierende, verwendete Kennzeichen. Quelle: projectstructure_h_kennzeichenausblenden.htm

## 13. Bedeutung für QET (Zusammenfassung)

| EPLAN | QET-Nachbau |
|---|---|
| Seitenname ohne Struktur, Unterseite nach erstem Punkt | `folio` = `27` bzw. `27.a`; Struktur in `plant`/`locmach` |
| Seitenname nur innerhalb identifizierender Struktur eindeutig | Folio-Schlüssel immer `=Anlage+Ort/Seite` |
| Reihenfolge der Listendarstellung bestimmt Druck/PDF | Folio-Reihenfolge = PDF-Reihenfolge des Originals |
| Pfadbezeichnungen seitenweise/projektweise/je Struktur | Spaltenbeschriftung des Folio-Rasters wie im Original |
| Logik-/Grafik-/Auswertungsseiten | Seitentyp als Zusatzinfo je Folio; nur Logikseiten verknüpfen |
| Zähler Seitenzahl (ID 25020) vs. Blattnummer | laufende Blattnummer (`%f`) und Seitenname (`%F`) nicht verwechseln |
| Gehe zu / Gegenstück | PDF-Linkziele wie in `pdf-export.md` |
| Kopieren: sichtbares BMK bleibt, Struktur vom Ziel | Bauteil-/Seitenimport: sichtbares BMK unverändert, Struktur des Ziel-Folios |

## 14. Quellen

Alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/

- Seiten-Navigator (Konzepte): pagebrowsergui_k_start.htm, pagebrowsergui_k_prinzip.htm, pagebrowsergui_k_eigenschaften.htm,
  pagebrowsergui_k_seitentypen.htm, pagebrowsergui_k_filter.htm, pagebrowsergui_k_fremddokumente.htm
- Seiten-Navigator (Handlungen): pagebrowsergui_h_eigenschaftenbearbeiten.htm, pagebrowsergui_h_seitenanlegen.htm,
  pagebrowsergui_h_seitenkopieren.htm, pagebrowsergui_h_seitenkopierenprojekte.htm, pagebrowsergui_h_seitennummerieren.htm,
  pagebrowsergui_h_seitenoeffnen.htm, pagebrowsergui_h_seitenstrukturanpassen.htm, pagebrowsergui_h_seitenvorschau.htm
- Seiten-Navigator (Dialoge): pagebrowsergui_d_manuelleseitensortierung.htm, pagebrowsergui_d_projekteinstellungen.htm,
  pagebrowsergui_d_seiteneigenschaften.htm, pagebrowsergui_d_seitenkopieren.htm, pagebrowsergui_d_seitenname.htm,
  pagebrowsergui_d_seitennummerierung.htm, pagebrowsergui_d_seitennummerierungsvorschau.htm, pagebrowsergui_d_seitenuebersicht.htm,
  pagebrowsergui_d_strukturenbearbeiten.htm, pagebrowsergui_r_ortkaestenstruktur.htm, pagebrowsergui_r_seitenstruktur.htm
- Suchen/Ersetzen: searchandreplacegui_k_start.htm, searchandreplacegui_k_prinzip.htm, searchandreplacegui_k_platzhalter.htm,
  searchandreplacegui_k_ergebnislisten.htm, searchandreplacegui_k_eintraegeergebnisliste.htm, searchandreplacegui_h_suchen.htm,
  searchandreplacegui_h_ersetzen.htm, searchandreplacegui_h_bearbeiten.htm, searchandreplacegui_h_auswahlsynchronisieren.htm,
  searchandreplacegui_d_suchen.htm, searchandreplacegui_d_ersetzen.htm, searchandreplacegui_d_suchergebnisse.htm
- Projektstruktur: projectstructure_k_start.htm, projectstructure_k_verhkopinbmubauteilen.htm,
  projectstructure_h_kennzeichenausblenden.htm, projectstructure_h_frhierarchienerstellen.htm
- Querbezug: devicetaggui_k_grafischeschaltzeichen.htm
