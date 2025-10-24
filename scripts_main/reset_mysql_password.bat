@echo off
REM Reset MySQL Root Password
REM Execute como Administrador

echo =========================================
echo   Reset MySQL Root Password
echo =========================================
echo.

net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERRO] Execute como Administrador!
    pause
    exit /b 1
)

echo [1/4] Parando MySQL80...
net stop MySQL80
timeout /t 2 >nul
echo.

echo [2/4] Iniciando MySQL em modo seguro...
echo ATENCAO: Isso pode levar alguns segundos...
echo.

cd "C:\Program Files\MySQL\MySQL Server 8.0\bin"

start /B mysqld --console --skip-grant-tables --shared-memory

timeout /t 5 >nul

echo [3/4] Resetando senha do root...
echo.

mysql -u root --skip-password -e "FLUSH PRIVILEGES; ALTER USER 'root'@'localhost' IDENTIFIED BY '';"

echo.
echo [4/4] Parando MySQL seguro e reiniciando normal...

taskkill /F /IM mysqld.exe >nul 2>&1
timeout /t 3 >nul

net start MySQL80

if %errorLevel% equ 0 (
    echo.
    echo =========================================
    echo   [OK] SENHA RESETADA! (senha vazia)
    echo =========================================
    echo.
    echo Agora no Workbench:
    echo 1. Deixe o campo Password VAZIO
    echo 2. Ou clique "Clear" em "Store in Vault"
    echo 3. Test Connection novamente
    echo.
) else (
    echo [ERRO] Falha ao reiniciar MySQL80
)

pause

