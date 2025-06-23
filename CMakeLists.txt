cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# C++20 modules
include(CMake/SetModuleUUID.cmake)
setModuleID()

# Default build set to release
if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "." FORCE)
endif()

project(KaguEngine VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(KaguEngine)