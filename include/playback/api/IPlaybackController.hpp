#pragma once
#include <memory>
#include <vector>

#include "IPlaybackEventListener.hpp"
#include "MediaFormat.hpp"
#include "PlaybackConfig.hpp"

namespace playback {

class IPlaybackController {
   public:
    virtual ~IPlaybackController() = default;

    virtual bool play() = 0;
    virtual bool pause() = 0;
    virtual bool stop() = 0;
    virtual bool seek(uint64_t positionMs) = 0;

    // Playback rate
    virtual bool setPlaybackRate(double rate) = 0;
    [[nodiscard]] virtual double getPlaybackRate() const = 0;

    // Volume controls
    virtual bool setVolume(double volume) = 0;  // 0.0 a 1.0
    [[nodiscard]] virtual double getVolume() const = 0;

    // Quality controls
    virtual bool setQuality(const MediaFormat& format) = 0;
    [[nodiscard]] virtual MediaFormat getCurrentQuality() const = 0;
    [[nodiscard]] virtual std::vector<MediaFormat> getAvailableQualities() const = 0;

    // Statistics
    struct PlaybackStats {
        double networkBandwidth;  // bps
        double framesPerSecond;
        double droppedFrames;
        uint64_t bytesDownloaded;
        double bufferHealth;  // 0.0 to 1.0
    };

    [[nodiscard]] virtual PlaybackStats getStats() const = 0;

    // Current state
    [[nodiscard]] virtual PlaybackState getState() const = 0;
    [[nodiscard]] virtual uint64_t getCurrentPosition() const = 0;
    [[nodiscard]] virtual uint64_t getDuration() const = 0;

    // Configs
    virtual bool configure(const PlaybackConfig& config) = 0;
    [[nodiscard]] virtual PlaybackConfig getConfig() const = 0;

    // Events
    virtual void addEventListener(std::shared_ptr<IPlaybackEventListener> listener) = 0;
    virtual void removeEventListener(std::shared_ptr<IPlaybackEventListener> listener) = 0;

    // Streaming session
    [[nodiscard]] virtual std::string getSessionId() const = 0;
};

}  // namespace playback
