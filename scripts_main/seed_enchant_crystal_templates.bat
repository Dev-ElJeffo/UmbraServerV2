@echo off
REM Cria/atualiza Extrator + templates "Cristal de Teste — {stat}" (sem player_id / sem bag).
SETLOCAL

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

SET SCRIPT=%~dp0..\www\umbra_api\scripts\seed_enchant_crystal_templates.php
IF NOT EXIST "%SCRIPT%" (
  echo Script nao encontrado: %SCRIPT%
  EXIT /B 1
)

"%PHP%" "%SCRIPT%"
ENDLOCAL
