cmake_minimum_required(VERSION 3.15)

project(TelegramAntiRevoke VERSION 0.4.4)

if(MSVC AND ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20.4")
    #
    # Workaround
    #
    # The current C++20 <format> is defective, and some ABI-breaking changes will be applied soon.
    # So we can't use it in the MSVC stable ABI C++20 (compiler option `/std:c++20`).
    # The way to use <format> now is to specify the `/std:c++latest` option to MSVC.
    #
    # But since CMake 3.20.4, when the value of `CMAKE_CXX_STANDARD` is `20`, the specified option
    # is no longer `/std:c++latest` but `/std:c++20`.
    #
    # There are two workarounds:
    # 1. Set `CMAKE_CXX_STANDARD` to `23`, which makes CMake specify `/std:c++latest` to the compiler.
    # 2. Modify the compiler option specified when `CMAKE_CXX_STANDARD` is `20`.
    #
    # Here we choose the second one, because I think it is relatively cleaner.
    #
    # TODO: Remove this workaround when <format> is available in `/std:c++20`.
    #
    # For more details:
    # https://github.com/microsoft/STL/issues/1814
    # https://gitlab.kitware.com/cmake/cmake/-/commit/3aaf1d91bf353559aef7ec56ad35924383ed613a
    #
    set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
    set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


##################################################
# Check arguments
#
if (CMAKE_GENERATOR_PLATFORM STREQUAL "" AND NOT DEFINED TAR_PLATFORM)
    message(FATAL_ERROR "Neither \"CMAKE_GENERATOR_PLATFORM\" nor \"TAR_PLATFORM\" are defined.")
endif()

if (NOT DEFINED TAR_OS)
    message(FATAL_ERROR "\"TAR_OS\" is undefined.")
endif()


##################################################
# Configure project
#
if (MSVC)

    if (TAR_OS STREQUAL "WIN7" OR TAR_OS STREQUAL "WIN10")
        add_compile_definitions("OS_WIN" "OS_${TAR_OS}")
    else()
        message(FATAL_ERROR "\"TAR_OS\" is invalid.")
    endif()

    if (NOT DEFINED TAR_PLATFORM)
        if (CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set(TAR_PLATFORM "X86")
        elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
            set(TAR_PLATFORM "X64")
        else()
            message(FATAL_ERROR "\"CMAKE_GENERATOR_PLATFORM\" is invalid.")
        endif()
    endif()

    add_compile_definitions("PLATFORM_${TAR_PLATFORM}")

    add_compile_options(
        "/MP"       # Multi-processor compilation = Yes
    )

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

else()
    message(FATAL_ERROR "Unsupported compiler.")
endif()


add_subdirectory(Source)
