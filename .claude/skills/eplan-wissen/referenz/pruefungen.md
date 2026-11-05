# EPLAN: Prüfläufe, Prüfschemata, Meldungsverwaltung – und Prüfliste für QET-Nachbauten

EPLAN prüft Projekte mit konfigurierbaren Prüfläufen (offline auf Knopfdruck oder online beim Zeichnen) und sammelt die Ergebnisse in der Meldungsverwaltung. Die Meldungsnummern (Klasse + laufende Nummer) beschreiben zusammen sehr genau, was ein „sauberer“ EPLAN-Plan erfüllen muss. Diese Datei fasst die Mechanik zusammen (Abschnitte 1–6) und macht aus den Meldungsklassen eine **Prüfliste für QET-Nachbauten** (Abschnitt 7–8).

Grundlage: EPLAN-Hilfe Plattform 2.9 (`msgmanagementgui_*`, `xesinspectiongui_*`, Aktion `check`) und die Klassenübersichten `messages_o_*.htm` (am 2026-11-05 von eplan.help geladen). Die Einzelseiten `messages_p_<Nummer>.htm` (Ursache + Korrekturvorschlag je Meldung) wurden **nicht** gelesen; die Prüfliste stützt sich auf die Meldungstitel. **Ergänzung:** Inzwischen sind alle 586 Einzelseiten ausgewertet – Nachschlagetabelle je Meldung in [meldungen.md](meldungen.md), daraus abgeleitete Regeln in §8.10.

## Inhalt

1. Zweck und Funktionsumfang der Meldungsverwaltung
2. Meldungsnummer, Meldungsklasse, Kategorie
3. Prüfarten (Nein / Offline / Online / Fehler verhindern / Modulbezogen)
4. Prüfschemata und Projekteinstellungen
5. Prüflauf ausführen (Umfang, Erledigt-Logik), Online-Prüfung, Meldungstext am Symbol
6. Dialog Meldungsverwaltung (Spalten, Sprünge, Filter) und Artikelstammdaten-Prüfung
7. Übersicht aller Meldungsklassen
8. Prüfliste für einen sauberen Plan (nach Klassen, mit QET-Prüfweg; 8.9 neue Prüfläufe 2023–2026; 8.10 Schärfung aus den Einzelseiten)
9. Bedeutung für QET (Werkzeuge, Prüfskript)
10. Quellen

---

## 1. Zweck und Funktionsumfang

- Beim Projektieren entstehen gewollte oder ungewollte Inkonsistenzen, elektrotechnische und logische Fehler; diese führen zu falschen oder unvollständigen Auswertungen. EPLAN bietet dafür Logikprüfungen der Schaltplanseiten; Ergebnisse landen in der Meldungsverwaltung. Zusätzlich lassen sich die Artikelstammdaten der Artikeldatenbank prüfen.
- Funktionen: frei wählbare Prüfkriterien und Kategorisierung; Online-Prüfung jedes Arbeitsschritts oder Offline-Prüflauf zu beliebigem Zeitpunkt; Prüfung des ganzen Projekts, von Strukturteilen oder einzelnen Seiten; Artikelprüfung für im Artikelstammdaten-Navigator markierte Artikel; **modulbezogene** Prüfungen für Daten, die Module erzeugen (z. B. automatische Kabelauswahl); Abschalten einzelner Prüfungen per Prüfart „Nein“; Sprungfunktion zum Ort der Ursache (Projekt oder Artikelverwaltung); zu jeder Meldung eine Hilfeseite mit Korrekturvorschlägen.

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_k_start.htm

- Geprüft werden Betriebsmitteldaten (unvollständige Definitionen von Klemmen, Steckern, Kabeln, Geräten, SPS …; mehrfache nicht querverwiesene oder nicht mehr existierende Betriebsmittel) und funktionsbezogene Dinge wie **Kurzschlüsse, Ringschlüsse, unvollständige Querverweise oder Abbruchstellen, falsche Potenzialdefinitionen**. Prüfkriterien liegen in einem Schema, das exportiert und in anderen Projekten genutzt werden kann.

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_k_prueflaufprinzip.htm

## 2. Meldungsnummer, Meldungsklasse, Kategorie

- Jede Prüflaufmeldung hat eine **sechsstellige Nummer**: Ziffern 1–3 = Klasse (z. B. 001 Klemmen, 002 Stecker), Ziffern 4–6 = Meldung innerhalb der Klasse. Beispiel 002004 = Stecker ohne Steckerkontakte. In der Hilfe heißen die Seiten mit Präfix `P` (P002004).
- **Kategorien**: Fehler, Warnung, Hinweis (je mit Icon). Die Kategorie ist je Meldung einstellbar und wird projektbezogen im Schema gespeichert, bleibt also bei Weitergabe des Projekts erhalten.
- Meldungstexte folgen der Dialogsprache; vorhandene Meldungen erscheinen nach Sprachwechsel automatisch in der neuen Sprache (kein Neuerzeugen nötig).

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_k_prinzip.htm

- Im Filterdialog ist „ID“ die dreistellige Nummer innerhalb der Klasse.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_d_filter.htm

## 3. Prüfarten

| Prüfart | Wirkung |
|---|---|
| Nein | Prüfung läuft nicht, keine Meldung. |
| Offline | Meldungen erst nach einem Prüflauf. Sind im Schema Verbindungsprüfungen aktiv, werden **vor** dem Offline-Lauf die Verbindungen automatisch aktualisiert (Seite oder Projekt). |
| Online / offline | Meldung sofort bei Entstehung, sofern der Dialog Meldungsverwaltung offen ist. Behobene Meldungen verschwinden **nicht** von selbst, sondern erst beim nächsten Offline-Lauf; bis dahin als erledigt markieren und filtern. |
| Fehler verhindern | Eine Änderung, die das Kriterium verletzt, wird nach Datenübernahme zurückgenommen; ausgegeben werden zusätzlich Seite, BMK und Position. |
| Modulbezogen | Prüfungen, die andere Module selbst durchführen (z. B. Kabel automatisch erzeugen, SPS adressieren). Fest vorgegeben; nur auf „Nein“ abschaltbar. |

Nicht jede Prüfart steht für jede Prüfung zur Verfügung. Nach der Installation liefert EPLAN einige vorkonfigurierte Schemata mit.

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_k_prueflaufprinzip.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_einstellungenmeldungen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_konfigurieren.htm

## 4. Prüfschemata und Projekteinstellungen

- Konfiguration: *Projektdaten > Meldungen > Prüflauf ausführen* → [...] neben „Einstellungen“ → Dialog **Einstellungen: Meldungen und Prüfläufe**. Links Liste der Meldungsklassen (ID/Klasse), rechts Tabelle Nummer / Meldungstext / **Kategorie** / **Prüfart**; zusätzlich je Meldung ein **Filter** auf Gewerke (vorgegebene oder eigene Filterschemata). Schemata über die Schema-Symbolleiste neu anlegen, speichern, kopieren, löschen, importieren, exportieren.
- Werte lassen sich für viele Meldungen auf einmal setzen: alle Klassen markieren, eine Prüfart in die Zwischenablage kopieren, Spalte markieren, [Strg]+[V].

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_konfigurieren.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_einstellungenmeldungen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_artikelstammdatenpruefen.htm

- Projekteinstellung *Optionen > Einstellungen > Projekte > "Projekt" > Verwaltung > Meldungsverwaltung*: **Prüflaufabhängige Einstellungen** (welches Schema für die Anzeige/Online-Prüfung gilt) und **Max. Anzahl Prüflaufmeldungen** (Pflichtfeld, 1–10 000; intern hart auf 10 000 begrenzt).

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_projekteinstellungen.htm

## 5. Prüflauf ausführen

**Offline (Projekte prüfen).** Projekte können nur offline geprüft werden. Im Seiten-Navigator Projekt, Strukturknoten (z. B. Anlage, Einbauort) oder Seiten markieren → *Projektdaten > Meldungen > Prüflauf ausführen* → Schema wählen → [OK]. Wird das **ganze Projekt** geprüft, kommen zusätzlich **nicht platzierte** Funktionen, Abbruchstellen, Geräte, Artikelreferenzen und Vorplanungsobjekte dazu. Markierter Strukturknoten = alle Seiten darunter; einzelne Seite = nur diese. Die Ergebnisse ersetzen die des vorigen Laufs in der Meldungsdatenbank. Prüfungen, die nicht laufen können, landen im Systemmeldungsdialog.

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_prueflauf.htm

**Dialog Prüflauf ausführen.** „Anwenden auf gesamtes Projekt“ erweitert eine Seiten-/Strukturauswahl auf das Projekt (bei markiertem Projekt fest an). „Nur erledigte Meldungen prüfen“ prüft nur Meldungen mit Häkchen *Erledigt*; alle anderen bleiben unverändert, Schema und Auswahl werden ignoriert. Aufruf auch über Kontextmenü *Wiederholen: Prüflauf* im Grafischen Editor.

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_prueflauf.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_prueflauf.htm

**Online.** Mit Schaltplanseite im Editor und Schema mit Prüfart „Online“: Jeder Bearbeitungsschritt (Kopieren, Verschieben, Eigenschaften ändern) wird geprüft, Meldungen erscheinen sofort.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_prueflaufonline.htm

**Meldungstext direkt am Symbol.** Ebene **EPLAN520** („Eigenschaftsplatzierung.Meldungstext“) sichtbar schalten, am Schaltzeichen auf der Registerkarte *Anzeige* die Eigenschaft „Meldungstext“ hinzufügen (Eigenschaftsanordnung wird „Benutzerdefiniert“; als Standard für gleiche Symbolvariante speicherbar). Voraussetzung Prüfart „Online / offline“. Nach Korrektur bleibt der Text stehen, bis ein Offline-Lauf ihn entfernt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_meldungstexteanzeigen.htm

**Per Aktion/Skript/Kommandozeile:** Aktion `check` mit `/TYPE:PROJECT|PAGE|INSTALLATIONSPACE`, `/VERIFICATIONSCHEME:<Schemaname>` (ohne Pfad; Vorgabe zuletzt benutztes), `/PAGENAME:` oder `/SELn:`, `/USEPAGEFILTER:1`, `/INSTALLATIONSPACENAME:` (ID 36452). Details in [api-aktionen.md](api-aktionen.md).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_check.htm

### Bedeutung für QET
- EPLAN-Prüfläufe sind **Momentaufnahmen**: Ein Original-PDF, das ohne Prüflauf ausgegeben wurde, kann Fehler enthalten. Beim Nachbau den QET-Plan prüfen, nicht das PDF als fehlerfrei annehmen (passt zu „Inhalt vor Optik“).
- Ein QET-Prüfskript sollte wie EPLAN zwei Umfänge kennen: einzelne Folios (beim Nachzeichnen je Blatt) und Gesamtprojekt (zusätzlich „nicht platzierte“ Dinge, in QET z. B. Folio-Reports ohne Gegenstück, verwaiste Querverweise).
- Prüfart „Online“ entspricht in QET am ehesten einem Prüfaufruf nach jedem `gesamt.py`/`verknuepfen.py`-Lauf.

## 6. Dialog Meldungsverwaltung und Artikelprüfung

- Tabelle mit Meldungsnummer und -text; zuschaltbar (*Darstellung konfigurieren*): Status (Icon), Kategorie, **Seite**, Bauraum, **BMK**, Artikelnummer, Artikelvariante, **Quelle** und **Ziel** (BMK der angeschlossenen Funktionen), **Erledigt**, Erzeugt von (EPLAN oder **eigene Meldungen über die API**), X/Y (gerundet), Erzeugt durch Prüfart (offline/online). Sortieren per Doppelklick auf Spaltenkopf.
- Kontextmenü: Anzeige löschen (danach nur neu verursachte Meldungen bis zur nächsten projektweiten Aktualisierung), Prüflauf ausführen, Nur erledigte prüfen, **Gehe zu (querverwiesene)**, **Gehe zu (alle Darstellungsarten)**, **Gehe zu (Grafik)** (Doppelklick; springt auf erstes platziertes Objekt), Gehe zu 2. Koordinate (Einfügepunkt), Auswahl synchronisieren (Navigatoren), Tabellarisch bearbeiten, Eigenschaften / Eigenschaften (übergreifend).
- Checkbox **Auswahl**: Meldungen nur zur im Navigator/Editor gewählten Funktion bzw. Seite; funktioniert nur bis 1000 Meldungen in der Datenbank. Fehlerhafte Funktionen tragen nach dem Lauf ein **Ausrufezeichen** in den Navigatoren. [F1] auf einer Meldung öffnet deren Hilfe.
- Grafische Vorschau zeigt die betroffene Seite mit hervorgehobener Funktion.

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_d_meldungsverwaltung.htm

- Filter (*Filter: Meldungen*): Auswahl der Klassen, Gewerke (Elektrotechnik, Verfahrenstechnik, Fluid, Mechanik; „Allgemein“ = ohne Gewerk, z. B. Übersetzung, Artikelverwaltung), Kategorien (überlagert Projekteinstellung), „Erledigte Meldungen“ ein-/ausblenden. Filter bleibt gespeichert; Häkchen *Aktiv* schaltet ihn ein. Arbeitsablauf: Meldung → Gehe zu (Grafik) → Eigenschaften ändern → neuer Prüflauf; erledigte Meldungen entfernt man mit „Nur erledigte Meldungen prüfen“.

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_d_filter.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_meldungenansehen.htm

- **Artikelstammdaten prüfen**: eigenes „Prüfprojekt“ (darf seitenlos sein, muss als einziges Projekt offen sein); in der Meldungsverwaltung Spalten Artikelnummer/-variante einblenden; Artikeldatenbank wählen; Schema anlegen, in dem alle Klassen auf „Nein“ stehen und nur Klasse **501 Artikelstammdaten** aktiv ist; im Artikelstammdaten-Navigator den Zweig (ggf. mit Filter) markieren; Prüflauf. Fehlerhafte Artikel bekommen ein rotes Ausrufezeichen; über die Eigenschaft „Meldungen“ lässt sich danach filtern.

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_artikelstammdatenpruefen.htm

- Neuerung 2.9: P501005 (benutzerdefinierte Ebene in Artikelmakro) erkennt solche Ebenen jetzt zuverlässiger → deutlich mehr Meldungen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_prueflaufmeldungen_benutzerdefebene_230943.htm

### Bedeutung für QET
- Ein QET-Prüfbericht sollte dieselben Spalten liefern: Meldungsnummer (gern EPLAN-Nummer als Referenz), Kategorie, Folio, BMK, Quelle/Ziel, Position – damit man wie in EPLAN „Gehe zu“ machen kann (`qet_live_show_folio`).
- Für den Bauteil-Import aus EPLAN-Makros/EDZ ist Klasse 501 die Vorlage für eine **Importprüfung**: Makro vorhanden, Funktionsschablonen passen zum Makro, Bestell-/Typnummer nicht leer, Artikelnummer ohne Leerzeichen am Rand, SPS-Typbezeichnung gefüllt, Anschlussbezeichnungen der Schablonen = Anschlussbild.

## 7. Übersicht aller Meldungsklassen

| Klasse | Bereich | Anzahl Meldungen (2.9) | Relevanz QET |
|---|---|---|---|
| 001 | Klemmen | 25 | hoch |
| 002 | Stecker | 14 | hoch |
| 003 | Kabel | 21 | hoch |
| 004 | SPS / Bus | 78 | hoch |
| 005 | Verbindungen | 78 | hoch |
| 007 | Geräte | 33 | hoch |
| 008 | Fremdsprachen | 5 | gering |
| 010 | Querverweise | 2 | mittel |
| 011 | Abbruchstellen | 8 | hoch |
| 012 | 2D-Schaltschrankaufbau | 9 | gering |
| 013 | Artikeldaten (Import/Stückliste) | 9 | mittel |
| 014 | Makro | 6 | mittel |
| 015 | Auswertungen | 4 | mittel |
| 016 | Gerätekästen | 2 | mittel |
| 017 | BMK | 13 | hoch |
| 020 | Projektvergleich | 37 | gering |
| 021 | Platzhalterobjekte | 5 | gering |
| 022 | Sonstiges (Symbole, Seiten, Texte) | 20 | mittel |
| 024 | Fluid | 18 | – |
| 025 | Projekteinstellungen | 1 | gering |
| 026 | 3D-Montageaufbau | 69 | – |
| 027 | Topologie | 23 | – |
| 028 / 029 | Vorplanung / Vorplanungstiefe | 31 / 30 | – |
| 030 | Teilprojekte | 8 | – |
| 032 | Bohrbild | 2 | – |
| 033 | Kabelbaum | 3 | – |
| 501 | Artikelstammdaten | 36 | mittel (Import) |

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_start.htm (Anzahl aus den jeweiligen Klassenseiten `messages_o_<Klasse>….htm`, siehe Quellen)

## 8. Prüfliste für einen sauberen Plan

Abgeleitet aus den Meldungstiteln (nicht aus den Detailseiten). Ursache/Korrektur jeder einzelnen Meldung: [meldungen.md](meldungen.md); Bedingungen und Ausnahmen aus den Detailseiten: §8.10. Spalte „QET-Prüfweg“: **check** = `qet_check`, **cont** = `qet_continuity`, **Skript** = eigenes Prüfskript nötig (XML auswerten), **Hand** = Sichtprüfung. Nummern in Klammern = EPLAN-Meldung.

### 8.1 BMK und Betriebsmittel (017, 007, 016)
| Regel | EPLAN | QET-Prüfweg |
|---|---|---|
| Jedes Betriebsmittel hat ein BMK, ist nummeriert und hat einen zulässigen Kennbuchstaben | 017001–017003 | check (unlabelled_masters), Skript für Kennbuchstaben |
| BMK eindeutig: nicht mehrere Hauptfunktionen je BMK; keine zweite Spule zum gleichen BMK | 017005, 017006 | check (duplicate_master_labels) |
| Gleiche BMK sind querverwiesen (Haupt/Neben verknüpft) | 017007 | Skript (Master/Slave-Links in QET) |
| BMK folgt dem eingestellten Nummerierungsformat | 017008 | Skript |
| Sichtbares BMK: keine überflüssigen Strukturkennzeichen, Vorzeichen (`=`, `+`, `-`) vorhanden | 017011, 017012 | Skript/Hand |
| Überlappende Strukturkästen widersprechen sich nicht; Gerät nicht teils mit, teils ohne Struktur | 017010, 017013 | Hand |
| Jedes Betriebsmittel hat eine Hauptfunktion; Hauptfunktion hat Artikel | 007005, 007017 | check (masters_without_manufacturer_reference) |
| Zu jeder Spule gibt es passende Kontakte; Funktionsanzahl passt zum Artikel (nicht mehr, nicht weniger Funktionen als Artikel bietet) | 007009, 007001, 007002 | Skript (Kontaktanzahl vs. Artikel/Kontaktspiegel) |
| Jede Funktion hat Anschlussbezeichnungen, keine doppelt am Gerät; Symbol hat nicht mehr Anschlüsse als die Funktionsdefinition | 007006, 007004, 007022 | Skript (Terminal-Namen je Element) |
| Funktionen eines Betriebsmittels haben keine widersprüchlichen Daten; „Funktionstext (automatisch)“ gleich in allen Darstellungsarten | 007010, 007014 | Skript |
| Artikel ist im Projekt eingelagert und nicht als „nicht mehr verwenden“ markiert; Pflicht-Zubehör vorhanden | 007012, 007013, 007024, 007018 | Hand |
| Montageort (beschreibend) existiert | 007007 | Hand |
| Schaltzeichen nicht in zwei nicht geschachtelten Gerätekästen; Gerätekasten hat Anschlüsse | 016001, 016002 | Hand |

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_017DT.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_007Devices.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_016Blackboxes.htm

### 8.2 Verbindungen und Potenziale (005)
| Regel | EPLAN | QET-Prüfweg |
|---|---|---|
| Potenzialanschlüsse/-definitionspunkte und Verbindungen tragen ein Potenzial; nur eine Potenzialdefinition je Potenzial; Definition passt zum Potenzial | 005001, 005010, 005028, 005021, 005020 | cont (potential_mismatch) + Skript |
| Kein Ringschluss; kein Widerspruch Signal/Potenzial | 005027, 005019 | Skript |
| Keine offenen Anschlüsse (allgemein, an Klemmen, an Steckerkontakten, an Verbindungssymbolen) | 005026, 005053, 005052, 005051 | cont (unconnected_terminal) |
| Nicht zu viele Verbindungen an einem Anschluss | 005025 | check (crowded_terminals) |
| Verbindungsdefinitionspunkt: genau auf einer Verbindung, mit Eigenschaften, nur einer je Verbindung, kein Widerspruch zwischen mehreren | 005012, 005018, 005022–005024 | Skript (QET: Leiterbeschriftung je Leiter) |
| Ziele einer Verbindung haben gleiche Anschlussart, Potenzialtyp, Darstellungsart, Querschnitt | 005013–005016, 005039, 005073, 005074 | cont (potential_mismatch) teilweise |
| Gleiche Verbindungsbezeichnung nicht auf verschiedenen Potenzialen/Signalen/Netzen; innerhalb eines Potenzials/Signals/Netzes keine unterschiedlichen Bezeichnungen | 005044, 005045, 005069–005072 | cont (potential_mismatch) |
| Farbe/Nummer nur aus den Projekt-„Verbindungsfarben“, Querschnitt aus Projektliste; Kabelverbindung hat Farbe/Nummer | 005042, 005043, 005082 | Skript |
| Verbindungsquerschnitt im zulässigen Bereich des Klemmenartikels | 005093 | Hand |
| Einpolig/allpolig konsistent (keine doppelte allpolige Verbindung, Definition passt zu Quelle/Ziel) | 005060–005063 | – (QET kennt keine allpolige Darstellung) |
| Kein Ziel fehlt, keine ungültige Verbindung, keine zum Löschen markierte Verbindung | 005017, 005064, 005085 | cont |

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_005Connections.htm

### 8.3 Klemmen (001) und Stecker (002)
| Regel | EPLAN | QET-Prüfweg |
|---|---|---|
| Klemme hat Bezeichnung und Ziele; gleichbezeichnete Klemme nicht doppelt in einer Leiste | 001001, 001016, 001005 | Skript (Terminal-Elemente je Leiste) |
| Klemmenleistendefinition vorhanden, enthält Klemmen und alle Klemmen der Leiste | 001010, 001011, 001021 | Skript (QET-Klemmenleiste/terminal_strip) |
| Nicht zu viele Ziele je Klemmenanschluss | 001008 | check (crowded_terminals) |
| Brücken korrekt (keine übereinanderliegenden Stegbrücken, nicht zu viele Drahtbrückenebenen, keine Anschlussbezeichnung am Brückenanschluss) | 001012, 001015, 001022, 001025 | Hand |
| Intern/extern nicht vertauscht verbunden | 001024 | Hand (Seite der Klemme) |
| Teilklemmen: gleich bezeichnete sind verbunden, keine einzelne Teilklemme, gleiche Funktionsdefinition | 001026–001028, 001030, 001035 | Skript |
| Genau eine Hauptklemme, der ersten Funktionsschablone zugeordnet | 001023, 001037 | Skript |
| Stecker hat Definition und Kontakte, Kontakte haben Ziele und sind eindeutig bezeichnet | 002002–002004, 002014 | Skript |
| Stift- **und** Buchsenseite vorhanden, gleiche Anzahl; nie Stift auf Stift oder Buchse auf Buchse | 002005, 002006, 002008, 002012 | Skript/Hand |
| Keine mehreren unzusammenhängenden Steckerdefinitionen; Formularposition nicht doppelt | 002007, 002009 | Hand |

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_001Terminals.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_002Plugs.htm

### 8.4 Kabel (003)
| Regel | EPLAN | QET-Prüfweg |
|---|---|---|
| Jede Kabelverbindung gehört zu einem Kabel; Verbindungs-BMK = Kabel-BMK | 003008, 003003 | Skript (QET-Leiter-Eigenschaft cable) |
| Kabelziele haben vollständiges BMK; kein Kabel zwischen demselben Bauteil; nicht mehrere Kabelziele | 003005–003007 | Skript |
| Nicht mehr Adern belegt als das Kabel hat; Farbe/Nummer nicht doppelt (bei gleichem Schirm/Paar); PE- und N-Ader nur einmal; Schirm nicht zu oft aufgelegt | 003011–003014, 003018 | Skript (Adernliste je Kabel) |
| Kabeldaten an allen Darstellungen gleich; Länge und Verbindungsanzahl angegeben und gültig | 003009, 003022, 003025, 003026 | Skript |
| Kabel nutzt alle Adern des Artikels (Hinweis auf Reserven) | 003028 | Skript |

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_003Cables.htm

### 8.5 SPS / Bus (004)
| Regel | EPLAN | QET-Prüfweg |
|---|---|---|
| Am Ausgang ein Aktor, am Eingang ein Sensor; höchstens ein E/A-Anschluss je Kanal | 004018–004020, 004097 | Skript (Ziel der SPS-Klemme) |
| Adresse vorhanden, innerhalb einer CPU eindeutig, im Adressbereich der Karte; Adressbereiche zweier Karten überschneiden sich nicht | 004021, 004023, 004106, 004107 | Skript |
| Anschlussbeschreibung vorhanden und je Kanal eindeutig; Kanalbezeichnung korrekt | 004022, 004024, 004027 | Skript |
| Symbolische Adresse vorhanden und je CPU eindeutig; „Funktionstext (automatisch)“ je CPU eindeutig | 004030, 004031, 004073 | Skript |
| E/A und zugehöriger Versorgungsanschluss im Kanal widerspruchsfrei | 004028 | Hand |
| SPS-Anschlüsse und Bus-Anschlüsse erscheinen auch auf der Übersicht; SPS-Kasten hat Anschlüsse | 004004–004006 | Skript (Übersicht ↔ Schaltplan, vgl. sps.md) |
| Steckplatz vorhanden und je Baugruppenträger eindeutig; Kartenname eindeutig; CPU-Name eindeutig | 004009, 004025, 004088, 004089, 004094 | Skript |
| Bus: Bus-Adresse vorhanden und je Netz eindeutig; keine Verbindung zwischen verschiedenen Bus-Systemen; physikalisches Netz benannt | 004035, 004037, 004039, 004072, 004110 | Hand |
| Safety-Adresse vorhanden und eindeutig | 004095, 004096 | Skript |
| SPS-Typbezeichnung bzw. Gerätebeschreibungsdatei gefüllt (Voraussetzung für Datenaustausch) | 004086 | Hand |

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_004PLCbus.htm

### 8.6 Querverweise, Abbruchstellen (010, 011)
| Regel | EPLAN | QET-Prüfweg |
|---|---|---|
| Abbruchstelle hat BMK (Potenzial-/Signalname), Gegenstück und Ziel | 011001–011003 | check (reports_with_several_wires) + Skript (Folio-Report ohne Partner) |
| Keine Verbindung über Abbruchstellen auf **derselben** Seite | 011004 | Skript |
| Pfeilrichtungen querverwiesener Abbruchstellen passen zusammen (Quelle → Ziel) | 011005 | Skript (QET: Report-Typ in/out) |
| Kein Ringschluss über Abbruchstellen; keine Verzweigung vor einer Sternquelle; nur eine Sternquelle je BMK | 011006–011008 | Skript |
| Platzierung liegt im Auswertungsbereich des Normblatts (sonst kein Querverweis/keine Spaltenangabe) | 010001 | Skript (Element innerhalb Rahmen-Spalten) |
| Je Strukturkastenname nur eine Hauptplatzierung | 010002 | Hand |

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_011Interruptionpoints.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_010Crossreferences.htm

### 8.7 Seiten, Symbole, Texte, Auswertungen, Makros (022, 025, 015, 014)
| Regel | EPLAN | QET-Prüfweg |
|---|---|---|
| Seitennummer nicht doppelt; Seitennamen ggf. ohne Buchstaben (wenn so eingestellt) | 022003, 022016 | Skript (Folio-Titel/-Index) |
| Keine übereinanderliegenden identischen Symbole oder gleichgerichteten Anschlüsse | 022004 | qet_layout_check (Überlappung) |
| Symbol aus vorhandener Bibliothek mit gültiger Funktionsdefinition | 022001, 022002, 022019 | QET: fehlende Elemente beim Öffnen |
| Texte: Winkel nur 0°/90°, nicht abgeschnitten, nicht zu klein | 022025–022027 | Skript/Hand |
| Seitenraster entspricht Projekteinstellung | 025001 | qet_layout_check (Raster) |
| Auswertung hat bekannten Typ; verwendete Unterformulare sind eingelagert; Brücken ausgebbar | 015001, 015003, 015004 | Hand |
| Makroname eindeutig und vorhanden; Version am Makrokasten passt | 014001, 014003, 014004 | Import-Skript |

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_022Other.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_025Projectsettings.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_015Reports.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_014Macro.htm

### 8.8 Artikel und Artikelstammdaten (013, 501) – für Bauteil-Import
| Regel | EPLAN | Bedeutung für Import |
|---|---|---|
| Höchstens 50 Artikel je Betriebsmittel | 013051 | Fork-Import begrenzen |
| Makro, Bilddatei, Dokument, Anschlussbild, Bohrbild am Artikel existieren | 501001–501003, 501008, 501009 | Dateiverweise beim EDZ-Import prüfen |
| Typnummer, Bestellnummer, Bezeichnung 1, Produktgruppen, Gewerk gefüllt | 501011–501017 | Pflichtfelder der QET-Elementinfos |
| Artikelnummer ohne Leerzeichen am Anfang/Ende | 501024 | beim Import trimmen |
| Abmessungen am Artikel passen zum Makro (Toleranz) | 501007, 501018 | nur Schaltschrankaufbau |
| Funktionsschablonen aktuell, Anschlussmaße und identifizierende Anschlussdaten = Anschlussbild; Kanalbezeichnung korrekt | 501022, 501023, 501032, 501033 | Abgleich Schablone ↔ Makroanschlüsse |
| SPS-Artikel: SPS-Typbezeichnung oder Gerätebeschreibungsdatei gefüllt; Bus-Anschlüsse im einpoligen Makro vorhanden, mit Steckerbezeichnung | 501025, 501026, 501028 | SPS-Kartenimport |
| Makrobestandteile gruppiert; keine benutzerdefinierten Ebenen im Makro | 501006, 501005 | Makroleser |
| Allgemeine Funktionsdefinition nicht am Artikel verwenden | 501031 | Kategorie-Zuordnung |

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_501Partmasterdata.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_013Partsdata.htm

### 8.9 Neue Prüfläufe 2023–2026 (aus den Neuerungen)
| Regel | EPLAN | QET-Prüfweg |
|---|---|---|
| Anschlussbezeichnungen der platzierten Funktion = Funktionsschablone/Artikel | 007034 (2024) | Skript (Terminal-Namen ↔ Elementvorlage) |
| Objekt mit Eigenschaftsanordnung „Benutzerdefiniert“ finden | 022062 (2024) | – |
| Logisches Netz über unterschiedliche Bus-Systeme verbunden | 004122 (2024) | Hand |
| Eingelagerter Artikel fehlt in der Artikeldatenbank | 013056 (2026) | Import-Skript |
| Stegbrücke an Klemme ohne Stegbrückenanschluss; 001012 auch bei Stegbrücke über verschiedene Etagen | 001039, 001012 (2026) | Skript |
| Ader eines vorkonfektionierten Kabels am falschen Steckerkontakt | 003029, 005094 (2026) | Hand |
| „Wert mit Einheit“: Einheit fehlt / falsche Einheit | 022063, 022064 (2026) | – |
| Artikelstammdaten-Prüfung ab 2026 in eigener Klasse **502** direkt in der Artikelverwaltung (ohne Projekt) | 502xxx | – |

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2024/Content/htm/news_p_plattform_notes.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/news_p_plattform_notes.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/news_p_artikelverwaltung_artikelprueflaeufe_489829.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/news_p_eigenschaften_einheitengruppen_471994.htm

- Ab 2025 zeigt die Meldungsverwaltung bei Verbindungen in der BMK-Spalte Quelle ↔ Ziel (z. B. `+(+A1-XD1:2 ↔ =GAA+A1-FC1:3)`) und „Meldungsquelle existiert nicht mehr“, wenn das Objekt gelöscht wurde; ab 2024 Prüfdauer je Klasse unter *Datei > Informationen > Status*. Gutes Vorbild für das Format eines QET-Prüfberichts. Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/news_p_meldungsverwaltung_verbesserungenverlegen_441283.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2024/Content/htm/news_p_plattform_notes.htm

### 8.10 Schärfung aus den Einzelseiten (Ursache/Lösung je Meldung)

Die 586 Einzelseiten `messages_p_<Nr>.htm` sind jetzt ausgewertet; die vollständige Nachschlagetabelle (Nr. | Kurztitel | Ursache | Korrektur | QET-Prüfweg, **Regel**-Kennzeichnung) steht in [meldungen.md](meldungen.md). Hier nur die fachlichen Regeln, die über die Meldungstitel in 8.1–8.9 hinausgehen (Bedingungen, Ausnahmen, steuernde Einstellungen). **Regel** = direkt umzusetzen im QET-Nachbau bzw. in `verknuepfen.py`.

#### 8.10.1 Hauptfunktion, BMK, Spule/Kontakte (007, 017)
- **Regel:** Je BMK genau eine Hauptfunktion; alle weiteren gleichnamigen Funktionen sind Nebenfunktionen. Auch eine allein platzierte Nebenfunktion ohne Hauptfunktion ist ein Fehler; zwei Spulen mit gleichem BMK immer. (P007005, P017005, P017006)
- **Regel:** Spule ohne Kontakt wird auch gemeldet, wenn ein Kontakt-/Spulenindex nur an der Spule steht – der Index muss an Spule und Kontakten gleich oder überall leer sein. (P007009)
- **Regel:** BMK werden nicht als Text verglichen, sondern zerlegt (Vorziffer, Kennbuchstabe, Zähler, Unterzähler). Ein Zähler im Kennbuchstabenfeld („X1“ als Kennbuchstabe) ergibt ein anderes BMK und keinen Querverweis; ein nicht-numerischer Rest nach dem Kennbuchstaben (z. B. `-FA`) zählt zum Kennbuchstaben, der Zähler fehlt dann. `verknuepfen.py` muss BMK normalisiert vergleichen. (P017007, P017008, P011002)
- BMK mit „?“ gilt als nicht nummeriert. Das Produkt-BMK beginnt mit `-`; Strukturkennzeichen, die schon die Seite liefert, sollen am Symbol nicht wiederholt werden (im vollständigen BMK entfernt EPLAN Doppelungen, am Symbol bleiben sie sichtbar). (P017002, P017011, P017012)
- Klemmenleisten, Stecker, Kabel und Abbruchstellen tragen dieselbe Kennzeichnungsstruktur wie andere Betriebsmittel. (P017013)
- **Regel:** Anschlussbezeichnungen sind innerhalb eines Betriebsmittels über alle Funktionen eindeutig (nicht zweimal 13/14); bei Bus-Anschlüssen zählt Anschluss + Stecker + Schnittstellenname. Unbezeichnete Anschlüsse nur bei Anschlussart Intern, Direktanschluss, Stegbrücke. (P007004, P007006, P026053)
- Haupt- und Nebenfunktionen dürfen sich in Eigenschaften nicht widersprechen; die Artikelnummer gehört an Hauptfunktion bzw. Hauptklemme. Bei Artikeln mit Funktionsschablonen: zusätzliche Funktion mit gleichem BMK = Fehler, fehlende nur je nach „Anzahl freie Funktionen“. (P007010, P007017, P007001, P007002)
- Logikweiterleitung („Potenzial weiterleiten nach“, „Zielverfolgung (SPS) nach“) darf nur auf existierende Anschlüsse desselben BMK zeigen. (P007027)
- Jedes Gerät im Aufbau/in der Stückliste braucht eine Hauptfunktion im Schaltplan mit gleichem BMK (Ausnahme Gewerk Mechanik). Ventilspulen und Fluid-Analogsensoren erscheinen im Fluidplan nur als Paarquerverweis, der Master bleibt im Stromlaufplan. (P012010, P026016, P024003)

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_007001.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_007002.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_007004.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_007005.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_007006.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_007009.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_007010.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_007017.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_007027.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_011002.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_012010.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_017002.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_017005.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_017006.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_017007.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_017008.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_017011.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_017012.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_017013.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_024003.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_026016.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_026053.htm

#### 8.10.2 Abbruchstellen und Querverweise (011, 010)
- **Regel:** Abbruchstellen bilden nur ein Paar, wenn das BMK identisch **und** gleich strukturiert ist (siehe BMK-Zerlegung); ungepaarte melden. Jede Abbruchstelle braucht mindestens einen Leiter. (P011001, P011002, P011003)
- **Regel:** Gepaarte Abbruchstellen liegen auf verschiedenen Seiten; zwei gleiche auf derselben Seite sind ein Fehler. (P011004)
- **Regel:** Richtung: Pfeilspitze der einen zeigt auf den Pfeilstumpf der anderen; zwei Spitzen gegeneinander sind falsch. Ein Leiter, der von der Gegenstelle direkt zurück zum Stumpf führt, ist ein Ringschluss. In QET: Folio-Report „out“ ↔ „in“ paaren, nie out↔out. (P011005, P011006)
- **Regel:** Sternquelle: je BMK höchstens eine; vor ihrem Pfeilstumpf keine Verzweigung, der Stumpf hat genau ein Ziel. (P011007, P011008)
- Topologie- und normale Abbruchstellen dürfen kein gemeinsames BMK haben. (P027013)
- **Regel:** Elemente außerhalb des Auswertungsbereichs des Normblatts bekommen keine Spaltenadresse und damit keinen gültigen Querverweis. (P010001)

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_010001.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_011001.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_011002.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_011003.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_011004.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_011005.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_011006.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_011007.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_011008.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_027013.htm

#### 8.10.3 Potenziale, Signale, Leiternummern, Verbindungen (005)
- Potenzial = galvanisch Zusammenhängendes über Leitungen, Verbindungssymbole und Funktionen mit „Potenzial weiterleiten nach“ (z. B. Klemme 1↔2). **Regel für QET:** Klemmen leiten das Potenzial durch, Sicherungen, Kontakte und Spulen nicht. (P005044)
- Signal = Teilstück eines Potenzials; Funktionen mit „Signaltrennend“ (z. B. Sicherungsklemme) teilen es. Gleiche Leiternummer beiderseits einer signaltrennenden Funktion ist ein Fehler. (P005045, P005071)
- **Regel:** Leiternummer ↔ Potenzial (bzw. Signal/Netz) eindeutig in beide Richtungen: eine Nummer nur in einem Potenzial, bei potenzialorientierter Nummerierung je Potenzial nur eine Nummer. Typischer Auslöser: gelöschter Winkel/Abzweig trennt ein Stück ab, die alte Nummer bleibt stehen – nach Löschen von Verbindungssymbolen Nummern neu prüfen. (P005044, P005069, P005070, P005072)
- **Regel:** Je Potenzial genau eine Potenzialdefinition (Einspeisung oder Definitionspunkt); jeder verwendete Potenzialname muss irgendwo definiert sein; eine Definition ohne Namen ist ein Fehler. Gleichnamige Definitionen stimmen in Typ, Wert und Frequenz überein. (P005001, P005021, P005028, P005057, P005058, P005019, P005020, P005056)
- Potenzialtyp (L, N, PE …) und Anschlussart an allen Zielen einer Verbindung gleich; „Undefiniert“ passt immer. P005014 kommt nicht bei Brücken jeder Art, Direktverbindung oder „Undefiniert“. **Regel:** PE nie mit L/N verbinden. (P005013, P005014, P005015)
- **Regel:** Je Verbindung (Anschluss–Anschluss) höchstens ein Verbindungsdefinitionspunkt, auf einem eindeutigen Leitungsstück (nicht auf T-Stück, Kreuzung, Strang oder daneben), nicht ohne Eigenschaften. In QET: genau eine Beschriftung je Leiter. (P005012, P005018, P005022, P005023, P005024, P005089)
- **Regel:** Offene Anschlüsse werden getrennt für Schaltzeichen, Verbindungssymbole, Steckerkontakte und Klemmen gemeldet; bei Klemme und Steckerkontakt müssen **alle** Symbolanschlüsse verbunden sein. (P005026, P005051, P005052, P005053)
- Ringschluss: Leitung, die über T-Stück zum Ausgangsanschluss zurückkehrt, ohne ein zweites Ziel zu erreichen. In QET: Zyklen im Leitergraphen ohne fremden Anschluss suchen. Stranganschlüsse treten immer paarweise mit gleicher Bezeichnung auf. (P005027, P005040)
- Farben und Querschnitte aus Projekt-Vorgabelisten; Querschnitt im Klemmbereich des Klemmenartikels (nur geprüft, wenn beide Seiten Werte haben; mm² = qmm; nicht bei Stegbrücke, Direktverbindung, Intern, Funk). (P005042, P005043, P005093)
- Jede Kabelader und Paketverbindung braucht Farbe oder Nummer; eine „Einzelverbindung“ trägt kein BMK (BMK heißt Kabel/Paket). (P005082, P005083, P005088)
- Einpolige und allpolige Darstellung derselben Verbindung stimmen in allen Eigenschaften überein; Übersichtsanschlüsse nie direkt mit Stromlaufanschlüssen verdrahten. (P005060, P005061, P005063, P005065, P005039)
- Teilklemmen im Netz: mindestens ein Teilklemmenanschluss muss verbunden sein (P005046). Kopierte Netzteilnehmer nehmen die interne Netz-ID mit – nach Kopieren Kette prüfen (in QET analog: kopierte Leiter behalten alte Nummern). (P005050)
- Nummerierungs-Infomeldungen (Bereich, Mehrfachverbindung, Mindeststellen) sind kein Planfehler. (P005033, P005034, P005036)

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005001.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005012.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005013.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005014.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005015.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005018.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005019.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005020.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005021.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005022.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005023.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005024.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005026.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005027.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005028.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005033.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005034.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005036.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005039.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005040.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005042.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005043.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005044.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005045.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005046.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005050.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005051.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005052.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005053.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005056.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005057.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005058.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005060.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005061.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005063.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005065.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005069.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005070.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005071.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005072.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005082.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005083.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005088.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005089.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005093.htm

#### 8.10.4 Klemmen und Stecker (001, 002)
- Klemme braucht Bezeichnung und eine Klemmenleistendefinition mit gleichem BMK; gleiches BMK + Bezeichnung nur mit „Gleiche Bezeichnung erlaubt“ (verteilte Darstellung). (P001001, P001010, P001005)
- „Anzahl Ziele“ der Anschlusslogik begrenzt die Leiter je Klemmen- bzw. Steckeranschluss. (P001008, P002001)
- Stegbrücken nicht zwischen Etagen einer Mehrstockklemme (dort Drahtbrücke); Anschlüsse brauchen „Anzahl Stegbrücken“ ≥ 1; Stegbrücken an einfachen Klemmen dürfen sich nicht überlappen; Stegbrückenanschlüsse mit Brücke tragen keine Bezeichnung. (P001012, P001022, P002010, P001025)
- **Regel:** Verbundene Klemmenanschlüsse liegen auf derselben Seite (intern/extern). (P001024)
- Teilklemmen gleicher Bezeichnung gehören zusammen („Zusammengehörige Teilklemmen automatisch verbinden“); eine einzelne Teilklemme ist verdächtig; belegte Teilklemmenanschlüsse brauchen Bezeichnungen. Bei mehreren Darstellungsarten nur eine Hauptklemme. (P001026–P001028, P001030, P001035, P001037)
- **Regel:** Stecker „für Stifte“ verlangt Gegenstück „für Buchsen“ mit gleichem BMK (und umgekehrt), Paar = gleiches BMK + gleiche Kontaktbezeichnung (oder Direktanschluss), gleiche Kontaktzahl. Stift und zugehörige Buchse dürfen gleich heißen, gleichartige Kontakte einer Seite nicht. (P002005–P002008, P002013, P002014)

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001001.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001005.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001008.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001010.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001012.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001022.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001024.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001025.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001026.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001027.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001028.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001030.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001035.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001037.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_002001.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_002005.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_002006.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_002007.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_002008.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_002010.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_002013.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_002014.htm

#### 8.10.5 Kabel (003, 027)
- **Regel:** Aderpunkte erben das Kabel-BMK; ein abweichendes BMK am Aderpunkt ist ein Widerspruch; der Punkt liegt auf der Kabeldefinitionslinie. Quelle und Ziel eines Kabels haben vollständige, verschiedene BMK. (P003003, P003008, P003005, P003006)
- **Regel:** Benutzte Adern ≤ Adernzahl (Schirm zählt nicht); Farbe/Nummer je Kabel eindeutig, bei Paaren über Paarindex/Schirm unterschieden; nur eine PE-Ader (außer „Mehrere PE-Adern zulassen“), nur eine N-Ader. Kabeltyp, Adernzahl, Querschnitt an allen Darstellungen gleich. (P003011–P003014, P003009, P027019)
- Schirm gleichen Namens nur einseitig aufgelegt, außer „Beidseitig aufgelegte Abschirmungen zulassen“ ist am Kabel gesetzt. (P003018)

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003003.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003005.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003006.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003008.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003009.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003011.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003012.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003013.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003014.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003018.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_027019.htm

#### 8.10.6 SPS / Bus (004)
- **Regel:** SPS-Kanal = Karten-BMK + Steckerbezeichnung + Kanalbezeichnung; Kanalbezeichnung rein numerisch > 0. „SPS-Anschluss, intern, allgemein“ ist von Adress-, Beschreibungs- und Kanalprüfungen ausgenommen. (P004027, P004023, P004024, P004091, P501032)
- **Regel:** Je Karte und Kanal ein aktiver E/A-Anschluss; Zweitdarstellungen als „Deaktivierter E/A-Anschluss“. Mischkarten dürfen E1 und A1 im selben Kanal haben (eindeutig je Signalart und Untergerät). (P004020, P004097)
- **Regel:** Alle Anschlüsse eines Kanals (E/A + Versorgung) tragen dieselben Kanaldaten; gemeldet nur, wenn beide Felder gefüllt und verschieden sind. Anschlussbeschreibungen im Kanal verschieden. (P004028, P004022)
- **Regel:** SPS-Adresse, symbolische Adresse und automatischer Funktionstext je CPU eindeutig (CPU aus „CPU: Name“ am SPS-Kasten); leere Funktionstexte werden nicht gemeldet. Die symbolische Adresse entsteht aus dem BMK des per Zielverfolgung erreichten Geräts. (P004021, P004031, P004073, P004030)
- **Regel:** Zielverfolgung: vom Eingang ein Sensor, vom Ausgang ein Aktor erreichbar; Klemmen/Stecker dazwischen brauchen Durchleitung 1→2 und 2→1. `verknuepfen.py`: Kanal → Leiter → Klemme (durchreichen) → Gerät verfolgen. (P004018, P004019)
- Kanaladresse im Kartenbereich Startadresse … Startadresse + Datenlänge (Bit, dezimal); Bereiche verschiedener Karten einer CPU überschneidungsfrei; geprüft nur mit CPU-Name und passendem Adressierungsschema. (P004106, P004107, P004087)
- **Regel:** SPS-Strukturdaten nur an der Hauptfunktion des SPS-Kastens (seit 2.4); Schaltplan- und Übersichtsdarstellung einer Karte werden nur über identisches BMK verknüpft. SPS-Kasten bzw. Gerätekasten ohne Anschluss ist ein Fehler. (P004046, P004004, P004005, P004109, P004006, P016002)
- Bus: gleiches BMK + Bus-System + Schnittstellenname = intern verbunden, identische Netzdaten; je Schnittstelle ein Haupt-Bus-Anschluss. Bus-Leitungen nur innerhalb eines Bus-Systems. Träger-, Steckplatz-, Bus-Adress- und Netzprüfungen laufen nur mit eingetragenem Konfigurationsprojekt – für den QET-Nachbau weitgehend entbehrlich. (P004037, P004040, P004071, P004039, P004007–P004016, P004035)

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004004.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004005.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004006.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004007.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004009.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004010.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004013.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004016.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004018.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004019.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004020.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004021.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004022.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004023.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004024.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004027.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004028.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004030.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004031.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004035.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004037.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004039.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004040.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004046.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004071.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004073.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004087.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004091.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004097.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004106.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004107.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004109.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_016002.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501032.htm

#### 8.10.7 Bauteil-Import und Artikel (501, 013, 012/026)
- Pflichtfelder Typnummer, Bestellnummer, Bezeichnung 1, Produktgruppen (nicht „Undefiniert“); Artikelnummer trimmen; Anschlussnamen der Funktionsschablonen = Anschlussbild; Kanalnummern numerisch; Klemmen mit konkreter statt allgemeiner Funktionsdefinition; Bus-Kabelanschluss-Schablonen ohne Anschlussdaten, aber mit Steckerbezeichnung; Modul-Unterartikel mit BMK oder BMK-ID. (P501011–P501016, P501024, P501027–P501029, P501031–P501033)
- Artikelnummer und Stückzahl der Hauptfunktion decken die Aufbau-Platzierungen ab; Klemme und Klemmenleistendefinition tragen den Artikel der platzierten Klemme. (P012009, P012011, P026019)

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_012009.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_012011.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_026019.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501011.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501012.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501013.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501014.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501015.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501016.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501024.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501027.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501028.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501029.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501031.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501032.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501033.htm

#### 8.10.8 Übrige
- „Nicht übersetzt“ entfällt bei „Nicht automatisch übersetzen“, gekennzeichneten Texten und Wörtern der Ausschlussliste; Segmentierung ist Projekteinstellung. (P008004, P008005)
- Kabelbaum-/Gruppennamen ohne gleichnamige Definition (typisch nach Kopieren aus fremden Projekten) sind ungültig. (P033001, P033002)
- Klemmenplan: zu wenige Brückenebenen unterdrücken Brücken; Ebenenzahl 0 gibt alle Brücken aus. (P015003)

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_008004.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_008005.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_015003.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_033001.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_033002.htm

#### Bedeutung für QET (`verknuepfen.py` und Prüfskript)
- Pflichtregeln für `verknuepfen.py`: BMK zerlegt vergleichen (8.10.1); je BMK genau ein Master, Kontakte nur mit gleichem Kontakt-/Spulenindex; Folio-Reports nur paaren bei gleicher BMK-Struktur, auf verschiedenen Folios und mit Richtung out→in, Ungepaarte melden (8.10.2); SPS-Kanal über Karte + Stecker + Kanalnummer identifizieren und per Zielverfolgung durch Klemmen hindurch an Sensor/Aktor binden (8.10.6).
- Prüfskript-Erweiterungen mit hohem Nutzen: Leiternummer ↔ Potenzial eindeutig (Klemmen durchleitend, Sicherungen trennend), offene Anschlüsse an Klemmen/Steckern, Aderdoppelbelegung inkl. PE/N, Stift/Buchse-Paare.
- Korrektur zu §7: Für Klasse 501 liegen in der Hilfe 2.9 nur 32 Einzelseiten vor (Klassenübersicht: 36); die übrigen Klassenzahlen stimmen mit den Einzelseiten überein. Vermutung: 4 Meldungen der Übersicht haben keine eigene Seite.

## 9. Bedeutung für QET

- **Vorhandene Werkzeuge** decken nur einen Teil ab: `qet_check` (doppelte/fehlende BMK, unnummerierte Leiter, leere Folios, Master ohne Herstellerreferenz, überfüllte Anschlüsse, Reports mit mehreren Leitern), `qet_continuity` (offene Anschlüsse, Potenzialwidersprüche bei Nummer/Farbe/Querschnitt/Kabel, Report-Paar-Widersprüche), `qet_layout_check` (Überlappung, Raster). Kontakt-/Spulenlogik, SPS-Adressen, Kabeladern, Stecker Stift/Buchse und Abbruchstellen-Richtung prüft keines davon.
- **Vorschlag Prüfskript** (Vermutung, nicht umgesetzt): ein `pruefen.py` nach jedem `gesamt.py` + `verknuepfen.py`, das die Tabellen 8.1–8.6 mit „Skript“ als Regeln abarbeitet und eine Tabelle Nr./Kategorie/Folio/BMK/Quelle/Ziel/Text ausgibt (Spalten wie EPLAN-Meldungsverwaltung). Regeln mit höchstem Nutzen beim Nachbau: 017007 (unverknüpfte gleiche BMK), 007009 (Spule ohne Kontakt), 011002/011005 (Report ohne Partner bzw. falsche Richtung), 004019/004018 (SPS-Kanal ohne Sensor/Aktor), 004021 (doppelte SPS-Adresse), 003012–003014 (Aderdoppelbelegung), 001005 (doppelte Klemme).
- Kategorien wie EPLAN: Fehler (verhindert korrekte Auswertung, z. B. doppelte Hauptfunktion), Warnung (wahrscheinlich falsch, z. B. offener Anschluss an Klemme), Hinweis (Stil, z. B. Text zu klein). Welche Meldung EPLAN standardmäßig als was einstuft, steht nicht in den gelesenen Seiten (Lücke).
- Wo das EPLAN-Original selbst gegen eine Regel verstößt (z. B. Reserveklemme ohne Ziel, 001016), den Verstoß übernehmen und im Prüfbericht als „wie Original“ quittieren – entspricht dem EPLAN-Häkchen *Erledigt*.

## 10. Quellen

Meldungsverwaltung und Prüfläufe:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_k_prueflaufprinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_konfigurieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_prueflauf.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_prueflaufonline.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_meldungenansehen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_meldungstexteanzeigen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_h_artikelstammdatenpruefen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_d_meldungsverwaltung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/msgmanagementgui_d_filter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_einstellungenmeldungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_projekteinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_prueflauf.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_check.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_prueflaufmeldungen_benutzerdefebene_230943.htm

Einzelseiten aller Meldungen (Ursache/Lösung): Schema https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_<Nr>.htm, alle 586 Nummern in [meldungen.md](meldungen.md) „Quellen“; die in §8.10 benutzten Seiten stehen dort unter den Unterabschnitten.

Meldungsklassen (Titel aller Meldungen):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_001Terminals.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_002Plugs.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_003Cables.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_004PLCbus.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_005Connections.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_007Devices.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_010Crossreferences.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_011Interruptionpoints.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_013Partsdata.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_014Macro.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_015Reports.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_016Blackboxes.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_017DT.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_022Other.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_025Projectsettings.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_501Partmasterdata.htm
- Neuerungen 2024–2026 zu Prüfläufen: siehe URLs in §8.9 und [neuerungen.md](neuerungen.md) §10
- Nur gezählt, nicht ausgewertet: `messages_o_0122Dpanellayout.htm`, `…_008Foreignlanguages.htm`, `…_020Projectcomparison.htm`, `…_021Placeholderobjects.htm`, `…_024Fluidpower.htm`, `…_0263Dmountinglayout.htm`, `…_027Topology.htm`, `…_028Preplanning.htm`, `…_029Preplanningdepth.htm`, `…_030Subprojects.htm`, `…_032Drillingpattern.htm`, `…_033Wireharness.htm` (alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/)
