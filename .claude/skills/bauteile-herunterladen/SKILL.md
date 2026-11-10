---
name: bauteile-herunterladen
description: Herstellerdaten (EPLAN-Makro EDZ, DXF-Maßbild, STEP-3D) für fehlende Bauteile holen und in die Firmensammlung bzw. Daten\3D einsortieren – Siemens SiePortal, Rittal, 3Dfindit, SEW, WAGO. Immer verwenden, wenn Bauteile/Komponenten „fehlen“, „holen“, „herunterladen“, „in die Datenbank“, wenn eine Fehlliste (Fehlende_Komponenten*.txt) abgearbeitet wird oder Makros/DXF/STEP zu Bestellnummern gesucht werden.
---

# Bauteile herunterladen (EDZ / DXF / STEP)

Ziel: zu jeder Bestellnummer Schaltbild + Aufbau (DXF) + 3D (STEP) in den eigenen Daten.
Reihenfolge je Artikel: **EPLAN-Makro (EDZ)** > DXF-Maßbild > nur STEP. Die EDZ enthält Artikeldaten,
Schaltbild- und Aufbau-Makro; der Fork importiert sie vollständig.

## Ablage (fest)

- Firmensammlung (Schaltbild, `*_Aufbau.elmt`): `Code Projekte\QElectroTech\Daten\elements-company\<Hersteller>\<Kategorie>\<Bestellnr>\`
- Rohdateien (EDZ/DXF/STP wie geladen): `Daten\herstellerdaten\<Hersteller>\<Bestellnr>\` (nicht ins öffentliche Repo)
- STEP für den Schaltschrank: `Daten\3D\<Hersteller>\<Bestellnummer mit Bindestrichen>.stp`; Varianten gleicher Bauform in
  `Daten\3D\gleiche_modelle.txt` (`3RT201* = 3RT2015-1BB41`) statt einzeln laden.
- Fehllisten: `Daten\Fehlende_Komponenten_nach_Hersteller.txt` (gesamt), `<Projekt>\CAD\QET\<Nr>_fehlende_Komponenten.txt`.
- Danach in `Daten`: `git add -A; git commit; git push nas main` (nie GitHub – Herstellerdaten).

## Import in die Sammlung (Fork-Kommandozeile)

```
QElectroTech.exe --import-eplan <datei.edz|datei.dxf|paket.zip> <Daten\elements-company> --manufacturer <Hersteller> --type <Bestellnr> [--category <Kategorie>] [--edz teil.edz]
```
EDZ und Einzel-DXF brauchen `--manufacturer` und `--type`. Ausgabe `-> <pfad>` je geschriebenem Element.
Ohne Fenster: `QT_QPA_PLATFORM=offscreen` (qoffscreen.dll liegt in build\deploy\platforms).

## Automatisch mit dem Fork (Branch feature/herstellerportale, ab 09.11.2026)

```
QElectroTech.exe --fetch-parts Daten\Fehlende_Komponenten_nach_Hersteller.txt --manufacturer SIE [--dry-run] [--limit N] [--json]
QElectroTech.exe --portal-status
```
Holt je Teil: Rohdateien aus `herstellerdaten\<H>\<Nr>\`, Siemens-Links aus
`herstellerdaten\Siemens\sieportal_links.txt` (`<Nr><Tab><static.siemens.com-Link>`), Kategorie und DXF-Paket über
die Data-Portal-API (Token im Windows-Anmeldeinformationsspeicher, `--portal-login dataportal` mit Token auf stdin),
importiert EDZ/DXF in die Firmensammlung und legt STEP nach `3D\<H>\<Nr>.stp`. Links sammeln: Skript
`tools\herstellerportale\sieportal_links.js` in Chrome auf sieportal.siemens.com. 3Dfindit-Anforderungen mit
`--portal-request 3dfindit <Nr>` zählen (Limit 50/24 h wird erzwungen). Skript-API: `qet.fetchParts/fetchPart/portalStatus`.
**Nie** SiePortal per curl/Programm abfragen: Akamai antwortet 403, danach zeigte auch Chrome eine Weile „Access Denied“ (09.11.2026).

## Siemens – SiePortal (Nutzer in Chrome angemeldet)

- Produktseite: `https://sieportal.siemens.com/de-de/products-services/detail/<Bestellnr ohne Leerzeichen>`
  (Leerzeichen der EPLAN-Schreibweise entfernen; `6EP14373BA00` → `6EP1437-3BA00`).
- Knopf „Download von Produktbildern und Daten für M-CAD und E-CAD (n)“ öffnet einen Dialog mit Vorschaubildern;
  jedes Vorschaubild anklicken → Links erscheinen im Dialog (`[role=dialog] a`):
  - `EPLAN_MAKRO_P8EDZ` → `https://static.siemens.com/mimes/imagedb_edz/<ID>V.edz`
  - `BILDDB_VECTOR_GRAPHIC_RGB_DXF_VDXF` → `.../imagedb_dxf/<ID>V.dxf` (Maßbilder, Schaltbild – mehrere)
  - `BILDDB_VECTOR_GRAPHIC_RGB_STP_VSTP` → `.../imagedb_stp/<ID>V.stp` (AP214)
- Die `static.siemens.com`-Dateien sind öffentlich: direkt mit curl laden, kein Browser-Download nötig.
- Die Medienliste kommt mit dem Seitenaufbau (kein eigener API-Aufruf beim Öffnen des Dialogs) → automatisieren
  über **unsichtbare iframes** auf derselben Seite (gleiche Herkunft), 3 parallel, ~10 s je Artikel:
  iframe auf die Detail-URL, warten bis der Knopf da ist, klicken, alle Bilder `[role=dialog] img` mit Breite < 80
  anklicken (600 ms Pause), Links `imagedb_(edz|dxf|stp)` sammeln. Vorlage: `vorlagen/sieportal_links.js`.
- Motoren (1FK2 …) haben im Dialog nur Bilder (3D über den Motor-Konfigurator) – für den Schrank nicht nötig.

## 3Dfindit (STEP, Nutzer in Chrome angemeldet)

- Limit **50 CAD-Anforderungen je 24 h** und Konto. Erst `gleiche_modelle.txt` pflegen.
- Produkt-URL `https://www.3dfindit.com/de/digitaltwin/x?path=<katalogpfad>` → 6 s → CAD-Knopf → **4 s warten**
  (sonst bricht der Seitenwechsel ab und zählt trotzdem). Format einmal auf STEP AP214 stellen (Zahnrad).
- Fertige Dateien gesammelt im Aktivitäten-Panel (Glocke); Chrome braucht „automatische Downloads erlauben“.
- Einige Hersteller (WAGO u. a.) bieten dort zusätzlich „EPLAN EDZ“ – mitnehmen, neben die .stp legen.
- Platzhalter (wenige KB) aussortieren (3RV20/24/28, 5SY, 3SU1401-1BB50; 6ES7512 ohne 3D). Nicht bei 3Dfindit:
  Rittal, Murrelektronik, Pilz, Finder (teils leer), SEW, Kraus & Naimer, Fiessler, Cybelec, LazerSafe, Givi Misure,
  ABB, Eaton/Moeller (nur ein allgemeiner „Eaton's CAD“-Katalog), STEGO, Hager.

### 3Dfindit-Ablauf (erprobt 10.11.2026, 48 Teile)

- Formate einmal im Zahnrad-Dialog neben „CAD“ wählen: **STEP AP214 + DXF AUTOCAD 2010-2012 (2D)** (max. 5).
  Ein Paket enthält dann `<nr>.stp` + `<nr>_front|right|top.dxf`; zählt als **eine** Anforderung.
- Je Teil: `https://www.3dfindit.com/de/search?q=<nr>` → 5 s → Ergebniskarte per JS klicken
  (`a[href*=digitaltwin]`, Text ohne Leerzeichen enthält Hersteller + Nr; Einträge mit „Download“ sind das
  Aktivitäten-Panel, ausfiltern) → 7–8 s → Knopf `CAD(n)` bzw. `CAD` per JS klicken → 3 s.
- Danach erscheint je nach Hersteller ein Zusatz-Dialog „Optional: Zusätzliche Formate“ (Shadow-DOM, per JS nicht
  greifbar → Koordinaten). Phoenix: nur „SymRepDXF1“, Knopf „CAD-Dateien erzeugen“ bei (783, 269) im 1565×784-Raster.
  **WAGO**: „EPLAN EDZ (zip)“ anhaken (592, 231) – muss **jedes Mal** neu gesetzt werden –, Knopf dann bei (782, 338).
  Weidmüller: kein Dialog. Der Dialog zeigt „noch N von 50 Downloads“ – zum Mitzählen.
- **Varianten-Falle:** Produktfamilien öffnen oft eine andere Zeile (750-626 → 750-626/020-000). Vor dem CAD-Klick
  `document.title` mit der Nr vergleichen; sonst die Tabellenzeile mit exakter Nr anklicken. Abbrechen per X kostet nichts.
- Abgekündigte Teile (WAGO 750-66x/000-003) sind nicht ladbar; der Dialog nennt den Ersatz (/000-004) – den laden
  und in `gleiche_modelle.txt` zuordnen, wenn die Maße (BREITE/HOEHE/TIEFE) gleich sind.
- Herunterladen: Aktivitäten-Panel öffnen, alle Knöpfe „Download“ per JS nacheinander klicken (2 s Abstand; Tab muss
  sichtbar bleiben, sonst bremst Chrome). Dateien: `Downloads\3dfindit-<HERSTELLER>-<nr>-<id>.zip` (WAGO: `/` → `_`).
- Maße für die Ansichtswahl stehen im `mident` der Panel-Links: ETIM `BAF016` = Breite, `BAA020` = Höhe, `BAB577` = Tiefe;
  WAGO `BREITE/HOEHE/TIEFE`. Fast immer passt „front“; bei WAGO „right“ (Gehäuse mit Rastnasen etwas größer).
  Beim Vergleich Bögen nicht als Vollkreise zählen (sonst wird „front“ zu groß gerechnet).
- WAGO-Pakete enthalten `<nr>_edz.zip` → entpacken, `.edz` mit dem Fork importieren (liefert Schaltbild, kein 2D-Aufbau).
- Skript `tdf_laden.py` (Scratchpad) + `tdf_mass.txt` (`nr B H T`): STEP → `Daten\3D\<H>\<nr>.stp`, EDZ-Import,
  passende DXF-Ansicht → `<nr>_Aufbau.elmt`.

## SEW-EURODRIVE (ohne Anmeldung)

`https://www.sew-eurodrive.de/os/dud/?tab=cad&country=DE&language=de_de&search=<Typ>&3D_output_format=STEP`
→ 10–15 s („Dateien werden generiert“) → Link „STP“. Optionskarten nur „CAD-Modell anfragen“.

### Siemens-Fallen (09.11.2026)

- Zu schnell (3 iframes parallel, ~190 Artikel) → Seite liefert „Access Denied“ (ab ~130 Abrufen). Höchstens 2
  parallel, bei „Access Denied“ sofort pausieren und danach einzeln mit ~20 s Abstand weiter. Nicht umgehen.
- Ergebnisse aus der Seite holen: Tool-Ausgabe wird ab ~1000 Zeichen abgeschnitten und Text mit Query-Strings
  geblockt; `fetch` auf 127.0.0.1 wird blockiert. Funktioniert: JSON als Blob-Download speichern
  (`a.download='sieportal_links_1.json'`) → landet in `C:\Users\test\Downloads`.
- EDZ-Inhalt: oft nur `<typ>_2D.ema` (MacroFuncType 6 = 2D-Montageaufbau) + `_3D.ema` (10), oder Schaltbild (1) und
  2D (6) in **einer** .ema. Der Fork liest seit 09.11.2026 beides (`EplanMacroReader::readLayout`; Teilobjekte S31x5
  Linie A531/A532, S32x5 Bogen A553/A554 rad, A555 Mitte, A556 Radius, S89x5 Rechteck A1651/A1652, S30x5 Text
  A501/A511; nur Teilobjekte mit R7 = O5-Grafik) und schreibt `<Bestellnr>_Aufbau.elmt` (2,5 px/mm).
- Einige EDZ sind ZIP statt 7z (z. B. 3NA3832) → Fork-Import kann sie noch nicht.
- Durchlauf 09.11.2026: 131 von 192 Artikeln mit Links, 117 EDZ importiert, 89 mit Aufbau.
- Skripte (Scratchpad, bei Bedarf neu anlegen): `sie_laden.py` (Links-JSON → Daten\herstellerdaten\Siemens\<nr>,
  STEP → Daten\3D\Siemens\<nr>.stp), `edz_import.py <Hersteller>` (alle EDZ → Sammlung, Kategorie aus vorhandenem
  Ordner bzw. Präfix).

## Rittal (rittal.com, Anmeldung nicht nötig)

- Produktseite: `https://www.rittal.com/de-de/product/show/variantdetail.action?productID=<Artikelnr ohne SV>`
  (leitet auf `/de-de/products/.../PRO…?variantId=<nr>` um).
- **EPLAN-Makros gibt es dort nicht**; der Abschnitt „Downloads“ verlinkt nur auf das EPLAN Data Portal
  (`dataportal.eplan.com/part-details/RIT/RIT.<nr>/...`). Deshalb DXF (2D) + STEP über den CAD-Abschnitt.
- CAD-Abschnitt = CADENAS PARTserver. Formular (nach „CAD“ → „Direkter Download“) postet an
  `https://webapi.partcommunity.com/cgi2pview.asp` mit `cgiaction=download`, `downloadflags=ZIP`, `firm=rittal`,
  `language=german`, `format=<Qualifier>`, `detail=2` (niedrig; 1 hoch),
  `part={$CADENAS_DATA\23d-libs\...prj},{BEST=<nr mit Punkt>},{DETAIL=2}`, `ok_url=<Produktseite>…caddlxml=<%download_xml%>`.
  Qualifier aus `cadqualifier.asp?firm=rittal`: `DXF2D-2010`, `STEP-214`.
- Nach dem Absenden kehrt die Seite mit `caddlxml=<…/download.xml>` zurück; download.xml abfragen, bis `<ZIPFILE>`
  da ist → `…/partserver/preview/<id>/PARTserver<id>.zip` (öffentlich, curl).
- **Schnellweg (8 s je Anfrage, im Browser-Tab der Rittal-Seite):** CAD-Daten per `fetch('https://api.rittal.com/v1/cadenas/<nr>')`
  (JSON: `CNS_FILENAME`, `BEST`, `DETAIL` = erlaubte Detailstufen, `CNS_MIDENT` mit `{B=…},{H=…},{T=…}` = Datenblattmaße),
  dann ein eigenes Formular mit obigen Feldern in ein verstecktes iframe posten; `ok_url=https://www.rittal.com/robots.txt?caddlxml=<%download_xml%>`
  → iframe landet auf derselben Herkunft, `caddlxml` aus `contentWindow.location.search` lesen. Fehlt in `DETAIL` die 2,
  den ersten Wert nehmen; ohne Detailstufe kein `detail`-Feld. Der langsame Weg über das Produktseiten-iframe braucht
  wegen der Hintergrund-Drosselung von Chrome ~2 min je Anfrage.
- Aufruf von `api.rittal.com` außerhalb des Browsers (curl mit Origin-Kopf) ist nicht erlaubt – im Tab bleiben.
- Ergebnisse ausleiten: Der zweite Blob-Download im selben Tab wird von Chrome still blockiert → kompakt ausgeben
  (`<nr> D|S <id>`, ~22 Zeilen je Aufruf) und in `rittal_kurz.txt` sammeln; Maße als `rittal_mass.txt` (`nr B H T`).
- `rit_laden.py` lädt die ZIPs, legt STEP unter `Daten\3D\Rittal\<nr>.stp` ab, wählt die DXF-Ansicht nach B×H
  (nur front/top/right/left/bottom – „back“ ist gespiegelt) und importiert sie mit dem Fork als `<nr>_Aufbau.elmt`.
- Durchlauf 09.11.2026: 86 Artikel, 131 Anfragen ok, 47 Aufbauten, 84 STEP; 3539000 hat keine CAD-Daten.
  Offen: 9343000 passt nur gedreht (top 214×106 statt 106×194) – Drehung fehlt noch.
- DXF-ZIP enthält sechs Ansichten `<nr>_front|back|left|right|top|bottom.dxf` in mm. **Achtung:** „front“ ist die
  Ansicht des 3D-Modells, nicht unbedingt die Schrankansicht – beim NH-Trenner 9343000 ist es „top“ (214 × 106 mm,
  Datenblatt B 106 / H 194 + Klemmen). Ansicht wählen, deren Maße zum Datenblatt (Breite × Höhe) passen.

## Regeln

- Downloads nur von den Herstellerseiten bzw. deren statischen Servern; keine Passwörter eingeben, keine CAPTCHAs.
- Data Portal: nur die öffentliche Schnittstelle des Forks (DXF); den undokumentierten Makro-Download nicht nutzen.
- Kabel nicht laden (Kabelkataloge LAPP/igus vorhanden).
- Maße immer aus Herstellerdaten, nie schätzen.
- Neue Erkenntnisse (URL-Muster, Fallen) sofort hier nachtragen.
