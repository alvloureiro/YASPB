#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "FileMediaSource.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/IPlaybackEventListener.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/apple/ApplePlaybackController.hpp"
#include "playback/core/PlaybackEngine.hpp"
#include "playback/core/PlaybackFactory.hpp"

#ifdef __APPLE__
// Forward declaration - we'll use a helper function instead of importing Foundation
extern "C" void processRunLoop(double seconds);
#endif

using namespace playback;

// Simple event listener to demonstrate event handling
class ExampleEventListener : public IPlaybackEventListener {
   public:
    void onPlaybackEvent(const PlaybackEvent& event) override {
        switch (event.type) {
            case PlaybackEvent::Type::STATE_CHANGED:
                std::cout << "[Event] State changed: ";
                switch (event.state) {
                    case PlaybackState::IDLE:
                        std::cout << "IDLE";
                        break;
                    case PlaybackState::BUFFERING:
                        std::cout << "BUFFERING";
                        break;
                    case PlaybackState::PLAYING:
                        std::cout << "PLAYING";
                        break;
                    case PlaybackState::PAUSED:
                        std::cout << "PAUSED";
                        break;
                    case PlaybackState::STOPPED:
                        std::cout << "STOPPED";
                        break;
                    case PlaybackState::ENDED:
                        std::cout << "ENDED";
                        break;
                    case PlaybackState::ERROR:
                        std::cout << "ERROR";
                        break;
                }
                std::cout << std::endl;
                break;

            case PlaybackEvent::Type::POSITION_CHANGED:
                if (event.durationMs > 0) {
                    double progress = (double)event.positionMs / event.durationMs * 100.0;
                    std::cout << "\r[Progress] " << std::fixed << std::setprecision(1) << progress
                              << "% (" << formatTime(event.positionMs) << " / "
                              << formatTime(event.durationMs) << ")";
                    std::cout.flush();
                }
                break;

            case PlaybackEvent::Type::BUFFERING_PROGRESS:
                std::cout << "\n[Buffer] " << std::fixed << std::setprecision(1)
                          << event.bufferLevel * 100.0 << "%" << std::endl;
                break;

            case PlaybackEvent::Type::ERROR_OCCURRED:
                std::cout << "\n[Error] " << event.message << std::endl;
                break;

            default:
                break;
        }
    }

    void onVideoFrame(const void* data, size_t size, const VideoFormat& format) override {
        (void)data;
        (void)size;
        (void)format;
        // Not used for audio-only playback
    }

    void onAudioData(const void* data, size_t size, const AudioFormat& format) override {
        (void)data;
        (void)size;
        (void)format;
        // Could process audio data here if needed
    }

   private:
    std::string formatTime(uint64_t ms) {
        uint64_t seconds = ms / 1000;
        uint64_t minutes = seconds / 60;
        seconds %= 60;
        return std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
    }
};

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <audio_file_path>" << std::endl;
    std::cout << "\nExample:" << std::endl;
    std::cout << "  " << programName << " /path/to/audio.mp3" << std::endl;
    std::cout << "  " << programName << " /path/to/audio.m4a" << std::endl;
    std::cout << "  " << programName << " /path/to/audio.aac" << std::endl;
    std::cout << "\nSupported formats: MP3, M4A, AAC, MP4 (audio/video)" << std::endl;
}

void printPlaybackInfo(std::unique_ptr<IPlaybackController>& controller) {
    std::cout << "\n=== Playback Information ===" << std::endl;
    std::cout << "Session ID: " << controller->getSessionId() << std::endl;
    std::cout << "Duration: " << controller->getDuration() << " ms" << std::endl;
    std::cout << "Volume: " << controller->getVolume() << std::endl;
    std::cout << "Playback Rate: " << controller->getPlaybackRate() << "x" << std::endl;

    auto formats = controller->getAvailableQualities();
    if (!formats.empty()) {
        std::cout << "Available Formats: " << formats.size() << std::endl;
        for (size_t i = 0; i < formats.size(); ++i) {
            const auto& format = formats[i];
            std::cout << "  Format " << (i + 1) << ": " << format.mimeType;
            if (format.audio.has_value()) {
                std::cout << " - " << format.audio->sampleRate << " Hz, " << format.audio->channels
                          << " channels";
            }
            std::cout << std::endl;
        }
    }
    std::cout << "=============================\n" << std::endl;
}

#ifdef __APPLE__
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string audioFilePath = argv[1];

    std::cout << "Apple Backend Audio Playback Example" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "File: " << audioFilePath << std::endl;
    std::cout << std::endl;

    try {
        // Create playback engine using factory
        auto engine = PlaybackFactory::createEngine();
        if (!engine) {
            std::cerr << "Failed to create playback engine" << std::endl;
            return 1;
        }

        // Get available backends
        auto backends = engine->getAvailableBackends();
        std::cout << "Available backends: ";
        for (const auto& backend : backends) {
            std::cout << backend << " ";
        }
        std::cout << std::endl;

        // Get Apple backend
        auto appleBackend = engine->getBackend("Apple AVFoundation");
        if (!appleBackend) {
            std::cerr
                << "Apple backend not available. Make sure ENABLE_AVFOUNDATION_BACKEND is enabled."
                << std::endl;
            return 1;
        }

        std::cout << "Using backend: " << appleBackend->getName() << " v"
                  << appleBackend->getVersion() << std::endl;

        // Create media source
        auto mediaSource = std::make_shared<FileMediaSource>();
        if (!mediaSource->load(audioFilePath)) {
            std::cerr << "Failed to load media source: " << audioFilePath << std::endl;
            return 1;
        }

        std::cout << "Media source loaded successfully" << std::endl;
        std::cout << "Title: " << mediaSource->getTitle() << std::endl;

        // Create playback configuration
        PlaybackConfig config;
        config.bufferSizeMs = 5000;      // 5 seconds buffer
        config.maxBufferSizeMs = 10000;  // 10 seconds max buffer
        config.hardwareAcceleration = true;

        // Create controller using Apple backend
        auto controller = appleBackend->createController(mediaSource, config);
        if (!controller) {
            std::cerr << "Failed to create playback controller" << std::endl;
            return 1;
        }

        // Load the media file
        // Note: We need to use loadMedia() because the controller doesn't automatically
        // load from the source. The FileMediaSource stores the path, but we need to
        // pass it to the underlying player.
        // Cast to ApplePlaybackController to access loadMedia() method
        auto appleController = dynamic_cast<ApplePlaybackController*>(controller.get());
        if (!appleController) {
            std::cerr << "Controller is not an ApplePlaybackController" << std::endl;
            return 1;
        }

        std::cout << "Loading media file: " << audioFilePath << std::endl;
        if (!appleController->loadMedia(audioFilePath)) {
            std::cerr << "Failed to load media file: " << audioFilePath << std::endl;
            return 1;
        }

        // Add event listener
        auto eventListener = std::make_shared<ExampleEventListener>();
        controller->addEventListener(eventListener);

        // Wait for the media to be ready to play
        std::cout << "Waiting for media to be ready..." << std::endl;
        int maxWaitAttempts = 100;  // 10 seconds max (increased for slower loading)
        int attempts = 0;
        bool isReady = false;

        while (attempts < maxWaitAttempts) {
            auto state = controller->getState();

            // PAUSED state means the player item is ready (status is ReadyToPlay) but not playing
            // yet This is what we want - ready to play
            if (state == PlaybackState::PAUSED) {
                std::cout << "Media is ready! (PAUSED state means ReadyToPlay)" << std::endl;
                isReady = true;
                break;
            }
            if (state == PlaybackState::PLAYING) {
                std::cout << "Media is already playing!" << std::endl;
                isReady = true;
                break;
            }
            if (state == PlaybackState::ERROR) {
                std::cerr << "Error loading media! State: ERROR" << std::endl;
                return 1;
            }
            if (state == PlaybackState::BUFFERING) {
                // Still loading, continue waiting
                if (attempts % 10 == 0) {  // Log every second
                    std::cout << "Still buffering... (attempt " << attempts << "/"
                              << maxWaitAttempts << ")" << std::endl;
                }
            }

    // On macOS, process the run loop to allow AVFoundation to work
    // This is critical for command-line apps
    #ifdef __APPLE__
            processRunLoop(0.1);  // Process run loop for 0.1 seconds
    #else
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    #endif
            attempts++;
        }

        if (!isReady) {
            auto finalState = controller->getState();
            std::cerr << "Timeout waiting for media to be ready after " << (attempts * 100) << "ms"
                      << std::endl;
            std::cerr << "Final state: " << static_cast<int>(finalState) << std::endl;
            std::cerr << "Duration: " << controller->getDuration() << " ms" << std::endl;
            return 1;
        }

        // Set volume to 1.0 (full volume) - volume range is 0.0 to 1.0
        controller->setVolume(1.0);
        std::cout << "Volume set to: " << controller->getVolume() << std::endl;

        // Print playback information
        printPlaybackInfo(controller);

        // Start playback
        std::cout << "Starting playback..." << std::endl;
        if (!controller->play()) {
            std::cerr << "Failed to start playback" << std::endl;
            std::cerr << "Current state: " << static_cast<int>(controller->getState()) << std::endl;
            return 1;
        }

        // Wait for playback to actually start
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Verify playback started
        auto playState = controller->getState();
        std::cout << "Playback state after play(): " << static_cast<int>(playState) << std::endl;
        if (playState != PlaybackState::PLAYING) {
            std::cerr << "WARNING: Playback did not start. State is: "
                      << static_cast<int>(playState) << std::endl;
            std::cerr << "Duration: " << controller->getDuration() << " ms" << std::endl;
            std::cerr << "Position: " << controller->getCurrentPosition() << " ms" << std::endl;
        } else {
            std::cout << "Playback started successfully!" << std::endl;
        }

        // Monitor playback
        std::cout << "\nPlayback started. Press Enter to pause, then Enter again to resume..."
                  << std::endl;
        std::cout << "Or wait for playback to finish.\n" << std::endl;

        // Simple interactive loop
        bool paused = false;
        while (controller->getState() != PlaybackState::ENDED &&
               controller->getState() != PlaybackState::ERROR) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Check if user wants to pause/resume (non-blocking check)
            // In a real application, you'd use proper input handling
            auto state = controller->getState();
            if (state == PlaybackState::PLAYING && !paused) {
                // Continue playing
            } else if (state == PlaybackState::PAUSED && paused) {
                // Continue paused
            }

            // Print stats periodically
            static auto lastStatsTime = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastStatsTime).count() >=
                5) {
                auto stats = controller->getStats();
                std::cout << "\n[Stats] Buffer Health: " << std::fixed << std::setprecision(2)
                          << stats.bufferHealth * 100.0 << "%, "
                          << "FPS: " << stats.framesPerSecond << std::endl;
                lastStatsTime = now;
            }
        }

        std::cout << "\n\nPlayback finished." << std::endl;

        // Cleanup
        controller->stop();
        controller.reset();
        mediaSource.reset();
        engine.reset();

        std::cout << "Example completed successfully." << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}

#else
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    std::cerr << "This example requires Apple platform (macOS or iOS)" << std::endl;
    return 1;
}
#endif
