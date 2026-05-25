cmake_minimum_required(VERSION 3.20)

# ---- Images / Fonts ----

set(RC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Resources.rc")
set(H_FILE  "${CMAKE_CURRENT_SOURCE_DIR}/Resources.h")
set(RC_CONTENT "")
set(H_CONTENT "#pragma once\n\n")

set(IMAGE_ID 300)
set(FONT_ID 500)

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

file(GLOB_RECURSE IMAGES "${CMAKE_CURRENT_SOURCE_DIR}/Images/*.png" "${CMAKE_CURRENT_SOURCE_DIR}/Images/*.jpg" "${CMAKE_CURRENT_SOURCE_DIR}/Images/*.jpeg")
file(GLOB_RECURSE FONTS  "${CMAKE_CURRENT_SOURCE_DIR}/Fonts/*.ttf" "${CMAKE_CURRENT_SOURCE_DIR}/Fonts/*.otf")

embed_resources(IMG  300 ${IMAGES})
embed_resources(FONT 500 ${FONTS})

file(WRITE ${RC_FILE} ${RC_CONTENT})
file(WRITE ${H_FILE}  ${H_CONTENT})

# ---- Shaders ----

file(GLOB_RECURSE SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.comp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.geom"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.tesc"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.tese"
)

set(SPIRV_FILES)
foreach(SHADER_FILE ${SHADERS})
    get_filename_component(FNAME ${SHADER_FILE} NAME)
    set(OUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FNAME}.spv")

    get_filename_component(EXT ${SHADER_FILE} EXT)
    string(TOLOWER ${EXT} EXT_LOWER)

    if (EXT_LOWER STREQUAL ".vert")
        set(STAGE_FLAG "-fshader-stage=vert")
    elseif (EXT_LOWER STREQUAL ".frag")
        set(STAGE_FLAG "-fshader-stage=frag")
    elseif (EXT_LOWER STREQUAL ".comp")
        set(STAGE_FLAG "-fshader-stage=comp")
    elseif (EXT_LOWER STREQUAL ".geom")
        set(STAGE_FLAG "-fshader-stage=geom")
    elseif (EXT_LOWER STREQUAL ".tesc")
        set(STAGE_FLAG "-fshader-stage=tesc")
    elseif (EXT_LOWER STREQUAL ".tese")
        set(STAGE_FLAG "-fshader-stage=tese")
    else()
        message(FATAL_ERROR "[Shader] Unknown stage for ${SHADER_FILE}")
            endif()

    add_custom_command(
        OUTPUT ${OUT_FILE}
        COMMAND glslc ${STAGE_FLAG} ${SHADER_FILE} -o ${OUT_FILE} --target-env=vulkan1.3 -O
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling ${FNAME}"
        VERBATIM
    )
    list(APPEND SPIRV_FILES ${OUT_FILE})
endforeach()

set(SHADERS_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/Shaders.h")
list(JOIN SPIRV_FILES "|" SPIRV_FILES_STR)

add_custom_command(
    OUTPUT ${SHADERS_HEADER}
    COMMAND ${CMAKE_COMMAND}
        -DSPIRV_FILES=${SPIRV_FILES_STR}
        -DOUTPUT_FILE=${SHADERS_HEADER}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/GenerateShadersHeader.cmake"
    DEPENDS ${SPIRV_FILES}
    COMMENT "Generating Shaders.h"
    VERBATIM
)

add_custom_target(CompileEngineShaders ALL DEPENDS ${SHADERS_HEADER})

set(BALLISTIC_ENGINE_RC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources.rc"
    CACHE INTERNAL "Engine RC files"
)