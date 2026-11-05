# EPLAN-Wissen: Makros, Symbole, Artikel, Data Portal

Zusammenfassung der offiziellen EPLAN-Hilfe (Plattform 2.9, ergänzt um 2023–2026 und die Data-Portal-Hilfe) in
eigenen Worten. Zweck: Bauteile aus EPLAN-Makros (EDZ, Data-Portal-DXF) richtig in den QET-Fork importieren
(`sources\import\eplandxf\`: `eplanmacroreader`, `eplanpartimporter`, `eplanelementbuilder`, `dataportalclient`)
und beim Nachzeichnen die richtigen Makros/Varianten wählen. Jede Aussage hat ihre Quelle; „Vermutung“ markiert
eigene Schlüsse, „Fork-Code“ Aussagen aus dem Quelltext statt aus der Hilfe.

Hausgrundsätze: Herstellermakro ergänzen > QET-IEC-Symbol + Artikeldaten > PDF-Nachzeichnung nur ohne Daten.
SPS-Kanal = Kanalmakro des Moduls, nie je Adresse ein eigenes Symbol.

## Inhalt
1. Makroarten und Dateiendungen
2. Inhalt eines Makros, was nicht drinsteckt
3. Varianten und Darstellungsarten
4. Makrokasten, Anfasspunkt, Makroprojekt
5. Platzhalterobjekte, Variablen, Wertesätze
6. Makros einfügen (Varianten, BMK-Nummerierung)
7. Makros aus DXF/DWG
8. Symbole, Symbolbibliotheken, Symbolvarianten A–H
9. Funktionsdefinitionen
10. Artikelverwaltung: Aufbau, Varianten, Zubehör
11. Funktionsschablonen und Gerätedaten
12. Makro-Zuordnung zum Artikel, Normen und Firmenstandards
13. Artikel-Import/-Export, EDZ
14. Data Portal
15. Quellen

---

## 1. Makroarten und Dateiendungen

- **Fenstermakro `.ema`** und **Symbolmakro `.ems`**: markierte Elemente bzw. ein Seitenausschnitt; ein Makrokasten
  in Größe des Ausschnitts wird mitgespeichert. Eingelagerte Bilder werden mitgespeichert.
- **Seitenmakro `.emp`**: eine oder mehrere ganze Seiten (im Grafikeditor nur zusammenhängende).
- Eine Variante entsteht nicht automatisch: Grafik manuell anpassen und unter gleichem Namen mit neuer
  Variantenbezeichnung speichern; vorhandene Variante wird nach Rückfrage überschrieben.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_h_makroserzeugen.htm

- Zweck der Makros: wiederkehrende Schaltungsteile mit gleicher Grafik, aber anderen Daten/Artikeln; Varianten-
  daten lassen sich hinterlegen und über Platzhalterobjekte nach dem Einfügen setzen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_k_start.htm

**Bedeutung für QET:** Herstellerpakete (EDZ) enthalten vor allem `.ema`; der Fork liest diese
(`EdzImporter`/`importEdz` sucht `*.ema` im entpackten Archiv). `.ems` hat dasselbe XML-Format (Vermutung,
nicht geprüft) und könnte vom gleichen Leser verarbeitet werden; `.emp` bräuchte einen Seitenimport.

## 2. Inhalt eines Makros, was nicht drinsteckt

- Gespeichert: Makroname, Version, Quellprojekt, Beschreibung, Quelle/Referenz; außerdem Seitendaten (darum kann
  jedes Fenster-/Symbolmakro auch als Seitenmakro eingefügt werden; beim Einfügen als Ausschnitt wird die Seite
  ignoriert).
- **Nicht** eingelagert: Funktionsdefinitionen, Symbole, Artikel, Formulare, Normblätter – nur deren Quelle und
  Nummer/Name. Fehlt z. B. die Symbolbibliothek im Projekt, wird das Makro eventuell nicht dargestellt; Abhilfe in
  EPLAN: Stammdaten abgleichen, Artikel mit der Datenbank abgleichen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_k_daten.htm

**Bedeutung für QET:** Ein `.ema` enthält Grafik, die direkt im Makro gezeichnet ist (Linien O34, Rechtecke O89,
Kästen und Anschlusspunkte O17 – so liest es `eplanmacroreader`, Fork-Code). Stammt die Grafik dagegen aus einem
EPLAN-Symbol (z. B. Spule aus einer IEC-Bibliothek), steht im Makro nur der Verweis – die Zeichnung fehlt dann
beim Import. Vermutung: das ist der Hauptgrund für „leere“ oder unvollständige Importe; dann QET-IEC-Symbol +
Artikeldaten verwenden (Hausregel Stufe 2), statt die Grafik zu raten. Artikeldaten kommen nicht aus dem Makro,
sondern aus dem Artikel (EDZ-Teil `part.xml` bzw. `commercialdata.csv`).

## 3. Varianten und Darstellungsarten

- Im Makrokasten wählbare **Darstellungsarten**: Allpolig, Allpolig Fluid, Übersicht, Paarquerverweis, Einpolig,
  Topologie, RI-Fließbild, Funktional, Schaltschrankaufbau, 3D-Montageaufbau, Vorplanung, Funktionsübersicht
  (Fluid), Grafik (nur Grafik, ohne Auswertung/Prüfung/Querverweis). Die Darstellungsart dient laut Hilfe der
  Sortierung/Verwaltung und ändert nicht die Funktionen im Makro.
- **Variante**: Standard „Variante A“; **bis zu 26 Varianten je Darstellungsart**.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_r_makrokasten.htm
- Die Seite „Makros erstellen“ formuliert es als bis zu 26 Varianten je Dateiname (Widerspruch zur
  Makrokasten-Seite; maßgeblich für den Import ist, dass eine Datei mehrere Darstellungsarten × Varianten tragen
  kann).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_h_makroserzeugen.htm
- Eine Funktion kann mehrere Darstellungsarten gleichzeitig haben, jede nur einmal; die Darstellungsart beeinflusst
  die Behandlung in Auswertungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_darstellungsarten.htm
- Feste Variantenrollen bei SPS-Kartenmakros: Variante A = E/A-Übersicht, Variante E = Baugruppenträger-Übersicht,
  E/F = Abbruch/Weiterführung; Beschaltungsmakros in Variante A (allpolig und einpolig).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_makrosschaltplanerzeugen.htm

**Bedeutung für QET:**
- Im `.ema`-XML trägt jede `MacroVariant` ein Attribut `MacroFuncType` (Fork-Code). Die Zahlencodes nennt die Hilfe
  nicht; der Fork nimmt `1` = allpolig, und für DXF-Paketdateien die Endungen `_A1`, `_B4` … = Variante + Darstellung
  (1 allpolig, 2 einpolig, 4 Übersicht, 6 Aufbau) (Fork-Code `eplanpartimporter.cpp`, nicht aus der Hilfe belegt).
- `eplanmacroreader` liest nur die **erste** allpolige Variante. Bei Mehrkanal-, Kanal- und SPS-Makros gehen so
  weitere Varianten (B, C …) und die Übersicht verloren. Vermutung/Empfehlung: alle Varianten je Darstellungsart als
  eigene Elemente importieren, benannt nach Variante und Darstellung (wie der DXF-Paketimport es schon tut).
- Im Nachbau Variante und Darstellungsart des Originals treffen: Übersichtsblatt = Übersichtsvariante, Funktionsblatt
  = allpolige Kanalvariante, Aufbauplan = Aufbauvariante.

## 4. Makrokasten, Anfasspunkt, Makroprojekt

- Makrokasten (im Makroprojekt) legt fest, welche Objekte und Umrisse in ein Makro gehen; daraus werden Fenster-/
  Symbolmakros automatisch erzeugt bzw. aktualisiert. In Schaltplanprojekten kann man keine Makrokästen einfügen,
  der Kasten wird aber beim Erstellen mitgespeichert. Seitenmakros haben keinen Makrokasten (Daten in den
  Seiteneigenschaften).
- **Verwendungsart**: Definierend (nur Erzeugen), Referenzierend (nur Aktualisieren), Untergeordnet (keins),
  Nicht festgelegt (beides, Kompatibilität vor 2.8). Beim Einfügen in ein Schaltplanprojekt wird automatisch
  „Referenzierend“. Verschachtelung: innen referenzierend, außen definierend.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_k_makrokaesten.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_r_makrokasten.htm
- Makrokasten-Felder: Name (= Dateiname des Makros, ohne Namen kein Makro), Darstellungsart, Variante, Version,
  Quellprojekt, Quelle/Referenz, Beschreibung, Manuelle Objektzuordnung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_r_makrokasten.htm
- Registerkarte Einstellungen: geschützte Gruppe beim Einfügen, Makrokasten mit einfügen, Anschlussbezeichnungen
  beim Gerätezuordnen berücksichtigen, **Anfasspunkt** (aktiv, X/Y-Position).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_r_makrokasteneinstellungen.htm
- Vorbereiten für die automatische Erzeugung: Makrokasten-Inhalt als Gerätegruppe gruppieren, Seitenmakros
  gruppieren, Anfasspunkte aktivieren, damit sich Makros nach Grafikänderung nicht verschieben.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_d_makrosvorbereiten.htm
- Makroprojekte: eingeschränkte Logik (keine seitenübergreifenden Verbindungen, keine nicht platzierten Funktionen);
  „Makros referenzierend einfügen“ macht sie aktualisierbar, aber kaum änderbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_k_makroprojekte.htm

**Bedeutung für QET:** Der Anfasspunkt entspricht dem Hotspot/Ursprung des QET-Elements. Vermutung: Ist im
`.ema` ein aktiver Anfasspunkt gespeichert, sollte der Import ihn als Elementursprung nehmen, damit das Element
wie in EPLAN am Raster einrastet; sonst Standardposition. Der Makrokasten selbst (Ebene EPLAN308 ff.) ist keine
Bauteilgrafik und gehört nicht ins Element.

## 5. Platzhalterobjekte, Variablen, Wertesätze

- Platzhalterobjekt: Anker-Symbol (Ebene EPLAN322) über einem markierten Bereich; Registerkarte „Zuordnung“
  verknüpft Eigenschaften der Objekte mit Variablen, „Werte“ enthält Variablen und **Wertesätze** (Spalten). „Veränderbar“
  erlaubt Anpassung bei der Auswahl. Werte können z. B. Artikelnummern, Funktionsdefinitionen, Verbindungsfarben
  oder Strukturkennzeichen sein.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_h_platzhalterobjekteerzeugen.htm
- Syntax: Variable in spitzen Klammern `<Name>`, mehrere hintereinander möglich, Text dazwischen wird übernommen
  (`A<PLZH1>:<PLZH2>B`). Umbruch `<Name[24]>` mit Option 0 (hart) oder 1 (am Wortende). `<` `>` als Text doppelt.
  Variablennamen: Buchstabe/Unterstrich am Anfang, keine Sonderzeichen. Ebenen-IDs: Standardebenen tragen die ID im
  Namen (EPLAN100 = 100), eigene ab 1000.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_k_platzhalteraufbauarbeitsweise.htm
- Beim Einfügen eines Makros mit Wertesatz erscheint die Wertesatzauswahl; die Schaltzeichen übernehmen die Daten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_h_makrosauswaehlen.htm
- Bei SPS-Makros geben Variablen den SPS-Funktionen keine Werte; die kommen erst beim Abbuchen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_abbuchverhalten.htm

**Bedeutung für QET:** Texte wie `<Funktionstext>` oder leere Felder in Herstellermakros sind Platzhalter, keine
festen Beschriftungen. Der Fork macht leere EPLAN-Felder zu Benutzertexten (`EplanElementBuilder`); Texte mit
`<…>` sollten ebenso als ausfüllbare Felder statt als feste Zeichnungstexte enden (Vermutung). Das Anker-Symbol
selbst nicht importieren.

## 6. Makros einfügen (Varianten, BMK-Nummerierung)

- Fenster-/Symbolmakro einfügen über Einfügen-Menü bzw. Einfügecenter; Variante über die Liste „Variante“ oder beim
  Positionieren mit Strg + Kreisbewegung. Y/X + Maus fixieren Richtung. Das Kommentarfeld zeigt Quellprojekt und
  Beschreibung (bei DXF/DWG-Makros nur Herkunft).
- Enthält das Makro BMK und ist „Nummerierung (online) mit Abfrage“ aktiv, erscheint der Einfügemodus-Dialog.
- Seitenmakros: Seite > Seitenmakro > Einfügen, Einordnung in die Seitenstruktur je Makro; Seiteneigenschaft
  „Makro: Eingefügt durch“ wird gefüllt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_h_makrosauswaehlen.htm
- BMK mit „?“ (z. B. -?K1) werden beim Einfügen neu nummeriert; gleiche Kennbuchstaben mit gleicher Zahl erhalten
  dieselbe Benennung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_abbuchverhalten.htm

**Bedeutung für QET:** BMK-Vorgaben im Makro wie `-?K1` oder nur Kennbuchstaben sind Vorlagen. Im Import als
Kennbuchstabe (`label` = „-K“) übernehmen, die Nummer kommt beim Platzieren (der Fork setzt schon
`-<Kennbuchstaben>` aus dem Kasten, Fork-Code).

## 7. Makros aus DXF/DWG

- Dienstprogramme > Makros erzeugen > Aus DXF-/DWG-Dateien: mit Importschema in ein Zielverzeichnis, je Datei ein
  Fenstermakro `<Name>.ema`; im Auswahldialog als „DXF“ bzw. „DWG“ gekennzeichnet.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_h_makrosausdxf.htm
- Data-Portal-DXF zeigt Aufbau und Struktur des Artikels, enthält aber **keine logischen Informationen** für EPLAN.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/EDP/Content/htm/EPLANDataPortalHelp_h_dxfdownload.htm

**Bedeutung für QET:** Aus DXF kommt nur Grafik; Anschlüsse, Funktionsdefinitionen und Kanalzuordnung muss der
Fork selbst ableiten (heute über Anschlusspunkte/Texte im DXF und `commercialdata.csv`, Fork-Code). Ein `.ema` aus
dem EDZ ist darum der DXF-Variante vorzuziehen, wenn beides vorliegt (Vermutung, folgt aus „keine Logik im DXF“).

## 8. Symbole, Symbolbibliotheken, Symbolvarianten A–H

- Symbol = Grafik einer Funktion; enthält Grafik, Anschlusspunkte, Symbolgruppe und Logik. Es kann keine, eine,
  mehrere oder einen Teil einer Funktion darstellen. Die Logik liegt meist in der Funktionsdefinition, Anschluss-
  Logik kann im Symbol stehen.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_k_start.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_symbole.htm
- Symbolbibliothek: beliebig viele Symbole; beim Bearbeiten wird nur das einzelne Symbol gespeichert. Symbole werden
  beim Einfügen aufgelöst (keine referenzierenden Symbole); Bearbeitung nur mit offenem Projekt (Projekteinstellungen).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_k_start.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_k_arbeitsweise.htm
- **Symbolvarianten**: bis zu acht, **A bis H**, dazu Kontaktspiegel- und Verbindungspunktvariante. Alle Varianten
  haben dieselbe Funktion und **dieselbe Anzahl Anschlüsse**; sie unterscheiden sich nur in der grafischen Lage der
  Anschlüsse (typisch: gedreht/gespiegelt). Neue Variante entsteht aus einer vorhandenen mit Drehwinkel und optional
  Spiegelung an der Y-Achse. Anzeigeform `<Bibliothek>;<Symbolname>:<Nummer>;<Variante>`.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_mitsymbolvariantenarbeiten.htm

**Bedeutung für QET:** Symbolvarianten A–H sind keine verschiedenen Bauteile. In QET ein Element und Drehung/
Spiegelung beim Platzieren statt acht Elemente (Vermutung/Empfehlung). Makrovarianten (A–Z) können dagegen
wirklich verschiedene Grafiken sein (andere Kanalzahl, andere Beschaltung) und verdienen eigene Elemente.

## 9. Funktionsdefinitionen

- Hierarchie der Funktionsdefinitionsbibliothek: Gewerk → Bereich → **Kategorie** (bestimmt Systemverhalten und
  Eigenschaften, z. B. Kontakte, Spulen, Klemmen) → **Gruppe** (Grundfunktion, z. B. Schließer 2 Anschlüsse) →
  Funktionsdefinition (konkret, z. B. Leistungsschließer).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_funktionsdefinitionsbibliotheken.htm
- SPS-Beispiele: „SPS-Kasten“, „SPS-Anschluss, E / A, 1 Anschluss“, „SPS-Anschluss, Multifunktion“,
  „Netzwerk- / Bus-Kabelanschluss, allgemein“, „SPS-Anschluss, intern, allgemein“.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/plcgui_k_ansichten.htm ·
  https://www.eplan.help/techtipps/de-DE/SPS/TechTip-Configuring-PLC-devices-with-device-description-files.pdf ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_symbolischeadressen.htm

**Bedeutung für QET:** Die Funktionsdefinition entscheidet über den QET-Elementtyp: Kasten/Gerät → `simple`/
Master, Kontakt → Slave, Klemme → `terminal`, SPS-Anschluss → Kanal (Slave des Moduls). Der Fork liest aus O17 die
Anschlussbezeichnung (A185) und die Anschlussbeschreibung (P20406) (Fork-Code); die Kategorie der
Funktionsdefinition auszuwerten wäre der nächste Schritt für den richtigen Elementtyp (Vermutung).

## 10. Artikelverwaltung: Aufbau, Varianten, Zubehör

- Stammdaten für Artikel/Lieferanten/Preise, mehrsprachig; Datenbank Access oder SQL Server.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_start.htm
- Artikeltypen Einzelteil, Baugruppe, Modul (Kabel/Gehäuse sind Einzelteile; ohne Elektro/Fluid-Bezug Gewerk
  Mechanik). Baumebenen u. a. Zubehörliste, Zubehörplatzierung, Bohrbild, **Anschlussbild**, Kunde, Hersteller/
  Lieferant. **Artikelvariante** = ein Zeichen 0–9 oder A–Z; Artikel- und Variantennummer zusammen eindeutig.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelanlegen.htm
- Zubehör: „Artikel ist Zubehör“ (sonst Hauptartikel, der Zubehör/Zubehörlisten erhalten kann), „Erforderlich“ =
  Pflichtzubehör, optional Zubehörplatzierung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_zubehoer.htm
- Hauptartikel eines SPS-Kastens = erster Artikel, der kein Zubehör ist; seine Makrodatei wird übernommen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_makrosschaltplanerzeugen.htm
- Ab 2023 heißen die Hilfeseiten `articlesgui_*` statt `partsmanagementgui_*`; Menü 2.9 „Dienstprogramme > Artikel >
  Verwaltung“, neuer „Stammdaten > Artikel > Verwaltung“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2024/Content/htm/articlesgui_h_funktionsschablonenzuweisen.htm

**Bedeutung für QET:** Im Element (`elementInformations`) gehören Hersteller, Bestellnummer, Typ und Beschreibung
des **Hauptartikels**. Zubehör (BaseUnit, Beschriftungsstreifen, Schirmauflage) nicht als zweites Element, sondern
als weitere Artikelzeile bzw. in die Stückliste (Vermutung: QET kennt keine Zubehörliste am Element).
Pflichtzubehör wie BaseUnits ist trotzdem in der Stückliste zu führen.

## 11. Funktionsschablonen und Gerätedaten

- **Gerätedefinition** = Typ mit Funktionsschablonen; Schablonen enthalten die minimalen identifizierenden
  Eigenschaften und erzeugen Funktionen, platziert oder nicht platziert. Nicht platzierte Funktionen haben alle
  Daten, aber keine Grafik (Symbol, Lage entstehen erst beim Platzieren).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_datenvonnichtplatziertenfunktionen.htm
- Registerkarte Funktionsschablone: eine Zeile je Funktionsdefinition und Artikelvariante; nur **exakte**
  Funktionsdefinitionen verwenden (nicht „Klemme, allgemein“). Hinterlegte Anschlussbezeichnungen werden beim
  Gerätevergleich geprüft (13;14 hinterlegt → 23;24 Fehler). Beschreibung wird nicht an die Funktion übertragen.
  Module haben keine Funktionsschablonen. Beim Platzieren übernimmt die Artikelplatzierung die Anschlüsse der
  Schablonen, wenn sie im Anschlussbild vollständig vorhanden sind oder dort keine definiert sind.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_funktionsschablone.htm
- **Funktionsschablonen zuweisen** (aus Makros): Makro im Artikel (Eigenschaft Schaltplanmakro) wird nach Funktionen
  durchsucht, beginnend mit der Hauptfunktion; Makro muss **genau eine Hauptfunktion** haben. Danach Artikelabgleich
  im Projekt nötig.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2024/Content/htm/articlesgui_h_funktionsschablonenzuweisen.htm
- SPS: Signalbereich und „SPS-Untergerät: Index“ können aus Funktionsschablonen übernommen werden; Bussystem steht
  in der Funktionsschablone; beim SPS-Import muss die **Reihenfolge der Schablonen der Kanalreihenfolge** entsprechen.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsanschluesse.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_untergeraete.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_importbuskonfig.htm
- Gerät mit Artikelmakros einfügen: ohne Makro entsteht ein Kasten mit reinen Funktionsschablonen, mit Makros
  überlagern diese die Schablonen; Reste bleiben nicht platzierte Funktionen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_kanalorientiertarbeitsweise.htm

**Bedeutung für QET:** Gibt es kein Makro, sind die Funktionsschablonen der EDZ die verlässliche Quelle für
Anschlussbezeichnungen und Kanalreihenfolge – daraus baut der Fork das generische Element (`importEdz`, dann
zusätzlich das „Schaltbild“ aus dem `.ema`, Fork-Code). Beim Abgleich Makro ↔ Artikel gilt: Anschlussbezeichnungen
des Makros müssen zu den Schablonen passen; Abweichungen im Import melden statt still übernehmen (Vermutung).

## 12. Makro-Zuordnung zum Artikel, Normen und Firmenstandards

- 2.9: Registerkarte Technische Daten, Feld **Makro** (Symbolmakro oder Kontur „Kontur-Extrusion“; für Module
  wichtig, sonst Meldung beim Speichern). Ein Artikel mit Makro lässt sich aus der Geräteliste per Drag & Drop
  platzieren. Weitere Felder: technische Kenngrößen (aus erster Hauptschablone), Gruppennummer, Teileart,
  Funktionsgruppe.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_technischedaten.htm
- Neuere Versionen: Eigenschaft **Schaltplanmakro (ID 22145)**; zusätzlich normspezifisch IEC (22870), NFPA mm
  (22871), NFPA inch (22872), GB/CCC (22873), GOST (22874) und „Schaltplanmakros für Firmenstandard“ (22882, bis zu
  zehn). Reihenfolge beim Einfügen: Firmenstandard → bevorzugte Norm → 22145. Makros tragen „Makro: Norm“ (23016)
  und „Makro: Firmenstandard“ (23017); Prüflauf 501037 prüft die Passung. Spalten der Funktionsvorlagen heißen jetzt
  „Symbol (Kompatibilität)“ / „Symbolmakro (Kompatibilität)“.
  Quelle: https://www.eplan.help/en-US/Infoportal/Content/Plattform/2023/Content/htm/news_p_makros_normenfuermakros_351772.htm
- Bei der SPS-Schaltplanerzeugung kommt die Makrodatei des Kastens vom Hauptartikel und muss alle benötigten
  Darstellungsarten enthalten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_makrosschaltplanerzeugen.htm

**Bedeutung für QET:** Beim EDZ-Import das im Artikel genannte Schaltplanmakro bevorzugen (IEC-Variante vor NFPA/GOST,
weil die Pläne IEC-Stil sind) statt irgendeines `.ema` im Archiv. Heute importiert der Fork **alle** `.ema` im
Archiv (Fork-Code); Vermutung: die Zuordnung aus `part.xml` (Eigenschaft 22145 bzw. 22870) zu lesen würde das
richtige Hauptmakro markieren.

## 13. Artikel-Import/-Export, EDZ

- Import-Dialog „Datensätze importieren“: Dateityp wählbar (u. a. CSV, VDMA 66413-XML für Sicherheitskennwerte),
  Schema für Feldzuordnung; Modi: nur neue hinzufügen / nur vorhandene aktualisieren / beides. Attribute und freie
  Eigenschaften werden ergänzt, **Funktionsschablonen ersetzt**.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artikelimport.htm
- Export: CSV (getrennt nach Adressen, Anschlüssen, Artikeln, Bohrbildern, Zubehör), Text (Konfiguration
  textimportexport.cfg), XML, **EDZ (Data-Portal-Austauschformat)**; „Referenzierte Daten vollständig exportieren“
  nimmt Unterbaugruppen/-module mit; benutzerdefinierte Eigenschaften nur bei EDZ und XML.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artikelexport.htm

**Bedeutung für QET:** EDZ = Artikeldaten (+ referenzierte Makros/Bilder, Vermutung aus der Praxis des Forks:
`EdzArchive` entpackt `part.xml` und `.ema`). Für den QET-Import ist das EDZ die vollständigste Quelle: Artikel,
Funktionsschablonen und Herstellermakro in einem.

## 14. Data Portal

Offizielle Hilfe:
- Import in die EPLAN-Plattform über „Import part“ aus Teileliste, Detailansicht oder Downloadliste; Importtypen: nur
  neue Teile / nur vorhandene aktualisieren / aktualisieren und neue hinzufügen / **ersetzen** und neue hinzufügen;
  Umfang und Schema für geschützte Felder wählbar. Voraussetzung: Anmeldung in der Eplan Cloud in einer Organisation,
  nur aus der installierten Plattform.
  Quelle: https://www.eplan.help/en-us/Infoportal/Content/EDP_Cloud/Content/htm/DataPortalCS_h_PartsImport.htm
- **DXF-Download** im Browser für Artikel mit DXF-Merkmal: ZIP je Artikel (Name aus Artikelnummer), mehrere über den
  Warenkorb als `EDataPortalDXF.zip`; Inhalt beschreibt eine Readme.txt im Archiv; DXF ohne Logik; Gastzugang genügt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/EDP/Content/htm/EPLANDataPortalHelp_h_dxfdownload.htm
- Data-Portal-Hilfe (ältere Fassung): Zugang für angemeldete EPLAN-Kunden, volle Funktion mit passender Lizenz;
  Einstellungen inkl. Aktualitätsprüfung heruntergeladener Artikel.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/EDP/Content/htm/EPLANDataPortalHelp_k_start.htm
- Gliederung der Cloud-Hilfe: Grundlagen, Artikel suchen, importieren, Details, anfragen, Release Notes.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/EDP_Cloud/Content/htm/DataPortalCS_k_home.htm

Aus der eigenen API-Notiz (Daten aus der OpenAPI-Spezifikation des Portals, nicht aus der Hilfe):
- Server `https://dataportal.eplan.com/api`, JSON:API (`application/vnd.api+json`). `GET /parts?search=…&filter[manufacturer]=…`
  und `GET /partscheck/{nummer}` gehen ohne Token; `relationships.data_types` enthält `dxf_data`, wenn ein DXF-Paket
  existiert; `Accept-Language: de_DE` liefert deutsche Katalognamen; `filter[features]` lehnt der Server ab.
- `GET /download/dxf_data/part/{id}` liefert `MAC_<makro>.zip`, braucht aber Token (`Bearer PAT:<token>` aus dem
  ePulse-Profil oder kurzlebiges JWT); 404 = kein DXF oder Hersteller verbietet. Laut Spezifikation kein PAT für die
  „ePulse Community“-Organisation. Rate-Limits nicht dokumentiert.
  Quelle: eigene Notiz `side-session-notes\…\dataportal-api.md`, dort: https://dataportal.eplan.com/api/doc
- Paketaufbau laut Fork-Code: `commercialdata.csv` (Artikeldaten, eine Zeile je Artikel) und
  `dxf/<makro>/<darstellung>/…_<Variante><Darstellung>.dxf` (Fork-Code `eplanpartimporter.cpp`).

**Bedeutung für QET:** Data-Portal-DXF ist Stufe „Grafik + Artikeldaten ohne Logik“; besser als PDF-Nachzeichnen,
schlechter als ein `.ema` aus dem EDZ. Für Anschlüsse und Kanäle bei DXF-Importen immer gegen Datenblatt bzw.
Funktionsschablonen prüfen. Ein Token gehört nur in den lokalen QET-Einstellungsspeicher, nie in Dateien oder Logs.

## 15. Quellen

- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_h_makroserzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_k_daten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_k_makrokaesten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_r_makrokasten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_r_makrokasteneinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_d_makrosvorbereiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_k_makroprojekte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_h_platzhalterobjekteerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_k_platzhalteraufbauarbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_h_makrosauswaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/macrosgui_h_makrosausdxf.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_darstellungsarten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_makrosschaltplanerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_abbuchverhalten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_kanalorientiertarbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsanschluesse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_untergeraete.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_importbuskonfig.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_symbolischeadressen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_k_arbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_h_mitsymbolvariantenarbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_symbole.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_funktionsdefinitionsbibliotheken.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_h_artikelanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_zubehoer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_funktionsschablone.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_r_technischedaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artikelimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsmanagementgui_d_artikelexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/sparecomponent_k_datenvonnichtplatziertenfunktionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2024/Content/htm/articlesgui_h_funktionsschablonenzuweisen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/plcgui_k_ansichten.htm
- https://www.eplan.help/en-US/Infoportal/Content/Plattform/2023/Content/htm/news_p_makros_normenfuermakros_351772.htm
- https://www.eplan.help/en-us/Infoportal/Content/EDP_Cloud/Content/htm/DataPortalCS_h_PartsImport.htm
- https://www.eplan.help/de-de/Infoportal/Content/EDP_Cloud/Content/htm/DataPortalCS_k_home.htm
- https://www.eplan.help/de-de/Infoportal/Content/EDP/Content/htm/EPLANDataPortalHelp_h_dxfdownload.htm
- https://www.eplan.help/de-de/Infoportal/Content/EDP/Content/htm/EPLANDataPortalHelp_k_start.htm
- https://www.eplan.help/techtipps/de-DE/SPS/TechTip-Configuring-PLC-devices-with-device-description-files.pdf
- https://dataportal.eplan.com/api/doc (über eigene Notiz dataportal-api.md)
