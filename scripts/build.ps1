# Build script for Neon District Sandbox
# Run from: Developer Command Prompt or PowerShell

param(
    [string]$Configuration = "Development",
    [string]$Platform = "Win64"
)

$ErrorActionPreference = "Stop"
$ProjectName = "NeonDistrictSandbox"
$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent Split-Path $ProjectDir  # Go up 2 levels from scripts/
Set-Location $ProjectDir

Write-Host "=====================================" -ForegroundColor Blue
Write-Host "Neon District Sandbox - Build Script" -ForegroundColor Blue
Write-Host "=====================================" -ForegroundColor Blue
Write-Host ""
Write-Host "Project Directory: $ProjectDir" -ForegroundColor Green
Write-Host "Configuration: $Configuration" -ForegroundColor Green
Write-Host "Platform: $Platform" -ForegroundColor Green
Write-Host ""

# Find UE installation
$UEPath = "C:\Program Files\Epic Games\UE_5.8"
if (-not (Test-Path $UEPath)) {
    Write-Error "Unreal Engine 5.8 not found at $UEPath"
    exit 1
}

$UEBuildPath = Join-Path $UEPath "Engine\Build\BatchFiles"
$ProjectFile = Join-Path $ProjectDir "NeonDistrictSandbox.uproject"

if (-not (Test-Path $ProjectFile)) {
    Write-Error "Project file not found: $ProjectFile"
    exit 1
}

# Step 1: Generate project files
Write-Host "Step 1: Generating project files..." -ForegroundColor Yellow
& "$UEBuildPath\GenerateProjectFiles.bat" -project="$ProjectFile" -game -engine
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to generate project files"
    exit 1
}
Write-Host "✓ Project files generated" -ForegroundColor Green

# Step 2: Build
Write-Host "Step 2: Building $Configuration configuration..." -ForegroundColor Yellow
& "$UEBuildPath\Build.bat" $ProjectName $Platform $Configuration -project="$ProjectFile" -build
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed!"
    exit 1
}
Write-Host "✓ Build complete" -ForegroundColor Green

# Step 3: Cook and package
Write-Host "Step 3: Cooking and packaging..." -ForegroundColor Yellow
$DistDir = Join-Path $ProjectDir "dist"
& "$UEBuildPath\RunUAT.bat" BuildCookRun `
    -project="$ProjectFile" `
    -noP4 `
    -platform=$Platform `
    -clientconfig=$Configuration `
    -cook `
    -allmaps `
    -build `
    -stage `
    -archive `
    -archivedirectory="$DistDir" `
    -compile `
    -skipstage

if ($LASTEXITCODE -ne 0) {
    Write-Error "Cooking/Packaging failed!"
    exit 1
}

Write-Host "✓ Package complete" -ForegroundColor Green

# Step 4: Verify
$ExePath = Join-Path $DistDir "Windows\$ProjectName\Binaries\Win64\$ProjectName.exe"
if (Test-Path $ExePath) {
    Write-Host "✓ Executable found: $ExePath" -ForegroundColor Green
} else {
    Write-Error "Executable not found: $ExePath"
    exit 1
}

Write-Host ""
Write-Host "=====================================" -ForegroundColor Green
Write-Host "BUILD COMPLETE!" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green
Write-Host ""
Write-Host "To run the benchmark:" -ForegroundColor Cyan
Write-Host "  ""$ExePath"" ND_City -benchmark -unattended -log" -ForegroundColor Cyan