---
name: qet-schaltplan-zeichnen
description: Schaltpläne in QElectroTech (EPLAN-Fork) per MCP bzw. qet_mcp-Funktionen zeichnen oder einen bestehenden Plan (EPLAN-PDF) nachzeichnen – Layoutregeln, Bauteilwahl aus der Benutzersammlung, Verdrahtung, Beschriftung, SPS-Kanäle, Prüfen per PNG. Immer verwenden, wenn ein Schaltplanblatt in QET entstehen oder aus einem PDF übertragen werden soll, auch bei „zeichne Blatt X nach“, „mach den Plan in QET“, „mach das hübsch“.
---

# Schaltplanblätter in QElectroTech zeichnen

Gelernt beim Nachzeichnen eines EPLAN-Plans (Pressensteuerung) in QElectroTech. Jede neue
Erkenntnis hier nachtragen (Abschnitt „Gelernt“), damit das nächste Blatt besser wird.

## Umgebung

Pfade kommen aus Umgebungsvariablen (siehe `vorlagen/qet_direkt.py`): `QET_BINARY` (gebaute
QElectroTech.exe), `QET_MCP` (Pfad zu `qet_mcp.py`), `QET_PROJEKTE` (Zielordner der Projekte).


- Fork: dieses Repository, lauffähig unter
  `build\deploy\QElectroTech.exe`.
- MCP-Server `qelectrotech` (User-Scope): `QET_BINARY` = Fork-exe,
  `QET_MCP_ELEMENTS` = Benutzersammlung + Fork-`elements`,
  `QET_MCP_WORKSPACE` = Ordner mit den Projekten,
  `QET_ENABLE_SCRIPTING=1`. Neue Registrierung wirkt erst in einer neuen Claude-Sitzung.
- Ohne geladene MCP-Werkzeuge: dieselben Funktionen direkt aus Python, siehe
  `vorlagen/qet_direkt.py` (mit dem Python aus Python mit den qet_mcp-Abhängigkeiten
  ausführen). Projekte in einen eigenen Ordner je Plan (`QET_PROJEKTE`).
- Benutzersammlung: `%APPDATA%\QElectroTech\QElectroTech\elements`, Ordner
  Hersteller/Kategorie/Typ; im Projekt als `custom://Hersteller/Kategorie/Typ/Datei.elmt`.
  Fehlende Symbole: Data-Portal-Paket mit „Bauteil-Import“ der Fork oder nachzeichnen mit
  `tools/symbole/nachzeichnen_symbole.py` (Muster für eigene Symbole).

## Ablauf je Blatt

1. **Vorlage lesen**: Seite mit pymupdf als Bild (120 dpi) ansehen; Beschriftungen nicht vom
   Bild abschreiben, sondern mit `page.get_text("words")` samt Position lesen (kleine Pin-Namen!).
   Verbindungen gegen Klemmenplan, Steckerplan und Kabelübersicht des PDFs prüfen.
2. **Bauteile suchen**: `tool_element_search(USER, "<Bestellnummer>")` bzw. in `COMMON`.
   Ergebnis liefert `path` für add_element und die Anschlussnamen in Index-Reihenfolge.
   Maße/Anschlusslage direkt aus der .elmt lesen (hotspot, terminal x/y/orientation).
3. **Layout auf Papier festlegen** (siehe Regeln), dann alle Operationen in **einem**
   `tool_edit`-Aufruf auf ein leeres Projekt (`tool_project_new`).
4. **Prüfen**: `ok`, Liste der fehlgeschlagenen Operationen, `hint`; dann
   `tool_export(..., "png", ...)` und das PNG ansehen. Erst wenn es sauber aussieht weiter.

## Layoutregeln (Blatt 1020 × 640 px nutzbar, Raster 10 px, Spalten 60 px)

- **Blattformat wie das Original** (EPLAN-Pläne meist A3 quer): `set_folio_border` `preset`
  `a3-landscape` (1566 × 1053 px), dann `columns` 10, `column-width` 156, `display-rows` false
  wie im EPLAN-Rahmen. Auf dem kleinen Standardblatt wirkt alles groß und gedrängt („Original sieht
  wesentlich besser aus“). Positionen aus dem Original umrechnen (px ≈ PDF-pt × 1,31) und als
  Ankervariablen (XE, XT, YX …) führen, Bauteile einer Spalte relativ dazu.
- **Gleichmäßige Schrift in drei Stufen**: Kennzeichen/Funktionstexte/Verweise 10, Daten 7,
  Kleintext (Aderfarbe, Klemmen-/Anschlussnummern) 5–6. `add_text` kennt keine Größe → nach
  `tool_edit` im .qet setzen: `<input font=…>` **und** `font-size:…pt` im HTML; Kennzeichen über
  `dynamic_elmt_text` mit `<info_name>label`, Klemmen (`borne…`) dabei klein lassen
  (`schrift()` in `vorlagen/beispiel_einspeisung_a3.py`).
- **Waagerechte Bänder** von oben nach unten, wie im EPLAN-Plan: Gerät (y≈90) → Kabel
  (≈205) → Stecker (≈260) → Kabel (≈330) → Klemmleiste (≈390) → SPS-Kanäle (≈480) →
  Funktionstexte (≈625).
- **Spalten so legen, dass jede Leitung senkrecht läuft**: Gerätesymbol, Steckerstift,
  Klemme und SPS-Anschluss auf dieselbe x-Koordinate. Wenn nötig das Gerätesymbol
  anpassen (Kontaktabstände = Spaltenabstände), statt schräge Leitungen hinzunehmen.
- ET-200SP-Kanalboxen (`…_DI-VS_B1` usw.): DI und VS liegen 20 px auseinander, die Box ist
  ≈110 px breit → Kanäle im 110-px-Raster.
- Links ≥ 40 px Rand lassen: QET setzt die Kabelbeschriftung (Name, Typ, Länge) links
  neben die Kabellinie.
- Black-Box-Geräte (Fußschalter, Sensoren): beide Kontaktenden nach unten
  (`contact_down` im Nachzeichner), damit keine Leitung über das Gerät läuft.

## Beschriftung

- Stecker- und Klemmenpins nur mit der Nummer beschriften (`set_label "1"`), das
  Kennzeichen (-XS180, -XB180, -X8) einmal als Text am rechten Rand.
- Leere Leitungsnummern ausblenden: `set_conductor_default displaytext=false` **vor** den
  Leitungen.
- SPS-Kanal: Kennzeichen über `set_label` (-A9 …), Adresse/symbolische Adresse/Funktionstext
  in die EPLAN-Felder: `set_element_text` mit `index` 1, 2, 3 (0 ist das Kennzeichen),
  leere Felder auf `""` setzen, sonst steht „…“ da.
- Unter dem Blatt die Funktionsgruppen wie im Original („Presse abwärts“ …) als Texte.

## API-Fallen (qet_mcp / Scripting)

- Blätter in `tool_edit`-Operationen ab **0** zählen (`folio: 0` = erstes Blatt).
- Anschlüsse per **Index**: Reihenfolge oben→unten, dann links→rechts (wie die Suche sie
  liefert), nicht Dateireihenfolge. Standardbauteile haben leere Namen.
- Feld-UUIDs aus der .elmt gelten im Projekt nicht (werden beim Platzieren neu vergeben) →
  Index verwenden.
- Bei der ersten fehlgeschlagenen Operation bricht das Skript ab (`stopped_early`, `hint`).
- `QET_ENABLE_SCRIPTING=1` muss in der Umgebung stehen, sonst „Scripts are disabled“.
- Referenzen auf neu angelegte Elemente: `"id": "s180"` und später `"$s180"`.
- Die QET-exe kehrt in der Shell ohne Pipe sofort zurück: Ausgaben mit `| cat` abwarten.

## Sauberes Aussehen („nicht matschig“)

- Ausgabe immer als **PDF** (`tool_export(..., "pdf", ...)`), nicht PNG: der PNG-Export hat
  nur 1 px je Rasterpunkt und wirkt verwaschen. Zur Kontrolle das PDF mit pymupdf bei
  200 dpi rendern und ansehen, Ausschnitte bei 400 dpi.
- Klemme: `borne_continuite2.elmt` (nur Kennzeichen); `borne_continuite.elmt` hat ein
  vorbelegtes „_“-Feld, das als Strich unter jeder Klemme erscheint.
- Der EPLAN-Import der Fork entfernt doppelte Linien, fügt zerstückelte gerade Linien
  zusammen und schreibt Kanalnamen (DI, VS) nur einmal waagerecht in den Kasten. Symbole,
  die vor dem 03.11.2026 importiert wurden, neu importieren.
- Kabeltyp kurz halten (z. B. „12x1 mm²“), sonst schneidet der linke Rand ihn ab.

## Keine überlappenden Texte (Pflicht)

- Nach jedem Zeichnen `tool_layout_check(BIN, projekt)` laufen lassen; Ziel: keine
  `label_on_wire`, keine sichtbaren Überlappungen. `overlapping_symbols` misst die deklarierte
  Symbolgröße samt Rand – dann im 300-dpi-Ausschnitt des PDFs nachsehen, ob es wirklich überlappt.
- Zusätzlich immer selbst prüfen: PDF bei 300 dpi in Ausschnitten rendern und ansehen.
- Pins im Paar (20 px Abstand): Beschriftung des **linken** Pins links (`set_element_text`
  index 0, property x, Stecker `-8-8·Ziffern`, Klemme `-6-6·Ziffern`), des rechten rechts (x=4).
- SPS-Kanal-Kennzeichen (-A9) über den eigenen Kasten rechts neben die VS-Leitung (x=30),
  nicht auf die DI-Leitung und nicht so weit links, dass es zum Nachbarkasten gehört.
- Kanalkästen 120 px Raster (Kasten 110 px); zwei Kanäle eines Moduls (DI + AUX) wenn möglich
  mit dem kombinierten EPLAN-Kanalsymbol (`…_DI-AUX_C1`), wie im Original.
- Kennzeichen von Stecker/Klemmleiste (-XS180, -X8) links in die freie Zeile (x=25), nicht
  neben den letzten Pin. Kabel erst bei x≈95 beginnen, damit Typ/Länge nicht in die
  Zeilenleiste ragen.
- Anschlüsse von Gerätesymbolen nicht über den Rahmen hinweg (PE auf den Rahmen setzen).

## Schriftfeld (Logo, Auftragsdaten)

- Vorlage als `.titleblock` in `%APPDATA%\QElectroTech\QElectroTech\titleblocks\`; Muster:
  `vorlagen/schriftfeld_aus_pdf.py (eigenes Skript)` (Logo aus dem PDF mit pymupdf
  `Pixmap(doc, xref)` ziehen, als PNG base64 einbetten, Spalten nach dem Original).
- Spaltenbreiten **nur ganze Prozent** (`t7%`), letzte Spalte `r100%` – Dezimalwerte verwirft
  QET still, dann bleibt das Schriftfeld leer.
- Ins Projekt: `embed_title_block_template name=…`, dann `set_folio template=…`,
  `plant`/`locmach` ohne =/+ (die Vorlage setzt sie davor), `folio` = Blattnummer des Originals.
- Variablen: `%title %author %date %plant %locmach %indexrev %folio %{folio-id}
  %{folio-total} %{previous-folio-num} %{next-folio-num} %{saveddate-eu} %projecttitle`.

## Funktionsvorlagen (wiederkehrende Technik in einem Schritt)

Ab dem Fork-Stand „Funktionsvorlagen“: Motorabgang, Sensor, Ventil usw. nicht Symbol für Symbol zeichnen, sondern als
Funktionsvorlage einfügen – mit Artikeldaten, Klemmen, Kabel und Verknüpfung.
- Liste: `qet.listTemplates()`, Einzelheiten (Parameter, Tabellen): `qet.templateInfo("Motorabgang DOL")`.
- Einfügen: `qet.insertTemplate(folio, "Motorabgang DOL", {P_kW: "3", ADR: "%Q0.0"}, x, y)`; `folio = -1` legt ein
  neues Blatt an, ohne `x, y` bleibt die gespeicherte Lage. MCP: `qet_edit`-Operation `insert_template`.
  Kennzeichen (`kind label`), Klemmennummern (`kind terminal`) und Kabel (`kind cable`) nicht mitgeben: sie werden
  auf die nächsten freien gesetzt. Die Antwort nennt `values` (alles, was eingesetzt wurde) und `notes`.
- SPS-Kanäle der Vorlagen tragen `plc_address`; gibt es im Projekt ein SPS-Modul mit dieser Adresse, wird der Kanal
  damit verknüpft. Deshalb zuerst die SPS-Station/Module zeichnen, dann die Vorlagen einfügen.
- Eigene Vorlage: Blatt mit `{BMK}`, `{ADR}`, `{X}:{XN+1}` usw. zeichnen, dann `qet.saveTemplate(folio, name,
  {category, parameters, tables})` bzw. MCP `save_template`. Beispiele und Generator: `funktionsvorlagen/`,
  `tools/funktionsvorlagen/beispiele.js`.
- Gelernt beim Bau der Beispiele: Klemmen 20 px auseinander vertragen keine Bezeichnung „X1:3“ an jeder Klemme
  (liegt auf der Nachbarleitung) – an der Klemme nur die Nummer, den Leistennamen einmal links davor. Kabellinie mit
  Abstand zu den Klemmen, sonst stoßen Aderfarben und Klemmentexte zusammen. Feldgeräte mit allen Anschlüssen oben
  wählen (Ventilstecker, eigener 3-Leiter-Sensor), damit ein Kabel alle Adern auf einer Linie kreuzt. Anschlussbelegung
  des Symbols vorher prüfen (Ventilstecker: +, −, PE).


## Konfigurator (Plan aus einer Tabelle, nach EEC Pro)

Ab Fork 1.69.0, Konzept `docs/konfigurator.md`. Für Maschinen aus wiederkehrenden Objekten
(Antriebe, Ventile, Sensoren) nicht Blatt für Blatt zeichnen, sondern eine Tabelle anlegen und erzeugen:
- Tabelle (xlsx erstes Blatt oder CSV `;`): `Objekt;Typical;Anlage;Ort;P_kW;TEXT` …, eine Zeile je Objekt.
  `Typical` = Funktionsvorlage („Antrieb“, „Ventil“, „Sensor“ aus `funktionsvorlagen/Typicals`, oder jede andere).
  `BMK` und SPS-Adressen nicht eintragen: sie werden gezählt und bleiben beim Neuerzeugen stabil.
- Erzeugen: Menü *Projekt > Plan aus Konfiguration erzeugen…*, CLI `--generate-from-config <tabelle> <ziel.qet>
  [--overwrite] [--json]` oder MCP `qet_generate_from_config`. Danach `qet_layout_check` und PDF ansehen.
- Neu erzeugen nach Tabellenänderung in dasselbe Ziel: Unverändertes bleibt, Geändertes wird neu gezeichnet.
  Exit 3 = Konflikt: ein Blatt war von Hand geändert, es bleibt, das neue folgt mit „[new]“. Handänderung in die
  Tabelle bzw. Vorlage übernehmen, altes Blatt löschen – oder `--overwrite`. Nie still überschreiben.
- Eigene Typicals: `.qetmak` mit `<variant when="P_kW &gt; 5.5" template="../Motoren/…"/>` (erste passende gilt),
  `<formula name="QA_ART" expr="lookup('3RV2'; I_N; 'QA_ART')"/>`, `<table name="…" file="../Tabellen/….csv"
  key="…"/>` (Tabellenwerte nur aus Datenblatt/Norm, Spalte `Quelle`), Adressparameter
  `kind="address" io="DQ" count="3"`. `{ADR+1}` zählt Kanäle (`%Q0.7` + 1 = `%Q1.0`).
- Produktnummer statt Einzelwerten: Spalte `Produkt` (z. B. `MS-400-7K5-D`), Regeln in `<Familie>.product.json`
  im Vorlagenordner (Segmente, Codes als Wertesätze, Regeln -> Vorlage/Variante). Neue Familie: Regeldatei anlegen,
  dann `--check-product-rules` bzw. MCP `qet_product_rules` bis ohne Fehler; Nummer prüfen mit `--decode-product`.
  Unbekannte Codes nie raten – Regel ergänzen.
- SPS: Der Generator legt allgemeine DI/DQ-Module zu 16 Kanälen auf die SPS-Übersicht und verknüpft die Kanäle.
  Soll eine bestimmte Station (Herstellermakro) her: deren Module vorher auf ein Blatt des Ziels legen – Kanäle mit
  passender Adresse werden mit ihnen verknüpft (Hausstil: Station/Fabrikat nur nach Rückfrage tauschen).

## Gelernt (fortschreiben)

- 23.11.2026, Plan prüfen: Nach dem Zeichnen `--check-project <plan.qet> bericht.json` (oder MCP `qet_plan_check`, im Programm Projekt > *Projekt prüfen*, ein Panel für alle Prüfungen) laufen lassen und alle Fehler beheben; Warnungen ansehen. Klemmen nur dann ohne Warnung, wenn sie in einer Klemmleiste stehen (`add_terminal_strip` + `add_to_strip`), Geräte brauchen `manufacturer_reference`, SPS-Kanäle eine Karte mit Adresse (`link_plc_io`). Rückgabewert 3 = Fehler gefunden.

- 20.11.2026, Einkaufsliste: Für Einkauf/ERP `--export-purchase <plan> liste.xlsx --erp` (oder Menü Projekt >
  *Einkaufsliste exportieren…*). Damit sie taugt, am Gerät `manufacturer`, `manufacturer_reference`, `description`
  und ggf. `supplier`/`quantity` setzen; Preise/Lieferzeiten pflegt die Firma in `Daten\einkauf\preisliste.csv`
  (`Bestellnummer;Preis;Lieferzeit;Lieferant`). Das Data Portal liefert nur EAN/GTIN und Lebenszyklus, keine Preise,
  Lieferzeiten oder VPE (auch nicht mit Token) – nie Werte erfinden, leer lassen. Geräte ohne Artikel erscheinen als
  „Artikel offen“; ein BMK, dessen anderes Element einen Artikel hat (SPS-Kanal neben Modul), gilt nicht als offen.
- 03.11.2026, Blatt „Fußschalter“: erster Wurf hatte schräge/um das Gerät laufende Leitungen,
  überlappende Pin-Kennzeichen, „…“-Felder und abgeschnittene Kabelnamen – Ursachen und
  Abhilfen stehen oben. Vorlage mit dem fertigen Stand: `vorlagen/beispiel_fussschalter.py`.
- Zuordnung -K3 → Steckerstift 11/12 war im Original nicht eindeutig (angenommen: 23/24);
  solche Unsicherheiten dem Nutzer nennen statt still zu raten.
- 03.11.2026, Blatt „Einspeisung“ (Leistungsteil): Muster `vorlagen/beispiel_einspeisung_a3.py`.
  - `add_text` y = **Oberkante** des Textes; Schienennamen daher bei y−17 über die Linie setzen.
  - Unverknüpfte Folgepfeile (`02going_arrow`) zeigen kein Ziel: Label leer, Text „L1  /9.0“
    über der Linie vor dem Pfeil.
  - Abgriffe von Sammelschienen mit `bod`-Verbindungspunkten (114_connections) – sonst laufen
    Leitungen schräg oder ohne Punkt.
  - Hauptschalter 3VA: importiertes Symbol zu groß → Standardsymbol `fa4202…` + Typtext
    (3VA2220, IR, LI) daneben. 3RV/5SY liefern im Portal nur Aufbausymbole → Standardsymbole.
  - Brücken (Q 4–5, Trafo 2–5/11–10) als Leitung vom Element zu sich selbst; layout_check
    zählt sie als Biegung/Kreuzung – Score ≈ 80 ist dann normal, Sichtprüfung entscheidet.
  - LS-Schalter in EPLAN-Form: `10_allpole/…/11_circuit_breakers/disjonct_m_1f.elmt`
    (Unterstrich!; Feld 2 = „B6A“, Feld 1 leeren). `disjonct-m_1f` (Bindestrich) hat den
    französischen Bimetall-„Huckel“ unter dem Kontakt – Nutzer fand das falsch.
  - Herstellergeräte (Trafo) zusätzlich mit Hersteller + Typ beschriften („Murrelektronik 86153“),
    sonst wirkt das Kästchen unbeschriftet.
  - Immer zum Schluss **Original und Neu nebeneinander** als PDF liefern (pymupdf
    `show_pdf_page`, z. B. `<Blatt>_Vergleich.pdf`) – der Nutzer vergleicht so.
  - **Geräte außerhalb des Schranks (NSHV, Motor, Lüfter) bekommen ein Kabel** mit vollem Typ
    („ÖLFLEX CLASSIC 100 G 4x70 mm²“) und Aderkennung (BN/BK/GY/GNYE bzw. 1/2/GNYE) samt PE-Ader;
    schrankinterne Leitungen stattdessen Farbe/Querschnitt („BK 70“, „BK 1,5“, „1,5“).
    `add_cable` `core_colors` in Kreuzungsreihenfolge **ab x1** angeben.
  - Kabeltexte klein wie im Original: nach `tool_edit` in jedes `<cable>` des .qet
    `<texts><text key="type" font="Liberation Sans,5,…"/><text key="cores" font="…5…"/></texts>`
    einsetzen (`kabel_schrift()` im Beispiel), dann exportieren. Zeilenumbruch im Typ wird abgeschnitten.
  - Leiterbeschriftung (`set_conductor num`) und `style` gelten fürs **ganze Potenzial, auch über
    Klemmen hinweg** → landet sonst auf den Kabeladern. Aderfarbe/Querschnitt daher als kleiner
    Elementtext (`add_element_text` mit `id`, dann `set_element_text index "$id"` size 5, rotation 270).
    Gestrichelt (`style` = `dashed`) nur für Potenziale, die sonst nichts tragen (Lüfter-PE).
  - Motor/Lüfter wie EPLAN: eigenes Symbol `custom://Allgemein/Lüfter/…` (M 1~, L/N oben, PE rechts
    gestrichelt, Lüfterrad) aus `tools/symbole/nachzeichnen_symbole.py: luefter()`; Standard-`motor-mono-2` sieht
    mit Leitungen von oben und unten falsch aus.
  - **Herstellermakro zuerst**: Steckt in der EDZ (P8-Download, z. B. `<HERSTELLER>.<Artikel>.edz`) ein
    `items/macro/**/*.ema`, erzeugt „Bauteil-Import → EDZ“ (bzw. `--import-eplan … --edz`) daraus
    `<Typ>_Schaltbild.elmt` mit Original-Grafik (gefüllte Wicklungen, IN/OUT, Technikdaten). Das
    schlägt jede Nachzeichnung; nachzeichnen nur, wenn es kein Makro gibt. Logos werden nicht übernommen.
  - Anschlüsse liegen bei EPLAN-Makros **im** Kasten: Brücken zwischen zwei Anschlüssen laufen
    sonst auf der Rahmenlinie → `move_conductor_segment` (horizontales Segment, dy ±15) herausziehen.
  - **Originalsymbol aus dem Vorlage-PDF statt Nachzeichnung** (Nutzer: „Der FI/LS ist doch ein
    Witz? Wieso nutzt du nicht das originale?“): EPLAN-PDFs sind Vektor-PDFs. Mit
    `qelectrotech-eplan/tools/symbole/pdf_symbol.py plan.pdf <seite> --clip x0,y0,x1,y1 --list`
    den Ausschnitt ansehen, Anschlüsse an den Stummelenden ablesen (Pinnummer steht rechts daneben),
    dann mit `--pin Name:x,y:n|s|e|w` in die Benutzersammlung schreiben (8 mm → 30 px). Reihenfolge:
    Herstellermakro (EDZ) > Original-PDF > Nachzeichnen. Ausschnitte aus Kundenplänen nicht in den
    öffentlichen Fork. Alle Nachbarn im Raster mitziehen (Klemmen 30 px statt 20 px).
  - **Leitungsecken ohne Punkt**: `bod` zeichnet an jeder Ecke einen Verbindungspunkt (falsch,
    EPLAN zeigt dort keinen), `corner.elmt` zeigt einen Kasten. Stattdessen direkt verbinden und den
    Weg mit `move_conductor_segment` legen. QET speichert den Weg erst nach einer Verschiebung; jede
    Verschiebung kann ein Segment einfügen → Index danach aus dem `.qet` lesen
    (`<conductor …terminalname1=…><segment length orientation>`). Blatt 10: +(1) erst Segment 1 dy,
    dann Segment 3 dx.
  - **Gerätekennzeichen und Daten großer Kästen rechtsbündig vor der Kastenkante** wie EPLAN
    („-F110“, darunter „In: …“, „Out: …“ in Datengröße 7). Elementtexte haben keine Ausrichtung →
    x = Kante − 4 − 0,62 · Größe · Zeichenzahl (`links_vom_kasten()` in blatt11).
  - **Freie Texte im .qet**: QET maskiert „>“ im `text`-Attribut von `<input>` nicht (nur „<“).
    Muster `<input [^<]*` verwenden, nicht `<input [^>]*>` – sonst bleibt `font-size` im HTML auf 9 pt
    stehen und alle Texte sind gleich groß. Nach jeder Änderung an `schrift()` die Größen im .qet zählen.
  - PDF-Texte laufen in QET breiter (Liberation Sans statt Tahoma): `pdf_symbol.py` verkleinert sie auf
    ihre Originalbreite; Glyphen-Texte (■ für Schalterstellungen) bleiben unverändert.
  - **Wiederkehrende Kleinteile im 4-mm-Raster** (Verteilerklemme mit zwei Klemmpunkten): mit
    `pdf_symbol.py --raster 0` übernehmen (Anschlüsse 15 px auseinander), alle Lagen aus den PDF-Koordinaten
    umrechnen (`X(pt)`, `Y(pt)` in blatt13). Querbrücken als `add_shape line`, Π-Brücken als Leitung
    von einem oberen Anschluss zu einem Abgangspfeil (QET legt den Bogen selbst). layout_check fällt dabei
    stark ab (off_grid, misaligned) – das ist hier kein Fehler, Sichtprüfung entscheidet.
  - Senkrechte Verweistexte: `add_text` mit `id`, dann `rotate_text index "$id" angle 270`;
    x = Leitung − 1,3 · Größe, damit der Text mittig über der Leitung steht. Unter einem Pfeil:
    unteres Ende = Pfeilende + 0,68 · Größe · Zeichenzahl + 8 (mit 0,6 stößt der Text an die Spitze).
  - **Folgepfeile gefüllt wie EPLAN**: `custom://Allgemein/Verweise/02going_arrow_gefuellt.elmt` und
    `…/01coming_arrow_gefuellt.elmt` (Kopien der QET-Pfeile mit `filling:black`, im Fork unter
    `bauteile/nachgezeichnet`). Die QET-Standardpfeile sind hohl.
  - Ortsrahmen (+BP1 …) strichpunktiert: `add_shape rectangle` mit `id`, dann `set_shape index "$id"
    line-style dashdot` (Formen: solid/dashed/dotted/dashdot – Leiter dagegen dashed/dashdotted).
  - Gefüllte Flächen in EPLAN-PDFs (Steckerstifte, F-DI-Balken) bestehen aus Einzellinien;
    `pdf_symbol.py` setzt sie zu einem gefüllten Polygon zusammen.
  - Pinnamen mit „-“ am Anfang an pdf_symbol.py als `--pin=-(1):x,y:s` übergeben.
  - Datenblatt-Schaltbilder (z. B. WAGO) sind oft Rasterbilder im PDF → kein exakter Import, nachzeichnen.
  - **Jeder Anschluss trägt seine Nummer**, auch bei Fremdgeräten: NSHV-Abgänge als Anschlusspunkt
    mit „L1/L2/L3/PE“, Sicherungen mit ihren Klemmennummern 1/2, 3/4, 5/6 (Elementtext oben/unten
    rechts am `fusible`). Standardsymbole ohne Pinnamen nie unbeschriftet lassen.
  - Kennzeichen großer Kästen (Trafo) mit `set_element_text` x **und** y neben die obere linke
    Ecke („IN“-Höhe) legen; Motor-Kennzeichen dicht links neben den Kreis (x≈−42).
  - **Abzweige wie EPLAN**: kein QET-Verbindungspunkt (`bod`), wo das Original einen T-Knoten mit
    45°-Schräge zeigt; Abzweig-Symbole aus dem PDF (`Allgemein/Verbindungen/Abzweig_Plan`, Arten
    unten_links/unten_rechts/oben_rechts/rechts_unten, Helfer `abzweig.py`). Leitungsecken ohne
    Punkt über das unsichtbare „Ecke (Plan)“ (Anschlüsse N/W/E/S wie `bod`). Punkte nur dort, wo
    auch das Original einen Punkt hat. Vor jedem Blatt im Original nachsehen, welche Art es ist.
  - Fehler, die beim Vergleich auffallen, sofort ausbessern statt sie nur zu melden.
  - **Ganzer Plan aus einem fremden Vektor-PDF** (SEE Electrical, WSCAD, EPLAN …): `tools/pdf_import/pdf_projekt.py
    plan.pdf --nur-analyse` zeigt die erkannte Leiterstärke und die Seitenarten (Deckblatt/Listen werden übersprungen),
    `--ziel SAMMLUNG --ordner Import/Plan --aus OUT` schreibt je Schaltplanseite das JSON von `pdf_blatt.py` (dazu
    Blattnummer, Titel, Ort, Leiterfarbe nach IEC 60757, Drahtnummer, BMK) und `projekt.json` mit gepaarten
    Unterbrechungspunkten. Ob ein Bauteil Spule oder Kontakt ist, weiß die Grafik nicht: Verknüpfungen danach setzen.
  - **Blatt als Text** (QET-Text v1): `qet_folio_text` gibt ein Blatt zeilenweise aus (`bauteil -K1 typ=… @x,y uuid=…`,
    `leiter -S1:14 -K1:A1 nummer=3.01 farbe=BU`), `qet_folio_text_check` prüft einen geänderten Text mit Zeilennummern,
    `qet_folio_text_apply` ersetzt das Blatt (Bauteile mit uuid bleiben, Fehlendes wird gelöscht) oder hängt eines an.
    Für Änderungen an ganzen Blättern meist schneller als einzelne qet_edit-Operationen.
  - **Drahtnummern nach Muster** statt QET-Nummerierungsbausteinen: `qet.setWireNumberPattern({pattern:
    "{seite}.{n:2}", scope: "folio"|"cabinet"|"project", reserve: 3, order: "columns"|"rows", mode:
    "fixed"|"recalculate", active: true})`, dann `qet.numberWires()`; doppelte Nummern im Prüfpanel mit *Beheben*.
  - **SPS-Adressen je Station**: Elementinfo `plc_station` an den Modulen, `qet.setPlcStation("PLC1", {format:
    "siemens_de"|"siemens"|"iec"|"omron"|"twincat", order: "analog_first"})`, `qet.assignPlcAddresses(true)`;
    IO-Link-Port als Adresse `IOL:2/2` (Bytes ein/aus) eintragen.
  - **Netzwerk**: `qet.setIpNetworks("Maschine = 10.0.0.0/24, gw 10.0.0.1, from 10")`, `qet.assignIpAddresses()`,
    `qet.assignProfinetNames()`; Tabelle unter *Projekt > Netzwerkadressen …*.
  - **Ganze Blätter automatisch** (ab Blatt 36): `tools/symbole/pdf_blatt.py` im Fork setzt ein EPLAN-PDF-Blatt
    komplett um – Leitungen (waagerecht/senkrecht, Leiterstärke, freie Enden ausgeschlossen, Kästen erkannt)
    werden echte Leiter, Ecken/Abzweige Knoten (unsichtbare „Ecke“), zusammenhängende Grafik wird je ein Bauteil
    mit Anschlüssen genau an den Leitungsenden, Texte im Kasten gehören zum Bauteil, freie Texte/Ortsrahmen/Bilder
    gehen aufs Blatt. Blattskript-Muster: `blatt_auto.py <PDF-Seite>` im Projektordner (liest Titel,
    =Anlage, +Ort, Blattnummer aus dem Schriftfeld, zeichnet, erzeugt den Vergleich). Danach Vergleich ansehen
    und nur Auffälliges von Hand nacharbeiten.
- Beschriftung (Fork, Unveröffentlicht): Schilder für Phoenix/Weidmüller/WAGO kommen aus den Plandaten. Damit sie
  stimmen: Drahtnummern setzen (Drähte ohne Nummer bekommen kein Schild), Klemmen in eine `terminal_strip` legen
  und die Klemmenbeschriftung als `X1:3` oder `3` führen (das Schild zeigt `3`), Artikelnummer an der Klemme
  (wird die Materialreferenz), Kabel als echtes Kabel mit Bezeichnung. Export: `--export-marking <qet> <csv>
  --format phoenix|weidmueller|wago|neutral [--kinds …] [--folios N-M] [--plant/--location]` oder
  `qet.exportMarking()`. Spalten und Belege: `docs/beschriftung-export.md`.
- THM-Druck (Fork, Unveröffentlicht; Menü *Etikettendruck* neben Schaltschrankaufbau): dieselben Schilder direkt
  an einen Weidmüller THM (cab-JScript, TCP 9100). Nimmt ein Drucker auf 9100 die Verbindung an, druckt aber nichts
  (Etikettenzähler `cmd=3` bleibt stehen), Sendeweg LPD wählen: `--thm-transport <ID> lpd` (Port 515, Warteschlange
  `lp`, jeder Schritt bestätigt). Vor dem Senden Teilung (`ESC l`) mit `dy` der Vorlage vergleichen; Erfolg am
  Zähler der Weboberfläche prüfen, nicht an der TCP-Übergabe.
  Für Skripte nur Dateien erzeugen: `qet.exportThmJob(<prn>, '{"printer": "<ID>", "kinds": "wires"}')` oder
  `--thm-prn <qet> <prn> --printer <ID>`; ohne eingerichteten Drucker mit `--width/--height/--gap/--sensor`.
  **Nie ohne ausdrückliche Zustimmung des Nutzers drucken**: `--thm-send` sendet nur mit `--yes`; `--thm-status
  <ID>` fragt nur ab. Drucker/Material: `--thm-printers`. Belege und offene Punkte: `docs/beschriftung-export.md`,
  Abschnitt „THM direkt drucken“.
  Profil je Artikel aus der Layoutdatei (JSON aus der eigenen M-Print-Installation, nie ins Repo):
  `--thm-prn <qet> <prn> --material <Nr> --layouts <json> --family mmp|multimark`. Gerätegrenzen je Familie
  (MMP: Heizung −10…+20, 30–125 mm/s; MultiMark: −20…+20, 30–150 mm/s). Rüstungswechsel nur nach Bestätigung:
  `--thm-load <ID> <Nr> [--usual]`. Gelernt: lupdate findet `tr()` in anonymen Namensräumen nicht – dort
  `QCoreApplication::translate("Kontext", …)` ausschreiben; den Vergleich mit M-Print-`.prn` außerhalb des Repos
  halten (Herstellerdaten).

- Weidmüller-Druck: `--export-marking <qet> <ordner> --printer printjet|thm --mprint [--open] [--print]` schreibt je
  Material eine Datei für `MPrintPRO.exe <datei> -ImportFilter:CSV -MatNo:<nr>`. PrintJet CONNECT druckt nur MultiCards
  (MC), THM MultiMark nur Rollen (MM) und THM-Etiketten – Material nie mischen. Ohne installiertes M-Print PRO mit
  `--mprint-exe` auf ein Ersatzprogramm prüfen, das seine Argumente protokolliert.

## Data instead of hand-typed text: PLC channels and contacts like EPLAN

- **PLC channel**: bind the fields of the channel symbol to element information (`text_from="ElementInfo"`):
  `plc_address`, `plc_comment`, `plc_function`, `plc_t1`…`plc_t4`. The card (PLC master) holds the I/O table
  `<plcMasterData><plcIOs><plcIO type address functionText comment terminalCount><terminal>…`; each channel is tied
  to its row by `group_index` on its `link_uuid`. Edits to the table reach the channels. PLC slaves read their
  ElementInfo fields from themselves, other slaves from their master.
- `plc_tN` follows the order in which the fork hands out terminal labels: the slave's terminals **sorted by name**
  (numeric aware), e.g. "AUX" before "DI", "DQ-M" before "DQ-P".
- ET 200SP channel name and first terminal follow from card type and address: 16-channel cards count over two bytes
  (channel = (byte mod 2)·8 + bit), terminal = channel + 1. Second terminals (VS, M, AUX) are a wiring choice.
- **Contacts**: the contact mirror shows the terminal names of the contacts. A placed master may carry its own
  `<slaveContactGroups><group type="SW" terminalCount="3"><label>11</label>…` (overrides the symbol's groups); the
  contact links with `group_index`. Name changeover terminals by their IEC digit 1/2/4 with types Common/Nc/No so
  that sorting hands out 11/12/14 correctly.
- Cross reference position "text field" (`xrefpos="AlignHCenter"`) keeps the slave cross reference off the sheet.
- **Stamps**: title block variables `watermark` (diagonal) and `watermark-side` (upright, right edge), light grey,
  may use variables such as `%{printdate}` and `%{saveddate-de}`; empty = off.
- **Cable types from manufacturer catalogues**: every CSV in `cable-catalogs` beside the cable type file is a
  read-only catalogue in the type window (filters: catalogue, cores, core cross section, energy chain, properties).
  Build it from EPLAN article data with `tools/kabelkatalog/kabelkatalog_eplan.py` (cable article = function
  template category 400, cores = category 4 / id 4 with `connectionDesignation` as colour).
- **Every drawing is checked before it is shown**, test plans and pictures for an answer included:
  `qet_layout_check` over the whole project (no `folio`), then the PDF at 300 dpi in parts. Hide empty conductor
  numbers with `set_conductor_default displaytext=false` before the wires, on fluid and GRAFCET folios too.
- **GRAFCET**: the fork ships its own symbols (`bauteile/grafcet`, generator `tools/symbole/grafcet_symbole.py`,
  same paths as the collection). Step box 40 x 40 with terminals `in`/`out`/`action` on its edge; transition
  `in`/`out`, 20 high; an action's terminal `step` goes straight onto the step's `action` terminal (x + 20, no
  wire); parallel branch `in` at the top, branches `1`/`2` at ±100, synchronisation mirrored. On the 10 grid,
  step – transition – step every 50 px makes every wire straight and 20 px long. Step number, receptivity and
  action text are the labels (`set_label`).
- QElectroTech does not draw the terminal stub in PDF/PNG: a symbol's graphics must reach each terminal, or a gap
  shows.

## Steuerstromkreise – Muster aus den FluidSIM-Beispielen (gezählt 17.11.2026)

Gezählt über 229 Schaltungen mit 24 V, 0 V und Spulen (Elektropneumatik, Elektrohydraulik, Elektrotechnik).
Die Zahl in Klammern ist der Beleg; so zeichnen, wenn der Hausstil oben nichts anderes sagt.

- **24 V oben, 0 V unten** (189 von 229; der Rest sind quer liegende Leiterdiagramme nach NEMA).
- **Spulen unten, direkt an 0 V**: Relais- und Ventilspulen (1067 von 1072). Kontakte, Taster und Sensoren
  darüber, zur 24-V-Schiene hin.
- **Ein senkrechter Strompfad je Spule**, Abstand Median 96 FluidSIM-Einheiten ≈ 8 Raster. In QET mit
  Kontaktspiegel unter der Spule **mindestens 10–12 Raster** lassen, sonst überlappen die Spiegel der
  Nachbarpfade.
- **Signalteil links, Leistungsteil rechts**: zuerst die Pfade mit Sensoren/Tastern und Relaisspulen, rechts
  die Ventilspulen, geschaltet über Relaiskontakte (301 Ventilspulen hinter einem Relaiskontakt, 239 direkt
  hinter Taster oder Sensor; 153 Schaltungen trennen so Signal- und Leistungsteil).
- **Selbsthaltung** (31 Schaltungen): Ein-Taster -SF1 (Schließer 13/14) parallel zum Halteschließer der eigenen
  Spule (-KF1 13/14), der Aus-Taster -SF2 (Öffner 11/12) in Reihe hinter der Parallelschaltung, darunter die
  Spule -KF1 – Aus dominant. Ein zweiter Schließer der Spule (-KF1 23/24) schaltet im Nachbarpfad die
  Ventilspule.
- **Kontaktnummern** (erste Ziffer = Ordnungsnummer am Relais, zweite = Funktion):
  Schließer 13/14, 23/24 (134); Öffner 11/12, 21/22 (66); Wechsler 11/12/14, 21/22/24 (273); Taster
  13/14 oder 3/4 (336); Näherungsschalter dreiadrig 1 (+), 3 (0 V), 4 (Ausgang) (141); Endschalter als
  Wechsler 1/2/4.
- **Kennzeichnung**: Relais -KF (511 von ≈ 700), Ventilspule -MB (307), Taster -SF (310), Sensor/Endschalter
  -BG (166), Leuchte -PF oder -E, Spannungsquelle -G. Die Ventilspule trägt in QET als Slave die Kennzeichnung
  ihres Ventils (-QM1), die eigene (-MB1) bleibt in den Daten.
- **Nie Textkasten**: Ein Bauteil ohne passendes Symbol bekommt sein IEC-60617-Kennzeichen (Maschine: Kreis mit
  M 3~/MS 3~/G; Messgerät: Kreis mit V/A/W/cos φ; Gleichrichter: Kasten mit Diagonale und ~/=; Logik: &, ≥1, 1 mit
  Negationskreis; Regler: PID/PT1/Σ), nie einen Kasten mit (abgeschnittenem) Namen. Mehrpolige Schalter als
  einzelne Pole mit gestrichelter Wirkverbindung (Leitungsschutz: Kreuz am festen Kontakt; Motorschutz: Auslöser „I>“).
- **Nach jedem Zeichnen/Import `qet_layout_check` und Treffer zählen**: `wires_through_symbols`,
  `labels_on_wires` und `overlaps` sollen 0 sein. Leitungen durch Symbole mit `route: "avoid"` neu anlegen; eine Kennzeichnung auf
  einer Leitung mit `placeElementTexts(folio, [uuid], "right"|"left"|…)` an eine freie Stelle setzen, nicht das
  Bauteil verschieben.
- **Leitungen gerade, nur bei Bedarf herum** (24.11.2026, Vollzählung der FluidSIM-Beispiele): nicht jede Leitung
  mit `route: "avoid"` anlegen – das hält Abstand zu allen Symbolen und macht Umwege und Kreuzungen. Erst den
  geraden Weg, dann `qet_layout_check`; nur die Leitungen aus `wire_through_symbol` neu mit avoid anlegen
  (`route_conductor` erreicht Leitungen in Netzen nicht). Ein getrenntes Programm (LOGO, SPS-Baustein) gehört auf
  ein eigenes Blatt, nicht über die Schaltung. Eine mechanische Kopplung (Welle Motor ↔ Belastungsmaschine) ist
  eine Linie, keine Leitung.
- **Keine Simulationswerte im Plan**: angezeigte Messgrößen (P_out, I, U als Ergebnis) und Sollwerte 0 weglassen;
  nur Kennwerte (24 V, p = 6 bar, 47 kOhm) als Text am Bauteil.
- **Endschalter** stehen im Steuerstromkreis als Schließer/Wechsler mit -BG-Kennzeichen; am Zylinder sind sie
  nur eine Wegmarke (siehe Skill `fluidtechnik`).

## Redrawing foreign schematics: sPlan 8

- Importer `tools/splan_import/splan_nach_qet.py` (File > Import sPlan schematic); format in `FORMATBESCHREIBUNG_SPL8.md`.
- sPlan stores no nets or pins: connections come from line ends (end on a line or a junction dot = connected, crossing
  without dot = not). Assign arrow texts globally by distance, otherwise closely spaced arrows swap their targets.
- Redraw, do not trace: rows in the original's top-to-bottom order, a terminal strip as one block, each device above or
  below its partner. Fixed bands (terminals always at the bottom) make detours when the original feeds devices from above.
- Several wires between two rows: one track each (corner elements, `Blatt.leitung(ueber=…)`); QET's autoroute puts them
  on one height, which reads like a busbar.
- Never rewrite a generated symbol with new terminal UUIDs while `symbole.symbol` has it cached (add_conductor fails).
- Apply `qet_layout_check` fixes automatically; `tool_edit` needs a different output file.
