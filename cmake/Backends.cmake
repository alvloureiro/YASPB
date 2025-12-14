#=============================================================================
# Backend Configuration
#=============================================================================
# This module handles the configuration and building of playback backends.
# To add a new backend:
#   1. Add an option for it in the main CMakeLists.txt
#   2. Add a corresponding section in configure_backends() function below
#   3. Create the backend source file in src/backends/
#=============================================================================

function(configure_backends)
    # Mock Backend (no external dependencies, works on all platforms)
    if(ENABLE_MOCK_BACKEND)
        add_backend(
            NAME mock
            SOURCE src/backends/mock/MockBackend.cpp
        )
        message(STATUS "Mock backend enabled (no external dependencies)")
    endif()

    # FFmpeg Backend
    if(ENABLE_FFMPEG_BACKEND)
        find_package(FFmpeg REQUIRED)

        add_backend(
            NAME ffmpeg
            SOURCE src/backends/FFmpegBackend.cpp
            LINK_LIBRARIES ${FFMPEG_LIBRARIES}
            INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS}
        )
    endif()

    # GStreamer Backend
    if(ENABLE_GSTREAMER_BACKEND)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

        add_backend(
            NAME gstreamer
            SOURCE src/backends/GStreamerBackend.cpp
            LINK_LIBRARIES ${GSTREAMER_LIBRARIES}
            INCLUDE_DIRS ${GSTREAMER_INCLUDE_DIRS}
        )
    endif()

    # Apple Backend (Apple platforms only)
    # Configured in separate module for better organization
    include(cmake/AppleBackend.cmake)
    configure_apple_backend()

    # Warn if no backends are enabled
    set(ANY_BACKEND_ENABLED FALSE)
    if(ENABLE_MOCK_BACKEND)
        set(ANY_BACKEND_ENABLED TRUE)
    endif()
    if(ENABLE_FFMPEG_BACKEND)
        set(ANY_BACKEND_ENABLED TRUE)
    endif()
    if(ENABLE_GSTREAMER_BACKEND)
        set(ANY_BACKEND_ENABLED TRUE)
    endif()
    if(ENABLE_AVFOUNDATION_BACKEND AND PLATFORM_APPLE)
        set(ANY_BACKEND_ENABLED TRUE)
    endif()

    if(NOT ANY_BACKEND_ENABLED)
        message(WARNING "No backends enabled! Enable at least one backend (ENABLE_MOCK_BACKEND is recommended)")
    endif()
endfunction()

#=============================================================================
# Helper function to add a backend library
#=============================================================================
# Parameters:
#   NAME: Backend name (e.g., "ffmpeg", "gstreamer")
#   SOURCE: Source file path
#   LINK_LIBRARIES: Libraries to link against
#   INCLUDE_DIRS: Include directories
#=============================================================================
# Note: find_package() for dependencies should be called in configure_backends()
# before calling this function, as find_package() must be called in the parent scope.
#=============================================================================
function(add_backend)
    set(options "")
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCE LINK_LIBRARIES INCLUDE_DIRS)
    cmake_parse_arguments(BACKEND "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT BACKEND_NAME)
        message(FATAL_ERROR "add_backend: NAME is required")
    endif()

    if(NOT BACKEND_SOURCE)
        message(FATAL_ERROR "add_backend: SOURCE is required")
    endif()

    # Create backend library
    add_library(${BACKEND_NAME}_backend ${BACKEND_SOURCE})

    # Handle Objective-C++ files (.mm) for Apple backends
    # Need to check each source file individually since BACKEND_SOURCE is a list
    foreach(SOURCE_FILE ${BACKEND_SOURCE})
        if(SOURCE_FILE MATCHES "\\.mm$")
            # Set language to OBJCXX for .mm files
            set_source_files_properties(${SOURCE_FILE} PROPERTIES
                LANGUAGE OBJCXX
            )
        endif()
    endforeach()

    # Link to core playback library (except for mock and apple which don't need linking)
    # These backends only need headers, not the library itself
    # This avoids circular dependencies since playback links to these backends
    if(NOT BACKEND_NAME STREQUAL "mock" AND NOT BACKEND_NAME STREQUAL "apple")
        target_link_libraries(${BACKEND_NAME}_backend
            PUBLIC
                playback
        )
    else()
        # Mock and Apple backends only need include directories
        target_include_directories(${BACKEND_NAME}_backend
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        )
    endif()

    # Add external libraries if provided
    if(BACKEND_LINK_LIBRARIES)
        target_link_libraries(${BACKEND_NAME}_backend
            PRIVATE
                ${BACKEND_LINK_LIBRARIES}
        )
    endif()

    # Add include directories if provided
    if(BACKEND_INCLUDE_DIRS)
        target_include_directories(${BACKEND_NAME}_backend
            PUBLIC
                ${BACKEND_INCLUDE_DIRS}
        )
    endif()

    # Configure platform-specific flags
    configure_platform_flags(${BACKEND_NAME}_backend)
    configure_platform_linker_flags(${BACKEND_NAME}_backend)

    # Set backend properties
    set_target_properties(${BACKEND_NAME}_backend PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    message(STATUS "Configured backend: ${BACKEND_NAME}")
endfunction()

