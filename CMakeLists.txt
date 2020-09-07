# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(paf)

file(GLOB check_sources ${PROJECT_SOURCE_DIR}/main/*.c)
file(GLOB check_headers ${PROJECT_SOURCE_DIR}/main/*.h)
set(check_files
    ${check_sources}
    ${check_headers})

if(ENABLE_ASTYLE)

    list(APPEND ASTYLE_CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
    )

    ExternalProject_Add(
        astyle
        GIT_REPOSITORY      https://github.com/Bareflank/astyle.git
        GIT_TAG             v1.2
        GIT_SHALLOW         1
        CMAKE_ARGS          ${ASTYLE_CMAKE_ARGS}
        PREFIX              ${CMAKE_BINARY_DIR}/external/astyle/prefix
        TMP_DIR             ${CMAKE_BINARY_DIR}/external/astyle/tmp
        STAMP_DIR           ${CMAKE_BINARY_DIR}/external/astyle/stamp
        DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/astyle/download
        SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/astyle/src
        BINARY_DIR          ${CMAKE_BINARY_DIR}/external/astyle/build
    )

    list(APPEND ASTYLE_ARGS
        --style=linux
        --lineend=linux
        --suffix=none
        --pad-oper
        --unpad-paren
        --break-closing-brackets
        --align-pointer=name
        --align-reference=name
        --indent-preproc-define
        --indent-switches
        --indent-col1-comments
        --keep-one-line-statements
        --keep-one-line-blocks
        --pad-header
        --convert-tabs
        --min-conditional-indent=0
        --indent=spaces=4
        --close-templates
        --add-brackets
        --break-after-logical
        ${check_files}
    )

    if(NOT WIN32 STREQUAL "1")
        add_custom_target(
            format
            COMMAND ${CMAKE_BINARY_DIR}/bin/astyle ${ASTYLE_ARGS}
            COMMENT "running astyle"
        )
    else()
        add_custom_target(
            format
            COMMAND ${CMAKE_BINARY_DIR}/bin/astyle.exe ${ASTYLE_ARGS}
            COMMENT "running astyle"
        )
    endif()
endif()

add_custom_target(
    commit
    COMMAND echo COMMAND git diff --check HEAD^
    COMMENT "Running git check"
)

IF(NOT ESP_OPENOCD)
    FIND_PROGRAM(ESP_OPENOCD openocd-esp32)
    IF(ESP_OPENOCD STREQUAL "ESP_OPENOCD-NOTFOUND")
        FIND_PROGRAM(ESP_OPENOCD openocd
            HINTS $ENV{HOME}/.espressif/tools
            PATH_SUFFIXES openocd-esp32)
    ENDIF()
ENDIF()

IF(ESP_OPENOCD STREQUAL "ESP_OPENOCD-NOTFOUND")
    MESSAGE(STATUS "openocd-esp32 not found, debugging target unavailable")
ELSE()
    MESSAGE(STATUS "openocd-esp32 found: ${ESP_OPENOCD}, debugging target available")
    ADD_CUSTOM_TARGET(
        debug
        COMMAND ${ESP_OPENOCD} -f $ENV{OPENOCD_SCRIPTS}/target/esp32.cfg -f $ENV{OPENOCD_SCRIPTS}/interface/jlink.cfg
        COMMAND xtensa-esp32-elf-gdb -tui -x ${CMAKE_CURRENT_LIST_DIR}/GDBCommands ${CMAKE_CURRENT_LIST_DIR}/build/${CMAKE_PROJECT_NAME}.elf
        )
ENDIF()

