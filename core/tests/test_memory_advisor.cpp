/// @file test_memory_advisor.cpp
/// @brief Google Test suite for MemoryAdvisor (Milestone M7 Task T014).

#include "ume/event/memory_advisor.h"

#include <gtest/gtest.h>

namespace ume::event {
namespace {

TEST(MemoryAdvisorTest, RegisterRuleAndHealthDeductions) {
    StatisticsCollector stats;
    PredictionEngine predictor;
    MemoryAdvisor advisor(stats, predictor);

    // Initial state has perfect health
    AdvisorReport r1 = advisor.generate_report();
    EXPECT_EQ(r1.health_score, 100.0);
    EXPECT_TRUE(r1.recommendations.empty());

    // Fake high RAM usage (95%) to trigger health deduction warnings
    stats.record_allocation(TierClass::kRam, 95 * 1024 * 1024);

    // Explicitly update stats peak to 100 MB via record and free to set peak_ram_bytes
    stats.record_allocation(TierClass::kRam, 5 * 1024 * 1024);
    stats.record_free(TierClass::kRam, 5 * 1024 * 1024);

    AdvisorReport r2 = advisor.generate_report();
    EXPECT_LT(r2.health_score, 100.0);
    ASSERT_FALSE(r2.recommendations.empty());
    EXPECT_EQ(r2.recommendations[0].severity, AdvisorSeverity::kWarning);
}

} // namespace
} // namespace ume::event
