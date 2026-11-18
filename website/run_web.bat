@echo off
title Smart Restaurant Inventory - Web Server
cd /d "%~dp0"
echo Compiling server.cpp...
g++ -std=c++11 -I.. server.cpp -lws2_32 -o restaurant_web.exe
if %errorlevel% neq 0 (
    echo Note: Compilation failed or g++ not in PATH.
    if exist restaurant_web.exe goto run
    if exist restaurant.exe (
        start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8080"
        restaurant.exe
        exit /b
    )
    pause
    exit /b
)
:run
echo Starting Web Server at http://localhost:8080 ...
start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8080"
restaurant_web.exe
pause
