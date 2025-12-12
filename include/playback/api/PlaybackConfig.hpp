#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <string>

namespace playback {

struct PlaybackConfig {
    // Network configuration
    uint32_t bufferSizeMs = 15000;
    uint32_t maxBufferSizeMs = 30000;
    uint32_t networkTimeoutMs = 10000;
    uint32_t maxRetries = 3;

    // Video configuration
    bool hardwareAcceleration = true;
    uint32_t maxVideoWidth = 3840;
    uint32_t maxVideoHeight = 2160;
    double maxFrameRate = 60.0;

    // Audioi configuration
    uint32_t audioSampleRate = 48000;
    uint32_t audioChannels = 2;

    // Adaptive stream configuration
    bool adaptiveBitrate = true;
    uint32_t initialBitrate = 2000000;  // 2 Mbps
    uint32_t maxBitrate = 20000000;     // 20 Mbps

    // Quality configuration
    double qualitySwitchThreshold = 0.75;
    uint32_t qualitySwitchCooldownMs = 5000;

    // Subtitle configuration
    bool enableSubtitles = true;
    std::string defaultSubtitleLanguage = "en";

    // Cache configuration
    size_t cacheSizeBytes = 100 * 1024 * 1024;  // 100 MB
    bool persistentCache = false;
    std::string cacheDirectory;

    // custom callbacks
    std::function<void(const std::string&)> logCallback;
    std::function<void(int64_t, int64_t)> downloadProgressCallback;

    std::map<std::string, std::string> backendOptions;
};

}  // namespace playback