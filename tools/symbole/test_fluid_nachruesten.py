"""Tests für fluid_nachruesten.py:  python -m unittest tools/symbole/test_fluid_nachruesten.py"""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import fluid_nachruesten as fn  # noqa: E402

# 5/2-Ventil wie elements/50_pneumatic/.../502010602010_5_2_way_010.elmt: die Portnummern stehen jeweils links
# neben ihrer Anschlusslinie, die „1“ liegt dadurch näher an Anschluss 5 als die „5“; „12“ und „14“ beschriften
# die Betätigungsseiten.
VENTIL = """<definition version="0.90" type="element" link_type="simple" width="90" height="50" hotspot_x="42" hotspot_y="31">
    <uuid uuid="{64df94dd-03f5-171e-85de-1b917631e75b}"/>
    <description>
        <dynamic_text x="3" y="1" text_from="UserText" font="Liberation Sans,3,-1,5,0,0,0,0,0,0,Regular"><text>5</text></dynamic_text>
        <dynamic_text x="23" y="-24" text_from="UserText" font="Liberation Sans,3,-1,5,0,0,0,0,0,0,Regular"><text>2</text></dynamic_text>
        <dynamic_text x="-35" y="-20" text_from="UserText" font="Liberation Sans,3,-1,5,0,0,0,0,0,0,Regular"><text>14</text></dynamic_text>
        <dynamic_text x="32" y="-20" text_from="UserText" font="Liberation Sans,3,-1,5,0,0,0,0,0,0,Regular"><text>12</text></dynamic_text>
        <dynamic_text x="23" y="1" text_from="UserText" font="Liberation Sans,3,-1,5,0,0,0,0,0,0,Regular"><text>3</text></dynamic_text>
        <dynamic_text x="13" y="1" text_from="UserText" font="Liberation Sans,3,-1,5,0,0,0,0,0,0,Regular"><text>1</text></dynamic_text>
        <dynamic_text x="3" y="-24" text_from="UserText" font="Liberation Sans,3,-1,5,0,0,0,0,0,0,Regular"><text>4</text></dynamic_text>
        <dynamic_text x="-35" y="-40" text_from="ElementInfo"><text></text><info_name>label</info_name></dynamic_text>
        <terminal uuid="{0d5afb67-53af-4352-927f-0f355bde32a3}" name="" x="10" y="10" orientation="s" type="Generic"/>
        <terminal uuid="{0eb4e305-c12d-4d7e-aec5-a95e45c51a16}" name="" x="30" y="-20" orientation="n" type="Generic"/>
        <terminal uuid="{96cd18ff-c198-4c40-b5bf-6b7994c06dc9}" name="" x="30" y="10" orientation="s" type="Generic"/>
        <terminal uuid="{cd1228b7-ce2b-4176-b8ae-56e60a0ec85c}" name="" x="10" y="-20" orientation="n" type="Generic"/>
        <terminal uuid="{fa41901e-252d-4b01-83c8-1f76cdb5c7d9}" name="" x="20" y="10" orientation="s" type="Generic"/>
    </description>
</definition>
"""


class Nachruesten(unittest.TestCase):
    def test_ventil_bekommt_seine_portnummern(self):
        neu, namen = fn.nachruesten(VENTIL, "pneumatic")
        self.assertEqual(namen, ["5", "2", "3", "4", "1"])
        self.assertEqual(neu.count('medium="pneumatic"'), 5)

    def test_neue_uuid_grafik_unveraendert(self):
        neu, _ = fn.nachruesten(VENTIL, "pneumatic")
        self.assertNotIn("64df94dd-03f5-171e-85de-1b917631e75b", neu)
        alt_ohne = [z for z in VENTIL.splitlines() if "<terminal" not in z and "<uuid" not in z]
        neu_ohne = [z for z in neu.splitlines() if "<terminal" not in z and "<uuid" not in z]
        self.assertEqual(alt_ohne, neu_ohne)

    def test_vorhandenes_medium_bleibt(self):
        text = VENTIL.replace('type="Generic"/>', 'type="Generic" medium="hydraulic"/>', 1)
        neu, _ = fn.nachruesten(text, "pneumatic")
        self.assertEqual(neu.count('medium="hydraulic"'), 1)
        self.assertEqual(neu.count('medium="pneumatic"'), 4)

    def test_doppelte_namen_werden_eindeutig(self):
        self.assertEqual(fn.eindeutig(["2", "1", "1", "", ""]), ["2", "1.1", "1.2", "", ""])
        self.assertEqual(fn.eindeutig(["1", "2", "3"]), ["1", "2", "3"])

    def test_uuid_bei_jedem_lauf_gleich(self):
        a, _ = fn.nachruesten(VENTIL, "pneumatic")
        b, _ = fn.nachruesten(VENTIL, "pneumatic")
        self.assertEqual(a, b)

    def test_ohne_text_kein_name(self):
        anschluesse = [(0.0, 0.0, "n")]
        self.assertEqual(fn.zuordnen(anschluesse, [(40.0, 40.0, "1")]), [None])


if __name__ == "__main__":
    unittest.main()
