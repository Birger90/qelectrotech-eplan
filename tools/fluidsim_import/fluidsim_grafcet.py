"""GRAFCET aus FluidSIM-6-Schaltungen mit den GRAFCET-Symbolen der Sammlung (20_logic/…/202020_grafcet).

FluidSIM speichert einen GRAFCET als Schaltungsobjekte Grafcet.Step, Grafcet.Transition, Grafcet.Action,
Grafcet.MultiPin (Verzweigung) und Grafcet.SyncRule (Doppellinie: Parallelverzweigung oder Synchronisation),
verbunden über <connect> wie Leitungen. Übernommen wird:
- jeder Schritt mit seiner Nummer (Anfangsschritt: flags="initial");
- jede Transition mit ihrer Weiterschaltbedingung in der Schreibweise des Simulationskerns
  (FluidSIM „*“ → „·“, „+“ → „∨“, „RE x“ → „↑x“, „FE x“ → „↓x“, „[…]“ → Bedingung, „-SF1.signal“ → „-SF1“);
- jede Aktion: kontinuierlich (actionType 0), bedingt (1), speichernd bei Aktivierung (2),
  bei Deaktivierung (3), beim Auslösen (4, angenähert als bei Aktivierung), bei Ereignis (5);
  Zuweisungen als „A := 1“; aneinandergehängte Aktionen gehören zum Schritt der ersten;
- die Verbindungen; Doppellinien werden zur Parallelverzweigung bzw. Synchronisation.

Nicht übernommen: Einschließende Schritte/Teil-GRAFCETs (SubGrafcet), Zwangssteuerungen (actionType 6),
Makroschritte. Sie stehen im Bericht.
"""
import re

G = "20_logic/2020_flow_chart/202020_grafcet/"
DATEI = {
    "anfang": G + "20202000_step.elmt", "schritt": G + "20202001_step.elmt",
    "transition": G + "20202060_transition.elmt",
    "0": G + "20202020_action.elmt", "1": G + "20202021_action.elmt", "2": G + "20202022_action.elmt",
    "3": G + "20202023_action.elmt", "4": G + "20202022_action.elmt", "5": G + "20202024_action.elmt",
    "parallel": G + "20202070_parallel_branch.elmt", "sync": G + "20202071_synchronization.elmt",
}
# FluidSIM-Anschluss -> Anschluss des QET-Symbols
ANSCHLUSS = {"GC_S_in": "in", "GC_S_out": "out", "GC_S_A": "action", "GC_T_in": "in", "GC_T_out": "out"}


def bedingung(text):
    """FluidSIM-Schreibweise einer Bedingung in die des Simulationskerns (IEC 60848)."""
    s = (text or "").strip()
    s = s.replace(".signal", "")
    s = re.sub(r"\[(.*?)\]", r"\1", s)
    s = re.sub(r"(^|[\s(*+])RE\s+", "\\1↑", s)
    s = re.sub(r"(^|[\s(*+])FE\s+", "\\1↓", s)
    s = s.replace("*", " · ").replace("+", " ∨ ")
    return re.sub(r"\s+", " ", s).strip()


def werte(o):
    return {e.get("ident"): e.get("value") or "" for e in o if e.tag in ("const", "parameter")}


def terminals(pfad_elmt):
    """{Anschlussname: (uuid, x, y)} eines Symbols der Sammlung."""
    text = pfad_elmt.read_text(encoding="utf-8")
    out = {}
    for tag in re.findall(r"<terminal\b[^>]*>", text):
        name = re.search(r'\sname="([^"]*)"', tag)
        uuid = re.search(r'uuid="\{([^}]+)\}"', tag)
        x = float(re.search(r'\sx="([-\d.]+)"', tag).group(1))
        y = float(re.search(r'\sy="([-\d.]+)"', tag).group(1))
        if name and uuid:
            out[name.group(1)] = (uuid.group(1), x, y)
    return out


def umwandeln(blatt, objekte, verbindungen, sammlung, K, mitte):
    """Operationen für qet_edit; @return (ops, zahl, offen)."""
    ops, offen = [], []
    zahl = {"schritte": 0, "transitionen": 0, "aktionen": 0, "verbindungen": 0}
    gc = {o.get("ident"): o for o in objekte if (o.get("object-ref") or "").startswith("Grafcet.")}
    if not gc:
        return ops, zahl, offen
    anschl = {k: terminals(sammlung / v) for k, v in DATEI.items()}
    platziert = {}     # FluidSIM-Ident -> (id, art)

    def neu(o, art, x, y, label=None):
        oid = f"g{blatt}_{o.get('ident')}"
        ops.append({"op": "add_element", "id": oid, "folio": blatt, "path": "common://" + DATEI[art],
                    "x": round(x, 1), "y": round(y, 1)})
        if label:
            ops.append({"op": "set_label", "folio": blatt, "element": "$" + oid, "label": label})
        platziert[o.get("ident")] = (oid, art)
        return oid

    for ident, o in gc.items():
        art = o.get("object-ref")
        w = werte(o)
        mx, my = mitte(o)
        if art == "Grafcet.Step":
            flags = w.get("flags", "")
            if flags not in ("", "initial"):
                offen.append(f"Schritt {w.get('name')}: {flags} (wie ein einfacher Schritt übernommen)")
            neu(o, "anfang" if flags == "initial" else "schritt", mx * K, my * K, w.get("name", ""))
            zahl["schritte"] += 1
        elif art == "Grafcet.Transition":
            oid = neu(o, "transition", mx * K, my * K, bedingung(w.get("condition")) or "1")
            if w.get("cond-descr"):
                ops.append({"op": "set_info", "folio": blatt, "element": "$" + oid, "key": "comment",
                            "value": w["cond-descr"]})
            zahl["transitionen"] += 1
        elif art == "Grafcet.Action":
            typ = w.get("actionType", "0")
            if typ not in DATEI:
                offen.append(f"Aktion Typ {typ} ({w.get('var')}, {w.get('steps')}) nicht übernommen")
                continue
            var = w.get("var", "").strip()
            if typ in ("0", "1"):
                text = var
            else:
                text = f"{var} := {w.get('allocation') or '1'}"
            x1 = float(o.get("x1", 0))
            oid = neu(o, typ, x1 * K, my * K, text)
            cond = bedingung(w.get("condition"))
            if typ == "5" and cond and not cond.startswith(("↑", "↓")):
                cond = f"↑({cond})"
            if typ in ("1", "5") and cond:
                ops.append({"op": "add_element_text", "folio": blatt, "element": "$" + oid, "source": "text",
                            "value": cond, "x": 84, "y": -42})
            if typ == "4":
                offen.append(f"Aktion beim Auslösen „{text}“ als Aktion bei Aktivierung übernommen")
            if w.get("alloc-descr"):
                ops.append({"op": "set_info", "folio": blatt, "element": "$" + oid, "key": "comment",
                            "value": w["alloc-descr"]})
            zahl["aktionen"] += 1
        elif art in ("Grafcet.MultiPin", "Grafcet.SyncRule"):
            pass
        else:
            offen.append(f"{art} nicht übernommen")

    # Netze: Verzweigungen und Doppellinien verbinden alle ihre Anschlüsse
    eltern = {}

    def wurzel(k):
        while eltern.setdefault(k, k) != k:
            eltern[k] = eltern[eltern[k]]
            k = eltern[k]
        return k

    def knoten(bezug):
        oid, _, a = bezug.partition(".")
        o = gc.get(oid)
        if o is None:
            return None
        if o.get("object-ref") in ("Grafcet.MultiPin", "Grafcet.SyncRule"):
            return ("knoten", oid)
        return (oid, a)

    for v in verbindungen:
        a, b = knoten(v.get("con1", "")), knoten(v.get("con2", ""))
        if a and b:
            eltern[wurzel(a)] = wurzel(b)

    # Aktionen am Schritt: alle Aktionen, die (auch über andere Aktionen) an GC_S_A hängen
    aktion_schritt = {}
    for k in list(eltern):
        if k[0] == "knoten":
            continue
        if gc[k[0]].get("object-ref") == "Grafcet.Step" and k[1] == "GC_S_A":
            r = wurzel(k)
            for k2 in list(eltern):
                if k2[0] != "knoten" and wurzel(k2) == r and gc[k2[0]].get("object-ref") == "Grafcet.Action":
                    aktion_schritt[k2[0]] = k[0]
    for aid, sid in aktion_schritt.items():
        if aid in platziert and sid in platziert:
            (ea, art_a), (es, art_s) = platziert[aid], platziert[sid]
            ops.append({"op": "add_conductor", "folio": blatt, "from": "$" + es,
                        "from_terminal": "{" + anschl[art_s]["action"][0] + "}", "to": "$" + ea,
                        "to_terminal": "{" + anschl[art_a]["step"][0] + "}"})
            zahl["verbindungen"] += 1

    # Ablauf: Schritte und Transitionen je Netz
    netze = {}
    for k in list(eltern):
        if k[0] == "knoten" or k[1] not in ANSCHLUSS or k[1] == "GC_S_A" or k[0] not in platziert:
            continue
        netze.setdefault(wurzel(k), []).append(k)
    for r, glieder in netze.items():
        if len(glieder) < 2:
            continue
        oben = [g for g in glieder if g[1].endswith("_out")]     # enden hier von oben
        unten = [g for g in glieder if g[1].endswith("_in")]     # beginnen hier nach unten
        bar = next((k[1] for k in list(eltern) if k[0] == "knoten" and wurzel(k) == r
                    and gc[k[1]].get("object-ref") == "Grafcet.SyncRule"), None)

        def lage(g):
            return mitte(gc[g[0]])

        def draht(g, ziel_id, ziel_anschluss):
            eid, art = platziert[g[0]]
            ops.append({"op": "add_conductor", "folio": blatt, "from": "$" + eid,
                        "from_terminal": "{" + anschl[art][ANSCHLUSS[g[1]]][0] + "}", "to": "$" + ziel_id,
                        "to_terminal": "{" + ziel_anschluss + "}"})
            zahl["verbindungen"] += 1

        if bar is not None and (len(oben) > 1 or len(unten) > 1):
            o = gc[bar]
            art = "sync" if len(oben) > len(unten) else "parallel"
            mx, my = mitte(o)
            bid = neu(o, art, mx * K, my * K)
            t = anschl[art]
            for g in oben + unten:
                gx = lage(g)[0]
                if art == "parallel":
                    name = "in" if g in oben else ("1" if gx < mx else "2")
                else:
                    name = "out" if g in unten else ("1" if gx < mx else "2")
                draht(g, bid, t[name][0])
            continue
        # sonst der Reihe nach von oben nach unten verbinden
        glieder.sort(key=lambda g: (lage(g)[1], lage(g)[0]))
        for g1, g2 in zip(glieder, glieder[1:]):
            eid2, art2 = platziert[g2[0]]
            draht(g1, eid2, anschl[art2][ANSCHLUSS[g2[1]]][0])
    return ops, zahl, offen
