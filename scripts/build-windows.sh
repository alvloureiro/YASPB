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
CLEAN_BUILD=false

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean|-c)
            CLEAN_BUILD=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --clean, -c    Remove existing build directory before building"
            echo "  --help, -h     Show this help message"
            echo ""
            echo "Environment variables:"
            echo "  BUILD_DIR              Build directory (default: \$PROJECT_ROOT/build-windows)"
            echo "  MINGW_PATH             Path to MinGW-w64 installation (default: /usr/x86_64-w64-mingw32)"
            echo "  NINJA                  Path to ninja binary (default: ninja)"
            echo ""
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}Windows Cross-Compilation Build${NC}"
echo "=================================="
echo "Build Directory: ${BUILD_DIR}"
echo "MinGW Path: ${MINGW_PATH}"
echo "Generator: Ninja"
echo "Clean Build: ${CLEAN_BUILD}"
echo ""

# Check if Ninja is available
NINJA="${NINJA:-ninja}"
if ! command -v "$NINJA" &> /dev/null; then
    echo -e "${YELLOW}Warning: Ninja not found in PATH${NC}"
    echo ""
    echo "To install Ninja:"
    echo "  macOS: brew install ninja"
    echo "  Ubuntu/Debian: sudo apt-get install ninja-build"
    echo "  Or set NINJA environment variable to the path of ninja binary"
    echo ""
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

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

# Check if build directory exists and handle clean build
if [ -d "$BUILD_DIR" ]; then
    if [ "$CLEAN_BUILD" = true ]; then
        echo -e "${YELLOW}Removing existing build directory: ${BUILD_DIR}${NC}"
        rm -rf "$BUILD_DIR"
    else
        echo -e "${YELLOW}Warning: Build directory already exists: ${BUILD_DIR}${NC}"
        echo "  Use --clean or -c flag to remove it before building"
        echo ""
    fi
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake with Ninja generator
echo -e "${GREEN}Configuring CMake with Ninja generator...${NC}"
cmake \
    -G "Ninja" \
    -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/toolchains/windows-cross.cmake" \
    -DMINGW_PATH="$MINGW_PATH" \
    -DCMAKE_BUILD_TYPE=Release \
    "$PROJECT_ROOT"

# Build using Ninja
echo -e "${GREEN}Building with Ninja...${NC}"
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo "Binaries are in: ${BUILD_DIR}/bin"
echo "Libraries are in: ${BUILD_DIR}/lib"

