set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
# set(GLFW_VULKAN_STATIC ON CACHE BOOL "" FORCE)
set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)

add_subdirectory(glfw)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

file(GLOB IMGUI_CORE imgui/*.cpp)
set(IMGUI_SRC ${IMGUI_CORE} imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_vulkan.cpp)
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC imgui ${Vulkan_INCLUDE_DIRS})
target_link_libraries(imgui PUBLIC glfw)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE vma/include ${Vulkan_INCLUDE_DIRS})

add_library(iconfontcppheaders INTERFACE)
target_include_directories(iconfontcppheaders INTERFACE iconfontcppheaders)

add_library(tinyfiledialogs STATIC tinyfiledialogs/tinyfiledialogs.c)
target_include_directories(tinyfiledialogs PUBLIC tinyfiledialogs)

add_library(tomlplusplus INTERFACE)
target_include_directories(tomlplusplus INTERFACE tomlplusplus/include)