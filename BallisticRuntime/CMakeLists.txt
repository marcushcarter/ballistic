file(GLOB_RECURSE RUNTIME_SRC src/*.cpp)

if(RUNTIME_SRC)
    add_executable(BallisticRuntime ${RUNTIME_SRC})

    # Include the src folder so any internal headers can be found
    target_include_directories(BallisticRuntime PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(BallisticRuntime PRIVATE Ballistic)
endif()