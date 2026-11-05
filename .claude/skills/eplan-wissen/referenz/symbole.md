# EPLAN: Symbole, Symbolbibliotheken, Symboleditor, Symbolauswahl

Was ein EPLAN-Symbol ist (Grafik + Anschlüsse + Eigenschaftsplatzierungen, Logik über die Funktionsdefinition), wie
Symbolbibliotheken (`*.slk`) aufgebaut und eingelagert sind, wie Varianten, Anschlusspunkte, Eigenschaftsanordnungen
und Kontaktspiegel im Symbol definiert werden und wie Symbole ausgewählt, platziert und getauscht werden. Zweck:
EPLAN-Symbole/-Makros richtig nach QET importieren und Originalpläne richtig deuten. Ergänzt
[makros-artikel.md](makros-artikel.md) §8–9 (Varianten A–H, Funktionsdefinitionen) und
[schaltplan-editor.md](schaltplan-editor.md) §13 (Drehen/Spiegeln). In eigenen Worten; jede Aussage mit Quelle.

## Inhalt

1. Symbol, Funktion, Schaltzeichen
2. Identifizierende Eigenschaften (Nummer, Name, Anschlussbezeichnung)
3. Symbolbibliotheken: Einlagern, Vorzeichen, Sperre, Tausch, Abgleich
4. Symbolvarianten A–H, Kontaktspiegel- und Verbindungspunktvariante
5. Symboleditor: Symbol anlegen, Einfügepunkt, Raster, Ebenen
6. Anschlusspunkte (Richtung, Nummerierung, gedrehte Bezeichnungen)
7. Eigenschaftsplatzierungen, Eigenschaftsanordnungen, Kontaktspiegel am Symbol
8. Symboleigenschaften, symbolbezogene Anschlusslogik, Transformationspunkt
9. Symbolprüfung, Import/Export, Kopieren, Skalieren, Dokumentation
10. Symbolauswahl und Platzieren
11. Symbol tauschen vs. Schaltzeichen tauschen
12. Symbolmakros und Bibliotheksposition
13. Quellen

---

## 1. Symbol, Funktion, Schaltzeichen

- **Symbol** = genormte Grafik einer Funktion mit Anschlusspunkten, Symbolgruppe, Platzhaltern für BMK,
  Anschlussbezeichnung/-beschreibung, Querverweis und weitere Eigenschaften. Die **Logik** steht in der
  **Funktionsdefinition**, auf die das Symbol verweist. Das Symbol selbst enthält „keine Logik“; ein **Schaltzeichen** ist
  ein platziertes Symbol **mit** zugeordneter Funktion.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_k_aufbaueigenschaften.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symboletauschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_k_arbeitsweise.htm
- Verhältnis Symbol ↔ Funktion: **keine** Funktion (Winkel, T-Stück), **eine** (Schließer, Öffner), **mehrere**
  (Motorschutzschalter, Dreifachsicherung), **Teil** einer Funktion (Geräteanschluss, Wechslerhaken).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_k_start.htm

**Bedeutung für QET:** Ein QET-Element vereint Grafik **und** Logik (Anschlüsse, link_type, Texte). Beim Import
eines EPLAN-Symbols die Funktionsdefinition mitlesen: sie bestimmt Master/Slave/Simple, Kontaktart (Schließer/Öffner/
Wechsler) und welche Anschlüsse intern verbunden sind. Ein Symbol mit **mehreren** Funktionen (3-poliger
Motorschutzschalter) ist in EPLAN **ein** Schaltzeichen mit mehreren Funktionen; in QET ein Element mit allen
Anschlüssen (nicht drei Elemente). Ein Symbol, das nur **Teil** einer Funktion ist (Wechslerhaken), ergibt kein
eigenständiges QET-Element mit Kennzeichen.

## 2. Identifizierende Eigenschaften

- **Symbolnummer**: Position in der Bibliothek, eindeutig je Bibliothek, nur intern; über sie werden Symbole intern
  **referenziert**. Start 0, Bereich 0–32 000, nicht negativ.
- **Symbolname**: sollte mit dem BM-Kennbuchstaben beginnen (z. B. `SL`, `QL3`, `M3`), Suchkriterium.
- **Anschlussbezeichnung** (am Symbol) = fortlaufende **Anschlussnummer** 1, 2, 3 … zur Identifikation des
  Anschlusses; die logischen Daten der Anschlüsse (z. B. welcher beim Wechsler die Wurzel ist) liegen in der
  Funktionsdefinition. **Anschlussbeschreibung** identifiziert nicht, darf mehrfach vorkommen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_k_aufbaueigenschaften.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symboleanlegen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_symboleigenschaften.htm
- Tipp der Hilfe zur Anschlussbelegung: Anschlussbezeichnungen beim Platzieren durch Absatzmarken getrennt
  durchnummerieren; die Anzeige zeigt die Reihenfolge der Funktionsanschlüsse laut Anschlusslogik.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symboleplatzieren.htm

**Bedeutung für QET:** Die Reihenfolge der Symbolanschlüsse (1, 2, 3 …) ist die Reihenfolge, in der EPLAN die
**Anschlussbezeichnungen** des Betriebsmittels zuordnet (mehrzeiliges Feld Anschlussbezeichnung, eine Zeile je
Anschluss). Beim Import müssen QET-Terminals in **dieser** Reihenfolge angelegt bzw. benannt werden, sonst landen
`13/14` an den falschen Pins. Symbolnummer und -name als Herkunft im Element (z. B. Beschreibung) mitführen, damit ein
späterer Abgleich möglich ist (Vermutung/Empfehlung).

## 3. Symbolbibliotheken: Einlagern, Vorzeichen, Sperre, Tausch, Abgleich

- Bibliothek `*.slk` mit beliebig vielen Symbolen; gespeichert wird je **einzelnes** Symbol (Mehrbenutzer). Raster
  ist **Bibliothekseigenschaft** (gilt für alle Symbole). Neue Bibliothek speichert Firmenkennung, Stationsnummer,
  Mitarbeiterkennung. Austauschformate: Bibliothek `*.esl`, einzelnes Symbol `*.esy` (Export mit allen Varianten).
  Reorganisieren entfernt gelöschte Daten endgültig.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolbibliothekenanlegen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolebearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolbibliothekenimportierenexportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symboleimportierenexportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbibreorganisieren.htm
- Verwendung im Projekt: Bibliothek in die Projekteinstellung **Verwaltung > Symbolbibliotheken** eintragen → sie wird
  ins Projekt **eingelagert** (verknüpft mit den Stammdaten). Je Bibliothek ein **Vorzeichen** (Standard: erster
  Buchstabe, z. B. `I:SL` = Schließer-Leistungskontakt der IEC-Bibliothek), doppelt belegbar zum Zusammenfassen, und eine
  **Standardvariante**.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_k_vorgehensweise.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_d_symbolauswahl.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symbolvarianteausw.htm
- Normbibliotheken für: Anlagen-Elektrik, Montageplatten-Aufbau, MSR, **einpolig**, Hydraulik, Pneumatik, Elektronik,
  Prozessdiagramme. **Spezialbibliotheken** (Geräte-, SPS-, Strukturkästen, Abschirmungen, Verbindungssymbole =
  `SPECIAL.slk`) sind gesperrt, werden bei jedem neuen Projekt automatisch eingelagert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_k_arbeitsweise.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolbibliothekenbearbeiten.htm
- **Bibliothek tauschen** (Projekteinstellung): nur, wenn alle verwendeten Symbole **und Varianten** in beiden
  Bibliotheken vorhanden sind; maßgeblich ist die **Symbolnummer**, nicht der Name. **Symbole aktualisieren**
  (Stammdatenabgleich, „nach links kopieren“): überschreibt nur die **Grafik**, die Funktion bleibt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symbolbibltauschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symboleaktualisieren.htm
- **Neue Platzierung verhindern** (Spalte in der Symbolauswahl-Liste): Symbol gesperrt, bis es freigegeben wird;
  bereits platzierte bleiben nutzbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symbolplatzierungverhindern.htm
- Projektsymbole im Überblick: `Projektdaten > Symbole` (verwendete Bibliotheken und Symbole, Drag & Drop, bibliotheks-
  oder funktionsorientierter Baum).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_d_pddsymbolauswahl.htm

**Bedeutung für QET:** Ein EPLAN-Projekt trägt seine Symbolbibliotheken **eingelagert** in sich; Symbolverweise sind
`Bibliothek + Symbolnummer + Variante`. Beim Import aus einem EPLAN-Projekt/Makro deshalb die **im Projekt
eingelagerte** Bibliothek als Quelle nehmen, nicht eine gleichnamige Systembibliothek (kann andere Grafik haben).
Gleicher Symbolname in zwei Bibliotheken ≠ gleiches Symbol. QET-Sammlung analog organisieren: je EPLAN-Bibliothek ein
Ordner, Elementname = Symbolname (Haus-Regel Ordner Hersteller/Kategorie/Typ gilt für Herstellerbauteile).

## 4. Symbolvarianten A–H, Kontaktspiegel- und Verbindungspunktvariante

- Bis zu **acht Varianten A–H**, dazu eine **Kontaktspiegelvariante** und eine Variante für die **Darstellung als
  Verbindungspunkt**. Alle Varianten teilen die Symboleigenschaften, bilden dieselbe Funktion ab, haben **dieselbe
  Anschlusszahl** und unterscheiden sich nur in der grafischen Anordnung der Anschlüsse.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_mitsymbolvariantenarbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symbolvarianteausw.htm
- Neue Variante aus vorhandener: Drehwinkel, optional Grafik an der Y-Achse spiegeln, Anschlussbezeichnungen drehen,
  platzierte Eigenschaften drehen. Seitenname im Editor `<Bibliothek>;<Symbolname>:<Nummer>;<Variante>`
  (z. B. `neu.test:1;A`). Beim Blättern bleibt die gewählte Variante, fehlt sie: „Variante existiert nicht“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_mitsymbolvariantenarbeiten.htm
- Belegung beim Drehen/Spiegeln im Schaltplan: A→B→C→D = 0°/90°/180°/270°, E–H = gespiegelte Varianten
  (schaltplan-editor.md §13). Beim Platzieren: `[Strg]`+Maus oder `[Tab]` blättert durch die Varianten; Startvariante
  = Standardvariante der Bibliothek.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_drehenspiegeln.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symbolvarianteausw.htm

**Bedeutung für QET:** Import nur **Variante A** als Element; B–D über QET-Drehung, E–H über Spiegelung. Ausnahme: wenn
eine Variante andere Grafik hat als die gedrehte A (z. B. lesbare Texte, verschobene Anschlüsse) – dann ist die
gedrehte QET-Variante optisch falsch, logisch aber richtig; nach Haus-Regel „Inhalt vor Optik“ zulässig. Die
**Kontaktspiegelvariante** ist die kleine Grafik im Kontaktspiegel – in QET liefert der Kontaktspiegel des Masters
(XRef) eigene Grafik, also nicht als Element importieren. Die **Verbindungspunktvariante** gibt es nur bei
Verbindungssymbolen (T-Stück als Punkt).

## 5. Symboleditor: Symbol anlegen, Einfügepunkt, Raster, Ebenen

- Symboleditor nur mit **geöffnetem Projekt** (Eigenschaften kommen aus Projekteinstellungen); Bearbeitung über
  Rechteverwaltung geregelt. Jedes geöffnete Symbol in eigenem Editorfenster, Daten über Zwischenablage austauschbar;
  geöffnet wird die erste vorhandene Variante.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_k_arbeitsweise.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolebearbeiten.htm
- Anlegen: Bibliothek wählen → Variante wählen → Symboleigenschaften (Nummer = nächste freie, Name, **Funktions-
  definition** über Auswahldialog). Der **Einfügepunkt** kann verschoben, aber nicht gelöscht werden: an ihm hängt das
  Symbol beim Platzieren, er bestimmt die Position und damit den **Querverweis** (Spalte/Zeile).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symboleanlegen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolebearbeiten.htm
- Im Symboleditor: keine referenzierenden Symbole (eingefügte Symbole werden zu Grafik aufgelöst), **nur Standard-
  Ebenen** (Elemente auf freien Ebenen werden angepasst; beim Prüfen nach **EPLAN300** konvertiert). Linien- und
  Textvorgaben kommen aus den Ebenen Symbolgrafik bzw. Eigenschaftsplatzierung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolepruefen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_schaltzeicheningrafik.htm
- Bibliothekseigenschaften: Ersteller, Anzahl Symbole, Erstellungsdatum (fest), Beschreibung, Version (änderbar).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_symbolbibliothekseigenschaften.htm

**Bedeutung für QET:** EPLAN-Einfügepunkt ↔ QET-Hotspot des Elements. Beim Import den Hotspot auf den EPLAN-Einfügepunkt
legen, dann stimmt (a) die Lage beim Nachbau aus PDF-Koordinaten und (b) die Spalte/Zeile des Querverweises (EPLAN
rechnet den Verweis vom Einfügepunkt). Symbolgrafik auf mehreren EPLAN-Ebenen (Farben) → QET-Formen mit
entsprechenden Farben/Strichen.

## 6. Anschlusspunkte

- Einfügen > Anschluss „Richtung“ (oben/unten/links/rechts; `[Strg]`+Maus oder `[Tab]` dreht gegen den Uhrzeigersinn);
  Anschlussnummer = nächste freie; je Anschluss Eigenschaftssätze **Anschlussbezeichnung/-beschreibung ungedreht** und
  **gedreht** (für die neue DIN, umschaltbar über Ansicht bzw. Projekteinstellung „Anschlussbezeichnung gedreht
  darstellen“). Die Anschlussgrafik zeigt nur die Richtung und gehört nicht zum späteren Symbol.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_anschluessebearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_anschlusspunkt.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenprojektallgemein.htm
- **Anschlusspunkte nummerieren**: Reihenfolge „von oben nach unten, dann links nach rechts“ oder „links nach rechts,
  dann oben nach unten“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_anschlusspunktenummerieren.htm
- Regeln: **keine übereinanderliegenden Anschlüsse** eines Symbols (Editor korrigiert automatisch); beim Platzieren dürfen
  Einfügepunkte und gleichgerichtete Anschlüsse nicht übereinander liegen (Ausnahme: entgegengesetzte Anschlüsse an
  gleicher Stelle, z. B. T-Verbinder). Doppelte Nummern und Lücken korrigiert die Prüfung; abweichende Anschlusszahl
  zwischen Varianten oder gegenüber der Funktionsdefinition → Meldung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_anschluessebearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symboleplatzieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolepruefen.htm
- Ansicht „Eigenschaften als Zahlen“ zeigt Platzhalter als Nummern – wirkt auch auf Druck/PDF.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_anschlusspunkt.htm

**Bedeutung für QET:** Anschlussrichtung ↔ QET-Terminal-Orientierung (n/s/e/w); der Anschlusspunkt liegt am Ende der
kurzen Richtungsgrafik → QET-Terminal genau dort, sonst passt das Raster nicht. Gedrehte Anschlussbezeichnungen (DIN):
im Original prüfen, ob die Pin-Nummern quer stehen; in QET Elementtext-Drehung entsprechend. Prüfung beim Import:
Anschlusszahl Symbol = Funktionsdefinition, sonst Warnung.

## 7. Eigenschaftsplatzierungen, Eigenschaftsanordnungen, Kontaktspiegel am Symbol

- `Bearbeiten > Platzierte Eigenschaften`: Registerkarte **Schaltzeichen** legt fest, welche Eigenschaften (BMK,
  Funktionstext, Anschlussbezeichnung/-beschreibung …) mit dem Symbol platziert werden, mit Format, Textrahmen,
  Positionsrahmen, Wert/Einheit, Position. Registerkarte **Kontaktspiegel**: Ausrichtung **Im Schaltplanpfad** (je nach
  Normblatt unten oder rechts im Pfad) oder **Am Schaltzeichen**; im Symboleditor nur Position einstellbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_eigenschaftsplatzierung.htm
- **Eigenschaftsanordnungen** = benannte Anordnungssätze am Symbol, je Variante eine **Standard-Eigenschaftsanordnung
  für Variante x**; eigene anlegen/löschen unter `Bearbeiten > Eigenschaftsanordnungen`. Im Projekt: manuell
  verschobene Texte → „Benutzerdefiniert“; Projekteinstellung regelt Konflikte gleichnamiger Anordnungen beim Kopieren
  (schaltplan-editor.md §20).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_eigenschaftsanordnung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_eigenschaftsanordnungenbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenprojektallgemein.htm
- **Verbindungssymbole** (Winkel, T-Stück, Kreuzung) dürfen **keine** Eigenschaftsplatzierungen haben; sie werden beim
  Speichern entfernt (auch die automatische Anschlussbezeichnung der Anschlusspunkte).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_eigenschaftsplatzierung.htm
- Am Symbol lassen sich auch Blockeigenschafts-Formate vordefinieren (Blockeigenschaft: Format [n]).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blockproperties_k_vordefinition.htm

**Bedeutung für QET:** Eigenschaftsplatzierungen ↔ dynamische Elementtexte (label, function, Anschlussnummern …) mit
Position relativ zum Hotspot. Beim Import die **Standard-Eigenschaftsanordnung der Variante A** übernehmen. Ein Symbol
mit Kontaktspiegel-Registerkarte ist ein Master (Spule, Motorschutzschalter) → QET-Master mit XRef-Lage: „Im
Schaltplanpfad“ ≈ `snapto` Bottom, „Am Schaltzeichen“ ≈ Label (querverweise.md §6). Verbindungssymbole ohne Texte
importieren (QET-Knoten ohne Beschriftung).

## 8. Symboleigenschaften, symbolbezogene Anschlusslogik, Transformationspunkt

- Symboleigenschaften gelten für **alle** Varianten. Bei Funktionsdefinition „… variabel“ ist die **Anschlusszahl**
  einstellbar.
- **[Logik]** zeigt die Anschlussdaten der Funktionsdefinition und erlaubt **symbolbezogene** Anpassung; beim Einfügen
  gelten diese, wenn sie von der Funktionsdefinition abweichen. Neue Funktionsdefinition ersetzt die Symbol-Anschluss-
  daten. Zweck: Sonderlogik für bestimmte Symbole.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_symboleigenschaften.htm
- **Transformationspunkt (ID 16045)** je Variante (Index) als Koordinate relativ zum Einfügepunkt, z. B. `2.0/2.0`;
  Bezugspunkt für Drehen/Spiegeln statt des ersten Anschlusses, gegen inkonsistent aufgebaute Varianten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_drehenspiegeln.htm
- Darstellungsart **Funktional** auch im Symboleditor wählbar → eigene Spezialsymbole ohne Grafik für Ablaufdiagramme.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functionaldiagramgui_k_prinzip.htm

**Bedeutung für QET:** Beim Import zuerst die **symbolbezogene** Anschlusslogik lesen (falls vorhanden), sonst die der
Funktionsdefinition – sie sagt, welche Anschlüsse intern durchverbunden sind (Klemme oben/unten, Potential weiterleiten)
und welcher intern/extern ist. QET-Elemente kennen interne Verbindungen nur bei bestimmten Typen (Klemme, „bridge“) –
Vermutung: im Fork über Terminal-Eigenschaften abbilden.

## 9. Symbolprüfung, Import/Export, Kopieren, Skalieren, Dokumentation

- Prüfung (`Dienstprogramme > Symbol prüfen`, teils automatisch): doppelte Anschlussnummern und übereinanderliegende
  Anschlüsse korrigiert, Lücken entfernt, referenzierende Symbole aufgelöst, freie Ebenen → EPLAN300; abweichende
  Anschlusszahl (Varianten/Funktionsdefinition) gemeldet.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolepruefen.htm
- Import `*.esy`: Nummer = nächste freie, Name = Original oder „(Kopie)“, Funktionsdefinition vom Original. Kopieren
  zwischen Bibliotheken optional **mit Varianten** (sonst nur die erste vorhandene).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symboleimportierenexportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolekopieren.htm
- **Bibliothek kopieren und skalieren**: Quellrastermaß (fest) → Zielrastermaß in mm; freie Texte/Eigenschafts-
  platzierungen 1:1 oder mit ganzzahligem Faktor, optional auf Standardgrößen (mm/inch) runden. Einzelne Symbole per
  Bereich + Faktor skalieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_symbolbiblskalieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symboleskalieren.htm
- Dokumentation: Auswertung „Symbolübersicht“ (Formular in Projekteinstellung; Formulareigenschaft **Nur verwendete
  Symbole, ID 13114**), auch in separates Projekt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolbibliothekendokumentieren.htm
- Übrige Verwaltungsschritte (Öffnen, Schließen, Löschen, Kopieren der Bibliothek): nur überflogen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolbibliothekenkopieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symbolbibliothekenschliessen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_symboleloeschen.htm

**Bedeutung für QET:** Das Rastermaß der EPLAN-Bibliothek (meist 4 mm Elektro, 2 mm Fluid) ist der Umrechnungsfaktor
beim Import: 1 EPLAN-Raster ↔ 10 px QET (Vermutung zur Fluidbibliothek: dort 2 mm ↔ 5 px oder hochskalieren – je
Bibliothek festlegen). Eine „Symbolübersicht“ im Original-PDF ist eine Auswertung, kein Schaltplan.

## 10. Symbolauswahl und Platzieren

- Dialog Symbolauswahl: Filter, Baum **bibliotheksorientiert** (Bibliothek > Funktionsdefinitionen) oder
  **funktionsorientiert** (Kategorie wie „Spulen und Kontakte“, „Meldeeinrichtungen“), Liste mit konfigurierbaren
  Spalten, **Direkteingabe** (sucht in der ersten Listenspalte), Vorschau aller Varianten (bei mehreren Symbolen
  Variantenwahl, „Standardvariante“). Im Grafischen Editor nur ein Symbol wählbar, im Symboleditor mehrere.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_d_symbolauswahl.htm
- Aufruf aus Registerkarte Symbol-/Funktionsdaten zusätzlich: **Nur zur Darstellungsart passende Symbole**, **Nur
  Symbole gleicher Funktionskategorie**, **Nur Symbol austauschen** (nur Symbole mit nicht mehr Anschlüssen als die
  Funktionsdefinition; Funktion bleibt). Rücktaste nach „Einfügen > Kasten“ öffnet vorgefilterte Auswahl.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_d_symbolauswahl.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_h_polygonalekaesten.htm
- Platzieren: Vorschau der Autoconnect-Linien; Hilfslinie vom ersten platzierten Symbol zur Cursorposition; mehrfach
  platzieren durch Ziehen mit gedrückter Maustaste über mehrere mögliche Verbindungen (ohne Eigenschaftsdialog, Daten aus
  Vorgaben und Nummerierungsmodus). Danach Eigenschaften des Schaltzeichens (BMK, Funktionstext …).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symboleplatzieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_k_vorgehensweise.htm
- Einpolige Symbole: Filter „IEC-Symbole einpolig“; Symbole mit gleicher Nummer und Funktionsdefinition in ein- und
  allpoliger Bibliothek sind tauschbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_h_stromlaufplaenezeichnen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_hintergrund.htm

**Bedeutung für QET:** Funktionsorientierte Kategorien der EPLAN-Symbolauswahl eignen sich als Ordnerstruktur für die
importierten IEC-Symbole in der QET-Benutzersammlung (Spulen und Kontakte, Meldeeinrichtungen …). Haus-Regel
Symbolquellen-Reihenfolge bleibt: Herstellermakro > QET-IEC-Symbol + Artikeldaten > PDF.

## 11. Symbol tauschen vs. Schaltzeichen tauschen

- **Symbol tauschen**: Logik/Funktion bleibt, nur die Grafik ändert sich; erlaubt nur Symbole mit **gleicher oder
  geringerer** Anschlusszahl (bei „variabel“ alle); Funktionsdefinition bleibt.
- **Schaltzeichen tauschen**: auch die Funktion wird ersetzt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_h_symboletauschen.htm
- Gerät einfügen mit Symbolauswahl: gefiltert nach Darstellungsart + „Undefiniert“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_d_geraeteinfuegen.htm

**Bedeutung für QET:** Weil EPLAN Symbole tauschen kann, ohne die Funktion zu ändern, kann ein Original eine
untypische Grafik für eine Standardfunktion zeigen (z. B. Herstellergrafik für einen Schließer). Für die Logik zählt die
Funktion (Querverweis, Kontaktspiegel), nicht die Grafik – im Nachbau Elementtyp nach Funktion wählen.

## 12. Symbolmakros und Bibliotheksposition

- Symbolmakro `*.ems` (`Bearbeiten > Fenster-/Symbolmakro erstellen`) speichert: Liste der Symbolbibliotheken,
  **Position (Zeilennummer)** der Bibliothek in den Projekteinstellungen des Quellprojekts, **Symbolnummer**,
  **Variantennummer**. Beim Einfügen wird die Position auf die Liste des Zielprojekts umgerechnet; fehlt die Bibliothek,
  wird die Bibliothek an der gespeicherten Position genommen; fehlt dort Bibliothek, Symbol oder Variante → **rotes
  „X“** + Meldung (mit Name/Position der Ursprungsbibliothek).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsbgui_k_symbolmakros.htm

**Bedeutung für QET:** Für den Makro-Import (`eplanmacroreader`, makros-artikel.md) heißt das: Symbolverweise in
`.ema/.ems` sind **Bibliotheksindex + Symbolnummer + Variante**, nicht Namen. Der Importer braucht die
Bibliotheksliste des Quellprojekts (bzw. aus dem Makro) und die zugehörigen `.slk`, um die Grafik aufzulösen; sonst
entsteht wie in EPLAN ein Platzhalter („rotes X“) – dann melden, nicht still eine Ersatzgrafik wählen (Haus-Regel „Keine
Abkürzung“).

## 13. Quellen

Alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ :

- Symbole/Symbolauswahl: xsbgui_k_start.htm, xsbgui_k_arbeitsweise.htm, xsbgui_k_aufbaueigenschaften.htm,
  xsbgui_k_vorgehensweise.htm, xsbgui_k_symbolmakros.htm, xsbgui_d_symbolauswahl.htm, xsbgui_d_pddsymbolauswahl.htm,
  xsbgui_h_symboleplatzieren.htm, xsbgui_h_symboletauschen.htm, xsbgui_h_symbolvarianteausw.htm,
  xsbgui_h_symbolbibltauschen.htm, xsbgui_h_symboleaktualisieren.htm, xsbgui_h_symbolplatzierungverhindern.htm.
- Symboleditor: symboleditorgui_k_start.htm, symboleditorgui_k_arbeitsweise.htm, symboleditorgui_d_anschlusspunkt.htm,
  symboleditorgui_d_anschlusspunktenummerieren.htm, symboleditorgui_d_eigenschaftsanordnung.htm,
  symboleditorgui_d_eigenschaftsanordnungenbearbeiten.htm, symboleditorgui_d_eigenschaftsplatzierung.htm,
  symboleditorgui_d_symbolbibliothekseigenschaften.htm, symboleditorgui_d_symbolbiblskalieren.htm,
  symboleditorgui_d_symboleigenschaften.htm, symboleditorgui_h_anschluessebearbeiten.htm,
  symboleditorgui_h_mitsymbolvariantenarbeiten.htm, symboleditorgui_h_symbibreorganisieren.htm,
  symboleditorgui_h_symbolbibliothekenanlegen.htm, symboleditorgui_h_symbolbibliothekenbearbeiten.htm,
  symboleditorgui_h_symbolbibliothekendokumentieren.htm, symboleditorgui_h_symbolbibliothekenimportierenexportieren.htm,
  symboleditorgui_h_symboleanlegen.htm, symboleditorgui_h_symbolebearbeiten.htm,
  symboleditorgui_h_symboleimportierenexportieren.htm, symboleditorgui_h_symbolekopieren.htm,
  symboleditorgui_h_symbolepruefen.htm, symboleditorgui_h_symboleskalieren.htm;
  überflogen: symboleditorgui_h_symbolbibliothekenkopieren.htm, symboleditorgui_h_symbolbibliothekenschliessen.htm,
  symboleditorgui_h_symboleloeschen.htm.
- Weitere: gededitgui_k_drehenspiegeln.htm, gededitgui_h_schaltzeicheningrafik.htm,
  gedviewer_d_einstellungenprojektallgemein.htm, blockproperties_k_vordefinition.htm, blackbox_h_polygonalekaesten.htm,
  functionaldiagramgui_k_prinzip.htm, singlepole_h_stromlaufplaenezeichnen.htm, singlepole_k_hintergrund.htm,
  egedgui_d_geraeteinfuegen.htm.
