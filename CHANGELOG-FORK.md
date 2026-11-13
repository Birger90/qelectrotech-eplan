# Änderungen im Fork

Neuere Versionen stehen oben. Das Schema erklärt [VERSIONIERUNG.md](VERSIONIERUNG.md). Die Änderungen des
Original-QElectroTech stehen in `ChangeLog*`.

## 1.9.1 – 13.11.2026

- qet-mcp: Syntaxfehler in der Beschreibung von `add_terminals` behoben (der MCP-Server startete seit 1.9.0 nicht).
- Abgleich mit dem Original-QElectroTech (Stand 13.11.2026): Strukturkästen, Ansichten für Betriebsmittel, Kabel,
  Klemmen, Potentiale und SPS-E/A, Artikel-Register am Bauteil, PDF-Ränder unter Windows u. a.
- Kabelliste: die Korrektur „Kabelende an der Klemmleiste“ kommt jetzt aus dem Original.

## 1.9.0 – 13.11.2026

- Neues Werkzeug „Klemmleiste zeichnen“ (Werkzeugleiste neben „Kabel zeichnen“): ein Strich quer über Leitungen
  setzt auf jede Leitung eine Klemme, trennt die Leitung dort auf und behält Drahtnummer und Farbe auf beiden
  Seiten. Der Vorschlag (Leiste, erste Nummer, Symbol, Artikel, PE-Artikel) lässt sich vor dem Einfügen ändern;
  die Klemmen kommen als `X1:n` in die Klemmleiste. Skript `qet.addTerminals(...)`, MCP-Operation `add_terminals`.
- Klemmenkatalog mit Filter (Hersteller, Art, Querschnitt, Anschlussart), maßstäblicher Vorschau, Produktfoto
  und Suche im EPLAN Data Portal. Eigene Kataloge als CSV in `<Datenordner>/terminal-catalogs/`.
- Mehrstockklemmen: eine Klemme mit mehreren Etagen (z. B. L/N/PE) nimmt mehrere Leitungen auf; die Etagen werden
  in der Klemmleiste zu einer Klemme zusammengefasst.
- Kabelliste: alle Klemmen einer Klemmleiste zählen als ein Kabelende (`X1` statt `X1:1` → `X1:2`).

## 1.8.0 – 12.11.2026

- Herstellerportale: Keyence. Benutzername und Passwort für keyence.de im Windows-Anmeldeinformationsspeicher
  (Dialog Herstellerportale, `--portal-status`, `--portal-login keyence`, `--portal-logout keyence`).
  keyence.de meldet nur über die eigene Webseite an, QElectroTech sendet das Passwort nicht.
- `--fetch-parts` übernimmt den CAD-Download aus dem Browser: ZIP oder STEP in `herstellerdaten/Keyence/<Nr>/`
  wird entpackt und als `3D/Keyence/<Nr>.stp` abgelegt (STEP aus ZIP-Archiven auch für andere Hersteller).
  Fehlt die Datei, nennt es den Suchlink auf keyence.de und den Zielordner. Herstellerkürzel `KEY`.

## 1.7.1 – 11.11.2026

- EPLAN-Import: symbolische Adressen wie EPLAN (Zielverfolgung bis Sensor/Aktor bzw. Steckerklemme ohne
  Weiterleitung, vollständiger Text; nur für SPS-Anschlüsse mit SPS-Adresse).
- EPLAN-Import: Herstellername (22222) aus dem Herstellersatz, BMK am SPS-Kanal nur, wo EPLAN es sichtbar gespeichert hat.
- EPLAN-Import: Pfad-Funktionstext über den Pfad (Spalte des Normblatts) statt über einen Abstand; Übersichten
  zeigen den Funktionstext des Gegenstücks.
- EPLAN-Import: Normblatt-Platzhalter ohne Zeilen, die unter den Blattrand laufen (wie EPLAN).

## 1.7.0 – 11.11.2026

- EPLAN-Import: Bilder aus Formularen (z. B. Firmenlogo auf dem Deckblatt) und Bilder mit der alten Pfadschreibweise
  `$IMG\…` kommen mit (Testprojekt: 201 Bilder, 0 nicht auflösbar).
- EPLAN-Import: Linienbreite wie in EPLAN (eigene Breite, sonst Breite der Ebene) für Blattgrafik und
  Bauteilgrafik; dicke Balken und Rahmen sind wieder dick.
- EPLAN-Import: fette und kursive Texte (Textformat 60965) bleiben fett/kursiv.

## 1.6.0 – 11.11.2026

- Claude-Assistent: Bilder und PDFs mitschicken. Ins Eingabefeld einfügen (Strg+V, z. B. ein Bildschirmfoto),
  Dateien hineinziehen oder mit der Büroklammer wählen; die Anhänge stehen als kleine Knöpfe über dem Feld und
  verschwinden per Klick wieder. Bilder werden auf höchstens 2000 px verkleinert (Fotos als JPEG), PDFs bis
  20 MB, höchstens 10 Anhänge je Frage. Geht mit API-Schlüssel und über das Claude-Konto.

## 1.5.0 – 11.11.2026

- EPLAN-Import: Auch Steuerungen anderer Hersteller (z. B. Cybelec) werden SPS-Master. Ihre Kanaladresse steht in
  EPLAN oft nicht als SPS-Adresse, sondern in der Anschlussbezeichnung bzw. -beschreibung (DO5.8, DI1.2); der
  Import liest sie von dort.
- EPLAN-Import: Funktionstexte der Kanäle (z. B. „Hand von CNC“) kommen mit – eigener Text, sonst der
  Pfad-Funktionstext wie in EPLAN – und stehen in der Kanaltabelle der Karte.
- Adresse, symbolische Adresse und Funktionstext eines verknüpften Kanals kommen nur aus der Kanaltabelle der Karte:
  Beim Wechsel auf einen anderen Kanal wandern die Texte mit.
- Nach jedem Import liegt `sps_bericht.txt` im Arbeitsordner: welche Kanäle nicht verknüpft sind und warum.

## 1.4.0 – 11.11.2026

- Claude-Assistent über das eigene Claude-Konto (Pro/Max) statt eines API-Schlüssels: keine API-Kosten, die
  Nutzung zählt gegen das Abo. Die Einrichtung hat dafür einen Knopf „Mit Claude verbinden“: Er installiert das
  Python-Paket `claude-agent-sdk` (es bringt sein Claude Code mit) und öffnet die Claude-Anmeldung im Browser.
  Danach wechselt das Fenster zum Chat. Die Brücke startet dazu mit `--konto`; Claude bekommt nur die
  QET-Werkzeuge, keine Datei-, Shell- oder Webwerkzeuge, und ein API-Schlüssel aus der Umgebung wird
  ausgeblendet. „Stattdessen einen API-Schlüssel verwenden“ schaltet zurück.
- Nur in eigenen Builds mit `-DQET_CLAUDE_KONTO=ON` (dort voreingestellt): Anthropic erlaubt Programmen nicht
  ohne Freigabe, die claude.ai-Anmeldung anzubieten. Ohne den Schalter bleibt es beim API-Schlüssel.

## 1.3.0 – 11.11.2026

- EPLAN-Import: SPS-Karten werden SPS-Master mit Kanaltabelle (Adresse, Funktionstext, symbolische Adresse,
  Klemme je Kanal, Art DI/DQ/AI/AQ aus EPLAN), SPS-Kanäle werden SPS-Slaves und sind beim Import schon mit
  ihrer Karte und Kanalzeile verknüpft. Damit lässt sich ein importierter Kanal im Fenster Auswahleigenschaften
  > SPS auf einen anderen freien Kanal legen. Kanäle ohne Karte im Projekt bleiben SPS-Slaves zum Verknüpfen
  von Hand. Versorgungsanschlüsse ohne Adresse (L+, M, 1A …) stehen nicht in der Kanaltabelle.

## 1.2.0 – 11.11.2026

- Neuer Knopf „Claude“ (Symbol und Text) ganz vorn in der Hauptwerkzeugleiste. Er öffnet und schließt den
  Claude-Assistenten rechts. Wer seine Hauptwerkzeugleiste schon angepasst hatte, bekommt den Knopf einmal
  vorn eingefügt; danach lässt er sich wie jeder andere Knopf verschieben oder entfernen.
- Geführte Einrichtung im Assistenten statt des Einstellungsdialogs: API-Schlüssel (mit Link zu
  console.anthropic.com und „Prüfen“, das den Schlüssel mit einer kurzen, kostenlosen Anfrage testet und erst dann
  in der Windows-Anmeldeinformationsverwaltung speichert), Python und das Paket „anthropic“ (Knopf
  „Jetzt installieren“ ruft pip auf und zeigt die Ausgabe). Jeder Schritt bekommt einen grünen Haken oder ein rotes
  Kreuz; sind alle grün, wechselt das Fenster von selbst zum Chat. „Einstellungen“ im Chat führt zurück.

## 1.1.2 – 11.11.2026

- EPLAN-Import: Ein SPS-Kanal ist jetzt EIN Bauteil – Kanalkasten, Kopfzeile (Karte, Typ), Anschluss, Klemme,
  Kanal, Adresse, symbolische Adresse und Funktionstext zusammen. Vorher kam nur ein Anschlusspunkt als Bauteil,
  der Rest als lose Texte und Formen aufs Blatt. Gleiches für Gerätekästen/SPS-Kartenübersichten (Black Box):
  die Texte ihrer Zeilen stehen im Bauteil.

## 1.1.1 – 11.11.2026

- Das Claude-Fenster heißt jetzt „Claude-Assistent“ (Ansicht > Claude-Assistent), das Protokoll des Direktmodus
  „Direktmodus“. Vorher hießen beide „Assistent“ und hatten dieselbe Kennung, sodass QET ihre Plätze beim Start
  vertauschen konnte.

## 1.1.0 – 11.11.2026

- Datei > EPLAN-Projekt importieren zeigt einen grünen Fortschrittsbalken (0–100 %) und den laufenden Schritt.
  Das Import-Programm meldet dazu Zeilen `FORTSCHRITT <prozent> <schritt>`.
- Deutsche Oberfläche vollständig: alle 913 offenen Texte übersetzt (EPLAN-Import, Assistent, Data Portal,
  Gerätenavigator, Brücken u. a.), drei falsche alte Übersetzungen korrigiert.
- `tools/deploy_lokal.ps1` bringt Programm, Übersetzungen und Python-Teile nach build\deploy.
  Vorher kam nur die exe dorthin, deshalb waren neue Fenster englisch.

## 1.0.1 – 11.11.2026

- Die exe trägt wieder das QElectroTech-Programmsymbol (Explorer, Desktop-Verknüpfung, Taskleiste).

## 1.0.0 – 10.11.2026

Erste gezählte Version. Sie fasst alle Fork-Funktionen zusammen, die vorher in einzelnen Branches lagen,
auf dem Stand von QElectroTech 0.200.1-dev.

**Neu in dieser Version**
- Eingebauter Claude-Assistent als Andockfenster (Ansicht > Assistent). Er arbeitet über `mcp/qet-assistent/bruecke.py`,
  der Schlüssel liegt in der Windows-Anmeldeinformationsverwaltung.
- Datei > EPLAN-Projekt importieren (.zw1). Ein Dialog startet den Import und öffnet das Ergebnis.
- SPS-Kanäle: Modul und Kanal im Fenster Auswahleigenschaften wählen, allgemeiner Ein-/Ausgangskanal. SPS-Module
  bekommen ihre Kanaltabelle schon beim Import.
- Die Fork-Version steht im Info-Dialog und bei `--version`.

**Bauteil-Import**
- EPLAN-Makros und EDZ-Pakete: Grafik aus DXF, Schaltplan aus `.ema`, Anschlüsse und Artikeldaten, 2D-Montageaufbau
  als `<Typ>_Aufbau.elmt`.
- DWG-Zeichnungen (AutoCAD 2004–2018) direkt, Artikeldaten aus WSCAD `.wspak`.
- EPLAN Data Portal: Suche, Filter, Bildvorschau, Import, auch per Skript (`importPartFromDataPortal`).
- Herstellerportale: Konten im Windows-Anmeldeinformationsspeicher, fehlende Bauteile automatisch holen
  (`--fetch-parts`).

**Schaltplan wie EPLAN**
- Dreifachsprung am Bauteil und im PDF, Sprungziele für Texte und Bauteile, verknüpfte Folioverweise als Link.
- Gefüllte Folioverweis-Pfeile, Gerätenavigator mit Filterschemata, SPS-E/A-Tabelle, Kontaktgruppen.
- Kabeltyp-Fenster mit Herstellerkatalogen, Typ eines platzierten Elements tauschen.

**Schaltschrankaufbau**
- Montageflächen, Hutschienen, Kabelkanäle, Klemmenleisten, Tür und Seitenwände, Bohrbild, DXF-Fertigungsexport.
- 3D-Ansicht mit STEP-Modellen der Hersteller, glTF-Export, Aufbauprüfung, Aufbau aus EPLAN-Projekten.

**Werkzeuge und Skills**
- MCP-Server `qet_mcp` mit Live-Modus, Skript-API, Data-Portal-Suche.
- Skills `qet-schaltplan-zeichnen`, `eplan-wissen` und `bauteile-herunterladen` unter `.claude/skills/`.
