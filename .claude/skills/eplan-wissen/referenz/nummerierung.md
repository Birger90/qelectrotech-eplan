# EPLAN: Nummerierung – BMK online/offline, Verbindungen (Drahtnummern), Klemmen/Stecker, BMK-Prüfung

Wie EPLAN Betriebsmittelkennzeichen automatisch vergibt (online beim Einfügen, offline als Lauf), wie
Verbindungsbezeichnungen (Drahtnummern) über Verbindungsdefinitionspunkte (VDP), Formatgruppen, Umfang und Zähler
entstehen, wie Klemmen und Steckerkontakte nummeriert werden und wie die BMK-Syntaxprüfung arbeitet. Grundlage:
EPLAN-Plattform-Hilfe 2.9, in eigenen Worten. URL-Präfix aller Quellen, sofern nicht anders:
`https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/` (genannt wird nur der Dateiname).
Ergänzt `kennzeichnung.md` Abschnitt 11. „Vermutung“ = nicht durch die Hilfe belegt.

## Inhalt

1. BMK-Online-Nummerierung (beim Einfügen)
2. Online-Format: Formatelemente und Regeln
3. Einfügemodus bei Makros und Kopien
4. Kennbuchstabensätze und Unterklassen
5. BMK-Offline-Nummerierung (Lauf)
6. BMK-Syntaxprüfung
7. Verbindungsnummerierung: Prinzip und Ablauf
8. Verbindungsnummerierung: Filter, Platzierung, Anzeige
9. Formatgruppen, Umfang, Formatelemente
10. Zähler, Unterzähler, Rücksetzbereich, Mehrfachbezeichnungen
11. Bezeichnen, Überschreiben, Löschen, „manuell gesetzt“
12. Klemmen- und Steckerkontaktnummerierung
13. Bedeutung für QET (Zusammenfassung)
14. Ergänzung: Verbindungsbezeichnungen auf Umfang übertragen
15. Quellen

---

## 1. BMK-Online-Nummerierung (beim Einfügen)

- Neue Betriebsmittel (Einfügen > Symbol, „Neu“ im Navigator), kopierte und aus Makros eingefügte bekommen beim Einfügen
  ein BMK; der Zähler wird hochgezählt. Abschaltbar gesamt (Modus) oder je Betriebsmittelgruppe (Kontrollkästchen im
  Format). Ausgeschlossene Gruppen erhalten Kennbuchstabe + Zähler, aber ohne Hochzählen. Quelle: numerationgui_k_start.htm,
  numerationgui_h_einstellungentreffen.htm, numerationgui_h_online.htm
- Einstellungen (Projekt > Betriebsmittel > Nummerierung (online)): **Beim Einfügen von Symbolen** = Leerfeld (kein BMK) /
  Nummerieren / Nummerieren mit Merker „?“ (Fragezeichen vor dem Kennbuchstaben, später gezielt nachnummerierbar) –
  jeweils mit eigenem Nummerierungsformat. **Beim Einfügen von Makros und Kopien** = Nicht verändern / Nummerieren /
  Nummerieren mit Merker / Mit Abfrage. Außerdem aktueller Kennbuchstabensatz und Schema der Kennbuchstaben-Unterklassen.
  Quelle: numerationgui_d_einstellnummerierung.htm, numerationgui_h_ohnebmk.htm
- Funktionen: eingefügte werden nummeriert, bereits platzierte bleiben, nicht-platzierte werden berücksichtigt (belegen
  Nummern), zusammengehörige bleiben zusammen, **freie Funktionen** (nur über Artikel existent) bekommen ihre Bezeichnung
  bevorzugt. Quelle: numerationgui_k_verhaltenfunktionen.htm
- Im Navigator mit markiertem Strukturkennzeichen wird das BMK mit diesen Strukturkennzeichen vorgeschlagen; auf
  Projektebene ohne. Ohne BMK erzeugte Funktionen landen im Navigator unter „Ohne BMK“. Quelle: numerationgui_h_online.htm,
  numerationgui_h_ohnebmk.htm
- BMK-Übernahme (Abschnitt 6 in `betriebsmittel.md`) und Anschlussbezeichnungsübernahme setzen Online-Nummerierung voraus;
  Online-Nummerierung aus = keine BMK-Übernahme. Quelle: devicetaggui_h_bmkuebernahmefestlegen.htm

## 2. Online-Format: Formatelemente und Regeln

- Formatelemente: **Kennbuchstabe (I), Seite (S), Spalte (P), Zeile (R), Zähler (C), Trennzeichen**; je Element
  Mindestanzahl Stellen und Füllzeichen. Standardformat = Kennbuchstabe + Zähler. Optionen: **Vorziffer beibehalten**,
  **Buchstaben-Bestandteile des Zählers beibehalten** (nur der numerische Teil zählt hoch). Quelle: numerationgui_d_bmkformat.htm,
  numerationgui_h_onlineformat.htm
- Regeln: Kennbuchstabe Pflicht; kein Element darf einen Punkt enthalten (Punkt = Unterebene); Zähler am Ende (damit
  EPLAN vorhandene Zähler erkennt); Seite/Spalte/Zeile vor dem Zähler, zwischen Kennbuchstabe und Zähler mit
  Mindeststellenzahl; Trennzeichen nur zwischen Kennbuchstabe und Zähler; Punkt, Minus oder Struktur-Vorzeichen als
  Trennzeichen → EPLAN setzt runde Klammern (Beispiel Sicherung `-2F(.4.)3` für Seite 2, Spalte 4, Zähler 3;
  Klammern ausblendbar über Betriebsmittel > BMK „Klammern im BMK ausblenden“). Quelle: numerationgui_k_format.htm,
  numerationgui_d_bmkformat.htm
- Beispiele der Hilfe: `-2M3` (Seite 2 vor M), `MTR23` (Seite 2 nach MTR), `M0023` (Seite 002), `-2M4` (Spalte 4 nach M),
  `M0025` (Seite 002, Spalte 5), `-M052` (Spalte 05, Zähler 2). Spalte/Zeile ist je nach Lage/Drehung der Seite
  Pfad oder Position. Quelle: numerationgui_k_format.htm
- Betriebsmittelgruppen (Registerkarte BMK): Allg. Betriebsmittel, Klemmenleisten, Stecker, Gerätekästen, SPS, Kabel,
  Fluid-BM, Fluid-Leitungsverteiler, Mechanik-BM; Option „Geschachtelte Schaltzeichen nummerieren“. Registerkarte
  **Bezeichnungen**: Klemmen, Steckerkontakte, Geräteanschlüsse, SPS-Anschlüsse, Sammelschienenanschlüsse, SPS-Adressen,
  **Klemmen-Sortierkennungen** (beim Einfügen aus Makro/Kopie: höchste Sortierkennung der Leiste + 1 → neue Klemmen ans
  Ende, zusammenhängende bleiben zusammen; aus → Einsortierung nach Kennung/Bezeichnung, Mehrstockklemmen können
  getrennt werden), PE/N/SH-Klemmen nicht verändern. Quelle: numerationgui_d_nummerierungsformate.htm
- Vorzeichen `-` je Betriebsmittelgruppe ein/aus (Betriebsmittel > BMK). Quelle: numerationgui_h_vorzeichensetzen.htm

## 3. Einfügemodus bei Makros und Kopien

- **Nicht verändern**: sichtbares BMK bleibt, Zähler zählt nicht; das vollständige BMK übernimmt die Strukturkennzeichen
  der **Zielseite**, wenn das sichtbare BMK keine eigenen enthält (`-F1` bleibt `-F1`). Doppelte Hauptfunktionen mit
  gleichem BMK bzw. Funktionen mit gleichem BMK und gleicher Anschlussbezeichnung erscheinen in der Meldungsverwaltung.
- **Nummerieren**: erster freier Zähler des Kennbuchstabens (`-F1,-F2,-F3` → `-F23,-F24,-F25`).
- **Nummerieren mit Merker**: wie oben mit `?` (`-?F23`).
- **Bevorzugt Vorziffern nummerieren** (nur mit „Vorziffer beibehalten“): Kopien von `M1` → `1M1, 2M1, 3M1 …` statt
  `M1, M2, M3 …`.
- Bei „Mit Abfrage“ erscheint der Dialog bei jedem Einfügen; Mehrfach-Einfügen behält den gewählten Modus bis Esc.
  Quelle: numerationgui_d_einfuegemodus.htm, numerationgui_h_einfuegemoduswaehlen.htm, numerationgui_h_insertdtviamacros.htm,
  numerationgui_h_kopierenimged.htm

**Bedeutung für QET:** Für den Nachbau ist „Nicht verändern“ das Vorbild: Makros/Kopien mit Original-BMK einfügen,
Strukturkennzeichen kommen vom Ziel-Folio. Der QET-Import von EPLAN-Makros darf BMK nicht automatisch hochzählen.

## 4. Kennbuchstabensätze und Unterklassen

- Kennbuchstabensatz = Zuordnung Kennbuchstabe ↔ Funktion (Kategorie/Gruppe/Funktionsdefinition), normabhängig (IEC,
  IEC 61346, IEC 81346, NFPA, benutzerdefiniert); Beispiele: Drehstrommotor M (NFPA MTR); SPS-Anschluss A (IEC) bzw. K
  (61346/81346), PLC (NFPA); Behälter B/C/U. Mehrbuchstabige Kennbuchstaben stehen vor dem Zähler (`-MTR2`).
  Übersicht: Dienstprogramme > Stammdaten > Kennbuchstabe. Quelle: numerationgui_k_format.htm, numerationgui_d_einstellnummerierung.htm
- **Kennbuchstaben-Unterklassen**: benutzerdefiniertes Schema mit Unterklasse (z. B. zweistellig) und „Aufgabe der
  Unterklasse“ (übersetzbar); Auswahl im Feld Sichtbares BMK erzeugt ein neues BMK (alter Kennbuchstabe verworfen);
  optional Aufgabe als Funktionstext der Hauptfunktion übernehmen (überschreibt). Quelle: numerationgui_d_kennbuchstabenunterklassen.htm,
  numerationgui_d_kennbuchstabenunterklassenauswaehlen.htm

## 5. BMK-Offline-Nummerierung (Lauf)

- Projektdaten > Betriebsmittel > Nummerieren: ganzes Projekt oder Auswahl, platzierte und nicht-platzierte; Meldung bei
  logischen Unstimmigkeiten. Quelle: offlinenumerationgui_k_start.htm, offlinenumerationgui_h_formatfestlegen.htm
- **Umfang** je Auswahlort: Projekt (Seiten-Navigator) → alle Funktionen; Seite(n) → alle platzierten Hauptfunktionen dieser
  Seiten + zugehörige platzierte Nebenfunktionen auf **anderen** Seiten + nicht-platzierte Nebenfunktionen; markierte
  Schaltzeichen → ebenso; Navigator-Auswahl/Strukturkennzeichen → alle gewählten Funktionen mit allen Nebenfunktionen.
  Quelle: offlinenumerationgui_k_umfang.htm
- Dialog: Filter (nur über Hauptfunktionen), Tabelle je **Kennbuchstabe** mit Format, Nummerieren, „Nur gekennzeichnete
  nummerieren“ (`?`), Startwert (ab 0), Schrittweite (ab 1), **Gruppe** (Kennbuchstabe als Präfix: `K` erfasst auch `KT`,
  `KA`; Beispiel K1, K1, KT1, KA1 → K1, K2, KT3, KA4). Ergebnisvorschau mit editierbarem neuen BMK, „Ursprüngliches BMK
  verwenden“, Fehlerspalte. Quelle: offlinenumerationgui_d_nummerieren.htm, offlinenumerationgui_d_ergebnisvorschau.htm
- Offline-Formatelemente: Seite S, Spalte P, Zeile R, Kennbuchstabe I, **CL** zeilenweise fortlaufend (seitenübergreifend),
  **CC** spaltenweise fortlaufend, **SL** je Seite zeilenweise neu, **Sc** je Seite spaltenweise neu, Trennzeichen
  (Leerzeichen-Anzahl oder beliebige Zeichen). Optionen Vorziffer / Zähler (ohne Unterzähler) / Buchstabenteil des
  Zählers beibehalten. Zählerdialog: Mindeststellen + Füllzeichen (`0001`), **Zähler ist ein Unterzähler** (startet neu,
  wenn sich der Rest ändert), **alphanumerischer Zähler** mit Zeichenvorrat (z. B. „AKS“: A, K, S, AA, AK … wie
  Excel-Spalten). Seitenbezogene Formate folgen der **Seitensortierung**; reine 3D-Platzierungen nur ohne Element Seite.
  Quelle: offlinenumerationgui_d_nummerierungsformate.htm, offlinenumerationgui_d_zaehler.htm, offlinenumerationgui_d_trennzeichen.htm,
  offlinenumerationgui_h_formatbearbeiten.htm
- Nicht-platzierte Hauptfunktion (Klemmenleisten-/Steckerdefinition): Seite/Spalte/Zeile kommen von der **grafisch ersten
  platzierten** Klemme bzw. dem ersten Steckerkontakt. Quelle: offlinenumerationgui_k_formatseite.htm
- Betroffen: Übersichts- und einpolige Funktionen, Artikelplatzierungen, Klemmenleisten, Stecker, Kabel. **Nicht**: Potenziale,
  Abbruchstellen, Verbindungen, Kabelverbindungen, automatisch erzeugte Kabel, grafische und externe Funktionen; ist die
  Hauptfunktion grafisch/extern, wird das ganze Betriebsmittel nicht nummeriert. Quelle: offlinenumerationgui_k_funktion.htm
- **Geschachtelte BMK**: nur das übergeordnete wird nummeriert, untergeordnete folgen (Beispiel Start 10, Schritt 10:
  `-U1` → `-U10`, darin `-U1-K9` → `-U10-K9`, `-U1-A9-X2` → `-U10-A9-X2`, `-U2-K9` → `-U20-K9`).
  Quelle: offlinenumerationgui_k_geschachteltebmk.htm

**Bedeutung für QET:** Seitenbezogene BMK im Original (`-2M3`, `-K381`) sind Ergebnisse dieser Formate: Seite/Spalte
geben die Lage zum Zeitpunkt der Nummerierung wieder – nach späterem Verschieben stimmen sie oft nicht mehr. Nie aus
der Position „korrigieren“; BMK des Originals sind Daten. Ein `?` vor dem Kennbuchstaben im PDF ist ein Merker für
„noch nachnummerieren“ und gehört trotzdem 1:1 ins label.

## 6. BMK-Syntaxprüfung

- Prüft Länge und gültige Zeichen; auch beim Eingeben in der Strukturkennzeichen-Verwaltung; abschaltbar; je
  Kennzeichnungsblöcke, BMK-Bestandteile und Zählnummer eigene Einstellungen und zulässige Sonderzeichen. Einstellungen:
  Projekt > Betriebsmittel > BMK-Syntaxüberprüfung, Registerkarten **Strukturkennzeichen** und **Elektrotechnik/Fluid**.
  Quelle: devicetagcheckgui_k_start.htm, devicetagcheckgui_h_bmueberpruefen.htm
- Kennzeichnungsblöcke: nur Zahlen / Großbuchstaben+Zahlen / Groß-, Kleinbuchstaben+Zahlen, zulässige Sonderzeichen,
  Textsonderzeichen (z. B. Umlaute), „Bei Anlagennummern nur Zahlen“. Ist die Prüfung in Elektrotechnik/Fluid aktiv, wird
  das **ganze vollständige BMK** geprüft, die Blöcke nach den Strukturkennzeichen-Einstellungen (auch wenn dort aus).
  Quelle: devicetagcheckgui_d_einstellungenprojektbmsyntaxueberpruefung.htm
- Fehlerdialog hebt fehlerhafte Teile hervor; [Ignorieren] übernimmt trotzdem. Quelle: devicetagcheckgui_d_syntaxfehlermeldung.htm
- Siehe auch Prüflauf P017011 (unnötig lange sichtbare BMK) in `kennzeichnung.md` Abschnitt 7.

## 7. Verbindungsnummerierung: Prinzip und Ablauf

- Verbindungsbezeichnungen (Drahtnummern) werden **offline** auf Basis der Verbindungsdaten vergeben; vor jedem Lauf
  erzeugt EPLAN die Verbindungen neu. Zwei getrennte Schritte: **1. VDP platzieren**, **2. Bezeichnen** (schreibt die
  Bezeichnung an die VDP). Manuell vergebene und automatische Bezeichnungen können gemischt werden; Verbindungen lassen
  sich ausschließen; Bezeichnungen frei positionierbar; Buchstaben und Sonderzeichen erlaubt; Ausgabe auch als Liste/
  Formular (Beschriftungsautomaten, Verbindungslisten). Grundsätzlich **potenzialorientiert** oder **verbindungsorientiert**.
  Quelle: wirenumberinggui_k_start.htm, wirenumberinggui_k_prinzip.htm
- Menü Projektdaten > Verbindungen > Nummerierung: Platzieren, Bezeichnen, Neu ausrichten und formatieren, Löschen;
  Einstellungen als **Schema** (Registerkarten Filter, Platzierung, Bezeichnung, Anzeige). Quelle: wirenumberinggui_h_einstellungen.htm,
  wirenumberinggui_d_verbnumeinstellungen.htm
- Bezeichnet werden kann nur, wenn Quelle **und** Ziel der Verbindung in der Auswahl sind. Grafische und externe
  Verbindungen werden nie bezeichnet. Quelle: wirenumberinggui_h_verbnumbezeichnen.htm, devicetaggui_k_grafischeschaltzeichen.htm

## 8. Verbindungsnummerierung: Filter, Platzierung, Anzeige

- **Filter**: Gewerke und Verbindungs-Funktionsdefinitionen; „Ader/Draht“ aufgeteilt in **Ader** (allpolig im Kabel),
  **Kabel** (einpolig im Kabel), **Draht** (allpolig außerhalb Kabel), **Drahtbündel** (einpolig außerhalb). Quelle: wirenumberinggui_r_filter.htm
- **Platzierung** (Anzahl): auf jedes einzeln verlaufende Teilstück (zentriert) / **einmal pro Verbindung** (grafisch
  erstes Teilstück) / einmal pro Seite (für seitenübergreifende Verbindungen auf jeder Seite) / an Anfang und Ende (nahe
  Quelle/Ziel). „Grafisch erstes“ folgt der **Auswertungsrichtung des Normblatts** (senkrecht/IEC: das am weitesten links
  liegende), unabhängig von Quelle/Ziel. **Auf Teilstücken, die mehrere Verbindungen tragen** (gemeinsame
  Autoconnect-Linie, T-Abzweig-Strecke) **wird kein VDP gesetzt** – die Verbindungen haben trotzdem eine Bezeichnung.
  Option „Platzierungen zueinander ausrichten“ (parallele Teilstücke mit gleichem Abstand und gemeinsamem
  Koordinatenbereich). Eigenes Symbol für den VDP wählbar. Quelle: wirenumberinggui_r_platzierung.htm, wirenumberinggui_k_platzierungvdp.htm
- Automatisch gesetzte VDP: „Erlaubte Daten = Nur Verbindungsbezeichnung“, alle anderen Eigenschaften neutral/leer;
  Bezeichnung `????`, solange die Verbindung unbezeichnet ist. Vorhandener VDP, der eine Bezeichnung tragen kann, wird
  wiederverwendet (kein zweiter). Ein VDP „keine Verbindungsbezeichnung“ genau an der Zielposition wird auf „alle Daten“
  umgestellt (mit Hinweis). Quelle: wirenumberinggui_k_platzierungvdp.htm, wirenumberinggui_h_vdpplatzieren.htm
- **Anzeige**: Abstand horizontal (zu senkrechten Teilstücken) / vertikal (zu waagerechten), Format, Ausrichtung
  1–12 (oben/mitte/basis/unten × links/zentriert/rechts) plus EPLAN-5-Spezialausrichtungen. Die Bezeichnung „steht fest
  an der Verbindungslinie“. Neu ausrichten und formatieren setzt verschobene VDP auf Ursprungsposition/-format zurück
  (wahlweise alle / nur reine Bezeichnungs-VDP / alle bezeichnungsfähigen; „manuell gesetzte“ beibehalten).
  Quelle: wirenumberinggui_r_anzeige.htm, wirenumberinggui_h_verbnumausrichtenformatieren.htm, wirenumberinggui_d_verbnumausrichtenformatieren.htm

**Bedeutung für QET:** Im EPLAN-PDF steht die Drahtnummer **zentriert auf einem einzeln verlaufenden Teilstück**, bei
senkrechten Leitungen seitlich (horizontaler Abstand), bei waagerechten darüber (vertikaler Abstand). Auf gemeinsam
genutzten Strecken steht keine Nummer. Für QET: Drahtnummer als Leitertext (conductor `num`) an genau der Leitung, auf deren
Teilstück der Text im Original steht; Position QET-seitig über Textausrichtung am Leiter. Leitungsabschnitte ohne Text
haben trotzdem die Nummer ihres Potenzials/Netzes (bei potenzialorientierter Nummerierung) – Verknüpfungsskript soll die
Nummer über verbundene Leiter (gleiches Netz/Potenzial) weitergeben, wo der Umfang „Netz“/„Potenzial“ erkennbar ist.

## 9. Formatgruppen, Umfang, Formatelemente

- **Verbindungsgruppen** (vordefiniert u. a.): Allgemeine Verbindungen; an SPS-Anschlüsse angeschlossene (ohne
  Kartenversorgung/Bus-Kabel); an „SPS-Anschluss E/A, 1 Anschluss“ oder „variabel“ angeschlossene (deaktivierte nicht,
  Multifunktion nur mit Signalart E/A); an Klemmen, an Stecker …; außerdem **Betriebsmittel…** (alle Verbindungen an
  einem bestimmten BMK) und **Gruppierung…** (Wert der Eigenschaft Gruppierung an Verbindung, VDP, Potenzial, Signal,
  PDP). Quelle: wirenumberinggui_d_verbnumformate.htm, wirenumberinggui_k_prinzip.htm, wirenumberinggui_d_betriebsmittelliste.htm,
  wirenumberinggui_h_formatgruppenerstellen.htm
- **Reihenfolge der Formatgruppen zählt**: Gruppen werden der Reihe nach nummeriert; „Allgemeine Verbindungen“ muss
  **zuletzt** stehen, sonst greifen die speziellen Formate (SPS, Klemmen) nie. Quelle: wirenumberinggui_r_bezeichnung.htm
- **Umfang** = Menge mit gleicher Bezeichnung: Einzelverbindung, **Netz**, **Signal**, **Potenzial/Rohrleitung**, für SPS
  zusätzlich „Bis zu jedem Aktor/Sensor (Netz)“. Bei „Netz“ tragen alle Verbindungen eines Netzes dieselbe Bezeichnung
  (außer mit Unterzähler). Quelle: wirenumberinggui_k_prinzip.htm
- **Netz** = Teilmenge eines Signals; endet normalerweise an Klemmen, Steckerkontakten, Funktionsanschlüssen. Die
  **Netzerweiterung** (Liste von Funktionen, über die das Netz weiterläuft, z. B. Klemmen) gilt für alle Formatgruppen mit
  Umfang „Netz“ und ist **unabhängig von der Eigenschaft „Netzverbindend“**. Quelle: wirenumberinggui_h_netzerweiterung.htm,
  wirenumberinggui_d_netzerweiterung.htm
- „Bis zu jedem Aktor/Sensor“: Suche ab SPS-Anschluss DE/AE (→ Sensor) bzw. DA/AA (→ Aktor), bei variablen Anschlüssen
  nach Signalart; unterwegs wird an jedem Anschluss die Signalart geprüft (z. B. Anschluss „Digitaleingang“ gilt als Aktor,
  „Digitalausgang“ als Sensor, Anschlussversorgung als Sensor, Geräteversorgung als Aktor); ohne passende Signalart geht
  die Suche über „Potenzial weiterleiten nach“ weiter, sonst Ende. Quelle: wirenumberinggui_k_prinzip.htm
- **Formatelemente**: Daten der Verbindung (W), der Quellfunktion (B), der Zielfunktion (E), der Seite der Quelle (Pb)
  bzw. des Ziels (Pe), des Potenzials (V), der Rohrleitung (V), des Objekts der Verbindungsgruppe (O), Seite (S, ohne
  Unterzähler der Seite), Spalte (P), Zeile (R) – jeweils **von Quelle oder von Ziel** –, Zähler (C), Unterzähler (Cs),
  Trennzeichen (Leerzeichen, Zeilenumbruch, andere; `#`, `<`, `>` nur mit `\`). Eigenschaften mit Mindeststellen/Füllzeichen,
  „leere/Null unterdrücken“ (optional inkl. Trennzeichen davor/danach), Sprache, „Werte von Einzelverbindungen anzeigen“
  (sonst Wert der ersten Verbindung des Umfangs für den ganzen Umfang). Quelle: wirenumberinggui_d_verbnumformate.htm,
  wirenumberinggui_d_formateigenschaften.htm, wirenumberinggui_d_formatseite.htm, wirenumberinggui_d_formattrennzeichen.htm
- Optionen am Format: **Klemmenbezeichnungen überschreiben**, **Abbruchstellenbezeichnungen überschreiben**,
  **Stranganschlussbezeichnungen überschreiben** – jeweils mit der Bezeichnung der ersten angeschlossenen Verbindung.
  Quelle: wirenumberinggui_d_verbnumformate.htm

**Bedeutung für QET:** Typische EPLAN-Drahtnummern sind daher z. B. `Seite.Spalte` (Quelle) + Zähler, Potenzialname +
Unterzähler oder BMK:Anschluss der Quelle. Beim Nachbau Format aus dem Original **ablesen, nicht neu erzeugen**. Wenn
Klemmenbezeichnungen = Drahtnummern sind (Option „Klemmenbezeichnungen überschreiben“), stammt die Klemmennummer von der
ersten angeschlossenen Verbindung – im Nachbau trotzdem den Wert aus dem Klemmenplan übernehmen. Abbruchstellen mit
Drahtnummer als Name: dieselbe Logik.

## 10. Zähler, Unterzähler, Rücksetzbereich, Mehrfachbezeichnungen

- Zähler: **Reserve/Seite** (letzter Zähler 230, Reserve 100 → nächste Seite beginnt bei 300; Reserve 50 → 250),
  Mindeststellen/Füllzeichen, **Richtung** zeilenweise (links→rechts) / spaltenweise (oben→unten) / nach Normblattlogik,
  **Geräteweise nummerieren** (Verbindungen zwischen zwei Schaltzeichen in Planreihenfolge, auch über BMK-Übernahme
  verbundene; nur sinnvoll bei Umfang Einzelverbindung), alphanumerischer Zähler, **Zurücksetzen pro Struktur/Seite**
  (Rücksetzbereich: je Kennzeichnungsblock bzw. Unterkennzeichen, Seite, Unterseite – bezogen auf Quelle oder Ziel),
  **Zähler ist ein Unterzähler** mit eigenem Umfang (Einzelverbindung, Netz …), **Ersten Zähler/Unterzähler unterdrücken**
  (Beispiel Potenzial + Unterzähler je Einzelverbindung: `1. / 1.2 / 1.3`, mit „inklusive vorheriges Trennzeichen“
  `1 / 1.2 / 1.3`). Quelle: wirenumberinggui_d_formatzaehler.htm, wirenumberinggui_d_zaehlerruecksetzbereich.htm
- Unterzähler ist innerhalb der ganzen Formatgruppe eindeutig (bei sonst gleichen Bezeichnungen automatisch hochgezählt).
  Quelle: wirenumberinggui_k_prinzip.htm
- Bezeichnen-Dialog: Startwert/Schrittweite je Verbindungsgruppe (Schrittweite 0 erlaubt), „Gruppe“ = gemeinsame Werte für
  alle Zähler der Gruppe. **Mehrfachbezeichnungen vermeiden**: keine / in der Auswahl / im gesamten Projekt / pro
  Zähler-Rücksetzbereich (wirkt nur auf Zähler, nicht Unterzähler). Beim Vermeiden zählt EPLAN Zähler hoch, wenn der
  Unterzähler auf dem Startwert steht, sonst den Unterzähler (Beispiel vorhanden `001.1`, `002.2` → neu `002.1`, `002.3`).
  Innerhalb eines vollständig gewählten Umfangs sind gleiche Bezeichnungen gewollt (Seite + Unterzähler je Signal →
  alle `1-1`). Quelle: wirenumberinggui_d_verbnumbezeichnen.htm

## 11. Bezeichnen, Überschreiben, Löschen, „manuell gesetzt“

- **Überschreiben**: alle / außer „manuell gesetzte“ / keine (nur leere oder mit `?` bezeichnete werden neu vergeben).
  Bei „außer manuell“ und „keine“ übernehmen hinzukommende Verbindungen die Bezeichnung ihres Umfangs; bei mehreren
  vorhandenen gilt die **grafisch erste**. So behalten Verbindungen desselben Signals beim Nachnummerieren dieselbe Nummer.
  **Sichtbarkeit**: nicht verändern / alle sichtbar / **ein Mal pro Seite und Umfang** (nur an der grafisch ersten Verbindung
  sichtbar). „Als manuell gesetzt markieren“. Ergebnisvorschau: Quelle, Ziel (vollständiges BMK + Anschluss), alte/neue
  Bezeichnung, „Alte Verbindungsbezeichnungen beibehalten“. Quelle: wirenumberinggui_d_verbnumbezeichnen.htm,
  wirenumberinggui_d_ergebnisbezeichnen.htm, wirenumberinggui_h_verbnumbezeichnen.htm
- Ein manuell gesetzter VDP mit „Manuell gesetzt“ markiert die **ganze Verbindung**; alle übrigen Verbindungseigenschaften
  (auch die Bezeichnung) kommen vom **grafisch ersten VDP**. Quelle: wirenumberinggui_k_platzierungvdp.htm
- **Löschen**: nur Bezeichnungen / leere VDP (nur-Bezeichnung, leer oder `????`) / beides; „manuell gesetzte“ beibehalten.
  Quelle: wirenumberinggui_d_verbindloeschen.htm, wirenumberinggui_h_verbindloeschen.htm

**Bedeutung für QET:** Steht im PDF dieselbe Drahtnummer nur einmal pro Seite, obwohl mehrere Leiter dazu gehören, war
vermutlich „Ein Mal pro Seite und Umfang“ aktiv – die anderen Leiter tragen dieselbe Nummer unsichtbar. Im QET-Nachbau
allen Leitern des Umfangs die Nummer geben, aber nur den im Original sichtbaren Text anzeigen (QET: Leitertext
ausblenden). `????` im PDF = VDP ohne vergebene Nummer; übernehmen oder als offenen Punkt melden.

## 12. Klemmen- und Steckerkontaktnummerierung

- Dialog Klemmen/Steckerkontakte nummerieren (aus Klemmenleisten- bzw. Stecker-Navigator): Schema; **Reihenfolge** wie
  Sortierung / seitenorientiert (Auswertungsrichtung des Normblatts: IEC spaltenweise, NFPA zeilenweise) / kabelorientiert
  (Reihenfolge der aufgelegten Kabeladern) / etagenorientiert (nur Klemmen, Mehrstockzuordnung bleibt) / seitenorientiert
  alternativ; **Umfang** markierte / alle der gewählten Leisten / alle im Projekt; Startwert, Schrittweite (Stellenzahl aus
  dem Online-Format), Präfix/Suffix. Quelle: terminalplugnumberinggui_d_klemmennummerieren.htm
- **Potenzialgebundene** PE/PEN-, N-, SH-Klemmen (mindestens ein Anschluss dieses Potenzialtyps): nicht verändern / nummerieren /
  nicht verändern, Reihenfolge berücksichtigen (nur Klemmen). Beispiel `1, 4, PE, 8, 2` → `1, 2, PE, 3, 4` bzw. `1, 2, 3, 4, 5`
  bzw. `1, 2, PE, 4, 5`. **Mehrfachklemmen** (Gleiche Bezeichnung erlaubt): nicht verändern / gleich / einzeln nummerieren.
  **Alphabetisch bezeichnete**: nicht verändern / Buchstabenteile beibehalten (`1, 1+, 1PE` → `2, 2+, 2PE`) / nummerieren
  (`2, 3, 4`). „Änderung der Sortierung zulassen“ (aus: leere Sortierkennungen werden vorher aus der bisherigen Bezeichnung
  gefüllt). „Nur gekennzeichnete nummerieren“ (`?` oder leer) mit „Lücken füllen“ (`1, ?, 3, 4, ?, ?, 7, 8` → 1…8; ohne:
  `1, 1, 3, 4, 2, 3, 7, 8`). „Durch SPS beeinflusste mit nummerieren“. Quelle: terminalplugnumberinggui_d_klemmennummerieren.htm
- Schema: Standardformat **nach Online-Nummerierungsformat** oder **nach Gerätedefinition** (Bezeichnungen aus den
  Funktionsschablonen), sonst feste Tabelle der n-ten Bezeichnungen (per Excel einfügbar, z. B. A001–A100). Bei Schema
  Gerätedefinition/Initiatoren bleiben Klemmen ohne freie Bezeichnung unnummeriert. Quelle: terminalplugnumberinggui_d_nummerierungsschema.htm,
  terminalplugnumberinggui_d_klemmennummerieren.htm
- Online: Klemmen, Steckerkontakte, Geräte-/SPS-Anschlüsse, Sammelschienenanschlüsse über Registerkarte Bezeichnungen des
  Online-Formats; im Navigator per Nummerierungsmuster (`1a,1b,2a-8b`). Quelle: numerationgui_d_nummerierungsformate.htm,
  navigatorgui_h_mehrerfunktionerzeugen.htm

**Bedeutung für QET:** Klemmennummern sind im Original Daten (oft nicht fortlaufend, PE/N mit Sonderbezeichnung,
Mehrfachklemmen mit gleicher Nummer). Im Nachbau exakt übernehmen; Lücken nicht schließen. Reihenfolge in der Leiste
= Sortierkennung bzw. Bezeichnung (Klemmenplan des Originals ist Referenz).

## 13. Bedeutung für QET (Zusammenfassung)

| EPLAN | QET-Nachbau |
|---|---|
| Online-/Offline-BMK-Nummerierung | im Nachbau nie nummerieren; BMK aus Original; QET-Autonummerierung aus |
| Einfügemodus „Nicht verändern“ | Makro-/Bauteil-Import ohne Hochzählen; Struktur vom Ziel-Folio |
| `?` im BMK | Merker, unverändert übernehmen |
| VDP mit Drahtnummer, zentriert auf Einzelteilstück | Leitertext an genau dieser Leitung |
| kein VDP auf gemeinsam genutzten Teilstücken | Strecken ohne Text: Nummer über Netz/Potenzial weitergeben |
| Umfang Netz/Signal/Potenzial | gleiche Nummer für alle Leiter des Umfangs |
| „Ein Mal pro Seite und Umfang“ | Nummer an allen Leitern, nur einmal sichtbar |
| Formatgruppen-Reihenfolge, Formate (Seite/Spalte, Potenzial, BMK) | Format im Original ablesen, nicht erfinden |
| Klemmen-/Steckernummerierung | Bezeichnungen aus Klemmenplan, nicht neu vergeben |

## 14. Ergänzung: Verbindungsbezeichnungen auf Umfang übertragen
Ist in **Einstellungen: Verbindungssymbole** „Verbindungsbezeichnungen auf Umfang übertragen“ aktiviert, fragt EPLAN nach dem manuellen Ändern einer Verbindungsbezeichnung (Eigenschaftendialog einer Verbindung oder eines Verbindungsdefinitionspunkts), wohin die neue Bezeichnung übertragen wird: an keine weiteren Verbindungen, an alle Verbindungen des **Potenzials**, des **Signals** oder des **Netzes**.
- Dabei gilt der Filter der Verbindungsnummerierung (Registerkarte Filter in Einstellungen: Verbindungsnummerierung).
- Die Einstellung wirkt nicht, wenn die Bezeichnungen durch die Verbindungsnummerierung selbst geändert werden.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/wirenumberinggui_d_verbbezanumfang.htm

**Bedeutung für QET**: QET überträgt einen geänderten Leitertext auf Wunsch auf alle Leiter desselben Potenzials – das entspricht der Stufe „Potenzial“. Die feineren Stufen Signal (endet an signaltrennenden Funktionen) und Netz (direkt verbundene Anschlüsse) muss der Fork bei Bedarf selbst über die Verbindungslogik abgrenzen.

## 15. Quellen

Alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/

- BMK online: numerationgui_k_start.htm, numerationgui_k_format.htm, numerationgui_k_verhaltenfunktionen.htm,
  numerationgui_h_online.htm, numerationgui_h_onlineformat.htm, numerationgui_h_einstellungentreffen.htm,
  numerationgui_h_einfuegemoduswaehlen.htm, numerationgui_h_insertdtviamacros.htm, numerationgui_h_kopierenimged.htm,
  numerationgui_h_ohnebmk.htm, numerationgui_h_vorzeichensetzen.htm, numerationgui_d_bmkformat.htm,
  numerationgui_d_einfuegemodus.htm, numerationgui_d_einstellnummerierung.htm, numerationgui_d_nummerierungsformate.htm,
  numerationgui_d_kennbuchstabenunterklassen.htm, numerationgui_d_kennbuchstabenunterklassenauswaehlen.htm
- BMK offline: offlinenumerationgui_k_start.htm, offlinenumerationgui_k_umfang.htm, offlinenumerationgui_k_funktion.htm,
  offlinenumerationgui_k_formatseite.htm, offlinenumerationgui_k_geschachteltebmk.htm, offlinenumerationgui_h_formatfestlegen.htm,
  offlinenumerationgui_h_formatbearbeiten.htm, offlinenumerationgui_d_nummerieren.htm, offlinenumerationgui_d_ergebnisvorschau.htm,
  offlinenumerationgui_d_nummerierungsformate.htm, offlinenumerationgui_d_zaehler.htm, offlinenumerationgui_d_trennzeichen.htm
- BMK-Prüfung: devicetagcheckgui_k_start.htm, devicetagcheckgui_h_bmueberpruefen.htm,
  devicetagcheckgui_d_einstellungenprojektbmsyntaxueberpruefung.htm, devicetagcheckgui_d_syntaxfehlermeldung.htm
- Verbindungsnummerierung: wirenumberinggui_k_start.htm, wirenumberinggui_k_prinzip.htm, wirenumberinggui_k_platzierungvdp.htm,
  wirenumberinggui_h_einstellungen.htm, wirenumberinggui_h_formatgruppenerstellen.htm, wirenumberinggui_h_formatgruppenbearbeiten.htm,
  wirenumberinggui_h_netzerweiterung.htm, wirenumberinggui_h_vdpplatzieren.htm, wirenumberinggui_h_verbnumbezeichnen.htm,
  wirenumberinggui_h_verbnumausrichtenformatieren.htm, wirenumberinggui_h_verbindloeschen.htm,
  wirenumberinggui_d_verbnumeinstellungen.htm, wirenumberinggui_d_verbnumformate.htm, wirenumberinggui_d_formateigenschaften.htm,
  wirenumberinggui_d_formatseite.htm, wirenumberinggui_d_formattrennzeichen.htm, wirenumberinggui_d_formatzaehler.htm,
  wirenumberinggui_d_zaehlerruecksetzbereich.htm, wirenumberinggui_d_netzerweiterung.htm, wirenumberinggui_d_betriebsmittelliste.htm,
  wirenumberinggui_d_vdpplatzieren.htm, wirenumberinggui_d_verbnumbezeichnen.htm, wirenumberinggui_d_ergebnisbezeichnen.htm,
  wirenumberinggui_d_verbnumausrichtenformatieren.htm, wirenumberinggui_d_verbindloeschen.htm, wirenumberinggui_r_filter.htm,
  wirenumberinggui_r_platzierung.htm, wirenumberinggui_r_bezeichnung.htm, wirenumberinggui_r_anzeige.htm
- Klemmen/Stecker: terminalplugnumberinggui_d_klemmennummerieren.htm, terminalplugnumberinggui_d_nummerierungsschema.htm
- Weitere (Querbezug): devicetaggui_h_bmkuebernahmefestlegen.htm, devicetaggui_k_grafischeschaltzeichen.htm,
  navigatorgui_h_mehrerfunktionerzeugen.htm

Zusatz-Dialogseiten (Ergänzung Abschnitt 14):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/wirenumberinggui_d_verbbezanumfang.htm
