"""Inhaltlicher Abgleich eines erzeugten Plans mit einem vorhandenen Original-PDF (nicht Lage/Optik).

Verglichen werden Mengen: Gerätekennzeichen, SPS-Adressen, Kabel. Ergebnis: was im Original steht und im
Plan fehlt, und umgekehrt – als Liste zum Entscheiden, nicht zum stillen Übernehmen.

    python -m planvorlagen.abgleich plan.qet original.pdf 27-30
"""
import re
import sys
import xml.etree.ElementTree as ET

BMK = re.compile(r"(-[A-Z]{1,3}\d+(?:\.\d+)?)$")       # auch am Ende von =DA+CA-FC311
ADR = re.compile(r"^[EA]\d+\.[0-7]$")
KABEL = re.compile(r"^W\d{2,}$")                     # W1/U1 sind Motoranschlüsse


def _sortiere(werte, t):
    m = BMK.search(t)
    if m and (t.startswith("-") or t.startswith("=")):
        werte["Kennzeichen"].add(m.group(1))
    elif ADR.match(t):
        werte["SPS-Adressen"].add(t)
    elif KABEL.match(t):
        werte["Kabel"].add(t)


def _leer():
    return {"Kennzeichen": set(), "SPS-Adressen": set(), "Kabel": set()}


def aus_plan(qet):
    root = ET.parse(qet).getroot()
    werte = _leer()
    for e in root.iter("element"):
        for d in e.iter("dynamic_elmt_text"):
            _sortiere(werte, (d.findtext("text") or "").strip())
        for i in e.iter("elementInformation"):
            if i.get("name") == "label":
                _sortiere(werte, (i.text or "").strip())
    for c in root.iter("cable"):
        _sortiere(werte, c.get("designation", ""))
    for t in root.iter("input"):                 # freie Texte (z. B. Klemmleisten-Kennzeichen)
        _sortiere(werte, re.sub(r"<[^>]+>", " ", t.get("text", "")).split()[-1] if t.get("text") else "")
    return werte


def aus_pdf(pdf, seiten):
    import pymupdf
    doc = pymupdf.open(pdf)
    werte = _leer()
    for s in seiten:
        for w in doc[s - 1].get_text("words"):
            if w[1] < 800:                       # ohne Schriftfeld
                _sortiere(werte, w[4])
    return werte


def vergleiche(qet, pdf, seiten):
    p, o = aus_plan(qet), aus_pdf(pdf, seiten)
    zeilen = []
    for k in p:
        fehlt, mehr = sorted(o[k] - p[k]), sorted(p[k] - o[k])
        zeilen.append(f"{k}: {len(p[k] & o[k])} gleich"
                      + (f"; fehlt im Plan: {', '.join(fehlt)}" if fehlt else "")
                      + (f"; nur im Plan: {', '.join(mehr)}" if mehr else ""))
    return zeilen


if __name__ == "__main__":
    a, b = sys.argv[3].split("-")
    print("\n".join(vergleiche(sys.argv[1], sys.argv[2], range(int(a), int(b) + 1))))
