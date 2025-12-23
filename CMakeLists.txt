cmake_minimum_required(VERSION 3.20)
project(BallisticProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/glfw)

add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE external/glm)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE external/stb)

file(GLOB IMGUI_CORE external/imgui/*.cpp)
set(IMGUI_BACKENDS
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
set(IMGUI_SRC ${IMGUI_CORE} ${IMGUI_BACKENDS})
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC external/imgui external/imgui/backends)
target_link_libraries(imgui PUBLIC glfw)

add_subdirectory(external/entt)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE external/json/single_include)

add_library(tinyfiledialogs STATIC external/tinyfiledialogs/tinyfiledialogs.c)
target_include_directories(tinyfiledialogs PUBLIC external/tinyfiledialogs)

add_subdirectory(Ballistic)
add_subdirectory(BallisticRuntime)
add_subdirectory(BallisticEditor)