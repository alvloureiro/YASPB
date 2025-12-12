# API Analysis

## Overview

The StreamingPlayback API is designed with a clean separation of concerns, following the interface segregation principle and dependency inversion. The architecture is modular and extensible, allowing for multiple backend implementations while maintaining a unified interface.

## Core Components

### 1. PlaybackEngine (Entry Point)
**Location**: `playback::core::PlaybackEngine`

The main orchestrator of the library. It:
- Manages multiple backend implementations
- Creates playback controllers
- Provides auto-discovery of the best backend for a given media source
- Maintains global default configuration

**Key Responsibilities**:
- Backend registration and management
- Controller creation (manual or automatic backend selection)
- Configuration management

### 2. IPlaybackBackend (Backend Interface)
**Location**: `playback::backends::IPlaybackBackend`

Abstract interface for media playback backends (FFmpeg, GStreamer, etc.). Each backend:
- Creates controllers and media sources
- Reports supported formats and capabilities
- Handles initialization and shutdown

**Key Methods**:
- `createController()`: Factory method for controllers
- `createMediaSource()`: Factory method for media sources
- `isSupported()`: Format compatibility check
- `isHardwareAccelerationSupported()`: Hardware capability check

### 3. IPlaybackController (Playback Control)
**Location**: `playback::api::IPlaybackController`

The primary interface for controlling media playback. Provides:
- **Playback Control**: play, pause, stop, seek
- **Rate Control**: playback speed adjustment
- **Volume Control**: audio level management
- **Quality Control**: adaptive bitrate and quality switching
- **Statistics**: real-time playback metrics
- **State Management**: current playback state
- **Event Handling**: listener registration for playback events
- **Configuration**: runtime configuration updates

**State Management**:
- Tracks current position, duration, and playback state
- Provides statistics (bandwidth, FPS, dropped frames, buffer health)

### 4. IMediaSource (Media Source)
**Location**: `playback::api::IMediaSource`

Represents a media source (file, URL, stream, etc.). Provides:
- **Loading**: from URI or raw data
- **Format Discovery**: available formats and adaptive streams
- **Metadata**: title, artist, album, thumbnail
- **Capabilities**: live stream detection, seekability, duration
- **DRM Support**: license management

**Key Features**:
- Supports both static files and live streams
- Provides adaptive streaming information (DASH, HLS)
- Metadata extraction

### 5. IPlaybackEventListener (Event Handling)
**Location**: `playback::api::IPlaybackEventListener`

Observer interface for playback events. Receives:
- **Playback Events**: state changes, errors, position updates, buffering progress
- **Video Frames**: raw video frame data
- **Audio Data**: raw audio sample data

**Event Types**:
- State changes (playing, paused, buffering, etc.)
- Position updates
- Quality changes
- Errors
- Metadata updates
- Seek completion

### 6. Data Structures

#### MediaFormat
Comprehensive format information including:
- Media type (audio, video, subtitle)
- Codec information
- Container format
- Optional video/audio format details
- MIME type and duration

#### PlaybackConfig
Extensive configuration options:
- **Network**: buffer sizes, timeouts, retries
- **Video**: hardware acceleration, max resolution/framerate
- **Audio**: sample rate, channels
- **Adaptive Streaming**: bitrate limits, quality switching thresholds
- **Subtitles**: language preferences
- **Caching**: size, persistence, directory
- **Callbacks**: logging, download progress
- **Backend Options**: backend-specific settings

#### PlaybackEvent
Event payload containing:
- Event type
- Current state
- Error information
- Position and duration
- Buffer level
- Custom messages

#### PlaybackStats
Real-time playback metrics:
- Network bandwidth
- Frame rate and dropped frames
- Bytes downloaded
- Buffer health

## Architecture Patterns

### 1. Factory Pattern
- `IPlaybackBackend::createController()` - Creates controllers
- `IPlaybackBackend::createMediaSource()` - Creates media sources
- `PlaybackEngine::createController()` - High-level factory

### 2. Observer Pattern
- `IPlaybackController` maintains a list of `IPlaybackEventListener`
- Events are broadcast to all registered listeners

### 3. Strategy Pattern
- Multiple backend implementations (FFmpeg, GStreamer)
- Backend selection based on format support or manual selection

### 4. Dependency Injection
- Controllers receive `IMediaSource` and `PlaybackConfig` as dependencies
- Backends are injected into `PlaybackEngine`

## Relationships

```
PlaybackEngine
    ├── Manages: IPlaybackBackend (many)
    ├── Creates: IPlaybackController
    └── Uses: IMediaSource, PlaybackConfig

IPlaybackBackend
    ├── Creates: IPlaybackController, IMediaSource
    └── Uses: MediaFormat, PlaybackConfig

IPlaybackController
    ├── Notifies: IPlaybackEventListener (many)
    ├── Uses: MediaFormat, PlaybackConfig
    └── Returns: PlaybackStats, PlaybackState

IMediaSource
    └── Returns: MediaFormat, AdaptiveStream

MediaFormat
    ├── Contains: VideoFormat (optional)
    ├── Contains: AudioFormat (optional)
    └── Uses: MediaType, Codec, ContainerFormat (enums)
```

## Design Strengths

1. **Separation of Concerns**: Clear boundaries between components
2. **Extensibility**: Easy to add new backends without changing the API
3. **Testability**: Interfaces allow for easy mocking
4. **Flexibility**: Supports multiple media sources and formats
5. **Modern C++**: Uses smart pointers, optional, and modern features
6. **Comprehensive Configuration**: Extensive configuration options
7. **Event-Driven**: Reactive architecture with event listeners

## Potential Improvements

1. **Error Handling**: Consider adding exception specifications or error result types
2. **Async Operations**: Some operations (like loading) might benefit from async/await patterns
3. **Builder Pattern**: `PlaybackConfig` could use a builder for easier construction
4. **Type Safety**: Consider using strong types instead of primitives (e.g., `Duration`, `Bitrate`)
5. **Documentation**: Add more detailed Javadoc-style comments
6. **Thread Safety**: Document thread-safety guarantees for each interface

## Usage Flow

1. **Initialization**:
   ```
   PlaybackEngine engine;
   engine.registerBackend(std::make_unique<FFmpegBackend>());
   ```

2. **Media Loading**:
   ```
   auto backend = engine.getBackend("ffmpeg");
   auto source = backend->createMediaSource();
   source->load("https://example.com/video.mp4");
   ```

3. **Controller Creation**:
   ```
   PlaybackConfig config;
   config.bufferSizeMs = 20000;
   auto controller = engine.createAutoController(source, config);
   ```

4. **Event Handling**:
   ```
   auto listener = std::make_shared<MyEventListener>();
   controller->addEventListener(listener);
   ```

5. **Playback Control**:
   ```
   controller->play();
   controller->setVolume(0.8);
   controller->seek(5000); // 5 seconds
   ```

## Enums Summary

- **MediaType**: AUDIO, VIDEO, SUBTITLE
- **Codec**: H264, H265, VP9, AV1, AAC, OPUS, MP3, UNKNOWN
- **ContainerFormat**: MP4, MKV, WEBM, MPEGTS, DASH, HLS, UNKNOWN
- **PlaybackState**: IDLE, BUFFERING, PLAYING, PAUSED, STOPPED, ENDED, ERROR
- **PlaybackError**: NONE, NETWORK_ERROR, DECODE_ERROR, FORMAT_ERROR, DRM_ERROR, BUFFER_UNDERRUN
- **PlaybackEvent::Type**: STATE_CHANGED, POSITION_CHANGED, BUFFERING_PROGRESS, QUALITY_CHANGED, ERROR_OCCURRED, METADATA_UPDATED, SEEK_COMPLETED

