#include "MockBackend.hpp"

#include "MockMediaSource.hpp"
#include "MockPlaybackController.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/IPlaybackBackend.hpp"

namespace playback {

MockBackend::MockBackend() : initialized_(false) {}

std::string MockBackend::getName() const {
    return "Mock";
}

std::string MockBackend::getVersion() const {
    return "1.0.0";
}

std::unique_ptr<IPlaybackController> MockBackend::createController(
    std::shared_ptr<IMediaSource> source, const PlaybackConfig& config) {
    return std::make_unique<MockPlaybackController>(source, config);
}

std::shared_ptr<IMediaSource> MockBackend::createMediaSource() {
    return std::make_shared<MockMediaSource>();
}

bool MockBackend::isSupported(const MediaFormat& /*format*/) const {
    // Mock backend supports all formats
    return true;
}

bool MockBackend::isHardwareAccelerationSupported() const {
    return false;
}

bool MockBackend::initialize() {
    initialized_ = true;
    return true;
}

void MockBackend::shutdown() {
    initialized_ = false;
}

}  // namespace playback

// Factory function for PlaybackFactory
// This allows the factory to create MockBackend instances
// Using C++ linkage (not extern "C") to match the forward declaration
namespace playback {
std::unique_ptr<IPlaybackBackend> createMockBackendFactory() {
    return std::make_unique<MockBackend>();
}
}  // namespace playback
