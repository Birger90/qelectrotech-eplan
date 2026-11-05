---
name: eplan-wissen
description: Die gesamte EPLAN-Plattform-Hilfe 2.9 (rund 3.100 Seiten, abgeglichen mit 2023–2026) als Wissen – PDF-Export mit Dreifachsprung, Sprungfolge, Lesezeichen, BM-Seiten; Querverweise, Kontaktspiegel, Abbruchstellen; Kennzeichnung IEC 81346; Betriebsmittel, Nummerierung, Drahtnummern; Schaltplan-Editor, Verbindungen, Potentiale, Symbole; Klemmen, Stecker, Kabel, Verdrahtung; SPS, Fluid; Auswertungen, Formulare, Platzhalter; Makros, Artikel, Funktionsdefinitionen, Data Portal, XML-Formate, alle Eigenschafts-IDs; Prüfläufe und alle Meldungen; Projekte, Revision, Einstellungen, API; Schaltschrankaufbau, 3D, Vorplanung; Glossar. Jede Regel mit Quell-URL und „Bedeutung für QET“. Immer verwenden, wenn ein EPLAN-Plan in QElectroTech nachgebaut, verknüpft, geprüft oder als PDF wie EPLAN ausgegeben wird, wenn EPLAN-Bauteile/Makros/EDZ importiert werden, oder bei Fragen „wie macht EPLAN das“, „was bedeutet dieser Querverweis/diese Eigenschaft/diese Meldung“, „warum springt das PDF dahin“. Ergänzt schaltplan-wissen (Lesen von Plänen) und qet-schaltplan-zeichnen (Werkzeug).
---

# EPLAN-Wissen (aus der offiziellen EPLAN-Hilfe)

Zusammenfassung der **gesamten** EPLAN-Plattform-Hilfe 2.9 (https://www.eplan.help/de-de/Infoportal/):
alle 2.091 Themen des Inhaltsverzeichnisses, 586 Meldungsseiten, 396 Glossarseiten und 28 weitere Dialoge,
dazu ausgewählte Seiten der Stände 2023–2026. In eigenen Worten; jede Aussage trägt ihre Quell-URL
(manche Dateien definieren oben ein URL-Präfix und zitieren dann nur den Dateinamen). Aussagen ohne
Hilfe-Beleg sind als **Vermutung** oder „aus dem Fork-Code“ markiert, Widersprüche als **Korrektur:**.

**Grundsatz:** Erst hier nachschlagen, dann aus Original-PDFs ableiten. Wo die Hilfe schweigt, das
Original-PDF mit pymupdf (`get_links()`, `get_toc()`) prüfen und das Ergebnis in der passenden Datei als
„Beobachtet am Original-PDF“ nachtragen. Jede Datei hat oben ein Inhaltsverzeichnis – nur den nötigen
Abschnitt lesen, die großen Tabellen (IDs, XML, Meldungen) per Grep durchsuchen.

## Wann welche Referenz lesen

**Nachbau eines Plans – Kernthemen**

| Aufgabe / Frage | Datei |
|---|---|
| PDF-Navigation, `verknuepfen.py`, jump_next/main/prev, Lesezeichen, BM-Seiten, Blinkrahmen, PDF/A, Seitenreihenfolge | [pdf-export.md](referenz/pdf-export.md) |
| Spule ↔ Kontakte, Kontaktspiegel, Paarquerverweis, Format `/2.6`, Abbruchstellen Stern/Kette, Querverweis-Einstellungen | [querverweise.md](referenz/querverweise.md) |
| `=Anlage+Ort-BMK`, Vorzeichen, sichtbares/vollständiges BMK, Strukturkästen, Vererbung, Kennbuchstaben | [kennzeichnung.md](referenz/kennzeichnung.md) |
| Funktionen, Haupt-/Nebenfunktion, Registerkarten der BM-Eigenschaften, BMK-Übernahme von links, Gerätekästen, Geräteliste | [betriebsmittel.md](referenz/betriebsmittel.md) |
| BMK-Nummerierung online/offline, Drahtnummern (Formate, Platzierung, Umfang), Klemmen-/Steckernummerierung | [nummerierung.md](referenz/nummerierung.md) |
| Seitennavigator, Seitenname/-typ, Pfadnummerierung, Suchen/Ersetzen, Projektstruktur | [navigatoren-seiten.md](referenz/navigatoren-seiten.md) |
| Zeichnen: Autoconnect, Winkel/T-Abzweig (Zielreihenfolge!), Leitungsverteiler, Kästen, Pfad-Funktionstexte, Einpolig, Ebenen, GRAFCET | [schaltplan-editor.md](referenz/schaltplan-editor.md) |
| Verbindungen, Quelle/Ziel, Vererbung der Leiterdaten, Verbindungsdefinitionspunkt, netzbasiert, Potentiale/Signale | [verbindungen-potentiale.md](referenz/verbindungen-potentiale.md) |
| Symbole, Bibliotheken, Symbolnummer, Varianten A–H, Anschlusspunkte, Einfügepunkt | [symbole.md](referenz/symbole.md) |
| Klemmenleiste, Mehrstockklemmen, Brücken, Teilklemmen, Sortierung, intern/extern, Stecker | [klemmen.md](referenz/klemmen.md) |
| Kabel, Adern, Schirm, Quelle/Ziel, Kabel-BMK mit Klammern, Automatikkabel | [kabel.md](referenz/kabel.md) |
| Topologie, Routing, Fertigungsverdrahtung, Drahtlisten, Kabelbaum, Beschriftung | [verdrahtung-fertigung.md](referenz/verdrahtung-fertigung.md) |
| SPS-Station/Karte/Kanal, Adresse vs. Identität, Übersicht ↔ allpolig, Adressierung, Bus, AutomationML | [sps.md](referenz/sps.md) |
| Fluid (Hydraulik/Pneumatik), DIN ISO 1219-2, Schläuche, Rohrleitungen | [fluid.md](referenz/fluid.md) |

**Listen, Formulare, Ausgabe**

| Aufgabe / Frage | Datei |
|---|---|
| Stückliste, Summenstückliste, Inhaltsverzeichnis, Auswertungsblöcke, Aktualisieren, Seitentypen, Normblatt | [auswertungen.md](referenz/auswertungen.md) |
| Formulareditor, Platzhalter je Formulartyp f01–f51, **Seitenumbruch Klemmen-/Kabelplan**, Wiederholzeichen | [formulare-platzhalter.md](referenz/formulare-platzhalter.md) |
| Drucken, DXF/DWG (Ebenen EPLANxxx, Blockattribute), EPDZ | [dxf-dwg-druck.md](referenz/dxf-dwg-druck.md) |
| Mehrsprachige Texte, Übersetzung, Wörterbuch | [uebersetzung.md](referenz/uebersetzung.md) |
| Schaltschrankaufbau 2D, Montageplatte, Legenden/Positionsnummern | [schaltschrankaufbau.md](referenz/schaltschrankaufbau.md) |
| 3D-Montageaufbau, Kupfer, Kontureditor, NC, STEP, PXF | [3d-nc-kupfer.md](referenz/3d-nc-kupfer.md) |
| Vorplanung, Segmente, R&I | [vorplanung.md](referenz/vorplanung.md) |

**Bauteil-Import und Daten**

| Aufgabe / Frage | Datei |
|---|---|
| Makroarten .ema/.ems/.emp, Varianten, Darstellungsarten, Makrokasten, Wertesätze, Makro am Artikel | [makros-artikel.md](referenz/makros-artikel.md) |
| Artikeldatenbank (tblPart, tblFunctionTemplate …), Funktionsschablonen, Zubehör, Import/Export, BMEcat, Data Portal | [artikelverwaltung.md](referenz/artikelverwaltung.md) |
| Funktionsdefinitionen (Kategorie → QET-Elementtyp), Anschlusslogik | [funktionsdefinitionen.md](referenz/funktionsdefinitionen.md) |
| `part.xml`/XML-Tags und Attribute, alle 1.353 Funktionsdefinitionen, Produktgruppen, Auswahllisten | [xml-datenformate.md](referenz/xml-datenformate.md) (Grep) |
| Eigenschafts-ID → Name/API-Define (alle 2.569, z. B. P20406, 22145) | [eigenschaften-ids.md](referenz/eigenschaften-ids.md) (Grep) |

**Prüfen, Projekt, Werkzeuge**

| Aufgabe / Frage | Datei |
|---|---|
| Prüfliste für QET-Nachbauten, Prüfläufe, Schemata | [pruefungen.md](referenz/pruefungen.md) |
| Meldung Pnnnnnn: Ursache, Korrektur, QET-Prüfweg (alle 586; 130 als **Regel** markiert) | [meldungen.md](referenz/meldungen.md) (Grep) |
| Projektarten, Dateiendungen, Vorlagen, Sichern/Packen, Normwechsel, Projektkorrektur | [projektverwaltung.md](referenz/projektverwaltung.md) |
| Revision, Änderungsmarkierung, Vergleich ohne UUIDs | [revision.md](referenz/revision.md) |
| Einstellungsebenen, Schemata, wichtige Einstellungen mit Wirkung, Mehrbenutzer | [einstellungen-oberflaeche.md](referenz/einstellungen-oberflaeche.md) |
| API-Aktionen mit Parametern (export, check, …), Skripte, Kommandozeile | [api-aktionen.md](referenz/api-aktionen.md) |
| Neuerungen 2.9–2026 nach Thema | [neuerungen.md](referenz/neuerungen.md) |
| Begriff unklar | [glossar.md](referenz/glossar.md) |

## Die wichtigsten Regeln auf einen Blick

Quellen stehen in den Referenzdateien beim jeweiligen Abschnitt.

1. **Dreifachsprung** (pdf-export): Mitte = Nebenfunktion → Hauptfunktion, Hauptfunktion → BM-Seite; links
   vorwärts, rechts rückwärts. Die Sprungfolge umfasst alle Darstellungsarten, Platzierungen **und
   Auswertungseinträge**, geordnet wie die „Verweise“ der BM-Seite; Sprungbereich inkl. Eigenschaftstexte.
   Seit 2.9 roter Blinkrahmen, Zoom aus (bei PDF/A/Schreibschutz kein Rahmen).
2. **Haupt-/Nebenfunktion**: Verknüpfung über gleiches (zerlegtes!) BMK + passende Funktionsdefinition; genau
   eine Hauptfunktion je BMK, nur sie trägt Artikel. Kontakte ohne eigenes BMK übernehmen es von links
   (Einfügepunkt auf gleicher Höhe), sonst vom umgebenden Kasten (betriebsmittel, meldungen P007005/P017007).
3. **Abbruchstellen**: Kette paarweise 1↔2, 3↔4 nach Sortierkennung/Seitenfolge; Pfeilspitze → Pfeilstumpf,
   verschiedene Seiten; Stern = eine Quelle listet alle Ziele (querverweise, meldungen P011001–008).
4. **Verbindungen**: Nur Autoconnect-Linien sind elektrisch; T-Abzweig legt die Zielreihenfolge fest (gerade =
   1. Ziel); Leiterdaten werden vererbt (Anschluss → … → Netz → … → Verbindungsdefinitionspunkt); Teilklemmen
   sind ein Potential (schaltplan-editor, verbindungen-potentiale).
5. **BMK**: sichtbar nur, was von Seite/Strukturkasten abweicht; Kasten wirkt über den Einfügepunkt; Schlüssel
   ist immer `=Anlage+Ort-BMK`; Anzeige kann umgeformt sein (Großschreibung, Klammern ausgeblendet) –
   normalisieren. Kabel-BMK mit Klammerteil `-W(-X1/-M1).1` nicht zerlegen (kennzeichnung, kabel, einstellungen).
6. **SPS**: Identität = Karten-BMK + Stecker + Anschlussbezeichnung, **nicht** die Adresse; Kanalmakro je Kanal;
   SPS-Daten nur am Master-Kasten; nur die grafisch erste Kanalplatzierung bekommt den Übersichtspartner (sps).
7. **Klemmenplan**: Reihenfolge = Geräteposition (20367) → Sortierkennung (20809) → Bezeichnung; extern links,
   intern rechts; Seitenumbruch bei vollem Datenbereich, neuer Leiste, Schemawechsel, voller Kabelspalte,
   neuem Strukturblock (klemmen, formulare-platzhalter).
8. **Auswertungen** sind Momentaufnahmen; weicht eine Original-Liste vom Schaltplan ab, führt der Schaltplan.
   Strukturkennzeichen-Reihenfolge folgt der Kennzeichen-Datenbank, nicht dem Alphabet (auswertungen).
9. **Darstellungsarten** sind nummeriert (0 neutral, 1 allpolig, 2 einpolig, 3 Paarquerverweis, 4 Übersicht …
   11 funktional), Makrovarianten A–P = 0–15 (api-aktionen, makros-artikel). „Extern“/„Grafik“ werden nicht
   ausgewertet; funktionale Platzierungen nicht in Stückliste/Klemmenplan.
10. **Makros/Artikel**: Makros speichern nur Verweise (Bibliothek + Symbolnummer + Variante); Artikel nennt sein
    Makro (22145/`P_ARTICLE_MACRO`); Funktionsschablonen sind die Soll-Funktionsliste eines Geräts, Zuordnung über
    Funktionsdefinition **und** Anschlussbezeichnung (artikelverwaltung, funktionsdefinitionen).

## Gefundene Fork-Baustellen (aus Code-Durchsicht, ungeprüft gegen echte Daten)

- **PDF** (`sources/pdf_links.cpp`): Dreifachsprung vorhanden; es fehlen Lesezeichen (/Outlines), BM-Seiten,
  Blinkrahmen und Auswertungseinträge in der Sprungfolge; Fork zoomt per /FitR, EPLAN zeigt die ganze Seite.
- **Makro-Import** (`eplanmacroreader.cpp`): liest nur die erste Variante mit `MacroFuncType="1"` (Übersicht,
  Kanäle, „Allpolig Fluid“ gehen verloren); `pin.function = P20406` – P20406 ist laut Hilfe die
  *Steckerbezeichnung*, die Anschlussbeschreibung ist P20029/P20023; Funktionsdefinition der Objekte wird nicht
  ausgewertet.
- **EDZ-Import** (`sources/import/edz/edzpart.cpp`): `connectionDesignation` ist zeilenweise getrennt, wird aber
  als ein Pin gelesen (A1/A2 → ein Pin); `terminalNr` bedeutet je Produktgruppe Stecker, Kontakt oder Querschnitt;
  `functiondefcategory/-group/-id`, `P_ARTICLE_PRODUCTGROUP`, `P_ARTICLE_MACRO` ungenutzt.
- **Querverweis-Formel**: `%F` = Blattname, `%f` = Index; für EPLAN-Nachbauten `%F.%c` statt `%f-%l%c`.
- **Verknüpfung**: SPS über die Adresse ist nur eine Notlösung (Regel 6); Paar- und Sternquerverweis fehlen.

## Offene Fragen (gegen ein Original-PDF prüfen)

- Läuft die Sprungkette am Ende um? Wohin führt die Mitte einer Hauptfunktion ohne BM-Seiten?
- Wie erzeugt EPLAN den Blinkrahmen technisch (JavaScript, /Hide)? Sind Querverweistexte selbst klickbar?
- Werden Mehrstockklemmen über ein Seitenende getrennt? Standard-Zeilenzahl der Formulare?
- Voreingestellte Kategorie (Fehler/Warnung/Hinweis) je Prüfmeldung; Aufbau von `.epj`/`.epdz`.

## Pflege

Neue Erkenntnisse in die passende Referenzdatei, mit Quell-URL (Hilfe) oder „Beobachtet am Original-PDF“.
Keine Kunden-, Auftrags- oder Plannummern in diesen Skill. Projektspezifisches gehört in `schaltplan-wissen`
bzw. die Memory. Der Skill liegt zusätzlich versioniert im Fork unter `.claude/skills/eplan-wissen/` – nach
Änderungen dorthin kopieren.
