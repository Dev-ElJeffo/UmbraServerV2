@echo off
REM Fix MySQL Port Conflict
REM Execute como Administrador

echo =========================================
echo   Fix MySQL Port 3306 Conflict
echo =========================================
echo.

REM Verificar admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERRO] Execute como Administrador!
    echo Right-click -^> "Executar como Administrador"
    pause
    exit /b 1
)

echo [1/4] Parando todos os servicos MySQL/MariaDB...
echo.

REM Parar WAMP
net stop wampmysqld64 >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] WAMP MySQL parado
) else (
    echo [  ] WAMP MySQL nao estava rodando
)

REM Parar MySQL80
net stop MySQL80 >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] MySQL80 parado
) else (
    echo [  ] MySQL80 nao estava rodando
)

echo.
echo [2/4] Finalizando processos mysqld.exe...
taskkill /F /IM mysqld.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] Processos mysqld.exe finalizados
) else (
    echo [  ] Nenhum processo mysqld.exe encontrado
)

echo.
echo [3/4] Aguardando liberacao da porta 3306...
timeout /t 3 >nul
echo [OK] Porta deve estar livre agora

echo.
echo [4/4] Iniciando MySQL80...
net start MySQL80

if %errorLevel% equ 0 (
    echo.
    echo =========================================
    echo   [OK] MySQL80 INICIADO COM SUCESSO!
    echo =========================================
    echo.
    echo Verificando...
    timeout /t 2 >nul
    sc query MySQL80 | find "RUNNING"
    
    echo.
    echo Proximo passo:
    echo 1. Abra MySQL Workbench
    echo 2. Conecte em "Local instance MySQL80"
    echo 3. Execute: File -^> Open SQL Script
    echo 4. Arquivo: setup_database.sql
    echo.
) else (
    echo.
    echo [ERRO] Falha ao iniciar MySQL80
    echo.
    echo Verifique os logs em:
    echo C:\ProgramData\MySQL\MySQL Server 8.0\Data\*.err
    echo.
)

pause

