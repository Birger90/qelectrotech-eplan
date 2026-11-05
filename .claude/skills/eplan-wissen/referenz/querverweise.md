# EPLAN: Querverweise, Kontaktspiegel, Abbruchstellen, Potentiale

Zusammenfassung der offiziellen EPLAN-Hilfe (Plattform 2.9, abgeglichen mit 2026) zu allem, was im
EPLAN-PDF als Verweis, Kontaktspiegel oder Sprungziel erscheint. Zweck: EPLAN-Pläne in QET inhaltlich
richtig nachbauen. In eigenen Worten; jede Aussage mit Quelle. „Vermutung“ = nicht aus der Hilfe belegt.

Begriffe: Die deutsche EPLAN-Hilfe sagt **Abbruchstelle** (engl. „interruption point“); „Unterbrechungspunkt“
und „Potentialpfeil“ sind Umgangssprache bzw. Übersetzung. Ebenso heißt es **Kontaktspiegel** (engl. „contact
image“, daher „Kontaktbild“). Menüpfad bis 2.9 `Optionen > Einstellungen`, in 2026 `Datei > Einstellungen`.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/interruptionpointgui_k_darstellungabbruchstellen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/xessettingsgui_d_einstellungenprojektqvwallgemein.htm

## Inhalt

1. Grundbegriffe: Funktion, Hauptfunktion, Nebenfunktion
2. Darstellungsarten
3. Querverweisarten im Überblick
4. Betriebsmittelquerverweis (Haupt ↔ Neben)
5. Paarquerverweis
6. Kontaktspiegel
7. Querverweisformat und Anzeige
8. Abbruchstellen: Stern und Kette
9. SPS-Querverweise und Übersicht
10. Strukturkästen als Querverweisziel
11. Potentiale, Signale, Potentialdefinitionspunkte
12. Verbindungsdefinitionspunkte
13. Seitenraster (Normblatt, Spalten, Zeilen, Pfade)
14. Neuerungen in neueren Versionen
15. Abbruchstellen im Detail: Sortierkennung, Abbuchen, Navigator, Smart-Connecting
16. Querverweise an Kästen, Geräteanschlüssen, Sammelschienen und funktionalen Platzierungen
17. Sprünge zum Gegenstück im Editor
18. Querverweis-Einstellungen: Anzeige, manuelle QVW, Montageplatten, Betriebsmittellisten
19. Quellen

---

## 1. Grundbegriffe: Funktion, Hauptfunktion, Nebenfunktion

- **Funktion** = Objekt mit Funktionsdefinition; platziert heißt sie **Schaltzeichen**. Ein Betriebsmittel hat
  eine oder mehrere Funktionen (Schütz = Spule + Kontakte).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_funktionen.htm
- **Hauptfunktion** vertritt ein verteilt dargestelltes Betriebsmittel (z. B. Spule), trägt allein die
  Gerätedefinition; genau eine je Betriebsmittel.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_hauptfunktionen.htm
- **Nebenfunktionen** = die übrigen Teile (z. B. Kontakte der Spule).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_nebenfunktionen.htm
- Kontrollkästchen **Hauptfunktion** (Registerkarte Symbol-/Funktionsdaten): an = Hauptfunktion mit Artikeldaten,
  aus = Nebenfunktion (Registerkarte Artikel verschwindet). Klemmen nutzen stattdessen „Hauptklemme“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_symbolfunktionsdaten.htm
- Rollentausch: „Hauptfunktion zuweisen“ macht eine Nebenfunktion zur Hauptfunktion; Artikeldaten wandern mit,
  die alte wird Nebenfunktion.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/adjustdata_h_nebenfunktionaendern.htm
- Zusammengehörigkeit erkennt EPLAN an **gleichem BMK + passender Funktionsdefinition**. Identifizierende
  Eigenschaften einer Funktion: BMK, Anschlussbezeichnung, Funktionsdefinition, Darstellungsart. Beschreibend:
  Funktionstext, technische Kenngrößen, Graviertext, Bemerkung, manueller Querverweis u. a.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_grundlagen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_k_bmeigenschaften.htm

**Bedeutung für QET:** EPLAN-Hauptfunktion = QET-**Master** (Typ coil/protection/commutator, plc), Nebenfunktion
= **Slave**. Artikeldaten (manufacturer, manufacturer_reference …) nur am Master pflegen, sonst doppelt in der
Stückliste. QET verknüpft nicht über das BMK, sondern über explizite Links; deshalb muss die Verknüpfung aus
dem **vollständigen** Kennzeichen (=Anlage+Ort-BMK) berechnet werden (siehe kennzeichnung.md) und das Kennzeichen
im elementInformation `label` stehen.

## 2. Darstellungsarten

Eigenschaft „Darstellungsart“ (Symbol-/Funktionsdaten) steuert, wie ein Schaltzeichen ausgewertet wird; eine
Funktion kann mehrere Darstellungsarten haben, jede nur einmal. Werte: **Allpolig**, **Übersicht**
(Übersichtsdarstellung des Geräts), **Paarquerverweis**, **Einpolig**, RI-Fließbild, Topologie, Funktional,
Funktionsübersicht (Fluid), **Extern** (nicht ausgewertet, aber als Ziel erfasst), **Grafik** (gar nicht
ausgewertet, nicht in Navigatoren). Nicht platzierte Funktionen kennen nur allpolig/einpolig.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_symbolfunktionsdaten.htm

- Darstellungsart folgt dem Seitentyp: eine Funktion wird auf einpoliger Seite einpolig, auf allpoliger Seite
  allpolig platziert; Tausch über gleiche Symbolnummer + Funktionsdefinition in beiden Bibliotheken.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/singlepole_k_hintergrund.htm
- Querverweise allpolig ↔ einpolig nur, wenn unter Querverweise/Kontaktspiegel > Anzeige freigegeben. Paarquerverweis
  zu einpolig ist unmöglich; maßgeblich ist die Darstellungsart der Hauptfunktion (muss allpolig sein), nicht der Seitentyp.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm
- Dialog „Anzeige“: je Seitentyp festlegen, zu welchen anderen Seitentypen Hauptfunktionen Querverweise zeigen
  (z. B. Schaltschrankaufbau, Betriebsmittelliste); Option „Von jeder Darstellungsart nur den ersten Querverweis
  anzeigen“; Gruppe Übersichtsquerverweise (Übersicht → Schaltplan, Schaltplan → Übersicht, an Versorgungsanschlüssen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektqvwanzeige.htm

**Bedeutung für QET:** QET kennt keine Darstellungsart, nur Simple/Master/Slave/Report. Abbildung:
allpolig = normale Elemente; **Übersicht** (SPS-Modulübersicht, Gerät mit allen Steckern) = Master, Kanal/Pinansicht
auf Funktionsblättern = Slave (praktische Entscheidung, in EPLAN ist die Übersicht keine Nebenfunktion, sondern
eine weitere Darstellungsart). **Grafik** = Simple-Element ohne Kennzeichen, nicht in Auswertungen; **Extern** →
Vermutung: als Simple mit Kennzeichen, aber nicht verknüpfen.

## 3. Querverweisarten im Überblick

EPLAN bildet: Betriebsmittelquerverweise (allgemeine Schaltzeichen), Kontaktquerverweise im Kontaktspiegel,
Abbruchstellenquerverweise, Paarquerverweise, dazu SPS-Querverweise und Querverweise auf Betriebsmittellisten.
Betriebsmittel-, Kontakt- und Abbruchstellen-QVW entstehen online automatisch; Paarquerverweise nur, wenn das
Schaltzeichen dafür eingestellt ist.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_grundlagen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm

Eigenschaft **Querverweisanzeige** (ID 20021): 0 = automatisch (nur bei nicht leerem sichtbarem BMK), 1 = nie,
2 = immer.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_grundlagen.htm

Präfixe: je Seitentyp kann ein Präfix die Querverweisart kennzeichnen (Tabelle „Seitentyp / Präfix“); eigenes
Präfix für externe QVW aus Teilprojekten.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektqvwallgemein.htm

**Bedeutung für QET:** Kennzeichen leer ⇒ in EPLAN standardmäßig kein Verweis. Ein Element ohne label im
Original also nicht als Master/Slave verknüpfen. QET-Präfixe (`power`/`delay`/`switch` in XRefProperties) wirken
nur auf Kontaktarten, nicht auf Seitentypen – EPLAN-Seitentyp-Präfixe (z. B. für Verweise auf Übersicht) im
QET-Format als festen Text in die Formel schreiben (Vermutung, je Plan prüfen).

## 4. Betriebsmittelquerverweis (Haupt ↔ Neben)

- Hauptfunktion zeigt auf alle Nebenfunktionen, jede Nebenfunktion zurück auf die Hauptfunktion.
- Reihenfolge: erst Nebenfunktionen aus der Gerätedefinition, dann außerhalb definierte; bei Anschlussbezeichnungen
  numerisch vor alphanumerisch (Stellenzahl berücksichtigt).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm
- Erzeugen = allen Teilen dasselbe sichtbare BMK geben („verteilte Darstellung“); funktioniert auch zwischen
  allpoliger und einpoliger Seite (z. B. 3-poliger -Q1 ↔ einpoliger -Q1).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_symbolquerverweiseerzeugen.htm
- Verweis auf Betriebsmittelliste: an Hauptfunktionen (optional auch Nebenfunktionen) möglich; Voraussetzung Anzeige
  allpolig ↔ Betriebsmittelliste freigegeben und Querverweisanzeige = „Immer anzeigen“; mit Präfix gekennzeichnet.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm

**Bedeutung für QET:** Ein Gerät, das auf mehreren Blättern in gleichwertigen Teilen erscheint (Servoregler
Leistung/Steuerstecker), hat in EPLAN genau eine Hauptfunktion. In QET: den Teil mit Artikeldaten (meist die erste
oder die Übersicht) als Master, alle anderen als Slaves. Mehrere Master mit gleichem Kennzeichen sind ein Fehler
wie in EPLAN („überzählige Hauptfunktionen“).

## 5. Paarquerverweis

- Zweck: Hilfskontakte von Motorschutz-/Leistungsschaltern (auch Lasttrennern) zweimal darstellen: einmal rechts
  neben der Hauptfunktion auf gleicher Höhe, meist unverbunden, Darstellungsart **Paarquerverweis** (BMK wird
  übernommen); einmal verdrahtet an beliebiger Stelle als **Nebenfunktion** mit dem BMK der Hauptfunktion.
- Bedingungen: beide auf allpoligen Seiten, gleiche Funktionsdefinition, gleiches sichtbares BMK, identische
  Anschlussbezeichnungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm
- Ergebnis: Paarquerverweis-Kontakt und verdrahteter Kontakt verweisen **aufeinander** statt auf die Hauptfunktion.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_paarquerverweiseerzeugen.htm
- Abbruchstellen-Einstellung „Zielanzeige an Paarquerverweisen“: zuerst Ziel, dann Verweis (Beispiel `L1/ X1:1 /7.04`).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektabbruchstellen.htm

**Bedeutung für QET:** QET hat keinen Paarquerverweis. Erkennen im PDF: Kontakt mit Verweistext direkt rechts neben
einem 3RV/LS, ohne Leitungen. Umsetzung (Vermutung, bewährt sich noch): den Kontakt neben dem Master als
**Slave** verknüpfen (zeigt dann auf den Master) und den verdrahteten Kontakt ebenfalls als Slave; der
Kontaktspiegel des Masters listet dann beide. Alternativ den Paarkontakt als reine Grafik ohne label und den
EPLAN-Verweistext als freien Text mit `destination` auf den verdrahteten Kontakt (Sprung wie im EPLAN-PDF).
Inhaltlich wichtig: nur **ein** Kontakt ist elektrisch real – der Paarkontakt darf nicht in Klemmen-/Kabelpläne.

## 6. Kontaktspiegel

- Zeigt alle platzierten Schaltzeichen eines Betriebsmittels, auch **nicht platzierte und freie** Funktionen der
  Gerätedefinition. Je Betriebsmittel wählbar: Kontaktspiegel oder reine Querverweisliste; Ausrichtung
  **„Im Schaltplanpfad“** oder **„Am Schaltzeichen“**; ohne Ausrichtung nur Liste. Bei Spulen und Motorschutzschaltern
  trägt EPLAN den Eintrag automatisch ein.
- Sortierung der Anschlüsse: numerisch, alphanumerisch, leer; Gerätedefinition hat Vorrang; leere/doppelte nach
  Lage im Schaltplan.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm
- „Automatisch ausrichten“: Spiegel senkrecht unter der Hauptfunktion; sonst X/Y-Versatz zum Einfügepunkt.
  Einstellung am Schaltzeichen hat Vorrang vor Projekteinstellung („Benutzerdefiniert“).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_einstellungenkontaktspiegel.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_anzeigekontaktspiegel.htm
- Darstellungsoptionen (Projekt bzw. je Gerät): eigenen QVW darstellen; **Als Symbolspiegel** (alle Schaltzeichen als
  Symbole, Hauptfunktion unterdrückt); senkrecht anordnen; QVW-Texte senkrecht; Öffner/Wechsler unterstreichen;
  Texte verkleinern; Symbole anzeigen; QVW links vom Symbol; linke/rechte Anschlussbezeichnung ausblenden;
  **Tabellenform** (Kopf aus „Kontaktspiegeltabelle: Beschriftung“, z. B. NO/NC bzw. S/Ö), Tabellenkreuz variabler
  Länge, Drehung, Anzeige 1./2. Artikelnummer und Artikeltyp; alternativ **eingebettetes Formular** (*.f45).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektmotorschutzschalter.htm
- Formularbasierte Kontaktspiegel: frei gestaltbare Tabellen, mehrere Varianten je Projekt, Einträge springen wie
  Hyperlinks zum Gegenstück.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_qvwjapanische.htm

**Bedeutung für QET:** XRefProperties je Mastertyp (`coil`, `protection`, `commutator`, `plc`, `cable`):
`displayhas` Cross ↔ EPLAN-Querverweisliste, Contacts ↔ Kontaktspiegel mit Symbolen; `snapto` Bottom/Label ≈
Lage unter dem Master (Vermutung: „bottom“ entspricht am ehesten „Im Schaltplanpfad“ am Pfadfuß, „label“ dem
Spiegel „Am Schaltzeichen“); `showpowerctc` = Hauptkontakte zeigen; `showallconfiguredslaves` ≈ EPLAN zeigt auch
**nicht platzierte** Kontakte der Gerätedefinition (Fork: Kontaktgruppen über `group_index`). Unterstreichen
von Öffnern, Tabellenkopf NO/NC und Artikelanzeige im Spiegel hat QET nicht – nicht nachbauen, Inhalt zählt.
Im EPLAN-PDF ist jeder Spiegeleintrag ein Link → QET-Spiegel ist per Doppelklick navigierbar; im PDF-Export
Sprungziele prüfen.

## 7. Querverweisformat und Anzeige

Aufbau eines Querverweises: (vollständiges BMK bzw. Strukturkennzeichen) + Trennzeichen + Seite + Trennzeichen +
Spalte und/oder Zeile im Normblatt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_qvwanzeigeoptimieren.htm

Projekteinstellungen (Querverweise/Kontaktspiegel > Allgemein):
- Anordnung zeilen- oder spaltenweise, **Anzahl pro Zeile/Spalte** (>1 ⇒ Matrix; bei 3: zeilenweise `0 1 2 / 3 4 5 / 6`).
- Trennzeichen **vor der Seite** (z. B. `/`), **zwischen Seite und Spalte** (z. B. `.` ⇒ `/2.6`), **zwischen Zeile und
  Spalte** (nur bei Normblatt mit Zeilen).
- **Im QVW Strukturkennzeichen vollständig anzeigen**: `=AA1+BC1/2.5` statt `/2.5`.
- **QVW-Kürzungsregeln auf BMK beziehen** (nur ohne vorige Option).
- **Ohne Seitennummer**; **Gleiche Seite unterdrücken** (`/2.6` → `.6`, nur wenn alle identifizierenden Eigenschaften
  gleich); „Trennzeichen vor Seite immer anzeigen“.
- Klammern um QVW (GOST); Trennzeichen der Dokumentenart unterdrücken; **Zeilen- und Spaltenangabe tauschen**
  (IEC `20.1.A` → `20.A.1`).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektqvwallgemein.htm

Automatische Kürzung: Bezugsebene ist nur die **aktuelle Seite** (Strukturkästen zählen nicht). Weicht ein
identifizierendes Kennzeichen (z. B. Anlage) ab, erscheinen die abweichenden Blöcke; gleiche Blöcke können entfallen.
Zeigt ein Verweis nur Spalte/Zeile, liegt das Ziel auf derselben Seite.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_qvwanzeigeoptimieren.htm

Normblatt-Bezug: Schaltzeichen außerhalb des Normblatts erzeugen Verweise ohne Spalte; ein Schaltzeichen innerhalb
erzeugt am äußeren trotzdem einen vollständigen Verweis.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_grundlagen.htm

**Bedeutung für QET:** Das Verweisformat ist Projekteinstellung – im Original ablesen, nicht raten. Typische Formen:
`/2.5` (Seite.Spalte), `/38/1` (beide Trennzeichen „/“), `=DA/28/8` (abweichende Anlage vorangestellt). QET-Formel
(XRef master/slave label, Report-Label): `%F` Blattname, `%f` Blattindex, `%c` Spalte, `%l` Zeile, `%M` Anlage,
`%LM` Ort (Quellcode `sources/autoNum/assignvariables.cpp`). EPLAN `=AA1+BC1/2.5` ≈ QET `=%M+%LM/%F.%c`; nur Seite.Spalte
≈ `/%F.%c`. QET-Standard ist `%f-%l%c` (Index, nicht Blattname!) → immer umstellen. „Gleiche Seite unterdrücken“
und die automatische Kürzung kann QET-Formel nicht bedingt (Vermutung) – Kompromiss je Plan: Anlage immer mitschreiben,
wenn das Original sie bei fremden Anlagen zeigt. Spalten: EPLAN zählt ab 0, QET ab 1, solange `border-columns_0` aus ist.

## 8. Abbruchstellen: Stern und Kette

- Abbruchstelle führt Verbindung, Netz, Potential oder Signal weiter; Name = z. B. Signalname. Keine Quell/Ziel-
  Unterscheidung (Richtung ermittelt EPLAN). Abbruchstellen sind Verbindungssymbole (SPECIAL.slk) **ohne
  Funktionsdefinition**; offene Verbindung an einer Abbruchstelle ist kein Fehler.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_k_start.htm
- Zuordnung über das **sichtbare BMK** (Name) der Abbruchstelle, wählbar aus vorhandenen Namen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_h_abbruchstelleneinfuegen.htm
- **Sternquerverweis**: Eine Abbruchstelle hat „Sternquelle“ gesetzt; alle gleichnamigen verweisen auf sie, sie zeigt eine
  formatierbare Liste aller anderen. **Kettenquerverweis** (Sternquelle aus): 1. ↔ 2., 3. ↔ 4. usw., also Seite zu Seite.
  Am ersten Pfeil einer Kette optional Verweise auf alle anderen (einer pro Seite).
- Reihenfolge der Kette: Dialog „Abbruchstellen sortieren“ oder **Sortierkennung**; beim Stern bestimmt die
  Sortierkennung die Zielnummer. Fehlt ein Gegenstück, meldet EPLAN einen Fehler.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_k_darstellungabbruchstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_abbruchstellenquerverweiseerzeugen.htm
- Ein Abzweig (T-Knoten) vor einer Sternquelle ist ein Prüffehler: Sternquelle darf nur ein Ziel haben.
  Quelle: https://www.eplan.help/en-us/infoportal/content/Plattform/2.9/Content/htm/messages_p_011007.htm
- Einstellungen (Querverweise/Kontaktspiegel > Abbruchstellen): Matrix zeilen-/spaltenweise; Querverweis anzeigen;
  **nur Seitennamen** (`7` statt `7.8`); **Strukturkennzeichen vollständig** (`=AA1+BC1/2.5`); Zielanzeige an Paar- und
  Sternquerverweisen (Zielanschluss, z. B. `X1:1`), Ziel unter dem Verweis; **Trennzeichen zwischen Name und QVW**.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektabbruchstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/xessettingsgui_d_einstellungenprojektabbruchstellen.htm
- Mit erweiterter Referenzkennzeichnung wird das Produktkennzeichen an Abbruchstellen geschachtelt; optional erben
  Abbruchstellen Strukturen von Geräte-/SPS-Kästen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_darstellungerwprjstrukturen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_vererbung.htm

**Bedeutung für QET:** QET-Folio-Reports verknüpfen genau **1:1** (next_report gehend ↔ previous_report kommend).
- **Kette** passt direkt: in EPLAN-Reihenfolge (Seitenfolge bzw. Sortierkennung) paarweise 1↔2, 3↔4 verknüpfen.
  Ein Potential, das auf Blatt A abgeht und auf B, C, D ankommt, ist in EPLAN eine Kette A→B, B→C … – auf B braucht es
  daher einen kommenden **und** einen gehenden Pfeil. Im Original lesen, welcher Pfeil worauf zeigt.
- **Stern** (Pfeil mit Verweisliste, z. B. „L1 /9.0 /12.0 /15.0“): QET kann das nicht. Sternquelle als gehenden Pfeil mit
  dem ersten Ziel verknüpfen, die übrigen Ziele als freien Text mit `destination` je Ziel daneben (Sprünge wie EPLAN);
  die Zielpfeile als kommende Pfeile, je einer mit eigenem Partner (Vermutung, Muster noch festlegen).
- Abbruchstellen sind keine Betriebsmittel: kein Artikel, nicht in Stückliste. Name des Pfeils = label des Reports;
  Potential/Signal fließt über die Verknüpfung durch (QET-Potential über Reports).
- Fehlendes Gegenstück = Fehler wie in EPLAN: nach `verknuepfen.py` alle unverknüpften Pfeile melden.

## 9. SPS-Querverweise und Übersicht

- Byteorientierte SPS-Anschlüsse im Schaltplan verweisen auf die Übersichtsseite und zurück; Bedingung: gleiches BMK
  (SPS-Karte/-Kasten, an Anschlüsse vererbt), gleiche Funktionsdefinition, gleiche Steckerbezeichnung (Bus: zusätzlich
  Schnittstellenname) und gleiche **Anschlussbezeichnung**. Die **SPS-Adresse identifiziert nicht** und ist für
  Querverweise ohne Belang.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_spsquerverweiseerzeugen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_querverweisarten.htm

**Bedeutung für QET:** Kanal ↔ Übersicht über **Modul-Kennzeichen + Anschlussnummer** verknüpfen, nicht primär
über die Adresse. Fehlt im PDF das Kennzeichen am Kanal, ist die Adresse nur Hilfsschlüssel (so macht es
`verknuepfen.py` heute) – das ist eine Notlösung, kein EPLAN-Verhalten.

## 10. Strukturkästen als Querverweisziel

Strukturkästen sind keine Betriebsmittel, können aber querverweisfähig werden: gleiches BMK, genau einer mit
„Hauptplatzierung“, an allen die Anzeige „Querverweis (Haupt-/Nebenfunktion)“ aktiv.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_k_grundlagen.htm

**Bedeutung für QET:** Ortsrahmen sind in QET nur Formen (`add_shape` dashdot) ohne Verweis. Zeigt das Original
einen Verweis am Rahmen, als freien Text mit `destination` nachbilden.

## 11. Potentiale, Signale, Potentialdefinitionspunkte

- **Potential** = elektrische Ebene (z. B. L1), **Signal** = benannter Leiter auf einem Potential (eigener Name, eigene
  Daten).
- **Potentialdefinitionspunkt** und **Potentialanschluss** sind gleichwertig, wirken auf das **ganze Netz**, nicht auf
  eine Verbindung; übergeben Potentialname, Signalname, Potentialtyp, Wert, Frequenz, Gegenpotentiale. Am Punkt wird
  standardmäßig der Potentialname angezeigt. Sie gehören nicht zur Projektstruktur.
- Potentialtypen: L, N, PE/PEN, +, M, -, SH, Undefiniert. Typ vom Potential hat Vorrang vor dem der Anschlüsse;
  Widerspruch = Fehler. Jeder Typ liegt auf eigener Ebene (Farbe einstellbar).
- Mehrere Definitionspunkte gleichen Namens erlaubt; Widersprüche = Fehler; bei Konflikt gewinnt der **grafisch erste**
  (Normblatt-Reihenfolge, DIN: links oben nach rechts unten).
- Weiterleitung durch Bauteile über die **Anschlusslogik** („Potenzial weiterleiten nach“); Funktionen sind
  meist netztrennend, „Signaltrennend“ stoppt Signale.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_k_prinzippotenziale.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_h_pdpzeichnen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_symbolfunktionsdaten.htm

**Bedeutung für QET:** Potentialnamen (5L4+, 9M5) im Original stehen an Abbruchstellen oder Potentialdefinitionspunkten.
In QET: Leiterfunktion/-nummer des Potentials (`set_conductor`) – gilt fürs ganze QET-Potential, auch über Klemmen,
also wie EPLAN „ganzes Netz“. Daher Aderfarbe/Querschnitt nicht in die Leiternummer schreiben (siehe Skill
qet-schaltplan-zeichnen). Potentialtyp (L/N/PE/+/M) hat QET nicht; Prüfung „Verbraucher an passendem Gegenpotential“
selbst machen (5L4+ ↔ 5M…).

## 12. Verbindungsdefinitionspunkte

- Weist den durchlaufenden Verbindungen Eigenschaften zu (Artikel, Länge, Bezeichnung, Querschnitt, Farbe, grafische
  Darstellung); hat **Vorrang** vor Potential- und Projekteinstellungen. Nur hier bekommt eine Verbindung BMK,
  Funktionsdefinition, „Quelle/Ziel tauschen“.
- Mehrere Punkte je Verbindung möglich; Bezeichnung aus nur „?“ gilt als undefiniert, dann zählt der grafisch erste
  Punkt mit anderer Bezeichnung. Auswertungen über Verbindungen, nicht über die Punkte.
- Automatisch gesetzt durch Verbindungsnummerierung (seitenübergreifend oder >3 Teilstücke: Anfang und Ende) und
  beim automatischen Kabelerzeugen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_vdp.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/Glossary_o_verbindungsdefinitionspunkte.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_k_prinzippotenziale.htm
- Verbindungen werden offline erzeugt (nicht online aktualisiert); Autoconnecting verbindet gegenüberliegende Anschlüsse.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_start.htm

**Bedeutung für QET:** Der schräge Strich „DBU 0,5“ im EPLAN-PDF ist ein Verbindungsdefinitionspunkt. Im Fork gibt es
`ConductorDefinitionMark` (schräger Strich, Farbe oben, Querschnitt unten, verschiebbar, `m_definition_mark` in den
Leitereigenschaften; `sources/qetgraphicsitem/conductordefinitionmark.h`) – passt fachlich, weil es am **einzelnen**
Leiter hängt, nicht am Potential. Vorzuziehen vor dem bisherigen Elementtext-Behelf (Vermutung: Prüfen, ob Auswertungen
die Farbe aus dem Leiter lesen).

## 13. Seitenraster (Normblatt, Spalten, Zeilen, Pfade)

- Spalten von links nach rechts, Zeilen von oben nach unten, **Start standardmäßig 0**; Anfangswert, Format
  (numerisch/alphanumerisch) am Normblatt; ohne Format wird die Nummer ausgeblendet. Alphanumerisch: Zahl als Startwert,
  0 und 1 → A, 2 → B …
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/formeditorgui_h_normblaetterpfadeaktualisieren.htm
- Pfadnummerierung **seitenweise** (je Seite ab Startwert), **projektweise** (fortlaufend, nur Logikseiten) oder **je
  Strukturkennzeichen**; Optionen Lücken für fehlende Seiten, Unternummern bei Unterseiten (Seite 1.A → Pfade 0.A–9.A).
  Änderung wirkt erst nach „Pfadbezeichnungen aktualisieren“. Normblatt beeinflusst die Pfadnummerierung nicht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_d_projekteinstellungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projects_h_normblattpfad.htm
- Standardraster Schaltplan Elektro 4 mm (Fluid 2 mm).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_d_projekteinstellungen.htm
- Konfliktreihenfolge „grafisch erster“ folgt dem Normblatt (DIN: links oben → rechts unten).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_k_prinzippotenziale.htm

**Bedeutung für QET:** Spaltenzahl und -breite aus dem Original-Normblatt (meist 10 Spalten 0–9, ohne Zeilen) übernehmen;
`border-columns_0` einschalten, wenn die Verweise exakt wie im Original (ab 0) lauten sollen, sonst liegen alle um eins
daneben. Bei projektweiser Pfadnummerierung (Pfade 101, 102 …) passt QETs Spaltenzählung nicht – dann Spaltentexte
selbst setzen (Vermutung, bisher nicht gesehen). Verweisziele immer auf das Raster des Zielblatts beziehen.

## 14. Neuerungen in neueren Versionen

- 2026: Inhalt der Querverweis- und Abbruchstellenseiten unverändert gegenüber 2.9; Menü heißt `Datei > Einstellungen`;
  „Eplan“ statt „EPLAN“. Keine neuen Querverweisarten gefunden.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/xessettingsgui_k_querverweisarten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/xessettingsgui_d_einstellungenprojektabbruchstellen.htm
- Seit 2.9: Querverweise zwischen grafischer Vorplanung/RI-Fließbild und Vorplanungsauswertungen (für Stromlaufpläne
  unerheblich).
  Quelle: https://eplan.help/en-us/Infoportal/Content/Plattform/2.9/Content/htm/news_p_preplanning_querverweise_auswertungen_241563.htm

## 15. Abbruchstellen im Detail: Sortierkennung, Abbuchen, Navigator, Smart-Connecting

- Name der Abbruchstelle = Zusammengehörigkeit: Potentialname (`L1`, `L+`, `PE`, `PEN`, `N`), Wirkungsangabe (`Not`,
  `+15V`, Signalname) oder beliebiger Text. Jedes Projekt führt eine **Liste der Abbruchstellennamen**; neue
  Abbruchstellen werden einem vorhandenen Namen zugeordnet (Dialog „Abbruchstelle abbuchen“, Baum
  Kennzeichnungsblock > Name > Quelle/Ziel). Beim Abbuchen eines Namens wird nur der Name übernommen; beim Abbuchen einer
  einzelnen Funktion alle schreibbaren Eigenschaften **außer** Darstellung, **Sortierkennung** und **Sternquelle**.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_d_abbruchstellenabbuchen.htm
- **Sortierkennung**: Standard **100** = Sortierung nur nach Seite, Zeile, Spalte (grafische Reihenfolge); kleinere
  Werte ziehen eine Abbruchstelle nach vorn; **0 macht sie zur Quelle**. Gleiche Kennung → Reihenfolge im Schaltplan.
  Der Dialog „Abbruchstellen sortieren“ zeigt Position, Typ (Quelle/Ziel), Verbindung und Kennung; Pfeiltasten ändern die
  **Paarbildung der Kette**; „Grafisch sortieren“ setzt alle auf 100.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_d_abbruchstellensortieren.htm
- **Korrektur/Präzisierung:** §8 sagt „keine Quell/Ziel-Unterscheidung“. Das gilt fürs Zeichnen (eine Abbruchstelle
  kann nach beiden Seiten angebunden werden, Richtung ermittelt EPLAN). Für die **Paarbildung** führt EPLAN dennoch einen
  Typ Quelle/Ziel, den man über Sortierkennung 0 bzw. Reihenfolge beeinflusst.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_d_abbruchstellensortieren.htm
- Ungewollte Anbindung an der zweiten Seite einer Abbruchstelle → **Unterbrechung** einfügen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_k_start.htm
- **Abbruchstellen-Navigator** (`Projektdaten > Verbindungen`): alle Abbruchstellen auf Schaltplan- und Übersichtsseiten;
  Platzieren (neue Abbruchstelle mit gewähltem Namen; Rücktaste = anderes Symbol), **Zuweisen** (Name/Daten auf ein
  vorhandenes Schaltzeichen übertragen), Löschen (auch die Platzierung), Sortieren, Gehe zu, Tabellarisch bearbeiten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_d_projektdatenabbruchstellen.htm
- Eingabe: sichtbares BMK direkt oder über [...] aus der Namensliste; vollständiges BMK (z. B. anderer Einbauort) einstellbar.
  **Untergeordnetes BMK** an Abbruchstellen nur mit **erweiterter Referenzkennzeichnung** (Projekt > Struktur).
  Standardposition der „Beschreibung der Abbruchstelle“ je Variante als Eigenschaftsanordnung speicherbar (Standard für
  neue Abbruchstellen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_h_abbruchstellenbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_h_abbruchstelleneinfuegen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_h_standardposabbruchstellenbeschrfestlegen.htm
- Grafikregeln: Abbruchstelle darf einen Winkel/ein T-Stück **überzeichnen**, nicht umgekehrt; Abbruchstellen werden nicht
  in Gerätekästen geschachtelt; keine direkte Textbearbeitung (F2); Format übertragen nur Registerkarte Anzeige.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_verbindungsstueckeaustauschen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_k_schachteln.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/inplaceeditinggui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_formatuebertragen.htm
- **Smart-Connecting**: Ausschneiden/Einfügen eines Schaltplanteils auf eine andere Seite erzeugt auf beiden Seiten
  Abbruchstellen mit Querverweis, Namen aus Zähler (Mindeststellen, Füllzeichen, numerisch oder alphanumerisch A, K, S,
  AA …; mit Vorzeichen, wenn „Vorzeichen setzen für Abbruchstellen“ an). Gleiche Einstellungen gelten für „Neues BMK
  ermitteln“ am Feld sichtbares BMK.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_smartconnecting.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_d_einstellsmartconnecting.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_d_einstellsmartabbruchstellen.htm
- **Stränge** lassen sich über Abbruchstellen seitenübergreifend fortführen (dick erst nach Verbindungen aktualisieren);
  netzbasierte Verbindungen ebenso. Beim Makro-Aktualisieren bleiben BMK der Abbruchstellen erhalten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_straenge_in_einpoligerdarstellung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenmakrosaktualisieren.htm

**Bedeutung für QET:** `verknuepfen.py` sollte die Kettenpaarung nach EPLAN-Regel bilden: je Name alle Pfeile nach
(Sortierkennung, Seite, Zeile, Spalte) sortieren und paarweise 1↔2, 3↔4 verknüpfen. Die Sortierkennung steht nicht im
PDF – maßgeblich ist daher, worauf der Verweistext im Original zeigt; nur wenn er fehlt, grafische Reihenfolge. Pfeil mit
Verweis auf **sich selbst gleiche Seite** ist kein Fehler (Smart-Connecting-Paare auf derselben Seite, Vermutung).
Abbruchstellen tragen kein Geräte-Kennzeichen im Sinne des Kastens: auch innerhalb eines Ortsrahmens/Gerätekastens
keinen Kasten-Präfix ableiten, nur die Strukturkennzeichen (Anlage/Ort), wenn das Original sie zeigt.

## 16. Querverweise an Kästen, Geräteanschlüssen, Sammelschienen und funktionalen Platzierungen

- **Geräteanschlüsse im Gerätekasten ohne eigenes BMK** sind immer Nebenfunktionen und zeigen **nie** Querverweise (der
  Kasten verweist). Außerhalb eines Kastens werden Geräteanschlüsse querverwiesen; Hauptfunktion kann Geräteanschluss
  oder Kasten sein. Ein Gerätekasten kann für Sonderschütze auch einen **Kontaktspiegel** tragen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_h_anschluesseeinfuegen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_k_start.htm
- Lampe `-H2` im Kasten `-U1` wird mit `-U1-H2` außerhalb querverwiesen (bei eingeschaltetem Schachteln); bei Kasten-in-
  Kasten mit unklarer Lage gibt es einen Hinweis in der Meldungsverwaltung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_k_arbeitsweise.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_k_schachteln.htm
- **Strukturkästen**: „Gehe zu (Gegenstück)“ springt zur Hauptplatzierung; „Gehe zu (alle Darstellungsarten)“ und „Gehe
  zu (querverwiesene)“ listen alle Strukturkästen gleichen vollständigen Namens.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_k_grundlagen.htm
- **Sammelschienen** und **Leitungsverteiler/-verbinder** sind Funktionen mit Haupt- und Nebenfunktionen gleichen BMK
  (Artikel an der Hauptfunktion; mehrere Verteiler eines BMK brauchen verschiedene Anschlussbezeichnungen) → sie werden
  wie Betriebsmittel querverwiesen (Vermutung: Querverweis Haupt ↔ Neben wie §4).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_k_sammelschienen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_k_verteiler.htm
- **BMK zusammenführen**: mehrere Schaltzeichen mit gemeinsamem vollständigem BMK → automatisch Betriebsmittelquerverweise.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_bmkzusammenfuehren.htm
- **Darstellungsart „Funktional“** (Ablaufdiagramme): funktionales Platzieren erzeugt eine **Kopie als Nebenfunktion**,
  mehrfach platzierbar, nur Grafik, nicht logisch ausgewertet. Die Rangfolge der Darstellungsarten bei Gruppen lautet
  Allpolig > Übersicht > Paarquerverweis > Einpolig > RI-Fließbild > Extern.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functionaldiagramgui_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_gruppierung.htm
- Kontaktspiegel am Symbol: Ausrichtung **Im Schaltplanpfad** = je nach Normblatt **unten oder rechts** im Pfad; **Am
  Schaltzeichen** = daneben (Symboleditor, Registerkarte Kontaktspiegel). Kontaktspiegel lässt sich wie Eigenschaftstexte
  manuell verschieben (Strg+B).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/symboleditorgui_d_eigenschaftsplatzierung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_h_symboltexteverschieben.htm
- Einpolig ↔ allpolig: dieselbe Funktion wird je Seitentyp ein-/allpolig platziert; abweichende Daten verteilter
  Darstellungen lassen sich projektweit abgleichen; Funktionstexte aller Funktionen eines BMK über „Funktionstexte
  abgleichen“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_hintergrund.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/egedgui_h_funktionstexteabgleichen.htm

**Bedeutung für QET:** Pins eines Gerätekastens (FU, Netzteil) im Original ohne Verweistext → in QET **keine** Slaves,
sondern Anschlüsse des Kasten-Elements; nur der Kasten (Master) und gleichnamige Kästen/Teile auf anderen Blättern
(Slaves) werden verknüpft. Einträge in Ablaufdiagrammen (Funktional) nicht als Master/Slave in den Kontaktspiegel
aufnehmen (Vermutung: EPLAN zeigt sie in der Verweisliste nicht, weil nicht ausgewertet). Kontaktspiegel „Im
Schaltplanpfad“ bei Normblatt mit Zeilen rechts statt unten – XRef-Lage in QET danach wählen.

## 17. Sprünge zum Gegenstück im Editor

- `[Strg]`+Klick oder `[Strg]`+`[Leertaste]` auf einen Querverweis springt zum Gegenstück, mit `[Umschalt]` zusätzlich in
  neuem Editorfenster; `F` = Gegenstück anspringen (auch bei Abbruchstellen); `Alt+Bild↓/↑` = nächste/vorige
  querverwiesene Funktion.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededitgui_k_tastaturbefehle.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_smartconnecting.htm

**Bedeutung für QET:** Das Editor-Verhalten entspricht dem PDF-Dreifachsprung (pdf-export.md): im QET-Fork
Doppelklick auf XRef/Report = Gegenstück; „nächste querverwiesene Funktion“ ≈ Sprungfolge links/rechts.

## 18. Querverweis-Einstellungen: Anzeige, manuelle QVW, Montageplatten, Betriebsmittellisten

### 18.1 Grundlage und Einstellungsorte
- Grundlage jedes Querverweises ist die Einteilung des Normblatts in Zeilen (horizontale) und Spalten (vertikale Abschnitte); beide Achsen können getrennt alphabetisch oder numerisch nummeriert sein. Ein Querverweis enthält mindestens den Seitennamen, optional Spalte und zusätzlich Zeile. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_start.htm
- Alle Anzeigeeinstellungen liegen in den **Projekteinstellungen** unter Querverweise / Kontaktspiegel, verteilt auf drei Bereiche: **Abbruchstellen** (Gruppenfeld Darstellung = wie QVW an Abbruchstellen erscheinen), **Allgemein** (Darstellung am Bauteil; Trennzeichen vor der Seite, zwischen Seite und Spalte, zwischen Zeile und Spalte) und **Anzeige** (je Seitentyp, zu welchen anderen Seitentypen Querverweise erscheinen und in welcher Reihenfolge; Gruppenfeld „Übersichtsquerverweise anzeigen“). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_einstellungenqvwanzeige.htm

### 18.2 Manuelle Querverweise
- Ein manueller Querverweis ist ein frei eingegebener, auch mehrzeiliger Text (Eigenschaft „Manueller Querverweis“, über (Neu) in der Eigenschaftsliste der Registerkarte <Funktionskategorie> hinzuzufügen; Zeilenumbruch über Kontextmenü bzw. Strg+Eingabe im Dialog Mehrzeilige Eingabe). EPLAN zeigt ihn **zusammen mit** dem automatischen Querverweis am Schaltzeichen, und zwar **davor**. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_manuelleqvwerzeugen.htm

### 18.3 Montageplattenquerverweise (Schaltschrankaufbau)
- Wird der Artikel einer Hauptfunktion (Beispiel Schützspule -K1 mit Artikel, der Breite/Höhe in den Montagedaten hat) über den 2D-Schaltschrankaufbau-Navigator auf eine Montageplatte einer Seite vom Typ „Schaltschrankaufbau“ abgesetzt, entsteht ein **wechselseitiger** Querverweis: an der Spule im Schaltplan auf den platzierten Artikel und am Artikel auf die Spule. Voraussetzung: in Einstellungen > Anzeige (Gruppenfeld Betriebsmittelquerverweise anzeigen) ist die Beziehung allpoliger Schaltplan ↔ Schaltschrankaufbau in beide Richtungen freigegeben. Die Abmessungen der abgesetzten Grafik stammen aus den Artikeldaten.
- **Markierung:** In Einstellungen > Allgemein, Gruppenfeld Markierung, Feld Schaltschrankaufbau, kann ein beliebiges Zeichen (Beispiel „%“) eingetragen werden, das Montageplattenquerverweise an Hauptfunktionen kennzeichnet; wirksam nach Ansicht > Neu zeichnen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_montageplattenqvwerzeugen.htm

### 18.4 Querverweise auf Betriebsmittellisten
- An Haupt- und Nebenfunktionen kann ein zusätzlicher Querverweis auf die **Betriebsmittelliste** (Auswertungsseite) stehen, in der das Betriebsmittel steht; umgekehrt verweisen die Einträge der Betriebsmittelliste auf die Hauptfunktion im Schaltplan. Ablauf im Beispiel: Schaltzeichen mit sichtbarem BMK (-T1, -F1, -H1), Hauptfunktion gesetzt, Artikel zugeordnet; dann Auswertung „Betriebsmittelliste“ erzeugen. Voraussetzung: Anzeige-Einstellung allpoliger Schaltplan ↔ Betriebsmittelliste freigegeben.
- Option in Einstellungen > Allgemein: **„Auch Nebenfunktionen zeigen auf die Betriebsmittelliste“** (sonst nur Hauptfunktionen).
- **Markierung:** Gruppenfeld Markierung, Feld Betriebsmittelliste, ein frei wählbares Zeichen (Beispiel „>“) unterscheidet diese Verweise von den übrigen; wirksam nach Neu zeichnen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_qvwaufbetriebsmittellisten.htm

### 18.5 Bedeutung für QET
- **Manueller QVW vor dem automatischen:** Steht im Original vor dem gewohnten Verweis ein freier Text (z. B. Hinweis auf ein Fremdgerät oder eine andere Dokumentation), ist das ein manueller Querverweis. Im Nachbau als eigenes Textfeld/Element-Info **links bzw. oberhalb** des QET-XRef platzieren; nicht in die XRef-Formel mischen, weil er nicht navigierbar ist.
- **Markierungszeichen erkennen:** Ein einzelnes Sonderzeichen vor einem Verweis (z. B. `%` oder `>`) bedeutet „Verweis auf Schaltschrankaufbau“ bzw. „auf Betriebsmittelliste“, nicht auf eine Schaltplanseite. Solche Verweise beim Verknüpfen (`verknuepfen.py`) **nicht** als Master/Slave-Beziehung zwischen Schaltplanseiten werten; im PDF-Export sollten sie, falls nachgebildet, auf die Aufbau- bzw. Listenseite springen (Vermutung: im EPLAN-PDF sind sie wie andere QVW verlinkt).
- **Aufbau- und Listen-Verweise sind wechselseitig:** Erzeugt der Fork Montageaufbau- oder Betriebsmittellisten-Folios, sollte jeder Eintrag dort auf die Hauptfunktion zurückverweisen, und die Hauptfunktion auf den Eintrag – analog zu Haupt-/Nebenfunktion.
- **Seitentyp-Matrix:** Ob ein Verweistyp überhaupt erscheint, hängt in EPLAN an der Einstellung „Anzeige“ je Seitentyp-Paar. Fehlt im Original z. B. ein Verweis von der Übersicht auf den Schaltplan, ist das eine Projekteinstellung, kein Zeichenfehler; im Nachbau ebenso weglassen.

## 19. Quellen

Alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ (sofern nicht anders):
xessettingsgui_k_grundlagen.htm, xessettingsgui_k_querverweisarten.htm, xessettingsgui_h_symbolquerverweiseerzeugen.htm,
xessettingsgui_h_paarquerverweiseerzeugen.htm, xessettingsgui_h_einstellungenkontaktspiegel.htm,
xessettingsgui_d_einstellungenprojektmotorschutzschalter.htm, devicetaggui_r_anzeigekontaktspiegel.htm,
xessettingsgui_k_qvwjapanische.htm, xessettingsgui_d_einstellungenprojektqvwallgemein.htm,
xessettingsgui_d_einstellungenprojektqvwanzeige.htm, xessettingsgui_h_qvwanzeigeoptimieren.htm,
xessettingsgui_h_abbruchstellenquerverweiseerzeugen.htm, xessettingsgui_d_einstellungenprojektabbruchstellen.htm,
xessettingsgui_h_spsquerverweiseerzeugen.htm, interruptionpointgui_k_start.htm,
interruptionpointgui_k_darstellungabbruchstellen.htm, interruptionpointgui_h_abbruchstelleneinfuegen.htm,
devicetaggui_r_symbolfunktionsdaten.htm, devicetaggui_k_bmeigenschaften.htm, Glossary_o_funktionen.htm,
Glossary_o_hauptfunktionen.htm, Glossary_o_nebenfunktionen.htm, Glossary_o_verbindungsdefinitionspunkte.htm,
potentialbrowsergui_k_prinzippotenziale.htm, potentialbrowsergui_h_pdpzeichnen.htm, connectionbrowsergui_k_start.htm,
connectionbrowsergui_k_vdp.htm, locationbox_k_grundlagen.htm, formeditorgui_h_normblaetterpfadeaktualisieren.htm,
projects_h_normblattpfad.htm, pagebrowsergui_d_projekteinstellungen.htm, prjmanagementgui_r_vererbung.htm,
prjmanagementgui_r_darstellungerwprjstrukturen.htm.

Ergänzt (Abschnitte 15–17): interruptionpointgui_d_abbruchstellenabbuchen.htm,
interruptionpointgui_d_abbruchstellensortieren.htm, interruptionpointgui_d_projektdatenabbruchstellen.htm,
interruptionpointgui_d_einstellsmartconnecting.htm, interruptionpointgui_d_einstellsmartabbruchstellen.htm,
interruptionpointgui_h_abbruchstellenbearbeiten.htm, interruptionpointgui_h_standardposabbruchstellenbeschrfestlegen.htm,
connectionbrowsergui_h_smartconnecting.htm, egedgui_h_verbindungsstueckeaustauschen.htm, egedgui_h_bmkzusammenfuehren.htm,
egedgui_h_funktionstexteabgleichen.htm, egedgui_k_sammelschienen.htm, egedgui_k_verteiler.htm, blackbox_k_start.htm,
blackbox_k_arbeitsweise.htm, blackbox_k_schachteln.htm, blackbox_h_anschluesseeinfuegen.htm, inplaceeditinggui_k_start.htm,
gededitgui_h_formatuebertragen.htm, gededitgui_k_gruppierung.htm, gededitgui_k_tastaturbefehle.htm,
gededitgui_h_symboltexteverschieben.htm, symboleditorgui_d_eigenschaftsplatzierung.htm, functionaldiagramgui_k_prinzip.htm,
singlepole_k_straenge_in_einpoligerdarstellung.htm, netbasedwires_k_start.htm, gedviewer_d_einstellungenmakrosaktualisieren.htm.

Weitere:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2025/Content/htm/adjustdata_h_nebenfunktionaendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/singlepole_k_hintergrund.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/interruptionpointgui_k_darstellungabbruchstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/xessettingsgui_k_querverweisarten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/xessettingsgui_d_einstellungenprojektqvwallgemein.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/xessettingsgui_d_einstellungenprojektabbruchstellen.htm
- https://www.eplan.help/en-us/infoportal/content/Plattform/2.9/Content/htm/messages_p_011007.htm
- https://eplan.help/en-us/Infoportal/Content/Plattform/2.9/Content/htm/news_p_preplanning_querverweise_auswertungen_241563.htm
- QET-Fork (gelesen, nicht geändert): `sources/properties/xrefproperties.cpp/.h`, `sources/qetgraphicsitem/crossrefitem.cpp`,
  `sources/autoNum/assignvariables.cpp`, `sources/properties/reportproperties.cpp`, `sources/qetgraphicsitem/element.h`,
  `sources/qetgraphicsitem/conductordefinitionmark.h`

Querverweis-Einstellungen (§18):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_einstellungenqvwanzeige.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_manuelleqvwerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_montageplattenqvwerzeugen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_qvwaufbetriebsmittellisten.htm
