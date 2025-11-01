@echo off
REM Script para iniciar Zone Server na porta 8082 (Zone ID 0)
REM Este script deve ser executado ANTES de iniciar o cliente Unreal

echo =========================================
echo   Iniciando Zone Server (Porta 8082)
echo =========================================
echo.

REM Verificar se o executável existe
if not exist "build\bin\Release\zone_server.exe" (
    echo ERRO: zone_server.exe nao encontrado!
    echo Por favor, compile o projeto primeiro.
    pause
    exit /b 1
)

REM Navegar para o diretório correto
cd /d "%~dp0.."

REM Criar diretório de logs se não existir
if not exist "build\bin\Release\logs" mkdir "build\bin\Release\logs"

echo Iniciando Zone Server com Zone ID 0 (porta 8082)...
echo.
echo IMPORTANTE:
echo - Porta configurada: 8082
echo - WebSocket URL: ws://127.0.0.1:8082
echo - Mantenha esta janela aberta enquanto testar o cliente
echo.
echo Pressione Ctrl+C para parar o servidor
echo.

REM Executar o Zone Server com Zone ID 0 (porta 8082)
cd build\bin\Release
zone_server.exe 0

pause

