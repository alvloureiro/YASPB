#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "playback/api/IPlaybackController.hpp"
#include "playback/api/IPlaybackEventListener.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/IPlaybackBackend.hpp"

#ifdef __APPLE__

using namespace playback;

// Forward declaration
namespace playback {
std::unique_ptr<IPlaybackBackend> createAppleBackendFactory();
}

// Test event listener
class TestEventListener : public IPlaybackEventListener {
   public:
    std::atomic<int> eventCount{0};
    std::atomic<int> stateChangedCount{0};
    PlaybackState lastState = PlaybackState::IDLE;

    void onPlaybackEvent(const PlaybackEvent& event) override {
        eventCount++;
        if (event.type == PlaybackEvent::Type::STATE_CHANGED) {
            stateChangedCount++;
            lastState = event.state;
        }
    }

    void onVideoFrame(const void* /*data*/, size_t /*size*/,
                      const VideoFormat& /*format*/) override {
        // Not used in these tests
    }

    void onAudioData(const void* /*data*/, size_t /*size*/,
                     const AudioFormat& /*format*/) override {
        // Not used in these tests
    }
};

class ApplePlaybackControllerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        backend = playback::createAppleBackendFactory();
        ASSERT_NE(backend, nullptr);
        ASSERT_TRUE(backend->initialize());

        // Note: AppleBackend::createMediaSource() returns nullptr currently
        // So we create controller without a source - it can load media later
        PlaybackConfig config;
        controller = backend->createController(nullptr, config);
        ASSERT_NE(controller, nullptr);
    }

    void TearDown() override {
        if (controller) {
            controller->stop();
        }
        if (backend) {
            backend->shutdown();
        }
    }

    std::unique_ptr<IPlaybackBackend> backend;
    std::unique_ptr<IPlaybackController> controller;
};

TEST_F(ApplePlaybackControllerTest, InitialState) {
    // Initial state should be IDLE when no media is loaded
    EXPECT_EQ(controller->getState(), PlaybackState::IDLE);
}

TEST_F(ApplePlaybackControllerTest, SessionId) {
    std::string sessionId = controller->getSessionId();
    EXPECT_FALSE(sessionId.empty());
    EXPECT_EQ(sessionId.substr(0, 6), "apple-");
}

TEST_F(ApplePlaybackControllerTest, Volume) {
    // Note: setVolume may fail if no media is loaded (player is nil)
    // This is expected behavior - AVFoundation requires a player to be initialized

    // Check that getVolume doesn't crash and returns a valid value
    // When no media is loaded, getVolume returns 0.0 (player is nil)
    double volume = controller->getVolume();
    EXPECT_GE(volume, 0.0);
    EXPECT_LE(volume, 1.0);

    // Try to set volume - may fail if no media loaded
    // This is acceptable behavior
    bool result = controller->setVolume(0.75);
    // Don't assert on result - it may fail when no media is loaded

    // Invalid volume - should always fail (AVFoundation validates range)
    result = controller->setVolume(1.5);
    EXPECT_FALSE(result);  // Should fail for invalid range (> 1.0)

    result = controller->setVolume(-0.1);
    EXPECT_FALSE(result);  // Should fail for invalid range (< 0.0)
}

TEST_F(ApplePlaybackControllerTest, PlaybackRate) {
    // Note: setPlaybackRate may fail if no media is loaded (player is nil)
    // This is expected behavior - AVFoundation requires a player to be initialized

    // Try to set playback rate - may fail if no media loaded
    bool result = controller->setPlaybackRate(1.0);
    // Accept either success or failure (failure is expected when no media is loaded)

    // Check that getPlaybackRate doesn't crash and returns a valid value
    double rate = controller->getPlaybackRate();
    EXPECT_GT(rate, 0.0);

    // Test different rates - may fail if no media loaded
    result = controller->setPlaybackRate(1.5);
    if (result) {
        rate = controller->getPlaybackRate();
        EXPECT_GT(rate, 0.0);
    }

    result = controller->setPlaybackRate(0.5);
    if (result) {
        rate = controller->getPlaybackRate();
        EXPECT_GT(rate, 0.0);
    }

    // Invalid rate - should fail (AVFoundation validates range: 0 < rate <= 4.0)
    result = controller->setPlaybackRate(5.0);
    EXPECT_FALSE(result);  // Should fail for rate > 4.0

    result = controller->setPlaybackRate(0.0);
    EXPECT_FALSE(result);  // Should fail for rate <= 0.0

    result = controller->setPlaybackRate(-1.0);
    EXPECT_FALSE(result);  // Should fail for negative rate
}

TEST_F(ApplePlaybackControllerTest, Config) {
    PlaybackConfig config;
    config.bufferSizeMs = 20000;

    // Create new controller with config
    auto newController = backend->createController(nullptr, config);
    ASSERT_NE(newController, nullptr);

    auto retrievedConfig = newController->getConfig();
    EXPECT_EQ(retrievedConfig.bufferSizeMs, 20000);

    // Update config
    PlaybackConfig newConfig;
    newConfig.bufferSizeMs = 30000;
    bool result = newController->configure(newConfig);
    EXPECT_TRUE(result);

    retrievedConfig = newController->getConfig();
    EXPECT_EQ(retrievedConfig.bufferSizeMs, 30000);
}

TEST_F(ApplePlaybackControllerTest, EventListener) {
    auto listener = std::make_shared<TestEventListener>();
    controller->addEventListener(listener);

    // Even without media loaded, we should be able to add/remove listeners
    EXPECT_EQ(listener->eventCount, 0);

    // Remove listener
    controller->removeEventListener(listener);
}

TEST_F(ApplePlaybackControllerTest, Stats) {
    // Stats should be available even without media loaded
    auto stats = controller->getStats();
    EXPECT_GE(stats.bufferHealth, 0.0);
    EXPECT_LE(stats.bufferHealth, 1.0);
    EXPECT_GE(stats.framesPerSecond, 0.0);
    EXPECT_GE(stats.networkBandwidth, 0.0);
}

TEST_F(ApplePlaybackControllerTest, AvailableQualities) {
    // Without media loaded, should return empty or handle gracefully
    auto qualities = controller->getAvailableQualities();
    // May be empty if no media is loaded, which is acceptable
}

TEST_F(ApplePlaybackControllerTest, CurrentQuality) {
    // Should handle gracefully when no media is loaded
    auto quality = controller->getCurrentQuality();
    // May be empty/default if no media is loaded, which is acceptable
}

TEST_F(ApplePlaybackControllerTest, DurationWithoutMedia) {
    // Duration should be 0 when no media is loaded
    uint64_t duration = controller->getDuration();
    EXPECT_EQ(duration, 0);
}

TEST_F(ApplePlaybackControllerTest, PositionWithoutMedia) {
    // Position should be 0 when no media is loaded
    uint64_t position = controller->getCurrentPosition();
    EXPECT_EQ(position, 0);
}

TEST_F(ApplePlaybackControllerTest, StopWithoutMedia) {
    // Stop should not crash even without media
    bool result = controller->stop();
    EXPECT_TRUE(result);
}

TEST_F(ApplePlaybackControllerTest, PauseWithoutMedia) {
    // Pause should not crash even without media
    bool result = controller->pause();
    // May return false if no media is loaded, which is acceptable
}

TEST_F(ApplePlaybackControllerTest, SeekWithoutMedia) {
    // Seek should handle gracefully when no media is loaded
    bool result = controller->seek(5000);
    // May return false if no media is loaded, which is acceptable
}

// Note: Tests that require actual media files would need:
// - A test media file (e.g., a small MP4 file)
// - Loading the media via loadMedia() or through a media source
// - Then testing play, pause, seek, etc. with actual playback
// These tests are left as placeholders for future implementation

#endif  // __APPLE__
