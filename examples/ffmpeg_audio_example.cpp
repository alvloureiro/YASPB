#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "FileMediaSource.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/IPlaybackEventListener.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/core/PlaybackEngine.hpp"
#include "playback/core/PlaybackFactory.hpp"

#ifdef ENABLE_FFMPEG_BACKEND

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
                    const double progress =
                        static_cast<double>(event.positionMs) / event.durationMs * 100.0;
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
    static std::string formatTime(uint64_t ms) {
        uint64_t seconds = ms / 1000;
        const uint64_t minutes = seconds / 60;
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
    std::cout << "\nSupported formats: MP3, M4A, AAC, MP4, MKV, WebM (audio/video)" << std::endl;
}

void printPlaybackInfo(std::unique_ptr<IPlaybackController>& controller) {
    std::cout << "\n=== Playback Information ===" << std::endl;
    std::cout << "Session ID: " << controller->getSessionId() << std::endl;
    std::cout << "Duration: " << controller->getDuration() << " ms" << std::endl;
    std::cout << "Volume: " << controller->getVolume() << std::endl;
    std::cout << "Playback Rate: " << controller->getPlaybackRate() << "x" << std::endl;

    if (const auto formats = controller->getAvailableQualities(); !formats.empty()) {
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string audioFilePath = argv[1];

    std::cout << "FFmpeg Backend Audio Playback Example" << std::endl;
    std::cout << "=====================================" << std::endl;
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

        // Get FFmpeg backend
        auto ffmpegBackend = engine->getBackend("FFmpeg");
        if (!ffmpegBackend) {
            std::cerr << "FFmpeg backend not available. Make sure ENABLE_FFMPEG_BACKEND is enabled "
                         "and FFmpeg is installed."
                      << std::endl;
            return 1;
        }

        std::cout << "Using backend: " << ffmpegBackend->getName() << " v"
                  << ffmpegBackend->getVersion() << std::endl;

        // Create media source using FFmpeg backend
        auto mediaSource = ffmpegBackend->createMediaSource();
        if (!mediaSource) {
            std::cerr << "Failed to create media source" << std::endl;
            return 1;
        }

        if (!mediaSource->load(audioFilePath)) {
            std::cerr << "Failed to load media source: " << audioFilePath << std::endl;
            return 1;
        }

        std::cout << "Media source loaded successfully" << std::endl;
        std::cout << "Title: " << mediaSource->getTitle() << std::endl;
        std::cout << "Duration: " << mediaSource->getDurationMs() << " ms" << std::endl;

        // Create playback configuration
        PlaybackConfig config;
        config.bufferSizeMs = 5000;      // 5 seconds buffer
        config.maxBufferSizeMs = 10000;  // 10 seconds max buffer
        config.hardwareAcceleration = ffmpegBackend->isHardwareAccelerationSupported();

        // Create controller using FFmpeg backend
        auto controller = ffmpegBackend->createController(mediaSource, config);
        if (!controller) {
            std::cerr << "Failed to create playback controller" << std::endl;
            return 1;
        }

        // Add event listener
        auto eventListener = std::make_shared<ExampleEventListener>();
        controller->addEventListener(eventListener);

        // Wait for the media to be ready to play
        std::cout << "Waiting for media to be ready..." << std::endl;
        int maxWaitAttempts = 100;  // 10 seconds max
        int attempts = 0;
        bool isReady = false;

        while (attempts < maxWaitAttempts) {
            auto state = controller->getState();

            if (state == PlaybackState::PAUSED || state == PlaybackState::PLAYING) {
                std::cout << "Media is ready!" << std::endl;
                isReady = true;
                break;
            }
            if (state == PlaybackState::ERROR) {
                std::cerr << "Error loading media! State: ERROR" << std::endl;
                return 1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            attempts++;
        }

        if (!isReady) {
            std::cerr << "Timeout waiting for media to be ready after " << (attempts * 100) << "ms"
                      << std::endl;
            return 1;
        }

        // Set volume to 1.0 (full volume)
        controller->setVolume(1.0);
        std::cout << "Volume set to: " << controller->getVolume() << std::endl;

        // Print playback information
        printPlaybackInfo(controller);

        std::cout << "Media is ready. Use keyboard controls to play, pause, stop, or seek."
                  << std::endl;

        // Keyboard input handling
        std::atomic running{true};
        std::atomic command{'\0'};
        std::mutex inputMutex;

        // Print controls
        std::cout << "\n=== Keyboard Controls ===" << std::endl;
        std::cout << "  p - Play" << std::endl;
        std::cout << "  s - Stop" << std::endl;
        std::cout << "  d - Pause" << std::endl;
        std::cout << "  w - Seek forward 10 seconds" << std::endl;
        std::cout << "  q - Quit" << std::endl;
        std::cout << "========================\n" << std::endl;

        // Keyboard input thread
        std::thread inputThread([&running, &command, &inputMutex]() {
            while (running.load()) {
                char c;
                if (std::cin >> c) {
                    if (c == 'q') {
                        running = false;
                        break;
                    }
                    std::lock_guard<std::mutex> lock(inputMutex);
                    command.store(c);
                } else {
                    running = false;
                    break;
                }
            }
        });

        // Main playback monitoring loop
        while (running && controller->getState() != PlaybackState::ENDED &&
               controller->getState() != PlaybackState::ERROR) {
            // Process keyboard commands
            char cmd = '\0';
            {
                std::lock_guard<std::mutex> lock(inputMutex);
                cmd = command.exchange('\0');
            }

            switch (cmd) {
                case 'p':  // Play
                    if (controller->play()) {
                        std::cout << "\n[Command] Playing..." << std::endl;
                    } else {
                        std::cout << "\n[Command] Failed to play" << std::endl;
                    }
                    break;

                case 's':  // Stop
                    if (controller->stop()) {
                        std::cout << "\n[Command] Stopped" << std::endl;
                    } else {
                        std::cout << "\n[Command] Failed to stop" << std::endl;
                    }
                    break;

                case 'd':  // Pause
                    if (controller->pause()) {
                        std::cout << "\n[Command] Paused" << std::endl;
                    } else {
                        std::cout << "\n[Command] Failed to pause" << std::endl;
                    }
                    break;

                case 'w':  // Seek forward 10 seconds
                {
                    uint64_t currentPos = controller->getCurrentPosition();
                    uint64_t duration = controller->getDuration();
                    uint64_t seekPos = currentPos + 10000;  // +10 seconds
                    if (seekPos > duration) {
                        seekPos = duration;
                    }
                    if (controller->seek(seekPos)) {
                        std::cout << "\n[Command] Seeking to " << (seekPos / 1000) << "s"
                                  << std::endl;
                    } else {
                        std::cout << "\n[Command] Failed to seek" << std::endl;
                    }
                } break;

                default:
                    break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Print stats periodically
            static auto lastStatsTime = std::chrono::steady_clock::now();
            if (auto now = std::chrono::steady_clock::now();
                std::chrono::duration_cast<std::chrono::seconds>(now - lastStatsTime).count() >=
                15) {
                auto stats = controller->getStats();
                std::cout << "\n[Stats] Buffer Health: " << std::fixed << std::setprecision(2)
                          << stats.bufferHealth * 100.0 << "%, "
                          << "FPS: " << stats.framesPerSecond << std::endl;
                lastStatsTime = now;
            }
        }

        // Stop input thread
        running = false;
        if (inputThread.joinable()) {
            inputThread.join();
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
    std::cerr << "This example requires FFmpeg backend to be enabled (ENABLE_FFMPEG_BACKEND=ON)"
              << std::endl;
    return 1;
}
#endif
