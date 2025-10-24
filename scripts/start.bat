@echo off
REM UmbraEternum Server Startup Script (Windows)
REM Version: 1.3.0

echo =========================================
echo   Starting UmbraEternum Server Stack
echo =========================================
echo.

REM Check if build directory exists
if not exist "..\build" (
  echo Error: Build directory not found!
  echo Please run CMake and build the project first.
  pause
  exit /b 1
)

REM Create logs directory
if not exist "..\logs" mkdir "..\logs"

REM Check if executable exists
if not exist "..\build\bin\Release\umbra_server.exe" (
  if not exist "..\build\bin\Debug\umbra_server.exe" (
    echo Error: umbra_server.exe not found!
    echo Please build the project first.
    pause
    exit /b 1
  )
)

echo Starting servers...
echo.

REM Start Auth Server
if exist "..\build\bin\Release\auth_server.exe" (
  echo [1/5] Starting Auth Server...
  start "Auth Server" /MIN ..\build\bin\Release\auth_server.exe
  timeout /t 2 /nobreak >nul
)

REM Start World Server
if exist "..\build\bin\Release\world_server.exe" (
  echo [2/5] Starting World Server...
  start "World Server" /MIN ..\build\bin\Release\world_server.exe
  timeout /t 2 /nobreak >nul
)

REM Start Zone Server
if exist "..\build\bin\Release\zone_server.exe" (
  echo [3/5] Starting Zone Server...
  start "Zone Server" /MIN ..\build\bin\Release\zone_server.exe 1
  timeout /t 2 /nobreak >nul
)

REM Start Chat Server
if exist "..\build\bin\Release\chat_server.exe" (
  echo [4/5] Starting Chat Server...
  start "Chat Server" /MIN ..\build\bin\Release\chat_server.exe
  timeout /t 2 /nobreak >nul
)

REM Start Gateway Server
if exist "..\build\bin\Release\gateway_server.exe" (
  echo [5/5] Starting Gateway Server...
  start "Gateway Server" /MIN ..\build\bin\Release\gateway_server.exe
  timeout /t 2 /nobreak >nul
)

echo.
echo =========================================
echo   All servers started successfully!
echo =========================================
echo.
echo Check the individual console windows for server output.
echo Close the windows to stop each server.
echo.
pause

