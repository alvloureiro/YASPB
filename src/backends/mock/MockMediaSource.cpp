#include "MockMediaSource.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace playback {

MockMediaSource::MockMediaSource()
    : loaded_(false), durationMs_(0), live_(false), seekable_(true) {}

bool MockMediaSource::load(const std::string& sourceUri) {
    loaded_ = true;
    sourceUri_ = sourceUri;

    // Simulate loading different formats based on URI
    if (sourceUri.find(".mp4") != std::string::npos) {
        durationMs_ = 120000;  // 2 minutes
    } else if (sourceUri.find(".mkv") != std::string::npos) {
        durationMs_ = 180000;  // 3 minutes
    } else {
        durationMs_ = 60000;  // 1 minute default
    }

    return true;
}

bool MockMediaSource::load(const std::vector<uint8_t>& data) {
    loaded_ = true;
    data_ = data;
    durationMs_ = 60000;  // 1 minute default
    return true;
}

std::vector<MediaFormat> MockMediaSource::getAvailableFormats() const {
    std::vector<MediaFormat> formats;

    if (loaded_) {
        MediaFormat format;
        format.type = MediaType::VIDEO;
        format.codec = Codec::H264;
        format.container = ContainerFormat::MP4;
        format.mimeType = "video/mp4";
        format.durationMs = durationMs_;

        VideoFormat video;
        video.width = 1920;
        video.height = 1080;
        video.frameRate = 30.0;
        video.bitrate = 5000000;
        video.colorSpace = "yuv420p";
        format.video = video;

        AudioFormat audio;
        audio.sampleRate = 48000;
        audio.channels = 2;
        audio.bitrate = 192000;
        audio.channelLayout = "stereo";
        format.audio = audio;

        formats.push_back(format);
    }

    return formats;
}

std::vector<AdaptiveStream> MockMediaSource::getAdaptiveStreams() const {
    std::vector<AdaptiveStream> streams;

    if (loaded_) {
        AdaptiveStream stream;
        stream.url = sourceUri_;
        stream.bitrate = 5000000;
        stream.width = 1920;
        stream.height = 1080;
        stream.codecs = "avc1.640028,mp4a.40.2";

        MediaFormat format;
        format.type = MediaType::VIDEO;
        format.codec = Codec::H264;
        format.container = ContainerFormat::MP4;
        format.mimeType = "video/mp4";
        format.durationMs = durationMs_;
        stream.format = format;

        streams.push_back(stream);
    }

    return streams;
}

bool MockMediaSource::isLive() const {
    return live_;
}

bool MockMediaSource::isSeekable() const {
    return seekable_;
}

uint64_t MockMediaSource::getDurationMs() const {
    return durationMs_;
}

bool MockMediaSource::hasDRM() const {
    return false;
}

bool MockMediaSource::setDRMLicense(const std::string& license) {
    (void)license;  // Unused
    return false;
}

std::string MockMediaSource::getTitle() const {
    return "Mock Media Title";
}

std::string MockMediaSource::getArtist() const {
    return "Mock Artist";
}

std::string MockMediaSource::getAlbum() const {
    return "Mock Album";
}

std::vector<uint8_t> MockMediaSource::getThumbnail() const {
    // Return empty thumbnail
    return std::vector<uint8_t>();
}

}  // namespace playback
