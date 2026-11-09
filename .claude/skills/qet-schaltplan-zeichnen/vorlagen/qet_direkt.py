"""qet_mcp-Funktionen direkt aus Python nutzen (ohne geladenen MCP-Server).

Umgebungsvariablen: QET_MCP (Pfad zu qet_mcp.py, sonst misc/qet-mcp), QET_BINARY (QElectroTech.exe),
QET_PROJEKTE (Zielordner). Ohne QET_BINARY wird build/deploy dieses Repositorys genommen.
"""
import importlib.util
import os
from pathlib import Path

os.environ["QET_ENABLE_SCRIPTING"] = "1"
REPO = Path(__file__).resolve().parents[4]
_spec = importlib.util.spec_from_file_location("qet_mcp", os.environ.get("QET_MCP", str(REPO / "misc" / "qet-mcp" / "qet_mcp.py")))
q = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(q)
BIN = os.environ.get("QET_BINARY", str(REPO / "build" / "deploy" / "QElectroTech.exe"))
USER = str(Path(os.environ.get("APPDATA", Path.home())) / "QElectroTech" / "QElectroTech" / "elements")
COMMON = str(Path(BIN).parent / "elements")
PROJ = os.environ.get("QET_PROJEKTE", os.getcwd())
