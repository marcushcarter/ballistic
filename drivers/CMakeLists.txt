file(GLOB_RECURSE BALLISTIC_DRIVERS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(ballistic_drivers STATIC ${BALLISTIC_DRIVERS_SOURCES})

target_include_directories(ballistic_drivers
PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/generated
)

target_link_options(ballistic_drivers PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/ignore:4099>)

target_link_libraries(ballistic_drivers
PUBLIC
    ballistic_compiler_options
    Vulkan::Vulkan
    Vulkan::shaderc_combined
    imgui
    dwmapi
)