cmake_minimum_required(VERSION 3.20)

# ---- Images ----

file(GLOB_RECURSE IMAGES
    "${CMAKE_CURRENT_SOURCE_DIR}/Images/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/Images/*.jpg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Images/*.jpeg"
)

set(RC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Images.rc")
set(H_FILE  "${CMAKE_CURRENT_SOURCE_DIR}/Images.h")
set(RC_CONTENT "")
set(H_CONTENT  "#pragma once\n\n")
set(IMAGE_ID   300)

foreach(IMAGE_FILE ${IMAGES})
    get_filename_component(FNAME ${IMAGE_FILE} NAME)
    string(TOUPPER ${FNAME} FNAME_UPPER)
    string(REPLACE "." "_" RES_NAME ${FNAME_UPPER})
    set(RES_NAME "IMG_${RES_NAME}")

    string(APPEND RC_CONTENT "${IMAGE_ID} RCDATA \"${IMAGE_FILE}\"\n")
    string(APPEND H_CONTENT "#define ${RES_NAME} ${IMAGE_ID}\n")
    math(EXPR IMAGE_ID "${IMAGE_ID} + 1")
endforeach()

file(WRITE ${RC_FILE} ${RC_CONTENT})
file(WRITE ${H_FILE}  ${H_CONTENT})

# ---- Fonts ----

file(GLOB_RECURSE FONTS
    "${CMAKE_CURRENT_SOURCE_DIR}/Fonts/*.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/Fonts/*.otf"
)

set(FONT_ID 500)

foreach(FONT_FILE ${FONTS})
    get_filename_component(FNAME ${FONT_FILE} NAME)
    string(TOUPPER ${FNAME} FNAME_UPPER)
    string(REPLACE "." "_" RES_NAME ${FNAME_UPPER})
    set(RES_NAME "FONT_${RES_NAME}")

    string(APPEND RC_CONTENT "${FONT_ID} RCDATA \"${FONT_FILE}\"\n")
    string(APPEND H_CONTENT "#define ${RES_NAME} ${FONT_ID}\n")
    math(EXPR FONT_ID "${FONT_ID} + 1")
endforeach()

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

    if(EXT_LOWER STREQUAL ".vert")
        set(STAGE_FLAG "-fshader-stage=vert")
    elseif(EXT_LOWER STREQUAL ".frag")
        set(STAGE_FLAG "-fshader-stage=frag")
    elseif(EXT_LOWER STREQUAL ".comp")
        set(STAGE_FLAG "-fshader-stage=comp")
    elseif(EXT_LOWER STREQUAL ".geom")
        set(STAGE_FLAG "-fshader-stage=geom")
    elseif(EXT_LOWER STREQUAL ".tesc")
        set(STAGE_FLAG "-fshader-stage=tesc")
    elseif(EXT_LOWER STREQUAL ".tese")
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
    "${CMAKE_CURRENT_SOURCE_DIR}/Images.rc"
    CACHE INTERNAL "Engine RC files"
)