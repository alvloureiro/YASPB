#=============================================================================
# Apple Backend Configuration
#=============================================================================
# This module handles the configuration and building of the Apple backend
# using AVFoundation for native macOS/iOS support.
#=============================================================================

function(configure_apple_backend)
    if(NOT ENABLE_APPLE_BACKEND OR NOT PLATFORM_APPLE)
        return()
    endif()

    # Add backend with both C++ and Objective-C++ sources
    add_backend(
        NAME apple
        SOURCE
            src/backends/apple/impl/AppleBackendImpl.cpp
            src/backends/apple/ApplePlaybackController.cpp
            src/backends/apple/AVFoundationWrapper.mm
    )

    # Xcode-specific attributes (only set if using Xcode generator)
    if(CMAKE_GENERATOR STREQUAL "Xcode")
        set_target_properties(apple_backend PROPERTIES
            XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17"
            XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
        )
    endif()

    # Set macOS rpath property
    set_target_properties(apple_backend PROPERTIES
        MACOSX_RPATH ON
    )

    # Link Apple frameworks using -framework flags (correct way for Apple frameworks)
    # Note: Do NOT link to playback here - that creates a circular dependency!
    # The add_backend function already handles include directories.
    # The playback library links to apple_backend in CMakeLists.txt, not the other way around.
    target_link_libraries(apple_backend
        PRIVATE
            "-framework AVFoundation"
            "-framework Foundation"
            "-framework CoreGraphics"
            "-framework ImageIO"
            "-framework CoreServices"
            "-framework CoreMedia"
            "-framework CoreVideo"
            "-framework VideoToolbox"
            "-framework AudioToolbox"
            "-framework Metal"
            "-framework MetalKit"
    )

    # Apply Objective-C++ specific flags only to .mm files
    # The add_backend function already sets LANGUAGE OBJCXX for .mm files
    # We just need to add ARC and module flags for Objective-C++ files
    set_source_files_properties(
        src/backends/apple/AVFoundationWrapper.mm
        PROPERTIES
            COMPILE_FLAGS "-fobjc-arc -fmodules -fcxx-modules"
    )

    # Set minimum deployment target
    if(IOS)
        set_target_properties(apple_backend PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "12.0"
        )
        target_compile_options(apple_backend PRIVATE
            "-mios-version-min=12.0"
        )
    else()
        set_target_properties(apple_backend PROPERTIES
            MACOSX_DEPLOYMENT_TARGET "10.15"
        )
        target_compile_options(apple_backend PRIVATE
            "-mmacosx-version-min=10.15"
        )
    endif()

    message(STATUS "Apple backend enabled (AVFoundation native)")
endfunction()

