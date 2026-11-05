# Projektverwaltung in EPLAN (Plattform 2.9)

Wie EPLAN ein Projekt physisch ablegt, aus Vorlagen erzeugt, sichert, verteilt (Teilprojekte), korrigiert, bereinigt
und auf eine andere Norm umstellt. Grundlage ist die offizielle EPLAN-Plattform-Hilfe 2.9; jede Aussage trägt ihre Quelle.
Für den QET-Nachbau ist vor allem wichtig: welche Dateien zu einem Projekt gehören, was eine Vorlage überträgt und welche
Korrektur-/Komprimierungsläufe EPLAN anbietet (Vorbild für Prüf- und Aufräumskripte im Fork).

## Inhalt
1. [Projekt-Prinzip und Dateiformate](#1-projekt-prinzip-und-dateiformate)
2. [Vorlagen: Projektvorlage und Basisprojekt](#2-vorlagen-projektvorlage-und-basisprojekt)
3. [Projekt erstellen, kopieren, umbenennen, öffnen](#3-projekt-erstellen-kopieren-umbenennen-öffnen)
4. [Projekteigenschaften und Projektstruktur](#4-projekteigenschaften-und-projektstruktur)
5. [Projektverwaltung (Datenbank) und Projektkontrolle](#5-projektverwaltung-datenbank-und-projektkontrolle)
6. [Projektdaten- und Stammdatenabgleich](#6-projektdaten--und-stammdatenabgleich)
7. [Sichern, Packen, Wiederherstellen](#7-sichern-packen-wiederherstellen)
8. [Export/Import XML, Veröffentlichen](#8-exportimport-xml-veröffentlichen)
9. [Teilprojekte](#9-teilprojekte)
10. [Projekt korrigieren](#10-projekt-korrigieren)
11. [Projekt komprimieren und reorganisieren](#11-projekt-komprimieren-und-reorganisieren)
12. [Normentausch](#12-normentausch)
13. [Projektoptionen (Varianten im Plan)](#13-projektoptionen-varianten-im-plan)
    - [13a. Import von Betriebsmittel- und Verdrahtungslisten](#13a-import-von-betriebsmittel--und-verdrahtungslisten)
14. [Übergreifendes Bearbeiten auf Auswertungsseiten](#14-übergreifendes-bearbeiten-auf-auswertungsseiten)
15. [Bedeutung für QET (Zusammenfassung)](#15-bedeutung-für-qet-zusammenfassung)
16. [Ergänzung: Dialog Projekt überprüfen (nach unsauberem Schließen)](#16-ergänzung-dialog-projekt-überprüfen-nach-unsauberem-schließen)
17. [Quellen](#quellen)

---

## 1. Projekt-Prinzip und Dateiformate

- Ein EPLAN-Projekt ist eine **Datenbank**: Seiten (Schaltplan, Listen, Übersichten) plus alle darin benutzten
  Stammdaten (Symbole, Normblätter, Formulare, Artikeldaten) sind **im Projekt eingelagert**, nicht nur referenziert.
- Physisch: Projektdatei `<Name>.elk` (Verweis) + Verzeichnis `<Name>.edb` mit Datenbank und projektbezogenen
  Stammdaten. Geöffnet wird immer über die `.elk`. Löschen geht in den Windows-Papierkorb.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_start.htm

Dateiendungen der Projektdatei (der Zustand steckt in der Endung):

| Endung | Zustand | Erzeugt durch | Bearbeitbar |
|---|---|---|---|
| `.elk` | Standardprojekt (normal, exklusiv oder schreibgeschützt geöffnet) | Neu | ja |
| `.elp` | gepackt (Inhalt als `.zw0`) | Projekt > Packen | nein, erst entpacken (Öffnen entpackt automatisch) |
| `.els` | ausgelagert für externe Bearbeitung (+ `.zw1` im Sicherungsordner) | Sichern, Methode „Auslagern“ | nur lesen |
| `.elx` | ausgelagert und gepackt | Packen eines `.els` | erst entpacken |
| `.ela` | archiviert, Projekt bis auf Infodatei gelöscht (+ `.zw1`) | Sichern, Methode „Archivieren“ | nein |
| `.elr` | abgeschlossen bzw. Referenzprojekt | Revisionsverwaltung > Projekt abschließen / Referenzprojekt erzeugen | nur lesen |
| `.ell` | Revisionsprojekt mit Änderungsverfolgung | Revisionsverwaltung > Revision erzeugen | ja |
| `.elt` | temporäres Vergleichsprojekt | Revisionsvergleich | – |

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_icons.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_k_projekte.htm

Weitere Dateitypen rund ums Projekt:

| Endung | Inhalt |
|---|---|
| `.ept` | Projektvorlage (aktuell) |
| `.epb` | alte Projektvorlage (nur noch lesbar, nicht mehr erzeugbar) |
| `.zw9` | Basisprojekt |
| `.epj` | Projekt im XML-Format (Export/Import) |
| `.epdz` | Veröffentlichung für EPLAN View (gepackt: Struktur + je Seite `.svg`, je Bauraum `.e3d`) |
| `.zw1` | Datensicherung Projekt (`.000.zw1`, `.001.zw1` … bei Aufteilung) |
| `.zx1` / `.zx2` | Sicherung Basisprojekt mit Stammdaten / Projektvorlage |
| `.zw2` … `.zw8`, `.zwa` | Sicherung Symbolbibliotheken, Normblätter, Formulare, Makros, Artikeldaten, Wörterbücher, Benutzer-/Stationsdaten, Konturen |
| `.zw0` | gepackter Inhalt eines `.elp` |
| `.mdb` | Projektverwaltungsdatenbank (Access; alternativ SQL-Server) |

Sicherungen werden mit 7-Zip gepackt; Doppelklick auf eine Sicherungsdatei startet EPLAN im Modus Wiederherstellen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_prinzip.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_projektvorlagen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_exportieren.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_veroeffentlichen.htm

**Bearbeitungsmodi beim Öffnen:** Standard (Mehrbenutzer gleichzeitig), Exklusiv (nur ich, andere gesperrt),
Schreibgeschützt (grauer Texthintergrund im Navigator). Die Liste zuletzt geöffneter Projekte merkt sich den Modus.
Ist „Projektstammdaten beim Öffnen abgleichen“ (Projekt > Verwaltung > Allgemein) aktiv, fragt EPLAN bei veralteten
Projektstammdaten nach (Ja = automatisch aktualisieren, Benutzerdefiniert = Stammdatenabgleich-Dialog).
Beim Öffnen prüft EPLAN, ob die Projektdatenbank unsachgemäß verlassen wurde (Strom-/Netzausfall), und schließt sie dann wieder.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_bearbeitungsmodusbeimoeffnen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projektoeffnen.htm

### Bedeutung für QET
- Ein QET-Projekt (`.qet`) bettet seine Elemente ebenfalls ein (Projekt-Sammlung) – das entspricht dem EPLAN-Prinzip
  „Stammdaten im Projekt eingelagert“. Beim Nachbau also immer die im Projekt eingebettete Elementversion als maßgeblich
  betrachten, nicht die Benutzersammlung.
- Zustände wie „abgeschlossen“ (`.elr`) oder „ausgelagert“ (`.els`) hat QET nicht; Vermutung: nachbildbar über eine
  Projekteigenschaft (z. B. `projektstatus`) plus Schreibschutz der Datei.

## 2. Vorlagen: Projektvorlage und Basisprojekt

Jedes neue Projekt entsteht aus einer Vorlage – Projektvorlage `.ept` oder Basisprojekt `.zw9`.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_basisprojekt.htm

| | Projektvorlage `.ept` | Basisprojekt `.zw9` |
|---|---|---|
| alle Projekteinstellungen (inkl. Schemata Seiten- und BM-Struktur) | ja | ja |
| Projektdaten (platzierte und nicht platzierte Betriebsmittel) | ja | ja |
| alle Seiten | ja | ja |
| eingelagerte Stammdaten (Formulare, Symbole, Normblatt) | nein | ja |
| eingelagerte Fremddokumente/Bilder, referenzierte Daten | nein | ja (beim Erstellen eingelagert; http-Links nicht) |

- Vorlage ohne Seiten: Projekt kopieren, Seiten in der Kopie löschen, daraus Vorlage erstellen.
- Mitgelieferte Beispiele (`..\Vorlagen\EPLAN`): `IEC_tpl001.ept` / `IEC_bas001.zw9` (IEC-Kennzeichnungsstruktur),
  `Num_tpl001.ept` / `Num_bas001.zw9` (Gesamtnummerierung), `GOST_tpl001.ept` / `GOST_bas001.zw9` (GOST).
- Speicherort-Vorschlag ist das Vorlagen-Verzeichnis aus Benutzer > Verwaltung > Verzeichnisse.
- Welche Vorlage benutzt wurde, zeigt die Projekteigenschaft „Projektvorlage“.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_projektvorlagen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_basisprojekteerweitert.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_basisprojekterstellen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_projektvorlageerstellen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_projekteerstellen.htm

**Aus der Vorlage übernommen und danach anpassbar:** Standard-Normblatt (Projekt > Verwaltung > Seiten; hat keinen
Einfluss auf die Pfadnummerierung), Pfadnummerierung (gleicher Dialog; Änderung wirkt erst nach
Dienstprogramme > Pfadbezeichnungen aktualisieren), BMK-Nummerierungsformat (Betriebsmittel > Nummerierung (online)),
SPS-Nummerierungsschema (Betriebsmittel > SPS).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_normblattpfad.htm

### Bedeutung für QET
- Für die Nachbau-Projekte eine QET-„Basisvorlage“ pflegen (leeres `.qet` mit Titelblock-Vorlage, Folio-Eigenschaften,
  eingebetteten Herstellersymbolen) – das ist das Gegenstück zum Basisprojekt `.zw9`. Eine reine Einstellungsvorlage
  ohne Elemente entspricht `.ept`.
- Pfadnummerierung und Normblatt hängen in EPLAN am Projekt, nicht an der Seite; beim Import aus EPLAN daher einmal je
  Projekt auslesen und auf alle Folios anwenden.

## 3. Projekt erstellen, kopieren, umbenennen, öffnen

- **Erstellen:** Projektname ohne Endung (Sonderzeichen/Umlaute erlaubt), Speicherort, Vorlage. Optional
  „Erstellungsdatum setzen“ / „Ersteller setzen“ – sonst gelten Datum und Ersteller der Vorlage.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projekterstellen.htm
- **Kopieren** (kopiert `.elk` + `.edb`), Umfang wählbar:
  Gesamt mit Auswertung / Gesamt ohne Auswertung / Nur Kopfdaten (ohne Seiten und Auswertungen, aber mit Ebenen) /
  Nur interaktive Seiten (Schaltplan allpolig/einpolig, Fluid, Grafik, Fremddokument, Übersicht, Schaltschrankaufbau,
  RI-Fließbild; ohne Auswertungen und ohne nicht platzierte Funktionen). Die Kopie eines ausgelagerten Projekts ist nicht ausgelagert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projektkopieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projektkopieren.htm
- **Umbenennen:** ausgelagerte/archivierte Projekte nicht. Ein vorhandener Zielname wird nach Rückfrage überschrieben
  (alter in den Papierkorb).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projektumbenennen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projektumbenennen.htm
- Öffnen per Drag & Drop einer `.elk` in den Seiten-Navigator möglich.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_bearbeitungsmodusbeimoeffnen.htm

## 4. Projekteigenschaften und Projektstruktur

Der Projekteigenschaften-Dialog (Seiten-Navigator > Projekt > Eigenschaften, oder Projektverwaltung) hat die
Registerkarten **Eigenschaften**, **Statistik**, **Struktur**, **Status**.

- **Eigenschaften:** frei hinzufügbare/löschbare Projekteigenschaften (Dialog Eigenschaftsauswahl, gefiltert nach
  Kategorie). Automatisch erzeugte Werte sind grau und nicht editierbar. Kunden-/Herstelleradressen lassen sich aus der
  Artikelverwaltung in die Eigenschaftsgruppen „Ersteller“ (aus Hersteller/Lieferant), „Kunde“ und „Endkunde“ einlesen
  (überschreibt vorhandene Werte).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_eigenschaften.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projekteigenschaftenloeschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_kundendateneinlesen.htm
- **Statistik:** Seitenanzahl je Seitentyp, eingefrorene Auswertungsseiten, eingegebene vs. erzeugte Auswertungsseiten,
  Gesamtseitenzahl, Anzahl Bauräume.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_statisitk.htm
- **Status:** zuletzt ausgeführte Prüfläufe mit Datum, EPLAN-Version/Build, Prüfschema (z. B. „Offline“) und Anzahl
  Fehler/Warnungen/Hinweise (identisch mit Meldungsverwaltung).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_status.htm
- **Struktur:**
  - *Erweiterte Referenzkennzeichnung* (EN 81346): Funktionsaspekt = Anlage `=`, Ortsaspekt = Einbauort `+`,
    Produktaspekt = Produktkennzeichen `-`. Aus: Verhalten wie EPLAN ≤ 2.2, Produktkennzeichen kein Strukturelement.
  - *Seiten*: Schema der Seitenstruktur; wird beim Erstellen festgelegt, bei Projekten mit Seiten nur eingeschränkt
    änderbar. Im Schema je Kennzeichnungsblock „identifizierend / beschreibend / nicht verfügbar“ und die Reihenfolge
    (bestimmt die Baumreihenfolge im Seiten-Navigator). Objektkennzeichen nach DIN EN 61355-1 nur zusammen mit
    Dokumentenart und nur bei aktiver erweiterter Referenzkennzeichnung.
  - *Betriebsmittel*: je BM-Art (allgemein … Mechanik) ein Kennzeichnungsschema (gleiche Tabelle).
  - *Übergeordnet* (Klemmenleisten, Stecker, Kabel, Abbruchstellen): BMK ohne `-` werden wie gesamtnummeriert behandelt
    (Beispiel: `-X3` → `=Z1+A1-X3`, aber `X3` bleibt `X3`). Abgekündigt; nur sichtbar, wenn schon benutzt oder unter
    Verwaltung > Kompatibilität freigeschaltet.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_struktur.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_seitenstruktur.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_betriebsmittelstruktur.htm
- **[Weitere] > Darstellung:** Vorzeichen und Unterkennzeichen-Trennzeichen: Funktionale Zuordnung `==`, Anlage `=`,
  Aufstellungsort `++`, Einbauort `+`, Produktkennzeichen `-` (Unterkennzeichen-Trenner jeweils `.` oder das Vorzeichen
  selbst), Dokumentenart `&` (oder Leerzeichen), Anlagennummer frei, ein benutzerdefinierter Block.
  „Auch zwischen Ziffer und Buchstabe trennen“: Wechsel Ziffer→Buchstabe gilt als Unterstruktur
  (`-K01Q04S01` = `-K01`,`Q04`,`S01`; aus: Anzeige `-K01.Q04.S01`). Produktkennzeichen „zur Strukturierung einsetzen“:
  `-` immer im vollständigen BMK, Produktkennzeichen auch an Abbruchstellen geschachtelt. Punkt als Trenner beim
  Produktkennzeichen kollidiert mit Unterzählern.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_darstellungerwprjstrukturen.htm
- **[Weitere] > Vererbung** (wie Schaltzeichen Kennzeichen von der Umgebung erben):
  - Verschachtelte Strukturkästen: an → `=A2.A3.A4+O2.O3.O4-F3`; aus → nur innerster Kasten `=A4+O4-F3`.
  - Gerätekasten vererbt BMK an allgemeine BM (`=AP+ST1-U1-H1` statt `=AP+ST1-H1`), getrennt schaltbar für
    Klemmenleisten, Stecker, Kabel, Fluid; „Gewerkübergreifend“ standardmäßig aus.
  - „Strukturkennzeichen an Schaltzeichen immer schachteln“: `=.A2-F1` in Umgebung `=A.A1` → `=A.A1.A2-F1`.
  - „Struktur zur Seitenstruktur“: Seitenkennzeichen wird mit dem des Schaltzeichens verschachtelt
    (`=DD+EE-M1` auf Seite `=AB+OC/3` → `=AB.DD+OC.EE-M1`); Ausnahme: BMK beginnt mit `>`.
  - „Strukturen von Gerätekästen an Abbruchstellen vererben“ (Verhalten wie EPLAN 21).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_vererbung.htm

### Bedeutung für QET
- Die Kennzeichen-Vererbung (Seite → Strukturkasten → Gerätekasten → Schaltzeichen) muss das Verknüpfungsskript
  nachbilden, wenn es aus sichtbaren Kurz-BMK das vollständige BMK rekonstruiert: zuerst Folio-Kennzeichen (`=`/`+`)
  als Umgebung, dann umgebende Kästen, `>` am Anfang beendet die Vererbung. Details zur Kennzeichnung in `kennzeichnung.md`.
- Die Trennregel „Ziffer/Buchstabe“ beeinflusst, ob `-K01Q04` ein oder drei Strukturebenen ist – beim Parsen von
  EPLAN-BMK aus PDF die Projekteinstellung nicht kennbar; Vermutung: Standard ist ausgeschaltet, also `.` als Trenner werten.

## 5. Projektverwaltung (Datenbank) und Projektkontrolle

- Die Projektverwaltung arbeitet immer mit einer **Projektverwaltungsdatenbank** (Access `.mdb`, Standard in
  `$(MD_PROJECTS)`, oder SQL-Server; bis 2.3 fest `Projects.mdb`). Feld leer = ohne Projektverwaltung.
  Einstellung: Benutzer > Verwaltung > Projektverwaltungsdatenbank.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projekteverwalten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_prjverwaltungsdb.htm
- Verzeichnisse einlesen (rekursiv, nur Ordner mit Projekten), Baum unterscheidet eigene Arbeitsstation / Server
  (UNC-Freigabe) / fremde Arbeitsstation; geöffnete Projekte fett. Filter über Projekteigenschaften und Suchverzeichnis.
  Ansichts- vs. Bearbeitungsmodus ([Bearbeiten]/[Speichern]). Markieren eines Ordners = alle Projekte darin
  (z. B. „Automatisiert bearbeiten“ für alle). Weitere Funktionen: Projektliste als `ProjList.xml`, Projektinfo als
  `PrjInfo.htm`, Benutzer anzeigen (wer arbeitet gerade im Projekt, mit Modus), Projektstammdaten aktualisieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projektverwaltung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_verzeichniseinlesen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projektefiltern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projektinfoausgeben.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_benutzeranzeigen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projekteigenschaftenanzeigen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projektanlegen.htm

**Projektkontrolle** (Eingangskontrolle eines von Dritten zurückgelieferten Projekts):
- Vergleichsprojekt = Ursprungsstand (`.elk`, `.ell`, `.els`, `.elr`, `.elt`); Eingangsprojekt = fortgeschrittener Stand.
- Ablauf: Referenzprojekt (`.elr`) erzeugen → Projekt versenden → Rücklauf → Schemata anlegen → kontrollieren.
- Prüfbar: Einstellungen (Einstellungsvergleich-Schema über alle Projekteinstellungen), Eigenschaften
  (Eigenschaftenvergleich-Schema je Datengruppe Betriebsmittel, Verbindungen, Grafik, Seiten, Potenziale, Projekt),
  Prüfläufe (Prüflaufschema: je Meldung Kategorie Fehler/Warnung/Hinweis und Prüfart Online/Offline/Nein),
  Projektstrukturen, Ebenen (optional neue Ebenen melden, z. B. aus DXF-Import), „beim ersten Unterschied stoppen“.
- Ergebnis im Dialog Projektkontrolle: Ergebnisse (Ampel je Vergleich) und detailliert in der Meldungsverwaltung des
  Eingangsprojekts. Bei unterschiedlichen Programmversionen werden Neuerungen evtl. nicht gemeldet.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_k_projekte.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_k_vorgehensweise.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_h_eingangskontrolvorber.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_h_vergleichsschema.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_h_eigenschaftenvergleich.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_h_pruefschema.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projekteingangskontrolle.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_ergebnisse.htm

### Bedeutung für QET
- Die Projektkontrolle ist das Vorbild für einen Nachbau-Abgleich: „Referenzstand“ (Original-EPLAN-PDF bzw. vorige
  `.qet`-Fassung) gegen „Eingangsstand“ (neue `.qet`), getrennt nach Einstellungen, Eigenschaften, Struktur und
  Prüfläufen, Ergebnis als Meldungsliste. `qet_diff` + `qet_check` decken Eigenschaften bzw. Prüfläufe ab.

## 6. Projektdaten- und Stammdatenabgleich

- **Projektdatenabgleich** (Projekt > Organisieren > Abgleichen): überträgt Ebenen, benutzerdefinierte Eigenschaften,
  Eigenschaftsanordnungen, Projekteinstellungen, Prüflaufschemata, Auswertungsvorlagen aus einem Quellprojekt
  (z. B. Basisprojekt). Status je Eintrag (Neuer/Älter/Identisch/Unterschiedlich). „Projekt aktualisieren“ ersetzt
  veraltete/abweichende Daten, „Projekt vervollständigen“ ergänzt nur fehlende. Abgleich von Typ „Einstellung“ ist
  **nicht rückgängig** zu machen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_projekteabgleichen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projektdatenabgleich.htm
- **Stammdatenabgleich** (Dienstprogramme > Stammdaten > Aktuelles Projekt abgleichen): Projektstammdaten ↔
  Systemstammdaten in beide Richtungen; Stammdaten werden eingelagert (kopiert), nicht referenziert. Vergleich über
  Änderungsdatum; Dateinamen ohne Unterscheidung von Groß-/Kleinschreibung. Bei Symbolbibliotheken prüft EPLAN die
  Kompatibilität (z. B. Anschlussbezeichnungen) und bricht sonst ab. Verzeichnisse als Schema wählbar
  (je Kunde eigene Stammdatenpfade).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_stammdatenabgleich.htm

### Bedeutung für QET
- Gegenstück in QET: eingebettete Elemente im Projekt gegen die Benutzersammlung abgleichen. Regel aus EPLAN übernehmen:
  Austausch eines Symbols nur, wenn die Anschlussbezeichnungen kompatibel sind – sonst würden Leiter abreißen.

## 7. Sichern, Packen, Wiederherstellen

- **Prinzip:** Sichern und Wiederherstellen sperren das Projekt – nicht mehrbenutzerfähig, alle Bearbeiter müssen raus;
  Editor-/Navigatorfenster des Projekts werden geschlossen. UNC-Pfade werden durchgängig unterstützt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_prinzip.htm
- **Methoden** (Projekt > Sichern > Projekt):
  - *Zusätzlich speichern*: Kopie als `.zw1`, `.elk` bleibt.
  - *Auslagern für externe Bearbeitung*: Kopie `.zw1`, Original wird `.els` (schreibgeschützt; Bearbeitungsversuch
    meldet „ausgelagert“). Zweck: Weitergabe an Unterlieferanten, ohne dass parallel geändert wird.
  - *Archivieren*: Kopie `.zw1`, Original bis auf Infodatei gelöscht, wird `.ela`.
  Optionen: „Projekt komplett sichern“ (auch redundante Index-Dateien → schneller wiederherstellen), „Referenzierte Daten
  kopieren“ (Fremddokumente/Bilder nach `<Projekt>\DOC` bzw. `\Images` einlagern), eingelagerte Fremddokumente/Bilder
  mitsichern. Eingelagerte Stammdaten werden immer gesichert. Pfad+Dateiname max. 260 Zeichen. Medium Datenträger oder
  E-Mail (MAPI, Teilgröße 1–10 MB oder unbegrenzt, Betreff „<Projekt> n/m“). Einstellungen werden je Projekt gemerkt;
  Benutzer-Voreinstellungen gelten nur beim ersten Sichern bzw. bei Mehrfachauswahl.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_d_projektesichern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_arbeitsweise.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_h_projektesichern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_start.htm
- **Aufgeteilte Sicherung:** Teile `.000` bis `.999`; zum Wiederherstellen alle Teile in einen Ordner, nur `.000.zw1`
  wird angezeigt, EPLAN fügt zusammen. Deshalb keine Dateinamen mit `.<dreistellige Zahl>` verwenden.
  Korrektur/Hinweis: Die Prinzip-Seite nennt im Beispiel `Mein_Projekt.000.elk`; die E-Mail-Seite und der Wiederherstellen-
  Dialog sprechen von `*.000.zw1` – gemeint ist die Sicherungsdatei `.zw1`.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_h_projektmailversenden.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_prinzip.htm
- **Fremddokumente:** gesichert wird nur, was im Projektverzeichnis liegt (Unterordner Dokumente bzw. Bilder);
  beim Einfügen entscheidet man Kopie ins Projekt vs. Referenz. Gleiches gilt für Hyperlinks.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_prinzip.htm
- **Langzeitarchiv:** EPLAN-Archivierung (ZIP-Format) plus datenbankunabhängige Exporte DXF/DWG, Bild (TIFF, JPEG, BMP), PDF.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_prinzip.htm
- **Packen** (Projekt > Packen): `.elk` → `.elp` im selben Ordner, Projekt wird geschlossen; Öffnen/Doppelklick
  entpackt automatisch; „Entpacken“ entpackt ohne zu öffnen. Packen behält alle Daten (anders als Komprimieren).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_h_packen.htm
- **Wiederherstellen:** Ziel frei wählbar, optional ursprüngliches Unterverzeichnis, anderer Projektname.
  Existiert das Projekt: nicht geöffnet → Rückfrage; von mir geöffnet → Rückfrage, dann schließen/überschreiben/neu öffnen
  (aktuelle Änderungen gehen verloren); von anderem geöffnet → Abbruch. Stammdaten werden projektbezogen eingespielt und
  mit dem Stammdatenpool verglichen; geänderte Projekteinstellungen erzeugen Vergleichsmeldungen. Sicherungsmeldungen
  stehen unter Dienstprogramme > Systemmeldungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakrestoredlggui_h_projektewiederherstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakrestoredlggui_d_projektewiederherstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakrestoredlggui_d_inhaltsicherungsdatei.htm
- **Stammdaten sichern/wiederherstellen** (Basisprojekte, Projektvorlagen, Symbolbibliotheken, Normblätter, Konturen,
  Formulare, Makros, Artikeldaten, Wörterbücher, projektunabhängige Einstellungen): immer nur Kopie (kein Auslagern/
  Archivieren); ohne Markierung wird alles gesichert; Grafiken in Normblättern/Formularen nur, wenn eingelagert.
  Beim Wiederherstellen Hinweis, welche Datei neuer ist.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupmdatadlggui_h_stammdatensichern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupmdatadlggui_d_stammdatensichern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakrestoremdatadlggui_h_stammdatenwiederherstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakrestoremdatadlggui_d_stammdatenwiederherstellen.htm
- **Nicht-ASCII-Stammdatennamen** aus alten Versionen werden beim Öffnen/Wiederherstellen nach Unicode konvertiert
  (Abfrage „Suchindizes aktualisieren“); danach nicht mehr in älteren EPLAN-Versionen bearbeiten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_prinzip.htm

### Bedeutung für QET
- Wenn Birger EPLAN-Projekte als `.zw1` bekommt: das ist ein 7-Zip-Archiv des `.edb`-Ordners. Vermutung: mit 7-Zip
  entpackbar, liefert aber die proprietäre Projektdatenbank – für den QET-Import ist der EPLAN-PDF- oder XML-Export
  (`.epj`) der praktikablere Weg.

## 8. Export/Import XML, Veröffentlichen

- **Export** (Projekt > Organisieren > Exportieren): ganzes Projekt als `.epj` im XML-Format, in externen Editoren
  bearbeitbar und reimportierbar. **Import**: `.epj` wählen, optional „Artikelabgleich durchführen“, anderer Zielname.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_exportieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_importieren.htm
- **Veröffentlichen** (Projekt > Veröffentlichen): `.epdz` für die EPLAN View App; enthält Projekt-, Seiten-, Bauraum-
  und Funktionseigenschaften, je Seite eine `.svg`, je Bauraum eine `.e3d`. Schema in Einstellungen: Veröffentlichen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_veroeffentlichen.htm

### Bedeutung für QET
- `.epj` (XML, ganzes Projekt) und `.epdz` (gepackt, SVG je Seite + Funktionseigenschaften) sind die beiden offenen
  Wege, an EPLAN-Projektdaten zu kommen, ohne die Datenbank zu lesen. Vermutung: `.epdz` ist ein ZIP; die SVGs plus
  Funktionseigenschaften könnten für einen Importer reichen (Geometrie + BMK), falls Kunden so etwas liefern.

## 9. Teilprojekte

Zweck: große Projekte an externe Bearbeiter ohne Netzzugang verteilen. Basis sind **Bearbeitungsbereiche**
(vom Projektadministrator definiert, dürfen sich nicht überlappen = kein Objekt in zwei Bereichen).
Dialog Projekt > Teilprojekte nur bei **exklusiv** geöffnetem Projekt (EPLAN öffnet notfalls exklusiv neu).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_k_grundlagen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_d_teilprojekte.htm

- **Auslagern:** Projekt kopieren mit „Nur Kopfdaten“ (alle Eigenschaften, Einstellungen, Stammdaten werden vererbt) +
  Seiten, Bauräume, Vorplanungssegmente des Bereichs. Name `<Original>#003` (dreistellig, Reihenfolge der Bereiche;
  Unterteilprojekt `#003#002`). Im Teilprojekt: Eigenschaft **Teilprojekt von (ID 25100)** = Originalname, Teilprojektnummer;
  Funktionen und Abbruchstellen mit Querverweis nach außerhalb bekommen **Querverweis (extern) (ID 25102)** und
  **Änderungsschutz (ID 3014)**. Im Original: betroffene Seiten/Bauräume/Segmente/Strukturkennzeichen gesperrt (ID 3014),
  Nummer in **Teilprojektnummer (ID 25101)**. „Nur erweitern“: Objekte des Teilprojekts mit ID 3014 geschützt, nur
  Neues hinzufügbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_k_auslagern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_d_teilprojekte.htm
- **Querverweis (extern):** `@` im Wert steht für die Position in der Querverweisliste; mehrere externe Ziele mehrzeilig;
  bei Kontaktspiegeln zusätzlich Kontaktspiegelsymbol mit Anschlussbezeichnungen. Optionales Präfix über
  Projekt > Querverweise / Kontaktspiegel > Allgemein > „Präfix für Querverweis (extern)“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_k_grundlagen.htm
- **Einlagern:** Seiten/Bauräume/Segmente 1:1 zurück, überschreiben das Original, externe Querverweise und Schutz werden
  entfernt, Teilprojekt verschwindet. Bei Änderungen im geschützten Original oder Objekten außerhalb des Bereichs:
  Rückfrage + Prüflaufmeldungen. Danach Strukturkennzeichen-Sortierung prüfen, Auswertungen aktualisieren.
  [Aktualisieren] übernimmt neue Stände ohne erneutes Auslagern.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_k_einlagern.htm
- Regeln: Original/Teilprojekte nicht umbenennen oder verschieben (Zuordnung geht verloren); projektweite Einstellungen,
  Eigenschaften und Stammdaten erst nach dem Einlagern aller Teilprojekte ändern.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_k_grundlagen.htm

### Bedeutung für QET
- Das Konzept „Querverweis (extern)“ ist exakt das, was der Fork für projektübergreifende Verweise bräuchte: am
  Schaltzeichen den fremden Verweistext als Eigenschaft speichern und so anzeigen, als wäre das Gegenstück da.
  Für den Nachbau großer Pläne in mehreren `.qet`-Dateien: Verweise in andere Dateien als Text-Eigenschaft mit
  Präfix führen, statt sie zu verwerfen.

## 10. Projekt korrigieren

Projekt > Organisieren > Korrigieren: Korrekturläufe über das ganze Projekt (Schema + Filter, z. B. nach Strukturkennzeichen,
Kennbuchstaben, Funktionsdefinition), nur mehrpolige Funktionen; Ergebnis in den Systemmeldungen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ProjectCorrectionGui_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ProjectCorrectionGui_h_projektkorrigieren.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ProjectCorrectionGui_d_projektkorrigieren.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ProjectCorrectionGui_d_korrektureinstellungen.htm

| Bereich | Korrektur | Wirkung |
|---|---|---|
| Klemmen | Anzahl Ziele bei fehlerhaften Klemmen vertauschen | tauscht in der Anschlusslogik „Anzahl Ziele“ von Anschluss 1 und 2, wenn die echte Zielzahl spiegelverkehrt passt |
| Klemmen | Fehlende Klemmenleistendefinitionen hinzufügen | erzeugt nicht platzierte Leistendefinition für Leisten, die nur über Klemmen-BMK existieren |
| Klemmen | Gleiche Bezeichnungen zulassen | setzt „Gleiche Bezeichnung erlaubt“, doppelte Klemmennummern nicht mehr als Fehler |
| Klemmen | Manuelle Stegbrücken korrigieren | Brückeneinstellung ab 2.5 nur an der Klemme am Brückenanfang (Altprojekte/Makros ≤ 2.4) |
| Stecker | Fehlende Steckerdefinitionen hinzufügen | je nach Kontakten Definition für Stifte, Buchsen oder beides (ggf. zwei je BMK) |
| Kabel | Bemerkung → Funktionstext | für Kabeldefinitionslinien und Schirme, nur wenn Funktionstext leer, inkl. Anzeigeeigenschaften |
| Kabel | Leerzeichen aus Querschnitt entfernen | „1,5“, „ 1,5“, „1,5 “ sonst verschieden; Ausgabe mit Einheit über ID 20070 |
| Verbindungen | Verbindungssymbole außerhalb eines Netzes korrigieren | setzt „Ziele festlegen“ wieder an T-Stücken/Kreuzungen/Brücken ohne Netzdefinitionspunkt |
| SPS | Anschlussbezeichnungen zurückschreiben | von SPS-Übersicht auf Schaltplan-SPS-Anschlüsse (Zuordnung über SPS-Adresse; Anschluss muss an Klemme/Stecker hängen) |
| SPS | … und an Steckerkontakte/Klemmen übertragen | zusätzlich Steckerbezeichnung → sichtbares BMK, Anschlussbez. → Klemmen-/Kontaktbezeichnung (nur wenn Klemmen im SPS-Kasten mit gleichem BMK) |
| SPS | Datentypen setzen | Datentyp aus Kennung, nur leere Felder |
| SPS | Bus-Schnittstellennamen abgleichen | Name an alle Darstellungsarten desselben Bus-Anschlusses (nur wenn eindeutig) |
| SPS | Namen der physikalischen Netze abgleichen | vom Bus-Master auf Slaves gleichen logischen Netzes und Konfigurationsprojekts; Prüfläufe 004103/004104 |
| SPS | Überflüssiges Konfigurationsprojekt löschen | wenn gleich dem des SPS-Kastens |

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ProjectCorrectionGui_k_korrekturdaten.htm

### Bedeutung für QET
- Gute Vorlage für ein Korrekturskript im Fork nach `gesamt.py`/`verknuepfen.py`: (1) fehlende Klemmenleisten-/
  Steckerdefinitionen anlegen, (2) Querschnitte normalisieren (Leerzeichen trimmen, Komma), (3) SPS-Anschluss-
  bezeichnungen von der Kartenübersicht auf die Kanaldarstellungen im Schaltplan übertragen (Zuordnung über SPS-Adresse).
- Punkt (3) zeigt: in EPLAN ist die SPS-Übersicht die Quelle der Anschlussbezeichnungen, der Schaltplan wird daraus
  befüllt – beim Nachbau Widersprüche zugunsten der Übersicht auflösen (Vermutung, gilt nur wenn so korrigiert wurde).

## 11. Projekt komprimieren und reorganisieren

**Komprimieren** (Projekt > Organisieren > Komprimieren; Schema + Filter, projektweit, Ergebnis als Systemmeldung)
entfernt nicht verwendete Daten. Wichtigste Gruppen:
- *Allgemein:* Suchergebnis- und Meldungsliste leeren, unbenutzte Fremddokumente/Bitmaps, Strukturkennzeichen, Ebenen,
  Symbolbibliotheken, Formulare, Normblätter, Artikel; Nicht-Übersetzungssprachen aus mehrsprachigen Texten;
  alle Projektoptionen; Elementgruppen und geschützte Gruppen auflösen (Grafik/Makroinhalt bleibt); Eigenschaft
  „Makro“ (ID 20468) an Schaltzeichen leeren; „Automatisch erzeugt“ zurücksetzen; unbenutzte Eigenschaftsanordnungen;
  AutomationML-GUIDs; Bus-Daten an nicht-einpoligen Bus-Anschlüssen; Verbindungsdefinitionspunkte ohne Verbindung.
- *Betriebsmittel überarbeiten:* Reserve-BM (alle Funktionen nicht platziert), nicht platzierte doppelte Funktionen
  (gleiche identifizierende Eigenschaften wie eine andere), überzählige Gerätefunktionen (nicht von Geräteschablone überlagert).
- *Nicht platzierte Funktionen:* Kabel-/Schirmdefinitionen, Gerätekästen, Geräteanschlüsse, SPS-Kästen, SPS-Klemmen,
  Klemmen, Stecker, Leisten-/Steckerdefinitionen (mit oder ohne Kontakte), einpolige, externe, alle übrigen.
- *Nicht platzierte Verbindungen:* fehlerhafte (Ziele fehlen), doppelte (gleiche Ziele wie platzierte), alle übrigen.
- *Platzierte Funktionen:* Platzhalterobjekte, Makrokästen, leere Leisten-/Steckerdefinitionen.
- *Vollständig nicht platzierte BM* und *nicht platzierte Funktionen ohne Hauptfunktion* je Art.
- *Symbole:* unbenutzte gedrehte/gespiegelte Varianten E–H bzw. alle unbenutzten Symbole.
- *Netzdefinitionspunkte:* ungültige Verbindungen entfernen, zusammenfassen (grafisch erster gewinnt), überflüssige
  entfernen (Eigenschaften wandern in einen Verbindungsdefinitionspunkt), alle entfernen und Verbindungssymbole
  korrigieren (T-Stücke zeigen dann wieder die Zielverdrahtung).
- *Revisionsverwaltung:* Ergebnisse des Eigenschaftenvergleichs löschen. *3D-Daten*, *Schutz* (Geräteschutz,
  manueller und automatischer Änderungsschutz aus Teilprojekt, Vorplanung, Pro-Panel-Import).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectcleangui_k_komprimierungsdaten.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectcleangui_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectcleangui_h_projektsaeubern.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectcleangui_d_komprimierungseinstellungen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectcleangui_d_projektkomprimieren.htm

**Reorganisieren** (Projekt > Organisieren > Reorganisieren): entfernt gelöschte Daten endgültig, verkleinert die
Datenbank. *Erweiterter Modus*: doppelte Artikel entfernen, Verbindungen löschen und neu erzeugen/optimieren,
Feldlängenüberschreitungen (zu lange Kennzeichen/Texte, sonst abgeschnitten) durch Umbau der Datenbank auf
Datenlänge + 10 % beheben, Sortier-IDs der Projektstrukturen prüfen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_projektreorganisieren.htm

### Bedeutung für QET
- „Verbindungen löschen und neu erzeugen“ zeigt: EPLAN-Verbindungen sind abgeleitete Daten aus der Grafik. Im QET-Nachbau
  ebenso: Leiter/Potenziale aus der Zeichnung ableiten, nicht als eigene Wahrheit pflegen.
- Ein QET-Aufräumskript kann sich an der Liste orientieren: unbenutzte eingebettete Elemente entfernen, doppelte
  „nicht platzierte“ Elemente (in QET: Elemente ohne Folio gibt es nicht – entspricht verwaisten Querverweis-Partnern).

## 12. Normentausch

Dienstprogramme > Normentausch erzeugt aus einem Quellprojekt ein **neues Zielprojekt** (Quelle bleibt unverändert).
Assistent: Projekt → Drehung/Spiegelung → Stammdaten → Symbole → Struktur → Nummerieren.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_projektanpassen.htm

- Nur Objekte **innerhalb des Auswertungsbereichs des Normblatts** (Zeilen/Spalten) werden ins Zielprojekt kopiert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_projektanpassen.htm
- *Projekt:* Zielname, Speicherort, „Projekteigenschaften aus Quellprojekt übernehmen“ (sonst aus Vorlage;
  benutzerdefinierte Eigenschaften immer aus der Quelle).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_projekt.htm
- *Drehung/Spiegelung + Vorlage `.ept`:* 0° (keine Drehung), 90° (Linien/Texte/Hyperlinks und Einfügepunkte 90° gegen
  Uhrzeigersinn; IEC→NFPA, Schema „Special-IEC-NFPA-IEC“, Einzeln zuordnen automatisch an), 270° (NFPA→IEC).
  Symboldrehung selbst über die Variante im Zuordnungsschema. Nicht gedreht werden Grafik-, Schaltschrankaufbau-,
  Fluid-, RI-, Modell-Ansicht-Seiten (jeweils interaktiv) und alle Auswertungsseiten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_drehenspiegeln.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_vorlagefestlegen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_k_seitendrehung.htm
- *Stammdaten:* Formulare, Normblätter, Funktionsdefinitions- und Symbolbibliotheken je Zeile durch „Neuer Name“ ersetzen
  (leer = aus Quelle kopiert). Normblatt bestimmt Format, Hoch/Quer, Schriftkopf, Raster, Pfade und Positionsbereiche.
  Zuordnungen werden benutzerbezogen projektübergreifend gespeichert (Benutzer > Normentausch > Projektstammdaten/Symbolschema).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_projektstammdaten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_stammdatenaustauschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_k_stammdaten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_d_projektstammdaten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_d_symbolschema.htm
- *Symbolaustausch:* ganze Bibliothek oder einzeln per Zuordnungsschema (Quellsymbole automatisch eintragen, passende
  Zielsymbole vorschlagen). Status „OK“ nur wenn **Funktionsdefinition gleich** und **Anschlusszahl Ziel ≥ Quelle**;
  Anschlussrichtungen müssen zur Drehung passen; gleicher Aufbau und gleiche Größe nötig. Symbole mit Nummer > 3000
  sind vom Normentausch ausgenommen; IEC_single_Symbol → NFPA_single_Symbol nur bedingt, manuelle Nacharbeit.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_k_symbolaustausch.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_d_symboletauschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_stammdatenersetzen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_symboletauschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_symbole.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_d_quellzielbibliothekwaehlen.htm
- *Struktur:* Seiten- und BM-Kennzeichnungsschemata (aus Vorlage, anpassbar), Trennzeichen über [Weitere].
  *Nummerieren:* optional alle BM neu nummerieren (sinnvoll, wenn die Strukturen nicht passen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_struktur.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_projektstrukturanpassen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_nummerierung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_nummerieren.htm

### Bedeutung für QET
- Die Austauschregel ist direkt auf den Fork übertragbar, wenn ein Symbol in einem bestehenden Plan durch ein anderes
  (z. B. Herstellersymbol statt QET-IEC-Symbol) ersetzt wird: gleiche Funktion (Elementtyp/Kontaktart), Zielsymbol hat
  mindestens so viele Anschlüsse, Anschlüsse in gleicher Lage/Richtung, Anschlüsse über Namen zuordnen – sonst Leiter neu ziehen.
- Objekte außerhalb des Normblatt-Auswertungsbereichs (Rand, Schriftfeld) gehören für EPLAN nicht zum Schaltplan –
  beim PDF-Nachbau Elemente im Schriftfeld nicht als Betriebsmittel übernehmen.

## 13. Projektoptionen (Varianten im Plan)

- Hierarchie: **Projektoptionengruppe** → **Projektoption** → **Ausschnitte**. Ausschnitt = Menge von Seiten *oder*
  Funktionen (nicht gemischt), auch Schaltplanausschnitte und Bauraumobjekte. Im Projekt gespeichert.
- Ein-/Ausschalten wirkt auf Schaltplan und Auswertungen (ausgeschaltet = nicht ausgewertet). Je Gruppe ist höchstens
  eine Option an (neue Option schaltet die anderen der Gruppe aus); alle aus ist möglich. Grünes Häkchen = an.
- Ausgeschaltete Optionen transparent (heller) oder ausgeblendet darstellbar – nur Konstruktionshilfe; ganze Seiten und
  nicht platzierte Funktionen nur ausblendbar.
- Löschen einer Option/eines Ausschnitts: war sie **an**, bleiben die Objekte; war sie **aus**, werden die Objekte gelöscht.
- Seite gelöscht, auf der ein Ausschnitt eingeblendet werden soll → Option nicht mehr einschaltbar.
- Platzhalterobjekte (mit Wertesätzen) hängen ebenfalls im Projektoptionen-Navigator; Auswertung „Optionsübersicht“.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_d_optionsnavigator.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_h_optioneneinaus.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_h_optionenerzeugen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_h_ausschnitteerzeugen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_h_ausschnittebearbeiten.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_d_ausschnitt.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_d_option.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_d_optionsgruppe.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_d_platzhalternavigator.htm

- Prinzip: ein Ausgangsprojekt mit allen denkbaren Maschinenvarianten (Serienmaschinenbau), je Auftrag Optionen zu-
  und abschalten. Gruppierungen kommen nur vollständig oder gar nicht in einen Ausschnitt. **Position und Nummerierung
  der Schaltzeichen ändern sich beim Umschalten nicht** – Konsistenz der Varianten ist Sache des Konstrukteurs. Nach
  Umschalten verbindungsrelevanter Optionen Verbindungen mit [Umschalt] neu erzeugen. Nur im Grafischen Editor, nicht in
  Stammdateneditoren. Suche findet Objekte in ausgeblendeten Optionen nur mit Kontrollkästchen „Ausgeblendete Projektoptionen“.
- Auswertung: Projektoptionenübersicht (Eigenschaften Gruppe/Option Name+Beschreibung, „Projektoption aktiv“, Ausschnitt
  Name/Beschreibung indiziert, Platzhalterobjekt Name/Wertesatz/Platzierung) und Platzhalterobjektübersicht. Nach
  Entfernen aller Optionen (z. B. Komprimieren) diese Auswertungen nicht mehr aktualisieren.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_k_prinzip.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_k_auswertungen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_h_optionsgruppenerzeugen.htm

### Bedeutung für QET
- In einem EPLAN-PDF sind nur die eingeschalteten Optionen sichtbar; ausgeschaltete fehlen ganz (oder erscheinen nicht
  ausgewertet). Lücken in Pfaden, Seitennummern oder BMK-Zählern können daher absichtlich sein – nicht „auffüllen“.
- QET kennt keine Varianten; Vermutung: nachbildbar über Ebenen/Folio-Gruppen, aber für den 1:1-Nachbau nicht nötig.

## 13a. Import von Betriebsmittel- und Verdrahtungslisten

- Projektdaten > Betriebsmittel > Importieren bzw. Verbindungen > Importieren > Verlegeverbindungen; Quellen TXT, CSV
  (Unicode, kommagetrennt), ASC, Excel (XLS/XLSX/XLSB/XLSM). Einstellungen: Tabelle (Excel), Anzahl Kopfzeilen,
  Zeile mit Feldbezeichnungen, Trennzeichen (Leer, Tab, beliebig). Feldzuordnung als Schema (vordefiniert „Standard“).
- Ergebnis: **nicht platzierte** Betriebsmittel bzw. **nicht verlegte** Verlegeverbindungen, danach im Bauraum platzieren/verlegen.
- Pflichtzuordnungen: Hauptfunktion braucht **BMK (vollständig) ID 20006**; Funktionsdefinition über **Artikelnummer [x]
  ID 20100 x** (erste Funktionsschablone des Artikels, oder Makro, wenn „Beim Geräte erzeugen das Makro berücksichtigen“
  bzw. keine Schablone), sonst direkt **Funktionsdefinition ID 20026**. Klemmen/Stecker: zusätzlich
  **Klemmen-/Steckerkontaktbezeichnung ID 20030**; Klemmen mit Artikel werden Hauptklemmen; **Etage ID 20034** erkennt
  Mehrstockklemmen auch ohne Artikel (Etage ohne Artikelnummer → Nebenklemme). Verbindungen: **Quelle ID 31019**,
  **Ziel ID 310209** (Schreibweise laut Hilfe), mit BMK → Kabelverbindung, Standard-Funktionsdefinition „Ader / Draht“.
  Anschlussbezeichnungen (alle) ID 20038 mit Trenner `¶` (Alt+0182).
- Ersatzschlüssel für Artikelzuordnung: Bestellnummer ID 20919, Typenbezeichnung ID 20200; bei Verbindungen Farbe/Nummer
  ID 31004, Querschnitt ID 31002. Unbekannte Artikelnummern fett, „Artikel zuweisen“ sucht über die Ersatzschlüssel.
- Abgleich je Zeile: Neu → Importieren/Keine; Identisch → Keine; Zusätzlich (nur im Projekt) → Löschen/Keine;
  Geändert → Änderung übernehmen/Keine (geänderte Felder dunkelgrau, QuickInfo zeigt Projektwert); Ungültig → Keine.
  Klemmenanschlussseite über Bezeichnungen „Intern“/„Extern“.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_k_prinzip.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_h_betriebsmittelimportieren.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_h_verlegeverbindungenimportieren.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_d_abgleich.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_d_bmdatenimport.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_d_datenquelleauswaehlen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_d_feldzuordnung.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_d_verbindungsdatenimport.htm

### Bedeutung für QET
- Ein CSV-Importer für den Fork (BMK-/Klemmenliste → Elemente) sollte dieselben Schlüssel verwenden: vollständiges BMK
  als Identität, Klemmenbezeichnung zur Unterscheidung bei gleichem BMK, Etage für Mehrstock, Artikelnummer → Symbol.
  Der Vier-Status-Abgleich (Neu/Identisch/Zusätzlich/Geändert) ist ein gutes Muster für wiederholbare Importe.

## 14. Übergreifendes Bearbeiten auf Auswertungsseiten

- Auswertungsseiten sind über Platzhaltertexte mit den Quellobjekten verknüpft. „Übergreifend bearbeiten“ öffnet den
  Dialog des Quellobjekts (Seite → Seiteneigenschaften aus dem Inhaltsverzeichnis; Funktion → Eigenschaften aus BM-Liste,
  Klemmenplan, Geräteanschlussplan; Verbindung → aus Verbindungsliste/Kabelplan; Artikel → aus Stücklisten;
  Projektoption, Platzhalterobjekt, Strukturkennzeichen → jeweilige Übersicht).
- Änderung aktualisiert die Logikseiten und nur die betroffene Zeile/Spalte der aktuellen Auswertungsseite; andere
  Auswertungsseiten erst beim Aktualisieren. Ohne „übergreifend“ ändert ein Doppelklick nur den Platzhaltertext lokal –
  geht beim nächsten Aktualisieren verloren. Auch aus Suchergebnissen und per „Direktes Bearbeiten“ möglich.
- Nicht übergreifend bearbeitbar sind Auswertungen vom Typ Formulardokumentation, Normblattdokumentation,
  Revisionsübersicht, Symbolübersicht sowie in anderen Projekten erzeugte Auswertungen (Bezug verloren).
  Benutzereinstellung „Auswertungen beim Öffnen von Seiten aktualisieren“ (Benutzer > Darstellung > Allgemein).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Reverseengineering_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_uebergreifendesbearbeiten.htm

**Eigenschaften (übergreifend)-Modus** (Optionen > Eigenschaften (übergreifend)): Eine Funktion kann mehrfach
dargestellt sein (allpolig, einpolig, Übersicht, Paarquerverweis, RI-Fließbild …). Im Modus werden gemeinsame
Eigenschaften (z. B. Funktionstext) an allen Darstellungen gleichzeitig geändert; Registerkarte „<Kategorie> (BM)“
wirkt auf das ganze Betriebsmittel. Nicht übergreifend: Querverweisanzeige-Darstellung und Zeilen/Spalten,
Revisionsmarkierung, Blockeigenschaften, „Automatisch erzeugt“, „Makro“, sowie die Registerkarten Anzeige,
Symbol-/Funktionsdaten, Format. Sichtbares BMK einer Nebenfunktion ist dort nicht editierbar (kann je Darstellung
abweichen); ändert man das vollständige BMK der Nebenfunktion, gehört sie danach nicht mehr zum Betriebsmittel.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_k_modus.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_bmmodusaktivieren.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_alledarstellungenbm.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_alledarstellungenfunktion.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_ausgewaehltefunktion.htm

**Rangfolge der Darstellungsarten** (welcher Wert gilt beim übergreifenden Bearbeiten, **Auswerten und Abgleichen**, wenn
die Darstellungen verschiedene Werte haben): Funktionen: Allpolig > Übersicht > Paarquerverweis > Einpolig > RI-Fließbild
> Topologie > Schaltschrankaufbau > Schaltschrankaufbau detailliert > 3D-Montageaufbau > Funktional > Funktionsübersicht
(Fluid). Leeres Feld → nächste Darstellungsart. Beim Abgleich werden nur bereits gefüllte Felder überschrieben, leere bleiben
leer. Bool-Eigenschaften (z. B. Hauptfunktion) gelten als gesetzt, wenn an irgendeiner Darstellung gesetzt.
Verbindungen: Allpolig > 3D-Montageaufbau > Einpolig > RI-Fließbild > Topologie.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_k_reihenfolge.htm

### Bedeutung für QET
- **Wichtig für Verknüpfungsskript und Auswertungen:** Wenn ein Betriebsmittel im Nachbau mehrfach erscheint (Übersicht +
  allpoliger Schaltplan, Paarquerverweis), gilt der Wert der **allpoligen** Darstellung; nur wenn dort leer, der aus der
  Übersicht. Funktionstext, Artikel usw. beim Zusammenführen in dieser Reihenfolge auflösen; Hauptfunktion = an einer
  Darstellung gesetzt genügt.
- Bestätigt den Grundsatz des Nachbaus: Listen/Übersichten (Klemmenplan, BM-Liste, Inhaltsverzeichnis) sind **abgeleitet**;
  Korrekturen gehören in den Schaltplan bzw. die Elementeigenschaften, nicht in die Listenseite. In QET-Auswertungen
  (Nomenklatur, Klemmenleisten-Tabellen) nie Werte händisch nachtragen, die bei Neuerzeugung verloren gingen.

## 15. Bedeutung für QET (Zusammenfassung)

- Projekt = Datenbank mit eingelagerten Stammdaten (`.elk` + `.edb`); Zustand steckt in der Dateiendung
  (`.elp/.els/.ela/.elr/.ell/.elt`). Für Importe sind `.epj` (XML) und `.epdz` (SVG je Seite) die offenen Wege.
- Vorlage `.ept` = Einstellungen + Daten + Seiten, Basisprojekt `.zw9` = zusätzlich Stammdaten → in QET eine
  Basisvorlage mit eingebetteten Herstellersymbolen pflegen.
- Kennzeichen-Vererbung (Seite → Strukturkasten → Gerätekasten → Schaltzeichen, `>` stoppt) im Verknüpfungsskript nachbilden.
- Bei mehrfach dargestellten Funktionen gilt die Rangfolge Allpolig > Übersicht > Paarquerverweis > Einpolig …;
  leere Felder fallen auf die nächste Darstellung durch.
- Auswertungen sind abgeleitet; Korrekturen am Schaltplan, nicht an Listenseiten.
- Projektkorrektur und Komprimierung liefern einen fertigen Katalog für QET-Prüf-/Aufräumskripte (fehlende
  Leisten-/Steckerdefinitionen, Querschnitt-Leerzeichen, SPS-Anschlussbezeichnungen aus der Übersicht, unbenutzte
  Elemente, Netzdefinitionspunkte).
- Symbolaustausch nur bei gleicher Funktion und Ziel-Anschlusszahl ≥ Quelle; Anschlüsse über Namen zuordnen.
- Objekte außerhalb des Normblatt-Auswertungsbereichs gehören nicht zum Schaltplan; ausgeschaltete Projektoptionen
  fehlen im PDF (Lücken in Nummerierung sind dann gewollt).
- „Querverweis (extern)“ der Teilprojekte ist das Muster für Verweise in andere `.qet`-Dateien.

## 16. Ergänzung: Dialog Projekt überprüfen (nach unsauberem Schließen)
Wurde ein Projekt nicht ordnungsgemäß geschlossen (z. B. Stromausfall), bietet EPLAN beim Öffnen eine Datenprüfung an, weil die Datenbank unvollständige Einträge enthalten kann.
- *Projekt überprüfen*: ohne Befund wird geöffnet, sonst Meldung und Möglichkeit zur Korrektur.
- *Projekt überprüfen und korrigieren*: korrigiert wenn möglich und öffnet; misslingt das, an den Support wenden.
- *Projekt packen und sichern* (nur bei „korrigieren“): erstellt vorher eine gepackte Sicherung – Standard und empfohlen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_projektueberpruefen.htm

## Quellen

Projekt korrigieren:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ProjectCorrectionGui_d_korrektureinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ProjectCorrectionGui_d_projektkorrigieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ProjectCorrectionGui_h_projektkorrigieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ProjectCorrectionGui_k_korrekturdaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ProjectCorrectionGui_k_start.htm

Übergreifendes Bearbeiten:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Reverseengineering_k_start.htm

Sichern, Packen:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_d_projektesichern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_h_packen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_h_projektesichern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_h_projektmailversenden.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_arbeitsweise.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupdlggui_k_start.htm

Stammdaten sichern:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupmdatadlggui_d_stammdatensichern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakbackupmdatadlggui_h_stammdatensichern.htm

Wiederherstellen:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakrestoredlggui_d_inhaltsicherungsdatei.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakrestoredlggui_d_projektewiederherstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakrestoredlggui_h_projektewiederherstellen.htm

Stammdaten wiederherstellen:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakrestoremdatadlggui_d_stammdatenwiederherstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/bakrestoremdatadlggui_h_stammdatenwiederherstellen.htm

Normentausch:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_d_projektstammdaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_d_quellzielbibliothekwaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_d_symboletauschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_d_symbolschema.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_nummerieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_projektanpassen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_projektstrukturanpassen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_stammdatenaustauschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_stammdatenersetzen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_symboletauschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_h_vorlagefestlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_k_seitendrehung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_k_stammdaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_k_symbolaustausch.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_drehenspiegeln.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_nummerierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_projekt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_projektstammdaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_struktur.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/changestandardsgui_r_symbole.htm

Projektverwaltung, Projekteigenschaften, Struktur:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_benutzeranzeigen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_betriebsmittelstruktur.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_prjverwaltungsdb.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projektdatenabgleich.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projekteingangskontrolle.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projekterstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projektinfoausgeben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projektkopieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projektumbenennen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_projektverwaltung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_seitenstruktur.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_stammdatenabgleich.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_kundendateneinlesen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projektanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projektefiltern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projekteigenschaftenanzeigen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projekteigenschaftenloeschen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projekteverwalten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projektkopieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projektoeffnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_projektumbenennen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_h_verzeichniseinlesen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_darstellungerwprjstrukturen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_eigenschaften.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_ergebnisse.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_statisitk.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_status.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_struktur.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_vererbung.htm

Projektkontrolle:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_h_eigenschaftenvergleich.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_h_eingangskontrolvorber.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_h_pruefschema.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_h_vergleichsschema.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_k_projekte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/processreceiptcontrol_k_vorgehensweise.htm

Komprimieren:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectcleangui_d_komprimierungseinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectcleangui_d_projektkomprimieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectcleangui_h_projektsaeubern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectcleangui_k_komprimierungsdaten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectcleangui_k_start.htm

Projektoptionen:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_d_ausschnitt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_d_option.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_d_optionsgruppe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_d_optionsnavigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_d_platzhalternavigator.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_h_ausschnittebearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_h_ausschnitteerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_h_optioneneinaus.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_h_optionenerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_h_optionsgruppenerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_k_auswertungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectoptionsgui_k_start.htm

Import Betriebsmittel/Verlegeverbindungen:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_d_abgleich.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_d_bmdatenimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_d_datenquelleauswaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_d_feldzuordnung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_d_verbindungsdatenimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_h_betriebsmittelimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_h_verlegeverbindungenimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectprocessinggui_k_start.htm

Projekte, Vorlagen, Dateiformate:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_basisprojekterstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_exportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_importieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_normblattpfad.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_projekteabgleichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_projekteerstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_projektreorganisieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_projektvorlageerstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_veroeffentlichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_basisprojekt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_basisprojekteerweitert.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_bearbeitungsmodusbeimoeffnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_icons.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_projektvorlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_k_start.htm

Übergreifendes Bearbeiten:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_alledarstellungenbm.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_alledarstellungenfunktion.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_ausgewaehltefunktion.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_bmmodusaktivieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_h_uebergreifendesbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_k_modus.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/reverseengineering_k_reihenfolge.htm

Teilprojekte:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_d_teilprojekte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_k_auslagern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_k_einlagern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/subprojectservicesgui_k_start.htm

Zusatz-Dialogseiten (Ergänzung Abschnitt 16):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_projektueberpruefen.htm
