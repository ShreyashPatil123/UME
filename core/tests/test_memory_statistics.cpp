/// @file test_memory_statistics.cpp
/// @brief Google Test suite for Memory Statistics Engine (Milestone M3 Task T007).

#include "ume/event/memory_statistics.h"

#include <gtest/gtest.h>

namespace ume::event {
namespace {

TEST(MemoryStatisticsTest, RecordAllocationsAndFrees) {
    StatisticsCollector collector;

    collector.record_allocation(TierClass::kRam, 1048576, 100);  // 1 MB RAM (PID 100)
    collector.record_allocation(TierClass::kVram, 2097152, 100); // 2 MB VRAM (PID 100)

    MemoryStatisticsSnapshot snap1 = collector.snapshot();
    EXPECT_EQ(snap1.current_ram_bytes, 1048576U);
    EXPECT_EQ(snap1.peak_ram_bytes, 1048576U);
    EXPECT_EQ(snap1.current_vram_bytes, 2097152U);
    EXPECT_EQ(snap1.peak_vram_bytes, 2097152U);
    EXPECT_EQ(snap1.total_allocations, 2U);
    EXPECT_EQ(snap1.active_allocations_count, 2U);

    collector.record_free(TierClass::kRam, 1048576, 500000, 100);
    MemoryStatisticsSnapshot snap2 = collector.snapshot();
    EXPECT_EQ(snap2.current_ram_bytes, 0U);
    EXPECT_EQ(snap2.peak_ram_bytes, 1048576U);
    EXPECT_EQ(snap2.total_frees, 1U);
    EXPECT_EQ(snap2.active_allocations_count, 1U);
    EXPECT_EQ(snap2.avg_lifetime_ns, 500000.0);

    collector.reset();
    MemoryStatisticsSnapshot snap3 = collector.snapshot();
    EXPECT_EQ(snap3.current_ram_bytes, 0U);
    EXPECT_EQ(snap3.total_allocations, 0U);
}

} // namespace
} // namespace ume::event
