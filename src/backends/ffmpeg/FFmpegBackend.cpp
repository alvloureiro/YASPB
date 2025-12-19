#include "playback/backends/ffmpeg/FFmpegBackend.hpp"

#include <algorithm>
#include <memory>
#include <vector>

#include "FFmpegMediaSource.hpp"
#include "FFmpegPlaybackController.hpp"
#include "playback/api/IMediaSource.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/MediaFormat.hpp"
#include "playback/api/PlaybackConfig.hpp"

#ifdef ENABLE_FFMPEG_BACKEND

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
}

namespace playback {

// Forward declaration for FFmpegBackendImpl
class FFmpegBackendImpl {
   public:
    FFmpegBackendImpl() : initialized_(false) {}
    ~FFmpegBackendImpl() = default;

    bool initialize() {
        if (initialized_) {
            return true;
        }
        // FFmpeg doesn't require explicit initialization in newer versions
        // but we can check if it's available
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

FFmpegBackend::FFmpegBackend() {
    impl_ = std::make_unique<FFmpegBackendImpl>();
}

FFmpegBackend::~FFmpegBackend() {
    shutdown();
}

std::string FFmpegBackend::getName() const {
    return "FFmpeg";
}

std::string FFmpegBackend::getVersion() const {
    // Return FFmpeg version information
    return std::string("FFmpeg ") + av_version_info();
}

std::unique_ptr<IPlaybackController> FFmpegBackend::createController(
    std::shared_ptr<IMediaSource> source, const PlaybackConfig& config) {
    return std::make_unique<FFmpegPlaybackController>(source, config);
}

std::shared_ptr<IMediaSource> FFmpegBackend::createMediaSource() {
    return std::make_shared<FFmpegMediaSource>();
}

bool FFmpegBackend::isSupported(const MediaFormat& format) const {
    // FFmpeg supports a wide variety of formats
    // Check based on container and codec
    const std::vector<std::string> supportedMimeTypes = {
        "video/mp4",
        "video/x-matroska",
        "video/webm",
        "video/quicktime",
        "video/x-msvideo",
        "video/x-flv",
        "video/mpeg",
        "audio/mp4",
        "audio/mpeg",
        "audio/aac",
        "audio/ogg",
        "audio/webm",
        "audio/wav",
        "audio/flac",
        "application/x-mpegURL",  // HLS
        "application/dash+xml",   // DASH
        "video/MP2T"              // HLS segments
    };

    // Check MIME type
    if (std::find(supportedMimeTypes.begin(), supportedMimeTypes.end(), format.mimeType) !=
        supportedMimeTypes.end()) {
        return true;
    }

    // Also check container format
    switch (format.container) {
        case ContainerFormat::MP4:
        case ContainerFormat::MKV:
        case ContainerFormat::WEBM:
        case ContainerFormat::MPEGTS:
        case ContainerFormat::DASH:
        case ContainerFormat::HLS:
            return true;
        default:
            return false;
    }
}

bool FFmpegBackend::isHardwareAccelerationSupported() const {
    // FFmpeg supports hardware acceleration on various platforms
    // This is a simplified check - actual support depends on build configuration
    // and available hardware
    #ifdef __APPLE__
    // VideoToolbox on macOS/iOS
    return true;
    #elif defined(__linux__)
    // VAAPI, VDPAU, etc. on Linux
    return true;
    #elif defined(_WIN32)
    // DXVA2, D3D11VA on Windows
    return true;
    #else
    return false;
    #endif
}

bool FFmpegBackend::initialize() {
    return impl_->initialize();
}

void FFmpegBackend::shutdown() {
    impl_->shutdown();
}

// Factory function for PlaybackFactory
// This allows the factory to create FFmpegBackend instances
std::unique_ptr<IPlaybackBackend> createFFmpegBackendFactory() {
    return std::make_unique<FFmpegBackend>();
}

}  // namespace playback

#endif  // ENABLE_FFMPEG_BACKEND
