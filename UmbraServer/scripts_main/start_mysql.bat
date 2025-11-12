@echo off
REM UmbraEternum - Iniciar MySQL 8.0
REM Execute como Administrador (Right-click → Run as Administrator)

echo =========================================
echo   UmbraEternum - Iniciar MySQL
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

REM Verificar se MySQL80 existe
sc query MySQL80 >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERRO] Servico MySQL80 nao encontrado!
    echo.
    echo Verifique se MySQL 8.0 esta instalado em:
    echo   C:\Program Files\MySQL\MySQL Server 8.0
    echo.
    pause
    exit /b 1
)

REM Verificar status atual
sc query MySQL80 | find "RUNNING" >nul
if %errorLevel% equ 0 (
    echo [INFO] MySQL ja esta rodando
    echo.
) else (
    echo [1/2] Iniciando MySQL Server...
    net start MySQL80 >nul 2>&1
    REM Aguardar alguns segundos para o servico iniciar
    timeout /t 5 /nobreak >nul
    
    REM Verificar se realmente iniciou (nao confiar apenas no errorLevel)
    sc query MySQL80 | find "RUNNING" >nul
    if %errorLevel% equ 0 (
        echo [OK] MySQL iniciado com sucesso!
        echo.
    ) else (
        echo [ERRO] Falha ao iniciar MySQL
        echo.
        echo Verificando status do servico...
        sc query MySQL80 | find "STATE"
        echo.
        echo Tente iniciar manualmente:
        echo   Win+R -> services.msc -> MySQL80 -> Start
        echo.
        pause
        exit /b 1
    )
)

REM Testar conexao
echo [2/2] Testando conexao...
echo.

"C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe" -u root -p -e "SELECT VERSION();" 2>nul
if %errorLevel% equ 0 (
    echo [OK] MySQL conectado com sucesso!
    echo.
) else (
    echo [AVISO] Nao foi possivel testar conexao automaticamente
    echo   (Pode ser necessario digitar a senha)
    echo.
)

echo =========================================
echo   MySQL esta pronto para uso!
echo =========================================
echo.
pause

