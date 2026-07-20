#pragma once

/// @file memory_statistics.h
/// @brief Live memory metrics tracking across RAM, VRAM, and SSD tiers.
///
/// Part of UME Milestone M3 (Task T007).

#include "ume/status.h"
#include "ume/types.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace ume::event {

/// @brief Point-in-time snapshot of memory statistics across all tiers and processes.
struct MemoryStatisticsSnapshot {
    uint64_t current_ram_bytes{0};
    uint64_t peak_ram_bytes{0};
    uint64_t current_vram_bytes{0};
    uint64_t peak_vram_bytes{0};
    uint64_t current_ssd_bytes{0};
    uint64_t peak_ssd_bytes{0};

    uint64_t total_allocations{0};
    uint64_t total_frees{0};
    uint64_t total_bytes_allocated{0};
    uint64_t total_bytes_freed{0};

    double avg_allocation_size_bytes{0.0};
    double avg_lifetime_ns{0.0};
    uint64_t active_allocations_count{0};
};

/// @brief Per-tier statistics metrics.
struct TierStatistics {
    uint64_t current_bytes{0};
    uint64_t peak_bytes{0};
    uint64_t allocation_count{0};
    uint64_t free_count{0};
};

/// @brief Per-process memory metrics.
struct ProcessStatistics {
    uint32_t process_id{0};
    uint64_t current_bytes{0};
    uint64_t peak_bytes{0};
    uint64_t allocation_count{0};
    uint64_t free_count{0};
};

/// @brief Collector and aggregator for live hierarchical memory statistics.
class StatisticsCollector {
public:
    StatisticsCollector() noexcept = default;
    ~StatisticsCollector() = default;

    // Disable copy
    StatisticsCollector(const StatisticsCollector&) = delete;
    StatisticsCollector& operator=(const StatisticsCollector&) = delete;

    /// @brief Records an allocation event.
    void record_allocation(TierClass tier, uint64_t size_bytes, uint32_t pid = 0) noexcept;

    /// @brief Records a free event.
    void record_free(TierClass tier, uint64_t size_bytes, uint64_t lifetime_ns = 0,
                     uint32_t pid = 0) noexcept;

    /// @brief Records a migration between memory tiers.
    void record_migration(TierClass src_tier, TierClass tgt_tier, uint64_t bytes,
                          uint32_t pid = 0) noexcept;

    /// @brief Obtains a point-in-time snapshot of overall memory statistics.
    [[nodiscard]] MemoryStatisticsSnapshot snapshot() const noexcept;

    /// @brief Obtains statistics for a specific tier class.
    [[nodiscard]] TierStatistics tier_statistics(TierClass tier) const noexcept;

    /// @brief Obtains statistics for a specific process ID.
    [[nodiscard]] ProcessStatistics process_statistics(uint32_t pid) const noexcept;

    /// @brief Resets all statistics counters to zero.
    void reset() noexcept;

private:
    mutable std::mutex mutex_;

    uint64_t current_ram_bytes_{0};
    uint64_t peak_ram_bytes_{0};
    uint64_t current_vram_bytes_{0};
    uint64_t peak_vram_bytes_{0};
    uint64_t current_ssd_bytes_{0};
    uint64_t peak_ssd_bytes_{0};

    std::atomic<uint64_t> total_allocations_{0};
    std::atomic<uint64_t> total_frees_{0};
    std::atomic<uint64_t> total_bytes_allocated_{0};
    std::atomic<uint64_t> total_bytes_freed_{0};

    uint64_t total_lifetime_ns_{0};
    uint64_t completed_lifetimes_count_{0};

    std::unordered_map<uint8_t, TierStatistics> tier_stats_;
    std::unordered_map<uint32_t, ProcessStatistics> process_stats_;
};

} // namespace ume::event
