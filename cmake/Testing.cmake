#=============================================================================
# Testing Configuration
#=============================================================================
# This module handles the configuration and building of test executables using Google Test.
# To add a new test:
#   1. Create a test file in tests/ directory
#   2. Use Google Test macros (TEST, TEST_F, etc.)
#=============================================================================

function(add_tests)
    # Try to find system-installed GTest first
    find_package(GTest QUIET)

    # If not found, fetch from GitHub
    if(NOT GTest_FOUND AND NOT TARGET gtest AND NOT TARGET GTest::gtest)
        message(STATUS "GTest not found on system, fetching from GitHub...")
        message(STATUS "If this fails, install GTest: brew install googletest (macOS) or apt-get install libgtest-dev (Linux)")

        # Google Test via FetchContent
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.12.1
        )
        FetchContent_MakeAvailable(googletest)
    endif()

    # Collect all test source files
    file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

    if(NOT TEST_SOURCES)
        message(STATUS "No test files found in tests/ directory")
        return()
    endif()

    # Create a single test executable with all test files
    add_executable(playback_tests ${TEST_SOURCES})

    # Link to core playback library
    target_link_libraries(playback_tests
        PRIVATE
            playback
    )

    # Link to Google Test
    if(TARGET GTest::gtest)
        target_link_libraries(playback_tests PRIVATE GTest::gtest GTest::gtest_main)
    elseif(TARGET gtest)
        target_link_libraries(playback_tests PRIVATE gtest gtest_main)
    else()
        message(FATAL_ERROR "Google Test not found and could not be fetched")
    endif()

    # Link to mock_backend if enabled (needed for tests)
    if(ENABLE_MOCK_BACKEND AND TARGET mock_backend)
        target_link_libraries(playback_tests
            PRIVATE
                mock_backend
        )
    endif()

    # Configure platform-specific flags
    configure_platform_flags(playback_tests)

    # Set rpath for test executable to find libraries
    if(PLATFORM_APPLE)
        # On macOS, use @loader_path to find libraries relative to executable
        # Set rpath to find libraries in the build directory
        set_target_properties(playback_tests PROPERTIES
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH "@loader_path/../lib"
        )
        # Add rpath for build directory
        target_link_options(playback_tests PRIVATE
            "LINKER:-rpath,${CMAKE_BINARY_DIR}/lib"
        )
    elseif(PLATFORM_LINUX)
        set_target_properties(playback_tests PROPERTIES
            BUILD_RPATH "${CMAKE_BINARY_DIR}/lib"
            INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
        )
    endif()

    # Add test to CTest
    add_test(NAME playback_tests COMMAND playback_tests)

    # Enable test discovery (optional - can also run tests manually)
    # Note: gtest_discover_tests may try to run during configure, so we add it manually
    # Users can run: ctest or ./bin/playback_tests

    message(STATUS "Added test executable: playback_tests")
    message(STATUS "Run tests with: ctest or ./bin/playback_tests")
endfunction()
