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

option(BUILD_SHARED_LIBS "Build shared library instead of static" ON)
option(BUILD_TESTS "Build test executables" ON)
option(BUILD_EXAMPLES "Build example executables" OFF)
option(ENABLE_FFMPEG_BACKEND "Enable FFmpeg backend support" ON)
option(ENABLE_GSTREAMER_BACKEND "Enable GStreamer backend support" OFF)

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

# Set library properties
set_target_properties(playback PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include"
)


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
