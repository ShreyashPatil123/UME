/// @file test_thread.cpp
/// @brief Unit tests for thread management platform abstraction.

#include "ume/platform/thread.h"

#include <gtest/gtest.h>
#include <thread>

namespace ume::platform {
namespace {

TEST(ThreadTest, GetHardwareConcurrency) {
    size_t cores = get_hardware_concurrency();
    EXPECT_GE(cores, 1U);
}

TEST(ThreadTest, SetCurrentThreadName) {
    Status s = set_current_thread_name("ume_test_thread");
    EXPECT_TRUE(is_ok(s));
}

TEST(ThreadTest, SetCurrentThreadNameNull) {
    Status s = set_current_thread_name(nullptr);
    EXPECT_EQ(s, Status::kInvalidArgument);
}

TEST(ThreadTest, SetCurrentThreadAffinityCore0) {
    Status s = set_current_thread_affinity(0);
    EXPECT_TRUE(is_ok(s));
}

TEST(ThreadTest, SetCurrentThreadAffinityInvalidCore) {
    size_t invalid_core = get_hardware_concurrency() + 100;
    Status s = set_current_thread_affinity(invalid_core);
    EXPECT_EQ(s, Status::kInvalidArgument);
}

TEST(ThreadTest, SetCurrentThreadPriority) {
    Status s1 = set_current_thread_priority(ThreadPriority::kNormal);
    EXPECT_TRUE(is_ok(s1));

    Status s2 = set_current_thread_priority(ThreadPriority::kHigh);
    EXPECT_TRUE(is_ok(s2));
}

} // namespace
} // namespace ume::platform
