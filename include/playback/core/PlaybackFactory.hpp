#pragma once
#include <memory>

#include "../api/PlaybackConfig.hpp"

namespace playback {

class PlaybackEngine;
class IPlaybackBackend;

class PlaybackFactory {
   public:
    static std::unique_ptr<PlaybackEngine> createEngine();

    // Methods to create specific backends
#ifdef ENABLE_MOCK_BACKEND
    static std::unique_ptr<IPlaybackBackend> createMockBackend();
#endif
#ifdef ENABLE_FFMPEG_BACKEND
    static std::unique_ptr<IPlaybackBackend> createFFmpegBackend();
#endif
    static std::unique_ptr<IPlaybackBackend> createGStreamerBackend();
#ifdef ENABLE_AVFOUNDATION_BACKEND
    static std::unique_ptr<IPlaybackBackend> createAppleBackend();
#endif

    // Creation of default configurations for different use cases
    static PlaybackConfig createLowLatencyConfig();
    static PlaybackConfig createHighQualityConfig();
    static PlaybackConfig createMobileConfig();

    // Configuration validation
    static bool validateConfig(const PlaybackConfig& config);
};

}  // namespace playback