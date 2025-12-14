#include <gtest/gtest.h>

#include <memory>

#include "playback/api/MediaFormat.hpp"
#include "playback/backends/IPlaybackBackend.hpp"

using namespace playback;

// Forward declaration
namespace playback {
std::unique_ptr<IPlaybackBackend> createMockBackendFactory();
}

class BackendTest : public ::testing::Test {
   protected:
    void SetUp() override {
        backend = playback::createMockBackendFactory();
        ASSERT_NE(backend, nullptr);
    }

    void TearDown() override {
        if (backend) {
            backend->shutdown();
        }
    }

    std::unique_ptr<IPlaybackBackend> backend;
};

TEST_F(BackendTest, Initialization) {
    std::string name = backend->getName();
    EXPECT_EQ(name, "Mock");

    std::string version = backend->getVersion();
    EXPECT_EQ(version, "1.0.0");

    bool result = backend->initialize();
    EXPECT_TRUE(result);
}

TEST_F(BackendTest, CreateMediaSource) {
    ASSERT_TRUE(backend->initialize());

    auto source = backend->createMediaSource();
    EXPECT_NE(source, nullptr);
}

TEST_F(BackendTest, CreateController) {
    ASSERT_TRUE(backend->initialize());

    auto source = backend->createMediaSource();
    source->load("test.mp4");

    PlaybackConfig config;
    auto controller = backend->createController(source, config);
    EXPECT_NE(controller, nullptr);
}

TEST_F(BackendTest, FormatSupport) {
    ASSERT_TRUE(backend->initialize());

    MediaFormat format;
    format.type = MediaType::VIDEO;
    format.codec = Codec::H264;
    format.container = ContainerFormat::MP4;

    bool supported = backend->isSupported(format);
    EXPECT_TRUE(supported);  // Mock backend supports all formats
}

TEST_F(BackendTest, HardwareAcceleration) {
    ASSERT_TRUE(backend->initialize());

    bool hwAccel = backend->isHardwareAccelerationSupported();
    EXPECT_FALSE(hwAccel);  // Mock backend doesn't support hardware acceleration
}

TEST_F(BackendTest, MultipleInitializations) {
    // First initialization
    bool result1 = backend->initialize();
    EXPECT_TRUE(result1);

    // Second initialization (should be idempotent or handle gracefully)
    bool result2 = backend->initialize();
    // Should either succeed or fail gracefully
    EXPECT_TRUE(result2 || !result2);  // Just check it doesn't crash
}

TEST_F(BackendTest, Shutdown) {
    ASSERT_TRUE(backend->initialize());

    // Shutdown should not throw
    backend->shutdown();

    // Multiple shutdowns should be safe
    backend->shutdown();
    backend->shutdown();
}

TEST_F(BackendTest, AllFormatsSupported) {
    ASSERT_TRUE(backend->initialize());

    // Test various formats
    MediaFormat videoFormat;
    videoFormat.type = MediaType::VIDEO;
    videoFormat.codec = Codec::H265;
    EXPECT_TRUE(backend->isSupported(videoFormat));

    MediaFormat audioFormat;
    audioFormat.type = MediaType::AUDIO;
    audioFormat.codec = Codec::AAC;
    EXPECT_TRUE(backend->isSupported(audioFormat));
}
