#pragma once

#include <memory>

#include "IPlaybackBackend.hpp"
#include "apple/AVFoundationWrapper.hpp"

namespace playback {

// Forward declarations
class AppleBackendImpl;

// Pure C++ interface for Apple backend
// This implements IPlaybackBackend using AVFoundationWrapper
class AppleBackend : public IPlaybackBackend {
   public:
    AppleBackend();
    ~AppleBackend() override;

    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::string getVersion() const override;

    std::unique_ptr<IPlaybackController> createController(std::shared_ptr<IMediaSource> source,
                                                          const PlaybackConfig& config) override;

    std::shared_ptr<IMediaSource> createMediaSource() override;

    [[nodiscard]] bool isSupported(const MediaFormat& format) const override;
    [[nodiscard]] bool isHardwareAccelerationSupported() const override;

    bool initialize() override;
    void shutdown() override;

   private:
    std::unique_ptr<AppleBackendImpl> impl_;
};

}  // namespace playback
