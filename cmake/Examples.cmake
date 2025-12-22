#=============================================================================
# Examples Configuration
#=============================================================================
# This module handles the configuration and building of example executables.
# To add a new example:
#   1. Create an example file in examples/ directory
#   2. Call add_example_executable() function below
#
# Note: If a specific example is enabled (e.g., BUILD_FFMPEG_AUDIO_EXAMPLE=ON),
# BUILD_EXAMPLES will be automatically enabled.
#=============================================================================

# Function to check and auto-enable BUILD_EXAMPLES if needed
# This is called early (before add_examples) to set BUILD_EXAMPLES
function(check_and_enable_examples)
    # Auto-enable BUILD_EXAMPLES if any specific example is enabled
    if(BUILD_APPLE_AUDIO_EXAMPLE OR BUILD_FFMPEG_AUDIO_EXAMPLE)
        if(NOT BUILD_EXAMPLES)
            message(STATUS "Auto-enabling BUILD_EXAMPLES (specific example is enabled)")
            set(BUILD_EXAMPLES ON CACHE BOOL "Build example executables" FORCE)
        endif()
    endif()
endfunction()

function(add_examples)
    # Collect all example source files
    file(GLOB EXAMPLE_SOURCES "${CMAKE_SOURCE_DIR}/examples/*.cpp")

    if(NOT EXAMPLE_SOURCES)
        message(STATUS "No example files found in examples/ directory")
        return()
    endif()

    # Add each example file as a separate executable
    foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)

        # Skip examples based on build options and backend availability
        if(EXAMPLE_NAME STREQUAL "apple_audio_example")
            if(NOT BUILD_APPLE_AUDIO_EXAMPLE)
                message(STATUS "Skipping ${EXAMPLE_NAME} (BUILD_APPLE_AUDIO_EXAMPLE is OFF)")
                continue()
            endif()
            if(NOT ENABLE_APPLE_BACKEND OR NOT PLATFORM_APPLE)
                message(STATUS "Skipping ${EXAMPLE_NAME} (Apple backend not enabled)")
                continue()
            endif()
        endif()

        if(EXAMPLE_NAME STREQUAL "ffmpeg_audio_example")
            if(NOT BUILD_FFMPEG_AUDIO_EXAMPLE)
                message(STATUS "Skipping ${EXAMPLE_NAME} (BUILD_FFMPEG_AUDIO_EXAMPLE is OFF)")
                continue()
            endif()
            if(NOT ENABLE_FFMPEG_BACKEND)
                message(STATUS "Skipping ${EXAMPLE_NAME} (FFmpeg backend not enabled)")
                continue()
            endif()
        endif()

        add_example_executable(
            NAME ${EXAMPLE_NAME}
            SOURCE ${EXAMPLE_SOURCE}
        )
    endforeach()
endfunction()

#=============================================================================
# Helper function to add an example executable
#=============================================================================
# Parameters:
#   NAME: Example name (without .cpp extension)
#   SOURCE: Source file path
#   LINK_LIBRARIES: Additional libraries to link (optional)
#=============================================================================
function(add_example_executable)
    set(options "")
    set(oneValueArgs NAME SOURCE)
    set(multiValueArgs LINK_LIBRARIES)
    cmake_parse_arguments(EXAMPLE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT EXAMPLE_NAME)
        message(FATAL_ERROR "add_example_executable: NAME is required")
    endif()

    if(NOT EXAMPLE_SOURCE)
        message(FATAL_ERROR "add_example_executable: SOURCE is required")
    endif()

    # Create example executable
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})

    # Add examples directory to include path (for example-specific headers)
    target_include_directories(${EXAMPLE_NAME}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/examples
    )

    # Link to core playback library
    target_link_libraries(${EXAMPLE_NAME}
        PRIVATE
            playback
    )

    # Link to specific backends if the example needs them
    # apple_audio_example needs apple_backend for ApplePlaybackController
    if(EXAMPLE_NAME STREQUAL "apple_audio_example" AND TARGET apple_backend)
        target_link_libraries(${EXAMPLE_NAME}
            PRIVATE
                apple_backend
        )
        message(STATUS "  Linked ${EXAMPLE_NAME} to apple_backend")
    endif()

    # ffmpeg_audio_example needs ffmpeg_backend
    if(EXAMPLE_NAME STREQUAL "ffmpeg_audio_example" AND TARGET ffmpeg_backend)
        target_link_libraries(${EXAMPLE_NAME}
            PRIVATE
                ffmpeg_backend
        )
        message(STATUS "  Linked ${EXAMPLE_NAME} to ffmpeg_backend")
    endif()

    # Add additional libraries if provided
    if(EXAMPLE_LINK_LIBRARIES)
        target_link_libraries(${EXAMPLE_NAME}
            PRIVATE
                ${EXAMPLE_LINK_LIBRARIES}
        )
    endif()

    # Set properties for better IDE integration (CLion, etc.)
    set_target_properties(${EXAMPLE_NAME} PROPERTIES
        FOLDER "Examples"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    message(STATUS "Added example: ${EXAMPLE_NAME}")
endfunction()

