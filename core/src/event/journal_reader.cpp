/// @file journal_reader.cpp
/// @brief Implementation of sequential journal segment reader and record validation.

#include "ume/event/journal_reader.h"

#include <algorithm>
#include <cstring>
#include <filesystem>

namespace ume::event {

void JournalReader::close() noexcept {
    if (current_mmap_.is_mapped()) {
        current_mmap_.close();
    }
    segment_paths_.clear();
    current_segment_index_ = 0;
    current_offset_ = sizeof(JournalSegmentHeader);
    is_open_ = false;
}

Result<void> JournalReader::open_segment(const std::string& segment_path) noexcept {
    close();
    segment_paths_.push_back(segment_path);
    current_segment_index_ = 0;
    return load_current_segment();
}

Result<void> JournalReader::open_directory(const std::string& journal_directory) noexcept {
    close();
    std::error_code ec;
    if (!std::filesystem::exists(journal_directory, ec)) {
        return Status::kFileNotFound;
    }

    for (const auto& entry : std::filesystem::directory_iterator(journal_directory, ec)) {
        if (entry.is_regular_file() && entry.path().extension() == ".umej") {
            segment_paths_.push_back(entry.path().string());
        }
    }

    if (segment_paths_.empty()) {
        return Status::kFileNotFound;
    }

    std::sort(segment_paths_.begin(), segment_paths_.end());
    current_segment_index_ = 0;
    return load_current_segment();
}

Result<void> JournalReader::load_current_segment() noexcept {
    if (current_segment_index_ >= segment_paths_.size()) {
        return Status::kNotFound;
    }

    if (current_mmap_.is_mapped()) {
        current_mmap_.close();
    }

    const Status open_res = current_mmap_.open_read_only(segment_paths_[current_segment_index_]);
    if (is_error(open_res)) {
        return open_res;
    }

    if (current_mmap_.size() < sizeof(JournalSegmentHeader)) {
        return Status::kJournalCorrupted;
    }

    std::memcpy(&current_segment_header_, current_mmap_.data(), sizeof(JournalSegmentHeader));
    if (current_segment_header_.magic != 0x554D454A) { // "UMEJ"
        return Status::kJournalCorrupted;
    }

    const uint32_t computed_crc =
        compute_crc32(reinterpret_cast<const uint8_t*>(&current_segment_header_), 44);
    if (computed_crc != current_segment_header_.header_crc32) {
        return Status::kJournalCorrupted;
    }

    current_offset_ = sizeof(JournalSegmentHeader);
    stats_.segments_read++;
    is_open_ = true;

    return Status::kOk;
}

Result<void> JournalReader::read_next(Event& out_event) noexcept {
    if (!is_open_) {
        return Status::kNotInitialized;
    }

    while (true) {
        const size_t total_mapped_size = current_mmap_.size();
        constexpr size_t record_min_size = sizeof(JournalRecordHeader) + sizeof(Event);

        if (current_offset_ + record_min_size > total_mapped_size) {
            // End of current segment, try loading next segment
            current_segment_index_++;
            const auto load_res = load_current_segment();
            if (load_res.ok()) {
                continue;
            } else {
                return Status::kNotFound; // End of journal
            }
        }

        const uint8_t* record_ptr = current_mmap_.data() + current_offset_;
        JournalRecordHeader record_hdr{};
        std::memcpy(&record_hdr, record_ptr, sizeof(JournalRecordHeader));

        // Validate Record Magic
        if (record_hdr.magic != 0x52454344) { // "RECD"
            // Corrupted byte, scan forward by 1 byte
            current_offset_++;
            stats_.corrupted_records_skipped++;
            continue;
        }

        // Validate Record Header CRC32
        const uint32_t header_crc = compute_crc32(record_ptr, 24);
        if (header_crc != record_hdr.header_crc32) {
            current_offset_ += 8; // Advance 8-byte boundary
            stats_.corrupted_records_skipped++;
            continue;
        }

        // Validate Payload CRC32
        const uint8_t* payload_ptr = record_ptr + sizeof(JournalRecordHeader);
        const uint32_t payload_crc = compute_crc32(payload_ptr, sizeof(Event));
        if (payload_crc != record_hdr.payload_crc32) {
            current_offset_ += record_hdr.record_size;
            stats_.corrupted_records_skipped++;
            continue;
        }

        // Deserialize Event
        const auto deser_res = EventSerializer::deserialize(payload_ptr, sizeof(Event));
        if (!deser_res.ok()) {
            current_offset_ += record_hdr.record_size;
            stats_.corrupted_records_skipped++;
            continue;
        }

        out_event = deser_res.value();
        current_offset_ += record_hdr.record_size;
        stats_.records_read++;
        stats_.bytes_read += record_hdr.record_size;

        return Status::kOk;
    }
}

} // namespace ume::event
