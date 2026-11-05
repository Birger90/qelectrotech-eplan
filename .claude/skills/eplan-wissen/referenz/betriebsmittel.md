# EPLAN: Betriebsmittel, Funktionen, Eigenschaftendialoge, Geräte

Was ein Betriebsmittel in EPLAN logisch ist (Funktion, Haupt-/Nebenfunktion, Funktionsdefinition, Darstellungsart),
welche Registerkarten der Eigenschaftendialog hat und was die Felder fachlich bedeuten, wie Artikel und Geräte
(Funktionsschablonen, Makros) an Betriebsmittel kommen und wie Geräteliste, benutzerdefinierte Eigenschaften,
tabellarische Bearbeitung und Datenabgleich arbeiten. Grundlage: EPLAN-Plattform-Hilfe 2.9, in eigenen Worten.
URL-Präfix aller Quellen, sofern nicht anders: `https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/`
(in den Quellenangaben wird nur der Dateiname genannt). „Vermutung“ = nicht durch die Hilfe belegt.

## Inhalt

1. Grundbegriffe: Betriebsmittel, Funktion, Haupt-/Nebenfunktion
2. Identifizierende und beschreibende Eigenschaften
3. Registerkarte <Funktionskategorie> (Felder und Bedeutung)
4. Registerkarte Symbol-/Funktionsdaten: Funktionsdefinition, Darstellungsart, Logik
5. Darstellungsarten, grafische und externe Funktionen
6. BMK-Übernahme (Suchrichtung, Kästen)
7. Anschlussbezeichnungen und ihre Übernahme
8. Registerkarte Anzeige: Eigenschaftsanordnung, Kontaktspiegel, Textplatzierung
9. Registerkarten spezieller Betriebsmittel (Klemme, Stecker, Verbindung, VDP, Abbruchstelle, Strukturkasten, SPS, Kabel)
10. Artikel am Betriebsmittel, Geräte, Funktionsschablonen, Makros
11. Löschen, Kopieren, nicht-platzierte Funktionen, Gerätegruppen
12. Geräte- und Änderungsschutz
13. Geräteliste (Betriebsmittelliste der Vorplanung)
14. Benutzerdefinierte Eigenschaften
15. Tabellarische Bearbeitung (Funktionsdatentabelle)
16. Daten abgleichen
17. Navigatoren (Betriebsmittel-Navigator)
18. Bedeutung für QET (Zusammenfassung)
19. Ergänzung: Dialoge BMK-Auswahl, Eigenschaftsauswahl, Symboldaten
20. Geräte verschalten (nicht-platzierte Verbindungen)
21. Quellen

---

## 1. Grundbegriffe: Betriebsmittel, Funktion, Haupt-/Nebenfunktion

- EPLAN unterscheidet **allgemeine** Betriebsmittel (Motor, Sicherung, Ventil …) und **spezielle** (Klemmen, Kabel,
  SPS-Anschlüsse, Gerätekästen …). Allgemeine werden über die Symbolauswahl eingefügt, bekommen ein BMK vorgeschlagen,
  verbinden sich automatisch (Autoconnecting), erscheinen in Auswertungen und werden automatisch querverwiesen; die
  Logik der Funktion ist änderbar (z. B. für das Querverweisverhalten). Quelle: devicetaggui_k_start.htm
- Jedes platzierte Schaltzeichen beruht auf einer **Funktion** mit einer **Funktionsdefinition** (z. B. „Schalter“),
  die ein Logikmodell mitbringt; Teile der Anschlusslogik sind an der Funktion änderbar. Quelle: devicetaggui_k_bmeigenschaften.htm
- Ein **Betriebsmittel** (Gerät) = alle Funktionen mit gleichem vollständigem BMK. Genau eine davon ist die
  **Hauptfunktion** (z. B. Schützspule); nur sie trägt Artikel. Nebenfunktionen (Kontakte) haben keine Registerkarte
  Artikel. Wird ein BMK abgebucht, unter dem schon eine Hauptfunktion existiert, oder eine bereits platzierte
  Hauptfunktion erneut platziert, schaltet EPLAN „Hauptfunktion“ an der Zielfunktion automatisch aus.
  Quelle: devicetaggui_r_bmdaten.htm, devicetaggui_r_symbolfunktionsdaten.htm, devicetaggui_h_artikeldatenzuweisen.htm
- Klemmen nutzen statt „Hauptfunktion“ die Eigenschaft **Hauptklemme** (Kompatibilitätsoption „Klemmen als
  Hauptfunktion zulassen“ nur für Altverhalten ≤ 2.5, nicht empfohlen). Quelle: devicetaggui_r_symbolfunktionsdaten.htm
- Betriebsmittel ohne Hauptfunktion gelten als fehlerhaft, werden aber weiter verwaltet. Quelle: devicelistgui_k_loeschenkopieren.htm
- **Eigenschaften (übergreifend)** (Optionen-Menü oder Kontextmenü): bearbeitet alle Darstellungen einer Funktion
  gemeinsam; zusätzlich erscheint die Registerkarte „<Funktionskategorie> (BM)“ für die Hauptfunktion, also das ganze
  Betriebsmittel. Ohne den Modus sind verteilte Darstellungen einzeln bearbeitbar. Quelle: devicetaggui_r_bmdaten.htm,
  devicetaggui_h_bmkaendern.htm
- BMK ändern: übergreifend → an allen Funktionen und Darstellungen; an der Funktion (übergreifend) → alle Darstellungen
  dieser Funktion; ohne Modus → nur die gewählte. Im BM-Navigator „Umbenennen“ ändert alle Darstellungen.
  „Betriebsmittel tauschen“ tauscht die BMK zweier Geräte an allen Funktionen. Quelle: devicetaggui_h_bmkaendern.htm,
  devicetaggui_h_bmtauschen.htm

## 2. Identifizierende und beschreibende Eigenschaften

- **Identifizierend** (bestimmen, welche Funktion es ist): BMK, Anschlussbezeichnung, Funktionsdefinition, Darstellungsart.
- **Beschreibend**: technische Kenngrößen, Funktionstext, Graviertext, Montageort (beschreibend), Zusatzfelder,
  Bemerkung, manueller Querverweis.
  Quelle: devicetaggui_k_bmeigenschaften.htm
- Aufbau des BMK und Projektstruktur: siehe `kennzeichnung.md` (Abschnitte 1–8). Groß-/Kleinschreibung und Vorzeichen
  je Betriebsmittelart: Projekteinstellungen > Betriebsmittel > Allgemein; Syntaxprüfung: > BMK-Syntaxüberprüfung
  (Abschnitt in `nummerierung.md`). Quelle: devicetaggui_k_aufbauvonbm.htm

**Bedeutung für QET:** Zwei Schaltzeichen sind dieselbe Funktion nur, wenn BMK **und** Anschlussbezeichnungen **und**
Funktionsdefinition **und** Darstellungsart passen. Für die Master/Slave-Zuordnung reicht das BMK (gleiches Gerät);
für „ist das dieselbe Klemme/derselbe Kontakt in einpoliger und allpoliger Darstellung“ muss auch die
Anschlussbezeichnung übereinstimmen. Funktionstext, Graviertext, Montageort (beschreibend) gehören nie in den Schlüssel.

## 3. Registerkarte <Funktionskategorie> (Felder und Bedeutung)

Die erste Registerkarte heißt nach der Funktionskategorie (z. B. „Motor“, „Schließer“). Felder:

| Feld | Bedeutung |
|---|---|
| Sichtbares BMK | steht am Schaltzeichen; bei Online-Nummerierung vorgeschlagen; Strg+N = nächster freier Zähler |
| Vollständiges BMK | Strukturkennzeichen der Umgebung + sichtbares BMK; nur über Dialog „Vollständiges BMK“ änderbar |
| Anschlussbezeichnung | Bezeichnungen der Anschlüsse, durch Absatzmarke getrennt (Eingabe Strg+Enter); zeigt auch die der Folgeschaltzeichen, die das BMK übernehmen (Abschnitt 7) |
| Anschlussbeschreibung | je Anschluss ein Beschreibungstext |
| Technische Kenngrößen | z. B. Bemessungsdaten |
| Funktionstext | manueller Text; dazu automatisch: **Funktionstext (automatisch)** = manueller Text, sonst Pfad-Funktionstext; **Funktionstext (gemeinsam)** = eigener automatischer, sonst der der Hauptfunktion |
| Graviertext | Schild-/Gravurtext, Liste bietet projektweit schon verwendete Werte |
| Montageort (beschreibend) | nur Information, nicht identifizierend |
| Hauptfunktion | Kontrollkästchen, synchron mit Registerkarte Symbol-/Funktionsdaten |
| Tabelle Eigenschaften | weitere Eigenschaften, Kategorien „Daten“, „Einstellungen“, „Benutzerdefiniert“ |

- „Anschlussbezeichnung“ ist keine einzelne Eigenschaft; zum Anzeigen am Symbol dient **Anschlussbezeichnungen (alle)
  (ID 20038)** – enthält nur die eigenen, nicht die der Folgeschaltzeichen. Einen Gesamteintrag mit nur einem Wert zu
  überschreiben ändert nur den ersten Anschluss; Löschen des ganzen Eintrags löscht alle.
- „In Betriebsmittelliste unterdrücken“ (Kategorie Einstellungen) nimmt die Funktion aus der Betriebsmittelliste.
  Quelle: devicetaggui_r_bmdaten.htm
- Eigenschaftentabelle konfigurierbar (Kontextmenü Konfigurieren, benutzerbezogen gespeichert): angehakte Eigenschaften
  immer anzeigen, andere nur wenn gefüllt; gefüllte manuell veränderbare Eigenschaften lassen sich nicht ausblenden.
  „Eigenschaften / Werte einfügen“ überträgt kopierte Eigenschaften samt Werten von einem anderen Objekt.
  Quelle: devicetaggui_h_bmeigenschaftenbearbeiten.htm
- Dialog **Vollständiges BMK**: Tabelle Strukturkennzeichen (Spalte Wert, mit erweiterter Referenzkennzeichnung
  Sichtbar/Vollständig), darunter Vorziffer, Kennbuchstabe, Zähler, Unterzähler (Trennzeichen vor Unterzähler nicht
  editierbar) für BMK und untergeordnetes BMK; mit erweiterter Referenzkennzeichnung eine Zeile je Produktkennzeichen.
  Fluid nach DIN ISO 1219-2: Medienschlüssel, Schaltkreisnummer, Bauteilnummer, Vorziffer leer. Vor-/Trennzeichen
  stammen aus „Erweiterte Projektstrukturen“. Quelle: devicetaggui_d_vollstaendigesbmk.htm

**Bedeutung für QET:** EPLAN-Feld → QET-elementInformation (Vorschlag, Vermutung zur Zuordnung): sichtbares BMK → `label`,
Funktionstext → `function`, technische Kenngrößen → `description` oder `comment`, Graviertext → `auxiliary1`
(oder eigenes Feld), Montageort (beschreibend) → `comment`, nicht `location`. Anschlussbezeichnungen gehören in die
Terminal-Namen des Elements. Der „Pfad-Funktionstext“ (Text in der Pfadleiste) ist der Rückfall, wenn am Gerät kein
Funktionstext steht – beim Import/Nachbau für Stückliste und SPS-Kommentar mitführen.

## 4. Registerkarte Symbol-/Funktionsdaten: Funktionsdefinition, Darstellungsart, Logik

- **Symboldaten (Grafik)**: Symbol bzw. Symbolvariante tauschen. **Funktionsdaten (Logik)**: Funktionsdefinition
  tauschen, Anzahl Anschlüsse (fest aus der Definition), Darstellungsart, Kontrollkästchen, [Logik] öffnet den Dialog
  Anschlusslogik. Quelle: devicetaggui_r_symbolfunktionsdaten.htm
- Kontrollkästchen:
  - **Hauptfunktion** (s. o.).
  - **Sicherheitsfunktion**: sicherheitsrelevantes Signal (z. B. F-Eingang, Kontakt Sicherheitsschalter); EPLAN kann
    prüfen, ob der Artikel eine passende als Sicherheitsfunktion markierte Funktionsschablone hat.
  - **Eigensicher**: alle in der Anschlusslogik dafür freigegebenen Anschlüsse werden eigensicher.
  - **Signaltrennend**: Signal wird an der Funktion nicht weitergeleitet. Aus = signalweiterleitend; dann muss in der
    Anschlusslogik „Potenzial weiterleiten nach“ die Gegenanschlüsse nennen (bei Klemmen/Sicherungen vorbelegt, bei
    Lampen/Motoren selbst eintragen).
  - **Netzverbindend**: Netz läuft durch die Funktion durch; Standard sind die meisten Funktionen netztrennend. Ein Netz
    endet an jeder potenzial-, signal- oder netztrennenden Funktion. Signaltrennend impliziert netztrennend.
    Netzverbindend wirkt **nicht** auf die Netzerweiterung der Verbindungsnummerierung.
  Quelle: devicetaggui_r_symbolfunktionsdaten.htm

**Bedeutung für QET:** Für Potenzial-/Drahtnummern-Verfolgung im Verknüpfungsskript braucht jedes Bauteil die Info,
welche Anschlüsse ein Potenzial durchreichen (Klemme 1↔2, Sicherung 1↔2, Kontakt nur bei Brücke/Klemme nicht).
In QET gibt es das nicht als Elementlogik; Vermutung: im Import eine Tabelle „durchleitende Anschlusspaare“ je
Funktionsdefinition mitführen (Klemmen, Sicherungen, Brücken, Stecker durchleitend; Spulen, Lampen, Motoren trennend).

## 5. Darstellungsarten, grafische und externe Funktionen

- Darstellungsarten: Allpolig, Einpolig, Übersicht, Paarquerverweis, RI-Fließbild, Topologie, Funktional,
  Funktionsübersicht (Fluid), Extern, Grafik. Nicht platzierte Funktionen nur Allpolig/Einpolig. Eine Funktion darf in
  mehreren Darstellungsarten vorkommen, jede aber **nur einmal**. Bei abweichenden Daten zwischen Darstellungen
  entscheidet eine feste Reihenfolge der Darstellungsarten, welche Werte ausgewertet werden.
- **Paarquerverweis**: Schaltzeichen, das auf sein verdrahtetes allpoliges Gegenstück verweist (und umgekehrt); ändert
  das Querverweisverhalten von Haupt- und Nebenfunktionen.
  Quelle: devicetaggui_r_symbolfunktionsdaten.htm
- **Grafisch/Extern**: werden nicht direkt ausgewertet (keine Meldungen, Auswertungen, Querverweise, Artikel in
  Stücklisten), aber als **Ziel** erfasst. Grafisch entsteht durch Platzieren auf einer Grafikseite oder Umschalten der
  Darstellungsart; extern nur durch Umschalten. Beide werden andersfarbig gezeichnet, nicht im BM-Navigator gezeigt,
  nicht zurückgebucht, nicht querverwiesen, nicht geprüft, nicht für automatische Kabel und nicht für die
  Verbindungsnummerierung berücksichtigt. Quelle: devicetaggui_k_grafischeschaltzeichen.htm,
  devicetaggui_k_verhaltengrafischerschaltzeichen.htm
- **Verbindungen** übernehmen ohne VDP (bzw. VDP „Nicht definiert“) die Art ihrer Ziele: ein grafisches Ziel → grafisch;
  ein externes → extern (grafisch + extern → grafisch); nur zwei logische Ziele → logisch. Ein VDP „Einpolig/Allpolig“
  zwischen zwei grafischen Zielen erzwingt eine logische Verbindung. Nicht platzierte Verbindungen sind immer logisch.
  Potenziale und SPS-Zielverfolgung laufen auch über grafische Verbindungen. Quelle: devicetaggui_k_grafischeschaltzeichen.htm
- **Ziele in Auswertungen** hängen an der Verbindung, nicht am Ziel: über grafische Verbindung kein Ziel; über externe
  ja. Externe Verbindungen erscheinen indirekt (z. B. Klemmenplan), nicht in Verbindungslisten.
- BMK-Übernahme und Anschlussbezeichnungsübergabe funktionieren auch bei grafischen/externen Funktionen.
  Strukturkennzeichen nur an grafischen Funktionen erscheinen nicht in der Strukturkennzeichen-Verwaltung.
  Quelle: devicetaggui_k_verhaltengrafischerschaltzeichen.htm

**Bedeutung für QET:** Im EPLAN-PDF andersfarbige (oft grau) Symbole/Leitungen sind meist **grafisch oder extern** –
z. B. Fremdgeräte, Hinweisdarstellungen. Beim Nachbau: zeichnen, aber nicht in Stückliste/Querverweise aufnehmen und
keine Drahtnummer vergeben (Vermutung: in QET als Element ohne `label`-Verknüpfung oder mit eigener Kennung
„grafisch“ in einem Zusatzfeld markieren, damit Verknüpfungsskript und Stücklisten-Export sie überspringen).

## 6. BMK-Übernahme (Suchrichtung, Kästen)

- Funktionen **ohne sichtbares BMK** übernehmen das BMK der nächsten Funktion in Suchrichtung. Erkennungsmerkmal ist der
  **Einfügepunkt**: aus der vorigen Spalte nur bei gleicher Y-Höhe, aus der vorigen Zeile nur bei gleicher X-Höhe.
  Hat die gefundene Funktion selbst kein BMK, wird weitergesprungen (wenn auch dort eine Suchrichtung gilt); sonst
  leeres BMK und Meldung. Quelle: devicetaggui_k_bmkuebernahme.htm
- Eigenschaft **BMK-Übernahme: Suchrichtung**: „Entsprechend der Normblattausrichtung“ (IEC senkrecht → von **links**;
  NFPA waagerecht → von oben; GOST → rechts/unten), „Alternativ“ (IEC → von **oben**), „Nur von Kästen“ (nur vom
  umgebenden Geräte-/SPS-Kasten, sonst keine Übernahme). Normblatteigenschaft „BMK-Übernahme: Nur von Kästen“ gilt
  für ganze Seiten. Generell abschalten: Online-Nummerierung deaktivieren. Quelle: devicetaggui_k_bmkuebernahme.htm,
  devicetaggui_h_bmkuebernahmefestlegen.htm
- Übernahme auch von grafischen und externen Funktionen. Übernahme-Quellen: allgemeine Funktion, Klemme, Steckerkontakt,
  Gerätekasten, Geräteanschluss (Kabeldefinition/Abschirmung nur eingeschränkt). Nie beteiligt: Strukturkästen (als
  Quelle von links/oben), Klemmenleisten-/Steckerdefinitionen, Kabelverbindungen, VDP, Abbruchstellen, Verbindungssymbole
  (Winkel, T). Quelle: devicetaggui_k_bmkuebernahmesonder.htm
- **Gerätekasten**: Funktionen im Kasten übernehmen vom Kasten (bzw. vom Bauteil gleicher Höhe im Kasten), nie von außen;
  außen liegende Funktionen dürfen von innen übernehmen. Zwei Gerätekästen auf gleicher Höhe werden nicht verkettet.
  Klemmen/Steckerkontakte übernehmen auch von Kästen. Gleiches gilt für Strukturkästen. Quelle: devicetaggui_k_bmkuebernahmesonder.htm
- **SPS-Anschlüsse**: Suchrichtung übernimmt auch die Steckerkontaktbezeichnung; auf Übersichtsseiten wird standardmäßig
  nach oben gesucht. Quelle: devicetaggui_k_bmkuebernahmesonder.htm
- Verschieben mit **Umschalt**: das vollständige BMK bleibt erhalten (das sichtbare kann sich ändern); Stecker- und
  Kanalbezeichnung werden so nachgeführt, dass die (automatisch)-Werte gleich bleiben. Normales Verschieben kann das BMK
  ändern. Quelle: devicetaggui_k_bmkuebernahme.htm

**Bedeutung für QET:** Im EPLAN-PDF steht an vielen Kontakten/Klemmen **kein** BMK – es kommt aus der Übernahme von links
(gleiche Einfügepunkt-Höhe) oder vom umgebenden Gerätekasten. Der Nachbau darf das nicht als „ohne BMK“ deuten:
Verknüpfungsskript muss für Elemente ohne label das BMK nach EPLAN-Regel suchen (nach links auf gleicher Y des
Einfügepunkts, sonst umgebender Geräte-/SPS-Kasten) und als label setzen (Anzeige ggf. ausblenden, damit die Optik wie
im Original bleibt). Mehrpolige Schütz-/Leistungsschalter-Kontakte, die als Einzelsymbole nebeneinander stehen, sind so
zu einem Gerät verbunden.

## 7. Anschlussbezeichnungen und ihre Übernahme

- Standard: Anschlussbezeichnungen kommen aus der **Funktionsdefinition** (Liste möglicher Werte, erster Eintrag =
  Vorbelegung) bzw. bei Geräten aus den vorgegebenen Gerätedaten (werden beim Platzieren abgebucht). Bei Geräte-/
  Artikelauswahl werden im Plan eingetragene Bezeichnungen **überschrieben**, außer das Auswahlkriterium
  „Anschlussbezeichnung“ ist aktiv. Quelle: devicetaggui_k_anschlussbezuebergabe.htm
- **Übernahme aus Suchrichtung** (Projekteinstellung Verwaltung > Kompatibilität „Anschlussbezeichnung übernehmen“,
  dazu Online-Nummerierung): an der ersten Funktion werden alle Bezeichnungen für die Folgeschaltzeichen (die das BMK
  übernehmen) eingetragen, z. B. `1;2;3;4;5;6` an drei nebeneinander stehenden Leistungsschließern → 1/2, 3/4, 5/6.
  Nur Überzählige beeinflussen die Folgenden; `1;2;;;;` leert die Folgeschaltzeichen; zu viele werden verworfen.
  Nicht für Kabel, Kabelverbindungen, Gerätekästen, Klemmen, Steckerkontakte. Quelle: devicetaggui_k_anschlussbezuebergabe.htm,
  devicetaggui_h_anschlusbezuebernehmen.htm
- Bei Klemmen und Steckerkontakten: Eingabe `X1:5` im sichtbaren BMK überträgt alles nach dem Doppelpunkt in die
  Bezeichnung (Eigenschaft Klemmen-/Steckerkontaktbezeichnung). Quelle: devicetaggui_r_klemmen.htm, devicetaggui_r_steckerkontakt.htm

## 8. Registerkarte Anzeige: Eigenschaftsanordnung, Kontaktspiegel, Textplatzierung

- Die **Eigenschaftsanordnung** legt fest, welche Eigenschaften am Schaltzeichen platziert sind, mit welchen
  Anzeigeeinstellungen, außerdem Anschluss-Texte und Kontaktspiegel. Vordefinierte Anordnungen sind am Symbol
  hinterlegt („Standardvorgabe“); jede Änderung schaltet auf „Benutzerdefiniert“. Unterregisterkarten: Schaltzeichen
  (bzw. 3D-Funktionen), Anschlüsse (nur bei Symbolen mit Anschlüssen), Kontaktspiegel. Texte können **angedockt**
  werden (gemeinsam verschieben, mehrzeilige Texte überlappen nicht). Quelle: devicetaggui_r_anzeige.htm,
  devicetaggui_k_bmeigenschaften.htm
- Benutzerdefinierte Anordnungen werden **projektbezogen** gespeichert und gelten je **Symbolvariante** (3D: je
  Funktionsdefinition); „Als Standard verwenden“ macht sie zur Vorgabe der Variante. Überschreiben einer Anordnung
  ändert alle Schaltzeichen, die sie verwenden. Export als `*.emc`; zentrale Verwaltung über Projektdaten >
  Eigenschaftsanordnungen konfigurieren; Zuweisung auch per Eigenschaft **Eigenschaftsanordnung (ID 19307)**
  (Platzhalterobjekte, Symbolleistenaktion XEsSetPropertyAction). Beim Kopieren/Makro-Einfügen in andere Projekte
  entscheidet die Projekteinstellung „Eigenschaftsanordnungen aus dem Projekt übernehmen“ bei Namensgleichheit.
  „Eigenschaftsanordnung zurücksetzen“ stellt Standard wieder her. Über [Bearbeiten] lässt sich z. B. BMK (sichtbar)
  durch BMK (vollständig) ersetzen. Für VDP gibt es „Als Standard für Verbindungen/Kabelverbindungen verwenden“
  (auch für bei der Verbindungsnummerierung entstehende VDP). Quelle: devicetaggui_h_eigschanordnungen.htm,
  devicetaggui_d_eigschanordnungspeichern.htm, devicetaggui_d_eigschanordnungexportieren.htm
- Anzeigeeigenschaften je Text: Format, Wert/Einheit, Position, Textrahmen, Positionsrahmen, Datum/Zeit. Bezugspunkt
  standardmäßig der **Einfügepunkt** des Schaltzeichens. „Y-Koordinate automatisch (Schaltplanpfad)“ legt Texte
  pfadweise in eine feste Höhe (Wert in **ID 12062**, bezogen auf Seitennullpunkt), „X-Koordinate automatisch“ zeilenweise
  (ID 12063) – so entstehen Texte in einer Zeile unter dem Schaltplan. Positionsrahmen hat Vorrang vor Ausrichtung;
  „Text einpassen erlaubt“ für Rahmen. Quelle: devicetaggui_r_anzeigeeigenschaften.htm
- **Kontaktspiegel**: Ausrichtung „Im Schaltplanpfad“ (unten bzw. rechts auf der Seite im Pfad des Schaltzeichens)
  oder „Am Schaltzeichen“. Einstellungen projektweit (Querverweise/Kontaktspiegel), am Schaltzeichen überschreibbar
  („Benutzerdefiniert“, hat Vorrang). Optionen u. a.: eigenen QVW darstellen, Symbolspiegel (alle Schaltzeichen, Hauptfunktion
  unterdrückt), senkrecht anordnen, QVW von Öffner/Wechsler unterstreichen, Symbole anzeigen, Tabellenform mit
  Überschriften (z. B. S/Ö aus Projekteinstellung), Tabellenkreuz variabler Länge, Drehung, Anzeige 1./2. Artikel/Typ,
  eingebettetes Formular. Position relativ zum Einfügepunkt. Quelle: devicetaggui_r_anzeigekontaktspiegel.htm
  (Details zu Querverweisen siehe `querverweise.md`).

**Bedeutung für QET:** Was im EPLAN-PDF als Text an einem Gerät steht, ist eine Auswahl der Eigenschaften gemäß
Eigenschaftsanordnung – nicht alle Daten sind sichtbar. Die Texte gehören zum Element (QET: dynamische Texte des
Elements, nicht freie Folio-Texte), damit sie beim Verschieben mitgehen. Texte in einer Linie unter dem Plan („Y
automatisch“) sind trotzdem Elementtexte. Kontaktspiegel unter der Spule gehören zur Spule (QET: Querverweis-Anzeige
des Masters), nicht als freie Grafik.

## 9. Registerkarten spezieller Betriebsmittel

- **Klemme**: sichtbares/vollständiges BMK (Leiste), **Bezeichnung** (Position in der Leiste; Eigenschaft
  Klemmen-/Steckerkontaktbezeichnung), Beschreibung, Anschlussbezeichnungen, **Etage** (0 = einfache Klemme, ≥1 =
  Mehrstockklemme, 1 = unterste Etage), **Stegbrücke** (Automatisch / Brückenanfang / Brückenende / keine automatische
  Brücke; automatische Brücken nur bei nebeneinanderliegenden, irgendwie verbundenen Klemmen mit Stegbrückenmöglichkeit;
  manuelle Brücken werden am Brückenanfang gespeichert), **Hauptklemme**, **Teilklemme** (stellt nur einen Teil der
  Anschlüsse dar). Eigenschaften: Klemmenausführung (Durchgangs-, Trenn-, Dioden-… für Grafiken in Auswertungen),
  „Gleiche Bezeichnung erlaubt“ (z. B. PE; nicht bei mehreren Darstellungsarten verwenden), „Klemme: Geräteposition“
  (alle Klemmen eines Klemmengeräts, z. B. Mehrstock, gleich; Sortierkennung ordnet innerhalb). Quelle: devicetaggui_r_klemmen.htm
- **Klemmenleistendefinition**: Klemmen-/Anschlussplanformular (Vorrang vor Auswertungseinstellung), Funktionstext,
  Montageort, „Hauptklemmen nicht verwenden“, Schalter „Keine Ausgabe im Klemmenplan/Anschlussplan/Übersicht/Aufreihplan“.
  Quelle: devicetaggui_r_klemmenleisten.htm
- **Steckerdefinition** analog (Stifte, Buchsen oder beides; Steckerplanformular). **Steckerkontakt**: Bezeichnung = Position
  im Stecker, „Zuordnung zu Stift-/Buchsenseite“ (Automatisch über gleiches BMK + Funktionsdefinition, Stift- oder
  Buchsenseite), Stegbrücke nur manuell (Anfang/Mitte/Ende). Quelle: devicetaggui_r_steckerdefinition.htm, devicetaggui_r_steckerkontakt.htm
- **Registerkarte Verbindungen** an Klemmen/Steckerkontakten (nur mit Kompatibilitäts-Einstellung): je Anschluss mit
  Anschlussart „Ader/Draht“ eine Unterkarte mit Zielen; Spalte **Anschluss** intern/extern, **Kabelanschluss**
  (für automatische Kabelerzeugung), **Form. pos.** (als wievieltes Ziel A–D im Klemmenplan), **Potenzialtyp**
  (−, +, L, M, N, PE, PEN, SH, undefiniert). Intern/extern tauschen im Dialog Klemmenleiste bearbeiten.
  Quelle: devicetaggui_r_verbindungenklemmen.htm
- **Verbindung / Verbindungsdefinitionspunkt (VDP)**: Verbindungsbezeichnung (aus Verbindungsnummerierung oder manuell,
  „Neue Bezeichnung ermitteln“), Beschreibung (nicht identifizierend), Kabel/Paket-BMK, **Farbe/Nummer** (identifizierend,
  Ader), **Paarindex** (Aderpaar), Querschnitt/Durchmesser mit Einheit, Darstellungsart (überschreibt die aus den Zielen
  ermittelte), Funktionsdefinition. VDP zusätzlich **Erlaubte Daten**: alle / keine Verbindungsbezeichnung / nur
  Verbindungsbezeichnung. Quelle: devicetaggui_r_verbindung.htm, devicetaggui_r_vdp.htm
- **Abbruchstelle**: BMK (Abbuchen über Dialog Abbruchstelle abbuchen), **Sortierkennung** (Standard 100 = Sortierung
  nur nach Seite/Zeile/Spalte), Beschreibung (bei der Quelle angezeigt), **Sternquelle**. Quelle: devicetaggui_r_abbruchstelle.htm
  (Logik siehe `querverweise.md`).
- **Strukturkasten** (Registerkarte Kennzeichen): Strukturkennzeichen-Tabelle; das sichtbare BMK zeigt standardmäßig
  nur die **geänderten** Kennzeichen (Beispiel: Kasten `=AP+ST1`, Anlage → ANL2: sichtbar `=ANL2`, vollständig
  `=ANL2+ST1`). Quelle: devicetaggui_r_locationbox.htm
- **SPS-Anschluss**: Adresse (nicht identifizierend, eindeutig je CPU), symbolische Adresse (manuell, sonst ermittelt
  = BMK des angeschlossenen Sensors/Aktors über Zielverfolgung), Kanalbezeichnung (eindeutig je SPS-Karte, automatisch
  oder manuell; unterdrückbar ID 20578), Steckerbezeichnung (automatisch unterdrückbar ID 20579), Funktionstext
  (Rückfall: Funktionstext des angeschlossenen Sensors/Aktors). Versorgungsanschlüsse werden nicht adressiert.
  **SPS-Karte**: Startadressen (getrennt für E/A), Adressbereich STEP 7 Classic. **SPS-Struktur**: Baugruppenträger ab 0
  je Station lückenlos, CPU-Name projektweit eindeutig, Bus-Schnittstelle: Name (ID 20447). Quelle: devicetaggui_r_spsanschluss.htm,
  devicetaggui_r_spskarte.htm, devicetaggui_r_spsstruktur.htm, devicetaggui_r_busdaten.htm (Details in `sps.md`).
- **Kabel**: Aderanzahl (z. B. „4G“/„3+PE“: PE/PEN/N als enthaltene Ader erkannt), Querschnitt, Abschirmung.
  Quelle: devicetaggui_r_kabeldaten.htm (Details in `kabel.md`).
- **Netzdefinitionspunkt** hat Vorrang vor Potenzialdefinitionspunkt; Eigenschaften gelten für alle Verbindungen des
  Netzes. Quelle: devicetaggui_r_ndp.htm, devicetaggui_r_pdp.htm, devicetaggui_r_netzverbindung.htm
- **Format**: grafische Bestandteile (Geräte-/SPS-Kasten, Abschirmung, Kabeldefinitionslinie) – u. a. „Unsichtbar“
  (Element existiert, wird nicht gezeigt). Quelle: devicetaggui_r_format.htm

**Bedeutung für QET:** Klemmen: Leisten-BMK (`X1`) in label, Klemmenbezeichnung (`5`) als eigenes Feld bzw. im Text
`X1:5`; Etage und Brücken in Bauteildaten des Klemmen-Elements (QET-Terminal-Strip-Funktionen). Ader-Farbe/Nummer am
Leiter (`conductor` „function“/„tension_protocol“ oder Text) – nur dort, wo das Original einen VDP/Kabel zeigt.

## 10. Artikel am Betriebsmittel, Geräte, Funktionsschablonen, Makros

- **Artikel nur an der Hauptfunktion**; Auswahl über [...] im Feld Artikelnummer (Mehrfachauswahl), Stückzahl
  vorbelegt 1. Quelle: devicetaggui_h_artikeldatenzuweisen.htm
- Zwei Arbeitsweisen: **logikorientiert** (Gerät auswählen, dann Symbol/Makro platzieren – komplette Logik aus den
  Artikeldaten sofort im Projekt) und **grafikorientiert** (erst zeichnen, dann Geräteauswahl); mischbar. Artikel dürfen
  ohne Grafik/BMK im Projekt existieren. Quelle: devicelistgui_k_start.htm
- **Gerätedefinition** = Satz von **Funktionsschablonen** am Artikel (Registerkarte Funktionsschablonen der
  Artikelverwaltung), je Schablone Funktionsdefinition + identifizierende Daten (v. a. Anschlussbezeichnungen) +
  optional Symbol. Gespeichert über den Artikel an der Hauptfunktion (Schütz: Spule; Stecker: Steckerdefinition);
  löschen/kopieren der Hauptfunktion löscht/kopiert die Definition. Schablonen sind nicht Projekt-, sondern Artikelbestandteil;
  sie werden von Funktionen mit gleichen identifizierenden Eigenschaften **überlagert**; Zusatzdaten nur an der
  Funktion. Nur Funktionen mit Gerätedefinition sind prüfbar. Das Symbolfeld des Artikels gilt nur ohne Schablonen.
  **Modul** = Sammlung von Artikeln mehrerer geschachtelter Betriebsmittel mit eigener Artikelnummer.
  Quelle: devicelistgui_k_prinzip.htm, devicelistgui_h_geraetedefinitionenanlegen.htm
- **Gerät einfügen** – Priorität der Artikeldaten: 1. Makro (Symbol-/Fenstermakro), 2. Funktionsschablonen,
  3. Grafikmakro mit Abmessungen (nur Schaltschrankaufbau), 4. nichts. BMK nach Kopiermodus der Online-Nummerierung.
  Ohne Hauptfunktion im Makro: erst die per Funktionsdefinition als Hauptfunktion definierte, sonst die grafisch erste.
  Mehrere Hauptfunktionen: Artikel an die erste passende; keine passende → zuerst **Gerätekasten**, sonst grafisch erste.
  Vorhandene Artikel an der Ziel-Hauptfunktion werden ersetzt; übertragen werden technische Kenngrößen,
  Anschlussbezeichnungen, Kennbuchstabe. Mit Schablonen: erst Hauptfunktion, dann Nebenfunktionen in
  Definitionsreihenfolge. [N] blättert durch Funktionen, [Umschalt]+[Tab] durch Darstellungsarten, [Tab] durch
  Makrovarianten. Quelle: devicelistgui_k_vorgehengeraeteeinfuegen.htm, devicelistgui_h_geraeteeinfuegen.htm
- **Neues Gerät** im Navigator: Standard (Einstellung „Beim Geräte erzeugen das Makro berücksichtigen“ aus) → nur die
  erste Hauptfunktion der Schablonen wird nicht-platzierte Funktion; restliche Schablonen bleiben frei zum Zuweisen.
  Einstellung an → Funktionen aus dem Makro; bei zusätzlichen Schablonen Variante für Variante (A, B, C …), solange
  freie Schablonen überlagert werden können, sonst Abbruch. Quelle: devicelistgui_k_vorgehengeraeteeinfuegen.htm,
  devicelistgui_k_platzieren.htm
- **Platzieren ≠ Makro einfügen**: Einfügen erzeugt neue Funktionen; Platzieren setzt vorhandene Funktionen/Schablonen
  in die Grafik des Makros. Ablauf: bestes Makro der Ziel-Darstellungsart (deckt die meisten Funktionen ab, bei
  Gleichstand wenigste Zusatzfunktionen), im Makro bestes Betriebsmittel, dann Zuordnung nach größter Übereinstimmung
  (gleiche Anschlussbezeichnungen zuerst, dann nur gleiche Funktionsdefinition), Rest wird zusätzlich eingefügt und
  bekommt dasselbe BMK. **Funktionsdaten aus Projekt/Schablone überschreiben das Makro** (leere überschreiben nicht),
  **Grafikdaten** (Symbol, Eigenschaftsanordnung, BMK-Suchrichtung) stammen aus dem Makro. **Schablonenverbund
  (allpolig)** platziert zusammengehörige Funktionen gemeinsam (nur allpolige Seiten). Quelle: devicelistgui_k_vorgehengeraeteplatzieren.htm
- **Stecker-/Kanalbezeichnung** werden vom Artikel nur übertragen, wenn am Ziel schon ein Wert steht (nie durch leer
  überschrieben) oder die BMK-Übernahme sonst einen falschen (automatisch)-Wert ergäbe; in Makros wird die
  (automatisch)-Eigenschaft gespeichert und zur Zuordnung verwendet. Quelle: devicelistgui_k_steckerbmk.htm

**Bedeutung für QET:** Beim Import von EPLAN-Makros/EDZ: Funktionsschablonen des Artikels sind die **Soll-Liste** der
Funktionen eines Geräts (Spule + n Kontakte mit Anschlussbezeichnungen). Der QET-Import sollte daraus (a) das
Hauptelement mit Artikeldaten und (b) die zulässigen Slave-Kontakte mit festen Anschlussbezeichnungen ableiten; beim
Verknüpfen Kontakt ↔ Schablone über die Anschlussbezeichnung zuordnen (wie EPLAN: gleiche Anschlussbezeichnung zuerst).
Artikeldaten nur am Master (`manufacturer`, `manufacturer_reference`). Grafik aus dem Makro, Daten aus dem Plan – nie
Plan-Daten durch leere Makro-Werte überschreiben.

## 11. Löschen, Kopieren, nicht-platzierte Funktionen, Gerätegruppen

- Löschen/Kopieren wirkt nur auf markierte Funktionen. Löschen einer platzierten Funktion löscht Platzierung und
  Funktion; Nebenfunktion eines Geräts bleibt als nicht-platzierte erhalten (Zusatzdaten weg). Löschen der Hauptfunktion
  löscht die Gerätedefinition. Nicht-platzierte Funktionen nur in Navigatoren oder per Komprimieren löschbar.
  „Platzierung löschen“ entfernt nur die Grafik. Kopieren einer Hauptfunktion kopiert Nebenfunktionen standardmäßig
  **nicht**; Makros enthalten standardmäßig keine nicht-platzierten Funktionen. Inkonsistenzen (zwei Hauptfunktionen mit
  gleichem BMK) werden gemeldet, Projekt bleibt bearbeitbar. Eigenschaft „Aus Makro nicht-platziert einfügen“.
  Quelle: devicelistgui_k_loeschenkopieren.htm
- **Nicht-platzierte Nebenfunktionen an der Hauptfunktion verwalten**: kopiert, benennt um und löscht die
  nicht-platzierten Nebenfunktionen mit der Hauptfunktion (wie Schablonen); erlaubt einpolig zu zeichnen und allpolige
  Daten im Hintergrund zu führen. Verhalten beim Abbuchen je nach Quelle/Ziel tabellarisch geregelt.
  Quelle: devicetaggui_k_verwaltungnichtplatziertefunktionen.htm
- **Gerätegruppe** (nur im Makroprojekt erstellbar): genau eine Hauptfunktion „Allpolig“ oder „RI-Fließbild“ + eine oder
  mehrere Funktionen „Paarquerverweis“ (auch mit anderem BMK/Gewerk) + beliebig Grafik. Im BM-Navigator stehen die
  Paarquerverweise unter der Hauptfunktion. Formatelement „Gerätegruppe (weitere Funktionen)“ greift auf Daten der
  anderen Gruppenmitglieder zu. Quelle: devicelistgui_k_geraetegruppierung.htm

## 12. Geräte- und Änderungsschutz

- **Geräteschutz (ID 20475)**: schützt v. a. die Artikel (keine Geräte-/Artikelauswahl, kein Austauschen); Funktionen
  lassen sich nicht löschen (Löschen im Plan entfernt nur die Platzierung), das BMK ändert sich bei grafischen Aktionen
  nicht. Andere Eigenschaften (Funktionstext) bleiben editierbar. Wird in Makros/Kopien an der Hauptfunktion mitgespeichert.
  Gerätegruppen-Mitglieder einzeln schützen. Quelle: devicetaggui_k_geschuetztebm.htm, devicetaggui_k_geschuetztebmverwendung.htm,
  devicetaggui_h_bmschuetzen.htm
- **Änderungsschutz (ID 3014, hierarchisch ID 3015)**: alles ausgegraut; automatisch bei Teilprojekt-Auslagerung,
  Vorplanungs-Seiten, revisionierten Bearbeitungsbereichen, CAD-Import (Pro Panel); manuell über „Schutz konfigurieren“;
  Gründe wirken additiv. Auch Strukturkennzeichen schützbar. Quelle: devicetaggui_k_aenderungsschutz.htm, devicetaggui_h_manuellschuetzen.htm

## 13. Geräteliste (Betriebsmittelliste der Vorplanung)

- Die **Geräteliste** (Projektdaten > Geräte/Artikel > Geräteliste) dient der **Vorplanung und Überwachung**: Artikel mit
  **erlaubter Stückzahl**, optional Anlagenteil und Gerätebeschreibung (nicht aus der Artikelverwaltung; ein Artikel darf
  unter verschiedenen Beschreibungen mehrfach stehen). Sie ist unabhängig von den Schaltplandaten, gehört aber zum
  Projekt; ihre Einträge zählen **nicht** in die Artikelstückliste (Projektartikel stehen im Stückliste-Navigator).
  Quelle: devicelistgui_k_geraeteliste.htm, devicelistgui_h_geraetelistebearbeiten.htm
- Spalten: Reserve = erlaubte Stückzahl (gesamt, Summe gleicher Artikelnummern) minus **verbaute Stückzahl** (gezählte
  Artikelreferenzen im Projekt); Reserve < 0 wird markiert und im Prüflauf gemeldet; Artikel im Projekt, aber nicht in der
  Liste, werden nicht gemeldet. Artikelnummer bis 50 Zeichen; Variante, ERP-, Typ-, Bestellnummer, Bezeichnung 1–3,
  Hersteller, Makro kommen aus der Artikelverwaltung; Gewerk/Untergewerk werden beim Platzieren an die Schaltzeichen
  übertragen. Quelle: devicelistgui_d_geraeteliste.htm, devicelistgui_d_eigenschaften.htm
- Modulartikel erscheinen nicht (nur der Modulartikel selbst); Baugruppen können aufgelöst erscheinen. Kein
  Artikelabgleich, keine Komprimierung für die Geräteliste. Aus der Liste lässt sich ein Gerät direkt platzieren
  („Gerät einfügen“); Strg+Umschalt+Ziehen auf ein Schaltzeichen fügt einen Artikel hinzu (ersetzen/ergänzen).
  Export/Import für Datenaustausch. Quelle: devicelistgui_k_geraeteliste.htm, devicelistgui_h_geraeteeinfuegen.htm
- „In Betriebsmittelliste unterdrücken“ an einer Funktion betrifft dagegen die **Auswertung** Betriebsmittelliste
  (Formular), nicht die Geräteliste. Quelle: devicetaggui_r_bmdaten.htm

**Bedeutung für QET:** Die Geräteliste ist kein Teil des gedruckten Plans; für den Nachbau irrelevant. Die gedruckte
Betriebsmittelliste/Stückliste im EPLAN-PDF ist eine Auswertung (siehe `auswertungen.md`) und bildet die **verbauten**
Artikel ab – sie ist die Referenz, welche Geräte und Artikel im QET-Nachbau existieren müssen.

## 14. Benutzerdefinierte Eigenschaften

- Eigene Eigenschaften mit eindeutigem, später **nicht änderbarem identifizierendem Namen**, mehrsprachigem
  Anzeigenamen, Beschreibung, **Zuordnung** (Projekt, Seite, Funktion, Segmentvorlage …; in der Artikelverwaltung nur
  „Artikel“, optional auch für Artikelreferenzen), Feldtyp, Eingabehilfe (frei / Auswahlliste / frei mit Liste / sich
  selbst erweiternde Liste) mit Vorgabewerten, Kategorie, Option „Eigenschaftswert übersetzen“. Projekt-Eigenschaften
  werden mit dem Projekt gespeichert und weitergegeben, Artikel-Eigenschaften in der Artikeldatenbank; per Artikel-/
  Geräteauswahl landen sie an der **Hauptfunktion** und erscheinen dann auch in der Projektkonfiguration; sie gehen in
  Artikelabgleich und EDZ-/XML-Export/-Import ein. Quelle: userdefproperties_k_start.htm, userdefproperties_k_prinzip.htm,
  userdefproperties_h_eigenschafterstellen.htm
- Löschen zweistufig: „Nicht mehr verwenden“, dann „Nicht mehr zu verwendende Eigenschaften entfernen“ (oder
  Komprimieren). Verwendete Projekt-Eigenschaften erst bei „Anzahl Verwendungen = 0“. Makros (und Formulare,
  Normblätter) bringen ihre benutzerdefinierten Eigenschaften mit; Projekteinstellung „Neue Eigenschaftskonfigurationen
  aus Makros nicht verwenden“ unterdrückt das. Quelle: userdefproperties_h_eigenschaftloeschen.htm
- Ältere Alternativen: feste **Zusatzfelder [n]** (bis 1000) – Projekt **ID 10901**, Seite **ID 11901**, Funktion
  **ID 20901**; in der Artikelverwaltung **freie Eigenschaften** (Beschreibung/Wert/Einheit [n], nur aus
  Kompatibilitätsgründen). Quelle: userdefproperties_k_zusatzfelder.htm

**Bedeutung für QET:** Beim EDZ-Import tauchen benutzerdefinierte Artikeleigenschaften und freie Eigenschaften auf. Sie
sollten in QET als zusätzliche Element-Informationen (eigene Schlüssel) mitgenommen werden, nicht verworfen;
Zusatzfelder ID 20901[n] entsprechend als `auxiliary`-Felder (Vermutung zur Zuordnung).

## 15. Tabellarische Bearbeitung (Funktionsdatentabelle)

- Bearbeiten > Tabellarisch: Funktionen, Verbindungen, Makros, Abbruchstellen, Potenziale, Rohrleitungen, Segmente
  seitenübergreifend in Tabellen (eine Registerkarte je Objekttyp, wenn gemischt), blockweise änderbar, per
  Kopieren/Einfügen mit Excel austauschbar. Spalten über **Schema** (benutzerbezogen, für Weitergabe exportieren).
  Quelle: functiondatagridgui_k_start.htm, functiondatagridgui_d_schematabellarischebearbeitung.htm,
  functiondatagridgui_h_tabellarischbearbeiten.htm
- Änderung in der Spalte „Name sichtbar“ durchläuft die BMK-Syntaxprüfung. Bei **Verbindungen** werden Werte der
  Verbindung und aller VDP auf ihr gemeinsam gezeigt (bei Abweichung der Wert des grafisch ersten VDP mit gefülltem
  Feld); eine Änderung geht an Verbindung und alle VDP, an denen das Feld gefüllt war (leere bleiben leer); ohne VDP
  setzt EPLAN **automatisch einen VDP**. Quelle: functiondatagridgui_d_tabellarischbearbeiten.htm,
  functiondatagridgui_h_tabellarischbearbeiten.htm
- **BMK-Aufteilung**: Standardspalten Teil-BMK 1, Teil-BMK 2, Präfix, Kennbuchstabe, Zähler, Unterzähler; alternativ
  frei benannte Spalten, gefüllt nach Zeichengruppenwechsel (Ziffern / Sonderzeichen `= + - _ / : & * ? , .` / andere),
  Rest in die letzte Spalte; nur der Block „BMK“ wird zerlegt. Beispiele: `1V2` → 1 | V | 2; geschachteltes `U1-F1`
  → U | 1 | - | F | 1 (Trenn-Minus ausgegraut). Felder übernommener BMK sind gesperrt. Quelle: functiondatagridgui_k_bmkaufteilung.htm

**Bedeutung für QET:** Die Zeichengruppen-Regel ist ein brauchbarer, EPLAN-konformer Parser für BMK im Import
(Kennbuchstabe = erste Buchstabengruppe, Zähler = folgende Ziffern; z. B. `K381` → K | 381, `2M3` → 2 | M | 3).

## 16. Daten abgleichen

- Funktionen können verteilt (einpolig, allpolig, Übersicht, Paarquerverweis, RI, Topologie) oder in einer
  Darstellungsart mehrfach (z. B. SPS-Kasten auf zwei Seiten) vorkommen; ohne übergreifenden Modus entstehen
  abweichende Werte. **Dienstprogramme > Abgleichen > Funktionen > „Darstellungsart → alle“** überträgt die Werte der
  markierten Funktionen an die anderen Darstellungen (Beispiel: einpolig 5,5 kW überschreibt allpolig 2,2 kW).
  Behandlung leerer Eigenschaften: Projekteinstellung Betriebsmittel > Allgemein, Gruppenfeld Abgleich. Analog für
  Verbindungen (allpolig, einpolig, Topologie, 3D). Quelle: adjustdata_k_start.htm, adjustdata_h_funktionabgleichen.htm,
  adjustdata_h_verbindungenabgleichen.htm
- **Hauptfunktionen korrigieren**: wandelt überzählige Hauptfunktionen (und Hauptklemmen derselben Klemme in mehreren
  Darstellungen) projektweit in Nebenfunktionen; gewählt wird, welche Darstellungsart Hauptfunktion bleibt.
  **Hauptfunktion zuweisen** / **Hauptklemme zuweisen**: markierte Nebenfunktion wird Hauptfunktion, die alte wird
  Nebenfunktion; Artikeldaten wandern mit. Quelle: adjustdata_h_funktionkorrigieren.htm, adjustdata_h_nebenfunktionaendern.htm

**Bedeutung für QET:** Im Nachbau gilt dieselbe Regel: **ein Master je Gerät**. Erscheint ein Gerät einpolig und
allpolig, nur eine Darstellung (Vermutung: meist die allpolige) als QET-Master mit Artikel; die andere als Slave/Verweis.
Abweichende Daten zwischen Darstellungen im PDF als Hinweis auf fehlenden Abgleich im Original melden, nicht
stillschweigend vereinheitlichen.

## 17. Navigatoren (Betriebsmittel-Navigator)

- Logische Sicht: filtern, Ansichten, erkennen ob platziert, Funktionen anlegen/bearbeiten/löschen/platzieren. Baum nach
  Strukturkennzeichen der Betriebsmittelstruktur; Ebenen „Ohne Strukturkennzeichen“ und darunter „Ohne BMK“; Icons
  zeigen Zustand und Darstellungsart; abhängige Funktionen werden fett hervorgehoben. Quelle: navigatorgui_k_start.htm,
  navigatorgui_d_navigator.htm
- **Neu**: nicht-platzierte Funktion über Funktionsdefinition (neues BMK = neues Betriebsmittel, gleiches = zum markierten).
  **Neue Funktionen** mit Nummerierungsmuster (Klemmen, Steckerkontakte, Spulenkontakte, Geräte-/SPS-Anschlüsse,
  Sammelschienen): `1a,1b,2a-8b` erzeugt 16 Funktionen. **Zuweisen**: BMK (alle Funktionen nacheinander), nicht-platzierte
  Funktion (ersetzt die des Schaltzeichens), platzierte (Daten kopieren), Funktionsschablone (überlagern); die
  zugewiesene Funktion braucht mindestens so viele Anschlüsse wie das Schaltzeichen und dieselbe Funktionsgruppe (sonst
  Meldung); Mehrfachzuweisung per Rahmen: Navigator-Reihenfolge ↔ grafische Reihenfolge. Platzieren aus dem Navigator:
  Darstellungsart aus dem Seitentyp, erstes passendes oder das zugeordnete Symbol; Platzierung löschen nicht bei
  Extern/Grafik/Schaltschrankaufbau. Mechanische Betriebsmittel verhalten sich wie Gerätegruppen.
  Quelle: navigatorgui_h_funktionenanlegen.htm, navigatorgui_h_mehrerfunktionerzeugen.htm,
  navigatorgui_k_zuweisenvonfunktionsdaten.htm, navigatorgui_h_funktionzuweisen.htm, navigatorgui_h_grafischevorschau.htm,
  devicelistgui_h_geraeteplatzieren.htm, navigatorgui_k_mechbetriebsmittel.htm
- Weitere Navigatoren und Seiten: `navigatoren-seiten.md`.

## 18. Bedeutung für QET (Zusammenfassung)

| EPLAN | QET-Nachbau / Import |
|---|---|
| Betriebsmittel = Funktionen mit gleichem vollständigem BMK | Master + Slaves mit gleichem `=Anlage+Ort-BMK` |
| genau eine Hauptfunktion trägt Artikel | Artikel nur am Master; Fehler melden, wenn zwei Master |
| Funktion ohne sichtbares BMK | BMK-Übernahme nachbilden: links auf gleicher Y (Einfügepunkt), sonst umgebender Geräte-/SPS-Kasten |
| Anschlussbezeichnungen identifizierend | Terminalnamen exakt wie im Original; Kontakt ↔ Funktionsschablone über Anschlussbezeichnung |
| grafische/externe Funktionen | zeichnen, aber nicht auswerten, keine Drahtnummer, kein Querverweis |
| Eigenschaftsanordnung | Texte als Elementtexte, nicht als freie Folio-Texte |
| Funktionstext (automatisch) ← Pfad-Funktionstext | Pfadtext als Rückfall für Funktionstext merken |
| Funktionsschablonen am Artikel | Soll-Liste der Gerätefunktionen beim EDZ-/Makro-Import |
| Platzieren: Plan-Daten schlagen Makro-Daten, Grafik aus Makro | Import nie Planwerte durch leere Makrowerte überschreiben |
| benutzerdefinierte/freie Eigenschaften, Zusatzfelder | als zusätzliche Element-Infos übernehmen |

## 19. Ergänzung: Dialoge BMK-Auswahl, Eigenschaftsauswahl, Symboldaten
**BMK-Auswahl** (Schaltfläche [...] neben „Sichtbares BMK“): Baum und Liste zeigen die Funktionen wie die Projektdaten-Navigatoren.
- Markierter BMK-Knoten (z. B. „F4“) → nur das BMK wird der aktuellen Funktion zugewiesen, **keine Daten** übertragen; [Nächste] liefert das nächste freie BMK mit demselben Kennbuchstaben.
- Markierte Funktion oder Funktionsschablone → **Abbuchen**: deren Daten gehen an die aktuelle Funktion. Erlaubt nur, wenn die aktuelle Funktion mindestens so viele Anschlüsse hat wie die markierte (an einem Schließer lässt sich ein Öffner oder Wechsler abbuchen); sonst bleibt [OK] gesperrt.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_bmkauswahl.htm

**Eigenschaftsauswahl**: wählt Eigenschaften, denen man Werte zuweist (nur schreibbare) oder die in Bäumen/Listen angezeigt werden; das Angebot hängt vom Aufrufort ab. Der Filter durchsucht nur Eigenschaftsnamen, nicht Werte, ohne UND/ODER und ohne Platzhalter `*`/`?`.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_eigenschaftsauswahl.htm

**Eigenschaftsauswahl (für Anzeige)** – Registerkarte Anzeige, Symboleditor „Platzierte Eigenschaften“, Sondertexte, Eigenschaftenübersicht. Bei mehreren möglichen Quellen wählt man ein **Quellobjekt**:
- *Funktion* (Nebenfunktion/Funktionsschablone selbst), *Hauptfunktion* (von einer Nebenfunktion aus oder von einem Verbindungsdefinitionspunkt, dessen Verbindungen zu einem Kabel/Paket gehören), *Artikelreferenz* (am BM änderbare und berechnete Artikeleigenschaften) und *Artikel* – Index = Reihenfolge in der Registerkarte Artikel.
- Vorplanung: *Segment* (Index läuft im Vorplanungsbaum von unten nach oben), *Struktursegment hierarchisch* (gezählt vom obersten Knoten abwärts), *PLT-Stelle*, *PLT-Stellenfunktion*, *Segment: Artikel(-referenz)*, *Platzierung*.
- Verbindungen/Rohre: *Verbindungsdefinitionspunkt*, *Verbindung*, *Rohrleitungsdefinition*, *Rohrklasse*, *Rohrleitungs-Planungsobjekt*.
- Sondertexte: *Seite* und *<Kennzeichnungsblock>* (bei Strukturierung über Kennzeichnungsblöcke).
- Kategorien grenzen ein: „Betriebsmittel“ (BMK-Teile, Strukturkennzeichen), „Daten“ (Funktionstexte, technische Kenngrößen, Zusatzfelder, Blockeigenschaften), „Einstellungen“ (z. B. „In Betriebsmittelliste unterdrücken“, Querverweisdarstellung).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_eigenschaftsauswahlanzeige.htm

**Registerkarte Symboldaten**: erscheint bei Objekten ohne Funktionsdaten (Abbruchstelle, Strukturkasten, Platzhalterobjekt, Vorplanungssegment); dort lässt sich Symbol bzw. Symbolvariante über die Symbolauswahl tauschen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_symboldaten.htm

**Bedeutung für QET**
- Nebenfunktionen können Eigenschaften der **Hauptfunktion** (und deren Artikel) anzeigen – im Fork heißt das: Slave-Texte dürfen Infos des Masters (z. B. Artikelnummer, Funktionstext) per Formel übernehmen; beim Nachbau eines EPLAN-PDFs sind solche Texte am Kontakt also kein eigener Wert des Kontakts.
- Abbuch-Regel „Anschlüsse neu ≥ Anschlüsse alt“ als Plausibilitätsprüfung beim Import von Kontakten an Hauptfunktionen verwenden.

## 20. Geräte verschalten (nicht-platzierte Verbindungen)

**Zwei Aufrufe, zwei Wirkungen**
- **Dialog** über Projektdaten > Betriebsmittel > Geräte verschalten: Man wählt frei eine **Betriebsmittel-Quelle** und ein **Betriebsmittel-Ziel** und erzeugt **nicht-platzierte Verbindungen** zwischen deren Anschlüssen – auch zwischen Betriebsmitteln, die noch auf keiner Seite stehen. Dabei können auch noch nicht platzierte **Kabel** entstehen (neues Kabel-BMK eintippen, Artikel über Kabeltyp […]); sie erscheinen im Kabel-Navigator. Ein Kabel ohne Artikel nimmt beliebig viele Kabelverbindungen auf.
- **Registerkarte** „Geräte verschalten“ am **Verbindungsdefinitionspunkt in einpoliger Darstellung** (Eigenschaften, Darstellungsart „Einpolig“): definiert zu einer einpoligen Verbindung die **allpoligen** Einzelverbindungen. Quelle, Ziel und Kabel sind durch die einpolige Verbindung vorgegeben und nicht frei wählbar; es werden nur Anschlüsse **nicht-platzierter allpoliger Funktionen** angeboten; neue Kabel lassen sich hier nicht anlegen (sonst entstünden nicht-platzierte Funktionen); bei Klemmenleisten werden nur die mit der einpoligen Verbindung verschalteten Seiten (intern/extern) angeboten.

**Wichtige Elemente und Regeln**
- Quell-/Zieltabelle listen alle Anschlüsse des Betriebsmittels, vorhandene Verbindungen stehen in der Spalte „Verbindungsfarbe / -nummer“; an belegten Anschlüssen kann eine **weitere** Verbindung angelegt werden. Ist in einem Navigator ein Betriebsmittel mit Anschlüssen bzw. ein Kabel markiert, wird es automatisch eingetragen. Klemmenleisten: Auswahl intern / extern / beide.
- **Statistik:** freie Kabelverbindungen, freie PE/PEN-Adern, freie Abschirmungen.
- **[Verbindungen erzeugen]** verbindet die markierten Quell- mit den markierten Zielanschlüssen der Reihe nach; bei ungleicher Anzahl nur bis zur kleineren Menge, und nur so viele, wie das Kabel noch freie Kabelverbindungen hat. Mit „Kabel bearbeiten“ öffnet sich danach direkt der Kabeldialog.
- Kontextmenü: **Kabelverbindungen tauschen** (zwei Adern eines Kabels tauschen, Quelle/Ziel bleiben), **Drahtbrücke erzeugen** (zwischen zwei markierten Anschlüssen, als eigene Zeile), **Isolierung hinzufügen** (Verbindung mit isoliertem Ende; im Eigenschaftendialog das BMK angeben, an dem sie endet), Anschlusslogik, Eigenschaften bzw. Eigenschaften (übergreifend) der Verbindung, Verbindungen löschen. Reihenfolge der Einträge über Verschiebe-/Tausch-Schaltflächen.
- **Stegbrücken berücksichtigen:** zeigt Anschlüsse der Anschlussart „Stegbrücke“ und automatisch erzeugter Stegbrücken. Da das Logikmodell „Klemme, allgemein“ **vier Anschlüsse** hat (zwei davon für Stegbrücken), erscheinen Klemmen dann mehrfach – Spalte „Anschlusslogik: Anschlussart“ einblenden.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesconnectdevicegui_d_geraeteverschalten.htm

**Bedeutung für QET**
- In EPLAN können Verbindungen (und ganze Kabel) **existieren, ohne gezeichnet zu sein**; sie erscheinen in Verbindungs-, Kabel- und Klemmenplänen sowie in der Fertigungsausgabe, aber auf keiner Schaltplanseite. Weicht eine Auswertungsseite (Kabelplan, Klemmenplan) des Originals vom Schaltplan ab – Ader oder Brücke ohne sichtbare Linie –, ist das wahrscheinlich eine nicht-platzierte Verbindung, kein Zeichenfehler. QET kennt nur gezeichnete Leiter: solche Verbindungen im Nachbau als Hinweis/Report bzw. in der Klemmen-/Kabelliste (Element-Info) führen und **nicht** als zusätzliche Leiter in den Schaltplan erfinden.
- Einpolige Verbindung mit allpoligem Inhalt: Zeigt das Original eine einpolige Leitung (Schrägstriche mit Polzahl), so hängen die Einzeladern in EPLAN an deren Verbindungsdefinitionspunkt. Im QET-Nachbau die einpolige Darstellung zeichnen und die Aderzuordnung als Leitertext/Kabelinfo übernehmen; die Polzahl muss zur Zahl der hinterlegten Adern passen (Vermutung: EPLAN prüft das nicht automatisch).
- Klemmen mit vier Anschlüssen (zwei für Stegbrücken): Beim Import von Klemmenartikeln/-makros aus EPLAN die beiden Brückenanschlüsse nicht als normale Leiteranschlüsse in das QET-Element übernehmen, sonst entstehen doppelte Klemmenzeilen.

## 21. Quellen

Alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/

- Betriebsmittel-Konzepte: devicetaggui_k_start.htm, devicetaggui_k_bmeigenschaften.htm, devicetaggui_k_aufbauvonbm.htm,
  devicetaggui_k_bmkuebernahme.htm, devicetaggui_k_bmkuebernahmesonder.htm, devicetaggui_k_anschlussbezuebergabe.htm,
  devicetaggui_k_grafischeschaltzeichen.htm, devicetaggui_k_verhaltengrafischerschaltzeichen.htm,
  devicetaggui_k_verwaltungnichtplatziertefunktionen.htm, devicetaggui_k_geschuetztebm.htm,
  devicetaggui_k_geschuetztebmverwendung.htm, devicetaggui_k_aenderungsschutz.htm
- Handlungen: devicetaggui_h_anschlusbezuebernehmen.htm, devicetaggui_h_artikeldatenzuweisen.htm,
  devicetaggui_h_bmeigenschaftenbearbeiten.htm, devicetaggui_h_bmkaendern.htm, devicetaggui_h_bmkfestlegenundformatieren.htm,
  devicetaggui_h_bmkuebernahmefestlegen.htm, devicetaggui_h_bmschuetzen.htm, devicetaggui_h_bmtauschen.htm,
  devicetaggui_h_eigschanordnungen.htm, devicetaggui_h_manuellschuetzen.htm
- Dialoge/Registerkarten: devicetaggui_d_vollstaendigesbmk.htm, devicetaggui_d_eigschanordnungspeichern.htm,
  devicetaggui_d_eigschanordnungexportieren.htm, devicetaggui_r_bmdaten.htm, devicetaggui_r_symbolfunktionsdaten.htm,
  devicetaggui_r_anzeige.htm, devicetaggui_r_anzeigeeigenschaften.htm, devicetaggui_r_anzeigekontaktspiegel.htm,
  devicetaggui_r_format.htm, devicetaggui_r_klemmen.htm, devicetaggui_r_klemmenleisten.htm,
  devicetaggui_r_steckerdefinition.htm, devicetaggui_r_steckerkontakt.htm, devicetaggui_r_verbindungenklemmen.htm,
  devicetaggui_r_verbindung.htm, devicetaggui_r_vdp.htm, devicetaggui_r_abbruchstelle.htm, devicetaggui_r_locationbox.htm,
  devicetaggui_r_spsanschluss.htm, devicetaggui_r_spskarte.htm, devicetaggui_r_spsstruktur.htm, devicetaggui_r_busdaten.htm,
  devicetaggui_r_kabeldaten.htm, devicetaggui_r_ndp.htm, devicetaggui_r_pdp.htm, devicetaggui_r_netzverbindung.htm,
  devicetaggui_r_anschlussbild.htm, devicetaggui_r_blackboxdaten.htm, devicetaggui_r_paket.htm,
  devicetaggui_r_verbindungsgrafik.htm, devicetaggui_r_loeschzeichen.htm, devicetaggui_r_sammelschiene.htm
- Nur überflogen (3D, Topologie, Fluid, Schaltschrankaufbau): devicetaggui_r_adp.htm, devicetaggui_r_kupferstrang.htm,
  devicetaggui_r_kabelbaum.htm, devicetaggui_r_montageplatte.htm, devicetaggui_r_artikelplatzierung.htm,
  devicetaggui_r_artikelplatzierung3d.htm, devicetaggui_r_abmessung3d.htm, devicetaggui_r_format3d.htm,
  devicetaggui_r_cablingstrecke.htm, devicetaggui_r_cablinganschlussbild.htm, devicetaggui_r_cablingstrukturen.htm,
  devicetaggui_r_stranganschluss.htm, devicetaggui_r_kurve.htm, devicetaggui_r_biegestellen.htm, devicetaggui_r_rohrleitungsdp.htm
- Geräte/Geräteliste: devicelistgui_k_start.htm, devicelistgui_k_prinzip.htm, devicelistgui_k_geraeteliste.htm,
  devicelistgui_k_platzieren.htm, devicelistgui_k_vorgehengeraeteeinfuegen.htm, devicelistgui_k_vorgehengeraeteplatzieren.htm,
  devicelistgui_k_loeschenkopieren.htm, devicelistgui_k_geraetegruppierung.htm, devicelistgui_k_steckerbmk.htm,
  devicelistgui_h_geraetedefinitionenanlegen.htm, devicelistgui_h_geraeteeinfuegen.htm, devicelistgui_h_geraetelistebearbeiten.htm,
  devicelistgui_h_geraeteplatzieren.htm, devicelistgui_d_geraeteliste.htm, devicelistgui_d_eigenschaften.htm
- Benutzerdefinierte Eigenschaften: userdefproperties_k_start.htm, userdefproperties_k_prinzip.htm,
  userdefproperties_k_zusatzfelder.htm, userdefproperties_h_eigenschafterstellen.htm, userdefproperties_h_eigenschaftloeschen.htm
- Tabellarische Bearbeitung: functiondatagridgui_k_start.htm, functiondatagridgui_k_bmkaufteilung.htm,
  functiondatagridgui_h_tabellarischbearbeiten.htm, functiondatagridgui_d_tabellarischbearbeiten.htm,
  functiondatagridgui_d_schematabellarischebearbeitung.htm
- Abgleich: adjustdata_k_start.htm, adjustdata_h_funktionabgleichen.htm, adjustdata_h_funktionkorrigieren.htm,
  adjustdata_h_nebenfunktionaendern.htm, adjustdata_h_verbindungenabgleichen.htm
- Navigator: navigatorgui_k_start.htm, navigatorgui_k_zuweisenvonfunktionsdaten.htm, navigatorgui_k_mechbetriebsmittel.htm,
  navigatorgui_h_funktionenanlegen.htm, navigatorgui_h_funktionzuweisen.htm, navigatorgui_h_grafischevorschau.htm,
  navigatorgui_h_mehrerfunktionerzeugen.htm, navigatorgui_d_navigator.htm

Zusatz-Dialogseiten (Ergänzung Abschnitt 19):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_bmkauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_eigenschaftsauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/modaldialogsdb_d_eigenschaftsauswahlanzeige.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_symboldaten.htm

Geräte verschalten (Abschnitt 20):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xesconnectdevicegui_d_geraeteverschalten.htm
