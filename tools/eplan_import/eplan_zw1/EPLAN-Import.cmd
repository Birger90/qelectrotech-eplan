@echo off
rem EPLAN-Projektarchiv (.zw1) nach QElectroTech importieren: .zw1-Datei auf diese Datei ziehen.
rem Ergebnis: <Ordner des Archivs>\QET\<Plannummer>_EPLAN_Import.qet (+ PDF), danach wird es in QET geoeffnet.
if "%~1"=="" (echo Bitte eine .zw1-Datei auf diese Datei ziehen. & pause & exit /b 1)
set QET_COMPANY_ELEMENTS_DIR=C:\Users\test\Documents\Code Projekte\QElectroTech\Daten\elements-company
set PYTHONIOENCODING=utf-8
"C:\Users\test\AppData\Local\Programs\Python\Python311\python.exe" "C:\Users\test\Documents\Code Projekte\QElectroTech\Daten\vorlagen\eplan_zw1\eplan_nach_qet.py" "%~1" --pdf --oeffnen --dataportal
if errorlevel 1 (echo Import fehlgeschlagen. & pause & exit /b 1)
pause
