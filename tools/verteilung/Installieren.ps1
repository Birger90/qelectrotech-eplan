# Installiert oder aktualisiert den QET-Fork aus dem Verteilordner (SharePoint-Bibliothek, mit OneDrive
# synchronisiert). Liegt im Verteilordner neben version.json; Doppelklick auf Installieren.cmd.
#
#   Installieren.cmd                            erste Installation oder Neuinstallation
#   Installieren.ps1 -Update -WartenAuf <PID>   so ruft das Programm es beim Update auf
#
# Ziel: %LOCALAPPDATA%\QET-Fork\Programm (ohne Adminrechte); die vorige Version bleibt als Programm.alt.
#   -Basis <Ordner> -Probelauf   zum Testen: anderer Zielordner, keine Einstellung, keine Verknüpfung, kein Start
param(
    [switch]$Update,
    [int]$WartenAuf = 0,
    [string]$Basis = "",
    [switch]$Probelauf
)
$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.IO.Compression.FileSystem

function Melden([string]$text, [string]$art = "Information") {
    if ($Probelauf) { Write-Host $text; return }
    [System.Windows.Forms.MessageBox]::Show($text, "QET-Fork", "OK", $art) | Out-Null
}

try {
    $quelle = $PSScriptRoot
    $info = Get-Content (Join-Path $quelle "version.json") -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($info.product -ne "QET-Fork") { throw "version.json gehört nicht zum QET-Fork." }
    $paket = Join-Path $quelle $info.package
    if (-not (Test-Path $paket)) { throw "Das Programmpaket $paket fehlt (noch nicht synchronisiert?)." }

    $basis = if ($Basis) { $Basis } else { Join-Path $env:LOCALAPPDATA "QET-Fork" }
    $programm = Join-Path $basis "Programm"
    New-Item -ItemType Directory -Force $basis | Out-Null

    # das laufende Programm beendet sich beim Update selbst; ein anderes Fenster muss der Nutzer schließen
    if ($WartenAuf -gt 0) {
        Wait-Process -Id $WartenAuf -ErrorAction SilentlyContinue
    }
    while (Get-Process QElectroTech -ErrorAction SilentlyContinue | Where-Object { $_.Path -like "$programm\*" }) {
        $antwort = [System.Windows.Forms.MessageBox]::Show(
            "QElectroTech läuft noch. Bitte schließen und dann OK drücken.", "QET-Fork", "OKCancel", "Warning")
        if ($antwort -ne "OK") { exit 1 }
    }

    # OneDrive hält das Paket evtl. nur online: Kopieren lädt es herunter
    $zip = Join-Path $env:TEMP ("QET-Fork-" + $info.version + ".zip")
    Copy-Item $paket $zip -Force
    $neu = Join-Path $basis "Programm.neu"
    if (Test-Path $neu) { Remove-Item $neu -Recurse -Force }
    [System.IO.Compression.ZipFile]::ExtractToDirectory($zip, $neu)
    Remove-Item $zip -Force
    if (-not (Test-Path (Join-Path $neu "QElectroTech.exe"))) { throw "Im Paket fehlt QElectroTech.exe." }

    $alt = Join-Path $basis "Programm.alt"
    if (Test-Path $programm) {
        if (Test-Path $alt) { Remove-Item $alt -Recurse -Force }
        Rename-Item $programm "Programm.alt"
    }
    Rename-Item $neu "Programm"

    if ($Probelauf) { Write-Host "Probelauf: installiert nach $programm"; exit 0 }

    # QElectroTech merkt sich den Verteilordner (Einstellung distribution/folder)
    $schluessel = "HKCU:\Software\QElectroTech\QElectroTech\distribution"
    New-Item -Force $schluessel | Out-Null
    Set-ItemProperty $schluessel -Name "folder" -Value ($quelle -replace "\\", "/")

    # Verknüpfungen im Startmenü und auf dem Desktop
    $exe = Join-Path $programm "QElectroTech.exe"
    $shell = New-Object -ComObject WScript.Shell
    foreach ($ort in @([Environment]::GetFolderPath("Programs"), [Environment]::GetFolderPath("Desktop"))) {
        $lnk = $shell.CreateShortcut((Join-Path $ort "QElectroTech (Firma).lnk"))
        $lnk.TargetPath = $exe
        $lnk.WorkingDirectory = $programm
        $lnk.Description = "QElectroTech-Fork " + $info.version
        $lnk.Save()
    }

    if (-not $Update) {
        Melden ("QElectroTech-Fork " + $info.version + " ist installiert. Neue Versionen meldet das Programm beim Start.")
    }
    Start-Process $exe -WorkingDirectory $programm
}
catch {
    Melden ("Installation abgebrochen: " + $_.Exception.Message) "Error"
    exit 1
}
