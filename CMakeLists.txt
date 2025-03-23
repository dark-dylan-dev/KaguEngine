# CMakeList.txt : fichier projet CMake de niveau supérieur, effectuez une configuration globale
cmake_minimum_required (VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)

# Activez Rechargement à chaud pour les compilateurs MSVC si cela est pris en charge.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Vulkan3DEngine" LANGUAGES CXX VERSION 1.0.0)

add_subdirectory ("Vulkan3DEngine")