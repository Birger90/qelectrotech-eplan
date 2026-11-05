# EPLAN: Kennzeichnung (IEC/EN 81346), Strukturen, Nummerierung

Wie EPLAN Seiten und Betriebsmittel kennzeichnet, woraus sich das vollständige BMK zusammensetzt (Seite,
Strukturkasten, Gerätekasten) und wie das sichtbare BMK verkürzt wird. Grundlage: offizielle EPLAN-Hilfe
(Plattform 2.9, abgeglichen mit 2025/2026), in eigenen Worten. Jede Aussage mit Quelle; „Vermutung“ = nicht belegt.

## Inhalt

1. Kennzeichnungsblöcke und Vorzeichen
2. Haupt- und Unterkennzeichen, Trennzeichen
3. Identifizierend vs. beschreibend
4. Projektstruktur: Seitenstruktur und Betriebsmittelstruktur
5. Erweiterte Referenzkennzeichnung (EN 81346, Produktaspekt)
6. Seitenname und Seitenkennzeichnung (EN 61355, Dokumentart)
7. Das BMK: Bestandteile, sichtbar vs. vollständig
8. Vererbung: Seite, Strukturkasten, Gerätekasten, „>“
9. Strukturkästen (Ortsrahmen)
10. Kennbuchstaben (IEC 81346-2)
11. Nummerierung online/offline
12. Abbildung in QET (Zusammenfassung)
13. Ergänzungen (BMK-Übernahme, Nummerierungsformat, Seiten, Suche, Kopieren)
14. Quellen

---

## 1. Kennzeichnungsblöcke und Vorzeichen

| Block | Vorzeichen | Zweck |
|---|---|---|
| Funktionale Zuordnung | `==` (alternativ `#`, `≠`) | funktionale Zusammenhänge zwischen Anlagen/Betriebsmitteln |
| Anlage | `=` | Anlage, Teilanlage, technische Einrichtung (Funktionsaspekt) |
| Aufstellungsort | `++` | topographischer Ort (Gebäude, Raum) |
| Einbauort | `+` | Schrank, Tafel, Einbauplatz (Ortsaspekt) |
| Anlagennummer | frei wählbar | (Fluid: Anlagenbezeichnung nach ISO 1219-2) |
| Dokumentenart | `&` (oder Leerzeichen) | Schaltplan, Klemmenplan, Inhaltsverzeichnis … |
| Benutzerdefiniert | frei (Vorbelegung `#`) | eigene Hierarchie |
| Betriebsmittel | `-` | BMK im engeren Sinn (Produktaspekt) |

Alle Blöcke sind beliebig kombinierbar und gelten für Seiten; für Betriebsmittel alle außer der Dokumentenart.
Blöcke müssen nicht gefüllt werden (leere Strukturkennzeichen sind erlaubt).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_hauptkundunterk.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pleditorgui_k_grundlagen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_darstellungerwprjstrukturen.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_k_aufbauvonbm.htm

Reihenfolge im BMK laut Hilfe: benutzerdefiniert, `==`, `=`, `++`, `+`, `-`. Beispiel `=AP+ST1-M1`: Motor M1 am
Einbauort ST1 der Anlage AP. Seite `=AP+ST1/6`: `/` trennt den Seitennamen.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_k_aufbauvonbm.htm,
https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_aufbaukennzeichnungsbl.htm

Verbotene Vor-/Trennzeichen: `/` (vor Seitenname), `\`, `?` (Platzhalter Offline-Nummerierung), `:` (vor
Anschlussbezeichnung, z. B. `-X5:3`).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_h_frhierarchienerstellen.htm

Unterstützte Normen/Schemata: IEC 81346, DIN ISO 1219, NFPA, NEC, GOST (NEC/GOST nicht vollständig).
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_hinterg.htm

## 2. Haupt- und Unterkennzeichen, Trennzeichen

- Jeder Block hat ein Hauptkennzeichen und Unterkennzeichen (Gliederungsstufen): Anlage 1–3, Einbauort 1–5,
  Aufstellungsort 1–2, Funktionale Zuordnung 1–2, Dokumentenart 1–3, Betriebsmittel 1.
- Trennzeichen für Unterkennzeichen: `.` oder Wiederholung des Vorzeichens (`=A1.A1` bzw. `=A1=A1`); beim Punkt dürfen
  Abschnitte am Anfang/Ende entfallen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_hauptkundunterk.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_k_aufbauvonbm.htm
- Option „Auch zwischen Ziffer und Buchstabe trennen“: Wechsel Ziffer→Buchstabe gilt als Unterstruktur
  (`=A1A2A3` ≙ `=A1.A2.A3`; Produkt `-K01Q04S01` ≙ `-K01`,`Q04`,`S01`).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_darstellungerwprjstrukturen.htm

**Bedeutung für QET:** `+BP1.S1` ist **ein** Einbauort mit Unterkennzeichen, kein zweiter Ort. In QET als
location-Wert komplett (`BP1.S1`) speichern; beim Zusammensetzen nicht am Punkt trennen.

## 3. Identifizierend vs. beschreibend

- Je Kennzeichnungsblock legt das Schema fest: **identifizierend**, **beschreibend** oder **nicht verfügbar**. Nur dann
  sind Vor- und Trennzeichen wählbar. Seitenstruktur erweitern geht immer (auch beschreibend → identifizierend);
  reduzieren nur, wenn keine doppelten Seiten entstehen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_h_frhierarchienerstellen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_hinterg.htm
- Identifizierende Strukturkennzeichen unterscheiden Objekte (zwei Seiten `/1` in verschiedenen Anlagen sind verschiedene
  Seiten) und erscheinen standardmäßig in den Navigator-Bäumen; beschreibende sind nur Information.
  Quelle: https://eplan.help/en-us/Infoportal/Content/Plattform/2.8/Content/htm/projectstructure_h_kennzeichenausblenden.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_objektkennzeichen.htm
- Bei Funktionen sind identifizierend: BMK, Anschlussbezeichnung, Funktionsdefinition, Darstellungsart; beschreibend
  u. a. „Montageort (beschreibend)“, Funktionstext, Graviertext, technische Kenngrößen. Bei Strukturkästen:
  identifizierend sind die Felder der Projekthierarchie (Anlage, Aufstellungsort, Einbauort).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_k_bmeigenschaften.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_k_grundlagen.htm
- Querverweise kürzen nur, wenn **alle identifizierenden** Eigenschaften gleich sind („Gleiche Seite unterdrücken“).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_h_qvwanzeigeoptimieren.htm

**Bedeutung für QET:** Gleiches BMK in verschiedenen Anlagen/Orten = **verschiedene Geräte** (z. B. `=DA-K1` und
`=SB-K1`). Master/Slave-Zuordnung und Stückliste daher immer über `=Anlage+Ort-BMK`. Ein beschreibender Ort im Original
(z. B. Montageort-Text) gehört nicht in den Schlüssel (Vermutung: an der Strukturübersicht des Plans erkennbar).

## 4. Projektstruktur: Seitenstruktur und Betriebsmittelstruktur

- Projektstruktur = Seitenstruktur + Betriebsmittelstruktur; letztere getrennt je Typ (allgemeine Betriebsmittel,
  Klemmenleisten, Stecker, Kabel, Gerätekästen …), je über ein Kennzeichnungsschema. Vorgabe aus Projektvorlage/
  Basisprojekt, später in den Projekteigenschaften (Registerkarte Struktur) änderbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_h_prjstrukturdefinieren.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_struktur.htm
- Seitenstruktur-Dialog: Blöcke, Reihenfolge (Pfeiltasten) und je Block identifizierend/beschreibend/nicht verfügbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_seitenstruktur.htm
- Option „Übergeordnet“ (Klemmenleisten, Stecker, Kabel, Abbruchstellen; abgekündigt): ohne „-“ im BMK wie
  gesamtnummeriert ohne Strukturblöcke (`X3` bleibt `X3`, `-X3` wird `Z1+A1-X3`).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_struktur.htm
- Stimmen Betriebsmittel- und Seitenstruktur überein, übernimmt ein Schaltzeichen die Strukturkennzeichen seiner Seite.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_d_vollstaendigesbmk.htm

**Bedeutung für QET:** Die Strukturübersicht/das Schriftfeld des Originals nennt Anlage und Ort je Blatt. In QET je Folio
`plant` (ohne `=`) und `locmach` (ohne `+`) setzen; das sind die Werte, die ein Gerät ohne eigene Angabe erbt. QET
vererbt nicht automatisch in elementInformation – das vollständige Kennzeichen beim Verknüpfen selbst bilden
(Folio-Werte, dann Ortsrahmen, dann Elementwerte).

## 5. Erweiterte Referenzkennzeichnung (EN 81346, Produktaspekt)

- Strukturierung nach Funktions- (`=` Anlage), Orts- (`+` Einbauort) und **Produktaspekt** (`-` Produktkennzeichen).
  Option „Erweiterte Referenzkennzeichnung“ (Registerkarte Struktur); ohne sie Verhalten wie EPLAN 2.2.
- Mit Option: Produktkennzeichen wird über Strukturkästen vererbt; Funktionen ohne sichtbares BMK übernehmen die
  übergeordnete Struktur; „>“ wirkt **je Kennzeichnungsblock**; Seitenstruktur wird nicht zur Strukturierung benutzt
  (Tipp der Hilfe: Strukturkasten um die ganze Seite); „Vollständiges BMK“ zeigt je Produktkennzeichen eine Zeile.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_referenzkennzeichnung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_struktur.htm
- „Zur Strukturierung einsetzen“: `-` erscheint immer im vollständigen BMK, Produktkennzeichen in der
  Strukturkennzeichen-Verwaltung, an Abbruchstellen geschachtelt; Trennzeichen `-` oder `.`.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_darstellungerwprjstrukturen.htm

**Bedeutung für QET:** Mehrstufige Produktkennzeichen (`-A1-K3`, Gerät im Gerät) in QET als label komplett
(`A1-K3`) führen; QET hat keinen eigenen Block dafür.

## 6. Seitenname und Seitenkennzeichnung (EN 61355, Dokumentart)

- Vollständiger Seitenname = (Strukturkennzeichen bzw. Objektkennzeichen) + Dokumentenart + `/` + Seitenname, z. B.
  `=AP+ST1/6` oder `-K01.Q01&EFS/1` (Objekt `-K01.Q01`, Dokumentart `EFS`, Seite `1`). Objektkennzeichen nur mit
  erweiterter Referenzkennzeichnung und nur zusammen mit dem Block Dokumentenart; es ist identifizierend und nutzt genau
  einen Aspekt (`=`, `+` oder `-`) oder freien Text. Vorlage `IEC_tpl002.ept` nutzt das.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_objektkennzeichen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_d_seitenstruktur.htm
- Seiten-Einstellungen: Zeichen für Unterseiten (Klein-/Großbuchstaben/Zahlen/keine), Mindestlänge des Seitennamens
  (führende Nullen), Standard-Normblatt, manuelle Seitensortierung (wirkt auf Blättern, Druck, Querverweise).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_d_projekteinstellungen.htm
- Querverweise können das Dokumentart-Trennzeichen unterdrücken.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_einstellungenprojektqvwallgemein.htm

**Bedeutung für QET:** Seitennamen sind nur **innerhalb ihrer Struktur** eindeutig: `=DA/27` und `=SB/27` sind zwei Blätter.
QET-Folio `folio` = Seitenname des Originals (`%F`), die laufende Blattnummer ist `%f` – in Verweisen den Seitennamen
nutzen. Unterseiten (`27.a`) als Folio-Name übernehmen.

## 7. Das BMK: Bestandteile, sichtbar vs. vollständig

- BMK-Block `-` besteht aus **Vorziffer, Kennbuchstabe, Zähler, Trennzeichen, Unterzähler**.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ftechnic_k_betriebsmittelstruktur.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_d_vollstaendigesbmk.htm
- **Sichtbares BMK** = was im Plan steht (editierbar); **vollständiges BMK** = berechnet, nicht editierbar.
  Beispiel: Lampe in Anlage A1, Ort O1 → sichtbar `-H1`, vollständig `=A1+O1-H1`. Wird sichtbar `+PT1-H1` eingetragen,
  ändert sich der Einbauort.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_d_vollstaendigesbmk.htm
- **Verkürzte Anzeige**: Strukturkennzeichen, die schon von der Seite kommen, gehören nicht ins sichtbare BMK; der Prüflauf
  P017011 meldet sie. Beispiel: Seite `=ANL+EBO/1`, sichtbar `-H1` = richtig; `+EBO-H1` ändert das vollständige BMK nicht,
  ist aber unnötig lang. Sichtbar bleibt also nur, was von der Umgebung **abweicht**.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/messages_p_017011.htm
- Welche Blöcke identifizieren, legt die Seitenstruktur fest; wie das BMK im Plan aussieht (Vorziffer, Unterzähler), der
  Dialog „Vollständiges BMK“ – unabhängig von der Projektstruktur.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_k_aufbauvonbm.htm
- Vorzeichen `-` im sichtbaren BMK je Betriebsmittelgruppe ein-/ausschaltbar (`-M1` / `M1`); Großbuchstabenumwandlung;
  „Klammern im BMK ausblenden“ (`W(X1/X2)` → `WX1/X2`).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/numerationgui_h_vorzeichensetzen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/xessettingsgui_d_betriebsmittelbmk.htm
- „Neues BMK ermitteln“ (Strg+N im Feld) vergibt den nächsten freien Zähler.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_h_bmkfestlegenundformatieren.htm

**Bedeutung für QET:** Was im PDF am Gerät steht, ist das **sichtbare** BMK. `-K381` auf einem Blatt `=DC+CA` heißt
vollständig `=DC+CA-K381`; `=SB-A5` auf demselben Blatt heißt `=SB+CA-A5` (Ort von der Seite, Anlage abweichend).
In QET: label = nur BMK-Teil (`K381`), plant/location am Element nur setzen, wenn sie von der Seite **abweichen**
(sonst leer lassen und vom Folio ableiten). Anzeige im Plan wie im Original (verkürzt). Vermutung zu prüfen: QET-
Kennzeichen-Text als Formel `%prefix`/label – fest auf das sichtbare BMK setzen, nicht das vollständige.

## 8. Vererbung: Seite, Strukturkasten, Gerätekasten, „>“

Registerkarte „Vererbung“ (Erweiterte Projektstrukturen):
- **Strukturkästen**: Schaltzeichen ohne eigene Strukturkennzeichen übernehmen die der Umgebung. Geschachtelte Kästen
  `=A2+O2` ⊃ `=A3+O3` ⊃ `=A4+O4`, Sicherung F3: Option an → `=A2.A3.A4+O2.O3.O4-F3`, aus → nur innerster `=A4+O4-F3`.
- **Gerätekästen** (je Typ: allgemeine BM, Klemmenleisten, Stecker, Kabel, Fluid): an → BMK des Kastens wird
  vorangestellt (`=AP+ST1-U1-H1`), aus → `=AP+ST1-H1`.
- **Strukturkennzeichen immer schachteln**: `=.A2-F1` in Umgebung `=A.A1` → `=A.A1.A2-F1`.
- **Struktur zur Seitenstruktur** (EN-Verhalten): Struktur des Schaltzeichens wird **mit der Seite verschachtelt**;
  Motor `=DD+EE` auf Seite `=AB+OC/3` → an: `=AB.DD+OC.EE-M1`, aus: `=DD+EE-M1`. Ausnahme nur mit vorangestelltem **„>“**.
- Strukturen von Geräte-/SPS-Kästen an Abbruchstellen vererben (Verhalten EPLAN 21).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/prjmanagementgui_r_vererbung.htm
- „>“ vor einem Strukturkennzeichen: Kennzeichen ist bezogen auf die oberste Ebene vollständig, übergeordnete werden ignoriert
  (mit erweiterter Referenzkennzeichnung je Block).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_referenzkennzeichnung.htm
- Ergänzungsreihenfolge von außen nach innen: äußerster Kasten ergänzt aus der Seite, der nächste aus dem umgebenden
  Kasten, zuletzt das Schaltzeichen aus dem innersten Kasten. Übergeordnet sind Seite, Strukturkasten, Gerätekasten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_h_geschachtelte.htm

**Bedeutung für QET:** Ohne Kenntnis der Projekteinstellung gilt im PDF meist der Normalfall (Option „Struktur zur
Seitenstruktur“ aus): ein am Gerät angeschriebenes `=SB` **ersetzt** die Anlage der Seite, statt sie zu verschachteln.
Verschachtelte Formen (`=AB.DD`) nur annehmen, wenn das Original sie in Listen/Stückliste so zeigt (Vermutung: dort
immer vollständig ablesbar → Stückliste als Schiedsrichter). SPS-Kanäle in einem SPS-Kasten erben dessen BMK (-A5) –
in QET das Modulkennzeichen an jedem Kanal setzen.

## 9. Strukturkästen (Ortsrahmen)

- Strukturkasten = Gruppierung, **kein Betriebsmittel**, keine Artikelnummer, grafisch wie Gerätekasten, Linienart nach
  Norm. Er kann ein BMK tragen; Schaltzeichen darin stehen zu ihm wie sonst zur Seite und können so andere
  Strukturkennzeichen als die Seite haben. Hierarchie wird bei Verschieben/Größenänderung neu berechnet.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_k_start.htm
- Fehlende Teile seines BMK ergänzt er aus der übergeordneten Einheit. Anders als Gerätekästen übernimmt er **kein BMK von
  links oder oben**; ein Schaltzeichen gehört nur dazu, wenn sein **Einfügepunkt** im Kasten liegt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_k_grundlagen.htm
- Bei Schachtelung wird das Kennzeichen im inneren Kasten immer als Unterkennzeichen gelesen (Punkt darf fehlen).
  Umschalt beim Herausziehen behält das geschachtelte BMK.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/locationbox_h_geschachtelte.htm

**Bedeutung für QET:** Ortsrahmen (`+MASCHINE`, `+BP1`) sind in QET Formen ohne Logik. Für Auswertung und Verknüpfung:
Element gehört zum Rahmen, wenn sein **Einfügepunkt (hotspot/pos)** im Rechteck liegt – nicht, wenn es nur überlappt.
Dann location des Elements = Rahmen-Ort (bzw. Anlage, falls der Rahmen `=` trägt). Geschachtelte Rahmen: innerster gewinnt
(Standardfall Option aus). Klemmen/Kabel am Rahmenrand: Einfügepunkt entscheidet.

## 10. Kennbuchstaben (IEC 81346-2)

- Kennbuchstabensätze je Norm/Land/Kunde, auch mehrstellig (`KT`, `KTP`); projektweise gewählt unter Betriebsmittel >
  Nummerierung (online). Jede Funktionsdefinition hat einen Kennbuchstaben; beim Anlegen übernimmt das BMK ihn fest
  (nicht referenziert, manuell änderbar).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/fctdeflibdataexchangegui_k_kennbuchstaben.htm
- Kennbuchstabe ist normabhängig (IEC 61346, IEC 81346, NFPA, benutzerdefiniert), Zuordnung über Kategorie/Gruppe/
  Funktionsdefinition; Beispiel Drehstrommotor: `M` (IEC), `MTR` (NFPA).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/numerationgui_k_format.htm
- Hauptklassen nach IEC 81346-2:2019 (Normwissen, **nicht aus der EPLAN-Hilfe**, Tabelle in der Norm prüfen): A mehrere
  Zwecke, B Erfassen (Sensor), C Speichern, E Energie liefern (Licht, Wärme), F Schützen, G Erzeugen (Generator, Netzteil
  als Quelle), H Stoff erzeugen, K Signale verarbeiten (Relais, SPS-Baugruppe), M mechanische Energie (Motor, Ventilantrieb),
  P Darstellen (Leuchte, Anzeige, HMI), Q Energie/Stoff schalten (Schütz, Leistungsschalter), R Begrenzen (Widerstand,
  Diode), S Handbetätigung umsetzen (Taster), T Umformen (Trafo, Umrichter), U Halten, W Leiten (Kabel, Schiene),
  X Verbinden (Klemme, Stecker). Zweite Stelle = Unterklasse (z. B. QA, KF, MA).
  Quelle (Norm, Katalogseite): https://standards.iteh.ai/catalog/standards/sist/4f0e397e-5dec-4e77-a55f-fc0f75010c3b/sist-en-iec-81346-2-2019

**Bedeutung für QET:** Der Kennbuchstabe sagt, was ein Gerät **ist** – hilft beim Prüfen, ob ein Symbol stimmt (-Q… ist
schaltend, -K… steuernd, -X… Klemme). Ältere Pläne mischen alte (DIN 40719: K = Schütz, Y = Ventil) und neue Belegung –
Original nicht „korrigieren“, Kennzeichen 1:1 übernehmen.

## 11. Nummerierung online/offline

- **Online**: Neue/kopierte Betriebsmittel (auch aus Makros) bekommen beim Einfügen das nächste BMK; abschaltbar gesamt
  oder je Betriebsmittelgruppe.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/numerationgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/numerationgui_d_nummerierungsformate.htm
- Online-Format aus Kennbuchstabe, Seite, Spalte, Zeile, Zähler, Trennzeichen. Kennbuchstabe Pflicht; kein Element mit Punkt
  (Punkt = Unterebene); Zähler am Ende; Seite/Spalte zwischen Kennbuchstabe und Zähler mit Mindeststellenzahl. Beispiele:
  `-2M3` (Seite 2, Zähler 3), `M0023` (Seite 002, Zähler 3), `-M052` (Spalte 05, Zähler 2). Trennzeichen `.`, `-` oder
  Vorzeichen setzt EPLAN in runde Klammern.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/numerationgui_k_format.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/numerationgui_h_onlineformat.htm
- **Offline**: ganzes Projekt oder Auswahl neu nummerieren, platzierte und nicht platzierte; Formate mit Elementen Seite (S),
  Spalte (P), Zeile (R), Kennbuchstabe (I), Zähler zeilen-/spaltenweise fortlaufend (CL/CC) oder je Seite neu (SL/Sc),
  Trennzeichen; Optionen Vorziffer/Zähler/Buchstabenteil des Zählers beibehalten. Seitenbezogene Nummerierung folgt der
  Seitensortierung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationgui_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/offlinenumerationgui_d_nummerierungsformate.htm
- Startwert/Schrittweite (Fluid-Beispiel P1.1–P1.3 → P1.5, P1.8, P1.11).
  Quelle: https://eplan.help/en-us/Infoportal/Content/Plattform/2.8/Content/htm/ftechnic_h_fluidbetriebsmittel_offline_nummerieren.htm
- Online-Bezeichnungen: Klemmen, Steckerkontakte, Geräteanschlüsse, SPS-Adressen; PE/N/SH-Klemmen optional unverändert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/numerationgui_d_nummerierungsformate.htm

**Bedeutung für QET:** Pfadbezogene BMK wie `-K381` (Seite 38, Spalte/Zähler 1) sind Ergebnis eines Formats „Seite +
Kennbuchstabe/Zähler“ (Vermutung: Format Kennbuchstabe + Seite + Spalte; die Hilfe nennt das Beispiel `-2M4`). Daraus
folgt: Beim Nachbau nie neu nummerieren, die BMK des Originals sind Daten. QET-Autonummerierung für Elemente auf
Nachbauprojekten ausschalten bzw. label explizit setzen. Klemmen-Nummern gehören ins Bauteil.

## 12. Abbildung in QET (Zusammenfassung)

| EPLAN | QET |
|---|---|
| Seite `=A+O/27` | Folio: `plant`=A, `locmach`=O, `folio`=27 |
| sichtbares BMK `-K1` | elementInformation `label`=K1 (Vorzeichen `-` nur in Anzeige/Formel) |
| abweichende Anlage/Ort am Gerät `=SB-K1` | elementInformation `plant`=SB (`location` nur wenn abweichend) |
| vollständiges BMK | selbst berechnen: Element > innerster Ortsrahmen > Folio |
| Strukturkasten | Form (dashdot) + Text; Zugehörigkeit über Einfügepunkt |
| Gerätekasten (SPS-Karte) | Kennzeichen an jedem Kanal/Teilsymbol setzen |
| Funktionstext (beschreibend) | elementInformation `function` |
| Montageort (beschreibend) | Vermutung: `comment` oder `description`, nicht `location` |
| Artikel an Hauptfunktion | `manufacturer`, `manufacturer_reference` nur am Master |

Feldnamen aus `sources/qetinformation.cpp` (label, plant, location, comment, function, description, designation,
manufacturer, manufacturer_reference, auxiliary1–3 …) und Verweisvariablen `%M`/`%LM`/`%F`/`%c`/`%l` aus
`sources/autoNum/assignvariables.cpp` des Forks (gelesen, nicht geändert).

## 13. Ergänzungen (BMK-Übernahme, Nummerierungsformat, Seiten, Suche, Kopieren)

Ausführlich in `betriebsmittel.md`, `nummerierung.md`, `navigatoren-seiten.md`; hier die Punkte, die die Kennzeichnung
direkt betreffen.

- **BMK-Übernahme**: Funktionen **ohne sichtbares BMK** bekommen ihr BMK von der nächsten Funktion in Suchrichtung
  (IEC: von links, gleiche Y-Höhe des Einfügepunkts; alternativ von oben) oder nur vom umgebenden Geräte-/SPS-Kasten;
  Strukturkästen geben ihr BMK nicht nach links/oben weiter, wohl aber an Funktionen in ihrem Inneren. Ihr sichtbares BMK
  bleibt leer – die Suche findet sie über das BMK nicht, Ersetzen ändert sie nicht. Quelle:
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_k_bmkuebernahme.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_k_bmkuebernahmesonder.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/searchandreplacegui_k_prinzip.htm
- **Strukturkasten, sichtbares BMK**: zeigt standardmäßig nur die gegenüber der Umgebung **geänderten** Kennzeichen
  (Kasten `=AP+ST1`, Anlage → ANL2: sichtbar `=ANL2`, vollständig `=ANL2+ST1`). Quelle:
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetaggui_r_locationbox.htm
- **Präzisierung zu Abschnitt 11 (pfadbezogene BMK)**: Seite, Spalte oder Zeile **zwischen** Kennbuchstabe und Zähler
  sind nur mit fester Mindeststellenzahl zulässig (`M0023` = Seite 002 + Zähler 3; `-M052` = Spalte 05 + Zähler 2).
  Ein BMK wie `-K381` ist daher nur mit bekannter Stellenzahl eindeutig zerlegbar (Seite 38 + Zähler 1 oder Seite 3 +
  Spalte 8 + Zähler 1 – Vermutung, am Original prüfen). Punkt, Minus oder Struktur-Vorzeichen als Trennzeichen erzeugen
  Klammern (`-2F(.4.)3`), ausblendbar. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/numerationgui_k_format.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/numerationgui_d_bmkformat.htm
- **Einfügemodus „Nicht verändern“**: sichtbares BMK bleibt, das vollständige übernimmt die Strukturkennzeichen der
  Zielseite, wenn das sichtbare keine eigenen enthält. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/numerationgui_d_einfuegemodus.htm
- **Identifizierende Seiteneigenschaft ändern** (z. B. Anlage der Seite) benennt alle BMK auf der Seite mit derselben
  Anlage mit um. Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_h_eigenschaftenbearbeiten.htm
- **Kopieren in Projekte mit anderer Struktur**: Seiten behalten Blöcke, die im Ziel beschreibend sind, verlieren Blöcke,
  die dort nicht verfügbar sind; Betriebsmittel behalten **immer ihr sichtbares BMK**, das identifizierende Kennzeichen
  enthält nur die Blöcke der Zielstruktur. Reduktion der Seitenstruktur scheitert, wenn doppelte Seiten entstünden (bis
  zu 10 werden gemeldet). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_verhkopinbmubauteilen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_k_hinterg.htm
- **Suche mit Struktur**: BMK-Suchbegriffe mit Struktur müssen vom BMK nach links eine lückenlose Teilstruktur mit
  Vorzeichen enthalten (bei `==`,`=`,`++`,`+`: `+O-K1` ja, `=A-K1` nein), ohne Platzhalter. Quelle:
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/searchandreplacegui_k_prinzip.htm
- **BMK-Aufteilung** (tabellarische Bearbeitung): Standardteile Teil-BMK 1, Teil-BMK 2, Präfix, Kennbuchstabe, Zähler,
  Unterzähler; freie Aufteilung beim Wechsel der Zeichengruppe Ziffer / Sonderzeichen / sonstige (`1V2` → 1|V|2,
  `U1-F1` → U|1|-|F|1). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/functiondatagridgui_k_bmkaufteilung.htm
- **BMK-Syntaxprüfung**: prüft bei aktivierter Elektro-/Fluid-Prüfung das ganze vollständige BMK; Blöcke nach
  Registerkarte Strukturkennzeichen (Zahlen / Großbuchstaben / Groß+Klein, Sonderzeichen, Umlaute, Anlagennummer nur
  Zahlen). Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/devicetagcheckgui_d_einstellungenprojektbmsyntaxueberpruefung.htm
- **Seitenname**: ohne Strukturkennzeichen, Unterseite nach dem **ersten** Punkt von links, max. 32 bzw. 16 Zeichen;
  „Seitenname (identifizierend)“ = identifizierende Kennzeichen + Name, „(vollständig)“ = alle. Quelle:
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_eigenschaften.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/pagebrowsergui_k_start.htm
- **Baumdarstellung** der Navigatoren (identifizierende Kennzeichen standardmäßig sichtbar, Blöcke aus-/einblendbar,
  Seitentyp/Kennbuchstabe zusätzlich): auch in 2.9 dokumentiert (bisher nur 2.8-Quelle zitiert). Quelle:
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/projectstructure_h_kennzeichenausblenden.htm

**Bedeutung für QET:** Verknüpfungsskript braucht für Elemente ohne label eine BMK-Übernahme nach EPLAN-Regel
(Einfügepunkt-Höhe links bzw. umgebender Gerätekasten), bevor Master/Slave gebildet wird. Beim Import/Kopieren immer das
sichtbare BMK des Originals beibehalten und die Struktur vom Ziel-Folio ableiten.

## 14. Quellen

Alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ (sofern nicht anders):
projectstructure_k_hinterg.htm, projectstructure_k_aufbaukennzeichnungsbl.htm, projectstructure_k_hauptkundunterk.htm,
projectstructure_h_prjstrukturdefinieren.htm, projectstructure_h_frhierarchienerstellen.htm,
projectstructure_k_referenzkennzeichnung.htm, projectstructure_k_objektkennzeichen.htm, prjmanagementgui_r_struktur.htm,
prjmanagementgui_d_seitenstruktur.htm, prjmanagementgui_r_vererbung.htm, prjmanagementgui_r_darstellungerwprjstrukturen.htm,
pleditorgui_k_grundlagen.htm, ftechnic_k_betriebsmittelstruktur.htm, devicetaggui_k_aufbauvonbm.htm,
devicetaggui_k_bmeigenschaften.htm, devicetaggui_d_vollstaendigesbmk.htm, devicetaggui_h_bmkfestlegenundformatieren.htm,
messages_p_017011.htm, xessettingsgui_d_betriebsmittelbmk.htm, numerationgui_h_vorzeichensetzen.htm,
numerationgui_k_start.htm, numerationgui_k_format.htm, numerationgui_h_onlineformat.htm,
numerationgui_d_nummerierungsformate.htm, offlinenumerationgui_k_start.htm, offlinenumerationgui_d_nummerierungsformate.htm,
locationbox_k_start.htm, locationbox_k_grundlagen.htm, locationbox_h_geschachtelte.htm, pagebrowsergui_d_projekteinstellungen.htm,
xessettingsgui_h_qvwanzeigeoptimieren.htm, xessettingsgui_d_einstellungenprojektqvwallgemein.htm.

Ergänzt (Abschnitt 13): devicetaggui_k_bmkuebernahme.htm, devicetaggui_k_bmkuebernahmesonder.htm,
devicetaggui_r_locationbox.htm, numerationgui_d_bmkformat.htm, numerationgui_d_einfuegemodus.htm,
pagebrowsergui_h_eigenschaftenbearbeiten.htm, pagebrowsergui_k_eigenschaften.htm, pagebrowsergui_k_start.htm,
projectstructure_k_verhkopinbmubauteilen.htm, projectstructure_k_start.htm, projectstructure_h_kennzeichenausblenden.htm,
searchandreplacegui_k_prinzip.htm, functiondatagridgui_k_bmkaufteilung.htm,
devicetagcheckgui_d_einstellungenprojektbmsyntaxueberpruefung.htm.

Weitere:
- https://www.eplan.help/de-de/Infoportal/Content/Plattform/2026/Content/htm/fctdeflibdataexchangegui_k_kennbuchstaben.htm
- https://eplan.help/en-us/Infoportal/Content/Plattform/2.8/Content/htm/projectstructure_h_kennzeichenausblenden.htm
- https://eplan.help/en-us/Infoportal/Content/Plattform/2.8/Content/htm/ftechnic_h_fluidbetriebsmittel_offline_nummerieren.htm
- https://standards.iteh.ai/catalog/standards/sist/4f0e397e-5dec-4e77-a55f-fc0f75010c3b/sist-en-iec-81346-2-2019 (Norm, nur Katalog)
- QET-Fork (gelesen): `sources/qetinformation.cpp`, `sources/autoNum/assignvariables.cpp`
