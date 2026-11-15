@echo off
setlocal
REM Wrapper: eleva UAC e mantem a janela aberta para ler sucesso/erro.
REM Preferivel ao .ps1 no Explorer (duplo clique).

cd /d "%~dp0"

net session >nul 2>&1
if %errorlevel% NEQ 0 (
  echo Solicitando privilegios de Administrador...
  powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
  exit /b 0
)

echo.
set "UMBRA_DUMPS_FROM_BAT=1"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0enable_zone_crash_dumps.ps1"
set "ERR=%ERRORLEVEL%"
set "UMBRA_DUMPS_FROM_BAT="

echo.
pause
exit /b %ERR%
