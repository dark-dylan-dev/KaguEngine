cmake_minimum_required(VERSION 3.7)

# C++ 20 standard required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCES src/*.cpp)

# Adding the executable
add_executable(KaguEngine ${SOURCES})

# Adding external libraries
add_subdirectory(extern/glfw)
target_include_directories(KaguEngine PRIVATE extern/glfw/include)
target_link_libraries(KaguEngine PRIVATE glfw)
target_include_directories(KaguEngine PRIVATE extern/glm)
target_include_directories(KaguEngine PRIVATE extern/stb)
target_include_directories(KaguEngine PRIVATE extern/tinyobjloader)
target_include_directories(KaguEngine PRIVATE extern/imgui)

# Adding Vulkan
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found")
endif()

target_include_directories(KaguEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(KaguEngine PRIVATE Vulkan::Vulkan)