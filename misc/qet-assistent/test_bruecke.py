#!/usr/bin/env python3
"""Tests der Assistenten-Brücke ohne Oberfläche.

Die Brücke wird wie von QElectroTech als Unterprozess gestartet und über
JSON-Zeilen gesteuert; sie arbeitet in der Dateibetriebsart (--datei) auf
einem frisch angelegten Testprojekt.

    QET_BINARY=...\\QElectroTech.exe python test_bruecke.py

Ohne ANTHROPIC_API_KEY läuft nur die Attrappe (feste Antworten statt
Claude). Mit Schlüssel fragt zusätzlich ein Test das echte Modell, das ein
Element setzen soll.
"""

from __future__ import annotations

import json
import os
import queue
import subprocess
import sys
import tempfile
import threading
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
BRUECKE = HERE / "bruecke.py"
sys.path.insert(0, str(HERE.parent / "qet-mcp"))
import qet_mcp as qm  # noqa: E402

ELEMENT = "common://10_electric/10_allpole/310_relays_contactors_contacts/01_coils/bobine3.elmt"
BINARY = os.environ.get("QET_BINARY", "").strip()
# Das Testprojekt entsteht über ein Skript in QElectroTech ohne Oberfläche
os.environ["QET_ENABLE_SCRIPTING"] = "1"


def _binary() -> str:
    if not BINARY or not Path(BINARY).is_file():
        raise unittest.SkipTest("QET_BINARY nicht gesetzt")
    return BINARY


class Bruecke:
    """Die Brücke als Unterprozess, wie AssistantBridge sie startet."""

    def __init__(self, *args, env=None):
        full_env = dict(os.environ, PYTHONIOENCODING="utf-8", PYTHONUTF8="1")
        full_env.update(env or {})
        self.proc = subprocess.Popen(
            [sys.executable, "-u", str(BRUECKE), *args],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            env=full_env, text=True, encoding="utf-8")
        self.lines: "queue.Queue[dict]" = queue.Queue()
        self.stderr: list = []
        threading.Thread(target=self._read, daemon=True).start()
        threading.Thread(target=self._read_err, daemon=True).start()

    def _read(self):
        for line in self.proc.stdout:
            if line.strip():
                self.lines.put(json.loads(line))

    def _read_err(self):
        for line in self.proc.stderr:
            self.stderr.append(line)

    def send(self, **msg):
        self.proc.stdin.write(json.dumps(msg) + "\n")
        self.proc.stdin.flush()

    def until(self, kind: str, timeout: float = 300, answer_confirm=None) -> list:
        """Nachrichten bis einschließlich der ersten vom Typ kind."""
        seen = []
        while True:
            msg = self.lines.get(timeout=timeout)
            seen.append(msg)
            if msg["type"] == "confirm" and answer_confirm is not None:
                self.send(type="confirm_answer", id=msg["id"], allow=answer_confirm)
            if msg["type"] == kind:
                return seen

    def close(self):
        try:
            self.send(type="quit")
        except OSError:
            pass
        try:
            self.proc.wait(timeout=15)
        except subprocess.TimeoutExpired:
            self.proc.kill()
            self.proc.wait(timeout=5)
        for stream in (self.proc.stdin, self.proc.stdout, self.proc.stderr):
            try:
                stream.close()
            except OSError:
                pass


def _new_project(tmp: Path) -> Path:
    out = tmp / "assistent_test.qet"
    res = qm.tool_project_new(_binary(), str(out), title="Assistent-Test", folios=1)
    assert out.is_file(), res
    return out


def _element_count(path: Path) -> int:
    return qm.tool_project_info(str(path))["folios"][0]["elements"]


class Einheiten(unittest.TestCase):
    """Bausteine der Brücke, ohne Unterprozess."""

    @classmethod
    def setUpClass(cls):
        sys.path.insert(0, str(HERE))
        import bruecke
        cls.b = bruecke

    def test_anhaenge_als_bloecke(self):
        bloecke, fehlt = self.b.anhang_bloecke([
            {"name": "foto.png", "mime": "image/png", "data": "iVBORw0KGgo="},
            {"name": "blatt.pdf", "mime": "application/pdf", "data": "JVBERi0x"},
            {"name": "notiz.txt", "mime": "text/plain", "data": "aGFsbG8="},
            {"name": "leer.jpg", "mime": "image/jpeg", "data": ""}])
        self.assertEqual([b["type"] for b in bloecke], ["image", "document"])
        self.assertEqual(bloecke[0]["source"], {"type": "base64", "media_type": "image/png",
                                                "data": "iVBORw0KGgo="})
        self.assertEqual(bloecke[1]["title"], "blatt.pdf")
        self.assertEqual(fehlt, ["notiz.txt", "leer.jpg"])

    def test_anhaenge_gehen_an_claude(self):
        """Mit Schlüssel: die Bilder stehen in der Benutzernachricht der Anfrage."""
        import copy
        from types import SimpleNamespace
        anfragen = []

        def stream(**kw):
            anfragen.append(copy.deepcopy(kw["messages"]))
            usage = SimpleNamespace(input_tokens=0, output_tokens=0, cache_read_input_tokens=0)
            return self.b._FakeStream(SimpleNamespace(
                content=[SimpleNamespace(type="text", text="Ein Schaltschrank.")],
                stop_reason="end_turn", usage=usage, model="test", stop_details=None))
        client = SimpleNamespace(beta=SimpleNamespace(messages=SimpleNamespace(stream=stream)))
        gesendet = []
        alt = self.b.emit
        self.b.emit = lambda **m: gesendet.append(m)
        try:
            g = self.b.Gespraech(client, None, [], {}, attrappe=True)
            g.ask("", {"projekt": "Test"},
                  [{"name": "schrank.jpg", "mime": "image/jpeg", "data": "/9j/4AAQ"}])
        finally:
            self.b.emit = alt
        inhalt = anfragen[0][-1]["content"]
        self.assertEqual([b["type"] for b in inhalt], ["text", "text", "image", "text"])
        self.assertIn("schrank.jpg", inhalt[1]["text"])
        self.assertEqual(inhalt[2]["source"]["media_type"], "image/jpeg")
        self.assertTrue(inhalt[3]["text"])          # leere Frage: ein Satz zum Anhang
        self.assertEqual(gesendet[-1]["type"], "done")

    def test_konto_nachricht(self):
        import asyncio
        self.assertEqual(self.b.konto_nachricht("Hallo"), "Hallo")
        bloecke = [{"type": "image", "source": {"type": "base64", "media_type": "image/png",
                                                 "data": "iVBORw0KGgo="}},
                   {"type": "text", "text": "Was ist das?"}]

        async def sammeln():
            return [m async for m in self.b.konto_nachricht(bloecke)]
        nachrichten = asyncio.run(sammeln())
        self.assertEqual(len(nachrichten), 1)
        self.assertEqual(nachrichten[0]["type"], "user")
        self.assertEqual(nachrichten[0]["message"], {"role": "user", "content": bloecke})

    def test_werkzeuge_ohne_programmpfad(self):
        tools, meta = self.b.build_tools("live")
        names = [t["name"] for t in tools]
        self.assertEqual(names, sorted(names))
        for wanted in ("qet_live_run_script", "qet_live_layout_check", "qet_live_check",
                       "qet_element_search", "qet_live_status"):
            self.assertIn(wanted, names)
        for t in tools:
            self.assertNotIn("binary", t["input_schema"].get("properties", {}))
            self.assertTrue(t["eager_input_streaming"])
        self.assertTrue(meta["qet_live_run_script"][1])
        self.assertFalse(meta["qet_live_status"][1])
        datei, _ = self.b.build_tools("datei")
        self.assertNotIn("qet_live_screenshot", [t["name"] for t in datei])

    def test_pruefung_der_eingaben(self):
        schema = {"type": "object", "properties": {"source": {"type": "string"},
                                                   "timeout": {"type": "integer"}},
                  "required": ["source"]}
        self.assertIsNone(self.b.validate(schema, {"source": "x"}))
        self.assertIn("Pflichtfeld", self.b.validate(schema, {}))
        self.assertIn("Typ", self.b.validate(schema, {"source": "x", "timeout": "5"}))
        self.assertIn("Typ", self.b.validate(schema, {"source": "x", "timeout": True}))
        self.assertIsNotNone(self.b.validate(schema, ["x"]))

    def test_live_kanal_aus_der_umgebung(self):
        old = {k: os.environ.get(k) for k in ("QET_LIVE_SOCKET", "QET_LIVE_TOKEN")}
        try:
            os.environ["QET_LIVE_SOCKET"] = r"\\.\pipe\qet-live-test"
            os.environ["QET_LIVE_TOKEN"] = "abc"
            self.assertEqual(qm._live_session(),
                             {"socket": r"\\.\pipe\qet-live-test", "token": "abc"})
        finally:
            for k, v in old.items():
                if v is None:
                    os.environ.pop(k, None)
                else:
                    os.environ[k] = v

    def test_ohne_schluessel_keine_anfrage(self):
        b = Bruecke(env={"ANTHROPIC_API_KEY": ""})
        try:
            ready = b.until("ready", timeout=60)[-1]
            self.assertFalse(ready["key"])
            b.send(type="ask", text="Hallo")
            seen = b.until("done", timeout=60)
            self.assertTrue(any(m["type"] == "error" and "Schlüssel" in m["message"]
                                for m in seen))
        finally:
            b.close()


class Attrappe(unittest.TestCase):
    """Die ganze Kette mit festen Antworten: Frage -> Status -> Rückfrage
    -> Skript auf der Datei -> Antwort."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="qet-assistent-test-")
        self.projekt = _new_project(Path(self.tmp.name))

    def tearDown(self):
        self.tmp.cleanup()

    def _lauf(self, erlauben: bool) -> list:
        b = Bruecke("--datei", str(self.projekt), "--attrappe", "--attrappe-element", ELEMENT,
                    env={"QET_BINARY": _binary()})
        try:
            ready = b.until("ready", timeout=60)[-1]
            self.assertEqual(ready["mode"], "datei")
            b.send(type="ask", text="Setze eine Spule auf Blatt 1.",
                   kontext={"projekt": "Assistent-Test", "blatt_index_ab_0": 0})
            return b.until("done", timeout=300, answer_confirm=erlauben)
        finally:
            b.close()

    def test_setzt_element_nach_freigabe(self):
        vorher = _element_count(self.projekt)
        seen = self._lauf(True)
        kinds = [m["type"] for m in seen]
        self.assertIn("confirm", kinds)
        tool_ends = [m for m in seen if m["type"] == "tool" and m["phase"] == "end"]
        self.assertEqual([(m["name"], m["ok"]) for m in tool_ends],
                         [("qet_live_status", True), ("qet_live_run_script", True)])
        text = "".join(m["delta"] for m in seen if m["type"] == "text")
        self.assertIn("Fertig", text)
        self.assertEqual(seen[-1]["stop_reason"], "end_turn")
        self.assertEqual(_element_count(self.projekt), vorher + 1)

    def test_ablehnung_aendert_nichts(self):
        vorher = _element_count(self.projekt)
        seen = self._lauf(False)
        text = "".join(m["delta"] for m in seen if m["type"] == "text")
        self.assertIn("nicht geklappt", text)
        self.assertEqual(_element_count(self.projekt), vorher)


@unittest.skipUnless(os.environ.get("ANTHROPIC_API_KEY", "").strip(),
                     "kein ANTHROPIC_API_KEY: echter Test übersprungen")
class EchtesModell(unittest.TestCase):
    """Claude beantwortet eine Frage zum Testprojekt und setzt ein Element."""

    def test_frage_und_element(self):
        with tempfile.TemporaryDirectory(prefix="qet-assistent-echt-") as tmp:
            projekt = _new_project(Path(tmp))
            vorher = _element_count(projekt)
            b = Bruecke("--datei", str(projekt), env={"QET_BINARY": _binary()})
            try:
                self.assertTrue(b.until("ready", timeout=60)[-1]["key"])
                b.send(type="ask", text=(
                    "Wie viele Blätter hat das offene Projekt? Setze danach das Element "
                    f"{ELEMENT} auf das erste Blatt bei x=200, y=200 und sag mir, ob es "
                    "geklappt hat."))
                seen = b.until("done", timeout=900, answer_confirm=True)
            finally:
                b.close()
            text = "".join(m["delta"] for m in seen if m["type"] == "text")
            print("\n--- Antwort von Claude ---\n" + text + "\n---")
            self.assertTrue(text.strip())
            self.assertNotEqual(seen[-1]["stop_reason"], "error",
                                [m for m in seen if m["type"] == "error"])
            self.assertEqual(_element_count(projekt), vorher + 1)


if __name__ == "__main__":
    unittest.main(verbosity=2)
