"""Beispielblatt „Einspeisung“ in QET zeichnen (Skill qet-schaltplan-zeichnen).

Ausführen mit einem Python, das qet_mcp laden kann (siehe qet_direkt.py).
"""
import os
import sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from qet_direkt import *  # q, BIN, USER, COMMON, PROJ

F = 0
E = "common://10_electric/10_allpole/"
QS = E + "200_fuses_protective_gears/12_magneto_thermal_circuit_breakers/fa4202_disjoncteur_moteur_3p.elmt"
LS = E + "200_fuses_protective_gears/11_circuit_breakers/disjonct_m_1f.elmt"  # EPLAN-Form (Bimetall + Magnet)
PF = E + "100_folio_referencing/02going_arrow.elmt"
BOD = E + "114_connections/bod.elmt"
KL = E + "130_terminals_terminal_strips/borne_continuite2.elmt"
LUE = "custom://Allgemein/Lüfter/Filterlüfter_230VAC/Filterluefter_230V_AC_235m_h.elmt"  # L, N, PE
SI = "common://10_electric/11_singlepole/200_fuses_protective_gears/10_fuses/fusible.elmt"
TR = "custom://Murrelektronik/Transformatoren/86153/86153_Schaltbild.elmt"
TH = "custom://Finder/Thermostate/7T.81.0.000.2303/Thermostat_7T.81_Schliesser.elmt"
OUT = PROJ + r"\Beispiel_Einspeisung.qet"

# Anschlussindizes (oben->unten, dann links->rechts)
QS_IN = {1: 0, 3: 1, 5: 2}; QS_OUT = {2: 3, 4: 4, 6: 5}
BOD_N, BOD_W, BOD_E, BOD_S = 0, 1, 2, 3
TR_PIN = {"1": 0, "2": 1, "3": 2, "4": 3, "5": 4, "6": 5, "7": 6, "8": 7, "PE": 8,
          "9": 9, "11": 10, "10": 11, "12": 12}
# A3 quer wie das Original (1560 x 1050 px), 10 Spalten, keine Zeilenleiste; Lage aus dem Original umgerechnet
Y = {"L1": 90, "L2": 110, "L3": 130, "T1": 150, "T2": 170}
X_PF = 1440          # Pfeile nach Blatt 9, Ziel rechts daneben
XE = 210             # Einspeisung: Q81 / X0 / NSHV (Pins XE, XE+20, XE+40)
XP = 530             # PE-Schiene -PE:1 und PE der NSHV
XT = 410             # Steuertrafo: Q82 / T82 / Q82.1
XT1, XT2 = 850, 880  # T1, T2 senkrecht zu den Schienen
XL = 1350            # Lüfter: Mitte des Motors (L, N, PE bei -30, +30, +90)
YX, YW, YN = 740, 800, 840   # Klemmen X0/X2, Kabel W81, Oberkante NSHV
YF = 990             # Funktionstexte
LABEL, BIG, DATA = 10, 10, 7  # Schriftgrößen: Kennzeichen, Funktion/Verweis, Daten
SIZES = {}           # freier Text -> Schriftgröße (nach dem Zeichnen gesetzt)

ops = [{"op": "set_folio_title", "folio": F, "title": "Einspeisung"},
       {"op": "embed_title_block_template", "name": "MEIN_SCHRIFTFELD"},
       {"op": "set_folio", "folio": F, "property": "template", "value": "MEIN_SCHRIFTFELD"},
       {"op": "set_folio", "folio": F, "property": "plant", "value": "BA"},
       {"op": "set_folio", "folio": F, "property": "locmach", "value": "CA"},
       {"op": "set_folio", "folio": F, "property": "folio", "value": "8"},
       {"op": "set_folio_border", "folio": F, "property": "preset", "value": "a3-landscape"},
       {"op": "set_folio_border", "folio": F, "property": "columns", "value": "10"},
       {"op": "set_folio_border", "folio": F, "property": "column-width", "value": "156"},
       {"op": "set_folio_border", "folio": F, "property": "display-rows", "value": "false"},
       {"op": "set_conductor_default", "folio": F, "property": "displaytext", "value": "false"}]


def el(ident, path, x, y, label=None, label_x=None):
    out = [{"op": "add_element", "folio": F, "path": path, "x": x, "y": y, "id": ident}]
    if label is not None:
        out.append({"op": "set_label", "folio": F, "element": f"${ident}", "label": label})
    if label_x is not None:
        out.append({"op": "set_element_text", "folio": F, "element": f"${ident}", "index": 0,
                    "property": "x", "value": str(label_x)})
    return out


def w(a, ta, b, tb):
    return {"op": "add_conductor", "folio": F, "from": f"${a}", "from_terminal": ta, "to": f"${b}", "to_terminal": tb}


def etext(e, ident, text, x, y, size=5, rot=0):
    """Kleiner Text am Element (Koordinaten des Elements), z. B. Aderfarbe/Querschnitt an einer Leitung.

    Leiternummern gelten fürs ganze Potenzial (auch über Klemmen hinweg) und landen dann auf der
    Kabelader; ein Elementtext steht genau dort, wo er hin soll.
    """
    out = [{"op": "add_element_text", "folio": F, "element": f"${e}", "source": "text", "value": text,
            "x": x, "y": y, "id": ident},
           {"op": "set_element_text", "folio": F, "element": f"${e}", "index": f"${ident}",
            "property": "size", "value": str(size)}]
    if rot:
        out.append({"op": "set_element_text", "folio": F, "element": f"${e}", "index": f"${ident}",
                    "property": "rotation", "value": str(rot)})
    return out


def txt(t, x, y, size=DATA):
    SIZES[t] = size
    return {"op": "add_text", "folio": F, "text": t, "x": x, "y": y}


# Blattverweise L1-L3, T1, T2 -> Blatt 9: Ziel rechts neben dem Pfeil wie im Original
for n in Y:
    ops += el(f"pf{n}", PF, X_PF, Y[n], "")
    ops.append(txt(f"{n} / 9.0", X_PF + 16, Y[n] - 8, BIG))

# Hauptschalter -Q81, Sammelschienen 20 x 5 je Leiter an ihrem Anfang
ops += el("q81", QS, XE, 450, "-Q81")
ops += etext("q81", "q81a", "3VA2220", -85, 42, 6) + etext("q81", "q81b", "IR=140...200A", -85, 50, 6) \
    + etext("q81", "q81c", "LI=5...10 x IN", -85, 58, 6)
ops += el("bL2", BOD, XT, Y["L2"]) + el("bL3", BOD, XT + 20, Y["L3"])
ops += [w("q81", QS_IN[1], "pfL1", 0),
        w("q81", QS_IN[3], "bL2", BOD_W), w("bL2", BOD_E, "pfL2", 0),
        w("q81", QS_IN[5], "bL3", BOD_W), w("bL3", BOD_E, "pfL3", 0)]
for i, n in enumerate(("L1", "L2", "L3")):
    ops.append(txt(f"-{n}  20 x 5", XE + 20 * i + 6, Y[n] - 20))

# Einspeisung von der NSHV (außerhalb): X0, Kabel W81 mit Adern, NSHV mit L1..L3/PE und Sicherungen
for i in (1, 2, 3):
    x = XE + 20 * (i - 1)
    ops += el(f"x0_{i}", KL, x, YX, str(i), 4)
    ops += el(f"nL{i}", KL, x, YN + 20, f"L{i}", 4)
    ops += el(f"f{i}", SI, x, YN + 70, "")
    ops += etext(f"f{i}", f"f{i}a", str(2 * i - 1), 3, -21, 5) + etext(f"f{i}", f"f{i}b", str(2 * i), 3, 14, 5)
    ops += [w("q81", QS_OUT[2 * i], f"x0_{i}", 0), w(f"x0_{i}", 1, f"nL{i}", 0), w(f"nL{i}", 1, f"f{i}", 0)]
    ops += etext("q81", f"bk{i}", "BK 70", 20 * (i - 1) + 2, 150, 5, 270)
ops += [txt("-X0", XE - 40, YX - 6, LABEL), txt("NSHV", XE - 120, YN - 2, BIG), txt("200 A", XE - 50, YN + 62),
        {"op": "add_shape", "folio": F, "shape": "rectangle", "x1": XE - 70, "y1": YN, "x2": XP + 70, "y2": YN + 115}]
# PE: Schiene -PE:1 (20 x 5 mm) über dem Kabel, PE der NSHV darunter; Abzweig von T1 am Knoten
ops += el("pe1", KL, XP, YX, "-PE:1", -40)
ops += el("nshvPE", KL, XP, YN + 20, "PE", 4)
ops += [w("pe1", 1, "nshvPE", 0), txt("20 x 5 mm", XP + 20, YX - 14),
        {"op": "add_cable", "folio": F, "designation": "W81", "type": "ÖLFLEX CLASSIC 100 G 4x70 mm²",
         "core_colors": ["GNYE", "GY", "BK", "BN"], "length": "", "x1": XP + 30, "y1": YW, "x2": XE - 20, "y2": YW}]

# Steuertrafo: -Q82 -> -T82 -> -Q82.1
ops += el("q82", QS, XT, 190, "-Q82") + etext("q82", "q82t", "4,5...6,3A", -78, 4, 6)
ops += [w("bL2", BOD_S, "q82", QS_IN[1]), w("bL3", BOD_S, "q82", QS_IN[3]),
        w("q82", QS_OUT[4], "q82", QS_IN[5])]                       # 4-5 gebrückt
ops += el("t82", TR, XT, 370, "-T82", -60)  # Herstellersymbol aus dem EPLAN-Makro (Murr P8-EDZ)
ops.append({"op": "set_element_text", "folio": F, "element": "$t82", "index": 0, "property": "y", "value": "-14"})
ops += [w("q82", QS_OUT[2], "t82", TR_PIN["1"]), w("q82", QS_OUT[6], "t82", TR_PIN["6"]),
        w("t82", TR_PIN["2"], "t82", TR_PIN["5"]),                   # Primär 2-5
        w("t82", TR_PIN["11"], "t82", TR_PIN["10"])]                 # Sekundär 11-10 (Reihe)
# die Brücken liefen auf der Rahmenlinie (Anschlüsse liegen im Kasten): waagerechtes Stück herausschieben
ops += [{"op": "move_conductor_segment", "folio": F, "element": "$t82", "terminal": TR_PIN["2"], "segment": 2, "dx": 0, "dy": -15},
        {"op": "move_conductor_segment", "folio": F, "element": "$t82", "terminal": TR_PIN["11"], "segment": 1, "dx": 0, "dy": 15}]
ops += etext("q82", "bkq1", "BK 1,5", 2, 130, 5, 270) + etext("q82", "bkq2", "BK 1,5", 42, 100, 5, 270)
ops += el("q821", QS, XT, 560, "-Q82.1") + etext("q821", "q821t", "7...10A", -70, 4, 6)
ops += [w("t82", TR_PIN["9"], "q821", QS_IN[1]), w("t82", TR_PIN["12"], "q821", QS_IN[3]),
        w("q821", QS_OUT[4], "q821", QS_IN[5])]
# T1 (von 6) mit Knoten zur PE-Schiene, T2 (von 2); senkrecht rechts vom Trafo zu den Schienen
ops += el("bNode", BOD, 790, 640) + el("bT1u", BOD, XT1, 640) + el("bT1o", BOD, XT1, Y["T1"])
ops += el("bT2u", BOD, XT2, 670) + el("bT2o", BOD, XT2, Y["T2"])
ops += [w("q821", QS_OUT[6], "bNode", BOD_W), w("bNode", BOD_E, "bT1u", BOD_W), w("bNode", BOD_S, "pe1", 0),
        w("bT1u", BOD_N, "bT1o", BOD_S),
        w("q821", QS_OUT[2], "bT2u", BOD_W), w("bT2u", BOD_N, "bT2o", BOD_S)]
ops += etext("bT1u", "t1q", "RDWH 1,5", 4, -60, 5, 270) + etext("bT2u", "t2q", "RD 1,5", 4, -90, 5, 270)

# Schrankbelüftung (außerhalb): -F88 an T2, -S88, -X2, Kabel W88, -EM88; N von T1
ops += el("bT2a", BOD, XL - 30, Y["T2"]) + el("bT1a", BOD, XL + 30, Y["T1"])
ops += [w("bT1o", BOD_E, "bT1a", BOD_W), w("bT1a", BOD_E, "pfT1", 0),
        w("bT2o", BOD_E, "bT2a", BOD_W), w("bT2a", BOD_E, "pfT2", 0)]
ops += el("f88", LS, XL - 30, 240, "-F88")
ops += [{"op": "set_element_text", "folio": F, "element": "$f88", "index": i, "property": "text", "value": v}
        for i, v in ((1, ""), (2, "B6A"))]  # Feld 2 = Charakteristik/Nennstrom
ops += el("s88", TH, XL - 30, 450, "-S88", -50) + etext("s88", "s88t", "0-60°C", -60, 30, 6)
ops += el("x2_1", KL, XL - 30, YX, "1", 4) + el("x2_2", KL, XL + 30, YX, "2", 4) + el("x2_3", KL, XL + 90, YX, "3", 4)
ops += [w("bT2a", BOD_S, "f88", 0), w("f88", 1, "s88", 0), w("s88", 1, "x2_1", 0),
        w("bT1a", BOD_S, "x2_2", 0), txt("-X2", XL - 70, YX - 6, LABEL)]
ops += etext("f88", "f88q", "1,5", 2, 120, 5, 270) + etext("s88", "s88q", "1,5", 2, 160, 5, 270)
ops += el("em88", LUE, XL, 860, "-EM88", 25)
ops.append({"op": "set_element_text", "folio": F, "element": "$em88", "index": 0, "property": "y", "value": "45"})
ops += [w("x2_1", 1, "em88", 0), w("x2_2", 1, "em88", 1), w("x2_3", 1, "em88", 2),
        {"op": "set_conductor", "folio": F, "element": "$x2_3", "terminal": 1, "property": "style",
         "value": "dashed"},
        {"op": "add_cable", "folio": F, "designation": "W88", "type": "ÖLFLEX CLASSIC 110 G 3x1,5 mm²",
         "core_colors": ["GNYE", "2", "1"], "length": "", "x1": XL + 110, "y1": 775, "x2": XL - 60, "y2": 775}]

ops += [txt("Einspeisung 400V AC", XE, YF, BIG), txt("Steuertrafo 2x115V AC", XT, YF, BIG),
        txt("Schrankbelüftung", XL - 40, YF, BIG)]


def kabel_schrift(path):
    """Typ und Aderfarben der Kabel klein schreiben (Kabel-<texts> im .qet, wie Kabel > Textformat)."""
    import re
    s = open(path, encoding="utf-8").read()
    font = "Liberation Sans,{},-1,5,50,0,0,0,0,0,Regular"
    texts = ('<texts><text key="type" font="%s"/><text key="cores" font="%s"/></texts>'
             % (font.format(5), font.format(5)))
    s = re.sub(r'(<cable [^>]*[^/]>)(?!<texts>)', lambda m: m.group(1) + texts, s)
    open(path, "w", encoding="utf-8").write(s)


def schrift(path):
    """Einheitliche Schrift: Kennzeichen LABEL, freie Texte nach SIZES (Attribut und HTML)."""
    import re
    s = open(path, encoding="utf-8").read()

    def inp(m):
        tag = m.group(0)
        t = re.search(r'text-indent:0px;&quot;(?:>|&gt;)(.*?)&lt;/p', tag)
        size = SIZES.get(t.group(1).replace("&amp;", "&") if t else "", DATA)
        tag = re.sub(r'font="Liberation Sans,\d+,', f'font="Liberation Sans,{size},', tag)
        return re.sub(r'font-size:\d+pt', f'font-size:{size}pt', tag)
    # QET maskiert ">" im Text-Attribut nicht, "<" schon: das Element reicht bis zum nächsten "<"
    s = re.sub(r'<input [^<]*', inp, s)

    def element(m):
        # Klemmen tragen nur ihre Nummer: klein wie die Anschlussnummern, Geräte LABEL
        size = DATA if "borne" in m.group(1) else LABEL

        def lab(t):
            block = t.group(0)
            if "<info_name>label</info_name>" not in block:
                return block
            return re.sub(r'font="Liberation Sans,\d+,', f'font="Liberation Sans,{size},', block, count=1)
        return re.sub(r'<dynamic_elmt_text .*?</dynamic_elmt_text>', lab, m.group(0), flags=re.S)
    s = re.sub(r'<element [^>]*type="([^"]*)".*?</element>', element, s, flags=re.S)
    open(path, "w", encoding="utf-8").write(s)


if __name__ == "__main__":
    q.tool_project_new(BIN, PROJ + r"\leer.qet", title="Beispielanlage", folios=1,
                       author="", overwrite=True)
    r = q.tool_edit(BIN, PROJ + r"\leer.qet", ops, OUT, timeout=300)
    kabel_schrift(OUT)
    schrift(OUT)
    fail = [(o["index"], o["op"]) for o in r.get("operations", []) if not o.get("succeeded")]
    print("ok:", r.get("ok"), "ops:", len(r.get("operations", [])), "fehl:", fail[:8], r.get("hint"))
    print("pdf:", q.tool_export(BIN, OUT, "pdf", OUT.replace(".qet", ".pdf")).get("ok"))
    lc = q.tool_layout_check(BIN, OUT)
    print("layout:", lc.get("score"), {k: v for k, v in lc.get("summary", {}).items() if v and k != "flow"})
    for f in lc.get("findings", [])[:25] if isinstance(lc.get("findings"), list) else []:
        print("  ", f.get("rule"), f.get("labels") or f.get("label"), (f.get("note") or "")[:80])
