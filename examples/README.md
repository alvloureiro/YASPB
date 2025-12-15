# Examples

This directory contains example programs demonstrating how to use the StreamingPlayback library.

## Apple Audio Example

The `apple_audio_example` demonstrates how to use the Apple AVFoundation backend to play real audio files.

### Building the Example

To build the example, configure CMake with `BUILD_EXAMPLES=ON`:

```bash
cmake -DBUILD_EXAMPLES=ON -B build
cmake --build build
```

The example executable will be built in `build/bin/apple_audio_example`.

### Running the Example

```bash
./build/bin/apple_audio_example /path/to/audio/file.mp3
```

### Supported Audio Formats

The example supports common audio formats that AVFoundation can handle:
- MP3 (`.mp3`)
- AAC (`.aac`, `.m4a`)
- MP4 audio/video (`.mp4`)
- Other formats supported by AVFoundation

### Example Usage

```bash
# Play an MP3 file
./build/bin/apple_audio_example ~/Music/song.mp3

# Play an AAC file
./build/bin/apple_audio_example ~/Music/song.m4a

# Play an MP4 audio file
./build/bin/apple_audio_example ~/Music/song.mp4
```

### What the Example Demonstrates

1. Creating a `PlaybackEngine` using `PlaybackFactory`
2. Getting the Apple AVFoundation backend
3. Creating a `FileMediaSource` to represent the audio file
4. Creating a playback controller with the backend
5. Loading and playing the audio file
6. Handling playback events (state changes, position updates, etc.)
7. Displaying playback statistics

### FileMediaSource

The example includes a simple `FileMediaSource` implementation that wraps file paths. This is a basic implementation for demonstration purposes. In a production application, you would use a more robust media source implementation provided by the backend or create your own based on your needs.

