/// @file test_scheduler_pipeline_integration.cpp
/// @brief Google Test for complete end-to-end integration: Journal -> Replay -> Dispatch ->
/// EventAnalyzer -> StatisticsCollector -> MemoryScheduler -> MemoryPlacementEngine.

#include "ume/event/event_analyzer.h"
#include "ume/event/event_dispatcher.h"
#include "ume/event/journal_writer.h"
#include "ume/event/memory_placement.h"
#include "ume/event/memory_scheduler.h"
#include "ume/event/memory_statistics.h"
#include "ume/event/replay_engine.h"

#include <filesystem>
#include <gtest/gtest.h>

namespace ume::event {
namespace {

class SchedulerPipelineTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ =
            (std::filesystem::temp_directory_path() / "ume_scheduler_pipeline_test").string();
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    std::string test_dir_;
};

TEST_F(SchedulerPipelineTest, EndToEndSchedulerPipeline) {
    // 1. Write allocation and access sample events to journal
    JournalWriter writer;
    ASSERT_TRUE(writer.open(test_dir_, 1024 * 1024).ok());

    Event alloc_ev =
        Event::create_object_event(EventId{1}, Timestamp{1000ULL}, EventType::kObjectCreated,
                                   ObjectId{55}, TierId{1}, 1048576); // 1 MB

    ASSERT_TRUE(writer.append(alloc_ev).ok());
    ASSERT_TRUE(writer.close().ok());

    // 2. Setup Pipeline Components
    StatisticsCollector stats;
    EventAnalyzer analyzer(&stats);
    EventDispatcher dispatcher;
    dispatcher.subscribe(&analyzer);

    MemoryScheduler scheduler(stats, analyzer);
    MemoryPlacementEngine placement_engine(100 * 1024 * 1024, 20 * 1024 * 1024, 1000 * 1024 * 1024);

    // 3. Replay Journal to Dispatcher
    ReplayEngine replay;
    auto replay_res = replay.replay_directory(test_dir_, dispatcher);
    ASSERT_TRUE(replay_res.ok());

    // 4. Drive scheduling and placement for object 55
    auto alloc_res = analyzer.get_allocation(ObjectId{55});
    ASSERT_TRUE(alloc_res.ok());
    ActiveAllocation alloc = alloc_res.value();

    // Access count set to 15 (exceeds hot threshold of 10)
    SchedulingDecision decision =
        scheduler.schedule_allocation(alloc.object_id, alloc.size_bytes, 15);
    EXPECT_EQ(decision.preferred_tier, TierClass::kVram);
    EXPECT_EQ(decision.reason, DecisionReason::kHotAccessVram);

    // Run placement planner
    auto plan_res = placement_engine.plan_placement(decision);
    ASSERT_TRUE(plan_res.ok());
    EXPECT_TRUE(plan_res.value().is_valid);
    EXPECT_EQ(plan_res.value().target_tier, TierClass::kVram);

    // Enqueue placement request
    auto req_id = placement_engine.enqueue_request(decision);
    ASSERT_TRUE(req_id.ok());
    EXPECT_EQ(placement_engine.queue_size(), 1U);
}

} // namespace
} // namespace ume::event
