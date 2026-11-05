# EPLAN-Prüflaufmeldungen: Nachschlagetabelle (Ursache, Korrektur, QET-Prüfweg)

Alle 586 Einzelseiten `messages_p_<Nr>.htm` der EPLAN-Plattform-Hilfe 2.9, je Meldung in eigenen Worten: was die Meldung auslöst und was im Plan zu korrigieren ist, dazu der Prüfweg im QET-Nachbau. Mechanik der Prüfläufe, Prüfliste und Prüfskript-Vorschlag stehen in [pruefungen.md](pruefungen.md); die aus den Einzelseiten abgeleiteten fachlichen Regeln sind dort in §8.10 gesammelt.

**Legende QET-Prüfweg:** `qet_check` (doppelte/fehlende BMK, unnummerierte Leiter, Master ohne Herstellerreferenz, überfüllte Anschlüsse, Reports mit mehreren Leitern) · `qet_continuity` (offene Anschlüsse, Potenzialwidersprüche Nummer/Farbe/Querschnitt/Kabel, Report-Paare) · `qet_layout_check` (Überlappung, Raster) · `eigenes Skript` (QET-XML auswerten) · `Sichtprüfung` · `nicht relevant` (Funktion gibt es im QET-Nachbau nicht). **Regel** = Meldung liefert direkt eine Regel für den QET-Nachbau oder das Verknüpfungsskript `verknuepfen.py`.

**URL-Schema:** jede Meldung Pxxxxxx hat die Seite `https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_<Nr>.htm` (z. B. https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001001.htm). Abweichende URLs gab es nicht.

Hinweis: Einige Seiten haben nur eine „Beschreibung“ ohne Ursache/Lösung (fast alle 020, 030004–030008, P004029, P007026, P013053, P017009, P022011–P022013, P022018, P022020); dort ist die Zeile aus der Beschreibung abgeleitet. Alle 30 Meldungen 029001–029030 haben denselben Text.

## Inhalt

- 001 Klemmen (25 Meldungen)
- 002 Stecker (14 Meldungen)
- 003 Kabel (21 Meldungen)
- 004 SPS / Bus (78 Meldungen)
- 005 Verbindungen / Potenziale (78 Meldungen)
- 007 Geräte (33 Meldungen)
- 008 Fremdsprachen / Übersetzung (5 Meldungen)
- 010 Querverweise (2 Meldungen)
- 011 Abbruchstellen (8 Meldungen)
- 012 2D-Schaltschrankaufbau (9 Meldungen)
- 013 Artikeldaten / Stücklistenimport (9 Meldungen)
- 014 Makro (6 Meldungen)
- 015 Auswertungen (4 Meldungen)
- 016 Gerätekästen (2 Meldungen)
- 017 BMK (13 Meldungen)
- 020 Projektvergleich (37 Meldungen)
- 021 Platzhalterobjekte (5 Meldungen)
- 022 Sonstiges (20 Meldungen)
- 024 Fluid (18 Meldungen)
- 025 Projekteinstellungen (1 Meldung)
- 026 3D-Montageaufbau (69 Meldungen)
- 027 Topologie (23 Meldungen)
- 028 Vorplanung (31 Meldungen)
- 029 Vorplanungstiefe (30 Meldungen)
- 030 Teilprojekte (8 Meldungen)
- 032 Bohrbild (2 Meldungen)
- 033 Kabelbaum (3 Meldungen)
- 501 Artikelstammdaten (32 Meldungen)
- Quellen

## 001 Klemmen (25 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_001nnn.htm – Nummern: 001001, 001005, 001008, 001010, 001011, 001012, 001015, 001016, 001021, 001022, 001023, 001024, 001025, 001026, 001027, 001028, 001029, 001030, 001031, 001032, 001033, 001034, 001035, 001036, 001037

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 001001 | Klemme ohne Bezeichnung | Klemme ist im Navigator angelegt oder im Plan gesetzt, trägt aber keine Klemmenbezeichnung (z. B. „1“). | Jeder Klemme eine Bezeichnung geben; BMK der Leiste allein genügt nicht. | eigenes Skript **Regel** |
| 001005 | Gleichbezeichnete Klemme mehrfach verwendet | Gleiches BMK und gleiche Bezeichnung mehrfach vorhanden, obwohl Mehrfachvergabe nicht erlaubt ist. | Eindeutige Bezeichnung vergeben oder an der Klemme „Gleiche Bezeichnung erlaubt“ setzen (gewollte Doppelung, z. B. verteilte Darstellung). | qet_check **Regel** |
| 001008 | Klemmenanschluss hat zu viele Ziele | Ein Funktionsanschluss der Klemme ist mit mehr Schaltzeichen verbunden als „Anzahl Ziele“ in der Anschlusslogik zulässt. | Überzählige Ziele entfernen oder „Anzahl Ziele“ je Anschluss erhöhen (Doppelstockklemme/Zweileiteranschluss). | qet_check **Regel** |
| 001010 | Klemmenleistendefinition fehlt | Klemme existiert ohne zugehörige Klemmenleistendefinition mit gleichem BMK. | Klemmenleistendefinition mit dem BMK der Klemme anlegen bzw. platzieren. | eigenes Skript **Regel** |
| 001011 | Klemmenleiste ohne Klemmen | Klemmenleistendefinition vorhanden, aber keine Klemme mit ihrem BMK. | Mindestens eine Klemme mit BMK der Leiste und Bezeichnung anlegen oder Definition löschen. | eigenes Skript |
| 001012 | Falsche Stegbrückenbildung | Stegbrücke zwischen Etagen einer Mehrstockklemme oder an Klemmen mit „Anzahl Stegbrücken“ = 0. | Zwischen Etagen Drahtbrücke statt Stegbrücke verwenden; sonst Anzahl Stegbrücken am Anschluss ≥ 1 setzen. | eigenes Skript **Regel** |
| 001015 | Zu viele Drahtbrückenebenen | Klemmenplan-Formular hat weniger Brückenebenen, als übereinanderliegende Drahtbrücken auftreten. | Formulareigenschaft „Brücken: Anzahl der Ebenen“ erhöhen; Prüfung nur bei Ebenen > 0 und nur Leisten mit Definition. | nicht relevant |
| 001016 | Klemme ohne Ziele | Platzierte Klemme ist mit keinem Schaltzeichen verbunden; unplatzierte Klemmen werden nicht geprüft. | Mindestens einen Klemmenanschluss anschließen oder Klemme als Reserve kennzeichnen/entfernen. | qet_continuity |
| 001021 | Klemme nicht in Leistendefinition enthalten | Leiste hat Funktionsschablonen, eine Klemme gehört aber nicht dazu. | Funktionsschablonen der Leiste neu erzeugen, damit alle Klemmen enthalten sind. | nicht relevant |
| 001022 | Übereinanderliegende Stegbrücken | Bei einfachen Klemmen würden sich mehrere Stegbrücken durch die Klemmenreihenfolge überlappen. | Klemmenreihenfolge so ändern, dass Brücken hintereinander liegen, oder Drahtbrücken verwenden. | eigenes Skript **Regel** |
| 001023 | Hauptklemme nicht erster Schablone zugeordnet | Hauptklemme landet bei einer späteren statt der ersten Funktionsschablone des Klemmenartikels. | Andere Klemme zur Hauptklemme machen oder passenden Klemmenartikel wählen. | nicht relevant |
| 001024 | Intern/extern-Anschlüsse verbunden | Verbundene Klemmenanschlüsse haben unterschiedliche Einstellung Intern/Extern. | Anschlusseigenschaft Intern/Extern an verbundenen Anschlüssen vereinheitlichen. | eigenes Skript **Regel** |
| 001025 | Anschlussbezeichnung an Brückenanschluss | Stegbrückenanschluss mit angeschlossener Steg- oder interner Brücke trägt eine Anschlussbezeichnung. | Anschlussbezeichnung am Brückenanschluss löschen. | Sichtprüfung |
| 001026 | Unverbundene Teilklemmen gleicher Bezeichnung | Teilklemmen gleicher Bezeichnung sind nicht verbunden; Einstellung „Zusammengehörige Teilklemmen automatisch verbinden“ aus. | Projekteinstellung (Betriebsmittel > Klemmen) aktivieren oder Teilklemmen logisch verbinden. | qet_continuity **Regel** |
| 001027 | Einzelne Teilklemme | Nur eine einzige Teilklemme mit dieser Klemmenbezeichnung im Projekt. | Zweite Teilklemme gleicher Bezeichnung ergänzen (nicht per Autoconnect verbunden) oder normale Klemme verwenden. | eigenes Skript |
| 001028 | Teilklemmen mit unterschiedlicher Funktionsdefinition | Teilklemmen einer Klemme haben verschiedene Funktionsdefinitionen. | Gleiche Funktionsdefinition für alle Teilklemmen einer Klemme. | eigenes Skript |
| 001029 | Verbindungsdefinitionspunkt zwischen Teilklemmen | Bei Brückenoptimierung liegt ein Verbindungsdefinitionspunkt auf Verbindung zwischen Stegbrückenanschlüssen verschieden bezeichneter Teilklemmen. | Punkt löschen oder Einstellung „Brückenoptimierung“ ausschalten. | nicht relevant |
| 001030 | Belegter Teilklemmenanschluss ohne Bezeichnung | Angeschlossener Teilklemmenanschluss (kein Stegbrückenanschluss) hat keine Anschlussbezeichnung. | Anschlussbezeichnung an der Teilklemme ergänzen. | eigenes Skript |
| 001031 | Teilklemme nicht optimierbar | Beim Optimieren passt der gewählte Artikel nicht: unbekannte Bezeichnungen, zu wenig Anschlüsse oder kein Klemmenartikel. | Klemmenartikel mit passender Funktionsdefinition und Anschlussbezeichnungen wählen. | nicht relevant |
| 001032 | Schaltbrücke nicht korrekt geschlossen | Brückenanfang/-ende ohne externen oder internen Anschluss, kein Stegbrückenanschluss, oder Brücke an letzter Klemme ohne Nachbar. | Intern/Extern bzw. Anschlussart „Stegbrücke“ korrigieren; Schaltbrücke an letzter Klemme auf „Keine“. | nicht relevant |
| 001033 | Manuelle Stegbrücke nicht erzeugbar | Wert „Manuelle Stegbrücken (intern/extern)“ ungültig, zu lang (keine Klemme am Ende) oder Klemme ohne passenden Stegbrückenanschluss. | Eigenschaft löschen und Brücke neu erzeugen; Klemme mit internem/externem Stegbrückenanschluss wählen. | nicht relevant |
| 001034 | Alte manuelle Stegbrückeneinstellung | Manuelle Stegbrücke aus EPLAN ≤ 2.4 (z. B. „Brückenanfang“) im Projekt, Logik ab 2.5 geändert. | Feld Stegbrücke auf „Automatisch“, manuelle Brücke neu erzeugen. | nicht relevant |
| 001035 | Teilklemmenanschlüsse mit gleicher Bezeichnung | Mehrere Teilklemmen tragen dieselbe Anschlussbezeichnung. | Jeder Teilklemme eine eindeutige Anschlussbezeichnung geben. | eigenes Skript |
| 001036 | Lokale Funktionsschablonen an Klemmenleiste | Leiste (aus Version < 2.4) hat lokale Schablonen trotz Artikel mit Schablonen. | Artikelmenge kurz ändern, damit lokale Schablonen verworfen werden. | nicht relevant |
| 001037 | Mehrfach vorhandene Hauptklemmen | Dieselbe Hauptklemme in mehreren Darstellungsarten (z. B. allpolig und Übersicht). | Nur eine Darstellung als Hauptklemme, übrige Nebenklemme oder umbenennen. | eigenes Skript **Regel** |

## 002 Stecker (14 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_002nnn.htm – Nummern: 002001, 002002, 002003, 002004, 002005, 002006, 002007, 002008, 002009, 002010, 002012, 002013, 002014, 002015

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 002001 | Steckerkontaktanschluss hat zu viele Ziele | Ein Funktionsanschluss des Steckerkontakts hat mehr Verbindungen als „Anzahl Ziele“ erlaubt. | Überzählige Verbindungen entfernen oder Anzahl Ziele erhöhen. | qet_check **Regel** |
| 002002 | Steckerkontakt ohne Ziele | Platzierter Steckerkontakt unverbunden oder nur im Navigator vordefiniert, nicht platziert. | Kontakt platzieren und mindestens einen Anschluss verbinden. | qet_continuity |
| 002003 | Steckerdefinition fehlt | Steckerkontakt ohne Steckerdefinition gleichen BMK (für Buchsen, Stifte oder beides). | Passende Steckerdefinition mit BMK des Steckers anlegen. | eigenes Skript **Regel** |
| 002004 | Stecker ohne Steckerkontakte | Steckerdefinition vorhanden, aber kein Kontakt mit ihrem BMK. | Mindestens einen Steckerkontakt mit BMK und Bezeichnung anlegen. | eigenes Skript |
| 002005 | Nur Stiftseite vorhanden | Steckerdefinition „für Stifte“ ohne Gegenstück „für Buchsen“ gleichen BMK. | Buchsen-Steckerdefinition mit gleichem BMK ergänzen. | eigenes Skript **Regel** |
| 002006 | Nur Buchsenseite vorhanden | Steckerdefinition „für Buchsen“ ohne Gegenstück „für Stifte“ gleichen BMK. | Stift-Steckerdefinition mit gleichem BMK ergänzen. | eigenes Skript **Regel** |
| 002007 | Nicht zusammengehörige Steckerdefinitionen | Zwei Steckerdefinitionen gleichen BMK haben unpassende Funktionsdefinitionen; keine Stift/Buchse-Paarung. | Eine Definition auf „für Buchsen“, die andere auf „für Stifte“ stellen. | eigenes Skript |
| 002008 | Ungleiche Anzahl Buchsen und Stifte | Getrennte Stift-/Buchsendarstellung: eine Seite hat mehr Kontakte als die andere. | Fehlende Kontakte ergänzen, bis beide Seiten gleich viele haben. | eigenes Skript **Regel** |
| 002009 | Formularposition doppelt belegt | Ziele an einem Steckerkontaktanschluss haben gleichnamige Formularpositionen. | Formularpositionen der Ziele eindeutig benennen. | nicht relevant |
| 002010 | Falsche Stegbrückenbildung (Stecker) | Stegbrücke zwischen Steckerkontakten (Winkel mit VDP „Stegbrücke“ oder Brückenanfang/-ende), aber Anzahl Stegbrücken = 0. | Anzahl Stegbrücken ≥ 1 setzen oder VDP auf „Verbindung allgemein“. | nicht relevant |
| 002012 | Stift auf Stift / Buchse auf Buchse | Paar über Direktanschluss mit richtiger Seitenzuordnung, aber Funktionsdefinition einer Seite vertauscht. | Stift „Steckerstift“, Buchse „Steckerbuchse“ als Funktionsdefinition. | Sichtprüfung **Regel** |
| 002013 | Zu viele Verbindungen auf Steckseite | Auf Stift- oder Buchsenseite hat ein Kontakt eine bereits vergebene Bezeichnung. | Kontaktbezeichnung je Seite eindeutig machen. | qet_check |
| 002014 | Steckerkontakt mehrfach verwendet | Gleiches vollständiges BMK samt Kontaktbezeichnung mehrfach im Projekt. | Eindeutige Kontaktbezeichnungen; nur Stift und zugehörige Buchse dürfen gleich heißen. | qet_check **Regel** |
| 002015 | Lokale Funktionsschablonen an Steckerdefinition | Steckerdefinition (aus Version < 2.4) hat lokale Schablonen trotz Artikel mit Schablonen. | Artikelmenge kurz ändern, damit lokale Schablonen gelöscht werden. | nicht relevant |

## 003 Kabel (21 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_003nnn.htm – Nummern: 003001, 003002, 003003, 003005, 003006, 003007, 003008, 003009, 003011, 003012, 003013, 003014, 003018, 003019, 003022, 003023, 003024, 003025, 003026, 003027, 003028

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 003001 | Automatische Kabelauswahl abgebrochen | Kabel-Vorauswahl der automatischen Kabelauswahl enthält keinen Kabelartikel. | Kabelartikel in die Vorauswahl (Einstellungen Kabel automatisch) aufnehmen. | nicht relevant |
| 003002 | Kein passendes Filterkriterium | Manuelle Kabeldaten (Adernzahl, Spannung, Querschnitt) passen zu keinem Artikel der Vorauswahl. | Passenden Kabelartikel zur Vorauswahl ergänzen. | nicht relevant |
| 003003 | Verbindungs-BMK widerspricht Kabel-BMK | VDP im Bereich einer Kabeldefinitionslinie/Abschirmung hat eigenes, abweichendes BMK. | Eigenes BMK am VDP löschen, BMK kommt vom Kabel. | qet_continuity **Regel** |
| 003005 | Kabelziel ohne vollständiges BMK | Quell- oder Zielfunktion des Kabels hat kein vollständiges BMK. | Quelle und Ziel mit vollständigem BMK versehen. | qet_check |
| 003006 | Kabel zwischen gleichem Bauteil | Quelle und Ziel des Kabels haben dasselbe BMK und sind querverwiesen. | Unterschiedliche BMK für Quelle und Ziel. | eigenes Skript |
| 003007 | Mehrere Kabelziele | Nummerierungsformat „Nach Quelle und Ziel“, aber Kabel hat mehrere Ziele. | Nur ein Ziel je Kabel oder anderes Nummerierungsformat. | nicht relevant |
| 003008 | Kabelverbindung ohne Kabel | VDP liegt nicht mehr auf der Kabeldefinitionslinie bzw. nicht im Bereich der Abschirmung. | VDP auf den Schnittpunkt schieben oder Kabellinie/Abschirmung neu zeichnen. | eigenes Skript |
| 003009 | Unterschiedliche Kabeldaten | Verteilt dargestelltes Kabel: Haupt- und Nebenfunktionen mit widersprüchlichem Typ, Adernzahl, Querschnitt. | Identische Kabeldaten an allen Darstellungen. | qet_continuity **Regel** |
| 003011 | Zu viele Kabelverbindungen | Eingetragene Verbindungsanzahl kleiner als tatsächlich verwendete Adern (Schirm, freie Schablonen zählen nicht). | Verbindungsanzahl erhöhen oder passenden Kabeltyp wählen. | eigenes Skript **Regel** |
| 003012 | Farbe/Nummer doppelt belegt | Zwei Adern gleiche Farbe/Nummer; gleichnamige Schirme bei Paaren gleicher Farbe; Paare mit gleichem Paarindex. | Aderkennungen eindeutig; Schirme eindeutig benennen; eindeutige Paarindizes. | eigenes Skript **Regel** |
| 003013 | PE-Ader mehrfach belegt | Mehrere PE/PEN-Adern, obwohl am Kabel nur eine zugelassen ist. | Potenzialtyp einer Ader ändern oder „Mehrere PE-Adern zulassen“ am Kabel setzen. | eigenes Skript **Regel** |
| 003014 | N-Ader doppelt belegt | Kabel hat zwei oder mehr Adern mit Potenzialtyp N. | Potenzialtyp einer Ader korrigieren. | eigenes Skript **Regel** |
| 003018 | Abschirmung zu oft aufgelegt | Schirm gleichen Namens mehrfach aufgelegt, beidseitiges Auflegen am Kabel nicht erlaubt. | Einseitig auflegen oder „Beidseitig aufgelegte Abschirmungen zulassen“ aktivieren. | eigenes Skript |
| 003019 | Unterkabel erzeugt | Hinweis: automatische Kabelerzeugung hat Unterkabel angelegt. | – (Information, durch neuen Prüflauf löschbar). | nicht relevant |
| 003022 | Ungültige Kabellänge | Negative Kabellänge eingetragen. | Positive Länge eintragen; Einheiten wie „2M“ werden automatisch zu 2 m. | Sichtprüfung |
| 003023 | Artikelmenge widerspricht Längensummierung | „Beim Summieren der Kabellängen nicht berücksichtigen“ aktiv und Artikelmenge 0. | Eine der beiden Einstellungen ändern (Menge > 0 oder Summierung zulassen). | nicht relevant |
| 003024 | Eigensicherheitsstatus Kabel ≠ Adern | Kabel eigensicher, aber nicht alle Kabelverbindungen (oder umgekehrt). | Eigenschaft Eigensicher an Kabel und allen Adern einheitlich. | nicht relevant |
| 003025 | Verbindungsanzahl nicht angegeben | An der Kabel-Hauptfunktion fehlt die Adernzahl. | Verbindungsanzahl eintragen oder Kabeltyp per Artikel zuweisen. | eigenes Skript |
| 003026 | Kabellänge nicht angegeben | An der Kabel-Hauptfunktion fehlt die Länge. | Länge eintragen. | eigenes Skript |
| 003027 | Funktionstext des Kabels unterschiedlich | Automatischer Funktionstext weicht zwischen Platzierungen eines verteilten Kabels ab. | Funktionstext an allen Platzierungen angleichen. | eigenes Skript |
| 003028 | Kabel nutzt nicht alle Adern des Artikels | Kabel als Gerät platziert, aber weniger Adern verwendet als der Artikel bietet. | Passenden Artikel wählen; abhängig von „Freie Funktionen minimieren“/„Anzahl freie Funktionen“. | nicht relevant |

## 004 SPS / Bus (78 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_004nnn.htm – Nummern: 004003, 004004, 004005, 004006, 004007, 004009, 004010, 004013, 004016, 004018, 004019, 004020, 004021, 004022, 004023, 004024, 004025, 004027, 004028, 004029, 004030, 004031, 004035, 004036, 004037, 004038, 004039, 004040, 004045, 004046, 004050, 004051, 004052, 004053, 004054, 004055, 004056, 004057, 004058, 004070, 004071, 004072, 004073, 004080, 004081, 004082, 004083, 004084, 004085, 004086, 004087, 004088, 004089, 004090, 004091, 004092, 004093, 004094, 004095, 004096, 004097, 004098, 004099, 004100, 004101, 004102, 004103, 004104, 004106, 004107, 004108, 004109, 004110, 004111, 004200, 004201, 004202, 004203

Vorbemerkung: Viele 004-Prüfungen greifen nur an SPS-Kästen bzw. Bus-Anschlüssen, an denen ein **Konfigurationsprojekt** eingetragen ist (in der Tabelle als „nur mit Konfig.-Projekt“ vermerkt). Meldungen, die „durch den SPS-Import“ oder den Adressierungslauf entstehen, sind modulbezogen und verschwinden erst mit einem neuen Prüflauf.

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 004003 | Konfigurationswert am SPS-Kasten nicht numerisch | Adressierungsschema zählt über einen Strukturwert (z. B. An Position / Steckplatz), am SPS-Kasten steht dort aber Text. | Im verwendeten Strukturfeld rein numerischen Wert eintragen; Meldung stammt aus dem Adressierungslauf. | nicht relevant |
| 004004 | Bus-Anschluss fehlt auf Übersicht | Bus-Anschluss im Schaltplan findet kein Gegenstück auf der Übersichtsseite, weil das BMK am SPS-Kasten dort fehlt/abweicht. | Gleiches BMK an Bus-Anschluss in Schaltplan und Übersicht vergeben. | eigenes Skript **Regel: Übersichts- und Schaltplan-Darstellung einer Karte nur über identisches BMK + Anschluss verknüpfen** |
| 004005 | SPS-Anschluss fehlt auf Übersicht | SPS-Anschluss im Schaltplan hat kein Gegenstück in der Übersicht; BMK am Übersichts-SPS-Kasten fehlt oder weicht ab. | Am SPS-Kasten in Schaltplan und Übersicht dasselbe BMK eintragen. | eigenes Skript **Regel: jeder Kanal im Schaltplan braucht Gegenstück mit gleichem Karten-BMK und Anschluss auf der SPS-Übersicht (und umgekehrt)** |
| 004006 | SPS-Kasten ohne Anschluss | SPS-Kasten enthält keinen einzigen SPS-Anschluss; geprüft auf Schaltplan- und Übersichtsseiten. | Mindestens einen SPS-Anschluss in den Kasten setzen. | eigenes Skript **Regel: SPS-Kasten/Kartenrahmen ohne Kanal-Element ist Fehler** |
| 004007 | Baugruppenträger steckt auf sich selbst | Feld Baugruppenträger und „SPS-Karte steckt auf Baugruppenträger“ tragen denselben Namen. | Unterschiedliche Werte eintragen; nur mit Konfig.-Projekt. | nicht relevant |
| 004009 | Steckplatz am Baugruppenträger doppelt | Zwei Karten mit gleichem Träger und Konfig.-Projekt belegen dieselbe Steckplatzposition. | Steckplätze eindeutig machen; erlaubt ist gleiche Nummer, wenn eine Karte auf dem Träger (seitlich), die andere auf der Kopfstation (oben) steckt. | eigenes Skript **Regel: Steckplatz je Träger eindeutig** |
| 004010 | Unbekannter Baugruppenträger | Am Steckplatz ist ein Trägername eingetragen, den kein SPS-Kasten definiert. | Träger anlegen oder vorhandenen Träger angeben; nur mit Konfig.-Projekt. | nicht relevant |
| 004013 | Ringschluss Baugruppenträger | Zwei SPS-Kästen verweisen über Träger/„steckt auf“ gegenseitig aufeinander. | Strukturdaten so ändern, dass kein Zyklus entsteht. | nicht relevant |
| 004016 | Baugruppenträger mehrfach | Derselbe Trägername ist an mehreren SPS-Kästen als Träger vergeben. | Trägernamen eindeutig vergeben; nur mit Konfig.-Projekt. | nicht relevant |
| 004018 | Kein Aktor am Ausgang | Zielverfolgung findet am SPS-Ausgang keinen Aktor: Signalart an Ausgang/Aktor falsch oder Zielverfolgung an Klemme/Stecker dazwischen fehlt. | Signalart passend zur Funktionsdefinition (DA → Digitalausgang), am Aktor Signalart „Aktor“; Klemme/Stecker Zielverfolgung (SPS) Anschluss 1 = 2, Anschluss 2 = 1. | qet_continuity + eigenes Skript **Regel: jeder belegte Ausgang muss über Leiter/Klemmen bis zu einem Aktor verfolgbar sein** |
| 004019 | Kein Sensor am Eingang | Zielverfolgung findet am SPS-Eingang keinen Sensor; Signalart oder Zielverfolgung an zwischengeschalteter Klemme/Stecker falsch. | DE → Signalart Digitaleingang, am Sensor Signalart „Sensor“; Klemme/Stecker Zielverfolgung 1 = 2, 2 = 1. | qet_continuity + eigenes Skript **Regel: jeder belegte Eingang muss über Leiter/Klemmen bis zu einem Sensor verfolgbar sein** |
| 004020 | Mehrere E/A-Anschlüsse an einem Kanal | Mehrere E/A-Anschlüsse tragen dieselbe Kanalbezeichnung. | Je Kanal nur ein aktiver E/A-Anschluss; weitere Darstellungen als „Deaktivierter E/A-Anschluss“ kennzeichnen. Karten mit E und A im selben Kanal → P004097. | eigenes Skript **Regel: je Karte und Kanal genau ein aktiver E/A-Anschluss** |
| 004021 | Adresse doppelt innerhalb CPU | Eine SPS-Adresse kommt innerhalb derselben CPU mehrfach vor. | Adressen je CPU eindeutig machen; CPU-Zuordnung über Eigenschaften CPU und CPU: Name am SPS-Kasten. | eigenes Skript **Regel: SPS-Adresse je CPU eindeutig** |
| 004022 | Anschlussbeschreibung im Kanal doppelt | Innerhalb eines Kanals tragen z. B. E-Anschluss und Plus-Versorgung dieselbe Anschlussbeschreibung. | Jedem Anschluss des Kanals eigene Anschlussbeschreibung geben. | eigenes Skript **Regel: Anschlussbeschreibung innerhalb eines Kanals eindeutig** |
| 004023 | Fehlende Adresse | SPS-Anschluss ohne Adresse; „SPS-Anschluss, intern, allgemein“ ist ausgenommen. | Adresse eintragen. | eigenes Skript **Regel: E/A-Kanal ohne Adresse melden (interne Anzeige-Anschlüsse ausnehmen)** |
| 004024 | Fehlende Anschlussbeschreibung | SPS-Anschluss ohne Anschlussbeschreibung; interne allgemeine Anschlüsse ausgenommen. | Anschlussbeschreibung (Klemmenbezeichnung der Karte) eintragen. | eigenes Skript **Regel: Kanal-Element ohne Anschlussbeschreibung melden** |
| 004025 | Fehlender Steckplatz | Träger bei „steckt auf Baugruppenträger“ angegeben, Steckplatzposition aber leer. | Steckplatz ergänzen; nur mit Konfig.-Projekt. | nicht relevant |
| 004027 | Kanalbezeichnung fehlt oder fehlerhaft | Kanalbezeichnung an DE/DA/AE/AA leer, alphanumerisch oder negativ; interne allgemeine Anschlüsse ausgenommen. | Kanalbezeichnung als reine Zahl > 0 eintragen. | eigenes Skript **Regel: Kanalnummer rein numerisch und > 0** |
| 004028 | Widerspruch E/A zu Versorgung im Kanal | Ein-/Ausgang und Versorgungsanschluss desselben Kanals (gleiches Karten-BMK, Stecker, Kanal) haben unterschiedliche Funktionstexte, Adressen, Datentypen o. ä. | Felder angleichen; leere Felder lösen nichts aus, nur zwei verschieden gefüllte. | eigenes Skript **Regel: Kanaldaten (Adresse, Funktionstext, symb. Adresse) an allen Anschlüssen eines Kanals gleich, leere Felder tolerieren** |
| 004029 | Objekt zum Löschen markiert | SPS-Objekt ist im Konfig.-Projekt, fehlte aber in der Importdatei (bzw. AML: symbolische Adresse leer) und wurde markiert. | Markierte Objekte prüfen und ggf. löschen; Markierung bleibt nach Meldung bestehen. | nicht relevant |
| 004030 | Fehlende symbolische Adresse | Keine symbolische Adresse und kein angeschlossenes Ziel mit BMK, aus dem sie automatisch gebildet würde. | Symbolische Adresse eintragen oder Kanal mit Sensor/Aktor mit BMK verbinden. | eigenes Skript **Regel: symbolische Adresse = BMK des angeschlossenen Geräts, sonst manuell** |
| 004031 | Symbolische Adresse doppelt in CPU | Gleiche symbolische Adresse an zwei SPS-Anschlüssen derselben CPU. | Symbolische Adresse eindeutig ändern. | eigenes Skript **Regel: symbolische Adresse je CPU eindeutig** |
| 004035 | Bus-Adresse doppelt im logischen Netz | Gleiche Bus-Adresse (Positionsnummer 1 oder 2) im selben logischen Netz; nur mit Konfig.-Projekt. | Adressen unterscheiden oder anderem logischen Netz zuordnen. | eigenes Skript |
| 004036 | Unbekanntes logisches Netz | Slave verweist auf logisches Netz, zu dem kein Master-Bus-Anschluss existiert. | Master mit diesem Netznamen anlegen oder vorhandenen Netznamen wählen. | nicht relevant |
| 004037 | Fehlende Bus-Adresse | Haupt-Bus-Anschluss (Netzwerk-/Bus-Kabelanschluss, allgemein) ohne Bus-Adresse. | Adresse eintragen oder „Fehlende Bus-Adresse ignorieren“ setzen; Neben-Bus-Anschlüsse (Schnittstellenname gefüllt, nicht Haupt) werden nicht gemeldet. | eigenes Skript **Regel: Bus-Anschluss identifiziert sich über Anschluss + Stecker + Schnittstellenname** |
| 004038 | Datenrate am Bus-Slave | Am Slave ist eine Datenübertragungsrate eingetragen; erlaubt nur am Master. | Datenrate am Slave entfernen. | nicht relevant |
| 004039 | Verbindung zwischen Bus-Systemen | Bus-Anschlüsse unterschiedlicher Bus-Systeme (z. B. Profibus mit Ethernet) sind verbunden. | Beide Seiten auf gleiches Bus-System stellen. | eigenes Skript **Regel: Busleitung nur zwischen Anschlüssen gleichen Bus-Systems** |
| 004040 | Widersprüchliche Daten an zugehörigem Bus-Anschluss | Bus-Kabelanschlüsse mit gleichem BMK, Bus-System und Schnittstellennamen gelten als intern verbunden, haben aber abweichende Netzdaten (Netzname, Master, Adresse, Gerätename …). | Daten an allen zusammengehörigen Bus-Anschlüssen angleichen. | eigenes Skript **Regel: Bus-Anschlüsse mit gleichem BMK + Bus-System + Schnittstelle sind ein Anschluss mit identischen Daten** |
| 004045 | Kommunikationseinheit am SPS-Kasten | Alte Projekte/Makros enthalten SPS-Kästen mit veralteten Kommunikationseinheiten. | Daten auf Bus-Anschluss übertragen, Kommunikationseinheiten per Komprimieren entfernen. | nicht relevant |
| 004046 | Konflikt bei Übernahme SPS-Daten | Seit 2.4 liegen SPS-Strukturdaten nur an der Hauptfunktion; Daten der Nebenfunktionen ließen sich nicht übertragen. | Daten an Hauptfunktion bereinigen, dann Komprimieren mit „SPS-Strukturdaten an SPS-Nebenfunktionen entfernen“. | eigenes Skript **Regel: Kartendaten (Typ, Adressen, Steckplatz) nur am Master-Element der Karte halten** |
| 004050 | Verbindung fehlt (Import) | SPS-Import kennt eine Geräteverbindung, die im Projekt fehlt. | Verbindung mit Bus-Anschlüssen herstellen. | nicht relevant |
| 004051 | Gerät nicht am physikalischen Netz | SPS-Import erwartet Gerät am physikalischen Netz, Verbindung fehlt im Projekt. | Gerät per Bus-Anschluss ans Netz anschließen. | nicht relevant |
| 004052 | Verbindung zum falschen Netz | Projektverbindung widerspricht der importierten Verbindung. | Verbindung an das gemeldete Netz umhängen. | nicht relevant |
| 004053 | Zu viele Netzwerkunterschiede | Widerspruch zwischen Import und Projekt zu komplex zur Zuordnung. | Alle Bus-Anschlüsse auf richtiges Netz prüfen. | nicht relevant |
| 004054 | Verbindung nur im Projekt | Netzverbindung im Projekt, die im letzten Import fehlte. | Verbindung entfernen oder Konfiguration nachziehen. | nicht relevant |
| 004055 | Mehr als ein Baugruppenträger (Import) | Schneider Unity Pro M340: Verbindungskarten zwischen Trägern fehlen in der Importdatei. | Trägererweiterungen mit richtiger Hardware projektieren. | nicht relevant |
| 004056 | Artikel nicht gefunden (SPS-Import) | Kein Artikel mit passendem Gerätebeschreibungs-Dateinamen (AML: plus Index) bzw. SPS-Typbezeichnung. | Artikel mit diesen Eigenschaften anlegen, Import wiederholen. | nicht relevant |
| 004057 | Mehrere Artikel zur SPS-Typbezeichnung | Typbezeichnung passt auf mehrere Artikel (doppelt oder Platzhalterzeichen); der erste wird genommen. | Artikeldaten bereinigen bzw. zugeordneten Artikel prüfen. | nicht relevant |
| 004058 | Zubehörartikel nur im Projekt | AML-Import: Zubehörartikel an Funktion fehlt in der Importdatei. | Zubehör prüfen, ggf. löschen, Import wiederholen. | nicht relevant |
| 004070 | Kein Bus-Ausgang zur Vorgängeradresse | Beckhoff TwinCAT: zum Bus-Eingang fehlt der Bus-Ausgang an der Vorgängeradresse. | Bus-Ausgang an Vorgängeradresse prüfen/ergänzen. | nicht relevant |
| 004071 | Haupt-Bus-Anschluss fehlt oder mehrfach | Bus-Anschlüsse einer Schnittstelle (gleicher Schnittstellenname) haben keinen oder mehrere Haupt-Bus-Anschlüsse. | Genau einen Anschluss je Schnittstelle als Haupt-Bus-Anschluss markieren. | eigenes Skript **Regel: je Bus-Schnittstelle genau ein Haupt-Anschluss (Master), Rest Slaves** |
| 004072 | Bus-Adresse doppelt im physikalischen Netz | Gleiche Bus-Adresse im selben physikalischen Netz; nur mit Konfig.-Projekt. | Adressen unterscheiden oder anderes Netz zuordnen. | eigenes Skript |
| 004073 | Automatischer Funktionstext doppelt in CPU | Gleicher automatischer Funktionstext an SPS-Anschlüssen einer CPU (je Sprache). | Funktionstexte eindeutig machen und übersetzen; leere Texte werden nicht gemeldet. | eigenes Skript |
| 004080 | Fehlendes Konfigurationsprojekt | SPS-Kasten oder Bus-Anschluss ohne Konfigurationsprojekt; nötig für SPS-Export. | Konfigurationsprojekt eintragen. | nicht relevant |
| 004081 | Fehlender Stationsname | Haupt-SPS-Kasten mit Konfig.-Projekt hat keinen Stationsnamen. | Stationsnamen eintragen. | nicht relevant |
| 004082 | Konfigurationsprojekt ohne CPU | Im Konfigurationsprojekt ist kein SPS-Kasten als CPU markiert. | Kontrollkästchen CPU setzen; eine CPU je Konfig.-Projekt genügt, nicht je Station. | nicht relevant |
| 004083 | Eingetragene CPU existiert nicht | CPU: Name am SPS-Kasten verweist auf keine CPU im Projekt (nur Hauptfunktionen). | Namen der vorhandenen CPU eintragen. | eigenes Skript |
| 004084 | MasterSystemID fehlerhaft | Profibus braucht 1–99, Ethernet-basierte Busse ≥ 100; nur Siemens relevant. | MasterSystemID im passenden Bereich setzen. | nicht relevant |
| 004085 | Fehlender Datentyp | Digital-/Analog-E/A ohne Datentyp; Multifunktion je nach Signalart geprüft, „allgemein“/„variabel“ nicht. | Datentyp (z. B. BOOL, INT) eintragen. | eigenes Skript |
| 004086 | SPS-Typ oder Gerätebeschreibung fehlt | SPS-Kasten mit Konfig.-Projekt ohne SPS-Typbezeichnung bzw. GSD-Dateiname; Ausnahme „Integrierte Baugruppe“. | Eines der beiden Felder füllen. | eigenes Skript |
| 004087 | Fehlende Startadresse | Karte mit E/A ohne Startadresse: nur E → Startadresse 1; nur A → 1 oder 2; E+A → beide. | Startadressen gemäß E/A-Bestückung eintragen. | eigenes Skript **Regel: Kanaladresse = Startadresse der Karte + Kanaloffset** |
| 004088 | Kartenname bei Träger/CPU doppelt | Zwei Karten, die Träger oder CPU sind, haben denselben SPS-Kartennamen. | Eindeutige Kartennamen vergeben. | nicht relevant |
| 004089 | Kartenname im Träger doppelt | Zwei Karten desselben Trägers tragen denselben SPS-Kartennamen. | Kartennamen eindeutig machen. | nicht relevant |
| 004090 | SPS-Stationstyp Projekt ≠ Artikel | Stationstyp am CPU-/Kopfstations-Kasten weicht vom Artikelwert ab. | Wert vom Artikel übernehmen oder passenden Artikel wählen. | eigenes Skript |
| 004091 | Fehlende Steckerbezeichnung | SPS- oder Bus-Anschluss ohne Steckerbezeichnung (z. B. X1, X2 der Karte). | Steckerbezeichnung eintragen. | eigenes Skript **Regel: Kanal-Identität = Karten-BMK + Stecker + Anschluss** |
| 004092 | Fehlender Bus-Schnittstellenname | Ethernet-Bus-Anschluss ohne Schnittstellennamen; nur mit Konfig.-Projekt. | Bus-Schnittstelle: Name eintragen (z. B. X1 P1). | nicht relevant |
| 004093 | Verweis auf fehlendes SPS-Untergerät | SPS-Untergerät: Index am Anschluss zeigt auf Untergerät ohne Name/Steckplatz/Startadresse am Kasten. | Untergerätdaten am Kasten ergänzen oder Index korrigieren. | nicht relevant |
| 004094 | CPU-Name doppelt | Gleicher CPU-Name in derselben Station und Konfig.-Projekt. | CPU-Namen ändern. | nicht relevant |
| 004095 | Fehlende Safety-Adresse | Sicherheitsrelevanter SPS-Kasten ohne Safety-Adresse (F-Adresse). | Safety-Adresse eintragen. | eigenes Skript **Regel: F-Karten brauchen F-Adresse** |
| 004096 | Safety-Adresse doppelt | Zwei sicherheitsrelevante Kästen mit gleicher Safety-Adresse. | Eindeutige Safety-Adressen vergeben. | eigenes Skript **Regel: F-Adresse projektweit eindeutig** |
| 004097 | Mehrere E/A je Kanal, Signalart, Untergerät | Mischkarte: mehr als ein E/A-Anschluss gleicher Signalart und gleichen Untergerät-Index im selben Kanal. | Kanal je Signalart und Untergerät nur einmal belegen (E1→Kanal 1 und A1→Kanal 1 zulässig). | eigenes Skript **Regel: bei Mischkarten Kanal eindeutig je Signalart** |
| 004098 | Falsches BMK-Format (Import) | Beim Busdaten-Import Baugruppe ohne BMK oder mit unzulässigen Zeichen. | Sonderzeichen in BMK-Syntaxüberprüfung zulassen oder BMK im Konfigurator ändern. | nicht relevant |
| 004099 | Karte steckt nicht auf Kopfstation | „Steckt auf Kopfstation“ gesetzt, aber Trägerkarte ist nicht als Bus-Koppler/Kopfstation oder CPU definiert. | Trägerkarte als Kopfstation/CPU deklarieren bzw. Träger anlegen. | nicht relevant |
| 004100 | CPU ohne Steckplatz/Träger | Als CPU definierte Karte ohne Steckplatz oder Trägerzuordnung. | Steckplatz bzw. Träger und „steckt auf“ eintragen. | nicht relevant |
| 004101 | Träger nicht per Erweiterungskabel verbunden | Träger derselben Station nicht oder nicht mit Bus-System „Local-Bus: Erweiterung“ verbunden. | Verbinden und beide Bus-Anschlüsse auf Local-Bus: Erweiterung stellen, oder Station ändern. | nicht relevant |
| 004102 | Erweiterungskabel zwischen Stationen | Local-Bus: Erweiterung verbindet Träger verschiedener Stationen. | Richtiges Bus-System wählen oder Stationsnamen vereinheitlichen. | nicht relevant |
| 004103 | Slave/Master: physikalisches Netz verschieden | Slave und Master desselben logischen Netzes haben unterschiedlichen physikalischen Netznamen. | Netznamen vom Master übernehmen (auch per Projektkorrektur). | nicht relevant |
| 004104 | Bus-Anschlüsse des Netzes nicht verbunden | Bus-Anschlüsse desselben physikalischen Netzes sind grafisch nicht verbunden; kann gewollt sein. | Mit Verbindungssymbolen verbinden oder bewusst ignorieren. | qet_continuity |
| 004106 | Adresse außerhalb Kartenbereich | Kanaladresse liegt nicht zwischen Startadresse und Startadresse + Datenlänge der Karte. | Adresse oder Startadresse/Datenlänge anpassen; prüft nur DE/DA/AE/AA/Multifunktion mit CPU-Name und Adressierungsschema. | eigenes Skript **Regel: Kanaladresse im Bereich Startadresse … Startadresse + Datenlänge (Bit)** |
| 004107 | Adressbereiche zweier Karten überlappen | Startadresse + Datenlänge zweier Karten derselben CPU überschneiden sich (inkl. Untergeräte). | Startadressen/Datenlängen entflechten; Einstellung „Getrennter Adressbereich für E und A“ beachten. | eigenes Skript **Regel: Kartenadressbereiche je CPU überschneidungsfrei** |
| 004108 | Unbekannte benutzerdefinierte Eigenschaft (Import) | Importierte Karteneigenschaft fehlt am veralteten Artikel. | Aktuellen Herstellerartikel importieren oder Eigenschaft am Artikel ergänzen. | nicht relevant |
| 004109 | Einpolig/allpolig: BMK-Unterschied im Netz | Parallel gezeichnete ein- und allpolige Netzwerkstruktur hat abweichende BMK an Bus-Kabelanschlüssen. | BMK in beiden Darstellungen angleichen. | eigenes Skript **Regel: Übersichts- und Detaildarstellung eines Busgeräts tragen dasselbe BMK** |
| 004110 | Physikalisches Netz ohne Namen | Verbundene Bus-Anschlüsse bilden ein Netz ohne Namen. | Namen an mindestens einem Bus-Anschluss des Netzes eintragen. | nicht relevant |
| 004111 | Verschiedene Netznamen in einem Netz | Verbundene Bus-Anschlüsse tragen unterschiedliche physikalische Netznamen. | Einheitlichen Namen oder nur an einem Anschluss eintragen. | eigenes Skript |
| 004200 | Siemens: Kartenzuordnung fehlerhaft | Siemens-Regelprüfung: Karte falsch zu Träger/Steckplatz/CPU zugeordnet; Details im Meldungstext. | Gemeldete Strukturdaten laut Vorgabe korrigieren. | nicht relevant |
| 004201 | Siemens: Stationstyp fehlerhaft | Karte passt nicht zum gewählten Siemens-Stationstyp. | Stationstyp-Eigenschaft laut Vorgabe korrigieren. | nicht relevant |
| 004202 | Siemens: Bus-Anschluss fehlerhaft | Steckerbezeichnung, Bus-System oder Schnittstellenname am Bus-Anschluss nicht regelkonform. | Gemeldete Eigenschaften korrigieren. | nicht relevant |
| 004203 | Siemens: kein gültiger Baugruppenträger | ET200-Karte als Träger ungültig, da Bus-Koppler/Kopfstation bzw. CPU nicht passend gesetzt. | Kopfstation/CPU aktivieren und Baugruppenträger-Feld füllen. | nicht relevant |

## 005 Verbindungen / Potenziale (78 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_005nnn.htm – Nummern: 005001, 005010, 005012, 005013, 005014, 005015, 005016, 005017, 005018, 005019, 005020, 005021, 005022, 005023, 005024, 005025, 005026, 005027, 005028, 005031, 005033, 005034, 005035, 005036, 005037, 005039, 005040, 005042, 005043, 005044, 005045, 005046, 005047, 005048, 005049, 005050, 005051, 005052, 005053, 005054, 005055, 005056, 005057, 005058, 005060, 005061, 005062, 005063, 005064, 005065, 005066, 005067, 005068, 005069, 005070, 005071, 005072, 005073, 005074, 005075, 005076, 005077, 005078, 005079, 005080, 005081, 005082, 005083, 005084, 005085, 005086, 005087, 005088, 005089, 005090, 005091, 005092, 005093

Begriffe: Potenzial = alles galvanisch Verbundene (über Leitungen, Verbindungssymbole und Funktionen mit „Potenzial weiterleiten“, z. B. Klemme, Sicherungsklemme); Signal = Teilstück eines Potenzials zwischen signaltrennenden Funktionen; Verbindung = Leitung zwischen genau zwei Anschlüssen (Quelle/Ziel); Netz = vom Netzdefinitionspunkt bestimmte Verbindungskette. In QET entspricht das Potenzial der Leitergruppe, die über Leiter/Abzweige und Folio-Reports zusammenhängt; die Eigenschaften eines Verbindungsdefinitionspunkts (Nummer, Farbe, Querschnitt, Kabel) stehen in QET direkt am Leiter.

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 005001 | Potenzialanschluss/-definitionspunkt ohne Namen | Potenzialeinspeisung oder -definitionspunkt ohne Potenzialnamen gesetzt. | Potenzialnamen (z. B. L1, 24V, 0V) eintragen. | `eigenes Skript` **Regel: jedes Einspeise-/Potenzial-Element (Folio-Report-Quelle, Potenzialanschluss) braucht einen Namen** |
| 005010 | Verbindung ohne Potenzial | Verbindung gehört zu keinem definierten Potenzial (keine Einspeisung erreichbar). | Potenzialdefinitionspunkt mit Namen auf die Verbindung setzen oder an Einspeisung anschließen. | `eigenes Skript` |
| 005012 | Mehrere Verbindungsdefinitionspunkte auf Verbindung | Eine Verbindung (zwischen zwei Anschlüssen) trägt mehr als einen Definitionspunkt. | Nur einen Verbindungsdefinitionspunkt je Verbindung behalten. | `Sichtprüfung` **Regel: je Leiter genau eine Beschriftung/Leiternummer** |
| 005013 | Unterschiedliche Anschlussart der Ziele | Zielanschlüsse einer Verbindung haben verschiedene Anschlussarten (z. B. Draht gegen LWL). | Einheitliche Anschlussart setzen; „Undefiniert“ passt zu allen. | `nicht relevant` |
| 005014 | Anschlussart Verbindung ≠ Anschlusspunkt | Funktionsdefinition am Verbindungsdefinitionspunkt passt nicht zur einheitlichen Anschlussart der Ziele. Nicht bei Brücken aller Art, Direktverbindung oder „Undefiniert“. | Gleiche Anschlussart an Definitionspunkt und Zielen. | `nicht relevant` |
| 005015 | Unterschiedliche Potenzialtypen der Ziele | Ziele einer Verbindung haben unterschiedlichen Potenzialtyp (z. B. L gegen PE/PEN). | Potenzialtypen angleichen; „Undefiniert“ ist neutral. | `qet_continuity` **Regel: PE-Anschluss nie mit L/N-Anschluss verbinden** |
| 005016 | Potenzialtyp Verbindung ≠ Anschlusspunkt | Potenzialdefinitionspunkt hat anderen Potenzialtyp als die (einheitlichen) Ziele. | Potenzialtyp an Definitionspunkt und Zielen angleichen. | `qet_continuity` |
| 005017 | Ziel nicht vorhanden | Nicht platzierte Verbindung ohne Quell- oder Zielfunktion (Datenleiche). | Quelle/Ziel nachtragen oder per Komprimieren „Nicht-platzierte Verbindung entfernen“. | `nicht relevant` |
| 005018 | Widersprüchliche Verbindungsdefinitionspunkte | Mehrere Definitionspunkte einer Verbindung mit verschiedenen Farben/Querschnitten. | Eigenschaften aller Punkte gleichsetzen bzw. überzählige löschen. | `qet_continuity` |
| 005019 | Widerspruch Signal und Potenzial | Signaldefinition am Potenzialdefinitionspunkt weicht im Typ, Wert oder Frequenz von gleichnamiger Potenzialdefinition ab. | Typ, Wert, Frequenz angleichen. | `eigenes Skript` |
| 005020 | Widerspruch gleicher Potenzialdefinitionen | Gleichnamige Potenzialdefinitionen einer Verbindung mit verschiedenem Typ oder Wert. | Alle gleichnamigen Definitionen gleich parametrieren. | `eigenes Skript` |
| 005021 | Potenzialdefinitionspunkt mit unbekanntem Potenzial | Punkt mit Potenzial- und Signalnamen, dessen Potenzial nirgends im Plan definiert ist. | Existierendes Potenzial wählen oder Potenzialanschluss mit diesem Namen anlegen. | `eigenes Skript` **Regel: jeder Potenzialname braucht eine Quelle (Einspeisung)** |
| 005022 | Definitionspunkt auf mehreren Verbindungen | Punkt sitzt an T-Stück-Verzweigung oder auf Kreuzung zweier Leitungen. | Punkt auf eindeutiges Leitungsstück verschieben, Abzweige eigene Punkte. | `qet_layout_check` + `Sichtprüfung` |
| 005023 | Definitionspunkt nicht auf Verbindung | Verbindungsdefinitionspunkt liegt neben jeder Autoconnect-Linie. | Punkt auf die Leitung schieben. | `Sichtprüfung` |
| 005024 | Definitionspunkt ohne Eigenschaften | Verbindungsdefinitionspunkt ohne jeden Eintrag. | Mindestens sichtbares BMK (bzw. Nummer/Farbe) eintragen oder löschen. | `qet_check` |
| 005025 | Zu viele Verbindungen am Anschluss | Anschluss hat mehr Ziele als in der Anschlusslogik („Anzahl Ziele“) erlaubt. | Zielanzahl in der Logik erhöhen oder überzählige Anschlüsse entfernen. | `qet_check` **Regel: max. Ziele je Anschluss (Standard 2, Klemme 1 je Seite) einhalten** |
| 005026 | Nicht verbundener Anschluss | Anschluss eines Schaltzeichens hat keine Verbindung. | Anschluss verdrahten (oder bewusst offen lassen, Prüfung abschalten). | `qet_continuity` |
| 005027 | Ringschluss | Verbindung kehrt zum selben Anschluss zurück, ohne ein anderes Ziel zu erreichen (z. B. T-Stück zurück an Ausgangsanschluss). | Schleife auflösen oder eine Funktion als zweites Ziel einfügen. | `eigenes Skript` **Regel: kein Leiterzyklus ohne zweites Ziel** |
| 005028 | Mehrere Potenzialdefinitionen auf Potenzial | Auf einer gezeichneten Verbindung liegen mehrere Potenzialdefinitionspunkte/-anschlüsse. | Auf genau eine gültige Potenzialdefinition reduzieren. | `eigenes Skript` **Regel: je Potenzial genau eine Einspeisung** |
| 005031 | Definitionspunkt erhält Verbindungsbezeichnung | Info: zentrierter Punkt mit „Keine Verbindungsbezeichnung“ wird beim Nummerieren auf „Alle Daten“ umgestellt. | – (Info, mit neuem Prüflauf löschbar) | `nicht relevant` |
| 005033 | Bezeichnungslauf Formatgruppe erste/letzte Nummer | Info nach Verbindungsnummerierung: erste und letzte vergebene Bezeichnung je Formatgruppe. | – (Info) | `nicht relevant` |
| 005034 | Bezeichneter Definitionspunkt auf Mehrfachverbindung | Info: beim Nummerieren liegt ein Punkt auf einer Mehrfachverbindung. | – (Info) | `Sichtprüfung` |
| 005035 | Definitionspunkt auf Mehrfachverbindung neu ausgerichtet | Info: Punkt auf Mehrfachverbindung beim Ausrichten/Formatieren geändert. | – (Info) | `nicht relevant` |
| 005036 | Mindeststellenzahl Format überschritten | Vergebene Verbindungsnummer hat mehr Stellen als Formatelement vorsieht (Einstellung „Mindestanzahl Stellen“). | Mindeststellen im Nummerierungsformat erhöhen. | `nicht relevant` |
| 005037 | Widerspruch Eigensicherheit | Ein Ziel ist eigensicher („Eigensicher“ + „Eigensicherheit möglich“), das andere nicht. | Alle Ziele einheitlich eigensicher oder nicht. | `nicht relevant` |
| 005039 | Unterschiedliche Darstellungsart der Ziele | Ziele einer Verbindung: eines allpolig, eines einpolig dargestellt. | Gleiche Darstellungsart für beide Ziele. | `nicht relevant` |
| 005040 | Stranganschluss nicht paarweise | Stranganschluss nur einmal platziert. | Zweiten Stranganschluss gleicher Bezeichnung im Strang setzen. | `eigenes Skript` **Regel: wie Folio-Report – gleiche Bezeichnung immer genau paarweise** |
| 005042 | Farbe/Nummer nicht in Verbindungsfarben | Farbcode der Verbindung fehlt in Projekteinstellung „Verbindungsfarben“. | Farbe aus Tabelle wählen oder Tabelle erweitern. | `eigenes Skript` **Regel: Leiterfarben nur aus fester Liste (IEC 60757: BK, BN, RD, OG, YE, GN, BU, VT, GY, WH, PK, GNYE …)** |
| 005043 | Querschnitt nicht in Projektvorgabe | Querschnitt/Durchmesser fehlt in Projekteinstellung „Querschnitte / Durchmesser“. | Wert aus Vorgabeliste wählen oder Liste erweitern. | `eigenes Skript` **Regel: Querschnitte nur aus Normreihe (0,5/0,75/1/1,5/2,5/4/6/10/16 … mm²)** |
| 005044 | Gleiche Verbindungsbezeichnung, verschiedene Potenziale | Gleiche Leiternummer auf getrennten Potenzialen, z. B. Winkel gelöscht oder Klemme ohne „Potenzial weiterleiten nach“. | Fehlendes Verbindungssymbol ergänzen bzw. Potenzialweiterleitung in Anschlusslogik eintragen; sonst umnummerieren. | `qet_continuity` **Regel: gleiche Leiternummer nur innerhalb eines Potenzials** |
| 005045 | Gleiche Verbindungsbezeichnung, verschiedene Signale | Gleiche Nummer beiderseits einer signaltrennenden Funktion (Kontrollkästchen „Signaltrennend“, z. B. Sicherungsklemme). | „Signaltrennend“ prüfen/deaktivieren oder Teilstücke verschieden nummerieren. | `qet_continuity` **Regel: hinter signaltrennender Funktion (Sicherung, Trennklemme) neue Nummer** |
| 005046 | Unvollständiges Netz | Nicht alle Anschlüsse eines Netzes in der Verbindungskette, Kette zerfallen, Verbindung gelöscht oder kopiert. | Verbindungskette am Netzdefinitionspunkt vervollständigen; bei Teilklemmen mind. einen Anschluss im Netz verbinden. | `qet_continuity` |
| 005047 | Netzdefinitionspunkt mit Lücke | Netzteilnehmer gelöscht; Kette hat Lücke (Warndreieck in Netzverbindungen). | Teilnehmer wieder einfügen und Kette schließen bzw. Lückeneintrag löschen. | `nicht relevant` |
| 005048 | Netz mit „Ziele festlegen“ | Verbindungssymbol im Netz hat feste Zielfestlegung statt Punktdarstellung. | „Ziele festlegen“ deaktivieren, Symbol als Punkt. | `nicht relevant` |
| 005049 | Mehrere Netzdefinitionspunkte | Ein Netz hat mehr als einen Netzdefinitionspunkt. | Überzählige Netzdefinitionspunkte löschen. | `nicht relevant` |
| 005050 | Doppelte Netz-ID | Kopierter Netzteilnehmer übernimmt die ID der netzbasierten Verbindung; noch nicht in Kette. | Neuen Anschluss in die Verbindungskette einordnen. | `nicht relevant` |
| 005051 | Verbindungssymbol mit offenem Anschluss | Winkel, T-Stück, Brücke, Kreuzung oder Diagonale mit offenem Ende. | Offenes Ende verbinden oder Symbol löschen. | `qet_continuity` **Regel: keine Abzweige/Winkel mit losem Ende** |
| 005052 | Offener Anschluss an Steckerkontakt | Steckerkontakt (auch mit mehreren Symbolanschlüssen) nicht vollständig verbunden. | Alle Symbolanschlüsse verbinden oder Kontakt löschen. | `qet_continuity` |
| 005053 | Offener Anschluss an Klemme | Klemmenanschluss offen, auch wenn nur ein Anschluss der Klemme verbunden ist. | Beide Seiten verdrahten oder Klemme löschen. | `qet_continuity` **Regel: Klemme beidseitig angeschlossen (innen und außen)** |
| 005054 | Gleicher Netzname, verschiedene Netze | Getrennte Netze mit gleichem Netznamen oder mit verschiedenen Potenzialeinspeisungen. | Netze verbinden und einen Definitionspunkt löschen bzw. Potenzialnamen angleichen. | `eigenes Skript` |
| 005055 | Mehrere Signaldefinitionen auf Signal | Signal trägt mehrere Signaldefinitionen/Netzpunkte mit verschiedenen Signalnamen. | Auf eine Definition reduzieren oder gleichen Signalnamen eintragen. | `eigenes Skript` |
| 005056 | Widerspruch Netz und Potenzial/Signal | Netzdefinitionspunkt weicht in Typ, Wert oder Frequenz von gleichnamigem Potenzial/Signal ab. | Eigenschaften angleichen. | `nicht relevant` |
| 005057 | Netzdefinitionspunkt mit unbekanntem Potenzial | Netzpunkt nennt Potenzial, das nirgends definiert ist. | Existierendes Potenzial wählen oder Potenzialanschluss anlegen. | `eigenes Skript` |
| 005058 | Netzdefinitionspunkt mit unbekanntem Signal | Netzpunkt nennt Signalnamen, das nirgends definiert ist. | Existierendes Signal wählen oder Potenzialanschluss mit Potenzial- und Signalnamen anlegen. | `nicht relevant` |
| 005060 | Anschluss ein- und allpolig allpolig verbunden | Über „Geräte verschalten“ verschalteter Anschluss hat schon eine allpolige Verbindung. | Eine der beiden Verbindungen löschen. | `nicht relevant` |
| 005061 | Mehrere allpolige Definitionen an einpoliger Verbindung | Mehrere Definitionspunkte einer einpoligen Leitung tragen „Geräte verschalten“-Einträge. | Allpolige Verschaltung nur an einem Punkt halten. | `nicht relevant` |
| 005062 | Allpolige Definition passt nicht zu Quelle/Ziel | Einpoliger Punkt verschoben/kopiert oder Ziel-Anschlüsse geändert/gelöscht. | Funktionen/Anschlüsse wiederherstellen oder Definition löschen. | `nicht relevant` |
| 005063 | Anschluss in allpoliger Definition fehlt | Nur ein Teil der Anschlüsse in „Geräte verschalten“ hat Ziele. | Alle oder keinen Anschluss verschalten. | `nicht relevant` |
| 005064 | Ungültige Verbindung, Anschluss fehlt | Quelle/Ziel einer mehrfach dargestellten Verbindung fehlt in einer Darstellungsart (z. B. 3D). | Fehlende Quelle/Ziel platzieren. | `nicht relevant` |
| 005065 | Widersprüchliche Daten über Darstellungsarten | Dieselbe Verbindung in allpolig und einpolig mit verschiedenen Werten (z. B. Farbe RD/BU). | Wert in allen Darstellungen angleichen. | `qet_continuity` **Regel: Übersichts-/Detaildarstellung dürfen sich in Farbe/Querschnitt nicht widersprechen** |
| 005066 | Allpolig ohne 3D-Gegenstück | Schaltplanverbindung im Bauraum nicht verlegt. | Im Bauraum verlegen. | `nicht relevant` |
| 005067 | 3D-Verbindung ohne allpoliges Gegenstück | Bauraumverbindung ohne Schaltplanverbindung (Ziel gelöscht/verschoben). | Quelle/Ziel im Schaltplan ergänzen. | `nicht relevant` |
| 005068 | 3D-Anschluss nicht im Anschlussbild | Bauraumanschluss fehlt im Anschlussbild oder hat Position 0/0/0. | Anschlussbild korrigieren. | `nicht relevant` |
| 005069 | Gleiche Bezeichnung auf verschiedenen Netzen | Verbindungsbezeichnung schon auf anderem Netz vergeben. | Umbenennen oder neu nummerieren. | `qet_continuity` |
| 005070 | Verschiedene Bezeichnungen in einem Potenzial | Innerhalb eines Potenzials Verbindungen mit unterschiedlichen Nummern. | Einheitliche Bezeichnung im Potenzial (bei potenzialorientierter Nummerierung). | `qet_continuity` **Regel: potenzialorientiert = eine Nummer je Potenzial** |
| 005071 | Verschiedene Bezeichnungen in einem Signal | Innerhalb eines Signals unterschiedliche Verbindungsnummern. | Einheitliche Bezeichnung je Signal. | `qet_continuity` |
| 005072 | Verschiedene Bezeichnungen in einem Netz | Innerhalb eines Netzes unterschiedliche Verbindungsnummern. | Einheitliche Bezeichnung je Netz. | `qet_continuity` |
| 005073 | Anschlussquerschnitt der Ziele verschieden | Indizierte Eigenschaft Anschlussquerschnitt der Ziele einer Verbindung unterschiedlich. | Gleichen Anschlussquerschnitt an Zielen eintragen. | `nicht relevant` |
| 005074 | Anschlussquerschnitt ≠ Verbindungsquerschnitt | Anschlussquerschnitt einer Funktion weicht vom Querschnitt der angeschlossenen Verbindung ab. | Querschnitt an Anschluss und Verbindung angleichen. | `eigenes Skript` |
| 005075 | Unterschiedliche Stoffe in Rohrleitung | Fluid: widersprüchliche Stoffe auf einer Rohrleitung. | – | `nicht relevant` |
| 005076 | Unterschiedliche Eigenschaft in Rohrleitung | Fluid: gleiche Rohrleitung, abweichende Verbindungseigenschaft. | – | `nicht relevant` |
| 005077 | Rohrleitung ohne Stoff | Fluid: Rohrleitungsdefinition ohne Stoff. | – | `nicht relevant` |
| 005078 | Stoff ohne Rohrleitungsdefinition | Fluid: Stoff an Verbindung ohne Rohrleitungsdefinition. | – | `nicht relevant` |
| 005079 | Unterschiedliche Zustände in Rohrleitung | Fluid: widersprüchlicher Zustand des Stoffs. | – | `nicht relevant` |
| 005080 | Topologie-Verbindung ohne Schaltplan-Gegenstück | Topologie-Verlegeverbindung ohne allpolige Verbindung im Schaltplan. | – | `nicht relevant` |
| 005081 | Unrealistische Durchverbindung am Leitungsverteiler | Durchgehende Verbindung am Verteiler bei Schlauch, LWL oder Brücke unzulässig. | Verteilereinstellung oder Funktionsdefinition ändern. | `nicht relevant` |
| 005082 | Kabelverbindung ohne Farbe/Nummer | Ader (Kabelverbindung) ohne Aderfarbe oder -nummer. | Aderfarbe/-nummer am Definitionspunkt eintragen. | `eigenes Skript` **Regel: jede Kabelader braucht Farbe oder Nummer** |
| 005083 | Paketverbindung ohne Farbe/Nummer | Verbindung eines Pakets (z. B. verdrilltes Paar) ohne Farbe/Nummer. | Farbe/Nummer eintragen. | `eigenes Skript` |
| 005084 | Stoff widerspricht Rohrleitungsdefinition | Fluid: Stoff an Verbindung ≠ Rohrleitungsdefinition. | – | `nicht relevant` |
| 005085 | Verbindung zum Löschen markiert | Aus Fremdprogramm importierte, dort gelöschte Verbindung („Zum Löschen markiert“). | Markierung entfernen oder Verbindung löschen. | `nicht relevant` |
| 005086 | Unterschiedliches Anschlussmaß | Fluid/Schlauch: Anschlussmaß Quelle/Ziel ≠ Anschlussmaß der Funktion. | – | `nicht relevant` |
| 005087 | Direktverbindung: unterschiedliches Anschlussmaß | Fluid: Anschlussmaße beider Funktionen an Direktverbindung verschieden. | – | `nicht relevant` |
| 005088 | Einzelverbindung mit BMK | Verbindung mit Zugehörigkeit „Einzelverbindung“ trägt trotzdem ein BMK (Kabel-/Paket-BMK). | BMK löschen oder Zugehörigkeit (Kabel/Paket) setzen. | `eigenes Skript` **Regel: Leiter mit Kabel-BMK gehört zu einem Kabel, sonst kein BMK** |
| 005089 | Definitionspunkt auf Strang | Verbindungsdefinitionspunkt oder Kabeldefinitionslinie auf einem Strang (Leitungsbündel). | Auf die ein-/austretenden Einzelleitungen setzen. | `Sichtprüfung` |
| 005090 | Rohrklasse widerspricht Rohrleitungsdefinition | Fluid: Rohrklasse Verbindung ≠ Rohrleitungsdefinition. | – | `nicht relevant` |
| 005091 | Rohrklasse Verbindung ≠ Funktion | Fluid: Rohrklasse an Verbindung ≠ Quelle/Ziel-Funktion. | – | `nicht relevant` |
| 005092 | Artikel nicht in Rohrklasse | Fluid: Artikel der Verbindung fehlt in Rohrklasse. | – | `nicht relevant` |
| 005093 | Querschnitt außerhalb Klemmenbereich | Leiterquerschnitt liegt außerhalb „Klemmen: Querschnitt von/bis“ (bzw. AWG) des Klemmenartikels. | Querschnitt oder Klemmenartikel passend wählen. | `eigenes Skript` **Regel: Leiterquerschnitt im zulässigen Klemmbereich des Artikels** |

## 007 Geräte (33 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_007nnn.htm – Nummern: 007001, 007002, 007003, 007004, 007005, 007006, 007007, 007008, 007009, 007010, 007011, 007012, 007013, 007014, 007015, 007016, 007017, 007018, 007019, 007020, 007021, 007022, 007023, 007024, 007025, 007026, 007027, 007028, 007029, 007030, 007031, 007032, 007033

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 007001 | Mehr Funktionen als Artikel bietet | Zum Gerät eine Funktion mit gleichem BMK hinzugefügt, die der Artikel nicht vorsieht; auch Sicherheitsfunktion ohne passende Schablone. | Artikel mit passenden Funktionsschablonen wählen oder Artikel erweitern. | eigenes Skript **Regel** |
| 007002 | Weniger Funktionen als Artikel bietet | Gerät platziert, aber nicht alle Funktionen des Artikels verwendet. | Passenden Artikel wählen; Meldung abhängig von „Anzahl freie Funktionen“ (0 = jede freie Schablone ist Fehler). | eigenes Skript |
| 007003 | Maximale Artikelanzahl erreicht | Beim Summieren der Kabellängen Grenze von 1000 Projektartikeln erreicht. | Überflüssige Projektartikel löschen. | nicht relevant |
| 007004 | Anschlussbezeichnung doppelt | Zwei Funktionen eines Betriebsmittels (z. B. zwei Schließer 13/14) oder eine Funktion mit gleichen Anschlussbezeichnungen. | Eindeutige Anschlussbezeichnungen je Betriebsmittel; bei Bus zählen Stecker- und Schnittstellenname mit. | eigenes Skript **Regel** |
| 007005 | Betriebsmittel ohne Hauptfunktion | Keine Funktion des Betriebsmittels ist Hauptfunktion, auch bei einzeln platzierter Funktion. | Genau eine Funktion (z. B. Spule) als Hauptfunktion kennzeichnen. | qet_check **Regel** |
| 007006 | Funktion ohne Anschlussbezeichnung | Platzierte Funktion hat mindestens einen unbezeichneten Anschluss; Anschlussarten Intern, Direktanschluss, Stegbrücke ausgenommen. | Fehlende Anschlussbezeichnungen ergänzen. | eigenes Skript **Regel** |
| 007007 | Montageort (beschreibend) fehlt | Funktion ohne beschreibenden Montageort. | Montageort eintragen. | nicht relevant |
| 007008 | Artikel aus Geräteliste zu oft verwendet | Artikel häufiger verbaut als „Erlaubte Stückzahl“ in der Geräteliste. | Überzählige entfernen oder erlaubte Stückzahl erhöhen. | nicht relevant |
| 007009 | Spule ohne passenden Kontakt | Spule hat Kontakt-/Spulenindex, Kontakte gleichen BMK nicht – oder Spule ohne Index und ganz ohne Kontakt. | Gleichen Index an Spule und Kontakt oder Index überall leer; mindestens einen Kontakt gleichen BMK. | eigenes Skript **Regel** |
| 007010 | Widersprüchliche Daten querverwiesener Funktionen | Haupt- und Nebenfunktion haben unterschiedliche Werte bei einer Eigenschaft. | Eigenschaft an Haupt- und Nebenfunktion angleichen. | eigenes Skript **Regel** |
| 007011 | Einpolige nicht eindeutig zur allpoligen | In einpoliger Darstellung fehlt eine Angabe (z. B. Klemmenbezeichnung), die allpolig gefüllt ist. | Fehlende Daten aus allpoliger Darstellung übernehmen. | eigenes Skript |
| 007012 | Artikel/Variante nicht eingelagert | Artikelnummer oder Variante nachträglich manuell geändert, nicht im Projekt vorhanden. | Artikel/Variante anlegen und Gerät neu platzieren; Prüfung nur projektweit korrekt. | qet_check |
| 007013 | Variante nicht eingelagert | Manuell gewählte Artikelvariante existiert nicht in den Stammdaten. | Variante anlegen und verwenden. | nicht relevant |
| 007014 | Funktionstext je Darstellungsart unterschiedlich | Automatischer Funktionstext weicht zwischen allpolig, einpolig, Übersicht ab. | Funktionstext übergreifend vereinheitlichen. | eigenes Skript |
| 007015 | Artikelmakro ≠ Makrokasten | Artikel an erster Position der Hauptfunktion hat anderes Makro als der Makrokasten mit Artikeldatenzuweisung. | Artikel mit gleichem Makronamen wie Makrokasten zuweisen. | nicht relevant |
| 007016 | Doppelte Funktionsschablonen | Artikel/Gerät hat mehrere Schablonen mit gleichen Anschlussbezeichnungen; Schablonen ohne Bezeichnung zählen nicht. | Anschlussbezeichnungen der Schablonen im Artikel eindeutig machen. | nicht relevant |
| 007017 | Hauptfunktion/Hauptklemme ohne Artikel | An Hauptfunktion oder Hauptklemme ist keine Artikelnummer hinterlegt. | Artikel an Hauptfunktion bzw. Hauptklemme eintragen. | qet_check **Regel** |
| 007018 | Erforderlicher Zubehörartikel fehlt | Artikel verlangt Zubehör als „erforderlich“, das am Betriebsmittel nicht eingetragen ist. | Zubehörartikelnummer zusätzlich eintragen. | nicht relevant |
| 007019 | Überbelegung geschützter Hauptfunktion | An geschützte Funktion/Hauptklemme Funktion ergänzt oder Eigenschaft geändert, die der Artikel nicht vorsieht. | Passenden Artikel wählen, Änderung rückgängig machen oder Artikel erweitern. | nicht relevant |
| 007020 | Geschützte Daten ≠ Artikeldaten | Eigenschaft (z. B. Technische Kenngrößen) einer geschützten Hauptfunktion nachträglich geändert; Teilklemmen werden zusammengefasst. | Änderung zurücknehmen oder anderen Artikel wählen. | nicht relevant |
| 007021 | Geschützte und ungeschützte Funktionen gemischt | Ein Betriebsmittel hat geschützte und ungeschützte Funktionen. | „Geschützte Funktion“ an allen Funktionen einheitlich. | nicht relevant |
| 007022 | Symbol hat mehr Anschlüsse als Definition | Altprojekt: Symbol und Funktionsdefinition mit unterschiedlicher Anschlusszahl. | Symbol oder Funktionsdefinition anpassen. | Sichtprüfung |
| 007023 | Hauptfunktion ≠ Artikeldaten | Aus Artikel übernommene Eigenschaft manuell geändert; Prüfart „Fehler verhindern“ möglich. | Artikeldaten neu zuweisen und überschreiben. | eigenes Skript |
| 007024 | Auslaufartikel verwendet | Gewählter Artikel ist als Auslaufartikel gekennzeichnet. | Aktuellen Nachfolgeartikel wählen. | nicht relevant |
| 007025 | BMK-ID mehrfach vergeben | Funktion eines Modulmakros hat bereits vergebene BMK-ID. | Eindeutige BMK-ID eintragen. | nicht relevant |
| 007026 | Logikmodell weicht von Funktionsdefinition ab | Anschlusslogik (Anschlussart, Anzahl Ziele, Symbolanschluss) manuell abweichend von der Definition. | Abweichung bewusst lassen oder auf Standard zurücksetzen. | nicht relevant |
| 007027 | Ungültige Anschlussverweise im Logikmodell | „Potenzial weiterleiten nach“, „Verbraucher/Quelle nach“ oder „Zielverfolgung (SPS) nach“ verweist auf nicht existierende Anschlussbezeichnung. | Verweis auf existierenden Anschluss gleichen BMK korrigieren. | eigenes Skript **Regel** |
| 007028 | BMK-ID fehlt an Nebenfunktion | Nebenfunktion eines Modulmakros ohne BMK-ID, Hauptfunktion hat eine. | BMK-ID der Hauptfunktion übernehmen. | nicht relevant |
| 007029 | BMK-ID Neben- ≠ Hauptfunktion | BMK-ID der Nebenfunktion weicht von Hauptfunktion ab. | BMK-ID angleichen. | nicht relevant |
| 007030 | BMK-ID ≠ Schablone am Modulartikel | BMK-ID der Funktion weicht von der an der Funktionsschablone des Modulartikels ab. | BMK-ID im Artikel angleichen, Schablonen summieren. | nicht relevant |
| 007031 | Artikel nicht in Rohrklasse | RI-Fließbild: Artikel der Funktion gehört nicht zur gewählten Rohrklasse. | Artikel zur Rohrklasse ergänzen oder passende Rohrklasse wählen. | nicht relevant |
| 007032 | Übergeordnete Funktion mit BMK-ID fehlt | Makro: geschachteltes BMK (z. B. -A1-B1) mit BMK-ID, aber -A1 fehlt oder ohne BMK-ID. | BMK-ID an übergeordneter Funktion oder Gerätekasten mit äußerem BMK. | nicht relevant |
| 007033 | Manueller Funktionstext trotz Blockformat | Manueller Funktionstext überdeckt die Blockeigenschaft für automatischen Funktionstext. | Manuellen Text löschen oder Blockeigenschaft entfernen. | nicht relevant |

## 008 Fremdsprachen / Übersetzung (5 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_008nnn.htm – Nummern: 008001, 008002, 008003, 008004, 008005

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 008001 | Text nicht in Sprache übersetzbar | Wörterbuch hat für die Zielsprache keine passenden Stichwörter | Wörterbuchsprache einbinden, Stichwort samt Übersetzung anlegen, neu übersetzen | nicht relevant |
| 008002 | Textfeld ist leer | Übersetzung eines Textfelds ohne Inhalt versucht | Text eintragen, dann übersetzen | nicht relevant |
| 008003 | Wörterbuchsprache fehlt im Wörterbuch | Die verwendete Sprache wurde aus dem Wörterbuch entfernt | Sprache im Wörterbuch wieder hinzufügen | nicht relevant |
| 008004 | Text nicht übersetzbar | Stichwörter für den Text fehlen im Wörterbuch | Stichwörter ergänzen; projektbezogene Segmentierungseinstellung (ganzer Eintrag / Satz / Wort) beachten | nicht relevant |
| 008005 | Projekttext nicht übersetzt | Ein Projekttext (Text, Pfad-Funktionstext, Funktionseigenschaft) fehlt in mindestens einer Übersetzungssprache | Feld übersetzen; fehlende Übersetzungen im Wörterbuch ergänzen. Kommt nicht bei „Nicht automatisch übersetzen“, als nicht zu übersetzend gekennzeichnetem Text oder Wörtern auf der Ausschlussliste | Sichtprüfung **Regel**: mehrsprachige Texte (Funktionstexte, Pfadtexte) im Nachbau nur in den Sprachen des Originals führen; Original-PDF zeigt, welche Sprachen gefordert sind |

## 010 Querverweise (2 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_010nnn.htm – Nummern: 010001, 010002

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 010001 | Platzierung außerhalb des Normblatts | Haupt- oder Nebenfunktion liegt außerhalb des Auswertungsbereichs des Normblatts; Querverweis/Spaltenadresse nicht bildbar. | Funktion in den Rahmen des Blatts verschieben. | eigenes Skript **Regel** |
| 010002 | Mehrere Hauptplatzierungen Strukturkasten | Mehrere Strukturkästen gleichen BMK haben „Hauptplatzierung“. | Nur einen Strukturkasten je BMK als Hauptplatzierung. | Sichtprüfung |

## 011 Abbruchstellen (8 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_011nnn.htm – Nummern: 011001, 011002, 011003, 011004, 011005, 011006, 011007, 011008

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 011001 | Abbruchstelle ohne BMK | Am vollständigen BMK der Abbruchstelle fehlen Strukturkennzeichen; nur Vorzeichen sichtbar. | Strukturkennzeichen und Kennzeichen (Kennbuchstabe, Zähler) vollständig eintragen. | qet_check **Regel** |
| 011002 | Abbruchstelle ohne Gegenstück | Keine zweite Abbruchstelle gleichen BMK; oft weil BMK einmal als sichtbares BMK, einmal Kennbuchstabe+Zähler zusammen eingegeben. | BMK identisch und gleich strukturiert eingeben (Kennbuchstabe und Zähler getrennt). | qet_check **Regel** |
| 011003 | Abbruchstelle ohne Ziel | Abbruchstelle ist mit keinem Schaltzeichen verbunden. | Mindestens einen Anschluss per Leiter anschließen. | qet_continuity **Regel** |
| 011004 | Abbruchstellen auf derselben Seite | Zwei querverwiesene Abbruchstellen liegen auf demselben Blatt. | Gegenstück auf ein anderes Blatt legen (sonst direkt verbinden). | eigenes Skript **Regel** |
| 011005 | Pfeilspitzen passen nicht zusammen | Pfeilspitzen zweier querverwiesener Abbruchstellen zeigen aufeinander. | Eine Abbruchstelle um 180° drehen: Spitze zeigt auf Stumpf des Gegenstücks. | eigenes Skript **Regel** |
| 011006 | Ringschluss für Abbruchstelle | Pfeilspitze des Gegenstücks ist mit dem Stumpf der ersten Abbruchstelle verbunden. | Verbindung auflösen, kein Rückweg zwischen Paar. | eigenes Skript **Regel** |
| 011007 | Verzweigung vor Sternquelle | Vor dem Pfeilstumpf einer Sternquelle liegt ein T-Stück, Stumpf führt zu zwei Zielen. | Verzweigung entfernen oder drehen; Stumpf der Sternquelle genau ein Ziel. | eigenes Skript **Regel** |
| 011008 | Mehrere Sternquellen gleichen BMK | Mehrere Abbruchstellen gleichen BMK sind als Sternquelle markiert. | Nur eine Sternquelle je BMK (übrige deaktivieren oder umbenennen). | eigenes Skript **Regel** |

## 012 2D-Schaltschrankaufbau (9 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_012nnn.htm – Nummern: 012001, 012002, 012003, 012004, 012005, 012008, 012009, 012010, 012011

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 012001 | Artikelplatzierung ohne Artikel | Platzierung auf der Montageplatte hat keine Artikelnummer | Artikelnummer eintragen | nicht relevant |
| 012002 | Artikelplatzierung überlappt Sperrfläche | Artikel ragt in eine Sperrfläche der Montageplatte | Artikel aus der Sperrfläche schieben | nicht relevant |
| 012003 | Artikelplatzierung teilweise außerhalb Montageplatte | Artikel liegt nur teilweise auf der Platte | Vollständig auf die Platte schieben | nicht relevant |
| 012004 | Abmessungen weichen vom Artikel ab | X-/Y-Ausdehnung der Platzierung ≠ Breite/Höhe des Artikels (nachträglich geändert) | Maße angleichen | nicht relevant |
| 012005 | Montageort (beschreibend) Platte ≠ Hauptfunktion | Feld Montageort (beschreibend) an Hauptfunktion und Montageplatte verschieden – auch nur in einer Sprache oder durch Kennzeichnung „nicht übersetzen“ | Gleichen Text in allen Sprachen an beiden Stellen | nicht relevant |
| 012008 | Strukturkennzeichen ≠ Montageplatte | Anlage/Einbauort an Artikelplatzierung und Montageplatte verschieden | Gleiche Strukturkennzeichen vergeben | nicht relevant |
| 012009 | Artikel an Hauptfunktion fehlt / Stückzahl zu klein | Platzierung trägt Artikel, Hauptfunktion keinen/anderen, oder mehr Platzierungen als Stückzahl an der Hauptfunktion | Artikelnummer und Stückzahl der Hauptfunktion angleichen | nicht relevant |
| 012010 | Bauteil auf Platte ohne Schaltplan-Darstellung | Hauptfunktion des platzierten Bauteils fehlt im Schaltplan (geprüft: allpolig, einpolig, Übersicht) | Hauptfunktion im Schaltplan nachtragen | nicht relevant **Regel**: jedes BMK aus Aufbauplan/Stückliste braucht im Schaltplan eine Hauptfunktion (Master) |
| 012011 | Klemmenartikel ≠ Artikelplatzierung | Klemme und Klemmenleistendefinition tragen anderen Artikel als die platzierte Klemme | Gleiche Artikelnummer zuweisen | nicht relevant |

## 013 Artikeldaten / Stücklistenimport (9 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_013nnn.htm – Nummern: 013045, 013046, 013047, 013048, 013050, 013051, 013053, 013054, 013055

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 013045 | Ungültiger Artikel in Importdatei | Feldname der Artikelreferenz im device-Tag der Stücklisten-XML verändert. | Feldname in XML korrigieren (siehe P013048), erneut importieren. | nicht relevant |
| 013046 | Unbekanntes BMK (Stücklistenimport) | BMK aus importierter Stückliste existiert im Projekt nicht. | BMK in Plan und Importdatei angleichen. | eigenes Skript **Regel: Stücklisten-BMK muss einem Plan-BMK entsprechen** |
| 013047 | Artikelindex doppelt | REFERENCE_POS mehrfach gleich; Index = Reihenfolge der Artikel am Betriebsmittel. | Indizes in der Datei eindeutig machen. | nicht relevant |
| 013048 | Unbekannter Feldname | Tippfehler im Feldnamen der Importdatei (z. B. P_ARTICLE_MANUFACTURER); Daten werden ignoriert. | Feldnamen nach Exportfeldliste korrigieren. | nicht relevant |
| 013050 | Projektartikel nicht hinzufügbar | Mehr als 1.000 Projektartikel im Import. | Überzählige Projektartikel entfernen. | nicht relevant |
| 013051 | Mehr als 50 Artikel am Betriebsmittel | Einem BMK sollen über 50 Artikel zugeordnet werden (Grenze 50). | Überzählige Artikel entfernen. | nicht relevant |
| 013053 | Artikel nicht änderbar | Import versucht einen nicht änderbaren Artikel zu ändern. | – | nicht relevant |
| 013054 | Eigenschaft der Artikelreferenz nicht änderbar | Import will z. B. Bestellnummer an Artikelreferenz ändern. | Änderung zurücknehmen, neu importieren. | nicht relevant |
| 013055 | Artikel an geschütztem Betriebsmittel | Import will Artikel an geschütztem Betriebsmittel ändern. | Änderung zurücknehmen. | nicht relevant |

## 014 Makro (6 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_014nnn.htm – Nummern: 014001, 014002, 014003, 014004, 014005, 014006

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 014001 | Makroname mehrfach (Makrokasten) | Im Makroprojekt tragen zwei Makrokästen oder Zeilen einer Makro-Definition denselben Makronamen. | Eindeutige Makronamen vergeben. | eigenes Skript **Regel: beim Ablegen importierter Makros als QET-Elemente Dateinamen eindeutig halten** |
| 014002 | Makroname mehrfach (Bauraum) | Gleicher Makroname an mehreren Bauräumen bzw. doppelt in der Makro-Definition. | Namen eindeutig machen. | nicht relevant |
| 014003 | Makroversion weicht ab | Version am referenzierenden Makrokasten ≠ Version der Makrodatei. | Makro aktualisieren. | nicht relevant |
| 014004 | Kein Makroname am Makrokasten | Makrokasten ohne Namen in Makrokasten- und Makro-Definition-Register (beide Felder werden gespiegelt). | Makronamen in erster Zeile der Makro-Definition eintragen. | nicht relevant |
| 014005 | Kein Makroname am Bauraum | Bauraum im Makroprojekt ohne Makronamen. | Makronamen eintragen. | nicht relevant |
| 014006 | Verwendungsart am Makrokasten falsch | Verwendungsart „Nicht festgelegt“, „Definierend“ im Schaltplanprojekt oder falsche Verschachtelungsfolge. | Von außen nach innen: Definierend (nur Makroprojekt) → Referenzierend → Untergeordnet. | nicht relevant |

## 015 Auswertungen (4 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_015nnn.htm – Nummern: 015001, 015002, 015003, 015004

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 015001 | Unbekannter Auswertungstyp | Auswertung aus neuerer EPLAN-Version, hier nicht aktualisierbar. | Programm aktualisieren oder Auswertung löschen. | nicht relevant |
| 015002 | Montageplatte ≠ Verbindungsdiagramme | Artikelplatzierung oder Verbindungsdiagramm nach der Auswertung gelöscht/aus dem Bereich geschoben. | Einfügepunkte zurück in Platte/Gerätekasten, Diagramme neu auswerten. | nicht relevant |
| 015003 | Brücke nicht ausgegeben | Eingestellte Zahl der Brückenebenen im Klemmenplan zu klein. | Brückenebenen erhöhen oder 0 (= alle Brücken als Ziele ausgeben). | Sichtprüfung **Regel: Klemmenplan muss jede Brücke zeigen** |
| 015004 | Folgeformular nicht eingelagert | Formular nennt ein Folgeformular, das im Projekt fehlt. | Folgeformular einlagern. | nicht relevant |

## 016 Gerätekästen (2 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_016nnn.htm – Nummern: 016001, 016002

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 016001 | Schaltzeichen in zwei Gerätekästen | Symbol liegt in zwei überlappenden, nicht geschachtelten Gerätekästen; BMK-Zuordnung mehrdeutig. | Kästen schachteln oder so verschieben, dass das Symbol eindeutig in einem liegt. | qet_layout_check + Sichtprüfung **Regel: Gerätekästen dürfen sich nur vollständig geschachtelt überlappen** |
| 016002 | Gerätekasten ohne Anschluss | Gerätekasten enthält weder Geräteanschluss noch Funktion mit Anschlüssen. | Mindestens einen Anschluss bzw. eine Funktion einfügen. | eigenes Skript **Regel: Gerätekasten ohne Anschluss melden** |

## 017 BMK (13 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_017nnn.htm – Nummern: 017001, 017002, 017003, 017004, 017005, 017006, 017007, 017008, 017009, 017010, 017011, 017012, 017013

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 017001 | BMK ist leer | Funktion ohne Betriebsmittelkennzeichen ist nicht identifizierbar. | Vollständiges BMK eintragen. | qet_check **Regel: jedes Element mit Funktion braucht ein BMK** |
| 017002 | Nicht nummeriertes BMK | Online-Nummerierung mit Merker „?“ hat Platzhalter-BMK hinterlassen. | Merker entfernen bzw. nachnummerieren („Kennbuchstabe + Zähler“, nur gekennzeichnete); oder online ohne Merker nummerieren. | qet_check **Regel: BMK mit „?“ gilt als unnummeriert** |
| 017003 | Falscher Kennbuchstabe | Kennbuchstabe passt nicht zum Kennbuchstabensatz für die Funktionsdefinition. | Vorgeschlagenen Kennbuchstaben verwenden oder eigenen Kennbuchstabensatz anlegen und Stammdaten abgleichen. | eigenes Skript **Regel: Kennbuchstabe muss zur Gerätefunktion passen (Kennbuchstabensatz des Projekts, z. B. IEC 81346-2)** |
| 017004 | Unzulässige Zeichen im BMK | BMK enthält Zeichen außerhalb der BMK-Syntax. | Zeichen ersetzen oder Sonderzeichen (+ - _ * ?) bzw. Textsonderzeichen in der BMK-Syntaxüberprüfung zulassen. | eigenes Skript **Regel: BMK nur aus Buchstaben, Ziffern und ausdrücklich erlaubten Sonderzeichen** |
| 017005 | BMK mehrfach, zu viele Hauptfunktionen | Mehrere Hauptfunktionen tragen dasselbe BMK. | BMK unterscheiden oder genau eine Funktion als Hauptfunktion, übrige als Nebenfunktion. | qet_check **Regel: je BMK genau eine Hauptfunktion (Master), alle übrigen Slaves** |
| 017006 | Spule bereits vorhanden | Zwei Schützspulen mit gleichem BMK. | BMK einer Spule ändern. | qet_check **Regel: je BMK genau eine Spule; Kontakte verweisen auf diese** |
| 017007 | BMK nicht querverwiesen | Gleiches BMK einmal normal, einmal mit Zähler im Kennbuchstaben-Feld eingegeben → intern verschieden. | Kennbuchstabe und Zähler getrennt eingeben. | eigenes Skript **Regel: BMK vor Vergleich normalisieren (Präfix, Kennbuchstabe, Zähler); gleicher Text muss gleiche Zerlegung haben** |
| 017008 | BMK passt nicht zum Nummerierungsformat | BMK widerspricht Online-Nummerierungsformat, z. B. -FA ohne Zähler bei „Kennbuchstabe + Zähler“. | BMK gemäß Format (Kennbuchstabe, Zähler, ggf. Seite) ändern. | eigenes Skript **Regel: BMK braucht Zähler, wenn Format einen Zähler vorsieht; bei „Seite + KB + Zähler“ muss die Seite stimmen** |
| 017009 | BMK durch neue Berechnung angepasst | Einfügepunkt lag genau auf Gerätekastenrahmen; ab 2.1 BMK-Schachtelung einheitlich neu berechnet. | Geänderte BMK (alt/neu in Meldung) prüfen. | nicht relevant |
| 017010 | Überlappende Strukturkästen widersprüchlich | Zwei überlappende Strukturkästen geben für denselben Kennzeichnungsblock unterschiedliche Kennzeichen (z. B. +O1 und +E1). | Kennzeichen der Kästen angleichen. | Sichtprüfung **Regel: überlappende Strukturkästen dürfen sich in keinem Kennzeichnungsblock widersprechen** |
| 017011 | Sichtbares BMK mit überflüssiger Struktur | Sichtbares BMK wiederholt Strukturkennzeichen, die schon über die Seite im vollständigen BMK stehen. | Überflüssige =/+-Teile aus dem sichtbaren BMK löschen. | eigenes Skript **Regel: Seitenstruktur nicht im Symbol-BMK wiederholen; vollständiges BMK = Seitenstruktur + sichtbares BMK** |
| 017012 | Sichtbares BMK ohne Vorzeichen | Betriebsmittel-BMK ohne führendes „-“; berücksichtigt Projekteinstellung BMK. | „-“ vor den Kennbuchstaben setzen. | eigenes Skript **Regel: Produkt-BMK beginnt mit „-“** |
| 017013 | Betriebsmittel mit und ohne übergeordnete Struktur | Altprojekt nutzte „Übergeordnet“ für Klemmenleisten/Stecker/Kabel/Abbruchstellen; Option entfällt. | Allen Funktionen „-“ voranstellen, Strukturkennzeichen vereinheitlichen. | eigenes Skript **Regel: Klemmenleisten, Stecker, Kabel, Abbruchstellen tragen ebenfalls die Struktur; gesamtnummeriert ist nicht normgerecht** |

## 020 Projektvergleich (37 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_020nnn.htm – Nummern: 020001, 020003, 020004, 020010, 020011, 020012, 020013, 020014, 020015, 020016, 020017, 020018, 020026, 020027, 020028, 020029, 020030, 020031, 020032, 020033, 020034, 020035, 020036, 020037, 020038, 020039, 020040, 020041, 020042, 020043, 020044, 020045, 020046, 020047, 020048, 020049, 020050

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 020001 | Vergleich Einstellungen: Datum/Zeit | Protokoll: Zeitpunkt des Einstellungsvergleichs | – | nicht relevant |
| 020003 | Anzahl verglichener Einstellungen | Protokoll: Zahl verglichener Einstellungen | – | nicht relevant |
| 020004 | Anzahl Unterschiede (Einstellungen) | Protokoll: Zahl gefundener Unterschiede | – | nicht relevant |
| 020010 | Unterschied in Definitionseinstellung | Beim Wiederherstellen ausgelagerter Projekte: Definitionseinstellung weicht ab | – | nicht relevant |
| 020011 | Unterschied in Beschreibung | Ausgelagertes vs. Quellprojekt: Beschreibung weicht ab | – | nicht relevant |
| 020012 | Unterschied im Datentyp | Ausgelagertes vs. Quellprojekt: Datentyp weicht ab | – | nicht relevant |
| 020013 | Unterschied in Standardwerten | Ausgelagertes vs. Quellprojekt: Standardwerte weichen ab | – | nicht relevant |
| 020014 | Unterschied in Wertebereichen | Ausgelagertes vs. Quellprojekt: Wertebereiche weichen ab | – | nicht relevant |
| 020015 | Einstellung existiert nicht | Einstellung fehlt im Vergleichspartner | – | nicht relevant |
| 020016 | Anzahl Werte geändert | Einstellung hat andere Anzahl Werte als im Vergleichsprojekt | – | nicht relevant |
| 020017 | Einzelwert geändert | Wert an Position y alt → neu | – | nicht relevant |
| 020018 | Einstellungsbereich: n Abweichungen | Nur für Bereiche „Nummerieren mit SPS-Daten“ und „Nummerierung (offline)“ | – | nicht relevant |
| 020026 | Vergleich Eigenschaften: Datum/Zeit | Protokoll: Zeitpunkt des Eigenschaftsvergleichs | – | nicht relevant |
| 020027 | Erstes / zweites Projekt | Protokoll: Pfade Eingangs- und Vergleichsprojekt | – | nicht relevant |
| 020028 | Anzahl verglichener Eigenschaften | Protokoll | – | nicht relevant |
| 020029 | Anzahl Unterschiede (Eigenschaften) | Protokoll | – | nicht relevant |
| 020030 | Seite entfernt/hinzugefügt/geändert | Seitenname und Art der Änderung | – | nicht relevant (QET: `qet_diff` vergleicht zwei Stände) |
| 020031 | Eigenschaft geändert | Name, Index, alter und neuer Wert | – | nicht relevant |
| 020032 | Verbindung Quelle→Ziel geändert | Verbindung entfernt/hinzugefügt/geändert, Angabe über BMK von Quelle und Ziel; ohne BMK, wenn nur im Navigator | – | nicht relevant (QET: `qet_diff`) – Verbindungen werden über Quell-/Ziel-BMK identifiziert |
| 020033 | Potenzial geändert | Potenzialdefinition oder -anschluss entfernt/hinzugefügt/geändert | – | nicht relevant |
| 020034 | Betriebsmittel geändert | Funktion mit vollst. BMK und X/Y-Position; Position 0 = nur im Navigator, unplatziert | – | nicht relevant (QET: `qet_diff`) |
| 020035 | Klemme geändert | Klemme mit vollst. BMK und Position; 0 = unplatziert | – | nicht relevant |
| 020036 | Erweitertes Strukturelement geändert | z. B. Trennzeichen der erweiterten Projektstruktur geändert | – | nicht relevant |
| 020037 | Seiten-/BMK-Struktur geändert | Kennzeichnungsblock der Seiten- oder Betriebsmittelstruktur geändert | – | nicht relevant |
| 020038 | Einstellung BMK in Einzelfeldern geändert | Nur kundenspezifische EPLAN-Version | – | nicht relevant |
| 020039 | BMK-Format geändert | BMK-Format eines Betriebsmitteltyps geändert | – | nicht relevant |
| 020040 | Reihenfolge Seitenstruktur geändert | Logische Reihenfolge der Kennzeichnungsblöcke der Seitenstruktur geändert | – | nicht relevant |
| 020041 | Ebene der Eigenschaftsplatzierung abweichend | Eigenschaftsplatzierung eines Symbols im Plan liegt auf anderer Ebene als in der Symbolbibliothek | Ebene an die Bibliothek angleichen | nicht relevant |
| 020042 | Ebeneneinstellung geändert | Einstellung einer Ebene alt → neu | – | nicht relevant |
| 020043 | Neue Ebene gefunden | Ebene neu angelegt | – | nicht relevant |
| 020044 | Bild geändert | Eingefügte Bilddatei entfernt/hinzugefügt/geändert | – | nicht relevant |
| 020045 | Hyperlink geändert | Hyperlink entfernt/hinzugefügt/geändert | – | nicht relevant |
| 020046 | Vergleich Projektstruktur: Datum/Zeit | Protokoll | – | nicht relevant |
| 020047 | Vergleich Ebenen: Datum/Zeit | Protokoll | – | nicht relevant |
| 020048 | Text / Pfad-Funktionstext geändert | Text oder Pfad-Funktionstext an Position x/y geändert | – | nicht relevant |
| 020049 | Vorplanungssegment geändert | Vorplanungssegment entfernt/hinzugefügt/geändert | – | nicht relevant |
| 020050 | Vorplanungsplatzierung geändert | Vorplanungsplatzierung entfernt/hinzugefügt/geändert | – | nicht relevant |

## 021 Platzhalterobjekte (5 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_021nnn.htm – Nummern: 021001, 021002, 021003, 021004, 021005

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 021001 | Platzhalterobjekt ohne Variablen | Am Platzhalterobjekt ist keine Variable definiert | Variable und Wertesatz anlegen und einer Objekteigenschaft zuordnen | nicht relevant |
| 021002 | Platzhalterobjekt ohne Objektverweise | Beim Erweitern gingen die zugeordneten Objekte verloren; Variablen ohne Ziel | Objekte erneut dem Platzhalterobjekt zuordnen, Variablen zuweisen | nicht relevant |
| 021003 | Ungültige Objektverweise | Ein zugeordnetes Schaltzeichen wurde gelöscht | Dialog einmal öffnen/schließen (aktualisiert Verweise) oder Objekt wiederherstellen | nicht relevant |
| 021004 | Ungültige Variablenzuordnung | Datentyp der Variable passt nicht zur Eigenschaft (z. B. Text auf Ja/Nein) | Passend typisierte Variable zuordnen | nicht relevant |
| 021005 | Gleichnamige Platzhalterobjekte, andere Variablen | Im Makroprojekt tragen Platzhalterobjekte gleichen Namen mit verschiedenen Variablen (Fehler in EEC One) | Umbenennen oder Variablen vereinheitlichen | nicht relevant **Regel**: beim Makro-Import (EPLAN-Makros mit Wertesätzen) nur die im Plan gewählte Variante übernehmen; Platzhalter-Logik gibt es in QET nicht |

## 022 Sonstiges (20 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_022nnn.htm – Nummern: 022001, 022002, 022003, 022004, 022011, 022012, 022013, 022014, 022015, 022016, 022017, 022018, 022019, 022020, 022025, 022026, 022027, 022050, 022060, 022061

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 022001 | Symbolbibliothek an Position fehlt | Makro erwartet eine Symbolbibliothek an bestimmter Listenposition, im Zielprojekt nicht eingelagert. | Bibliothek an dieser Position einlagern, Makro neu einfügen. | nicht relevant |
| 022002 | Symbolvariante fehlt in Bibliothek | Makro nutzt ein aus der Bibliothek gelöschtes Symbol; erscheint als rotes x. | Symbol neu anlegen bzw. Stammdaten abgleichen, Makro neu einfügen. | eigenes Skript **Regel: nach Bauteil-Import jede Elementreferenz im Projekt auflösbar** |
| 022003 | Seitennummer doppelt | Datenübernahme (z. B. EPLAN 21) erzeugt doppelte Seitennummern. | Eine Seite umbenennen. | eigenes Skript **Regel: Folio-Nummern eindeutig** |
| 022004 | Anschlüsse/Symbole übereinander | Gleichgerichtete Anschlüsse oder identische Symbole liegen deckungsgleich übereinander. | Symbole verschieben, damit eindeutige Verbindungen entstehen. | qet_layout_check |
| 022011 | EEC-Hinweis | Freitextmeldung von EPLAN Engineering Configuration, Kategorie Hinweis. | – | nicht relevant |
| 022012 | EEC-Warnung | Freitextmeldung von EPLAN Engineering Configuration, Kategorie Warnung. | – | nicht relevant |
| 022013 | EEC-Fehler | Freitextmeldung von EPLAN Engineering Configuration, Kategorie Fehler. | – | nicht relevant |
| 022014 | Eigenschaft für Seitenanzahl pro Eigenschaft leer | Für „Seitenanzahl / -namen pro Eigenschaft“ gewählte Seiteneigenschaft ist auf einer Seite leer. | Wert an der Seite eintragen, Funktion neu ausführen. | nicht relevant |
| 022015 | Zusammengefasste Seiten nicht fortlaufend | Seiten mit gleichem Eigenschaftswert liegen nicht hintereinander. | Seiten neu nummerieren (Start/Schritt 1, ggf. strukturbezogen). | nicht relevant |
| 022016 | Buchstaben im Seitennamen | Seitenname enthält nach „/“ Buchstaben. | Seitennummer rein numerisch machen. | eigenes Skript |
| 022017 | Feldlänge in Datenbank überschritten | Sehr lange Kennzeichen werden in EPLAN-Datenbanken gekappt. | Projekt reorganisieren; BMK-Grenzen: Vorziffer 16, Kennbuchstabe 16, Zähler 64, Unterzähler 16 Zeichen. | nicht relevant |
| 022018 | Linientyp nicht unterstützt | P&ID-Linientyp an Objekt, das keine Linie/Rechteck/Polylinie ist. | – | nicht relevant |
| 022019 | Ungültige Funktionsdefinition | Symbol nutzt Funktionsdefinition, die in der eingelagerten Bibliothek fehlt. | Stammdaten des Projekts aktualisieren. | eigenes Skript |
| 022020 | Veraltetes Übernahmesymbol | Symbol mit Nummer > 3000 aus EPLAN-5/21-Übernahme; sollte ersetzt werden. | Durch aktuelles Symbol ersetzen. | nicht relevant |
| 022025 | Textwinkel ungleich 0°/90° | Text auf Schaltplan-/Übersichtsseite schräg; Bemaßungen, Blöcke, Auswertungen ausgenommen. | Winkel auf 0° oder 90° stellen. | eigenes Skript **Regel: Texte nur 0° oder 90°** |
| 022026 | Text abgeschnitten | Text ragt aus aktiviertem Positionsrahmen. | Rahmen vergrößern, Schrift verkleinern oder „Text einpassen erlaubt“. | Sichtprüfung |
| 022027 | Text zu klein | Schriftgröße unter der projektweiten Mindestschriftgröße. | Schrift vergrößern oder Mindestwert anpassen. | eigenes Skript |
| 022050 | GUID (AutomationML) nicht eindeutig | Mehrere Hauptfunktionen mit gleicher AML-GUID. | Eindeutige GUID je Hauptfunktion. | nicht relevant |
| 022060 | Benutzerdef. Eigenschaft mit ungültigem Wert | Durch Kopieren/Makro gelangte ein Wert außerhalb der Auswahlliste in eine benutzerdefinierte Eigenschaft. | Gültigen Listenwert wählen. | nicht relevant |
| 022061 | Benutzerdef. Eigenschaft mit nicht vorgegebenem Wert | Manueller Wert nicht in der konfigurierten Auswahlliste. | Listenwert wählen oder Liste erweitern. | nicht relevant |

## 024 Fluid (18 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_024nnn.htm – Nummern: 024001, 024002, 024003, 024004, 024005, 024006, 024007, 024008, 024009, 024010, 024011, 024012, 024013, 024014, 024015, 024016, 024017, 024018

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 024001 | Druck-/Steuerleitung widersprüchlich | Anschlusslogik von Quelle und Ziel nennt verschiedene Leitungsarten (z. B. Druck- vs. Leckölleitung) | Leitungsart an beiden Anschlüssen angleichen | nicht relevant |
| 024002 | Verbindung zwischen Gewerken | Verbindung koppelt Funktionen verschiedener Gewerke (Pneumatik/Hydraulik) | Gewerk vereinheitlichen | nicht relevant |
| 024003 | E-Funktion mit Fluid-Charakter ohne Paarquerverweis | Ventilspule bzw. Analogsensor (Druck, Durchfluss, Füllstand, Temperatur) auf Fluidseite nicht als Darstellungsart „Paarquerverweis“ | Darstellungsart „Paarquerverweis“ setzen | Sichtprüfung **Regel**: Ventilspule im Fluidplan ist nur Paarquerverweis-Darstellung; Hauptfunktion (Master) bleibt im Stromlaufplan, beide gleiche BMK |
| 024004 | Verbindung zwischen Untergewerken | Untergewerke der Ziele verschieden (z. B. Wasser- vs. Ölkühlung) | Untergewerk vereinheitlichen | nicht relevant |
| 024005 | Undefinierter mit definiertem Anschluss verbunden | Ein Ende „Undefiniert“, anderes Ende definierte Leitungsart | Leitungsart an beiden Enden setzen | nicht relevant |
| 024006 | Medienschlüssel passt nicht zum Gewerk | BMK-Medienbuchstabe widerspricht DIN ISO 1219-2 (z. B. H in Pneumatik) | Medienschlüssel normgerecht wählen | nicht relevant |
| 024007 | Hauptfunktion mit undefiniertem Gewerk | Gewerk der Fluid-Hauptfunktion steht auf „undefiniert“ | Gewerk setzen, passenden Medienschlüssel im BMK | nicht relevant |
| 024008 | Unterschiedliche Paketdaten | Verteilt dargestelltes Paket: Haupt- und Nebenfunktionen widersprechen sich in Typ, Verbindungsanzahl, Querschnitt/Durchmesser | Daten an allen Darstellungen vereinheitlichen | nicht relevant (analog Kabel: `qet_continuity`) |
| 024009 | Paketverbindung ohne Paket | Verbindungsdefinitionspunkt liegt nicht mehr auf der Paketdefinitionslinie | Punkt auf den Schnittpunkt schieben | nicht relevant |
| 024010 | Ungültige Paketlänge | Negative Länge an der Paketdefinition | Positive Länge eintragen | nicht relevant |
| 024011 | Paketziel ohne vollständiges BMK | Quelle/Ziel einer Paketverbindung ohne vollständiges BMK | BMK an den Zielen vergeben | nicht relevant |
| 024012 | Zu viele Paketverbindungen | Verbindungsanzahl im Paket kleiner als tatsächliche Verbindungen (freie Schablonen zählen nicht) | Verbindungsanzahl erhöhen | nicht relevant |
| 024013 | Paket: Funktionstext unterschiedlich | Funktionstext (automatisch) an Paketdarstellungen in einer Sprache verschieden | Texte angleichen | nicht relevant |
| 024014 | Paketlänge fehlt | Hauptfunktion des Pakets hat keine Länge | Länge eintragen | nicht relevant |
| 024015 | Paket nutzt nicht alle Artikelverbindungen | Weniger Paketverbindungen platziert als der Artikel Schablonen hat | Passenden Artikel wählen oder Schablonen reduzieren | nicht relevant |
| 024016 | Verbindungs-BMK ≠ Paket-BMK | Verbindungsdefinitionspunkt im Paketbereich trägt eigenes, widersprechendes BMK | Eigenes BMK am Verbindungspunkt löschen | nicht relevant (analog Kabel: Ader erbt Kabel-BMK) |
| 024017 | Funktionsübersicht-Funktion ohne Schaltplan-Gegenstück | Funktion auf Seite „Funktionsübersicht (Fluid)“ hat kein allpoliges Gegenstück | Allpolige Darstellung ergänzen (Übersicht ist nie Hauptfunktion) | nicht relevant |
| 024018 | Funktionsübersicht-Verbindung ohne Schaltplan-Gegenstück | Verbindung der Funktionsübersicht hat keine allpolige Entsprechung | Allpolige Verbindung ergänzen | nicht relevant |

## 025 Projekteinstellungen (1 Meldung)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_025nnn.htm – Nummern: 025001

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 025001 | Seitenraster weicht von Projekteinstellung ab | Seiteneigenschaft Raster (z. B. 2 mm) weicht von „Raster für Seitentyp“ (z. B. 4 mm) ab. | Seitenraster und Projekteinstellung angleichen. | qet_layout_check |

## 026 3D-Montageaufbau (69 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_026nnn.htm – Nummern: 026010, 026011, 026012, 026013, 026014, 026015, 026016, 026017, 026018, 026019, 026020, 026021, 026023, 026024, 026025, 026026, 026027, 026028, 026029, 026030, 026033, 026034, 026036, 026037, 026038, 026039, 026040, 026041, 026042, 026043, 026044, 026045, 026052, 026053, 026054, 026055, 026056, 026057, 026058, 026062, 026070, 026071, 026072, 026073, 026074, 026075, 026076, 026077, 026078, 026079, 026080, 026081, 026089, 026090, 026091, 026092, 026093, 026094, 026095, 026096, 026097, 026098, 026099, 026100, 026101, 026102, 026103, 026104, 026105

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 026010 | Artikel vollständig außerhalb Montagefläche | Artikel ohne Kontakt zur Montagefläche | Auf die Fläche schieben | nicht relevant |
| 026011 | 3D-Grafik kollidiert | Artikel durchdringt Schrank, Platte, Schiene oder anderen Artikel | Verschieben | nicht relevant |
| 026012 | Kollision mit Platziersperrfläche | Artikel und Platziersperrfläche überlappen | Eines von beiden verschieben | nicht relevant |
| 026013 | Kollision mit Bohrsperrfläche | Artikel und Bohrsperrfläche überlappen | Eines von beiden verschieben | nicht relevant |
| 026014 | Einbauabstände nicht eingehalten | Abstand zum Nachbarn kleiner als Einbauabstand aus Montagedaten | Artikel verschieben | nicht relevant |
| 026015 | Artikelplatzierung ohne Artikel | Platzierung ohne Artikelnummer | Artikel zuweisen | nicht relevant |
| 026016 | Betriebsmittel ohne Schaltplan-Darstellung | Hauptfunktion des platzierten Geräts fehlt im Schaltplan (Gewerk Mechanik ausgenommen) | Hauptfunktion mit gleichem BMK nachtragen | nicht relevant **Regel**: Stücklisten-/Aufbau-BMK ohne Master im Schaltplan ist ein Fehler, außer mechanische Teile |
| 026017 | Strukturkennzeichen ≠ Montagefläche | Anlage/Einbauort von Platzierung und Montagefläche verschieden | Angleichen | nicht relevant |
| 026018 | Abmessungen ≠ Artikel | Artikel getauscht oder Artikeldaten geändert, Platzierung nicht aktualisiert | Artikelplatzierung aktualisieren | nicht relevant |
| 026019 | Artikel/Stückzahl an Hauptfunktion/Hauptklemme fehlt | Platzierung und Hauptfunktion/-klemme mit abweichendem Artikel oder zu kleiner Stückzahl | Artikel und Stückzahl angleichen | nicht relevant |
| 026020 | Funktionsdefinition ≠ Artikel | Funktionsdefinition der Platzierung weicht von der Funktionsschablone der Hauptfunktion ab | Angleichen | nicht relevant |
| 026021 | Montageort Fläche ≠ Hauptfunktion | Montageort (beschreibend) an Hauptfunktion/-klemme und Montagefläche verschieden, auch sprachabhängig | Gleiche Daten eintragen | nicht relevant |
| 026023 | Länge > Lieferlänge | Tragschiene/Kanal länger als Lieferlänge des Artikels | Kürzen oder anderen Artikel | nicht relevant |
| 026024 | Bauraumname mehrfach | Über Schnittstellen doppelte Bauraumnamen | Bauraum umbenennen | nicht relevant |
| 026025 | Artikel nicht auf Tragschiene | Schiene gekürzt oder Mittenversatz/Aufklipshöhe geändert | Neu aufsetzen / Schiene verlängern | nicht relevant |
| 026026 | Maßpunkt außerhalb Modellansicht | Bemaßungspunkt liegt außerhalb der Modell-Ansicht | Neu bemaßen | nicht relevant |
| 026027 | Durchbruch nicht auf Montagefläche | Durchbruch ragt über die Fläche | Verschieben | nicht relevant |
| 026028 | Artikel komplett im Durchbruch | Artikel ohne Kontakt zur Fläche | Verschieben | nicht relevant |
| 026029 | Kein gültiges Bohrbild | Bohrung gefordert, aber kein/ungültiges Bohrbild | Gültiges Bohrbild zuweisen | nicht relevant |
| 026030 | NC-Datensatz fehlt für Kontur | Benutzerdefinierte Kontur ohne NC-Daten | NC-Datensatz anlegen | nicht relevant |
| 026033 | Bündel in mehreren Bündelgruppen | Verbindung einem bereits anders gruppierten Bündel zugeordnet | Bündel/Gruppe korrigieren | nicht relevant |
| 026034 | NC-Montagefläche ohne Feldgröße | 3D-Makro mit NC-Fläche ohne Feldgröße | Feldgröße ergänzen | nicht relevant |
| 026036 | Artikel im Verlegebereich | Artikel steht in einem Verlegebereich | Verschieben oder Bereich ändern | nicht relevant |
| 026037 | Strecken übereinander | Strecken/Kurven überdecken sich | Verschieben oder löschen | nicht relevant |
| 026038 | Streckennetz veraltet | Streckennetzobjekt geändert, Netz nicht neu erzeugt | Streckennetz erzeugen | nicht relevant |
| 026039 | Verbindung durch ungültige Strecken | Strecke/Kanal nach dem Verlegen gelöscht | Netz erzeugen, neu verlegen | nicht relevant |
| 026040 | Verbindung/Kabel ohne Durchmesser | Füllgradberechnung findet keinen gültigen Durchmesser | Durchmesser angeben | nicht relevant |
| 026041 | Quelle/Ziel in verschiedenen Bauräumen | Verbindungsenden in verschiedenen Bauräumen | In einen Bauraum legen | nicht relevant |
| 026042 | Kein Einstieg ins Streckennetz | Anschluss findet in Verlegerichtung keine Strecke | Verlegerichtung / Umfeld prüfen | nicht relevant |
| 026043 | Kein Verlegeweg Quelle–Ziel | Streckennetz lückenhaft oder Verbindungsfilter sperrt (Seite ohne U/L-Gliederung) | Netz ergänzen, Filter prüfen | nicht relevant |
| 026044 | Kein Anschlussbild | Artikelplatzierung ohne Anschlussbild | Anschlussbild hinterlegen | nicht relevant |
| 026045 | Verlegeverbindung endet nicht am Ziel | Ziel nach dem Verlegen verschoben | Neu verlegen | nicht relevant |
| 026052 | Quelle/Ziel nicht platziert | Ende einer Verlegeverbindung fehlt im Bauraum | Platzieren | nicht relevant |
| 026053 | Anschluss fehlt, nicht erzeugbar | Mehrere Platzierungen mit gleichem BMK, keine enthält die gesuchte Anschlussbezeichnung | Anschlussbezeichnung ergänzen | nicht relevant **Regel**: bei Geräten aus mehreren Teilen (gleiches BMK) muss jede Anschlussbezeichnung genau einmal vorkommen |
| 026054 | Kabelverbindungen nicht eindeutig | Kabel ohne Adern, oder Adernenden unplatziert/verschiedene Bauräume/Anschluss fehlt oder mehrfach (Seite ohne U/L-Gliederung) | Enden platzieren, Anschlussbilder bereinigen | nicht relevant **Regel**: ein Kabel ohne zugeordnete Adern ist ein Fehler |
| 026055 | Anschluss mehrfach vorhanden | Gleiche Anschlussbezeichnung in mehreren Teilen eines BMK – Ziel mehrdeutig | Anschlussbezeichnungen eindeutig machen | `qet_check` **Regel**: Anschlussbezeichnung je BMK eindeutig (sonst Verbindung nicht zuordenbar) |
| 026056 | Querschnitt > Max. im Anschlussbild | Leiterquerschnitt größer als zulässiger Klemmbereich | Querschnitt oder Anschlussbild anpassen | nicht relevant |
| 026057 | Querschnitt < Min. im Anschlussbild | Leiterquerschnitt kleiner als zulässiger Klemmbereich | Querschnitt oder Anschlussbild anpassen | nicht relevant |
| 026058 | Zu viele Drähte am Anschluss | Mehr Drähte als „Max. Anzahl Drähte“ im Anschlussbild | Zielverdrahtung (T-Stücke/Winkel) prüfen oder Grenze erhöhen | `qet_check` (überfüllte Anschlüsse) **Regel**: Verbindungsreihenfolge über T-/Winkel-Symbole bestimmt Drähte je Anschluss |
| 026062 | Allpolige Verbindung nicht verlegt | Keine Verlegeverbindung bzw. Verlegeweg leer | Neu verlegen | nicht relevant |
| 026070 | Gebogene Sammelschiene: Länge ungültig | Biegungstabelle ohne gültige Korrekturfaktoren | Tabelle ergänzen, aktualisieren | nicht relevant |
| 026071 | Werkzeug fehlt (Kupfer-NC) | Werkzeugliste der Maschine unvollständig | Liste ergänzen / Durchbruch ändern | nicht relevant |
| 026072 | Keine Drahtkonfektionierung | Export: Querschnitt/Farbe o. ä. fehlt oder zu groß | Daten ergänzen, neu exportieren | nicht relevant |
| 026073 | Verlegeverbindung ohne Artikel/Farbe/Querschnitt | Mindestens eine dieser Eigenschaften leer | Ergänzen | nicht relevant |
| 026074 | NC-Export-Fehler | Einstellung für NC-Export fehlt (z. B. Konturdatensatz) | Ergänzen | nicht relevant |
| 026075 | Biegeradius zu klein | Schlauch/Rohr unter Mindestbiegeradius | Radius vergrößern | nicht relevant |
| 026076 | Kabelschutz/-binder nicht auf Strecke | Binder/Schutz von der Strecke verschoben | Löschen, neu verlegen | nicht relevant |
| 026077 | Bündeldurchmesser zu groß | Bündel größer als Binder-Maximum | Anderen Artikel | nicht relevant |
| 026078 | Bündeldurchmesser zu klein | Bündel kleiner als Binder-Minimum | Anderen Artikel | nicht relevant |
| 026079 | Export Rittal Athex | Unzulässige Einstellung, z. B. Schiene zu lang | Laut Platzhalter korrigieren | nicht relevant |
| 026080 | Exportdaten nicht erzeugbar | Kein 3D-Aufbau oder keine Durchbrüche | Aufbau/Durchbrüche ergänzen | nicht relevant |
| 026081 | Armatur nicht platzierbar | Grafikmakro der Armatur fehlt/falsch | Artikeldaten korrigieren | nicht relevant |
| 026089 | Verlegeverbindung kollidiert | Verbindung durchdringt Verbindung/Artikel | Umverlegen | nicht relevant |
| 026090 | Verlustleistung nicht berechenbar | Werte wie Verlustleistung/Gleichzeitigkeitsfaktor fehlen | Werte ergänzen | nicht relevant |
| 026091 | Projektwerte Klimatisierung fehlen | Gleichzeitigkeitsfaktor, Spannung, Frequenz fehlen | Projekteigenschaften ergänzen | nicht relevant |
| 026092 | Neue Klimatisierungsberechnung nötig | Verlustleistung im Klimabereich geändert | Neu berechnen | nicht relevant |
| 026093 | Umgebungsbedingungen am Schrank fehlen | Aufstellsituation, Außentemperatur u. a. fehlen | Ergänzen | nicht relevant |
| 026094 | Klimabereichsnummer doppelt | Klimabereichsnummer im Projekt mehrfach | Eindeutig vergeben | nicht relevant |
| 026095 | Durchbrüche überlappen | Zwei Durchbrüche überdecken sich | Verschieben | nicht relevant |
| 026096 | Zusatzlänge Armatur = 0 | Anschlussbild des Armaturmakros mit Zusatzlänge 0 | Wert > 0 | nicht relevant |
| 026097 | Außerhalb optimal klimatisierten Bereichs | Artikelmittelpunkt außerhalb Kühlbereich | Verschieben | nicht relevant |
| 026098 | Kollision lüftungstechnischer Sperrraum | Artikel ragt in Luftweg | Verschieben | nicht relevant |
| 026099 | Ungleiche Verlustleistungsverteilung | Schwerpunkt oben oder unten im Klimabereich | Umverteilen | nicht relevant |
| 026100 | Aktualisieren der Platzierung unmöglich | Artikel fehlt, Funktionsdefinition passt nicht u. a. | Laut Platzhalter | nicht relevant |
| 026101 | Automatische Verdrahtung unmöglich | Averex-Roboter: z. B. Anschlussausführung nicht Schraube/Push-In (Seite ohne normale U/L) | Laut Platzhalter | nicht relevant |
| 026102 | Artikel nicht migrierbar (TS8→VX25) | Auslaufartikel | Alternativen VX25-Artikel setzen | nicht relevant |
| 026103 | Fehlerhafte Artikelplatzierung | Nach Tausch/Migration fehlt die Montagefläche | Neu platzieren | nicht relevant |
| 026104 | Artikel noch nicht migriert | TS8-Artikel nicht auf VX25 umgestellt | Manuell migrieren | nicht relevant |
| 026105 | BMK mehrfach (Bauraum) | Kopierte/umbenannte Tragschiene oder Kanal mit gleichem BMK | Eindeutiges BMK | nicht relevant |

## 027 Topologie (23 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_027nnn.htm – Nummern: 027002, 027003, 027004, 027007, 027009, 027010, 027012, 027013, 027014, 027015, 027016, 027017, 027018, 027019, 027020, 027021, 027024, 027025, 027026, 027027, 027028, 027029, 027030

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 027002 | Topologie-Funktion ohne Strecke | Funktion/Verlegepunkt nicht an Strecke angeschlossen | Mit Strecke verbinden | nicht relevant |
| 027003 | Topologie-Funktion ohne Verbindung | Keine Verlegeverbindung am Anschluss | Ziel ergänzen / verlegen | nicht relevant |
| 027004 | Verbindung nicht durch Strecke verlegt | Nur Luftlinie zwischen zwei Topologie-Funktionen | Strecken ergänzen, verlegen | nicht relevant |
| 027007 | Mehrere Topologie-Funktionen je BMK | Gleiches BMK mehrfach im Streckennetz (auch Verlegepunkte/Strecken); Klemmenleistendefinitionen ausgenommen | Umbenennen/löschen | nicht relevant |
| 027009 | Strecke mit offenem Ende | Streckenende ohne Funktion, Abbruchstelle oder Verlegepunkt | Ende abschließen | nicht relevant |
| 027010 | Verlegeweg mit ungültiger Strecke | Strecke gelöscht, nicht neu verlegt | Lücke schließen, neu verlegen | nicht relevant |
| 027012 | Mehrere Verlegewege gefüllt | Verbindung in Topologie und Bauraum zugleich verlegt | Eine Darstellung entfernen | nicht relevant |
| 027013 | Gleiches BMK Topologie-/normale Abbruchstelle | Topologie-Abbruchstelle trägt BMK einer normalen Abbruchstelle | Anderes BMK vergeben | `qet_check` **Regel**: Abbruchstellen-Namen sind projektweit eindeutig je Art; keine Namensgleichheit mit fremden Abbruchstellen-Arten |
| 027014 | Topologie-Strecken übereinander | Strecken doppelt gezeichnet | Überzählige löschen | nicht relevant |
| 027015 | Topologie-Streckennetz veraltet | Netzobjekt geändert ohne Neuverlegen | Projektweit neu verlegen | nicht relevant |
| 027016 | Kein Verlegeweg (Filter) | Verbindungsfilter an Strecken sperrt den Weg | Gültigen Weg schaffen | nicht relevant |
| 027017 | Topologie: kein Anschlussbild | Weder lokales noch Artikel-Anschlussbild | Anschlussbild zuweisen | nicht relevant |
| 027018 | Anschluss fehlt, nicht erzeugbar | Bei mehrfach dargestellten Klemmenleisten fehlt Anschluss im Anschlussbild | Anschlussbild ergänzen | nicht relevant |
| 027019 | Kabelverbindungen nicht eindeutig | Zwei Adern eines Kabels mit gleicher Farbe/Nummer und/oder gleichem Paarindex | Eindeutige Werte je Ader | eigenes Skript **Regel**: innerhalb eines Kabels muss die Kombination Farbe/Nummer + Paarindex je Ader eindeutig sein |
| 027020 | Anschluss mehrfach vorhanden | Anschluss bei mehrfach dargestellter Klemmenleiste doppelt im Anschlussbild | Bereinigen | nicht relevant |
| 027021 | Allpolige Verbindung nicht verlegt | Topologie-Funktionen x und y existieren, aber ohne Verlegeverbindung | Strecken ergänzen, verlegen | nicht relevant |
| 027024 | Funktionsdefinition ≠ Hauptfunktion | Topologie-Nebenfunktion mit anderer Funktionsdefinition als allpolige Hauptfunktion | Angleichen | nicht relevant |
| 027025 | Klemmenleistendefinition mehrfach im Netz | Gleiche Klemmenleistendefinition mehrfach im selben Streckennetz (projektweit erlaubt) | Eine entfernen | nicht relevant |
| 027026 | Leitungsverteiler fehlt in Topologie | Allpoliger Verteiler ohne Topologie-Gegenstück | Gegenstück platzieren | nicht relevant |
| 027027 | Leitungsverteiler nicht am Verlegeweg | Verteiler liegt nicht am Weg der Verbindung | Umverlegen | nicht relevant |
| 027028 | Strecke > Lieferlänge | Strecke länger als Lieferlänge ihres Artikels | Kürzen | nicht relevant |
| 027029 | Anschluss in Topologie-Funktion doppelt | Anschluss im Anschlussbild mehrfach | Bereinigen | nicht relevant |
| 027030 | Teilstrecken nicht paarweise | Zwei Teilstrecken gleichen BMK sind beide Haupt- oder beide Nebenfunktion | Eine als Haupt-, eine als Nebenfunktion | nicht relevant |

## 028 Vorplanung (31 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_028nnn.htm – Nummern: 028001, 028002, 028003, 028004, 028005, 028006, 028007, 028008, 028009, 028010, 028011, 028012, 028013, 028014, 028015, 028016, 028017, 028018, 028019, 028020, 028021, 028022, 028023, 028024, 028025, 028026, 028027, 028028, 028029, 028030, 028031

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 028001 | Struktursegment mehrfach | Gleiche Bezeichnung mehrfach | Umbenennen | nicht relevant |
| 028002 | Planungsobjekt mehrfach | Gleiche Bezeichnung mehrfach | Umbenennen | nicht relevant |
| 028003 | Strukturkennzeichen in Segmenten mehrfach | Kennzeichen in mehreren Struktursegmenten | Eindeutig machen | nicht relevant |
| 028004 | Mehrere potenzielle Hauptfunktionen | Mehrere Artikel am Planungsobjekt bringen je eine Hauptfunktion mit | Artikel bereinigen | nicht relevant |
| 028005 | Wertesatz passt nicht zum Makro | Platzhalter-Wertesatz geändert oder Makro gelöscht | Makro/Wertesatz neu zuordnen | nicht relevant |
| 028006 | Vollständiges BMK ≠ Vorplanung | BMK-Teile, die das Planungsobjekt vorgibt, weichen an der Funktion ab (nur vorgegebene Blöcke werden geprüft) | Umbenennen/neu nummerieren | nicht relevant |
| 028007 | Artikel/Stückzahl Hauptfunktion ≠ Planungsobjekt | Artikeldaten verschieden | Angleichen | nicht relevant |
| 028008 | Funktionsschablone ohne Funktion | Freie Schablone noch nicht im Schaltplan platziert | Detailplanung erstellen | nicht relevant |
| 028009 | Nicht alle Darstellungen am Planungsobjekt | Kontakte manuell mit gleichem BMK eingefügt statt aus der Vorplanung | Detailplanung aktualisieren / offline nummerieren / zuweisen | nicht relevant |
| 028010 | SPS-Anschluss ≠ SPS-Adresse der Vorplanung | Gleiche CPU + symbolische Adresse, aber Adresse, Funktionstext, Datentyp oder Richtung anders | Daten abgleichen | nicht relevant (Hinweis: SPS-Anschlüsse werden über CPU-Name + symbolische Adresse identifiziert; Richtung aus Funktionsdefinition) |
| 028011 | SPS-Adresse ohne platzierten Anschluss | Vorgeplante Adresse ohne Detailplanung | Platzieren | nicht relevant |
| 028012 | Struktursegment mehrfach platziert | Mehr als eine grafische Platzierung | Überzählige löschen | nicht relevant |
| 028013 | Planungsobjekt mehrfach platziert | Mehr als eine Platzierung in einer Darstellungsart | Überzählige löschen | nicht relevant |
| 028014 | Artikelschablonen plus eigene Schablone | Eigene Schablone und Artikel mit Schablonen zugleich | Eines entfernen | nicht relevant |
| 028015 | Segmentdefinition „nicht mehr verwenden“ | Gesperrte Segmentdefinition im Einsatz | Freigeben oder tauschen | nicht relevant |
| 028016 | Unerlaubte Segment-Unterordnung | Segment unter verbotener Definition | Andere Definition | nicht relevant |
| 028017 | PLT-Stellen-Bezeichnung mehrfach | Identifizierende Bezeichnung laut Nummerierungseinstellung doppelt | Umbenennen | nicht relevant |
| 028018 | Segment ≠ Segmentvorlage | Eigenschaft nach Zuordnung geändert | Vorlagenwert übernehmen | nicht relevant |
| 028019 | Makro am Planungsobjekt ≠ platziert | Makroname verschieden | Angleichen | nicht relevant |
| 028020 | Makro nicht allpolig platziert | Eingetragenes Makro fehlt in allpoliger Darstellung | Platzieren | nicht relevant |
| 028021 | Makro nicht einpolig platziert | Eingetragenes Makro fehlt in einpoliger Darstellung | Platzieren | nicht relevant |
| 028022 | Planungsobjekt nicht in Vorplanung platziert | Fehlt auf Vorplanungsseite | Platzieren | nicht relevant |
| 028023 | PLT-Stelle nicht im RI-Fließbild | Fehlt im RI-Fließbild | Platzieren | nicht relevant |
| 028024 | Zugeordnete Vorplanung gelöscht | Segment gelöscht, Objekte im Plan verwaist | Objekte prüfen | nicht relevant |
| 028025 | Makro mehrfach in Darstellungsart | Makro mehrmals gleich platziert | Überzählige löschen | nicht relevant |
| 028026 | Rohrleitung/Kabel ohne Quell-/Zielsegment | Quell- oder Zielsegment fehlt | Segmente eintragen | nicht relevant |
| 028027 | Rohrleitungs-Quelle/Ziel passt nicht | Vertauscht oder ohne Vorplanungsobjekt | „Quelle und Ziel vertauschen“ prüfen, zuordnen | nicht relevant |
| 028028 | Stoff nicht als Vorlage | Stoff fehlt in Projektvorlagen | Vorlage anlegen | nicht relevant |
| 028029 | Rohrklasse nicht als Vorlage | Rohrklasse fehlt in Projektvorlagen | Vorlage anlegen | nicht relevant |
| 028030 | Rohrklasse Definitionspunkt ≠ Planungsobjekt | Rohrklassen verschieden | Angleichen | nicht relevant |
| 028031 | Segmentartikel nicht in Rohrklasse | Artikel gehört nicht zur Rohrklasse | Rohrklasse oder Artikel anpassen | nicht relevant |

## 029 Vorplanungstiefe (30 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_029nnn.htm – Nummern: 029001, 029002, 029003, 029004, 029005, 029006, 029007, 029008, 029009, 029010, 029011, 029012, 029013, 029014, 029015, 029016, 029017, 029018, 029019, 029020, 029021, 029022, 029023, 029024, 029025, 029026, 029027, 029028, 029029, 029030

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 029001 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029002 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029003 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029004 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029005 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029006 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029007 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029008 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029009 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029010 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029011 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029012 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029013 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029014 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029015 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029016 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029017 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029018 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029019 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029020 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029021 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029022 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029023 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029024 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029025 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029026 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029027 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029028 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029029 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |
| 029030 | Pflichteigenschaft leer | Für die Segmentdefinition festgelegte Pflichteigenschaft hat keinen Wert | Wert eintragen | nicht relevant |

## 030 Teilprojekte (8 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_030nnn.htm – Nummern: 030001, 030002, 030003, 030004, 030005, 030006, 030007, 030008

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 030001 | Original geändert im Teilprojekt-Bereich | Im Originalprojekt Seiten/Bauräume/Segmente im ausgelagerten Bereich angelegt (nur im Original gemeldet) | Dort löschen/verschieben, dann einlagern | nicht relevant |
| 030002 | Teilprojekt ändert außerhalb Bereich | Teilprojekt legt Objekte außerhalb seines Bearbeitungsbereichs an (nur im Teilprojekt gemeldet) | In den Bereich verschieben oder löschen | nicht relevant |
| 030003 | Weitere Teilprojekte ausgelagert | Aus Teilprojekt erneut ausgelagert; Einlagern löscht das Zwischenprojekt | In umgekehrter Reihenfolge einlagern | nicht relevant |
| 030004 | Seite passt nicht in Bearbeitungsbereich | Seite in Struktur außerhalb der Filterkriterien | Struktur anpassen | nicht relevant |
| 030005 | Bauraum passt nicht in Bearbeitungsbereich | Bauraum außerhalb der Filterkriterien | Struktur anpassen | nicht relevant |
| 030006 | Vorplanungsobjekt passt nicht in Bereich | Vorplanungsobjekt außerhalb der Filterkriterien | Struktur anpassen | nicht relevant |
| 030007 | Unplatzierte Funktion passt nicht in Bereich | Nicht platzierte Funktion außerhalb der Filterkriterien | Struktur anpassen | nicht relevant |
| 030008 | Unplatzierte Verbindung passt nicht in Bereich | Nicht platzierte Verbindung außerhalb der Filterkriterien | Struktur anpassen | nicht relevant |

## 032 Bohrbild (2 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_032nnn.htm – Nummern: 032001, 032002

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 032001 | Bohrbild nicht (vollständig) erzeugt | Kein Name, Block nicht aufgelöst, Kontur offen oder kein Durchbruch erkannt | Laut Platzhalter beheben | nicht relevant |
| 032002 | Bohrbildname mehrfach | Mehrere Bohrbilder gleichen Namens | Eindeutige Namen | nicht relevant |

## 033 Kabelbaum (3 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_033nnn.htm – Nummern: 033001, 033002, 033003

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 033001 | Funktion mit ungültigem Kabelbaumnamen | Kabelbaumname an Funktion ohne gleichnamige Kabelbaumdefinition (Kopie aus anderem Projekt, Umbenennung) | Namen löschen oder existierenden zuweisen | eigenes Skript **Regel**: jeder Verweis auf eine Gruppe (Kabel, Kabelbaum) braucht eine gleichnamige Definition im Projekt |
| 033002 | Verbindung mit ungültigem Kabelbaumnamen | Wie 033001, aber an einer Verbindung | Namen löschen oder existierenden zuweisen | eigenes Skript (wie 033001) |
| 033003 | Kabelbaum-Baugruppe ≠ Bestandteile | Artikel/Längen der Komponenten weichen von der Baugruppe ab, Bestandteil gelöscht oder ergänzt | Artikel und Längen abgleichen | nicht relevant |

## 501 Artikelstammdaten (32 Meldungen)
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_501nnn.htm – Nummern: 501001, 501002, 501003, 501004, 501005, 501006, 501007, 501008, 501009, 501010, 501011, 501012, 501013, 501014, 501015, 501016, 501017, 501018, 501019, 501020, 501022, 501023, 501024, 501025, 501026, 501027, 501028, 501029, 501030, 501031, 501032, 501033

Vorbemerkung: Diese Prüfläufe prüfen die Artikeldatenbank selbst, nicht den Plan. Für den Bauteil-Import aus EPLAN-Makros/EDZ in QET-Elemente sind sie eine Checkliste, welche Felder ein vollständiger Artikel haben muss und welche Konsistenzen zwischen Artikel, Funktionsschablonen, Makro und Anschlussbild gelten.

| Nr. | Kurztitel | Ursache (knapp) | Korrektur im Plan | QET-Prüfweg |
|---|---|---|---|---|
| 501001 | Am Artikel eingetragenes Makro fehlt | Makro, Grafikmakro oder Symbolmakro des Artikels ist nicht auffindbar. | Pfad korrigieren, Makro anlegen oder Eintrag löschen. | eigenes Skript **Regel: beim EDZ-Import Makroverweis des Artikels auflösen; fehlt er, Artikel als „ohne Makro“ melden** |
| 501002 | Bilddatei fehlt | Am Artikel hinterlegte Bilddatei nicht auffindbar. | Bild neu zuordnen oder Eintrag löschen. | eigenes Skript |
| 501003 | Dokument fehlt | Verlinktes Dokument (Datenblatt) nicht auffindbar. | Pfad/Hyperlink korrigieren oder löschen. | eigenes Skript |
| 501004 | Veraltete Funktionsdefinition | Funktionsschablone nutzt nicht mehr zulässige Funktionsdefinition. | Aktuelle Funktionsdefinition wählen. | eigenes Skript **Regel: Funktionsdefinition der Schablone auf aktuelle QET-Kategorie abbilden** |
| 501005 | Makro mit benutzerdefinierter Ebene | Makro des Artikels liegt auf benutzerdefinierter statt EPLAN-Ebene. | Makro mit EPLAN-Ebenen verwenden. | nicht relevant |
| 501006 | Makrobestandteile nicht gruppiert | 2D-Makro des Artikels nicht gruppiert; 3D und Makros mit unplatzierten Funktionen ausgenommen. | Gruppiertes Makro verwenden/erstellen. | nicht relevant |
| 501007 | Höhe/Breite/Tiefe fehlt | Montagemaße unvollständig; mindestens zwei von drei nötig, bei 3D-Makro alle drei. | Maße ergänzen. | eigenes Skript |
| 501008 | Anschlussbild fehlt | Eingetragenes Anschlussbild nicht vorhanden. | Anschlussbild zuordnen. | eigenes Skript |
| 501009 | Bohrbild fehlt | Eingetragenes Bohrbild (Fertigung) nicht vorhanden. | Bohrbild zuordnen. | nicht relevant |
| 501010 | Artikel der Baugruppe fehlt | Unterartikel einer Baugruppe/eines Moduls existiert nicht als Datensatz. | Unterartikel anlegen/importieren und zuordnen. | eigenes Skript **Regel: Modul-/Baugruppen-Unterartikel beim Import mitziehen** |
| 501011 | Typnummer leer | Artikel ohne Typnummer. | Typnummer eintragen. | eigenes Skript **Regel: Typnummer Pflichtfeld im importierten Element** |
| 501012 | Bestellnummer leer | Artikel ohne Bestellnummer. | Bestellnummer eintragen. | eigenes Skript **Regel: Bestellnummer Pflichtfeld** |
| 501013 | Bezeichnung 1 leer | Artikel ohne Bezeichnung 1. | Bezeichnung eintragen. | eigenes Skript **Regel: Bezeichnung Pflichtfeld** |
| 501014 | Produktobergruppe undefiniert | Produktobergruppe steht auf „Undefiniert“. | Passende Obergruppe wählen. | eigenes Skript **Regel: Produktgruppen bestimmen Ablageordner Hersteller/Kategorie/Typ** |
| 501015 | Produktgruppe undefiniert | Produktgruppe steht auf „Undefiniert“. | Passende Produktgruppe wählen. | eigenes Skript |
| 501016 | Produktuntergruppe undefiniert | Produktuntergruppe steht auf „Undefiniert“. | Passende Untergruppe wählen. | eigenes Skript |
| 501017 | Kein Gewerk | Artikel ohne Gewerk/Untergewerk. | Gewerk zuordnen. | nicht relevant |
| 501018 | Breite/Höhe ≠ Makroabmessung | 2D-Fenstermakro (nur Variante A geprüft) weicht über Toleranz von Artikelmaßen ab. | Artikelmaße an Makro angleichen. | nicht relevant |
| 501019 | Symbolbibliothek des Symbolmakros fehlt | Symbolmakro nutzt Bibliothek, die im Prüfprojekt nicht eingelagert ist (oft Nicht-Standardbibliothek). | Makro aus Standardsymbolen bauen oder Bibliothek einlagern. | eigenes Skript **Regel: fremde Symbolbibliotheken im Makro auf QET-IEC-Symbol abbilden** |
| 501020 | Zubehörartikel fehlt | Artikel in Zubehörliste existiert nicht als Datensatz. | Zubehörartikel anlegen, als Zubehör kennzeichnen, Liste zuordnen. | nicht relevant |
| 501022 | Anschlussmaße Schablone ≠ Anschlussbild | Anschlussmaße in Funktionsschablonen und Anschlussbild verschieden. | Maße im Anschlussbild an Schablonen angleichen. | nicht relevant |
| 501023 | Funktionsschablonen der Baugruppe veraltet | Summierte Schablonen eines Moduls/einer Baugruppe passen nicht mehr zu den Unterartikeln. | Funktionsschablonen neu summieren. | nicht relevant |
| 501024 | Artikelnummer mit Leerzeichen | Führendes/nachfolgendes Leerzeichen in der Artikelnummer. | Leerzeichen entfernen. | eigenes Skript **Regel: Artikelnummer beim Import trimmen** |
| 501025 | SPS-Typ/Gerätebeschreibung leer | SPS-Artikel (oder Artikel mit Bus-Kabelanschluss-Schablone) ohne SPS-Typbezeichnung bzw. GSD-Dateiname. | Eines der Felder füllen. | eigenes Skript |
| 501026 | Bus-Anschluss Schablone ≠ Makro (einpolig) | Bus-Anschluss der Schablone weicht im Makro in Schnittstellenname, Bus-System, Funktionsdefinition oder Stecker ab. | Werte in Makro oder Artikel angleichen. | eigenes Skript |
| 501027 | Anschlussdaten am Bus-Anschluss gefüllt | Netzwerk-/Bus-Kabelanschluss-Schablone trägt Anschlussbezeichnung/-beschreibung/-querschnitt/-maß. | Diese Anschlussdaten an Bus-Schablonen leeren. | nicht relevant |
| 501028 | Steckerbezeichnung am Bus-Anschluss leer | Bus-Kabelanschluss-Schablone ohne Steckerbezeichnung. | Steckerbezeichnung eintragen. | eigenes Skript |
| 501029 | Modulposition ohne BMK/BMK-ID | Unterartikel eines Moduls hat weder BMK noch BMK-ID. | BMK oder BMK-ID am Unterartikel vergeben. | eigenes Skript |
| 501030 | Bohrbild-Kontur fehlt | Bohrbild verweist auf nicht vorhandene Kontur. | Vorhandene Kontur eintragen. | nicht relevant |
| 501031 | Allgemeine Funktionsdefinition am Artikel | Klemmenartikel nutzt „allgemeine“ statt genauer Funktionsdefinition (nur Klemmen gemeldet). | Genaue Funktionsdefinition (z. B. Durchgangs-, PE-, Mehrstockklemme) wählen. | eigenes Skript **Regel: Klemmen-Element mit konkretem Klemmentyp anlegen, nicht „allgemein“** |
| 501032 | Kanalbezeichnung in Schablone fehlerhaft | SPS-Artikel: Kanalbezeichnung nicht rein numerisch oder mit Leerzeichen. | Reine Zahlen ohne Leerzeichen eintragen. | eigenes Skript **Regel: Kanalnummern aus EDZ-Schablonen numerisch und getrimmt übernehmen** |
| 501033 | Anschlussbezeichnungen Schablone ≠ Anschlussbild | Identifizierende Anschlussbezeichnungen der Schablonen passen nicht zum Anschlussbild (dort je Anschluss eine Zeile). | Anschlussbild korrigieren. | eigenes Skript **Regel: Anschlussnamen des QET-Elements = Anschlussbezeichnungen der Funktionsschablonen** |

## Quellen

Alle Seiten nach dem Schema `https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_<Nr>.htm` (586 Seiten), ausgewertet am 2026-11-05 aus dem lokal bereinigten Text der Hilfe 2.9. Nummern je Klasse:

- **001** (25): 001001, 001005, 001008, 001010, 001011, 001012, 001015, 001016, 001021, 001022, 001023, 001024, 001025, 001026, 001027, 001028, 001029, 001030, 001031, 001032, 001033, 001034, 001035, 001036, 001037
- **002** (14): 002001, 002002, 002003, 002004, 002005, 002006, 002007, 002008, 002009, 002010, 002012, 002013, 002014, 002015
- **003** (21): 003001, 003002, 003003, 003005, 003006, 003007, 003008, 003009, 003011, 003012, 003013, 003014, 003018, 003019, 003022, 003023, 003024, 003025, 003026, 003027, 003028
- **004** (78): 004003, 004004, 004005, 004006, 004007, 004009, 004010, 004013, 004016, 004018, 004019, 004020, 004021, 004022, 004023, 004024, 004025, 004027, 004028, 004029, 004030, 004031, 004035, 004036, 004037, 004038, 004039, 004040, 004045, 004046, 004050, 004051, 004052, 004053, 004054, 004055, 004056, 004057, 004058, 004070, 004071, 004072, 004073, 004080, 004081, 004082, 004083, 004084, 004085, 004086, 004087, 004088, 004089, 004090, 004091, 004092, 004093, 004094, 004095, 004096, 004097, 004098, 004099, 004100, 004101, 004102, 004103, 004104, 004106, 004107, 004108, 004109, 004110, 004111, 004200, 004201, 004202, 004203
- **005** (78): 005001, 005010, 005012, 005013, 005014, 005015, 005016, 005017, 005018, 005019, 005020, 005021, 005022, 005023, 005024, 005025, 005026, 005027, 005028, 005031, 005033, 005034, 005035, 005036, 005037, 005039, 005040, 005042, 005043, 005044, 005045, 005046, 005047, 005048, 005049, 005050, 005051, 005052, 005053, 005054, 005055, 005056, 005057, 005058, 005060, 005061, 005062, 005063, 005064, 005065, 005066, 005067, 005068, 005069, 005070, 005071, 005072, 005073, 005074, 005075, 005076, 005077, 005078, 005079, 005080, 005081, 005082, 005083, 005084, 005085, 005086, 005087, 005088, 005089, 005090, 005091, 005092, 005093
- **007** (33): 007001, 007002, 007003, 007004, 007005, 007006, 007007, 007008, 007009, 007010, 007011, 007012, 007013, 007014, 007015, 007016, 007017, 007018, 007019, 007020, 007021, 007022, 007023, 007024, 007025, 007026, 007027, 007028, 007029, 007030, 007031, 007032, 007033
- **008** (5): 008001, 008002, 008003, 008004, 008005
- **010** (2): 010001, 010002
- **011** (8): 011001, 011002, 011003, 011004, 011005, 011006, 011007, 011008
- **012** (9): 012001, 012002, 012003, 012004, 012005, 012008, 012009, 012010, 012011
- **013** (9): 013045, 013046, 013047, 013048, 013050, 013051, 013053, 013054, 013055
- **014** (6): 014001, 014002, 014003, 014004, 014005, 014006
- **015** (4): 015001, 015002, 015003, 015004
- **016** (2): 016001, 016002
- **017** (13): 017001, 017002, 017003, 017004, 017005, 017006, 017007, 017008, 017009, 017010, 017011, 017012, 017013
- **020** (37): 020001, 020003, 020004, 020010, 020011, 020012, 020013, 020014, 020015, 020016, 020017, 020018, 020026, 020027, 020028, 020029, 020030, 020031, 020032, 020033, 020034, 020035, 020036, 020037, 020038, 020039, 020040, 020041, 020042, 020043, 020044, 020045, 020046, 020047, 020048, 020049, 020050
- **021** (5): 021001, 021002, 021003, 021004, 021005
- **022** (20): 022001, 022002, 022003, 022004, 022011, 022012, 022013, 022014, 022015, 022016, 022017, 022018, 022019, 022020, 022025, 022026, 022027, 022050, 022060, 022061
- **024** (18): 024001, 024002, 024003, 024004, 024005, 024006, 024007, 024008, 024009, 024010, 024011, 024012, 024013, 024014, 024015, 024016, 024017, 024018
- **025** (1): 025001
- **026** (69): 026010, 026011, 026012, 026013, 026014, 026015, 026016, 026017, 026018, 026019, 026020, 026021, 026023, 026024, 026025, 026026, 026027, 026028, 026029, 026030, 026033, 026034, 026036, 026037, 026038, 026039, 026040, 026041, 026042, 026043, 026044, 026045, 026052, 026053, 026054, 026055, 026056, 026057, 026058, 026062, 026070, 026071, 026072, 026073, 026074, 026075, 026076, 026077, 026078, 026079, 026080, 026081, 026089, 026090, 026091, 026092, 026093, 026094, 026095, 026096, 026097, 026098, 026099, 026100, 026101, 026102, 026103, 026104, 026105
- **027** (23): 027002, 027003, 027004, 027007, 027009, 027010, 027012, 027013, 027014, 027015, 027016, 027017, 027018, 027019, 027020, 027021, 027024, 027025, 027026, 027027, 027028, 027029, 027030
- **028** (31): 028001, 028002, 028003, 028004, 028005, 028006, 028007, 028008, 028009, 028010, 028011, 028012, 028013, 028014, 028015, 028016, 028017, 028018, 028019, 028020, 028021, 028022, 028023, 028024, 028025, 028026, 028027, 028028, 028029, 028030, 028031
- **029** (30): 029001, 029002, 029003, 029004, 029005, 029006, 029007, 029008, 029009, 029010, 029011, 029012, 029013, 029014, 029015, 029016, 029017, 029018, 029019, 029020, 029021, 029022, 029023, 029024, 029025, 029026, 029027, 029028, 029029, 029030
- **030** (8): 030001, 030002, 030003, 030004, 030005, 030006, 030007, 030008
- **032** (2): 032001, 032002
- **033** (3): 033001, 033002, 033003
- **501** (32): 501001, 501002, 501003, 501004, 501005, 501006, 501007, 501008, 501009, 501010, 501011, 501012, 501013, 501014, 501015, 501016, 501017, 501018, 501019, 501020, 501022, 501023, 501024, 501025, 501026, 501027, 501028, 501029, 501030, 501031, 501032, 501033
