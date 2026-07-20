/// @file test_m7_pipeline_integration.cpp
/// @brief Google Test for complete integration pipeline driving MemoryAdvisor and DigitalTwin
/// strategy simulation from replayed journal events.

#include "ume/event/digital_twin.h"
#include "ume/event/event_analyzer.h"
#include "ume/event/event_dispatcher.h"
#include "ume/event/journal_writer.h"
#include "ume/event/memory_advisor.h"
#include "ume/event/replay_engine.h"

#include <filesystem>
#include <gtest/gtest.h>

namespace ume::event {
namespace {

class UmeM7PipelineIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = (std::filesystem::temp_directory_path() / "ume_m7_pipeline_test").string();
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    std::string test_dir_;
};

TEST_F(UmeM7PipelineIntegrationTest, EndToEndM7Pipeline) {
    // 1. Write allocation events to journal to mock memory activity
    JournalWriter writer;
    ASSERT_TRUE(writer.open(test_dir_, 1024 * 1024).ok());

    ObjectId obj{99};
    Event alloc_ev = Event::create_object_event(EventId{1}, Timestamp{1000ULL},
                                                EventType::kObjectCreated, obj, TierId{1}, 4096);

    ASSERT_TRUE(writer.append(alloc_ev).ok());
    ASSERT_TRUE(writer.close().ok());

    // 2. Setup System Components
    StatisticsCollector stats;
    EventAnalyzer analyzer(&stats);
    EventDispatcher dispatcher;
    dispatcher.subscribe(&analyzer);

    PredictionEngine predictor(nullptr, nullptr);
    MemoryAdvisor advisor(stats, predictor);
    DigitalTwinEngine twin(stats);

    // 3. Replay Journal to drive analyzer and statistics
    ReplayEngine replay;
    auto replay_res = replay.replay_directory(test_dir_, dispatcher);
    ASSERT_TRUE(replay_res.ok());

    // 4. Verify Advisor health report
    AdvisorReport report = advisor.generate_report();
    EXPECT_EQ(report.health_score, 100.0); // perfect health initially

    // 5. Run Twin Simulation Context
    SimulationContext ctx{};
    ctx.max_ram_bytes = 100 * 1024 * 1024;
    ctx.max_vram_bytes = 20 * 1024 * 1024;

    SimulationPlan p1{1, "RAM Placement", obj, TierClass::kRam};
    SimulationPlan p2{2, "VRAM Promotion", obj, TierClass::kVram};

    auto sim_res = twin.simulate_strategies(obj, 1024, ctx, {p1, p2});
    ASSERT_TRUE(sim_res.ok());
    EXPECT_EQ(sim_res.value().selected_plan_id,
              2U); // VRAM selected (2U) due to HBM bandwidth scoring!
}

} // namespace
} // namespace ume::event
