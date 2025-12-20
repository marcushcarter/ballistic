file(GLOB_RECURSE RUNTIME_SRC src/*.cpp)

if(RUNTIME_SRC)
    add_executable(BallisticRuntime ${RUNTIME_SRC})

    target_include_directories(BallisticRuntime PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(BallisticRuntime PRIVATE Ballistic)
endif()