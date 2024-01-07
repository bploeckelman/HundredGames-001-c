cmake_minimum_required(VERSION 3.15)
project(100games_001_prong C)

### Settings ------------------------------------------------------------------

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")

# Raylib specific flags
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
set(RAYLIB_VERSION 5.0)
set(RAYGUI_VERSION 4.0)

include(FetchContent)

### Dependencies --------------------------------------------------------------

find_package(raylib ${RAYLIB_VERSION} QUIET)
if (NOT raylib_FOUND)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)

    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
    endif()
endif()

find_package(raygui ${RAYGUI_VERSION} QUIET)
if (NOT raygui_FOUND)
    FetchContent_Declare(
            raygui
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raygui/archive/refs/tags/${RAYGUI_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raygui)

    if (NOT raygui_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raygui)
        # the single file header-only library is added
        # as a target include directory via ${raygui_SOURCE_DIR}
    endif()
endif()

### Executable ----------------------------------------------------------------

add_executable(${PROJECT_NAME}
        src/main.c
        include/common.h
)
#set(raylib_VERBOSE 1)

target_include_directories(${PROJECT_NAME}
        PRIVATE "${raygui_SOURCE_DIR}/src"
        PRIVATE "${raygui_SOURCE_DIR}/styles"
        PRIVATE include
)

### Library linkage -----------------------------------------------------------

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
endif()
