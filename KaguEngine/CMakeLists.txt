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
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found")
endif()

target_include_directories(KaguEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(KaguEngine PRIVATE Vulkan::Vulkan)

# Linker options (to disable console window in Release build)
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    if(MSVC)
        target_link_options(KaguEngine PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
    else()
        target_link_options(KaguEngine PRIVATE -mwindows)
    endif()
endif()

# Copy the assets in the build directory
set(SOURCE_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(DEST_ASSETS_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_custom_command(
    OUTPUT ${DEST_ASSETS_DIR}/.copied
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SOURCE_ASSETS_DIR} ${DEST_ASSETS_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${DEST_ASSETS_DIR}/.copied
    COMMENT "Copying ${SOURCE_ASSETS_DIR} to ${DEST_ASSETS_DIR}"
)
add_custom_target(copy_assets ALL
    DEPENDS ${DEST_ASSETS_DIR}/.copied
)