"""Hochaufgelösten OpenModelica-Lauf des hydraulischen Zylinderzyklus erzeugen (Roadmap R02, Endanschlag).

Rechnet OwnHydraulics.Examples.CylinderCycle aus dem Referenzpaket (models/OwnSim.mo, models/OwnHydraulics.mo)
mit DASSL, Toleranz 1e-9, Ausgaberaster 10 µs und schreibt eine reduzierte CSV: jede Millisekunde, in den
Anschlagfenstern (20 ms ab jedem Erreichen eines Hubendes) jeden Ausgabepunkt (10 µs). Dazu Weg, Geschwindigkeit,
Kammerdrücke und Kontaktkraft. Herkunft (Modellprüfsummen, omc-Fassung) steht in einer .json daneben.

    python erzeuge_hydraulik_fein.py --paket <Referenzpaket> --omc "C:/Program Files/OpenModelica1.26.3-64bit/bin/omc.exe"
"""
import argparse
import csv
import hashlib
import json
import shutil
import subprocess
import tempfile
from pathlib import Path

HIER = Path(__file__).resolve().parent
ZIEL = HIER / "OwnHydraulics.Examples.CylinderCycle_fein.csv"
SPALTEN = ["time", "cylinder.x", "cylinder.pressureA", "cylinder.pressureB", "cylinder.v", "cylinder.contact"]


def sha(p):
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--paket", type=Path, required=True, help="Referenzpaket (Ordner mit models/)")
    ap.add_argument("--omc", default=shutil.which("omc"))
    ap.add_argument("--toleranz", type=float, default=1e-9)
    a = ap.parse_args()
    modelle = [a.paket / "models" / "OwnSim.mo", a.paket / "models" / "OwnHydraulics.mo"]
    with tempfile.TemporaryDirectory(prefix="omc-fein-") as tmp:
        for m in modelle:
            shutil.copy(m, tmp)
        filt = "|".join(s.replace(".", "\\\\.") for s in SPALTEN[1:])
        skript = "\n".join([
            'loadFile("OwnSim.mo");', 'loadFile("OwnHydraulics.mo");',
            f'simulate(OwnHydraulics.Examples.CylinderCycle, stopTime=1.8, numberOfIntervals=180000, '
            f'tolerance={a.toleranz}, method="dassl", outputFormat="csv", variableFilter="{filt}");',
            "getErrorString();"])
        (Path(tmp) / "lauf.mos").write_text(skript, encoding="utf-8")
        r = subprocess.run([a.omc, "lauf.mos"], cwd=tmp, capture_output=True, text=True, timeout=3600)
        roh = Path(tmp) / "OwnHydraulics.Examples.CylinderCycle_res.csv"
        if not roh.exists():
            raise SystemExit("omc lieferte kein Ergebnis:\n" + r.stdout[-3000:] + r.stderr[-3000:])
        zeilen = list(csv.DictReader(roh.open(encoding="utf-8")))
        # Anschlagfenster: ab jedem Erreichen eines Hubendes (0 oder 0,1 m) 20 ms voll aufgelöst
        fenster = []
        for z0, z1 in zip(zeilen, zeilen[1:]):
            x0, x1, t = float(z0["cylinder.x"]), float(z1["cylinder.x"]), float(z1["time"])
            if ((x0 > 0.0 >= x1) or (x0 < 0.1 <= x1)) and (not fenster or t > fenster[-1][0] + 0.05):
                fenster.append((t, t + 0.020))
        out, letzte_ms = [], -1
        for z in zeilen:
            t = float(z["time"])
            ms = round(t * 1000)
            voll = any(w0 - 1e-9 <= t <= w1 for w0, w1 in fenster)
            if voll or (abs(t * 1000 - ms) < 1e-6 and ms != letzte_ms):
                out.append([z[s] for s in SPALTEN])
                letzte_ms = ms
        with ZIEL.open("w", encoding="utf-8", newline="") as f:
            w = csv.writer(f)
            w.writerow(SPALTEN)
            w.writerows(out)
        ver = subprocess.run([a.omc, "--version"], capture_output=True, text=True).stdout.strip()
        (ZIEL.with_suffix(".json")).write_text(json.dumps({
            "modell": "OwnHydraulics.Examples.CylinderCycle", "solver": "dassl", "toleranz": a.toleranz,
            "ausgaberaster_s": 1e-5, "omc": ver, "modelle_sha256": {m.name: sha(m) for m in modelle},
            "zeilen_roh": len(zeilen), "zeilen_reduziert": len(out),
            "anschlagfenster_s": fenster, "csv_sha256": sha(ZIEL)}, indent=1, ensure_ascii=False), encoding="utf-8")
        print(f"{ZIEL.name}: {len(out)} Zeilen aus {len(zeilen)}, Anschläge {fenster}")


if __name__ == "__main__":
    main()
