@echo off
rem Installiert den QElectroTech-Fork aus diesem Ordner (Doppelklick genuegt, keine Adminrechte noetig).
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Installieren.ps1" %*
