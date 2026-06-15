cmake_minimum_required(VERSION 3.28)
project(Ballistic VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BALLISTIC_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# execute_process(
#     COMMAND git rev-parse --short HEAD
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#     OUTPUT_VARIABLE GIT_COMMIT_HASH
#     OUTPUT_STRIP_TRAILING_WHITESPACE
#     ERROR_QUIET
# )

# if(NOT GIT_COMMIT_HASH)
#     set(GIT_COMMIT_HASH "unknown")
# endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

include(CompilerOptions)
include(Platform)
include(Dependencies)

add_subdirectory(thirdparty)

option(BALLISTIC_BUILD_EDITOR "Build the editor" ON)
option(BALLISTIC_BUILD_GAME "Build the game runtime" ON)
option(BALLISTIC_BUILD_LAUNCHER "Build the launcher" ON)

add_subdirectory(engine)

add_subdirectory(launcher)

# add_subdirectory(editor)
# add_subdirectory(game)