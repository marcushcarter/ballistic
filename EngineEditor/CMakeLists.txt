file(GLOB_RECURSE EDITOR_SRC src/*.cpp)

if(EDITOR_SRC)
    add_executable(EngineEditor ${EDITOR_SRC})

    if(WIN32)
        set(RUNTIME_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")
        target_sources(EngineEditor PRIVATE ${RUNTIME_ICON})
    endif()

    target_include_directories(EngineEditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(EngineEditor PRIVATE Engine)
endif()
