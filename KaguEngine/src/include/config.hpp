#pragma once

#include <string_view>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace Config {
    // --- Engine Info ---
    constexpr std::string_view engineName = "Kagu Engine";
    constexpr std::string_view engineVersion = "0.0.1";

    // --- Build Info ---
#ifdef NDEBUG
    constexpr bool isDebug = false;
    constexpr std::string_view buildType = "Release";
#else
    constexpr bool isDebug = true;
    constexpr std::string_view buildType = "Debug";
#endif

    constexpr std::string_view compileDate = __DATE__;
    constexpr std::string_view compileTime = __TIME__;

    // --- Platform & Architecture ---
#if defined(_WIN32)
    constexpr std::string_view platform = "Windows";
#elif defined(__APPLE__)
    constexpr std::string_view platform = "Apple";
#elif defined(__linux__)
    constexpr std::string_view platform = "Linux";
#elif defined(__unix__)
    constexpr std::string_view platform = "Unix";
#else
    constexpr std::string_view platform = "Unsupported";
#error Unsupported platform
#endif

#if defined(_M_X64) || defined(__x86_64__)
    constexpr std::string_view architecture = "x86_64";
    constexpr int platformBits = 64;
#elif defined(_M_IX86) || defined(__i386__)
    constexpr std::string_view architecture = "x86";
    constexpr int platformBits = 32;
#elif defined(__aarch64__)
    constexpr std::string_view architecture = "ARM64";
    constexpr int platformBits = 64;
#elif defined(__arm__)
    constexpr std::string_view architecture = "ARM";
    constexpr int platformBits = 32;
#else
    constexpr std::string_view architecture = "Unknown";
    constexpr int platformBits = 0;
#pragma message("Your architecture might not support the application.")
#endif

    // --- Compiler Info ---
#if defined(_MSC_VER)
    constexpr std::string_view compiler = "MSVC";
    constexpr int compilerVersion = _MSC_VER;
#elif defined(__clang__)
    constexpr std::string_view compiler = "Clang";
    constexpr int compilerVersion = (__clang_major__ * 100) + (__clang_minor__ * 10) + __clang_patchlevel__;
#elif defined(__GNUC__)
    constexpr std::string_view compiler = "GCC";
    constexpr int compilerVersion = (__GNUC__ * 100) + (__GNUC_MINOR__ * 10) + __GNUC_PATCHLEVEL__;
#else
    constexpr std::string_view compiler = "Unknown";
    constexpr int compilerVersion = 0;
#pragma message("Your compiler might not work with this application.")
#endif

    // --- C++ version ---
#if __cplusplus > 202302L
    // The official macro value for C++26 is not yet finalized.
    constexpr std::string_view cppStandard = "C++26 (Experimental)";
#elif __cplusplus == 202302L
    constexpr std::string_view cppStandard = "C++23";
#elif __cplusplus == 202002L
    constexpr std::string_view cppStandard = "C++20";
#else
    constexpr std::string_view cppStandard = "Pre-C++20";
#pragma message("Your C++ version might not support C++20 modules properly.")
#endif

    // --- Vulkan Specifics ---
    constexpr bool enableValidationLayers = isDebug;
    constexpr uint32_t vulkanApiVersion = VK_API_VERSION_1_3;

    // --- File Paths ---
    constexpr std::string_view assetPath = "assets/";
    constexpr std::string_view shaderPath = "assets/shaders/";

} // Namespace Config