/// @file memory_statistics.cpp
/// @brief Implementation of live memory metrics collection across RAM, VRAM, and SSD tiers.

#include "ume/event/memory_statistics.h"

#include <algorithm>

namespace ume::event {

void StatisticsCollector::record_allocation(TierClass tier, uint64_t size_bytes,
                                            uint32_t pid) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    total_allocations_.fetch_add(1, std::memory_order_relaxed);
    total_bytes_allocated_.fetch_add(size_bytes, std::memory_order_relaxed);

    // Update Tier stats
    switch (tier) {
        case TierClass::kVram:
            current_vram_bytes_ += size_bytes;
            peak_vram_bytes_ = (std::max)(peak_vram_bytes_, current_vram_bytes_);
            break;
        case TierClass::kNvme:
            current_ssd_bytes_ += size_bytes;
            peak_ssd_bytes_ = (std::max)(peak_ssd_bytes_, current_ssd_bytes_);
            break;
        case TierClass::kRam:
        default:
            current_ram_bytes_ += size_bytes;
            peak_ram_bytes_ = (std::max)(peak_ram_bytes_, current_ram_bytes_);
            break;
    }

    auto& tstats = tier_stats_[static_cast<uint8_t>(tier)];
    tstats.current_bytes += size_bytes;
    tstats.peak_bytes = (std::max)(tstats.peak_bytes, tstats.current_bytes);
    tstats.allocation_count++;

    if (pid != 0) {
        auto& pstats = process_stats_[pid];
        pstats.process_id = pid;
        pstats.current_bytes += size_bytes;
        pstats.peak_bytes = (std::max)(pstats.peak_bytes, pstats.current_bytes);
        pstats.allocation_count++;
    }
}

void StatisticsCollector::record_free(TierClass tier, uint64_t size_bytes, uint64_t lifetime_ns,
                                      uint32_t pid) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    total_frees_.fetch_add(1, std::memory_order_relaxed);
    total_bytes_freed_.fetch_add(size_bytes, std::memory_order_relaxed);

    if (lifetime_ns > 0) {
        total_lifetime_ns_ += lifetime_ns;
        completed_lifetimes_count_++;
    }

    switch (tier) {
        case TierClass::kVram:
            if (current_vram_bytes_ >= size_bytes)
                current_vram_bytes_ -= size_bytes;
            else
                current_vram_bytes_ = 0;
            break;
        case TierClass::kNvme:
            if (current_ssd_bytes_ >= size_bytes)
                current_ssd_bytes_ -= size_bytes;
            else
                current_ssd_bytes_ = 0;
            break;
        case TierClass::kRam:
        default:
            if (current_ram_bytes_ >= size_bytes)
                current_ram_bytes_ -= size_bytes;
            else
                current_ram_bytes_ = 0;
            break;
    }

    auto& tstats = tier_stats_[static_cast<uint8_t>(tier)];
    if (tstats.current_bytes >= size_bytes)
        tstats.current_bytes -= size_bytes;
    else
        tstats.current_bytes = 0;
    tstats.free_count++;

    if (pid != 0) {
        auto& pstats = process_stats_[pid];
        if (pstats.current_bytes >= size_bytes)
            pstats.current_bytes -= size_bytes;
        else
            pstats.current_bytes = 0;
        pstats.free_count++;
    }
}

void StatisticsCollector::record_migration(TierClass src_tier, TierClass tgt_tier, uint64_t bytes,
                                           uint32_t pid) noexcept {
    record_free(src_tier, bytes, 0, pid);
    record_allocation(tgt_tier, bytes, pid);
}

MemoryStatisticsSnapshot StatisticsCollector::snapshot() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);

    MemoryStatisticsSnapshot snap{};
    snap.current_ram_bytes = current_ram_bytes_;
    snap.peak_ram_bytes = peak_ram_bytes_;
    snap.current_vram_bytes = current_vram_bytes_;
    snap.peak_vram_bytes = peak_vram_bytes_;
    snap.current_ssd_bytes = current_ssd_bytes_;
    snap.peak_ssd_bytes = peak_ssd_bytes_;

    snap.total_allocations = total_allocations_.load(std::memory_order_relaxed);
    snap.total_frees = total_frees_.load(std::memory_order_relaxed);
    snap.total_bytes_allocated = total_bytes_allocated_.load(std::memory_order_relaxed);
    snap.total_bytes_freed = total_bytes_freed_.load(std::memory_order_relaxed);

    if (snap.total_allocations > 0) {
        snap.avg_allocation_size_bytes = static_cast<double>(snap.total_bytes_allocated) /
                                         static_cast<double>(snap.total_allocations);
    }

    if (completed_lifetimes_count_ > 0) {
        snap.avg_lifetime_ns = static_cast<double>(total_lifetime_ns_) /
                               static_cast<double>(completed_lifetimes_count_);
    }

    if (snap.total_allocations >= snap.total_frees) {
        snap.active_allocations_count = snap.total_allocations - snap.total_frees;
    }

    return snap;
}

TierStatistics StatisticsCollector::tier_statistics(TierClass tier) const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = tier_stats_.find(static_cast<uint8_t>(tier));
    if (it != tier_stats_.end()) {
        return it->second;
    }
    return TierStatistics{};
}

ProcessStatistics StatisticsCollector::process_statistics(uint32_t pid) const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = process_stats_.find(pid);
    if (it != process_stats_.end()) {
        return it->second;
    }
    return ProcessStatistics{};
}

void StatisticsCollector::reset() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    current_ram_bytes_ = 0;
    peak_ram_bytes_ = 0;
    current_vram_bytes_ = 0;
    peak_vram_bytes_ = 0;
    current_ssd_bytes_ = 0;
    peak_ssd_bytes_ = 0;

    total_allocations_.store(0);
    total_frees_.store(0);
    total_bytes_allocated_.store(0);
    total_bytes_freed_.store(0);

    total_lifetime_ns_ = 0;
    completed_lifetimes_count_ = 0;

    tier_stats_.clear();
    process_stats_.clear();
}

} // namespace ume::event
