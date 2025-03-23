cmake_minimum_required(VERSION 3.12)

# Spécifiez le standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ajoutez l'exécutable
add_executable(Vulkan3DEngine "src/Vulkan3DEngine.cpp" "src/Vulkan3DEngine.hpp" "src/main.cpp")

# Ajoutez GLFW
add_subdirectory(extern/glfw)
target_link_libraries(Vulkan3DEngine PRIVATE glfw)
target_include_directories(Vulkan3DEngine PRIVATE extern/glfw/include)

# Ajoutez Vulkan
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found")
endif()

target_include_directories(Vulkan3DEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(Vulkan3DEngine PRIVATE Vulkan::Vulkan)