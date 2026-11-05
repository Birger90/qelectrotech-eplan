---
name: eplan-wissen
description: Offizielles Verhalten von EPLAN (Electric P8 / Plattform 2.9 bis 2026) nach der EPLAN-Hilfe – PDF-Export mit Dreifachsprung, Sprungfolge, Lesezeichen und BM-Seiten; Querverweise, Kontaktspiegel, Abbruchstellen (Stern/Kette); Kennzeichnung nach IEC 81346 (=, +, -, Strukturkästen); Klemmen, Mehrstockklemmen, Brücken, Stecker; Kabel, Adern, Schirm; SPS-Karten, Kanäle, Adressen; Auswertungen, Formulare, Platzhalter; Makros, Artikel, Data Portal. Jede Regel mit Quell-URL und „Bedeutung für QET“. Immer verwenden, wenn ein EPLAN-Plan in QElectroTech nachgebaut, verknüpft oder als PDF wie EPLAN ausgegeben wird, wenn EPLAN-Bauteile/Makros importiert werden, oder bei Fragen „wie macht EPLAN das“, „was bedeutet dieser Querverweis“, „warum springt das PDF dahin“. Ergänzt schaltplan-wissen (Lesen von Plänen) und qet-schaltplan-zeichnen (Werkzeug).
---

# EPLAN-Wissen (aus der offiziellen EPLAN-Hilfe)

Zusammenfassung der EPLAN-Hilfe (https://www.eplan.help/de-de/Infoportal/, Plattform 2.9, abgeglichen
mit 2023–2026) in eigenen Worten. Jede Aussage in den Referenzdateien trägt ihre Quell-URL. Aussagen
ohne Hilfe-Beleg sind als **Vermutung** oder als „aus dem Fork-Code“ markiert.

**Grundsatz:** Erst hier nachschlagen, dann aus Original-PDFs ableiten. Wo die Hilfe schweigt
(z. B. Ende der Sprungkette, Mitte ohne BM-Seiten), das Original-PDF mit pymupdf (`get_links()`,
`get_toc()`) prüfen und das Ergebnis in der passenden Referenzdatei als „Beobachtet“ nachtragen.

## Wann welche Referenz lesen

| Aufgabe / Frage | Datei |
|---|---|
| PDF-Navigation nachbauen, `verknuepfen.py`, jump_next/main/prev, Lesezeichen, BM-Seiten, roter Blinkrahmen, PDF/A | [referenz/pdf-export.md](referenz/pdf-export.md) |
| Spule ↔ Kontakte, Kontaktspiegel, Paarquerverweis, Querverweisformat (`/2.6`), Abbruchstellen (Potentialpfeile), Stern/Kette, Potential- und Verbindungsdefinitionspunkte, Spalten/Zeilen-Raster | [referenz/querverweise.md](referenz/querverweise.md) |
| BMK `=Anlage+Ort-BMK`, Vorzeichen (`==`, `=`, `++`, `+`, `&`, `#`, `-`), sichtbares vs. vollständiges BMK, Strukturkästen/Ortsrahmen, Vererbung, Seitennamen, Kennbuchstaben, Nummerierung | [referenz/kennzeichnung.md](referenz/kennzeichnung.md) |
| Klemmenleiste, Haupt-/Nebenklemme, Mehrstockklemmen/Etagen, Brücken, Sortierung, intern/extern, Klemmenplan-Formulare, Stecker (Stift/Buchse) | [referenz/klemmen.md](referenz/klemmen.md) |
| Kabeldefinition, Adern (Farbe/Nr., „4G1,5“ vs. „4PE“), Schirm, Quelle/Ziel, Kabelplan/-übersicht, Verbindungseigenschaften und ihre Vererbung | [referenz/kabel.md](referenz/kabel.md) |
| SPS-Station/Karte/Kanal, Anschlussarten, Adresse vs. Identität, Übersicht ↔ allpolig, Kanalmakros, SPS-Navigator, Datenaustausch (AutomationML, TIA) | [referenz/sps.md](referenz/sps.md) |
| Stückliste, Summenstückliste, Inhaltsverzeichnis, Deckblatt, Formulare (*.f01 …), Platzhalter, Seitenumbruch, Normblatt, Seitentypen, Seitennummerierung | [referenz/auswertungen.md](referenz/auswertungen.md) |
| Bauteil-Import: Makroarten (.ema/.ems/.emp), Varianten/Darstellungsarten, Symbolvarianten A–H, Funktionsdefinitionen, Artikel und Funktionsschablonen, Makro am Artikel, EDZ, Data Portal | [referenz/makros-artikel.md](referenz/makros-artikel.md) |

Jede Datei hat ein Inhaltsverzeichnis am Anfang und eine Quellenliste am Ende. Nur den nötigen
Abschnitt lesen.

## Die wichtigsten Regeln auf einen Blick

Quellen stehen in den Referenzdateien beim jeweiligen Abschnitt.

1. **Dreifachsprung** (pdf-export §2): Mitte = Nebenfunktion → Hauptfunktion, Hauptfunktion → BM-Seite;
   links vorwärts, rechts rückwärts in der Sprungfolge. Die Folge umfasst alle Darstellungsarten,
   Platzierungen **und Auswertungseinträge**, geordnet wie die „Verweise“ der BM-Seite. Der Sprungbereich
   schließt die Eigenschaftstexte des Symbols ein.
2. **Sprungziel** (pdf-export §6): roter Blinkrahmen, „Zoom verwenden“ standardmäßig aus; bei PDF/A und
   Schreibschutz kein Rahmen.
3. **Haupt-/Nebenfunktion** (querverweise §1, §4): Verknüpfung über gleiches BMK + passende
   Funktionsdefinition; genau eine Hauptfunktion je Betriebsmittel, nur sie trägt Artikeldaten.
4. **Querverweisformat** ist Projekteinstellung; gleiche Seite wird unterdrückt (`/2.6` → `.6`), gekürzt
   gegen die aktuelle Seitenstruktur. Spalten zählen in EPLAN oft ab 0 (querverweise §7, §13).
5. **Abbruchstellen** (querverweise §8): Kette = paarweise 1→2, 3→4 (passt zu QET-Folio-Reports);
   Stern = eine Quelle listet alle Ziele (in QET nur mit Ersatzlösung).
6. **BMK** (kennzeichnung §7–9): sichtbar ist nur, was von Seite/Strukturkasten abweicht; ein
   Strukturkasten wirkt, wenn der Einfügepunkt des Symbols darin liegt; Schlüssel für Verknüpfung und
   Stückliste ist immer das vollständige `=Anlage+Ort-BMK`.
7. **Mehrstockklemme** (klemmen §5): gleiches BMK, direkt aufeinanderfolgend, Etagen stetig, gleiche
   Geräteposition; Reihenfolge der Leiste = Geräteposition, dann Sortierkennung, dann Bezeichnung.
8. **Kabelader** (kabel §3, §6): Verbindung gehört zum Kabel über Kabeldefinitionslinie,
   Verbindungsdefinitionspunkt oder Kabelanschluss; volle Kabeldaten nur an der Hauptfunktion; Kennbuchstabe
   X ist immer Ziel.
9. **SPS** (sps §4–6): Identität eines Anschlusses = Karten-BMK + Stecker + Anschlussbezeichnung, **nicht**
   die Adresse. Querverweis Übersicht ↔ Schaltplan über dieselben Merkmale. Kanalmakro je Kanal (deckt
   den Hausgrundsatz „SPS-Kanal = Kanalmakro des Moduls“).
10. **Auswertungen** sind Momentaufnahmen (auswertungen §8): Weicht eine Original-Liste vom Schaltplan ab,
    führt der Schaltplan.
11. **Makros** speichern nur Verweise auf Symbole, Funktionsdefinitionen und Artikel (makros-artikel §2);
    Variante A der SPS-Makros = E/A-Übersicht. Am Artikel gilt beim Einfügen: Firmenstandard → normspezifisches
    Makro → Schaltplanmakro (ID 22145).

## Bekannte Lücken im QET-Fork (Stand der Recherche, aus Code-Durchsicht)

- Dreifachsprung vorhanden (`sources/pdf_links.cpp`); **fehlen**: Lesezeichen (/Outlines), BM-Seiten,
  roter Blinkrahmen, Auswertungseinträge in der Sprungfolge.
- Fork springt per /FitR aufs Bauteil, EPLAN standardmäßig auf die ganze Seite.
- `eplanmacroreader` liest nur die erste Variante mit `MacroFuncType="1"`; weitere Varianten (Übersicht,
  Kanäle) gehen verloren. Codes für `MacroFuncType` stammen nur aus dem Fork, nicht aus der Hilfe.
- QET-Querverweisformel: `%F` = Blattname, `%f` = Index; für EPLAN-Nachbauten eher `%F.%c` statt `%f-%l%c`.
- SPS-Verknüpfung über die Adresse ist nur eine Notlösung (siehe Regel 9).

## Offene Fragen (gegen ein Original-PDF prüfen)

- Läuft die Sprungkette am Ende um? Was tut die Mitte einer Hauptfunktion ohne BM-Seiten?
- Wie erzeugt EPLAN den Blinkrahmen technisch (JavaScript, /Hide-Aktionen)?
- Sind Querverweis- und Abbruchstellentexte selbst klickbar, und wohin?
- Seitenumbruchregeln von Klemmen- und Kabelplan (Platzhalter-Hilfe `placeholder_o_main.htm` noch nicht gelesen).

## Pflege

Neue Erkenntnisse in die passende Referenzdatei, mit Quell-URL (Hilfe) oder „Beobachtet am Original-PDF“.
Keine Kunden-, Auftrags- oder Plannummern in diesen Skill schreiben. Projektspezifisches gehört in
`schaltplan-wissen` bzw. die Memory.
