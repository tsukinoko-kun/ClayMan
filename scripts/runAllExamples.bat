@echo off
setlocal enabledelayedexpansion

:: Define paths
set "PROJECT_DIR=%~dp0.."
set "SHORTCUT_DIR=%PROJECT_DIR%\shortcuts"
set "SCRIPT_DIR=%PROJECT_DIR%\scripts"
set "LOG_DIR=%PROJECT_DIR%\logs"

:: Ensure the logs directory exists
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"

:: Change to shortcut directory
cd /d "%SHORTCUT_DIR%" || (
    echo ERROR: Shortcuts directory not found: "%SHORTCUT_DIR%"
    exit /b 1
)

:: Cycle through each .lnk file, execute, and log output
for %%F in (*.lnk) do (
    set "EXE_NAME=%%~nF"

    echo Launching !EXE_NAME!...
    start "" "%%F" > "%LOG_DIR%\!EXE_NAME!.txt" 2>&1

    timeout /t 10 /nobreak >nul
)

echo All examples launched. Exiting.
exit /b 0
