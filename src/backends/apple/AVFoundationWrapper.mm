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
    AVFPlayerImpl() : is_stopped(false), internal_([[AVFPlayerInternal alloc] init]) {}
    ~AVFPlayerImpl() { [internal_ cleanup]; }

    AVFPlayerInternal* internal() { return internal_; }

    bool is_stopped;

private:
    AVFPlayerInternal* __strong internal_;
};

// Implementação dos métodos wrapper
AVFPlayerWrapper::AVFPlayerWrapper() : impl_(std::make_unique<AVFPlayerImpl>()) {}

AVFPlayerWrapper::~AVFPlayerWrapper() = default;

bool AVFPlayerWrapper::load(const std::string& url) {
    @autoreleasepool {
        impl_->is_stopped = false;
        std::cout << "[AVF] load() called with URL: " << url << std::endl;

        NSString* nsUrl = [NSString stringWithUTF8String:url.c_str()];
        NSURL* assetUrl = nil;

        // Check if it's a file path (doesn't contain ://)
        if ([nsUrl rangeOfString:@"://"].location == NSNotFound) {
            // It's a file path, expand ~ to home directory and resolve to absolute path
            std::cout << "[AVF] Detected file path, expanding and converting to file URL" << std::endl;

            // Expand ~ to home directory
            NSString* expandedPath = [nsUrl stringByExpandingTildeInPath];

            // Resolve to absolute path (handles relative paths)
            NSString* absolutePath = [expandedPath stringByStandardizingPath];

            std::cout << "[AVF] Original path: " << [nsUrl UTF8String] << std::endl;
            std::cout << "[AVF] Expanded path: " << [expandedPath UTF8String] << std::endl;
            std::cout << "[AVF] Absolute path: " << [absolutePath UTF8String] << std::endl;

            // Check if file exists
            BOOL isDirectory = NO;
            BOOL fileExists = [[NSFileManager defaultManager] fileExistsAtPath:absolutePath isDirectory:&isDirectory];
            if (!fileExists) {
                std::cerr << "[AVF] ERROR: File does not exist at path: " << [absolutePath UTF8String] << std::endl;
                return false;
            }
            if (isDirectory) {
                std::cerr << "[AVF] ERROR: Path is a directory, not a file: " << [absolutePath UTF8String] << std::endl;
                return false;
            }

            assetUrl = [NSURL fileURLWithPath:absolutePath];
        } else {
            // It's a URL, use URLWithString
            std::cout << "[AVF] Detected URL string" << std::endl;
            assetUrl = [NSURL URLWithString:nsUrl];
        }

        if (!assetUrl) {
            std::cerr << "[AVF] ERROR: Failed to create NSURL from: " << url << std::endl;
            return false;
        }

        std::cout << "[AVF] Created NSURL: " << [[assetUrl absoluteString] UTF8String] << std::endl;
        [impl_->internal() setupPlayerWithURL:assetUrl];
        std::cout << "[AVF] Player setup completed" << std::endl;
        return true;
    }
}

bool AVFPlayerWrapper::play() {
    @autoreleasepool {
        impl_->is_stopped = false;
        if (!impl_->internal()->player) {
            std::cerr << "[AVF] ERROR: play() called but player is nil!" << std::endl;
            return false;
        }

        if (!impl_->internal()->playerItem) {
            std::cerr << "[AVF] ERROR: play() called but playerItem is nil!" << std::endl;
            return false;
        }

        AVPlayerItemStatus status = impl_->internal()->playerItem.status;
        std::cout << "[AVF] play() called - PlayerItem status: ";
        switch (status) {
            case AVPlayerItemStatusUnknown:
                std::cout << "Unknown";
                break;
            case AVPlayerItemStatusReadyToPlay:
                std::cout << "ReadyToPlay";
                break;
            case AVPlayerItemStatusFailed:
                std::cout << "Failed";
                if (impl_->internal()->playerItem.error) {
                    NSError* error = impl_->internal()->playerItem.error;
                    std::cerr << " - Error: " << [[error localizedDescription] UTF8String] << std::endl;
                }
                break;
        }
        std::cout << std::endl;

        double currentVolume = impl_->internal()->player.volume;
        std::cout << "[AVF] Current volume: " << currentVolume << std::endl;

        if (status == AVPlayerItemStatusReadyToPlay) {
            [impl_->internal()->player play];
            std::cout << "[AVF] Player.play() called successfully" << std::endl;
            std::cout << "[AVF] Player rate after play: " << impl_->internal()->player.rate << std::endl;
            return true;
        } else {
            std::cerr << "[AVF] WARNING: Cannot play - player item not ready (status: " << (int)status << ")" << std::endl;
            return false;
        }
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
        if (!impl_->internal()->playerItem) {
            return PlaybackState::IDLE;
        }

        AVPlayerItemStatus status = impl_->internal()->playerItem.status;

        // Log status for debugging
        static AVPlayerItemStatus lastStatus = AVPlayerItemStatusUnknown;
        if (status != lastStatus) {
            std::cout << "[AVF] getState() - PlayerItem status: ";
            switch (status) {
                case AVPlayerItemStatusUnknown:
                    std::cout << "Unknown";
                    break;
                case AVPlayerItemStatusReadyToPlay:
                    std::cout << "ReadyToPlay";
                    break;
                case AVPlayerItemStatusFailed:
                    std::cout << "Failed";
                    if (impl_->internal()->playerItem.error) {
                        NSError* error = impl_->internal()->playerItem.error;
                        std::cerr << " - Error: " << [[error localizedDescription] UTF8String] << std::endl;
                    }
                    break;
            }
            std::cout << std::endl;
            lastStatus = status;
        }

        if (status == AVPlayerItemStatusFailed) {
            return PlaybackState::ERROR;
        }

        if (impl_->is_stopped) {
            return PlaybackState::STOPPED;
        }

        if (status != AVPlayerItemStatusReadyToPlay) {
            return PlaybackState::BUFFERING;
        }

        double rate = impl_->internal()->player.rate;
        if (rate == 0.0) {
            CMTime currentTime = impl_->internal()->player.currentTime;
            CMTime duration = impl_->internal()->playerItem.duration;
            if (CMTIME_IS_VALID(duration) && CMTIME_IS_VALID(currentTime)) {
                if (CMTIME_COMPARE_INLINE(currentTime, >=, duration)) {
                    return PlaybackState::ENDED;
                }
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

// Helper function for C++ code to process the run loop
// This is needed for command-line apps on macOS
extern "C" void processRunLoop(double seconds) {
    @autoreleasepool {
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
    }
}


@implementation AVFPlayerInternal

- (void)setupPlayerWithURL:(NSURL*)url {
    std::cout << "[AVF] setupPlayerWithURL: " << [[url absoluteString] UTF8String] << std::endl;

    AVAsset* asset = [AVAsset assetWithURL:url];
    if (!asset) {
        std::cerr << "[AVF] ERROR: Failed to create AVAsset from URL" << std::endl;
        return;
    }
    std::cout << "[AVF] AVAsset created successfully" << std::endl;

    // Check if asset is playable before creating player item
    NSArray* keys = @[@"playable"];
    NSError* loadError = nil;
    AVKeyValueStatus playableStatus = [asset statusOfValueForKey:@"playable" error:&loadError];

    if (playableStatus == AVKeyValueStatusUnknown) {
        std::cout << "[AVF] Loading 'playable' property asynchronously..." << std::endl;
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        [asset loadValuesAsynchronouslyForKeys:keys completionHandler:^{
            NSError* error = nil;
            AVKeyValueStatus status = [asset statusOfValueForKey:@"playable" error:&error];
            if (status == AVKeyValueStatusLoaded) {
                std::cout << "[AVF] Asset is playable: " << (asset.playable ? "YES" : "NO") << std::endl;
                if (!asset.playable) {
                    std::cerr << "[AVF] ERROR: Asset is not playable!" << std::endl;
                }
            } else if (status == AVKeyValueStatusFailed) {
                std::cerr << "[AVF] ERROR: Failed to load playable property: "
                          << (error ? [[error localizedDescription] UTF8String] : "unknown") << std::endl;
            }
            dispatch_semaphore_signal(semaphore);
        }];
        dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC);
        if (dispatch_semaphore_wait(semaphore, timeout) != 0) {
            std::cerr << "[AVF] WARNING: Timeout waiting for asset to load" << std::endl;
        }
    } else if (playableStatus == AVKeyValueStatusLoaded) {
        std::cout << "[AVF] Asset playable: " << (asset.playable ? "YES" : "NO") << std::endl;
        if (!asset.playable) {
            std::cerr << "[AVF] ERROR: Asset is not playable!" << std::endl;
            return;
        }
    }

    // Create player item - AVFoundation will load it asynchronously
    playerItem = [AVPlayerItem playerItemWithAsset:asset];
    if (!playerItem) {
        std::cerr << "[AVF] ERROR: Failed to create AVPlayerItem" << std::endl;
        return;
    }
    std::cout << "[AVF] AVPlayerItem created successfully" << std::endl;

    // Create player immediately
    player = [AVPlayer playerWithPlayerItem:playerItem];
    if (!player) {
        std::cerr << "[AVF] ERROR: Failed to create AVPlayer" << std::endl;
        return;
    }
    std::cout << "[AVF] AVPlayer created successfully" << std::endl;

    // Set default volume to 1.0 (full volume)
    player.volume = 1.0;
    std::cout << "[AVF] Player volume set to: " << player.volume << std::endl;

    // Configurar audio session (iOS only - not available on macOS)
    #if TARGET_OS_IOS
    AVAudioSession* audioSession = [AVAudioSession sharedInstance];
    NSError* error = nil;
    [audioSession setCategory:AVAudioSessionCategoryPlayback error:&error];
    if (error) {
        std::cerr << "[AVF] WARNING: Failed to set audio session category: " << [[error localizedDescription] UTF8String] << std::endl;
    }
    [audioSession setActive:YES error:&error];
    if (error) {
        std::cerr << "[AVF] WARNING: Failed to activate audio session: " << [[error localizedDescription] UTF8String] << std::endl;
    }
    #else
    std::cout << "[AVF] Running on macOS - audio session not needed" << std::endl;
    #endif

    // Setup observers immediately
    [self setupObservers];
    std::cout << "[AVF] Observers setup completed" << std::endl;
}

- (void)setupObservers {
    std::cout << "[AVF] Setting up observers..." << std::endl;

    // Observer para mudanças de estado
    [playerItem addObserver:self
                 forKeyPath:@"status"
                    options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                    context:nil];
    std::cout << "[AVF] Added observer for 'status' key path" << std::endl;

    // Observer para buffer
    [playerItem addObserver:self
                 forKeyPath:@"loadedTimeRanges"
                    options:NSKeyValueObservingOptionNew
                    context:nil];
    std::cout << "[AVF] Added observer for 'loadedTimeRanges' key path" << std::endl;

    // Observer para errors
    [playerItem addObserver:self
                 forKeyPath:@"error"
                    options:NSKeyValueObservingOptionNew
                    context:nil];
    std::cout << "[AVF] Added observer for 'error' key path" << std::endl;

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
    std::cout << "[AVF] Added time observer" << std::endl;

    // Notifications
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(playerDidFinishPlaying:)
                                                 name:AVPlayerItemDidPlayToEndTimeNotification
                                               object:playerItem];
    std::cout << "[AVF] Added notification observer for playback end" << std::endl;

    // Force initial status check and set up periodic polling
    // This is needed because on macOS command-line apps, the run loop might not be processing events
    dispatch_async(dispatch_get_main_queue(), ^{
        // Check immediately
        AVPlayerItemStatus status = self->playerItem.status;
        std::cout << "[AVF] Initial status check (on main queue): ";
        switch (status) {
            case AVPlayerItemStatusUnknown:
                std::cout << "Unknown";
                break;
            case AVPlayerItemStatusReadyToPlay:
                std::cout << "ReadyToPlay";
                break;
            case AVPlayerItemStatusFailed:
                std::cout << "Failed";
                if (self->playerItem.error) {
                    NSError* error = self->playerItem.error;
                    std::cerr << " - Error: " << [[error localizedDescription] UTF8String] << std::endl;
                }
                break;
        }
        std::cout << std::endl;

        // Check for errors
        if (self->playerItem.error) {
            NSError* error = self->playerItem.error;
            std::cerr << "[AVF] PlayerItem has error: " << [[error localizedDescription] UTF8String] << std::endl;
            std::cerr << "[AVF] Error domain: " << [error.domain UTF8String] << ", code: " << error.code << std::endl;
        }

        // Try to trigger loading by accessing tracks
        AVAsset* asset = self->playerItem.asset;
        if (asset) {
            NSArray* tracks = [asset tracksWithMediaType:AVMediaTypeAudio];
            std::cout << "[AVF] Asset has " << tracks.count << " audio track(s)" << std::endl;
            if (tracks.count == 0) {
                std::cerr << "[AVF] WARNING: Asset has no audio tracks!" << std::endl;
            }
        }

        // Set up periodic status polling (fallback if KVO doesn't work)
        // This helps in command-line apps where the run loop might not process events properly
        __block int pollCount = 0;
        __block dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
        dispatch_source_set_timer(timer, DISPATCH_TIME_NOW, 0.1 * NSEC_PER_SEC, 0.05 * NSEC_PER_SEC);
        dispatch_source_set_event_handler(timer, ^{
            pollCount++;
            AVPlayerItemStatus currentStatus = self->playerItem.status;

            // Log every 10 polls (1 second) or when status changes
            static AVPlayerItemStatus lastPolledStatus = AVPlayerItemStatusUnknown;
            if (currentStatus != lastPolledStatus || pollCount % 10 == 0) {
                std::cout << "[AVF] Status poll #" << pollCount << ": ";
                switch (currentStatus) {
                    case AVPlayerItemStatusUnknown:
                        std::cout << "Unknown";
                        break;
                    case AVPlayerItemStatusReadyToPlay:
                        std::cout << "ReadyToPlay";
                        // Cancel timer when ready
                        dispatch_source_cancel(timer);
                        break;
                    case AVPlayerItemStatusFailed:
                        std::cout << "Failed";
                        if (self->playerItem.error) {
                            NSError* error = self->playerItem.error;
                            std::cerr << " - Error: " << [[error localizedDescription] UTF8String] << std::endl;
                        }
                        dispatch_source_cancel(timer);
                        break;
                }
                std::cout << std::endl;
                lastPolledStatus = currentStatus;
            }

            // Stop after 10 seconds (100 polls)
            if (pollCount >= 100) {
                std::cerr << "[AVF] Status polling timeout after 10 seconds" << std::endl;
                dispatch_source_cancel(timer);
            }
        });
        dispatch_resume(timer);
    });
}

- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context {
    std::cout << "[AVF] Observer fired for keyPath: " << [keyPath UTF8String] << std::endl;
    std::lock_guard<std::mutex> lock(callbackMutex);

    if ([keyPath isEqualToString:@"status"]) {
        AVPlayerItemStatus status = (AVPlayerItemStatus)[change[NSKeyValueChangeNewKey] integerValue];

        std::cout << "[AVF] PlayerItem status changed to: ";
        switch (status) {
            case AVPlayerItemStatusUnknown:
                std::cout << "Unknown";
                break;
            case AVPlayerItemStatusReadyToPlay:
                std::cout << "ReadyToPlay";
                if (playerItem.duration.value > 0) {
                    double durationSeconds = CMTimeGetSeconds(playerItem.duration);
                    std::cout << " - Duration: " << std::fixed << std::setprecision(2) << durationSeconds << "s";
                }
                break;
            case AVPlayerItemStatusFailed:
                std::cout << "Failed";
                if (playerItem.error) {
                    NSError* error = playerItem.error;
                    std::cerr << " - Error: " << [[error localizedDescription] UTF8String];
                    if (error.localizedFailureReason) {
                        std::cerr << " (" << [error.localizedFailureReason UTF8String] << ")";
                    }
                    if (error.localizedRecoverySuggestion) {
                        std::cerr << " - Suggestion: " << [error.localizedRecoverySuggestion UTF8String];
                    }
                    std::cerr << " - Error code: " << error.code << ", domain: " << [error.domain UTF8String] << std::endl;
                } else {
                    std::cerr << " (no error object available)" << std::endl;
                }
                break;
        }
        std::cout << std::endl;

        if (callbacks.onStateChanged) {
            switch (status) {
                case AVPlayerItemStatusReadyToPlay:
                    // Don't automatically set to PLAYING - let the user call play()
                    // Just notify that it's ready
                    std::cout << "[AVF] PlayerItem is ready to play - waiting for play() call" << std::endl;
                    break;
                case AVPlayerItemStatusFailed:
                    callbacks.onStateChanged(playback::PlaybackState::ERROR);
                    break;
                default:
                    break;
            }
        }
    }
    else if ([keyPath isEqualToString:@"error"]) {
        if (playerItem.error) {
            NSError* error = playerItem.error;
            std::cerr << "[AVF] PlayerItem error detected: " << [[error localizedDescription] UTF8String] << std::endl;
            std::cerr << "[AVF] Error domain: " << [error.domain UTF8String] << ", code: " << error.code << std::endl;
        }
    }
    else if ([keyPath isEqualToString:@"loadedTimeRanges"]) {
        if (callbacks.onBufferProgress && playerItem.loadedTimeRanges.count > 0) {
            CMTimeRange range = [playerItem.loadedTimeRanges.firstObject CMTimeRangeValue];
            double loadedSeconds = CMTimeGetSeconds(range.start) + CMTimeGetSeconds(range.duration);
            double totalSeconds = CMTimeGetSeconds(playerItem.duration);
            if (totalSeconds > 0) {
                double progress = loadedSeconds / totalSeconds;
                std::cout << "[AVF] Buffer progress: " << std::fixed << std::setprecision(1)
                          << (progress * 100.0) << "% (" << loadedSeconds << "s / " << totalSeconds << "s)" << std::endl;
                callbacks.onBufferProgress(progress);
            }
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
