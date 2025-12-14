#include <gtest/gtest.h>

#include <memory>

#include "playback/api/PlaybackConfig.hpp"
#include "playback/core/PlaybackEngine.hpp"
#include "playback/core/PlaybackFactory.hpp"

using namespace playback;

class PlaybackFactoryTest : public ::testing::Test {
   protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(PlaybackFactoryTest, CreateEngine) {
    auto engine = PlaybackFactory::createEngine();
    EXPECT_NE(engine, nullptr);

    auto backends = engine->getAvailableBackends();
    // Mock backend should be registered by default
    EXPECT_FALSE(backends.empty());
}

TEST_F(PlaybackFactoryTest, CreateMockBackend) {
    auto backend = PlaybackFactory::createMockBackend();
    EXPECT_NE(backend, nullptr);
    EXPECT_EQ(backend->getName(), "Mock");

    bool result = backend->initialize();
    EXPECT_TRUE(result);

    backend->shutdown();
}

TEST_F(PlaybackFactoryTest, CreateLowLatencyConfig) {
    auto config = PlaybackFactory::createLowLatencyConfig();

    EXPECT_EQ(config.bufferSizeMs, 2000);
    EXPECT_EQ(config.maxBufferSizeMs, 5000);
    EXPECT_EQ(config.networkTimeoutMs, 3000);
    EXPECT_FALSE(config.adaptiveBitrate);
    EXPECT_DOUBLE_EQ(config.qualitySwitchThreshold, 0.5);
}

TEST_F(PlaybackFactoryTest, CreateHighQualityConfig) {
    auto config = PlaybackFactory::createHighQualityConfig();

    EXPECT_EQ(config.bufferSizeMs, 30000);
    EXPECT_EQ(config.maxBufferSizeMs, 60000);
    EXPECT_EQ(config.maxVideoWidth, 3840);
    EXPECT_EQ(config.maxVideoHeight, 2160);
    EXPECT_DOUBLE_EQ(config.maxFrameRate, 60.0);
    EXPECT_EQ(config.maxBitrate, 50000000);
    EXPECT_TRUE(config.adaptiveBitrate);
    EXPECT_DOUBLE_EQ(config.qualitySwitchThreshold, 0.9);
}

TEST_F(PlaybackFactoryTest, CreateMobileConfig) {
    auto config = PlaybackFactory::createMobileConfig();

    EXPECT_EQ(config.bufferSizeMs, 10000);
    EXPECT_EQ(config.maxBufferSizeMs, 20000);
    EXPECT_EQ(config.maxVideoWidth, 1920);
    EXPECT_EQ(config.maxVideoHeight, 1080);
    EXPECT_DOUBLE_EQ(config.maxFrameRate, 30.0);
    EXPECT_EQ(config.maxBitrate, 5000000);
    EXPECT_TRUE(config.adaptiveBitrate);
    EXPECT_TRUE(config.hardwareAcceleration);
}

TEST_F(PlaybackFactoryTest, ValidateConfig) {
    // Valid config
    auto validConfig = PlaybackFactory::createLowLatencyConfig();
    bool result = PlaybackFactory::validateConfig(validConfig);
    EXPECT_TRUE(result);

    // Invalid config - buffer size > max buffer size
    PlaybackConfig invalidConfig;
    invalidConfig.bufferSizeMs = 10000;
    invalidConfig.maxBufferSizeMs = 5000;  // Invalid
    result = PlaybackFactory::validateConfig(invalidConfig);
    EXPECT_FALSE(result);

    // Invalid config - zero dimensions
    PlaybackConfig invalidConfig2;
    invalidConfig2.maxVideoWidth = 0;
    invalidConfig2.maxVideoHeight = 1080;
    result = PlaybackFactory::validateConfig(invalidConfig2);
    EXPECT_FALSE(result);

    // Invalid config - invalid frame rate
    PlaybackConfig invalidConfig3;
    invalidConfig3.maxFrameRate = -1.0;
    result = PlaybackFactory::validateConfig(invalidConfig3);
    EXPECT_FALSE(result);

    // Invalid config - invalid threshold
    PlaybackConfig invalidConfig4;
    invalidConfig4.qualitySwitchThreshold = 1.5;  // > 1.0
    result = PlaybackFactory::validateConfig(invalidConfig4);
    EXPECT_FALSE(result);
}

TEST_F(PlaybackFactoryTest, ConfigPresets) {
    // Test that all preset configs are valid
    auto lowLatency = PlaybackFactory::createLowLatencyConfig();
    EXPECT_TRUE(PlaybackFactory::validateConfig(lowLatency));

    auto highQuality = PlaybackFactory::createHighQualityConfig();
    EXPECT_TRUE(PlaybackFactory::validateConfig(highQuality));

    auto mobile = PlaybackFactory::createMobileConfig();
    EXPECT_TRUE(PlaybackFactory::validateConfig(mobile));
}

TEST_F(PlaybackFactoryTest, ValidateConfigBitrate) {
    // Invalid config - initial bitrate > max bitrate
    PlaybackConfig invalidConfig;
    invalidConfig.initialBitrate = 10000000;
    invalidConfig.maxBitrate = 5000000;
    bool result = PlaybackFactory::validateConfig(invalidConfig);
    EXPECT_FALSE(result);
}
