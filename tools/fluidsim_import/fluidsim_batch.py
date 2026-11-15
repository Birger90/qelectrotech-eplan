"""Alle FluidSIM-Beispielschaltungen übernehmen und simulieren (Prüfung des Imports und der Simulation).

Je Schaltung: Import mit fluidsim_plan.py in ein eigenes Projekt (mit --bericht), dann `QElectroTech --simulate`
über einige Sekunden. Nacheinander wird jeder Taster bzw. Schalter einmal gedrückt und jedes handbetätigte
Ventil einmal links betätigt; gemessen wird, ob sich danach ein Zylinder bewegt und wie oft Ventile schalten.

    python fluidsim_batch.py --ausgabe ORDNER [--circ ".../FluidSIM6/circ"] [--parallel 6] [--nur pneu/TP101]

Ergebnis: ORDNER/ergebnis.csv (je Schaltung) und ORDNER/zusammenfassung.json.
"""
import argparse
import csv
import json
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

HIER = Path(__file__).resolve().parent
REPO = HIER.parents[1]
QET = next((p for p in (HIER.parent / "QElectroTech.exe", REPO / "build" / "deploy" / "QElectroTech.exe") if p.exists()),
           REPO / "build" / "deploy" / "QElectroTech.exe")   # neben der Installation oder im Quellbaum
FELDER = ["datei", "import", "fehlops", "bauteile", "leitungen", "grafcet", "modellfehler", "programmfehler",
          "zylinder", "bewegt", "schaltungen", "meldung"]


def eine(circ: Path, ziel: Path, dauer: float):
    zeile = dict.fromkeys(FELDER, "")
    zeile["datei"] = circ.as_posix()
    stamm = re.sub(r"[^\w\-]+", "_", circ.relative_to(circ.parents[1]).with_suffix("").as_posix())
    qet = ziel / (stamm + ".qet")
    bericht = ziel / (stamm + ".json")
    env = dict(os.environ, PYTHONIOENCODING="utf-8")
    try:
        r = subprocess.run([sys.executable, str(HIER / "fluidsim_plan.py"), str(circ), str(qet), "--qet", str(QET),
                            "--bericht", str(bericht)],
                           capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=600, env=env)
    except subprocess.TimeoutExpired:
        zeile.update({"import": "Fehler", "meldung": "Import: Zeitüberschreitung"})
        return zeile
    if r.returncode != 0 or not qet.exists() or not bericht.exists():
        letzte = (r.stderr or r.stdout).strip().splitlines()
        zeile.update({"import": "Fehler", "meldung": (letzte[-1] if letzte else "")[:300]})
        return zeile
    b = json.loads(bericht.read_text(encoding="utf-8"))
    zeile.update({"import": "ok", "bauteile": b["bauteile"], "leitungen": b["leitungen"], "fehlops": b["fehlops"],
                  "grafcet": b["schritte"]})
    ereignisse, t = [], 0.3
    for lbl in b["taster"]:
        ereignisse += ["--event", f"{t:.2f} {lbl} an", "--event", f"{t + 0.3:.2f} {lbl} aus"]
        t += 1.0
    # FluidSIM lässt jedes Ventil per Klick betätigen, auch Magnetventile ohne Spule in der Schaltung
    for lbl in b.get("ventile", b["handventile"]):
        for seite in ("links", "rechts"):
            ereignisse += ["--event", f"{t:.2f} {lbl} {seite} an", "--event", f"{t + 0.6:.2f} {lbl} {seite} aus"]
            t += 1.2
    alle = b["taster"] + b["handventile"]
    if len(alle) > 1:
        # zum Schluss alles zugleich (Zweihandbedienung, UND-Verknüpfungen)
        for lbl in b["taster"]:
            ereignisse += ["--event", f"{t:.2f} {lbl} an", "--event", f"{t + 1.5:.2f} {lbl} aus"]
        for lbl in b["handventile"]:
            ereignisse += ["--event", f"{t:.2f} {lbl} links an", "--event", f"{t + 1.5:.2f} {lbl} links aus"]
        t += 2.0
    csv_pfad = qet.with_suffix(".csv")
    try:
        s = subprocess.run([str(QET), "--simulate", str(qet), "--duration", f"{max(dauer, t + 1.0):.1f}",
                            "--csv", str(csv_pfad)] + ereignisse,
                           capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=900)
    except subprocess.TimeoutExpired:
        zeile["meldung"] = "Simulation: Zeitüberschreitung"
        return zeile
    fehler = [l[len("Model error: "):] for l in s.stderr.splitlines() if l.startswith("Model error: ")]
    prog = [l[len("PLC program: "):] for l in s.stderr.splitlines() if l.startswith("PLC program: ")]
    zeile["modellfehler"] = len(fehler)
    zeile["programmfehler"] = len(prog)
    zeile["schaltungen"] = len(re.findall(r"schaltet in Stellung", s.stdout))
    meldung = fehler + prog + [l for l in s.stderr.splitlines() if "not understood" in l or "no valve" in l]
    if s.returncode not in (0, 3):
        meldung.append(f"Exit {s.returncode}")
    zeile["meldung"] = "; ".join(meldung[:3])[:400]
    if csv_pfad.exists():
        zeilen = list(csv.reader(csv_pfad.open(encoding="utf-8"), delimiter=";"))
        if zeilen:
            spalten = [i for i, k in enumerate(zeilen[0]) if k.endswith("x [mm]")]
            zeile["zylinder"] = len(spalten)
            bewegt = False
            for i in spalten:
                werte = [float(z[i]) for z in zeilen[1:] if len(z) > i]
                bewegt = bewegt or bool(werte and max(werte) - min(werte) > 1.0)
            zeile["bewegt"] = "ja" if bewegt else "nein"
        csv_pfad.unlink()
    return zeile


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--circ", type=Path, default=Path("C:/Program Files (x86)/Didactic/FluidSIM6/circ"))
    ap.add_argument("--ausgabe", type=Path, required=True)
    ap.add_argument("--parallel", type=int, default=6)
    ap.add_argument("--dauer", type=float, default=3.0)
    ap.add_argument("--nur", default="", help="nur Schaltungen, deren Pfad dies enthält")
    a = ap.parse_args()
    a.ausgabe.mkdir(parents=True, exist_ok=True)
    circs = [c for c in sorted(a.circ.rglob("*.circ")) if a.nur.replace("\\", "/") in c.as_posix()]
    ergebnis = []
    with ThreadPoolExecutor(a.parallel) as pool:
        auftraege = {pool.submit(eine, c, a.ausgabe, a.dauer): c for c in circs}
        for i, f in enumerate(as_completed(auftraege), 1):
            try:
                z = f.result()
            except Exception as e:      # ein Fehler im Prüfwerkzeug soll die anderen nicht aufhalten
                z = dict.fromkeys(FELDER, "")
                z.update({"datei": auftraege[f].as_posix(), "import": "Fehler", "meldung": f"Prüfwerkzeug: {e}"})
            ergebnis.append(z)
            if i % 25 == 0:
                print(f"{i}/{len(circs)}", flush=True)
    ergebnis.sort(key=lambda z: z["datei"])
    with (a.ausgabe / "ergebnis.csv").open("w", encoding="utf-8-sig", newline="") as f:
        w = csv.DictWriter(f, FELDER, delimiter=";")
        w.writeheader()
        w.writerows(ergebnis)
    ok = [z for z in ergebnis if z["import"] == "ok"]
    mit_zyl = [z for z in ok if str(z["zylinder"]) not in ("", "0")]
    zusammen = {
        "schaltungen": len(ergebnis), "import_ok": len(ok),
        "ohne_fehlgeschlagene_operationen": sum(1 for z in ok if str(z["fehlops"]) == "0"),
        "ohne_modellfehler": sum(1 for z in ok if str(z["modellfehler"]) == "0"),
        "mit_grafcet": sum(1 for z in ok if str(z["grafcet"]) not in ("", "0")),
        "ohne_programmfehler": sum(1 for z in ok if str(z["programmfehler"]) == "0"),
        "mit_zylinder": len(mit_zyl), "zylinder_bewegt": sum(1 for z in mit_zyl if z["bewegt"] == "ja"),
    }
    (a.ausgabe / "zusammenfassung.json").write_text(json.dumps(zusammen, indent=1, ensure_ascii=False), encoding="utf-8")
    print(json.dumps(zusammen, indent=1, ensure_ascii=False))


if __name__ == "__main__":
    main()
