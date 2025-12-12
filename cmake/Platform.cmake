#=============================================================================
# Platform Configuration
#=============================================================================
# This module handles platform detection and platform-specific settings.
#=============================================================================

# Detect platform
if(APPLE)
    set(PLATFORM_OS "macOS")
    set(PLATFORM_APPLE TRUE)
elseif(UNIX)
    set(PLATFORM_OS "Linux")
    set(PLATFORM_LINUX TRUE)
elseif(WIN32)
    set(PLATFORM_OS "Windows")
    set(PLATFORM_WINDOWS TRUE)
else()
    set(PLATFORM_OS "Unknown")
endif()

# Detect architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv6" OR CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
        set(PLATFORM_ARCH "ARM32")
        set(PLATFORM_ARM32 TRUE)
    else()
        set(PLATFORM_ARCH "ARM64")
        set(PLATFORM_ARM64 TRUE)
    endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
    set(PLATFORM_ARCH "x86_64")
    set(PLATFORM_X86_64 TRUE)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386" OR CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
    set(PLATFORM_ARCH "x86")
    set(PLATFORM_X86 TRUE)
else()
    set(PLATFORM_ARCH "Unknown")
endif()

# Detect if cross-compiling
if(CMAKE_CROSSCOMPILING)
    set(PLATFORM_CROSS_COMPILING TRUE)
    message(STATUS "Cross-compiling for: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
else()
    set(PLATFORM_CROSS_COMPILING FALSE)
endif()

# Platform-specific compiler flags
function(configure_platform_flags target)
    # Common flags for all platforms
    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    # Platform-specific flags
    if(PLATFORM_WINDOWS)
        # Windows-specific flags
        target_compile_definitions(${target} PRIVATE
            _WIN32_WINNT=0x0A00  # Windows 10
            NOMINMAX             # Avoid min/max macros
            WIN32_LEAN_AND_MEAN  # Reduce Windows.h includes
        )
        
        # Use static runtime on Windows by default
        if(NOT BUILD_SHARED_LIBS)
            set_property(TARGET ${target} PROPERTY
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set_property(TARGET ${target} PROPERTY
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
        
    elseif(PLATFORM_APPLE)
        # macOS-specific flags
        target_compile_definitions(${target} PRIVATE
            __APPLE__=1
        )
        
        # Set minimum macOS version
        if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
        endif()
        
        set_target_properties(${target} PROPERTIES
            MACOSX_RPATH ON
            BUILD_WITH_INSTALL_RPATH ON
        )
        
    elseif(PLATFORM_LINUX)
        # Linux-specific flags
        target_compile_definitions(${target} PRIVATE
            __LINUX__=1
        )
        
        # Position Independent Code (required for shared libraries on Linux)
        if(BUILD_SHARED_LIBS)
            set_target_properties(${target} PROPERTIES
                POSITION_INDEPENDENT_CODE ON
            )
        endif()
        
        # Raspberry Pi specific optimizations
        if(PLATFORM_ARM32 OR PLATFORM_ARM64)
            if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv6")
                # Raspberry Pi Zero/1
                target_compile_options(${target} PRIVATE
                    -march=armv6zk
                    -mtune=arm1176jzf-s
                    -mfpu=vfp
                )
            elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
                # Raspberry Pi 2/3
                target_compile_options(${target} PRIVATE
                    -march=armv7-a
                    -mtune=cortex-a7
                    -mfpu=neon-vfpv4
                )
            elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
                # Raspberry Pi 4/5 (64-bit)
                target_compile_options(${target} PRIVATE
                    -march=armv8-a
                )
            endif()
        endif()
    endif()
    
    # Debug/Release specific flags
    target_compile_options(${target} PRIVATE
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
        $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
        $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
    )
endfunction()

# Platform-specific linker flags
function(configure_platform_linker_flags target)
    if(PLATFORM_LINUX)
        # Linux-specific linker flags
        target_link_options(${target} PRIVATE
            -Wl,--as-needed
        )
        
        # Raspberry Pi specific
        if(PLATFORM_ARM32 OR PLATFORM_ARM64)
            target_link_options(${target} PRIVATE
                -Wl,--no-undefined
            )
        endif()
    elseif(PLATFORM_APPLE)
        # macOS-specific linker flags
        target_link_options(${target} PRIVATE
            -Wl,-dead_strip
        )
    elseif(PLATFORM_WINDOWS)
        # Windows-specific linker flags
        target_link_options(${target} PRIVATE
            /WX
        )
    endif()
endfunction()

# Print platform information
function(print_platform_info)
    message(STATUS "Platform Information:")
    message(STATUS "  OS: ${PLATFORM_OS}")
    message(STATUS "  Architecture: ${PLATFORM_ARCH}")
    message(STATUS "  System: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "  Processor: ${CMAKE_SYSTEM_PROCESSOR}")
    message(STATUS "  Cross-compiling: ${PLATFORM_CROSS_COMPILING}")
    
    if(PLATFORM_APPLE)
        message(STATUS "  macOS Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
    
    if(CMAKE_C_COMPILER)
        message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
    endif()
    if(CMAKE_CXX_COMPILER)
        message(STATUS "  C++ Compiler: ${CMAKE_CXX_COMPILER}")
    endif()
endfunction()

