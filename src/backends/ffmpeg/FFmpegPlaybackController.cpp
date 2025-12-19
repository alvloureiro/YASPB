#include "FFmpegPlaybackController.hpp"

#include "FFmpegMediaSource.hpp"

#ifdef ENABLE_FFMPEG_BACKEND

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/error.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/opt.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
}

// Suppress FFmpeg log messages for timestamp warnings (they're usually harmless)
static void ffmpeg_log_callback(void* /*ptr*/, int level, const char* fmt, va_list vl) {
    // Only show errors and fatal messages, suppress warnings about timestamps
    if (level <= AV_LOG_ERROR) {
        char log_buffer[1024];
        vsnprintf(log_buffer, sizeof(log_buffer), fmt, vl);
        // Filter out timestamp warnings
        if (strstr(log_buffer, "Could not update timestamps") == nullptr) {
            fprintf(stderr, "[FFmpeg] %s", log_buffer);
        }
    }
}

    #include <algorithm>
    #include <chrono>
    #include <cstdio>
    #include <cstring>
    #include <iostream>
    #include <random>
    #include <sstream>

namespace playback {

FFmpegPlaybackController::FFmpegPlaybackController(std::shared_ptr<IMediaSource> source,
                                                   const PlaybackConfig& config)
    : formatContext_(nullptr), videoCodecContext_(nullptr), audioCodecContext_(nullptr),
      videoStreamIndex_(-1), audioStreamIndex_(-1),
    #ifdef __APPLE__
      audioQueue_(nullptr),
    #endif
      source_(source), config_(config), state_(PlaybackState::IDLE), currentPositionMs_(0),
      durationMs_(0), playbackRate_(1.0), volume_(1.0), shouldStop_(false) {
    // Generate session ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << dis(gen);
    }
    sessionId_ = ss.str();

    // Initialize stats
    stats_ = PlaybackStats{0.0, 0.0, 0.0, 0, 0.0};

    // Get available qualities from source
    if (source_) {
        availableQualities_ = source_->getAvailableFormats();
        if (!availableQualities_.empty()) {
            currentQuality_ = availableQualities_[0];
        }
        durationMs_ = source_->getDurationMs();

        // Try to open media if source is already loaded
        // This allows the controller to be ready immediately
        if (openMedia()) {
            state_ = PlaybackState::PAUSED;  // Media is ready but not playing
        }
    }
}

FFmpegPlaybackController::~FFmpegPlaybackController() {
    stop();
    cleanupFFmpeg();
}

bool FFmpegPlaybackController::initializeFFmpeg() {
    // FFmpeg doesn't require explicit initialization in newer versions
    // Set log level to suppress warnings (timestamp warnings for MP3 are usually harmless)
    av_log_set_level(AV_LOG_ERROR);
    return true;
}

void FFmpegPlaybackController::cleanupFFmpeg() {
    closeMedia();
}

bool FFmpegPlaybackController::openMedia() {
    if (!source_) {
        return false;
    }

    // Try to get URI from FFmpegMediaSource
    // If the source is an FFmpegMediaSource, we can get the URI directly
    std::string uri;
    auto ffmpegSource = std::dynamic_pointer_cast<FFmpegMediaSource>(source_);
    if (ffmpegSource) {
        uri = ffmpegSource->getSourceUri();
    }

    // If we don't have a URI, we can't open the media with FFmpeg
    // (in-memory data would require a custom AVIO context)
    if (uri.empty()) {
        return false;
    }

    // Open input file
    int ret = avformat_open_input(&formatContext_, uri.c_str(), nullptr, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
        std::cerr << "[FFmpeg] Failed to open input in controller: " << uri << " (error: " << errbuf
                  << ", code: " << ret << ")" << std::endl;
        return false;
    }

    // Find stream info
    ret = avformat_find_stream_info(formatContext_, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
        std::cerr << "[FFmpeg] Failed to find stream info in controller: " << errbuf
                  << " (code: " << ret << ")" << std::endl;
        avformat_close_input(&formatContext_);
        formatContext_ = nullptr;
        return false;
    }

    // Find video and audio streams
    videoStreamIndex_ = -1;
    audioStreamIndex_ = -1;

    for (unsigned int i = 0; i < formatContext_->nb_streams; i++) {
        AVCodecParameters* codecpar = formatContext_->streams[i]->codecpar;

        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStreamIndex_ < 0) {
            videoStreamIndex_ = i;
            const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
            if (codec) {
                videoCodecContext_ = avcodec_alloc_context3(codec);
                if (videoCodecContext_) {
                    avcodec_parameters_to_context(videoCodecContext_, codecpar);
                    if (avcodec_open2(videoCodecContext_, codec, nullptr) >= 0) {
                        // Successfully opened video codec
                    } else {
                        avcodec_free_context(&videoCodecContext_);
                        videoCodecContext_ = nullptr;
                        videoStreamIndex_ = -1;
                    }
                }
            }
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audioStreamIndex_ < 0) {
            audioStreamIndex_ = i;
            const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
            if (codec) {
                audioCodecContext_ = avcodec_alloc_context3(codec);
                if (audioCodecContext_) {
                    avcodec_parameters_to_context(audioCodecContext_, codecpar);
                    if (avcodec_open2(audioCodecContext_, codec, nullptr) >= 0) {
                        // Successfully opened audio codec
                    } else {
                        avcodec_free_context(&audioCodecContext_);
                        audioCodecContext_ = nullptr;
                        audioStreamIndex_ = -1;
                    }
                }
            }
        }
    }

    // Update duration
    if (formatContext_->duration != AV_NOPTS_VALUE) {
        durationMs_ = (formatContext_->duration / AV_TIME_BASE) * 1000;
    }

    // Setup audio output if we have an audio stream
    #ifdef __APPLE__
    if (audioStreamIndex_ >= 0 && audioCodecContext_) {
        setupAudioOutput();
    }
    #endif

    return true;
}

void FFmpegPlaybackController::closeMedia() {
    #ifdef __APPLE__
    cleanupAudioOutput();
    #endif

    if (videoCodecContext_) {
        avcodec_free_context(&videoCodecContext_);
        videoCodecContext_ = nullptr;
    }

    if (audioCodecContext_) {
        avcodec_free_context(&audioCodecContext_);
        audioCodecContext_ = nullptr;
    }

    if (formatContext_) {
        avformat_close_input(&formatContext_);
        formatContext_ = nullptr;
    }

    videoStreamIndex_ = -1;
    audioStreamIndex_ = -1;
}

bool FFmpegPlaybackController::play() {
    std::lock_guard<std::mutex> lock(stateMutex_);

    if (state_ == PlaybackState::PLAYING) {
        return true;
    }

    // Open media if not already open
    if (state_ == PlaybackState::IDLE || state_ == PlaybackState::STOPPED) {
        if (!openMedia()) {
            notifyError(PlaybackError::FORMAT_ERROR, "Failed to open media");
            return false;
        }
    }

    // If media was already open (PAUSED state), just start playing
    if (state_ == PlaybackState::PAUSED) {
        // Media is already open, just start the playback thread
    }

    state_ = PlaybackState::PLAYING;

    if (!playbackThread_.joinable()) {
        shouldStop_ = false;
        playbackThread_ = std::thread(&FFmpegPlaybackController::playbackLoop, this);
    }

    notifyStateChange();
    return true;
}

bool FFmpegPlaybackController::pause() {
    std::lock_guard<std::mutex> lock(stateMutex_);

    if (state_ == PlaybackState::PAUSED) {
        return true;
    }

    if (state_ == PlaybackState::PLAYING) {
        state_ = PlaybackState::PAUSED;
        notifyStateChange();
        return true;
    }

    return false;
}

bool FFmpegPlaybackController::stop() {
    shouldStop_ = true;

    if (playbackThread_.joinable()) {
        playbackThread_.join();
    }

    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        state_ = PlaybackState::STOPPED;
        currentPositionMs_ = 0;
    }

    closeMedia();

    notifyStateChange();
    return true;
}

bool FFmpegPlaybackController::seek(uint64_t positionMs) {
    std::lock_guard<std::mutex> lock(stateMutex_);

    if (!formatContext_ || positionMs > durationMs_) {
        return false;
    }

    int64_t timestamp = (positionMs * AV_TIME_BASE) / 1000;
    int ret = av_seek_frame(formatContext_, -1, timestamp, AVSEEK_FLAG_BACKWARD);

    if (ret >= 0) {
        // Flush codec buffers
        if (videoCodecContext_) {
            avcodec_flush_buffers(videoCodecContext_);
        }
        if (audioCodecContext_) {
            avcodec_flush_buffers(audioCodecContext_);
        }

        currentPositionMs_ = positionMs;
        notifySeekCompleted();
        return true;
    }

    return false;
}

bool FFmpegPlaybackController::setPlaybackRate(double rate) {
    if (rate <= 0.0 || rate > 4.0) {
        return false;
    }

    playbackRate_ = rate;
    return true;
}

double FFmpegPlaybackController::getPlaybackRate() const {
    return playbackRate_;
}

bool FFmpegPlaybackController::setVolume(double volume) {
    if (volume < 0.0 || volume > 1.0) {
        return false;
    }

    volume_ = volume;
    return true;
}

double FFmpegPlaybackController::getVolume() const {
    return volume_;
}

bool FFmpegPlaybackController::setQuality(const MediaFormat& format) {
    // Check if format is available
    auto it = std::find_if(availableQualities_.begin(), availableQualities_.end(),
                           [&format](const MediaFormat& f) {
                               return f.type == format.type && f.codec == format.codec;
                           });

    if (it != availableQualities_.end()) {
        currentQuality_ = format;
        notifyQualityChanged();
        return true;
    }

    return false;
}

MediaFormat FFmpegPlaybackController::getCurrentQuality() const {
    return currentQuality_;
}

std::vector<MediaFormat> FFmpegPlaybackController::getAvailableQualities() const {
    return availableQualities_;
}

IPlaybackController::PlaybackStats FFmpegPlaybackController::getStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
}

PlaybackState FFmpegPlaybackController::getState() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return state_;
}

uint64_t FFmpegPlaybackController::getCurrentPosition() const {
    return currentPositionMs_;
}

uint64_t FFmpegPlaybackController::getDuration() const {
    return durationMs_;
}

bool FFmpegPlaybackController::configure(const PlaybackConfig& config) {
    config_ = config;
    return true;
}

PlaybackConfig FFmpegPlaybackController::getConfig() const {
    return config_;
}

void FFmpegPlaybackController::addEventListener(std::shared_ptr<IPlaybackEventListener> listener) {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    listeners_.push_back(std::weak_ptr<IPlaybackEventListener>(listener));
}

void FFmpegPlaybackController::removeEventListener(
    std::shared_ptr<IPlaybackEventListener> listener) {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    listeners_.erase(std::remove_if(listeners_.begin(), listeners_.end(),
                                    [&listener](const std::weak_ptr<IPlaybackEventListener>& weak) {
                                        return weak.lock() == listener;
                                    }),
                     listeners_.end());
}

std::string FFmpegPlaybackController::getSessionId() const {
    return sessionId_;
}

void FFmpegPlaybackController::playbackLoop() {
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    if (!packet || !frame) {
        notifyError(PlaybackError::DECODE_ERROR, "Failed to allocate FFmpeg structures");
        av_packet_free(&packet);
        av_frame_free(&frame);
        return;
    }

    auto lastUpdateTime = std::chrono::steady_clock::now();

    while (!shouldStop_) {
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            if (state_ != PlaybackState::PLAYING) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
        }

        // Read packet
        int ret = av_read_frame(formatContext_, packet);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                // End of file
                std::lock_guard<std::mutex> lock(stateMutex_);
                state_ = PlaybackState::ENDED;
                notifyStateChange();
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Process video packet
        if (packet->stream_index == videoStreamIndex_ && videoCodecContext_) {
            ret = avcodec_send_packet(videoCodecContext_, packet);
            if (ret >= 0) {
                ret = avcodec_receive_frame(videoCodecContext_, frame);
                if (ret >= 0) {
                    // Update position
                    if (frame->pts != AV_NOPTS_VALUE) {
                        AVRational timeBase = formatContext_->streams[videoStreamIndex_]->time_base;
                        int64_t pts = frame->best_effort_timestamp != AV_NOPTS_VALUE
                                          ? frame->best_effort_timestamp
                                          : frame->pts;
                        currentPositionMs_ = (pts * timeBase.num * 1000) / timeBase.den;
                    }

                    // Notify listeners (simplified - would need proper format conversion)
                    // TODO: Convert frame to proper format and notify listeners
                }
            }
        }

        // Process audio packet
        if (packet->stream_index == audioStreamIndex_ && audioCodecContext_) {
            ret = avcodec_send_packet(audioCodecContext_, packet);
            if (ret >= 0) {
                // Keep receiving frames until the decoder is empty
                while (ret >= 0) {
                    ret = avcodec_receive_frame(audioCodecContext_, frame);
                    if (ret >= 0) {
                        // Update position from audio stream
                        if (frame->pts != AV_NOPTS_VALUE && audioStreamIndex_ >= 0) {
                            AVRational timeBase =
                                formatContext_->streams[audioStreamIndex_]->time_base;
                            int64_t pts = frame->best_effort_timestamp != AV_NOPTS_VALUE
                                              ? frame->best_effort_timestamp
                                              : frame->pts;
                            if (pts != AV_NOPTS_VALUE) {
                                currentPositionMs_ = (pts * timeBase.num * 1000) / timeBase.den;
                            }
                        }

    // Output audio to speakers (macOS CoreAudio)
    #ifdef __APPLE__
                        outputAudioFrame(frame);
    #endif

                        // Notify listeners
                        if (auto listener = getFirstListener()) {
                            // Convert frame format for listener
                            AudioFormat audioFormat;
                            audioFormat.sampleRate = frame->sample_rate;
    // Get channel count - use new API if available, fallback to old
    #if LIBAVUTIL_VERSION_MAJOR >= 58
                            audioFormat.channels = frame->ch_layout.nb_channels > 0
                                                       ? frame->ch_layout.nb_channels
                                                       : 2;  // Default to stereo
    #else
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                            audioFormat.channels = frame->channels;
        #pragma GCC diagnostic pop
    #endif
                            audioFormat.bitrate = 0;               // Not available from frame
                            audioFormat.channelLayout = "stereo";  // Simplified

                            // Get audio data
                            int channels = audioFormat.channels;
                            size_t dataSize =
                                frame->nb_samples * channels *
                                av_get_bytes_per_sample((AVSampleFormat)frame->format);
                            listener->onAudioData(frame->data[0], dataSize, audioFormat);
                        }
                    } else if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        // Need more input or end of stream
                        break;
                    }
                }
            } else if (ret == AVERROR(EAGAIN)) {
                // Decoder needs more input, but we'll continue
            } else if (ret < 0 && ret != AVERROR_EOF) {
                // Error decoding packet (but continue anyway)
                // The timestamp warning is usually harmless
            }
        }

        av_packet_unref(packet);

        // Update position periodically
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime).count() >
            100) {
            notifyPositionChanged();
            lastUpdateTime = now;
        }

        // Apply playback rate
        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<int>(10.0 / playbackRate_)));
    }

    av_packet_free(&packet);
    av_frame_free(&frame);
}

void FFmpegPlaybackController::notifyStateChange() {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::STATE_CHANGED;
    event.state = state_;
    event.positionMs = currentPositionMs_;
    event.durationMs = durationMs_;

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto it = listeners_.begin(); it != listeners_.end();) {
        if (auto listener = it->lock()) {
            listener->onPlaybackEvent(event);
            ++it;
        } else {
            it = listeners_.erase(it);
        }
    }
}

void FFmpegPlaybackController::notifyPositionChanged() {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::POSITION_CHANGED;
    event.state = state_;
    event.positionMs = currentPositionMs_;
    event.durationMs = durationMs_;

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto it = listeners_.begin(); it != listeners_.end();) {
        if (auto listener = it->lock()) {
            listener->onPlaybackEvent(event);
            ++it;
        } else {
            it = listeners_.erase(it);
        }
    }
}

void FFmpegPlaybackController::notifySeekCompleted() {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::SEEK_COMPLETED;
    event.state = state_;
    event.positionMs = currentPositionMs_;
    event.durationMs = durationMs_;

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto it = listeners_.begin(); it != listeners_.end();) {
        if (auto listener = it->lock()) {
            listener->onPlaybackEvent(event);
            ++it;
        } else {
            it = listeners_.erase(it);
        }
    }
}

void FFmpegPlaybackController::notifyQualityChanged() {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::QUALITY_CHANGED;
    event.state = state_;
    event.positionMs = currentPositionMs_;
    event.durationMs = durationMs_;

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto it = listeners_.begin(); it != listeners_.end();) {
        if (auto listener = it->lock()) {
            listener->onPlaybackEvent(event);
            ++it;
        } else {
            it = listeners_.erase(it);
        }
    }
}

void FFmpegPlaybackController::notifyError(PlaybackError error, const std::string& message) {
    PlaybackEvent event;
    event.type = PlaybackEvent::Type::ERROR_OCCURRED;
    event.state = PlaybackState::ERROR;
    event.error = error;
    event.message = message;
    event.positionMs = currentPositionMs_;
    event.durationMs = durationMs_;

    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (auto it = listeners_.begin(); it != listeners_.end();) {
        if (auto listener = it->lock()) {
            listener->onPlaybackEvent(event);
            ++it;
        } else {
            it = listeners_.erase(it);
        }
    }

    {
        std::lock_guard<std::mutex> stateLock(stateMutex_);
        state_ = PlaybackState::ERROR;
    }
}

std::shared_ptr<IPlaybackEventListener> FFmpegPlaybackController::getFirstListener() const {
    std::lock_guard<std::mutex> lock(listenersMutex_);
    for (const auto& weak : listeners_) {
        if (auto listener = weak.lock()) {
            return listener;
        }
    }
    return nullptr;
}

    #ifdef __APPLE__
        #include <AudioToolbox/AudioToolbox.h>
        #include <CoreAudio/CoreAudio.h>

// AudioQueue callback
static void audioQueueCallback(void* userData, AudioQueueRef queue, AudioQueueBufferRef buffer) {
    // Buffer has been played, can be reused
    // For now, we'll handle this in the output function
    (void)userData;
    (void)queue;
    (void)buffer;
}

void FFmpegPlaybackController::setupAudioOutput() {
    if (!audioCodecContext_ || audioStreamIndex_ < 0) {
        return;
    }

    AudioStreamBasicDescription asbd = {};
    asbd.mSampleRate = audioCodecContext_->sample_rate;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    asbd.mBitsPerChannel = 16;
    // Get channel count - use new API if available, fallback to old
    int channels = 2;  // Default to stereo
        #if LIBAVCODEC_VERSION_MAJOR >= 59
    if (audioCodecContext_->ch_layout.nb_channels > 0) {
        channels = audioCodecContext_->ch_layout.nb_channels;
    }
        #else
    if (audioCodecContext_->channels > 0) {
        channels = audioCodecContext_->channels;
    }
        #endif
    asbd.mChannelsPerFrame = channels;
    asbd.mBytesPerFrame = asbd.mChannelsPerFrame * (asbd.mBitsPerChannel / 8);
    asbd.mFramesPerPacket = 1;
    asbd.mBytesPerPacket = asbd.mBytesPerFrame * asbd.mFramesPerPacket;

    OSStatus status = AudioQueueNewOutput(&asbd, audioQueueCallback, this, nullptr, nullptr, 0,
                                          (AudioQueueRef*)&audioQueue_);
    if (status != noErr) {
        std::cerr << "[FFmpeg] Failed to create audio queue: " << status << std::endl;
        audioQueue_ = nullptr;
        return;
    }

    // Set volume
    AudioQueueSetParameter((AudioQueueRef)audioQueue_, kAudioQueueParam_Volume, volume_);

    // Start the queue
    status = AudioQueueStart((AudioQueueRef)audioQueue_, nullptr);
    if (status != noErr) {
        std::cerr << "[FFmpeg] Failed to start audio queue: " << status << std::endl;
        AudioQueueDispose((AudioQueueRef)audioQueue_, true);
        audioQueue_ = nullptr;
    }
}

void FFmpegPlaybackController::cleanupAudioOutput() {
    if (audioQueue_) {
        AudioQueueStop((AudioQueueRef)audioQueue_, true);
        AudioQueueDispose((AudioQueueRef)audioQueue_, true);
        audioQueue_ = nullptr;
    }
}

void FFmpegPlaybackController::outputAudioFrame(::AVFrame* frame) {
    if (!audioQueue_ || !frame || !audioCodecContext_) {
        return;
    }

    // Convert frame to PCM format if needed
    // For now, assume we need to convert to 16-bit PCM
    static SwrContext* swrContext = nullptr;

    if (!swrContext) {
        // Get channel layout - use new API if available, fallback to old
        uint64_t inChannelLayout = AV_CH_LAYOUT_STEREO;  // Default to stereo

        #if LIBAVCODEC_VERSION_MAJOR >= 59
        if (audioCodecContext_->ch_layout.u.mask) {
            inChannelLayout = audioCodecContext_->ch_layout.u.mask;
        }
        #else
        if (audioCodecContext_->channel_layout) {
            inChannelLayout = audioCodecContext_->channel_layout;
        }
        #endif

        // Use deprecated API with warning suppression (it's still widely used)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        swrContext = swr_alloc_set_opts(
            nullptr, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, audioCodecContext_->sample_rate,
            inChannelLayout, (AVSampleFormat)frame->format, frame->sample_rate, 0, nullptr);
        #pragma GCC diagnostic pop

        if (!swrContext || swr_init(swrContext) < 0) {
            std::cerr << "[FFmpeg] Failed to initialize audio resampler" << std::endl;
            if (swrContext) {
                swr_free(&swrContext);
            }
            return;
        }
    }

    // Allocate output buffer
    int outSamples = av_rescale_rnd(frame->nb_samples, audioCodecContext_->sample_rate,
                                    frame->sample_rate, AV_ROUND_UP);
    uint8_t* outBuffer = nullptr;
    int outLinesize = 0;
    int outBufferSize =
        av_samples_alloc(&outBuffer, &outLinesize, 2, outSamples, AV_SAMPLE_FMT_S16, 0);

    if (outBufferSize < 0) {
        return;
    }

    // Convert
    int converted = swr_convert(swrContext, &outBuffer, outSamples, (const uint8_t**)frame->data,
                                frame->nb_samples);

    if (converted > 0) {
        // Allocate AudioQueue buffer
        AudioQueueBufferRef audioBuffer;
        OSStatus status =
            AudioQueueAllocateBuffer((AudioQueueRef)audioQueue_, converted * 2 * 2, &audioBuffer);
        if (status == noErr) {
            memcpy(audioBuffer->mAudioData, outBuffer, converted * 2 * 2);
            audioBuffer->mAudioDataByteSize = converted * 2 * 2;
            audioBuffer->mPacketDescriptionCount = 0;

            status = AudioQueueEnqueueBuffer((AudioQueueRef)audioQueue_, audioBuffer, 0, nullptr);
            if (status != noErr) {
                std::cerr << "[FFmpeg] Failed to enqueue audio buffer: " << status << std::endl;
            }
        }
    }

    av_freep(&outBuffer);
}

    #endif  // __APPLE__

}  // namespace playback

#endif  // ENABLE_FFMPEG_BACKEND
