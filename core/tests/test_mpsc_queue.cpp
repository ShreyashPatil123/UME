/// @file test_mpsc_queue.cpp
/// @brief Unit tests and concurrency stress tests for LockFreeMpscQueue.

#include "ume/concurrency/mpsc_queue.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace ume::concurrency {
namespace {

TEST(MpscQueueTest, BasicEnqueueDequeue) {
    LockFreeMpscQueue<int, 16> queue;
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.capacity(), 16U);

    EXPECT_TRUE(queue.enqueue(42));
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.approx_size(), 1U);

    int val = 0;
    EXPECT_TRUE(queue.dequeue(val));
    EXPECT_EQ(val, 42);
    EXPECT_TRUE(queue.empty());
}

TEST(MpscQueueTest, CapacityFull) {
    LockFreeMpscQueue<int, 4> queue;

    EXPECT_TRUE(queue.enqueue(1));
    EXPECT_TRUE(queue.enqueue(2));
    EXPECT_TRUE(queue.enqueue(3));
    EXPECT_TRUE(queue.enqueue(4));

    // 5th enqueue should fail (queue full)
    EXPECT_FALSE(queue.enqueue(5));

    int val = 0;
    EXPECT_TRUE(queue.dequeue(val));
    EXPECT_EQ(val, 1);

    // Now enqueue should succeed
    EXPECT_TRUE(queue.enqueue(5));
}

TEST(MpscQueueTest, ConcurrentMultiProducerSingleConsumer) {
    constexpr size_t kQueueCapacity = 1024;
    constexpr size_t kNumProducers = 8;
    constexpr size_t kItemsPerProducer = 10000;
    constexpr size_t kTotalItems = kNumProducers * kItemsPerProducer;

    LockFreeMpscQueue<size_t, kQueueCapacity> queue;
    std::atomic<bool> start_signal{false};

    std::vector<std::thread> producers;
    for (size_t p = 0; p < kNumProducers; ++p) {
        producers.emplace_back([&queue, &start_signal, p]() {
            while (!start_signal.load(std::memory_order_relaxed)) {
            }
            for (size_t i = 0; i < kItemsPerProducer; ++i) {
                size_t val = p * kItemsPerProducer + i + 1;
                while (!queue.enqueue(val)) {
                    std::this_thread::yield();
                }
            }
        });
    }

    std::vector<bool> received(kTotalItems + 1, false);
    size_t dequeued_count = 0;

    start_signal.store(true, std::memory_order_release);

    while (dequeued_count < kTotalItems) {
        size_t val = 0;
        if (queue.dequeue(val)) {
            ASSERT_GE(val, 1U);
            ASSERT_LE(val, kTotalItems);
            ASSERT_FALSE(received[val]) << "Duplicate value dequeued: " << val;
            received[val] = true;
            dequeued_count++;
        } else {
            std::this_thread::yield();
        }
    }

    for (auto& t : producers) {
        t.join();
    }

    EXPECT_EQ(dequeued_count, kTotalItems);
}

} // namespace
} // namespace ume::concurrency
