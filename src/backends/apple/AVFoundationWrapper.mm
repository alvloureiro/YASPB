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
#include <iostream>
#include <iomanip>

// Classe Objective-C interna
@interface AVFPlayerInternal : NSObject {
    @public
    AVPlayer* player;
    AVPlayerItem* playerItem;
    id timeObserver;
    playback::apple::AVFCallbacks callbacks;
    std::mutex callbackMutex;
    NSString* tempFilePath;
    dispatch_source_t statusPollTimer;
    std::shared_ptr<int> lifetimeTracker;
}

- (void)setupPlayerWithURL:(NSURL*)url;
- (void)setupPlayerWithData:(NSData*)data;
- (void)setupObservers;
- (void)cleanup;

@end

namespace playback::apple {

// Implementação PIMPL em Objective-C++
class AVFPlayerImpl {
public:
    AVFPlayerImpl() : is_stopped(false), internal_([[AVFPlayerInternal alloc] init]) {}
    ~AVFPlayerImpl() {
        if (internal_) {
            [internal_ cleanup];
            internal_ = nil;
        }
    }

    AVFPlayerInternal* internal() { return internal_; }

    bool is_stopped;

private:
    AVFPlayerInternal* __strong internal_;
};

// Implementação dos métodos wrapper
AVFPlayerWrapper::AVFPlayerWrapper() : impl_(std::make_unique<AVFPlayerImpl>()) {}

AVFPlayerWrapper::~AVFPlayerWrapper() = default;

std::weak_ptr<void> AVFPlayerWrapper::getLifetimeTracker() const {
    return impl_->internal()->lifetimeTracker;
}

bool AVFPlayerWrapper::load(const std::string& url) {
    @autoreleasepool {
        impl_->is_stopped = false;
        std::cout << "[AVF] load() called with URL: " << url << std::endl;

        NSString* nsUrl = [NSString stringWithUTF8String:url.c_str()];
        NSURL* assetUrl = nil;

        if ([nsUrl rangeOfString:@"://"].location == NSNotFound) {
            NSString* expandedPath = [nsUrl stringByExpandingTildeInPath];
            NSString* absolutePath = [expandedPath stringByStandardizingPath];

            BOOL isDirectory = NO;
            BOOL fileExists = [[NSFileManager defaultManager] fileExistsAtPath:absolutePath isDirectory:&isDirectory];
            if (!fileExists || isDirectory) {
                std::cerr << "[AVF] ERROR: Invalid file path: " << [absolutePath UTF8String] << std::endl;
                return false;
            }
            assetUrl = [NSURL fileURLWithPath:absolutePath];
        } else {
            assetUrl = [NSURL URLWithString:nsUrl];
        }

        if (!assetUrl) return false;

        [impl_->internal() setupPlayerWithURL:assetUrl];
        return true;
    }
}

bool AVFPlayerWrapper::play() {
    @autoreleasepool {
        impl_->is_stopped = false;
        AVFPlayerInternal* internal = impl_->internal();
        if (!internal->player || !internal->playerItem) return false;

        AVPlayerItemStatus status = internal->playerItem.status;
        if (status == AVPlayerItemStatusReadyToPlay) {
            [internal->player play];
            return true;
        }
        return false;
    }
}

bool AVFPlayerWrapper::pause() {
    @autoreleasepool {
        impl_->is_stopped = false;
        if (!impl_->internal()->player) return true;
        [impl_->internal()->player pause];
        return true;
    }
}

bool AVFPlayerWrapper::stop() {
    @autoreleasepool {
        impl_->is_stopped = true;
        if (!impl_->internal()->player) return true;
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
        AVFPlayerInternal* internal = impl_->internal();
        if (!internal->playerItem) return PlaybackState::IDLE;

        AVPlayerItemStatus status = internal->playerItem.status;
        if (status == AVPlayerItemStatusFailed) return PlaybackState::ERROR;
        if (impl_->is_stopped) return PlaybackState::STOPPED;
        if (status != AVPlayerItemStatusReadyToPlay) return PlaybackState::BUFFERING;

        if (internal->player.rate == 0.0) {
            CMTime currentTime = internal->player.currentTime;
            CMTime duration = internal->playerItem.duration;
            if (CMTIME_IS_VALID(duration) && CMTIME_IS_VALID(currentTime)) {
                if (CMTIME_COMPARE_INLINE(currentTime, >=, duration)) return PlaybackState::ENDED;
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
        return CMTIME_IS_VALID(currentTime) ? static_cast<uint64_t>(CMTimeGetSeconds(currentTime) * 1000.0) : 0;
    }
}

uint64_t AVFPlayerWrapper::getDuration() const {
    @autoreleasepool {
        if (!impl_->internal()->playerItem) return 0;
        CMTime duration = impl_->internal()->playerItem.duration;
        return (CMTIME_IS_VALID(duration) && !CMTIME_IS_INDEFINITE(duration)) ?
               static_cast<uint64_t>(CMTimeGetSeconds(duration) * 1000.0) : 0;
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
                "yuv420p", std::nullopt
            };
        }

        if (audioTracks.count > 0) {
            format.type = (format.type == MediaType::VIDEO) ? MediaType::VIDEO : MediaType::AUDIO;
            AVAssetTrack* track = audioTracks[0];
            format.audio = AudioFormat{
                static_cast<uint32_t>(track.naturalTimeScale),
                2, static_cast<uint32_t>(track.estimatedDataRate), "stereo"
            };
        }
    }
    return format;
}

std::vector<MediaFormat> AVFPlayerWrapper::getAvailableFormats() const {
    std::vector<MediaFormat> formats;
    MediaFormat format = getCurrentFormat();
    if (format.video.has_value() || format.audio.has_value()) formats.push_back(format);
    return formats;
}

bool AVFPlayerWrapper::loadFromData(const std::vector<uint8_t>& data) {
    @autoreleasepool {
        NSData* nsData = [NSData dataWithBytes:data.data() length:data.size()];
        if (!nsData) return false;
        [impl_->internal() setupPlayerWithData:nsData];
        return true;
    }
}

void AVFPlayerWrapper::setAudioSessionCategory(const std::string& category) {
#if TARGET_OS_IOS
    @autoreleasepool {
        [[AVAudioSession sharedInstance] setCategory:[NSString stringWithUTF8String:category.c_str()] error:nil];
    }
#endif
}

void AVFPlayerWrapper::setAudioSessionMode(const std::string& mode) {
#if TARGET_OS_IOS
    @autoreleasepool {
        [[AVAudioSession sharedInstance] setMode:[NSString stringWithUTF8String:mode.c_str()] error:nil];
    }
#endif
}

bool AVFPlayerWrapper::supportsHDR() const { return true; }
bool AVFPlayerWrapper::supportsDolbyVision() const { return true; }
bool AVFPlayerWrapper::hasDRM() const { return false; }
bool AVFPlayerWrapper::setDRMLicense(const std::string& license) { return false; }

void AVFPlayerWrapper::setCallbacks(const AVFCallbacks& callbacks) {
    std::lock_guard<std::mutex> lock(impl_->internal()->callbackMutex);
    impl_->internal()->callbacks = callbacks;
}

void* AVFPlayerWrapper::getMetalLayer() { return nullptr; }
void AVFPlayerWrapper::setOutputView(void* view) {}

} // namespace playback::apple

extern "C" void processRunLoop(double seconds) {
    @autoreleasepool {
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
    }
}

@implementation AVFPlayerInternal

- (instancetype)init {
    self = [super init];
    if (self) {
        lifetimeTracker = std::make_shared<int>(1);
    }
    return self;
}

- (void)dealloc {
    [self cleanup];
}

- (void)setupPlayerWithURL:(NSURL*)url {
    std::cout << "[AVF] setupPlayerWithURL: " << [[url absoluteString] UTF8String] << std::endl;
    AVAsset* asset = [AVAsset assetWithURL:url];
    if (!asset) return;

    playerItem = [AVPlayerItem playerItemWithAsset:asset];
    if (!playerItem) return;

    player = [AVPlayer playerWithPlayerItem:playerItem];
    if (!player) return;

    player.volume = 1.0;
    [self setupObservers];
}

- (void)setupObservers {
    [playerItem addObserver:self forKeyPath:@"status" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial context:nil];
    [playerItem addObserver:self forKeyPath:@"loadedTimeRanges" options:NSKeyValueObservingOptionNew context:nil];
    [playerItem addObserver:self forKeyPath:@"error" options:NSKeyValueObservingOptionNew context:nil];

    __weak AVFPlayerInternal* weakSelf = self;
    timeObserver = [player addPeriodicTimeObserverForInterval:CMTimeMake(1, 10) queue:dispatch_get_main_queue() usingBlock:^(CMTime time) {
        AVFPlayerInternal* strongSelf = weakSelf;
        if (strongSelf) {
            std::lock_guard<std::mutex> lock(strongSelf->callbackMutex);
            if (strongSelf->callbacks.onPositionChanged) {
                strongSelf->callbacks.onPositionChanged(CMTimeGetSeconds(time) * 1000);
            }
        }
    }];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playerDidFinishPlaying:) name:AVPlayerItemDidPlayToEndTimeNotification object:playerItem];

    dispatch_async(dispatch_get_main_queue(), ^{
        AVFPlayerInternal* strongSelf = weakSelf;
        if (!strongSelf) return;

        strongSelf->statusPollTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
        dispatch_source_set_timer(strongSelf->statusPollTimer, DISPATCH_TIME_NOW, 0.1 * NSEC_PER_SEC, 0.05 * NSEC_PER_SEC);

        dispatch_source_set_event_handler(strongSelf->statusPollTimer, ^{
            AVFPlayerInternal* timerSelf = weakSelf;
            if (!timerSelf) return;
            if (timerSelf->playerItem.status == AVPlayerItemStatusReadyToPlay || timerSelf->playerItem.status == AVPlayerItemStatusFailed) {
                if (timerSelf->statusPollTimer) {
                    dispatch_source_cancel(timerSelf->statusPollTimer);
                    timerSelf->statusPollTimer = nil;
                }
            }
        });
        dispatch_resume(strongSelf->statusPollTimer);
    });
}

- (void)observeValueForKeyPath:(NSString*)keyPath ofObject:(id)object change:(NSDictionary*)change context:(void*)context {
    std::lock_guard<std::mutex> lock(callbackMutex);
    if ([keyPath isEqualToString:@"status"]) {
        AVPlayerItemStatus status = (AVPlayerItemStatus)[change[NSKeyValueChangeNewKey] integerValue];
        if (callbacks.onStateChanged) {
            if (status == AVPlayerItemStatusReadyToPlay) callbacks.onStateChanged(playback::PlaybackState::PAUSED); // Ready to play
            else if (status == AVPlayerItemStatusFailed) callbacks.onStateChanged(playback::PlaybackState::ERROR);
        }
    } else if ([keyPath isEqualToString:@"loadedTimeRanges"]) {
        if (callbacks.onBufferProgress && playerItem.loadedTimeRanges.count > 0) {
            CMTimeRange range = [playerItem.loadedTimeRanges.firstObject CMTimeRangeValue];
            double total = CMTimeGetSeconds(playerItem.duration);
            if (total > 0) callbacks.onBufferProgress((CMTimeGetSeconds(range.start) + CMTimeGetSeconds(range.duration)) / total);
        }
    } else if ([keyPath isEqualToString:@"error"]) {
        if (playerItem.error && callbacks.onStateChanged) {
            callbacks.onStateChanged(playback::PlaybackState::ERROR);
        }
    }
}

- (void)playerDidFinishPlaying:(NSNotification*)notification {
    std::lock_guard<std::mutex> lock(callbackMutex);
    if (callbacks.onStateChanged) callbacks.onStateChanged(playback::PlaybackState::ENDED);
}

- (void)setupPlayerWithData:(NSData*)data {
    NSString* tempFile = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"playback_%lu.tmp", (unsigned long)[data hash]]];
    if ([data writeToFile:tempFile atomically:YES]) {
        tempFilePath = tempFile;
        [self setupPlayerWithURL:[NSURL fileURLWithPath:tempFile]];
    }
}

- (void)cleanup {
    { std::lock_guard<std::mutex> lock(callbackMutex); callbacks = {}; }
    if (statusPollTimer) { dispatch_source_cancel(statusPollTimer); statusPollTimer = nil; }
    if (player) {
        [player pause];
        if (timeObserver) { [player removeTimeObserver:timeObserver]; timeObserver = nil; }
        [player replaceCurrentItemWithPlayerItem:nil];
    }
    if (playerItem) {
        [playerItem removeObserver:self forKeyPath:@"status"];
        [playerItem removeObserver:self forKeyPath:@"loadedTimeRanges"];
        [playerItem removeObserver:self forKeyPath:@"error"];
    }
    if (tempFilePath) { [[NSFileManager defaultManager] removeItemAtPath:tempFilePath error:nil]; tempFilePath = nil; }
    player = nil; playerItem = nil;
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
