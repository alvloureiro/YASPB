#pragma once

#include <memory>

#include "MediaFormat.hpp"

namespace playback {

enum class PlaybackState { IDLE, BUFFERING, PLAYING, PAUSED, STOPPED, ENDED, ERROR };

enum class PlaybackError {
    NONE,
    NETWORK_ERROR,
    DECODE_ERROR,
    FORMAT_ERROR,
    DRM_ERROR,
    BUFFER_UNDERRUN
};

struct PlaybackEvent {
    enum class Type {
        STATE_CHANGED,
        POSITION_CHANGED,
        BUFFERING_PROGRESS,
        QUALITY_CHANGED,
        ERROR_OCCURRED,
        METADATA_UPDATED,
        SEEK_COMPLETED
    };

    Type type;
    PlaybackState state;
    PlaybackError error;
    uint64_t positionMs;
    uint64_t durationMs;
    double bufferLevel;
    std::string message;
};

class IPlaybackEventListener {
   public:
    virtual ~IPlaybackEventListener() = default;

    virtual void onPlaybackEvent(const PlaybackEvent& event) = 0;
    virtual void onVideoFrame(const void* data, size_t size, const VideoFormat& format) = 0;
    virtual void onAudioData(const void* data, size_t size, const AudioFormat& format) = 0;
};

}  // namespace playback
