# Herstellerportale

QElectroTech holt zu fehlenden Bestellnummern die Herstellerdaten selbst:
EPLAN-Makro (EDZ) und DXF in die Firmensammlung, STEP nach `Daten\3D\<Hersteller>\<Bestellnummer>.stp`,
die geladenen Rohdateien nach `Daten\herstellerdaten\<Hersteller>\<Bestellnummer>\`.

Menü: **Bauteil-Import > Herstellerportale…** (ein Reiter je Portal: Anmelden, Abmelden, Status;
darunter „Fehlende Bauteile holen“ für eine Fehlliste).

## Was je Portal geht (Stand 09.11.2026)

| Portal | Offizielle Schnittstelle | Was QET tut |
|---|---|---|
| EPLAN Data Portal | ja, dokumentierte API (`https://dataportal.eplan.com/api`, Personal Access Token) | Suche (ohne Anmeldung, liefert auch die Kategorie), DXF-Paket mit Artikeldaten (mit Token) |
| Siemens SiePortal | nein; die Seite sperrt Programme (Akamai), keine CAx-API | lädt die **öffentlichen** Dateien `static.siemens.com/mimes/imagedb_edz|dxf|stp/…`, sobald ihre Links bekannt sind (Linkdatei) |
| Rittal | nein | Rittal-Daten kommen über das Data Portal |
| 3Dfindit | nein (PARTcommunity-Webservice nur für Katalogpartner) | Bauteil im Browser anfordern; QET zählt die Anforderungen, **50 je 24 h und Konto**, und lehnt darüber ab |

Ein Konto je Portal. Passwörter und Tokens liegen in der Windows-Anmeldeinformationsverwaltung
(`QElectroTech/portal/<id>`), nie in den Einstellungen; das alte Klartext-Token
`eplanimport/dataportal_token` wird beim ersten Lesen dorthin verschoben. QET füllt keine Anmeldeformulare
aus, löst keine CAPTCHAs und verteilt nichts über mehrere Konten.

## Siemens-Links

`Daten\herstellerdaten\Siemens\sieportal_links.txt`, je Zeile `<Bestellnummer><Tab><Link>`.
Erzeugen im eigenen Browser mit `sieportal_links.js` (Konsole auf sieportal.siemens.com,
`copy(await sieportalLinks([...]))`) oder im Dialog „Links hinzufügen…“. Nur Links auf
`https://static.siemens.com/mimes/imagedb_(edz|dxf|stp)/` werden angenommen.

## Kommandozeile

```
qelectrotech --fetch-parts Daten\Fehlende_Komponenten_nach_Hersteller.txt --manufacturer SIE [--dry-run] [--limit 10] [--json] [--report bericht.txt]
qelectrotech --portal-status [--json]
echo <token> | qelectrotech --portal-login dataportal
echo <passwort> | qelectrotech --portal-login siemens --user name@firma.de
qelectrotech --portal-logout siemens
qelectrotech --portal-request 3dfindit 3RT2015-1BB41
```

Weitere Optionen von `--fetch-parts`: `--collection`, `--raw-dir`, `--3d-dir`, `--links`, `--pause`,
`--force`, `--no-dataportal`. Ohne Fenster: `QT_QPA_PLATFORM=offscreen`.
Für Tests: `QET_CREDENTIAL_PREFIX` (eigener Bereich im Anmeldeinformationsspeicher),
`QET_PORTAL_DATA_DIR`, `QET_3D_DIR`, `QET_SETTINGS_DIR`, `QET_DATAPORTAL_URL`.

## Skript-API

```js
qet.portalStatus()                       // JSON, ohne Geheimnisse
qet.portalLoggedIn("dataportal")
qet.portalRequestsLeft("3dfindit")
qet.portalCountRequest("3dfindit", "3RT2015-1BB41")
qet.fetchParts(listPath, JSON.stringify({manufacturers: "SIE", dry_run: true}))
qet.fetchPart("3RV2011-0FA10", "SIE", "{}")
```

Anmelden geht bewusst nur im Dialog oder mit `--portal-login`, nicht aus Skripten.
