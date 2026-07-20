/// @file test_spsc_queue.cpp
/// @brief Unit tests and concurrency tests for LockFreeSpscQueue.

#include "ume/concurrency/spsc_queue.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>

namespace ume::concurrency {
namespace {

TEST(SpscQueueTest, BasicPushPop) {
    LockFreeSpscQueue<int, 16> queue;
    EXPECT_TRUE(queue.empty());

    EXPECT_TRUE(queue.push(100));
    EXPECT_FALSE(queue.empty());

    int val = 0;
    EXPECT_TRUE(queue.pop(val));
    EXPECT_EQ(val, 100);
    EXPECT_TRUE(queue.empty());
}

TEST(SpscQueueTest, CapacityFull) {
    LockFreeSpscQueue<int, 4> queue;

    EXPECT_TRUE(queue.push(1));
    EXPECT_TRUE(queue.push(2));
    EXPECT_TRUE(queue.push(3));
    EXPECT_FALSE(queue.push(4)); // Queue full (capacity - 1 usable)

    int val = 0;
    EXPECT_TRUE(queue.pop(val));
    EXPECT_EQ(val, 1);

    EXPECT_TRUE(queue.push(4));
}

TEST(SpscQueueTest, ConcurrentProducerConsumer) {
    constexpr size_t kItems = 100000;
    LockFreeSpscQueue<size_t, 1024> queue;

    std::thread producer([&queue]() {
        for (size_t i = 1; i <= kItems; ++i) {
            while (!queue.push(i)) {
                std::this_thread::yield();
            }
        }
    });

    size_t expected = 1;
    while (expected <= kItems) {
        size_t val = 0;
        if (queue.pop(val)) {
            EXPECT_EQ(val, expected);
            expected++;
        } else {
            std::this_thread::yield();
        }
    }

    producer.join();
    EXPECT_EQ(expected - 1, kItems);
}

} // namespace
} // namespace ume::concurrency
