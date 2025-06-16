@echo off
setlocal enabledelayedexpansion

:: === CONFIG ===
set "SOURCE=C:\agrolib2"
set "TARGET=C:\Github\PROVAFEDEGRAZZINI"
set "SCRIPT=%~dp0git_update.txt"

echo ================================================
echo 🔄 Aggiornamento da %SOURCE% a %TARGET%
echo Generazione script: %SCRIPT%
echo ================================================

:: Pulizia script precedente
if exist "%SCRIPT%" del "%SCRIPT%"

:: === Copia file (escludendo .git e simili) ===
robocopy "%SOURCE%" "%TARGET%" /MIR /XD ".git" ".vs" ".idea" /XF Thumbs.db .DS_Store /NFL /NDL /NJH /NJS /NP >nul



