# CLion Setup Guide

This guide will help you set up the project in CLion to easily run the `apple_audio_example` from the play button.

## Quick Setup

### 1. Open Project in CLion

1. Open CLion
2. Select **File → Open**
3. Navigate to the project directory (`/Users/loureiro/projects/yaspb`)
4. Click **Open**

### 2. Configure CMake

CLion will automatically detect the `CMakeLists.txt` file. The project includes `CMakePresets.json` which makes configuration easier.

**Important**: If you see a generator mismatch error (Ninja vs Unix Makefiles), clean the build directory first:
- Delete the `cmake-build-debug` folder, or
- In CLion: **File → Invalidate Caches... → Invalidate and Restart**

#### Option A: Use CMake Presets (Recommended)

1. In CLion, go to **File → Settings** (or **CLion → Preferences** on macOS)
2. Navigate to **Build, Execution, Deployment → CMake**
3. You should see the presets from `CMakePresets.json`:
   - **default** - Debug build with examples enabled (uses Ninja)
   - **debug** - Debug build (uses Ninja)
   - **release** - Release build (uses Ninja)

4. Select the **default** or **debug** preset (both have `BUILD_EXAMPLES=ON`)
5. Make sure the **Generator** is set to **Ninja** (CLion's default and recommended)

#### Option B: Manual CMake Configuration

1. In CLion, go to **File → Settings** (or **CLion → Preferences** on macOS)
2. Navigate to **Build, Execution, Deployment → CMake**
3. Click the **+** button to add a new configuration
4. Set:
   - **Name**: Debug
   - **Build type**: Debug
   - **Generator**: Ninja (recommended) or Unix Makefiles
   - **CMake options**: `-DBUILD_EXAMPLES=ON -DENABLE_AVFOUNDATION_BACKEND=ON`
   - **Build directory**: `cmake-build-debug`

### 3. Reload CMake Project

After configuration:
1. Click **File → Reload CMake Project** (or use the CMake reload button in the toolbar)
2. Wait for CMake to configure and generate the build files
3. You should see `apple_audio_example` in the **CMake** tool window under **Executables**

### 4. Set Up Run Configuration

CLion should automatically create a run configuration for `apple_audio_example`. To set it up:

1. Click the **Run/Debug Configurations** dropdown (top toolbar, next to the play button)
2. Select **Edit Configurations...**
3. Find **apple_audio_example** in the list
4. In the **Program arguments** field, add the path to an audio file:
   ```
   /path/to/your/audio/file.mp3
   ```

   Example using a macOS system sound:
   ```
   /System/Library/Sounds/Glass.aiff
   ```

   Or use a file from your Music library:
   ```
   ~/Music/your-song.mp3
   ```

5. Click **OK**

### 5. Run the Example

1. Make sure **apple_audio_example** is selected in the run configuration dropdown
2. Click the **Run** button (green play button) or press `Shift+F10`
3. The example will build (if needed) and run

## Troubleshooting

### Example Not Showing in Run Configurations

If `apple_audio_example` doesn't appear:
1. Make sure `BUILD_EXAMPLES=ON` is set in your CMake configuration
2. Reload CMake project: **File → Reload CMake Project**
3. Check the CMake tool window to see if there are any errors

### Build Errors

If you get build errors:
1. Check that `ENABLE_AVFOUNDATION_BACKEND=ON` is set
2. Make sure you're on macOS (Apple backend only works on Apple platforms)
3. Check the CMake output for detailed error messages

### Runtime Errors

If the program runs but fails:
1. Make sure the audio file path in program arguments is correct and the file exists
2. Check that the file format is supported (MP3, AAC, M4A, MP4)
3. Check the console output for error messages

## Quick Test

To quickly test if everything is set up correctly:

1. Set program arguments to: `/System/Library/Sounds/Glass.aiff`
2. Run the example
3. You should hear the sound play and see playback information in the console

## Additional Tips

- **Multiple Run Configurations**: You can create multiple run configurations with different audio files
- **Working Directory**: The working directory is automatically set to the project root
- **Environment Variables**: If needed, you can add environment variables in the run configuration
- **Debugging**: You can set breakpoints and debug the example just like any other program

## CMake Presets

The project includes `CMakePresets.json` with three presets:
- **default**: Debug build with examples and tests enabled
- **debug**: Same as default
- **release**: Release build with examples enabled, tests disabled

You can switch between presets in CLion's CMake settings.

