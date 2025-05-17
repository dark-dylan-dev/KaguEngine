cmake_minimum_required(VERSION 3.12)

# C++ 20 standard required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adding an executable
add_executable(KaguEngine src/main.cpp)
target_sources(KaguEngine
        PRIVATE
        src/KaguEngine.hpp
        src/KaguEngine.cpp
        src/EngineInit/VulkanInit.cpp
        src/EngineInit/VulkanDebugger.cpp
        src/EngineInit/VulkanDeviceChoice.cpp
        src/EngineInit/VulkanQueues.cpp
        src/EngineInit/VulkanSwapChain.cpp
        src/EngineInit/VulkanValidationLayers.cpp
        src/EngineInit/VulkanVertexBuffer.cpp
        src/EngineShaders/VulkanShaders.cpp
        src/EngineTextures/VulkanTextureGestion.cpp
        src/EngineLoop/VulkanFrameRendering.cpp
        src/EngineCleanup/VulkanCleanup.cpp
)

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