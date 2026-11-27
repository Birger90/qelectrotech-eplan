"""Tests des WSCAD-Imports ohne WSCAD-Installation (nur selbst gebaute Daten).

    python -m unittest tools/wscad_import/test_wscad_import.py

Mit QET_WSCAD_TESTPROJEKT=<WSCAD-Projektordner> läuft zusätzlich der ganze Import dieses Projekts (braucht die
gebaute QElectroTech wie planvorlagen.Plan.erzeuge); das Ergebnis landet in einem temporären Ordner.
"""
import gzip
import os
import sqlite3
import struct
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

HIER = Path(__file__).resolve().parent
sys.path.insert(0, str(HIER))
import wscad as W  # noqa: E402
import wscad_nach_qet as Q  # noqa: E402


def gepackt(inhalt, version=(6, 20, 0, 0)):
    """Abschnitt wie WSCADs StreamPackService.PackDataStream: Version, Länge, Schlüssel- und IV-Länge, Schlüssel,
    IV, drei Prüfbytes, dann AES-CBC (PKCS7) über GZip."""
    from cryptography.hazmat.primitives import padding
    from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
    schluessel, iv = bytes(range(32)), bytes(range(100, 116))
    p = padding.PKCS7(128).padder()
    klar = p.update(gzip.compress(inhalt)) + p.finalize()
    e = Cipher(algorithms.AES(schluessel), modes.CBC(iv)).encryptor()
    geheim = e.update(klar) + e.finalize()
    kopf = bytes(version) + struct.pack("<I", len(inhalt)) + bytes([32, 16]) + schluessel + iv
    return kopf + bytes([schluessel[0], schluessel[1], iv[0]]) + geheim


def seitendatei(pfad, xml):
    """Seitendatei: Kennung, Version 6.4, Zähler, fünf Abschnittseinträge à 19 Byte, Abschnitt 2 = Seite."""
    seite = gepackt(b"\x06\x14\x00\x00" + xml.encode("utf-8"))
    beginn = 12 + 5 * 19
    eintraege = struct.pack("<BBBIId", 1, 1, 2, beginn, len(seite), 0.0)
    eintraege += b"\x00" * 19 * 4
    with open(pfad, "wb") as f:
        f.write(b"\x01\x02\x00\x00" + bytes((6, 4, 0, 0)) + struct.pack("<I", 103) + eintraege + seite)


SEITE = """<?xml version="1.0" encoding="utf-8"?><WS><Solution><Project><File>
<Header><PaperSize val="297 210" /></Header>
<SL type="1.0" x="8.75" y="5" hid="11" pid="12"><P id="BMKZ" prd="-Q1" />
  <line x1="1.25" y1="0" x2="1.25" y2="5" />
  <PHL x="-1" y="3" style="wstype:BMKZ;align:26;size:3.75" xpath="../P[@id='BMKZ']" />
  <Pin connDir="2" pos="1.25 5" nr="2" /></SL>
<SL type="10.2" x="18.75" y="10"><P id="BtName"><T id="DE">Winkel: links -&gt; unten</T></P>
  <polyline points="0 0 1.25 0 1.25 1.25" />
  <Pin connDir="8" pos="0 0" /><Pin connDir="4" pos="1.25 1.25" /></SL>
<SL type="1.0" x="18.75" y="30" hid="21" pid="22"><P id="BMKZ" prd="-K1" />
  <Pin connDir="1" pos="1.25 0" nr="A1" /></SL>
</File></Project></Solution></WS>"""


class Seitendatei(unittest.TestCase):
    def test_auspacken_liefert_das_xml(self):
        with tempfile.TemporaryDirectory() as d:
            pfad = os.path.join(d, "=A1+ST_PLAN.0001.wsELD")
            seitendatei(pfad, SEITE)
            f = W.seite_xml(pfad)
            self.assertEqual(f.tag, "File")
            self.assertEqual([sl.get("type") for sl in f.findall("SL")], ["1.0", "10.2", "1.0"])

    def test_falsche_auffuellung_faellt_auf(self):
        roh = bytearray(gepackt(b"abc"))
        roh[-1] ^= 0xFF                              # letzter Block verdorben
        with self.assertRaises(Exception):
            W.auspacken(bytes(roh))


class Texte(unittest.TestCase):
    def test_ausrichtung_wie_am_pdf_geeicht(self):
        self.assertEqual(Q._ausrichtung({"align": "26"}), (2, "unten"))      # BMK rechts unten
        self.assertEqual(Q._ausrichtung({"align": "22"}), (1, "mitte"))      # Spaltennummer
        self.assertEqual(Q._ausrichtung({"align": "16"}), (0, "mitte"))      # Pfeil-Querverweis
        self.assertEqual(Q._ausrichtung({"align": "6"}), (1, "oben"))
        self.assertEqual(Q._ausrichtung({}), (0, "grund"))

    def test_sps_adresse(self):
        self.assertEqual(Q.sps_adresse("0#|#True#|#True#|#EN|T|I|L|DE|T|E|L|HU|T|I#|##|#0#|#7#|#1#|#.#|#"), "E0.7")
        self.assertEqual(Q.sps_adresse("0#|#True#|#True#|#HU|T|Q|L|DE|T|A#|##|#2#|#3#|#1#|#.#|#"), "A2.3")
        self.assertEqual(Q.sps_adresse("0#|#False#|#True#|#EN|T|AIW|L|DE|T|AEW#|##|#1#|#-2147483648#|#1#|#.#|#"),
                         "AEW1")

    def test_eigene_struktur_im_querverweis(self):
        Q.EIGENE_STRUKTUR[0] = "=A1+ST"
        try:
            self.assertEqual(Q._ohne_eigene_struktur("=A1+ST/CE:1.3"), "/CE:1.3")
            self.assertEqual(Q._ohne_eigene_struktur("=A1+P1/2.4"), "=A1+P1/2.4")
        finally:
            Q.EIGENE_STRUKTUR[0] = ""

    def test_zeilen(self):
        self.assertEqual(Q._zeilen("Einspeisung\r\n\r\nHV Schaltraum"), "Einspeisung\nHV Schaltraum")
        self.assertEqual(Q._zeilen("Messwandler\\aTrennklemmen"), "Messwandler\nTrennklemmen")

    def test_mehrzeilig_unten_beginnt_am_anker(self):
        f = Q.text_formen("a\nb", 10, 20, {"align": "30"}, Q.Abbild(0, 0, 210))
        self.assertAlmostEqual(f[0][1][1], 210 - 20)                         # erste Zeile am Anker
        self.assertAlmostEqual(f[1][1][1], 210 - 22.5)                       # zweite 2,5 mm darunter

    def test_kasten_umbruch_und_kuerzen(self):
        lang = "Rückmeldung Not-Halt Schaltschrank und Transporttisch"
        umbr, box = Q.kasten(lang, ET.fromstring('<PH wd="20" hg="7" />'), {"tc": "6"})
        self.assertGreater(umbr.count("\n"), 0)
        self.assertEqual(box, (20.0, 7.0))
        kurz, _ = Q.kasten(lang, ET.fromstring('<PHL wd="20" hg="2.5" />'), {"tc": "5"})
        self.assertTrue(lang.startswith(kurz) and len(kurz) < len(lang))


class Leitungen(unittest.TestCase):
    def test_weg_ueber_winkel_wird_eine_ecke(self):
        f = ET.fromstring(SEITE).find(".//File")
        se = Q.Seite({"breite": 297, "hoehe": 210}, f)
        w = se.weg((10.0, 10.0), (20.0, 30.0))
        self.assertIsNotNone(w)
        self.assertEqual(Q.ecken(w), [(20.0, 10.0)])

    def test_schraege_des_winkels_wird_ecke(self):
        self.assertEqual(Q.ecken([(40.01, 12.5), (126.25, 12.5), (127.5, 13.75), (127.5, 40)]), [(127.5, 12.5)])

    def test_punktschluessel_kippt_nicht(self):
        self.assertEqual(Q._schluessel((40.01, 170.0)), Q._schluessel((40.010000000000005, 170.0)))

    def test_anschlusslage_nach_pinid(self):
        f = ET.fromstring(SEITE).find(".//File")
        se = Q.Seite({"breite": 297, "hoehe": 210}, f)
        # Datenbank: PinID 1 relativ (1,25; 5) zur Symbolbasis, die bei gedrehten Pfeilen abweichen kann
        self.assertEqual(se.anschlusslage(11, (1, ("", "2", 1.25, 5.0))), (10.0, 10.0))
        self.assertIsNone(se.anschlusslage(99, (1, ("", "", 0, 0))))


class Sammlung(unittest.TestCase):
    def test_hersteller_auf_vorhandene_ordner(self):
        vorhanden = ["Siemens", "Phoenix Contact", "Weidmüller", "Lapp", "SICK"]
        self.assertEqual(Q.hersteller_ordner("SIEMENS", vorhanden), "Siemens")
        self.assertEqual(Q.hersteller_ordner("Siemens AG", vorhanden), "Siemens")
        self.assertEqual(Q.hersteller_ordner("PHOENIX", vorhanden), "Phoenix Contact")
        self.assertEqual(Q.hersteller_ordner("Weidmüller Interface GmbH & Co. KG", vorhanden), "Weidmüller")
        self.assertEqual(Q.hersteller_ordner("Weidmueller", vorhanden), "Weidmüller")
        self.assertEqual(Q.hersteller_ordner("LAPP KABEL", vorhanden), "Lapp")
        self.assertEqual(Q.hersteller_ordner("Sick", vorhanden), "SICK")
        self.assertEqual(Q.hersteller_ordner("BINDER", vorhanden), "Binder")

    def test_kategorie(self):
        def sl(typ, name):
            return ET.fromstring(f'<SL type="{typ}"><P id="ReferenzName" val="{name}" /></SL>')
        self.assertEqual(Q.kategorie_wscad(sl("4.0", "Klemme (allgemein)")), "Klemmen")
        self.assertEqual(Q.kategorie_wscad(sl("2.0", "Leistungsschütz")), "Relais, Schütze")
        self.assertEqual(Q.kategorie_wscad(sl("1.0", "Automat_(MCB)_1P")), "Schutzeinrichtungen")
        self.assertEqual(Q.kategorie_wscad(sl("15.323", "CPU")), "SPS")
        self.assertEqual(Q.kategorie_wscad(sl("1.0", "Drehzahlregler")), "Geräte")


class Pfeile(unittest.TestCase):
    def test_paare_nach_querverweis_und_frueheres_blatt_gehend(self):
        def seite(nr, pfeile):
            sls = "".join(f'<SL type="14.3" x="0" y="0"><P id="Leitungsname" val="{n}" />'
                          f'<QV><P id="QV" val="{qv}" /></QV><Pin connDir="8" pos="0 0" /></SL>' for n, qv in pfeile)
            f = ET.fromstring(f'<File><Template><PH><P id="Blattnummer" val="{nr}" /></PH></Template>{sls}</File>')
            return Q.Seite({"breite": 297, "hoehe": 210, "anlage": "A1", "ort": "ST"}, f)
        # L1 auf Blatt 1 zeigt auf Blatt 3 (nicht auf das nähere Blatt 2); Namen ohne Richtung im Symbol
        s1, s2, s3 = seite("1", [("L1", "/3.0")]), seite("2", [("L1", "/9.0")]), seite("3", [("L1", "/1.0")])
        paare, gehend, allein = Q.pfeilpaare([s1, s2, s3])
        self.assertEqual(len(paare), 1)
        a, b = paare[0]
        self.assertIs(a, s1.symbole[0])
        self.assertIs(b, s3.symbole[0])
        self.assertTrue(gehend[id(a)])
        self.assertFalse(gehend[id(b)])
        self.assertEqual(allein, 1)


class Datenbank(unittest.TestCase):
    def test_seitenfolge_und_dateien(self):
        with tempfile.TemporaryDirectory() as d:
            os.makedirs(os.path.join(d, "Database"))
            c = sqlite3.connect(os.path.join(d, "Database", "WSCAD6.db"))
            c.execute("CREATE TABLE Page (PageID INTEGER, FileName TEXT, FileType TEXT, Ref TEXT, Width REAL, "
                      "Height REAL, AspF TEXT, AspL TEXT)")
            for pid, name in ((29, "=A1+P1_PLAN.0001.WSELD"), (6, "=A1+ST_PLAN.0002.WSELD"),
                              (5, "=A1+ST_PLAN.0001.WSELD")):
                c.execute("INSERT INTO Page VALUES (?, ?, 'wsELD', '', 297, 210, 'A1', ?)",
                          (pid, name, name[4:6]))
            c.commit()
            c.close()
            for name in ("=A1+ST_Plan.0001.wsELD", "=A1+ST_Plan.0002.wsELD", "=A1+P1_Plan.0001.wsELD"):
                seitendatei(os.path.join(d, name), SEITE)
            P = W.Projekt(d)
            self.assertEqual([s["name"] for s in P.schaltplanseiten()],
                             ["=A1+ST_PLAN.0001.WSELD", "=A1+ST_PLAN.0002.WSELD", "=A1+P1_PLAN.0001.WSELD"])
            self.assertTrue(all(os.path.isfile(s["datei"]) for s in P.schaltplanseiten()))
            P.db.close()

    def test_projektordner_aus_datei(self):
        with tempfile.TemporaryDirectory() as d:
            os.makedirs(os.path.join(d, "Database"))
            db = os.path.join(d, "Database", "WSCAD6.db")
            open(db, "wb").close()
            self.assertEqual(os.path.normpath(Q.projektordner(db)), os.path.normpath(d))


@unittest.skipUnless(os.environ.get("QET_WSCAD_TESTPROJEKT"), "QET_WSCAD_TESTPROJEKT nicht gesetzt")
class GanzesProjekt(unittest.TestCase):
    def test_import(self):
        quelle = os.environ["QET_WSCAD_TESTPROJEKT"]
        with tempfile.TemporaryDirectory() as d:
            os.environ["QET_COMPANY_ELEMENTS_DIR"] = os.path.join(d, "sammlung")
            Q.S.COMPANY = os.environ["QET_COMPANY_ELEMENTS_DIR"]
            erg = Q.umsetzen(quelle, os.path.join(d, "import.qet"))
            text = open(erg, encoding="utf-8").read()
            self.assertGreater(text.count("<diagram "), 0)
            self.assertGreater(text.count("<conductor "), 0)
            bericht = open(next(Path(d).glob("_wscad_import_*/import_bericht.txt")), encoding="utf-8").read()
            self.assertIn("fehlgeschlagene Operationen 0", bericht)


if __name__ == "__main__":
    unittest.main()
