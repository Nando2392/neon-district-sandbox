# Build script for Neon District Sandbox - PowerShell version
param(
    [string]$BuildType = "Development",
    [string]$Platform = "Win64"
)

$ErrorActionPreference = "Stop"

# Set paths
$ProjectDir = "C:\Users\fjmn2\Dev\neon-district-sandbox"
$UERoot = "C:\Program Files\Epic Games\UE_5.8"
$ProjectName = "NeonDistrictSandbox"

Write-Host "=== Neon District Sandbox Build Script ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectDir"
Write-Host "UE Version: 5.8"

# Change to project directory
Set-Location $ProjectDir

# Generate project files
Write-Host "Generating project files..." -ForegroundColor Yellow
& "$UERoot\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="$ProjectDir\$ProjectName.uproject" -game -engine

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to generate project files" -ForegroundColor Red
    exit 1
}

# Build Development
Write-Host "Building $BuildType $Platform..." -ForegroundColor Yellow
& "$UERoot\Engine\Build\BatchFiles\Build.bat" $ProjectName $Platform $BuildType -project="$ProjectDir\$ProjectName.uproject" -nohotfixes

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    exit 1
}

Write-Host "Build completed successfully!" -ForegroundColor Green

# Run cook and package
Write-Host ""
Write-Host "=== Cooking and Packaging ===" -ForegroundColor Cyan
& "$UERoot\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
    -project="$ProjectDir\$ProjectName.uproject" `
    -noP4 `
    -platform=$Platform `
    -clientconfig=$BuildType `
    -cook `
    -allmaps `
    -build `
    -stage `
    -archive `
    -archivedirectory="$ProjectDir\dist" `
    -compile `
    -skipstage

Write-Host ""
Write-Host "=== Done ===" -ForegroundColor Green
Write-Host "Package available at: $ProjectDir\dist\Windows\$ProjectName\"