cmake_minimum_required(VERSION 3.15)

include(FetchContent)

project(Core VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES C CXX)


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
    GIT_TAG "624f59eacb538904e72ccc887f8808cd0ff17e36"
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

# sigmatch
#
message("Fetching 'sigmatch'...")
FetchContent_Declare(
    sigmatch
    GIT_REPOSITORY "https://github.com/SpriteOvO/sigmatch.git"
    GIT_TAG "v0.1.0"
)
FetchContent_MakeAvailable(sigmatch)
message("Fetch 'sigmatch' done.")

##################################################
# Configure project
#
if (MSVC)
    enable_language(ASM_MASM)
else()
    message(FATAL_ERROR "Unsupported compiler.")
endif()


##################################################
# Code files
#
add_library(
    Core SHARED

    "DllMain.cpp"
    "RealMain.cpp"
    "IAntiRevoke.cpp"
    "Logger.cpp"
    "IRuntime.cpp"
    "IUpdater.cpp"
    "QtString.cpp"
    "Telegram.cpp"
    "Utils.cpp"

    "Assembly.asm"

    "Resource.rc"
)

configure_file("../Common/Config.h.in" "Config.h")
target_include_directories(Core PRIVATE ${PROJECT_BINARY_DIR})


##################################################
# Configure the target
#
if (MSVC)
    # Configure for masm code file
    #
    if (TAR_PLATFORM STREQUAL "X86")
        set_source_files_properties ("Assembly.asm" PROPERTIES COMPILE_FLAGS "/safeseh")
    endif()

    # Prevent MSBuild from adding the build configuration to the end of the binary directory for binary file output
    #
    set(TAR_BINARY_OUT_DIR "${CMAKE_BINARY_DIR}/Binary")
    set(TAR_OUTPUT_DIRECTORY_TYPES RUNTIME LIBRARY ARCHIVE)
    foreach (OUTPUT_DIRECTORY_TYPE ${TAR_OUTPUT_DIRECTORY_TYPES})
        set_target_properties(Core PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY ${TAR_BINARY_OUT_DIR})
        set_target_properties(Core PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_DEBUG ${TAR_BINARY_OUT_DIR})
        set_target_properties(Core PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_RELEASE ${TAR_BINARY_OUT_DIR})
        set_target_properties(Core PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_MINSIZEREL ${TAR_BINARY_OUT_DIR})
        set_target_properties(Core PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_RELWITHDEBINFO ${TAR_BINARY_OUT_DIR})
    endforeach()

    # Rename binary file name after build
    #
    string(TOLOWER ${TAR_PLATFORM} TAR_PLATFORM_L)
    add_custom_command(
        TARGET Core
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TAR_BINARY_OUT_DIR}/TAR-Resources"
        COMMAND ${CMAKE_COMMAND} -E rename "${TAR_BINARY_OUT_DIR}/Core.dll" "${TAR_BINARY_OUT_DIR}/TAR-Resources/${TAR_PLATFORM_L}.dll"
    )
endif()


##################################################
# Link third-party libraries
#
target_link_libraries(
    Core PRIVATE
    
    nlohmann_json::nlohmann_json
    minhook
    spdlog::spdlog
    sigmatch::sigmatch
)
