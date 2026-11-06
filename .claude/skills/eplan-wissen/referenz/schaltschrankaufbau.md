# EPLAN: Schaltschrankaufbau (2D-Montageplatte, Artikelplatzierung, Legenden, Modell-Ansichten)

Referenz für den Nachbau von EPLAN-Aufbauplänen („Aufbauplan Schaltschrank“, Montageplatte maßstäblich, Gerätefronten,
Legende) als 2D-Blatt in QElectroTech. Grundlage ist die EPLAN-Plattform-Hilfe 2.9 (Module 2D-Schaltschrankaufbau
`panellayoutgui_*` und – für aus 3D abgeleitete 2D-Blätter – Modell-Ansichten `gededit3dgui_*` von EPLAN Pro Panel).
Alles in eigenen Worten; jede Aussage mit Quelle. Mit **(Vermutung)** markierte Stellen sind nicht aus der Hilfe belegt.
3D-Montageaufbau, Montageflächen, Kupfer, NC und STEP stehen in `3d-nc-kupfer.md`.

## Inhalt

1. Zwei Wege zum Aufbauplan: 2D-Schaltschrankaufbau oder Modell-Ansicht aus 3D
2. Seitentyp, Montageplatte, Maßstab
3. Artikelplatzierung: Abmessungen, Anfasspunkt, Einbauabstände
4. Platzierungsarten: einzeln, mehrfach, auf Tragschiene, Sperrflächen, ohne Schaltplan
5. Zusammenhang mit dem Schaltplan: Hauptfunktion, Darstellungsart, Sprünge, Prüfung
6. 2D-Schaltschrankaufbau-Navigator (Status, Baum, Liste, Filter)
7. Schaltschranklegende (Fenster- und Seitenlegende, Legendenposition, Nummerierung)
8. Verbindungsdiagramme auf dem Aufbaublatt
9. Modell-Ansichten (2D-Ansicht des 3D-Bauraums): Blickpunkt, Stil, Maßstab, Beschriftung, Bemaßung
10. Automatisch erzeugte Ansichtsseiten (Vorlagen)
11. Montageliste
12. Bedeutung für QET: Aufbauplan als 2D-Blatt nachbauen
13. Ergänzung: Bauraum-Dialoge (Strukturkennzeichen, Montageplatte 3D, Durchbruch, Eigenschaftsanordnung)
14. Quellen

---

## 1. Zwei Wege zum Aufbauplan

- **2D-Schaltschrankaufbau** (Grundplattform): Montageplatten und Bauteile werden auf Seiten vom Typ
  „Schaltschrankaufbau“ als 2D-Grafik platziert; Bauteile werden durch Grafikobjekte oder „zusammengesetzte Makros“
  aus der Artikelverwaltung dargestellt. Dazu gibt es Bauteillegenden.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_start.htm
- **3D-Montageaufbau mit EPLAN Pro Panel**: Aufbau im Bauraum (3D); daraus werden 2D-**Modell-Ansichten** erzeugt und
  auf beliebige Projektseiten gesetzt, ergänzt um Bemaßung und Texte als Fertigungsunterlage; Schaltschranklegende und
  Stückliste kommen aus denselben Daten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_k_start.htm
- Erkennen im PDF **(Vermutung)**: Modell-Ansichten zeigen Körperkanten mit verdeckten Kanten/isometrische Blicke,
  einen Ansichtsrahmen mit Ansichtsname/Beschreibung und oft automatische Kettenbemaßung an Tragschienen; der
  2D-Aufbau zeigt dagegen flache Makrografiken in einem Montageplatten-Kasten.

## 2. Seitentyp, Montageplatte, Maßstab

- Montageplatten gibt es nur auf Seiten vom Typ „Schaltschrankaufbau“; im Grafischen Editor ist die Montageplatte ein
  spezieller **Gerätekasten**, im Hoch- oder Querformat, mit eigenem BMK nach der Projektstruktur der normalen Bauteile.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_grundlagen.htm
- Drei Arten, eine Montageplatte zu setzen: (a) Gehäuseartikel aus der Artikelverwaltung (Produktobergruppe Mechanik >
  Gehäuse; Symbol wird auf die Artikelgröße skaliert, nur Werte > 0 zulässig), (b) „freie“ Montageplatte über
  Einfügen > Kasten / Anschluss / Montageplatte > Montageplatte (Rechteck aufziehen), (c) als Gerät. Danach im Dialog
  Eigenschaften, Registerkarte Montageplatte, das **Sichtbare BMK** eintragen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_montageplattenplatzieren.htm
- Montageplatte **als Gerät**: Artikel muss Produktobergruppe „Mechanik“, Produktgruppe „Gehäuse“, Produktuntergruppe
  „Montageplatten“ haben (Produktuntergruppe ID 22028), sonst wird er als normales Gerät behandelt. EPLAN sucht ein
  Grafikmakro (Registerkarte Montagedaten) oder Makro (Technische Daten), das mit Darstellungsart „Artikelplatzierung“
  gespeichert wurde; fehlt es, werden die Artikelabmessungen angeboten; fehlen auch diese, Abbruch. Einfügewege:
  Einfügen > Gerät, Drag & Drop aus Artikelstammdaten-Navigator oder Geräteliste, Data Portal.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_platzierungsarten.htm
- Weil die Platte realitätsgetreu gezeichnet ist, arbeitet man mit einem **Maßstab**, der die ganze Platte auf A3/A4
  bringt – deshalb werden Bauteile klein und Beschriftungen schwer lesbar (Grund für Legenden, Abschnitt 7).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_legenden.htm
- Bebaubare Höhe/Breite und maximale Einbautiefe einer Montageplatte sind Artikeleigenschaften (IDs 22116, 22117,
  22118); Positionen auf der Platte X/Y/Z/Winkel als „Montageplattenpositionen“ (IDs 19650–19654).
  Quelle: `eigenschaften-ids.md` (Nachschlagetabelle).

## 3. Artikelplatzierung: Abmessungen, Anfasspunkt, Einbauabstände

- Ein Bauteil auf der Platte ist eine **Artikelplatzierung** (Funktion mit Darstellungsart „Artikelplatzierung“, eigene
  Funktionsdefinition, siehe Abschnitt 4). Breite, Höhe, Tiefe und Einbauabstand sind Eigenschaften der
  Artikelplatzierung und stammen aus Artikeldatenbank oder Artikelmakro.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_grundlagen.htm
- Dialog **Einstellungen: 2D-Schaltschrankaufbau** (Navigator > Kontextmenü > Einstellungen oder Optionen >
  Einstellungen > Benutzer > Grafische Bearbeitung > 2D-Schaltschrankaufbau):
  - Bestückungsrichtung horizontal (links → rechts) oder vertikal (oben → unten) für Mehrfachplatzierung.
  - Drehwinkel beim Platzieren; **Anfasspunkt** (Standard: Mitte links, damit Geräte leicht auf eine Tragschiene
    passen; je Projekt einheitlich halten); „Vorrangig aus Makro“ nimmt den Anfasspunkt aus dem Makro.
  - Übernahme: „Abmessung aus Artikelstamm“, „Makro und Abmessung aus Artikelstamm“, „Makro aus Artikelstamm“ (Makro in
    Originalmaßen, Artikelmaße ignoriert), „Manuelle Eingabe“ (Dialog Ausdehnung und Abstand: Breite/Höhe sowie
    horizontaler/vertikaler Abstand zum Nachbarn), „Manuelle Eingabe, Makro aus Artikelstamm“ (Vorbelegung aus Stamm).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_d_schaltschrankaufbaueinstellungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_d_ausdehnungabstand.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_platzierungsarten.htm
- Ohne Makro wird ein Artikel als Rechteck in Artikelgröße dargestellt (Symbol auf Größe skaliert, nur Werte > 0).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_bauteileohneschaltplan.htm
- **Einbauabstände** müssen in den Artikeldaten stehen, damit Gerätewände nebeneinander statt aufeinander liegen; sie
  werden beim Positionieren automatisch berücksichtigt (Abmessungen aus Artikelverwaltung, Registerkarte Montagedaten).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_bauteileplatzieren.htm
- **Artikelabmessungen aktualisieren** (Kontextmenü auf der Aufbauseite): prüft jede Platzierung gegen die aktuellen
  Artikelmaße und fragt je Abweichung nach.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_artikelabmessungenaktualisieren.htm
- Beschriftung: Ausrichtung über die Winkelvariante der Symbolplatzierung, je Eigenschaftsplatzierung änderbar;
  Eigenschaftstexte lassen sich zu einem Block andocken (gemeinsam verschieben, keine Überlappung mehrzeiliger Texte).
  „Sichtbares BMK“ wählt, welches BMK an der Platzierung erscheint.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_grundlagen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_eigenschaftenartikelplatzierungenbearbeiten.htm
- Eine **Artikelreferenz** wird immer mit Menge 1 platziert; hat ein Betriebsmittel mehrere Artikel/Mengen, können
  mehrere Platzierungen mit demselben BMK auf einer Platte liegen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_eigenschaftenartikelplatzierungenbearbeiten.htm

## 4. Platzierungsarten

- **Einzelplatzierung**: Navigator zeigt alle im Schaltplan verwendeten bzw. in der Artikelvorauswahl vorhandenen
  Geräte (je nach Filter); beim Platzieren werden X/Y **auf der Montageplatte** (nicht auf der Seite) angezeigt.
- **Mehrfachplatzierung**: mehrere Betriebsmittel markieren, nach dem ersten Klick reihen sich die weiteren gemäß
  Bestückungsrichtung an, solange Platz ist; Überschreiten des Plattenrands wird gemeldet.
- Ablauf: im Navigator (Liste) nicht platzierte Artikel markieren > Kontextmenü **Absetzen auf Montageplatte**;
  „Stückzahl / Menge (nicht platziert)“ sinkt, im Baum erscheinen sie unter der Montageplatte mit Haken.
- **Klemmenleiste**: beim ersten Klemmenartikel fragt EPLAN, ob die ganze Leiste platziert werden soll; dann ist die
  Breite die Summe der Klemmenbreiten, die Höhe die größte Klemmenhöhe; sonst Klemmen einzeln.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_platzierungsarten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_bauteileplatzieren.htm
- **Absetzen auf Tragschiene** (Hilfsfunktion): Tragschiene wird als Linie, Polylinie, geschlossene Polylinie, Rechteck
  oder Bézierkurve gezeichnet (nicht Kreis/Ellipse); der Einfügepunkt der Platzierung läuft auf diesem Objekt,
  Einbauabstände gelten, neue Teile nur im freien Bereich, Anreihung links → rechts.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_hutschienenplatzierung.htm
- **Sperrflächen**: eigenständige Rechteck-Objekte (Kontextmenü > Sperrfläche); darin abgesetzte Platzierungen werden
  verweigert. Artikelplatzierungen sind nur innerhalb einer Montageplatte erlaubt (sonst Meldung).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_sperrflaechenzeichnen.htm
- **Bauteile ohne Schaltplan**: Kontextmenü > Neues Gerät > Artikel wählen > platzieren > BMK in Registerkarte
  Artikelplatzierung. Funktionsdefinition wird aus der Produktgruppe abgeleitet: Klemmen/Tragschiene →
  „Artikelplatzierung Klemmenleiste“, Klemmen/sonst → „Artikelplatzierung Klemme“, Stecker → „Artikelplatzierung
  Stecker Stift/Buchse“, SPS → „Artikelplatzierung SPS-Karte“, alles andere → „Artikelplatzierung normales Bauteil“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_bauteileohneschaltplan.htm
- **Darstellungsart „Schaltschrankaufbau detailliert“**: dieselbe Platzierung ein zweites Mal (z. B. vergrößerter
  Ausschnitt auf zweiter Aufbauseite) zeigen, ohne die Stücklistenmenge zu erhöhen (Menge bleibt 1). Standard ist
  Darstellungsart „Schaltschrankaufbau“. Einstellung in Registerkarte Artikelplatzierung > Darstellungsart.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_platzierungsarten.htm

## 5. Zusammenhang mit dem Schaltplan

- Die Artikelplatzierung gehört zum **Betriebsmittel** des Schaltplans (gleiches BMK); im Navigator springt
  **Gehe zu (Grafik)** bei nicht platziertem Betriebsmittel zur Seite der Hauptfunktion (hervorgehoben), bei
  platziertem zum Makro auf der Montageplatte. Sprungfunktionen führen immer zur Hauptfunktion.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_sprungfunktionen.htm
- Seit der 2.9-Neuerung „Markierungen für Sprungziele im PDF“ sind im PDF Sprünge zwischen Haupt- und Nebenfunktionen
  von 2D-Montageplatten auf Schaltschrankaufbau-Seiten möglich (siehe `pdf-export.md`). Ob die Hauptfunktion einen
  Querverweis zur Aufbauseite zeigt, regelt die Querverweis-Anzeige je Seitentyp (siehe `querverweise.md`).
- **Hauptbauteile aktualisieren**: Wurde zuerst im Aufbau platziert und erst danach das Schaltzeichen gezeichnet, hat
  die Hauptfunktion oft keine Artikel, die Platzierungen aber schon. Der Befehl überträgt alle Artikelreferenzen der
  zugehörigen Platzierungen in die Hauptfunktion und **löscht** vorher deren Artikeldaten (Warnung).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_hauptbauteileaktualisieren.htm
- **Prüfung der Platte**: Beim Absetzen übernimmt die Platzierung den Einbauort der Montageplatte. Geprüft wird über die
  Strukturkennzeichenblöcke von Montageplattenname und Betriebsmittel (z. B. =A1+O1-MP1 mit =A1+O1-K1 korrekt, mit
  =A1+O2-K1 Fehler), optional auch „Montageort (beschreibend)“. Aktivierung über Meldungsverwaltung, Prüflaufklasse
  „2D-Schaltschrankaufbau“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_artikelplatzierungenueberpruefen.htm
- Filter im Navigator wirken auf **Artikelreferenzdaten**, nicht auf Artikeldaten: z. B. Filter auf Montagefläche
  (ID 22022) liest die Referenzeigenschaft „Artikel: Montagefläche“ (ID 20918), die vom Stammartikel abweichen kann.
  Eigenschaften „Funktion ist mit Darstellungsart ‚…‘ vorhanden“ filtern z. B. Teile, die **nur** auf der Platte
  existieren (Kontrollkästchen auf Nein).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_grundlagen.htm

## 6. 2D-Schaltschrankaufbau-Navigator

- Aufruf: Projektdaten > Geräte / Artikel > 2D-Schaltschrankaufbau-Navigator. **Liste** = BMK-orientiert (Spalten frei,
  z. B. Stückzahl/Menge (nicht platziert)); **Baum** = platzierungsorientiert, oberste Ebene ist der Artikelstatus.
  Artikel von Verbindungen werden immer ausgefiltert; mehrere Artikel eines Betriebsmittels stehen in der Reihenfolge der
  Registerkarte Artikel. Ein eigenes Icon markiert Platzierungen ohne Gegenstück im Schaltplan.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_d_schaltschrankaufbaupdd.htm
- Status: **Alle Artikel** (grüner Haken an Artikelnummer = platziert, am BMK = alle Artikel platziert),
  <Montageplatte>, **Fehlerhaft platziert** (nachträglich aus der Platte geschoben und mit F5 aktualisiert; normales
  Absetzen außerhalb wird verweigert). Sortierung nach Strukturkennzeichen; „Ohne Strukturkennzeichen“ (nur bei
  Kennzeichnungsstruktur) und darunter „Ohne BMK“. Ansichten „Projektstruktur“ und „Projektstruktur mit Kennzeichen“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_grundlagen.htm

## 7. Schaltschranklegende

- Die Schaltschranklegende ist eine **funktionsbezogene Auswertung**: das BMK der Montageplatte muss eindeutig sein.
  Statt des BMK steht am Bauteil eine kurze **Positionsnummer**, in der Legende Name des Betriebsmittels und wahlweise
  Typnummer, Querverweis usw. Ist für eine Platte keine Legende aktiv, wird das komplette BMK ans Artikelsymbol
  geschrieben; sonst bekommen alle Platzierungen fortlaufende Nummern.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_legenden.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_grundlagen.htm
- Zusammenfassen gleicher Artikelnummern in einer Zeile über Formulareigenschaft „In einer Zeile zusammenfassen“ mit
  Artikelnummer (ID 20100); Anzahl über Platzhalter **Gesamtmenge (Stückzahl)** (ID 20499).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_legenden.htm
- **Fensterlegende**: Legende auf derselben Seite wie die Platte; frei positionierbares Grafikobjekt mit Einfügepunkt
  und Symboleigenschaften (z. B. BMK); unter ihr steht der Name des Montagebereichs. Je Montageplatte standardmäßig
  eine eigene. Erzeugen: Dienstprogramme > Auswertungen > Erzeugen > [Neu] > Ausgabeform **Manuelle Platzierung** >
  Schaltschranklegende > auf der Seite absetzen; EPLAN bildet dabei eine Vorab-Stückliste der Teile auf der Platte.
  Unterdrücken: Eigenschaft „Erzeugung der Schaltschranklegende unterdrücken“ an der Montageplatte.
- **Seitenlegende**: eigenes Formular auf eigenen Auswertungsseiten (Ausgabe in Seiten, Formular für Auswertungstyp
  Schaltschranklegende, optional Manuelle Auswahl der Platten, Strukturkennzeichen/Seitenbezeichnung festlegen); auf der
  Aufbauseite selbst stehen dann trotzdem nur Positionsnummern. Tipp: Baugruppen und Module können vorher in Einzelartikel
  aufgelöst werden (Einstellungen: Artikel).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_legendenanlegen.htm
- **Legendenposition bearbeiten** (Navigator, Platte im Baum markieren; keine Mehrfachauswahl): Reihenfolge verschieben,
  [Nummerieren] mit Startwert, Schrittweite und Richtung: „Gegebene Reihenfolge“, **Horizontal/Vertikal** (nach Lage des
  Anfasspunkts auf der Platte; nur im 2D, nicht im 3D-Bauraum) oder „Automatisch“ (nur Pro Panel: erst Bauteilklassen
  nach Hierarchie/Subhierarchie, innerhalb davon geometrisch). „Gleiche Nummer für identische Geräte“ (wichtig für GOST).
  Spalte Legendenposition in der Liste einblendbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_legendenaktualisieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_d_nummerieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_d_legendenposbearbeiten.htm
- Bei **Modell-Ansichten** wird die Legende nicht vom Bauraum, sondern je Modell-Ansicht erzeugt: mehrere Ansichten auf
  einer Seite → je eine Legende mit den Teilen der jeweiligen Basis-Bauteile; für den ganzen Bauraum eine Ansicht ohne
  Einschränkung der Basis-Bauteile anlegen. Unterdrückbar mit derselben Eigenschaft an der Modell-Ansicht.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_k_legende.htm

## 8. Verbindungsdiagramme

- Verbindungsdiagramm: Betriebsmittel in derselben **Reihenfolge** wie auf der Montageplatte (Koordinaten und Maße egal),
  dazu je Ziel eine Linie mit Angaben zu angeschlossenen Verbindungen; bei mehreren Zielen größerer Abstand; je Funktion
  ein (beliebiges) Symbol möglich.
- Voraussetzung: Gerätekasten mit dem BMK der Montageplatte, Darstellungsart „Übersicht“, neben der Platte oder auf
  einer Übersichtsseite (nie auf einer Auswertungsseite). Erzeugen per Manuelle Platzierung mit Auswertungstyp
  „Geräte-/Klemmen-/Steckeranschlussplan (für Verbindungsdiagramm)“, Manuelle Auswahl, Montageplatte wählen, optional
  bereits ausgewertete ausblenden; dann in den Gerätekasten setzen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_verbindungsdiagramme.htm

## 9. Modell-Ansichten (aus 3D abgeleitete 2D-Blätter)

- Standardisierte 2D-Ansicht des 3D-Montageaufbaus (Anlage, bestückte Montagefläche, einzelne Komponente), durch zwei
  Punkte als Rahmen definiert, Inhalt passend skaliert; in **jeden Seitentyp** einfügbar (Einfügen > Grafik >
  Modell-Ansicht). Kopie in ein anderes Projekt verliert den Bauraumbezug (nur noch Grafik).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_h_bearbeiten.htm
- Registerkarte Ansicht: eindeutiger **Ansichtsname**, Beschreibung, genau ein **Bauraum**, **Basis-Bauteile** (auch
  tiefer liegende wie Montageflächen; ohne Grafik wird das nächsthöhere Bauteil mit Grafik ergänzt), [Ein-/ausblenden].
  - **Blickpunkt**: orthogonal Oben/Unten/Links/Rechts/Vorne/Hinten oder isometrisch SW/SO/NO/NW (Y-Achse = Norden,
    60° nach unten geneigt). In isometrischen Ansichten keine Bemaßung und keine Texte.
  - **Stil**: Drahtdarstellung (ohne Verdeckung, schwarze Kanten 0,5 mm), **Verdeckte Kanten** (Voreinstellung),
    Verdeckte Kanten / Vereinfachte Darstellung (Klemmenleisten und 3D-Makros vereinfacht), Shading (Bitmap, keine
    Bemaßung möglich), Shading / Vereinfacht.
  - **Maßstab**: Automatisch (nur Maßstäbe nach DIN ISO 5455), Einpassen (beliebige ganze Zahl, z. B. 1:12), Manuell (1:x).
  - Eigenschaften: Bauteilbeschriftung (Schema je Funktionsdefinition mit Eigenschaftsanordnung, wahlweise „Aus
    Bauraum“), Drehung 90/180/270° gegen Uhrzeiger (Rahmen dreht nicht mit; Texte drehen mit, außer von rechts zu
    lesende → −90°), Texte aus Bauraum nicht automatisch drehen, Schema für Automatische Bemaßung, Auswahlschema
    (welche Bauteiltypen sichtbar; nach manuellem Ein-/Ausblenden „Benutzerdefiniert“), Blickpunkt auf Montagefläche
    beziehen (schräge Flächen unverkürzt), **Eingefroren** (keine Aktualisierung, auch wenn Bauraum gelöscht),
    **Kantenbreite** (fester Wert, so auch im PDF), Bauteilkanten in Schwarz.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_modell.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_einstellbeschriftung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_einstellbauteilauswahl.htm
- Aktualisieren: geänderter Bauraum → Meldung „Modell-Ansicht nicht aktuell“; Dienstprogramme > Auswertungen >
  Aktualisieren (vorher Einfrieren aus). Inhalt zentrieren / Inhalt verschieben ändern nur die Lage im Rahmen.
  Eigenschaftstexte einzelner Bauteile in der Ansicht verschiebbar und wieder zurücksetzbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_h_veraendern.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_h_eigenschaftstexteverschieben.htm
- **Automatische Bemaßung** (Schema, projektbezogen): für Bauteile und NC-Durchbrüche in Modell-Ansichten,
  2D-Bohransichten und Kupferabwicklungen; nicht in isometrischen/seitlichen Ansichten und **nicht im
  2D-Schaltschrankaufbau**. Neu erzeugen löscht alte automatische Maße (manuelle bleiben, Nachbearbeitung geht verloren).
  Bei Trag-/Profil-/Sammelschienen und Klemmenleisten: erster Maßpunkt an einer wählbaren Ecke des Trägers, nur
  steigende Bemaßung, nur das erste aufgereihte Bauteil (nach jeder Lücke wieder das erste). Einstellungen:
  Registerkarte Bemaßung (erster Maßpunkt, Maßpunkt am Bauteil, Maßpunkt liegt auf Montagefläche/Feldgröße/umgebendem
  Rechteck, Lage und Abstand der Maßlinie in mm zum hierarchisch höchsten Bauteil, Maßhilfslinie) und Registerkarte
  Bauteile (Bauteilart nach Funktionsdefinition, Bemaßen bis zur Hierarchiestufe, Maßpunkt am Bauteil).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_k_autobemassung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_bemassung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_bauteile.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_einstellautobemassung.htm
- **Ansichten nummerieren** (Dienstprogramme > Auswertungen): alle Ansichten des Projekts (Modell-, Bohr-,
  Kupferansichten gemeinsam) fortlaufend, Startwert, Mindeststellenzahl mit führenden Nullen (0 = keine).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_ansichtennummerieren.htm

## 10. Automatisch erzeugte Ansichtsseiten (Vorlagen)

- Dialog Modell-Ansichten (bzw. Kupferabwicklungen, 2D-Bohransichten): Baum Auswertungsart > Bauraum > Ansichten >
  Seiten; Aktualisieren, Löschen, Seite öffnen. Registerkarte **Vorlagen**: Name, Startseite des Auswertungsblocks,
  Filter für Basis-Bauteile (je Treffer eine Seite; ohne Filter je Bauraum eine), Sortierung, Funktionsfilter (welche
  untergeordneten Teile sichtbar), Gewerk, manuelle/automatische Ansichts- und Seitenbeschreibung mit Format,
  Seitensortierung, **Makro** (Fenster-/Symbol-/Seitenmakro mit vorbereiteter Ansicht). Vorlagen enthalten keine
  Geometrie; Import/Export möglich.
- Ablauf: Ansicht einmal bauen, als Seitenmakro speichern, Vorlage anlegen, [Auswerten]. „Projekt auswerten“ erzeugt
  bzw. aktualisiert diese Seiten ebenfalls.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_k_autoansichterstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_h_autoansichterstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_vorlagen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_modellansichten.htm

## 11. Montageliste

- Pro-Panel-Auswertung „Montageliste“ (Formular *.f32): bildet die Hierarchie im Bauraum ab. Ausgewertet werden
  3D-Platzierungen mit **In Montageliste ausgeben** (ID 36032); Kopf = Daten dieses Objekts (z. B. Schaltschrank),
  Datenbereich = nur die **unmittelbar** darunterliegende Ebene, sortiert nach Legendenposition. Für die Teile auf einer
  Montageplatte im Schrank braucht die Platte die Eigenschaft ebenfalls (zweite Liste). Platzhalter aus Artikel-,
  Artikelplatzierungs-, Artikelreferenz-, Betriebsmitteldaten und Datensatz; Manuelle Auswahl möglich.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_auswertungmontageliste.htm

## 12. Bedeutung für QET: Aufbauplan als 2D-Blatt nachbauen

QET hat weder Seitentyp „Schaltschrankaufbau“ noch Artikelplatzierungen, Bauraum oder Legendenauswertung. Der Nachbau
ist ein normales Folio mit maßstäblichen Bauteilen; der Inhalt (wer ist wo, welcher Artikel) muss trotzdem als Daten
stimmen. Konkret:

1. **Blatt und Maßstab**: Maßstab aus einem bekannten Maß bestimmen (Plattenbreite oder Schrankbreite im PDF ÷ mm) und
   alle Makros mit demselben Faktor skalieren (so macht es bereits `aufbau_massstab.skaliere`, siehe
   qet-schaltplan-zeichnen). Bei Modell-Ansichten steht der Maßstab oft im Ansichtstext; DIN-ISO-5455-Maßstäbe
   (1:5, 1:10, 1:20 …) deuten auf „Automatisch“, krumme ganze Zahlen auf „Einpassen“ **(Vermutung)**.
2. **Montageplatte** = großes QET-Bauteil (Rechteck in Plattenmaß) mit eigenem Kennzeichen (z. B. -MP1/-U1) und
   Ortskennzeichen; nicht als freie Linien. Sie entspricht dem „Gerätekasten“ der Montageplatte.
3. **Jedes Gerät auf der Platte** = QET-Bauteil mit Herstellergrafik (Aufbau-/Frontmakro, Darstellungsart
   Artikelplatzierung/2D) und **demselben Kennzeichen und Artikel wie im Stromlaufplan**, damit Stückliste und Navigation
   stimmen. Mehrfach platzierte Artikel eines Betriebsmittels (Menge > 1) dürfen dasselbe Kennzeichen tragen. Eine zweite
   Detaildarstellung („Schaltschrankaufbau detailliert“) darf in Auswertungen nicht doppelt zählen → in QET als Bauteil
   ohne Stücklistenrelevanz bzw. Bild markieren **(Vermutung, Umsetzung offen)**.
4. **Hutschienen und Kanäle**: als Linien/Rechtecke oder skalierte Herstellergrafik mit Artikeldaten (eigene Bauteile,
   damit sie in Stücklisten erscheinen); längenvariable Teile mit Länge als Datum. Geräte im PDF sitzen mit dem
   Anfasspunkt „Mitte links“ auf der Schiene – das hilft beim Ausrichten der Bauteil-Einfügepunkte **(Vermutung, abgeleitet
   aus der Standardeinstellung)**.
5. **Positionsnummern statt BMK**: Zeigt das Original kurze Nummern an den Geräten, ist eine Legende aktiv (Fenster- oder
   Seitenlegende). Dann im Bauteil das echte Kennzeichen als Eigenschaft setzen, sichtbar nur die Positionsnummer
   (Text ausblenden wie bisher), und die Zuordnung Nummer ↔ BMK/Artikel aus der Legende oder Artikelstückliste
   übernehmen (`stueckliste.py`). Steht das volle BMK am Symbol, war keine Legende aktiv.
6. **Legende auf dem Blatt** (Fensterlegende) ist eine eingebettete Auswertung: aus den QET-Daten der Platte erzeugen
   (Tabelle/Text), nicht als Grafik kopieren; gleiche Artikelnummern ggf. in einer Zeile mit Stückzahl zusammenfassen.
   Unter der Legende steht der Name des Montagebereichs. Eine Seitenlegende ist ein eigenes Auswertungsblatt.
7. **Navigation**: Aufbauplatz und Schaltplan gehören zum selben Betriebsmittel. Im EPLAN-PDF springt man ab 2.9 zwischen
   Haupt- und Nebenfunktionen der 2D-Platte; im QET-Nachbau also Bauteil auf dem Aufbaublatt mit der Hauptfunktion
   verknüpfen (Verknüpfungsskript: Aufbaubauteil als zusätzlicher Teilnehmer derselben Kennzeichengruppe, nicht als
   eigenes Gerät) **(Vermutung zur Umsetzung)**. Ob die Hauptfunktion einen sichtbaren Querverweis „Aufbau“ trägt,
   richtet sich nach der EPLAN-Querverweisanzeige je Seitentyp – im Original nachsehen.
8. **Prüfung**: Ortskennzeichen (+) der Geräte auf der Platte muss zum Ortskennzeichen der Platte passen (EPLAN-Prüfung
   über Strukturkennzeichenblöcke). Lohnt als Check im Abgleichskript: Gerät auf Platte +S1 mit Kennzeichen +S2 → Fehler.
9. **Modell-Ansichten** (Pro Panel): Ansichtsrahmen mit Ansichtsname/Beschreibung, Bemaßung und isometrische Bilder sind
   reine Grafik; Bemaßung als QET-Linien/Text übernehmen (aufbau_rest), Bauteile trotzdem als Datenbauteile wie oben.
   Shading-Ansichten sind Bitmaps → als Bild übernehmen, Bauteile als Umriss-Bauteil mit Artikeldaten (`bildbauteil`).
10. **Verbindungsdiagramm** auf dem Aufbaublatt ist eine Auswertung (Geräte-/Klemmen-/Steckeranschlussplan) in einem
    Übersichts-Gerätekasten – aus den Verbindungsdaten erzeugen, nicht abzeichnen.

## 13. Ergänzung: Bauraum-Dialoge (Strukturkennzeichen, Montageplatte 3D, Durchbruch, Eigenschaftsanordnung)
**Strukturkennzeichen des Bauraums**: Artikelplatzierungen im Bauraum übernehmen grundsätzlich die in den Projekteigenschaften festgelegte Struktur der **Mechanik-Betriebsmittel**; angezeigt werden die Kennzeichen des Bauraums, in dem die Platzierung liegt. Ohne erweiterte Referenzkennzeichnung gibt es die Spalte „Wert“, mit ihr „Sichtbar“ und „Vollständig“ (inkl. aller übergeordneten Kennzeichen). Vor- und Trennzeichen sind hier nicht editierbar, sie stammen aus „Erweiterte Projektstrukturen“ (Registerkarte Struktur > [Weitere]).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_d_vollstaendigesbmk_bauraum.htm

**Registerkarte Montageplatte (3D)**: sichtbares BMK wird bei aktiver Online-Nummerierung vorgeschlagen; vollständiges BMK = Strukturkennzeichen der Umgebung + sichtbares BMK. Das Feld „Bauteil“ kommt aus der Funktionsdefinition und dient der Anzeige im Bauraum-Navigator; die Funktionsdefinition der Platte wird passend zum Bauteil automatisch zugeordnet.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_montageplatte3d.htm

**Registerkarte Durchbruch**: Typen Bohrung (Durchmesser), Gewindebohrung (metrisches ISO-Gewinde), Kontur (Datei `*.fc1` aus dem Makro-/Konturverzeichnis), Langloch (Länge muss größer als Höhe sein), Rechteck (eckig/gerundet/gefast mit Radius bzw. Fasenabstand), Sechseck und Achteck (Kantenlänge). Einheiten aus den Projekteinstellungen, Inch-Eingaben werden in mm umgerechnet.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_durchbruch.htm

**Eigenschaftsanordnungen (Bauteilbeschriftung in Modell-Ansichten)**: vordefinierte Anordnungen bestimmen, welche Eigenschaften wie platziert werden. Einzeln platzierte Eigenschaften lassen sich **andocken** – sie bilden einen Block, verschieben sich gemeinsam und mehrzeilige Texte überlappen nicht. „Löschen“ entfernt nur den Listeneintrag, nicht den Wert; Eigenschaften mit Wert erscheinen beim nächsten Öffnen wieder. Anzeigeparameter je Eigenschaft: Format, Textrahmen, Positionsrahmen, Wert/Einheit, Position.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_eigenschaftsanordnung.htm

**Bedeutung für QET**: angedockte Eigenschaftsblöcke entsprechen am ehesten einer Textgruppe im Element (gemeinsam verschiebbar); für 2D-Aufbaupläne genügt die Beschriftung mit BMK bzw. Positionsnummer (siehe Abschnitt 12).

## 13a. Ergänzungen (2026-11-06): Student Handbook und Pro-Panel-Ablauf

Quelle Handbuch: EPLAN Education Handbook „Student Handbook“, Stand 16.12.2021,
https://www.eplan.com.tr/fileadmin/eplan_content_international/tr/data/Student_Handbook.pdf

- 2D-Aufbauseiten liegen im selben Electric-P8-Projekt wie der Schaltplan; die Bauteile bleiben mit den
  Schaltplankomponenten verknüpft. Die 2D-„Footprint“-Makros kommen aus der Artikelverwaltung, ein Aufbau ist auch
  **ohne Schaltplan** möglich. (Student Handbook, S. 34)
- Die Projektprüfung (Meldungsverwaltung) gilt auch für die Geräte auf der 2D-Montageplatte. Die Schaltschranklegende
  listet alle Geräte auf **Montageplatte, Türen und Schrankflächen**. (Student Handbook, S. 35)
- Auswertungen lassen sich als eingebettete Auswertung direkt neben die Montageplatte setzen (Gerätelegende auf dem
  Aufbaublatt). (Student Handbook, S. 43) – deckt sich mit Abschnitt 7 (Fensterlegende).
- Das Bild auf S. 34 zeigt die typische Anordnung: Aufbauseite in eigener Struktur (Seitenname „Mounting panel“),
  Montageplatte als Rechteck mit sichtbarem BMK (z. B. „-UR1“) und Kettenmaßen, rechts daneben die Legende „Enclosure
  legend“ mit Spalten Position/BMK/Artikel. (Beobachtet im Student Handbook, S. 34)

**Pro-Panel-Ablauf als Arbeitsreihenfolge** (zusammengesetzt aus der Hilfe; Einzelquellen in `3d-nc-kupfer.md` 2, 3, 8,
9, 14a):
1. Bauraum anlegen (Bauraum-Navigator) – Strukturkennzeichen des Bauraums = Ort der Mechanik.
2. Schaltschrank aus der Artikelverwaltung einfügen (Montageplatte ist Teil des Schrankartikels) oder Montageplatte
   einzeln/frei setzen; Montagefläche der Platte aktivieren.
3. Längenvariable Teile: Verdrahtungskanäle und Tragschienen per Start-/Endpunkt aufziehen, „Länge übernehmen“,
   „Mittig platzieren“, „Einpassen“ an Plattenkante/Nachbarkanal, „Gleichmäßig verteilen“.
4. Geräte aus dem 3D-Montageaufbau-Navigator (Schaltplan-BM mit Artikel) auf Schienen/Platte setzen; Lage aus Anfasspunkt,
   Aufklipshöhe (Tiefe) und Mittenversatz (quer zur Schiene); Einbauabstände und Kollisionskontrolle beachten,
   Sperrflächen sperren Bereiche.
5. Durchbrüche entstehen aus den Bohrbildern der Artikel; Bohrsperrflächen und Feldgröße/Nullschnitt prüfen.
6. Modell-Ansichten und 2D-Bohransicht auf Seiten setzen, Legende/Montageliste auswerten, NC-Daten (Perforex,
   Steinhauer, DXF, Bohrschablone) und ggf. Athex/Kupfer-Daten exportieren.

## 14. Quellen

2D-Schaltschrankaufbau:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_grundlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_legenden.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_k_platzierungsarten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_artikelabmessungenaktualisieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_artikelplatzierungenueberpruefen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_bauteileohneschaltplan.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_bauteileplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_eigenschaftenartikelplatzierungenbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_hauptbauteileaktualisieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_hutschienenplatzierung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_legendenaktualisieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_legendenanlegen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_montageplattenplatzieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_sperrflaechenzeichnen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_sprungfunktionen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_h_verbindungsdiagramme.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_d_ausdehnungabstand.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_d_legendenposbearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_d_nummerieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_d_schaltschrankaufbaueinstellungen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/panellayoutgui_d_schaltschrankaufbaupdd.htm

Modell-Ansichten, Legende, Montageliste (Pro Panel):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/cabinetgui_k_auswertungmontageliste.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_k_start.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_k_legende.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_k_autobemassung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_k_autoansichterstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_h_autoansichterstellen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_h_bearbeiten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_h_eigenschaftstexteverschieben.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_h_veraendern.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_modell.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_anzeige.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_bemassung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_bauteile.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_r_vorlagen.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_modellansichten.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_3dobjektauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_3dobjekteeinausblenden.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_ansichtennummerieren.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_einstellautobemassung.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_einstellbauteilauswahl.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gededit3dgui_d_einstellbeschriftung.htm

Zusatz-Dialogseiten (Ergänzung Abschnitt 13):
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_d_vollstaendigesbmk_bauraum.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_montageplatte3d.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_durchbruch.htm
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/eservicesgui_d_eigenschaftsanordnung.htm
