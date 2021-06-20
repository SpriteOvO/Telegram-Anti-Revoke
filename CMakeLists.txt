cmake_minimum_required(VERSION 3.15)

include(FetchContent)

project(TelegramAntiRevoke VERSION 0.3.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
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
# Third-party libraries
#

# minhook
#
message("Fetching 'minhook'...")
FetchContent_Declare(
    minhook
    GIT_REPOSITORY "https://github.com/TsudaKageyu/minhook.git"
    GIT_TAG "423d1e45af2ed2719a5c31e990e935ef301ed9c3"
)
FetchContent_MakeAvailable(minhook)
message("Fetch 'minhook' done.")

# json
#
message("Fetching 'json'...")
FetchContent_Declare(
    json
    GIT_REPOSITORY "https://github.com/nlohmann/json.git"
    GIT_TAG "v3.9.1"
)
FetchContent_MakeAvailable(json)
message("Fetch 'json' done.")

# spdlog
#
message("Fetching 'spdlog'...")
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG "v1.8.5"
)
FetchContent_MakeAvailable(spdlog)
message("Fetch 'spdlog' done.")


##################################################
# Configure project
#
if (MSVC)
    enable_language(ASM_MASM)

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
else()
    message(FATAL_ERROR "Unsupported compiler.")
endif()


##################################################
# Code files
#
add_library(
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

configure_file("Source/Config.h.in" "Config.h")
target_include_directories(TelegramAntiRevoke PRIVATE ${PROJECT_BINARY_DIR})


##################################################
# Configure the target
#
if (MSVC)
    # Configure for masm code file
    #
    if (TAR_PLATFORM STREQUAL "X86")
        set_source_files_properties ("Source/Assembly.asm" PROPERTIES COMPILE_FLAGS "/safeseh")
    endif()

    # Prevent MSBuild from adding the build type to the end of the binary directory for binary file output
    #
    set(TAR_BINARY_OUT_DIR "${CMAKE_BINARY_DIR}/Binary")
    set(TAR_OUTPUT_DIRECTORY_TYPES RUNTIME LIBRARY ARCHIVE)
    foreach (OUTPUT_DIRECTORY_TYPE ${TAR_OUTPUT_DIRECTORY_TYPES})
        set_target_properties(TelegramAntiRevoke PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY ${TAR_BINARY_OUT_DIR})
        set_target_properties(TelegramAntiRevoke PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_DEBUG ${TAR_BINARY_OUT_DIR})
        set_target_properties(TelegramAntiRevoke PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_RELEASE ${TAR_BINARY_OUT_DIR})
        set_target_properties(TelegramAntiRevoke PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_MINSIZEREL ${TAR_BINARY_OUT_DIR})
        set_target_properties(TelegramAntiRevoke PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_RELWITHDEBINFO ${TAR_BINARY_OUT_DIR})
    endforeach()

    # Rename binary file name after build
    #
    add_custom_command(
        TARGET TelegramAntiRevoke
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename "${TAR_BINARY_OUT_DIR}/TelegramAntiRevoke.dll" "${TAR_BINARY_OUT_DIR}/version.dll"
    )
endif()


##################################################
# Link third-party libraries
#
target_link_libraries(
    TelegramAntiRevoke PRIVATE
    
    nlohmann_json::nlohmann_json
    minhook
    spdlog::spdlog
)
