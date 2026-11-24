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

## THM direkt drucken (Projekt > THM-Etiketten drucken…)

Die Schilder gehen ohne M-Print PRO direkt an einen Weidmüller-Thermotransferdrucker der Reihe THM (THM MMP, THM MultiMark). Diese Drucker beruhen auf cab-Technik und verstehen cabs Druckersprache JScript. QElectroTech schickt den Auftrag als rohe Bytes über TCP, Standardport 9100.

### Was belegt ist und woher

Quelle aller Befehle: cab, „JScript Programming Manual“, Ausgabe 07/2026, Firmware 5.48: <https://www.cab.de/media/pushfile.cfm?file=3047> (abgerufen 24.11.2026; Seitenzahlen = gedruckte Seiten).

| Befehl | Verwendung in QET | Seite |
|---|---|---|
| `v` CR | Firmware und Modell abfragen, Antwort z. B. `5.28 Sep 05, 2019 (SQUIX 4/300MP)` | 68 |
| `ESC s` | Status, Antwort `XYNNNNNNZ` (9 Zeichen): X `Y` online / `N` offline; Y `-` kein Fehler, sonst Fehlerbuchstabe; NNNNNN noch zu druckende Etiketten; Z `Y` Interpreter aktiv (Auftrag in Arbeit) / `N` Bereitschaft | 29–30 |
| `ESC l` | Material, Antwort `XNNNN` (5 Zeichen): X `Y` synchronisiert / `N` nicht; NNNN Etikettenabstand in mm, Unterkante bis Unterkante des nächsten Etiketts; `0000` unbekannt | 24 |
| `ESC p1`, `ESC p0` | Pause, Fortsetzen | 26–27 |
| `ESC t` | Abbruch: beendet den Auftrag und leert den Eingangspuffer; danach mindestens 1 s warten | 31 |
| `ESC oUTF-8;` | Codepage für den nächsten Auftrag, vor den Etikettendaten | 25 |
| `m m` | Maßeinheit Millimeter | 53 |
| `J` | Auftragsbeginn | 202 |
| `H speed,heat,T\|D` | Geschwindigkeit (mm/s), Heizkorrektur −20…+20, Thermotransfer bzw. Thermodirekt | 196–197 |
| `S ptype;xo,yo,ho,dy,wd` | Etikettgröße: Sensor (`e` endlos, `l1` Lücke, `l0` Reflexmarke oben, `l2` Reflexmarke unten), Verschiebung, Höhe, Höhe + Lücke, Breite | 220–222 |
| `O R` | Inhalt um 180° drehen; `O` steht direkt nach `S` | 212–213 |
| `T x,y,r,font,size;text` | Textfeld; Schrift 3 = Swiss 721, Größe in mm | 224–225 |
| `[J:cW]` | Text in einem W mm breiten Feld zentrieren | 329 |
| `[U:$5B]` | ein Zeichen per Unicode einsetzen; QET schreibt so `[`, damit es nicht als Sonderfeld gelesen wird | 241, 353 |
| `A n` | Ende des Etiketts und Anzahl, mit CR abschließen | 72 |

Weitere Vorgaben aus dem Handbuch, die QET einhält: Zeilenende CR, LF oder CR/LF (S. 7–8); `,` und `;` trennen Parameter, also schreibt QET Dezimalpunkte (S. 8); mindestens 0,5 s zwischen zwei Statusabfragen (S. 30).

Fehlerbuchstaben von `ESC s` übersetzt QET nach der Tabelle auf S. 29–30, z. B. `D` Druckkopf oder Andruckrolle offen, `E` Synchronisationsfehler (kein Etikett gefunden), `F` Farbband zu Ende, `P` Papier zu Ende.

### Aufbau des Auftrags

Je Text ein Block, bei mehreren Spalten je Reihe ein Block:

```
[ESC]oUTF-8;          nur wenn der Block Zeichen außerhalb von ASCII enthält
m m
J
H 50,0,T              nur wenn im Profil „Geschwindigkeit und Heizung setzen“ an ist
S l1;0,0,6,8,12       Sensor; xo,yo; Höhe; Höhe + Lücke; Gesamtbreite
O R                   nur bei „um 180° drehen“
T 0,4.5,0,3,3;[J:c12]-K1
A 3
```

Die Zahlen sind ein Beispiel (Profil 12 × 6 mm, Lücke 2 mm); sie stammen aus dem Test, nicht von einem realen Material.

- Mehrere Spalten (z. B. Karten mit mehreren Schildern nebeneinander) schreibt QET als **ein** Etikett über die ganze Breite (`(Spalten − 1) × Spaltenabstand + Breite`) mit einem `T`-Feld je Spalte. Der Drucker füllt so jede Reihe mit den nächsten Texten. Die Parameter `dx` und `col` des `S`-Befehls nutzt QET nicht, weil das Handbuch nicht beschreibt, wie der Drucker verschiedene Texte auf die Spalten verteilt.
- Gleiche Reihen hintereinander werden ein Block mit größerem `A`.
- Steuerzeichen im Text werden Leerzeichen.

### Drucker und Material einrichten

Unter *Drucker…* legt man jeden Drucker mit ID, Name, Modell, Adresse, Port und der **gerüsteten Artikelvariante** an, dazu das Druckprofil dieses Materials: Breite, Höhe, Lücke, Sensor, Spalten, Spaltenabstand, Ränder links/rechts, Verschiebung x/y, Schriftnummer, Textgröße, Grundlinie, optional Geschwindigkeit/Heizung/Thermotransfer und Drehung. Gespeichert wird in den Einstellungen unter `thm_print/printers`. QET enthält keine Adresse und keine Etikettenmaße: Breite und Höhe 0 lassen keinen Druck zu.

Den Materialkatalog liest QET zur Laufzeit aus einer Weidmüller-Katalogdatei (`my_cat*.mca`), die man unter *Katalogdatei…* wählt. Gelesen wird das Wurzelelement `multiprint_catalog`, darin der `cat_item` mit `title="MultiMark"` und darunter jedes Element mit Materialnummer (`materialno`), Name (`name`) und Beschreibung (`description`). Ob diese Werte im echten Katalog Attribute oder Unterelemente sind, ist nicht belegt; QET liest beides. Herstellerdaten liegen nicht im Repository. Der Test nutzt die erfundene Datei `tests/qttest/fixtures/thm_catalog_example.mca`.

Die Zielwahl richtet sich nach der Artikelvariante:

- genau ein Drucker mit diesem Artikel: er ist vorgewählt;
- mehrere: der Nutzer wählt, vorgewählt ist keiner;
- keiner, kein Drucker eingerichtet oder kein Artikel gewählt: eine Meldung sagt, was fehlt.

### Dialog

- Auswahl wie beim Export: Drähte, Kabel, Klemmen, Geräte; Blätter von … bis; vollständiges Kennzeichen; je Draht- und Kabelende ein Schild; gleiche Texte zusammenfassen; Kopien je Schild.
- Vorschau: Text und Anzahl je Zeile, Summe der Etiketten.
- *Status abfragen* (`v`, dann `ESC s`), *Material abfragen* (`ESC l`, mit Vergleich gegen Höhe + Lücke des Profils; bis 1 mm Abweichung gilt als passend, weil der Drucker ganze Millimeter meldet), *Pause*, *Fortsetzen*, *Auftrag abbrechen…* (nur nach Rückfrage).
- *Als .prn speichern…* schreibt die Bytes in eine Datei, ohne zu senden.
- *Drucken…* zeigt erst eine Zusammenfassung (Ziel, Artikel, Menge, gemeldetes Material) und sendet nur nach „Ja“.
- Zeitgrenzen: 3 s für den Verbindungsaufbau, 3 s für Antwort bzw. Senden. Ein nicht erreichbarer Drucker ergibt eine Meldung mit Adresse und Port.

### Kommandozeile und Skript

```
qelectrotech --thm-printers [--json]
qelectrotech --thm-prn anlage.qet drähte.prn --printer thm1 --kinds wires --folios 2-5 --copies 1
qelectrotech --thm-prn anlage.qet test.prn --width 12 --height 6 --gap 2 --sensor l1
qelectrotech --thm-status thm1 [--json]
qelectrotech --thm-status <adresse>[:port]
qelectrotech --thm-send anlage.qet --material <nummer> --kinds wires
qelectrotech --thm-send anlage.qet --printer thm1 --yes
qelectrotech --thm-send auftrag.prn --printer thm1 --yes
```

- Auswahloptionen wie `--export-marking` (`--kinds`, `--folios`, `--plant`, `--location`, `--one-end`) sowie `--full`/`--short`, `--copies N`, `--no-merge`. Ohne `--kinds` nimmt THM nur Drähte, weil Schilder verschiedener Art auf verschiedenem Material sitzen.
- Profilwerte überschreiben das Profil des Druckers: `--width`, `--height`, `--gap`, `--sensor`, `--columns`, `--pitch`, `--margin`, `--text-size`, `--font`, `--baseline`, `--speed`, `--heat`, `--rotate`.
- `--thm-send` gibt Ziel, Artikel und Menge aus und sendet **nur mit `--yes`**; vorher fragt es den Status ab und sendet nicht, wenn der Drucker offline ist oder einen Fehler meldet. Rückgabewerte: 0 gesendet, 1 keine Verbindung oder Antwort, 2 Aufruf falsch, 3 Drucker offline oder Fehler, 4 nichts gesendet (ohne `--yes`).
- `--thm-status`: 0 online ohne Fehler, 3 offline oder Fehler, 1 keine Antwort.

```js
var n = qet.exportThmJob("C:/temp/draehte.prn", '{"printer": "thm1", "kinds": "wires", "copies": 2}');
var m = qet.exportThmJob("C:/temp/test.prn", '{"profile": {"width": 12, "height": 6, "gap": 2, "sensor": "l1"}}');
```

`qet.exportThmJob` schreibt nur die Datei und gibt die Zahl der Etiketten zurück, bei einem Fehler -1. Es sendet nichts.

### Nicht belegt, am echten Drucker zu prüfen

- Das Handbuch gilt für cab-Drucker mit Firmware 5.48 und sagt selbst, dass cab nicht alle Funktionen auf OEM-Geräten garantiert (S. 7). Ein THM MMP meldete bei einer Abfrage `4.29 Jan 03, 2017 (THM MMP)`, also eine ältere Firmware. Welche Druckergeneration (Ax, X2, X3, X4) ein THM ist und welche Befehle diese Firmware kennt, ist nicht belegt.
- Ein THM MMP antwortete mit `Y-000000Y`, also Z = `Y` („Interpreter aktiv“) bei 0 offenen Etiketten. Das Handbuch nennt Z = `Y` für einen laufenden Auftrag; ob der Drucker dabei druckte, ist nicht bekannt. QET zeigt den Wert, wertet ihn aber nicht als Fehler.
- Ob `v` mit CR, LF oder gar nicht abschließt: QET nimmt die Antwort bis zum ersten Zeilenende oder, falls keines kommt, was bis zur Zeitgrenze kam.
- Senkrechte Lage des Textes: QET setzt die Grundlinie auf (Höhe + Textgröße) / 2. Das nimmt an, dass die Textgröße etwa der Versalhöhe entspricht; das Handbuch zeigt die Grundlinie nur im Bild (S. 226). Die Grundlinie lässt sich im Profil fest einstellen.
- Ob `;` im Text eines `T`-Felds als Text gedruckt wird.
- Die Reihenfolge `H` vor `S` folgt dem Beispiel auf S. 223. Das Beispiel auf S. 223 stellt `O R` vor `S`, der Text auf S. 212 verlangt `O` direkt nach `S`; QET folgt dem Text.
- Ob mehrere Blöcke (`J` … `A`) in einer Verbindung als getrennte Aufträge sauber nacheinander laufen.
- Geschwindigkeiten und Heizwerte des THM: das Handbuch verweist auf die Betriebsanleitung des Druckers.

Ohne Drucker geprüft sind die Bytes des Auftrags, die Auswertung der Antworten (`Y-000000Y`, `Y0037`, Fehlerantworten) und die Zielwahl, mit einem lokalen `QTcpServer` als Gegenstelle (`tests/qttest/tst_thmprint.cpp`).
