# Konfigurator: Plan aus einer Tabelle erzeugen

Vorbild ist EPLAN EEC Pro (Engineering Configuration): Ein Plan entsteht nicht durch Kopieren alter Blätter, sondern
aus einer Tabelle der Objekte einer Maschine (Antriebe, Ventile, Sensoren mit ihren Kennwerten) und einem Satz
wiederverwendbarer Schaltungsteile, die je nach Kennwert ihre Variante wählen. Übernommen ist nur das Prinzip, nichts
Proprietäres: kein EEC-Dateiformat, keine EEC-Formelsprache, keine EPLAN-Makros.

Fachquelle für das Prinzip: Canary-ECAD/eec-pro-skill und Canary-ECAD/eplan-scripting-skill (`eec-typicals.md`),
beide MIT. Was dort beschrieben ist und wie es hier abgebildet wird:

| EEC Pro | Hier |
|---|---|
| Mechatronisches Modell: Objekte mit Parametern | Konfigurationstabelle: eine Zeile je Objekt, Spalten = Kennwerte |
| Typical: kapselt Makros hinter Varianten und Bedingungen | Funktionsvorlage (`.qetmak`) mit `<variant when="…">` |
| Bedingung im Makronamen (`_VOLTAGE=400`, `Y(…)` UND, `O(…)` ODER) | Ausdruck im Attribut `when`: `P_kW > 5.5 and U = 400` |
| Formeln auf Parametern | `<formula name="…" expr="…"/>` in der Vorlage |
| Generierung Struktur → Projekt in einem Lauf | ein Lauf: Tabelle → Varianten → Formeln → Zähler → Blätter → SPS-Übersicht → Listen |
| Werte nur auf dem Modell setzen, nicht auf der ECAD-Kopie (sonst beim nächsten Lauf weg) | Neuerzeugen erkennt Handänderungen je Blatt und überschreibt sie nicht still |
| Seitenzähler aus der Tabelle statt nachträglicher Umnummerierung | Blätter in Tabellenreihenfolge, BMK und Adressen bleiben beim Neuerzeugen stabil |
| Erfolgsmeldung ist kein Beweis | jeder Lauf schreibt einen Bericht: erzeugt, behalten, Konflikt |

## 1. Die Konfigurationstabelle

Excel (`.xlsx`, erstes Tabellenblatt) oder CSV (Trennzeichen `;`, `,` oder Tab, UTF-8). Zeile 1 sind die
Spaltennamen, jede weitere Zeile ist ein Objekt. Leere Zeilen und Zeilen, deren erste Zelle mit `#` beginnt, zählen
nicht.

| Spalte | Pflicht | Bedeutung |
|---|---|---|
| `Objekt` | ja | Name des Objekts, eindeutig in der Tabelle („Förderband“). Er ist der Schlüssel beim Neuerzeugen. |
| `Typical` | ja | Name oder Pfad der Funktionsvorlage („Antrieb“, „Ventile/Ventil mit DQ-Kanal“) |
| `BMK` | nein | Betriebsmittelkennzeichen; leer: das nächste freie nach dem Vorgabewert der Vorlage |
| `Anlage`, `Ort` | nein | `=`- und `+`-Kennzeichen des Blatts |
| `Blatt` | nein | Gruppe: Zeilen mit demselben Wert kommen nebeneinander auf ein Blatt |
| `Produkt` | nein | Produktnummer statt einzelner Kennwerte (Abschnitt 1a); `Typical` darf dann fehlen |
| alle weiteren | nein | Werte der Parameter der Vorlage, Spaltenname = Parametername (`P_kW`, `TEXT`, `X`, `W_TYP` …) |

Groß-/Kleinschreibung der Spaltennamen zählt nicht; Zahlen dürfen Komma oder Punkt haben. Eine Spalte, die die
Vorlage nicht kennt, wird im Bericht genannt und nicht still übergangen.

Beispiel (`examples/konfigurator/maschine.csv`):

```
Objekt;Typical;Anlage;Ort;P_kW;TEXT
Förderband;Antrieb;=FB;+M1;3;Förderband
Hauptpumpe;Antrieb;=HY;+M2;11;Hydraulikpumpe
Zylinder ausfahren;Ventil;=HY;+M2;;Zylinder 1 aus
Endlage vorn;Sensor;=HY;+M2;;Zylinder 1 vorn
```


## 1a. Zweiter Eingang: Produktnummer

Nach der Idee „product macros“ (Canary-ECAD/eplan-scripting-skill, `docs/ideas/product-macros.md`, MIT; nur die
Idee übernommen): Statt einzelner Kennwerte trägt eine Zeile in der Spalte `Produkt` eine Produktnummer wie
`MS-400-7K5-D`. Eine Regeldatei je Familie zerlegt sie:

```json
{
  "family": "MS", "separator": "-",
  "segments": [
    {"name": "U", "label": "Spannung", "codes": {"400": {"values": {"U": "400"}}}},
    {"name": "P", "label": "Leistung", "codes": {"7K5": {"values": {"P_kW": "7.5"}}, "11K": {"values": {"P_kW": "11"}}}},
    {"name": "ANL", "label": "Anlauf", "optional": true, "codes": {"": {}, "D": {"label": "Direktstart"}}}
  ],
  "rules": [
    {"template": "Typicals/Antrieb"},
    {"when": {"ANL": "D"}, "template": "Typicals/Antrieb", "variant": "Direktstart"}
  ]
}
```

- **Ablage:** `<Familie>.product.json` in einem Vorlagenordner (mitgeliefert: `funktionsvorlagen/Produkte/MS.product.json`;
  eigene im Vorlagenordner des Nutzers, dessen Familie gewinnt).
- **Segmente:** in der Reihenfolge der Nummer nach dem Familienkürzel; optionale nur am Ende. Jeder Code ist ein
  **Wertesatz** (`values`).
- **Regeln:** Eine Regel gilt, wenn jeder ihrer `when`-Codes in der Nummer steht; Regeln ohne `when` gelten nur,
  wenn keine andere gilt. Sie bestimmen **Vorlage**, **Variante** (über den reservierten Wert `VARIANTE`, der die
  Variante gleichen Namens wählt, unabhängig von ihrer Bedingung) und weitere Werte. Zwei geltende Regeln, die
  Verschiedenes sagen, sind ein Fehler – keine Reihenfolge entscheidet.
- **Bestimmt:** Ein unbekannter Code ist ein Fehler mit den erlaubten Codes, nie eine Vermutung. Dieselbe Nummer
  ergibt immer dieselbe Schaltung (die Zerlegung ist reine Regelauswertung).
- **In der Tabelle:** Spalte `Produkt` (die Spalte `Typical` darf dann fehlen). Werte, die die Zeile selbst nennt
  (z. B. `TEXT`, `I_N`), gehen vor; eine abweichende Spalte `Typical` geht vor und wird gemeldet.
- **Prüfung der Vorlagensammlung gegen die Regeln:** `qelectrotech --check-product-rules [<datei>] [--json]` (MCP
  `qet_product_rules`) zerlegt jede Nummer, die die Regeln erlauben (bis 5000), und rechnet sie durch die Vorlagen.
  *Fehler:* Vorlage fehlt, Variante gibt es nicht, Regeln widersprechen sich, Regel nennt unbekanntes Segment oder
  unbekannten Code, keine Variante passt. *Warnung:* Wert, den keine Vorlage kennt; Code setzt einen Wert leer;
  Meldungen der Tabellen (z. B. Strom unter dem Bereich). *Hinweis:* Regel gilt für keine Nummer. Exit 1 bei Fehlern.
  `--decode-product <nummer> [--json]` zeigt, was eine Nummer wird.
- Beispiel: `examples/konfigurator/maschine_produkte.csv` (MS-400-4K0 → Direktstart, MS-400-11K → Stern-Dreieck,
  MS-400-7K5-D → Direktstart erzwungen). Die Familie MS kennt nur 400 V, weil die Auswahltabellen der Vorlagen für
  400 V gelten; das Typical *Antrieb* hat dafür die Bedingung `U = 400`.

## 2. Typicals, Varianten und Bedingungen

Ein Typical ist eine Funktionsvorlage. Neu im Block `<function_template>`:

```xml
<function_template version="2" name="Antrieb" category="Typicals">
  <parameter name="P_kW" label="Leistung" unit="kW" default="4"/>
  <table name="3RV2" file="../Tabellen/siemens_3RV2_direktstart.csv" key="I_max"/>
  <formula name="QA_ART" expr="lookup('3RV2', I_N, 'QA_ART')"/>
  <variant name="Stern-Dreieck" when="P_kW > 5.5" template="Motoren/Motorabgang Stern-Dreieck"/>
  <variant name="Direktstart" template="Motoren/Motorabgang DOL"/>
</function_template>
```

- **Variante:** Die Varianten werden von oben nach unten geprüft; die erste, deren `when` wahr ist (oder die kein
  `when` hat), wird eingefügt. Eine Variante verweist auf eine andere Vorlage (`template`, Name oder Pfad relativ zum
  Vorlagenordner) und reicht ihr alle Werte weiter; die Zielvorlage darf selbst Varianten haben (höchstens acht
  Stufen, eine Schleife wird gemeldet). Ein Typical, das nur aus Varianten besteht, braucht keine eigene Zeichnung.
- **Bedingung:** ein Ausdruck der Formelsprache (Abschnitt 3), der wahr oder falsch ergibt. Die Varianten werden
  nach den Formeln des Typicals geprüft, eine Bedingung darf also errechnete Werte nutzen.
- Dieselbe Auswahl gilt in der Seitenleiste *Funktionsvorlagen* und bei `insertTemplate`: „Antrieb“ mit 11 kW
  eingefügt ergibt den Stern-Dreieck-Abgang.

## 3. Formeln

`<formula name="X" expr="…"/>` setzt den Platzhalter `{X}`. Reihenfolge beim Auflösen: Eingabewerte und Vorschläge,
Auswahltabellen (`key`), dann die Formeln in Dateireihenfolge; eine Formel sieht alle Werte davor. Ein Wert, der für
den Namen der Formel ausdrücklich gegeben ist (Tabellenspalte), hat Vorrang: die Formel liefert nur, was fehlt.

| Element | Schreibweise |
|---|---|
| Zahlen, Texte | `5.5`, `5,5` (nur außerhalb von Funktionsargumenten), `'Direkt'`, `"Direkt"` |
| Rechnen | `+ - * / ^`, Klammern; `+` mit einem Text hängt an (`'-Q' + NR`) |
| Vergleich | `= == != <> < <= > >= ≤ ≥` (Zahlen als Zahlen, sonst Text ohne Groß-/Kleinschreibung) |
| Logik | `and or not` bzw. `&& \|\| !` |
| Funktionen | `if(b; a; c)`, `min`, `max`, `round(x; n)`, `ceil`, `floor`, `abs`, `sqrt`, `text(x; n)` (n Nachkommastellen) |
| Tabellen | `lookup('tabelle'; wert; 'spalte')`: Zeile mit dem kleinsten Schlüssel ≥ wert (nächstgrößere Stufe), bei Text die gleiche; `has(NAME)`: ob ein Wert gesetzt ist |

Argumente trennt `;` oder `,` (ein Komma direkt zwischen Ziffern ist das Dezimalkomma: `round(x; 2)` oder
`round(x, 2)`, aber `5,5` ist eine Zahl).

Tabellen stehen in der Vorlage (`<row …/>`) oder als CSV daneben (`file="…"`, Pfad relativ zur Vorlage). Eine
Tabelle aus Datenblatt oder Norm trägt eine Spalte `Quelle`. Mitgeliefert: `funktionsvorlagen/Tabellen/
siemens_3RV2_direktstart.csv` – Motorschutzschalter 3RV2 und Schütz 3RT2 je Einstellbereich, Werte wörtlich aus der
Siemens-Auswahlhilfe „SIRIUS Systembaukasten“ (Seite 4–5, Lastabzweige für Direktstart, CLASS 10, 400 V).
Schutzorgan aus Motorstrom: `lookup('3RV2'; I_N; 'QA_ART')` nimmt den kleinsten Schutzschalter, dessen
Einstellbereich bis `I_N` reicht; liegt `I_N` unter dessen Untergrenze (`I_min`), meldet der Lauf das.

## 4. Zähler

- **BMK:** Parameter `kind="label"` zählen wie bisher auf das nächste freie Kennzeichen (`-MA1`, `-MA2` …), über alle
  Zeilen eines Laufs hinweg. Ein BMK in der Tabelle gilt wie gegeben.
- **SPS-Adressen:** neue Parameterart `kind="address"` mit `io="DI|DQ|AI|AQ"` und `count="n"` (so viele Kanäle
  nacheinander, z. B. Stern-Dreieck: Netz, Stern, Dreieck = 3). Der Generator vergibt den nächsten freien Kanal der
  Art in Tabellenreihenfolge (`%I0.0`, `%I0.1` … für Eingänge, `%Q…` für Ausgänge); in der Seitenleiste wird der
  nächste freie Kanal der SPS-Module des Projekts vorgeschlagen. `{ADR+1}` ist der folgende Kanal (`%Q0.7` + 1 =
  `%Q1.0`).
- **Klemmen, Kabel:** wie bisher (`kind="terminal"`, `kind="cable"`).

## 5. Was erzeugt wird

1. **Schaltplanblätter** in Tabellenreihenfolge. Aufeinanderfolgende Objekte gleicher Vorlage und gleicher
   Anlage/Ort teilen sich ein Blatt nebeneinander, je auf der nächsten Spalte, solange sie in den Rahmen passen
   (zwei Direktstarter, drei Sensoren); ein Stern-Dreieck-Abgang füllt ein Blatt allein. Die Spalte `Blatt`
   erzwingt eine Gruppe. Titel: Name des Typicals und die Objekte mit BMK („Antrieb: -MA1 Förderband, -MA2 Lüfter“),
   Anlage und Ort aus der Tabelle.
2. **Hausstil:** Der SPS-Kanal steht beim Gerät im Strompfad (er ist Teil der Vorlage). Die SPS ist eine Station auf
   dem Blatt **SPS-Übersicht**: je Kanalart Module zu 16 Kanälen (`-KF1` DI 16, `-KF2` DQ 16 …) mit Kanaltabelle und
   Querverweis zu jedem Gerät; jeder Kanal im Strompfad ist mit seinem Modulkanal verknüpft (Querverweis zurück).
3. **Auswertungen** am Ende, als Tabellen auf eigenen Blättern, die der Plan aktuell hält: **Bestellliste**
   (Geräte mit Artikel), **Klemmenplan** (je Klemme Leiste, Nummer, Ziele) und **Kabelliste** (Kabel, Typ, Adern,
   Blatt). Lange Listen gehen auf Folgeblätter.

## 6. Neu erzeugen, ohne Handänderungen zu überschreiben

Der Lauf legt im Projekt einen Block `<configurator>` ab (wie die Revisionen: ein fremder Block der Projektdatei):

- die Quelle (Name der Tabelle) und je Objekt: Prüfsumme der Zeile, Vorlage und gewählte Variante, vergebenes BMK
  und vergebene SPS-Adressen, das Blatt, auf dem es steht;
- je erzeugtem Blatt seinen **Fingerabdruck** wie bei der Revisionsverwaltung (`QetRevision::fingerprint`: jedes
  Symbol, jede Leitung, jeder Text mit Lage und Inhalt) und die Kennung, ob es ein Schaltplan-, Übersichts- oder
  Listenblatt ist.

Beim nächsten Lauf auf dasselbe Ziel:

| Lage | Was passiert |
|---|---|
| Objekt unverändert | Blatt bleibt, wie es ist – auch mit Handänderungen |
| Objekt geändert, Blatt seit dem letzten Lauf nicht von Hand geändert | Blatt wird an derselben Stelle neu erzeugt |
| Objekt geändert **und** Blatt von Hand geändert | **Konflikt:** das alte Blatt bleibt unberührt, das neue kommt direkt dahinter mit „[neu]“ im Titel; der Bericht nennt jede Handänderung („Blatt 3: Symbol -QA1 geändert“). Erst `--overwrite` ersetzt es. |
| Objekt neu | neues Blatt am Ende der Schaltplanblätter |
| Objekt entfernt | Blatt fällt weg, wenn unverändert; mit Handänderung bleibt es und wird gemeldet |

Teilen sich mehrere Objekte ein Blatt, ist das Blatt die Einheit: ändert sich eines, wird das Blatt neu erzeugt.
BMK, SPS-Adressen, Klemmen- und Kabelnummern bleiben die alten (aus dem Block), neue Objekte bekommen freie. So verschiebt eine neue Zeile in
der Mitte keine Kennzeichen der folgenden. SPS-Übersicht und Auswertungen sind reine Erzeugnisse: sie werden jedes Mal
neu gebaut (eine Handänderung daran wird gemeldet). Vor dem Überschreiben bleibt die alte Datei als
`<name>.vor-konfigurator.qet` daneben liegen.

## 7. Bedienung

- **Menü:** *Projekt > Plan aus Konfiguration erzeugen…*: Tabelle wählen, Ziel wählen (neues Projekt oder
  vorhandene Datei), Erzeugen; danach öffnet sich das Projekt, und der Bericht erscheint.
- **Kommandozeile:** `qelectrotech --generate-from-config <tabelle.xlsx|csv> <ziel.qet> [--overwrite] [--json]`.
  Exit 0 erzeugt, 3 erzeugt mit Konflikten, 1 Fehler, 2 Aufruf falsch.
- **MCP:** Werkzeug `qet_generate_from_config` (Tabelle, Ziel, overwrite) in beiden MCP-Kopien.

## 8. Grenzen von Stufe 1

- Eine Ebene Objekte (keine verschachtelten Baugruppen wie in EEC Pro), keine Formularoberfläche für die Tabelle –
  sie bleibt in Excel.
- SPS-Module sind allgemeine Module zu 16 Kanälen, kein Herstellerfabrikat. Wer eine bestimmte Station will, legt
  deren Module vorher auf ein Blatt des Zielprojekts: Kanäle mit passender Adresse werden mit ihnen verknüpft, nur für
  fehlende Kanäle legt der Generator allgemeine Module an.
- Kein Schrankaufbau aus der Tabelle.
- Die Klemmleisten der Vorlagen heißen nur `X1`, `X2` … ohne Anlage: Objekte verschiedener Anlagen zählen auf
  derselben Leiste weiter (=FB … X1:1–8, =HY … X1:9 …). BMK zählen dagegen je Anlage (=FB-MA1 und =HY-MA1).
- Analogkanäle (AI/AQ) zählt der Generator noch nicht; ihre Adresse kommt aus der Tabelle.
- Die Siemens-Auswahlhilfe nennt für 3RV2021-4AA10/-4BA10/-4CA10/-4DA10 auf Seite 5 und 6 verschiedene
  Einstellbereiche; die Tabelle nimmt Seite 5 (Direktstart). Vor der Bestellung im Produktdatenblatt prüfen.

## 9. Geprüft

`python tools/konfigurator/test_konfigurator.py` (headless, Kommandozeile): Beispiel mit vier Antrieben (3 und
0,75 kW Direktstart, 11 und 7,5 kW Stern-Dreieck), zwei Ventilen, drei Sensoren → 5 Schaltplanblätter, SPS-Übersicht
mit KF1 (DI) und KF2 (DQ), 13 verknüpfte Kanäle (Stern-Dreieck über die Bytegrenze `%Q0.5`–`%Q0.7`), Bestellliste,
Klemmenplan, Kabelliste; Neuerzeugen unverändert (alles bleibt), mit geänderter Zeile (ein Blatt neu, Nummern
bleiben, keine Reste in den Listen), mit Handänderung (Konflikt, Exit 3, mit `--overwrite` ersetzt, Kanäle wieder
verknüpft); Formeln und Bedingungen an einer eigenen Testvorlage; `qet_layout_check` Note 100 ohne Treffer
(Kreuzungen zählt die Prüfung nur als Information); PDF.
