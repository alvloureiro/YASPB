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

# Build options - defaults depend on whether only Mock backend is enabled
option(BUILD_SHARED_LIBS "Build shared library instead of static" ON)

# Backend options - defaults to Mock backend only
option(ENABLE_MOCK_BACKEND "Enable Mock backend (no dependencies, default)" ON)
option(ENABLE_FFMPEG_BACKEND "Enable FFmpeg backend support" OFF)
option(ENABLE_GSTREAMER_BACKEND "Enable GStreamer backend support" OFF)
if(PLATFORM_APPLE)
    option(ENABLE_APPLE_BACKEND "Enable Apple backend support (Apple platforms only)" OFF)
else ()
    # On non-Apple platforms, Apple backend is always disabled
    set(ENABLE_APPLE_BACKEND OFF CACHE BOOL "Enable Apple backend support (Apple platforms only)" FORCE)
endif ()

# Auto-disable Mock backend if any other backend is explicitly enabled
# This allows users to just pass -DENABLE_FFMPEG_BACKEND=ON without needing to disable Mock
set(NON_MOCK_BACKEND_ENABLED FALSE)
if (ENABLE_FFMPEG_BACKEND OR ENABLE_APPLE_BACKEND OR ENABLE_GSTREAMER_BACKEND)
    set(NON_MOCK_BACKEND_ENABLED TRUE)
    if (ENABLE_MOCK_BACKEND)
        message(STATUS "Auto-disabling Mock backend (another backend is enabled)")
        set(ENABLE_MOCK_BACKEND OFF CACHE BOOL "Enable Mock backend" FORCE)
    endif ()
endif()

# Set defaults for tests, examples, and coverage based on backend selection
# Default: Mock backend only -> tests and coverage ON, examples OFF
# Non-Mock backend enabled -> tests, examples, coverage OFF (must be explicit)
if (ENABLE_MOCK_BACKEND AND NOT NON_MOCK_BACKEND_ENABLED)
    # Default behavior: Mock backend + tests + coverage
    option(BUILD_TESTS "Build test executables" ON)
    option(ENABLE_COVERAGE "Enable code coverage (requires gcov and lcov)" ON)
    option(BUILD_EXAMPLES "Build example executables" OFF)
else ()
    # Non-Mock backend: tests/examples/coverage must be explicit
    option(BUILD_TESTS "Build test executables" OFF)
    option(ENABLE_COVERAGE "Enable code coverage (requires gcov and lcov)" OFF)
    option(BUILD_EXAMPLES "Build example executables" OFF)
endif ()

# Options to control which examples to build (useful for selective builds)
option(BUILD_APPLE_AUDIO_EXAMPLE "Build apple_audio_example" OFF)
option(BUILD_FFMPEG_AUDIO_EXAMPLE "Build ffmpeg_audio_example" OFF)

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

if (ENABLE_APPLE_BACKEND AND PLATFORM_APPLE)
    add_definitions(-DENABLE_AVFOUNDATION_BACKEND)  # Keep for source code compatibility
    add_definitions(-DENABLE_APPLE_BACKEND)
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

# Code coverage configuration (include early so functions are available)
if (ENABLE_COVERAGE)
    include(cmake/Coverage.cmake)
endif ()

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

if (ENABLE_APPLE_BACKEND AND PLATFORM_APPLE AND TARGET apple_backend)
    target_link_libraries(playback PRIVATE apple_backend)
endif()

if(BUILD_TESTS)
    enable_testing()
    include(cmake/Testing.cmake)
    add_tests()
endif()

# Enable coverage for core playback library and add per-backend coverage targets
# Coverage is explicit - user must enable it
if (ENABLE_COVERAGE)
    # Enable coverage for core playback library
    enable_coverage_for_target(playback)

    # Add per-backend coverage targets (only for enabled backends)
    if (ENABLE_MOCK_BACKEND)
        add_backend_coverage(mock "${CMAKE_SOURCE_DIR}/src/backends/mock")
    endif ()

    if (ENABLE_FFMPEG_BACKEND)
        add_backend_coverage(ffmpeg "${CMAKE_SOURCE_DIR}/src/backends/ffmpeg")
    endif ()

    if (ENABLE_APPLE_BACKEND AND PLATFORM_APPLE)
        add_backend_coverage(apple "${CMAKE_SOURCE_DIR}/src/backends/apple")
    endif ()
else ()
    message(STATUS "Code coverage: DISABLED (enable with -DENABLE_COVERAGE=ON)")
endif ()

# Include Examples.cmake to check if examples should be auto-enabled
# (e.g., if BUILD_FFMPEG_AUDIO_EXAMPLE=ON, BUILD_EXAMPLES will be auto-enabled)
include(cmake/Examples.cmake)
check_and_enable_examples()

if (BUILD_EXAMPLES)
    add_examples()
endif()

include(cmake/Install.cmake)
configure_installation()

#=============================================================================
# Build Configuration Summary
#=============================================================================
message(STATUS "")
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS "Backends:")
if (ENABLE_MOCK_BACKEND)
    message(STATUS "  ✓ Mock backend: ENABLED")
else ()
    message(STATUS "  ✗ Mock backend: DISABLED")
endif ()
if (ENABLE_FFMPEG_BACKEND)
    message(STATUS "  ✓ FFmpeg backend: ENABLED")
else ()
    message(STATUS "  ✗ FFmpeg backend: DISABLED")
endif ()
if (ENABLE_APPLE_BACKEND AND PLATFORM_APPLE)
    message(STATUS "  ✓ Apple backend: ENABLED")
elseif (PLATFORM_APPLE)
    message(STATUS "  ✗ Apple backend: DISABLED")
else ()
    message(STATUS "  ✗ Apple backend: N/A (not on Apple platform)")
endif ()
if (ENABLE_GSTREAMER_BACKEND)
    message(STATUS "  ✓ GStreamer backend: ENABLED")
else ()
    message(STATUS "  ✗ GStreamer backend: DISABLED")
endif ()
message(STATUS "")
message(STATUS "Build Components:")
if (BUILD_TESTS)
    message(STATUS "  ✓ Tests: ENABLED")
else ()
    message(STATUS "  ✗ Tests: DISABLED")
endif ()
if (BUILD_EXAMPLES)
    message(STATUS "  ✓ Examples: ENABLED")
    if (BUILD_APPLE_AUDIO_EXAMPLE)
        message(STATUS "    - apple_audio_example: ENABLED")
    endif ()
    if (BUILD_FFMPEG_AUDIO_EXAMPLE)
        message(STATUS "    - ffmpeg_audio_example: ENABLED")
    endif ()
else ()
    message(STATUS "  ✗ Examples: DISABLED")
endif ()
if (ENABLE_COVERAGE)
    message(STATUS "  ✓ Code Coverage: ENABLED")
else ()
    message(STATUS "  ✗ Code Coverage: DISABLED")
endif ()
message(STATUS "")
message(STATUS "Build Behavior:")
message(STATUS "  Default (cmake ..): Mock backend + tests + coverage")
message(STATUS "  Enable other backend: Mock auto-disabled, only specified backend built")
message(STATUS "")
message(STATUS "Examples:")
message(STATUS "  Default: cmake ..")
message(STATUS "  FFmpeg only: cmake .. -DENABLE_FFMPEG_BACKEND=ON")
message(STATUS "  FFmpeg + tests: cmake .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_TESTS=ON")
message(STATUS "  FFmpeg + example: cmake .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_EXAMPLES=ON -DBUILD_FFMPEG_AUDIO_EXAMPLE=ON")
message(STATUS "")
