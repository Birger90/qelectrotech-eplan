"""SPS-Module der Firmensammlung zu SPS-Mastern mit Kanaltabelle machen.

    python sps_kanaele.py <elements-company> [--sicherung <ordner>] [--probe]

Ein vom Data-Portal-/EDZ-Import erzeugtes Modul (z. B. Siemens/SPS/6ES7132-6BF01-0BA0) hat Bauteile, an deren
Klemmen der Kanaltext steht ("DQ0" neben Klemme 1, "M" neben Klemme 9, ...). Daraus entsteht die Kanaltabelle
(plcMasterData/plcIOs): je Kanal Typ (DI/DQ/AI/AQ), Kanalname als Adresse und die Klemmen (Signal, danach die
direkt folgende Bezugsklemme M/L+/0V). Die Tabelle kommt in jedes Bauteil des Ordners, das alle Kanalklemmen hat
(Übersicht, Gerätebild, Black Box), außer den Aufbauansichten; diese werden SPS-Master
(link_type="master", type=plc). Dann bietet QElectroTech beim allgemeinen Kanalbauteil (Allgemein/SPS) genau die
Kanäle der Module im Projekt an.

Außerdem legt es die beiden allgemeinen Kanalbauteile an (Allgemein/SPS/SPS_Eingang_Kanal.elmt,
SPS_Ausgang_Kanal.elmt), aus dem Kanalkasten eines Moduls ohne dessen Modultexte.
--probe zeigt nur, was geändert würde. Vor dem Schreiben wird jede Datei nach --sicherung kopiert."""
import datetime
import os
import re
import shutil
import sys
import uuid
import xml.etree.ElementTree as ET

KANAL = re.compile(r"^(DI|DQ|DO|AI|AQ|AO)\s*(\d+)$", re.I)
BEZUG = re.compile(r"^(M|L\+|L-|0V|0VDC|24V|24VDC|\+|-|1M|2M|1L\+|2L\+)$", re.I)
TYP = {"DI": "entree_digitale", "DQ": "sortie_digitale", "DO": "sortie_digitale",
       "AI": "entree_analogique", "AQ": "sortie_analogique", "AO": "sortie_analogique"}


def _f(e, k):
    try:
        return float(e.get(k, "0"))
    except ValueError:
        return 0.0


def klemmen(root):
    # <terminal> in einem plcIO ist ein Klemmenname der Kanaltabelle, keine Klemme
    return [(t.get("name", ""), _f(t, "x"), _f(t, "y")) for t in root.iter("terminal") if t.get("x") is not None]


def texte(root):
    """Statische und Benutzer-Texte (text, x, y) der Beschreibung."""
    out = [(t.get("text", ""), _f(t, "x"), _f(t, "y")) for t in root.iter("text") if t.get("text") is not None]
    for d in root.iter("dynamic_text"):
        if d.get("text_from", "UserText") == "UserText":
            tx = d.find("text")
            if tx is not None and (tx.text or "").strip():
                out.append((tx.text.strip(), _f(d, "x"), _f(d, "y")))
    return out


def funktionstext(kl, txt):
    """Der Kanal-/Bezugstext neben einer Klemme: links davon, knapp unter ihrer Höhe (EPLAN-Anschlusstext)."""
    name, x, y = kl
    best = None
    for s, tx, ty in txt:
        s = s.strip()
        if s == name or not (KANAL.match(s) or BEZUG.match(s)):
            continue
        if tx > x + 1 or tx < x - 40:
            continue
        d = abs(ty - (y + 6))
        if d <= 8 and (best is None or d < best[0]):
            best = (d, s)
    return best[1] if best else None


def kanaltabelle(root):
    """[(typ, kanalname, [klemmen])] in Kanalreihenfolge, oder [] ohne Kanaltexte."""
    kls = klemmen(root)
    txt = texte(root)
    fk = [(k, funktionstext(k, txt)) for k in kls]
    fk.sort(key=lambda p: (p[0][2], p[0][1]))
    kanaele = []
    letzte = None
    for (name, x, y), f in fk:
        m = KANAL.match(f or "")
        if m:
            letzte = [m.group(1).upper(), f"{m.group(1).upper()}{int(m.group(2))}", [name], y]
            kanaele.append(letzte)
        elif f and BEZUG.match(f) and letzte and 0 < y - letzte[3] <= 25:
            letzte[2].append(name)
            letzte = None          # je Kanal höchstens die eine folgende Bezugsklemme
        else:
            letzte = None
    kanaele.sort(key=lambda k: (k[0], int(re.sub(r"\D", "", k[1]) or 0)))
    return [(TYP[k[0]], k[1], k[2]) for k in kanaele]


def plc_xml(tabelle):
    plc = ET.Element("plcMasterData", rowHeight="8.00")
    ios = ET.SubElement(plc, "plcIOs")
    for typ, name, kl in tabelle:
        io = ET.SubElement(ios, "plcIO", type=typ, address=name, functionText="", comment="", crossRef="",
                           terminalCount=str(len(kl)))
        for k in kl:
            ET.SubElement(io, "terminal").text = k
    return plc


def als_master(root, tabelle):
    root.set("link_type", "master")
    kind = root.find("kindInformations")
    if kind is None:
        kind = ET.Element("kindInformations")
        root.insert(2, kind)
    for k in list(kind):
        if k.tag == "plcMasterData" or (k.tag == "kindInformation" and k.get("name") == "type"):
            kind.remove(k)
    t = ET.Element("kindInformation", name="type")
    t.text = "plc"
    kind.insert(0, t)
    kind.append(plc_xml(tabelle))


def allgemeiner_kanal(quelle, ziel, art, name_de, name_en):
    """Kanalkasten eines Moduls ohne Modultexte und Artikeldaten, mit neuer UUID."""
    tree = ET.parse(quelle)
    root = tree.getroot()
    u = root.find("uuid")
    if u is not None:
        u.set("uuid", "{%s}" % uuid.uuid4())
    names = root.find("names")
    for n in list(names):
        names.remove(n)
    for lang, txt in (("de", name_de), ("en", name_en)):
        ET.SubElement(names, "name", lang=lang).text = txt
    infos = root.find("elementInformations")
    if infos is not None:
        for i in list(infos):
            if i.get("name") != "label":
                infos.remove(i)
    desc = root.find("description")
    for t in list(desc):
        if t.tag == "text":          # "DQ 8x24 VDC/0.5A", "ET 200SP"
            desc.remove(t)
    inf = root.find("informations")
    if inf is not None:
        inf.text = ("Allgemeiner SPS-%s: Modul und Kanal im Fenster Auswahleigenschaften > SPS wählen; "
                    "BMK, Adresse und Klemme kommen vom Modul." % art)
    ET.indent(tree, "    ")
    tree.write(ziel, encoding="utf-8", xml_declaration=False)


def main():
    argv = sys.argv[1:]
    probe = "--probe" in argv
    sich = None
    if "--sicherung" in argv:
        i = argv.index("--sicherung")
        sich = argv[i + 1]
        del argv[i:i + 2]
    args = [a for a in argv if not a.startswith("--")]
    sammlung = args[0]
    if sich is None:
        sich = os.path.join(os.path.dirname(os.path.abspath(sammlung)), "_sicherung",
                            "sps_kanaele_" + datetime.datetime.now().strftime("%Y-%m-%d_%H%M%S"))
    geaendert = 0
    quelle_di = quelle_dq = None
    for ordner, _, dateien in os.walk(sammlung):
        elmts = sorted(f for f in dateien if f.endswith(".elmt"))
        if not elmts:
            continue
        baeume = {}
        for f in elmts:
            try:
                baeume[f] = ET.parse(os.path.join(ordner, f))
            except ET.ParseError:
                continue
        for f, t in baeume.items():
            if t.getroot().get("link_type") == "slave" and "_Kanal_Original" in f:
                if f.endswith("_DI_Kanal_Original.elmt") and quelle_di is None:
                    quelle_di = os.path.join(ordner, f)
                if f.endswith("_DQ_Kanal_Original.elmt") and quelle_dq is None:
                    quelle_dq = os.path.join(ordner, f)
        # die Kanaltabelle aus dem Bauteil mit den meisten Kanälen
        tab, herkunft = [], None
        for f, t in baeume.items():
            k = kanaltabelle(t.getroot())
            if len(k) > len(tab):
                tab, herkunft = k, f
        if len(tab) < 2:             # ein einzelner "DO1" ist ein Meldekontakt, keine E/A-Karte
            continue
        noetig = {k for _, _, kl in tab for k in kl}
        for f, t in baeume.items():
            r = t.getroot()
            if "_Aufbau" in f or r.get("link_type") not in ("simple", "master"):
                continue
            if not noetig <= {n for n, _, _ in klemmen(r)}:
                continue
            print(f"{os.path.relpath(os.path.join(ordner, f), sammlung)}: {len(tab)} Kanäle "
                  f"({tab[0][1]}..{tab[-1][1]}, {tab[0][1]} an Klemme {'/'.join(tab[0][2])}) aus {herkunft}")
            if probe:
                continue
            ziel = os.path.join(sich, os.path.relpath(ordner, sammlung))
            os.makedirs(ziel, exist_ok=True)
            shutil.copy2(os.path.join(ordner, f), os.path.join(ziel, f))
            als_master(r, tab)
            t.write(os.path.join(ordner, f), encoding="utf-8", xml_declaration=False)
            geaendert += 1
    allg = os.path.join(sammlung, "Allgemein", "SPS")
    for quelle, datei, art, de, en in ((quelle_di, "SPS_Eingang_Kanal.elmt", "Eingang",
                                        "SPS-Eingang (Kanal, allgemein)", "PLC input (channel, generic)"),
                                       (quelle_dq, "SPS_Ausgang_Kanal.elmt", "Ausgang",
                                        "SPS-Ausgang (Kanal, allgemein)", "PLC output (channel, generic)")):
        if quelle is None:
            print("kein Kanalkasten als Vorlage für", datei)
            continue
        print(f"Allgemein/SPS/{datei} aus {os.path.basename(quelle)}")
        if probe:
            continue
        os.makedirs(allg, exist_ok=True)
        if not os.path.exists(os.path.join(allg, "qet_directory")):
            with open(os.path.join(allg, "qet_directory"), "w", encoding="utf-8") as fd:
                fd.write('<qet-directory>\n    <names>\n        <name lang="de">SPS</name>\n'
                         '        <name lang="en">PLC</name>\n    </names>\n</qet-directory>\n')
        allgemeiner_kanal(quelle, os.path.join(allg, datei), art, de, en)
    print(f"{geaendert} Bauteile zu SPS-Modulen gemacht" + ("" if probe else f", Sicherung in {sich}"))


if __name__ == "__main__":
    main()
