add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

file(GLOB IMGUI_CORE imgui/*.cpp)
file(GLOB IMPLOT_CORE implot/*.cpp)
set(IMGUI_SRC ${IMGUI_CORE} imgui/backends/imgui_impl_win32.cpp imgui/backends/imgui_impl_vulkan.cpp ${IMPLOT_CORE})
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC imgui implot ${Vulkan_INCLUDE_DIRS})

add_library(IconFontCppHeaders INTERFACE)
target_include_directories(IconFontCppHeaders INTERFACE IconFontCppHeaders)

# add_library(tomlplusplus INTERFACE)
# target_include_directories(tomlplusplus INTERFACE tomlplusplus/include)