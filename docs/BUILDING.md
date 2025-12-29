# Building Guide

This guide covers everything you need to know about building the yaspb project.

## Prerequisites

- CMake 3.16 or higher
- Ninja build system (recommended for faster builds)
  - macOS: `brew install ninja`
  - Linux (Ubuntu/Debian): `sudo apt-get install ninja-build`
- C++17 compatible compiler
- **No external dependencies required for Mock backend** (default)
- FFmpeg development libraries (only if using FFmpeg backend)
- GStreamer development libraries (only if using GStreamer backend)

## Native Build Instructions

### macOS / Linux

**Using the build script (recommended):**

The build script provides convenient command-line options for configuring your build:

```bash
# Build with default settings (Debug build, Mock backend, tests enabled)
./scripts/build-native.sh

# Clean build
./scripts/build-native.sh --clean

# Build FFmpeg backend with tests
./scripts/build-native.sh --clean --backend ffmpeg --tests

# Build Apple backend with example in Release mode
./scripts/build-native.sh --backend apple --example apple-audio --release

# Build with coverage enabled
./scripts/build-native.sh --clean --backend mock --tests --coverage
```

**Build Script Options:**

- `--clean, -c` - Remove existing build directory before building
- `--debug` - Build in Debug mode (default)
- `--release` - Build in Release mode
- `--build-dir <dir>` - Specify build directory
- `--backend <name>` - Select backend: mock, ffmpeg, apple, gstreamer
- `--enable-ffmpeg-backend` - Enable FFmpeg backend
- `--enable-apple-backend` - Enable Apple backend (macOS only)
- `--enable-gstreamer-backend` - Enable GStreamer backend
- `--tests, --enable-tests` - Enable building tests
- `--no-tests` - Disable building tests
- `--examples, --enable-examples` - Enable building examples
- `--example <name>` - Build specific example: apple-audio, ffmpeg-audio
- `--coverage, --enable-coverage` - Enable code coverage
- `--help, -h` - Show help message

**Manual build with Ninja:**

```bash
# Create build directory
mkdir build && cd build

# Configure with Ninja generator
cmake -G "Ninja" ..

# Build
cmake --build .

# Run tests (if enabled)
ctest
# Or run directly:
./bin/playback_tests
```

**Note:** The project uses Ninja as the default build backend for faster builds. Make sure Ninja is installed:
- macOS: `brew install ninja`
- Linux (Ubuntu/Debian): `sudo apt-get install ninja-build`

### Windows

```bash
# Using Visual Studio
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release

# Or using Ninja (if available)
mkdir build && cd build
cmake -G "Ninja" ..
cmake --build .
```

## Cross-Platform Building

The project supports cross-compilation for multiple platforms. See [Cross-Platform Building Guide](CROSS_PLATFORM_BUILDING.md) for detailed instructions.

**Quick Start:**

```bash
# Build for Raspberry Pi
./scripts/build-raspberry-pi.sh

# Build for Windows (from Linux/macOS)
./scripts/build-windows.sh
```

## Build Options

### General Options

- `BUILD_SHARED_LIBS`: Build shared library instead of static (default: ON)
- `BUILD_TESTS`: Build test executables (default: ON)
- `BUILD_EXAMPLES`: Build example executables (default: OFF)
- `ENABLE_MOCK_BACKEND`: Enable Mock backend (no dependencies, works on all platforms) (default: ON)
- `ENABLE_FFMPEG_BACKEND`: Enable FFmpeg backend support (default: OFF)
- `ENABLE_GSTREAMER_BACKEND`: Enable GStreamer backend support (default: OFF)

### Platform-Specific Options

**Linux:**
- `ENABLE_HARDWARE_ACCELERATION`: Enable hardware acceleration (VAAPI, VDPAU) (default: ON)
- `USE_SYSTEM_FFMPEG`: Use system-installed FFmpeg (default: ON)

**Windows:**
- `ENABLE_DXVA2`: Enable DirectX Video Acceleration (default: ON)
- `STATIC_RUNTIME`: Use static runtime libraries (default: OFF)

**macOS:**
- `ENABLE_VIDEOTOOLBOX`: Enable VideoToolbox hardware acceleration (default: ON)
- `ENABLE_METAL`: Enable Metal rendering (default: OFF)

**Raspberry Pi:**
- `RASPBERRY_PI_VERSION`: Raspberry Pi version (2, 3, 4, or 5) (default: 4)
- `RPI_64BIT`: Build for 64-bit (default: ON for Pi 4/5)

### Example with Custom Options

```bash
# Using the build script
./scripts/build-native.sh --clean --backend ffmpeg --tests --coverage

# Or manually
mkdir build && cd build
cmake -G "Ninja" -DBUILD_EXAMPLES=ON -DENABLE_GSTREAMER_BACKEND=ON ..
cmake --build .
```

## Build Configurations

For detailed information on how to configure builds for specific scenarios, see [Build Configurations Guide](BUILD_CONFIGURATIONS.md).

### Default Behavior

**When you run `cmake ..` with no options:**
- ✅ Mock backend: **ENABLED**
- ✅ Tests: **ENABLED** (Mock backend tests)
- ✅ Coverage: **ENABLED**
- ❌ Examples: **DISABLED**

### Quick Examples

**Build only FFmpeg backend:**
```bash
./scripts/build-native.sh --clean --backend ffmpeg
```
*(Mock backend is auto-disabled, tests/coverage are OFF by default)*

**Build FFmpeg backend + tests:**
```bash
./scripts/build-native.sh --clean --backend ffmpeg --tests
```

**Build FFmpeg backend + tests + coverage:**
```bash
./scripts/build-native.sh --clean --backend ffmpeg --tests --coverage
```

**Build FFmpeg backend + example:**
```bash
./scripts/build-native.sh --clean --backend ffmpeg --example ffmpeg-audio
```
*(BUILD_EXAMPLES is automatically enabled when you enable a specific example)*

**Note**:
- When you enable a non-Mock backend, Mock is automatically disabled and tests/coverage/examples default to OFF (must be explicitly enabled).
- When you enable a specific example (e.g., `--example ffmpeg-audio`), `BUILD_EXAMPLES` is automatically enabled.

