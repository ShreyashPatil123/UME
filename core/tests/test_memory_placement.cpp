/// @file test_memory_placement.cpp
/// @brief Google Test suite for MemoryPlacementEngine (Milestone M4 Task T009).

#include "ume/event/memory_placement.h"

#include <gtest/gtest.h>

namespace ume::event {
namespace {

TEST(MemoryPlacementTest, CapacityValidation) {
    // RAM = 10MB, VRAM = 2MB, SSD = 20MB
    MemoryPlacementEngine engine(10 * 1024 * 1024, 2 * 1024 * 1024, 20 * 1024 * 1024);

    SchedulingDecision decision1{};
    decision1.object_id = ObjectId{1};
    decision1.allocation_size_bytes = 1 * 1024 * 1024;
    decision1.preferred_tier = TierClass::kRam;

    auto plan_res1 = engine.plan_placement(decision1);
    ASSERT_TRUE(plan_res1.ok());
    EXPECT_TRUE(plan_res1.value().is_valid);
    EXPECT_EQ(plan_res1.value().target_tier, TierClass::kRam);

    // Try VRAM allocation exceeding 2MB VRAM capacity
    SchedulingDecision decision2{};
    decision2.object_id = ObjectId{2};
    decision2.allocation_size_bytes = 3 * 1024 * 1024; // 3MB VRAM
    decision2.preferred_tier = TierClass::kVram;

    auto plan_res2 = engine.plan_placement(decision2);
    EXPECT_FALSE(plan_res2.ok());
    EXPECT_EQ(plan_res2.status(), Status::kCapacityExceeded);
}

TEST(MemoryPlacementTest, EnqueueDequeueAndDuplicates) {
    MemoryPlacementEngine engine(1024, 1024, 1024);

    SchedulingDecision decision{};
    decision.object_id = ObjectId{42};
    decision.allocation_size_bytes = 100;
    decision.preferred_tier = TierClass::kRam;

    auto req_id = engine.enqueue_request(decision);
    ASSERT_TRUE(req_id.ok());
    EXPECT_EQ(engine.queue_size(), 1U);

    // Duplicate enqueue must fail
    auto duplicate_res = engine.enqueue_request(decision);
    EXPECT_FALSE(duplicate_res.ok());
    EXPECT_EQ(duplicate_res.status(), Status::kAlreadyExists);

    auto deq_res = engine.dequeue_request();
    ASSERT_TRUE(deq_res.ok());
    EXPECT_EQ(deq_res.value().decision.object_id, ObjectId{42});
    EXPECT_EQ(engine.queue_size(), 0U);
}

} // namespace
} // namespace ume::event
