# Einstellungen, Oberfläche, Rechte und Mehrbenutzer in EPLAN (Plattform 2.9)

Wo EPLAN Einstellungen speichert (Projekt, Benutzer, Station, Firma), wie Schemata funktionieren, welche Einstellungen
für den Nachbau eines EPLAN-Plans Wirkung haben, wie Rechte, Mehrbenutzerbetrieb und das Eigenschaften-Prinzip (IDs)
aufgebaut sind. Die Bedienoberfläche ist bewusst knapp gehalten. Grundlage: offizielle EPLAN-Plattform-Hilfe 2.9.

## Inhalt
1. [Einstellungsebenen und Speicherorte](#1-einstellungsebenen-und-speicherorte)
2. [Schemata](#2-schemata)
3. [Einstellungen ansehen, zurücksetzen, importieren/exportieren](#3-einstellungen-ansehen-zurücksetzen-importierenexportieren)
4. [Verzeichnisse und Pfadvariablen](#4-verzeichnisse-und-pfadvariablen)
5. [Wichtige Einstellungen mit Wirkung auf Plan und Ausgabe](#5-wichtige-einstellungen-mit-wirkung-auf-plan-und-ausgabe)
6. [Meldungen, Prüfläufe, Systemmeldungen](#6-meldungen-prüfläufe-systemmeldungen)
7. [Eigenschaften-Prinzip (IDs, Index, Feldtyp)](#7-eigenschaften-prinzip-ids-index-feldtyp)
8. [Rechteverwaltung](#8-rechteverwaltung)
9. [Mehrbenutzerbetrieb und Multiuser-Monitor](#9-mehrbenutzerbetrieb-und-multiuser-monitor)
10. [Benutzeroberfläche (knapp)](#10-benutzeroberfläche-knapp)
11. [Start, Update, Add-ons, Hilfe](#11-start-update-add-ons-hilfe)
12. [Bedeutung für QET](#12-bedeutung-für-qet)
13. [Ergänzung: Kompatibilität, Einstellungen importieren, Internetverbindungen, Verbesserungsprogramm](#13-ergänzung-kompatibilität-einstellungen-importieren-internetverbindungen-verbesserungsprogramm)
14. [Quellen](#quellen)

---

## 1. Einstellungsebenen und Speicherorte

Alle Einstellungen liegen zentral unter Optionen > Einstellungen in vier Hauptkategorien; jede Kategorie hat eine eigene
Datenbank `.xdb` mit Zugriffsdatei `.xlk` im Konfigurationsverzeichnis `CFG` (lokal oder auf einem Server-Laufwerk).

| Ebene | Gilt für | Datei | Bemerkung |
|---|---|---|---|
| **Projekt** | alle Benutzer, die das Projekt öffnen, unabhängig von der Station | `PROJECT.<Version>.xdb/.xlk` | gespeichert mit dem Projekt; Standardwerte vorgegeben |
| **Benutzer** | Windows-Anmeldename, auf jeder Station | `USER.<Version>.<Benutzer>.xdb/.xlk` | Arbeitsumgebung, Verzeichnisse, Darstellung |
| **Station** | Arbeitsplatz, jeder Benutzer dort | `STATION.<Version>.<Station>.xdb/.xlk` | z. B. Pfad der Systemmeldungen, Terminalserver |
| **Firma** | alle Stationen | `COMPANY.<Version>.<Firma>.xdb/.xlk` | Firmenname aus Betriebssystem, sonst `COMPANY.Basic.xlk` |

- Modulspezifische Einstellungsdialoge (aus Funktionen heraus geöffnet) sind „von lokaler Bedeutung“ und überschreiben
  die projektweite Einstellung.
- Korrektur: Die Hilfe-Seite bezeichnet im Absatz „Projektbezogene Einstellungen“ die Datenbank `PROJECT.<Version>.xdb`
  als „benutzerbezogen“ – das ist offensichtlich ein Kopierfehler; gemeint ist die Projekt-Datenbank.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_arbeitsweise.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_systeinstellungen.htm

**Zugriff:** Rechte auf Einstellungen regelt die Rechteverwaltung; nur der Administrator darf immer. Zwei Benutzer können
nicht gleichzeitig dieselben Projekteinstellungen bearbeiten (Meldung nennt den anderen Benutzer); gesperrt wird nur das
Projekt, in dem man gerade ändert. Projekteinstellungen mit „exklusivem Zugriff“ sind bei Mehrbenutzerbetrieb gesperrt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_rechteundzugriff.htm

**Weitergeben von Projekteinstellungen:** nur über eine Projektvorlage/ein Basisprojekt (Vorlage aus Quellprojekt
erstellen, neues Projekt daraus) oder über den Projektdatenabgleich (siehe `projektverwaltung.md`). Ein Export der
Einstellungen mehrerer Projekte auf einmal ist nicht möglich.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_h_einstellungenaustauschen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_import_export.htm

**Update:** Bei neuer Programmversion werden Einstellungen übernommen; beim ersten Start fragt EPLAN, welche Ebenen
(Benutzer, Station, Firma) übernommen werden – inkl. Eigenschaftskonfiguration, Dialoggedächtnis, Baum-/Listenkonfiguration,
externe Programme. Danach ggf. Systemstammdaten-Abgleich (EPLAN-Originalstammdaten → eigene Stammdaten; Aktionen
„Systemstammdaten (ältere)“, „Ergänzungen“, beides).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_einstellungenbeimupdate.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/startingeplan_h_startenundbeenden.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_stammdatenabgleich.htm

## 2. Schemata

- Ein **Schema** ist eine benannte Gruppe von Einstellungen (Nummerierung, Filter, Fertigungsdatenexport, Arbeitsbereiche,
  Prüfläufe, Komprimierung …). EPLAN-Standardschemata sind nicht editierbar und liegen unter `\Schemata\<Firmenkennung>`.
- Symbolleiste neben „Schema“: Bearbeiten, Neu, Speichern, Kopieren, Löschen, Exportieren (einzeln), Importieren
  (Mehrfachauswahl). Dateiname-Vorschlag `Aktion.Schemaname.xml`, z. B. `CBcr.Standard.xml` (Kabel erzeugen, Schema Standard).
- Dialoge, deren Werte aus einem Schema kommen und nur für den Einzelfall geändert werden, haben **kein Dialoggedächtnis**
  – beim nächsten Aufruf gelten wieder die Schemawerte.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/schemeconfig_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/schemeconfig_m_schemaschaltflaechen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/schemeconfig_h_schematabearbeiten.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_dialoge.htm

**Filter** sind ebenfalls Schemata: feldbasiert, Werte mit Platzhaltern `*`/`?`, mehrere Werte mit `;` = ODER;
„Schnelleingabe“ für genau ein Kriterium (Feld „Wert: <Eigenschaft>“ im Navigator).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_k_filter.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_filterwerteauswahl.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_filternnach.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_filterkriterium.htm

## 3. Einstellungen ansehen, zurücksetzen, importieren/exportieren

- **Suchen** im Einstellungsdialog: findet Oberflächentexte (nicht Werte), auch in Folgedialogen, ohne Platzhalter/
  Operatoren, nur in Dialogsprache; durch Lizenz oder Rechte ausgeblendete Dialoge werden nicht gefunden. Bei mehreren
  offenen Projekten wird die Baumstruktur für alle Projekte gefiltert – nützlich zum Vergleich von Projekteinstellungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_systeinstellungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_h_einstellungenansehen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projekteingangskontrolle.htm
- **Auf Standard zurücksetzen** (Kontextmenü): einzelnes Feld oder ganzer Baumbereich. Unter „Benutzer“ werden dabei auch
  Eigenschaftskonfiguration, Dialoggedächtnis und Baum-/Listenkonfigurationen gelöscht, unter „Station“ die externen Programme.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_h_einstellungenaufstandard.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_import_export.htm
- **Export/Import** als XML, Dateiname `<Hauptkategorie>+<Unterkategorie>+<Bereich>.xml`; markierter Knoten schließt alle
  Unterbereiche ein. „Exportieren (erweitern)“ hängt weitere Bereiche an eine bestehende Datei an (ohne Namensänderung).
  Zusatzbereiche Eigenschaftskonfiguration, Dialoggedächtnis (gleiche Bildschirmauflösung empfohlen), Baum- und
  Listenkonfigurationen, externe Programme, Filter – nur für Benutzer/Station (Export auch Firma), nicht für Projekte.
  Projekt-Baumstruktur-Einstellungen werden beim Import/Export ignoriert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_h_importieren_exportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellexport.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellerweitert.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellimport.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellexportprojekt.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellimportprojekt.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_einstellprjstruktur.htm
- **Projektkontrolle/Einstellungsvergleich:** Schema mit Kontrollkästchen über alle Projekteinstellungen (Knoten =
  alle Unterbereiche).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_vergleicheinstell.htm

## 4. Verzeichnisse und Pfadvariablen

- Benutzer > Verwaltung > Verzeichnisse (als Schema, z. B. je Kunde eigener Stammdatenpool): Projekte, Vorlagen, Symbole,
  Formulare, Normblätter, Makros, Dokumente, XML usw. – definiert die Werte der Pfadvariablen `$(MD_PROJECTS)`,
  `$(MD_MACROS)` … Voreinstellung z. B. `<Programmverzeichnis>\Projekte\<Firmenkennung>`. In Dateiauswahldialogen setzt
  „Auf Standard zurücksetzen“ diese Verzeichnisse.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_einstellungenverzeichnisse.htm
- Pfadvariablen (Kontextmenü „Pfadvariable einfügen“):
  - Programm: `$(BIN)`, `$(CFG)`, `$(CFG_COMPANY)`, `$(CFG_STATION)`, `$(CFG_USER)`, `$(EPLAN)`, `$(EPLAN_DATA)`,
    `$(LOCALDATE)` (JJJJMMTT), `$(LOCALTIME)` (HHMMSS), `$(MD_DOCUMENTS)` … `$(MD_XML)`, `$(TMP)`.
  - Projekt: `$(DOC)`, `$(IMG)`, `$(P)` / `$(PROJECTPATH)` (Projektverzeichnis), `$(PROJECTNAME)` (ohne Pfad/Endung),
    `$(PROJPROP_<ID>)` (beliebige Projekteigenschaft).
  - Umgebung: `$(ENVVAR_<Name>)`; per API zur Laufzeit setzbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_pfadvariablen.htm

## 5. Wichtige Einstellungen mit Wirkung auf Plan und Ausgabe

| Pfad | Einstellung | Wirkung |
|---|---|---|
| Projekt > Darstellung > Baumstruktur (Seiten) | **Darstellungsreihenfolge auch für Seitenausgaben nutzen** | an: Baumreihenfolge gilt auch für Blättern, Drucken, **PDF-Export**, DXF/DWG-Export und Folgeblatt/Rückblatt; aus (Standard): **logische Reihenfolge der Listendarstellung** |
| dto. (Baumstruktur …) | Auch beschreibende Kennzeichen anzeigen; aufgelöste Darstellung der Unterstrukturen (`Neu.2` → Knoten `Neu` > `2`) | nur Navigator-Anzeige |
| Projekt > Betriebsmittel > BMK | Großbuchstabenkonvertierung | sichtbares/vollständiges BMK, Verbindungsbezeichnung, Farbe/Nummer, Klemmenbezeichnung, SPS-Stecker/Kanal/Adresse, Strukturkennzeichen in Großbuchstaben; **nicht** Anschlussbezeichnungen von Klemmen; bestehende Werte bleiben, Prüflauf meldet Kleinbuchstaben |
| dto. | Klammern im BMK ausblenden | `W(X1/X2)` wird als `WX1/X2` angezeigt/ausgewertet/exportiert (nur äußeres Paar); im Eigenschaftendialog immer sichtbar; wirkt auch auf symbolische Adresse (automatisch/ermittelt) |
| dto. | Vorzeichen setzen (je BM-Gruppe) | sichtbares BMK mit `-` (z. B. `-M1`) oder ohne (`M1`) |
| dto. | Fluid: Gewerkskennung als Kennbuchstabe (Medienschlüssel, DIN ISO 1219-2) | Gewerk ID 20466 → Kennbuchstabe ID 20013; Vorziffer verboten |
| Projekt > Betriebsmittel > Allgemein | Leere Eigenschaften überschreiben gefüllte / Gefüllte überschreiben leere | Abgleich zusammengehöriger Funktionen (Quelle → Ziel) |
| dto. | Beim Geräte erzeugen das Makro berücksichtigen (Standard aus) | an: alle Gerätefunktionen aus dem Artikelmakro als nicht platzierte Funktionen; aus: nur erste Hauptfunktion aus Funktionsschablonen |
| dto. | Artikel nach ihrer Position ausgeben | Leerzeilen zwischen Artikeleinträgen bleiben in Auswertungen erhalten |
| Benutzer > Betriebsmittel > Allgemein | Bei der Gerätezuordnung die Anschlussbezeichnungen berücksichtigen | Funktionszuordnung zusätzlich über identische Anschlussbezeichnungen; beim Makro-Aktualisieren werden Anschlussbezeichnungen übertragen (nur wenn Makrokasten „Aus Benutzereinstellungen“) |
| dto. | Symbol-/Makrovariante beim Gerät platzieren beibehalten; manuelle Stegbrücken beim Kopieren löschen (IDs 20350/20351) | Einfüge-/Kopierverhalten |
| Projekt > Darstellung > Datum / Uhrzeit / Zahlen | Format des Betriebssystems oder projektbezogen (z. B. `TTMMJJJJ` → 05.01.2007, Trenner `.`/`/`/`-`, Zeit `HHMMSS`, AM/PM); **Bei Zahlen Einheit mit Leerzeichen ausgeben** | Datumsfelder im Schriftfeld, Einheiten wie „1,5 mm²“ vs. „1,5mm²“ |
| Projekt > Darstellung > Projektgruppe (Zuordnung) | Gruppe 1 Schaltplanprojekte, 2 Makroprojekte, 3–5 frei (Benutzer > Darstellung > Projektgruppen (Definition): Farbe, bis 10 zusätzliche Navigatoren) | Farbrand im Navigator/Editor-Reiter |
| Projekt > Grafische Bearbeitung > Darstellung in Makroprojekten | Anzeigemodus für Blockeigenschaften/Querverweise (ausblenden, Name, Wert) | nur Makroprojekte; vor 2.8: Blockeigenschaften als Name, Querverweise ausgeblendet |
| Projekt > Verwaltung > Seiten | Standard-Normblatt, Pfadnummerierung | siehe `projektverwaltung.md` |
| Projekt > Verwaltung > Kompatibilität | z. B. „Nicht normgerechte Struktureinstellung ‚Übergeordnet‘ zulassen“ | Altverhalten |
| Projekt > Querverweise / Kontaktspiegel > … | Allgemein, Anzeige, Abbruchstellen, Kontaktspiegel | siehe `querverweise.md` |
| Benutzer > Darstellung > Allgemein | Auswertungen beim Öffnen von Seiten aktualisieren; Revisionsdaten beim Einfügen beibehalten | – |
| Benutzer > Darstellung > Oberfläche | **Identifizierende Nummern anzeigen** | Eigenschafts-IDs in Dialogen als `<20026> Funktionsdefinition` (oder dahinter), Meldungsnummern, Darstellungsart-Nummer |
| dto. | Anzahl zuletzt geöffneter Projekte (max. 10), letzte Projekte/Seite wieder öffnen, Grafische Editoren projektbezogen, Farbschema schreibgeschützter Seiten, Kennzeichnung mehrsprachiger Felder | Bedienung |
| Benutzer > Darstellung > Anzeigeeinheiten | mm oder Inch (0–8 Nachkommastellen), kg oder lb | Anzeige |
| Benutzer > Darstellung > Sprachen | Dialogsprache (Neustart), alternative Sprache (wenn Projektsprache nicht im Wörterbuch) | – |
| Benutzer > Übersetzung > Allgemein | projektunabhängige Übersetzung (Artikeltexte): Sprachen, Quellsprache, bei Eingabe übersetzen, Segment Wort/Satz/Gesamter Eintrag, Groß-/Kleinschreibung der Ausgabe | Übersetzung; Projekt-Pendant unter Projekt > Übersetzung |
| Station > Darstellung > Allgemein | Optimierung für Terminalserver | kein Hintergrundbild, keine QuickInfos |
| Station > Meldungen > System | Pfad der Systemmeldungsdatenbank | Neustart nötig |
| Firma > Verwaltung > Add-ons / EPLAN-Version prüfen / Änderungsverfolgung (Indexnummerierung) / Eigenschaftenvergleich | firmenweit | – |

Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_einstellprjstruktur.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_betriebsmittelbmk.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_betriebsmittelallgemein.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_betriebsmittelallgemeinbenutzer.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektdatum.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_projektgruppezuordnung.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_projektgruppendefinition.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_darstellungmakroprojekten.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_oberflaecheneinstellungen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellanzeigeeinheiten.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellungensprachen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_r_einstellungprojekt.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellterminalserver.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_uebergreifendesbearbeiten.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_struktur.htm

**Übersetzung (Details der Segmentierung):** Satzende = `.`, `!`, `?` jeweils mit Leerzeichen; diese Zeichen dürfen im
Wörterbuch nicht am Stichwortende stehen. „Wort“: erst Satz, dann jedes Wort; fehlt ein Wort, bleibt der ganze Satz
unübersetzt. „Satz“: nur satzweise. „Gesamter Eintrag“: ganzes Feld (für Abkürzungen mit Punkt). Zusätzliche Worttrenner
(z. B. `-`) nur bei „Wort“. Zeilenumbrüche im übersetzten Text standardmäßig nach Wortlänge, optional „textorientiert“.
Sprache `yy_XX` als Quellsprache für eindeutige Übersetzungs-IDs. Wörterbuch-Übernahme nur, wenn die Sprache im Wörterbuch existiert.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_r_einstellungprojekt.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_mehrsprachigeeingabe.htm

## 6. Meldungen, Prüfläufe, Systemmeldungen

- **Prüflaufschema** (Projekt > Verwaltung > Meldungsverwaltung > Prüflaufabhängige Einstellungen bzw. Projektdaten >
  Meldungen > Prüflauf ausführen): je Meldung (Nummer = 3-stellige Klasse + 3-stellige laufende Nummer) Kategorie
  Fehler/Warnung/Hinweis und **Prüfart**: Nein; Offline (erst nach Prüflauf; Verbindungen werden vorher automatisch
  aktualisiert, wenn Verbindungsprüfungen aktiv); Online/offline (sofort bei Entstehung, Abgearbeitetes erst beim
  Offline-Lauf entfernt); **Fehler verhindern** (widersprechende Änderung wird rückgängig gemacht); Modulbezogen (fest).
  Zusätzlich Gewerksfilter, „Erzeugt von“ (eigene Meldungen per API), „Lizenziert“.
  Max. Anzahl Prüflaufmeldungen 1–10 000. Prüflauf auf Auswahl oder ganzes Projekt; „Nur erledigte Meldungen prüfen“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_einstellungenmeldungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_projekteinstellungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_prueflauf.htm
- **Meldungsklassen:** 001 Klemmen, 002 Stecker, 003 Kabel, 004 SPS/Bus, 005 Verbindungen, 007 Geräte, 008 Fremdsprachen,
  010 Querverweise, 011 Abbruchstellen, 012 2D-Schaltschrankaufbau, 013 Artikeldaten, 014 Makro, 015 Auswertungen,
  016 Gerätekästen, 017 BMK, 020 Projektvergleich, 021 Platzhalterobjekte, 022 Sonstiges, 024 Fluid,
  025 Projekteinstellungen, 026 3D-Montageaufbau, 027 Topologie, 028 Vorplanung, 029 Vorplanungstiefe, 030 Teilprojekte,
  032 Bohrbild, 033 Kabelbaum, 501 Artikelstammdaten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_start.htm
- Nach Prüfläufen tragen fehlerhafte Funktionen in den Navigatoren ein Ausrufezeichen; „Auswahl“ in der Meldungsverwaltung
  filtert auf markierte Objekte/Seiten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_besonderheitennavigatoren.htm
- **Systemmeldungen** (Dienstprogramme > Systemmeldungen): 6-stellige Nummer (3 Modul + 3 laufend), Kategorien Hinweis,
  Warnung, Fehler (Programm neu starten), Fataler Fehler (sofort beenden). Je Sitzung gruppiert (Sitzungs-/Prozessnummer,
  Bearbeiter, Station). Datenbank `EplLog.eldb`, ab 30 MB beim nächsten Start nach `EplLog_bak.eldb` gesichert;
  Doppelklick auf `.eldb` konvertiert nach CSV. Fehler-Icon in der Statusleiste bei neuen Fehlern.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/errorhelp_k_start.htm

## 7. Eigenschaften-Prinzip (IDs, Index, Feldtyp)

- Jedes Objekt hat Eigenschaften; Objekte gehören zu **Objektklassen** (Projekte, Formulare, Symbole, Funktionen,
  Artikelreferenzen …). Eine **Eigenschafts-ID** identifiziert die Eigenschaft eindeutig und kann in mehreren Objektklassen
  vorkommen (z. B. 20117 Funktionsdefinitionsbeschreibung bei Funktionen, Symbolen, Verbindungen).
- Je Eigenschaft: ID, Bezeichnung, Objektklassen, Beschreibung, **Kategorie** (Anzeigegruppe), **Feldtyp** (z. B. Bool,
  Dezimalzahl, Mehrsprachiger Text), **Index (max.)**, **R/O** (schreibgeschützt), **API-Define** (Name in der API).
- **Index:** Mehrfachwerte, z. B. Artikelnummer 20100 `[3]` = dritte Artikelnummer; `[0]` = normalerweise alle Werte.
- IDs in der Oberfläche einblenden: Benutzer > Darstellung > Oberfläche > Identifizierende Nummern anzeigen.
- Nachschlagetabelle aller IDs: `eigenschaften-ids.md`.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/properties_k_prinzip.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_oberflaecheneinstellungen.htm

- **Eigenschaftskonfiguration:** In jedem tabellarischen Eigenschaftendialog (Projekt, Seite, BM) lassen sich eigene
  Zusammenstellungen speichern; die dockbare **Eigenschaftenübersicht** bearbeitet mehrere markierte Objekte zugleich
  (Schemata „Grafik formatieren“, „Funktionsunabhängige Eigenschaften“ = funktionsübergreifende Blockbearbeitung).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_h_benutzerdefeigenschaftskonfigurationen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_eigenschaftenuebersicht.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_eigenschaftenuebersicht.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_eigenschaftsanordnung.htm

## 8. Rechteverwaltung

- **Rechtedatenbank** (zentrale Datei, verschleiert, Passwörter verschlüsselt; ab 2.3 EPLAN-Format `.erm`, bis 2.2 Access
  `.mdb`, letztere nicht in 64-Bit importierbar). Ohne Rechtedatenbank startet EPLAN nicht – auch ohne Rechteverwaltung.
  Lokal (Ausfallsicherheit, Vorgabe) oder auf Server (zentrale Pflege; bei Serverausfall weiterarbeiten bis Programmende).
  Per Betriebssystem schützen; schreibgeschützte DB wird in der Titelleiste angezeigt. Administration nicht
  mehrbenutzerfähig (wer zuletzt schließt, gewinnt). Netzwerk-Dateirechte haben Vorrang vor EPLAN-Rechten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_rechte.htm
- **Arbeitsmodi:** *Anmelden* (EPLAN-Benutzer + Passwort, Windows-Name vorbelegt; importierte Windows-Benutzer nicht
  möglich), *Anmelden mit Windows-Benutzername* (automatisch, auch Active-Directory-Benutzer/-Gruppen), *Ohne
  Rechteverwaltung* (Windows-Name ohne Passwort, alle sind Administratoren). Benutzer wechseln = EPLAN neu starten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_arbeitsmodus.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_h_benutzerwechseln.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/urrightsmanager_d_anmelden.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_passwortaendern.htm
- **Benutzer** (Namen intern in Großbuchstaben; vordefiniert ADMINISTRATOR, GUEST, nicht änderbar) erhalten Rechte nur
  über **Gruppen** (Namen mit Groß-/Kleinschreibung). Vordefiniert: Administratoren (alle Rechte, fest), Gäste
  (eingeschränkt, fest), Stammdatenbearbeiter, Schaltplanbearbeiter (Beispiele, änderbar; Namen nicht ändern, sonst
  keine Übersetzung mehr). AD-Gruppen werden wie Benutzer geführt und EPLAN-Gruppen zugeordnet; Mitglied mehrerer
  AD-Gruppen erhält alle Rechte.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_h_benutzer.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_h_gruppen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_gruppe.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_benutzer.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_k_windowsbenutzer.htm
- **Rechtearten** im Baum je Themenbereich: Menüpunkte (entzogen → ausgeblendet, außer mehrfach genutzte wie Kopieren/
  Löschen → Meldung), `[Dialog]` (öffnen ja, bearbeiten nein; Einstellungsdialoge werden ganz ausgeblendet),
  `[Schnittstelle]` (API-Aktion/Befehlszeile). Neue Rechte: Symbolleisten nach Neuladen des Arbeitsbereichs, Menüs nach Neustart.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_gruppe.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_k_prinzip.htm

## 9. Mehrbenutzerbetrieb und Multiuser-Monitor

- Mehrere Benutzer bearbeiten dasselbe Projekt gleichzeitig, wenn es im Modus **Standard** geöffnet ist (Exklusiv sperrt
  andere aus). Konflikt → Dialog Multiuser-Konflikt mit Anmeldename, Name, Telefon, E-Mail, Computer (Daten aus
  Benutzer > Darstellung > Benutzerkennung / Adresse); E-Mail mit Betreff „Multiuser-Konflikt“ (MAPI-Client nötig).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_k_projektebearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_d_konflikte.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_h_konfliktmailversenden.htm
- **Sperrlogik:** Ändern einer Eigenschaft sperrt alle Eigenschaften des Objekts (nur Änderungssperre, Lesen erlaubt),
  sofort wieder frei. Bearbeiten auf einer Seite/in einem Bauraum sperrt Seite bzw. Bauraum mit allen dort platzierten
  Funktionen; bloßes Öffnen sperrt nichts. Im Navigator: Funktion + (falls platziert) ihre Seite; Baumnachbarn nicht,
  außer auf derselben Seite. Beispiel: Umbenennen der Leistendefinition X1 (Seite 8) betrifft nur die Definition,
  nicht die Klemmen; für alle Funktionen das Betriebsmittel-Knoten umbenennen (sperrt alle Funktionen mit dem BMK).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_k_datensperren.htm
- **Projektweit exklusiv:** Projekteigenschaften bearbeiten, Änderungen im Bauraum, Strukturkennzeichen-Verwaltung (außer
  Strukturbeschreibungen 1–3 und Zusatzfelder 1–10, je Kennzeichen ein Benutzer), Komprimieren, Sichern, Wiederherstellen,
  Stammdatenabgleich ins Projekt. Verbindungen aktualisieren: Konflikt nur bei gleichzeitiger Aktualisierung derselben
  Seiten. Rückgängig nur eigene Aktionen und nur, solange kein anderer dasselbe Objekt weiter geändert hat.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_k_projektebearbeiten.htm
- **Multiuser-Monitor:** Client-Server (Dienst „EPLAN Multiuser Monitor“ auf beliebigem Rechner; Client-Einstellung Benutzer
  > Verwaltung > Multiuser-Monitor: Server, Port, Aktiv, [Prüfen]). Zeigt eigene gemeinsam genutzte Projekte mit Benutzer,
  Bereich, Produktversion, laufender Aktion und Fortschritt sowie nur von anderen geöffnete Projekte. Zweck: lange Aktionen
  (Prüflauf, Auswertung) abstimmen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.multiusermonitor_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.multiusermonitor_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.multiusermonitor_k_vorgehensweise.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.multiusermonitor_d_monitor.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.multiusermonitor_d_einstellungen.htm
- **Benutzerkennung / Adresse** (Benutzer > Darstellung): Kennung, Name, Telefon, E-Mail, Kundennummer; diese Daten
  nennt EPLAN im Multiuser-Konflikt als Absender der Sperre. **Anmeldename** wird automatisch gefüllt (Windows-Anmeldung
  bzw. Anmeldung der Rechteverwaltung) und ist nicht änderbar. **„Anmeldename für Änderungen nutzen“**: an = automatisch
  gefüllte Eigenschaften wie „Ersteller“ und „Letzter Bearbeiter“ (Projekt, Seiten, Formulare, Normblätter) erhalten den
  Anmeldenamen; aus = stattdessen Kennung und Name; sind beide leer, meldet EPLAN das und aktiviert die Option wieder.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xusersettingsgui_d_einstellungenmehrbenutzer.htm
  **Bedeutung für QET:** Steht im Schriftfeld des Originals ein Kürzel als Bearbeiter/Ersteller, ist das je nach Option der
  Windows-Anmeldename oder die Kennung – im Nachbau als festen Text des Titelblocks übernehmen (QET-Variable des
  Folio-Autors), nicht aus dem eigenen Anmeldenamen erzeugen.
- Für Sichern/Wiederherstellen, Teilprojekte und Rechteverwaltung gilt: nicht mehrbenutzerfähig bzw. exklusiv (siehe
  `projektverwaltung.md`).

## 10. Benutzeroberfläche (knapp)

- **Navigatoren** (Seiten, Betriebsmittel, Klemmenleisten, Kabel, SPS …) zeigen Teilmengen des Projekts, je Baum und Liste.
  Der Betriebsmittel-Navigator zeigt keine Funktionen der Darstellungsarten **„Extern“ und „Grafik“ – diese gehören
  nicht zum Projekt und werden nicht ausgewertet**. Sortierung ignoriert führende Nullen des Zählers
  (X001311, X1312, X01313, X1511 statt Excel-Reihenfolge). Mehrere offene Projekte → Projektname als oberste Baumebene.
  Markierte Ebene = alle Objekte darunter (gefiltert nach Navigator und aktivem Filter).
  „Anwenden auf gesamtes Projekt“ (z. B. Nummerieren) wirkt auf alle BM inkl. nicht platzierter und ignoriert Filter.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_besonderheitennavigatoren.htm
- **Dialogtypen:** dockbare Dialoge (Navigatoren, Ebenen-, Meldungsverwaltung; als Registerkartengruppen), Meldungsfenster
  (fixe Größe), Navigatoren mit Gedächtnis, schreibgeschützte Dialoge (Rechte), Dialogsplitter.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_dialoge.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_h_bedienelementeandocken.htm
- **Arbeitsbereiche** (Ansicht > Arbeitsbereich, als Schema): Hauptfensterlage, dockbare Dialoge, Menü- und
  Symbolleisten inkl. benutzerdefinierter; nicht die Editoren. Vordefinierte Arbeitsbereiche als Vorlage.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_h_mitarbeitsbereichenarbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_arbeitsbereich.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_arbeitsbereichbearbeiten.htm
- **Symbolleisten** anpassbar (vordefinierte nicht löschbar), Schaltflächen auch für **Aktionen** (z. B. Arbeitsbereich
  öffnen, Einstellung setzen, Einstellungen importieren, Fertigungsdaten exportieren, Formatierung von Schaltzeichen/
  Texten/Grafik/Verbindungsdefinitionspunkten auf feste Werte setzen, Symbole/Makros einfügen). Tastenkombinationen unter
  Benutzer > Verwaltung > Tastenkombinationen; externe Programme unter Optionen > Externe Programme.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_symbolleisten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_h_symbolleistenanpassen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/configtoolbars_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/configtoolbars_k_aktionensymbolleisten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/configtoolbars_k_aktionenformatierungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/configtoolbars_h_schaltflaecheneinstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ot_d_symbolleisteneditor.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ot_d_tastenkombinationen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ot_d_externeprogramme.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_einstellungenschaltflaeche.htm
- Weitere Seiten (Drag & Drop, Tastaturbedienung, Kontextmenüpunkte in Tabellen, Tabellensortierung, Listen-/Baum-
  Spaltenkonfiguration, Grafische Vorschau, Farbauswahl, Bilddatei-Export, Icons in Navigatoren) nur überflogen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_hintergrund.htm

## 11. Start, Update, Add-ons, Hilfe

- Start über Windows-Startmenü (Dongle + Lizenz nötig); bei Vorversion Dialog Einstellungen importieren. Beenden speichert
  Daten und Oberflächenzustand. Firma > Verwaltung > EPLAN-Version prüfen: Hinweis + Statusleisten-Icon → Download Manager.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/startingeplan_h_startenundbeenden.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_einstellungenupdate.htm
- **Add-ons** (Dienstprogramme > Add-ons): Registrierung über `install.xml`, manuell (Kontrollkästchen Registriert) oder
  automatisch aus einem Serververzeichnis (Firma > Verwaltung > Add-ons) beim Start; EPLAN-Add-ons nicht löschbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_h_zusatzmodulregistrieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsettingsservicesgui_d_zusatzmodule.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsettingsservicesgui_d_einstellzusatzmodule.htm
- **SQL-Server** für Artikelverwaltung, Projektverwaltung und Wörterbuch; alternativer Treiber unter Station >
  Schnittstellen > Treiber für SQL-Server.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_treibersqlserver.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_sqlserver.htm
- **Hilfe:** online im Browser, kontextabhängig mit F1; Kapitel gegliedert in Grundlagen / So geht's / Referenz;
  zusätzlich „EPLAN-Übersichten“ (Eigenschaften, Platzhalterelemente, Prüflaufmeldungen). Für den Skill heißt das:
  `_k_` = Grundlagen, `_h_` = So geht's, `_d_`/`_r_` = Referenz (Dialog/Registerkarte), `_o_` = Übersicht, `_m_` = Menü.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/helpsystem_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/helpsystem_h_nutzen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/helpsystem_k_hinweise.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/helpsystem_m_hilfe.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/EPLAN_Help_k_start.htm

## 12. Bedeutung für QET

- **Seitenreihenfolge im PDF:** Standardmäßig folgt der EPLAN-PDF-Export der **logischen Reihenfolge der Seitenliste**
  (Seitenstruktur-Sortierung), nicht der Baumreihenfolge – außer „Darstellungsreihenfolge auch für Seitenausgaben nutzen“
  ist an. Beim Nachbau Folio-Reihenfolge = Reihenfolge im PDF übernehmen; bei Abweichung von der Sortierung nach
  Kennzeichen war vermutlich diese Option aktiv. Auch Folgeblatt/Rückblatt-Felder im Schriftfeld folgen derselben Regel
  (relevant für `pdf-export.md`).
- **Großbuchstaben/Klammern/Vorzeichen:** Was im PDF als BMK steht, ist bereits durch „Großbuchstabenkonvertierung“,
  „Klammern im BMK ausblenden“ und „Vorzeichen setzen“ geformt. Ein `W(X1/X2)` im Datenbestand erscheint im PDF als
  `WX1/X2`; ein BMK ohne `-` im PDF kann trotzdem ein `-`-BMK sein (Vorzeichen aus). Beim Verknüpfen BMK daher
  normalisieren: Groß/klein ignorieren, führendes `-` optional.
- **Klemmen-Anschlussbezeichnungen** werden nie in Großbuchstaben konvertiert – Groß-/Kleinschreibung dort beibehalten.
- **Einheit mit Leerzeichen** (Projekt > Datum/Uhrzeit/Zahlen) erklärt „1,5 mm²“ vs. „1,5mm²“ in Kabel-/Adertexten –
  Vergleich beim Import leerzeichentolerant machen.
- **Funktionen mit Darstellungsart „Extern“ oder „Grafik“** gehören nicht zum Projekt und erscheinen in keiner
  Auswertung. Im PDF erkennbare, aber nicht in Listen auftauchende Symbole sind wahrscheinlich solche – im QET-Nachbau
  als reine Grafik/Text ohne Betriebsmittelrolle zeichnen (nicht in Nomenklatur/Klemmenplan).
- **Navigator-Sortierung ignoriert führende Nullen** (X01313 nach X1312) – für Listen-Nachbau (Klemmenleistenübersicht)
  dieselbe „natürliche“ Sortierung verwenden.
- **Eigenschafts-IDs** sind der stabile Schlüssel für jeden Import (Makro-/EDZ-Daten, `.epj`-XML): Feldtyp und Index
  beachten (Artikelnummer 20100 [1…n]); Mehrsprachigkeit als eigener Feldtyp.
- **Prüfart „Fehler verhindern“** als Vorbild für QET-Prüfskripte: bestimmte Regeln (z. B. doppelte BMK) nicht nur melden,
  sondern Bearbeitung zurückweisen; Meldungsklassen 001–033 als Gliederung für eigene Prüfungen nutzen.
- Für QET irrelevant: Rechteverwaltung, Multiuser, Arbeitsbereiche, Terminalserver, Add-on-Registrierung.

## 13. Ergänzung: Kompatibilität, Einstellungen importieren, Internetverbindungen, Verbesserungsprogramm
**Einstellungen: Kompatibilität** (Projekte > „Projekt“ > Verwaltung > Kompatibilität) – nur für alte Projekte gedacht, in neuen nicht verwenden:
- *Anschlussbezeichnung übernehmen*: zeigt im Feld Anschlussbezeichnung zusätzlich die Bezeichnungen der Funktionen, die das BMK von der aktuellen übernehmen; aus = nur eigene, nichts wird übertragen.
- *Texte vergrößert anzeigen*: Texte wie in alten Versionen um ca. **20 % größer**, Textlängen und Zeilenabstände weichen leicht ab; aus = maßstabsgetreue Anzeige und Ausgabe.
- *Nicht normgerechte Struktureinstellung „Übergeordnet“ zulassen*: Klemmenleisten, Stecker, Kabel, Abbruchstellen ohne „-“ im BMK werden wie gesamtnummeriert behandelt; ab 2.6 in neuen Projekten nicht aktivierbar, in alten ausgegraut, solange „Übergeordnet“ verwendet wird.
- *Registerkarte „Verbindungen“ bei Klemmen und Steckerkontakten*, *Klemmen als Hauptfunktion zulassen* (Verhalten vor 2.6; empfohlen: Hauptklemme), *Teilklemmen bei Kabelbaumdaten getrennt* – siehe klemmen.md.
- *Auswertungen: Identische Funktionstexte ersetzen* (siehe formulare-platzhalter.md), *Bus-Schnittstellenname in Zielinformationen* (siehe sps.md).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektkompatibilitaet.htm

**Einstellungen importieren** (erster Start nach Versionsupdate): je Bereich Benutzer/Station/Firma wählbar, ob die Einstellungen der Vorversion (ermittelt über Registry-Einträge des Setups) übernommen werden; zusätzlich gehen Eigenschaftsanordnung, Dialoggedächtnis, Baum-/Listenkonfigurationen und externe Programme mit. Existiert die Einstellungsdatenbank eines Bereichs schon, ist er ausgegraut. Start über `W3u.exe /Quiet` unterdrückt den Dialog – dann keine Übernahme.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsettingsservicesgui_d_einstellimport.htm

**Einstellungen: Internetverbindungen** (Benutzer > Verwaltung): gilt für alle Programmkomponenten mit Internetzugriff. LAN: ohne Proxy, Proxyeinstellungen des Systems (optional mit eigenen Anmeldedaten) oder eigener Proxy über Adresse+Port oder Konfigurationsskript. Registerkarte Verbindungstest prüft jede Komponente (über den Proxy); „Browserdaten löschen“ entfernt gespeicherte Anmeldungen aller eingebetteten Browserfenster und wirkt erst nach Neustart.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.edata_r_verbindung.htm

**Kundenorientiertes Verbesserungsprogramm** (Hilfe-Menü): Ja = anonyme Nutzungs-/Konfigurationsstatistik an EPLAN; Ja oder Nein unterdrücken den Dialog beim nächsten Start, „Später entscheiden“ zeigt ihn bei jedem Start.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_benutzerfreundlichkeit.htm

**Bedeutung für QET**: Stammt ein Vorlage-PDF aus einem alten Projekt mit „Texte vergrößert“, sind die Schrifthöhen rund 20 % größer als die eingestellten Werte – beim Nachbau die im PDF gemessene Höhe übernehmen, nicht den Nennwert. „Übergeordnet“ erklärt Klemmenleisten/Kabel ohne „-“-Vorzeichen in alten Plänen.

## Quellen

Einstellungen allgemein:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_arbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_einstellungenbeimupdate.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_import_export.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_k_rechteundzugriff.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_h_einstellungenansehen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_h_einstellungenaufstandard.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_h_einstellungenaustauschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_h_importieren_exportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_h_zusatzmodulregistrieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_systeinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_darstellungmakroprojekten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellanzeigeeinheiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellerweitert.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellexportprojekt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellimportprojekt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellterminalserver.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_einstellungensprachen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_oberflaecheneinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_projektgruppendefinition.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/settingsmastergui_d_projektgruppezuordnung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsettingsservicesgui_d_einstellzusatzmodule.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsettingsservicesgui_d_zusatzmodule.htm

Schemata, Filter, Verzeichnisse, modale Dialoge:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/schemeconfig_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/schemeconfig_h_schematabearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/schemeconfig_m_schemaschaltflaechen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_k_filter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_filterkriterium.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_filternnach.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_filterwerteauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_pfadvariablen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_einstellungenverzeichnisse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_vergleicheinstell.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_r_einstellungprojekt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_mehrsprachigeeingabe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_sqlserver.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_treibersqlserver.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_einstellungenupdate.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_arbeitsbereich.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_arbeitsbereichbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_h_mitarbeitsbereichenarbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_einstellungenschaltflaeche.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_baumkonfiguration.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_spaltenkonfiguration.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_h_spaltenkonfigurieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_schemaspaltenkonfiguration.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_einstellprjstruktur.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_eigenschaftenuebersicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_eigenschaftsanordnung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_grafischevorschau.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_export.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_stammdatenabgleich.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/mf_d_farbauswahl.htm

Projekteinstellungen Betriebsmittel, Meldungen:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_betriebsmittelallgemein.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_betriebsmittelallgemeinbenutzer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_betriebsmittelbmk.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektdatum.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_einstellungenmeldungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_projekteinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesinspectiongui_d_prueflauf.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_o_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/errorhelp_k_start.htm
- Querverweis-Einstellungsseiten (nur überflogen, Inhalt in `querverweise.md`): xessettingsgui_k_start, _k_grundlagen,
  _k_querverweisarten, _k_qvwjapanische, _d_einstellungenprojektqvwallgemein, _d_einstellungenprojektqvwanzeige,
  _d_einstellungenprojektabbruchstellen, _d_einstellungenprojektmotorschutzschalter, _h_* (10 Seiten),
  xesconnectdevicegui_d_geraeteverschalten – alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/

Eigenschaften:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/properties_k_prinzip.htm

Rechte, Mehrbenutzer:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_k_windowsbenutzer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_h_benutzer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_h_benutzerwechseln.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_h_gruppen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_arbeitsmodus.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_benutzer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_gruppe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_passwortaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/uradministration_d_rechte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/urrightsmanager_d_anmelden.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_k_projektebearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_k_datensperren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_d_konflikte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesmultiuserconflictgui_h_konfliktmailversenden.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.multiusermonitor_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.multiusermonitor_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.multiusermonitor_k_vorgehensweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.multiusermonitor_d_monitor.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.multiusermonitor_d_einstellungen.htm

Oberfläche, Start, Hilfe:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_hintergrund.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_dialoge.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_besonderheitennavigatoren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_eigenschaftenuebersicht.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_symbolleisten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_dragdropfunktionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_k_iconsnavigatoren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_h_anzeigesortierreihenfolgefestlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_h_bedienelementeandocken.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_h_benutzerdefeigenschaftskonfigurationen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_h_eplanmittastaturbedienen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_h_symbolleistenanpassen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/userinterface_m_kontextmenu.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/configtoolbars_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/configtoolbars_k_aktionensymbolleisten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/configtoolbars_k_aktionenformatierungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/configtoolbars_h_schaltflaecheneinstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ot_d_externeprogramme.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ot_d_symbolleisteneditor.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ot_d_tastenkombinationen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/startingeplan_h_startenundbeenden.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/helpsystem_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/helpsystem_k_hinweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/helpsystem_h_nutzen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/helpsystem_m_hilfe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/EPLAN_Help_k_start.htm

Zusatz-Dialogseiten (Ergänzung Abschnitt 13):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektkompatibilitaet.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xsettingsservicesgui_d_einstellimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eplan.eplcoreaddin.edata_r_verbindung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogs_d_benutzerfreundlichkeit.htm

Mehrbenutzer-Ergänzung (Abschnitt 9):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xusersettingsgui_d_einstellungenmehrbenutzer.htm
