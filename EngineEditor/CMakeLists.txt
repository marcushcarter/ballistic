file(GLOB_RECURSE EDITOR_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Panels/*.cpp
)

add_executable(EngineEditor ${EDITOR_SRC})

if(WIN32)
    set(EDITOR_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons/icon.rc")
    target_sources(EngineEditor PRIVATE ${EDITOR_ICON})
endif()

target_include_directories(EngineEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(EngineEditor PRIVATE Engine)
