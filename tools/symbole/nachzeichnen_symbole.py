"""Symbole für Teile ohne EPLAN-Herstellerdaten, nach vorhandenen Schaltplänen nachgezeichnet.

Erzeugt QET-Bauteile (.elmt) in der Benutzersammlung (oder --ziel), Ordner
Hersteller/Kategorie/Typ. Raster 10 px, Anschlüsse auf dem Raster, Beschriftung
wie im Plan.

    python tools/symbole/nachzeichnen_symbole.py [--ziel ORDNER]
"""
import argparse
import math
import os
import re
import uuid
import xml.etree.ElementTree as ET
from pathlib import Path

STYLE = "line-style:normal;line-weight:normal;filling:none;color:black"
FONT = "Liberation Sans,{size},-1,5,50,0,0,0,0,0,Regular"


def _u():
    return "{" + str(uuid.uuid4()) + "}"


def _n(v):
    v = round(v, 2)
    return str(int(v)) if v == int(v) else str(v)


def write_xml(root, path):
    ET.indent(root, space="    ")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(ET.tostring(root, encoding="unicode") + "\n", encoding="utf-8")


def ensure_dirs(base, levels):
    """Ordnerkette mit qet_directory-Dateien (Anzeigename je Ebene) anlegen."""
    d = base
    for name in levels:
        d = d / re.sub(r'[<>:"/\\|?*]', "-", name)
        d.mkdir(parents=True, exist_ok=True)
        f = d / "qet_directory"
        if not f.exists():
            r = ET.Element("qet-directory")
            nn = ET.SubElement(r, "names")
            for lang in ("de", "en"):
                ET.SubElement(nn, "name", {"lang": lang}).text = name
            write_xml(r, f)
    return d


def safe(name):
    for a, b in (("ä", "ae"), ("ö", "oe"), ("ü", "ue"), ("Ä", "Ae"), ("Ö", "Oe"), ("Ü", "Ue"), ("ß", "ss")):
        name = name.replace(a, b)
    return re.sub(r"[^A-Za-z0-9._+-]+", "_", name).strip("_")

USER = Path(os.environ.get("APPDATA", "")) / "QElectroTech" / "QElectroTech" / "elements"
DASHED = STYLE.replace("line-style:normal", "line-style:dashed")


class Elem:
    """Ein Bauteil aus Linien, Kreisen, Texten und Anschlüssen (Koordinaten in px)."""

    def __init__(self, name, label, info):
        self.name, self.label, self.info = name, label, info
        self.parts, self.terms, self.xs, self.ys = [], [], [], []

    def _box(self, *pts):
        for x, y in pts:
            self.xs.append(x)
            self.ys.append(y)

    def line(self, x1, y1, x2, y2, style=STYLE):
        self._box((x1, y1), (x2, y2))
        self.parts.append(("line", {"x1": _n(x1), "y1": _n(y1), "x2": _n(x2), "y2": _n(y2),
                                    "end1": "none", "end2": "none", "length1": "1.5", "length2": "1.5",
                                    "antialias": "false", "style": style}))

    def rect(self, x, y, w, h, style=STYLE):
        self.line(x, y, x + w, y, style)
        self.line(x + w, y, x + w, y + h, style)
        self.line(x + w, y + h, x, y + h, style)
        self.line(x, y + h, x, y, style)

    def circle(self, cx, cy, r):
        self._box((cx - r, cy - r), (cx + r, cy + r))
        self.parts.append(("ellipse", {"x": _n(cx - r), "y": _n(cy - r), "width": _n(2 * r),
                                       "height": _n(2 * r), "antialias": "true", "style": STYLE}))

    def arc(self, cx, cy, r, start, angle):
        """Kreisbogen um (cx, cy); Winkel in Grad gegen den Uhrzeigersinn ab 3 Uhr."""
        self._box((cx - r, cy - r), (cx + r, cy + r))
        self.parts.append(("arc", {"x": _n(cx - r), "y": _n(cy - r), "width": _n(2 * r),
                                   "height": _n(2 * r), "start": _n(start), "angle": _n(angle),
                                   "antialias": "true", "style": STYLE}))

    def text(self, x, y, s, size=5, rot=0, bold=False):
        self._box((x, y))
        font = FONT.format(size=size)
        if bold:
            font = font.replace(",50,", ",75,").replace("Regular", "Bold")
        self.parts.append(("text", {"x": _n(x), "y": _n(y), "text": s, "rotation": _n(rot),
                                    "font": font, "color": "#000000"}))

    def term(self, x, y, ori, name):
        self._box((x, y))
        self.terms.append({"x": x, "y": y, "orientation": ori, "name": name})

    def pin(self, x, y, ori, name, show=None):
        """Anschluss mit kurzem Stich und kleinem Kreis wie im EPLAN-Plan; Name außen."""
        dx, dy = {"n": (0, -10), "s": (0, 10), "e": (10, 0), "w": (-10, 0)}[ori]
        self.line(x, y, x + dx, y + dy)
        self.circle(x + dx * 0.5, y + dy * 0.5, 2)
        s = name if show is None else show
        if s:
            if ori in ("n", "s"):
                self.text(x + 2, y + dy + (-1 if ori == "n" else 3 + 3.2 * len(s)), s, rot=-90)
            else:
                self.text(x + dx + (2 if ori == "e" else -2 - 3.2 * len(s)), y - 2, s)
        self.term(x + dx, y + dy, ori, name)

    def write(self, base, levels, fname=None):
        names = [t["name"] for t in self.terms]
        dup = {n for n in names if names.count(n) > 1}
        assert not dup, f"{self.name}: doppelte Anschlussnamen {dup}"
        left, top = min(self.xs), min(self.ys)
        width, height = max(self.xs) - left, max(self.ys) - top
        pad = 20
        root = ET.Element("definition", {
            "version": "0.100.0", "type": "element", "link_type": "simple",
            "width": _n(math.ceil((width + 2 * pad) / 10) * 10),
            "height": _n(math.ceil((height + 2 * pad + 20) / 10) * 10),
            "hotspot_x": _n(round(-left + pad)), "hotspot_y": _n(round(-top + pad + 20))})
        ET.SubElement(root, "uuid", {"uuid": _u()})
        nn = ET.SubElement(root, "names")
        for lang in ("de", "en"):
            ET.SubElement(nn, "name", {"lang": lang}).text = self.name
        ei = ET.SubElement(root, "elementInformations")
        for k, v in dict(self.info, label=self.label).items():
            if v:
                ET.SubElement(ei, "elementInformation", {"name": k, "show": "1"}).text = v
        ET.SubElement(root, "informations").text = "Nachgezeichnet (QElectroTech EPLAN-Fork, tools/symbole)"
        desc = ET.SubElement(root, "description")
        for kind, attrs in self.parts:
            ET.SubElement(desc, kind, attrs)
        dyn = ET.SubElement(desc, "dynamic_text", {
            "Halignment": "AlignLeft", "Valignment": "AlignTop", "font": FONT.format(size=8),
            "frame": "false", "keep_visual_rotation": "false", "rotation": "0",
            "text_from": "ElementInfo", "text_width": "-1", "uuid": _u(),
            "x": _n(left), "y": _n(top - 20), "z": "20"})
        ET.SubElement(dyn, "text").text = ""
        ET.SubElement(dyn, "info_name").text = "label"
        for t in self.terms:
            ET.SubElement(desc, "terminal", {"name": t["name"], "orientation": t["orientation"],
                                             "type": "Generic", "uuid": _u(),
                                             "x": _n(t["x"]), "y": _n(t["y"])})
        folder = ensure_dirs(base, levels)
        out = folder / f"{safe(fname or self.name)}.elmt"
        write_xml(root, out)
        return out


def info(hersteller, bestell, typ, beschreibung):
    return {"manufacturer": hersteller, "manufacturer_reference": bestell,
            "designation": typ, "description": beschreibung}


# ---------------------------------------------------------------- Bausteine

def plc_box(name, label, inf, title, pins, width=110):
    """SPS-Anschlussbox wie im Plan: Signalname innen, Anschluss rechts mit Nummer."""
    e = Elem(name, label, inf)
    h = 20 * len(pins) + 20
    e.rect(-width, -20, width, h)
    e.text(-width + 4, -8, title, size=6)
    for i, (signal, nummer) in enumerate(pins):
        y = 10 + 20 * i
        e.text(-4 - 3.2 * len(signal), y + 2, signal)
        e.pin(0, y, "e", signal, show=nummer)
    return e


def contact(e, x, y, a, b, kind):
    """Kontakt senkrecht, Anschluss a oben (y), b unten (y+40). kind: 'no' | 'nc'."""
    e.line(x, y, x, y + 10)
    e.line(x, y + 30, x, y + 40)
    if kind == "no":
        e.line(x, y + 30, x - 8, y + 12)
    else:
        e.line(x, y + 10, x + 6, y + 10)
        e.line(x, y + 30, x + 8, y + 8)
    e.text(x + 3, y + 8, a.split(":")[-1])
    e.text(x + 3, y + 40, b.split(":")[-1])
    e.term(x, y, "n", a)
    e.term(x, y + 40, "s", b)


def contact_down(e, x, y, a, b, kind, w=20):
    """Kontakt mit beiden Enden unten (wie im EPLAN-Black-Box-Symbol): a unten links
    am Kontakt, b über einen Bügel w px rechts daneben. Anschlüsse bei y+50."""
    e.line(x, y + 30, x, y + 50)
    e.line(x, y + 10, x, y)
    e.line(x, y, x + w, y)
    e.line(x + w, y, x + w, y + 50)
    if kind == "no":
        e.line(x, y + 30, x - 8, y + 12)
    else:
        e.line(x, y + 10, x + 6, y + 10)
        e.line(x, y + 30, x + 8, y + 8)
    e.text(x - 12, y + 48, a.split(":")[-1])
    e.text(x + w + 2, y + 48, b.split(":")[-1])
    e.term(x, y + 50, "s", a)
    e.term(x + w, y + 50, "s", b)


def changeover(e, x, y, com, nc, no):
    """Wechsler: com unten, nc oben links, no oben rechts."""
    e.line(x, y + 40, x, y + 30)
    e.line(x, y + 30, x + 10, y + 12)
    e.line(x - 10, y, x - 10, y + 12)
    e.line(x - 10, y + 12, x - 4, y + 12)
    e.line(x + 10, y, x + 10, y + 10)
    for px, py, n, o in ((x, y + 40, com, "s"), (x - 10, y, nc, "n"), (x + 10, y, no, "n")):
        e.term(px, py, o, n)
        e.text(px + 2, py + (8 if o == "n" else -2), n)


def coil(e, x, y, a1="A1", a2="A2", timer=False):
    e.line(x, y, x, y + 12)
    e.rect(x - 12, y + 12, 24, 16)
    if timer:
        e.line(x - 12, y + 12, x - 4, y + 28)
        e.line(x - 4, y + 12, x - 12, y + 28)
    e.line(x, y + 28, x, y + 40)
    e.term(x, y, "n", a1)
    e.term(x, y + 40, "s", a2)
    e.text(x + 2, y + 8, a1)
    e.text(x + 2, y + 38, a2)


# ---------------------------------------------------------------- Bauteile

def cybelec(base):
    out = []
    inf = info("Cybelec", "N10.01.0173", "MVA-AA6", "CybMVA Verstärkermodul")
    lv = ["Cybelec", "Pressensteuerung", "MVA-AA6"]
    v = [("+24V DC", ""), ("GND", "")]
    groups = [("CPU Versorgung", v),
              ("CPU Digitalausgänge", [(f"DO1.{i}", f"OUT {i}") for i in range(1, 5)]),
              ("CPU Digitaleingänge", [(f"DI1.{i}", f"IN {i}") for i in range(1, 5)]),
              ("POWER", v),
              ("ANALOG Versorgung", v),
              ("ANALOG Pressure & Crowning", [("P+", "1"), ("P-", "2"), ("C-", "3"), ("C+", "4")]),
              ("ANALOG Analogausgänge", [(f"AO3.{i // 2 + 1}{'+-'[i % 2]}", str(i + 1)) for i in range(8)]),
              ("ANALOG Analogeingänge", [(f"AI3.{i // 2 + 1}{'+-'[i % 2]}", str(i + 1)) for i in range(8)])]
    for ax in (4, 5):
        groups += [(f"AX.{ax} Versorgung", v),
                   (f"AX.{ax} Digitalausgänge", [(f"DO{ax}.{i}", str(i)) for i in range(1, 9)]),
                   (f"AX.{ax} Digitaleingänge", [(f"DI{ax}.{i}", str(i)) for i in range(1, 9)]),
                   (f"AX.{ax} Analogausgänge", [(f"AO{ax}.{i // 2 + 1}{'+-'[i % 2]}", str(i + 1))
                                                for i in range(4)])]
    enc = ["0V (5V)", "\\Channel A", "\\Channel B", "\\Index", "V+", "Channel A", "Channel B", "Index"]
    for g in ("E1.1", "E1.2", "E4.1", "E4.2", "E5.1", "E5.2"):
        groups.append((f"Encoder {g}", [(s, f"Pin {i + 1}") for i, s in enumerate(enc)]))
    for title, pins in groups:
        e = plc_box(f"MVA-AA6 {title}", "-A", inf, title, pins)
        out.append(e.write(base, lv))
    return out


def pcss(base):
    out = []
    inf = info("LazerSafe", "N12.01.0022", "PCSS-A1", "LazerSafe PCSS-A1 Safety Controller")
    lv = ["LazerSafe", "Sicherheitssteuerung", "PCSS-A1"]
    cn = {"CN1": [f"Y0{i // 2}{'HL'[i % 2]}" for i in range(12)],
          "CN2": ["Y06", "Y07", "Y08", "Y09", "Y10", "Y11", "0VDC.1", "0VDC.2", "0VDC.3",
                  "24VDC.1", "24VDC.2", "24VDC.3"],
          "CN3": [f"X{i:02d}" for i in range(0, 12)],
          "CN4": [f"X{i:02d}" for i in range(12, 24)],
          "CN5": [f"X{i:02d}" for i in range(24, 36)],
          "CN6": [f"P{i:02d}" for i in range(24, 36)],
          "CN7": [f"X{i:02d}" for i in range(36, 44)] + ["", "", "", ""],
          "CN8": ["P36", "P37", "P38", "P39", "Y12", "Y13", "Y14", "Y15", "", "", "", ""]}
    for name, sig in cn.items():
        pins = [((s or f"{name}:{n}"), str(n)) for n, s in reversed(list(enumerate(sig, 1)))]
        e = plc_box(f"PCSS-A1 {name}", "-A", inf, name, pins)
        out.append(e.write(base, lv))
    for name, use in (("CN9A", "COM2"), ("CN11", "ENC1 OUT"), ("CN12", "ENC1 IN"), ("CN13", "ENC2 OUT"),
                      ("CN14", "ENC2 IN"), ("CN15", "004 RX"), ("CN16", "004 TX"), ("CN17", "005 RX"),
                      ("CN18", "005 TX")):
        pins = [(f"{name}:{n}", str(n)) for n in range(9, 0, -1)]
        e = plc_box(f"PCSS-A1 {name} {use}", "-A", inf, f"{name} {use}", pins, width=70)
        out.append(e.write(base, lv))
    return out


def irt(base):
    e = Elem("Servomotor Drive 4005", "-T", info("IRT SA", "N03.01.0005", "4005", "Servomotor Drive"))
    w = 760
    e.rect(0, 0, w, 160)
    e.text(300, 70, "Servomotor Drive", size=9)
    e.text(340, 86, "4005", size=9)
    for i, n in enumerate(("L1", "L2", "L3", "PE")):
        e.pin(20 + 20 * i, 0, "n", n)
    # XSTO Sicherheitsrelais
    e.rect(620, 0, 100, 30)
    e.text(624, 40, "XSTO Safety Relay 24V DC", size=4)
    e.pin(640, 0, "n", "XSTO:+", show="+")
    e.pin(700, 0, "n", "XSTO:-", show="-")
    x = 20
    e.text(x, 150, "XMOTOR")
    for n, s in (("1", "U"), ("2", "V"), ("3", "W"), ("4", "PE")):
        e.text(x - 3, 140, s)
        e.pin(x, 160, "s", f"XMOTOR:{n}", show=n)
        x += 20
    e.pin(x + 20, 160, "s", "SH1", show="SH")
    x += 60
    e.text(x, 130, "CO2")
    for n, s in (("4", "COS1"), ("8", "COS2"), ("3", "SIN1"), ("7", "SIN2"), ("5", "REFOUT"),
                 ("9", "REFOUTB"), ("6", "TH2"), ("2", "TH1"), ("1", "GND")):
        e.text(x + 2, 150, s, rot=-90)
        e.pin(x, 160, "s", f"CO2:{n}", show=n)
        x += 20
    e.pin(x + 10, 160, "s", "SH2", show="SH")
    x += 50
    e.text(x, 130, "XCOD")
    for n, s in (("11", "A"), ("2", "GND"), ("10", "A\\"), ("3", "A'"), ("9", "B"), ("4", "A'\\"),
                 ("8", "B\\"), ("5", "B'"), ("7", "Z"), ("13", "B'\\"), ("6", "Z\\"), ("14", "Z'"),
                 ("12", "GND"), ("15", "Z'\\"), ("1", "GND")):
        e.text(x + 2, 150, s, rot=-90)
        e.pin(x, 160, "s", f"XCOD:{n}", show=n)
        x += 20
    e.pin(x + 10, 160, "s", "SH3", show="SH")
    return [e.write(base, ["IRT", "Antriebe", "4005"])]


def fiessler(base):
    e = Elem("Sicherheits-Fußschalter FE-FS2", "-S",
             info("Fiessler Elektronik", "FE-FS2-SU1ASDU1/U1-U-RD", "FE-FS2-SU1ASDU1/U1-U-RD",
                  "Sicherheits-Fußschalter, zweipedalig"))
    # beide Enden jedes Kontakts unten; Abstände wie die Spalten des Blatts 18
    # (SPS-Kanäle im 120-px-Raster, DI/VS 20 px auseinander, Reserve 9/10 bei 460)
    e.rect(-30, -20, 690, 90, DASHED)
    e.text(-26, -8, "-K1", size=6)
    k1 = [("K1:21", "K1:22", "nc"), ("K1:13", "K1:14", "no"), ("K1:11", "K1:12", "nc"), ("K1:23", "K1:24", "no")]
    for i, (a, b, k) in enumerate(k1):
        contact_down(e, 120 * i, 0, a, b, k)
    e.line(-8, 20, 372, 20, DASHED)
    e.line(420, -20, 420, 70, DASHED)
    e.text(424, -8, "-K3", size=6)
    contact_down(e, 460, 0, "K3:11", "K3:12", "nc")
    contact_down(e, 540, 0, "K3:23", "K3:24", "no")
    e.line(452, 20, 552, 20, DASHED)
    e.pin(620, 70, "s", "PE")
    return [e.write(base, ["Fiessler", "Befehlsgeräte", "FE-FS2"])]


def murr_trafo(base):
    e = Elem("MST Steuer- und Trenntransformator 2000VA", "-T",
             info("Murrelektronik", "86153", "MST 2000-208...550/2x115", "Einphasen Steuer- und Trenntransformator"))
    e.rect(0, 0, 340, 120)
    for i in range(8):
        e.pin(20 + 30 * i, 0, "n", str(i + 1))
    e.pin(260, 0, "n", "PE")
    e.text(4, 10, "IN")
    e.rect(20, 40, 60, 8)
    e.rect(110, 40, 150, 8)
    e.rect(20, 66, 60, 8)
    e.rect(170, 66, 60, 8)
    for x, n in ((20, "9"), (80, "11"), (170, "10"), (230, "12")):
        e.pin(x, 120, "s", n)
    e.text(4, 112, "OUT")
    e.text(270, 60, "P: 2000VA")
    e.text(270, 72, "IN: 208...550VAC")
    e.text(270, 84, "OUT: 2x115VAC")
    return [e.write(base, ["Murrelektronik", "Transformatoren", "86153"], "86153 Schaltbild")]


def finder(base):
    out = []
    e = Elem("Zeitschaltuhr 12.51", "-K", info("Finder", "12.51.8.230.0000", "12.51.8.230.0000",
                                                 "Digitale Zeitschaltuhr 1 Kanal 16A 230VAC"))
    coil(e, 0, 0, timer=True)
    changeover(e, 50, 0, "11", "12", "14")
    e.line(12, 20, 46, 20, DASHED)
    out.append(e.write(base, ["Finder", "Zeitschaltuhren", "12.51.8.230.0000"]))
    e = Elem("Thermostat 7T.81 Schließer", "-S", info("Finder", "7T.81.0.000.2303", "7T.81.0.000.2303",
                                                       "Thermostat Schließer 250VAC/10A, 0-60°C"))
    contact(e, 0, 0, "13", "14", "no")
    e.rect(-30, 12, 14, 14)
    e.text(-29, 23, "ϑ", size=8)
    e.line(-16, 19, -6, 19, DASHED)
    out.append(e.write(base, ["Finder", "Thermostate", "7T.81.0.000.2303"]))
    return out


def phoenix(base):
    out = []
    e = Elem("VIP-2/SC/D9SUB/M Übergabemodul", "-XSUB",
             info("Phoenix Contact", "2315117", "VIP-2/SC/D9SUB/M", "Übergabemodul D-SUB 9 Stift"))
    e.rect(0, 0, 220, 40)
    e.text(4, 22, "D-SUB 9")
    for i in range(9):
        e.pin(20 + 20 * i, 0, "n", f"D{i + 1}", show=str(i + 1))
        e.pin(20 + 20 * i, 40, "s", str(i + 1))
        e.line(20 + 20 * i, 0, 20 + 20 * i, 40, DASHED)
    e.pin(200, 0, "n", "SHD")
    e.pin(200, 40, "s", "M")
    out.append(e.write(base, ["Phoenix Contact", "Klemmen", "VIP-2-SC-D9SUB-M"]))
    e = Elem("PLC-RPT-24DC/21 Relaismodul", "-K",
             info("Phoenix Contact", "2900299", "PLC-RPT-24DC/21", "PLC-INTERFACE Relais 24VDC 1 Wechsler"))
    coil(e, 0, 0, a1="A1", a2="A2")
    changeover(e, 50, 0, "11", "12", "14")
    e.line(12, 20, 46, 20, DASHED)
    out.append(e.write(base, ["Phoenix Contact", "Relais, Schütze", "PLC-RPT-24DC-21"]))
    return out


def harting(base):
    out = []
    for art, typ, art_name in (("09 32 018 3001", "Han 18 EE-M-C", "Stifteinsatz"),
                               ("09 32 018 3101", "Han 18 EE-F-C", "Buchseneinsatz")):
        e = Elem(f"{typ} {art_name} 18-polig", "-X", info("HARTING", art, typ, f"{art_name} 18 POL Crimp"))
        e.rect(0, 0, 400, 30)
        e.text(4, 22, typ)
        for i in range(18):
            e.pin(20 + 20 * i, 0, "n", str(i + 1))
        e.pin(390, 0, "n", "PE")
        out.append(e.write(base, ["HARTING", "Steckverbinder", typ.replace(" ", "-")]))
    return out


def petzold(base):
    out = []
    e = Elem("Schaltschrankleuchte LED-13D-T-OS", "-E",
             info("Petzold&Otto", "LED-13D-T-OS", "LED-13D-T-OS", "Schaltschrankleuchte 13W 110-240VAC"))
    e.circle(30, 40, 12)
    e.line(22, 32, 38, 48)
    e.line(38, 32, 22, 48)
    for x, n in ((10, "L"), (30, "N"), (50, "PE")):
        e.pin(x, 0, "n", n)
    e.line(10, 0, 10, 20)
    e.line(10, 20, 24, 30)
    e.line(30, 0, 30, 28)
    e.line(50, 0, 50, 20)
    out.append(e.write(base, ["Petzold&Otto", "Schaltschrankzubehör", "LED-13D-T-OS"]))
    e = Elem("Schaltschranksteckdose AST 11 LED", "-X",
             info("Petzold&Otto", "AST 11 LED", "AST 11 LED", "Schuko-Steckdose 16A mit LED, Hutschiene"))
    e.rect(0, 20, 60, 30)
    e.text(6, 40, "Schuko")
    for x, n in ((10, "L"), (30, "N"), (50, "PE")):
        e.pin(x, 20, "n", n)
    out.append(e.write(base, ["Petzold&Otto", "Schaltschrankzubehör", "AST 11 LED"]))
    return out


def luefter(base):
    """Schrankbelüftung wie im Plan: M 1~ mit L/N oben, PE rechts (gestrichelt), Lüfterrad darunter.

    Anschlüsse L, N, PE im 60-px-Raster (-30, 30, 90), damit Klemme, Kabel und Motor senkrecht fluchten.
    """
    e = Elem("Filterlüfter 230V AC 235m³/h", "-EM",
             info("", "", "Filterlüfter 230V AC", "Schaltschranklüfter 230V AC, 235 m³/h"))
    e.circle(0, 0, 20)
    e.text(-5, 0, "M", size=9)
    e.text(-6, 12, "1~", size=8)
    for x, d, n in ((-30, -1, "L"), (30, 1, "N")):
        e.term(x, -60, "n", n)
        e.line(x, -60, x, -30)
        e.line(x, -30, 14.1 * d, -14.1)
        e.text(x + 3, -34, n)
    e.line(14.1, 14.1, 22, 22)
    e.line(18, 26, 26, 18)
    e.line(22, 22, 22, 34)
    e.text(25, 31, "PE")
    e.line(22, 34, 90, 34, DASHED)
    e.term(90, 34, "n", "PE")
    e.line(0, 20, 0, 30, DASHED)
    e.circle(0, 50, 20)
    e.line(-12, 66, -2, 31)
    e.line(12, 66, 2, 31)
    e.text(25, 68, "230V AC", size=6)  # rechts neben dem Lüfterrad, unter dem Kennzeichen
    e.text(25, 77, "235m³/h", size=6)
    return [e.write(base, ["Allgemein", "Lüfter", "Filterlüfter_230VAC"])]


def fi_ls(base):
    """FI/LS-Schalter 1P+N wie im EPLAN-Plan: 1 und N oben, 2 und N. unten, Antrieb links,
    Auslöser (I>, IΔ) als Kasten über beide Pole. Siemens liefert für 5SU1 nur Aufbaudaten."""
    e = Elem("FI/LS-Schalter 1P+N B6A 30mA", "-F",
             info("Siemens", "5SU1356-6KK06", "5SU1356-6KK06", "FI/LS-Schalter 1P+N, B6A, 30 mA, Typ A"))
    for x, top, bottom in ((0, "1", "2"), (20, "N", "N.")):
        e.term(x, 0, "n", top)
        e.term(x, 80, "s", bottom)
        e.line(x, 0, x, 14)
        e.line(x, 32, x - 8, 16)  # Schaltstück, Drehpunkt unten
        e.line(x, 32, x, 80)
        e.text(x + 2, 8, top)
        e.text(x + 2, 78, bottom)
    e.line(-40, 24, 16, 24, DASHED)       # mechanische Verbindung
    e.rect(-48, 18, 10, 10)               # Antrieb (Schaltschloss)
    e.line(-48, 23, -38, 23)
    e.line(-43, 18, -43, 28)
    e.rect(-6, 40, 32, 16)                # Auslöser beider Pole
    e.text(-4, 51, "I>", size=6)
    e.rect(-30, 42, 16, 12)               # Fehlerstromauslöser
    e.text(-28, 52, "IΔ", size=6)
    e.line(-14, 48, -6, 48, DASHED)
    return [e.write(base, ["Siemens", "Schutzeinrichtungen", "5SU1356-6KK06"], "5SU1356-6KK06 Schaltbild")]


def steckdose(base):
    """Schutzkontaktsteckdose 1P+N+PE wie im EPLAN-Plan: drei Buchsen im 20-px-Raster, gestrichelter
    Rahmen, PE mit Schutzkontaktstrich."""
    e = Elem("Steckdose 1P+N+PE", "-XS", info("", "", "Schuko 230V 16A", "Schutzkontaktsteckdose 230 V, 16 A"))
    for x, n in ((0, "L"), (20, "N"), (40, "PE")):
        e.term(x, 0, "n", n)
        e.line(x, 0, x, 20)
        e.arc(x, 26, 6, 0, 180)           # Buchse, nach unten offen
        e.text(x + 2, 8, n)
    e.line(40, 20, 40, 30)                # Schutzkontakt
    e.rect(-12, 14, 64, 20, DASHED)
    return [e.write(base, ["Allgemein", "Steckdosen", "Schuko_1P_N_PE"])]


def wago_788_312(base):
    """WAGO 788-312 Relaismodul 24 V DC, 2 Wechsler, nach dem Schaltbild im Datenblatt:
    A1(+) / A2(-) mit Verpolschutzdiode, LED und Freilaufdiode, Spule; Wechsler 11/12/14 und 21/22/24."""
    e = Elem("Relaismodul 788-312 24V DC 2 Wechsler", "-K",
             info("WAGO", "788-312", "788-312", "Relaismodul 24 V DC, 2 Wechsler, 8 A, Statusanzeige"))
    e.pin(0, 0, "w", "A1")
    e.pin(0, 60, "w", "A2")
    e.text(-14, 10, "+")
    e.text(-14, 70, "-")
    e.line(0, 0, 80, 0)
    e.line(0, 60, 80, 60)
    # Verpolschutzdiode in A2 (Durchlass zu A2 hin)
    e.line(14, 55, 14, 65)
    e.line(14, 60, 24, 55)
    e.line(14, 60, 24, 65)
    e.line(24, 55, 24, 65)
    # LED und Freilaufdiode im Kasten zwischen den Schienen
    e.line(40, 0, 40, 16)
    e.line(40, 44, 40, 60)
    e.rect(30, 16, 30, 28)
    e.line(36, 26, 44, 26)        # LED: Dreieck nach unten
    e.line(36, 26, 40, 34)
    e.line(44, 26, 40, 34)
    e.line(36, 34, 44, 34)
    e.line(46, 27, 50, 23)        # Lichtpfeile
    e.line(46, 31, 50, 27)
    e.line(52, 34, 56, 34)        # Freilaufdiode: Dreieck nach oben
    e.line(52, 34, 54, 26)
    e.line(56, 34, 54, 26)
    e.line(52, 26, 56, 26)
    # Spule
    e.line(80, 0, 80, 20)
    e.line(80, 40, 80, 60)
    e.rect(72, 20, 16, 20)
    e.line(88, 30, 112, 30, DASHED)
    e.line(112, 30, 112, 90, DASHED)
    # zwei Wechsler: Schließer 14/24 oben, Mitte 11/21, Öffner 12/22 unten (Ruhelage 11-12)
    for k, y in ((1, 0), (2, 60)):
        e.pin(160, y + 10, "e", f"{k}4")
        e.pin(160, y + 30, "e", f"{k}1")
        e.pin(160, y + 50, "e", f"{k}2")
        e.line(124, y + 10, 160, y + 10)
        e.line(124, y + 10, 124, y + 18)
        e.line(140, y + 30, 160, y + 30)
        e.line(160, y + 50, 120, y + 50)
        e.line(120, y + 50, 120, y + 44)
        e.line(120, y + 44, 140, y + 30)
    return [e.write(base, ["WAGO", "Relais, Schütze", "788-312"], "788-312 Schaltbild")]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ziel", default=str(USER))
    a = ap.parse_args()
    base = Path(a.ziel)
    files = []
    for f in (cybelec, pcss, irt, fiessler, murr_trafo, finder, phoenix, harting, petzold, luefter, fi_ls, steckdose, wago_788_312):
        files += f(base)
    for p in files:
        print(p)
    print(len(files), "Bauteile")


if __name__ == "__main__":
    main()
