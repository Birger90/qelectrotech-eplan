"""Plan und Blatt: Raster, Bauteile mit Anker-Anschluss, Leitungen, Potentialschienen, Verweise, Erzeugen.

Raster (A3 quer, Pixel wie QET): 10 Spalten à 156 px. Waagerechte Bänder von oben nach unten:
Potentialschienen (y 80–160) -> Schutz/Schalten -> Steuerung/SPS -> Klemmen -> Feldgeräte -> Funktionstext.
Bausteine bekommen eine Spalte (oder mehrere) und legen ihre Symbole selbst in die Bänder.

Potentialschienen entstehen aus den Abgriffen: jeder Baustein fragt `abgriff("L1", x)` und bekommt einen
T-Abzweig auf der Schiene; beim Erzeugen werden die Abzweige von links nach rechts verbunden, links ein
kommender, rechts ein gehender Verweispfeil. Gleichnamige Schienen aufeinanderfolgender Blätter werden als
echte QET-Folgeverweise verknüpft (den Querverweis „/Blatt.Spalte“ schreibt QET selbst).
"""
import html
import os
import re
import sys

from . import symbole as S

sys.path.insert(0, os.environ.get("QET_DIREKT_DIR", r"C:/Users/test/.claude/skills/qet-schaltplan-zeichnen/vorlagen"))

SPALTEN, SPALTE_B, X0 = 10, 156, 20          # Spalte i: X0 + i*156 … +156 (X0 am Testblatt prüfen)
BAND = {"schiene": 80, "schutz": 230, "schalten": 360, "steuer": 470, "klemme": 690, "feld": 800, "text": 985}
SCHIENEN_ABSTAND = 20
# halbe Zeichenbreite je Punkt Schriftgröße (am PDF gemessen: „Druckschalter“ 10 pt = 79 px bei 13 Zeichen)
TEXT_HALB = 0.31
GROESSE = {"label": 10, "daten": 7, "funktion": 10, "verweis": 7, "klemme": 7}


def _richtung(von, nach):
    """Anschluss der Ecke bei `von`, der zum Punkt `nach` zeigt."""
    dx, dy = nach[0] - von[0], nach[1] - von[1]
    if abs(dx) >= abs(dy):
        return "E" if dx > 0 else "W"
    return "S" if dy > 0 else "N"


def raster(x):
    """Auf QETs 10-px-Raster runden (Symbole rasten dort ein, Leitungen laufen dann gerade)."""
    return int(round(x / 10.0)) * 10


def spalte_links(i):
    return raster(X0 + i * SPALTE_B)


def spalte_mitte(i):
    return raster(X0 + i * SPALTE_B + SPALTE_B / 2)


class Teil:
    """Ein gesetztes Bauteil; an(name) liefert den Anschluss, pos(name) seine Lage auf dem Blatt."""

    def __init__(self, blatt, ident, pfad, x, y, drehung=0):
        self.blatt, self.ident, self.pfad, self.x, self.y = blatt, ident, pfad, x, y
        self.symbol, self.drehung = S.symbol(pfad), drehung % 360

    @property
    def ref(self):
        return f"${self.ident}"

    def an(self, wer):
        return self.symbol.anschluss(wer)

    def pos(self, wer):
        a = self.an(wer)
        dx, dy = _dreh(a.x, a.y, self.drehung)
        return (round(self.x + dx, 1), round(self.y + dy, 1))


def _dreh(x, y, grad):
    """Lage eines Anschlusses nach Drehung im Uhrzeigersinn (QET, y nach unten)."""
    return {0: (x, y), 90: (-y, x), 180: (-x, -y), 270: (y, -x)}[grad % 360]


def _label_frei(sym, label):
    """Steht das Kennzeichen an seinem Standardplatz im Weg einer Anschlussleitung (Leitungen laufen von oben bzw.
    unten senkrecht an die Anschlüsse), kommt es links neben das Symbol, an allen Anschlüssen vorbei.
    Rückgabe neue (x, y) oder None, wenn der Standardplatz frei ist."""
    if not sym.label_lage or sym.label_lage[3] != "AlignLeft" or not sym.anschluesse:
        return None
    lx, ly, g, _ = sym.label_lage
    breite, hoehe = 0.62 * g * len(label) + 4, 1.9 * g
    for a in sym.anschluesse:
        if not lx - 1 <= a.x <= lx + breite:
            continue
        if (a.richtung == "n" and ly < a.y) or (a.richtung == "s" and ly + hoehe > a.y):
            links = min(t.x for t in sym.anschluesse)
            return (round(links - 8 - breite), ly)
    return None


class Schiene:
    def __init__(self, name, y, x1, x2, text="", anfang=None, ende=None):
        self.name, self.y, self.x1, self.x2, self.text = name, y, x1, x2, text
        # Pfeile an den Enden: None = automatisch (nur wenn ein anderes Blatt das Potential führt),
        # False = keiner (Schiene entsteht hier bzw. endet hier), True = immer
        self.anfang, self.ende = anfang, ende
        self.abgriffe = []                                    # (x, Teil)
        self.kommend = self.gehend = None


class Blatt:
    def __init__(self, plan, folio, titel, anlage, ort, nummer):
        self.plan, self.folio, self.titel = plan, folio, titel
        self.anlage, self.ort, self.nummer = anlage, ort, str(nummer)
        self.ops, self.schienen, self.n, self.funktionen = [], {}, 0, []
        self.kaesten = []          # Mitten der SPS-Kanalkästen (x) – Funktionstext mittig darunter
        f = folio
        self.ops += [
            {"op": "set_folio_title", "folio": f, "title": titel},
            {"op": "set_folio", "folio": f, "property": "template", "value": plan.schriftfeld},
            {"op": "set_folio", "folio": f, "property": "plant", "value": anlage},
            {"op": "set_folio", "folio": f, "property": "locmach", "value": ort},
            {"op": "set_folio", "folio": f, "property": "folio", "value": str(nummer)},
            {"op": "set_folio_border", "folio": f, "property": "preset", "value": "a3-landscape"},
            {"op": "set_folio_border", "folio": f, "property": "columns", "value": str(SPALTEN)},
            {"op": "set_folio_border", "folio": f, "property": "column-width", "value": str(SPALTE_B)},
            {"op": "set_folio_border", "folio": f, "property": "display-rows", "value": "false"},
            {"op": "set_conductor_default", "folio": f, "property": "displaytext", "value": "false"}]

    # --- Grundbausteine -------------------------------------------------------------------------------
    def _id(self, ident=None):
        self.n += 1
        return f"f{self.folio}_{ident or 'e'}_{self.n}"

    def setze(self, pfad, x, y, anker=None, label=None, infos=None, felder=None, ident=None, label_xy=None, dy=0,
              drehung=0, label_pt=None):
        """Symbol so setzen, dass Anschluss `anker` bei x/y liegt (ohne Anker: Einfügepunkt bei x/y).
        infos: Artikeldaten {manufacturer, manufacturer_reference, designation, …} -> set_info.
        felder: Texte der Benutzerfelder in Reihenfolge (SPS-Kanal: Adresse, Symbol, Funktion …)."""
        sym = S.symbol(pfad)
        if label is not None and not isinstance(label, str):
            label = str(label)
        ex, ey = x, y + dy
        if anker is not None:
            a = sym.anschluss(anker)
            ax, ay = _dreh(a.x, a.y, drehung)
            ex, ey = x - ax, y + dy - ay
        t = Teil(self, self._id(ident), pfad, round(ex, 1), round(ey, 1), drehung)
        if "_Kanal" in pfad or "/lesbar/" in pfad or "_DI-" in pfad or "_DQ-" in pfad:
            self.kaesten.append(ex + sym.mitte_x)
        self.ops.append({"op": "add_element", "folio": self.folio, "path": pfad, "x": t.x, "y": t.y, "id": t.ident})
        if drehung % 360:
            self.ops.append({"op": "rotate_element", "folio": self.folio, "element": t.ref, "angle": drehung % 360})
        self.ops.append({"op": "set_label", "folio": self.folio, "element": t.ref, "label": label or ""})
        for k, v in (infos or {}).items():
            if v:
                self.ops.append({"op": "set_info", "folio": self.folio, "element": t.ref, "key": k, "value": str(v)})
        if sym.felder:
            # Benutzerfelder über ihren Index in der Symboldatei; ohne Vorgaben nur Platzhalter „…“ leeren
            # (Spulen tragen z. B. „A1“/„A2“ als Benutzerfeld)
            for k, (i, vorgabe) in enumerate(zip(sym.feld_index, sym.feld_vorgabe)):
                if felder is not None:
                    wert = felder[k] if k < len(felder) else ""
                elif vorgabe.strip() in ("…", "...", "..", "?"):
                    wert = ""
                else:
                    continue
                self.ops.append({"op": "set_element_text", "folio": self.folio, "element": t.ref, "index": i,
                                 "property": "text", "value": wert})
        if label and label_xy is None and not drehung % 360:
            label_xy = _label_frei(sym, label)
        if label_xy:
            # erst nach den Verknüpfungen setzen: link_elements setzt die Kennzeichen-Lage sonst zurück
            for k, v in zip(("x", "y"), label_xy):
                self.plan.lagen.append({"op": "set_element_text", "folio": self.folio, "element": t.ref,
                                 "index": sym.label_index, "property": k, "value": str(v)})
        if label and label_pt:
            self.plan.lagen.append({"op": "set_element_text", "folio": self.folio, "element": t.ref,
                                    "index": sym.label_index, "property": "size", "value": str(label_pt)})
        if label and label_pt:
            self.plan.lagen.append({"op": "set_element_text", "folio": self.folio, "element": t.ref,
                                    "index": sym.label_index, "property": "size", "value": str(label_pt)})
        return t

    def draht(self, a, an_a, b, an_b, gestrichelt=False):
        self.ops.append({"op": "add_conductor", "folio": self.folio, "from": a.ref, "from_terminal": a.an(an_a).uuid,
                         "to": b.ref, "to_terminal": b.an(an_b).uuid})
        if gestrichelt:
            self.ops.append({"op": "set_conductor", "folio": self.folio, "element": a.ref,
                             "terminal": a.an(an_a).uuid, "property": "style", "value": "dashed"})

    def leitung(self, a, an_a, b, an_b, ueber=()):
        """Leitung über Eckpunkte (Ecke ohne Punkt, wie EPLAN-Winkel): ueber = [(x, y), …]."""
        vorher, vorher_an = a, an_a
        p_alt = a.pos(an_a)
        punkte = list(ueber) + [b.pos(an_b)]
        for i, (x, y) in enumerate(ueber):
            e = self.setze(S.ECKE, x, y, anker="N")
            nach = punkte[i + 1]
            self.draht(vorher, vorher_an, e, _richtung((x, y), p_alt))
            vorher, vorher_an, p_alt = e, _richtung((x, y), nach), (x, y)
        self.draht(vorher, vorher_an, b, an_b)

    def text(self, s, x, y, groesse="daten", drehung=0):
        """Freier Text, Ursprung oben links; drehung 270 = senkrecht, von unten nach oben zu lesen."""
        # je Blatt und Text: derselbe Text kann auf verschiedenen Blättern verschieden groß sein
        self.plan.schrift[(self.folio, s)] = GROESSE[groesse] if isinstance(groesse, str) else groesse
        op = {"op": "add_text", "folio": self.folio, "text": s, "x": round(x, 1), "y": round(y, 1)}
        self.ops.append(op)
        if drehung:
            op["id"] = self._id("t")
            self.ops.append({"op": "rotate_text", "folio": self.folio, "index": f"${op['id']}", "angle": drehung})

    def rechts_text(self, s, x_ende, y, groesse="daten"):
        """Text, der etwa bei x_ende aufhört (Kennzeichen links neben einem Symbol)."""
        g = GROESSE[groesse] if isinstance(groesse, str) else groesse
        # reichlich geschätzte Breite (Abstand zum Symbol), nie über den linken Blattrand hinaus
        self.text(s, max(24, x_ende - 0.78 * g * len(s)), y, groesse)

    def verbinde(self, master, *slaves):
        """Master/Slave verknüpfen (Spule -> Kontakte, Schutzschalter -> Hilfskontakt), auch blattübergreifend."""
        for s in slaves:
            self.plan.links.append((s.blatt.folio, s, master.blatt.folio, master))

    def funktion(self, von, bis, zeilen):
        """Funktionstext unter den Spalten von..bis (mittig), wie der EPLAN-Pfadfunktionstext."""
        self.funktionen.append((von, bis, zeilen))

    # --- Potentialpfeile (Abbruchstellen) -----------------------------------------------------------
    def pfeil(self, name, x, y, richtung, text=True):
        """Senkrechter Potentialpfeil bei x/y (Anschluss dort). richtung "ab": Leitung kommt von oben und geht
        über den Pfeil weg (gehend, Pfeil zeigt nach unten); "von_oben": Potential kommt von oben herein
        (kommend, Pfeil zeigt nach unten auf die Leitung). Die Verknüpfung mit dem Gegenstück gleichen Namens
        auf einem anderen Blatt macht Plan.erzeuge; QET schreibt dann „/Blatt/Spalte“ daneben."""
        # Ursprung des gedrehten Pfeils (Anschluss 13 px daneben) aufs Raster: y so verschieben, dass er passt
        # senkrechte Pfeile als eigene Symbole (nicht gedreht): QET legt den Verweistext gedrehter Pfeile
        # waagerecht mittig auf die Leitung, und eng benachbarte Pfeile überdecken sich
        pfad, dreh = {"ab": (S.GEHEND_S, 0), "von_unten": (S.KOMMEND, 270)}.get(richtung, (S.KOMMEND_S, 0))
        a = S.symbol(pfad).anschluss(0)
        dy = _dreh(a.x, a.y, dreh)[1]
        y = raster(y - dy) + dy
        if richtung == "ab":
            t = self.setze(S.GEHEND_S, x, y, anker=0)
            self.plan.quellen.append((self.folio, name, t))
            ty = y + 60 + 7 * len(name)              # unter dem senkrechten Verweis (≈ 6 Zeichen)
        elif richtung == "von_unten":                 # Einspeisung von unten (Pfeil unter der Leitung, zeigt hinauf)
            t = self.setze(S.KOMMEND, x, y, anker=0, drehung=270)
            self.plan.ziele.append((self.folio, name, t))
            ty = y + 36 + 7 * len(name)
        else:
            t = self.setze(S.KOMMEND_S, x, y, anker=0)
            self.plan.ziele.append((self.folio, name, t))
            ty = y - 74                              # über dem senkrechten Verweis (≈ 6 Zeichen)
        if text:                                   # senkrecht wie EPLAN, Leserichtung von unten
            self.text(name, x - 6, ty, "funktion", drehung=270)
        return t

    # --- Potentialschienen ---------------------------------------------------------------------------
    def schiene(self, name, y=None, von_spalte=0, bis_spalte=SPALTEN - 1, text="", anfang=None, ende=None):
        """Potentialschiene. anfang=False: das Potential entsteht auf diesem Blatt (Trafo, Netzteil) – kein
        ankommender Pfeil; ende=False: endet hier. Ohne Angabe entscheidet Plan.erzeuge nach den anderen Blättern."""
        if y is None:
            y = BAND["schiene"] + SCHIENEN_ABSTAND * len(self.schienen)
        # Enden so, dass die Verweispfeile (Anschluss 13 px vom Ursprung) auf dem Raster sitzen
        s = Schiene(name, y, spalte_links(von_spalte) + 43, spalte_links(bis_spalte + 1) - 53, text=text,
                    anfang=anfang, ende=ende)
        self.schienen[name] = s
        return s

    def abgriff(self, name, x, richtung=None):
        """T-Abzweig auf der Schiene `name` bei x. richtung "unten" (Gerät unter der Schiene) oder "oben"
        (Gerät darüber); ohne Angabe: unten. Rückgabe (Teil, Anschlussname) für die Leitung zum Gerät."""
        s = self.schienen[name]
        oben = richtung == "oben"
        pfad = S.ABZWEIG_OBEN if oben else S.ABZWEIG
        sym = S.symbol(pfad)
        w, ab = sym.anschluss("W"), sym.anschluss("N" if oben else "S")
        # Abgang senkrecht bei x, Schiene (W/E) auf Höhe s.y
        t = self.setze(pfad, x - ab.x, s.y - w.y)
        s.abgriffe.append((x, t))
        return t, ("N" if oben else "S")

    def _schienen_fertig(self, partner=None):
        """partner(name, folio) -> (kommt_von_anderswo, geht_weiter): automatische Pfeile an den Enden."""
        for s in self.schienen.values():
            von, weiter = partner(s.name, self.folio) if partner else (True, True)
            mit_anfang = s.anfang if s.anfang is not None else von
            mit_ende = s.ende if s.ende is not None else weiter
            abz = [t for _, t in sorted(s.abgriffe, key=lambda a: a[0])]
            if not abz and not (mit_anfang and mit_ende):
                mit_anfang = mit_ende = True              # Schiene ohne Abgriff: wenigstens durchverbinden
            punkte = [(t, "W", "E") for t in abz]
            if mit_anfang:
                s.kommend = self.setze(S.KOMMEND, s.x1, s.y, anker=0)
                self.plan.ziele.append((self.folio, s.name, s.kommend))
                punkte.insert(0, (s.kommend, 0))
            if mit_ende:
                s.gehend = self.setze(S.GEHEND, s.x2, s.y, anker=0)
                self.plan.quellen.append((self.folio, s.name, s.gehend))
                punkte.append((s.gehend, 0))
            for a, b in zip(punkte, punkte[1:]):
                self.draht(a[0], a[2] if len(a) == 3 else a[1], b[0], b[1])
            # ohne Anfangspfeil beginnt die Schiene am ersten Abzweig: Name rechts daneben, nicht auf dessen Leitung
            x_links = s.x1 if mit_anfang else (min(x for x, _ in s.abgriffe) + 6 if s.abgriffe else s.x1)
            self.text(s.name, x_links + 8, s.y - 22, "funktion")
            if mit_ende:
                # rechts: Name über der Schiene vor dem Pfeil (am Pfeil steht QETs Folgeverweis „/31/0“)
                self.text(s.name, s.x2 - 9 * len(s.name) - 14, s.y - 22, "funktion")
            if s.text:
                self.text(s.text, s.x1 - 52, s.y - 20, "verweis")

    def _funktionen_fertig(self):
        g = GROESSE["funktion"]
        for von, bis, zeilen in self.funktionen:
            x1, x2 = spalte_links(von), spalte_links(bis + 1)
            mitte = (x1 + x2) / 2
            # liegen SPS-Kanalkästen in den Spalten, steht der Text mittig unter ihnen (nicht unter der Spalte)
            k = [x for x in self.kaesten if x1 <= x < x2]
            if k:
                mitte = (min(k) + max(k)) / 2
            for i, z in enumerate(zeilen):
                self.text(z, mitte - TEXT_HALB * g * len(z), BAND["text"] + 14 * i, "funktion")


class Plan:
    def __init__(self, titel, ordner, schriftfeld, autor="Birger Siemering"):
        self.titel, self.ordner, self.schriftfeld, self.autor = titel, ordner, schriftfeld, autor
        self.blaetter, self.schrift, self.links = [], {}, []
        self.quellen, self.ziele = [], []          # (folio, Potentialname, Teil): gehende / kommende Pfeile
        self.lagen = []                           # Kennzeichen-Lagen, nach den Verknüpfungen gesetzt

    def blatt(self, titel, anlage, ort, nummer):
        b = Blatt(self, len(self.blaetter), titel, anlage, ort, nummer)
        self.blaetter.append(b)
        return b

    def _verweise(self):
        """Gehende Pfeile (Schienenende rechts, Pfeil „ab“) mit dem nächsten kommenden Pfeil gleichen Namens auf
        einem späteren Blatt verknüpfen (sonst auf einem früheren); jedes Ziel nur einmal. Dazu die
        Master/Slave-Verknüpfungen der Bausteine. Offene Enden: self.offen (zum Prüfen)."""
        ops, frei, self.offen = [], list(self.ziele), []
        for f, name, q in sorted(self.quellen, key=lambda a: a[0]):
            kand = [z for z in frei if z[1] == name and z[0] != f]
            spaeter = [z for z in kand if z[0] > f]
            wahl = min(spaeter, key=lambda z: z[0]) if spaeter else (min(kand, key=lambda z: -z[0]) if kand else None)
            if wahl is None:
                self.offen.append((f, name))
                continue
            frei.remove(wahl)
            ops.append({"op": "link_elements", "folio": f, "element": q.ref, "to_folio": wahl[0], "to": wahl[2].ref})
        for f, slave, mf, master in self.links:
            ops.append({"op": "link_elements", "folio": f, "element": slave.ref, "to_folio": mf, "to": master.ref})
        return ops

    def erzeuge(self, name, pdf=True):
        os.environ.setdefault("QET_SCRIPT_TIMEOUT_MS", "900000")
        os.environ.setdefault("QET_MCP_COLUMNS_0", "1")
        from qet_direkt import q, BIN
        ops = [{"op": "embed_title_block_template", "name": self.schriftfeld}]
        # wer führt welches Potential? Schienen und Pfeile aller Blätter (für die Pfeile an den Schienenenden)
        schienen = {}
        for b in self.blaetter:
            for s in b.schienen.values():
                schienen.setdefault(s.name, []).append((b.folio, s))
        pf_quellen = {(f, n) for f, n, _ in self.quellen}
        pf_ziele = {(f, n) for f, n, _ in self.ziele}

        def partner(name, folio):
            andere = [(f, s) for f, s in schienen.get(name, []) if f != folio]
            von = any(s.ende is not False for _, s in andere) or any(n == name and f != folio for f, n in pf_quellen)
            weiter = any(f > folio and s.anfang is not False for f, s in andere) or                 any(n == name and f != folio for f, n in pf_ziele)
            return von, weiter
        for b in self.blaetter:
            b._schienen_fertig(partner)
            b._funktionen_fertig()
            ops += b.ops
        ops += self._verweise()
        ops += self.lagen
        leer = os.path.join(self.ordner, "_leer_vorlagen.qet")
        out = os.path.join(self.ordner, name)
        # Neuanlage höchstens 200 Blätter; weitere vorn als add_folio anhängen (große EPLAN-Importe)
        q.tool_project_new(BIN, leer, title=self.titel, folios=min(200, len(self.blaetter)), author=self.autor,
                           overwrite=True)
        if len(self.blaetter) > 200:
            ops = ops[:1] + [{"op": "add_folio"}] * (len(self.blaetter) - 200) + ops[1:]
        r = q.tool_edit(BIN, leer, ops, out, timeout=3000)
        fehl = [(o["index"], o["op"]) for o in r.get("operations", []) if not o.get("succeeded")]
        self._schrift(out)
        print("ok:", r.get("ok"), "ops:", len(ops), "fehl:", fehl[:8], r.get("hint") or "")
        if fehl:
            print("erste fehlgeschlagene op:", ops[fehl[0][0]])
        if pdf:
            print("pdf:", q.tool_export(BIN, out, "pdf", out.replace(".qet", ".pdf")).get("ok"))
        return out, r

    def _schrift(self, path):
        """Schriftgrößen der freien Texte (die Skript-API setzt keine)."""
        s = open(path, encoding="utf-8").read()

        teile = re.split(r'(<diagram [^>]*>)', s)        # Blattweise: Index des Blatts = Zahl der Köpfe davor
        folio = -1
        for k, teil in enumerate(teile):
            if teil.startswith("<diagram "):
                folio += 1
                continue
            if folio < 0:
                continue

            def inp(m, f=folio):
                tag = m.group(0)
                t = re.search(r'text-indent:0px;&quot;(?:>|&gt;)(.*?)&lt;/p', tag)
                text = html.unescape(html.unescape(t.group(1))) if t else ""
                size = self.schrift.get((f, text), GROESSE["daten"])
                tag = re.sub(r'font="([^",]+),\d+,', lambda m_: f'font="{m_.group(1)},{size},', tag)
                return re.sub(r'font-size:\d+pt', f'font-size:{size}pt', tag)
            teile[k] = re.sub(r'<input [^<]*', inp, teil)
        s = "".join(teile)
        s = re.sub(r'(master_label|slave_label)="[^"]*"', lambda m: m.group(1) + '="/%F/%c"', s)
        s = re.sub(r'<report label="[^"]*"', '<report label="/%F/%c"', s)
        open(path, "w", encoding="utf-8").write(s)
