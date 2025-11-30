file(GLOB_RECURSE CPP_SOURCES src/*.cpp)
file(GLOB_RECURSE CPP_MODULES src/*.ixx)

# Adding the executable
add_executable(KaguEngine)

# Enforces C++23 on the target
target_compile_features(KaguEngine PRIVATE cxx_std_23)

# Add source files & modules
target_sources(KaguEngine PRIVATE ${CPP_SOURCES})
target_sources(KaguEngine PRIVATE
    FILE_SET all_modules TYPE CXX_MODULES
    FILES ${CPP_MODULES}
)
set_target_properties(KaguEngine PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

# Third-party
add_subdirectory(extern)

# Adding Vulkan
find_package(Vulkan COMPONENTS glslangValidator REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found")
endif()

target_include_directories(KaguEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(KaguEngine PRIVATE Vulkan::Vulkan)

# Linker options (to disable console window on Windows in Release build)
if(WIN32 AND (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"))
    if(MSVC)
        target_link_options(KaguEngine PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
    else()
        target_link_options(KaguEngine PRIVATE -mwindows)
    endif()
endif()

# Copy the assets in the build directory
set(SOURCE_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(DEST_ASSETS_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets)
set(TEMP_ASSETS_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets_tmp)
add_custom_command(
    OUTPUT ${DEST_ASSETS_DIR}/.copied
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SOURCE_ASSETS_DIR} ${TEMP_ASSETS_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${TEMP_ASSETS_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${TEMP_ASSETS_DIR} ${DEST_ASSETS_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${TEMP_ASSETS_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${DEST_ASSETS_DIR}/.copied
    COMMENT "Copying assets"
)
add_custom_target(copy_assets ALL
    DEPENDS ${DEST_ASSETS_DIR}/.copied
)

# Compile shaders
set(SHADER_SRC_DIR ${SOURCE_ASSETS_DIR}/shaders)
set(SHADER_OUT_DIR ${DEST_ASSETS_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})
file(GLOB_RECURSE SHADERS
    ${SHADER_SRC_DIR}/*.vert
    ${SHADER_SRC_DIR}/*.frag
)
set(SPV_SHADERS "")
foreach(SH ${SHADERS})
    get_filename_component(FILENAME ${SH} NAME)
    set(SPV_OUTPUT ${SHADER_OUT_DIR}/${FILENAME}.spv)

    add_custom_command(
        OUTPUT ${SPV_OUTPUT}
        COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V ${SH} -o ${SPV_OUTPUT}
        DEPENDS ${SH}
        COMMENT "Compiling shader ${FILENAME}"
    )

    list(APPEND SPV_SHADERS ${SPV_OUTPUT})
endforeach()
add_custom_target(compile_shaders ALL
    DEPENDS ${SPV_SHADERS}
)

add_dependencies(KaguEngine copy_assets compile_shaders)