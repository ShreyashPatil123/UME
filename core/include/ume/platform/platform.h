#pragma once

/// @file platform.h
/// @brief Platform detection macros.
///
/// Detects the operating system, compiler, and architecture at compile time.
/// All platform-specific code uses these macros to select implementations.

// ──────────────────────────────────────────────────────────────────────
// Operating System Detection
// ──────────────────────────────────────────────────────────────────────

#if defined(_WIN32) || defined(_WIN64)
    #define UME_PLATFORM_WINDOWS 1
    #define UME_PLATFORM_NAME "Windows"
#elif defined(__linux__)
    #define UME_PLATFORM_LINUX 1
    #define UME_PLATFORM_NAME "Linux"
#elif defined(__APPLE__) && defined(__MACH__)
    #define UME_PLATFORM_MACOS 1
    #define UME_PLATFORM_NAME "macOS"
#else
    #error "UME: Unsupported platform. Only Windows, Linux, and macOS are supported."
#endif

// ──────────────────────────────────────────────────────────────────────
// Compiler Detection
// ──────────────────────────────────────────────────────────────────────

#if defined(_MSC_VER)
    #define UME_COMPILER_MSVC 1
    #define UME_COMPILER_NAME "MSVC"
    #define UME_COMPILER_VERSION _MSC_VER
#elif defined(__clang__)
    #define UME_COMPILER_CLANG 1
    #define UME_COMPILER_NAME "Clang"
    #define UME_COMPILER_VERSION \
        (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined(__GNUC__)
    #define UME_COMPILER_GCC 1
    #define UME_COMPILER_NAME "GCC"
    #define UME_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
    #error "UME: Unsupported compiler. Only MSVC, Clang, and GCC are supported."
#endif

// ──────────────────────────────────────────────────────────────────────
// Architecture Detection
// ──────────────────────────────────────────────────────────────────────

#if defined(__x86_64__) || defined(_M_X64)
    #define UME_ARCH_X86_64 1
    #define UME_ARCH_NAME "x86_64"
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define UME_ARCH_ARM64 1
    #define UME_ARCH_NAME "arm64"
#else
    #define UME_ARCH_OTHER 1
    #define UME_ARCH_NAME "unknown"
#endif

// ──────────────────────────────────────────────────────────────────────
// Cache Line Size
// ──────────────────────────────────────────────────────────────────────

#if defined(UME_ARCH_X86_64)
    /// @brief Hardware destructive interference size (cache line size).
    ///
    /// Used for alignas() to prevent false sharing in concurrent data structures.
    /// 64 bytes on x86_64, 128 bytes on ARM64 (Apple M-series uses 128-byte lines).
    #define UME_CACHE_LINE_SIZE 64
#elif defined(UME_ARCH_ARM64)
    #define UME_CACHE_LINE_SIZE 128
#else
    #define UME_CACHE_LINE_SIZE 64
#endif

// ──────────────────────────────────────────────────────────────────────
// Build Configuration
// ──────────────────────────────────────────────────────────────────────

#if defined(UME_DEBUG)
    #define UME_BUILD_TYPE "Debug"
#elif defined(NDEBUG)
    #define UME_BUILD_TYPE "Release"
#else
    #define UME_BUILD_TYPE "Unknown"
#endif

// ──────────────────────────────────────────────────────────────────────
// Compiler Hints
// ──────────────────────────────────────────────────────────────────────

#if defined(UME_COMPILER_MSVC)
    #define UME_FORCEINLINE __forceinline
    #define UME_NOINLINE __declspec(noinline)
    #define UME_LIKELY(x) (x)
    #define UME_UNLIKELY(x) (x)
    #define UME_RESTRICT __restrict
#else
    /// @brief Force inline hint for hot-path functions.
    #define UME_FORCEINLINE __attribute__((always_inline)) inline
    /// @brief Prevent inlining for cold-path functions.
    #define UME_NOINLINE __attribute__((noinline))
    /// @brief Branch prediction hint: condition is likely true.
    #define UME_LIKELY(x) __builtin_expect(!!(x), 1)
    /// @brief Branch prediction hint: condition is likely false.
    #define UME_UNLIKELY(x) __builtin_expect(!!(x), 0)
    /// @brief Restrict pointer aliasing hint.
    #define UME_RESTRICT __restrict__
#endif

// ──────────────────────────────────────────────────────────────────────
// Debug Assertions
// ──────────────────────────────────────────────────────────────────────

/// @brief Debug-only assertion. Compiles to nothing in release builds.
///
/// Unlike standard assert(), UME_ASSERT provides a custom message and
/// is guaranteed to be removed in NDEBUG builds without side effects.
#if defined(UME_DEBUG)
    #include <cstdio>
    #include <cstdlib>
    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
    #define UME_ASSERT(condition, msg)                                                        \
        do {                                                                                  \
            if (UME_UNLIKELY(!(condition))) {                                                 \
                std::fprintf(stderr, "UME_ASSERT failed: %s\n  at %s:%d\n  %s\n", #condition, \
                             __FILE__, __LINE__, (msg));                                      \
                std::abort();                                                                 \
            }                                                                                 \
        } while (false)
#else
    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
    #define UME_ASSERT(condition, msg) ((void)0)
#endif

// ──────────────────────────────────────────────────────────────────────
// DLL Export / Import
// ──────────────────────────────────────────────────────────────────────

#if defined(UME_PLATFORM_WINDOWS)
    #if defined(UME_BUILDING_DLL)
        /// @brief Export symbol from shared library.
        #define UME_API __declspec(dllexport)
    #elif defined(UME_USING_DLL)
        /// @brief Import symbol from shared library.
        #define UME_API __declspec(dllimport)
    #else
        #define UME_API
    #endif
#else
    #if defined(UME_BUILDING_DLL)
        #define UME_API __attribute__((visibility("default")))
    #else
        #define UME_API
    #endif
#endif
