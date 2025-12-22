#=============================================================================
# Code Coverage Configuration
#=============================================================================
# This module handles code coverage using gcov and lcov.
# It generates coverage reports for each backend implementation separately.
#
# Usage:
#   cmake .. -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
#   cmake --build .
#   ctest
#   cmake --build . --target coverage
#   cmake --build . --target coverage-html
#
# Coverage reports will be generated in:
#   - ${CMAKE_BINARY_DIR}/coverage/ - Combined coverage info
#   - ${CMAKE_BINARY_DIR}/coverage/html/ - HTML reports
#   - ${CMAKE_BINARY_DIR}/coverage/backend-*/ - Per-backend reports
#=============================================================================

# Check if coverage is enabled
if(NOT ENABLE_COVERAGE)
    # Define no-op functions if coverage is disabled
    function(enable_coverage_for_target target_name)
        # No-op
    endfunction()

    function(add_backend_coverage backend_name source_dir)
        # No-op
    endfunction()

    return()
endif()

# Check for required tools
find_program(GCOV_PATH gcov)
find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)

if(NOT GCOV_PATH)
    message(WARNING "gcov not found. Code coverage will not be available.")
    message(WARNING "Install gcov: brew install gcc (macOS) or apt-get install gcov (Linux)")
    # Define no-op functions
    function(enable_coverage_for_target target_name)
        # No-op
    endfunction()

    function(add_backend_coverage backend_name source_dir)
        # No-op
    endfunction()
    return()
endif()

if(NOT LCOV_PATH)
    message(WARNING "lcov not found. Code coverage will not be available.")
    message(WARNING "Install lcov: brew install lcov (macOS) or apt-get install lcov (Linux)")
    # Define no-op functions
    function(enable_coverage_for_target target_name)
        # No-op
    endfunction()

    function(add_backend_coverage backend_name source_dir)
        # No-op
    endfunction()
    return()
endif()

if(NOT GENHTML_PATH)
    message(WARNING "genhtml not found. HTML reports will not be available.")
    message(WARNING "Install lcov (includes genhtml): brew install lcov (macOS) or apt-get install lcov (Linux)")
endif()

message(STATUS "Code coverage enabled")
message(STATUS "  gcov: ${GCOV_PATH}")
message(STATUS "  lcov: ${LCOV_PATH}")
if(GENHTML_PATH)
    message(STATUS "  genhtml: ${GENHTML_PATH}")
endif()

# Coverage flags
set(COVERAGE_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
set(COVERAGE_LINK_FLAGS "--coverage")

# Coverage output directories
set(COVERAGE_DIR "${CMAKE_BINARY_DIR}/coverage")
set(COVERAGE_HTML_DIR "${COVERAGE_DIR}/html")
set(COVERAGE_INFO_FILE "${COVERAGE_DIR}/coverage.info")
set(COVERAGE_INFO_FILE_FILTERED "${COVERAGE_DIR}/coverage_filtered.info")

# Create coverage directory
file(MAKE_DIRECTORY ${COVERAGE_DIR})
file(MAKE_DIRECTORY ${COVERAGE_HTML_DIR})

#=============================================================================
# Function to enable coverage for a target
#=============================================================================
function(enable_coverage_for_target target_name)
    if(NOT ENABLE_COVERAGE)
        return()
    endif()

    # Add coverage flags to compilation
    target_compile_options(${target_name} PRIVATE ${COVERAGE_FLAGS})

    # Add coverage flags to linking
    target_link_options(${target_name} PRIVATE ${COVERAGE_LINK_FLAGS})

    # Set coverage output directory
    set_target_properties(${target_name} PROPERTIES
        COMPILE_OPTIONS "${COVERAGE_FLAGS}"
    )

    message(STATUS "Coverage enabled for target: ${target_name}")
endfunction()

#=============================================================================
# Function to collect coverage data for a backend
#=============================================================================
function(add_backend_coverage backend_name source_dir)
    if(NOT ENABLE_COVERAGE)
        return()
    endif()

    set(BACKEND_COVERAGE_DIR "${COVERAGE_DIR}/backend-${backend_name}")
    set(BACKEND_COVERAGE_INFO "${BACKEND_COVERAGE_DIR}/coverage.info")
    set(BACKEND_COVERAGE_INFO_FILTERED "${BACKEND_COVERAGE_DIR}/coverage_filtered.info")
    set(BACKEND_COVERAGE_HTML "${BACKEND_COVERAGE_DIR}/html")

    file(MAKE_DIRECTORY ${BACKEND_COVERAGE_DIR})
    file(MAKE_DIRECTORY ${BACKEND_COVERAGE_HTML})

    # Create target to generate backend-specific coverage
    add_custom_target(coverage-${backend_name}
        COMMAND ${CMAKE_COMMAND} -E echo "Collecting coverage data for ${backend_name} backend..."

        # Reset counters
        COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters || true

        # Capture baseline (required for lcov)
        COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture --initial --output-file ${BACKEND_COVERAGE_INFO} || true

        # Run tests (if test target exists)
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R ${backend_name} || ${CMAKE_CTEST_COMMAND} --output-on-failure || true

        # Capture coverage data
        COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${BACKEND_COVERAGE_INFO} || true

        # Filter to only include backend source files
        COMMAND ${LCOV_PATH} --extract ${BACKEND_COVERAGE_INFO} "${source_dir}/*" --output-file ${BACKEND_COVERAGE_INFO_FILTERED} || true

        # Remove system headers and test files
        COMMAND ${LCOV_PATH} --remove ${BACKEND_COVERAGE_INFO_FILTERED}
            '/usr/*'
            '*/tests/*'
            '*/test/*'
            '*/googletest/*'
            '*/gtest/*'
            '*/gmock/*'
            '*/include/*'
            --output-file ${BACKEND_COVERAGE_INFO_FILTERED} || true

        # Generate HTML report (if genhtml is available)
        COMMAND ${CMAKE_COMMAND} -E echo "Generating HTML report..."
        COMMAND ${GENHTML_PATH} ${BACKEND_COVERAGE_INFO_FILTERED}
            --output-directory ${BACKEND_COVERAGE_HTML}
            --title "${backend_name} Backend Coverage"
            --show-details
            --legend
            --demangle-cpp || ${CMAKE_COMMAND} -E echo "genhtml not available, skipping HTML report"

        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage report for ${backend_name} backend: ${BACKEND_COVERAGE_HTML}/index.html"

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report for ${backend_name} backend"
    )

    # Add dependency on test targets
    if(TARGET playback_tests_mock)
        add_dependencies(coverage-${backend_name} playback_tests_mock)
    endif()
    if(TARGET playback_tests_apple)
        add_dependencies(coverage-${backend_name} playback_tests_apple)
    endif()
    if(TARGET playback_tests_ffmpeg)
        add_dependencies(coverage-${backend_name} playback_tests_ffmpeg)
    endif()
endfunction()

#=============================================================================
# Main coverage target - generates combined coverage report
#=============================================================================
add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND} -E echo "Collecting coverage data..."

    # Reset counters
    COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters

    # Run all tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure || true

    # Capture baseline
    COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture --initial --output-file ${COVERAGE_INFO_FILE} || true

    # Capture coverage data
    COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${COVERAGE_INFO_FILE} || true

    # Remove system headers and test files
    COMMAND ${LCOV_PATH} --remove ${COVERAGE_INFO_FILE}
        '/usr/*'
        '*/tests/*'
        '*/test/*'
        '*/googletest/*'
        '*/gtest/*'
        '*/gmock/*'
        '*/include/*'  # Exclude header-only files
        --output-file ${COVERAGE_INFO_FILE_FILTERED} || true

    COMMAND ${CMAKE_COMMAND} -E echo "Coverage info file: ${COVERAGE_INFO_FILE_FILTERED}"
    COMMAND ${CMAKE_COMMAND} -E echo "Use 'make coverage-html' to generate HTML report"

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage data"
)

# Add dependency on test targets
if(TARGET playback_tests_mock)
    add_dependencies(coverage playback_tests_mock)
endif()
if(TARGET playback_tests_apple)
    add_dependencies(coverage playback_tests_apple)
endif()
if(TARGET playback_tests_ffmpeg)
    add_dependencies(coverage playback_tests_ffmpeg)
endif()

#=============================================================================
# HTML coverage report target
#=============================================================================
if(GENHTML_PATH)
    add_custom_target(coverage-html
        DEPENDS coverage

        COMMAND ${CMAKE_COMMAND} -E echo "Generating HTML coverage report..."

        # Generate HTML report
        COMMAND ${GENHTML_PATH} ${COVERAGE_INFO_FILE_FILTERED}
            --output-directory ${COVERAGE_HTML_DIR}
            --title "StreamingPlayback Coverage Report"
            --show-details
            --legend
            --demangle-cpp || true

        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated: ${COVERAGE_HTML_DIR}/index.html"
        COMMAND ${CMAKE_COMMAND} -E echo "Open in browser to view detailed coverage information"

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating HTML coverage report"
    )
endif()

#=============================================================================
# Clean coverage target
#=============================================================================
add_custom_target(coverage-clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${COVERAGE_DIR}
    COMMAND ${CMAKE_COMMAND} -E echo "Coverage directory removed: ${COVERAGE_DIR}"
    COMMENT "Cleaning coverage files"
)

#=============================================================================
# Coverage summary target - prints coverage summary
#=============================================================================
add_custom_target(coverage-summary
    DEPENDS coverage

    COMMAND ${CMAKE_COMMAND} -E echo "Coverage Summary:"
    COMMAND ${LCOV_PATH} --summary ${COVERAGE_INFO_FILE_FILTERED} || true

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage summary"
)

message(STATUS "Coverage targets available:")
message(STATUS "  coverage          - Generate coverage data")
if(GENHTML_PATH)
    message(STATUS "  coverage-html     - Generate HTML coverage report")
endif()
message(STATUS "  coverage-summary  - Print coverage summary")
message(STATUS "  coverage-clean    - Clean coverage files")
message(STATUS "  coverage-<backend> - Generate per-backend coverage (e.g., coverage-mock, coverage-ffmpeg)")

