#pragma once

/// @file journal_reader.h
/// @brief Sequential, memory-mapped journal segment reader and record validator.
///
/// Part of UME Milestone M2 (Task T004).

#include "ume/event/event.h"
#include "ume/event/event_serializer.h"
#include "ume/event/journal_writer.h"
#include "ume/platform/mmap.h"
#include "ume/status.h"
#include "ume/types.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ume::event {

/// @brief Statistics gathered during journal segment reading.
struct JournalReaderStats {
    uint64_t segments_read{0};
    uint64_t records_read{0};
    uint64_t bytes_read{0};
    uint64_t corrupted_records_skipped{0};
};

/// @brief Sequential reader for binary .umej journal segment files.
class JournalReader {
public:
    JournalReader() noexcept = default;
    ~JournalReader() = default;

    // Disable copy
    JournalReader(const JournalReader&) = delete;
    JournalReader& operator=(const JournalReader&) = delete;

    // Move semantics
    JournalReader(JournalReader&&) noexcept = default;
    JournalReader& operator=(JournalReader&&) noexcept = default;

    /// @brief Opens a single journal segment file (.umej) for reading.
    ///
    /// @param segment_path Path to segment file.
    /// @return Status::kOk if header is valid, or error status.
    Result<void> open_segment(const std::string& segment_path) noexcept;

    /// @brief Discovers and opens all .umej segments in a directory in sorted order.
    ///
    /// @param journal_directory Directory path containing .umej files.
    /// @return Status::kOk if directory has valid segments, or error status.
    Result<void> open_directory(const std::string& journal_directory) noexcept;

    /// @brief Reads the next valid Event from the open journal.
    ///
    /// Validates record header, magic, payload CRC32, and header CRC32.
    /// Safely skips corrupted bytes/records until valid record or EOF.
    ///
    /// @param out_event Output reference to store deserialized Event.
    /// @return Status::kOk if event read, Status::kNotFound at end of segment/journal, or error.
    Result<void> read_next(Event& out_event) noexcept;

    /// @brief Closes current open segment files and resets reader state.
    void close() noexcept;

    /// @brief Returns true if reader has open segments and is active.
    [[nodiscard]] bool is_open() const noexcept { return is_open_; }

    /// @brief Returns reading statistics.
    [[nodiscard]] const JournalReaderStats& stats() const noexcept { return stats_; }

private:
    Result<void> load_current_segment() noexcept;

    std::vector<std::string> segment_paths_;
    size_t current_segment_index_{0};
    platform::MmapFile current_mmap_;
    uint64_t current_offset_{sizeof(JournalSegmentHeader)};
    JournalSegmentHeader current_segment_header_{};
    JournalReaderStats stats_{};
    bool is_open_{false};
};

} // namespace ume::event
