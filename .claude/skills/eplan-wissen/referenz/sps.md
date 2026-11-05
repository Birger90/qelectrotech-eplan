# EPLAN-Wissen: SPS (Kästen, Anschlüsse, Kanäle, Adressen, Datenaustausch)

Zusammenfassung der offiziellen EPLAN-Hilfe (Plattform 2.9, ergänzt um 2024–2027, wo erreichbar) in eigenen
Worten. Zweck: EPLAN-SPS-Blätter in QElectroTech (Birgers Fork) inhaltlich richtig nachbauen, also
Übersicht ↔ Kanal-Querverweise, Kanalmakros und SPS-Daten (Adresse, symbolische Adresse, Funktionstext)
so abbilden, wie EPLAN sie erzeugt. Jede Aussage hat ihre Quelle. „Vermutung“ kennzeichnet eigene Schlüsse.

Hausgrundsatz (gilt überall): **Ein SPS-Kanal ist das Kanalmakro des Moduls, nie ein eigenes Symbol je Adresse.**
Herstellermakros (EDZ/Data Portal) gehen vor generischen QET-Symbolen.

## Inhalt
1. Aufbau: Konfiguration, Station, Kopfstation, Baugruppenträger, Karte
2. SPS-Kasten (Karte) und Hauptfunktion
3. SPS-Anschlüsse: Arten und Eigenschaften
4. SPS-Kanäle und Kanalzugehörigkeit
5. Übersichtsdarstellung, einpolig, allpolig – Querverweise und Abgleich
6. Kanalorientiertes Arbeiten und Makros für SPS-Schaltpläne
7. SPS-Navigator
8. Adressierung, Adressformate, Umadressierung, Untergeräte
9. Symbolische Adressen
10. Kartenversorgung und Versorgungsanschlüsse
11. Bussysteme und Busanschlüsse
12. Datenaustausch: Zuordnungslisten, Konfigurationsdateien, AutomationML, TIA
13. Auswertungen: SPS-Diagramm, Karten- und Adressübersicht
14. Neuerungen in neueren Versionen
15. SPS-Navigator und Dialog „Adressen / Zuordnungslisten“
16. Einstellungen: SPS und automatische Adressierung
17. Neu adressieren, Ergebnisvorschau, Adressraumbelegung
18. Abbuchen: Adressen und Anschlüsse
19. SPS-Schaltplanerzeugung (Dialog und Ablauf)
20. Bus-Anschlüsse allpolig, SPS-Daten exportieren/importieren, AutomationML
21. Schema „SPS-spezifische Einstellungen“, Zuordnungslisten importieren/exportieren
22. Nummerierung mit SPS-Daten
23. Sicherheitskennwerte importieren (VDMA 66413)
24. Ergänzung: Neue CPU, Konfigurationsprojekt beim AML-Import, AML-Exportoptionen
25. Quellen

---

## 1. Aufbau: Konfiguration, Station, Kopfstation, Baugruppenträger, Karte

- Ein Projekt kann mehrere SPS und Bussysteme enthalten; Adressen werden einer CPU zugeordnet, damit mehrere
  SPS unterscheidbar bleiben. SPS-Kästen, Busanschlüsse und E/A-Anschlüsse mit demselben Konfigurationsprojekt
  bilden eine SPS-Konfiguration; E/A-Anschlüsse hängen über ihren SPS-Kasten daran.
- Eine Station besteht aus Baugruppenträgern mit SPS-Geräten und mindestens einer CPU (für den Datenaustausch
  mindestens eine CPU auf dem Hauptträger). Vollständiger CPU-Name: `Konfigurationsprojekt.Stationsname.CPU-Kennung`.
- Baugruppenträger werden selbst als SPS-Kasten projektiert; die Geräte darauf gelten als über einen Rückwandbus
  verbunden, der nicht gezeichnet wird.
- **Kopfstation** (Feldbusstation) = erste Karte einer Station, trägt die Busanschlüsse und ist zugleich
  Baugruppenträger; weitere Karten stehen neben, auf oder in ihr.
- Alles mit E/A-Anschlüssen oder SPS-Strukturdaten (CPU, Netzteil, E/A-Karte, Kommunikationsbaugruppe, Switch)
  wird immer als SPS-Kasten projektiert. Busfähige Geräte ohne E/A (Umrichter, Ventilinsel, Motor) bekommen nur
  Busanschlüsse unter gemeinsamem BMK oder als Gerätekasten.
- Antriebe: Funktionen lassen sich über „Antrieb [1–10]“ (ID 20576) bis zu zehn Antrieben zuordnen;
  „Antrieb: Gerätetyp“ (ID 20577, am Artikel ID 40349) wird bei der Artikelauswahl übernommen.

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_prinzip.htm

**Bedeutung für QET:** Station/Träger/Steckplatz gibt es in QET nicht als Daten. Für die Nachbildung reicht es,
die Reihenfolge der Module (Kopfstation zuerst, dann Steckplätze) in der Stationsübersicht und den
Modul-BMK (-A…) korrekt zu übernehmen; Strukturdaten höchstens als Elementinformation (Vermutung: freies Feld).

## 2. SPS-Kasten (Karte) und Hauptfunktion

- Ein SPS-Kasten stellt ein SPS-Gerät dar; grafisch wie ein Gerätekasten, funktional aber anders.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_sps_kaesten.htm
- Eine Karte kann auf mehrere Kästen verteilt sein (alle mit gleichem BMK, auch über mehrere Seiten). Nur der
  als **Hauptfunktion** markierte Kasten zählt für Auswertungen; Daten an den anderen Kästen werden ignoriert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_prinzip.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_spsquerverweiseerzeugen.htm
- Registerkarte SPS-Kasten: sichtbares/vollständiges BMK, technische Kenngrößen, Funktionstext (manuell,
  automatisch = sonst Pfad-Funktionstext, gemeinsam = sonst der Hauptfunktion), Graviertext, Montageort,
  Hauptfunktion, Objektbeschreibung, CPU-Name, **Startadresse** (Eingänge) und **Startadresse 2** (Ausgänge),
  Datenlänge Ein-/Ausgänge in Bit, Adressbereich (STEP 7 Classic).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_spskarte.htm
- Einfügen: Rechteck über „SPS-Kasten“ zeichnen oder als Polylinie (Symbol PLCC2); Anschlüsse innerhalb des
  Kastens übernehmen dessen BMK, einige Kasteneigenschaften sind am Anschluss sichtbar, aber nicht änderbar.
  Steht der Artikel fest, über „Gerät einfügen“ (Taste N blättert durch die Funktionen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_spszeichnen.htm
- Beim Platzieren aus dem Navigator: Darstellungsart wählen (Allpolig, Einpolig, Übersicht); ist ein
  Artikelmakro hinterlegt, ist „Artikelmakro“ vorbelegt; Tab/Umschalt+Tab wechselt Makrovarianten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/plcgui_h_spsanlegen.htm

**Bedeutung für QET:** Die Übersicht (ein Übersichtsmakro je Modul) ist der Master, die Kanalkästen auf den
Funktionsblättern sind die Slaves; alle tragen dasselbe Modul-BMK. Ein verteiltes Modul darf in QET nur einmal
als „Hauptfunktion“ (Master mit Artikeldaten) vorkommen, sonst zählt die Stückliste doppelt.

## 3. SPS-Anschlüsse: Arten und Eigenschaften

Arten (Funktionsdefinitionen):
- **E/A-Anschluss** (DE, DA, AE, AA): gehört zur Karte bzw. zu einem Kanal; hat immer eine Anschlussbezeichnung,
  oft eine Anschlussbeschreibung.
- **Multifunktion** („SPS-Anschluss, Multifunktion“): Ein- oder Ausgang je nach Signalart (Standard Digitaleingang);
  andere Signalarten verhalten sich wie Versorgung.
- **Anschlussversorgung / Kartenversorgung**: ohne Ein-/Ausgang (z. B. 24 V, M).
- **Netzwerk-/Bus-Kabelanschluss**: siehe Abschnitt 11.
- „SPS-Anschluss, intern, allgemein“ zeigt freie Adressen ohne Kanal (z. B. LED-Zustände).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsanschluesse.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_symbolischeadressen.htm

Identifikation und Eindeutigkeit:
- Ein E/A-Anschluss ist eindeutig über **BMK des Kastens + Steckerbezeichnung + Anschlussbezeichnung**.
  Anschlussbezeichnung meist einmal je Karte (mehrfach nur mit unterschiedlichem Stecker), Anschlussbeschreibung
  einmal je Kanal; Kartenversorgungen dürfen gleiche Beschreibung tragen.
- Die **Adresse ist nicht identifizierend**; sie muss beim Zeichnen nicht vergeben sein und kann später
  automatisch vergeben oder aus einer Zuordnungsliste kommen. Innerhalb einer CPU muss sie eindeutig sein.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsanschluesse.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_spsanschluss.htm

Eigenschaften (Registerkarte SPS-Anschluss):
- Anschlussbezeichnung, Anschlussbeschreibung, **Steckerbezeichnung** (z. B. -X1; in Auswertungen als -A1-X1:1),
  **Funktionstext** (manuell oder aus dem Pfad; bei Kanalsymbolen zählt der Einfügepunkt des Symbols, nicht der
  Anschluss), **Adresse**, **Symbolische Adresse**, **Kanalbezeichnung** (manuell nur nötig, wenn keine grafische
  Zuordnung geht, typisch bei Versorgung), **Datentyp** (BYTE, WORD …, abhängig vom SPS-Typ, wirkt auf die
  automatische Adressierung), **Signalart** (auch für Querverweise), **Signalbereich** (Freitext wie 0…10 V,
  4…20 mA, aus Funktionsschablonen übernehmbar, identifizierend bei der Geräteauswahl).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsanschluesse.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_spsanschluss.htm

BMK-Übernahme: Anschlüsse ohne Kasten übernehmen das BMK per Suchrichtung; auf Übersichtsseiten wird
standardmäßig nach oben gesucht (am Anschluss auf links umstellbar). SPS-Anschlüsse dürfen ohne Kasten stehen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_arbeitsweise.htm

**Bedeutung für QET:**
- Verknüpfungsschlüssel Übersicht ↔ Kanal = Modul-BMK + Stecker + Anschlussbezeichnung (z. B. „-A9“, „X1“, „3“),
  **nicht** die Adresse. Die Adresse (E50.3) ist nur ein angezeigter Wert. Ein Abgleich über die Adresse ist in
  EPLAN lediglich eine Zusatzprüfung (siehe Abschnitt 5).
- Am Kanalelement gehören Adresse, symbolische Adresse und Funktionstext in die Kanalfelder des Makros
  (EplanElementBuilder macht jedes leere EPLAN-Feld zu einem eigenen Benutzertext) – nicht als freie Texte daneben.
- Funktionstext in EPLAN oft automatisch aus dem Pfad: im PDF steht er dann über dem Strompfad, nicht am Kanal.
  Beim Nachzeichnen trotzdem in das Funktionstextfeld des Kanals schreiben, sonst fehlt er in der Übersicht.

## 4. SPS-Kanäle und Kanalzugehörigkeit

- Ein Kanal ist eine Ein- oder Ausgangsstelle einer Karte (oft 8/16/32 je Karte, je eine Adresse) und kann aus
  mehreren Anschlüssen bestehen: Signal plus Versorgung (Beispiel 3-Leiter-Näherungsschalter: zwei Versorgungs-,
  ein Signalanschluss).
- Kanaldaten (symbolische Adresse, Funktionstext) liegen an dem Anschluss mit Funktionsdefinition DE, AE, DA
  oder AA. Zusammengehörende Anschlüsse bekommen dieselbe Kanalbezeichnung.
- Mehrere E/A-Anschlüsse in einem Kanal: nur einer darf aktiv sein, die übrigen tragen „Deaktivierter E/A-Anschluss“
  und gelten dann als Versorgung. Nutzen: ein Makro/Artikel für mehrere Beschaltungen (z. B. Analogkanal mit
  Strom- und Spannungseingang). Bei F-SPS teilen sich mehrere Kanäle eine symbolische Adresse; nur einer bleibt aktiv.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_kanaele.htm

- Grafische Kanalzugehörigkeit von Versorgungsanschlüssen: Suche vom Versorgungsanschluss zuerst nach oben,
  dann gegen den Uhrzeiger in wachsendem Abstand („Uhr-Suche“); der erste gefundene E/A-Anschluss bestimmt den
  Kanal, weitere Versorgungsanschlüsse auf dem Weg gehören mit dazu. Auf SPS-Übersichtsseiten beginnt die Suche
  nach unten. Kein Fund ist kein Fehler.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_kanalzugehoerigkeit.htm

**Bedeutung für QET:** Genau deshalb ist das Kanalmakro die Einheit: DI + VS (+ M) eines Kanals bilden einen
Kanal mit einer Kanalbezeichnung. Ein ET-200SP-Kanalmakro `…_DI-VS_B1` ist ein Kanal, nicht zwei Symbole. Ein
Analogkanal mit Strom/Spannung wird mit dem Herstellermakro gezeichnet, der unbenutzte E/A-Anschluss bleibt
sichtbar, aber ohne Adresse (Vermutung: in QET als Anschluss ohne Kanaltext).

## 5. Übersichtsdarstellung, einpolig, allpolig – Querverweise und Abgleich

- SPS-Daten sind an jeder Stelle bearbeitbar: Schaltplan, Übersichtsseite, Navigator. **Hardwaredaten wie die
  Anschlussbezeichnung, im Schaltplan geändert, werden auf der Übersicht nicht automatisch nachgezogen.**
- Kartenübersichten liegen auf Seiten vom Typ „Übersicht“ und zeigen belegte und freie Ein-/Ausgänge sowie die
  Schaltplanseiten der Funktionen. Sie können vor dem Schaltplan entstehen; die Anschlüsse werden dann im
  Schaltplan „abgebucht“.
- Abgleich zwischen Darstellungen: Funktionstext geht von der Übersicht in den Schaltplan, wenn dort keiner steht;
  umgekehrt überschreibt der Schaltplantext den der Übersicht. Symbolische Adresse: Schaltplan → Übersicht nur,
  wenn dort leer; Übersicht → Schaltplan immer.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_arbeitsweise.htm

- **Querverweis Übersicht ↔ Schaltplan** entsteht, wenn übereinstimmen: BMK der Karte, Funktionsdefinition,
  Steckerbezeichnung (bei Bus zusätzlich Bus-Schnittstellenname) und Anschlussbezeichnung. Die SPS-Adresse spielt
  für Querverweise keine Rolle. Voraussetzung: in den Projekteinstellungen „Übersichtsquerverweise anzeigen“ aktiv.
  Ablauf in der Hilfe: Kasten mit BMK allpolig, Kasten mit gleichem BMK in der Übersicht, Anschluss mit gleicher
  Anschlussbezeichnung in beiden → Verweis auf beiden Seiten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_spsquerverweiseerzeugen.htm
- Abweichende Angabe auf der Arbeitsweise-Seite: Stimmen die Merkmale überein, wird zusätzlich über die
  SPS-Adresse querverwiesen. Die beiden Hilfeseiten widersprechen sich hier; sicher ist nur, dass die Adresse
  allein keinen Querverweis erzeugt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_arbeitsweise.htm
- Die Signalart beeinflusst die Querverweisdarstellung auf der Übersicht; über eine Einstellung bekommen auch
  Versorgungsanschlüsse Querverweise.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsanschluesse.htm
- Netzwerkstrukturen werden nur aus Busanschlüssen ermittelt, die einpolig (bei AutomationML auch allpolig)
  dargestellt sind; die Übersichtsdarstellung dient nur der Anzeige. Empfehlung: Busdaten an die einpolige
  Darstellung schreiben.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_prinzip.htm
- Eine Funktion kann mehrere Darstellungsarten haben, jede aber nur einmal (allpolig, einpolig, Übersicht,
  Paarquerverweis, grafisch …).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_darstellungsarten.htm

**Bedeutung für QET:**
- Im EPLAN-PDF verweist jeder Kanal der Übersicht auf die Seite/Spalte seines Kanalmakros und umgekehrt
  (Dreifachsprung siehe Notiz EPLAN-PDF-Sprunglogik). In QET: Übersichtsmakro = Master, Kanal = Slave, Link über
  Modul-BMK + Anschlussbezeichnung. `verknuepfen.py` muss diesen Schlüssel nutzen; die Adresse nur zur Kontrolle.
- Freie Kanäle der Übersicht haben keinen Querverweis – das ist korrekt, kein Fehler.
- Stimmen Übersicht und Funktionsblatt in einem Text nicht überein, ist im Zweifel der Schaltplan maßgeblich
  (Funktionstext), bei der symbolischen Adresse die Übersicht (Abgleichrichtung oben) – Vermutung für den Fall,
  dass das PDF widersprüchlich ist.

## 6. Kanalorientiertes Arbeiten und Makros für SPS-Schaltpläne

- **Geräteorientiert:** Karte über „Neues Gerät“ anlegen. Ohne Artikelmakro entsteht ein SPS-Kasten mit
  Funktionsschablonen; mit Artikelmakros überlagern allpolige Makros die Schablonen. Bei mehreren Makrovarianten
  wird eine nach der anderen verwendet, bis keine passenden Anschlüsse übrig sind; **pro Kanal wird das Makro
  platziert, meist das am Artikel hinterlegte.** Reste erscheinen als nicht platzierte Funktionen.
- **Funktionsorientiert:** erst allpolige Makros/Seitenmakros (möglichst nur SPS-Anschlüsse, kein Kasten mit
  Artikel) als Ziele platzieren, BMK mit „?“ markieren, dann im Navigator je Kanal „Zuweisen“ oder
  „SPS-Anschlüsse blockweise abbuchen“.
- Im Navigator sind Kanäle mit Icons für „sicherheitsrelevant“ und „nicht platzierte Anschlüsse“ gekennzeichnet.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_kanalorientiertarbeitsweise.htm

SPS-Schaltplanerzeugung (Makrozuordnung):
- Makrodatei wird bei SPS-Kästen vom **Hauptartikel** übernommen (= erster Artikel, der kein Zubehör ist) und muss
  alle benötigten Darstellungsarten enthalten.
- **Kartenmakro:** allpolige Variante = Kasten + Kanalanschlüsse ohne Beschaltung; einpolige = Kasten + Signal-
  anschlüsse. Die ersten Varianten müssen Fenstermakros sein. Makros sollen ganze Seiten füllen, **typisch 8 Kanäle
  je Seite**; das erste Fenstermakro bestimmt die Seitendaten.
- **Beschaltungsmakros** (Sensor, Aktor an den E/A) sind eigene Dateien je Anschlusszeile, allpolig und einpolig je
  in Variante A; Zuordnung über Anschluss- und Steckerbezeichnung, Anfasspunkt auf dem nächsten Rasterpunkt in
  Anschlussrichtung.
- **E/A-Übersicht:** Übersichtsmakro aus **Variante A** der Makrodatei, genau eines je Karte.
- **Baugruppenträger-Übersicht:** Makro aus **Variante E**, Karten von links nach rechts in Navigatorreihenfolge;
  Umbruch über Zusatzmakro (E Abbruch, F Weiterführung).
- Platzhaltervariablen der Makros erscheinen als Spalten; Werte über Wertesätze oder manuell.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_makrosschaltplanerzeugen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsschaltplanerzeugen.htm

Abbuchen beim Platzieren:
- SPS-Anschlüsse passen, wenn Funktionskategorie und Anschlussbezeichnung übereinstimmen; beginnt die Bezeichnung
  mit „?“, muss zusätzlich der Kanal passen; eine „?“-Bezeichnung überschreibt nie eine echte.
- SPS-Kästen passen über die Funktionsdefinition. BMK mit „?“ (z. B. -?K1) werden neu nummeriert.
- Variablen geben SPS-Funktionen keine Werte; diese kommen erst beim Abbuchen.
- Kasten im Makro als Nebenfunktion → zusätzliche Funktion mit Daten der Hauptfunktion; als Hauptfunktion → wird
  Hauptfunktion, oder Nebenfunktion, falls schon ein platzierter Hauptkasten existiert.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_abbuchverhalten.htm

**Bedeutung für QET:**
- Das ist die offizielle Begründung für den Hausgrundsatz: EPLAN setzt **je Kanal das Kanalmakro des Artikels**.
  Im Nachbau also je Kanal das importierte Herstellermakro (z. B. `…_DI_B1`, `…_DI-VS_B1`, `…_DI-AUX_C1`), nie je
  Adresse ein selbstgebautes Symbol.
- Makrodatei des Moduls enthält mehrere Varianten: Variante A = Übersicht (in Übersicht-Darstellung), weitere
  Varianten = Kanalvarianten, E/F = Trägerübersicht. Der Fork-Leser `eplanmacroreader` nimmt derzeit nur die
  **erste** Variante mit `MacroFuncType="1"` (allpolig) – Kanalvarianten B, C … und die Übersicht (Variante A in der
  Übersichtsdarstellung) gehen dabei verloren. Vermutung: Für SPS-Module alle Varianten je Darstellungsart als
  eigene Elemente importieren (Name mit Variante und Darstellung, wie `eplanpartimporter` es für DXF-Pakete tut).
- Beschaltung (Sensor, Klemme, Kabel) gehört nicht ins Kanalmakro; sie wird getrennt gezeichnet.

## 7. SPS-Navigator

- Dialog „SPS – <Projektname>“ (2026: Registerkarte Betriebsmittel > SPS > Navigator). Zeigt immer alle SPS-Daten:
  Kästen (auch leere bzw. nur mit Funktionsschablonen), Anschlüsse, Funktionsschablonen.
- Ansichten: **BMK-orientiert** (Projektstruktur), **adressenorientiert** (E/A und Versorgung unter der Adresse,
  gruppierbar je CPU), **kanalorientiert** (unter der Kanalbezeichnung; ohne Kanalbezeichnung nach BMK),
  **baugruppenträgerorientiert** (Träger → Karten nach Steckplatz, Unterplätze als „2.1“; Kopfstation hat das
  Träger-Icon, ihre Karten zuerst), **antriebsorientiert** (Konfigurationsprojekt → Antrieb → Station).
- Filter/Darstellung konfigurierbar (z. B. nur Anschlüsse mit SPS-Adresse; nur Funktionsgruppe
  „SPS-Anschluss, E / A, 1 Anschluss“).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/plcgui_k_ansichten.htm
- Im Navigator lassen sich nicht platzierte Kästen/Anschlüsse anlegen („Neu“ → Funktionsdefinitionen) und später
  platzieren; Strukturdaten (Träger, Steckplatz) am besten tabellarisch zuweisen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/plcgui_h_spsanlegen.htm

**Bedeutung für QET:** Die kanalorientierte Sicht entspricht der Soll-Liste fürs Nachzeichnen: je Modul alle
Kanäle mit Adresse, symbolischer Adresse, Funktionstext und Zielseite. Die `sps_uebersicht_*.json` sollte genau
so gegliedert sein (Modul-BMK → Kanal → Anschlüsse).

## 8. Adressierung, Adressformate, Umadressierung, Untergeräte

- Adressen manuell (Feld Adresse), **automatisch beim Einfügen** (Schema aus Projekteinstellungen bzw. der CPU;
  im Mehrbenutzerbetrieb abschalten) oder **nachträglich** für viele Anschlüsse (Projektdaten > SPS > Adressieren).
  Anschlüsse mit „Beim Adressieren nicht berücksichtigen“ (an allen Darstellungsarten setzen) behalten ihre Adresse.
- Startadresse / Startadresse 2 der Karte trennen Ein- und Ausgänge; über den Konfigurationswert nutzbar.
  Prüfläufe 004106/004107 kontrollieren Adressbereiche und Überschneidungen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_adressierung.htm

- Adressformat = Zähler-Formatelemente `[C<Zahlensystem, Anfang, Ende, Stellen, Konfigurationswert>]` mit
  Trennzeichen. Zahlensysteme d/o/h; Konfigurationswerte O (keiner), N (Station), R (Träger), M (Steckplatz),
  G (Startadresse der Karte), C (CPU-Name). Beispiel `[C<d,1,127,0,G>].[C<o,0,7,0,O>]` = Byte ab Kartenstart,
  Bit 0–7 oktal.
- Digital: Kennbuchstabe + Byte + Bit; analog: meist nur Byte, Schritt 1/2/4 je Datentyp. Mit einheitlichem
  Konfigurationswert laufen Adressen über Kartengrenzen weiter, sonst beginnt jede Karte neu.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_k_adressformate.htm

- **Untergeräte** (seit 2.9): bis zu 12 integrierte Baugruppen mit eigenen Adressbereichen unter einer
  Artikelnummer; Name + Position identifizieren, Startadressen nur im Projekt; Anschlüsse ordnen sich über
  „SPS-Untergerät: Index“ zu (in Funktionsschablonen hinterlegbar). Index in AML-Datei mit Kontext-Präfix
  D/, R/, DAP/, M/ (TechTipp 2027 zusätzlich SM/).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_untergeraete.htm ·
  https://www.eplan.help/techtipps/de-DE/SPS/TechTip-Configuring-PLC-devices-with-device-description-files.pdf

**Bedeutung für QET:** Adressen werden aus dem PDF bzw. der Übersicht übernommen, nicht neu gerechnet. Zur
Plausibilitätsprüfung: Kanal n eines DI-8-Moduls mit Startbyte 50 hat Bit n−1 (E50.0 … E50.7); ein Sprung
im PDF ist meist ein Steckplatz- oder Startadresswechsel, kein Zeichenfehler (Vermutung).

## 9. Symbolische Adressen

- Drei Eigenschaften: manuell, automatisch (= manuell, sonst ermittelt), ermittelt (aus Zielverfolgung).
- Automatische Ermittlung (in den SPS-Einstellungen einschaltbar): Zielverfolgung sucht den angeschlossenen Sensor
  (Eingang) bzw. Aktor (Ausgang) über die Signalart der Anschlusslogik; dessen BMK wird symbolische Adresse.
  Kein Fund → BMK der letzten Funktion, an der die Suche abbrach. Bei Wechslern/3-Leiter-Initiatoren steuert
  „Zielverfolgung (SPS)“ den Weiterweg.
- Formatierung: Vorzeichen ersetzbar, Kennzeichnungsblöcke (Anlage, Ort) unterdrückbar, Anschlussbezeichnung
  anhängbar. Für den Export muss die symbolische Adresse je CPU eindeutig sein.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_symbolischeadressen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_exportbuskonfig.htm

**Bedeutung für QET:** Steht im PDF als symbolische Adresse ein BMK (z. B. -B12), ist sie meist automatisch
ermittelt; sie muss zum BMK des tatsächlich angeschlossenen Sensors/Aktors passen – guter Inhaltscheck für den
Nachbau (Vermutung, wenn die Einstellung im Original unbekannt ist).

## 10. Kartenversorgung und Versorgungsanschlüsse

- Karten- und Kanalversorgung sind meist getrennte Anschlüsse; mehrere Kartenversorgungen dürfen dieselbe
  Beschreibung haben.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_prinzip.htm
- Versorgungsanschlüsse haben keinen Ein-/Ausgang; Kanalzuordnung grafisch (Abschnitt 4) oder per
  Kanalbezeichnung; Querverweise per Einstellung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsanschluesse.htm
- Anschlussversorgung, Kartenversorgung, „Allgemein“ und „2 Anschlüsse“ werden beim SPS-Datenaustausch nicht
  übertragen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_exportbuskonfig.htm

**Bedeutung für QET:** Die Kartenversorgung (L+/M der BaseUnit) ist ein eigener Kasten desselben Moduls mit
eigenem Versorgungsmakro, nicht Teil eines Kanals. Im Nachbau als eigenes Herstellermakro auf das
Versorgungsblatt, gleiches Modul-BMK, Slave des Moduls.

## 11. Bussysteme und Busanschlüsse

- Busanschlüsse = SPS-Anschlüsse der Kategorie „Netzwerk-/Bus-Kabelanschluss“, standardmäßig netzverbindend und
  signalweiterleitend. Identifikation: BMK + Bus-Schnittstellenname + Steckerbezeichnung + Anschlussbezeichnung
  (beim Import nur Schnittstellenname + Stecker; die Anschlussbezeichnung identifiziert nicht).
- Ethernet-basierte, untereinander kompatible Systeme: CC-Link IE, EtherCAT, Ethernet, EtherNet/IP, Modbus-TCP,
  Powerlink, Profinet, SERCOS III, SSCNET III, VARAN. Bussystem wird in den Funktionsschablonen festgelegt.
- Automatische Verbindung: gleiches BMK und Bussystem (Ethernet: gleicher Schnittstellenname), Daten nur am
  Haupt-Busanschluss; Switch-Anschlüsse (passive Geräte) verhalten sich wie Sammelschienen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsanschluesse.htm
- Für den Export: Busdaten (physikalisches Netz, Bussystem, ggf. logisches Netz/Master), bei Ethernet
  Bus-Schnittstellenname, Steckerbezeichnung gefüllt; alle Anschlüsse eines logischen Netzes im selben
  physikalischen Netz. Prüfläufe 004103/004104.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_exportbuskonfig.htm
- Der erste Busanschluss wird als „Bus-Schnittstelle: Haupt-Bus-Anschluss“ markiert und trägt alle Busdaten.
  Quelle: https://www.eplan.help/techtipps/de-DE/SPS/TechTip-Configuring-PLC-devices-with-device-description-files.pdf

**Bedeutung für QET:** Profinet-Ports (P1/P2 an X1) sind Anschlüsse des Moduls mit Steckerbezeichnung; ein
Buskabel verbindet Port mit Port. Im Nachbau den Port des Herstellermakros verwenden, Stecker- und Portname wie
im PDF.

## 12. Datenaustausch: Zuordnungslisten, Konfigurationsdateien, AutomationML, TIA

Zuordnungslisten:
- Enthalten SPS-Adresse, symbolische Adresse, Funktionstext; Format herstellerabhängig. Verwaltung im Dialog
  „Adressen / Zuordnungslisten“ (alle E/A aller SPS, Excel-Austausch per Kopieren). Import/Export als Siemens
  **SDF, SEQ** oder **ASCII (ASC)**; Schema je CPU. Mehrteilige und mehrsprachige Funktionstexte brauchen je Teil
  und Sprache ein Formatelement „Funktionstext“; Sprachreihenfolge bei Export und Import gleich halten.
- Abgleich: EPLAN-Liste exportieren, externe Liste importieren, dann EPLAN-Liste importieren.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_zuordnungslisten.htm

Konfigurationsdateien:
- Ausgetauscht werden Hardware (Trägeraufbau mit Artikeln), Symboltabelle und Netzwerkstruktur. EPLAN kennt
  Versorgungsanschlüsse, die die SPS-Software nicht braucht; umgekehrt fehlen EPLAN z. B. Router-Infos.
- Geräteidentifizierung über **SPS-Typbezeichnung** (oft Bestellnummer, exakte Schreibweise) **oder
  Gerätebeschreibungsdatei** (GSD/GSDML/EDS; Dateiname ohne Pfad, dazu Objektbeschreibung bzw. bei AML
  „Index in Datei“). Je Karte nur eine Identifizierung.
- Empfohlen: Trägeraufbau einpolig mit einpoligen Kästen als Hauptfunktion; E/A mit Sensoren/Aktoren allpolig,
  **bitweise mit Makros pro Kanal**; Belegung im Navigator oder auf Übersichtsseiten prüfen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_austauschbuskonfig.htm
- Export setzt u. a. voraus: CPU aktiv mit projektweit eindeutigem Namen, symbolische Adresse je CPU eindeutig,
  Datentyp gesetzt, Träger und Steckplatz gefüllt (bei Träger/Kopfstation Steckplatz leer). Vorher Prüfläufe der
  Klasse 004 „SPS / Bus“ und 007010.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_exportbuskonfig.htm
- Import: Artikel über Typbezeichnung/Gerätebeschreibung + Index/Objektbeschreibung zugeordnet (erster Treffer;
  Platzhalter * und ? erlaubt); ohne Treffer einfacher SPS-Kasten mit Meldung. Fehlende Anschlussbezeichnungen
  werden „?1 … ?n“; bei bekanntem Artikel kommen sie aus den Funktionsschablonen, **deren Reihenfolge der
  Kanalreihenfolge entsprechen muss**. Versorgungsanschlüsse bleiben unberücksichtigt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_importbuskonfig.htm

AutomationML / unterstützte Programme (TechTipp, Stand April 2026, für Version 2027):
- EPLAN stellt den Austausch auf **AutomationML „AR APC“** um. Schnittstellen auf dieser Basis u. a.: Beckhoff
  TwinCAT 3, Bosch Nexeed Automation, logi.cals logi.CAD 3, MELSOFT iQ Works, OMRON Sysmac Studio, Phoenix Contact
  PLCnext Engineer, Rockwell Studio 5000 Logix Designer, **Siemens STEP 7 TIA Portal 19/20/21**, Siemens TIA
  Selection Tool. Über das EPLAN-SPS-Standardaustauschformat V1/V2: ABB Automation Builder, B&R Automation Studio,
  Bosch Rexroth IndraWorks. Sonstige: Schneider Unity Pro, STEP 7 5.6.
  Quelle: https://www.eplan.help/techtipps/de-DE/SPS/Overview-of-supported-PLC-systems.pdf
- Aus TIA exportierte AML-Daten: integrierte Baugruppen erscheinen als eigene Module (M/…); in EPLAN als
  Untergeräte am Hauptartikel hinterlegen, sonst werden sie als eigene Geräte importiert.
  Quelle: https://www.eplan.help/techtipps/de-DE/SPS/TechTip-Configuring-PLC-devices-with-device-description-files.pdf

**Bedeutung für QET:** QET hat keinen SPS-Datenaustausch. Nutzbar ist eine Zuordnungsliste (ASC/SEQ/Excel) bzw.
eine TIA-Variablentabelle als Quelle für Adresse/symbolische Adresse/Kommentar, um die Kanalfelder zu füllen
oder den Nachbau gegenzuprüfen (Vermutung: Zuordnung über Adresse, weil Listen keine BMK tragen). Der TIA-MCP
dieser Umgebung kann Variablentabellen exportieren.

## 13. Auswertungen: SPS-Diagramm, Karten- und Adressübersicht

- **SPS-Diagramm** (Formular *.f19): funktionsbezogen, ein Diagramm je SPS-Karte mit deren SPS-Anschlüssen;
  besonders für Dateiausgabe sinnvoll. Funktionsbezogene Auswertungen gibt es je Betriebsmittel nur einmal.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_funktionsbezogen.htm
- **SPS-Kartenübersicht** (*.f20): alle Karten mit ihrer Adressbelegung. **SPS-Adressübersicht** (*.f48): alle
  SPS-Adressen, auch Zuordnung zu Planungsobjekten. Artikelstückliste (*.f01).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm
- Zu Betriebsmitteln lässt sich über den Platzhalter „Angeschlossener SPS-Anschluss“ dessen Daten ausgeben.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_arbeitsweise.htm
- Übersicht der Auswertungstypen (funktionsbezogen, Auswertungsübersichten, grafisch).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungstypen.htm

**Bedeutung für QET:** Die grafische Modulübersicht im PDF ist meist kein Formular, sondern das Übersichtsmakro
(Darstellungsart Übersicht) auf einer Seite vom Typ Übersicht; SPS-Diagramm/Kartenübersicht sind dagegen
Formular-Auswertungen (Tabellen). In QET: Übersichtsmakro als Element nachbauen, Tabellen-Auswertungen als
Nomenklatur/Tabelle oder Text (Vermutung zur Unterscheidung: Formularseiten haben Tabellenkopf und kein
Kastensymbol).

## 14. Neuerungen in neueren Versionen

- 2.9: SPS-Untergeräte (bis 12) in der Artikelverwaltung; ab 2022 Assistenz-Workflow dafür.
  Quelle: https://www.eplan.help/techtipps/de-DE/SPS/TechTip-Configuring-PLC-devices-with-device-description-files.pdf
- 2026: Navigator mit baugruppenträger- und antriebsorientierter Ansicht, Menü „Betriebsmittel > SPS > Navigator“;
  allpolige Darstellung kann bei AML AR APC die einpolige für die Netzwerkermittlung ersetzen.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/plcgui_k_ansichten.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/plcgui_h_spsanlegen.htm
- 2027 (Stand TechTipp April 2026): Austausch auf AML AR APC, TIA Portal 19–21.
  Quelle: https://www.eplan.help/techtipps/de-DE/SPS/Overview-of-supported-PLC-systems.pdf

## 15. SPS-Navigator und Dialog „Adressen / Zuordnungslisten“ (Dialogdetails)

SPS-Navigator (Projektdaten > SPS > Navigator):
- Jede Karte erscheint mit Kartendaten, Versorgungsanschlüssen, Adressen (Kanälen) und den zugehörigen
  Anschlüssen; die Listenansicht zeigt standardmäßig Funktionstext (automatisch), Adresse, symbolische Adresse.
- Kontextmenü mit fachlicher Wirkung: **Zuweisen** (Funktion auf ein Schaltzeichen ziehen; die Funktion muss
  gleich viele oder mehr Anschlüsse haben als das Symbol; blockweise per Rahmen), **Platzieren** (Umschalt =
  einzelne Funktionen, Rücktaste = Makroauswahl/Darstellungsart), **Bearbeiten** am Baugruppenträger
  (Kartenreihenfolge), **Adressieren**, **Gehe zu (querverwiesene / alle Darstellungsarten / Grafik)**.
- **Löschen** im Navigator entfernt platzierte und nicht platzierte Funktionen; dabei können Verbindungen
  entstehen oder verschwinden.
- Am Kartenartikel hängt das Artikelmakro; per Drag & Drop aus dem Baum direkt platzierbar.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_spsdaten.htm

Ergänzung aus der 2.9-Fassung der Seiten zu Ansichten und Anlegen (die 2026-Fassung ist in Abschnitt 7 zitiert):
- Adressenorientiert stehen Anschlussversorgungen nur dann unter der Adresse, wenn an ihnen dieselbe SPS-Adresse
  eingetragen ist; Gruppierung je CPU nur, wenn die CPU am SPS-Kasten steht.
- Für den klassischen SPS-Datenaustausch ist die **einpolige** Darstellung der SPS-Konfiguration Pflicht (Busdaten an
  die einpoligen Busanschlüsse); bei AML genügt alternativ die allpolige.
- Strukturdaten tabellarisch: Bauteile einpolig/Übersicht platzieren, markieren, Bearbeiten > Tabellarisch mit
  Spalten Hauptfunktion, Platzierung, Baugruppenträger, „SPS-Karte steckt auf Baugruppenträger“, „An Position /
  Steckplatz“; daraus schließt EPLAN auf den Rückwandbus. Beim Platzieren: Tab = Makrovarianten, Umschalt+Tab =
  Darstellungsarten, N = Funktionen, Leertaste = Nebenfunktionen abbrechen.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_ansichten.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_spsanlegen.htm

Baugruppenträger bearbeiten: Karten sortieren nach „An Position / Steckplatz“; wird die Reihenfolge im Dialog
geändert, bekommen die Karten eine **Sortierkennung (für Position im Baugruppenträger)**, die dann vorgeht
(nützlich bei gleichen/leeren Steckplätzen). Kontextmenü nummeriert „An Position / Steckplatz“ mit Startwert und
Schrittweite neu. Spalten: Steckplatz, BMK, SPS-Typbezeichnung, SPS-Kartenname.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_baugrtraegerbearbeiten.htm

Dialog Adressen / Zuordnungslisten (Projektdaten > SPS > Adressen / Zuordnungslisten):
- Verwaltet alle E/A aller CPUs; Spalten u. a. CPU (indirekt), SPS-Adresse, Datentyp, Symbolische Adresse
  (manuell / automatisch / ermittelt), **Symbolische Adresse: Gruppe** (frei, z. B. Eingänge, Safety; wird im
  AML-Austausch genutzt), Funktionstext / Funktionstext (automatisch), BMK (identifizierend),
  Anschlussbezeichnung mit Steckerbezeichnung, **Richtung** (Eingang/Ausgang/Undefiniert), Zum Löschen markiert,
  Vorplanung, Kanalbezeichnung.
- Neue Adressen = leere Zeilen; neue freie symbolische Adressen brauchen mindestens den Datentyp. Austausch mit
  Excel per Kopieren/Einfügen. Im Projekt verbaute Adressen lassen sich nicht löschen.
- Änderungen am Funktionstext landen im manuellen Funktionstext; stammte der Text aus dem Pfad-Funktionstext oder
  vom Sensor/Aktor, wird **dorthin zurückgeschrieben**. Geänderte symbolische Adresse → manuelle symbolische Adresse.
- „Mit Gerät erzeugen“ erzeugt aus markierten Adressen per Geräteauswahl nicht platzierte Funktionen.
- „Zeilenumbrüche im Projekt beibehalten“ stellt Zeilenumbrüche (und Übersetzungen) wieder her, wenn sich die
  Texte nur in Umbrüchen/Leerzeichen unterscheiden.
- Import/Export immer nur **je CPU** (Filter auf eine CPU setzen; „Filter für CPU erstellen“ im Kontextmenü).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_spseadaten.htm

**Bedeutung für QET:** Die Tabelle ist das Muster für eine Prüf-/Soll-Liste je CPU: Adresse, Datentyp, Richtung,
symbolische Adresse, Funktionstext, BMK + Stecker:Anschluss, Kanalbezeichnung. Ein Export aus dem QET-Nachbau in
genau diesen Spalten wäre direkt mit einer EPLAN- oder TIA-Liste vergleichbar (Vermutung, kein Fork-Feature).

## 16. Einstellungen: SPS (projektweit) und automatische Adressierung

Dialog Einstellungen: SPS (Optionen > Einstellungen > Projekte > … > Betriebsmittel > SPS):
- **Bei erfolgloser Suche der symbolischen Adresse die Adresse nehmen:** findet die Zielverfolgung keinen
  Sensor/Aktor, wird die Realadresse (z. B. E1.0) als symbolische Adresse eingetragen; sonst bleibt sie leer.
- **Sensoren und Aktoren gleich behandeln:** Zielverfolgung stoppt am ersten Sensor oder Aktor; sonst sucht ein
  Eingang nur Sensoren, ein Ausgang nur Aktoren.
- **Erweiterte Pfad-Funktionstextübernahme** (Standard aus): gibt es weder Pfad-Funktionstext noch einen
  Funktionstext (automatisch) am verbundenen Sensor/Aktor, wird in den Schaltplanpfaden weitergesucht, je nach
  Anschlussrichtung von links oder von oben.
- **Kennbuchstabe beim Import vergeben** (nur AML-Import): neue Geräte ohne BMK bekommen Kennbuchstabe aus dem
  Satz der Online-Nummerierung plus „?“ als Merker (Beispiel IEC: `-?A1`), sonst nur Zahl (`-?1`). BMK aus der
  Importdatei überschreiben immer; leeres BMK in der Datei lässt ein vorhandenes BMK stehen (Identifikation per
  AML-GUID).
- **Umbruchmakro für den Baugruppenträgeraufbau** (Variante E = Umbruch, F = Weiterführung).
- **SPS-spezifische Einstellungen:** Standardschema, gilt für jede CPU ohne eigenes Schema.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_allgemeinespseinstellungen.htm

Automatisch beim Einfügen adressieren: Schalter **„SPS-Adressen“** in den Nummerierungsformaten der
Online-Nummerierung (Registerkarte Bezeichnungen) – **nach der Installation aktiv**. Wirkt beim Einfügen, auch
über Makros, und beim Kopieren; Schrittweite und Format aus dem Schema der CPU (sonst Projektschema). Ausgeschaltet
bleibt das Adressfeld leer. Vor dem Einfügen/Kopieren das Schema prüfen, sonst falsch formatierte Vorschläge;
im Mehrbenutzerbetrieb abschalten.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_automatisch_adressieren.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_spszeichnen.htm

**Bedeutung für QET:**
- Steht im EPLAN-PDF als symbolische Adresse eine Realadresse (E1.0), war vermutlich „die Adresse nehmen“ aktiv
  und kein Sensor gefunden – nicht als Fehler „korrigieren“, sondern übernehmen.
- Kopierte Kanalmakros in EPLAN tragen sofort neue Adressen; im PDF fortlaufende Adressen bei kopierten Blättern
  sind deshalb normal. QET adressiert nicht automatisch: beim Kopieren von Kanälen Adressen immer neu setzen.

## 17. Neu adressieren, Ergebnisvorschau, Adressraumbelegung

SPS-Anschlüsse neu adressieren (Projektdaten > SPS > Adressieren, auf markierte Anschlüsse/Seiten/Projekt):
- Nur Anschlüsse **einer** CPU (bei Mischauswahl CPU-Abfrage); Schema der CPU (sonst Projektschema).
- Digital und analog getrennt ein-/ausschaltbar, je eigene Startadresse.
- **Sortierung** (immer zuerst nach Karten-BMK): nach grafischer Platzierung (nur sinnvoll, wenn alles platziert),
  nach **Kanalbezeichnung** oder nach **Anschlussbezeichnung** (Stecker zuerst: `-A1-X1:2` vor `-A1-X2:1`).
- „Anwenden auf gesamte CPU“ dehnt auf die ganze CPU aus.
- **Nur markierte Platzierungen** bekommen die neue Adresse: sind Schaltplan und Übersicht nicht beide markiert,
  weichen die Adressen danach voneinander ab.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_anschluesseneuadressieren.htm

SPS-Adressen neu vergeben (in der Zuordnungsliste, Kontextmenü Adressieren): freie und verbaute Adressen werden
neu vergeben und an die Funktionen zurückgeschrieben; Auswahl der Adressierungsart aus dem Schema
(Eingangs-/Ausgangs-Bit = digital, sonst analog). **Konfigurationswerte des Adressformats (G, M …) werden hier
nicht verwendet**, weil die Liste nur Anschluss-, keine Kastendaten kennt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_eaneuadressieren.htm

Ergebnisvorschau: alte und neue Adresse nebeneinander, neue editierbar; **Mehrfachvergabe** (Adresse anderswo im
Projekt schon benutzt) wird rot markiert; Anschlüsse mit „Beim Adressieren nicht berücksichtigen“ fehlen in der
Vorschau.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_vorschauadressierung.htm

Adressraumbelegung exportieren (CSV/TXT, sprachunabhängig, je CPU): je Byte eine Zeile
`"Bytenummer","BMK Eingangskarte","BMK Ausgangskarte"`, mehrbytige Karten in Folgezeilen wiederholt, freie Bytes
leer, Ende beim letzten belegten Byte. Nur Karten mit **Startadresse und Datenlänge** werden berücksichtigt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_exportadressraumbelegung.htm

## 18. Abbuchen: Adressen und Anschlüsse (einzeln, blockweise)

Adressen abbuchen/zurückbuchen:
- Einzeln am Anschluss (Feld Adresse, [...] öffnet „Zum Abbuchen auswählen“ mit freien und verbauten Adressen
  der Zuordnungsliste). Zurückbuchen nur, wenn die Adresse an keiner anderen Darstellung des Anschlusses steht;
  die Adresse wird freie Adresse, Funktionstext (automatisch) und symbolische Adresse (automatisch) bleiben,
  **manuelle** Texte/symbolische Adressen werden gelöscht.
- Beim gemeinsamen Zurückbuchen gelten Adressen mit/ohne Leerzeichen als gleich („E0.0“ = „E 0.0“).
- Blockweise (Projektdaten > SPS > Adressen blockweise abbuchen): linke Tabelle „An“ = markierte Anschlüsse in
  **grafischer Reihenfolge**, rechte „Von“ = gewählte Adressen (nach CPU, Anschlussbezeichnung sortiert); Pfeile
  verschieben nur rechts. An Versorgungsanschlüssen werden Adresse/symbolische Adresse/Funktionstext nur
  übertragen, wenn dort schon Werte standen. Sind Schaltplan- und Übersichtsplatzierung markiert, gilt die
  Adresse für beide; unterschiedliche Adressen der Platzierungen zeigt die Tabelle als `<<...>>`.
- Abbuchen auf Funktionsschablonen macht diese zu nicht platzierten Funktionen.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_adressenabbuchen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_adressenblockweiseabbuchen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_adressenabbuchen.htm

Anschlüsse abbuchen (nicht platzierte SPS-Funktionen auf platzierte übertragen):
- Einzeln über [...] neben „Sichtbares BMK“ (BMK-Auswahl). Blockweise über Projektdaten > SPS > Anschlüsse
  blockweise abbuchen; Status-Icon je Zeile: grün = Kategorie und Gruppe der Funktionsdefinition gleich,
  gelb = nur Kategorie gleich (möglich, evtl. falsch), rot = Kategorie verschieden (gesperrt).
- **Fallstrick:** Anschlüsse in einem SPS-Kasten bekommen beim Abbuchen das BMK der abgebuchten Anschlüsse; der
  Kasten behält seins → Zuordnung zum Kasten geht verloren. Deshalb den Kasten mit abbuchen.
- Schaltplan- und Übersichtsplatzierung gemeinsam markieren, sonst geht ihre Zusammengehörigkeit verloren. Gibt
  es einen Anschluss mehrfach im Schaltplan, aber einmal in der Übersicht, bekommt nur die **grafisch erste**
  Schaltplanplatzierung den Übersichtspartner.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_anschluesseabbuchen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_blockweiseabbuchen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_anschlblockweiseabbuchenauswahl.htm

**Bedeutung für QET:** Die Paarbildung Übersicht ↔ Schaltplan ist in EPLAN eine echte Zusammengehörigkeit
derselben Funktion (zwei Platzierungen), nicht nur Textgleichheit. Kommt ein Kanal im PDF zweimal im Schaltplan
vor, verweist die Übersicht auf die **grafisch erste** Stelle – `verknuepfen.py` sollte bei Dubletten so wählen
(Seite, dann Spalte/Position) und die Dublette melden.

## 19. SPS-Schaltplanerzeugung (Dialog und Ablauf)

- Projektdaten > SPS > Schaltplan erzeugen; das **Projekt wird beim Öffnen komplett gesperrt**. Ein Betriebsmittel
  erscheint immer vollständig (Karte + alle Anschlüsse), jede Funktion nur einmal.
- Spalten: Objekte (BMK-Baum), Makrodatei (vom Hauptartikel; tauschbar, auch per Drag & Drop), Status
  (grün/gelb/rot; „Meldungen anzeigen“ z. B. „Darstellungsart Allpolig ist in dem Makro nicht vorhanden“),
  Funktionstext, Seite Einpolig / Allpolig / Übersicht (Variante A) / Baugruppenträgeraufbau (Variante E),
  je Makrovariable eine Spalte (Wertesatz oder manuell; gleichnamige Variablen teilen sich eine Spalte).
- Mit Warnungen/Fehlern werden Makros trotzdem platziert; an den betroffenen Kästen/Anschlüssen wird **nur das
  BMK** abgebucht.
- Beschaltungsmakros: für noch nicht platzierte Anschlüsse manuell als Fenstermakro je Anschluss angeben (ein
  Makro kann alle Anschlüsse eines Kanals belegen). Automatische Erkennung bei schon platzierten Anschlüssen:
  in Anschlussrichtung wird der Anfasspunkt eines Makros gesucht (genau auf dem Anschlusspunkt oder nächster
  Rasterpunkt), der Makroname kommt aus dem Makrokasten.
- Aktive Darstellungen bestimmen den Fehlerstatus (nur allpolig erzeugen → fehlende Übersichtsvariante A ist kein
  Fehler). Reihenfolge: **einpolig → allpolig → E/A-Übersichten → Baugruppenträgerübersichten**; beide Übersichten
  werden in Darstellungsart „Übersicht“ platziert.
- Seiten berechnen: Startseite je Darstellungsart (Dialog Startseiten, mit Strukturkennzeichen); weitere Seiten
  werden davon fortgezählt. Makros an ihrer ursprünglichen Position, mehrere je Seite neben- oder untereinander
  (abhängig vom Normblatt).
- Erzeugte Seiten tragen die Seiteneigenschaft **„Automatisch erzeugt“** (Wert = Darstellungsart); beim erneuten
  Erzeugen werden sie überschrieben. Schutz: Eigenschaft auf „Nein“ setzen. Konfiguration des Dialogs lässt sich
  exportieren/importieren.
- Eingang der Funktionen: Navigator, Import einer SPS-Konfigurationsdatei oder Zuordnungsliste + „Mit Gerät
  erzeugen“.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_spsschaltplanerzeugen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_spsschaltplanerzeugen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsschaltplanprinzip.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_startseiten.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_fehler.htm

**Bedeutung für QET:** Viele EPLAN-SPS-Blätter sind generiert: gleichförmige Seiten mit 8 Kanälen, Kanalmakros an
festen Positionen, Seitenfolge einpolig → allpolig → Übersicht. Für einen Generator im Fork gilt dasselbe Muster:
Makro je Kanal aus der Makrodatei des Hauptartikels, Übersicht = Variante A, Träger = Variante E. Seiten mit
„Automatisch erzeugt“ sind beim Nachzeichnen besonders regelmäßig – Abweichungen im PDF deuten auf manuelle
Nacharbeit (Vermutung).

## 20. Bus-Anschlüsse allpolig, SPS-Daten exportieren/importieren, AutomationML

Bus-Anschlüsse allpolig:
- AML ermittelt das Netzwerk aus einpoligen **oder** allpoligen Busanschlüssen, alle anderen Formate nur aus
  einpoligen. Allpolige Busdarstellung ist nicht nötig (Details besser im Kabeldarstellungsplan).
- Ein Busanschluss = ein Stecker = **eine Funktionsschablone**. Gemischt (Bus einpolig, Rest allpolig) über
  Artikelmakros mit ein- und allpoliger Variante; zur korrekten Überlagerung braucht der einpolige Busanschluss
  Steckerbezeichnung (ggf. + Bus-Schnittstellenname) oder Anschlussbezeichnung.
- Alle Adern allpolig: Anschlusszahl am Busanschluss erhöhen und passendes Symbol platzieren; der allpolige
  Busanschluss darf nur **einmal** mit allen Kontakten in einem Symbol stehen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_busanschluesseallpoligdarstellen.htm

SPS-Daten exportieren (Projektdaten > SPS > Daten exportieren, XML):
- Alle gültigen Kästen/Anschlüsse eines **Konfigurationsprojekts** (Wert am Kasten oder an einem Busanschluss).
  Platzierungen einer Funktion werden zusammengefasst; bei Kästen nur die Hauptfunktion. Freie
  Funktionsschablonen und E/A-Einträge aus Zuordnungslisten werden nicht exportiert.
- Daten einsprachig (gewählte Sprache); AML exportiert alle Sprachen, gewählte = Hauptsprache.
- STEP 7 5.6: statt Datei ein Verzeichnis, je Station eine Datei mit Stationsnamen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_exportbuskonfig.htm

SPS-Daten importieren:
- **Funktionen abgleichen:** Vergleich über **BMK und Typbezeichnung**; vorhandene überschrieben, neue als nicht
  platzierte Funktionen. Ethernet-basierte Bussysteme gelten als kompatibel (Ethernet im Projekt ← Profinet aus
  Datei). **Alle Funktionen neu erzeugen:** alles als nicht platzierte Funktionen. Optional anschließend
  „SPS-Schaltplan erzeugen“.
- Bei AML erfolgt die Zuordnung **über die GUID**; Abgleich/Neu erzeugen/Schaltplan erzeugen entfallen dort.
- Ältere Dateiformate: fehlerfreier Austausch nicht garantiert.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_importbuskonfig.htm

AutomationML-Besonderheiten:
- Jedes Objekt und jeder Anschluss (als ExternalInterface) bekommt eine **GUID** (beim Export vergeben, nicht ändern;
  Export nur aus bearbeitbaren Projekten). Entfernen über Komprimieren (Option „AutomationML GUIDs“).
- Zubehör (Artikelpositionen 2–50 der Hauptfunktion) optional mit eigener GUID (Eigenschaft „AutomationML GUID
  (Zubehör)“); braucht SPS-Typbezeichnung am Artikel (z. B. für TIA Selection Tool).
- Exportiert wird **jedes Betriebsmittel mit mindestens einem Busanschluss** (auch Gerätekästen, Motoren).
  Portgenaue Ethernet-Verschaltung abschaltbar (flexible Verkabelung).
- Baugruppennamen je Träger eindeutig: exportiert wird **SPS-Kartenname (ID 20437)**, ersatzweise Objekt-
  beschreibung mit `_1`, `_2` …, sonst erzeugt und gespeichert.
- Funktionstexte von Anschlüssen **und** Kästen (Funktionstext (automatisch)) in allen Sprachen; Hauptsprache als
  Value, weitere als Attribute.
- Einstellbare Anschlüsse (Multifunktion) werden zu normalen E/A und beim Import zurückgewandelt, wenn der Artikel
  eine Schablone „SPS-Anschluss, Multifunktion“ hat.
- Import: leere BMK-Teile überschreiben keine gefüllten; Geräte/Anschlüsse, die in der Datei fehlen, werden
  **„Zum Löschen markiert“** (Anschluss: symbolische Adresse in Datei leer, in EPLAN automatisch gefüllt; Wert
  bleibt), auffindbar mit Prüflauf **004029**. Freie symbolische Adressen sind austauschbar und danach abbuchbar.
- Pflicht-/Optionalfelder am Busanschluss je Bussystem (Ethernet, Profibus DP, ASI, DRIVE-CLiQ, PortToPort,
  ET-Connection, Local-Bus-Erweiterung): Konfigurationsprojekt **20161** (nur wenn abweichend vom Kasten),
  Bus-System **20308**, Steckerbezeichnung **20406** und Bus-Schnittstelle: Name **20447** (Ethernet Pflicht),
  Haupt-Bus-Anschluss **20448**, Physikalisches Netz: Name **20413**, Bus-Adresse/Positionsnummer **20311**
  (und 2: **20386** für ASI-Doppelgeräte), Subnetzmaske **20446**, Logisches Netz: Name **20414**, Bus-Anschluss
  ist Master **20310**, Fehlende Bus-Adresse ignorieren **20412** (für Prüflauf 004037). Leere optionale Felder →
  Import möglich, aber Nacharbeit im SPS-Programm.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_amlbusdatenaustausch.htm

**Bedeutung für QET:** Für einen späteren Datenaustausch aus dem Fork (z. B. AML an TIA) wären je Busport mindestens
Bus-System, Stecker, Schnittstellenname und physikalisches Netz nötig; als Elementinformationen am Port-Element
vorhalten (Vermutung). Für den reinen Nachbau genügt: Ports mit Steckerbezeichnung wie im PDF, Kartenname/Typ als
Elementinfo.

## 21. Schema „SPS-spezifische Einstellungen“ (Adressen, Formate, Zuordnungsliste)

Das Schema wird projektbezogen gespeichert, ist je CPU zuordenbar (sonst Projektschema) und hat vier
Registerkarten:
- **Adressen:** je Adressierungsart (bit-, byte-, wort-, doppelwort-, langwortweise; je Ein- und Ausgang) eine
  **Kennung** (Buchstaben wie E/A oder Ziffern wie 1/2), ein Datentyp (BOOL, BYTE, WORD, DWORD, LWORD; zusätzlich
  INT/DINT/LINT = WORD/DWORD/LWORD, für AML), die Signalart (digital/analog) und die **Schrittweite**. Über
  Kennung + Datentyp erkennt EPLAN SPS-E/A im Schaltplan und filtert beim Listenimport.
- **Adressenformate:** getrennte Formate für digital und analog (Zähler + Trennzeichen, siehe Abschnitt 8).
  **Getrennter Adressbereich für Ein- und Ausgänge** (nur bei gleichen E/A-Formaten): an → Byte/Bit muss über E und
  A eindeutig sein (gibt es A001.0, ist E001.0 verboten); aus → gemeinsamer Bereich, Präfix unterscheidet, E1.0 und
  A1.0 dürfen beide existieren. Bei aus: Karten mit E und A können über **„Adressbereich 2“** am SPS-Kasten einen
  eigenen Ausgangsbereich bekommen.
- **Format der Zuordnungsliste:** Zeichensatz ANSI/ASCII/UNICODE (Unicode nötig für z. B. Chinesisch/Russisch),
  Umlaute zulassen (sonst ersetzt), Kopf-/Fußzeile, Format der Zeile (Elemente: Adresse, Funktionstext – mehrfach
  für mehrteilige/mehrsprachige Texte –, Symbolische Adresse, **Symbolische Adresse (ermittelt)** = angeschlossenes
  BMK, nur Export, Gruppe, Datentyp, Trennzeichen, **Ignorierter Text** für fremde Spalten) und Format der Adresse
  (Kennung, Byte, Bit, Trennzeichen). Längen: unbegrenzt variabel / begrenzt variabel / fest (mit Leerzeichen
  aufgefüllt, optional rechtsbündig). Nach einem Element variabler Länge muss ein Trennzeichen folgen.
- **Format der symbolischen Adresse:** automatische Bildung an/aus; welche Kennzeichnungsblöcke (Funktionale
  Zuordnung … Benutzerdefiniert, je mit Unterkennzeichen) hinein; jedes Vorzeichen (=, +, - …) ersetzbar, weil
  viele Steuerungen keine Sonderzeichen in Symbolen erlauben; BMK und Unterzähler sind **immer** enthalten;
  optional Steckerbezeichnung und Anschlussbezeichnung; ist der SPS-Anschluss mit einer Klemme verbunden, ist die
  **Klemmenbezeichnung immer** Teil der symbolischen Adresse.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_d_spsspezifischeeinstellungen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_r_adressen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_r_adressenformate.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_h_adressenformateeinstellen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_d_formatspsadresse.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_d_formatadressetext.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_r_formatzuordnungsliste.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_r_formatsymbolischeadresse.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_k_start.htm

Zuordnungsliste importieren / abgleichen und exportieren (ergänzend zu Abschnitt 12):
- Formate SDF, SEQ, ASC und beliebige Endungen mit passendem Zeilenaufbau; Dateitypen hängen vom Schema der CPU ab.
- Import in **neue CPU** = reiner Import (Dialog Neue CPU: Konfigurationsprojekt, Stationsname, CPU, Schema);
  Import in vorhandene CPU = **Abgleich**: vorhandene Einträge überschrieben, neue ergänzt.
- **Bezugswert:** CPU wird immer verglichen, dazu entweder Adresse oder symbolische Adresse.
- Mit eingestelltem Adressformat werden nur passende Adressen importiert; „Alle Kennungen importieren“ nimmt auch
  Kennungen ohne EPLAN-Entsprechung (Merker, Timer).
- Funktionstexte werden nur bei Änderung überschrieben (Übersetzungen bleiben). **Sprache:** Die gewählte Sprache
  wird der Liste zugeordnet und ins Wörterbuch geschrieben – falsche Wahl ergibt gemischte Wörterbucheinträge;
  „Alle Anzeigesprachen“ möglich. Ohne „Zeilenumbrüche beibehalten“ gehen Umbrüche verloren und Übersetzungen
  des geänderten Texts werden gelöscht.
- Export: alle Karten mit gleichem CPU-Namen in eine Datei.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_importzuli.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_importzuli.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_exportzuli.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_exportzuli.htm

**Bedeutung für QET:**
- Ob im PDF `E1.0` und `A1.0` gleichzeitig vorkommen dürfen, hängt vom Schalter „Getrennter Adressbereich“ ab;
  eine Prüfung im Fork muss Adressen **mit Präfix** vergleichen (Standardfall gemeinsamer Bereich).
- Eine symbolische Adresse wie `K1_X1_3` oder `A1-K1` ist aus Kennzeichnungsblöcken mit ersetzten Vorzeichen
  gebildet; zum Abgleich mit dem BMK Vorzeichen normalisieren (Vermutung).
- Liest ein Skript eine EPLAN-Zuordnungsliste ein, braucht es das Zeilenformat (Feste/variable Längen,
  Trennzeichen) – SEQ ist tabgetrennt, SDF kommagetrennt mit Anführungszeichen (Vermutung nach Siemens-Konvention,
  nicht aus der Hilfe).

## 22. Nummerierung mit SPS-Daten (BMK, Klemmen, Stecker aus der SPS-Adresse)

- Zweck: angeschlossene Betriebsmittel tragen Teile der SPS-Adresse oder der Anschlussbezeichnung im BMK, Klemmen
  und Steckerkontakte in ihrer Bezeichnung (üblich nach **NFPA**, Klemmenbezeichnung nach **GOST**). Beispiel für
  eine Klemme am Eingang E10.4: `E10.4`, `X1:E10.4`, `X1:10.4`, `X1:104`.
- Nummeriert werden nur Betriebsmittel, deren **Hauptfunktion** im Umfang liegt (Nebenfunktionen immer mit). Am
  Ausgang nur, wenn der angeschlossene Anschluss ein **Aktor**, am Eingang nur, wenn er ein **Sensor** ist.
  SPS-Anschlüsse/-Kästen, geschachtelte, grafische und externe Schaltzeichen werden nie nummeriert; nur digitale
  und analoge E/A zählen, nicht „allgemeine“ SPS-Anschlüsse.
- Adressebenen für das Format (von rechts): 1. Ebene = Bit, 2. = Byte, 3. = Karte/Steckplatz, 4. = Gerät/
  Netzteilnehmer, 5. = Netzstrang; **Präfix** = Buchstaben links bis zum ersten Trennzeichen/zur ersten Ziffer.
  Trennzeichen = jedes Zeichen außer Buchstabe/Ziffer (z. B. %).
- Formatgruppen: **Klemmen**, **Steckerkontakte**, **Betriebsmittel** (nur allgemeine Funktionen; Kästen,
  Geräteanschlüsse, Sammelschienen, Kabel, Stecker, Klemmenleisten bleiben). Reihenfolge der Gruppen egal.
- Umfang je Gruppe: **direkt angeschlossenes erstes Ziel**; **Netz** (alle Enden des Netzes, plus
  **Netzerweiterung** über gewählte Funktionen wie Klemmen hinweg, unabhängig von „Netzverbindend“); **bis zum
  ersten Sensor/Aktor** (dasselbe Ziel wie die automatische symbolische Adresse); **bis zu jedem Sensor/Aktor
  (Netz)**.
- Formatelemente: Daten des SPS-Anschlusses, Daten der SPS-Adresse (gesamt, Byte, Bit, Präfix, Trennzeichen),
  Kennbuchstabe, Seite/Spalte/Zeile (vom SPS-Anschluss oder vom Ziel), Trennzeichen; je Element Mindeststellen,
  Füllzeichen, Leere/Null unterdrücken (mit Nachbartrennzeichen), Sprache.
- Eigenschaften an Funktionen: **Mit SPS-Daten nummeriert** (automatisch gesetzt bei „Als … markieren“),
  **Nummerierung mit SPS-Daten über die Funktion hinaus fortsetzen**; am SPS-Anschluss **Nummerierung mit
  SPS-Daten verhindern**.
- Überschreiben: Alle / nur bereits mit SPS-Daten nummerierte / nur mit „?“ gekennzeichnete. Ergebnisvorschau mit
  zwei Registerkarten (Klemmen-/Steckerkontaktbezeichnungen, Betriebsmittel), Fehlerzellen rot.
- **Wechselwirkung:** Eine spätere normale Klemmen-/BMK-Nummerierung überschreibt die SPS-basierten Bezeichnungen,
  wenn markierte Funktionen nicht ausgeschlossen werden.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_k_start.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_k_prinzip.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_k_eigenschaften.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_h_bmnummerieren.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_h_formatfestlegen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_h_netzerweiterung.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_h_ausschliessen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_h_einstellungen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_einstellungen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_nummerierungmitsps.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_format.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_formateigenschaften.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_formatkennbuchstabe.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_formatseite.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_formattrennzeichen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_netzerweiterung.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_ergebnisvorschau.htm

**Bedeutung für QET:** Tragen Klemmen oder Sensoren im PDF Bezeichnungen wie `X1:E10.4` oder `-B104`, stammen sie
evtl. aus dieser Nummerierung. Für den Nachbau gilt: Bezeichnung so übernehmen, wie sie im PDF steht; nicht
„normalisieren“. Prüfidee: Klemmenbezeichnung ↔ Adresse des verbundenen SPS-Anschlusses muss zusammenpassen.

## 23. Sicherheitskennwerte importieren (VDMA 66413)

- In der Artikelverwaltung (Extras > Importieren, Dateityp „VDMA 66413“) lassen sich Sicherheitskennwerte aus
  einer XML-Datei übernehmen. Im Dialog Zuordnung VDMA 66413 wird eine identifizierende VDMA-Eigenschaft
  (**Identifier** = eindeutige Gerätekennung des Herstellers oder **PartNumber** = Bestell-/Artikelnummer) einer
  eindeutigen EPLAN-Artikeleigenschaft (Artikelnummer, ERP-Nummer, Bestellnummer …) zugeordnet; nur bei eindeutiger
  Zuordnung werden die Werte übertragen.
- Mit Zuordnung auf **Artikelnummer** und Option „Vorhandene aktualisieren und neue hinzufügen“ entstehen auch
  neue Artikel; nur ergänzen = „Nur vorhandene Datensätze aktualisieren“.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportsafetyrelatedvaluesgui_d_vdmaimport.htm

**Bedeutung für QET:** VDMA-66413-Dateien (Sicherheitskennwerte wie PL, SIL, MTTFd, B10d – Vermutung zum Inhalt,
die Hilfeseite nennt die Kennwerte nicht) könnten der Fork-Import an Artikel hängen; für den Schaltplan-Nachbau
nicht nötig. Sicherheitsrelevante SPS-Kanäle sind im Navigator markiert (Abschnitt 6) – im PDF nicht sichtbar.

## 24. Ergänzung: Neue CPU, Konfigurationsprojekt beim AML-Import, AML-Exportoptionen
**Neue CPU** (Adressen/Zuordnungslisten > [Extras] > Zuordnungsliste importieren > Neue CPU): Der CPU-Name setzt sich aus **Konfigurationsprojekt, Stationsname und CPU-Kennung** zusammen, durch Punkte getrennt – deshalb darf keiner der drei Werte einen Punkt enthalten. Zusätzlich wird ein Schema „SPS-spezifische Einstellungen“ gewählt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_d_neuecpu.htm

**Konfigurationsprojekt auswählen** (SPS-Daten importieren, AutomationML): erscheint, wenn das Konfigurationsprojekt der Importdatei im Projekt fehlt. Man wählt ein vorhandenes oder behält das aus der Datei – dann wird ein neues Konfigurationsprojekt angelegt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_konfigprojektwaehlen.htm

**Einstellungen: AutomationML-Export** (SPS-Daten exportieren > [Optionen]):
- *Portgenaue Verschaltung exportieren* (relevant für Ethernet-basierte Busse, DRIVE-CLiQ, PortToPort, ET-Connection, Local-Bus-Erweiterung): an = zusätzlich, welcher Bus-Anschluss mit welchem verbunden ist; aus = nur, welche SPS-Kästen verbunden sind – Verbindungen Baugruppenträger ↔ Erweiterungsträger fehlen dann ganz.
- *Zubehör exportieren*: Zubehör = Artikel auf Position 2–50 der Registerkarte Artikel einer Hauptfunktion; sie brauchen zwingend eine **SPS-Typbezeichnung** in der Artikelverwaltung.
- *Gerätebezogene Konfigurationswerte* (ab AML-Format V1.1.0): benutzerdefinierte Artikeleigenschaften mit Kategorie „SPS-Daten“, Zuordnung „Artikel“ und „Auch für Artikelreferenzen freischalten“; das Präfix bis zum ersten Punkt im identifizierenden Namen wird beim Export entfernt und beim Import aus dem Artikel ergänzt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_optionsaml.htm

**Kompatibilität „Bus-Schnittstellenname in Zielinformationen anzeigen“**: Seit 2.8 identifiziert die Kombination Bus-Schnittstellenname + Steckerbezeichnung einen Bus-Anschluss. Aktiv: Zielanzeigen (Quelle/Ziel, Verbindungen-Navigator, Querverweise, Auswertungen, Klemmen-/Steckerdialoge) zeigen zusätzlich den Schnittstellennamen; aus = nur Steckerbezeichnung (vor 2.8).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektkompatibilitaet.htm

**Bedeutung für QET**: Beim Nachbau von Bus-Zielangaben aus EPLAN-PDFs kann ein Ziel wie „-A1:X1P1“ den Schnittstellennamen enthalten – das ist Teil der Anschluss-Identität, kein Tippfehler. Für einen späteren AML-Export aus dem Fork gilt: ohne portgenaue Daten nur Kasten-zu-Kasten-Verbindungen.

## 25. Quellen

- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsanschluesse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_kanaele.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_kanalzugehoerigkeit.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_kanalorientiertarbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_arbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_spszeichnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_spsquerverweiseerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_adressierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_k_adressformate.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_untergeraete.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_symbolischeadressen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_zuordnungslisten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_austauschbuskonfig.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_exportbuskonfig.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_importbuskonfig.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsschaltplanerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_makrosschaltplanerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_abbuchverhalten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_spsanschluss.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_spskarte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_sps_kaesten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_darstellungsarten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungstypen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_funktionsbezogen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formgeneratorgui_k_auswertungsuebersicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/plcgui_k_ansichten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/plcgui_h_spsanlegen.htm
- https://www.eplan.help/techtipps/de-DE/SPS/Overview-of-supported-PLC-systems.pdf
- https://www.eplan.help/techtipps/de-DE/SPS/TechTip-Configuring-PLC-devices-with-device-description-files.pdf
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_spsdaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_baugrtraegerbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_spseadaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_allgemeinespseinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_automatisch_adressieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_anschluesseneuadressieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_eaneuadressieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_vorschauadressierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_exportadressraumbelegung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_adressenabbuchen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_adressenblockweiseabbuchen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_adressenabbuchen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_anschluesseabbuchen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_blockweiseabbuchen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_anschlblockweiseabbuchenauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_spsschaltplanerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_spsschaltplanerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_spsschaltplanprinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_startseiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_fehler.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_busanschluesseallpoligdarstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_exportbuskonfig.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_importbuskonfig.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_amlbusdatenaustausch.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_d_spsspezifischeeinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_r_adressen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_r_adressenformate.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_h_adressenformateeinstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_d_formatspsadresse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_d_formatadressetext.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_r_formatzuordnungsliste.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_r_formatsymbolischeadresse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcprocessorgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_importzuli.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_importzuli.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_exportzuli.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_exportzuli.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_k_eigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_h_bmnummerieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_h_formatfestlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_h_netzerweiterung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_h_ausschliessen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_h_einstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_einstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_nummerierungmitsps.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_format.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_formateigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_formatkennbuchstabe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_formatseite.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_formattrennzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_netzerweiterung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationplcgui_d_ergebnisvorschau.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/partsimportsafetyrelatedvaluesgui_d_vdmaimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_ansichten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_h_spsanlegen.htm

Zusatz-Dialogseiten (Ergänzung Abschnitt 24):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_d_neuecpu.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_konfigprojektwaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_optionsaml.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektkompatibilitaet.htm
