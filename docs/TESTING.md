# Testing and Coverage Guide

This guide covers testing and code coverage for the yaspb project.

## Testing

The project uses Google Test (GTest) for unit testing. Tests are automatically discovered and can be run with CTest.

### Prerequisites for Testing

Google Test (GTest) is required for running tests. It will be automatically fetched if not found, but you can also install it manually:

**macOS:**
```bash
brew install googletest
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install libgtest-dev
```

**Or set GTEST_ROOT:**
```bash
export GTEST_ROOT=/path/to/gtest
```

### Running Tests

**Using the build script:**

```bash
# Build with tests enabled (default)
./scripts/build-native.sh

# Build with specific backend and tests
./scripts/build-native.sh --clean --backend ffmpeg --tests
```

**Manual build:**

```bash
mkdir build && cd build
cmake -G "Ninja" .. -DBUILD_TESTS=ON
cmake --build .
```

**Running tests:**

```bash
# Run all tests
ctest

# Run tests with verbose output
ctest --verbose

# Run tests directly
./bin/playback_tests_mock

# Run specific test
./bin/playback_tests_mock --gtest_filter=MediaSourceTest.*

# Disable tests if GTest is not available
cmake .. -DBUILD_TESTS=OFF
```

### Test Structure

Tests are organized by component:
- `test_api_mediasource.cpp` - Tests for IMediaSource interface
- `test_api_playbackcontroller.cpp` - Tests for IPlaybackController interface
- `test_backends.cpp` - Tests for IPlaybackBackend interface
- `test_core_playbackengine.cpp` - Tests for PlaybackEngine
- `test_core_playbackfactory.cpp` - Tests for PlaybackFactory

All tests use the Mock backend for testing, which requires no external dependencies.

## Code Coverage

The project supports code coverage analysis using `gcov` and `lcov` to track how much of the codebase is covered by tests. Coverage reports can be generated for each backend implementation separately.

### Prerequisites for Coverage

**macOS:**
```bash
brew install lcov
# gcov is typically included with Xcode Command Line Tools
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install lcov gcov
```

### Generating Coverage Reports

**Using the build script:**

```bash
# Build with coverage enabled
./scripts/build-native.sh --clean --backend mock --tests --coverage

# Build and run tests
cd build
cmake --build .
ctest

# Generate coverage report
cmake --build . --target coverage

# Generate HTML coverage report
cmake --build . --target coverage-html

# View the report
open coverage/html/index.html  # macOS
# or
xdg-open coverage/html/index.html  # Linux
```

**Manual build:**

```bash
mkdir build && cd build
cmake -G "Ninja" .. -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build .

# Run tests to generate coverage data
ctest

# Generate coverage report
cmake --build . --target coverage

# Generate HTML coverage report
cmake --build . --target coverage-html
```

### Per-Backend Coverage Reports

You can generate coverage reports for individual backends:

```bash
# Coverage for Mock backend
cmake --build . --target coverage-mock

# Coverage for FFmpeg backend (if enabled)
cmake --build . --target coverage-ffmpeg

# Coverage for Apple backend (if enabled, macOS only)
cmake --build . --target coverage-apple
```

Each backend report will be available in:
- `coverage/backend-<name>/html/index.html` - HTML report
- `coverage/backend-<name>/coverage_filtered.info` - Coverage data file

### Coverage Targets

- `coverage` - Generate combined coverage data for all backends
- `coverage-html` - Generate HTML coverage report (requires genhtml)
- `coverage-summary` - Print coverage summary to console
- `coverage-clean` - Remove all coverage files
- `coverage-<backend>` - Generate coverage for specific backend (e.g., `coverage-mock`, `coverage-ffmpeg`)

### Coverage Report Locations

- Combined report: `coverage/html/index.html`
- Per-backend reports: `coverage/backend-<name>/html/index.html`
- Coverage data files: `coverage/*.info`

## Writing Tests

### Test Structure

Tests follow Google Test conventions:

```cpp
#include <gtest/gtest.h>
#include <playback/api/IPlaybackController.h>

class PlaybackControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }

    void TearDown() override {
        // Cleanup code
    }
};

TEST_F(PlaybackControllerTest, BasicPlayback) {
    // Test implementation
}
```

### Best Practices

1. **Use descriptive test names**: Test names should clearly describe what is being tested
2. **Test one thing per test**: Each test should verify a single behavior
3. **Use fixtures**: Share setup code using test fixtures
4. **Mock external dependencies**: Use the Mock backend for testing
5. **Test error cases**: Don't just test happy paths
6. **Keep tests independent**: Tests should not depend on each other

## Continuous Integration

For CI/CD pipelines, you can use the build scripts or CMake commands directly:

```yaml
# Example GitHub Actions
- name: Build and Test
  run: |
    ./scripts/build-native.sh --clean --backend mock --tests --coverage
    cd build
    cmake --build .
    ctest --output-on-failure
    cmake --build . --target coverage-html
```

## Coverage Goals

While there are no strict coverage requirements, aim for:
- **Core components**: > 80% coverage
- **Backend implementations**: > 70% coverage
- **Public API**: > 90% coverage

Use coverage reports to identify untested code and improve test coverage over time.

