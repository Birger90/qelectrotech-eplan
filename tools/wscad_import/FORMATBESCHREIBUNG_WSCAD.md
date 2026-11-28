# WSCAD-Projekt (ELECTRIX / SUITE ab 6.x) – Formatbeschreibung

Belegt an WSCAD ELECTRIX 2025 (Programmversion 7.3, Datenbankversion 6.4) und den drei mitgelieferten Projekten
(Demoprojekt, P&I-Musterprojekt aus SUITE 2018, ProjectWizard), geprüft gegen deren PDF-Exporte. Das Format wurde aus den Metadaten der installierten .NET-Programmteile gelesen
(Methodennamen, IL, Zeichenketten; nichts davon ausgeführt) und an den Dateien nachgeprüft. Umsetzung:
`wscad.py` (Lesen), `wscad_nach_qet.py` (QET-Projekt).

## Projektordner

| Datei | Inhalt |
|---|---|
| `Database/WSCAD6.db` | SQLite: die logische Sicht (Seiten, Symbole, Anschlüsse, Verbindungen, Potentiale, Kabeladern, Texte) |
| `<Struktur>_<Name>.<Nr>.wsELD` | Schaltplanseite (z. B. `=A1+ST_Plan.0001.wsELD`) |
| `.wsFLD`, `.wsCBD`, `.wsTEC`, `.wsCBC`, `.wsPLC`, `.wsMAL`, `.wsRFC`, `.wsSML`, `.wsCSD` … | Fluid, Schrank, Klemmen-, Kabel-, SPS-Plan, Materialliste, BMK-Liste, Inhalt, Deckblatt (Auswertungen; nicht übernommen) |
| `PRJDATA6.db`, `article.db` | Projekteigenschaften, Artikel des Projekts |
| `Settings/*.wsCFG`, `Templates/` | Einstellungen, Vorlagen |
| `Export/*.pdf` | PDF-Export (zum Vergleich) |

## Seitendatei (.wsELD und die anderen .ws*-Seiten)

```
0   4  Dateikennung (01 02 00 00)
4   4  Version (6.4.0.0; Build/Revision 14.208 = Education, 255.255 = Demo)
8   4  Zähler
12  5 × 19 Byte Abschnittseinträge:
       +0 bool benutzt, +1 Index, +2 Art (WSFilePartitionType), +3 uint32 Versatz, +7 uint32 Länge, +11 double
```

Art 1 = Kopf (Benutzer, Base64), Art 2 = Seite, Art 5 = Info (Seiteninhalt, Datum), Art 3 = Vorschaubilder.
Art 2 und 5 sind gepackt (StreamPackService.UpPackDataStream):

```
+0  4  Version            +4  4  Länge ausgepackt
+8  1  Schlüssellänge (32) +9  1  IV-Länge (16)
+10    Schlüssel, IV, 3 Prüfbytes (Schlüssel[0], Schlüssel[1], IV[0])
dann   AES-CBC mit PKCS7 über GZip
```

Schlüssel und IV stehen im Abschnitt selbst: das ist Verschleierung, keine Verschlüsselung mit einem Geheimnis.
Ausgepackt: 4 Byte Version + UTF-8-XML `<WS><Solution><Project><File>…`.

## Seiten-XML

| Knoten | Bedeutung |
|---|---|
| `Header/PaperSize val="297 210"` | Blattgröße in mm |
| `Template` | Rahmen und Schriftfeld: `rect`, `polyline`, `text`, `PH` (Feld mit Wert, Rahmen `wd`/`hg`), `logo/image` |
| `SL` | Symbol: `type="Art.Unterart"`, `x`/`y` Ursprung (= Symbol.xBase/yBase), `ix`/`iy` Einfügepunkt, `hid` = Symbol.SymbolID |
| `SL/line`, `polyline`, `polygon`, `rect`, `kreis`, `text`, `image` | Grafik relativ zum Ursprung |
| `SL/Pin` | Anschluss: `pos`, `connDir` (1 oben, 2 rechts, 4 unten, 8 links), `nr`/`posNR`/`styleNR`, `nm`/`posNM`, `hideNR`/`hideNM` |
| `SL/VPin` | nur Text eines Anschlusses (SPS-Kanalliste) |
| `SL/PHL` | Platzhalter: `xpath="../P[@id='BMKZ']"`, `style`, `hide` (vorhanden = unsichtbar), `wd`/`hg` |
| `SL/DataBlock params="QV,False;Funktionstext,True;…"` | Zeilen unter dem BMK; `True` = ausgeblendet; Zeilenabstand 1,2 × size |
| `SL/QV` | Querverweis; mehrere `P id="QV" lineid=…` untereinander |
| `SL/PinGP` | Anschlussgruppe (Kontaktgruppe, SPS-Kanal) mit eigenen `P`, `PHL`, `PH`, `QV` |
| `CN/polyline` | gezeichneter Leitungszug |
| `Cableline` | Kabelstrich |
| `text` (direkt unter File) | freier Text |

Koordinaten in mm, **y nach unten**, zwei Nachkommastellen.

### Symbolarten (`SL type`, Teil vor dem Punkt)

1 Standard, 2 Spule/Hauptfunktion, 3 Kontakt, 4 Klemme, 6 Grafik, 7 Master, 10 Verbindungssymbol (Winkel, T,
Brücke, Kreuzung), 11 Kabelschirm, 12 Stecker, 14 Pfeil mit Ziel (Abbruchsymbol), 15 SPS-Karte, 16 SPS-Kanal,
19 Buchse, 20 Kabelader, 23 Leitungsattribut (Drahtnummer, Potentialname).

### Texte (`style`)

`size` Schriftgröße in mm (Vorgabe 2,5; das Geviert im PDF ist 0,933 × size), `winkel` Grad gegen den
Uhrzeigersinn, `align` Bits, am PDF geeicht (≈ 1.000 Texte):

| Bits | Bedeutung |
|---|---|
| 2 | rechtsbündig |
| 4 | zentriert (4+2 ebenfalls) |
| 8 | Anker unten (Grundlinie); mehrzeilig: erste Zeile am Anker |
| 16 (ohne 8) | Anker in der Mitte |
| weder 8 noch 16 | Anker oben |
| kein `align` | Grundlinie links |

`PH`/`PHL` mit `wd`/`hg`: Ausrichtung im Rahmen; `tc:5` schneidet am Rahmen ab, `tc:6` bricht in Zeilen um.
Zeilenwechsel in Texten: Zeilenumbruch oder `\a`.

### Bögen

`kreis` mit `as`/`ag`: Startwinkel und Spanne in Grad (GDI: im Uhrzeigersinn bei y nach unten).
`type="KreisH" qd=…`: Halbkreis, `qd` = Viertel-Bits (3 = 0–180°, 6 = 90–270°, 12 = 180–360°, 9 = 270–90°).

### SPS-Kanaladresse (`P type="WSPlcChannelAddress"`)

`0#|#True#|#True#|#EN|T|I|L|DE|T|E#|##|#0#|#7#|#1#|#.#|#` → Felder `#|#`: 1 Bitadressierung, 3 Präfix je Sprache
(`Sprache|T|Präfix`, mit `|L|` getrennt), 5 Byte, 6 Bit, 8 Trenner → „E0.7“; ohne Bitadressierung „AEW1“.

## Datenbank (WSCAD6.db)

| Tabelle | Spalten (Auszug) |
|---|---|
| `Page` | PageID, FileName, FileType (wsELD …), Width, Height, AspF (=), AspL (+) |
| `Symbol` | SymbolID, ParentID, ItemType, BMK (`=A1+ST-K1`), PageID, x, y, xBase, yBase |
| `Pin` | SymbolID, PinID, Name, Number, x, y (relativ zu xBase/yBase) |
| `Connection` | SymbolID1, Pin1, SymbolID2, Pin2, PageID, Colorcode, Dimension, Potential, WireName, CableID |
| `Data`, `String` | Eigenschaften je ID und Typ (`Types`: 57 Artikel, 63 Hersteller, 97 BtName …) |

- `Pin.PinID` zählt die `<Pin>` des Symbols in Dateireihenfolge (am Demoprojekt bei allen 752 Symbolen mit gleich
  vielen Anschlüssen); die relative Lage stimmt immer. `xBase/yBase` weicht bei gedrehten Pfeilen vom Seitenursprung
  ab – Lage daher aus der Seite.
- `Connection` nennt die logischen Paare über Winkel und T hinweg; Brücken (Klemmen, Verteiler) stehen als
  vollständiges Netz (jede mit jeder).

## Übersetzung nach QET (wscad_nach_qet.py)

- Blatt je Seite, A4/A3 quer, Maßstab 96 dpi, Ursprung bei Szene (5, 5) (QETs `Diagram::margin`); Rahmen und
  Schriftfeld als Blattgrafik, Seitentitel, Blatt, Anlage und Ort in die Blatteigenschaften.
- Symbol mit `hid` und Anschlüssen → Bauteil (eigene Grafik, Texte, Anschlüsse in WSCAD-Richtung) in
  `company://WSCAD-Import/`; BMK als Kennzeichen; Artikel/Hersteller/Bezeichnung/Funktionstext als
  Elementinformation. Rechtsbündige Kennzeichen liegen im Schaltbild um ihre Breite rechts (QET hält beim Setzen die
  rechte Kante ab dem leeren Text fest).
- Leitungen: `Connection` je Seite; Weg über das gezeichnete Netz (Anschlüsse, die sich in Anschlussrichtung
  gegenüberstehen, Winkel/T-Symbole, `CN`); das 1,25-mm-Schrägstück der Winkel wird zur Ecke. Je Blatt ein Spannbaum
  (erst Verbindungen mit Weg, dann die kürzesten).
- Pfeile mit Ziel → verknüpfte QET-Verweise. Gepaart wird vorab über alle Seiten nach Leitungsname: zuerst Paare,
  deren Querverweise aufeinander zeigen (`/2.0` = Blatt 2 derselben Struktur, `=A1+P1/3.2` eine andere), dann der
  Querverweis des Pfeils, dann das nächste spätere Blatt. Gehend ist der Pfeil auf dem früheren Blatt – die Richtung
  im Symbolnamen taugt nicht (ältere Projekte ohne Namen, alle „Richtung rechts“). Farbe, Querschnitt und Potential
  über Verweispaare einheitlich, sonst verweigert QET die Verknüpfung.
- Schrift Roboto Mono wie im WSCAD-PDF (Laufweiten aus der installierten Schrift; fehlt sie, Tahoma).
- Logo im Schriftfeld: Bild mit Maß 0 (SUITE 2018) nimmt den Rahmen des `<logo>`.
- Firmensammlung: Artikel (`Artikel`/`Hersteller` am Symbol, sonst `article.db`: article.artManufacturer =
  adress.adrAutoValue), die die Sammlung noch nicht kennt (Index nach Bestellnummer, `eplan_bibliothek.Sammlung`),
  werden ein sauberes Bauteil (Grafik, Anschlüsse, Kennzeichen, Artikeldaten; keine Plantexte) unter
  `Hersteller/Kategorie/Bestellnummer/<Nr>_WSCAD-Schaltbild_<Symbol>.elmt`; Hersteller auf vorhandene Ordner
  abgeglichen („SIEMENS“, „Siemens AG“ → Siemens, „PHOENIX“ → Phoenix Contact), ohne Hersteller „Sonstige“.
  `--ohne-sammlung` schaltet das ab.
- Spule (2) / Master (7) und Kontakte (3) gleichen BMK → Master/Slave.
- Daneben `<Name>_Geraete.csv` und `<Name>_Verbindungen.csv`.

## Offen

- PE-Leitungen zeichnet WSCAD gestrichelt (Leitungsart des Potentials); im Import durchgezogen.
- SPS-Karte (15) und -Kanäle (16) werden Bauteile, aber noch nicht als SPS-Master/-Kanal verknüpft.
- Leitungsattribute (23: Drahtnummer, Potentialname) nicht als Leiternummer übernommen.
- `.wsPRAX` (gepackte Projektsicherung) wird noch nicht gelesen, nur der Projektordner.
- Fluid-, Schrank- und Auswertungsseiten werden nicht übernommen.
