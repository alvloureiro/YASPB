#include "MockPlaybackController.hpp"

#include <algorithm>
#include <chrono>
#include <random>
#include <thread>
#include <utility>

#include "playback/api/IMediaSource.hpp"

namespace playback {

MockPlaybackController::MockPlaybackController(std::shared_ptr<IMediaSource> source,
                                               PlaybackConfig config)
    : source_(std::move(source)), config_(std::move(config)), state_(PlaybackState::IDLE),
      positionMs_(0), playbackRate_(1.0), volume_(1.0), running_(false) {
    sessionId_ = generateSessionId();
}

MockPlaybackController::~MockPlaybackController() {
    // RAII: Ensure thread is properly cleaned up
    // ThreadGuard destructor will automatically join the thread
    // But we signal it to stop first
    running_ = false;
    // ThreadGuard destructor will handle joining
}

bool MockPlaybackController::play() {
    if (state_ == PlaybackState::PLAYING) {
        return true;
    }

    state_ = PlaybackState::BUFFERING;
    notifyStateChange();

    // Simulate buffering
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // RAII: Set running flag before creating thread for exception safety
    running_ = true;

    try {
        state_ = PlaybackState::PLAYING;
        // ThreadGuard ensures thread is properly managed
        playbackThread_.start(&MockPlaybackController::playbackLoop, this);
        notifyStateChange();
    } catch (...) {
        // If thread creation fails, reset state
        running_ = false;
        state_ = PlaybackState::STOPPED;
        throw;
    }

    return true;
}

bool MockPlaybackController::pause() {
    if (state_ == PlaybackState::PLAYING) {
        state_ = PlaybackState::PAUSED;
        notifyStateChange();
        return true;
    }
    return false;
}

bool MockPlaybackController::stop() {
    // Set running to false first to signal thread to stop
    running_ = false;

    // RAII: ThreadGuard will join automatically, but we do it explicitly here
    // to ensure state is updated after thread stops
    playbackThread_.join();

    state_ = PlaybackState::STOPPED;
    positionMs_ = 0;
    notifyStateChange();
    return true;
}

bool MockPlaybackController::seek(uint64_t positionMs) {
    if (!source_->isSeekable()) {
        return false;
    }

    positionMs_ = positionMs;
    notifySeekCompleted();
    return true;
}

bool MockPlaybackController::setPlaybackRate(double rate) {
    if (rate > 0.0 && rate <= 4.0) {
        playbackRate_ = rate;
        return true;
    }
    return false;
}

double MockPlaybackController::getPlaybackRate() const {
    return playbackRate_;
}

bool MockPlaybackController::setVolume(double volume) {
    if (volume >= 0.0 && volume <= 1.0) {
        volume_ = volume;
        return true;
    }
    return false;
}

double MockPlaybackController::getVolume() const {
    return volume_;
}

bool MockPlaybackController::setQuality(const MediaFormat& format) {
    currentQuality_ = format;
    notifyQualityChanged();
    return true;
}

MediaFormat MockPlaybackController::getCurrentQuality() const {
    return currentQuality_;
}

std::vector<MediaFormat> MockPlaybackController::getAvailableQualities() const {
    return source_->getAvailableFormats();
}

IPlaybackController::PlaybackStats MockPlaybackController::getStats() const {
    PlaybackStats stats;
    stats.networkBandwidth = 5000000.0;  // 5 Mbps
    stats.framesPerSecond = 30.0;
    stats.droppedFrames = 0.0;
    stats.bytesDownloaded = positionMs_ * 1000;  // Simulated
    stats.bufferHealth = 0.95;
    return stats;
}

PlaybackState MockPlaybackController::getState() const {
    return state_;
}

uint64_t MockPlaybackController::getCurrentPosition() const {
    return positionMs_;
}

uint64_t MockPlaybackController::getDuration() const {
    return source_->getDurationMs();
}

bool MockPlaybackController::configure(const PlaybackConfig& config) {
    config_ = config;
    return true;
}

PlaybackConfig MockPlaybackController::getConfig() const {
    return config_;
}

void MockPlaybackController::addEventListener(std::shared_ptr<IPlaybackEventListener> listener) {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    listeners_.push_back(listener);
}

void MockPlaybackController::removeEventListener(std::shared_ptr<IPlaybackEventListener> listener) {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), listener), listeners_.end());
}

std::string MockPlaybackController::getSessionId() const {
    return sessionId_;
}

void MockPlaybackController::playbackLoop() {
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

void MockPlaybackController::notifyStateChange() {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::STATE_CHANGED;
    event.state = state_;
    event.error = PlaybackError::NONE;
    event.positionMs = positionMs_;
    event.durationMs = source_->getDurationMs();
    notifyListeners(event);
}

void MockPlaybackController::notifyPositionChanged() {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::POSITION_CHANGED;
    event.state = state_;
    event.error = PlaybackError::NONE;
    event.positionMs = positionMs_;
    event.durationMs = source_->getDurationMs();
    notifyListeners(event);
}

void MockPlaybackController::notifySeekCompleted() {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::SEEK_COMPLETED;
    event.state = state_;
    event.error = PlaybackError::NONE;
    event.positionMs = positionMs_;
    event.durationMs = source_->getDurationMs();
    notifyListeners(event);
}

void MockPlaybackController::notifyQualityChanged() {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::QUALITY_CHANGED;
    event.state = state_;
    event.error = PlaybackError::NONE;
    event.positionMs = positionMs_;
    event.durationMs = source_->getDurationMs();
    notifyListeners(event);
}

void MockPlaybackController::notifyListeners(const PlaybackEvent& event) {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (const auto& listener : listeners_) {
        if (listener) {
            listener->onPlaybackEvent(event);
        }
    }
}

std::string MockPlaybackController::generateSessionId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::string id = "mock-";
    for (int i = 0; i < 8; ++i) {
        id += "0123456789abcdef"[dis(gen)];
    }
    return id;
}

}  // namespace playback
