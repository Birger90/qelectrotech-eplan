# EPLAN: Verbindungen, Netze, netzbasierte Verdrahtung, Potentiale

Wie EPLAN (Plattform 2.9, abgeglichen mit 2026) aus Autoconnect-Linien **Verbindungen** macht, woher eine
Verbindung ihre Daten erbt (Projekt → Potential → Signal → Netz → Verbindungsdefinitionspunkt), was Netze und
netzbasierte Verbindungen sind und wie Potentiale/Signale definiert und verfolgt werden. Ergänzt
[kabel.md](kabel.md) (Kabeladern, Quelle/Ziel bei Kabeln, Vererbungstabelle) und
[querverweise.md](querverweise.md) §11–12 (Potential- und Verbindungsdefinitionspunkte, Abbruchstellen). Grafik der
Verbindungssymbole: [schaltplan-editor.md](schaltplan-editor.md). In eigenen Worten; jede Aussage mit Quelle.

## Inhalt

1. Begriffe: Autoconnect-Linie, Verbindung, Teilverbindung, Netz, Potential, Signal
2. Entstehen und Aktualisieren von Verbindungen
3. Quelle und Ziel einer Verbindung
4. Verbindungseigenschaften und ihre Vererbung
5. Verbindungsdefinitionspunkt (Details)
6. Verwaltungsarten, nicht platzierte Verbindungen, Abbuchen, Platzieren
7. Netze und netzbasierte Verbindungen (Net Based Wiring)
8. Potentiale und Signale
9. Potentiale-Navigator, Potential-/Signal-/Netzverfolgung
10. Verbindungen-Navigator
11. Projekteinstellungen Verbindungen (Allgemein, Eigenschaften, Farben, Querschnitte, Verlegung)
12. Smart-Connecting
13. Einpolige Verbindungen (Verweis)
14. Quellen

---

## 1. Begriffe

- **Verbindung**: Strecke zwischen genau **zwei Anschlüssen**, über die Energie, Information oder Medium von einer
  Quelle zu einem Ziel läuft. Weil Verbindungen über viele Seiten gehen können, werden sie **offline erzeugt**, nicht
  online aktualisiert.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_start.htm
- **Autoconnect-Linie**: zunächst rein grafisch; erst beim Erzeugen/Aktualisieren der Verbindungen (z. B. Seite
  öffnen) entsteht eine Verbindung mit Daten. Eigenschaften einer Verbindung sind daher **nicht** durch Anklicken der
  Linie erreichbar, nur im Verbindungen-Navigator oder am Verbindungsdefinitionspunkt. Seitenübergreifende
  Verbindungen werden erst beim Schließen der Seite aus den **Teilverbindungen** zusammengeführt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_prinzipverbindungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm
- Arbeitsweise „vom Groben zum Feinen“: erst nur „diese Anschlüsse sind verbunden“, später Führung und Material.
  Verbindungsart (Kabel, Draht, Brücke, Drahtbrücke, Stegbrücke, Sammelschiene) ist Verbindungseigenschaft
  (Funktionsdefinition).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm
- **Potential** = elektrische Ebene (z. B. L1), **Signal** = benannter Teil eines Potentials mit anderem Namen und
  eigenen Daten (zur Orientierung, z. B. welches Signal hinter welcher Sicherung abgeht), **Netz** = Menge direkt
  verbundener Anschlüsse bis zur nächsten netztrennenden Funktion (Details §7/§8).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_k_prinzippotenziale.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_k_prinzip.htm

**Bedeutung für QET:** QET-Leiter = EPLAN-Verbindung (zwei Anschlüsse). Ein EPLAN-T-Abzweig ergibt **zwei
Verbindungen**, nicht eine Leitung mit Abzweig – also in QET zwei Leiter, deren Verlauf der Zielreihenfolge folgt
(schaltplan-editor.md §4). QET-„Potential“ (alle über Leiter und Reports verbundenen Leiter) entspricht in EPLAN eher
dem **Netz** bzw. dem Potential über Abbruchstellen; EPLAN-Potentiale laufen zusätzlich durch Funktionen mit
Potentialweiterleitung (Klemmen, Sammelschienen) – QET verbindet über Klemmen-Elemente nur, wenn deren Anschlüsse
intern verbunden sind (Vermutung, je Element prüfen).

## 2. Entstehen und Aktualisieren von Verbindungen

- Wege, eine Verbindung zu definieren: Verbindungsdefinitionspunkt (manuell oder durch Verbindungsnummerierung),
  Schnitt mit einer **Kabeldefinitionslinie**, automatisches Kabelerzeugen (Vergleich von Quelle/Ziel), Vorbelegung
  vom Potential/Projekt. Die Anschlussart wird beim Entstehen von den Funktionsanschlüssen übernommen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm
- Status: `#` in der Statusleiste = Projekt hat nicht aktuelle Verbindungen, `*` = auch die offene Seite.
  Benutzereinstellung „Beim Seitenwechsel Verbindungen aktualisieren“ (Standard an): auf der bearbeiteten Seite nach
  jedem Schritt, beim Seitenwechsel auch auf Seiten, die über Abbruchstellen beteiligt sind. Manuell `Projektdaten >
  Verbindungen > Aktualisieren` (Standard projektweit); `[Umschalt]` = alle Verbindungen neu erzeugen (übernimmt auch
  geänderte Projektvorgaben).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_verbindungenbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_verbindungseigenschaftendefinieren.htm
- Verschiebt man ein Schaltzeichen so, dass seine Anschlüsse kein Gegenüber mehr haben, wird die Verbindung
  gelöscht, Verbindungsdefinitionspunkte bleiben. Wird eine Verbindung mit Definitionspunkt durch ein neues Zeichen
  geteilt, behält nur die Seite mit dem Punkt die Daten; die andere Hälfte ist neu und ohne Sonderdaten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_verbindungenbearbeiten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm
- Potentialdaten werden erst bei der **projektweiten** Aktualisierung an die Verbindungen geschrieben; beim
  Seitenwechsel nur Verbindungsdaten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_h_potenzialverfolgung.htm
- Mit Bearbeitungsbereichen aktualisiert EPLAN nur Verbindungen des Bereichs (schaltplan-editor.md §20).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/workingsection_k_prinzip.htm

**Bedeutung für QET:** Ein EPLAN-PDF zeigt den Stand der letzten Aktualisierung. Weicht eine Liste (Verbindungsliste,
Klemmenplan) vom Schaltbild ab, waren die Verbindungen evtl. nicht aktuell → Schaltbild führt (auswertungen.md).

## 3. Quelle und Ziel einer Verbindung

- Standard (seit 1.9 SP1): Vergleich der **BMK** der angeschlossenen Funktionen; das „kleinere“ wird Quelle
  (Reihenfolge der Strukturkennzeichen in der Strukturkennzeichen-Verwaltung, dann Kennbuchstabe/Zähler/
  Anschlussbezeichnung: `EB3+ET4-K1:2` < `-X1:2`; `+ET1-X1:10` Quelle vor `+ET3-X1:4`). Mehrere Verbindungen zwischen
  denselben zwei Funktionen haben dieselbe Richtung. Option „aus der Platzierung ermitteln“ = altes grafisches
  Verhalten; **Kabelverbindungen** haben eigene Regeln (kabel.md §6).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_prinzipverbindungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_d_einstellungenallgemein.htm
- **Korrektur (Widerspruch in der Hilfe):** Die Seite „Verbindungseigenschaften“ sagt noch, Quelle und Ziel würden
  „durch die Position im Schaltplan bestimmt“. Das ist nur mit der genannten Option richtig; Standard ist der
  BMK-Vergleich.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm
- Verbindungssymbole (Winkel, T-Stück) sind **keine Ziele**; Ausnahme runde Knotenpunkte bei Sammelschienen und
  Abschirmungen; einseitig aufgelegte Abschirmung ist selbst Ziel.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm
- „Quelle und Ziel tauschen“ tauscht auch alle quell-/zielabhängigen Eigenschaften (u. a. 31098/31099 Doppelhülse,
  31114/31115 Verlegerichtung, 31053/31054 Hülsenquerschnitt, 31080/31083 Anschlusslänge, 31055/31056 Absetzlänge,
  31051/31052 Verbindungsende-Behandlung, 31096/31097 Anschlussmaß, 31148–31153 Verlegeweg-Koordinaten) und dreht
  die Verlegewege (31095, 20237) um; über alle Darstellungsarten. Platzierte Verbindungen: Richtung im Dialog nicht
  änderbar (nur bei Teilverbindungen).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_prinzipverbindungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm

**Bedeutung für QET:** Für Verbindungslisten/Klemmenpläne im Nachbau „von/nach“ nach dem BMK-Vergleich berechnen
(vollständiges Kennzeichen, Strukturreihenfolge aus dem Projekt, dann Kennbuchstabe alphabetisch, dann Zähler,
dann Anschluss), nicht nach Zeichenrichtung des QET-Leiters. QET speichert terminal1/terminal2 willkürlich.

## 4. Verbindungseigenschaften und ihre Vererbung

- Vererbungsreihenfolge (spätere Stufe gewinnt): **Anschlüsse** (Anschlussart + Potentialtyp) → **Projekt** →
  **Ebene** → **Klemmenziele** → **Potential** → Rohrleitungsdefinition → **Signal** → **Netz** → **Brückenerzeugung**
  → **Abschirmung** → **Verbindungsdefinitionspunkt**. Steht am Quellanschluss „Undefiniert“, gilt der Zielanschluss.
  (Ausführlich mit Beispieltabelle in kabel.md.)
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm
- Nur über einen **Verbindungsdefinitionspunkt** (nicht über Potentialdefinitionspunkt) bekommt eine Verbindung:
  BMK, Funktionsdefinition, „Quelle und Ziel vertauschen“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_k_prinzippotenziale.htm
- Weitere Eigenschaften: mehrere **Artikel** je Verbindung, auch an Quelle/Ziel (Kabelschuhe); **BMK** (v. a. für
  Kabelverbindungen); **Verbindungsbezeichnung** (manuell oder Nummerierung; `Strg+N` = nächste freie);
  **Funktionsdefinition** aus der Anschlussart oder vom Definitionspunkt (Draht, Brücke, Kabelverbindung …).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_eigenschaftenbearbeiten.htm
- **Verbindungsfarbe (mehrsprachig)**: aus Farbe/Nummer über Tabelle Farbcode (IEC 60757) → Farbname, **groß-/
  kleinschreibungsabhängig** (`BK` → Schwarz, `bk` bleibt `bk`); Tabelle auch für Bauraumfarbe.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_d_einstellungenaderfarben.htm
- Einheiten: Länge mm, cm, dm, m, km, ft, yd, inch, mil; Querschnitt „von Projekt“, mm²/qmm, AWG, mm, KCM, MCM,
  inch, µm, kcmil, µm², Undefiniert. Alternativer Querschnitt (z. B. AWG neben mm²) manuell, unabhängig vom Artikel.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_einheitenverbindungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_r_projekteinstellungenelektrotechnik.htm
- Linienart einer Verbindung: einstellbar für Projekt, Potential oder einzelne Verbindung (Registerkarte
  Verbindungsgrafik); Potentialtypen L, N, PE/PEN, +, M, -, SH liegen auf eigenen Ebenen (z. B. **EPLAN540**
  …Autoconnecting.Potenzialtyp [L]), Standard wie EPLAN311 (allgemeine Autoconnect-Linien); Ebenenfarbe wirkt nur,
  wenn keine andere Verbindungsgrafik gesetzt ist.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_k_prinzippotenziale.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_prinzipverbindungen.htm

**Bedeutung für QET:** Leiterdaten (Farbe, Querschnitt, Typ) im Nachbau **nicht** nur dort setzen, wo im PDF ein
Definitionspunkt steht: EPLAN erbt sie vom Projekt/Potential; fehlt am Leiter eine Angabe, gilt die Projektvorgabe
(im Original aus Verbindungsliste ablesen). Farbe genau so schreiben wie im Original (`BK`, `GNYE`), Groß-/
Kleinschreibung zählt in EPLAN. Farbige Leitungen im PDF (rot/blau) kommen meist aus der Potentialtyp-Ebene oder der
Potential-Verbindungsgrafik → Hinweis auf den Potentialtyp.

## 5. Verbindungsdefinitionspunkt (Details)

- Weist den darunter durchlaufenden Verbindungen Eigenschaften zu; hat Vorrang vor Potential und Projekt. Einpolig
  oder bei Strängen wirkt einer auf **mehrere** Verbindungen. Mehrere je Verbindung erlaubt (ergänzen/widersprechen,
  Prüflauf meldet). Er kann fremde (von der Verbindung geerbte) Eigenschaften nur anzeigen; manche Werte gibt es dann
  doppelt (direkt/indirekt), indirekte erst nach Aktualisieren gefüllt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_vdp.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm
- **Auswertungen immer über Verbindungen**, nicht über Definitionspunkte: ein Punkt mit Artikel + Länge auf zwei
  Verbindungen ergibt zwei Listeneinträge.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_vdp.htm
- Bezeichnung aus nur `?` = undefiniert → die Verbindung nimmt die Bezeichnung des grafisch ersten Punkts mit
  „echter“ Bezeichnung; sonst des ersten nicht leeren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_vdp.htm
- Automatisch gesetzt durch Verbindungsnummerierung bei seitenübergreifenden oder >3-teiligen Verbindungen (Anfang und
  Ende, jeweils auf dem ersten/letzten Teilstück mit nur einer Verbindung) und beim Kabelerzeugen. Mehrere auf einmal:
  Maus über mehrere Autoconnect-Linien ziehen; eine Kabeldefinitionslinie setzt Punkte auf allen gekreuzten Verbindungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_vdp.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_definitionspunktezeichnen.htm
- Anzeige: Eigenschaften und das Symbol selbst einzeln unsichtbar schaltbar; Eigenschaftsanordnung speicherbar als
  Standard getrennt für normale/nummerierungsgesetzte Punkte und für Punkte aus Kabeldefinition/Abschirmung (je
  Symbolvariante).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm
- Daten, die außerhalb des Editors (Navigator, externe Bearbeitung) an eine Verbindung geschrieben werden, gehen an
  alle zugehörigen Definitionspunkte; fehlt einer, setzt EPLAN automatisch einen an eindeutiger Stelle. Übergreifendes
  Bearbeiten nur im Navigator, Blockbearbeitung mehrerer Punkte möglich.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_arbeitsweiseverbindungen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_vdpeigenschaften.htm
- In Makros werden nur Definitionspunkte gespeichert, nicht Verbindungen; beim Einfügen neu erzeugt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_vdp.htm

**Bedeutung für QET:** Eine Verbindungsbezeichnung (Drahtnummer) im EPLAN-PDF steht oft nur an Anfang und Ende einer
langen Verbindung (automatische Punkte). In QET gehört sie an **alle** Leiter dieser Verbindung (QET-Leitertext
gilt ohnehin fürs QET-Potential). Ein Definitionspunkt (Schrägstrich) mit Daten gehört genau zu der Verbindung, die er
schneidet – im Fork `ConductorDefinitionMark` an genau diesem Leiter (kabel.md §9; querverweise.md §12).

## 6. Verwaltungsarten, nicht platzierte Verbindungen, Abbuchen, Platzieren

- **Verwaltungsart**: Platziert, Nicht-platziert, **Netzbasiert** (am Netzdefinitionspunkt), **Brücke (automatisch)**
  (aus manuellen Stegbrücken an Klemmen/Steckern oder Brückenbildung zwischen Teilklemmen), **Direkt (automatisch)**
  (Stift ↔ Buchse über Direktanschlüsse), **Allpolig (automatisch)** (einpoliger Verbindungsdefinitionspunkt,
  „Geräte verschalten“), Nicht-verlegt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm
- Nicht platzierte Verbindungen (zwischen nicht platzierten oder gemischten Anschlüssen) werden **genauso
  ausgewertet** wie platzierte. Entstehen z. B. durch gelöschte Platzierung oder „Geräte verschalten“. Mehrere
  Verbindungen zwischen gleichen Anschlüssen erlaubt (**Querschnittverdopplung**). Ohne Ziele: bleiben bestehen,
  erscheinen in keiner Auswertung, reparierbar oder beim Komprimieren löschbar. Potential wird beim Aktualisieren
  ermittelt, nicht eingebbar.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_npv.htm
- **Abbuchen**: Verbindungsdefinitionspunkt auf eine platzierte Verbindung, dann vorhandene (nicht) platzierte Verbindung
  auswählen – Reihenfolge der Liste: beide Ziele gleich, ein Ziel gleich, ohne Ziele, Rest. Die nicht platzierte wird
  danach **immer gelöscht**. Entsteht im Editor eine Verbindung, die schon nicht platziert existiert, wird **nicht**
  automatisch abgebucht (Doppelung bis zum Abbuchen). Nur bei eindeutiger Zuordnung des Punkts; nicht bei
  Zugehörigkeit „Kabel“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_abbuchen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_npv.htm
- **Platzieren** einer Verbindung = Definitionspunkt auf Autoconnect-Linie setzen; **Platzierung löschen** nur über den
  Definitionspunkt → es entsteht eine nicht platzierte Verbindung (mit den Sonderdaten) neben der weiter bestehenden
  platzierten. Werden Schaltzeichen gelöscht, bleiben ihre Verbindungen nur als nicht platzierte erhalten, wenn auch
  die Definitionspunkte mit gelöscht wurden.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_verbindungenplatzieren.htm

**Bedeutung für QET:** QET kennt nur platzierte Leiter. Verbindungen, die im Original nur in Listen vorkommen (nicht
platziert, „Allpolig (automatisch)“, automatische Brücken), lassen sich nicht als Leiter zeichnen → in Klemmen-/
Verbindungslisten des Nachbaus als Daten führen (Vermutung: Brücken über Klemmen-Brückenfunktion des Forks, falls
vorhanden; sonst Liste). Querschnittverdopplung (zwei Drähte parallel zwischen denselben Anschlüssen) = zwei QET-
Leiter zwischen denselben Anschlüssen.

## 7. Netze und netzbasierte Verbindungen (Net Based Wiring)

- Standard: Verbindungsreihenfolge entsteht beim Zeichnen über Verbindungssymbole mit Zielfestlegung. Netzbasiert:
  Plan zeichnen, **ohne** die Reihenfolge festzulegen; EPLAN merkt den Verlauf intern unabhängig von der Grafik.
  Mischbetrieb im Projekt und sogar im selben Netz erlaubt; Verbindungssymbole werden dann **immer als Punkt**
  gezeichnet; Nummerierung und Abbruchstellen funktionieren; Lizenz „Net Based Wiring“ (sonst kein Kontrollkästchen
  „Ziele festlegen“).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_k_start.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_k_prinzip.htm
- **Netzdefinitionspunkt**: Netzname/-eigenschaften (Registerkarte Netzdefinition) und Netzverbindungen (welche
  Anschlüsse verbunden sind). Anschlüsse über eine im Netz eindeutige **Anschluss-ID** (Eigenschaft „ID für netzbasierte
  Verbindungen“, automatisch, änderbar, wird mitkopiert). Verbindungen sichtbar im Navigator erst nach Aktualisieren.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_h_ndpzeichnen.htm
- **Verbindungsketten**: Kontextmenü „Verbindungskette erstellen“, Anschlüsse nacheinander anklicken, „beenden“;
  ein Netz ist unvollständig, bis alle Anschlüsse verbunden und alle Ketten untereinander verbunden sind.
  „Zielreihenfolge ändern“ je Anschluss.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_h_verbindungsketten.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_h_zielreihenfolgeaendern.htm
- **Netzgrenzen**: Netz endet an potential-, signal- oder netztrennenden Funktionen (Standard: die meisten Funktionen
  sind netztrennend). Eigenschaft **Netzverbindend** führt es weiter (z. B. Klemme -X100, dann H10 und H20 im selben
  Netz); Potentialanschlüsse sind immer netzverbindend.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_k_prinzip.htm
- Ändern: Umbenennen von Funktionen schadet nicht (IDs bleiben); gelöschte Funktion → Verbindungen „ungültig“ (manuell
  oder beim Komprimieren löschen); neue Funktion mit gleichen IDs übernimmt die Verbindungen. Kopieren mit Punkt
  kopiert Verbindungen; Teilnetz → ungültige Enden; Einfügen auf netztrennende Funktion teilt, auf Verbindungssymbol
  erweitert das Netz. Punkt löschen = alle Netzverbindungen weg; Punkt in anderes Netz schieben = Wiederherstellen über
  gleiche IDs.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_k_netzbearbeiten.htm
- **Kabel**: Kabeldefinitionslinien wirken nicht auf netzbasierte Verbindungen bzw. auf Verbindungen, die ein
  Verbindungssymbol ohne Zielfestlegung enthalten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_k_prinzip.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_k_start.htm
- Auswertung: **Netzindex**, **Verbindungskettenindex**, **Verbindungsindex**; Filter Verwaltungsart = „Netzbasiert“.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/netbasedwires_k_prinzip.htm
- Einstellungen (Projekt > Verbindungen > Allgemein): „Stegbrückenanschlüsse im Netzdefinitionspunkt verwalten“;
  „Alle Netzverbindungen im Netzdefinitionspunkt verwalten“ (dann auch direkte Verbindungen zwischen netzverbindenden
  Funktionen manuell; ein T-Stück mit Zielfestlegung legt trotzdem fest). **Netzbasiert und Brückenoptimierung von
  Teilklemmen schließen sich aus.** Benutzereinstellung: „Ziele festlegen“ beim Einfügen eines Netzdefinitionspunkts
  automatisch abschalten.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_d_einstellungenallgemein.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenverbindungssymbole.htm
- Netze ohne Netznamen oder ohne Netzdefinition erscheinen nicht im Potentiale-Navigator.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_d_potenziale.htm

**Bedeutung für QET:** Ist das Original netzbasiert (alle Abzweige als Punkte, kein Schräg-T), ist die echte
Verdrahtungsreihenfolge **nicht aus der Grafik** ablesbar; sie steht nur in der Verbindungsliste/dem Klemmenplan des
Originals. Für den QET-Nachbau dann die Leiter so führen, wie die Verbindungsliste die Paare nennt (Kette A→B→C), und
die Punkte (`bod`) nur optisch setzen. Netzverbindende Klemme = Klemme ist Teil des Netzes (für Verdrahtungslisten).

## 8. Potentiale und Signale

- Ein Potential oder Signal wird durch **Potentialdefinitionspunkt** oder **Potentialanschluss** (gleichberechtigt)
  definiert; die Daten gelten fürs **ganze** Potential/Signal: Potentialname, Signalname, Potentialtyp, Wert,
  Frequenz, mögliche Gegenpotentiale (z. B. L1 230/400 V 50 Hz, Gegenpotentiale L2, L3, N → Verbraucher prüfbar).
  Signalname muss vom Potentialnamen abweichen. Nicht in Projektstruktur, nicht geschachtelt.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_k_prinzippotenziale.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_k_start.htm
- **Potentialtyp** kommt nur vom Potential oder von den Anschlüssen (Potential hat Vorrang); Widerspruch Verbindung ↔
  Anschluss = Fehler; „Undefiniert“ passt zu allem; widersprechen sich Anschlüsse, gewinnt der grafisch erste (Fehler).
- Mehrere Punkte **gleichen** Namens erlaubt; widersprüchliche Eigenschaften = Fehler (auch auf verschiedenen
  Potentialen). Punkte **verschiedener** Namen auf demselben Potential ergänzen sich. Bei Inkonsistenz gewinnt der
  **grafisch erste** (Normblatt-Suchrichtung, DIN links oben → rechts unten), seitenübergreifend erst bei der
  Vollverbindung entschieden.
- Am Definitionspunkt standardmäßig der Potentialname sichtbar (weitere platzierbar), am Potentialanschluss nur der
  Potentialname. Der Potentialanschluss ist eine **Funktion** mit Logik (Weiterleitung bei mehreren Anschlusspunkten);
  sein Potentialtyp gilt auch für seine Anschlusspunkte.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_k_prinzippotenziale.htm
- Definitionspunkte auf mehrere Linien auf einmal: Maus über Autoconnect-Linien ziehen. Eigenschaften gehen an die
  durchlaufende bzw. angeschlossene Verbindung und an **alle Verbindungen des Potentials**.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_h_pdpzeichnen.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_pdpeigenschaften.htm
- Potentialanschlüsse sind nicht in Kästen schachtelbar (schaltplan-editor.md §7).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/blackbox_k_schachteln.htm

**Bedeutung für QET:** Potentialname an Potentialanschluss/-definitionspunkt (z. B. `24V`, `0V`, `L1`) im Original =
Name des QET-Potentials → Leiterfunktion bzw. Leitertext aller Leiter dieses Potentials setzen (wirkt in QET ohnehin
potentialweit). Signalname (abweichend vom Potential, z. B. `L1.1` hinter einer Sicherung) markiert in EPLAN einen
Abschnitt **nach** einer signaltrennenden Funktion – in QET ist das ein eigenes Potential (hinter der Sicherung
getrennt) und bekommt den Signalnamen. Gegenpotentiale geben eine Prüfliste für „Verbraucher zwischen L und N/M“.

## 9. Potentiale-Navigator, Potential-/Signal-/Netzverfolgung

- Navigator zeigt Potentiale, Signale, Netze und platzierte Potential-/Netzdefinitionspunkte (Baum/Liste: Potential-,
  Signal-, Netzname, Platzierung; zusätzlich Wert, Frequenz); Vorschau der Platzierung; Platzieren (Rücktaste =
  Platzierdialog), Gehe zu, Tabellarisch bearbeiten. Rohrleitungen-Navigator analog für RI-Fließbilder.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_d_potenziale.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_d_rohrleitungen.htm
- `Ansicht > Potenzialverfolgung` / `Signalverfolgung` / `Netzverfolgung`: Klick auf Verbindung färbt das ganze
  Potential/Signal/Netz (auch seitenübergreifend) in der Benutzerfarbe „Potenzial verfolgen“; nur bei aktuellen
  Verbindungen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/potentialbrowsergui_h_potenzialverfolgung.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenbenutzerallgemein.htm

**Bedeutung für QET:** Prüfwerkzeug für den Nachbau: QET-Potentialhervorhebung bzw. `qet_continuity` entspricht der
Netz-/Potentialverfolgung; Ergebnis mit dem Original vergleichen (gleicher Potentialname auf allen Leitern).

## 10. Verbindungen-Navigator

- Baum nach Strukturkennzeichen, jede Verbindung **doppelt** (unter Quell- und unter Ziel-BMK); Liste standardmäßig
  Funktionsdefinition, Quelle, Ziel, Farbe/Nummer, Querschnitt, Potentialname, erste Artikelnummer. Mehrstockklemme nur
  als 3D-Platzierung → BMK + Etage in eckigen Klammern (auch in Auswertungen).
- Kontextmenü: **Neu** (nicht platzierte Verbindung mit Funktionsdefinition, Quelle/Ziel angeben), Löschen (nur nicht
  platzierte), Platzieren, **Zuweisen** (Funktionsdaten auf ein Schaltzeichen übertragen; Funktion braucht ≥ so viele
  Anschlüsse), Verlegen/Bündeln (Bauraum, Topologie), Gehe zu (Grafik-Quelle/-Ziel), Tabellarisch bearbeiten.
  Leereinträge in Quelle/Ziel = unvollständige Verbindung.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_d_verbindungen.htm
- Verbindungsbezeichnungen auf Umfang übertragen: bei manueller Änderung optional auf alle Verbindungen des
  Potentials/Signals/Netzes (berücksichtigt Nummerierungsfilter, nicht bei Nummerierung selbst).
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/gedviewer_d_einstellungenverbindungssymbole.htm

## 11. Projekteinstellungen Verbindungen

- **Allgemein**: Quelle/Ziel aus Platzierung (§3), Stegbrücken-/alle Netzverbindungen im Netzdefinitionspunkt (§7),
  Topologie-Verlegeweg (ID 20237) mit/ohne nicht platzierte Strecken bzw. Verlegepunkte mit Länge.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_d_einstellungenallgemein.htm
- **Eigenschaften > Elektrotechnik**: Vorgaben Querschnitt + Einheit, alternativer Querschnitt, Länge (m) + Einheit,
  Typbezeichnung (wird vom Artikel überschrieben), Verbindungsende-Behandlung/Hülsenquerschnitt/Absetzlänge je Quelle/
  Ziel (für Konfektioniermaschinen), Artikelnummer (Empfehlung: Artikel ohne Länge/Preis; Außendurchmesser für
  Füllgrad). **Gilt nur für Einzelverbindungen**, nicht für Kabelverbindungen (die erben vom Kabelartikel).
  Fluid-Registerkarten analog.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_r_projekteinstellungenelektrotechnik.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_r_projekteinstellungenhydraulik.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_k_verbindungseigenschaften.htm
- **Verbindungsfarben** (Farbcode IEC 60757 ↔ Farbname, übersetzbar, Bauraumfarbe) und **Querschnitte/Durchmesser**
  (Auswahlliste mit Einheit) sind Projekttabellen.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_d_einstellungenaderfarben.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_d_einstellungenquerschnitt.htm
- Verlegeverbindungen (Pro Panel, nur überflogen): globaler Längenzuschlag absolut/prozentual je Gewerk;
  Verbindungsende-Behandlung je Anschlussausführung, Doppelhülsen (immer / wenn vorgeschrieben = Standard / nie);
  Verbindungsfilter für Streckennetze (IDs 36026, 20247; nur ohne Verlegeweg-Vorgabe 31094); Verlegung: Toleranz
  Kanäle 5 mm, Zuschlag Durchbruch 2 mm, Biegeradius, Füllgrad rot ≥ 80 %/gelb ≥ 70 %, diagonal ab Querschnitt,
  Länge ohne Anschlussbild nach Anschlussposition bzw. maximalem Verlegeweg, Verlegebereich diagonal (Standard) oder
  orthogonal.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_r_einstellungenglobalerlaengenzuschlag.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_r_einstellungenverbindungsende.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_r_einstellungenverbindungsfilter.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionsettingsgui_r_einstellungenverlegung.htm

**Bedeutung für QET:** Projektvorgaben (z. B. „alle Drähte 0,75 mm² BK, H05V-K“) aus Deckblatt/Verbindungsliste des
Originals als Standard-Leitereigenschaften des QET-Projekts setzen (Vermutung: über die Projekt-Standardleiter-
eigenschaften des Forks), Abweichungen nur an den betroffenen Leitern.

## 12. Smart-Connecting

- `Optionen > Smart-Connecting`: verschobene Symbole behalten ihre Autoconnect-Verbindungen (Linien werden
  nachgeführt). Ausschneiden + Einfügen auf anderer Seite setzt auf Quell- **und** Zielseite automatisch
  **Abbruchstellen** mit Querverweisen (Einfügemodus „Nicht verändern“), nummeriert nach Projekteinstellung
  Smart-Connecting (Schema je Gewerk und Anschlussrichtung, Abstand 4 mm, Unterbrechung am überflüssigen Anschluss,
  BMK-Zähler mit Mindeststellen, Füllzeichen, alphanumerischem Zeichenvorrat wie Excel-Spalten: A, K, S, AA, AK …).
  Nicht projektübergreifend sinnvoll. `F` springt zum Gegenstück.
  Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/connectionbrowsergui_h_smartconnecting.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_d_einstellsmartconnecting.htm,
  https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/interruptionpointgui_d_einstellsmartabbruchstellen.htm

**Bedeutung für QET:** Abbruchstellen mit fortlaufenden Kurznamen (A, B, …, AA) im Original stammen oft vom
Smart-Connecting: sie sind 1:1-Paare (Kette) zwischen genau zwei Seiten → in QET je ein Folio-Report-Paar.

## 13. Einpolige Verbindungen (Verweis)

Einpolige Verbindungsdefinitionspunkte („Anzahl der Funktionen“ `3+PE`), „Geräte verschalten“ (Verwaltungsart
Allpolig (automatisch)) und Stränge: siehe schaltplan-editor.md §15.
Quelle: https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/singlepole_k_verbindungen.htm

## 14. Quellen

Alle unter https://www.eplan.help/de-de/Infoportal/Content/Plattform/2.9/Content/htm/ :

- Verbindungen: connectionbrowsergui_k_start.htm, connectionbrowsergui_k_prinzipverbindungen.htm,
  connectionbrowsergui_k_arbeitsweiseverbindungen.htm, connectionbrowsergui_k_verbindungseigenschaften.htm,
  connectionbrowsergui_k_vdp.htm, connectionbrowsergui_k_npv.htm, connectionbrowsergui_k_einheitenverbindungen.htm,
  connectionbrowsergui_h_abbuchen.htm, connectionbrowsergui_h_definitionspunktezeichnen.htm,
  connectionbrowsergui_h_eigenschaftenbearbeiten.htm, connectionbrowsergui_h_pdpeigenschaften.htm,
  connectionbrowsergui_h_smartconnecting.htm, connectionbrowsergui_h_vdpeigenschaften.htm,
  connectionbrowsergui_h_verbindungenbearbeiten.htm, connectionbrowsergui_h_verbindungenplatzieren.htm,
  connectionbrowsergui_h_verbindungseigenschaftendefinieren.htm, connectionbrowsergui_d_verbindungen.htm.
- Verbindungseinstellungen: connectionsettingsgui_d_einstellungenallgemein.htm,
  connectionsettingsgui_d_einstellungenaderfarben.htm, connectionsettingsgui_d_einstellungenquerschnitt.htm,
  connectionsettingsgui_r_projekteinstellungenelektrotechnik.htm, connectionsettingsgui_r_projekteinstellungenhydraulik.htm;
  überflogen (Verlegung/Pro Panel): connectionsettingsgui_r_einstellungenglobalerlaengenzuschlag.htm,
  connectionsettingsgui_r_einstellungenverbindungsende.htm, connectionsettingsgui_r_einstellungenverbindungsfilter.htm,
  connectionsettingsgui_r_einstellungenverlegung.htm.
- Netzbasiert: netbasedwires_k_start.htm, netbasedwires_k_prinzip.htm, netbasedwires_k_netzbearbeiten.htm,
  netbasedwires_h_ndpzeichnen.htm, netbasedwires_h_verbindungsketten.htm, netbasedwires_h_zielreihenfolgeaendern.htm.
- Potentiale: potentialbrowsergui_k_start.htm, potentialbrowsergui_k_prinzippotenziale.htm,
  potentialbrowsergui_h_pdpzeichnen.htm, potentialbrowsergui_h_potenzialverfolgung.htm,
  potentialbrowsergui_d_potenziale.htm, potentialbrowsergui_d_rohrleitungen.htm.
- Weitere: interruptionpointgui_d_einstellsmartconnecting.htm, interruptionpointgui_d_einstellsmartabbruchstellen.htm,
  gedviewer_d_einstellungenverbindungssymbole.htm, gedviewer_d_einstellungenbenutzerallgemein.htm,
  workingsection_k_prinzip.htm, blackbox_k_schachteln.htm, singlepole_k_verbindungen.htm.
