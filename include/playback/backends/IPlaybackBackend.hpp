#pragma once
#include <memory>

#include "../api/IMediaSource.hpp"
#include "../api/IPlaybackController.hpp"
#include "../api/PlaybackConfig.hpp"

namespace playback {

class IPlaybackBackend {
   public:
    virtual ~IPlaybackBackend() = default;

    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual std::string getVersion() const = 0;

    virtual std::unique_ptr<IPlaybackController> createController(
        std::shared_ptr<IMediaSource> source, const PlaybackConfig& config) = 0;

    virtual std::shared_ptr<IMediaSource> createMediaSource() = 0;

    [[nodiscard]] virtual bool isSupported(const MediaFormat& format) const = 0;
    [[nodiscard]] virtual bool isHardwareAccelerationSupported() const = 0;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
};

}  // namespace playback
