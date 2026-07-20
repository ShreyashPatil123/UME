/// @file clock.cpp
/// @brief Implementation of platform-specific clock utilities.

#include "ume/platform/clock.h"

#if defined(UME_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif

namespace ume::platform {

#if defined(UME_PLATFORM_WINDOWS)
namespace {
/// @brief Pre-warms QueryPerformanceFrequency on Windows.
/// This ensures the first call to monotonic_now_ns doesn't have extra latency.
struct ClockInitializer {
    ClockInitializer() {
        LARGE_INTEGER freq{};
        QueryPerformanceFrequency(&freq);
    }
};

static ClockInitializer g_clock_initializer;
} // namespace
#endif

bool validate_clock_resolution() {
    // Measure minimum clock step
    Timestamp start = monotonic_now_ns();
    Timestamp next = start;

    // Spin until the clock ticks to find the minimum resolution
    for (int i = 0; i < 1000000; ++i) {
        next = monotonic_now_ns();
        if (to_raw(next) != to_raw(start)) {
            break;
        }
    }

    // Return true if the clock resolution is strictly better than 1000ns
    return (elapsed_ns(start, next) < 1000);
}

} // namespace ume::platform
