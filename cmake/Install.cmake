#=============================================================================
# Installation Configuration
#=============================================================================
# This module handles the installation rules for the project.
#=============================================================================

function(configure_installation)
    # Install headers
    install(
        DIRECTORY include/
        DESTINATION include
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.h"
    )

    # Install core library
    install(
        TARGETS playback
        EXPORT StreamingPlaybackTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )

    # Install backends if they exist
    if(ENABLE_MOCK_BACKEND AND TARGET mock_backend)
        install(
            TARGETS mock_backend
            EXPORT StreamingPlaybackTargets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
        )
    endif()

    if(ENABLE_FFMPEG_BACKEND AND TARGET ffmpeg_backend)
        install(
            TARGETS ffmpeg_backend
            EXPORT StreamingPlaybackTargets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
        )
    endif()

    if(ENABLE_GSTREAMER_BACKEND AND TARGET gstreamer_backend)
        install(
            TARGETS gstreamer_backend
            EXPORT StreamingPlaybackTargets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
        )
    endif()

    if(ENABLE_AVFOUNDATION_BACKEND AND PLATFORM_APPLE AND TARGET apple_backend)
        install(
            TARGETS apple_backend
            EXPORT StreamingPlaybackTargets
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
        )
    endif()

    # Install CMake package configuration
    install(
        EXPORT StreamingPlaybackTargets
        FILE StreamingPlaybackTargets.cmake
        NAMESPACE StreamingPlayback::
        DESTINATION lib/cmake/StreamingPlayback
    )

    # Create and install package config file
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/StreamingPlaybackConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/StreamingPlaybackConfigVersion.cmake"
        DESTINATION lib/cmake/StreamingPlayback
    )
endfunction()

