@echo off
echo ========================================
echo Executando Teste Fase 1
echo ========================================
echo.

REM Adicionar DLLs ao PATH
set "PATH=C:\Program Files\MySQL\MySQL Server 8.0\lib;C:\Program Files\MySQL\MySQL Server 8.0\bin;C:\Program Files\OpenSSL-Win64\bin;%PATH%"

cd build\bin\Release

echo Executando teste...
echo.
test_fase1.exe

if errorlevel 1 (
    echo.
    echo ERRO: Teste falhou com codigo %ERRORLEVEL%
    pause
    exit /b 1
) else (
    echo.
    echo Teste concluido com sucesso!
    pause
)

