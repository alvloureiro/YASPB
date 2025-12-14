#include <gtest/gtest.h>

#include <memory>

#include "playback/api/IMediaSource.hpp"
#include "playback/api/MediaFormat.hpp"
#include "playback/backends/IPlaybackBackend.hpp"

using namespace playback;

// Forward declaration - MockBackend factory
namespace playback {
std::unique_ptr<IPlaybackBackend> createMockBackendFactory();
}

class MediaSourceTest : public ::testing::Test {
   protected:
    void SetUp() override {
        backend = playback::createMockBackendFactory();
        ASSERT_NE(backend, nullptr);
        ASSERT_TRUE(backend->initialize());
        source = backend->createMediaSource();
        ASSERT_NE(source, nullptr);
    }

    void TearDown() override {
        if (backend) {
            backend->shutdown();
        }
    }

    std::unique_ptr<IPlaybackBackend> backend;
    std::shared_ptr<IMediaSource> source;
};

TEST_F(MediaSourceTest, LoadFromUri) {
    bool result = source->load("test_video.mp4");
    EXPECT_TRUE(result);

    uint64_t duration = source->getDurationMs();
    EXPECT_EQ(duration, 120000);  // 2 minutes for .mp4
}

TEST_F(MediaSourceTest, LoadFromData) {
    std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03};

    bool result = source->load(data);
    EXPECT_TRUE(result);

    uint64_t duration = source->getDurationMs();
    EXPECT_EQ(duration, 60000);  // Default 1 minute
}

TEST_F(MediaSourceTest, LoadFromUriMkv) {
    bool result = source->load("test.mkv");
    EXPECT_TRUE(result);

    uint64_t duration = source->getDurationMs();
    EXPECT_EQ(duration, 180000);  // 3 minutes for .mkv
}

TEST_F(MediaSourceTest, GetAvailableFormats) {
    source->load("test.mkv");

    auto formats = source->getAvailableFormats();
    EXPECT_FALSE(formats.empty());

    const auto& format = formats[0];
    EXPECT_EQ(format.type, MediaType::VIDEO);
    EXPECT_EQ(format.codec, Codec::H264);
    EXPECT_EQ(format.container, ContainerFormat::MP4);
    EXPECT_TRUE(format.video.has_value());
    EXPECT_TRUE(format.audio.has_value());

    if (format.video.has_value()) {
        EXPECT_EQ(format.video->width, 1920);
        EXPECT_EQ(format.video->height, 1080);
        EXPECT_EQ(format.video->frameRate, 30.0);
    }
}

TEST_F(MediaSourceTest, GetAdaptiveStreams) {
    source->load("test_video.mp4");

    auto streams = source->getAdaptiveStreams();
    EXPECT_FALSE(streams.empty());

    const auto& stream = streams[0];
    EXPECT_EQ(stream.bitrate, 5000000);
    EXPECT_EQ(stream.width, 1920);
    EXPECT_EQ(stream.height, 1080);
    EXPECT_FALSE(stream.url.empty());
}

TEST_F(MediaSourceTest, Properties) {
    source->load("test.mp4");

    EXPECT_FALSE(source->isLive());
    EXPECT_TRUE(source->isSeekable());
    EXPECT_FALSE(source->hasDRM());
}

TEST_F(MediaSourceTest, Metadata) {
    source->load("test.mp4");

    std::string title = source->getTitle();
    std::string artist = source->getArtist();
    std::string album = source->getAlbum();
    auto thumbnail = source->getThumbnail();

    EXPECT_EQ(title, "Mock Media Title");
    EXPECT_EQ(artist, "Mock Artist");
    EXPECT_EQ(album, "Mock Album");
    // Thumbnail can be empty for mock
}

TEST_F(MediaSourceTest, DRM) {
    source->load("test.mp4");

    EXPECT_FALSE(source->hasDRM());

    bool result = source->setDRMLicense("test_license");
    EXPECT_FALSE(result);  // Mock backend doesn't support DRM
}
