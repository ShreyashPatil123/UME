/// @file test_event_dispatcher.cpp
/// @brief Google Test suite for EventDispatcher and Subscription System (Milestone M2 Task T005).

#include "ume/event/event_dispatcher.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace ume::event {
namespace {

class MockEventListener : public IEventListener {
public:
    void on_event(const Event& event) noexcept override {
        received_count.fetch_add(1);
        last_event_id = event.header.id;
    }

    std::atomic<size_t> received_count{0};
    EventId last_event_id{kNullEventId};
};

TEST(EventDispatcherTest, SubscribeAndDispatch) {
    EventDispatcher dispatcher;
    MockEventListener listener;

    auto token = dispatcher.subscribe(&listener);
    EXPECT_NE(token, SubscriptionToken::kInvalid);
    EXPECT_EQ(dispatcher.subscriber_count(), 1U);

    Event ev = Event::create_object_event(EventId{42}, Timestamp{100ULL}, EventType::kObjectCreated,
                                          ObjectId{1}, TierId{1}, 1024);

    dispatcher.dispatch(ev);
    EXPECT_EQ(listener.received_count.load(), 1U);
    EXPECT_EQ(listener.last_event_id, EventId{42});

    auto unsub_res = dispatcher.unsubscribe(token);
    EXPECT_TRUE(unsub_res.ok());
    EXPECT_EQ(dispatcher.subscriber_count(), 0U);

    dispatcher.dispatch(ev);
    EXPECT_EQ(listener.received_count.load(), 1U); // Not received after unsubscribe
}

TEST(EventDispatcherTest, CategoryAndTypeFiltering) {
    EventDispatcher dispatcher;
    std::atomic<size_t> object_events{0};
    std::atomic<size_t> migration_events{0};

    // Filter only MemoryObject category events
    dispatcher.subscribe(
        [&object_events](const Event&) { object_events++; },
        EventFilter{EventCategory::kMemoryObject, EventType::kUnknown, EventPriority::kLow});

    // Filter only TierMigration category events
    dispatcher.subscribe(
        [&migration_events](const Event&) { migration_events++; },
        EventFilter{EventCategory::kTierMigration, EventType::kUnknown, EventPriority::kLow});

    Event obj_ev = Event::create_object_event(
        EventId{1}, Timestamp{100ULL}, EventType::kObjectCreated, ObjectId{1}, TierId{1}, 1024);
    Event mig_ev =
        Event::create_migration_event(EventId{2}, Timestamp{200ULL}, EventType::kMigrationCompleted,
                                      ObjectId{1}, TierId{1}, TierId{2}, 1024);

    dispatcher.dispatch(obj_ev);
    EXPECT_EQ(object_events.load(), 1U);
    EXPECT_EQ(migration_events.load(), 0U);

    dispatcher.dispatch(mig_ev);
    EXPECT_EQ(object_events.load(), 1U);
    EXPECT_EQ(migration_events.load(), 1U);
}

TEST(EventDispatcherTest, ConcurrentMultiThreadedDispatch) {
    EventDispatcher dispatcher;
    std::atomic<size_t> total_calls{0};

    dispatcher.subscribe([&total_calls](const Event&) { total_calls++; });

    constexpr size_t kThreads = 4;
    constexpr size_t kDispatchesPerThread = 1000;

    std::vector<std::thread> threads;
    for (size_t t = 0; t < kThreads; ++t) {
        threads.emplace_back([&dispatcher, t]() {
            for (size_t i = 0; i < kDispatchesPerThread; ++i) {
                Event ev = Event::create_object_event(EventId{t * kDispatchesPerThread + i + 1},
                                                      Timestamp{100ULL}, EventType::kObjectCreated,
                                                      ObjectId{1}, TierId{1}, 1024);
                dispatcher.dispatch(ev);
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    EXPECT_EQ(total_calls.load(), kThreads * kDispatchesPerThread);
}

} // namespace
} // namespace ume::event
