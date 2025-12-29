file(GLOB_RECURSE RUNTIME_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(BallisticRuntime ${RUNTIME_SRC})

target_include_directories(BallisticRuntime PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BallisticRuntime PRIVATE Ballistic)