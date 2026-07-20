/// @file test_journal_writer.cpp
/// @brief Google Test suite for JournalWriter (Milestone M2 Task T003).

#include "ume/event/journal_writer.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace ume::event {
namespace {

class JournalWriterTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = (std::filesystem::temp_directory_path() / "ume_journal_test").string();
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    std::string test_dir_;
};

TEST_F(JournalWriterTest, OpenAndAppendEvent) {
    JournalWriter writer;
    auto open_res = writer.open(test_dir_, 1024 * 1024); // 1 MB segment
    ASSERT_TRUE(open_res.ok());
    EXPECT_TRUE(writer.is_open());

    Event ev = Event::create_object_event(EventId{1}, Timestamp{1000ULL}, EventType::kObjectCreated,
                                          ObjectId{10}, TierId{1}, 4096);

    auto append_res = writer.append(ev);
    ASSERT_TRUE(append_res.ok());
    EXPECT_EQ(append_res.value(), 1ULL);
    EXPECT_EQ(writer.total_events_written(), 1ULL);

    auto flush_res = writer.flush();
    EXPECT_TRUE(flush_res.ok());

    auto close_res = writer.close();
    EXPECT_TRUE(close_res.ok());
    EXPECT_FALSE(writer.is_open());
}

TEST_F(JournalWriterTest, MultiThreadedConcurrentAppends) {
    JournalWriter writer;
    ASSERT_TRUE(writer.open(test_dir_, 4 * 1024 * 1024).ok()); // 4 MB segment

    constexpr size_t kThreads = 4;
    constexpr size_t kEventsPerThread = 500;

    std::vector<std::thread> threads;
    for (size_t t = 0; t < kThreads; ++t) {
        threads.emplace_back([&writer, t]() {
            for (size_t i = 0; i < kEventsPerThread; ++i) {
                Event ev = Event::create_object_event(EventId{t * kEventsPerThread + i + 1},
                                                      Timestamp{100ULL}, EventType::kObjectCreated,
                                                      ObjectId{t + 1}, TierId{1}, 1024);
                EXPECT_TRUE(writer.append(ev).ok());
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(writer.total_events_written(), kThreads * kEventsPerThread);
    EXPECT_TRUE(writer.close().ok());
}

} // namespace
} // namespace ume::event
