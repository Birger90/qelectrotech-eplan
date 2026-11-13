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

`pneumatik/` enthält Beispiele für Pneumatikpläne (Branch `feature/pneumatik`): ein 5/2-Magnetventil mit
Federrückstellung als Master (Anschlüsse 1–5 pneumatisch, Schalldämpfer an 3 und 5) und die Ventilspule als
Slave für das elektrische Blatt. Spule und Ventil verknüpft man wie Spule und Kontakt.

Weitere Pneumatikbauteile entstehen per Skript. `tools/deploy_lokal.ps1` erledigt das bei jedem Deploy in der
Standardsammlung von `build\deploy`: `50_pneumatic` und `30_hydraulic` werden durch die nachgerüsteten Bauteile
ersetzt (gleiche Pfade), die FluidSIM-Symbole kommen nach `50_pneumatic/5090_FluidSIM`. Von Hand:

    python tools/symbole/fluidsim_symbole.py --fluidsim "C:/Program Files (x86)/Didactic/FluidSIM6" --ziel <Datenordner>/elements/FluidSIM
    python tools/symbole/fluid_nachruesten.py elements/50_pneumatic --ziel <Datenordner>/elements/Pneumatik_nachgeruestet

Das erste braucht eine FluidSIM-6-Installation und wandelt deren Bibliothek um; das zweite rüstet die
Pneumatikbauteile der QET-Sammlung mit Medium und Portnummern nach, damit sie sich mit den neuen pneumatischen
Anschlüssen verbinden lassen.
