/// @file test_prefetch_engine.cpp
/// @brief Google Test suite for PrefetchEngine (Milestone M5 Task T011).

#include "ume/event/prefetch_engine.h"

#include <gtest/gtest.h>

namespace ume::event {
namespace {

TEST(PrefetchEngineTest, SequentialAccessPatternDetection) {
    MemoryPlacementEngine placement(1024, 1024, 1024);
    PrefetchEngine engine(placement);

    ObjectId obj{10};
    // Sequential addresses: delta = 4096 (1 page)
    engine.record_access(obj, 0x1000);
    engine.record_access(obj, 0x2000);
    engine.record_access(obj, 0x3000);

    // Stride delta detected -> PrefetchRequest enqueued!
    EXPECT_EQ(engine.queue_size(), 1U);
}

TEST(PrefetchEngineTest, PrefetchCancellation) {
    MemoryPlacementEngine placement(1024, 1024, 1024);
    PrefetchEngine engine(placement);

    ObjectId obj{20};
    engine.record_access(obj, 0x1000);
    engine.record_access(obj, 0x2000);
    engine.record_access(obj, 0x3000);

    EXPECT_EQ(engine.queue_size(), 1U);

    engine.cancel_prefetch(obj);
    EXPECT_EQ(engine.queue_size(), 0U);
}

} // namespace
} // namespace ume::event
