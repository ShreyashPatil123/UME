/// @file test_replay_engine.cpp
/// @brief Google Test suite for ReplayEngine and end-to-end Event Pipeline Integration (Milestone
/// M2 Tasks T004 & T005).

#include "ume/event/event_dispatcher.h"
#include "ume/event/journal_writer.h"
#include "ume/event/replay_engine.h"

#include <atomic>
#include <filesystem>
#include <gtest/gtest.h>
#include <vector>

namespace ume::event {
namespace {

class ReplayEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = (std::filesystem::temp_directory_path() / "ume_replay_test").string();
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    std::string test_dir_;
};

TEST_F(ReplayEngineTest, JournalToReplayToDispatcherPipeline) {
    // 1. Write events to journal
    JournalWriter writer;
    ASSERT_TRUE(writer.open(test_dir_, 1024 * 1024).ok());

    constexpr size_t kEventsToWrite = 50;
    for (size_t i = 0; i < kEventsToWrite; ++i) {
        Event ev = Event::create_object_event(EventId{i + 1}, Timestamp{1000ULL + i},
                                              EventType::kObjectCreated, ObjectId{i + 10},
                                              TierId{1}, 4096);
        ASSERT_TRUE(writer.append(ev).ok());
    }
    ASSERT_TRUE(writer.close().ok());

    // 2. Setup Dispatcher and Subscriber
    EventDispatcher dispatcher;
    std::atomic<size_t> replayed_count{0};

    dispatcher.subscribe([&replayed_count](const Event& ev) {
        replayed_count++;
        EXPECT_TRUE(has_flag(ev.header.flags, EventFlags::kReplayed));
    });

    // 3. Replay journal directory
    ReplayEngine replay;
    auto replay_res = replay.replay_directory(test_dir_, dispatcher);
    ASSERT_TRUE(replay_res.ok());

    ReplayStats stats = replay_res.value();
    EXPECT_EQ(stats.events_replayed, kEventsToWrite);
    EXPECT_EQ(replayed_count.load(), kEventsToWrite);
    EXPECT_EQ(stats.corrupted_records_skipped, 0U);
}

} // namespace
} // namespace ume::event
