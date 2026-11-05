# EPLAN-Wissen: Übersetzung, mehrsprachige Texte, Wörterbuch

Zusammenfassung der EPLAN-Plattform-Hilfe 2.9 zur Übersetzung (Projekt- und Artikeltexte), zu mehrsprachigen
Feldern und zum Wörterbuch, in eigenen Worten. Zweck: mehrsprachige EPLAN-PDFs (übereinanderstehende Texte in
mehreren Sprachen) in QElectroTech richtig deuten und nachbauen, und wissen, welche Texte überhaupt übersetzt werden.
„Vermutung“ kennzeichnet eigene Schlüsse.

## Inhalt
1. Grundbegriffe: sprachneutral, sprachabhängig, Quell-, Übersetzungs-, Anzeigesprache
2. Automatische und manuelle Übersetzung
3. Projekteinstellungen Übersetzung (Allgemein): Segment, Groß-/Kleinschreibung, Zeilenumbrüche, Fehlende Übersetzung
4. Umfang: welche Texte und Seitentypen übersetzt werden
5. Wörterbuch: Stichwörter, Sätze, Platzhalter, Zahlen, Sonderzeichen, Trennvorschlag
6. Wörterbuch-Einstellungen (Benutzer) und Wörterbuchpflege
7. Fehlwortliste, Import/Export, Übersetzung entfernen
8. Auswirkungen auf PDF, Auswertungen, SPS-Austausch
9. Bedeutung für QET (Zusammenfassung)
10. Quellen

---

## 1. Grundbegriffe: sprachneutral, sprachabhängig, Quell-, Übersetzungs-, Anzeigesprache

- Viele Eigenschaften (Projekt, Seite, Betriebsmittel, Artikel) und freie Texte sind **mehrsprachig**. Ein Icon vor
  dem Feld (Benutzereinstellung: alle / nur bearbeitbare / keine) zeigt, ob der Inhalt **sprachneutral** (keiner
  Sprache zugeordnet oder leer) oder **sprachabhängig** ist. Felder ohne Icon (bei „immer anzeigen“) sind nicht
  mehrsprachig.
- Neu eingegebener Text ist sprachneutral und erscheint unabhängig von der Sprache so, wie eingegeben. Erst eine
  Übersetzung (automatisch, Dialog „Mehrsprachige Eingabe“ mit [OK] – auch ohne Änderung –, Kontextmenü Übersetzen)
  ordnet ihn der Quellsprache zu. Zurück: „Übersetzung entfernen“. Ausschließen: „Nicht zu übersetzenden Text
  kennzeichnen“ (Darstellung im Dialog als `{{+A}}`).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_k_mehrsprachigetexte.htm
- **Übersetzungssprachen:** in diese wird übersetzt; die Texte werden **im Projekt eingelagert**.
  **Quellsprache:** Eingabesprache (Statusleiste), Basis der Wörterbuchsuche, Sprache der Projekttexte in Dialogen
  (z. B. Seitenbeschreibung im Seiten-Navigator); wählbar „##_## (Dialogsprache)“ = folgt der Oberflächensprache.
  **Anzeigesprachen:** welche Sprachen in **welcher Reihenfolge** im Grafischen Editor sichtbar sind (bis 5); die
  Quellsprache muss nicht dabei sein. Beispiel der Hilfe: Quelle en_US, Anzeige zh_CN + ru_RU → Plan zeigt nur
  Chinesisch und Russisch, Dialoge Englisch.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungprojekt.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_sprachenwaehlen.htm
- Einzelne Texte lassen sich auf **eine** Sprache festlegen: Registerkarte Format, Eigenschaft Format > Sprache =
  **„Einsprachig (variabel)“**; welche Sprache das ist, regelt die Projekteinstellung „Einsprachig (variabel)“
  (feste Übersetzungssprache, Dialogsprache oder Anzeigesprache 1 … 5).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungprojekt.htm

## 2. Automatische und manuelle Übersetzung

- **Automatisch:** Übersetzungslauf (Dienstprogramme > Übersetzung > Übersetzen) für Projekt oder Auswahl; alle
  mehrsprachigen Eigenschaften und freien Texte im Umfang werden übersetzt, sprachneutrale werden der Quellsprache
  zugeordnet. Oder **„Bei der Eingabe übersetzen“**: Texte, Seiten-, Projekt- und Betriebsmitteleigenschaften werden
  sofort übersetzt; ohne Wörterbuchtreffer bleibt der Text sprachneutral. Seiteneigenschaften werden im Lauf nur
  übersetzt, wenn die Seite markiert ist, Projekteigenschaften nur bei markiertem Projekt.
- **Manuell:** bei der Eingabe einen Übersetzungstext aus dem Wörterbuch wählen oder direkt eingeben. Bei
  Mehrfachbedeutungen (gleiches Stichwort, verschiedene Übersetzungen, unterschieden per Kommentar) zeigt der
  Dialog **Gefundene Wörter** Vorschläge mit Kontext (Texttyp/Funktionskategorie, BMK, Eigenschaft) und Wörter, die
  mit dem Eingegebenen beginnen.
- Auch Artikeldaten der Artikelverwaltung sind übersetzbar (eigene, benutzerbezogene Auswahl der Felder).
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_k_start.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_onlineuebersetzen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_d_gefundenewoerter.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_artikelverwaltung.htm
- **Testeingabe** (Dienstprogramme > Übersetzung > Testeingabe): Text probeweise in alle Übersetzungssprachen
  übersetzen; bei fehlendem Ergebnis Wörterbuch, Wörterbuchsprache, Stichwort und Übersetzungstexte prüfen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_d_testeingabe.htm

## 3. Projekteinstellungen Übersetzung (Allgemein)

Optionen > Einstellungen > Projekte > … > Übersetzung > Allgemein (projektbezogen):
- **Groß-/Kleinschreibung beachten** (nur automatisch): an → Stichwort muss exakt passen; aus → mehr Treffer.
- **Bereits übersetzte Texte verändern** (nur automatisch): an → neu übersetzen und überschreiben; aus → bleibt.
  Manuell wird immer überschrieben.
- **Manuelle Auswahl bei Mehrfachbedeutungen**: an → Vorschläge; aus → vorhandene Übersetzung, sonst **erstes
  Stichwort** der Liste. Tipp der Hilfe: Pseudosprache **yy_XX** als Quellsprache für eindeutige Übersetzungs-IDs.
- **Segment** (Übersetzungseinheit und Fehlwortausgabe): Satzende = `.`, `!`, `?` + Leerzeichen/Umbruch.
  - **Wort:** erst Satz, dann jedes Wort; fehlt **ein** Wort, bleibt der ganze Satz unübersetzt. Zusätzliche
    Worttrennzeichen `-` oder `/` trennen Komposita (Not-Aus → Not, -, Aus).
  - **Satz:** nur satzweise; ohne Treffer keine Übersetzung (Kontext bleibt erhalten).
  - **Gesamter Eintrag:** ganzes Textfeld als Einheit, Punkte im Text sind kein Satzende (gut für Abkürzungen);
    der Punkt darf nicht am Stichwortende stehen.
- **Zeilenumbrüche: Textorientiert** (nur bei Wort): an → Umbruch an der Position des Quelltexts; aus (Standard) →
  EPLAN setzt den Umbruch im Übersetzungstext nach Wortlänge an Leerzeichen, Bindestrich oder Trennvorschlag.
- **Groß-/Kleinschreibung der Ausgabe:** „Übersetzung …“ (nur Übersetzungen) oder „Alles …“ (Quelle + Übersetzung):
  wie im Wörterbuch, GROSS (zur Unterscheidung der Sprachen), klein, Anfang groß (bei Gesamter Eintrag nur erster
  Buchstabe, sonst jeder Satz).
- **Fehlende Übersetzung: Anzeigen + Text**: Hinweistext überall im Grafischen Editor, wo nicht übersetzt werden
  konnte; ohne Text erscheint ein **Zeilenumbruchzeichen** als Hinweis.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungprojekt.htm

## 4. Umfang: welche Texte und Seitentypen übersetzt werden

- Übersetzung > **Umfang** (projektbezogen): Bereiche Bauteile, Formular und Normblatt, Projekt, Seite, Symbol,
  Symbolbibliothek, Vorplanung; je Bereich Häkchen pro **beschreibender Eigenschaft**. Dazu **Freie Texte**,
  **Pfad-Funktionstexte**, **Hyperlink Text/Bezeichnung**. „Beliebige Eingabe für nicht zu übersetzende Texte
  erlauben“ hebelt eine Rechteverwaltungs-Sperre (nur Wörterbuchtexte) für ausgeschlossene Texte aus.
  **[Korrektur]** wandelt ausgeschlossene Texte mit Übersetzung in sprachneutrale um (Ja = alle, Nein = nur leere
  Übersetzungen, typisch aus Projekten vor Version 2.0).
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungumfang.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_textenichtuebersetzen.htm
- **Übersetzbare Seiten:** Häkchen je Seitentyp; nicht angehakte Seitentypen werden nie übersetzt.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungseiten.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_seitennichtuebersetzen.htm
- Für Schriftzeichen außerhalb der Standardschriften eine passende Schrift installieren und unter Einstellungen >
  Firma > Grafische Bearbeitung > Schriftarten wählen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_schriftartfestlegen.htm

**Bedeutung für QET:** Steht im PDF ein Text nur einsprachig, während andere zweisprachig sind, ist er entweder
sprachneutral, ausgeschlossen, „Einsprachig (variabel)“ oder liegt auf einem nicht übersetzten Seitentyp – im
Nachbau genauso einsprachig lassen.

## 5. Wörterbuch: Stichwörter, Sätze, Platzhalter, Zahlen, Sonderzeichen, Trennvorschlag

- Mehrere Wörterbücher (Access *.mdb oder SQL-Server), je mehrere Sprachen, Unicode, Sonderzeichen wie ©,
  Liste „Wörter nicht übersetzen“ (gilt nur bei wortweiser Übersetzung), AutoKorrektur-Liste.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_k_start.htm
- **Stichwort** = Einzelwort, Satz (≥ 2 Wörter) oder Eintrag mit **Platzhaltern %0 … %9** (max. 10). Ein Einzelwort
  kann in der Zielsprache ein Satz sein und umgekehrt. Platzhalter füllen sich beim Übersetzen mit Zahlen
  (`Terminal with %0 connectors`) oder mit als „nicht zu übersetzen“ markierten Teilen (`Emergency-stop switch %0`
  → `… +A`, `… +B`), dann segmentunabhängig.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_k_eingabemoeglichkeit.htm
- **Zahlen:** Import von Projekt-/Artikeltexten und Fehlwortexport ersetzen Ziffern durch Platzhalter (über
  10 Ziffern im Segment: bleiben); Wörterbuch-Import/-Export behält Ziffern. Beim Übersetzen zuerst exakter Treffer
  mit Ziffern, dann Platzhalter-Stichwort.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_k_zahlen.htm
- **Reservierte Zeichen:** `%` Platzhalter, `|` Trennvorschlag (Alt Gr + |, z. B. `Tacho|meter`; ein Zeilenumbruch
  im Projekttext bricht die Übersetzung dort um), `.`/`!`/`?` am Ende erlaubt (Einstellung „Satzendezeichen
  entfernen“), innerhalb nur mit Segment „Gesamter Eintrag“; **¶** (Umbruch im Grafischen Editor) und **TAB**
  (Exporttrenner) sind verboten.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_k_ueberblickzeichen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_h_trennvorschlag.htm
- Stichwörter sind **groß-/kleinschreibungsgenau**; jedes hat eine Quellsprache und muss dort einen Text haben;
  Kommentar unterscheidet Mehrfachbedeutungen (Beispiel „Connector“ als Kommentar vs. Funktionsdefinition).
  Volltextfilter ohne Platzhalter `*`, `?`, `_`, ohne UND/ODER, ohne Groß-/Klein-Unterschied.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_h_stichwortanlegen.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_r_uebersetzbar.htm
- **AutoKorrektur:** unerwünschte Wörter der **Quellsprache** werden vor dem Übersetzen durch Ersetzungstexte
  getauscht (einheitliche Schreibweise); benutzerbezogen einschalten.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_r_gesperrtebegriffe.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_texteersetzen.htm
- Wörter nicht übersetzen: Wörter, die in allen Sprachen gleich bleiben (nur bei Segment Wort wirksam).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_r_nichtuebersetzbar.htm

## 6. Wörterbuch-Einstellungen (Benutzer) und Wörterbuchpflege

Optionen > Einstellungen > Benutzer > Übersetzung > Wörterbuch (benutzerbezogen):
- Wörterbuch wählen (Access/SQL, 15 zuletzt genutzte), Wörterbuchsprachen.
- **Satzendezeichen entfernen:** an → Suche des Stichworts ohne Endzeichen (und Aufnahme ohne Endzeichen beim
  Import/Fehlwortexport); aus → mit Endzeichen. Andere Treffer, wenn beide Varianten existieren (Beispiel der Hilfe).
- **Zahlen als Text behandeln:** reine Ziffernfolgen (z. B. 111111) werden übersetzt bzw. als Fehlwort behandelt;
  sonst unverändert. Gilt nicht für Mischungen wie A1234.
- **Dezimalzahlen erkennen:** Ziffern mit `,` `.` `+` `-` `/` ohne Leerzeichen zählen als **eine** Zahl → ein
  Stichwort `Voltage %0V` deckt 12V, 2,5V, +-2.5V ab; aus → je Variante eigene Stichwörter (`Voltage %0,%1V` …).
- Import: **Neue Übersetzungen anhängen**; bestehende **überschreiben** (alle Sprachen!), nicht überschreiben,
  abfragen, **Doppelte zulassen** (neuer Eintrag bei abweichender Übersetzung).
- **AutoVervollständigen** beim Tippen; **AutoKorrektur** in der ersten Anzeigesprache.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungbenutzer.htm

Wörterbuchpflege (Dienstprogramme > Übersetzung > Wörterbuch bearbeiten, Registerkarte Verwaltung):
- Neues Wörterbuch (Neu neben Access/SQL), Sprachen hinzufügen; [Extras] Import/Export als **XML *.etd** oder
  **tabgetrenntes Unicode *.txt**, Export Fehlwortliste (Quellsprache = Projekt-Quellsprache), Komprimieren.
- **Projekttexte importieren** / **Artikeltexte importieren**: alle übersetzbaren Texte der gewählten Sprachen
  werden Stichwörter, immer als **gesamter Eintrag** (auch bei Segment Wort), inkl. „nicht zu übersetzen“-Teilen;
  Sprachen müssen Wörterbuchsprachen sein, sonst „Sprache nicht vorhanden“.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_r_verwaltung.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_h_woerterbuchanlegen.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_h_projekttexteimportieren.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_h_artikeltexteimportieren.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_d_sprachewaehlen.htm

## 7. Fehlwortliste, Import/Export, Übersetzung entfernen

- **Fehlwortliste exportieren** (Projekt, Seiten oder markierte Elemente): nicht übersetzbare Texte je Segment
  (Wort/Satz/Gesamter Eintrag) für eine oder mehrere Zielsprachen; Format *.etd (EPLAN-intern) oder
  tabgetrenntes Unicode-TXT (Excel). Beispiel der Hilfe: ein Eintrag ergibt bei „Wort“ acht, bei „Satz“ drei, bei
  „Gesamter Eintrag“ ein Segment.
- **Fehlwortliste importieren** über Wörterbuch bearbeiten > [Extras] > Import; Quellsprache wählen; die
  nachübersetzten Texte landen im Wörterbuch, danach erneut übersetzen.
- **Übersetzung aus Projekt entfernen:** löscht Übersetzungstexte gewählter Sprachen (auch nicht angezeigte);
  diese Sprachen sind danach keine Übersetzungssprachen mehr.
Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_fehlwortexport.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_fehlwortimport.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_entfernenuebersetzung.htm ·
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungprojekt.htm

## 8. Auswirkungen auf PDF, Auswertungen, SPS-Austausch

- **Grafik/PDF:** Der Grafische Editor zeigt die **Anzeigesprachen** in festgelegter Reihenfolge; das PDF nimmt
  standardmäßig die Projekt-Anzeigesprachen oder eigene „Alternative Sprachen“ (Details in `pdf-export.md`,
  Abschnitt Ausgabesprachen). Fehlt eine Übersetzung und ist „Fehlende Übersetzung anzeigen“ aktiv, steht dort der
  Hinweistext bzw. ein Umbruchzeichen.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungprojekt.htm ·
  pdf-export.md (dieser Skill)
- **Auswertungen/Formulare:** Formular- und Normblatttexte sind ein eigener Übersetzungsbereich im Umfang
  (Abschnitt 4); Auswertungsseiten sind Seitentypen, die in „Übersetzbare Seiten“ ein- oder ausgeschlossen werden.
  Vermutung: Ausgegebene mehrsprachige Eigenschaften (z. B. Funktionstext in einer Stückliste) folgen den
  Anzeigesprachen bzw. den Sprachplatzhaltern im Formular – die ausgewerteten Hilfeseiten sagen dazu nichts.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungumfang.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungseiten.htm
- **Zuordnungslisten (SPS):** Import/Export in einer gewählten Sprache oder „Alle Anzeigesprachen“; je Sprache und
  Textteil ein Formatelement Funktionstext; beim Import wird die gewählte Sprache ins **Wörterbuch** geschrieben
  (falsche Wahl = gemischte Einträge); unveränderte Funktionstexte behalten ihre Übersetzungen, geänderte verlieren
  sie (ohne „Zeilenumbrüche beibehalten“). Unicode-Sprachen brauchen Zeichensatz UNICODE im Schema.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_importzuli.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_exportzuli.htm
- **SPS-Datenaustausch:** klassische Formate einsprachig; **AutomationML** exportiert/importiert alle Sprachen, die
  gewählte ist Hauptsprache (Value), weitere als Attribute.
  Quellen: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_exportbuskonfig.htm ·
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_amlbusdatenaustausch.htm

## 9. Bedeutung für QET (Zusammenfassung)

- Ein zweisprachiges EPLAN-PDF zeigt je mehrsprachigem Text die Anzeigesprachen **untereinander in fester
  Reihenfolge** (z. B. de_DE oben, en_US darunter). Das ist **ein** Text (eine Eigenschaft mit mehreren Sprachen),
  nicht zwei Texte. QET kennt mehrsprachige Felder nicht: im Nachbau als ein Text mit Zeilenumbruch zwischen den
  Sprachen ablegen, Reihenfolge wie im PDF (Vermutung zur sinnvollsten Abbildung).
- Erkennungszeichen im PDF: Übersetzungen in GROSSBUCHSTABEN (Ausgabeeinstellung), Hinweistext oder leere Zeile bei
  fehlender Übersetzung, gleich bleibende Teile wie `+A` (als „nicht zu übersetzen“ markiert).
- Für ein Verknüpfungsskript zählt nur die **Quellsprache** bzw. die erste Zeile – BMK, Adressen, Anschluss-
  bezeichnungen sind nie mehrsprachig. Funktionstexte beim Abgleich Übersicht ↔ Kanal sprachweise vergleichen,
  nicht als Gesamtstring (Vermutung).
- Wird ein Fork-Export mehrsprachig gewünscht, wäre das Wörterbuchformat **tabgetrenntes Unicode-TXT** der einfachste
  Austauschweg (Spalten je Sprache; genaue Spaltenfolge in der Hilfe nicht beschrieben – Vermutung).

## 10. Quellen

Übersetzung (translategui):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_k_mehrsprachigetexte.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungprojekt.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungbenutzer.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungumfang.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_einstellungseiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_r_artikelverwaltung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_d_gefundenewoerter.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_d_testeingabe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_onlineuebersetzen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_sprachenwaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_textenichtuebersetzen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_seitennichtuebersetzen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_schriftartfestlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_texteersetzen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_fehlwortexport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_fehlwortimport.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translategui_h_entfernenuebersetzung.htm

Wörterbuch (translatedbgui):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_k_eingabemoeglichkeit.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_k_zahlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_k_ueberblickzeichen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_h_stichwortanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_h_trennvorschlag.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_h_woerterbuchanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_h_projekttexteimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_h_artikeltexteimportieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_d_sprachewaehlen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_r_uebersetzbar.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_r_verwaltung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_r_gesperrtebegriffe.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/translatedbgui_r_nichtuebersetzbar.htm

Querbezüge SPS (Sprachverhalten im Austausch):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_importzuli.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_exportzuli.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_d_exportbuskonfig.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/plcgui_k_amlbusdatenaustausch.htm
