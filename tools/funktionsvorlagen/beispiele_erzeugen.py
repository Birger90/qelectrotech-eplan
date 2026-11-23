"""Erzeugt die Beispiel-Funktionsvorlagen des Forks neu (funktionsvorlagen/<Kategorie>/<Name>.qetmak).

QElectroTech zeichnet jede Vorlage headless mit beispiele.js auf ein eigenes Blatt und speichert sie
mit qet.saveTemplate() -- derselbe Weg wie "Als Funktionsvorlage speichern…" im Kontextmenü.
Verwendet nur die Standardsammlung (common://) und die Fork-eigenen Symbole aus
bauteile/nachgezeichnet (company://), damit die Beispiele keine Kunden- oder Herstellerdateien tragen.

    python tools/funktionsvorlagen/beispiele_erzeugen.py [--qet build/deploy/QElectroTech.exe]
                                                       [--elements <Standardsammlung>] [--ziel funktionsvorlagen]
"""
import argparse
import os
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "misc" / "qet-mcp"))
import qet_mcp  # noqa: E402


def standardsammlung() -> Path:
    for kandidat in (REPO / "elements", REPO.parent / "qelectrotech-eplan" / "elements"):
        if (kandidat / "10_electric").is_dir():
            return kandidat
    raise SystemExit("Standardsammlung nicht gefunden: --elements angeben")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--qet", default=str(REPO / "build" / "deploy" / "QElectroTech.exe"))
    ap.add_argument("--elements", default=None, help="Ordner der Standardsammlung")
    ap.add_argument("--ziel", default=str(REPO / "funktionsvorlagen"))
    args = ap.parse_args()

    elements = Path(args.elements) if args.elements else standardsammlung()
    ziel = Path(args.ziel).resolve()
    ziel.mkdir(parents=True, exist_ok=True)
    skript = (Path(__file__).with_name("beispiele.js").read_text(encoding="utf-8")
              .replace("@ZIEL@", ziel.as_posix()))
    os.environ.setdefault("QET_ENABLE_SCRIPTING", "1")
    with tempfile.TemporaryDirectory(prefix="funktionsvorlagen-") as tmp:
        projekt = Path(tmp) / "beispiele.qet"
        neu = qet_mcp.tool_project_new(args.qet, str(projekt), title="Funktionsvorlagen", folios=0,
                                       elements_dir=str(elements))
        if not neu.get("ok", True) or not projekt.is_file():
            print(neu)
            return 1
        lauf = qet_mcp._run_qet(args.qet, [str(projekt)], timeout=600, elements_dir=str(elements),
                                script=skript, tail=20000,
                                extra_env={"QET_COMPANY_ELEMENTS_DIR": str(REPO / "bauteile" / "nachgezeichnet")})
    ausgabe = (lauf.get("stdout", "") + "\n" + lauf.get("stderr", "")).splitlines()
    vorlagen = [z for z in ausgabe if "VORLAGE " in z or "FEHLER" in z or "qet." in z]
    print("\n".join(vorlagen))
    ok = lauf.get("ok") and any("FERTIG" in z for z in ausgabe) and not any("FEHLER" in z for z in ausgabe)
    if not ok:
        print("\n".join(ausgabe[-40:]))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
