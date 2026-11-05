# EPLAN-Eigenschaften: IDs, Namen, API-Defines

Faktentabelle aller 2569 Eigenschaften der EPLAN-Plattform 2.9, maschinell aus der Hilfe-Übersicht extrahiert (ohne Beschreibungstexte).
Quelle (gesamte Tabelle): https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/properties_o_main.htm

Spalten: ID | Bezeichnung | Objektklassen | Feldtyp | Index (indiziert, ggf. Anzahl) | API-Define.

## Bedeutung für QET

- EPLAN-Makros (.ema), Artikel-Exporte und EDZ speichern Eigenschaften als numerische IDs (z. B. `P22145` Schaltplanmakro am Artikel, `20367` Geräteposition, `20809` Sortierkennung). Diese Tabelle übersetzt sie beim Import in Namen und QET-Elementinformationen.
- Indizierte Eigenschaften (Spalte Index = Ja, n) tragen in Dateien einen Index, z. B. `ID[1]`.
- Querverweise auf Eigenschafts-IDs in den anderen Referenzdateien lassen sich hier nachschlagen (Strg+F nach der ID).

## Inhalt (Kategorien)

- Allgemein (15)
- Archivierungsdaten (10)
- Artikel (64)
- Betriebsmittel (205)
- Daten (1046)
- Einstellungen (244)
- Endkunde (18)
- Formate (80)
- Funktionsdaten (32)
- Keine Kategorie (365)
- Kunde (18)
- Makro (26)
- Meldungen (18)
- Revision (140)
- SPS-Daten (240)
- Spezial (12)
- Status (9)
- Symboldaten (6)
- Thermische Auslegung (19)
- Verbindung (2)

## Allgemein

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 19002 | X-Koordinate | Basisklasse für Symbolreferenzen Platzierungen auf Ebenen | Dezimalzahl | Nein | INSTANCE_XCOORD |
| 19003 | Y-Koordinate | Basisklasse für Symbolreferenzen Platzierungen auf Ebenen | Dezimalzahl | Nein | INSTANCE_YCOORD |
| 19005 | Spaltennummer | Platzierungen auf Ebenen Basisklasse für Symbolreferenzen Artikelreferenzen | Einsprachiger Text | Nein | INSTANCE_PATHID |
| 19006 | Zeilennummer | Platzierungen auf Ebenen Basisklasse für Symbolreferenzen Artikelreferenzen | Einsprachiger Text | Nein | INSTANCE_POSID |
| 19007 | Platzierung | Platzierungen auf Ebenen Basisklasse für Symbolreferenzen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) Verbindungen | Einsprachiger Text | Nein | INSTANCE_FULLPLACEMENTLOCATION |
| 19020 | Seitentyp | Basisklasse für Symbolreferenzen Platzierungen auf Ebenen | Einsprachiger Text | Nein | INSTANCE_PAGETYPE |
| 19022 | Seitenname | Basisklasse für Symbolreferenzen Artikelreferenzen Platzierungen auf Ebenen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | INSTANCE_PAGENAME |
| 19023 | Seitenname (vollständig) | Basisklasse für Symbolreferenzen Artikelreferenzen Platzierungen auf Ebenen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | INSTANCE_PAGEFULLNAME |
| 19024 | Seitenbeschreibung | Platzierungen auf Ebenen | Mehrsprachiger Text | Nein | INSTANCE_PAGENOMINATION |
| 20861 | Gerätegruppen-Hauptfunktion | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_IS_DEVICEGROUP_MAINFUNCTION |
| 20862 | BMK (identifizierend) der Gerätegruppen-Hauptfunktion | Funktionen | Einsprachiger Text | Nein | FUNC_DEVICEGROUP_MAINFUNCTION_IDENTNAME |
| 36076 | Produktkonfiguratorkennzeichen | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_ID_EXTERNALPRODUCTCONFIGURATOR_AUTOMATIC |
| 44049 | Segmentvorlage: Beschreibung | Segmentvorlagen (Vorplanung) Rohrklassen Stoffe | Mehrsprachiger Text | Nein | DMPLASPECIFICATION_DESCRIPTION |
| 44050 | Segmentvorlage: Name (identifizierend) | Segmentvorlagen (Vorplanung) Rohrklassen Stoffe | Einsprachiger Text | Nein | DMPLASPECIFICATION_NAME |
| 44051 | Segmentvorlage | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_SPECIFICATIONNAME |

## Archivierungsdaten

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 10370 | Kunde | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMER |
| 10510 | Datensicherung: Komplett sichern | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_BACKUP_FULL |
| 10511 | Datensicherung: Externe Dokumente sichern | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_BACKUP_EXTDOCS |
| 10512 | Datensicherung: Bilddateien sichern | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_BACKUP_IMAGES |
| 10513 | Datensicherung: Projekt reorganisieren | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_BACKUP_COMPRESS |
| 10514 | Datensicherung: Methode | Projekte | Ganzzahlig | Nein | PROJ_BACKUP_METHOD |
| 10515 | Datensicherung: Sicherungsmedium | Projekte | Ganzzahlig | Nein | PROJ_BACKUP_MEDIUM |
| 10516 | Datensicherung: E-Mail-Teilgröße in MB | Projekte | Dezimalzahl | Nein | PROJ_BACKUP_SPLITSIZE |
| 10517 | Datensicherung: Sicherungsverzeichnis | Projekte | Einsprachiger Text | Nein | PROJ_BACKUP_BACKUPPATH |
| 10518 | Datensicherung: Beschreibung | Projekte | Einsprachiger Text | Nein | PROJ_BACKUP_DESCRIPTION |

## Artikel

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 20100 | Artikelnummer | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_PARTNR |
| 20101 | Artikelvariante | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_VARIANT |
| 20102 | Stückzahl / Menge | Projekte Verbindungen Verbindungsdefinitionspunkte Funktionen Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Ganzzahlig | Ja, 50 | FUNC_ARTICLE_COUNT |
| 20103 | Datensatztyp | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Ganzzahlig | Ja, 50 | FUNC_ARTICLE_PARTTYPE |
| 20104 | Einsortierung des Artikels auf der Klemmenleiste | Funktionen | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_TERMINALSORTCODE |
| 20105 | In Stückliste unterdrücken (wenn gefiltert) | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Bool (Wahr / Falsch) | Ja, 50 | FUNC_ARTICLE_SUPPRESSINPARTSLIST |
| 20190 | Artikel: Externes Dokument 1 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_1 |
| 20191 | Artikel: Externes Dokument 2 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_2 |
| 20192 | Artikel: Externes Dokument 3 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_3 |
| 20193 | Artikel: Bezeichnung 1 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Mehrsprachiger Text | Ja, 50 | FUNC_ARTICLE_DESCR1 |
| 20194 | Artikel: Bezeichnung 2 | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Mehrsprachiger Text | Ja, 50 | FUNC_ARTICLE_DESCR2 |
| 20200 | Typenbezeichnung des Artikels | Funktionen Verbindungen | Einsprachiger Text | Ja, 50 | FUNC_TYPENOTATION |
| 20203 | Artikel: Bezeichnung 3 | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Mehrsprachiger Text | Ja, 50 | FUNC_ARTICLE_DESCR3 |
| 20263 | Artikel: Externes Dokument 4 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_4 |
| 20264 | Artikel: Externes Dokument 5 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_5 |
| 20265 | Artikel: Externes Dokument 6 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_6 |
| 20266 | Artikel: Externes Dokument 7 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_7 |
| 20267 | Artikel: Externes Dokument 8 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_8 |
| 20268 | Artikel: Externes Dokument 9 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_9 |
| 20269 | Artikel: Externes Dokument 10 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_10 |
| 20270 | Artikel: Externes Dokument 11 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_11 |
| 20271 | Artikel: Externes Dokument 12 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_12 |
| 20272 | Artikel: Externes Dokument 13 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_13 |
| 20273 | Artikel: Externes Dokument 14 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_14 |
| 20274 | Artikel: Externes Dokument 15 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_15 |
| 20275 | Artikel: Externes Dokument 16 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_16 |
| 20276 | Artikel: Externes Dokument 17 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_17 |
| 20277 | Artikel: Externes Dokument 18 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_18 |
| 20278 | Artikel: Externes Dokument 19 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_19 |
| 20279 | Artikel: Externes Dokument 20 | Funktionen Projekte Verbindungen Verbindungsdefinitionspunkte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_EXTERNAL_DOCUMENT_20 |
| 20284 | Verwendetes Bohrbild | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Ganzzahlig | Ja, 50 | FUNC_ARTICLE_CONSTRUCTION |
| 20307 | Sicherheitskennwerte: Verwendeter Anwendungsfall | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Ganzzahlig | Ja, 50 | FUNC_ARTICLE_USED_SAFETYRELATEDVALUE |
| 20358 | Harness proD GUID | Funktionen Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_HARNESSPROD_GUID |
| 20399 | AutomationML GUID (Zubehör) | Funktionen Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_AML_GUID |
| 20464 | Positionsnummer | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_POSNR |
| 20485 | Montageplatte | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_MOUNTINGPLATE |
| 20902 | Funktionsgruppe | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_FUNCTIONGROUP |
| 20903 | Teileart | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_PIECETYPE |
| 20904 | Artikelzuordnung | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Ganzzahlig | Ja, 50 | FUNC_ARTICLE_ASSIGNMENT |
| 20905 | Baugruppe | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_ASSEMBLY |
| 20907 | Ersatzteil | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_SPARE |
| 20908 | Verschleißteil | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_WEAR |
| 20909 | Lebensdauer | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_LIFETIME |
| 20910 | Beanspruchung | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_STRESS |
| 20911 | Beschaffung | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_USAGE |
| 20912 | Schmierung / Wartung | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_MAINTENANCE |
| 20915 | Zusatzfeld Text | Verbindungen Funktionen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_ADDITIONAL_TEXTFIELD |
| 20916 | Zusatzfeld Ja / Nein | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Bool (Wahr / Falsch) | Ja, 50 | FUNC_ARTICLE_ADDITIONAL_BOOLFIELD |
| 20917 | Externe Platzierung | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Bool (Wahr / Falsch) | Ja, 50 | FUNC_ARTICLE_EXTERNAL_PLACEMENT |
| 20918 | Artikel: Montagefläche | Verbindungen Funktionen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Ganzzahlig | Ja, 50 | FUNC_ARTICLE_MOUNTINGSITE |
| 20919 | Bestellnummer | Verbindungen Funktionen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_ORDERNR |
| 20920 | Lieferant | Verbindungen Funktionen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_SUPPLIER |
| 20921 | Hersteller | Verbindungen Funktionen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_MANUFACTURER |
| 20923 | Baugruppenvariante | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_ASSEMBLYVARIANT |
| 20924 | Stücklistengruppe | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLEREF_PARTSLISTGROUP |
| 31008 | Teilmenge / Länge | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_PARTIAL_LENGTH |
| 31014 | Artikelbeschreibung | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Mehrsprachiger Text | Ja, 50 | FUNC_ARTICLE_NOTE |
| 31040 | Teilmenge / Länge in Einheit des Projekts | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_PARTIAL_LENGTH_IN_PROJECT_UNIT |
| 31043 | Teilmenge / Länge mit Einheit des Projekts | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_PARTIAL_LENGTH_WITH_PROJECT_UNIT |
| 31044 | Menge / Teilmenge in Einheit des Projekts | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_QUANTITY_IN_PROJECT_UNIT |
| 31091 | Teilmenge / Länge (vollständig) | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_PARTIAL_LENGTH_FULL |
| 31117 | ERP-Nummer | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_ERPNR |
| 36700 | Geschützte Gruppe: Artikelnummer | Geschützte Gruppen | Einsprachiger Text | Nein | DMPROTECTEDGROUP_PARTNR |
| 44056 | Montageanordnung | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_HOOKUP |

## Betriebsmittel

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 1100 | Anlage (Hauptkennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_PLANT |
| 1101 | Anlage (Unterkennzeichen 1) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLANT1 |
| 1102 | Anlage (Unterkennzeichen 2) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLANT2 |
| 1103 | Anlage (Unterkennzeichen 3) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLANT3 |
| 1104 | Anlage (Unterkennzeichen 4) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLANT4 |
| 1105 | Anlage (Unterkennzeichen 5) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLANT5 |
| 1106 | Anlage (Unterkennzeichen 6) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLANT6 |
| 1107 | Anlage (Unterkennzeichen 7) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLANT7 |
| 1108 | Anlage (Unterkennzeichen 8) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLANT8 |
| 1109 | Anlage (Unterkennzeichen 9) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLANT9 |
| 1120 | Anlage | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Klemmenleisten (in Auswertungen) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLPLANT |
| 1121 | Anlage (Unterkennzeichen, vollständig) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLSUBPLANT |
| 1128 | Anlage (Einzelbestandteil) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_PLANT_PART |
| 1129 | Anlage (sichtbar) | Basisklasse für Funktionen | Einsprachiger Text | Nein | DESIGNATION_PLANT_VISIBLE |
| 1130 | Anlage (Hauptkennzeichen): Beschreibung | Basisklasse für Funktionen Verbindungen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_PLANT_DESCR |
| 1131 | Anlage (Unterkennzeichen 1): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLANT1_DESCR |
| 1132 | Anlage (Unterkennzeichen 2): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLANT2_DESCR |
| 1133 | Anlage (Unterkennzeichen 3): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLANT3_DESCR |
| 1134 | Anlage (Unterkennzeichen 4): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLANT4_DESCR |
| 1135 | Anlage (Unterkennzeichen 5): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLANT5_DESCR |
| 1136 | Anlage (Unterkennzeichen 6): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLANT6_DESCR |
| 1137 | Anlage (Unterkennzeichen 7): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLANT7_DESCR |
| 1138 | Anlage (Unterkennzeichen 8): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLANT8_DESCR |
| 1139 | Anlage (Unterkennzeichen 9): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLANT9_DESCR |
| 1140 | Anlage mit Vorzeichen | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | DESIGNATION_FULLPLANT_WITHPREFIX |
| 1150 | Anlage: Beschreibung | Verbindungen Seiten Basisklasse für Funktionen Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_FULLPLANT_DESCR |
| 1200 | Einbauort (Hauptkennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_LOCATION |
| 1201 | Einbauort (Unterkennzeichen 1) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBLOCATION1 |
| 1202 | Einbauort (Unterkennzeichen 2) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBLOCATION2 |
| 1203 | Einbauort (Unterkennzeichen 3) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBLOCATION3 |
| 1204 | Einbauort (Unterkennzeichen 4) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBLOCATION4 |
| 1205 | Einbauort (Unterkennzeichen 5) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBLOCATION5 |
| 1206 | Einbauort (Unterkennzeichen 6) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBLOCATION6 |
| 1207 | Einbauort (Unterkennzeichen 7) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBLOCATION7 |
| 1208 | Einbauort (Unterkennzeichen 8) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBLOCATION8 |
| 1209 | Einbauort (Unterkennzeichen 9) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBLOCATION9 |
| 1220 | Einbauort | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLLOCATION |
| 1221 | Einbauort (Unterkennzeichen, vollständig) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLSUBLOCATION |
| 1228 | Einbauort (Einzelbestandteil) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_LOCATION_PART |
| 1229 | Einbauort (sichtbar) | Basisklasse für Funktionen | Einsprachiger Text | Nein | DESIGNATION_LOCATION_VISIBLE |
| 1230 | Einbauort (Hauptkennzeichen): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_LOCATION_DESCR |
| 1231 | Einbauort (Unterkennzeichen 1): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBLOCATION1_DESCR |
| 1232 | Einbauort (Unterkennzeichen 2): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBLOCATION2_DESCR |
| 1233 | Einbauort (Unterkennzeichen 3): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBLOCATION3_DESCR |
| 1234 | Einbauort (Unterkennzeichen 4): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBLOCATION4_DESCR |
| 1235 | Einbauort (Unterkennzeichen 5): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBLOCATION5_DESCR |
| 1236 | Einbauort (Unterkennzeichen 6): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBLOCATION6_DESCR |
| 1237 | Einbauort (Unterkennzeichen 7): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBLOCATION7_DESCR |
| 1238 | Einbauort (Unterkennzeichen 8): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBLOCATION8_DESCR |
| 1239 | Einbauort (Unterkennzeichen 9): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBLOCATION9_DESCR |
| 1240 | Einbauort mit Vorzeichen | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | DESIGNATION_FULLLOCATION_WITHPREFIX |
| 1250 | Einbauort: Beschreibung | Verbindungen Seiten Basisklasse für Funktionen Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_FULLLOCATION_DESCR |
| 1300 | Funktionale Zuordnung (Hauptkennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FUNCTIONALASSIGNMENT |
| 1301 | Funktionale Zuordnung (Unterkennzeichen 1) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT1 |
| 1302 | Funktionale Zuordnung (Unterkennzeichen 2) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT2 |
| 1303 | Funktionale Zuordnung (Unterkennzeichen 3) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT3 |
| 1304 | Funktionale Zuordnung (Unterkennzeichen 4) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT4 |
| 1305 | Funktionale Zuordnung (Unterkennzeichen 5) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT5 |
| 1306 | Funktionale Zuordnung (Unterkennzeichen 6) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT6 |
| 1307 | Funktionale Zuordnung (Unterkennzeichen 7) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT7 |
| 1308 | Funktionale Zuordnung (Unterkennzeichen 8) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT8 |
| 1309 | Funktionale Zuordnung (Unterkennzeichen 9) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT9 |
| 1320 | Funktionale Zuordnung | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Klemmenleisten (in Auswertungen) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLFUNCTIONALASSIGNMENT |
| 1321 | Funktionale Zuordnung (Unterkennzeichen, vollständig) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLSUBFUNCTIONALASSIGNMENT |
| 1328 | Funktionale Zuordnung (Einzelbestandteil) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FUNCTIONALASSIGNMENT_PART |
| 1329 | Funktionale Zuordnung (sichtbar) | Basisklasse für Funktionen | Einsprachiger Text | Nein | DESIGNATION_FUNCTIONALASSIGNMENT_VISIBLE |
| 1330 | Funktionale Zuordnung (Hauptkennzeichen): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_FUNCTIONALASSIGNMENT_DESCR |
| 1331 | Funktionale Zuordnung (Unterkennzeichen 1): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT1_DESCR |
| 1332 | Funktionale Zuordnung (Unterkennzeichen 2): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT2_DESCR |
| 1333 | Funktionale Zuordnung (Unterkennzeichen 3): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT3_DESCR |
| 1334 | Funktionale Zuordnung (Unterkennzeichen 4): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT4_DESCR |
| 1335 | Funktionale Zuordnung (Unterkennzeichen 5): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT5_DESCR |
| 1336 | Funktionale Zuordnung (Unterkennzeichen 6): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT6_DESCR |
| 1337 | Funktionale Zuordnung (Unterkennzeichen 7): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT7_DESCR |
| 1338 | Funktionale Zuordnung (Unterkennzeichen 8): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT8_DESCR |
| 1339 | Funktionale Zuordnung (Unterkennzeichen 9): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBFUNCTIONALASSIGNMENT9_DESCR |
| 1340 | Funktionale Zuordnung mit Vorzeichen | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | DESIGNATION_FULLFUNCTIONALASSIGNMENT_WITHPREFIX |
| 1350 | Funktionale Zuordnung: Beschreibung | Verbindungen Seiten Basisklasse für Funktionen Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_FULLFUNCTIONALASSIGNMENT_DESCR |
| 1400 | Aufstellungsort (Hauptkennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_PLACEOFINSTALLATION |
| 1401 | Aufstellungsort (Unterkennzeichen 1) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION1 |
| 1402 | Aufstellungsort (Unterkennzeichen 2) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION2 |
| 1403 | Aufstellungsort (Unterkennzeichen 3) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION3 |
| 1404 | Aufstellungsort (Unterkennzeichen 4) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION4 |
| 1405 | Aufstellungsort (Unterkennzeichen 5) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION5 |
| 1406 | Aufstellungsort (Unterkennzeichen 6) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION6 |
| 1407 | Aufstellungsort (Unterkennzeichen 7) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION7 |
| 1408 | Aufstellungsort (Unterkennzeichen 8) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION8 |
| 1409 | Aufstellungsort (Unterkennzeichen 9) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION9 |
| 1420 | Aufstellungsort | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLPLACEOFINSTALLATION |
| 1421 | Aufstellungsort (Unterkennzeichen, vollständig) | Basisklasse für Funktionen Seiten Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLSUBPLACEOFINSTALLATION |
| 1428 | Aufstellungsort (Einzelbestandteil) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_PLACEOFINSTALLATION_PART |
| 1429 | Aufstellungsort (sichtbar) | Basisklasse für Funktionen | Einsprachiger Text | Nein | DESIGNATION_PLACEOFINSTALLATION_VISIBLE |
| 1430 | Aufstellungsort (Hauptkennzeichen): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_PLACEOFINSTALLATION_DESCR |
| 1431 | Aufstellungsort (Unterkennzeichen 1): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION1_DESCR |
| 1432 | Aufstellungsort (Unterkennzeichen 2): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION2_DESCR |
| 1433 | Aufstellungsort (Unterkennzeichen 3): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION3_DESCR |
| 1434 | Aufstellungsort (Unterkennzeichen 4): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION4_DESCR |
| 1435 | Aufstellungsort (Unterkennzeichen 5): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION5_DESCR |
| 1436 | Aufstellungsort (Unterkennzeichen 6): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION6_DESCR |
| 1437 | Aufstellungsort (Unterkennzeichen 7): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION7_DESCR |
| 1438 | Aufstellungsort (Unterkennzeichen 8): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION8_DESCR |
| 1439 | Aufstellungsort (Unterkennzeichen 9): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPLACEOFINSTALLATION9_DESCR |
| 1440 | Aufstellungsort mit Vorzeichen | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | DESIGNATION_FULLPLACEOFINSTALLATION_WITHPREFIX |
| 1450 | Aufstellungsort: Beschreibung | Verbindungen Seiten Basisklasse für Funktionen Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_FULLPLACEOFINSTALLATION_DESCR |
| 1500 | Dokumentenart (Hauptkennzeichen) | Seiten Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_DOCTYPE |
| 1520 | Dokumentenart | Seiten Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLDOCTYPE |
| 1521 | Dokumentenart (Unterkennzeichen, vollständig) | Seiten Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLSUBDOCTYPE |
| 1528 | Dokumentenart (Einzelbestandteil) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_DOCTYPE_PART |
| 1530 | Dokumentenart (Hauptkennzeichen): Beschreibung | Seiten Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_DOCTYPE_DESCR |
| 1540 | Dokumentenart mit Vorzeichen | Seiten Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLDOCTYPE_WITHPREFIX |
| 1550 | Dokumentenart: Beschreibung | Seiten Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_FULLDOCTYPE_DESCR |
| 1600 | Benutzerdefinierte Struktur (Hauptkennzeichen) | Basisklasse für Funktionen Seiten Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED |
| 1601 | Benutzerdefinierte Struktur (Unterkennzeichen 1) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB1 |
| 1602 | Benutzerdefinierte Struktur (Unterkennzeichen 2) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB2 |
| 1603 | Benutzerdefinierte Struktur (Unterkennzeichen 3) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB3 |
| 1604 | Benutzerdefinierte Struktur (Unterkennzeichen 4) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB4 |
| 1605 | Benutzerdefinierte Struktur (Unterkennzeichen 5) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB5 |
| 1606 | Benutzerdefinierte Struktur (Unterkennzeichen 6) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB6 |
| 1607 | Benutzerdefinierte Struktur (Unterkennzeichen 7) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB7 |
| 1608 | Benutzerdefinierte Struktur (Unterkennzeichen 8) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB8 |
| 1609 | Benutzerdefinierte Struktur (Unterkennzeichen 9) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB9 |
| 1620 | Benutzerdefinierte Struktur | Basisklasse für Funktionen Seiten Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLUSERDEFINED |
| 1621 | Benutzerdefinierte Struktur (Unterkennzeichen, vollständig) | Basisklasse für Funktionen Seiten Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLSUBUSERDEFINED |
| 1628 | Benutzerdefinierte Struktur (Einzelbestandteil) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_PART |
| 1629 | Benutzerdefinierte Struktur (sichtbar) | Basisklasse für Funktionen | Einsprachiger Text | Nein | DESIGNATION_USERDEFINED_VISIBLE |
| 1630 | Benutzerdefinierte Struktur (Hauptkennzeichen): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_USERDEFINED_DESCR |
| 1631 | Benutzerdefinierte Struktur (Unterkennzeichen 1): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB1_DESCR |
| 1632 | Benutzerdefinierte Struktur (Unterkennzeichen 2): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB2_DESCR |
| 1633 | Benutzerdefinierte Struktur (Unterkennzeichen 3): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB3_DESCR |
| 1634 | Benutzerdefinierte Struktur (Unterkennzeichen 4): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB4_DESCR |
| 1635 | Benutzerdefinierte Struktur (Unterkennzeichen 5): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB5_DESCR |
| 1636 | Benutzerdefinierte Struktur (Unterkennzeichen 6): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB6_DESCR |
| 1637 | Benutzerdefinierte Struktur (Unterkennzeichen 7): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB7_DESCR |
| 1638 | Benutzerdefinierte Struktur (Unterkennzeichen 8): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB8_DESCR |
| 1639 | Benutzerdefinierte Struktur (Unterkennzeichen 9): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_USERDEFINED_SUB9_DESCR |
| 1640 | Benutzerdefinierte Struktur mit Vorzeichen | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | DESIGNATION_FULLUSERDEFINED_WITHPREFIX |
| 1650 | Benutzerdefinierte Struktur: Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_FULLUSERDEFINED_DESCR |
| 1700 | Anlagennummer (Hauptkennzeichen) | Basisklasse für Funktionen Verbindungen Seiten Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_INSTALLATIONNUMBER |
| 1720 | Anlagennummer | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLINSTALLATIONNUMBER |
| 1721 | Anlagennummer (Unterkennzeichen, vollständig) | Artikelreferenzen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) Basisklasse für Funktionen Seiten Klemmenleisten (in Auswertungen) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLSUBINSTALLATIONNUMBER |
| 1728 | Anlagennummer (Einzelbestandteil) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_INSTALLATIONNUMBER_PART |
| 1729 | Anlagennummer (sichtbar) | Basisklasse für Funktionen | Einsprachiger Text | Nein | DESIGNATION_INSTALLATIONNUMBER_VISIBLE |
| 1730 | Anlagennummer (Hauptkennzeichen): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_INSTALLATIONNUMBER_DESCR |
| 1740 | Anlagennummer mit Vorzeichen | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | DESIGNATION_FULLINSTALLATIONNUMBER_WITHPREFIX |
| 1750 | Anlagennummer: Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_FULLINSTALLATIONNUMBER_DESCR |
| 1800 | Produkt (Hauptkennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_PRODUCT |
| 1801 | Produkt (Unterkennzeichen 1) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPRODUCT1 |
| 1802 | Produkt (Unterkennzeichen 2) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPRODUCT2 |
| 1803 | Produkt (Unterkennzeichen 3) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPRODUCT3 |
| 1804 | Produkt (Unterkennzeichen 4) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPRODUCT4 |
| 1805 | Produkt (Unterkennzeichen 5) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPRODUCT5 |
| 1806 | Produkt (Unterkennzeichen 6) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPRODUCT6 |
| 1807 | Produkt (Unterkennzeichen 7) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPRODUCT7 |
| 1808 | Produkt (Unterkennzeichen 8) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPRODUCT8 |
| 1809 | Produkt (Unterkennzeichen 9) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_SUBPRODUCT9 |
| 1820 | Produkt | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Klemmenleisten (in Auswertungen) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLPRODUCT |
| 1821 | Produkt (Unterkennzeichen, vollständig) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_FULLSUBPRODUCT |
| 1828 | Produktaspekt (Einzelbestandteil) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DESIGNATION_PRODUCT_PART |
| 1829 | Produkt (sichtbar) | Basisklasse für Funktionen | Einsprachiger Text | Nein | DESIGNATION_PRODUCT_VISIBLE |
| 1830 | Produkt (Hauptkennzeichen): Beschreibung | Basisklasse für Funktionen Verbindungen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_PRODUCT_DESCR |
| 1831 | Produkt (Unterkennzeichen 1): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPRODUCT1_DESCR |
| 1832 | Produkt (Unterkennzeichen 2): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPRODUCT2_DESCR |
| 1833 | Produkt (Unterkennzeichen 3): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPRODUCT3_DESCR |
| 1834 | Produkt (Unterkennzeichen 4): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPRODUCT4_DESCR |
| 1835 | Produkt (Unterkennzeichen 5): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPRODUCT5_DESCR |
| 1836 | Produkt (Unterkennzeichen 6): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPRODUCT6_DESCR |
| 1837 | Produkt (Unterkennzeichen 7): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPRODUCT7_DESCR |
| 1838 | Produkt (Unterkennzeichen 8): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPRODUCT8_DESCR |
| 1839 | Produkt (Unterkennzeichen 9): Beschreibung | Verbindungen Basisklasse für Funktionen Seiten Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | DESIGNATION_SUBPRODUCT9_DESCR |
| 1840 | Produkt mit Vorzeichen | Verbindungen Basisklasse für Funktionen Seiten Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | DESIGNATION_FULLPRODUCT_WITHPREFIX |
| 1850 | Produkt: Beschreibung | Verbindungen Seiten Basisklasse für Funktionen Segmente (Vorplanung) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | DESIGNATION_FULLPRODUCT_DESCR |
| 20000 | Name (identifizierend) | Basisklasse für Funktionen Verbindungen | Einsprachiger Text | Nein | FUNC_IDENTNAME |
| 20001 | Name (vollständig) | Basisklasse für Funktionen Verbindungen Artikelreferenzen | Einsprachiger Text | Nein | FUNC_FULLNAME |
| 20002 | Name (sichtbar) | Basisklasse für Funktionen | Einsprachiger Text | Nein | FUNC_VISIBLENAME |
| 20003 | BMK (übergeordnet, ohne Projektstrukturen) | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DEVICETAG_MAIN |
| 20004 | BMK (untergeordnet, ohne Projektstrukturen) | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DEVICETAG_NESTED |
| 20005 | BMK (identifizierend) | Basisklasse für Funktionen Verbindungen Klemmenleisten (in Auswertungen) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | FUNC_IDENTDEVICETAG |
| 20006 | BMK (vollständig) | Basisklasse für Funktionen Verbindungen Klemmenleisten (in Auswertungen) Stecker (in Auswertungen) Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_FULLDEVICETAG |
| 20008 | BMK (identifizierend, ohne Projektstrukturen) | Verbindungen Basisklasse für Funktionen Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | FUNC_IDENTDEVICETAGWITHOUTSTRUCTURES |
| 20010 | BMK (sichtbar) | Basisklasse für Funktionen | Einsprachiger Text | Nein | FUNC_VISIBLEDEVICETAG |
| 20012 | BMK: Vorziffer | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_PREFIX |
| 20013 | BMK: Kennbuchstabe | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CODE |
| 20014 | BMK: Zähler | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_COUNTER |
| 20015 | BMK: Unterzähler | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_SUFFIX |
| 20016 | BMK (untergeordnet): Vorziffer | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_NESTEDPREFIX |
| 20017 | BMK (untergeordnet): Kennbuchstabe | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_NESTEDCODE |
| 20018 | BMK (untergeordnet): Zähler | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_NESTEDCOUNTER |
| 20019 | BMK (untergeordnet): Unterzähler | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_NESTEDSUFFIX |
| 20020 | BMK (nur Projektstrukturen) | Basisklasse für Funktionen Verbindungen | Einsprachiger Text | Nein | FUNC_DEVICETAG_ONLYSTRUCTURES |
| 20030 | Klemmen- / Steckerkontaktbezeichnung | Funktionen Funktionsschablonen | Einsprachiger Text | Nein | FUNC_PINORTERMINALNUMBER |
| 20032 | Zusätzlicher identifizierender Namensbestandteil | Funktionen Verbindungen | Einsprachiger Text | Nein | FUNC_ADDITIONALIDENTIFYINGNAMEPART |
| 20036 | Anschlussbezeichnung (mit sichtbarem BMK) | Funktionen | Einsprachiger Text | Nein | FUNC_GEDNAMEWITHCONNECTIONDESIGNATION |
| 20037 | SPS-Adresse (mit sichtbarem BMK) | Funktionen | Einsprachiger Text | Nein | FUNC_GEDNAMEWITHPLCADRESS |
| 20211 | BMK (übergeordnet, ohne Projektstrukturen, mit Vorzeichen) | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DEVICETAG_MAIN_WITHSEPARATOR |
| 20212 | BMK (untergeordnet, ohne Projektstrukturen, mit Vorzeichen) | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DEVICETAG_NESTED_WITHSEPARATOR |
| 20215 | BMK (identifizierend, ohne Projektstrukturen, mit Vorzeichen) | Verbindungen Basisklasse für Funktionen Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | FUNC_IDENTDEVICETAGWITHOUTSTRUCTURES_WITHSEPARATOR |
| 20225 | Klemmen- / Steckerkontaktbeschreibung | Funktionen Funktionsschablonen | Einsprachiger Text | Nein | FUNC_PINORTERMINALDESCRIPTION |
| 20316 | Medienschlüssel | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CRAFTCODE |
| 20317 | Schaltkreisnummer | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_CIRCUITNUMBER |
| 20318 | Bauteilnummer | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_COMPONENTNUMBER |
| 20342 | Name (vollständig mit Anschlussbezeichnungen) | Funktionen | Einsprachiger Text | Nein | FUNC_FULLNAME_WITH_CONNECTIONDESIGNATIONS |
| 24300 | Querverweis (konfigurierbar) | Abbruchstellen | Einsprachiger Text | Nein | INTERRUPTIONPOINT_CROSSREFERENCE |
| 24301 | Querverweis | Abbruchstellen | Einsprachiger Text | Nein | INTERRUPTIONPOINT_CROSSREFERENCEONLY |
| 24302 | Querverweis für NFPA-Norm | Abbruchstellen | Einsprachiger Text | Nein | INTERRUPTIONPOINT_CROSSREFERENCE_NFPA |
| 24351 | Gegenziel im Querverweis | Abbruchstellen | Einsprachiger Text | Nein | INTERRUPTIONPOINT_CROSSREFDESTINATION |

## Daten

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 1000 | Strukturkennzeichen | Strukturkennzeichen | Einsprachiger Text | Nein | LOCATION_NAME |
| 1001 | Vollständiges Strukturkennzeichen | Strukturkennzeichen | Einsprachiger Text | Nein | LOCATION_FULLNAME |
| 1002 | Strukturbeschreibung | Strukturkennzeichen | Mehrsprachiger Text | Nein | LOCATION_DESCRIPTION |
| 1003 | Vollständiges Strukturkennzeichen mit Vorzeichen | Strukturkennzeichen | Einsprachiger Text | Nein | LOCATION_FULLNAME_WITHPREFIX |
| 1004 | Kennzeichnungsblock | Strukturkennzeichen | Einsprachiger Text | Nein | LOCATION_TYPENAME |
| 1007 | Strukturbeschreibung 2 | Strukturkennzeichen | Mehrsprachiger Text | Nein | LOCATION_DESCRIPTION_2 |
| 1008 | Strukturbeschreibung 3 | Strukturkennzeichen | Mehrsprachiger Text | Nein | LOCATION_DESCRIPTION_3 |
| 1009 | Strukturbeschreibung Zusatzfeld | Strukturkennzeichen | Mehrsprachiger Text | Ja, 10 | LOCATION_DESCRIPTION_SUPPLEMENTARYFIELD |
| 1010 | Strukturkennzeichen: Übergeordneter Namensbestandteil | Strukturkennzeichen | Einsprachiger Text | Ja, 10 | LOCATION_NAMEPART |
| 1011 | Strukturkennzeichen übergeordnet | Strukturkennzeichen | Einsprachiger Text | Ja, 10 | LOCATION_UPPERLOCATION |
| 1012 | Strukturkennzeichen übergeordnet: Beschreibung | Strukturkennzeichen | Mehrsprachiger Text | Ja, 10 | LOCATION_UPPERLOCATION_DESCRIPTION |
| 3014 | Änderungsschutz | Funktionen Verbindungen Seiten Strukturkennzeichen Segmente (Vorplanung) Abbruchstellen Segmentdefinitionen (Vorplanung) Segmentvorlagen (Vorplanung) | Bool (Wahr / Falsch) | Nein | WRITEPROTECTED |
| 3015 | Änderungsschutz (hierarchisch) | Funktionen Verbindungen Seiten Strukturkennzeichen Segmente (Vorplanung) Platzierungen Abbruchstellen Segmentdefinitionen (Vorplanung) Segmentvorlagen (Vorplanung) | Bool (Wahr / Falsch) | Nein | WRITEPROTECTED_AUTOMATIC |
| 10009 | Projektname (vollständig) | Projekte | Einsprachiger Text | Nein | PROJ_FULL_PROJECTNAME |
| 10010 | Projektpfad | Projekte | Einsprachiger Text | Nein | PROJ_PROJECTPATH |
| 10011 | Projektbeschreibung | Projekte | Mehrsprachiger Text | Nein | PROJ_INSTALLATIONNAME |
| 10013 | Projektnummer | Projekte | Einsprachiger Text | Nein | PROJ_DRAWINGNUMBER |
| 10014 | Kommission | Projekte | Mehrsprachiger Text | Nein | PROJ_COMMISSION |
| 10015 | Firmenname | Projekte | Einsprachiger Text | Nein | PROJ_COMPANYNAME |
| 10016 | Firmenadresse 1 | Projekte | Einsprachiger Text | Nein | PROJ_COMPANYADDRESS1 |
| 10017 | Firmenadresse 2 | Projekte | Einsprachiger Text | Nein | PROJ_COMPANYADDRESS2 |
| 10030 | Fabrikat | Projekte | Mehrsprachiger Text | Nein | PROJ_MAKE |
| 10031 | Projekt: Typ | Projekte | Mehrsprachiger Text | Nein | PROJ_TYPE |
| 10032 | Installationsort | Projekte | Mehrsprachiger Text | Nein | PROJ_LOCATIONOFINSTALLATION |
| 10033 | Teilebesonderheiten | Projekte | Mehrsprachiger Text | Nein | PROJ_PARTFEATURES |
| 10034 | Sonderumwelt | Projekte | Mehrsprachiger Text | Nein | PROJ_ENVIRONMENTALCONSIDERATION |
| 10035 | Standort | Projekte | Mehrsprachiger Text | Nein | PROJ_LOCATION |
| 10036 | Vorschrift | Projekte | Mehrsprachiger Text | Nein | PROJ_REGULATION |
| 10037 | Schutzart | Projekte | Mehrsprachiger Text | Nein | PROJ_DEGOFPROTECTION |
| 10038 | Schaltschränke | Projekte | Mehrsprachiger Text | Nein | PROJ_ENCLOSURES |
| 10039 | Einspeisung | Projekte | Einsprachiger Text | Nein | PROJ_POWERINPUT |
| 10040 | Zuleitung | Projekte | Einsprachiger Text | Nein | PROJ_LEADIN |
| 10041 | Steuerspannung | Projekte | Einsprachiger Text | Nein | PROJ_CONTROLVOLTAGE |
| 10042 | Baujahr | Projekte | Einsprachiger Text | Nein | PROJ_MANUFACTURINGDATE |
| 10045 | Projektpfad (vollständig) | Projekte | Einsprachiger Text | Nein | PROJ_FULL_PROJECTPATH |
| 10065 | Anzahl eingefrorener Seiten pro Seitentyp | Projekte | Ganzzahlig | Ja, 100 | PROJ_FROZENPERPAGETYPE |
| 10069 | Projektvorlage | Projekte | Einsprachiger Text | Nein | PROJ_ORIGIN |
| 10180 | Kundenkennung | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMIDENTIFICATION |
| 10200 | Seitenanzahl pro Seitentyp | Projekte | Ganzzahlig | Ja, 100 | PROJ_COUNTPERPAGETYPE |
| 10300 | Gesamtseitenanzahl | Projekte | Ganzzahlig | Nein | PROJ_PAGECOUNT |
| 10301 | Eingegebene Seiten | Projekte | Ganzzahlig | Nein | PROJ_EDITEDPAGES |
| 10302 | Erzeugte Auswertungsseiten | Projekte | Ganzzahlig | Nein | PROJ_GENERATEDPAGES |
| 10304 | Anzahl der Bauräume | Projekte | Ganzzahlig | Nein | PROJECT_INSTALLATIONSPACECOUNT |
| 10500 | Quellsprache | Projekte | Ganzzahlig | Nein | PROJ_SOURCELANGUAGE |
| 10901 | Zusatzfeld | Projekte | Mehrsprachiger Text | Ja, 1000 | PROJ_SUPPLEMENTARYFIELD |
| 10950 | AutomationML: Firma | Projekte | Einsprachiger Text | Nein | PROJECT_ENTERPRISE |
| 10951 | AutomationML: Werk | Projekte | Einsprachiger Text | Nein | PROJECT_SITE |
| 10952 | AutomationML: Gebäude | Projekte | Einsprachiger Text | Nein | PROJECT_AREA |
| 10953 | AutomationML: Anlage | Projekte | Einsprachiger Text | Nein | PROJECT_PRODUCTION_LINE |
| 10954 | AutomationML: Bereich | Projekte | Einsprachiger Text | Nein | PROJECT_WORK_CELL |
| 11000 | Seitenname | Seiten | Einsprachiger Text | Nein | PAGE_NAME |
| 11009 | Seitenname (vollständig) | Seiten | Einsprachiger Text | Nein | PAGE_FULLNAME |
| 11010 | Seitenname (identifizierend) | Seiten | Einsprachiger Text | Nein | PAGE_IDENTNAME |
| 11011 | Seitenbeschreibung | Seiten | Mehrsprachiger Text | Nein | PAGE_NOMINATIOMN |
| 11030 | Zeichnungsnummer | Seiten | Mehrsprachiger Text | Nein | PAGE_DRAWINGID |
| 11031 | Zusatzfeld Seite | Seiten | Mehrsprachiger Text | Nein | PAGE_ADDITIONALPAGE |
| 11032 | Zusatzfeld Gruppe | Seiten | Mehrsprachiger Text | Nein | PAGE_ADDITIONALGROUP |
| 11033 | Zusatzfeld Blattnummer | Seiten | Einsprachiger Text | Nein | PAGE_ADDITIONALSHEETNUMBER |
| 11036 | Objektkennzeichen | Seiten | Einsprachiger Text | Nein | PAGE_OBJECTCODE |
| 11037 | Gewerk | Seiten | Ganzzahlig | Nein | PAGE_CRAFT |
| 11042 | Seitennummer | Seiten | Einsprachiger Text | Nein | PAGE_NUMBER |
| 11059 | Besondere Anmerkungen des Erstellers | Seiten | Mehrsprachiger Text | Nein | PAGE_ANNOTATION |
| 11066 | Quelle | Seiten | Einsprachiger Text | Nein | PAGE_SOURCE |
| 11067 | Auswertung: Quellprojekt | Seiten | Einsprachiger Text | Nein | PAGE_SOURCE_PROJECT |
| 11090 | Blockeigenschaft | Seiten | Einsprachiger Text | Ja, 100 | PAGE_BLOCK_VALUE |
| 11901 | Zusatzfeld | Seiten | Mehrsprachiger Text | Ja, 1000 | PAGE_SUPPLEMENTARYFIELD |
| 13000 | Formulartyp | Formulare | Einsprachiger Text | Nein | FORM_TYPE |
| 14100 | Konturtyp / Verlauftyp | Konturdefinitionen | Einsprachiger Text | Nein | CONTOURDEFINITION_TYPE |
| 14101 | Kontur: Variante | Konturdefinitionen | Ganzzahlig | Nein | CONTOURDEFINITION_VARIANT |
| 15000 | Symbolbibliothek | Symbolbibliotheken Symbole | Einsprachiger Text | Nein | SYMBLIB_NAME |
| 15011 | Symbolbibliotheksbeschreibung | Symbolbibliotheken | Mehrsprachiger Text | Nein | SYMBLIB_DESC |
| 15015 | Anzahl Symbole | Symbolbibliotheken | Ganzzahlig | Nein | SYMBLIB_SYMBOLCOUNT |
| 15016 | Anzahl gesperrte Symbole | Symbolbibliotheken | Ganzzahlig | Nein | SYMBLIB_LOCKEDSYMBOLCOUNT |
| 15017 | Anzahl Symbolvarianten | Symbolbibliotheken | Ganzzahlig | Nein | SYMBLIB_VARIANTCOUNT |
| 15031 | Komprimierte Symbolbibliothek | Symbolbibliotheken | Bool (Wahr / Falsch) | Nein | SYMBOLLIBRARY_REMOVED_VARIANT |
| 15901 | Zusatzfeld | Symbolbibliotheken | Mehrsprachiger Text | Ja, 1000 | SYMBLIB_SUPPLEMENTARYFIELD |
| 16002 | Nummer | Symbole | Ganzzahlig | Nein | SYMB_NR |
| 16010 | Zielverfolgung | Symbole | Einsprachiger Text | Nein | SYMB_LOGICMODEL |
| 16011 | Symbolbeschreibung | Symbole | Mehrsprachiger Text | Nein | SYMB_DESC |
| 16013 | Zusatzfeld | Symbole | Mehrsprachiger Text | Ja, 99 | SYMB_SUPPLEMENTARYFIELD |
| 16016 | PLT-Typ | Symbole PLT-Stellenplatzierungen Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Ganzzahlig | Nein | SYMB_PLTTYPE |
| 16017 | Gewerk (Funktionsdefinition) | Symbole | Mehrsprachiger Text | Nein | SYMB_CRAFT |
| 16030 | Verbindungsdefinitionspunkt-Symbol: Erste Variante verwenden | Symbole | Bool (Wahr / Falsch) | Nein | SYMB_CABLE_RELATED_VDP_SYMBOL_ID_USE_1ST_VARIANT_ONLY |
| 16043 | Netzverbindend | Symbole | Ganzzahlig | Nein | SYMB_NETCONNECTING |
| 16045 | Transformationspunkt | Symbole | Einsprachiger Text | Ja, 16 | SYMBOL_TRANSFORMATION_POINT |
| 18000 | Name (Formular, Normblatt, Kontur, Verlauf) | Basisklasse für Normblätter und Formulare | Einsprachiger Text | Nein | FORMBASE_NAME |
| 18011 | Beschreibung (Formular, Normblatt, Kontur, Verlauf) | Basisklasse für Normblätter und Formulare | Mehrsprachiger Text | Nein | FORMBASE_DESC |
| 18021 | Firmenkennung | Basisklasse für Normblätter und Formulare | Einsprachiger Text | Nein | FORMBASE_COMPANY |
| 18041 | Zusatzfeld | Basisklasse für Normblätter und Formulare | Mehrsprachiger Text | Ja, 99 | FORMBASE_ADDITION |
| 19070 | Stranganschlussbezeichnung | Stranganschlusssymbole Verbindungen | Einsprachiger Text | Nein | STRANDCONNECTOR_DESIGNATION |
| 19071 | Stranganschlussbeschreibung | Stranganschlusssymbole | Einsprachiger Text | Nein | STRANDCONNECTOR_DESCRIPTION |
| 19072 | Stranganschlussdaten | Stranganschlusssymbole | Einsprachiger Text | Nein | STRANDCONNECTOR_DATA_AUTOMATIC |
| 19201 | Textinhalt | Texte Hyperlinks | Mehrsprachiger Text | Nein | TEXT_CONTENTS |
| 19300 | Platzhalterobjektname | Platzhaltertexte | Einsprachiger Text | Nein | PLACEHOLDER_NAME |
| 19301 | Platzhalterobjekt: Zuletzt gewählter Wertesatz | Platzhaltertexte | Einsprachiger Text | Nein | PLACEHOLDER_LASTUSEDVALUESET |
| 19303 | Wertesatzname | Platzhaltertexte | Einsprachiger Text | Ja, 100 | PLACEHOLDER_VALUESET_NAME |
| 19304 | Datei / Dokument | Hyperlinks Bilder | Einsprachiger Text | Nein | FILE |
| 19305 | Beschreibung (Hyperlink) | Hyperlinks | Einsprachiger Text | Nein | DESCRIPTION |
| 19306 | Bilddatei: Beschreibung | Bilder | Mehrsprachiger Text | Nein | DESCRIPTION_ML |
| 19307 | Eigenschaftsanordnung | Basisklasse für Symbolreferenzen Modell-Ansichten | Einsprachiger Text | Nein | INSTANCE_ACTIVE_PROPERTYSET |
| 19308 | Platzhalterobjektbeschreibung | Platzhaltertexte | Mehrsprachiger Text | Nein | PLACEHOLDER_DESCRIPTION |
| 19310 | X-Koordinate Startpunkt | Linien | Koordinaten (nur X oder Y) | Nein | LINE_START_X |
| 19311 | Y-Koordinate Startpunkt | Linien | Koordinaten (nur X oder Y) | Nein | LINE_START_Y |
| 19312 | X-Koordinate Endpunkt | Linien | Koordinaten (nur X oder Y) | Nein | LINE_END_X |
| 19313 | Y-Koordinate Endpunkt | Linien | Koordinaten (nur X oder Y) | Nein | LINE_END_Y |
| 19320 | Kreisradius | Kreisbögen | Koordinaten (nur X oder Y) | Nein | DMG_ARC_RADIUS |
| 19321 | X-Koordinate des Mittelpunkts | Kreisbögen | Koordinaten (nur X oder Y) | Nein | DMG_ARC_CENTER_X |
| 19322 | Y-Koordinate des Mittelpunkts | Kreisbögen | Koordinaten (nur X oder Y) | Nein | DMG_ARC_CENTER_Y |
| 19323 | Startwinkel | Kreisbögen | Dezimalzahl | Nein | DMG_ARC_STARTANGLE |
| 19324 | Endwinkel | Kreisbögen | Dezimalzahl | Nein | DMG_ARC_ENDANGLE |
| 19340 | X-Koordinate Startpunkt | Rechtecke Makrokästen | Koordinaten (nur X oder Y) | Nein | RECTANGLE_START_X |
| 19341 | Y-Koordinate Startpunkt | Rechtecke Makrokästen | Koordinaten (nur X oder Y) | Nein | RECTANGLE_START_Y |
| 19342 | Drehwinkel für Rechteck | Rechtecke | Dezimalzahl | Nein | RECTANGLE_ANGLE |
| 19380 | Ebene: Beschreibung | Beschreibungstext einer Ebene; mehrsprachige Eingabe ist möglich. | Mehrsprachiger Text | Nein | DMG_LAYER_DESCRIPTION |
| 19400 | Dateiname | Mechanikobjekte | Einsprachiger Text | Nein | DMG_EXTERNALINSTANCE_DOCNAME |
| 19401 | Schaltplanverknüpfung | Mechanikobjekte | Ganzzahlig | Nein | DMG_EXTERNALINSTANCE_HAS_ASSOCIATION |
| 19402 | Gewerk 'Fluid' | Mechanikobjekte | Bool (Wahr / Falsch) | Nein | DMG_EXTERNALINSTANCE_CRAFT_FLUID |
| 19403 | Gewerk 'Elektrotechnik' | Mechanikobjekte | Bool (Wahr / Falsch) | Nein | DMG_EXTERNALINSTANCE_CRAFT_ELECTRICAL |
| 19404 | Objektname | Mechanikobjekte | Einsprachiger Text | Nein | DMG_EXTERNALINSTANCE_OBJNAME |
| 19405 | Gewerk 'Verfahrenstechnik' | Mechanikobjekte | Bool (Wahr / Falsch) | Nein | DMG_EXTERNALINSTANCE_CRAFT_PROCESS |
| 19406 | Information | Mechanikobjekte | Mehrsprachiger Text | Nein | DMG_EXTERNALINSTANCE_NOTE |
| 19407 | Mechanik-Modell ist veraltet | Mechanikobjekte | Bool (Wahr / Falsch) | Nein | DMG_EXTERNALINSTANCE_IS_DIRTY |
| 19500 | Verfasser | PDF-Kommentare | Einsprachiger Text | Nein | COMMENT_AUTHOR |
| 19501 | Status | PDF-Kommentare | Ganzzahlig | Nein | COMMENT_REVIEWSTATE |
| 19600 | X-Position | Türpositionen | Dezimalzahl | Nein | ARTICLEDOORPOSITION_XPOS |
| 19601 | Y-Position | Türpositionen | Dezimalzahl | Nein | ARTICLEDOORPOSITION_YPOS |
| 19602 | Z-Position | Türpositionen | Dezimalzahl | Nein | ARTICLEDOORPOSITION_ZPOS |
| 19603 | Tür: Artikelnummer | Türpositionen | Einsprachiger Text | Nein | ARTICLEDOORPOSITION_PARTNR |
| 19604 | Tür: Variante | Türpositionen | Einsprachiger Text | Nein | ARTICLEDOORPOSITION_VARIANT |
| 19605 | Offset X-Richtung (Bohrbild) | Türpositionen | Dezimalzahl | Nein | ARTICLEPARTCONSTRUCTIONPOSITION_OFFSET_X |
| 19606 | Offset Y-Richtung (Bohrbild) | Türpositionen | Dezimalzahl | Nein | ARTICLEPARTCONSTRUCTIONPOSITION_OFFSET_Y |
| 19607 | Name (Bohrbild) | Türpositionen | Einsprachiger Text | Nein | ARTICLEPARTCONSTRUCTIONPOSITION_NAME |
| 19608 | Offset X-Richtung (Anschlussbild) | Türpositionen | Dezimalzahl | Nein | ARTICLEPARTTERMINALPOSITION_OFFSET_X |
| 19609 | Offset Y-Richtung (Anschlussbild) | Türpositionen | Dezimalzahl | Nein | ARTICLEPARTTERMINALPOSITION_OFFSET_Y |
| 19610 | Offset Z-Richtung (Anschlussbild) | Türpositionen | Dezimalzahl | Nein | ARTICLEPARTTERMINALPOSITION_OFFSET_Z |
| 19611 | Name (Anschlussbild) | Türpositionen | Einsprachiger Text | Nein | ARTICLEPARTTERMINALPOSITION_NAME |
| 19621 | Blockeigenschaft: Wert | QR-Codes | Einsprachiger Text | Nein | QRCODE_BLOCK_VALUE |
| 19650 | X-Position | Montageplattenpositionen | Dezimalzahl | Nein | ARTICLEMOUNTINGPANELPOSITION_XPOS |
| 19651 | Y-Position | Montageplattenpositionen | Dezimalzahl | Nein | ARTICLEMOUNTINGPANELPOSITION_YPOS |
| 19652 | Z-Position | Montageplattenpositionen | Dezimalzahl | Nein | ARTICLEMOUNTINGPANELPOSITION_ZPOS |
| 19653 | Einbauort | Montageplattenpositionen | Ganzzahlig | Nein | ARTICLEMOUNTINGPANELPOSITION_LOCATION |
| 19654 | Winkel | Montageplattenpositionen | Dezimalzahl | Nein | ARTICLEMOUNTINGPANELPOSITION_ANGLE |
| 19655 | Montageplatte: Artikelnummer | Montageplattenpositionen | Einsprachiger Text | Nein | ARTICLEMOUNTINGPANELPOSITION_PARTNR |
| 19656 | Montageplatte: Variante | Montageplattenpositionen | Einsprachiger Text | Nein | ARTICLEMOUNTINGPANELPOSITION_VARIANT |
| 19700 | X-Position | Sperrflächenpositionen | Dezimalzahl | Nein | ARTICLELOCKEDMOUNTINGAREAPOSITION_XPOS |
| 19701 | Y-Position | Sperrflächenpositionen | Dezimalzahl | Nein | ARTICLELOCKEDMOUNTINGAREAPOSITION_YPOS |
| 19702 | Vorderseite | Sperrflächenpositionen | Bool (Wahr / Falsch) | Nein | ARTICLELOCKEDMOUNTINGAREAPOSITION_FRONTSIDE |
| 19703 | Breite | Sperrflächenpositionen | Dezimalzahl | Nein | ARTICLELOCKEDMOUNTINGAREAPOSITION_WIDTH |
| 19704 | Höhe | Sperrflächenpositionen | Dezimalzahl | Nein | ARTICLELOCKEDMOUNTINGAREAPOSITION_HEIGHT |
| 19705 | Typ | Sperrflächenpositionen | Ganzzahlig | Nein | ARTICLELOCKEDMOUNTINGAREAPOSITION_TYPE |
| 19750 | X-Position | Tragschienenpositionen auf Sammelschienenadaptern | Dezimalzahl | Nein | ARTICLESUPPORTBARPOSITION_XPOS |
| 19751 | Y-Position | Tragschienenpositionen auf Sammelschienenadaptern | Dezimalzahl | Nein | ARTICLESUPPORTBARPOSITION_YPOS |
| 19752 | Z-Position | Tragschienenpositionen auf Sammelschienenadaptern | Dezimalzahl | Nein | ARTICLESUPPORTBARPOSITION_ZPOS |
| 19753 | Tragschiene: Artikelnummer | Tragschienenpositionen auf Sammelschienenadaptern | Einsprachiger Text | Nein | ARTICLESUPPORTBARPOSITION_PARTNR |
| 19754 | Tragschiene: Variante | Tragschienenpositionen auf Sammelschienenadaptern | Einsprachiger Text | Nein | ARTICLESUPPORTBARPOSITION_VARIANT |
| 19755 | Länge | Tragschienenpositionen auf Sammelschienenadaptern | Dezimalzahl | Nein | ARTICLESUPPORTBARPOSITION_LENGTH |
| 19756 | Erforderlich | Zubehör (am Artikel eingetragen) | Bool (Wahr / Falsch) | Nein | ARTICLEACCESSORYPOSITION_NESSECCARY |
| 20011 | Funktionstext | Funktionen Strukturkästen Klemmenleisten (in Auswertungen) Stecker (in Auswertungen) SPS-Adressen (in Auswertungen) | Mehrsprachiger Text | Nein | FUNC_TEXT |
| 20024 | Montageort (beschreibend) | Strukturkästen Funktionen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) | Mehrsprachiger Text | Nein | FUNC_MOUNTINGLOCATION |
| 20025 | Graviertext | Funktionen Strukturkästen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Mehrsprachiger Text | Nein | FUNC_GRAVINGTEXT |
| 20027 | Technische Kenngrößen | Funktionen Strukturkästen Funktionsschablonen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | FUNC_TECHNICAL_CHARACTERISTIC |
| 20031 | Funktionstext (automatisch) | Funktionen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Klemmenleisten (in Auswertungen) Stecker (in Auswertungen) Kabel (in Auswertungen) SPS-Adressen (in Auswertungen) | Mehrsprachiger Text | Nein | FUNC_TEXT_AUTOMATIC |
| 20033 | Ausgabegruppe | Funktionen Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | FUNC_OUTPUTASSEMBLY |
| 20034 | Etage | Funktionen | Ganzzahlig | Nein | FUNC_TERMINALLEVEL |
| 20040 | Kabel / Paket: Typ | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLETYPE |
| 20041 | Kabel / Paket: Länge mit Einheit | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLELENGTH |
| 20042 | Kabel / Paket: Verbindungsanzahl | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLEWIRECOUNT |
| 20043 | Kabel / Paket: Querschnitt / Durchmesser | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLEWIRECROSSSECTION |
| 20044 | Kabel: Spannung | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLEVOLTAGE |
| 20045 | Bemerkung | Potenzialdefinitionspunkte Funktionen Kabel (in Auswertungen) Verbindungen Modell-Ansichten Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Mehrsprachiger Text | Nein | FUNC_COMMENT |
| 20047 | Kontakt- / Spulenindex | Funktionen | Einsprachiger Text | Nein | FUNC_COILINDEX |
| 20053 | Kabel / Paket: Verbindungsanzahl und Querschnitt / Durchmesser | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLEWIRECOUNTANDCROSSSECTION |
| 20054 | Spule: Spannung | Funktionen | Einsprachiger Text | Nein | FUNC_COILVOLTAGE |
| 20055 | Kontakt: Schaltleistung | Funktionen | Einsprachiger Text | Nein | FUNC_CONTACTORPOWER |
| 20056 | Stecker: Anzahl Steckerkontakte | Funktionen | Einsprachiger Text | Nein | FUNC_PINCOUNT |
| 20057 | Anschlussquerschnitt | Funktionen | Einsprachiger Text | Nein | FUNC_CONNECTIONCROSSSECTION |
| 20067 | Kabel / Paket: Bezeichnung in der Grafik | Funktionen Kabel (in Auswertungen) | Mehrsprachiger Text | Nein | FUNC_CABLE_DESIGNATION |
| 20068 | Kabel / Paket: Einheit für Querschnitt / Durchmesser | Funktionen Kabel (in Auswertungen) | Ganzzahlig | Nein | FUNC_CABLE_CROSSSECTION_UNIT |
| 20070 | Kabel / Paket: Querschnitt / Durchmesser mit Einheit | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_AUTO_CROSSSECTION_UNIT |
| 20071 | Kabel / Paket: Verbindungsanzahl, Querschnitt / Durchmesser mit Einheit | Kabel (in Auswertungen) Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_AUTO_WIRECOUNT_CROSSSECTION_UNIT |
| 20072 | Kabel / Paket: Verbindungsanzahl, Querschnitt / Durchmesser mit Einheit, Länge mit Einheit | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLE_AUTO_WIRECOUNT_CROSSSECTION_UNIT_LENGTH_UNIT |
| 20073 | Kabel / Paket: Typ, Verbindungsanzahl, Querschnitt / Durchmesser mit Einheit | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLE_AUTO_TYPE_WIRECOUNT_CROSSSECTION_UNIT |
| 20074 | Kabel / Paket: Typ, Verbindungsanzahl, Querschnitt / Durchmesser mit Einheit, Länge mit Einheit | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLE_AUTO_TYPE_WIRECOUNT_CROSSSECTION_UNIT_LENGTH_UNIT |
| 20078 | Kabel / Paket: Länge in Einheit des Projekts | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLE_AUTO_LENGTH |
| 20079 | Kabel / Paket: Länge mit Einheit des Projekts | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLE_AUTO_LENGTH_UNIT |
| 20080 | Kabel / Paket: Verschraubungsgröße | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_SCREWCONNECTIONSIZE |
| 20081 | Kabel / Paket: Absetzlänge Quelle | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_STRIPPINGLENGTHSOURCE |
| 20082 | Kabel / Paket: Absetzlänge Ziel | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_STRIPPINGLENGTHDESTINATION |
| 20110 | Anzahl der Funktionen | Verbindungen Verbindungsdefinitionspunkte Funktionen | Einsprachiger Text | Nein | FUNC_SINGLE_LINE_FUNCTION_COUNT |
| 20120 | Funktionstext (gemeinsam) | Funktionen | Mehrsprachiger Text | Nein | FUNC_TEXT_COMMON |
| 20124 | Kabel / Paket: Einheit für Querschnitt / Durchmesser (automatisch) | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_CROSSSECTION_AUTO_UNIT |
| 20125 | Kabel / Paket: Alternativer Querschnitt / Durchmesser | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLE_ALTERNATE_WIRECROSSSECTION |
| 20126 | Kabel / Paket: Einheit für alternativen Querschnitt / Durchmesser | Funktionen Kabel (in Auswertungen) | Ganzzahlig | Nein | FUNC_CABLE_ALTERNATE_CROSSSECTION_UNIT |
| 20127 | Kabel / Paket: Alternativer Querschnitt / Durchmesser mit Einheit | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_AUTO_ALTERNATE_CROSSSECTION_UNIT |
| 20166 | Potenzialtyp der Anschlüsse | Funktionen | Ganzzahlig | Nein | FUNC_POTENTIALTYPE_TERMINALPOINTS |
| 20168 | Symbolnummer | Basisklasse für Funktionen Potenzialdefinitionspunkte | Einsprachiger Text | Nein | FUNC_SYMB_NUMBER |
| 20183 | Anschlussbezeichnung (mit Steckerbezeichnung (automatisch)) | Funktionen SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | FUNC_PLCAUTOPLUG_AND_CONNPTDESIGNATION |
| 20185 | Meldungstext | Funktionen Verbindungen Abbruchstellen | Einsprachiger Text | Nein | FUNCTION_MESSAGETEXT |
| 20197 | Topologie: Quelle der Strecke | Strecken (Topologie) | Einsprachiger Text | Nein | CABLINGSEGMENT_SOURCE |
| 20201 | Blockeigenschaft | Funktionen Abbruchstellen Verbindungen Verbindungsdefinitionspunkte PLT-Stellenplatzierungen Segmente (Vorplanung) Strukturkästen Potenzialdefinitionspunkte | Einsprachiger Text | Ja, 100 | FUNC_BLOCK_VALUE |
| 20208 | X-Koordinate | Artikelreferenzen Funktionen | Koordinaten (nur X oder Y) | Nein | FUNC_ARTICLEPLACEMENT_GRIPPEROFFSET_X |
| 20209 | Y-Koordinate | Artikelreferenzen Funktionen | Koordinaten (nur X oder Y) | Nein | FUNC_ARTICLEPLACEMENT_GRIPPEROFFSET_Y |
| 20219 | IDs für netzbasierte Verbindungen | Funktionen Potenzialdefinitionspunkte | Einsprachiger Text | Nein | FUNCTION_NETBASEDWIRING_IDS |
| 20220 | Hauptklemme | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_MAIN |
| 20221 | Breite | Funktionen Strukturkästen Rechtecke Makrokästen | Koordinaten (nur X oder Y) | Nein | RECTANGLE_WIDTH |
| 20222 | Höhe | Strukturkästen Funktionen Rechtecke Makrokästen | Koordinaten (nur X oder Y) | Nein | RECTANGLE_HEIGHT |
| 20228 | Nebenklemme | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_SECONDARY |
| 20235 | Bauraumbeschreibung | 3D-Platzierungen Artikelreferenzen | Mehrsprachiger Text | Nein | FUNC_ARTICLEPLACEMENT_INSTALLATIONSPACE_DESCRIPTION |
| 20237 | Topologie: Verlegeweg | Funktionen Verbindungen | Einsprachiger Text | Nein | CABLING_PATH |
| 20240 | Topologie: Durchlauflänge | Funktionen | Einsprachiger Text | Nein | FUNC_CABLING_LENGTH |
| 20241 | Topologie: Anschlusslängenzuschlag | Funktionen | Einsprachiger Text | Nein | FUNC_CABLING_CP_ADDITIONAL_LENGTH |
| 20243 | Kabel: Anschlusslänge Quelle | Funktionen | Einsprachiger Text | Nein | FUNC_SOURCE_CP_LENGTH |
| 20245 | Topologie: Struktur | Funktionen Verbindungen | Einsprachiger Text | Nein | CABLING_STRUCTURE |
| 20246 | Kabel: Anschlusslänge Ziel | Funktionen | Einsprachiger Text | Nein | FUNC_DEST_CP_LENGTH |
| 20248 | Topologie: Streckenquerschnitt | Strecken (Topologie) | Einsprachiger Text | Nein | CABLINGSEGMENT_CROSSSECTION |
| 20250 | Topologie: Durchlauflänge (automatisch) | Strecken (Topologie) | Einsprachiger Text | Nein | CABLINGSEGMENT_LENGTH_AUTO |
| 20252 | Teilklemmenquerverweis | Funktionen | Einsprachiger Text | Nein | FUNC_DISTRIBUTED_TERMINAL_CROSSREFERENCE |
| 20256 | Topologie: Ziel der Strecke | Strecken (Topologie) | Einsprachiger Text | Nein | CABLINGSEGMENT_DESTINATION |
| 20257 | Kabel / Paket: Länge (vollständig) | Funktionen Kabel (in Auswertungen) | Einsprachiger Text | Nein | FUNC_CABLELENGTH_FULL |
| 20261 | Bauraumname (der Artikelplatzierung) | 3D-Platzierungen Artikelreferenzen | Einsprachiger Text | Nein | FUNC_ARTICLEPLACEMENT_INSTALLATIONSPACE |
| 20280 | Teilklemme | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_DISTRIBUTED_TERMINAL |
| 20285 | Zustand | Funktionen | Mehrsprachiger Text | Ja, 10 | FUNC_FUNCTIONALTEXT |
| 20287 | EPLAN Data Portal: Konfigurator | Funktionen | Einsprachiger Text | Nein | FUNCTION_EDP_ADAPTER |
| 20288 | EPLAN Data Portal: Konfigurator-Zusatzfeld | Funktionen | Einsprachiger Text | Nein | FUNCTION_EDP_ADAPTER_ADDITIONAL |
| 20294 | Betriebsmittelgruppe | Funktionen Verbindungen Verbindungsdefinitionspunkte | Ganzzahlig | Nein | FUNC_DEVICETYPE_MANUAL |
| 20300 | Querverweis (Haupt- / Nebenfunktion) | Strukturkästen Funktionen | Einsprachiger Text | Nein | FUNC_CROSSREFERENCE |
| 20301 | Übersichtsquerverweis | Funktionen | Einsprachiger Text | Nein | FUNC_PLCPAIRCROSSREFERENCE |
| 20302 | Manueller Querverweis | Funktionen Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | FUNC_MANUALCROSSREFERENCE |
| 20304 | Querverweis auf Steckergegenstück | Funktionen | Einsprachiger Text | Nein | FUNC_CROSSREFERENCETODIRECTLYCONNECTEDCOUNTERPART |
| 20305 | Hauptplatzierung | Strukturkästen | Bool (Wahr / Falsch) | Nein | LOCATIONBOX_MAINPLACEMENT |
| 20306 | Querverweis (nur Hauptfunktionen) | Funktionen | Einsprachiger Text | Nein | FUNC_MAINCROSSREFERENCE |
| 20330 | Druckniveau | Funktionen | Einsprachiger Text | Nein | FUNC_PRESSURESTANDARD |
| 20332 | Topologie: Füllgrad | Strecken (Topologie) | Einsprachiger Text | Nein | CABLINGSEGMENT_FILLING |
| 20343 | Topologie: Einpolig erzeugt | Verlegeverbindungen (Topologie) | Bool (Wahr / Falsch) | Nein | CONNECTIONCABLING_SINGLELINE_CREATED |
| 20344 | Topologie: Verlegearten | Funktionen Verbindungen | Mehrsprachiger Text | Nein | CABLING_DUCT_TYPES |
| 20345 | Topologie: Verlegeart (Strecke) | Strecken (Topologie) | Mehrsprachiger Text | Nein | CABLINGSEGMENT_DUCT_TYPE |
| 20346 | Positionsnummernliste | Funktionen | Einsprachiger Text | Nein | FUNC_ARTICLE_POSNR_LIST |
| 20347 | Topologie: Höhendifferenz | Strecken (Topologie) | Einsprachiger Text | Nein | CABLINGSEGMENT_HEIGHT_DIFF |
| 20348 | Topologie: Richtung für Höhendifferenz | Strecken (Topologie) Symbole | Ganzzahlig | Nein | CABLINGSEGMENT_DIRECTION_OF_HEIGHT_DIFF |
| 20350 | Manuelle Stegbrücken (intern) | Funktionen | Einsprachiger Text | Ja, 1 | FUNC_TERMINAL_JUMPER_INTERN |
| 20351 | Manuelle Stegbrücken (extern) | Funktionen | Einsprachiger Text | Ja, 1 | FUNC_TERMINAL_JUMPER_EXTERN |
| 20359 | BMK-ID | Funktionen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Verbindungen | Einsprachiger Text | Nein | FUNC_MODULE_ID |
| 20366 | Sicherheitsprojekt | Funktionen | Einsprachiger Text | Nein | FUNC_SAFETY_PROJECT |
| 20375 | Anschlussmaß (alle) | Funktionen | Einsprachiger Text | Nein | FUNC_ALLCONNECTIONDIMENSIONS |
| 20376 | Kabel: Quelle | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_SOURCE |
| 20377 | Kabel: Ziel | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_DESTINATION |
| 20383 | Kabel: Quelle (mit Steckerbezeichnung) | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_SOURCE_WITH_PLCPLUG_DESIGNATION |
| 20385 | Kabel: Ziel (mit Steckerbezeichnung) | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_DESTINATION_WITH_PLCPLUG_DESIGNATION |
| 20389 | Funktionsschablone: Beschreibung | Funktionen Verbindungen | Mehrsprachiger Text | Nein | FUNCTION_TEMPLATE_DESCRIPTION |
| 20406 | Steckerbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCPLUG_DESIGNATION_MANUAL |
| 20429 | Funktionsschablone: Schablonenverbund (allpolig) | Funktionen Verbindungen | Einsprachiger Text | Nein | FUNCTION_TEMPLATE_COMBINATION |
| 20431 | Steckerbezeichnung (automatisch) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCPLUG_DESIGNATION_AUTOMATIC |
| 20435 | Anschlussbezeichnung (mit Steckerbezeichnung) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCPLUG_AND_CONNPTDESIGNATION |
| 20443 | Bus-Schnittstelle: Name (mit Steckerbezeichnung) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCBUSINTERFACENAME_AND_PLCPLUG |
| 20450 | Legendenposition | Funktionen Artikelreferenzen | Ganzzahlig | Nein | FUNC_ARTICLEPLACEMENT_CAPTIONID |
| 20451 | Drehwinkel | Artikelreferenzen Funktionen | Dezimalzahl | Nein | FUNC_ARTICLEPLACEMENT_ROTATIONANGLE |
| 20458 | X-Ausdehnung des Artikels | Funktionen Artikelreferenzen | Dezimalzahl | Nein | FUNC_ARTICLEPLACEMENT_PLACEMENT_XSIZE |
| 20459 | Y-Ausdehnung des Artikels | Funktionen Artikelreferenzen | Dezimalzahl | Nein | FUNC_ARTICLEPLACEMENT_PLACEMENT_YSIZE |
| 20460 | Technische Kenngrößen (der Hauptfunktion) | Funktionen Artikelreferenzen | Einsprachiger Text | Nein | FUNC_ARTICLEPLACEMENT_DEVICE_TECHNICAL_CHARACTERISTIC |
| 20461 | Funktionsdefinition (der Hauptfunktion) | Funktionen Artikelreferenzen | Mehrsprachiger Text | Nein | FUNC_ARTICLEPLACEMENT_DEVICE_FUNCTIONDEFINITION |
| 20462 | Montageort (der Montageplatte) | Funktionen Artikelreferenzen | Mehrsprachiger Text | Nein | FUNC_ARTICLEPLACEMENT_MOUNTINGPLATE_MOUNTINGLOCATION |
| 20463 | Funktionstext (der Hauptfunktion) | Funktionen Artikelreferenzen | Mehrsprachiger Text | Nein | FUNC_ARTICLEPLACEMENT_DEVICE_FUNCTIONTEXT |
| 20465 | Montageplatte (identifizierend) | Funktionen Artikelreferenzen | Einsprachiger Text | Nein | FUNC_ARTICLEPLACEMENT_MOUNTINGPLATE |
| 20466 | Gewerk | Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Gerätelisteneinträge Segmentvorlagen (Vorplanung) Segmente (Vorplanung) Segmentdefinitionen (Vorplanung) | Ganzzahlig | Nein | FUNC_CRAFT |
| 20467 | Untergewerk | Verbindungen Funktionen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Gerätelisteneinträge Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Mehrsprachiger Text | Nein | FUNC_SUBCRAFT |
| 20468 | Makro | Verbindungen Verbindungsdefinitionspunkte Funktionen | Einsprachiger Text | Nein | FUNC_MACRO |
| 20469 | Gerätegruppiert | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_INDEVICEGROUP |
| 20480 | BMK | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_IDENTNAME |
| 20481 | Artikelnummer | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_PARTNO |
| 20482 | Stückzahl / Menge | Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen | Ganzzahlig | Nein | ARTICLEREF_COUNT |
| 20483 | Stückzahl / Menge (platziert) | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLEREF_COUNT_PLACED |
| 20484 | Stückzahl / Menge (nicht-platziert) | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLEREF_COUNT_NOTPLACED |
| 20486 | Datensatztyp | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLEREF_PARTTYPE |
| 20487 | Positionsnummer | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_POSNR |
| 20488 | Artikelvariante | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_VARIANT |
| 20489 | Funktionsgruppe | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_FUNCTIONGROUP |
| 20490 | Teileart | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_PIECETYPE |
| 20491 | Artikelzuordnung | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLEREF_ASSIGNMENT |
| 20492 | Baugruppe | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_ASSEMBLY |
| 20493 | Artikel ist Bestandteil eines Moduls | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | ARTICLEREF_MODULE_PART |
| 20494 | In Stückliste unterdrücken (wenn gefiltert) | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | ARTICLEREF_SUPPRESSINPARTSLIST |
| 20495 | Projektartikel | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | ARTICLEREF_PROJECTARTICLE |
| 20496 | Teilmenge / Länge | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_PARTIAL_LENGTH |
| 20497 | Teilmenge / Länge: Wert | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Dezimalzahl | Nein | ARTICLE_PARTIAL_LENGTH_VALUE |
| 20498 | Einheit für Teilmenge / Länge | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLE_PARTIAL_LENGTH_UNIT |
| 20501 | Zusatzfeld Text | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_ADDITIONAL_TEXTFIELD |
| 20502 | Zusatzfeld Ja / Nein | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | ARTICLEREF_ADDITIONAL_BOOLFIELD |
| 20505 | Teilmenge / Länge in Einheit des Projekts | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_PARTIAL_LENGTH_IN_PROJECT_UNIT |
| 20506 | Teilmenge / Länge mit Einheit des Projekts | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_PARTIAL_LENGTH_WITH_PROJECT_UNIT |
| 20507 | Menge / Teilmenge in Einheit des Projekts | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_QUANTITY_IN_PROJECT_UNIT |
| 20508 | Artikel einer Artikeldefinition | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | ARTICLEREF_ARTICLEDEFINITION |
| 20509 | Stückzahl / Menge (nicht-platziert, 3D) | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLEREF_COUNT_NOTPLACED_3D |
| 20510 | Teilmenge / Länge (vollständig) | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_PARTIAL_LENGTH_FULL |
| 20511 | Baugruppenstruktur | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_ASSEMBLYSTRUCTURE |
| 20512 | Baugruppenvariante | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_ASSEMBLYVARIANT |
| 20856 | Stecker: Codierung | Funktionen | Einsprachiger Text | Nein | FUNC_PLUGCODING |
| 20863 | Rohrklasse | Funktionen Funktionsschablonen | Einsprachiger Text | Nein | FUNC_PIPECLASS |
| 20864 | Werkstoff / Material | Funktionen | Mehrsprachiger Text | Nein | FUNC_MATERIAL |
| 20865 | Nenndruckstufe | Funktionen | Einsprachiger Text | Nein | FUNC_PRESSURELEVEL |
| 20866 | Schlauchleitungslänge | Funktionen | Einsprachiger Text | Nein | FUNC_HOSE_LENGTH |
| 20867 | Schlauchleitungskennzeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_HOSE_MARKING |
| 20868 | Herstelldatum | Funktionen | Uhrzeit / Datum | Nein | FUNC_HOSE_CREATEDATE |
| 20869 | Verdrehwinkel | Funktionen | Einsprachiger Text | Nein | FUNC_ROTATION_ANGLE |
| 20870 | Max. Betriebsdruck | Funktionen | Einsprachiger Text | Nein | FUNC_PRESSURE |
| 20901 | Zusatzfeld | Funktionen Strukturkästen Klemmenleisten (in Auswertungen) Abbruchstellen Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Mehrsprachiger Text | Ja, 1000 | FUNC_SUPPLEMENTARYFIELD |
| 20913 | Gewerk der Artikelreferenz | Projekte Funktionen | Ganzzahlig | Ja, 50 | PROJ_ARTICLEREF_CRAFT |
| 20914 | Untergewerk der Artikelreferenz | Funktionen Projekte | Mehrsprachiger Text | Ja, 50 | PROJ_ARTICLEREF_SUBCRAFT |
| 20930 | Meldungen in der Meldungsverwaltung | Funktionen Abbruchstellen Verbindungen Verbindungsdefinitionspunkte Artikelreferenzen Artikel (eingelagert) Segmente (Vorplanung) | Bool (Wahr / Falsch) | Nein | MESSAGEMGMT_MESSAGES |
| 21001 | Symbol | Funktionsschablonen | Einsprachiger Text | Nein | FUNCTEMPLATE_SYMBOL |
| 21003 | Eigensicher | Funktionsschablonen | Bool (Wahr / Falsch) | Nein | FUNCTEMPLATE_INTRINSICALLYSAFE |
| 21004 | Funktionsdefinition | Funktionsschablonen | Einsprachiger Text | Nein | FUNCTEMPLATE_FUNCTIONDEFINITION |
| 21006 | Sicherheitsfunktion | Funktionsschablonen | Bool (Wahr / Falsch) | Nein | FUNCTEMPLATE_SAFETYRELEVANT |
| 21008 | Symbolmakro | Funktionsschablonen | Einsprachiger Text | Nein | FUNCTEMPLATE_SYMBOLMACRO |
| 21009 | Artikelnummer / Name | Zubehör (am Artikel eingetragen) | Einsprachiger Text | Nein | ARTICLEACCESSORYPOSITION_PARTNR |
| 21010 | Variante | Zubehör (am Artikel eingetragen) | Einsprachiger Text | Nein | ARTICLEACCESSORYPOSITION_VARIANT |
| 21011 | Datensatztyp | Zubehör (am Artikel eingetragen) | Ganzzahlig | Nein | ARTICLEACCESSORYPOSITION_PARTTYPE |
| 21012 | Zubehörplatzierung | Zubehör (am Artikel eingetragen) | Einsprachiger Text | Nein | ARTICLEACCESSORYPOSITION_ACCESSORYPLACEMENT |
| 21013 | Zubehörplatzierung | Zubehörlisteneinträge | Einsprachiger Text | Nein | ARTICLEACCESSORYLISTPOSITION_ACCESSORYPLACEMENT |
| 21014 | Einbauvariante | Zubehörartikel in Zubehörplatzierungen | Einsprachiger Text | Nein | ARTICLEACCESSORYPLACEMENTPOSITION_NAME |
| 21015 | Bezugspunkt | Zubehörartikel in Zubehörplatzierungen | Ganzzahlig | Nein | ARTICLEACCESSORYPLACEMENTPOSITION_REFERENCEPOINT |
| 21016 | Drehung | Zubehörartikel in Zubehörplatzierungen | Dezimalzahl | Nein | ARTICLEACCESSORYPLACEMENTPOSITION_ROTATION |
| 21017 | Versatz X-Richtung | Zubehörartikel in Zubehörplatzierungen | Dezimalzahl | Nein | ARTICLEACCESSORYPLACEMENTPOSITION_XSPACING |
| 21018 | Versatz Y-Richtung | Zubehörartikel in Zubehörplatzierungen | Dezimalzahl | Nein | ARTICLEACCESSORYPLACEMENTPOSITION_YSPACING |
| 21019 | Versatz Z-Richtung | Zubehörartikel in Zubehörplatzierungen | Dezimalzahl | Nein | ARTICLEACCESSORYPLACEMENTPOSITION_ZSPACING |
| 21020 | Verschiebbar | Zubehörartikel in Zubehörplatzierungen | Bool (Wahr / Falsch) | Nein | ARTICLEACCESSORYPLACEMENTPOSITION_MOVABLE |
| 22000 | Gewerk 'Verfahrenstechnik' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_PROCESS |
| 22001 | Artikelnummer | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PARTNR |
| 22002 | Typnummer | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_TYPENR |
| 22003 | Bestellnummer | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_ORDERNR |
| 22004 | Artikel: Bezeichnung 1 | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | ARTICLE_DESCR1 |
| 22005 | Artikel: Bezeichnung 2 | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | ARTICLE_DESCR2 |
| 22006 | Artikel: Bezeichnung 3 | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | ARTICLE_DESCR3 |
| 22007 | Hersteller | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_MANUFACTURER |
| 22008 | Lieferant | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_SUPPLIER |
| 22009 | Beschreibung | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | ARTICLE_NOTE |
| 22010 | Grafikmakro (inkl. Verzeichnis) | Artikelreferenzen Artikel (eingelagert) Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_MACRO |
| 22012 | Höhe | Artikel (eingelagert) Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen | Dezimalzahl | Nein | ARTICLE_HEIGHT |
| 22013 | Breite | Artikel (eingelagert) Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen | Dezimalzahl | Nein | ARTICLE_WIDTH |
| 22014 | Tiefe | Artikel (eingelagert) Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen | Dezimalzahl | Nein | ARTICLE_DEPTH |
| 22017 | Technische Kenngrößen | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CHARACTERISTICS |
| 22018 | Grafikmakro | Artikelreferenzen Artikel (eingelagert) Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_MACRONAME |
| 22019 | Bus-Koppler / Kopfstation | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_PLCISBUSCOUPLER |
| 22020 | CPU | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_PLCISCPU |
| 22022 | Montagefläche | Artikel (eingelagert) Funktionen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLE_MOUNTINGSITE |
| 22023 | Datensatztyp | Artikel (eingelagert) Adressangaben bei Artikeln Zubehörlisten Zubehörplatzierungen Bohrbilder Anschlussbilder | Ganzzahlig | Nein | ARTICLE_PARTTYPE |
| 22024 | Variante | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_VARIANT |
| 22025 | Zubehörkennung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_ACCESSORYID |
| 22026 | Funktionsgruppe | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_FUNCTIONGROUP |
| 22027 | Teileart | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PIECETYPE |
| 22028 | Produktuntergruppe | Artikel (eingelagert) Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen | Ganzzahlig | Nein | ARTICLE_PRODUCTSUBGROUP |
| 22030 | Kabeltyp / Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CABLETYPE |
| 22031 | Verbindungsanzahl | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CABLEWIRECOUNT |
| 22032 | Verbindungsquerschnitt / -durchmesser | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CABLEWIRECROSSSECTION |
| 22033 | Spannung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_VOLTAGE |
| 22034 | Kabeldarstellungsplanformular | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CABLEDISPLAYFORM |
| 22035 | Stecker: Anzahl Steckerkontakte | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PINCOUNT |
| 22036 | Anschlussquerschnitt | Artikel (eingelagert) | Mehrsprachiger Text | Nein | ARTICLE_CONNECTIONCROSSSECTION |
| 22037 | Gerätebeschreibung: Dateiname | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICE_ID |
| 22038 | Objektbeschreibung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCOBJECT_DESCRIPTION |
| 22039 | Anschlussbezeichnung am Artikel | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CONNECTIONDESIGNATION |
| 22040 | Baugruppe verteilt platzieren | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_ASSEMBLY_POS_PLACE_SPREADED |
| 22041 | Produktgruppe | Artikel (eingelagert) Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen | Ganzzahlig | Nein | ARTICLE_PRODUCTGROUP |
| 22042 | Mengeneinheit | Artikel (eingelagert) | Mehrsprachiger Text | Nein | ARTICLE_QUANTITYUNIT |
| 22043 | Preiseinheit | Artikel (eingelagert) | Ganzzahlig | Nein | ARTICLE_PRICEUNIT |
| 22044 | Gruppennummer | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_GROUPNUMBER |
| 22045 | Bilddatei | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_PICTUREFILE |
| 22046 | Gewicht | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_WEIGHT |
| 22047 | Platzbedarf | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_MOUNTINGSPACE |
| 22048 | Zertifizierung: Allgemein | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CERTIFICATE |
| 22049 | Zertifizierung: UL-Kennung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CERTIFICATE_UL |
| 22050 | Zertifizierung: VDE-Kennung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CERTIFICATE_VDE |
| 22051 | Attribute | Artikel (eingelagert) Bohrbilder Zubehörlisten Zubehörplatzierungen Anschlussbilder | Einsprachiger Text | Ja, 100 | ARTICLE_ATTRIBUTE_VALUE |
| 22052 | Spannungsversorgung | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_PLCISPOWERSUPPLY |
| 22053 | Bus-Verteiler | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_PLCISBUSDISTRIBUTOR |
| 22054 | Artikel ist Zubehör | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_IS_ACCESSORY |
| 22055 | Länge (vorkonfektioniert) | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_CABLELENGTH |
| 22056 | ERP-Nummer | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_ERPNR |
| 22058 | Lieferlänge | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DELIVERYLENGTH |
| 22059 | Gewicht in der Anzeigeeinheit | Artikel (eingelagert) Artikelreferenzen | Einsprachiger Text | Nein | ARTICLE_WEIGHT_DISPLAY_UNIT |
| 22060 | Eindeutige Artikel-ID | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_UNIQUEID |
| 22063 | Min. Biegeradius | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_BENDINGRADIUS |
| 22064 | Kabel / Paket: Bezeichnung in der Grafik | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | ARTICLE_CABLEDESIGNATION |
| 22065 | Außendurchmesser | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_OUTERDIAMETER |
| 22066 | Kupferzahl | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_COPPERNUMBER |
| 22067 | Gewicht/Länge | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CABLEWEIGHT |
| 22068 | Einheit für Verbindungsquerschnitt / -durchmesser | Artikel (eingelagert) | Ganzzahlig | Nein | ARTICLE_WIRECROSSSECTION_UNIT |
| 22069 | Verbindungsanzahl und Querschnitt / Durchmesser | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_WIRECROSSSECTION_AND_DIAMETER |
| 22070 | Spannungsart | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_VOLTAGETYPE |
| 22071 | Strom | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_ELECTRICALCURRENT |
| 22072 | Schaltleistung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_ELECTRICALPOWER |
| 22073 | Halteleistung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_HOLDINGPOWER |
| 22074 | Max. Verlustleistung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_POWERDISSIPATION |
| 22075 | Auslösestrom | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_TRIGGERCURRENT |
| 22076 | EMI-Modell | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_3DMACRO |
| 22077 | Untergewerk 'Elektrotechnik' | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_SUBCRAFT_ELECTRICAL |
| 22078 | Montageplatte: Montageplatz | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PANELMOUNTINGSPACE |
| 22079 | Tür: Montageplatz | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DOORMOUNTINGSPACE |
| 22080 | Farbe | Artikel (eingelagert) | Mehrsprachiger Text | Nein | ARTICLE_COLOR |
| 22081 | Werkstoff / Material | Artikel (eingelagert) Verbindungen Verbindungsdefinitionspunkte Rohrleitungsdefinitionspunkte | Mehrsprachiger Text | Nein | ARTICLE_MATERIAL |
| 22082 | Klemmen: Schutzart | Artikel (eingelagert) | Mehrsprachiger Text | Nein | ARTICLE_DEGOFPROTECTION |
| 22084 | Klemmen: Querschnitt von | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CROSSSECTIONFROM |
| 22085 | Klemmen: Querschnitt bis | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CROSSSECTIONTILL |
| 22086 | Klemmen: AWG von | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_AWGFROM |
| 22087 | Klemmen: AWG bis | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_AWGTILL |
| 22088 | Klemmen: Strom IEC | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CURRENTIEC |
| 22089 | Klemmen: Spannung IEC | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_VOLTAGEIEC |
| 22090 | Klemmen: Strom UL | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CURRENTUL |
| 22091 | Klemmen: Spannung UL | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_VOLTAGEUL |
| 22092 | Klemmen: Strom CSA | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CURRENTCSA |
| 22093 | Klemmen: Spannung CSA | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_VOLTAGECSA |
| 22094 | Untergewerk 'Mechanik' | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_SUBCRAFT_MECHANICS |
| 22095 | Stecker: Anordnung der Steckerkontakte | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CONTACTOR_ARRANGEMENT |
| 22096 | Stecker: Luftstrecke | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_AIRGAP |
| 22097 | Stecker: Kriechstrecke | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CREEPAGEDISTANCE |
| 22098 | Stecker: Standard / invers | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_STANDARDINVERS |
| 22099 | Stecker: Steckerkontaktart | Artikel (eingelagert) | Mehrsprachiger Text | Nein | ARTICLE_CONTACTTYPE |
| 22100 | Stecker: Bauform | Artikel (eingelagert) | Mehrsprachiger Text | Nein | ARTICLE_DESIGN |
| 22101 | Stecker: Anschlusstechnik | Artikel (eingelagert) | Mehrsprachiger Text | Nein | ARTICLE_CONNECTIONMETHOD |
| 22102 | Stecker: Voreilende Steckerkontakte | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_ADVANCECONTACTS |
| 22103 | Stecker: Codierung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CODING |
| 22104 | Version | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_FIRMWAREVERSION |
| 22105 | SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCTYPE |
| 22106 | Adressbereich (SIEMENS STEP 7 Classic) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_ADDRESSRANGE |
| 22107 | Verkaufspreis Währung 1 | Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SALESPRICE_1 |
| 22108 | Verkaufspreis Währung 2 | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SALESPRICE_2 |
| 22109 | Einkaufspreis/Preiseinheit Währung 1 | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PURCHASEPRICE_1 |
| 22110 | Einkaufspreis/Preiseinheit Währung 2 | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PURCHASEPRICE_2 |
| 22111 | Einkaufspreis/Verpackung Währung 1 | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PACKAGINGPRICE_1 |
| 22112 | Einkaufspreis/Verpackung Währung 2 | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PACKAGINGPRICE_2 |
| 22113 | Zertifizierung: CE-Kennung | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CERTIFICATE_CE |
| 22114 | Eigensicher | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_INTRINSICSAFETY |
| 22115 | Kurzschlussfest | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_SHORTCIRCUITRESISTANT |
| 22116 | Montageplatte: Bebaubare Höhe | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PANELHEIGHT |
| 22117 | Montageplatte: Bebaubare Breite | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PANELWIDTH |
| 22118 | Montageplatte: Max. Einbautiefe | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PANELDEPTH |
| 22119 | Tür: Bebaubare Höhe | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DOORHEIGHT |
| 22120 | Tür: Bebaubare Breite | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DOORWIDTH |
| 22121 | Tür: Max. Einbautiefe | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DOORDEPTH |
| 22122 | Menge/Verpackung | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PACKAGINGQUANTITY |
| 22123 | Anzahl der PE-Adern | Artikel (eingelagert) | Ganzzahlig | Nein | ARTICLE_PE_WIRE_COUNT |
| 22124 | Max. Betriebsdruck | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PRESSURE |
| 22125 | Regelbereich | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_ADJUSTRANGE |
| 22126 | Durchfluss | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_FLOW |
| 22127 | Gewinde | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_THREAD |
| 22128 | Innendurchmesser | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_INNERDIAMETER |
| 22129 | Hublänge | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_STROKELENGTH |
| 22130 | Anschluss | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CONNECTION |
| 22131 | Gewerk 'Elektrotechnik' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_ELECTRICAL |
| 22132 | Gewerk 'Fluid' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_FLUID |
| 22133 | Gewerk 'Mechanik' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_MECHANICS |
| 22134 | Gewerk 'Hydraulik' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_HYDRAULICS |
| 22135 | Gewerk 'Pneumatik' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_PNEUMATICS |
| 22136 | Gewerk 'Schmierung' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_LUBRICATION |
| 22137 | Gewerk 'Kühlung' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_COOLING |
| 22138 | Produktobergruppe | Artikel (eingelagert) Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen | Ganzzahlig | Nein | ARTICLE_PRODUCTTOPGROUP |
| 22139 | Verschleißteil | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_WEAR |
| 22140 | Ersatzteil | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_SPARE |
| 22141 | Schmierung / Wartung | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_MAINTENANCE |
| 22142 | Lebensdauer | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_LIFETIME |
| 22143 | Beanspruchung | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_STRESS |
| 22144 | Beschaffung | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_USAGE |
| 22145 | Makro | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_GROUPSYMBOLMACRO |
| 22146 | Freie Eigenschaften: Anzeigename | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Ja, 1000 | ARTICLE_FREE_DATA_DESCRIPTION |
| 22147 | Freie Eigenschaften: Wert | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Ja, 1000 | ARTICLE_FREE_DATA_VALUE |
| 22148 | Freie Eigenschaften: Einheit | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Ja, 1000 | ARTICLE_FREE_DATA_UNIT |
| 22152 | Einbauabstand Breite links | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SPACING_LEFT |
| 22153 | Einbauabstand Breite rechts | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SPACING_RIGHT |
| 22154 | Einbauabstand Höhe oberhalb | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SPACING_ABOVE |
| 22155 | Einbauabstand Höhe unterhalb | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SPACING_BELOW |
| 22156 | Einbauabstand Tiefe vorne | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SPACING_FRONT |
| 22157 | Einbauabstand Tiefe hinten | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SPACING_REAR |
| 22158 | Untergewerk 'Hydraulik' | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_SUBCRAFT_HYDRAULICS |
| 22159 | Untergewerk 'Pneumatik' | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_SUBCRAFT_PNEUMATICS |
| 22160 | Türfalz | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DOOR_RABBET |
| 22161 | Türöffnung: Versatz oben | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DOOR_OFFSET_TOP |
| 22162 | Türöffnung: Versatz rechts | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DOOR_OFFSET_RIGHT |
| 22163 | Überstand Seitenwand vorne | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SIDEPANELPROJECTIONFRONT |
| 22164 | Überstand Seitenwand hinten | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SIDEPANELPROJECTIONBACK |
| 22165 | Überstand Seitenwand oben | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SIDEPANELPROJECTIONTOP |
| 22166 | Überstand Seitenwand unten | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SIDEPANELPROJECTIONBOTTOM |
| 22167 | Abstand Seitenwand | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SIDEPANELDISTANCE |
| 22168 | Tiefe Seitenwand | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SIDEPANELDEPTH |
| 22169 | Überstand Rückwand links | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_REARPANELPROJECTIONLEFT |
| 22170 | Überstand Rückwand rechts | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_REARPANELPROJECTIONRIGHT |
| 22171 | Überstand Rückwand oben | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_REARPANELPROJECTIONTOP |
| 22172 | Überstand Rückwand unten | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_REARPANELPROJECTIONBOTTOM |
| 22173 | Abstand Rückwand | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_REARPANELDISTANCE |
| 22174 | Tiefe Rückwand | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_REARPANELDPEPTH |
| 22175 | Überstand Dach links | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_TOPPANELPROJECTIONLEFT |
| 22176 | Überstand Dach rechts | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_TOPPANELPROJECTIONRIGHT |
| 22177 | Überstand Dach vorne | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_TOPPANELPROJECTIONFRONT |
| 22178 | Überstand Dach hinten | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_TOPPANELPROJECTIONBACK |
| 22179 | Abstand Dach | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_TOPPANELDISTANCE |
| 22180 | Tiefe Dach | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_TOPPANELDPEPTH |
| 22181 | Überstand Boden links | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_BOTTOMPANELPROJECTIONLEFT |
| 22182 | Überstand Boden rechts | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_BOTTOMPANELPROJECTIONRIGHT |
| 22183 | Überstand Boden vorne | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_BOTTOMPANELPROJECTIONFRONT |
| 22184 | Überstand Boden hinten | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_BOTTOMPANELPROJECTIONBACK |
| 22185 | Abstand Boden | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_BOTTOMPANELDISTANCE |
| 22186 | Tiefe Boden | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_BOTTOMPANELDEPTH |
| 22187 | Höhe Profil quer | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PROFILEHEIGHT |
| 22188 | Tiefe Profil quer | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PROFILEDEPTH |
| 22189 | Breite Profil vertikal | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_VPROFILEWIDTH |
| 22190 | Tiefe Profil vertikal | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_VPROFILEDPETH |
| 22191 | Abstand bei Anreihung | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_PROFILEDISTANCE |
| 22192 | Tür: Typ | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_DOORTYPE |
| 22193 | Tür: Scharnier | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_HINGEPOSITION |
| 22194 | Tür: Wandstärke | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DOORTHICKNESS |
| 22195 | Untergewerk 'Schmierung' | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_SUBCRAFT_LUBRICATION |
| 22196 | Untergewerk 'Kühlung' | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_SUBCRAFT_COOLING |
| 22197 | Untergewerk 'Verfahrenstechnik' | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_SUBCRAFT_PROCESS |
| 22198 | Breite oben | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_WIDTHTOP |
| 22199 | Breite unten | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_WIDTHBOTTOM |
| 22200 | Sammelschienen: Profilgeometrie T x H (nur EPLAN Cabinet) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_BARGEOMETRY |
| 22201 | Sammelschienen: Anzahl der Schienen | Artikel (eingelagert) | Ganzzahlig | Nein | ARTICLE_BARCOUNT |
| 22202 | Sammelschienen: Abstand der Schienen | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_BARDISTANCE |
| 22203 | Sammelschienen: Abstand der Schienen zur Montageplatte | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_BARMOUNTINGPLATEDISTANCE |
| 22204 | Sammelschienenhalter: Artikelnummer | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_BUSBARHOLDERPARTNR |
| 22205 | Sammelschienenhalter: Artikelvariante | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_BUSBARHOLDERVARIANT |
| 22207 | Sammelschienenhalter: Vertikaler Versatz | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_INSERTPOINTOFFSETX |
| 22208 | Strichcode-Nummer | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_IDENTCODE |
| 22209 | Strichcode-Typ | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_IDENTTYPE |
| 22210 | Externes Dokument | Artikel (eingelagert) | Einsprachiger Text | Ja, 20 | ARTICLE_EXTERNAL_DOCUMENT |
| 22211 | Aufklipshöhe | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SNAPHEIGHT |
| 22214 | Kennung für Auswertungen | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLE_REPORT_IDENTIFIER |
| 22215 | Mittenversatz | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_MIDDLEOFFSET |
| 22216 | Wandstärke | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_WALLTHICKNESS |
| 22217 | Bohrbild | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_REF_CONSTRUCTION_NAME |
| 22218 | Spule: Spannung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_COILVOLTAGE |
| 22219 | Textur | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_ECABINET_MACRO |
| 22220 | Externe Platzierung | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | ARTICLE_EXTERNAL_PLACEMENT |
| 22221 | Rabatt | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DISCOUNT |
| 22222 | Herstellername | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_MANUFACTURER_NAME |
| 22223 | Lieferantenname | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_SUPPLIER_NAME |
| 22224 | Rohrklasse | Artikel (eingelagert) Verbindungen Verbindungsdefinitionspunkte Rohrleitungsdefinitionspunkte | Einsprachiger Text | Nein | ARTICLE_PIPECLASS |
| 22225 | Nennweite | Artikel (eingelagert) Verbindungen Verbindungsdefinitionspunkte Rohrleitungsdefinitionspunkte | Einsprachiger Text | Nein | ARTICLE_WIDTHRATING |
| 22226 | Nenndruckstufe | Artikel (eingelagert) Verbindungen Verbindungsdefinitionspunkte Rohrleitungsdefinitionspunkte | Einsprachiger Text | Nein | ARTICLE_PRESSURELEVEL |
| 22227 | Norm | Artikel (eingelagert) Verbindungen Verbindungsdefinitionspunkte Rohrleitungsdefinitionspunkte | Einsprachiger Text | Nein | ARTICLE_NORM |
| 22228 | Symbol für Auswertungen | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Ja, 20 | ARTICLE_REPORT_SYMBOL |
| 22229 | Aufreihbar | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CAN_BE_LINED_UP |
| 22230 | Max. Betriebsdruck (vollständig) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PRESSURE_FULL |
| 22231 | Regelbereich (vollständig) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_ADJUSTRANGE_FULL |
| 22232 | Durchfluss (vollständig) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_FLOW_FULL |
| 22233 | Gewicht (vollständig) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_WEIGHT_FULL |
| 22234 | Freie Eigenschaften: Wert und Einheit (vollständig) | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Ja, 1000 | ARTICLE_FREE_DATA_FULL |
| 22252 | Sammelschienen: Artikelnummer | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_BUSBARRAILPARTNR |
| 22253 | Sammelschienen: Artikelvariante | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_BUSBARRAILVARIANT |
| 22254 | Verbindungstyp | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_WIRETYPE |
| 22255 | Verbindung: Einheit für Verbindungsquerschnitt / -durchmesser | Artikel (eingelagert) | Ganzzahlig | Nein | ARTICLE_CONNECTION_WIRECROSSSECTION_UNIT |
| 22257 | Biegeradius | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_BENDINGRADIUS_COPPER |
| 22258 | Auslaufartikel | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | ARTICLE_DISCONTINUED |
| 22259 | Bündeldurchmesser maximal | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_BUNDLE_MAXDIAMETER |
| 22260 | Bündeldurchmesser minimal | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_BUNDLE_MINDIAMETER |
| 22261 | Adressbereich 2 (SIEMENS STEP 7 Classic) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_ADDRESSRANGE_2 |
| 22262 | Gewerk 'Kühlschmiermittel' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_COOLINGLUBRICANT |
| 22263 | Gewerk 'Gastechnik' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_GASTECHNOLOGY |
| 22264 | Gewerk 'Fluid (undefiniert)' | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_CRAFT_FLUID_UNDEFINED |
| 22265 | Untergewerk 'Kühlschmiermittel' | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_SUBCRAFT_COOLINGLUBRICANT |
| 22266 | Untergewerk 'Gastechnik' | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_SUBCRAFT_GASTECHNOLOGY |
| 22267 | Untergewerk 'Fluid (undefiniert)' | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_SUBCRAFT_FLUID_UNDEFINED |
| 22268 | Einbautiefe | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_INSTALLATION_DEPTH |
| 22269 | SPS-Stationstyp | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCSTATIONTYPE |
| 22270 | Zertifizierung: ATEX-Kennung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CERTIFICATE_ATEX |
| 22271 | Schienenquerschnitt | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_RAILCROSSSECTION |
| 22272 | Schienenwerkstoff | Artikel (eingelagert) | Ganzzahlig | Nein | ARTICLE_RAILMATERIAL |
| 22273 | Leitfähigkeit (bei +20°C) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CONDUCTIVITY |
| 22274 | Temperaturkoeffizient | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_TEMPERATURE_COEFFICIENT |
| 22275 | Fluid- / Verfahrenstechnik: Wandstärke der Verbindung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_FLUID_WALLTHICKNESS |
| 22277 | Anschlussbild: Versatz X-Richtung | Artikel (eingelagert) Artikel (referenziert) | Dezimalzahl | Nein | ARTICLE_REF_TERMINAL_OFFSET_X |
| 22278 | Anschlussbild: Versatz Y-Richtung | Artikel (eingelagert) Artikel (referenziert) | Dezimalzahl | Nein | ARTICLE_REF_TERMINAL_OFFSET_Y |
| 22279 | Externes Dokument: Bezeichnung | Artikel (eingelagert) | Mehrsprachiger Text | Ja, 20 | ARTICLE_EXTERNAL_DOCUMENT_DESIGNATION |
| 22280 | Externes Dokument: Datei / Hyperlink | Artikel (eingelagert) | Einsprachiger Text | Ja, 20 | ARTICLE_EXTERNAL_DOCUMENT_URL |
| 22281 | Anschlussmaß Quelle | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_TERMINALSIZE_SOURCE |
| 22282 | Anschlussmaß Ziel | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_TERMINALSIZE_DESTINATION |
| 22283 | Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICE_INDEX |
| 22285 | Stegbreite | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_TAB_WIDTH |
| 22286 | Schlitzbreite | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_SLOT_GAP |
| 22287 | Abstand der Drahtrückhaltenase | Artikel (eingelagert) | Dezimalzahl | Nein | ARTICLE_DISTANCE_WIRE_HOLD_BACK_NOSE |
| 22288 | Modul verteilt platzieren | Artikel (eingelagert) | Ganzzahlig | Nein | ARTICLE_MODULE_POS_PLACE_SPREADED |
| 22289 | Stücklistengruppe | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_PARTSLISTGROUP |
| 22290 | SPS-Karte steckt auf Kopfstation | Artikel (eingelagert) | Bool (Wahr / Falsch) | Nein | ARTICLE_PLCISMOUNTEDONHEADMODULE |
| 22292 | In Artikelsummenstückliste auflösen | Artikel (eingelagert) | Ganzzahlig | Nein | ARTICLE_DISASSEMBLE_MODE |
| 22293 | SPS-Untergerät 1: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_1 |
| 22294 | SPS-Untergerät 2: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_2 |
| 22295 | SPS-Untergerät 3: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_3 |
| 22296 | SPS-Untergerät 4: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_4 |
| 22297 | SPS-Untergerät 5: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_5 |
| 22298 | SPS-Untergerät 6: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_6 |
| 22299 | SPS-Untergerät 7: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_7 |
| 22300 | SPS-Untergerät 8: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_8 |
| 22301 | SPS-Untergerät 9: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_9 |
| 22302 | SPS-Untergerät 10: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_10 |
| 22303 | SPS-Untergerät 11: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_11 |
| 22304 | SPS-Untergerät 12: Name | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENAME_12 |
| 22305 | SPS-Untergerät 1: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_1 |
| 22306 | SPS-Untergerät 2: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_2 |
| 22307 | SPS-Untergerät 3: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_3 |
| 22308 | SPS-Untergerät 4: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_4 |
| 22309 | SPS-Untergerät 5: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_5 |
| 22310 | SPS-Untergerät 6: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_6 |
| 22311 | SPS-Untergerät 7: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_7 |
| 22312 | SPS-Untergerät 8: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_8 |
| 22313 | SPS-Untergerät 9: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_9 |
| 22314 | SPS-Untergerät 10: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_10 |
| 22315 | SPS-Untergerät 11: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_11 |
| 22316 | SPS-Untergerät 12: An Position / Steckplatz | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCDEVICENUMBER_12 |
| 22338 | TemplateReference | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCTEMPLATEREFERENCE |
| 22340 | Antrieb: Gerätetyp | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCAXIS_DEVICETYPE |
| 22900 | Kurzname | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_SHORTNAME |
| 22902 | Ersteller / Erstellungsdatum | Artikel (referenziert) Artikel (eingelagert) | Einsprachiger Text | Nein | PART_CREATE |
| 22908 | Langname | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_LONGNAME |
| 22909 | Anrede | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_TITLE |
| 22910 | Name 1 | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_NAME1 |
| 22911 | Name 2 | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_NAME2 |
| 22912 | Name 3 | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_NAME3 |
| 22913 | Straße | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_STREET |
| 22914 | PLZ (Wohnort) | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_ZIPTOWN |
| 22915 | Wohnort | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_TOWN |
| 22916 | PLZ (Postfach) | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_ZIPPOBOX |
| 22917 | Postfach | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_POBOX |
| 22918 | Land | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_STATE |
| 22919 | Telefon | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_PHONE |
| 22920 | E-Mail | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_EMAIL |
| 22921 | Kundennummer | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_NUMBER |
| 22922 | Beschreibung (Adresse) | Artikel (referenziert) Adressangaben bei Artikeln | Mehrsprachiger Text | Nein | PART_ADDRESS_NOTE |
| 22923 | Fax | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_FAX |
| 22930 | Beschreibung (Bohrbild) | Artikel (referenziert) Bohrbilder | Mehrsprachiger Text | Nein | PART_CONSTRUCTION_DESCRIPTION |
| 22931 | Name | Artikel (referenziert) Bohrbilder | Einsprachiger Text | Nein | PART_CONSTRUCTION_NAME |
| 22932 | Bohrbildvariante | Artikel (referenziert) Bohrbilder | Einsprachiger Text | Nein | PART_CONSTRUCTION_VARIANT |
| 22936 | Ersteller / Erstellungsdatum (Zubehörliste) | Artikel (referenziert) Zubehörlisten | Einsprachiger Text | Nein | PART_ACCESSORYLIST_CREATE |
| 22939 | Ersteller / Erstellungsdatum (Bohrbild) | Artikel (referenziert) Bohrbilder | Einsprachiger Text | Nein | PART_CONSTRUCTION_CREATE |
| 22941 | Anschlussbild | Artikel (referenziert) Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_REF_TERMINAL_NAME |
| 22942 | Zusatzlänge (Standard) | Artikel (referenziert) Anschlussbilder | Dezimalzahl | Nein | PART_TERMINAL_ADDITIONALLENGTHDEFAULT |
| 22943 | Ersteller / Erstellungsdatum (Anschlussbild) | Artikel (referenziert) Anschlussbilder | Einsprachiger Text | Nein | PART_TERMINAL_CREATE |
| 22945 | Name (Anschlussbild) | Artikel (referenziert) Anschlussbilder | Einsprachiger Text | Nein | PART_TERMINAL_NAME |
| 22946 | Beschreibung (Anschlussbild) | Artikel (referenziert) Anschlussbilder | Mehrsprachiger Text | Nein | PART_TERMINAL_DESCRIPTION |
| 22947 | Verbindungsende-Behandlung (EPLAN Cabinet, Standard) | Artikel (referenziert) Anschlussbilder | Ganzzahlig | Nein | PART_TERMINAL_TYPEDEFAULT |
| 22948 | Verlegerichtung (Standard) | Artikel (referenziert) Anschlussbilder | Ganzzahlig | Nein | PART_TERMINAL_DIRECTION |
| 22959 | Beschreibung (Zubehörliste) | Artikel (referenziert) Zubehörlisten | Mehrsprachiger Text | Nein | PART_ACCESSORYLIST_DESCRIPTION |
| 22960 | Name (Zubehörliste) | Artikel (referenziert) Zubehörlisten | Einsprachiger Text | Nein | PART_ACCESSORYLIST_NAME |
| 22963 | Artikelnummer im Zubehör | Artikel (referenziert) | Einsprachiger Text | Nein | PART_USED_AS_ACCESSORY_FILTER |
| 22966 | Artikelnummer im Modul | Artikel (referenziert) | Einsprachiger Text | Nein | PART_USED_IN_MODULE_FILTER |
| 22967 | Artikelnummer in der Baugruppe | Artikel (referenziert) | Einsprachiger Text | Nein | PART_USED_IN_ASSEMBLY_FILTER |
| 22968 | Anschlussausführung (Standard) | Artikel (referenziert) Anschlussbilder | Ganzzahlig | Nein | PART_TERMINAL_TYPEOFTERMINAL_DEFAULT |
| 22969 | Anschlussmaß (Standard) | Artikel (referenziert) Anschlussbilder | Einsprachiger Text | Nein | PART_TERMINAL_TERMINALSIZE_DEFAULT |
| 22970 | Name (Zubehörplatzierung) | Artikel (referenziert) Zubehörplatzierungen | Einsprachiger Text | Nein | PART_ACCESSORYPLACEMENT_NAME |
| 22972 | Ersteller / Erstellungsdatum (Zubehörplatzierung) | Artikel (referenziert) Zubehörplatzierungen | Einsprachiger Text | Nein | PART_ACCESSORYPLACEMENT_CREATE |
| 22974 | Beschreibung (Zubehörplatzierung) | Artikel (referenziert) Zubehörplatzierungen | Mehrsprachiger Text | Nein | PART_ACCESSORYPLACEMENT_DESCRIPTION |
| 22982 | Ersteller | Artikel (referenziert) Artikel (eingelagert) Zubehörlisten Zubehörplatzierungen Bohrbilder Anschlussbilder | Einsprachiger Text | Nein | PART_CREATE_USER |
| 22983 | Erstellungsdatum | Artikel (referenziert) Artikel (eingelagert) | Uhrzeit / Datum | Nein | PART_CREATE_DATE |
| 22984 | Eindeutige externe Artikel-ID | Artikel (eingelagert) Artikelreferenzen Artikel (referenziert) | Einsprachiger Text | Nein | ARTICLE_CPMS_GUID |
| 23009 | Makro: Name | Makrokästen Makrokästen (referenziert) Geschützte Gruppen | Einsprachiger Text | Nein | MACROBOX_NAME |
| 23015 | Makro: Verzeichnis | Makrokästen Makrokästen (referenziert) | Einsprachiger Text | Nein | MACROBOX_DIRECTORY |
| 23100 | Projektoptionengruppe: Name | Projektoptionen Projektoptionenausschnitte Projektoptionengruppen | Einsprachiger Text | Nein | OPTIONGROUP_NAME |
| 23101 | Projektoption: Name | Projektoptionen Projektoptionenausschnitte | Einsprachiger Text | Nein | OPTION_NAME |
| 23102 | Projektoptionenausschnitt: Name | Projektoptionenausschnitte | Einsprachiger Text | Nein | OPTIONFRAGMENT_NAME |
| 23103 | Projektoption aktiv | Projektoptionen Projektoptionenausschnitte | Bool (Wahr / Falsch) | Nein | OPTION_ISACTIVE |
| 23104 | Projektoptionengruppe: Beschreibung | Projektoptionengruppen Projektoptionen Projektoptionenausschnitte | Mehrsprachiger Text | Nein | OPTIONGROUP_DESCR |
| 23105 | Projektoptionenausschnitt: Beschreibung | Projektoptionenausschnitte | Mehrsprachiger Text | Nein | OPTIONFRAGMENT_DESCR |
| 23106 | Projektoption: Beschreibung | Projektoptionen Projektoptionenausschnitte | Mehrsprachiger Text | Nein | OPTION_DESCR |
| 23200 | Artikelnummer | Gerätelisteneinträge | Einsprachiger Text | Nein | DEVICELISTENTRY_PARTNR |
| 23201 | Erlaubte Stückzahl | Gerätelisteneinträge | Ganzzahlig | Nein | DEVICELISTENTRY_COUNTALLOWED |
| 23202 | Gerätebeschreibung | Gerätelisteneinträge | Einsprachiger Text | Nein | DEVICELISTENTRY_DESCRIPTION |
| 23203 | Typnummer | Gerätelisteneinträge | Einsprachiger Text | Nein | DEVICELISTENTRY_TYPENR |
| 23204 | Erlaubte Stückzahl (gesamt) | Gerätelisteneinträge | Ganzzahlig | Nein | DEVICELISTENTRY_COUNTALLOWEDALL |
| 23205 | Verbaute Stückzahl | Gerätelisteneinträge | Ganzzahlig | Nein | DEVICELISTENTRY_COUNTUSED |
| 23206 | Reserve | Gerätelisteneinträge | Ganzzahlig | Nein | DEVICELISTENTRY_RESERVE |
| 23207 | Bestellnummer | Gerätelisteneinträge | Einsprachiger Text | Nein | DEVICELISTENTRY_ORDERNR |
| 23208 | Anlagenteil | Gerätelisteneinträge | Einsprachiger Text | Nein | DEVICELISTENTRY_PLANT |
| 23209 | Variante | Gerätelisteneinträge | Einsprachiger Text | Nein | DEVICELISTENTRY_VARIANT |
| 23210 | Artikel: Bezeichnung 1 | Gerätelisteneinträge | Mehrsprachiger Text | Nein | DEVICELISTENTRY_DESIGNATION1 |
| 23211 | Artikel: Bezeichnung 2 | Gerätelisteneinträge | Mehrsprachiger Text | Nein | DEVICELISTENTRY_DESIGNATION2 |
| 23212 | Artikel: Bezeichnung 3 | Gerätelisteneinträge | Mehrsprachiger Text | Nein | DEVICELISTENTRY_DESIGNATION3 |
| 23213 | Hersteller | Gerätelisteneinträge | Einsprachiger Text | Nein | DEVICELISTENTRY_MANUFACTURER |
| 23214 | Makro | Gerätelisteneinträge | Einsprachiger Text | Nein | DEVICELISTENTRY_MACRO |
| 23215 | Auslaufartikel | Gerätelisteneinträge | Bool (Wahr / Falsch) | Nein | DEVICELISTENTRY_DISCONTINUED |
| 23216 | ERP-Nummer | Gerätelisteneinträge | Einsprachiger Text | Nein | DEVICELISTENTRY_ERPNR |
| 23303 | SPS-Objekt-ID | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_PLCOBJECTID |
| 23500 | Bohrbild: Name | Bohrbildkästen | Einsprachiger Text | Nein | DRILLPATTERNBOX_DRILLPATTERN_NAME |
| 23502 | Bohrbild: Unterverzeichnis für Kontur (NC-Daten) | Bohrbildkästen | Einsprachiger Text | Nein | DRILLPATTERNBOX_CONTOURPATH |
| 24810 | Beschreibung der Abbruchstelle | Abbruchstellen | Mehrsprachiger Text | Nein | INTERRUPTIONPOINT_DESCRIPTION |
| 24811 | Beschreibung der Abbruchstelle (automatisch) | Abbruchstellen | Mehrsprachiger Text | Nein | INTERRUPTIONPOINT_DESCRIPTIONAUTOMATIC |
| 25020 | Zähler Seitenzahl | Seiten | Einsprachiger Text | Nein | PAGE_COUNTERPAGENUMBER |
| 25030 | AutomationML GUID | Verbindungen Funktionen Segmente (Vorplanung) Verbindungsdefinitionspunkte Strukturkennzeichen SPS-Adressen SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | AUTOMATIONML_OBJECTID |
| 25031 | AutomationML GUID 2 | Funktionen SPS-Adressen SPS-Adressen (in Auswertungen) | Einsprachiger Text | Ja, 12 | AUTOMATIONML_OBJECTID_2 |
| 25102 | Querverweis (extern) | Funktionen Abbruchstellen Strukturkästen | Einsprachiger Text | Nein | FUNCTION_EXTERNALCROSSREFERENCE |
| 31002 | Verbindungsquerschnitt / -durchmesser | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_WIRECROSSSECTION |
| 31003 | Verbindung: Länge mit Einheit | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_WIRELENGTH |
| 31004 | Verbindungsfarbe / -nummer | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_WIRENUMBER |
| 31006 | Potenzialtyp | Verbindungen Potenzialdefinitionspunkte | Ganzzahlig | Nein | POTENTIAL_TYPE |
| 31007 | Verbindungsquerschnitt / -durchmesser mit Einheit | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_WIRECROSSSECTION_WITH_UNIT |
| 31009 | Verbindungsbeschreibung | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Mehrsprachiger Text | Nein | CONNECTION_DESCRIPTION |
| 31011 | Verbindungsbezeichnung | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte Symbole | Einsprachiger Text | Nein | CONNECTION_DESIGNATION |
| 31013 | Quelle und Ziel vertauschen | Verbindungen Verbindungsdefinitionspunkte | Bool (Wahr / Falsch) | Nein | CONNECTION_SWAP_SRC_DEST |
| 31019 | Quelle | Verbindungen | Einsprachiger Text | Nein | CONNECTION_SOURCE |
| 31020 | Ziel | Verbindungen | Einsprachiger Text | Nein | CONNECTION_DESTINATION |
| 31021 | Platzierung Quelle | Verbindungen | Einsprachiger Text | Nein | CONNECTION_LOCATION_SOURCE |
| 31022 | Platzierung Ziel | Verbindungen | Einsprachiger Text | Nein | CONNECTION_LOCATION_DESTINATION |
| 31023 | Zielnummer Quelle | Verbindungen | Ganzzahlig | Nein | CONNECTION_NR_SOURCE |
| 31024 | Zielnummer Ziel | Verbindungen | Ganzzahlig | Nein | CONNECTION_NR_DESTINATION |
| 31025 | Alternativer Verbindungsquerschnitt / -durchmesser | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_ALTERNATE_WIRECROSSSECTION |
| 31027 | Zugehörige Verbindungen: Betriebsmittelkennzeichen (identifizierend) | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_DT |
| 31028 | Zugehörige Verbindungen: Verbindungsbezeichnung | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_DESIGNATION |
| 31029 | Zugehörige Verbindungen: Verbindungsquerschnitt / -durchmesser | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_WIRECROSSSECTION |
| 31031 | Zugehörige Verbindungen: Verbindungsfarbe / -nummer | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_WIRENUMBER |
| 31032 | Zugehörige Verbindungen: Verbindungsbeschreibung | Verbindungsdefinitionspunkte | Mehrsprachiger Text | Nein | CDP_CON_DESCRIPTION |
| 31033 | Zugehörige Verbindungen: Länge | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_WIRELENGTH |
| 31034 | Zugehörige Verbindungen: Bemerkung | Verbindungsdefinitionspunkte | Mehrsprachiger Text | Nein | CDP_CON_NOTE |
| 31035 | Zugehörige Verbindungen: Funktionsdefinition | Verbindungsdefinitionspunkte | Mehrsprachiger Text | Nein | CDP_CON_TYPE |
| 31036 | Zugehörige Verbindungen: Quelle und Ziel vertauschen | Verbindungsdefinitionspunkte | Bool (Wahr / Falsch) | Nein | CDP_CON_SWAP_SRC_DEST |
| 31048 | Verbindung: Typbezeichnung | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_WIRETYPE |
| 31049 | Abgeschirmt von | Verbindungen | Einsprachiger Text | Nein | CONNECTION_CABLEWIRE_SHIELDEDBY |
| 31050 | Paarindex | Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_CABLEWIRE_PAIRINDEX |
| 31051 | Verbindungsende-Behandlung Quelle | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_TYPEOFENDSTOP_SOURCE |
| 31052 | Verbindungsende-Behandlung Ziel | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_TYPEOFENDSTOP_DESTINATION |
| 31053 | Hülsenquerschnitt Quelle | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_SLEEVECROSSSECTION_SOURCE |
| 31054 | Hülsenquerschnitt Ziel | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_SLEEVECROSSSECTION_DESTINATION |
| 31055 | Absetzlänge Quelle | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_STRIPPINGLENGTH_SOURCE |
| 31056 | Absetzlänge Ziel | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_STRIPPINGLENGTH_DESIGNATION |
| 31059 | Einheit für Verbindungsquerschnitt / -durchmesser | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Ganzzahlig | Nein | CONNECTION_WIRECROSSSECTION_UNIT |
| 31060 | Einheit für Verbindungsquerschnitt /-durchmesser (automatisch) | Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_AUTO_WIRECROSSSECTION_UNIT |
| 31061 | Verbindungsfarbe (mehrsprachig) | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Mehrsprachiger Text | Nein | CONNECTION_MLWIRENUMBER |
| 31062 | Zugehörige Verbindungen: Potenzialtyp | Verbindungsdefinitionspunkte | Ganzzahlig | Nein | CDP_CON_POTENTIAL_TYPE |
| 31063 | Zugehörige Verbindungen: Einheit des Verbindungsquerschnitts / -durchmessers (automatisch) | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_AUTO_WIRECROSSSECTION_UNIT |
| 31064 | Alternativer Verbindungsquerschnitt / -durchmesser mit Einheit | Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_AUTO_ALTERNATE_WIRECROSSSECTION_WITH_UNIT |
| 31065 | Einheit für alternativen Verbindungsquerschnitt / -durchmesser | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Ganzzahlig | Nein | CONNECTION_ALTERNATE_WIRECROSSSECTION_UNIT |
| 31066 | Zugehörige Verbindungen: Gewerk | Verbindungsdefinitionspunkte | Ganzzahlig | Nein | CDP_CON_CRAFT |
| 31067 | Zugehörige Verbindungen: Untergewerk | Verbindungsdefinitionspunkte | Mehrsprachiger Text | Nein | CDP_CON_SUBCRAFT |
| 31068 | Stoff | Verbindungen Verbindungsdefinitionspunkte Rohrleitungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_MEDIUM |
| 31069 | Gruppierung | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_GROUPING |
| 31080 | Verbindung: Anschlusslänge Quelle | Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_SOURCE_CP_LENGTH |
| 31081 | Zustand (Stoff) | Verbindungen Rohrleitungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_STATE |
| 31082 | Druck | Verbindungen Rohrleitungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_PRESSURE |
| 31083 | Verbindung: Anschlusslänge Ziel | Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_DEST_CP_LENGTH |
| 31084 | Temperatur | Verbindungen Rohrleitungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_TEMPERATURE |
| 31085 | Durchfluss | Verbindungen Rohrleitungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_FLOW |
| 31089 | Verbindungsdefinitionspunkt vorhanden | Verbindungen | Bool (Wahr / Falsch) | Nein | CONNECTION_HAS_CDP |
| 31090 | Verbindung: Länge (vollständig) | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_WIRELENGTH_FULL |
| 31092 | Bündel | Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_WIRE_BUNDLE |
| 31093 | Bündelgruppe | Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_BUNDLE_GROUP |
| 31094 | Bauraum: Verlegeweg-Vorgabe | Verbindungen | Einsprachiger Text | Nein | CONNECTION_WIRING_PATH_PRESETTING |
| 31095 | Bauraum: Verlegeweg | Verbindungen Funktionen | Einsprachiger Text | Nein | CONNECTION_WIRING_PATH |
| 31096 | Anschlussmaß Quelle | Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_TERMINALSIZE_SOURCE |
| 31097 | Anschlussmaß Ziel | Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_TERMINALSIZE_DESTINATION |
| 31098 | Doppelhülse vorgeschrieben bei 2 Zielen an der Quelle | Verbindungen Verbindungsdefinitionspunkte | Bool (Wahr / Falsch) | Nein | CONNECTION_DOUBLE_SLEEVES_SOURCE |
| 31099 | Doppelhülse vorgeschrieben bei 2 Zielen am Ziel | Verbindungen Verbindungsdefinitionspunkte | Bool (Wahr / Falsch) | Nein | CONNECTION_DOUBLE_SLEEVES_DESTINATION |
| 31100 | Bauraum: Vorgabe für Streckennetz-Eintritt (Quelle) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_WIRING_PATH_PRESETTING_AT_SOURCE |
| 31101 | Anlage (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_FULLPLANT_AUTOMATIC |
| 31102 | Anlage: Hauptkennzeichen (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_PLANT_AUTOMATIC |
| 31103 | Einbauort (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_FULLLOCATION_AUTOMATIC |
| 31104 | Einbauort: Hauptkennzeichen (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_LOCATION_AUTOMATIC |
| 31105 | Funktionale Zuordnung (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_FULLFUNCTIONALASSIGNMENT_AUTOMATIC |
| 31106 | Funktionale Zuordnung: Hauptkennzeichen (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_FUNCTIONALASSIGNMENT_AUTOMATIC |
| 31107 | Aufstellungsort (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_FULLPLACEOFINSTALLATION_AUTOMATIC |
| 31108 | Aufstellungsort: Hauptkennzeichen (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_PLACEOFINSTALLATION_AUTOMATIC |
| 31109 | Benutzerdefinierte Struktur (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_FULLUSERDEFINED_AUTOMATIC |
| 31110 | Benutzerdefinierte Struktur: Hauptkennzeichen (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_USERDEFINED_AUTOMATIC |
| 31111 | Anlagennummer (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_FULLINSTALLATIONNUMBER_AUTOMATIC |
| 31112 | Anlagennummer: Hauptkennzeichen (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_INSTALLATIONNUMBER_AUTOMATIC |
| 31113 | Geschützte Verlegung | Verbindungen Verbindungsdefinitionspunkte Funktionen | Bool (Wahr / Falsch) | Nein | CONNECTION_PROTECTED_ROUTING |
| 31114 | Verlegerichtung Quelle | Verbindungen Verbindungsdefinitionspunkte | Ganzzahlig | Nein | CONNECTION_ROUTING_DIRECTION_SOURCE |
| 31115 | Verlegerichtung Ziel | Verbindungen Verbindungsdefinitionspunkte | Ganzzahlig | Nein | CONNECTION_ROUTING_DIRECTION_DESTINATION |
| 31116 | Bauraum: Vorgabe für Streckennetz-Eintritt (Ziel) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_WIRING_PATH_PRESETTING_AT_DESTINATION |
| 31119 | Topologie: Verlegeweg-Vorgabe | Verbindungen | Einsprachiger Text | Nein | CONNECTION_CABLING_PATH_PRESETTING |
| 31133 | Produkt: Hauptkennzeichen (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_PRODUCT_AUTOMATIC |
| 31134 | Produkt (automatisch) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_FULLPRODUCT_AUTOMATIC |
| 31136 | Durchlaufene Leitungsverteiler | Verbindungen | Einsprachiger Text | Nein | CONNECTION_DISTRIBUTORS |
| 31137 | Topologie: Durchlaufene Leitungsverteiler (sortiert) | Verbindungen | Einsprachiger Text | Ja, 20 | CONNECTION_CABLING_DISTRIBUTOR |
| 31138 | Topologie: Abstand der Leitungsverteiler zur Quelle | Verbindungen | Einsprachiger Text | Ja, 20 | CONNECTION_CABLING_DISTRIBUTOR_DISTANCE |
| 31139 | Leitungsverteiler vorhanden | Verbindungen | Bool (Wahr / Falsch) | Nein | CONNECTION_HAS_DISTRIBUTOR |
| 31140 | Intern- / Extern-Index: Quelle | Verbindungen | Ganzzahlig | Nein | CONNECTION_TARGETINFO_SOURCE |
| 31141 | Intern- / Extern-Index: Ziel | Verbindungen | Ganzzahlig | Nein | CONNECTION_TARGETINFO_DESTINATION |
| 31143 | Kabelbaumname | Verbindungen Funktionen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | HARNESS_NAME |
| 31148 | Verlegeweg-Quelle: X-Koordinate | Verbindungen | Koordinaten (nur X oder Y) | Nein | CONNECTION_ROUTE_SOURCE_XPOS |
| 31149 | Verlegeweg-Quelle: Y-Koordinate | Verbindungen | Koordinaten (nur X oder Y) | Nein | CONNECTION_ROUTE_SOURCE_YPOS |
| 31150 | Verlegeweg-Quelle: Z-Koordinate | Verbindungen | Koordinaten (nur X oder Y) | Nein | CONNECTION_ROUTE_SOURCE_ZPOS |
| 31151 | Verlegeweg-Ziel: X-Koordinate | Verbindungen | Koordinaten (nur X oder Y) | Nein | CONNECTION_ROUTE_DESTINATION_XPOS |
| 31152 | Verlegeweg-Ziel: Y-Koordinate | Verbindungen | Koordinaten (nur X oder Y) | Nein | CONNECTION_ROUTE_DESTINATION_YPOS |
| 31153 | Verlegeweg-Ziel: Z-Koordinate | Verbindungen | Koordinaten (nur X oder Y) | Nein | CONNECTION_ROUTE_DESTINATION_ZPOS |
| 31157 | Rohrleitungsname | Verbindungen Rohrleitungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_PIPENAME |
| 33000 | Potenzialname | Verbindungen Potenzialdefinitionspunkte | Einsprachiger Text | Nein | POTENTIAL_NAME |
| 33001 | Übergeordnetes Potenzial | Potenzialdefinitionspunkte Verbindungen | Einsprachiger Text | Nein | POTENTIAL_PARENT |
| 33003 | Potenzialwert | Potenzialdefinitionspunkte Verbindungen | Einsprachiger Text | Nein | POTENTIAL_VALUE |
| 33006 | Signalname | Potenzialdefinitionspunkte Verbindungen | Einsprachiger Text | Nein | POTENTIAL_SIGNAL |
| 33007 | Netzname | Verbindungen Potenzialdefinitionspunkte | Einsprachiger Text | Nein | POTENTIAL_NETNAME |
| 33100 | Konturdefinitionspunkt: Name | Konturdefinitionspunkte | Einsprachiger Text | Nein | CONTOURDEFINITIONPOINT_NAME |
| 33101 | Konturdefinitionspunkt: Typ | Konturdefinitionspunkte | Einsprachiger Text | Nein | CONTOURDEFINITIONPOINT_TYPE |
| 35000 | Klemmenleisten: Anzahl SH-Klemmen | Klemmenleisten (in Auswertungen) | Ganzzahlig | Nein | TERMINALSTRIP_COUNTOFSH |
| 35001 | Klemmenleisten: Gesamtzahl Objekte auf der Klemmenleiste | Klemmenleisten (in Auswertungen) | Ganzzahlig | Nein | TERMINAL_COUNTOFINTERNALOBJECTS |
| 35002 | Klemmenleisten: Anzahl allgemeine Klemmen | Klemmenleisten (in Auswertungen) | Ganzzahlig | Nein | TERMINALSTRIP_COUNTOFCOMMON |
| 35003 | Klemmenleisten: Anzahl N-Klemmen | Klemmenleisten (in Auswertungen) | Ganzzahlig | Nein | TERMINALSTRIP_COUNTOFN |
| 35004 | Klemmenleisten: Anzahl PE-Klemmen | Klemmenleisten (in Auswertungen) | Ganzzahlig | Nein | TERMINALSTRIP_COUNTOFPE |
| 35005 | Klemmenleisten: Anzahl Klemmen und Zubehör für Klemmenplan | Klemmenleisten (in Auswertungen) | Ganzzahlig | Nein | TERMINALSTRIP_ITEMCOUNT |
| 35006 | Klemmenleisten: Anzahl Klemmen | Klemmenleisten (in Auswertungen) | Ganzzahlig | Nein | TERMINALSTRIP_COUNTOFTERMINALS |
| 35007 | Klemmenleisten: Erste Klemme | Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | TERMINALSTRIP_FIRSTTERMINAL |
| 35008 | Klemmenleisten: Letzte Klemme | Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | TERMINALSTRIP_LASTTERMINAL |
| 35009 | Klemmenleisten: Seitenname des Klemmenplans | Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | TERMINALSTRIP_DIAGRAMPAGE |
| 35010 | Klemmenleisten: Seitenname des Klemmenanschlussplans | Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | TERMINALSTRIP_CONNECTIONDIAGRAMPAGE |
| 35011 | Klemmenleisten: Seitenname des Klemmenaufreihplans | Klemmenleisten (in Auswertungen) | Einsprachiger Text | Nein | TERMINALSTRIP_TERMINALLINEUPDIAGRAMPAGE |
| 35100 | Kabel: Anzahl der verwendeten Verbindungen | Kabel (in Auswertungen) | Ganzzahlig | Nein | CABLE_COUNTOFUSEDWIRES |
| 35101 | Kabel: Anzahl der freien Verbindungen | Kabel (in Auswertungen) | Ganzzahlig | Nein | CABLE_COUNTOFFREEWIRES |
| 35102 | Kabel: Anzahl der Abschirmungen | Kabel (in Auswertungen) | Ganzzahlig | Nein | CABLE_COUNTOFSHIELDS |
| 35103 | Kabel: Anzahl PE-Adern | Kabel (in Auswertungen) | Ganzzahlig | Nein | CABLE_COUNTOFPEWIRES |
| 35104 | Kabel: Anzahl N-Adern | Kabel (in Auswertungen) | Ganzzahlig | Nein | CABLE_COUNTOFNWIRES |
| 35105 | Kabel: Anzahl allgemeine Verbindungen | Kabel (in Auswertungen) | Ganzzahlig | Nein | CABLE_COUNTOFNORMALWIRES |
| 35106 | Kabel: Seitenname des Kabelplans | Kabel (in Auswertungen) | Einsprachiger Text | Nein | CABLE_DIAGRAMPAGE |
| 35107 | Kabel: Seitenname des Kabelanschlussplans | Kabel (in Auswertungen) | Einsprachiger Text | Nein | CABLE_CONNECTIONDIAGRAMPAGE |
| 35108 | Kabel: Anzahl aller Verbindungen | Kabel (in Auswertungen) | Ganzzahlig | Nein | CABLE_COUNTOFALLWIRES |
| 35109 | Anzahl der verwendeten Verbindungen (einpolige Kabel) | Funktionen | Ganzzahlig | Nein | CABLE_COUNTOFUSEDWIRES_MANUAL |
| 35110 | Kabel: Anzahl der Verbindungen zu diesem Ziel / dieser Quelle | Kabel (in Auswertungen) | Ganzzahlig | Nein | CABLE_COUNTOFWIRES_DESTINATION |
| 35111 | Kabel: Durchlaufene Leitungsverteiler | Kabel (in Auswertungen) | Einsprachiger Text | Nein | CABLE_DISTRIBUTORS |
| 35200 | Stecker: Anzahl SH-Steckerkontakte | Stecker (in Auswertungen) | Ganzzahlig | Nein | PLUGSTRIP_COUNTOFSH |
| 35201 | Stecker: Seitenname des Steckerplans | Stecker (in Auswertungen) | Einsprachiger Text | Nein | PLUGSTRIP_DIAGRAMPAGE |
| 35202 | Stecker: Seitenname des Steckeranschlussplans | Stecker (in Auswertungen) | Einsprachiger Text | Nein | PLUGSTRIP_CONNECTIONDIAGRAMPAGE |
| 35203 | Stecker: Erster Steckerkontakt | Stecker (in Auswertungen) | Einsprachiger Text | Nein | PLUGSTRIP_FIRSTCONTACT |
| 35204 | Stecker: Letzter Steckerkontakt | Stecker (in Auswertungen) | Einsprachiger Text | Nein | PLUGSTRIP_LASTCONTACT |
| 35205 | Stecker: Anzahl PE-Steckerkontakte | Stecker (in Auswertungen) | Ganzzahlig | Nein | PLUGSTRIP_COUNTOFPE |
| 35206 | Stecker: Anzahl N-Steckerkontakte | Stecker (in Auswertungen) | Ganzzahlig | Nein | PLUGSTRIP_COUNTOFN |
| 35207 | Stecker: Anzahl Steckerkontakte und Zubehör für Steckerplan | Stecker (in Auswertungen) | Ganzzahlig | Nein | PLUGSTRIP_ITEMCOUNT |
| 35208 | Stecker: Anzahl allgemeine Steckerkontakte | Stecker (in Auswertungen) | Ganzzahlig | Nein | PLUGSTRIP_COUNTOFCOMMON |
| 35209 | Stecker: Gesamtanzahl Steckerkontakte | Stecker (in Auswertungen) | Ganzzahlig | Nein | PLUGSTRIP_COUNTOFINTERNALOBJECTS |
| 35210 | Stecker: Anzahl der Stifte | Stecker (in Auswertungen) | Ganzzahlig | Nein | PLUGSTRIP_COUNTOFPINS |
| 35211 | Stecker: Anzahl der Buchsen | Stecker (in Auswertungen) | Ganzzahlig | Nein | PLUGSTRIP_COUNTOFSOCKETS |
| 35212 | Stecker: Anzahl Steckerkontakte (Stift und Buchse) | Stecker (in Auswertungen) | Ganzzahlig | Nein | PLUGSTRIP_COUNTOFJOINED |
| 35500 | Bohrtyp | Durchbrüche (in Bohrbildern) | Ganzzahlig | Nein | ARTICLEDRILLINGPOSITION_TYPE |
| 35501 | Subtyp | Durchbrüche (in Bohrbildern) | Ganzzahlig | Nein | ARTICLEDRILLINGPOSITION_SUBTYPE |
| 35502 | Konturname | Durchbrüche (in Bohrbildern) | Einsprachiger Text | Nein | ARTICLEDRILLINGPOSITION_OUTLINENAME |
| 35503 | X-Position | Durchbrüche (in Bohrbildern) | Dezimalzahl | Nein | ARTICLEDRILLINGPOSITION_XPOS |
| 35504 | Y-Position | Durchbrüche (in Bohrbildern) | Dezimalzahl | Nein | ARTICLEDRILLINGPOSITION_YPOS |
| 35505 | Winkel | Durchbrüche (in Bohrbildern) | Dezimalzahl | Nein | ARTICLEDRILLINGPOSITION_ANGLE |
| 35506 | 1. Dimension | Durchbrüche (in Bohrbildern) | Dezimalzahl | Nein | ARTICLEDRILLINGPOSITION_DIMENSION1 |
| 35507 | 2. Dimension | Durchbrüche (in Bohrbildern) | Dezimalzahl | Nein | ARTICLEDRILLINGPOSITION_DIMENSION2 |
| 35508 | 3. Dimension | Durchbrüche (in Bohrbildern) | Dezimalzahl | Nein | ARTICLEDRILLINGPOSITION_DIMENSION3 |
| 35509 | Wiederholungsabstand | Durchbrüche (in Bohrbildern) | Dezimalzahl | Nein | ARTICLEDRILLINGPOSITION_SPACINGREPEAT |
| 35510 | Endabstand | Durchbrüche (in Bohrbildern) | Dezimalzahl | Nein | ARTICLEDRILLINGPOSITION_SPACINGEND |
| 35511 | Jedes n-te Loch bohren | Durchbrüche (in Bohrbildern) | Ganzzahlig | Nein | ARTICLEDRILLINGPOSITION_DRILLNTHHOLE |
| 35512 | Artikelnummer | Zubehörlisteneinträge | Einsprachiger Text | Nein | ARTICLEACCESSORYLISTPOSITION_PARTNR |
| 35513 | Variante | Zubehörlisteneinträge | Einsprachiger Text | Nein | ARTICLEACCESSORYLISTPOSITION_VARIANT |
| 35514 | Immer fertigen | Durchbrüche (in Bohrbildern) | Bool (Wahr / Falsch) | Nein | ARTICLEDRILLINGPOSITION_FABRICATEALWAYS |
| 35550 | Anschlussbezeichnung | Anschlüsse (in Anschlussbildern) | Einsprachiger Text | Nein | ARTICLETERMINALPOSITION_NAME |
| 35551 | X-Position | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_XPOS |
| 35552 | Y-Position | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_YPOS |
| 35553 | Z-Position | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_ZPOS |
| 35554 | Verlegerichtung | Anschlüsse (in Anschlussbildern) | Ganzzahlig | Nein | ARTICLETERMINALPOSITION_DIRECTION |
| 35555 | Zusatzlänge | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_ADDITIONALLENGTH |
| 35556 | Verbindungsende-Behandlung (EPLAN Cabinet) | Anschlüsse (in Anschlussbildern) | Ganzzahlig | Nein | ARTICLETERMINALPOSITION_CONTACTTYPE |
| 35557 | Min. Querschnitt | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_MINCROSSSECTION |
| 35558 | Max. Querschnitt | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_MAXCROSSSECTION |
| 35559 | Max. Anzahl Verbindungen | Anschlüsse (in Anschlussbildern) | Ganzzahlig | Nein | ARTICLETERMINALPOSITION_MAXWIRECOUNT |
| 35560 | Doppelhülse vorgeschrieben | Anschlüsse (in Anschlussbildern) | Bool (Wahr / Falsch) | Nein | ARTICLETERMINALPOSITION_TWINSLEEVE |
| 35561 | Anschlussausführung | Anschlüsse (in Anschlussbildern) | Ganzzahlig | Nein | ARTICLETERMINALPOSITION_TYPEOFTERMINAL |
| 35562 | Anschlussmaß | Anschlüsse (in Anschlussbildern) | Einsprachiger Text | Nein | ARTICLETERMINALPOSITION_TERMINALSIZE |
| 35563 | Steckerbezeichnung | Anschlüsse (in Anschlussbildern) | Einsprachiger Text | Nein | ARTICLETERMINALPOSITION_DEVICETAG |
| 35564 | Etage | Anschlüsse (in Anschlussbildern) | Ganzzahlig | Nein | ARTICLETERMINALPOSITION_LEVEL |
| 35565 | Intern- / Extern-Index | Anschlüsse (in Anschlussbildern) | Ganzzahlig | Nein | ARTICLETERMINALPOSITION_TARGETINFO |
| 35567 | X-Vektor | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_XDIR |
| 35568 | Y-Vektor | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_YDIR |
| 35569 | Z-Vektor | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_ZDIR |
| 35571 | Min. AWG | Anschlüsse (in Anschlussbildern) | Einsprachiger Text | Nein | ARTICLETERMINALPOSITION_MINCROSSSECTION_AWG |
| 35572 | Max. AWG | Anschlüsse (in Anschlussbildern) | Einsprachiger Text | Nein | ARTICLETERMINALPOSITION_MAXCROSSSECTION_AWG |
| 35573 | Abtriebsgröße | Anschlüsse (in Anschlussbildern) | Einsprachiger Text | Nein | ARTICLETERMINALPOSITION_SCREWDRIVES |
| 35574 | Bus-Schnittstelle: Name | Anschlüsse (in Anschlussbildern) | Einsprachiger Text | Nein | ARTICLETERMINALPOSITION_PLC_BUS_INTERFACENAME |
| 35575 | Min. Anzugsdrehmoment | Anschlüsse (in Anschlussbildern) | Dezimalzahl | Nein | ARTICLETERMINALPOSITION_MINTIGHTENINGTORQUE |
| 35576 | Max. Anzugsdrehmoment | Anschlüsse (in Anschlussbildern) | Dezimalzahl | Nein | ARTICLETERMINALPOSITION_MAXTIGHTENINGTORQUE |
| 35578 | Abisolierlänge | Anschlüsse (in Anschlussbildern) | Dezimalzahl | Nein | ARTICLETERMINALPOSITION_STRIPPINGLENGTH |
| 35580 | X-Position Werkzeug | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_OPERATIONVEC_XPOS |
| 35581 | Y-Position Werkzeug | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_OPERATIONVEC_YPOS |
| 35582 | Z-Position Werkzeug | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_OPERATIONVEC_ZPOS |
| 35583 | X-Vektor Werkzeug | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_OPERATIONVEC_XDIR |
| 35584 | Y-Vektor Werkzeug | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_OPERATIONVEC_YDIR |
| 35585 | Z-Vektor Werkzeug | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_OPERATIONVEC_ZDIR |
| 35586 | Schaftdurchmesser | Anschlüsse (in Anschlussbildern) | Dezimalzahl | Nein | ARTICLETERMINALPOSITION_HOLEDIAMETER |
| 35587 | Klemmraumversatz | Anschlüsse (in Anschlussbildern) | Dezimalzahl | Nein | ARTICLETERMINALPOSITION_CLAMPSPACE_OFFSET |
| 35594 | X-Startpunkt Klemmraum | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_CLAMPSPACE_STARTXPOS |
| 35595 | Y-Startpunkt Klemmraum | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_CLAMPSPACE_STARTYPOS |
| 35596 | Z-Startpunkt Klemmraum | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_CLAMPSPACE_STARTZPOS |
| 35597 | X-Endpunkt Klemmraum | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_CLAMPSPACE_ENDXPOS |
| 35598 | Y-Endpunkt Klemmraum | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_CLAMPSPACE_ENDYPOS |
| 35599 | Z-Endpunkt Klemmraum | Anschlüsse (in Anschlussbildern) | Koordinaten (nur X oder Y) | Nein | ARTICLETERMINALPOSITION_CLAMPSPACE_ENDZPOS |
| 35800 | Identifizierender Name |  | Einsprachiger Text | Nein | DMUSERPROPDEFTABLEENTRY_IDENTNAME |
| 35801 | Anzeigename |  | Mehrsprachiger Text | Nein | DMUSERPROPDEFTABLEENTRY_DISPLAYNAME |
| 35802 | Zuordnungen |  | Einsprachiger Text | Nein | DMUSERPROPDEFTABLEENTRY_ASSIGNMENT |
| 35803 | Kategorie |  | Ganzzahlig | Nein | DMUSERPROPDEFTABLEENTRY_CATEGORY |
| 35804 | Eingabehilfe |  | Ganzzahlig | Nein | DMUSERPROPDEFTABLEENTRY_INPUTASSISTANCE_TYPE |
| 35805 | Vorgabewert für Auswahlliste |  | Mehrsprachiger Text | Ja, 100 | DMUSERPROPDEFTABLEENTRY_INPUTASSISTANCE_VALUE |
| 35806 | Nicht mehr verwenden |  | Bool (Wahr / Falsch) | Nein | DMUSERPROPDEFTABLEENTRY_DONOTUSE |
| 35807 | Anzahl Verwendungen | Gibt an, wie oft eine benutzerdefinierte Eigenschaft im Projekt verwendet wird. | Ganzzahlig | Nein | DMUSERPROPDEFTABLEENTRY_USAGE |
| 35808 | Artikelreferenz zugeordnet |  | Bool (Wahr / Falsch) | Nein | DMUSERPROPDEFTABLEENTRY_USED_BY_ARTICLEREF |
| 35809 | Eigenschaftswert übersetzen | Legt fest, ob der Eigenschaftswert bei der Übersetzung berücksichtigt wird. | Bool (Wahr / Falsch) | Nein | DMUSERPROPDEFTABLEENTRY_TRANSLATE_VALUE |
| 35810 | Datentyp |  | Ganzzahlig | Nein | DMUSERPROPDEFTABLEENTRY_DATATYPE |
| 35811 | Beschreibung |  | Mehrsprachiger Text | Nein | DMUSERPROPDEFTABLEENTRY_DESCRIPTION |
| 35900 | Identifizierender Name | Identifizierender Name der Eigenschaftsanordnung. | Einsprachiger Text | Nein | PROPERTYVIEW_IDENTNAME |
| 35901 | Anzeigename |  | Mehrsprachiger Text | Nein | PROPERTYVIEW_DISPLAYNAME |
| 35902 | Standardanordnung |  | Bool (Wahr / Falsch) | Nein | PROPERTYVIEW_DEFAULT |
| 35903 | Anzahl Verwendungen | Gibt an, wie oft eine benutzerdefinierte Eigenschaftsanordnung im Projekt verwendet wird. | Ganzzahlig | Nein | PROPERTYVIEW_USAGE |
| 35904 | Typ | Gibt den Typ der Eigenschaftsanordnung an:-1 = Undefiniert0 = Schaltzeichen1 = Artikelplatzierung (3D)2 = Modellansicht. | Ganzzahlig | Nein | PROPERTYVIEW_TYPE |
| 35905 | Zuordnung | Funktionsdefinition oder Symbol, dem die Eigenschaftsanordnung zugeordnet ist. | Einsprachiger Text | Nein | PROPERTYVIEW_ASSIGNMENT |
| 36000 | Bauteilbezeichnung | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_DESIGNATION |
| 36002 | Gruppierungsvorzeichen für Bauteilbezeichnung (automatisch) | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_DESIGNATIONPREFIX_AUTOMATIC |
| 36003 | Montageort (beschreibend, gemeinsam) | 3D-Platzierungen Artikelreferenzen | Mehrsprachiger Text | Nein | FUNCTION3D_MOUNTINGLOCATION_AUTOMATIC |
| 36004 | Mechanische Bearbeitung | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_ISFORMECHANICALPROCESSING |
| 36005 | Lokales Anschlussbild | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_ARTICLETERMINAL_ISLOCAL |
| 36006 | Durchbruch: Durchmesser der Bohrung | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_OPENING_DIAMETER |
| 36007 | Durchbruch: Nenndurchmesser der Gewindebohrung | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_OPENING_NOMINAL_DIAMETER |
| 36010 | Bauteil ist unverschiebbar am übergeordneten Bauteil befestigt | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_ELEM_ISFIXONPARENT |
| 36011 | Vorkonfektioniert | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_ISPREFABRICATED |
| 36012 | Gruppierungsvorzeichen für Bauteilbezeichnung | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_DESIGNATIONPREFIX |
| 36013 | Vollständige Bauteilbezeichnung | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_FULLDESIGNATION |
| 36014 | Bauteil benötigt Bohrung auf Montagefläche | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_ELEM_NEEDSDRILL |
| 36015 | Ist zugehöriges Montagematerial | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_ISINSTALLATIONMATERIAL |
| 36016 | Bezeichnung in der Mechanik-Struktur | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_IDENTNAME_IN_MECHANICAL_STRUCTURE |
| 36017 | Abstand zur Montagefläche | 3D-Platzierungen | Dezimalzahl | Nein | FUNCTION3D_MOUNTINGPLATEDISTANCE |
| 36018 | Bauteilbeschreibung | 3D-Platzierungen | Mehrsprachiger Text | Nein | FUNCTION3D_DESCRIPTION |
| 36020 | Durchbruch: Breite | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_OPENING_WIDTH |
| 36021 | Durchbruch: Höhe | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_OPENING_HEIGHT |
| 36022 | Durchbruch: Länge | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_OPENING_LENGTH |
| 36023 | Durchbruch: Radius / Fasenabstand | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_OPENING_CHAMFER_OR_RADIUS |
| 36024 | Durchbruch: Kantenlänge | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_OPENING_EDGE_LENGTH |
| 36025 | Durchbruch: Kontur | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_OPENING_CONTOUR |
| 36027 | Strecke / Kurve: Länge | 3D-Platzierungen | Koordinaten (nur X oder Y) | Nein | FUNCTION3D_ROUTING_SEGMENT_LENGTH_MANUAL |
| 36028 | Strecke / Kurve: Länge (automatisch) | 3D-Platzierungen | Koordinaten (nur X oder Y) | Nein | FUNCTION3D_ROUTING_SEGMENT_LENGTH_AUTOMATIC |
| 36029 | Strecke / Kurve: Querschnitt | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_ROUTING_SEGMENT_CROSSSECTION |
| 36030 | Strecke / Kurve: Automatisch erzeugt | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_ROUTING_SEGMENT_CREATED_AUTOMATICALLY |
| 36031 | Längenzuschlag | 3D-Platzierungen | Koordinaten (nur X oder Y) | Nein | FUNCTION3D_ROUTING_WIRING_OPENING_EXTRALENGTH |
| 36035 | Bauteil-ID (relativ zum Makro, EEC) | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_ID_RELATIVE |
| 36036 | Mehrfach-Durchbruch möglich | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_CANHAVE_MULTI_OPENING |
| 36037 | Durchbruchtiefe (Kupfer) | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_OPENING_DEPTH_COPPER |
| 36041 | Variable für platzierbare Funktionen (EEC One) | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_ID_EEC_ONE |
| 36042 | Name der M-CAD-Datei | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_DOCNAME |
| 36100 | Objekt wurde bei der Artikelmigration übertragen | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_OUTDATED |
| 36103 | Besitzt Montagelinien | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_HAS_MOUNTINGLINES |
| 36104 | Besitzt Montagepunkte | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_HAS_MOUNTINGPOINTS |
| 36105 | Besitzt Montageraster | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_HAS_MOUNTINGGRIDS |
| 36106 | Besitzt Bezugspunkte | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_HAS_REFERENCEPOINTS |
| 36452 | Bauraumname | Bauräume | Einsprachiger Text | Nein | INSTALLATIONSPACE_DESIGNATION |
| 36453 | Bauraumname (identifizierend) | Bauräume | Einsprachiger Text | Nein | INSTALLATIONSPACE_IDENTNAME |
| 36454 | Bauraumname (vollständig) | Bauräume | Einsprachiger Text | Nein | INSTALLATIONSPACE_FULLNAME |
| 36455 | Bauraumname (in Auswertungen) | Bauräume | Einsprachiger Text | Nein | INSTALLATIONSPACE_NAME_IN_EVALUATION_FORMAT |
| 36458 | Kupferstrang: Name | Kupferstränge Sammelschienen | Einsprachiger Text | Nein | COPPERBUNDLE_DESIGNATION |
| 36500 | Modell-Ansicht: Beschreibung | Modell-Ansichten | Mehrsprachiger Text | Nein | DMG_VIEWPLACEMENT_DESCRIPTION |
| 36502 | Modell-Ansicht: Eingefroren | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_FROZEN_VIEW |
| 36503 | Modell-Ansicht: Name | Modell-Ansichten | Einsprachiger Text | Nein | DMG_VIEWPLACEMENT_DESIGNATION |
| 36504 | Modell-Ansicht: Bauraumname | Modell-Ansichten | Einsprachiger Text | Nein | DMG_VIEWPLACEMENT_BUILDINGAREA |
| 36505 | Modell-Ansicht: Bauraumbeschreibung | Modell-Ansichten | Mehrsprachiger Text | Nein | DMG_VIEWPLACEMENT_BUILDINGAREA_DESCRIPTION |
| 36506 | Modell-Ansicht: Auswahlschema | Modell-Ansichten | Einsprachiger Text | Nein | DMG_VIEWPLACEMENT_UNITSELECTIONSCHEME |
| 36507 | Modell-Ansicht: Bauteilbeschriftung | Modell-Ansichten | Einsprachiger Text | Nein | DMG_VIEWPLACEMENT_LABELINGSCHEME |
| 36508 | Modell-Ansicht: Blickpunkt auf Montagefläche beziehen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_CONSIDER_TRANSFORM_OF_PLANE |
| 36509 | Modell-Ansicht: Basis-Bauteil | Modell-Ansichten | Einsprachiger Text | Nein | DMG_VIEWPLACEMENT_MAINOBJECT |
| 36510 | Modell-Ansicht: Maßstab | Modell-Ansichten | Einsprachiger Text | Nein | DMG_VIEWPLACEMENT_SCALE |
| 36511 | Modell-Ansicht: Schema für Automatische Bemaßung | Modell-Ansichten | Einsprachiger Text | Nein | DMG_VIEWPLACEMENT_ADIMSCHEME |
| 36513 | Modell-Ansicht: Bauteilsilhouetten anzeigen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_SHOWSILOUETTES |
| 36514 | Modell-Ansicht: Drehung | Modell-Ansichten | Ganzzahlig | Nein | DMG_VIEWPLACEMENT_ORIENTATION |
| 36515 | Modell-Ansicht: Frei verlegte Kabel anzeigen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_DISPLAY_FREE_ROUTED_CABLES |
| 36516 | Modell-Ansicht: Frei verlegte Drähte anzeigen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_FREE_ROUTED_CONNECTIONS |
| 36517 | Modell-Ansicht: Frei verlegte Schlauchleitungen anzeigen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_ROUTED_HOSE_ASSEMBLY |
| 36518 | Modell-Ansicht: Frei verlegte Schläuche anzeigen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_FREE_ROUTED_HOSES |
| 36519 | Bohrbild einblenden | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_SHOWDRILLINGS |
| 36520 | Biegungslinien darstellen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_SHOWBENDINGLINES |
| 36521 | Biegungsenden darstellen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_SHOWBENDINGAREA |
| 36522 | Biegungshinweis darstellen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_SHOWBENDINGHINT |
| 36523 | Modell-Ansicht: Frei verlegte Rohre anzeigen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_DISPLAY_FREE_ROUTED_PIPES |
| 36524 | Modell-Ansicht: Kantenbreite | Modell-Ansichten | Koordinaten (nur X oder Y) | Nein | DMG_VIEWPLACEMENT_EDGE_WIDTH |
| 36525 | Modell-Ansicht: Texte aus Bauraum nicht automatisch drehen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_TEXTDIRECTION_LIKE_IN_3D |
| 36526 | Modell-Ansicht: Bauteilkanten in Schwarz darstellen | Modell-Ansichten | Bool (Wahr / Falsch) | Nein | DMG_VIEWPLACEMENT_SHOWBLACKWIHTE |
| 40321 | Sicherheitskennwerte: Hierarchieebene 1 | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Mehrsprachiger Text | Nein | ARTICLE_SAFETYRELATED_HIERARCHY_1 |
| 40322 | Sicherheitskennwerte: Hierarchieebene 2 | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Mehrsprachiger Text | Nein | ARTICLE_SAFETYRELATED_HIERARCHY_2 |
| 40323 | Sicherheitskennwerte: Hierarchieebene 3 | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Mehrsprachiger Text | Nein | ARTICLE_SAFETYRELATED_HIERARCHY_3 |
| 40324 | Sicherheitskennwerte: Hierarchieebene 4 | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Mehrsprachiger Text | Nein | ARTICLE_SAFETYRELATED_HIERARCHY_4 |
| 40325 | Sicherheitskennwerte: Hierarchieebene 5 | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Mehrsprachiger Text | Nein | ARTICLE_SAFETYRELATED_HIERARCHY_5 |
| 40326 | Sicherheitskennwerte: Eingang (Erfassen) | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Bool (Wahr / Falsch) | Nein | ARTICLE_SAFETYRELATED_INPUT |
| 40327 | Sicherheitskennwerte: Ausgang (Reagieren) | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Bool (Wahr / Falsch) | Nein | ARTICLE_SAFETYRELATED_OUTPUT |
| 40328 | Sicherheitskennwerte: Logik (Auswerten) | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Bool (Wahr / Falsch) | Nein | ARTICLE_SAFETYRELATED_LOGIC |
| 40329 | Sicherheitskennwerte: PL | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Einsprachiger Text | Nein | ARTICLE_SAFETYRELATED_PL |
| 40330 | Sicherheitskennwerte: SIL CL | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Einsprachiger Text | Nein | ARTICLE_SAFETYRELATED_SILCL |
| 40331 | Sicherheitskennwerte: PFHD | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Dezimalzahl | Nein | ARTICLE_SAFETYRELATED_PFHD |
| 40332 | Sicherheitskennwerte: TMT1 | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Dezimalzahl | Nein | ARTICLE_SAFETYRELATED_TMT1 |
| 40333 | Sicherheitskennwerte: MTTFD | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Dezimalzahl | Nein | ARTICLE_SAFETYRELATED_MTTFD |
| 40334 | Sicherheitskennwerte: Lambda-D | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Dezimalzahl | Nein | ARTICLE_SAFETYRELATED_LAMBDAD |
| 40335 | Sicherheitskennwerte: MTTF | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Dezimalzahl | Nein | ARTICLE_SAFETYRELATED_MTTF |
| 40336 | Sicherheitskennwerte: MTBF | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Dezimalzahl | Nein | ARTICLE_SAFETYRELATED_MTBF |
| 40337 | Sicherheitskennwerte: RDF | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Dezimalzahl | Nein | ARTICLE_SAFETYRELATED_RDF |
| 40338 | Sicherheitskennwerte: B10 | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Dezimalzahl | Nein | ARTICLE_SAFETYRELATED_B10 |
| 40339 | Sicherheitskennwerte: B10 D | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Sicherheitskennwerte | Dezimalzahl | Nein | ARTICLE_SAFETYRELATED_B10D |
| 40340 | Verwendetes Bohrbild: Name | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_CONSTRUCTION_NAME |
| 40342 | Einsortierung des Artikels auf der Klemmenleiste | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_TERMINALSORTCODE |
| 40343 | Gewerk der Artikelreferenz | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLEREF_CRAFT |
| 40344 | Untergewerk der Artikelreferenz | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | ARTICLEREF_SUBCRAFT |
| 40345 | Sicherheitskennwerte: Verwendeter Anwendungsfall | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLEREF_USED_SAFETYRELATEDVALUE |
| 40346 | Sicherheitskennwerte: Verwendeter Anwendungsfall (Name) | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Nein | ARTICLEREF_USED_SAFETYRELATEDVALUE_NAME |
| 40347 | Harness proD GUID | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_HARNESSPROD_GUID |
| 40348 | AutomationML GUID (Zubehör) | Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_AML_GUID |
| 44004 | Bezeichnung | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_DESIGNATION |
| 44005 | Beschreibung (Segment) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Mehrsprachiger Text | Nein | DMPLAOBJECT_DESCRIPTION |
| 44006 | Technische Beschreibung | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Mehrsprachiger Text | Nein | DMPLAOBJECT_TECHNICAL_DESCRIPTION |
| 44007 | Dokumente | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Ja, 20 | DMPLAOBJECT_DOCUMENTS |
| 44008 | Makro | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_MACRO |
| 44009 | Bezeichnung (vollständig) | Segmentvorlagen (Vorplanung) Segmente (Vorplanung) Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | DMPLAOBJECT_FULLDESIGNATION |
| 44010 | Energiebedarf | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_POWER_REQUIREMENT |
| 44012 | Aufwand Planung | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_PLANNINGTIME |
| 44014 | Aufwand Bau | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_BUILDINGTIME |
| 44016 | Aufwand Software | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_PROGRAMMINGTIME |
| 44018 | Kalkulationswert | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Dezimalzahl | Nein | DMPLAOBJECT_PRICE |
| 44026 | Stufe im Baum | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_TREELEVEL |
| 44027 | Vorplanungspositionsnummer | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_POSITIONNUMBER |
| 44028 | Gesamtanzahl digitale SPS-Eingänge | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_DIGITAL_PLCINPUT_COUNT |
| 44029 | Gesamtanzahl digitale SPS-Ausgänge | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_DIGITAL_PLCOUTPUT_COUNT |
| 44030 | Gesamtanzahl analoge SPS-Eingänge | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_ANALOG_PLCINPUT_COUNT |
| 44031 | Gesamtanzahl analoge SPS-Ausgänge | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_ANALOG_PLCOUTPUT_COUNT |
| 44032 | Strukturkennzeichen (Segment) | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_ALLSTRUCTUREPARTS |
| 44035 | Technische Einrichtung | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_PCT_IDENTIFIER |
| 44036 | Messgröße | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_PCT_MEASURAND |
| 44037 | Verarbeitungsfunktion | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_PCT_FUNCTION |
| 44038 | Nummer | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_PCT_NUMBER |
| 44041 | Segmentdefinition: Anzeigename | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) Rohrklassen Stoffe | Mehrsprachiger Text | Nein | DMPLAOBJECT_DEFINITIONDISPLAYNAME |
| 44042 | Basissegmentdefinition: Anzeigename | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) Rohrklassen Stoffe | Mehrsprachiger Text | Nein | DMPLAOBJECT_BASEDEFINITIONDISPLAYNAME |
| 44043 | Realisierung | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_PCT_IMPLEMENTATION |
| 44044 | Bedienort | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_PCT_LOCATION |
| 44054 | Benutzerdefinierte Eigenschaft: Name | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Mehrsprachiger Text | Ja, 100 | DMPLAOBJECT_USERPROPERTY_NAME |
| 44055 | Benutzerdefinierte Eigenschaft: Wert | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Mehrsprachiger Text | Ja, 100 | DMPLAOBJECT_USERPROPERTY_VALUE |
| 44057 | Anzahl Verwendungen | Segmentvorlagen (Vorplanung) | Ganzzahlig | Nein | DMPLASPECIFICATION_OBJECT_COUNT |
| 44059 | Gesamtanzahl SPS-Zähleingänge | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_PLCCOUNTER_COUNT |
| 44060 | Sicherheitsrelevant | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Bool (Wahr / Falsch) | Nein | DMPLAOBJECT_SAFETY_RELEVANT |
| 44061 | GMP-relevant | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Bool (Wahr / Falsch) | Nein | DMPLAOBJECT_GMP_RELEVANT |
| 44062 | Qualitätsrelevant | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Bool (Wahr / Falsch) | Nein | DMPLAOBJECT_QUALITY_RELEVANT |
| 44063 | Verknüpfte Segmente: Name | Segmente (Vorplanung) | Einsprachiger Text | Ja, 10 | DMPLAOBJECT_LINK_CHILD_NAME |
| 44065 | Bezeichnung (sichtbar) | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_DESIGNATION_VISIBLE |
| 44066 | Ist mit übergeordneten Segmenten verknüpft | Segmente (Vorplanung) | Bool (Wahr / Falsch) | Nein | DMPLAOBJECT_IS_LINKED |
| 44067 | Besitzt verknüpfte Segmente | Segmente (Vorplanung) | Bool (Wahr / Falsch) | Nein | DMPLAOBJECT_HAS_LINKS |
| 44068 | Dokumente: Datei / Hyperlink | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Ja, 20 | DMPLAOBJECT_DOCUMENTS_URL |
| 44069 | Dokumente: Bezeichnung | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Mehrsprachiger Text | Ja, 20 | DMPLAOBJECT_DOCUMENTS_DESIGNATION |
| 44070 | Quellsegment | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_SOURCE_SEGMENT |
| 44071 | Zielsegment | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_TARGET_SEGMENT |
| 44072 | Rohrklasse | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_PIPECLASS |
| 44073 | Artikelnummer | Rohrklassen | Einsprachiger Text | Ja, 1000 | DMPLAPIPECLASS_PARTS |
| 44075 | Nenndruckstufe | Rohrklassen | Einsprachiger Text | Nein | DMPLAPIPECLASS_PRESSURELEVEL |
| 44076 | Werkstoff / Material | Rohrklassen | Mehrsprachiger Text | Nein | DMPLAPIPECLASS_MATERIAL |
| 44077 | Norm | Rohrklassen | Einsprachiger Text | Nein | DMPLAPIPECLASS_NORM |
| 44078 | Stoff | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_SUBSTANCE |
| 44079 | Anzahl digitale SPS-Eingänge | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_SEGMENT_DIGITAL_PLCINPUT_COUNT |
| 44080 | Anzahl digitale SPS-Ausgänge | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_SEGMENT_DIGITAL_PLCOUTPUT_COUNT |
| 44081 | Anzahl analoge SPS-Eingänge | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_SEGMENT_ANALOG_PLCINPUT_COUNT |
| 44082 | Anzahl analoge SPS-Ausgänge | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_SEGMENT_ANALOG_PLCOUTPUT_COUNT |
| 44083 | Anzahl SPS-Zähleingänge | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_SEGMENT_PLCCOUNTER_COUNT |
| 44084 | Makro für Vorplanung und RI-Fließbild | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_PLAMACRO |
| 44085 | Verbindungsquerschnitt / -durchmesser | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_CROSSSECTION |
| 59999 | EPLAN PPE-Projekt | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_IS_PPE |

## Einstellungen

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 10902 | Art des Projekts | Projekte | Ganzzahlig | Nein | PROJ_NUMERICTYPE |
| 11006 | Automatisch erzeugt | Seiten | Ganzzahlig | Nein | PAGE_AUTO_CREATED_INFO |
| 11015 | Formularname | Seiten | Einsprachiger Text | Nein | PAGE_FORMULAR |
| 11016 | Normblattname | Seiten | Einsprachiger Text | Nein | PAGE_FORMPLOT |
| 11017 | Seitentyp | Seiten | Einsprachiger Text | Nein | PAGE_TYPE |
| 11035 | Artikel bei Platzierung beibehalten | Seiten | Bool (Wahr / Falsch) | Nein | PAGE_KEPP_PARTS_BY_INSERT_MACRO |
| 11044 | Maßstab (Verhältnis) | Seiten | Einsprachiger Text | Nein | PAGE_SCALE_RELATION |
| 11048 | Maßstab | Seiten | Dezimalzahl | Nein | PAGE_SCALE |
| 11051 | Raster | Seiten | Koordinaten (nur X oder Y) | Nein | PAGE_SOLUTION |
| 11058 | Fremddokument | Seiten | Einsprachiger Text | Nein | PAGE_EXTDOCUMENT |
| 11060 | Normentausch: Nicht drehen und spiegeln | Seiten | Bool (Wahr / Falsch) | Nein | PAGE_DONT_CHANGESTANDARD |
| 12002 | Zeilenhöhe | Normblätter | Koordinaten (nur X oder Y) | Ja, 1000 | FRAME_POSITIONSIZE |
| 12003 | Auswertungsbereich Anfangspunkt X | Normblätter | Koordinaten (nur X oder Y) | Nein | FRAME_EVALUATION_AREA_START_POINT_X |
| 12004 | Auswertungsbereich Anfangspunkt Y | Normblätter | Koordinaten (nur X oder Y) | Nein | FRAME_EVALUATION_AREA_START_POINT_Y |
| 12005 | Anzahl der Spalten | Normblätter | Ganzzahlig | Nein | FRAME_NUMBER_OF_ROWS |
| 12006 | Anzahl der Zeilen | Normblätter | Ganzzahlig | Nein | FRAME_NUMBER_OF_COLUMS |
| 12007 | Anzahl Ladder | Normblätter | Ganzzahlig | Nein | FRAME_NUMBER_OF_LADDERS |
| 12009 | Zeilennummerierungsformat | Normblätter | Ganzzahlig | Nein | FRAME_FORMAT_OF_NUMBERING_POSITION |
| 12010 | Alphabetische Spalten- / Zeileneinteilung | Normblätter | Einsprachiger Text | Nein | FRAME_ROW_AND_POS_ALPHABETIC_ALLOCATION |
| 12011 | Spaltennummerierungsformat | Normblätter | Ganzzahlig | Nein | FRAME_FORMAT_OF_ROW_NUMBERS |
| 12012 | Spaltenbreite asynchron | Normblätter | Einsprachiger Text | Nein | FRAME_ASYNCHRON_ROWS |
| 12013 | Zeilen asynchron | Normblätter | Einsprachiger Text | Nein | FRAME_ASYNCHRON_POS |
| 12014 | Spaltenbezeichnung auf nicht-logischen Seiten anzeigen | Normblätter | Bool (Wahr / Falsch) | Nein | FRAME_ROW_DESC_OF_NON_LOGICAL_PAGES |
| 12016 | Seitenausrichtung / Papierformat | Normblätter | Ganzzahlig | Nein | FRAME_PAGE_ORIENTATION |
| 12025 | Anfangswert (Spalte) | Normblätter Seiten | Ganzzahlig | Nein | FRAME_PATH_STARTID |
| 12026 | Anfangswert (Zeile) | Normblätter Seiten | Ganzzahlig | Nein | FRAME_POS_STARTID |
| 12027 | Zeichenkette für Spaltenbezeichnung | Seiten Normblätter | Einsprachiger Text | Ja, 1000 | FRAME_PATH_ALPHASET |
| 12028 | Zeichenkette für Zeilenbezeichnung | Normblätter Seiten | Einsprachiger Text | Ja, 1000 | FRAME_POS_ALPHASET |
| 12029 | Anzahl Zeichen für Spalte / Zeile | Normblätter Seiten | Ganzzahlig | Nein | FRAME_PATHPOS_MINDIGITS |
| 12030 | Seitennamen hinzufügen | Normblätter | Bool (Wahr / Falsch) | Nein | FRAME_ADD_PAGENAME |
| 12031 | Durchgehende Zeilen- / Spaltenbezeichnung | Normblätter | Bool (Wahr / Falsch) | Nein | FRAME_CONT_LADDER_DESCR |
| 12032 | Zeichenkette für Alphanumerisch | Normblätter | Einsprachiger Text | Nein | FRAME_DEFAULT_ALPHASET |
| 12035 | Invertierte Spaltennummerierung | Normblätter | Bool (Wahr / Falsch) | Nein | FRAME_PATH_REVERSE_NUMBERING |
| 12036 | Invertierte Zeilennummerierung | Normblätter | Bool (Wahr / Falsch) | Nein | FRAME_POS_REVERSE_NUMBERING |
| 12050 | DXF-Export: Blockname | Normblätter | Einsprachiger Text | Nein | FRAME_DXF_BLOCKNAME |
| 12051 | DXF-Export: Einfügepunkt X | Normblätter | Koordinaten (nur X oder Y) | Nein | FRAME_DXF_INSERTXCOORD |
| 12052 | DXF-Export: Einfügepunkt Y | Normblätter | Koordinaten (nur X oder Y) | Nein | FRAME_DXF_INSERTYCOORD |
| 12059 | Kontaktspiegelabstand (am Schaltzeichen) | Normblätter Seiten | Koordinaten (nur X oder Y) | Nein | FRAME_MOTORSWITCH_OFFSET |
| 12060 | Kontaktspiegelabstand (im Schaltplanpfad) | Normblätter Seiten | Koordinaten (nur X oder Y) | Nein | FRAME_CONTACTIMAGE_OFFSET |
| 12061 | Kontaktspiegelverschiebung | Normblätter Seiten | Koordinaten (nur X oder Y) | Nein | FRAME_CONTACTIMAGE_POSTPONEMENT |
| 12062 | Eigenschaftsanordnung: Y-Koordinate automatisch (Schaltplanpfad) | Normblätter Seiten | Koordinaten (nur X oder Y) | Nein | FRAME_AUTOY_OFFSET |
| 12063 | Eigenschaftsanordnung: X-Koordinate automatisch (Schaltplanpfad) | Normblätter Seiten | Koordinaten (nur X oder Y) | Nein | FRAME_AUTOX_OFFSET |
| 12102 | Spaltenbreite | Normblätter | Koordinaten (nur X oder Y) | Ja, 1000 | FRAME_PATHSIZE |
| 12103 | Auswertungsrichtung | Normblätter | Ganzzahlig | Nein | FRAME_EVALUATION_DIRECTION |
| 12104 | BMK-Übernahme: Suchrichtung entsprechend GOST-Norm | Normblätter | Bool (Wahr / Falsch) | Nein | FRAME_DEVTAGSEARCHDIR_FOR_GOST |
| 12105 | BMK-Übernahme: Nur von Kästen | Normblätter | Bool (Wahr / Falsch) | Nein | FRAME_DEVTAGSEARCHDIR_BOXESONLY |
| 13001 | Normblatt zur Bearbeitung des Formulars | Formulare | Einsprachiger Text | Nein | FORM_FRAME |
| 13002 | Formularhandhabung | Formulare | Bool (Wahr / Falsch) | Nein | FORM_BEHAVIOUR_STATIC_DYN |
| 13003 | Überschrift neu erzeugen bei Änderung | Formulare | Einsprachiger Text | Nein | FORM_RECREATE_MAINTITLE |
| 13004 | Spaltenanzahl | Formulare | Ganzzahlig | Nein | FORM_COL_SIZE |
| 13005 | Zeilenanzahl | Formulare | Ganzzahlig | Nein | FORM_ROW_SIZE |
| 13006 | Spaltenbreite | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_COL_WIDTH |
| 13007 | Zeilenhöhe | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_ROW_HEIGHT |
| 13008 | Formularausrichtung | Formulare | Ganzzahlig | Nein | FORM_BEHAVIOUR_ROW_COL |
| 13009 | Dynamische Zeilenanpassung | Formulare | Bool (Wahr / Falsch) | Nein | FORM_DYN_ROW_ALIGN |
| 13010 | Brücken: Höhe erste Ebene | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_JUMPER_FIRST_LEV_HEIGHT |
| 13011 | Verbindungen | Formulare | Einsprachiger Text | Nein | FORM_CABLE_ASSIGNMENT_CONNECTED_TERMINALS |
| 13012 | Brücken: Abstand | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_JUMPER_DISTANCE |
| 13013 | Brücken: Radius der Anschlusspunkte | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_JUMPER_CP_RADIUS |
| 13014 | Brücken: Radius für Knoten | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_JUMPER_NODE_RADIUS |
| 13015 | Brücken: Höhe für interne Drahtbrücke / Einlegebrücke | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_JUMPER_WIRE_JUMPER_INT_HEIGHT |
| 13016 | Brücken: Höhe für externe Drahtbrücke / Einlegebrücke | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_JUMPER_WIRE_JUMPER_EXT_HEIGHT |
| 13017 | Brücken: Abstand für interne Drahtbrücke / Einlegebrücke | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_JUMPER_WIRE_JUMPER_INT_DISTANCE |
| 13018 | Brücken: Abstand für externe Drahtbrücke / Einlegebrücke | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_JUMPER_WIRE_JUMPER_EXT_DISTANCE |
| 13019 | Format für automatische Seitenbeschreibung | Formulare | Einsprachiger Text | Nein | FORM_PAGE_DESC_AUTOFORMAT |
| 13020 | Zuordnung Index zu Grafik | Formulare | Einsprachiger Text | Nein | FORM_RELATION_INDEX_GRAPHIC |
| 13021 | Zuordnung Funktionsdefinition zu Grafik | Formulare | Einsprachiger Text | Nein | FORM_RELATION_FUNC_DEF_GRAPHIC |
| 13026 | Zuordnung Eigenschaft / Wert zu Grafik | Formulare Normblätter | Einsprachiger Text | Ja, 10 | FORM_BEHAVIOUR_PROPERTY_VAL_GRAPHIC |
| 13027 | Brückendarstellung | Formulare | Ganzzahlig | Nein | FORM_JUMPER_BEHAVIOUR_LINE_ANGLE |
| 13028 | Brücken: Linienstärke | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_JUMPER_LINE_WIDTH |
| 13029 | Brücken: Anzahl der Ebenen | Formulare | Ganzzahlig | Nein | FORM_JUMPER_COUNT_LAYERS |
| 13030 | Kabeltabelle: Kabelanzahl A | Formulare | Ganzzahlig | Nein | FORM_CABLE_COUNT_A |
| 13031 | Kabeltabelle: Abstand Datenbereich A | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLE_DISTANCE_A |
| 13032 | Kabeltabelle: Kabelanzahl B | Formulare | Ganzzahlig | Nein | FORM_CABLE_COUNT_B |
| 13033 | Kabeltabelle: Abstand Datenbereich B | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLE_DISTANCE_B |
| 13034 | Kabeltabelle: Kabelanzahl C | Formulare | Ganzzahlig | Nein | FORM_CABLE_COUNT_C |
| 13035 | Kabeltabelle: Abstand Datenbereich C | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLE_DISTANCE_C |
| 13036 | Kabeltabelle: Kabelanzahl D | Formulare | Ganzzahlig | Nein | FORM_CABLE_COUNT_D |
| 13037 | Kabeltabelle: Abstand Datenbereich D | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLE_DISTANCE_D |
| 13038 | Kabeltabelle: Kabelanzahl intern | Formulare | Ganzzahlig | Nein | FORM_CABLE_COUNT_INT |
| 13039 | Kabeltabelle: Abstand Datenbereich intern | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLE_DISTANCE_INT |
| 13040 | Kabeltabelle: Kabelanzahl extern | Formulare | Ganzzahlig | Nein | FORM_CABLE_COUNT_EXT |
| 13041 | Kabeltabelle: Abstand Datenbereich extern | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLE_DISTANCE_EXT |
| 13042 | Symbolabstand nebeneinander | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_SYMBOL_ABREAST_DISTANCE |
| 13043 | Anzahl der Symbole nebeneinander | Formulare | Ganzzahlig | Nein | FORM_SYMBOL_ABREAST_COUNT |
| 13052 | Mehrstockklemmen zusammenfassen | Formulare | Bool (Wahr / Falsch) | Nein | FORM_COMBINE_MULTI_LEVEL_TERMINAL |
| 13053 | Strukturkennzeichenübersicht | Formulare | Ganzzahlig | Nein | FORM_STRUCTIDENTIFIEROVERVIEW_TYPE |
| 13054 | Externe und interne Ziele als interne zusammenfassen | Formulare | Bool (Wahr / Falsch) | Nein | FORM_ALL_TARGETS_AS_INTERN |
| 13055 | Normblatt zur Bearbeitung des Formulars bei Auswertungen verwenden | Formulare | Bool (Wahr / Falsch) | Nein | FORM_FRAME_USEINREPORTS |
| 13056 | Formular für Folgeseiten | Formulare | Einsprachiger Text | Nein | FORM_NEXTPAGEFORM |
| 13059 | Trennzeichen bei mehreren Werten | Formulare | Einsprachiger Text | Nein | FORM_MULTIRESULT_SEPARATOR |
| 13060 | Für die ersten x Sortierungseigenschaften Überschriften erzeugen | Formulare | Ganzzahlig | Nein | FORM_RECREATE_MAINTITLE_WITH_SORTPROPS |
| 13064 | Hersteller- / Lieferantenliste | Formulare | Ganzzahlig | Nein | FORM_MANUFACTURERLIST_TYPE |
| 13067 | Kabeltabelle: Abstand Kopfbereich A | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLETABLE_HEAD_DISTANCE_A |
| 13068 | Kabeltabelle: Abstand Kopfbereich B | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLETABLE_HEAD_DISTANCE_B |
| 13069 | Kabeltabelle: Abstand Kopfbereich C | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLETABLE_HEAD_DISTANCE_C |
| 13070 | Kabeltabelle: Abstand Kopfbereich D | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLETABLE_HEAD_DISTANCE_D |
| 13071 | Kabeltabelle: Abstand Kopfbereich intern | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLETABLE_HEAD_DISTANCE_INT |
| 13072 | Kabeltabelle: Abstand Kopfbereich extern | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CABLETABLE_HEAD_DISTANCE_EXT |
| 13076 | Kabeltabelle: Kopfbereich quer zum Datenbereich | Formulare | Bool (Wahr / Falsch) | Nein | FORM_CABLETABLEHEAD_ATHWART_TO_CABLETABLEDATA |
| 13077 | Anschlussplan: Anzahl Stufen intern | Formulare | Ganzzahlig | Nein | FORM_CONNECTIONDIAGRAM_INT_STEPCOUNT |
| 13078 | Anschlussplan: Stufenabstand intern | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CONNECTIONDIAGRAM_INT_STEPOFFSET |
| 13079 | Anschlussplan: Anzahl Stufen extern | Formulare | Ganzzahlig | Nein | FORM_CONNECTIONDIAGRAM_EXT_STEPCOUNT |
| 13080 | Anschlussplan: Stufenabstand extern | Formulare | Koordinaten (nur X oder Y) | Nein | FORM_CONNECTIONDIAGRAM_EXT_STEPOFFSET |
| 13081 | Leerzeilen unterdrücken | Formulare | Bool (Wahr / Falsch) | Nein | FORM_SUPPRESS_EMPTY_LINES |
| 13082 | Trennzeichen bei zusammengefassten Werten | Formulare | Einsprachiger Text | Nein | FORM_SUMUP_SEPARATOR |
| 13083 | Kopfbereich bei neuer Spalte wiederholen | Formulare | Bool (Wahr / Falsch) | Nein | FORM_HEADER_IN_EACH_COLUMN |
| 13084 | Anschlussplan: Alle Ziele anzeigen | Formulare | Bool (Wahr / Falsch) | Nein | FORM_CONNECTIONDIAGRAM_ALLDESTINATIONS |
| 13085 | Anschlussplan: An Zielen nur angeschlossene Anschlüsse ausgeben | Formulare | Bool (Wahr / Falsch) | Nein | FORM_CONNECTIONDIAGRAM_ONLYCONNECTEDCP |
| 13086 | Anschlussplan: Interne Ziele tabellarisch | Formulare | Bool (Wahr / Falsch) | Nein | FORM_CONNECTIONDIAGRAM_TABULAR_INT |
| 13087 | Anschlussplan: Externe Ziele tabellarisch | Formulare | Bool (Wahr / Falsch) | Nein | FORM_CONNECTIONDIAGRAM_TABULAR_EXT |
| 13088 | Revisionsausgabe mit gelöschten Seiten | Formulare | Bool (Wahr / Falsch) | Nein | FORM_REVISIONOVERVIEW_TYPE_DELETEDPAGES |
| 13090 | Anschlussplan: Ziele nicht teilen | Formulare | Bool (Wahr / Falsch) | Nein | FORM_CONNECTIONDIAGRAM_NOBREAKBLOCKSONPAGE |
| 13096 | Brücken: Grafik für Stegmöglichkeiten unterdrücken | Formulare | Bool (Wahr / Falsch) | Nein | FORM_SUPPRESS_SADDLE_JUMPER_POSSIBILITY |
| 13102 | Zeilenhöhe dynamisch anpassen | Formulare | Bool (Wahr / Falsch) | Nein | FORM_ADAPT_LINEHEIGHT |
| 13103 | Jedes Betriebsmittel in neuer Spalte beginnen | Formulare | Bool (Wahr / Falsch) | Nein | FORM_NEWDEVICE_IN_NEWCOLUMN |
| 13104 | Aufreihbares Zubehör anzeigen | Formulare | Bool (Wahr / Falsch) | Nein | FORM_VIEW_ACCESSORIES |
| 13105 | Kein Zeilenvorschub für Überschrift | Formulare | Bool (Wahr / Falsch) | Nein | FORM_CAPTION_IN_DATALINE |
| 13106 | Revisionsausgabeart | Formulare | Bool (Wahr / Falsch) | Nein | FORM_REVISIONOVERVIEW_TYPE |
| 13108 | Variable Werte durch Text ersetzen | Formulare | Bool (Wahr / Falsch) | Nein | FORM_REPLACE_VAR_VALUES |
| 13109 | Symbolvariante | Formulare | Ganzzahlig | Nein | FORM_SYMBOL_VARIANT |
| 13110 | Überschrift bei neuer Spalte wiederholen | Formulare | Bool (Wahr / Falsch) | Nein | FORM_CAPTION_IN_EACH_COLUMN |
| 13111 | In einer Zeile zusammenfassen nach | Formulare | Einsprachiger Text | Nein | FORM_SUMMARIZE_PROPERTIES |
| 13112 | Grafiken am Einfügepunkt ausrichten (ohne Positionsrahmen) | Formulare | Bool (Wahr / Falsch) | Nein | FORM_USE_PHT_INSERTIONPOINT |
| 13113 | Anschlussbezogene Ausgabe | Formulare | Bool (Wahr / Falsch) | Nein | FORM_TERMCON_ORIENTED |
| 13114 | Nur verwendete Symbole ausgeben | Formulare | Bool (Wahr / Falsch) | Nein | FORM_USED_SYMBOLS_ONLY |
| 13115 | Alphanumerische Sortierung für interne Anschlüsse | Formulare | Bool (Wahr / Falsch) | Nein | FORM_SORT_INTERNAL_CP |
| 13117 | Anschlussplan: Zielverfolgung über alle Betriebsmittel | Formulare | Bool (Wahr / Falsch) | Nein | FORM_CONNECTIONDIAGRAM_TARGET_TRACKING_ALL |
| 13118 | Kürzungsregeln auf alle Struktureigenschaften anwenden | Formulare | Bool (Wahr / Falsch) | Nein | FORM_REDUCE_STRUCTUREPROPS |
| 13119 | Alle untergeordneten Segmente ausgeben | Formulare | Bool (Wahr / Falsch) | Nein | FORM_SUBOBJECTS_RECURSIVE |
| 13120 | Eigenschaftsplatzierungen unterdrücken | Formulare | Bool (Wahr / Falsch) | Nein | FORM_SUPRESS_PROPERTYINSTANCES |
| 13121 | Anschlussplan: Nur angeschlossene Anschlüsse ausgeben | Formulare | Bool (Wahr / Falsch) | Nein | FORM_MAINEVALUATIONOBJECT_ONLYCONNECTEDCP |
| 15013 | Raster | Symbolbibliotheken | Koordinaten (nur X oder Y) | Nein | SYMBLIB_LOGICGRID |
| 15030 | Normblatt zur Bearbeitung des Symbols | Symbolbibliotheken | Einsprachiger Text | Nein | SYMBLIB_FRAME |
| 16003 | Standardvariante | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_VARIANT |
| 16004 | Standard-Eigenschaftsanordnung für Variante A | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_PROPERTYSET_A |
| 16005 | Standard-Eigenschaftsanordnung für Variante B | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_PROPERTYSET_B |
| 16006 | Standard-Eigenschaftsanordnung für Variante C | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_PROPERTYSET_C |
| 16007 | Standard-Eigenschaftsanordnung für Variante D | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_PROPERTYSET_D |
| 16008 | Standard-Eigenschaftsanordnung für Variante E | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_PROPERTYSET_E |
| 16009 | Standard-Eigenschaftsanordnung für Variante F | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_PROPERTYSET_F |
| 16012 | Neue Platzierung verhindern | Symbole | Bool (Wahr / Falsch) | Nein | SYMB_PREVENTPLACEMENT |
| 16014 | Standard-Eigenschaftsanordnung für Variante G | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_PROPERTYSET_G |
| 16015 | Standard-Eigenschaftsanordnung für Variante H | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_PROPERTYSET_H |
| 16018 | Hauptfunktion | Symbole | Ganzzahlig | Nein | SYMB_MAINFUNCTION |
| 16019 | Eigensicher | Symbole | Ganzzahlig | Nein | SYMB_INTRINSICALLYSAFE |
| 16027 | Symbol-Darstellungsart (codiert) | Symbole | Ganzzahlig | Nein | SYMB_SYBMOLFUNCTIONTYPE |
| 16033 | Standard-Eigenschaftsanordnung für Variante A (alternativ) | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_ALTERNATIVE_PROPERTYSET_A |
| 16034 | Standard-Eigenschaftsanordnung für Variante B (alternativ) | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_ALTERNATIVE_PROPERTYSET_B |
| 16035 | Standard-Eigenschaftsanordnung für Variante C (alternativ) | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_ALTERNATIVE_PROPERTYSET_C |
| 16036 | Standard-Eigenschaftsanordnung für Variante D (alternativ) | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_ALTERNATIVE_PROPERTYSET_D |
| 16037 | Standard-Eigenschaftsanordnung für Variante E (alternativ) | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_ALTERNATIVE_PROPERTYSET_E |
| 16038 | Standard-Eigenschaftsanordnung für Variante F (alternativ) | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_ALTERNATIVE_PROPERTYSET_F |
| 16039 | Standard-Eigenschaftsanordnung für Variante G (alternativ) | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_ALTERNATIVE_PROPERTYSET_G |
| 16040 | DXF-Export: Blockname | Symbole | Einsprachiger Text | Nein | SYMB_DXF_BLOCKNAME |
| 16041 | Standard-Eigenschaftsanordnung für Variante H (alternativ) | Symbole | Ganzzahlig | Nein | SYMB_DEFAULT_ALTERNATIVE_PROPERTYSET_H |
| 16042 | Signaltrennend | Symbole | Ganzzahlig | Nein | SYMB_POTENTIALSEPARATED |
| 16044 | Sicherheitsfunktion | Symbole | Ganzzahlig | Nein | SYMB_SAFETYRELEVANT |
| 18061 | Raster | Basisklasse für Normblätter und Formulare | Koordinaten (nur X oder Y) | Nein | FORMBASE_LOGICGRID |
| 19100 | Parametrische Regel | Platzierungen auf Ebenen | Einsprachiger Text | Nein | INSTANCE_PARAMETRICRULE |
| 19314 | Platzhalterobjekt in EPLAN Cogineer verwenden | Platzhaltertexte | Bool (Wahr / Falsch) | Nein | PLACEHOLDER_USE_IN_COGINEER |
| 19315 | Platzhalterobjekt nach dem Erzeugen mit EPLAN Cogineer entfernen | Platzhaltertexte | Bool (Wahr / Falsch) | Nein | PLACEHOLDER_DELETE_AFTER_USAGE_IN_COGINEER |
| 20021 | Querverweisanzeige | Funktionen | Ganzzahlig | Nein | FUNC_SUPPRESSCROSSREFERENCE |
| 20035 | BMK-Übernahme: Suchrichtung | Funktionen | Ganzzahlig | Nein | FUNC_DEVICETAGSEARCHDIRECTION |
| 20049 | Querverweisanzeige: Darstellung | Funktionen Abbruchstellen | Ganzzahlig | Nein | FUNC_SHOWCROSSREFERENCEBLOCK |
| 20050 | Querverweisanzeige: Anzahl der Zeilen / Spalten | Funktionen Abbruchstellen | Ganzzahlig | Nein | FUNC_NOOFCROSSREFERENCESINBLOCK |
| 20051 | Sichtbares BMK als vollständiges BMK verwenden | Basisklasse für Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_DISABLENAMEEVALUATION |
| 20059 | Automatisch erzeugt | Funktionen Kabel (in Auswertungen) | Bool (Wahr / Falsch) | Nein | FUNC_CABLEAUTOCREATED |
| 20060 | Kabel beim Summieren der Kabellängen nicht berücksichtigen | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CABLE_DONT_COUNT_CABLE |
| 20061 | Nur manuelle Kabelauswahl zulassen | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CABLE_ALLOW_ONLY_MANUEL_CABLE_SELECTION |
| 20062 | Mehrere PE-Adern zulassen | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CABLE_ALLOW_TWO_PE_WIRES |
| 20063 | Beidseitig aufgelegte Abschirmungen zulassen | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CABLE_ALLOW_TWO_SIDED_SHIELDINGS |
| 20064 | Kabel: Quelle und Ziel tauschen | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CABLE_CHANGE_SOURCE_TARGET |
| 20066 | Spezialkabel | Funktionen Kabel (in Auswertungen) | Bool (Wahr / Falsch) | Nein | FUNC_CABLE_SPECIALCABLE |
| 20069 | Kabelname enthält Quelle / Ziel | Funktionen Verbindungen | Bool (Wahr / Falsch) | Nein | FUNC_CABLE_DEVTAGINCLUDESSOURCEORDESTINATION |
| 20083 | Kabelplanformular | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_FORM |
| 20084 | Kabelanschlussplanformular | Funktionen | Einsprachiger Text | Nein | FUNC_CABLE_CONNECTIONDIAGRAM_FORM |
| 20085 | Keine Ausgabe im Kabelplan | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CABLE_NODIAGRAM |
| 20086 | Keine Ausgabe in der Kabelübersicht | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CABLE_NOOVERVIEW |
| 20087 | Keine Ausgabe im Kabelanschlussplan | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CABLE_NOCONNECTIONDIAGRAM |
| 20089 | Grafikdarstellung in Auswertungen | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_EVALUATION_GRAPHICCODE |
| 20091 | Name automatisch vergeben | Kabel (in Auswertungen) Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CABLEAUTONAMED |
| 20092 | Kabeldarstellungsplanformular | Funktionen Artikelreferenzen (Darstellungsarten zusammengefasst) Artikelreferenzen | Einsprachiger Text | Nein | FUNC_CABLE_LAYOUT_FORM |
| 20093 | In Betriebsmittelliste unterdrücken (wenn gefiltert) | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_SUPPRESSINDEVICELIST |
| 20094 | Aus Makro nicht-platziert einfügen | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_INSERTNOTPLACED |
| 20187 | SPS-Diagrammformular | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDIAGRAM_FORM |
| 20198 | Querverweisanzeige: Max. Anzahl der Querverweise | Funktionen Abbruchstellen | Ganzzahlig | Nein | FUNC_MAXCROSSREFERENCEBLOCKCOUNT |
| 20223 | Teilklemmenindex | Funktionen | Einsprachiger Text | Nein | FUNC_DISTRIBUTED_TERMINALINDEX |
| 20229 | Hauptklemmen nicht verwenden | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_STRIP_DONT_USE_MAINTERMINAL |
| 20230 | Klemmenausführung | Funktionen Funktionsschablonen | Ganzzahlig | Nein | FUNC_TERMINAL_FUNCTION |
| 20231 | Klemme mit LED | Funktionen Funktionsschablonen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_HAS_LED |
| 20232 | Klemme geöffnet | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_IS_OPEN |
| 20234 | Formular für Verbindungsdiagramm | Funktionen | Einsprachiger Text | Nein | FUNC_FORM_CONNECTIONDIAGRAM |
| 20242 | Topologie: Automatisch erzeugte Strecke | Strecken (Topologie) | Bool (Wahr / Falsch) | Nein | CABLINGSEGMENT_AUTO_CREATED |
| 20247 | Topologie: Verbindungsfilter | Strecken (Topologie) | Einsprachiger Text | Nein | CABLINGSEGMENT_CONNECTIONFILTER |
| 20283 | Klemme mit Steckadapter | Funktionen Funktionsschablonen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_HAS_PLUGADAPTER |
| 20290 | Klemmenleisten: Gleiche Bezeichnungen innerhalb von Mehrstockklemmen zulassen | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_STRIP_ALLOW_SAME_NAME_IN_MULTILEVEL |
| 20291 | Schaltbrücke (intern) | Funktionen | Ganzzahlig | Nein | FUNC_TERMINAL_SWITCHABLE_JUMPER_INTERN |
| 20292 | Schaltbrücke (extern) | Funktionen | Ganzzahlig | Nein | FUNC_TERMINAL_SWITCHABLE_JUMPER_EXTERN |
| 20293 | Schaltsperre | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_SWITCH_LOCK |
| 20314 | Querverweisanzeige: Nebenfunktion als Hauptfunktion | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CROSSREFERENCEMAINFUNCTION |
| 20331 | Durchgehende Verbindung zwischen den Anschlüssen | Funktionen | Ganzzahlig | Nein | FUNC_DISTRIBUTOR_CONNECTED_CONNPOINTS |
| 20365 | Beschriftungsart | Funktionen Funktionsschablonen | Einsprachiger Text | Nein | FUNC_TERMINAL_LABELTYPE |
| 20367 | Klemme: Geräteposition | Funktionen | Einsprachiger Text | Nein | FUNC_TERMINALDEVICEPOSITION |
| 20430 | Nummerierung mit SPS-Daten verhindern | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLCPREVENT_NUMERATION |
| 20440 | Schaltschranklegendenformular | Funktionen Modell-Ansichten | Einsprachiger Text | Nein | FUNC_MOUNTINGPLATE_CAPTIONFORM |
| 20441 | Erzeugung der Schaltschranklegende unterdrücken | Funktionen Modell-Ansichten | Bool (Wahr / Falsch) | Nein | FUNC_MOUNTINGPLATE_FORMEVALUATION |
| 20442 | Größenveränderung sperren | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_BOXFUNCTION_FIXEDSIZE |
| 20476 | Nicht-platzierte Nebenfunktionen an der Hauptfunktion verwalten | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_MANAGE_FUNCTIONS |
| 20806 | Klemmen- / Steckerplanformular | Funktionen | Einsprachiger Text | Nein | FUNC_TERMINAL_FORM |
| 20808 | Stegbrückenmöglichkeit | Funktionen | Ganzzahlig | Nein | FUNC_TERMINAL_JUMPERBAR |
| 20809 | Sortierkennung (Klemme / Steckerkontakt) | Funktionen | Einsprachiger Text | Nein | FUNC_TERMINALSORTCODE |
| 20810 | Sortierung (grafisch) | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINALSORTCODE_GRAPHIC |
| 20811 | Gleiche Bezeichnung erlaubt | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_CANHAVESAMENAME |
| 20812 | Mit SPS-Daten nummeriert | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CHANGEDBYPLCNUM |
| 20813 | Nummerierung mit SPS-Daten über die Funktion hinaus fortsetzen | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_CONTINUEPLCNUM |
| 20851 | Keine Ausgabe im Klemmen- / Steckerplan | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_NODIAGRAM |
| 20852 | Klemmen- / Steckeranschlussplanformular | Funktionen | Einsprachiger Text | Nein | FUNC_TERMINAL_CONNECTIONDIAGRAM |
| 20853 | Keine Ausgabe im Klemmen- / Steckeranschlussplan | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_NOCONNECTIONDIAGRAM |
| 20857 | Keine Ausgabe in der Klemmenleisten- / Steckerübersicht | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_NOOVERVIEW |
| 20859 | Keine Ausgabe im Klemmenaufreihplan | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_TERMINAL_NOLINEUPDIAGRAM |
| 20906 | Artikel ist Bestandteil eines Moduls | Funktionen | Bool (Wahr / Falsch) | Ja, 50 | FUNC_ARTICLE_MODULE_PART |
| 24020 | Anzeige aller Querverweise | Abbruchstellen | Bool (Wahr / Falsch) | Nein | INTERRUPTIONPOINT_SHOWALLCROSSREFERENCES |
| 24021 | Anzeige aller Gegenziele | Abbruchstellen | Bool (Wahr / Falsch) | Nein | INTERRUPTIONPOINT_SHOWALLCROSSREFERENCEDESTINATIONS |
| 24350 | Trennzeichen zwischen Abbruchstelle und Querverweis | Abbruchstellen | Einsprachiger Text | Nein | INTERRUPTIONPOINT_CROSSREFSEPARATOR |
| 24809 | Sortierkennung (Abbruchstelle) | Abbruchstellen | Ganzzahlig | Nein | INTERRUPTIONPOINT_SORT_CODE |
| 31005 | Autoconnect-Linie: Farbe | Verbindungen Verbindungsdefinitionspunkte Autoconnect-Linien Potenzialdefinitionspunkte Symbole | Ganzzahlig | Nein | CONNECTION_ACL_COLOR |
| 31015 | Autoconnect-Linie: Linientyp | Verbindungen Verbindungsdefinitionspunkte Autoconnect-Linien Potenzialdefinitionspunkte Symbole | Ganzzahlig | Nein | CONNECTION_ACL_STYLE |
| 31016 | Autoconnect-Linie: Linienstärke | Verbindungen Verbindungsdefinitionspunkte Autoconnect-Linien Potenzialdefinitionspunkte Symbole | Koordinaten (nur X oder Y) | Nein | CONNECTION_ACL_WIDTH |
| 31017 | Autoconnect-Linie: Ebene | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte Symbole | Ganzzahlig | Nein | CONNECTION_ACL_LAYER |
| 31018 | Autoconnect-Linie: Musterlänge | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte Symbole | Koordinaten (nur X oder Y) | Nein | CONNECTION_ACL_FACTOR |
| 31045 | Erlaubte Daten | Verbindungsdefinitionspunkte | Ganzzahlig | Nein | CDP_ALLOWED_DATA |
| 31046 | Manuell gesetzt | Verbindungsdefinitionspunkte Verbindungen | Bool (Wahr / Falsch) | Nein | CDP_SET_AS_MANUAL |
| 31057 | Druck- / Steuerleitung | Verbindungen Verbindungsdefinitionspunkte | Ganzzahlig | Nein | CONNECTION_WORKING_CONTROL_LINE |
| 31142 | Verbindung: Zugehörigkeit | Verbindungen Verbindungsdefinitionspunkte | Ganzzahlig | Nein | CONNECTION_KINDOFWIRE |
| 33301 | Rohrleitung: Quelle und Ziel tauschen | Rohrleitungsdefinitionspunkte | Bool (Wahr / Falsch) | Nein | PIPELINE_CHANGE_SOURCE_TARGET |
| 36026 | Bauraum: Verbindungsfilter | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_ROUTING_CONNECTIONFILTER |
| 36032 | In Montageliste ausgeben | 3D-Platzierungen | Bool (Wahr / Falsch) | Nein | FUNCTION3D_CREATE_MOUNTINGLIST |
| 36456 | Automatisch erzeugt | Bauräume | Ganzzahlig | Nein | INSTALLATIONSPACE_AUTO_CREATED_INFO |
| 36457 | Artikel bei Platzierung beibehalten | Bauräume | Bool (Wahr / Falsch) | Nein | INSTALLATIONSPACE_KEPP_PARTS_BY_INSERT_MACRO |
| 44034 | Nicht auf Vorplanungstiefe prüfen | Segmente (Vorplanung) | Bool (Wahr / Falsch) | Nein | DMPLAOBJECT_IGNORE_MANDATORY_PROPERTY_VERIFICATION |
| 44040 | Fertig vorgeplant | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Bool (Wahr / Falsch) | Nein | DMPLAOBJECT_PLANNING_COMPLETE |
| 44074 | Automatisch erzeugt | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_AUTO_CREATED_INFO |
| 44086 | Farbe für RI-Fließbild | Segmente (Vorplanung) | Ganzzahlig | Nein | DMPLAOBJECT_PID_COLOR |

## Endkunde

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 10130 | Endkunde: Kurzname | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERID |
| 10131 | Endkunde: Anrede | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERTITLE |
| 10132 | Endkunde: Name 1 | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERNAME1 |
| 10133 | Endkunde: Name 2 | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERNAME2 |
| 10134 | Endkunde: Name 3 | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERNAME3 |
| 10135 | Endkunde: Straße | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERSTREET |
| 10136 | Endkunde: Postfach | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERPOBOX |
| 10137 | Endkunde: PLZ (Wohnort) | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERZIPCODE |
| 10138 | Endkunde: Wohnort | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERCITY |
| 10139 | Endkunde: Land | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERCOUNTRY |
| 10140 | Endkunde: Telefon | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERTELEPHONE |
| 10141 | Endkunde: Telefax | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERFAX |
| 10142 | Endkunde: E-Mail | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMEREMAIL |
| 10143 | Endkunde: PLZ (Postfach) | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMERZIPCODEPOBOX |
| 10144 | Endkunde: Staat / Region | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMER_REGION |
| 10145 | Endkunde: Langname | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMER_LONGNAME |
| 10146 | Endkunde: Kundennummer | Projekte | Einsprachiger Text | Nein | PROJ_ENDCUSTOMER_NUMBER |
| 10147 | Endkunde: Beschreibung | Projekte | Mehrsprachiger Text | Nein | PROJ_ENDCUSTOMER_NOTE |

## Formate

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 10096 | Blockeigenschaft: Format (Potenzialdefinition) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_POTENTIALDEFINITION |
| 10097 | Blockeigenschaft: Format (Rohrleitungsdefinition) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_PROCESSDEFINITION |
| 10600 | Blockeigenschaft: Format (Allgemeine Betriebsmittel) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_FUNCTION |
| 10601 | Blockeigenschaft: Format (Klemmenleistendefinition) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_TERMINAL_DEF_TEXT |
| 10602 | Blockeigenschaft: Format (Klemme) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_TERMINAL |
| 10603 | Blockeigenschaft: Format (Steckerdefinition) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_CONNECTOR_DEF_TEXT |
| 10604 | Blockeigenschaft: Format (Steckerkontakt) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_PLUG |
| 10605 | Blockeigenschaft: Format (SPS-Kasten) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_PLCBOX |
| 10606 | Blockeigenschaft: Format (SPS-Anschluss) | Projekte | Einsprachiger Text | Ja, 300 | PROJ_BLOCKFORMAT_PLC_TERMINAL |
| 10607 | Blockeigenschaft: Format (Kabel / Abschirmung) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_CABLE |
| 10608 | Blockeigenschaft: Format (Verbindung) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_CONNECTION |
| 10609 | Blockeigenschaft: Format (Abbruchstelle) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_INTERRUPTION_POINT |
| 10610 | Blockeigenschaft: Format (Gerätekasten) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_BLACKBOX |
| 10611 | Blockeigenschaft: Format (Geräteanschluss) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_DEVICE_END_TERMINAL |
| 10612 | Blockeigenschaft: Format (Sammelschiene) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_BUSBAR |
| 10613 | Blockeigenschaft: Format (Fluid-Betriebsmittel) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_FLUIDDEVICE |
| 10614 | Blockeigenschaft: Format (Fluid-Leitungsverteiler / -verbinder) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_DISTRIBUTOR |
| 10615 | Blockeigenschaft: Format (PLT-Stelle) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_PLT |
| 10616 | Blockeigenschaft: Format (PLT-Stellenfunktion) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_PLTFUNCTION |
| 10617 | Blockeigenschaft: Format (Verfahrenstechnik) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_PROCESS |
| 10618 | Blockeigenschaft: Format (Seite) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_PAGE |
| 10619 | Blockeigenschaft: Format (Mechanik) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_MECHANIC |
| 10620 | Anzeigeformat sichtbares BMK: Allgemeine Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_FUNCTION |
| 10621 | Anzeigeformat sichtbares BMK: Klemmenleistendefinition | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_TERMINAL_DEF_TEXT |
| 10622 | Anzeigeformat sichtbares BMK: Klemme | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_TERMINAL |
| 10623 | Anzeigeformat sichtbares BMK: Steckerdefinition | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_CONNECTOR_DEF_TEXT |
| 10624 | Anzeigeformat sichtbares BMK: Steckerkontakt | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_PLUG |
| 10625 | Anzeigeformat sichtbares BMK: SPS-Kasten | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_PLCBOX |
| 10626 | Anzeigeformat sichtbares BMK: SPS-Anschluss | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_PLC_TERMINAL |
| 10627 | Anzeigeformat sichtbares BMK: Kabel / Abschirmung | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_CABLE |
| 10629 | Anzeigeformat sichtbares BMK: Abbruchstelle | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_INTERRUPTION_POINT |
| 10630 | Anzeigeformat sichtbares BMK: Gerätekasten | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_BLACKBOX |
| 10631 | Anzeigeformat sichtbares BMK: Geräteanschluss | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_DEVICE_END_TERMINAL |
| 10632 | Anzeigeformat sichtbares BMK: Sammelschiene | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_BUSBAR |
| 10633 | Anzeigeformat sichtbares BMK: Fluid-Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_FLUIDDEVICE |
| 10634 | Anzeigeformat sichtbares BMK: Fluid-Leitungsverteiler / -verbinder | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_DISTRIBUTOR |
| 10635 | Anzeigeformat sichtbares BMK: Kabelverbindung | Projekte | Einsprachiger Text | Nein | PROJ_DTFORMAT_CABLE_CONNECTION |
| 10640 | Anzeigeformat vollständiges BMK: Allgemeine Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_FUNCTION |
| 10641 | Anzeigeformat vollständiges BMK: Klemmenleistendefinition | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_TERMINAL_DEF_TEXT |
| 10642 | Anzeigeformat vollständiges BMK: Klemme | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_TERMINAL |
| 10643 | Anzeigeformat vollständiges BMK: Steckerdefinition | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_CONNECTOR_DEF_TEXT |
| 10644 | Anzeigeformat vollständiges BMK: Steckerkontakt | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_PLUG |
| 10645 | Anzeigeformat vollständiges BMK: SPS-Kasten | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_PLCBOX |
| 10646 | Anzeigeformat vollständiges BMK: SPS-Anschluss | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_PLC_TERMINAL |
| 10647 | Anzeigeformat vollständiges BMK: Kabel / Abschirmung | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_CABLE |
| 10649 | Anzeigeformat vollständiges BMK: Abbruchstelle | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_INTERRUPTION_POINT |
| 10650 | Anzeigeformat vollständiges BMK: Gerätekasten | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_BLACKBOX |
| 10651 | Anzeigeformat vollständiges BMK: Geräteanschluss | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_DEVICE_END_TERMINAL |
| 10652 | Anzeigeformat vollständiges BMK: Sammelschiene | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_BUSBAR |
| 10653 | Anzeigeformat vollständiges BMK: Fluid-Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_FLUIDDEVICE |
| 10654 | Anzeigeformat vollständiges BMK: Fluid-Leitungsverteiler / -verbinder | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_DISTRIBUTOR |
| 10655 | Anzeigeformat vollständiges BMK: Kabelverbindung | Projekte | Einsprachiger Text | Nein | PROJ_FULLDTFORMAT_CABLE_CONNECTION |
| 10660 | Blockeigenschaft: Ersetzungstext | Projekte | Einsprachiger Text | Ja, 10 | PROJ_BLOCKPROPERTY_REPLACETEXT |
| 10661 | Blockeigenschaft: Format (Vorplanung) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_PLAOBJECT |
| 10662 | Blockeigenschaft: Format (Strukturkasten) | Projekte | Einsprachiger Text | Ja, 100 | PROJ_BLOCKFORMAT_LOCATIONBOX |
| 11091 | Blockeigenschaft: Format | Seiten | Einsprachiger Text | Ja, 100 | PAGE_BLOCK_FORMAT |
| 12033 | Normblattabmessungen X-Achse | Normblätter | Koordinaten (nur X oder Y) | Nein | FRAME_AREA_X |
| 12034 | Normblattabmessungen Y-Achse | Normblätter | Koordinaten (nur X oder Y) | Nein | FRAME_AREA_Y |
| 13116 | Eigenschaften formatiert: Ersetzungstext | Formulare | Einsprachiger Text | Ja, 10 | FORM_BLOCKPROPERTY_REPLACETEXT |
| 19010 | Farbe | Platzierungen auf Ebenen | Ganzzahlig | Nein | INSTANCE_COLOR |
| 19011 | Linientyp | Basisklasse für grafische Objekte | Ganzzahlig | Nein | INSTANCE_LINESTYLE |
| 19015 | Schriftart |  | Ganzzahlig | Nein | INSTANCE_FONT |
| 19016 | Ausrichtung |  | Ganzzahlig | Nein | INSTANCE_DIRECTION |
| 19017 | Schriftgröße | Symbolbibliotheken | Koordinaten (nur X oder Y) | Nein | INSTANCE_SIZE |
| 19019 | Ebene | Platzierungen auf Ebenen | Ganzzahlig | Nein | INSTANCE_LAYER |
| 19101 | Unsichtbar | Platzierungen auf Ebenen | Ganzzahlig | Nein | INSTANCE_INVISIBLE |
| 19102 | Linienstärke | Basisklasse für grafische Objekte | Koordinaten (nur X oder Y) | Nein | INSTANCE_WIDTH |
| 19150 | Musterlänge | Basisklasse für grafische Objekte | Koordinaten (nur X oder Y) | Nein | INSTANCE_LINESTYLEFACTOR |
| 19151 | Linienabschluss | Basisklasse für grafische Objekte | Ganzzahlig | Nein | INSTANCE_LINEENDTYPE |
| 19620 | Blockeigenschaft: Format | QR-Codes | Einsprachiger Text | Nein | QRCODE_BLOCK_FORMAT |
| 20099 | Trennzeichen für Klemmenleisten- und Steckerdefinitionen | Funktionen | Einsprachiger Text | Nein | FUNC_DEFINITIONSEPARATOR |
| 20202 | Blockeigenschaft: Format | Funktionen Symbole Abbruchstellen Verbindungsdefinitionspunkte PLT-Stellenplatzierungen Segmentvorlagen (Vorplanung) Segmente (Vorplanung) Segmentdefinitionen (Vorplanung) Strukturkästen Potenzialdefinitionspunkte | Einsprachiger Text | Ja, 100 | FUNC_BLOCK_FORMAT |
| 20205 | Anzeigeformat sichtbares BMK | Funktionen Abbruchstellen | Einsprachiger Text | Nein | FUNC_DTFORMAT |
| 20206 | Anzeigeformat vollständiges BMK | Funktionen Abbruchstellen | Einsprachiger Text | Nein | FUNC_FULLDTFORMAT |
| 20545 | Blockeigenschaft: Format (Funktionstext, automatisch) | Funktionen | Einsprachiger Text | Nein | FUNC_BLOCKFORMAT_FUNCTIONTEXT |
| 44002 | Anzeigeformat: Segmentname | Segmentdefinitionen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECTDEFINITION_DISPLAYNAME_FORMAT |
| 44003 | Segmentdefinition: Anzeigename | Segmentdefinitionen (Vorplanung) | Mehrsprachiger Text | Nein | DMPLAOBJECTDEFINITION_NAME |
| 44033 | Strukturkennzeichen ohne Trennzeichen | Segmentdefinitionen (Vorplanung) | Bool (Wahr / Falsch) | Nein | DMPLAOBJECTDEFINITION_NO_STRUCTUREDELIMITER |
| 44045 | Segmentdefinition: Vorzeichen | Segmentdefinitionen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECTDEFINITION_PREFIX |
| 44046 | Segmentdefinition: Vorzeichen auch am Anfang | Segmentdefinitionen (Vorplanung) | Bool (Wahr / Falsch) | Nein | DMPLAOBJECTDEFINITION_BEGINWITHPREFIX |

## Funktionsdaten

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 20026 | Funktionsdefinition | Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Symbole | Mehrsprachiger Text | Nein | FUNC_COMPONENTTYPE |
| 20038 | Anschlussbezeichnungen (alle) | Funktionen | Einsprachiger Text | Nein | FUNC_ALLCONNECTIONDESIGNATIONS |
| 20039 | Anschlussbeschreibungen (alle) | Funktionen | Einsprachiger Text | Nein | FUNC_ALLCONNECTIONDESCRIPTIONS |
| 20115 | Funktionsdefinition: Kategorie | Funktionen Symbole Verbindungen | Mehrsprachiger Text | Nein | FUNC_CATEGORY |
| 20116 | Funktionsdefinition: Gruppe | Funktionen Symbole Verbindungen | Mehrsprachiger Text | Nein | FUNC_GROUP |
| 20117 | Funktionsdefinition: Beschreibung | Funktionen Symbole Verbindungen | Mehrsprachiger Text | Nein | FUNC_DESC |
| 20121 | Darstellungsart | Basisklasse für Funktionen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Verbindungen | Ganzzahlig | Nein | FUNC_TYPE |
| 20122 | Hauptfunktion | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_MAINFUNCTION |
| 20123 | Signaltrennend | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_POTENTIALSEPARATED |
| 20188 | Funktionsdefinition: Kategorie/Gruppe/Definition | Funktionen Symbole Verbindungen | Einsprachiger Text | Nein | FUNC_CATEGORY_GROUP_ID |
| 20216 | Sicherheitsfunktion | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_SAFETYRELEVANT |
| 20217 | Netzverbindend | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_NETCONNECTING |
| 20296 | Anschlussquerschnitt / -durchmesser (alle) | Funktionen | Einsprachiger Text | Nein | FUNC_ALLCONNECTIONCROSSSECTIONS |
| 20297 | Anschlussquerschnitt / -durchmesser (alle, abhängig von der BMK-Übernahme) | Funktionen | Einsprachiger Text | Nein | FUNC_DEPENDEDALLCONNECTIONCROSSSECTIONS |
| 20321 | Anschlusslogik: Ist Extern | Funktionen | Bool (Wahr / Falsch) | Ja, 1000 | FUNC_LOGDEF_ISEXTERN |
| 20322 | Anschlusslogik: Ist Kabelanschluss | Funktionen | Bool (Wahr / Falsch) | Ja, 1000 | FUNC_LOGDEF_ISCABLE |
| 20323 | Anschlusslogik: Eigensicherheit möglich | Funktionen | Bool (Wahr / Falsch) | Ja, 1000 | FUNC_LOGDEF_SAFETYISPOSSIBLE |
| 20324 | Anschlusslogik: Erlaubte Anzahl Ziele | Funktionen | Ganzzahlig | Ja, 1000 | FUNC_LOGDEF_DESTINATIONCOUNT |
| 20325 | Anschlusslogik: Erlaubte Anzahl Stegbrücken | Funktionen | Ganzzahlig | Ja, 1000 | FUNC_LOGDEF_BRIDGECOUNT |
| 20326 | Anschlusslogik: Potenzial weiterleiten nach | Funktionen | Einsprachiger Text | Ja, 1000 | FUNC_LOGDEF_POTENTIALTRANSFER |
| 20327 | Anschlusslogik: Verbraucher / Quelle nach | Funktionen | Einsprachiger Text | Ja, 1000 | FUNC_LOGDEF_CONSUMERSOURCES |
| 20329 | Anschlusslogik: Anschlussart | Funktionen | Ganzzahlig | Ja, 1000 | FUNC_LOGDEF_TYPE |
| 20369 | Anschlusslogik: Druck- / Steueranschluss | Funktionen | Ganzzahlig | Ja, 1000 | FUNC_LOGDEF_CONTROL_CONPOINT |
| 20370 | Anschlusslogik: Potenzialtyp | Funktionen | Ganzzahlig | Ja, 1000 | FUNC_LOGDEF_POTENTIAL_TYPE |
| 20371 | Anschlusslogik: Signalart | Funktionen | Ganzzahlig | Ja, 1000 | FUNC_LOGDEF_SIGNAL_TYPE |
| 20372 | Anschlusslogik: Gleiche Anschlussbezeichnung erlauben | Funktionen | Bool (Wahr / Falsch) | Ja, 1000 | FUNC_LOGDEF_ALLOW_SAME_DESIGNATION |
| 20373 | Anschlusslogik: Symbolanschluss | Funktionen | Ganzzahlig | Ja, 1000 | FUNC_LOGDEF_SYMBOL_CONPOINT |
| 20378 | Anschlussmaß Quelle | Funktionen | Einsprachiger Text | Nein | FUNC_TERMINALSIZE_SOURCE |
| 20379 | Anschlussmaß Ziel | Funktionen | Einsprachiger Text | Nein | FUNC_TERMINALSIZE_DESTINATION |
| 20449 | Anschlusslogik: Zielverfolgung (SPS) nach | Funktionen | Einsprachiger Text | Ja, 1000 | FUNC_LOGDEF_PLCDESTINATIONTRACKING |
| 20475 | Geräteschutz | Funktionen Verbindungen Segmente (Vorplanung) | Bool (Wahr / Falsch) | Nein | FUNC_FIXED_DEVICE |
| 31030 | Eigensicher | Verbindungen Funktionen | Bool (Wahr / Falsch) | Nein | INTRINSICSAFETY |

## Keine Kategorie

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 1005 | Anzahl Verwendungen | Strukturkennzeichen | Einsprachiger Text | Nein | LOCATION_USE |
| 1006 | Kennzeichnungsblock (ID) | Strukturkennzeichen | Ganzzahlig | Nein | LOCATION_TYPENAME_NR |
| 1122 | Anlage (führende Kennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Ja, 10 | DESIGNATION_PLANT_LEADINGPARTS |
| 1222 | Einbauort (führende Kennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Ja, 10 | DESIGNATION_LOCATION_LEADINGPARTS |
| 1322 | Funktionale Zuordnung (führende Kennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Ja, 10 | DESIGNATION_FUNCTIONALASSIGNMENT_LEADINGPARTS |
| 1422 | Aufstellungsort (führende Kennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Ja, 10 | DESIGNATION_PLACEOFINSTALLATION_LEADINGPARTS |
| 1522 | Dokumentenart (führende Kennzeichen) | Seiten Segmente (Vorplanung) | Einsprachiger Text | Ja, 10 | DESIGNATION_DOCTYPE_LEADINGPARTS |
| 1622 | Benutzerdefinierte Struktur (führende Kennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Ja, 10 | DESIGNATION_USERDEFINED_LEADINGPARTS |
| 1722 | Anlagennummer (führende Kennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Ja, 10 | DESIGNATION_INSTALLATIONNUMBER_LEADINGPARTS |
| 1822 | Produkt (führende Kennzeichen) | Seiten Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Einsprachiger Text | Ja, 10 | DESIGNATION_PRODUCT_LEADINGPARTS |
| 2000 | Objektidentifikation | Basisklasse für alle Objekte | Einsprachiger Text | Nein | PROPUSER_DBOBJECTID |
| 10001 | Projektstruktur: Funktionale Zuordnung | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_FUNCTIONALASSIGNMENT |
| 10002 | Projektstruktur: Anlage | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_PLANT |
| 10003 | Projektstruktur: Aufstellungsort | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_PLACEOFINSTALLATION |
| 10004 | Projektstruktur: Einbauort | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_LOCATION |
| 10005 | Projektstruktur: Anlagennummer | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_INSTALLATIONNUMBER |
| 10006 | Projektstruktur: Dokumentenart | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_DOCUMENTSTRUCTURE |
| 10007 | Projektstruktur: Benutzerdefinierte Struktur | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_USERDEFINED |
| 10008 | Registerkarte für BMK-Strukturen anzeigen | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_SHOWDTSTRUCTURETAB |
| 10012 | Seitenname ins BMK übernehmen | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_HIERARCHY_TAKEOVERPAGENUMBER |
| 10018 | Trennzeichen der Strukturen | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_CODEDSEPARATORS |
| 10019 | Verschachtelungen der Betriebsmittelkennzeichen | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_CODEDNESTINGSETTINGS |
| 10029 | Übergeordnete Strukturformate | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_SPECIALDINMODE |
| 10050 | Strukturformat für Seiten | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_PAGE |
| 10051 | Strukturformat für Abbruchstellen | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_INTERRUPTIONPOINT |
| 10053 | Strukturformat für Sammelschienen | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_BUSBAR |
| 10054 | Übergeordnetes Strukturformat für alle Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_MASTERDEVICES |
| 10055 | Strukturformat für allgemeine Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_STANDARDDEVICE |
| 10056 | Strukturformat für Gerätekästen | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_DEVICEBOX |
| 10057 | Strukturformat für SPS- / Bus-Technik | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_PLC |
| 10059 | Strukturformat für Klemmenleisten | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_TERMINALSTRIP |
| 10060 | Strukturformat für Stecker | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_PLUG |
| 10061 | Strukturformat für Kabel | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_CABLE |
| 10062 | Strukturformat für Strukturkästen | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_LOCATIONBOX |
| 10063 | Strukturformat für Fluid-Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_FLUID |
| 10064 | Strukturformat für Fluid-Leitungsverteiler / -verbinder | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_DISTRIBUTOR |
| 10070 | Namensformat für Seiten | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_PAGE |
| 10071 | Namensformat für Abbruchstellen | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_INTERRUPTIONPOINT |
| 10073 | Namensformat für Sammelschienen | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_BUSBAR |
| 10074 | Übergeordnetes Namensformat für alle Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_MASTERDEVICES |
| 10075 | Namensformat für Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_DEVICE |
| 10076 | Namensformat für Gerätekästen | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_DEVICEBOX |
| 10077 | Namensformat für SPS- / Bus-Technik | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_PLC |
| 10079 | Namensformat für Klemmenleisten | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_TERMINALSTRIP |
| 10080 | Namensformat für Stecker | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_PLUG |
| 10081 | Namensformat für Kabel | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_CABLE |
| 10082 | Namensformat für Strukturkästen | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_LOCATIONBOX |
| 10083 | Namensformat für Fluid-Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_FLUID |
| 10084 | Namensformat für Fluid-Leitungsverteiler / -verbinder | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_DISTRIBUTOR |
| 10085 | Strukturformat für Mechanik-Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_MECHANIC |
| 10086 | Namensformat für Mechanik-Betriebsmittel | Projekte | Einsprachiger Text | Nein | PROJ_NAMEFORMAT_MECHANIC |
| 10089 | Reihenfolge der Projektstrukturen im Seitennamen | Projekte | Einsprachiger Text | Nein | PROJ_STRUCTURESEQUENCE_PAGE |
| 10090 | BMK in Einzelfeldern bearbeiten | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_DEVTAGFORMAT_ENABLECUSTOMFORMAT |
| 10091 | Reihenfolge der BMK-Einzeleigenschaften | Projekte | Einsprachiger Text | Nein | PROJ_DEVTAGFORMAT_CUSTOMFORMAT |
| 10092 | Aufbau der Baumstruktur im Seitenbaum | Projekte | Einsprachiger Text | Nein | PROJ_PDD_PAGES_HIERARCHY |
| 10093 | Beschreibende Kennzeichen im Seitenbaum anzeigen | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_PDD_PAGES_SHOWDESCRSTRUCTURES |
| 10094 | Aufbau der Baumstruktur in den Geräte-Navigatoren | Projekte | Einsprachiger Text | Nein | PROJ_PDD_DEVICES_HIERARCHY |
| 10095 | Beschreibende Kennzeichen in Geräte-Navigatoren anzeigen | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_PDD_DEVICES_SHOWDESCRSTRUCTURES |
| 10099 | Projektstruktur: Produkt | Projekte | Einsprachiger Text | Nein | PROJ_HIERARCHY_PRODUCT |
| 10154 | Revision: Änderungsverfolgung gestartet | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_REVISION_LOG_START |
| 10164 | Revision: Temporäres Referenzprojekt | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_REVISION_TEMP |
| 10184 | Eindeutige Projekt-ID | Projekte | Einsprachiger Text | Nein | PROJ_GUID |
| 10303 | Projektstatus | Projekte | Einsprachiger Text | Nein | PROJ_PROJECTSTATUS |
| 10450 | Dokumentenart an den Anfang des Seitennamens stellen | Projekte | Bool (Wahr / Falsch) | Nein | PROJ_ADDITIONALINFO_DOCUMENTSTRUCTUREINFRONT |
| 11012 | Seitenzähler | Seiten | Einsprachiger Text | Nein | PAGE_COUNTER |
| 11013 | Seitenunterzähler | Seiten | Einsprachiger Text | Nein | PAGE_SUBCOUNTER |
| 11029 | Seitentyp (ID) | Seiten | Ganzzahlig | Nein | PAGE_TYPE_NUMERIC |
| 11038 | Im Objektkennzeichen abgelegtes Strukturkennzeichen ohne Trennzeichen | Seiten | Einsprachiger Text | Nein | PAGE_OBJECTCODE_STRUCTUREWITHOUTPREFIX |
| 11039 | Im Objektkennzeichen abgelegter Teil des Strukturkennzeichens ohne Trennzeichen | Seiten | Einsprachiger Text | Ja, 10 | PAGE_OBJECTCODE_STRUCTUREPARTWITHOUTPREFIX |
| 11045 | Enthält Ergebnisse in der Suchergebnisliste | Seiten | Bool (Wahr / Falsch) | Nein | PAGE_HAS_RESULTS_IN_SEARCHLIST |
| 11075 | Zugehörige Projektrevision (aus Änderungsverfolgung) | Seiten Bauräume | Einsprachiger Text | Ja, 1000 | PAGE_REVISION_LOG_PROJ_REV |
| 13044 | Grafik für Steg-, Einlege- und Drahtbrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_ALL |
| 13045 | Grafik für interne Drahtbrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_WIRE_INTERNAL |
| 13046 | Grafik für externe Drahtbrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_WIRE_EXTERNAL |
| 13047 | Grafik für Stegbrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_SADDLE |
| 13048 | Verbindungsgrafik | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_CONNECTION |
| 13049 | Symbolgrafik | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_SYMBOL |
| 13050 | Schaltzeichengrafik | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_SYMBREF |
| 13051 | Grafik für Drahtbrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_WIRE |
| 13057 | Ergebnis aus Wertesatz | Formulare | Einsprachiger Text | Ja, 100 | FORM_PLACEHOLDER_RESULT |
| 13058 | Platzhalterformat | Formulare | Einsprachiger Text | Nein | FORM_PLACEHOLDER_FORMAT |
| 13061 | Grafik für Brücken: Buchsenseite | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_JACKSIDE |
| 13062 | Grafik für Brücken: Stiftseite | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_PINSIDE |
| 13063 | Fortlaufende Nummer | Formulare | Einsprachiger Text | Nein | FORM_LINE_COUNTER |
| 13065 | Grafik für interne Stegbrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_SADDLE_INTERNAL |
| 13066 | Grafik für externe Stegbrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_SADDLE_EXTERNAL |
| 13073 | Grafik für Einlegebrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_INSERTION |
| 13074 | Grafik für interne Einlegebrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_INSERTION_INTERNAL |
| 13075 | Grafik für externe Einlegebrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_INSERTION_EXTERNAL |
| 13089 | Anzahl zusammengefasster Datensätze | Formulare | Ganzzahlig | Nein | FORM_SUMOBJECTSCOUNT |
| 13091 | Anzahl der Ziele am Zielanschluss | Formulare | Ganzzahlig | Nein | FORM_NUMBER_OF_DESTINATION |
| 13092 | Fortlaufende Nummer (Überschrift) | Formulare | Einsprachiger Text | Nein | FORM_LINE_COUNTER_CAPTION |
| 13093 | Fortlaufende Nummer (Seite) | Formulare | Einsprachiger Text | Nein | FORM_LINE_COUNTER_PAGE |
| 13094 | Grafik für interne Schaltbrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_SWITCHABLE_INTERNAL |
| 13095 | Grafik für externe Schaltbrücken | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_JUMPER_SWITCHABLE_EXTERNAL |
| 13107 | Kabelgrafik | Formulare | Einsprachiger Text | Nein | FORM_GRAPHIC_CABLELINE |
| 15098 | Name der ursprünglichen Symbolbibliothek | Symbolbibliotheken | Einsprachiger Text | Nein | SYMBLIB_OLDNAME |
| 15500 | Funktionsdefinitionsbibliothek | Funktionsdefinitionsbibliotheken | Einsprachiger Text | Nein | FCTDEFLIB_NAME |
| 15501 | Änderungsdatum | Funktionsdefinitionsbibliotheken | Uhrzeit / Datum | Nein | FCTDEFLIB_LASTMODIFICATIONDATE |
| 15502 | Versionsinformation der Funktionsdefinitionsbibliothek | Funktionsdefinitionsbibliotheken | Einsprachiger Text | Nein | FCTDEFLIB_VERSION |
| 15503 | Ersteller | Funktionsdefinitionsbibliotheken | Einsprachiger Text | Nein | FCTDEFLIB_CREATOR |
| 15504 | Erstellungsdatum | Funktionsdefinitionsbibliotheken | Uhrzeit / Datum | Nein | FCTDEFLIB_CREATIONDATE |
| 15505 | Letzter Bearbeiter: Anmeldename | Funktionsdefinitionsbibliotheken | Einsprachiger Text | Nein | FCTDEFLIB_LASTMODIFICATOR |
| 16000 | Name | Symbole | Einsprachiger Text | Nein | SYMB_NAME |
| 16001 | Anschlussnummer | Symbole | Einsprachiger Text | Ja, 100 | SYMB_CONNECTIONNUMBER |
| 16028 | Symbol-Darstellungsart | Symbole | Einsprachiger Text | Nein | SYMB_SYBMOLFUNCTIONTYPE_NAME |
| 16031 | Symbolvarianten-Zuordnung (intern) | Symbole | Ganzzahlig | Ja, 4 | SYMB_IMPORTEDVARIANTMAPPING |
| 16050 | Eigenschaftsanordnungen | Symbole | Mehrsprachiger Text | Ja, 64 | SYMB_PROPERTYINSTANCESET |
| 16901 | Variantenname | Symbolvarianten | Einsprachiger Text | Nein | SYMBVAR_NAME |
| 16902 | Anzahl der Anschlusspunkte | Symbolvarianten | Ganzzahlig | Nein | SYMBVAR_CONPOINTSCOUNT |
| 19008 | Benutzerdefinierte Eigenschaftsanordnung: Name | Basisklasse für Symbolreferenzen Modell-Ansichten | Einsprachiger Text | Nein | INSTANCE_USER_DEFINED_PROPERTYSET |
| 19021 | Darstellungsart / Auswertungstyp | Basisklasse für Symbolreferenzen Verbindungen Artikelreferenzen | Einsprachiger Text | Nein | INSTANCE_TYPE |
| 19409 | Lüftungstechnischer Sperrraum: X-Koordinate Startpunkt | Mechanikobjekte | Dezimalzahl | Nein | DMG_EXTERNALINSTANCE_VENTILATION_RESTRICTED_AREA_START_X |
| 19410 | Lüftungstechnischer Sperrraum: Y-Koordinate Startpunkt | Mechanikobjekte | Dezimalzahl | Nein | DMG_EXTERNALINSTANCE_VENTILATION_RESTRICTED_AREA_START_Y |
| 19411 | Lüftungstechnischer Sperrraum: Z-Koordinate Startpunkt | Mechanikobjekte | Dezimalzahl | Nein | DMG_EXTERNALINSTANCE_VENTILATION_RESTRICTED_AREA_START_Z |
| 19412 | Lüftungstechnischer Sperrraum: X-Koordinate Endpunkt | Mechanikobjekte | Dezimalzahl | Nein | DMG_EXTERNALINSTANCE_VENTILATION_RESTRICTED_AREA_END_X |
| 19413 | Lüftungstechnischer Sperrraum: Y-Koordinate Endpunkt | Mechanikobjekte | Dezimalzahl | Nein | DMG_EXTERNALINSTANCE_VENTILATION_RESTRICTED_AREA_END_Y |
| 19414 | Lüftungstechnischer Sperrraum: Z-Koordinate Endpunkt | Mechanikobjekte | Dezimalzahl | Nein | DMG_EXTERNALINSTANCE_VENTILATION_RESTRICTED_AREA_END_Z |
| 19502 | Kommentartext | PDF-Kommentare | Mehrsprachiger Text | Nein | COMMENT_CONTENTS |
| 20007 | BMK (identifizierend, ohne untergeordnetes BMK) | Basisklasse für Funktionen | Einsprachiger Text | Nein | FUNC_IDENTDEVICETAGWITHOUTNESTEDDEVTAG |
| 20009 | BMK (vollständig, ohne Projektstrukturen) | Basisklasse für Funktionen Verbindungen Projekte Artikelreferenzen Segmente (Vorplanung) | Einsprachiger Text | Nein | FUNC_DEVICETAG_FULL |
| 20022 | Anschlussbezeichnung der Funktion | Anschlüsse Funktionen | Einsprachiger Text | Ja, 1000 | FUNC_CONNECTIONDESIGNATION |
| 20023 | Anschlussbeschreibung der Funktion | Anschlüsse Funktionen | Einsprachiger Text | Ja, 1000 | FUNC_CONNECTIONDESCRIPTION |
| 20028 | Anschlussbezeichnung | Anschlüsse Funktionen | Einsprachiger Text | Ja, 100 | FUNC_TERMINALDESIGNATION |
| 20029 | Anschlussbeschreibung | Anschlüsse Funktionen | Einsprachiger Text | Ja, 100 | FUNC_TERMINALDESCRIPTION |
| 20046 | Kabel / Paket: Länge | Funktionen | Dezimalzahl | Nein | FUNC_CABLELENGTH_VALUE |
| 20048 | Name des Zielanschlusses (vollständig) | Funktionen Verbindungen | Einsprachiger Text | Ja, 1000 | FUNC_FULLCONNECTIONDESTINATIONNAME |
| 20052 | Stecker: Zuordnung zu Stift- / Buchsenseite | Funktionsschablonen Funktionen | Ganzzahlig | Nein | FUNC_PINANDJACK_PLUGSIDE |
| 20058 | Name (ohne Projektstrukturen) | Artikelreferenzen Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DEVICETAG_FULLNAME |
| 20065 | Kabel / Paket: Längeneinheit | Funktionen | Ganzzahlig | Nein | FUNC_CABLELENGTH_UNIT |
| 20075 | Anschlussbezeichnung 1 | Funktionen | Einsprachiger Text | Nein | FUNC_CONNECTIONDESIGNATION1 |
| 20076 | Anschlussbezeichnung 2 | Funktionen | Einsprachiger Text | Nein | FUNC_CONNECTIONDESIGNATION2 |
| 20077 | Name des Zielanschlusses | Funktionen Verbindungen | Einsprachiger Text | Ja, 1000 | FUNC_CONNECTIONDESTINATIONNAME |
| 20088 | Funktionsdefinition: Bereich | Funktionen Symbole Verbindungen | Mehrsprachiger Text | Nein | FUNC_CATEGORY_REGION |
| 20090 | Zusatzfeld für CLIP PROJECT-Daten | Funktionen | Einsprachiger Text | Ja, 1000 | FUNC_EXTERNAL_CLIPPROJECT |
| 20095 | Identifizierende Namensbestandteile | Basisklasse für Funktionen Verbindungen | Einsprachiger Text | Nein | FUNC_IDENTNAMEPARTS |
| 20096 | Identifizierende BMK-Bestandteile | Basisklasse für Funktionen Verbindungen | Einsprachiger Text | Nein | FUNC_IDENTDEVICETAGPARTS |
| 20098 | Objekt-ID der Kommunikationseinheit | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_OBJECTID |
| 20106 | Kommunikationseinheit (an Kommunikationseinheiten) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_NAME |
| 20107 | Konfigurationsprojekt (an Kommunikationseinheiten) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_CONFIGURATIONPROJECT |
| 20109 | Bus-System (nicht mehr verwenden) | Funktionen | Ganzzahlig | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_BUSSYSTEM |
| 20118 | Subnetzmaske (nicht mehr verwenden) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUBNETMASK |
| 20119 | Standardgateway | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_STANDARDGATEWAY |
| 20128 | MAC-Adresse | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_MACADDRESS |
| 20129 | DNS/PROFINET Gerätename (nicht mehr verwenden) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_DNS |
| 20130 | Teilnehmergruppe (Interbus) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_PARTICIPANTGROUP |
| 20131 | Alternative (Interbus) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_ALTERNATIVE |
| 20132 | Service-Info (Interbus) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SERVICEINFO |
| 20133 | Strang / Netz (nicht mehr verwenden) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_DP_SYSTEM |
| 20134 | Adressbereich (nicht mehr verwenden) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_ADDRESSRANGE |
| 20135 | Startadresse der SPS-Karte (nicht mehr verwenden) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_GROUP_STARTADDRESS |
| 20136 | Bus-Master / Bus-Koppler (nicht mehr verwenden) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_THISMASTER |
| 20137 | Hängt an Bus-Master / Bus-Koppler (nicht mehr verwenden) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_MASTER |
| 20138 | An Position / Bus-Adresse (nicht mehr verwenden) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_BUS_ADDRESS |
| 20139 | SPS-Objekt-ID (an Kommunikationseinheiten) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_PLCOBJECTID |
| 20140 | Zusatzfeld 1 | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUPPLEMENTARYFIELD01 |
| 20141 | Zusatzfeld 2 | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUPPLEMENTARYFIELD02 |
| 20142 | Zusatzfeld 3 | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUPPLEMENTARYFIELD03 |
| 20143 | Zusatzfeld 4 | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUPPLEMENTARYFIELD04 |
| 20144 | Zusatzfeld 5 | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUPPLEMENTARYFIELD05 |
| 20145 | Zusatzfeld 6 | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUPPLEMENTARYFIELD06 |
| 20146 | Zusatzfeld 7 | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUPPLEMENTARYFIELD07 |
| 20147 | Zusatzfeld 8 | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUPPLEMENTARYFIELD08 |
| 20148 | Zusatzfeld 9 | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUPPLEMENTARYFIELD09 |
| 20149 | Zusatzfeld 10 | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUPPLEMENTARYFIELD10 |
| 20150 | BMK: Seite | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_PAGECOUNTER |
| 20151 | BMK: Unterseite | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_PAGESUBCOUNTER |
| 20152 | BMK: Spalte | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_COLUMN |
| 20153 | BMK: Zeile | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_ROW |
| 20154 | BMK: Abschnitt | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_SECTION |
| 20155 | BMK: Anwendung | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_FUNCTIONCODE |
| 20156 | BMK: Zusatzfeld 1 | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_SUPPLEMENTARYFIELD01 |
| 20157 | BMK: Zusatzfeld 2 | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_SUPPLEMENTARYFIELD02 |
| 20158 | BMK: Zusatzfeld 3 | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_SUPPLEMENTARYFIELD03 |
| 20159 | BMK: Zusatzfeld 4 | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_SUPPLEMENTARYFIELD04 |
| 20160 | BMK: Zusatzfeld 5 | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT_SUPPLEMENTARYFIELD05 |
| 20163 | Kommunikationseinheit (an SPS-Anschlüssen) | Funktionen SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | FUNC_PLCBUSTERMCOMMENTITY_NAME |
| 20165 | Start IP-Adresse | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_STARTIPADDRESS |
| 20169 | End IP-Adresse | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_ENDIPADDRESS |
| 20170 | BMK (untergeordnet): Seite | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_PAGECOUNTER |
| 20171 | BMK (untergeordnet): Unterseite | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_PAGESUBCOUNTER |
| 20172 | BMK (untergeordnet): Spalte | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_COLUMN |
| 20173 | BMK (untergeordnet): Zeile | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_ROW |
| 20174 | BMK (untergeordnet): Abschnitt | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_SECTION |
| 20175 | BMK (untergeordnet): Anwendung | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_FUNCTIONCODE |
| 20176 | BMK (untergeordnet): Zusatzfeld 1 | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_SUPPLEMENTARYFIELD01 |
| 20177 | BMK (untergeordnet): Zusatzfeld 2 | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_SUPPLEMENTARYFIELD02 |
| 20178 | BMK (untergeordnet): Zusatzfeld 3 | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_SUPPLEMENTARYFIELD03 |
| 20179 | BMK (untergeordnet): Zusatzfeld 4 | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_SUPPLEMENTARYFIELD04 |
| 20180 | BMK (untergeordnet): Zusatzfeld 5 | Basisklasse für Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DT2_SUPPLEMENTARYFIELD05 |
| 20181 | Domain Postfix | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_DOMAINPOSTFIX |
| 20182 | DHCP verwenden | Funktionen | Bool (Wahr / Falsch) | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_USEDHCP |
| 20204 | Untersteckplatz (nicht mehr verwenden) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_SUBSLOT |
| 20213 | BMK (vollständig, ohne Projektstrukturen, mit Vorzeichen) | Basisklasse für Funktionen Verbindungen Projekte Artikelreferenzen | Einsprachiger Text | Nein | FUNC_DEVICETAG_FULL_WITHSEPARATOR |
| 20214 | Name (ohne Projektstrukturen, mit Vorzeichen) | Artikelreferenzen Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DEVICETAG_FULLNAME_WITHSEPARATOR |
| 20218 | ID für netzbasierte Verbindungen | Funktionen Potenzialdefinitionspunkte | Einsprachiger Text | Ja, 100 | FUNCTION_NETBASEDWIRING_ID |
| 20226 | Anschlussbezeichnung 3 | Funktionen | Einsprachiger Text | Nein | FUNC_CONNECTIONDESIGNATION3 |
| 20227 | Anschlussbezeichnung 4 | Funktionen | Einsprachiger Text | Nein | FUNC_CONNECTIONDESIGNATION4 |
| 20254 | Beschreibung | Funktionsschablonen | Mehrsprachiger Text | Nein | FUNCTEMPLATE_DESCRIPTION |
| 20282 | Datenübertragungsrate (nicht mehr verwenden) | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCCOMMUNICATIONENTITY_TRANSMISSIONRATE |
| 20295 | Anschlussquerschnitt / -durchmesser | Anschlüsse Funktionen | Einsprachiger Text | Ja, 1000 | FUNC_CONNECTIONCROSSSECTIONS |
| 20303 | Kontaktspiegeldarstellung | Funktionen | Ganzzahlig | Nein | FUNC_CONTACTIMAGE |
| 20335 | Übergeordnetes Produktkennzeichen inkl. Namenszusatz | Artikelreferenzen Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DEVICETAG_MAINNAME |
| 20336 | Untergeordnetes Produktkennzeichen inkl. Namenszusatz | Artikelreferenzen Funktionen Verbindungen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | FUNC_DEVICETAG_NESTEDNAME |
| 20339 | Anfasspunkt aus Makro verwenden | Artikelreferenzen Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_ARTICLEPLACEMENT_USEGRIPPERFROMMACRO |
| 20340 | Anfasspunkt aus Makro: X-Koordinate | Artikelreferenzen Funktionen | Koordinaten (nur X oder Y) | Nein | FUNC_ARTICLEPLACEMENT_GRIPPERFROMMACRO_X |
| 20341 | Anfasspunkt aus Makro: Y-Koordinate | Artikelreferenzen Funktionen | Koordinaten (nur X oder Y) | Nein | FUNC_ARTICLEPLACEMENT_GRIPPERFROMMACRO_Y |
| 20374 | Anschlussmaß | Funktionen Funktionsschablonen Anschlüsse | Einsprachiger Text | Ja, 1000 | FUNC_CONNECTIONDIMENSIONS |
| 20436 | Kanalbezeichnung (ermittelt) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCCHANNEL_DESIGNATION_CALCULATED |
| 20477 | Alle Platzierungen | Funktionen | Einsprachiger Text | Ja, 50 | FUNC_ALLPLACEMENTLOCATIONS |
| 20499 | Gesamtmenge (Stückzahl) | Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLEREF_COUNT_TOTAL |
| 20500 | Kabellängensumme | Artikelreferenzen (Darstellungsarten zusammengefasst) | Dezimalzahl | Nein | ARTICLEREF_CABLE_LENGTH_SUM |
| 20503 | Gesamteinkaufspreis Währung 1 | Artikelreferenzen (Darstellungsarten zusammengefasst) | Dezimalzahl | Nein | ARTICLEREF_TOTALPURCHASEPRICE_1 |
| 20504 | Gesamteinkaufspreis Währung 2 | Artikelreferenzen (Darstellungsarten zusammengefasst) | Dezimalzahl | Nein | ARTICLEREF_TOTALPURCHASEPRICE_2 |
| 20513 | Gesamtlänge mit Einheit des Projekts | Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Nein | ARTICLEREF_LENGTH_SUM |
| 20514 | Gesamtanzahl Verpackungen | Artikelreferenzen (Darstellungsarten zusammengefasst) | Ganzzahlig | Nein | ARTICLEREF_NUMBEROFPACKAGES |
| 20858 | Kennung für Auswertungen | Funktionen | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_REPORT_IDENTIFIER |
| 20922 | Baugruppenstruktur | Funktionen Verbindungen Verbindungsdefinitionspunkte Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Einsprachiger Text | Ja, 50 | FUNC_ARTICLE_ASSEMBLY_STRUCTURE |
| 21000 | Anschlussbezeichnungen | Funktionsschablonen | Einsprachiger Text | Ja, 1000 | FUNCTEMPLATE_CONNPT_DESIGNATION |
| 21002 | Index für zusätzliche Daten | Funktionsschablonen | Einsprachiger Text | Ja, 2 | FUNCTEMPLATE_INDEX |
| 21005 | Untergeordnetes BMK / BMK-ID | Funktionsschablonen | Einsprachiger Text | Nein | FUNCTEMPLATE_NESTEDDEVICETAG |
| 21007 | Anschlussbeschreibungen | Funktionsschablonen | Einsprachiger Text | Ja, 1000 | FUNCTEMPLATE_CONNPT_DESCRIPTION |
| 21021 | Anschlussquerschnitt / -durchmesser | Funktionsschablonen | Einsprachiger Text | Ja, 1000 | FUNCTEMPLATE_CONNPT_CROSSSECTION |
| 21023 | Schablonenverbund (allpolig) | Funktionsschablonen | Einsprachiger Text | Nein | FUNCTEMPLATE_COMBINATION |
| 22011 | Kennbuchstabe | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_CODELETTER |
| 22149 | Externes Dokument 1 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_1 |
| 22150 | Externes Dokument 2 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_2 |
| 22151 | Externes Dokument 3 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_3 |
| 22212 | API Parts Management Extension: Name des Add-ins | Artikel (eingelagert) Adressangaben bei Artikeln Zubehörplatzierungen Zubehörlisten Anschlussbilder Bohrbilder | Einsprachiger Text | Ja, 100 | ARTICLE_CUSTOM_DATA_INDEX |
| 22213 | API Parts Management Extension: Wert aus Add-in | Artikel (eingelagert) Adressangaben bei Artikeln Zubehörplatzierungen Zubehörlisten Anschlussbilder Bohrbilder | Einsprachiger Text | Ja, 100 | ARTICLE_CUSTOM_DATA_VALUE |
| 22235 | Externes Dokument 4 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_4 |
| 22236 | Externes Dokument 5 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_5 |
| 22237 | Externes Dokument 6 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_6 |
| 22238 | Externes Dokument 7 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_7 |
| 22239 | Externes Dokument 8 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_8 |
| 22240 | Externes Dokument 9 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_9 |
| 22241 | Externes Dokument 10 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_10 |
| 22242 | Externes Dokument 11 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_11 |
| 22243 | Externes Dokument 12 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_12 |
| 22244 | Externes Dokument 13 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_13 |
| 22245 | Externes Dokument 14 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_14 |
| 22246 | Externes Dokument 15 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_15 |
| 22247 | Externes Dokument 16 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_16 |
| 22248 | Externes Dokument 17 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_17 |
| 22249 | Externes Dokument 18 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_18 |
| 22250 | Externes Dokument 19 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_19 |
| 22251 | Externes Dokument 20 | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_EXTERNAL_DOCUMENT_20 |
| 22336 | Benutzerdefinierte Eigenschaften: Identifizierender Name | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Einsprachiger Text | Ja, 1000 | ARTICLE_FREE_DATA_IDENTNAME |
| 22337 | Benutzerdefinierte Eigenschaften: Wert | Artikel (eingelagert) Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Mehrsprachiger Text | Ja, 1000 | ARTICLE_FREE_DATA_NEWVALUE |
| 23300 | Kommunikationseinheit | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_NAME |
| 23301 | Konfigurationsprojekt | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_CONFIGURATIONPROJECT |
| 23302 | End IP-Adresse | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_ENDIPADDRESS |
| 23304 | Bus-System | SPS-Kommunikationseinheiten | Ganzzahlig | Nein | COMMUNICATIONENTITY_BUSSYSTEM |
| 23305 | Subnetzmaske (nicht mehr verwenden) | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUBNETMASK |
| 23306 | Standardgateway | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_STANDARDGATEWAY |
| 23307 | MAC-Adresse | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_MACADDRESS |
| 23308 | DNS/PROFINET Gerätename | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_DNS |
| 23309 | Teilnehmergruppe (Interbus) | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_PARTICIPANTGROUP |
| 23310 | Alternative (Interbus) | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_ALTERNATIVE |
| 23311 | Service-Info (Interbus) | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SERVICEINFO |
| 23312 | Strang / Netz (nicht mehr verwenden) | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_DP_SYSTEM |
| 23313 | Adressbereich | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_ADDRESSRANGE |
| 23314 | Startadresse der SPS-Karte | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_GROUP_STARTADDRESS |
| 23315 | Bus-Master / Bus-Koppler (nicht mehr verwenden) | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_THISMASTER |
| 23316 | Hängt an Bus-Master / Bus-Koppler (nicht mehr verwenden) | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_MASTER |
| 23317 | An Position / Bus-Adresse (nicht mehr verwenden) | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_BUS_ADDRESS |
| 23318 | Objekt-ID der Kommunikationseinheit | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_OBJECTID |
| 23319 | Start IP-Adresse | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_STARTIPADDRESS |
| 23320 | Domain Postfix | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_DOMAINPOSTFIX |
| 23321 | DHCP verwenden | SPS-Kommunikationseinheiten | Bool (Wahr / Falsch) | Nein | COMMUNICATIONENTITY_USEDHCP |
| 23322 | Untersteckplatz | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUBSLOT |
| 23323 | Datenübertragungsrate | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_TRANSMISSIONRATE |
| 23330 | Zusatzfeld 1 | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUPPLEMENTARYFIELD01 |
| 23331 | Zusatzfeld 2 | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUPPLEMENTARYFIELD02 |
| 23332 | Zusatzfeld 3 | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUPPLEMENTARYFIELD03 |
| 23333 | Zusatzfeld 4 | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUPPLEMENTARYFIELD04 |
| 23334 | Zusatzfeld 5 | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUPPLEMENTARYFIELD05 |
| 23335 | Zusatzfeld 6 | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUPPLEMENTARYFIELD06 |
| 23336 | Zusatzfeld 7 | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUPPLEMENTARYFIELD07 |
| 23337 | Zusatzfeld 8 | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUPPLEMENTARYFIELD08 |
| 23338 | Zusatzfeld 9 | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUPPLEMENTARYFIELD09 |
| 23339 | Zusatzfeld 10 | SPS-Kommunikationseinheiten | Einsprachiger Text | Nein | COMMUNICATIONENTITY_SUPPLEMENTARYFIELD10 |
| 23400 | Name der Kommunikationseinheit | SPS-Adressen | Einsprachiger Text | Nein | PLCIOENTRY_COMMUNICATIONENTITY_NAME |
| 23403 | Richtung | SPS-Adressen SPS-Adressen (in Auswertungen) | Ganzzahlig | Nein | PLCIOENTRY_DIRECTION |
| 23404 | Vorplanung | SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | PLCIOENTRYCOMPOUND_RELATEDPLANNINGOBJECT_DESIGNATION |
| 24000 | Ziel der Abbruchstelle | Abbruchstellen | Einsprachiger Text | Nein | INTERRUPTIONPOINT_DESTINATION |
| 24001 | Automatisch erzeugt | Abbruchstellen | Bool (Wahr / Falsch) | Nein | INTERRUPTIONPOINT_AUTO_CREATED |
| 25000 | Gewerk (Bearbeitungsbereiche) | Verbindungen Seiten Funktionen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) Segmente (Vorplanung) | Ganzzahlig | Nein | EDITINGAREA_CRAFT |
| 31000 | Verbindung: Länge | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Dezimalzahl | Nein | CONNECTION_WIRELENGTH_VALUE |
| 31001 | Verbindung: Längeneinheit | Verbindungen Verbindungsdefinitionspunkte Potenzialdefinitionspunkte | Ganzzahlig | Nein | CONNECTION_WIRELENGTH_UNIT |
| 31010 | Teilmenge / Länge: Wert | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Dezimalzahl | Ja, 50 | FUNC_ARTICLE_PARTIAL_LENGTH_VALUE |
| 31012 | Einheit für Teilmenge / Länge | Verbindungen Verbindungsdefinitionspunkte Funktionen Projekte Segmentvorlagen (Vorplanung) Segmente (Vorplanung) | Ganzzahlig | Ja, 50 | FUNC_ARTICLE_PARTIAL_LENGTH_UNIT |
| 31026 | Name der Seite, auf der die Verbindung liegt | Verbindungen Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CONNECTION_PAGENAME |
| 31037 | Zugehörige Verbindungen: Linienstärke Autoconnect-Linie | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_ACL_WIDTH |
| 31038 | Zugehörige Verbindungen: Farbe Autoconnect-Linie | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_ACL_COLOR |
| 31039 | Zugehörige Verbindungen: Linienart Autoconnect-Linie | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_ACL_STYLE |
| 31041 | Zugehörige Verbindungen: Musterlänge Autoconnect-Linie | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_ACL_FACTOR |
| 31042 | Zugehörige Verbindungen: Ebene Autoconnect-Linie | Verbindungsdefinitionspunkte | Einsprachiger Text | Nein | CDP_CON_ACL_LAYER |
| 31058 | Kabelverbindung | Verbindungen Verbindungsdefinitionspunkte | Bool (Wahr / Falsch) | Nein | CONNECTION_IS_CABLE |
| 31075 | Verwaltungsart | Verbindungen | Ganzzahlig | Nein | CONNECTION_TYPE |
| 31076 | Netzindex | Verbindungen | Einsprachiger Text | Nein | CONNECTION_NET_INDEX |
| 31077 | Verbindungskettenindex | Verbindungen | Einsprachiger Text | Nein | CONNECTION_DAISYCHAIN_INDEX |
| 31078 | Verbindungsindex | Verbindungen | Einsprachiger Text | Nein | CONNECTION_CONNECTION_INDEX |
| 31086 | Paketverbindung | Verbindungen Verbindungsdefinitionspunkte | Bool (Wahr / Falsch) | Nein | CONNECTION_IS_PACKET |
| 31118 | Verbindung: Zugehöriger Klemmenanschluss (Anschlussbezeichnung) | Verbindungen | Einsprachiger Text | Nein | CONNECTION_TERMINAL_CONNECTIONDESIGNATION |
| 31135 | Phasenschienenverbindung | Verbindungen Verbindungsdefinitionspunkte | Bool (Wahr / Falsch) | Nein | CONNECTION_IS_PHASEBAR |
| 31146 | Leitung | Verbindungen Verbindungsdefinitionspunkte | Bool (Wahr / Falsch) | Nein | CONNECTION_IS_PIPE |
| 31160 | Objektidentifikation: Quelle | Verbindungen | Einsprachiger Text | Nein | CONNECTION_SOURCE_DBOBJECTID |
| 31161 | Objektidentifikation: Ziel | Verbindungen | Einsprachiger Text | Nein | CONNECTION_DESTINATION_DBOBJECTID |
| 35012 | Klemmenleisten: Anzahl Hauptklemmen | Klemmenleisten (in Auswertungen) | Ganzzahlig | Nein | TERMINALSTRIP_COUNTOFTERMINALMAINS |
| 35566 | Klemmenbezeichnung | Anschlüsse (in Anschlussbildern) | Einsprachiger Text | Nein | ARTICLETERMINALPOSITION_TERMINALNUMBER |
| 36033 | Übergeordnete Montagefläche | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_PARENT_OBJECTID |
| 36040 | Übergeordnetes Betriebsmittel | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_PARENT_OBJECT |
| 36056 | Anschlussbild: X-Position | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_XPOS |
| 36057 | Anschlussbild: Y-Position | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_YPOS |
| 36058 | Anschlussbild: Z-Position | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_ZPOS |
| 36059 | Anschlussbild: Verlegerichtung | 3D-Platzierungen | Ganzzahlig | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_DIRECTION |
| 36060 | Anschlussbild: Zusatzlänge | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_ADDITIONALLENGTH |
| 36061 | Anschlussbild: Verbindungsende-Behandlung (EPLAN Cabinet) | 3D-Platzierungen | Ganzzahlig | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_CONTACTTYPE |
| 36062 | Anschlussbild: Min. Querschnitt | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_MINCROSSSECTION |
| 36063 | Anschlussbild: Max. Querschnitt | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_MAXCROSSSECTION |
| 36064 | Anschlussbild: Max. Anzahl Verbindungen | 3D-Platzierungen | Ganzzahlig | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_MAXWIRECOUNT |
| 36065 | Anschlussbild: Doppelhülse vorgeschrieben | 3D-Platzierungen | Bool (Wahr / Falsch) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_TWINSLEEVE |
| 36066 | Anschlussbild: Anschlussausführung | 3D-Platzierungen | Ganzzahlig | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_TYPEOFTERMINAL |
| 36067 | Anschlussbild: Anschlussmaß | 3D-Platzierungen | Einsprachiger Text | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_TERMINALSIZE |
| 36068 | Anschlussbild: Steckerbezeichnung | 3D-Platzierungen | Einsprachiger Text | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_DEVICETAG |
| 36069 | Anschlussbild: Etage | 3D-Platzierungen | Ganzzahlig | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_LEVEL |
| 36070 | Anschlussbild: Intern- / Extern-Index | 3D-Platzierungen | Ganzzahlig | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_TARGETINFO |
| 36071 | Anschlussbild: Klemmenbezeichnung | 3D-Platzierungen | Einsprachiger Text | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_TERMINALNUMBER |
| 36072 | Anschlussbild: X-Vektor | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_XDIR |
| 36073 | Anschlussbild: Y-Vektor | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_YDIR |
| 36074 | Anschlussbild: Z-Vektor | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_ZDIR |
| 36077 | Anschlussbild: Min. AWG | 3D-Platzierungen | Einsprachiger Text | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_MINCROSSSECTION_AWG |
| 36078 | Anschlussbild: Max. AWG | 3D-Platzierungen | Einsprachiger Text | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_MAXCROSSSECTION_AWG |
| 36079 | Max. Anzugsdrehmoment | 3D-Platzierungen | Dezimalzahl | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_MAXTIGHTENINGTORQUE |
| 36080 | X-Startpunkt Klemmraum | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_CLAMPSPACE_STARTXPOS |
| 36081 | Y-Startpunkt Klemmraum | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_CLAMPSPACE_STARTYPOS |
| 36082 | Z-Startpunkt Klemmraum | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_CLAMPSPACE_STARTZPOS |
| 36083 | X-Endpunkt Klemmraum | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_CLAMPSPACE_ENDXPOS |
| 36084 | Y-Endpunkt Klemmraum | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_CLAMPSPACE_ENDYPOS |
| 36085 | Z-Endpunkt Klemmraum | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_CLAMPSPACE_ENDZPOS |
| 36086 | Klemmraumversatz | 3D-Platzierungen | Dezimalzahl | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_CLAMPSPACE_OFFSET |
| 36087 | Schaftdurchmesser | 3D-Platzierungen | Dezimalzahl | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_HOLEDIAMETER |
| 36088 | X-Position Werkzeug | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_OPERATIONVEC_XPOS |
| 36089 | Y-Position Werkzeug | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_OPERATIONVEC_YPOS |
| 36090 | Z-Position Werkzeug | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_OPERATIONVEC_ZPOS |
| 36091 | X-Vektor Werkzeug | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_OPERATIONVEC_XDIR |
| 36092 | Y-Vektor Werkzeug | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_OPERATIONVEC_YDIR |
| 36093 | Z-Vektor Werkzeug | 3D-Platzierungen | Koordinaten (nur X oder Y) | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_OPERATIONVEC_ZDIR |
| 36094 | Abtriebsgröße | 3D-Platzierungen | Einsprachiger Text | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_SCREWDRIVES |
| 36095 | Min. Anzugsdrehmoment | 3D-Platzierungen | Dezimalzahl | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_MINTIGHTENINGTORQUE |
| 36096 | Abisolierlänge | 3D-Platzierungen | Dezimalzahl | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_STRIPPINGLENGTH |
| 36101 | Anschlussbild: Bus-Schnittstellenname | 3D-Platzierungen | Einsprachiger Text | Ja, 1000 | FUNCTION3D_TERMINALPOSITION_PLC_BUS_INTERFACENAME |
| 36450 | Rastergröße | Bauräume | Koordinaten (nur X oder Y) | Nein | INSTALLATIONSPACE_GRIDSIZE |
| 36501 | Modell-Ansicht: Manuelle Verschiebung | Modell-Ansichten | Koordinaten (X/Y) | Nein | DMG_VIEWPLACEMENT_MANUAL_CENTER |
| 36512 | Modell-Ansicht: Name der Auswertungsvorlage | Modell-Ansichten | Einsprachiger Text | Nein | DMG_VIEWPLACEMENT_TEMPLATE_NAME |
| 36610 | Zugehörige Projektrevision | Löschzeichen | Einsprachiger Text | Nein | DMDELETEDOBJECTINFO_PROJECTREVISION |
| 36611 | Zugehörige Projektrevision (weitere) | Löschzeichen | Einsprachiger Text | Ja, 50 | DMDELETEDOBJECTINFO_PROJECTREVISION_HIST |
| 44001 | Segmentdefinition: Name (identifizierend) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) Rohrklassen Stoffe | Einsprachiger Text | Nein | DMPLAOBJECT_DEFINITIONNAME |
| 44011 | Gesamtenergiebedarf | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_POWER_REQUIREMENT_TOTAL |
| 44013 | Gesamtaufwand Planung | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_PLANNINGTIME_TOTAL |
| 44015 | Gesamtaufwand Bau | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_BUILDINGTIME_TOTAL |
| 44017 | Gesamtaufwand Software | Segmente (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_PROGRAMMINGTIME_TOTAL |
| 44019 | Gesamtkalkulationswert | Segmente (Vorplanung) | Dezimalzahl | Nein | DMPLAOBJECT_PRICE_TOTAL |
| 44039 | Basissegmentdefinition: Name (identifizierend) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) Rohrklassen Stoffe | Einsprachiger Text | Nein | DMPLAOBJECT_BASEDEFINITIONNAME |

## Kunde

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 10100 | Kunde: Kurzname | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERID |
| 10101 | Kunde: Anrede | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERTITLE |
| 10102 | Kunde: Name 1 | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERNAME1 |
| 10103 | Kunde: Name 2 | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERNAME2 |
| 10104 | Kunde: Name 3 | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERNAME3 |
| 10105 | Kunde: Straße | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERSTREET |
| 10106 | Kunde: Postfach | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERPOBOX |
| 10107 | Kunde: PLZ (Wohnort) | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERZIPCODE |
| 10108 | Kunde: Wohnort | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERCITY |
| 10109 | Kunde: Land | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERCOUNTRY |
| 10110 | Kunde: Telefon | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERTELEPHONE |
| 10111 | Kunde: Telefax | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERFAX |
| 10112 | Kunde: E-Mail | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMEREMAIL |
| 10113 | Kunde: PLZ (Postfach) | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERZIPCODEPOBOX |
| 10114 | Kunde: Staat / Region | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERREGION |
| 10115 | Kunde: Langname | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERLONGNAME |
| 10116 | Kunde: Kundennummer | Projekte | Einsprachiger Text | Nein | PROJ_CUSTOMERNUMBER |
| 10117 | Kunde: Beschreibung | Projekte | Mehrsprachiger Text | Nein | PROJ_CUSTOMERNOTE |

## Makro

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 11008 | Makro: Name | Seiten Makrokästen Bauräume | Einsprachiger Text | Ja, 100 | PAGE_MACRO_NAME |
| 11014 | Makro: Beschreibung | Seiten Makrokästen Bauräume | Mehrsprachiger Text | Ja, 100 | PAGE_MACRO_DESCRIPTION |
| 11018 | Makro: Name | Bauräume | Einsprachiger Text | Nein | INSTALLATIONSPACE_MACRO_NAME |
| 11019 | Makro: Beschreibung | Bauräume | Mehrsprachiger Text | Nein | INSTALLATIONSPACE_MACRO_DESCRIPTION |
| 11025 | Makro: Version | Bauräume | Einsprachiger Text | Nein | INSTALLATIONSPACE_MACRO_VERSION |
| 11026 | Makro: Quelle / Referenz | Bauräume | Einsprachiger Text | Nein | INSTALLATIONSPACE_MACROSOURCEREF |
| 11034 | Makro: Quelle / Referenz | Seiten | Einsprachiger Text | Nein | PAGE_MACROSOURCEREF |
| 11056 | Makro: Quellprojekt | Seiten | Einsprachiger Text | Nein | PAGE_MACROSOURCEPROJECT |
| 11057 | Makro: Beschreibung | Seiten | Mehrsprachiger Text | Nein | PAGE_MACRODESCRIPTION |
| 11910 | Makro: Name | Seiten | Einsprachiger Text | Nein | PAGE_MACRO |
| 11911 | Makro: Version | Seiten | Einsprachiger Text | Nein | PAGE_MACRO_VERSION |
| 23001 | Makro: Dateiname | Makrokästen Makros Makrokästen (referenziert) Geschützte Gruppen | Einsprachiger Text | Nein | MACROBOX_MACRO |
| 23002 | Makro: Version | Makrokästen Makrokästen (referenziert) | Einsprachiger Text | Nein | MACROBOX_MACRO_VERSION |
| 23003 | Makro: Quelle / Referenz | Makrokästen Makrokästen (referenziert) | Einsprachiger Text | Nein | MACROBOX_MACRO_SOURCE |
| 23004 | Makro: Beschreibung | Makrokästen Makrokästen (referenziert) | Mehrsprachiger Text | Nein | MACROBOX_MACRO_DESCRIPTION |
| 23005 | Makro: Quellprojekt | Makrokästen Makrokästen (referenziert) | Einsprachiger Text | Nein | MACROBOX_SOURCEPROJECT |
| 23006 | Makro: Eingefügt durch | Makrokästen Seiten Makrokästen (referenziert) | Einsprachiger Text | Nein | MACROBOX_INSERTED_BY |
| 23007 | Makro: Darstellungsart | Makrokästen Makrokästen (referenziert) Geschützte Gruppen | Ganzzahlig | Nein | MACROBOX_REPRESENTATION_TYPE |
| 23008 | Makro: Variante | Makrokästen Makrokästen (referenziert) Geschützte Gruppen | Ganzzahlig | Nein | MACROBOX_VARIANT |
| 23010 | Makro: Typ | Makrokästen (referenziert) | Ganzzahlig | Nein | MACROBOX_TYPE |
| 23011 | Makro: Verwendungsart | Makrokästen (referenziert) Makrokästen | Ganzzahlig | Nein | MACROBOX_USAGE |
| 23012 | Makro: Geschützte Gruppe | Makrokästen Makrokästen (referenziert) | Bool (Wahr / Falsch) | Nein | MACROBOX_USEPROTECTEDGROUP |
| 23013 | Makro: Makrokasten mit einfügen | Makrokästen Makrokästen (referenziert) | Ganzzahlig | Nein | MACROBOX_INSERTMODE |
| 23014 | Makro: Anschlussbezeichnungen berücksichtigen | Makrokästen Makrokästen (referenziert) | Ganzzahlig | Nein | MACROBOX_USECONNECTIONPOINTDESIGNATION |
| 23501 | Bohrbild: Beschreibung | Bohrbildkästen | Mehrsprachiger Text | Nein | DRILLPATTERNBOX_DRILLPATTERN_DESCRIPTION |
| 36019 | Makro: Variante | Bauräume | Ganzzahlig | Nein | INSTALLATIONSPACE_MACRO_VARIANT |

## Meldungen

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 12099 | Anzahl Meldungen | Normblätter | Ganzzahlig | Nein | FRAME_ERRORCOUNT |
| 12100 | Fehler: Grafik | Normblätter | Einsprachiger Text | Ja, 1024 | FRAME_GRAPHICERROR |
| 12101 | Fehler: Eigenschaftsplatzierung | Normblätter | Einsprachiger Text | Ja, 1024 | FRAME_PLACEDPROPERROR |
| 12200 | Fehler (Import) | Normblätter | Einsprachiger Text | Ja, 1024 | FRAME_IMPORTERROR |
| 13099 | Anzahl Meldungen | Formulare | Ganzzahlig | Nein | FORM_ERRORCOUNT |
| 13100 | Fehler: Grafik | Formulare | Einsprachiger Text | Ja, 1024 | FORM_GRAPHICERROR |
| 13101 | Fehler: Eigenschaftsplatzierung | Formulare | Einsprachiger Text | Ja, 1024 | FORM_PLACEDPROPERROR |
| 13200 | Fehler (Import) | Formulare | Einsprachiger Text | Ja, 1024 | FORM_IMPORTERROR |
| 15099 | Anzahl Meldungen | Symbolbibliotheken | Ganzzahlig | Nein | SYMBLIB_ERRORCOUNT |
| 15100 | Fehler: Funktionsdefinition | Symbolbibliotheken | Einsprachiger Text | Ja, 1024 | SYMBLIB_FUNCDEFERROR |
| 15101 | Fehler: Anschluss | Symbolbibliotheken | Einsprachiger Text | Ja, 1024 | SYMBLIB_CONNECTIONERROR |
| 15102 | Fehler: Symbolvariante | Symbolbibliotheken | Einsprachiger Text | Ja, 1024 | SYMBLIB_VARIANTERROR |
| 15103 | Fehler: Darstellungsart | Symbolbibliotheken | Einsprachiger Text | Ja, 1024 | SYMBLIB_FUNCTYPEERROR |
| 15104 | Fehler: Anschlusslogik | Symbolbibliotheken | Einsprachiger Text | Ja, 1024 | SYMBLIB_LOGICERROR |
| 15105 | Fehler: Symbolgrafik | Symbolbibliotheken | Einsprachiger Text | Ja, 1024 | SYMBLIB_GRAPHICERROR |
| 15106 | Fehler: Eigenschaftsplatzierung | Symbolbibliotheken | Einsprachiger Text | Ja, 1024 | SYMBLIB_PLACEDPROPERROR |
| 15200 | Fehler (Import) | Symbolbibliotheken | Einsprachiger Text | Ja, 1024 | SYMBLIB_IMPORTERROR |
| 15201 | Fehler (Symboleditor) | Symbolbibliotheken | Einsprachiger Text | Ja, 1024 | SYMBLIB_ERRORSYMBOLED |

## Revision

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 3010 | Letzter Bearbeiter: Kennung | Projekte Seiten Basisklasse für Normblätter und Formulare Funktionsdefinitionsbibliotheken Symbole Symbolbibliotheken | Einsprachiger Text | Nein | PROPUSER_LAST_USERCODE |
| 3011 | Letzter Bearbeiter: Name | Projekte Seiten Basisklasse für Normblätter und Formulare Funktionsdefinitionsbibliotheken Symbole Symbolbibliotheken | Einsprachiger Text | Nein | PROPUSER_LAST_USERNAME |
| 3012 | Letzter Bearbeiter: Telefon | Projekte Seiten Basisklasse für Normblätter und Formulare Funktionsdefinitionsbibliotheken Symbole Symbolbibliotheken | Einsprachiger Text | Nein | PROPUSER_LAST_USERPHONE |
| 3013 | Letzter Bearbeiter: E-Mail | Projekte Seiten Basisklasse für Normblätter und Formulare Funktionsdefinitionsbibliotheken Symbole Symbolbibliotheken | Einsprachiger Text | Nein | PROPUSER_LAST_USEREMAIL |
| 10000 | Projektname | Projekte | Einsprachiger Text | Nein | PROJ_NAME |
| 10020 | Ersteller | Projekte | Einsprachiger Text | Nein | PROJ_CREATOR |
| 10021 | Erstellungsdatum | Projekte | Uhrzeit / Datum | Nein | PROJ_CREATIONDATE |
| 10022 | Letzter Bearbeiter: Anmeldename | Projekte | Einsprachiger Text | Nein | PROJ_LASTMODIFICATOR |
| 10023 | Änderungsdatum | Projekte | Uhrzeit / Datum | Nein | PROJ_LASTMODIFICATIONDATE |
| 10024 | Letzte Fremdsprachenübersetzung: Datum | Projekte | Uhrzeit / Datum | Nein | PROJ_LASTTRANSLATIONDATE |
| 10025 | Projektverantwortlicher | Projekte | Einsprachiger Text | Nein | PROJ_RESPONSIBLEFORPROJECT |
| 10026 | Projektende | Projekte | Uhrzeit / Datum | Nein | PROJ_PROJECTEND |
| 10027 | Tagesdatum | Projekte | Uhrzeit / Datum | Nein | PROJ_ACTUALDATE |
| 10028 | Projektanfang | Projekte | Uhrzeit / Datum | Nein | PROJ_PROJECTBEGIN |
| 10043 | Zuletzt verwendete EPLAN-Version | Projekte | Einsprachiger Text | Nein | PROJ_LASTUSEDVERSION |
| 10044 | Zuletzt verwendete EPLAN-Buildnummer | Projekte | Einsprachiger Text | Nein | PROJ_LASTUSEDBUILDNUMBER |
| 10046 | Erstellungszeit | Projekte | Einsprachiger Text | Nein | PROJ_CREATIONTIME |
| 10047 | Änderungszeit | Projekte | Einsprachiger Text | Nein | PROJ_LASTMODIFICATIONTIME |
| 10048 | Letzte Fremdsprachenübersetzung: Zeit | Projekte | Einsprachiger Text | Nein | PROJ_LASTTRANSLATIONTIME |
| 10058 | Tageszeit | Projekte | Uhrzeit / Datum | Nein | PROJ_ACTUALTIME |
| 10098 | Höchster Revisionsindex (aus Änderungsverfolgung) | Projekte | Einsprachiger Text | Nein | PROJECT_HIGHEST_REVISION_INDEX |
| 10149 | Revision: Ursprungsprojektname | Projekte | Einsprachiger Text | Nein | PROJ_REVISIONSPARENT_NAME |
| 10150 | Revisionen | Projekte | Einsprachiger Text | Ja, 1000 | PROJ_REVISIONS |
| 10151 | Revision: Ursprungsprojektname (vollständig) | Projekte | Einsprachiger Text | Nein | PROJ_REVISIONSPARENT |
| 10152 | Revisionsmarkierung (aus Eigenschaftenvergleich) | Projekte Seiten Platzierungen Verbindungen Segmente (Vorplanung) | Einsprachiger Text | Nein | INSTANCE_REVISIONMARKER |
| 10153 | Revisionsänderungsmarkierung (aus Eigenschaftenvergleich) | Projekte Platzierungen Seiten Verbindungen Segmente (Vorplanung) | Einsprachiger Text | Nein | INSTANCE_REVISIONID |
| 10155 | Revisionsname (aus Änderungsverfolgung) | Projekte | Einsprachiger Text | Ja, 1000 | PROJ_REVISION_LOG_NAME |
| 10156 | Revisionskommentar (aus Änderungsverfolgung) | Projekte | Mehrsprachiger Text | Ja, 1000 | PROJ_REVISION_LOG_DESCRIPTION |
| 10157 | Revisionsersteller (aus Änderungsverfolgung) | Projekte | Einsprachiger Text | Ja, 1000 | PROJ_REVISION_LOG_USER |
| 10158 | Revisionsdatum (aus Änderungsverfolgung) | Projekte | Uhrzeit / Datum | Ja, 1000 | PROJ_REVISION_LOG_DATE |
| 10159 | Revisionszeit (aus Änderungsverfolgung) | Projekte | Einsprachiger Text | Ja, 1000 | PROJ_REVISION_LOG_TIME |
| 10160 | Geprüft von | Projekte | Einsprachiger Text | Nein | PROJ_REVISION_APPROVEDBY |
| 10161 | Geprüft am | Projekte | Uhrzeit / Datum | Nein | PROJ_REVISION_APPROVEDDATE |
| 10162 | Kontrolliert von | Projekte | Einsprachiger Text | Nein | PROJ_REVISION_CHECKEDBY |
| 10163 | Kontrolliert am | Projekte | Uhrzeit / Datum | Nein | PROJ_REVISION_CHECKEDDATE |
| 10181 | Benutzer | Projekte | Einsprachiger Text | Nein | PROJ_IDENTIFICATION |
| 10182 | Station | Projekte | Einsprachiger Text | Nein | PROJ_STATION |
| 10185 | Lizenznummer des Dongles | Projekte Seiten | Einsprachiger Text | Nein | PROJ_SERIALNUMBERDONGLE |
| 10190 | Benutzerkennung (aus Änderungsverfolgung) | Projekte Seiten Bauräume | Einsprachiger Text | Ja, 1000 | PROJ_REVISION_LOG_USERCODE |
| 10191 | Benutzername (aus Änderungsverfolgung) | Projekte Seiten Bauräume | Einsprachiger Text | Ja, 1000 | PROJ_REVISION_LOG_USERNAME |
| 10192 | Benutzer: Telefon (aus Änderungsverfolgung) | Projekte Seiten Bauräume | Einsprachiger Text | Ja, 1000 | PROJ_REVISION_LOG_USERPHONE |
| 10193 | Benutzer: E-Mail (aus Änderungsverfolgung) | Projekte Seiten Bauräume | Einsprachiger Text | Ja, 1000 | PROJ_REVISION_LOG_USEREMAIL |
| 10195 | Bearbeitungsbereich (aus Änderungsverfolgung) | Projekte | Mehrsprachiger Text | Ja, 1000 | PROJ_REVISION_LOG_EDITINGAREA |
| 10230 | Ersteller: Kurzname | Projekte | Einsprachiger Text | Nein | PROJ_CREATORID |
| 10231 | Ersteller: Anrede | Projekte | Einsprachiger Text | Nein | PROJ_CREATORTITLE |
| 10232 | Ersteller: Name 1 | Projekte | Einsprachiger Text | Nein | PROJ_CREATORNAME1 |
| 10233 | Ersteller: Name 2 | Projekte | Einsprachiger Text | Nein | PROJ_CREATORNAME2 |
| 10234 | Ersteller: Name 3 | Projekte | Einsprachiger Text | Nein | PROJ_CREATORNAME3 |
| 10235 | Ersteller: Straße | Projekte | Einsprachiger Text | Nein | PROJ_CREATORSTREET |
| 10236 | Ersteller: Postfach | Projekte | Einsprachiger Text | Nein | PROJ_CREATORPOBOX |
| 10237 | Ersteller: PLZ (Wohnort) | Projekte | Einsprachiger Text | Nein | PROJ_CREATORZIPCODE |
| 10238 | Ersteller: Wohnort | Projekte | Einsprachiger Text | Nein | PROJ_CREATORCITY |
| 10239 | Ersteller: Land | Projekte | Einsprachiger Text | Nein | PROJ_CREATORCOUNTRY |
| 10240 | Ersteller: Telefon | Projekte | Einsprachiger Text | Nein | PROJ_CREATORTELEPHONE |
| 10241 | Ersteller: Telefax | Projekte | Einsprachiger Text | Nein | PROJ_CREATORFAX |
| 10242 | Ersteller: E-Mail | Projekte | Einsprachiger Text | Nein | PROJ_CREATOREMAIL |
| 10243 | Ersteller: PLZ (Postfach) | Projekte | Einsprachiger Text | Nein | PROJ_CREATORZIPCODEPOBOX |
| 10244 | Ersteller: Staat / Region | Projekte | Einsprachiger Text | Nein | PROJ_CREATORREGION |
| 10245 | Ersteller: Vollständiger Name | Projekte | Einsprachiger Text | Nein | PROJ_CREATORLONGNAME |
| 10246 | Ersteller: Kundennummer | Projekte | Einsprachiger Text | Nein | PROJ_CREATORNUMBER |
| 10247 | Ersteller: Beschreibung | Projekte | Mehrsprachiger Text | Nein | PROJ_CREATORNOTE |
| 10250 | Letztes Änderungsdatum pro Seitentyp | Projekte | Uhrzeit / Datum | Ja, 100 | PROJ_DATEPERPAGETYPE |
| 10251 | Letzte Änderungszeit pro Seitentyp | Projekte | Einsprachiger Text | Ja, 50 | PROJ_TIMEPERPAGETYPE |
| 11001 | Änderungszeit (automatisch) | Seiten | Einsprachiger Text | Nein | PAGE_LASTAUTOMODIFICATIONTIME |
| 11002 | Erstellungszeit | Seiten | Einsprachiger Text | Nein | PAGE_CREATIONTIME |
| 11003 | Wasserzeichen (Revision) | Seiten | Einsprachiger Text | Nein | PAGE_REVISION_WATERMARK |
| 11005 | Mindestens ein Objekt mit Revisionsmarkierung (aus Eigenschaftenvergleich) | Seiten | Bool (Wahr / Falsch) | Nein | PAGE_INSTANCE_WITH_REVISIONMARKER |
| 11007 | Mindestens ein Objekt oder Seite mit Revisionsmarkierung (aus Eigenschaftenvergleich) | Seiten | Bool (Wahr / Falsch) | Nein | PAGE_INSTANCE_OR_PAGE_WITH_REVISIONMARKER |
| 11020 | Ersteller | Seiten | Einsprachiger Text | Nein | PAGE_CREATOR |
| 11021 | Erstellungsdatum | Seiten | Uhrzeit / Datum | Nein | PAGE_CREATIONDATE |
| 11022 | Letzter Bearbeiter: Anmeldename | Seiten | Einsprachiger Text | Nein | PAGE_LASTMODIFICATOR |
| 11023 | Änderungsdatum (manuell) | Seiten | Uhrzeit / Datum | Nein | PAGE_LASTMANUMODIFICATIONDATE |
| 11024 | Änderungsdatum (automatisch) | Seiten | Uhrzeit / Datum | Nein | PAGE_LASTAUTOMODIFICATIONDATE |
| 11070 | Revisionsersteller (aus Änderungsverfolgung) | Seiten Bauräume | Einsprachiger Text | Ja, 1000 | PAGE_REVISION_LOG_USER |
| 11071 | Revisionsindex (aus Änderungsverfolgung) | Seiten Bauräume | Einsprachiger Text | Ja, 1000 | PAGE_REVISION_LOG_NAME |
| 11072 | Revisionsbeschreibung (aus Änderungsverfolgung) | Seiten Bauräume | Mehrsprachiger Text | Ja, 1000 | PAGE_REVISION_LOG_DESCRIPTION |
| 11073 | Revisionsänderungsgrund (aus Änderungsverfolgung) | Seiten Bauräume | Mehrsprachiger Text | Ja, 1000 | PAGE_REVISION_LOG_CHANGE |
| 11074 | Revisionsdatum (aus Änderungsverfolgung) | Seiten Bauräume | Uhrzeit / Datum | Ja, 1000 | PAGE_REVISION_LOG_DATE |
| 11076 | Seite in Entwurfsmodus | Seiten | Bool (Wahr / Falsch) | Nein | PAGE_REVISION_LOG_DRAFT |
| 11077 | Zugehöriger Revisionsname (aus Änderungsverfolgung) | Seiten Bauräume | Einsprachiger Text | Ja, 1000 | PAGE_REVISION_LOG_PROJ_REVNAME |
| 11078 | Revisionszeit (aus Änderungsverfolgung) | Seiten Bauräume | Einsprachiger Text | Ja, 1000 | PAGE_REVISION_LOG_TIME |
| 11079 | Bearbeitungsbereich (aus Änderungsverfolgung) | Seiten Bauräume | Mehrsprachiger Text | Ja, 1000 | PAGE_REVISION_LOG_EDITINGAREA |
| 11080 | Geprüft von | Seiten | Einsprachiger Text | Nein | PAGE_REVISION_APPROVEDBY |
| 11081 | Geprüft am | Seiten | Uhrzeit / Datum | Nein | PAGE_REVISION_APPROVEDDATE |
| 11082 | Kontrolliert von | Seiten | Einsprachiger Text | Nein | PAGE_REVISION_CHECKEDBY |
| 11083 | Kontrolliert am | Seiten | Uhrzeit / Datum | Nein | PAGE_REVISION_CHECKEDDATE |
| 11084 | Ursprung | Seiten | Einsprachiger Text | Nein | PAGE_REVISION_SOURCE |
| 11085 | Ersatz von | Seiten | Einsprachiger Text | Nein | PAGE_REVISION_SUBSTITUTIONOF |
| 11086 | Ersetzt durch | Seiten | Einsprachiger Text | Nein | PAGE_REVISION_REPLACEDBY |
| 11087 | Revision: Geprüft von (aus Änderungsverfolgung) | Seiten Bauräume | Einsprachiger Text | Ja, 1000 | PAGE_REVISION_LOG_APPROVEDBY |
| 11088 | Revision: Geprüft am (aus Änderungsverfolgung) | Seiten Bauräume | Uhrzeit / Datum | Ja, 1000 | PAGE_REVISION_LOG_APPROVEDDATE |
| 14020 | Ersteller | Basisklasse für Normblätter und Formulare | Einsprachiger Text | Nein | DMGDOC_CREATOR |
| 14021 | Erstellungsdatum | Basisklasse für Normblätter und Formulare | Uhrzeit / Datum | Nein | DMGDOC_CREATIONDATE |
| 14022 | Letzter Bearbeiter: Anmeldename | Basisklasse für Normblätter und Formulare | Einsprachiger Text | Nein | DMGDOC_LASTMODIFICATOR |
| 14023 | Änderungsdatum (automatisch) | Basisklasse für Normblätter und Formulare | Uhrzeit / Datum | Nein | DMGDOC_LASTMODIFICATIONDATE |
| 15012 | Version | Symbolbibliotheken | Einsprachiger Text | Nein | SYMBLIB_VERSION |
| 15019 | Firmenkennung | Symbolbibliotheken | Einsprachiger Text | Nein | SYMBLIB_COMPANY |
| 15020 | Ersteller | Symbolbibliotheken | Einsprachiger Text | Nein | SYMBLIB_CREATOR |
| 15021 | Erstellungsdatum | Symbolbibliotheken | Uhrzeit / Datum | Nein | SYMBLIB_CREATIONDATE |
| 15022 | Letzter Bearbeiter: Anmeldename | Symbolbibliotheken | Einsprachiger Text | Nein | SYMBLIB_LASTMODIFICATOR |
| 15023 | Änderungsdatum (automatisch) | Symbolbibliotheken | Uhrzeit / Datum | Nein | SYMBLIB_LASTMODIFICATIONDATE |
| 15024 | Änderungsdatum (manuell) | Symbolbibliotheken | Einsprachiger Text | Nein | SYMBLIB_MANUALDATE |
| 16020 | Ersteller | Symbole | Einsprachiger Text | Nein | SYMB_CREATOR |
| 16021 | Erstellungsdatum | Symbole | Uhrzeit / Datum | Nein | SYMB_CREATIONDATE |
| 16022 | Letzter Bearbeiter: Anmeldename | Symbole | Einsprachiger Text | Nein | SYMB_LASTMODIFICATOR |
| 16023 | Änderungsdatum (automatisch) | Symbole | Uhrzeit / Datum | Nein | SYMB_LASTAUTOMODIFICATIONDATE |
| 16024 | Änderungshinweis | Symbole | Mehrsprachiger Text | Nein | SYMB_MODIFICATIONCOMMENT |
| 16025 | Änderungsdatum (manuell) | Symbole | Einsprachiger Text | Nein | SYMB_LASTMANUMODIFICATIONDATE |
| 16026 | Version | Symbole | Einsprachiger Text | Nein | SYMB_VERSION |
| 18031 | Änderungsdatum (manuell) | Basisklasse für Normblätter und Formulare | Einsprachiger Text | Nein | FORMBASE_MANUALDATE |
| 18051 | Version | Basisklasse für Normblätter und Formulare | Einsprachiger Text | Nein | FORMBASE_VERSION |
| 19030 | Revisionsmarkierung (aus Änderungsverfolgung) | Platzierungen Projektoptionen Segmente (Vorplanung) | Einsprachiger Text | Nein | INSTANCE_REVISION_LOG_MARKER |
| 19031 | Revisionsmarkierungsformat (aus Änderungsverfolgung) | Platzierungen Projekte Projektoptionen Segmente (Vorplanung) | Einsprachiger Text | Nein | INSTANCE_REVISION_LOG_MARKER_FORMAT |
| 19032 | Änderungsdatum (aus Änderungsverfolgung) | Platzierungen Projektoptionen Segmente (Vorplanung) | Uhrzeit / Datum | Nein | INSTANCE_REVISION_LOG_DATE |
| 19033 | Ersteller (aus Änderungsverfolgung) | Platzierungen Projektoptionen Segmente (Vorplanung) | Einsprachiger Text | Nein | INSTANCE_REVISION_LOG_USER |
| 19034 | Änderungszeit (aus Änderungsverfolgung) | Platzierungen Projektoptionen Segmente (Vorplanung) | Einsprachiger Text | Nein | INSTANCE_REVISION_LOG_TIME |
| 20199 | Revision: Kontaktspiegeltext (für Eigenschaftenvergleich) | Funktionen | Einsprachiger Text | Nein | FUNC_CONTACTIMAGETEXT |
| 22901 | Letzter Bearbeiter / Änderungsdatum | Artikel (referenziert) Artikel (eingelagert) | Einsprachiger Text | Nein | PART_LASTCHANGE |
| 22937 | Letzter Bearbeiter / Änderungsdatum (Adresse) | Artikel (referenziert) Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_ADDRESS_LASTCHANGE |
| 22938 | Letzter Bearbeiter / Änderungsdatum (Bohrbild) | Artikel (referenziert) Bohrbilder | Einsprachiger Text | Nein | PART_CONSTRUCTION_LASTCHANGE |
| 22944 | Letzter Bearbeiter / Änderungsdatum (Anschlussbild) | Artikel (referenziert) Anschlussbilder | Einsprachiger Text | Nein | PART_TERMINAL_LASTCHANGE |
| 22958 | Letzter Bearbeiter / Änderungsdatum (Zubehörliste) | Artikel (referenziert) Zubehörlisten | Einsprachiger Text | Nein | PART_ACCESSORYLIST_LASTCHANGE |
| 22973 | Letzter Bearbeiter / Änderungsdatum (Zubehörplatzierung) | Artikel (referenziert) Zubehörplatzierungen | Einsprachiger Text | Nein | PART_ACCESSORYPLACEMENT_LASTCHANGE |
| 22980 | Letzter Bearbeiter | Artikel (referenziert) Artikel (eingelagert) Zubehörlisten Zubehörplatzierungen Bohrbilder Anschlussbilder Adressangaben bei Artikeln | Einsprachiger Text | Nein | PART_LASTCHANGE_USER |
| 22981 | Änderungsdatum | Artikel (referenziert) Artikel (eingelagert) | Uhrzeit / Datum | Nein | PART_LASTCHANGE_DATE |
| 36451 | Bauraum in Entwurfsmodus | Bauräume | Bool (Wahr / Falsch) | Nein | INSTALLATIONSPACE_REVISION_LOG_DRAFT |
| 36600 | Gelöschtes Objekt: Name | Löschzeichen | Einsprachiger Text | Nein | DMDELETEDOBJECTINFO_OBJECTNAME |
| 36601 | Gelöschtes Objekt: Typ | Löschzeichen | Einsprachiger Text | Nein | DMDELETEDOBJECTINFO_OBJECTTYPE |
| 36602 | Benutzername | Löschzeichen | Einsprachiger Text | Nein | DMDELETEDOBJECTINFO_USER |
| 36603 | Löschdatum | Löschzeichen | Uhrzeit / Datum | Nein | DMDELETEDOBJECTINFO_TIME |
| 36604 | Zugehöriger Revisionsname | Löschzeichen | Einsprachiger Text | Nein | DMDELETEDOBJECTINFO_PROJECTREVISION_NAME |
| 36605 | Gelöschtes Objekt (weitere): Name | Löschzeichen | Einsprachiger Text | Ja, 50 | DMDELETEDOBJECTINFO_OBJECTNAME_HIST |
| 36606 | Gelöschtes Objekt (weitere): Typ | Löschzeichen | Einsprachiger Text | Ja, 50 | DMDELETEDOBJECTINFO_OBJECTTYPE_HIST |
| 36607 | Benutzername (weitere) | Löschzeichen | Einsprachiger Text | Ja, 50 | DMDELETEDOBJECTINFO_USER_HIST |
| 36608 | Löschdatum (weitere) | Löschzeichen | Uhrzeit / Datum | Ja, 50 | DMDELETEDOBJECTINFO_TIME_HIST |
| 36609 | Zugehöriger Revisionsname (weitere) | Löschzeichen | Einsprachiger Text | Ja, 50 | DMDELETEDOBJECTINFO_PROJECTREVISION_NAME_HIST |
| 36614 | Gelöschtes Objekt: Revisionsindex | Löschzeichen | Einsprachiger Text | Nein | DMDELETEDOBJECTINFO_REVISION_LOG_NAME |
| 36615 | Gelöschtes Objekt: Revisionsbeschreibung (aus Änderungsverfolgung) | Löschzeichen | Mehrsprachiger Text | Nein | DMDELETEDOBJECTINFO_REVISION_LOG_DESCRIPTION |
| 36616 | Gelöschtes Objekt: Revisionsänderungsgrund (aus Änderungsverfolgung) | Löschzeichen | Mehrsprachiger Text | Nein | DMDELETEDOBJECTINFO_REVISION_LOG_CHANGE |
| 36617 | Bearbeitungsbereich | Löschzeichen | Mehrsprachiger Text | Nein | DMDELETEDOBJECTINFO_EDITINGAREA |

## SPS-Daten

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 20097 | Prozessobjekt | Funktionen | Einsprachiger Text | Nein | FUNC_PLCPROCESSDATA |
| 20108 | Konfigurationsprojekt (indirekt) | Funktionen SPS-Adressen (in Auswertungen) SPS-Objekte SPS-Adressen | Einsprachiger Text | Nein | FUNC_PLCCONFIGURATIONPROJECT_INDIRECT |
| 20161 | Konfigurationsprojekt (an SPS-Kästen und Bus-Anschlüssen) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCCONFIGURATIONPROJECT |
| 20162 | SPS-Objekt-ID (an Funktionen) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCOBJECTID |
| 20164 | Bus-Koppler / Kopfstation | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLCISBUSCOUPLER |
| 20167 | CPU | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLCISCPU |
| 20184 | Spannungsversorgung | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLCISPOWERSUPPLY |
| 20189 | Bus-Verteiler | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLCISBUSDISTRIBUTOR |
| 20196 | Ohne Sensor- / Aktor-Suche für die symbolische Adresse | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLCNO_TARGETSEARCH_FOR_AUTOSYMBOLICADDRESS |
| 20253 | CPU: Name | Funktionen | Einsprachiger Text | Ja, 128 | FUNC_PLCCPU_NAME |
| 20255 | Startadresse 2 der SPS-Karte | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_2 |
| 20258 | Sortierkennung (für Position im Baugruppenträger) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCMODULE_SORTCODE |
| 20259 | Anzahl der Positionen im Baugruppenträger | Funktionen | Einsprachiger Text | Nein | FUNC_PLCRIGHTRACKSIZE |
| 20260 | Anzahl der Positionen im Baugruppenträger (nach links) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCLEFTRACKSIZE |
| 20289 | Integrierte Baugruppe | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLCMODULE_ISINTEGRATED |
| 20298 | Startadresse 2 der SPS-Karte (indirekt) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_2_INDIRECT |
| 20299 | Adressbereich 2 (SIEMENS STEP 7 Classic) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCADDRESSRANGE_2 |
| 20308 | Bus-System | Funktionen Funktionsschablonen | Ganzzahlig | Nein | FUNC_PLC_BUSSYSTEM |
| 20309 | DNS/PROFINET Gerätename | Funktionen | Einsprachiger Text | Nein | FUNC_PLC_DNS |
| 20310 | Logisches Netz: Bus-Anschluss ist Master | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLCISMASTER |
| 20311 | Physikalisches Netz: Bus-Adresse / Positionsnummer | Funktionen | Einsprachiger Text | Nein | FUNC_PLC_BUS_ADDRESS |
| 20312 | Untersteckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLC_SUBSLOT |
| 20313 | Datenübertragungsrate | Funktionen | Einsprachiger Text | Nein | FUNC_PLC_TRANSMISSIONRATE |
| 20315 | SPS-spezifische Einstellungen | Funktionen | Einsprachiger Text | Nein | FUNC_PLCCPU_RELATEDPLCSCHEMA |
| 20334 | MasterSystemID | Funktionen | Einsprachiger Text | Nein | FUNC_PLC_MASTERSYSTEMID |
| 20338 | Bus-System (indirekt) | Funktionen Funktionsschablonen | Ganzzahlig | Nein | FUNC_PLC_BUSSYSTEM_INDIRECT |
| 20380 | Beim Adressieren nicht berücksichtigen | Funktionen SPS-Adressen | Bool (Wahr / Falsch) | Nein | FUNC_PLCTERMINAL_NO_ADRESSING |
| 20381 | Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICE_INDEX |
| 20382 | SPS-Untergerät 3: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_3_INPUTS |
| 20384 | SPS-Untergerät: Index | Funktionen SPS-Adressen Funktionsschablonen | Ganzzahlig | Nein | FUNC_PLCTERMINAL_INDEX_OF_STARTADDRESS |
| 20386 | Physikalisches Netz: Bus-Adresse / Positionsnummer 2 | Funktionen | Einsprachiger Text | Nein | FUNC_PLC_BUS_ADDRESS_2 |
| 20387 | Physikalisches Netz: Bus-Adresse / Positionsnummer 2 (indirekt) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCBUS_ADDRESS_2_INDIRECT |
| 20388 | Signalbereich | Funktionen Funktionsschablonen SPS-Adressen (in Auswertungen) SPS-Adressen | Einsprachiger Text | Nein | FUNC_PLC_SIGNALRANGE |
| 20390 | Bus-Schnittstelle: Name (indirekt) | Funktionen | Einsprachiger Text | Nein | FUNC_PLC_BUS_INTERFACENAME_INDIRECT |
| 20391 | Bus-Schnittstelle: Haupt-Bus-Anschluss (indirekt) | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLC_BUS_INTERFACENAME_MASTER_INDIRECT |
| 20392 | SPS-Untergerät 4: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_4_INPUTS |
| 20393 | SPS-Untergerät 5: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_5_INPUTS |
| 20394 | SPS-Untergerät 6: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_6_INPUTS |
| 20395 | SPS-Untergerät 7: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_7_INPUTS |
| 20396 | SPS-Untergerät 8: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_8_INPUTS |
| 20397 | SPS-Untergerät 9: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_9_INPUTS |
| 20398 | SPS-Untergerät 10: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_10_INPUTS |
| 20400 | SPS-Adresse | Funktionen SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | FUNC_PLCADDRESS |
| 20402 | Symbolische Adresse | Funktionen SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | FUNC_PLCSYMBOLICADDRESS_MANUAL |
| 20403 | Symbolische Adresse (ermittelt) | Funktionen SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | FUNC_PLCSYMBOLICADDRESS_CALCULATED |
| 20404 | Symbolische Adresse (automatisch) | Funktionen SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | FUNC_PLCSYMBOLICADDRESS_AUTOMATIC |
| 20405 | Datentyp | Funktionen SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | FUNC_PLCDATATYPE |
| 20407 | Kanalbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCCHANNEL_DESIGNATION_MANUAL |
| 20408 | SPS-Stationsname | Funktionen | Einsprachiger Text | Nein | FUNC_PLCSTATIONNAME |
| 20409 | SPS-Stationstyp | Funktionen | Einsprachiger Text | Nein | FUNC_PLCSTATIONTYPE |
| 20410 | SPS-Karte steckt auf Baugruppenträger | Funktionen | Einsprachiger Text | Nein | FUNC_PLCRACK |
| 20411 | An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCSOCKET |
| 20412 | Fehlende Bus-Adresse ignorieren | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLC_IGNORE_BUSADDRESS |
| 20413 | Physikalisches Netz: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDP_SYSTEM |
| 20414 | Logisches Netz: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCMASTER |
| 20415 | Gerätebeschreibung: Dateiname | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICE_ID |
| 20416 | SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCTYPE_ID |
| 20417 | Objektbeschreibung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCOBJECT_DESCRIPTION |
| 20418 | Version | Funktionen | Einsprachiger Text | Nein | FUNC_PLCFIRMWARE_VERSION |
| 20419 | Startadresse der SPS-Karte | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS |
| 20420 | SPS-Stationsname (indirekt) | Funktionen SPS-Adressen (in Auswertungen) SPS-Objekte SPS-Adressen | Einsprachiger Text | Nein | FUNC_PLCSTATIONNAME_INDIRECT |
| 20421 | SPS-Karte steckt auf Baugruppenträger (indirekt) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCRACK_INDIRECT |
| 20422 | An Position / Steckplatz (indirekt) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCSOCKET_INDIRECT |
| 20423 | Startadresse der SPS-Karte (indirekt) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_INDIRECT |
| 20424 | Logisches Netz: Name (indirekt) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCMASTER_INDIRECT |
| 20425 | Physikalisches Netz: Name (indirekt) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDP_SYSTEM_INDIRECT |
| 20426 | Physikalisches Netz: Bus-Adresse / Positionsnummer (indirekt) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCBUS_ADDRESS_INDIRECT |
| 20427 | Baugruppenträger | Funktionen | Einsprachiger Text | Nein | FUNC_PLCTHISRACK |
| 20428 | Kanalbezeichnung (automatisch) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCCHANNEL_DESIGNATION_AUTOMATIC |
| 20432 | Adressbereich (SIEMENS STEP 7 Classic) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCADDRESSRANGE |
| 20433 | CPU: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCCPU |
| 20434 | CPU (indirekt) | Funktionen SPS-Adressen (in Auswertungen) SPS-Objekte SPS-Adressen | Einsprachiger Text | Nein | FUNC_PLCCPU_INDIRECT |
| 20437 | SPS-Kartenname | Funktionen | Einsprachiger Text | Nein | FUNC_PLCMODULENAME |
| 20438 | Deaktivierter E-/A-Anschluss | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLCDEACTIVATED_IO |
| 20439 | Safety-Adresse | Funktionen | Einsprachiger Text | Nein | FUNC_PLCSAFETYADDRESS |
| 20444 | SPS-Karte steckt auf Kopfstation | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLCISMOUNTEDONHEADMODULE |
| 20446 | Subnetzmaske | Funktionen | Einsprachiger Text | Nein | FUNC_PLC_SUBNETMASK |
| 20447 | Bus-Schnittstelle: Name | Funktionen Funktionsschablonen | Einsprachiger Text | Nein | FUNC_PLC_BUS_INTERFACENAME |
| 20448 | Bus-Schnittstelle: Haupt-Bus-Anschluss | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_PLC_BUS_INTERFACENAME_MASTER |
| 20452 | SPS-Untergerät 11: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_11_INPUTS |
| 20453 | SPS-Untergerät 12: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_12_INPUTS |
| 20454 | SPS-Untergerät 2: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_2_INPUTS |
| 20455 | SPS-Untergerät 2: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_2_OUTPUTS |
| 20456 | SPS-Untergerät 3: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_3_OUTPUTS |
| 20457 | SPS-Untergerät 4: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_4_OUTPUTS |
| 20478 | SPS-Untergerät 5: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_5_OUTPUTS |
| 20479 | SPS-Untergerät 6: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_6_OUTPUTS |
| 20515 | SPS-Untergerät 7: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_7_OUTPUTS |
| 20516 | SPS-Untergerät 8: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_8_OUTPUTS |
| 20517 | SPS-Untergerät 9: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_9_OUTPUTS |
| 20518 | SPS-Untergerät 10: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_10_OUTPUTS |
| 20519 | SPS-Untergerät 11: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_11_OUTPUTS |
| 20520 | SPS-Untergerät 12: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_12_OUTPUTS |
| 20521 | SPS-Untergerät 1: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_1 |
| 20522 | SPS-Untergerät 2: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_2 |
| 20523 | SPS-Untergerät 3: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_3 |
| 20524 | SPS-Untergerät 4: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_4 |
| 20525 | SPS-Untergerät 5: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_5 |
| 20526 | SPS-Untergerät 6: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_6 |
| 20527 | SPS-Untergerät 7: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_7 |
| 20528 | SPS-Untergerät 8: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_8 |
| 20529 | SPS-Untergerät 9: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_9 |
| 20530 | SPS-Untergerät 10: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_10 |
| 20531 | SPS-Untergerät 11: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_11 |
| 20532 | SPS-Untergerät 12: Name | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENAME_12 |
| 20533 | SPS-Untergerät 1: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_1 |
| 20534 | SPS-Untergerät 2: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_2 |
| 20535 | SPS-Untergerät 3: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_3 |
| 20536 | SPS-Untergerät 4: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_4 |
| 20537 | SPS-Untergerät 5: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_5 |
| 20538 | SPS-Untergerät 6: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_6 |
| 20539 | SPS-Untergerät 7: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_7 |
| 20540 | SPS-Untergerät 8: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_8 |
| 20541 | SPS-Untergerät 9: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_9 |
| 20542 | SPS-Untergerät 10: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_10 |
| 20543 | SPS-Untergerät 11: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_11 |
| 20544 | SPS-Untergerät 12: An Position / Steckplatz | Funktionen | Einsprachiger Text | Nein | FUNC_PLCDEVICENUMBER_12 |
| 20547 | SPS-Untergerät 2: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_2 |
| 20548 | SPS-Gerät: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS |
| 20549 | SPS-Untergerät 2: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_2 |
| 20550 | SPS-Gerät: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS |
| 20551 | SPS-Untergerät 3: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_3 |
| 20552 | SPS-Untergerät 4: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_4 |
| 20553 | SPS-Untergerät 5: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_5 |
| 20554 | SPS-Untergerät 6: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_6 |
| 20555 | SPS-Untergerät 7: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_7 |
| 20556 | SPS-Untergerät 8: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_8 |
| 20557 | SPS-Untergerät 9: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_9 |
| 20558 | SPS-Untergerät 10: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_10 |
| 20559 | SPS-Untergerät 11: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_11 |
| 20560 | SPS-Untergerät 3: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_3 |
| 20561 | SPS-Untergerät 4: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_4 |
| 20562 | SPS-Untergerät 5: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_5 |
| 20563 | SPS-Untergerät 6: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_6 |
| 20564 | SPS-Untergerät 7: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_7 |
| 20565 | SPS-Untergerät 8: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_8 |
| 20566 | SPS-Untergerät 9: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_9 |
| 20567 | SPS-Untergerät 10: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_10 |
| 20568 | SPS-Untergerät 11: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_11 |
| 20569 | SPS-Untergerät 12: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_12 |
| 20570 | SPS-Untergerät 12: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_12 |
| 20571 | SPS-Gerät: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS |
| 20572 | SPS-Untergerät 2: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_2 |
| 20573 | SPS-Gerät: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS |
| 20574 | SPS-Untergerät 2: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_2 |
| 20576 | Antrieb | Funktionen | Einsprachiger Text | Ja, 10 | FUNC_PLCAXIS_DESIGNATION |
| 20577 | Antrieb: Gerätetyp | Funktionen | Einsprachiger Text | Nein | FUNC_ARTICLE_PLCAXIS_DEVICETYPE |
| 20578 | Kanalbezeichnung (automatisch): Suche unterdrücken | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_SUPRESS_SEARCH_CHANNELDESIGNATION |
| 20579 | Steckerbezeichnung (automatisch): Suche unterdrücken | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_SUPRESS_SEARCH_PLCPLUG_DESIGNATION |
| 20580 | TemplateReference | Funktionen | Einsprachiger Text | Nein | FUNC_ARTICLE_PLCTEMPLATEREFERENCE |
| 20581 | Konfigurationsprojekt (automatisch, an Bus-Anschlüssen) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCCONFIGURATIONPROJECT_AUTOMATIC |
| 20582 | SPS-Untergerät 2: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_2 |
| 20583 | SPS-Untergerät 3: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_3 |
| 20584 | SPS-Untergerät 4: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_4 |
| 20585 | SPS-Untergerät 5: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_5 |
| 20586 | SPS-Untergerät 6: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_6 |
| 20587 | SPS-Untergerät 7: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_7 |
| 20588 | SPS-Untergerät 8: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_8 |
| 20589 | SPS-Untergerät 9: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_9 |
| 20590 | SPS-Untergerät 10: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_10 |
| 20591 | SPS-Untergerät 11: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_11 |
| 20592 | SPS-Untergerät 12: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_12 |
| 20593 | SPS-Untergerät 2: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_2 |
| 20594 | SPS-Untergerät 3: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_3 |
| 20595 | SPS-Untergerät 4: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_4 |
| 20596 | SPS-Untergerät 5: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_5 |
| 20597 | SPS-Untergerät 6: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_6 |
| 20598 | SPS-Untergerät 7: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_7 |
| 20599 | SPS-Untergerät 8: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_8 |
| 20600 | SPS-Untergerät 9: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_9 |
| 20601 | SPS-Untergerät 10: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_10 |
| 20602 | SPS-Untergerät 11: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_11 |
| 20603 | SPS-Untergerät 12: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_12 |
| 20604 | SPS-Untergerät 1: Datenlänge (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_OUTPUTS_1 |
| 20605 | SPS-Untergerät 1: Datenlänge (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_DATALENGTH_INPUTS_1 |
| 20606 | SPS-Untergerät 1: Gerätebeschreibung: Index in Datei | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_INDEXINFILE_1 |
| 20607 | SPS-Untergerät 1: SPS-Typbezeichnung | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_TYPEIDENTIFIER_1 |
| 20608 | SPS-Untergerät 1: Startadresse (Ausgänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_1_OUTPUTS |
| 20609 | SPS-Untergerät 1: Startadresse (Eingänge) | Funktionen | Einsprachiger Text | Nein | FUNC_PLCGROUP_STARTADDRESS_1_INPUTS |
| 20610 | Symbolische Adresse: Gruppe | Funktionen SPS-Adressen (in Auswertungen) | Einsprachiger Text | Nein | FUNC_PLCSYMBOLICADDRESS_GROUP |
| 20854 | Angeschlossene SPS-Adresse | Funktionen | Einsprachiger Text | Nein | FUNC_TERMINAL_PLCADDRESS_MANUAL |
| 20855 | Angeschlossene SPS-Adresse (automatisch) | Funktionen | Einsprachiger Text | Nein | FUNC_TERMINAL_PLCADDRESS_AUTOMATIC |
| 22291 | SPS-Untergerät 3: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_3 |
| 22317 | SPS-Untergerät 4: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_4 |
| 22318 | SPS-Untergerät 5: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_5 |
| 22319 | SPS-Untergerät 6: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_6 |
| 22320 | SPS-Untergerät 7: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_7 |
| 22321 | SPS-Untergerät 8: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_8 |
| 22322 | SPS-Untergerät 9: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_9 |
| 22323 | SPS-Untergerät 10: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_10 |
| 22324 | SPS-Untergerät 11: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_11 |
| 22325 | SPS-Untergerät 12: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_12 |
| 22326 | SPS-Untergerät 3: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_3 |
| 22327 | SPS-Untergerät 4: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_4 |
| 22328 | SPS-Untergerät 5: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_5 |
| 22329 | SPS-Untergerät 6: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_6 |
| 22330 | SPS-Untergerät 7: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_7 |
| 22331 | SPS-Untergerät 8: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_8 |
| 22332 | SPS-Untergerät 9: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_9 |
| 22333 | SPS-Untergerät 10: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_10 |
| 22334 | SPS-Untergerät 11: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_11 |
| 22335 | SPS-Untergerät 12: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_12 |
| 22341 | SPS-Untergerät 2: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_2 |
| 22342 | SPS-Untergerät 3: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_3 |
| 22343 | SPS-Untergerät 4: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_4 |
| 22344 | SPS-Untergerät 5: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_5 |
| 22345 | SPS-Untergerät 6: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_6 |
| 22346 | SPS-Untergerät 7: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_7 |
| 22347 | SPS-Untergerät 8: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_8 |
| 22348 | SPS-Untergerät 9: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_9 |
| 22349 | SPS-Untergerät 10: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_10 |
| 22350 | SPS-Untergerät 11: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_11 |
| 22351 | SPS-Untergerät 12: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_12 |
| 22352 | SPS-Untergerät 2: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_2 |
| 22353 | SPS-Untergerät 3: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_3 |
| 22354 | SPS-Untergerät 4: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_4 |
| 22355 | SPS-Untergerät 5: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_5 |
| 22356 | SPS-Untergerät 6: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_6 |
| 22357 | SPS-Untergerät 7: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_7 |
| 22358 | SPS-Untergerät 8: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_8 |
| 22359 | SPS-Untergerät 9: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_9 |
| 22360 | SPS-Untergerät 10: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_10 |
| 22361 | SPS-Untergerät 11: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_11 |
| 22362 | SPS-Untergerät 12: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_12 |
| 22363 | SPS-Untergerät 1: Datenlänge (Eingänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_INPUTS_1 |
| 22364 | SPS-Untergerät 1: Datenlänge (Ausgänge) | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_DATALENGTH_OUTPUTS_1 |
| 22365 | SPS-Untergerät 1: SPS-Typbezeichnung | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_TYPIDENTIFIER_1 |
| 22366 | SPS-Untergerät 1: Gerätebeschreibung: Index in Datei | Artikel (eingelagert) | Einsprachiger Text | Nein | ARTICLE_PLCGROUP_INDEXINFILE_1 |
| 44020 | SPS-Adresse: Richtung | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Ganzzahlig | Ja, 50 | DMPLAOBJECT_PLCADDRESS_DIRECTION |
| 44021 | SPS-Adresse: Datentyp | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Ja, 50 | DMPLAOBJECT_PLCADDRESS_DATATYPE |
| 44022 | SPS-Adresse: CPU | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Ja, 50 | DMPLAOBJECT_PLCADDRESS_CPU |
| 44023 | SPS-Adresse: Adresse | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Ja, 50 | DMPLAOBJECT_PLCADDRESS_ADDRESS |
| 44024 | SPS-Adresse: Symbolische Adresse | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Ja, 50 | DMPLAOBJECT_PLCADDRESS_SYMBOLICADDRESS |
| 44025 | SPS-Adresse: Funktionstext | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Mehrsprachiger Text | Ja, 50 | DMPLAOBJECT_PLCADDRESS_FUNCTIONTEXT |
| 44047 | SPS-Adresse: Station | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Ja, 50 | DMPLAOBJECT_PLCADDRESS_STATION |
| 44048 | SPS-Adresse: Konfigurationsprojekt | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Ja, 50 | DMPLAOBJECT_PLCADDRESS_CONFIGURATIONPROJECT |
| 44052 | SPS-Adresse: Symbolische Adresse (Einzelbestandteil) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Ja, 50 | DMPLAOBJECT_PLCADDRESS_SYMBOLICADDRESS_PART |
| 44053 | Symbolische Adresse (Einzelbestandteil) | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Nein | DMPLAOBJECT_SYMBOLICADDRESS_PART |
| 44064 | SPS-Adresse: Zähler | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Bool (Wahr / Falsch) | Ja, 50 | DMPLAOBJECT_PLCADDRESS_COUNTER |
| 44087 | SPS-Adresse: Symbolische Adresse: Gruppe | Segmente (Vorplanung) Segmentvorlagen (Vorplanung) | Einsprachiger Text | Ja, 50 | DMPLAOBJECT_PLCADDRESS_SYMBOLICADDRESS_GROUP |

## Spezial

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 11004 | Seitenanzahl pro Struktur | Seiten | Ganzzahlig | Nein | PAGE_NUMBERPAGES_IN_LOCATION |
| 11040 | Folgeblatt | Seiten | Einsprachiger Text | Nein | PAGE_NEXT |
| 11041 | Rückblatt | Seiten | Einsprachiger Text | Nein | PAGE_PREV |
| 11043 | Höchste Seitenzahl pro Struktur | Seiten | Einsprachiger Text | Nein | PAGE_MAXNUMBERLOCATION |
| 11049 | Eingefrorene Auswertungsseite | Seiten | Bool (Wahr / Falsch) | Nein | PAGE_FROZEN_EVALUATIONPAGE |
| 11050 | Automatikseite geändert | Seiten | Bool (Wahr / Falsch) | Nein | PAGE_AUTOPAGECHANGED |
| 11062 | Seitenanzahl pro Eigenschaft | Seiten | Ganzzahlig | Nein | PAGE_NUMBERPAGES_WITH_PROPERTY |
| 11063 | Seitennamen pro Eigenschaft | Seiten | Einsprachiger Text | Nein | PAGE_PAGENAMES_WITH_PROPERTY |
| 11064 | Seitenzähler pro Eigenschaft | Seiten | Ganzzahlig | Nein | PAGE_PAGENUMBER_WITH_PROPERTY |
| 19800 | Betroffene Funktionen | Pfad-Funktionstexte | Einsprachiger Text | Nein | PATHTEXT_FUNCTIONS |
| 25100 | Teilprojekt von | Projekte | Einsprachiger Text | Nein | PROJECT_SUBPROJECTORIGIN |
| 25101 | Teilprojektnummer | Seiten Projekte Bauräume Segmente (Vorplanung) Verbindungen Basisklasse für Funktionen | Einsprachiger Text | Nein | SUBPROJECT_NUMBER |

## Status

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 20186 | Zum Löschen markiert | Funktionen SPS-Adressen Verbindungen Verbindungsdefinitionspunkte | Bool (Wahr / Falsch) | Nein | FUNCTION_PLCIMPORTCOMPARE_MARKEDTOBEDELETED |
| 20286 | Funktion ist mit Darstellungsart 'Funktional' vorhanden | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_ISPLACEDIN_FUNCTIONAL |
| 20333 | Funktion ist mit Darstellungsart 'Topologie' vorhanden | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_ISPLACEDIN_CABLING |
| 20445 | Funktion ist mit Darstellungsart 'Funktionsübersicht (Fluid)' vorhanden | Funktionen | Bool (Wahr / Falsch) | Nein | FUNC_ISPLACEDIN_FUNCTIONALOVERVIEW |
| 20470 | Funktion ist mit Darstellungsart 'Allpolig' vorhanden | Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | FUNC_ISPLACEDIN_CIRCUIT |
| 20471 | Funktion ist mit Darstellungsart 'Einpolig' vorhanden | Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | FUNC_ISPLACEDIN_SINGLELINE |
| 20472 | Funktion ist mit Darstellungsart 'Paarquerverweis' vorhanden | Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | FUNC_ISPLACEDIN_PAIRCROSSREFERENCE |
| 20473 | Funktion ist mit Darstellungsart 'Übersicht' vorhanden | Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | FUNC_ISPLACEDIN_OVERVIEW |
| 20474 | Funktion ist mit Darstellungsart 'RI-Fließbild' vorhanden | Funktionen Verbindungen Artikelreferenzen Artikelreferenzen (Darstellungsarten zusammengefasst) | Bool (Wahr / Falsch) | Nein | FUNC_ISPLACEDIN_PROCESSANDINSTDIAGRAM |

## Symboldaten

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 16032 | Symbol für Motorschutzkontaktspiegel | Symbole | Einsprachiger Text | Nein | SYMB_MOTOR_CONTACT_IMAGE |
| 20111 | Symbolbibliothek | Basisklasse für Funktionen Potenzialdefinitionspunkte | Einsprachiger Text | Nein | FUNC_SYMBLIB_NAME |
| 20112 | Symbolname | Basisklasse für Funktionen Potenzialdefinitionspunkte | Einsprachiger Text | Nein | FUNC_SYMB_NAME |
| 20113 | Symbolvariante | Basisklasse für Funktionen Potenzialdefinitionspunkte | Einsprachiger Text | Nein | FUNC_SYMB_VARIANT |
| 20114 | Symbolbeschreibung (Funktion) | Basisklasse für Funktionen Potenzialdefinitionspunkte | Mehrsprachiger Text | Nein | FUNC_SYMB_DESC |
| 20575 | Symbol | Basisklasse für Funktionen Potenzialdefinitionspunkte | Einsprachiger Text | Nein | FUNC_SYMBOL |

## Thermische Auslegung

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 10310 | Thermische Auslegung: Spannung | Projekte | Ganzzahlig | Nein | PROJECT_POWERDISSIPATION_VOLTAGE |
| 10311 | Thermische Auslegung: Frequenz | Projekte | Ganzzahlig | Nein | PROJECT_POWERDISSIPATION_FREQUENCY |
| 10312 | Thermische Auslegung: Gleichzeitigkeitsfaktor | Projekte | Dezimalzahl | Nein | PROJECT_POWERDISSIPATION_SIMULTANEITYFACTOR |
| 10313 | Thermische Auslegung: Gesamtverlustleistung für Klimabereich | Projekte | Einsprachiger Text | Ja, 100 | PROJECT_POWERDISSIPATION_POWER_OF_ZONE |
| 36044 | Thermische Auslegung: Gleichzeitigkeitsfaktor (automatisch) | 3D-Platzierungen | Dezimalzahl | Nein | FUNCTION3D_POWERDISSIPATION_SIMULTANEITYFACTOR_AUTOMATIC |
| 36045 | Thermische Auslegung: Gleichzeitigkeitsfaktor (Gerät) | 3D-Platzierungen | Dezimalzahl | Nein | FUNCTION3D_POWERDISSIPATION_SIMULTANEITYFACTOR |
| 36046 | Thermische Auslegung: Klimabereich (automatisch) | 3D-Platzierungen | Ganzzahlig | Nein | FUNCTION3D_POWERDISSIPATION_ZONE_AUTOMATIC |
| 36047 | Thermische Auslegung: Klimabereich (Gerät) | 3D-Platzierungen | Ganzzahlig | Nein | FUNCTION3D_POWERDISSIPATION_ZONE |
| 36048 | Thermische Auslegung: Geräteverlustleistung | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_POWERDISSIPATION_AUTOMATIC |
| 36049 | Thermische Auslegung: Aufstellsituation | 3D-Platzierungen | Ganzzahlig | Nein | FUNCTION3D_POWERDISSIPATION_CABINET_SITUATION |
| 36050 | Thermische Auslegung: Min. Außentemperatur | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_POWERDISSIPATION_CABINET_MIN_OUT_TEMP |
| 36051 | Thermische Auslegung: Max. Außentemperatur | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_POWERDISSIPATION_CABINET_MAX_OUT_TEMP |
| 36052 | Thermische Auslegung: Min. Innentemperatur | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_POWERDISSIPATION_CABINET_MIN_IN_TEMP |
| 36053 | Thermische Auslegung: Max. Innentemperatur | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_POWERDISSIPATION_CABINET_MAX_IN_TEMP |
| 36054 | Thermische Auslegung: Klimabereich | 3D-Platzierungen | Ganzzahlig | Nein | FUNCTION3D_POWERDISSIPATION_CABINET_ZONE |
| 36055 | Thermische Auslegung: Betriebsstrom | 3D-Platzierungen | Einsprachiger Text | Nein | FUNCTION3D_POWERDISSIPATION_BUSBAR_CURRENT |
| 36460 | Thermische Auslegung: Maximale Wurfweite | Basisklasse für Pro Panel-Objekte | Koordinaten (nur X oder Y) | Nein | DMCABBASE_THERMDIST |
| 36461 | Thermische Auslegung: Ansaugwinkel | Basisklasse für Pro Panel-Objekte | Dezimalzahl | Nein | DMCABBASE_ANGLE_IN |
| 36462 | Thermische Auslegung: Ausblaswinkel | Basisklasse für Pro Panel-Objekte | Dezimalzahl | Nein | DMCABBASE_ANGLE_OUT |

## Verbindung

| ID | Bezeichnung | Objektklassen | Feldtyp | Index | API-Define |
|---|---|---|---|---|---|
| 33004 | Frequenz | Potenzialdefinitionspunkte | Einsprachiger Text | Nein | POTENTIAL_FREQUENCE |
| 33005 | Mögliche Gegenpotenziale | Potenzialdefinitionspunkte | Einsprachiger Text | Nein | POTENTIAL_VALID_POTENTIALS |
