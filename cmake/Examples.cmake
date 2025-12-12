#=============================================================================
# Examples Configuration
#=============================================================================
# This module handles the configuration and building of example executables.
# To add a new example:
#   1. Create an example file in examples/ directory
#   2. Call add_example_executable() function below
#=============================================================================

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
    
    # Link to core playback library
    target_link_libraries(${EXAMPLE_NAME} 
        PRIVATE 
            playback
    )

    # Add additional libraries if provided
    if(EXAMPLE_LINK_LIBRARIES)
        target_link_libraries(${EXAMPLE_NAME} 
            PRIVATE 
                ${EXAMPLE_LINK_LIBRARIES}
        )
    endif()

    message(STATUS "Added example: ${EXAMPLE_NAME}")
endfunction()

