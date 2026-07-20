/// @file test_memory_optimizer.cpp
/// @brief Google Test suite for MemoryOptimizer (Milestone M5 Task T010).

#include "ume/event/memory_optimizer.h"

#include <gtest/gtest.h>

namespace ume::event {
namespace {

TEST(MemoryOptimizerTest, RecordAccessAndTemperaturePromotion) {
    StatisticsCollector stats;
    MemoryPlacementEngine placement(1024, 1024, 1024);
    MemoryOptimizer optimizer(stats, placement);

    optimizer.record_access(ObjectId{1}, 512, TierClass::kRam);
    auto residency_res = optimizer.get_residency(ObjectId{1});
    ASSERT_TRUE(residency_res.ok());
    EXPECT_EQ(residency_res.value().current_tier, TierClass::kRam);
    EXPECT_GT(residency_res.value().temperature, 50.0);
}

TEST(MemoryOptimizerTest, AgingAndPromotionPlanPass) {
    StatisticsCollector stats;
    MemoryPlacementEngine placement(1000 * 1024 * 1024, 1000 * 1024 * 1024, 1000 * 1024 * 1024);
    MemoryOptimizer optimizer(stats, placement);

    // Warm up the object to hot temperature (90+) to trigger promotion to VRAM
    ObjectId obj{42};
    for (int i = 0; i < 5; ++i) {
        optimizer.record_access(obj, 1048576, TierClass::kRam); // RAM
    }

    optimizer.optimize_pass();

    // Verify placement queue received promotion task
    EXPECT_EQ(placement.queue_size(), 1U);
    auto deq_res = placement.dequeue_request();
    ASSERT_TRUE(deq_res.ok());
    EXPECT_EQ(deq_res.value().decision.object_id, obj);
    EXPECT_EQ(deq_res.value().decision.preferred_tier, TierClass::kVram);
}

} // namespace
} // namespace ume::event
