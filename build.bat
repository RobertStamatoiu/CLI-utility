@echo off
setlocal

for %%I in ("%~dp0.") do set "ROOT=%%~fI"
set "SOURCE=%ROOT%\dev.cpp"
set "BIN=%ROOT%\bin"
set "OUTPUT=%BIN%\dev.exe"

where g++ >nul 2>nul
if errorlevel 1 (
    echo Error: g++ was not found on PATH.
    exit /b 1
)

if not exist "%SOURCE%" (
    echo Error: Source file not found: "%SOURCE%"
    exit /b 1
)

if not exist "%BIN%" mkdir "%BIN%"
if errorlevel 1 exit /b 1

echo Building dev.cpp -^> "%OUTPUT%"
g++ -std=c++20 -Wall -Wextra -I "%ROOT%" "%SOURCE%" -o "%OUTPUT%"
if errorlevel 1 exit /b %errorlevel%

echo Built "%OUTPUT%"
exit /b 0
