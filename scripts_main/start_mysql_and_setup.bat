@echo off
REM UmbraEternum - Start MySQL and Setup Database
REM Execute como Administrador (Right-click → Run as Administrator)

echo =========================================
echo   UmbraEternum - MySQL Quick Setup
echo =========================================
echo.

REM Verificar admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERRO] Execute como Administrador!
    echo.
    echo Como executar:
    echo 1. Right-click neste arquivo
    echo 2. Selecione "Executar como Administrador"
    echo.
    pause
    exit /b 1
)

echo [OK] Executando como Administrador
echo.

REM Iniciar MySQL
echo [1/3] Iniciando MySQL Server...
net start MySQL80 >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] MySQL iniciado com sucesso
) else (
    sc query MySQL80 | find "RUNNING" >nul
    if %errorLevel% equ 0 (
        echo [OK] MySQL ja estava rodando
    ) else (
        echo [ERRO] Falha ao iniciar MySQL
        echo Verifique se MySQL 8.0 esta instalado
        pause
        exit /b 1
    )
)
echo.

REM Parar WAMP se estiver rodando
echo [2/3] Parando WAMP MySQL (se existir)...
net stop wampmysqld64 >nul 2>&1
echo [OK] WAMP verificado
echo.

REM Executar setup
echo [3/3] Criando banco de dados...
echo.
echo Digite a senha do MySQL root quando solicitado
echo (Se nao tiver senha, apenas pressione Enter)
echo.

"C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" -u root -p < setup_database.sql

if %errorLevel% equ 0 (
    echo.
    echo =========================================
    echo   [OK] BANCO DE DADOS CRIADO!
    echo =========================================
    echo.
    echo Verificando instalacao...
    echo.
    "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" -u root -p -e "USE umbra_eternum; SHOW TABLES; SELECT * FROM schema_version;"
    echo.
    echo =========================================
    echo   PRONTO PARA USAR!
    echo =========================================
    echo.
    echo Proximo passo:
    echo   cd build\bin\Release
    echo   umbra_server.exe
    echo.
) else (
    echo.
    echo [ERRO] Falha ao criar banco de dados
    echo Verifique a senha e tente novamente
    echo.
)

pause

