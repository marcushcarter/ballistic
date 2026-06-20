file(GLOB_RECURSE BALLISTIC_DRIVERS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(ballistic_drivers STATIC ${BALLISTIC_DRIVERS_SOURCES})
target_include_directories(ballistic_drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ballistic_drivers PUBLIC Vulkan::Vulkan ballistic_compiler_options)