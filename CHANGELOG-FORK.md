# Änderungen im Fork

Neuere Versionen stehen oben. Das Schema erklärt [VERSIONIERUNG.md](VERSIONIERUNG.md). Die Änderungen des
Original-QElectroTech stehen in `ChangeLog*`.

## 1.2.0 – 11.11.2026

- Neuer Knopf „Claude“ (Symbol und Text) ganz vorn in der Hauptwerkzeugleiste. Er öffnet und schließt den
  Claude-Assistenten rechts. Wer seine Hauptwerkzeugleiste schon angepasst hatte, bekommt den Knopf einmal
  vorn eingefügt; danach lässt er sich wie jeder andere Knopf verschieben oder entfernen.
- Geführte Einrichtung im Assistenten statt des Einstellungsdialogs: API-Schlüssel (mit Link zu
  console.anthropic.com und „Prüfen“, das den Schlüssel mit einer kurzen, kostenlosen Anfrage testet und erst dann
  in der Windows-Anmeldeinformationsverwaltung speichert), Python und das Paket „anthropic“ (Knopf
  „Jetzt installieren“ ruft pip auf und zeigt die Ausgabe). Jeder Schritt bekommt einen grünen Haken oder ein rotes
  Kreuz; sind alle grün, wechselt das Fenster von selbst zum Chat. „Einstellungen“ im Chat führt zurück.

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
