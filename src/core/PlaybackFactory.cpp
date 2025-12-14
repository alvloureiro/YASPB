#include "playback/core/PlaybackFactory.hpp"

#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/IPlaybackBackend.hpp"
#include "playback/core/PlaybackEngine.hpp"

namespace playback {

// Forward declarations for factory functions from backends
// These functions are defined in backend implementation files
// They are in the playback namespace in their implementation files
// Only declare if the backend is enabled
#ifdef ENABLE_MOCK_BACKEND
extern std::unique_ptr<IPlaybackBackend> createMockBackendFactory();
#endif

#ifdef ENABLE_AVFOUNDATION_BACKEND
// Apple backend factory (Apple platforms only)
extern std::unique_ptr<IPlaybackBackend> createAppleBackendFactory();
#endif

std::unique_ptr<PlaybackEngine> PlaybackFactory::createEngine() {
    auto engine = std::make_unique<PlaybackEngine>();

    // Register enabled backends
#ifdef ENABLE_MOCK_BACKEND
    // Mock backend (no dependencies)
    if (auto mockBackend = createMockBackend()) {
        engine->registerBackend(std::move(mockBackend));
    }
#endif

#ifdef ENABLE_AVFOUNDATION_BACKEND
    // Apple backend on Apple platforms
    if (auto appleBackend = createAppleBackend()) {
        engine->registerBackend(std::move(appleBackend));
    }
#endif

    return engine;
}

#ifdef ENABLE_MOCK_BACKEND
std::unique_ptr<IPlaybackBackend> PlaybackFactory::createMockBackend() {
    // Use factory function from MockBackend
    // This avoids circular dependency issues
    return createMockBackendFactory();
}
#endif

std::unique_ptr<IPlaybackBackend> PlaybackFactory::createFFmpegBackend() {
    // TODO: Implement FFmpeg backend creation
    // This will be implemented when FFmpeg backend is added
    return nullptr;
}

std::unique_ptr<IPlaybackBackend> PlaybackFactory::createGStreamerBackend() {
    // TODO: Implement GStreamer backend creation
    // This will be implemented when GStreamer backend is added
    return nullptr;
}

#ifdef ENABLE_AVFOUNDATION_BACKEND
std::unique_ptr<IPlaybackBackend> PlaybackFactory::createAppleBackend() {
    return createAppleBackendFactory();
}
#endif

PlaybackConfig PlaybackFactory::createLowLatencyConfig() {
    PlaybackConfig config;
    config.bufferSizeMs = 2000;           // 2 seconds
    config.maxBufferSizeMs = 5000;        // 5 seconds
    config.networkTimeoutMs = 3000;       // 3 seconds
    config.adaptiveBitrate = false;       // Disable adaptive for low latency
    config.qualitySwitchThreshold = 0.5;  // Lower threshold
    return config;
}

PlaybackConfig PlaybackFactory::createHighQualityConfig() {
    PlaybackConfig config;
    config.bufferSizeMs = 30000;     // 30 seconds
    config.maxBufferSizeMs = 60000;  // 60 seconds
    config.maxVideoWidth = 3840;     // 4K
    config.maxVideoHeight = 2160;    // 4K
    config.maxFrameRate = 60.0;
    config.maxBitrate = 50000000;  // 50 Mbps
    config.adaptiveBitrate = true;
    config.qualitySwitchThreshold = 0.9;  // Higher threshold for stability
    return config;
}

PlaybackConfig PlaybackFactory::createMobileConfig() {
    PlaybackConfig config;
    config.bufferSizeMs = 10000;     // 10 seconds
    config.maxBufferSizeMs = 20000;  // 20 seconds
    config.maxVideoWidth = 1920;     // 1080p max
    config.maxVideoHeight = 1080;
    config.maxFrameRate = 30.0;
    config.maxBitrate = 5000000;  // 5 Mbps
    config.adaptiveBitrate = true;
    config.qualitySwitchThreshold = 0.75;
    config.hardwareAcceleration = true;  // Use hardware acceleration on mobile
    return config;
}

bool PlaybackFactory::validateConfig(const PlaybackConfig& config) {
    // Validate buffer sizes
    if (config.bufferSizeMs > config.maxBufferSizeMs) {
        return false;
    }

    // Validate video dimensions
    if (config.maxVideoWidth == 0 || config.maxVideoHeight == 0) {
        return false;
    }

    // Validate frame rate
    if (config.maxFrameRate <= 0.0 || config.maxFrameRate > 120.0) {
        return false;
    }

    // Validate bitrate
    if (config.maxBitrate > 0 && config.initialBitrate > config.maxBitrate) {
        return false;
    }

    // Validate volume-related settings (if any)
    // Validate quality switch threshold
    if (config.qualitySwitchThreshold < 0.0 || config.qualitySwitchThreshold > 1.0) {
        return false;
    }

    return true;
}

}  // namespace playback
