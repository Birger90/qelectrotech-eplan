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

## Revisionen wie bei EPLAN

- Menü Projekt > Revision > *Revision abschließen…*: Index (Vorschlag A, B, … bzw. 01, 02, …) und Kurztext. Der Index
  steht danach in jedem Schriftfeld (`%indexrev`), die Revisionstabelle in `%{rev1_index}`, `%{rev1_date}`,
  `%{rev1_text}`, `%{rev1_author}` (rev1 = neueste) – das Schriftfeld *revision* zeigt die letzten drei. Neben dem
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

## Kabel

- Menü Projekt > *Kabelliste (Typen wählen)…*: alle Kabel des Projekts mit Typ, Adern, Farben, Anlage, Ort und
  Blättern; *Typ wählen…* ändert den Typ in einem Rückgängig-Schritt.
- Herstellerkataloge: Jede CSV im Ordner `cable-catalogs` neben der Kabeltypdatei wird als schreibgeschützter
  Katalog gelesen. Filter nach Katalog, Aderzahl, Aderquerschnitt, Energiekettentauglichkeit und Eigenschaften
  (geschirmt, halogenfrei, UL/CSA, torsionsfest …).
- `tools/kabelkatalog/kabelkatalog_eplan.py` erzeugt solche Kataloge aus den frei erhältlichen EPLAN-Artikeldaten der
  Hersteller (z. B. igus chainflex, LAPP): Adern mit Farben, Querschnitt, Außendurchmesser, Spannung, Artikelnummer.

## Schaltschrankaufbau und Tafelfertigung

Menü *Schaltschrankaufbau* – der 2D-Ablauf von EPLAN Pro Panel auf einem normalen Blatt:

- *Schaltschrank aufbauen…*: der ganze Aufbau in sechs geführten Schritten mit Haken – Schrank mit Montageplatte
  (Rittal über die Bestellnummer), Hutschienen und Kanäle als Raster (Kanäle am Rand und zwischen den Reihen, eine
  Schiene je Reihe), Geräte und Klemmenleisten aus dem Schaltplan, Prüfung, 3D und Export. Die Einzelbefehle
  stehen unter *Einzelschritte*.
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
    (`face="door|left|right"`), ihre Geräte zeigen nach außen.
  - Zubehör (`accessories`, `artikel_N` am Schaltplan-Gerät) wird mit eingebaut, z. B. die ET-200SP-BaseUnit unter
    ihrem Modul; Klemmenleisten zeigen jede Klemme der QElectroTech-Klemmenleiste mit ihrem Modell.
  - Kanäle ohne Modell als geschlitzte Kanäle mit Deckel. Export als glTF (`.glb`) für jeden 3D-Betrachter.
- `bohrbilder.csv` in Firmen- oder Benutzersammlung: Bohrungen, Gewinde und Ausschnitte für Teile ohne Bohrbild
  (Bestellnummer mit Platzhaltern, Maße von der Gerätemitte, Quelle je Zeile – Werte aus Datenblättern).
- Skript-API: `setShapePanel`, `shapePanel`, `panelReport`, `exportPanelDxf`, `stepInfo`, `exportPanel3D`,
  `addPanelGrid`.

## EPLAN-Projekte und eingebauter Assistent

- Datei > *EPLAN-Projekt importieren* (`.zw1`): das ganze Projekt als verknüpftes QET-Projekt – Formulare und
  Schriftfelder, SPS-Karten als SPS-Master mit Kanaltabelle, symbolische Adressen wie in EPLAN verfolgt, Bericht über
  nicht verknüpfte Kanäle (`sps_bericht.txt`).
- Claude-Assistent als Andockfenster (Knopf *Claude* in der Hauptwerkzeugleiste): liest und ändert das offene Projekt
  über den MCP-Server unten; Bilder und PDFs lassen sich mitschicken. Mit API-Schlüssel oder in eigenen Builds
  (`-DQET_CLAUDE_KONTO=ON`) über das eigene Claude-Konto.
- *Klemmleiste zeichnen*: ein Strich quer über Leitungen setzt auf jede eine Klemme aus dem Klemmenkatalog (Filter,
  maßstäbliche Vorschau, Foto), auch Mehrstockklemmen (L/N/PE).
- Herstellerportale (Siemens, Rittal, 3Dfindit, SEW, WAGO, Keyence): Konten im Schlüsselspeicher des Systems, fehlende
  Bauteile automatisch holen (`--fetch-parts`). Läuft unter Windows und macOS (Workflow `macos-build.yml`).

## Pneumatik, Hydraulik und Simulation

Anders als im allgemeinen Teil über QElectroTech beschrieben, simuliert dieser Fork.

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
  Vorwahlzähler, Zweihandsteuerblock, Vakuumsaugventil.
  Leitungen unter Druck werden blau bzw. rot, stromführende Drähte rot, aktive Schritte bekommen eine Marke; Taster und
  Ventile bedient man im Fenster oder per Klick auf das Symbol; eine Liste zeigt die SPS- und GRAFCET-Variablen.
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
