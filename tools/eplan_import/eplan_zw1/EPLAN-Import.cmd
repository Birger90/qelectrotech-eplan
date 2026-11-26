@echo off
rem EPLAN-Projektarchiv (.zw1) nach QElectroTech importieren: .zw1-Datei auf diese Datei ziehen.
rem Ergebnis: <Ordner des Archivs>\QET\<Plannummer>_EPLAN_Import.qet (+ PDF), danach wird es in QET geoeffnet.
if "%~1"=="" (echo Bitte eine .zw1-Datei auf diese Datei ziehen. & pause & exit /b 1)
if "%QET_DATEN%"=="" set "QET_DATEN=%USERPROFILE%\Documents\Code Projekte\QElectroTech\Daten"
set "QET_COMPANY_ELEMENTS_DIR=%QET_DATEN%\elements-company"
set PYTHONIOENCODING=utf-8
python "%~dp0eplan_nach_qet.py" "%~1" --pdf --oeffnen --dataportal
if errorlevel 1 (echo Import fehlgeschlagen. & pause & exit /b 1)
pause
