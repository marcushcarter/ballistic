cmake_minimum_required(VERSION 3.28)
project(Ballistic VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BALLISTIC_VERSION "0.1.0")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompilerOptions)
include(Platform)
include(Dependencies)

add_subdirectory(Thirdparty)
add_subdirectory(Engine)

option(BALLISTIC_BUILD_EDITOR "Build the editor" ON)
option(BALLISTIC_BUILD_GAME "Build the game runtime" ON)

if(BALLISTIC_BUILD_EDITOR)
    add_subdirectory(Editor)
endif()

if(BALLISTIC_BUILD_GAME)
    add_subdirectory(Game)
endif()