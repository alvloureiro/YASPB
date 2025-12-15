#include "playback/backends/apple/ApplePlaybackController.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <vector>

#include "playback/api/IPlaybackEventListener.hpp"
#include "playback/api/MediaFormat.hpp"

#ifdef __APPLE__

namespace playback {

ApplePlaybackController::ApplePlaybackController(std::shared_ptr<IMediaSource> source,
                                                 const PlaybackConfig& config)
    : source_(source), config_(config), sessionId_(generateSessionId()) {
    player_ = std::make_unique<apple::AVFPlayerWrapper>();

    apple::AVFCallbacks callbacks;
    callbacks.onStateChanged = [this](PlaybackState state) { notifyStateChanged(state); };
    callbacks.onPositionChanged = [this](uint64_t pos) { notifyPositionChanged(pos); };
    callbacks.onBufferProgress = [this](double progress) { notifyBufferProgress(progress); };
    callbacks.onError = [this](const std::string& error) { notifyError(error); };
    callbacks.onVideoFormatChanged = [this](const VideoFormat& format) {
        notifyVideoFormatChanged(format);
    };
    callbacks.onAudioFormatChanged = [this](const AudioFormat& format) {
        notifyAudioFormatChanged(format);
    };

    player_->setCallbacks(callbacks);

    // Load media from source if available
    // Note: The source needs to be loaded first with load() before creating the controller
    // Or use loadMedia() method after creation
}

// IPlaybackController implementation
bool ApplePlaybackController::play() {
    return player_->play();
}

bool ApplePlaybackController::pause() {
    return player_->pause();
}

bool ApplePlaybackController::stop() {
    // AVFoundation não tem stop nativo, fazemos pause + seek para início
    player_->pause();
    player_->seek(0);
    return true;
}

bool ApplePlaybackController::seek(uint64_t positionMs) {
    return player_->seek(positionMs);
}

bool ApplePlaybackController::setVolume(double volume) {
    std::cout << "ApplePlaybackController::setVolume " << volume << std::endl;
    return player_->setVolume(volume);
}

double ApplePlaybackController::getVolume() const {
    return player_->getVolume();
}

PlaybackState ApplePlaybackController::getState() const {
    return player_->getState();
}

uint64_t ApplePlaybackController::getCurrentPosition() const {
    return player_->getCurrentPosition();
}

uint64_t ApplePlaybackController::getDuration() const {
    return player_->getDuration();
}

bool ApplePlaybackController::setPlaybackRate(double rate) {
    return player_->setPlaybackRate(rate);
}

double ApplePlaybackController::getPlaybackRate() const {
    return player_->getPlaybackRate();
}

bool ApplePlaybackController::setQuality(const MediaFormat& format) {
    // AVFoundation handles quality automatically
    // Store current quality for reporting
    currentQuality_ = format;
    notifyQualityChanged();
    return true;
}

MediaFormat ApplePlaybackController::getCurrentQuality() const {
    if (player_) {
        return player_->getCurrentFormat();
    }
    return currentQuality_;
}

std::vector<MediaFormat> ApplePlaybackController::getAvailableQualities() const {
    if (player_) {
        return player_->getAvailableFormats();
    }
    if (source_) {
        return source_->getAvailableFormats();
    }
    return {};
}

IPlaybackController::PlaybackStats ApplePlaybackController::getStats() const {
    PlaybackStats stats{};
    if (player_) {
        // Get stats from player if available
        // For now, return default stats
        stats.networkBandwidth = 0.0;
        stats.framesPerSecond = 30.0;
        stats.droppedFrames = 0.0;
        stats.bytesDownloaded = 0;
        stats.bufferHealth = 1.0;
    }
    return stats;
}

bool ApplePlaybackController::configure(const PlaybackConfig& config) {
    config_ = config;
    // Apply configuration to player if needed
    return true;
}

PlaybackConfig ApplePlaybackController::getConfig() const {
    return config_;
}

void ApplePlaybackController::addEventListener(std::shared_ptr<IPlaybackEventListener> listener) {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    listeners_.push_back(listener);
}

void ApplePlaybackController::removeEventListener(
    std::shared_ptr<IPlaybackEventListener> listener) {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), listener), listeners_.end());
}

std::string ApplePlaybackController::getSessionId() const {
    return sessionId_;
}

bool ApplePlaybackController::loadMedia(const std::string& filePathOrUrl) {
    if (!player_) {
        return false;
    }

    // AVFPlayerWrapper::load() can handle both file paths and URLs
    // It will automatically detect file paths and convert them properly
    return player_->load(filePathOrUrl);
}

// Private helper methods
void ApplePlaybackController::notifyStateChanged(PlaybackState state) {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::STATE_CHANGED;
    event.state = state;

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto& listener : listeners_) {
        if (listener) {
            listener->onPlaybackEvent(event);
        }
    }
}

void ApplePlaybackController::notifyPositionChanged(uint64_t position) {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::POSITION_CHANGED;
    event.state = getState();
    event.error = PlaybackError::NONE;
    event.positionMs = position;
    event.durationMs = getDuration();

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto& listener : listeners_) {
        if (listener) {
            listener->onPlaybackEvent(event);
        }
    }
}

void ApplePlaybackController::notifyBufferProgress(double progress) {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::BUFFERING_PROGRESS;
    event.state = getState();
    event.error = PlaybackError::NONE;
    event.bufferLevel = progress;

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto& listener : listeners_) {
        if (listener) {
            listener->onPlaybackEvent(event);
        }
    }
}

void ApplePlaybackController::notifyError(const std::string& errorMsg) {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::ERROR_OCCURRED;
    event.state = PlaybackState::ERROR;
    event.error = PlaybackError::NETWORK_ERROR;  // Default, could be more specific
    event.message = errorMsg;

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto& listener : listeners_) {
        if (listener) {
            listener->onPlaybackEvent(event);
        }
    }
}

void ApplePlaybackController::notifyVideoFormatChanged(const VideoFormat& format) {
    (void)format;  // Unused parameter - format info could be added to event in future
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::QUALITY_CHANGED;
    event.state = getState();
    event.error = PlaybackError::NONE;

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto& listener : listeners_) {
        if (listener) {
            listener->onPlaybackEvent(event);
        }
    }
}

void ApplePlaybackController::notifyAudioFormatChanged(const AudioFormat& format) {
    (void)format;  // Unused parameter - format info could be added to event in future
    // Similar to video format changed
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::METADATA_UPDATED;
    event.state = getState();
    event.error = PlaybackError::NONE;

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto& listener : listeners_) {
        if (listener) {
            listener->onPlaybackEvent(event);
        }
    }
}

void ApplePlaybackController::notifyQualityChanged() {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::QUALITY_CHANGED;
    event.state = getState();
    event.error = PlaybackError::NONE;
    event.positionMs = getCurrentPosition();
    event.durationMs = getDuration();

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto& listener : listeners_) {
        if (listener) {
            listener->onPlaybackEvent(event);
        }
    }
}

std::string ApplePlaybackController::generateSessionId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::string id = "apple-";
    for (int i = 0; i < 8; ++i) {
        id += "0123456789abcdef"[dis(gen)];
    }
    return id;
}

}  // namespace playback

#endif  // __APPLE__
