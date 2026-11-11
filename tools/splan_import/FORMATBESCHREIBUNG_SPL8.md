# sPlan 8 (.spl8) – unabhängig dekodierte Struktur

Abacom veröffentlicht das Format nicht, und es gibt keinen freien Leser (Websuche November 2026: nur Dateiendungs-
Verzeichnisse, keine Spezifikation, kein Parser). Grundlage dieser Beschreibung sind 7 Dateien:

| Datei | Herkunft | Blätter | vollständig gelesen |
|---|---|---|---|
| Projekt A (Kundenplan, 4 MB) | sPlan 8, Kennung `F@` | 22 | ja, bis zum letzten Byte |
| Projekt B (Kundenplan, 230 kB) | sPlan 8, Kennung `F@` | 2 | ja |
| `Dimmer.spl8`, `Parent-Child.spl8`, `Elemente.spl8`, `Kunst.spl8` | Beispiele der sPlan-8-Installation | 3 / 1 / 16 / 5 | ja |

`Elemente.spl8` enthält je Blatt eine Objektart (Rechtecke, Kreise, Polygone, Linien, Bezierkurven, Lötpunkte, Texte,
Mengentext, Gruppen, Variablen, aktive Verknüpfungen) und war die wichtigste Gegenprobe. Die Bedeutung der
Bauteilfelder ist mit der Programmhilfe `splan80.CHM` abgeglichen (Kapitel „Was ist ein Bauteil?“, „Kontakte“,
„Parent-Child Bauteilbeziehungen“, „Lötpunkte“).

Kennzeichnung: **FAKT** = durch den Leser an allen 7 Dateien nachgewiesen (jede Datei wird lückenlos bis zum Ende
gelesen, jeder Zeiger landet auf dem nächsten gültigen Objekt) · **BELEGT** = Bedeutung durch die sPlan-Hilfe oder einen
Vergleich mit dem gedruckten PDF gestützt · **HYPOTHESE** = abgeleitet, nicht unabhängig bestätigt.

Leser: `splan8.py` (Funktionen tragen die Abschnittsnummern dieser Datei).

## 1. Grundtypen
- FAKT: Unkomprimierter Delphi-Strom, little endian. Keine Prüfsummen.
- FAKT: `i32` = int32, `f32` = float32, `u8` = Byte.
- FAKT: Text `ws` = `i32` Zeichenzahl + UTF-16LE (ohne Nullzeichen). Schriftname = `i32` + Pascal-Kurzstring
  (`u8` Länge + Latin-1), z. B. `05 00 00 00 05 'Arial'`.
- FAKT: Koordinaten sind `f32` in 0,1 mm, Ursprung oben links, y nach unten. Belegt am Rahmen: A4 quer reicht von
  (20, 20) bis (2920, 2040); der innere Rahmen liegt bei 72 (7,2 mm), das Schriftfeld beginnt bei y ≈ 1760.
- BELEGT: Zum gedruckten PDF (A4, 842 × 595 pt) gilt pt = 0,2747 · Wert + (3,4 | 2,3) – nur zum Vergleichen genutzt,
  nie zum Nachbauen.

## 2. Dateikopf
- FAKT: Byte 0–7 `07 'SPLAN80'` (Pascal-Kurzstring), Byte 8–15 Version (`46 40 …` bzw. `48 40 …`, beide lesbar).
- FAKT: danach `u8`, `ws`, `ws`, `i32`, `i32` (Bedeutung offen; in Projekt A „“, „11,12“, 20, 21; in Kunst.spl8
  „++3 -“, „1,3“).

## 3. Blatt
- FAKT: `ws` Blattname, `i32` Breite mm, `i32` Höhe mm (210 × 297; Hochformat: vertauscht), `i32` Raster,
  `i32`, `i32` (Bildlauf), `f32` Zoom, `i32` **Anzahl − 1** der Objekte, dann die Objekte.
- FAKT: Nach den Objekten folgt ein Blattfuß (8 × `FF`, `f64` 0,0 und 1,0, Blattmaße 2800 × 1900 als `f32` …). Seine
  Länge schwankt je Datei (102 bzw. 104 Byte); der Leser springt deshalb zum nächsten gültigen Blattkopf (Name +
  plausible Blattmaße). Nach dem letzten Blatt folgt nur noch dieser Fuß.

## 4. Objektkopf (alle Objektarten)
- FAKT: `u8` Typ, dann 23 Byte: 3 Byte Strich (Byte 2 = Linienbreite in 0,1 mm, z. B. 1, 2, 5), 11 Byte (beim Lötpunkt
  Größe und Automatik, sonst 0), 4 Byte Füllfarbe, 4 Byte Linienfarbe, 1 Byte Flag.

| Typ | Art | Inhalt nach dem Kopf |
|---|---|---|
| 01 | Ellipse/Kreis | `i32` n, n+1 Punkte (als Punktfolge), 47 Byte |
| 02 | Rechteck | 5 Punkte (4 Ecken + Schluss), 31 Byte (Eckenradius u. a.) |
| 03 | Text | §5 |
| 04 | Linienzug | `i32` n, n+1 Punkte, 21 Byte (§6) |
| 05 | Polygon (geschlossen) | `i32` n, n+1 Punkte, 27 Byte |
| 06 | Gruppe („Clip“) | `i32` Kinder − 1, Kinder (§10) |
| 07 | Bauteil | §10 |
| 08 | Lötpunkt | 1 Punkt |
| 09 | Bild | 4 Ecken, zlib-Strom (Inhalt: `u32` Länge + BMP-Datei), 4 Byte |
| 0A | Mengentext | 4 Ecken, `u8`, Schrift, 9 Byte, `i32` Zeilen, Zeilen als `ws`, 7 Byte |
| 0B | Kontakt (Anschlussbeschriftung eines Bauteils) | wie Text (§5), danach `ws` Kontaktname |
| 0C | Bezierkurve | wie Linienzug |

## 5. Text
- FAKT: Kopf, 4 Ecken des Textrahmens, `ws` Text, Schrift, 14 Byte Attribute (Byte 5 = Schriftgrad), `ws` aktive
  Verknüpfung (URL/Datei, sonst leer; belegt in Elemente.spl8 „www.abacom-online.de“), 16 Byte GUID, 22 Byte.
- FAKT: Enthält ein freier Text eine Variable (`<PAGENO>` …), folgt ein zweiter `ws` mit dem ausgewerteten Text
  (Projekt B: „<PAGENO>“ → „1“). BELEGT: Variablen laut Hilfe „Feste Variablen“.
- FAKT: Drehung aus dem Rahmen: Richtung Ecke 0 → Ecke 1 = Leserichtung (0°, ±90° kommen vor); Ecke 0 → Ecke 3 zeigt
  zur Grundlinie. Schrifthöhe = Länge Ecke 0 → Ecke 3.

## 6. Linienzug, Pfeilenden
- FAKT: 21 Byte nach den Punkten; Byte 5 = Linienende am Anfang, Byte 6 = am Ende (0 = keins, 2 = Pfeil; 1, 5, 10, 11
  kommen ebenfalls vor). BELEGT: In Projekt A tragen genau die im PDF mit Pfeil gedruckten Linien den Wert 2.
- FAKT: In älteren Blättern sind Pfeile ein eigenes kleines Polygon (Typ 05, 6 Punkte, ≤ 3 mm) am Linienende.

## 7. Flächen
- FAKT: Rechteck, Polygon und Ellipse wie in §4; Ellipsen sind bereits als Punktfolge (32 Punkte) gespeichert.

## 8. Lötpunkt
- FAKT: ein Punkt. BELEGT (Hilfe „Lötpunkte“): zeigt an, dass sich kreuzende Linien verbunden sind.

## 9. Bild
- FAKT: zlib-Strom direkt nach den 4 Ecken, ohne Längenangabe; das Ende liefert der zlib-Dekoder. Inhalt `u32` + BMP.

## 10. Gruppe und Bauteil
- FAKT: `i32` Kinder − 1, dann die Kinder (beliebige Objekte, rekursiv).
- FAKT: Bauteil (07) danach:
  - Bezeichner als Text ohne Typbyte (§5) + `ws` Bezeichner ausgewertet (z. B. „IC1-1“ aus „<PARENT_ID>-<CHILDNO>“),
  - Wert als Text ohne Typbyte (§5); enthält er eine Variable, folgt `ws` mit dem ausgewerteten Wert,
  - 4 Byte Schalter (sichtbar / automatisch nummerieren / Stückliste – HYPOTHESE),
  - `ws` **Name des Bibliotheksbauteils** (z. B. „Schliesser“, „Öffner“, „PHO Klemme grau1“, „Beckhoff I/o 8Fach“,
    „Netzteil“; mehrsprachig mit `\r` getrennt; einmal nur das Steuerzeichen 07), BELEGT durch die Hilfe
    („Unterschrift in der Bibliothek“),
  - `ws` Bezeichner-Vorlage der automatischen Nummerierung („K“, „<PARENT_ID>“), `i32` laufende Nummer (−1 = aus),
  - `f32`, `f32`, `i32`, `u8` (Lage der Kind-Beschriftung u. a., HYPOTHESE),
  - 4 × `ws` Zusatztexte 1–4 für die Stückliste (BELEGT: Hilfe „Vorhandene Bauteile ändern“),
  - `u8` ist Parent, `u32` **eigene ID**, `u32` **Parent-ID** (0 = keiner), `u8`.
- BELEGT: In Parent-Child.spl8 tragen die vier Gatter IC1-1 … IC1-4 die ID des 7400 als Parent-ID; in Projekt A tragen
  die Hilfskontakte von K4, K6, K15 … die ID ihres Schützes. Daraus wird Spule ↔ Kontakt (Master/Slave) in QET.

## 11. Blattfuß
- siehe §3. HYPOTHESE: Druckbereich, Ränder, Raster, Ansichtsposition.

## 12. Verbindungen (keine Netzliste in der Datei)
sPlan speichert keine Anschlüsse und keine Netze – ein Bauteil ist laut Hilfe nur eine Gruppe von Zeichenelementen.
Der Import leitet die Verbindungen deshalb aus der Geometrie ab (`splan_analyse.py`), mit diesen Regeln:

1. Leitung = Linienzug auf oberster Ebene (nicht in einem Bauteil), nicht im Rahmen/Schriftfeld (Rahmenlinien liegen
   samt Segmentmitten im Randstreifen; Flächen gehören zum Rahmen, wenn ihre Mitte dort liegt).
2. Zwei Leitungen sind verbunden, wenn ein Ende auf der anderen liegt (Abstand ≤ 0,3 mm) oder ein Lötpunkt auf beiden
   liegt. Kreuzungen ohne Lötpunkt sind nicht verbunden (Hilfe „Lötpunkte“).
3. Anschluss = Leitungsende, das auf einer Linie eines Bauteils oder in seinem Rahmen endet (≤ 0,3 mm). Reihenklemmen:
   alle Enden einer Seite (oben/unten) sind eine Schraube.
4. Anschlussname: nächster sPlan-„Kontakt“ des Bauteils, sonst nächster Text im Bauteil; Sammeltexte wie „PE I L I N“
   werden in Einzelnamen an ihrer Lage im Text zerlegt. Ohne Text: Nummer nach Lage (→ Offene_Fragen.txt).
5. Potentialname = Text am Pfeilende bzw. am freien Leitungsende. Die Zuordnung Text ↔ Pfeil erfolgt global nach dem
   kleinsten Abstand, damit eng stehende Pfeile ihre Texte nicht tauschen (BELEGT: Netzteil T1 „+“ → „K1:3“,
   „−“ → „X4:1“, gegengeprüft an der Klemmenliste des Blatts „24V Klemmen“: T1:− liegt auf X4).
6. Bilder (Typ 09), auf denen Leitungen enden, sind Geräte ohne Daten (Projekt A: Sicherheitsrelais K1/K2 als Foto);
   ihre Anschlussnamen stehen nur im Bild → Offene_Fragen.txt.

## Offen
- Bedeutung der Kopffelder (§2), der 4 Schalterbytes und der beiden `f32` im Bauteil.
- Ellipsen/Kreisbögen: Start-/Endwinkel in den 47 Byte (für den Import nicht nötig, die Punktfolge reicht).
- Mengentext: Ausrichtung/Zeilenabstand in den 9 Byte.
