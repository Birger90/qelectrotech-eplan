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


if __name__ == "__main__":
    unittest.main()
