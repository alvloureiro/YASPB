# Usage Guide

This guide shows how to use the yaspb library in your projects.

## Quick Start

The library provides a clean API for streaming playback. See the `examples/` directory for usage examples (when built with `BUILD_EXAMPLES=ON`).

### Quick Start (Mock Backend)

The Mock backend is enabled by default and requires no external dependencies. You can build and use it immediately:

```bash
# Build with Mock backend (default)
./scripts/build-native.sh

# The Mock backend provides a fully functional implementation
# that simulates playback for testing and development
```

The Mock backend:
- ✅ Works on all platforms (macOS, Linux, Windows, Raspberry Pi)
- ✅ No external dependencies
- ✅ Full API implementation
- ✅ Simulates playback, events, and statistics
- ✅ Perfect for testing and development

## Basic Usage

### Creating a Playback Engine

```cpp
#include <playback/core/PlaybackFactory.h>
#include <playback/api/IPlaybackController.h>
#include <playback/api/IMediaSource.h>

using namespace playback;

// Create a playback engine
auto engine = PlaybackFactory::createEngine();

// Create a media source
auto mediaSource = engine->createMediaSource("path/to/media/file.mp4");

// Create a playback controller
auto controller = engine->createPlaybackController(mediaSource);
```

### Controlling Playback

```cpp
// Start playback
controller->play();

// Pause playback
controller->pause();

// Stop playback
controller->stop();

// Seek to a specific position
controller->seek(5000); // Seek to 5 seconds

// Get current position
auto position = controller->getPosition();

// Get duration
auto duration = controller->getDuration();
```

### Handling Events

```cpp
// Register event callbacks
controller->onStateChanged([](PlaybackState state) {
    switch (state) {
        case PlaybackState::Playing:
            std::cout << "Playback started" << std::endl;
            break;
        case PlaybackState::Paused:
            std::cout << "Playback paused" << std::endl;
            break;
        case PlaybackState::Stopped:
            std::cout << "Playback stopped" << std::endl;
            break;
        case PlaybackState::Error:
            std::cout << "Playback error occurred" << std::endl;
            break;
    }
});

controller->onPositionChanged([](int64_t position) {
    std::cout << "Position: " << position << " ms" << std::endl;
});
```

## Backend Selection

### Using Mock Backend (Default)

The Mock backend is enabled by default and requires no configuration:

```cpp
// Mock backend is automatically used
auto engine = PlaybackFactory::createEngine();
```

### Using FFmpeg Backend

Build with FFmpeg backend enabled:

```bash
./scripts/build-native.sh --clean --backend ffmpeg
```

Then use it in your code:

```cpp
// FFmpeg backend will be used automatically if available
auto engine = PlaybackFactory::createEngine();
```

### Using Apple Backend (macOS/iOS)

Build with Apple backend enabled:

```bash
./scripts/build-native.sh --clean --backend apple
```

The Apple backend uses AVFoundation and is only available on Apple platforms.

### Using GStreamer Backend

Build with GStreamer backend enabled:

```bash
./scripts/build-native.sh --clean --backend gstreamer
```

## Examples

The project includes example applications that demonstrate library usage:

### Apple Audio Example

```bash
# Build the example
./scripts/build-native.sh --clean --backend apple --example apple-audio

# Run the example
./build/bin/apple_audio_example /path/to/audio/file.aiff
```

### FFmpeg Audio Example

```bash
# Build the example
./scripts/build-native.sh --clean --backend ffmpeg --example ffmpeg-audio

# Run the example
./build/bin/ffmpeg_audio_example /path/to/audio/file.mp3
```

## Integration

### CMake Integration

To use yaspb in your CMake project:

```cmake
# Find the yaspb package
find_package(yaspb REQUIRED)

# Link your target against yaspb
target_link_libraries(your_target PRIVATE yaspb::playback)
```

### Manual Integration

1. Include the headers from `include/playback/`
2. Link against the library from `build/lib/`
3. Ensure required backend dependencies are available

## Best Practices

1. **Always check for errors**: The API returns error codes that should be checked
2. **Use RAII**: The library uses smart pointers for automatic resource management
3. **Handle events asynchronously**: Event callbacks may be called from different threads
4. **Clean up resources**: Stop playback and release resources when done
5. **Choose the right backend**: Use Mock for testing, FFmpeg for broad format support, Apple for macOS/iOS native features

## API Reference

For detailed API documentation, see:
- [API Analysis](API_ANALYSIS.md) - Detailed analysis of the API architecture
- [API Diagram](api-diagram.puml) - Visual representation of API components

## Troubleshooting

### Backend Not Available

If a backend is not available, the factory will fall back to Mock backend (if enabled) or return an error. Check:
- Backend was enabled during build
- Required dependencies are installed
- Platform compatibility (e.g., Apple backend only on macOS/iOS)

### Playback Issues

- Ensure media file format is supported by the selected backend
- Check file path is correct and accessible
- Verify backend-specific dependencies are installed

