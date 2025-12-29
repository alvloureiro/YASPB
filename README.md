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

See the [Building Guide](docs/BUILDING.md) for complete instructions on building the project.

**Quick Start:**

```bash
# Build with default settings (Mock backend, tests enabled)
./scripts/build-native.sh

# Build FFmpeg backend with tests
./scripts/build-native.sh --clean --backend ffmpeg --tests
```

For detailed build instructions, cross-platform building, and all build options, see [docs/BUILDING.md](docs/BUILDING.md).

## Project Structure

See [Project Structure](docs/PROJECT_STRUCTURE.md) for a detailed overview of the project organization.

## Usage

See the [Usage Guide](docs/USAGE.md) for examples and instructions on using the library in your projects.

**Quick Start:**

```bash
# Build with Mock backend (default, no dependencies)
./scripts/build-native.sh
```

## Documentation

See the [Documentation Guide](docs/DOCUMENTATION.md) for all available documentation, including:
- API documentation and analysis
- Usage examples
- Architecture diagrams

## Testing and Coverage

See the [Testing Guide](docs/TESTING.md) for complete information on running tests and generating coverage reports.

**Quick Start:**

```bash
# Build with tests and coverage
./scripts/build-native.sh --clean --backend mock --tests --coverage

# Run tests
cd build && ctest

# Generate coverage report
cmake --build . --target coverage-html
```

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

