# EPLAN: Verlegung (Topologie und Routing), Verdrahtungsdaten, Fertigungsverdrahtung, Kabelbaum, Beschriftung

Diese Referenz beschreibt, wie EPLAN aus dem Schaltplan Verlegewege, Leitungslängen, Verdrahtungslisten, Maschinendaten für die Drahtkonfektionierung, Kabelbaumdaten und Beschriftungsdateien gewinnt. Für den QET-Nachbau eines EPLAN-Plans ist das meist **Hintergrundwissen**: Längen, Verlegewege und Drahtdaten erscheinen im PDF nur als Eigenschaften an Kabeln, Verbindungen und in Auswertungsblättern. Aussagen stehen in eigenen Worten mit Quell-URL. „Bedeutung für QET“ überträgt sie auf den Fork, mit **(Vermutung)** markierte Stellen sind nicht aus der Hilfe belegt.

## Inhalt

1. Überblick: vier Wege von der Verbindung zur Fertigung
2. Topologie: Streckennetze in 2D
3. Routing im 3D-Bauraum (Pro Panel)
4. Phasenschienen
5. Fertigungsverdrahtung: Drahtdaten und Maschinenexporte
6. Kabelbaum (Harness)
7. Beschriftung (Labeling) und Exportformate
8. Zusammenfassung für den QET-Nachbau
9. Quellen

---

## 1. Überblick: vier Wege von der Verbindung zur Fertigung

- **Topologie**: 2D-Streckennetze (Kanäle, Pritschen, Rohre) auf eigenen Seiten. Verbindungen und Kabel werden durchgelegt, EPLAN ermittelt Verlegeweg und Länge. Das ist Gebäude- und Anlagenverkabelung. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_start.htm
- **Routing** (Pro Panel, 3D-Bauraum): Verdrahtungskanäle, Tragschienen und Geräte im 3D-Montageaufbau. Verbindungen werden durch ein Streckennetz im Schaltschrank verlegt, das liefert reale Drahtlängen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_start.htm
- **Fertigungsverdrahtung**: exportiert Drahtdaten (Länge, Farbe, Querschnitt, Aderendbehandlung, Beschriftung) an Konfektionierautomaten und Verdrahtungsterminals. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_k_start.htm
- **Beschriftung**: gibt Projektdaten (BMK, Klemmen, Kabel, Verbindungen …) als Datei für Beschriftungssysteme oder Excel aus. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_start.htm

## 2. Topologie: Streckennetze in 2D

### 2.1 Grundbegriffe

- Die Topologie ist ein Bereich der 2D-Bearbeitung, in dem man Streckennetze plant und beliebige Verbindungen, Kabel, Schläuche und Rohre verlegt. Ein Streckennetz kann platziert (Seitentyp „Topologie“) oder nur im Navigator existieren. Es gibt die eigene **Darstellungsart „Topologie“** für Funktionen und Verbindungen. Ein Verbindungsfilter steuert, was wo verlegt werden darf. Beim Verlegen entsteht die Länge. Auswertungen geben Verlegewege, Strecken und durchlaufende Kabel aus. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_start.htm
- Auf Topologie-Seiten bekommen Symbole automatisch die Darstellungsart „Topologie“, und es gibt **kein Autoconnecting**. Strukturiert wird über die Kennzeichnung (z. B. ein Streckennetz je Einbauort). Ein DXF/DWG lässt sich auf eine Ebene mit der Einstellung „Hintergrund“ legen und dient dann als Grundriss. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_prinzip.htm
- **Topologie-Funktion** = normale Funktion mit Darstellungsart „Topologie“. Es gibt Symbolbibliotheken für die Maßstäbe 1:1, 1:20, 1:50 und 1:100. Je Betriebsmittel ist nur **eine** Topologie-Funktion erlaubt (die Hauptfunktion), Ausnahme Klemmenleisten. Topologie-Anschlüsse bauen nur das Netz auf. Sie sind keine Funktionsanschlüsse und dürfen nicht mit normalen Anschlüssen in einem Symbol gemischt werden. Verlegeverbindungen hängen an den logischen Anschlüssen im **Anschlussbild**. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_topologiefunktionen.htm
- Sonderfälle:
  - **Klemmenleisten**: nur die Leiste ist Topologie-Funktion, die Klemmen stehen in ihrem (immer lokalen) Anschlussbild. Eine Leiste darf in mehreren Netzen vorkommen, in einem Netz aber nur einmal. Klemmen ohne Anschlussbezeichnung werden über die Anschlusslogik (intern/extern) verlegt.
  - **Stecker**: je Steckerdefinition eine Topologie-Funktion.
  - **Kabeldefinitionslinien** auf Topologie-Seiten bekommen die Funktionsdefinition „Topologie-Kabel“ und übernehmen kein BMK auf geschnittene Verbindungen.
  - **Topologie-Abbruchstellen** führen eine Strecke auf einer anderen Seite weiter.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_topologiefunktionen.htm
- Einfügen: aus einem Navigator platzieren (allpolige Funktion → passende Topologie-Funktion), als Symbol (später zuweisen) oder als Gerät (Makro mit Darstellungsart „Topologie“). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_funktioneneinfuegen.htm

### 2.2 Strecken, Verlegepunkte, Teilstrecken

- **Strecken** verbinden Verlegepunkte oder Topologie-Funktionen. Sie haben immer die Darstellungsart Topologie und können einen Artikel (Material) tragen.
  - Manuelle Strecke: dunkelrot, als Polylinie gezeichnet. Nur Start und Ende sind Fangpunkte, beide müssen an Verlegepunkt, Funktion oder Abbruchstelle liegen. Kreuzen sich zwei Strecken, braucht der Schnittpunkt einen Verlegepunkt.
  - Eigenschaften „Topologie: Durchlauflänge (automatisch)“ (geometrisch) und „Topologie: Durchlauflänge“ (manuell, z. B. bei nicht maßstäblicher Zeichnung oder Z-Anteil).
  - **Automatische Strecken** (orange) entstehen beim Verlegen von einer nicht angebundenen Funktion zum nächsten Verlegepunkt und bleiben bestehen. Ihre Länge wird nicht berechnet, nur ein manueller Eintrag zählt.
  - Verbindungen treten nie mitten in eine Strecke ein.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_strecken.htm
- Einfügen einer Strecke: Die Eigenschaften stehen bei ungerader Teilstückzahl am mittleren Teilstück, bei gerader am Teilstück vor der Mitte. [Leertaste] schließt eine Strecke frei ab. Im Navigator entstehen nicht platzierte Strecken. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_streckeeinfuegen.htm
- **Verlegepunkte** sind markante Stellen (Kurve, Wanddurchbruch, Schrankeinführung). Sie unterbrechen Strecken und haben höchstens einen Topologie-Anschluss. **Strukturverlegepunkte** (Funktionsdefinition „Topologie-Strukturverlegepunkt“) stehen für eine Kennzeichnungsstruktur: Kabel zwischen Schaltschränken lassen sich verlegen, ohne alle Geräte einzeln zu platzieren. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegepunkte.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_verlegepunkteeinfuegen.htm
- Ablauf zwischen zwei Strukturen:
  1. Zwei Strukturverlegepunkte mit einer Strecke verbinden.
  2. „Topologie-Funktionen erzeugen > Automatisch“ (oder manuell im Dialog: Hauptfunktionen ohne Topologie → Angeschlossene Topologie-Funktionen) erzeugt nicht platzierte Topologie-Funktionen und Strecken. Die Registerkarte „Angeschlossene Strukturen“ kann dabei filtern.
  3. Verlegen.
  4. Optional platzieren mit „Platzieren > Topologie“ und für Strecken „Platzieren > Automatisch“.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_strukturenverbinden.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_d_funktionenerzeugen.htm
- **Teilstrecken** (Funktionsdefinition „Topologie-Teilstrecke“) bilden vertikale Strecken zwischen Etagen oder führen ein Netz auf einer anderen Seite fort.
  - Es gibt immer genau zwei mit gleichem BMK: Hauptfunktion mit Durchlauflänge und Richtung „Nach oben“, Nebenfunktion mit „Nach unten“ und leerer Länge.
  - Symbole CABDSU/CABDSD in der Bibliothek SPECIAL.
  - Eigenschaften „Topologie: Höhendifferenz“ (ID 20347, mit Vorzeichen) und „Topologie: Richtung für Höhendifferenz“ (ID 20348).

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_teilstrecke.htm
- **Verlegeart** je Strecke: „Topologie: Verlegeart (Strecke)“ (ID 20345, z. B. Rohr, Kanal, Kabelbühne; eine je Strecke). Die durchlaufenen Verlegearten stehen an Kabel und Verbindung in „Topologie: Verlegearten“ (ID 20344, semikolongetrennt in Wegreihenfolge) und sind in Kabelauswertungen und Verbindungslisten ausgebbar. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegeart.htm

### 2.3 Verlegen und Verlegeverbindungen

- **Verlegeverbindungen** sind Verbindungen mit Darstellungsart Topologie. Sie entstehen beim Verlegen aus allpoligen Verbindungen oder manuell im Dialog „Geräte verschalten“. Quelle und Ziel brauchen je eine Topologie-Funktion. Ohne gefundenen Weg bleibt eine **unverlegte** Verlegeverbindung (dünne orange Linie, direkt, auch diagonal). Verlegte Verbindungen erscheinen rosa. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegeverbindungen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_vorgehensweise.htm
- Verlegt werden nur die Funktionsdefinitionen Ader/Draht, Verbindung allgemein, Schlauch, Rohr, nicht elektrische Verbindung, Lichtwellenleiter und Verfahrenstechnik. Einpolige **Kabel** sind verlegbar (Kennzeichen „Topologie: Einpolig erzeugt“, ID 20343), einpolige Einzelverbindungen nicht. Die allpolige Darstellung hat Vorrang. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegeverbindungen.htm
- **Geschützte Verlegung** am Kabel oder an einer seiner Adern schließt das ganze Kabel vom Verlegen aus. Länge und Weg bleiben dann, typisch bei vorkonfektionierten Kabeln (Artikeleigenschaft „Länge (vorkonfektioniert)“). Prüflauf 007023 findet Abweichungen zwischen Kabel und Artikel. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegeverbindungen.htm
- Beim Verlegen eingetragene Eigenschaften:
  - An Verbindungen: **Verbindung: Länge** (auf volle mm aufgerundet), Anschlusslänge Quelle/Ziel, **Topologie: Verlegeweg** (Strecken-BMK, semikolongetrennt; leer = nicht verlegt), Verlegeweg-Vorgabe, Verbindungsende-Behandlung, Anschlussmaß.
  - An Kabeln: **Länge (vorkonfektioniert)**, Kabel: Anschlusslänge, Verlegeweg, **Kabel/Paket: Absetzlänge Quelle/Ziel**.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegeverbindungen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_verlegen.htm
- Die Projekteinstellungen „Verlegeverbindungen“ gelten für Bauraum und Topologie. In der Topologie gelten Besonderheiten: Doppelhülsen werden nicht ausgewertet, und die Registerkarte „Verlegung“ wird ignoriert. Die Filterwerte erscheinen an Strecken in „Topologie: Verbindungsfilter“ (ID 20247). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegeverbindungen.htm
- **Verbindungsfilter**: In den Projekteinstellungen wählt man eine Verbindungseigenschaft (z. B. Verbindungsfarbe/-nummer) und bis zu 15 Textwerte. An der Strecke aktiviert man dann die erlaubten Werte. Der Vergleich ist **rein textuell**. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_verbindungsfilter.htm
- **Verlegung ändern**: den Mittelpunkt einer Strecke (oder die orange Linie) auf eine andere Strecke ziehen und die Verbindungen wählen. Das füllt „Topologie: Verlegeweg-Vorgabe“, danach gelten für diese Verbindung keine Verbindungsfilter mehr. Die Vorgabe hat Vorrang, auch gegen den Filter. Löschen der Vorgabe stellt das Filterverhalten wieder her. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_verlegewegaendern.htm
- **Kabel / Verbindungen** einer Strecke (Kontextmenü): Liste der durchlaufenden Verbindungen. Läuft ein ganzes Kabel durch, steht nur das Kabel-BMK. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_d_kabelverbindungen.htm
- **Füllgrad**: Mit „Topologie: Streckenquerschnitt“ (mit Einheit) an Strecken und Außendurchmessern aus den Artikeln berechnet „Füllgrad bestimmen“ projektweit „Topologie: Füllgrad“ in Prozent. Ohne Querschnitt wird der Wert gelöscht. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_fuellgradbestimmen.htm
- **Topologie-Navigator**: Strecken, Verlegepunkte und Ziele (nur Topologie-Funktionen) mit vollständiger Kennzeichnung. Kontextmenü: Verlegepunkt/Strecke/Ziel erzeugen, Neues Gerät, Verlegen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_d_navigator.htm

### 2.4 Längenermittlung, Anschlussbilder, Leitungsverteiler, Bestelllänge

- **Länge einer Verlegeverbindung** = Summe der durchlaufenen Strecken und Verlegepunkte + Anschlusslängen von Quelle und Ziel (aus dem Anschlussbild: X+Y+Z = rechtwinklige Verdrahtung, plus Zusatzlänge bzw. „Zusatzlänge (Standard)“ des Artikels, plus „Topologie: Anschlusslängenzuschlag“ an der Funktion) + **globaler Längenzuschlag** (je ½ auf die Weglänge, je ¼ auf die Anschlusslängen von Quelle und Ziel). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_laengenermittlung.htm
- **Kabel**: Wird die erste Ader verlegt, folgen alle anderen. Der „gemeinsame Weg“ der Mehrheit der Adern wird in „Topologie: Verlegeweg“ am Kabel eingetragen, dann verlegt EPLAN alle Adern erneut danach. Kabellänge = gemeinsamer Weg + Anschlusslängen (längste Aderanschlusslänge + Zuschlag). Kabel lassen sich nur verlegen, wenn alle Adern eindeutig sind (unterschiedliche **Farbe/Nummer und Paarindex**). Absetzlänge = größter Anschlusswert aller Adern aus dem Anschlussbild. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_laengenermittlung.htm
- **Anschlussbild**: Ein lokales Anschlussbild geht vor dem Artikel. Fehlen X/Y/Z, wird auf (0,0,0) verlegt, dann besser einen größeren Anschlusslängenzuschlag setzen, und die Absetzlänge wird 0. Ohne Anschlussbild erzeugt EPLAN fehlende Anschlüsse. Passen mehrere Topologie-Funktionen gleich gut, ist die Verbindung nicht verlegbar. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_anschlussbild.htm
- **Leitungsverteiler**: Nur einer je Betriebsmittel ist Topologie-Funktion. Bei durchgehenden Verbindungen steht in „Durchlaufene Leitungsverteiler“ (ID 31136), welche Verteiler die Verbindung passieren muss. Der Verteiler wird über eine Strecke der Länge 0 an einen Verlegepunkt gebunden. Weitere IDs: „Leitungsverteiler vorhanden“ (31139), „Topologie: Durchlaufene Leitungsverteiler (sortiert)“ (31137), „Topologie: Abstand der Leitungsverteiler zur Quelle“ (31138). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verteilertopologie.htm
- **Bestelllänge** für Kanäle, Pritschen und Rohre: Artikel der Produktgruppe „Strecke (Topologie)“ mit **Lieferlänge** (mm, ID 22058). „Teilmenge / Länge“ (ID 31008) wird aus „Topologie: Durchlauflänge (automatisch)“ (ID 20250) gefüllt. Im Stücklistenformular rechnet ein Platzhalter „Eigenschaften formatiert/berechnet“ mit „Gesamtlänge mit Einheit des Projekts“ (ID 20513) / Lieferlänge × 1000 + 0,4999 auf ganze Stück auf. Beispiel der Hilfe: 10,5 m Kanal bei 2-m-Stücken → 12 m bzw. 6 Stück. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_bestelllaenge.htm
- Auswertungen: **Topologie: Streckenliste**, **Topologie: Streckenplan** (je Strecke die durchlaufenden Kabel/Verbindungen), **Topologie: Verlegte Kabel / Verbindungen**. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegeverbindungen.htm

**Bedeutung für QET:** QET hat keine Topologie. Im EPLAN-PDF sieht man sie nur indirekt: Kabellängen mit krummen mm-Werten, Spalten „Verlegeweg“ oder „Verlegeart“ in Kabelübersicht und Verbindungsliste, eigene Topologie-Seiten (Grundriss mit roten/orangen Linien) und Auswertungen Streckenliste/Streckenplan. Beim Nachbau Topologie-Seiten als Grafik übernehmen (keine Leiter, kein Autoconnect, sie sind keine Schaltplanlogik). Kabellängen und Verlegewege als Kabeleigenschaft (`length`, ggf. Zusatztext) eintragen und nicht nachrechnen. Wichtig für Verknüpfungsskripte: Topologie-Symbole sind eine eigene Darstellungsart desselben Betriebsmittels und kein zweites Gerät. Ein Gerät, das auf einer Topologie-Seite noch einmal auftaucht, darf keine neuen Querverweise oder Klemmenziele erzeugen **(Vermutung zur Umsetzung)**.

## 3. Routing im 3D-Bauraum (Pro Panel)

### 3.1 Prinzip und Ergebnis

- Verlegen erzeugt **Verlegeverbindungen mit Darstellungsart „3D-Montageaufbau“**. Verlegt werden Verbindungen, deren Ziele als 3D-Artikelplatzierung im Bauraum stehen. Einpolige Verbindungen sind nicht verlegbar. Angezeigt werden die Verbindungen in Verbindungsfarbe und Außendurchmesser. Das Ergebnis: Verdrahtungswege, Drahtlängen, Aderendbehandlung, Füllgrad der Kanäle. Daraus entstehen Drahtstücklisten als Montageanweisung und Steuerdateien für Konfektioniermaschinen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_start.htm
- Ablauf:
  1. Verbindungen im Schaltplan mit **Drahtartikel** anlegen (ohne Artikel entstehen nur Linien, weil der Außendurchmesser fehlt).
  2. 3D-Platzierung.
  3. Streckennetz.
  4. Verlegen.
  5. Verlegeweg ermitteln.

  Verlegt werden die Funktionsdefinitionen Ader/Draht, Verbindung allgemein, Schlauch, Rohr, nicht elektrische Verbindung, Lichtwellenleiter, **Drahtbrücke** und Verfahrenstechnik. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_prinzip.htm
- Beim Verlegen eingetragene Eigenschaften:
  - **Länge**: Strecken + Verlegebereiche + Zusatzlänge aus dem Anschlussbild + Längenzuschlag der Durchbrüche + Biegeradien.
  - **Bauraum: Verlegeweg** (semikolongetrennt, leer = nicht verlegt) und **Verlegeweg-Vorgabe** (hat Vorrang vor Filtern).
  - Verbindungsende-Behandlung Quelle/Ziel und Anschlussmaß.
  - **Verlegerichtung Quelle/Ziel** (z. B. „Nach rechts, nach unten“, Information für den Verdrahter).
  - **Doppelhülse vorgeschrieben** (aus dem Anschlussbild).

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_prinzip.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verlegen.htm
- Abstände: intern mindestens 10 mm zwischen Bauteil und Eintrittspunkt in die Strecke. Darunter rechnet EPLAN dynamisch mit Drahtdurchmesser und Mindestbiegeradius. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_abstaende.htm

### 3.2 Streckennetz im Bauraum

- **Streckennetz erzeugen**: Durch jeden Verdrahtungskanal läuft eine automatische Strecke. Aneinanderliegende Kanäle werden verbunden, wobei die Projekteinstellung „Toleranzabstand Verdrahtungskanäle“ Lücken überbrückt. Offene Kanalenden bekommen eine Querstrecke. Verlegebereiche gehören ohne Strecken zum Netz. Je Bauraum entsteht ein eigenes Netz, Verlegen über Bauraumgrenzen geht nicht. Nach jeder Änderung neu erzeugen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_streckennetzerzeugen.htm
- **Manuelle Strecken** (blau) haben Länge und Länge (automatisch). Die manuelle Länge dient z. B. als Reserve für die Türverdrahtung. Der **Streckenquerschnitt** ist die Grundlage für den Füllgrad. Automatische Strecken sind nicht bearbeitbar und lassen sich vor dem Verlegen löschen, wenn nur manuelle Wege gelten sollen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_streckeeinfuegen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_autostreckenloeschen.htm
- **Verlegebereiche** (transparente Quader): darin wird immer der kürzeste direkte Weg genommen, z. B. für die Rückseite der Montageplatte. Darin dürfen keine Bauteile liegen (Prüflauf P026036). Benachbarte Bereiche verbindet man über manuelle Strecken. **Verdrahtungsdurchbrüche** lenken Drähte auf die Rückseite. Sie haben genau eine Strecke (nur eine Richtung) und einen **Längenzuschlag** (leer = Projekteinstellung), der je durchlaufender Verbindung einmal addiert wird. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verlegebereicheinfuegen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verdrahtungsdurchbrucheinfuegen.htm
- **Verdrahtungssystem**: Verdrahtungskämme sind benutzerdefinierte Schienen aus einer Extrusionskontur mit den Montageflächen „Verdrahtungskamm Vorderseite/Rückseite“. Ein automatisch erzeugter Durchbruch gehört dazu, auf der Rückseite liegt ein Verlegebereich. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_verdrahtungssystem.htm
- **Kurven** und **freies Verlegen**: Raumkurven mit Stützpunkten (Steigung, Krümmungsfaktor). Frei verlegte Verbindungen außerhalb des Netzes eignen sich für große Querschnitte, Ethernet, Türgeräte und Schläuche. Sie treten tangential aus dem Anschluss aus und tangential ins Ziel ein. Bearbeitung über „Neuer Stützpunkt“, „Kurvenverlauf ändern“, „tangential ausrichten“. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_freiverlegen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_kurveeinfuegen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_kurveneuerstuetzpunkt.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_kurvenverlaufaendern.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_kurvenverlauftangential.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_verlegeverbindung.htm
- **Verlegezubehör**: Kabel-/Schlauchbefestigung (Schelle, auf einer Montagefläche; Verbindung per „Verlegung ändern“ hindurchführen), Kabel-/Schlauchbinder und Kabel-/Schlauchschutz (z. B. Spiralschlauch, mit Start- und Endpunkt). Grundlage sind „Bündeldurchmesser minimal/maximal“ am Zubehörartikel und der **Außendurchmesser** am Verbindungsartikel. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_verlegezubehoer.htm

### 3.3 Steuern, prüfen, optimieren

- **Verbindungsfilter** an Kanälen, Strecken, Verlegebereichen und Durchbrüchen („Bauraum: Verbindungsfilter“): textuelle Werte einer Verbindungseigenschaft, die auch am **Verbindungsartikel** stehen müssen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verbindungsfilter.htm
- **Verlegung ändern** über Modifikationspunkte (füllt „Bauraum: Verlegeweg-Vorgabe“, Filter wirken dann nicht mehr). **Eintritt ins Streckennetz** gibt per Rechteck und Kanalwahl vor, wo Drähte ins Netz gehen. **Verlegerichtung ändern** (Automatisch/links/oben/rechts/unten) überschreibt die Anschlussrichtung des Anschlussbilds. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verlegewegaendern.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_eintrittinsstreckennetz.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_verlegerichtungaendern.htm
- Anzeige: Anschlüsse als rote Quader, Richtungen als Pfeile, Streckenansicht (auch nach Filter), Streckennetzverfolgung (zeigt Lücken). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_anschlussansicht.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_streckenansicht.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_streckennetzverfolgung.htm
- **Füllgrad** der Kanäle: rot = voll (über der Füllgradgrenze), gelb = zwischen Warngrenze und Füllgradgrenze, grün = unter der Warngrenze. Voreinstellung: Füllgradgrenze **80 %**, Warngrenze **70 %**. Nach Änderungen „Füllgrad aktualisieren“. Ohne Anschlussbild („Nach Anschlussposition (Automatisch)“) entstehen zu lange Drähte, dann ist der Füllgrad unrealistisch. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_fuellgradanzeigen.htm
- **Verlegeverbindung einfügen** (manuell im Bauraum) erzeugt **kein** Gegenstück im Schaltplan. Prüflauf 005067 meldet fehlende allpolige Entsprechungen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verlegeverbindungeinfuegen.htm
- **Netze automatisch optimieren**: baut die Verbindungen eines Netzes so um, dass alle Anschlüsse angeschlossen bleiben, die Drahtsumme minimal ist, **höchstens zwei Drähte je Anschluss** enden und immer eine **Kette** entsteht (optional als Ring geschlossen). Netze ab 2 bis höchstens 2000 Verbindungen. Alle Verbindungen übernehmen die Eigenschaften der **ersten** Verbindung. Brücken, Direktverbindungen und interne Verbindungen bleiben fest. **Netz manuell optimieren**: Reihenfolge per Anklicken (rot = zu verbinden, grün = zwei Verbindungen, gelb = eine). Netze entstehen über „Netzverbindend“, Potenzialweiterleitung oder Brücken. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_netzautooptimieren.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_netzeoptimieren.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_netzeoptimierenvorschau.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_netzmanuelloptimieren.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_netzmanuelloptimieren.htm
- **Bündeln** schreibt Bündelnummern in die Eigenschaften **Bündel** und **Bündelgruppe** der Verlegeverbindungen, automatisch oder von Hand.
  - Mehrere Bündelschemata laufen von oben nach unten. Ein früher vergebenes Bündel wird nicht überschrieben (Beispiel: erst rote, dann schwarze, dann übrige Drähte, jeweils nach Querschnitt).
  - Jedes Schema hat Filter, Sortierkriterien (z. B. Gruppierungsvorzeichen der Bauteilbezeichnung = je Schrank/Montageplatte, BMK Quelle/Ziel, Netz, Farbe/Nummer, Querschnitt, Bündelgruppe, Typbezeichnung, Artikelnummer, Montagefläche, Anschlussbezeichnung) und „Neues Bündel bei geändertem Eigenschaftswert“.
  - Grenzen: Startwert/Schrittweite (nur freie Nummern, Lücken werden gefüllt), max. Drähte je Bündel, max. Summe der Aderquerschnitte (maschinenspezifisch).
  - Ergebnisvorschau mit „Bündel neu“ (von Hand änderbar).

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_autobuendeln.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_autobuendeln.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_einstellungenbuendeln.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_sortierungbuendeln.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_neuesbuendeleigenschaft.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_verbindungenbuendelnvorschau.htm

**Bedeutung für QET:** Der 3D-Bauraum ist für den QET-Nachbau nicht nachbildbar. Wichtig ist er aus zwei Gründen. (1) Netzoptimierung: Im verlegten Projekt kann EPLAN die **Drahtreihenfolge eines Potenzials** umgebaut haben (Kette, höchstens zwei Drähte je Klemmstelle). Die Verbindungsliste oder Drahtliste im PDF zeigt dann andere Paare von Quelle/Ziel als die gezeichneten Linien im Stromlaufplan, und beides ist richtig. Für die QET-Leiter gilt die Zeichnung, für Verdrahtungslisten das Original. (2) Längen, Verlegewege, Bündel, Verlegerichtungen und „Doppelhülse“ in einer EPLAN-Drahtliste sind berechnete Fertigungsdaten. Beim Nachbau eines Verdrahtungslistenblatts übernimmt man sie als Text und rechnet sie nicht nach.

## 4. Phasenschienen

- Phasenschienen ersetzen Verdrahtung zwischen benachbarten Geräten (z. B. Motorschutzschalter). In EPLAN tragen solche Verbindungen die Funktionsdefinition **„Phasenschienenverbindung“** und fallen aus der Verdrahtungsberechnung und der Drahtkonfektionierung heraus. Es gibt zwei Wege:
  - **Umwandeln** im Bauraum: Bauteile per Rechteck wählen. Erlaubt sind Artikelplatzierungen „normales Bauteil“, „SPS-Karte“, „Gerätekasten“. Verbindungen mit Darstellungsart allpolig und 3D zwischen ihnen umwandeln. Dabei werden **alle Eigenschaften gelöscht**, aus 3D-Verbindungen werden unverlegte. Optional BMK und Artikel zuweisen.
  - **Einfügen > Phasenschiene** im Schaltplan als Definitionslinie über die Verbindungen. Sie bekommt automatisch ein BMK und den Artikel und wird aus dem 3D-Montageaufbau-Navigator platziert (ohne 3D-Makro mit Abmessungen).

  Phasenschienenverbindung löschen setzt die Funktionsdefinition auf „Ader / Draht“ zurück. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_phasenschienenverbdg.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_phasenschienenvbdgerzg.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_phasenschieneneinfg.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_phasenschienenvbdgerzg.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_phasenschienezuweisen.htm

**Bedeutung für QET:** Eine Phasenschienen-Definitionslinie im EPLAN-Stromlaufplan (Linie quer über L1/L2/L3 mit eigenem BMK) gleicht grafisch einer Kabeldefinitionslinie, ist aber kein Kabel. In QET daher **kein** `Cable` anlegen. Die Leiter bleiben normale Leiter, ihre Funktion ist „Phasenschiene“ (Text in `m_function` oder Zusatztext), und das BMK der Schiene wird ein eigenes Bauteil oder Textobjekt **(Vermutung zur Abbildung)**. In Drahtlisten dürfen diese Verbindungen nicht erscheinen.

## 5. Fertigungsverdrahtung: Drahtdaten und Maschinenexporte

### 5.1 Inhalt der Drahtstückliste und Formate

- Pro Panel gibt Fertigungsdaten aus für **Rittal Wire Terminal WT, Komax, CadCabel, Metzner (Triathlon Kabelcenter), Schleuniger (Easy Production Server), Steinhauer (Personal Wiring Assistant)**. Jede Schnittstelle wird einzeln lizenziert. Dazu kommt die konfigurierbare **„Allgemeine Drahtstückliste“**. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_k_start.htm
- Eine Drahtstückliste enthält je Draht:
  - Drahtnummer
  - Start-BMK (eindeutig) mit Anschlusspunkt
  - Ziel-BMK mit Anschlusspunkt
  - Artikelnummer des Drahts
  - Länge
  - Startrichtung und Endrichtung
  - Bündelnummer

  Nach jedem Export meldet ein Abschlussdialog: Format, Schema, Pfad, Anzahl, alternative Exporte, **nicht exportierte Komponenten mit Grund** und Systemmeldungen. Für Rittal WT ist eine Importvorlage aus Wire Cockpit nötig. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_k_start.htm
- Voraussetzungen: Verlegeverbindungen erzeugt, Bauraum offen, Auswahl im Verbindungen-Navigator oder im Bauraum. Die Drahtstückliste wird **textuell nach Bündelnummer** sortiert („1, 10, 2 …“). Dagegen helfen führende Nullen. Herstellerformate erzeugen teils mehrere Dateien mit festen Namen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_h_drahtdatenexportieren.htm
- Exportdialog: Schema (Maschine), **Sprache** für Verlegerichtungstexte (wenn keine eigenen Texte eingetragen sind), Pfad (mit Pfadvariablen), Dateiname (Endung vom Format). **„Gefertigte Verdrahtungen erneut exportieren“**: Ist die Option aus, gehen nur neue Verbindungen hinaus. Gelöschte Verbindungen werden nie exportiert. Optional das gesamte Projekt. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_d_exportdrahtdaten.htm
- **Entfallstückliste**: tabulatorgetrennte Unicode-Textdatei mit bereits exportierten, später gelöschten Verbindungen. Mit „Nur gelöschte Verbindungen“ fallen solche mit nur geänderter Länge heraus. Voraussetzung ist ein früherer Export. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_d_ausgabeentfallstueckliste.htm
- **EPLAN Smart Wiring**: Export als **EPDZ** mit Projekt-, Seiten- und Bauraumeigenschaften (ganze Projektstruktur), Verbindungs- und Kabeldaten der Verlegeverbindungen und je Bauraum eine **E3D**-Datei mit 3D-Grafik. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_h_smartwiringdatenexportieren.htm

### 5.2 Maschinendefinition (Firmeneinstellungen)

- Je Maschine ein Schema unter Optionen > Einstellungen > Firma > Drahtkonfektionierung. Registerkarten je nach Maschine: Allgemein, Drähte, Aderendbehandlung, Maschinenbefehl, Beschriftung, Einstellungen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_d_einstellungendrahtkonfektionierung.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_h_maschinekonfigurieren.htm
- **Allgemein**: Pfad und Dateiname. Zeichensatz (nur Allgemeine Drahtstückliste): **ANSI** (Standard, Windows), ASCII (DOS) oder **UNICODE** (für Sonderzeichen in Anschlussbezeichnungen). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_allgemein.htm
- **Drähte** (Stammdaten je Farbe/Querschnitt/Artikel; „Aktualisieren“ liest das Projekt ein):
  - Komax: WireKey (z. B. „2.5 SW“), FontKey (Druckschrift an der Maschine), Abstand der Beschriftung von den Enden, Abstand Endlostext, **Abisolierlänge**, **Abzugslänge** (das Isolierstück bleibt als Schutz auf dem Litzenende).
  - CadCabel: Drahttyp und Kabelcode laut CadCabel-Excel.
  - Schleuniger: Schleuniger-Artikelnummer und Abstände.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_draehte.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_draehtekomax.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_draehtecadcabel.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_draehteschleuniger.htm
- **Aderendbehandlung**: ordnet jeder projektweiten **Verbindungsende-Behandlung** eine Maschinenbehandlung zu: einfach, **Twin** (zwei Drähte in einer Hülse), **Twin Mitte** (Komax-Doppelcrimpmodul: Doppelhülse in der Mitte einer Zweierfolge, Enden nur abisoliert). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_aderendbehandlung.htm
- **Maschinenbefehl**: je Aderendbehandlung und „bis Querschnitt“ ein Befehl in Maschinensprache. Komax-Beispiele: `AEH1-n` (TerminalKey, Hülse passend zum Querschnitt), `$strip$` (abisolieren), `$ultrasonic,xx$` (Ultraschallschweißen über xx mm). `$strip,a,b$` setzt Abisolier- und Abzugslänge für Doppelhülsen und überschreibt die Werte der Registerkarte Drähte. Fehlt b, gilt a/2. Fehlt a, gilt der Wert der Registerkarte Drähte. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_maschinenbefehl.htm
- **Beschriftung**: bis zu drei Texte je Draht, jeweils aus Formatelementen. Komax/Schleuniger: 1. Seite, 2. Seite, Endlostext. Rittal WT: Drucktext Quelle, Ziel, Mitte. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_beschriftung.htm
- **Einstellungen** (Maschinengrenzen, Voreinstellungen):

  | Maschine | Drahtlänge min/max | Querschnitt max | Drähte je Bündel max | Summe Querschnitte max |
  |---|---|---|---|---|
  | Allgemeine Drahtstückliste | 1 / 150 000 mm | 50 mm² | 10 000 | 10 000 mm² |
  | CadCabel | 230 / 15 000 mm | 16 mm² | 32 | 80 mm² |
  | Komax | 240 / 3 000 mm | 6 mm² | 50 | 80 mm² |
  | Metzner | 230 / 15 000 mm | 6 mm² | – | – |
  | Schleuniger | 240 / 3 000 mm | 6 mm² | 1 | 6 mm² |
  | Steinhauer PWA | 240 / 3 000 mm | 6 mm² | – | – |
  | Rittal WT | 230 / 10 000 mm (Querschnitt min 0,5 mm²) | 2,5 mm² | – | – |

  Kürzere Drähte gehen mit der Minimallänge an die Maschine. Weitere Werte: „Endlostext ab Drahtlänge“ (leer = immer Endlostext) und eigene Texte für die Verlegerichtungen oben/unten/links/rechts.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenallgemdrahtsl.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungencadcabel.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenkomax.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenmetzner.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenschleuniger.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungensteinhauerpwa.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenwireterminal.htm
- Ausgewählte Sonderoptionen:
  - Allgemeine Drahtstückliste: „Verbindungsquerschnitt nicht in mm² umrechnen“ (AWG-Werte wie „18AWG“ bleiben erhalten).
  - Komax: max. Druckzeichensatzlänge 39, Bündelung einseitig/zweiseitig, „Drucktexte automatisch drehen“ (Text am Drahtende um 180° gedreht, zeigt die Anschlussseite), Doppelcrimpmodul, Flexbündler (alle Bündel in einem Lead-Job, max. 99 Drähte).

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenallgemdrahtsl.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenkomax.htm

**Bedeutung für QET:** Die Fertigungsdaten setzen den 3D-Bauraum voraus und sind in einem EPLAN-PDF meist gar nicht enthalten. Eine Drahtliste im PDF ist eine normale Auswertung (Verbindungsliste) und wird als Auswertungsblatt nachgebaut. Für einen eigenen Drahtlisten-Export aus QET (z. B. CSV aus Leitern) liefert die Spaltenliste oben eine sinnvolle Vorlage: Drahtnummer, Quelle-BMK:Anschluss, Ziel-BMK:Anschluss, Artikel, Farbe, Querschnitt, Länge, Endbehandlung, Bündel. Die Länge fehlt in QET und bleibt leer **(Vorschlag, keine EPLAN-Regel)**. Zeichensatz Unicode wählen, wenn Anschlussbezeichnungen Sonderzeichen enthalten.

## 6. Kabelbaum (Harness)

- Kabelbäume werden in **EPLAN Harness proD** konstruiert, in die Plattform importiert, dort bearbeitet und ausgewertet und wieder exportiert. Man kann Kabelbäume vordefinieren oder als Symbol einfügen, ihnen Funktionen, Verbindungen und Artikel zuordnen. Ihre Artikel erscheinen in Auswertungen und im Fertigungsdaten-/Beschriftungsexport. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_k_start.htm
- **Kabelbaumdefinition** identifiziert den Baum (BMK **und Kabelbaumname**) und trägt seine Artikel: Zeile 1 = Artikel der Kabelbaum-Baugruppe (optional), Zeilen 2 bis 1000 (bis zu 999 weitere) = Artikel ohne eigene EPLAN-Funktion (Zubehör). **Zuordnung** von Funktionen und Verbindungen zum Baum über die Eigenschaft **Kabelbaumname (ID 31143)**, auch per tabellarischer Bearbeitung, als Filter oder Spalte. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_k_verwaltungkabelbaum.htm
- Stückliste: In „Einstellungen: Artikel“ wählt man **Kabelbaum-Baugruppenartikel** (Zeile 1) und/oder **Kabelbaumkomponenten-Artikel** (Hauptfunktionen mit Kabelbaumname plus Zeilen 2–1000). Beides ist **UND-verknüpft** mit den übrigen Optionen: Ein Kabel im Kabelbaum erscheint nur, wenn auch „Kabelartikel“ an ist. Gleiche Optionen gibt es im Fertigungsdaten-/Beschriftungsexport. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_k_verwaltungkabelbaum.htm
- Einfügen: Symbol **„HDEF“** über die Symbolauswahl (Direkteingabe). Kopieren geht nur bei platzierter Definition. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_h_definitioneneinfuegen.htm
- **Import** (*.hpdex): Abgleichdialog mit Status (neu / vorhanden / abweichender Stand; geänderte Felder dunkelgrau) und Aktion (importieren / ändern / nicht importieren). Nur im Projekt vorhandene Funktionen/Verbindungen können gelöscht werden. Neue Daten kommen **nicht platziert** ins Projekt. **Export** (*.hpdex) der markierten Bäume, wahlweise ergänzen oder überschreiben. Mit „Anwenden auf gesamtes Projekt“ geht das ganze Projekt mit allen Projektdaten hinaus. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_h_importkabelbaum.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_h_datenexportieren.htm
- **Kabelbaum-Navigator**: je Baum der erste Artikel und die Komponenten nach Kategorien der Funktionsdefinition. „Sonstige Kabelbaumkomponenten“ = weitere Artikel. Bei Kabelverbindungen steht Quelle → Ziel. Artikel mit „Artikelzuordnung“ = Zubehör (Quelle/Ziel) bekommen eine eigene Ebene. „Umbenennen“ ändert den Kabelbaumnamen samt Komponenten. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_d_navigator.htm

**Bedeutung für QET:** Ein Kabelbaum ist in EPLAN eine **Zuordnung per Name** (ID 31143), kein grafisches Objekt. Er hängt an Leitern, Kabeln und Steckerkontakten. Im QET-Nachbau ist das eine Zusatzinformation (z. B. Leiter- oder Bauteil-Info „Kabelbaum“). Wichtig wird sie nur, wenn das Original eine Kabelbaum-Stückliste oder Beschriftung mit Kabelbaumname enthält **(Vermutung zur Abbildung)**. Das HDEF-Symbol im Stromlaufplan als Text/Rahmen nachzeichnen.

## 7. Beschriftung (Labeling) und Exportformate

### 7.1 Zweck und Ablauf

- „Fertigungsdatenexport / Beschriftung“ gibt identifizierende und beschreibende Daten von Bauteilen und Verbindungen für Etiketten und Schilder aus. Es gibt vordefinierte und eigene **Schemata** und eine wählbare Sprache. Ausgabe als ***.txt** oder **Excel**, optional mit Excel-Vorlage und direktem Start von Excel. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_start.htm
- Ablauf: Schema anlegen → Kopf-, Etiketten- und Fußbereich festlegen → txt- oder Excel-Einstellungen → Ausgabe (Umfang, Reihenfolge). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_vorgehensweise.htm
- Ein Schema gehört zu genau **einem Auswertungstyp**, der sich **nicht nachträglich ändern** lässt. Die Auswertungstypen entsprechen den Formulartypen (Klemmenplan, Kabelplan, Artikelsummenstückliste …). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_auswertungstyp.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_h_beschriftungsschema.htm
- Die Schemata liegen in den **Benutzereinstellungen** (Optionen > Einstellungen > Benutzer > Schnittstellen > Fertigungsdatenexport / Beschriftung). Registerkarten: Datei, Kopfbereich/Fußbereich, Etikett, Einstellungen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_beschriftungeinstell.htm
- Über dieselbe Funktion entstehen auch **externe Auswertungen** (Projektdaten in eine Datei statt auf Formularseiten). Office-Formate setzen ein freigegebenes, installiertes Microsoft Office voraus. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_externeauswertungen.htm

### 7.2 Aufbau der Datei und Formatelemente

- Eine Datei besteht aus einer oder mehreren **Seiten**, jede mit **Kopfbereich** (einmal oder auf jeder Seite), **Etikettenbereich** (Etikettenreihen aus nebeneinander liegenden Etiketten) und **Fußbereich** (einmal oder auf jeder Seite). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_aufbaubeschriftung.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_r_kopfzeile.htm
- **Formatelemente** (abhängig vom Auswertungstyp):
  - In allen Bereichen: Eigenschaft, Trennzeichen, Zeilenumbruch.
  - Nur im Fußbereich: **Seitenumbruch** und **Seitenumbruch bei BMK-Wechsel**.
  - Nur im Etikett: **Berechnung** und **Aktion** (kundenspezifische Erweiterung).

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_formatelemente.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_h_formatelemfestlegen.htm
- **Eigenschaft** formatieren:
  - Länge: unbegrenzt variabel, begrenzt variabel (kürzt auf n Zeichen) oder fest (n Zeichen, optional rechtsbündig).
  - „Umbrüche ausgeben“: Absatzmarken im Text werden zu Zeilenumbrüchen.
  - „Trennzeichen bei Index 0“: Ein übergeordnetes Platzhalterelement (z. B. „Klemmenartikel“ = alle Artikel der Leiste, Index 0 statt [1]–[10]) gibt seine Werte mit Trennzeichen aus.
  - „In einer Zeile summieren“ (nur Artikelsummenstückliste): Stückzahlen oder Gesamtlängen werden summiert.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_eigenschaft.htm
- **Trennzeichen**: Leerzeichen, Tab, Zeilenumbruch oder beliebige Zeichen (Zähler zeigt die Anzahl, hilfreich bei mehreren Leerzeichen). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_trennzeichen.htm
- **Berechnung**: Formel aus Platzhaltertexten, + − * /, Klammern und Zahlen (Beispiel der Hilfe: Gesamteinkaufspreis × (100 − Rabatt) / 100). Ergebnisformat mit Mindeststellenzahl, Füllzeichen (4 Stellen mit „0“ → „0001“), Nachkommastellen (gerundet) und „Nachkommastellen variabel“. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_format.htm
- Registerkarte **Etikett**:
  - „Etiketten nebeneinander“ (n Datensätze je Zeile, mit Trennzeichen dazwischen, z. B. `K1 ANL1;K2 ANL2`).
  - „Etikettenreihen auf einer Seite“ (1 = eine Seite je Objekt; bei Excel beginnt danach ein neues Tabellenblatt; höchstens 1 048 500 Zeilen).
  - Bei Excel mit Vorlage braucht **jede Spalte einen Platzhalter**: 3 Eigenschaften × 3 Etiketten = 9 Platzhalterspalten.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_r_datensatz.htm

### 7.3 Dateitypen und Excel-Vorlagen

- Dateitypen (Registerkarte Datei): **Textdatei (*.txt)**, **Excel (*.xls = 97–2003, *.xlsx ohne Makros, *.xlsm mit Makros, *.xlsb binär)** und **XML (*.xml)**. Bei Excel ist eine Vorlage möglich. [Optionen] öffnet die Text- bzw. Excel-Export-Einstellungen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_r_datei.htm
- txt-Ausgabe: Zeichensatz (ANSI für Windows-Anwendungen), „Umlaute zulassen“, Anwendungsprogramm zum Öffnen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_h_textausgabe.htm
- Excel-Ausgabe: Dateityp Excel, Zieldatei, Vorlage. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_h_exceldatei.htm
- **Excel-Vorlagen**:
  - Platzhalter in Zellen: **`#H#`** für Formatelemente des Kopfbereichs, **`###`** für Etikett-Formatelemente, **`#F#`** für den Fußbereich.
  - Ausgewertet wird von oben nach unten und von links nach rechts. Je ausgewähltem Formatelement muss es genau einen Platzhalter geben.
  - Nach dem Export werden die Spalten auf optimale Breite gesetzt.
  - „**Seitenumbruch bei BMK-Wechsel**“ beginnt bei jedem BMK-Wechsel (z. B. je Klemmenleiste) ein neues Arbeitsblatt und schreibt das **BMK in den Blattnamen**.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_excelexportvorlagen.htm

### 7.4 Ausgabe und Artikelauswertung

- Ausgabedialog:
  - Schema; Auswertungstyp (nur Anzeige).
  - **Sprache**: „Alle Anzeigesprachen“ (mehrsprachig) oder eine Sprache. Bei Excel wählt die Sprachkennung das **Tabellenblatt** (z. B. `en_US`, `de_DE`). Fehlt es, nimmt EPLAN das Blatt der Dialogsprache, dann das der Alternativsprache, dann das erste.
  - Zieldatei; **Wiederholungen pro Etikett** und **der Gesamtausgabe**; „Exportieren“ bzw. „Exportieren und Anwendung starten“.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_beschriftungausgabe.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_h_beschriftungausgeben.htm
- **Artikelauswertungen hängen an der Auswahl**: Markierte Klemmen ergeben Klemmenartikel. Markierte Klemmenleisten ergeben Klemmen- und Leistenartikel. Beides zusammen zählt Klemmenartikel nur einmal. Für Kabel und Verbindungen gilt dasselbe. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_beschriftungausgabe.htm
- Registerkarte **Einstellungen**:
  - Baugruppen/Module bis Stufe n auflösen.
  - **„Artikel eines Betriebsmittels zusammenfassen“** (Artikelstückliste). Beispiel: Ist die Option an, gibt es eine Position „25 × Klemme“ und „10 Aderhülsen“, sonst 25 Einzelzeilen.
  - Filter- und Sortierschemata, Aktion.
  - „Artikel berücksichtigen“ je Artikelart. Die Einstellungen für BMK ohne Artikelnummer, Klemmenleisten-, Klemmen-, Kabel-, Stecker-, Steckerkontakt-, Sammelschienen- und Sammelschienenanschluss-Artikel gelten auch für die Betriebsmittelliste. „Vorplanungsartikel“ zählt nur ohne Hauptfunktion oder Artikelplatzierung.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_paramartikelauswertung.htm
- Die Beschriftungstexte der Drahtkonfektionierung (Komax, Schleuniger) werden im Dialog „Formatelemente auswählen“ mit Vorschau zusammengesetzt. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_formatelementeauswaehlen.htm

**Bedeutung für QET:** Die EPLAN-Beschriftung ist eine reine Dateiausgabe und nicht Teil des PDF. Für einen QET-Export von Etiketten (Klemmen, BMK, Drähte, Kabel) ist das Modell aber eine gute Vorlage: ein Schema je Auswertungstyp, Kopf/Etikett/Fuß, Formatelemente mit Länge, fester Breite und Trennzeichen, „Seitenumbruch bei BMK-Wechsel“ (z. B. ein Excel-Blatt je Klemmenleiste mit BMK als Blattname), Excel-Vorlagen mit `#H#`/`###`/`#F#`. Eine Umsetzung im Fork (z. B. als Skript-Button, der CSV/XLSX aus Klemmleisten schreibt) sollte diese Platzhalterlogik übernehmen, dann lassen sich vorhandene EPLAN-Excel-Vorlagen weiterverwenden **(Vorschlag)**. Wichtig: Die Ausgabe folgt der **Auswahl**, also in QET erst Leiste/Seiten wählen, dann exportieren.

## 8. Zusammenfassung für den QET-Nachbau

- Topologie-Seiten, 3D-Bauraum, Drahtkonfektionierung und Beschriftung erzeugen im EPLAN-PDF höchstens **Eigenschaftswerte** (Länge, Verlegeweg, Verlegeart, Bündel, Kabelbaumname) und **Auswertungsblätter** (Streckenliste, Streckenplan, Verlegte Kabel, Verbindungsliste, Kabeldarstellungsplan). Diese Werte wörtlich übernehmen und nicht nachrechnen. Quellen: siehe Abschnitte 2–7.
- Topologie-Symbole und Verlegeverbindungen sind **weitere Darstellungsarten** desselben Betriebsmittels bzw. derselben Verbindung. Sie dürfen in QET keine zusätzlichen Geräte, Leiter, Querverweise oder Klemmenziele erzeugen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_topologiefunktionen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_start.htm
- Phasenschienen-Definitionslinien sind keine Kabel. Phasenschienenverbindungen gehören nicht in Drahtlisten. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_phasenschienenverbdg.htm
- Optimierte Netze (Kette, max. zwei Drähte je Anschluss) können in Verbindungs- und Drahtlisten andere Quelle/Ziel-Paare zeigen als die Zeichnung. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_netzautooptimieren.htm
- Für eigene Exporte aus QET (Drahtliste, Etiketten) liefern die EPLAN-Spaltenlisten und das Platzhaltermodell der Excel-Vorlagen eine erprobte Struktur. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_k_start.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_excelexportvorlagen.htm

## 9. Quellen

Alle ausgewerteten Seiten (Plattform 2.9, Präfix `https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/`), vollständig ausgeschrieben:


**Topologie**

- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_d_funktionenerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_d_kabelverbindungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_d_navigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_fuellgradbestimmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_streckeeinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_strukturenverbinden.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_verbindungsfilter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_verlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_verlegepunkteeinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_h_verlegewegaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_anschlussbild.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_bestelllaenge.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_funktioneneinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_laengenermittlung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_strecken.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_teilstrecke.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_topologiefunktionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegeart.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegepunkte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verlegeverbindungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_verteilertopologie.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablinggui_k_vorgehensweise.htm

**Routing im Bauraum und Phasenschienen**

- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_autobuendeln.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_einstellungenbuendeln.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_netzeoptimieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_netzeoptimierenvorschau.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_netzmanuelloptimieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_neuesbuendeleigenschaft.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_phasenschienenvbdgerzg.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_phasenschienezuweisen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_sortierungbuendeln.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_verbindungenbuendelnvorschau.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_verlegerichtungaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_d_verlegeverbindung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_anschlussansicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_autobuendeln.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_autostreckenloeschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_eintrittinsstreckennetz.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_freiverlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_fuellgradanzeigen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_kurveeinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_kurveneuerstuetzpunkt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_kurvenverlaufaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_kurvenverlauftangential.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_netzautooptimieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_netzmanuelloptimieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_phasenschieneneinfg.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_phasenschienenvbdgerzg.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_streckeeinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_streckenansicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_streckennetzerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_streckennetzverfolgung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verbindungsfilter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verdrahtungsdurchbrucheinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verlegebereicheinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verlegeverbindungeinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_h_verlegewegaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_abstaende.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_phasenschienenverbdg.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_verdrahtungssystem.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/routinggui_k_verlegezubehoer.htm

**Fertigungsverdrahtung**

- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_d_ausgabeentfallstueckliste.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_d_einstellungendrahtkonfektionierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_d_exportdrahtdaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_h_drahtdatenexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_h_maschinekonfigurieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_h_smartwiringdatenexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_aderendbehandlung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_allgemein.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_beschriftung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_draehte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_draehtecadcabel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_draehtekomax.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_draehteschleuniger.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenallgemdrahtsl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungencadcabel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenkomax.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenmetzner.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenschleuniger.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungensteinhauerpwa.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_einstellungenwireterminal.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/productionwiringgui_r_maschinenbefehl.htm

**Kabelbaum**

- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_d_navigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_h_datenexportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_h_definitioneneinfuegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_h_importkabelbaum.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/harnessgui_k_verwaltungkabelbaum.htm

**Beschriftung**

- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_auswertungstyp.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_beschriftungausgabe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_beschriftungeinstell.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_eigenschaft.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_format.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_formatelementeauswaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_paramartikelauswertung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_d_trennzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_h_beschriftungausgeben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_h_beschriftungsschema.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_h_exceldatei.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_h_formatelemfestlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_h_textausgabe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_aufbaubeschriftung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_excelexportvorlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_externeauswertungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_formatelemente.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_k_vorgehensweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_r_datei.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_r_datensatz.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/labelgui_r_kopfzeile.htm
