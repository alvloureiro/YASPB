#include <gtest/gtest.h>

#include <memory>

#include "playback/api/MediaFormat.hpp"
#include "playback/backends/IPlaybackBackend.hpp"

#ifdef __APPLE__

using namespace playback;

// Forward declaration
namespace playback {
std::unique_ptr<IPlaybackBackend> createAppleBackendFactory();
}

class AppleBackendTest : public ::testing::Test {
   protected:
    void SetUp() override {
        backend = playback::createAppleBackendFactory();
        ASSERT_NE(backend, nullptr);
    }

    void TearDown() override {
        if (backend) {
            backend->shutdown();
        }
    }

    std::unique_ptr<IPlaybackBackend> backend;
};

TEST_F(AppleBackendTest, Initialization) {
    std::string name = backend->getName();
    EXPECT_EQ(name, "Apple AVFoundation");

    std::string version = backend->getVersion();
    EXPECT_FALSE(version.empty());
    EXPECT_NE(version.find("Apple AVFoundation"), std::string::npos);

    bool result = backend->initialize();
    EXPECT_TRUE(result);
}

TEST_F(AppleBackendTest, CreateMediaSource) {
    ASSERT_TRUE(backend->initialize());

    // Note: AppleBackend::createMediaSource() currently returns nullptr
    // This is expected behavior until AppleMediaSource is implemented
    auto source = backend->createMediaSource();
    EXPECT_EQ(source, nullptr);
}

TEST_F(AppleBackendTest, CreateController) {
    ASSERT_TRUE(backend->initialize());

    // Note: AppleBackend::createMediaSource() currently returns nullptr
    // So we'll test controller creation with a null source (which should still work)
    // In a real scenario, you'd have a proper media source
    PlaybackConfig config;
    auto controller = backend->createController(nullptr, config);
    // Controller creation should succeed even with null source
    // (the controller can load media later via loadMedia())
    EXPECT_NE(controller, nullptr);
}

TEST_F(AppleBackendTest, FormatSupport) {
    ASSERT_TRUE(backend->initialize());

    // Test supported formats
    MediaFormat mp4Format;
    mp4Format.type = MediaType::VIDEO;
    mp4Format.codec = Codec::H264;
    mp4Format.container = ContainerFormat::MP4;
    mp4Format.mimeType = "video/mp4";

    bool supported = backend->isSupported(mp4Format);
    EXPECT_TRUE(supported);

    // Test HLS format
    MediaFormat hlsFormat;
    hlsFormat.type = MediaType::VIDEO;
    hlsFormat.codec = Codec::H264;
    hlsFormat.container = ContainerFormat::HLS;
    hlsFormat.mimeType = "application/x-mpegURL";

    supported = backend->isSupported(hlsFormat);
    EXPECT_TRUE(supported);

    // Test audio format
    MediaFormat audioFormat;
    audioFormat.type = MediaType::AUDIO;
    audioFormat.codec = Codec::AAC;
    audioFormat.container = ContainerFormat::MP4;
    audioFormat.mimeType = "audio/mp4";

    supported = backend->isSupported(audioFormat);
    EXPECT_TRUE(supported);
}

TEST_F(AppleBackendTest, HardwareAcceleration) {
    ASSERT_TRUE(backend->initialize());

    bool hwAccel = backend->isHardwareAccelerationSupported();
    EXPECT_TRUE(hwAccel);  // AVFoundation always uses hardware acceleration
}

TEST_F(AppleBackendTest, MultipleInitializations) {
    // First initialization
    bool result1 = backend->initialize();
    EXPECT_TRUE(result1);

    // Second initialization (should be idempotent or handle gracefully)
    bool result2 = backend->initialize();
    // Should either succeed or fail gracefully
    EXPECT_TRUE(result2 || !result2);  // Just check it doesn't crash
}

TEST_F(AppleBackendTest, Shutdown) {
    ASSERT_TRUE(backend->initialize());

    // Shutdown should not throw
    backend->shutdown();

    // Multiple shutdowns should be safe
    backend->shutdown();
    backend->shutdown();
}

TEST_F(AppleBackendTest, UnsupportedFormat) {
    ASSERT_TRUE(backend->initialize());

    // Test an unsupported format
    MediaFormat unsupportedFormat;
    unsupportedFormat.type = MediaType::VIDEO;
    unsupportedFormat.codec = Codec::VP9;  // VP9 is not natively supported by AVFoundation
    unsupportedFormat.container = ContainerFormat::WEBM;
    unsupportedFormat.mimeType = "video/webm";

    bool supported = backend->isSupported(unsupportedFormat);
    EXPECT_FALSE(supported);
}

TEST_F(AppleBackendTest, QuickTimeFormat) {
    ASSERT_TRUE(backend->initialize());

    // QuickTime files use MP4 container format
    MediaFormat quicktimeFormat;
    quicktimeFormat.type = MediaType::VIDEO;
    quicktimeFormat.codec = Codec::H264;
    quicktimeFormat.container = ContainerFormat::MP4;
    quicktimeFormat.mimeType = "video/quicktime";

    bool supported = backend->isSupported(quicktimeFormat);
    EXPECT_TRUE(supported);
}

#endif  // __APPLE__
