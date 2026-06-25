@echo off
REM Empacota UmbraManager WPF (.NET 8 single-file self-contained)
setlocal

set ROOT=%~dp0..
set WPF_DIR=%ROOT%\tools\UmbraManagerWpf
set DIST_DIR=%ROOT%\dist\UmbraManager

where dotnet >nul 2>&1
if errorlevel 1 (
  echo [ERRO] dotnet SDK nao encontrado. Instale .NET 8 SDK.
  exit /b 1
)

echo Publicando UmbraManager para %DIST_DIR% ...
if exist "%DIST_DIR%" rmdir /s /q "%DIST_DIR%"
mkdir "%DIST_DIR%"

dotnet publish "%WPF_DIR%\UmbraManager\UmbraManager.csproj" -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true -o "%DIST_DIR%"
if errorlevel 1 (
  echo [ERRO] dotnet publish falhou.
  exit /b 1
)

mkdir "%DIST_DIR%\config" 2>nul
copy /Y "%ROOT%\config\manager.json.example" "%DIST_DIR%\config\manager.json.example"
if exist "%ROOT%\config\manager.json" copy /Y "%ROOT%\config\manager.json" "%DIST_DIR%\config\manager.json"
copy /Y "%ROOT%\config\admin.key.example" "%DIST_DIR%\config\admin.key.example"

echo [OK] Dist em %DIST_DIR%\UmbraManager.exe
endlocal
