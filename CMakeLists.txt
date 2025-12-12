#=============================================================================
# CMake Configuration for StreamingPlayback
#=============================================================================

cmake_minimum_required(VERSION 3.16)
project(StreamingPlayback 
    VERSION 1.0.0 
    DESCRIPTION "Streaming Playback Library"
    LANGUAGES CXX
)

# Add cmake modules directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include platform configuration
include(cmake/Platform.cmake)

# Build options
option(BUILD_SHARED_LIBS "Build shared library instead of static" ON)
option(BUILD_TESTS "Build test executables" ON)
option(BUILD_EXAMPLES "Build example executables" OFF)
option(ENABLE_FFMPEG_BACKEND "Enable FFmpeg backend support" ON)
option(ENABLE_GSTREAMER_BACKEND "Enable GStreamer backend support" OFF)

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
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include"
)

# Print platform information
print_platform_info()


include(cmake/Backends.cmake)
configure_backends()

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
