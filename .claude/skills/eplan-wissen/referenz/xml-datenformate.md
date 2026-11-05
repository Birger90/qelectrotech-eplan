# XML-Datenformate von EPLAN: Artikel-XML, Funktionsdefinitionen, Produktgruppen, Auswahllisten (EPLAN 2.9)

EPLAN tauscht Artikeldaten (Artikelverwaltung, Data-Portal-Pakete, `part.xml` in EDZ-Dateien) als XML mit dem Wurzel-Tag `<partsmanagement>` aus. Diese Datei fasst das Format nach der Hilfe zusammen und enthält kompakte Nachschlagetabellen: alle Tags mit Attributen, die Funktionsdefinitions-Kennungen `Kategorie.Gruppe.Id`, die Produktgruppen-Nummern und die Auswahllisten. Am Ende steht, was der EDZ-/Makro-Import im QET-Fork davon nutzt und was fehlt.

## Inhalt
1. [Aufbau und Grundregeln](#1-aufbau-und-grundregeln)
2. [Datentypen und mehrsprachige Texte](#2-datentypen-und-mehrsprachige-texte)
3. [Tags und Attribute (Nachschlagetabellen)](#3-tags-und-attribute-nachschlagetabellen)
4. [functiontemplate: Bedeutung je Produktgruppe](#4-functiontemplate-bedeutung-je-produktgruppe)
5. [Funktionsdefinitionen: Format und wichtigste Kennungen](#5-funktionsdefinitionen-format-und-wichtigste-kennungen)
6. [Produktgruppen und Auswahllisten](#6-produktgruppen-und-auswahllisten)
7. [Bedeutung für QET (Fork-Import `sources/import/edz`, `sources/import/eplandxf`)](#7-bedeutung-für-qet-fork-import)
8. [Anhang: Vollständige Tabelle der Funktionsdefinitionen](#8-anhang-vollständige-tabelle-der-funktionsdefinitionen)
9. [Quellen](#quellen)

---

## 1. Aufbau und Grundregeln

- Wurzel `<partsmanagement>` **genau einmal**; alle anderen Tags beliebig oft oder weglassbar (z.B. Variante ohne Funktionsschablone, Bohrbild ohne Daten). **Ausnahme**: jedes `<part>` braucht mindestens eine `<variant>`. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_k_start.htm
- Tags laut Übersicht: `partsmanagement`, `part`, `variant`, `functiontemplate`, `freeproperty`, `blockingSurfacePosition`, `doorPosition`, `mountingPanelPosition`, `supportBarPosition`, `assemblyposition`, `moduleposition`, `attributeposition`, `accessoryposition`, `accessorylist`, `accessorylistposition`, `construction`, `drillingPosition`, `terminal`, `terminalPosition`, `safetyRelatedValuePosition`, `address`; die Tag-Tabelle nennt zusätzlich `accessoryplacement`, `accessoryplacementposition`, `constructionPosition`. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_k_start.htm, https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_tags.htm
- Verschachtelung beschreibt die Hilfe nicht ausdrücklich. Beobachtet in einer EDZ (Eaton, `part.xml`, Export 2023): `<partsmanagement>` → `<part …>` → `<variant P_ARTICLE_VARIANT="1"/>` und `<freeproperty>`; Vermutung: `<functiontemplate>` liegt unter `<variant>` (Schablonen sind variantenbezogen), Positionen (`terminalPosition`, `drillingPosition`) unter `terminal`/`construction`.
- Feste Pflicht-/Sonderwerte: `part/P_ARTICLE_PARTTYPE` muss 1 (Einzelteil), 4 (Baugruppe) oder 5 (Modul) sein; `accessorylist/class` nur 13, `accessoryplacement/class` nur 14, `construction/class` nur 6, `terminal/class` nur 7; `accessoryposition/parttype` nur Einzelteil oder Zubehörliste; `address/class` nur 11 Kunde / 12 Hersteller-Lieferant; `freeproperty/pos` max. 200. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_tags.htm
- `<partsmanagement>`-Attribute: `length-unit` (optional, Vorgabe „mm“, auch „inch“), `type` (nur „EPLAN.PartsManagement“), `build` (Build-Nr. beim Export). Neuere Exporte haben zusätzlich `count`, `weight-unit`, `version` (beobachtet, nicht in der 2.9-Hilfe). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_tags.htm
- Indizierte Eigenschaften werden mit Minus-Index ausgegeben, z.B. eigenes erstes Untergewerk unter Elektrotechnik als `P_ARTICLE_SUBCRAFT_ELECTRICAL-1="??_??@test;"`. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_tags.htm

## 2. Datentypen und mehrsprachige Texte

| Typ | Regel |
|---|---|
| Bool | `"0"` oder `" "` = nein, `"1"` = ja |
| Integer / Long | z.B. `"-5"`, `"0"`; `""` = 0; bei Auswahllisten nur die gelisteten Werte |
| Double / Currency | Dezimal**punkt** (`"2.5"`); `""` = 0 |
| Text / Memo | beliebiger Text, Länge laut Tabelle (Memo: unbegrenzt, „0“) |
| Mehrsprachig | `de_DE@Schütz;en_US@Contactor;` – Sprachkennung, `@`, Text, `;` je Sprache; `??_??@` = sprachneutral |

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_tags.htm

- Pfade in Attributen dürfen Variablen enthalten: `P_ARTICLE_GROUPSYMBOLMACRO="$(MD_MACROS)\Macro.ema"`, Textur `$(MD_IMAGES)\…`. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_tags.htm
- Anschlussbezeichnungen in `connectionDesignation` sind durch **Zeilenumbruch** getrennt (eine Funktionsschablone = eine Funktion mit mehreren Anschlüssen, z.B. Spule A1/A2). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_tags.htm

## 3. Tags und Attribute (Nachschlagetabellen)

Spalten: Attribut (wie im XML), Typ (Länge; B = Byte), deutsche Feldbezeichnung, Hinweis. Auswahllisten-Werte stehen in Abschnitt 6. Quelle aller Tabellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_tags.htm

#### `<partsmanagement>`

| Attribut | Feld | Hinweis |
|---|---|---|
| `length-unit` | Längeneinheit | optional, Vorgabe „mm“ |
| `type` | Typ | nur „EPLAN.PartsManagement“ |
| `build` | Build-Nummer | optional |

#### `<part>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `P_ARTICLE_PARTTYPE` | Integer (2B) | Typ | **Pflicht**: 1 = Einzelteil, 4 = Baugruppe, 5 = Modul |
| `P_ARTICLE_PARTNR` | Text (255) | Artikelnummer |  |
| `P_ARTICLE_IS_ACCESSORY` | Bool (1B) | Zubehör |  |
| `P_ARTICLE_CERTIFICATE` | Text (50) | Zertifizierung: Allgemein |  |
| `P_ARTICLE_CERTIFICATE_CE` | Bool (1B) | Zertifizierung: CE-Kennung |  |
| `P_ARTICLE_CERTIFICATE_UL` | Text (50) | Zertifizierung: UL-Kennung |  |
| `P_ARTICLE_CERTIFICATE_VDE` | Text (50) | Zertifizierung: VDE-Kennung |  |
| `P_ARTICLE_CERTIFICATE_ATEX` | Text (50) | Zertifizierung: ATEX-Kennung |  |
| `P_ARTICLE_REF_CONSTRUCTION_NAME` | Text (40) | Bohrbild |  |
| `P_ARTICLE_CRAFT_COOLING` | Bool (1B) | Gewerk Kühlung |  |
| `P_ARTICLE_CRAFT_ELECTRICAL` | Bool (1B) | Gewerk Elektrotechnik |  |
| `P_ARTICLE_CRAFT_FLUID` | Bool (1B) | Gewerk Fluid |  |
| `P_ARTICLE_CRAFT_HYDRAULICS` | Bool (1B) | Gewerk Hydraulik |  |
| `P_ARTICLE_CRAFT_LUBRICATION` | Bool (1B) | Gewerk Schmierung |  |
| `P_ARTICLE_CRAFT_MECHANICS` | Bool (1B) | Gewerk Mechanik |  |
| `P_ARTICLE_CRAFT_PNEUMATICS` | Bool (1B) | Gewerk Pneumatik |  |
| `P_ARTICLE_CRAFT_COOLINGLUBRICANT` | Bool (1B) | Gewerk Kühlschmiermittel |  |
| `P_ARTICLE_CRAFT_GASTECHNOLOGY` | Bool (1B) | Gewerk Gastechnik |  |
| `P_ARTICLE_CRAFT_FLUID_UNDEFINED` | Bool (1B) | Gewerk Fluid (undefiniert) |  |
| `P_ARTICLE_CRAFT_PROCESS` | Bool (1B) | Gewerk Verfahrenstechnik |  |
| `P_PART_CREATE` | Text (40) | Erstellungsdatum |  |
| `P_ARTICLE_DEPTH` | Double (8B) | Tiefe |  |
| `P_ARTICLE_DISCONTINUED` | Bool (1B) | Auslaufartikel |  |
| `P_ARTICLE_SPACING_FRONT` | Double (8B) | Einbauabstand Tiefe vorne |  |
| `P_ARTICLE_SPACING_REAR` | Double (8B) | Einbauabstand Tiefe hinten |  |
| `P_ARTICLE_DESCR1` | Memo (0) | Bezeichnung 1 | Mehrsprachig |
| `P_ARTICLE_DESCR2` | Memo (0) | Bezeichnung 2 | Mehrsprachig |
| `P_ARTICLE_DESCR3` | Memo (0) | Bezeichnung 3 | Mehrsprachig |
| `P_ARTICLE_DISCOUNT` | Double (8B) | Rabatt |  |
| `P_ARTICLE_ECABINET_MACRO` | Text (255) | Textur | Beispiel: "C:\texture.jpg" oder "$(MD_IMAGES)\texture.jpg" |
| `P_ARTICLE_ERPNR` | Text (50) | ERP-Nr. |  |
| `P_ARTICLE_EXTERNAL_DOCUMENT_1 … _20` | Memo (0) | Externes Dokument 1–20 |  |
| `P_ARTICLE_EXTERNAL_PLACEMENT` | Bool (1B) | Externe Platzierung |  |
| `P_ARTICLE_FUNCTIONGROUP` | Text (30) | Funktionsgruppe |  |
| `P_ARTICLE_IDENTCODE` | Text (20) | Strichcode-Nummer |  |
| `P_ARTICLE_IDENTTYPE` | Text (20) | Strichcode-Typ |  |
| `P_ARTICLE_MACRO` | Text (255) | Grafikmakro (inkl. Verzeichnis) |  |
| `P_ARTICLE_GROUPNUMBER` | Text (10) | Gruppennummer |  |
| `P_ARTICLE_GROUPSYMBOLMACRO` | Text (255) | Makro | Beispiel: "C:\macro.ema" oder "$(MD_MACROS)\Macro.ema" |
| `P_ARTICLE_HEIGHT` | Double (8B) | Höhe |  |
| `P_ARTICLE_SPACING_ABOVE` | Double (8B) | Einbauabstand Höhe oberhalb |  |
| `P_ARTICLE_SPACING_BELOW` | Double (8B) | Einbauabstand Höhe unterhalb |  |
| `P_PART_LASTCHANGE` | Text (40) | Letzte Änderung |  |
| `P_ARTICLE_LIFETIME` | Text (16) | Lebensdauer |  |
| `P_ARTICLE_MAINTENANCE` | Text (16) | Schmierung / Wartung |  |
| `P_ARTICLE_MANUFACTURER` | Text (30) | Hersteller |  |
| `P_ARTICLE_MOUNTINGSITE` | Long (4B) | Montagefläche | Auswahlliste |
| `P_ARTICLE_MOUNTINGSPACE` | Double (8B) | Platzbedarf |  |
| `P_ARTICLE_NOTE` | Memo (0) | Beschreibung | Mehrsprachig |
| `P_ARTICLE_ORDERNR` | Text (255) | Bestellnummer |  |
| `P_ARTICLE_PACKAGINGPRICE_1` | Currency (8B) | Einkaufspreis 1/Verpackung |  |
| `P_ARTICLE_PACKAGINGPRICE_2` | Currency (8B) | Einkaufspreis 2/Verpackung |  |
| `P_ARTICLE_PACKAGINGQUANTITY` | Double (8B) | Menge/Verpackung |  |
| `P_ARTICLE_PICTUREFILE` | Text (255) | Bilddatei |  |
| `P_ARTICLE_PIECETYPE` | Text (10) | Teileart |  |
| `P_ARTICLE_PRICEUNIT` | Integer (2B) | Preiseinheit |  |
| `P_ARTICLE_PRODUCTTOPGROUP` | Integer (2B) | Produktobergruppe | Auswahlliste |
| `P_ARTICLE_PRODUCTGROUP` | Integer (2B) | Produktgruppe | Auswahlliste |
| `P_ARTICLE_PRODUCTSUBGROUP` | Integer (2B) | Produktuntergruppe | Auswahlliste |
| `P_ARTICLE_PURCHASEPRICE_1` | Currency (8B) | Einkaufspreis 1/Preiseinheit |  |
| `P_ARTICLE_PURCHASEPRICE_2` | Currency (8B) | Einkaufspreis 2/Preiseinheit |  |
| `P_ARTICLE_QUANTITYUNIT` | Memo (0) | Mengeneinheit | Mehrsprachig |
| `P_ARTICLE_REF_TERMINAL_OFFSET_X` | Double (8B) | Versatz X-Richtung (Anschlussbild) |  |
| `P_ARTICLE_REF_TERMINAL_OFFSET_Y` | Double (8B) | Versatz Y-Richtung (Anschlussbild) |  |
| `P_ARTICLE_REPORT_IDENTIFIER` | Text (20) | Kennung für Auswertungen |  |
| `P_ARTICLE_REPORT_SYMBOL` | Memo (0) | Symbol für Auswertungen |  |
| `P_ARTICLE_SALESPRICE_1` | Currency (8B) | Verkaufspreis 1 |  |
| `P_ARTICLE_SALESPRICE_2` | Currency (8B) | Verkaufspreis 2 |  |
| `P_ARTICLE_SNAPHEIGHT` | Double (8B) | Aufklipshöhe |  |
| `P_ARTICLE_SNAPHEIGHTMIDDLEOFFSET` | Double (8B) | Mittenversatz |  |
| `P_ARTICLE_INSTALLATION_DEPTH` | Double (8B) | Einbautiefe |  |
| `P_ARTICLE_SPARE` | Text (16) | Ersatzteil |  |
| `P_ARTICLE_STRESS` | Text (16) | Beanspruchung |  |
| `P_ARTICLE_SUBCRAFT_ELECTRICAL` | Memo (0) | Untergewerk "Elektrotechnik" | Mehrsprachig; eigene Untergewerke als `P_ARTICLE_SUBCRAFT_<Gewerk>-n` |
| `P_ARTICLE_SUBCRAFT_MECHANICS` | Memo (0) | Untergewerk "Mechanik" | Mehrsprachig |
| `P_ARTICLE_SUBCRAFT_HYDRAULICS` | Memo (0) | Untergewerk "Hydraulik" | Mehrsprachig |
| `P_ARTICLE_SUBCRAFT_PNEUMATICS` | Memo (0) | Untergewerk "Pneumatik" | Mehrsprachig |
| `P_ARTICLE_SUBCRAFT_LUBRICATION` | Memo (0) | Untergewerk "Schmierung" | Mehrsprachig |
| `P_ARTICLE_SUBCRAFT_COOLING` | Memo (0) | Untergewerk "Kühlung" | Mehrsprachig |
| `P_ARTICLE_SUBCRAFT_PROCESS` | Memo (0) | Untergewerk "Verfahrenstechnik" | Mehrsprachig |
| `P_ARTICLE_SUPPLIER` | Text (30) | Lieferant |  |
| `P_ARTICLE_REF_TERMINAL_NAME` | Text (40) | Anschlussbezeichnung ( Anschlüsse) |  |
| `P_ARTICLE_TERMINALSIZE_DESTINATION` | Text (255) | Anschlussmaß Ziel |  |
| `P_ARTICLE_TERMINALSIZE_SOURCE` | Text (255) | Anschlussmaß Quelle |  |
| `P_ARTICLE_TYPENR` | Text (255) | Typnummer |  |
| `P_ARTICLE_UNIQUEID` | Text (50) | Eindeutige Artikel- ID | Entstammt dem Data Portal. |
| `P_ARTICLE_USAGE` | Text (16) | Beschaffung |  |
| `P_ARTICLE_WEAR` | Text (16) | Verschleißteil |  |
| `P_ARTICLE_WEIGHT` | Double (8B) | Gewicht |  |
| `P_ARTICLE_WIDTH` | Double (8B) | Breite |  |
| `P_ARTICLE_SPACING_LEFT` | Double (8B) | Einbauabstand Breite links |  |
| `P_ARTICLE_SPACING_RIGHT` | Double (8B) | Einbauabstand Breite rechts |  |
| `P_ARTICLE_3DMACRO` | Text (255) | Grafikmakro (3D) | danach Mechanik-Zusatzattribut |
| `P_ARTICLE_HINGEPOSITION` | Long (4B) | Tür: Scharnier | Eigenschaften; Auswahlliste |

#### `<variant>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `P_ARTICLE_VARIANT` | Text (3) | Variante |  |
| `P_ARTICLE_ASSEMBLY_POS_PLACE_SPREADED` | Bool (1B) | Baugruppe verteilt platzieren | Baugruppe |
| `P_ARTICLE_CONNECTIONDESIGNATION` | Text (255) | Anschlussbezeichnung |  |
| `P_ARTICLE_SYMBOLFILE` | Text (30) | Symbolbibliothek |  |
| `P_ARTICLE_SYMBOLNUMBER` | Text (30) | Symbolnummer |  |
| `P_ARTICLE_CHARACTERISTICS` | Text (60) | Technische Kenngrößen |  |
| `P_ARTICLE_WIRECROSSSECTION_UNIT` | Integer (2B) | Einheit für | Verbindungsquerschnitt / -durchmesser Eigenschaften; Auswahlliste |

#### `<functiontemplate>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `functiondefcategory` | Integer (2B) | Kategorie (Funktionsdaten) | Auswahlliste |
| `functiondefgroup` | Integer (2B) | Gruppe (Funktionsdaten) | Auswahlliste |
| `functiondefid` | Integer (2B) | Funktionsdefinition (Funktionsdaten) | Auswahlliste; Kategorie.Gruppe.Id bilden zusammen die Funktionsdefinition |
| `additionaldescription` | Memo (0) | Beschreibung (Funktionsdaten) | Mehrsprachig |
| `characteristics` | Text (60) | Technische Kenngrößen | je Produktgruppe verschieden (s. Tabelle unten) |
| `connectioncrosssection` | Text (255) | Anschlussquerschnitt / -durchmesser |  |
| `connectionDesignation` | Text (255) | Anschlussbezeichnungen | je Produktgruppe verschieden (s. Tabelle unten) |
| `connectiondescription` | Text (255) | Anschlussbeschreibungen |  |
| `connectiondimension` | Text (255) | Anschlussmaß |  |
| `description` | Text (30) | Klemmen- / Steckerkontaktbeschreibung |  |
| `hasled` | Bool (1B) | Klemme mit LED |  |
| `hasplugadapter` | Bool (1B) | Klemme mit Steckadapter | Nur für Funktionsschablonen von Klemmen verfügbar. |
| `idx1` | Text (30) | Index 1 | je Produktgruppe verschieden (s. Tabelle unten) |
| `idx2` | Text (30) | Index 2 | je Produktgruppe verschieden (s. Tabelle unten) |
| `intrinsicsafety` | Bool (1B) | Eigensicher |  |
| `nesteddevicetag` | Text (30) | Untergeordnetes BMK |  |
| `pipeclass` | Text (20) | Rohrklasse |  |
| `plcbussystem` | Integer (2B) | Bus-System | Auswahlliste |
| `safetyrelevant` | Bool (1B) | Sicherheitsfunktion |  |
| `symbol` | Text (50) | Symbol |  |
| `Symbolmacro` | Text (255) | Symbolmakro |  |
| `terminalfunction` | Integer (2B) | Klemmenausführung | Auswahlliste |
| `terminallabeltype` | Text (20) | Beschriftungsart |  |
| `terminalNr` | Text (30) | Klemmennummer | je Produktgruppe verschieden (s. Tabelle unten) |

#### `<freeproperty>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position | Maximal 200 möglich! |
| `P_ARTICLE_FREE_DATA_DESCRIPTION` | Memo (0) | Freie Eigenschaften: Beschreibung | Mehrsprachig |
| `P_ARTICLE_FREE_DATA_UNIT` | Text (50) | Freie Eigenschaften: Einheit |  |
| `P_ARTICLE_FREE_DATA_VALUE` | Memo (0) | Freie Eigenschaften: Wert | Mehrsprachig |

#### `<blockingSurfacePosition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `frontside` | Bool (1B) | Vorderseite |  |
| `height` | Double (8B) | Höhe |  |
| `type` | Long (4B) | Typ | Auswahlliste |
| `width` | Double (8B) | Breite |  |
| `xpos` | Double (8B) | X-Position |  |
| `ypos` | Double (8B) | Y-Position |  |

#### `<doorPosition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `partnr` | Text (255) | Artikelnummer |  |
| `variant` | Text (3) | Variante |  |
| `xpos` | Double (8B) | X-Position |  |
| `ypos` | Double (8B) | Y-Position |  |
| `zpos` | Double (8B) | Z-Position |  |

#### `<mountingPanelPosition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `angle` | Double (8B) | Winkel |  |
| `location` | Long (4B) | Einbauort | Auswahlliste |
| `partnr` | Text (255) | Artikelnummer |  |
| `variant` | Text (3) | Variante |  |
| `xpos` | Double (8B) | X-Position |  |
| `ypos` | Double (8B) | Y-Position |  |
| `zpos` | Double (8B) | Z-Position |  |

#### `<supportBarPosition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `length` | Double (8B) | Länge |  |
| `partnr` | Text (255) | Artikelnummer |  |
| `variant` | Text (3) | Variante |  |
| `xpos` | Double (8B) | X-Position |  |
| `ypos` | Double (8B) | Y-Position |  |
| `zpos` | Double (8B) | Z-Position |  |

#### `<assemblyposition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `count` | Integer (2B) | Menge |  |
| `partnr` | Text (255) | Artikelnummer |  |
| `variant` | Text (3) | Variante |  |
| `posnr` | Text (50) | Positionsnummer |  |
| `additionaltext` | Text (50) | Zusatztext |  |

#### `<moduleposition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `count` | Integer (2B) | Menge |  |
| `devicetag` | Text (255) | BMK |  |
| `moduleid` | Text (16) | BMK-ID |  |
| `partnr` | Text (255) | Artikelnummer |  |
| `variant` | Text (3) | Variante |  |

#### `<attributeposition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `val` | Text (80) | Wert |  |

#### `<accessoryposition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `necessary` | Bool (1B) | Erforderlich |  |
| `partnr` | Text (255) | Artikelnummer |  |
| `pos` | Integer (2B) | Position |  |
| `variant` | Text (3) | Variante |  |
| `parttype` | Integer (2B) | Datensatztyp | Auswahlliste; nur "Einzelteil" und "Zubehörliste" werden unterstützt! |
| `placement` | Text (50) | Zubehörplatzierung |  |

#### `<accessorylist>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `class` | Integer (2B) | Typ | Es wird nur der Wert 13 unterstützt! |
| `P_PART_ACCESSORYLIST_CREATE` | Text (40) | Erstellungsdatum |  |
| `P_PART_ACCESSORYLIST_DESCRIPTION` | Memo (0) | Beschreibung |  |
| `P_PART_ACCESSORYLIST_LASTCHANGE` | Text (40) | Letzte Änderung |  |
| `P_PART_ACCESSORYLIST_NAME` | Text (40) | Name |  |

#### `<accessorylistposition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `partnr` | Text (255) | Artikelnummer |  |
| `pos` | Integer (2B) | Position |  |
| `variant` | Text (3) | Variante |  |
| `placement` | Text (50) | Zubehörplatzierung |  |

#### `<accessoryplacement>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `class` | Integer (2B) | Typ | Es wird nur der Wert 14 unterstützt! |
| `P_PART_ACCESSORYPLACEMENT_CREATE` | Text (40) | Erstellungsdatum |  |
| `P_PART_ACCESSORYPLACEMENT_DESCRIPTION` | Memo (0) | Beschreibung |  |
| `P_PART_ACCESSORYPLACEMENT_LASTCHANGE` | Text (40) | Letzte Änderung |  |
| `P_PART_ACCESSORYPLACEMENT_NAME` | Text (50) | Name |  |

#### `<accessoryplacementposition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `name` | Text (40) | Einbauvariante |  |
| `pos` | Integer (2B) | Position |  |
| `referencepoint` | Integer (2B) | Bezugspunkt |  |
| `rotation` | Double (8B) | Drehung |  |
| `xspacing` | Double (8B) | Versatz X-Richtung |  |
| `yspacing` | Double (8B) | Versatz Y-Richtung |  |
| `zspacing` | Double (8B) | Versatz Z-Richtung |  |
| `movable` | Bool (1B) | Verschiebbar |  |

#### `<construction>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `class` | Integer (2B) | Typ | Es wird nur der Wert 6 unterstützt! |
| `P_PART_CONSTRUCTION_CREATE` | Text (40) | Erstellungsdatum |  |
| `P_PART_CONSTRUCTION_DESCRIPTION` | Memo (0) | Beschreibung | Mehrsprachig |
| `P_PART_CONSTRUCTION_LASTCHANGE` | Text (40) | Letzte Änderung |  |
| `P_PART_CONSTRUCTION_NAME` | Text (40) | Name |  |

#### `<constructionPosition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `name` | Text (40) | Name |  |
| `offsetx` | Double (8B) | Versatz X-Richtung |  |
| `offsety` | Double (8B) | Versatz Y-Richtung |  |

#### `<drillingPosition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `angle` | Double (8B) | Winkel |  |
| `dimension1` | Double (8B) | 1. Dimension |  |
| `dimension2` | Double (8B) | 2. Dimension |  |
| `dimension3` | Double (8B) | 3. Dimension |  |
| `drillnthhole` | Long (4B) | Jedes n-te Loch bohren |  |
| `outlinename` | Text (255) | Konturname |  |
| `spacingend` | Double (8B) | Endabstand |  |
| `spacingrepeat` | Double (8B) | Wiederholungsabstand |  |
| `subtype` | Long (4B) | Subtyp | Auswahlliste |
| `type` | Long (4B) | Bohrtyp | Auswahlliste |
| `xpos` | Double (8B) | X-Position |  |
| `ypos` | Double (8B) | Y-Position |  |
| `fabricatealways` | Bool (1B) | Immer fertigen |  |

#### `<terminal>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `class` | Integer (2B) | Typ | Es wird nur der Wert 7 unterstützt! |
| `P_PART_TERMINAL_ADDITIONALLENGTHDEFAULT` | Double (8B) | Zusatzlänge |  |
| `P_PART_TERMINAL_CREATE` | Text (40) | Erstellungsdatum |  |
| `P_PART_TERMINAL_DESCRIPTION` | Memo (0) | Beschreibung | Mehrsprachig |
| `P_PART_TERMINAL_DIRECTION` | Long (4B) | Verlegerichtung | Auswahlliste |
| `P_PART_TERMINAL_LASTCHANGE` | Text (40) | Letzte Änderung |  |
| `P_PART_TERMINAL_NAME` | Text (40) | Name |  |
| `P_PART_TERMINAL_TYPEDEFAULT` | Long (4B) | Verbindungsende-Behandlung | (EPLAN Cabinet) Auswahlliste |
| `P_PART_TERMINAL_TERMINALSIZE_DEFAULT` | Text (20) | Anschlussmaß |  |
| `P_PART_TERMINAL_TYPEOFTERMINAL_DEFAULT` | Long (4B) | Anschlussausführung | Auswahlliste |

#### `<terminalPosition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `additionallength` | Double (8B) | Zusatzlänge |  |
| `devicetag` | Text (255) | Steckerbezeichnung |  |
| `direction` | Long (4B) | Verlegerichtung | Auswahlliste |
| `level` | Integer (2B) | Etage |  |
| `maxcrosssection` | Double (8B) | Max. Querschnitt / Durchmesser |  |
| `maxwirecount` | Long (4B) | Max. Anzahl Verbindungen |  |
| `mincrosssection` | Double (8B) | Min. Querschnitt / Durchmesser |  |
| `maxcrosssectionawg` | Text (20) | Max. AWG |  |
| `mincrosssectionawg` | Text (20) | Min. AWG |  |
| `name` | Text (20) | Anschlussbezeichnung |  |
| `targetinfo` | Integer (2B) | Intern- / Extern-Index |  |
| `terminalsize` | Text (20) | Anschlussmaß |  |
| `twinsleeve` | Bool (1B) | Doppelhülse vorgeschrieben |  |
| `typeofterminal` | Long (4B) | Anschlussausführung | Auswahlliste |
| `xpos` | Double (8B) | X-Position |  |
| `ypos` | Double (8B) | Y-Position |  |
| `zpos` | Double (8B) | Z-Position |  |
| `xdir` | Double (8B) | X-Vektor |  |
| `ydir` | Double (8B) | Y-Vektor |  |
| `zdir` | Double (8B) | Z-Vektor |  |

#### `<safetyRelatedValuePosition>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `pos` | Integer (2B) | Position |  |
| `posid` | Integer (2B) | Positions-ID |  |
| `hierarchy1` | Memo (0) | Ebene 1 (Text in der Baumstruktur) | Mehrsprachig |
| `hierarchy2` | Memo (0) | Ebene 2 (Text in der Baumstruktur) | Mehrsprachig |
| `hierarchy3` | Memo (0) | Ebene 3 (Text in der Baumstruktur) | Mehrsprachig |
| `hierarchy4` | Memo (0) | Ebene 4 (Text in der Baumstruktur) | Mehrsprachig |
| `hierarchy5` | Memo (0) | Ebene 5 (Text in der Baumstruktur) | Mehrsprachig |
| `input` | Bool (1B) | Eingang (Erfassen) |  |
| `logic` | Bool (1B) | Logik (Auswerten) |  |
| `output` | Bool (1B) | Ausgang (Reagieren) |  |
| `pl` | Text (30) | PL |  |
| `silcl` | Text (30) | SIL CL |  |
| `pfhd` | Double (8B) | PFHD |  |
| `tmt1` | Double (8B) | TMT1 |  |
| `mttfd` | Double (8B) | MTTFD |  |
| `lamdad` | Double (8B) | Lambda-d |  |
| `mttf` | Double (8B) | MTTF |  |
| `mtbf` | Double (8B) | MTBF |  |
| `rdf` | Double (8B) | RDF |  |
| `b10` | Double (8B) | B10 |  |
| `b10d` | Double (8B) | B10 D |  |

#### `<address>`

| Attribut | Typ (Länge) | Feld | Hinweis |
|---|---|---|---|
| `class` | Integer (2B) | Typ | Auswahlliste; nur "Kunde" und "Hersteller / Lieferant" werden unterstützt! |
| `P_PART_ADDRESS_EMAIL` | Text (70) | E-Mail |  |
| `P_PART_ADDRESS_FAX` | Text (30) | Telefax |  |
| `P_PART_ADDRESS_LASTCHANGE` | Text (40) | Letzte Änderung |  |
| `P_PART_ADDRESS_LONGNAME` | Text (70) | Langname |  |
| `P_PART_ADDRESS_NAME1` | Text (40) | Name 1 |  |
| `P_PART_ADDRESS_NAME2` | Text (40) | Name 2 |  |
| `P_PART_ADDRESS_NAME3` | Text (40) | Name 3 |  |
| `P_PART_ADDRESS_NOTE` | Memo (0) | Beschreibung |  |
| `P_PART_ADDRESS_NUMBER` | Text (30) | Kundennummer |  |
| `P_PART_ADDRESS_PHONE` | Text (50) | Telefon |  |
| `P_PART_ADDRESS_POBOX` | Text (30) | Postfach |  |
| `P_PART_ADDRESS_SHORTNAME` | Text (30) | Kurzname |  |
| `P_PART_ADDRESS_STATE` | Text (30) | Land |  |
| `P_PART_ADDRESS_STREET` | Text (100) | Straße |  |
| `P_PART_ADDRESS_TITLE` | Text (30) | Anrede |  |
| `P_PART_ADDRESS_TOWN` | Text (100) | Wohnort |  |
| `P_PART_ADDRESS_ZIPPOBOX` | Text (30) | PLZ / Postfach |  |
| `P_PART_ADDRESS_ZIPTOWN` | Text (30) | PLZ / Wohnort |  |


## 4. functiontemplate: Bedeutung je Produktgruppe

Mehrere `<functiontemplate>`-Attribute ändern ihre Bedeutung mit der Produktgruppe des Artikels (– = nicht benutzt):

| Attribut | Allgemeine | Kabel | Relais, Schütze | Stecker | Klemmen | SPS |
|---|---|---|---|---|---|---|
| `characteristics` | Technische Kenngrößen | **Potenzialtyp** (Auswahlliste) | Technische Kenngrößen | – | – | – |
| `connectionDesignation` | Anschlussbezeichnungen | **Verbindungsfarbe/-nummer** (Ader) | Anschlussbezeichnungen | Anschlussbezeichnungen | Anschlussbezeichnungen | Anschlussbezeichnungen |
| `idx1` | – | **Abgeschirmt von** | **Kontakt-/Spulenindex** | – | **Etage** | **Kanalbezeichnung** |
| `idx2` | – | **Paarindex** | – | – | – | – |
| `terminalNr` | – | **Verbindungsquerschnitt/-durchmesser** | – | Klemmen-/Steckerkontaktbezeichnung | Klemmen-/Steckerkontaktbezeichnung | **Steckerbezeichnung** |

Weitere: `hasplugadapter` nur für Klemmen-Schablonen; `terminalfunction` (Klemmenausführung) nur Klemmen; `nesteddevicetag` = untergeordnetes BMK; `symbol`/`Symbolmacro` = Symbol bzw. Symbolmakro der Funktion; `plcbussystem` = Bus-System (Auswahlliste). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_tags.htm, https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_sellists.htm

## 5. Funktionsdefinitionen: Format und wichtigste Kennungen

- Die Funktionsdefinition einer Schablone steht in drei Integer-Attributen `functiondefcategory`, `functiondefgroup`, `functiondefid`; geschrieben als **Kategorie.Gruppe.Definition**, z.B. `200.1.1`. Die Tabelle ordnet jede Kennung einem Gewerk, Bereich, einer Kategorie, Gruppe und Definition zu; 1353 Einträge in 2.9. Einträge „(nicht mehr verwenden)“ sind unten mit „(alt)“ markiert. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_funcdef.htm
- Kategorie-Nummernbereiche (Gewerk → Bereich): 4–22 Allgemeine Sonderfunktionen (Verbindung 4, Artikelplatzierung 8, Isoliertes Verbindungsende 15, Topologie 21, Kabelbaum 22); **100–106** Klemmen und Stecker; **200–204** Spulen, Kontakte, Schutzbeschaltung; **300–301** SPS/Bus; 400/401/430 Kabel, Abschirmung, Antenne; 500–507 Schutzeinrichtungen; 600–606 Sensorik/Schalter/Taster; 700–701 Spannungsquelle/Generator; 800–802 Trafo/Umformer/Gleichrichter; 900–904 Meldeeinrichtungen/Messgeräte; 1000–1002 Motor/Leuchte/Heizung; 1100–1102 Ventil/Bremse/Kupplung; 1201–1211 Elektronik/Logik; 1301–1306 Sonderfunktionen (Gerätekasten 1303, Sammelschiene 1304); 1400 Leitungsverteiler; 5000–6201 Fluid; 10000 ff. Verfahrenstechnik; 15000 ff. Mechanik. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_funcdef.htm
- Die Eigenschaft **Funktionsdefinition: Kategorie/Gruppe/Definition** (ID 20188) trägt diese Kennung an Funktionen; Funktionsdefinition an Funktionsschablonen (ID 21004). Quelle: `referenz/eigenschaften-ids.md`

**Wichtigste Kennungen für den Bauteil-Import** (Kategorie → Gruppen; vollständig im Anhang):

| Kat. | Kategorie | Gruppen (`Kat.Gruppe` Name) | QET-Abbildung (Empfehlung) |
|---|---|---|---|
| 4 | Verbindung | `4.1` Verbindungsdefinition | Verbindung/Ader/Brücke (kein Element; QET-Leiter bzw. Brücke) |
| 100 | Klemme | `100.2` Klemme, 2 Anschlüsse (alt); `100.3` Klemmen, 1 Anschluss; `100.4` Klemmen mit Steg, 1 Anschluss; `100.5` Klemmen, 2 Anschlüsse; `100.6` Klemmen mit Steg, 2 Anschlüsse; `100.7` Klemmen, 3 Anschlüsse; `100.8` Klemmen mit Steg, 3 Anschlüsse; `100.9` Klemmen, 4 Anschlüsse; `100.10` Klemmen mit Steg, 4 Anschlüsse; `100.11` Klemmen, 6 Anschlüsse; `100.12` Klemmen mit Steg, 6 Anschlüsse; … | QET `terminal`-Element (Klemme) |
| 101 | Steckerkontakt | `101.2` Steckerkontakt, 2 Anschlüsse; `101.98` Grafik; `101.99` Steckerkontakt, variabel | Steckerkontakt (Stift/Buchse) → Element mit Anschluss |
| 102 | Klemmenleistendefinition | `102.1` Klemmenleistendefinition | Klemmenleistendefinition (kein Symbol, Leistendaten) |
| 103 | Steckerdefinition | `103.1` Steckerdefinition | Steckerdefinition (Stecker-Hauptfunktion) |
| 104 | Trennklemme | `104.1` Trennklemme, 2 Anschlüsse; `104.2` Trennklemme, 3 Anschlüsse; `104.3` Trennklemme, 4 Anschlüsse; `104.98` Grafik; `104.99` Trennklemme, variabel | Trennklemme → terminal |
| 200 | Spule | `200.1` Spule, 2 Anschlüsse; `200.2` Spule mit Sonderansteuerung, 3 Anschlüsse; `200.3` Spule mit Sonderansteuerung, 4 Anschlüsse; `200.98` Grafik; `200.99` Spule, variabel | Spule → **Master** |
| 201 | Schließer | `201.1` Schließer, 2 Anschlüsse; `201.2` Zweifachschließer, 4 Anschlüsse; `201.3` Dreifachschließer, 6 Anschlüsse; `201.98` Grafik; `201.99` Schließer, variabel | Schließer → **Slave** |
| 202 | Öffner | `202.1` Öffner, 2 Anschlüsse; `202.2` Zweifachöffner, 4 Anschlüsse; `202.3` Dreifachöffner, 6 Anschlüsse; `202.98` Grafik; `202.99` Öffner, variabel | Öffner → **Slave** |
| 203 | Wechsler | `203.1` Wechsler, 3 Anschlüsse; `203.98` Grafik; `203.99` Wechsler, variabel | Wechsler → **Slave** |
| 204 | Schutzbeschaltung | `204.1` Schutzbeschaltung, 2 Anschlüsse; `204.98` Grafik; `204.99` Schutzbeschaltung, variabel | Schutzbeschaltung |
| 300 | SPS-Anschluss | `300.1` SPS-Anschluss, E / A, 1 Anschluss; `300.2` SPS-Anschluss, Kartenversorgung, 1 Anschluss; `300.3` SPS-Anschluss, Anschlussversorgung, 1 Anschluss; `300.4` SPS-Anschluss, Netzwerk- / Bus-Kabel; `300.5` SPS-Anschluss, Netzwerk- / Bus-Kabel, 2 Anschlüsse; `300.6` SPS-Anschluss, 2 Anschlüsse; `300.7` SPS-Anschluss, intern; `300.98` Grafik; `300.99` SPS-Anschluss, variabel | SPS-Anschluss → Kanal/Pin (Slave des SPS-Kastens) |
| 301 | SPS-Kasten | `301.1` SPS-Kasten | SPS-Kasten → **Master** (Karte/Modul) |
| 400 | Kabel | `400.1` Kabel | Kabel (Kabeldefinition) |
| 401 | Abschirmung | `401.1` Abschirmung | Abschirmung |
| 1302 | Geräteanschluss | `1302.1` Geräteanschluss; `1302.2` Geräteanschluss, 2 Anschlüsse; `1302.98` Grafik; `1302.99` Geräteanschluss, variabel | Geräteanschluss |
| 1303 | Gerätekasten | `1303.1` Gerätekasten | Gerätekasten (Black Box) → Master |


## 6. Produktgruppen und Auswahllisten

Format: interner Wert, Anzeigewert. Lücken in der Nummerierung sind so in der Hilfe. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_productgroups.htm (Produktgruppen), https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_sellists.htm (übrige Listen)



**`part: P_ARTICLE_PRODUCTTOPGROUP (Produktobergruppe)`**: 0 Undefiniert · 1 Elektrotechnik · 2 Fluid · 3 Mechanik · 4 Verfahrenstechnik

**`part: P_ARTICLE_PRODUCTGROUP (Produktgruppe)`**: 0 Undefiniert · 1 Allgemeine · 2 Relais, Schütze · 3 Klemmen · 4 Stecker · 5 Umformer · 6 Schutzeinrichtungen · 7 Röhren, Halbleiter · 8 Meldeeinrichtungen · 9 Motoren · 10 Messgeräte, Prüfeinrichtungen · 11 Widerstände · 12 Sensorik, Schalter und Taster · 13 Transformatoren · 14 Modulatoren · 15 Elektrisch betätigte mechanische Einrichtungen · 16 Elektrotechnik Sonderbauteile · 17 Verschiedenes · 18 Kondensatoren · 19 Logikbauteile · 20 Spannungsquelle und Generator · 21 Induktivitäten · 22 Verstärker, Regler · 23 Starkstrom-Schaltgeräte · 24 Abschlüsse, Filter · 25 Übertragungswege · 26 SPS · 29 Kabel · 30 Aggregate und Anlagen · 32 Aktoren, allgemein · 33 Fluidmotor · 34 Filter · 35 Fluid Control Terminal · 36 Kupplungen · 37 Verbindungen · 38 Messanschlüsse · 39 Messgeräte · 40 Pumpen · 41 Signalaufnehmer · 42 Fluid Sonderbauteile · 43 Speicher · 44 Ventile · 46 Wärmetauscher · 47 Zubehör · 48 Anschlussplatten · 49 Gehäuse · 50 Gehäusezubehör Außenanbau · 51 Gehäusezubehör Innenanbau · 52 Verschlusssysteme · 53 Kabelkanäle · 54 Sammelschienen · 55 Schaltschrank · 56 Abzugshaube · 57 Elektrolysezelle · 58 Lagerung · 59 Schornstein · 61 Antriebsmaschine · 62 Absperrarmatur · 63 Dreiwegearmatur · 64 Rückschlagarmatur · 65 Behälter · 66 Behälteranschluss · 67 Abscheider · 68 Filter · 69 Sieb · 70 Förderer · 71 Heber · 72 Transporter · 73 Behälter-Mantel · 74 Behälter-Rohrschlange · 75 Dampfkessel · 76 Kühler · 77 Ofen · 78 Trockner · 79 Verdampfer · 80 Wärmetauscher · 81 Kneter · 82 Mischer · 83 Rührer · 84 Kompressor / Verdichter · 85 Pumpen · 86 Vakuumpumpe · 87 Ventilator · 89 Rohrleitungsteil · 91 Zentrifuge · 92 Messeinrichtung · 93 Waage · 94 Formgeber · 95 Sichter · 96 Sortierer · 97 Zerkleinerer · 98 Zuteiler · 99 Feldverteiler · 100 Verbindungen · 101 Montageplatten · 102 Heizung · 103 Leuchte · 104 Heizung · 105 Zylinder · 106 Bremse · 107 Gleichrichter · 108 Bremse · 109 Kupplungen · 110 Ventile · 111 Druckübersetzer · 112 Abscheider · 113 Leitungsverteiler / -verbinder · 114 Zubehör · 115 Meldeeinrichtungen · 116 Benutzerdefinierte Schiene · 117 19 Zoll-Ausbautechnik · 118 Strecke ( Topologie) · 119 Verlegepunkt (Topologie) · 121 Verbindungen · 125 Verlegezubehör · 126 Montageanordnung · 127 Leitungen / Pakete · 128 Leitungen / Pakete · 129 Leitungsverteiler / -verbinder · 130 Leitungsverteiler / -verbinder

**`part: P_ARTICLE_PRODUCTSUBGROUP (Produktuntergruppe)`**: 0 Undefiniert · 1 Allgemeine · 2 Schütze · 3 Hilfsblock · 4 Klemme · 5 Tragschiene · 6 Endwinkel · 7 Klemmenschild · 8 Weitere Schilder · 9 Leistenschild · 10 Querverbinder · 11 Schienen · 12 Abdeckung · 13 Abschluss · 14 Trennwand · 15 Steckergehäuse · 16 Steckerzubehör · 17 Werkzeug · 18 Testzubehör · 19 Artikel (Quelle) · 20 Artikel (Ziel) · 47 Hochdruckanlagen · 48 Hochdrucktanks · 49 Hydrauliktanks · 50 Komplettaggregate · 51 Kühlaggregate · 52 Schaltschrankkühlgeräte · 53 Schraubenspindelkompressoren · 54 Scrollkompressoren · 55 Wärmetauscher · 56 Zentralschmieraggregate · 57 Anschlussplatten · 58 Abdeckplatten · 59 Adapterplatten · 60 Blockkugelhähne · 61 Flansche · 62 Kugelhähne · 63 Reduzierplatten · 64 Reihenanschlussplatten · 65 Steuerblöcke · 66 Zwischenplatten · 67 Hydromotoren · 68 Schwenkantriebe · 69 Zylinder · 71 Belüftungsfilter · 72 Doppelumschaltfilter · 73 Druckluftfilter · 74 Einfachfilter · 75 Leitungsfilter · 76 Rücklauffilter · 77 Rückspülfilter vollautomatisch · 79 Kupplung-Motorpumpen · 80 Messkupplungen · 81 Verschlusskupplungen · 82 Schläuche · 83 Verschiedenes · 85 Manometer · 86 Thermometer · 87 Kompressoren · 88 Konstantpumpen · 89 Kreiselpumpen · 90 Schraubenspindelpumpen · 91 Verstellpumpen · 92 Druckschalter · 93 Füllstandsschalter · 94 Signalaufnehmer-Manometer · 95 Näherungsschalter · 96 Strömungswächter · 97 Temperatursensoren · 99 Absperrspeicher · 100 Druckspeicher · 101 Kolbenspeicher · 102 Sicherheitsspeicher · 103 Speicherblöcke · 104 Druckübersetzer · 105 Druckventile · 106 Entlüftungsventile · 107 Kolbenverteiler · 108 Kugelhahn pneumatisch · 109 Manometerschutzventile · 110 Manometerwahlschalter · 111 Progressivverteiler · 112 Rückschlagventile · 113 Sperrventile · 114 Stromventile · 115 Stromverteilerventile · 116 Thermostatventile · 117 Wegekoaxialventile · 118 Wegeventile · 121 Kupplungen · 122 Pumpenträger · 123 Schalldämpfer · 124 Tanks · 126 Verschraubungen · 127 Wasserabscheider · 128 Zeichnungsbehälter · 130 Tür · 131 Rück- / Seitenwand · 132 Dach · 133 Boden · 134 Sockel · 135 Montageplatten · 136 Zubehör · 139 Kabelabfangschienen · 140 Tragschiene · 141 Gleitschienen · 142 C-Profilschienen · 146 System · 147 Halter · 148 Adapter · 150 Einzelteil · 151 Korpus · 152 SPS-Eingangskarte analog · 153 SPS-Eingangskarte digital · 154 SPS-Ausgangskarte analog · 155 SPS-Ausgangskarte digital · 156 Tanks · 157 Bunker · 158 Silo · 159 Kolonne · 160 Reaktor · 161 Absperrarmatur · 162 Absperrventil · 163 Absperrklappe · 164 Rückschlagarmatur · 165 Rückschlagventil · 166 Rückschlagklappe · 167 Dreiwegearmatur · 168 Dreiwegeventil · 169 Dreiwegeklappe · 170 Drosselscheibe · 171 Blindscheibe · 172 Trichter · 173 Auslass · 174 Schalldämpfer · 175 Schauglas · 176 Mischdüse · 177 Schmutzfänger · 178 Kondensatableiter · 179 Flanschpaar · 180 Reduzierung · 181 Kupplungen · 182 Syphon · 183 Presse · 184 Vorkonfektioniert · 185 Anzeigegerät · 186 Meldeleuchte · 187 Messwertschreiber · 188 Meldeeinrichtung, akustisch · 189 Fehlerspannungs-Schutzschalter · 190 Fehlerstrom-Schutzschalter · 191 Motorschutzschalter · 192 Schmelzsicherung · 193 Schutzschalter · 194 Sicherungsautomat · 195 Meldeeinrichtung, akustisch · 196 Meldeeinrichtung, optisch · 197 Messwertschreiber · 198 Logikbauteil · 199 Regler · 200 Ventil / Verteiler · 201 Schutzrelais · 202 Analoger Sensor · 203 Digitaler Sensor, allgemein · 204 Endschalter · 205 Grenzwertgeber · 206 Lichtschranke · 207 Näherungsschalter · 208 Schalter / Taster · 209 Generator · 210 Spannungsquelle · 211 Sammelschienen-Abdeckung · 212 Schwenkrahmen · 213 Montage- Chassis · 214 Montageschiene · 215 Stifteinsatz · 216 Buchseneinsatz · 217 Rohre · 218 Schaltsperre · 219 Schaltbrücke · 220 Steckaufsatz · 221 Schiene · 223 Schläuche · 224 Rohre · 225 Kabel- / Schlauchbefestigung · 226 Kabel- / Schlauchbinder · 227 Kabel- / Schlauchschutz · 228 Flachbandkabel · 229 Crimpkontakt · 230 Kontakteinsatz



**`functiontemplate: characteristics (Kenngröße)`**: 0 Undefiniert · 1 L · 2 N · 3 PE · 4 + · 5 M · 6 - · 7 SH · 8 PEN
  
  _Hinweis_: bei Kabeln/Verbindungen = Potenzialtyp.

**`functiontemplate: terminalfunction (Klemmenausführung)`**: 0 Durchgangsklemme · 1 Trennklemme · 2 Schaltklemme · 3 Diodenklemme · 4 Sicherungsklemme · 5 Widerstandsklemme · 6 Thermoelementklemme
  
  _Hinweis_: nur bei Klemmen.

**`functiontemplate: plcbussystem (Bus-System)`**: 0 Andere Bus-Systeme · 1 CANbus · 2 DeviceNet · 3 EtherCAT · 4 Ethernet · 5 Foundation Fieldbus · 6 Interbus · 7 Lightbus · 8 Local-Bus: Baugruppenträger / SPS-Karte · 9 ModBus · 10 MPI-Bus · 11 Profibus · 12 Profinet · 13 ASI · 14 Local-Bus: Erweiterung · 15 X2X · 16 Powerlink · 17 CC-Link · 18 ControlNet · 19 DALI · 20 EIB · 21 EtherNet/IP · 22 FIPIO · 23 LON · 24 RS232/RS485 · 25 SERCOS interface · 26 USB · 27 IPLink · 28 Axioline · 29 DIOBus · 30 RIOBus · 31 ERIOBus · 32 RIO/DIO-Bus · 33 MDI I/O-Bus · 34 VARAN · 35 CC-Link IE · 36 Modbus-TCP · 37 SERCOS III · 38 DRIVE-CLiQ · 39 SSCNET III · 40 CC-Link IE Control · 41 CC-Link IE Field · 42 IO-Link · 43 SSI · 44 ET-Connection · 45 CC-Link IE Field Basic · 46 PortToPort

**`part : Montageort`**: 0 Nicht definiert · 1 Montageplatte · 2 Schwenkrahmen · 3 Tür · 4 Seitenteil · 5 Dach

**`part : Tür: Scharnier`**: 0 Links · 1 Rechts

**`variant : Einheit: Aderquerschnitt / Durchmesser`**: 0 von Projekt · 1 mm² · 2 qmm · 3 AWG · 4 mm · 5 KCM · 6 MCM · 7 Zoll · 8 " · 9 inch · 10 µm · 11 kcmil · 12 µm²

**`terminal : Anschlussrichtung`**: 0 Automatisch · 1 Nach oben · 2 Nach unten · 3 Nach links · 4 Nach rechts

**`terminal : Anschlussausführung (Standard) / terminalPosition : Anschlussausführung`**: 0 Undefiniert · 1 Schraubklemmanschluss einfach · 2 Schraubklemmanschluss zweifach · 3 Federklemmanschluss · 4 Schneidklemmanschluss · 5 Schraubanschluss (Ring- oder U-Kabelschuh) · 6 Steckanschluss (Kabelschuh) · 7 Steckeranschluss · 8 Lötanschluss · 9 Steckverbinderanschluss (Fluid) · 10 Nippel · 11 Innengewinde · 12 Außengewinde · 13 Überwurfmutter · 14 Schneidringverschraubung · 1015 Schraubanschluss · 1016 Zugfederanschluss · 1017 Push-in Anschluss · 1018 Durchdringanschluss · 1019 Steckanschluss · 1020 Steckklemmanschluss · 1021 Bolzenanschluss · 1022 Flachsteckanschluss · 1023 Crimpanschluss · 1024 Bandanschluss · 1025 Schienenanschluss · 1026 Lötfahnenanschluss · 1027 Wickelanschluss

**`part : Typ`**: 1 Einzelteil · 4 Baugruppe · 5 Modul

**`terminal : Typ (Standard)`**: 0 Undefiniert · 1 Schneiden · 2 Abisolieren · 3 Crimpen · 4 Sonstiges  
  _Hinweis_: eigene Werte ab 5 in der Datenbanktabelle `tblLookup`, Schlüssel `TerminalType` (Felder `keyName`, `nr` = interner Wert, `name` = mehrsprachiger Anzeigewert); Beispiele der Hilfe: 5 Doppelhülse, 6/7 Flachstecker schmal/breit, 8–11 Kabelschuh M4.

**`blockingSurfacePosition : Typ`**: 0 Platziersperrfläche · 1 Bohrsperrfläche

**`mountingPanelPosition : Einbauort`**: 0 Hinten · 1 Links · 2 Rechts

**`drillingPosition : Bohrtyp`**: 0 Undefiniert · 1 Bohren · 2 Gewinde · 3 Rechteckig · 4 Langloch · 5 Sechseck · 6 Achteck · 7 Benutzerdefinierte Kontur

**`drillingPosition : Subtyp`**: 0 Undefiniert · 1 Eckig · 2 Gefast · 3 Gerundet

**`terminalPosition : Kontaktart`**: wie `terminal : Typ (Standard)`

**`terminalPosition : Richtung`**: 0 Automatisch · 1 Nach oben · 2 Nach unten · 3 Nach links · 4 Nach rechts

**`address : Typ`**: 11 Kunde · 12 Hersteller / Lieferant

**`functiontemplate : idx1 (Bauteil-Ids (3d) für Funktionsdefinition 15000.3.1)`**: 2 Rahmenprofil vertikal links vorne · 3 Rahmenprofil vertikal links hinten · 4 Rahmenprofil vertikal rechts hinten · 5 Rahmenprofil vertikal rechts vorne · 6 Rahmenprofil horizontal · 133 Rahmenprofil - allgemein · 137 Rahmenprofil horizontal Dach · 138 Rahmenprofil horizontal Boden

**`functiontemplate : idx1 (Bauteil-Ids (3d) für Funktionsdefinition 15100.4.4)`**: 7 Seitenwand links · 8 Seitenwand rechts · 9 Boden · 10 Dachblech · 11 Rückwand · 12 Wand allgemein

**`functiontemplate : idx1 (Bauteil-Ids (3d) für Funktionsdefinition 15100.6.1)`**: 14 Tür · 15 Tür links · 16 Tür rechts

**`functiontemplate : idx1 (Bauteil-Ids (3d) für Funktionsdefinition 16000.1.2)`**: 10001 Montagefläche · 10002 Montageplatte Vorderseite · 10003 Montageplatte Rückseite · 10004 Tür außen · 10005 Tür innen · 10006 Tür links außen · 10007 Tür links innen · 10008 Tür rechts außen · 10009 Tür rechts innen · 10010 Seitenwand links außen · 10011 Seitenwand links innen · 10012 Seitenwand rechts außen · 10013 Seitenwand rechts innen · 10014 Dach außen · 10015 Dach innen · 10016 Boden außen · 10017 Boden innen · 10018 Rückwand außen · 10019 Rückwand innen · 10020 Wand außen · 10021 Wand innen · 10022 Profilebene · 10023 Profilebene links innen · 10024 Profilebene rechts innen · 10025 Profilebene hinten innen · 10026 Profilebene vorn innen · 10027 Profilebene Boden innen · 10028 Profilebene Dach innen · 10029 Profilebene links außen · 10030 Profilebene rechts außen · 10031 Profilebene hinten außen · 10032 Profilebene vorn außen · 10033 Bodenblech innen · 10034 Bodenblech außen · 10035 Profilebene links innen vertikal · 10036 Profilebene rechts innen vertikal · 10037 Profilebene hinten innen vertikal · 10038 Profilebene vorn innen vertikal · 10039 Montagefläche NC vorn · 10040 Fläche Bodenbleche · 10041 Profilebene Tür · 10042 Profilebene Tür links · 10043 Profilebene Tür rechts · 10044 Flanschplatte innen · 10045 Flanschplatte außen · 10046 19 Zoll-Profilebene · 10047 Montagefläche Kupfer · 10049 Verdrahtungskamm Vorderseite · 10050 Verdrahtungskamm Rückseite

**`functiontemplate : idx1 (Bauteil-Ids (3d) für Funktionsdefinition 16001.1.3)`**: 1000 Rechteck eckig · 1001 Rechteck gerundet · 1002 Rechteck gefast


- Beispiel aus einer EDZ: Hilfsschalterblock mit `P_ARTICLE_PRODUCTTOPGROUP="1"` (Elektrotechnik), `P_ARTICLE_PRODUCTGROUP="23"` (Starkstrom-Schaltgeräte), `P_ARTICLE_PRODUCTSUBGROUP="1"` (Allgemeine). Die Untergruppen-Nummer ist **global**, nicht je Produktgruppe (z.B. 152–155 SPS-Karten E/A analog/digital).

## 7. Bedeutung für QET (Fork-Import)

Stand des Fork-Codes (gelesen, nicht geändert): `sources/import/edz/edzpart.cpp` liest das `part.xml` der EDZ, `sources/import/eplandxf/eplanpartimporter.cpp` verbindet EDZ, Data-Portal-DXF-Pakete und Makros, `eplanmacroreader.cpp` liest `.ema` (internes PXF-XML, **nicht** dieses Artikelformat).

**Was der Code aus dem Artikel-XML benutzt** (`edzpart.cpp`):
- `<part>`: `P_ARTICLE_PARTNR`, `P_ARTICLE_MANUFACTURER`, `P_ARTICLE_ORDERNR`, `P_ARTICLE_TYPENR`, `P_ARTICLE_DESCR1` (Name, mehrsprachig), `P_ARTICLE_DESCR2` (Kommentar), `P_ARTICLE_PICTUREFILE`.
- `<functiontemplate>` (alle, per `elementsByTagName` unabhängig von der Verschachtelung): `connectionDesignation` (→ Pin), `connectiondescription`, `terminalNr` (Gruppierung), Fallback `functiondefinition` (Textattribut am Tag oder Elternelement).
- Mehrsprachige Texte: Regex `([a-z?]{2}_[A-Z?]{2})@([^;]*)(;|$)`, bevorzugt `de_DE`, dann `en_US` — passt zur Hilfe inkl. `??_??@`.

**Abweichungen zur Hilfe / Verbesserungen** (Empfehlungen, nicht umgesetzt):
1. `connectionDesignation` ist laut Hilfe eine **zeilenumbruch-getrennte Liste**; `edzpart.cpp` macht daraus genau **einen** Pin (nur `trimmed()`). Eine Spulen-Schablone mit `A1⏎A2` ergäbe einen Pin „A1 A2“. Fix: an `\n` (und `\r\n`) splitten, `connectiondescription` gleich splitten und paarweise zuordnen.
2. `terminalNr` heißt je Produktgruppe Verschiedenes (Abschnitt 4): bei SPS = Steckerbezeichnung (Gruppierung richtig), bei Klemmen/Steckern = Kontaktbezeichnung, bei **Kabeln = Querschnitt** (Gruppierung falsch). Gruppierung nur für Produktgruppe 26 (SPS) bzw. 4 (Stecker) per `terminalNr`, sonst nicht.
3. Ein Attribut `functiondefinition` (Text) kennt die 2.9-Hilfe nicht; dort gibt es nur `functiondefcategory/-group/-id` (Vermutung: Fallback stammt aus einem anderen/älteren Exportformat). Besser die drei Zahlen lesen und über Abschnitt 5 einordnen:
   - `200.x.x` Spule, `301.x.x` SPS-Kasten, `1303.x.x` Gerätekasten → **Master**-Element (Hauptfunktion);
   - `201`–`203` Schließer/Öffner/Wechsler → **Slave** (Kontakt), Index aus `idx1` (Kontakt-/Spulenindex);
   - `100`/`104` Klemme, `101` Steckerkontakt → QET-`terminal`-Element, Etage aus `idx1`;
   - `300.1.x` SPS-Anschluss E/A (DE/DA/AE/AA), `300.2/300.3` Karten-/Anschlussversorgung → Kanal bzw. Versorgungspin; Kanalbezeichnung aus `idx1`. Das deckt die Hausregel „SPS-Kanal = Kanalmakro des Moduls“ datenseitig ab.
4. Kategorie-Ordner: `importEdz()` nimmt die vom Nutzer eingegebene Kategorie (Vorgabe „Allgemein“); `P_ARTICLE_PRODUCTGROUP` (+ `…TOPGROUP`, `…SUBGROUP`) steht im XML und lässt sich über Abschnitt 6 in einen Ordnernamen übersetzen (z.B. 23 → „Starkstrom-Schaltgeräte“, 26 → „SPS“, 3 → „Klemmen“). Für Data-Portal-Pakete nutzt der Code bereits die CSV-Spalte „Product group“.
5. Makro finden: `P_ARTICLE_MACRO` nennt das zugehörige Makro (`$(MD_MACROS)\Hersteller\name.ema`); `importEdz()` durchsucht stattdessen alle `*.ema` im Archiv. Mit `P_ARTICLE_MACRO` ließe sich gezielt das Artikelmakro wählen (die EDZ liegt unter `items/macro/<Hersteller>/…`).
6. Unbekannte Attribute tolerieren: neuere Exporte enthalten Attribute, die die 2.9-Hilfe nicht kennt (`P_ARTICLE_CAN_BE_LINED_UP`, `P_ARTICLE_EDP_CHECKSUM`, Wurzel `count`, `weight-unit`, `version`). Der QDom-Ansatz ist dafür robust.
7. Zahlen: Dezimalpunkt, `""` = 0, Bool `" "` = nein — beim Lesen von Maßen (`P_ARTICLE_WIDTH/HEIGHT/DEPTH`) beachten.
8. `eplanmacroreader.cpp` liest im `.ema` u.a. `P20406` als Anschluss-Funktionstext. Laut `eigenschaften-ids.md` ist **20406 = Steckerbezeichnung** (`FUNC_PLCPLUG_DESIGNATION_MANUAL`); Anschlussbeschreibung wäre 20029/20023. Prüfen, ob die gelesenen Werte (z.B. „DI0“) wirklich Steckerbezeichnungen sind oder ob 20029 zusätzlich gelesen werden sollte (Vermutung, an Makrodateien zu verifizieren).

---

## 8. Anhang: Vollständige Tabelle der Funktionsdefinitionen

Format je Zeile: `Kategorie.Gruppe` Gruppenname: Definitions-Id Definitionsname · … (also Kennung = Kategorie.Gruppe.Id). „(alt)“ = in EPLAN als „nicht mehr verwenden“ gekennzeichnet. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_funcdef.htm

#### Allgemein – Allgemeine Sonderfunktionen

**4 Verbindung**
- `4.1` Verbindungsdefinition: 1 Verbindung, allgemein · 4 Ader / Draht · 5 Interne Brücke · 6 Brücke · 7 Stegbrücke · 8 Einlegebrücke · 9 Drahtbrücke · 10 Direktverbindung · 11 Lichtwellenleiter · 12 Funkstrecke · 13 Nicht elektrische Verbindung · 14 Funktionale Verbindung (Software) · 15 Schlauch · 16 Rohr · 17 Intern · 18 Verfahrenstechnik · 19 Schaltbrücke · 20 Phasenschienenverbindung
**5 Wechslerhaken**
- `5.90` Wechslerhaken, 1 Anschluss: 1 Wechslerhaken, Öffner (dreipfadig) (alt) · 2 Wechsler, Mitte (dreipfadig) (alt) · 3 Wechslerhaken, Schließer (dreipfadig) (alt) · 4 Wechslerhaken, Schließer (zweipfadig) (alt)
**6 Spulenzusatzanschluss**
- `6.1` Spulenzusatzanschluss, 1 Anschluss: 1 Spulenzusatzanschluss, 1 Anschluss (alt)
- `6.2` Spulenzusatzanschluss, 2 Anschlüsse: 1 Spulenzusatzanschluss, 2 Anschlüsse (alt)
**7 Montageplatte**
- `7.1` Montageplatte: 1 Montageplatte
**8 Artikelplatzierung**
- `8.1` Artikelplatzierung: 1 Artikelplatzierung, allgemeines Betriebsmittel · 2 Artikelplatzierung, Klemmenleiste · 3 Artikelplatzierung, Klemme · 4 Artikelplatzierung, Stecker (Stift / Buchse) · 5 Artikelplatzierung, Stecker (Buchse) · 6 Artikelplatzierung, Stecker (Stift) · 7 Artikelplatzierung, SPS-Karte · 8 Artikelplatzierung, allgemeines Fluid-Betriebsmittel · 9 Artikelplatzierung, Fluid-Verteiler · 10 Artikelplatzierung, Sammelschiene · 11 Artikelplatzierung, Gerätekasten · 12 Artikelplatzierung, Kabel · 13 Artikelplatzierung, Steckerkontakt (Stift / Buchse) · 14 Artikelplatzierung, Steckerkontakt (Buchse) · 15 Artikelplatzierung, Steckerkontakt (Stift) · 16 Artikelplatzierung, Sammelschienenanschluss · 17 Artikelplatzierung, Kabelbaum
**9 Funktionale Funktion (Software)**
- `9.98` Grafik: 1 Grafik
- `9.99` Funktionale Funktion (Software), variabel: 1 Funktionale Funktion (Software)
**10 Artikeldefinition**
- `10.1` Artikeldefinition: 1 Artikeldefinitionspunkt
**11 Funktionale Platzierung**
- `11.1` Funktionale Platzierung: 1 Funktionale Platzierung, allgemeines Betriebsmittel · 2 Funktionale Platzierung, allgemeines Fluid-Betriebsmittel · 3 Funktionale Platzierung, Mechanik-Betriebsmittel · 4 Funktionale Platzierung, Verfahrenstechnik-Betriebsmittel
**15 Isoliertes Verbindungsende**
- `15.1` Isoliertes Verbindungsende: 1 Isoliertes Verbindungsende, allgemeines Betriebsmittel · 2 Isoliertes Verbindungsende, Klemmenleiste · 3 Isoliertes Verbindungsende, Stecker (Stift / Buchse) · 4 Isoliertes Verbindungsende, SPS-Karte · 5 Isoliertes Verbindungsende, allgemeines Fluid-Betriebsmittel · 6 Isoliertes Verbindungsende, Fluid-Verteiler · 7 Isoliertes Verbindungsende, Sammelschiene · 8 Isoliertes Verbindungsende, Gerätekasten · 9 Isoliertes Verbindungsende, Stecker (Stift) · 10 Isoliertes Verbindungsende, Stecker (Buchse)
**21 Topologie-Verlegeweg**
- `21.1` Topologie-Verlegepunkt: 1 Topologie-Verlegepunkt · 2 Topologie-Strukturverlegepunkt
- `21.2` Topologie-Strecke: 1 Topologie-Strecke
- `21.3` Topologie-Teilstrecke: 1 Topologie-Teilstrecke
**22 Kabelbaum**
- `22.1` Kabelbaum: 1 Kabelbaumdefinition

#### Elektrotechnik – Klemmen und Stecker

**100 Klemme**
- `100.2` Klemme, 2 Anschlüsse (alt): 0 Klemme, allgemein (alt) · 1 Durchgangsklemme, 2 Ziele (alt) · 2 Durchgangsklemme, 2 Ziele, mit Steg (alt) · 3 Durchgangsklemme, 3 Ziele (alt) · 4 Durchgangsklemme, 3 Ziele, mit Steg (alt) · 5 Durchgangsklemme, 4 Ziele (alt) · 6 Durchgangsklemme, 4 Ziele, mit Steg (alt) · 7 N-Durchgangsklemme, 2 Ziele, mit Steg (alt) · 8 N-Trennklemme, 1 Ziel, mit Steg (alt) · 9 N-Durchgangsklemme, 4 Ziele, mit Steg (alt) · 10 N-Trennklemme, 4 Ziele, mit 2*N (alt) · 11 PE-Trennklemme, 4 Ziele, mit 2*PE (alt) · 12 PE-Klemme, 2 Ziele (alt) · 13 PE-Klemme, 4 Ziele (alt) · 14 PE für Aktorklemme, 2 Ziele (alt) · 15 N für Aktorklemme, 2 Ziele, mit Steg (alt) · 16 ~ für Aktorklemme, 2 Ziele, mit Steg (alt) · 17 + für Aktorklemme, 2 Ziele, mit Steg (alt) · 18 - für Aktorklemme, 2 Ziele, mit Steg (alt) · 19 Klemme / Steckadapter, 2 Ziele, mit Steg (alt) · 20 Klemmenanschluss / Steckadapter, 2 Ziele, mit Steg (alt) · 22 Zweileiter-Initiator, Aktorklemme, 2 Ziele (alt) · 23 Zweileiter-Initiator, Aktorklemme, 3 Ziele (alt) · 24 Zweileiter-Initiator, Aktorklemme, 2 (4) Ziele (alt) · 25 Dreileiter-Initiator, Aktorklemme, 2 (3) Ziele (alt) · 26 Dreileiter-Initiator, Aktorklemme, 2 (4) Ziele (alt) · 27 Dreileiter-Initiator, Aktorklemme, 2 (5) Ziele (alt) · 29 Trennklemme, 2 Ziele (alt) · 30 Leuchtdiodenklemme (alt) · 31 Sicherungsklemme, 2 Ziele (alt) · 32 Diodenbaustein / Mehrpfadklemme, 2 Ziele (alt) · 48 Folgeklemme (alt) · 49 Sicherungsklemme mit LED 4 (alt) · 50 Trennklemme, 4 Ziele (alt) · 51 N-Durchgangsklemme, 3 Ziele, mit Steg (alt) · 52 PE-Klemme, 3 Ziele (alt) · 53 SH-Durchgangsklemme, 2 Ziele, mit Steg (alt) · 54 SH für Aktorklemme, 2 Ziele, mit Steg (alt) · 55 SH-Trennklemme, 1 Ziel, mit Steg (alt)
- `100.3` Klemmen, 1 Anschluss: 0 Klemme, allgemein, 1 Anschluss · 1 Klemme, 1 Anschluss · 2 N-Klemme, 1 Anschluss · 3 PE-Klemme, 1 Anschluss · 4 SH-Klemme, 1 Anschluss
- `100.4` Klemmen mit Steg, 1 Anschluss: 0 Klemme, allgemein, mit Steg, 1 Anschluss · 1 Klemme mit Steg, 1 Anschluss · 2 N-Klemme mit Steg, 1 Anschluss · 3 PE-Klemme mit Schienenkontakt, 1 Anschluss · 4 SH-Klemme mit Steg, 1 Anschluss
- `100.5` Klemmen, 2 Anschlüsse: 0 Klemme, allgemein, 2 Anschlüsse · 1 Klemme, 2 Anschlüsse · 2 N-Klemme, 2 Anschlüsse · 3 PE-Klemme, 2 Anschlüsse · 4 SH-Klemme, 2 Anschlüsse
- `100.6` Klemmen mit Steg, 2 Anschlüsse: 0 Klemme, allgemein, mit Steg, 2 Anschlüsse · 1 Klemme mit Steg, 2 Anschlüsse · 2 N-Klemme mit Steg, 2 Anschlüsse · 3 PE-Klemme mit Schienenkontakt, 2 Anschlüsse · 4 SH-Klemme mit Steg, 2 Anschlüsse
- `100.7` Klemmen, 3 Anschlüsse: 0 Klemme, allgemein, 3 Anschlüsse · 1 Klemme, 3 Anschlüsse · 2 N-Klemme, 3 Anschlüsse · 3 PE-Klemme, 3 Anschlüsse · 4 SH-Klemme, 3 Anschlüsse
- `100.8` Klemmen mit Steg, 3 Anschlüsse: 0 Klemme, allgemein, mit Steg, 3 Anschlüsse · 1 Klemme mit Steg, 3 Anschlüsse · 2 N-Klemme mit Steg, 3 Anschlüsse · 3 PE-Klemme mit Schienenkontakt, 3 Anschlüsse · 4 SH-Klemme mit Steg, 3 Anschlüsse
- `100.9` Klemmen, 4 Anschlüsse: 0 Klemme, allgemein, 4 Anschlüsse · 1 Klemme, 4 Anschlüsse · 2 N-Klemme, 4 Anschlüsse · 3 PE-Klemme, 4 Anschlüsse · 4 SH-Klemme, 4 Anschlüsse
- `100.10` Klemmen mit Steg, 4 Anschlüsse: 0 Klemme, allgemein, mit Steg, 4 Anschlüsse · 1 Klemme mit Steg, 4 Anschlüsse · 2 N-Klemme mit Steg, 4 Anschlüsse · 3 PE-Klemme mit Schienenkontakt, 4 Anschlüsse · 4 SH-Klemme mit Steg, 4 Anschlüsse
- `100.11` Klemmen, 6 Anschlüsse: 0 Klemme, allgemein, 6 Anschlüsse · 1 Klemme, 6 Anschlüsse · 2 N-Klemme, 6 Anschlüsse · 3 PE-Klemme, 6 Anschlüsse · 4 SH-Klemme, 6 Anschlüsse
- `100.12` Klemmen mit Steg, 6 Anschlüsse: 0 Klemme, allgemein, mit Steg, 6 Anschlüsse · 1 Klemme mit Steg, 6 Anschlüsse · 2 N-Klemme mit Steg, 6 Anschlüsse · 3 PE-Klemme mit Schienenkontakt, 6 Anschlüsse · 4 SH-Klemme mit Steg, 6 Anschlüsse
- `100.13` Klemmen, 8 Anschlüsse: 0 Klemme, allgemein, 8 Anschlüsse · 1 Klemme, 8 Anschlüsse · 2 N-Klemme, 8 Anschlüsse · 3 PE-Klemme, 8 Anschlüsse · 4 SH-Klemme, 8 Anschlüsse
- `100.14` Klemmen mit Steg, 8 Anschlüsse: 0 Klemme, allgemein, mit Steg, 8 Anschlüsse · 1 Klemme mit Steg, 8 Anschlüsse · 2 N-Klemme mit Steg, 8 Anschlüsse · 3 PE-Klemme mit Schienenkontakt, 8 Anschlüsse · 4 SH-Klemme mit Steg, 8 Anschlüsse
- `100.15` Klemmen, 5 Anschlüsse: 0 Klemme, allgemein, 5 Anschlüsse · 1 Klemme, 5 Anschlüsse · 2 N-Klemme, 5 Anschlüsse · 3 PE-Klemme, 5 Anschlüsse · 4 SH-Klemme, 5 Anschlüsse
- `100.16` Klemmen mit Steg, 5 Anschlüsse: 0 Klemme, allgemein, mit Steg, 5 Anschlüsse · 1 Klemme mit Steg, 5 Anschlüsse · 2 N-Klemme mit Steg, 5 Anschlüsse · 3 PE-Klemme mit Schienenkontakt, 5 Anschlüsse · 4 SH-Klemme mit Steg, 5 Anschlüsse
- `100.17` Klemmen, 7 Anschlüsse: 0 Klemme, allgemein, 7 Anschlüsse · 1 Klemme, 7 Anschlüsse · 2 N-Klemme, 7 Anschlüsse · 3 PE-Klemme, 7 Anschlüsse · 4 SH-Klemme, 7 Anschlüsse
- `100.18` Klemmen mit Steg, 7 Anschlüsse: 0 Klemme, allgemein, mit Steg, 7 Anschlüsse · 1 Klemme mit Steg, 7 Anschlüsse · 2 N-Klemme mit Steg, 7 Anschlüsse · 3 PE-Klemme mit Schienenkontakt, 7 Anschlüsse · 4 SH-Klemme mit Steg, 7 Anschlüsse
- `100.19` Klemmen, 9 Anschlüsse: 0 Klemme, allgemein, 9 Anschlüsse · 1 Klemme, 9 Anschlüsse · 2 N-Klemme, 9 Anschlüsse · 3 PE-Klemme, 9 Anschlüsse · 4 SH-Klemme, 9 Anschlüsse
- `100.20` Klemmen mit Steg, 9 Anschlüsse: 0 Klemme, allgemein, mit Steg, 9 Anschlüsse · 1 Klemme mit Steg, 9 Anschlüsse · 2 N-Klemme mit Steg, 9 Anschlüsse · 3 PE-Klemme mit Schienenkontakt, 9 Anschlüsse · 4 SH-Klemme mit Steg, 9 Anschlüsse
- `100.98` Grafik: 1 Grafik
- `100.99` Klemme, variabel: 1 Klemme, variabel · 2 N-Klemme, variabel · 3 PE-Klemme, variabel · 4 SH-Klemme, variabel
**101 Steckerkontakt**
- `101.2` Steckerkontakt, 2 Anschlüsse: 0 Steckerkontakt, allgemein · 1 Steckerstift und -buchse · 2 PE-Steckerstift und -buchse · 3 Klemmen- / Steckadapter-Steckerstift und -buchse · 4 Steckerstift, 2 Anschlüsse · 5 Steckerbuchse, 2 Anschlüsse · 6 PE-Steckerstift, 2 Anschlüsse · 7 PE-Steckerbuchse, 2 Anschlüsse · 9 N-Steckerstift und -buchse · 10 N-Steckerstift, 2 Anschlüsse · 11 N-Steckerbuchse, 2 Anschlüsse · 12 SH-Steckerstift und -buchse · 13 SH-Steckerstift, 2 Anschlüsse · 14 SH-Steckerbuchse, 2 Anschlüsse
- `101.98` Grafik: 1 Grafik
- `101.99` Steckerkontakt, variabel: 1 Steckerkontakt, variabel · 2 Steckerstift, variabel · 3 Steckerbuchse, variabel
**102 Klemmenleistendefinition**
- `102.1` Klemmenleistendefinition: 1 Klemmenleistendefinition
**103 Steckerdefinition**
- `103.1` Steckerdefinition: 1 Steckerdefinition für Stifte und Buchsen · 2 Steckerdefinition für Stifte · 3 Steckerdefinition für Buchsen
**104 Trennklemme**
- `104.1` Trennklemme, 2 Anschlüsse: 1 Trennklemme, 2 Anschlüsse
- `104.2` Trennklemme, 3 Anschlüsse: 1 Trennklemme, 3 Anschlüsse
- `104.3` Trennklemme, 4 Anschlüsse: 1 Trennklemme, 4 Anschlüsse
- `104.98` Grafik: 1 Grafik
- `104.99` Trennklemme, variabel: 1 Trennklemme, variabel
**105 Klemmenzubehör**
- `105.1` Klemmenzubehör: 0 Allgemeine · 1 Endwinkel · 2 Trennwand · 3 Abschluss · 4 Abdeckung
**106 Steckdose**
- `106.1` Steckdose, 2 Anschlüsse: 0 Steckdose, allgemein, 2 Anschlüsse · 1 Steckdose ohne PE, 2 Anschlüsse
- `106.2` Steckdose, 3 Anschlüsse: 0 Steckdose, allgemein, 3 Anschlüsse · 1 Steckdose mit PE, 3 Anschlüsse · 2 Steckdose ohne PE, 3 Anschlüsse
- `106.3` Steckdose, 4 Anschlüsse: 0 Steckdose, allgemein, 4 Anschlüsse · 1 Steckdose mit PE, 4 Anschlüsse · 2 Steckdose ohne PE, 4 Anschlüsse
- `106.4` Steckdose, 5 Anschlüsse: 0 Steckdose, allgemein, 5 Anschlüsse · 1 Steckdose mit PE, 5 Anschlüsse
- `106.5` Stecker, 2 Anschlüsse: 0 Stecker, allgemein, 2 Anschlüsse · 1 Stecker ohne PE, 2 Anschlüsse
- `106.6` Stecker, 3 Anschlüsse: 0 Stecker, allgemein, 3 Anschlüsse · 1 Stecker mit PE, 3 Anschlüsse · 2 Stecker ohne PE, 3 Anschlüsse
- `106.7` Stecker, 4 Anschlüsse: 0 Stecker, allgemein, 4 Anschlüsse · 1 Stecker mit PE, 4 Anschlüsse · 2 Stecker ohne PE, 4 Anschlüsse
- `106.8` Stecker, 5 Anschlüsse: 0 Stecker, allgemein, 5 Anschlüsse · 1 Stecker mit PE, 5 Anschlüsse
- `106.98` Grafik: 1 Grafik
- `106.99` Steckdose / Stecker, variabel: 1 Steckdose mit PE, variabel · 2 Steckdose, variabel · 3 Stecker mit PE, variabel · 4 Stecker, variabel

#### Elektrotechnik – Spulen, Kontakte und Schutzbeschaltungen

**200 Spule**
- `200.1` Spule, 2 Anschlüsse: 0 Spule, allgemein · 1 Spule für Leistungsschütz · 2 Spule, anzugsverzögert · 3 Spule, abfallverzögert · 4 Spule, anzugs- und abfallverzögert · 5 Spule, Taktgeber · 6 Spule, Wechselstrom · 7 Spule, Remanenzrelais / Sperrschütz · 8 Spule, Multifunktionsrelais · 9 Stromauslöser · 10 Spule für Hilfsrelais
- `200.2` Spule mit Sonderansteuerung, 3 Anschlüsse: 0 Spule, allgemein, 3 Anschlüsse · 1 Spule, 3 Anschlüsse · 2 Spule, anzugsverzögert, 3 Anschlüsse · 3 Spule, abfallverzögert, 3 Anschlüsse · 4 Spule, anzugs- und abfallverzögert, 3 Anschlüsse · 5 Taktgeber, 3 Anschlüsse · 7 Remanenzrelais / Sperrschütz, 3 Anschlüsse · 8 Multifunktionsrelais, 3 Anschlüsse
- `200.3` Spule mit Sonderansteuerung, 4 Anschlüsse: 0 Spule, allgemein, 4 Anschlüsse · 1 Spule, 4 Anschlüsse · 2 Spule, anzugsverzögert, 4 Anschlüsse · 3 Spule, abfallverzögert, 4 Anschlüsse · 4 Spule, anzugs- und abfallverzögert, 4 Anschlüsse · 5 Taktgeber, 4 Anschlüsse · 7 Remanenzrelais / Sperrschütz, 4 Anschlüsse · 8 Multifunktionsrelais, 4 Anschlüsse
- `200.98` Grafik: 1 Grafik
- `200.99` Spule, variabel: 1 Spule, variabel
**201 Schließer**
- `201.1` Schließer, 2 Anschlüsse: 0 Schließer, allgemein · 1 Schließer (alt) · 2 Schließer, Hilfskontakt · 3 Leistungsschließer · 4 Multifunktionsschließer · 5 Schließer, schließt verzögert · 6 Schließer, öffnet verzögert · 7 Schließer, voreilend · 8 Schließer, nacheilend · 9 Schließer, schließt und öffnet verzögert · 10 Schließer, Wischkontaktgabe nach rechts · 11 Schließer, Wischkontaktgabe nach links · 12 Schließer, Wischkontaktgabe nach rechts und links · 13 Schließer mit selbsttätigem Rückgang · 14 Schließer, Thermoauslöser
- `201.2` Zweifachschließer, 4 Anschlüsse: 0 Doppelschließer, allgemein · 1 Doppelschließer
- `201.3` Dreifachschließer, 6 Anschlüsse: 0 Dreifachschließer, allgemein · 1 Dreifachschließer
- `201.98` Grafik: 1 Grafik
- `201.99` Schließer, variabel: 1 Schließerkontakt, variabel
**202 Öffner**
- `202.1` Öffner, 2 Anschlüsse: 0 Öffner, allgemein · 1 Öffner (alt) · 2 Öffner, Hilfskontakt · 3 Leistungsöffner · 4 Multifunktionsöffner · 5 Öffner, schließt verzögert · 6 Öffner, öffnet verzögert · 7 Öffner, voreilend · 8 Öffner, nacheilend · 9 Öffner, schließt und öffnet verzögert · 10 Öffner, Wischkontaktgabe nach rechts · 11 Öffner, Wischkontaktgabe nach links · 12 Öffner, Wischkontaktgabe nach rechts und links · 13 Öffner mit selbsttätigem Rückgang · 14 Öffner, Thermoauslöser · 999 Wechsler, Öffner (zweipfadig) (alt)
- `202.2` Zweifachöffner, 4 Anschlüsse: 0 Doppelöffner, allgemein · 1 Doppelöffner
- `202.3` Dreifachöffner, 6 Anschlüsse: 0 Dreifachöffner, allgemein · 1 Dreifachöffner
- `202.98` Grafik: 1 Grafik
- `202.99` Öffner, variabel: 1 Öffnerkontakt, variabel
**203 Wechsler**
- `203.1` Wechsler, 3 Anschlüsse: 0 Wechsler, allgemein · 1 Wechsler (alt) · 2 Wechsler, Hilfskontakt · 3 Leistungswechsler · 4 Multifunktionswechsler · 5 Wechsler, ausschaltverzögert · 6 Wechsler, einschaltverzögert · 7 Wechsler, voreilend · 8 Wechsler, nacheilend · 9 Wechsler, ein- und ausschaltverzögert · 10 Wechsler, Wischkontaktgabe nach rechts · 11 Wechsler, Wischkontaktgabe nach links · 12 Wechsler, Wischkontaktgabe nach rechts und links · 13 Wechsler mit selbsttätigem Rückgang · 14 Wechsler, Thermoauslöser · 15 Wechsler, vor- und nacheilend
- `203.98` Grafik: 1 Grafik
- `203.99` Wechsler, variabel: 1 Wechslerkontakt, variabel
**204 Schutzbeschaltung**
- `204.1` Schutzbeschaltung, 2 Anschlüsse: 0 Schutzbeschaltung, allgemein · 1 Schutzbeschaltung · 2 Schutzbeschaltung, Diode · 3 Schutzbeschaltung, Diode mit Zener-Diode · 4 Schutzbeschaltung, RC-Glied · 5 Schutzbeschaltung, Varistor
- `204.98` Grafik: 1 Grafik
- `204.99` Schutzbeschaltung, variabel: 1 Schutzbeschaltung, variabel

#### Elektrotechnik – SPS / Bus

**300 SPS-Anschluss**
- `300.1` SPS-Anschluss, E / A, 1 Anschluss: 1 SPS-Anschluss, allgemein · 2 SPS-Anschluss, DE · 3 SPS-Anschluss, DA · 4 SPS-Anschluss, AE · 5 SPS-Anschluss, AA · 6 SPS-Anschluss, Multifunktion
- `300.2` SPS-Anschluss, Kartenversorgung, 1 Anschluss: 1 SPS-Anschluss, SPS-KV (Undefiniert) · 2 SPS-Anschluss, SPS-KV (L) · 3 SPS-Anschluss, SPS-KV (N) · 4 SPS-Anschluss, SPS-KV (PE) · 5 SPS-Anschluss, SPS-KV (+) · 6 SPS-Anschluss, SPS-KV (M) · 7 SPS-Anschluss, SPS-KV (-) · 8 SPS-Anschluss, SPS-KV (SH)
- `300.3` SPS-Anschluss, Anschlussversorgung, 1 Anschluss: 1 SPS-Anschluss, SPS-AV (Undefiniert) · 2 SPS-Anschluss, SPS-AV (L) · 3 SPS-Anschluss, SPS-AV (N) · 4 SPS-Anschluss, SPS-AV (PE) · 5 SPS-Anschluss, SPS-AV (+) · 6 SPS-Anschluss, SPS-AV (M) · 7 SPS-Anschluss, SPS-AV (-) · 8 SPS-Anschluss, SPS-AV (SH)
- `300.4` SPS-Anschluss, Netzwerk- / Bus-Kabel: 1 Netzwerk- / Bus-Kabelanschluss, allgemein · 2 Netzwerk- / Bus-Kabelanschluss, Quelle (alt) · 3 Netzwerk- / Bus-Kabelanschluss, Eingang (alt) · 4 Netzwerk- / Bus-Kabelanschluss, Ausgang (alt) · 5 Netzwerk- / Bus-Kabelanschluss, Switch-Anschluss · 6 Netzwerk- / Bus-Kabelanschluss, Ende (alt)
- `300.5` SPS-Anschluss, Netzwerk- / Bus-Kabel, 2 Anschlüsse: 1 Netzwerk- / Bus-Kabelanschluss, allgemein · 2 Netzwerk- / Bus-Kabelanschluss, Durchgang
- `300.6` SPS-Anschluss, 2 Anschlüsse: 1 SPS-Anschluss, allgemein, 2 Anschlüsse
- `300.7` SPS-Anschluss, intern: 1 SPS-Anschluss, intern, allgemein
- `300.98` Grafik: 1 Grafik
- `300.99` SPS-Anschluss, variabel: 0 SPS-Anschluss, allgemein, variabel · 1 SPS-Anschluss, variabel · 2 SPS-Anschluss, variabel, DE · 3 SPS-Anschluss, variabel, DA · 4 SPS-Anschluss, variabel, AE · 5 SPS-Anschluss, variabel, AA
**301 SPS-Kasten**
- `301.1` SPS-Kasten: 1 SPS-Kasten

#### Elektrotechnik – Kabel / Antenne

**400 Kabel**
- `400.1` Kabel: 1 Kabeldefinition
**401 Abschirmung**
- `401.1` Abschirmung: 1 Abschirmung, Kabeldefinition
**430 Antenne**
- `430.1` Antenne: 1 Antenne

#### Elektrotechnik – Schutzeinrichtung

**500 Schmelzsicherung**
- `500.1` Einfachschmelzsicherung: 0 Schmelzsicherung, allgemein · 1 Schmelzsicherung · 2 Sicherungstrenner, einfach
- `500.2` Zweifachschmelzsicherung: 0 Zweifachschmelzsicherung, allgemein · 1 Zweifachschmelzsicherung · 2 Sicherungstrenner, zweifach
- `500.3` Dreifachschmelzsicherung: 0 Dreifachschmelzsicherung, allgemein · 1 Dreifachschmelzsicherung · 2 Sicherungstrenner, dreifach
- `500.4` Vierfachschmelzsicherung: 0 Vierfachschmelzsicherung, allgemein · 1 Vierfachschmelzsicherung · 2 Sicherungstrenner, vierfach
- `500.98` Grafik: 1 Grafik
- `500.99` Schmelzsicherung, variabel: 1 Schmelzsicherung, variabel
**501 Sicherungsautomat**
- `501.1` Einfachsicherungsautomat: 1 Sicherungsautomat
- `501.2` Zweifachsicherungsautomat: 1 Doppelsicherungsautomat
- `501.3` Dreifachsicherungsautomat: 1 Dreifachsicherungsautomat
- `501.4` Vierfachsicherungsautomat: 1 Vierfachsicherungsautomat
- `501.98` Grafik: 1 Grafik
- `501.99` Sicherungsautomat, variabel: 1 Sicherungsautomat, variabel
**502 Motorschutzschalter**
- `502.1` Motorschutzschalter, 6 Anschlüsse: 1 Motorschutzschalter dreipolig
- `502.2` Motorschutzschalter, 8 Anschlüsse: 1 Motorschutzschalter vierpolig
- `502.3` Motorschutzschalter, 2 Anschlüsse: 1 Motorschutzschalter einpolig
- `502.4` Motorschutzschalter, 4 Anschlüsse: 1 Motorschutzschalter zweipolig
- `502.98` Grafik: 1 Grafik
- `502.99` Motorschutzschalter, variabel: 1 Motorschutzschalter, variabel
**503 Schutzrelais**
- `503.1` Thermoauslöser, 6 Anschlüsse: 1 Thermoauslöser (Bimetallrelais)
- `503.2` Thermoauslöser, 8 Anschlüsse: 1 Thermoauslöser (Bimetallrelais) vierpolig
- `503.98` Grafik: 1 Grafik
- `503.99` Schutzrelais, variabel: 1 Schutzrelais, variabel
**504 Fehlerstrom-Schutzschalter**
- `504.1` FI-Schutzschalter, 4 Anschlüsse: 1 FI-Schutzschalter, zweipolig
- `504.2` FI-Schutzschalter, 8 Anschlüsse: 1 FI-Schutzschalter, vierpolig
- `504.3` FI-Schutzschalter, 6 Anschlüsse: 1 FI-Schutzschalter, dreipolig
- `504.98` Grafik: 1 Grafik
- `504.99` FI-Schutzschalter, variabel: 1 FI-Schutzschalter, variabel
**505 Fehlerspannungs-Schutzschalter**
- `505.1` Spannungsauslöser, 2 Anschlüsse: 1 Unterspannungsauslöser · 2 Überspannungsauslöser
- `505.98` Grafik: 1 Grafik
- `505.99` FU-Schutzschalter, variabel: 1 FU-Schutzschalter, variabel
**506 Schutzschalter**
- `506.1` Schutzschalter, 2 Anschlüsse: 1 Schutzschalter einpolig
- `506.2` Schutzschalter, 4 Anschlüsse: 1 Schutzschalter zweipolig
- `506.3` Schutzschalter, 6 Anschlüsse: 1 Schutzschalter dreipolig
- `506.4` Schutzschalter, 8 Anschlüsse: 1 Schutzschalter vierpolig
- `506.98` Grafik: 1 Grafik
- `506.99` Schutzschalter, variabel: 1 Schutzschalter, variabel
**507 Überspannungsableiter**
- `507.1` Überspannungsableiter, 2 Anschlüsse: 1 Überspannungsableiter, einpolig
- `507.2` Überspannungsableiter, 3 Anschlüsse: 1 Überspannungsableiter, zweipolig, 3 Anschlüsse
- `507.3` Überspannungsableiter, 4 Anschlüsse: 1 Überspannungsableiter, zweipolig, 4 Anschlüsse · 2 Überspannungsableiter, dreipolig, 4 Anschlüsse
- `507.4` Überspannungsableiter, 5 Anschlüsse: 1 Überspannungsableiter, vierpolig, 5 Anschlüsse
- `507.5` Überspannungsableiter, 6 Anschlüsse: 1 Überspannungsableiter, dreipolig, 6 Anschlüsse
- `507.6` Überspannungsableiter, 8 Anschlüsse: 1 Überspannungsableiter, vierpolig, 8 Anschlüsse
- `507.98` Grafik: 1 Grafik
- `507.99` Überspannungsableiter, variabel: 1 Überspannungsableiter, variabel · 2 Überspannungsableiter mit PE, variabel

#### Elektrotechnik – Sensorik, Schalter und Taster

**600 Schalter / Taster**
- `600.1` Schalter / Taster, Schließer, 2 Anschlüsse: 0 Schalter / Taster, allgemein, Schließer · 1 Schalter, Schließer · 2 Taster, Schließer · 3 Taster mit Raststellung, Schließer
- `600.2` Schalter / Taster, Öffner, 2 Anschlüsse: 0 Schalter / Taster, allgemein, Öffner · 1 Schalter, Öffner · 2 Taster, Öffner · 3 Taster mit Raststellung, Öffner
- `600.3` Schalter / Taster, Wechsler, 3 Anschlüsse: 0 Schalter / Taster, allgemein, Wechsler · 1 Schalter, Wechsler · 2 Taster, Wechsler
- `600.4` Zweifachschalter / -taster, 4 Anschlüsse: 1 Schalter, Doppelschließer · 2 Schalter, Doppelöffner · 3 Schalter, Schließer-Öffner-Kombination
- `600.5` Dreifachschalter / -taster, 6 Anschlüsse: 1 Schalter, Dreifachschließer · 2 Schalter, Dreifachöffner
- `600.6` Vierfachschalter / -taster, 8 Anschlüsse: 1 Schalter, Vierfachschließer · 2 Schalter, Vierfachöffner
- `600.98` Grafik: 1 Grafik
- `600.99` Schalter / Taster, variabel: 1 Schalter, variabel
**601 Endschalter, mechanisch**
- `601.1` Endschalter, Schließer, 2 Anschlüsse: 1 Endschalter, Schließer
- `601.2` Endschalter, Öffner, 2 Anschlüsse: 1 Endschalter, Öffner
- `601.3` Endschalter, Wechsler, 3 Anschlüsse: 1 Endschalter, Wechsler
- `601.4` Endschalter, Zweifachschalter, 4 Anschlüsse: 0 Endschalter, allgemein, 4 Anschlüsse · 1 Endschalter, Doppelschließer · 2 Endschalter, Doppelöffner · 3 Endschalter, Schließer-Öffner-Kombination
- `601.98` Grafik: 1 Grafik
- `601.99` Endschalter, variabel: 1 Endschalter, variabel
**602 Lichtschranke**
- `602.1` Lichtschranke, Schließer, 2 Anschlüsse: 1 Lichtschranke, Schließer
- `602.2` Lichtschranke, Öffner, 2 Anschlüsse: 1 Lichtschranke, Öffner
- `602.3` Lichtschranke, Sender, 2 Anschlüsse: 1 Lichtschranke, Sender
- `602.4` Lichtschranke, 3 Anschlüsse: 0 Lichtschranke, allgemein, 3 Anschlüsse · 1 Lichtschranke, Wechsler · 2 Lichtschranke, Schließer mit Spannungsversorgung · 3 Lichtschranke, Öffner mit Spannungsversorgung
- `602.5` Lichtschranke, Zweifachschalter, 4 Anschlüsse: 0 Lichtschranke, allgemein, 4 Anschlüsse · 1 Lichtschranke, Doppelschließer · 2 Lichtschranke, Doppelöffner · 3 Lichtschranke, Schließer-Öffner-Kombination
- `602.98` Grafik: 1 Grafik
- `602.99` Lichtschranke, variabel: 1 Lichtschranke, variabel
**603 Näherungsschalter**
- `603.1` Näherungsschalter, Schließer, 2 Anschlüsse: 1 Näherungsschalter, Schließer
- `603.2` Näherungsschalter, Öffner, 2 Anschlüsse: 1 Näherungsschalter, Öffner
- `603.3` Näherungsschalter, 3 Anschlüsse: 0 Näherungsschalter, allgemein, 3 Anschlüsse · 1 Näherungsschalter, Wechsler · 2 Näherungsschalter, Schließer mit Spannungsversorgung · 3 Näherungsschalter, Öffner mit Spannungsversorgung
- `603.4` Näherungsschalter, Zweifachschalter, 4 Anschlüsse: 0 Näherungsschalter, allgemein, 4 Anschlüsse · 1 Näherungsschalter, Doppelschließer · 2 Näherungsschalter, Doppelöffner · 3 Näherungsschalter, Schließer-Öffner-Kombination
- `603.98` Grafik: 1 Grafik
- `603.99` Näherungsschalter, variabel: 1 Näherungsschalter, variabel
**604 Digitaler Sensor, allgemein**
- `604.1` Digitaler Sensor, Schließer, 2 Anschlüsse: 0 Digitaler Sensor, allgemein, Schließer · 1 Digitaler Sensor, Schließer · 2 Digitaler Drucksensor, Schließer · 3 Digitaler Durchflusssensor, Schließer · 4 Digitaler Füllstandssensor, Schließer · 5 Digitaler Temperatursensor, Schließer
- `604.2` Digitaler Sensor, Öffner, 2 Anschlüsse: 0 Digitaler Sensor, allgemein, Öffner · 1 Digitaler Sensor, Öffner · 2 Digitaler Drucksensor, Öffner · 3 Digitaler Durchflusssensor, Öffner · 4 Digitaler Füllstandssensor, Öffner · 5 Digitaler Temperatursensor, Öffner
- `604.3` Digitaler Sensor, 3 Anschlüsse: 0 Digitaler Sensor, allgemein, 3 Anschlüsse · 1 Digitaler Sensor, Wechsler · 2 Digitaler Sensor, Schließer mit Spannungsversorgung · 3 Digitaler Sensor, Öffner mit Spannungsversorgung · 4 Digitaler Drucksensor, Wechsler · 5 Digitaler Durchflusssensor, Wechsler · 6 Digitaler Füllstandssensor, Wechsler · 7 Digitaler Temperatursensor, Wechsler
- `604.4` Digitaler Sensor, Zweifachschalter, 4 Anschlüsse: 0 Digitaler Sensor, allgemein, 4 Anschlüsse · 1 Digitaler Sensor, Doppelschließer · 2 Digitaler Sensor, Doppelöffner · 3 Digitaler Sensor, Schließer-Öffner-Kombination
- `604.98` Grafik: 1 Grafik
- `604.99` Digitaler Sensor, variabel: 1 Digitaler Sensor, variabel
**605 Analoger Sensor**
- `605.1` Analoger Sensor, 2 Anschlüsse: 0 Analoger Sensor, allgemein, 2 Anschlüsse · 1 Analoger Sensor, 2 Anschlüsse · 2 Analoger Drucksensor, 2 Anschlüsse · 3 Analoger Durchflusssensor, 2 Anschlüsse · 4 Analoger Füllstandssensor, 2 Anschlüsse · 5 Analoger Temperatursensor, 2 Anschlüsse
- `605.2` Analoger Doppelsensor, 4 Anschlüsse: 0 Analoger Doppelsensor, allgemein, 4 Anschlüsse · 1 Analoger Doppelsensor, 4 Anschlüsse · 2 Analoger Doppel-Drucksensor, 4 Anschlüsse · 3 Analoger Doppel-Durchflusssensor, 4 Anschlüsse · 4 Analoger Doppel-Füllstandssensor, 4 Anschlüsse · 5 Analoger Doppel-Temperatursensor, 4 Anschlüsse
- `605.3` Analoger Sensor mit Versorgung, 4 Anschlüsse: 0 Analoger Sensor, allgemein, mit Versorgung, 4 Anschlüsse · 1 Analoger Temperatursensor mit Versorgung, 4 Anschlüsse · 2 Analoger Drucksensor mit Versorgung, 4 Anschlüsse · 3 Analoger Durchflusssensor mit Versorgung, 4 Anschlüsse · 4 Analoger Füllstandssensor mit Versorgung, 4 Anschlüsse · 5 Analoger Sensor, mit Versorgung, 4 Anschlüsse
- `605.98` Grafik: 1 Grafik
- `605.99` Analoger Sensor, variabel: 1 Analoger Sensor, variabel
**606 Grenzwertgeber**
- `606.98` Grafik: 1 Grafik
- `606.99` Grenzwertgeber, variabel: 1 Grenzwertgeber, variabel

#### Elektrotechnik – Spannungsquelle und Generator

**700 Spannungsquelle**
- `700.1` Spannungsquelle, 2 Anschlüsse: 1 Spannungsquelle, 2 Anschlüsse
- `700.98` Grafik: 1 Grafik
- `700.99` Spannungsquelle, variabel: 1 Spannungsquelle, variabel
**701 Generator**
- `701.1` Generator, 2 Anschlüsse: 1 Generator, 2 Anschlüsse
- `701.98` Grafik: 1 Grafik
- `701.99` Generator, variabel: 1 Generator, variabel

#### Elektrotechnik – Umformer, Trafo und Gleichrichter

**800 Transformator**
- `800.1` Transformator, 4 Anschlüsse: 1 Transformator, 4 Anschlüsse
- `800.2` Transformator, 5 Anschlüsse: 1 Transformator, 5 Anschlüsse mit PE · 2 Transformator, 5 Anschlüsse ohne PE
- `800.3` Transformator, 6 Anschlüsse: 1 Transformator, 6 Anschlüsse
- `800.4` Transformator, 7 Anschlüsse: 1 Transformator, 7 Anschlüsse mit PE
- `800.98` Grafik: 1 Grafik
- `800.99` Transformator, variabel: 1 Transformator, variabel · 2 Transformator mit PE, variabel
**801 Umformer**
- `801.98` Grafik: 1 Grafik
- `801.99` Umformer, variabel: 0 Umformer, allgemein, variabel · 1 Umformer, variabel · 2 Messumformer, variabel · 3 Trennverstärker, variabel · 4 Speisetrenner, variabel · 5 Messumformer, Temperatur, variabel · 6 Messumformer, Druck, variabel · 7 Messumformer, Durchfluss, variabel · 8 Messumformer, Füllstand, variabel · 9 Messumformer, Strom, variabel · 10 Messumformer, Spannung, variabel
**802 Gleichrichter**
- `802.98` Grafik: 1 Grafik
- `802.99` Gleichrichter, variabel: 1 Gleichrichter, variabel

#### Elektrotechnik – Meldeeinrichtung, optisch und akustisch

**900 Meldeleuchte**
- `900.1` Lampe, einfach: 1 Lampe, einfach
- `900.2` Lampe, 3 Anschlüsse: 0 Lampe, allgemein, 3 Anschlüsse · 1 Lampe, 3 Anschlüsse · 2 Leuchtdiode, 2 Farben
- `900.98` Grafik: 1 Grafik
- `900.99` Lampe, variabel: 1 Lampe, variabel
**901 Meldeeinrichtung, akustisch**
- `901.1` Meldeeinrichtung, akustisch, einfach: 1 Meldeeinrichtung, akustisch, einfach
- `901.2` Meldeeinrichtung, akustisch, 3 Anschlüsse: 0 Meldeeinrichtung, akustisch, allgemein, 3 Anschlüsse · 1 Meldeeinrichtung, akustisch, 3 Anschlüsse · 2 Meldeeinrichtung, akustisch (2 Töne)
- `901.98` Grafik: 1 Grafik
- `901.99` Meldeeinrichtung, akustisch, variabel: 1 Meldeeinrichtung, akustisch, variabel
**902 Messgerät**
- `902.1` Messgerät, 2 Anschlüsse: 0 Messgerät, allgemein, 2 Anschlüsse · 1 Messgerät, 2 Anschlüsse · 2 Strommessgerät · 3 Spannungsmessgerät
- `902.2` Messgerät, 4 Anschlüsse: 0 Messgerät, allgemein, 4 Anschlüsse · 1 Messgerät, 4 Anschlüsse · 2 Wechselstromzähler, 4 Anschlüsse
- `902.3` Messgerät, 5 Anschlüsse: 0 Messgerät, allgemein, 5 Anschlüsse · 1 Messgerät, 5 Anschlüsse · 2 Wechselstromzähler, 5 Anschlüsse
- `902.4` Messgerät, 8 Anschlüsse: 2 Drehstromzähler, 8 Anschlüsse
- `902.5` Messgerät, 11 Anschlüsse: 2 Drehstromzähler, 11 Anschlüsse
- `902.98` Grafik: 1 Grafik
- `902.99` Messgerät, variabel: 1 Messgerät, variabel · 2 Zähler, variabel
**903 Messwertschreiber**
- `903.1` Messwertschreiber, 2 Anschlüsse: 1 Messwertschreiber, 2 Anschlüsse
- `903.98` Grafik: 1 Grafik
- `903.99` Messwertschreiber, variabel: 1 Messwertschreiber, variabel
**904 Anzeigegerät**
- `904.1` Anzeigegerät, 2 Anschlüsse: 1 Anzeigegerät, 2 Anschlüsse
- `904.2` Anzeigegerät, 4 Anschlüsse: 1 Anzeigegerät, 4 Anschlüsse
- `904.3` Anzeigegerät, 5 Anschlüsse: 1 Anzeigegerät, 5 Anschlüsse
- `904.98` Grafik: 1 Grafik
- `904.99` Anzeigegerät, variabel: 1 Anzeigegerät, variabel

#### Elektrotechnik – Leistungsverbraucher (Motor, Heizung, Leuchte)

**1000 Motor**
- `1000.1` Motor ohne PE und Wicklung, 2 Anschlüsse: 0 Einfachmotor, allgemein · 1 Einfachmotor ohne PE · 2 Gleichstrommotor im Schutzgehäuse · 3 Wechselstrommotor im Schutzgehäuse · 4 Motorwicklung · 5 Anker · 6 Feld
- `1000.2` Motor mit PE, 3 Anschlüsse: 0 Einfachmotor mit PE, allgemein · 1 Einfachmotor mit PE · 2 Gleichstrommotor mit PE · 3 Wechselstrommotor mit PE
- `1000.3` Motor mit PE, 4 Anschlüsse: 0 Motor mit PE, allgemein, 4 Anschlüsse · 1 Drehstrommotor · 2 Wechselstrommotor, zwei Richtungen
- `1000.4` Motor ohne PE, 4 Anschlüsse: 1 Schrittmotor
- `1000.98` Grafik: 1 Grafik
- `1000.99` Motor, variabel: 1 Motor mit PE, variabel · 2 Motor, variabel
**1001 Leuchte**
- `1001.1` Leuchte, 2 Anschlüsse: 1 Leuchte, 2 Anschlüsse
- `1001.2` Leuchte, 3 Anschlüsse: 1 Leuchte, 3 Anschlüsse
- `1001.98` Grafik: 1 Grafik
- `1001.99` Leuchte, variabel: 1 Leuchte, variabel · 2 Leuchte mit PE, variabel
**1002 Heizung**
- `1002.1` Heizung, 2 Anschlüsse: 1 Heizung, 2 Anschlüsse
- `1002.2` Heizung, 5 Anschlüsse: 1 Heizung, 5 Anschlüsse
- `1002.3` Heizung, 3 Anschlüsse: 1 Heizung, 3 Anschlüsse
- `1002.98` Grafik: 1 Grafik
- `1002.99` Heizung, variabel: 1 Heizung, variabel · 2 Heizung mit PE, variabel

#### Elektrotechnik – Ventil, Bremse und Kupplung

**1100 Ventil**
- `1100.1` Ventil, einfach: 1 Ventil, einfach
- `1100.2` Ventil, 3 Anschlüsse: 0 Ventil, allgemein, 3 Anschlüsse · 1 Ventil, 3 Anschlüsse, ohne PE · 2 Ventil, 3 Anschlüsse, mit PE
- `1100.98` Grafik: 1 Grafik
- `1100.99` Ventil, variabel: 1 Ventil, variabel
**1101 Bremse**
- `1101.1` Bremse, einfach: 1 Bremse, einfach
- `1101.2` Bremse, 3 Anschlüsse: 0 Bremse, allgemein, 3 Anschlüsse · 1 Bremse, 3 Anschlüsse, ohne PE · 2 Bremse, 3 Anschlüsse, mit PE
- `1101.98` Grafik: 1 Grafik
- `1101.99` Bremse, variabel: 1 Bremse, variabel
**1102 Kupplung**
- `1102.1` Kupplung, einfach: 1 Kupplung, einfach
- `1102.2` Kupplung, 3 Anschlüsse: 0 Kupplung, allgemein, 3 Anschlüsse · 1 Kupplung, 3 Anschlüsse, ohne PE · 2 Kupplung, 3 Anschlüsse, mit PE
- `1102.98` Grafik: 1 Grafik
- `1102.99` Kupplung, variabel: 1 Kupplung, variabel

#### Elektrotechnik – Elektronik und Logikbauteil

**1201 Kondensator**
- `1201.1` Kondensator, einfach: 1 Kondensator, einfach
- `1201.2` Kondensator, 3 Anschlüsse: 1 Kondensator, 3 Anschlüsse
- `1201.98` Grafik: 1 Grafik
- `1201.99` Kondensator, variabel: 1 Kondensator, variabel
**1202 Induktivität**
- `1202.1` Induktivität, einfach: 1 Induktivität, einfach
- `1202.2` Induktivität, 3 Anschlüsse: 1 Induktivität, 3 Anschlüsse
- `1202.98` Grafik: 1 Grafik
- `1202.99` Induktivität, variabel: 1 Induktivität, variabel
**1203 Widerstand**
- `1203.1` Widerstand, einfach: 1 Widerstand, einfach
- `1203.2` Widerstand, 3 Anschlüsse: 1 Widerstand, 3 Anschlüsse
- `1203.98` Grafik: 1 Grafik
- `1203.99` Widerstand, variabel: 1 Widerstand, variabel
**1204 Halbleiter**
- `1204.1` Halbleiter, 2 Anschlüsse: 1 Halbleiter, 2 Anschlüsse
- `1204.2` Halbleiter, 3 Anschlüsse: 1 Halbleiter, 3 Anschlüsse
- `1204.98` Grafik: 1 Grafik
- `1204.99` Halbleiter, variabel: 1 Halbleiter, variabel
**1205 Logikbauteil**
- `1205.1` Logikbauteil, 2 Anschlüsse: 1 Logikbauteil, 2 Anschlüsse
- `1205.2` Logikbauteil, 3 Anschlüsse: 1 Logikbauteil, 3 Anschlüsse
- `1205.3` Logikbauteil, 4 Anschlüsse: 1 Logikbauteil, 4 Anschlüsse
- `1205.98` Grafik: 1 Grafik
- `1205.99` Logikbauteil, variabel: 1 Logikbauteil, variabel
**1206 Regler**
- `1206.98` Grafik: 1 Grafik
- `1206.99` Regler, variabel: 1 Regler, variabel
**1207 Diode**
- `1207.1` Diode, 2 Anschlüsse: 0 Diode, allgemein, 2 Anschlüsse · 1 Zener-Diode, 2 Anschlüsse · 2 Durchbruch-Diode, 2 Anschlüsse · 3 Zweirichtungs-Diode (Diac), 2 Anschlüsse · 4 Thyristordiode (Vierschichtdiode), 2 Anschlüsse · 5 Leuchtdiode, 2 Anschlüsse
- `1207.2` Diode, 3 Anschlüsse: 0 Thyristortriode (Thyristor), allgemein, 3 Anschlüsse · 1 Zweirichtungs-Thyristortriode (Triac), 3 Anschlüsse
- `1207.98` Grafik: 1 Grafik
- `1207.99` Diode, variabel: 1 Diode, variabel
**1208 Transistor**
- `1208.1` Transistor, 2 Anschlüsse: 0 Transistor, allgemein, 2 Anschlüsse
- `1208.2` Transistor, 3 Anschlüsse: 0 Transistor, allgemein, 3 Anschlüsse · 1 Transistor, NPN, 3 Anschlüsse · 2 Transistor, PNP, 3 Anschlüsse · 3 IGBT-Transistor, N-Kanal, 3 Anschlüsse · 4 IGBT-Transistor, P-Kanal, 3 Anschlüsse · 5 Feldeffekt-Transistor, N-Kanal, 3 Anschlüsse · 6 Feldeffekt-Transistor, P-Kanal, 3 Anschlüsse · 7 Unijunction-Transistor, N-Kanal, 3 Anschlüsse · 8 Unijunction-Transistor, P-Kanal, 3 Anschlüsse
- `1208.3` Transistor, 4 Anschlüsse: 0 Transistor, allgemein, 4 Anschlüsse · 1 Transistor, PNIN, 4 Anschlüsse · 2 Transistor, PNIP, 4 Anschlüsse · 3 Feldeffekt-Transistor, N-Kanal, 4 Anschlüsse · 4 Feldeffekt-Transistor, P-Kanal, 4 Anschlüsse
- `1208.98` Grafik: 1 Grafik
- `1208.99` Transistor, variabel: 1 Transistor, variabel
**1209 Koppler**
- `1209.1` Koppler, 4 Anschlüsse: 0 Koppler, allgemein, 4 Anschlüsse · 1 Optokoppler, 4 Anschlüsse
- `1209.98` Grafik: 1 Grafik
- `1209.99` Koppler, variabel: 1 Koppler, variabel
**1210 Piezoelektrischer Kristall**
- `1210.1` Piezoelektrischer Kristall, 2 Anschlüsse: 0 Piezoelektrischer Kristall, allgemein, 2 Anschlüsse
- `1210.2` Piezoelektrischer Kristall, 3 Anschlüsse: 0 Piezoelektrischer Kristall, allgemein, 3 Anschlüsse
- `1210.98` Grafik: 1 Grafik
- `1210.99` Piezoelektrischer Kristall, variabel: 1 Piezoelektrischer Kristall, variabel
**1211 Hall-Generator**
- `1211.1` Hall-Generator, 4 Anschlüsse: 0 Hall-Generator, allgemein, 4 Anschlüsse
- `1211.98` Grafik: 1 Grafik
- `1211.99` Hall-Generator, variabel: 1 Hall-Generator, variabel

#### Elektrotechnik – Elektrotechnik Sonderfunktion

**1301 Sonstige**
- `1301.98` Grafik: 1 Grafik
- `1301.99` Sonstige, variabel: 1 Sonstige, variabel · 2 Sonstige mit PE, variabel
**1302 Geräteanschluss**
- `1302.1` Geräteanschluss: 0 Geräteanschluss, allgemein · 1 Geräteanschluss · 2 PE-Geräteanschluss · 3 N-Geräteanschluss · 4 SH-Geräteanschluss
- `1302.2` Geräteanschluss, 2 Anschlüsse: 1 Geräteanschluss, 2 Anschlüsse · 2 PE-Geräteanschluss, 2 Anschlüsse · 3 N-Geräteanschluss, 2 Anschlüsse · 4 SH-Geräteanschluss, 2 Anschlüsse
- `1302.98` Grafik: 1 Grafik
- `1302.99` Geräteanschluss, variabel: 1 Geräteanschluss, variabel
**1303 Gerätekasten**
- `1303.1` Gerätekasten: 1 Gerätekasten · 2 Feldverteiler-Kasten
**1304 Sammelschiene**
- `1304.1` Sammelschienenanschluss: 1 Sammelschienenanschluss
- `1304.2` Sammelschienenanschluss, 2 Anschlüsse: 1 Sammelschienenanschluss, 2 Anschlüsse
- `1304.98` Grafik: 1 Grafik
- `1304.99` Sammelschienenanschluss, variabel: 1 Sammelschienenanschluss, variabel
**1305 Sammelschienendefinition**
- `1305.1` Sammelschienendefinition: 1 Sammelschiene
**1306 Phasenschienendefinition**
- `1306.1` Phasenschienendefinition: 1 Phasenschiene

#### Elektrotechnik – Leitungsverteiler / -verbinder

**1400 Leitungsverteiler / -verbinder**
- `1400.1` Leitungsverbinder, 2 Anschlüsse: 1 Leitungsverbinder
- `1400.2` Leitungsverteiler, 3 Anschlüsse: 1 Leitungsverteiler, 3 Anschlüsse
- `1400.3` Leitungsverteiler, 4 Anschlüsse: 1 Leitungsverteiler, 4 Anschlüsse
- `1400.98` Grafik: 1 Grafik
- `1400.99` Leitungsverteiler, variabel: 1 Leitungsverteiler, variabel

#### Fluid – Pumpe und Druckübersetzer

**5000 Pumpe**
- `5000.1` Pumpe, 1 Anschluss: 1 Pumpe, 1 Anschluss
- `5000.2` Pumpe, 2 Anschlüsse: 1 Pumpe, 2 Anschlüsse
- `5000.98` Grafik: 1 Grafik
- `5000.99` Pumpe, variabel: 1 Pumpe, variabel
**5001 Druckübersetzer**
- `5001.1` Druckübersetzer, 1 Anschluss: 1 Druckübersetzer, 1 Anschluss
- `5001.2` Druckübersetzer, 2 Anschlüsse: 1 Druckübersetzer, 2 Anschlüsse
- `5001.3` Druckübersetzer, 3 Anschlüsse: 1 Druckübersetzer, 3 Anschlüsse
- `5001.4` Druckübersetzer, 4 Anschlüsse: 1 Druckübersetzer, 4 Anschlüsse
- `5001.98` Grafik: 1 Grafik
- `5001.99` Druckübersetzer, variabel: 1 Druckübersetzer, variabel

#### Fluid – Ventil, Verteiler und Logikbauteil

**5100 Wegeventil**
- `5100.1` 2/2-Wegeventil: 0 2/2-Wegeventil, allgemein · 1 2/2-Wegeventil · 2 2/2-Wegeventil, 1,2 · 3 2/2-Wegeventil, 1<>2 · 4 2/2-Wegeventil, 1>2 · 5 2/2-Wegeventil, 1<2
- `5100.2` 3/2-Wegeventil: 0 3/2-Wegeventil, allgemein · 1 3/2-Wegeventil · 2 3/2-Wegeventil, 1,2,3 · 3 3/2-Wegeventil, 1<>2,3 · 4 3/2-Wegeventil, 1<>3,2 · 5 3/2-Wegeventil, 1>2,3 · 6 3/2-Wegeventil, 1>3,2 · 7 3/2-Wegeventil, 1<2,3 · 8 3/2-Wegeventil, 1<3,2
- `5100.3` 3/3-Wegeventil: 0 3/3-Wegeventil, allgemein · 1 3/3-Wegeventil · 2 3/3-Wegeventil, 1,2,3 · 3 3/3-Wegeventil, 1<>2,3 · 4 3/3-Wegeventil, 1<>3,2 · 5 3/3-Wegeventil, 1>2,3 · 6 3/3-Wegeventil, 1>3,2 · 7 3/3-Wegeventil, 1<2,3 · 8 3/3-Wegeventil, 1<3,2
- `5100.4` 4/2-Wegeventil: 0 4/2-Wegeventil, allgemein · 1 4/2-Wegeventil · 2 4/2-Wegeventil, 1,2,3,4 · 3 4/2-Wegeventil, 1<2,3,4 · 4 4/2-Wegeventil, 1,2,3>4 · 5 4/2-Wegeventil, 1<>2,3<>4 · 6 4/2-Wegeventil, 1<>4,2<>3 · 7 4/2-Wegeventil, 1<2,3>4 · 8 4/2-Wegeventil, 1>4,2>3 · 9 4/2-Wegeventil, 1,3,2-4 · 10 4/2-Wegeventil, 1-3,2,4 · 11 4/2-Wegeventil, 1-3-4,2 · 12 4/2-Wegeventil, 1-2-3,4 · 13 4/2-Wegeventil, 1-2-3-4
- `5100.5` 4/3-Wegeventil: 0 4/3-Wegeventil, allgemein · 1 4/3-Wegeventil · 2 4/3-Wegeventil, 1,2,3,4 · 3 4/3-Wegeventil, 1<2,3,4 · 4 4/3-Wegeventil, 1,2,3>4 · 5 4/3-Wegeventil, 1<>2,3<>4 · 6 4/3-Wegeventil, 1<>4,2<>3 · 7 4/3-Wegeventil, 1<2,3>4 · 8 4/3-Wegeventil, 1>4,2>3 · 9 4/3-Wegeventil, 1,3,2-4 · 10 4/3-Wegeventil, 1-3,2,4 · 11 4/3-Wegeventil, 1-3-4,2 · 12 4/3-Wegeventil, 1-2-3,4 · 13 4/3-Wegeventil, 1-2-3-4 · 14 4/3-Wegeventil, 1>4<3, 2 · 15 4/3-Wegeventil, 1<2>3, 4
- `5100.6` 4/4-Wegeventil: 0 4/4-Wegeventil, allgemein · 1 4/4-Wegeventil · 2 4/4-Wegeventil, 1,2,3,4 · 3 4/4-Wegeventil, 1<2,3,4 · 4 4/4-Wegeventil, 1,2,3>4 · 5 4/4-Wegeventil, 1<>2,3<>4 · 6 4/4-Wegeventil, 1<>4,2<>3 · 7 4/4-Wegeventil, 1<2,3>4 · 8 4/4-Wegeventil, 1>4,2>3 · 9 4/4-Wegeventil, 1,3,2-4 · 10 4/4-Wegeventil, 1-3,2,4 · 11 4/4-Wegeventil, 1-3-4,2 · 12 4/4-Wegeventil, 1-2-3,4 · 13 4/4-Wegeventil, 1-2-3-4
- `5100.7` 5/2-Wegeventil: 0 5/2-Wegeventil, allgemein · 1 5/2-Wegeventil · 2 5/2-Wegeventil, 1,2,3,4,5 · 3 5/2-Wegeventil, 1<>2,3<>4,5 · 4 5/2-Wegeventil, 1<>2,3,4<>5 · 5 5/2-Wegeventil, 1<>3<>4,2,5 · 6 5/2-Wegeventil, 1>2,3>4,5 · 7 5/2-Wegeventil, 1>2,3,4>5 · 8 5/2-Wegeventil, 1>3,2,4<5 · 9 5/2-Wegeventil, 1<3>4,2,5 · 10 5/2-Wegeventil, 1<2, 3<>4, 5
- `5100.8` 5/3-Wegeventil: 0 5/3-Wegeventil, allgemein · 1 5/3-Wegeventil · 2 5/3-Wegeventil, 1,2,3,4,5 · 3 5/3-Wegeventil, 1<>2,3<>4,5 · 4 5/3-Wegeventil, 1<>2,3,4<>5 · 5 5/3-Wegeventil, 1<>3<>4,2,5 · 6 5/3-Wegeventil, 1>2,3>4,5 · 7 5/3-Wegeventil, 1>2,3,4>5 · 8 5/3-Wegeventil, 1>3,2,4<5 · 9 5/3-Wegeventil, 1<3>4,2,5 · 10 5/3-Wegeventil, 1>4,2>3,5 · 11 5/3-Wegeventil, 1<3,2,4>5 · 12 5/3-Wegeventil, 1, 2, 3<>4, 5 · 13 5/3-Wegeventil, 1>5<4, 2, 3 · 14 5/3-Wegeventil, 2>1<3, 1<5, 4 · 15 5/3-Wegeventil, 1>4, 2, 3, 5
- `5100.9` 5/4-Wegeventil: 0 5/4-Wegeventil, allgemein · 1 5/4-Wegeventil · 2 5/4-Wegeventil, 1,2,3,4,5 · 3 5/4-Wegeventil, 1<>2,3<>4,5 · 4 5/4-Wegeventil, 1<>2,3,4<>5 · 5 5/4-Wegeventil, 1<>3<>4,2,5 · 6 5/4-Wegeventil, 1>2,3>4,5 · 7 5/4-Wegeventil, 1>2,3,4>5 · 8 5/4-Wegeventil, 1>3,2,4<5 · 9 5/4-Wegeventil, 1<3>4,2,5
- `5100.98` Grafik: 1 Grafik
- `5100.99` Wegeventil, variabel: 1 Wegeventil, variabel
**5101 Ventil**
- `5101.1` Ventil, 2 Anschlüsse: 0 Ventil, allgemein, 2 Anschlüsse · 1 Ventil, 2 Anschlüsse · 2 Stetigventil, 2 Anschlüsse · 3 Druckventil, 2 Anschlüsse · 4 Sperrventil, 2 Anschlüsse · 5 Stromventil, 2 Anschlüsse · 6 Drosselrückschlagventil, 2 Anschlüsse · 7 Stromverteilerventil, 2 Anschlüsse · 8 Thermostatventil, 2 Anschlüsse · 9 Drosselventil, 2 Anschlüsse · 10 Rückschlagventil, 2 Anschlüsse · 11 Sicherheitsventil, 2 Anschlüsse · 12 Schnellentlüftungsventil, 2 Anschlüsse
- `5101.98` Grafik: 1 Grafik
- `5101.99` Ventil, variabel: 0 Ventil, allgemein, variabel · 1 Ventil, variabel · 2 Stetigventil, variabel · 3 Druckventil, variabel · 4 Sperrventil, variabel · 5 Stromventil, variabel · 6 Drosselrückschlagventil, variabel · 7 Stromverteilerventil, variabel · 8 Thermostatventil, variabel · 9 Drosselventil, variabel · 10 Rückschlagventil, variabel · 11 Sicherheitsventil, variabel · 12 Schnellentlüftungsventil, variabel
**5102 Ventil / Verteiler**
- `5102.1` Ventil / Verteiler, 2 Anschlüsse: 0 Ventil / Verteiler, allgemein, 2 Anschlüsse · 1 Ventil / Verteiler, 2 Anschlüsse · 2 Kolbenverteiler, 2 Anschlüsse · 3 Progressivverteiler, 2 Anschlüsse
- `5102.98` Grafik: 1 Grafik
- `5102.99` Ventil / Verteiler, variabel: 0 Ventil / Verteiler, allgemein, variabel · 1 Ventil / Verteiler, variabel · 2 Kolbenverteiler, variabel · 3 Progressivverteiler, variabel
**5103 Betätigung**
- `5103.1` Betätigung, 1 Anschluss: 1 Betätigung, 1 Anschluss
- `5103.2` Betätigung, 2 Anschlüsse: 1 Betätigung, 2 Anschlüsse
- `5103.98` Grafik: 1 Grafik
- `5103.99` Betätigung, variabel: 1 Betätigung, variabel
**5104 Logikbauteil**
- `5104.1` Logikbauteil, 2 Anschlüsse: 1 Logikbauteil, 2 Anschlüsse
- `5104.2` Logikbauteil, 3 Anschlüsse: 1 Logikbauteil, 3 Anschlüsse
- `5104.3` Logikbauteil, 4 Anschlüsse: 1 Logikbauteil, 4 Anschlüsse
- `5104.98` Grafik: 1 Grafik
- `5104.99` Logikbauteil, variabel: 1 Logikbauteil, variabel
**5105 Regler**
- `5105.98` Grafik: 1 Grafik
- `5105.99` Regler, variabel: 1 Regler, variabel

#### Fluid – Bremse und Kupplung

**5200 Bremse**
- `5200.1` Bremse, 1 Anschluss: 1 Bremse, 1 Anschluss
- `5200.2` Bremse, 2 Anschlüsse: 1 Bremse, 2 Anschlüsse
- `5200.98` Grafik: 1 Grafik
- `5200.99` Bremse, variabel: 1 Bremse, variabel
**5201 Kupplung**
- `5201.1` Kupplung, 1 Anschluss: 1 Kupplung, 1 Anschluss
- `5201.2` Kupplung, 2 Anschlüsse: 1 Kupplung, 2 Anschlüsse
- `5201.98` Grafik: 1 Grafik
- `5201.99` Kupplung, variabel: 1 Kupplung, variabel

#### Fluid – Speicher und Filter

**5400 Speicher**
- `5400.1` Speicher, 1 Anschluss: 1 Speicher, 1 Anschluss
- `5400.2` Speicher, 2 Anschlüsse: 1 Speicher, 2 Anschlüsse
- `5400.3` Speicher, 3 Anschlüsse: 1 Speicher, 3 Anschlüsse
- `5400.98` Grafik: 1 Grafik
- `5400.99` Speicher, variabel: 1 Speicher, variabel
**5401 Filter**
- `5401.1` Filter, 1 Anschluss: 1 Filter, 1 Anschluss
- `5401.2` Filter, 2 Anschlüsse: 1 Filter, 2 Anschlüsse
- `5401.3` Filter, 3 Anschlüsse: 1 Filter, 3 Anschlüsse
- `5401.98` Grafik: 1 Grafik
- `5401.99` Filter, variabel: 1 Filter, variabel
**5402 Abscheider**
- `5402.1` Abscheider, 1 Anschluss: 1 Abscheider, 1 Anschluss
- `5402.2` Abscheider, 2 Anschlüsse: 1 Abscheider, 2 Anschlüsse
- `5402.3` Abscheider, 3 Anschlüsse: 1 Abscheider, 3 Anschlüsse
- `5402.98` Grafik: 1 Grafik
- `5402.99` Abscheider, variabel: 1 Abscheider, variabel

#### Fluid – Aktoren, Heizung, Wärmetauscher

**5500 Zylinder**
- `5500.1` Zylinder, 1 Anschluss: 1 Zylinder, 1 Anschluss
- `5500.2` Zylinder, 2 Anschlüsse: 1 Zylinder, 2 Anschlüsse
- `5500.3` Zylinder, 3 Anschlüsse: 1 Zylinder, 3 Anschlüsse
- `5500.4` Zylinder, 4 Anschlüsse: 1 Zylinder, 4 Anschlüsse
- `5500.98` Grafik: 1 Grafik
- `5500.99` Zylinder, variabel: 1 Zylinder, variabel
**5501 Fluidmotor**
- `5501.1` Motor, 1 Anschluss: 1 Motor, 1 Anschluss
- `5501.2` Motor, 2 Anschlüsse: 1 Motor, 2 Anschlüsse
- `5501.3` Motor, 3 Anschlüsse: 1 Motor, 3 Anschlüsse
- `5501.98` Grafik: 1 Grafik
- `5501.99` Motor, variabel: 1 Motor, variabel
**5502 Heizung**
- `5502.1` Heizung, 1 Anschluss: 1 Heizung, 1 Anschluss
- `5502.2` Heizung, 2 Anschlüsse: 1 Heizung, 2 Anschlüsse
- `5502.3` Heizung, 3 Anschlüsse: 1 Heizung, 3 Anschlüsse
- `5502.98` Grafik: 1 Grafik
- `5502.99` Heizung, variabel: 1 Heizung, variabel
**5503 Wärmetauscher**
- `5503.1` Wärmetauscher, 1 Anschluss: 1 Wärmetauscher, 1 Anschluss
- `5503.2` Wärmetauscher, 2 Anschlüsse: 1 Wärmetauscher, 2 Anschlüsse
- `5503.3` Wärmetauscher, 3 Anschlüsse: 1 Wärmetauscher, 3 Anschlüsse
- `5503.98` Grafik: 1 Grafik
- `5503.99` Wärmetauscher, variabel: 1 Wärmetauscher, variabel
**5504 Aktoren, allgemein**
- `5504.1` Aktor, allgemein, 1 Anschluss: 0 Aktor, allgemein, 1 Anschluss · 1 Aktor, 1 Anschluss · 2 Saugnapf, 1 Anschluss
- `5504.2` Aktor, allgemein, 2 Anschlüsse: 1 Aktor, 2 Anschlüsse
- `5504.3` Aktor, allgemein, 3 Anschlüsse: 1 Aktor, 3 Anschlüsse
- `5504.98` Grafik: 1 Grafik
- `5504.99` Aktor, allgemein, variabel: 1 Aktor, variabel

#### Fluid – Meldeeinrichtung, optisch und akustisch

**5600 Meldeeinrichtung, optisch**
- `5600.1` Meldeeinrichtung, optisch, 1 Anschluss: 1 Meldeeinrichtung, optisch, 1 Anschluss
- `5600.2` Meldeeinrichtung, optisch, 2 Anschlüsse: 1 Meldeeinrichtung, optisch, 2 Anschlüsse
- `5600.98` Grafik: 1 Grafik
- `5600.99` Meldeeinrichtung, optisch, variabel: 1 Meldeeinrichtung, optisch, variabel
**5601 Meldeeinrichtung, akustisch**
- `5601.1` Meldeeinrichtung, akustisch, 1 Anschluss: 0 Meldeeinrichtung, akustisch, allgemein, 1 Anschluss · 1 Meldeeinrichtung, akustisch, 1 Anschluss · 2 Horn, Hupe, 1 Anschluss
- `5601.2` Meldeeinrichtung, akustisch, 2 Anschlüsse: 0 Meldeeinrichtung, akustisch, allgemein, 2 Anschlüsse · 1 Meldeeinrichtung, akustisch, 2 Anschlüsse · 2 Horn, Hupe, 2 Anschlüsse
- `5601.98` Grafik: 1 Grafik
- `5601.99` Meldeeinrichtung, akustisch, variabel: 1 Meldeeinrichtung, akustisch, variabel
**5602 Messgerät**
- `5602.1` Messgerät, 1 Anschluss: 1 Messgerät, 1 Anschluss
- `5602.2` Messgerät, 2 Anschlüsse: 1 Messgerät, 2 Anschlüsse
- `5602.98` Grafik: 1 Grafik
- `5602.99` Messgerät, variabel: 1 Messgerät, variabel
**5603 Messwertschreiber**
- `5603.1` Messwertschreiber, 1 Anschluss: 1 Messwertschreiber, 1 Anschluss
- `5603.2` Messwertschreiber, 2 Anschlüsse: 1 Messwertschreiber, 2 Anschlüsse
- `5603.98` Grafik: 1 Grafik
- `5603.99` Messwertschreiber, variabel: 1 Messwertschreiber, variabel

#### Fluid – Paket / Leitung

**5800 Paket / Leitung**
- `5800.1` Paket / Leitung: 1 Paketdefinition · 2 Schlauchleitungsdefinition · 3 Rohrleitungsdefinition

#### Fluid – Leitungsverteiler / -verbinder, Zubehör

**5900 Leitungsverteiler / -verbinder**
- `5900.1` Leitungsverbinder, 2 Anschlüsse: 1 Leitungsverbinder · 2 Reduzierung · 3 Kompensator
- `5900.2` Leitungsverteiler, 3 Anschlüsse: 1 Leitungsverteiler, 3 Anschlüsse
- `5900.3` Leitungsverteiler, 4 Anschlüsse: 1 Leitungsverteiler, 4 Anschlüsse
- `5900.98` Grafik: 1 Grafik
- `5900.99` Leitungsverteiler, variabel: 1 Leitungsverteiler, variabel
**5901 Zubehör**
- `5901.1` Zubehör, 1 Anschluss: 0 Zubehör, allgemein, 1 Anschluss · 1 Zubehör, 1 Anschluss · 2 Schalldämpfer · 3 Verschluss, Stopfen
- `5901.98` Grafik: 1 Grafik
- `5901.99` Zubehör, variabel: 1 Zubehör, variabel
**5902 Steckverbinder**
- `5902.1` Steckverbinder, 2 Anschlüsse: 0 Steckverbinder, allgemein · 1 Steckerstift und -buchse · 2 Steckerstift, 2 Anschlüsse · 3 Steckerbuchse, 2 Anschlüsse
**5903 Armatur / Verschraubung**
- `5903.1` Armatur / Verschraubung, 2 Anschlüsse: 1 Armatur / Verschraubung, 2 Anschlüsse

#### Fluid – Sonderfunktion

**6001 Sonstige (Fluid)**
- `6001.98` Grafik: 1 Grafik
- `6001.99` Sonstige (Fluid), variabel: 1 Sonstige (Fluid), variabel
**6002 Geräteanschluss (Fluid)**
- `6002.1` Geräteanschluss (Fluid): 1 Geräteanschluss (Fluid)
- `6002.2` Geräteanschluss (Fluid), 2 Anschlüsse: 1 Geräteanschluss (Fluid), 2 Anschlüsse
- `6002.98` Grafik: 1 Grafik
- `6002.99` Geräteanschluss (Fluid), variabel: 1 Geräteanschluss (Fluid), variabel
**6003 Gerätekasten (Fluid)**
- `6003.1` Gerätekasten (Fluid): 1 Gerätekasten (Fluid)

#### Fluid – Sensorik

**6100 Sensor**
- `6100.1` Sensor, 1 Fluid-Anschluss: 0 Sensor, allgemein · 1 Drucksensor · 2 Durchflusssensor · 3 Füllstandssensor · 4 Temperatursensor
- `6100.2` Sensor, 2 Fluid-Anschlüsse: 0 Sensor, allgemein · 1 Drucksensor · 2 Durchflusssensor · 3 Füllstandssensor · 4 Temperatursensor
- `6100.98` Grafik: 1 Grafik
- `6100.99` Sensor, variabel: 1 Sensor, variabel

#### Fluid – SPS / Bus (Fluid)

**6200 SPS-Anschluss (Fluid)**
- `6200.1` SPS-Anschluss (Fluid), E / A, 1 Anschluss: 1 SPS-Anschluss (Fluid), allgemein · 2 SPS-Anschluss (Fluid), DE · 3 SPS-Anschluss (Fluid), DA · 4 SPS-Anschluss (Fluid), AE · 5 SPS-Anschluss (Fluid), AA · 6 SPS-Anschluss (Fluid), Multifunktion
- `6200.2` SPS-Anschluss (Fluid), Kartenversorgung, 1 Anschluss: 1 SPS-Anschluss (Fluid), SPS-KV (Undefiniert) · 2 SPS-Anschluss (Fluid), SPS-KV (L) · 3 SPS-Anschluss (Fluid), SPS-KV (N) · 4 SPS-Anschluss (Fluid), SPS-KV (PE) · 5 SPS-Anschluss (Fluid), SPS-KV (+) · 6 SPS-Anschluss (Fluid), SPS-KV (M) · 7 SPS-Anschluss (Fluid), SPS-KV (-) · 8 SPS-Anschluss (Fluid), SPS-KV (SH)
- `6200.3` SPS-Anschluss (Fluid), Anschlussversorgung, 1 Anschluss: 1 SPS-Anschluss (Fluid), SPS-AV (Undefiniert) · 2 SPS-Anschluss (Fluid), SPS-AV (L) · 3 SPS-Anschluss (Fluid), SPS-AV (N) · 4 SPS-Anschluss (Fluid), SPS-AV (PE) · 5 SPS-Anschluss (Fluid), SPS-AV (+) · 6 SPS-Anschluss (Fluid), SPS-AV (M) · 7 SPS-Anschluss (Fluid), SPS-AV (-) · 8 SPS-Anschluss (Fluid), SPS-AV (SH)
- `6200.4` SPS-Anschluss (Fluid), Netzwerk- / Bus-Kabel: 1 Netzwerk- / Bus-Kabelanschluss (Fluid), allgemein · 5 Netzwerk- / Bus-Kabelanschluss (Fluid), Switch-Anschluss
- `6200.5` SPS-Anschluss (Fluid), Netzwerk- / Bus-Kabel, 2 Anschlüsse: 1 Netzwerk- / Bus-Kabelanschluss (Fluid), allgemein · 2 Netzwerk- / Bus-Kabelanschluss (Fluid), Durchgang
- `6200.6` SPS-Anschluss (Fluid), 2 Anschlüsse: 1 SPS-Anschluss (Fluid), allgemein, 2 Anschlüsse
- `6200.7` SPS-Anschluss (Fluid), intern: 1 SPS-Anschluss (Fluid), intern, allgemein
- `6200.98` Grafik: 1 Grafik
- `6200.99` SPS-Anschluss (Fluid), variabel: 0 SPS-Anschluss (Fluid), allgemein, variabel · 1 SPS-Anschluss (Fluid), variabel · 2 SPS-Anschluss (Fluid), variabel, DE · 3 SPS-Anschluss (Fluid), variabel, DA · 4 SPS-Anschluss (Fluid), variabel, AE · 5 SPS-Anschluss (Fluid), variabel, AA
**6201 SPS-Kasten (Fluid)**
- `6201.1` SPS-Kasten (Fluid): 1 SPS-Kasten (Fluid)

#### Verfahrenstechnik – Behälter

**10000 Behälter**
- `10000.1` Behälter: 1 Behälter · 2 Tank · 3 Bunker · 4 Silo · 5 Kolonne · 6 Reaktor
**10001 Behälteranschluss**
- `10001.1` Behälteranschluss: 1 Behälteranschluss
- `10001.2` Behälteranschluss, 2 Anschlüsse: 1 Behälteranschluss, 2 Anschlüsse
- `10001.98` Grafik: 1 Grafik
- `10001.99` Behälteranschluss, variabel: 1 Behälteranschluss, variabel

#### Verfahrenstechnik – Heizung, Kühlung, Trocknung

**10100 Behälter-Mantel**
- `10100.1` Behälter-Mantel, 2 Anschlüsse: 1 Behälter-Mantel, 2 Anschlüsse
- `10100.98` Grafik: 1 Grafik
- `10100.99` Behälter-Mantel, variabel: 1 Behälter-Mantel, variabel
**10101 Behälter-Rohrschlange**
- `10101.1` Behälter-Rohrschlange, 2 Anschlüsse: 1 Behälter-Rohrschlange, 2 Anschlüsse
- `10101.98` Grafik: 1 Grafik
- `10101.99` Behälter-Rohrschlange, variabel: 1 Behälter-Rohrschlange, variabel
**10102 Wärmetauscher**
- `10102.1` Wärmetauscher, 4 Anschlüsse: 1 Wärmetauscher, 4 Anschlüsse
- `10102.98` Grafik: 1 Grafik
- `10102.99` Wärmetauscher, variabel: 1 Wärmetauscher, variabel
**10103 Kühler**
- `10103.1` Kühler, 2 Anschlüsse: 0 Kühler, allgemein, 2 Anschlüsse · 1 Kühler, 2 Anschlüsse · 2 Kühlturm, 2 Anschlüsse
- `10103.2` Kühler, 4 Anschlüsse: 1 Kühler, 4 Anschlüsse
- `10103.98` Grafik: 1 Grafik
- `10103.99` Kühler, variabel: 1 Kühler, variabel
**10104 Dampfkessel**
- `10104.1` Dampfkessel, 2 Anschlüsse: 1 Dampfkessel, 2 Anschlüsse
- `10104.98` Grafik: 1 Grafik
- `10104.99` Dampfkessel, variabel: 1 Dampfkessel, variabel
**10105 Verdampfer**
- `10105.1` Verdampfer, 5 Anschlüsse: 1 Verdampfer, 5 Anschlüsse
- `10105.98` Grafik: 1 Grafik
- `10105.99` Verdampfer, variabel: 1 Verdampfer, variabel
**10106 Ofen**
- `10106.1` Ofen, 2 Anschlüsse: 1 Ofen, 2 Anschlüsse
- `10106.2` Ofen, 4 Anschlüsse: 1 Ofen, 4 Anschlüsse
- `10106.98` Grafik: 1 Grafik
- `10106.99` Ofen, variabel: 1 Ofen, variabel
**10107 Trockner**
- `10107.1` Trockner, 3 Anschlüsse: 1 Trockner, 3 Anschlüsse
- `10107.2` Trockner, 4 Anschlüsse: 1 Trockner, 4 Anschlüsse
- `10107.98` Grafik: 1 Grafik
- `10107.99` Trockner, variabel: 1 Trockner, variabel

#### Verfahrenstechnik – Filter, Abscheider, Sieb

**10200 Filter**
- `10200.1` Filter, 2 Anschlüsse: 1 Filter, 2 Anschlüsse
- `10200.2` Filter, 3 Anschlüsse: 1 Filter, 3 Anschlüsse
- `10200.98` Grafik: 1 Grafik
- `10200.99` Filter, variabel: 1 Filter, variabel
**10201 Abscheider**
- `10201.1` Abscheider, 3 Anschlüsse: 1 Abscheider, 3 Anschlüsse
- `10201.98` Grafik: 1 Grafik
- `10201.99` Abscheider, variabel: 1 Abscheider, variabel
**10202 Sieb**
- `10202.1` Sieb, 3 Anschlüsse: 1 Sieb, 3 Anschlüsse
- `10202.98` Grafik: 1 Grafik
- `10202.99` Sieb, variabel: 1 Sieb, variabel

#### Verfahrenstechnik – Mischer, Rührer, Kneter

**10400 Mischer**
- `10400.1` Mischer, 3 Anschlüsse: 1 Mischer, 3 Anschlüsse
- `10400.98` Grafik: 1 Grafik
- `10400.99` Mischer, variabel: 1 Mischer, variabel
**10401 Rührer**
- `10401.1` Rührer, 3 Anschlüsse: 1 Rührer, 3 Anschlüsse
- `10401.98` Grafik: 1 Grafik
- `10401.99` Rührer, variabel: 1 Rührer, variabel
**10402 Kneter**
- `10402.1` Kneter, 3 Anschlüsse: 1 Kneter, 3 Anschlüsse
- `10402.98` Grafik: 1 Grafik
- `10402.99` Kneter, variabel: 1 Kneter, variabel

#### Verfahrenstechnik – Pumpe, Kompressor, Verdichter

**10500 Pumpe**
- `10500.1` Pumpe, 2 Anschlüsse: 1 Pumpe, 2 Anschlüsse
- `10500.2` Pumpe, 3 Anschlüsse: 1 Pumpe, 3 Anschlüsse
- `10500.98` Grafik: 1 Grafik
- `10500.99` Pumpe, variabel: 1 Pumpe, variabel
**10501 Kompressor**
- `10501.1` Kompressor, 2 Anschlüsse: 1 Kompressor, 2 Anschlüsse
- `10501.2` Kompressor, 3 Anschlüsse: 1 Kompressor, 3 Anschlüsse
- `10501.98` Grafik: 1 Grafik
- `10501.99` Kompressor, variabel: 1 Kompressor, variabel
**10502 Verdichter**
- `10502.1` Verdichter, 2 Anschlüsse: 1 Verdichter, 2 Anschlüsse
- `10502.2` Verdichter, 3 Anschlüsse: 1 Verdichter, 3 Anschlüsse
- `10502.98` Grafik: 1 Grafik
- `10502.99` Verdichter, variabel: 1 Verdichter, variabel
**10503 Vakuumpumpe**
- `10503.1` Vakuumpumpe, 2 Anschlüsse: 1 Vakuumpumpe, 2 Anschlüsse
- `10503.2` Vakuumpumpe, 3 Anschlüsse: 1 Vakuumpumpe, 3 Anschlüsse
- `10503.98` Grafik: 1 Grafik
- `10503.99` Vakuumpumpe, variabel: 1 Vakuumpumpe, variabel
**10504 Ventilator**
- `10504.1` Ventilator, 2 Anschlüsse: 1 Ventilator, 2 Anschlüsse
- `10504.2` Ventilator, 3 Anschlüsse: 1 Ventilator, 3 Anschlüsse
- `10504.98` Grafik: 1 Grafik
- `10504.99` Ventilator, variabel: 1 Ventilator, variabel

#### Verfahrenstechnik – Heber, Förderer, Transporter

**10600 Heber**
- `10600.1` Heber, 2 Anschlüsse: 1 Heber, 2 Anschlüsse
- `10600.2` Heber, 3 Anschlüsse: 1 Heber, 3 Anschlüsse
- `10600.98` Grafik: 1 Grafik
- `10600.99` Heber, variabel: 1 Heber, variabel
**10601 Förderer**
- `10601.1` Förderer, 2 Anschlüsse: 1 Förderer, 2 Anschlüsse
- `10601.2` Förderer, 3 Anschlüsse: 1 Förderer, 3 Anschlüsse
- `10601.98` Grafik: 1 Grafik
- `10601.99` Förderer, variabel: 1 Förderer, variabel
**10602 Transporter**
- `10602.1` Transporter, 2 Anschlüsse: 1 Transporter, 2 Anschlüsse
- `10602.2` Transporter, 3 Anschlüsse: 1 Transporter, 3 Anschlüsse
- `10602.98` Grafik: 1 Grafik
- `10602.99` Transporter, variabel: 1 Transporter, variabel

#### Verfahrenstechnik – Waage, Messeinrichtung

**10700 Waage**
- `10700.1` Waage, 2 Anschlüsse: 1 Waage, 2 Anschlüsse
- `10700.2` Waage, 3 Anschlüsse: 1 Waage, 3 Anschlüsse
- `10700.98` Grafik: 1 Grafik
- `10700.99` Waage, variabel: 1 Waage, variabel
**10701 Messeinrichtung**
- `10701.1` Messeinrichtung, 2 Anschlüsse: 1 Messeinrichtung, 2 Anschlüsse
- `10701.2` Messeinrichtung, 3 Anschlüsse: 1 Messeinrichtung, 3 Anschlüsse
- `10701.98` Grafik: 1 Grafik
- `10701.99` Messeinrichtung, variabel: 1 Messeinrichtung, variabel

#### Verfahrenstechnik – Antriebsmaschine

**10800 Antriebsmaschine**
- `10800.1` Antriebsmaschine, 1 Anschluss: 1 Antriebsmaschine, 1 Anschluss
- `10800.98` Grafik: 1 Grafik
- `10800.99` Antriebsmaschine, variabel: 1 Antriebsmaschine, variabel

#### Verfahrenstechnik – Armatur

**10900 Absperrarmatur**
- `10900.1` Absperrarmatur, 2 Anschlüsse: 0 Absperrarmatur, allgemein, 2 Anschlüsse · 1 Absperrarmatur, 2 Anschlüsse · 2 Absperrventil, 2 Anschlüsse · 3 Absperrklappe, 2 Anschlüsse · 4 Absperrhahn, 2 Anschlüsse
- `10900.98` Grafik: 1 Grafik
- `10900.99` Absperrarmatur, variabel: 1 Absperrarmatur, variabel
**10901 Dreiwegearmatur**
- `10901.1` Dreiwegearmatur, 3 Anschlüsse: 0 Dreiwegearmatur, allgemein, 3 Anschlüsse · 1 Dreiwegearmatur, 3 Anschlüsse · 2 Dreiwegeventil, 3 Anschlüsse · 3 Dreiwegeklappe, 3 Anschlüsse · 4 Dreiwegehahn, 3 Anschlüsse
- `10901.98` Grafik: 1 Grafik
- `10901.99` Dreiwegearmatur, variabel: 1 Dreiwegearmatur, variabel
**10902 Rückschlagarmatur**
- `10902.1` Rückschlagarmatur, 2 Anschlüsse: 0 Rückschlagarmatur, allgemein, 2 Anschlüsse · 1 Rückschlagarmatur, 2 Anschlüsse · 2 Rückschlagventil, 2 Anschlüsse · 3 Rückschlagklappe, 2 Anschlüsse
- `10902.98` Grafik: 1 Grafik
- `10902.99` Rückschlagarmatur, variabel: 1 Rückschlagarmatur, variabel
**10903 Sicherheitsarmatur**
- `10903.1` Sicherheitsarmatur, 2 Anschlüsse: 0 Sicherheitsarmatur, allgemein, 2 Anschlüsse · 1 Sicherheitsarmatur, 2 Anschlüsse · 2 Sicherheitsventil, 2 Anschlüsse · 3 Sicherheitsklappe, 2 Anschlüsse · 4 Sicherheitshahn, 2 Anschlüsse
- `10903.98` Grafik: 1 Grafik
- `10903.99` Sicherheitsarmatur, variabel: 1 Sicherheitsarmatur, variabel
**10904 Regelarmatur**
- `10904.1` Regelarmatur, 2 Anschlüsse: 0 Regelarmatur, allgemein, 2 Anschlüsse · 1 Regelarmatur, 2 Anschlüsse · 2 Regelventil, 2 Anschlüsse · 3 Regelklappe, 2 Anschlüsse
- `10904.98` Grafik: 1 Grafik
- `10904.99` Regelarmatur, variabel: 1 Regelarmatur, variabel
**10905 Vierwegearmatur**
- `10905.1` Vierwegearmatur, 4 Anschlüsse: 0 Vierwegearmatur, allgemein, 4 Anschlüsse · 1 Vierwegearmatur, 4 Anschlüsse · 2 Vierwegeventil, 4 Anschlüsse · 3 Vierwegeklappe, 4 Anschlüsse · 4 Vierwegehahn, 4 Anschlüsse
- `10905.98` Grafik: 1 Grafik
- `10905.99` Vierwegearmatur, variabel: 1 Vierwegearmatur, variabel

#### Verfahrenstechnik – Rohrleitungsteil

**11000 Rohrleitungsteile**
- `11000.1` Rohrleitungsteil, 1 Anschluss: 0 Rohrleitungsteil, allgemein, 1 Anschluss · 1 Rohrleitungsteil, 1 Anschluss · 2 Drosselscheibe, 1 Anschluss · 3 Blindscheibe, 1 Anschluss · 4 Trichter, 1 Anschluss · 5 Auslass, 1 Anschluss · 6 Schalldämpfer, 1 Anschluss
- `11000.2` Rohrleitungsteil, 2 Anschlüsse: 0 Rohrleitungsteil, allgemein, 2 Anschlüsse · 1 Rohrleitungsteil, 2 Anschlüsse · 2 Schauglas, 2 Anschlüsse · 3 Schalldämpfer, 2 Anschlüsse · 4 Mischdüse, 2 Anschlüsse · 5 Schmutzfänger, 2 Anschlüsse · 6 Kondensatableiter, 2 Anschlüsse · 7 Flanschpaar, 2 Anschlüsse · 8 Reduzierung, 2 Anschlüsse · 9 Kupplung, 2 Anschlüsse · 10 Syphon, 2 Anschlüsse
- `11000.98` Grafik: 1 Grafik
- `11000.99` Rohrleitungsteil, variabel: 1 Rohrleitungsteil, variabel

#### Verfahrenstechnik – Zerkleinerer, Sichter, Sortierer, Zuteiler

**11100 Zerkleinerer**
- `11100.1` Zerkleinerer, 2 Anschlüsse: 1 Zerkleinerer, 2 Anschlüsse
- `11100.2` Zerkleinerer, 3 Anschlüsse: 1 Zerkleinerer, 3 Anschlüsse
- `11100.98` Grafik: 1 Grafik
- `11100.99` Zerkleinerer, variabel: 1 Zerkleinerer, variabel
**11101 Formgeber**
- `11101.1` Formgeber, 2 Anschlüsse: 0 Formgeber, allgemein, 2 Anschlüsse · 1 Formgeber, 2 Anschlüsse · 2 Presse, 2 Anschlüsse
- `11101.2` Formgeber, 3 Anschlüsse: 0 Formgeber, allgemein, 3 Anschlüsse · 1 Formgeber, 3 Anschlüsse · 2 Presse, 3 Anschlüsse
- `11101.98` Grafik: 1 Grafik
- `11101.99` Formgeber, variabel: 1 Formgeber, variabel
**11102 Zuteiler**
- `11102.1` Zuteiler, 2 Anschlüsse: 1 Zuteiler, 2 Anschlüsse
- `11102.2` Zuteiler, 3 Anschlüsse: 1 Zuteiler, 3 Anschlüsse
- `11102.98` Grafik: 1 Grafik
- `11102.99` Zuteiler, variabel: 1 Zuteiler, variabel
**11103 Sichter**
- `11103.1` Sichter, 2 Anschlüsse: 1 Sichter, 2 Anschlüsse
- `11103.2` Sichter, 3 Anschlüsse: 1 Sichter, 3 Anschlüsse
- `11103.98` Grafik: 1 Grafik
- `11103.99` Sichter, variabel: 1 Sichter, variabel
**11104 Sortierer**
- `11104.1` Sortierer, 2 Anschlüsse: 1 Sortierer, 2 Anschlüsse
- `11104.2` Sortierer, 3 Anschlüsse: 1 Sortierer, 3 Anschlüsse
- `11104.98` Grafik: 1 Grafik
- `11104.99` Sortierer, variabel: 1 Sortierer, variabel

#### Verfahrenstechnik – Zentrifuge

**11200 Zentrifuge**
- `11200.1` Zentrifuge, 2 Anschlüsse: 1 Zentrifuge, 2 Anschlüsse
- `11200.2` Zentrifuge, 3 Anschlüsse: 1 Zentrifuge, 3 Anschlüsse
- `11200.98` Grafik: 1 Grafik
- `11200.99` Zentrifuge, variabel: 1 Zentrifuge, variabel

#### Verfahrenstechnik – Andere Anlagenteile

**11300 Lagerung**
- `11300.1` Lagerung, 1 Anschluss: 1 Lagerung, 1 Anschluss
- `11300.2` Lagerung, 2 Anschlüsse: 1 Lagerung, 2 Anschlüsse
- `11300.98` Grafik: 1 Grafik
- `11300.99` Lagerung, variabel: 1 Lagerung, variabel
**11301 Schornstein**
- `11301.1` Schornstein, 1 Anschluss: 1 Schornstein, 1 Anschluss
- `11301.2` Schornstein, 2 Anschlüsse: 1 Schornstein, 2 Anschlüsse
- `11301.98` Grafik: 1 Grafik
- `11301.99` Schornstein, variabel: 1 Schornstein, variabel
**11302 Elektrolysezelle**
- `11302.1` Elektrolysezelle, 2 Anschlüsse: 1 Elektrolysezelle, 2 Anschlüsse
- `11302.2` Elektrolysezelle, 3 Anschlüsse: 1 Elektrolysezelle, 3 Anschlüsse
- `11302.98` Grafik: 1 Grafik
- `11302.99` Elektrolysezelle, variabel: 1 Elektrolysezelle, variabel
**11303 Abzugshaube**
- `11303.1` Abzugshaube, 1 Anschluss: 1 Abzugshaube, 1 Anschluss
- `11303.2` Abzugshaube, 2 Anschlüsse: 1 Abzugshaube, 2 Anschlüsse
- `11303.98` Grafik: 1 Grafik
- `11303.99` Abzugshaube, variabel: 1 Abzugshaube, variabel
**11304 Sonstige**
- `11304.98` Grafik: 1 Grafik
- `11304.99` Sonstige (Verfahrenstechnik), variabel: 1 Sonstige (Verfahrenstechnik), variabel
**11305 Geräteanschluss (Verfahrenstechnik)**
- `11305.1` Geräteanschluss (Verfahrenstechnik): 1 Geräteanschluss (Verfahrenstechnik)
- `11305.2` Geräteanschluss (Verfahrenstechnik), 2 Anschlüsse: 1 Geräteanschluss (Verfahrenstechnik), 2 Anschlüsse
- `11305.98` Grafik: 1 Grafik
- `11305.99` Geräteanschluss (Verfahrenstechnik), variabel: 1 Geräteanschluss (Verfahrenstechnik), variabel
**11306 Gerätekasten (Verfahrenstechnik)**
- `11306.1` Gerätekasten (Verfahrenstechnik): 1 Gerätekasten (Verfahrenstechnik)

#### Verfahrenstechnik – Leitungsverteiler / -verbinder, Zubehör

**11400 Leitungsverteiler / -verbinder**
- `11400.1` Leitungsverbinder, 2 Anschlüsse: 0 Leitungsverbinder, allgemein · 1 Leitungsverbinder · 2 Reduzierung · 3 Kompensator
- `11400.2` Leitungsverteiler, 3 Anschlüsse: 1 Leitungsverteiler, 3 Anschlüsse
- `11400.3` Leitungsverteiler, 4 Anschlüsse: 1 Leitungsverteiler, 4 Anschlüsse
- `11400.98` Grafik: 1 Grafik
- `11400.99` Leitungsverteiler, variabel: 1 Leitungsverteiler, variabel
**11401 Zubehör**
- `11401.1` Zubehör, 1 Anschluss: 1 Zubehör
- `11401.98` Grafik: 1 Grafik
- `11401.99` Zubehör, variabel: 1 Zubehör, variabel

#### Mechanik – Schaltschranksystem

**15000 Schaltschrank**
- `15000.1` Schrank: 1 Schrank
- `15000.2` Korpus: 1 Korpus
- `15000.3` Rahmenprofil: 1 Rahmenprofil
- `15000.4` Profil: 1 Profil

#### Mechanik – Systemzubehör

**15100 Gehäuse**
- `15100.1` Boden: 1 Bodenblech · 2 Flanschplatte
- `15100.2` Dach: 1 Aufsatzmodul · 2 Seitenwand für Aufsatzmodul · 3 Dachbauteil
- `15100.3` Montageplatte: 1 Montageplatte
- `15100.4` Wand: 1 Abdeckhaube · 2 Modulplatte · 3 Trennwand · 4 Wand
- `15100.5` Sockel: 1 Kabelrangierraum · 2 Kabelrangierraum, Seitenwand · 3 Sockel · 4 Sockelblende · 5 Sockelelement
- `15100.6` Tür: 1 Tür
- `15100.7` Deckel: 1 Deckel
**15101 Leitungsführung**
- `15101.1` Verdrahtungskanal: 1 Verdrahtungskanal
**15102 Gehäusezubehör für Innenanbau**
- `15102.1` Chassis: 1 Chassis
- `15102.2` Schiene: 1 Schiene · 2 Tragschiene · 3 Profilschiene
**15103 Verschlusssystem**
- `15103.1` Verschlusssystem: 1 Verschlusssystem · 2 Griff · 3 Scharnier · 4 Verschlusseinsatz
**15104 Systemzubehör**
- `15104.1` Allgemein: 1 Schrankbauteil · 2 Transportöse · 3 Kabelverschraubung · 4 Anreihzubehör
**15105 19''-Ausbautechnik**
- `15105.1` Schwenkrahmen: 1 Schwenkrahmen · 2 Schwenkrahmen, Einbausatz
- `15105.2` Profil: 1 Profil, 19 Zoll
- `15105.3` Frontrahmen: 1 Frontrahmen
**15106 Verlegezubehör**
- `15106.1` Kabel- / Schlauchbefestigung: 1 Kabel- / Schlauchbefestigung
- `15106.2` Kabel- / Schlauchbinder: 1 Kabel- / Schlauchbinder
- `15106.3` Kabel- / Schlauchschutz: 1 Kabel- / Schlauchschutz

#### Mechanik – Stromverteilung

**15500 Sammelschiene**
- `15500.1` Schiene: 1 Kupfer · 2 Kupferschiene, lamelliert · 3 Sammelschiene
- `15500.2` Adapter: 1 Adapter · 2 Anschlussadapter · 3 Geräteadapter
- `15500.3` Halter: 1 Halter
- `15500.4` Abdeckung: 1 Schutzabdeckung
**15530 Kupferstränge**
- `15530.1` Kupferstrang: 1 Kupferstrang
**15550 Sammelschienensystem**
- `15550.1` Sammelschienensystem: 1 Sammelschienensystem

#### Mechanik – Konstruktion

**16000 Bereichsdefinition**
- `16000.1` Montagebereich: 1 Bauraum · 2 Montagefläche
- `16000.2` Sperrbereich: 1 Platziersperrfläche · 2 Bohrsperrfläche · 3 Verlegesperrbereich
**16001 NC-Bearbeitung**
- `16001.1` Freie Bohrung: 1 Bohrung · 2 Gewindebohrung · 3 Rechteck · 4 Langloch · 5 Sechseck · 6 Achteck · 7 Kontur
**16002 Hilfselement**
- `16002.1` Hilfselement: 1 Hilfslinie
**16100 Verlegeweg**
- `16100.1` Strecke: 1 Strecke
- `16100.2` Verdrahtungsdurchbruch: 1 Verdrahtungsdurchbruch
- `16100.3` Verlegebereich: 1 Verlegebereich
- `16100.4` Kurve: 1 Kurve


---

## Quellen

- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_tags.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_funcdef.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_productgroups.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xmlexport_o_sellists.htm
- Eigenschafts-IDs: `referenz/eigenschaften-ids.md` (20188, 21004, 20406, 20029, 20023)
- Fork-Code (gelesen): `sources/import/edz/edzpart.cpp`, `sources/import/eplandxf/eplanpartimporter.cpp`, `sources/import/eplandxf/eplanmacroreader.cpp`
- Beobachtung: `part.xml` einer Hersteller-EDZ (Export 2023), nur Struktur und Gruppen-Nummern ausgewertet
