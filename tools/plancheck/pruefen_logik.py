#!/usr/bin/env python3
"""Tests of the checks of "Check plan" that read the plan as a circuit.

Builds each small test project with tools/plancheck/testprojekte.js (script
API, --run) on an empty project, runs --check-project on it and compares the
findings and the verdict of the report with what the case must give.

    python tools/plancheck/pruefen_logik.py --qet build/qelectrotech.exe [--keep DIR]

Exit code 0 when every case gives what it must, 1 otherwise.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent

EMPTY = """<?xml version="1.0" encoding="UTF-8"?>
<project title="Pruefen" version="0.200.1">
    <properties/>
    <diagram title="Blatt 1" order="1" cols="17" colsize="60" rows="8" rowsize="80" height="660" version="0.200.1" displaycols="true" displayrows="true">
        <defaultconductor type="multi"/>
        <elements/>
        <conductors/>
    </diagram>
</project>
"""

# case: (rules that must be there as (rule, severity), rules that must not be there, verdict)
CASES = {
    "wende_richtig": ([], ["interlock_missing", "interlock_one_way", "interlock_buttons_only",
                           "self_hold_no_off", "circuit_unstable"], None),
    "wende_ohne": ([("interlock_missing", "error")], ["interlock_one_way"], "correction"),
    "wende_einseitig": ([("interlock_one_way", "warning")], ["interlock_missing"], None),
    "stern_dreieck_richtig": ([], ["interlock_missing", "interlock_one_way", "interlock_buttons_only",
                                   "self_hold_no_off", "circuit_unstable"], None),
    "stern_dreieck_falsch": ([("interlock_missing", "error")], [], "correction"),
    "reihe": ([], ["interlock_missing", "interlock_one_way", "interlock_buttons_only"], None),
    "selbsthaltung_ohne_aus": ([("self_hold_no_off", "error")], [], "correction"),
    "pumpen": ([("circuit_unstable", "warning")], ["self_hold_no_off"], None),
    "kabel_zu_klein": ([("cable_capacity", "warning")], [], None),
    "kabel_richtig": ([], ["cable_capacity"], None),
    "ohne_angaben": ([("cable_capacity", "info"), ("feed_data_missing", "info")], [], "incomplete"),
    "abschaltung": ([("short_circuit", "error"), ("loop_impedance", "error")], [], "correction"),
    "fi": ([("rcd_in_series", "warning"), ("socket_without_rcd", "warning")], [], None),
}

LOGIC_RULES = {"interlock_missing", "interlock_one_way", "interlock_buttons_only", "self_hold_no_off",
               "circuit_unstable", "control_data_missing", "cable_capacity", "short_circuit",
               "loop_impedance", "rcd_in_series", "socket_without_rcd", "feed_data_missing"}


def run(qet, args, env, timeout=120):
    return subprocess.run([str(qet), *args], env=env, capture_output=True, text=True,
                          encoding="utf-8", errors="replace", timeout=timeout)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--qet", required=True, help="qelectrotech executable of this branch")
    ap.add_argument("--keep", help="folder to keep the test projects and reports in")
    ap.add_argument("--case", action="append", help="only these cases")
    a = ap.parse_args()

    qet = Path(a.qet).resolve()
    env = dict(os.environ, QET_ENABLE_SCRIPTING="1", QT_QPA_PLATFORM="offscreen")
    work = Path(a.keep) if a.keep else Path(tempfile.mkdtemp(prefix="pruefen-logik-"))
    work.mkdir(parents=True, exist_ok=True)
    script = (HERE / "testprojekte.js").read_text(encoding="utf-8")
    chosen = [c for c in CASES if not a.case or c in a.case]

    failed = 0
    for case in chosen:
        must, must_not, verdict = CASES[case]
        empty = work / f"{case}_leer.qet"
        empty.write_text(EMPTY, encoding="utf-8")
        out = work / f"{case}.qet"
        js = work / f"{case}.js"
        js.write_text(f"var CASE = {json.dumps(case)};\nvar OUT = {json.dumps(str(out))};\n" + script,
                      encoding="utf-8")
        p = run(qet, ["--run", str(js), str(empty)], env)
        if p.returncode != 0 or not out.exists():
            print(f"FEHLER {case}: Testprojekt nicht gebaut (exit {p.returncode})\n{p.stdout[-1500:]}{p.stderr[-1500:]}")
            failed += 1
            continue
        report = work / f"{case}.json"
        p = run(qet, ["--check-project", str(out), str(report)], env)
        if not report.exists():
            print(f"FEHLER {case}: kein Prüfbericht (exit {p.returncode})\n{p.stderr[-2000:]}")
            failed += 1
            continue
        data = json.loads(report.read_text(encoding="utf-8"))
        found = [(f["check"], f["severity"]) for f in data.get("findings", [])]
        problems = []
        for rule, severity in must:
            if (rule, severity) not in found:
                problems.append(f"fehlt: {rule} ({severity})")
        for rule in must_not:
            if any(r == rule for r, _ in found):
                problems.append(f"zu viel: {rule}")
        if verdict and data.get("status") != verdict:
            problems.append(f"Status {data.get('status')} statt {verdict}")
        if data.get("status") == "no_finding" and data.get("summary", {}).get("incomplete"):
            problems.append("Status kein Befund trotz fehlender Angaben")
        print(("ok     " if not problems else "FEHLER ") + f"{case}  [{data.get('statusText')}]")
        for line in problems:
            print("       " + line)
        for f in data.get("findings", []):
            if f["check"] in LOGIC_RULES:
                print(f"         · {f['severity']}/{f['check']}: {f['text']}")
        failed += bool(problems)

    if not a.keep:
        shutil.rmtree(work, ignore_errors=True)
    print(f"\n{len(chosen) - failed} von {len(chosen)} Fällen richtig")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
