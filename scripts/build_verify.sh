#!/bin/bash
# Script to verify and build Neon District Sandbox
# Run from the project root

set -e

echo "=============================================="
echo "Neon District Sandbox - Build Verification"
echo "=============================================="

# Check if running on Windows (needed for UE path)
if [[ ! -d "/c/Program Files/Epic Games/UE_5.8" ]] && [[ ! -d "C:/Program Files/Epic Games/UE_5.8" ]]; then
    echo "Error: UE 5.8 not found at default location"
    exit 1
fi

# Set UE path
export UE_ROOT="/c/Program Files/Epic Games/UE_5.8"

echo "Building project..."
"$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
    -project="/c/Users/fjmn2/Dev/neon-district-sandbox/NeonDistrictSandbox.uproject" \
    -noP4 \
    -platform=Win64 \
    -clientconfig=Development \
    -cook \
    -map=/Game/Maps/ND_City \
    -map=/Game/Maps/ND_MainMenu \
    -build \
    -stage \
    -archive \
    -archivedirectory="/c/Users/fjmn2/Dev/neon-district-sandbox/dist"

echo "Build complete!"
echo "Check: /c/Users/fjmn2/Dev/neon-district-sandbox/dist/Windows/NeonDistrictSandbox/Binaries/Win64/"