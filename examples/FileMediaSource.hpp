#pragma once

#include <memory>
#include <string>
#include <vector>

#include "playback/api/IMediaSource.hpp"
#include "playback/api/MediaFormat.hpp"

namespace playback {

// Simple file-based media source implementation for examples
class FileMediaSource : public IMediaSource {
   public:
    FileMediaSource() : loaded_(false), durationMs_(0), live_(false), seekable_(true) {}

    bool load(const std::string& sourceUri) override {
        loaded_ = true;
        sourceUri_ = sourceUri;
        // Duration will be determined by the backend when loading
        durationMs_ = 0;  // Unknown until loaded by backend
        return true;
    }

    bool load(const std::vector<uint8_t>& data) override {
        loaded_ = true;
        data_ = data;
        durationMs_ = 0;
        return true;
    }

    std::vector<MediaFormat> getAvailableFormats() const override {
        std::vector<MediaFormat> formats;
        if (loaded_) {
            MediaFormat format;
            // Determine format based on file extension
            if (sourceUri_.find(".mp3") != std::string::npos) {
                format.type = MediaType::AUDIO;
                format.codec = Codec::MP3;
                format.container = ContainerFormat::MP4;
                format.mimeType = "audio/mpeg";

                AudioFormat audio;
                audio.sampleRate = 44100;
                audio.channels = 2;
                audio.bitrate = 128000;
                audio.channelLayout = "stereo";
                format.audio = audio;
            } else if (sourceUri_.find(".m4a") != std::string::npos ||
                       sourceUri_.find(".aac") != std::string::npos) {
                format.type = MediaType::AUDIO;
                format.codec = Codec::AAC;
                format.container = ContainerFormat::MP4;
                format.mimeType = "audio/mp4";

                AudioFormat audio;
                audio.sampleRate = 48000;
                audio.channels = 2;
                audio.bitrate = 192000;
                audio.channelLayout = "stereo";
                format.audio = audio;
            } else if (sourceUri_.find(".mp4") != std::string::npos) {
                format.type = MediaType::VIDEO;
                format.codec = Codec::H264;
                format.container = ContainerFormat::MP4;
                format.mimeType = "video/mp4";

                VideoFormat video;
                video.width = 1920;
                video.height = 1080;
                video.frameRate = 30.0;
                video.bitrate = 5000000;
                video.colorSpace = "yuv420p";
                format.video = video;
            } else {
                // Default to audio
                format.type = MediaType::AUDIO;
                format.codec = Codec::AAC;
                format.container = ContainerFormat::MP4;
                format.mimeType = "audio/mp4";
            }

            format.durationMs = durationMs_;
            formats.push_back(format);
        }
        return formats;
    }

    std::vector<AdaptiveStream> getAdaptiveStreams() const override {
        return {};  // No adaptive streams for file-based sources
    }

    bool isLive() const override {
        return live_;
    }

    bool isSeekable() const override {
        return seekable_;
    }

    uint64_t getDurationMs() const override {
        return durationMs_;
    }

    bool hasDRM() const override {
        return false;
    }

    bool setDRMLicense(const std::string& license) override {
        (void)license;
        return false;
    }

    std::string getTitle() const override {
        // Extract filename without extension
        size_t lastSlash = sourceUri_.find_last_of("/\\");
        size_t lastDot = sourceUri_.find_last_of(".");
        if (lastDot != std::string::npos && lastDot > lastSlash) {
            return sourceUri_.substr(lastSlash + 1, lastDot - lastSlash - 1);
        }
        return sourceUri_;
    }

    std::string getArtist() const override {
        return "";
    }

    std::string getAlbum() const override {
        return "";
    }

    std::vector<uint8_t> getThumbnail() const override {
        return {};
    }

    // Helper method to get the source URI
    std::string getSourceUri() const {
        return sourceUri_;
    }

   private:
    bool loaded_;
    std::string sourceUri_;
    std::vector<uint8_t> data_;
    uint64_t durationMs_;
    bool live_;
    bool seekable_;
};

}  // namespace playback
