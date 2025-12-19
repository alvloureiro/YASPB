#pragma once
#include <cstddef>
#include <functional>
#include <map>
#include <string>

namespace playback {

struct PlaybackConfig {
    // Default values for configuration
    static constexpr uint32_t DEFAULT_BUFFER_SIZE_MS = 15000;
    static constexpr uint32_t DEFAULT_MAX_BUFFER_SIZE_MS = 30000;
    static constexpr uint32_t DEFAULT_NETWORK_TIMEOUT_MS = 10000;
    static constexpr uint32_t DEFAULT_MAX_RETRIES = 3;
    static constexpr uint32_t DEFAULT_MAX_VIDEO_WIDTH = 3840;
    static constexpr uint32_t DEFAULT_MAX_VIDEO_HEIGHT = 2160;
    static constexpr double DEFAULT_MAX_FRAME_RATE = 60.0;
    static constexpr uint32_t DEFAULT_AUDIO_SAMPLE_RATE = 48000;
    static constexpr uint32_t DEFAULT_AUDIO_CHANNELS = 2;
    static constexpr uint32_t DEFAULT_INITIAL_BITRATE = 2000000;  // 2 Mbps
    static constexpr uint32_t DEFAULT_MAX_BITRATE = 20000000;     // 20 Mbps
    static constexpr double DEFAULT_QUALITY_SWITCH_THRESHOLD = 0.75;
    static constexpr uint32_t DEFAULT_QUALITY_SWITCH_COOLDOWN_MS = 5000;
    static constexpr size_t DEFAULT_CACHE_SIZE_BYTES =
        static_cast<const size_t>(100 * 1024 * 1024);  // 100 MB

    // Network configuration
    uint32_t bufferSizeMs = DEFAULT_BUFFER_SIZE_MS;
    uint32_t maxBufferSizeMs = DEFAULT_MAX_BUFFER_SIZE_MS;
    uint32_t networkTimeoutMs = DEFAULT_NETWORK_TIMEOUT_MS;
    uint32_t maxRetries = DEFAULT_MAX_RETRIES;

    // Video configuration
    bool hardwareAcceleration = true;
    uint32_t maxVideoWidth = DEFAULT_MAX_VIDEO_WIDTH;
    uint32_t maxVideoHeight = DEFAULT_MAX_VIDEO_HEIGHT;
    double maxFrameRate = DEFAULT_MAX_FRAME_RATE;

    // Audio configuration
    uint32_t audioSampleRate = DEFAULT_AUDIO_SAMPLE_RATE;
    uint32_t audioChannels = DEFAULT_AUDIO_CHANNELS;

    // Adaptive stream configuration
    bool adaptiveBitrate = true;
    uint32_t initialBitrate = DEFAULT_INITIAL_BITRATE;
    uint32_t maxBitrate = DEFAULT_MAX_BITRATE;

    // Quality configuration
    double qualitySwitchThreshold = DEFAULT_QUALITY_SWITCH_THRESHOLD;
    uint32_t qualitySwitchCooldownMs = DEFAULT_QUALITY_SWITCH_COOLDOWN_MS;

    // Subtitle configuration
    bool enableSubtitles = true;
    std::string defaultSubtitleLanguage = "en";

    // Cache configuration
    size_t cacheSizeBytes = DEFAULT_CACHE_SIZE_BYTES;
    bool persistentCache = false;
    std::string cacheDirectory;

    // custom callbacks
    std::function<void(const std::string&)> logCallback;
    std::function<void(int64_t, int64_t)> downloadProgressCallback;

    std::map<std::string, std::string> backendOptions;
};

}  // namespace playback
