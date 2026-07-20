/// @file test_digital_twin.cpp
/// @brief Google Test suite for DigitalTwinEngine (Milestone M7 Task T015).

#include "ume/event/digital_twin.h"

#include <gtest/gtest.h>

namespace ume::event {
namespace {

TEST(DigitalTwinTest, StrategySelectionAndCapacityPenalty) {
    StatisticsCollector stats;
    DigitalTwinEngine twin(stats);

    SimulationContext ctx{};
    ctx.current_ram_usage = 10 * 1024 * 1024;
    ctx.current_vram_usage = 8 * 1024 * 1024;
    ctx.max_ram_bytes = 100 * 1024 * 1024;
    ctx.max_vram_bytes = 10 * 1024 * 1024; // strictly limited VRAM (10MB)

    // Plan A: promote to VRAM (size 3MB) -> overcapacity (8+3 > 10)
    SimulationPlan planA{};
    planA.plan_id = 1;
    planA.name = "Plan A: Promote to VRAM";
    planA.target_tier = TierClass::kVram;

    // Plan B: keep in RAM (size 3MB) -> fits easily
    SimulationPlan planB{};
    planB.plan_id = 2;
    planB.name = "Plan B: Keep in RAM";
    planB.target_tier = TierClass::kRam;

    std::vector<SimulationPlan> candidates = {planA, planB};

    auto res = twin.simulate_strategies(ObjectId{1}, 3 * 1024 * 1024, ctx, candidates);
    ASSERT_TRUE(res.ok());
    EXPECT_EQ(res.value().selected_plan_id, 2U); // Plan B selected due to VRAM capacity penalty!
}

} // namespace
} // namespace ume::event
