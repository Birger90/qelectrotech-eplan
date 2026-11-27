# Vergleich OpenWiring ↔ QElectroTech-Fork

Stand: OpenWiring 2.1.16 (Commit 794108904ea9, TypeScript/Electron, GPL-3.0), Fork 1.63.0 (`main` e33d2c5af).
Grundlage: deutsches Handbuch `docs/manuale/de/`, `docs/AGENTE_IA.md` und Quelltext von OpenWiring (nur
gelesen, nichts ausgeführt, kein Code übernommen); README.de.md, CHANGELOG-FORK.md und `sources/` des Forks.

Spalte **Besser**: wer die Funktion heute besser löst (OW = OpenWiring, QET = Fork, = gleichwertig).
Spalte **Maßnahme**: **Ü** übernehmen (fehlt bei uns), **A** angleichen (bei uns vorhanden, OW löst es besser),
**–** nichts zu tun, **N** bewusst nicht (mit Grund). Der Stand der Umsetzung steht am Ende in „Umgesetzt“.

## 1. Querverweise, Unterbrechungspunkte

| Funktion | OpenWiring | Fork | Besser | Maßnahme |
|---|---|---|---|---|
| Kontaktspiegel an der Spule | unten auf der Seite oder unter dem Symbol, jeder Kontakt mit eigenem Symbol, Reservekontakte aus dem Artikel | Kreuz oder Kontakte, Unter-Beschriftung/Unter-Symbol/Fußzeile/frei, unbelegte Kontakte, Kontaktgruppen bis 20 Bezeichnungen | = | – |
| Querverweis Kontakt → Spule `/Seite.Spalte` | ja, klickbar, PDF-Link | ja (`%f-%l%c`, `%=M`), PDF-Dreifachsprung | QET | – |
| Kontaktnummerierung EN 50005 (13-14, 21-22 …) | automatisch je Kontakt | über Kontaktgruppen an der Spule | = | – |
| „Zum Hauptelement / Zu den Kontakten“ | Kontextmenü | Kontextmenü, Strg+Doppelklick | = | – |
| **Paarquerverweis** (Kontakt auf anderer Seite nur dargestellt) | Darstellung ohne eigene Verbindungen, folgt BMK und Anschlüssen des echten Kontakts, Verweis in beide Richtungen, kein Doppel im Kontaktspiegel/Listen, verwaiste Darstellung → Prüfung mit Löschknopf | fehlt (Skill: „kein direktes Gegenstück“) | OW | **Ü** |
| Unterbrechungspunkt Quelle/Ziel 1:1 | ja, Auswahl freier Gegenstücke, Umbenennen zieht mit | ja, Verkettung nach Name/Leiterfunktion, Auswahl im Dialog | = | – |
| **Stern-Unterbrechungspunkt** (eine Quelle, viele Ziele) | *Sternförmig*: verbindet alle freien gleichnamigen Ziele, zeigt Seitenliste; Prüfung schlägt Stern vor (🔧) | fehlt, nur Ketten von Paaren | OW | **Ü** |
| Zustand „wartend“ `/…` und „kein Gegenstück“ `//` | ja, Zähler in der Statusleiste | Prüfung „Unterbrechungspunkt ohne Partner“ (report_without_partner) | OW | **A** (Zustand „endet hier“ als Hinweis statt Fehler) |
| Potenzial hervorheben über alle Seiten | orange, Seitenliste klickbar | Potential-Hervorhebung vorhanden (Changelog) | = | – |

## 2. Prüfung (Prüfpanel)

| Funktion | OpenWiring | Fork | Besser | Maßnahme |
|---|---|---|---|---|
| Schweregrade, Filter, Klick springt | Fehler/Warnung/ungenutzte Daten | Fehler/Warnung/Hinweis, EPLAN-Meldungsnummer, Akzeptieren mit Begründung | QET | – |
| **Ein-Klick-Korrekturen** (🔧) | BMK vergeben, leere Seite löschen, Darstellung löschen, Unterbrechungspunkt umbenennen, Stern bilden, Kabel aus Liste entfernen, Adern aus Artikel, Kabel-BMK vereinheitlichen, Drehung zulassen, Draht an Anschluss teilen, doppelte Drahtnummer neu vergeben, Doppel löschen | fehlt („nur lesend“) | OW | **Ü** (im Panel, `sources/projectcheck/`) |
| Kurzschluss verschiedener Potenziale, scheinbare Verbindung (Draht über Anschluss), Überlappungen | ja | Potentialtypen, offene Enden, Aufbau | = | – (neue Logik-/Leistungsprüfungen baut die Sitzung „Plan prüfen: Steuerlogik und Leistung“) |
| Ungenutzte Daten (Artikel, Symbole, leere Orte) | ja | nein | OW | N – QET hält Symbole in der Projektsammlung; „Sammlung bereinigen“ gibt es upstream |

## 3. Drahtnummern

| Funktion | OpenWiring | Fork | Besser | Maßnahme |
|---|---|---|---|---|
| Muster `{seite}.{n:2}` | `{pagina}.{n:2}`, `{colonna}`, `{quadro}`, `{n:A}`/`{n:a}` | Bausteine unit/ten/hundred/alpha/folio/plant…, Variablen %F %f %c %l %M %LM | OW (lesbarer) | **Ü** Muster als zweite Eingabeform |
| Zähler je Seite / Schrank / Projekt | ja | je Blatt, je Anlage, global | = | im Muster mit „je Schrank“ (Ort `+`) |
| **Reserve je Seite** | n Nummern frei nach jeder Seite | fehlt | OW | **Ü** |
| Reihenfolge nach Spalten oder Zeilen | ja | nur Zeilen (oben links) | OW | **Ü** |
| **Gesperrt / mitlaufend** | fixiert (neue Leiter bekommen erste freie Nummer, vorhandene bleiben) / neu berechnet | Einfrieren je Leiter, Neunummerieren aller | OW | **Ü** Modus „nur Lücken füllen“ |
| Feste Nummern (Hand, Unterbrechungspunkt, SPS-Kanal) überspringen | ja | eingefrorene und nicht gewählte Blätter | = | im Muster berücksichtigt |
| Nummer läuft durch Klemmen | Kästchen je Element | Klemme = gleiches Potential | = | – |
| Querschnitt/Farbe am Schrägstrich | ja | als Text (`%ws`, `%wc`) | OW | N – Darstellungsfrage, Text reicht für EPLAN-Nachbauten |
| Reserveadern gestrichelt | ja | Kabeladern „Reserve“ in Kabelliste | = | – |

## 4. SPS

| Funktion | OpenWiring | Fork | Besser | Maßnahme |
|---|---|---|---|---|
| Karten/Stationen mit Typ, IP, Gerätename | eigene SPS-Ansicht | SPS-Master mit E/A-Tabelle, Makros (ET 200SP, WAGO …) | QET (echte Herstellermakros) | – |
| **Adressformat je Station** | Siemens `I0.0`, **Siemens deutsch `E0.0`**, IEC `%IX0.0`, **Omron `0.00/100.00/2000/2100`**, **TwinCAT `%I*`/`%Q*`** | Siemens international, deutsche Mnemonik nur beim Einlesen gewandelt | OW | **Ü** |
| **„Analoge zuerst“** (WAGO/CODESYS-Koppler) | Bereichsreihenfolge je Station | fehlt | OW | **Ü** |
| **IO-Link-Prozessdaten** (Bytes je Port → `IB4…IB5`) | ja | fehlt | OW | **Ü** |
| Stabile Adressen / Neu vergeben | ja | Vergabe ab Startadresse in Blattfolge | = | im neuen Vergabebefehl |
| Export TIA xlsx, CODESYS-GVL, PLCopen | ja | ja, dazu SimaticML, Abgleich `--compare-tia-tags` | QET | – |
| TwinCAT-GVL mit `AT %I*` | ja | GVL nur mit festen Adressen | OW | **Ü** (über das neue Format) |
| Übersichtsseite je Karte, immer aktuell | ja | E/A-Liste, Kartenquerverweis als Tabelle | OW | N – EPLAN-Nachbau übernimmt die SPS-Übersicht aus dem Original; Liste vorhanden |

## 5. Netzwerk

| Funktion | OpenWiring | Fork | Besser | Maßnahme |
|---|---|---|---|---|
| Netzwerkplan mit Knoten, Bus-Protokollen, Netzwerkkabeln | ja | Leitungen mit Funktion, Kabel; Prüfung Netzwerk | OW | N (Zeichnen) – Netzpläne kommen aus dem EPLAN-Original |
| **IP-Netze, IP-Adressplan** | Netz/Präfix/Gateway, Liste | Prüfung ip_address/subnet_mask (gültig, doppelt, Broadcast, außerhalb) | OW | **Ü** Tabelle |
| **Automatische IP-Vergabe** | erste freie Adresse je Netz | fehlt | OW | **Ü** |
| **Automatische PROFINET-Namen** (`+Q1 -2A1` → `q1-2a1`) | ja | Prüfung nach IEC 61158-6-10 | OW | **Ü** |
| Export der Adresstabelle (CSV/Excel) | ja | fehlt | OW | **Ü** (CSV) |

## 6. Schaltschrankaufbau

| Funktion | OpenWiring | Fork | Besser | Maßnahme |
|---|---|---|---|---|
| Platte, Hutschienen, Kanäle, Geräte maßstäblich, Autoplatzieren | ja | Assistent in sechs Schritten, 3D/STEP, Klima IEC/TR 60890 | QET | – |
| Bohrbild-DXF 1:1 | ja | ja mit Bohr- und Schnittliste | QET | – |
| **Kanal-Füllgrad** | Leiterfläche aus Außendurchmesser ÷ Kanal B×H aus Bezeichnung, Grenze 70 % | Potentiale als Spannbaum, Kanalinnenraum (B−4)×(T−4), 70 %/56 % | QET | – (OW nimmt die Außenmaße als Nutzfläche) |
| **Geschätzte Drahtlänge je Draht** | Weg über Kanäle + Reserve je Ende, in der Drahtliste | nur Summen je Kanal/Schrank | OW | **Ü** |
| **Verdrahtungsreihenfolge** (von–nach) | Hand > T-Knoten-Richtung > Verteiler (Stern) > Position (Kette oben→unten, links→rechts) | Verdrahtungsliste in Blattfolge, kein Kettenplan | OW | **Ü** |
| Aderendhülse je Draht (DIN 46228) | ja | fehlt | OW | **Ü** (Werte aus Herstellerdatenblatt) |
| Loser Draht in Einkaufsliste nach Querschnitt/Farbe | +10 % | Einkaufsliste mit Kabeln | OW | **Ü** über die Drahtlängen (Summe in der Liste) |

## 7. Import, Export, KI

| Funktion | OpenWiring | Fork | Besser | Maßnahme |
|---|---|---|---|---|
| **Plan-Import aus Vektor-PDF fremder CAD (SEE Electrical)** | ganzes Projekt: Blätter, Drähte mit Farbe, BMK, Anschlüsse, Klemmen, Unterbrechungspunkte | `pdf_blatt.py` je Blatt (auf EPLAN-PDF eingestellt), im Programm nur als Bild | OW | **Ü** Projektlauf über alle Seiten |
| **Textformat je Seite für KI-Agenten** | „OpenWiring testo v1“: eine Zeile je Element, `collega -2S1:14 -2K1:A1`, prüfen/anwenden, CLI | MCP-Server mit Abfragen und `qet_edit`, Skript-API | QET (Werkzeug), OW (lesbarer Text) | **Ü** Textexport/-prüfung/-anwendung je Blatt |
| KI-Assistent im Programm | Claude/ChatGPT/Gemini, Abo-CLI | Claude-Assistent (Andockfenster), MCP live | = | – |
| EPLAN: EDZ, XML, Data Portal | EDZ, EPLAN-XML, commercial_data.csv | EDZ, Makro-DXF/EMA, .epdz, .zw1, Makropaket, Data Portal mit Suche | QET | – |
| QET-Import | `.elmt/.qet` | nativ | QET | – |
| PDF-Export | Links, Lesezeichen, Sprache, Schränke wählen | EPLAN-Dreifachsprung, Lesezeichen, PDF/A | QET | – |
| DXF-Export je Seite | R12, Layer | DXF mit Blöcken/Attributen | QET | – |
| Beschriftung Weidmüller/Cembre | M-Print PRO, Cembre GeniusPRO | M-Print PRO, Phoenix, WAGO, THM | QET | N Cembre – bei MeKo nicht im Einsatz |
| Mehrsprachigkeit Plan/Wörterbuch | Wörterbuch, KI-Übersetzung, zweisprachig | Projektsprachen mit CSV | = | – |
| Projektvergleich | `.ecad` vergleichen | `--compare-revisions`, `qet_diff` | = | – |

## 8. Zeichnen und Oberfläche

| Funktion | OpenWiring | Fork | Besser | Maßnahme |
|---|---|---|---|---|
| Menüband, Befehlssuche ⌘K, Radialmenü | ja | Qt-Menüs, Werkzeugleisten | OW | N – Oberflächenumbau außerhalb des Auftrags |
| Leiterbus (dicke Linie mit Leiternamen) | ja | Bündel/Kabel | OW | N – in EPLAN-Plänen selten, Aufwand hoch |
| Einpoliger Übersichtsplan mit Verweisen | ja | Symbole vorhanden, keine eigene Seitenart | OW | N – EPLAN-Übersichtsdarstellung wird als Folio nachgebaut |
| Projizierte Seiten anderer Schränke | ja | fehlt | OW | N – Sonderfall, bei uns über Unterbrechungspunkte |
| Klemmleisten, Mehrstock, Brücken, Klemmleistenplan | ja | Klemmleisten-Editor, Strich-Werkzeug, Katalog | = | – |
| Kabel, Steckverbinder, Pinplan | ja | ja, Kabelkataloge | QET | – |
| Makros/Vorlagen | `.ecmacro` | `.qetmak` mit Platzhaltern und Auswahltabelle | QET | – |
| Revisionen mit Wolken | ja | ja, Snapshot, Vergleich | QET | – |
| Simulation, Fluid, SolidWorks/3D | – | ja | QET | – |

## Umgesetzt

Branch `feature/openwiring-abgleich`, Version 1.67.0. Getestet mit Skripten über die gebaute exe
(`QET_ENABLE_SCRIPTING=1`, `--run`), MCP-Werkzeuge über `qet_edit`, PDF-Import an einem echten Vektor-PDF.

| Funktion | Stand | Wo |
|---|---|---|
| Stern-Unterbrechungspunkt | übernommen: Verknüpfungsfenster *Stern*, automatisches Verketten an den Stern, Querverweis mit allen Zielen, Undo | `ReportElement`, `ReportStar`, `qet.setReportStar()` |
| Paarquerverweis | übernommen: Darstellung eines Kontakts auf anderem Blatt, gegenseitiger Verweis, nicht im Kontaktspiegel, Prüfung verwaister/verdrahteter Darstellungen | `PairRepresentation`, `qet.addPairRepresentation()` |
| Ein-Klick-Korrekturen | übernommen: Unterbrechungspunkt verbinden / Stern bilden / an Stern anschließen, Kontakt mit Spule verbinden, BMK vergeben, doppelte Drahtnummer, verwaiste Darstellung löschen | `sources/projectcheck/checkfixes.cpp`, Prüfpanel *Beheben* |
| Drahtnummern-Muster | übernommen: `{seite}.{n:2}`, `{n:A}`, je Blatt/Schrank/Projekt, Reserve je Seite, Spalten/Zeilen, gesperrt/mitlaufend | `WireNumberPattern`, *Projekt > Drahtnummern nach Muster …* |
| SPS-Adressformate je Station | übernommen: Siemens deutsch, Omron, TwinCAT `%I*` (GVL `AT %I*`), IO-Link, analoge zuerst | `PlcAddressing`, `qet.assignPlcAddresses()` |
| Netzwerk-Adresstabelle | übernommen: Tabelle mit Problemen, IP-Vergabe aus IP-Netzen, PROFINET-Namen aus Ort und BMK, CSV | `NetworkAddresses`, *Projekt > Netzwerkadressen …* |
| Drahtlänge je Draht, Verdrahtungsreihenfolge, Aderendhülse | übernommen (Routing des Forks blieb, war schon genauer) | `PanelWiring::runs`, `qet.exportPanelWireList()`, FORMELN.md W9–W11 |
| Vektor-PDF fremder CAD | übernommen als Projektlauf über alle Seiten | `tools/pdf_import/pdf_projekt.py` |
| Textformat je Seite für KI | übernommen als QET-Text v1 | MCP `qet_folio_text`, `qet_folio_text_check`, `qet_folio_text_apply` |
| Wartend/„endet hier“ | **offen**: Zustand „kein Gegenstück“ als Hinweis statt Fehler nicht gebaut | – |

### Angeglichen

- **Drahtnummern:** zusätzlich zu den QET-Bausteinen das lesbare Muster wie in OpenWiring; Reserve, Spalten-
  reihenfolge und „gesperrt“ gab es vorher nicht.
- **Prüfpanel:** war nur lesend, hat jetzt Korrekturen wie OpenWiring (🔧), aber mit Undo-Schritt und Neuprüfung.
- **TIA/CODESYS-Export:** TwinCAT-Adressen `%I*` gehen jetzt durch (vorher als ungültige Adresse gemeldet).

### Bewusst nicht

- **Kanal-Füllgrad nach OpenWiring:** OpenWiring rechnet mit den Außenmaßen aus der Kanalbezeichnung; der Fork
  nimmt den Innenraum und den Spannbaum je Potential – bleibt.
- **Menüband, Befehlssuche, Radialmenü, Leiterbus, einpoliger Plan als Seitenart, projizierte Seiten, Cembre-
  Beschriftung, „ungenutzte Daten“:** Oberflächenumbau bzw. ohne Nutzen für EPLAN-Nachbauten (Gründe oben je Zeile).
- **SPS-Übersichtsseite je Karte:** EPLAN-Nachbauten übernehmen die Übersicht aus dem Original.
- **Live-Neunummerierung bei jeder Änderung:** „mitlaufend“ nummeriert beim Zeichnen eines Leiters und auf Knopfdruck
  neu, nicht bei jedem Verschieben (Undo-Stapel bliebe sonst voller Neunummerierungen).
