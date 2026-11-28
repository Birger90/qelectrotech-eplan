"""Prüft den Konfigurator (docs/konfigurator.md) über die Kommandozeile von QElectroTech.

    python tools/konfigurator/test_konfigurator.py [--qet build/qelectrotech.exe] [--keep <ordner>]

Läuft headless in einem Temp-Ordner (mit --keep in dem genannten Ordner, der Plan bleibt dann liegen):
1. Plan aus examples/konfigurator/maschine.csv: Varianten (> 5,5 kW Stern-Dreieck), Schutzschalter aus der
   Siemens-Tabelle, SPS-Kanäle fortlaufend (auch über die Bytegrenze) und mit den Modulen verknüpft, BMK je Anlage,
   SPS-Übersicht, Listenblätter.
2. Dieselbe Tabelle noch einmal: alles bleibt.
3. Eine Zeile geändert: nur deren Blatt wird neu gezeichnet, BMK und Adressen bleiben, keine Reste in Listen.
4. Ein Blatt von Hand geändert und seine Zeile geändert: Konflikt, das alte Blatt bleibt, das neue folgt mit [new].
   Mit --overwrite: ersetzt.
5. Formeln und Bedingungen an einer eigenen Test-Vorlage (QET_FUNCTION_TEMPLATES_DIR).
6. Layoutprüfung ohne Treffer, PDF.
"""
import argparse
import html
import json
import os
import re
import shutil
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "misc" / "qet-mcp"))
import qet_mcp  # noqa: E402

FEHLER = []
MSYS_BIN = Path("C:/msys64/ucrt64/bin")


def pruefe(name, ok, detail=""):
    print(("OK    " if ok else "FEHLER") + " " + name + (f" – {detail}" if detail and not ok else ""))
    if not ok:
        FEHLER.append(name)


def erzeugen(qet, tabelle, ziel, overwrite=False, extra_env=None):
    args = ["--generate-from-config", str(tabelle), str(ziel), "--json"]
    if overwrite:
        args.append("--overwrite")
    lauf = qet_mcp._run_qet(qet, args, 600, tail=400000, extra_env=extra_env)
    text = lauf.get("stdout", "")
    start = text.find("{")
    try:
        daten = json.loads(text[start:]) if start >= 0 else {}
    except json.JSONDecodeError:
        daten = {}
    daten["exit_code"] = lauf.get("exit_code")
    if not daten.get("ok"):
        print(text[-3000:], lauf.get("stderr", "")[-3000:])
    return daten


def manifest(projekt):
    text = Path(projekt).read_text(encoding="utf-8")
    m = re.search(r"&lt;configurator .*?&lt;/configurator&gt;|<configurator .*?</configurator>", text, re.S)
    if not m:
        return {"objekte": {}, "blaetter": []}
    block = m.group(0)
    if block.startswith("&lt;"):
        block = html.unescape(block)
    root = ET.fromstring(block)
    objekte = {}
    for o in root.findall("object"):
        objekte[o.get("name")] = {"template": o.get("template"), "variant": o.get("variant"),
                                  "folio": o.get("folio"), "hash": o.get("hash"),
                                  **{v.get("name"): v.get("value") for v in o.findall("value")}}
    blaetter = [(f.get("kind"), [o.get("name") for o in f.findall("object")]) for f in root.findall("folio")]
    return {"objekte": objekte, "blaetter": blaetter}


def zeile_aendern(quelle, ziel, objekt, spalte, wert):
    zeilen = Path(quelle).read_text(encoding="utf-8").splitlines()
    kopf = None
    neu = []
    for z in zeilen:
        if z.startswith("#") or not z.strip():
            neu.append(z)
            continue
        zellen = z.split(";")
        if kopf is None:
            kopf = zellen
        elif zellen[0] == objekt:
            zellen[kopf.index(spalte)] = wert
        neu.append(";".join(zellen))
    Path(ziel).write_text("\n".join(neu) + "\n", encoding="utf-8")


def listen(bericht):
    return [z for z in bericht.get("report", []) if z.startswith(("Lists:", "Listen:"))]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--qet", default=str(REPO / "build" / "qelectrotech.exe"))
    ap.add_argument("--keep", default=None, help="Ordner, in dem die Pläne liegen bleiben")
    args = ap.parse_args()
    if MSYS_BIN.is_dir():
        os.environ["PATH"] = str(MSYS_BIN) + os.pathsep + os.environ.get("PATH", "")
    # qet_query, qet_edit und qet_layout_check laufen über Skripte
    os.environ["QET_ENABLE_SCRIPTING"] = "1"
    qet = str(Path(args.qet).resolve())
    arbeit = Path(args.keep) if args.keep else Path(tempfile.mkdtemp(prefix="konfigurator-"))
    arbeit.mkdir(parents=True, exist_ok=True)
    tabelle = arbeit / "maschine.csv"
    shutil.copy(REPO / "examples" / "konfigurator" / "maschine.csv", tabelle)
    plan = arbeit / "maschine.qet"
    for alt in (plan, arbeit / "maschine.vor-konfigurator.qet"):
        if alt.exists():
            alt.unlink()

    # 1. Erzeugen
    r = erzeugen(qet, tabelle, plan)
    pruefe("1 erzeugt", r.get("ok") and r.get("exit_code") == 0, r.get("error", ""))
    pruefe("1 neun Objekte", r.get("objects") == 9, str(r.get("objects")))
    bericht = "\n".join(r.get("report", []))
    print("      " + "\n      ".join(r.get("report", [])))
    for objekt, vorlage in (("Förderband", "Motorabgang DOL (Direktstart)"),
                            ("Lüfter Schaltschrank", "Motorabgang DOL (Direktstart)"),
                            ("Hauptpumpe", "Motorabgang Stern-Dreieck (Stern-Dreieck)"),
                            ("Rührwerk", "Motorabgang Stern-Dreieck (Stern-Dreieck)")):
        pruefe(f"1 Variante {objekt}", f"{objekt}: Motoren/{vorlage}" in bericht)
    text = plan.read_text(encoding="utf-8") if plan.exists() else ""
    # Schutzschalter nach Leistung aus der Siemens-Tabelle (Seite 4/5 der Auswahlhilfe)
    for artikel in ("3RV2011-1HA10", "3RV2011-1BA10", "3RV2021-4CA10", "3RV2011-4AA10"):
        pruefe(f"1 Schutzschalter {artikel}", artikel in text)
    m = manifest(plan) if plan.exists() else {"objekte": {}, "blaetter": []}
    o = m["objekte"]
    adr = {k: v.get("ADR") for k, v in o.items()}
    erwartet = {"Förderband": "%Q0.0", "Lüfter Schaltschrank": "%Q0.1", "Hauptpumpe": "%Q0.2",
                "Rührwerk": "%Q0.5", "Zylinder ausfahren": "%Q1.0", "Zylinder einfahren": "%Q1.1",
                "Endlage hinten": "%I0.0", "Endlage vorn": "%I0.1", "Füllstand min": "%I0.2"}
    pruefe("1 Adressen fortlaufend", adr == erwartet, str(adr))
    # Je Kanal das Gerät im Strompfad (verknüpft); Rührwerk Stern-Dreieck: ADR, ADR+1, ADR+2 = %Q0.5 ... %Q0.7,
    # dahinter %Q1.0 (Bytegrenze)
    abfrage = qet_mcp.tool_query(qet, str(plan), "SELECT p.address AS a FROM qet_plc_io p "
                                                 "WHERE p.slave_uuid IS NOT NULL ORDER BY p.structure_id, p.entry")
    belegt = sorted(z["a"] for z in abfrage.get("rows") or [])
    soll = sorted(["Q0.0", "Q0.1", "Q0.2", "Q0.3", "Q0.4", "Q0.5", "Q0.6", "Q0.7", "Q1.0", "Q1.1",
                   "I0.0", "I0.1", "I0.2"])
    pruefe("1 13 Kanäle mit den Modulen verknüpft", belegt == soll, str(belegt))
    # BMK je Anlage gezählt (IEC 81346: =FB-MA1 und =HY-MA1 sind verschiedene Geräte)
    bmk = {k: v.get("BMK") for k, v in o.items()}
    pruefe("1 BMK gezählt", bmk == {"Förderband": "MA1", "Lüfter Schaltschrank": "MA2", "Hauptpumpe": "MA1",
                                     "Rührwerk": "MA2", "Zylinder ausfahren": "MB1", "Zylinder einfahren": "MB2",
                                     "Endlage hinten": "BG1", "Endlage vorn": "BG2", "Füllstand min": "BG3"}, str(bmk))
    arten = [k for k, _ in m["blaetter"]]
    pruefe("1 SPS-Übersicht", arten.count("plc") == 1, str(arten))
    pruefe("1 Listenblätter", arten.count("list") >= 3, str(arten))
    plaene = [obj for k, obj in m["blaetter"] if k == "plan"]
    pruefe("1 kleine Objekte teilen Blätter", any(len(p) > 1 for p in plaene), str(plaene))
    listen_1 = listen(r)
    shutil.copy(plan, arbeit / "maschine_erster_lauf.qet")

    # 2. Unverändert
    r = erzeugen(qet, tabelle, plan)
    pruefe("2 unverändert: alles bleibt", r.get("ok") and r.get("created") == 0 and r.get("replaced") == 0
           and r.get("kept") == len(plaene) and r.get("exit_code") == 0,
           json.dumps({k: r.get(k) for k in ("created", "replaced", "kept", "conflicts", "exit_code")},
                      ensure_ascii=False))

    # 3. Eine Zeile geändert
    geaendert = arbeit / "maschine_geaendert.csv"
    zeile_aendern(tabelle, geaendert, "Förderband", "P_kW", "4")
    r = erzeugen(qet, geaendert, plan)
    pruefe("3 nur ein Blatt neu", r.get("ok") and r.get("replaced") == 1 and r.get("created") == 0
           and r.get("exit_code") == 0,
           json.dumps({k: r.get(k) for k in ("created", "replaced", "kept", "conflicts", "exit_code")},
                      ensure_ascii=False))
    m3 = manifest(plan)["objekte"]
    pruefe("3 BMK und Adressen bleiben", all(m3.get(k, {}).get("BMK") == bmk.get(k)
                                             and m3.get(k, {}).get("ADR") == adr.get(k) for k in bmk),
           str({k: (v.get("BMK"), v.get("ADR")) for k, v in m3.items()}))
    klemmen = {k: (v.get("XN"), v.get("W")) for k, v in o.items()}
    pruefe("3 Klemmen- und Kabelnummern bleiben", {k: (v.get("XN"), v.get("W")) for k, v in m3.items()} == klemmen,
           f"{klemmen} / {[(k, v.get('XN'), v.get('W')) for k, v in m3.items()]}")
    pruefe("3 neuer Schutzschalter 4 kW", "3RV2011-1JA10" in plan.read_text(encoding="utf-8"))
    pruefe("3 Listen gleich groß (keine Reste des alten Blatts)", listen(r) == listen_1, f"{listen_1} / {listen(r)}")
    lauf = qet_mcp.tool_query(qet, str(plan), "SELECT COUNT(*) AS n FROM qet_cables")
    pruefe("3 keine verwaisten Klemmen", "lists terminal" not in lauf.get("stderr", ""),
           lauf.get("stderr", "")[-600:])

    # 4. Handänderung
    blatt = uuid = None
    uuids = [d.get("uuid") for d in ET.parse(plan).getroot().findall("diagram")]
    pumpe = manifest(plan)["objekte"].get("Hauptpumpe", {}).get("folio")
    index = uuids.index(pumpe) if pumpe in uuids else -1
    # qet_elements zählt Blätter ab 1, qet_edit ab 0
    for e in qet_mcp.tool_elements(str(plan), index + 1, None, 5000).get("elements", []):
        if (e.get("label") or "").lstrip("-").endswith("MA1"):
            blatt, uuid = e.get("folio"), e.get("uuid")
            break
    pruefe("4 Hauptpumpe gefunden", blatt is not None, f"Blatt {index}")
    if blatt is not None:
        kopie = arbeit / "maschine_hand.qet"
        edit = qet_mcp.tool_edit(qet, str(plan), [{"op": "set_info", "folio": blatt - 1, "element": uuid,
                                                   "key": "comment", "value": "von Hand"}], str(kopie))
        if kopie.is_file():
            shutil.move(kopie, plan)
        pruefe("4 von Hand geändert", "von Hand" in plan.read_text(encoding="utf-8"), str(edit)[:300])
        zeile_aendern(geaendert, geaendert, "Hauptpumpe", "TEXT", "Hauptpumpe Hydraulik")
        r = erzeugen(qet, geaendert, plan)
        pruefe("4 Konflikt gemeldet", r.get("exit_code") == 3 and bool(r.get("conflicts")),
               json.dumps({k: r.get(k) for k in ("exit_code", "conflicts")}, ensure_ascii=False))
        print("      " + "\n      ".join(r.get("conflicts", [])[:6]))
        text = plan.read_text(encoding="utf-8")
        pruefe("4 Handänderung erhalten", "von Hand" in text)
        pruefe("4 neues Blatt mit [new]", "[new]" in text or "[neu]" in text)
        shutil.copy(plan, arbeit / "maschine_konflikt.qet")
        r = erzeugen(qet, geaendert, plan, overwrite=True)
        text = plan.read_text(encoding="utf-8")
        pruefe("4 mit --overwrite ersetzt", r.get("exit_code") == 0 and "von Hand" not in text
               and "[new]" not in text and "[neu]" not in text,
               json.dumps({k: r.get(k) for k in ("exit_code", "conflicts", "removed")}, ensure_ascii=False))
        pruefe("4 Listen gleich groß", listen(r) == listen_1, f"{listen_1} / {listen(r)}")
        abfrage = qet_mcp.tool_query(qet, str(plan), "SELECT p.address AS a FROM qet_plc_io p "
                                                     "WHERE p.slave_uuid IS NOT NULL")
        belegt4 = sorted(z["a"] for z in abfrage.get("rows") or [])
        pruefe("4 alle Kanäle wieder verknüpft", belegt4 == soll, str(belegt4))

    # 5. Formeln und Bedingungen
    vorlagen = arbeit / "vorlagen"
    (vorlagen / "Test").mkdir(parents=True, exist_ok=True)
    (vorlagen / "Test" / "Formeltest.qetmak").write_text("""<?xml version="1.0" encoding="UTF-8"?>
<qet_macro>
    <function_template category="Test" name="Formeltest" version="2">
        <parameter default="BG1" kind="label" name="BMK"/>
        <parameter default="2" name="N"/>
        <table key="N" name="stufen"><row N="1" S="klein"/><row N="5" S="mittel"/><row N="10" S="gross"/></table>
        <formula name="TEXT" expr="'N=' + text(N * 1,5; 1) + ' ' + lookup('stufen'; N; 'S') + if(N &gt;= 5 and not (N = 7); ' ja'; ' nein')"/>
        <variant name="Sensor" template="Sensoren/DI-Sensor mit Kabel" when="has(N)"/>
    </function_template>
</qet_macro>
""", encoding="utf-8")
    ftab = arbeit / "formel.csv"
    ftab.write_text("Objekt;Typical;N\nA;Test/Formeltest;3\nB;Test/Formeltest;7\nC;Test/Formeltest;6\n",
                    encoding="utf-8")
    fplan = arbeit / "formel.qet"
    if fplan.exists():
        fplan.unlink()
    r = erzeugen(qet, ftab, fplan, extra_env={"QET_FUNCTION_TEMPLATES_DIR": str(vorlagen)})
    text = fplan.read_text(encoding="utf-8") if fplan.exists() else ""
    for soll_text in ("N=4,5 mittel nein", "N=10,5 gross nein", "N=9,0 gross ja"):
        pruefe(f"5 Formel {soll_text}", soll_text in text, "\n".join(r.get("report", []))[:600])

    # 7. Produktnummern (zweiter Eingang): Regeldatei funktionsvorlagen/Produkte/MS.product.json
    ptab = arbeit / "maschine_produkte.csv"
    shutil.copy(REPO / "examples" / "konfigurator" / "maschine_produkte.csv", ptab)
    pplan = arbeit / "maschine_produkte.qet"
    if pplan.exists():
        pplan.unlink()
    r = erzeugen(qet, ptab, pplan)
    bericht = "\n".join(r.get("report", []))
    for objekt, vorlage in (("Kühlpumpe", "Motorabgang DOL (Direktstart)"),
                            ("Absaugung", "Motorabgang Stern-Dreieck (Stern-Dreieck)"),
                            ("Späneförderer", "Motorabgang DOL (Direktstart)")):
        pruefe(f"7 Produktnummer {objekt}", f"{objekt}: Motoren/{vorlage}" in bericht, bericht[-600:])
    text = pplan.read_text(encoding="utf-8") if pplan.exists() else ""
    # 7,5 kW mit erzwungenem Direktstart: Schutzschalter nach Leistung 3RV2011-4AA10 im Direktstart-Abgang
    pruefe("7 Schutzschalter aus Leistung der Nummer", "3RV2011-4AA10" in text and "3RV2021-4CA10" in text)

    def lauf_json(args):
        lauf = qet_mcp._run_qet(qet, args, 300, tail=400000, extra_env=None)
        t = lauf.get("stdout", "")
        start = min([i for i in (t.find("{"), t.find("[")) if i >= 0], default=-1)
        return (json.loads(t[start:]) if start >= 0 else None), lauf.get("exit_code")

    d, code = lauf_json(["--decode-product", "MS-400-7K5-D", "--json"])
    pruefe("7 Nummer zerlegt", code == 0 and d and d.get("variant") == "Direktstart"
           and d.get("values", {}).get("P_kW") == "7.5", str(d))
    d, code = lauf_json(["--decode-product", "MS-230-4K0", "--json"])
    pruefe("7 unbekannter Code wird abgelehnt", code == 1 and d and not d.get("ok") and "230" in d.get("error", ""),
           str(d))
    d, code = lauf_json(["--check-product-rules", "--json"])
    ms = [f for f in (d or []) if f.get("family") == "MS"]
    fehler_ms = [x for f in ms for x in f.get("findings", []) if x.get("severity") in ("error", "Fehler")]
    pruefe("7 Regeln der Familie MS ohne Lücken", code == 0 and ms and not fehler_ms and ms[0].get("numbers") == 24,
           json.dumps(ms, ensure_ascii=False)[:1200])
    kaputt = arbeit / "kaputt.product.json"
    kaputt.write_text(json.dumps({
        "family": "XX", "segments": [{"name": "P", "codes": {"1": {"values": {"P_kW": "1"}},
                                                            "2": {"values": {"P_kW": ""}}}}],
        "rules": [{"template": "Typicals/Gibtsnicht"},
                  {"when": {"P": "3"}, "template": "Typicals/Antrieb"},
                  {"when": {"P": "2"}, "template": "Typicals/Antrieb", "variant": "Dreieck"}]}), encoding="utf-8")
    d, code = lauf_json(["--check-product-rules", str(kaputt), "--json"])
    texte = " | ".join(x.get("text", "") for f in (d or []) for x in f.get("findings", []))
    pruefe("7 Lücken einer fehlerhaften Regeldatei gemeldet", code == 1 and "Gibtsnicht" in texte and "3" in texte
           and "Dreieck" in texte, texte[:1200])

    # 6. Layout und PDF
    lay = qet_mcp.tool_layout_check(qet, str(plan))
    # Kreuzungen zählt die Prüfung nur zum Vergleich (Info, senken die Note nicht); alles andere ist ein Treffer
    treffer = [f for f in lay.get("findings", []) if f.get("rule") != "crossing"]
    print(f"      Layout: Note {lay.get('score')}, Kreuzungen {lay.get('summary', {}).get('crossings')}")
    pruefe("6 Layoutprüfung ohne Treffer", bool(lay.get("ok")) and not treffer,
           json.dumps(treffer or lay, ensure_ascii=False)[:1500])
    pdf = arbeit / "maschine.pdf"
    ex = qet_mcp.tool_export(qet, str(plan), "pdf", str(pdf))
    pruefe("6 PDF", pdf.is_file() and pdf.stat().st_size > 10000, str(ex)[:300])

    print(f"\n{len(FEHLER)} Fehler" + (": " + ", ".join(FEHLER) if FEHLER else "") + f"  (Ordner {arbeit})")
    return 1 if FEHLER else 0


if __name__ == "__main__":
    sys.exit(main())
