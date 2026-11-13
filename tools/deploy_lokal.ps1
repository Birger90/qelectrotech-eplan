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

# Fluidtechnik: die Pneumatik- und Hydraulikbauteile der Sammlung durch nachgerüstete ersetzen (Medium und
# Portnummern an jedem Anschluss, gleiche Pfade), dazu die FluidSIM-Symbole, wenn FluidSIM 6 installiert ist.
# Quelle ist immer die unveränderte Sammlung im Repository, so bleibt das Ergebnis bei jedem Lauf gleich.
$sammlung = Join-Path $deploy "elements"
$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) {
    Write-Warning "Python fehlt: Pneumatik-Bauteile nicht nachgerüstet."
} elseif (Test-Path $sammlung) {
    $symbole = Join-Path $repo "tools\symbole"
    foreach ($teil in @(@("50_pneumatic", "pneumatic"), @("30_hydraulic", "hydraulic"))) {
        $quelle = Join-Path $repo "elements\$($teil[0])"
        if (Test-Path $quelle) {
            & $python.Source -X utf8 (Join-Path $symbole "fluid_nachruesten.py") $quelle `
                --ziel (Join-Path $sammlung $teil[0]) --medium $teil[1]
            # Kennbuchstaben je Ordner, nur wenn es noch keine Datei gibt ("Präfixe konfigurieren" bleibt)
            & $python.Source -X utf8 (Join-Path $symbole "fluid_nachruesten.py") --labels `
                (Join-Path $symbole "qet_labels\$($teil[0]).xml") (Join-Path $sammlung $teil[0])
        }
    }
    $fluidsim = "C:\Program Files (x86)\Didactic\FluidSIM6"
    if (Test-Path (Join-Path $fluidsim "sym\sym_fs.lib")) {
        foreach ($teil in @(@("50_pneumatic\5090_FluidSIM", "~FolderPneumatic"),
                            @("30_hydraulic\3090_FluidSIM", "~FolderHydraulic"))) {
            $fs = Join-Path $sammlung $teil[0]
            New-Item -ItemType Directory -Force $fs | Out-Null
            Set-Content -Encoding utf8 (Join-Path $fs "qet_directory") @"
<qet-directory>
    <names>
        <name lang="de">FluidSIM (Festo Didactic)</name>
        <name lang="en">FluidSIM (Festo Didactic)</name>
    </names>
</qet-directory>
"@
            & $python.Source -X utf8 (Join-Path $symbole "fluidsim_symbole.py") --fluidsim $fluidsim `
                --bereich $teil[1] --ziel $fs
        }
    }
}

Write-Output "build\deploy aktuell: $((& $exe --version | Out-String).Trim())"
