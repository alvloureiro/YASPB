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
  - FFmpeg (default, enabled)
  - GStreamer (optional)

## Building

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler
- FFmpeg development libraries (if using FFmpeg backend)
- GStreamer development libraries (if using GStreamer backend)

### Build Instructions

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run tests (if enabled)
ctest
```

### Build Options

- `BUILD_SHARED_LIBS`: Build shared library instead of static (default: ON)
- `BUILD_TESTS`: Build test executables (default: ON)
- `BUILD_EXAMPLES`: Build example executables (default: OFF)
- `ENABLE_FFMPEG_BACKEND`: Enable FFmpeg backend support (default: ON)
- `ENABLE_GSTREAMER_BACKEND`: Enable GStreamer backend support (default: OFF)

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
│   └── install-hooks.sh    # Script to install git hooks
├── include/                # Public headers
│   └── playback/
│       ├── api/            # Public API interfaces
│       ├── backends/       # Backend interfaces
│       └── core/           # Core engine
├── src/                    # Implementation
│   ├── core/               # Core implementation
│   └── backends/           # Backend implementations
├── tests/                  # Test files
└── examples/               # Example applications
```

## Usage

The library provides a clean API for streaming playback. See the `examples/` directory for usage examples (when built with `BUILD_EXAMPLES=ON`).

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

