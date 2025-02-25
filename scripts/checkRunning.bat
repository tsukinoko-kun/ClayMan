@echo off
:: Usage: checkRunning.bat filename.exe
:: Example: call checkRunning.bat full-example.exe
:: This script checks if a given executable is running.
:: If running, it returns exit code 1; otherwise, it returns exit code 0.

if "%~1"=="" (
    echo Usage: checkRunning.bat filename.exe
    exit /b 2
)

tasklist | findstr /I "%~1" >nul
if %ERRORLEVEL% equ 0 (
    echo %~1 is running. Exiting task.
    exit /b 1
)

echo %~1 is not running. Proceeding with the task.
exit /b 0
