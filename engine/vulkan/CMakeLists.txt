file(GLOB_RECURSE VULKAN_SOURCES CONFIGURE_DEPENDS "*.cpp" "*.h")

add_library(ballistic_vulkan STATIC ${VULKAN_SOURCES})

target_include_directories(ballistic_vulkan
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ballistic_vulkan
    PUBLIC
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        vma
        ballistic_platform
)