# Project Structure

This document describes the organization and structure of the yaspb project.

## Directory Layout

```
yaspb/
├── CMakeLists.txt          # Main build configuration
├── LICENSE                 # MIT License
├── README.md               # Project overview and quick start
├── .clang-format           # Code formatting configuration
├── .clang-tidy             # Static analysis configuration
├── .gitattributes          # Git attributes for line endings
├── .gitignore              # Git ignore patterns
├── CMakePresets.json       # CMake presets for IDEs
│
├── cmake/                  # Modular CMake configuration files
│   ├── Backends.cmake      # Backend configuration
│   ├── Testing.cmake        # Test setup
│   ├── Examples.cmake      # Example executables
│   ├── Install.cmake       # Installation rules
│   ├── Coverage.cmake      # Code coverage configuration
│   ├── Platform.cmake       # Platform detection
│   ├── AppleBackend.cmake  # Apple backend specific config
│   └── FindFFmpeg.cmake    # FFmpeg find module
│
├── .githooks/              # Git hooks (version controlled)
│   └── pre-commit          # Pre-commit hook for linting
│
├── scripts/                # Utility scripts
│   ├── install-hooks.sh    # Script to install git hooks
│   ├── build-native.sh     # Native build script
│   ├── build-raspberry-pi.sh  # Raspberry Pi cross-compilation script
│   ├── build-windows.sh    # Windows cross-compilation script
│   └── install-gtest.sh    # GTest installation helper
│
├── toolchains/              # CMake toolchain files
│   ├── raspberry-pi.cmake  # Raspberry Pi cross-compilation toolchain
│   └── windows-cross.cmake # Windows cross-compilation toolchain
│
├── include/                 # Public headers
│   └── playback/
│       ├── api/            # Public API interfaces
│       │   ├── IMediaSource.h
│       │   ├── IPlaybackController.h
│       │   └── ...
│       ├── backends/       # Backend interfaces
│       │   └── IPlaybackBackend.h
│       └── core/           # Core engine
│           ├── PlaybackEngine.h
│           ├── PlaybackFactory.h
│           └── ...
│
├── src/                     # Implementation
│   ├── core/               # Core implementation
│   │   ├── PlaybackEngine.cpp
│   │   ├── PlaybackFactory.cpp
│   │   └── ...
│   └── backends/           # Backend implementations
│       ├── mock/           # Mock backend
│       │   ├── MockBackend.cpp
│       │   ├── MockMediaSource.cpp
│       │   └── ...
│       ├── ffmpeg/         # FFmpeg backend
│       │   └── ...
│       ├── apple/          # Apple backend (macOS/iOS)
│       │   └── ...
│       └── gstreamer/      # GStreamer backend
│           └── ...
│
├── tests/                   # Test files
│   ├── test_api_mediasource.cpp
│   ├── test_api_playbackcontroller.cpp
│   ├── test_backends.cpp
│   ├── test_core_playbackengine.cpp
│   └── test_core_playbackfactory.cpp
│
├── examples/                # Example applications
│   ├── apple_audio_example.cpp
│   └── ffmpeg_audio_example.cpp
│
└── docs/                    # Documentation
    ├── BUILDING.md          # Building guide
    ├── PROJECT_STRUCTURE.md # This file
    ├── USAGE.md             # Usage examples
    ├── DOCUMENTATION.md     # API documentation
    ├── TESTING.md           # Testing and coverage guide
    ├── API_ANALYSIS.md      # API architecture analysis
    ├── BUILD_CONFIGURATIONS.md # Build configuration details
    ├── CROSS_PLATFORM_BUILDING.md # Cross-platform build guide
    ├── CLION_SETUP.md       # CLion IDE setup
    ├── api-diagram.puml     # PlantUML API diagram
    └── RAII_REVIEW.md       # RAII pattern review
```

## Key Components

### Build System

- **CMakeLists.txt**: Main CMake configuration file
- **cmake/**: Modular CMake configuration files for better organization
- **CMakePresets.json**: Pre-configured CMake presets for IDEs like CLion

### Source Code Organization

- **include/**: Public API headers that users of the library will include
- **src/**: Implementation files, organized by component (core, backends)

### Backends

Each backend is implemented in its own directory under `src/backends/`:
- **mock**: Mock implementation (no dependencies, for testing)
- **ffmpeg**: FFmpeg-based backend
- **apple**: Apple AVFoundation backend (macOS/iOS only)
- **gstreamer**: GStreamer-based backend

### Testing

- **tests/**: Unit tests using Google Test framework
- Tests are organized by component (API, backends, core)

### Examples

- **examples/**: Example applications demonstrating library usage
- Each example shows how to use a specific backend

### Documentation

- **docs/**: Comprehensive documentation covering all aspects of the project
- Markdown files for easy reading and maintenance

## Build Artifacts

When you build the project, the following directories are created:

- **build/**: Default build directory (or as specified)
  - **bin/**: Compiled executables (tests, examples)
  - **lib/**: Compiled libraries
  - **coverage/**: Code coverage reports (if enabled)

## Scripts

All build and utility scripts are in the `scripts/` directory:
- **build-native.sh**: Native build script with convenient options
- **build-raspberry-pi.sh**: Cross-compilation for Raspberry Pi
- **build-windows.sh**: Cross-compilation for Windows
- **install-hooks.sh**: Install git hooks for code quality

