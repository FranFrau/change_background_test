@echo off
REM ======================================================
REM Script Batch (.bat) per scaricare un'immagine e
REM impostarla come sfondo su Windows.
REM
REM Uso:
REM   imposta_sfondo.bat
REM ======================================================

:: URL dell'immagine da scaricare
set "URL_IMMAGINE=https://media.istockphoto.com/id/1352010441/it/foto/coppia-di-gattini-addormentati-innamorati-il-giorno-di-san-valentino-i-nasi-dei-gatti-si.jpg?s=612x612&w=0&k=20&c=bd60KCbS1ImSwQbdND-8c3uGUVGK9VLrDPMQ-ebAJ0Q="
:: Nome file da salvare
set "IMG_NAME=sfondo_gattini.jpg"

:: Scegli una cartella di destinazione. Qui la cartella corrente:
set "IMG_PATH=%cd%\%IMG_NAME%"

echo Scarico l'immagine con PowerShell...
powershell -Command ^
  "(New-Object System.Net.WebClient).DownloadFile('%URL_IMMAGINE%', '%IMG_PATH%')" ^
  || (echo Errore durante il download & exit /b 1)

echo Immagine scaricata in: %IMG_PATH%

:: Imposta lo sfondo. Ci sono diversi modi.
:: Uno dei più comuni è usare le chiavi di registro e poi forzare l'aggiornamento.

echo Imposto sfondo come: %IMG_PATH%
reg add "HKCU\Control Panel\Desktop" /v Wallpaper /t REG_SZ /d "%IMG_PATH%" /f

:: Forza l'aggiornamento dello sfondo. 
:: - SystemParametersInfoW(20, 0, path, 0x01 | 0x02) in C, 
::   qui usiamo RUNDLL32 per chiamare la funzione nativa.
rundll32.exe user32.dll, UpdatePerUserSystemParameters 
:: In alcuni casi potrebbe servire:
:: rundll32.exe user32.dll, SystemParametersInfoW 20, 0, %IMG_PATH%, 0

echo Operazione completata. Premi un tasto per uscire.
pause
