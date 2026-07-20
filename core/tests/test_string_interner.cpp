/// @file test_string_interner.cpp
/// @brief Unit tests and concurrency tests for StringInterner.

#include "ume/concurrency/string_interner.h"

#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace ume::concurrency {
namespace {

TEST(StringInternerTest, InternEmptyString) {
    StringInterner interner;
    InternedString h = interner.intern("");
    EXPECT_EQ(h, InternedString::kNull);
    EXPECT_EQ(interner.resolve(h), "");
}

TEST(StringInternerTest, InternDeduplication) {
    StringInterner interner;
    InternedString h1 = interner.intern("llama.weight.layer.0");
    InternedString h2 = interner.intern("llama.weight.layer.0");
    InternedString h3 = interner.intern("llama.weight.layer.1");

    EXPECT_NE(h1, InternedString::kNull);
    EXPECT_EQ(h1, h2);
    EXPECT_NE(h1, h3);
    EXPECT_EQ(interner.unique_string_count(), 2U);
}

TEST(StringInternerTest, ResolveHandle) {
    StringInterner interner;
    InternedString h = interner.intern("gpu_memory_vram");
    EXPECT_EQ(interner.resolve(h), "gpu_memory_vram");
}

TEST(StringInternerTest, ConcurrentInterning) {
    StringInterner interner;
    constexpr size_t kThreads = 8;
    constexpr size_t kStringsPerThread = 100;

    std::vector<std::thread> threads;
    for (size_t t = 0; t < kThreads; ++t) {
        threads.emplace_back([&interner, t]() {
            for (size_t i = 0; i < kStringsPerThread; ++i) {
                std::string s = "string_" + std::to_string(i); // Duplicate strings across threads
                InternedString h = interner.intern(s);
                EXPECT_NE(h, InternedString::kNull);
                EXPECT_EQ(interner.resolve(h), s);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(interner.unique_string_count(), kStringsPerThread);
}

} // namespace
} // namespace ume::concurrency
