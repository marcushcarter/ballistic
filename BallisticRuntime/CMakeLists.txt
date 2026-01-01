file(GLOB_RECURSE RUNTIME_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
if(WIN32)
    add_executable(BallisticRuntime WIN32 ${RUNTIME_SRC})
else()
    add_executable(BallisticRuntime ${RUNTIME_SRC})
endif()

target_include_directories(BallisticRuntime PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BallisticRuntime PRIVATE Ballistic)