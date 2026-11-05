# EPLAN: Kabel, Adern, Abschirmungen und Verbindungen

Referenz für den inhaltlich korrekten Nachbau von EPLAN-Kabeln (Kabeldefinition, Adern, Schirm), ihren Auswertungen (Kabelplan, Kabelanschlussplan, Kabelübersicht) und den Verbindungseigenschaften in QElectroTech (Fork). Grundlage ist die offizielle EPLAN-Hilfe (Plattform 2.9, abgeglichen mit 2023 bis 2026). Alles ist in eigenen Worten zusammengefasst. Jede Aussage nennt ihre Quelle. „Bedeutung für QET“ überträgt das auf den Fork. Mit **(Vermutung)** markierte Stellen sind nicht aus der Hilfe belegt.

Die Themenstruktur der Kabelhilfe ist von 2.9 bis 2026 gleich. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/cablegui_k_start.htm

## Inhalt

1. Grundprinzip: Kabel beruhen auf Verbindungen
2. Kabeldefinition (BMK, Typ, Verbindungsanzahl, Querschnitt, Länge)
3. Kabelverbindungen (Adern) und ihre Zuordnung
4. Grafische Darstellung: Kabeldefinitionslinie, verteilte Darstellung, einpolig
5. Abschirmung (Schirm)
6. Quelle und Ziel
7. Dialog „Kabel bearbeiten“, Adernzuordnung zum Artikel
8. Automatische Kabelerzeugung, -nummerierung, -auswahl
9. Verbindungen und Verbindungseigenschaften
10. Kabel und Klemmen
11. Auswertungen: Kabelplan, Kabelanschlussplan, Kabelübersicht, Kabeldarstellungsplan, Längen
12. Neuerungen neuerer Versionen
13. Projekteinstellungen „Kabel“: Sortierung, geschachtelte Ziele, Einheiten, Standardkabel
14. Kabel-BMK-Format und Nummerierungsreihenfolge
15. Automatisch erzeugte Kabel: Ablauf, Kennzeichnung, Löschen
16. Automatische Kabelauswahl: Schema und Regeln
17. Kabel-Navigator
18. Kabellängen summieren (Dialog) und Kabeldarstellungsplan erzeugen
19. Darstellungseinstellungen für Kabel und Pakete
20. Quellen

---

## 1. Grundprinzip: Kabel beruhen auf Verbindungen

- Die Kabelfunktion baut auf Verbindungen auf. Kabelverbindungen werden über Verbindungsdefinitionspunkte festgelegt. Ein Kabel kann auch nicht-aderbezogene Verbindungen enthalten (Hybridkabel mit LWL/Schläuchen). Kabellängen werden je Kabeltyp summiert. Reserveadern und Mehrfachabschirmungen werden verwaltet. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_start.htm
- Verbindungen werden beim Zeichnen nicht sofort aktualisiert; Kabel lassen sich erst nach „Verbindungen aktualisieren“ erzeugen. Kabelverbindungen entstehen über Verbindungsdefinitionspunkte oder über die Eigenschaft **Kabelanschluss** in der Anschlusslogik einer Funktion. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_prinzip.htm
- Drei Wege, eine Verbindung zur Kabelverbindung zu machen: (1) Kabeldefinitionslinie darüberziehen (Funktionsdefinition wird „Ader / Draht“, **Verbindung: Zugehörigkeit = Kabel**), (2) Verbindungsdefinitionspunkt setzen und Zugehörigkeit „Kabel“ wählen, (3) Kabelanschluss in der Anschlusslogik. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_arbeitsweise.htm
- Die Einstellung am Verbindungsdefinitionspunkt hat Vorrang vor der Anschlusseinstellung. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_aderndefinieren.htm

**Bedeutung für QET:** Der Fork hat ein projektweites `Cable`-Objekt (`sources/cable/cable.h`): Bezeichnung, BMK, Anlage (installation), Ort (location), Länge, Typ, Aderzahl, Aderfarben je Ader und gezeichnete Teile (`CablePartData` = Stammlinie je Blatt). Jede belegte Ader ist ein `CableCore` (Adernummer, Teil, Leiter-UUID). Der Leiter selbst trägt nur den daraus erzeugten Text (z. B. „W1:BN“, `Cable::labelOfCore`) sowie `m_cable_uuid`/`m_cable_slot`. EPLAN-„Zugehörigkeit = Kabel“ bedeutet in QET also, dass der Leiter einer Ader eines `Cable` zugeordnet ist. Ein Kabel nur als Grafik oder Text erscheint in keiner Auswertung.

## 2. Kabeldefinition

- Ein Kabel wird durch eine **Kabeldefinition** repräsentiert, grafisch als Kabeldefinitionslinie oder Abschirmung. Kabel lassen sich im Kabel-/Betriebsmittel-Navigator ohne Grafik vordefinieren (Typ und Eigenschaften wie Verbindungsanzahl, Querschnitt eindeutig). Eine Länge kann vordefiniert werden (vorkonfektionierte Reservekabel). Ein nicht platziertes Kabel kann später auf Geräteanschlüsse oder Klemmen „aufgelegt“ werden. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_arbeitsweise.htm
- Registerkarte Kabel/Abschirmung: sichtbares/vollständiges BMK (bei Abschirmung leer), **Typ** (Auswahl eines Kabelartikels überschreibt die alten Werte ohne Rückfrage), Funktionstext, **Verbindungsanzahl**, **Länge** (ohne Einheit gilt die Projekteinstellung), **Verbindungsquerschnitt/-durchmesser**, Einheit, **Spannung**, Name der Abschirmung (nur Abschirmung), Hauptfunktion. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_kabeldaten.htm
- Schreibweise der **Verbindungsanzahl**: „4PE“ = 4 gesamt, davon eine PE/PEN/N-Ader. „4G“/„4X“ = nur die Zahl zählt. „3x3“ = multipliziert (9). „3+3“ = addiert (6). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_kabeldaten.htm
- Kabel: Quelle und Kabel: Ziel, Farbe/Nummer der Kabelverbindung, Potenzialtyp (z. B. SH) und Name der Abschirmung sind die zentralen Eigenschaften. Für Kabel gibt es eigene BMK-Formate (z. B. Quelle und Ziel im Kabel-BMK). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_prinzip.htm
- Alternative Angaben „Kabel / Paket: Alternativer Querschnitt / Durchmesser“ und „Verbindungen: Alternativer Verbindungsquerschnitt“ sind unabhängig von der Artikelverwaltung, werden manuell gepflegt und sind auswertbar. Die Position der Kabeleigenschaften wird über die Eigenschaftsanordnung (vier Winkel, vordefinierte oder benutzerdefinierte Anordnung relativ zum Einfügepunkt) gesteuert. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_kabelbezeichnungformat.htm
- **Standardkabel** (Projekteinstellung): Eine gezeichnete Linie ohne vorhandenes Kabel-BMK erzeugt ein neues BMK. Die Verbindungen bekommen die passenden Kabelverbindungen des Standardkabels; Artikel und Kabeltyp werden nicht übernommen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_prinzip.htm

**Bedeutung für QET:** Abbildung: EPLAN-BMK (z. B. `-W12`) → `Cable::designation`/BMK. Typ inklusive Aderzahl x Querschnitt (z. B. „ÖLFLEX CLASSIC 110 4G1,5“) → `type` (wörtlich aus dem Plan, „4G“ bedeutet: grün-gelbe Ader enthalten). Aderzahl → `core_count`. Länge mit Einheit → `length`. Anlage/Ort → `installation`/`location`. Den Querschnitt hat QET nicht am Kabel, sondern am Leiter (`m_wire_section`). Bei „4G1,5“ muss jede Ader 1,5 tragen. Die Spannung hat im Fork-`Cable` kein eigenes Feld. Bei Bedarf gehört sie in den Typtext **(Vermutung)**. Die Darstellung „Typ links neben der Kabellinie“ folgt den Fork-Textfeldern (designation, type, installation, location, length einzeln ein-/ausblendbar).

## 3. Kabelverbindungen (Adern) und ihre Zuordnung

- Jede Kabelverbindung hat Quelle und Ziel. **Reserve-Kabelverbindungen** werden automatisch auf freie Klemmen der Leiste gelegt (Meldung, wenn keine frei ist). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_prinzip.htm
- Zugehörigkeit zum Kabel: Verbindungen gehören zu einem Kabel, wenn sie **dasselbe BMK** tragen. Schneidet eine Kabeldefinitionslinie eine Verbindung, setzt EPLAN dort einen (unsichtbaren) Verbindungsdefinitionspunkt (sichtbar über Ansicht > Einfügepunkte), der das BMK der Linie übernimmt. Übertragen werden sichtbares BMK, Länge, Querschnitt/Durchmesser und Einheit, sofern am Punkt leer. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_arbeitsweise.htm
- Manuell: Einfügen > Verbindungsdefinitionspunkt, Kabel-BMK ins sichtbare BMK, „Verbindung: Zugehörigkeit“ = Kabel. Mehrere Kabeldefinitionslinien mit gleichem BMK ergeben eine **verteilte Darstellung** des Kabels. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_aderndefinieren.htm
- **Identifizierende Eigenschaften** beim Zuordnen zum Kabelartikel: Funktionsdefinition, Abschirmungsbezeichnung („Abgeschirmt von“), **Paarindex**, Potenzialtyp, **Farbe/Nummer**, Querschnitt/Durchmesser. Gefüllte Artikelwerte müssen gleich sein, leere passen zu allem (z. B. Artikel 5 mm² passt nicht zu leerer Kabelverbindung, Artikel leer passt zu jedem Querschnitt). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelauflegen.htm
- Zuordnung „Alle neu zuordnen“ (vergleicht nur den Potenzialtyp und überschreibt alles mit Artikelwerten) vs. „Vorhandene Eigenschaften beibehalten“ (nur bei voller Übereinstimmung, ergänzt fehlende Werte). Die Reihenfolge wird dabei neu sortiert: bei Kabeln nach Quelle/Ziel-Regeln, bei Klemmen nach der Klemmenreihenfolge. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelauflegen.htm
- Der Artikel ordnet Adern unter Berücksichtigung der Anschlussart automatisch zu, z. B. **PE-/PEN-Adern zu PE-/PEN-Klemmen**. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_prinzip.htm
- **Paarindex**: Zusammengehörige Verbindungen haben denselben Index vor einem Trennpunkt („2“, „2.1“, „2.2“ gehören zusammen); genutzt für verdrillte Paare. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2023/Content/htm/news_p_kabel_kabelbearbeiten_355040.htm
- Hybridkabel: Punkte aus Linie/Abschirmung bekommen automatisch „Ader / Draht“, andere Anschlussarten manuell. Bei Widerspruch meldet das Aktualisieren einen Fehler. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_arbeitsweise.htm

**Bedeutung für QET:** Jede Ader im EPLAN-Plan (Strich mit Farbe/Nummer an der Kabellinie) wird eine QET-Ader (`CableCore`) des Kabels, die dem Leiter zugeordnet ist. Die Aderkennung (BN, BK, GY, GNYE bzw. 1, 2, 3 …) steht in `core_colors` des Kabeltyps und wird nie frei am Leiter getippt. Die Adernummer in QET ist 0-basiert und folgt der Reihenfolge im Kabeltyp. Für EPLAN-Pläne die Reihenfolge des Kabelartikels übernehmen (die stimmt mit dem EPLAN-Kabelplan überein), nicht die Zeichenreihenfolge. Reserveadern = Adern ohne Leiter (`freeCoreCount()`). EPLAN legt Reserven auf Klemmen, also im Klemmenplan auch die Reserve-Klemmen mit Kabelader übernehmen. Verdrillte Paare (Paarindex) hat QET nicht als Datum. Sie lassen sich nur grafisch (gekreuzte Adern) und über die Aderreihenfolge abbilden **(Vermutung, kein Feld im Fork gefunden)**.

## 4. Grafische Darstellung

- Kabel erscheinen als **Kabeldefinitionslinie** und/oder **Abschirmung**. Beide liegen auf eigener Ebene, Linientyp/-stärke/-farbe kommen aus den Schaltzeichen-Eigenschaften, beide sind unsichtbar schaltbar (Symbol vor Ebene) und querverweisfähig. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_grafischedarstellung.htm
- Linie orthogonal oder schräg (schräg: Meldung, aber unterstützt). Einfügepunkt ist das linke Ende, bei senkrechten Linien das obere. An jedem Schnittpunkt entsteht ein Verbindungsdefinitionspunkt. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_grafischedarstellung.htm
- Verteilte Darstellung: Gleiches BMK bindet die Teile zusammen. Die vollständige Definition (Typ, Verbindungsanzahl, Artikel) steht nur an der **Hauptfunktion**, alle anderen Darstellungen sind Nebenfunktionen mit nur dem BMK. Bei Widerspruch wird die grafisch erste Funktion Hauptfunktion (Prüflauf meldet). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_grafischedarstellung.htm
- Beim Verkleinern einer Linie verschwinden Punkte an nicht mehr gekreuzten Verbindungen, beim Vergrößern entstehen neue. Über eine Verbindung vom Typ „Kabel“ gezogen setzt EPLAN die Darstellungsart „Einpolig“. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelzeichnen.htm
- Einpolig: „Darstellungsart des Kabels“ folgt dem Seitentyp. Die Kabeleigenschaft „Anzahl der verwendeten Verbindungen (einpolige Kabel)“ zählt die Adern. Abschirmungen wirken auf einpoligen Seiten nur als Kabeldefinition. Angaben wie „3+PE“ (Schrägstrich-Anzahl) sind zulässig. Stränge entstehen über **Stranganschlüsse** (Einfügen > Stranganschluss). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_besonderheitenkabel.htm

**Bedeutung für QET:** Ein EPLAN-Kabel, das auf mehreren Blättern gezeichnet ist (gleiches BMK), wird in QET **ein** `Cable` mit mehreren `CablePartData`, wenn es wirklich dasselbe Kabel ist. Gleicher Name heißt dabei nicht automatisch dasselbe Kabel: Prüfen, ob die Stücke dieselben Endgeräte teilen. Typ und Länge nur einmal pflegen (Hauptfunktion). Auf Nebenblättern zeigt das Original oft nur das BMK, dort die Typ-/Längenanzeige ausblenden. Die Kabellinie quer über die Adern legen, damit jede Ader einen Schnittpunkt hat (`add_cable`).

## 5. Abschirmung (Schirm)

- Abschirmung wird wie ein Rechteck über die Verbindungen gezogen (senkrecht oder waagerecht). Jede Verbindung darin bekommt einen Verbindungsdefinitionspunkt. Das Abschirmungssymbol hat **einen Anschluss**, dessen Anschlussbezeichnung zugleich der **Name der Abschirmung** ist. Einfügepunkt am Ende mit Anschluss. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_grafischedarstellung.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelzeichnen.htm
- Gleicher Name und gleiches BMK bedeuten dieselbe Abschirmung (z. B. beidseitige Auflage). Verteilte Abschirmungen können unangeschlossen, einseitig oder per T-Stück beidseitig angeschlossen sein. Mehrere gleichnamige Abschirmungen an derselben Verbindung ergeben nur eine Kabelverbindung. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_grafischedarstellung.htm
- Abschirmungen sind nur zeichnerisch definiert; an einer Verbindung kann man nicht angeben, von welcher Abschirmung sie umgeben ist. Unbenannte Abschirmungen funktionieren, wenn die angeschlossene Verbindung eine Kabeldefinitionslinie schneidet (BMK kommt von dort). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_verhaltenabschirmungen.htm
- **Schirmanschluss**: Die Abschirmung wird an einer Verbindung aufgelegt (**Abschirmungsverbindung**). Diese bestimmt das BMK der Abschirmung und stammt aus einer Artikelfunktion „Ader / Draht“ mit **Potenzialtyp SH**. Der Name kommt aus „Farbe / Nr.“ dieser Verbindung. Abschirmungen nicht als eigene Funktionsschablone im Artikel anlegen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_verhaltenabschirmungen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_prinzip.htm
- Einseitig aufgelegte Abschirmungen sind selbst das Ziel der Verbindung. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_prinzipverbindungen.htm
- Mehrfachabschirmungen ohne Namen erkennt EPLAN erst nach zweimaliger Zuordnung im Dialog „Kabel bearbeiten“ (Namen kommen aus dem Artikel). Zweiseitig aufgelegte Abschirmung mit Darstellungsart „Extern“ macht die geschirmte Verbindung extern (nicht im Verbindungen-Navigator). Bei automatischer Kabelerzeugung übernimmt die Abschirmung das BMK der Abschirmungsverbindung (Beispiel der Hilfe: W8 → W7). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_verhaltenabschirmungen.htm
- Beim Zuordnen zählt zuerst der Abschirmungsname. Ungeschirmte Verbindungen dürfen auf geschirmte Schablonen, aber nicht in eine andere Abschirmung. SH-Adern tauschen nur untereinander. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_adernzuordnen.htm

**Bedeutung für QET:** Den Schirm als eigene Ader mit Kennung „SH“ (bzw. dem Schirmnamen aus dem Original) im Kabeltyp führen und ihren Leiter zum Schirmanschluss (PE-Schiene, SH-Klemme, Schirmklemme) zeichnen. Die Schirm-Ellipse um die Adern ist nur Grafik. An der Klemme markiert der Fork Schirmadern über `terminal_shield` (`RealTerminal::isShield()`), im Klemmleisten-Editor als Spalte „Shield“. Ein- oder beidseitige Auflage muss aus der Zeichnung erkennbar bleiben: einseitig = Schirmader nur an einem Ende angeschlossen.

## 6. Quelle und Ziel

- Regeln für **Kabel** (in dieser Reihenfolge): (1) Hierarchie aus Strukturkennzeichen zuerst. (2) Klemmen/Stecker mit gleichem Kennbuchstaben: kleinerer Zähler ist Quelle (X1 vor X4 vor X33). (3) Verschiedene Kennbuchstaben, keiner „X“: alphabetisch erster ist Quelle (A44 vor B34). (4) Ist einer „X“: **X ist immer Ziel** (Y4 → X33). (5) Keine Klemme/Stecker an beiden Enden: alphabetisch erster. (6) Klemme/Stecker nur an einem Ende: dieses Ende ist immer Quelle, auch gegen Regel 1. (7) Mehrere Ziele: Regeln auf die grafisch erste Kabelverbindung. (8) BMK wechselt an einem Ende: bleibt. (9) An beiden Enden: neu bestimmen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_quellezielermittlung.htm
- Hinweis: Regeln 4 und 6 widersprechen sich scheinbar. Regel 4 gilt laut Hilfe, wenn beide Enden Klemmen/Stecker sind, Regel 6, wenn nur ein Ende eine Klemme/ein Stecker ist **(Lesart, aus der Struktur der Hilfe abgeleitet)**. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_quellezielermittlung.htm
- Sortierung der Kabelverbindungen: eine Quelle → Reihenfolge im Schaltplan, bei Klemmenleiste als Quelle die Klemmensortierung. Mehrere Quellen → gruppenweise nach Quellen. Ausnahme Option „Kabelverbindungen immer nach Steckerreihenfolge sortieren“ (Ziel Stecker, Quelle kein Stecker). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_quellezielermittlung.htm
- Quelle/Ziel des Kabels ergeben sich aus den enthaltenen Kabelverbindungen (die Quelle einer Ader kann Ziel des Kabels sein). Bei Widerspruch gilt die grafisch erste Kabelverbindung. Kabel-Navigator (je Ader) und „Kabel bearbeiten“ (je Kabel) können verschiedene Quelle/Ziel zeigen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_prinzip.htm
- Für **Verbindungen** allgemein: Das „kleinere“ BMK ist Quelle (Strukturkennzeichen-Reihenfolge der Verwaltungstabelle, dann Kennbuchstabe/Anschlussbezeichnung, z. B. EB3+ET4-X1:2 vor -X1:5). Mehrere Verbindungen zwischen zwei Funktionen haben dieselbe Richtung. Alternativ grafisch (Verhalten vor 1.9 SP1; betrifft keine Kabelverbindungen). „Quelle und Ziel tauschen“ tauscht abhängige Eigenschaften mit (Verlegerichtung, Hülsenquerschnitt, Absetzlänge …). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_prinzipverbindungen.htm

**Bedeutung für QET:** Für Kabelplan/Kabelübersicht im QET-Nachbau Quelle und Ziel nach diesen Regeln berechnen. Dann stimmen die Spalten „von/nach“ mit dem EPLAN-PDF überein: Die Klemmleiste steht meist als Quelle bzw. bei Gerät↔Klemme die Klemme als Quelle (Regel 6), bei Gerät↔Gerät alphabetisch. Die Aderreihenfolge im Kabelplan entspricht bei Klemmen-Quelle der Klemmenreihenfolge. Weichen Original und Regel ab, hat das Original manuell sortiert. Dann gilt das Original.

## 7. Dialog „Kabel bearbeiten“, Adernzuordnung zum Artikel

- Zweck: Verbindungen des Kabelartikels (Funktionsschablonen) den Kabelverbindungen des Projekts zuordnen. Kabel mit Darstellungsart „Extern“/„Grafik“ werden nicht gezeigt. Kontrollkästchen „Verbindung dieses Kabels“ (aktiv: Kabelname und Zugehörigkeit „Kabel“, inaktiv: BMK entfernt). Kontextmenü: Neu, Löschen (nur nicht platzierte), **Kabelverbindungen sammeln** (gleiche Quelle/Ziel, noch ohne Kabel), Kabelverbindungen zuordnen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabel.htm
- Neuer Dialog (ab 2023, Stand 2025): Tabelle **Kabelinformationen** (Artikelnummer, Länge mit Einheit, weitere Spalten wie Quelle/Ziel per „Darstellung konfigurieren“) und Tabelle **Verbindungen** (Status, Verbindungsfarbe/-nummer mit Auswahl aus „Verbindungsfarben“, Farbe (Grafik), Abschirmung (Grafik), Abgeschirmt von, Paarindex (Grafik), Verbindung: Zugehörigkeit = Kabel/Einzelverbindung). Ohne Artikel: benannte Verbindungen alphanumerisch, unbenannte grafisch sortiert. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/cablegui_d_kabelbearbeiten.htm
- Manuelles Zuordnen: Schablonenreihenfolge (links) bleibt fest, nur Verbindungen werden verschoben/getauscht. Beim Tauschen wechseln Paarindex, Farbe/Nummer und Querschnitt an den Definitionspunkten. PE/PEN und SH nur auf gleichen Potenzialtyp. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_adernzuordnen.htm

**Bedeutung für QET:** Die Adernreihenfolge eines Kabels in QET ist die Reihenfolge des Kabeltyps (Artikel). Beim Nachbau eines EPLAN-Kabels die Aderliste aus dem EPLAN-Kabelplan (der die Artikelreihenfolge zeigt) in `core_colors` eintragen und die Leiter danach den Adern zuordnen. Unbekannte Farbcodes zeigt EPLAN als schwarze, beschriftete Grafik, also den Text unverändert übernehmen.

## 8. Automatische Kabelerzeugung, -nummerierung, -auswahl

- **Kabel automatisch erzeugen**: ordnet freie Kabelverbindungen passenden Kabeln zu oder legt neue an, nummeriert sie und wählt Artikel. Voraussetzung: Zugehörigkeit „Kabel“ oder Kabelanschluss am Quellanschluss, Verbindungen aktualisiert. Berücksichtigt nur Verbindungen ohne vollständiges BMK. **Abschirmungen werden nicht automatisch erzeugt.** Verbindungen mit grafischem/externem Ziel bekommen kein Kabel. Je Kabel wird nur an der grafisch ersten Verbindung eine Linie gezeichnet, an den anderen nur Definitionspunkte mit gleichem BMK. Automatisch erzeugte Objekte werden beim nächsten Lauf gelöscht und neu gezeichnet (Umwandeln: Projektdaten > Kabel > Automatisch -> Manuell). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelerzeugen.htm
- Einstellungen Kabelerzeugung: Eigenschaften an Linie und/oder Definitionspunkt platzieren, „Nur die vorhandenen Kabel vervollständigen“, **„Hauptfunktionen zwischen gleichen Zielen zusammenfassen“** (Standard an: zwei Linien zwischen denselben Zielen = ein Kabel mit den Daten der ersten). Gesammelt werden Kabelverbindungen zwischen denselben Zielen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenkabelerzeugen.htm
- **Kabel nummerieren**: Schema (Reihenfolge, Eigenschaften im BMK, Format), Startwert, Schrittweite, „Vorhandene Kabel-BMK beibehalten“ (nur „?“-Kennbuchstaben neu: W2 ?W W3 W5 → W2 W1 W3 W5), „Kennbuchstabe beibehalten“ (WB2 WB3 WD3 ?WD3 WD5 → WB1 WB2 WD1 WD2 WD3). Geschachtelte BMK sind ausgenommen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/cablegui_d_kabelnummerierung.htm
- **Automatische Kabelauswahl** ersetzt nur den Hauptartikel (erster Artikel der Produktgruppe Kabel), überspringt Kabel mit „Nur manuelle Kabelauswahl zulassen“. Kriterien: Spannung (Kabel und über Potenziale), Verbindungsanzahl, Reserveadern, Hersteller. PE-Adern-Feld (0/1/2). Gibt es kein Kabel ohne PE, wird eines mit PE gewählt (Ader kommt hinzu). Fehlender Querschnitt wird spannungsabhängig ergänzt. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelauswahlautomatisch.htm
- Abbuchen: Linien nur mit Linien, Verbindungen nur mit Verbindungen. Bei einem Definitionspunkt mit Zugehörigkeit „Kabel“ öffnet [...] die BMK-Auswahl für Kabel. „Nächste“ erzeugt ein neues BMK oder eine weitere Kabeldefinition zum selben BMK. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_abbuchen.htm

**Bedeutung für QET:** Beim Nachbau nichts automatisch erzeugen oder umnummerieren: Kabel-BMK, Typ und Aderzuordnung kommen wörtlich aus dem PDF. Unbenannte Kabel des Originals nicht erfinden. Der Fork hat eine Umnummerierung (`cable/cablerenumber`), die man hier nicht einsetzt. Ein EPLAN-PDF mit Kabel ohne sichtbares BMK auf einem Blatt hat es meist nur auf dem grafisch ersten Blatt. Dann über gleiche Endgeräte zusammenführen.

## 9. Verbindungen und Verbindungseigenschaften

- Autoconnect-Linien sind zunächst nur Grafik. Daten tragende Verbindungen entstehen beim Aktualisieren (z. B. Seite öffnen) oder manuell. Definition auf vier Wegen: Verbindungsdefinitionspunkt (auch durch Verbindungsnummerierung), Kabeldefinitionslinie, Vergleich Quelle/Ziel vorhandener Kabel, Standardwerte (Erbe von Potenzial/Projekt). Verbindungsart (Kabel, Draht, Brücke, Drahtbrücke, Stegbrücke, Sammelschiene) ist Eigenschaft der Verbindung. Wird eine definierte Verbindung durch ein Schaltzeichen unterbrochen, behält nur die Seite mit dem Definitionspunkt ihre Daten. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm
- **Vererbungsreihenfolge** der Verbindungseigenschaften (spätere Stufe gewinnt): Anschlüsse (Anschlussart + Potenzialtyp) → Projekt → Ebene → Klemmenziele → Potenzial → Rohrleitungsdefinition → Signal → Netz → Brückenerzeugung → Abschirmung → **Verbindungsdefinitionspunkt**. Steht am Quellanschluss „Undefiniert“, gilt der Zielanschluss. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm
- **Farbe/Nummer**: Code wird über die Zuordnungstabelle (Projekt) einem Farbnamen zugeordnet. Groß-/Kleinschreibung zählt („BK“ → Schwarz, „bk“ bleibt Text). Die mehrsprachige „Verbindungsfarbe“ aktualisiert sich selbst. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm . Die Farbcodes folgen **IEC 60757**, die Farbnamen sind übersetzbar, und es gibt eine Bauraum-Anzeigefarbe je Code. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_d_einstellungenaderfarben.htm
- Funktionsdefinition der Verbindung (Draht, Brücke, Undefiniert …) kommt aus der Anschlussart oder vom Definitionspunkt (Punkt hat Vorrang). Quelle/Ziel ergeben sich aus der Lage. Winkel/T-Stücke sind keine Ziele (Ausnahme: runde Knoten bei Sammelschienen und Abschirmungen). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm
- Verbindungsdefinitionspunkt: ergänzt/ersetzt Eigenschaften und hat Vorrang vor Potenzial/Projekt. Mehrere Punkte je Verbindung sind möglich, Widersprüche meldet die Prüfung. Die Bezeichnung kommt vom grafisch ersten Punkt mit nicht nur „?“. Die Verbindungsnummerierung setzt bei mehrseitigen oder > 3-teiligen Verbindungen je einen Punkt an Anfang und Ende. In Makros werden nur Punkte gespeichert, nicht Verbindungen. **Auswertungen über Verbindungen erzeugen, nicht über Punkte** (sonst Länge/Artikel nur einmal statt zweimal). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_vdp.htm
- Querschnitt/Durchmesser, Farbe/Nummer und Verbindungsbezeichnung werden an Definitionspunkten hinterlegt und sind in Formularen auswertbar. Platzierte Verbindungen erlauben kein Ändern von Quelle/Ziel im Dialog. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm

**Bedeutung für QET:** EPLAN-Verbindungseigenschaften → QET-Leitereigenschaften (`sources/conductorproperties.h`): Farbe/Nummer → `m_wire_color` (Text, z. B. „BK“, „GNYE“, „1“). Querschnitt → `m_wire_section`. Verbindungsbezeichnung → `text` bzw. Formel. Funktion → `m_function`. Spannung/Protokoll → `m_tension_protocol`. Aderart (H05V-K …) → `m_conductor_type`. Potenzialtyp (L, N, PE …) → `m_potential_type`. Kabelader → `m_cable_uuid` + `m_cable_slot`. Den EPLAN-Definitionspunkt (Schrägstrich mit „BK 1,5“) bildet der Fork als `m_definition_mark` (schräger Strich mit Farbe und Querschnitt, Position `m_definition_mark_at`, optional statt des Leitertexts) ab. Da EPLAN die Daten über Potenzial und Projekt vererbt, steht im PDF oft nur an einzelnen Leitern ein Definitionspunkt. Für korrekte Auswertungen die Werte trotzdem an alle Leiterstücke derselben Verbindung schreiben, die Markierung aber nur dort anzeigen, wo das Original sie zeigt.

## 10. Kabel und Klemmen

- Der Klemmenanschluss mit Kontrollkästchen **Kabelanschluss** (Registerkarte Verbindungen) macht die Verbindung zur Kabelverbindung (für automatische Kabelerzeugung). Der Potenzialtyp des Anschlusses steuert die Aderzuordnung. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_verbindungenklemmen.htm
- Im Dialog „Klemmenleiste bearbeiten“ zeigen die Spalten **Kabel (extern/intern)** Kabel-BMK und Verbindungsfarbe/-nummer je Klemmenseite. Sortierung „Nach externen Kabeln“ setzt Klemmen eines Kabels nebeneinander (nur für schon zugeordnete Verbindungen). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- Klemmennummerierung „Kabelorientiert“ folgt den aufgelegten Kabelverbindungen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalplugnumberinggui_d_klemmennummerieren.htm
- Ist die Quelle eines Kabels eine Klemmenleiste, folgt die Aderreihenfolge der Klemmensortierung. Reserveadern liegen auf freien Klemmen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_quellezielermittlung.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_prinzip.htm

**Bedeutung für QET:** Im Fork ermittelt `RealTerminal::automaticCableTarget()` aus dem Projekt, wo die Adern eines Kabels landen. Der Klemmleisten-Editor zeigt Kabel und Ader je Klemme (`terminal_cable`, `terminal_cable_wire`), bei Bedarf auch handschriftlich. Für einen EPLAN-treuen Klemmenplan das Kabel in der Spalte der richtigen Klemmenseite (meist extern) zeigen. Die Kabelspalte trägt im Kopf das Kabel-BMK (und Typ), in den Zeilen die Aderkennung.

## 11. Auswertungen

- **Kabelplan**: listet die Kabelverbindungen der Kabel auf Basis der Kabeleigenschaften. **Kabelanschlussplan**: gleiche Daten, andere Formulare, mehrere Ebenen, Sortierung nach Kabeln. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_funktionsbezogen.htm
- **Kabelübersicht** (*.f10): Liste aller Kabel. **Verbindungsliste** (*.f27): Liste aller Verbindungen. **Kabeldarstellungsplan** (*.f08): Aufbau eines Kabels inklusive Beschaltung an den Enden (z. B. Stecker) und Verwendung im Projekt. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm
- Kabeldarstellungsplan: nur Kabel mit eingetragenem **Kabeldarstellungsplanformular**. Je Formularname ein Auswertungsblock, Sortierung nur innerhalb des Blocks. Den Kabelplan kann man per Filter auf Kabel ohne dieses Formular beschränken. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_kabeldarstellungsplan.htm
- Kabelartikel in Stücklisten: Einstellung, welche Kabelartikel einfließen (an Linie/Abschirmung, Kabelprojektartikel aus der Längensummierung, Verbindungsartikel). Filter für Kabel mit Stückzahl 0. „Ausgabe von Kabeln einschränken“ ab Mindestaderzahl. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelauswertung.htm
- **Kabellängen summieren**: je vollständigem Kabeltyp (physikalische Eigenschaften, nicht Artikel) projektweit addiert und als Kabelprojektartikel gespeichert. Die Stückzahl der Einzelkabel wird 0. Der Dialog zeigt Gesamtlänge, längstes Einzelkabel, Trommelzahl (3 × 57 m → 3 Trommeln à 100 m). Ausschluss über „Kabel beim Summieren der Kabellängen nicht berücksichtigen“. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelmengenberechnen.htm
- Funktionsbezogene Auswertungen (Kabelplan): Kopf = Kabel, Daten = Adern. Seitensortierung, Filter-, Sortier- und Seitenumbruch-Schemata wie beim Klemmenplan. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_funkbezogauswerterzeugen.htm

**Bedeutung für QET:** Der Fork hat eine Kabelliste (`sources/cablelist/`: Modell, Abfrage, Export, Kabeltypliste). Für EPLAN-gleiche Blätter erzeugt man Kabelübersicht und Kabelplan als echte Blätter aus den QET-Kabeldaten, mit Sprungzielen zu Gerät und Klemme. Kabeltyp, Querschnitt und Länge kommen aus dem Stromlaufplan. Die EPLAN-Kabelübersicht ist nicht immer nachgeführt, die Länge steht oft nur dort. Abweichungen zwischen Übersicht und Stromlaufplan im Bericht nennen. Längensummen (Kabelprojektartikel) erscheinen nur in der EPLAN-Stückliste, nicht am einzelnen Kabel.

## 12. Neuerungen neuerer Versionen

- **2023:** Neuer Dialog „Kabel bearbeiten“: gerätebezogene Eigenschaften (Artikel, Länge, Quelle/Ziel einblendbar), Schablonen und Verbindungen in einer gemeinsamen Tabelle, grafische Spalten für Farbe, Abschirmung und Paarindex, Vorschau mit Artikelbild bzw. Fundstelle. Der alte Dialog bleibt als „Kabel bearbeiten (Kompatibilität)“ erhalten. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2023/Content/htm/news_p_kabel_kabelbearbeiten_355040.htm
- **2025:** Aufruf über Registerkarte Verbindungen > Befehlsgruppe Kabel (Menüband). Inhalt wie 2023. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/cablegui_d_kabelbearbeiten.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/cablegui_d_kabelnummerierung.htm
- **2026:** Themenstruktur der Kabelhilfe unverändert. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/cablegui_k_start.htm
- Laut einer Pressemitteilung zur Plattform 2024 (nicht eplan.help) können Reserveadern automatisch angeschlossen werden. In der Hilfe ist das nicht nachgeprüft. Quelle: https://rittal.com/imf/none/24_4357/Presseinfo%20Eplan%20Plattform%202024

## 13. Projekteinstellungen „Kabel“: Sortierung, geschachtelte Ziele, Einheiten, Standardkabel

- **„Kabelverbindungen immer nach Steckerreihenfolge sortieren“**: Ist die Option an, folgen Kabelverbindungen zwischen Klemmen und Steckern immer der Reihenfolge der Steckerkontaktbezeichnungen. Ist sie aus, sind Klemme und Stecker gleichberechtigt, und es gilt die Sortierung an der Quelle. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenkabelmanuell.htm
- **„Bei geschachtelten Zielen untergeordnete BMK ignorieren“** wirkt auf Kabelnummerierung, automatische Kabelerzeugung und Kabelauswertungen. Ist die Option an, gilt das übergeordnete Betriebsmittel (z. B. Gerätekasten) als Kabelziel. Die Zielanzeige der einzelnen Kabelverbindungen zeigt weiter das untergeordnete BMK. Beispiel der Hilfe: Verbindungen zwischen -U1-X1/-U2-X3 und -U1-X2/-U2-X4 ergeben ohne die Option zwei Kabel, mit der Option ein Kabel -U1↔-U2. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenkabelmanuell.htm
- **Maßeinheit für Kabellänge**: gilt in Kabelplänen für Längen ohne Einheit. Im Schaltplan bleibt der Eintrag unverändert. „Querschnitte mit Leerzeichen“ und „Längen mit Leerzeichen“ steuern, ob zwischen Wert und Einheit ein Leerzeichen steht („1,5 mm²“ vs. „1,5mm²“, „12 m“ vs. „12m“). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenkabelmanuell.htm
- **Standardkabel** im Detail:
  - Schneidet die neue Kabellinie nur Verbindungen ohne Kabel, ordnet EPLAN die Kabelverbindungen des Standardkabels über die identifizierenden Eigenschaften zu. Übertragen werden nur die Schabloneneigenschaften, also kein Artikel und kein Typ.
  - Gehört schon eine geschnittene Verbindung zu einem Kabel, übernimmt die Linie das erste gefundene Kabel-BMK. Die neuen Verbindungen bekommen dann freie Adern dieses Kabels.
  - Ohne Standardkabel ordnet EPLAN beim Zeichnen nichts zu.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenkabelmanuell.htm

**Bedeutung für QET:** Längen- und Querschnittsangaben im Kabelplan wörtlich mit oder ohne Leerzeichen übernehmen, wie im Original. Endet ein EPLAN-Kabel in einem Gerätekasten mit eigener Klemmenleiste (z. B. -U1-X1), kann der Kabelplan als Ziel „-U1“ zeigen statt „-U1-X1“. Das ist eine Einstellung und kein Fehler. Im QET-Kabelplan-Generator wird man das Ziel des Kabels deshalb wahlweise auf das übergeordnete Kennzeichen kürzen.

## 14. Kabel-BMK-Format und Nummerierungsreihenfolge

- Formate (Registerkarte Format der Kabelnummerierung):
  - **Aus Projektstruktur** (Betriebsmittelstruktur für Kabel; nur dieses Format hat die Registerkarte Reihenfolge).
  - **Nach Quelle** (z. B. `-W=A1+O1-X1`), **Nach Ziel** (`-W=A1+O1-M1`), **Nach Quelle und Ziel** (`-W=A1+O1-X1/=A1+O1-M1`), **Nach Ziel und Quelle**.
  - Das Trennzeichen zwischen Quelle und Ziel ist frei, bei DIN Standard „/“.
  - Kennzeichen, die schon in der Struktur des Kabels stehen, werden im Zähler unterdrückt.
  - Bei SPS-Anschlüssen kommt die Steckerbezeichnung des SPS-Anschlusses mit ins BMK, bei Bus-Anschlüssen zusätzlich der Bus-Schnittstellenname.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_r_einstkabelnumformat.htm
- **Klammern**: Quelle/Ziel stehen in runden oder eckigen Klammern, z. B. `-W(-X1/-M1).1` (Ziel M1, Unterkabel 1) vs. `-W(-X1/-M1.1)` (Ziel M1.1). Die Klammern lassen sich über „Klammern im BMK ausblenden“ abschalten. Dann ist `-W-X1/-M1.1.1` mehrdeutig. Bei der BMK-Zerlegung gilt ein geklammerter Teil als **ein** Bestandteil und je nach Stellung als Vorziffer, Zähler oder Unterzähler (z. B. `-W(=A+O-X1).1` → Zähler; `=A+O-(=A-X1/=B-X2)W1` → Vorziffer). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_r_einstkabelnumformat.htm
- **Unterkabel erzeugen** (Standard bei Formaten mit Quelle/Ziel): Mehrere Kabel zwischen denselben Zielen unterscheiden sich durch Unterkabel-BMK (`.1`, `.2`). Ohne diese Option zählt der normale Zähler weiter (nur bei „Aus Projektstruktur“). **Anschlussbezeichnung anhängen** (bei Quelle/Ziel): hängt die grafisch erste Anschlussbezeichnung mit einem Trennzeichen an (Standard „,“; „?“, „:“ und „.“ sind verboten). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_r_einstkabelnumformat.htm
- Reihenfolge (nur bei „Aus Projektstruktur“):
  - **Manuelle Kabel zuerst nummerieren** (in grafischer Reihenfolge; Vorrang vor „Kabel an Steckern zuerst“).
  - **Klemmen-/steckerorientiert nummerieren**: Kabel an Klemmen oder Steckern zuerst, in der Reihenfolge der Leisten, maßgeblich ist das Quell-BMK. Ohne die Option gilt die grafische Reihenfolge nach Normblatt und Lage der Kabel-Hauptfunktion.
  - **Kabel an gleichen Zielen aufeinander folgend** (Beispiel: W1, W3 → X7 und W2, W4 → X8 ergibt W1, W3→W2, W2→W3, W4).
  - **Kabel an Steckern zuerst**: Nein / Alle / Nur die automatisch erzeugten, mit festgelegter Gesamtreihenfolge zusammen mit „manuell zuerst“.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_r_einstkabelnumreihenfolge.htm
- **Ergebnisvorschau** der Nummerierung: Tabelle alt/neu für Linien, Abschirmungen und Definitionspunkte. „Ursprüngliches BMK verwenden“ behält die alten BMK. Was vorher gleich hieß, heißt danach wieder gleich, damit bleiben die Zuordnungen erhalten. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_ergebniskabelnummerierung.htm
- Der Kabel-Navigator kennt zwei Nummerierungen: „Kabel-BMK nummerieren“ (kabelspezifische Formate wie oben) und „BMK nummerieren“ (allgemeine Formatelemente für alle Betriebsmittel). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabelnavigator.htm

**Bedeutung für QET:** Kabel-BMK wie `-W(-X1/-M1).1` oder `-W-X1/-M1` sind aus Quelle und Ziel **berechnete** Namen, kein freier Text. Beim Nachbau trotzdem wörtlich übernehmen, auch mit Klammern. Für Sprung- und Verknüpfungsskripte gilt dabei: Der geklammerte Teil ist ein Bestandteil des BMK und darf nicht an „-“ oder „/“ zerlegt werden. Wer das Kennzeichen am „-“ trennt (z. B. zur Strukturerkennung), muss Klammerinhalte vorher ausnehmen. Unterkabel `.1`/`.2` sind zwei verschiedene Kabel mit gleicher Strecke, also in QET zwei `Cable`-Objekte.

## 15. Automatisch erzeugte Kabel: Ablauf, Kennzeichnung, Löschen

- Der Automatiklauf aktualisiert zuerst die Verbindungen. Dann folgen drei Schritte: Kabel erzeugen → Kabelnummerierung → automatische Kabelauswahl (Artikel). Nummerierung und Auswahl lassen sich auch einzeln aufrufen. Automatisch erzeugte Kabellinien liegen auf einer **eigenen Ebene**. Linien und Definitionspunkte tragen die Eigenschaft **„Automatisch erzeugt“**, daher lässt sich der Lauf beliebig wiederholen. Am Definitionspunkt kann man das Kabel-BMK als **„Manuell gesetzt“** kennzeichnen und bei der Nummerierung entscheiden, ob auch solche BMK neu vergeben werden. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_automatikkabel.htm
- Bedingung für ein automatisches Kabel: „Verbindung: Zugehörigkeit“ = Kabel (an der Verbindung oder am Definitionspunkt), **oder** der **grafisch erste** Anschluss der Verbindung (nach Normblattausrichtung) ist ein Kabelanschluss. Ob der zweite Anschluss einer ist, spielt keine Rolle. Ist der grafisch erste Anschluss kein Kabelanschluss, entsteht kein Kabel. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_automatikkabel.htm
- Dialog „Kabel automatisch erzeugen“: drei Schemata (Kabelerzeugung, Kabelnummerierung, Kabelauswahl), dazu Startwert, Schrittweite und „Vorhandene Kabel-BMK beibehalten“ (nur „?“-Kennbuchstaben). Die Voreinstellung der drei Schemata steht in den Projekteinstellungen „Kabel (automatisch)“. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabelerzeugen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenkabelautomatisch.htm
- **Automatisch erzeugte Kabel löschen** (Markierung oder ganzes Projekt): entfernt alle als automatisch gekennzeichneten Linien und Definitionspunkte. An manuell gezeichneten Linien und Punkten werden nur die automatisch vergebenen BMK gelöscht, die Objekte bleiben. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_automatischekabelloeschen.htm

**Bedeutung für QET:** In EPLAN-PDFs ist ein automatisch erzeugtes Kabel oft nur eine kurze Linie an der grafisch ersten Verbindung, an den übrigen Adern stehen nur Definitionspunkte (siehe Abschnitt 8). Im Nachbau trotzdem alle Adern dem Kabel zuordnen, auch die ohne eigene Kabellinie im Bild.

## 16. Automatische Kabelauswahl: Schema und Regeln

- Das Schema enthält eine **Kabel-Vorauswahl** (Liste von Kabelartikeln). Nur aus dieser Liste wird gewählt. Je Zeile: Hersteller (leer = erster gefundener), PE-Adern (0/1/2), Verbindungsanzahl (mehrere durch Kommata), **Spannung in Volt** (leer = unbegrenzt). Gewählt wird der **kleinste passende Spannungswert**. Beispiel der Hilfe: Vorauswahl 300/500/1000 V; Kabel mit 300 V → 300-V-Typ, Kabel mit 400 V → 500-V-Typ. Ist am Kabel schon ein Typ eingetragen, der nicht in der Liste steht, wählt EPLAN ein Kabel dieses Typs mit passender Aderzahl. Ein fehlender Querschnitt kommt spannungsabhängig aus den Vorgaben. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenautomatischekabelauswahl.htm
- **Reserve**: Ab „Mindestzahl für Reserve“ Adern fügt EPLAN die eingetragene Zahl Reserveadern hinzu (z. B. für Werksnormen). **„Vorgegebene Verbindungsanzahl anpassen“**: tatsächliche Adern + Reserve, mit dem Eintrag verglichen, der größere Wert gilt, dann das passende oder nächstgrößere Kabel. PE/PEN zählen mit, Abschirmungen nicht. **„Alle Werte an die Schaltzeichen übertragen“**: Ist die Option an, füllt EPLAN auch leere Felder. Ist sie aus, überschreibt es nur schon gefüllte. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenautomatischekabelauswahl.htm
- Dialog „Automatische Kabelauswahl“ (Kabel-Navigator, Mehrfachauswahl): Schema wählen. „Nur automatisch erzeugte oder benannte Kabel“ schließt manuelle Kabel aus. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_automatischekabelauswahl.htm

**Bedeutung für QET:** Reserveadern im EPLAN-Kabelplan können aus dieser Regel stammen (z. B. 7 statt 5 Adern). Sie gehören als freie Adern ins QET-Kabel (`core_count` größer als die Zahl belegter Adern), nicht als Fehler.

## 17. Kabel-Navigator

- Baum: je Kabel-BMK die Kabeldefinitionslinien, Abschirmungen und Kabelverbindungen. Liste: Name, Funktionsdefinition, Quell- und Ziel-BMK sowie alle Verbindungs- und Kabeleigenschaften (Verbindungsanzahl, Querschnitt …). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabelnavigator.htm
- Kontextmenü mit Wirkung:
  - **Verlegen (Topologie)** sowie Verlegeverbindungen ein-/ausblenden (siehe verdrahtung-fertigung.md).
  - **Kabelverbindungen sammeln**: weitere Verbindungen zwischen denselben Quellen/Zielen, die noch keinem Kabel gehören.
  - **Kabelverbindungen zuordnen**: sortiert die Aderreihenfolge neu nach den Quelle/Ziel-Regeln bzw. der Klemmenreihenfolge, mit „Vorhandene Eigenschaften beibehalten“ oder „Alle neu zuordnen“.
  - **Gehe zu (Grafik)** springt zum ersten platzierten Objekt mit Bezug auf die markierte Ader, Definition oder Abschirmung.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabelnavigator.htm

## 18. Kabellängen summieren (Dialog) und Kabeldarstellungsplan erzeugen

- **Vollständiger Kabeltyp** = gleicher Typ (z. B. NYM) + gleiche Verbindungsanzahl + gleicher Querschnitt/Durchmesser + gleiche Einheit. Je vollständigem Kabeltyp gibt es eine Zeile: berechnete Gesamtlänge, längste Einzellänge, Artikelnummer und -variante des längsten Einzelkabels (über das Kontextmenü änderbar, auch auf einen kürzeren Artikel), Artikellänge, **erforderliche Stückzahl = Gesamtlänge / Artikellänge** (manuell änderbar). Löschen einer Zeile löscht den Kabelprojektartikel. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabelmengenberechnung.htm
- **Kabeldarstellungsplan** (*.f08) wird mit einem **Folgeformular** Artikelsummenstückliste (*.f02) erzeugt:
  - Auf dem f08-Formular wird ein **Folgeformulareinfügepunkt** (Auswertungstyp Artikelsummenstückliste) gesetzt. „Neue Seite“ entscheidet, ob die Stückliste auf eigener Seite oder auf derselben Seite steht.
  - In den Auswertungseinstellungen „Ausgabe in Seiten“ wird das Folgeformular eingetragen.
  - Jedem Kabel wird die Eigenschaft **Kabeldarstellungsplanformular** gegeben. Am bequemsten geht das im Kabel-Navigator mit dem Filter „Hauptfunktion“ und der Spalte „Kabel / Paket: Typ“, alle Kabel eines Typs zugleich.
  - Je Formular entsteht ein Auswertungsblock (Kabeldarstellungsplan + Stückliste), sortiert wird nur innerhalb des Blocks.

  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabeldarstellungsplan.htm

**Bedeutung für QET:** Ein Kabeldarstellungsplan im EPLAN-PDF fasst alle Kabel **eines Typs** auf einem Block zusammen, mit Stückliste darunter oder danach. Im Nachbau als eigenes Auswertungsblatt je Kabeltyp anlegen, mit Sprungzielen zu den einzelnen Kabeln. Die Längensumme aus „Kabellängen summieren“ steht nur in der Stückliste.

## 19. Darstellungseinstellungen für Kabel und Pakete

- **Darstellung der Autoconnect-Linie**: kabelorientiert (Linien nach Kabel eingefärbt) oder potenzialorientiert. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_r_allgemein.htm
- Getrennt für allpolig und einpolig lässt sich festlegen, welches Symbol an den Definitionspunkten von Kabeldefinitionslinien (auch bei automatisch erzeugten Kabeln) und von Abschirmungen steht. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_r_allpoligeinpolig.htm
- **Pakete**: Die Definitionspunkte einer Paketdefinition haben standardmäßig ein Symbol **ohne Grafik** und sind nur über die Einfügepunkte sichtbar. Ein anderes Symbol ist in „Einstellungen: Paket“ wählbar. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenpaket.htm

**Bedeutung für QET:** Farbige Linien im EPLAN-PDF können Kabel- **oder** Potenzialfarben sein, je nach dieser Einstellung. Vor einer Übernahme von Farben als Leiterfarbe prüfen, welche Lesart das Original nutzt. Die Schrägstriche bzw. Symbole an den Adern sind Definitionspunkt-Symbole. Ihre Form hängt an der Projekteinstellung und trägt keine eigene Bedeutung.

## 20. Quellen

- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_arbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_grafischedarstellung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_verhaltenabschirmungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_kabelbezeichnungformat.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_quellezielermittlung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_kabeldarstellungsplan.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelzeichnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_aderndefinieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_adernzuordnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelauflegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelauswahlautomatisch.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_abbuchen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelmengenberechnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabelauswertung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenkabelerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_kabeldaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_verbindungenklemmen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_besonderheitenkabel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_prinzipverbindungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_vdp.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_d_einstellungenaderfarben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/stripmanagementgui_d_klemmenleistebearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/terminalplugnumberinggui_d_klemmennummerieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_funktionsbezogen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_h_funkbezogauswerterzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2023/Content/htm/news_p_kabel_kabelbearbeiten_355040.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/cablegui_d_kabelbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/cablegui_d_kabelnummerierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/cablegui_k_start.htm
- https://rittal.com/imf/none/24_4357/Presseinfo%20Eplan%20Plattform%202024 (Pressemitteilung, keine Hilfe)
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenkabelmanuell.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenkabelautomatisch.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenkabelnummerierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_r_einstkabelnumformat.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_r_einstkabelnumreihenfolge.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_ergebniskabelnummerierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabelnummerierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_k_automatikkabel.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabelerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_automatischekabelloeschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenautomatischekabelauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_automatischekabelauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabelnavigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_kabelmengenberechnung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_h_kabeldarstellungsplan.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_r_allgemein.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_r_allpoligeinpolig.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cablegui_d_einstellungenpaket.htm
- Fork-Quellcode (nur gelesen): `C:\Users\test\Documents\Code Projekte\qelectrotech-eplan\sources\cable\cable.h`, `sources\cable\cable.cpp`, `sources\conductorproperties.h`, `sources\TerminalStrip\realterminal.h`, `sources\cablelist\`
