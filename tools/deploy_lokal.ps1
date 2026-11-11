# Bringt einen fertigen Build in den lauffähigen Ordner build\deploy:
# Programm, Übersetzungen (lang\*.qm), MCP-Server, Assistent-Brücke und EPLAN-Import.
# Aufruf nach "ninja" im Ordner build:  powershell -File tools\deploy_lokal.ps1
# QElectroTech aus build\deploy muss dafür geschlossen sein (die exe ist sonst gesperrt).
$ErrorActionPreference = "Stop"
$repo   = Split-Path -Parent $PSScriptRoot
$build  = Join-Path $repo "build"
$deploy = Join-Path $build "deploy"

$exe = Join-Path $deploy "QElectroTech.exe"
if (Get-Process | Where-Object { $_.Path -eq $exe }) {
    throw "QElectroTech läuft aus $deploy - bitte erst schließen."
}
Copy-Item (Join-Path $build "qelectrotech.exe") $exe -Force

# Übersetzungen: der Build schreibt sie nach build\lang
New-Item -ItemType Directory -Force (Join-Path $deploy "lang") | Out-Null
Copy-Item (Join-Path $build "lang\*.qm") (Join-Path $deploy "lang") -Force

# Python-Teile, wie sie "cmake --install" ablegt
New-Item -ItemType Directory -Force (Join-Path $deploy "mcp\qet-assistent") | Out-Null
Copy-Item (Join-Path $repo "misc\qet-mcp\qet_mcp.py") (Join-Path $deploy "mcp") -Force
Copy-Item (Join-Path $repo "misc\qet-assistent\bruecke.py") (Join-Path $deploy "mcp\qet-assistent") -Force
$ziel = Join-Path $deploy "eplan_import"
New-Item -ItemType Directory -Force $ziel | Out-Null
foreach ($teil in "eplan_zw1", "planvorlagen") {
    Copy-Item (Join-Path $repo "tools\eplan_import\$teil") $ziel -Recurse -Force
}
Get-ChildItem $ziel -Recurse -Directory -Filter "__pycache__" | Remove-Item -Recurse -Force

Write-Output "build\deploy aktuell: $((& $exe --version | Out-String).Trim())"
