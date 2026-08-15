@echo off
REM Build script for Neon District Sandbox
REM Run from: C:\Users\fjmn2\Dev\neon-district-sandbox

setlocal

set PROJECT_DIR=%~dp0
set UE_VERSION=5.8
set UE_ROOT=C:\Program Files\Epic Games\UE_%UE_VERSION%

echo === Neon District Sandbox Build Script ===
echo Project: %PROJECT_DIR%
echo UE Version: %UE_VERSION%

REM Check if UE exists
if not exist "%UE_ROOT%\Engine\Build\BatchFiles\GenerateProjectFiles.bat" (
    echo ERROR: UE %UE_VERSION% not found at %UE_ROOT%
    echo Please install UE %UE_VERSION% or update UE_ROOT
    exit /b 1
)

REM Clean previous build
if exist "%PROJECT_DIR%Binaries\Win64" (
    echo Cleaning Binaries...
    rd /s /q "%PROJECT_DIR%Binaries\Win64" 2>nul
)

REM Generate project files
echo Generating project files...
"%UE_ROOT%\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="%PROJECT_DIR%NeonDistrictSandbox.uproject" -game -engine

if errorlevel 1 (
    echo ERROR: Failed to generate project files
    exit /b 1
)

REM Build Development
echo Building Development...
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" NeonDistrictSandbox Win64 Development -project="%PROJECT_DIR%NeonDistrictSandbox.uproject" -nohotfixes

if errorlevel 1 (
    echo ERROR: Build failed
    exit /b 1
)

echo Build completed successfully!

REM Run cook and package
echo.
echo === Cooking and Packaging ===
"%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
    -project="%PROJECT_DIR%NeonDistrictSandbox.uproject" ^
    -noP4 ^
    -platform=Win64 ^
    -clientconfig=Development ^
    -cook ^
    -allmaps ^
    -build ^
    -stage ^
    -archive ^
    -archivedirectory="%PROJECT_DIR%dist" ^
    -compile ^
    -skipstage

echo.
echo === Done ===
echo Package available at: %PROJECT_DIR%dist\Windows\NeonDistrictSandbox\
endlocal