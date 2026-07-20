/// @file test_analysis_pipeline_integration.cpp
/// @brief Google Test for complete end-to-end pipeline: Journal -> Replay -> Deserialize ->
/// Dispatch -> EventAnalyzer -> Statistics Engine.

#include "ume/event/event_analyzer.h"
#include "ume/event/event_dispatcher.h"
#include "ume/event/journal_writer.h"
#include "ume/event/memory_statistics.h"
#include "ume/event/replay_engine.h"

#include <filesystem>
#include <gtest/gtest.h>

namespace ume::event {
namespace {

class AnalysisPipelineTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = (std::filesystem::temp_directory_path() / "ume_pipeline_test").string();
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    std::string test_dir_;
};

TEST_F(AnalysisPipelineTest, EndToEndReplayToAnalyzerPipeline) {
    // 1. Write allocation and free events to journal
    JournalWriter writer;
    ASSERT_TRUE(writer.open(test_dir_, 1024 * 1024).ok());

    Event alloc_ev1 =
        Event::create_object_event(EventId{1}, Timestamp{1000ULL}, EventType::kObjectCreated,
                                   ObjectId{100}, TierId{1}, 1048576); // 1 MB
    Event alloc_ev2 =
        Event::create_object_event(EventId{2}, Timestamp{2000ULL}, EventType::kObjectCreated,
                                   ObjectId{101}, TierId{1}, 2097152); // 2 MB
    Event free_ev1 =
        Event::create_object_event(EventId{3}, Timestamp{3000ULL}, EventType::kObjectDestroyed,
                                   ObjectId{100}, TierId{1}, 1048576);

    ASSERT_TRUE(writer.append(alloc_ev1).ok());
    ASSERT_TRUE(writer.append(alloc_ev2).ok());
    ASSERT_TRUE(writer.append(free_ev1).ok());
    ASSERT_TRUE(writer.close().ok());

    // 2. Setup Dispatcher, StatisticsCollector, and EventAnalyzer
    StatisticsCollector stats;
    EventAnalyzer analyzer(&stats);

    EventDispatcher dispatcher;
    dispatcher.subscribe(&analyzer);

    // 3. Replay Journal to Dispatcher
    ReplayEngine replay;
    auto replay_res = replay.replay_directory(test_dir_, dispatcher);
    ASSERT_TRUE(replay_res.ok());

    EXPECT_EQ(replay_res.value().events_replayed, 3U);

    // 4. Verify Analyzer and Statistics State
    AnalysisContext actx = analyzer.context();
    EXPECT_EQ(actx.total_events_processed, 3U);
    EXPECT_EQ(actx.active_allocations_count, 1U); // ObjectId 101 still allocated
    EXPECT_TRUE(analyzer.is_allocated(ObjectId{101}));
    EXPECT_FALSE(analyzer.is_allocated(ObjectId{100}));

    MemoryStatisticsSnapshot snap = stats.snapshot();
    EXPECT_EQ(snap.total_allocations, 2U);
    EXPECT_EQ(snap.total_frees, 1U);
    EXPECT_EQ(snap.current_ram_bytes, 2097152U); // 2 MB active
    EXPECT_EQ(snap.peak_ram_bytes, 3145728U);    // 3 MB peak
}

} // namespace
} // namespace ume::event
