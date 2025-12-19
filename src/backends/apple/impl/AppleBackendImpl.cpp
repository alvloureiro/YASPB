#include <algorithm>
#include <memory>
#include <vector>

#include "playback/api/IMediaSource.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/MediaFormat.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/apple/AppleBackend.hpp"
#include "playback/backends/apple/ApplePlaybackController.hpp"

#ifdef __APPLE__

namespace playback {

// Forward declaration for AppleBackendImpl
class AppleBackendImpl {
   public:
    AppleBackendImpl() : initialized_(false) {}
    ~AppleBackendImpl() = default;

    bool initialize() {
        initialized_ = true;
        return true;
    }

    void shutdown() {
        initialized_ = false;
    }

    bool isInitialized() const {
        return initialized_;
    }

   private:
    bool initialized_;
};

AppleBackend::AppleBackend() {
    impl_ = std::make_unique<AppleBackendImpl>();
}

AppleBackend::~AppleBackend() = default;

std::string AppleBackend::getName() const {
    return "Apple AVFoundation";
}

std::string AppleBackend::getVersion() const {
    // Return a simple version string without Objective-C dependencies
    // The actual OS version can be obtained at runtime if needed
    return "Apple AVFoundation Backend 1.0.0";
}

std::unique_ptr<IPlaybackController> AppleBackend::createController(
    std::shared_ptr<IMediaSource> source, const PlaybackConfig& config) {
    // ApplePlaybackController is defined in ApplePlaybackController.cpp
    return std::make_unique<ApplePlaybackController>(source, config);
}

std::shared_ptr<IMediaSource> AppleBackend::createMediaSource() {
    // For now, return nullptr - would need to implement AppleMediaSource
    // This should be implemented to create a media source that works with AVFoundation
    return nullptr;
}

bool AppleBackend::isSupported(const MediaFormat& format) const {
    // AVFoundation supports common formats
    // For a more accurate check, this should be moved to AVFoundationWrapper
    // For now, return true for common formats
    const std::vector<std::string> supportedMimeTypes = {
        "video/mp4",
        "video/quicktime",
        "video/x-m4v",
        "audio/mp4",
        "audio/mpeg",
        "audio/aac",
        "application/x-mpegURL",  // HLS
        "video/MP2T"              // HLS segments
    };

    return std::find(supportedMimeTypes.begin(), supportedMimeTypes.end(), format.mimeType) !=
           supportedMimeTypes.end();
}

bool AppleBackend::isHardwareAccelerationSupported() const {
    return true;  // AVFoundation sempre usa hardware acceleration
}

bool AppleBackend::initialize() {
    return impl_->initialize();
}

void AppleBackend::shutdown() {
    impl_->shutdown();
}

// Factory function for PlaybackFactory
// This allows the factory to create AppleBackend instances
// Using C++ linkage (not extern "C") to match the forward declaration
std::unique_ptr<IPlaybackBackend> createAppleBackendFactory() {
    return std::make_unique<AppleBackend>();
}

}  // namespace playback

#endif  // __APPLE__
