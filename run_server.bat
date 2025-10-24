@echo off
REM Run UmbraEternum Server from build directory

echo =========================================
echo   UmbraEternum Server Launcher
echo =========================================
echo.

cd /d "%~dp0build\bin\Release"

if not exist "umbra_server.exe" (
    echo [ERRO] umbra_server.exe nao encontrado!
    echo.
    echo Execute primeiro:
    echo   cd build
    echo   cmake --build . --config Release
    echo.
    pause
    exit /b 1
)

echo [INFO] Verificando configuracoes...

REM Criar diretorios necessarios
if not exist "config" mkdir config
if not exist "logs" mkdir logs

REM Copiar configs se nao existirem
if not exist "config\server.json" (
    echo [INFO] Copiando server.json...
    copy /Y "..\..\..\config\server.json" "config\" >nul
)

if not exist "config\db.json" (
    echo [INFO] Copiando db.json...
    copy /Y "..\..\..\config\db.json" "config\" >nul
)

if not exist "config\jwt_secret.key" (
    echo [INFO] Copiando jwt_secret.key...
    copy /Y "..\..\..\config\jwt_secret.key" "config\" >nul
)

echo [OK] Configuracoes prontas
echo.
echo =========================================
echo   Iniciando servidor...
echo =========================================
echo.

umbra_server.exe

echo.
echo =========================================
echo   Servidor encerrado
echo =========================================
echo.
pause

