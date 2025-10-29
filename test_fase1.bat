@echo off
echo ========================================
echo Teste das Implementacoes Fase 1
echo ========================================
echo.

cd build

if not exist "bin\Release\test_fase1.exe" (
    echo Compilando teste...
    cmake --build . --config Release --target test_fase1
    if errorlevel 1 (
        echo ERRO: Falha ao compilar teste
        pause
        exit /b 1
    )
)

echo.
echo Executando testes...
echo.

bin\Release\test_fase1.exe

if errorlevel 1 (
    echo.
    echo ERRO: Alguns testes falharam
    pause
    exit /b 1
) else (
    echo.
    echo Todos os testes passaram!
)

pause

