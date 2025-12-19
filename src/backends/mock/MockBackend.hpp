#pragma once

#include <memory>
#include <string>

#include "playback/api/IMediaSource.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/MediaFormat.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/IPlaybackBackend.hpp"

namespace playback {

// Mock Backend Implementation
class MockBackend : public IPlaybackBackend {
   public:
    MockBackend();
    ~MockBackend() override = default;

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
    bool initialized_;
};

}  // namespace playback
