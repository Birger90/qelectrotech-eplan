"""FluidSIM-6-Schaltungen (.circ) als QElectroTech-Projekt (.qet) übernehmen.

Eine .circ ist gzip-gepacktes XML (Beschreibung: PLAN-PNEUMATIK.md, Abschnitt 1); jede wird ein Blatt.
Übernommen werden:
- jedes Bauteil mit Symbol und Anschlüssen (Pneumatik, Elektrik, Hydraulik) als eingebettetes QET-Bauteil,
  umgewandelt wie in tools/symbole/fluidsim_symbole.py, mit Drehung, Spiegelung und Maßstab der Schaltung,
  deutschem Namen (FluidSIMs eigene Übersetzung) und, bei Zylindern, Kolben-/Stangendurchmesser und Hub;
- die Kennzeichnung (-MM1, -QM1, -MB1 …), Beschreibung und technische Werte als Bauteilinformation
  (description, comment), die in FluidSIM sichtbaren Werte (24 V, p = 6 bar …) als Text am Bauteil;
- jede Leitung als QET-Leitung zwischen denselben Anschlüssen (QET legt den Weg neu); Verzweigungen werden zu
  Leitungen am selben Netz;
- Schalldämpfer und Blindstopfen als Abschluss des Anschlusses, an dem sie sitzen;
- Spule ↔ Ventil (Verweis am Ventilanschluss) und Relaisspule ↔ Kontakte (gleiche Kennzeichnung) als
  Master/Slave, auch über Blattgrenzen.

Nicht übernommen: Rahmen und Schriftfeld, Tabellen, Zustandsdiagramme, Stücklisten, GRAFCET, Simulationswerte.
Artikeldaten (Hersteller, Bestellnummer) speichert FluidSIM in den Schaltungen nicht.

    python fluidsim_plan.py SCHALTUNG.circ [WEITERE.circ …] AUSGABE.qet [--qet build/deploy/QElectroTech.exe]
    python fluidsim_plan.py ORDNER_MIT_CIRC AUSGABE.qet          # jede .circ ein Blatt, nach Namen sortiert
    python fluidsim_plan.py --pruefe-drehung "C:/Program Files (x86)/Didactic/FluidSIM6/circ"

Das Projekt schreibt QElectroTech selbst (über --run, wie das MCP qet_edit); dafür setzt das Werkzeug für
diesen einen Aufruf QET_ENABLE_SCRIPTING=1.
"""
import argparse
import gzip
import hashlib
import json
import math
import os
import re
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "tools" / "symbole"))
import fluidsim_symbole as fs  # noqa: E402

FLUIDSIM = Path("C:/Program Files (x86)/Didactic/FluidSIM6")
AUSLASSEN = ("Grafcet.", "Digital.")  # Objektarten, die keine Bauteile eines Fluid-/Elektroplans sind


def lesen(pfad: Path):
    roh = pfad.read_bytes()
    if roh[:2] == b"\x1f\x8b":
        roh = gzip.decompress(roh)
    return ET.fromstring(roh)


def abschnitt(wurzel, art):
    return next((s for s in wurzel.findall("section") if s.get("type") == art), None)


def objekte(wurzel):
    """Alle Schaltungsobjekte mit Symbol, auch in Gruppen."""
    co = abschnitt(wurzel, "circuit-objects")
    if co is None:
        return []
    return [o for o in co.iter() if o.tag in ("component", "terminal") and o.get("symbol-ref")]


def anschluss_lage(symbole, objekt, anschluss):
    """Lage eines Anschlusses eines Objekts auf dem Blatt (FluidSIM-Einheiten), oder None."""
    ebene_name, ident = fs.anschluss_schluessel(objekt, anschluss)
    symbol = symbole.get(objekt.get("symbol-ref"))
    if symbol is None:
        return None
    for eb in symbol.findall("symbol-layer"):
        if eb.get("ident", "") != ebene_name:
            continue
        for con in eb.findall("connector"):
            if con.get("ident") == ident:
                lx = fs.zahl(eb, "ref-x") + fs.zahl(eb, "scale-x", 1.0) * fs.zahl(con, "cx")
                ly = fs.zahl(eb, "ref-y") + fs.zahl(eb, "scale-y", 1.0) * fs.zahl(con, "cy")
                q = fs.Trafo.von(objekt).punkt(lx, ly)
                m = fs.mitte(objekt)
                return (m[0] + q[0], m[1] + q[1])
    return None


def pruefe_drehung(ordner: Path):
    """Zählt für SINN = +1 und -1, wie viele Leitungen an gedrehten Bauteilen genau passen."""
    treffer = {1: 0, -1: 0}
    gesamt = 0
    for f in sorted(ordner.rglob("*.circ")):
        try:
            w = lesen(f)
        except Exception:
            continue
        grafik = abschnitt(w, "symbol-graphics")
        if grafik is None:
            continue
        symbole = {s.get("ident"): s for s in grafik.findall("symbol")}
        nach_id = {o.get("ident"): o for o in objekte(w)}
        for v in w.iter("connect"):
            a, b = v.get("con1", ""), v.get("con2", "")
            oa, _, xa = a.partition(".")
            ob, _, xb = b.partition(".")
            if oa not in nach_id or ob not in nach_id:
                continue
            if not (fs.zahl(nach_id[oa], "rotation") or fs.zahl(nach_id[ob], "rotation")):
                continue
            pkte = (v.get("points") or "").split()
            if not pkte:
                continue
            lx, ly = (float(z) for z in pkte[-1].split(","))
            gesamt += 1
            for sinn in (1, -1):
                fs.Trafo.SINN = sinn
                pa, pb = anschluss_lage(symbole, nach_id[oa], xa), anschluss_lage(symbole, nach_id[ob], xb)
                if pa and pb and math.hypot(pa[0] + lx - pb[0], pa[1] + ly - pb[1]) < 1.0:
                    treffer[sinn] += 1
    fs.Trafo.SINN = -1
    print(f"Leitungen an gedrehten Bauteilen: {gesamt}; passend mit Uhrzeigersinn: {treffer[1]}, "
          f"gegen den Uhrzeigersinn: {treffer[-1]}")
    return treffer


def dateiname(name, saat):
    """Lesbarer Name der eingebetteten Bauteildatei: deutscher Name und ein Stück der Prüfsumme."""
    kurz = re.sub(r"[^\w\-]+", "_", name, flags=re.UNICODE).strip("_")[:40] or "Bauteil"
    return f"{kurz}_{saat[-8:]}.elmt"


def sichtbare_werte(objekt, textfelder):
    """Die in FluidSIM am Bauteil angezeigten Parameter (^#…, z. B. „p0 = 6 bar“) mit ihrem Textfeld."""
    out = []
    for a in objekt.iter("attribute"):
        name, ref = a.get("name", ""), a.get("display-ref")
        if not name.startswith("^#") or not ref:
            continue
        tf = textfelder.get(ref)
        wert = (tf.get("value") if tf is not None else None) or a.get("value") or ""
        if wert.strip() and tf is not None and tf.get("visible") != "false":
            out.append((wert.strip(), tf))
    return out


def plan(circs, sammlung: Path, sprache):
    """Bauteile in sammlung schreiben; @return die Operationen für qet_edit und eine Zusammenfassung."""
    ops = []
    zahl = {"bauteile": 0, "leitungen": 0, "verknuepfungen": 0, "texte": 0, "ausgelassen": 0}
    bauteile = []   # dict: folio, id, label, link, klasse, ventil_ziele [(comp-ref, label)], ident

    for blatt, circ in enumerate(circs):
        w = lesen(circ)
        grafik = abschnitt(w, "symbol-graphics")
        if grafik is None:
            continue
        ops.append({"op": "set_folio_border", "folio": blatt, "property": "preset", "value": "a3-landscape"})
        symbole = {s.get("ident"): s for s in grafik.findall("symbol")}
        alle = objekte(w)
        verbindungen = list(w.iter("connect"))
        nach_id = {o.get("ident"): o for o in alle}
        co = abschnitt(w, "circuit-objects")
        textfelder = {t.get("ident"): t for t in co.iter("text-box")} if co is not None else {}
        hier = {}       # FluidSIM-Ident -> Bauteil dieses Blatts

        for o in alle:
            objekt = o.get("object-ref") or ""
            if o.tag == "component" and (not objekt or objekt in fs.ABSCHLUSS or objekt.startswith(AUSLASSEN)):
                continue
            beschr = o.find("dki-data/description")
            en = (beschr.get("value") if beschr is not None else "") or objekt
            de = sprache.deutsch(en) if en else ""
            if not en:
                # Klemmen einer Klemmleiste haben keine Beschreibung
                en, de = ("Terminal", "Klemme") if o.tag == "terminal" else ("FluidSIM", "FluidSIM")
            # Zwei Durchläufe: die UUIDs hängen am Inhalt, so werden gleiche Symbole ein Bauteil
            probe = fs.bauteil_umwandeln(symbole, o, alle, verbindungen, (de, en), circ.name, "probe", lage=True)
            if probe is None or not probe["uuids"]:
                zahl["ausgelassen"] += 1
                continue
            saat = "plan/" + hashlib.sha1(probe["text"].encode("utf-8")).hexdigest()
            erg = fs.bauteil_umwandeln(symbole, o, alle, verbindungen, (de, en), circ.name, saat, lage=True)
            datei = sammlung / dateiname(de, saat)
            datei.write_text(erg["text"], encoding="utf-8", newline="\n")
            m = fs.mitte(o)
            x = round((m[0] + erg["anker"][0]) * fs.K, 1)
            y = round((m[1] + erg["anker"][1]) * fs.K, 1)
            oid = f"b{blatt}_{o.get('ident')}"
            ops.append({"op": "add_element", "id": oid, "folio": blatt, "path": f"common://{datei.name}",
                        "x": x, "y": y})
            el = "$" + oid
            kenn = o.find("dki-data/designation")
            label = kenn.get("value", "") if kenn is not None else ""
            if label.startswith("?"):
                label = ""
            if label:
                ops.append({"op": "set_label", "folio": blatt, "element": el, "label": label})
            ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "description", "value": de})
            # Ein Wert, der wie die Kennzeichnung lautet („24 V“ an der Spannungsquelle), steht schon da
            werte = [(v, tf) for v, tf in sichtbare_werte(o, textfelder) if v.replace(" ", "") != label.replace(" ", "")]
            if werte:
                ops.append({"op": "set_info", "folio": blatt, "element": el, "key": "comment",
                            "value": "; ".join(v for v, _ in werte)})
            for wert, tf in werte:
                # In Bauteilkoordinaten: das Bauteil steht mit seinem Ursprung bei (x, y)
                ops.append({"op": "add_element_text", "folio": blatt, "element": el, "source": "text",
                            "value": wert, "x": round(fs.zahl(tf, "x1") * fs.K - x, 1),
                            "y": round(fs.zahl(tf, "y1") * fs.K - y, 1)})
                zahl["texte"] += 1
            ziele = []
            for con in o.findall("connector"):
                art = con.find("link-props/type")
                ziel = con.find("link-props/target")
                if art is not None and ziel is not None and art.get("value") == "solenoid-valve-coil-target":
                    ziele.append((ziel.get("comp-ref", ""), ziel.get("value", "")))
            b = {"folio": blatt, "id": oid, "label": label, "link": erg["link"], "klasse": erg["klasse"],
                 "uuids": erg["uuids"], "ziele": ziele, "ident": o.get("ident")}
            bauteile.append(b)
            hier[o.get("ident")] = b
            zahl["bauteile"] += 1

        # Das ganze Blatt so verschieben, dass es im Rahmen beginnt: FluidSIM-Schaltungen liegen teils bei
        # negativen Koordinaten oder höher als das QET-Blatt. Texte stehen relativ zum Bauteil und gehen mit.
        hier_ops = [op for op in ops if op["op"] == "add_element" and op["folio"] == blatt]
        if hier_ops:
            rand = 50.0
            dx = max(0.0, rand - min(op["x"] for op in hier_ops))
            dy = max(0.0, rand - min(op["y"] for op in hier_ops))
            for op in hier_ops:
                op["x"] = round(op["x"] + dx, 1)
                op["y"] = round(op["y"] + dy, 1)

        # Leitungen dieses Blatts
        def anschluss_uuid(bezug):
            oid, _, x = bezug.partition(".")
            b = hier.get(oid)
            if b is None:
                return None, None
            return b["id"], b["uuids"].get(fs.anschluss_schluessel(nach_id[oid], x))

        # Verzweigungen (Interface.MultiPin) sind in QET kein Bauteil: alle Anschlüsse, die über eine oder
        # mehrere Verzweigungen zusammenhängen, bilden ein Netz und werden der Reihe nach verbunden
        def ist_verzweigung(bezug):
            o = nach_id.get(bezug.partition(".")[0])
            return o is not None and (o.get("object-ref") or "").endswith("Interface.MultiPin")

        eltern = {}

        def wurzel(k):
            while eltern.setdefault(k, k) != k:
                eltern[k] = eltern[eltern[k]]
                k = eltern[k]
            return k

        im_netz = set()
        paare = []
        for v in verbindungen:
            a, b2 = v.get("con1", ""), v.get("con2", "")
            va, vb = ist_verzweigung(a), ist_verzweigung(b2)
            if not va and not vb:
                paare.append((a, b2))
                continue
            ka = a.partition(".")[0] if va else a
            kb = b2.partition(".")[0] if vb else b2
            eltern[wurzel(ka)] = wurzel(kb)
            im_netz.update((ka, kb))

        def lage(bezug):
            oid, _, x = bezug.partition(".")
            return anschluss_lage(symbole, nach_id[oid], x) or (0.0, 0.0)

        netze = {}
        for k in im_netz:
            if k not in nach_id:          # nicht die Verzweigung selbst
                netze.setdefault(wurzel(k), []).append(k)
        for glieder in netze.values():
            glieder.sort(key=lambda bz: (round(lage(bz)[1]), lage(bz)[0]))
            paare += list(zip(glieder, glieder[1:]))

        for a, b2 in paare:
            ea, ta = anschluss_uuid(a)
            eb, tb = anschluss_uuid(b2)
            if ea and eb and ta and tb:
                ops.append({"op": "add_conductor", "folio": blatt, "from": "$" + ea,
                            "from_terminal": "{" + ta + "}", "to": "$" + eb, "to_terminal": "{" + tb + "}"})
                zahl["leitungen"] += 1

    # Verknüpfungen, auch über Blattgrenzen: zuerst über den Verweis im selben Blatt, sonst über die Kennzeichnung
    verknuepft = set()

    def verbinde(master, slave):
        if slave["id"] in verknuepft:
            return
        verknuepft.add(slave["id"])
        # Vom Slave aus: QETs linkElements setzt die Verknüpfungen des ersten Elements auf genau das zweite;
        # vom Master aus würde jede weitere Spule (zweiter Magnet, weitere Kontakte) die vorige ersetzen
        ops.append({"op": "link_elements", "folio": slave["folio"], "element": "$" + slave["id"],
                    "to_folio": master["folio"], "to": "$" + master["id"]})
        zahl["verknuepfungen"] += 1

    spulen = [b for b in bauteile if b["link"] == "slave" and b["klasse"] == "solenoid-valve-coil"]
    for ventil in (b for b in bauteile if b["link"] == "master" and b["ziele"]):
        for comp_ref, ziel_label in ventil["ziele"]:
            spule = next((s for s in spulen if s["folio"] == ventil["folio"] and s["ident"] == comp_ref), None)
            if spule is None and ziel_label:
                spule = next((s for s in spulen if s["label"] == ziel_label and s["id"] not in verknuepft), None)
            if spule is not None:
                verbinde(ventil, spule)
    relais = [b for b in bauteile if b["link"] == "master" and b["klasse"] == "solenoid-coil" and b["label"]]
    for kontakt in (b for b in bauteile if b["link"] == "slave" and b["klasse"] in fs.KONTAKT and b["label"]):
        spule = next((r for r in relais if r["label"] == kontakt["label"]), None)
        if spule is not None:
            verbinde(spule, kontakt)
    return ops, zahl


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("pfade", nargs="*", type=Path, help=".circ-Dateien oder ein Ordner, zuletzt AUSGABE.qet")
    ap.add_argument("--qet", type=Path, default=REPO / "build" / "deploy" / "QElectroTech.exe")
    ap.add_argument("--fluidsim", type=Path, default=FLUIDSIM, help="FluidSIM-Installation (für db/lang.db)")
    ap.add_argument("--titel", default=None)
    ap.add_argument("--pruefe-drehung", type=Path, metavar="ORDNER")
    ap.add_argument("--nur-zeigen", action="store_true", help="Operationen als JSON ausgeben, nichts schreiben")
    a = ap.parse_args(argv)

    if a.pruefe_drehung:
        pruefe_drehung(a.pruefe_drehung)
        return
    pfade = list(a.pfade)
    ausgabe = None
    if pfade and pfade[-1].suffix.lower() == ".qet":
        ausgabe = pfade.pop()
    circs = []
    for p in pfade:
        circs += sorted(p.glob("*.circ")) if p.is_dir() else [p]
    if not circs or (ausgabe is None and not a.nur_zeigen):
        ap.error("SCHALTUNG.circ (oder Ordner) und AUSGABE.qet angeben")

    sprache = fs.Uebersetzung(a.fluidsim / "db" / "lang.db")
    with tempfile.TemporaryDirectory(prefix="fluidsim-plan-") as tmp:
        sammlung = Path(tmp) / "elements"
        sammlung.mkdir()
        ops, zahl = plan(circs, sammlung, sprache)
        if a.nur_zeigen:
            print(json.dumps(ops, indent=1, ensure_ascii=False))
            print(zahl)
            return
        sys.path.insert(0, str(REPO / "misc" / "qet-mcp"))
        try:
            import qet_mcp  # noqa: E402
        except SyntaxError as e:
            sys.exit(f"misc/qet-mcp/qet_mcp.py lässt sich nicht laden: {e}")
        os.environ["QET_ENABLE_SCRIPTING"] = "1"
        leer = Path(tmp) / "leer.qet"
        titel = a.titel or (circs[0].stem if len(circs) == 1 else circs[0].parent.name)
        qet_mcp.tool_project_new(str(a.qet), str(leer), title=titel, folios=[c.stem for c in circs],
                                 elements_dir=str(sammlung))
        erg = qet_mcp.tool_edit(str(a.qet), str(leer), ops, str(ausgabe), elements_dir=str(sammlung),
                                timeout=600)
        fehler = [o for o in erg.get("operations", []) if not o.get("succeeded")]
        print(f"{ausgabe}: {len(circs)} Blatt/Blätter, {zahl['bauteile']} Bauteile, {zahl['leitungen']} Leitungen, "
              f"{zahl['verknuepfungen']} Verknüpfungen, {zahl['texte']} Werte als Text, "
              f"{zahl['ausgelassen']} Objekte ohne Anschluss ausgelassen; {len(fehler)} Operationen fehlgeschlagen.")
        for o in fehler[:10]:
            print("  FEHLER", json.dumps(o, ensure_ascii=False))
        if not erg.get("saved"):
            sys.exit("Projekt nicht gespeichert: " + json.dumps(erg, ensure_ascii=False)[:2000])


if __name__ == "__main__":
    main()
