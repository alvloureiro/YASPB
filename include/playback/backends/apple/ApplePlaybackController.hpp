#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "playback/api/IMediaSource.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/apple/AVFoundationWrapper.hpp"

#ifdef __APPLE__

namespace playback {

// Forward declarations
class IPlaybackEventListener;
struct VideoFormat;
struct AudioFormat;

// Apple Playback Controller implementation
// This class implements IPlaybackController using AVFoundation
class ApplePlaybackController : public IPlaybackController {
   public:
    ApplePlaybackController(std::shared_ptr<IMediaSource> source, const PlaybackConfig& config);
    ~ApplePlaybackController() override = default;

    // IPlaybackController interface
    bool play() override;
    bool pause() override;
    bool stop() override;
    bool seek(uint64_t positionMs) override;

    bool setPlaybackRate(double rate) override;
    double getPlaybackRate() const override;

    bool setVolume(double volume) override;
    double getVolume() const override;

    bool setQuality(const MediaFormat& format) override;
    MediaFormat getCurrentQuality() const override;
    std::vector<MediaFormat> getAvailableQualities() const override;

    PlaybackStats getStats() const override;

    PlaybackState getState() const override;
    uint64_t getCurrentPosition() const override;
    uint64_t getDuration() const override;

    bool configure(const PlaybackConfig& config) override;
    PlaybackConfig getConfig() const override;

    void addEventListener(std::shared_ptr<IPlaybackEventListener> listener) override;
    void removeEventListener(std::shared_ptr<IPlaybackEventListener> listener) override;

    std::string getSessionId() const override;

   private:
    // Private helper methods
    void notifyStateChanged(PlaybackState state);
    void notifyPositionChanged(uint64_t position);
    void notifyBufferProgress(double progress);
    void notifyError(const std::string& errorMsg);
    void notifyVideoFormatChanged(const VideoFormat& format);
    void notifyAudioFormatChanged(const AudioFormat& format);
    void notifyQualityChanged();
    std::string generateSessionId();

    std::unique_ptr<apple::AVFPlayerWrapper> player_;
    std::shared_ptr<IMediaSource> source_;
    PlaybackConfig config_;
    MediaFormat currentQuality_;
    std::string sessionId_;

    mutable std::mutex listenersMutex_;
    std::vector<std::shared_ptr<IPlaybackEventListener>> listeners_;
};

}  // namespace playback

#endif  // __APPLE__
