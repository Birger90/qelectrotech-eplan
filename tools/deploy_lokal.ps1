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
# Referenzläufe der Simulation (--simulate-reference)
New-Item -ItemType Directory -Force (Join-Path $deploy "simulation") | Out-Null
Copy-Item (Join-Path $repo "tools\simulation\referenz") (Join-Path $deploy "simulation") -Recurse -Force
# Beispiel-Funktionsvorlagen (Seitenleiste "Funktionsvorlagen", qet.insertTemplate)
if (Test-Path (Join-Path $repo "funktionsvorlagen")) {
    Copy-Item (Join-Path $repo "funktionsvorlagen") $deploy -Recurse -Force
}
Copy-Item (Join-Path $repo "misc\qet-assistent\bruecke.py") (Join-Path $deploy "mcp\qet-assistent") -Force
$ziel = Join-Path $deploy "eplan_import"
New-Item -ItemType Directory -Force $ziel | Out-Null
foreach ($teil in "eplan_zw1", "planvorlagen") {
    Copy-Item (Join-Path $repo "tools\eplan_import\$teil") $ziel -Recurse -Force
}
Get-ChildItem $ziel -Recurse -Directory -Filter "__pycache__" | Remove-Item -Recurse -Force
# FluidSIM-Import (Datei > FluidSIM-Schaltung importieren) mit dem Symbolumwandler, den er braucht
$fs = Join-Path $deploy "fluidsim_import"
New-Item -ItemType Directory -Force (Join-Path $fs "symbole") | Out-Null
Copy-Item (Join-Path $repo "tools\fluidsim_import\*.py") $fs -Force
foreach ($datei in "fluidsim_symbole.py", "fluid_schaltwege.py", "fluid_kennwerte.py") {
    Copy-Item (Join-Path $repo "tools\symbole\$datei") (Join-Path $fs "symbole") -Force
}

# Fluidtechnik: die Pneumatik- und Hydraulikbauteile der Sammlung durch nachgerüstete ersetzen (Medium und
# Portnummern an jedem Anschluss, gleiche Pfade).
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
}

# Pneumatik- und Hydrauliksymbole der Standardsammlung mit Simulationsdaten (bauteile/fluid, gleiche Pfade)
# ersetzen die Originale. Frühere Stände (FluidSIM-Symbole, alte ISO-1219-Ordner) werden vorher entfernt.
$fluid_eigen = Join-Path $repo "bauteile\fluid"
if ((Test-Path $fluid_eigen) -and (Test-Path $sammlung)) {
    foreach ($alt in @("50_pneumatic\5090_FluidSIM", "30_hydraulic\3090_FluidSIM",
                       "50_pneumatic\5095_ISO1219", "30_hydraulic\3095_ISO1219")) {
        $p = Join-Path $sammlung $alt
        if (Test-Path $p) { Remove-Item -Recurse -Force $p }
    }
    foreach ($teil in @("50_pneumatic", "30_hydraulic")) {
        $q = Join-Path $fluid_eigen $teil
        if (Test-Path $q) {
            Copy-Item -Recurse -Force (Join-Path $q "*") (Join-Path $sammlung $teil)
        }
    }
}

# GRAFCET-Symbole im 10er-Raster mit mittiger Schrittnummer (bauteile/grafcet, gleiche Pfade) ersetzen die Originale
$grafcet_eigen = Join-Path $repo "bauteile\grafcet\20_logic"
if ((Test-Path $grafcet_eigen) -and (Test-Path $sammlung)) {
    Copy-Item -Recurse -Force (Join-Path $grafcet_eigen "*") (Join-Path $sammlung "20_logic")
}

# Weidmüller-Dateien für den THM-Druck (Katalog my_cat_DE.mca, Layoutdatei mprint-multimark-layouts.json):
# Herstellerdaten, deshalb nicht im Repository, sondern im privaten Datenordner. Sie kommen neben das Programm
# (build\deploy\weidmueller) und damit in jedes Verteilpaket: QET braucht dann kein M-Print PRO.
# Quelle: $env:QET_WEIDMUELLER_DIR, sonst "weidmueller" neben der Firmensammlung der Einstellungen,
# sonst ..\QElectroTech\Daten\weidmueller neben dem Repository.
$weid = $env:QET_WEIDMUELLER_DIR
if (-not $weid) {
    $p = "HKCU:\Software\QElectroTech\QElectroTech\elements-collections"
    $firma = if (Test-Path $p) { (Get-ItemProperty $p -ErrorAction SilentlyContinue)."company-collection-path" } else { $null }
    if ($firma -and $firma -ne "default") { $weid = Join-Path (Split-Path -Parent $firma.TrimEnd("\", "/")) "weidmueller" }
}
if (-not $weid -or -not (Test-Path $weid)) {
    $weid = Join-Path (Split-Path -Parent $repo) "QElectroTech\Daten\weidmueller"
}
if (Test-Path $weid) {
    New-Item -ItemType Directory -Force (Join-Path $deploy "weidmueller") | Out-Null
    Copy-Item (Join-Path $weid "*") (Join-Path $deploy "weidmueller") -Force
    Write-Output "Weidmüller-Dateien: $weid"
} else {
    Write-Warning "Weidmüller-Dateien nicht gefunden (QET_WEIDMUELLER_DIR): THM-Druck braucht dann M-Print PRO oder eine gewählte Datei."
}

Write-Output "build\deploy aktuell: $((& $exe --version | Out-String).Trim())"
