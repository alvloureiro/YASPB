#=============================================================================
# Testing Configuration
#=============================================================================
# This module handles the configuration and building of test executables.
# To add a new test:
#   1. Create a test file in tests/ directory
#   2. Call add_test_executable() function below
#=============================================================================

function(add_tests)
    # Collect all test source files
    file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")
    
    if(NOT TEST_SOURCES)
        message(STATUS "No test files found in tests/ directory")
        return()
    endif()

    # Add each test file as a separate executable
    foreach(TEST_SOURCE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
        
        add_test_executable(
            NAME ${TEST_NAME}
            SOURCE ${TEST_SOURCE}
        )
    endforeach()
endfunction()

#=============================================================================
# Helper function to add a test executable
#=============================================================================
# Parameters:
#   NAME: Test name (without .cpp extension)
#   SOURCE: Source file path
#   LINK_LIBRARIES: Additional libraries to link (optional)
#=============================================================================
function(add_test_executable)
    set(options "")
    set(oneValueArgs NAME SOURCE)
    set(multiValueArgs LINK_LIBRARIES)
    cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT TEST_NAME)
        message(FATAL_ERROR "add_test_executable: NAME is required")
    endif()

    if(NOT TEST_SOURCE)
        message(FATAL_ERROR "add_test_executable: SOURCE is required")
    endif()

    # Create test executable
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    # Link to core playback library
    target_link_libraries(${TEST_NAME} 
        PRIVATE 
            playback
    )

    # Add additional libraries if provided
    if(TEST_LINK_LIBRARIES)
        target_link_libraries(${TEST_NAME} 
            PRIVATE 
                ${TEST_LINK_LIBRARIES}
        )
    endif()

    # Add test to CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    message(STATUS "Added test: ${TEST_NAME}")
endfunction()

