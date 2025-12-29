#!/bin/bash
#
# Build script for Raspberry Pi cross-compilation
# This script helps set up and build the project for Raspberry Pi
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Default values
RPI_VERSION="${RASPBERRY_PI_VERSION:-4}"
RPI_64BIT="${RPI_64BIT:-ON}"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build-rpi}"
TOOLCHAIN_PATH="${RPI_TOOLCHAIN_PATH:-/opt/rpi-toolchain}"
SYSROOT_PATH="${RPI_SYSROOT:-/opt/rpi-sysroot}"
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
            echo "  BUILD_DIR              Build directory (default: \$PROJECT_ROOT/build-rpi)"
            echo "  RASPBERRY_PI_VERSION    Raspberry Pi version: 2, 3, 4, or 5 (default: 4)"
            echo "  RPI_64BIT               Build for 64-bit: ON or OFF (default: ON)"
            echo "  RPI_TOOLCHAIN_PATH      Path to Raspberry Pi toolchain (default: /opt/rpi-toolchain)"
            echo "  RPI_SYSROOT             Path to sysroot (default: /opt/rpi-sysroot)"
            echo "  NINJA                   Path to ninja binary (default: ninja)"
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

echo -e "${GREEN}Raspberry Pi Cross-Compilation Build${NC}"
echo "=================================="
echo "Raspberry Pi Version: ${RPI_VERSION}"
echo "64-bit: ${RPI_64BIT}"
echo "Build Directory: ${BUILD_DIR}"
echo "Toolchain: ${TOOLCHAIN_PATH}"
echo "Sysroot: ${SYSROOT_PATH}"
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

# Check if toolchain exists
if [ ! -d "$TOOLCHAIN_PATH" ]; then
    echo -e "${YELLOW}Warning: Toolchain not found at ${TOOLCHAIN_PATH}${NC}"
    echo ""
    echo "To set up the Raspberry Pi toolchain:"
    echo "1. Download from: https://github.com/raspberrypi/tools"
    echo "2. Extract to /opt/rpi-toolchain"
    echo "3. Or set RPI_TOOLCHAIN_PATH environment variable"
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
    -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/toolchains/raspberry-pi.cmake" \
    -DRASPBERRY_PI_VERSION="$RPI_VERSION" \
    -DRPI_64BIT="$RPI_64BIT" \
    -DRPI_TOOLCHAIN_PATH="$TOOLCHAIN_PATH" \
    -DRPI_SYSROOT="$SYSROOT_PATH" \
    -DCMAKE_BUILD_TYPE=Release \
    "$PROJECT_ROOT"

# Build using Ninja
echo -e "${GREEN}Building with Ninja...${NC}"
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo "Binaries are in: ${BUILD_DIR}/bin"
echo "Libraries are in: ${BUILD_DIR}/lib"

