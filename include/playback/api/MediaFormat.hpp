#pragma once

#include <optional>
#include <string>

namespace playback {

enum class MediaType { AUDIO, VIDEO, SUBTITLE };

enum class Codec { H264, H265, VP9, AV1, AAC, OPUS, MP3, UNKNOWN };

enum class ContainerFormat { MP4, MKV, WEBM, MPEGTS, DASH, HLS, UNKNOWN };

struct VideoFormat {
    uint32_t width;
    uint32_t height;
    double frameRate;
    uint32_t bitrate;
    std::string colorSpace;
    std::optional<uint32_t> rotation;
};

struct AudioFormat {
    uint32_t sampleRate;
    uint32_t channels;
    uint32_t bitrate;
    std::string channelLayout;
};

struct MediaFormat {
    MediaType type;
    Codec codec;
    ContainerFormat container;
    std::optional<VideoFormat> video;
    std::optional<AudioFormat> audio;
    std::string mimeType;
    uint64_t durationMs;
};

struct AdaptiveStream {
    std::string url;
    uint32_t bitrate;
    MediaFormat format;
    std::string codecs;
    uint32_t width;
    uint32_t height;
};

}  // namespace playback
