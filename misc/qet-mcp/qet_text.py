# SPDX-License-Identifier: GPL-2.0-or-later
"""QET-Text v1: ein Schaltplanblatt als Text, eine Zeile je Element.

Für KI-Agenten und Menschen gleichermaßen lesbar und schreibbar; das MCP-Werkzeug
qet_folio_text gibt ein Blatt so aus, qet_folio_text_check prüft einen Text
gegen das Projekt (Fehler mit Zeilennummer), qet_folio_text_apply macht aus dem
Text qet_edit-Operationen und wendet sie auf eine Kopie des Projekts an.

    # QET-Text v1
    blatt 3 titel="Steuerung"
    bauteil -K1 typ="embed://import/schuetz.elmt" @200,300 rot=0 uuid={…} function="Pumpe"
    bauteil -S1 typ="embed://import/taster.elmt" @200,150
    leiter -S1:14 -K1:A1 nummer=3.01 farbe=BU querschnitt=1,5

- Koordinaten in Pixeln der Zeichnung (Raster 10 px), Drehung 0/90/180/270.
- Ein Bauteil ohne Kennzeichen heißt #n (n zählt die Bauteile des Blatts ab 1).
- Ein Anschluss heißt wie im Symbol (A1, 14); ohne Namen sein Index: -K1:@0.
- Ein Blatt mit vorhandener Nummer wird ersetzt: Bauteile mit uuid= bleiben und
  werden verschoben, umbenannt, ergänzt; Bauteile und Leiter, die der Text nicht
  mehr nennt, werden gelöscht. Eine neue Nummer hängt ein Blatt an.
"""
from __future__ import annotations

import re
import shlex
import xml.etree.ElementTree as ET

HEADER = "# QET-Text v1: ein Element je Zeile; Koordinaten in px (Raster 10), Drehung 0/90/180/270"
_ORIENT = {"0": 0, "1": 90, "2": 180, "3": 270}
_TERM_O = {"n": 0, "e": 1, "s": 2, "w": 3}
_DOCK = {0: (0.0, 4.0), 1: (-4.0, 0.0), 2: (0.0, -4.0), 3: (4.0, 0.0)}
_INFO_SKIP = {"label", "formula"}
_WIRE_PROPS = (("nummer", "num"), ("farbe", "conductor_color"),
               ("querschnitt", "conductor_section"), ("funktion", "function"))


def _uuid(value: str) -> str:
    return (value or "").strip().strip("{}").lower()


def _orientation(value: str) -> int:
    value = (value or "").strip()
    if value[:1] in _TERM_O:
        return _TERM_O[value[:1]]
    try:
        return int(value) % 4
    except ValueError:
        return 0


def _definitions(root: ET.Element) -> dict:
    """embed://… -> [(uuid, name, x, y, orientation)] of the embedded symbols."""
    out = {}

    def walk(node, path):
        for child in node:
            if child.tag == "category":
                walk(child, path + [child.get("name", "")])
            elif child.tag == "element":
                terms = []
                for t in child.findall("definition/description/terminal"):
                    try:
                        terms.append((_uuid(t.get("uuid")), (t.get("name") or "").strip(),
                                      float(t.get("x")), float(t.get("y")),
                                      _orientation(t.get("orientation"))))
                    except (TypeError, ValueError):
                        continue
                out["embed://" + "/".join(path + [child.get("name", "")])] = terms
    coll = root.find("collection")
    if coll is not None:
        walk(coll, [])
    return out


def _info(el: ET.Element) -> dict:
    out = {}
    bag = el.find("elementInformations")
    if bag is not None:
        for i in bag.findall("elementInformation"):
            if i.get("name"):
                out[i.get("name")] = (i.text or "").strip()
    return out


def _q(value: str) -> str:
    """A value as the text writes it: bare when it can be, quoted else."""
    if value and re.fullmatch(r"[^\s\"'=#]+", value):
        return value
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def _folio_model(root: ET.Element, folio: int) -> dict:
    """Elements and conductors of folio @p folio (from 1), the names the text uses."""
    diagrams = list(root.iter("diagram"))
    if folio < 1 or folio > len(diagrams):
        raise ValueError(f"folio {folio} does not exist ({len(diagrams)} folio(s))")
    d = diagrams[folio - 1]
    defs = _definitions(root)
    elements, by_uuid, term_of_id = [], {}, {}
    for n, el in enumerate(d.findall("elements/element"), start=1):
        info = _info(el)
        label = info.get("label", "").strip()
        etype = el.get("type", "")
        e = {"uuid": _uuid(el.get("uuid")), "type": etype,
             "name": label if label and not label.startswith("#") and " " not in label else f"#{n}",
             "label": label, "x": float(el.get("x", "0")), "y": float(el.get("y", "0")),
             "rot": _ORIENT.get((el.get("orientation") or "0")[:1], 0),
             "info": {k: v for k, v in info.items() if k not in _INFO_SKIP and v},
             "terms": defs.get(etype, [])}
        elements.append(e)
        by_uuid[e["uuid"]] = e
        for t in el.findall("terminals/terminal"):
            try:
                tx, ty, to = float(t.get("x")), float(t.get("y")), _orientation(t.get("orientation"))
            except (TypeError, ValueError):
                continue
            for k, (_, _, x, y, o) in enumerate(e["terms"]):
                dx, dy = _DOCK[o]
                if abs(tx - (x + dx)) < 1e-6 and abs(ty - (y + dy)) < 1e-6 and o == to:
                    term_of_id[t.get("id")] = (e["uuid"], k)
                    break
    conductors = []
    for c in d.findall("conductors/conductor"):
        ends = []
        for ea, ta in (("element1", "terminal1"), ("element2", "terminal2")):
            owner, tid = _uuid(c.get(ea)), c.get(ta, "")
            if owner and owner in by_uuid:
                terms = by_uuid[owner]["terms"]
                ends.append((owner, next((i for i, t in enumerate(terms) if t[0] == _uuid(tid)), None)))
            else:
                ends.append(term_of_id.get(tid, (None, None)))
        conductors.append({"ends": ends, "num": c.get("num", ""), "conductor_color": c.get("conductor_color", ""),
                           "conductor_section": c.get("conductor_section", ""),
                           "function": c.get("function", "")})
    return {"title": d.get("title", ""), "elements": elements, "by_uuid": by_uuid,
            "conductors": conductors, "count": len(diagrams)}


def _end_text(model: dict, end) -> str:
    owner, k = end
    e = model["by_uuid"].get(owner)
    if not e or k is None:
        return "?:?"
    name = e["terms"][k][1] if k < len(e["terms"]) else ""
    ok = name and re.fullmatch(r"[^\s:\"'=#@]+", name)
    return f"{e['name']}:{name if ok else '@' + str(k)}"


def export_folio(root: ET.Element, folio: int) -> str:
    """Folio @p folio (from 1) in QET-Text v1."""
    m = _folio_model(root, folio)
    lines = [HEADER, f"blatt {folio} titel={_q(m['title'])}"]
    for e in m["elements"]:
        parts = [f"bauteil {e['name']} typ={_q(e['type'])} @{e['x']:g},{e['y']:g}"]
        if e["rot"]:
            parts.append(f"rot={e['rot']}")
        parts.append(f"uuid={e['uuid']}")
        parts += [f"{k}={_q(v)}" for k, v in sorted(e["info"].items())]
        lines.append(" ".join(parts))
    for c in m["conductors"]:
        parts = [f"leiter {_end_text(m, c['ends'][0])} {_end_text(m, c['ends'][1])}"]
        parts += [f"{word}={_q(c[attr])}" for word, attr in _WIRE_PROPS if c[attr]]
        lines.append(" ".join(parts))
    return "\n".join(lines) + "\n"


def parse(text: str) -> tuple[list, list]:
    """The lines of @p text as commands, and the errors (line number, text)."""
    commands, errors = [], []
    for n, raw in enumerate(text.splitlines(), start=1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        try:
            tokens = shlex.split(line, posix=True)
        except ValueError as exc:
            errors.append((n, f"Anführungszeichen nicht geschlossen ({exc})"))
            continue
        word, opts, pos = tokens[0].lower(), {}, []
        for a in tokens[1:]:
            if "=" in a and not a.startswith("@"):
                k, v = a.split("=", 1)
                opts[k.strip()] = v
            else:
                pos.append(a)
        cmd = {"line": n, "word": word, "opts": opts, "pos": pos}
        if word == "blatt":
            if not pos or not pos[0].isdigit() or int(pos[0]) < 1:
                errors.append((n, "blatt braucht eine Blattnummer ab 1"))
                continue
            cmd["folio"] = int(pos[0])
        elif word == "bauteil":
            at = [p for p in pos if p.startswith("@")]
            names = [p for p in pos if not p.startswith("@")]
            if not names:
                errors.append((n, "bauteil braucht ein Kennzeichen (-K1) oder #n"))
                continue
            cmd["name"] = names[0]
            if not at:
                errors.append((n, f"{names[0]}: Position @x,y fehlt"))
                continue
            try:
                cmd["x"], cmd["y"] = (float(v) for v in at[0][1:].split(","))
            except ValueError:
                errors.append((n, f"{names[0]}: Position {at[0]} ist keine Zahl x,y"))
                continue
            if opts.get("rot", "0") not in ("0", "90", "180", "270"):
                errors.append((n, f"{names[0]}: Drehung {opts['rot']} – erlaubt sind 0, 90, 180, 270"))
                continue
            cmd["rot"] = int(opts.get("rot", "0"))
        elif word == "leiter":
            if len(pos) != 2 or any(":" not in p for p in pos):
                errors.append((n, "leiter braucht zwei Enden Kennzeichen:Anschluss (-S1:14 -K1:A1)"))
                continue
            cmd["ends"] = [tuple(p.rsplit(":", 1)) for p in pos]
        else:
            errors.append((n, f"unbekanntes Wort {tokens[0]!r} (blatt, bauteil, leiter)"))
            continue
        commands.append(cmd)
    return commands, errors


def _terminal_index(terms: list, name: str):
    if name.startswith("@") and name[1:].isdigit():
        k = int(name[1:])
        return k if not terms or k < len(terms) else None
    return next((k for k, t in enumerate(terms) if t[1] == name), None)


def check(root: ET.Element, text: str) -> dict:
    """The errors of @p text against the project, each with its line."""
    commands, errors = parse(text)
    heads = [c for c in commands if c["word"] == "blatt"]
    if len(heads) != 1:
        errors.append((0, "der Text braucht genau eine Zeile blatt <Nummer>"))
        return {"ok": False, "errors": [{"line": n, "text": t} for n, t in sorted(errors)]}
    folio = heads[0]["folio"]
    count = len(list(root.iter("diagram")))
    if folio > count + 1:
        errors.append((heads[0]["line"], f"Blatt {folio}: das Projekt hat {count} Blätter, "
                                         f"anhängen geht nur als Blatt {count + 1}"))
    model = _folio_model(root, folio) if folio <= count else {"by_uuid": {}}
    defs = _definitions(root)
    names = {}
    for c in (c for c in commands if c["word"] == "bauteil"):
        if c["name"] in names:
            errors.append((c["line"], f"{c['name']} steht zweimal auf dem Blatt "
                                      f"(Zeile {names[c['name']]['line']})"))
        names[c["name"]] = c
        u = _uuid(c["opts"].get("uuid", ""))
        if u and u not in model["by_uuid"]:
            errors.append((c["line"], f"{c['name']}: uuid {u} liegt nicht auf Blatt {folio}"))
        if not u and not c["opts"].get("typ"):
            errors.append((c["line"], f"{c['name']}: neues Bauteil braucht typ=<Symbolpfad>"))
    for c in (c for c in commands if c["word"] == "leiter"):
        for label, term in c["ends"]:
            b = names.get(label)
            if not b:
                errors.append((c["line"], f"{label} ist auf diesem Blatt nicht als bauteil genannt"))
                continue
            u = _uuid(b["opts"].get("uuid", ""))
            etype = b["opts"].get("typ") or model["by_uuid"].get(u, {}).get("type", "")
            terms = defs.get(etype)
            if terms is None:
                if not (term.startswith("@") and term[1:].isdigit()):
                    errors.append((c["line"], f"{label}:{term}: die Anschlüsse von {etype} kennt das "
                                              f"Projekt nicht – Anschluss als Index angeben ({label}:@0)"))
            elif _terminal_index(terms, term) is None:
                known = ", ".join(t[1] or f"@{k}" for k, t in enumerate(terms))
                errors.append((c["line"], f"{label}: kein Anschluss {term!r} (vorhanden: {known})"))
    return {"ok": not errors, "folio": folio, "new_folio": folio > count,
            "errors": [{"line": n, "text": t} for n, t in sorted(errors)]}


def operations(root: ET.Element, text: str) -> list:
    """The qet_edit operations that make the folio what @p text says."""
    result = check(root, text)
    if not result["ok"]:
        raise ValueError("; ".join(f"Zeile {e['line']}: {e['text']}" for e in result["errors"]))
    commands, _ = parse(text)
    folio = result["folio"]
    defs = _definitions(root)
    head = next(c for c in commands if c["word"] == "blatt")
    ops = []
    if result["new_folio"]:
        ops.append({"op": "add_folio", "id": "folio"})
        fref = "$folio"
        model = {"by_uuid": {}, "elements": [], "conductors": []}
    else:
        fref = folio - 1
        model = _folio_model(root, folio)
    if "titel" in head["opts"]:
        ops.append({"op": "set_folio_title", "folio": fref, "title": head["opts"]["titel"]})

    # Bauteile: vorhandene (uuid=) bleiben und werden angepasst, die anderen kommen dazu
    refs, kept, types = {}, set(), {}
    for n, c in enumerate(c for c in commands if c["word"] == "bauteil"):
        u = _uuid(c["opts"].get("uuid", ""))
        if u:
            e = model["by_uuid"][u]
            ref = "{" + u + "}"
            kept.add(u)
            types[c["name"]] = e["type"]
            if (e["x"], e["y"]) != (c["x"], c["y"]):
                ops.append({"op": "set_position", "folio": fref, "element": ref, "x": c["x"], "y": c["y"]})
            if (c["rot"] - e["rot"]) % 360:
                ops.append({"op": "rotate_element", "folio": fref, "element": ref,
                            "angle": (c["rot"] - e["rot"]) % 360})
            old_label, old_info = e["label"], e["info"]
        else:
            ref = f"$e{n}"
            types[c["name"]] = c["opts"]["typ"]
            ops.append({"op": "add_element", "id": f"e{n}", "folio": fref, "path": c["opts"]["typ"],
                        "x": c["x"], "y": c["y"]})
            if c["rot"]:
                ops.append({"op": "rotate_element", "folio": fref, "element": ref, "angle": c["rot"]})
            old_label, old_info = "", {}
        if not c["name"].startswith("#") and c["name"] != old_label:
            ops.append({"op": "set_label", "folio": fref, "element": ref, "label": c["name"]})
        for k, v in c["opts"].items():
            if k not in ("typ", "uuid", "rot") and old_info.get(k) != v:
                ops.append({"op": "set_info", "folio": fref, "element": ref, "key": k, "value": v})
        refs[c["name"]] = ref

    # Leiter: die der Text nicht mehr nennt, gehen; neue kommen dazu
    def key_of(a, b):
        return tuple(sorted([a, b], key=str))
    wanted = {}
    for c in (c for c in commands if c["word"] == "leiter"):
        ends = []
        for label, term in c["ends"]:
            terms = defs.get(types[label], [])
            k = _terminal_index(terms, term)
            ends.append((refs[label], k, terms[k][0] if terms and k is not None else k))
        wanted[key_of((ends[0][0], ends[0][1]), (ends[1][0], ends[1][1]))] = (ends, c["opts"])
    existing = {}
    for cond in model["conductors"]:
        (o1, k1), (o2, k2) = cond["ends"]
        if o1 is None or o2 is None or k1 is None:
            continue
        key = key_of(("{" + o1 + "}", k1), ("{" + o2 + "}", k2))
        existing[key] = cond
        if key not in wanted and o1 in kept and o2 in kept:
            terms = model["by_uuid"][o1]["terms"]
            ops.append({"op": "delete_conductor", "folio": fref, "element": "{" + o1 + "}",
                        "terminal": terms[k1][0] if terms else k1})
    for key, (ends, opts) in wanted.items():
        (r1, _, t1), (r2, _, t2) = ends
        old = existing.get(key)
        if old is None:
            ops.append({"op": "add_conductor", "folio": fref, "from": r1, "from_terminal": t1,
                        "to": r2, "to_terminal": t2})
        for word, prop in _WIRE_PROPS:
            if word in opts and (old is None or old.get(prop, "") != opts[word]):
                ops.append({"op": "set_conductor", "folio": fref, "element": r1, "terminal": t1,
                            "property": prop, "value": opts[word]})

    # Bauteile, die der Text nicht mehr nennt (ihre Leiter gehen mit)
    for e in model["elements"]:
        if e["uuid"] not in kept:
            ops.append({"op": "delete_element", "folio": fref, "element": "{" + e["uuid"] + "}"})
    return ops
