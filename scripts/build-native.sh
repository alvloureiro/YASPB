#!/bin/bash
#
# Native build script for macOS/Linux
# This script builds the project using CMake with Ninja as the backend
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Default values
BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build}"
NINJA="${NINJA:-ninja}"
CLEAN_BUILD=false

# CMake options (will be passed to cmake)
CMAKE_OPTS=()

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean|-c)
            CLEAN_BUILD=true
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --backend)
            case "$2" in
                mock)
                    CMAKE_OPTS+=(-DENABLE_MOCK_BACKEND=ON)
                    CMAKE_OPTS+=(-DENABLE_FFMPEG_BACKEND=OFF)
                    CMAKE_OPTS+=(-DENABLE_APPLE_BACKEND=OFF)
                    CMAKE_OPTS+=(-DENABLE_GSTREAMER_BACKEND=OFF)
                    ;;
                ffmpeg)
                    CMAKE_OPTS+=(-DENABLE_MOCK_BACKEND=OFF)
                    CMAKE_OPTS+=(-DENABLE_FFMPEG_BACKEND=ON)
                    CMAKE_OPTS+=(-DENABLE_APPLE_BACKEND=OFF)
                    CMAKE_OPTS+=(-DENABLE_GSTREAMER_BACKEND=OFF)
                    ;;
                apple)
                    CMAKE_OPTS+=(-DENABLE_MOCK_BACKEND=OFF)
                    CMAKE_OPTS+=(-DENABLE_FFMPEG_BACKEND=OFF)
                    CMAKE_OPTS+=(-DENABLE_APPLE_BACKEND=ON)
                    CMAKE_OPTS+=(-DENABLE_GSTREAMER_BACKEND=OFF)
                    ;;
                gstreamer)
                    CMAKE_OPTS+=(-DENABLE_MOCK_BACKEND=OFF)
                    CMAKE_OPTS+=(-DENABLE_FFMPEG_BACKEND=OFF)
                    CMAKE_OPTS+=(-DENABLE_APPLE_BACKEND=OFF)
                    CMAKE_OPTS+=(-DENABLE_GSTREAMER_BACKEND=ON)
                    ;;
                *)
                    echo "Error: Unknown backend '$2'. Valid options: mock, ffmpeg, apple, gstreamer"
                    exit 1
                    ;;
            esac
            shift 2
            ;;
        --enable-ffmpeg-backend)
            CMAKE_OPTS+=(-DENABLE_FFMPEG_BACKEND=ON)
            shift
            ;;
        --enable-apple-backend)
            CMAKE_OPTS+=(-DENABLE_APPLE_BACKEND=ON)
            shift
            ;;
        --enable-gstreamer-backend)
            CMAKE_OPTS+=(-DENABLE_GSTREAMER_BACKEND=ON)
            shift
            ;;
        --enable-mock-backend)
            CMAKE_OPTS+=(-DENABLE_MOCK_BACKEND=ON)
            shift
            ;;
        --tests|--enable-tests)
            CMAKE_OPTS+=(-DBUILD_TESTS=ON)
            shift
            ;;
        --no-tests|--disable-tests)
            CMAKE_OPTS+=(-DBUILD_TESTS=OFF)
            shift
            ;;
        --examples|--enable-examples)
            CMAKE_OPTS+=(-DBUILD_EXAMPLES=ON)
            shift
            ;;
        --no-examples|--disable-examples)
            CMAKE_OPTS+=(-DBUILD_EXAMPLES=OFF)
            shift
            ;;
        --example)
            case "$2" in
                apple-audio)
                    CMAKE_OPTS+=(-DBUILD_EXAMPLES=ON)
                    CMAKE_OPTS+=(-DBUILD_APPLE_AUDIO_EXAMPLE=ON)
                    ;;
                ffmpeg-audio)
                    CMAKE_OPTS+=(-DBUILD_EXAMPLES=ON)
                    CMAKE_OPTS+=(-DBUILD_FFMPEG_AUDIO_EXAMPLE=ON)
                    ;;
                *)
                    echo "Error: Unknown example '$2'. Valid options: apple-audio, ffmpeg-audio"
                    exit 1
                    ;;
            esac
            shift 2
            ;;
        --coverage|--enable-coverage)
            CMAKE_OPTS+=(-DENABLE_COVERAGE=ON)
            shift
            ;;
        --no-coverage|--disable-coverage)
            CMAKE_OPTS+=(-DENABLE_COVERAGE=OFF)
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Build Options:"
            echo "  --clean, -c              Remove existing build directory before building"
            echo "  --debug                   Build in Debug mode (default)"
            echo "  --release                 Build in Release mode"
            echo "  --build-dir <dir>         Build directory (default: \$PROJECT_ROOT/build)"
            echo ""
            echo "Backend Selection:"
            echo "  --backend <name>         Select backend: mock, ffmpeg, apple, gstreamer"
            echo "  --enable-ffmpeg-backend   Enable FFmpeg backend"
            echo "  --enable-apple-backend   Enable Apple backend (macOS only)"
            echo "  --enable-gstreamer-backend Enable GStreamer backend"
            echo "  --enable-mock-backend     Enable Mock backend"
            echo ""
            echo "Build Components:"
            echo "  --tests, --enable-tests   Enable building tests"
            echo "  --no-tests               Disable building tests"
            echo "  --examples, --enable-examples  Enable building examples"
            echo "  --no-examples            Disable building examples"
            echo "  --example <name>        Build specific example: apple-audio, ffmpeg-audio"
            echo "  --coverage, --enable-coverage  Enable code coverage"
            echo "  --no-coverage            Disable code coverage"
            echo ""
            echo "Examples:"
            echo "  $0 --clean --backend ffmpeg --tests"
            echo "  $0 --backend apple --example apple-audio --release"
            echo "  $0 --enable-ffmpeg-backend --tests --coverage"
            echo ""
            echo "Environment variables:"
            echo "  BUILD_DIR              Build directory (default: \$PROJECT_ROOT/build)"
            echo "  CMAKE_BUILD_TYPE       Build type: Debug or Release (default: Debug)"
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

echo -e "${GREEN}Native Build (CMake + Ninja)${NC}"
echo "=================================="
echo "Build Type: ${BUILD_TYPE}"
echo "Build Directory: ${BUILD_DIR}"
echo "Generator: Ninja"
echo "Clean Build: ${CLEAN_BUILD}"
if [ ${#CMAKE_OPTS[@]} -gt 0 ]; then
    echo "CMake Options: ${CMAKE_OPTS[*]}"
fi
echo ""

# Check if Ninja is available
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
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    "${CMAKE_OPTS[@]}" \
    "$PROJECT_ROOT"

# Build using Ninja
echo -e "${GREEN}Building with Ninja...${NC}"
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo "Binaries are in: ${BUILD_DIR}/bin"
echo ""
echo "To run tests:"
echo "  cd ${BUILD_DIR} && ctest"
echo ""
echo "Examples:"
echo "  ./scripts/build-native.sh --clean --backend ffmpeg --tests"
echo "  ./scripts/build-native.sh --backend apple --example apple-audio --release"
echo "  ./scripts/build-native.sh --enable-ffmpeg-backend --tests --coverage"
echo ""
echo "For more options, use: ./scripts/build-native.sh --help"

