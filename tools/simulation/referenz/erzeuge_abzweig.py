"""Erzeugt plaene/zylinder_abzweig.qet aus plaene/zylinder_b_offen.qet.

Die Zuleitung von -PZ1 zu -QM1 läuft über einen Leitungsabzweig (Elementtyp "junction",
Upstream 20.11.2026): Quelle W -> Abzweig -> E -> Ventil 1, der dritte Anschluss S bleibt frei.
Die Simulation muss den Abzweig als einen Knoten rechnen; der Zylinder fährt dann genauso
aus wie ohne Abzweig (Prüfung in simulationreference.cpp).

    python erzeuge_abzweig.py
"""
from pathlib import Path

HIER = Path(__file__).resolve().parent / "plaene"
QUELLE, ZIEL = HIER / "zylinder_b_offen.qet", HIER / "zylinder_abzweig.qet"

J_ELEM = "{5b1e0c6a-7f39-4c1e-9a52-0d4c2f6a1e01}"
J_DEF = "{5b1e0c6a-7f39-4c1e-9a52-0d4c2f6a1e02}"
T_W, T_E, T_S = ("{5b1e0c6a-7f39-4c1e-9a52-0d4c2f6a1e1%d}" % i for i in range(3))
C_NEU = "{5b1e0c6a-7f39-4c1e-9a52-0d4c2f6a1e20}"

DEFINITION = f"""        <element name="abzweig_pneumatik.elmt">
            <definition height="20" hotspot_x="10" hotspot_y="10" link_type="junction" type="element" version="0.100.0" width="20">
                <uuid uuid="{J_DEF}"/>
                <names>
                    <name lang="de">Abzweig Pneumatik</name>
                    <name lang="en">Pneumatic junction</name>
                </names>
                <kindInformations>
                    <kindInformation name="junction_source">{T_W}</kindInformation>
                    <kindInformation name="junction_target_1">{T_E}</kindInformation>
                    <kindInformation name="junction_target_2">{T_S}</kindInformation>
                </kindInformations>
                <informations>Referenzplan der Simulation</informations>
                <description>
                    <ellipse antialias="true" height="3" style="line-style:normal;line-weight:normal;filling:black;color:black" width="3" x="-1.5" y="-1.5"/>
                    <terminal medium="pneumatic" name="W" orientation="w" type="Generic" uuid="{T_W}" x="0" y="0"/>
                    <terminal medium="pneumatic" name="E" orientation="e" type="Generic" uuid="{T_E}" x="0" y="0"/>
                    <terminal medium="pneumatic" name="S" orientation="s" type="Generic" uuid="{T_S}" x="0" y="0"/>
                </description>
            </definition>
        </element>
"""

INSTANZ = f"""            <element freezeLabel="false" is_movable="1" orientation="0" prefix="" type="embed://import/abzweig_pneumatik.elmt" uuid="{J_ELEM}" x="240" y="400" z="10">
                <terminals/>
                <inputs/>
                <elementInformations/>
                <dynamic_texts/>
                <texts_groups/>
            </element>
"""


def main():
    s = QUELLE.read_text(encoding="utf-8")
    # Abzweig-Definition direkt in die Kategorie "import"
    marke = '        <category name="import">\n'
    i = s.index(marke)
    j = s.index("</names>\n", i) + len("</names>\n")
    s = s[:j] + DEFINITION + s[j:]
    # Instanz vor </elements>
    s = s.replace("        </elements>\n", INSTANZ + "        </elements>\n", 1)
    # Leitung -PZ1 -> -QM1 aufteilen: -PZ1 -> Abzweig W, Abzweig E -> -QM1
    a = s.index('<conductor ', s.index("<conductors>"))
    e = s.index("</conductor>", a) + len("</conductor>")
    alt = s[a:e]
    assert 'element1_label="-PZ1"' in alt and 'element2_label="-QM1"' in alt, alt[:200]
    zu_abzweig = (alt.replace('element2="{9dd18712-3694-4b74-846f-673cb82ec26b}"', f'element2="{J_ELEM}"')
                  .replace('element2_label="-QM1"', 'element2_label=""')
                  .replace('element2_name="5/2e [elec]-[spring]"', 'element2_name="Abzweig Pneumatik"')
                  .replace('terminal2="{fa41901e-252d-4b01-83c8-1f76cdb5c7d9}"', f'terminal2="{T_W}"')
                  .replace('terminalname2="1"', 'terminalname2="W"'))
    ab_abzweig = (alt.replace('element1="{dfab4cc9-5b86-448d-8646-8cfa5a3273fe}"', f'element1="{J_ELEM}"')
                  .replace('element1_label="-PZ1"', 'element1_label=""')
                  .replace('element1_name="Pneumatic energy source"', 'element1_name="Abzweig Pneumatik"')
                  .replace('terminal1="{80922545-03a6-4f12-83c4-e963a497feae}"', f'terminal1="{T_E}"')
                  .replace('terminalname1="1"', 'terminalname1="E"')
                  .replace('uuid="{2c617555-99c3-4203-9902-2165db4ef37d}"', f'uuid="{C_NEU}"'))
    assert zu_abzweig != alt and ab_abzweig != alt
    s = s[:a] + zu_abzweig + "\n            " + ab_abzweig + s[e:]
    ZIEL.write_text(s, encoding="utf-8", newline="\n")
    print(ZIEL)


if __name__ == "__main__":
    main()
