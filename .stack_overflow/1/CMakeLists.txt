cmake_minimum_required(VERSION 3.28)
project(MRTRepro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_executable(mrt_repro main.cpp)

target_link_libraries(mrt_repro PRIVATE Vulkan::Vulkan)

target_include_directories(mrt_repro PRIVATE
    "$ENV{VULKAN_SDK}/Include"
    "$ENV{VULKAN_SDK}/Include/vma"
)

if (MSVC)
    target_compile_options(mrt_repro PRIVATE /W3)
endif()