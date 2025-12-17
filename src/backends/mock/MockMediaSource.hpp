#pragma once

#include <memory>
#include <string>
#include <vector>

#include "playback/api/IMediaSource.hpp"
#include "playback/api/MediaFormat.hpp"

namespace playback {

// Mock Media Source Implementation
class MockMediaSource : public IMediaSource {
   public:
    MockMediaSource();
    ~MockMediaSource() override = default;

    bool load(const std::string& sourceUri) override;
    bool load(const std::vector<uint8_t>& data) override;

    std::vector<MediaFormat> getAvailableFormats() const override;
    std::vector<AdaptiveStream> getAdaptiveStreams() const override;

    bool isLive() const override;
    bool isSeekable() const override;
    uint64_t getDurationMs() const override;

    bool hasDRM() const override;
    bool setDRMLicense(const std::string& license) override;

    std::string getTitle() const override;
    std::string getArtist() const override;
    std::string getAlbum() const override;
    std::vector<uint8_t> getThumbnail() const override;

   private:
    bool loaded_;
    std::string sourceUri_;
    std::vector<uint8_t> data_;
    uint64_t durationMs_;
    bool live_;
    bool seekable_;
};

}  // namespace playback
