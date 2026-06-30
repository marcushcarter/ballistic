cmake_minimum_required(VERSION 3.28)
project(Ballistic VERSION 2.0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BALLISTIC_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/Version.gen.h.in
    ${CMAKE_BINARY_DIR}/generated/core/version_generated.gen.h
    @ONLY
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} /I\"${CMAKE_SOURCE_DIR}\" /I\"${CMAKE_BINARY_DIR}/generated\"")

option(BALLISTIC_BUILD_EDITOR "Build the editor" ON)
option(BALLISTIC_BUILD_GAME "Build the game runtime" ON)
option(BALLISTIC_BUILD_CONSOLE_WRAPPER "Build the console wrapper" ON)

include(CompilerOptions)
include(Platform)
include(Dependencies)
include(EmbedAssets)

add_subdirectory(thirdparty)
add_subdirectory(drivers)
add_subdirectory(core)
add_subdirectory(editor)

set(BALLISTIC_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${BALLISTIC_GENERATED_DIR})

ballistic_generate_asset_rc(
    ${BALLISTIC_GENERATED_DIR}/embedded_assets_shared.rc
    ${CMAKE_SOURCE_DIR}/assets/shared/shaders
    ${CMAKE_SOURCE_DIR}/assets/shared/images
)

ballistic_generate_asset_rc(
    ${BALLISTIC_GENERATED_DIR}/embedded_assets_editor.rc
    ${CMAKE_SOURCE_DIR}/assets/editor/logos
    ${CMAKE_SOURCE_DIR}/assets/editor/fonts
)

add_subdirectory(app/windows)