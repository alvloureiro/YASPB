#pragma once

#include <memory>
#include <string>
#include <vector>

#include "playback/api/IMediaSource.hpp"
#include "playback/api/MediaFormat.hpp"

// Forward declarations for FFmpeg types (in global namespace)
struct AVFormatContext;

namespace playback {

// FFmpeg Media Source Implementation
class FFmpegMediaSource : public IMediaSource {
   public:
    FFmpegMediaSource();
    ~FFmpegMediaSource() override;

    bool load(const std::string& sourceUri) override;
    bool load(const std::vector<uint8_t>& data) override;

    [[nodiscard]] std::vector<MediaFormat> getAvailableFormats() const override;
    [[nodiscard]] std::vector<AdaptiveStream> getAdaptiveStreams() const override;

    [[nodiscard]] bool isLive() const override;
    [[nodiscard]] bool isSeekable() const override;
    [[nodiscard]] uint64_t getDurationMs() const override;

    [[nodiscard]] bool hasDRM() const override;
    bool setDRMLicense(const std::string& license) override;

    [[nodiscard]] std::string getTitle() const override;
    [[nodiscard]] std::string getArtist() const override;
    [[nodiscard]] std::string getAlbum() const override;
    [[nodiscard]] std::vector<uint8_t> getThumbnail() const override;

    // Helper method to get the source URI (for use by FFmpegPlaybackController)
    [[nodiscard]] std::string getSourceUri() const;

   private:
    bool loaded_;
    std::string sourceUri_;
    std::vector<uint8_t> data_;
    uint64_t durationMs_;
    bool live_;
    bool seekable_;
    std::vector<MediaFormat> formats_;
    std::vector<AdaptiveStream> adaptiveStreams_;

    // FFmpeg context (opaque pointer)
    ::AVFormatContext* formatContext_;

    void cleanup();
    bool probeFormat(const std::string& uri);
    bool probeFormat(const std::vector<uint8_t>& data);
};

}  // namespace playback
