cmake_minimum_required (VERSION 3.15)

project (TelegramAntiRevoke VERSION 0.3.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Check arguments
#
if (CMAKE_GENERATOR_PLATFORM STREQUAL "" AND NOT DEFINED TAR_PLATFORM)
    message (FATAL_ERROR "Neither \"CMAKE_GENERATOR_PLATFORM\" nor \"TAR_PLATFORM\" are defined.")
endif ()

if (NOT DEFINED TAR_OS)
    message (FATAL_ERROR "\"TAR_OS\" is undefined.")
endif ()


# Configure project
#
if (MSVC)
    enable_language (ASM_MASM)

    if (TAR_OS STREQUAL "WIN7" OR TAR_OS STREQUAL "WIN10")
        add_compile_definitions ("OS_${TAR_OS}")
    else ()
        message (FATAL_ERROR "\"TAR_OS\" is invalid.")
    endif ()

    if (NOT DEFINED TAR_PLATFORM)
        if (CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set (TAR_PLATFORM "X86")
        elseif (CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
            set (TAR_PLATFORM "X64")
        else ()
            message (FATAL_ERROR "\"CMAKE_GENERATOR_PLATFORM\" is invalid.")
        endif ()
    endif ()

    add_compile_definitions ("PLATFORM_${TAR_PLATFORM}")
else ()
    message (FATAL_ERROR "Unsupported compiler.")
endif ()


# Third-party libraries
#
add_subdirectory ("Source/ThirdParty/json")
add_subdirectory ("Source/ThirdParty/minhook")
add_subdirectory ("Source/ThirdParty/spdlog")


# Code files
#
add_library (
    TelegramAntiRevoke SHARED

    "Source/DllMain.cpp"
    "Source/RealMain.cpp"
    "Source/IAntiRevoke.cpp"        "Source/IAntiRevoke.h"
    "Source/Logger.cpp"             "Source/Logger.h"
    "Source/IRuntime.cpp"           "Source/IRuntime.h"
    "Source/IUpdater.cpp"           "Source/IUpdater.h"
    "Source/QtString.cpp"           "Source/QtString.h"
    "Source/Telegram.cpp"           "Source/Telegram.h"
    "Source/Utils.cpp"              "Source/Utils.h"

    "Source/Assembly.asm"

    "Source/TelegramAntiRevoke.rc"  "Source/Resource.h"
)

configure_file ("Source/Config.h.in" "Config.h")
target_include_directories (TelegramAntiRevoke PRIVATE ${PROJECT_BINARY_DIR})


# Configure the target
#
if (MSVC)
    # Configure for masm code file
    #
    if (TAR_PLATFORM STREQUAL "X86")
        set_source_files_properties ("Source/Assembly.asm" PROPERTIES COMPILE_FLAGS "/safeseh")
    endif ()

    # Prevent MSBuild from adding the build type to the end of the binary directory for binary file output
    #
    set (TAR_BINARY_OUT_DIR "${CMAKE_BINARY_DIR}/Binary")

    set_target_properties (TelegramAntiRevoke PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${TAR_BINARY_OUT_DIR})

    set_target_properties (TelegramAntiRevoke PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${TAR_BINARY_OUT_DIR})

    set_target_properties (TelegramAntiRevoke PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${TAR_BINARY_OUT_DIR})
    set_target_properties (TelegramAntiRevoke PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${TAR_BINARY_OUT_DIR})

    # Rename binary file name after build
    #
    add_custom_command (
        TARGET TelegramAntiRevoke
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename "${TAR_BINARY_OUT_DIR}/TelegramAntiRevoke.dll" "${TAR_BINARY_OUT_DIR}/version.dll"
    )
endif ()


# Link third-party libraries
#
target_link_libraries (TelegramAntiRevoke PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries (TelegramAntiRevoke PRIVATE minhook)
target_link_libraries (TelegramAntiRevoke PRIVATE spdlog::spdlog)
