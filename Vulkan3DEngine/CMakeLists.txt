cmake_minimum_required(VERSION 3.12)

# C++ 20 standard required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adding an executable
add_executable(Vulkan3DEngine "src/Vulkan3DEngine.cpp" "src/Vulkan3DEngine.hpp" "src/main.cpp" "src/EngineInit/VulkanInit.cpp" "src/EngineInit/VulkanDebugger.cpp" "src/EngineInit/VulkanSwapChain.cpp" "src/EngineInit/VulkanDeviceChoice.cpp" "src/EngineShaders/VulkanShaders.cpp" "src/EngineLoop/VulkanFrameRendering.cpp" "src/EngineInit/VulkanQueues.cpp" "src/EngineCleanup/VulkanCleanup.cpp" "src/EngineInit/VulkanValidationLayers.cpp")

# Adding GLFW
add_subdirectory(extern/glfw)
target_link_libraries(Vulkan3DEngine PRIVATE glfw)
target_include_directories(Vulkan3DEngine PRIVATE extern/glfw/include)

# Adding Vulkan
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found")
endif()

target_include_directories(Vulkan3DEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(Vulkan3DEngine PRIVATE Vulkan::Vulkan)