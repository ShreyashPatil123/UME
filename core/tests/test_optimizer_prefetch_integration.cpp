/// @file test_optimizer_prefetch_integration.cpp
/// @brief Google Test for complete integration: Optimizer, Prefetcher, Replay, Dispatcher, and
/// Placement Engine.

#include "ume/event/event_analyzer.h"
#include "ume/event/event_dispatcher.h"
#include "ume/event/journal_writer.h"
#include "ume/event/memory_optimizer.h"
#include "ume/event/prefetch_engine.h"
#include "ume/event/replay_engine.h"

#include <filesystem>
#include <gtest/gtest.h>

namespace ume::event {
namespace {

class OptimizerPrefetchIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = (std::filesystem::temp_directory_path() / "ume_m5_integration").string();
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    std::string test_dir_;
};

TEST_F(OptimizerPrefetchIntegrationTest, EndToEndM5Pipeline) {
    // 1. Write allocation and repeated access patterns to journal
    JournalWriter writer;
    ASSERT_TRUE(writer.open(test_dir_, 1024 * 1024).ok());

    ObjectId obj{7};
    Event alloc_ev = Event::create_object_event(EventId{1}, Timestamp{1000ULL},
                                                EventType::kObjectCreated, obj, TierId{1}, 4096);

    ASSERT_TRUE(writer.append(alloc_ev).ok());
    ASSERT_TRUE(writer.close().ok());

    // 2. Setup System Components
    StatisticsCollector stats;
    EventAnalyzer analyzer(&stats);
    EventDispatcher dispatcher;
    dispatcher.subscribe(&analyzer);

    MemoryPlacementEngine placement(100 * 1024 * 1024, 20 * 1024 * 1024, 1000 * 1024 * 1024);
    MemoryOptimizer optimizer(stats, placement);
    PrefetchEngine prefetcher(placement);

    // 3. Replay journal to drive analyzer
    ReplayEngine replay;
    auto replay_res = replay.replay_directory(test_dir_, dispatcher);
    ASSERT_TRUE(replay_res.ok());

    // 4. Record access events directly to drive Optimizer and Prefetcher
    optimizer.record_access(obj, 4096, TierClass::kRam);
    optimizer.record_access(obj, 4096, TierClass::kRam);
    optimizer.record_access(obj, 4096, TierClass::kRam);
    optimizer.record_access(obj, 4096, TierClass::kRam);
    optimizer.record_access(obj, 4096, TierClass::kRam);

    prefetcher.record_access(obj, 0x1000);
    prefetcher.record_access(obj, 0x2000);
    prefetcher.record_access(obj, 0x3000);

    // Check prefetcher stride detection enqueued target prefetch request
    EXPECT_EQ(prefetcher.queue_size(), 1U);

    // Run Optimizer pass
    optimizer.optimize_pass();

    // Verify placement queue holds requests from prefetch/optimizer runs
    EXPECT_GE(placement.queue_size(), 1U);
}

} // namespace
} // namespace ume::event
