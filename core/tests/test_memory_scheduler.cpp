/// @file test_memory_scheduler.cpp
/// @brief Google Test suite for MemoryScheduler decision engine (Milestone M4 Task T008).

#include "ume/event/memory_scheduler.h"

#include <gtest/gtest.h>

namespace ume::event {
namespace {

TEST(MemorySchedulerTest, ScheduleDefaultRAM) {
    StatisticsCollector stats;
    EventAnalyzer analyzer(&stats);
    MemoryScheduler scheduler(stats, analyzer);

    // Default allocation goes to RAM
    auto decision = scheduler.schedule_allocation(ObjectId{1}, 1024);
    EXPECT_EQ(decision.preferred_tier, TierClass::kRam);
    EXPECT_EQ(decision.reason, DecisionReason::kDefaultPlacement);
}

TEST(MemorySchedulerTest, ScheduleHotVRAMPromotion) {
    StatisticsCollector stats;
    EventAnalyzer analyzer(&stats);
    MemoryScheduler scheduler(stats, analyzer);

    // Access count >= threshold (10) schedules VRAM
    auto decision = scheduler.schedule_allocation(ObjectId{2}, 1024, 15);
    EXPECT_EQ(decision.preferred_tier, TierClass::kVram);
    EXPECT_EQ(decision.reason, DecisionReason::kHotAccessVram);
}

TEST(MemorySchedulerTest, ScheduleSsdSpillOnPressure) {
    StatisticsCollector stats;
    EventAnalyzer analyzer(&stats);
    MemoryScheduler scheduler(stats, analyzer);

    // Fake high RAM pressure: Peak = 100 MB, Current = 90 MB (90% > 85% threshold)
    stats.record_allocation(TierClass::kRam, 90 * 1024 * 1024);

    // Explicitly update stats peak to 100 MB via record and free to set peak_ram_bytes
    stats.record_allocation(TierClass::kRam, 10 * 1024 * 1024);
    stats.record_free(TierClass::kRam, 10 * 1024 * 1024);

    auto decision = scheduler.schedule_allocation(ObjectId{3}, 1024);
    EXPECT_EQ(decision.preferred_tier, TierClass::kNvme);
    EXPECT_EQ(decision.reason, DecisionReason::kRamThresholdExceededSsdSpill);
}

} // namespace
} // namespace ume::event
