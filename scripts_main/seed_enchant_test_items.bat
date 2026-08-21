@echo off
REM Cria Extrator + 1 cristal de teste por status (enchant_stat_weights).
REM Opcional: seed_enchant_test_items.bat 1 20   (player_id e quantidade na bag)
SETLOCAL
SET PLAYER_ID=%~1
SET QTY=%~2
IF "%QTY%"=="" SET QTY=20

SET PHP=
IF EXIST "C:\wamp64\bin\php\php8.3.14\php.exe" SET PHP=C:\wamp64\bin\php\php8.3.14\php.exe
IF EXIST "C:\wamp64\bin\php\php8.4.0\php.exe" SET PHP=C:\wamp64\bin\php\php8.4.0\php.exe
IF "%PHP%"=="" (
  WHERE php >NUL 2>&1 && SET PHP=php
)
IF "%PHP%"=="" (
  echo Nao achei php.exe. Instale PHP no PATH ou ajuste este .bat.
  EXIT /B 1
)

SET SCRIPT=%~dp0..\www\umbra_api\scripts\seed_enchant_test_items.php
IF NOT EXIST "%SCRIPT%" (
  echo Script nao encontrado: %SCRIPT%
  EXIT /B 1
)

IF "%PLAYER_ID%"=="" (
  "%PHP%" "%SCRIPT%"
) ELSE (
  "%PHP%" "%SCRIPT%" --player_id=%PLAYER_ID% --qty=%QTY%
)
ENDLOCAL
