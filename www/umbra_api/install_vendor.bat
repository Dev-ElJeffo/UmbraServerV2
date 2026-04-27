@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

echo ============================================
echo  Umbra API - Composer install (vendor/)
echo  Pasta: %CD%
echo ============================================
echo.

set "PHP_EXE="

where php >nul 2>&1
if not errorlevel 1 (
  for /f "delims=" %%A in ('where php 2^>nul') do (
    set "PHP_EXE=%%A"
    goto :found_php
  )
)

echo PHP nao esta no PATH. A procurar em pastas tipicas do WAMP...
for /d %%D in ("C:\wamp64\bin\php\php*") do (
  if exist "%%~D\php.exe" set "PHP_EXE=%%~D\php.exe"
)
if not defined PHP_EXE for /d %%D in ("C:\wamp\bin\php\php*") do (
  if exist "%%~D\php.exe" set "PHP_EXE=%%~D\php.exe"
)
if not defined PHP_EXE for /d %%D in ("D:\wamp64\bin\php\php*") do (
  if exist "%%~D\php.exe" set "PHP_EXE=%%~D\php.exe"
)

:found_php
if not defined PHP_EXE (
  echo ERRO: Nao encontrei php.exe.
  echo Adiciona o PHP do WAMP ao PATH do Windows, ou edita este .bat e define manualmente:
  echo   set "PHP_EXE=C:\wamp64\bin\php\php8.x.x\php.exe"
  echo.
  pause
  exit /b 1
)

echo Usando: "%PHP_EXE%"
"%PHP_EXE%" -v
echo.

if exist "vendor\autoload.php" (
  echo vendor\autoload.php ja existe. A atualizar dependencias...
) else (
  echo vendor\ ainda nao existe. A instalar dependencias...
)

if not exist "composer.phar" (
  echo A descarregar composer.phar...
  powershell -NoProfile -ExecutionPolicy Bypass -Command "try { Invoke-WebRequest -Uri 'https://getcomposer.org/download/latest-stable/composer.phar' -OutFile '%CD%\composer.phar' -UseBasicParsing } catch { exit 1 }"
  if errorlevel 1 (
    echo ERRO: Falha ao descarregar composer.phar. Verifica rede/firewall.
    pause
    exit /b 1
  )
)

if not exist "composer.json" (
  echo ERRO: composer.json nao encontrado nesta pasta.
  pause
  exit /b 1
)

echo A executar: composer install
echo.
"%PHP_EXE%" "%CD%\composer.phar" install --no-interaction
if errorlevel 1 (
  echo.
  echo ERRO: composer install falhou. Copia a mensagem acima.
  pause
  exit /b 1
)

if not exist "vendor\autoload.php" (
  echo ERRO: vendor\autoload.php nao foi criado.
  pause
  exit /b 1
)

echo.
echo OK: vendor\autoload.php criado. Reinicia o Apache no WAMP.
echo.
pause
endlocal
exit /b 0
