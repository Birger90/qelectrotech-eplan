# Legt die aktuelle Version in den Verteilordner der Firma (SharePoint-Bibliothek, mit OneDrive synchronisiert):
#   Programm\QET-Fork-<Version>.zip   aus build\deploy (ohne alte Test-Exes QElectroTech_*.exe)
#   Installieren.ps1 / Installieren.cmd
#   Bauteile\                         Spiegel der Firmensammlung (ohne .git)
#   Meldungen\                        Fehler- und Wunschmeldungen der Kollegen
#   version.json                      zuletzt: erst wenn alles andere da ist, sehen die Kollegen das Update
#
#   powershell -File tools\verteilung\veroeffentlichen.ps1 -Ziel "C:\Users\...\<Firma>\QET-Fork - Dokumente" -Hinweis "R06: Regler"
#
# Ohne -Ziel: der Verteilordner aus den QET-Einstellungen (distribution/folder) bzw. QET_DISTRIBUTION_DIR.
# Ohne -Bauteile: die Firmensammlung aus den QET-Einstellungen (elements-collections/company-collection-path).
# -Programmordner: ein anderer fertiger Programmordner als build\deploy (z. B. aus einem anderen Worktree).
param(
    [string]$Ziel = "",
    [string]$Bauteile = "",
    [string]$Hinweis = "",
    [switch]$OhneBauteile,
    [string]$Programmordner = ""
)
$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$deploy = if ($Programmordner) { (Resolve-Path $Programmordner).Path } else { Join-Path $repo "build\deploy" }
$einst = "HKCU:\Software\QElectroTech\QElectroTech"

function Einstellung([string]$gruppe, [string]$name) {
    $p = Join-Path $einst $gruppe
    if (Test-Path $p) { return (Get-ItemProperty $p -ErrorAction SilentlyContinue).$name }
    return $null
}

if (-not $Ziel) { $Ziel = $env:QET_DISTRIBUTION_DIR }
if (-not $Ziel) { $Ziel = Einstellung "distribution" "folder" }
if (-not $Ziel) { throw "Kein Verteilordner: -Ziel angeben (der mit OneDrive synchronisierte SharePoint-Ordner)." }
$Ziel = $Ziel -replace "/", "\"
if (-not (Test-Path $Ziel)) { throw "Verteilordner $Ziel gibt es nicht." }

$version = (Select-String -Path (Join-Path $repo "sources\qetversion.cpp") -Pattern 'forkVersion\(\)' -Context 0,3 |
    ForEach-Object { $_.Context.PostContext } | Select-String -Pattern '"(\d+\.\d+\.\d+)"').Matches[0].Groups[1].Value
if (-not $version) { throw "Fork-Version in sources\qetversion.cpp nicht gefunden." }
$exe = Join-Path $deploy "QElectroTech.exe"
if (-not (Test-Path $exe)) { throw "build\deploy\QElectroTech.exe fehlt: erst bauen und tools\deploy_lokal.ps1 ausführen." }
Write-Host "Version $version nach $Ziel"

# 1. Programmpaket
$programm = Join-Path $Ziel "Programm"
New-Item -ItemType Directory -Force $programm | Out-Null
$name = "QET-Fork-$version.zip"
$tmp = Join-Path $env:TEMP $name
if (Test-Path $tmp) { Remove-Item $tmp -Force }
$zip = [System.IO.Compression.ZipFile]::Open($tmp, "Create")
try {
    $n = 0
    Get-ChildItem $deploy -Recurse -File | Where-Object {
        $_.FullName -notmatch '\\__pycache__\\' -and $_.Name -notmatch '^QElectroTech_.+\.exe$'
    } | ForEach-Object {
        $rel = $_.FullName.Substring($deploy.Length + 1)
        [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $_.FullName, $rel, "Optimal") | Out-Null
        $n++
    }
} finally { $zip.Dispose() }
Copy-Item $tmp (Join-Path $programm $name) -Force
Remove-Item $tmp -Force
Write-Host "  Programm: $n Dateien in Programm\$name"
# nur die letzten drei Pakete behalten
Get-ChildItem $programm -Filter "QET-Fork-*.zip" | Sort-Object LastWriteTime -Descending | Select-Object -Skip 3 | Remove-Item -Force

# 2. Installierskript
Copy-Item (Join-Path $PSScriptRoot "Installieren.ps1") $Ziel -Force
Copy-Item (Join-Path $PSScriptRoot "Installieren.cmd") $Ziel -Force

# 3. Bauteile (Spiegel; im Ziel gelöschte Bauteile verschwinden auch dort)
if (-not $OhneBauteile) {
    if (-not $Bauteile) { $Bauteile = Einstellung "elements-collections" "company-collection-path" }
    $ziel_bauteile = Join-Path $Ziel "Bauteile"
    if (-not $Bauteile -or -not (Test-Path $Bauteile)) { throw "Firmensammlung nicht gefunden: -Bauteile angeben oder -OhneBauteile." }
    if ((Resolve-Path $Bauteile).Path -eq $ziel_bauteile) { throw "Die Firmensammlung ist schon der Verteilordner: -Bauteile auf die eigene Sammlung zeigen lassen." }
    robocopy $Bauteile $ziel_bauteile /MIR /XD .git /NFL /NDL /NJH /NP | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "Robocopy der Bauteile meldet Fehler ($LASTEXITCODE)." }
    Write-Host "  Bauteile: $Bauteile gespiegelt"
}

# 4. Meldungen
New-Item -ItemType Directory -Force (Join-Path $Ziel "Meldungen") | Out-Null

# 5. version.json zuletzt
$info = [ordered]@{ product = "QET-Fork"; version = $version; package = "Programm/$name"; notes = $Hinweis;
                    published = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ss") }
$json = $info | ConvertTo-Json
[System.IO.File]::WriteAllText((Join-Path $Ziel "version.json"), $json, (New-Object System.Text.UTF8Encoding $false))
Write-Host "  version.json: $version - die Kollegen bekommen das Update beim nächsten Start"
