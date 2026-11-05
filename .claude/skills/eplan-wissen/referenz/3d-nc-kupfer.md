# EPLAN Pro Panel: 3D-Montageaufbau, Betriebsmittellogik, Kontureditor, Kupfer, NC, STEP/Design Space, externe Bearbeitung (PXF)

Knappe Referenz der Pro-Panel-Themen der EPLAN-Plattform-Hilfe 2.9 (`cabinetgui_*`, `ged3dmateeditorgui_*`,
`contoureditorgui_*`, `copper_*`, `ncgui_*`, `xdesignspaceimportgui_*`, `xstepgui_*`) sowie der Schnittstelle
„Eigenschaften extern bearbeiten“ (`pxfinterface_*`). Für den QET-Nachbau zählen vor allem: was im PDF als 2D-Ableitung
des 3D-Modells erscheint (Modell-, Bohr-, Kupferansicht, Legenden; siehe `schaltschrankaufbau.md`) und welche Daten
(Artikel, Maße, Bohrbilder) an Artikeln hängen. Eigene Worte, jede Aussage mit Quelle; **(Vermutung)** = nicht belegt.

## Inhalt

1. Bauraum, Navigatoren, logische Hierarchie
2. Platzieren im Bauraum: Geräte, Montageplatten, Schränke, längenvariable Teile, Hilfen
3. Betriebsmittellogik („Mate“): Einbaufläche, Montagefläche/-punkt/-linie/-raster, Anfasspunkt, Bezugspunkt
4. 3D-Makros (*.ema/*.ems) und STEP-Import
5. Zusammenhang mit dem Schaltplan (Hauptbauteile, Verbindungsvorschau, Anschlussbild)
6. Sammelschienensysteme und Kupferbauteile (Sammelschiene gebogen, Strang, Abwicklung)
7. Kontureditor (Extrusion *.fc2, NC-Kontur *.fc1, Verlauf *.fp1)
8. Durchbrüche, Bohrbilder, Sperrflächen, Feldgröße, Nullschnitt
9. NC-Fertigungsdaten (Perforex, Steinhauer, DXF, Bohrschablone, Athex, Kupfer-NC)
10. Thermische Auslegung, Verlustleistung
11. Schrankmigration TS8 → VX25, Automatische Interpretation
12. STEP-/VRML-Export, Design Space Exchange
13. Externe Bearbeitung (PXF-Schnittstelle)
14. Bedeutung für QET
15. Quellen

---

## 1. Bauraum, Navigatoren, logische Hierarchie

- Der **Bauraum** ist die seitenunabhängige 3D-Umgebung (Standard: isometrisch mit Shading; Achsen X rot, Y grün,
  Z blau). Bauräume werden im **Bauraum-Navigator** angelegt (Baum/Liste, Filter); löscht man einen Bauraum, bleiben
  die Betriebsmittel im Projekt. Der **3D-Montageaufbau-Navigator** listet alle Betriebsmittel mit Artikel zum
  Platzieren (immer kennbuchstabenorientiert; ohne BMK eigener Knoten; Icon für platziert mit/ohne Schaltplan-Gegenstück;
  Mehrfachauswahl und ganze Knoten platzierbar).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_navigator3dschaltschrankaufbau.htm
- **Logische Struktur**: alle Bauteile hängen hierarchisch voneinander ab (Tragschiene verschieben/löschen → Geräte
  darauf mit). Die Hierarchie wirkt in Auswertungen (Montageliste). Im Makroprojekt lässt sie sich per Drag & Drop
  umhängen (z. B. Tür an Rahmenprofil statt Schrank).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_prinzip.htm
- Montageplatten, Türen, Wände, Profile und Geräte mit 3D-Makro haben einzeln wählbare **Montageflächen**; Tragschienen
  nur eine interne (nicht im Baum). Bauraumeigenschaften: Strukturkennzeichen, Name (fortlaufende Nummer), Beschreibung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_bauraumeigenschaften.htm

## 2. Platzieren im Bauraum

- **Montageflächen aktivieren**: automatisch beim Überfahren (unverdeckte Flächen) oder gezielt aus dem Navigator
  (Gehe zu (Grafik) blendet nur das Bauteil und seine Bestückung ein).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_aktivierenautomatisch.htm
- **Gerät platzieren** (Einfügen > Gerät): Darstellung aus 3D-Makro, sonst Quader aus Höhe/Breite/Tiefe, sonst Dialog
  Abmessungen. Anfasspunkt mit [A] über neun Punkte der Grundfläche; Platzieroptionen (Drehwinkel 0/±45/±90/±135/180° oder
  frei, um Anfasspunkt und Z-Achse; bei Schränken und längenvariablen Teilen gesperrt). Mehrere Montagevarianten →
  je Variante ein Bohrbild, Auswahl beim Platzieren, gespeichert als „Verwendetes Bohrbild“ der Artikelreferenz.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_geraetplatzieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_platzieroptionen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_abmessungen.htm
- **Montageplatte** (Einfügen > Montageplatte): Vorschau in Artikelgröße, vier Anfasspunkte (Standard unten links).
  **Schaltschrank**: Typ „Korpus“ (Gehäuse ein Körper, z. B. AE/CM) oder „Einzelteil“ (Profile, Wände, Türen einzeln);
  Bestandteile mit „Bauteil ist unverschiebbar am übergeordneten Bauteil befestigt“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montageplattenplatzieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_schaltschrankplatzieren.htm
- **Längenvariable Bauteile** = Tragschienen, Verdrahtungskanäle, C-Profilschienen, benutzerdefinierte Schienen: Start-
  und Endpunkt aufziehen oder Länge eines vorhandenen Teils übernehmen (dann auch mittig zwischen zwei Teilen);
  Anfasspunkt Mitte/Oben/Unten; mit Bohrbild am Artikel entstehen automatisch Durchbrüche. Nachträglich: Länge ändern
  (einzeln), **Einpassen** an Plattenkante oder Nachbarkanal, Gleichmäßig verteilen (Kanäle: Abstand der Mittelachsen),
  Ausrichten horizontal/vertikal (nur gemeinsame Fläche/Schiene).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_varbtplatzieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_laengeaendern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_einpassen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_gleichmaessigverteilen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_horverausrichten.htm
- Hilfen: **Einbauabstände** (Artikel, Montagedaten, Breite/Höhe/Tiefe; als transparenter Körper einblendbar,
  Anfasspunkt wahlweise auf Einbauabstand bezogen), **Kollisionskontrolle** (färbt Überlappungen, wertet
  Einbauabstände aus, gewollte Kollisionen wie Monitor in Tür ausgenommen), Raster A–E (2D/3D getrennt), Hilfslinien,
  Messen (auf Montagefläche projiziert), Duplizieren, Montagefläche tauschen, Platziersperrflächen (z. B. für
  Kühlaufsätze von oben).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_einbauabstaende.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_kollisionskontrolle.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_raster.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_hilfslinienzeichnen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_messen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_objekteduplizieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montageflaechetauschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_sperrflaechenzeichnen.htm
- **Artikelplatzierung aktualisieren** nach Artikeltausch oder Makroänderung (Maße, Makro-/Bildname, neue Montagepunkte);
  wirkt auf Knoten und alles darunter. **Artikel tauschen (Schaltschrank)** ersetzt den Schrank samt Makro, Bestückung
  bleibt; Zubehör nur, wenn auch für den neuen Schrank vorhanden.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_artikelabmessungenaktualisieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_artikeltauschen.htm
- Ansicht: Zoom, Blickpunkte orthogonal/isometrisch wie bei Modell-Ansichten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_bauraumansichten.htm

## 3. Betriebsmittellogik („Mate“)

- Betriebsmittellogik = Eigenschaften, mit denen 3D-Objekte platziert werden, andere tragen und sich in die Struktur
  einordnen. Pflicht ist nur die **Einbaufläche**. Grafik-Definition: Importieren, Vereinigen, Drehen um Achse.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_betriebsmittellogik.htm
- Elemente (Bearbeiten > Betriebsmittellogik; anzeigen mit Ansicht > Montagehilfen):
  - **Einbaufläche**: Fläche, mit der das Objekt aufsitzt, bestimmt Einbautiefe; neun automatische Anfasspunkte; nur im
    Makroprojekt definier-/bearbeitbar.
  - **Montagefläche**: Fläche zum Bestücken (Nullpunkt links unten, umgebendes Rechteck; X/Y-Achse ausrichtbar; Größe
    änderbar, damit der Nullpunkt nicht an Anbauteilen hängt; mehrere Flächen einer Ebene zusammenfassbar).
  - **Montagepunkt**: Fangpunkt mit Z-Richtung einer gewählten Fläche, XY in 90°-Schritten oder frei drehbar.
  - **Montagelinie**: Linie auf nicht-längenvariablen Teilen (z. B. C-Profil) mit Ausrichtung, wie Tragschiene nutzbar.
  - **Montageraster**: Lochraster (Abstand und Anzahl in X/Y vom Flächennullpunkt) – Platzierung nur auf Schnittpunkten.
  - **Anfasspunkt**: Führungspunkt beim Platzieren; benutzerdefinierter heißt in den Platzieroptionen „Grafikmakro“.
    Registerkarte Logik: welche Montage-/Bezugspunkte er annehmen darf (Standard: alle).
  - **Bezugspunkt**: feste Punkte in Schränken für Zubehör (automatisch an Rahmenprofilen/Gehäuse bei Schränken aus der
    Artikeldatenbank; benutzerdefiniert im Makro; „Bezugspunktschema übertragen“ für Rittal TS8/AE/CM aus STEP).
  - Farben: Anfasspunkt benutzerdefiniert orange / Standard rot, Montagepunkt benutzerdefiniert grün / Standard blau,
    Bezugspunkt cyan.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_betriebsmittellogik.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_einbauflaeche.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montageflaechedefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_mfgroesseaendern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_xyachseausrichten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_zielmatesdefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montageliniedefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montagerasterdefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_anfasspunktdefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_bezugspunkte.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_bezugspunktdefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_bezugspunktschemauebertragen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montagehilfenanzeigen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ged3dmateeditorgui_r_anfasspunkt_logik.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ged3dmateeditorgui_d_montageraster.htm
- Mate-Editor-Dialoge (Montagepunkt/-linie, Anfasspunkt/Montageraster: Name, Beschreibung; Montageraster Format:
  Abstand und Linienzahl je Richtung) haben keine weiteren Regeln.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ged3dmateeditorgui_d_montagepunkt_montagelinie.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ged3dmateeditorgui_r_anfasspunktmontageraster.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ged3dmateeditorgui_r_montageraster_format.htm

## 4. 3D-Makros und STEP-Import

- 3D-Makros: Fenstermakro *.ema, Symbolmakro *.ems, Darstellungsart fest „3D-Montageaufbau“, Varianten (Standard
  Variante A) durch Drehen oder anderen Anfasspunkt. Im Makroprojekt ist jedes Makro ein eigener Bauraum (Eigenschaften
  Makro: Name/Beschreibung/Variante/Version/Quelle; Registerkarte Makro-Definition für mehrere Makros aus einem
  Bauraum). Makros liegen in X/Y-Ausrichtung; Platzhalterobjekte möglich (Anker im Navigator).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_3dmakros.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_3dmakroerstellen.htm
- **2D und 3D trennen**: 3D-Makro mit Darstellungsart „3D-Montageaufbau“ in die Artikelverwaltung, Registerkarte
  **Montagedaten > Grafikmakro**; 2D-Makro (allpolig, Übersicht …) wie bisher unter **Technische Daten > Makro**.
  Namensvorschlag mit Endungen `_3D` / `_2D`. Im 3D wird zuerst das Grafikmakro geprüft, dann das technische Makro;
  ohne Makro entsteht ein Quader aus H/B/T.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_3dmakros.htm
- Platzieren: Einfügen > Fenster-/Symbolmakro; [A] wechselt Anfasspunkt, Optionen > Drehwinkel ändern dreht 90°;
  bei neun Standard- plus einem eigenen Anfasspunkt bis zu 40 Lagen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_3dmakroplatzieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_3dmakrodrehwinkelaendern.htm
- **STEP-Import** (Bauraum > Importieren (3D-Grafik)): nur AP 203 oder AP 214; neuer Bauraum mit Dateinamen,
  automatisch Funktionsdefinitionen (Voreinstellung „Artikelplatzierung, normales Bauteil“). Einstellungen:
  Detaillierungsgrad (5 Stufen), **Bauteile separieren** (lose Körper → mehrere oder ein Logikbauteil; CAD-Baugruppen
  immer getrennt), Normalenvektoren korrigieren (nur ohne Separieren). Danach Vereinigen (macht Makros nicht kleiner,
  bei vielen gleichen Teilen eher größer), Drehen um Achse, Logik vergeben.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_3dgrafikimportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xstepgui_d_einstellungen3dimport.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_vereinigen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_drehenxyz.htm

## 5. Zusammenhang mit dem Schaltplan

- Platzierung und Schaltzeichen gehören über das **vollständige BMK** zusammen. **Hauptbauteile aktualisieren** (Bauraum
  im Navigator markieren) ersetzt die Artikelreferenzen der Hauptfunktion durch die der Platzierungen; gilt für
  Funktionsdefinitionen Fluid-Betriebsmittel, Leitungsverteiler, Gerätekasten, Klemme, Klemmenleiste, SPS-Karte,
  Sammelschiene, Stecker, normales Bauteil.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_hauptbauteileaktualisieren.htm
- **Verbindungsvorschau**: beim Platzieren aus dem 3D-Montageaufbau-Navigator zeigt eine blaue Linie, mit welchem schon
  platzierten Gerät das neue elektrisch verbunden ist (kurze Verdrahtung).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_verbindungsvorschau.htm
- **Anschlussbild**: Anschlüsse kommen aus dem Artikel; „Lokales Anschlussbild“ kopiert sie an die Platzierung (dann
  einzeln änderbar, auch grafisch neu definierbar) und lässt sich als neues Anschlussbild „Neu_n“ zurück in die
  Artikelverwaltung übertragen. **Verlegerichtung** aller Anschlüsse markierter Geräte änderbar (z. B. Motorschutz auf
  Sammelschienenadapter: Verdrahtung nach unten in den Kanal).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_anschlussdefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_anschlussbildnachartvw.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_anschlussrichtungaendern.htm
- Legendenpositionen werden im Bauraum genauso bearbeitet/nummeriert wie im 2D (ohne Horizontal/Vertikal).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_legendenpositionbearbeiten.htm

## 6. Sammelschienensysteme und Kupferbauteile

- **Sammelschienensystem**: Schienen und Halter in einem Arbeitsgang (Halterzahl einstellbar, auch ohne), nachträglich
  änderbar ohne Verlust der Bestückung; Halter mit Bohrbild; Lieferlängen; Geräte direkt auf Polen. Jeder Pol hat eigene
  Artikelnummer und Länge (Stückliste berücksichtigt das). Anfasspunkt aus Haltermakro bevorzugt, sonst Start links,
  Ende rechts. Unvollständige Artikeldaten → Abbruch.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_sammelschienensystem.htm
- **Sammelschiene (gebogen)** = Artikel mit rechteckigem Querschnitt (Breite/Tiefe in Montagedaten; Mechanik >
  Einzelteil > Sammelschienen > Schiene; Funktionsdefinition „Sammelschiene“) + **Verlauf** (*.fp1) + Biegeradius +
  Hochkant- oder Flachkantbiegung. Artikel mit Extrusionskontur *.fp1 statt Maßen sind dafür unzulässig. Lieferlänge
  begrenzt die Länge (Meldung). Eigenschaft **Vorkonfektioniert** (ID 36011) sperrt Änderungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_kupferanschlusselement.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_kupferartikel.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferartikeldefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferschieneplatzieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_sammelschienegebogen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_start.htm
- Ablauf: Verlauf zeichnen → Schiene platzieren → Länge/Winkel/Biegestellen direkt am Bauteil ändern (abgewickelte
  Gesamtlänge wird laufend neu berechnet) → weitere Schienen (Kupferstrang) → Montageliste → Abwicklung → Maschine.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_arbeitsweise.htm
- **Kupferstrang**: jede Kupferschiene gehört zu genau einem Strang (Navigator „Kupferstränge“; aktiver Strang nimmt neue
  Teile auf, [N] neuer Strang; [Umschalt] wählt Einzelschiene). Verschieben/Kopieren/Duplizieren bilden ggf. neue Stränge;
  **Vereinigen** macht daraus normale Artikelplatzierungen (kein Kupferteil mehr).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_kupferstrang.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_kupferbauteilbearbeiten.htm
- Bearbeiten: Länge ändern (bis Lieferlänge), Winkel ändern (grüner Punkt), Biegestellen ändern (Anfang/Ende bleiben fest),
  neue Biegestelle (flachkant auf Ober-/Unterseite, hochkant auf Seitenflächen, kombinierbar).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferteilbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferteillaengeaendern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferteilwinkelaendern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferteilbiegestelleaendern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferteilneuebiegestelle.htm
- **Abwicklung** (Einfügen > Grafik > Kupferabwicklung, auf Grafikseiten): ein Bauteil je Abwicklung, Draufsicht vor dem
  Biegen; Außenkontur immer Rechteck, Biegungslinien, Biegungsenden, Hinweise mit Winkel/Richtung, Pfeil bei
  Hochkantbiegung; bemaßbar, aktualisierbar, einfrierbar. Abgewickelte Gesamtlänge steht wie bei Kanälen in den
  Artikelreferenzdaten. Nötig ist eine **Biegungstabelle** (Firmeneinstellung Kupferbiegung: je Materialstärke
  Korrekturwerte je Biegewinkel/-radius); fehlt der Eintrag, Status an Registerkarte Biegestellen, danach
  Artikelplatzierung aktualisieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_abwicklung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferabwicklungerstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_einstellungenkupferbiegung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_ansichtabwicklung.htm
- **Bohrbild vererben**: liegt ein Kupferteil parallel auf einem anderen und ein Durchbruch vollständig in der
  Überdeckung, wird er referenziert (Bohransicht, Export); Voraussetzung Benutzereinstellung „Automatischer
  Mehrfach-Durchbruch“ (setzt „Mehrfach-Durchbruch möglich“). Trennen löscht die Referenz.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_bohrbildvererben.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_einstellungenkupferbiegungbenutzer.htm
- Montageliste für Kupferstränge mit „Montageliste erstellen“ (ID 36032): listet alle Kupferteile des Strangs.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_auswertungmontageliste.htm

## 7. Kontureditor

- Aufruf über Dienstprogramme > Stammdaten. Zwei Typen, nachträglich nicht änderbar: **Kontur-Extrusion** (*.fc2,
  Querschnitt benutzerdefinierter Schienen/Kämme, mehrere sich nicht schneidende Konturen erlaubt) und
  **Kontur-NC-Datensatzerzeugung** (*.fc1, genau eine geschlossene Kontur). Dazu **Verlauf Kupfer** (*.fp1, nur Linien
  und Polylinien, zusammenhängend ohne Verzweigung/Kreuzung; Raster vorbelegt 4 mm). Speicherort Makros\<Firmenkennung>.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_verlaufskontur.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_verlaufskonturerstellen.htm
- Zeichenregeln: Maßstab 1:1, nur Linie, Polylinie, Rechteck, Kreis, Kreisbogen, an allen Übergängen geschlossen.
  „Bereinigen“ entfernt Ellipsen, Bézier, 3D-Elemente und löst Blöcke auf. „Kontur prüfen“: unerlaubte Elemente,
  geschlossene Konturen ohne Doppelungen, keine geschachtelten Innenkonturen (NC: genau eine).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_k_grundlagen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_konturenbereinigen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_konturenpruefen.htm
- **Logikelemente** (nur Extrusion, Einfügen > Logik, als Konturdefinitionspunkte mit Zähler und Unterzähler 1/2):
  Führungsschiene (logische Tragschiene auf der Profilvorderseite), Führungsschiene mit Ausrichtung (Pfeil gibt
  Bestückrichtung), Montagefläche (Punktpaar im Uhrzeigersinn außen / gegen den Uhrzeigersinn innen), Montagefläche NC,
  Feldgröße (zu „Montagefläche NC 1.2“ ein „Feldgröße 1.2“), Verdrahtungskamm Vorder-/Rückseite (Flächen für
  Verdrahtungsdurchbrüche und Verlegebereiche).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_k_logikelemente.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_konturdefseigenschaftenbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_durchbrverdrahtungskaemdefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_r_daten.htm
- **Nullpunkt**: bei Extrusionen Anfasspunkt der Schiene; daraus entsteht die Einbaufläche, die auf die Montagefläche
  gesetzt wird. Bei Verläufen liegt im Nullpunkt (roter Kreis) der Anfasspunkt der Kupferschiene.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_k_nullpunkt.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_verlaufskontur.htm
- **NC-Konturdatensatz** (*.fc1): je Maschine (Perforex BC, Perforex LC, Steinhauer) ein Datensatz, mehrere Maschinen
  je Kontur möglich; Startpunkt auf einer Geraden (nicht nah an Bögen/Ecken), Fräserbahn wird gezeichnet; Kreise
  markieren Programmschritte (blau Vorbohrung, grün Fräser oberhalb, rot im Werkstück); Zusatzbohrungen und Stege
  platzierbar; Bohrdurchmesser nur aus der Maschinendefinition.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_ncdatensatz.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_nckonturdatenbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_d_ncdatensatz.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_d_bohrung.htm
- Übrige Seiten: Erstellen (Dialog Kontureigenschaften), Kopieren/Drehen, Auswahldialog mit Vorschau, Anzeige der
  Konturdefinitionspunkte – keine weiteren Regeln.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_konturenerzeugen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_konturenkopieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_d_konturauswahl.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_d_kontureigenschaften.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_r_anzeige.htm

## 8. Durchbrüche, Bohrbilder, Sperrflächen, Feldgröße, Nullschnitt

- **Durchbrüche** sind Platzhalter (kein echter Materialabtrag) mit Koordinaten und Materialstärke: Bohrung,
  Gewindebohrung, Rechteck (eckig/gerundet/gefast), Langloch, Sechseck, Achteck, benutzerdefinierte Kontur, aus Bohrbild;
  neun Anfasspunkte. Längenvariable Teile mit Bohrbild erzeugen sie automatisch, auf Bohrsperrflächen wird ein
  Ersatzdurchbruch daneben gesetzt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_durchbrucheinfuegen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_durchbrueche.htm
- **Bohrsperrfläche**: darin keine Durchbrüche platzierbar; Bohrbilder darunter erscheinen weder in Bohransicht noch im
  Export; bereits vorhandene Durchbrüche bleiben (Prüflauf 026013 findet sie).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_bohrsperrflaechenzeichnen.htm
- **Bohrbild erzeugen** aus Bohrbildkasten (nur im Makroprojekt bearbeitbar; Name = Dateiname); Konturen dazu als *.fc1,
  Bohrtyp/Durchmesser aus Perforex-Maschinenschema.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_r_bohrbildkasten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_bohrbilderzeugen.htm
- **Feldgröße**: fertigungstechnische Größe einer NC-Fläche (mit oder ohne Anbauteile eingespannt) – bestimmt Nullpunkt
  für die Maschine. **Nullschnitt** (Projekt > Mechanische Bearbeitung): Seite, von der das Bohrbild auf
  längenvariablen Teilen gerechnet wird, getrennt für horizontal/vertikal.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_feldgroesseaendern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_einstellungennullschnitt.htm

## 9. NC-Fertigungsdaten

- Prinzip: Export richtet sich nach der Auswahl im Bauraum/Navigator (alles darunter wird mitgenommen; einzelne
  Montagefläche → ganzes Bauteil; auf Projekt erweiterbar). NC-relevant ist ein Bauteil mit Eigenschaft **Mechanische
  Bearbeitung** – entscheidend ist das Trägerbauteil (Kanal auf Montageplatte → Löcher, auf Chassis → keine). Standardmäßig
  aktiv bei Bodenblech, Flanschplatte, Seitenwand Aufsatzmodul, Deckel, Montageplatte, Kabelrangierraum-Seitenwand,
  Sockelblende, Tür, Modulplatte, Trennwand, Wand, Sammelschiene, Korpus. Jede solche Fläche ist ein **Feld**.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_prinzip.htm
- Formate: Rittal Perforex BC / Secarex (Formate DOS, Windows bis/ab 1.9x), Perforex LC / Secarex (Laser; Startloch
  1,5 mm), Steinhauer (CNC mit Dezimalpunkt/-komma), **DXF** (AutoCAD 2013–2019, Ebenen/Farben je Bauteil, Bohrung,
  Gewinde, Kontur; Felder als Rechteck), **Bohrschablone** als 1:1-PDF zum manuellen Bohren (Ränder, Kürzel D/M/B/H/X/R/a),
  Rittal **Athex** (XML zur automatischen Bestückung von Tragschienen mit Klemmen: Artikel, BMK, Länge, Positionen;
  Schienen 150–1200 mm), Rohrbiegung/PipeFab (XML), Kupfer-DXF und Kupfer-NC (XML). Bohrsperrflächen nur an Perforex.
  Abschlussdialog nennt exportierte und nicht exportierte Teile mit Grund.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_h_ncdatenexportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_h_ncmaschinekonfigurieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_athex.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_h_autobestueckdatenexportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_allgemeinexportkiesling.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_allgemeinexportkieslinglc.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_allgemeinexportsteinhauer.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_dxfallgemein.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportbohrschablone.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_beschriftungbohrschablone.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_maschineathas.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportkupferdxf.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportkupfernc.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferabwicklungexportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportpipefab.htm
- **Bohrausnahmen**: je Maschine/DXF/Bohrschablone Ersatz für nicht fertigbare Bohrtypen (z. B. Sechseck → anderes);
  danach greift Werkzeugauswahl (fehlender Bohrer/Gewindeschneider → Alternative, ab Durchmesser fräsen). Werkzeugbelegung
  je Maschine; Stegparameter (Stege nur bei Rechtecken mit kürzerer Seite ≥ 20 mm; LC: Stegbreite 0,1 mm, Höhe 2 mm,
  ab 100 mm Kantenlänge ein Steg).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_Bohrausnahmen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_bohrausnahmen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_werkzeugauswahl.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_werkzeuge.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_stegparameter.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_stegparameterlc.htm
- **2D-Bohransicht** (Einfügen > Grafik > 2D-Bohransicht): Modell-Ansicht nur mit NC-Daten (Bohrungen, Durchbrüche,
  Umrisse von Basis-Bauteilen und Artikeln), bemaßbar, aktualisierbar – zum Abgleich vor der Fertigung.
  **Durchbruchlegende** (Formular *.f47) als Seite oder eingebettet (Manuelle Platzierung): Bohrtyp, X/Y des Anfasspunkts,
  Größe.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_2dbohransicht.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_h_2dbohransichterstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_h_durchbruchlegendeanlegen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_ansicht2dbohransicht.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_2dbohransichten.htm
- Übrige Einstellungsdialoge (Maschinenschemata „Standard“, Ausgabeverzeichnis, Präfix, Feldbezeichnung, DXF-Ebenen und
  Farben, Kupfer-Bauteil/Material) nur überflogen; keine weiteren fachlichen Regeln.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_einstellungenncexportkiesling.htm
  (stellvertretend; vollständige Liste unter Quellen)

## 10. Thermische Auslegung, Verlustleistung

- Thermal Design Integration: Gesamtverlustleistung, Verlustleistungsdichte (5 Farbstufen aus Max. Verlustleistung je
  Oberfläche), Verlustleistungsverteilung (obere/untere Hälfte, Warnung ab ≥ 25 % Unterschied), Schnittstelle Rittal Therm;
  mit Rittal-Daten aus dem Data Portal zusätzlich Lüftungspfeile (blau kalt, rot warm), optimal klimatisierte Bereiche,
  lüftungstechnische Sperrräume. Ziel u. a. IEC 61439.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_thermischeauslegung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_lueftungspfeile.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_verlustleistungsverteilung.htm
- Rechnung: Geräteverlustleistung = Max. Verlustleistung × Gleichzeitigkeitsfaktor (Projekt, Standard 0, je Gerät
  überschreibbar); Schiene: I² · R(65 °C) · l aus Querschnitt, Leitfähigkeit, Temperaturkoeffizient, Δθ = 45 K, Länge aus
  Teilmenge/Länge. Summe je Klimabereich in Projekteigenschaft. Werte mit Einheit eingeben („10 W“). Ergebnisse sind
  Richtwerte ohne Gewähr.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_verlustleistungprinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_verlustleistung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_verlustleistungberechnen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_verlustleistungberechnen.htm

## 11. Schrankmigration TS8 → VX25, Automatische Interpretation

- **Artikel migrieren** (nicht im Pro-Panel-Add-on): nur Rittal TS8, nur mit 3D-Makro platziert, ohne Schutz, nicht
  RICS-/STEP-Schränke; Ersatzartikel müssen in der Datenbank sein (fehlende als CSV). Prüfläufe 026102 (nicht migrierbar),
  026103 (Montagefläche fehlt → „Fehlerhaft platziert“), 026104 (migrierbar, offen); Platzierungen bleiben in X/Y zum
  Nullpunkt. VX25 ist 2 mm breiter → Kollision P026011, „Schaltschrankabstand korrigieren“. Danach BMK und
  Legendenpositionen neu nummerieren, Ansichten/Auswertungen aktualisieren, Verbindungen neu verlegen. Hilfselemente
  übertragbar laut Einstellung; Montageflächen, Anfass-/Montage-/Bezugspunkte, Raster, Einbaufläche nie.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_artikelmigrieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_artikelmigrieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_schrankmigration.htm
- **Automatische Interpretation**: Logik eines manuell definierten Musterschranks (gleiche Bauart) auf importierte Schränke
  übertragen (Funktionsdefinitionen, Montageflächen, Feldgrößen, Anfass-/Montage-/Bezugspunkte, Raster, Ebene EPLAN560 …);
  bei mehreren Vorlagen Übereinstimmungsprüfung mit Trefferliste (Quote in %).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_schrankinterpretation.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_schrankinterpretation.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_trefferliste.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_musterschrankwaehlen.htm

## 12. STEP-/VRML-Export, Design Space Exchange

- **STEP-Export**: facettierte Geometrie des Bauraums (keine analytischen Flächen), je Bauraum eine Datei, Schema
  benutzerbezogen, Ziel Datenträger oder E-Mail (Teilgröße 1–10 MB). **VRML-Export** für Browser; dazu *.wri mit IDs
  „ID000001…“ für alle Teile mit Artikelnummer oder BMK.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_exportierenstep.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_exportierenvrml.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_stepexport.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_einstellungenstepexport.htm
- **Design Space Exchange** (Pro Panel Professional): Konstruktionsraum aus M-CAD als STEP (AP 203/214) importieren →
  separater, schreibgeschützter Bauraum → Montageaufbau in Pro Panel → Export: je platziertem Artikel eine STEP-Datei,
  je Bauraum eine XML mit allen Objekten und Durchbrüchen (Optionen 3D-Montagedaten, NC-Daten, Hilfselemente,
  Verlegeverbindungen, vereinfachte Darstellung; Dateinamen aus Nummer/Bauraum/Eigenschaften; STEP AP 203 oder AP 214
  CD/DIS/IS). Rückführung ins CAD (genannt: Autodesk Inventor) über ein CAD-Add-in, das nicht im Lieferumfang ist.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_h_konstraumimportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_h_konstraumexportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_r_einstellungenstepexport.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_r_einstellungenstepimport.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_d_konstruktionsraumimport.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_d_konstruktionsraumexport.htm

## 13. Externe Bearbeitung (PXF-Schnittstelle)

- „PXF“ ist hier **nicht** der Schaltschrank, sondern **Eigenschaften extern bearbeiten** (Dienstprogramme >
  Eigenschaften extern bearbeiten): Daten **eines Objekttyps** (Funktionen, Kabel, Seiten, Verbindungen …) werden
  XML-kompatibel ausgegeben, extern bearbeitet und wieder eingelesen. Formate: Excel (*.xls/x/m/b, mit Vorlage),
  tabulatorgetrennter Text, Unicode-Text, XML (*.edc). Unformatiert, nur Spaltenreihenfolge wählbar; querverwiesene
  Funktionen zusammenhängend, Seiten in Projektreihenfolge; Verbindungsdefinitionspunkte ohne BMK nicht ausgebbar;
  automatisierbar über EPLAN-Actions.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_k_arbeitsweise.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xpxfconvertertext_d_pxftexteinstellungen.htm
- **Nicht importierbar** (nur Info): bei Funktionen vollständiges BMK, Funktionsart, Position (Seite/Pfad/Zeile),
  Funktionsdefinition, Symbol, Funktionstext (automatisch), symbolische Adresse (automatisch/ermittelt); bei Seiten
  vollständiger Seitenname, Seitentyp.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_k_arbeitsweise.htm
- Ausgabeart „In externer Anwendung bearbeiten und zurücklesen“ (nur mit Zielanwendung oder Excel): EPLAN wartet im
  Hintergrund und fragt beim Schließen nach dem Rückimport. Einstellungen: Optionen > Einstellungen > Benutzer >
  Schnittstellen > Externe Bearbeitung (Schema, Dateityp, Zieldatei, Vorlage, Formatelemente in Reihenfolge).
  Excel-Optionen „Spaltenbezeichnung mit ausgeben“: Nein / Nur Eigenschaftsname / Vollständig (mit Relation).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_h_exporteinstellungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_h_seitenexportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xpxfconverterexcel_d_pxfexceleinstellungen.htm
- **Excel-Vorlage**: Platzhalter `#H#` (Überschriften) und `###` (Daten) Pflicht; optional Formatblatt mit `#ID#`,
  `#HD#` (Kopfdaten), `#F#` (Fuß), `#RO#` (nur lesen), `#RW#` (zurückschreibbar); Excel-Makros werden übernommen.
  Neue Zeilen nur importiert, wenn der Excel-Name **EPL_VALUES** den Bereich mit abdeckt. **Neue Funktionen aus Excel**:
  Objekttyp „Alle Funktionen“, vorhandene Zeile kopieren, BMK (vollständig) und BMK (sichtbar) ändern, Zahl hinter „/“
  in der ersten Spalte auf 0 setzen, EPL_VALUES erweitern → Import legt **nicht platzierte Funktionen** im
  Betriebsmittel-Navigator an.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_k_excelexportvorlagen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_h_datenimportieren.htm

## 14. Bedeutung für QET

- **Kein 3D in QET.** Aus Pro-Panel-Projekten kommen im PDF nur 2D-Ableitungen: Modell-Ansichten, 2D-Bohransichten,
  Kupferabwicklungen, Schaltschranklegenden, Durchbruchlegenden, Montagelisten. Nachbau-Regeln dafür stehen in
  `schaltschrankaufbau.md` Abschnitt 12; Bohr- und Kupferansichten sind reine Fertigungsgrafik → als QET-Linien/Text
  (aufbau_rest) übernehmen, Durchbruch-/Montagelisten als Tabellen aus dem PDF bzw. als eingebettete Auswertung.
- **Makro-Import aus EPLAN/Data Portal**: Am Artikel können zwei Makros hängen – Technische Daten > Makro (2D,
  allpolig/Übersicht) und Montagedaten > Grafikmakro (3D oder 2D-Artikelplatzierung). Für QET-Aufbaupläne die Variante mit
  Darstellungsart „Artikelplatzierung“ (2D-Front) verwenden; Dateinamen mit `_3D` sind *.ema-3D-Makros und für QET
  unbrauchbar **(Vermutung zu Namenskonventionen der Hersteller)**. Fehlt jede Grafik, Rechteck aus Breite × Höhe der
  Montagedaten (wie EPLAN ohne Makro).
- **Artikeldaten mitnehmen**: Breite/Höhe/Tiefe, Einbauabstand, Produktgruppe/-untergruppe (z. B. Montageplatte
  = Mechanik/Gehäuse/Montageplatten, ID 22028) und Lieferlänge bei Schienen/Kanälen gehören beim Bauteil-Import in die
  QET-Elementinformationen, damit Aufbau-Maßstab und Stückliste stimmen.
- **Längenvariable Teile** (Kanal, Tragschiene, Sammelschiene): in Stücklisten mit Länge (Teilmenge/Länge); im QET-Nachbau
  Länge als Datum am Bauteil, nicht nur Grafik.
- **PXF/externe Bearbeitung** ist ein möglicher Weg, BMK-, Funktions- und Kabeldaten aus einem echten EPLAN-Projekt
  tabellarisch zu erhalten (Excel/Text/XML *.edc) – nützlich als Abgleichquelle für Verknüpfungsskript und Stückliste,
  falls der Lieferant so exportieren kann **(Vermutung: im Projekt nicht genutzt)**. Positionsspalten (Seite/Pfad/Zeile)
  liefern die Platzierung je Funktion.
- **STEP/DSE/NC** betreffen den QET-Nachbau nicht; nur relevant, wenn Kunden Bohrbilder oder DXF der Montageplatte
  liefern (DXF-Ebenen je Bauteil/Bohrung → evtl. als Vorlage für den maßstäblichen Aufbau nutzbar) **(Vermutung)**.

## 15. Quellen

3D-Montageaufbau:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_abmessungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_bauraumeigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_bohrbilderzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_durchbrueche.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_einstellungenkupferbiegung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_einstellungenkupferbiegungbenutzer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_einstellungennullschnitt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_musterschrankwaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_navigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_navigator3dschaltschrankaufbau.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_platzieroptionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_sammelschienegebogen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_schrankmigration.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_strukturanpassen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_trefferliste.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_d_verlustleistungberechnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_3dgrafikimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_3dmakrodrehwinkelaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_3dmakroerstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_3dmakroplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_aktivierenautomatisch.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_anfasspunktdefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_anschlussbildnachartvw.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_anschlussdefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_anschlussrichtungaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_artikelabmessungenaktualisieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_artikelmigrieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_artikeltauschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_bauraumansichten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_bauraumstrukturanpassen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_bezugspunktdefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_bezugspunktschemauebertragen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_bohrsperrflaechenzeichnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_drehenxyz.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_durchbrucheinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_einbauabstaende.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_einbauflaeche.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_einpassen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_exportierenstep.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_exportierenvrml.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_feldgroesseaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_geraetplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_gleichmaessigverteilen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_hauptbauteileaktualisieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_hilfslinienzeichnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_horverausrichten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_kollisionskontrolle.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_laengeaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_legendenpositionbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_messen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_mfgroesseaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montageflaechedefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montageflaechetauschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montagehilfenanzeigen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montageliniedefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montageplattenplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_montagerasterdefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_objekteduplizieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_raster.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_schaltschrankplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_schrankinterpretation.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_sperrflaechenzeichnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_varbtplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_verbindungsvorschau.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_vereinigen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_verlustleistungberechnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_verlustleistungsverteilung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_xyachseausrichten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_h_zielmatesdefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_3dmakros.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_artikelmigrieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_auswertungmontageliste.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_betriebsmittellogik.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_bezugspunkte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_lueftungspfeile.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_sammelschienensystem.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_schrankinterpretation.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_thermischeauslegung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_verlustleistung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_verlustleistungprinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_r_bohrbildkasten.htm

Mate-Editor:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ged3dmateeditorgui_d_montagepunkt_montagelinie.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ged3dmateeditorgui_d_montageraster.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ged3dmateeditorgui_r_anfasspunkt_logik.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ged3dmateeditorgui_r_anfasspunktmontageraster.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ged3dmateeditorgui_r_montageraster_format.htm

Bohr-/Kupferansichten, STEP-Export-Dialoge (Modell-Ansichten siehe schaltschrankaufbau.md):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_2dbohransichten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_einstellungenstepexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_stepexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_ansicht2dbohransicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_ansichtabwicklung.htm

Kontureditor:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_d_bohrung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_d_konturauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_d_kontureigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_d_ncdatensatz.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_durchbrverdrahtungskaemdefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_konturdefseigenschaftenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_konturenbereinigen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_konturenerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_konturenkopieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_konturenpruefen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_h_nckonturdatenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_k_logikelemente.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_k_nullpunkt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_r_anzeige.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/contoureditorgui_r_daten.htm

Kupfer:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_bohrbildvererben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferabwicklungerstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferabwicklungexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferartikeldefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferschieneplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferteilbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferteilbiegestelleaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferteillaengeaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferteilneuebiegestelle.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_kupferteilwinkelaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_h_verlaufskonturerstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_abwicklung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_arbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_auswertungmontageliste.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_kupferanschlusselement.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_kupferartikel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_kupferbauteilbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_kupferstrang.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/copper_k_verlaufskontur.htm

NC-Fertigung (Dialoge/Registerkarten _d_/_r_ nur überflogen):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_einstellungenathas.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_einstellungenkupferexportnc.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_einstellungenncbohrschablone.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_einstellungenncexportkiesling.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_einstellungenncexportkieslinglc.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_einstellungenncexportperdxf.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_einstellungenncexportsteinhauer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_einstellungenrohrbiegung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportathas.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportbohrschablone.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportkiesling.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportkieslinglc.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportkupferdxf.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportkupfernc.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportperdxf.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportpipefab.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_d_exportsteinhauer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_h_2dbohransichterstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_h_autobestueckdatenexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_h_durchbruchlegendeanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_h_ncdatenexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_h_ncmaschinekonfigurieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_2dbohransicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_Bohrausnahmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_athex.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_ncdatensatz.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_allgemeinathas.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_allgemeinbohrschablone.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_allgemeinexportkiesling.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_allgemeinexportkieslinglc.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_allgemeinexportsteinhauer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_allgemeinkupferexportnc.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_bauteilkupferexportnc.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_beschriftungbohrschablone.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_bohrausnahmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_dxfallgemein.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_dxfbauteile.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_dxfbohrungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_dxfgewinde.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_dxfkonturen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_feldbezeichner.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_kupferdxfbauteile.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_maschineathas.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_stegparameter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_stegparameter_steinhauer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_stegparameterlc.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_werkzeugauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ncgui_r_werkzeuge.htm

STEP-Import, Design Space Exchange:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_d_konstruktionsraumexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_d_konstruktionsraumimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_h_konstraumexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_h_konstraumimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_r_einstellungenstepexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xdesignspaceimportgui_r_einstellungenstepimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xstepgui_d_einstellungen3dimport.htm

Externe Bearbeitung (PXF):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_h_datenimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_h_exporteinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_h_seitenexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_k_arbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_k_excelexportvorlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pxfinterface_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xpxfconverterexcel_d_pxfexceleinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xpxfconvertertext_d_pxftexteinstellungen.htm
