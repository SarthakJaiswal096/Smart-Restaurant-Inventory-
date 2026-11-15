@echo off
title Smart Restaurant Inventory - Web Server
echo Compiling server.cpp...
g++ -std=c++11 server.cpp -lws2_32 -o restaurant_web.exe
if %errorlevel% neq 0 (
    echo Compilation failed! Make sure g++ is installed and httplib.h is in this folder.
    pause
    exit /b
)
echo Starting Web Server at http://localhost:8080 ...
REM Open the browser after a 2 second delay so the server is already listening.
start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8080"
restaurant_web.exe
pause
