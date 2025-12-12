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
            SOURCE src/backends/MockBackend.cpp
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

    # Warn if no backends are enabled
    if(NOT ENABLE_MOCK_BACKEND AND NOT ENABLE_FFMPEG_BACKEND AND NOT ENABLE_GSTREAMER_BACKEND)
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
    set(oneValueArgs NAME SOURCE)
    set(multiValueArgs LINK_LIBRARIES INCLUDE_DIRS)
    cmake_parse_arguments(BACKEND "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT BACKEND_NAME)
        message(FATAL_ERROR "add_backend: NAME is required")
    endif()

    if(NOT BACKEND_SOURCE)
        message(FATAL_ERROR "add_backend: SOURCE is required")
    endif()

    # Create backend library
    add_library(${BACKEND_NAME}_backend ${BACKEND_SOURCE})

    # Link to core playback library
    target_link_libraries(${BACKEND_NAME}_backend 
        PUBLIC 
            playback
    )

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

