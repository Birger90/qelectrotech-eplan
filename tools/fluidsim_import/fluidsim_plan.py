"""FluidSIM-6-Schaltung (.circ) als QElectroTech-Projekt (.qet) übernehmen.

Eine .circ ist gzip-gepacktes XML (Beschreibung: PLAN-PNEUMATIK.md, Abschnitt 1). Übernommen werden:
- jedes Bauteil mit Symbol und Anschlüssen (Pneumatik, Elektrik, Hydraulik), als eingebettetes QET-Bauteil,
  umgewandelt wie in tools/symbole/fluidsim_symbole.py, mit Drehung, Spiegelung und Maßstab der Schaltung;
- die Kennzeichnung (-MM1, -QM1, -MB1 …), wo FluidSIM eine vergibt;
- jede Leitung als QET-Leitung zwischen denselben Anschlüssen (QET legt den Weg neu);
- Schalldämpfer und Blindstopfen als Abschluss des Anschlusses, an dem sie sitzen;
- Spule ↔ Ventil: der Verweis am Ventilanschluss (solenoid-valve-coil-target) als Master/Slave-Verknüpfung.

Nicht übernommen: Rahmen und Schriftfeld, Tabellen, Zustandsdiagramme, Stücklisten, GRAFCET, freie Texte,
Simulationswerte; Kontakt ↔ Relais im elektrischen Teil (nur Kennzeichnung).

    python fluidsim_plan.py SCHALTUNG.circ AUSGABE.qet [--qet build/deploy/QElectroTech.exe]
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
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "tools" / "symbole"))
import fluidsim_symbole as fs  # noqa: E402

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


def plan(circ: Path, sammlung: Path):
    """Bauteile in sammlung schreiben; @return die Operationen für qet_edit und eine Zusammenfassung."""
    w = lesen(circ)
    grafik = abschnitt(w, "symbol-graphics")
    if grafik is None:
        raise SystemExit("keine Symbole in der Datei")
    symbole = {s.get("ident"): s for s in grafik.findall("symbol")}
    alle = objekte(w)
    verbindungen = list(w.iter("connect"))
    nach_id = {o.get("ident"): o for o in alle}

    ops = [{"op": "set_folio_border", "folio": 0, "property": "preset", "value": "a3-landscape"}]
    bauteile = {}      # ident -> {"uuids", "link", "id"}
    zahl = {"bauteile": 0, "leitungen": 0, "verknuepfungen": 0, "ausgelassen": 0}
    for o in alle:
        objekt = o.get("object-ref") or ""
        if o.tag == "component" and (not objekt or objekt in fs.ABSCHLUSS or objekt.startswith(AUSLASSEN)):
            continue
        beschr = o.find("dki-data/description")
        name = (beschr.get("value") if beschr is not None else "") or objekt or "FluidSIM"
        # Zwei Durchläufe: die UUIDs hängen am Inhalt, so werden gleiche Symbole ein Bauteil
        probe = fs.bauteil_umwandeln(symbole, o, alle, verbindungen, (name, name), circ.name, "probe", lage=True)
        if probe is None or not probe["uuids"]:
            zahl["ausgelassen"] += 1
            continue
        saat = "plan/" + hashlib.sha1(probe["text"].encode("utf-8")).hexdigest()
        erg = fs.bauteil_umwandeln(symbole, o, alle, verbindungen, (name, name), circ.name, saat, lage=True)
        datei = sammlung / f"{saat[5:17]}.elmt"
        datei.write_text(erg["text"], encoding="utf-8", newline="\n")
        m = fs.mitte(o)
        x = (m[0] + erg["anker"][0]) * fs.K
        y = (m[1] + erg["anker"][1]) * fs.K
        oid = "o" + o.get("ident")
        ops.append({"op": "add_element", "id": oid, "folio": 0, "path": f"common://{datei.name}",
                    "x": round(x, 1), "y": round(y, 1)})
        kenn = o.find("dki-data/designation")
        wert = kenn.get("value", "") if kenn is not None else ""
        if wert and not wert.startswith("?"):
            ops.append({"op": "set_label", "folio": 0, "element": "$" + oid, "label": wert})
        bauteile[o.get("ident")] = {"uuids": erg["uuids"], "link": erg["link"], "id": oid}
        zahl["bauteile"] += 1

    def anschluss_uuid(bezug):
        oid, _, x = bezug.partition(".")
        b = bauteile.get(oid)
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

    netze_mit_verzweigung = set()
    direkt = []
    for v in verbindungen:
        a, b = v.get("con1", ""), v.get("con2", "")
        va, vb = ist_verzweigung(a), ist_verzweigung(b)
        if not va and not vb:
            direkt.append((a, b))
            continue
        # jede Verzweigung ist ein Knoten (alle ihre Anschlüsse sind verbunden)
        ka = a.partition(".")[0] if va else a
        kb = b.partition(".")[0] if vb else b
        eltern[wurzel(ka)] = wurzel(kb)
        netze_mit_verzweigung.add(ka)
        netze_mit_verzweigung.add(kb)

    def lage(bezug):
        oid, _, x = bezug.partition(".")
        return anschluss_lage(symbole, nach_id[oid], x) or (0.0, 0.0)

    netze = {}
    for k in netze_mit_verzweigung:
        if k in nach_id:          # die Verzweigung selbst
            continue
        netze.setdefault(wurzel(k), []).append(k)
    paare = list(direkt)
    for glieder in netze.values():
        # Kette von links oben nach rechts unten: kurze, übersichtliche Leitungen
        glieder.sort(key=lambda b: (round(lage(b)[1]), lage(b)[0]))
        paare += list(zip(glieder, glieder[1:]))

    for a, b in paare:
        ea, ta = anschluss_uuid(a)
        eb, tb = anschluss_uuid(b)
        if ea and eb and ta and tb:
            ops.append({"op": "add_conductor", "folio": 0, "from": "$" + ea, "from_terminal": "{" + ta + "}",
                        "to": "$" + eb, "to_terminal": "{" + tb + "}"})
            zahl["leitungen"] += 1

    # Spule <-> Ventil: am Ventilanschluss steht der Verweis auf die Spule
    for o in alle:
        ventil = bauteile.get(o.get("ident"))
        if ventil is None or ventil["link"] != "master":
            continue
        for con in o.findall("connector"):
            ziel = con.find("link-props/target")
            art = con.find("link-props/type")
            if ziel is None or art is None or art.get("value") != "solenoid-valve-coil-target":
                continue
            spule = bauteile.get(ziel.get("comp-ref", ""))
            if spule and spule["link"] == "slave":
                ops.append({"op": "link_elements", "folio": 0, "element": "$" + ventil["id"],
                            "to_folio": 0, "to": "$" + spule["id"]})
                zahl["verknuepfungen"] += 1
    return ops, zahl


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("circ", nargs="?", type=Path)
    ap.add_argument("ausgabe", nargs="?", type=Path)
    ap.add_argument("--qet", type=Path, default=REPO / "build" / "deploy" / "QElectroTech.exe")
    ap.add_argument("--titel", default=None)
    ap.add_argument("--pruefe-drehung", type=Path, metavar="ORDNER")
    ap.add_argument("--nur-zeigen", action="store_true", help="Operationen als JSON ausgeben, nichts schreiben")
    a = ap.parse_args(argv)

    if a.pruefe_drehung:
        pruefe_drehung(a.pruefe_drehung)
        return
    if not a.circ or (not a.ausgabe and not a.nur_zeigen):
        ap.error("SCHALTUNG.circ und AUSGABE.qet angeben")

    with tempfile.TemporaryDirectory(prefix="fluidsim-plan-") as tmp:
        sammlung = Path(tmp) / "elements"
        sammlung.mkdir()
        ops, zahl = plan(a.circ, sammlung)
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
        qet_mcp.tool_project_new(str(a.qet), str(leer), title=a.titel or a.circ.stem, folios=[a.circ.stem],
                                 elements_dir=str(sammlung))
        erg = qet_mcp.tool_edit(str(a.qet), str(leer), ops, str(a.ausgabe), elements_dir=str(sammlung))
        fehler = [o for o in erg.get("operations", []) if not o.get("succeeded")]
        print(f"{a.ausgabe}: {zahl['bauteile']} Bauteile, {zahl['leitungen']} Leitungen, "
              f"{zahl['verknuepfungen']} Spule-Ventil-Verknüpfungen, {zahl['ausgelassen']} Objekte ohne Anschluss "
              f"ausgelassen; {len(fehler)} Operationen fehlgeschlagen.")
        for o in fehler[:10]:
            print("  FEHLER", json.dumps(o, ensure_ascii=False))
        if not erg.get("saved"):
            sys.exit("Projekt nicht gespeichert: " + json.dumps(erg, ensure_ascii=False)[:2000])


if __name__ == "__main__":
    main()
