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
| Referenz | Klemmen-Artikelnummer oder Alias der Art; nie leer, weil eine leere Zelle die Referenz der Vorzeile übernähme |
| Anzahl | Schilder dieser Zeile (Kabel mit Schild an beiden Enden: 2) |
| Beschriftung, Ziel, Gruppe, Art, Blatt, Anlage, Ort, Artikel | wie bei Phoenix Contact |

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
