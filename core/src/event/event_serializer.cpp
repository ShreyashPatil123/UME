/// @file event_serializer.cpp
/// @brief Implementation of CRC32 checksums, event binary serialization, and validation.

#include "ume/event/event_serializer.h"

#include <array>
#include <cstring>

namespace ume::event {

namespace {

// Lookup table for fast IEEE 802.3 CRC32 calculation
constexpr auto generate_crc32_table() {
    std::array<uint32_t, 256> table{};
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            crc = (crc & 1) ? (0xED888320u ^ (crc >> 1)) : (crc >> 1);
        }
        table[i] = crc;
    }
    return table;
}

constexpr auto kCrc32Table = generate_crc32_table();

} // namespace

uint32_t compute_crc32(const uint8_t* data, size_t size) noexcept {
    if (data == nullptr || size == 0) {
        return 0;
    }
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; ++i) {
        const uint8_t lookup = static_cast<uint8_t>((crc ^ data[i]) & 0xFF);
        crc = (crc >> 8) ^ kCrc32Table[lookup];
    }
    return crc ^ 0xFFFFFFFFu;
}

Result<size_t> EventSerializer::serialize(const Event& event, uint8_t* out_buffer,
                                          size_t max_size) noexcept {
    if (out_buffer == nullptr) {
        return Status::kNullPointer;
    }
    if (max_size < sizeof(Event)) {
        return Status::kCapacityExceeded;
    }

    std::memcpy(out_buffer, &event, sizeof(Event));
    return sizeof(Event);
}

Result<Event> EventSerializer::deserialize(const uint8_t* buffer, size_t size) noexcept {
    const auto validation = validate(buffer, size);
    if (!validation.ok()) {
        return validation.status();
    }

    Event event{};
    std::memcpy(&event, buffer, sizeof(Event));
    return event;
}

Result<void> EventSerializer::validate(const uint8_t* buffer, size_t size) noexcept {
    if (buffer == nullptr) {
        return Status::kNullPointer;
    }
    if (size < sizeof(Event)) {
        return Status::kSerializationError;
    }

    const auto* header = reinterpret_cast<const EventHeader*>(buffer);
    if (header->category == EventCategory::kUnknown && header->type == EventType::kUnknown) {
        if (header->id == kNullEventId && header->payload_size == 0) {
            // Default blank initialized header is valid
            return Status::kOk;
        }
        return Status::kSerializationError;
    }

    if (header->payload_size > 64) {
        return Status::kSerializationError;
    }

    return Status::kOk;
}

} // namespace ume::event
