# Yet Another StreamingPlayback (yaspb)

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
- Ninja build system (recommended for faster builds)
  - macOS: `brew install ninja`
  - Linux (Ubuntu/Debian): `sudo apt-get install ninja-build`
- C++17 compatible compiler
- **No external dependencies required for Mock backend** (default)
- FFmpeg development libraries (only if using FFmpeg backend)
- GStreamer development libraries (only if using GStreamer backend)

### Native Build Instructions

#### macOS / Linux

**Using the build script (recommended):**

```bash
# Build with default settings (Debug build)
./scripts/build-native.sh

# Build with custom settings
CMAKE_BUILD_TYPE=Release ./scripts/build-native.sh
```

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
# Using the build script
CMAKE_BUILD_TYPE=Release ./scripts/build-native.sh
cd build && cmake -G "Ninja" .. -DBUILD_EXAMPLES=ON -DENABLE_GSTREAMER_BACKEND=ON

# Or manually
mkdir build && cd build
cmake -G "Ninja" -DBUILD_EXAMPLES=ON -DENABLE_GSTREAMER_BACKEND=ON ..
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

## Build Configurations

For detailed information on how to configure builds for specific scenarios, see [docs/BUILD_CONFIGURATIONS.md](docs/BUILD_CONFIGURATIONS.md).

### Default Behavior

**When you run `cmake ..` with no options:**
- ✅ Mock backend: **ENABLED**
- ✅ Tests: **ENABLED** (Mock backend tests)
- ✅ Coverage: **ENABLED**
- ❌ Examples: **DISABLED**

### Quick Examples

**Build only FFmpeg backend:**
```bash
mkdir build && cd build
cmake -G "Ninja" .. -DENABLE_FFMPEG_BACKEND=ON
```
*(Mock backend is auto-disabled, tests/coverage are OFF by default)*

**Build FFmpeg backend + tests:**
```bash
mkdir build && cd build
cmake -G "Ninja" .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_TESTS=ON
```

**Build FFmpeg backend + tests + coverage:**
```bash
mkdir build && cd build
cmake -G "Ninja" .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON
```

**Build FFmpeg backend + example:**
```bash
mkdir build && cd build
cmake -G "Ninja" .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_FFMPEG_AUDIO_EXAMPLE=ON
```
*(BUILD_EXAMPLES is automatically enabled when you enable a specific example)*

**Note**:
- When you enable a non-Mock backend, Mock is automatically disabled and tests/coverage/examples default to OFF (must be explicitly enabled).
- When you enable a specific example (e.g., `BUILD_FFMPEG_AUDIO_EXAMPLE=ON`), `BUILD_EXAMPLES` is automatically enabled.

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
# Build with tests enabled (default) using the build script
./scripts/build-native.sh

# Or manually with Ninja
mkdir build && cd build
cmake -G "Ninja" .. -DBUILD_TESTS=ON
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

### Code Coverage

The project supports code coverage analysis using `gcov` and `lcov` to track how much of the codebase is covered by tests. Coverage reports can be generated for each backend implementation separately.

#### Prerequisites for Coverage

**macOS:**
```bash
brew install lcov
# gcov is typically included with Xcode Command Line Tools
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install lcov gcov
```

#### Generating Coverage Reports

```bash
# Build with coverage enabled using the build script
CMAKE_BUILD_TYPE=Debug ./scripts/build-native.sh
# Then reconfigure with coverage:
cd build && cmake -G "Ninja" .. -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug

# Or manually with Ninja
mkdir build && cd build
cmake -G "Ninja" .. -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# Run tests to generate coverage data
ctest

# Generate coverage report
cmake --build . --target coverage

# Generate HTML coverage report
cmake --build . --target coverage-html

# View the report
open coverage/html/index.html  # macOS
# or
xdg-open coverage/html/index.html  # Linux
```

#### Per-Backend Coverage Reports

You can generate coverage reports for individual backends:

```bash
# Coverage for Mock backend
cmake --build . --target coverage-mock

# Coverage for FFmpeg backend (if enabled)
cmake --build . --target coverage-ffmpeg

# Coverage for Apple backend (if enabled, macOS only)
cmake --build . --target coverage-apple
```

Each backend report will be available in:
- `coverage/backend-<name>/html/index.html` - HTML report
- `coverage/backend-<name>/coverage_filtered.info` - Coverage data file

#### Coverage Targets

- `coverage` - Generate combined coverage data for all backends
- `coverage-html` - Generate HTML coverage report (requires genhtml)
- `coverage-summary` - Print coverage summary to console
- `coverage-clean` - Remove all coverage files
- `coverage-<backend>` - Generate coverage for specific backend (e.g., `coverage-mock`, `coverage-ffmpeg`)

#### Coverage Report Locations

- Combined report: `coverage/html/index.html`
- Per-backend reports: `coverage/backend-<name>/html/index.html`
- Coverage data files: `coverage/*.info`

## IDE Setup

### CLion

The project is configured for easy use with CLion. See [CLION_SETUP.md](CLION_SETUP.md) for detailed instructions.

**Quick Start:**
1. Open the project in CLion
2. CLion will automatically detect `CMakeLists.txt` and `CMakePresets.json`
3. Select the **default** or **debug** CMake preset (both have `BUILD_EXAMPLES=ON`)
4. Reload CMake project
5. Find `apple_audio_example` in the CMake tool window
6. Set program arguments to an audio file path (e.g., `/System/Library/Sounds/Glass.aiff`)
7. Click the play button to run!

The example will automatically be available as a run configuration once CMake is configured.

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
cmake -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
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

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

