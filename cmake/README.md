# CMake Build System

This directory contains modular CMake configuration files for better organization and maintainability.

## Structure

- **Backends.cmake**: Handles configuration and building of playback backends (FFmpeg, GStreamer, etc.)
- **Testing.cmake**: Manages test executable creation and CTest integration
- **Examples.cmake**: Handles example executable creation
- **Install.cmake**: Configures installation rules and CMake package exports

## Adding a New Backend

1. Add a build option in the main `CMakeLists.txt`:
   ```cmake
   option(ENABLE_MY_BACKEND "Enable My Backend support" OFF)
   ```

2. Add backend configuration in `cmake/Backends.cmake`:
   ```cmake
   if(ENABLE_MY_BACKEND)
       find_package(MyBackend REQUIRED)
       
       add_backend(
           NAME my_backend
           SOURCE src/backends/MyBackend.cpp
           LINK_LIBRARIES ${MY_BACKEND_LIBRARIES}
           INCLUDE_DIRS ${MY_BACKEND_INCLUDE_DIRS}
       )
   endif()
   ```

3. Create the backend source file in `src/backends/MyBackend.cpp`

## Adding Tests

Simply create a `.cpp` file in the `tests/` directory. The build system will automatically:
- Create an executable with the same name (without extension)
- Link it to the playback library
- Register it with CTest

For custom test configuration, use `add_test_executable()` directly in `cmake/Testing.cmake`.

## Adding Examples

Create a `.cpp` file in the `examples/` directory. The build system will automatically:
- Create an executable with the same name (without extension)
- Link it to the playback library

For custom example configuration, use `add_example_executable()` directly in `cmake/Examples.cmake`.

