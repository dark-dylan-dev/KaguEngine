function(CompileShaders SHADER_DIR OUTPUT_DIR TARGET)
    file(GLOB SHADERS
            "${SHADER_DIR}/point_light.vert"
            "${SHADER_DIR}/point_light.frag"
            "${SHADER_DIR}/with_textures.vert"
            "${SHADER_DIR}/with_textures.frag"
            "${SHADER_DIR}/without_textures.vert"
            "${SHADER_DIR}/without_textures.frag"
    )

    set(COMPILED_SHADERS "")

    foreach(shader ${SHADERS})
        get_filename_component(name ${shader} NAME)
        set(out "${OUTPUT_DIR}/${name}.spv")

        add_custom_command(
                OUTPUT  ${out}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
                COMMAND glslc ${shader} -o ${out}
                DEPENDS ${shader}
                COMMENT "Compiling shader ${name}"
                VERBATIM
        )

        list(APPEND COMPILED_SHADERS ${out})
    endforeach()

    add_custom_target(compile_shaders DEPENDS ${COMPILED_SHADERS})
    add_dependencies(${TARGET} compile_shaders)
endfunction()