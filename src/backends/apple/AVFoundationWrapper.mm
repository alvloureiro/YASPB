#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <VideoToolbox/VideoToolbox.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#if TARGET_OS_IOS
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

#include "playback/backends/apple/AVFoundationWrapper.hpp"
#include "playback/api/MediaFormat.hpp"
#include <mutex>
#include <atomic>
#include <vector>
#include <string>

// Classe Objective-C interna
@interface AVFPlayerInternal : NSObject {
    @public
    AVPlayer* player;
    AVPlayerItem* playerItem;
    id timeObserver;
    playback::apple::AVFCallbacks callbacks;
    std::mutex callbackMutex;
}

- (void)setupPlayerWithURL:(NSURL*)url;
- (void)setupPlayerWithData:(NSData*)data;
- (void)setupObservers;
- (void)cleanup;

@end

// Namespace C++
namespace playback::apple {

// Implementação PIMPL em Objective-C++
class AVFPlayerImpl {
public:
    AVFPlayerImpl() : internal_([[AVFPlayerInternal alloc] init]) {}
    ~AVFPlayerImpl() { [internal_ cleanup]; }

    AVFPlayerInternal* internal() { return internal_; }

private:
    AVFPlayerInternal* __strong internal_;
};

// Implementação dos métodos wrapper
AVFPlayerWrapper::AVFPlayerWrapper() : impl_(std::make_unique<AVFPlayerImpl>()) {}

AVFPlayerWrapper::~AVFPlayerWrapper() = default;

bool AVFPlayerWrapper::load(const std::string& url) {
    @autoreleasepool {
        NSString* nsUrl = [NSString stringWithUTF8String:url.c_str()];
        NSURL* assetUrl = nil;

        // Check if it's a file path (doesn't contain ://)
        if ([nsUrl rangeOfString:@"://"].location == NSNotFound) {
            // It's a file path, use fileURLWithPath
            assetUrl = [NSURL fileURLWithPath:nsUrl];
        } else {
            // It's a URL, use URLWithString
            assetUrl = [NSURL URLWithString:nsUrl];
        }

        if (!assetUrl) return false;

        [impl_->internal() setupPlayerWithURL:assetUrl];
        return true;
    }
}

bool AVFPlayerWrapper::play() {
    @autoreleasepool {
        [impl_->internal()->player play];
        return true;
    }
}

bool AVFPlayerWrapper::pause() {
    @autoreleasepool {
        if (!impl_->internal()->player) return false;
        [impl_->internal()->player pause];
        return true;
    }
}

bool AVFPlayerWrapper::stop() {
    @autoreleasepool {
        if (!impl_->internal()->player) return false;
        [impl_->internal()->player pause];
        [impl_->internal()->player seekToTime:kCMTimeZero];
        return true;
    }
}

bool AVFPlayerWrapper::seek(uint64_t positionMs) {
    @autoreleasepool {
        if (!impl_->internal()->player) return false;
        CMTime time = CMTimeMakeWithSeconds(positionMs / 1000.0, 1000);
        [impl_->internal()->player seekToTime:time toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
        return true;
    }
}

bool AVFPlayerWrapper::setVolume(double volume) {
    @autoreleasepool {
        if (!impl_->internal()->player) return false;
        if (volume < 0.0 || volume > 1.0) return false;
        impl_->internal()->player.volume = volume;
        return true;
    }
}

double AVFPlayerWrapper::getVolume() const {
    @autoreleasepool {
        if (!impl_->internal()->player) return 0.0;
        return impl_->internal()->player.volume;
    }
}

bool AVFPlayerWrapper::setPlaybackRate(double rate) {
    @autoreleasepool {
        if (!impl_->internal()->player) return false;
        if (rate <= 0.0 || rate > 4.0) return false;
        impl_->internal()->player.rate = rate;
        return true;
    }
}

double AVFPlayerWrapper::getPlaybackRate() const {
    @autoreleasepool {
        if (!impl_->internal()->player) return 1.0;
        return impl_->internal()->player.rate;
    }
}

PlaybackState AVFPlayerWrapper::getState() const {
    @autoreleasepool {
        if (!impl_->internal()->playerItem) return PlaybackState::IDLE;

        AVPlayerItemStatus status = impl_->internal()->playerItem.status;
        if (status == AVPlayerItemStatusFailed) {
            return PlaybackState::ERROR;
        }
        if (status != AVPlayerItemStatusReadyToPlay) {
            return PlaybackState::BUFFERING;
        }

        if (impl_->internal()->player.rate == 0.0) {
            CMTime currentTime = impl_->internal()->player.currentTime;
            CMTime duration = impl_->internal()->playerItem.duration;
            if (CMTIME_COMPARE_INLINE(currentTime, ==, duration)) {
                return PlaybackState::ENDED;
            }
            return PlaybackState::PAUSED;
        }

        return PlaybackState::PLAYING;
    }
}

uint64_t AVFPlayerWrapper::getCurrentPosition() const {
    @autoreleasepool {
        if (!impl_->internal()->player) return 0;
        CMTime currentTime = impl_->internal()->player.currentTime;
        if (CMTIME_IS_VALID(currentTime)) {
            return static_cast<uint64_t>(CMTimeGetSeconds(currentTime) * 1000.0);
        }
        return 0;
    }
}

uint64_t AVFPlayerWrapper::getDuration() const {
    @autoreleasepool {
        if (!impl_->internal()->playerItem) return 0;
        CMTime duration = impl_->internal()->playerItem.duration;
        if (CMTIME_IS_VALID(duration) && !CMTIME_IS_INDEFINITE(duration)) {
            return static_cast<uint64_t>(CMTimeGetSeconds(duration) * 1000.0);
        }
        return 0;
    }
}

MediaFormat AVFPlayerWrapper::getCurrentFormat() const {
    MediaFormat format;
    @autoreleasepool {
        if (!impl_->internal()->playerItem) return format;

        AVAsset* asset = impl_->internal()->playerItem.asset;
        if (!asset) return format;

        NSArray<AVAssetTrack*>* videoTracks = [asset tracksWithMediaType:AVMediaTypeVideo];
        NSArray<AVAssetTrack*>* audioTracks = [asset tracksWithMediaType:AVMediaTypeAudio];

        if (videoTracks.count > 0) {
            format.type = MediaType::VIDEO;
            AVAssetTrack* track = videoTracks[0];
            format.video = VideoFormat{
                static_cast<uint32_t>(track.naturalSize.width),
                static_cast<uint32_t>(track.naturalSize.height),
                track.nominalFrameRate,
                static_cast<uint32_t>(track.estimatedDataRate),
                "yuv420p",
                std::nullopt
            };
        }

        if (audioTracks.count > 0) {
            format.type = MediaType::VIDEO; // Keep as video if both present
            AVAssetTrack* track = audioTracks[0];
            format.audio = AudioFormat{
                static_cast<uint32_t>(track.naturalTimeScale),
                2, // Default channels
                static_cast<uint32_t>(track.estimatedDataRate),
                "stereo"
            };
        }
    }
    return format;
}

std::vector<MediaFormat> AVFPlayerWrapper::getAvailableFormats() const {
    std::vector<MediaFormat> formats;
    @autoreleasepool {
        if (!impl_->internal()->playerItem) return formats;

        AVAsset* asset = impl_->internal()->playerItem.asset;
        if (!asset) return formats;

        MediaFormat format = getCurrentFormat();
        // Check if format is valid (has video or audio data)
        if (format.video.has_value() || format.audio.has_value()) {
            formats.push_back(format);
        }
    }
    return formats;
}

bool AVFPlayerWrapper::loadFromData(const std::vector<uint8_t>& data) {
    @autoreleasepool {
        NSData* nsData = [NSData dataWithBytes:data.data() length:data.size()];
        if (!nsData) return false;

        NSString* tempDir = NSTemporaryDirectory();
        NSString* tempFile = [tempDir stringByAppendingPathComponent:
            [NSString stringWithFormat:@"playback_%lu.tmp", (unsigned long)[nsData hash]]];

        if (![nsData writeToFile:tempFile atomically:YES]) {
            return false;
        }

        [impl_->internal() setupPlayerWithData:nsData];
        return true;
    }
}

void AVFPlayerWrapper::setAudioSessionCategory(const std::string& category) {
    // AVAudioSession is iOS-only, not available on macOS
    // On macOS, audio routing is handled automatically by the system
    #if TARGET_OS_IOS
    @autoreleasepool {
        AVAudioSession* session = [AVAudioSession sharedInstance];
        NSString* nsCategory = [NSString stringWithUTF8String:category.c_str()];
        [session setCategory:nsCategory error:nil];
    }
    #else
    (void)category; // Unused on macOS
    #endif
}

void AVFPlayerWrapper::setAudioSessionMode(const std::string& mode) {
    // AVAudioSession is iOS-only, not available on macOS
    #if TARGET_OS_IOS
    @autoreleasepool {
        AVAudioSession* session = [AVAudioSession sharedInstance];
        NSString* nsMode = [NSString stringWithUTF8String:mode.c_str()];
        [session setMode:nsMode error:nil];
    }
    #else
    (void)mode; // Unused on macOS
    #endif
}

bool AVFPlayerWrapper::supportsHDR() const {
    @autoreleasepool {
        // Check if device supports HDR
        #if TARGET_OS_IOS
        return [UIScreen mainScreen].traitCollection.displayGamut == UIDisplayGamutP3;
        #else
        return true; // macOS generally supports HDR
        #endif
    }
}

bool AVFPlayerWrapper::supportsDolbyVision() const {
    @autoreleasepool {
        // Check for Dolby Vision support
        #if TARGET_OS_IOS
        return [AVPlayerItem canPlayFastReverse] && [AVPlayerItem canPlayFastForward];
        #else
        return true; // macOS generally supports Dolby Vision
        #endif
    }
}

bool AVFPlayerWrapper::hasDRM() const {
    @autoreleasepool {
        if (!impl_->internal()->playerItem) return false;
        AVAsset* asset = impl_->internal()->playerItem.asset;
        if (!asset) return false;

        // Check for protected content using AVAsset's hasProtectedContent property
        // This is available on both iOS and macOS (iOS 4.3+, macOS 10.7+)
        // Use valueForKey for runtime safety across different SDK versions
        @try {
            id value = [asset valueForKey:@"hasProtectedContent"];
            if (value && [value isKindOfClass:[NSNumber class]]) {
                return [value boolValue];
            }
        }
        @catch (NSException* exception) {
            // Property not available on this SDK version
            (void)exception;
        }

        // If hasProtectedContent is not available, assume no DRM
        // (older macOS/iOS versions)
        return false;
    }
}

bool AVFPlayerWrapper::setDRMLicense(const std::string& license) {
    // FairPlay DRM implementation would go here
    // This is a placeholder
    (void)license; // Unused parameter
    return false;
}

void AVFPlayerWrapper::setCallbacks(const AVFCallbacks& callbacks) {
    std::lock_guard<std::mutex> lock(impl_->internal()->callbackMutex);
    impl_->internal()->callbacks = callbacks;
}

void* AVFPlayerWrapper::getMetalLayer() {
    @autoreleasepool {
        if (!impl_->internal()->player) return nullptr;
        // Return Metal layer for rendering
        // This would need AVPlayerLayer integration
        return nullptr;
    }
}

void AVFPlayerWrapper::setOutputView(void* view) {
    @autoreleasepool {
        if (!impl_->internal()->player) return;
        // Set output view for rendering
        // This would need AVPlayerLayer integration
        (void)view; // Unused parameter - placeholder for future implementation
    }
}

} // namespace playback::apple

// Implementação da classe Objective-C
@implementation AVFPlayerInternal

- (void)setupPlayerWithURL:(NSURL*)url {
    // Criar AVAsset
    AVAsset* asset = [AVAsset assetWithURL:url];

    // Criar player item
    playerItem = [AVPlayerItem playerItemWithAsset:asset];

    // Criar player
    player = [AVPlayer playerWithPlayerItem:playerItem];

    // Configurar audio session (iOS only - not available on macOS)
    #if TARGET_OS_IOS
    AVAudioSession* audioSession = [AVAudioSession sharedInstance];
    [audioSession setCategory:AVAudioSessionCategoryPlayback error:nil];
    [audioSession setActive:YES error:nil];
    #endif

    // Setup observers
    [self setupObservers];
}

- (void)setupObservers {
    // Observer para mudanças de estado
    [playerItem addObserver:self
                 forKeyPath:@"status"
                    options:NSKeyValueObservingOptionNew
                    context:nil];

    // Observer para buffer
    [playerItem addObserver:self
                 forKeyPath:@"loadedTimeRanges"
                    options:NSKeyValueObservingOptionNew
                    context:nil];

    // Time observer para posição
    __weak AVFPlayerInternal* weakSelf = self;
    timeObserver = [player addPeriodicTimeObserverForInterval:CMTimeMake(1, 10)
                                                        queue:dispatch_get_main_queue()
                                                   usingBlock:^(CMTime time) {
        AVFPlayerInternal* strongSelf = weakSelf;
        if (strongSelf) {
            std::lock_guard<std::mutex> lock(strongSelf->callbackMutex);
            if (strongSelf->callbacks.onPositionChanged) {
                uint64_t ms = CMTimeGetSeconds(time) * 1000;
                strongSelf->callbacks.onPositionChanged(ms);
            }
        }
    }];

    // Notifications
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(playerDidFinishPlaying:)
                                                 name:AVPlayerItemDidPlayToEndTimeNotification
                                               object:playerItem];
}

- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context {
    std::lock_guard<std::mutex> lock(callbackMutex);

    if ([keyPath isEqualToString:@"status"]) {
        AVPlayerItemStatus status = (AVPlayerItemStatus)[change[NSKeyValueChangeNewKey] integerValue];

        if (callbacks.onStateChanged) {
            switch (status) {
                case AVPlayerItemStatusReadyToPlay:
                    callbacks.onStateChanged(playback::PlaybackState::PLAYING);
                    break;
                case AVPlayerItemStatusFailed:
                    callbacks.onStateChanged(playback::PlaybackState::ERROR);
                    break;
                default:
                    break;
            }
        }
    }
    else if ([keyPath isEqualToString:@"loadedTimeRanges"]) {
        if (callbacks.onBufferProgress && playerItem.loadedTimeRanges.count > 0) {
            CMTimeRange range = [playerItem.loadedTimeRanges.firstObject CMTimeRangeValue];
            double progress = CMTimeGetSeconds(range.start) + CMTimeGetSeconds(range.duration);
            progress /= CMTimeGetSeconds(playerItem.duration);
            callbacks.onBufferProgress(progress);
        }
    }
}

- (void)playerDidFinishPlaying:(NSNotification*)notification {
    std::lock_guard<std::mutex> lock(callbackMutex);
    if (callbacks.onStateChanged) {
        callbacks.onStateChanged(playback::PlaybackState::ENDED);
    }
}

- (void)setupPlayerWithData:(NSData*)data {
    // Create temporary file from data
    NSString* tempDir = NSTemporaryDirectory();
    NSString* tempFile = [tempDir stringByAppendingPathComponent:
        [NSString stringWithFormat:@"playback_%lu.tmp", (unsigned long)[data hash]]];

    if (![data writeToFile:tempFile atomically:YES]) {
        return;
    }

    NSURL* url = [NSURL fileURLWithPath:tempFile];
    [self setupPlayerWithURL:url];
}

- (void)cleanup {
    if (timeObserver) {
        [player removeTimeObserver:timeObserver];
        timeObserver = nil;
    }

    if (playerItem) {
        [playerItem removeObserver:self forKeyPath:@"status"];
        [playerItem removeObserver:self forKeyPath:@"loadedTimeRanges"];
    }

    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
