@echo off
setlocal enabledelayedexpansion

:: Define paths
set "PROJECT_DIR=%~dp0.."
set "EXAMPLES_DIR=%PROJECT_DIR%\build\examples"
set "LOG_DIR=%PROJECT_DIR%\logs"

:: Ensure the logs directory exists
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"

:: Define executable paths
set "EXE_1=%EXAMPLES_DIR%\advanced-use-example\advanced-use-example.exe"
set "EXE_2=%EXAMPLES_DIR%\full-example\full-example.exe"
set "EXE_3=%EXAMPLES_DIR%\quick-example\quick-example.exe"
set "EXE_4=%EXAMPLES_DIR%\sdl2-example\sdl2-example.exe"
set "EXE_5=%EXAMPLES_DIR%\sdl3-example\sdl3-example.exe"
set "EXE_6=%EXAMPLES_DIR%\separate-sources-example\separate-sources-example.exe"

:: Launch each executable and wait for user to press Enter
for %%E in ("%EXE_1%" "%EXE_2%" "%EXE_3%" "%EXE_4%" "%EXE_5%" "%EXE_6%") do (
    :: Extract the directory path of the executable
    set "EXE_DIR=%%~dpE"

    :: Change to the directory where the executable is located
    cd /d "!EXE_DIR!"

    echo Launching "%%~nxE"...
    start "" "%%E" > "%LOG_DIR%\%%~nxE.txt" 2>&1

    echo Press Enter to continue to the next example...
    pause >nul
)

echo All examples launched. Exiting.
exit /b 0
