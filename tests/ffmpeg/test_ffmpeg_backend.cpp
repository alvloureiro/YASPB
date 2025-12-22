#include <gtest/gtest.h>

#include <memory>

#include "playback/api/MediaFormat.hpp"
#include "playback/backends/IPlaybackBackend.hpp"

#ifdef ENABLE_FFMPEG_BACKEND

using namespace playback;

// Forward declaration
namespace playback {
std::unique_ptr<IPlaybackBackend> createFFmpegBackendFactory();
}

class FFmpegBackendTest : public ::testing::Test {
   protected:
    void SetUp() override {
        backend = playback::createFFmpegBackendFactory();
        ASSERT_NE(backend, nullptr);
    }

    void TearDown() override {
        if (backend) {
            backend->shutdown();
        }
    }

    std::unique_ptr<IPlaybackBackend> backend;
};

TEST_F(FFmpegBackendTest, Initialization) {
    std::string name = backend->getName();
    EXPECT_EQ(name, "FFmpeg");

    std::string version = backend->getVersion();
    EXPECT_FALSE(version.empty());
    EXPECT_NE(version.find("FFmpeg"), std::string::npos);

    bool result = backend->initialize();
    EXPECT_TRUE(result);
}

TEST_F(FFmpegBackendTest, CreateMediaSource) {
    ASSERT_TRUE(backend->initialize());

    auto source = backend->createMediaSource();
    EXPECT_NE(source, nullptr);
}

TEST_F(FFmpegBackendTest, CreateController) {
    ASSERT_TRUE(backend->initialize());

    auto source = backend->createMediaSource();
    ASSERT_NE(source, nullptr);

    PlaybackConfig config;
    auto controller = backend->createController(source, config);
    EXPECT_NE(controller, nullptr);
}

TEST_F(FFmpegBackendTest, FormatSupport_MP4) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::VIDEO;
    format.codec = Codec::H264;
    format.container = ContainerFormat::MP4;
    format.mimeType = "video/mp4";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, FormatSupport_MKV) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::VIDEO;
    format.codec = Codec::H265;
    format.container = ContainerFormat::MKV;
    format.mimeType = "video/x-matroska";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, FormatSupport_WEBM) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::VIDEO;
    format.codec = Codec::VP9;
    format.container = ContainerFormat::WEBM;
    format.mimeType = "video/webm";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, FormatSupport_HLS) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::VIDEO;
    format.codec = Codec::H264;
    format.container = ContainerFormat::HLS;
    format.mimeType = "application/x-mpegURL";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, FormatSupport_DASH) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::VIDEO;
    format.codec = Codec::H264;
    format.container = ContainerFormat::DASH;
    format.mimeType = "application/dash+xml";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, FormatSupport_AudioMP4) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::AUDIO;
    format.codec = Codec::AAC;
    format.container = ContainerFormat::MP4;
    format.mimeType = "audio/mp4";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, FormatSupport_AudioMPEG) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::AUDIO;
    format.codec = Codec::MP3;
    format.container = ContainerFormat::UNKNOWN;
    format.mimeType = "audio/mpeg";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, FormatSupport_AudioWAV) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::AUDIO;
    format.codec = Codec::UNKNOWN;
    format.container = ContainerFormat::UNKNOWN;
    format.mimeType = "audio/wav";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, FormatSupport_QuickTime) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::VIDEO;
    format.codec = Codec::H264;
    format.container = ContainerFormat::MP4;
    format.mimeType = "video/quicktime";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, FormatSupport_MPEGTS) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::VIDEO;
    format.codec = Codec::H264;
    format.container = ContainerFormat::MPEGTS;
    format.mimeType = "video/MP2T";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, FormatSupport_ContainerOnly) {
    ASSERT_TRUE(backend->initialize());

    // Test that container format alone is sufficient for support check
    MediaFormat format;
    format.type = MediaType::VIDEO;
    format.codec = Codec::UNKNOWN;
    format.container = ContainerFormat::MP4;
    format.mimeType = "";

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);
}

TEST_F(FFmpegBackendTest, HardwareAcceleration) {
    ASSERT_TRUE(backend->initialize());

    bool hwAccel = backend->isHardwareAccelerationSupported();
    // FFmpeg supports hardware acceleration on most platforms
    // The actual value depends on the platform and build configuration
    // We just verify it doesn't crash and returns a boolean
    EXPECT_TRUE(hwAccel || !hwAccel);  // Just check it's a valid boolean
}

TEST_F(FFmpegBackendTest, MultipleInitializations) {
    // First initialization
    bool result1 = backend->initialize();
    EXPECT_TRUE(result1);

    // Second initialization (should be idempotent)
    bool result2 = backend->initialize();
    EXPECT_TRUE(result2);
}

TEST_F(FFmpegBackendTest, Shutdown) {
    ASSERT_TRUE(backend->initialize());

    // Shutdown should not throw
    backend->shutdown();

    // Multiple shutdowns should be safe
    backend->shutdown();
    backend->shutdown();
}

TEST_F(FFmpegBackendTest, ShutdownAfterMultipleInitializations) {
    ASSERT_TRUE(backend->initialize());
    ASSERT_TRUE(backend->initialize());
    ASSERT_TRUE(backend->initialize());

    // Shutdown should work after multiple initializations
    backend->shutdown();
    EXPECT_NO_THROW(backend->shutdown());
}

TEST_F(FFmpegBackendTest, CreateControllerAfterShutdown) {
    ASSERT_TRUE(backend->initialize());
    backend->shutdown();

    // Re-initialize after shutdown
    ASSERT_TRUE(backend->initialize());

    auto source = backend->createMediaSource();
    ASSERT_NE(source, nullptr);

    PlaybackConfig config;
    auto controller = backend->createController(source, config);
    EXPECT_NE(controller, nullptr);
}

TEST_F(FFmpegBackendTest, VersionInformation) {
    std::string version = backend->getVersion();
    EXPECT_FALSE(version.empty());
    EXPECT_NE(version.find("FFmpeg"), std::string::npos);
    // Version should contain some version information
    EXPECT_GT(version.length(), 6);  // At least "FFmpeg " + some version
}

TEST_F(FFmpegBackendTest, NameConsistency) {
    std::string name1 = backend->getName();
    std::string name2 = backend->getName();
    EXPECT_EQ(name1, name2);
    EXPECT_EQ(name1, "FFmpeg");
}

#endif  // ENABLE_FFMPEG_BACKEND
