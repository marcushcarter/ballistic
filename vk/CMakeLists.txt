cmake_minimum_required(VERSION 3.25)

file(GLOB_RECURSE VK_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(vk STATIC ${VK_SOURCES})

target_include_directories(vk PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(vk PUBLIC cxx_std_20)

find_package(Vulkan REQUIRED)
target_link_libraries(vk PUBLIC
    Vulkan::Vulkan
    Vulkan::shaderc_combined
    vma
    glfw
)

target_precompile_headers(vk PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/pch.h"
)