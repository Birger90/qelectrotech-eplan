# Eingebauter Assistent (Claude)

Andockfenster **Anzeige > Assistent** in QElectroTech. Man fragt etwas zum offenen
Plan oder sagt, was gezeichnet werden soll; Claude arbeitet mit den Werkzeugen aus
`misc/qet-mcp/qet_mcp.py` direkt am offenen Projekt.

## Aufbau

- `sources/assistant/assistantdock.*` – das Andockfenster (Verlauf, Eingabe,
  Abbrechen, Anzeige des gerade benutzten Werkzeugs).
- `sources/assistant/assistantbridge.*` – startet `bruecke.py` als QProcess,
  JSON-Zeilen über stdin/stdout (Nachrichten: siehe Kopf von `bruecke.py`).
- `sources/assistant/assistantsettingsdialog.*` – API-Schlüssel und Python.
- `bruecke.py` – Anthropic-SDK, Modell `claude-opus-5-5`, adaptives Denken,
  Streaming, Server-Fallback bei Ablehnungen, Prompt-Cache.
- `sources/scripting/liveserver.*` – eigener Kanal für den Assistenten (eigenes
  Token, nicht in `qet-assistant.json`, unabhängig von der Live-Modus-Einstellung).

## Sicherheit

- Der Schlüssel liegt in der Windows-Anmeldeinformationsverwaltung
  (`QElectroTech/portal/claude`), QET gibt ihn nur dem Unterprozess in
  `ANTHROPIC_API_KEY` mit. Er wird nirgends protokolliert.
- Jede Änderung von Claude ist ein eigener Rückgängig-Schritt (ein Skript-Lauf
  über den Live-Kanal).
- Vor der ersten Änderung eines Gesprächs fragt QET nach. Erst nach dem „Ja“
  lässt der Live-Kanal schreibende Befehle des Assistenten überhaupt zu
  (`LiveServer::setAssistantWritesAllowed`), „Neu“ nimmt das „Ja“ zurück.
- Speichern, Öffnen, Schließen und Drucken lehnt der Kanal für den Assistenten ab.

## Voraussetzungen

- Python 3 mit `py -m pip install anthropic`
- API-Schlüssel von console.anthropic.com (Assistent > Einstellungen…)

## Test ohne Oberfläche

```
set QET_BINARY=...\build\deploy\QElectroTech.exe
python misc\qet-assistent\test_bruecke.py
```

Ohne `ANTHROPIC_API_KEY` läuft die Kette mit einer Attrappe (feste Antworten):
Frage → Status → Rückfrage → Skript auf einer Projektdatei (`--datei`) → Antwort,
und die Ablehnung ändert nichts. Mit Schlüssel fragt ein weiterer Test das echte
Modell und prüft, dass ein Element gesetzt wurde.
