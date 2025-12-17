#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "ThreadGuard.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/IPlaybackEventListener.hpp"
#include "playback/api/MediaFormat.hpp"
#include "playback/api/PlaybackConfig.hpp"

namespace playback {

// Forward declaration
class IMediaSource;

// Mock Playback Controller Implementation
class MockPlaybackController : public IPlaybackController {
   public:
    MockPlaybackController(std::shared_ptr<IMediaSource> source, PlaybackConfig config);
    ~MockPlaybackController() override;

    MockPlaybackController(MockPlaybackController&&) = delete;
    MockPlaybackController& operator=(MockPlaybackController&&) = delete;

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
    void playbackLoop();
    void notifyStateChange();
    void notifyPositionChanged();
    void notifySeekCompleted();
    void notifyQualityChanged();
    void notifyListeners(const PlaybackEvent& event);
    std::string generateSessionId();

    std::shared_ptr<IMediaSource> source_;
    PlaybackConfig config_;
    PlaybackState state_;
    uint64_t positionMs_;
    double playbackRate_;
    double volume_;
    MediaFormat currentQuality_;
    std::string sessionId_;

    std::atomic<bool> running_;
    ThreadGuard playbackThread_;  // RAII: Automatically joins on destruction

    mutable std::mutex listenersMutex_;
    std::vector<std::shared_ptr<IPlaybackEventListener>> listeners_;
};

}  // namespace playback
