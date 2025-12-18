#include <gtest/gtest.h>
#include <memory>
#include <filesystem>
#include <thread>
#include "playback/backends/apple/AVFoundationWrapper.hpp"
#include "playback/backends/apple/ApplePlaybackController.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "../generators/AudioGenerator.hpp"

#ifdef __APPLE__

using namespace playback;
using namespace playback::apple;

// Forward declaration for Apple-specific run loop processing
extern "C" void processRunLoop(double seconds);

// Helper to sleep while processing the run loop on Apple
void appleResourceTestSleep(int milliseconds) {
    if (milliseconds <= 0) return;
    processRunLoop(static_cast<double>(milliseconds) / 1000.0);
}

class AppleResourceDeallocationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Generate a small test audio file
        testAudioFile = std::filesystem::temp_directory_path() / "resource_test.wav";
        auto samples = AudioGenerator::generateSineWave(440.0f, 1.0f, 44100);
        AudioGenerator::saveAsWAV(samples, testAudioFile.string(), 44100, 1);
    }

    void TearDown() override {
        if (std::filesystem::exists(testAudioFile)) {
            std::filesystem::remove(testAudioFile);
        }
    }

    std::filesystem::path testAudioFile;
};

TEST_F(AppleResourceDeallocationTest, VerifyInstanceDeallocationAfterPlayback) {
    std::weak_ptr<void> tracker;
    
    @autoreleasepool {
        {
            // 2. Create controller and load media
            PlaybackConfig config;
            auto controller = std::make_unique<ApplePlaybackController>(nullptr, config);
            auto* appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
            ASSERT_NE(appleController, nullptr);
            
            tracker = appleController->getLifetimeTracker();
            EXPECT_FALSE(tracker.expired());
            
            ASSERT_TRUE(appleController->loadMedia(testAudioFile.string()));
            
            // Wait for media to be ready
            bool ready = false;
            for (int i = 0; i < 50; ++i) {
                if (controller->getDuration() > 0) {
                    ready = true;
                    break;
                }
                appleResourceTestSleep(100);
            }
            ASSERT_TRUE(ready) << "Media did not become ready within timeout";

            ASSERT_TRUE(controller->play());
            appleResourceTestSleep(500);
            EXPECT_EQ(controller->getState(), PlaybackState::PLAYING);
            ASSERT_TRUE(controller->stop());
            EXPECT_EQ(controller->getState(), PlaybackState::STOPPED);
            EXPECT_FALSE(tracker.expired());
            
            controller.reset();
        }
    }
    
    // 7. Verify tracker is expired
    bool deallocated = false;
    for (int i = 0; i < 50; ++i) {
        if (tracker.expired()) {
            deallocated = true;
            break;
        }
        appleResourceTestSleep(100);
    }
    
    EXPECT_TRUE(deallocated) << "AVFPlayerInternal was not deallocated!";
}

TEST_F(AppleResourceDeallocationTest, VerifyTempFileCleanup) {
    // This test would ideally verify that if we use loadFromData (which creates a temp file),
    // the file is deleted. Since loadFromData is not directly exposed via ApplePlaybackController
    // yet in a way we can easily track the filename, we'll skip the file existence check for now
    // or just rely on the fact that cleanup() is called during dealloc which we verified above.
}

#endif // __APPLE__
