#=============================================================================
# CMake Configuration for StreamingPlayback
#=============================================================================

cmake_minimum_required(VERSION 3.16)

# Add cmake modules directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include platform configuration (needed before project() to set PLATFORM_APPLE)
include(cmake/Platform.cmake)

# Determine languages needed
set(PROJECT_LANGUAGES CXX)
if(APPLE)
    list(APPEND PROJECT_LANGUAGES OBJCXX)
endif()

project(StreamingPlayback
    VERSION 1.0.0
    DESCRIPTION "Streaming Playback Library"
    LANGUAGES ${PROJECT_LANGUAGES}
)

# Build options
option(BUILD_SHARED_LIBS "Build shared library instead of static" ON)
option(BUILD_TESTS "Build test executables" ON)
# Enable examples by default when using CLion or when explicitly requested
# CLion sets CMAKE_GENERATOR to "Unix Makefiles" or "Ninja", so we check for that
# Also check for CLion-specific environment variables
if(CMAKE_GENERATOR MATCHES "Unix Makefiles|Ninja|Xcode|Visual Studio" OR DEFINED ENV{CLION_IDE} OR DEFINED ENV{JETBRAINS_IDE})
    option(BUILD_EXAMPLES "Build example executables" ON)
else()
    option(BUILD_EXAMPLES "Build example executables" OFF)
endif()
# Options to control which examples to build (useful for selective builds)
option(BUILD_APPLE_AUDIO_EXAMPLE "Build apple_audio_example" ON)
option(BUILD_FFMPEG_AUDIO_EXAMPLE "Build ffmpeg_audio_example" ON)
option(ENABLE_MOCK_BACKEND "Enable Mock backend (no dependencies, default)" ON)
option(ENABLE_FFMPEG_BACKEND "Enable FFmpeg backend support" OFF)
option(ENABLE_GSTREAMER_BACKEND "Enable GStreamer backend support" OFF)
if(PLATFORM_APPLE)
    option(ENABLE_AVFOUNDATION_BACKEND "Enable AVFoundation backend (Apple platforms only)" ON)
endif()

# Platform-specific build options
if(PLATFORM_LINUX)
    option(ENABLE_HARDWARE_ACCELERATION "Enable hardware acceleration (VAAPI, VDPAU)" ON)
    option(USE_SYSTEM_FFMPEG "Use system-installed FFmpeg instead of bundled" ON)
endif()

if(PLATFORM_WINDOWS)
    option(ENABLE_DXVA2 "Enable DirectX Video Acceleration" ON)
    option(STATIC_RUNTIME "Use static runtime libraries" OFF)
endif()

if(PLATFORM_APPLE)
    option(ENABLE_VIDEOTOOLBOX "Enable VideoToolbox hardware acceleration" ON)
    option(ENABLE_METAL "Enable Metal rendering" OFF)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define preprocessor macros for enabled backends (for conditional compilation)
if(ENABLE_MOCK_BACKEND)
    add_definitions(-DENABLE_MOCK_BACKEND)
endif()

if(ENABLE_AVFOUNDATION_BACKEND AND PLATFORM_APPLE)
    add_definitions(-DENABLE_AVFOUNDATION_BACKEND)
endif()

if(ENABLE_FFMPEG_BACKEND)
    add_definitions(-DENABLE_FFMPEG_BACKEND)
endif()

if(ENABLE_GSTREAMER_BACKEND)
    add_definitions(-DENABLE_GSTREAMER_BACKEND)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Export compile commands for clang-tidy and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Collect source files
file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_SOURCE_DIR}/src/core/*.cpp"
)

add_library(playback ${CORE_SOURCES})

target_include_directories(playback
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Configure platform-specific flags
configure_platform_flags(playback)
configure_platform_linker_flags(playback)

# Set library properties
set_target_properties(playback PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Print platform information
print_platform_info()


include(cmake/Backends.cmake)
configure_backends()

# Link backends to playback library if enabled (needed for PlaybackFactory)
# This is safe because backends don't link back to playback
if(ENABLE_MOCK_BACKEND AND TARGET mock_backend)
    target_link_libraries(playback PRIVATE mock_backend)
endif()

if(ENABLE_FFMPEG_BACKEND AND TARGET ffmpeg_backend)
    target_link_libraries(playback PRIVATE ffmpeg_backend)
endif()

if(ENABLE_AVFOUNDATION_BACKEND AND PLATFORM_APPLE AND TARGET apple_backend)
    target_link_libraries(playback PRIVATE apple_backend)
endif()

if(BUILD_TESTS)
    enable_testing()
    include(cmake/Testing.cmake)
    add_tests()
endif()

if(BUILD_EXAMPLES)
    include(cmake/Examples.cmake)
    add_examples()
endif()

include(cmake/Install.cmake)
configure_installation()
