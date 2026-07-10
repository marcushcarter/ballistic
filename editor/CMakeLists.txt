if(NOT BALLISTIC_BUILD_EDITOR)
    return()
endif()

file(GLOB_RECURSE BALLISTIC_EDITOR_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(ballistic_editor STATIC ${BALLISTIC_EDITOR_SOURCES})

target_include_directories(ballistic_editor
PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(ballistic_editor
PUBLIC
    ballistic_compiler_options
    ballistic_core
)