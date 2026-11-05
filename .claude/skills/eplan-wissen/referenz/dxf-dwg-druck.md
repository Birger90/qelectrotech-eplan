# EPLAN Drucken, DXF/DWG-Import und -Export, Veröffentlichen (EPDZ)

Was die EPLAN-Hilfe (Plattform 2.9, gelesen am 05.11.2026) zum Drucken, zur DXF-/DWG-Schnittstelle mit ihren
Zuordnungstabellen (Ebenen, Farben, Schriften, Linientypen, Blöcke) und zum Export ins EPDZ-Format („Veröffentlichen“,
Hilfekapitel `edaexportgui`) sagt, und was daraus für den QET-Nachbau folgt. PDF-Export: `pdf-export.md`. Eigene
Worte; jede Aussage mit Quelle; „Vermutung“ = nicht durch die Hilfe belegt.

## Inhalt

1. Drucken: Prinzip und Umfang
2. Dialog Drucken (Seitenbereich, Exemplare, Reihenfolge, nur geänderte Seiten)
3. Einstellungen: Drucken (Druckgröße, Maßstab, Korrekturfaktor, Ränder, Schwarz-Weiß)
4. Fremddokumente und Begleitdokumente
5. DXF/DWG: Grundlagen und Grenzen
6. DXF/DWG-Export (Dateien, Dateinamen, Unterverzeichnisse, Linientypen-/Plotstildatei)
7. DXF/DWG-Import (Grafikseite, Einfügen, Fenstermakro, Skalierung, Seiten zuweisen)
8. Zuordnungstabellen: Ebenen, Farben, Schriftarten, Linientypen
9. Blockdefinition und Blockattribute
10. Veröffentlichen: EPDZ-Export („EDA-Export“)
11. Bedeutung für den QET-Nachbau (Zusammenfassung)
12. Ergänzung: Export für EPLAN Smart Wiring (EPDZ)
13. Quellen

---

## 1. Drucken: Prinzip und Umfang

- Drucken aus Seiten-Navigator oder Grafischem Editor; außer Projektseiten auch Symbolbibliotheken, Makroprojekte,
  Normblätter und Formulare. Schwarz-Weiß, farbig oder Graustufen, auf Drucker oder in Datei; Druckvorschau.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_k_start.htm
- Gleiche Seite auf verschiedenen Druckern kann verschieden aussehen (eigene Schriften und Ränder je Treiber).
  Typisches Problem: A3-Normblatt auf A4 gedruckt -> Lochrand an falscher Stelle, Klemmenplan (Ausrichtung „links nach
  rechts“) steht beim Abheften auf dem Kopf. Seiten werden standardmäßig auf das Druckerformat skaliert
  (Seitenverhältnis wahlweise erhalten); Skalierung abschaltbar für maßstäbliche Grafik-/Montageseiten. Umgekehrte
  Druckreihenfolge für Drucker ohne passende Ablage.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_k_prinzip.htm

## 2. Dialog Drucken

- Drucker mit Windows-Eigenschaften; **Ausgabe in Datei umleiten** erzeugt eine *.PRN-Datei.
- Seitenbereich: **Aktuelle Seite** (aus dem Editor), **Markierung** (im Navigator markierte Seiten; ist das Projekt
  markiert und ein Seitenfilter aktiv, nur die gefilterten Seiten), **Ganzes Projekt** (Seitenfilter wird ignoriert).
- Exemplare: Anzahl, Sortieren (satzweise), Umgekehrte Druckreihenfolge; sonst Reihenfolge wie im Projekt.
- **Nur geänderte Seiten**: Seiten, deren „Änderungsdatum (automatisch)“ jünger ist als das letzte Druckdatum.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_drucken.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_h_drucken.htm
- Druckvorschau zeigt die Position auf dem Papier; Farbdruck immer im Farbschema „Farbig auf Weiß“; Kontrollkästchen
  „Seite wird gedruckt“ zeigt bei „Nur geänderte Seiten“, was entfällt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_druckvorschau.htm

## 3. Einstellungen: Drucken

- **Maßstäblich drucken (1:1)**: Maßstab aus dem Normblatt, zu Großes wird abgeschnitten; **Seitenmaßstab
  berücksichtigen** erlaubt gemischte Maßstäbe; **Korrekturfaktor** horizontal/vertikal (Standard 1; Beispiel: Linie
  2000 mm bei 1:10 druckt 199 statt 200 mm -> Faktor 200/199 = 1,00502).
- **Auf Seite skalieren**: auf das Druckerformat, mit oder ohne „Seitenverhältnis beibehalten“ (ohne = Verzerrung).
- **Druckränder** links/rechts/oben/unten beziehen sich auf den **bedruckbaren Bereich** des Druckers (dessen Abstand
  zum Blattrand kommt hinzu); **Druckposition** auf dem Blatt. Dieselben Ränder nutzt der PDF-Export.
- **Schwarz-Weiß drucken** ist Standard: alles schwarz auf weiß außer Bilddateien (immer Originalfarben); aus = Farben
  nach „Farbig auf Weiß“. Farbe auf SW-Drucker -> Graustufen (gelbe Linie kaum sichtbar).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_druckoptionen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_druckraender.htm

## 4. Fremddokumente und Begleitdokumente

- **Fremddokument-Seiten** werden über das zugehörige Programm gedruckt: ohne Normblatt, ohne Mitzählen der
  Seitennummern, nicht in der Vorschau; Reihenfolge, Anzahl und Sortieren wirken nicht; sie kommen **immer am Ende**
  in getrennten Druckjobs.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_k_prinzip.htm
- **Begleitdokumente** (Projekt > Begleitdokumente drucken): über Hyperlinks eingefügte oder an Artikeln eingetragene
  externe Dateien (lokal oder Netzlaufwerk). Nicht enthalten: Fremddokument-Seiten, Links auf Auswertungsseiten (über
  die Artikel abgedeckt), http-Links. Tabelle Dokument / Seite bzw. Artikel / Drucken (Standard: alle an).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_begleitdokumente.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_h_begleitdokumente.htm

**Bedeutung für QET:** Datenblätter, die hinten an einem EPLAN-Plan hängen, sind meist Fremddokument-Seiten bzw.
Begleitdokumente – nicht nachzeichnen, sondern als PDF anhängen bzw. per Hyperlink referenzieren.

## 5. DXF/DWG: Grundlagen und Grenzen

- DXF/DWG (Autodesk) ist ein Grafik-Austauschformat; **Schaltplanlogik wird nicht übertragen** (ein BMK ist dort nur
  Text, ein Symbol nur Linien; die Querverweisfähigkeit geht verloren). Gedacht für allgemeine Grafik, Mechanik und
  Lieferung an Kunden ohne EPLAN.
- Drei Importwege: als **Grafikseite** ins Projekt, **auf eine Seite einfügen**, **Fenstermakro** erzeugen.
- Unterstützte AutoCAD-Formatversionen stehen in der Registerkarte Export; Elemente außerhalb dieser Versionen werden
  überlesen. Import nur **2D**. Bilder werden beim Export nur referenziert (mitliefern!), beim Import nachgeladen,
  wenn vorhanden; Fremddokumente sind nicht exportierbar.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_k_grundlagen.htm
- Einstellungen als Schema (Benutzer > Schnittstellen > DXF-/DWG-Export und -Import): Registerkarten Ebenen, Farben,
  Schriftarten, Linientypen gelten für Export **und** Import; Export und Import getrennt; Blockdefinition und
  Blockattribut nur beim Export.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_einstellungen.htm

## 6. DXF/DWG-Export

- Seite > Exportieren > DXF / DWG: **eine Datei je Seite** (im Unterschied zum PDF: eine Datei für alles). Quelle =
  Projekt oder vollständige Seitennamen; Schema; Exportmedium Datenträger (Ausgabeverzeichnis) oder E-Mail (gepackt,
  Teilgröße 1–10 MB oder unbegrenzt, Betreff „<Dateiname> n/m“, MAPI-Client nötig). Freier Dateiname nur bei **einer**
  Seite; „Anwenden auf gesamtes Projekt“; Dateityp DXF oder DWG im Schema.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_export.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_h_seitendxfexportieren.htm
- Registerkarte Export: Seitenfilter (fehlt er später, bricht der Export ab); **Dateinamen erzeugen** aus fortlaufender
  Nummer (nächste freie), aus Seitennamen oder aus Eigenschaften (Formatdialog); **Unterverzeichnisse** keine (dann
  trotzdem ein projektbezogener Ordner gegen Überschreiben), aus Seitenbaum (z. B. Ordner `=Anlage1`) oder aus
  Eigenschaften (Formatelement „Verzeichnistrenner“); **Linientypendatei** (vom CAD geliefert, z. B. ACAD.lin) und
  **Plotstildatei** (benannte Plotstile; ctb wird ohne Pfad eingetragen und muss im AutoCAD-Plotstil-Ordner liegen;
  monochrome.ctb macht den Export nicht einfarbig).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_export.htm

## 7. DXF/DWG-Import

- Seite > Importieren > DXF / DWG (Mehrfachauswahl, Dateityp-Filter, Drag & Drop in den Seiten-Navigator): **je Datei
  eine Seite vom Typ „Grafik“**. Einfügen > Grafik > DXF / DWG setzt eine Datei auf die offene Seite (auch im Symbol-,
  Formular- und Normblatteditor; Drag & Drop in den Editor). Dienstprogramme > Makros erzeugen > Aus DXF-/DWG-Dateien
  erzeugt **Fenstermakros** `<ursprünglicher_Name>.ema` im Zielverzeichnis.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_import.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_h_seitendxfimportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_h_seitengrafeditoreinfuegen.htm
- **Seiten zuweisen**: je Datei Überschreiben ja/nein, Strukturkennzeichen (Anlage, Einbauort …), Seitenname,
  Zusatzfeld Blattnummer, [Nummerieren].
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_seitenzuweisen.htm
- Registerkarte Import:
  - Startverzeichnis; **Zuweisung zu Seiten** in Auswahlreihenfolge oder numerisch (nicht-numerische Namen
    alphanumerisch dahinter) bzw. alphanumerisch (ASCII) nach Dateiname; Startseite 1–99 999 und Schrittweite.
  - **Gefrorene Ebenen ignorieren**: an = nicht importiert; aus = importiert, Ebene aber unsichtbar und gesperrt.
  - Skalierung: **Skalierungsdialog** mit Zeichnungsgrenzen vom Zeichnungskopf / aus den Koordinaten / vom größten
    Element, oder **feste Skalierung** (Faktor Standard 1, Position Standard 0,00).
  - **Seitenmaßstab anpassen** (im Schema „Standard“ an): Import 1:1, mittig im Normblatt, Seitenmaßstab passend zur
    Zeichnung (für Hallen-/Geländepläne). Nicht beim Einfügen auf bestehende Seiten; solche Seiten nicht erneut
    DXF-exportieren und -importieren (Texte auf skalierbaren Ebenen werden zu groß).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_import.htm
- **Importformatierungen** (bei Option „Manuelle Korrektur“): Skalierung automatisch (wenn größer als Normblatt) /
  manuell / keine; Position zentriert / unten links (= Normblatteigenschaft „Auswertungsbereich Anfangspunkt X/Y“,
  meist der Nullpunkt) / manuell; Breite/Höhe, Zeichnungsgrenzen, Seitenverhältnis beibehalten, Vorschau im Normblatt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_importformatierung.htm

## 8. Zuordnungstabellen: Ebenen, Farben, Schriftarten, Linientypen

| Tabelle | Inhalt | Nicht Zugeordnetes | Quelle |
|---|---|---|---|
| Ebenen | EPLAN-Ebene (Name z. B. `EPLAN100`, Beschreibung z. B. „Symbolgrafik.Allgemein“; Standardebenen `EPLANxxx` mit fester Beschreibung) -> **CAD-Layername** (Vorgabe = EPLAN-Name); Sichtbar/Drucken/Gesperrt wie Ebenenverwaltung (ohne Rückwirkung aufs offene Projekt); [Hinzufügen] übernimmt alle im Projekt benutzten Ebenen, ohne vorhandene Zuordnungen zu überschreiben | 1:1, Eigenschaften (Linienstärke, Schriftgröße) möglichst mit | https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_ebenen.htm |
| Farben | EPLAN-Farbe -> CAD-Farbnummer (Beispiel Schwarz -> 250); nur zu konvertierende Farben eintragen | 1:1 | https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_farben.htm |
| Schriftarten | EPLAN-Schriftart (aus Projekteinstellung Grafische Bearbeitung > Schriftarten, z. B. „0“) -> CAD-Schrift + **Skalierungsfaktor** (Beispiel „Standard“, 100) | nicht genannt | https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_schriftarten.htm |
| Linientypen | EPLAN-Linientyp -> CAD-Linientyp (Beispiel Durchgezogen -> CONTINUOUS); Linienstärken in AutoCAD-Anzeige einschalten | nicht genannt | https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_linientypen.htm |

Hinweis der Hilfe: Logische Elemente einer Seite lassen sich in DXF/DWG nur über getrennte Layer erkennen – daher die
Ebenenzuordnung pflegen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_ebenen.htm

## 9. Blockdefinition und Blockattribute

- Registerkarte **Blockdefinition** (nur Export): dynamische und bedingte Formularbereiche werden als Blöcke mit
  wählbarem Präfix exportiert (z. B. „KOPF“ für den Kopfbereich). Optionen: unsichtbare Attribute exportieren; **pro
  Symbol einen Block** (aus = reine Grafik); Block für Normblätter; Block für Standardformulare. Bei Blöcken gehen die
  EPLAN-Objekteigenschaften als erweiterte Blockeigenschaften mit.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_blockdefinition.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_k_grundlagen.htm
- Registerkarte **Blockattribut**: grundsätzlich werden **alle** Eigenschaften exportiert; die Tabelle legt nur fest,
  unter welcher **Attributbezeichnung** und **Eingabeaufforderung** eine Eigenschaft eines Elements in AutoCAD
  erscheint (Dialog Attributdefinition mit Kategorie-Filter).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_blockattribut.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_attributdefinition.htm

**Bedeutung für QET:** Liefert ein Kunde statt PDF ein EPLAN-DXF/DWG mit „pro Symbol einen Block“, stehen BMK, Artikel
und weitere Eigenschaften als Blockattribute darin – eine deutlich bessere Datenquelle als PDF-Text (**Vermutung**:
Attributnamen hängen vom Exportschema ab; je Datei prüfen). Ohne Blöcke ist es reine Grafik wie ein PDF. Layernamen
`EPLANxxx` verraten die Elementart (Symbolgrafik, Texte, Verbindungen) und helfen, Grafik und Beschriftung zu trennen.
Ein DXF-Import in den Fork müsste Layer -> QET-Grafik/Text und Blöcke -> Bauteile mit Informationen abbilden
(**Vermutung**: ein solcher Import fehlt im Fork, Code nicht geprüft).

## 10. Veröffentlichen: EPDZ-Export („EDA-Export“)

- Projekt > Veröffentlichen exportiert ins **EPDZ**-Format für die EPLAN View App (Mobilgeräte, z. B. über Dropbox).
  Inhalt: Projekt-, Seiten- und Bauraumeigenschaften (damit die ganze Projektstruktur), **je Seite eine SVG-Datei**
  mit der 2D-Vektorgrafik, **je Bauraum eine E3D-Datei** mit 3D-Daten. EPDZ für EPLAN Smart Wiring liegt unter
  Dienstprogramme > Fertigungsdaten > EPLAN Smart Wiring.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_d_veroeffentlichen.htm
- Dialog: Quelle (Projektname), Schema, EPDZ-Datei (Standard Projektname.epdz), Datenträger (auch USB/CD) oder E-Mail
  **ohne Aufteilung** (Mobilgeräte lesen keine geteilten Dateien).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_d_veroeffentlichen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_r_export.htm
- Schema-Registerkarten: Export, Projekt-, Seiten-, Bauraum-, Funktions-, Abbruchstelleneigenschaften. Ausgabe:
  **Quellsprache** (bei `##_##` die aktuelle Dialogsprache) oder **eine** alternative Übersetzungssprache (die
  Anzeigesprachen werden dann ignoriert); „Extern verlinkte Dokumente ins Ausgabeverzeichnis kopieren“ (Fremddokument-
  Seiten, Datei-Hyperlinks, Datei-Links in Texten/Eigenschaften; sonst nur eingelagerte oder im Dokumente-Standard-
  ordner liegende).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_d_einstellungenveroeffentlichen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_r_export.htm
- Registerkarte Projekteigenschaften: Liste der exportierten Projekteigenschaften, erweiterbar (Neu, Löschen,
  Reihenfolge; Löschen entfernt nur aus der Liste – belegte Eigenschaften erscheinen wieder). Seiten- und
  Bauraumeigenschaften nur ansehbar. (Funktions- und Abbruchstelleneigenschaften: keine eigene Seite in der Liste.)
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_r_eigenschaften.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_r_seiteneigenschaften.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_r_bauraumeigenschaften.htm

**Bedeutung für QET:** Eine EPDZ-Datei wäre eine sehr gute Nachbauquelle: SVG je Seite = exakte Vektorgrafik mit
echten Koordinaten (besser als PDF-Extraktion), dazu Projekt-, Seiten-, Funktions- und Abbruchstelleneigenschaften.
Falls ein Kunde EPDZ liefern kann, vor dem PDF-Auslesen danach fragen. **Vermutung:** EPDZ ist ein ZIP-Container;
Aufbau vor Nutzung an einer echten Datei prüfen.

## 11. Bedeutung für den QET-Nachbau (Zusammenfassung)

1. Seitengröße/Lochrand: EPLAN skaliert beim Drucken/PDF unter Umständen; das Normblattformat ist maßgeblich.
2. Fremddokument-Seiten (Datenblätter) stehen beim Druck immer am Ende und zählen keine Seitennummern – nicht als
   Schaltplanseiten nachbauen.
3. Datenquellen nach Güte: EPLAN-Projekt > EPDZ (SVG + Eigenschaften) > DXF/DWG mit Blöcken/Attributen > PDF.
4. DXF-Ebenen `EPLANxxx` und Blockattribute erlauben eine Trennung von Symbolgrafik, Texten und Eigenschaften.
5. DXF ist auch bei EPLAN nur Grafik – Verknüpfungen gehen verloren; das PDF bleibt das Navigationsdokument.

## 12. Ergänzung: Export für EPLAN Smart Wiring (EPDZ)
Dienstprogramme > Fertigungsdaten > EPLAN Smart Wiring exportiert das Projekt als **EPDZ** zum Einlesen in Smart Wiring. Inhalt: Projekt-, Seiten- und Bauraumeigenschaften (damit die komplette Projektstruktur), Verbindungsdaten und Kabeleigenschaften der **Verlegeverbindungen** sowie je Bauraum eine **E3D-Datei** mit den 3D-Daten. Vorgabe-Dateiname = Projektname + `.epdz`, Ausgabeverzeichnis frei wählbar; die Quelle (Projekt) ist nicht änderbar.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_d_smartwiring.htm

**Bedeutung für QET**: Für Smart-Wiring-Daten werden verlegte Verbindungen aus dem 3D-Bauraum benötigt – aus einem reinen 2D-QET-Plan lässt sich so eine Datei nicht sinnvoll erzeugen. Vermutung: Dateiformat (Container) wie beim Veröffentlichen-EPDZ aus Abschnitt 10, aber mit Fertigungsinhalt.

## 13. Quellen

Drucken:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_drucken.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_h_drucken.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_druckvorschau.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_druckoptionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_druckraender.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_d_begleitdokumente.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/printgui_h_begleitdokumente.htm

DXF/DWG:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_einstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_export.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_h_seitendxfexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_export.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_import.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_h_seitendxfimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_h_seitengrafeditoreinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_seitenzuweisen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_import.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_importformatierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_ebenen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_farben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_schriftarten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_linientypen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_blockdefinition.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_r_blockattribut.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdxfgui_d_attributdefinition.htm

Veröffentlichen (EPDZ):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_d_veroeffentlichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_d_einstellungenveroeffentlichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_r_export.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_r_eigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_r_seiteneigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_r_bauraumeigenschaften.htm

Zusatz-Dialogseiten (Ergänzung Abschnitt 12):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/edaexportgui_d_smartwiring.htm
