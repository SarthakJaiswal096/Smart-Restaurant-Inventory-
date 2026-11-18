@echo off
title Smart Restaurant Inventory - Module Test Programs
REM Each module has its own small test program in the modules folder.
REM Start this file from the project folder, because the data folder path is relative.
if not exist bin mkdir bin

echo Compiling module programs...
g++ -std=c++11 -I. modules\recipe_menu_demo.cpp -o bin\recipe_menu_demo.exe || goto failed
g++ -std=c++11 -I. modules\order_system.cpp     -o bin\order_system.exe     || goto failed
g++ -std=c++11 -I. modules\in.cpp               -o bin\in.exe               || goto failed
g++ -std=c++11 -I. modules\report1.cpp          -o bin\report1.exe          || goto failed

:menu
echo.
echo ===== MODULE TEST PROGRAMS =====
echo 1. Menu and Recipe module   (modules\recipe_menu_demo.cpp)
echo 2. Order module             (modules\order_system.cpp)
echo 3. Inventory module         (modules\in.cpp)
echo 4. Report module            (modules\report1.cpp)
echo 0. Exit
set /p pick=Choice: 
if "%pick%"=="1" bin\recipe_menu_demo.exe
if "%pick%"=="2" bin\order_system.exe
if "%pick%"=="3" bin\in.exe
if "%pick%"=="4" bin\report1.exe
if "%pick%"=="0" exit /b
goto menu

:failed
echo Compilation failed! Make sure g++ is installed.
pause
exit /b
