#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../../api/IPlaybackEventListener.hpp"
#include "../../api/MediaFormat.hpp"

#ifdef __APPLE__

// Forward declaration for PlaybackState (defined in IPlaybackEventListener.hpp)
namespace playback {
enum class PlaybackState;
struct VideoFormat;
struct AudioFormat;
}  // namespace playback

namespace playback::apple {

// Forward declaration da classe Objective-C
class AVFPlayerImpl;

struct AVFCallbacks {
    std::function<void(PlaybackState)> onStateChanged;
    std::function<void(uint64_t)> onPositionChanged;
    std::function<void(double)> onBufferProgress;
    std::function<void(const std::string&)> onError;
    std::function<void(const VideoFormat&)> onVideoFormatChanged;
    std::function<void(const AudioFormat&)> onAudioFormatChanged;
};

// Wrapper C++ puro para AVFoundation
class AVFPlayerWrapper {
   public:
    AVFPlayerWrapper();
    ~AVFPlayerWrapper();

    bool load(const std::string& url);
    bool loadFromData(const std::vector<uint8_t>& data);

    bool play();
    bool pause();
    bool stop();
    bool seek(uint64_t positionMs);

    bool setVolume(double volume);
    double getVolume() const;

    bool setPlaybackRate(double rate);
    double getPlaybackRate() const;

    PlaybackState getState() const;
    uint64_t getCurrentPosition() const;
    uint64_t getDuration() const;

    MediaFormat getCurrentFormat() const;
    std::vector<MediaFormat> getAvailableFormats() const;

    void setAudioSessionCategory(const std::string& category);
    void setAudioSessionMode(const std::string& mode);

    // HDR/Dolby Vision
    bool supportsHDR() const;
    bool supportsDolbyVision() const;

    // DRM (FairPlay)
    bool hasDRM() const;
    bool setDRMLicense(const std::string& license);

    // Callbacks
    void setCallbacks(const AVFCallbacks& callbacks);

    void* getMetalLayer();           // Returns CAMetalLayer*
    void setOutputView(void* view);  // NSView/UIView

   private:
    // PIMPL Objective-C
    std::unique_ptr<AVFPlayerImpl> impl_;

    AVFPlayerWrapper(const AVFPlayerWrapper&) = delete;
    AVFPlayerWrapper& operator=(const AVFPlayerWrapper&) = delete;
};

}  // namespace playback::apple

#endif  // __APPLE__
