@echo off
title Smart Restaurant Inventory - Web Server
echo Compiling server.cpp...
g++ -std=c++11 server.cpp -lws2_32 -o restaurant_web.exe
if %errorlevel% neq 0 (
    echo Compilation failed! Make sure g++ and httplib.h are in this folder.
    pause
    exit /b
)
echo Starting Web Server at http://localhost:8080 ...
start "" "http://localhost:8080"
restaurant_web.exe
pause
