@echo off
echo ============================================
echo    UmbraEternum Server - Build e Teste
echo ============================================
echo.

REM Verificar se estamos no diretório correto
if not exist "src\main.cpp" (
    echo ERRO: Execute este script no diretório UmbraServer
    pause
    exit /b 1
)

REM Criar diretório de build se não existir
if not exist "build" mkdir build
cd build

echo [1/4] Configurando projeto com CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo ERRO: Falha na configuração do CMake
    pause
    exit /b 1
)

echo.
echo [2/4] Compilando projeto...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo ERRO: Falha na compilação
    pause
    exit /b 1
)

echo.
echo [3/4] Verificando executáveis...
if exist "bin\Release\umbra_server.exe" (
    echo ✅ umbra_server.exe criado
) else (
    echo ❌ umbra_server.exe não encontrado
)

if exist "bin\Release\auth_server.exe" (
    echo ✅ auth_server.exe criado
) else (
    echo ❌ auth_server.exe não encontrado
)

if exist "bin\Release\gateway_server.exe" (
    echo ✅ gateway_server.exe criado
) else (
    echo ❌ gateway_server.exe não encontrado
)

echo.
echo [4/4] Executando testes de integração TCP...
cd ..
python test_tcp_integration.py
if %ERRORLEVEL% neq 0 (
    echo ERRO: Testes falharam
    pause
    exit /b 1
)

echo.
echo ============================================
echo ✅ BUILD E TESTES CONCLUÍDOS COM SUCESSO!
echo ============================================
echo.
echo Para executar o servidor:
echo   cd build\bin\Release
echo   umbra_server.exe
echo.
echo Para executar servidores individuais:
echo   auth_server.exe
echo   gateway_server.exe
echo.
pause
