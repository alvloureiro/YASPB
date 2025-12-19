#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "playback/api/IMediaSource.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/IPlaybackEventListener.hpp"
#include "playback/api/MediaFormat.hpp"
#include "playback/api/PlaybackConfig.hpp"

// Forward declarations for FFmpeg types (in global namespace)
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

namespace playback {

// Forward declarations
class IPlaybackEventListener;
struct VideoFormat;
struct AudioFormat;

// FFmpeg Playback Controller implementation
// This class implements IPlaybackController using FFmpeg
class FFmpegPlaybackController : public IPlaybackController {
   public:
    FFmpegPlaybackController(std::shared_ptr<IMediaSource> source, const PlaybackConfig& config);
    ~FFmpegPlaybackController() override;

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
    void notifyError(PlaybackError error, const std::string& message);

// Audio output helpers
#ifdef __APPLE__
    void outputAudioFrame(::AVFrame* frame);
    void setupAudioOutput();
    void cleanupAudioOutput();
#endif

    std::shared_ptr<IPlaybackEventListener> getFirstListener() const;

    // FFmpeg-related members (opaque pointers)
    ::AVFormatContext* formatContext_;
    ::AVCodecContext* videoCodecContext_;
    ::AVCodecContext* audioCodecContext_;

    int videoStreamIndex_;
    int audioStreamIndex_;

// Audio output (platform-specific)
#ifdef __APPLE__
    void* audioQueue_;  // AudioQueueRef (opaque pointer)
#endif

    bool initializeFFmpeg();
    void cleanupFFmpeg();
    bool openMedia();
    void closeMedia();

    // Playback state
    std::shared_ptr<IMediaSource> source_;
    PlaybackConfig config_;
    PlaybackState state_;
    std::atomic<uint64_t> currentPositionMs_;
    std::atomic<uint64_t> durationMs_;
    std::atomic<double> playbackRate_;
    std::atomic<double> volume_;
    MediaFormat currentQuality_;
    std::vector<MediaFormat> availableQualities_;

    // Threading
    std::thread playbackThread_;
    std::atomic<bool> shouldStop_;
    mutable std::mutex stateMutex_;

    // Event listeners
    std::vector<std::weak_ptr<IPlaybackEventListener>> listeners_;
    mutable std::mutex listenersMutex_;

    // Session ID
    std::string sessionId_;

    // Statistics
    mutable std::mutex statsMutex_;
    IPlaybackController::PlaybackStats stats_;
};

}  // namespace playback
