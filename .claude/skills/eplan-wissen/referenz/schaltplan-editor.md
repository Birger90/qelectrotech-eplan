# EPLAN: Grafischer Editor und Schaltplanseiten

Wie EPLAN (Plattform 2.9, abgeglichen mit 2026) eine Schaltplanseite logisch auswertet: Autoconnecting,
Verbindungssymbole (Winkel, T-Stück, Kreuzung, Brücke) mit Zielfestlegung, Gerätekästen (Blackbox),
Strukturkästen, einpolige Darstellung mit Strängen, Pfad-Funktionstexte, Ebenen, Bemaßung, Redlining und
Ablaufdiagramme. Reine Bedienung (Zoom, Tasten) nur knapp. In eigenen Worten; jede Aussage mit Quelle.
„Vermutung“ = nicht aus der Hilfe belegt. Verbindungen/Potentiale/Navigatoren stehen in
[verbindungen-potentiale.md](verbindungen-potentiale.md), Symbole und Symbolbibliotheken in [symbole.md](symbole.md).

## Inhalt

1. Grundprinzip: Logikseite, Einfügepunkt, Raster, Koordinaten
2. Autoconnecting und Unterbrechung
3. Verbindungssymbole: Winkel, T-Stück, Kreuzung, Brücke, Diagonale
4. Zielverfolgung und Zielfestlegung (Reihenfolge der Ziele)
5. Darstellung als Punkt oder mit Zielfestlegung
6. Logische Verbindungssymbole: Leitungsverteiler, -verbinder, Sammelschienen
7. Gerätekästen (Blackbox), Geräteanschlüsse, Schachteln
8. Strukturkästen grafisch
9. Kästen als Polylinie (DC2, SC2, PLCC3 …)
10. Pfad-Funktionstexte und Funktionstext (automatisch)
11. Texte, Eigenschaftstexte, Andocken, direkte Bearbeitung
12. Blockeigenschaften (zusammengesetzte Anzeigetexte)
13. Drehen, Spiegeln, Symbolvarianten
14. Gruppen, Blöcke, Schaltzeichen in Grafik umwandeln
15. Einpolige Darstellung, Stränge, Stranganschlüsse
16. Ebenen (Ebenenverwaltung)
17. Bemaßung
18. Redlining (PDF-Kommentare)
19. Ablaufdiagramme: Funktionsdiagramm und GRAFCET
20. Blockbearbeitung, Bearbeitungsbereiche, Sonstiges (Bildexport, Hyperlinks, Undo)
21. Quellen

---

## 1. Grundprinzip: Logikseite, Einfügepunkt, Raster, Koordinaten

- Der Grafische Editor zeichnet Schaltpläne und mechanische Zeichnungen. Die Statusleiste zeigt, ob die Seite
  als **Logik** oder **Grafik** ausgewertet wird, den Maßstab und den Verbindungsstatus: `#` = Projekt hat
  nicht aktuelle Verbindungen, zusätzlich `*` = die offene Seite hat nicht aktuelle Verbindungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_prinzip.htm
- Sichtbarer Bereich (Bildschirm, Druck, PDF): mit Normblatt nur die Grafik **innerhalb** des Normblatts; ohne
  Normblatt alle Elemente.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_prinzip.htm
- Logische Objekte (Symbole, Kästen, Texte) haben einen **Einfügepunkt** (gefülltes Quadrat), grafische Elemente
  **Elementpunkte** (ungefüllt). Ob ein Schaltzeichen in einem Kasten „liegt“, entscheidet immer der Einfügepunkt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_objektfang.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_k_grundlagen.htm
- Koordinatensysteme je Seitentyp: Grafik (Ursprung unten links, mm, X/Y), **Elektrotechnik** (Ursprung links
  oben, in Rasterschritten, RX/RY, Standard-Raster 4 mm), Fluid/Verfahrenstechnik (unten links, Rasterschritte).
  Intern liegt der Ursprung immer unten links; die „oberste Schaltplanposition“ legt das Normblatt fest. Wird
  das Blatt vergrößert, wächst es nach oben/rechts.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_koordinatensysteme.htm
- Die Rastergröße ist **Seiteneigenschaft**; Rasterfang legt Einfüge- und Elementpunkte auf Rasterpunkte.
  „Am Raster ausrichten“ verschiebt nachträglich falsch liegende Objekte; ohne Rasterfang gezeichnete Objekte
  sind mit eingeschaltetem Fang evtl. nicht mehr greifbar. Ausrichten/gleichmäßig Verteilen ignorieren das
  Raster.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_raster.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_horverausrichten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_gleichmaessigverteilen.htm
- Seitenmaßstab (nur Grafik-/Montageplattenseiten wirksam): Grafik, Texte und Symbolbeschriftung bleiben in
  ihren Werten gleich; nur Bemaßungstext und Maßlinienbegrenzung skalieren, die Maßzahl bleibt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_seitenmassstab.htm

**Bedeutung für QET:** EPLAN-Raster 4 mm ↔ QET-Raster 10 px; Positionen aus dem PDF immer auf das Raster
runden, sonst liegen Anschlüsse nicht exakt gegenüber (QET-Leiter werden schräg/abgeknickt). Elemente außerhalb
des Normblatts gehören in EPLAN nicht zum Druckbild – beim Nachbau ignorieren.

## 2. Autoconnecting und Unterbrechung

- Stehen sich Anschlüsse zweier Symbole **waagerecht oder senkrecht genau gegenüber**, zeichnet EPLAN
  automatisch eine Verbindungslinie (Autoconnecting). **Nur diese Autoconnect-Linien gelten als elektrische
  Verbindung** und werden ausgewertet; aktiv nur auf Schaltplanseiten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_k_start.htm
- Ungewollte Autoconnect-Linien (z. B. zu Hilfskontakten eines Motorschutzschalters) trennt man mit dem
  Verbindungssymbol **Unterbrechung**: unsichtbares Symbol ohne Anschlüsse, verhindert/trennt die Linie.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_verbindungenunterbrechen.htm
- Logikfang (`Q`): beim Platzieren werden Anschlusspunkte bevorzugt auf bestehende/mögliche Verbindungslinien
  gezogen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_logikfang.htm
- Hintergrund-Ebenen liegen sogar hinter den Autoconnect-Linien.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_hintergrundbearbeiten.htm

**Bedeutung für QET:** QET hat keine Autoconnect-Logik im Sinne „gegenüberliegend = verbunden“; jede Linie im
PDF muss ein **echter Leiter** zwischen zwei Anschlüssen werden. Umgekehrt: Wo im EPLAN-PDF zwei Anschlüsse
fluchten, aber keine Linie ist, steckt eine Unterbrechung – dort keinen Leiter ziehen (typisch: Paarkontakt
neben Motorschutzschalter, siehe querverweise.md §5). Nicht verbundene, aber fluchtende Anschlüsse sind kein
Zeichenfehler.

## 3. Verbindungssymbole: Winkel, T-Stück, Kreuzung, Brücke, Diagonale

- Rein grafische Verbindungssymbole: **Winkel, T-Stück, Kreuzung, Brücke, Abbruchstelle**. Sie liegen in
  `SPECIAL.slk`, haben **keine Funktionsdefinition**, keine BMK-, Funktions- oder Artikeldaten. Ihre
  „Zielverfolgung“ (welche Anschlüsse intern verbunden sind) ist Symboleigenschaft.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_verbindungsstueckeeinfuegen.htm
- Varianten: T-Stück in 4 Richtungen × 4 Varianten; Kreuzung 2 Grundtypen (Suchrichtung senkrecht/waagerecht)
  × 2 Varianten; Brücke 4 Varianten; Winkel über `[Strg]`+Mausdrehung. Tasten: F3 Winkel unten-rechts,
  F4 unten-links, F6 oben-links, F7–F10 T-Stück unten/oben/rechts/links, Umschalt+F8 Brücke, Umschalt+F4
  Abbruchstelle.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_k_darstellungverbindungspunkte.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_tastaturbefehle.htm
- **Austauschen durch Überzeichnen**: neues Verbindungssymbol auf den Einfügepunkt eines vorhandenen setzen
  (z. B. Winkel → T-Stück). Eine Abbruchstelle darf einen Winkel/ein T-Stück überzeichnen, umgekehrt nicht.
  Doppelklick auf T-Stück/Kreuzung/Brücke öffnet die Variantenwahl (nicht bei Winkeln).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_verbindungsstueckeaustauschen.htm
- **Diagonale**: dynamisches Symbol (ohne feste Grafik) für schräge Verbindungen, z. B. Phasendrehung. Weitere
  dynamische Symbole in SPECIAL: Kabeldefinition, Abschirmung, Verbindungs- und Potentialdefinitionspunkt,
  Potentialanschluss.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_dynverbindungssymboleeinfuegen.htm

**Bedeutung für QET:** Ein EPLAN-Winkel ist kein Bauteil – in QET einfach Leiterknick (oder die unsichtbare
„Ecke (Plan)“ aus dem Haus-Skill, wenn ein Knoten nötig ist). Phasendrehung per Diagonale: in QET Leiter mit
schrägem Segment ist nicht möglich (QET-Leiter sind orthogonal) → zwei Leiter über Hilfsknoten oder Grafiklinie +
logischer Leiter (Vermutung: Grafiklinie über Leiter legen, Logik bleibt richtig).

## 4. Zielverfolgung und Zielfestlegung (Reihenfolge der Ziele)

- Zielverfolgung startet an einem Betriebsmittel und sucht entlang der Autoconnect-Linien alle anderen
  **Betriebsmittel** (nur Symbole, die Betriebsmittel darstellen, zählen als Ziel). Feste Regeln:
  1 Linie in Suchrichtung → 1 Ziel (gerade oder über Eck); Verzweigung → 2 Ziele; bei „gerade + über Eck“ zuerst
  das gerade, dann das über Eck; bei Verzweigung in zwei Richtungen zuerst der rechtwinklige, dann der
  **abgeschrägte** Zweig.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_k_grundlagen.htm
- **T-Stück**: drei Anschlusspunkte; der unbezeichnete Punkt zeigt, woher die Verbindung kommt, „1“ und „2“
  die Reihenfolge der Ziele. Im Symbol steht die gerade Linie für das 1. Ziel, die **schräge Linie** für das
  2. Ziel. Beispiel „T-Stück rechts, Variante 3“: von oben (1) oder unten (2) kommend ist das Ziel rechts; von
  rechts kommend ist oben das 1., unten das 2. Ziel.
- **Kreuzung** = zwei fest verbundene T-Stücke; gegenüberliegende Enden direkt verbunden (gerade = 1. Ziel), die
  anderen über die Schräge zum 2. Ziel.
- **Brücke** (Potentialverteilung auf Nachbarklemmen, Steg/Querverbinder): vier Punkte, gemeinsamer Punkt ohne
  Nummer; 1. Ziel immer gegenüber, 2. Ziel rechtwinklig links/oben, 3. Ziel rechtwinklig rechts/unten. Von
  einer Brücke aus: ein Ende mit drei Zielen, drei Enden mit je einem Ziel (Kreuzung: 2×2 + 2×1).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_k_darstellungverbindungspunkte.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_d_bruecke.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_d_tstueck.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_d_kreuzung.htm
- Die Zielreihenfolge bestimmt die **Verdrahtungsreihenfolge** der Einzelverbindungen (welcher Draht von
  welchem Anschluss zu welchem geht) und damit Verbindungslisten, Klemmenpläne und Verbindungsnummerierung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_k_darstellungverbindungspunkte.htm
  (Vermutung zur Auswirkung auf Klemmenpläne: aus „Wege für die Zielverfolgung bei den Auswertungen eindeutig
  vorgegeben“ abgeleitet.)

**Bedeutung für QET:** Das EPLAN-T-Stück mit 45°-Schräge ist **Verdrahtungsinformation**: die Schräge zeigt, in
welche Richtung der zweite Draht läuft. QET-Leiter sind Punkt-zu-Punkt-Drähte; die Zielreihenfolge muss daher
beim Nachbau direkt in die Leiterführung übersetzt werden: vom Herkunftsanschluss zuerst zum 1. Ziel (gerade
Linie), von dort bzw. vom Knoten zum 2. Ziel (Schräge). Die Haus-Abzweigsymbole `Abzweig_Plan`
(unten_links/unten_rechts/oben_rechts/rechts_unten) bilden genau diese Varianten ab – im Original Richtung der
Schräge ablesen. Für Klemmenpläne/Kabellisten (Ziel 1/Ziel 2 an der Klemme) entscheidet diese Reihenfolge, was
links und was rechts steht.

## 5. Darstellung als Punkt oder mit Zielfestlegung

- Benutzereinstellung „Verbindungsabzweigungen **zeichnen**“: Mit Zielfestlegung / Als Punkt / Mit
  Zielfestlegung oder als Punkt (immer Abfragedialog, letzte Wahl wird Vorbelegung). Optional
  „Verbindungssymbole mit Abfragedialog“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenverbindungssymbole.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_knotenalspunktoderzieldarstellen.htm
- Projekteinstellung „Verbindungsabzweigungen **darstellen**“ (Grafische Bearbeitung > Allgemein): **Mit
  Zielfestlegung** (Symbole mit Schräge), **Als Punkt** (alle Abzweige als Punkt), **Wie gezeichnet** (je Symbol).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenprojektallgemein.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_einstellungenanzeigeverdrahtung.htm
- Kontrollkästchen **Ziele festlegen** am T-Stück/Kreuzung/Brücke erscheint normalerweise nur mit Lizenz
  „netzbasierte Verbindungen“. Ist es aus, wertet EPLAN die Zielreihenfolge **nicht** aus, zeichnet einen Punkt
  und erzeugt die Verbindungen nicht automatisch – sie müssen manuell festgelegt werden (netzbasiert, siehe
  verbindungen-potentiale.md).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_d_bruecke.htm
- „Als Punkt zeichnen“ ist nur Darstellung; mit gesetzter Zielfestlegung bleibt die Reihenfolge logisch erhalten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_d_tstueck.htm

**Bedeutung für QET:** Ein Punkt im EPLAN-PDF heißt also **nicht** „ohne Zielreihenfolge“ – nur wenn das Projekt
„Als Punkt“ darstellt, sieht man die Reihenfolge nicht mehr. Dann die Reihenfolge aus Klemmenplan/Verbindungsliste
des Originals rekonstruieren (Vermutung) oder grafisch einfachste Führung wählen. Haus-Regel bleibt: Punkt nur,
wo das Original einen Punkt zeigt; Schräge wie im Original.

## 6. Logische Verbindungssymbole: Leitungsverteiler, -verbinder, Sammelschienen

- Leitungsverteiler (T-Stück/Kreuzung) und Leitungsverbinder (Winkel/Gerade) sind **Funktionen** mit BMK und
  Artikel, erscheinen in Verbindungs- und Stücklisten. Mehrere in einem Betriebsmittel: eine Hauptfunktion mit
  Artikel, unterschiedliche Anschlussbezeichnungen nötig. Elektrisch: Abzweig in eine durchlaufende Leitung
  (gekrallt), Kabelabzweig, Quetschverbinder.
- Standard: Verbindungen **enden** am Leitungsverteiler. Eigenschaft **Durchgehende Verbindung zwischen den
  Anschlüssen (ID 20331)** macht daraus eine durchgehende Verbindung (Kabelbaum); Listen und Nummerierung sehen
  dann die durchgehende Verbindung, Potentialweiterleitung die Teilverbindungen. SPS-Zielverfolgung geht nicht
  über solche Verteiler hinweg (symbolische Adresse dort manuell).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_k_verteiler.htm
- **Sammelschienen** = einzelne Sammelschienenanschlüsse mit gemeinsamem BMK (eine Haupt-, 0…n Nebenfunktionen),
  eigene Strukturregel (meist ohne Anlage/Ort), Anschlussbezeichnung optional (fehlend = kein Fehler).
  Potential und Signal laufen vom 1. Anschluss der Hauptfunktion zu allen Nebenfunktionen; „Signaltrennend“ an der
  Hauptfunktion stoppt alles, an einer Nebenfunktion nur diese. Verbindungsgruppe „An Sammelschienen angeschlossene
  Verbindungen“ für die Nummerierung. Symbol in SPECIAL, 4 Varianten, Textpositionen wie Geräteanschluss.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_k_sammelschienen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_sammelschieneneinfuegen.htm

**Bedeutung für QET:** Sammelschiene im Original (dicke Linie mit Abgriffen): Abgriffe sind Anschlüsse eines
Betriebsmittels, keine bloßen Knoten – Haus-Regel „Abgriffe mit `bod`-Punkten“ ist für die Optik richtig; für
Stücklisten zusätzlich ein Element mit Kennzeichen (Hauptfunktion) setzen, wenn das Original ein Sammelschienen-
BMK zeigt. Leitungsverteiler mit BMK im Original = eigenes QET-Element mit label und Artikel, nicht `bod`.

## 7. Gerätekästen (Blackbox), Geräteanschlüsse, Schachteln

- Einsatz: Geräte ohne passendes Symbol, unvollständige Symbole (fehlender PE), SPS-Baugruppen, komplexe Geräte
  über mehrere Seiten (FU) mit Querverweis, Motor mit Bremse unter einem BMK, **Reservekabelverbindungen** (ohne
  Kasten Fehler „Kabelverbindung ohne Kabel“), Schachtelung (Gerät -A1 mit Leisten -A1-X1, -A1-X2),
  BMK-Vergabe an Klemmen bei Platzmangel, Sonderschütze mit Kontaktspiegel.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_k_start.htm
- Drei Einsatzformen: Kasten + Geräteanschlüsse („Black Box“, Innenleben unbekannt), Kasten + Symbole ohne BMK
  (Anschlussdarstellung), Kasten + Symbole mit BMK (**Schachtelung**, abschaltbar je Funktionsgruppe).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_k_einsatzmoeglichkeiten.htm
- Zuordnung eines Schaltzeichens zum Gerät: durch Lage im Kasten, durch BMK am Kasten oder durch **Übernahme von
  links (bzw. oben)**: ohne eigenes BMK erbt ein Symbol das BMK des links/oben liegenden Symbols mit Einfügepunkt
  auf gleicher Höhe; liegt es im Kasten, erbt es das Kasten-BMK (Symbole außerhalb links zählen dann nicht).
  Lampe ohne BMK aus dem Kasten gezogen → Lampe ohne BMK; -H1 in -U1 geschoben → -U1-H1.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_k_arbeitsweise.htm
- Schachteln: BMK hat nur zwei Ebenen voll auswertbar; das vorderste BMK steht allein, der Rest im zweiten Teil.
  Es wird in den Kasten geschachtelt, dessen Einfügepunkt näher ist und in dem der Einfügepunkt des Zeichens
  liegt; Kasten im Kasten wird mitgeschachtelt (-U1-U2-H1). Nicht durch Kästen schachtelbar: **Abbruchstellen,
  Potentialanschlüsse, Strukturkästen**. Abschalten (Projekt > Struktur > Weitere > Vererbung) wirkt nur auf
  Zeichen mit eigenem BMK.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_k_schachteln.htm
- Gerätekasten bekommt eigenes BMK; Rasterfang bestimmt Lage.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_h_kasteneinfuegen.htm
- **Geräteanschlüsse** (ein- oder zweiseitig): zweiseitige haben eine gemeinsame Bezeichnung (wie Klemme),
  interne/externe Seite unterscheidbar, 4 Varianten, Anschlusslogik „Potential weiterleiten“, je ein
  Direktanschluss intern und allgemeiner Anschluss extern. Geräteanschlüsse **im Kasten ohne eigenes BMK sind
  immer Nebenfunktionen und zeigen nie Querverweise** (der Kasten verweist). Außerhalb eines Kastens werden sie
  querverwiesen; Hauptfunktion kann Geräteanschluss oder Kasten sein.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_h_anschluesseeinfuegen.htm
- Eingabe `-H4:1` im sichtbaren BMK trennt am ersten `:` die Anschlussbezeichnung ab; das vollständige BMK enthält
  nie die Anschlussbezeichnung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_k_anschluesseauswerten.htm

**Bedeutung für QET:** Ein EPLAN-Gerätekasten ist **ein Betriebsmittel** – in QET am besten ein Element (Kasten mit
Anschlüssen) mit label und Artikel; die Pins sind Anschlüsse des Elements, keine Einzelelemente mit Verweisen.
Verteilt dargestellte Kästen (gleiches BMK auf mehreren Blättern) → ein Master (mit Artikel), Rest Slaves. Symbole
ohne Kennzeichen im Kasten gehören zum Kasten-BMK (vollständiges Kennzeichen beim Verknüpfen und in der Stückliste
= Kasten-BMK). Geschachtelte BMK (-A1-X1) im Original vollständig als label übernehmen. Innenschaltbilder in
Kästen sind oft in Grafik umgewandelte Symbole (§14) → in QET reine Grafik ohne Anschlusslogik.

## 8. Strukturkästen grafisch

- Strukturkasten = Gruppierung, **kein Betriebsmittel**, kein Artikel, Standardlinienart nach Norm
  (strichpunktiert). Alles, dessen **Einfügepunkt** im Kasten liegt, steht zum Kasten wie sonst zur Seite; die
  Struktur wird beim Verschieben/Größenändern neu berechnet. Strukturkästen erben **nicht** von links/oben
  (Unterschied zum Gerätekasten). BMK-Übergabe auch, wenn am Schaltzeichen nur Anlage/Ort steht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_k_grundlagen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_h_einfuegen.htm
- Geschachtelte Strukturkästen (Vererbung > Verschachtelung > Strukturkasten an): Kennzeichen werden zu
  Unterkennzeichen verkettet (`=A2.A3.A4+O2.O3.O4-F3`), aus → nur innerster (`=A4+O4-F3`). Sicht von außen nach
  innen; das Zeichen ergänzt zuletzt aus dem innersten Kasten. `[Umschalt]` beim Herausziehen behält das BMK.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_h_geschachtelte.htm
- Option **Strukturkästen mit Aussparung zeichnen**: linke obere Ecke ausgespart, BMK dort (Eigenschaftsanordnung
  „Links oben, innen“); statt BMK (sichtbar, ID 20010) auch BMK (vollständig, ID 20006) oder BMK
  (identifizierend, ID 20005) platzierbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenprojektallgemein.htm
- Querverweisfähig über „Hauptplatzierung“ (siehe querverweise.md §10); „Gehe zu (Gegenstück)“ springt zur
  Hauptplatzierung. Bei Maßstabsänderung skalieren Kastentexte wie Schaltzeichentexte (außer Schaltschrankaufbau).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_k_grundlagen.htm
- Anlagenübersichten: einpolige Seiten mit Strukturkästen, verbunden durch Kabeldefinitionslinien.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_spinnenplaeneundanlagenuebersichten.htm

**Bedeutung für QET:** Ortsrahmen = strichpunktierter Rahmen (`add_shape` dashdot); entscheidend für das
**Kennzeichen** ist, ob der Einfügepunkt (in QET: Elementposition) des Symbols im Rahmen liegt – nicht, ob die
Grafik den Rahmen schneidet. Das Verknüpfungsskript muss das Ortskennzeichen aus dem Rahmen ableiten, in dem der
Element-Einfügepunkt liegt; bei geschachtelten Rahmen der innerste (Verkettung nur, wenn das Original
`+O2.O3`-Form zeigt). Aussparung oben links mit BMK ist Optik.

## 9. Kästen als Polylinie (DC2, SC2, PLCC3 …)

- Symbole in SPECIAL: DC2 Gerätekasten, DCF2 Fluid-Gerätekasten, **PLCC3 SPS-Kasten**, SC2 Strukturkasten, MC2
  Makrokasten, BB2 Bohrbildkasten (MC2/BB2 nur in Makroprojekten). Abschluss mit „Polylinie abschließen“;
  Rücktaste nach „Einfügen > Kasten“ öffnet gefilterte Symbolauswahl.
- Polylinienkästen haben nur **eine** Eigenschaftsanordnung am Einfügepunkt = der am weitesten links liegende
  Eckpunkt (bei mehreren übereinander der oberste).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_h_polygonalekaesten.htm

**Bedeutung für QET:** L-förmige oder abgestufte Kästen im Original sind solche Polylinienkästen; in QET als
Polygon-Form zeichnen. Für „liegt im Kasten“ (Kennzeichen) Punkt-in-Polygon prüfen, nicht Rechteck.

## 10. Pfad-Funktionstexte und Funktionstext (automatisch)

- Pfad-Funktionstext: freier Text im Schaltplanpfad (eigene Ebene **EPLAN110**, Grafik.Pfad-Funktionstexte);
  ausgewertet nur auf Logikseiten, in SPS-Querverweisen, Klemmenplänen, Stücklisten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_pfadtextstart.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_pfadtexteinfuegen.htm
- Jede Funktion hat **Funktionstext** (manuell) und **Funktionstext (automatisch)** (nicht editierbar): ist der
  manuelle leer, steht dort der gefundene Pfad-Funktionstext. Standard: nur übernommen, wenn Einfügepunkt von
  Text und Schaltzeichen **exakt** über-/untereinander liegen. Option „Pfad-Funktionstext auf den
  Schaltplanpfad erweitern“: irgendwo im selben Pfad genügt; bei mehreren gewinnt der erste (Suche links→rechts,
  unten→oben); bei Normblatt mit Spalten **und** Zeilen nur im selben Feld. Zeilenumbrüche werden in der Anzeige
  entfernt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_pfadtextprinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenprojektallgemein.htm
- Suchregeln: allgemeine Funktionen – Pfad je nach **Auswertungsrichtung des Normblatts** von oben nach unten bzw.
  links nach rechts, der oberste/linkste gewinnt; für Haupt- und Nebenfunktionen getrennt. Verbindungslisten zeigen
  den Funktionstext des angeschlossenen Zeichens, Betriebsmittel-/Stücklisten nur den der Hauptfunktion.
- **Klemmen/Steckerkontakte** (senkrecht): erst im eigenen Pfad nach oben, dann nach unten, dann die Pfade
  **links** davon jeweils von unten nach oben (waagerecht analog: links, rechts, dann Pfade darüber). Mehrstockklemmen
  je Etage; Verbindungsliste nimmt die gefundene Etage, Stückliste Etage 0.
- **SPS-/Geräteanschlüsse**: erst entgegen, dann in Anschlussrichtung (umkehrbar); nichts gefunden → Funktionstext
  (automatisch) des verbundenen Sensors/Aktors; optional „Erweiterte Pfad-Funktionstextübernahme“. Kanalsymbole
  richten sich nach dem ersten Anschluss bzw. Einfügepunkt. Auf SPS-Übersichtsseiten zeigt der Anschluss den
  automatischen Funktionstext des zugehörigen Anschlusses auf der allpoligen Seite.
- Klemmenleisten-/Steckerdefinitionen und Potentialanschlüsse werten keine Pfad-Funktionstexte aus.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_pfadtextauswertung.htm
- Beim Ausschneiden/Zurückbuchen ändert sich ein nur geerbter Funktionstext.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_pfadtextprinzip.htm
- Funktionstexte aller Funktionen eines BMK abgleichen: „Funktionstexte abgleichen“ überträgt Text inkl. Format und
  Übersetzung in Funktionstext und Funktionstext (automatisch).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_funktionstexteabgleichen.htm

**Bedeutung für QET:** Die Spaltentexte oben im EPLAN-Blatt („Pumpe ein“, „Not-Halt“) sind Pfad-Funktionstexte und
gehören logisch als `function` in die Elemente darunter – die Klemmen im Klemmenplan und die SPS-Kanäle zeigen sie.
Beim Nachbau: Text als freien Text übernehmen **und** `function` der Elemente im selben Pfad füllen (Element hat
eigenen Text → der gewinnt). Für Klemmen gilt die Suche nach links (Klemme rechts neben dem Text-Pfad erbt den
Text des linken Pfads) – das erklärt, warum im Original-Klemmenplan Klemmen einen Funktionstext tragen, der nicht
über ihnen steht. Haus-Regel „nächstgelegener Funktionstext“ (SKILL qet-schaltplan-zeichnen) ist eine Näherung;
korrekt wäre: gleicher Pfad oben→unten, dann Pfade links. SPS-Kanal ohne Pfadtext → Funktionstext des
angeschlossenen Sensors/Aktors übernehmen.

## 11. Texte, Eigenschaftstexte, Andocken, direkte Bearbeitung

- Textarten: freie **Texte**, **Eigenschaftstexte** (zeigen Eigenschaften des Schaltzeichens), **Sondertexte**
  (Projekt-/Seiteneigenschaft, aktualisieren sich; auch Pfad-Funktionstext), **Platzhaltertexte** (Formulare).
  Schriftarten 1–10 aus Projekt-/Firmeneinstellung, nicht im Projekt gespeichert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_texte.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_d_firmaschriftarten.htm
- **Andocken**: angedockte Texte übernehmen Winkel/Ausrichtung des Haupttexts, behalten Farbe/Größe, bewegen sich
  als Block und fließen nicht ineinander; Eigenschaftstexte werden in der Registerkarte Anzeige an-/abgedockt.
  Verschobene Eigenschaftstexte (nur abgedockte, `Strg+B`) lassen sich auf die Symbol-Standardposition
  zurücksetzen; `[Umschalt]` = Viertelraster. Auch der Kontaktspiegel ist so verschiebbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_texteandocken.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_symboltexteverschieben.htm
- Zeilenabstand 1 = 1,9 × Schriftgröße (Höhe „M“); Absatzabstand zwischen angedockten Eigenschaften, Zeilenabstand
  kommt hinzu. Textrahmen aus Ebene oder je Text; Bezugspfeil vom Text zur Funktion möglich.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/layermanager_d_ebenenverwaltung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_r_format.htm
- Positionsrahmen: mit „Positionsrahmen aktivieren, Textbreite fix, Texthöhe fix, Text einpassen erlaubt“
  verkleinert EPLAN die Schrift, bis der Text passt (nur verkleinern), automatisch bei Auswertungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_texteeinpassen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_texteeinpassen.htm
- Direkte Bearbeitung (`F2`): nur Funktionen (nicht Abbruchstellen, Strukturkästen), keine Sondertexte, keine
  Formatierung; `Tab` springt grafisch von oben links nach unten rechts durch Haupteigenschaften und angedockte
  Texte. Liste der direkt editierbaren Eigenschafts-IDs (Auswahl): 20010 BMK sichtbar, 20011 Funktionstext,
  20025 Graviertext, 20026 Funktionsdefinition, 20027 technische Kenngrößen, 20030 Klemmen-/Steckerkontakt-
  bezeichnung, 20034 Etage, 20038 Anschlussbezeichnungen, 20100 Artikelnummer, 20400 SPS-Adresse, 20402 symbolische
  Adresse, 20406 Steckerbezeichnung, 20407 Kanalbezeichnung, 20809 Sortierkennung, 31002 Verbindungsquerschnitt,
  31004 Verbindungsfarbe/-nummer, 31011 Verbindungsbezeichnung, 31013 Quelle/Ziel vertauschen, 31050 Paarindex,
  31142 Verbindung: Zugehörigkeit; Strukturblöcke 1120/1140 Anlage, 1220/1240 Einbauort, 1420/1440
  Aufstellungsort (je ohne/mit Vorzeichen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/inplaceeditinggui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/inplaceeditinggui_k_eigenschaften.htm
- Option „Anschlussbezeichnung gedreht darstellen“: Symbole können zwei Sätze Anschlussbezeichnungen haben
  (ungedreht/90° gedreht, DIN); wirkt auch auf Klemmenbezeichnungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_anschlussbezanzeigen.htm
- Ansicht-Schalter: Einfügepunkte (`I`), unsichtbare Elemente (`U`), **Externe Ziele** (Pfeil an externen
  Anschlüssen nach Anschlusslogik intern/extern), **Kabelverbindungen** (Pfeil an Anschlüssen mit Eigenschaft
  Kabelanschluss), leere Textfelder.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_externezieleanzeigen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_kabelanschluesseanzeigen.htm

**Bedeutung für QET:** Eigenschaftstexte entsprechen QET-Elementtexten (dynamische Texte an label, function …);
Funktionstext oben und BMK darunter sind in EPLAN oft **angedockt** – im Nachbau als Elementtexte am Element
halten, nicht als lose freie Texte, damit sie mitwandern. EPLAN-Schriftgröße × 1,9 ≈ Zeilenraster beim
Übernehmen mehrzeiliger Blöcke.

## 12. Blockeigenschaften (zusammengesetzte Anzeigetexte)

- Paar **Blockeigenschaft: Format [n]** (welche Eigenschaften, Trennzeichen) + **Blockeigenschaft [n]** (Ergebnis)
  mit gleichem Index; Ergebnis ist ein einfacher, nicht mehrsprachiger String, aktualisiert sich beim Anzeigen.
  Über „Nächste Stufe“ erreichbar: **indirekte Eigenschaften** von Projekt, Seite, verbundenen Objekten, angeschlossenen
  Verbindungen/Kabeln, Hauptfunktion, Artikeln. Eigene Ebene, damit in Sonderfarbe als „nicht hier editierbar“
  erkennbar.
- Vorrang: Format an der Funktion > am Symbol > am Projekt (je Funktionskategorie, z. B. Blockeigenschaft: Format
  (Klemme)/(Abbruchstelle)). Bis zu zehn Ersetzungstext-Tabellen (Projekt bzw. Formular, Formular hat Vorrang).
- **Blockeigenschaft: Format (Funktionstext, automatisch) (ID 20545)** baut einen automatischen Funktionstext
  (auch aus dem Pfad-Funktionstext); ein manueller Funktionstext hat Vorrang; Prüfung 007033 findet Doppelfälle.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blockproperties_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blockproperties_k_vordefinition.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blockproperties_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blockproperties_h_blockeigenschaftendefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blockproperties_h_blockeigenschaftenanzeigen.htm

**Bedeutung für QET:** Texte am Symbol, die Daten **anderer** Objekte zeigen (z. B. am Kanal das Kabel oder am
Klemmensymbol das Ziel), sind Blockeigenschaften. In QET nicht dynamisch nachbildbar → als fester Elementtext
oder als Report-/Verknüpfungsergebnis rechnen (Vermutung: im Verknüpfungsskript nachziehen, damit er nach
Änderungen stimmt).

## 13. Drehen, Spiegeln, Symbolvarianten

- Symbole und Makros nur in **90°-Schritten**, weil intern die **Symbolvariante getauscht** wird: Drehen 90/180/270°
  = Variante B/C/D (von A), Spiegeln = E–H (die Variante auf der anderen Seite der Achse). Eigenschaftsanordnung
  dreht/spiegelt mit. Grafik beliebig. Bezugspunkt = erster Anschluss oder **Transformationspunkt (ID 16045)** je
  Variante.
- Makros (*.ems/*.ema) drehen über **Makrovarianten** (Logik bleibt, Grafik aus der Variante); fehlt eine, wird
  symbolweise transformiert (Systemmeldung). `[Umschalt]` = symbolbasiert, `[Strg]` = Kopie.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_drehenspiegeln.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_elementedrehenspiegeln.htm

**Bedeutung für QET:** EPLAN-Variante A–H ↔ QET-Orientierung + Spiegelung; beim Import eines EPLAN-Symbols alle
Varianten nicht einzeln übernehmen, sondern Variante A als Element und Drehung/Spiegelung über QET. Achtung: EPLAN
kann je Variante andere Grafik haben (z. B. Text immer lesbar) – dann eigene QET-Elemente (siehe symbole.md).

## 14. Gruppen, Blöcke, Schaltzeichen in Grafik umwandeln

- **Gruppen** (Option „Gruppierung berücksichtigen“): Aktionen auf die ganze Gruppe; Eigenschaftsdialog nur für ein
  Objekt nach Rang der Darstellungsart: Allpolig > Übersicht > Paarquerverweis > Einpolig > RI-Fließbild > Extern;
  logische vor grafischen Objekten. `[Umschalt]` bearbeitet ein Einzelteil ohne Auflösen. Gerätegruppen ebenso.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_gruppierung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_elementegruppieren.htm
- **Block bilden**: Grafik + sprachneutrale Texte zu einem Datensatz (Blockreferenz, Name für DXF); mit einem
  Schaltzeichen wird die Grafik Teil des Schaltzeichens. Mehr als ein logisches Objekt → Abbruch.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_blockbildenaufloesen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_d_blockreferenzeigenschaften.htm
- **Schaltzeichen in Grafik umwandeln**: Logik geht unwiderruflich verloren, nicht in Auswertungen (anders als
  Darstellungsart „Grafik“, die umkehrbar ist). Gedacht für Innenschaltungen in Gerätekästen. Symbole im Symbol-,
  Formular- oder Normblatteditor werden automatisch zu Grafik.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_schaltzeicheningrafik.htm
- Format übertragen: Anzeige-/Format-Registerkarte nur zwischen gleichem Objekttyp (Tabelle: Bauteile/Klemmen/
  Abbruchstellen nur Anzeige; Kabel, Kästen, Abschirmung Anzeige + Format; Grafik/Texte nur Format).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_formatuebertragen.htm

**Bedeutung für QET:** Symbole im Original, die zwar wie Kontakte aussehen, aber weder Querverweis noch
Kennzeichen haben und in keiner Liste stehen, sind meist Grafik → in QET als Grafik (Form) oder Element ohne
Anschlüsse, nicht verknüpfen. Darstellungsart „Grafik/Extern/Paarquerverweis“ färbt EPLAN auf dem Bildschirm
anders (Benutzereinstellung 2D), im PDF nicht unterscheidbar.

## 15. Einpolige Darstellung, Stränge, Stranganschlüsse

- Einpolig: vereinfachte Zeichen mit 1–2 Anschlüssen, Kabel als eine Linie; eigener Seitentyp **Schaltplan einpolig
  (I)**, eigene Symbolbibliothek. Einpolige Zeichen sind auf ein- und allpoligen Seiten erlaubt. Eine Funktion aus
  dem Navigator wird je nach Seitentyp ein- oder allpolig platziert; Tausch, wenn beide Bibliotheken ein Symbol mit
  gleicher Nummer und Funktionsdefinition haben. Abweichende Daten verteilter Darstellungen projektweit abgleichbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_hintergrund.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_start.htm
- Vorgehen: Seite „Schaltplan einpolig“, Filter „IEC-Symbole einpolig“ (z. B. QL3 Nr. 124 Motorschutzschalter,
  M3 Nr. 62 Motor), Autoconnect, darauf **Verbindungsdefinitionspunkt mit Darstellungsart „Einpolig“** und Eigenschaft
  **Anzahl der Funktionen** (z. B. `3+PE`, wird am Schrägstrich angezeigt).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_h_stromlaufplaenezeichnen.htm
- Kabel: Darstellungsart der Kabeldefinition folgt dem Seitentyp; Kabeleigenschaft „Anzahl der verwendeten
  Verbindungen (einpolige Kabel)“; Abschirmung einpolig nur als Kabeldefinition; Verbindungsdefinitionspunkt mit
  Kabeldefinitionslinie auf einpoliger Seite → Verbindung wird Kabelverbindung (Zugehörigkeit „Kabel“).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_besonderheitenkabel.htm
- Klemmen einpolig: nur Klemmen- **oder** Leistenbezeichnung; keine Verbindungs- und Artikeldaten an der
  einpoligen Klemme, Artikel an der Klemmenleistendefinition.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_besonderheitenklemmen.htm
- SPS einpolig: Anschluss = Signalklemme, Versorgung oft weggelassen; Identität BMK + Anschlussnummer;
  **Bus-Anschlüsse** verbinden Master/Slaves als einpolige Kabel; Netzwerkstrukturen mit SPS-Kästen, Bus-Anschlüssen,
  Verbindungssymbolen, Kabeldefinitionen, Details in Kabeldarstellungsplänen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_sps.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_bustopologien.htm
- **Allpolige Verbindungen am einpoligen Verbindungsdefinitionspunkt** (Registerkarte „Geräte verschalten“):
  Voraussetzung: allpolige Funktionen (z. B. Steckerkontakte) als **nicht platzierte** Funktionen, einpolige
  Funktion ist Hauptfunktion („Nicht-platzierte Nebenfunktionen an der Hauptfunktion verwalten“). Gleich viele
  Anschlüsse links/rechts markieren → Verbindungen (mit Kabel: Kabelverbindungen, sonst Ader/Draht), Daten am Punkt
  gespeichert, Verwaltungsart **„Allpolig (Automatisch)“**. Nur der grafisch erste solche Punkt einer Verbindung
  zählt; Doppelung mit allpolig gezeichneten Verbindungen → Prüffehler.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_verbindungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_h_allpoligeverbindungendefinieren.htm
- **Stränge**: Verbindungen hinter der Quelle bündeln, dicke Linie, vor dem Ziel wieder auflösen.
  Stranganschlüsse: Gerade, Winkel, T-Stück, Kreuzung, Verteiler T-Stück; dünne Seite = Einzelverbindung, dicke =
  Strang. Identifizierend ist die **Stranganschlussbezeichnung**, sie tritt **paarweise** auf (Eintritt/Austritt);
  gebündelter Eintritt als `1/2`. EPLAN prüft Paarigkeit (fehlend oder >2 gleiche = Meldung). Über Abbruchstellen
  seitenübergreifend; dick erst nach „Verbindungen aktualisieren“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_straenge_in_einpoligerdarstellung.htm
- Kabeldefinitionslinie an den Strang = nur Kabelübersicht; über die allpoligen Abschnitte = auch Kabelplan mit
  Quelle/Ziel je Ader. „Stranganschlüsse überschreiben“ ersetzt 1, 2, 3, 4 durch Aderfarben (BK, BU, BN, GNYE).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_h_stranganschluesse_vom_kabel_uebernehmen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_h_gostnorm.htm

**Bedeutung für QET:** QET kennt keinen Strang. Logisch ist jede Ader eine eigene Verbindung von Quelle zu Ziel;
der Strang ist nur Bündelungsgrafik. Nachbau: dicke Linie als Grafik (oder Leiter mit großer Strichstärke ohne
Logik) und die Einzeladern als Leiter **vom Eintritts- zum Austrittsstranganschluss gleicher Bezeichnung**
(unsichtbar geführt oder über Folio-Reports), damit Klemmen-/Kabelpläne stimmen. Paarbildung über die
Stranganschlussbezeichnung (bzw. Aderfarbe nach „überschreiben“). Einpolige Übersicht (`3+PE`): nur Grafik +
Text, keine Einzeladern erfinden; allpolige Auswertung in EPLAN nur, wenn „Geräte verschalten“ belegt ist (dann
steht es im Original-Kabelplan).

## 16. Ebenen (Ebenenverwaltung)

- Jedes Element liegt auf einer Ebene (Grafik, Symbolgrafik, Autoconnect-Linien, Eigenschaftsplatzierungen,
  Sondertexte). Je Ebene: Linientyp, Musterlänge, Stärke, Farbe, Schriftgröße, Ausrichtung, Winkel, Zeilen-/
  Absatzabstand, Textrahmen, **Sichtbar**, **Drucken**, Gesperrt (nur DXF), **Hintergrund**, **Skalierbar**, 3D.
  Änderungen sofort gespeichert; Konfiguration als `*.elc` export-/importierbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/layermanager_d_ebenenverwaltung.htm
- Standardebenen `EPLANnnn` (Name = ID, z. B. EPLAN100 Grafik.Allgemein, **EPLAN300** allgemeine Symbolgrafik,
  EPLAN305 Symbolgrafik.Gerätekasten, EPLAN105 Mittellinien, EPLAN107 Bemaßungen, EPLAN108 Grafik.Texte,
  EPLAN110 Pfad-Funktionstexte, EPLAN705 Hintergrundbild); Name/Beschreibung schreibgeschützt, nicht löschbar.
  Eigene Ebenen ab ID 1000 (in `.elc` als `A1426`). Punkt in der Beschreibung = Unterknoten. Umbenennen auf
  vorhandenen Namen **verschmilzt** Ebenen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/layermanager_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/layermanager_h_ebenenbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_linienzeichnen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_r_text.htm
- Element-Eigenschaft „Unsichtbar“ (Aus Ebene / Ja / Nein) **überschreibt** die Ebene. Unsichtbare Elemente optional
  anzeigen (eigene Farbe) und drucken/exportieren (Benutzereinstellung, wirkt auf PDF/DXF).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/layermanager_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenbenutzerallgemein.htm
- Ein Schaltzeichen hat keine eigene Ebene (nicht als Ganzes ausblendbar); seine Grafik liegt meist auf EPLAN300,
  kann aber auf mehrere Ebenen verteilt sein (Farben, Linientypen). Kästen, Abschirmung, Kabeldefinitionslinie sind
  Einzelgrafik auf eigener Ebene. DXF-Export nur mit Ebenennamen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/layermanager_k_prinzip.htm
- Zweck: z. B. Kabeldefinitionslinien, Klemmenleistentexte, Hilfslinien oder Kommentare ausblenden.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/layermanager_k_start.htm

**Bedeutung für QET:** QET hat keine Ebenen. Was im EPLAN-PDF fehlt, kann auf einer unsichtbaren/nicht druckenden
Ebene liegen (z. B. Kabeldefinitionslinie, Anschlussbezeichnungen) – die Information steckt trotzdem in den
Auswertungen des Originals (Kabelplan, Klemmenplan). Inhalt aus den Listen übernehmen, Darstellung wie im PDF
(in QET ggf. Text unsichtbar/weglassen). Farben im PDF (z. B. blaue Bemaßung = EPLAN107) verraten die Ebene.

## 17. Bemaßung

- Typen: Einzel- (nur waagerecht/senkrecht), freie (beliebiger Winkel), Ketten-, steigende, Bezugspunkt-,
  Winkel-, Kreisbemaßung (Radius). Über Objektfang, nicht rasterabhängig; `[Strg]` beim Maßpunkt = Mitte zweier
  Punkte (nicht bei Kreis).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_h_einzelbemassung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_h_bemassungmitmittelpunkten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_h_kreisbemassung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_h_winkelbemassung.htm
- Kette: jedes Teilstück einzeln; steigend: alle Werte vom Startpunkt, gleiche Höhe, Zahl am Teilstückende
  (Startwert optional, Einstellung wirkt auch rückwirkend); Bezugspunkt: Werte vom Start, Maßlinien gestaffelt,
  Zahl mittig.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_h_kettenbemassung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_h_steigendebemassung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_h_bezugspunktbemassung.htm
- Elemente: Maßzahl (Standard 3,5 mm, zentriert, manuell überschreibbar → unmaßstäbliche Maße, Präfix/Suffix,
  Einheit je Maß, Genauigkeit, Null unterdrücken, Bruch, zwei Einheiten `88 mm[3,46'']`), Maßlinie und
  Maßhilfslinien 0,25 mm auf Ebene EPLAN107 (blau), Hilfslinien 2 mm Überstand. Projekteinstellungen gelten nur für
  neue Bemaßungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_k_bemassungenprinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_d_projektbemassung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_r_bemassung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_r_linien.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/dimensiongui_h_bemassungbearbeiten.htm
- Stretchen dehnt bemaßte Punkte samt Bemaßung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_elementeveraendern.htm

**Bedeutung für QET:** Bemaßungen kommen fast nur auf Schaltschrankaufbau-Seiten vor; QET hat keine Bemaßung →
Linien + Text als Grafik. Manuelle Maßzahlen (unmaßstäblich) im Original nicht nachrechnen, Text übernehmen.

## 18. Redlining (PDF-Kommentare)

- Ablauf: Projektbearbeiter exportiert PDF, gibt es in Acrobat zum Kommentieren frei bzw. versendet es zur
  Überprüfung; Kommentator zeichnet Anmerkungen (Acrobat Reader), Kommentartext, Antworten, Status; Bearbeiter führt
  zusammen und importiert (`Seite > Importieren > PDF-Kommentare`) – das EPLAN-Projekt muss im **selben Verzeichnis**
  liegen wie beim PDF-Export. Kommentare-Navigator mit „Gehe zu (Grafik)“; Entfernen über Projekt komprimieren
  (Option Kommentare).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/redlining_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/redlining_k_vorgehensweise.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/redlining_k_projektkommentator.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/redlining_h_pdfkommentareeinlesen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/redlining_h_projektkommentarebearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/redlining_h_projektkommentareloeschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/redlining_h_freigabe.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/redlining_h_permailversenden.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/redlining_h_pdfkommentare.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/redlining_h_zusammenfuehren.htm
- Kommentar-Eigenschaften: Verfasser (Windows-Anmeldename), Erstellungs-/Änderungsdatum, Text, Status (änderbar),
  Antworten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_r_kommentare.htm

**Bedeutung für QET:** Vermutung: Zuordnung beim Import läuft über die Seitenposition im PDF; für den eigenen
QET-PDF-Export (pdf-export.md) heißt das: Seitenreihenfolge/-größe stabil halten, wenn Kommentare zurückgespielt
werden sollen. Kommentare in Original-PDFs (Annotationen) sind kein Planinhalt – beim Nachbau ignorieren.

## 19. Ablaufdiagramme: Funktionsdiagramm und GRAFCET

- Typen: textorientiertes und symbolorientiertes **Funktionsdiagramm** (Weg-Zeit/Weg-Schritt nach VDI 3260) und
  **GRAFCET** (DIN EN 60848). Seitentyp empfohlen „Übersicht“, Raster 3 mm; Formulare F28_001.f28 (Text) und
  F28_002.f28 (Symbol), GRAFCET ohne Formular. Spezialsymbole OS1/OS2/OS3 aus `OS_SYM_ESS.slk` (nur
  Eigenschaftsplatzierung, keine Grafik, Darstellungsart **Funktional**); OS3 Variante C nur BMK, D nur Variable.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functionaldiagramgui_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functionaldiagramgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functionaldiagramgui_h_funktionsdiagerstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functionaldiagramgui_h_grafcetdiagerstellen.htm
- Platzieren aus dem Betriebsmittel-Navigator „Platzieren funktional > Mit Symbolgrafik/Mit Makrografik“: es entsteht
  eine **Kopie als Nebenfunktion** mit Darstellungsart Funktional (eigenes Icon), mehrfach platzierbar, kein
  Autoconnect, von Prüfung 007004 (Anschlussbezeichnung doppelt) ausgenommen. Texte und Grafik werden gruppiert.
  Eigenschaften: sichtbares BMK, Funktionstext, **Zustand [1–10] (ID 20285)**; Zustand [10] = GRAFCET-Variable.
  Makrografik: erste Darstellungsart und Variante des Artikelmakros der Hauptfunktion; nur verkleinert (Zielgröße).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functionaldiagramgui_k_funktionsdiagramme.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functionaldiagramgui_d_einstellungen.htm
- Ablauffolgen zeichnet der Anwender (Polylinien; Einfügen > Ablaufdiagramm: Pfeil, Signalpunkt, UND-, ODER-Bedingung;
  GRAFCET: Schritt, Schritt mit Transition, Aktion, Aktion mit Anbindung, Transition; `*` = einzutragender Text,
  `[Umschalt]`+Klick ändert Text ohne Gruppe aufzulösen). Makros OS_ESS (Table_GRAFCET.ema, Numbers.ema, Arrows.ema).
  Schritte und Transitionen wechseln sich ab; Transitionsname links in Klammern, Bedingung rechts.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functionaldiagramgui_k_grafcet.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functionaldiagramgui_h_grafcetdiagerstellen.htm

**Bedeutung für QET:** Ablaufdiagramme sind reine Grafik mit BMK-Texten. In QET: Formen + Texte; die Einträge mit
BMK als Slaves (nicht Master!) oder ohne Verknüpfung – sie sind in EPLAN Nebenfunktionen ohne elektrische Logik und
dürfen nicht in Stückliste/Klemmenplan. Vermutung: im PDF als Sprungziel der Betriebsmittel-Sprungfolge enthalten.

## 20. Blockbearbeitung, Bearbeitungsbereiche, Sonstiges

- **Blockbearbeitung**: mehrere markierte Objekte gleichen Typs gemeinsam ändern; unterschiedliche Werte zeigt EPLAN
  als `<<...>>`; gemischte Typen → Dialog Objektauswahl; „Eigenschaftenübersicht“ ändert funktionsunabhängige
  Eigenschaften (Funktionstext, Gravur, technische Kenngrößen) typübergreifend. Seiten nur im Seiten-Navigator und
  nur im selben Projekt; Schaltzeichen nur auf der offenen Seite; Navigatoren projektweit inkl. nicht platzierter
  Funktionen. „Objekte gleichen Typs markieren“ (z. B. alle Kabel einer Seite).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/multipleselection_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/multipleselection_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/multipleselection_h_objekteblockbearbeitung.htm
- **BMK zusammenführen**: mehrere Schaltzeichen markieren, Eigenschaftenübersicht > funktionsunabhängig > Name
  (vollständig) setzen → ein Betriebsmittel, Querverweise entstehen automatisch.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_bmkzusammenfuehren.htm
- **Bearbeitungsbereiche**: Filterschemata über Strukturkennzeichen und Eigenschaft „Gewerk“ (Seiten, Bauräume,
  Funktionen, Verbindungen); Navigatoren zeigen nur den Bereich (`%` vor dem Projektnamen). „Anwenden auf gesamtes
  Projekt“ wirkt dann nur im Bereich; **Verbindungsaktualisierung nur im Bereich** (Verbindung gehört dazu, sobald ein
  Ziel dazugehört) → nicht aktuelle Verbindungen außerhalb verfälschen Anschlusspläne. Auswertungsblöcke nur, wenn die
  Startseite im Bereich liegt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/workingsection_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/workingsection_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/workingsection_h_bereichedefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/workingsection_h_adminbereichedefinieren.htm
- Gerät einfügen/platzieren: Artikelmakro, einzelne Funktionen (Funktionsschablonen, `N` blättern, Leertaste neu
  bei Hauptfunktion), Makro- oder Symbolauswahl (gefiltert nach Darstellungsart + „Undefiniert“).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_d_geraeteinfuegen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_d_platzieren.htm
- Projekteinstellungen Grafische Bearbeitung > Allgemein (weitere): Makrokästen mit einfügen (Voraussetzung für
  Makro-Aktualisierung und Artikeldatenzuweisung des Kastens), Makros referenzierend einfügen (Makroprojekt),
  Suchreihenfolge SPS umkehren, alternative Eigenschaftsanordnung (GOST: BMK rechts oben), Eigenschaftsanordnungen
  aus Projekt übernehmen (sonst „Benutzerdefiniert“), min. Schriftgröße für Prüfung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenprojektallgemein.htm
- Makros aktualisieren: Zuordnung standardmäßig über Funktionsdefinition, optional grafisch (Position); Standard
  überträgt nur Grafik, „Nur das BMK beibehalten“ auch Logik (Anschlussbezeichnungen, Funktionstexte, Symbole,
  Artikel), BMK inkl. Abbruchstellen bleiben.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenmakrosaktualisieren.htm
- Hyperlinks: Text mit Datei/URL/Mail; Eigenschaftswerte, die Hyperlinks sind, werden automatisch erkannt, auch in
  Auswertungen und **im exportierten PDF**. QR-Codes aus Text/Eigenschaften (nicht dreh-/spiegelbar), in Formularen
  „Als QR-Code anzeigen“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_hyperlinks.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_qrcodes.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_d_hyperlinkkopieren.htm
- Bilddateien (GIF, JPG, BMP, ICO, PNG, TIFF) kopiert nach `Images` oder referenziert (nicht in Datensicherung);
  fehlende Datei → Rechteck mit Pfad.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_bilddateien.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_d_bildkopieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_d_bildeinlagern.htm
- Bildexport: je Seite eine Datei (BMP, GIF, JPG, PNG, TIFF), Name = vollständiger Seitenname, Projekt → Unterordner,
  feste Breite in Pixel, überschreibt ohne Rückfrage, optional Schwarz-Weiß.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedgraphicexportgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedgraphicexportgui_h_seitenexportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedgraphicexportgui_d_exporteinstellungen.htm
- Undo/Redo linear je Projekt und Anwender, auch über Liste; Liste nur temporär; nicht rückgängig machbare Aktion
  (z. B. Artikeldatenabgleich) löscht die Liste. Ansicht zurück/vorwärts getrennt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_undoundredo.htm
- Bedienung knapp (nur überflogen): Eingabezeile (erste Eingabe absolut, folgende relativ, immer grafisches
  Koordinatensystem), Bezugspunktverschiebung, Orthogonalfunktion, Objektfang, Konstruktionsmodus, Trimmen/Fasen,
  Zoom, Tastenkürzel (u. a. `Strg`+Klick am Querverweis = Gegenstück, `F` = Gegenstück, `Alt+Bild↓` = nächste
  querverwiesene Funktion).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_kommandozeile.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_tastaturbefehle.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_cursortastatur.htm

**Bedeutung für QET:** Bearbeitungsbereiche erklären, warum ein Original-PDF stellenweise veraltete Klemmen-/
Kabelpläne enthalten kann (Verbindungen außerhalb des Bereichs nicht aktualisiert) – Schaltplan führt
(auswertungen.md). Hyperlinks in Eigenschaften werden im EPLAN-PDF zu Links; beim eigenen PDF-Export optional
nachbilden.

## 21. Quellen

Alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ :

- Grafischer Editor: gededitgui_k_start.htm, gededitgui_k_prinzip.htm, gededitgui_k_koordinatensysteme.htm,
  gededitgui_k_objektfang.htm, gededitgui_k_seitenmassstab.htm, gededitgui_k_texte.htm, gededitgui_k_texteeinpassen.htm,
  gededitgui_k_pfadtextstart.htm, gededitgui_k_pfadtextprinzip.htm, gededitgui_k_pfadtextauswertung.htm,
  gededitgui_k_drehenspiegeln.htm, gededitgui_k_gruppierung.htm, gededitgui_k_undoundredo.htm, gededitgui_k_kommandozeile.htm,
  gededitgui_k_tastaturbefehle.htm, gededitgui_k_cursortastatur.htm, gededitgui_k_trimmen.htm, gededitgui_k_zoom.htm,
  gededitgui_h_raster.htm, gededitgui_h_logikfang.htm, gededitgui_h_hintergrundbearbeiten.htm,
  gededitgui_h_horverausrichten.htm, gededitgui_h_gleichmaessigverteilen.htm, gededitgui_h_pfadtexteinfuegen.htm,
  gededitgui_h_texteandocken.htm, gededitgui_h_symboltexteverschieben.htm, gededitgui_h_texteeinpassen.htm,
  gededitgui_h_anschlussbezanzeigen.htm, gededitgui_h_externezieleanzeigen.htm, gededitgui_h_kabelanschluesseanzeigen.htm,
  gededitgui_h_elementedrehenspiegeln.htm, gededitgui_h_elementegruppieren.htm, gededitgui_h_blockbildenaufloesen.htm,
  gededitgui_h_schaltzeicheningrafik.htm, gededitgui_h_formatuebertragen.htm, gededitgui_h_linienzeichnen.htm,
  gededitgui_h_elementeveraendern.htm, gededitgui_h_hyperlinks.htm, gededitgui_h_qrcodes.htm, gededitgui_h_bilddateien.htm,
  gededitgui_d_bildkopieren.htm, gededitgui_d_bildeinlagern.htm, gededitgui_d_hyperlinkkopieren.htm,
  gededitgui_d_blockreferenzeigenschaften.htm, gededitgui_d_firmaschriftarten.htm, gededitgui_r_format.htm,
  gededitgui_r_text.htm, gededitgui_r_kommentare.htm.
  Nur überflogen (Bedienung): gededitgui_d_formatgrafik.htm, gededitgui_d_koordinateneingabe.htm,
  gededitgui_d_relativekoordinaten.htm, gededitgui_d_schrittweite.htm, gededitgui_h_bezierzeichnen.htm,
  gededitgui_h_bezugspunktverschiebung.htm, gededitgui_h_cursor.htm, gededitgui_h_eckenrunden.htm,
  gededitgui_h_eigenschaftsnamen.htm, gededitgui_h_einfuegepunkte.htm, gededitgui_h_elementeausrichten.htm,
  gededitgui_h_elementeauswaehlen.htm, gededitgui_h_elementeduplizieren.htm, gededitgui_h_elementekopieren.htm,
  gededitgui_h_elementeloeschen.htm, gededitgui_h_elementetrimmen.htm, gededitgui_h_elementeverschieben.htm,
  gededitgui_h_fangpunkte.htm, gededitgui_h_kommandozeile.htm, gededitgui_h_konstruktionsmodus.htm,
  gededitgui_h_koordinatenbeimzeichnen.htm, gededitgui_h_kreisezeichnen.htm, gededitgui_h_lotrechttangentialzeichnen.htm,
  gededitgui_h_orthogonalfunktion.htm, gededitgui_h_schrittweitenfestlegen.htm, gededitgui_h_texte.htm,
  gededitgui_h_texteanzeigen.htm, gededitgui_h_textebearbeiten.htm, gededitgui_h_unsichtbareelemente.htm,
  gededitgui_h_zoom.htm, gededitgui_r_bilddatei.htm, gededitgui_r_hyperlink.htm.
- Einstellungen: gedviewer_d_einstellungenprojektallgemein.htm, gedviewer_d_einstellungenverbindungssymbole.htm,
  gedviewer_d_einstellungenbenutzerallgemein.htm, gedviewer_d_einstellungenmakrosaktualisieren.htm,
  gedviewer_d_einstelleigenschaftendialoge.htm; überflogen: gedviewer_d_einstellkommandozeile.htm,
  gedviewer_d_einstellungenbenutzerallgemein3d.htm, gedviewer_d_messergebnis.htm.
- Verbindungssymbole: egedgui_k_start.htm, egedgui_k_grundlagen.htm, egedgui_k_darstellungverbindungspunkte.htm,
  egedgui_k_verteiler.htm, egedgui_k_sammelschienen.htm, egedgui_h_verbindungsstueckeeinfuegen.htm,
  egedgui_h_verbindungsstueckeaustauschen.htm, egedgui_h_verbindungenunterbrechen.htm,
  egedgui_h_dynverbindungssymboleeinfuegen.htm, egedgui_h_sammelschieneneinfuegen.htm,
  egedgui_h_knotenalspunktoderzieldarstellen.htm, egedgui_h_einstellungenanzeigeverdrahtung.htm,
  egedgui_h_bmkzusammenfuehren.htm, egedgui_h_funktionstexteabgleichen.htm, egedgui_d_bruecke.htm, egedgui_d_tstueck.htm,
  egedgui_d_kreuzung.htm, egedgui_d_geraeteinfuegen.htm, egedgui_d_platzieren.htm, egedgui_d_funktionstextabgleich.htm.
- Gerätekästen: blackbox_k_start.htm, blackbox_k_arbeitsweise.htm, blackbox_k_einsatzmoeglichkeiten.htm,
  blackbox_k_schachteln.htm, blackbox_k_anschluesseauswerten.htm, blackbox_h_kasteneinfuegen.htm,
  blackbox_h_anschluesseeinfuegen.htm, blackbox_h_polygonalekaesten.htm.
- Strukturkästen: locationbox_k_start.htm, locationbox_k_grundlagen.htm, locationbox_h_einfuegen.htm,
  locationbox_h_geschachtelte.htm.
- Blockeigenschaften: blockproperties_k_start.htm, blockproperties_k_prinzip.htm, blockproperties_k_vordefinition.htm,
  blockproperties_h_blockeigenschaftendefinieren.htm, blockproperties_h_blockeigenschaftenanzeigen.htm.
- Direktes Bearbeiten: inplaceeditinggui_k_start.htm, inplaceeditinggui_k_eigenschaften.htm,
  inplaceeditinggui_h_direktesbearbeiten.htm.
- Blockbearbeitung: multipleselection_k_start.htm, multipleselection_k_prinzip.htm,
  multipleselection_h_objekteblockbearbeitung.htm.
- Einpolig: singlepole_k_start.htm, singlepole_k_hintergrund.htm, singlepole_k_besonderheitenkabel.htm,
  singlepole_k_besonderheitenklemmen.htm, singlepole_k_sps.htm, singlepole_k_bustopologien.htm,
  singlepole_k_spinnenplaeneundanlagenuebersichten.htm, singlepole_k_straenge_in_einpoligerdarstellung.htm,
  singlepole_k_verbindungen.htm, singlepole_h_stromlaufplaenezeichnen.htm, singlepole_h_allpoligeverbindungendefinieren.htm,
  singlepole_h_gostnorm.htm, singlepole_h_stranganschluesse_vom_kabel_uebernehmen.htm.
- Ebenen: layermanager_k_start.htm, layermanager_k_prinzip.htm, layermanager_h_ebenenbearbeiten.htm,
  layermanager_d_ebenenverwaltung.htm.
- Bemaßung: dimensiongui_k_start.htm, dimensiongui_k_bemassungenprinzip.htm, dimensiongui_d_projektbemassung.htm,
  dimensiongui_h_bemassungbearbeiten.htm, dimensiongui_h_bemassungmitmittelpunkten.htm,
  dimensiongui_h_bezugspunktbemassung.htm, dimensiongui_h_einzelbemassung.htm, dimensiongui_h_kettenbemassung.htm,
  dimensiongui_h_kreisbemassung.htm, dimensiongui_h_steigendebemassung.htm, dimensiongui_h_winkelbemassung.htm,
  dimensiongui_r_bemassung.htm, dimensiongui_r_linien.htm.
- Redlining: redlining_k_start.htm, redlining_k_vorgehensweise.htm, redlining_k_projektkommentator.htm,
  redlining_h_freigabe.htm, redlining_h_pdfkommentare.htm, redlining_h_pdfkommentareeinlesen.htm,
  redlining_h_permailversenden.htm, redlining_h_projektkommentarebearbeiten.htm, redlining_h_projektkommentareloeschen.htm,
  redlining_h_zusammenfuehren.htm.
- Ablaufdiagramme: functionaldiagramgui_k_start.htm, functionaldiagramgui_k_prinzip.htm,
  functionaldiagramgui_k_funktionsdiagramme.htm, functionaldiagramgui_k_grafcet.htm, functionaldiagramgui_d_einstellungen.htm,
  functionaldiagramgui_h_funktionsdiagerstellen.htm, functionaldiagramgui_h_grafcetdiagerstellen.htm.
- Bearbeitungsbereiche: workingsection_k_start.htm, workingsection_k_prinzip.htm, workingsection_h_bereichedefinieren.htm,
  workingsection_h_adminbereichedefinieren.htm.
- Bildexport: gedgraphicexportgui_k_start.htm, gedgraphicexportgui_h_seitenexportieren.htm,
  gedgraphicexportgui_d_exporteinstellungen.htm.
