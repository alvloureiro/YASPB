#pragma once
#include <map>
#include <memory>
#include <vector>

#include "../api/IMediaSource.hpp"
#include "../api/IPlaybackController.hpp"
#include "../api/PlaybackConfig.hpp"
#include "../backends/IPlaybackBackend.hpp"

namespace playback {

class PlaybackEngine {
   public:
    PlaybackEngine();
    ~PlaybackEngine();

    // Backends registers
    bool registerBackend(std::unique_ptr<IPlaybackBackend> backend);
    bool unregisterBackend(const std::string& name);

    // Control creation
    std::unique_ptr<IPlaybackController> createController(
        const std::string& backendName, std::shared_ptr<IMediaSource> source,
        const PlaybackConfig& config = PlaybackConfig());

    // Auto discover the best backend
    std::unique_ptr<IPlaybackController> createAutoController(
        std::shared_ptr<IMediaSource> source, const PlaybackConfig& config = PlaybackConfig());

    // Backend management
    [[nodiscard]] std::vector<std::string> getAvailableBackends() const;
    [[nodiscard]] std::shared_ptr<IPlaybackBackend> getBackend(const std::string& name) const;

    // Global configuration
    void setDefaultConfig(const PlaybackConfig& config);
    [[nodiscard]] PlaybackConfig getDefaultConfig() const;

   private:
    std::map<std::string, std::unique_ptr<IPlaybackBackend>> backends_;
    PlaybackConfig defaultConfig_;
};

}  // namespace playback
