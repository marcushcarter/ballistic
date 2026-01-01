cmake_minimum_required(VERSION 3.20)
set(CMAKE_BUILD_TYPE Debug)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(BallisticProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Thirdparty)
add_subdirectory(Ballistic)
add_subdirectory(BallisticRuntime)
add_subdirectory(BallisticEditor)
