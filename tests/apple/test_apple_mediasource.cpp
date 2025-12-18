#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <thread>

#include "AudioGenerator.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/IPlaybackEventListener.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/IPlaybackBackend.hpp"
#include "playback/backends/apple/ApplePlaybackController.hpp"

#ifdef __APPLE__

using namespace playback;

// Forward declaration
namespace playback {
std::unique_ptr<IPlaybackBackend> createAppleBackendFactory();
}

// Forward declaration for Apple-specific run loop processing
extern "C" void processRunLoop(double seconds);

// Helper to sleep while processing the run loop on Apple
void appleTestSleep(int milliseconds) {
    if (milliseconds <= 0)
        return;
    processRunLoop(static_cast<double>(milliseconds) / 1000.0);
}

// Test event listener for media source tests
class MediaSourceTestEventListener : public IPlaybackEventListener {
   public:
    std::atomic<int> eventCount{0};
    std::atomic<int> stateChangedCount{0};
    std::atomic<bool> playbackStarted{false};
    std::atomic<bool> playbackPaused{false};
    std::atomic<bool> playbackStopped{false};
    PlaybackState lastState = PlaybackState::IDLE;

    void onPlaybackEvent(const PlaybackEvent& event) override {
        eventCount++;
        if (event.type == PlaybackEvent::Type::STATE_CHANGED) {
            stateChangedCount++;
            lastState = event.state;
            if (event.state == PlaybackState::PLAYING) {
                playbackStarted = true;
            } else if (event.state == PlaybackState::PAUSED) {
                playbackPaused = true;
            } else if (event.state == PlaybackState::STOPPED) {
                playbackStopped = true;
            }
        }
    }

    void onVideoFrame(const void* /*data*/, size_t /*size*/,
                      const VideoFormat& /*format*/) override {
        // Not used for audio-only tests
    }

    void onAudioData(const void* /*data*/, size_t /*size*/,
                     const AudioFormat& /*format*/) override {
        // Could process audio data here if needed
    }
};

class AppleMediaSourceTest : public ::testing::Test {
   protected:
    void SetUp() override {
        backend = playback::createAppleBackendFactory();
        ASSERT_NE(backend, nullptr);
        ASSERT_TRUE(backend->initialize());

        // Create a temporary test audio file
        testAudioFile = std::filesystem::temp_directory_path() / "test_audio.wav";

        // Generate a 2-second sine wave audio file
        auto samples = AudioGenerator::generateSineWave(440.0f, 2.0f, 44100);
        bool saved = AudioGenerator::saveAsWAV(samples, testAudioFile.string(), 44100, 1);
        ASSERT_TRUE(saved) << "Failed to create test audio file";
        ASSERT_TRUE(std::filesystem::exists(testAudioFile)) << "Test audio file was not created";

        // Create controller
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

        // Clean up test file
        if (std::filesystem::exists(testAudioFile)) {
            std::filesystem::remove(testAudioFile);
        }
    }

    // Helper function to wait for media to be ready
    bool waitForMediaReady(int maxWaitMs = 5000) {
        int waited = 0;
        const int pollInterval = 100;  // Check every 100ms

        while (waited < maxWaitMs) {
            auto state = controller->getState();
            uint64_t duration = controller->getDuration();

            // Media is ready when we have a valid duration or state is not IDLE/BUFFERING
            if (duration > 0 ||
                (state != PlaybackState::IDLE && state != PlaybackState::BUFFERING)) {
                return true;
            }

            appleTestSleep(pollInterval);
            waited += pollInterval;
        }

        return false;
    }

    std::unique_ptr<IPlaybackBackend> backend;
    std::unique_ptr<IPlaybackController> controller;
    std::filesystem::path testAudioFile;
};

TEST_F(AppleMediaSourceTest, LoadMediaFromFile) {
    // Load media using loadMedia() method
    auto* appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
    ASSERT_NE(appleController, nullptr);

    bool result = appleController->loadMedia(testAudioFile.string());
    EXPECT_TRUE(result) << "Failed to load media from file: " << testAudioFile;

    // Wait for media to be ready (duration becomes available)
    bool ready = waitForMediaReady();
    ASSERT_TRUE(ready) << "Media did not become ready within timeout";

    // Check that duration is available
    uint64_t duration = controller->getDuration();
    EXPECT_GT(duration, 0) << "Duration should be greater than 0 after loading media";
}

TEST_F(AppleMediaSourceTest, PlayLoadedMedia) {
    auto* appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
    ASSERT_NE(appleController, nullptr);

    // Load media
    bool loaded = appleController->loadMedia(testAudioFile.string());
    ASSERT_TRUE(loaded) << "Failed to load media";

    // Wait for media to be ready
    bool ready = waitForMediaReady();
    ASSERT_TRUE(ready) << "Media did not become ready within timeout";

    // Add event listener
    auto listener = std::make_shared<MediaSourceTestEventListener>();
    controller->addEventListener(listener);

    // Play
    bool result = controller->play();
    EXPECT_TRUE(result) << "Failed to play media";

    // Wait for playback to start
    appleTestSleep(500);

    // Check state
    auto state = controller->getState();
    EXPECT_TRUE(state == PlaybackState::PLAYING || state == PlaybackState::BUFFERING)
        << "Expected PLAYING or BUFFERING, got: " << static_cast<int>(state);

    // Check that events were received
    EXPECT_GT(listener->eventCount, 0) << "Should have received playback events";
}

TEST_F(AppleMediaSourceTest, PausePlayback) {
    auto* appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
    ASSERT_NE(appleController, nullptr);

    // Load and play media
    bool loaded = appleController->loadMedia(testAudioFile.string());
    ASSERT_TRUE(loaded);

    // Wait for media to be ready
    bool ready = waitForMediaReady();
    ASSERT_TRUE(ready) << "Media did not become ready within timeout";

    bool played = controller->play();
    ASSERT_TRUE(played);

    appleTestSleep(500);

    // Pause
    bool result = controller->pause();
    EXPECT_TRUE(result) << "Failed to pause playback";

    // Wait a bit
    appleTestSleep(200);

    // Check state
    auto state = controller->getState();
    EXPECT_EQ(state, PlaybackState::PAUSED) << "Expected PAUSED state after pause";
}

TEST_F(AppleMediaSourceTest, StopPlayback) {
    auto* appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
    ASSERT_NE(appleController, nullptr);

    // Load and play media
    bool loaded = appleController->loadMedia(testAudioFile.string());
    ASSERT_TRUE(loaded);

    // Wait for media to be ready
    bool ready = waitForMediaReady();
    ASSERT_TRUE(ready) << "Media did not become ready within timeout";

    bool played = controller->play();
    ASSERT_TRUE(played);

    appleTestSleep(500);

    // Stop
    bool result = controller->stop();
    EXPECT_TRUE(result) << "Failed to stop playback";

    // Wait a bit
    appleTestSleep(200);

    // Check state
    auto state = controller->getState();
    EXPECT_EQ(state, PlaybackState::STOPPED) << "Expected STOPPED state after stop";

    // Position should be at 0 after stop
    uint64_t position = controller->getCurrentPosition();
    EXPECT_EQ(position, 0) << "Position should be 0 after stop";
}

TEST_F(AppleMediaSourceTest, SeekInMedia) {
    auto* appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
    ASSERT_NE(appleController, nullptr);

    // Load media
    bool loaded = appleController->loadMedia(testAudioFile.string());
    ASSERT_TRUE(loaded);

    // Wait for media to be ready
    bool ready = waitForMediaReady();
    ASSERT_TRUE(ready) << "Media did not become ready within timeout";

    uint64_t duration = controller->getDuration();
    ASSERT_GT(duration, 0) << "Duration should be available";

    // Seek to middle of the file
    uint64_t seekPosition = duration / 2;
    bool result = controller->seek(seekPosition);
    EXPECT_TRUE(result) << "Failed to seek to position: " << seekPosition;

    // Wait for seek to complete
    appleTestSleep(300);

    // Check position (allow some tolerance)
    uint64_t currentPosition = controller->getCurrentPosition();
    uint64_t tolerance = 500;  // 500ms tolerance
    EXPECT_GE(currentPosition, seekPosition - tolerance)
        << "Position should be close to seek position";
    EXPECT_LE(currentPosition, seekPosition + tolerance)
        << "Position should be close to seek position";
}

TEST_F(AppleMediaSourceTest, VolumeControlWithMedia) {
    auto* appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
    ASSERT_NE(appleController, nullptr);

    // Load media
    bool loaded = appleController->loadMedia(testAudioFile.string());
    ASSERT_TRUE(loaded);

    // Wait for media to be ready
    bool ready = waitForMediaReady();
    ASSERT_TRUE(ready) << "Media did not become ready within timeout";

    // Now volume control should work
    bool result = controller->setVolume(0.75);
    EXPECT_TRUE(result) << "setVolume should succeed when media is loaded";

    double volume = controller->getVolume();
    EXPECT_GE(volume, 0.0);
    EXPECT_LE(volume, 1.0);
    EXPECT_NEAR(volume, 0.75, 0.01) << "Volume should be set to 0.75";
}

TEST_F(AppleMediaSourceTest, PlaybackRateControlWithMedia) {
    auto* appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
    ASSERT_NE(appleController, nullptr);

    // Load media
    bool loaded = appleController->loadMedia(testAudioFile.string());
    ASSERT_TRUE(loaded);

    // Wait for media to be ready
    bool ready = waitForMediaReady();
    ASSERT_TRUE(ready) << "Media did not become ready within timeout";

    // Set playback rate
    bool result = controller->setPlaybackRate(1.5);
    EXPECT_TRUE(result) << "setPlaybackRate should succeed when media is loaded";

    double rate = controller->getPlaybackRate();
    EXPECT_GT(rate, 0.0);
    EXPECT_NEAR(rate, 1.5, 0.1) << "Playback rate should be set to 1.5";
}

TEST_F(AppleMediaSourceTest, PositionUpdatesDuringPlayback) {
    auto* appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
    ASSERT_NE(appleController, nullptr);

    // Load media
    bool loaded = appleController->loadMedia(testAudioFile.string());
    ASSERT_TRUE(loaded);

    // Wait for media to be ready
    bool ready = waitForMediaReady();
    ASSERT_TRUE(ready) << "Media did not become ready within timeout";

    // Add event listener to track position changes
    auto listener = std::make_shared<MediaSourceTestEventListener>();
    controller->addEventListener(listener);

    // Play
    bool played = controller->play();
    ASSERT_TRUE(played);

    // Wait for playback to progress
    appleTestSleep(1000);

    // Check that position has advanced
    uint64_t position = controller->getCurrentPosition();
    EXPECT_GT(position, 0) << "Position should advance during playback";

    // Check that events were received
    EXPECT_GT(listener->eventCount, 0) << "Should have received position update events";
}

TEST_F(AppleMediaSourceTest, DurationIsCorrect) {
    auto* appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
    ASSERT_NE(appleController, nullptr);

    // Load media (2-second audio file)
    bool loaded = appleController->loadMedia(testAudioFile.string());
    ASSERT_TRUE(loaded);

    // Wait for media to be ready
    bool ready = waitForMediaReady();
    ASSERT_TRUE(ready) << "Media did not become ready within timeout";

    // Check duration (should be approximately 2000ms for 2-second file)
    uint64_t duration = controller->getDuration();
    EXPECT_GT(duration, 0) << "Duration should be available";

    // Allow some tolerance (1900-2100ms for a 2-second file)
    EXPECT_GE(duration, 1900) << "Duration should be at least 1900ms";
    EXPECT_LE(duration, 2100) << "Duration should be at most 2100ms";
}

#endif  // __APPLE__
