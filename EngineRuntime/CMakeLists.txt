file(GLOB_RECURSE RUNTIME_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(EngineRuntime ${RUNTIME_SRC})

target_include_directories(EngineRuntime PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(EngineRuntime PRIVATE Engine)