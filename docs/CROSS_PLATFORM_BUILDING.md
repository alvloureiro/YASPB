# Cross-Platform Building Guide

This guide explains how to build StreamingPlayback for different platforms, including cross-compilation from macOS to Linux, Windows, and Raspberry Pi.

## Supported Platforms

- **macOS** (native and cross-compilation target)
- **Linux** (native and cross-compilation target)
- **Windows** (cross-compilation from Linux/macOS)
- **Raspberry Pi** (cross-compilation from Linux/macOS)

## Native Building

### macOS

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Linux

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Windows (Native)

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Cross-Compilation

### Building for Raspberry Pi

#### Prerequisites

1. **Install Raspberry Pi Toolchain**:
   ```bash
   # Download from: https://github.com/raspberrypi/tools
   # Or use a package manager
   
   # macOS (using Homebrew)
   brew install arm-linux-gnueabihf-binutils
   
   # Linux (Ubuntu/Debian)
   sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
   ```

2. **Set up Sysroot** (optional but recommended):
   ```bash
   # You can extract a sysroot from a Raspberry Pi SD card
   # Or download a pre-built sysroot
   ```

#### Using the Build Script

```bash
# Build for Raspberry Pi 4 (64-bit)
./scripts/build-raspberry-pi.sh

# Or with custom settings
RASPBERRY_PI_VERSION=4 RPI_64BIT=ON ./scripts/build-raspberry-pi.sh

# Build for Raspberry Pi 3 (32-bit)
RASPBERRY_PI_VERSION=3 RPI_64BIT=OFF ./scripts/build-raspberry-pi.sh
```

#### Manual Build

```bash
mkdir build-rpi && cd build-rpi

cmake \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/raspberry-pi.cmake \
    -DRASPBERRY_PI_VERSION=4 \
    -DRPI_64BIT=ON \
    -DRPI_TOOLCHAIN_PATH=/opt/rpi-toolchain \
    -DRPI_SYSROOT=/opt/rpi-sysroot \
    ..

cmake --build .
```

#### Raspberry Pi Versions

- **Raspberry Pi 2/3**: Use `RASPBERRY_PI_VERSION=3` (32-bit ARM)
- **Raspberry Pi 4/5**: Use `RASPBERRY_PI_VERSION=4` or `5` (64-bit recommended)

### Building for Windows (from Linux/macOS)

#### Prerequisites

1. **Install MinGW-w64**:
   ```bash
   # macOS
   brew install mingw-w64
   
   # Linux (Ubuntu/Debian)
   sudo apt-get install mingw-w64
   ```

#### Using the Build Script

```bash
./scripts/build-windows.sh
```

#### Manual Build

```bash
mkdir build-windows && cd build-windows

cmake \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/windows-cross.cmake \
    -DMINGW_PATH=/usr/x86_64-w64-mingw32 \
    ..

cmake --build .
```

### Building for Linux (from macOS)

For cross-compiling to Linux from macOS, you can use Docker or a Linux VM. Alternatively, use a Linux build environment.

## Platform-Specific Options

### Linux Options

```bash
cmake \
    -DENABLE_HARDWARE_ACCELERATION=ON \
    -DUSE_SYSTEM_FFMPEG=ON \
    ..
```

### Windows Options

```bash
cmake \
    -DENABLE_DXVA2=ON \
    -DSTATIC_RUNTIME=ON \
    ..
```

### macOS Options

```bash
cmake \
    -DENABLE_VIDEOTOOLBOX=ON \
    -DENABLE_METAL=OFF \
    ..
```

## Toolchain Files

Toolchain files are located in the `toolchains/` directory:

- `raspberry-pi.cmake`: Raspberry Pi cross-compilation
- `windows-cross.cmake`: Windows cross-compilation

### Custom Toolchain Paths

You can override default toolchain paths using environment variables:

```bash
# Raspberry Pi
export RPI_TOOLCHAIN_PATH=/path/to/toolchain
export RPI_SYSROOT=/path/to/sysroot

# Windows
export MINGW_PATH=/path/to/mingw
```

## Troubleshooting

### Raspberry Pi Build Issues

1. **Toolchain not found**:
   - Verify the toolchain path is correct
   - Check that the compiler binaries exist and are executable
   - Ensure the toolchain matches your Raspberry Pi architecture

2. **Missing dependencies**:
   - Cross-compiled dependencies (FFmpeg, etc.) must be available in the sysroot
   - Consider using a package manager or building dependencies separately

3. **Linker errors**:
   - Ensure sysroot contains required libraries
   - Check rpath settings in the toolchain file

### Windows Build Issues

1. **MinGW not found**:
   - Install MinGW-w64 using your package manager
   - Set `MINGW_PATH` environment variable to the installation path

2. **Missing Windows libraries**:
   - Some libraries may need to be cross-compiled separately
   - Consider using vcpkg or Conan for dependency management

### General Issues

1. **CMake cache issues**:
   ```bash
   rm -rf build
   # Reconfigure from scratch
   ```

2. **Compiler detection**:
   - Verify compiler paths in toolchain files
   - Check that compilers are in PATH or specified correctly

## Testing Cross-Compiled Binaries

### Raspberry Pi

1. Copy binaries to Raspberry Pi:
   ```bash
   scp -r build-rpi/bin/* pi@raspberrypi:/home/pi/
   scp -r build-rpi/lib/* pi@raspberrypi:/home/pi/
   ```

2. Test on device:
   ```bash
   ssh pi@raspberrypi
   cd /home/pi
   ./test_executable
   ```

### Windows

1. Copy binaries to Windows machine
2. Ensure all DLL dependencies are available
3. Run executables on Windows

## Continuous Integration

For CI/CD pipelines, you can use the build scripts or CMake commands directly:

```yaml
# Example GitHub Actions
- name: Build for Raspberry Pi
  run: |
    ./scripts/build-raspberry-pi.sh

- name: Build for Windows
  run: |
    ./scripts/build-windows.sh
```

## Additional Resources

- [CMake Cross Compiling](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling)
- [Raspberry Pi Toolchain](https://github.com/raspberrypi/tools)
- [MinGW-w64](https://www.mingw-w64.org/)

