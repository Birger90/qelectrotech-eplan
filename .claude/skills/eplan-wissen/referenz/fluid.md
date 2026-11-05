# EPLAN-Wissen: Fluidtechnik (Hydraulik, Pneumatik) – Symbole, Kennzeichnung, Schläuche, Rohrleitungen

Knappe Zusammenfassung der EPLAN-Plattform-Hilfe 2.9 zu EPLAN Fluid, Schlauchleitungskonfigurator (Fluid Hose
Configurator), Rohrleitungen im Bauraum (Piping) und Festo-Katalog, in eigenen Worten. Zweck: Fluidseiten aus einem
EPLAN-PDF in QElectroTech inhaltlich richtig nachbauen (Kennzeichnung nach DIN ISO 1219-2, Abbruchstellen,
Ventilspulen-Querverweise zum Elektroplan). „Vermutung“ kennzeichnet eigene Schlüsse.

## Inhalt
1. EPLAN Fluid: Umfang, Gewerke, Seitentypen
2. Kennzeichnung nach DIN ISO 1219-2 (Betriebsmittel)
3. BMK-Nummerierung (online, offline, BMK-Auswahl, tabellarisch)
4. Fluid-Verbindungen: Rohr/Schlauch, Leitungskennzeichnung, Leitungstypen und Farben
5. Verbindungssymbole, Leitungsverteiler, Ventilblöcke, Gerätekästen
6. Anschlusslogik von Fluid-Funktionen, Anschlussmaß
7. Abbruchstellen nach DIN ISO 1219-2
8. Querverweise Ventilspule ↔ Elektroplan, SPS-Funktionen auf Fluidseiten, Makros in Mischprojekten
9. Pakete, Rohr- und Schlauchleitungen als Betriebsmittel
10. Auswertungen und Prüfläufe
11. Schlauchleitungskonfigurator (DIN 20066)
12. Verlegen im Bauraum, Rohrleitungen und Rohrbiegen
13. Festo-Produktkatalog
14. Ergänzung: Gewerk beim Geräteeinfügen auswählen
15. Quellen

---

## 1. EPLAN Fluid: Umfang, Gewerke, Seitentypen

- EPLAN Fluid gibt es eigenständig oder als Add-on zu Electric P8/Pro Panel. Symbolbibliotheken für Hydraulik,
  Pneumatik, Kühlung, Kühlschmiermittel, Schmierung, Gastechnik; Abzweige/Richtungswechsel als einfache
  Verbindungssymbole oder als auswertbare Leitungsverteiler/-verbinder.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_start.htm
- **Gewerke:** Elektrotechnik, Hydraulik, Pneumatik, Kühlung, Schmierung, Kühlschmiermittel, Gastechnik,
  Fluid (undefiniert); dazu frei benennbare **Untergewerke** (z. B. Wasser-/Ölkühlung). Gewerk wird an der Funktion
  gesetzt, der zuletzt gewählte Wert ist Vorgabe; Magnetspulen aus Fluidbibliotheken bekommen automatisch
  „Elektrotechnik“. An Verbindungsdefinitionspunkten ist das Gewerk **nicht** einstellbar, es kommt von den
  angeschlossenen Funktionen. Artikel mit mehreren Gewerken → Abfrage beim Einfügen; „Fluid (undefiniert)“ lässt
  sich später filtern und tabellarisch korrigieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_gewerke_an_schaltzeichen.htm
- **Seitentyp „Schaltplan Fluid“:** nicht dreh-/spiegelbar (Normentausch). Elektrosymbole darauf (z. B. Ventilspule)
  bekommen automatisch Darstellungsart **„Paarquerverweis“**; Gerätekästen/-anschlüsse werden zu „Gerätekasten
  (Fluid)“ / „Geräteanschluss (Fluid)“. **„Funktionsübersicht (Fluid)“:** vereinfachte Übersicht mit Querverweisen
  zum Fluidplan, eigene Darstellungsart (rangiert hinter „Funktional“); ihre Verbindungen haben kein Gegenstück und
  zählen beim übergreifenden Bearbeiten nicht. Seiteneigenschaft **Gewerk (ID 11037)**.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_seitentyp_fluid.htm

**Bedeutung für QET:** QET kennt keinen Fluid-Seitentyp. Fluidblätter als normale Folios mit Fluidsymbolen
(IEC/ISO 1219-1) zeichnen; das Gewerk als Folio-Eigenschaft bzw. Elementinfo notieren (Vermutung). Die Ventilspule
auf dem Fluidblatt ist ein **Slave** (Paarquerverweis) der Spule im Elektroplan, nicht ein zweites Gerät.

## 2. Kennzeichnung nach DIN ISO 1219-2 (Betriebsmittel)

- Kennzeichnungsblöcke wie in der Elektrotechnik (== Funktionale Zuordnung, = Anlage, ++ Aufstellungsort,
  + Einbauort, **Anlagennummer** (ohne Vorzeichen), # benutzerdefiniert, - Betriebsmittel); für allgemeine
  Fluid-Betriebsmittel und für Leitungsverteiler/-verbinder getrennt festlegbar.
- Schlüssel nach DIN ISO 1219-2: **Anlage – Medienschlüssel Schaltkreisnummer . Bauteilnummer**, Beispiel `1-P3.1`.
  EPLAN bildet das so ab: Anlagenbezeichnung = Anlagennummer, „-“ fest, Medienschlüssel = **Gewerkskennung als
  Kennbuchstabe** (Projekteinstellung „Fluid: Gewerkskennung als Kennbuchstabe (Medienschlüssel) verwenden“),
  Schaltkreisnummer = BMK-**Zähler**, „.“ fest, Bauteilnummer = **Unterzähler**. P = Pneumatik (laut Beispiel).
- Anlagennummer darf bei nur einer Anlage, Medienschlüssel bei nur einem Medium entfallen; letzteres per
  „Fluid: Kennbuchstabe (Medienschlüssel) in der Grafik ausblenden“ – nur grafisch, im BMK bleibt er erhalten.
- Projektvorlage **FL_1219-2_tpl001.ept** liefert die Einstellungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_betriebsmittelstruktur.htm
- DIN ISO 1219-2 verlangt einen **rechteckigen Rahmen um das BMK**: Eigenschaft „Textrahmen zeichnen“
  (Registerkarte Anzeige) oder über die Ebenenverwaltung; Ellipse/Oval möglich, aber nicht normgerecht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_darstellungbmkanschlussb.htm
- Eigene **BMK-Syntaxüberprüfung** für Fluid (Einstellungen > Betriebsmittel > BMK-Syntaxüberprüfung, Registerkarte
  Fluid); mit Medienschlüssel als Kennbuchstabe ist die **Vorziffer verboten** – eine Ziffer vor dem Medienschlüssel
  wird als Fehler gemeldet.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_syntaxueberpruefung.htm

**Bedeutung für QET:** Fluid-BMK wie `1-P3.1` oder `H5.7` exakt übernehmen; der Rahmen um das BMK ist Normdarstellung,
kein eigenes Grafikobjekt (in QET Textfeld mit Rahmen oder Rechteck, Vermutung). Beim Verknüpfen ist das BMK ohne
Anlagennummer nicht eindeutig, wenn mehrere Anlagen existieren.

## 3. BMK-Nummerierung (online, offline, BMK-Auswahl, tabellarisch)

- **Online beim Symbol-Einfügen:** Vorschlag = Medienschlüssel (vom zuletzt eingefügten Gewerk) + Schaltkreis „0“ +
  „.“ + nächste Bauteilnummer (ab 1); Beispiel: vorhanden P0.1–P0.5 → neue P0.6, P0.7, P0.8.
- **Makros/Kopien:** Dialog Einfügemodus, Schalter **„Bevorzugt Schaltkreisnummer nummerieren“**: aus → Bauteilnummer
  hochzählen (P0.3, P0.4, P0.5), an → Schaltkreisnummer (P1.1, P2.1, P3.1).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_fluidbetriebsmittel_online_nummerieren.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_bmknummerierung.htm
- **Offline** (Projektdaten > Betriebsmittel > Nummerieren): Startwert und Schrittweite für die Bauteilnummer;
  Beispiel P1.1/P1.2/P1.3, Start 5, Schritt 3 → P1.5, P1.8, P1.11.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_fluidbetriebsmittel_offline_nummerieren.htm
- **BMK-Auswahl** [Nächste]: nächste freie Schaltkreis- oder Bauteilnummer (H1.1, H3.7, H4.5 → H5.7 bzw. H3.8);
  dazu in den Navigator-Baumeinstellungen BMK-Vorziffer/Kennbuchstabe/Zähler/Unterzähler einblenden.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_bmk_auswaehlen.htm
- **Tabellarisch:** Schema „Betriebsmittel Fluid“; Option „BMK in Spalten aufteilen“ zerlegt das BMK (ohne
  Anlagennummer) in Teil 1–5 + BMK-Vorschau, so lassen sich Anlagennummer, Schaltkreis- und Bauteilnummer spaltenweise
  ändern.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_bmk_tabellarisch_bearbeiten.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_tabellarische_bearbeitung.htm

## 4. Fluid-Verbindungen: Rohr/Schlauch, Leitungskennzeichnung, Leitungstypen und Farben

- Eine Fluid-Verbindung verbindet zwei Anschlüsse. Ob **Rohr** oder **Schlauch**, bestimmt die Funktionsdefinition
  eines Verbindungsdefinitionspunkts; ohne Punkt gilt die Vorbelegung je Gewerk (Optionen > Einstellungen >
  Projekte > … > Verbindungen > Eigenschaften, Registerkarten Hydraulik, Pneumatik … ). Werte am Punkt überschreiben
  die Vorbelegung.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verbindungen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_einstellungen_fluidverbindungen.htm
- Gewerk der Verbindung = aus Quelle und Ziel; Verbindungsnummerierung kann gewerkweise erfolgen. Fluidverbindungen
  haben normalerweise **kein BMK**, nur eine Verbindungsbezeichnung; BMK nur, wenn am Verbindungsdefinitionspunkt
  „Erlaubte Daten = Alle Daten“ (bei manuell gesetzten Punkten voreingestellt).
- **Leitungskennzeichnung DIN ISO 1219-2:** `Leitungsnummer (optional) - Ø technische Information`. Schlauch:
  Nenngröße (`2-Ø20`); Rohr: Außendurchmesser x Wanddicke (`1-Ø30x5`) oder Außen-/Innendurchmesser (`3-Ø10/6`), mm.
  Automatisch über Verbindungsnummerierung (Schemata je Gewerk, Filter).
- **Leitungstypen und Ebenen:** jede Leitungsart hat eine eigene Ebene mit Linienstärke, -typ, Farbe, Musterlänge
  (Ebenenverwaltung > Symbolgrafik > Verbindungssymbole > Autoconnecting), auch für interne Leitungen in
  Gerätekästen; Leitungen ausblendbar. Standard: **Druckleitung, orange, Ebene EPLAN507**; wird an beiden Zielen
  „Tankleitung“ gesetzt, wechselt die Verbindung auf **EPLAN521, hellblau**. Leitungstyp + Farbe folgen DIN ISO 1219-1.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verbindungen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_fluidleitungen_farblich_hervorheben.htm

**Bedeutung für QET:** Linienfarbe/-art einer Fluidleitung im PDF verrät den Leitungstyp (orange = Druck, hellblau
= Tank, Vermutung für weitere Farben nach Ebenenstandard). Im Nachbau Leiterfarbe und Strichart entsprechend setzen
und die Leitungskennzeichnung (`2-Ø20`) als Leitertext übernehmen.

## 5. Verbindungssymbole, Leitungsverteiler, Ventilblöcke, Gerätekästen

- **Rein grafisch** (keine Daten): Winkel, T-Stück, Kreuzung, Diagonale. **Funktionen mit Daten** (auswertbar,
  Stückliste): Leitungsverteiler (T-Stück, Kreuzung), Leitungsverbinder (Winkel, Gerade).
- **Ventilblöcke** = Gerätekasten; Verteiler/Verbinder im Inneren müssen Funktionen sein und als **Nebenfunktion**
  des Blocks gesetzt werden. Übergang Block ↔ außen: Geräteanschluss (zweiseitig) oder Leitungsverbinder.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verbindungssymbole.htm
- Komplexe Geräte (Ventilblock, Aggregat) als Gerätekasten (Fluid) mit Geräteanschlüssen (Fluid); Geräte mit
  Elektrokomponenten über **Gerätegruppe** (Bearbeiten > Weiteres > Gerät gruppieren), nur im Makroprojekt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_geraetekaesten_anschluesse.htm

**Bedeutung für QET:** T-Stücke ohne Daten sind nur Leiterabzweige (in QET Leiterknoten); Verteiler mit BMK sind
Elemente mit Artikel. Ventilinsel/Ventilblock als Gerätekasten-Rahmen mit Nebenfunktionen nachbauen.

## 6. Anschlusslogik von Fluid-Funktionen, Anschlussmaß

- Anschlussart: **Fluid** (Standard beim Einfügen), **Intern**, **Direktanschluss** (für Verteiler/Verbinder in
  Ventilgruppen). Nur Anschlussart Fluid + passendes Gewerk liefern Verrohrungs-/Verschlauchungslisten.
- **Druck-/Steueranschluss** (Leitungstyp): Undefiniert, Druck-, Steuer-, Lecköl-, Tank-, Verbraucherleitung,
  Prozessmedium, Heiz-/Kühlmedium. Der Typ geht vom Anschluss auf die Verbindung über, wenn die Verbindung
  „Undefiniert“ hat; Prüflauf (Klasse Verbindungen) meldet Widersprüche (z. B. Lecköl an Steuerleitung).
  **Signalart:** Druck-, Sauganschluss oder undefiniert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_anschlusslogik_fluidfunktionen.htm
- **Anschlussmaß:** an Funktionen „Anschlussmaß (alle)“ (**ID 20375**, mehrere Anschlüsse durch ¶ getrennt), an
  Verbindungsdefinitionspunkten Quelle/Ziel (**ID 31096/31097**), an Paketen/Leitungen Quelle/Ziel (**ID 20378/20379**).
  Am Artikel über die Funktionsschablonen (Spalte Anschlussmaß) bzw. bei Verbindungsartikeln; wird bei Artikel-/
  Geräteauswahl übertragen. Prüfläufe **005086** (Verbindung ↔ Funktion) und **005087** (Direktverbindung, z. B.
  Ventil–Schalldämpfer): Vergleich **groß-/kleinschreibungsgenau**, Leerzeichen ignoriert (`g3/4` ≠ `G3/4`).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_prueflaeufe.htm

## 7. Abbruchstellen nach DIN ISO 1219-2

- Fluidleitungen über Seitengrenzen brauchen auf beiden Seiten eine **gleich aufgebaute Referenz** mit Bindestrich
  und Zielseite; optional Planart und Pfad.
- Automatisch beim Ausschneiden/Einfügen auf Fluidseiten, wenn Vorlage FL_1219-2_tpl001 und **Smart-Connecting**
  aktiv. Inhalt im rechteckigen Textrahmen: BMK der Abbruchstelle – Gewerkskennzeichen der Gegenseite +
  Seitenname – Zeile / Spalte des Gegenstücks. Beispiel: `001-P4-B/2` auf Seite 3 (Zeile A, Spalte 4) zeigt auf
  `001-P3-A/4` auf Seite 4 (Zeile B, Spalte 2).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_abbruchstellen.htm

**Bedeutung für QET:** Fluid-Abbruchstellen nutzen **Zeile + Spalte** (Raster mit Buchstabenzeilen), nicht nur die
Spalte wie im Elektroplan. Für den QET-Nachbau: Abbruchstellenpaar als Folio-Report mit Text im Format
`<BMK>-<Medium><Seite>-<Zeile>/<Spalte>` (Vermutung zur Umsetzung; QET-Querverweistext muss dafür Zeile ausgeben).

## 8. Querverweise Ventilspule ↔ Elektroplan, SPS-Funktionen auf Fluidseiten, Makros in Mischprojekten

- Querverweis Magnetspule Fluidplan ↔ Elektroplan entsteht nur, wenn **BMK, Anschlussbezeichnungen und
  Funktionsdefinition** übereinstimmen, die Fluid-Spule **keine Hauptfunktion** ist, Darstellungsart
  **Paarquerverweis** und Gewerk **Elektrotechnik** hat (bei mitgelieferten Symbolen automatisch).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_querverweise.htm
- SPS-Kästen/-Anschlüsse auf Fluidseiten werden zu „SPS-Kasten (Fluid)“ / „SPS-Anschluss (Fluid)“, verhalten sich
  wie Elektro-SPS (Navigator, Zuordnungsliste, Datenaustausch, Prüfläufe „SPS / Bus“); Nummerierung nach den
  SPS-Einstellungen. Ist BMK-Übernahme für Fluid aus, müssen BMK und Steckerbezeichnung an jedem SPS-Anschluss
  einzeln stehen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_sps.htm
- Fenster-/Symbolmakros können Fluid- und Elektroteil zugleich enthalten, unterschieden über Darstellungsart
  **„Allpolig Fluid“** vs. „Allpolig“; beim Einfügen wählt EPLAN den Teil passend zum Seitentyp (auch beim
  Drag & Drop aus Geräteliste/Stückliste).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_makros.htm

**Bedeutung für QET:** Ventilspule `-Y1` im Elektroplan = Master (mit Artikel), Spule am Ventil im Fluidplan = Slave
mit gleichem BMK und gleichen Anschlussbezeichnungen (A1/A2). `verknuepfen.py` kann sie wie Spule ↔ Kontakt
verbinden. Beim Import von EPLAN-Makros mit Darstellungsart „Allpolig Fluid“: diese Variante als eigenes
Fluid-Element importieren, nicht verwerfen (Vermutung zum Fork-Leser).

## 9. Pakete, Rohr- und Schlauchleitungen als Betriebsmittel

- Funktionsdefinitionen der Kategorie Paket / Leitung: **Paketdefinition**, **Rohrleitungsdefinition**,
  **Schlauchleitungsdefinition**.
- **Schlauchpaket** = wie Kabeldefinition einfügen (Einfügen > Kabeldefinition, Linie PDL aus SPECIAL über die
  Verbindungen ziehen); an jedem Schnitt entsteht ein Verbindungsdefinitionspunkt, dessen Funktionsdefinition auf
  „Schlauch“ stehen muss. Sichtbares BMK, Länge, Querschnitt/Durchmesser, Einheit des Pakets gehen an die
  Verbindungen, sofern dort leer. Symbol/Variante der Punkte unter Einstellungen > Betriebsmittel > Paket.
- **Rohr-/Schlauchleitung mit BMK:** Symbolmakros **Pipe1–Pipe5** bzw. **Hose.ems** in die Verbindung setzen,
  BMK vergeben.
- Anzeige im **Betriebsmittel-Navigator** (nicht im Kabel-Navigator), gleiche Icons wie Kabel. Filter über
  **Verbindung: Zugehörigkeit (ID 31142)** = „Paket“ oder „Leitung“. Auswertungen: Artikel(summen)stückliste,
  Betriebsmittelliste, Verbindungsliste, **Paket / Leitungsplan**; keine kabelspezifischen Auswertungen.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_schlauchpakete.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_paketedefinieren.htm

**Bedeutung für QET:** Ein Schlauchpaket entspricht logisch einem Kabel mit Adern (Schläuchen); im Nachbau wie ein
Kabel (Kabeldefinitionslinie + Aderzuordnung) zeichnen. Eine Schlauchleitung mit BMK (Hose.ems) ist ein Element auf
der Leitung, nicht nur ein Leitertext.

## 10. Auswertungen und Prüfläufe

- Auswertungen über Schemata (Filter, Sortierung, Seitenumbruch): Betriebsmittel-, Artikellisten,
  **Verrohrungs-/Verschlauchungslisten** (Verbindungslisten mit Rohr/Schlauch, Ziel-BMK, Querschnitt, Länge),
  Paket-/Leitungspläne.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_auswertungen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verrohrungs_verschlauchungslisten.htm
- Ablauf Verrohrungsliste: Formular für Verbindungsliste (z. B. **F27_FT004**), Anschlussart Fluid an allen
  Anschlüssen, Vorbelegung Hydraulik = Rohr, Pneumatik = Schlauch, Durchmesser und Länge je Gewerk; Filter
  „Verbindung: Funktionsdefinition = Rohr ODER Schlauch“, Sortierung nach Gewerk. Alternativ über
  Verbindungsdefinitionspunkte mit Querschnitt, Verbindungsbezeichnung, Funktionsdefinition, Länge + Einheit.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_verrohrungsliste_auswerten_ausgeben.htm
- Prüfläufe: Meldungsklasse **Fluid = 024** (Anschlusslogik, Gewerk/Untergewerk, Darstellungsart); Anschlussmaße
  in Klasse 005 (005086, 005087), siehe Abschnitt 6.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_prueflaeufe.htm

## 11. Schlauchleitungskonfigurator (DIN 20066)

- Add-on (in allen Fluid-Ausbaustufen enthalten) oder Stand-alone. Regelwerk standardmäßig **DIN 20066 (Januar
  2012)**, Schema „DIN 20066_2012-01“; erzeugt Schlauchleitungskennzeichnung und **Typencode** (eindeutige
  Bestellangabe für jeden Hersteller).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_k_start.htm
- Ablauf: Schlauchleitungen (Hose.ems) mit BMK platzieren, markieren, Projektdaten > Verbindungen >
  Schlauchleitungen konfigurieren; im Dialog Bearbeiten Betriebsdruck (max. dynamisch, bar), Nenndurchmesser (mm),
  Schlauchtyp-Norm (folgt aus Druck + Durchmesser), Länge (zwischen Dichtköpfen bzw. Armaturmittelpunkten),
  Hersteller, Herstelldatum, Armaturen links/rechts (Bauform/Baureihe), Verdrehwinkel; Blockbearbeitung mehrerer
  Leitungen (`<<...>>` bei unterschiedlichen Werten).
- **Kennzeichnung** (Reihenfolge nach DIN 20066): Hersteller-Kurzname / Betriebsdruck mit Einheit (bar oder MPa) /
  Herstelldatum `JJMM`, getrennt durch „ / “.
- Rückgabe an EPLAN nur bei **gleichem vollständigem BMK**: Registerkarte Paket/Leitung bekommt Typ, Querschnitt/
  Durchmesser, Länge, Kennzeichnung, Herstelldatum, Verdrehwinkel, Betriebsdruck; Artikelpositionen 1 = Armatur
  links, 2 = Armatur rechts, 3 = Schlauchtyp, **4 = Typencode**.
- Austausch Konstruktion ↔ Fertigung über **tabulatorgetrennte Unicode-TXT** (Excel-tauglich).
- Einstellungen: Präfix für vorläufige BMK (Standard **H** → H1, H2 …), Artikelvorlage (leer = Artikelreferenz,
  gefüllt = Projektartikel; Standard „Fluid-Hose-1“), Typencode-Format (Elemente Querschnitt, Typ, Länge, Teileart 1/2
  = Armaturen, Verdrehwinkel, Trennzeichen), Längeneinheit (Standard mm, nicht im Typencode), Tabellen für
  Nenndurchmesser, Armaturen (mit Bilddateien), Hersteller, Schlauchgruppen, Schlauchzuordnungen (Druck → Gruppen,
  min./max. Durchmesser), bis zu 200 weitere Eigenschaften für herstellerspezifische Typencodes.
- Mit Pro Panel: Typencode zunächst ohne Länge; „Freies Verlegen (Bauraum)“ trägt die ermittelte Länge ein und
  aktualisiert sie bei Kurvenänderung.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_k_arbeitsweisenundanwendungsmoeglichkeiten.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_k_verbindungseigenschaften.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_k_datenaustausch.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_k_typencodeaus3d.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_d_schlauchleitungenbearbeiten.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_r_nummerierung.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_d_formattypencode.htm

**Bedeutung für QET:** Steht an einer Schlauchleitung im PDF ein Typencode, ist er Artikeldaten (Position 4), keine
Leitungsbezeichnung; im Nachbau als Elementinfo/Artikel der Schlauchleitung ablegen.

## 12. Verlegen im Bauraum, Rohrleitungen und Rohrbiegen

- Schläuche werden **frei verlegt** (ohne Streckennetz) entlang einer Raumkurve mit Stützpunkten; Voraussetzungen:
  3D-Grafikmakros (Registerkarte Montagedaten > Grafikmakro), Anschlussbilder (Anschlussrichtung), Artikel für
  Verschraubungen (Produktgruppe Zubehör > Verschraubungen, Funktionsdefinition „Artikelplatzierung,
  Fluid-Verteiler“), Verlegezubehör, Verbindungsdaten Farbe (z. B. BU) und Querschnitt/Durchmesser (`Ø10x1,5`).
  Ist eine Verschraubung platziert, wird an deren nächstem freien Anschluss verlegt. Verlegezubehör wird Strecken/
  Kurven zugewiesen, nicht leeren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verlegen.htm
- **Rohrleitungen:** im Schaltplan allpolig als Verbindung mit Verbindungsdefinitionspunkt „Rohr“ (ohne BMK) oder
  als Paket/Leitung „Rohrleitungsdefinition“ (mit BMK). Freies Verlegen erzeugt den kürzesten Weg mit 90°-Bögen,
  **ohne Kollisionsprüfung**; danach Segmente/Biegestellen verschieben, Radius ändern (Unterschreiten des
  Mindestbiegeradius = hellblau), Biegestellen einfügen (teilt Segment in Drittel, 4 neue Bögen) oder löschen.
  Export je Rohr als **XML für Rohrbiegemaschinen** (Dienstprogramme > Fertigungsdaten > Mechanische Bearbeitung >
  Rohrbiegung).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/piping_k_start.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/piping_h_freiverlegen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/piping_h_rohrleitungaendern.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/piping_h_rohrbogenaendern.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/piping_h_rohrleitungexportieren.htm

**Bedeutung für QET:** Für den 2D-Nachbau ohne Belang; nur die Schaltplan-Seite (Rohr vs. Rohrleitung mit BMK) ist
relevant.

## 13. Festo-Produktkatalog

- Import aus einem **Warenkorb** der Festo-Bestellverwaltung in die Artikelverwaltung oder Geräteliste
  (gleiche Artikel aus mehreren Warenkörben zusammengefasst) oder direkter Zugriff per API bei der Artikelauswahl
  (Datenquelle-Schema „Festo-Produktkatalog“, gefiltert nach Festo-Eigenschaft **SymbolId** des Makros/Symbols).
- Übertragen: Artikelnummer (aus mehreren Feldern zusammensetzbar), Makro-/Symbolzuordnung (fast jede Nummer hat ein
  Symbol nach **ISO 1219-1**; Zubehör ohne Symbol wird trotzdem importiert), kaufmännische Daten, **Hyperlink** zur
  Festo-Seite (Felder Makro auf Technische Daten, Datei/Hyperlink 1 auf Dokumente). Anzeige im Plan über
  „Artikel: Externes Dokument 1 [n]“, Strg+Klick öffnet die Seite.
- Platzierung aus der Geräteliste: zugeordnetes Makro, sonst Symbol, sonst Meldung.
- Steuerdatei **festo.cfg** (Verzeichnis `..\<Version>\cfg`): Abschnitte „Festo-Settings“ (Eigenschaftszuordnung)
  und „Festo-MacroMapping“; max. 400 000 Byte, Zeilenende `\r\n`, ≤ 255 Rechenoperationen je Zeile, IF mit einem
  Vergleichspaar (==, !=, <, >, <=, >=), groß-/kleinschreibungsgenau, kein ELSE, sequenziell. Auswahllistenwerte
  als Nummern (Datensatztyp: 1 Einzelteil, 4 Baugruppe, 5 Modul, 6 Bohrbild, 7 Anschlussbild, 11 Kunde,
  12 Hersteller/Lieferant).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_k_start.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_k_zugriffaufstueckliste.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_k_zuordnungsdatei.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_k_listeauswahllisten.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_k_listeeigenschaften.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_h_artikelimportieren.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_h_artikelauswaehlen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_h_hyperlinksanzeigen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verwaltung_artikelstammdaten.htm

**Bedeutung für QET:** Festo-Ventilinseln und Ventile kommen besser über Data Portal/EDZ (Herstellermakros, Regel
„Herstellersymbole zuerst“); die Festo-SymbolId ↔ Makro-Zuordnung ist nur ein Hinweis, welches ISO-1219-Symbol der
Hersteller vorsieht.

## 14. Ergänzung: Gewerk beim Geräteeinfügen auswählen
Hat ein Artikel in der Artikelverwaltung **mehrere fluidtechnische Gewerke**, fragt EPLAN beim Einfügen > Gerät nach dem Platzieren, welches Gewerk das Gerät erhält. Angeboten werden alle dem Artikel zugeordneten Fluid-Gewerke plus „Fluid (undefiniert)“; das gewählte Gewerk geht an die Funktionen des Geräts.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_gewerkauswaehlen.htm

## 15. Quellen

EPLAN Fluid (Konzepte):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_seitentyp_fluid.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_betriebsmittelstruktur.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_gewerke_an_schaltzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_anschlusslogik_fluidfunktionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verbindungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verbindungssymbole.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_einstellungen_fluidverbindungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_darstellungbmkanschlussb.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_bmknummerierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_syntaxueberpruefung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_abbruchstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_querverweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_sps.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_makros.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_geraetekaesten_anschluesse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_schlauchpakete.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verrohrungs_verschlauchungslisten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_auswertungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_prueflaeufe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_tabellarische_bearbeitung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_verwaltung_artikelstammdaten.htm

EPLAN Fluid (Handlungen):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_bmk_auswaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_bmk_tabellarisch_bearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_fluidbetriebsmittel_offline_nummerieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_fluidbetriebsmittel_online_nummerieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_fluidleitungen_farblich_hervorheben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_paketedefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_h_verrohrungsliste_auswerten_ausgeben.htm

Schlauchleitungskonfigurator (überflogen: Dialog-/Registerkartenseiten):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_k_arbeitsweisenundanwendungsmoeglichkeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_k_datenaustausch.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_k_typencodeaus3d.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_k_verbindungseigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_d_schlauchleitungenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_d_schlauchleitungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_d_einstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_d_formattypencode.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_d_armaturauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_d_herstellerauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_h_armaturenauswaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_h_schlauchleitungenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_h_schlauchleitungeneinlesen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_m_menuestandalone.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_r_armaturen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_r_hersteller.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_r_nenndurchmesser.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_r_nummerierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_r_schlauchgruppen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_r_schlauchzuordnungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/fluidhoseconfiggui_r_weitereeigenschaften.htm

Rohrleitungen im Bauraum:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/piping_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/piping_h_freiverlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/piping_h_rohrleitungaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/piping_h_rohrbogenaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/piping_h_rohrleitungexportieren.htm

Festo-Produktkatalog:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_k_zugriffaufstueckliste.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_k_zuordnungsdatei.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_k_listeauswahllisten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_k_listeeigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_h_artikelimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_h_artikelauswaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/festocatalog_h_hyperlinksanzeigen.htm

Zusatz-Dialogseiten (Ergänzung Abschnitt 14):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_gewerkauswaehlen.htm
