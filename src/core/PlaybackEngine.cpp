#include "playback/core/PlaybackEngine.hpp"

#include <algorithm>

#include "playback/api/IMediaSource.hpp"
#include "playback/api/IPlaybackController.hpp"
#include "playback/api/PlaybackConfig.hpp"
#include "playback/backends/IPlaybackBackend.hpp"

namespace playback {

PlaybackEngine::PlaybackEngine() {
    defaultConfig_ = PlaybackConfig();
}

PlaybackEngine::~PlaybackEngine() {
    // RAII: Ensure all backends are properly shut down
    // Use reverse iteration to avoid issues if shutdown modifies the container
    // (though it shouldn't, this is defensive programming)
    for (auto it = backends_.rbegin(); it != backends_.rend(); ++it) {
        if (it->second) {
            try {
                it->second->shutdown();
            } catch (...) {
                // Log error but continue cleanup
                // Destructors should not throw
            }
        }
    }
    backends_.clear();
}

bool PlaybackEngine::registerBackend(std::unique_ptr<IPlaybackBackend> backend) {
    if (!backend) {
        return false;
    }

    std::string name = backend->getName();
    if (backends_.find(name) != backends_.end()) {
        return false;  // Backend already registered
    }

    // RAII: Initialize before storing to ensure resource acquisition happens atomically
    // If initialization fails, backend is not stored (exception-safe)
    if (!backend->initialize()) {
        return false;
    }

    // Move backend into map (transfer ownership)
    // If move throws, backend is still valid and will be destroyed
    backends_[name] = std::move(backend);
    return true;
}

bool PlaybackEngine::unregisterBackend(const std::string& name) {
    auto it = backends_.find(name);
    if (it == backends_.end()) {
        return false;
    }

    // RAII: Shutdown before erasing to ensure proper resource cleanup
    // If shutdown throws, backend is still in map (defensive)
    if (it->second) {
        try {
            it->second->shutdown();
        } catch (...) {
            // Log error but continue with removal
            // Shutdown should not throw, but handle it gracefully
        }
    }

    // Erase from map (destroys unique_ptr and backend)
    backends_.erase(it);
    return true;
}

std::unique_ptr<IPlaybackController> PlaybackEngine::createController(
    const std::string& backendName, std::shared_ptr<IMediaSource> source,
    const PlaybackConfig& config) {
    auto backend = getBackend(backendName);
    if (!backend) {
        return nullptr;
    }

    PlaybackConfig finalConfig = config;
    // Merge with default config if needed
    if (finalConfig.bufferSizeMs == PlaybackConfig().bufferSizeMs) {
        finalConfig.bufferSizeMs = defaultConfig_.bufferSizeMs;
    }

    return backend->createController(source, finalConfig);
}

std::unique_ptr<IPlaybackController> PlaybackEngine::createAutoController(
    std::shared_ptr<IMediaSource> source, const PlaybackConfig& config) {
    if (backends_.empty()) {
        return nullptr;
    }

    // Try to find the best backend
    // For now, just use the first available backend
    // In the future, this could check format support, capabilities, etc.
    auto it = backends_.begin();
    if (it != backends_.end() && it->second) {
        PlaybackConfig finalConfig = config;
        if (finalConfig.bufferSizeMs == PlaybackConfig().bufferSizeMs) {
            finalConfig.bufferSizeMs = defaultConfig_.bufferSizeMs;
        }
        return it->second->createController(source, finalConfig);
    }

    return nullptr;
}

std::vector<std::string> PlaybackEngine::getAvailableBackends() const {
    std::vector<std::string> names;
    names.reserve(backends_.size());

    for (const auto& [name, backend] : backends_) {
        names.push_back(name);
    }

    return names;
}

std::shared_ptr<IPlaybackBackend> PlaybackEngine::getBackend(const std::string& name) const {
    auto it = backends_.find(name);
    if (it != backends_.end() && it->second) {
        // RAII: Create a shared_ptr that shares ownership without transferring it
        // The unique_ptr in the map retains ownership, but we provide shared access
        // This is safe as long as the backend is not unregistered while shared_ptr exists
        // Note: Consider using weak_ptr or changing storage to shared_ptr for better safety
        return std::shared_ptr<IPlaybackBackend>(it->second.get(), [](IPlaybackBackend*) {
            // Empty deleter - the unique_ptr in the map owns the object
            // This is safe because the map's lifetime exceeds any returned shared_ptr
        });
    }
    return nullptr;
}

void PlaybackEngine::setDefaultConfig(const PlaybackConfig& config) {
    defaultConfig_ = config;
}

PlaybackConfig PlaybackEngine::getDefaultConfig() const {
    return defaultConfig_;
}

}  // namespace playback
