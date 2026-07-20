/// @file test_event_analyzer.cpp
/// @brief Google Test suite for EventAnalyzer (Milestone M3 Task T006).

#include "ume/event/event_analyzer.h"

#include <gtest/gtest.h>

namespace ume::event {
namespace {

TEST(EventAnalyzerTest, ObjectLifecycleAndActiveMap) {
    StatisticsCollector collector;
    EventAnalyzer analyzer(&collector);

    Event create_ev = Event::create_object_event(
        EventId{1}, Timestamp{1000ULL}, EventType::kObjectCreated, ObjectId{42}, TierId{1}, 4096);

    analyzer.process_event(create_ev);

    EXPECT_TRUE(analyzer.is_allocated(ObjectId{42}));
    AnalysisContext ctx = analyzer.context();
    EXPECT_EQ(ctx.active_allocations_count, 1U);
    EXPECT_EQ(ctx.active_bytes, 4096U);
    EXPECT_EQ(ctx.peak_bytes, 4096U);

    Event destroy_ev = Event::create_object_event(
        EventId{2}, Timestamp{5000ULL}, EventType::kObjectDestroyed, ObjectId{42}, TierId{1}, 4096);

    analyzer.process_event(destroy_ev);

    EXPECT_FALSE(analyzer.is_allocated(ObjectId{42}));
    AnalysisContext ctx2 = analyzer.context();
    EXPECT_EQ(ctx2.active_allocations_count, 0U);
    EXPECT_EQ(ctx2.active_bytes, 0U);
    EXPECT_EQ(ctx2.peak_bytes, 4096U);
}

TEST(EventAnalyzerTest, OutOfOrderTimestampDetection) {
    EventAnalyzer analyzer;

    Event ev1 = Event::create_object_event(EventId{1}, Timestamp{5000ULL},
                                           EventType::kObjectCreated, ObjectId{1}, TierId{1}, 1024);
    Event ev2 = Event::create_object_event(EventId{2}, Timestamp{1000ULL},
                                           EventType::kObjectCreated, ObjectId{2}, TierId{1}, 1024);

    analyzer.process_event(ev1);
    analyzer.process_event(ev2);

    AnalysisContext ctx = analyzer.context();
    EXPECT_EQ(ctx.out_of_order_events, 1U);
}

} // namespace
} // namespace ume::event
