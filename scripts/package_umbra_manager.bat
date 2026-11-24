@echo off
REM ============================================================================
REM  package_umbra_manager.bat
REM  Empacota o Umbra Manager (WPF .NET 8): um unico .exe autocontido (win-x64)
REM  que roda em qualquer Windows 10/11 sem instalar o .NET.
REM
REM  Uso:
REM    package_umbra_manager.bat            ->  publica em dist\UmbraManager\
REM    package_umbra_manager.bat --nopause  ->  sem pausa no fim (automacao)
REM
REM  Saida: dist\UmbraManager\UmbraManager.exe (~85 MB comprimido) + config\ de exemplo.
REM  Feche o Umbra Manager antes (o exe aberto fica travado).
REM ============================================================================
setlocal EnableExtensions

set "ROOT=%~dp0.."
set "WPF_DIR=%ROOT%\tools\UmbraManagerWpf"
set "DIST_DIR=%ROOT%\dist\UmbraManager"
set "NOPAUSE=0"
if /i "%~1"=="--nopause" set "NOPAUSE=1"

where dotnet >nul 2>&1
if errorlevel 1 (
  echo [ERRO] dotnet SDK nao encontrado. Instale .NET 8 SDK.
  goto fail
)

tasklist /FI "IMAGENAME eq UmbraManager.exe" 2>nul | find /I "UmbraManager.exe" >nul
if not errorlevel 1 (
  echo [ERRO] O Umbra Manager esta aberto. Feche-o e rode de novo.
  goto fail
)

echo Publicando Umbra Manager para %DIST_DIR% ...
if exist "%DIST_DIR%" rmdir /s /q "%DIST_DIR%"
mkdir "%DIST_DIR%"

REM IncludeNativeLibrariesForSelfExtract: SkiaSharp (grafico) e SQLite dentro do exe.
dotnet publish "%WPF_DIR%\UmbraManager\UmbraManager.csproj" -c Release -r win-x64 --self-contained true ^
  -p:PublishSingleFile=true ^
  -p:IncludeNativeLibrariesForSelfExtract=true ^
  -p:EnableCompressionInSingleFile=true ^
  -p:DebugType=none ^
  -o "%DIST_DIR%"
if errorlevel 1 (
  echo [ERRO] dotnet publish falhou.
  goto fail
)

mkdir "%DIST_DIR%\config" 2>nul
copy /Y "%ROOT%\config\manager.json.example" "%DIST_DIR%\config\manager.json.example" >nul
if exist "%ROOT%\config\manager.json" copy /Y "%ROOT%\config\manager.json" "%DIST_DIR%\config\manager.json" >nul
copy /Y "%ROOT%\config\admin.key.example" "%DIST_DIR%\config\admin.key.example" >nul

echo [OK] Dist em %DIST_DIR%\UmbraManager.exe
if "%NOPAUSE%"=="0" pause
endlocal
exit /b 0

:fail
if "%NOPAUSE%"=="0" pause
endlocal
exit /b 1
