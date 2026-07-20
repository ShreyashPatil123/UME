/// @file test_m6_pipeline_integration.cpp
/// @brief Google Test for complete integration pipeline driving online learning and predictions
/// from replayed journal events.

#include "ume/event/event_analyzer.h"
#include "ume/event/event_dispatcher.h"
#include "ume/event/journal_writer.h"
#include "ume/event/pattern_learning.h"
#include "ume/event/prediction_engine.h"
#include "ume/event/replay_engine.h"

#include <filesystem>
#include <gtest/gtest.h>

namespace ume::event {
namespace {

class UmeM6PipelineIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = (std::filesystem::temp_directory_path() / "ume_m6_pipeline_test").string();
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    std::string test_dir_;
};

TEST_F(UmeM6PipelineIntegrationTest, EndToEndM6Pipeline) {
    // 1. Write allocation events to journal
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

    PatternLearningEngine learning;
    PredictionEngine predictor(nullptr, &learning);

    // 3. Replay Journal
    ReplayEngine replay;
    auto replay_res = replay.replay_directory(test_dir_, dispatcher);
    ASSERT_TRUE(replay_res.ok());

    // Verify replayed object analyzer tracking
    EXPECT_TRUE(analyzer.is_allocated(obj));

    // 4. Drive Learning with address access samples
    learning.learn_access(obj, 0x1000);
    learning.learn_access(obj, 0x2000);
    learning.learn_access(obj, 0x3000);
    learning.learn_access(obj, 0x4000);
    learning.learn_access(obj, 0x5000);

    // Verify pattern database captured the sequential stride
    auto pattern_res = learning.get_pattern(obj);
    ASSERT_TRUE(pattern_res.ok());
    EXPECT_EQ(pattern_res.value().last_stride, 4096);

    // 5. Query Prediction Engine
    auto pred_res = predictor.get_prediction(obj);
    ASSERT_TRUE(pred_res.ok());
    EXPECT_TRUE(pred_res.value().is_valid);
    EXPECT_EQ(pred_res.value().expected_tier,
              TierClass::kVram); // promoted due to sequential stride confidence
}

} // namespace
} // namespace ume::event
