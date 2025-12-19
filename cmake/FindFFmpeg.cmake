#=============================================================================
# FindFFmpeg.cmake
#=============================================================================
# Find FFmpeg libraries and headers
# This module finds the following components:
#   - avcodec
#   - avformat
#   - avutil
#   - swscale
#   - swresample
#=============================================================================

include(FindPackageHandleStandardArgs)

# Try pkg-config first (common on Linux)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(FFMPEG_PKG QUIET libavcodec libavformat libavutil libswscale libswresample)
endif()

# Find libraries
find_library(AVCODEC_LIBRARY
    NAMES avcodec
    PATHS
        ${FFMPEG_PKG_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
    PATH_SUFFIXES
        lib
        lib64
)

find_library(AVFORMAT_LIBRARY
    NAMES avformat
    PATHS
        ${FFMPEG_PKG_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
    PATH_SUFFIXES
        lib
        lib64
)

find_library(AVUTIL_LIBRARY
    NAMES avutil
    PATHS
        ${FFMPEG_PKG_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
    PATH_SUFFIXES
        lib
        lib64
)

find_library(SWSCALE_LIBRARY
    NAMES swscale
    PATHS
        ${FFMPEG_PKG_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
    PATH_SUFFIXES
        lib
        lib64
)

find_library(SWRESAMPLE_LIBRARY
    NAMES swresample
    PATHS
        ${FFMPEG_PKG_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
    PATH_SUFFIXES
        lib
        lib64
)

# Find headers
find_path(AVCODEC_INCLUDE_DIR
    NAMES libavcodec/avcodec.h
    PATHS
        ${FFMPEG_PKG_INCLUDE_DIRS}
        /usr/include
        /usr/local/include
        /opt/local/include
    PATH_SUFFIXES
        include
)

find_path(AVFORMAT_INCLUDE_DIR
    NAMES libavformat/avformat.h
    PATHS
        ${FFMPEG_PKG_INCLUDE_DIRS}
        /usr/include
        /usr/local/include
        /opt/local/include
    PATH_SUFFIXES
        include
)

find_path(AVUTIL_INCLUDE_DIR
    NAMES libavutil/avutil.h
    PATHS
        ${FFMPEG_PKG_INCLUDE_DIRS}
        /usr/include
        /usr/local/include
        /opt/local/include
    PATH_SUFFIXES
        include
)

find_path(SWSCALE_INCLUDE_DIR
    NAMES libswscale/swscale.h
    PATHS
        ${FFMPEG_PKG_INCLUDE_DIRS}
        /usr/include
        /usr/local/include
        /opt/local/include
    PATH_SUFFIXES
        include
)

find_path(SWRESAMPLE_INCLUDE_DIR
    NAMES libswresample/swresample.h
    PATHS
        ${FFMPEG_PKG_INCLUDE_DIRS}
        /usr/include
        /usr/local/include
        /opt/local/include
    PATH_SUFFIXES
        include
)

# Set variables
if(AVCODEC_INCLUDE_DIR AND AVFORMAT_INCLUDE_DIR AND AVUTIL_INCLUDE_DIR)
    # All include directories should be the same (they're in the same parent directory)
    set(FFMPEG_INCLUDE_DIRS
        ${AVCODEC_INCLUDE_DIR}
        ${AVFORMAT_INCLUDE_DIR}
        ${AVUTIL_INCLUDE_DIR}
    )

    # Add optional components
    if(SWSCALE_INCLUDE_DIR)
        list(APPEND FFMPEG_INCLUDE_DIRS ${SWSCALE_INCLUDE_DIR})
    endif()

    if(SWRESAMPLE_INCLUDE_DIR)
        list(APPEND FFMPEG_INCLUDE_DIRS ${SWRESAMPLE_INCLUDE_DIR})
    endif()

    # Remove duplicates
    list(REMOVE_DUPLICATES FFMPEG_INCLUDE_DIRS)
endif()

set(FFMPEG_LIBRARIES)
if(AVCODEC_LIBRARY)
    list(APPEND FFMPEG_LIBRARIES ${AVCODEC_LIBRARY})
endif()
if(AVFORMAT_LIBRARY)
    list(APPEND FFMPEG_LIBRARIES ${AVFORMAT_LIBRARY})
endif()
if(AVUTIL_LIBRARY)
    list(APPEND FFMPEG_LIBRARIES ${AVUTIL_LIBRARY})
endif()
if(SWSCALE_LIBRARY)
    list(APPEND FFMPEG_LIBRARIES ${SWSCALE_LIBRARY})
endif()
if(SWRESAMPLE_LIBRARY)
    list(APPEND FFMPEG_LIBRARIES ${SWRESAMPLE_LIBRARY})
endif()

# Handle standard arguments
find_package_handle_standard_args(FFmpeg
    FOUND_VAR FFMPEG_FOUND
    REQUIRED_VARS
        AVCODEC_LIBRARY
        AVFORMAT_LIBRARY
        AVUTIL_LIBRARY
        FFMPEG_INCLUDE_DIRS
)

# Mark as advanced
mark_as_advanced(
    AVCODEC_LIBRARY
    AVFORMAT_LIBRARY
    AVUTIL_LIBRARY
    SWSCALE_LIBRARY
    SWRESAMPLE_LIBRARY
    AVCODEC_INCLUDE_DIR
    AVFORMAT_INCLUDE_DIR
    AVUTIL_INCLUDE_DIR
    SWSCALE_INCLUDE_DIR
    SWRESAMPLE_INCLUDE_DIR
)

