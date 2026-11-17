"""Tests der Bausteine des FluidSIM-Imports ohne FluidSIM-Installation (nur selbst gebaute Daten).

    python -m unittest tools/fluidsim_import/test_fluidsim_import.py
"""
import struct
import sys
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

HIER = Path(__file__).resolve().parent
sys.path.insert(0, str(HIER))
import fluidsim_plan as fp  # noqa: E402
import fluidsim_grafcet as fg  # noqa: E402


def gepackt(*werte):
    return struct.pack("<%dd" % len(werte), *werte).hex().upper()


class Entpacken(unittest.TestCase):
    def test_linie_und_polylinie(self):
        w = ET.fromstring(f"<s><line end-cap='arrow1'>{gepackt(30, 30, -12, 12)}</line>"
                          f"<polyline count='3'>{gepackt(24, 30, 24, 36, 42, 48)}</polyline></s>")
        fp.fs.entpacken(w)
        li, pl = w.find("line"), w.find("polyline")
        self.assertEqual([float(li.get(k)) for k in ("x1", "y1", "x2", "y2")], [30, -12, 30, 12])
        self.assertEqual(pl.get("points"), "24.0,36.0 30.0,42.0 24.0,48.0")
        self.assertIsNone(li.text)

    def test_normaler_text_bleibt(self):
        w = ET.fromstring("<s><line x1='1' y1='2'/><text>Hallo</text></s>")
        fp.fs.entpacken(w)
        self.assertEqual(w.find("text").text, "Hallo")


class Bedingung(unittest.TestCase):
    def test_schreibweise(self):
        self.assertEqual(fg.bedingung("On * -KF3"), "On · -KF3")
        self.assertEqual(fg.bedingung("I0 * NOT I1 + NOT I7"), "I0 · NOT I1 ∨ NOT I7")
        self.assertEqual(fg.bedingung("RE -SF1"), "↑-SF1")
        self.assertEqual(fg.bedingung("NOT -SF2.signal"), "NOT -SF2")
        self.assertEqual(fg.bedingung("[C > 2]"), "C > 2")
        self.assertEqual(fg.bedingung("0.5s / X1"), "0.5s / X1")


class Simulationsfunktion(unittest.TestCase):
    def symbol(self):
        return ET.fromstring("<symbol><symbol-layer ident='StaticLayer'>"
                             "<connector type='pneumatic' ident='1' designation='1'/>"
                             "<connector type='pneumatic' ident='2' designation='2'/>"
                             "</symbol-layer></symbol>")

    def test_drosselrueckschlag(self):
        o = ET.fromstring("<component object-ref='Pneu.FlowValve.ThrottleCheckValve'/>")
        fkt, werte = fp.simulationsfunktion(o, self.symbol(), [("level", "50", "%"), ("qNN_n", "80", "l/min")])
        w = {n: (v, u) for n, v, u in werte}
        self.assertEqual(fkt, "flow_control_valve.one_way_flow_control_valve")
        self.assertEqual((w["free_from"][0], w["free_to"][0]), ("2", "1"))
        self.assertAlmostEqual(float(w["qNN"][0]), 40.0)

    def test_widerstand_in_bar(self):
        o = ET.fromstring("<component object-ref='Hyd.ShutoffValve.CheckValve01'/>")
        sym = ET.fromstring("<symbol><symbol-layer ident='S'><connector type='hydraulic' ident='1' designation='B'/>"
                            "<connector type='hydraulic' ident='2' designation='A'/></symbol-layer></symbol>")
        fkt, werte = fp.simulationsfunktion(o, sym, [("rT", "0.009", "MPa.min2/l2")])
        w = {n: (v, u) for n, v, u in werte}
        self.assertEqual(fkt, "shutoff_valve.check_valve")
        self.assertEqual(w["rT"][1], "bar.min2/l2")
        self.assertAlmostEqual(float(w["rT"][0]), 0.09)
        self.assertEqual((w["free_from"][0], w["free_to"][0]), ("A", "B"))

    def test_einfache_stromquelle_als_pumpe(self):
        o = ET.fromstring("<component object-ref='Hyd.Supply.SimpleFlowSource'/>")
        fkt, werte = fp.simulationsfunktion(o, self.symbol(), [("q0", "2", "l/min"), ("p0", "6", "MPa")])
        w = {n: (v, u) for n, v, u in werte}
        self.assertEqual(fkt, "supply.pump")
        self.assertEqual(w["pMax"], ("6", "MPa"))
        self.assertEqual((w["displacement"], w["rpm"]), (("2", "l"), ("1", "1/min")))


class Rollen(unittest.TestCase):
    def test_elektrische_rollen(self):
        leer = ET.fromstring("<component/>")
        self.assertEqual(fp.elektro_rolle("ET.Source.Pot24V", "", leer), "supply+")
        self.assertEqual(fp.elektro_rolle("ET.Source.Pot0V_ladder", "", leer), "supply-")
        self.assertEqual(fp.elektro_rolle("ET.Switch.MakeSwitchManualNoDetent", "contact-manual", leer), "button")
        self.assertEqual(fp.elektro_rolle("ET.Switch.BreakSwitchManualDetent", "contact-manual", leer), "switch_nc")
        self.assertEqual(fp.elektro_rolle("ET.Relay.ValveSolenoid", "solenoid-valve-coil", leer), "solenoid")
        self.assertEqual(fp.elektro_rolle("ET.PLC.GrafcetPLC_8I8Q", "", leer), "grafcet_io")

    def test_wegmarken_am_naechsten_zylinder(self):
        regel = ET.fromstring("<component object-ref='Mech.FS3.DistanceRule' x1='100' y1='0' width='30' height='30'>"
                              "<const ident='label1' value='-BG1'/><const ident='x11' value='0'/>"
                              "<const ident='x12' value='2'/><const ident='label2' value='-BG2'/>"
                              "<const ident='x21' value='99'/><const ident='x22' value='100'/>"
                              "<const ident='x31' value='-1'/><const ident='x32' value='-1'/></component>")
        marken = fp.wegmarken([regel], [("-MM1", (110, 20)), ("-MM2", (600, 20))])
        self.assertEqual(marken, {"-BG1": "-MM1 0..2", "-BG2": "-MM1 99..100"})


class Magnete(unittest.TestCase):
    def test_seite_und_spiegelung(self):
        sym = ET.fromstring("<symbol><symbol-layer ident='AEML'><connector type='electric-label' designation='-MB1'/>"
                            "</symbol-layer><symbol-layer ident='AEMR'><connector type='electric-label' "
                            "designation='-MB2'/></symbol-layer></symbol>")
        self.assertEqual(fp.magnete(sym, ET.fromstring("<component/>")), [("left", "-MB1"), ("right", "-MB2")])
        self.assertEqual(fp.magnete(sym, ET.fromstring("<component scale-x='-1'/>")),
                         [("right", "-MB1"), ("left", "-MB2")])


SAMMLUNG = HIER.parents[1] / "build" / "deploy" / "elements"


@unittest.skipUnless((SAMMLUNG / fg.DATEI["einschl"]).exists(), "verteilte GRAFCET-Symbole fehlen")
class GrafcetGliederung(unittest.TestCase):
    """R14: Teil-GRAFCET-Rahmen, einschließender Schritt, Makroschritt, zwangssteuernder Befehl"""

    def objekt(self, ident, ref, x, y, **werte):
        o = ET.Element("component", {"ident": ident, "object-ref": ref, "x1": str(x), "y1": str(y),
                                     "width": "20", "height": "20"})
        for k, v in werte.items():
            ET.SubElement(o, "const", {"ident": k, "value": v})
        return o

    def test_gliederung(self):
        objekte = [
            self.objekt("1", "Grafcet.Step", 0, 0, name="1", flags="initial"),
            self.objekt("2", "Grafcet.Step", 0, 100, name="2", flags="enclosing", enclosedSubs=" 7 "),
            self.objekt("3", "Grafcet.Step", 0, 200, name="5", flags="macro"),
            self.objekt("4", "Grafcet.Step", 0, 300, name="5", flags="mentry"),
            self.objekt("5", "Grafcet.Step", 300, 0, name="1", flags="alink"),
            self.objekt("6", "Grafcet.SubGrafcet", 250, -50, name="7"),
            self.objekt("7", "Grafcet.Action", 40, 100, actionType="6", **{"sub-grafcet": "7", "steps": "INIT"}),
        ]
        objekte[5].set("width", "200")
        objekte[5].set("height", "200")
        ops, zahl, offen = fg.umwandeln(0, objekte, [], SAMMLUNG, 1.0, lambda o: (float(o.get("x1")), float(o.get("y1"))))
        labels = [o["label"] for o in ops if o["op"] == "set_label"]
        infos = {(o["element"], o["key"]): o["value"] for o in ops if o["op"] == "set_info"}
        pfade = {o["id"]: o["path"] for o in ops if o["op"] == "add_element"}
        self.assertIn("M5", labels)
        self.assertIn("E5", labels)
        self.assertIn("G7{INIT}", labels)
        self.assertTrue(pfade["g0_2"].endswith("20202002_step.elmt"))
        self.assertTrue(pfade["g0_3"].endswith("20202003_step.elmt"))
        self.assertEqual(infos[("$g0_2", "sim_encloses")], "G7")
        self.assertEqual(infos[("$g0_5", "sim_grafcet")], "G7")
        self.assertEqual(infos[("$g0_5", "sim_activation")], "1")
        self.assertNotIn(("$g0_1", "sim_grafcet"), infos)
        self.assertEqual(offen, [])


class Blatt(unittest.TestCase):
    def test_kleine_schaltung_auf_a4_quer_oben_links(self):
        # FluidSIM zeichnet klein mitten auf ein A4-Blatt: nach links oben schieben (Vielfaches von 10), A4 quer
        alle = [ET.fromstring("<component x1='240' y1='300' width='60' height='36'/>"),
                ET.fromstring("<component x1='240' y1='420' width='24' height='24'/>")]
        ops = [{"x": 225.0, "y": 265.0}, {"x": 210.0, "y": 360.0}]
        dx, dy, papier = fp.blatt_lage(alle, ops)
        self.assertEqual((dx % 10, dy % 10), (0, 0))
        self.assertEqual(min(240 * fp.fs.K, 210.0) + dx, 50.0)   # linker Rand des Kastens bei RAND
        self.assertEqual(papier, "a4-landscape")

    def test_grosse_schaltung_auf_groesserem_papier(self):
        alle = [ET.fromstring("<component x1='0' y1='0' width='1500' height='900'/>")]
        self.assertEqual(fp.blatt_lage(alle, [{"x": 0.0, "y": 0.0}])[2], "a3-landscape")


class Lesen(unittest.TestCase):
    def test_blatt_im_abschnitt_circuits(self):
        # TP1011C, TP1024, TP1121: das Blatt steckt in section type="circuits" -> <file>
        roh = ("<fluidtools2><section type='circuits' count='1'><file name='x.circ'>"
               "<section type='circuit-objects'><component ident='O1'/></section></file></section></fluidtools2>")
        import tempfile
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "x.circ"
            p.write_text(roh, encoding="utf-8")
            w = fp.lesen(p)
        self.assertIsNotNone(fp.abschnitt(w, "circuit-objects"))


class EigeneGrafik(unittest.TestCase):
    def test_widerstand_ohne_aufschrift(self):
        import eigene_grafik as eg
        sym = ET.fromstring("<symbol><symbol-layer ident='default'>"
                            "<connector type='electric' ident='1' designation='p' cx='-24' cy='0'/>"
                            "<connector type='electric' ident='2' designation='n' cx='24' cy='0'/>"
                            "</symbol-layer></symbol>")
        neu = eg.ersetzen(sym, ET.fromstring("<component object-ref='ET.Basic.Resistor'/>"))
        eigen = next(e for e in neu if e.get("ident") == "EIGEN")
        self.assertEqual(eigen.findall("text"), [])          # kein Kasten mit Namen
        self.assertEqual(len(eigen.findall("polyline")), 1)  # der Widerstandskörper

    def test_npn_mit_pfeil_am_emitter(self):
        import eigene_grafik as eg
        sym = ET.fromstring("<symbol><symbol-layer ident='default'>"
                            "<connector type='electric' ident='1' designation='B' cx='-21' cy='0'/>"
                            "<connector type='electric' ident='2' designation='C' cx='21' cy='-18'/>"
                            "<connector type='electric' ident='3' designation='E' cx='21' cy='18'/>"
                            "</symbol-layer></symbol>")
        neu = eg.ersetzen(sym, ET.fromstring("<component object-ref='ET.Semiconductor.NPN_BC140'/>"))
        eigen = next(e for e in neu if e.get("ident") == "EIGEN")
        self.assertEqual(sum(1 for li in eigen.findall("line") if li.get("end-cap") == "arrow1"), 1)
        self.assertEqual(len(eigen.findall("circle")), 1)

    def _eigen(self, objekt, anschluesse, klasse=""):
        import eigene_grafik as eg
        sym = ET.fromstring("<symbol><symbol-layer ident='default'>" + "".join(
            f"<connector type='electric' ident='{i}' designation='{d}' cx='{x}' cy='{y}'/>"
            for i, (d, x, y) in enumerate(anschluesse, 1)) + "</symbol-layer></symbol>")
        neu = eg.ersetzen(sym, ET.fromstring(f"<component object-ref='{objekt}'/>"), klasse)
        return next(e for e in neu if e.get("ident") == "EIGEN")

    def test_motorschutzschalter_dreipolig_statt_spule(self):
        # FluidSIM klassifiziert ihn als solenoid-coil; gezeichnet werden drei Pole mit Wirkverbindung
        e = self._eigen("ET.Switch.MotorCircuitBreaker",
                        [("1", -12, -36), ("2", -12, 36), ("3", 12, -36), ("4", 12, 36), ("5", 36, -36), ("6", 36, 36)],
                        "solenoid-coil")
        self.assertTrue(any(li.get("style") == "dash" for li in e.findall("line")))
        self.assertEqual([t.text for t in e.findall("text")], ["I>"])

    def test_drehstrommotor_kreis_mit_kennzeichen(self):
        e = self._eigen("ET.Actuator.ThreePhaseSynchronousMotor2", [("", -6, -18), ("", 18, -18), ("", 6, 48)])
        self.assertEqual(len(e.findall("circle")), 1)
        self.assertEqual([t.text for t in e.findall("text")], ["MS", "3~"])

    def test_unbekanntes_bauteil_ohne_namen(self):
        e = self._eigen("ET.Controller.ControllerPID", [("", -24, 0), ("", 24, 0)])
        self.assertEqual([t.text for t in e.findall("text")], ["PID"])
        e = self._eigen("ET.Foo.SomethingUnknown", [("", -24, 0), ("", 24, 0)])
        self.assertEqual(e.findall("text"), [])


class Werte(unittest.TestCase):
    def test_simulationsgroessen_und_nullwerte_nicht_im_plan(self):
        tf = {"T1": ET.fromstring("<text-box value='P_out = 0 kW'/>"), "T2": ET.fromstring("<text-box value='0 V'/>"),
              "T3": ET.fromstring("<text-box value='p0 = 6 bar'/>")}
        o = ET.fromstring("<component><user-data>"
                          "<attribute name='^#P_out' comment='~FTParam~FTPVar~FTPUnit' display-ref='T1'/>"
                          "<attribute name='^#voltage_DC' comment='~FTParam~FTPUnit' display-ref='T2'/>"
                          "<attribute name='^#p0' comment='~FTParam~FTPUnit' display-ref='T3'/>"
                          "</user-data></component>")
        self.assertEqual([w for w, _ in fp.sichtbare_werte(o, tf)], ["p0 = 6 bar"])


if __name__ == "__main__":
    unittest.main()
