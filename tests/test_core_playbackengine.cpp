#include <gtest/gtest.h>

#include <memory>

#include "playback/backends/IPlaybackBackend.hpp"
#include "playback/core/PlaybackEngine.hpp"

using namespace playback;

// Forward declaration
namespace playback {
std::unique_ptr<IPlaybackBackend> createMockBackendFactory();
}

class PlaybackEngineTest : public ::testing::Test {
   protected:
    void SetUp() override {
        engine = std::make_unique<PlaybackEngine>();
    }

    void TearDown() override {
        engine.reset();
    }

    std::unique_ptr<PlaybackEngine> engine;
};

TEST_F(PlaybackEngineTest, Construction) {
    // Should construct without errors
    EXPECT_NE(engine, nullptr);
}

TEST_F(PlaybackEngineTest, RegisterBackend) {
    auto backend = playback::createMockBackendFactory();
    bool result = engine->registerBackend(std::move(backend));
    EXPECT_TRUE(result);

    auto backends = engine->getAvailableBackends();
    EXPECT_FALSE(backends.empty());
    EXPECT_EQ(backends[0], "Mock");
}

TEST_F(PlaybackEngineTest, RegisterDuplicateBackend) {
    auto backend1 = playback::createMockBackendFactory();
    bool result1 = engine->registerBackend(std::move(backend1));
    EXPECT_TRUE(result1);

    auto backend2 = playback::createMockBackendFactory();
    bool result2 = engine->registerBackend(std::move(backend2));
    EXPECT_FALSE(result2);  // Should fail - duplicate name
}

TEST_F(PlaybackEngineTest, UnregisterBackend) {
    auto backend = playback::createMockBackendFactory();
    engine->registerBackend(std::move(backend));

    bool result = engine->unregisterBackend("Mock");
    EXPECT_TRUE(result);

    auto backends = engine->getAvailableBackends();
    EXPECT_TRUE(backends.empty());
}

TEST_F(PlaybackEngineTest, UnregisterNonExistent) {
    bool result = engine->unregisterBackend("NonExistent");
    EXPECT_FALSE(result);
}

TEST_F(PlaybackEngineTest, GetBackend) {
    auto backend = playback::createMockBackendFactory();
    engine->registerBackend(std::move(backend));

    auto retrieved = engine->getBackend("Mock");
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getName(), "Mock");
}

TEST_F(PlaybackEngineTest, GetBackendNonExistent) {
    auto retrieved = engine->getBackend("NonExistent");
    EXPECT_EQ(retrieved, nullptr);
}

TEST_F(PlaybackEngineTest, CreateController) {
    auto backend = playback::createMockBackendFactory();
    engine->registerBackend(std::move(backend));

    auto sourceBackend = playback::createMockBackendFactory();
    sourceBackend->initialize();
    auto source = sourceBackend->createMediaSource();
    source->load("test.mp4");

    PlaybackConfig config;
    auto controller = engine->createController("Mock", source, config);
    EXPECT_NE(controller, nullptr);

    sourceBackend->shutdown();
}

TEST_F(PlaybackEngineTest, CreateControllerInvalidBackend) {
    auto sourceBackend = playback::createMockBackendFactory();
    sourceBackend->initialize();
    auto source = sourceBackend->createMediaSource();
    source->load("test.mp4");

    PlaybackConfig config;
    auto controller = engine->createController("NonExistent", source, config);
    EXPECT_EQ(controller, nullptr);

    sourceBackend->shutdown();
}

TEST_F(PlaybackEngineTest, CreateAutoController) {
    auto backend = playback::createMockBackendFactory();
    engine->registerBackend(std::move(backend));

    auto sourceBackend = playback::createMockBackendFactory();
    sourceBackend->initialize();
    auto source = sourceBackend->createMediaSource();
    source->load("test.mp4");

    PlaybackConfig config;
    auto controller = engine->createAutoController(source, config);
    EXPECT_NE(controller, nullptr);

    sourceBackend->shutdown();
}

TEST_F(PlaybackEngineTest, CreateAutoControllerNoBackends) {
    auto sourceBackend = playback::createMockBackendFactory();
    sourceBackend->initialize();
    auto source = sourceBackend->createMediaSource();
    source->load("test.mp4");

    PlaybackConfig config;
    auto controller = engine->createAutoController(source, config);
    EXPECT_EQ(controller, nullptr);  // No backends registered

    sourceBackend->shutdown();
}

TEST_F(PlaybackEngineTest, DefaultConfig) {
    PlaybackConfig defaultConfig = engine->getDefaultConfig();
    EXPECT_EQ(defaultConfig.bufferSizeMs, 15000);  // Default value

    PlaybackConfig newConfig;
    newConfig.bufferSizeMs = 20000;
    engine->setDefaultConfig(newConfig);

    PlaybackConfig retrieved = engine->getDefaultConfig();
    EXPECT_EQ(retrieved.bufferSizeMs, 20000);
}

TEST_F(PlaybackEngineTest, MultipleBackends) {
    auto backend1 = playback::createMockBackendFactory();
    engine->registerBackend(std::move(backend1));

    auto backends = engine->getAvailableBackends();
    EXPECT_EQ(backends.size(), 1);
    EXPECT_EQ(backends[0], "Mock");
}
