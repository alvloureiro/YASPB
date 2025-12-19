#include "FFmpegMediaSource.hpp"

#ifdef ENABLE_FFMPEG_BACKEND

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/error.h>
}

    #include <algorithm>
    #include <cstdlib>
    #include <cstring>
    #include <iostream>

namespace playback {

// Helper function to expand tilde in path
static std::string expandPath(const std::string& path) {
    if (path.empty() || path[0] != '~') {
        return path;
    }

    const char* home = std::getenv("HOME");
    if (!home) {
        return path;  // Can't expand, return as-is
    }

    if (path.size() == 1 || path[1] == '/') {
        // ~ or ~/path
        return std::string(home) + path.substr(1);
    }

    // ~user/path - not supported, return as-is
    return path;
}

FFmpegMediaSource::FFmpegMediaSource()
    : loaded_(false), sourceUri_(), data_(), durationMs_(0), live_(false), seekable_(false),
      formatContext_(nullptr) {}

FFmpegMediaSource::~FFmpegMediaSource() {
    cleanup();
}

void FFmpegMediaSource::cleanup() {
    if (formatContext_) {
        avformat_close_input(&formatContext_);
        formatContext_ = nullptr;
    }
    loaded_ = false;
}

bool FFmpegMediaSource::probeFormat(const std::string& uri) {
    AVFormatContext* ctx = nullptr;

    // Expand path (handle tilde, etc.)
    std::string expandedUri = expandPath(uri);

    // Open input file/stream
    int ret = avformat_open_input(&ctx, expandedUri.c_str(), nullptr, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
        std::cerr << "[FFmpeg] Failed to open input: " << expandedUri << " (error: " << errbuf
                  << ", code: " << ret << ")" << std::endl;
        return false;
    }

    // Read stream information
    ret = avformat_find_stream_info(ctx, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
        std::cerr << "[FFmpeg] Failed to find stream info: " << errbuf << " (code: " << ret << ")"
                  << std::endl;
        avformat_close_input(&ctx);
        return false;
    }

    formatContext_ = ctx;

    // Extract format information
    durationMs_ = ctx->duration != AV_NOPTS_VALUE ? (ctx->duration / AV_TIME_BASE) * 1000 : 0;

    live_ = (ctx->ctx_flags & AVFMTCTX_NOHEADER) == 0 &&
            (ctx->iformat->flags & AVFMT_NOFILE) == 0 && ctx->duration == AV_NOPTS_VALUE;

    seekable_ = !live_ && ctx->pb && avio_seek(ctx->pb, 0, SEEK_SET) >= 0;

    // Extract stream formats
    formats_.clear();
    for (unsigned int i = 0; i < ctx->nb_streams; i++) {
        AVStream* stream = ctx->streams[i];
        AVCodecParameters* codecpar = stream->codecpar;

        MediaFormat format;
        format.durationMs = durationMs_;

        // Determine media type
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            format.type = MediaType::VIDEO;
            VideoFormat videoFormat;
            videoFormat.width = codecpar->width;
            videoFormat.height = codecpar->height;
            videoFormat.frameRate =
                stream->avg_frame_rate.num > 0
                    ? static_cast<double>(stream->avg_frame_rate.num) / stream->avg_frame_rate.den
                    : 0.0;
            videoFormat.bitrate = codecpar->bit_rate > 0 ? codecpar->bit_rate : 0;
            format.video = videoFormat;
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            format.type = MediaType::AUDIO;
            AudioFormat audioFormat;
            audioFormat.sampleRate = codecpar->sample_rate;
    // Note: channels is deprecated in FFmpeg 5.1+, but we use it for compatibility
    // In the future, this should use ch_layout.nb_channels for newer versions
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            audioFormat.channels = codecpar->channels;
    #pragma GCC diagnostic pop
            audioFormat.bitrate = codecpar->bit_rate > 0 ? codecpar->bit_rate : 0;
            format.audio = audioFormat;
        } else if (codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            format.type = MediaType::SUBTITLE;
        } else {
            continue;  // Skip unknown stream types
        }

        // Determine codec
        switch (codecpar->codec_id) {
            case AV_CODEC_ID_H264:
                format.codec = Codec::H264;
                break;
            case AV_CODEC_ID_HEVC:
                format.codec = Codec::H265;
                break;
            case AV_CODEC_ID_VP9:
                format.codec = Codec::VP9;
                break;
            case AV_CODEC_ID_AV1:
                format.codec = Codec::AV1;
                break;
            case AV_CODEC_ID_AAC:
                format.codec = Codec::AAC;
                break;
            case AV_CODEC_ID_OPUS:
                format.codec = Codec::OPUS;
                break;
            case AV_CODEC_ID_MP3:
                format.codec = Codec::MP3;
                break;
            default:
                format.codec = Codec::UNKNOWN;
                break;
        }

        // Determine container
        if (ctx->iformat) {
            if (strcmp(ctx->iformat->name, "mov,mp4,m4a,3gp,3g2,mj2") == 0 ||
                strcmp(ctx->iformat->name, "mp4") == 0) {
                format.container = ContainerFormat::MP4;
            } else if (strcmp(ctx->iformat->name, "matroska,webm") == 0 ||
                       strcmp(ctx->iformat->name, "mkv") == 0) {
                format.container = ContainerFormat::MKV;
            } else if (strcmp(ctx->iformat->name, "webm") == 0) {
                format.container = ContainerFormat::WEBM;
            } else if (strcmp(ctx->iformat->name, "mpegts") == 0) {
                format.container = ContainerFormat::MPEGTS;
            } else {
                format.container = ContainerFormat::UNKNOWN;
            }
        }

        // Set MIME type based on container and codec
        if (format.container == ContainerFormat::MP4) {
            format.mimeType = format.type == MediaType::VIDEO ? "video/mp4" : "audio/mp4";
        } else if (format.container == ContainerFormat::MKV) {
            format.mimeType = "video/x-matroska";
        } else if (format.container == ContainerFormat::WEBM) {
            format.mimeType = format.type == MediaType::VIDEO ? "video/webm" : "audio/webm";
        } else {
            format.mimeType = "application/octet-stream";
        }

        formats_.push_back(format);
    }

    return true;
}

bool FFmpegMediaSource::probeFormat(const std::vector<uint8_t>& data) {
    // For in-memory data, we need to use a custom AVIO context
    // This is a simplified version - full implementation would use avio_alloc_context
    // For now, we'll just store the data and mark as loaded
    data_ = data;
    loaded_ = true;
    // Note: Full implementation would require creating a custom AVIO context
    // to read from memory buffer
    return true;
}

bool FFmpegMediaSource::load(const std::string& sourceUri) {
    cleanup();

    // Expand path before storing
    std::string expandedUri = expandPath(sourceUri);

    if (probeFormat(expandedUri)) {
        sourceUri_ = expandedUri;  // Store expanded path
        loaded_ = true;
        return true;
    }

    return false;
}

bool FFmpegMediaSource::load(const std::vector<uint8_t>& data) {
    cleanup();

    if (probeFormat(data)) {
        loaded_ = true;
        return true;
    }

    return false;
}

std::vector<MediaFormat> FFmpegMediaSource::getAvailableFormats() const {
    return formats_;
}

std::vector<AdaptiveStream> FFmpegMediaSource::getAdaptiveStreams() const {
    // FFmpeg can parse HLS and DASH manifests
    // This would require additional parsing logic
    return adaptiveStreams_;
}

bool FFmpegMediaSource::isLive() const {
    return live_;
}

bool FFmpegMediaSource::isSeekable() const {
    return seekable_;
}

uint64_t FFmpegMediaSource::getDurationMs() const {
    return durationMs_;
}

bool FFmpegMediaSource::hasDRM() const {
    // FFmpeg has limited DRM support
    // This would need to check for DRM-specific metadata
    return false;
}

bool FFmpegMediaSource::setDRMLicense(const std::string& /*license*/) {
    // FFmpeg DRM support is limited
    // This would require integration with DRM libraries
    return false;
}

std::string FFmpegMediaSource::getTitle() const {
    if (!formatContext_) {
        return "";
    }

    AVDictionaryEntry* tag = av_dict_get(formatContext_->metadata, "title", nullptr, 0);
    return tag ? tag->value : "";
}

std::string FFmpegMediaSource::getArtist() const {
    if (!formatContext_) {
        return "";
    }

    AVDictionaryEntry* tag = av_dict_get(formatContext_->metadata, "artist", nullptr, 0);
    return tag ? tag->value : "";
}

std::string FFmpegMediaSource::getAlbum() const {
    if (!formatContext_) {
        return "";
    }

    AVDictionaryEntry* tag = av_dict_get(formatContext_->metadata, "album", nullptr, 0);
    return tag ? tag->value : "";
}

std::vector<uint8_t> FFmpegMediaSource::getThumbnail() const {
    // FFmpeg can extract embedded thumbnails from media files
    // This would require additional implementation
    return std::vector<uint8_t>();
}

std::string FFmpegMediaSource::getSourceUri() const {
    return sourceUri_;
}

}  // namespace playback

#endif  // ENABLE_FFMPEG_BACKEND
