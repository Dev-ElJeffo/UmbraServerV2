@echo off
chcp 65001 > nul
title Corrigir MySQL - UmbraEternum

echo.
echo =========================================
echo   CORRIGIR MYSQL - UmbraEternum
echo =========================================
echo.

REM Verificar Admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERRO] Execute como Administrador!
    echo        Clique com botão direito e escolha "Executar como Administrador"
    echo.
    pause
    exit /b 1
)

echo [OK] Executando como Administrador
echo.

echo [1/3] Executando diagnóstico...
echo.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0diagnostico_mysql.ps1"

echo.
pause

