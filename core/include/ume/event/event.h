#pragma once

/// @file event.h
/// @brief Fixed-size EventHeader, Event payloads, and cache-aligned Event structure.
///
/// Part of UME Milestone M2 (Event System & Journal).

#include "ume/event/event_types.h"
#include "ume/types.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ume::event {

/// @brief Fixed-size header present at the start of every UME event.
///
/// Layout is explicitly padded and aligned to 64 bytes (1 cache line).
/// Trivially copyable and standard layout for zero-copy binary journal I/O.
struct alignas(64) EventHeader {
    EventId id{kNullEventId};                        ///< 8 bytes (0-7)
    Timestamp timestamp{Timestamp::kZero};           ///< 8 bytes (8-15)
    ObjectId object_id{ObjectId::kNull};             ///< 8 bytes (16-23)
    EventType type{EventType::kUnknown};             ///< 2 bytes (24-25)
    EventCategory category{EventCategory::kUnknown}; ///< 1 byte (26)
    EventPriority priority{EventPriority::kNormal};  ///< 1 byte (27)
    EventFlags flags{EventFlags::kNone};             ///< 4 bytes (28-31)
    TierId source_tier{TierId::kInvalid};            ///< 2 bytes (32-33)
    TierId target_tier{TierId::kInvalid};            ///< 2 bytes (34-35)
    uint32_t payload_size{0};                        ///< 4 bytes (36-39)
    uint32_t sequence_num{0};                        ///< 4 bytes (40-43)
    uint8_t reserved[20]{0};                         ///< 20 bytes (44-63) - pad to 64 bytes
};

static_assert(sizeof(EventHeader) == 64, "EventHeader must be exactly 64 bytes");
static_assert(alignof(EventHeader) == 64, "EventHeader must be aligned to 64 bytes");
static_assert(std::is_trivially_copyable_v<EventHeader>, "EventHeader must be trivially copyable");
static_assert(std::is_standard_layout_v<EventHeader>, "EventHeader must be standard layout");

// ──────────────────────────────────────────────────────────────────────
// Specific Event Payloads (Trivially Copyable PODs)
// ──────────────────────────────────────────────────────────────────────

/// @brief Payload for kObjectCreated, kObjectDestroyed, kObjectResized.
struct ObjectEventPayload {
    uint64_t size_bytes{0};
    uint32_t alignment{0};
    TierClass preferred_tier{TierClass::kRam};
    uint8_t reserved[19]{0};
};
static_assert(std::is_trivially_copyable_v<ObjectEventPayload>);

/// @brief Payload for kMigrationRequested, kMigrationCompleted, kMigrationFailed.
struct MigrationEventPayload {
    uint64_t bytes_transferred{0};
    uint64_t duration_ns{0};
    uint32_t status_code{0};
    uint8_t reserved[12]{0};
};
static_assert(std::is_trivially_copyable_v<MigrationEventPayload>);

/// @brief Payload for kAccessSample, kPageFault.
struct AccessEventPayload {
    uint64_t virtual_address{0};
    AccessType access_type{AccessType::kRead};
    uint32_t access_count{0};
    uint8_t reserved[19]{0};
};
static_assert(std::is_trivially_copyable_v<AccessEventPayload>);

/// @brief Payload for kTierPressureHigh, kTierRegistered.
struct SystemEventPayload {
    uint64_t capacity_bytes{0};
    uint64_t used_bytes{0};
    uint32_t metric_value{0};
    uint8_t reserved[12]{0};
};
static_assert(std::is_trivially_copyable_v<SystemEventPayload>);

/// @brief Unified Event envelope combining fixed EventHeader and optional payload.
struct alignas(64) Event {
    EventHeader header{};
    union Payload {
        ObjectEventPayload object;
        MigrationEventPayload migration;
        AccessEventPayload access;
        SystemEventPayload system;
        uint8_t raw[64];

        constexpr Payload() noexcept : raw{0} {}
        explicit constexpr Payload(const ObjectEventPayload& obj) noexcept : object(obj) {}
        explicit constexpr Payload(const MigrationEventPayload& mig) noexcept : migration(mig) {}
        explicit constexpr Payload(const AccessEventPayload& acc) noexcept : access(acc) {}
        explicit constexpr Payload(const SystemEventPayload& sys) noexcept : system(sys) {}
    } payload{};

    constexpr Event() noexcept = default;

    /// @brief Factory helper to create a Memory Object event.
    [[nodiscard]] static constexpr Event create_object_event(EventId id, Timestamp ts,
                                                             EventType type, ObjectId obj_id,
                                                             TierId tier, uint64_t size) noexcept {
        Event e{};
        e.header.id = id;
        e.header.timestamp = ts;
        e.header.type = type;
        e.header.category = EventCategory::kMemoryObject;
        e.header.priority = EventPriority::kNormal;
        e.header.flags = EventFlags::kJournaled;
        e.header.object_id = obj_id;
        e.header.source_tier = tier;
        e.header.payload_size = sizeof(ObjectEventPayload);

        e.payload = Payload(ObjectEventPayload{size, 0, TierClass::kRam, {0}});
        return e;
    }

    /// @brief Factory helper to create a Migration event.
    [[nodiscard]] static constexpr Event create_migration_event(EventId id, Timestamp ts,
                                                                EventType type, ObjectId obj_id,
                                                                TierId src, TierId tgt,
                                                                uint64_t bytes) noexcept {
        Event e{};
        e.header.id = id;
        e.header.timestamp = ts;
        e.header.type = type;
        e.header.category = EventCategory::kTierMigration;
        e.header.priority = EventPriority::kHigh;
        e.header.flags = EventFlags::kJournaled;
        e.header.object_id = obj_id;
        e.header.source_tier = src;
        e.header.target_tier = tgt;
        e.header.payload_size = sizeof(MigrationEventPayload);

        e.payload = Payload(MigrationEventPayload{bytes, 0, 0, {0}});
        return e;
    }
};

static_assert(sizeof(Event) == 128, "Event must be exactly 128 bytes (2 cache lines)");
static_assert(alignof(Event) == 64, "Event must be aligned to 64 bytes");
static_assert(std::is_trivially_copyable_v<Event>, "Event must be trivially copyable");
static_assert(std::is_standard_layout_v<Event>, "Event must be standard layout");

} // namespace ume::event
