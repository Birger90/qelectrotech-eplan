"""EPLAN-Projektarchiv (.zw1) auslesen: entpacken, Eigenschaftsströme dekodieren, Auswertungen als CSV/JSON.

    python zw1_import.py "<archiv>.zw1" <arbeitsordner>

Ergebnis in <arbeitsordner>\\out: Verbindungen_aufgeloest.csv, Funktionen_dekodiert.csv, Seiten_dekodiert.csv,
Netze.csv, Stueckliste_je_BMK.csv, Stueckliste_summiert.csv, Klemmenplan.csv, Kabeluebersicht.csv, Kabelplan.csv
und Zusammenfassungen (JSON). Der Dekoder (eodlib, parseall, export*) stammt aus der Formatanalyse in
FORMATBESCHREIBUNG_EOD.md (13 Projekte, EPLAN 2.7–2026); hier nur Windows-Aufruf und feste Pfade.
Das Archiv wird nur gelesen. Entpackt wird nach <arbeitsordner>\\x."""
import os
import subprocess
import sys

HIER = os.path.dirname(os.path.abspath(__file__))
SIEBENZIP = os.environ.get("SIEBENZIP", r"C:\Program Files\7-Zip\7z.exe")
SCHRITTE = ["parseall.py", "ins130.py", "export.py", "export2.py", "export3.py"]


def importieren(archiv, arbeit, neu_entpacken=False):
    arbeit = os.path.abspath(arbeit)
    x = os.path.join(arbeit, "x")
    if neu_entpacken or not os.path.exists(os.path.join(x, "Function.eod")):
        os.makedirs(x, exist_ok=True)
        subprocess.run([SIEBENZIP, "x", "-y", f"-o{x}", archiv], check=True, stdout=subprocess.DEVNULL)
    env = dict(os.environ, EOD_BASE=x, EOD_AUTO="1", EOD_OUT=os.path.join(arbeit, "parsed.pkl"),
               EOD_INS=os.path.join(arbeit, "ins130.pkl"), EOD_EXPORT=os.path.join(arbeit, "out"),
               PYTHONIOENCODING="utf-8")
    protokoll = []
    for s in SCHRITTE:
        r = subprocess.run([sys.executable, "-I", os.path.join(HIER, s)], cwd=HIER, env=env,
                           capture_output=True, text=True, encoding="utf-8", errors="replace")
        protokoll.append(f"== {s} (Rückgabe {r.returncode})\n{r.stdout[-3000:]}{r.stderr[-3000:]}")
        if r.returncode:
            raise RuntimeError("\n".join(protokoll))
    open(os.path.join(arbeit, "import_protokoll.txt"), "w", encoding="utf-8").write("\n".join(protokoll))
    return os.path.join(arbeit, "out")


if __name__ == "__main__":
    print(importieren(sys.argv[1], sys.argv[2], "--neu" in sys.argv))
