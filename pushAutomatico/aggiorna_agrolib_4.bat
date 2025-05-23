@echo off
setlocal enabledelayedexpansion

:: === CONFIG ===
set "GIT_PATH=C:\Users\avolta\AppData\Local\Programs\Git\bin"
set "REPO_DIR=C:\Github\PROVAFEDEGRAZZINI"
set "COMMANDS_FILE=%~dp0differenze.txt"

:: === AGGIUNGI GIT TEMPORANEAMENTE AL PATH ===
set "PATH=%GIT_PATH%;%PATH%"

:: === VALIDAZIONI ===
if not exist "%GIT_PATH%\git.exe" (
    echo ❌ Git non trovato in: %GIT_PATH%
    pause
    exit /b
)

if not exist "%COMMANDS_FILE%" (
    echo ❌ File comandi non trovato: %COMMANDS_FILE%
    pause
    exit /b
)

:: === ESECUZIONE COMANDI ===
pushd "%REPO_DIR%"
for /f "usebackq delims=" %%C in ("%COMMANDS_FILE%") do (
    echo ▶ Eseguo: %%C
    %%C
)
popd
del C:\Github\differenze.txt
echo ✅ Tutti i comandi sono stati eseguiti.

