/// @file test_platform.cpp
/// @brief Unit tests for UME platform utilities and clock abstractions.

#include "ume/platform/clock.h"
#include "ume/platform/platform.h"

#include <gtest/gtest.h>

namespace ume::platform {
namespace {

TEST(PlatformTest, PlatformDetection) {
    bool detected = false;
#if defined(UME_PLATFORM_WINDOWS) || defined(UME_PLATFORM_LINUX) || defined(UME_PLATFORM_MACOS)
    detected = true;
#endif
    EXPECT_TRUE(detected) << "No supported platform detected via UME_PLATFORM_* macros";
}

TEST(PlatformTest, CompilerDetection) {
    bool detected = false;
#if defined(UME_COMPILER_MSVC) || defined(UME_COMPILER_GCC) || defined(UME_COMPILER_CLANG)
    detected = true;
#endif
    EXPECT_TRUE(detected) << "No supported compiler detected via UME_COMPILER_* macros";
}

TEST(PlatformTest, CacheLineSize) {
    EXPECT_TRUE(UME_CACHE_LINE_SIZE == 64 || UME_CACHE_LINE_SIZE == 128)
        << "Expected cache line size to be 64 or 128, got " << UME_CACHE_LINE_SIZE;
}

TEST(PlatformTest, ClockMonotonicNowNsReturnsNonZero) {
    Timestamp t = monotonic_now_ns();
    EXPECT_GT(to_raw(t), 0ULL);
}

TEST(PlatformTest, ClockSequentialCallsMonotonicallyIncreasing) {
    Timestamp t1 = monotonic_now_ns();
    Timestamp t2 = monotonic_now_ns();
    EXPECT_GE(to_raw(t2), to_raw(t1));
}

TEST(PlatformTest, ClockElapsedNsWorksCorrectly) {
    Timestamp t1{1000};
    Timestamp t2{2500};

    EXPECT_EQ(elapsed_ns(t1, t2), 1500ULL);
}

TEST(PlatformTest, ClockConversions) {
    Timestamp t{2'000'000'000ULL}; // 2 seconds in ns

    EXPECT_EQ(to_us(t), 2'000'000ULL);
    EXPECT_EQ(to_ms(t), 2'000ULL);
    EXPECT_DOUBLE_EQ(to_seconds(t), 2.0);
}

TEST(PlatformTest, UmeAssert) {
    // Should not abort in debug or release mode when condition is true
    UME_ASSERT(true, "This assertion should not fail");
    SUCCEED();
}

} // namespace
} // namespace ume::platform
