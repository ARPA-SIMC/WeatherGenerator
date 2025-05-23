@echo off
setlocal enabledelayedexpansion

:: === CONFIG ===
set "DIR1=C:\agrolib2"
set "DIR2=C:\Github\PROVAFEDEGRAZZINI"
set "OUTFILE=%~dp0differenze.txt"

echo Confronto file tra:
echo   %DIR1%
echo   %DIR2%
echo Risultati in: %OUTFILE%

if exist "%OUTFILE%" del "%OUTFILE%"

:: === CREA LISTA FILE NON NASCOSTI ===
dir /b /s /a:-h "%DIR1%" > "%~dp0files1.txt"
dir /b /s /a:-h "%DIR2%" > "%~dp0files2.txt"

:: === CONFRONTO ===
for /f "usebackq delims=" %%F in ("%~dp0files1.txt") do (
    set "full1=%%~fF"
    set "rel=%%~F"
    set "relpath=!full1:%DIR1%\=!"
    set "file2=%DIR2%\!relpath!"

    if exist "!file2!" (
        fc /b "!full1!" "!file2!" >nul
        if errorlevel 1 (
            echo git add !relpath!>> "%OUTFILE%"
        )
    ) else (
        echo git add !relpath!>> "%OUTFILE%"
    )
)

for /f "usebackq delims=" %%F in ("%~dp0files2.txt") do (
    set "full2=%%~fF"
    set "relpath=!full2:%DIR2%\=!"
    if not exist "%DIR1%\!relpath!" (
        echo git rm !relpath!>> "%OUTFILE%"
    )
)

:: === PULIZIA ===
del "%~dp0files1.txt"
del "%~dp0files2.txt"

echo ✅ Confronto completato. Vedi: %OUTFILE%




