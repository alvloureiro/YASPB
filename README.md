# StreamingPlayback (yaspb)

A personal project to bring to life a modern C++ API for realtime streaming playback.

## Overview

StreamingPlayback is a library designed to provide a clean, extensible interface for realtime media streaming and playback. The project aims to abstract the complexities of different media backends behind a unified API, making it easy to integrate streaming capabilities into applications.

## Features

- **Unified API**: Clean, modern C++ interface for streaming playback
- **Multiple Backends**: Support for different media backends (FFmpeg, GStreamer, etc.)
- **Extensible**: Easy to add new backends and features
- **Modern C++**: Built with C++17 standards
- **Cross-platform**: Designed to work across different platforms

## Technology Stack

- **Language**: C++17
- **Build System**: CMake (3.16+)
- **Backends**:
  - **Mock** (default, no dependencies, works on all platforms)
  - FFmpeg (optional, requires FFmpeg libraries)
  - GStreamer (optional, requires GStreamer libraries)

## Building

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler
- **No external dependencies required for Mock backend** (default)
- FFmpeg development libraries (only if using FFmpeg backend)
- GStreamer development libraries (only if using GStreamer backend)

### Native Build Instructions

#### macOS / Linux

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run tests (if enabled)
ctest
# Or run directly:
./bin/playback_tests
```

#### Windows

```bash
# Using Visual Studio
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Cross-Platform Building

The project supports cross-compilation for multiple platforms. See [Cross-Platform Building Guide](docs/CROSS_PLATFORM_BUILDING.md) for detailed instructions.

**Quick Start:**

```bash
# Build for Raspberry Pi
./scripts/build-raspberry-pi.sh

# Build for Windows (from Linux/macOS)
./scripts/build-windows.sh
```

### Build Options

#### General Options

- `BUILD_SHARED_LIBS`: Build shared library instead of static (default: ON)
- `BUILD_TESTS`: Build test executables (default: ON)
- `BUILD_EXAMPLES`: Build example executables (default: OFF)
- `ENABLE_MOCK_BACKEND`: Enable Mock backend (no dependencies, works on all platforms) (default: ON)
- `ENABLE_FFMPEG_BACKEND`: Enable FFmpeg backend support (default: OFF)
- `ENABLE_GSTREAMER_BACKEND`: Enable GStreamer backend support (default: OFF)

#### Platform-Specific Options

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

Example with custom options:
```bash
cmake -DBUILD_EXAMPLES=ON -DENABLE_GSTREAMER_BACKEND=ON ..
```

## Project Structure

```
yaspb/
├── CMakeLists.txt          # Main build configuration
├── .clang-format           # Code formatting configuration
├── .clang-tidy             # Static analysis configuration
├── .gitattributes          # Git attributes for line endings
├── .gitignore              # Git ignore patterns
├── cmake/                  # Modular CMake configuration files
│   ├── Backends.cmake      # Backend configuration
│   ├── Testing.cmake       # Test setup
│   ├── Examples.cmake      # Example executables
│   └── Install.cmake       # Installation rules
├── .githooks/              # Git hooks (version controlled)
│   └── pre-commit          # Pre-commit hook for linting
├── scripts/                 # Utility scripts
│   ├── install-hooks.sh    # Script to install git hooks
│   ├── build-raspberry-pi.sh  # Raspberry Pi cross-compilation script
│   └── build-windows.sh    # Windows cross-compilation script
├── toolchains/              # CMake toolchain files
│   ├── raspberry-pi.cmake  # Raspberry Pi cross-compilation toolchain
│   └── windows-cross.cmake # Windows cross-compilation toolchain
├── include/                # Public headers
│   └── playback/
│       ├── api/            # Public API interfaces
│       ├── backends/       # Backend interfaces
│       └── core/           # Core engine
├── src/                    # Implementation
│   ├── core/               # Core implementation
│   └── backends/           # Backend implementations
├── tests/                  # Test files
├── examples/               # Example applications
└── docs/                   # Documentation
    ├── API_ANALYSIS.md     # API architecture analysis
    ├── api-diagram.puml    # PlantUML API diagram
    └── CROSS_PLATFORM_BUILDING.md  # Cross-platform build guide
```

## Usage

The library provides a clean API for streaming playback. See the `examples/` directory for usage examples (when built with `BUILD_EXAMPLES=ON`).

### Quick Start (Mock Backend)

The Mock backend is enabled by default and requires no external dependencies. You can build and use it immediately:

```bash
# Build with Mock backend (default)
mkdir build && cd build
cmake ..
cmake --build .

# The Mock backend provides a fully functional implementation
# that simulates playback for testing and development
```

The Mock backend:
- ✅ Works on all platforms (macOS, Linux, Windows, Raspberry Pi)
- ✅ No external dependencies
- ✅ Full API implementation
- ✅ Simulates playback, events, and statistics
- ✅ Perfect for testing and development

## Documentation

### API Documentation

- **API Analysis**: See `docs/API_ANALYSIS.md` for a detailed analysis of the API architecture, components, and relationships.
- **API Diagram**: A PlantUML diagram is available at `docs/api-diagram.puml` showing the relationships between all API components.

To generate a visual diagram from the PlantUML file:
```bash
# Install PlantUML (if not already installed)
# macOS: brew install plantuml
# Linux: apt-get install plantuml

# Generate PNG diagram
plantuml docs/api-diagram.puml

# Or use online tools like http://www.plantuml.com/plantuml/uml/
```

The diagram shows:
- All interfaces and their methods
- Data structures and their relationships
- Enums and their values
- Component dependencies and interactions

## Testing

The project uses Google Test (GTest) for unit testing. Tests are automatically discovered and can be run with CTest.

### Prerequisites for Testing

Google Test (GTest) is required for running tests. It will be automatically fetched if not found, but you can also install it manually:

**macOS:**
```bash
brew install googletest
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install libgtest-dev
```

**Or set GTEST_ROOT:**
```bash
export GTEST_ROOT=/path/to/gtest
```

### Running Tests

```bash
# Build with tests enabled (default)
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON
cmake --build .

# Run all tests
ctest

# Run tests with verbose output
ctest --verbose

# Run tests directly
./bin/playback_tests

# Run specific test
./bin/playback_tests --gtest_filter=MediaSourceTest.*

# Disable tests if GTest is not available
cmake .. -DBUILD_TESTS=OFF
```

### Test Structure

Tests are organized by component:
- `test_api_mediasource.cpp` - Tests for IMediaSource interface
- `test_api_playbackcontroller.cpp` - Tests for IPlaybackController interface
- `test_backends.cpp` - Tests for IPlaybackBackend interface
- `test_core_playbackengine.cpp` - Tests for PlaybackEngine
- `test_core_playbackfactory.cpp` - Tests for PlaybackFactory

All tests use the Mock backend for testing, which requires no external dependencies.

## Development

### Code Style

The project uses `clang-format` for code formatting and `clang-tidy` for static analysis. Configuration files are provided:

- `.clang-format`: Code formatting rules (based on Google style)
- `.clang-tidy`: Static analysis checks

To format code:
```bash
clang-format -i path/to/file.cpp
```

To check code style:
```bash
clang-tidy path/to/file.cpp
```

### Git Hooks

Pre-commit hooks are set up to automatically check code formatting and run static analysis before commits.

**Installation:**
```bash
./scripts/install-hooks.sh
```

The pre-commit hook will:
- Check code formatting with `clang-format`
- Run static analysis with `clang-tidy` (requires `compile_commands.json`)

**Generate compile_commands.json:**
```bash
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
# compile_commands.json will be created in the build directory
# Create a symlink in the project root for clang-tidy:
ln -s build/compile_commands.json ../compile_commands.json
```

**Prerequisites for hooks:**
- `clang-format`: Install with `brew install clang-format` (macOS) or `apt-get install clang-format` (Linux)
- `clang-tidy`: Install with `brew install llvm` (macOS) or `apt-get install clang-tidy` (Linux)

## Contributing

This is a personal project, but suggestions and feedback are welcome!

## License

[Add your license here]

