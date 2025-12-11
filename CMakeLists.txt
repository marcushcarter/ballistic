cmake_minimum_required(VERSION 3.20)
project(BallisticProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------- External libraries --------

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

# GLAD
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# Vulkan
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found! Make sure VULKAN_SDK env variable is set.")
endif()

# -------- Ballistic Engine --------

file(GLOB_RECURSE ENGINE_SRC
    Ballistic/src/Core/*.cpp
    Ballistic/src/Platform/*.cpp
)
add_library(Ballistic STATIC ${ENGINE_SRC})
target_include_directories(Ballistic PUBLIC
    Ballistic/src
    BallisticEditor/src
    BallisticRuntime/src
    ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(Ballistic PUBLIC glfw glad ${CMAKE_DL_LIBS})
target_precompile_headers(Ballistic PUBLIC Ballistic/src/bepch.h)

# -------- Ballistic Runtime --------

file(GLOB_RECURSE RUNTIME_SRC BallisticRuntime/src/*.cpp)
if(RUNTIME_SRC)
    add_executable(BallisticRuntime ${RUNTIME_SRC})
    target_link_libraries(BallisticRuntime PRIVATE Ballistic ${Vulkan_LIBRARIES})
endif()

# -------- Ballistic Editor --------

file(GLOB_RECURSE EDITOR_SRC BallisticEditor/src/*.cpp)
if(EDITOR_SRC)
    add_executable(BallisticEditor ${EDITOR_SRC})
    target_link_libraries(BallisticEditor PRIVATE Ballistic ${Vulkan_LIBRARIES})
endif()
