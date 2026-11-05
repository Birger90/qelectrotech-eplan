# EPLAN: Aktionen, Skripte, Kommandozeile, automatische Bearbeitung

EPLAN bildet jede sichtbare Programmfunktion als **Aktion** ab; ein Teil davon ist mit Parametern dokumentiert und lässt sich aus Symbolleisten, Skripten (C#/VB.NET), der Windows-Kommandozeile und der „automatisierten Bearbeitung“ aufrufen. Für den QET-Nachbau ist das vor allem ein Nachschlagewerk: Welche Arbeitsschritte (Prüflauf, PDF-Export mit Dreifachsprung, Nummerierung, Auswertungen, Stückliste, SPS-Austausch) EPLAN als zusammengehörige Einheit versteht und welche Optionen es dabei gibt – und wie man, falls EPLAN verfügbar ist, Originaldaten automatisiert ausleiten könnte.

Grundlage: EPLAN-Hilfe Plattform 2.9 (`availableactions_*`, `scripts_*`, `commandlinecall_*`, `autoprocgui_*`, `news_p_2_9_api`). Alle Aktionsseiten liegen unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_<name>.htm.

## Inhalt

1. Grundprinzip Aktionen
2. Gemeinsame Parameter-Regeln
3. Aktionen nach Thema (Tabelle)
4. Wichtige Aktionen im Detail: export (PDF), check, renumber, generate, reports, partslist, plcservice, Eigenschaften setzen, Makros; 4.10 ergänzende Einzelseiten (Projekt, Verbindungen, Ausgabe, Artikel, externe Bearbeitung, Sprache, Einstellungen, Oberfläche)
5. Skripte (Aufbau, Attribute, Laden/Ausführen, Einstellungen)
6. Kommandozeilenaufruf (Syntax, allgemeine Parameter)
7. Automatisierte Bearbeitung (Schemata, automatisierbare Aktionen)
8. Neuerungen API 2.9
9. Bedeutung für QET
10. Quellen

---

## 1. Grundprinzip

- Alle Menüpunkte und Symbolleisten-Schaltflächen sind mit einer Aktion verknüpft. Die dokumentierten Aktionen kann man als Schaltflächen in Symbolleisten legen, über die Kommandozeile starten, in Skripten verwenden; die Skripte der automatisierten Bearbeitung bestehen aus diesen Aktionen. **Groß-/Kleinschreibung** von Aktionsname und Parametern muss exakt stimmen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_k_start.htm
- In den Hilfebeispielen sind Zeilenumbrüche nur zur Lesbarkeit; beim Eingeben entfernen, Aktion und Parameter jeweils durch ein Leerzeichen trennen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_eplanaktionen.htm

## 2. Gemeinsame Parameter-Regeln

- **PROJECTNAME** (Projekt mit vollem Pfad, `.elk`) ist fast überall optional: aus der Oberfläche/Skript gilt das ausgewählte Projekt; **in der Kommandozeile Pflicht** oder vorher `ProjectAction` verwenden, sonst Systemmeldung. Ohne Angabe bei Kommandozeilen-Aktionen: zuletzt geöffnetes Projekt.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_check.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/commandlinecall_k_automatischeaktionen.htm
- Seitenauswahl, wiederkehrend bei check, export, generate, reports, translate, print: `PAGENAME` (eine Seite, z. B. `=EB3+ET1/2` – also vollständiger Seitenname mit Struktur), `PAGENAMEn` (`/PAGENAME1:… /PAGENAME2:…`), `SELn` (Objekt-ID, z. B. `/SEL1:38/4/12/0`), `USEPAGEFILTER:1` (nur im Seiten-Navigator gefilterte Seiten), teils `PAGEFILTERNAME`. Ist `PAGENAME` gesetzt, wird `USEPAGEFILTER` ignoriert; ohne beides: alle Seiten.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_export.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_generate.htm
- Schemata werden nur mit **Namen** (ohne Pfad) übergeben; fehlt der Parameter oder ist er leer, gilt das zuletzt benutzte Schema (`VERIFICATIONSCHEME`, `EXPORTSCHEME`, `CONFIGSCHEME`, `FILTERSCHEME` …).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_check.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_compress.htm
- Boolesche Parameter meist `0`/`1`. Mehrfach-Parameter werden durchnummeriert (`/LAYER1: … /VISIBLE1:`; `/STARTVALUE_1`, `/GROUP_1`).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_changelayer.htm

## 3. Aktionen nach Thema

Quelle (Gesamtliste): https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_eplanaktionen.htm ; Einzelseiten siehe Quellenliste.

| Thema | Aktion | Zweck | Wichtigste Parameter |
|---|---|---|---|
| Projekt | `ProjectOpen` | Projekt öffnen | `Project`, `OpenMode` STANDARD/READONLY/EXCLUSIVE |
| Projekt | `ProjectAction` | Projekt öffnen, eine Aktion ausführen, schließen | `Project`, `Action` (+ deren Parameter), `NOCLOSE:1`, `OpenMode` |
| Projekt | `edit` | Projekt/Seite/BMK öffnen, Cursor setzen | `PAGENAME`, `DEVICENAME` (z. B. `=EB3+ET1-F1`), `X`,`Y`, `INSTALLATIONSPACE`, `DEVICENAME3D` |
| Projekt | `projectmanagement` | Projektinfo-XML lesen, veröffentlichen, Schnappschuss, Eigenschaftsanordnungen ex/import, **Projekt korrigieren**, reorganisieren, Verzeichnis einlesen | `TYPE` READPROJECTINFO/ToggleSections/PUBLISHTODISK/PUBLISHTOEMAIL/CREATESNAPSHOTCOPY/EXPORT-/IMPORTPROPERTYPLACEMENTSSCHEMAS/CORRECTPROJECTITEMS/REORGANIZE/LOADDIRECTORY, `SCHEME`, `EXTENDEDMODE`, `PROJECTSDIRECTORY`, `SCANSUBDIRECTORIES` |
| Projekt | `backup` / `restore` | Sichern (.zw1) / Wiederherstellen von Projekt oder Stammdaten | `TYPE` PROJECT/MASTERDATA, `ARCHIVENAME`, `DESTINATIONPATH`, `BACKUPMETHOD` BACKUP/PACK/SOURCEOUT/ARCHIVE, `BACKUPAMOUNT`, `INCLEXTDOCS`, `INCLIMAGES`, `MDTYPE` (SYMBOLS/MACROS/FORMS/ARTICLES/LANGUAGES/STANDARDSHEET/STATIONDATA); restore: `UNPACKPROJECT`, `MODE` |
| Projekt | `compress` | Nicht benötigte Projektdaten entfernen | `CONFIGSCHEME`, `FILTERSCHEME`, `USEFILTER` |
| Projekt | `XPrjActionUpgradeProjects` | Projekte auf aktuelles Datenbankschema heben | `Project`/`Folder`, `Archive`, `UpgradeXMLProjects`, `UpdateConnections`, `NoBackup` |
| Projekt | `SwitchProjectType` | Schaltplan- ↔ Makroprojekt | `PROJECTTYPE` 1/2 |
| Projekt | `selectionset` | Gibt ausgewähltes Projekt bzw. Seiten zurück (`PAGES` = Seitennamen mit `;`) | `TYPE` PROJECT/PAGES |
| Prüfen | `check` | Prüflauf | `TYPE` PROJECT/PAGE(S)/INSTALLATIONSPACE, `VERIFICATIONSCHEME`, `PAGENAME`, `SELn`, `USEPAGEFILTER`, `INSTALLATIONSPACENAME` (ID 36452) |
| Verbindungen | `generate` | Verbindungen aktualisieren / Kabel erzeugen | siehe §4.4 |
| Verbindungen | `EsCorrectConnections` | Gleichartige Verbindungsdefinitionspunkte je Signal zu einem Potenzialdefinitionspunkt zusammenfassen (Signale „correction-1…“) | – |
| Verbindungen | `XCMRemoveUnnecessaryNDPsAction`, `XCMUniteNetDefinitionPointsAction` | Überflüssige Netzdefinitionspunkte entfernen / je Netz vereinigen | `Quiet` |
| Nummerieren | `Renumber` | BMK, Seiten, Klemmen, Kabel, Verbindungen | siehe §4.3 |
| Auswertungen | `reports` | Auswertungen aktualisieren | `TYPE` PROJECT/PAGES/UPDATEMODELVIEWPAGES/CREATEMODELVIEWS/CREATECOPPERUNFOLDS/CREATEDRILLINGVIEWS, Seitenauswahl |
| Ausgabe | `export` | PDF, DXF/DWG, Grafik, EPJ | siehe §4.1 |
| Ausgabe | `exportToGraphics` | Seiten/Projekt als BMP/TIF/GIF/PNG/JPG | `TYPE` GRAPHICPROJECT/GRAPHICPAGE, `EXPORTSCHEME`, `FORMAT`, `COLORDEPTH`, `IMAGEWIDTH`, `BLACKWHITE` |
| Ausgabe | `print` | Drucken | `TYPE` PROJECT/PAGES, `PRINTERNAME`, `NUMBER`, `PRINTCOLLATE` (Vorgabe 1), `PRINTREVERSE`, `DESTINATIONFILE`, `PRINTCHANGEDPAGES` |
| Ausgabe | `label` | Fertigungsdaten/Beschriftung (txt/xls/xml) – standardmäßig **ganzes Projekt**, Auswahl nur mit `USESELECTION:1` | `CONFIGSCHEME`, `FILTERSCHEME`, `SORTSCHEME`, `LANGUAGE`, `DESTINATIONFILE`, `RECREPEAT`, `TASKREPEAT` |
| Ausgabe | `exportProductionWiring` | Drahtkonfektionierung (1 Komax, 2 CadCabel, 3 Schleuniger, 4 Steinhauer PWA, 5 Metzner, 7 allg. Drahtstückliste) | `MachineType`, `MachineName`, `MultipleConnections` |
| Ausgabe | `exportNCData` | NC-Daten (Steinhauer, Perforex, DXF, Bohrschablone, Kupfer-NC/-DXF, Rohrbiegung) | `MachineType` 1–8, `MachineName`, `MaterialName` |
| Ausgabe | `export3d` | Bauräume als STEP | `INSTALLATIONSPACENAMEn`, `STRUCTUREn`, `SEPARATEFILES` |
| Import | `import` | EPJ-Projekt, DXF/DWG in Makros oder Seite, **PDF-Kommentare** | `TYPE` PXFPROJECT/DXFDWGFILES/DXFPAGE/DWGPAGE/PDFCOMMENTS, `IMPORTFILE`, `IMPORTSCHEME`, `XSCALE`/`YSCALE`/`XOFFSET`/`YOFFSET` |
| Artikel | `partslist` | Stückliste ex/import, Artikelverwaltung ex/import, gespeicherte Eigenschaften löschen | §4.6 |
| Artikel | `synchronize` | Datenabgleich allpolig/einpolig/Übersicht; Systemartikel → Projekt bzw. Projekt → System | `TYPE` MULTILINE/SINGLELINE/OVERVIEW/SYSTEMPARTSTOPROJECT/PARTSTOSYSTEM, `STOREMODE` 0/1/2 |
| Artikel | `XPamsDeviceSelectionAction` | Geräteauswahl (`selectDevice`: alle Gerätedaten neu) oder `updateDevice` (nur Gerätedaten, Artikelnummer bleibt) | `MODE` |
| Artikel | `XPamSelectPart` | Artikelauswahl, liefert `SelectedPartsCount`, `PartNr(n)`, `Variant(n)`, `PartType(n)` | `DatabaseId` |
| Artikel | `XDLInsertDeviceAction` | Gerät über Artikel einfügen | `PartNr`, `PartVariant`, `PropertyIndex` 0–50 |
| Artikel | `devicelist` | Geräteliste ex/import/löschen | `FORMAT` XDLXmlExporter/…Txt…/…Csv… |
| Stammdaten | `masterdata` | Projektstammdaten aktualisieren | `TYPE:UPDATEPROJECT` |
| SPS | `plcservice` | SPS-Daten ex/import, Schaltplan aus SPS-Daten erzeugen | §4.7 |
| Makros | `XGedStartInteractionAction` (`XMIaInsertMacro`) | Makro einfügen, Formate setzen | §4.9 |
| Makros | `XGedUpdateMacroAction` | Makros aktualisieren (Projekt oder markierter Makrokasten) | `ProjectName`, `AutoAssignLastUsedRecord` |
| Makros | `GenerateMacros` | Makros aus Makroprojekt erzeugen (nur „Art des Projekts = Makroprojekt“) | `PAGENAMEn`, `WINDOWMACRODIR`, `PAGEMACRODIR`, `FILTERSCHEME`, `OVERWRITE` |
| Makros | `XMDeleteReprTypeAction` | Darstellungsart aus Makros entfernen | `RepresentationType` 0–11, `Source`, `Destination` |
| Symbole | `XEGActionInsertSymRef` | Symbol einfügen | `SymbolLibName`, `SymbolId`, `VariantId`, `FctDefTag`, `Placementmode` (all/single/paircrossreference/overview/graphics/…) |
| Eigenschaften | `XEsSetPropertyAction`, `XEsSetPagePropertyAction`, `XEsSetProjectPropertyAction` | Eigenschaft am ausgewählten Schaltzeichen/Seite/Projekt setzen | `PropertyId` (Eigenschaftsnummer), `PropertyIdentName` (benutzerdef.), `PropertyIndex`, `PropertyValue` |
| Eigenschaften | `XEsUserPropertiesExportAction` / `…ImportAction` | Benutzerdefinierte Eigenschaften als XML | `XMLFile`, `Project`, `Overwrite` |
| Externe Bearbeitung | `XMExportFunctionAction`, `XMExportConnectionsAction`, `XMExportPagesAction`, `XMActionDCCommonExport`, `XMActionDCImport` | Funktions-, Verbindungs-, Seiteneigenschaften nach txt/xls/xml und zurück (.edc) | `ConfigScheme`, `CompleteProject`, `Destination`, `ExecutionMode` 0/1/2, `ImmediateImport`; Import: `ProjectLink`, `DataConfigurationFile` |
| Sprache | `translate`, `SetProjectLanguage`, `XTranslateSourceLanguage` | Übersetzen, Sprache entfernen, Fehlwortliste; Anzeigesprachen (übersetzt nicht) | `TYPE` TRANSLATEPROJECT/TRANSLATEPAGES/REMOVELANGUAGE/EXPORTMISSINGTRANSLATIONS/…, `LANGUAGE` |
| Suche | `search` | Geräte, Eigenschaften, Texte, Seiten-/Projektdaten suchen (Ergebnis nur bei geöffnetem Projekt sichtbar) | `TYPE` DEVICETAG/ALLPROPERTIES/TEXTS/PAGEDATA/PROJECTDATA, `SEARCHITEM`, `WHOLETEXT`, `NOTPLACEDFUNCTIONS` |
| Ebenen | `changelayer`, `GraphicalLayerTable` | Ebeneneigenschaften ändern; Ebenen ex/import (.elc) | `LAYER(n)`, `VISIBLE`, `PRINTED`, `TEXTHEIGHT`, `COLORID` |
| Einstellungen | `XAfActionSetting`, `XAfActionSettingProject`, `XSettingsExport`, `XSettingsImport` | Benutzer-/Stations-/Firmen- bzw. Projekteinstellung setzen; Einstellungen als XML | `set`, `index`, `value`; `Node`, `XMLFile`, `prj`/`Project`, `Option` INSERT/OVERWRITE/UPDATE |
| Skripte, Add-ins | `ExecuteScript`, `RegisterScript`, `UnregisterScript`, `EplApiModuleAction`, `XSettingsRegisterAction`, `XSettingsUnregisterAction` | Skript ausführen (nur `[Start]`), Skript anmelden/abmelden, API-Add-in-DLL laden, Add-ons registrieren | `ScriptFile`, `/Param…`, `RegisterAgain`; `register`/`unregister`; `path`/`installFile` |
| Oberfläche | `CopyToolBarAction`, `MfExport/Import/RemoveToolbarAction`, `OpenWorkspaceAction`, `SaveWorkspaceAction`, `XSDPreviewAction`, `gedRedraw`, `XGedClosePage` | Symbolleisten, Arbeitsbereiche, Vorschau, Neuzeichnen | – |
| Sonstiges | `Topology`, `UpdateSegmentsFilling`, `ImportPrePlanningData`, `Import/ExportSegmentsTemplate`, `InsertModelViewAction`, `SpExport/ImportSubProject`, `XPlaUpdateDetailAction` | Topologie, Vorplanung, Modellansichten, Teilprojekte | – |

## 4. Wichtige Aktionen im Detail

### 4.1 `export` – PDF und Co.
- `TYPE`: `PDFPROJECTSCHEME` (Projekt als PDF) und `PDFPAGESSCHEME` (Seiten, mit `PAGENAMEn`) sind die aktuellen; `PDFPROJECT`, `PDFPAGE`, `PDFPAGES` sind **veraltet**. Weiter `PXFPROJECT` (EPJ, `EXPORTMASTERDATA` Vorgabe 1, `EXPORTCONNECTIONS` Vorgabe 0), `GRAPHICPROJECT/-PAGE`, `DXFPROJECT/-PAGE`, `DWGPROJECT/-PAGE`, `DXFDWGPROJECTSCHEME`, `DXFDWGPPAGESSCHEME`.
- PDF-relevante Parameter: `EXPORTFILE` (voller Pfad der PDF), `EXPORTSCHEME`, `BLACKWHITE` (bei PDF: 1 = s/w, 0 = farbig, 2 = Graustufen), **`USEZOOM`** (1 = beim Sprung von Navigationsseiten zur Komponente Ausschnittfenster, Komponente zentriert), **`ZOOMLEVEL`** (Höhe des Ausschnitts in mm, 1–3500; nur mit USEZOOM=1), **`USESIMPLELINK`** (1 = nur einfache Verknüpfung, 0 = **„Drei-Wege-Sprünge“** für alle Komponenten), `FASTWEBVIEW`, `READONLYEXPORT` (schreibgeschützte PDF), `USEPRINTMARGINS` (nur veraltete PDF-Typen), `EXPORTMODEL` (3D-Modelle mit).
- Grafik: `FORMAT` BMP/TIF/GIF/PNG/JPG (Vorgabe TIF), `COLORDEPTH` (Vorgabe 24), `IMAGEWIDTH` (Vorgabe 80 px), `IMAGECOMPRESSION` (TIF: LZW/RLE/CCITT3/CCITT4/NONE), `BLACKWHITE` Vorgabe 1. DXF/DWG: `TARGET` Disk/EMail/FromSettings, `LANGUAGE`, `SPLITSIZE`.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_export.htm

**Bedeutung für QET:** Bestätigt die Optionen aus [pdf-export.md](pdf-export.md): Dreifachsprung ist der Normalfall (`USESIMPLELINK:0`), Zoom ist ein Zusatz (`USEZOOM` aus = ganze Seite). Ein Fork-Exportdialog sollte genau diese Schalter anbieten: Dreifach/einfach, Zoom an/aus + Höhe in mm, s/w/farbig/grau, schreibgeschützt.

### 4.2 `check`
Siehe [pruefungen.md](pruefungen.md) §5. Beispiel in der Hilfe nutzt `/TYPE:PAGES`, die Parameterliste nennt `PAGE` – beides kommt vor (**Vermutung:** beide Schreibweisen werden akzeptiert).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_check.htm

### 4.3 `Renumber`
- `TYPE` DEVICES / PAGES / TERMINALS / CABLES / CONNECTIONS; `CONFIGSCHEME`, `STARTVALUE`, `STEPVALUE`, `USESELECTION` (nur ohne PROJECTNAME).
- DEVICES: `POSTNUMERATE` (nur mit „?“ markierte), `ALSONUMERATEDBYPLC`, `IDENTIFIER` (Kennbuchstabe als Gruppe: „K“ erfasst auch „KT44“; sonst nur reines „K23“).
- PAGES: `STRUCTUREORIENTED` (je Strukturkennzeichen statt durchgehend), `KEEPINTERVAL`, `KEEPTEXT` (Buchstabenteil des Seitennamens behalten), `SUBPAGES` 0 behalten / 1 durchnummerieren (je Hauptseite ab 1) / 2 in Hauptseiten wandeln.
- TERMINALS: `SEQUENCE` 0 wie Sortierung / 1 seitenorientiert / 2 kabelorientiert / 3 etagenorientiert; `EXTENT` 0 markierte / 1 ganze gewählte Leisten / 2 alle Leisten; `POTENTIAL_N`, `POTENTIAL_PE`, `POTENTIAL_SH` je 0 nicht ändern / 1 nicht ändern aber in Reihenfolge mitzählen / 2 nummerieren; `MULTIPLETERMINALS` 0/1 gleich/2 einzeln; `KEEPALPHA` 0 ignorieren / 1 Buchstaben behalten, erste Zahl neu / 2 alles neu; `PERMITSORTCHANGE`, `FILLGAPS`, `PREFIX`, `SUFFIX`.
- CABLES: `KEEPEXISTING` (nur Kabel mit „?“ nachnummerieren).
- CONNECTIONS: `GROUP_n`, `STARTVALUE_n`, `STEPVALUE_n`; `OVERWRITE` 0 alle / 1 außer „manuell gesetzt“ / 2 keine; `AVOIDIDENTICALDESIGNATIONS` 0 projektweit eindeutig / 1 in Auswahl / 2 keine / 3 je Zähler-Rücksetzbereich (Struktur/Seite); `VISIBILITY` 0 alle sichtbar / 1 nicht ändern (Standard) / 2 **einmal je Seite und Umfang** (nur an der grafisch ersten Verbindung sichtbar); `MARKASMANUAL`.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_renumber.htm

**Bedeutung für QET:** Die Klemmen-Nummerierungsregeln (N/PE/SH-Klemmen nicht ändern, aber mitzählen; Etagen-/kabelorientiert) und die Leiterbeschriftung „einmal je Seite“ erklären typische EPLAN-Bilder: Ein Potenzial trägt seine Nummer oft nur einmal pro Blatt, obwohl alle Leiter dazugehören. Beim Nachbau nicht an jedem Leiterstück eine sichtbare Nummer erzwingen; in QET die Nummer an allen Leitern setzen, aber nur einen Text sichtbar lassen.

### 4.4 `generate`
`TYPE:CONNECTIONS` (Verbindungen erzeugen/aktualisieren; `REBUILDALLCONNECTIONS:1` = komplett neu aufbauen; Seitenauswahl per `PAGENAME(n)`, `SELn`, `USEPAGEFILTER`, `PAGEFILTERNAME`) bzw. `TYPE:CABLES` (`CREATIONSCHEME`, `NUMBERINGSCHEME`, `AUTOSELECTSCHEME`, `REGENERATECONNS` Vorgabe 1, `KEEPOLDNAMES` Vorgabe 1, `STARTVALUE`, `STEPVALUE`, `ONLYAUTOCABLES` Vorgabe 1).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_generate.htm

**Bedeutung für QET:** In EPLAN sind „Verbindungen“ abgeleitete Objekte, die aus der Grafik **neu berechnet** werden (vor Prüfläufen automatisch). Das entspricht dem QET-Konzept, Potenziale aus den Leitern zu ermitteln; ein Verknüpfungsskript sollte ebenso immer aus dem aktuellen Grafikstand neu rechnen statt alten Stand fortzuschreiben.

### 4.5 `reports`
`TYPE:PROJECT` aktualisiert alle Auswertungen; `PAGES` nur Auswertungen **vom Typ** der angegebenen Seiten.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_reports.htm

### 4.6 `partslist`
`TYPE` IMPORT/EXPORT (Stückliste; `FORMAT` XPalXmlExporter (Vorgabe) oder XPalCSVConverter), IMPORTTOSYSTEM (Artikelverwaltung; Formate IXPamImportE21, …E5, …Text, …CSV, …Ecad, …CSVAddresses, XPamImportXml (Vorgabe); `MODE` 0 nur neue / 1 nur vorhandene aktualisieren / 2 beides; `FIELDASSIGNMENTSCHEME`; `ADDITIONAL_LANGUAGE`), EXPORTFROMSYSTEM (XPamExportXml Vorgabe; SQL-Filter `SQLFILTERPART`, `…ADDRESS`, `…CONSTRUCTION` (Bohrbilder), `…TERMINAL` (Anschlussbilder), `…ACCESSORYLIST`, `…ACCESSORYPLACEMENT`; für Vollexport alle auf `1=1`), DELETESTOREDPROPERTIES.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_partslist.htm

**Bedeutung für QET:** Der Vollexport der Artikelverwaltung als XPamExportXml (mit Anschlussbildern und Zubehör) wäre die sauberste Quelle für Artikel-/Anschlussdaten beim Bauteil-Import, falls ein EPLAN-Zugang besteht.

### 4.7 `plcservice`
`TYPE` BUSDATAEXPORT / BUSDATAIMPORT / GENERATEPLCSCHEMATIC. `CONVERTERID` z. B. `PlcDcExchangerSiemensTIA16AML` (TIA 16), `…TIA151AML`, `…TIA15AML`, `…SiemensTIAAML` (14SP1), `PlcDcAMLExchangerGeneral` (AutomationML-Standard), `PlcDcXMLExchangerUniversal` (SPS-Standardaustauschformat), `PlcDcXMLExchangerSiemens` (STEP 7 5.6), Beckhoff, Rockwell, Schneider, Mitsubishi, B&R, ABB, Phoenix, logi.cals, Bosch. Export: `CONFIGURATIONPROJECT`, `DESTINATIONFILE`, `OVERWRITE`. Import: `SOURCEFILE`, `IMPORTMATCH` 0 über interne Objekt-IDs / 1 über identifizierende Namen (ggf. Vergleichsdialog) / 2 kein Abgleich (alles neu), `SHOWCOMPAREDLG`. Schaltplan erzeugen: `CONFIGFILE`, `SINGLELINEPAGES`, `MULTILINEPAGES`, `OVERVIEWS`, `RACKOVERVIEWS`.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_plcservice.htm

**Bedeutung für QET:** `IMPORTMATCH:1` bestätigt die Regel aus [sps.md](sps.md): Abgleich über identifizierende Namen (Karten-BMK, Stecker, Anschluss), nicht über die Adresse. Ein TIA-Austausch im Fork (TIA-MCP vorhanden) sollte genauso matchen.

### 4.8 Eigenschaften setzen
`XEsSetPropertyAction` / `XEsSetPagePropertyAction` / `XEsSetProjectPropertyAction`: `PropertyId` = Eigenschaftsnummer (siehe [eigenschaften-ids.md](eigenschaften-ids.md)), `PropertyIdentName` für benutzerdefinierte (z. B. `PEP.Project.PVW_1`), `PropertyIndex` (meist 0), `PropertyValue`. Wirkt auf die aktuelle Auswahl.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xessetpropertyaction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xessetprojectpropertyaction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xessetpagepropertyaction.htm

### 4.9 Makros und Formate (`XGedStartInteractionAction`, `XMDeleteReprTypeAction`)
- Interaktionen: `XGedIaFormatDefPoints` (Verbindungsgrafik: `/width`, `/color`, `/type`, `/patternlength`), `XGedIaFormatGraphic`, `XGedIaFormatSymbol`, `XGedIaFormatText`, `XMIaInsertMacro`. „?“ = Wert unverändert lassen; `-16002` = „Aus Ebene“.
- **Farbcodes**: 0 schwarz, 1 rot, 2 gelb, 3 grün, 4 cyan, 5 blau, 6 magenta, 7 schwarz, 8 weiß, 9 hellgrau, 252 dunkelgrau, 253 grau (0–256 möglich). **Linientypen**: 0 durchgezogen, 1 gestrichelt, 2 gepunktet, 3 Strich-Punkt, 4 Strich-Punkt-Punkt, 5 Strich-2 kurze Striche, 26 langer Strich-Punkt. **Textausrichtung** 1–9 (oben/mitte/unten × links/zentriert/rechts), 10–12 Basis; Winkel nur 0/±45/±90/±135/180. Schriftarten 0–9 = Schriftart 1–10 der Firmeneinstellung. Weitere Textschalter: `/language` (AllLanguagesColumn/Row, OneLanguageVariable, `de_DE`), Positionsrahmen `/framewidth`, `/frameheight`, `/adjustframe`, `/propertyId` (nur Text einer Eigenschaft), `/layer` (z. B. EPLAN501).
- `XMIaInsertMacro`: `/filename`, **`/variant` 0–15 = Variante A–P**, **`/RepresentationType` 1 allpolig, 2 einpolig, 3 Paarquerverweis, 4 Übersicht, 5 Grafik, 6 Schaltschrankaufbau, 7 RI-Fließbild, 8 allpolig Fluid, 9 Topologie, 10 3D-Montageaufbau, 11 Funktional** (`XMDeleteReprTypeAction` zusätzlich 0 = Neutral).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xgedstartinteractionaction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xmdeletereprtypeaction.htm

**Bedeutung für QET:** Diese Nummerierung der Darstellungsarten ist die einzige amtliche Zahlenliste in der Hilfe. **Vermutung:** Die Codes im Makro-XML (`.ema`) für die Darstellungsart folgen derselben Zählung (0 neutral, 1 allpolig, 2 einpolig, 3 Paarquerverweis, 4 Übersicht …); `eplanmacroreader` sollte damit abgeglichen werden (vgl. Lücke „MacroFuncType nur aus dem Fork“ in SKILL.md). Varianten A–P = Index 0–15. Farb- und Linientyp-Codes helfen beim Übertragen von EPLAN-Leiterfarben (z. B. Ebene „Verbindungen“) in QET-Leiterfarben.

### 4.10 Ergänzende Aktionsdetails (Einzelseiten, Stand 2.9)

Die folgenden Angaben ergänzen die Tabelle in §3 um Parameter, Vorgabewerte und Einschränkungen aus den Einzelseiten. Der Standardsatz zu `PROJECTNAME` (optional aus Oberfläche/Skript, Pflicht in der Kommandozeile oder vorher `ProjectAction`) gilt, wo nicht anders vermerkt, und wird nicht wiederholt.

**Projekt, Sitzung, Auswahl**
- `ProjectOpen`: `Project`, `OpenMode` STANDARD (bearbeitbar, im Multiuser-Betrieb gleichzeitig durch mehrere Benutzer) / READONLY / EXCLUSIVE (andere Benutzer haben keinen Zugriff). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_projectopen.htm
- `ProjectAction`: `Project`, `Action` (die Parameter der Unteraktion folgen direkt dahinter), `NOCLOSE:1` (Projekt bleibt offen; Standard: wird geschlossen), `OpenMode` wie oben. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xesprojectaction.htm
- `edit`: ohne weitere Parameter öffnet es nur das Projekt; `PAGENAME` öffnet eine Seite (Pflicht, wenn `X`/`Y` gesetzt – dann steht der Cursor auf der Koordinate); `DEVICENAME` (vollständiges BMK, z. B. `=EB3+ET1-F1`) öffnet die Seite des Bauteils; `INSTALLATIONSPACE` (nicht zusammen mit `PAGENAME`) und `DEVICENAME3D` für Bauräume. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_edit.htm
- `selectionset /TYPE:PROJECT|PAGES` liefert das Ergebnis in den aufrufenden Kontext: Parameter `PROJECT` = vollständiger Pfad mit `.elk`, bzw. `PAGES` = Seitennamen mit Strukturkennzeichen, durch `;` getrennt (z. B. `=EB3+ET1/1;=EB3+ET1/2`). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_selectionset.htm
- `projectmanagement`: Werte von `TYPE` siehe §3; zusätzlich schaltet `ToggleSections` bei aktivierten Bearbeitungsbereichen die temporäre Bearbeitung im ganzen Projekt für den angemeldeten Benutzer ein/aus (Projekt muss offen sein). `FILENAME` (XML für READPROJECTINFO); `SCHEME` gilt für PUBLISHTODISK/PUBLISHTOEMAIL/CREATESNAPSHOTCOPY/CORRECTPROJECTITEMS; `OVERWRITE` (Vorgabe 0) nur für IMPORTPROPERTYPLACEMENTSSCHEMAS; `EXTENDEDMODE` (Vorgabe 0) nur für REORGANIZE; LOADDIRECTORY liest ohne Parameter `$(MD_PROJECTS)` ein, fehlt `SCANSUBDIRECTORIES`, werden Unterordner mitgelesen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_projectmanagement.htm
- `restore`: `TYPE` PROJECT/MASTERDATA, `ARCHIVENAME` (Beispiele: `.zw1` Projekt, `.zw2` Stammdaten), `PROJECTNAME` nur bei PROJECT, `DESTINATIONPATH` nur bei MASTERDATA, `UNPACKPROJECT` (Vorgabe 0; 1 nur für vorher gepackte Projekte), `MODE` 0 = ohne ProjectInfo.xml, 1 = ProjectInfo.xml berücksichtigen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_restore.htm
- `XPrjActionUpgradeProjects`: `Project` oder `Folder` (inkl. Unterordner); `Archive:1` (.zw1 mit aktualisieren und wieder packen), `BaseProject:1` (.zw9/.zx1), `UpgradeWriteProtectedProjects:1` (.elr/.elt/.els/.elx), `UpgradeXMLProjects:1` (.ept/.epj/.zx2), `FileTypes` (alle, *.*), `PackOriginalProject:1` (Original als 7zip, Voreinstellung), `UpdateConnections:1` (Vorgabe 0), `NoBackup:1` (Vorgabe 0), `IgnoreUpgradeBackups:1` (Sicherungen `[PROJEKTNAME]_V[Version]_[Uhrzeit]` beim Ordnerlauf überspringen). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_XPrjActionUpgradeProjects.htm
- `SwitchProjectType`: `PROJECTTYPE` 1 = Schaltplanprojekt, 2 = Makroprojekt; ohne Parameter wird auf die jeweils andere Art umgeschaltet. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_switchprojecttype.htm
- `masterdata /TYPE:UPDATEPROJECT` aktualisiert die Projektstammdaten (einziger Typ). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_masterdata.htm
- `SpExportSubProject` / `SpImportSubProject` (Teilprojekte): `SPNAME`, `SPNR`, `DBID` (Projekt-ID des Ausgangsprojekts, sonst `PROJECTNAME`), `AREA` (Bearbeitungsbereich), `AREAPATH`; nur beim Export `EXPANDONLY:1` = Objekte des Teilprojekts (Seiten, Bauräume, Vorplanungssegmente) erhalten Änderungsschutz und dürfen nur erweitert, nicht geändert oder gelöscht werden. Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_SpExportSubProject.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_SpImportSubProject.htm

**Verbindungen, Topologie, Vorplanung**
- `EsCorrectConnections`: fasst gleiche grafische Eigenschaften (Farbe, Linientyp …) der Verbindungsdefinitionspunkte eines Signals zu **einem** Potenzialdefinitionspunkt je Signal zusammen, wenn sie für das ganze Signal gleich sind; die neuen Punkte heißen „correction-1“, „correction-2“ …; leere Verbindungsdefinitionspunkte werden gelöscht. Gedacht z. B. nach einer EPLAN-5-Datenübernahme. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_escorrectconnections.htm
- `XCMRemoveUnnecessaryNDPsAction`: einziger Parameter `Quiet:1` (keine Meldungen). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_XCMRemoveUnnecessaryNDPsAction.htm
- `Topology /TYPE:RouteConnections` (ausgewählte Topologie-Verbindungen verlegen) bzw. `/TYPE:CreateFunctions` (Topologie-Funktionen automatisch erzeugen). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_Topology.htm
- `UpdateSegmentsFilling`: nur `PROJECTNAME` (aktualisiert laut Name die Füllung der Topologie-Segmente – Vermutung, die Seite beschreibt den Zweck nicht). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_UpdateSegmentsFilling.htm
- `ImportPrePlanningData`: `FILENAME`, `SCHEMENAME` (Pflicht; Zuordnung externer Felder zu EPLAN-Eigenschaften), `TABLENAME` (Excel-Tabelle/-Bereich), `DELIMITER` (Textdatei), `HEADER` (Vorgabe 0; 1 = Spaltenköpfe in Spalte „Externes Feld“), `TARGETNAME` (Zielobjekt über „Bezeichnung (vollständig)“, ID 44009; leer = unter dem Projektnamen), `SKIPERRORS` (Vorgabe 1), `OVERWRITE` (Vorgabe 1, gleichnamige Daten überschreiben), `UPDATEONLY` (Vorgabe 0; 1 = nur vorhandene Segmente/Planungsdaten aktualisieren). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_importpreplanningdata.htm
- `ExportSegmentsTemplate` (`FILENAME` Pflicht, `DESCRIPTION` mehrsprachig) und `ImportSegmentsTemplate` (`FILENAME` Pflicht): Segmentvorlagen als Datei aus- bzw. einlesen. Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_exportsegmentstemplate.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_importsegmentstemplate.htm

**Ausgabe, Fertigung, 3D**
- `exportToGraphics`: `TYPE` GRAPHICPROJECT/GRAPHICPAGE, `PAGENAME` (nur GRAPHICPAGE), `EXPORTSCHEME` liefert Vorgaben für alle optionalen Parameter (leer = zuletzt benutztes), `DESTINATIONPATH` (wird angelegt; beim Projektexport Unterordner mit Projektnamen), `FORMAT` BMP/TIF/GIF/PNG/JPG, `COLORDEPTH` 1/8/16/24/32 (TIF ohne 16, bei GIF irrelevant), `IMAGEWIDTH` in Pixeln (Höhe aus dem Seitenmaß), `IMAGECOMPRESSION` nur TIF (CCITT3/CCITT4/RLE erzwingen 1 Bit), `BLACKWHITE` Vorgabe 1, `USEPAGEFILTER` Vorgabe 0 (PAGENAME hat Vorrang). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_exporttographics.htm
- `print`: `PRINTERNAME` (Vorgabe Systemdrucker), `PAGENAME`, `PRINTCOLLATE` (1), `PRINTREVERSE` (0), `NUMBER` (1), `DESTINATIONFILE` (Druck in Datei, z. B. `.prn`), `USEPAGEFILTER` (0), `PRINTCHANGEDPAGES` (nur geänderte Seiten); Seitenauswahl-Regel wie bei export. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_print.htm
- `label`: zusätzlich `SHOWOUTPUT` (0) und `USESELECTION` (0); das Dateiformat txt/xls/xml muss zur Endung im `CONFIGSCHEME` passen; ohne `FILTERSCHEME`/`SORTSCHEME` wird ungefiltert bzw. unsortiert ausgegeben; anders als der Menüpunkt ignoriert die Aktion die aktuelle Auswahl. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_label.htm
- `exportProductionWiring`: `MachineType` (1 Komax, 2 CadCabel, 3 Schleuniger, 4 Steinhauer PWA, 5 Metzner, 7 allgemeine Drahtstückliste) und `MachineName` (Name aus den Einstellungen) sind Pflicht; `USESELECTION` (0; nur ohne PROJECTNAME), `TargetDirectory`, `FileName`, `LANGUAGE`, `MultipleConnections` (0 = nur noch nicht exportierte Drähte, 1 = alle erneut). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_exportProductionWiring.htm
- `ExportNCData`: `MachineType` 1 NC-Steinhauer, 2 Rittal Perforex BC, 3 NC-DXF, 4 Bohrschablone, 5 Rittal Perforex LC, 6 Kupfer-NC, 7 Kupfer-DXF, 8 Rohrbiegung; `MachineName` Pflicht; `USESELECTION`, `TargetDirectory` (überschreibt die Einstellung), `MaterialName` (nur Perforex und Kupfer-NC). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_exportncdata.htm
- `export3D /TYPE:STEPINSTALLATIONSPACES`: `INSTALLATIONSPACENAME(n)`, `STRUCTURE(n)` (Strukturkennzeichen; nötig bei mehrdeutigen Bauraumnamen, sonst werden alle gleichnamigen ausgegeben), `DESTINATIONPATH` (wird angelegt), `EXPORTSCHEME`, `SEPARATEFILES` (1 = je Bauraum eine Datei; die Beschreibung von 0 ist in der Hilfe widersprüchlich). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_export3d.htm
- `InsertModelViewAction` (Modell-Ansicht auf einer Seite einfügen): Pflicht sind `LAYOUTSPACE`, `PAGENAME`, `X`/`Y` (linke untere Ecke), `DX`/`DY`, `VIEWNAME`; `STRUCTURE` bei mehrdeutigem Bauraumnamen; `ANGLE` 1 = 90° gegen, 2 = 90° im Uhrzeigersinn; `STYLE` 0 Draht, 1 verdeckte Kanten, 2 Shading, 3 verdeckte Kanten vereinfacht, 4 Shading vereinfacht; `VIEWPOINT` 0 Standard, 1 unten, 2 oben, 3 links, 4 rechts, 5 vorne, 6 hinten, 7 SO-, 8 SW-, 9 NO-, 10 NW-isometrisch; `SCALESETTING` 0 automatisch, 1 einpassen, 2 manuell (dann `SCALE`); außerdem `SELECTIONSCHEME`, `ITEMLABELING`, `ROOTELEMENTS`, `DESCRIPTION`, `OBJECTID`. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_InsertModelViewAction.htm
- `import`: `PROJECTNAME` ist bei PDFCOMMENTS **Pflicht**; `IMPORTFILE` (PXFPROJECT, DXFPAGE, DWGPAGE, PDFCOMMENTS), `SOURCEPATH` und `DESTINATIONPATH` (wird angelegt) nur bei DXFDWGFILES, `IMPORTSCHEME` nur als Name, `PAGENAME` bei DXFPAGE/DWGPAGE, `XSCALE`/`YSCALE` Vorgabe 1, `XOFFSET`/`YOFFSET` Vorgabe 0. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_import.htm

**Artikel, Geräte, Makros, Symbole**
- `devicelist /TYPE:IMPORT|EXPORT|DELETE`, `IMPORTFILE`/`EXPORTFILE`, `FORMAT` XDLXmlExporter (Vorgabe) / XDLTxtImporterExporter / XDLCsvImporterExporter / benutzerdefiniert. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_devicelist.htm
- `synchronize`: `STOREMODE` (nur SYSTEMPARTSTOPROJECT) 0 = nur neue hinzufügen (Vorgabe), 1 = nur vorhandene überschreiben, 2 = beides. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_synchronize.htm
- `XPamsDeviceSelectionAction`: Projekt wird bei Bedarf automatisch geöffnet und wieder geschlossen; `MODE:selectDevice` (Vorgabe) löscht alle Gerätedaten inkl. Artikelreferenzdaten und vergibt sie neu, `MODE:updateDevice` aktualisiert nur die Gerätedaten, Artikelnummern und -referenzen bleiben (entspricht „Gerätedaten aktualisieren“ im Stückliste-Navigator). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xpamsdeviceselectionaction.htm
- `XPamSelectPart`: Eingabe `DatabaseId`; Rückgabe `SelectedPartsCount`, `PartNr(n)`, `Variant(n)`, `PartType(n)` (n ab 1). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_XPamSelectPart.htm
- `XDLInsertDeviceAction`: `PartNr`, `PartVariant`, `ProjectId`, `PropertyIndex` (1–50 = Index des Projektartikels, 0 = kein Projektartikel). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xdlinsertdeviceaction.htm
- `GenerateMacros`: `PAGENAMEn`, `INSTALLATIONSPACENAMEn`, `STRUCTUREn`, `WINDOWMACRODIR`, `PAGEMACRODIR`, `FILTERSCHEME` (leer = zuletzt benutzt), `OVERWRITE`; wirkt nur bei „Art des Projekts = Makroprojekt“. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_generatemacros.htm
- `XGedUpdateMacroAction`: mit `ProjectName` wird das Projekt bei Bedarf geöffnet und wieder geschlossen; ohne Projekt wirkt sie auf das markierte Objekt, das ein Makrokasten oder ein ihm zugeordnetes Objekt sein muss; `AutoAssignLastUsedRecord` weist mehreren Platzhaltern den zuletzt verwendeten Wertesatz zu. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xgedupdatemacroaction.htm
- `XEGActionInsertSymRef`: `SymbolLibName`, `SymbolId`, `VariantId`, alternativ Suche über `FctDefTag` (Funktionsdefinition), `Placementmode` (all, single, paircrossreference, overview, graphics, processandinstdiagram = RI-Fließbild, cabling = Topologie, functional), `SymbolType`, `CustomSymbols` (projektbezogene Einstellung für Kundensymbole); braucht ein Hauptfenster; ein nicht gefundenes Symbol öffnet die Symbolauswahl. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xegactioninsertsymref.htm
- `XSDPreviewAction`: `PROJECTNAME` (ohne Pfad: `$(MD_PROJECTS)`), `PAGENAME` oder `MACRONAME` (Fenster-/Seitenmakro mit Endung, ohne Pfad: `$(MD_MACROS)`), `SHOW` 1 öffnen / 0 schließen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xsdpreviewaction.htm

**Eigenschaften extern bearbeiten**
- `XMExportFunctionAction`, `XMExportConnectionsAction`, `XMExportPagesAction`, `XMActionDCCommonExport`: `DatabaseId` (sonst ProjectName), `CompleteProject` (1 = alle Funktionen/Verbindungen/Seiten, 0 = nur Auswahl), `ConfigScheme` (Vorgabe zuletzt verwendetes; bei DCCommonExport ohne Schema Abfragedialog), `Language`, `Destination` (txt/xls/xml passend zur Endung im Schema), `ExecutionMode` 0 Export, 1 Export und Anwendung starten, 2 extern bearbeiten und zurücklesen, `ImmediateImport` (nur Modus 2; 1 = ohne Rückfrage einlesen). Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xmexportfunctionaction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xmexportconnectionsaction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xmexportpagesaction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_XMActionDCCommonExport.htm
- `XMActionDCImport`: `ProjectLink` (Projekt mit Pfad), `DataConfigurationFile` (.edc, entsteht z. B. über „Eigenschaften extern bearbeiten > Funktionen exportieren“), `ProgressTitle`; ändert Eigenschaften von Funktionen, Seiten und/oder Verbindungen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xmactiondcimport.htm
- `XEsUserPropertiesExportAction` (`XMLFile`, `Project`) / `XEsUserPropertiesImportAction` (`XMLFile` – fehlt er, Dateiauswahl; `Project` – fehlt es, aktuelles Projekt oder Auswahl; `Overwrite`). Hinweis: Die Hilfe beschreibt `Overwrite:0` als „überschreiben“ und die Vorgabe 1 als „Abfragedialog“ – so übernommen, wirkt aber wie ein Dokumentationsfehler. Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xesuserpropertiesexportaction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xesuserpropertiesimportaction.htm

**Sprache, Suche, Ebenen**
- `translate`: zusätzlich `IMPORTTOTRANSDB` / `EXPORTFROMTRANSDB` (Wörterbuch ein-/ausspielen, `IMPORTFILE`), `EXPORTFILE` und `CONVERTER` für die Fehlwortliste (`XTrLanguageDbXml2TabConverterImpl` = Vorgabe, Tab-getrenntes Unicode; `XTrLanguageDbXml2E21UnicodeTabConverterImpl` = EPLAN-21-Format), `LANGUAGE` für REMOVELANGUAGE/EXPORTMISSINGTRANSLATIONS (mehrere Sprachen mit Komma), `USEPAGEFILTER`/`PAGEFILTERNAME`/`PAGENAMEn`/`SELn` nur bei TRANSLATEPAGES. Die Quellsprache lässt sich nicht entfernen. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_translate.htm
- `SetProjectLanguage`: Projekt muss geöffnet sein (in der Kommandozeile nur über `ProjectAction`); `DISPLAY` (Anzeigesprachen, mehrere mit `;`), `VARIABLE` (einsprachige Anzeigesprache), `SOURCE` (Quellsprache); übersetzt nichts, ändert nur die Anzeige; bei schreibgeschützten Projekten nur temporär. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xessetprojectlanguageaction.htm
- `XTranslateSourceLanguage`: `PROJECTNAME`, `SOURCELANGUAGE` (z. B. en_US). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_XTranslateSourceLanguage.htm
- `search`: `SEARCHITEM`, Schalter `CASESENSITIVE`, `WHOLETEXT`, `LOGICPAGES`, `GRAPHICPAGES`, `EVALUATIONPAGES`, `NOTPLACEDFUNCTIONS` – fehlen sie, gilt der zuletzt in der Oberfläche eingestellte Wert; Treffer erscheinen nur im Dialog Suchergebnisse, wenn das Projekt geöffnet ist. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_search.htm
- `GraphicalLayerTable /TYPE:IMPORT|EXPORT` mit `IMPORTFILE`/`EXPORTFILE` (Ebenen-Konfigurationsdatei `.elc`). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_GraphicalLayerTable.htm

**Einstellungen, Skripte, Add-ons, Oberfläche**
- `XAfActionSetting`: `set` (Einstellungspfad, z. B. `USER.SYSTEM.GUI.SHOW_PROPERTY_NR` = „Identifizierende Nummern anzeigen“), `index` (Vorgabe 0), `value`. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xafactionsetting.htm
- `XSettingsExport`: `Node` (z. B. `USER` oder `PROJECT`; leer = alle Einstellungen), `XMLFile` (Pfadvariablen erlaubt), `prj` (geöffnetes Projekt). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xsettingsexport.htm
- `XSettingsImport`: `XMLFile` (vorher über Einstellungen > Kontextmenü > Exportieren erzeugt), `Node` (z. B. `USER.MacrosGui`; leer = alles), `Project` (dann nur Projekteinstellungen, sonst Benutzer/Station/Firma), `Option` INSERT (nur fehlende einfügen) / OVERWRITE (Werte und Vorgabewerte) / UPDATE (nur Werte). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xsettingsimport.htm
- `XSettingsRegisterAction` (`path` oder `installFile` = Pfad zur `install.xml`) / `XSettingsUnregisterAction` (zusätzlich `allowAutoInstall:1` = beim nächsten Start automatisch wieder installieren). Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_XSettingsRegisterAction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_XSettingsUnregisterAction.htm
- `RegisterScript`: `ScriptFile` (.cs oder .vb), `RegisterAgain` (1 = erneut registrieren), `ShowDecider` (Rückfrage; ignoriert bei RegisterAgain=1); `UnregisterScript`: `ScriptFile`. Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_registerscript.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_unregisterscript.htm
- Symbolleisten (alle brauchen ein Hauptfenster): `CopyToolBarAction` (`from`, `to`), `MfExportToolbarAction` (`File` XML, `ToolbarName`, weitere als `0`, `1` …), `MfImportToolbarAction` (`File`, `Replace` = gleichnamige ersetzen), `MfRemoveToolbarAction` (`ToolbarName`, `Silent:1` = keine Meldung, wenn nicht vorhanden). Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_copytoolbaraction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_mfexporttoolbaraction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_mfimporttoolbaraction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_mfremovetoolbaraction.htm
- Arbeitsbereiche: `OpenWorkspaceAction` / `SaveWorkspaceAction` mit `Workspacename` und `Silent` (unterdrückt alle Dialoge, auch Fehlerdialoge). Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xsvopenworkspaceaction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xsvsaveworkspaceaction.htm

### Bedeutung für QET (zu 4.10)
- **Sprungziel über BMK:** `edit /DEVICENAME:=…-F1` öffnet die Seite eines Bauteils über das vollständige BMK. Ein Fork-Kommando „gehe zu BMK“ sollte ebenso vollständige Kennzeichen (`=Anlage+Ort-BMK`) annehmen (Vermutung: Ziel ist die Hauptfunktion; die Hilfe sagt nur „Seite mit BMK öffnen“).
- **Seitennamen als Schlüssel:** `selectionset /TYPE:PAGES` und alle Seitenparameter verwenden `=Struktur/Seitennummer`. QET-Skripte, die Folios adressieren, sollten dieselbe Form nutzen, damit Listen aus EPLAN direkt zuordenbar sind.
- **Soll-Daten aus EPLAN:** Mit `XMExportFunctionAction`/`XMExportConnectionsAction`/`XMExportPagesAction` (`CompleteProject:1`, `ExecutionMode:0`, xml) und `devicelist /TYPE:EXPORT` ließe sich auf einem EPLAN-Rechner per `ProjectAction` unbeaufsichtigt eine vollständige Funktions-, Verbindungs- und Seitenliste erzeugen – als Prüfliste für `verknuepfen.py` (Vermutung, nicht erprobt).
- **Vergleichsbilder:** `exportToGraphics /TYPE:GRAPHICPAGE /FORMAT:PNG /IMAGEWIDTH:… /BLACKWHITE:1` liefert Rasterbilder je Seite mit fester Breite; für einen Bildvergleich mit dem QET-Export dieselbe Breite und s/w verwenden.
- **Ein Potenzialdefinitionspunkt je Signal:** `EsCorrectConnections` zeigt, dass EPLAN gleichartige Verbindungsdarstellungen bevorzugt über einen Potenzialdefinitionspunkt je Signal statt über viele Verbindungsdefinitionspunkte abbildet – beim Nachbau Leitereigenschaften (Farbe, Linientyp) möglichst je Potential setzen, nicht je Leiterstück.

## 5. Skripte

- Skripte sind C#- (`.cs`) oder VB.NET-Quelltexte (`.vb`), außerhalb von EPLAN geschrieben; EPLAN lädt, **kompiliert** und führt sie aus. Sie nutzen die API **ohne eigene API-Lizenz**. Erlaubt: .NET-Assemblies System, System.XML, System.Drawing, System.Windows.Forms sowie `Eplan.EplApi.Base` und `Eplan.EplApi.ApplicationFramework`; **keine** weiteren Assemblies referenzierbar.
- Möglichkeiten: einmalig ausführen; per Kommandozeile mit Parametern; EPLAN-Aktionen aufrufen; Einstellungen lesen/ändern; neue Aktionen, Menüpunkte, Symbolleisten-Schaltflächen; auf Ereignisse reagieren (z. B. Programmstart/-ende).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_start.htm

| Attribut | Wirkung |
|---|---|
| `[Start]` | Funktion, die beim **Ausführen** aufgerufen wird (genau eine je Skript; darf Parameter haben) |
| `[DeclareAction("Name")]` | Funktion wird beim **Laden** als Aktion angemeldet |
| `[DeclareEventHandler]` | reagiert auf EPLAN-Ereignisse |
| `[DeclareMenu]` | fügt Menüpunkte hinzu (`new Eplan.EplApi.Gui.Menu().AddMenuItem("Text","Aktion")`, Menü *Dienstprogramme*) |
| `[DeclareRegister]` / `[DeclareUnregister]` | beim Hinzufügen bzw. vor dem Entfernen der Skriptdatei aufgerufen |

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_attribute.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptaufbau.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptefuermenuepunkte.htm

- **Ausführen** (*Dienstprogramme > Scripte > Ausführen*): nur Skripte mit `[Start]`, einmalig. **Laden** (*… > Laden*): Skripte mit DeclareAction/-Menu/-EventHandler; geladene Skripte werden bei jedem Programmstart automatisch wieder geladen, bis man sie **entlädt**. Fehler landen in den Systemmeldungen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_h_scripteausfuehren.htm
- Aufbau minimal: öffentliche Klasse mit genau einer `[Start]`-Funktion.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptaufbau.htm
- EPLAN-Aktion aus Skript: `ActionCallingContext ctx = new ActionCallingContext(); ctx.AddParameter("TYPE","PAGES"); … new CommandLineInterpreter().Execute("print", ctx);`. Skripte aus der automatisierten Bearbeitung kann man als Vorlage nehmen; dafür in der Funktion `AutoTreat` den Parameter `String ProjectName` und seine Verwendung entfernen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptemiteplanaktion.htm
- Neue Aktion: `[DeclareAction("MyScriptAction")]`; nach dem Laden wie jede Aktion nutzbar (Kommandozeile, Menü, Schaltfläche). Neu in 2.9: Die Methode darf den ganzen `ActionCallingContext` als Parameter erhalten.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptemitneueraktion.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_2_9_api.htm
- Parameter: `W3u.exe ExecuteScript /ScriptFile:<Datei> /Param1:… /Param2:…`; alle Parameter der Start-Funktion müssen exakt so übergeben werden, sonst läuft das Skript nicht. Mit `/NoSplash /Frame:0 /Auto` unsichtbar starten und danach beenden.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptemitparameter.htm
- Einstellungen (nicht Projekteinstellungen!) per `Eplan.EplApi.Base.Settings`: `SetBoolSetting("USER.SYSTEM.GUI.SHOW_PROPERTY_NR", true, 0)` schaltet „Identifizierende Nummern anzeigen“ ein; analog `GetBoolSetting`, `SetStringSetting` … Den Einstellungspfad erhält man über ein verstecktes Kontextmenü (`USER.EnfMVC.ContextMenuSetting.ShowExtended` = true, Neustart → „Einstellungspfad in die Zwischenablage kopieren“). Projekteinstellungen gehen über die Aktion `XAfActionSettingProject` (Beispiel `MacrosGui.InsertMacrobox`).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptefuereinstellungen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_xafactionsettingproject.htm

## 6. Kommandozeilenaufruf

- Syntax: `Eplan.exe [/allgemeine Parameter] Aktion /Param1:Wert1 /Param2:Wert2`. Je Aufruf **genau eine** Aktion; alle nach dem Aktionsnamen folgenden, mit „/“ oder „-“ beginnenden Angaben gehen an die Aktion. Allgemeine Parameter **vor** den Aktionsnamen. Schreibweise exakt. Fehler → *Dienstprogramme > Systemmeldungen*.
- Beispiel unsichtbares Drucken: `Eplan.exe /Variant:"Electric P8" /NoSplash /Frame:0 /Auto /Quiet print /TYPE:PAGES /PROJECTNAME:"…\Projekt.elk" /PAGENAME:=GB1+A1&EFS1/1`.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/commandlinecall_k_start.htm

| Allgemeiner Parameter | Wirkung |
|---|---|
| `/NoLoadWorkspace` | kein Arbeitsbereich laden |
| `/NoSplash` | kein Startbild |
| `/Language:en_US` | Dialogsprache für diesen Start (nur installiert + lizenziert) |
| `/Auto` | EPLAN nach Ausführung beenden |
| `/Quiet` | keine Dialoge während der Ausführung |
| `/Frame:0…3` | Hauptfenster unsichtbar / normal / minimiert / maximiert |
| `/Setup:0`, `USER`, `STATION`, `COMPANY` | Einstellungen auf Installationsstand zurücksetzen (Datenbanken werden gesichert) |
| `/PathsScheme:<Name>` | Verzeichnisschema (sonst Standard) |
| `/Variant:"Electric P8"` | Programmvariante, nur beim Start über `EPLAN.exe` der Plattform |
| `/User`, `/Password`, `/Station`, `/Company`, `/SystemConfiguration` | Einstellungs- und Rechteverwaltungsauswahl |
| `/NoRemoting:1` | kein Remote-Server (Vorgabe 0 = wird gestartet) |
| `/License:<Datei.lis>` | Lizenzvorbelegungsdatei |

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/commandlinecall_k_allgemeineparameter.htm

- Für die Kommandozeile freigegebene „automatische Aktionen“ sind die in §3 genannten (backup, changelayer, check, compress, … XSettingsImport). Ohne `/PROJECTNAME` gilt das zuletzt geöffnete Projekt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/commandlinecall_k_automatischeaktionen.htm
- Ältere Beispiele nutzen `W3u.exe` statt `Eplan.exe` (z. B. `W3u.exe EplApiModuleAction /register:"…\Eplan.EplAddin.MyAddin3.dll"`).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_eplapimoduleaction.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_executescript.htm

## 7. Automatisierte Bearbeitung

- *Dienstprogramme > Automatisiert bearbeiten*: Abfolge vordefinierter Aktionen in einem **benutzerbezogenen Schema**, ohne Programmierkenntnisse; EPLAN erzeugt daraus automatisch ein Skript (`.cs`) in `<Programmverzeichnis>\Scripte\<Firmenkennung>`. Für ein Projekt (Auswahl im Seiten-Navigator oder „Anwenden auf gesamtes Projekt“) oder mehrere Projekte (Projektverwaltung, [Strg]-Mehrfachauswahl, Kontextmenü *Automatisiert bearbeiten*; nacheinander).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_k_start.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_h_protokolldatei.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_h_ausfuehren.htm
- Schema anlegen: Dialog *Einstellungen: Automatisiert bearbeiten* → Neu → Aktionen von „Verfügbare“ nach „Ausgewählte“ verschieben (ggf. Einstellungsdialog der Aktion), Reihenfolge mit Pfeilen, Speichern (erzeugt Skript). Skripte aus Version ≤ 2.1 lassen sich als Schema importieren. Schemata sind kopier-, export- und importierbar.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_h_startactions.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_d_einstellungen.htm , https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_d_ausfuehren.htm
- Automatisierbare Aktionen: Artikel im Projekt aktualisieren (veraltete eingelagerte Artikel durch neuere Systemartikel ersetzen) · Artikel im Projekt vervollständigen (fehlende einlagern) · Artikel im System vervollständigen · Auswertungen aktualisieren · DXF/DWG · Daten sichern (.zw1, Quelle bleibt) · Drucken · Eingelagerte Artikeleigenschaften löschen · Fertigungsdaten exportieren/beschriften · Modell-Ansichten aktualisieren · PDF exportieren · Projekt komprimieren · Projektdatenbank optimieren (nötig vor Bearbeitung alter Projekte) · **Prüfläufe ausführen** · Topologie-Funktionen erzeugen · **Verbindungen aktualisieren** · Verlegen (Topologie) · Übersetzen · Übersetzungen entfernen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_k_automprojektaktionen.htm
- „Daten sichern“: Methode „Zusätzlich speichern“ (komprimiert als .zw1 ins Sicherungsverzeichnis, Pfadvariablen erlaubt); Optionen Fremddokumente (`…\<Projekt>\DOC`) und Bilddateien (`…\<Projekt>\Images`) mitsichern.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_d_datensichern.htm

### Bedeutung für QET
- Die typische EPLAN-Abschlusskette vor der Ausgabe ist: **Verbindungen aktualisieren → Auswertungen aktualisieren → Prüfläufe → PDF exportieren**. Für den QET-Nachbau analog: `gesamt.py` → `verknuepfen.py` → Prüfskript (siehe pruefungen.md §9) → PDF-Export. Ein Fork-Knopf „Projekt fertigstellen“ könnte diese Kette abbilden.
- `.zw1` ist das EPLAN-Sicherungsformat (gepacktes Projekt); `.edc` die Datenkonfiguration der externen Bearbeitung; `.elc` Ebenen-Konfiguration; `.epj` XML-Projektexport.

## 8. Neuerungen API 2.9 (Auswahl)

- API braucht .NET Framework 4.7.2; „EPLAN API Extension“ ist kostenpflichtiges Zusatzmodul/EADN-Entwicklerpaket. Neu u. a.: `Cable.GetSourcesAndTargets` (Quelle/Ziel je Ader), `Search.Replace`, `DMObjectsFinder.GetAll` (alle Objekte eines Typs), `FunctionDefinition.Identifiers` / `FunctionDefinitionLibrary.IdentifierSets` (Kennbuchstaben aller Normen), `MDUserDefinedPropertyDefinition`, `PlcService.ExportAddressOverview` (Adressraumbelegung), `MacroBox.LogicalAreaSegments` (Makrokasten als Polylinie), `SelectionSet.Selection` liefert `MacroEntry` aus dem Makro-Navigator, `ConnectionPointPosition` mit SocketSize, Anzugsmomenten, Abisolierlänge; Aktionen `XSettingsRegisterAction`/`XSettingsUnregisterAction`; `projectmanagement /TYPE:LOADDIRECTORY`; `XPamsDeviceSelectionAction /MODE:selectDevice|updateDevice`; `ExportNCData` mit Kupferformaten.

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_2_9_api.htm

## 9. Bedeutung für QET (Zusammenfassung)

1. PDF-Export-Schalter wie EPLAN: Dreifachsprung (Standard) vs. einfache Links, Zoom an/aus + Höhe in mm, Farbmodus, Schreibschutz (§4.1).
2. Darstellungsart-Codes 0–11 und Varianten A–P = 0–15 für den Makroleser abgleichen (§4.9, Vermutung zur XML-Gleichheit).
3. Leiterbeschriftung „einmal je Seite und Umfang“ und Klemmennummerierung mit N/PE/SH-Sonderregeln als EPLAN-Normalfall kennen (§4.3).
4. SPS-Abgleich über identifizierende Namen (§4.7).
5. Abschlusskette Verbindungen → Auswertungen → Prüfung → PDF (§7).
6. Falls ein EPLAN-Rechner verfügbar ist: Originaldaten per Kommandozeile unsichtbar ausleiten, z. B. `XMExportFunctionAction`/`XMExportConnectionsAction` (alle Funktionen/Verbindungen als xls/xml), `partslist /TYPE:EXPORT`, `check` + Meldungsexport – das wäre eine Soll-Liste für den Abgleich des QET-Nachbaus (Vermutung: nicht erprobt).

## 10. Quellen

Grundlagen, Skripte, Kommandozeile, automatisierte Bearbeitung:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/availableactions_o_eplanaktionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_attribute.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptaufbau.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptefuereinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptefuermenuepunkte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptemiteplanaktion.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptemitneueraktion.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_k_scriptemitparameter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/scripts_h_scripteausfuehren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/commandlinecall_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/commandlinecall_k_allgemeineparameter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/commandlinecall_k_automatischeaktionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_k_automprojektaktionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_h_startactions.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_h_protokolldatei.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_h_ausfuehren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_d_ausfuehren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_d_einstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/autoprocgui_d_datensichern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/news_p_2_9_api.htm

Aktionsseiten (alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/):
availableactions_o_backup.htm, …_changelayer.htm, …_check.htm, …_compress.htm, …_copytoolbaraction.htm, …_devicelist.htm, …_edit.htm, …_eplapimoduleaction.htm, …_escorrectconnections.htm, …_executescript.htm, …_export.htm, …_export3d.htm, …_exportncdata.htm, …_exportProductionWiring.htm, …_exportsegmentstemplate.htm, …_exporttographics.htm, …_generate.htm, …_generatemacros.htm, …_GraphicalLayerTable.htm, …_import.htm, …_importpreplanningdata.htm, …_importsegmentstemplate.htm, …_InsertModelViewAction.htm, …_label.htm, …_masterdata.htm, …_mfexporttoolbaraction.htm, …_mfimporttoolbaraction.htm, …_mfremovetoolbaraction.htm, …_partslist.htm, …_plcservice.htm, …_print.htm, …_projectmanagement.htm, …_projectopen.htm, …_registerscript.htm, …_renumber.htm, …_reports.htm, …_restore.htm, …_search.htm, …_selectionset.htm, …_SpExportSubProject.htm, …_SpImportSubProject.htm, …_switchprojecttype.htm, …_synchronize.htm, …_Topology.htm, …_translate.htm, …_unregisterscript.htm, …_UpdateSegmentsFilling.htm, …_xafactionsetting.htm, …_xafactionsettingproject.htm, …_XCMRemoveUnnecessaryNDPsAction.htm, …_xdlinsertdeviceaction.htm, …_xegactioninsertsymref.htm, …_xesprojectaction.htm, …_xessetpagepropertyaction.htm, …_xessetprojectlanguageaction.htm, …_xessetprojectpropertyaction.htm, …_xessetpropertyaction.htm, …_xesuserpropertiesexportaction.htm, …_xesuserpropertiesimportaction.htm, …_xgedstartinteractionaction.htm, …_xgedupdatemacroaction.htm, …_XMActionDCCommonExport.htm, …_xmactiondcimport.htm, …_xmdeletereprtypeaction.htm, …_xmexportconnectionsaction.htm, …_xmexportfunctionaction.htm, …_xmexportpagesaction.htm, …_xpamsdeviceselectionaction.htm, …_XPamSelectPart.htm, …_XPrjActionUpgradeProjects.htm, …_xsdpreviewaction.htm, …_xsettingsexport.htm, …_xsettingsimport.htm, …_XSettingsRegisterAction.htm, …_XSettingsUnregisterAction.htm, …_xsvopenworkspaceaction.htm, …_xsvsaveworkspaceaction.htm, …_XTranslateSourceLanguage.htm

Ergänzung §4.10: Die Einzelseiten sind dort jeweils mit vollständiger URL zitiert.
