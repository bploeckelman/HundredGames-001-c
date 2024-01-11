cmake_minimum_required(VERSION 3.25)
project(100games_001_prong
        VERSION 0.1.0
        DESCRIPTION "100 games #001: prong"
        HOMEPAGE_URL "https://lando.systems"
        LANGUAGES C)
include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # generate compile_commands.json
set(CMAKE_C_STANDARD 11)

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")


### Dependencies --------------------------------------------------------------

# box2d 3 (ie. box2c - https://github.com/erincatto/box2c)
FetchContent_Declare(box2d
        GIT_REPOSITORY https://github.com/erincatto/box2c.git
        GIT_TAG main
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(box2d)

# raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
FetchContent_Declare(raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(raylib)

# raygui
FetchContent_Declare(raygui
        GIT_REPOSITORY https://github.com/raysan5/raygui.git
        GIT_TAG 4.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(raygui)


### Build and Link ------------------------------------------------------------

add_executable(${PROJECT_NAME}
        src/main.c
        src/physics.c
        include/common.h
        include/physics.h
)
#set(raylib_VERBOSE 1)

target_include_directories(${PROJECT_NAME}
        PRIVATE "${raygui_SOURCE_DIR}/src"
        PRIVATE "${raygui_SOURCE_DIR}/styles"
        PRIVATE include
)

# link raylib and box2d
target_link_libraries(${PROJECT_NAME} PRIVATE raylib box2d)

# link mac frameworks if needed
if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
endif()
