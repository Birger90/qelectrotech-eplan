# Beschriftungsexport (Projekt > Beschriftung exportieren…)

Der Export schreibt die Schilder eines Projekts als Datei für die Beschriftungsprogramme der Klemmenhersteller:

- Drahtschilder: je Drahtende die Drahtnummer, mit dem Ziel am anderen Ende
- Klemmenschilder: je Klemme die Klemmennummer und je Klemmleiste ein Leistenschild
- Geräteschilder: je Gerät das Betriebsmittelkennzeichen
- Kabelschilder: je Kabelende die Kabelbezeichnung, mit dem Kabeltyp

Die Daten stammen aus den öffentlichen Sichten der Projektdatenbank: `qet_wires` (Drahtnummern), `qet_terminals` (Klemmleisten aus `terminal_strip`), `qet_device_list` (Kennzeichen) und `qet_cables` (Kabel des CableManager). Das sind dieselben Zeilen, aus denen Verbindungsliste, Klemmenplan und Kabelliste entstehen.

## Auswahl

| Einstellung | Dialog | CLI | Skript (`optionsJson`) |
|---|---|---|---|
| Programm | Beschriftungsprogramm | `--format phoenix\|weidmueller\|wago\|neutral` | 2. Argument oder `"format"` |
| Art | Drähte, Klemmen, Geräte, Kabel | `--kinds wires,terminals,devices,cables` (auch `draht,klemme,geraet,kabel`, `all`) | `"kinds"` |
| Blätter | Blätter von … bis | `--folios 3` oder `--folios 2-5` | `"folioFrom"`, `"folioTo"` |
| Strukturbereich | Anlage (=), Ort (+) | `--plant A1 --location S1` | `"plant"`, `"location"` |
| Kennzeichen | Vollständiges Kennzeichen | `--short` schreibt nur `-K1` | `"fullDesignation": false` |
| Enden | Je Draht- und Kabelende ein Schild | `--one-end` | `"bothEnds": false` |

Anlage und Ort werden ohne Vorzeichen und ohne Groß-/Kleinschreibung verglichen. Eine leere Angabe nimmt alles. Anlage und Ort eines Drahts sind die seines Blatts. Bei Geräten, Klemmleisten und Kabeln gilt die eigene Angabe, sonst die des Strukturkastens oder des Blatts.

Der Dialog merkt sich Programm, Arten und die beiden Schalter. Blätter und Strukturbereich stehen bei jedem Aufruf wieder auf „alles“.

```
qelectrotech --export-marking anlage.qet anlage_phoenix.csv --format phoenix --kinds terminals,wires --folios 1-4
```

```js
var n = qet.exportMarking("C:/temp/schilder.csv", "weidmueller", '{"kinds": "terminals", "plant": "A1"}');
var liste = JSON.parse(qet.markings('{"kinds": "cables"}'));
```

`qet.exportMarking` gibt die Zahl der geschriebenen Schilder zurück, bei einem Fehler -1. `qet.markings` liefert die Schilder als JSON-Feld mit `kind`, `text`, `target`, `group`, `plant`, `location`, `folio`, `folioNumber`, `manufacturer`, `article`, `reference`, `count`.

## Was die Hersteller belegen

Kein Hersteller veröffentlicht ein festes Spaltenschema für den Import. Alle drei Programme ordnen die Spalten einer Fremddatei beim ersten Import von Hand zu und speichern diese Zuordnung. Der Export schreibt deshalb für jedes Programm eine **beschriebene CSV**. Sie folgt dem, was die Importe nachweislich lesen: Trennzeichen, Kopfzeile, Materialspalte, Anzahlspalte und Zeichensatz. Die Spaltenreihenfolge unten ist die Wahl von QElectroTech, keine Herstellervorgabe.

Stand der Quellen: 20.11.2026.

### Phoenix Contact MARKING system / PROJECT complete

Belegt:

- Broschüre „Schnittstelle PROJECT complete – EPLAN Electric P8 / EPLAN Pro Panel“, MNR 1162204/2023-08-15, S. 17–25: <https://assets.phoenixcontact.com/file/15459a7b-b286-4bb3-a123-815d2f6fe973/media/original>
  - Import über File > Data Exchange mit gespeicherten „Importkonfigurationen“.
  - Das Material wird über eine wählbare Spalte zugewiesen. Beim EPLAN-Standardexport ist das Spalte 8 „Klemmenartikelnummern“.
  - Alternativ setzt „Enforce default material“ ein festes Material.
  - Die Spalten werden in der „Field assignment“ auf die Schilder gezogen. Neue Artikel weist man einmal zu.
- Kurzanleitung PROJECT complete, S. 8–9: <https://assets.phoenixcontact.com/file/3706e4b1-1fb7-47fd-aedc-d04ae95cf8cb/media/original>
  - Excel-Import mit Vorschau und „Material mapping“.
  - Die Beschriftungstexte stehen in einer Spalte.
- Produktseite MARKING system: Import und Export über CSV, XML, Excel und ECAD-Systeme. Die Seite selbst ist für automatische Abrufe gesperrt; die Angabe stammt aus dem Suchauszug. <https://www.phoenixcontact.com/en-us/products/software-marking-system-software-1669297>

Nicht belegt: eine feste Dateiendung, Trennzeichen, Kodierung, Spaltennamen sowie der Aufbau der EPLAN-Austauschdatei.

Datei `--format phoenix`: UTF-8 mit BOM, Semikolon, alle Felder in `"…"`, CRLF, eine Kopfzeile. Je Schild eine Zeile, denn eine Anzahlspalte ist nicht belegt.

| Spalte | Inhalt |
|---|---|
| Beschriftung | der gedruckte Text (erste Spalte, siehe Kurzanleitung) |
| Material | Spalte für das „Material mapping“: bei Klemmen mit Artikel die Klemmen-Artikelnummer, sonst `Draht`, `Klemme`, `Klemmleiste`, `Geraet`, `Kabel` |
| Art | `Draht`, `Klemme`, `Klemmleiste`, `Geraet`, `Kabel` |
| Gruppe | Klemme: ihre Leiste; Draht: das Ende, an dem das Schild sitzt |
| Ziel | Draht: das andere Ende; Kabel: der Typ; Gerät: die Beschreibung |
| Blatt, Anlage, Ort | wie im Schriftfeld bzw. Kennzeichen |
| Hersteller, Artikel | aus den Artikeldaten |

### Weidmüller M-Print PRO

Belegt im Handbuch M-Print PRO, Version 2025-01, gedruckte Seiten: <https://assets.dam.weidmueller.com/assets/api/844fa739-ba16-494d-82c7-668ef394f349/original/User_Manual_DE.pdf>

- S. 227: Der Import liest unter anderem `*.csv`, `*.txt` und `*.xlsx`.
- S. 234–237: Spalten mit Trennzeichen (Semikolon, Tabulator …), „Textbegrenzungszeichen“, „Datenherkunft“ (Betriebssystem). Überschrift-Zeile sowie erste und letzte Importzeile sind wählbar.
- S. 262–266 (VK-Import):
  - Eine „Spalte mit Anzahl“ bestimmt, wie oft eine Zeile importiert wird.
  - Die „Referenzspalte“ enthält die Materialnummer oder einen Alias. Der Alias wird über eine Referenzdatei `*.mpr` einem Material zugeordnet.
  - Eine leere Referenzspalte übernimmt die Referenz der vorigen Zeile.
- S. 180–181: Die Option „UTF-8 Kompatibilität“ gibt es für den Fall, dass „Sonderzeichen nicht richtig dargestellt werden“.

Nicht belegt: der innere Aufbau von WEx-, VK- und WMEX-Dateien; die Beispiele im Handbuch sind Bilder.

Datei `--format weidmueller`: Windows-Text (ISO 8859-1; Zeichen außerhalb davon werden `?`), Semikolon, Textbegrenzer `"`, CRLF, eine Kopfzeile. Der Zeichensatz ist gewählt, weil das Handbuch UTF-8 nur als Abhilfe nennt. Erscheinen Umlaute falsch, „Datenherkunft: Windows“ wählen. Import: Datei > Importieren, Filter „VK Text Format“, Referenzspalte „Referenz“, Anzahlspalte „Anzahl“, Feldzuweisung „Beschriftung“ auf das Textfeld. Die Aliase `Draht`, `Klemme`, `Klemmleiste`, `Geraet`, `Kabel` und die Klemmen-Artikelnummern legt man einmal in einer Referenzdatei `*.mpr` auf die gewünschten MultiCard-Materialnummern.

| Spalte | Inhalt |
|---|---|
| Referenz | die Materialnummer des gewählten Druckers für diese Art (siehe unten); ohne Druckerzuordnung die Klemmen-Artikelnummer oder der Alias der Art. Nie leer, weil eine leere Zelle die Referenz der Vorzeile übernähme |
| Anzahl | Schilder dieser Zeile (Kabel mit Schild an beiden Enden: 2) |
| Beschriftung, Ziel, Gruppe, Art, Blatt, Anlage, Ort, Artikel | wie bei Phoenix Contact |

Mit Druckerzuordnung steht in „Referenz“ direkt die Materialnummer. Im VK-Import wählt man dann „Die Referenzspalte enthält Materialnummern“ (Handbuch S. 263–264), eine `*.mpr` ist nicht nötig.

#### Direkt an M-Print PRO und den Drucker

Knopf *In M-Print PRO öffnen* im Dialog, CLI `--mprint [--open] [--print]`, Skript `qet.mprintJobs()` / `qet.openInMPrintPro()`. QET schreibt je Material eine Datei und startet M-Print PRO je Datei mit

```
MPrintPRO.exe "<datei>.csv" [-p] -ImportFilter:CSV -MatNo:<Materialnummer>
```

Belege:

- **Aufruf:** Handbuch S. 223, Abschnitt „Importieren oder Drucken einer Datei“. Dort steht `MPrintPRO.exe "dateiname" [-p] [-ImportFilter:CSV | WTXT | WE | VK | XML] [-MatNo:materialnummer] [-Fallback:materialnummer]`, ebenso in der Programmhilfe von M-Print PRO 9.16.2 (`importierenoderdruckeneinerdatei1.htm`). Mit `-p` wird gedruckt, ohne das Programmfenster zu zeigen. `-MatNo` legt das Material des Imports fest.
- **Dateiaufbau:** Er folgt dem Standard-Importscript `csvfilter.mis`, das dem Installer `M_Print_PRO_Multi_Language_Version_9_16_2.exe` beiliegt. Das Handbuch nennt im Kapitel zu den Programmverzeichnissen für den Ordner `import` „Standard-Importscripte“. Das Script legt fest:
  - `<quote>"</quote>`, `<delimiter>;</delimiter>`, `<os>0</os>`
  - Feld `PROJECT` = Spalte 1, Feld `MARKER` = Spalte 2
  - keine Anzahlspalte (`counter column="-1"`), kein Startversatz, also keine Kopfzeile
- **Was QET daraus schreibt:** keine Kopfzeile, Windows-Text, je Schild eine Zeile `"Projekt";"Markierer"`. Projekt ist bei Klemmen die Klemmleiste, sonst `=Anlage+Ort`.
- **Schlussfolgerung, nicht belegt:** Dass `-ImportFilter:CSV` genau dieses mitgelieferte Script verwendet, schließe ich aus den gleichen Namen (CSV, WTXT, WE, VK, XML ↔ `csvfilter`, `wtxtfilter`, `wefilter`, `vkfilter`, `xmlfilter.mis`). Am installierten M-Print PRO ist das **noch nicht geprüft**.
- **Mehrere Materialien:** Je Material startet QET das Programm einmal. Ob M-Print PRO die Aufrufe in einem Fenster sammelt oder je Aufruf ein Projekt öffnet, ist nicht belegt.
- **Nicht genutzt:**
  - Eigene Importscripte (`*.mis`) und Referenzdateien (`*.mpr`) tragen eine Prüfsumme `<!--SHA::…::-->`, die keine einfache SHA-256 über den Inhalt ist. QET erzeugt deshalb keine solchen Dateien.
  - Die verschlüsselten Markierertypen (`*.mcf`) liest QET nicht.
  - Der AutoPrint-Modus (Ordnerüberwachung, XML `<mp_smmo>`, „What's New M-Print PRO 9.16“ S. 9–11) ist nur an einem Beispiel mit `itemType` „Connection“ beschrieben.

M-Print PRO wird gefunden über den im Dialog gewählten Pfad, sonst unter `Programme\Weidm*\…\MPrintPRO.exe`. Laut `mprintpro.xml` des Installers liegt das Programm in `…\ProgramFiles\bin\MPrintPRO.exe`.

#### Materialien je Drucker

Vorbelegung, im Dialog je Art änderbar (eigene Nummer eintippen). Die Auswahl wird je Drucker gespeichert.

- **PrintJet CONNECT:** Er bedruckt MultiCards und MetalliCards. Das Druckverfahren ist Tintenstrahl mit thermischer Fixierung. Beleg: Broschüre „MultiCard & PrintJet“, Druckschrift 3194230000/01/2026, S. 8. <https://assets.dam.weidmueller.com/assets/api/b74abe15-1232-43a6-82b0-2d2d0aec8dd5/Original/2811580000_MB_PRINTJET_CONNECT_DE_WEB.pdf>
- **THM MultiMark:** Er ist ein Thermotransferdrucker für Endlos- und Rollenmaterial (Varianten „MM“) und Etiketten. Beleg: „MultiMark Marking System“, Stand 25.02.2026. <https://assets.dam.weidmueller.com/assets/api/019ef5c2-672c-7c52-a3bd-19243e343438/Original/MultiMark_Marking_System.pdf>
- **Kein Material passt auf beide Drucker:** Eine MultiCard („MC“) gehört nie auf den THM, ein MM- oder THM-Material nie auf den PrintJet.

| Drucker | Art | Vorgabe | Weitere | Beleg |
|---|---|---|---|---|
| PrintJet CONNECT | Klemme | 1609801044 DEK 5/5 MC NE WS (5 mm) | 1609860000 WS 12/5 MC (Klippon Connect) | „MultiCard & PrintJet“ S. 15–16 |
| PrintJet CONNECT | Klemmleiste | 1876770000 CC 15/27 MC NE WS (ClipCard, Halter CC-H 15/27) | 1609940000 ESG 9/20 | S. 27; als Leistenschild **nicht belegt**, ein eigenes Leistenschild führt die Broschüre nicht |
| PrintJet CONNECT | Gerät | 1609940000 ESG 9/20 MC NE WS | 1876770000 CC 15/27 | S. 25, S. 27 |
| PrintJet CONNECT | Draht | 1918690000 SF 2/21 MC NE WS V2 (0,75–1,5 mm²) | 1918910000 SF 3/21 (1,5–2,5 mm²), 1918630000 SF 1/21 (0,5–0,8 mm²) | S. 19 |
| PrintJet CONNECT | Kabel | 1852490000 SFX 9/24 MC NE WS V2 (Kabelbinder) | 1860120000 SFX 11/60 MC | S. 20 |
| THM MultiMark | Klemme | 2007110000 DEK 5/5 MM WS | 2007190000 WS 12/5 MM WS | „MultiMark Marking System“ S. 14 |
| THM MultiMark | Klemmleiste | 1753231044 THM MT30X 15/6 WS | – | S. 38; als Leistenschild **nicht belegt** |
| THM MultiMark | Gerät | 1753231044 THM MT30X 15/6 WS („standard and most used“ Etikett) | 2005190000 ESG 9/17 MM WS | S. 12, 38, 31 |
| THM MultiMark | Draht | 2719090000 SFM 2/12 1.8-2.5 MM WS | 2437870000 HS-HF 1.6-3.2/25 MM W (Schrumpfschlauch) | S. 19, 25; Zuordnung zum Querschnitt in mm² nicht belegt |
| THM MultiMark | Kabel | 2005320000 SFX 9/24 MM WS (ab Ø 7 mm) | 1774350000 THM WRITEON 25/76 WS | S. 20, 22 |

Die Klemmenschilder passen zu 5-mm-Klemmen (z. B. WDU 2.5). Bei anderen Klemmenbreiten wählt man das passende DEK oder WS bzw. trägt die Nummer ein. Für die THM-Materialien „MM“ fehlen Einträge in der Produktreferenz `productreferences.xml` von M-Print PRO, die Markierertypen selbst liefert der Installer aber mit (`mm_dek_5_5_mm.mcf` u. a.). Die Firmware der Drucker fasst QET nicht an.

### WAGO Smart Script

Belegt:

- Tabelle „Marking export (CAE system to Smart Script)“: `.xlsx`, `.csv`, `.txt` und `.dat` aus EPLAN, WSCAD, ZUKEN und anderen. <https://www.wago.com/mx-en/smart-data/engineering-data>
- Praxistipp: Import mit Vorlage. Überschriften (Gruppennamen) festlegen, Quelldatei und Spalten zuordnen. <https://www.wago.com/global/industries/other-industries/control-cabinet-manufacturing/practical-tip-control-cabinet-smart-script>
- WAGO-EPLAN-Plug-in „Marking Export“: <https://techdocs.wago.com/Software/WAGO_EPLAN_Plug-in/en-US/2873315467.html>

Nicht belegt: Spalten, Trennzeichen, Kodierung und Materialzuordnung; ein Handbuch ist nicht öffentlich.

Datei `--format wago`: UTF-8 mit BOM, Semikolon, `"…"`, CRLF, Kopfzeile, eine Zeile je Schild. Spalten: Beschriftung, Art, Gruppe, Ziel, Blatt, Anlage, Ort, Artikel. Den Markierertyp wählt man in Smart Script, weil keine Materialspalte belegt ist.

### Neutral

Datei `--format neutral`: UTF-8 mit BOM, Semikolon, `"…"`, CRLF, Kopfzeile. Spalten: Art, Beschriftung, Ziel, Gruppe, Anlage, Ort, Blatt, Blattnummer, Hersteller, Artikel, Referenz, Anzahl. Sie ist für jedes andere Programm gedacht und für Prüfungen.

## Regeln der Texte

- Ein Draht ohne Nummer bekommt kein Schild. Jede Leitung des Projekts mit Nummer ergibt je Ende ein Schild; „Gruppe“ ist das Ende, „Ziel“ das andere.
- Klemmennummer: Die Beschriftung der Klemme ohne den Leistennamen, also wird `X1:3` auf Leiste `X1` zu `3`. Ebenen einer Klemme mit gleichem Namen ergeben ein Schild. Reserveklemmen ohne Beschriftung werden übersprungen.
- Geräte: Klemmen fehlen hier, sie stehen schon unter Klemmen. Querverweise fehlen ebenfalls. Ein Gerät aus mehreren Symbolen (Spule mit Kontakten) ist ein Schild.
- Kabel ohne Bezeichnung bekommen kein Schild.
