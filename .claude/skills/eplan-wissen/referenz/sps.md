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
15. Quellen

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

## 15. Quellen

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
