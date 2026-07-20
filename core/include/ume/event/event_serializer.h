#pragma once

/// @file event_serializer.h
/// @brief Serialization, deserialization, CRC32 calculation, and buffer validation.
///
/// Part of UME Milestone M2 (Tasks T002 & T003).

#include "ume/event/event.h"
#include "ume/status.h"

#include <cstddef>
#include <cstdint>

namespace ume::event {

/// @brief IEEE 802.3 CRC32 checksum calculation helper.
///
/// @param data Pointer to input bytes.
/// @param size Number of bytes.
/// @return 32-bit unsigned CRC checksum.
[[nodiscard]] uint32_t compute_crc32(const uint8_t* data, size_t size) noexcept;

/// @brief Event Serializer utility for binary zero-copy and FlatBuffers encoding.
class EventSerializer {
public:
    /// @brief Magic identifier for binary serialized event records.
    static constexpr uint32_t kEventBinaryMagic = 0x55455654; // "UEVT"

    /// @brief Serializes a 128-byte Event into a target memory buffer.
    ///
    /// @param event Event instance to serialize.
    /// @param out_buffer Output destination buffer.
    /// @param max_size Maximum available size in destination buffer.
    /// @return Result containing bytes written (128) or Status error.
    [[nodiscard]] static Result<size_t> serialize(const Event& event, uint8_t* out_buffer,
                                                  size_t max_size) noexcept;

    /// @brief Deserializes a binary buffer into an Event instance.
    ///
    /// @param buffer Input source buffer containing serialized event bytes.
    /// @param size Buffer size in bytes.
    /// @return Result containing deserialized Event or Status error.
    [[nodiscard]] static Result<Event> deserialize(const uint8_t* buffer, size_t size) noexcept;

    /// @brief Validates whether a buffer contains a valid serialized Event payload.
    ///
    /// @param buffer Source buffer pointer.
    /// @param size Buffer size in bytes.
    /// @return Status::kOk if valid, or appropriate error code.
    [[nodiscard]] static Result<void> validate(const uint8_t* buffer, size_t size) noexcept;
};

} // namespace ume::event
