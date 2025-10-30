@echo off
echo ========================================
echo Executando Teste PlayerDAO Parsing
echo ========================================
echo.

REM Adicionar DLLs ao PATH
set "PATH=C:\Program Files\MySQL\MySQL Server 8.0\lib;C:\Program Files\MySQL\MySQL Server 8.0\bin;C:\Program Files\OpenSSL-Win64\bin;%PATH%"

cd build\bin\Release

echo Executando teste...
echo.

REM Criar diretório de logs se não existir
if not exist logs mkdir logs

test_playerdao.exe

if %errorlevel% equ 0 (
    echo.
    echo Teste concluido com sucesso!
) else (
    echo.
    echo Ocorreu um erro durante o teste. Verifique o log.
)

pause


