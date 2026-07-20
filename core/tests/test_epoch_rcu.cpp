/// @file test_epoch_rcu.cpp
/// @brief Unit tests and multi-threaded stress tests for Epoch RCU.

#include "ume/concurrency/epoch_rcu.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace ume::concurrency {
namespace {

TEST(EpochRcuTest, BasicRetireAndReclaim) {
    EpochRcu rcu(100);

    bool deleted = false;
    rcu.retire([&deleted]() { deleted = true; });

    EXPECT_FALSE(deleted);
    size_t reclaimed = rcu.reclaim();
    EXPECT_GE(reclaimed, 1U);
    EXPECT_TRUE(deleted);
}

TEST(EpochRcuTest, ReaderHoldsEpochPreventsReclamation) {
    EpochRcu rcu(1000);

    bool deleted = false;
    size_t slot = rcu.register_thread();

    rcu.retire([&deleted]() { deleted = true; });
    rcu.enter(slot);

    // Reclaim while reader active
    rcu.reclaim();
    EXPECT_FALSE(deleted);

    rcu.exit(slot);

    // Reclaim after reader exits
    rcu.reclaim();
    EXPECT_TRUE(deleted);

    rcu.unregister_thread(slot);
}

TEST(EpochRcuTest, MultiThreadedRcuStress) {
    EpochRcu rcu(500);

    constexpr size_t kReaders = 4;
    constexpr size_t kDurationMs = 200;

    std::atomic<bool> running{true};
    std::atomic<uint64_t> read_count{0};
    std::atomic<uint64_t> retire_count{0};
    std::atomic<uint64_t> delete_count{0};

    // Reader threads
    std::vector<std::thread> readers;
    for (size_t i = 0; i < kReaders; ++i) {
        readers.emplace_back([&rcu, &running, &read_count]() {
            size_t slot = rcu.register_thread();
            while (running.load(std::memory_order_relaxed)) {
                rcu.enter(slot);
                read_count.fetch_add(1, std::memory_order_relaxed);
                rcu.exit(slot);
            }
            rcu.unregister_thread(slot);
        });
    }

    // Writer thread retiring objects
    std::thread writer([&rcu, &running, &retire_count, &delete_count]() {
        while (running.load(std::memory_order_relaxed)) {
            retire_count.fetch_add(1, std::memory_order_relaxed);
            rcu.retire([&delete_count]() { delete_count.fetch_add(1, std::memory_order_relaxed); });
            rcu.reclaim();
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(kDurationMs));
    running.store(false, std::memory_order_release);

    for (auto& t : readers)
        t.join();
    writer.join();

    // Final reclaim sweep
    rcu.reclaim();

    EXPECT_GT(read_count.load(), 1000ULL);
    EXPECT_GT(retire_count.load(), 0ULL);
}

} // namespace
} // namespace ume::concurrency
