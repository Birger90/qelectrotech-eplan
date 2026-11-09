"""EPLAN-Kontaktspiegel (DMContactImage) als Seitengrafik – nachgebildet nach dem dekompilierten EPLAN-2025-Code
(EEServicesLogu.erx, XEsContactImageInterface.cpp; Belege in FORMATBESCHREIBUNG_EOD.md Abschnitt 19).

Belegte Bausteine (Funktionsadressen EEServicesLogu.erx):
- Objekt: Klasse 100 als Platzierung (ea65) der Funktion, sonst der Symbolvariante. Felder (Metamodell-Typen,
  je Typ genau ein Member des DMContactImage-Konstruktors ETrDMProjectu 0x18060c720): f169 long = Anzeigemaske
  (+0x100), f16a EPoint = eigene Lage (+0xf0), f16b bool = automatisch ausrichten (+0x104, Vorgabe 1),
  f16c Text = Formular (+0x120).
- Maske (0x180303520): Bit 1 gesetzt -> Maske des Objekts; sonst Projekteinstellung
  EsCrossReference.ContactImage.ContactorDisplayBitMask bzw. bei Bit 0x10 MotorSwitchDisplayBitMask | 0x10.
- Lage (0x180302730 + DMContactImage::computeAutoAlignLocation): f16b = 0 -> f16a relativ zur Funktion; sonst
  „im Pfad“ (ohne Bit 0x10): (x + 12061, 12060 absolut), „am Schaltzeichen“ (Bit 0x10): (x + 12059, y + 12061);
  Werte aus Seite, sonst Normblatt, sonst 70 bzw. 24.
- Einträge (0x180305080/0x180305880/0x180301ab0/0x180301bf0/0x1803061b0/0x180302930/0x180302d50) siehe zeichnen().
Text-Hülle: Breite = Tahoma-Laufweiten × Geviert (1,243 × Texthöhe), Höhe = Zellhöhe 1,5 × Texthöhe
(ETextGraphicsTools::setFont) – abgeleitet, die Hüllberechnung von DmgText selbst ist nicht dekompiliert."""
import math

import eod_symbole as E
import eplan_grafik as G

EBENE_QVW, EBENE_KOPF = 440, 480                     # 0x1b8 Querverweis, 0x1e0/0x1e1 Artikelnummer/-typ


def objekt(pr_fkt, pr_var):
    """Kontaktspiegel-Objekt (Klasse 100): an der Funktion, sonst an der Symbolvariante -> Eigenschaftsliste."""
    for pr in (pr_fkt, pr_var):
        for q in pr or []:
            if q[0] == 0xea65 and q[1] == "obj" and q[2][0] == 100:
                return q[2][1]
    return None


def symbol_fuer(gr, lib, fd, c6, c7):
    """Symbol einer nicht platzierten Funktion (0x1803011c0): ohne eigenes Symbol sucht EPLAN über
    DMSymbolLibrary::getSymbol(Funktionsdefinition) zuerst in der Bibliothek der Hauptfunktion, dann in den anderen.
    Welcher von mehreren passenden Symbolen kommt, ist NICHT belegt (getSymbol nicht dekompiliert); genommen wird
    das mit der kleinsten Symbolnummer (eb52)."""
    for l_ in [lib] + [x for x in gr.libs if x != lib]:
        kand = [(E.wert(p, 0xeb52) or 0, sid) for sid, (c, p) in gr.sym.get(l_, {}).items()
                if p and (E.wert(p, 0xefc5), E.wert(p, 0xefc6), E.wert(p, 0xefc7)) == (fd, c6, c7)]
        if kand:
            return l_, min(kand)[1]
    return None


def maske(o, einst):
    m = E.wert(o, 0xf169) or 0
    if m & 1:
        return m
    try:
        if m & 0x10:
            return int(einst.get("EsCrossReference.ContactImage.MotorSwitchDisplayBitMask") or 0) | 0x10
        return int(einst.get("EsCrossReference.ContactImage.ContactorDisplayBitMask") or 0)
    except ValueError:
        return 0


def automatisch(o):
    """f16b (bool, +0x104): fehlt = 1 (Konstruktor); der Parser liest das gespeicherte 0 als leeren Text."""
    v = [q[2] for q in o if q[0] == 0xf16b]
    return True if not v else v[0] not in ("", 0, False, None)


def _zahl(kv, k):
    try:
        return float(str(kv.get(k) or "0").replace(",", "."))
    except ValueError:
        return 0.0


def ursprung(o, m, fpos, kv_seite, kv_nb):
    """Einfügepunkt des Spiegels (EPLAN-mm absolut)."""
    if not automatisch(o):
        d = E.wert(o, 0xf16a) or (0.0, 0.0)
        return fpos[0] + d[0], fpos[1] + d[1]
    im_pfad = not (m & 0x10)
    eig = 12060 if im_pfad else 12059

    def wert(k):
        return _zahl(kv_seite, k) or _zahl(kv_nb, k)
    d = wert(eig) or (70.0 if im_pfad else 24.0)
    off = wert(12061)
    if im_pfad:
        return fpos[0] + off, d
    return fpos[0] + d, fpos[1] + off


# --- Hüllen -----------------------------------------------------------------------------------------------
def _leer():
    return [math.inf, math.inf, -math.inf, -math.inf]


def _gueltig(b):
    return b[0] <= b[2] and b[1] <= b[3]


def _vereinige(b, c):
    if _gueltig(c):
        b[0], b[1], b[2], b[3] = min(b[0], c[0]), min(b[1], c[1]), max(b[2], c[2]), max(b[3], c[3])
    return b


def _schieb(b, dx, dy):
    return [b[0] + dx, b[1] + dy, b[2] + dx, b[3] + dy]


def text_huelle(t, h, ausr, ang=0.0, anker=(0.0, 0.0)):
    w = G.breite_px(t or " ", 1.0) * h * G.TAHOMA_EM
    c = G.ZELLE * h
    sp, ze = ((ausr or 7) - 1) % 3, ((ausr or 7) - 1) // 3
    x0 = -w * (0, 0.5, 1)[sp]
    y1 = (0.0, c / 2, c)[ze] if ze < 3 else c
    pts = [(x0, y1 - c), (x0 + w, y1 - c), (x0, y1), (x0 + w, y1)]
    cs, sn = math.cos(ang), math.sin(ang)
    pts = [(anker[0] + x * cs - y * sn, anker[1] + x * sn + y * cs) for x, y in pts]
    return [min(p[0] for p in pts), min(p[1] for p in pts), max(p[0] for p in pts), max(p[1] for p in pts)]


def form_huelle(formen):
    b = _leer()
    for f in formen:
        if f[0] == "L":
            pts = [f[1], f[2]]
        elif f[0] == "P":
            pts = f[1]
        elif f[0] == "A":
            pts = G._bogen_punkte(f[1], f[2], f[3], f[4]) if abs(f[4] - f[3]) > 1e-6 else \
                [(f[1][0] - f[2], f[1][1] - f[2]), (f[1][0] + f[2], f[1][1] + f[2])]
        else:
            continue
        for x, y in pts:
            _vereinige(b, [x, y, x, y])
    return b


def _ausr_qvw(m):
    """Ausrichtung des Querverweistexts (0x1803061b0) und dVar34."""
    zeig, links = m & 0x100, m & 0x200
    if m & 0x22 == 0:
        return ((2 if links else 8) if zeig else 5), 1.0
    if m & 0x22 == 0x22:
        return ((8 if links else 2) if zeig else 5), 1.0
    return ((((1 if links else 0) | 2) * 2) if zeig else 5), 0.0


def zeichnen(gr, m, O, eintraege, kopf):
    """eintraege: [{lib, sid, namen: [Anschlussbezeichnung je Anschluss], qvw: Text}] in Spiegelreihenfolge;
    kopf: {(Eigenschaft, Index): Text} der Hauptfunktion (20100/20200). -> (Formen absolut, Anzahl ohne Symbol)."""
    senkrecht = bool(m & 2)
    klein = 0.8 if m & 0x10000 else 1.0                      # 0x180306dd0: Texte verkleinern
    varcode = 0 if m & 0x10 else 16                          # 0x180305080: Variante 16 bzw. bei Motorschutz A
    formen, ohne = [], 0
    cur = [0.0, 0.0]                                          # +0xd0 / +0xd8
    gesamt = _leer()                                          # +0xe0 (je Eintrag ohne Cursor eingetragen)
    lay = []
    for e in eintraege:
        vid = gr.varianten.get((e["lib"], e["sid"]), {}).get(varcode)
        if vid is None:
            ohne += 1
            continue
        sf = [f for f in gr.formen(e["lib"], vid) if f[0] != "T"]
        S = form_huelle(sf)
        if not _gueltig(S):
            S = [0.0, 0.0, 0.0, 0.0]
        U = list(S)
        an = gr.anschluesse(e["lib"], vid)
        texte = []
        lagen = gr.textlagen(e["lib"], vid)
        for (eig, idx), (tx, ty, h, ang, ausr, am, _) in lagen.items():
            if eig != 20028 or not (1 <= idx <= len(an)):
                continue
            ax, ay, r = an[idx - 1]
            h *= klein
            w = e["namen"][idx - 1] if idx - 1 < len(e["namen"]) else ""
            if m & 0x412 == 2:                               # Anschlussrichtung bestimmt Lage und Ausrichtung
                x, y = ax, ay
                ausr = {1: 8, 2: 4, 4: 2, 8: 6}.get(r, ausr)
                x += {2: 1.0, 8: -1.0}.get(r, 0.0)
            else:
                x, y = (ax + tx, ay + ty) if am else (tx, ty)
            _vereinige(U, text_huelle(w, h, ausr, 0.0, (x, y)))
            if w.strip():
                texte.append(("T", (x, y), w, h, 0.0, ausr))
        if senkrecht:
            ox, oy = -(S[0] + S[2]) / 2, -U[3]
            _vereinige(gesamt, _schieb(U, ox, oy))
            oy += cur[1]
            q = [-1.0 if not (m & 0x200) else 1.0, cur[1] - (U[3] - U[1]) / 2]
            cur[1] -= U[3] - U[1]
        else:
            ox, oy = -U[0], -(S[1] + S[3]) / 2
            _vereinige(gesamt, _schieb(U, ox, oy))
            ox += cur[0]
            q = [(U[2] - U[0]) / 2 + cur[0], -1.0 if not (m & 0x200) else 1.0]
            cur[0] += U[2] - U[0] + 2.0
        # Anschluss mit Richtung 2, unterster unter 0 (0x180301bf0)
        ty0 = min([0.0] + [a[1] for a in an if a[2] == 2 and a[1] < 0])
        lay.append({"e": e, "sf": sf, "texte": texte, "o": [ox, oy], "q": q, "U": U, "ty": ty0})
    # Querverweislage (0x180301bf0)
    for L in lay:
        if senkrecht:
            L["q"] = [gesamt[2] + 1.0 if not (m & 0x200) else gesamt[0] - 1.0, L["o"][1] + L["ty"]]
            L["box"] = [gesamt[0], L["U"][1], gesamt[2], L["U"][3]]
        else:
            L["q"][1] = gesamt[3] + 1.0 if not (m & 0x200) else gesamt[1] - 1.0
            L["box"] = [L["U"][0], gesamt[1], L["U"][2], gesamt[3]]
    # Querverweistexte (0x1803061b0)
    ausr_q, d34 = _ausr_qvw(m)
    ang_q = math.pi / 2 if m & 0x20 else 0.0
    h_q = G.EBENEN.get(EBENE_QVW, (2.5, 7))[0] * klein
    acc = 0.0
    if not (m & 0x100):
        k_ = 1 if senkrecht else 0
        acc, cur[k_] = cur[k_], 0.0
    for L in lay:
        t = L["e"].get("qvw") or ""
        if not t:
            continue
        T = text_huelle(t, h_q, ausr_q, ang_q)
        B = L["box"]
        s = 0.0
        if senkrecht:
            d = T[3] - T[1] - ((B[3] - B[1]) if _gueltig(B) else 0.0)
            d *= -0.5
            s = d if d <= 0 else (0.0 if d34 <= d else d - d34)
            L["q"][1] += s + acc
            L["o"][1] += s + acc
            if abs(s) >= 1e-10:
                acc += 2 * s - d34
        else:
            d = T[2] - T[0] - ((B[2] - B[0]) if _gueltig(B) else 0.0)
            d *= -0.5
            s = d if d <= 0 else (0.0 if d34 <= d else d - d34)
            L["q"][0] += -s + acc
            L["o"][0] += -s + acc
            if abs(s) >= 1e-10:
                acc -= 2 * s - d34
        L["qtext"] = ("T", tuple(L["q"]), t, h_q, ang_q, ausr_q)
    if senkrecht:
        cur[1] += acc
    else:
        cur[0] += acc
    for L in lay:
        dx, dy = O[0] + L["o"][0], O[1] + L["o"][1]
        formen += [G_verschoben(f, (dx, dy)) for f in L["sf"]]
        formen += [G_verschoben(f, (dx, dy)) for f in L["texte"]]
        if "qtext" in L:
            formen.append(G_verschoben(L["qtext"], O))
    # Kopf und Fuß (0x180302930 / 0x180302d50): Artikelnummer 20100, Artikeltyp 20200
    for fuss, folge in ((False, ((0x8, 20200, 2), (0x400000, 20100, 2), (0x100000, 20200, 1), (0x4, 20100, 1))),
                        (True, ((0x80000, 20100, 1), (0x200000, 20200, 1), (0x800000, 20100, 2),
                                (0x1000000, 20200, 2)))):
        pos = [0.0, cur[1]] if (fuss and senkrecht) else ([cur[0], 0.0] if fuss else [0.0, 0.0])
        for bit, eig, idx in folge:
            t = kopf.get((eig, idx)) or ""
            if not (m & bit) or not t:
                continue
            h = G.EBENEN.get(EBENE_KOPF if eig == 20100 else EBENE_KOPF + 1, (2.5, 7))[0]
            T = text_huelle(t, h, 7)
            if senkrecht:
                pos[1] += -(T[3] - T[1]) if fuss else (T[3] - T[1])
                a = 8 if fuss else 2
            else:
                pos[0] += (T[2] - T[0] + 1.0) if fuss else -(T[2] - T[0] + 1.0)
                a = 6 if fuss else 4
            formen.append(("T", (O[0] + pos[0], O[1] + pos[1]), t, h, 0.0, a))
    return formen, ohne


def G_verschoben(f, d):
    def v(q):
        return (q[0] + d[0], q[1] + d[1])
    if f[0] == "L":
        return ("L", v(f[1]), v(f[2]), f[3])
    if f[0] == "A":
        return ("A", v(f[1])) + tuple(f[2:])
    if f[0] == "P":
        return ("P", [v(q) for q in f[1]]) + tuple(f[2:])
    return ("T", v(f[1])) + tuple(f[2:])
