/// @file journal_writer.cpp
/// @brief Implementation of append-only memory-mapped JournalWriter engine.

#include "ume/event/journal_writer.h"

#include "ume/platform/clock.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace ume::event {

JournalWriter::~JournalWriter() {
    (void)close();
}

JournalWriter::JournalWriter(JournalWriter&& other) noexcept {
    std::lock_guard<std::mutex> lock(other.rollover_mutex_);
    directory_path_ = std::move(other.directory_path_);
    segment_size_bytes_ = other.segment_size_bytes_;
    current_segment_id_ = other.current_segment_id_;
    current_mmap_ = std::move(other.current_mmap_);
    write_offset_.store(other.write_offset_.load());
    sequence_counter_.store(other.sequence_counter_.load());
    total_events_written_.store(other.total_events_written_.load());
    total_bytes_written_.store(other.total_bytes_written_.load());
    is_open_ = other.is_open_;
    other.is_open_ = false;
}

JournalWriter& JournalWriter::operator=(JournalWriter&& other) noexcept {
    if (this != &other) {
        (void)close();
        std::lock_guard<std::mutex> lock_other(other.rollover_mutex_);
        std::lock_guard<std::mutex> lock_this(rollover_mutex_);
        directory_path_ = std::move(other.directory_path_);
        segment_size_bytes_ = other.segment_size_bytes_;
        current_segment_id_ = other.current_segment_id_;
        current_mmap_ = std::move(other.current_mmap_);
        write_offset_.store(other.write_offset_.load());
        sequence_counter_.store(other.sequence_counter_.load());
        total_events_written_.store(other.total_events_written_.load());
        total_bytes_written_.store(other.total_bytes_written_.load());
        is_open_ = other.is_open_;
        other.is_open_ = false;
    }
    return *this;
}

std::string JournalWriter::build_segment_path(uint64_t segment_id) const {
    std::ostringstream ss;
    ss << "segment_" << std::setfill('0') << std::setw(6) << segment_id << ".umej";
    std::filesystem::path dir(directory_path_);
    return (dir / ss.str()).string();
}

Result<void> JournalWriter::open(const std::string& journal_directory,
                                 uint64_t segment_size_bytes) noexcept {
    std::lock_guard<std::mutex> lock(rollover_mutex_);
    if (is_open_) {
        return Status::kAlreadyInitialized;
    }
    if (segment_size_bytes < 1024 * 1024) {
        return Status::kInvalidArgument;
    }

    directory_path_ = journal_directory;
    segment_size_bytes_ = segment_size_bytes;
    current_segment_id_ = 0;

    std::error_code ec;
    std::filesystem::create_directories(directory_path_, ec);

    const auto roll_status = roll_segment_locked();
    if (!roll_status.ok()) {
        return roll_status;
    }

    is_open_ = true;
    return Status::kOk;
}

Result<void> JournalWriter::roll_segment_locked() noexcept {
    if (current_mmap_.is_mapped()) {
        (void)current_mmap_.flush();
        current_mmap_.close();
    }

    const std::string path = build_segment_path(current_segment_id_);
    const Status map_res = current_mmap_.open_read_write(path, segment_size_bytes_);
    if (is_error(map_res)) {
        return map_res;
    }

    // Initialize Segment Header
    JournalSegmentHeader header{};
    header.magic = 0x554D454A; // "UMEJ"
    header.version_major = 1;
    header.version_minor = 0;
    header.segment_id = current_segment_id_;
    header.created_timestamp = platform::monotonic_now_ns();
    header.segment_size_bytes = segment_size_bytes_;
    header.header_crc32 = compute_crc32(reinterpret_cast<const uint8_t*>(&header), 32);

    std::memcpy(current_mmap_.data(), &header, sizeof(JournalSegmentHeader));
    write_offset_.store(sizeof(JournalSegmentHeader), std::memory_order_release);

    return Status::kOk;
}

Result<uint64_t> JournalWriter::append(const Event& event) noexcept {
    if (!is_open_) {
        return Status::kNotInitialized;
    }

    constexpr uint32_t record_payload_size = sizeof(Event);
    constexpr uint32_t total_record_size = sizeof(JournalRecordHeader) + record_payload_size;

    uint64_t offset = write_offset_.fetch_add(total_record_size, std::memory_order_acq_rel);

    // Check if rollover required
    if (offset + total_record_size > segment_size_bytes_) {
        std::lock_guard<std::mutex> lock(rollover_mutex_);
        // Re-check inside lock
        if (write_offset_.load(std::memory_order_relaxed) + total_record_size >
            segment_size_bytes_) {
            current_segment_id_++;
            const auto roll_res = roll_segment_locked();
            if (!roll_res.ok()) {
                return roll_res.status();
            }
        }
        offset = write_offset_.fetch_add(total_record_size, std::memory_order_acq_rel);
    }

    const uint64_t seq = sequence_counter_.fetch_add(1, std::memory_order_relaxed) + 1;

    // Build Record Header
    JournalRecordHeader record_hdr{};
    record_hdr.magic = 0x52454344; // "RECD"
    record_hdr.record_size = total_record_size;
    record_hdr.sequence_num = seq;
    record_hdr.payload_crc32 =
        compute_crc32(reinterpret_cast<const uint8_t*>(&event), sizeof(Event));
    record_hdr.header_crc32 = compute_crc32(reinterpret_cast<const uint8_t*>(&record_hdr), 20);

    uint8_t* dst = current_mmap_.data() + offset;
    std::memcpy(dst, &record_hdr, sizeof(JournalRecordHeader));
    std::memcpy(dst + sizeof(JournalRecordHeader), &event, sizeof(Event));

    total_events_written_.fetch_add(1, std::memory_order_relaxed);
    total_bytes_written_.fetch_add(total_record_size, std::memory_order_relaxed);

    return seq;
}

Result<void> JournalWriter::flush() noexcept {
    std::lock_guard<std::mutex> lock(rollover_mutex_);
    if (!is_open_ || !current_mmap_.is_mapped()) {
        return Status::kNotInitialized;
    }
    return current_mmap_.flush();
}

Result<void> JournalWriter::close() noexcept {
    std::lock_guard<std::mutex> lock(rollover_mutex_);
    if (!is_open_) {
        return Status::kOk;
    }
    if (current_mmap_.is_mapped()) {
        (void)current_mmap_.flush();
        current_mmap_.close();
    }
    is_open_ = false;
    return Status::kOk;
}

} // namespace ume::event
