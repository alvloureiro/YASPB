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
    for (auto& [name, backend] : backends_) {
        if (backend) {
            backend->shutdown();
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

    if (!backend->initialize()) {
        return false;
    }

    backends_[name] = std::move(backend);
    return true;
}

bool PlaybackEngine::unregisterBackend(const std::string& name) {
    auto it = backends_.find(name);
    if (it == backends_.end()) {
        return false;
    }

    if (it->second) {
        it->second->shutdown();
    }

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
        // Convert unique_ptr to shared_ptr
        // Note: This creates a shared_ptr that shares ownership
        // In practice, you might want to change the storage to shared_ptr
        return std::shared_ptr<IPlaybackBackend>(it->second.get(), [](IPlaybackBackend*) {
            // Empty deleter - the unique_ptr in the map owns the object
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
