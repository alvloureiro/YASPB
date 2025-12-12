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
    static std::unique_ptr<IPlaybackBackend> createMockBackend();
    static std::unique_ptr<IPlaybackBackend> createFFmpegBackend();
    static std::unique_ptr<IPlaybackBackend> createGStreamerBackend();

    // Creation of default configurations for different use cases
    static PlaybackConfig createLowLatencyConfig();
    static PlaybackConfig createHighQualityConfig();
    static PlaybackConfig createMobileConfig();

    // Configuration validation
    static bool validateConfig(const PlaybackConfig& config);
};

}  // namespace playback