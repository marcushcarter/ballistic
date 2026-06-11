cmake_minimum_required(VERSION 3.28)

set(RC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
set(H_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources.h")
set(RC_CONTENT "")
set(H_CONTENT "#pragma once\n\n")

macro(embed_resources PREFIX START_ID)
    set(_ID ${START_ID})
    foreach(_FILE ${ARGN})
        get_filename_component(_FNAME ${_FILE} NAME)
        string(TOUPPER ${_FNAME} _UPPER)
        string(REPLACE "." "_" _NAME ${_UPPER})
        string(REPLACE "-" "_" _NAME ${_NAME})
        string(APPEND RC_CONTENT "${_ID} RCDATA \"${_FILE}\"\n")
        string(APPEND H_CONTENT  "#define ${PREFIX}_${_NAME} ${_ID}\n")
        math(EXPR _ID "${_ID} + 1")
    endforeach()
endmacro()

file(GLOB_RECURSE IMAGES "${CMAKE_CURRENT_SOURCE_DIR}/images/*.png" "${CMAKE_CURRENT_SOURCE_DIR}/images/*.jpg" "${CMAKE_CURRENT_SOURCE_DIR}/images/*.jpeg")
file(GLOB_RECURSE FONTS  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.ttf" "${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.otf")
file(GLOB_RECURSE SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.geom"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.tesc"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.tese"
)

embed_resources(IMG 300 ${IMAGES})
embed_resources(FONT 500 ${FONTS})
embed_resources(SHADER 700 ${SHADERS})

file(WRITE ${RC_FILE} ${RC_CONTENT})
file(WRITE ${H_FILE} ${H_CONTENT})

set(BALLISTIC_ENGINE_RC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc" CACHE INTERNAL "Engine RC files")