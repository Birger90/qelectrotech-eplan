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

## Gelernt (fortschreiben)

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
    x = Leitung − 1,3 · Größe, damit der Text mittig über der Leitung steht.
  - Pinnamen mit „-“ am Anfang an pdf_symbol.py als `--pin=-(1):x,y:s` übergeben.
  - Datenblatt-Schaltbilder (z. B. WAGO) sind oft Rasterbilder im PDF → kein exakter Import, nachzeichnen.
  - **Jeder Anschluss trägt seine Nummer**, auch bei Fremdgeräten: NSHV-Abgänge als Anschlusspunkt
    mit „L1/L2/L3/PE“, Sicherungen mit ihren Klemmennummern 1/2, 3/4, 5/6 (Elementtext oben/unten
    rechts am `fusible`). Standardsymbole ohne Pinnamen nie unbeschriftet lassen.
  - Kennzeichen großer Kästen (Trafo) mit `set_element_text` x **und** y neben die obere linke
    Ecke („IN“-Höhe) legen; Motor-Kennzeichen dicht links neben den Kreis (x≈−42).
