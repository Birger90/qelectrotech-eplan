"""Beispielblatt „Fussschalter 1“ in QET zeichnen (Skill qet-schaltplan-zeichnen).

Ablauf: leeres Projekt -> alle Operationen in einem tool_edit -> PDF-Export -> layout_check.
Ausführen mit einem Python, das qet_mcp laden kann (siehe qet_direkt.py).
"""
import os
import sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from qet_direkt import *  # q (qet_mcp), BIN, USER, COMMON, PROJ

F = 0  # Scripting-API: Blätter ab 0
FS = "custom://Fiessler/Befehlsgeräte/FE-FS2/Sicherheits-Fussschalter_FE-FS2.elmt"
FDI = "custom://Siemens/SPS/6ES7136-6BA01-0CA0/6ES7136-6BA01-0CA0_DI-VS_B1.elmt"
DIAUX = "custom://Siemens/SPS/6ES7131-6BH01-0BA0/6ES7131-6BH01-0BA0_DI-AUX_C1.elmt"
KL = "common://10_electric/10_allpole/130_terminals_terminal_strips/borne_continuite2.elmt"
ST = "common://10_electric/10_allpole/140_connectors_plugs/01_connectors_pins/prise_fiche.elmt"

# Spalten: SPS-Kanäle im 120-px-Raster (Kasten 110 px breit), DI/VS 20 px auseinander,
# links Platz für die Kabelbeschriftung (Kabel beginnt bei x=95).
PX = {1: 110, 2: 130, 3: 230, 4: 250, 5: 350, 6: 370, 7: 470, 8: 490, 9: 570, 10: 590,
      11: 650, 12: 670, 13: 770, 14: 790, 15: 890, 16: 910}
LEFT_OF_PAIR = {1, 3, 5, 7, 9, 11, 13, 15}
Y_S180, Y_W181, Y_ST, Y_W180, Y_KL, Y_SPS, Y_TXT = 90, 205, 260, 330, 390, 480, 625
OUT = PROJ + r"\Beispiel_Fussschalter.qet"

ops = [{"op": "set_folio_title", "folio": F, "title": "Fussschalter 1"},
       {"op": "embed_title_block_template", "name": "MEIN_SCHRIFTFELD"},
       {"op": "set_folio", "folio": F, "property": "template", "value": "MEIN_SCHRIFTFELD"},
       {"op": "set_folio", "folio": F, "property": "plant", "value": "CA"},
       {"op": "set_folio", "folio": F, "property": "locmach", "value": "FP1"},
       {"op": "set_folio", "folio": F, "property": "folio", "value": "18"},
       {"op": "set_conductor_default", "folio": F, "property": "displaytext", "value": "false"},
       {"op": "add_element", "folio": F, "path": FS, "x": 110, "y": Y_S180, "id": "s180"},
       {"op": "set_label", "folio": F, "element": "$s180", "label": "-S180"}]
for i, x in PX.items():
    left = i in LEFT_OF_PAIR
    ops += [{"op": "add_element", "folio": F, "path": ST, "x": x, "y": Y_ST, "id": f"xs{i}"},
            {"op": "set_label", "folio": F, "element": f"$xs{i}", "label": str(i)},
            # Nummer beim linken Stift links, beim rechten rechts: nie auf einer Leitung
            {"op": "set_element_text", "folio": F, "element": f"$xs{i}", "index": 0, "property": "x",
             "value": str(-8 - 8 * len(str(i)) if left else 4)},
            {"op": "add_element", "folio": F, "path": KL, "x": x, "y": Y_KL, "id": f"x8_{i}"},
            {"op": "set_label", "folio": F, "element": f"$x8_{i}", "label": str(i)},
            {"op": "set_element_text", "folio": F, "element": f"$x8_{i}", "index": 0, "property": "x",
             "value": str(-6 - 6 * len(str(i)) if left else 4)},
            {"op": "add_conductor", "folio": F, "from": f"$xs{i}", "from_terminal": 1, "to": f"$x8_{i}", "to_terminal": 0}]
ops += [{"op": "add_text", "folio": F, "text": t, "x": 25, "y": y}
        for t, y in (("-XS180", Y_ST - 8), ("-XB180", Y_ST + 12), ("-X8", Y_KL + 2))]
# Fußschalter: Anschlussindex (links nach rechts) -> Steckerstift; K3 11/12 = Reserve
for pin, t in {1: 0, 2: 1, 3: 2, 4: 3, 5: 4, 6: 5, 7: 6, 8: 7, 11: 10, 12: 11}.items():
    ops.append({"op": "add_conductor", "folio": F, "from": "$s180", "from_terminal": t, "to": f"$xs{pin}", "to_terminal": 0})
ops += [{"op": "add_conductor", "folio": F, "from": "$xs13", "from_terminal": 0, "to": "$xs14", "to_terminal": 0},
        {"op": "add_conductor", "folio": F, "from": "$xs15", "from_terminal": 0, "to": "$xs16", "to_terminal": 0}]
kan = [("a9a", FDI, "-A9", "E90.3", "FS1 abwärts K1", (1, 2)), ("a9b", FDI, "-A9", "E90.7", "FS1 abwärts K2", (3, 4)),
       ("a10a", FDI, "-A10", "E100.0", "FS1 Stop K1", (5, 6)), ("a10b", FDI, "-A10", "E100.4", "FS1 Stop K2", (7, 8)),
       ("sb", DIAUX, "=SB-A2", "E20.4", "FS1 rechts aufwärts", (11, 12)),
       ("a12a", FDI, "-A12", "E120.1", "FS1 gesteckt K1", (13, 14)), ("a12b", FDI, "-A12", "E120.5", "FS1 gesteckt K2", (15, 16))]
for ident, path, label, addr, func, pins in kan:
    ops += [{"op": "add_element", "folio": F, "path": path, "x": PX[pins[0]], "y": Y_SPS, "id": ident},
            {"op": "set_label", "folio": F, "element": f"${ident}", "label": label},
            # Kennzeichen über den eigenen Kasten, rechts neben der VS-Leitung
            {"op": "set_element_text", "folio": F, "element": f"${ident}", "index": 0, "property": "x", "value": "30"}]
    for idx, val in zip((1, 2, 3), (addr, "", func)):  # 0 = Kennzeichen, 1-3 = EPLAN-Felder
        ops.append({"op": "set_element_text", "folio": F, "element": f"${ident}", "index": idx, "property": "text", "value": val})
    for t, pin in enumerate(pins):
        ops.append({"op": "add_conductor", "folio": F, "from": f"$x8_{pin}", "from_terminal": 1, "to": f"${ident}", "to_terminal": t})
ops += [{"op": "add_cable", "folio": F, "designation": "W181", "type": "12x1 mm²", "core_colors": [],
         "length": "6 m", "x1": 1000, "y1": Y_W181, "x2": 95, "y2": Y_W181},
        {"op": "add_cable", "folio": F, "designation": "W180", "type": "25x1 mm²", "core_colors": [],
         "length": "10 m", "x1": 1000, "y1": Y_W180, "x2": 95, "y2": Y_W180}]
for text, x in (("Presse abwärts", 130), ("Presse Stop", 360), ("Presse aufwärts", 620), ("Stecker gesteckt", 800)):
    ops.append({"op": "add_text", "folio": F, "text": text, "x": x, "y": Y_TXT})

if __name__ == "__main__":
    q.tool_project_new(BIN, PROJ + r"\leer.qet", title="Beispielanlage", folios=1,
                       author="", overwrite=True)
    r = q.tool_edit(BIN, PROJ + r"\leer.qet", ops, OUT, timeout=300)
    fail = [(o["index"], o["op"]) for o in r.get("operations", []) if not o.get("succeeded")]
    print("ok:", r.get("ok"), "ops:", len(r.get("operations", [])), "fehl:", fail[:8], r.get("hint"))
    print("pdf:", q.tool_export(BIN, OUT, "pdf", OUT.replace(".qet", ".pdf")).get("ok"))
    lc = q.tool_layout_check(BIN, OUT)
    print("layout:", lc.get("score"), {k: v for k, v in lc.get("summary", {}).items() if v and k != "flow"})
    for f in lc.get("findings", [])[:20] if isinstance(lc.get("findings"), list) else []:
        print("  ", f.get("rule"), f.get("labels") or f.get("label"), (f.get("note") or "")[:90])
