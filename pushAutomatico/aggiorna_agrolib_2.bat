@echo off
setlocal enabledelayedexpansion

:: === CONFIG ===
set "INFILE=%~dp0differenze.txt"
set "OUTFILE=%~dp0differenze2.txt"

echo Creazione di %OUTFILE% da %INFILE%...

:: Verifica se il file di input esiste
if not exist "%INFILE%" (
    echo Errore: Il file "%INFILE%" non trovato.
    goto :eof
)

:: Cancella il file di output se esiste già, per assicurarsi che sia pulito
if exist "%OUTFILE%" del "%OUTFILE%"

:: Legge ogni riga dal file di input e scrive solo quelle che NON contengono ".git" nel file di output
for /f "usebackq delims=" %%L in ("%INFILE%") do (
    set "line=%%L"
    :: Controlla se la riga NON contiene ".git" (ignorando maiuscole/minuscole)
    echo !line! | findstr /i /c:".git" >nul
    if errorlevel 1 (
        echo !line! >> "%OUTFILE%"
    )
)

:: === AGGIUNGI COMANDI GIT ===
::echo. >> "%OUTFILE%"
echo git commit -am "aggiornamento files" >> "%OUTFILE%"
echo git push >> "%OUTFILE%"

echo ✅ File "%OUTFILE%" creato e comandi Git aggiunti con successo.

REM Copia differenze2.txt su differenze.txt
copy differenze2.txt differenze.txt

REM Cancella differenze2.txt
del differenze2.txt