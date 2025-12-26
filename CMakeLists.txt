cmake_minimum_required(VERSION 3.20)
project(BallisticProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Thirdparty/glfw)

add_library(glad STATIC Thirdparty/glad/src/glad.c)
target_include_directories(glad PUBLIC Thirdparty/glad/include)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE Thirdparty/glm)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE Thirdparty/stb)

file(GLOB IMGUI_CORE Thirdparty/imgui/*.cpp)
set(IMGUI_BACKENDS
    Thirdparty/imgui/backends/imgui_impl_glfw.cpp
    Thirdparty/imgui/backends/imgui_impl_opengl3.cpp
)
set(IMGUI_SRC ${IMGUI_CORE} ${IMGUI_BACKENDS})
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC Thirdparty/imgui Thirdparty/imgui/backends)
target_link_libraries(imgui PUBLIC glfw)

add_subdirectory(Thirdparty/entt)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE Thirdparty/json/single_include)

add_library(tinyfiledialogs STATIC Thirdparty/tinyfiledialogs/tinyfiledialogs.c)
target_include_directories(tinyfiledialogs PUBLIC Thirdparty/tinyfiledialogs)

add_subdirectory(Engine)
add_subdirectory(EngineRuntime)
add_subdirectory(EngineEditor)