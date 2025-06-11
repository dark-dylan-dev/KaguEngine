file(GLOB_RECURSE CPP_SOURCES src/*.cpp)
file(GLOB_RECURSE CPP_MODULES src/*.ixx)

# Adding the executable
add_executable(KaguEngine)

target_sources(KaguEngine PRIVATE ${CPP_SOURCES})
target_sources(KaguEngine PRIVATE
        FILE_SET all_modules TYPE CXX_MODULES
        FILES ${CPP_MODULES}
)

# Adding external libraries
add_subdirectory(extern/glfw)
target_include_directories(KaguEngine PRIVATE extern/glfw/include)
target_link_libraries(KaguEngine PRIVATE glfw)
target_include_directories(KaguEngine PRIVATE extern/glm)
target_include_directories(KaguEngine PRIVATE extern/stb)
target_include_directories(KaguEngine PRIVATE extern/tinyobjloader)
target_include_directories(KaguEngine PRIVATE extern/ImGuiFork)

# Adding Vulkan
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found")
endif()

target_include_directories(KaguEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(KaguEngine PRIVATE Vulkan::Vulkan)

# Enforces C++23 on the target
target_compile_features(KaguEngine PRIVATE cxx_std_23)

# Linker options (to disable console window in Release build)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        set(SUBSYSTEM_LINKER_OPTIONS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    else()
        set(SUBSYSTEM_LINKER_OPTIONS "-mwindows")
    endif()
    target_link_options(KaguEngine PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
endif()