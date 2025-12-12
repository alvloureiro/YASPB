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
echo ""

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

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
echo -e "${GREEN}Configuring CMake...${NC}"
cmake \
    -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/toolchains/raspberry-pi.cmake" \
    -DRASPBERRY_PI_VERSION="$RPI_VERSION" \
    -DRPI_64BIT="$RPI_64BIT" \
    -DRPI_TOOLCHAIN_PATH="$TOOLCHAIN_PATH" \
    -DRPI_SYSROOT="$SYSROOT_PATH" \
    -DCMAKE_BUILD_TYPE=Release \
    "$PROJECT_ROOT"

# Build
echo -e "${GREEN}Building...${NC}"
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo "Binaries are in: ${BUILD_DIR}/bin"
echo "Libraries are in: ${BUILD_DIR}/lib"

