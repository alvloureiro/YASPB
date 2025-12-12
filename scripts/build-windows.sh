#!/bin/bash
#
# Build script for Windows cross-compilation (from Linux/macOS)
# This script helps set up and build the project for Windows
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Default values
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build-windows}"
MINGW_PATH="${MINGW_PATH:-/usr/x86_64-w64-mingw32}"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}Windows Cross-Compilation Build${NC}"
echo "=================================="
echo "Build Directory: ${BUILD_DIR}"
echo "MinGW Path: ${MINGW_PATH}"
echo ""

# Check if MinGW is installed
if [ ! -d "$MINGW_PATH" ]; then
    echo -e "${YELLOW}Warning: MinGW-w64 not found at ${MINGW_PATH}${NC}"
    echo ""
    echo "To install MinGW-w64:"
    echo "  macOS: brew install mingw-w64"
    echo "  Ubuntu/Debian: sudo apt-get install mingw-w64"
    echo "  Or set MINGW_PATH environment variable"
    echo ""
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
echo -e "${GREEN}Configuring CMake...${NC}"
cmake \
    -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/toolchains/windows-cross.cmake" \
    -DMINGW_PATH="$MINGW_PATH" \
    -DCMAKE_BUILD_TYPE=Release \
    "$PROJECT_ROOT"

# Build
echo -e "${GREEN}Building...${NC}"
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo "Binaries are in: ${BUILD_DIR}/bin"
echo "Libraries are in: ${BUILD_DIR}/lib"

