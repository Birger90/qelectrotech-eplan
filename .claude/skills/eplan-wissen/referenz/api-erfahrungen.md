# EPLAN-API: Erfahrungen am laufenden EPLAN (eplan-mcp, Ordner Testing)

**Art der Quelle:** Das hier sind **Beobachtungen am laufenden EPLAN über die API** (EPLAN 2025 und 2027,
DataModel über Remoting bzw. In-Process-Add-in), aufgezeichnet von den Autoren des MCP-Servers *eplan-mcp*
(Canary-ECAD, MIT-Lizenz) beim Zeichnen von Testschaltungen in Wegwerf-Kopien eines Projekts. Es ist **nicht die
offizielle EPLAN-Hilfe**. Jede Aussage wurde dort durch Platzieren und Zurücklesen (`live_read_page`,
`live_read_connections`) gemessen, nicht am Bildschirm abgelesen. Werkzeugnamen `live_*`/`eplan_*` sind die des
eplan-mcp-Servers, Klassen und Eigenschaften (`Function`, `MacroBox`, `PointD` …) die der EPLAN-API.
In eigenen Worten und stark gekürzt; Widersprüche zu anderen Dateien dieses Skills sind als **Korrektur:** bzw.
**Widerspruch:** markiert.

**Quellen-Präfix:** `https://github.com/Canary-ECAD/eplan-mcp/blob/main/Testing/` – im Text steht nur der Dateiname,
z. B. „Quelle: 02-routing-and-connections.md“.

## Inhalt

1. Koordinatensystem, Raster, Einheiten
2. BMK und Kennzeichnung
3. Platzierung per Skript gegenüber interaktiv
4. Verbindungen, Autoconnect, Verbindungssymbole
5. Makrokasten
6. Hauptfunktion und Funktionstext
7. Symbolkataloge, Symbolnummern, Symbolübersicht
8. Nachbau einer echten Seite
9. Seite als ASCII, Bild zu Schaltplan
10. Bekannte Fallen und Lücken der API
11. Übersicht der Widersprüche und Korrekturen
12. Quellen

---

## 1. Koordinatensystem, Raster, Einheiten

- Seitenkoordinaten der API (`PointD`) sind **Millimeter, Ursprung unten links, +Y nach oben**. Gemessen mit vier
  Bezugsklemmen in den Ecken eines A3-Rahmens: x 0–420, y 16 (unten) bis 292 (oben). Passt dazu: Der „Up“-Anschluss
  eines Symbols hat immer positiven Y-Versatz (Schließer-Variante 0: oben (0,+4), unten (0,−4)).
  Quelle: 03-coordinate-system-and-designations.md
- Wer Bereiche „in Lesereihenfolge“ (oben zuerst, Y nach unten) beschreibt, liefert gespiegelte Y-Werte. Schnellste
  Klärung je Projekt: vier billige Bezugssymbole in die Rahmenecken setzen und zurücklesen.
  Quelle: 03-coordinate-system-and-designations.md
- Anschlusskoordinaten aus dem Symbolkatalog („raw“) sind **relativ zum Einfügepunkt** des Symbols; erst nach dem
  Platzieren gibt es absolute Seitenpunkte. Quelle: 01-schematic-authoring-primitives.md
- Es gibt **keine Verschiebe-Funktion**: Verschieben = Entfernen und an `Position + (dx, dy)` neu platzieren (dx/dy auf
  das Seitenraster `gridSize` runden), neu benennen, Verbindungen neu erzeugen. Alte Handles sind danach ungültig.
  Quelle: 03-coordinate-system-and-designations.md
- **Widerspruch (zur Quelle, nicht zum Skill):** Die Quelle hält „+Y oben“ für undokumentiert. Die Hilfe sagt es
  (schaltplan-editor.md Abschnitt 1, Koordinatensysteme): intern liegt der Ursprung immer unten links; nur die
  *Anzeige* der Elektrotechnik-Seite zählt RX/RY von links oben in Rasterschritten. Die API liefert also die interne
  Sicht in mm, nicht die Statusleisten-Anzeige.

**Bedeutung für QET:** QET-Szenen haben Y nach unten. Beim EPLAN-Import (`tools/eplan_import`, `.epdz`) und bei
jedem Weg über die API muss `y_qet = Blatthöhe − y_eplan` gerechnet werden (dxf-dwg-druck.md Abschnitt 10 zeigt
dasselbe am `.epdz`-SVG mit `matrix(1,0,0,-1,0,297)`). mm → px weiter über das 4-mm-Raster ↔ 10 px. Beim Nachzeichnen
aus dem PDF ist der Rahmen (Ecken) der Bezug, wie hier die vier Klemmen.

## 2. BMK und Kennzeichnung

- `Function.Name` ist direkt schreibbar und nimmt die **volle Kennzeichnung** `=Anlage++Aufstellungsort+Einbauort-BMK`
  wörtlich an (z. B. `=CTRL++CAB1+MANDO-K1`), ohne Umformatierung. Wer nur `-K1` schreibt, bekommt `=+++-K1` zurück
  (leere Segmente, ggf. mit von der Seite geerbtem Ort) – der Aufruf gelingt trotzdem, die Lücke fällt erst später auf.
  Quelle: 03-coordinate-system-and-designations.md
- Ein neu platziertes Symbol heißt zunächst nur `"+"` (anonym) – kein Fehler. Quelle: 01-schematic-authoring-primitives.md
- **Gleiches BMK = gleiches Gerät:** Ein zweites Symbol mit schon vorhandenem BMK wird als weitere Funktion desselben
  Betriebsmittels zusammengeführt (Spule + drei Hauptkontakte + Hilfskontakt = ein Gerät `-K1` mit vier Funktionen);
  `live_read_connections` ordnet danach alle Anschlüsse `-K1` zu. Quelle: 01-schematic-authoring-primitives.md
- Falle beim Vergleich: EPLAN speichert das BMK in der Form der Projektstruktur (`=+++-K4`), ein Vergleich mit dem
  angefragten `-K4` schlägt fehl. Duplikat-Prüfungen gegen die **gespeicherte** Form führen. Quelle: 08-replicating-a-real-page.md
- `FUNC_FULLDEVICETAG` (20006, BMK vollständig) ist **schreibgeschützt** (`SettingValueFailedException S511113`);
  das BMK setzt man über `Function.Name`, die Struktursegmente folgen daraus. Quelle: 12-main-function-and-function-text.md

**Bedeutung für QET:** Bestätigt kennzeichnung.md: Schlüssel ist immer die volle Kennzeichnung, Anzeige normalisieren.
Leere Segmente wie `=+++-K1` beim Import auf `-K1` (plus geerbte Struktur) zurückführen, nie als eigenes Präfix
übernehmen. Für `qet_mcp`: ein Werkzeug, das BMK setzt, sollte wie EPLAN „gleiches BMK = gleiches Gerät“ behandeln
(Haupt-/Nebenfunktion verknüpfen) und Duplikate nach Normalisierung prüfen.

## 3. Platzierung per Skript gegenüber interaktiv

- **Skript-Weg** `Function.Create(Page, SymbolVariant, PointD, PointD)`: ohne Oberfläche, in rund 20 Aufrufen
  fehlerfrei, das Symbolbild erscheint normal im Editor. Quelle: 01-schematic-authoring-primitives.md,
  04-interactive-vs-scripted-placement.md
- **Interaktiver Weg** `XEGActionInsertSymRef` (wie F3 „Symbol einfügen“): Parameter `SymbolLibName`, `SymbolId`,
  `VariantId`, `FctDefTag`, `Placementmode`, `SymbolType`, `CustomSymbols` – **keine Koordinate**. Die Aktion macht
  nur den Platziermodus scharf; erst ein Mausklick setzt das Symbol. `SymbolId` ist die numerische Symbolnummer, nicht
  der Kurzname. Quelle: 04-interactive-vs-scripted-placement.md
- Gemessen: Nach dem Aufruf reagierte EPLAN gar nicht mehr (Ping tot), erst ein Neuverbinden half – auf einem anderen
  Remoting-Port, also mit neu gestartetem Host. Danach stand ein Gerät **14 mm/2 mm versetzt** gegenüber der vorher
  gemeldeten Lage, ohne jede Fehlermeldung. Quelle: 04-interactive-vs-scripted-placement.md
- Regel: Nach jeder interaktiven Aktion oder jedem Neuverbinden Seite und Verbindungen **neu lesen** und mit dem
  letzten bekannten Stand vergleichen. Quelle: 04-interactive-vs-scripted-placement.md
- Hilfsfunktion „verbunden platzieren“: das neue Symbol so setzen, dass sein Anschluss mit Abstand d auf der Achse eines
  vorhandenen Anschlusses liegt. Klappt nur, wenn der Anker eine `Function` ist, nicht an Winkeln/T-Stücken
  (`SymbolReference`); dort den Anschlusspunkt (meist = Einfügepunkt) selbst rechnen. Quelle: 01-schematic-authoring-primitives.md

**Bedeutung für QET:** Für `qet_mcp` ist der Skript-Weg das Vorbild: Platzieren mit Koordinate, danach immer
zurücklesen (wie `qet_elements`/`qet_diff`). Live-Aktionen (`qet_live_*`) sollten nach jedem Schritt den Stand
neu lesen statt sich auf frühere Rückgaben zu verlassen. Eine Hilfe „am Anschluss X im Abstand d anbauen“ spart beim
Nachzeichnen die Koordinatenrechnung.

## 4. Verbindungen, Autoconnect, Verbindungssymbole

- **Kein Leitungsobjekt nötig:** Zwei fluchtende, gegenüberliegende Anschlüsse verbindet EPLAN selbst. Alle Knoten
  sind platzierte **Symbole** (`SymbolReference`), keine Linien. Explizite Linien (`DynamicConnectionLine`) nur, wo
  Ausrichten nicht geht. Quelle: 01-schematic-authoring-primitives.md, 02-routing-and-connections.md
- Verbindungssymbole (Kurzname, Nummer in `SPECIAL`, `Symbol.Type`): Winkel `CO` 70 (Routing, 4 Varianten =
  4 Quadranten; v0 rechts+unten, v1 rechts+oben, v2 links+oben, v3 links+unten), T-Stück oben `TLRO` 72 (auch
  `TLRO_1`, andere Anschlussreihenfolge), unten `TLRU` 64, links `TOUL` 67, rechts `TOUR` 66, Kreuzung `CR` 69,
  Brücke `BR` 68, Diagonale `CRL` 71, Abbruchstelle `BP` 8, Verbindungsdefinitionspunkt `CDP` 308 (19 Varianten,
  u. a. ohne Grafik, Lichtwellenleiter, Rohrleitungen `CDPCP2F1`–`F5`). Drei unabhängige Quellen (Projekt,
  Symbolbrowser, öffentlicher Datensatz) nennen dieselben Nummern. Quelle: 02-routing-and-connections.md,
  05-external-symbol-dataset.md
- Beim Winkel liegen beide Anschlüsse **genau auf dem Einfügepunkt**. T-Stücke wählt man über den Typ (Richtung),
  nicht über die Variante; `TOUR` hat 5 Varianten, nur **Variante 8** hat alle drei Beine bei (0,0) und sitzt damit
  sauber auf einem Anschluss. Quelle: 02-routing-and-connections.md
- **Widerspruch:** schaltplan-editor.md Abschnitt 3 nennt nach der Hilfe „T-Stück in 4 Richtungen × 4 Varianten“.
  Beobachtet hat `TOUR` 5 Varianten (darunter v8); die Hilfe zählt offenbar nur die im Dialog wählbaren
  Darstellungen, die Bibliothek hält mehr.
- **Geometrie ist keine Elektrik:** Erst die Aktion `generate` mit `TYPE:CONNECTIONS` (`REBUILDALLCONNECTIONS:1` für
  Neuaufbau) erzeugt `Connection`-Objekte; erst deren Zurücklesen beweist, was verbunden ist (nach BMK, Anschluss,
  Bezeichnung). Beispiel Selbsthaltung: 5 Verbindungen, der Parallelzweig ist echt, weil der Hilfskontakt an denselben
  zwei Knoten liegt wie der Taster. Quelle: 02-routing-and-connections.md
- **Stille Umgehung:** Liegt ein Gerät nur 2 mm neben der Achse, meldet `generate` keinen Fehler, sondern verbindet um
  das Gerät herum (Kette S0 → S1 statt S0 → F1 → S1). Erkennbar nur daran, dass das BMK in keiner Verbindung vorkommt.
  Quelle: 02-routing-and-connections.md, 04-interactive-vs-scripted-placement.md
- `live_read_connections` meldet als Endpunkt die **Lage des Geräts**, nicht des Anschlusses (Gerät bei y 106,
  Anschlüsse bei 110 und 102). Quelle: 07-page-to-ascii.md

**Bedeutung für QET:** Bestätigt schaltplan-editor.md Abschnitt 2: Im EPLAN-Original ist eine Verbindung dort, wo
Anschlüsse fluchten – der Import muss daraus echte QET-Leiter machen; Winkel/T-Stücke sind keine Leiter, sondern
Knoten. Die Varianten mit allen Beinen bei (0,0) entsprechen einem QET-Leiterknoten. Nach dem Nachzeichnen nie der
Zeichnung trauen: `qet_conductors`/`qet_continuity` sind das Gegenstück zu `live_read_connections`, und jedes Gerät,
das in keiner Verbindung vorkommt, ist verdächtig. `qet_layout_check` deckt nur die Geometrie ab (Leiter gerade,
Anschlüsse genau in Linie), nicht die Frage, ob jedes Gerät verbunden ist.

## 5. Makrokasten

- `MacroBox` ist eine eigene Platzierungsart neben `Function` und `SymbolReference`; sie entsteht z. B., wenn man in
  der Oberfläche eine Auswahl zum Makro macht. Sie hat **keine Eigenschaft `Name`**. Quelle: 06-macrobox-reflection.md
- Der Makropfad steht in **`MacroBox.MacroName`** (String, relativer Pfad zur `.ema`), die Variante in
  `MacroBox.Variant` (Zahl als Text, z. B. `"0"`). Direkt per Reflection schreibbar und zurückgelesen bestätigt.
  Quelle: 06-macrobox-reflection.md
- Umbenennen von `MacroName` ist **nur ein Etikett**: Ein gefüllter Kasten (12 referenzierte Objekte: 5 Verbindungs-
  symbole, 2 Funktionen, 2 Verbindungsdefinitionspunkte, 1 Pfad-Funktionstext, 1 Abbruchstelle) behielt alle Objekte,
  `IsValid`/`IsPlaced` blieben wahr. Ob ein Zeigen auf eine **andere** echte Makrodatei je nachgeladen wird, ist
  ungeprüft; wahrscheinlicher Weg ist eine Methode (`InsertMacroBox`) oder die Aktion `XMIaSwapMacro`.
  Quelle: 06-macrobox-reflection.md
- Die **Beschreibung** des Makros ist über das Objektmodell nicht erreichbar (weder am Kasten, noch an seiner
  `SymbolVariant`, noch in `MacroBox.Properties` – leer). Schluss der Autoren: Sie steckt nur in der `.ema`-Datei.
  Quelle: 06-macrobox-reflection.md
- **Widerspruch:** makros-artikel.md Abschnitt 4 führt nach der Hilfe „Beschreibung“ als Feld des Makrokastens. Am
  platzierten Kasten liefert die API sie nicht; das Feld ist also Datei- bzw. Dialogdaten, nicht Platzierungsdaten.
  Ebenfalls dort: „In Schaltplanprojekten kann man keine Makrokästen einfügen“ – beobachtet wurden sie trotzdem auf
  Schaltplanseiten; das passt zur Hilfe-Aussage, dass der Kasten beim Einfügen mitgespeichert wird (referenzierend).

**Bedeutung für QET:** Beim EPLAN-Import ist ein Makrokasten eine Gruppe mit Herkunft (`.ema`-Pfad + Variante), keine
Funktion – als Gruppe bzw. Elementinformation „Makro“ mitführen, damit Funktionsvorlagen (`.qetmak`) später erkennbar
bleiben. Die Beschreibung kommt aus der `.ema` selbst (`eplanmacroreader.cpp`), nicht aus dem Projekt.

## 6. Hauptfunktion und Funktionstext

- **Funktionstext löschen** (`FUNC_TEXT`, 20011): `SmartLock()`, dann leeren mehrsprachigen Text setzen. Lesen über die
  Mehrsprachform, damit auch Texte nur in einer Fremdsprache zählen. Unsichtbar bleibt das Löschen nur, wenn die Funktion
  **nicht Hauptfunktion** ist (Auswertungen wie die Geräteliste nehmen den Text der Hauptfunktion) und keine
  Eigenschaftsplatzierung 20011 oder 20031 zeigt – gemessen 625 Funktionen geleert, 0 Textänderungen im PDF. Texte aus
  Makros kommen beim nächsten Makro-Abgleich zurück. Quelle: 12-main-function-and-function-text.md
- **Hauptfunktion verlegen** (auf eine neue, nicht platzierte Funktion): alles in einem `LockingStep`; **vorher alle
  Funktionen mit diesem BMK und deren Artikelreferenzen sperren**, sonst `LockingException`, weil EPLAN beim
  Umschalten von `IsMainFunction` die übrigen Funktionen des Geräts mit umschreibt. Neue Funktion mit
  `Function.Create(Funktionsdefinition, Projekt)`, Name setzen, Eigenschaften kopieren (20006 auslassen), dann
  `IsMainFunction` umschalten und Artikel umhängen. Bei Fehler die halbfertige Funktion wieder entfernen.
  Quelle: 12-main-function-and-function-text.md
- Nach vollständiger Kopie unterscheiden sich nur platzierungs- und verbindungsabhängige IDs (19002–19023, 20002,
  20010, 1229, 46434/46435, 59968; 45xxx/46xxx/48xxx Verbindungsziele, 20038, 20219, 20342; 20221 Breite, 20470
  „im Schaltplan platziert“, 20122/20612 Hauptfunktions-Kennzeichen). Quelle: 12-main-function-and-function-text.md
- **Artikel steuern Beschriftung:** Zieht man die Artikel von einer platzierten Funktion ab, verschwinden alle Texte, die
  sie aus Artikeldaten zeigt (Bemessungswerte, Bestellnummern; 214 Texte auf 14 Seiten). Quelle:
  12-main-function-and-function-text.md
- Nachweis „keine sichtbare Änderung“: PDF vorher/nachher mit festem Schema s/w, Texte je Seite als Multimenge
  vergleichen, zusätzlich Pixelvergleich (nur Plotdatum darf abweichen). Quelle: 12-main-function-and-function-text.md

**Bedeutung für QET:** Bestätigt betriebsmittel.md (nur die Hauptfunktion trägt Artikel; EPLAN schaltet die
Hauptfunktion anderer Funktionen selbst um). Beim Import: Texte, die im PDF am Symbol stehen, können aus Artikeldaten
stammen – fehlt der Artikel, fehlt der Text; also Artikel an der Hauptfunktion mitführen, sonst stimmt das Bild nicht.
Der PDF-Text- und Pixelvergleich ist genau das Prüfverfahren für QET-Nachbauten gegen das Original-PDF.

## 7. Symbolkataloge, Symbolnummern, Symbolübersicht

- Symbolnamen sind nicht erratbar und nicht überall gleich; in `IEC_symbol`: `S` Schließer, `O` Öffner, `SL`
  Hauptkontakt-Schließer, `OL` Hauptkontakt-Öffner, `K` Spule allgemein, `SSA`/`SOA` Taster Schließer/Öffner, `FT1`
  Thermorelais einpolig, `SONOT1` Not-Halt Öffner, `X` Klemme. Quelle: 01-schematic-authoring-primitives.md,
  05-external-symbol-dataset.md
- Katalog in drei Tiefen: Bibliotheken → Symbole mit Anschlusszahl → Varianten mit Anschlussgeometrie. Praktischer Filter
  „verdrahtbar“: mindestens 2 Anschlusspunkte. Quelle: 01-schematic-authoring-primitives.md
- **Symbolnummern sind lückenhaft:** `SPECIAL` hat 0–72 lückenlos, dann erst wieder z. B. 308, 338, 402; insgesamt 452
  Symbole. Ein Durchlauf, der beim ersten Loch abbricht, sah nur 73 und meldete trotzdem „vollständig“ (inzwischen in
  eplan-mcp behoben). „Nicht in der Liste“ ist kein Beweis für „nicht vorhanden“; die Abfrage per exaktem Namen ist
  maßgeblich. Quelle: 09-symbol-catalog-enumeration-gap.md
- **Korrektur:** symbole.md Abschnitt 2 beschreibt die Symbolnummer nach der Hilfe als „Position in der Bibliothek,
  Start 0“. Das legt eine lückenlose Folge nahe; beobachtet ist sie **dünn besetzt** – eine Nummer ist eine Kennung,
  keine Zeilenposition. Wer Bibliotheken liest, muss Lücken überspringen.
- Externer Datensatz *covaga/electrical-symbols-dataset* (Hugging Face, rund 33.500 Zeilen IEC/NFPA/GB mit Bild,
  Kurzname, Nummer, Beschreibung, Variante A–D): liefert die **Bedeutung**, die die API nicht hat; seine Nummer ist die
  `SymbolId` (z. B. `SL` 0, `K` 20, `SSA` 159). Keine Anschlussgeometrie, keine Bibliothek, Nummer nicht global
  eindeutig, Groß-/Kleinschreibung uneinheitlich (`co` statt `CO`). Nur als Kandidatenliste, jede Wahl am Projekt
  bestätigen. Quelle: 05-external-symbol-dataset.md
- Auswertung **Symbolübersicht** (Seitentyp `SymbolOverview`, je Symbol vier Drehungen) als zweiter Weg zum Inventar:
  104 Seiten → 11.269 Zeilen (eine je Anschluss: Bibliothek, Symbol, Typ, Variante, Anschluss, Versatz, Richtung).
  Kein Symbol doppelt. Auffällig: Symbole mit allen Anschlüssen bei (0,0) – bei Verbindungssymbolen richtig (Knickpunkt
  = Einfügepunkt), bei Geräten verdächtig; Entscheidung je Symbol durch Sichtprüfung, nie automatisch.
  Quelle: 11-symbol-overview-inventory.md

**Bedeutung für QET:** Bestätigt symbole.md: Makros und `.epdz` verweisen über Bibliothek + Symbolnummer + Variante;
der Import (`tools/eplan_import`) muss Lücken in der Nummernfolge vertragen und darf ein fehlendes Symbol erst nach
direkter Suche als fehlend melden. Ein Inventar „Symbol → Anschlussversätze je Variante“ ist die Zuordnungstabelle
EPLAN-Symbol ↔ QET-Element; eine Symbolübersicht im Original-PDF kann dafür als Lesevorlage dienen.

## 8. Nachbau einer echten Seite

Versuch: eine Sicherheitsschaltungsseite (STO-Ketten, Klemmleiste mit 20 Klemmen, Kabel zu Steckerstiften) aus einem
Foto nachbauen. Die Kontaktketten gelangen und wurden elektrisch bestätigt (6 Verbindungen); vier Dinge scheiterten.
Quelle: 08-replicating-a-real-page.md

- **Aktives Projekt kann wechseln:** Zwei Werkzeuge sahen im Abstand von Minuten verschiedene Projekte, weil der Nutzer
  in EPLAN umgeschaltet hatte. Vor jeder Schreibserie aktuelles Projekt prüfen. Quelle: 08-replicating-a-real-page.md
- **Klemmen lassen sich mit `Function.Create` nicht platzieren:** `IEC_symbol/X` wird zu einem `Terminal`, das wirft
  `NotImplementedException`. Ein Geräteanschlusspunkt (`SPECIAL/DCP2JICM`) sieht ähnlich aus, ist aber `Function` und
  geht. Der Katalog meldet beide als `Function` – vorher nicht erkennbar. **Schlimmer:** der Fehlschlag hinterlässt ein
  verwaistes `Terminal` bei (0,0). Nach jedem gescheiterten Platzieren die Objektzahl der Seite prüfen.
  Quelle: 08-replicating-a-real-page.md
- **Zwischen zwei Abbruchstellen entsteht keine Verbindung**, selbst senkrecht fluchtend ohne etwas dazwischen; alle
  gelungenen Verbindungen hatten mindestens ein `Function`-Ende. Ob das an fehlenden Namen liegt, blieb offen.
  Quelle: 08-replicating-a-real-page.md
- **Abbruchstellen ließen sich nicht benennen** – das Werkzeug setzt Namen nur an `Function`. Quelle:
  08-replicating-a-real-page.md
- **Widerspruch (Einordnung):** querverweise.md beschreibt Abbruchstellen mit Namen, die seitenübergreifend paaren. Die
  fehlende Benennung ist also eine Grenze des eplan-mcp-Werkzeugs, nicht von EPLAN; unbenannte Abbruchstellen sind
  vermutlich schlicht unvollständig. Bestätigt dafür schaltplan-editor.md Abschnitt 3: Abbruchstellen haben keine
  Funktionsdefinition, sind also keine `Function`.
- Nicht nachgebaut: Klemmleiste, 0-V-Zweig, Potentialnamen, Kabel mit Steckerstiften, Querschnittsangaben (Eigenschaften
  des Verbindungsdefinitionspunkts – kein Werkzeug setzt sie), Pfad-Funktionstexte. Zuordnung Schiene → Spalte war aus
  dem Foto nicht sicher lesbar und ist als Annahme vermerkt. Quelle: 08-replicating-a-real-page.md

**Bedeutung für QET:** Beim Nachzeichnen sind Klemmen, Abbruchstellen mit Namen, Kabel und Verbindungsdefinitionspunkte
genau die Teile, die ein reiner Symbol-Platzierer nicht kann – in `qet_mcp` müssen Klemmen (terminal_strip), Querverweis-
Abbruchstellen und Leiterdaten eigene Werkzeuge haben. Nicht Ablesbares im Original als Annahme markieren
(Offene_Fragen.txt), nicht still raten.

## 9. Seite als ASCII, Bild zu Schaltplan

- **Seite als Zeichenkarte** (`page_to_ascii.py`): rendert das Lese-Ergebnis als Raster aus Einzelzeichen plus Legende
  (Geräte als Buchstaben, Winkel `┘└┌┐`, T-Stücke `┬┴├┤`, Kreuzung `┼`, Abbruchstelle `»`, Verbindungsdefinitionspunkt
  `=`, Kästen `·`), mit mm-Linealen. Lageprobleme (Gerät eine Spalte neben der Achse, fehlender Zweig) sieht man sofort
  statt Koordinatenpaare zu vergleichen. Quelle: 07-page-to-ascii.md
- Zeichen für Verbindungssymbole werden aus den **Anschlussrichtungen** abgeleitet, nie aus dem Namen ({links, oben} →
  `┘`); unbekannte Kombinationen als `?` und gelistet. Verluste werden gemeldet: überdeckte Zellen, nicht gerade
  Verbindungen (werden nicht gezeichnet, nur gelistet), abgeschnittene Lesungen. Grafik ist standardmäßig aus, weil
  Produktivseiten überwiegend aus Polylinien bestehen. Quelle: 07-page-to-ascii.md
- **Bild → Schaltplan** (nur Entwurf): Weil EPLAN selbst verbindet, braucht man aus dem Bild nur **Symbol + Variante +
  mm-Lage**; Leitungen sind kein Eingang, sondern **Prüforakel** (Soll aus Bild gegen Ist aus den Verbindungen).
  Stufen: Linien per Morphologie entfernen, Symbolblöcke ausschneiden, Vorlagenvergleich in 8 Lagen, OCR für BMK und
  Anschlussnummern, Pixel → mm über vier bekannte Eckpunkte (Homographie). Quelle: 10-image-to-schematic-pipeline.md
- Bild kann **Variante** und **Bibliothek** nicht liefern: Varianten sehen gleich aus und unterscheiden sich nur in
  Anschlussversätzen (T-Stück v0 vs v8; Schließer v0 vs v2 mit vertauschten Anschlüssen). Lösung: gemessene
  Andockpunkte der Leitungen gegen die Versätze aus dem Projekt-Inventar (Abschnitt 7) vergleichen. Regel: **eingrenzen,
  nie entscheiden** – Kandidaten mit Bewertung ausgeben, Bestätigung vor dem Schreiben. Quelle:
  10-image-to-schematic-pipeline.md

**Bedeutung für QET:** Für das Nachzeichnen aus PDF/Foto ist das die passende Arbeitsteilung: Symbole und Lage aus dem
Bild, Leitungen nur zur Kontrolle gegen `qet_conductors`. Eine ASCII-Karte aus `qet_items`/`qet_elements` wäre ein
billiges Prüfwerkzeug für Achsfehler im MCP-Server. Bei EPLAN-PDFs mit Textebene (pymupdf) ersetzen die Texte die OCR.

## 10. Bekannte Fallen und Lücken der API

- Ein gescheitertes Platzieren kann ein Objekt bei (0,0) hinterlassen (nicht atomar). Quelle: 08-replicating-a-real-page.md
- `generate` (Verbindungen) verbindet still um versetzte Geräte herum, meldet nie eine nicht erzeugte Verbindung.
  Quelle: 02-routing-and-connections.md
- `XEGActionInsertSymRef` hat keine Koordinate und kann EPLAN hängen lassen. Quelle: 04-interactive-vs-scripted-placement.md
- Nach Neuverbinden können Lagen abweichen; frühere Rückgaben sind keine Wahrheit. Quelle: 04-interactive-vs-scripted-placement.md
- Bibliotheksdurchlauf muss Lücken in den Symbolnummern überspringen. Quelle: 09-symbol-catalog-enumeration-gap.md
- Der Katalog sagt nicht voraus, ob ein Symbol als `Function` oder `Terminal` entsteht. Quelle: 08-replicating-a-real-page.md
- Lesefunktionen mit Obergrenze: Auf Produktivseiten erschöpfen Grafikobjekte die Grenze vor dem ersten Gerät – nach
  Typ `Function` filtern. Quelle: 01-schematic-authoring-primitives.md, 07-page-to-ascii.md
- `MacroBox` ohne `Name`, Makrobeschreibung nicht lesbar. Quelle: 06-macrobox-reflection.md
- 20006 schreibgeschützt; `IsMainFunction` umschalten braucht Sperre aller Gerätefunktionen. Quelle:
  12-main-function-and-function-text.md
- BMK-Vergleich gegen die gespeicherte Strukturform, nicht gegen die Eingabe. Quelle: 08-replicating-a-real-page.md
- Querschnitt/Farbe am Verbindungsdefinitionspunkt, Abbruchstellen-Namen, Klemmen, Kabel: mit den geprüften Werkzeugen
  nicht setzbar. Quelle: 08-replicating-a-real-page.md
- Aktives Projekt ist Sitzungszustand und kann wechseln; Schreibzugriffe nur auf Wegwerf-Kopien. Quelle: 00-INDEX.md,
  08-replicating-a-real-page.md

**Bedeutung für QET:** Dieselben Schutzregeln gelten für `qet_mcp` und `qet_live_*`: nach jedem Schreiben zurücklesen,
fehlgeschlagene Schritte auf Reste prüfen, Verbindungen getrennt von der Geometrie beweisen, Projektwechsel erkennen.
Für den Import heißt es: Was die EPLAN-API nicht hergibt (Makrobeschreibung), kommt aus den Dateien (`.ema`, `.epdz`).

## 11. Übersicht der Widersprüche und Korrekturen

| Stelle im Skill | Hilfe sagt | API-Beobachtung | Art |
|---|---|---|---|
| schaltplan-editor.md Abschnitt 1 | Ursprung intern unten links, Anzeige RX/RY oben links | `PointD` mm, unten links, +Y oben; Quelle hält es für undokumentiert | Widerspruch zur Quelle, Skill stimmt |
| schaltplan-editor.md Abschnitt 3 | T-Stück 4 Richtungen × 4 Varianten | `TOUR` hat 5 Varianten (v8 alle Beine bei 0,0) | Widerspruch |
| symbole.md Abschnitt 2 | Symbolnummer = Position in der Bibliothek, Start 0 | Nummern dünn besetzt (0–72, dann 308, 338, 402 …) | Korrektur |
| makros-artikel.md Abschnitt 4 | Makrokasten hat Feld „Beschreibung“ | über `MacroBox` nicht lesbar | Widerspruch |
| makros-artikel.md Abschnitt 4 | keine Makrokästen in Schaltplanprojekten einfügbar | `MacroBox` auf Schaltplanseiten vorhanden (beim Einfügen mitgespeichert) | Einordnung |
| querverweise.md (Abbruchstellen) | Abbruchstellen tragen Namen | Werkzeug kann sie nicht benennen | Grenze des Werkzeugs, kein EPLAN-Widerspruch |

## 12. Quellen

Alle unter `https://github.com/Canary-ECAD/eplan-mcp/blob/main/Testing/` (MIT-Lizenz):
00-INDEX.md · 01-schematic-authoring-primitives.md · 02-routing-and-connections.md ·
03-coordinate-system-and-designations.md · 04-interactive-vs-scripted-placement.md · 05-external-symbol-dataset.md ·
06-macrobox-reflection.md · 07-page-to-ascii.md · 08-replicating-a-real-page.md ·
09-symbol-catalog-enumeration-gap.md · 10-image-to-schematic-pipeline.md · 11-symbol-overview-inventory.md ·
12-main-function-and-function-text.md
