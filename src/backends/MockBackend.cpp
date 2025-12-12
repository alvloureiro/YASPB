#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "playback/api/IMediaSource.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/IPlaybackEventListener.hpp"
#include "playback/api/MediaFormat.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/IPlaybackBackend.hpp"

namespace playback {

// Forward declarations
class MockMediaSource;
class MockPlaybackController;

// Mock Media Source Implementation
class MockMediaSource : public IMediaSource {
   public:
    MockMediaSource() : loaded_(false), durationMs_(0), live_(false), seekable_(true) {}

    bool load(const std::string& sourceUri) override {
        loaded_ = true;
        sourceUri_ = sourceUri;

        // Simulate loading different formats based on URI
        if (sourceUri.find(".mp4") != std::string::npos) {
            durationMs_ = 120000;  // 2 minutes
        } else if (sourceUri.find(".mkv") != std::string::npos) {
            durationMs_ = 180000;  // 3 minutes
        } else {
            durationMs_ = 60000;  // 1 minute default
        }

        return true;
    }

    bool load(const std::vector<uint8_t>& data) override {
        loaded_ = true;
        data_ = data;
        durationMs_ = 60000;  // 1 minute default
        return true;
    }

    std::vector<MediaFormat> getAvailableFormats() const override {
        std::vector<MediaFormat> formats;

        if (loaded_) {
            MediaFormat format;
            format.type = MediaType::VIDEO;
            format.codec = Codec::H264;
            format.container = ContainerFormat::MP4;
            format.mimeType = "video/mp4";
            format.durationMs = durationMs_;

            VideoFormat video;
            video.width = 1920;
            video.height = 1080;
            video.frameRate = 30.0;
            video.bitrate = 5000000;
            video.colorSpace = "yuv420p";
            format.video = video;

            AudioFormat audio;
            audio.sampleRate = 48000;
            audio.channels = 2;
            audio.bitrate = 192000;
            audio.channelLayout = "stereo";
            format.audio = audio;

            formats.push_back(format);
        }

        return formats;
    }

    std::vector<AdaptiveStream> getAdaptiveStreams() const override {
        std::vector<AdaptiveStream> streams;

        if (loaded_) {
            AdaptiveStream stream;
            stream.url = sourceUri_;
            stream.bitrate = 5000000;
            stream.width = 1920;
            stream.height = 1080;
            stream.codecs = "avc1.640028,mp4a.40.2";

            MediaFormat format;
            format.type = MediaType::VIDEO;
            format.codec = Codec::H264;
            format.container = ContainerFormat::MP4;
            format.mimeType = "video/mp4";
            format.durationMs = durationMs_;
            stream.format = format;

            streams.push_back(stream);
        }

        return streams;
    }

    bool isLive() const override {
        return live_;
    }

    bool isSeekable() const override {
        return seekable_;
    }

    uint64_t getDurationMs() const override {
        return durationMs_;
    }

    bool hasDRM() const override {
        return false;
    }

    bool setDRMLicense(const std::string& license) override {
        return false;
    }

    std::string getTitle() const override {
        return "Mock Media Title";
    }

    std::string getArtist() const override {
        return "Mock Artist";
    }

    std::string getAlbum() const override {
        return "Mock Album";
    }

    std::vector<uint8_t> getThumbnail() const override {
        // Return empty thumbnail
        return std::vector<uint8_t>();
    }

   private:
    bool loaded_;
    std::string sourceUri_;
    std::vector<uint8_t> data_;
    uint64_t durationMs_;
    bool live_;
    bool seekable_;
};

// Mock Playback Controller Implementation
class MockPlaybackController : public IPlaybackController {
   public:
    MockPlaybackController(std::shared_ptr<IMediaSource> source, const PlaybackConfig& config)
        : source_(source), config_(config), state_(PlaybackState::IDLE), positionMs_(0),
          playbackRate_(1.0), volume_(1.0), running_(false) {
        sessionId_ = generateSessionId();
    }

    ~MockPlaybackController() {
        stop();
    }

    bool play() override {
        if (state_ == PlaybackState::PLAYING) {
            return true;
        }

        state_ = PlaybackState::BUFFERING;
        notifyStateChange();

        // Simulate buffering
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        state_ = PlaybackState::PLAYING;
        running_ = true;
        playbackThread_ = std::thread(&MockPlaybackController::playbackLoop, this);
        notifyStateChange();

        return true;
    }

    bool pause() override {
        if (state_ == PlaybackState::PLAYING) {
            state_ = PlaybackState::PAUSED;
            notifyStateChange();
            return true;
        }
        return false;
    }

    bool stop() override {
        running_ = false;
        if (playbackThread_.joinable()) {
            playbackThread_.join();
        }

        state_ = PlaybackState::STOPPED;
        positionMs_ = 0;
        notifyStateChange();
        return true;
    }

    bool seek(uint64_t positionMs) override {
        if (!source_->isSeekable()) {
            return false;
        }

        positionMs_ = positionMs;
        notifySeekCompleted();
        return true;
    }

    bool setPlaybackRate(double rate) override {
        if (rate > 0.0 && rate <= 4.0) {
            playbackRate_ = rate;
            return true;
        }
        return false;
    }

    double getPlaybackRate() const override {
        return playbackRate_;
    }

    bool setVolume(double volume) override {
        if (volume >= 0.0 && volume <= 1.0) {
            volume_ = volume;
            return true;
        }
        return false;
    }

    double getVolume() const override {
        return volume_;
    }

    bool setQuality(const MediaFormat& format) override {
        currentQuality_ = format;
        notifyQualityChanged();
        return true;
    }

    MediaFormat getCurrentQuality() const override {
        return currentQuality_;
    }

    std::vector<MediaFormat> getAvailableQualities() const override {
        return source_->getAvailableFormats();
    }

    PlaybackStats getStats() const override {
        PlaybackStats stats;
        stats.networkBandwidth = 5000000.0;  // 5 Mbps
        stats.framesPerSecond = 30.0;
        stats.droppedFrames = 0.0;
        stats.bytesDownloaded = positionMs_ * 1000;  // Simulated
        stats.bufferHealth = 0.95;
        return stats;
    }

    PlaybackState getState() const override {
        return state_;
    }

    uint64_t getCurrentPosition() const override {
        return positionMs_;
    }

    uint64_t getDuration() const override {
        return source_->getDurationMs();
    }

    bool configure(const PlaybackConfig& config) override {
        config_ = config;
        return true;
    }

    PlaybackConfig getConfig() const override {
        return config_;
    }

    void addEventListener(std::shared_ptr<IPlaybackEventListener> listener) override {
        std::lock_guard<std::mutex> lock(listenersMutex_);
        listeners_.push_back(listener);
    }

    void removeEventListener(std::shared_ptr<IPlaybackEventListener> listener) override {
        std::lock_guard<std::mutex> lock(listenersMutex_);
        listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), listener),
                         listeners_.end());
    }

    std::string getSessionId() const override {
        return sessionId_;
    }

   private:
    void playbackLoop() {
        auto lastUpdate = std::chrono::steady_clock::now();

        while (running_ && state_ == PlaybackState::PLAYING) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();

            positionMs_ += static_cast<uint64_t>(elapsed * playbackRate_);

            if (positionMs_ >= source_->getDurationMs()) {
                state_ = PlaybackState::ENDED;
                notifyStateChange();
                running_ = false;
                break;
            }

            notifyPositionChanged();
            lastUpdate = now;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void notifyStateChange() {
        PlaybackEvent event;
        event.type = PlaybackEvent::Type::STATE_CHANGED;
        event.state = state_;
        event.error = PlaybackError::NONE;
        event.positionMs = positionMs_;
        event.durationMs = source_->getDurationMs();
        notifyListeners(event);
    }

    void notifyPositionChanged() {
        PlaybackEvent event;
        event.type = PlaybackEvent::Type::POSITION_CHANGED;
        event.state = state_;
        event.error = PlaybackError::NONE;
        event.positionMs = positionMs_;
        event.durationMs = source_->getDurationMs();
        notifyListeners(event);
    }

    void notifySeekCompleted() {
        PlaybackEvent event;
        event.type = PlaybackEvent::Type::SEEK_COMPLETED;
        event.state = state_;
        event.error = PlaybackError::NONE;
        event.positionMs = positionMs_;
        event.durationMs = source_->getDurationMs();
        notifyListeners(event);
    }

    void notifyQualityChanged() {
        PlaybackEvent event;
        event.type = PlaybackEvent::Type::QUALITY_CHANGED;
        event.state = state_;
        event.error = PlaybackError::NONE;
        event.positionMs = positionMs_;
        event.durationMs = source_->getDurationMs();
        notifyListeners(event);
    }

    void notifyListeners(const PlaybackEvent& event) {
        std::lock_guard<std::mutex> lock(listenersMutex_);
        for (const auto& listener : listeners_) {
            if (listener) {
                listener->onPlaybackEvent(event);
            }
        }
    }

    std::string generateSessionId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);

        std::string id = "mock-";
        for (int i = 0; i < 8; ++i) {
            id += "0123456789abcdef"[dis(gen)];
        }
        return id;
    }

    std::shared_ptr<IMediaSource> source_;
    PlaybackConfig config_;
    PlaybackState state_;
    uint64_t positionMs_;
    double playbackRate_;
    double volume_;
    MediaFormat currentQuality_;
    std::string sessionId_;

    std::atomic<bool> running_;
    std::thread playbackThread_;

    mutable std::mutex listenersMutex_;
    std::vector<std::shared_ptr<IPlaybackEventListener>> listeners_;
};

// Mock Backend Implementation
class MockBackend : public IPlaybackBackend {
   public:
    MockBackend() : initialized_(false) {}

    std::string getName() const override {
        return "Mock";
    }

    std::string getVersion() const override {
        return "1.0.0";
    }

    std::unique_ptr<IPlaybackController> createController(std::shared_ptr<IMediaSource> source,
                                                          const PlaybackConfig& config) override {
        return std::make_unique<MockPlaybackController>(source, config);
    }

    std::shared_ptr<IMediaSource> createMediaSource() override {
        return std::make_shared<MockMediaSource>();
    }

    bool isSupported(const MediaFormat& /*format*/) const override {
        // Mock backend supports all formats
        return true;
    }

    bool isHardwareAccelerationSupported() const override {
        return false;
    }

    bool initialize() override {
        initialized_ = true;
        return true;
    }

    void shutdown() override {
        initialized_ = false;
    }

   private:
    bool initialized_;
};

}  // namespace playback

// Factory function (optional, for dynamic loading)
// Note: This is not required for static linking, but kept for potential future use
