# Zusätzliche Bauteile

`nachgezeichnet/` enthält Symbole für Teile, zu denen es keine EPLAN-Herstellerdaten gibt.
Sie sind nach vorhandenen Schaltplänen gezeichnet und werden mit
`tools/symbole/nachzeichnen_symbole.py` erzeugt:

    python tools/symbole/nachzeichnen_symbole.py --ziel bauteile/nachgezeichnet

Zum Benutzen den Ordner in die Benutzersammlung kopieren
(`%APPDATA%\QElectroTech\QElectroTech\elements`) oder dort neu erzeugen
(ohne `--ziel`). Ordner: Hersteller/Kategorie/Typ.

Mit „Bauteil-Import“ eingelesene Herstellerdaten (EPLAN Data Portal) gehören
nicht in dieses Repository.
