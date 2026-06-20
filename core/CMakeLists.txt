file(GLOB_RECURSE BALLISTIC_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(ballistic_core STATIC ${BALLISTIC_CORE_SOURCES})

target_include_directories(ballistic_core PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(ballistic_core PUBLIC
    ballistic_drivers
    ballistic_compiler_options
)