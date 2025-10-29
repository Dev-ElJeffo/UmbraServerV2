@echo off
echo ========================================
echo Executando Teste de Integracao Fase 1
echo ========================================
echo.

REM Adicionar DLLs ao PATH
set "PATH=C:\Program Files\MySQL\MySQL Server 8.0\lib;C:\Program Files\MySQL\MySQL Server 8.0\bin;C:\Program Files\OpenSSL-Win64\bin;%PATH%"

cd build\bin\Release

echo Testando servidor principal...
echo.

REM Criar diretório de logs se não existir
if not exist logs mkdir logs

REM Copiar config se necessário
if not exist config mkdir config
copy ..\..\..\config\server.json config\ 2>nul

echo Iniciando servidor por 5 segundos para teste basico...
start /B umbra_server.exe
timeout /t 5 /nobreak
taskkill /F /IM umbra_server.exe 2>nul

echo.
echo Teste concluido!
echo Verifique logs/umbra_server.log para detalhes

pause

