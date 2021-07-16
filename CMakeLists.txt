cmake_minimum_required(VERSION 3.15)

project(TelegramAntiRevoke VERSION 0.4.0)

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
        add_compile_definitions("OS_${TAR_OS}")
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
