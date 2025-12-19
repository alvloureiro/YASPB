#pragma once

#include <memory>

#include "../IPlaybackBackend.hpp"

namespace playback {

// Forward declarations
class FFmpegBackendImpl;

// FFmpeg backend implementation
// This implements IPlaybackBackend using FFmpeg libraries
class FFmpegBackend : public IPlaybackBackend {
   public:
    FFmpegBackend();
    ~FFmpegBackend() override;

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
    std::unique_ptr<FFmpegBackendImpl> impl_;
};

}  // namespace playback
