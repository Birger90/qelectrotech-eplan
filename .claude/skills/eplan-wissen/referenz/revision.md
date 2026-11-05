# Revisionsverwaltung in EPLAN (Plattform 2.9)

Wie EPLAN Änderungen an einem Projekt erfasst, markiert, abschließt und mit einem früheren Stand vergleicht – die
Grundlage für Revisionsindex im Schriftfeld, Revisionsübersicht, „Entwurf“-Wasserzeichen und Änderungswolken im PDF.
Zwei Arbeitsweisen: **Änderungsverfolgung** (laufendes Protokoll im Revisionsprojekt) und **Eigenschaftenvergleich**
(einmaliger Vergleich mit einem Referenzprojekt). Grundlage: offizielle EPLAN-Plattform-Hilfe 2.9.

## Inhalt
1. [Prinzip und Projektzustände](#1-prinzip-und-projektzustände)
2. [Änderungsverfolgung: Revision erzeugen, Entwurf, Seiten abschließen](#2-änderungsverfolgung)
3. [Revisionsindex und Revisionsdaten](#3-revisionsindex-und-revisionsdaten)
4. [Projekt / Bearbeitungsbereich abschließen, Schreibschutz](#4-projekt--bearbeitungsbereich-abschließen-schreibschutz)
5. [Revisionsmarkierungen und Löschzeichen](#5-revisionsmarkierungen-und-löschzeichen)
6. [Eigenschaftenvergleich von Projekten](#6-eigenschaftenvergleich-von-projekten)
7. [Alternative Identifizierung (Schlüsselkandidaten)](#7-alternative-identifizierung-schlüsselkandidaten)
8. [Revisionen löschen und zusammenfassen](#8-revisionen-löschen-und-zusammenfassen)
9. [Einstellungen im Überblick](#9-einstellungen-im-überblick)
10. [Bedeutung für QET](#10-bedeutung-für-qet)
11. [Quellen](#quellen)

---

## 1. Prinzip und Projektzustände

- Zweck: Änderungen gegenüber einem bestimmten Stand dokumentieren, alte Stände „einfrieren“, geänderte Seiten für
  Neudruck oder Genehmigungsstempel finden, Revisionsübersichten ausgeben (drucken oder als Auswertungsseiten).
- **Änderung einer Seite/eines Bauraums** = deren Ausdruck sieht optisch anders aus. **Änderung des Projekts** = auch
  unsichtbare Daten (BM-Eigenschaften, Artikelnummern) ändern sich, wodurch Auswertungsseiten anders aussehen.
- Projektzustände: Revisionsprojekt `.ell` (bearbeitbar, mit Änderungsverfolgung), abgeschlossenes Projekt bzw.
  Referenzprojekt `.elr` (schreibgeschützt, nicht änderbar/löschbar), temporäres Referenzprojekt `.elt`.
  Ersteller und Erstellungsdatum des Referenzprojekts werden als Projekteigenschaften gespeichert.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_start.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_prinzip.htm

## 2. Änderungsverfolgung

- **Revision erzeugen** (Dienstprogramme > Revisionsverwaltung > Änderungsverfolgung > Revision erzeugen): Name
  (Vorschlag aus Projektname + Zahl der Revisionen) und Kommentar; Benutzer, Bearbeitungsbereich, Datum automatisch.
  Voraussetzung: alle Seiten abgeschlossen. `.elk` wird `.ell`. Option „Bei neuer Revision vom abgeschlossenen Projekt
  Referenzprojekt erzeugen“ legt zusätzlich eine schreibgeschützte Kopie an (automatisch benannt).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_reverzeugen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_revisionerzeugen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_revauswaehlen.htm
- **Entwurf:** Jede geänderte Seite bekommt die Eigenschaft „Seite in Entwurfsmodus“ und ein **Wasserzeichen**
  (Text in Verwaltung > Revision (Änderungsverfolgung)). Position/Format über einen Sondertext
  „Wasserzeichen (Revisionsverwaltung)“ im Normblatt, sonst mittig. QuickInfo über geänderten Objekten zeigt Revisionsinfo.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_einstellungenaenderung.htm
- **Seiten abschließen** (… > Änderungsverfolgung > Seiten abschließen): je Seite Revisionsindex, Beschreibung,
  Änderungsgrund; Benutzer/Bereich/Datum automatisch. Entfernt die Entwurf-Kennzeichnung, speichert die Revisionsdaten
  an der Seite. „Seiten beim Abschließen auswerten“ aktualisiert markierte Auswertungsseiten (Inhaltsverzeichnis,
  Revisionsübersicht) und schließt sie mit demselben Index ab, auch wenn sie nicht im Entwurf waren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_seitenabschliessen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_beschreibungseitenaenderung.htm
- **Seitentypen gesondert:** „Immer abschließen“ (z. B. Titel-/Deckblatt, damit der neueste Index im Titelblatt-Schriftfeld
  steht – Seite muss mitmarkiert sein) und „Nie abschließen“ (z. B. Artikelstückliste, BM-Liste, damit aktualisierte
  Auswertungen die Revisionsübersicht nicht aufblähen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_einstellungenaenderung.htm
- Abfragen: „Beschreibung der Seitenänderung immer abfragen“ (bei Erzeugen, Löschen, Umbenennen inkl. Nummerieren,
  Import, Auswertung erzeugen) und „Markierungstext immer abfragen“ (bei jeder Änderung; sonst automatisch eingetragen).
  „Objekte in Optionen beim Ein- und Ausschalten berücksichtigen“: Umschalten von Projektoptionen wird protokolliert
  (für Masterprojekte); der Projektvergleich berücksichtigt nur eingeschaltete Optionen.
  „Projekt beim Abschließen als PDF ausgeben“: automatische PDF-Kopie beim Abschluss.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_einstellungenaenderung.htm
- **Revisionierung von Bearbeitungsbereichen:** Teilbereiche (nicht überlappend, seitenweise; Filter über
  Strukturkennzeichen oder Seiteneigenschaften, z. B. Gewerk Fluid) unabhängig revisionieren; abgeschlossener Bereich
  bekommt automatischen Änderungsschutz. Funktionen auf anderen Bereichen, die per Paarquerverweis mit geschützten Seiten
  verknüpft sind, bleiben bearbeitbar. Revision für einen Bereich nur, wenn das Projekt selbst nicht revisioniert ist
  (umgekehrt erlaubt).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_revteilprojekte.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_reverzeugen.htm

## 3. Revisionsindex und Revisionsdaten

- Revisionsindex: beliebige Zeichen; [...] wählt einen schon vergebenen. Vorschlag aus einem **Nummerierungsschema auf
  Firmenebene** (Firma > Verwaltung > Änderungsverfolgung (Nummerierung des Revisionsindex)) aus Formatelementen
  Trennzeichen (beliebig außer `#`, `<`, `>` – diese mit `\`) und Zähler (Mindeststellen, Füllzeichen, numerisch oder
  alphanumerisch). Alphanumerischer Zähler zählt wie Excel-Spalten über die vorgegebenen Zeichen
  (bei „AKS“: A, K, S, AA, AK, AS, KA …). Optional Warnung, wenn der Index nicht dem Schema entspricht; bei Übernahme
  wird die Zählfolge unterbrochen und beginnt neu.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_einstellungenseitenindex.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_formattrennzeichen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_formatzaehler.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_indexauswaehlen.htm
- Projekteigenschaft **Höchster Revisionsindex (aus Änderungsverfolgung) ID 10098**: höchster Index aller Seiten und
  Bauräume, als Sondertext platzierbar; Index muss dem eingestellten Format entsprechen und darf nicht nachträglich
  verändert werden.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_seitenabschliessen.htm
- **Revisionsdaten** = Änderungsgrund, Revisionsname, -index, -ersteller, -datum usw.; stehen in den Seiteneigenschaften
  und werden in Revisionsübersichten ausgegeben. „Revisionsdaten nachbearbeiten“ listet Revisionen und darunter die
  geänderten Seiten/Bauräume (Bauraumname als `=EB3+ET1 (2)` statt `/`), editierbar: Revisionsname, Kommentar, Index,
  Beschreibung, Änderungsgrund, „Geprüft von“, „Geprüft am“; fest: Benutzer, Datum, Kennung/Name/Telefon/E-Mail.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_revisionsdatennachbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_prinzip.htm
- Gelöschte Seiten (bei aktivem „Löschzeichen erzeugen und gelöschte Seiten auflisten“): Dialog Gelöschte Seiten mit
  Seitenname, Bereich, Index, Beschreibung (Vorschlag „Seite gelöscht“), Änderungsgrund, Projektrevision, Benutzer,
  Datum+Uhrzeit. In der Revisionsübersicht nur, wenn Formulareigenschaft **Revisionsausgabe mit gelöschten Seiten
  ID 13088** an und **Revisionsausgabeart ID 13106** = „Seiten“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_geloeschteseiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_markierungen.htm
- Beim Kopieren/Einfügen von Seiten, Ausschnitten, Makros mit Revisionsdaten entscheidet die Benutzereinstellung
  „Revisionsdaten und Revisionsmarkierungen beim Einfügen beibehalten“ (Benutzer > Darstellung > Allgemein).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_prinzip.htm

## 4. Projekt / Bearbeitungsbereich abschließen, Schreibschutz

- Dienstprogramme > Revisionsverwaltung > Projekt / Bereich abschließen: Standardprojekt → sofort schreibgeschützt als
  `.elr`. Revisionsprojekt → Dialog mit gemeinsamen Revisionsdaten für alle geänderten Seiten (Beschreibung vorbelegt
  mit Revisionskommentar). „Projekt beim Abschließen auswerten“: alle Auswertungsseiten aktualisieren, alle Entwürfe
  beenden, Auswertungen bekommen den zuletzt vergebenen Index. Bereich abschließen: nur dessen Seiten/Bauräume schreibgeschützt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_projektabschliessen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_projektabschliessen.htm
- Schreibschutz entfernen (Dienstprogramme > Revisionsverwaltung > Schreibschutz entfernen) für abgeschlossene/Referenz-/
  temporäre Projekte und Bereiche; erneutes Abschließen setzt ihn wieder. Automatischen Änderungsschutz einzelner
  Seiten temporär über „Schutz konfigurieren“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_schreibschutzentfernen.htm

## 5. Revisionsmarkierungen und Löschzeichen

- Markierungen entstehen automatisch: bei Änderungsverfolgung laufend, beim Eigenschaftenvergleich einmalig aus der
  **Revisionsergebnisliste** (in der Projektdatenbank; standardmäßig bei jedem Vergleich überschrieben; nach Löschen
  der Markierungen per „Revisionsmarkierungen erzeugen“ wiederherstellbar).
- **Grafisch:** Markierung um geänderte Objekte im Schaltplan und um geänderte Zeilen in Auswertungen; Aussehen je
  Änderungsart in Verwaltung > Revision (grafische Darstellung) – Registerkarten **Hinzugefügt, Geändert, Neu in
  Auswertung** (nur Änderungsverfolgung, nicht bei Stammdatenauswertungen), **Gelöscht**; Größe „an Objektgröße anpassen“
  (mit Mindestgröße) oder fest; „Gruppen in aufgelöster Darstellung markieren“ (sonst ganze Gruppe; ohne diese Option
  gilt eine neue Seite beim Vergleich als Gruppe; Mehrfachbemaßungen wie Gruppen); „Revisionsmarkierungstext nicht
  markieren“ (Text außerhalb der Wolke).
- **Textuell:** Markierungstext in Eigenschaft **Revisionsmarkierung (aus Änderungsverfolgung)** bzw. **(aus
  Eigenschaftenvergleich)**, bei der Änderungsverfolgung automatisch nach „Vorlage für automatische Markierung“
  (Format aus Eigenschaften wie Revisionsname, Bearbeiter, Änderungsdatum), manuell überschreibbar
  (Kontextmenü Revisionsmarkierung bearbeiten). Beim Vergleich legt man den Text im Vergleichsdialog fest.
- Ein-/Ausblenden über Ansicht > Revisionsmarkierungen (erste Revision schaltet ein; Abschalten bleibt bei weiteren
  Revisionen). Löschzeichen liegen auf eigener Ebene und bleiben bei „Unsichtbare Elemente“ sichtbar.
  Suchen nach Revisionsänderungen: Suche nach dem Markierungstext in „Alle Eigenschaften aller Schaltzeichen“, Logikseiten.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_markierungen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_einstellungengrafisch.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_revmarkierungen_anzeigen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_markierungstexte.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_revmarkierungenerzeugen.htm

**Löschzeichen** (Einstellung „Beim Löschen Löschzeichen erzeugen und gelöschte Seiten auflisten“ bzw. im
Vergleichsdialog „Für gelöschte Objekte Löschzeichen erzeugen“): ein Sonderobjekt an der Position des gelöschten Objekts,
in der Farbe für Gelöschtes, mit Eigenschaftendialog, der die Eigenschaften des Gelöschten speichert.
- Im Revisionsprojekt: verschieben → neue Markierung (Farbe „geändert“); „Endgültig löschen“ entfernt die Info.
  Normales Löschen von Markierung + Löschzeichen ersetzt das gelöschte Objekt durch identische neue Objekte (Vorsicht).
- Nach Eigenschaftenvergleich: normales Löschen entfernt endgültig; beim nächsten Vergleich mit unverändertem
  Referenzprojekt erscheint es wieder. Für gelöschte Objekte gilt: immer ein Löschzeichen je Objekt, auch in Gruppen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_loeschzeichenbearbeiten.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_loeschzeichenentfernen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_einstellungengrafisch.htm

## 6. Eigenschaftenvergleich von Projekten

- **Referenzprojekt erzeugen** (… > Eigenschaftenvergleich von Projekten > Referenzprojekt erzeugen; Berechtigung
  nötig): Name (Vorschlag aus Projektname + Anzahl), Beschreibung; `.elr` oder temporär `.elt` (löschbar ohne Recht zum
  Löschen von elr-Projekten). Anzeigen über Projekt > Öffnen, Dateityp „Abgeschlossene EPLAN-Projekte (*.elr)“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_vergleichsprojekterzeugen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_vergleichsprojekterzeugen.htm
- **Projekte vergleichen:** „Nur zugehörige Referenzprojekte anzeigen“ (schreibgeschützte des Projekts; ohne
  Schreibschutz gilt es nicht mehr als zugehörig), Vergleichsschema („-Nicht aktiviert-“ = Standard: alle Eigenschaften
  aller Objekte, langsam), Strukturkennzeichen-Schema (umbenannte Strukturkennzeichen Alt→Neu, damit sie nicht als
  gelöscht+neu gelten; nur sinnvoll mit alternativer Identifizierung), Revisionsmarkierungstext, Löschzeichen erzeugen,
  Markierung für Gelöschtes ins (nicht schreibgeschützte) Referenzprojekt zurückschreiben, alle bestehenden Markierungen
  löschen, nur Seiten des aktiven Seiten-Navigator-Filters vergleichen (ohne nicht platzierte Objekte), alternative
  Identifizierung, „Vergleichsergebnisse sammeln“ (anhängen statt überschreiben; bei jedem Öffnen wieder aus).
  Große Projekte: Vergleich ggf. über Nacht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_revvergleichen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_revvergleichen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtcomparegui_d_einstellungenstrukturkennzeichen.htm
- **Vergleichsschema** (auch auf Firmenebene: Firma > Verwaltung > Eigenschaftenvergleich von Projekten; wird auch von
  der Projektkontrolle benutzt): je Objekttyp (Kategorie) Eigenschaften von „Nicht vergleichen“ nach „Vergleichen“;
  Standard bei den meisten Typen: **Grafische Platzierung** (Position). „Gelöschte und neue Objekte berücksichtigen“
  je Typ (gelöschte stehen in der Ergebnisliste, aber ohne Markierung im Schaltplan). Typ komplett ausschließen = keine
  Eigenschaft gewählt und Kontrollkästchen aus. Verbindungen: normaler Vergleich nur nicht platzierte Verbindungen
  (platzierte über Verbindungsdefinitionspunkte); mit alternativer Identifizierung nur platzierte.
  Beispiel aus der Hilfe: Änderungen in Klemmenplänen finden → Kategorie Platzhaltertexte, Eigenschaft Textinhalt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_vergleichseinstellungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_vergleichseinstellungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_revvergleichen.htm
- **Ergebnisse** (… > Ergebnisse des Eigenschaftenvergleichs anzeigen; Anzahl begrenzt durch „Maximale Anzahl angezeigter
  Ergebnisse“, verglichen wird trotzdem alles): Objekttyp, BMK/Name (bei Verbindungen Quelle+Ziel), Seite, Änderungsart
  (geändert/gelöscht/neu), Eigenschaft, alter/neuer Wert, Artikelnummer [1], Potenzialname. „Auf alten Wert
  zurücksetzen“ nur für schreibbare Eigenschaften (nicht z. B. Funktionstext (automatisch)); Markierungen bleiben bis zum
  nächsten Vergleich. Sprung ins Projekt oder Referenzprojekt, Auswahl synchronisieren.
  „Revisionsmarkierung bei Verbindungen auf jeder Seite platzieren“: seitenübergreifende Verbindung auf jeder Seite statt
  nur der ersten markieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtcomparegui_d_ergebnisseprojektvergleich.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_einstellungen.htm

## 7. Alternative Identifizierung (Schlüsselkandidaten)

Statt interner Objekt-ID (bei automatisch erzeugten Projekten, z. B. aus EPLAN Engineering Configuration, jedes Mal neu)
werden Objekte über fest vorgegebene identifizierende Eigenschaften zugeordnet:
1. Seiten über vollständigen Seitennamen + Seitentyp.
2. Objekte zuerst auf der zugeordneten Seite, über denselben identifizierenden Namen (Verbindungen: Quelle und Ziel).
3. Erforderliche, dann optionale Eigenschaften je Objekttyp; zuletzt grafische Position. Kein Partner → „hinzugefügt“.

| Objekttyp | Erforderlich | Optional |
|---|---|---|
| Funktionen | Darstellungsart 20121, Funktionsdefinition 20188, Anschlussbezeichnungen (alle) 20038, zusätzlicher identifizierender Namensbestandteil 20032, dto. für Mehrfachklemmen 20233 (intern) | – |
| 3D-Artikelplatzierungen | 20121, 20188, 20038, Legendenposition 20450 | – |
| Abbruchstellen | 20121 | Seitenname 19022 |
| Verbindungen | 20121, Klemmen-/Steckerkontaktbezeichnung 20030, Steckerbezeichnung (automatisch) 20431, Anschlussbezeichnung der Funktion 20022 | Zielnummer Quelle 31023, Zielnummer Ziel 31024, Seitenname 19022 |
| Potenziale | Potenzialname 33000 | – |

Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_alternativeidentifizierung.htm

## 8. Revisionen löschen und zusammenfassen

- **Änderungsverfolgung > Revision löschen** (für markierte Seiten/Bauräume/Projekt): „Revisionsmarkierungen“ (Text +
  Grafik) und/oder „Revisionsdaten der Änderungsverfolgung“ (Abschlussdaten, Revisionen, am Symbol hinterlegte
  Revisionsänderungsformate; gelöschte Seiten zählen als Daten). Projektweit mit Revisionsdaten = Revisionierung
  komplett weg, keine Protokollierung mehr. Bei aktiven Bearbeitungsbereichen erst deaktivieren.
  Einzelobjekt: Kontextmenü „Revisionsmarkierung löschen“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_revmarkierungenentfernen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_revisionloeschenaenderung.htm
- **Eigenschaftenvergleich > Revision löschen:** Markierungen (inkl. Löschzeichen und gelöschte Seiten) und/oder
  Revisionsdaten des Vergleichs (u. a. Infos zu Referenzprojekten). Einzelobjekt: Werte der Eigenschaften
  Revisionsmarkierung / Revisionsänderungsmarkierung (aus Eigenschaftenvergleich) leeren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_revmarkierungenentfernen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_revisionloeschenvergleich.htm
- In „Revisionsdaten nachbearbeiten“: **Aktuellste Revision löschen** (nur Revisionsinfos aus den Seiteneigenschaften;
  Änderungen und Markierungen bleiben) und **Revisionen zusammenfassen** (neuer Name/Kommentar, Markierungen werden
  umbenannt, Datum = ältestes).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_revisionsdatennachbearbeiten.htm
- Komprimieren kann zusätzlich die Ergebnisse des Eigenschaftenvergleichs löschen (siehe `projektverwaltung.md`).

## 9. Einstellungen im Überblick

| Ebene / Pfad | Einstellung | Wirkung |
|---|---|---|
| Projekt > Verwaltung > Revision (Änderungsverfolgung) | Text des Wasserzeichens | „Entwurf“-Text auf geänderten Seiten |
| dto. | Vorlage für automatische Markierung | Format des automatischen Markierungstexts |
| dto. | Beschreibung der Seitenänderung / Markierungstext immer abfragen | Dialoge bei Änderungen |
| dto. | Projekt beim Abschließen als PDF ausgeben | automatische PDF-Kopie |
| dto. | Bei neuer Revision vom abgeschlossenen Projekt Referenzprojekt erzeugen | zusätzliches `.elr` |
| dto. | Beim Löschen Löschzeichen erzeugen und gelöschte Seiten auflisten | Löschzeichen + Liste gelöschter Seiten |
| dto. | Seitentypen immer / nie abschließen | Titelblatt immer, Listen nie |
| dto. | Objekte in Optionen beim Ein-/Ausschalten berücksichtigen | Optionsumschaltung protokollieren |
| Projekt > Verwaltung > Revision (Eigenschaftenvergleich) | Markierung bei Verbindungen auf jeder Seite; max. angezeigte Ergebnisse | s. o. |
| Projekt > Verwaltung > Revision (grafische Darstellung) | Aussehen je Änderungsart, Größe, Gruppen, Text außerhalb | Optik der Wolken |
| Firma > Verwaltung > Änderungsverfolgung (Nummerierung des Revisionsindex) | Indexschema | Vorschlag Revisionsindex |
| Firma > Verwaltung > Eigenschaftenvergleich von Projekten | Vergleichsschemata | auch Projektkontrolle |
| Benutzer > Darstellung > Allgemein | Revisionsdaten/-markierungen beim Einfügen beibehalten | Kopieren/Makros |

Quellen: wie in den Abschnitten 2–8.

## 10. Bedeutung für QET

- **Revisionsindex im Schriftfeld:** In EPLAN steht der Index an der **Seite** (Revisionsdaten beim Abschließen), der
  projektweit höchste in ID 10098. Beim Nachbau je Folio den Index aus dem PDF-Schriftfeld in eine Folio-Eigenschaft
  (z. B. `indexrev`) übernehmen, nicht projektweit setzen – verschiedene Seiten können verschiedene Indizes tragen.
  Das Titelblatt trägt meist den höchsten Index, weil es als „immer abschließen“ konfiguriert ist (Vermutung für den
  Einzelfall; prüfen).
- **„Entwurf“-Wasserzeichen** in einem EPLAN-PDF bedeutet: Seite geändert, aber nicht abgeschlossen. Im Nachbau als
  Hinweis notieren, nicht als Zeichnungsinhalt übernehmen.
- **Revisionswolken/-texte** sind Revisionsmarkierungen, keine Schaltplangrafik – im Nachbau nicht als Linien
  übernehmen; ggf. als Kommentar/Ebene.
- **Löschzeichen** in Revisions-PDFs markieren entfernte Objekte – sie gehören nicht in den Nachbau.
- **Revisionsübersicht** (Auswertungsseite) listet je Seite Index, Beschreibung, Änderungsgrund, Benutzer, Datum;
  gelöschte Seiten nur bei ID 13088/13106 – die Übersicht ist abgeleitet, Daten gehören an die Folios.
- **Vergleich zweier Nachbaustände:** Das Schlüsselkandidaten-Verfahren (Seite über Name+Typ, dann Objekt über BMK +
  Funktionsdefinition + Anschlussbezeichnungen, dann Position) ist ein robustes Vorbild für ein QET-Diff-Werkzeug,
  weil QET-Element-UUIDs beim Neuaufbau eines Plans (Skript `gesamt.py`) ebenfalls neu vergeben werden – also nicht
  über UUIDs vergleichen, sondern über BMK + Elementtyp + Anschlussnamen + Folio.
- Standardvergleich enthält die **grafische Platzierung**: reines Verschieben gilt in EPLAN als Änderung. Für einen
  „Inhalt vor Optik“-Vergleich die Position ausklammern.

## Quellen

Prinzip/Übersicht:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_prinzip.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_markierungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_revteilprojekte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_k_alternativeidentifizierung.htm

Handlungen:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_reverzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_seitenabschliessen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_projektabschliessen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_schreibschutzentfernen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_revmarkierungen_anzeigen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_revmarkierungenentfernen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_revmarkierungenerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_loeschzeichenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_loeschzeichenentfernen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_revvergleichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_vergleichseinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_h_vergleichsprojekterzeugen.htm

Dialoge:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_beschreibungseitenaenderung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_einstellungenaenderung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_einstellungenseitenindex.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_formattrennzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_formatzaehler.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_geloeschteseiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_indexauswaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_markierungstexte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_projektabschliessen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_revauswaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_revisionerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_revisionloeschenaenderung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionlogmgtgui_d_revisionsdatennachbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtcomparegui_d_einstellungenstrukturkennzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtcomparegui_d_ergebnisseprojektvergleich.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_einstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_einstellungengrafisch.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_revisionloeschenvergleich.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_revvergleichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_vergleichseinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/revisionmgtgui_d_vergleichsprojekterzeugen.htm
