# Weitergabe in der Firma (SharePoint)

Kollegen bekommen den Fork ohne Git und ohne GitHub-Konto über eine SharePoint-Bibliothek, die OneDrive auf
jeden Rechner spiegelt. Das Programm findet den Ordner selbst, sucht beim Start nach Updates, legt
Fehler- und Wunschmeldungen dort ab und nimmt die Firmen-Bauteile von dort.

## Aufbau des Verteilordners

```
QET-Fork\                     (SharePoint-Bibliothek „QET-Fork“, per OneDrive synchronisiert)
  version.json                {"product": "QET-Fork", "version": "1.31.0", "package": "Programm/QET-Fork-1.31.0.zip", "notes": "…"}
  Installieren.cmd            Doppelklick: installiert oder aktualisiert
  Installieren.ps1
  Programm\QET-Fork-<Version>.zip
  Bauteile\                   Firmensammlung (nur lesen)
  Meldungen\                  Fehler und Wünsche der Kollegen (schreiben erlaubt)
```

## Einmalig einrichten (du)

1. In SharePoint eine Website oder Bibliothek „QET-Fork“ anlegen.
2. Rechte: Kollegen **Lesen** für die Bibliothek, **Bearbeiten** nur für den Ordner `Meldungen`; du hast
   Vollzugriff.
3. Die Bibliothek mit „Synchronisieren“ auf deinem Rechner einbinden. OneDrive legt sie unter
   `C:\Users\<Name>\<Firma>\<Website> - QET-Fork` (o. ä.) ab.
4. Erste Veröffentlichung (siehe unten) mit `-Ziel` auf diesen Ordner.

## Neue Version veröffentlichen (du)

Nach Build, `tools\deploy_lokal.ps1` und den Prüfungen:

```
powershell -File tools\verteilung\veroeffentlichen.ps1 -Ziel "<synchronisierter Ordner>" -Hinweis "Was neu ist"
```

Das Skript packt `build\deploy` (ohne alte Test-Exes) nach `Programm\`, legt die Installierskripte daneben,
spiegelt die Firmensammlung nach `Bauteile\` (ohne `.git`; Quelle aus der QET-Einstellung oder `-Bauteile`)
und schreibt `version.json` zuletzt. Erst dann sehen die Kollegen das Update. Ohne `-Ziel` nimmt es den
Verteilordner aus den QET-Einstellungen. `-OhneBauteile` lässt die Bauteile aus. Es bleiben die letzten drei
Programmpakete.

## Kollegen

1. Die Bibliothek „QET-Fork“ in SharePoint öffnen und „Synchronisieren“ wählen.
2. Im synchronisierten Ordner `Installieren.cmd` doppelklicken. Installiert wird nach
   `%LOCALAPPDATA%\QET-Fork\Programm` (keine Adminrechte), mit Verknüpfung „QElectroTech (Firma)“ im
   Startmenü und auf dem Desktop.
3. Beim Start prüft das Programm `version.json`. Bei einer neueren Version: „Jetzt installieren“ schließt
   QElectroTech (mit Rückfrage für ungespeicherte Projekte), installiert und startet neu; „Später“ fragt beim
   nächsten Start wieder, „Diese Version überspringen“ erst bei der folgenden. Hilfe > *Nach Updates suchen...*
   prüft von Hand.
4. Hilfe > *Fehler oder Wunsch melden...*: Art, Titel, Text, auf Wunsch Bildschirmfoto und Diagnosebericht.
   Die Meldung landet als Ordner `Meldungen\<Datum>_<Fehler|Wunsch>_<Name>\` (meldung.txt, bild.png,
   diagnose.txt). Ohne Verbindung bleibt sie lokal und wird beim nächsten Start nachgereicht.
5. Die Firmensammlung ist `Bauteile\` im Verteilordner (OneDrive hält sie dauerhaft auf dem Rechner). Wer in
   QElectroTech eine eigene Firmensammlung eingestellt hat, behält sie.

## Wie das Programm den Ordner findet

Einstellung `distribution/folder` (schreibt `Installieren.ps1`), sonst Umgebungsvariable
`QET_DISTRIBUTION_DIR`, sonst die von OneDrive synchronisierten Ordner (Registry
`HKCU\Software\SyncEngines\Providers\OneDrive`) und die Ordner im Benutzerprofil bis zwei Ebenen tief. Gilt
nur ein Ordner mit `version.json` und `"product": "QET-Fork"`. Ohne Verteilordner läuft das Programm wie
bisher.

## Grenzen

- Ohne OneDrive-Synchronisation (oder bevor sie fertig ist) gibt es keine Updates und keine Meldungen an den
  Ordner; das Programm läuft trotzdem.
- Die Bauteile sind für Kollegen nur zum Lesen. Neue Bauteile trägst du in deine Sammlung ein und
  veröffentlichst sie.
- Die Programmpakete sind groß (mehrere hundert MB); OneDrive lädt sie erst beim Installieren.
- Keine Code-Signatur: Windows SmartScreen kann beim ersten Start nachfragen.
