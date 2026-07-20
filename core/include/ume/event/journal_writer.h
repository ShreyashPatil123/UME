#pragma once

/// @file journal_writer.h
/// @brief Append-only, thread-safe memory-mapped Journal Writer Engine.
///
/// Part of UME Milestone M2 (Task T003).

#include "ume/event/event.h"
#include "ume/event/event_serializer.h"
#include "ume/platform/mmap.h"
#include "ume/status.h"
#include "ume/types.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

namespace ume::event {

/// @brief Header placed at byte offset 0 of every binary journal segment file (.umej).
struct alignas(64) JournalSegmentHeader {
    uint32_t magic{0x554D454A};                    ///< "UMEJ" (0x554D454A)
    uint16_t version_major{1};                     ///< Major version 1
    uint16_t version_minor{0};                     ///< Minor version 0
    uint64_t segment_id{0};                        ///< Monotonic segment index
    Timestamp created_timestamp{Timestamp::kZero}; ///< Nanosecond timestamp
    uint64_t segment_size_bytes{0};                ///< Max capacity in bytes
    uint32_t header_crc32{0};                      ///< CRC32 of bytes 0..43
    uint8_t reserved[20]{0};                       ///< Padding to 64 bytes
};

static_assert(sizeof(JournalSegmentHeader) == 64, "JournalSegmentHeader must be 64 bytes");
static_assert(alignof(JournalSegmentHeader) == 64,
              "JournalSegmentHeader must be aligned to 64 bytes");
static_assert(std::is_trivially_copyable_v<JournalSegmentHeader>);

/// @brief Fixed header preceding each binary event record in the journal.
struct alignas(8) JournalRecordHeader {
    uint32_t magic{0x52454344}; ///< "RECD" (0x52454344)
    uint32_t record_size{0};    ///< Total size (Header + Event)
    uint64_t sequence_num{0};   ///< Monotonic global sequence number
    uint32_t payload_crc32{0};  ///< CRC32 of serialized Event
    uint32_t header_crc32{0};   ///< CRC32 of bytes 0..23
    uint8_t reserved[8]{0};     ///< Padding to 32 bytes
};

static_assert(sizeof(JournalRecordHeader) == 32, "JournalRecordHeader must be 32 bytes");
static_assert(alignof(JournalRecordHeader) == 8, "JournalRecordHeader must be 8-byte aligned");
static_assert(std::is_trivially_copyable_v<JournalRecordHeader>);

/// @brief Append-Only Memory-Mapped Journal Writer Engine.
class JournalWriter {
public:
    /// @brief Default segment size (64 MB).
    static constexpr uint64_t kDefaultSegmentSize = 64 * 1024 * 1024;

    JournalWriter() noexcept = default;
    ~JournalWriter();

    // Disable copy
    JournalWriter(const JournalWriter&) = delete;
    JournalWriter& operator=(const JournalWriter&) = delete;

    // Move semantics
    JournalWriter(JournalWriter&& other) noexcept;
    JournalWriter& operator=(JournalWriter&& other) noexcept;

    /// @brief Opens or creates the journal directory and starts the initial segment.
    ///
    /// @param journal_directory Directory where .umej segment files are saved.
    /// @param segment_size_bytes Target segment file size (minimum 1 MB).
    /// @return Status::kOk or failure status.
    Result<void> open(const std::string& journal_directory,
                      uint64_t segment_size_bytes = kDefaultSegmentSize) noexcept;

    /// @brief Appends an event to the active journal segment.
    ///
    /// Performs thread-safe atomic space reservation and binary event write.
    /// Automatically rolls over to a new segment file if capacity is reached.
    ///
    /// @param event Event instance to record.
    /// @return Result containing global sequence number or error.
    Result<uint64_t> append(const Event& event) noexcept;

    /// @brief Synchronizes all mapped memory pages to disk.
    Result<void> flush() noexcept;

    /// @brief Flushes pending writes and closes current segment file.
    Result<void> close() noexcept;

    /// @brief Returns true if journal writer is open and active.
    [[nodiscard]] bool is_open() const noexcept { return is_open_; }

    /// @brief Returns active segment index.
    [[nodiscard]] uint64_t active_segment_id() const noexcept { return current_segment_id_; }

    /// @brief Returns total count of events written across all segments.
    [[nodiscard]] uint64_t total_events_written() const noexcept {
        return total_events_written_.load(std::memory_order_relaxed);
    }

    /// @brief Returns total bytes written across all segments.
    [[nodiscard]] uint64_t total_bytes_written() const noexcept {
        return total_bytes_written_.load(std::memory_order_relaxed);
    }

private:
    Result<void> roll_segment_locked() noexcept;
    std::string build_segment_path(uint64_t segment_id) const;

    std::string directory_path_;
    uint64_t segment_size_bytes_{kDefaultSegmentSize};
    uint64_t current_segment_id_{0};

    platform::MmapFile current_mmap_;
    std::atomic<uint64_t> write_offset_{sizeof(JournalSegmentHeader)};
    std::atomic<uint64_t> sequence_counter_{0};
    std::atomic<uint64_t> total_events_written_{0};
    std::atomic<uint64_t> total_bytes_written_{0};

    mutable std::mutex rollover_mutex_;
    bool is_open_{false};
};

} // namespace ume::event
