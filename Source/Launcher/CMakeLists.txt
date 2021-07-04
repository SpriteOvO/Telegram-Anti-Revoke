cmake_minimum_required(VERSION 3.15)

project(Launcher VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX)


##################################################
# Code files
#

add_executable(
    Launcher WIN32
    
    "Main.cpp"

    "Resource.rc"
)

configure_file("../Common/Config.h.in" "Config.h")
target_include_directories(Launcher PRIVATE ${PROJECT_BINARY_DIR})


##################################################
# Configure the target
#
if (MSVC)

    # Prevent MSBuild from adding the build configuration to the end of the binary directory for binary file output
    #
    set(TAR_BINARY_OUT_DIR "${CMAKE_BINARY_DIR}/Binary")
    set(TAR_OUTPUT_DIRECTORY_TYPES RUNTIME LIBRARY ARCHIVE)
    foreach (OUTPUT_DIRECTORY_TYPE ${TAR_OUTPUT_DIRECTORY_TYPES})
        set_target_properties(Launcher PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY ${TAR_BINARY_OUT_DIR})
        set_target_properties(Launcher PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_DEBUG ${TAR_BINARY_OUT_DIR})
        set_target_properties(Launcher PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_RELEASE ${TAR_BINARY_OUT_DIR})
        set_target_properties(Launcher PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_MINSIZEREL ${TAR_BINARY_OUT_DIR})
        set_target_properties(Launcher PROPERTIES ${OUTPUT_DIRECTORY_TYPE}_OUTPUT_DIRECTORY_RELWITHDEBINFO ${TAR_BINARY_OUT_DIR})
    endforeach()

    # Rename binary file name after build
    #
    string(TOLOWER ${TAR_PLATFORM} TAR_PLATFORM_L)
    add_custom_command(
        TARGET Launcher
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename "${TAR_BINARY_OUT_DIR}/Launcher.exe" "${TAR_BINARY_OUT_DIR}/TAR-Launcher-${TAR_PLATFORM_L}.exe"
    )

endif()
