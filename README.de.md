![](logo.png)

# Dieser Fork: QElectroTech für EPLAN-Pläne und Herstellerdaten

🇬🇧 [English version](README.md) · Allgemeine Beschreibung von QElectroTech: [README.md](README.md#qelectrotech)

Dieser Fork erweitert QElectroTech so, dass sich EPLAN-Schaltpläne als echte,
verknüpfte Projekte nachbauen lassen – sie navigieren wie ein EPLAN-PDF und tragen ihre Daten so, wie EPLAN es tut –
und dass Herstellerbauteile und -kabel direkt nutzbar sind. Der Upstream
([qelectrotech-source-mirror](https://github.com/qelectrotech/qelectrotech-source-mirror)) wird regelmäßig
eingemischt; alles Folgende gibt es nur hier.

## Bauteil-Import

Menü „Bauteil-Import“, ohne Oberfläche: `--import-eplan`

- EPLAN-Makros und EDZ-Pakete: Grafik aus DXF, Schaltbild aus `.ema`, Anschlüsse und Artikeldaten; Black-Box-Rahmen,
  Kanalnamen und zerstückelte Linien werden bereinigt. Anschlussbezeichnungen werden zeilenweise geteilt, die
  Anschlussbeschreibung kommt aus der richtigen EPLAN-Eigenschaft.
- DWG-Zeichnungen (AutoCAD 2004–2018) direkt, ohne Konverter; Aufbauansichten im Maßstab der EPLAN-Aufbaumakros.
- Artikeldaten aus WSCAD-Paketen (`.wspak`).
- Suche und Import direkt aus dem EPLAN Data Portal (Herstellerliste, Filter, Bildvorschau).

## Navigation wie bei EPLAN

In QET und im exportierten PDF:

- EPLAN-Dreifachsprung am Bauteil: linkes Drittel vorwärts durch die Sprungfolge des Geräts, Mitte zur
  Hauptfunktion, rechtes Drittel rückwärts (in QET Strg+Doppelklick, im PDF drei Linkbereiche).
- Freie Texte, Bauteile und Kabel können auf ein Blatt oder ein Bauteil zeigen (`destination`): Zeilen von Klemmen-,
  Kabel- und Stücklisten springen zum Gerät; Strg+Klick folgt, der PDF-Link zoomt darauf.
- Ein verknüpfter Folioverweis ist selbst ein Link zum Gegenpfeil, nicht nur sein Text. Gefüllte Verweispfeile im
  EPLAN-Aussehen.

## Daten wie bei EPLAN statt Handtext

- SPS: Die Karte (SPS-Master) führt ihre E/A-Tabelle (Adresse, Funktionstext, Kommentar, Klemmen); jeder Kanal ist
  an seine Zeile gebunden (`group_index`) und zeigt die Werte über seine Felder – Tabelle ändern, die Kanäle ziehen
  nach.
- Kontakte: Eine platzierte Spule kann eigene Kontaktgruppen tragen (11/12/14, 21/22/24 …), wie die
  Funktionsschablone eines EPLAN-Artikels; die Kontakte übernehmen daraus ihre Anschlussbezeichnungen, der
  Kontaktspiegel zeigt sie.
- Querverweisformel `%=M`: Die Anlage erscheint nur, wenn sie von der des eigenen Blatts abweicht (`/42/1`,
  `=DC/38/2`), wie bei EPLAN.

## Blätter und Schriftfeld

- Hellgraue Stempel über dem Blatt wie bei einem EPLAN-Formular: Schriftfeld-Variablen `watermark` (schräg, z. B. das
  Druckdatum) und `watermark-side` (senkrecht am rechten Rand); leer = aus, je Blatt oder als Projektvorgabe.
- Neue Schriftfeld-Variablen `%{printdate}` und `%{saveddate-de}` (TT.MM.JJJJ).
- Skript-API: `outsideSheet(folio)` listet, was außerhalb des Blatts liegt.

## Kabel

- Menü Projekt > *Kabelliste (Typen wählen)…*: alle Kabel des Projekts mit Typ, Adern, Farben, Anlage, Ort und
  Blättern; *Typ wählen…* ändert den Typ in einem Rückgängig-Schritt.
- Herstellerkataloge: Jede CSV im Ordner `cable-catalogs` neben der Kabeltypdatei wird als schreibgeschützter
  Katalog gelesen. Filter nach Katalog, Aderzahl, Aderquerschnitt, Energiekettentauglichkeit und Eigenschaften
  (geschirmt, halogenfrei, UL/CSA, torsionsfest …).
- `tools/kabelkatalog/kabelkatalog_eplan.py` erzeugt solche Kataloge aus den frei erhältlichen EPLAN-Artikeldaten der
  Hersteller (z. B. igus chainflex, LAPP): Adern mit Farben, Querschnitt, Außendurchmesser, Spannung, Artikelnummer.

## Werkzeuge

In `tools/symbole/`:

- `pdf_symbol.py` / `pdf_blatt.py`: Symbole bzw. ganze Blätter 1:1 aus einem Vektor-PDF (z. B. EPLAN-Plan) übernehmen,
  mit echten Leitern und Knoten.
- `verknuepfung_setzen.py`: Master/Slave-Verknüpfungsart importierter Herstellersymbole nach ihrer Rolle setzen
  (Spule, Kontakt, Schutzschalter, SPS-Baugruppe, SPS-Kanal, Gerätepin).
- `anschluesse_ergaenzen.py`, `nachzeichnen_symbole.py`: Anschlüsse ergänzen, eigene Symbole nachzeichnen
  (`bauteile/nachgezeichnet/`).

## Für KI-Assistenten

Zwei Skills unter `.claude/skills/` (Claude Code lädt sie im Klon automatisch; andere Werkzeuge: die jeweilige
`SKILL.md` lesen, sie verweist auf ihre Unterdateien):

- `qet-schaltplan-zeichnen` – wie man EPLAN-artige Blätter per Skript zeichnet, prüft und verknüpft.
- `eplan-wissen` – die EPLAN-Plattform-Hilfe 2.9 in eigenen Worten (PDF-Sprünge, Querverweise, Kennzeichnung,
  Klemmen, Kabel, SPS, Auswertungen, Makros/Artikel, Eigenschafts-IDs, Prüfmeldungen …), jede Regel mit Quell-URL und
  „Bedeutung für QET“. Einstieg: `.claude/skills/eplan-wissen/SKILL.md`.

Herstellerdateien selbst sind nicht enthalten (Nutzungsbedingungen der Portale).
