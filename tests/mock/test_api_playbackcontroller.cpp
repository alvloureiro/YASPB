#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "playback/api/IPlaybackController.hpp"
#include "playback/api/IPlaybackEventListener.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/IPlaybackBackend.hpp"

using namespace playback;

// Forward declaration
namespace playback {
std::unique_ptr<IPlaybackBackend> createMockBackendFactory();
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
        // Not used in mock backend
    }

    void onAudioData(const void* /*data*/, size_t /*size*/,
                     const AudioFormat& /*format*/) override {
        // Not used in mock backend
    }
};

class PlaybackControllerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        backend = playback::createMockBackendFactory();
        ASSERT_NE(backend, nullptr);
        ASSERT_TRUE(backend->initialize());

        source = backend->createMediaSource();
        ASSERT_NE(source, nullptr);
        source->load("test.mp4");

        PlaybackConfig config;
        controller = backend->createController(source, config);
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
    std::shared_ptr<IMediaSource> source;
    std::unique_ptr<IPlaybackController> controller;
};

TEST_F(PlaybackControllerTest, InitialState) {
    EXPECT_EQ(controller->getState(), PlaybackState::IDLE);
}

TEST_F(PlaybackControllerTest, Play) {
    bool result = controller->play();
    EXPECT_TRUE(result);

    // Wait a bit for state to change
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Should be playing or buffering
    auto state = controller->getState();
    EXPECT_TRUE(state == PlaybackState::PLAYING || state == PlaybackState::BUFFERING);
}

TEST_F(PlaybackControllerTest, Pause) {
    controller->play();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool result = controller->pause();
    EXPECT_TRUE(result);
    EXPECT_EQ(controller->getState(), PlaybackState::PAUSED);
}

TEST_F(PlaybackControllerTest, Stop) {
    controller->play();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool result = controller->stop();
    EXPECT_TRUE(result);
    EXPECT_EQ(controller->getState(), PlaybackState::STOPPED);
}

TEST_F(PlaybackControllerTest, Seek) {
    // Seek to 5 seconds
    bool result = controller->seek(5000);
    EXPECT_TRUE(result);

    uint64_t position = controller->getCurrentPosition();
    EXPECT_EQ(position, 5000);
}

TEST_F(PlaybackControllerTest, Volume) {
    // Set volume
    bool result = controller->setVolume(0.75);
    EXPECT_TRUE(result);

    double volume = controller->getVolume();
    EXPECT_DOUBLE_EQ(volume, 0.75);

    // Invalid volume
    result = controller->setVolume(1.5);
    EXPECT_FALSE(result);

    result = controller->setVolume(-0.1);
    EXPECT_FALSE(result);
}

TEST_F(PlaybackControllerTest, PlaybackRate) {
    // Set playback rate
    bool result = controller->setPlaybackRate(1.5);
    EXPECT_TRUE(result);

    double rate = controller->getPlaybackRate();
    EXPECT_DOUBLE_EQ(rate, 1.5);

    // Invalid rate
    result = controller->setPlaybackRate(5.0);
    EXPECT_FALSE(result);
}

TEST_F(PlaybackControllerTest, Quality) {
    auto qualities = controller->getAvailableQualities();
    EXPECT_FALSE(qualities.empty());

    // Set quality
    bool result = controller->setQuality(qualities[0]);
    EXPECT_TRUE(result);

    auto currentQuality = controller->getCurrentQuality();
    EXPECT_EQ(currentQuality.type, qualities[0].type);
}

TEST_F(PlaybackControllerTest, Stats) {
    auto stats = controller->getStats();
    EXPECT_GT(stats.networkBandwidth, 0);
    EXPECT_GT(stats.framesPerSecond, 0);
    EXPECT_GE(stats.bufferHealth, 0.0);
    EXPECT_LE(stats.bufferHealth, 1.0);
}

TEST_F(PlaybackControllerTest, EventListener) {
    auto listener = std::make_shared<TestEventListener>();
    controller->addEventListener(listener);

    // Play to trigger events
    controller->play();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Should have received events
    EXPECT_GT(listener->eventCount, 0);
    EXPECT_GT(listener->stateChangedCount, 0);

    // Remove listener
    controller->removeEventListener(listener);
}

TEST_F(PlaybackControllerTest, Config) {
    PlaybackConfig config;
    config.bufferSizeMs = 20000;

    // Create new controller with config
    auto newController = backend->createController(source, config);
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

TEST_F(PlaybackControllerTest, SessionId) {
    std::string sessionId = controller->getSessionId();
    EXPECT_FALSE(sessionId.empty());
    EXPECT_EQ(sessionId.substr(0, 5), "mock-");
}

TEST_F(PlaybackControllerTest, Duration) {
    uint64_t duration = controller->getDuration();
    EXPECT_GT(duration, 0);
}
