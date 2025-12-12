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
├── cmake/                  # Modular CMake configuration files
│   ├── Backends.cmake      # Backend configuration
│   ├── Testing.cmake       # Test setup
│   ├── Examples.cmake      # Example executables
│   └── Install.cmake       # Installation rules
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

## Contributing

This is a personal project, but suggestions and feedback are welcome!

## License

[Add your license here]

