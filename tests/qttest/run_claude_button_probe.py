#!/usr/bin/env python3
"""Link the Claude button probe using an existing Ninja app build.

The probe opens the main window offscreen with settings of its own, clicks
the Claude button and walks through the assistant's setup page. It reads the
credential store but never writes it; Check is tried with a made-up key only.
Report and screenshots stay in build/claude-button/.
"""
import argparse
import os
import pathlib
import shutil
import subprocess
import sys

import ninja_probe

parser = argparse.ArgumentParser()
parser.add_argument('--build', required=True)
parser.add_argument('--binary', required=True)
parser.add_argument('--source', required=True)
parser.add_argument('--bridge', required=True)
parser.add_argument('--python', default=shutil.which('python') or 'python',
                    help='a Python with the package "anthropic"')
parser.add_argument('--python-without-package', default='',
                    help='a Python without it, to see the install button')
args = parser.parse_args()
build = pathlib.Path(args.build).resolve()
binary = pathlib.Path(args.binary).resolve()
source = pathlib.Path(args.source).resolve()
probe_binary = ninja_probe.build_probe(build, binary, source, 'claude-button-probe.ninja')
output = build / 'claude-button'
output.mkdir(exist_ok=True)
report = output / 'results.txt'
command = [str(probe_binary), str(report), str(output), str(pathlib.Path(args.bridge).resolve()), args.python]
if args.python_without_package:
    command.append(args.python_without_package)
env = dict(os.environ, QT_QPA_PLATFORM=os.environ.get('QT_QPA_PLATFORM', 'offscreen'))
result = subprocess.run(command, timeout=ninja_probe.timeout(180), env=env)
if report.exists():
    print(report.read_text(encoding='utf-8', errors='replace'))
sys.exit(result.returncode)
