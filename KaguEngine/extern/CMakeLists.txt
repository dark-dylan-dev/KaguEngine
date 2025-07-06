# Adding external libraries
add_subdirectory(glfw)
target_include_directories(KaguEngine PRIVATE glfw/include)
target_link_libraries(KaguEngine PRIVATE glfw)
target_include_directories(KaguEngine PRIVATE glm)
target_include_directories(KaguEngine PRIVATE stb)
target_include_directories(KaguEngine PRIVATE tinyobjloader)
target_include_directories(KaguEngine PRIVATE ImGuiFork)