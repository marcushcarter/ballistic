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

# GLM (header-only)
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE external/glm)

# ImGui
file(GLOB IMGUI_CORE external/imgui/*.cpp)
set(IMGUI_BACKENDS
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
set(IMGUI_SRC ${IMGUI_CORE} ${IMGUI_BACKENDS})
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC external/imgui external/imgui/backends)
target_link_libraries(imgui PUBLIC glfw)


# -------- Ballistic Engine --------

file(GLOB_RECURSE ENGINE_SRC
    Ballistic/src/*.cpp
    Ballistic/src/Core/*.cpp
    Ballistic/src/Platform/*.cpp
    Ballistic/src/Project/*.cpp
    Ballistic/src/Renderer/*.cpp
)

add_library(Ballistic STATIC ${ENGINE_SRC})

target_include_directories(Ballistic PUBLIC
    Ballistic/src
    BallisticEditor/src
    BallisticRuntime/src
)

target_link_libraries(Ballistic PUBLIC
    glfw
    glad
    glm
    imgui
    ${CMAKE_DL_LIBS}
)

target_precompile_headers(Ballistic PUBLIC Ballistic/src/bepch.h)

# -------- Ballistic Runtime --------

file(GLOB_RECURSE RUNTIME_SRC BallisticRuntime/src/*.cpp)
if(RUNTIME_SRC)
    add_executable(BallisticRuntime ${RUNTIME_SRC})
    target_link_libraries(BallisticRuntime PRIVATE Ballistic)
endif()

# -------- Ballistic Editor --------

file(GLOB_RECURSE EDITOR_SRC BallisticEditor/src/*.cpp)
if(EDITOR_SRC)
    add_executable(BallisticEditor ${EDITOR_SRC})

    # Windows icon
    if(WIN32)
        set(RUNTIME_ICON "${CMAKE_CURRENT_SOURCE_DIR}/Ballistic/res/icon.rc")
        target_sources(BallisticEditor PRIVATE ${RUNTIME_ICON})
    endif()

    target_link_libraries(BallisticEditor PRIVATE Ballistic)
endif()
