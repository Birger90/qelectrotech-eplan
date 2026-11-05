# Konzept: Schaltschrankaufbau bis zur gefertigten Tafel – EPLAN-Ablauf in QElectroTech

Ziel: Montageplatten, Türen und Wände so planen, dass daraus direkt gefertigt werden kann – Bohrungen und Ausschnitte
für Geräte, Tragschienen und Verdrahtungskanäle, Zuschnittliste für Schienen und Kanäle, NC-taugliche DXF je Tafel und
eine Bohrschablone 1:1. Vorbild ist der Ablauf in EPLAN (2D-Schaltschrankaufbau + Pro Panel/NC), umgesetzt in 2D, weil
QET kein 3D hat. Grundlage: EPLAN-Plattform-Hilfe 2.9, zusammengefasst im Skill `eplan-wissen`
(`referenz/schaltschrankaufbau.md`, `referenz/3d-nc-kupfer.md`, `referenz/xml-datenformate.md`).

---

## 1. Der EPLAN-Ablauf in sieben Schritten

| # | Schritt bei EPLAN | Was dabei an Daten entsteht |
|---|---|---|
| 1 | **Artikel mit Montagedaten**: Breite/Höhe/Tiefe, Einbauabstände, Grafikmakro, je Montagevariante ein **Bohrbild**, Produktgruppe (Montageplatte, Tragschiene, Kanal), Lieferlänge | Artikeldatenbank: `construction` mit `drillingPosition` (Bohrtyp, Ø, Lage, Wiederholung) |
| 2 | **Schrank / Montageplatte platzieren** (Artikel oder frei aufgezogen) | Montagefläche mit Größe, Nullpunkt links unten, Materialstärke; Eigenschaft „Mechanische Bearbeitung“ → ein NC-**Feld** |
| 3 | **Längenvariable Teile** aufziehen: Tragschienen, Verdrahtungskanäle, C-Profile – Start/Ende, Einpassen an Kante oder Nachbarkanal, gleichmäßig verteilen | Länge je Stück; aus dem Bohrbild des Artikels entstehen **automatisch Durchbrüche** in der Trägerfläche (Wiederholabstand, Endabstand, Nullschnitt) |
| 4 | **Geräte platzieren** aus dem Navigator (BMK aus dem Schaltplan, Status platziert/offen) auf Tragschiene oder Fläche: Anfasspunkt (9 Punkte), Drehung, Montagevariante | Position/Winkel je Gerät; „Verwendetes Bohrbild“; logische Hierarchie Gerät → Schiene → Platte |
| 5 | **Durchbrüche von Hand** (Tür: Taster Ø22,5, Displayausschnitt; Wand: Kabelverschraubungen) und **Bohrsperrflächen** | Durchbrüche: Bohrung, Gewinde, Rechteck (eckig/gerundet/gefast), Langloch, Sechseck, Achteck, Kontur |
| 6 | **Kontrolle**: Kollision/Einbauabstände, 2D-Bohransicht mit Bemaßung, Durchbruchlegende, Montageliste, Prüfläufe | Fertigungsunterlagen auf Seiten |
| 7 | **NC-Export je Feld**: DXF (Ebenen je Bohrtyp/Bauteil), Bohrschablone PDF 1:1, Perforex/Steinhauer, Athex (Tragschienenbestückung) – mit Bohrausnahmen und Werkzeugwahl | Dateien für Maschine/Werkstatt; Abschlussliste exportiert / nicht exportiert mit Grund |

Wichtig für die Umsetzung: Ein Durchbruch gehört immer zum **Trägerbauteil** mit „Mechanischer Bearbeitung“ – ein Kanal
auf der Montageplatte erzeugt Löcher in der Platte, ein Gerät auf der Tragschiene erzeugt keine (es rastet auf).

---

## 2. Abbildung in QET – Grundsatz

QET arbeitet in Blättern mit Szenenkoordinaten. EPLANs **2D-Schaltschrankaufbau** ist das passende Gegenstück: eine
Seite vom Typ „Schaltschrankaufbau“, auf der Montageplatten maßstäblich liegen. Was fehlt, ist die Bedeutung hinter der
Grafik: Millimeter, Flächen, Längen, Bohrbilder, Hierarchie. Die Grafik bleibt QET, die Fertigungsdaten kommen als
neues Datenmodell dazu.

### 2.1 Datenmodell

| EPLAN | QET (neu) | Speicherung |
|---|---|---|
| Seitentyp Schaltschrankaufbau, Maßstab | Blatteigenschaft `layout_scale` (mm je Szeneneinheit, z. B. 0,4 = 2,5 px/mm wie der DWG-Import) | `<diagram layout_scale="0.4">` |
| Montagefläche / Montageplatte / Tür / Wand | neues Objekt **Montagefläche**: Breite × Höhe (mm), Nullpunkt (Ecke), Materialstärke, „Mechanische Bearbeitung“ ja/nein, Feldname, optional Artikel | `<mountingsurface>` im Blatt, Rechteck mit Daten; verknüpft mit BMK (z. B. `+CA-MP1`) |
| Längenvariables Teil (Tragschiene, Kanal, C-Profil) | neues Objekt **Profil**: Start/Ende auf einer Montagefläche, Breite des Profils, Länge (mm, abgeleitet), Artikel, Bohrbild, Nullschnitt | `<rail type="din|duct|cprofile">` – gezeichnet wie ein Kabel/Leiter als eigenes Grafikobjekt |
| Bohrbild am Artikel (`construction` / `drillingPosition`) | **Bohrbild** in der Bauteil-Definition (.elmt), Maße in mm relativ zum Anfasspunkt, je Montagevariante | `<drillingpatterns><pattern variant="A"><hole type="thread" d1="4" x="…" y="…" angle="0" repeat="0" end="0" nth="0"/></pattern></drillingpatterns>` – 1:1 die EPLAN-Attribute |
| Montagedaten (B/H/T, Einbauabstände) | Bauteil-Informationen `width_mm`, `height_mm`, `depth_mm`, `clearance_*` | `<elementInformations>` |
| Platzierung (Position, Winkel, Variante, Träger) | am platzierten Bauteil: `mount_x_mm`, `mount_y_mm`, `mount_angle`, `mount_variant`, `mount_parent` (uuid der Fläche bzw. Schiene) | Bauteil-Informationen – aus der Lage auf dem Blatt berechnet, damit Grafik und Daten nicht auseinanderlaufen |
| Durchbruch von Hand | neues Objekt **Durchbruch** (Typ, Maße, Anfasspunkt) | `<cutout>` auf der Fläche |
| Bohrsperrfläche | neues Objekt **Sperrfläche** | `<keepout>` |
| Mechanische Bearbeitung / Feld | Eigenschaft der Montagefläche | – |

Der Fork hat dafür schon die Vorbilder: Kabel sind ein eigenes Objekt mit Teilen, Adern und Daten (`sources/cable/`),
Bauteile tragen Zusatzdaten im Exemplar (E/A-Tabelle, Kontaktgruppen). Profil, Montagefläche und Durchbruch folgen
demselben Muster.

### 2.2 Bohrbilder aus den Herstellerdaten

- EPLAN-Artikeldaten (EDZ, Data Portal) enthalten die Bohrbilder als `*.construction.xml`
  (`<construction P_PART_CONSTRUCTION_NAME>` → `<drillingPosition type dimension1..3 xpos ypos angle spacingrepeat
  spacingend drillnthhole outlinename>`) und am Artikel den Verweis (`constructionPosition name offsetx offsety`,
  `P_ARTICLE_REF_CONSTRUCTION_NAME`). Beispiel LAPP CubeFrame 16: vier Bohrungen Ø4,5 an (8,5|17,45), (8,5|49,45),
  (111,5|17,55), (111,5|49,55); LAPP M6x1,0_T: Gewinde M6 an (3|3).
- Bohrtypen 1:1: 1 Bohren · 2 Gewinde · 3 Rechteck (Subtyp eckig/gefast/gerundet) · 4 Langloch · 5 Sechseck ·
  6 Achteck · 7 Kontur. `spacingrepeat`/`spacingend`/`drillnthhole` beschreiben Lochreihen auf längenvariablen Teilen.
- Der EDZ-Import liest heute weder Bohrbilder noch Abmessungen – das ist die erste Lücke.
- **Eigene Bibliothek** für Teile ohne Herstellerbohrbild, im selben Format: Drucktaster/Not-Halt/Leuchtmelder Ø22,5
  (mit Verdrehschutz), Kabelverschraubungen M12–M63 (Durchgang oder Gewinde), Befestigung M4/M5/M6, Tragschiene TS35
  (z. B. M5 alle 100 mm), Kanäle (je Breite Befestigungsreihe), Display-Ausschnitte nach Datenblatt.

---

## 3. Arbeitsablauf in QET (1:1 zu EPLAN)

1. **Blatt „Schaltschrankaufbau“ anlegen**, Maßstab wählen (1:5, 1:10 …).
2. **Montagefläche setzen**: Artikel (Montageplatte, Tür, Seitenwand) oder frei aufziehen, Maße in mm eingeben.
   Nullpunkt und „Mechanische Bearbeitung“ einstellen.
3. **Tragschienen und Kanäle aufziehen** wie ein Kabel: Klick Start, Klick Ende, Fang an Plattenkante, Raster in mm,
   Länge wird angezeigt und kann eingegeben werden. Befehle **Einpassen** (bis Kante/Nachbarkanal), **Gleichmäßig
   verteilen**, **Ausrichten**. Aus dem Bohrbild des Artikels entstehen die Befestigungslöcher in der Platte
   automatisch, mit Nullschnitt (von welcher Seite die Lochreihe gerechnet wird) und Endabstand.
4. **Geräte platzieren** aus einem **Aufbau-Navigator**: alle BMK des Schaltplans mit Artikel, Status
   offen/platziert, Filter. Ziehen auf Schiene (rastet, keine Löcher) oder Fläche (Bohrbild der gewählten
   Montagevariante wird zu Durchbrüchen). Anfasspunkt 9-fach, Drehung, Variante. Die Grafik kommt aus dem
   Artikelplatzierungs-Makro, sonst Rechteck aus B × H – wie bei EPLAN.
5. **Durchbrüche von Hand** und **Sperrflächen** (Tür, Wand, Flansch).
6. **Prüfen**: Kollision und Einbauabstände (Überlappung färben), Löcher außerhalb der Fläche, Löcher übereinander,
   Löcher in Sperrfläche, Geräte ohne Bohrbild auf Fläche, BMK des Schaltplans nicht platziert.
7. **Fertigungsunterlagen erzeugen**:
   - **2D-Bohransicht** je Fläche als eigenes Blatt (nur Umriss, Durchbrüche, Bemaßung), automatisch aktualisiert.
   - **Durchbruchlegende** als Tabelle (Typ, X/Y, Größe) – wie die Kabelliste als QET-Tabelle.
   - **Montageliste / Zuschnittliste**: Tragschienen und Kanäle mit Länge und Stückzahl je Artikel, Geräte je Schiene
     in Reihenfolge (Grundlage für Bestückung).
   - **NC-DXF je Feld**: mm, Nullpunkt der Fläche, Ebenen `UMRISS`, `BOHRUNG_D<Ø>`, `GEWINDE_M<n>`, `AUSSCHNITT`,
     `KONTUR`, optional `BESCHRIFTUNG`; Bohrungen als Kreise (Mittelpunkt + Radius), Gewinde als Kreis mit
     Kernlochdurchmesser und Ebenenname, Ausschnitte als geschlossene Polylinien. Abschlussbericht: exportierte und
     nicht exportierte Teile mit Grund.
   - **Bohrschablone PDF 1:1** (Ausdruck auf die Platte legen, Kürzel D/M/B/H/X/R wie EPLAN).
   - **Bohrausnahmen / Werkzeugliste**: Gewinde → Kernloch oder Durchgangsbohrung, nicht vorhandener Bohrer →
     nächster, ab Ø x fräsen; je Ausgabe einstellbar.
   - Später: Rittal Perforex/Secarex, Steinhauer, Athex (Tragschienenbestückung als XML).

---

## 4. Umsetzung im Fork – Bausteine

| Baustein | Inhalt | Ort im Code (Vorbild) |
|---|---|---|
| A. Maßstab | Blatteigenschaft `layout_scale`, mm-Raster, Anzeige der Lage in mm in der Statusleiste | `BorderTitleBlock`/`Diagram`-Eigenschaften |
| B. Bohrbild im Bauteil | `<drillingpatterns>` in .elmt lesen/schreiben, Varianten; Anzeige im Elementeditor (Löcher als Hilfsgrafik) | `ElementData`, `Element::fromXml` |
| C. EDZ-/Data-Portal-Import | `construction.xml` + `constructionPosition` + Abmessungen + Produktgruppe → B; Montageplatten/Schienen/Kanäle als solche erkennen | `sources/import/edz/` |
| D. Montagefläche | Objekt mit Rechteck, Daten, Kontextmenü; Kinder (Profile, Geräte, Durchbrüche) über Lage oder Zuordnung | neues `sources/panel/` nach Muster `sources/cable/` |
| E. Profil (Schiene/Kanal) | Aufziehen, Länge, Einpassen, Verteilen, Bohrreihe aus Bohrbild + Nullschnitt + Endabstand | wie `CablePart`/`DiagramEventAddCable` |
| F. Geräteplatzierung | Anfasspunkt, Drehung, Variante, Fang auf Schiene; Platzierungsdaten berechnen | Element-Platzierung + Navigator (Dock) |
| G. Durchbruch / Sperrfläche | Objekte mit Typ und Maßen | `QetShapeItem` erweitern oder neu |
| H. Durchbruchberechnung | Alle Löcher einer Fläche sammeln: Geräte (auf Fläche), Profile, Hand-Durchbrüche; Sperrflächen, Bohrausnahmen | reine Datenklasse, testbar ohne GUI |
| I. Ausgaben | NC-DXF (über `createdxf`, aber in mm und mit Ebenen), Bohrschablone PDF, Bohransicht-Blatt, Legende/Zuschnittliste als Tabellen | `dxfexport`, `QetGraphicsTable` + Modell wie `CableListModel` |
| J. Prüfungen | Kollision, außerhalb, Sperrfläche, ohne Bohrbild, unplatziert | wie die Projektprüfung |
| K. Skript-API / MCP | Flächen, Profile, Durchbrüche per Skript anlegen und exportieren – damit Pläne automatisch nachgebaut werden | `qetscriptapi`, `qet_mcp.py` |

---

## 5. Reihenfolge – damit früh eine echte Tafel herauskommt

**Stufe 1 – erste fertigbare Platte**
A Maßstab · D Montagefläche · G Durchbruch · B Bohrbild im Bauteil · H Berechnung · I NC-DXF + Bohrschablone.
Tragschienen und Kanäle zunächst als Profil mit fester Länge per Dialog (Start/Ende in mm), Lochreihe aus dem
Bohrbild. Ergebnis: Montageplatte mit allen Löchern für Schienen, Kanäle und direkt geschraubte Geräte als DXF in mm.

**Stufe 2 – Arbeiten wie in EPLAN**
E Profile aufziehen/einpassen/verteilen · F Navigator und Platzieren mit Anfasspunkt/Variante · J Prüfungen ·
Durchbruchlegende, Zuschnitt-/Montageliste, Bohransicht-Blatt.

**Stufe 3 – Daten aus der Herstellerwelt und Maschinen**
C Bohrbilder/Abmessungen aus EDZ und Data Portal · eigene Bohrbild-Bibliothek · Tür-/Wandausschnitte ·
Perforex/Steinhauer/Athex · K Skript/MCP.

---

## 6. Offene Punkte / Entscheidungen

- **Welche Maschine/Werkstatt** verarbeitet die Daten? Bestimmt Ebenennamen, Gewindedarstellung (Kernloch oder
  Nenndurchmesser) und ob ein Maschinenformat nötig ist.
- **Befestigungsraster von Tragschienen und Kanälen**: Herstellerdaten (Phoenix, Rittal, Hager/Tehalit) prüfen, ob
  Bohrbilder mitkommen; sonst Werte festlegen (z. B. TS35 M5 alle 100 mm, Endabstand 25 mm).
- **Gewinde oder Durchgangsbohrung + Mutter** je Platte (Materialstärke, verzinkt/Edelstahl).
- **Bestehende Aufbaupläne** (z. B. aus PDF nachgezeichnet) brauchen für die Fertigung echte Objekte: Positionen
  lassen sich über den bekannten Maßstab aus der Zeichnung übernehmen, die Bedeutung (Fläche, Schiene, Gerät) muss
  zugeordnet werden.
- Geräte auf Tragschienen erzeugen keine Löcher; entscheidend sind Schienen, Kanäle, direkt geschraubte Geräte
  (Netzteile, Umrichter, Trafos) und alles in Tür und Wänden.
