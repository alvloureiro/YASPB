#pragma once
#include <memory>
#include <string>
#include <vector>

#include "MediaFormat.hpp"

namespace playback {

class IMediaSource {
   public:
    virtual ~IMediaSource() = default;

    virtual bool load(const std::string& sourceUri) = 0;
    virtual bool load(const std::vector<uint8_t>& data) = 0;

    virtual std::vector<MediaFormat> getAvailableFormats() const = 0;
    virtual std::vector<AdaptiveStream> getAdaptiveStreams() const = 0;

    virtual bool isLive() const = 0;
    virtual bool isSeekable() const = 0;
    virtual uint64_t getDurationMs() const = 0;

    virtual bool hasDRM() const = 0;
    virtual bool setDRMLicense(const std::string& license) = 0;

    virtual std::string getTitle() const = 0;
    virtual std::string getArtist() const = 0;
    virtual std::string getAlbum() const = 0;
    virtual std::vector<uint8_t> getThumbnail() const = 0;
};

}  // namespace playback