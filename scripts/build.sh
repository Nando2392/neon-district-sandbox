#!/bin/bash
# Build script for Neon District Sandbox
# Run from: Parent directory (Dev) or specify project path

set -e  # Exit on first error
set -o pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=====================================${NC}"
echo -e "${BLUE}Neon District Sandbox - Build Script${NC}"
echo -e "${BLUE}=====================================${NC}"

# Configuration
PROJECT_NAME="NeonDistrictSandbox"
PROJECT_FILE="NeonDistrictSandbox.uproject"

# Convert Windows paths to MSYS style
PROJECT_DIR="/c/Users/fjmn2/Dev/neon-district-sandbox"
UE_ROOT="/c/Program Files/Epic Games/UE_5.8"

# Convert to Windows-native paths for the batch files
PROJECT_ROOT="C:\\Users\\fjmn2\\Dev\\neon-district-sandbox"
UE_WINDOWS="C:\\Program Files\\Epic Games\\UE_5.8"

echo -e "${GREEN}Project directory: $PROJECT_DIR${NC}"

# Check if UE tools exist
if [ ! -d "$PROJECT_DIR" ]; then
    echo -e "${RED}ERROR: Project directory not found: $PROJECT_DIR${NC}"
    exit 1
fi

cd "$PROJECT_DIR"

# Check for Visual Studio
if ! command -v msbuild &> /dev/null; then
    echo -e "${YELLOW}Note: msbuild not found, using UE build tools${NC}"
fi

# Step 1: Run via cmd for proper UE build
echo -e "${YELLOW}Step 1: Installing dependencies and checking project...${NC}"

# Check for required files
if [ ! -f "$PROJECT_FILE" ]; then
    echo -e "${RED}ERROR: $PROJECT_FILE not found!${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Project file found${NC}"

# Check for Source directory
if [ ! -d "Source" ]; then
    echo -e "${RED}ERROR: Source directory not found!${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Source directory found${NC}"

# Check for compiled binaries
if [ -f "Binaries/Win64/NeonDistrictSandbox.exe" ]; then
    echo -e "${GREEN}✓ Existing binary found${NC}"
else
    echo -e "${YELLOW}No existing binary, will compile${NC}"
fi

echo ""
echo -e "${BLUE}Manual Build Instructions:${NC}"
echo "Since UE build tools are Windows-native, use one of these options:"
echo ""
echo "Option 1 - PowerShell (Recommended):"
echo "  cd '$PROJECT_ROOT'"
echo "  'C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Build\\BatchFiles\\GenerateProjectFiles.bat'"
echo "  'C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Build\\BatchFiles\\Build.bat' NeonDistrictSandbox Win64 Development"
echo ""
echo "Option 2 - Using UE Editor:"
echo "  1. Open Unreal Engine 5.8"
echo "  2. Add File -> Open Project -> NeonDistrictSandbox.uproject"
echo "  3. File -> Package -> Package Project"
echo ""
echo "Option 3 - Visual Studio Developer Command Prompt:"
echo "  Run from: 'C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat'"
echo "  Then run the Build.bat commands above"

echo ""
echo -e "${GREEN}Build preparation complete!${NC}"