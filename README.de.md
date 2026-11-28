![](logo.png)

# Dieser Fork: QElectroTech für EPLAN-Pläne und Herstellerdaten

🇬🇧 [English version](README.md) · Allgemeine Beschreibung von QElectroTech: [README.md](README.md#qelectrotech)

Dieser Fork erweitert QElectroTech so, dass sich EPLAN-Schaltpläne als echte,
verknüpfte Projekte nachbauen lassen – sie navigieren wie ein EPLAN-PDF und tragen ihre Daten so, wie EPLAN es tut –,
dass sich Pläne aus WSCAD, sPlan und FluidSIM übernehmen lassen und dass Herstellerbauteile und -kabel direkt nutzbar
sind. Der Upstream
([qelectrotech-source-mirror](https://github.com/qelectrotech/qelectrotech-source-mirror)) wird regelmäßig
eingemischt; alles Folgende gibt es nur hier.

## Pläne aus anderen Programmen übernehmen

Alle Importe auf einen Blick; Einzelheiten stehen in den Abschnitten darunter.

- **EPLAN-Projekt** (`.zw1`) und **EPLAN-Export** (`.epdz`, eView): Datei > *EPLAN-Projekt importieren* – ein
  verknüpftes, bearbeitbares Projekt mit Bauteilen, Leitungen, Querverweisen und Listen
  ([EPLAN-Projekte](#eplan-projekte-und-eingebauter-assistent)).
- **FluidSIM-Schaltung** (`.circ`): Datei > *FluidSIM-Schaltung importieren* – Pneumatik, Hydraulik, Elektrik und
  GRAFCET, sofort simulierbar ([Pneumatik, Hydraulik und Simulation](#pneumatik-hydraulik-und-simulation)).
- **sPlan-Plan** (`.spl8`, Abacom): Datei > *sPlan-Plan importieren* – neu gezeichnet als QET-Projekt; Netze aus der
  Zeichnung, Gerätekästen und SPS-Kanäle aus einer Zuordnungsdatei zu Herstellermakros
  ([tools/splan_import](tools/splan_import/README.md)).
- **WSCAD-Projekt** (Projektordner mit `Database/WSCAD6.db` und `.wsELD`-Seiten, ELECTRIX/SUITE ab 6.x): Datei >
  *WSCAD-Projekt importieren* – ein bearbeitbares Projekt mit WSCAD-Rahmen und Schriftfeld, Bauteilen, Leitungen,
  verknüpften Verweisen und Master/Slave; fehlende Artikel kommen sortiert in die Firmensammlung
  ([EPLAN-Projekte](#eplan-projekte-und-eingebauter-assistent),
  [Formatbeschreibung](tools/wscad_import/FORMATBESCHREIBUNG_WSCAD.md)).
- **Bauteile**: EPLAN-Makros und EDZ, Hersteller-Makropakete (z. B. Beckhoff), DWG, WSCAD-Artikeldaten (`.wspak`),
  Verwaltungsschalen (`.aasx`) und das EPLAN Data Portal ([Bauteil-Import](#bauteil-import)).

## Bauteil-Import

Menü „Bauteil-Import“, ohne Oberfläche: `--import-eplan`

- EPLAN-Makros und EDZ-Pakete: Grafik aus DXF, Schaltbild aus `.ema`, Anschlüsse und Artikeldaten; Black-Box-Rahmen,
  Kanalnamen und zerstückelte Linien werden bereinigt. Anschlussbezeichnungen werden zeilenweise geteilt, die
  Anschlussbeschreibung kommt aus der richtigen EPLAN-Eigenschaft. Die Makrografik kommt vollständig mit (Linien,
  Bögen, Texte, eingebettete Zeichnungen), jede allpolige Variante wird ein eigenes Schaltbild.
- DWG-Zeichnungen (AutoCAD 2004–2018) direkt, ohne Konverter; Aufbauansichten im Maßstab der EPLAN-Aufbaumakros.
- Artikeldaten aus WSCAD-Paketen (`.wspak`).
- Verwaltungsschale (`.aasx`, IEC 63278 / IDTA): Hersteller und Bestellnummer aus dem *Digital Nameplate*, das
  Bauteil aus dem eingebetteten EDZ (sonst DXF/DWG), STEP nach `3D`, Datenblätter und alle Teilmodelle (`aasx.json`)
  nach `herstellerdaten`.
- Hersteller-Makropakete (ZIP oder Ordner mit `.ema` und Artikeldaten, z. B. Beckhoff): Reiter *Herstellerpakete* in
  der Bauteilsammlung, Umwandlung je Typ bei Bedarf; SPS-Karten bekommen ihre Kanaltabelle aus den
  Funktionsschablonen. Ohne Oberfläche: `--import-eplan <sammlung> --macro-package <zip|ordner> --type T`.
- Suche und Import direkt aus dem EPLAN Data Portal (Herstellerliste, Filter, Bildvorschau); fehlt ein STEP-Modell,
  holt *Bauteile holen* EPLANs 3D-Modell (`.e3d`) aus dem Data Portal (mit Token).

## Navigation wie bei EPLAN

In QET und im exportierten PDF:

- EPLAN-Dreifachsprung am Bauteil: linkes Drittel vorwärts durch die Sprungfolge des Geräts, Mitte zur
  Hauptfunktion, rechtes Drittel rückwärts (in QET Strg+Doppelklick, im PDF drei Linkbereiche).
- Freie Texte, Bauteile und Kabel können auf ein Blatt oder ein Bauteil zeigen (`destination`): Zeilen von Klemmen-,
  Kabel- und Stücklisten springen zum Gerät; Strg+Klick folgt, der PDF-Link zoomt darauf.
- Ein verknüpfter Folioverweis ist selbst ein Link zum Gegenpfeil, nicht nur sein Text. Gefüllte Verweispfeile im
  EPLAN-Aussehen.
- Das PDF markiert das Sprungziel wie EPLAN 2.9: Nach einem Klick auf Potentialpfeil, Querverweis oder Gerätesprung
  blinkt ein roter Rahmen um das Ziel und verschwindet nach 5 s, in Acrobat wie in Chrome und Edge. Bei PDF/A keine
  Rahmen (`--export-pdf … --pdf-a`).

## Daten wie bei EPLAN statt Handtext

- SPS: Die Karte (SPS-Master) führt ihre E/A-Tabelle (Adresse, Funktionstext, Kommentar, Klemmen); jeder Kanal ist
  an seine Zeile gebunden (`group_index`) und zeigt die Werte über seine Felder – Tabelle ändern, die Kanäle ziehen
  nach.
- Siemens TIA Portal: *Projekt > SPS-Variablen und TIA Portal…* zeigt jeden Kanal so, wie er nach TIA geht (Adresse,
  Name, Datentyp, Kommentar), und schreibt ihn als PLC-Variablentabelle – `.xlsx` im Format des Imports der
  Variablentabelle (Blätter PLCTags/TagTableProperties, wie die STEP-7-Hilfe es beschreibt) oder SimaticML-`.xml` für
  Openness. Eine aus TIA exportierte Tabelle wieder eingelesen, markiert das Fenster die Unterschiede (Name,
  Kommentar, Adresse, fehlt, nur in TIA); ein Klick übernimmt Namen und Kommentare aus TIA in den Plan (ein
  Rückgängig-Schritt). EPLAN-Adressen (E 50.0, PEW256) werden zu %I50.0/%IW256; Kanäle, die nur DI0/DQ3 heißen,
  werden ab der Startadresse des Moduls gezählt (Elementinformation `plc_start_address`/`plc_start_address_2`,
  sonst die nächsten freien Bytes). Auch per `--export-tia-tags`, `--compare-tia-tags … [--apply neu.qet]` und
  `qet.exportTiaTags/compareTiaTags/applyTiaTags` in Skripten. Derselbe Export schreibt auch eine CODESYS-Liste
  globaler Variablen (`.gvl`, `.st`: `VAR_GLOBAL … AT %IX0.0 : BOOL`) und PLCopen-XML TC6 v2.01 (`.plcopen.xml`)
  für CODESYS, TwinCAT und Beremiz.
- Sicherheit: `qet.safetyFunction(json)` liefert den PL einer Sicherheitsfunktion nach ISO 13849-1 (Teilsysteme mit
  Kategorie, MTTFd oder B10d, DCavg, CCF) und den erforderlichen PL aus S/F/P – eine Planungsnäherung nach KoperCad,
  nicht Tabelle K.1 (Formeln S1–S6 in `FORMELN.md`).
- Kontakte: Eine platzierte Spule kann eigene Kontaktgruppen tragen (11/12/14, 21/22/24 …), wie die
  Funktionsschablone eines EPLAN-Artikels; die Kontakte übernehmen daraus ihre Anschlussbezeichnungen, der
  Kontaktspiegel zeigt sie.
- Querverweisformel `%=M`: Die Anlage erscheint nur, wenn sie von der des eigenen Blatts abweicht (`/42/1`,
  `=DC/38/2`), wie bei EPLAN.

## Ein Prüfpanel wie der EPLAN-Prüflauf

- *Projekt > Projekt prüfen* (auch als Knopf in der Werkzeugleiste) führt alle Prüfungen auf einmal aus und zeigt die
  Befunde in einem Panel, nach Prüfung gruppiert, jeder mit Schwere (Fehler, Warnung, Hinweis), Blatt und Feld; der
  Tooltip nennt die EPLAN-Meldungsnummer. Eine Auswahl zeigt alle Befunde, nur Fehler und Warnungen oder nur Fehler;
  Doppelklick öffnet das Blatt und markiert Gerät oder Leitung. Geprüft werden: doppelte Kennzeichen und
  Leiternummern, Potenzialtypen, offene Leitungsenden, Symbole ohne Leitung, Gerätetypen, Leitungen je Anschluss,
  Anschlussnamen und die Regeln des Forks: Kontakte, Spulen und Abbruchstellen ohne Gegenstück, SPS-Kanäle ohne
  Adresse, Kabel ohne Ziel, Klemmen ohne Klemmleiste, Geräte ohne Kennzeichen oder Artikel, offene Anschlüsse und
  Fluidanschlüsse, Leitungsquerschnitt zur Absicherung (In ≤ Iz nach EN 60204-1), Spannungsfall je Kabel mit Länge
  (IEC 60364-5-52 G, bei In des Schutzorgans; Grenze 3 %, sonst `voltage_drop_limit_pct`, Spannung
  `nominal_voltage_v`), IP-Adressen und PROFINET-Namen (gültig, eindeutig, im Subnetz; Elementinfos `ip_address`,
  `subnet_mask`, `network_name`, `profinet_name`), Schaltschrankaufbau.
- Der Bericht beginnt mit seinem Ergebnis: *Korrektur nötig* (Fehler), *Prüfen* (Warnungen), *Unvollständig
  gelesen* (nichts Falsches gefunden, aber Angaben fehlten) oder *Kein Befund*; eine fehlende Angabe ist nie
  „kein Befund“ (JSON `status`, `statusText`, `summary.incomplete`, `incomplete` je Befund).
- *Steuerstromkreis* liest die Schützlogik als Graph ihrer Potentiale: Schütze, deren Hauptkontakte zusammen die
  Einspeisung kurzschließen (Wendeschütz, Stern-Dreieck) und die gleichzeitig anziehen können (Fehler), Verriegelung
  nur in eine Richtung oder nur über Taster, Selbsthaltung ohne Aus-Kontakt, Kreis ohne Ruhezustand (Pumpen) –
  EN 60204-1 9.3.4. Schienen: `sim_role` supply+/supply-, Leitungsfunktion L+/M/24V/0V, sonst die Ausgänge der
  Schutzorgane und der gemeinsame A2-Rückleiter.
- *Kurzschluss, Abschaltung und FI*: In ≤ Iz nach IEC 60364-5-52 Anhang B (Kupfer, Verlegeart A1, A2, B1, B2, C,
  PVC oder VPE; Elementinfos `installation_method`, `insulation`, `ambient_c`, `grouping` oder Kabeltyp
  NYM/NYY/N2X/H07V), Ik3 an der Einspeisung nach IEC 60909-0 (vereinfacht; `ik3_ka`, `sk_mva`, `sr_kva` + `ukr_pct`
  + `un_v`) gegen das Schaltvermögen (`icu_ka` oder „6 kA“), Zs × Ia ≤ U0 nach IEC 60364-4-41 411.4.4 (`zs_ohm`
  oder `ik1_ka`, `u0_v`; Ia aus B/C/D oder `ia_a`), FI hintereinander (selektiv nur mit Typ S und 3 × IΔn;
  `idn_ma`, `rcd_type`), Steckdose ohne FI ≤ 30 mA (411.3.3). Formeln P01–P16 in `tools/simulation/FORMELN.md`,
  Tests `tools/plancheck/pruefen_logik.py`.
- Skript-API `qet.checkProject(folio = -1)`, CLI `--check-project <projekt.qet> [bericht.json]` (dieselben Regeln;
  Rückgabewert 3 bei Fehlern).
- **Ein-Klick-Korrekturen:** Knopf *Beheben* und Rechtsklick *Beheben: …*, wo die Lösung eindeutig ist
  (Unterbrechungspunkt verbinden oder Stern bilden, Kontakt mit seiner Spule verbinden, BMK vergeben, doppelte
  Drahtnummer neu vergeben, verwaisten Paarquerverweis löschen). Skript `qet.checkFixes()`, `qet.applyCheckFix()`.

## Querverweise, Drahtnummern, SPS-Adressen, Netzwerk (Abgleich mit OpenWiring)

Vergleichsliste: [docs/vergleich-openwiring.md](docs/vergleich-openwiring.md).

- **Stern-Unterbrechungspunkt:** ein Unterbrechungspunkt, verbunden mit allen freien gleichnamigen Gegenstücken
  (+24V auf viele Blätter), Querverweis mit allen Positionen. Verknüpfungsfenster *Stern*, `qet.setReportStar()`.
- **Paarquerverweis:** ein Kontakt auf einem anderen Blatt nur dargestellt; folgt BMK und Anschlüssen, zählt nicht im
  Kontaktspiegel, beide verweisen aufeinander. Verknüpfungsfenster *Als Paarquerverweis darstellen …*,
  `qet.addPairRepresentation()`.
- **Drahtnummern nach Muster** (*Projekt > Drahtnummern nach Muster …*): `{seite}.{n:2}`, `{spalte}`, `{zeile}`,
  `{schrank}`, `{anlage}`, `{n:A}`; Zähler je Blatt, Schrank oder Projekt; Reserve je Blatt; Spalten oder Zeilen;
  gesperrt oder mitlaufend. `qet.setWireNumberPattern()`, `qet.numberWires()`.
- **SPS-Adressen je Station:** Siemens international/deutsch, IEC, Omron, TwinCAT (`AT %I*`), IO-Link-Ports,
  „Analoge zuerst“ (WAGO). `qet.setPlcStation()`, `qet.assignPlcAddresses()`.
- **Netzwerkadressen** (*Projekt > Netzwerkadressen …*): Tabelle aller Netzgeräte mit Problemen, freie IP-Adressen
  aus den IP-Netzen, PROFINET-Namen aus Ort und BMK, CSV. `qet.assignIpAddresses()`, `qet.assignProfinetNames()`.
- **Drahtliste des Aufbaus:** jede Leitung in Verdrahtungsreihenfolge mit Querschnitt, Aderendhülse und Zuschnitt;
  `qet.exportPanelWireList()`.
- **QET-Text v1 für KI-Agenten:** ein Blatt als Text; MCP `qet_folio_text`, `qet_folio_text_check`,
  `qet_folio_text_apply`.
- **Import aus Vektor-PDF fremder CAD** (SEE Electrical …): `tools/pdf_import/pdf_projekt.py`, alle Seiten auf einmal.

## Blätter und Schriftfeld

- Hellgraue Stempel über dem Blatt wie bei einem EPLAN-Formular: Schriftfeld-Variablen `watermark` (schräg, z. B. das
  Druckdatum) und `watermark-side` (senkrecht am rechten Rand); leer = aus, je Blatt oder als Projektvorgabe.
- Neue Schriftfeld-Variablen `%{printdate}` und `%{saveddate-de}` (TT.MM.JJJJ).
- Skript-API: `outsideSheet(folio)` listet, was außerhalb des Blatts liegt.

## Revisionen wie bei EPLAN

- Menü Projekt > Revision > *Revision abschließen…*: Index (Vorschlag A, B, … bzw. 01, 02, …) und Kurztext. Der Index
  steht danach in jedem Schriftfeld (`%indexrev`), und die Revision ist eine neue Zeile der einen Revisionsliste des
  Projekts (Projekteigenschaften > Revisionen, dort auch von Hand bearbeitbar; *Listen > Revisionstabelle hinzufügen*
  druckt sie auf ein Blatt). Das Schriftfeld *revision* zeigt ihre letzten drei Zeilen über `%{rev1_index}`,
  `%{rev1_date}`, `%{rev1_text}`, `%{rev1_author}` (rev1 = neueste). Neben dem
  Projekt entsteht `<Name>_Revisionen/` mit der Momentaufnahme `<Name>_Rev_<Index>.qet` und einem PDF, in dem die
  Änderungen seit der vorigen Revision markiert sind; das Projekt wird gespeichert.
- *Änderungen markieren*: Was sich seit der letzten Revision geändert hat, bekommt automatisch eine rote
  Revisionswolke mit Dreieck (Symbole, Texte, Formen), Leitungen einen breiten Strich entlang ihres Wegs; Gelöschtes
  gestrichelt und durchgekreuzt, ein neues Blatt einen Hinweis über dem Schriftfeld. Sichtbar am Bildschirm, im
  Druck und im PDF; ab der ersten Revision eingeschaltet.
- *Revisionen vergleichen…*: zwei Stände (aktueller Stand, eine Revision, eine beliebige Projektdatei) als Liste
  mit Sprung per Doppelklick und als PDF mit markierten Änderungen. Zuordnung über UUID, sonst wie EPLANs
  „alternative Identifizierung“ über Blatt, Typ und BMK bzw. die verbundenen Anschlüsse – auch neu erzeugte Pläne
  lassen sich vergleichen.
- CLI: `--close-revision <projekt> <index> [text] [--no-pdf]`, `--revisions <projekt>`,
  `--revision-marks <projekt> on|off`, `--compare-revisions <alt|index> <neu> [ausgabe.pdf] [--json]`.
  Skript-API: `closeRevision`, `revisions`, `revisionChanges`, `nextRevisionIndex`, `setRevisionMarks`,
  `exportRevisionPdf`.

## Funktionsvorlagen

Seitenleiste *Anzeige > Funktionsvorlagen*, wie EPLAN-Makros mit Wertesätzen:

- Wiederverwendbare Schaltungsteile mit Platzhaltern: Vorlage in der Seitenleiste wählen (mit Vorschau), wenige Werte
  eingeben (Kennzeichen, Leistung, SPS-Adresse), per Klick einfügen oder auf ein neues Blatt legen – fertig verknüpft,
  mit Artikeldaten, Klemmen und Kabel, ein Rückgängig-Schritt. Beim nächsten Einfügen zählen Kennzeichen, Klemmen und
  Kabel auf die nächsten freien weiter.
- Platzhalter `{BMK}`, `{P_kW}`, `{ADR}`, `{XN+1}` in Kennzeichen, Texten, Elementinformationen, Drahtnummern und
  Kabeltypen. Eine Auswahltabelle wählt den Artikel nach Leistung (nächstgrößere Zeile: 3 kW bekommt Schutzschalter
  und Schütz der 4-kW-Zeile). Ein SPS-Kanal wird mit dem SPS-Modul des Projekts verknüpft, das seine Adresse hat.
- Eigene Vorlagen aus einer Auswahl: Kontextmenü *Als Funktionsvorlage speichern…* (Name, Kategorie, Platzhalter,
  eingefügte Excel-Tabelle). Ablage als `.qetmak` im Vorlagenordner des Nutzers (mit `--data-dir`:
  `Daten\macros\Funktionsvorlagen`); sechs Beispiele ohne Kundendaten in `funktionsvorlagen/` (neu erzeugt mit
  `tools/funktionsvorlagen/beispiele_erzeugen.py`).
- Skript-API `listTemplates()`, `templateInfo()`, `insertTemplate(folio, vorlage, werte, x, y)`,
  `saveTemplate(folio, name, optionen)`; MCP: `qet_edit`-Operationen `insert_template` und `save_template`.

## Konfigurator (Plan aus einer Tabelle)

Nach dem Vorbild von EPLAN EEC Pro, Konzept in [docs/konfigurator.md](docs/konfigurator.md):

- Menü *Projekt > Plan aus Konfiguration erzeugen…*: eine Excel- oder CSV-Tabelle mit einer Zeile je Objekt
  (Spalten `Objekt`, `Typical`, wahlweise `BMK`, `Anlage`, `Ort`, `Blatt` und die Werte der Vorlage wie `P_kW`,
  `TEXT`) wird zum Plan: Schaltplanblätter in Tabellenreihenfolge (kleine Objekte teilen sich ein Blatt), die
  SPS-Übersicht mit allgemeinen Modulen zu 16 Kanälen, deren Kanäle mit den Kanälen im Strompfad verknüpft sind
  (Querverweise in beide Richtungen), dazu Bestellliste, Klemmenplan und Kabelliste als Listenblätter.
- Typicals sind Funktionsvorlagen mit **Varianten und Bedingungen** (`<variant when="P_kW > 5.5" …>`): das Typical
  *Antrieb* wählt über 5,5 kW den Stern-Dreieck-, sonst den Direktstart-Abgang. **Formeln** (`<formula>`) rechnen
  Werte aus Werten und Tabellen: der Motorschutzschalter kommt aus der Siemens-Auswahlhilfe SIRIUS
  (`funktionsvorlagen/Tabellen/siemens_3RV2_direktstart.csv`, nach Typenschildstrom `I_N` oder nach Leistung).
  Tabellen dürfen CSV-Dateien neben der Vorlage sein. Formelsprache: `+ - * / ^`, Vergleiche, `and/or/not`,
  `if`, `min`, `max`, `round`, `text`, `lookup`, `has`.
- **Zähler:** BMK auf das nächste freie, SPS-Adressen mit der neuen Parameterart `kind="address" io="DQ" count="3"`
  auf den nächsten freien Kanal; `{ADR+1}` zählt Kanäle (`%Q0.7` + 1 = `%Q1.0`).
- **Neu erzeugen** mit geänderter Tabelle in dasselbe Projekt: Unveränderte Objekte behalten ihr Blatt,
  geänderte werden neu gezeichnet, BMK und Adressen bleiben. Ein Blatt, das seit dem Erzeugen von Hand geändert
  wurde, wird nicht still überschrieben: es bleibt, das neue folgt mit „[neu]“, der Bericht nennt jede Änderung
  (erst „Überschreiben“ bzw. `--overwrite` ersetzt es). Die alte Datei bleibt als `<name>.vor-konfigurator.qet`.
- **Produktnummer** als zweiter Eingang: Spalte `Produkt` mit z. B. `MS-400-7K5-D`; eine Regeldatei
  `<Familie>.product.json` zerlegt sie in Segmente, jeder Code ist ein Wertesatz, Regeln wählen Vorlage und Variante.
  Unbekannte Codes sind Fehler, nie geraten. `--check-product-rules` prüft die Vorlagen gegen die Regeln (jede
  erlaubte Nummer, Lücken nach Schwere), `--decode-product <nummer>` zeigt das Ergebnis; MCP `qet_product_rules`.
- Kommandozeile `--generate-from-config <tabelle.xlsx|csv> <ziel.qet> [--overwrite] [--json]` (Exit 3 bei
  Konflikten), MCP-Werkzeug `qet_generate_from_config`. Beispiel: `examples/konfigurator/maschine.csv`
  (vier Antriebe gemischt Direkt/Stern-Dreieck, zwei Ventile, drei Sensoren); Prüfung:
  `python tools/konfigurator/test_konfigurator.py`.

## Kabel

- Menü Projekt > *Kabelliste (Typen wählen)…*: alle Kabel des Projekts mit Typ, Adern, Farben, Anlage, Ort und
  Blättern; *Typ wählen…* ändert den Typ in einem Rückgängig-Schritt.
- Herstellerkataloge: Jede CSV im Ordner `cable-catalogs` neben der Kabeltypdatei wird als schreibgeschützter
  Katalog gelesen. Filter nach Katalog, Aderzahl, Aderquerschnitt, Energiekettentauglichkeit und Eigenschaften
  (geschirmt, halogenfrei, UL/CSA, torsionsfest …).
- `tools/kabelkatalog/kabelkatalog_eplan.py` erzeugt solche Kataloge aus den frei erhältlichen EPLAN-Artikeldaten der
  Hersteller (z. B. igus chainflex, LAPP): Adern mit Farben, Querschnitt, Außendurchmesser, Spannung, Artikelnummer.

## Beschriftung

- Menü Etikettendruck > *Beschriftung und Etiketten exportieren…*: Draht-, Klemmen-, Geräte- und Kabelschilder als Datei für
  Phoenix Contact MARKING system / PROJECT complete, Weidmüller M-Print PRO oder WAGO Smart Script, als neutrale CSV
  oder als einfache Etikettenliste (ein Etikett je Zeile in natürlicher Reihenfolge, für jeden Etikettendrucker; auch
  per `--export-labels` von Upstream);
  Auswahl nach Art, Blättern und Anlage/Ort. Auch per `--export-marking` und `qet.exportMarking()`/`qet.markings()`.
  Kein Hersteller veröffentlicht ein festes Importschema; jede Datei ist eine CSV nach dem, was der jeweilige Import
  belegt liest. Spalten und Belege in [docs/beschriftung-export.md](docs/beschriftung-export.md).
- Menü Etikettendruck > *THM-Etiketten drucken…* (neben Schaltschrankaufbau): dieselben Schilder direkt auf einen Weidmüller-Thermotransferdrucker THM
  (THM MMP, THM MultiMark), über TCP-Port 9100 oder je Drucker über LPD (Port 515) in cabs Druckersprache JScript, ohne M-Print PRO. Drucker, gerüstete
  Artikelvariante und Druckprofil richtet man im Dialog ein; die gewählte Artikelvariante bestimmt den Drucker.
  Status und Material lassen sich abfragen; gedruckt wird erst nach einer Zusammenfassung von Ziel, Artikel und
  Menge. Auch per `--thm-prn`, `--thm-status`, `--thm-send --yes`, `--thm-printers` und `qet.exportThmJob()`.
  Befehle, Belege und was am Drucker noch zu prüfen ist: dasselbe Dokument.
- Das Druckprofil je Artikel kann aus einer Layoutdatei der eigenen M-Print-PRO-Installation kommen (JSON, unter
  *Layoutdatei…* oder `--layouts` gewählt; keine mitgeliefert, nichts entschlüsselt): Seite, Schilder, Spalten,
  Textfeld und Drehung, Geschwindigkeit und Heizung je Gerätefamilie (THM MMP, THM MultiMark). Gegen vier
  M-Print-Druckdateien geprüft: `S`, `H` und `O` stimmen überein. Endlosmaterial mit Layoutregeln wird mit Begründung
  abgelehnt. Jeder Drucker führt seine übliche Rüstung getrennt von der aktuellen; *Material wechseln…* (oder
  `--thm-load`) bestätigt einen Wechsel. *Auswählen…* listet die Materialien wie M-Print PRO, mit Suche und Filtern
  nach Kategorie, Maß, Farbe und Seiten.

- Weidmüller-Drucker direkt: Drucker PrintJet CONNECT (MultiCard) oder THM MultiMark (Rollen, Etiketten) wählen, je
  Art ist ein passendes Weidmüller-Material vorbelegt (änderbar). *In M-Print PRO öffnen* übergibt jedem Material eine
  Datei über die Kommandozeile von M-Print PRO (`-ImportFilter:CSV -MatNo:…`, auf Wunsch `-p` zum Sofortdruck).
  Auch per `--printer`, `--material`, `--mprint [--open] [--print]` und `qet.mprintJobs()`/`qet.openInMPrintPro()`.

## Einkaufsliste für Einkauf und ERP

- Menü Projekt > *Einkaufsliste exportieren…*: eine Dateiauswahl, sonst nichts. Heraus kommt eine Excel-Mappe
  (`.xlsx`, oder CSV) mit einer Zeile je Artikel, gruppiert nach Lieferant (sonst Hersteller), Summe je Gruppe und
  Gesamtsumme, dazu Menge, VPE, Bestellmenge (auf VPE aufgerundet), Preis, Preiseinheit, Gesamtpreis, Lieferzeit,
  EAN/GTIN, Lebenszyklus und Preisquelle. Geräte ohne Artikel stehen am Ende mit „Artikel offen“. Preise lassen sich
  in Excel nachtragen: Bestellmenge, Gesamtpreis und Summen sind Formeln.
- Daneben entsteht `<Name>_ERP.csv`, ein neutrales Importformat: eine Zeile je Artikel, feste englische
  Spaltennamen (`line;supplier;manufacturer;order_number;description;references;quantity;order_quantity;unit;
  packaging_unit;unit_price;price_unit;currency;total_price;delivery_time;gtin;lifecycle;price_source;article_open`),
  `;` als Trenner, `.` als Dezimalzeichen, UTF-8 ohne BOM, keine Summenzeilen.
- Woher die Werte kommen (später überschreibt früher): Preis und Lieferant am Gerät → Artikeldaten aus dem EPLAN
  Data Portal und den EDZ-Paketen in `herstellerdaten` (Zwischenspeicher `einkauf/artikeldaten.csv`, 30 Tage) →
  eigene Preisliste `einkauf/preisliste.csv` im Datenordner (`Bestellnummer;Preis;Lieferzeit;Lieferant`, optional
  `Preiseinheit;VPE;EAN;Hersteller;Währung`; die Kopfzeile wird beim ersten Export angelegt).
- Ehrlich geprüft: Das Data Portal liefert ohne Token EAN/GTIN und Lebenszyklus, aber weder Preis noch Lieferzeit
  noch VPE; die kaufmännische CSV mit Token enthält nur Stammdaten. EDZ-Pakete haben VPE und Mengeneinheit, ihre
  Preisfelder sind bei den Herstellern 0. Preise und Lieferzeiten kommen deshalb aus der eigenen Preisliste;
  Fehlendes bleibt leer.
- CLI: `--export-purchase <projekt> <liste.xlsx|liste.csv> [--online] [--erp]`, `--fetch-commercial <projekt>
  [--offline]`. Skript-API: `purchaseList()`, `exportPurchaseList(datei, '{"online":true,"erp":true}')`,
  `fetchCommercialData()`, `purchasePriceListPath()`.

## Schaltschrankaufbau und Tafelfertigung

Menü *Schaltschrankaufbau* – der 2D-Ablauf von EPLAN Pro Panel auf einem normalen Blatt:

- *Schaltschrank aufbauen…*: der ganze Aufbau in sechs geführten Schritten mit Haken – Schrank mit Montageplatte
  (Rittal über die Bestellnummer), Hutschienen und Kanäle als Raster (Kanäle am Rand und zwischen den Reihen, eine
  Schiene je Reihe), Geräte und Klemmenleisten aus dem Schaltplan, Prüfung, 3D und Export. Die Einzelbefehle
  stehen unter *Einzelschritte*. Schränke findet man über Breite, Höhe und Tiefe (der passendste zuerst, die Maße
  aus der Artikelbeschreibung); jeder Schrank bekommt eine eigene Seite. *Montagefläche einfügen* fragt nur
  Kennzeichen, Breite und Höhe und nimmt Löcher und Kanten von der passenden Herstellerplatte einer Baureihe
  (KX, AX, VX …), auf das eigene Maß gestreckt.
- *Aufbauprüfung* als Seitenleiste: Teile übereinander, im Kanal, über die Fläche hinaus, ohne Bohrbild,
  Klemmenleiste zu kurz für ihre Klemmen (Breiten aus den Modellen); ein Klick wählt das Teil aus und zeigt es, die
  Liste folgt jeder Änderung. Teile desselben Geräts und an ihrem Partner hängende Geräte (Kombinationsliste) gelten
  als in Ordnung.
- Ein einfacher Klick auf Kanal, Schiene oder Fläche zeigt rechts deren eigene Eigenschaften: Bezeichnung, Länge,
  Breite, Höhe, Artikel, bei Flächen die Lage (Platte, Tür, Seitenwand links/rechts); jede Änderung ist ein
  Rückgängig-Schritt.
- Montageflächen (Platte, Tür, Wand) im Maßstab, Hutschienen, Kabelkanäle und C-Profile mit Befestigungsreihe
  (Gewinde oder Durchgangsloch, Abstand, Randabstand, Nullschnitt), Ausschnitte (rund, Rechteck, Langloch, Sechs-
  und Achtkant) und Sperrflächen – normale Formen mit einer `<panel>`-Rolle, also verschieb-, kopier- und rückgängig
  machbar wie jede Form.
- *Geräte aus dem Schaltplan platzieren*: jedes Gerät wird ein Rechteck in echter Größe (Vignette, nicht in der
  Stückliste), auf die Hutschienen gerastet, mit dem Bohrbild seines Artikels.
- Ein gezogenes Gerät sitzt mit seinem Einrastpunkt (EPLAN-Mittenversatz, sonst Gerätemitte) auf der Schienenmitte und
  rastet bis 4 mm bündig an die freie Kante eines Nachbarn an, auf der Schiene oder frei auf der Platte (Umschalt: frei).
- *Klimaberechnung…* (Flächen nach IEC/TR 60890, Verlustleistung `power_loss_w` aus dem EDZ-Import, nötiger Lüfter
  oder Kühlgerät) und der Kanalfüllgrad der Schaltplan-Leitungen auf dem kürzesten Weg durch die Kanäle (Befund ab
  70 %, farbig in *Kanalfüllgrad anzeigen*) stehen auch in der Aufbauprüfung; Formeln und Quellen in `FORMELN.md`.
  Leitungen ohne Kanal (Platte ohne Kanäle oder Geräte, die kein Kanal erreicht) werden als Bündel auf der Platte
  verlegt: spätere Leitungen folgen den früheren; Kabelbinder und Bündeldurchmesser stehen in der Zusammenfassung,
  die Bündel in der Anzeige.
- Geräte erscheinen mit der Herstellergrafik (Schaltschrankaufbau) aus dem EPLAN Data Portal (über die
  Bestellnummer, zwischengespeichert in Dokumente\panel-templates\devices); sie liefert auch die Maße.
- Montagevarianten je Artikel (`<drillingpatterns>`): auf der Hutschiene keine Bohrung, geschraubt das Bohrbild; der
  EDZ-Import liest Breite/Höhe/Tiefe, Einrasthöhe, Mittenversatz und die EPLAN-`construction`.
- Bohransicht über dem Blatt, Fertigungsbericht mit Warnungen (Loch in Sperrfläche, Überschneidung, Gerät ohne
  Bohrbild), Export: je Montagefläche eine DXF (R12, mm, Layer je Bohrungsart) plus Bohrliste und Schnittliste.
- 3D-Ansicht (Qt OpenGL) mit Platte, Schienen, Kanälen und Geräten; STEP-Modelle der Teile, wenn mit Open CASCADE
  gebaut (MSYS2-Paket `opencascade`, CMake findet es selbst). Klick wählt das Teil auf dem Blatt und hebt die ganze
  Klemmenleiste bzw. das Gerät mit Träger hervor.
  - Modelle werden über die Bestellnummer im Ordner `3D` neben der Firmensammlung gefunden
    (`3D/<Hersteller>/<Bestellnummer>.stp`, mit oder ohne Bindestriche) oder in `QET_3D_DIR`; die Grundfläche auf der
    Platte bestimmt, wie ein Modell steht, die Tiefe ist seine eigene. Ohne Maßangabe nimmt ein Gerät den Umriss
    aus dem Modell.
  - Der Schrank (über seinen Artikel) erscheint halbdurchsichtig; Tür und Seitenwände sind eigene Montageflächen
    (`face="door|left|right"`), ihre Geräte zeigen nach außen. Der Regler *Tür* öffnet die Tür samt Geräten
    (0–120°, Scharnier links); in Skripten `renderPanel3D(…, doorAngle)`, `exportPanel3D(…, doorAngle)`.
  - Zubehör (`accessories`, `artikel_N` am Schaltplan-Gerät) wird mit eingebaut, z. B. die ET-200SP-BaseUnit unter
    ihrem Modul; Klemmenleisten zeigen jede Klemme der QElectroTech-Klemmenleiste mit ihrem Modell.
  - Kanäle ohne Modell als geschlitzte Kanäle mit Deckel. Export als glTF (`.glb`) für jeden 3D-Betrachter.
  - Dunkler, verlaufender Hintergrund, Haupt- und Fülllicht mit Glanz, Boden mit weichem Kontaktschatten, Kanten an
    den Quadern, 4-fache Kantenglättung; Ansichtsknöpfe (Iso, Vorne, Oben, Links, Rechts, Einpassen) im Bild.
- `bohrbilder.csv` in Firmen- oder Benutzersammlung: Bohrungen, Gewinde und Ausschnitte für Teile ohne Bohrbild
  (Bestellnummer mit Platzhaltern, Maße von der Gerätemitte, Quelle je Zeile – Werte aus Datenblättern).
- Skript-API: `setShapePanel`, `shapePanel`, `panelReport`, `exportPanelDxf`, `stepInfo`, `exportPanel3D`,
  `renderPanel3D` (Bild), `addPanelGrid`, `panelClimate`, `panelDuctFill`.

## EPLAN-Projekte und eingebauter Assistent

- Datei > *EPLAN-Projekt importieren* (`.zw1`): das ganze Projekt als verknüpftes QET-Projekt – Formulare und
  Schriftfelder, SPS-Karten als SPS-Master mit Kanaltabelle, symbolische Adressen wie in EPLAN verfolgt, Bericht über
  nicht verknüpfte Kanäle (`sps_bericht.txt`). Derselbe Befehl liest auch einen EPLAN-Export (`.epdz`, z. B. aus
  EPLAN eView) als bearbeitbares Projekt: jedes EPLAN-Symbol wird ein Bauteil mit seinen Anschlüssen (Namen aus dem
  Export), Artikel, Bezeichnung und Funktionstext; die Verbindungen werden Leitungen, die so laufen, wie EPLAN sie
  gezeichnet hat, mit Farbe, Querschnitt und Potential; Unterbrechungspunkte werden verknüpfte Verweispfeile, Spulen
  und Kontakte Master und Slave; Rahmen, Schriftfeld und Strukturkästen bleiben. Daneben Geräteliste,
  Verbindungsliste und die 3D-Bauräume (`.e3d`). Ohne Python kommen die Seiten als Bild (`--import-epdz export.epdz`).
- Datei > *WSCAD-Projekt importieren*: ein WSCAD-Projektordner (ELECTRIX/SUITE ab 6.x, `Database/WSCAD6.db` und
  die `.wsELD`-Schaltplanseiten) wird ein bearbeitbares QET-Projekt – jede Seite ein Blatt mit WSCAD-Rahmen und
  Schriftfeld, jedes Symbol ein Bauteil mit seiner Grafik, seinen Anschlüssen, BMK, Artikel und Funktionstext;
  die Verbindungsliste wird zu Leitungen, die so laufen, wie WSCAD sie zeichnet (mit Farbe, Querschnitt,
  Potential); Pfeile mit Ziel werden verknüpfte Verweise, Spulen und Kontakte Master und Slave, SPS-Kanäle zeigen
  ihre Adressen (E0.7, AEW1); Texte in WSCADs Schrift Roboto Mono. Artikel, die die Firmensammlung noch nicht kennt,
  kommen als Bauteil nach Hersteller/Kategorie/Bestellnummer. Daneben Geräte- und Verbindungsliste (CSV). Format:
  [tools/wscad_import/FORMATBESCHREIBUNG_WSCAD.md](tools/wscad_import/FORMATBESCHREIBUNG_WSCAD.md).
- Claude-Assistent als Andockfenster (Knopf *Claude* in der Hauptwerkzeugleiste): liest und ändert das offene Projekt
  über den MCP-Server unten; Bilder und PDFs lassen sich mitschicken. Mit API-Schlüssel oder in eigenen Builds
  (`-DQET_CLAUDE_KONTO=ON`) über das eigene Claude-Konto.
- *Klemmleiste zeichnen*: ein Strich quer über Leitungen setzt auf jede eine Klemme aus dem Klemmenkatalog (Filter,
  maßstäbliche Vorschau, Foto), auch Mehrstockklemmen (L/N/PE).
- Herstellerportale (Siemens, Rittal, 3Dfindit, SEW, WAGO, Keyence, Beckhoff, RS Components): Konten im Schlüsselspeicher des Systems, fehlende
  Bauteile automatisch holen (`--fetch-parts`). Läuft unter Windows und macOS (Workflow `macos-build.yml`).

## Gemeinsame Ablage (SharePoint, OneDrive, Netzlaufwerk)

- Projektsperre: Wer ein Projekt öffnet, das ein Kollege offen hat, erfährt „seit … bei … geöffnet“ und wählt
  *Nur lesen öffnen*, *Trotzdem bearbeiten* (mit Warnung) oder *Abbrechen*. Eine Sperre ohne Herzschlag seit
  10 Minuten (Absturz, Rechner aus) wird zum Übernehmen angeboten; die eines Absturzes auf dem eigenen Rechner wird
  von selbst übernommen. Ohne Oberfläche: `--lock-status <Projekt>`.
- Nur lesen verhindert jedes Speichern auf die Datei (auch automatisch, per Skript oder MCP); Änderungen gehen mit
  *Speichern unter* in eine andere Datei.
- OneDrive-/SharePoint-Konfliktkopien („Projekt-<Rechnername>.qetz“) neben dem Projekt werden beim Öffnen gemeldet.

## Pneumatik, Hydraulik und Simulation

Anders als im allgemeinen Teil über QElectroTech beschrieben, simuliert dieser Fork.

- Fluid-Stückliste: Projekt > *Fluid-Stückliste exportieren…* bzw. `--export-fluid-bom` – Ventile, Zylinder und
  andere Fluidbauteile (gleiche zusammengefasst, mit Kenndaten) und die Leitungen nach Art und Größe mit der Summe
  ihrer Längen. Jede Fluidleitung kann eine eigene Länge tragen (Leitungsfenster, Skript/MCP `length`, z. B.
  „1,5 m“); sie gilt nur für diese Leitung, nicht für das ganze Potential.
- Reiter *Fluid power* im Eigenschaftsfenster eines Ventils oder Zylinders: eigene Zylindermaße je platziertem
  Bauteil (Kolben, Stange, Hub), ohne das Symbol zu ändern.

- Die 437 Pneumatik- und Hydrauliksymbole der Standardsammlung tragen Simulationsdaten (Grafik unverändert): benannte
  Anschlüsse mit Medium, Funktion, Stellungen und Schaltwege aller Wegeventile, Betätigung, Zylinderdaten und Kennwerte
  (`<fluid>`, Generator `tools/symbole/qet_fluid_daten.py`). Ventilspule ↔ Ventil und Relais ↔ Kontakte als
  Master/Slave, Schalldämpfer und Blindstopfen an freien Anschlüssen, *Projekt > Fluidtechnik prüfen*.
- Anzeige > *Simulation*: Pneumatik (Durchfluss nach ISO 6358), Hydraulik (Blenden, Pumpen, Druckbegrenzung,
  Hydrospeicher mit Gasgesetz), Zylinder mit Kraftbilanz und Reibung, elektrische Steuerung (Taster, End- und
  Druckschalter, Relais, Zeitrelais, Zähler, Spulen), SPS-Programme in Strukturiertem Text (Teil von IEC 61131-3 mit
  Schrittketten) und GRAFCET nach IEC 60848 mit den GRAFCET-Symbolen (im Raster neu gezeichnet, `bauteile/grafcet`),
  mit Teil-GRAFCETs, einschließenden Schritten, Makroschritten und Zwangssteuerung. Fehler wie in FluidSIM (Kolben klemmt,
  Spule unterbrochen, Leckage, verstopft, Rückschlagventil gesperrt/offen, Schalter schaltet nicht/immer) stehen am Bauteil
  (`sim_fault`) und werden per Ereignis repariert. Analoge Elektrik Stufe 1: R, C, L, Quellen und Dioden nach Kirchhoff
  (`sim_role` circuit); Stufe 2 koppelt sie an die Schaltlogik: Kontakte schalten im Stromkreis, Relais- und Ventilspulen
  (R, L) ziehen nach ihrem Strom an, SPS-Analogein- und -ausgänge (AI, AQ);
  Stufe 3 ergänzt Bipolartransistoren (Ebers-Moll) und Operationsverstärker,
  Stufe 4 Sicherungen, Leitungsschutzschalter (B/C/D) und Motorschutzschalter (Klasse 10),
  Stufe 5 Gleichstrom- und Asynchronmotoren am Drehstromnetz.
  Sonderantriebe (R12): pneumatischer Muskel Festo DMSP nach den Kraft-Weg-Kurven des Katalogs und hydraulischer
  Druckübersetzer (Bypass, Pumpkolben, entsperrbares Rückschlagventil) nach Datenblattwerten.
  FluidSIM-Bauteile seit 1.47.0: Proportionalventil MPYE (Festo-Kennlinie), pneumatische Timer, Verzögerungsventile,
  Vorwahlzähler, Zweihandsteuerblock, Vakuumsaugventil; seit 1.48.0 Taktstufen TAA/TAB und Muskel mit Führungszylindern.
  Leitungen unter Druck werden blau bzw. rot, stromführende Drähte rot, aktive Schritte bekommen eine Marke; Taster und
  Ventile bedient man im Fenster oder per Klick auf das Symbol; eine Liste zeigt die SPS- und GRAFCET-Variablen. Wie
  in FluidSIM bewegen sich die Symbole (1.54.0): Kolben und Kolbenstange fahren mit dem Hub aus und ein, auch im
  gedrehten Symbol, und die Felder der Wegeventile verschieben sich unter den Anschlüssen, beim Umschalten anteilig.
  Nur die Anzeige ändert sich; nach *Stopp* ist der Plan wie vorher.
- Ohne Oberfläche: `--simulate plan.qet --event "0.2 -SF1 an" --duration 2 --csv weg.csv --png ordner`.
  `--simulate-reference` prüft den Rechenkern gegen analytische Lösungen, OpenModelica-Läufe (Weg und Drücke) und
  Testprojekte von Anfang bis Ende; ungültige Daten und Rechenfehler brechen einen Lauf mit Meldung ab.
  Jedes Bauteil wird nur über den versionierten Modellkatalog gerechnet (`--simulate-catalog`); Bauteile ohne
  Modell verhindern den Start. `--signals/--measure` liefern Drücke, gerichtete Durchflüsse, Zylinderweg,
  -geschwindigkeit und -kraft, Motor- und Schwenkwinkel, Haltekraft von Saugern in SI-Einheiten. Gerechnet werden auch
  Hydromotoren, Vakuumerzeuger, Sauger, Vakuumschalter, Schwenkantriebe, Rohrleitungen mit Länge und Durchmesser,
  Schnellkupplungen (gekuppelt/getrennt), verstopfte Filter, 4/3-Stetigventile, Senkbrems- und Druckzuschaltventile, Druckwaagen, Schock-/Nachsaugblock,
  pneumatisches Folgeventil, Zylinder mit Einbauwinkel, Kraftprofil, Federkennlinie und FluidSIM-Reibung, Druckluftmotoren, Mehrstellungszylinder, LS-Aggregate, 6/3-Handhebelventile, Belastungseinheiten, Lenkventile, Vorsteuergeräte, Sperrblöcke, LS-Steuerblöcke, Prioritätsventile,
  analoge Signale und Regler (Funktionsgenerator, Sollwertkarte, PID- und Zustandsregler, PT2, I-Glied, Komparator,
  Proportional-Verstärker, lagegeregelte Proportionalmagnete, Druck-, Durchfluss- und Wegsensoren,
  Proportional-Druckbegrenzungsventil) – geschlossene Druck- und Lageregelkreise. Stand der Roadmap: [tools/simulation/ROADMAP_STAND.md](tools/simulation/ROADMAP_STAND.md).
  Anleitung: [tools/simulation/README.md](tools/simulation/README.md).
- TIA-Portal-Bausteine in KOP/FUP (SimaticML-Export) werden zu Strukturiertem Text für die Simulation:
  `--simaticml-to-st baustein.xml [programm.st]` oder `qet.simaticMlToSt(pfad)` – Kontakte, Spulen, Setzen/Rücksetzen,
  Vergleiche, Flanken, TON/TOF/CTU/CTD, Move und Rechnen; Aufrufe eigener Bausteine und SCL/STL-Netzwerke werden als
  nicht übersetzt gemeldet, das Ergebnis prüft die Simulation sofort.
- Weitergabe in der Firma ohne Git: über eine SharePoint-Bibliothek, die OneDrive spiegelt – Installation per
  Doppelklick, Update-Prüfung beim Start, Hilfe > *Fehler oder Wunsch melden...*, Firmen-Bauteile aus dem
  Verteilordner. Anleitung: [tools/verteilung/README.md](tools/verteilung/README.md).
- Datei > *FluidSIM-Schaltung importieren (.circ)*: FluidSIM-6-Schaltungen werden ein Projekt, das sofort simuliert –
  Bauteile, Leitungen, Spulen- und Relaisverknüpfungen, die Schaltwege jeder Ventilstellung, alle Kennwerte mit
  Einheit, die elektrischen Rollen (Versorgung, Taster, Endschalter aus dem Wegmaßstab, Ventilmagnete) und die
  GRAFCETs mit der GRAFCET-SPS. Auch als `tools/fluidsim_import/fluidsim_plan.py` und MCP `qet_fluidsim_import`;
  `fluidsim_batch.py` übernimmt und simuliert zur Prüfung alle FluidSIM-Beispiele. Jedes Bauteil wird mit eigenen
  Symbolen nach ISO 1219 / IEC 60617 gezeichnet (aus der Datei kommen nur Anschlüsse und Daten): jeder kann eine
  Schaltung übernehmen, ohne FluidSIM und ohne Festo-Grafik im Projekt.

## Werkzeuge

In `tools/symbole/`:

- `pdf_symbol.py` / `pdf_blatt.py`: Symbole bzw. ganze Blätter 1:1 aus einem Vektor-PDF (z. B. EPLAN-Plan) übernehmen,
  mit echten Leitern und Knoten.
- `verknuepfung_setzen.py`: Master/Slave-Verknüpfungsart importierter Herstellersymbole nach ihrer Rolle setzen
  (Spule, Kontakt, Schutzschalter, SPS-Baugruppe, SPS-Kanal, Gerätepin).
- `anschluesse_ergaenzen.py`, `nachzeichnen_symbole.py`: Anschlüsse ergänzen, eigene Symbole nachzeichnen
  (`bauteile/nachgezeichnet/`).

## MCP-Server

Unter `misc/qet-mcp/` liegt ein Model-Context-Protocol-Server (nur Python-Standardbibliothek), mit dem ein
KI-Assistent QElectroTech-Projekte lesen, prüfen und ändern kann: Projektinfo, Elemente, Leiter, Unterschiede
(Diff), Export ohne Oberfläche, Bearbeiten, Symbole bauen, Suche und Import im EPLAN Data Portal, Regel- und
Layoutprüfung, SQL auf die Projektdatenbank, Skript-Knöpfe, ein Live-Modus auf dem geöffneten Projekt und die
Simulation (`qet_simulate`, `qet_simulate_reference`) und der FluidSIM-Import (`qet_fluidsim_import`).
Einrichtung und alle Werkzeuge: [misc/qet-mcp/README.md](misc/qet-mcp/README.md).

## Für KI-Assistenten

Skills unter `.claude/skills/` (Claude Code lädt sie im Klon automatisch; andere Werkzeuge: die jeweilige
`SKILL.md` lesen, sie verweist auf ihre Unterdateien):

- `qet-schaltplan-zeichnen` – wie man EPLAN-artige Blätter per Skript zeichnet, prüft und verknüpft, auch GRAFCET.
- `fluidtechnik` – Pneumatik- und Hydraulikpläne: Datenmodell, Anschlüsse, Spule ↔ Ventil, FluidSIM-Übernahme.
- `qet-simulation` – Pläne simulieren: Konventionen, SPS-Programme, GRAFCET, Prüfung des Rechenkerns, Erfahrungen.
- `bauteile-herunterladen` – Herstellerdaten (EPLAN-Makros, DXF, STEP) für fehlende Bauteile holen.
- `eplan-wissen` – die EPLAN-Plattform-Hilfe 2.9 in eigenen Worten (PDF-Sprünge, Querverweise, Kennzeichnung,
  Klemmen, Kabel, SPS, Auswertungen, Makros/Artikel, Eigenschafts-IDs, Prüfmeldungen …), jede Regel mit Quell-URL und
  „Bedeutung für QET“. Einstieg: `.claude/skills/eplan-wissen/SKILL.md`.

Herstellerdateien selbst sind nicht enthalten (Nutzungsbedingungen der Portale).
