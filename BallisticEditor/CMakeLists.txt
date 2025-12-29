file(GLOB_RECURSE EDITOR_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(BallisticEditor ${EDITOR_SRC})

if(WIN32)
    set(EDITOR_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons/icon.rc")
    target_sources(BallisticEditor PRIVATE ${EDITOR_ICON})
endif()

target_include_directories(BallisticEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BallisticEditor PRIVATE Ballistic)

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/Resources")

add_custom_command(TARGET BallisticEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${RESOURCE_DIR}"
    $<TARGET_FILE_DIR:BallisticEditor>/Resources
    COMMENT "Copying Resources for BallisticEditor"
)