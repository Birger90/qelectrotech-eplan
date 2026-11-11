#!/usr/bin/env python3
"""Brücke zwischen dem Andockfenster „Assistent“ von QElectroTech und Claude.

QElectroTech startet dieses Skript als Unterprozess und spricht mit ihm in
JSON-Zeilen (eine JSON-Nachricht je Zeile, UTF-8) über stdin/stdout.

Von QElectroTech (stdin):
    {"type": "ask", "text": "...", "kontext": {...}}   neue Frage
    {"type": "cancel"}                                 laufende Antwort abbrechen
    {"type": "confirm_answer", "id": n, "allow": bool} Antwort auf "confirm"
    {"type": "reset"}                                  neues Gespräch
    {"type": "quit"}                                   beenden

An QElectroTech (stdout):
    {"type": "ready", "model": ..., "mode": "live"|"datei", "key": bool, "tools": [...]}
    {"type": "status", "text": "..."}                  z. B. „denkt nach …“
    {"type": "text", "delta": "..."}                   Antworttext, gestreamt
    {"type": "tool", "phase": "start"|"end", "name", "label", "write", "ok"}
    {"type": "confirm", "id": n, "tool", "label", "detail", "source"}
                                                       vor der ersten schreibenden
                                                       Aktion einer Sitzung
    {"type": "done", "stop_reason": ..., "usage": {...}}
    {"type": "error", "message": "..."}

Die Werkzeuge sind die von misc/qet-mcp/qet_mcp.py: die Live-Werkzeuge für
das offene Projekt, dazu Prüfen, Layoutprüfung und Bauteilsuche. Der
API-Schlüssel kommt aus der Umgebung (ANTHROPIC_API_KEY, von QElectroTech aus
dem Windows-Anmeldeinformationsspeicher gesetzt) und wird nie ausgegeben.

Betriebsarten:
    (ohne Option)       live: das in QElectroTech offene Projekt, über den
                        Kanal in QET_LIVE_SOCKET / QET_LIVE_TOKEN
    --datei PROJEKT.qet ohne Oberfläche: die „Live“-Werkzeuge arbeiten auf
                        einer Projektdatei (für Tests)
    --attrappe          statt Claude eine feste Abfolge von Antworten
                        (Test ohne Schlüssel)
    --konto             über das eigene Claude-Konto statt eines API-Schlüssels:
                        das Python-Paket claude-agent-sdk startet sein Claude Code,
                        das mit dem Claude-Abo angemeldet ist; kein Schlüssel nötig
    --konto-status      nur ausgeben, ob SDK und Anmeldung da sind (eine JSON-Zeile)
    --konto-login       nur anmelden (claude auth login, öffnet den Browser)
"""

from __future__ import annotations

import argparse
import copy
import json
import os
import queue
import shutil
import sys
import tempfile
import threading
import traceback
from pathlib import Path
from types import SimpleNamespace

MODEL = os.environ.get("QET_ASSISTENT_MODELL", "").strip() or "claude-opus-5-5"
MAX_TOKENS = 64000
EFFORT = os.environ.get("QET_ASSISTENT_EFFORT", "").strip() or "high"
# Server-seitiger Ersatz, falls die Sicherheitsfilter eine Anfrage ablehnen.
FALLBACK_BETA = "server-side-fallback-2026-07-01"
MAX_TOOL_TEXT = 60_000  # Zeichen je Werkzeugantwort, der Rest wird gekürzt


# --------------------------------------------------------------------------
# qet_mcp finden
# --------------------------------------------------------------------------

def _import_qet_mcp():
    here = Path(__file__).resolve().parent
    explicit = os.environ.get("QET_MCP_PATH", "").strip()
    candidates = ([Path(explicit)] if explicit else []) + [
        here.parent / "qet_mcp.py",               # installiert: mcp/qet-assistent/
        here.parent / "qet-mcp" / "qet_mcp.py",   # Quellbaum: misc/qet-assistent/
        here / "qet_mcp.py",
    ]
    for cand in candidates:
        if cand.is_file():
            sys.path.insert(0, str(cand.parent))
            import qet_mcp  # noqa: E402
            return qet_mcp
    raise SystemExit("qet_mcp.py nicht gefunden (QET_MCP_PATH setzen)")


qm = _import_qet_mcp()


# --------------------------------------------------------------------------
# Ausgabe an QElectroTech
# --------------------------------------------------------------------------

_out_lock = threading.Lock()


def emit(**msg) -> None:
    line = json.dumps(msg, ensure_ascii=False)
    with _out_lock:
        sys.stdout.write(line + "\n")
        sys.stdout.flush()


def log(text: str) -> None:
    """Diagnose auf stderr (QElectroTech schreibt sie ins Protokoll).
    Nie den Schlüssel oder die Umgebung ausgeben."""
    with _out_lock:
        sys.stderr.write(text.rstrip() + "\n")
        sys.stderr.flush()


# --------------------------------------------------------------------------
# Werkzeuge
# --------------------------------------------------------------------------

# Name -> (Anzeige im Andockfenster, schreibt in den Plan?)
LIVE_TOOLS = {
    "qet_about":             ("liest die Einstellungen", False),
    "qet_live_status":       ("schaut auf den offenen Plan", False),
    "qet_live_changes":      ("liest den Änderungsverlauf", False),
    "qet_live_show_folio":   ("blättert", False),
    "qet_live_screenshot":   ("sieht sich das Blatt an", False),
    "qet_live_layout_check": ("prüft das Layout", False),
    "qet_element_search":    ("sucht Bauteile", False),
    "qet_element_info":      ("liest ein Bauteil", False),
    "qet_script_api":        ("liest die Skript-Schnittstelle", False),
    "qet_live_run_script":   ("ändert den Plan", True),
    "qet_live_run_stored":   ("führt ein gespeichertes Skript aus", True),
    "qet_live_command":      ("führt einen Befehl aus", True),
    "qet_live_undo_last":    ("nimmt die letzte Änderung zurück", True),
}

# Eigenes Werkzeug der Brücke
CHECK_TOOL = {
    "name": "qet_live_check",
    "description": "Prüft den offenen Plan wie qet_check (doppelte Kennzeichen, "
                   "unbeschriftete Geräte, Leiter ohne Nummer, leere Blätter, "
                   "fehlende Artikelnummern) und meldet die Befunde nach Schwere. "
                   "Arbeitet auf einer Kopie mit allen ungespeicherten Änderungen; "
                   "ändert nichts.",
    "input_schema": {
        "type": "object",
        "properties": {
            "checks": {"type": "array", "items": {"type": "string"},
                       "description": "welche Prüfungen; weglassen für alle"},
            "sample": {"type": "integer", "description": "Beispiele je Prüfung (Standard 10)"},
        },
    },
    "eager_input_streaming": True,
}
OWN_LABELS = {"qet_live_check": ("prüft den Plan", False)}

# In der Dateibetriebsart gibt es keinen Bildschirm
FILE_MODE_SKIP = {"qet_live_show_folio", "qet_live_screenshot", "qet_live_run_stored",
                  "qet_live_command", "qet_live_undo_last"}


def _anthropic_tool(mcp_tool: dict) -> dict:
    schema = copy.deepcopy(mcp_tool.get("inputSchema") or {"type": "object", "properties": {}})
    props = schema.setdefault("properties", {})
    # Das Programm setzt die Brücke selbst ein, Claude soll es nicht wählen.
    props.pop("binary", None)
    if "required" in schema:
        schema["required"] = [r for r in schema["required"] if r != "binary"]
        if not schema["required"]:
            del schema["required"]
    return {"name": mcp_tool["name"], "description": mcp_tool["description"],
            "input_schema": schema, "eager_input_streaming": True}


def build_tools(mode: str) -> tuple[list, dict]:
    """(Werkzeuge für die API, Name -> (Anzeige, schreibt))"""
    by_name = {t["name"]: t for t in qm.TOOLS}
    tools, meta = [], {}
    for name, info in LIVE_TOOLS.items():
        if mode == "datei" and name in FILE_MODE_SKIP:
            continue
        if name not in by_name:
            log(f"Werkzeug fehlt in qet_mcp: {name}")
            continue
        tools.append(_anthropic_tool(by_name[name]))
        meta[name] = info
    tools.append(copy.deepcopy(CHECK_TOOL))
    meta.update(OWN_LABELS)
    # Feste Reihenfolge, damit der Zwischenspeicher (Prompt-Cache) trifft
    tools.sort(key=lambda t: t["name"])
    return tools, meta


def validate(schema: dict, value) -> str | None:
    """Kleine Prüfung gegen das JSON-Schema eines Werkzeugs: Pflichtfelder,
    Grundtypen und Aufzählungen der obersten Ebene. None = in Ordnung."""
    if not isinstance(value, dict):
        return "Eingabe ist kein JSON-Objekt"
    types = {"string": str, "integer": int, "number": (int, float),
             "boolean": bool, "array": list, "object": dict}
    for req in schema.get("required", []):
        if req not in value:
            return f"Pflichtfeld fehlt: {req}"
    for key, val in value.items():
        spec = (schema.get("properties") or {}).get(key)
        if spec is None:
            continue
        want = spec.get("type")
        if isinstance(want, list):
            ok = any(isinstance(val, types[w]) for w in want if w in types)
        elif want in types:
            ok = isinstance(val, types[want])
            if want in ("integer", "number") and isinstance(val, bool):
                ok = False
        else:
            ok = True
        if not ok:
            return f"Feld {key} hat den falschen Typ (erwartet {want})"
        if "enum" in spec and val not in spec["enum"]:
            return f"Feld {key}: {val!r} ist nicht erlaubt"
    return None


def _short(text: str) -> str:
    return text if len(text) <= MAX_TOOL_TEXT else text[:MAX_TOOL_TEXT] + "\n… (gekürzt)"


def _mcp_to_blocks(result: dict) -> tuple[list, bool]:
    """MCP-Ergebnis -> Inhaltsblöcke eines tool_result."""
    blocks = []
    for part in result.get("content", []):
        if part.get("type") == "image":
            blocks.append({"type": "image", "source": {
                "type": "base64", "media_type": part.get("mimeType", "image/png"),
                "data": part["data"]}})
        else:
            blocks.append({"type": "text", "text": _short(part.get("text", ""))})
    return blocks or [{"type": "text", "text": "(leer)"}], bool(result.get("isError"))


def _json_block(out) -> list:
    return [{"type": "text", "text": _short(json.dumps(out, ensure_ascii=False, indent=1))}]


class Werkzeuge:
    """Führt Werkzeugaufrufe aus: live über qet_mcp, oder auf einer Datei."""

    def __init__(self, mode: str, datei: Path | None):
        self.mode = mode
        self.datei = datei
        self.binary = os.environ.get("QET_BINARY", "").strip() or str(qm.resolve_binary() or "")
        self._api_cache = None
        self._runs = []

    def call(self, name: str, args: dict) -> tuple[list, bool]:
        try:
            if name == "qet_script_api" and self._api_cache is not None:
                return self._api_cache
            if name == "qet_live_check":
                return _json_block(self._check(args)), False
            if self.mode == "datei" and name.startswith("qet_live_"):
                out = self._file_tool(name, args)
                return _json_block(out), not out.get("ok", True)
            reply = qm.handle({"jsonrpc": "2.0", "id": 1, "method": "tools/call",
                               "params": {"name": name, "arguments": dict(args)}})
            if "error" in reply:
                return [{"type": "text", "text": reply["error"]["message"]}], True
            blocks, is_error = _mcp_to_blocks(reply["result"])
            if name == "qet_script_api" and not is_error:
                self._api_cache = (blocks, is_error)
            return blocks, is_error
        except Exception as exc:  # an Claude zurück, nicht an den Transport
            log(traceback.format_exc())
            return [{"type": "text", "text": f"{type(exc).__name__}: {exc}"}], True

    # -- Prüfen des offenen Plans ------------------------------------------
    def _check(self, args: dict) -> dict:
        if not self.binary:
            raise ValueError("kein QElectroTech-Programm (QET_BINARY)")
        checks = args.get("checks") or None
        sample = args.get("sample", 10)
        if self.mode == "datei":
            out = qm.tool_check(self.binary, str(self.datei), checks, sample)
            out["checked"] = "die Projektdatei"
            return out
        work = Path(tempfile.mkdtemp(prefix="qet-assistent-check-"))
        try:
            snap = qm._live_call({"cmd": "snapshot", "path": str(work / "live.qet")})
            if not snap.get("ok"):
                return snap
            out = qm.tool_check(self.binary, snap["path"], checks, sample)
            out["checked"] = "der offene Plan, wie auf dem Bildschirm (mit ungespeicherten Änderungen)"
            return out
        finally:
            shutil.rmtree(work, ignore_errors=True)

    # -- Dateibetriebsart --------------------------------------------------
    def _file_tool(self, name: str, args: dict) -> dict:
        if name == "qet_live_status":
            info = qm.tool_project_info(str(self.datei))
            return {"ok": True, "mode": "datei", "project": str(self.datei),
                    "folio": 0, "info": info}
        if name == "qet_live_changes":
            return {"ok": True, "steps": self._runs}
        if name == "qet_live_layout_check":
            folio = args.get("folio", "current")
            folio_no = 1 if folio == "current" else (None if folio == "all" else folio)
            return qm.tool_layout_check(self.binary, str(self.datei), folio=folio_no)
        if name == "qet_live_run_script":
            return self._run_on_file(args["source"], args.get("name") or "Skript")
        raise ValueError(f"{name} gibt es ohne Oberfläche nicht")

    def _run_on_file(self, source: str, name: str) -> dict:
        """Wie ein Live-Skript, aber auf der Datei: auf einer Kopie ausführen,
        speichern, bei Erfolg die Datei ersetzen. Ein Lauf = ein Schritt."""
        with tempfile.TemporaryDirectory(prefix="qet-assistent-run-") as tmp:
            kopie = Path(tmp) / self.datei.name
            shutil.copy2(self.datei, kopie)
            script = ("if (typeof qet.currentFolio !== 'function') "
                      "qet.currentFolio = function () { return qet.folioCount() ? 0 : -1; }; "
                      + source + "\n"
                      "qet.log(%s + JSON.stringify({kind: 'save', result: qet.save(%s)}));\n"
                      % (json.dumps(qm._MARKER), json.dumps(str(kopie))))
            result = qm._run_qet(self.binary, [str(kopie)], timeout=180, script=script,
                                 tail=200_000)
            streams = result.get("stdout", "") + "\n" + result.get("stderr", "")
            saved = None
            for line in streams.splitlines():
                idx = line.find(qm._MARKER)
                if idx >= 0:
                    try:
                        saved = json.loads(line[idx + len(qm._MARKER):]).get("result")
                    except json.JSONDecodeError:
                        pass
            errors = [ln.strip() for ln in streams.splitlines() if "Script error:" in ln]
            logged = [ln for ln in streams.splitlines()
                      if ln.strip() and qm._MARKER not in ln and "Script error:" not in ln]
            out = {"ok": saved is True and not errors, "errors": errors, "log": logged[-40:]}
            if out["ok"]:
                out["diff"] = qm.tool_diff(str(self.datei), str(kopie))
                shutil.copy2(kopie, self.datei)
                out["undo"] = name
                self._runs.append({"name": name, "mine": True})
            return out


# --------------------------------------------------------------------------
# Systemanweisung
# --------------------------------------------------------------------------

SYSTEM = """Du bist der eingebaute Assistent von QElectroTech (Fork mit EPLAN-Import, \
Schaltschrankaufbau und Herstellerportalen). Du hilfst bei dem Schaltplan, den der \
Benutzer gerade offen hat. Antworte auf Deutsch, kurz und sachlich, wie ein erfahrener \
Elektrokonstrukteur.

So arbeitest du:
- qet_live_status zeigt Projekt, Blatt und Auswahl. qet_about nennt die Bauteilsammlungen \
(folders), den Hausstil (house_style) und die Standardsymbole (standard_symbols); halte \
dich beim Zeichnen daran.
- Änderungen am Plan machst du nur mit qet_live_run_script. Jeder Lauf ist ein eigener \
Rückgängig-Schritt; gib ihm einen kurzen deutschen Namen, der sagt, was er tut. Lies vor \
deinem ersten Skript einmal qet_script_api. Lieber mehrere kleine, nachvollziehbare \
Schritte als ein großes Skript.
- Bauteile suchst du mit qet_element_search in den Sammlungen aus qet_about. Verwende \
vorhandene Herstellersymbole, bevor du etwas Eigenes zeichnest.
- Nach dem Zeichnen prüfst du mit qet_live_layout_check und wendest die Korrekturen an. \
qet_live_check findet inhaltliche Fehler (doppelte Kennzeichen usw.).
- Speichern, Schließen, Drucken und Exportieren macht der Benutzer selbst.
- Lehnt der Benutzer eine Änderung ab, frag nach, was er stattdessen möchte, statt es \
auf anderem Weg zu versuchen.
- Blattnummern: achte darauf, ob ein Werkzeug ab 0 oder ab 1 zählt (steht in seiner \
Beschreibung).
- Sag am Ende knapp, was du geändert hast und was offen ist."""


# --------------------------------------------------------------------------
# Attrappe: feste Antworten statt Claude
# --------------------------------------------------------------------------

class _FakeStream:
    def __init__(self, message):
        self._message = message

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        return False

    def __iter__(self):
        for block in self._message.content:
            if block.type == "text":
                for word in block.text.split(" "):
                    yield SimpleNamespace(type="text", text=word + " ")
            elif block.type == "tool_use":
                yield SimpleNamespace(type="content_block_start", content_block=block)

    def get_final_message(self):
        return self._message


class Attrappe:
    """Spielt eine feste Abfolge ab: Status lesen, ein Element setzen,
    antworten. So lässt sich die Brücke ohne Schlüssel prüfen."""

    def __init__(self, element: str):
        self.element = element
        self.beta = SimpleNamespace(messages=SimpleNamespace(stream=self._stream))
        self.requests = []

    def _stream(self, **kw):
        self.requests.append(kw)
        msgs = kw["messages"]
        last = msgs[-1]
        results = [b for b in (last["content"] if isinstance(last["content"], list) else [])
                   if isinstance(b, dict) and b.get("type") == "tool_result"]
        step = sum(1 for m in msgs if m["role"] == "assistant")
        usage = SimpleNamespace(input_tokens=0, output_tokens=0,
                                cache_read_input_tokens=0, cache_creation_input_tokens=0)
        if step == 0:
            content = [SimpleNamespace(type="text", text="Ich sehe mir den Plan an."),
                       SimpleNamespace(type="tool_use", id="toolu_a1", name="qet_live_status",
                                       input={})]
            stop = "tool_use"
        elif step == 1:
            src = ("var u = qet.addElement(0, %s, 100, 100);\n"
                   "qet.log('gesetzt ' + u);\n") % json.dumps(self.element)
            content = [SimpleNamespace(type="text", text="Ich setze das Element."),
                       SimpleNamespace(type="tool_use", id="toolu_a2",
                                       name="qet_live_run_script",
                                       input={"source": src, "name": "Element setzen"})]
            stop = "tool_use"
        else:
            ok = bool(results) and not results[0].get("is_error")
            text = ("Fertig: ein Element auf Blatt 1 gesetzt." if ok
                    else "Das Setzen hat nicht geklappt.")
            content = [SimpleNamespace(type="text", text=text)]
            stop = "end_turn"
        return _FakeStream(SimpleNamespace(content=content, stop_reason=stop, usage=usage,
                                           model="attrappe", stop_details=None))


# --------------------------------------------------------------------------
# Gespräch
# --------------------------------------------------------------------------

def _block_dict(block):
    """Ein Antwortblock für die nächste Anfrage, unverändert.

    SDK-Objekte gehen so zurück, wie sie kamen (Denkblöcke müssen
    unverändert bleiben); nur die Attrappe liefert einfache Objekte."""
    if isinstance(block, SimpleNamespace):
        return dict(vars(block))
    return block


class Gespraech:
    def __init__(self, client, werkzeuge: Werkzeuge, tools: list, meta: dict, attrappe: bool):
        self.client = client
        self.werkzeuge = werkzeuge
        self.tools = tools
        self.tools_by_name = {t["name"]: t for t in tools}
        self.meta = meta
        self.attrappe = attrappe
        self.messages: list = []
        self.cancel = threading.Event()
        self.write_allowed = False
        self.confirm_answers: "queue.Queue[dict]" = queue.Queue()
        self._confirm_id = 0

    def reset(self) -> None:
        self.messages = []
        self.write_allowed = False

    # Rückfrage vor der ersten schreibenden Aktion der Sitzung -------------
    def _ask_write(self, name: str, args: dict) -> bool:
        if self.write_allowed:
            return True
        self._confirm_id += 1
        cid = self._confirm_id
        detail = args.get("name") or args.get("id") or args.get("action") or ""
        emit(type="confirm", id=cid, tool=name, label=self.meta.get(name, (name,))[0],
             detail=str(detail), source=str(args.get("source", ""))[:4000])
        while True:
            if self.cancel.is_set():
                return False
            try:
                ans = self.confirm_answers.get(timeout=0.2)
            except queue.Empty:
                continue
            if ans.get("id") == cid:
                break
        self.write_allowed = bool(ans.get("allow"))
        return self.write_allowed

    def ask(self, text: str, kontext: dict | None) -> None:
        self.cancel.clear()
        start_len = len(self.messages)
        content = []
        if kontext:
            content.append({"type": "text", "text": "Zustand in QElectroTech: "
                            + json.dumps(kontext, ensure_ascii=False)})
        content.append({"type": "text", "text": text})
        self.messages.append({"role": "user", "content": content})
        answered = False
        json_retries = 0
        usage_total = {"input_tokens": 0, "output_tokens": 0, "cache_read_input_tokens": 0}
        stop_reason = None
        try:
            while True:
                emit(type="status", text="denkt nach …")
                try:
                    response = self._stream_once()
                    json_retries = 0
                except ValueError:
                    # Werkzeugeingabe war kein lesbares JSON: Runde wiederholen
                    json_retries += 1
                    if json_retries > 2:
                        raise
                    continue
                if response is None:      # abgebrochen
                    stop_reason = "cancelled"
                    break
                for k in usage_total:
                    usage_total[k] += getattr(response.usage, k, 0) or 0
                stop_reason = response.stop_reason
                if stop_reason == "refusal":
                    det = getattr(response, "stop_details", None)
                    cat = getattr(det, "category", None) if det is not None else None
                    emit(type="error", message="Claude hat die Anfrage abgelehnt"
                         + (f" ({cat})" if cat else "") + ".")
                    break
                if stop_reason == "pause_turn":
                    self.messages.append({"role": "assistant",
                                          "content": [_block_dict(b) for b in response.content]})
                    answered = True
                    continue
                tool_uses = [b for b in response.content if b.type == "tool_use"]
                if not tool_uses:
                    self.messages.append({"role": "assistant",
                                          "content": [_block_dict(b) for b in response.content]})
                    answered = True
                    if stop_reason == "max_tokens":
                        emit(type="error", message="Die Antwort wurde wegen der Längengrenze abgeschnitten.")
                    break
                if stop_reason == "max_tokens":
                    # abgeschnittene Werkzeugeingabe nie ausführen
                    emit(type="error", message="Die Antwort wurde abgeschnitten; bitte noch einmal fragen.")
                    break
                results = self._run_tools(tool_uses)
                self.messages.append({"role": "assistant",
                                      "content": [_block_dict(b) for b in response.content]})
                self.messages.append({"role": "user", "content": results})
                answered = True
                if self.cancel.is_set():
                    stop_reason = "cancelled"
                    break
        except Exception as exc:
            log(traceback.format_exc())
            emit(type="error", message=_api_error_text(exc))
            stop_reason = "error"
        if not answered:
            # Diese Frage hat keine Antwort bekommen: aus dem Verlauf nehmen,
            # damit sie nicht mit der nächsten verschmilzt.
            del self.messages[start_len:]
        emit(type="done", stop_reason=stop_reason, usage=usage_total)

    def _stream_once(self):
        kwargs = dict(
            model=MODEL, max_tokens=MAX_TOKENS, system=SYSTEM, tools=self.tools,
            messages=self.messages,
            thinking={"type": "adaptive", "display": "summarized"},
            output_config={"effort": EFFORT},
            cache_control={"type": "ephemeral"},
        )
        if not self.attrappe:
            kwargs.update(betas=[FALLBACK_BETA], fallbacks="default")
        thinking_shown = False
        with self.client.beta.messages.stream(**kwargs) as stream:
            for event in stream:
                if self.cancel.is_set():
                    return None
                etype = getattr(event, "type", "")
                if etype == "text":
                    emit(type="text", delta=event.text)
                elif etype == "thinking" and not thinking_shown:
                    emit(type="status", text="denkt nach …")
                    thinking_shown = True
                elif etype == "content_block_start":
                    block = event.content_block
                    if getattr(block, "type", "") == "tool_use":
                        label = self.meta.get(block.name, (block.name,))[0]
                        emit(type="status", text=f"bereitet vor: {label}")
            if self.cancel.is_set():
                return None
            return stream.get_final_message()

    def _run_tools(self, tool_uses) -> list:
        results = []
        for block in tool_uses:
            if self.cancel.is_set():
                results.append({"type": "tool_result", "tool_use_id": block.id, "is_error": True,
                                "content": "vom Benutzer abgebrochen"})
                continue
            name = block.name
            label, writes = self.meta.get(name, (name, False))
            schema = (self.tools_by_name.get(name) or {}).get("input_schema")
            if schema is None:
                results.append({"type": "tool_result", "tool_use_id": block.id, "is_error": True,
                                "content": f"unbekanntes Werkzeug: {name}"})
                continue
            problem = validate(schema, block.input)
            if problem:
                results.append({"type": "tool_result", "tool_use_id": block.id, "is_error": True,
                                "content": json.dumps({"INVALID_JSON": json.dumps(block.input, ensure_ascii=False),
                                                       "problem": problem}, ensure_ascii=False)})
                continue
            if writes and not self._ask_write(name, block.input):
                results.append({"type": "tool_result", "tool_use_id": block.id, "is_error": True,
                                "content": "Der Benutzer hat die Änderung am Plan nicht erlaubt."})
                emit(type="tool", phase="end", name=name, label=label, write=True, ok=False)
                continue
            emit(type="tool", phase="start", name=name, label=label, write=writes)
            blocks, is_error = self.werkzeuge.call(name, block.input)
            emit(type="tool", phase="end", name=name, label=label, write=writes, ok=not is_error)
            res = {"type": "tool_result", "tool_use_id": block.id, "content": blocks}
            if is_error:
                res["is_error"] = True
            results.append(res)
        return results


def _api_error_text(exc: Exception) -> str:
    try:
        import anthropic
    except ImportError:
        return f"{type(exc).__name__}: {exc}"
    if isinstance(exc, anthropic.AuthenticationError):
        return "Der API-Schlüssel wurde nicht angenommen. Bitte in den Einstellungen des Assistenten prüfen."
    if isinstance(exc, anthropic.PermissionDeniedError):
        return "Der API-Schlüssel hat keine Berechtigung für dieses Modell."
    if isinstance(exc, anthropic.RateLimitError):
        return "Zu viele Anfragen: bitte kurz warten und noch einmal versuchen."
    if isinstance(exc, anthropic.APIConnectionError):
        return "Keine Verbindung zum Claude-Dienst (Netzwerk prüfen)."
    if isinstance(exc, anthropic.APIStatusError):
        return f"Fehler vom Claude-Dienst ({exc.status_code}): {getattr(exc, 'message', exc)}"
    return f"{type(exc).__name__}: {exc}"


# --------------------------------------------------------------------------
# Konto: Claude über das eigene Claude-Konto (Claude Agent SDK)
# --------------------------------------------------------------------------
#
# Statt die API mit einem Schlüssel anzusprechen, startet die Brücke das
# Claude Code, das dem Python-Paket „claude-agent-sdk“ beiliegt. Es meldet
# sich mit dem Claude-Konto des Benutzers an (claude auth login), die
# Nutzung zählt gegen dessen Abo. Die Werkzeuge sind dieselben wie oben;
# sie laufen im Prozess der Brücke als MCP-Server „qet“, die eingebauten
# Werkzeuge von Claude Code (Dateien, Shell, Web) sind abgeschaltet.

KONTO_SERVER = "qet"
# Würde Claude Code einen Schlüssel aus der Umgebung finden, nähme es den
# und rechnete über die API ab: im Kontobetrieb nie weitergeben.
_KEY_VARS = ("ANTHROPIC_API_KEY", "ANTHROPIC_AUTH_TOKEN")


def _konto_cli() -> str | None:
    """Das Claude Code, das dem SDK beiliegt; sonst eines im PATH."""
    try:
        import claude_agent_sdk
    except ImportError:
        return None
    exe = "claude.exe" if os.name == "nt" else "claude"
    bundled = Path(claude_agent_sdk.__file__).resolve().parent / "_bundled" / exe
    if bundled.is_file():
        return str(bundled)
    return shutil.which("claude")


def _konto_env() -> dict:
    return {k: v for k, v in os.environ.items() if k not in _KEY_VARS and k != "CLAUDECODE"}


def konto_status() -> dict:
    """Ist das SDK da, ist Claude Code angemeldet? Für die Einrichtung."""
    import subprocess
    out = {"sdk": False, "loggedIn": False}
    try:
        import claude_agent_sdk
        out["sdk"] = True
        out["version"] = getattr(claude_agent_sdk, "__version__", "")
    except ImportError:
        return out
    cli = _konto_cli()
    if not cli:
        out["error"] = "Claude Code nicht gefunden"
        return out
    try:
        res = subprocess.run([cli, "auth", "status"], capture_output=True, text=True,
                             encoding="utf-8", errors="replace", timeout=60, env=_konto_env())
        data = json.loads(res.stdout or "{}")
    except Exception as exc:  # noqa: BLE001 - Meldung an die Einrichtung
        out["error"] = f"{type(exc).__name__}: {exc}"
        return out
    out["loggedIn"] = bool(data.get("loggedIn"))
    for key in ("authMethod", "email", "subscriptionType", "orgName"):
        if data.get(key):
            out[key] = data[key]
    return out


def konto_login() -> int:
    """claude auth login mit dem Claude-Abo; öffnet den Browser. Die Ausgabe
    geht durch, damit die Einrichtung den Link zeigen kann."""
    import subprocess
    cli = _konto_cli()
    if not cli:
        print("Claude Code nicht gefunden: erst das Paket claude-agent-sdk installieren.", flush=True)
        return 2
    return subprocess.call([cli, "auth", "login", "--claudeai"], env=_konto_env())


def _mcp_content(blocks: list) -> list:
    """tool_result-Blöcke -> MCP-Inhalt"""
    out = []
    for b in blocks:
        if b.get("type") == "image":
            src = b.get("source", {})
            out.append({"type": "image", "data": src.get("data", ""),
                        "mimeType": src.get("media_type", "image/png")})
        else:
            out.append({"type": "text", "text": b.get("text", "")})
    return out


class KontoGespraech:
    """Ein Gespräch über das Claude-Konto, mit derselben Schnittstelle wie
    Gespraech (ask, reset, cancel, confirm_answers). Der Client läuft in einer
    eigenen asyncio-Schleife; ask() wartet in seinem Faden auf das Ende."""

    def __init__(self, werkzeuge: Werkzeuge, tools: list, meta: dict):
        import asyncio
        self.asyncio = asyncio
        self.werkzeuge = werkzeuge
        self.tools = tools
        self.tools_by_name = {t["name"]: t for t in tools}
        self.meta = meta
        self.client = True   # für main(): es gibt etwas, das antworten kann
        self.cancel = threading.Event()
        self.write_allowed = False
        self.confirm_answers: "queue.Queue[dict]" = queue.Queue()
        self._confirm_id = 0
        self._sdk_client = None
        self._loop = asyncio.new_event_loop()
        threading.Thread(target=self._loop.run_forever, daemon=True).start()

    # dieselbe Rückfrage vor der ersten Änderung wie bei Gespraech
    _ask_write = Gespraech._ask_write

    def _run(self, coro, timeout=None):
        return self.asyncio.run_coroutine_threadsafe(coro, self._loop).result(timeout)

    def _server(self):
        from claude_agent_sdk import create_sdk_mcp_server, tool

        def make(spec: dict):
            name = spec["name"]
            label, writes = self.meta.get(name, (name, False))

            async def handler(args):
                return await self.asyncio.to_thread(self._call_tool, name, label, writes, args)
            return tool(name, spec.get("description", ""), spec["input_schema"])(handler)

        return create_sdk_mcp_server(KONTO_SERVER, tools=[make(t) for t in self.tools])

    def _call_tool(self, name: str, label: str, writes: bool, args: dict) -> dict:
        args = dict(args or {})
        problem = validate((self.tools_by_name.get(name) or {}).get("input_schema", {}), args)
        if problem:
            return {"content": [{"type": "text", "text": problem}], "is_error": True}
        if writes and not self._ask_write(name, args):
            emit(type="tool", phase="end", name=name, label=label, write=True, ok=False)
            return {"content": [{"type": "text", "text": "Der Benutzer hat die Änderung am "
                                 "Plan nicht erlaubt."}], "is_error": True}
        emit(type="tool", phase="start", name=name, label=label, write=writes)
        blocks, is_error = self.werkzeuge.call(name, args)
        emit(type="tool", phase="end", name=name, label=label, write=writes, ok=not is_error)
        return {"content": _mcp_content(blocks), "is_error": is_error}

    async def _connect(self):
        from claude_agent_sdk import ClaudeAgentOptions, ClaudeSDKClient
        for var in _KEY_VARS:
            os.environ.pop(var, None)
        model = os.environ.get("QET_ASSISTENT_MODELL", "").strip() or None
        options = ClaudeAgentOptions(
            system_prompt=SYSTEM,
            mcp_servers={KONTO_SERVER: self._server()},
            strict_mcp_config=True,
            tools=[],                    # keine Datei-, Shell- oder Webwerkzeuge
            allowed_tools=[f"mcp__{KONTO_SERVER}__{t['name']}" for t in self.tools],
            permission_mode="dontAsk",   # alles andere ablehnen, nie nachfragen
            setting_sources=[],          # keine CLAUDE.md, Skills oder Hooks des Benutzers
            include_partial_messages=True,
            cwd=tempfile.gettempdir(),
            cli_path=_konto_cli(),
            model=model,
            stderr=log,
        )
        client = ClaudeSDKClient(options=options)
        await client.connect()
        self._sdk_client = client

    async def _disconnect(self):
        if self._sdk_client is not None:
            client, self._sdk_client = self._sdk_client, None
            await client.disconnect()

    def reset(self) -> None:
        self.write_allowed = False
        try:
            self._run(self._disconnect(), timeout=30)
        except Exception:  # noqa: BLE001
            log(traceback.format_exc())

    def ask(self, text: str, kontext: dict | None) -> None:
        import concurrent.futures
        self.cancel.clear()
        prompt = text
        if kontext:
            prompt = ("Zustand in QElectroTech: " + json.dumps(kontext, ensure_ascii=False)
                      + "\n\n" + text)
        stop_reason, usage = None, {}
        try:
            if self._sdk_client is None:
                emit(type="status", text="verbindet mit Claude …")
                self._run(self._connect(), timeout=180)
            emit(type="status", text="denkt nach …")
            fut = self.asyncio.run_coroutine_threadsafe(self._turn(prompt), self._loop)
            interrupted = False
            while True:
                try:
                    stop_reason, usage = fut.result(timeout=0.2)
                    break
                except concurrent.futures.TimeoutError:
                    if self.cancel.is_set() and not interrupted:
                        interrupted = True
                        self._run(self._sdk_client.interrupt(), timeout=30)
            if interrupted:
                stop_reason = "cancelled"
        except Exception as exc:  # noqa: BLE001
            log(traceback.format_exc())
            emit(type="error", message=_konto_error_text(exc))
            stop_reason = "error"
            try:
                self._run(self._disconnect(), timeout=30)
            except Exception:  # noqa: BLE001
                pass
        emit(type="done", stop_reason=stop_reason, usage=usage or {})

    async def _turn(self, prompt: str):
        from claude_agent_sdk import AssistantMessage, ResultMessage, StreamEvent
        await self._sdk_client.query(prompt)
        stop_reason, usage = None, {}
        shown = set()   # dieselbe Meldung kommt sonst doppelt (Antwort und Ergebnis)

        def error(text):
            nonlocal stop_reason
            stop_reason = "error"
            msg = _konto_error_text(text)
            if msg not in shown:
                shown.add(msg)
                emit(type="error", message=msg)
        async for msg in self._sdk_client.receive_response():
            if isinstance(msg, StreamEvent):
                ev = msg.event or {}
                if msg.parent_tool_use_id:
                    continue
                if ev.get("type") == "content_block_delta":
                    delta = ev.get("delta") or {}
                    if delta.get("type") == "text_delta" and delta.get("text"):
                        emit(type="text", delta=delta["text"])
                elif ev.get("type") == "content_block_start":
                    block = ev.get("content_block") or {}
                    if block.get("type") == "tool_use":
                        name = str(block.get("name", "")).split("__")[-1]
                        emit(type="status", text="bereitet vor: "
                             + self.meta.get(name, (name,))[0])
            elif isinstance(msg, AssistantMessage) and msg.error:
                error(msg.error)
            elif isinstance(msg, ResultMessage):
                if stop_reason != "error":
                    stop_reason = msg.stop_reason or msg.subtype
                usage = {k: v for k, v in (msg.usage or {}).items()
                         if k in ("input_tokens", "output_tokens", "cache_read_input_tokens")}
                if msg.is_error and msg.result:
                    error(msg.result)
        return stop_reason, usage


def _konto_error_text(exc) -> str:
    text = str(exc)
    low = text.lower()
    if any(s in low for s in ("not logged", "log in", "login", "authenticat", "401")):
        return ("Claude ist nicht verbunden. Bitte unter „Einstellungen“ auf "
                "„Mit Claude verbinden“ klicken.")
    if "usage limit" in low or ("rate" in low and "limit" in low):
        return "Das Nutzungskontingent deines Claude-Abos ist gerade ausgeschöpft."
    if isinstance(exc, BaseException):
        return f"{type(exc).__name__}: {exc}"
    return text


# --------------------------------------------------------------------------
# Hauptschleife
# --------------------------------------------------------------------------

def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--datei", help="ohne Oberfläche auf dieser Projektdatei arbeiten")
    ap.add_argument("--attrappe", action="store_true", help="feste Antworten statt Claude")
    ap.add_argument("--attrappe-element", default="",
                    help="Element, das die Attrappe setzt (z. B. common://...elmt)")
    ap.add_argument("--konto", action="store_true",
                    help="über das eigene Claude-Konto (Claude Agent SDK) statt API-Schlüssel")
    ap.add_argument("--konto-status", action="store_true",
                    help="nur ausgeben, ob das SDK da und Claude angemeldet ist (JSON)")
    ap.add_argument("--konto-login", action="store_true",
                    help="nur mit dem Claude-Konto anmelden (öffnet den Browser)")
    opts = ap.parse_args(argv)

    for stream in (sys.stdin, sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8")
        except (AttributeError, ValueError):
            pass
    if opts.konto_status:
        print(json.dumps(konto_status(), ensure_ascii=False), flush=True)
        return 0
    if opts.konto_login:
        return konto_login()

    # Die Brücke darf Skripte ausführen; ob sie den Plan ändern darf, fragt
    # QElectroTech den Benutzer (und erzwingt es im Live-Kanal).
    os.environ["QET_ENABLE_SCRIPTING"] = "1"
    work = os.environ.get("QET_MCP_WORKSPACE", "")
    extra = [tempfile.gettempdir()]
    if opts.datei:
        extra.append(str(Path(opts.datei).resolve().parent))
    os.environ["QET_MCP_WORKSPACE"] = os.pathsep.join(
        [p for p in work.split(os.pathsep) if p] + extra)

    mode = "datei" if opts.datei else "live"
    datei = Path(opts.datei).resolve() if opts.datei else None
    if datei is not None and not datei.is_file():
        emit(type="error", message=f"Projektdatei fehlt: {datei}")
        return 2

    has_key = bool(os.environ.get("ANTHROPIC_API_KEY", "").strip())
    client = None
    konto_ok = False
    if opts.konto and not opts.attrappe:
        try:
            import claude_agent_sdk  # noqa: F401
            konto_ok = True
        except ImportError:
            emit(type="error", message="Das Python-Paket „claude-agent-sdk“ fehlt: "
                 "unter „Einstellungen“ auf „Mit Claude verbinden“ klicken.")
    elif opts.attrappe:
        client = Attrappe(opts.attrappe_element or "embed://import/element.elmt")
    else:
        try:
            import anthropic
        except ImportError:
            emit(type="error", message="Das Python-Paket „anthropic“ fehlt: "
                 "py -m pip install anthropic")
        else:
            if has_key:
                client = anthropic.Anthropic()
    werkzeuge = Werkzeuge(mode, datei)
    tools, meta = build_tools(mode)
    if konto_ok:
        gespraech = KontoGespraech(werkzeuge, tools, meta)
        modell = os.environ.get("QET_ASSISTENT_MODELL", "").strip() or "Claude-Konto"
    else:
        gespraech = Gespraech(client, werkzeuge, tools, meta, opts.attrappe)
        modell = "attrappe" if opts.attrappe else MODEL
    emit(type="ready", model=modell, mode=mode,
         key=gespraech.client is not None, konto=konto_ok, tools=[t["name"] for t in tools])

    worker: threading.Thread | None = None
    for raw in sys.stdin:
        raw = raw.strip()
        if not raw:
            continue
        try:
            msg = json.loads(raw)
        except json.JSONDecodeError:
            emit(type="error", message="unlesbare Nachricht von QElectroTech")
            continue
        kind = msg.get("type")
        if kind == "ask":
            if worker is not None and worker.is_alive():
                emit(type="error", message="Es läuft noch eine Antwort.")
                continue
            if gespraech.client is None:
                emit(type="error", message="Kein API-Schlüssel hinterlegt und kein "
                     "Claude-Konto verbunden (Claude-Assistent > Einstellungen).")
                emit(type="done", stop_reason="error", usage={})
                continue
            worker = threading.Thread(target=gespraech.ask,
                                      args=(str(msg.get("text", "")), msg.get("kontext")),
                                      daemon=True)
            worker.start()
        elif kind == "cancel":
            gespraech.cancel.set()
        elif kind == "confirm_answer":
            gespraech.confirm_answers.put(msg)
        elif kind == "reset":
            gespraech.cancel.set()
            if worker is not None:
                worker.join(timeout=30)
            gespraech.reset()
            emit(type="status", text="neues Gespräch")
        elif kind == "quit":
            break
    gespraech.cancel.set()
    if worker is not None:
        worker.join(timeout=10)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
