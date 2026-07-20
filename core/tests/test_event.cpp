/// @file test_event.cpp
/// @brief Google Test suite for UME Event Model and Type System (Milestone M2 Task T001).

#include "ume/event/event.h"
#include "ume/event/event_types.h"

#include <gtest/gtest.h>
#include <type_traits>

namespace ume::event {
namespace {

TEST(EventTypesTest, EventIdValidation) {
    EventId null_id{};
    EXPECT_FALSE(null_id.is_valid());
    EXPECT_EQ(null_id.to_raw(), 0ULL);

    EventId valid_id{42};
    EXPECT_TRUE(valid_id.is_valid());
    EXPECT_EQ(valid_id.to_raw(), 42ULL);
    EXPECT_NE(valid_id, null_id);
}

TEST(EventTypesTest, EventCategoryNames) {
    EXPECT_EQ(category_name(EventCategory::kMemoryObject), "MemoryObject");
    EXPECT_EQ(category_name(EventCategory::kTierMigration), "TierMigration");
    EXPECT_EQ(category_name(EventCategory::kAccessPattern), "AccessPattern");
    EXPECT_EQ(category_name(EventCategory::kSystemTopology), "SystemTopology");
    EXPECT_EQ(category_name(EventCategory::kPrediction), "Prediction");
    EXPECT_EQ(category_name(EventCategory::kAdvisor), "Advisor");
    EXPECT_EQ(category_name(EventCategory::kPlugin), "Plugin");
    EXPECT_EQ(category_name(EventCategory::kUnknown), "Unknown");
}

TEST(EventTypesTest, EventTypeNames) {
    EXPECT_EQ(type_name(EventType::kObjectCreated), "ObjectCreated");
    EXPECT_EQ(type_name(EventType::kMigrationCompleted), "MigrationCompleted");
    EXPECT_EQ(type_name(EventType::kPageFault), "PageFault");
    EXPECT_EQ(type_name(EventType::kTierPressureHigh), "TierPressureHigh");
    EXPECT_EQ(type_name(EventType::kPredictionGenerated), "PredictionGenerated");
    EXPECT_EQ(type_name(EventType::kUnknown), "Unknown");
}

TEST(EventTypesTest, EventFlagsBitmask) {
    EventFlags flags = EventFlags::kSynchronous | EventFlags::kJournaled;
    EXPECT_TRUE(has_flag(flags, EventFlags::kSynchronous));
    EXPECT_TRUE(has_flag(flags, EventFlags::kJournaled));
    EXPECT_FALSE(has_flag(flags, EventFlags::kReplayed));

    flags = flags & ~EventFlags::kSynchronous;
    EXPECT_FALSE(has_flag(flags, EventFlags::kSynchronous));
    EXPECT_TRUE(has_flag(flags, EventFlags::kJournaled));
}

TEST(EventTest, MemoryLayoutInvariants) {
    static_assert(sizeof(EventHeader) == 64);
    static_assert(alignof(EventHeader) == 64);
    static_assert(std::is_trivially_copyable_v<EventHeader>);
    static_assert(std::is_standard_layout_v<EventHeader>);

    static_assert(sizeof(Event) == 128);
    static_assert(alignof(Event) == 64);
    static_assert(std::is_trivially_copyable_v<Event>);
    static_assert(std::is_standard_layout_v<Event>);

    SUCCEED();
}

TEST(EventTest, CreateObjectEventFactory) {
    constexpr EventId id{1001};
    constexpr Timestamp ts{5000000ULL};
    constexpr ObjectId obj{42};
    constexpr TierId tier{1};
    constexpr uint64_t size = 1048576; // 1 MB

    constexpr Event ev =
        Event::create_object_event(id, ts, EventType::kObjectCreated, obj, tier, size);

    EXPECT_EQ(ev.header.id, id);
    EXPECT_EQ(ev.header.timestamp, ts);
    EXPECT_EQ(ev.header.type, EventType::kObjectCreated);
    EXPECT_EQ(ev.header.category, EventCategory::kMemoryObject);
    EXPECT_EQ(ev.header.priority, EventPriority::kNormal);
    EXPECT_TRUE(has_flag(ev.header.flags, EventFlags::kJournaled));
    EXPECT_EQ(ev.header.object_id, obj);
    EXPECT_EQ(ev.header.source_tier, tier);
    EXPECT_EQ(ev.payload.object.size_bytes, size);
}

TEST(EventTest, CreateMigrationEventFactory) {
    constexpr EventId id{1002};
    constexpr Timestamp ts{6000000ULL};
    constexpr ObjectId obj{99};
    constexpr TierId src_tier{1};
    constexpr TierId tgt_tier{2};
    constexpr uint64_t bytes = 4096;

    constexpr Event ev = Event::create_migration_event(id, ts, EventType::kMigrationCompleted, obj,
                                                       src_tier, tgt_tier, bytes);

    EXPECT_EQ(ev.header.id, id);
    EXPECT_EQ(ev.header.timestamp, ts);
    EXPECT_EQ(ev.header.type, EventType::kMigrationCompleted);
    EXPECT_EQ(ev.header.category, EventCategory::kTierMigration);
    EXPECT_EQ(ev.header.priority, EventPriority::kHigh);
    EXPECT_EQ(ev.header.source_tier, src_tier);
    EXPECT_EQ(ev.header.target_tier, tgt_tier);
    EXPECT_EQ(ev.payload.migration.bytes_transferred, bytes);
}

} // namespace
} // namespace ume::event
