@echo off
title Smart Restaurant Inventory - Console Mode
echo Compiling main.cpp...
g++ -std=c++11 main.cpp -o restaurant.exe
if %errorlevel% neq 0 (
    echo Compilation failed! Make sure g++ is installed.
    pause
    exit /b
)
echo Starting Console App...
restaurant.exe
pause
