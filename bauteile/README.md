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

`fluid/` enthält die Pneumatik- und Hydrauliksymbole der QElectroTech-Standardsammlung (437 Bauteile, Grafik
unverändert, Lizenz wie `elements/ELEMENTS.LICENSE`), erweitert um Simulationsdaten:
- benannte Anschlüsse mit Medium (aus der Beschriftung am Anschluss, sonst nach Norm aus der Lage);
- `<fluid kind=… function=…>` mit Kennbuchstaben und Kennwerten (Startwerte, z. B. Öffnungsdruck, Durchfluss,
  Kolbendurchmesser, Hub);
- bei Wegeventilen Stellungen und Schaltwege (aus den gezeichneten Pfeilen, Sperrzeichen und Verbindungen jedes
  Kästchens gelesen), Ruhestellung und Betätigung links/rechts.

Die Pfade sind dieselben wie in `elements/` (Submodul, unverändert). `tools/deploy_lokal.ps1`, die
CMake-Installation und das Mac-Bundle legen `fluid/` über die Standardsammlung und ersetzen so die Originale.
Neu erzeugen aus der Standardsammlung:

    python tools/symbole/qet_fluid_daten.py --bericht bericht.csv

Drei Hydraulikventile (`distributeur_2-2`, `electrovanne_2_2_nf`, `distributeur_4-2-centre_ouvert`) sind an der
Zeichnung abgelesen (`VON_HAND` im Werkzeug); bei den 2/2-Ventilen heißen die Anschlüsse am betätigten Kästchen
`P.2`/`A.2` und gehören zu keinem Schaltweg.

Die FluidSIM-Symbole (Festo Didactic) gehören nicht in dieses Repository; sie liegen aufbereitet in der
Firmensammlung (`tools/symbole/fluidsim_symbole.py`, `fluid_raster_einpassen.py`, `fluid_schaltwege.py`,
`fluid_kennwerte.py`).

`pneumatik/` enthält Beispiele für Pneumatikpläne (Branch `feature/pneumatik`): ein 5/2-Magnetventil mit
Federrückstellung als Master (Anschlüsse 1–5 pneumatisch, Schalldämpfer an 3 und 5) und die Ventilspule als
Slave für das elektrische Blatt. Spule und Ventil verknüpft man wie Spule und Kontakt.

Weitere Pneumatikbauteile entstehen per Skript. `tools/deploy_lokal.ps1` erledigt das bei jedem Deploy in der
Standardsammlung von `build\deploy`: `50_pneumatic` und `30_hydraulic` werden durch die nachgerüsteten Bauteile
ersetzt (gleiche Pfade). Von Hand:

    python tools/symbole/fluid_nachruesten.py elements/50_pneumatic --ziel <Datenordner>/elements/Pneumatik_nachgeruestet

Das rüstet die Pneumatikbauteile der QET-Sammlung mit Medium und Portnummern nach, damit sie sich mit den neuen pneumatischen
Anschlüssen verbinden lassen.
