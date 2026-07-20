#pragma once

/// @file clock.h
/// @brief High-resolution monotonic clock abstraction.
///
/// Provides cross-platform nanosecond-precision monotonic timestamps.
/// This is the single source of time for all UME timestamps. Every event,
/// measurement, and prediction uses this clock.
///
/// Platform implementations:
/// - Linux:   clock_gettime(CLOCK_MONOTONIC_RAW)
/// - Windows: QueryPerformanceCounter / QueryPerformanceFrequency
/// - macOS:   clock_gettime(CLOCK_MONOTONIC_RAW)

#include "ume/platform/platform.h"
#include "ume/types.h"

#include <cstdint>

#if defined(UME_PLATFORM_LINUX) || defined(UME_PLATFORM_MACOS)
    #include <time.h>
#elif defined(UME_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif

namespace ume::platform {

/// @brief Returns the current monotonic time in nanoseconds.
///
/// This clock is:
/// - Monotonically increasing (never goes backwards)
/// - Not affected by NTP or manual time adjustments
/// - High resolution (nanosecond or better)
///
/// The epoch is unspecified (typically system boot). Only use for
/// relative time comparisons and duration calculations.
///
/// @return Monotonic timestamp in nanoseconds.
/// @complexity O(1), typically < 25 ns per call.
[[nodiscard]] inline Timestamp monotonic_now_ns() noexcept {
#if defined(UME_PLATFORM_LINUX) || defined(UME_PLATFORM_MACOS)
    struct timespec ts {};
    #if defined(CLOCK_MONOTONIC_RAW)
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    #else
    clock_gettime(CLOCK_MONOTONIC, &ts);
    #endif
    return static_cast<Timestamp>(static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL +
                                  static_cast<uint64_t>(ts.tv_nsec));
#elif defined(UME_PLATFORM_WINDOWS)
    static const uint64_t frequency = []() -> uint64_t {
        LARGE_INTEGER freq{};
        QueryPerformanceFrequency(&freq);
        return static_cast<uint64_t>(freq.QuadPart);
    }();

    LARGE_INTEGER counter{};
    QueryPerformanceCounter(&counter);

    const uint64_t count = static_cast<uint64_t>(counter.QuadPart);
    const uint64_t whole_seconds = count / frequency;
    const uint64_t remainder = count % frequency;
    return static_cast<Timestamp>(whole_seconds * 1'000'000'000ULL +
                                  (remainder * 1'000'000'000ULL) / frequency);
#endif
}

/// @brief Returns the raw nanosecond value from a Timestamp.
[[nodiscard]] constexpr uint64_t to_ns(Timestamp ts) noexcept {
    return static_cast<uint64_t>(ts);
}

/// @brief Returns the microsecond value from a Timestamp.
[[nodiscard]] constexpr uint64_t to_us(Timestamp ts) noexcept {
    return to_ns(ts) / 1'000ULL;
}

/// @brief Returns the millisecond value from a Timestamp.
[[nodiscard]] constexpr uint64_t to_ms(Timestamp ts) noexcept {
    return to_ns(ts) / 1'000'000ULL;
}

/// @brief Returns the seconds value from a Timestamp.
[[nodiscard]] constexpr double to_seconds(Timestamp ts) noexcept {
    return static_cast<double>(to_ns(ts)) / 1'000'000'000.0;
}

/// @brief Computes the elapsed time in nanoseconds between two timestamps.
///
/// @param start The earlier timestamp.
/// @param end The later timestamp.
/// @return Elapsed nanoseconds. If end < start (clock anomaly), returns 0.
[[nodiscard]] constexpr uint64_t elapsed_ns(Timestamp start, Timestamp end) noexcept {
    const uint64_t s = static_cast<uint64_t>(start);
    const uint64_t e = static_cast<uint64_t>(end);
    return (e >= s) ? (e - s) : 0;
}

/// @brief Computes elapsed time in microseconds.
[[nodiscard]] constexpr uint64_t elapsed_us(Timestamp start, Timestamp end) noexcept {
    const uint64_t s = static_cast<uint64_t>(start);
    const uint64_t e = static_cast<uint64_t>(end);
    return ((e >= s) ? (e - s) : 0) / 1'000ULL;
}

/// @brief Computes elapsed time in milliseconds.
[[nodiscard]] constexpr uint64_t elapsed_ms(Timestamp start, Timestamp end) noexcept {
    const uint64_t s = static_cast<uint64_t>(start);
    const uint64_t e = static_cast<uint64_t>(end);
    return ((e >= s) ? (e - s) : 0) / 1'000'000ULL;
}

} // namespace ume::platform
