cmake_minimum_required(VERSION 3.12)

# C++ 20 standard required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCES src/*.cpp)

# Adding the executable
add_executable(KaguEngine ${SOURCES})

# Adding GLFW
add_subdirectory(extern/glfw)
target_include_directories(KaguEngine PRIVATE extern/glfw/include)
target_link_libraries(KaguEngine PRIVATE glfw)

# Adding GLM
target_include_directories(KaguEngine PRIVATE extern/glm)

# Adding stb
target_include_directories(KaguEngine PRIVATE extern/stb)

# Adding tinyobjloader
target_include_directories(KaguEngine PRIVATE extern/tinyobjloader)

# Adding Vulkan
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found")
endif()

target_include_directories(KaguEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(KaguEngine PRIVATE Vulkan::Vulkan)