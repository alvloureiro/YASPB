#=============================================================================
# Testing Configuration
#=============================================================================
# This module handles the configuration and building of test executables using Google Test.
# To add a new test:
#   1. Create a test file in tests/mock/ directory (for mock backend tests)
#   2. Create a test file in tests/apple/ directory (for Apple backend tests, Apple platforms only)
#   3. Use Google Test macros (TEST, TEST_F, etc.)
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

    # Helper function to create a test executable
    function(add_test_executable target_name test_sources)
        if(NOT test_sources)
            return()
        endif()

        # Create test executable
        add_executable(${target_name} ${test_sources})

        # Add generators directory to include path for tests
        target_include_directories(${target_name}
            PRIVATE
                ${CMAKE_SOURCE_DIR}/tests/generators
        )

        # Link to core playback library
        target_link_libraries(${target_name}
            PRIVATE
                playback
        )

        # Link to Google Test
        if(TARGET GTest::gtest)
            target_link_libraries(${target_name} PRIVATE GTest::gtest GTest::gtest_main)
        elseif(TARGET gtest)
            target_link_libraries(${target_name} PRIVATE gtest gtest_main)
        else()
            message(FATAL_ERROR "Google Test not found and could not be fetched")
        endif()

        # Configure platform-specific flags
        configure_platform_flags(${target_name})

        # Set rpath for test executable to find libraries
        if(PLATFORM_APPLE)
            # On macOS, use @loader_path to find libraries relative to executable
            # Set rpath to find libraries in the build directory
            set_target_properties(${target_name} PROPERTIES
                BUILD_WITH_INSTALL_RPATH OFF
                INSTALL_RPATH "@loader_path/../lib"
            )
            # Add rpath for build directory
            target_link_options(${target_name} PRIVATE
                "LINKER:-rpath,${CMAKE_BINARY_DIR}/lib"
            )
        elseif(PLATFORM_LINUX)
            set_target_properties(${target_name} PROPERTIES
                BUILD_RPATH "${CMAKE_BINARY_DIR}/lib"
                INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
            )
        endif()

        # Add test to CTest
        add_test(NAME ${target_name} COMMAND ${target_name})

        message(STATUS "Added test executable: ${target_name}")
    endfunction()

    # Collect test source files from tests/mock/ directory
    file(GLOB MOCK_TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/mock/*.cpp")

    if(MOCK_TEST_SOURCES)
        # Create mock backend tests executable
        add_test_executable(playback_tests_mock "${MOCK_TEST_SOURCES}")

        # Link to mock_backend if enabled (needed for tests)
        if(ENABLE_MOCK_BACKEND AND TARGET mock_backend)
            target_link_libraries(playback_tests_mock
                PRIVATE
                    mock_backend
            )
        endif()
    else()
        message(STATUS "No test files found in tests/mock/ directory")
    endif()

    # Collect test source files from tests/apple/ directory (Apple platforms only)
    if(PLATFORM_APPLE AND ENABLE_AVFOUNDATION_BACKEND)
        file(GLOB APPLE_TEST_SOURCES
            "${CMAKE_SOURCE_DIR}/tests/apple/*.cpp"
            "${CMAKE_SOURCE_DIR}/tests/apple/*.mm"
        )

        if(APPLE_TEST_SOURCES)
            # Create Apple backend tests executable
            add_test_executable(playback_tests_apple "${APPLE_TEST_SOURCES}")

            # Link to apple_backend if enabled (needed for tests)
            if(TARGET apple_backend)
                target_link_libraries(playback_tests_apple
                    PRIVATE
                        apple_backend
                )
            endif()

            # Apple tests need Objective-C++ support and Foundation framework
            set_target_properties(playback_tests_apple PROPERTIES
                LINKER_LANGUAGE CXX
            )
            target_link_libraries(playback_tests_apple PRIVATE "-framework Foundation")
        else()
            message(STATUS "No test files found in tests/apple/ directory")
        endif()
    endif()

    message(STATUS "Run tests with: ctest or ./bin/playback_tests_*")
endfunction()
